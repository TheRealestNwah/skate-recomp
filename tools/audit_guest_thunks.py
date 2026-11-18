"""Find potential unmapped PowerPC adjustment thunks in a local XEX inspection."""

import argparse
import re
import struct
from pathlib import Path


def read_layout(report):
    image_range = None
    sections = {}
    for line in report.splitlines():
        fields = line.split("\t")
        if fields[0] == "image":
            image_range = int(fields[1], 16), int(fields[2], 16)
        elif fields[0] == "section" and fields[1] in {".rdata", ".text"}:
            sections[fields[1]] = int(fields[2], 16), int(fields[3], 16)
    if image_range is None or sections.keys() != {".rdata", ".text"}:
        raise ValueError("Expected image, .rdata, and .text records from inspect_xex")
    return image_range, sections


def find_candidates(image, base, rdata, text, registered):
    rdata_start, rdata_size = rdata
    text_start, text_size = text
    for start, size in (rdata, text):
        if start % 4 or size % 4 or not base <= start < start + size <= base + len(image):
            raise ValueError("Invalid section range in the XEX inspection")

    rdata_offset = rdata_start - base
    text_end = text_start + text_size
    for index, (entry,) in enumerate(
        struct.iter_unpack(">I", memoryview(image)[rdata_offset : rdata_offset + rdata_size])
    ):
        if entry % 4 or not text_start <= entry <= text_end - 8 or entry in registered:
            continue
        adjust, branch = struct.unpack_from(">II", image, entry - base)
        # addi r3,r3,imm followed by a relative branch without link.
        if adjust >> 16 != 0x3863 or (branch & 0xFC000003) != 0x48000000:
            continue
        displacement = ((branch & 0x03FFFFFC) ^ 0x02000000) - 0x02000000
        destination = entry + 4 + displacement
        if text_start <= destination < text_end and destination in registered:
            adjustment = ((adjust & 0xFFFF) ^ 0x8000) - 0x8000
            yield rdata_start + index * 4, entry, adjustment, destination


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("report", type=Path, help="xex.tsv from inspect_xex, beside image.bin")
    parser.add_argument("registration", type=Path, help="Generated module's *_register.cpp")
    args = parser.parse_args()
    try:
        (base, image_size), sections = read_layout(args.report.read_text(encoding="utf-8"))
        image = args.report.with_name("image.bin").read_bytes()
        if len(image) != image_size:
            raise ValueError("image.bin size differs from xex.tsv")
        registered = {
            int(address, 16)
            for address in re.findall(
                r"\bSetFunction\(\s*0x([0-9a-fA-F]+)\s*,",
                args.registration.read_text(encoding="utf-8"),
            )
        }
        if not registered:
            raise ValueError("No SetFunction registrations found")
        candidates = list(
            find_candidates(image, base, sections[".rdata"], sections[".text"], registered)
        )
    except (OSError, IndexError, ValueError) as error:
        parser.exit(1, f"{error}\n")

    print("slot\tentry\tthis_adjustment\ttail_target")
    for slot, entry, adjustment, destination in candidates:
        print(f"0x{slot:08X}\t0x{entry:08X}\t{adjustment:+#x}\t0x{destination:08X}")


if __name__ == "__main__":
    main()
