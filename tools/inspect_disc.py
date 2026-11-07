"""Inventory a user's extracted disc without copying any payload into the repo."""

import argparse
import collections
import hashlib
import json
import struct
from pathlib import Path


def inspect_xex(path):
    data = path.read_bytes()
    if data[:4] != b"XEX2":
        raise ValueError(f"Expected XEX2: {path}")

    def u32(offset):
        return struct.unpack_from(">I", data, offset)[0]

    headers = {u32(24 + i * 8): u32(28 + i * 8) for i in range(u32(20))}
    execution = headers[0x40006]
    file_format = headers[0x3FF]
    security = u32(16)
    return {
        "size": len(data),
        "sha256": hashlib.sha256(data).hexdigest(),
        "module_flags": f"{u32(4):08X}",
        "header_size": u32(8),
        "entry_point": f"{headers[0x10100]:08X}",
        "base_address": f"{u32(security + 0x110):08X}",
        "image_size": u32(security + 4),
        "encryption": struct.unpack_from(">H", data, file_format + 4)[0],
        "compression": struct.unpack_from(">H", data, file_format + 6)[0],
        "lzx_window": u32(file_format + 8),
        "media_id": f"{u32(execution):08X}",
        "version": f"{u32(execution + 4):08X}",
        "base_version": f"{u32(execution + 8):08X}",
        "title_id": f"{u32(execution + 12):08X}",
        "region": f"{u32(security + 0x178):08X}",
        "allowed_media": f"{u32(security + 0x17C):08X}",
        "page_descriptors": u32(security + 0x180),
    }


def inspect_big(path):
    size = path.stat().st_size
    with path.open("rb") as archive:
        header = archive.read(16)
        magic = header[:4]
        if magic not in (b"BIG4", b"BIGF"):
            raise ValueError(f"Unrecognized BIG archive: {path}")
        declared_size = struct.unpack_from("<I", header, 4)[0]
        count, directory_end = struct.unpack_from(">II", header, 8)
        if declared_size != size or not 16 <= directory_end <= size:
            raise ValueError(f"Invalid BIG header size: {path}")
        if count > (directory_end - 16) // 9:
            raise ValueError(f"Invalid BIG directory count: {path}")
        directory = archive.read(directory_end - 16)

    entries = []
    cursor = 0
    for _ in range(count):
        offset, length = struct.unpack_from(">II", directory, cursor)
        name_end = directory.index(b"\0", cursor + 8)
        name = directory[cursor + 8 : name_end].decode("ascii")
        if offset < directory_end or offset + length > size:
            raise ValueError(f"Invalid BIG member range in {path}: {name}")
        entries.append({"name": name, "offset": offset, "size": length})
        cursor = name_end + 1
    return {
        "magic": magic.decode("ascii"),
        "size": size,
        "entries": count,
        "directory_end": directory_end,
        "extensions": dict(collections.Counter(
            Path(entry["name"]).suffix.lower() or "(none)" for entry in entries
        )),
        "members": entries,
    }


def inspect_disc(root):
    files = sorted(
        path for path in root.rglob("*")
        if path.is_file() and ".inspection" not in path.relative_to(root).parts
    )
    inventory = [
        {"path": path.relative_to(root).as_posix(), "size": path.stat().st_size}
        for path in files
    ]
    archives = {
        path.relative_to(root).as_posix(): inspect_big(path)
        for path in files if path.suffix.lower() == ".big"
    }
    native_report = root / ".inspection/xex.tsv"
    libraries = []
    sections = []
    imports = []
    if native_report.exists():
        for line in native_report.read_text(encoding="utf-8").splitlines():
            fields = line.split("\t")
            if fields[0] == "section":
                sections.append({
                    "name": fields[1], "address": fields[2].upper(),
                    "size": int(fields[3], 16),
                    "executable": fields[4] == "1", "writable": fields[5] == "1",
                })
            elif fields[0] == "library":
                libraries.append({
                    "name": fields[1], "version": fields[2].upper(),
                    "minimum_version": fields[3].upper(),
                })
            elif fields[0] == "import":
                imports.append({
                    "library": fields[1], "ordinal": int(fields[2], 16),
                    "name": fields[3], "tags": int(fields[6], 16),
                })
    return {
        "xex": inspect_xex(root / "default.xex"),
        "files": inventory,
        "total_bytes": sum(entry["size"] for entry in inventory),
        "extensions": dict(collections.Counter(path.suffix.lower() or "(none)" for path in files)),
        "archives": archives,
        "sections": sections,
        "libraries": libraries,
        "imports": imports,
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--title", choices=("skate", "skate3"), default="skate")
    parser.add_argument("--config", type=Path, default=Path(__file__).resolve().parents[1] / "local.json")
    parser.add_argument("--output", type=Path, required=True, help="Local metadata JSON, usually under build/")
    args = parser.parse_args()
    settings = json.loads(args.config.read_text(encoding="utf-8-sig"))
    root = Path(settings[args.title]["extracted"]).resolve(strict=True)
    report = inspect_disc(root)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(f"Inventoried {len(report['files'])} files and {len(report['archives'])} BIG archives.")


if __name__ == "__main__":
    main()
