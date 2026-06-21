#!/usr/bin/env bash
# Build the mod and package it into a .nrm.
#
# Requires:
#   - LLVM clang + ld.lld with MIPS support (Apple clang will NOT work)
#   - RecompModTool from https://github.com/N64Recomp/N64Recomp (built from source on macOS)
#   - the mm-decomp and Zelda64RecompSyms submodules/symlinks present at the repo root
#
# Override any of these via the environment if your paths differ.
set -euo pipefail
cd "$(dirname "$0")"

CC="${CC:-/opt/homebrew/opt/llvm/bin/clang}"
LD="${LD:-/opt/homebrew/bin/ld.lld}"
RECOMP_MOD_TOOL="${RECOMP_MOD_TOOL:-./tools/RecompModTool}"

echo ">> Building mod ELF"
make CC="$CC" LD="$LD" -j8

echo ">> Packaging .nrm"
"$RECOMP_MOD_TOOL" mod.toml build

echo ">> Done:"
ls -la build/*.nrm
