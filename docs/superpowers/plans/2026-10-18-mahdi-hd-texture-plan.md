# Mahdi HD Texture Replacement Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Add optional external DDS texture replacements to the fork's D3D12 texture path, with validation and fallback to the original image.

**Architecture:** Adapt Mahdi's texture-pack reader to fingerprint the decoded guest texture, locate a matching DDS under `textures/` beside the executable, validate its header, dimensions, mip count, format and payload length, then create the host texture from it. Any miss or invalid file uses the original decoded texture.

**Tech Stack:** C++20, Direct3D 12, DDS/BC3 and BGRA texture formats.

**Spec:** `docs/superpowers/specs/2026-10-18-tgp-mahdi-integration-design.md`

## Global Constraints

- Do not overwrite guest memory or game files.
- Missing or malformed replacement files must never block game startup.
- Do not commit assets, translations, DDS packs, or the user's game files.
- The local 11-entry menu-only allowlist is not generic HD texture support and must not be treated as such.

## Review Focus

- Fingerprints must include all source data that distinguishes the texture, including palette data for palettized formats.
- DDS sizes and mip chains must be validated before allocation to avoid overflow or oversized allocations.
- Unsupported formats, cubemaps, depth textures, and malformed headers must cleanly fall back.
- An absent texture directory must leave the original renderer path unchanged.
- Font atlases require generated, correctly keyed DDS assets; loader code alone does not prove font upscaling works.

---

### Task 1: Port and generalize the runtime loader

**Files:**
- Create: `src/d3d/texture_pack.cpp`
- Create: `src/d3d/texture_pack.h`
- Modify: `src/d3d/d3d12.cpp`
- Modify: `CMakeLists.txt` only if `src/*/*.cpp` is not automatically included.

- [ ] Compare Mahdi's `src/d3d/texture_pack.cpp`, header, and D3D12 integration against the exact fork renderer.
- [ ] Use a stable fingerprint over format, dimensions, decoded pixels, and palette where applicable; do not hard-code only the local 11 menu pages.
- [ ] Validate DDS signature, supported formats, dimensions, mip limits, arithmetic bounds, and exact payload length before allocation.
- [ ] Log successful replacement matches and safe fallback reasons without flooding per-frame logs.
- [ ] Keep original texture creation as the fallback for every miss or validation failure.

### Task 2: Document pack workflow

**Files:**
- Create or update: `docs/hd-textures.md`
- Create or update: `tools/import_hd_textures.py` only after verifying how the fingerprint is reproduced from supported source inputs.
- Modify: `README.md` with the optional `textures/` directory behavior.

- [ ] Document the filename key, supported DDS formats, directory, and the fact that no replacement assets ship in this PR.
- [ ] Include font-atlas requirements and avoid claiming that the loader itself improves image quality.
- [ ] Keep the current subtitle CI workflow and do not add or run tests in this change without a separate request.

### Task 3: Build and publish the combined branch

- [ ] Confirm the build input directory `F:\REBLACK\Black - XBE Recomp\BLACK v 3\assets` contains `default.xbe` and required game data; expose it to this checkout through an ignored local path without copying assets into Git.
- [ ] Run `build.bat` from the feature-branch root and confirm the output `out\build\release\BLACK.exe` exists.
- [ ] Record the output SHA-256 and report that compilation does not prove in-game rendering or audio behavior.
- [ ] Commit the texture-loader and documentation changes separately from the TGP subsystem commits.
- [ ] Push `codex/tgp-mahdi-integration` to `soldieg/BLACK-XBERecomp` and open one PR against its current `main`; do not merge or create a release.
