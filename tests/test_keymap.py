"""Check the editable keymap against the full-row Imprint layout."""

import json
from pathlib import Path
import re
import unittest


ROOT = Path(__file__).resolve().parents[1]


class ImprintKeymapTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        keymap = (ROOT / "config/imprint.keymap").read_text()
        cls.layers = [
            re.findall(r"&\w+(?:\s+(?!&)[A-Z0-9_]+|\s+\d+)*", block)
            for block in re.findall(r"bindings\s*=\s*<(.*?)>;", keymap, re.S)
        ]

    def test_three_full_row_layers(self):
        layout = json.loads((ROOT / "config/info.json").read_text())
        self.assertEqual(len(layout["layouts"]["Imprint"]["layout"]), 82)
        self.assertEqual(len(self.layers), 3)
        self.assertEqual([len(layer) for layer in self.layers], [82, 82, 82])

    def test_qwerty_letters(self):
        self.assertEqual(
            self.layers[0][24:60],
            [
                "&kp TAB", "&kp Q", "&kp W", "&kp E", "&kp R", "&kp T",
                "&kp Y", "&kp U", "&kp I", "&kp O", "&kp P", "&kp BSLH",
                "&kp LCTRL", "&kp A", "&kp S", "&kp D", "&kp F", "&kp G",
                "&kp H", "&kp J", "&kp K", "&kp L", "&kp SEMI", "&kp SQT",
                "&kp LSHFT", "&kp Z", "&kp X", "&kp C", "&kp V", "&kp B",
                "&kp N", "&kp M", "&kp COMMA", "&kp DOT", "&kp FSLH", "&kp RSHFT",
            ],
        )

    def test_colemak_dh_letters(self):
        self.assertEqual(
            self.layers[1][24:60],
            [
                "&kp TAB", "&kp Q", "&kp W", "&kp F", "&kp P", "&kp B",
                "&kp J", "&kp L", "&kp U", "&kp Y", "&kp SEMI", "&kp BSLH",
                "&kp LCTRL", "&kp A", "&kp R", "&kp S", "&kp T", "&kp G",
                "&kp M", "&kp N", "&kp E", "&kp I", "&kp O", "&kp SQT",
                "&kp LSHFT", "&kp Z", "&kp X", "&kp C", "&kp D", "&kp V",
                "&kp K", "&kp H", "&kp COMMA", "&kp DOT", "&kp FSLH", "&kp RSHFT",
            ],
        )

    def test_custom_layer_is_unassigned_except_for_cycle_keys(self):
        self.assertEqual(self.layers[2][:77], ["&none"] * 77)
        self.assertEqual(self.layers[2][79:], ["&none"] * 3)

    def test_bluetooth_cycle_is_immediately_right_of_layer_cycle_on_every_layer(self):
        self.assertEqual([layer[78] for layer in self.layers], ["&bt BT_NXT"] * 3)

    def test_five_host_profiles_reserve_a_slot_for_the_peripheral(self):
        central = (ROOT / "config/imprint_left.conf").read_text()
        self.assertRegex(central, r"(?m)^CONFIG_BT_MAX_CONN=6$")
        self.assertRegex(central, r"(?m)^CONFIG_BT_MAX_PAIRED=6$")
        shared = (ROOT / "config/imprint.conf").read_text()
        self.assertNotIn("CONFIG_BT_MAX_CONN", shared)
        self.assertNotIn("CONFIG_BT_MAX_PAIRED", shared)

    def test_left_middle_thumb_cycles_and_other_thumbs_do_not_change_layers(self):
        self.assertEqual([layer[77] for layer in self.layers], ["&to 1", "&to 2", "&to 0"])
        for layer in self.layers:
            self.assertFalse(any(binding.startswith(("&mo ", "&tog ")) for binding in layer))


if __name__ == "__main__":
    unittest.main()
