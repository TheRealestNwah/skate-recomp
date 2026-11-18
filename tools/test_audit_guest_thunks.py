import struct
import unittest

from audit_guest_thunks import find_candidates, read_layout


class GuestThunkAuditTests(unittest.TestCase):
    def test_finds_only_unregistered_pointer_adjustment_thunks(self):
        image = bytearray(0x80)
        struct.pack_into(">6I", image, 0, 0x1028, 0x1030, 0x1038, 0x1040, 0x1029, 0x107C)
        # 0x1028 adjusts this by -4 and branches backward to 0x1020.
        struct.pack_into(">II", image, 0x28, 0x3863FFFC, 0x4BFFFFF4)
        # 0x1030 is already registered.
        struct.pack_into(">II", image, 0x30, 0x3863FFFC, 0x4BFFFFEC)
        # 0x1038 branches with link, and 0x1040 targets an unmapped entry.
        struct.pack_into(">II", image, 0x38, 0x3863FFFC, 0x4BFFFFE5)
        struct.pack_into(">II", image, 0x40, 0x38630004, 0x48000004)

        candidates = find_candidates(image, 0x1000, (0x1000, 0x18), (0x1020, 0x60), {0x1020, 0x1030})

        self.assertEqual(list(candidates), [(0x1000, 0x1028, -4, 0x1020)])

    def test_rejects_a_section_outside_the_inspected_image(self):
        with self.assertRaisesRegex(ValueError, "Invalid section range"):
            list(find_candidates(bytes(16), 0x1000, (0x1000, 8), (0x1008, 12), set()))

    def test_uses_addresses_and_sizes_from_inspector_report(self):
        image_range, sections = read_layout(
            "image\t82000000\t80\t82000030\n"
            "section\t.text\t82000020\t60\t1\t0\n"
            "section\t.rdata\t82000000\t20\t0\t0\n"
        )
        self.assertEqual(image_range, (0x82000000, 0x80))
        self.assertEqual(sections, {".text": (0x82000020, 0x60), ".rdata": (0x82000000, 0x20)})
