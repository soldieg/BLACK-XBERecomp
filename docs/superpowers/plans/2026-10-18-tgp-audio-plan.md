# TGP Audio Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Port the TGP audio queue and console-matching output routing while preserving safe fallback behavior.

**Architecture:** Adapt XAudio2 queueing and DirectSound speaker routing in the existing host audio path. Keep queue depth bounded and use the current device configuration to select mono, stereo, or 5.1 behavior.

**Tech Stack:** C++20, XAudio2, DirectSound compatibility layer.

**Spec:** `docs/superpowers/specs/2026-10-18-tgp-mahdi-integration-design.md`

## Global Constraints

- Preserve existing language/subtitle, save, FOV, and gameplay options.
- Keep maximum queued audio at or below 100 ms.
- Do not replace the current audio pipeline wholesale.

## Review Focus

- Queue growth must be bounded and starvation detection must not spin or submit duplicate buffers.
- Stereo and mono downmix must not clip or reverse channels.
- 5.1 speaker order and in-game panning must match the intended Xbox mapping.
- Device changes and unsupported channel configurations must fall back safely.
- The normal stereo path must remain correct when surround is unavailable.

---

### Task 1: Adaptive output queue and downmix

**Files:**
- Modify: `src/audio/xaudio2.cpp`

- [ ] Reconcile `5e42e46` and `9ea0f6e` against the fork's current fixed queue.
- [ ] Start at the upstream lower-latency depth and grow only after starvation, capped at the existing 100 ms maximum.
- [ ] Port the console-matched mono/stereo downmix coefficients and retain bounded buffer storage.

### Task 2: Surround routing and panning

**Files:**
- Modify: `src/sound/device.cpp`
- Modify: `src/sound/mixer.cpp`
- Modify: `config/game.toml` only if the existing audio-output setting needs a matching serialized field.

- [ ] Port the channel layout and panning changes from `8252ed7` without changing the device-selection interface.
- [ ] Reconcile center/rear folding and channel ordering with the current mixer's mappings.
- [ ] Review mono, stereo, and 5.1 code paths and their fallback conditions before the bundle build.

### Task 3: Commit audio changes

- [ ] Review `git diff --check` and confirm the diff is limited to the audio files above.
- [ ] Commit this subsystem separately from rendering/timing, options/input, and texture loading.
