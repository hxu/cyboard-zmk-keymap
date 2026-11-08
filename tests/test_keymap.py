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

    def effective_layer(self, layer):
        # &to leaves only the selected layer and base layer active.
        return [
            base if binding == "&trans" else binding
            for base, binding in zip(self.layers[0], layer)
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
                "&kp CAPS", "&kp A", "&kp S", "&kp D", "&kp F", "&kp G",
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
                "&kp CAPS", "&kp A", "&kp R", "&kp S", "&kp T", "&kp G",
                "&kp M", "&kp N", "&kp E", "&kp I", "&kp O", "&kp SQT",
                "&kp LSHFT", "&kp Z", "&kp X", "&kp C", "&kp D", "&kp V",
                "&kp K", "&kp H", "&kp COMMA", "&kp DOT", "&kp FSLH", "&kp RSHFT",
            ],
        )

    def test_custom_letters_stay_blank_and_controls_match_qwerty(self):
        letter_positions = set(range(25, 35)) | set(range(37, 46)) | set(range(49, 56))
        custom = self.effective_layer(self.layers[2])
        for position in range(82):
            if position in letter_positions:
                self.assertEqual(custom[position], "&none")
            elif position != 77:  # Each layer's cycle key has its own target.
                self.assertEqual(custom[position], self.layers[0][position])

    def test_confirmed_controls_on_every_layer(self):
        for layer in self.layers:
            layer = self.effective_layer(layer)
            self.assertEqual([layer[i] for i in [0, 12, 24, 36, 48]],
                             ["&kp ESC", "&kp GRAVE", "&kp TAB", "&kp CAPS", "&kp LSHFT"])
            self.assertEqual(layer[60:70], [
                "&kp LCTRL", "&kp LALT", "&kp DOWN", "&kp UP", "&none",
                "&kp EQUAL", "&kp LEFT", "&kp RIGHT", "&kp LBKT", "&kp RBKT",
            ])
            self.assertEqual(layer[70:77], [
                "&kp RET", "&kp DEL", "&none", "&kp HOME", "&kp BSPC", "&kp SPACE", "&kp LGUI",
            ])
            self.assertEqual(layer[79:82], ["&kp END", "&mkp LCLK", "&mkp RCLK"])

    def test_left_trackball_scroll_sensitivity_is_halved(self):
        keymap = (ROOT / "config/imprint.keymap").read_text()
        listener = keymap.split("&trackball_central_listener", 1)[1]
        self.assertIn("<&zip_xy_scaler 1 12>", listener)
        self.assertIn("<&zip_xy_to_scroll_mapper>", listener)
        self.assertIn("<&zip_scroll_transform INPUT_TRANSFORM_Y_INVERT>", listener)

    def test_bluetooth_cycle_is_immediately_right_of_layer_cycle_on_every_layer(self):
        self.assertEqual(
            [self.effective_layer(layer)[78] for layer in self.layers],
            ["&bt BT_NXT"] * 3,
        )

    def test_five_host_profiles_reserve_a_slot_for_the_peripheral(self):
        central = (ROOT / "config/imprint_left.conf").read_text()
        self.assertRegex(central, r"(?m)^CONFIG_BT_MAX_CONN=6$")
        self.assertRegex(central, r"(?m)^CONFIG_BT_MAX_PAIRED=6$")
        shared = (ROOT / "config/imprint.conf").read_text()
        self.assertNotIn("CONFIG_BT_MAX_CONN", shared)
        self.assertNotIn("CONFIG_BT_MAX_PAIRED", shared)

    def test_left_middle_thumb_cycles_and_other_thumbs_do_not_change_layers(self):
        self.assertEqual([layer[77] for layer in self.layers], ["&to 1", "&to 2", "&to 0"])
        for layer in self.layers[1:]:
            self.assertEqual(layer[70:77] + layer[78:82], ["&trans"] * 11)
        for layer in self.layers:
            self.assertFalse(any(binding.startswith(("&mo ", "&tog ")) for binding in layer))


if __name__ == "__main__":
    unittest.main()
