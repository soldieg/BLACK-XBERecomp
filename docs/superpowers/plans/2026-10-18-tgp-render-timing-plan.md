# TGP Rendering and Timing Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Port the TGP ambient-occlusion and render-target corrections plus 30 FPS timing fixes without regressing the fork's FOV or graphics options.

**Architecture:** Selectively reconcile the upstream changes against the fork at `d84eadc`; keep existing option storage and rendering flow. Do not cherry-pick upstream history wholesale.

**Tech Stack:** C++20, CMake, Direct3D 12/8 runtime.

**Spec:** `docs/superpowers/specs/2026-10-18-tgp-mahdi-integration-design.md`

## Global Constraints

- Preserve translation, subtitle, save, 16x draw distance, and weapon-sway behavior.
- Preserve the requested 140-degree camera and viewmodel FOV maxima.
- Do not add game assets or translation packages.
- Do not modify the dirty `release-main` checkout.

## Review Focus

- Translucent surfaces and fog must not receive the wrong AO contribution; compare the complete TGP shader/state diff.
- Render-target reads must use the console pixel offset without shifting UI or ordinary texture sampling.
- 30 FPS timing corrections must not change 60 FPS behavior.
- 140-degree FOV must remain clamped and usable after integrating the zoom change.
- Preserve original rendering when AO is off and when no texture pack is installed.

---

### Task 1: Ambient occlusion and render-target alignment

**Files:**
- Modify: `src/d3d/d3d12.cpp`
- Modify: `src/d3d/d3d8.cpp`
- Modify: `src/d3d/occlusion.h`

- [ ] Compare commits `0a722e0` and `a91ddc8` against `origin/main`; identify hunks touching AO masks/fog and render-target coordinate constants.
- [ ] Port only the AO state/blend/fog correction and the console pixel-offset behavior; retain the fork's unrelated D3D12 changes.
- [ ] Review the final diff to confirm ordinary texture coordinates and the fork's 140-degree view scaling are unchanged.

### Task 2: 30 FPS gameplay timing

**Files:**
- Modify: `src/game/hooks.cpp`
- Modify: `config/game.toml` only if the upstream change requires a matching setting or hook entry.

- [ ] Port the walk-to-stop interpolation adjustment from `94f382e` and the glass-shard sound countdown adjustment from `46a3a2b`.
- [ ] Keep both adjustments derived from the existing frame/time data; do not introduce a second timing source.
- [ ] Review call sites and hook mappings against the generated game code before including them in the build.

### Task 3: Commit rendering and timing changes

- [ ] Review `git diff --check` and confirm the diff is limited to the files above.
- [ ] Commit this subsystem separately from options/input, audio, and texture loading.
