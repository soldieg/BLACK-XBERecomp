# TGP Options and Input Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Integrate TGP controller/pause options and menu animation settings while retaining the fork's language, subtitle, keyboard, and FOV features.

**Architecture:** Extend the existing options data and menu pages in place. Reconcile stock game pages with the fork's custom language and subtitle pages rather than replacing them with upstream versions.

**Tech Stack:** C++20, CMake, existing game Flash menu integration.

**Spec:** `docs/superpowers/specs/2026-10-18-tgp-mahdi-integration-design.md`

## Global Constraints

- Keep language and subtitle rows and their localized strings intact.
- Keep the existing 16x draw distance and mouse-driven weapon sway; do not duplicate either feature.
- Set camera and viewmodel FOV maxima to the user's requested 140 degrees, including settings load/save and UI ranges.
- Do not add translations or game assets.

## Review Focus

- Controller setup must not shadow or remove the existing mouse/keyboard page.
- Pause-menu options must show valid stock pages and preserve custom language/subtitle navigation.
- New settings must persist with safe defaults and load older `black.toml` files.
- The 140-degree camera/viewmodel bounds must match in UI and config loading.
- Menu animation disable must still process input, sound, and completion callbacks.

---

### Task 1: Reconcile menu pages and categories

**Files:**
- Modify: `src/game/options_menu.cpp`
- Modify: `src/common/translations.cpp`

- [ ] Compare `a7af1df`, `e8516e6`, and `1e9fcf8` against the fork's language/subtitle menu implementation.
- [ ] Add the controller options entry, pause-menu access to stock option pages, and recomp-settings categories without changing the fork's existing custom entries.
- [ ] Retain the current menu input/focus flow and language-specific labels; inspect the final entry counts and page transitions.

### Task 2: Persist options and connect input behavior

**Files:**
- Modify: `src/common/options.h`
- Modify: `src/common/game_info.cpp`
- Modify: `src/common/menu.cpp`
- Modify: `src/game/mouse.cpp`
- Modify: `src/game/options_menu.cpp`
- Modify: `config/game.toml` if required by the existing setting serialization pattern.

- [ ] Add the optional menu-animation setting with a default that preserves current animation behavior.
- [ ] Port the mouse-zoom sensitivity relationship from `1ad6c56`, using the currently selected camera FOV.
- [ ] Keep both camera and viewmodel FOV at 45–140 degrees across UI, load, and save paths.
- [ ] Keep the fork's mouse sway implementation and integrate controller options without replacing it.

### Task 3: Menu animation runtime hook

**Files:**
- Create: `src/game/menu_animations.cpp` by adapting `bdff724`.
- Modify: `CMakeLists.txt` only if the existing source glob does not include the new file.
- Modify: `src/common/game_info.cpp`, `src/common/menu.cpp`, and `src/common/options.h` for registration and persistence.

- [ ] Adapt the animation toggle to the fork's game-option lifecycle and hook registration.
- [ ] Preserve completion callbacks, input processing, and sound events while fast-forwarding animation frames.
- [ ] Review hook addresses and limits against the exact XBE mapping already used by this fork.

### Task 4: Commit options and input changes

- [ ] Review `git diff --check` and confirm the diff is limited to the files above.
- [ ] Commit this subsystem separately from rendering/timing, audio, and texture loading.
