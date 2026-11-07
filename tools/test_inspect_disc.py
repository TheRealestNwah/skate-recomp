import struct
import tempfile
import unittest
from pathlib import Path

from inspect_disc import inspect_big


class BigDirectoryTests(unittest.TestCase):
    def test_reads_mixed_endian_header_and_big_endian_members(self):
        for magic in (b"BIG4", b"BIGF"):
            with self.subTest(magic=magic):
                result = self.inspect_archive(magic, offset=32, member_size=3)
                self.assertEqual(result["directory_end"], 32)
                self.assertEqual(result["entries"], 1)
                self.assertEqual(result["members"], [{"name": "one.rx2", "offset": 32, "size": 3}])

    def test_rejects_member_ranges_outside_the_data_area(self):
        for offset, member_size in ((31, 3), (32, 4)):
            with self.subTest(offset=offset, member_size=member_size):
                with self.assertRaisesRegex(ValueError, "Invalid BIG member range"):
                    self.inspect_archive(b"BIG4", offset, member_size)

    @staticmethod
    def inspect_archive(magic, offset, member_size):
        member = struct.pack(">II", offset, member_size) + b"one.rx2\0"
        header = magic + struct.pack("<I", 35) + struct.pack(">II", 1, 32)
        with tempfile.TemporaryDirectory() as scratch:
            path = Path(scratch) / "test.big"
            path.write_bytes(header + member + b"abc")
            return inspect_big(path)


if __name__ == "__main__":
    unittest.main()
