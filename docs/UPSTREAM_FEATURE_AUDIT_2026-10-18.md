# Upstream and feature audit — 2026-10-18

This record compares the fork at `origin/main` (`2b4605b`, after PR #6) with TGP482's `main` (`0d283f0`) and reviews the requested features found in PauloVx's `main` (`60c0faa`). The repositories are not interchangeable: PauloVx's code has a separate launcher and renderer history, and its Git history has no common ancestor with this fork. The work here ports only small, isolated game changes; it does not replace this fork's core.

## TGP482 commits

| Commit | Finding in this fork | Action |
|---|---|---|
| `0a722e0` Ambient occlusion and fog on translucent surfaces | Equivalent translucent/fog handling already exists in the D3D12 renderer. | Already covered; no duplicate patch. |
| `a327085` Draw distance 12x/16x | The in-game setting already offers 2x through 16x. | Already covered. |
| `a7af1df` Controller options page | The in-game options already expose hold-to-aim, sensitivity and rumble. | Already covered. |
| `e8516e6` All option pages in pause menu | The fork already injects its recomp, keyboard/mouse, controller and weapon pages into the game's options flow. | Already covered by the fork's implementation. |
| `a7aa21a` Mouse-driven weapon sway | The fork already feeds mouse look into the game's weapon-sway values. | Already covered with a separate implementation. |
| `1ad6c56` Zoom-relative mouse sensitivity | `MouseLook` already scales sensitivity from the active camera FOV. | Already covered. |
| `94f382e` Walk-to-stop animation at 30 FPS | The fork already has frame-rate adjusted walk easing. | Already covered. |
| `bdff724` Menu animation option | The menu transition option already exists. | Already covered. |
| `1e9fcf8` Recomp settings categories | The options page already groups settings into categories. | Already covered. |
| `5e42e46` Grow audio queue after starvation | The output queue already grows to avoid repeated underruns. | Already covered. |
| `9ea0f6e` Stereo and mono downmix | The fork already implements the console-compatible downmix. | Already covered. |
| `8252ed7` 5.1 and in-game panning | The fork already maps the 5.1 channels and game panning. | Already covered. |
| `46a3a2b` Glass shard sound countdown at 30 FPS | The fork already applies the frame-rate timing correction. | Already covered. |
| `a91ddc8` Render-target pixel offset | The fork already adjusts render-target reads for the console pixel offset. | Already covered. |
| `0834d66`, `aca1d22` Viewmodel / near-plane fixes | The fork already has a separate near-plane and viewmodel depth-clamp path. | Already covered; preserve the fork's implementation. |
| `0d3ef51` Watermark V2 | The build still showed V1. | Updated to V2. |
| `ab14196` Keep EA logo film at 4:3 | The generic FMV scaling also affected the opening EA logo. | Added a 4:3 exception for the logo film. |
| `4537f00` Gyro aiming | No sensor input existed in this fork. The upstream code adds SDL gamepad sensor event handling, drift estimation and a new input path. | Deferred. This needs controller hardware/runtime validation and adds a sensor lifecycle/event-watch path; it should not ship based on a compile-only check. |
| `0d283f0` Controller aim-assist toggle | Aim assist was always enabled for controller input. | Added a persisted in-game toggle; default stays enabled, and keyboard/mouse still do not receive aim assist. |

## PauloVx feature candidates

The commit inventory includes the following requested work. These commits are not merged into this fork because the project histories and application layers differ.

| Feature | Evidence / current status | Feasibility and next step |
|---|---|---|
| Photo mode | PauloVx `3039e1a` adds a free camera and PNG capture; `docs/photo-mode.md` records constraints. No photo-mode implementation was found in this fork. | Feasible as a separate, opt-in game mode, but requires careful camera, input and frame-capture integration. Port after a small runtime prototype and a review of pausing/game state. |
| Screen-space reflections (SSR) | PauloVx `2138e1a` replaces environment-map reflections with screen-space reflections. This fork has no SSR setting or SSR pass. | High renderer effort: needs depth/color inputs, shader and resource-lifetime changes, plus performance and artifact evaluation on D3D12. Port the algorithm, not the renderer wholesale. |
| Ray-traced bounce light | PauloVx `f36223c` adds experimental ray-traced light on walls/floors. No equivalent path exists here. | Very high risk and hardware dependent; inspect required DXR support, fallback behavior and GPU cost before deciding whether to port. Do not enable by default. |
| Persistent checkpoints | PauloVx `b970c8d` stores checkpoint state between sessions. This fork currently protects and recovers the game's `Profile`/HDD files but does not add checkpoint serialization. | Feasible only after identifying the game's checkpoint state and proving a safe save/load boundary. Keep experimental checkpoint data separate from the original profile until round-trip tests pass. |
| Co-op / split-screen | PauloVx's `d96bc62` through `d28ec14` form a multi-commit experimental two-player implementation; the history explicitly includes a plan and experimental split-screen choice. No co-op path exists here. | Very high-risk game-core work: player ownership, AI targets, weapons, HUD/effects, camera views and rendering all change. Not a safe feature-only cherry-pick; would need a dedicated branch and staged runtime milestones. |
| Save editor | PauloVx `4209fe6` adds it to the standalone C++ launcher. This fork has no launcher UI or in-game save editor. | A standalone tool is possible, but requires a documented and validated `Profile` format, backup-first writes and compatibility checks. The current in-game save protection is separate and does not edit progress. |
| Disc/ISO import | PauloVx `6b39156` adds an import flow with progress to its launcher. This fork already accepts an ISO in `assets/` or extracted game files, but has no guided copy/import UI. | Core ISO support is already present. A progress/copy wizard belongs in a launcher; if built later, retain source files and verify copied content before changing the active path. |
| Diagnostics / problem report | PauloVx `2807591` adds launcher settings and problem reports. This fork logs diagnostics but has no user-facing report composer. | Moderate as a separate utility: gather logs, build/version and system summary, preview the report, and require user action before sharing. Do not collect game saves or personal paths by default. |
| Native controls pages | PauloVx `8bf8699` adds keyboard/mouse/controller pages. This fork already has native in-game settings and key rebinding through F10 and the game's Options page. | Already covered in the intended form; compare individual key-binding edge cases only if a report identifies one. |
| HD texture catalog | PauloVx `48f9561` maps disc textures to pack files; this fork already loads validated, fingerprint-named DDS files from `textures/` with fallback. | The formats and lookup rules differ. Compare catalog coverage and hashes before adopting mappings; do not copy their pack assets. |
| Launcher | PauloVx has a standalone launcher with install, settings, save and report flows. This fork currently opens the game and exposes settings in-game; it does not contain that launcher. | Separate product-sized task, not a drop-in feature. Keep it out of this game-code PR. |

## Save behavior in this fork

The current local save changes keep the writable HDD in the per-user persistent location, validate candidate `Profile` files before importing them, and search preserved AppData backups without deleting their source. A normal HDD with no profile is no longer automatically treated as a corrupt profile. If profile reading fails, the code keeps the original and reports the problem rather than silently replacing it. This is not a save editor and does not reconstruct progress from a corrupt profile.

## Subtitle and Portuguese font fixes

Subtitle drawing uses the game's subtitle font path, centers cutscene captions against the output viewport, wraps long lines to the available width, and places a short preview below the `PREVIEW` row. For the Portuguese font atlas, the missing `ã/Ã` and `õ/Õ` glyphs map to their supported `ä/Ä` and `ö/Ö` glyph slots. This is a compatibility mapping, not a claim that the font atlas itself gained newly drawn glyphs.

## Intentionally not part of this release

No translation packages, game files, ISO, texture pack, or PauloVx launcher assets are included. Gyro aiming is deferred pending sensor hardware validation. Photo mode, SSR, ray tracing, checkpoints, split-screen, the save editor, and a report composer remain documented candidates rather than implemented features.
