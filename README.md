# Warp From Map

A mod for [Majora's Mask: Recompiled](https://github.com/Mr-Wiseguy/Zelda64Recomp) that lets you
pick a Song of Soaring destination straight from the world map in the pause menu, instead of
playing the song.

## What it does

- Open the pause menu and go to the **map page**.
- Press **C-Down** (configurable) to switch the map into **warp mode**. A small `Ⓒ▼ Warp Map`
  prompt on the map page reminds you of this.
- Only owl statues you've **already activated** are selectable.
- Move the cursor with the **control stick**, press **A** to warp.
- Press **C-Down or B** to switch back to the normal map. **Start** still closes the menu.

The original map is left untouched - warp mode is an opt-in overlay you toggle on and off.

## How it works

The game already contains a complete owl-warp selector: the `PAUSE_STATE_OWL_WARP_*` states that
the Song of Soaring drops you into. It draws the map, a cursor over only the unlocked owl statues,
the location-name panel, a "Warp to ___? Yes/No" confirmation, and on close it spawns the actor
that performs the warp.

This mod doesn't reimplement any of that. It hooks `KaleidoScope_Update` and, when you're on the
overworld map page with at least one owl unlocked, flips the menu into that native selector (and
back out again). All it has to reproduce is the small bit of setup that
`func_800F4A10`'s owl-warp branch does - populating the unlocked-owl list and the cursor - because
the loaded map segments and world-map view are already correct while the map page is open.

## Config options (in the in-game mod menu)

- **Warp Map Button** - which C button toggles warp mode (`C-Down` default).
- **On-screen Prompt** - show or hide the `Ⓒ▼ Warp Map` hint.

## Building

You need an LLVM `clang` + `ld.lld` with MIPS support (Apple clang will not work), plus
`RecompModTool` from [N64Recomp](https://github.com/N64Recomp/N64Recomp) (build it from source on
macOS). On macOS:

```sh
brew install llvm lld
# build RecompModTool from a checkout of N64Recomp, then point build.sh at it
```

The `mm-decomp` and `Zelda64RecompSyms` submodules must be present at the repo root. You can either
add them as git submodules:

```sh
git submodule add https://github.com/zeldaret/mm mm-decomp
git submodule add https://github.com/Zelda64Recomp/Zelda64RecompSyms Zelda64RecompSyms
```

...or symlink them to an existing Zelda64Recomp checkout (faster if you already have one):

```sh
ln -s /path/to/Zelda64Recomp/lib/mm-decomp mm-decomp
ln -s /path/to/Zelda64Recomp/Zelda64RecompSyms Zelda64RecompSyms
```

Then:

```sh
./build.sh   # override CC / LD / RECOMP_MOD_TOOL via env if your paths differ
```

This produces `build/jackdecker_warp_from_map.nrm`.

## Installing

Drag `build/jackdecker_warp_from_map.nrm` onto the game window before starting a save, or use the
**Install Mods** button in the mod menu. Enable it in the mod menu and start playing.

## Regenerating the prompt texture

The on-screen prompt is a generated IA8 texture baked into `src/warp_prompt_tex.h`:

```sh
python3 tools/gen_prompt_tex.py   # needs Pillow
```
