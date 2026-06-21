# Warp From Map

A mod for [Majora's Mask: Recompiled](https://github.com/Mr-Wiseguy/Zelda64Recomp) that lets you
pick a Song of Soaring destination from the pause-menu map instead of playing the song.

Open the pause menu, go to the map page, and press **C-Down** to switch into warp mode. Only owl
statues you've activated are selectable; move with the stick, **A** to warp, **C-Down/B** to switch
back, **Start** to close. Both the button and the on-screen prompt are configurable in the mod menu.

It works by flipping the map page into the game's own owl-warp selector (the one the Song of Soaring
uses), so the map, cursor, confirmation, and warp are all native.

## Build

Needs LLVM `clang` + `ld.lld` (Apple clang won't work) and
[`RecompModTool`](https://github.com/N64Recomp/N64Recomp). Symlink or submodule `mm-decomp`
([zeldaret/mm](https://github.com/zeldaret/mm)) and `Zelda64RecompSyms`
([Zelda64Recomp/Zelda64RecompSyms](https://github.com/Zelda64Recomp/Zelda64RecompSyms)) at the repo
root, then run `./build.sh` to produce `build/jackdecker_warp_from_map.nrm`. Drag that onto the game
window to install.
