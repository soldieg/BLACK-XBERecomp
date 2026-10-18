# TGP and Mahdi Feature Integration Design

## Goal

Integrate the missing, compatible changes from the current TGP BLACK recomp branch and Mahdi's HD texture replacement work into the user's fork, build a new `BLACK.exe`, and submit the changes for review without replacing the fork's existing language, subtitle, save, or weapon-sway behavior.

## Verified baseline

- Fork base: `soldieg/BLACK-XBERecomp` `main` at `d84eadc3aaa8d39ff14dad5dceefab1b2f2f06f3`.
- TGP source: `TGP482/BLACK-XBERecomp` `main` at `a91ddc89de9702c0b58d9030e1a6ce62f13eebdc`.
- Mahdi source: `mahdip72/BLACK-XBERecomp` `main` at `44af0f788c6b93e758a34701771773d884a70423`.
- The fork already contains 12x/16x draw distance and mouse-driven weapon sway; preserve those implementations and do not layer duplicate patches over them.
- The user previously requested a 140-degree maximum for both camera and viewmodel FOV. The public fork currently clamps both to 100 degrees, so this integration must preserve the requested 140-degree cap rather than silently inherit that lower limit.
- The working copy `release-main` contains uncommitted user changes and a texture loader restricted to 11 known menu texture fingerprints. It is not the publication source and must remain untouched.
- The fork's existing translation and subtitle code remains the owner of locale selection and subtitle rendering. No translation packages or game texture assets will be included in this code PR.

## Scope: TGP changes to reconcile

Port or selectively adapt these changes from TGP after comparing their complete diffs against the fork:

1. Ambient occlusion blending/fog correction.
2. Controller options page and access to stock options from the pause menu, reconciled with the fork's custom language/subtitle screens and options panel.
3. Mouse zoom sensitivity tied to view FOV; keep the requested 140-degree camera/viewmodel FOV maximum.
4. Walk-to-stop transition timing corrected for 30 FPS.
5. Optional menu-animation setting and recomp-settings categories.
6. Adaptive XAudio2 queue depth that starts at lower latency and grows on starvation, bounded by the existing 100 ms ceiling.
7. Console-matched stereo/mono downmix and surround speaker/panning routing, including the required audio-flags query.
8. Glass-shard sound countdown corrected for 30 FPS.
9. D3D12 render-target pixel-offset alignment.

The existing draw-distance and weapon-sway changes are retained. Any TGP diff that conflicts with the user's subtitle, translation, save, or existing FOV behavior must be reconciled rather than copied wholesale.

## Scope: HD texture replacement

Implement Mahdi's runtime replacement concept in the D3D12 texture creation path. A replacement is an optional file beside the executable under `textures/`, identified by a stable fingerprint of the decoded original texture and validated before use. Preserve the original texture data and fall back to it on absent, malformed, unsupported, or mismatched replacement files. Keep replacement files out of the PR.

The local draft's 11-entry menu-only fingerprint allowlist is not sufficient for generic HD textures or translated font atlases. The implementation must derive keys from the actual supported decoded textures and document the exact import/key workflow. Font assets are usable only when valid replacement DDS files with matching keys exist; source PNG/`.bin` assets alone do not prove runtime replacement works.

## Design choices

- Use one feature branch and one PR against the user's fork's current `main`, with separate commits for rendering/timing, options/input, audio, and HD texture loading. This keeps the requested bundle reviewable while making each subsystem easy to identify or revert.
- Preserve the existing fork's option model and settings serialization. New options must have safe defaults and not change existing user settings.
- Keep the HD pack optional and fail-safe: no pack means original rendering; an invalid pack entry is ignored and logged.
- Extend existing CI where appropriate, but do not copy unrelated CI or project changes wholesale from upstream.
- Build a new `BLACK.exe` from the integrated branch. Do not publish a release or merge the PR without a separate request.

## Acceptance criteria

- Existing locale/subtitle selection, save preservation, 16x draw distance, and weapon sway remain present.
- Each newly ported setting has a documented default and works with the fork's existing options UI.
- Audio paths retain bounded buffering and valid output behavior for mono, stereo, and 5.1 configurations.
- Texture replacements are strictly validated and safely fall back to original game textures; missing files do not block startup.
- The project compiles and produces a new `BLACK.exe` from the branch.
- The branch is pushed to the user's fork and a PR is opened against its `main`; no game assets or translations are committed.

## Risks and limits

- Audio routing, D3D12 sampling, and engine timing can produce runtime regressions that compilation alone cannot detect. The PR must distinguish compile evidence from in-game confirmation.
- A generic loader enables compatible replacements but does not create high-quality font art or an atlas mapping. It does not by itself establish that the user's Big/Small font replacements render correctly.
- The upstream feature commits were authored against TGP's current source. They must be adapted to the fork's later translation/save changes; cherry-picking the complete upstream history is out of scope.
