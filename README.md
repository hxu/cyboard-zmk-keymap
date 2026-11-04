# ZMK Configuration Template
Wireless Cyboard keyboard configuration repository template for using ZMK firmware. [Instructions for use are located on our documentation site](https://docs.cyboard.digital/user-manual/quick-start/configure-layout).

## Personal Imprint layout

`config/imprint.keymap` selects the Imprint with a full function row and full
bottom row. It has three layers:

| Studio layer | ZMK layer ID | Layout | RGB color |
| --- | --- | --- | --- |
| Qwerty | 0 | Qwerty | Red |
| Colemak-DH | 1 | [Colemak-DH matrix letters](https://colemakmods.github.io/mod-dh/keyboards.html) | Blue |
| Custom | 2 | Blank letter positions with shared controls | Green |

All three layers share the following controls (each row reads left to right):

| Area | Bindings |
| --- | --- |
| Left outer column, top to bottom | Esc, grave/tilde, Tab, Caps Lock, Shift, Ctrl |
| Left bottom row | Ctrl, Alt/Option, Down, Up, unassigned |
| Right bottom row | Equals, Left, Right, Left bracket, Right bracket |
| Left upper thumb arc | Enter, Delete (forward), unassigned Function key |
| Left lower thumb arc | Command/GUI, layer cycle, Bluetooth cycle |
| Right upper thumb arc | Home, Backspace, Space |
| Right lower thumb arc | End, left mouse button, right mouse button |

Custom keeps its 26 letter positions unassigned, with the Qwerty layer's numbers,
function keys, punctuation, modifiers, navigation, and thumb controls. The
Function thumb key and the last key of the left bottom row use `&none` on all
layers, so neither falls through to a lower layer.

Tap the middle key on the lower left thumb arc to cycle through these layers.
This is the former `&mo 1` key, immediately after `LGUI` in the last keymap row
(binding position 77, counting from zero).
Its bindings are `&to 1`, `&to 2`, and `&to 0`. Keep these bindings when editing
the layers in Studio, so you can always leave the Custom layer.

The key immediately to its right (the innermost key on the lower left thumb arc,
binding position 78, counting from zero) cycles Bluetooth profiles with
`&bt BT_NXT` on all three layers. It cycles profiles 1 → 2 → 3 → 4 → 5 → 1
(ZMK IDs 0–4). An unused profile advertises for pairing; a paired profile
selects its host. The left-half config explicitly reserves six connections
and bonds: five hosts plus the right half. Five is the configured profile
count, not a hardware maximum; increasing it requires a rebuild and memory
validation. USB output takes precedence while connected by USB, so test host
switching with the left USB cable disconnected.

The left half also exposes the right half's battery level through an additional
Bluetooth Battery Service. Both halves must be on and connected. The normal
Bluetooth battery percentage is the left half's level; reading both values may
require a BLE inspection app because host support for multiple batteries varies.

Switching Bluetooth profiles briefly colors the entire keyboard for 1.5 seconds:

| Host profile | ZMK ID | Flash color |
| --- | --- | --- |
| 1 | 0 | Cyan |
| 2 | 1 | Magenta |
| 3 | 2 | Yellow |
| 4 | 3 | Orange |
| 5 | 4 | White |

These colors are separate from the red, blue, and green layer colors. After the
flash, lighting returns to the currently active layer. Rapid profile changes
restart the timer with the latest profile's color. Layer changes during a flash
take effect when the flash ends. Reconnecting to the same host does not trigger
or extend the flash. The saved Bluetooth profile loads without flashing at boot.
The indication is sent to both halves through the existing split RGB behaviors;
it shows the selected profile, not confirmation of a connected host.

The firmware sets solid RGB colors with brightness setting 25. It updates the color on
layer changes, after saved settings load at startup, and when it receives the
right half's battery report. Each half turns its lighting off after 30 seconds
without input. ZMK restores the previous on/off state when activity resumes.
Color updates and profile flashes preserve that state, so they cannot turn
idle or manually disabled lighting back on. Lighting starts on again after a
restart or deep-sleep wake, even if idle shutoff saved an off state.
Colors follow the layer IDs even if you rename or reorder layers in Studio.
Color changes require a firmware rebuild; normal key
changes can still be made in Studio. The Custom layer uses `&none`, so its empty
keys do not fall through to Qwerty. The left trackball keeps its scrolling setup,
with its scale reduced from 1/3 to 1/6 for half the previous sensitivity; the right
trackball's pointer sensitivity is unchanged.

Flash both halves from the same successful GitHub Actions build. If the keyboard
already has a keymap saved by Studio, save a copy of those edits and reset the
keymap to the firmware defaults in Studio to load these three layers. A normal
reflash preserves the Studio keymap and does not replace it with these defaults.

After flashing, check that each tap cycles red/Qwerty, blue/Colemak-DH,
green/Custom, and back to red/Qwerty. Check that Custom's letters produce no key
output, but its shared controls and cycle keys still work. Restart the keyboard from blue
or green and check that it returns to red/Qwerty. Also restart the right half
while Colemak-DH or Custom is active and check its color after it reconnects.
Unlock Studio with the existing A-position + F-position hold combo to configure
Custom. The combo uses physical positions and also works on the blank layer.

With both halves on battery, leave them untouched for at least 30 seconds and
check that both lights go out. Press a key on each half to check that its light
returns with the current layer color. Turn RGB off manually and check that layer
changes, profile changes, and right-half battery updates do not turn it back on.
After at least 15 minutes without input, press a key on each half and check that
lighting returns after waking from deep sleep. Use a BLE inspection app to read
the additional Battery Service while both halves are connected.

After updating this keymap, preserve any Studio customizations before using
**Restore Stock Settings** to load the new Bluetooth binding. Alternatively,
assign Bluetooth → Next Profile to the innermost lower-left thumb key and Move
to Layer → 1 / 2 / 0 to the middle lower-left thumb key on Qwerty / Colemak-DH /
Custom, respectively, directly in Studio. Test all five profiles and wraparound on every layer,
including Custom, and confirm the layer-cycle key still works independently.
Check each profile's flash color and return to the layer color after 1.5 seconds.
Also test quick repeated presses, changing layers during the flash, and a host
reconnect without a new profile selection. The flash works over USB even though
USB remains the preferred output. No Studio reset is needed for this lighting
update if your saved keymap already has the Bluetooth and layer cycle bindings.
For the revised keycap layout, preserve your Studio edits and use Restore Stock
Settings after flashing, or apply the new bindings to every layer in Studio.
Verify forward Delete versus Backspace, the unassigned keys, both mouse buttons
(including click-and-drag), the arrow keys, and the slower left-ball scrolling.

## Cyboard trackball configuration extension

This repository builds upstream ZMK with Studio **keymap editing** support.
It does not include Cyboard's trackball-configuration extension. The warning
in Cyboard Studio is independent of saved keymaps and cannot be fixed by
restoring stock settings or enabling `CONFIG_ZMK_STUDIO` again. Trackball
movement and the left ball's compiled-in scrolling configuration still use
the public driver.

The official [imprint-v2026.09.27 release](https://github.com/Cyboard-DigitalTailor/cyboard-imprint-zmk-studio-firmware/releases/tag/imprint-v2026.09.27)
includes that extension. Its published
[provenance.txt](https://github.com/Cyboard-DigitalTailor/cyboard-imprint-zmk-studio-firmware/releases/download/imprint-v2026.09.27/provenance.txt)
lists these sources:

| Component | Release revision |
| --- | --- |
| `zmk-private` (`develop`) | `83b72af1` |
| `zmk-keyboards` (`main`) | `522529e` |
| `zmk-pmw3610-driver-private` (`feature/axis-runtime-config`) | `ba040ba` |
| `zmk-studio-messages` | `8b1bd9e` |
| `zmk-environment` (`main`) | `d29f093` |

The private ZMK and trackball driver sources are unavailable to the current
GitHub account. Public `zmk` and `zmk-pmw3610-driver` branches are not substitutes
for these extensions. Migrating this custom build requires access to the
vendor's source stack and its matching Studio protocol/build configuration.
Until then, the [official firmware](https://docs.cyboard.digital/studio-firmware-update)
provides trackball configuration, but does not compile this repo's custom RGB
indicator code. Keep the current pins until a compatible migration can be built
and verified on both halves.

Run the keymap checks with:

```sh
uv run --no-project python -m unittest discover -s tests -v
```

The flash timing and lighting callback tests compile C code, so they need a host C compiler.
If one is unavailable, run the same command with `uv run --with ziglang` instead
of `uv run`.
The lighting callback test compiles both half roles with mocked Zephyr and ZMK
boundaries; it checks startup lighting and that color updates do not force RGB on.
These host tests do not verify hardware sleep or Bluetooth battery reporting.

> **Already have a copy of this template from before July 2026?** The board and
> layout definitions were reorganized in July 2026, and older copies track the
> moving `main` branch — so a rebuild can silently break your keymap (most often
> a dead thumb cluster). Follow [Updating a config repo created before July
> 2026](#updating-a-config-repo-created-before-july-2026) to move onto the pinned
> stable stack; after that, [Pinned versions](#pinned-versions) keeps it from
> happening again.

## Pinned versions
`config/west.yml` pins two projects to fixed releases so your firmware is reproducible and cannot change under you:

- **`zmk`** → upstream ZMK `v0.3.0`, supporting Studio keymap editing. This is not the private firmware stack that provides Cyboard Studio trackball configuration; see above.
- **`zmk-keyboards`** → a tagged release (currently `v2026.07`) of the Cyboard board, shield, and physical-layout definitions. Pinning this is what keeps your keymap working: the physical layouts — key positions, layout names, and the order bindings appear in — are frozen at that tag, so a later change on `zmk-keyboards` `main` cannot shift them under your keymap and silently break keys (e.g. a dead thumb cluster after a rebuild).

Version bumps are deliberate: when you want newer boards, layouts, or a newer stable ZMK, edit `config/west.yml` (and, for a new ZMK, the matching tag in `.github/workflows/build.yml`), push, and let CI prove the build before you flash. The [zmk-keyboards releases](https://github.com/Cyboard-DigitalTailor/zmk-keyboards/releases) page lists what each tag contains.

To track current ZMK `main` (Zephyr 4.1) instead, edit `config/west.yml`: set the `zmk` revision to `main` and the `zmk-keyboards` revision to `zephyr-4.1`.

## ZMK Studio support

The left-half firmware built from this template has [ZMK Studio](https://studio.cyboard.digital) enabled (see the `imprint_left` entry in `build.yaml`), so you can edit your keymap live over USB without reflashing. To unlock the keyboard for Studio, **hold the A- and F-position keys (left home row) for 3 seconds**. The unlock combo is tied to the physical key locations, so it keeps working no matter how you remap your keymap.

Keymap changes made in Studio are stored in the keyboard's flash, separately from the `.keymap` file in this repo: they survive reflashes of firmware built from this repo, and the `.keymap` file only provides the defaults Studio starts from (or falls back to after a settings reset).

## Selecting your keyboard model

The keymap selects your keyboard variant with a chosen **physical layout** node, e.g.:

```dts
chosen { zmk,physical-layout = &physical_layout_imprint_number_row; };
```

The available Imprint layouts are defined in
[`zmk-keyboards`](https://github.com/Cyboard-DigitalTailor/zmk-keyboards/blob/main/boards/shields/imprint/imprint-layouts.dtsi);
the `config/default keymaps/` folders contain a matching keymap for each. The
Dactyl and legacy single-arc keymaps still use the older
`zmk,matrix-transform` chosen node, as those models predate the physical
layout definitions.

## Updating a config repo created before July 2026

Older configs track ZMK `main` and select the keyboard model with a
`zmk,matrix-transform` chosen node. To move one onto the current stable stack:

1. In `config/west.yml`, set the `zmk` revision from `main` to `v0.3.0`, and
   the `zmk-keyboards` revision from `main` to the current release tag
   (`v2026.07`). Pinning `zmk-keyboards` to a tag rather than the moving `main`
   branch is what stops a future definitions change from silently breaking your
   keymap again — see [Pinned versions](#pinned-versions) above.
2. In `.github/workflows/build.yml`, change `@main` to `@v0.3.0`.
3. In your `config/imprint.keymap`, replace the chosen node

   ```dts
   chosen { zmk,matrix-transform = &imprint_<your model>; };
   ```

   with

   ```dts
   chosen { zmk,physical-layout = &physical_layout_imprint_<your model>; };
   ```

   Step 3 matters: with a chosen `zmk,matrix-transform`, ZMK ignores the
   physical layouts that the current `zmk-keyboards` shields are built
   around, and the firmware is not compatible with ZMK Studio.

   Pick the physical layout that matches the keys your board **physically
   has** (its rows, and whether it has a full bottom row). This is not always
   the same name as your old `matrix-transform`: some older configs selected a
   larger *superset* transform and left the unpopulated positions as `&trans`.
   For example, a board with three letter rows and no bottom row is
   `letters_only_no_bottom_row`, even if its old config named
   `function_row_full_bottom_row`. When unsure, flash and open
   [ZMK Studio](https://studio.cyboard.digital) — it renders the layout you
   selected, so you can see at a glance whether it matches your keyboard.
4. Re-lay-out your keymap to match the selected layout. **The physical
   layouts do not use the same key order as the old matrix transforms** — in
   particular the 12 thumb-cluster keys are the **last 12 bindings** of every
   current two-arc Imprint layout (two arcs of three per hand), after all the
   grid rows. The easiest way to get this right is to start from the matching
   keymap in `config/default keymaps/imprint_<your layout>/imprint.keymap` (the
   default-keymap directories are `imprint_`-prefixed) and drop your key choices
   into its slots. Each `bindings` block must have exactly as many
   entries as the layout has keys, or keys silently stop responding.

   > **Thumb keys stopped working after migrating?** That is the classic
   > symptom of a keymap whose bindings no longer line up with the selected
   > layout: the grid keys still work (they share positions across layouts) but
   > the thumb bindings land on the wrong — or nonexistent — positions, so the
   > thumb cluster goes dead. Rebuild the layer from the matching default
   > keymap so your thumb bindings occupy the last twelve slots. On a
   > single-arc board only one of the two thumb rows physically exists; use
   > ZMK Studio to confirm which, then bind those keys.
5. (Optional, for ZMK Studio) In `build.yaml`, add the Studio snippet and
   config to the `imprint_left` entry, as in this template's `build.yaml`:

   ```yaml
   - board: assimilator-bt
     shield: imprint_left
     snippet: studio-rpc-usb-uart
     cmake-args: -DCONFIG_ZMK_STUDIO=y
   ```
