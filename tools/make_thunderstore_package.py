#!/usr/bin/env python3
"""Assemble the Thunderstore package from an already-built .nrm.

Reads the version/description from mod.toml so there's a single source of truth,
generates thunderstore_package/manifest.json, copies the .nrm in, and zips the
package up as <NAME>.thunderstore.zip at the repo root.

Run AFTER the mod is built (build/<mod_filename>.nrm must exist).
"""
import os
import re
import json
import shutil
import zipfile

# --- Edit these if you want a different Thunderstore identity ---------------
NAME = "Warp_From_Map"  # Thunderstore package name (no spaces; [A-Za-z0-9_])
WEBSITE_URL = "https://github.com/jackowfish/mm-warp-from-map"
DEPENDENCIES = []
# ---------------------------------------------------------------------------

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PKG_DIR = os.path.join(ROOT, "thunderstore_package")


def toml_get(text, key):
    m = re.search(r'^\s*%s\s*=\s*"([^"]*)"' % re.escape(key), text, re.MULTILINE)
    return m.group(1) if m else None


def main():
    with open(os.path.join(ROOT, "mod.toml"), encoding="utf-8") as f:
        mod_toml = f.read()

    version = toml_get(mod_toml, "version")
    mod_filename = toml_get(mod_toml, "mod_filename")
    description = toml_get(mod_toml, "short_description") or ""
    if not version or not mod_filename:
        raise SystemExit("mod.toml is missing version or mod_filename")

    nrm = os.path.join(ROOT, "build", mod_filename + ".nrm")
    if not os.path.exists(nrm):
        raise SystemExit("Build the mod first; not found: " + nrm)

    os.makedirs(PKG_DIR, exist_ok=True)

    manifest = {
        "name": NAME,
        "version_number": version,
        "website_url": WEBSITE_URL,
        "description": description[:250],
        "dependencies": DEPENDENCIES,
    }
    with open(os.path.join(PKG_DIR, "manifest.json"), "w", encoding="utf-8") as f:
        json.dump(manifest, f, indent=4)

    shutil.copy2(nrm, os.path.join(PKG_DIR, os.path.basename(nrm)))

    # Thunderstore requires manifest.json, icon.png and README.md at the zip root.
    required = ["manifest.json", "icon.png", "README.md"]
    for r in required:
        if not os.path.exists(os.path.join(PKG_DIR, r)):
            raise SystemExit("thunderstore_package is missing required file: " + r)

    zip_path = os.path.join(ROOT, NAME + ".thunderstore.zip")
    with zipfile.ZipFile(zip_path, "w", zipfile.ZIP_DEFLATED) as z:
        for entry in sorted(os.listdir(PKG_DIR)):
            z.write(os.path.join(PKG_DIR, entry), entry)

    print("wrote", zip_path)
    print("  version", version)


if __name__ == "__main__":
    main()
