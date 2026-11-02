# ZMK Configuration Template
Wireless Cyboard keyboard configuration repository template for using ZMK firmware. [Instructions for use are located on our documentation site](https://docs.cyboard.digital/user-manual/quick-start/configure-layout).

## Personal Imprint layout

`config/imprint.keymap` selects the Imprint with a full function row and full
bottom row. It has three layers:

| Studio layer | ZMK layer ID | Layout | RGB color |
| --- | --- | --- | --- |
| Qwerty | 0 | Qwerty | Red |
| Colemak | 1 | Standard Colemak letters | Blue |
| Custom | 2 | Unassigned except for the cycle key | Green |

Tap the innermost key on the lower left thumb arc to cycle through these layers.
This is the former `&mo 2` key, two positions after `LGUI` in the last keymap row.
Its bindings are `&to 1`, `&to 2`, and `&to 0`. Keep these bindings when editing
the layers in Studio, so you can always leave the Custom layer. The other three
former momentary-layer thumb keys are unassigned and can be configured in Studio.

The firmware sets solid RGB colors at 25% brightness. It updates the color on
layer changes, after saved settings load at startup, and when it receives the
right half's battery report. Colors follow the layer IDs even if you rename or
reorder layers in Studio. Color changes require a firmware rebuild; normal key
changes can still be made in Studio. The Custom layer uses `&none`, so its empty
keys do not fall through to Qwerty. The left trackball keeps its scrolling setup.

Flash both halves from the same successful GitHub Actions build. If the keyboard
already has a keymap saved by Studio, save a copy of those edits and reset the
keymap to the firmware defaults in Studio to load these three layers. A normal
reflash preserves the Studio keymap and does not replace it with these defaults.

After flashing, check that each tap cycles red/Qwerty, blue/Colemak,
green/Custom, and back to red/Qwerty. Check that Custom produces no key output
until configured, but its cycle key still works. Restart the keyboard from blue
or green and check that it returns to red/Qwerty. Also restart the right half
while Colemak or Custom is active and check its color after it reconnects.
Unlock Studio with the existing A-position + F-position hold combo to configure
Custom. The combo uses physical positions and also works on the blank layer.

Run the keymap checks with:

```sh
uv run --no-project python -m unittest discover -s tests -v
```

> **Already have a copy of this template from before July 2026?** The board and
> layout definitions were reorganized in July 2026, and older copies track the
> moving `main` branch — so a rebuild can silently break your keymap (most often
> a dead thumb cluster). Follow [Updating a config repo created before July
> 2026](#updating-a-config-repo-created-before-july-2026) to move onto the pinned
> stable stack; after that, [Pinned versions](#pinned-versions) keeps it from
> happening again.

## Pinned versions
`config/west.yml` pins two projects to fixed releases so your firmware is reproducible and cannot change under you:

- **`zmk`** → the latest **stable** ZMK release (currently `v0.3.0`), matching the firmware stack behind [studio.cyboard.digital](https://studio.cyboard.digital).
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
