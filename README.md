# Black - XBE Recomp
An unofficial native PC port of the original Xbox version of Black, with extended graphics options, keyboard and mouse support, and other enhancements.

This port was created using the [Xbox XBE Recomp](https://github.com/TGP482/XboxXBE-Recomp) tool.

This repository is a community-maintained fork of [TGP482/BLACK-XBERecomp](https://github.com/TGP482/BLACK-XBERecomp). It is maintained for personal and community use, with the goal of keeping this fork current with selected upstream changes and community contributions. It is not an official release of the upstream project.

> [!NOTE]
> This project does not include any game assets. You must provide the files from your own legally acquired copy of the game to install or build.

## Installation
- Put your game files in the `assets` folder. This can be an ISO, or the extracted data files, of the USA or Europe release.
- Run `BLACK.exe`.
- Recomp settings can be adjusted in the in-game options menu while the game is running, or through the Settings overlay, which is opened with F10 by default.
- The release ZIP contains an empty `assets` folder as a destination; game files are not included and must come from your own legally acquired Xbox copy.

### Languages and Subtitles
- Open **Language** in the in-game settings and choose English, French, German, Italian, Spanish, Brazilian Portuguese, Bahasa Indonesia, or Russian.
- The game checks the Windows display language on first launch. If it is supported, it selects it; otherwise English remains the fallback.
- Language menus and subtitle packages are downloaded from [BLACK-SUBTITLES](https://github.com/soldieg/BLACK-SUBTITLES). An internet connection is needed to download or update them. Selecting a language downloads that package; **Update Translations** checks the manifest and downloads all available packages.
- Changing the selected language prompts for a restart. Subtitle display can be enabled or disabled, with options for size, bold text, color, shadow, and background.
- Downloaded files are stored in the `language` folder beside `BLACK.exe`. Translation packages are not bundled into the executable or release ZIP.
<img width="1920" height="1080" alt="{5852D359-24A0-48C8-9319-07C3A619E355}" src="https://github.com/user-attachments/assets/e0156eb8-c1b5-49d9-ab93-3abb46bf5f47" />


## Features
### Display
- **Monitor** - Picks the monitor the game shows on, for Fullscreen and Borderless (and where Windowed opens). Lists every connected monitor by its Windows number and desktop resolution.
- **Display Mode** - Switch between Fullscreen, Borderless and Windowed.
- **Resolution** - Lists every mode your monitor reports and defaults to your desktop resolution, replacing the original 640x480 resolution.
- **Render Resolution** - Renders the game at 20% to 200% of the output resolution, allowing supersampling or downscaling independent of the window size.
- **Field of View** - Sets the horizontal field of view during gameplay from 45 to 140 degrees, with 70 being the game's default.
- **Viewmodel Field of View** - Adjusts the first-person weapon and arms field of view from 45 to 140 degrees. The rendering changes also reduce weapon clipping at wider settings.
- **Max Frame Rate** - Caps the frame rate to your monitor's refresh rate or a custom value, or removes the cap entirely, replacing the Xbox's 30 FPS limiter.
- **V-Sync** - Reduces screen tearing.
- **HDR Output** - Enables HDR10 output on displays with Windows HDR turned on, detected automatically. Highlights extend up to the display's peak brightness.
- **FPS Counter** - Displays the frame rate on screen.
- **FMV Scale** - Choose between Fill, which zooms the 4:3 FMVs to fill widescreen displays, and 4:3, which keeps them at their original aspect ratio.

### Graphics
- **Multisample Antialiasing (MSAA)** - Enables 2x, 4x or 8x MSAA to smooth jagged edges while preserving a sharp image.
- **Subpixel Morphological Antialiasing (SMAA)** - Enables SMAA to smooth jagged edges across the whole frame with a softer image. SMAA also smooths alpha-tested edges such as fences and foliage, and can be combined with MSAA.
- **Anisotropic Filtering** - Forces the selected anisotropic filtering level on all textures.
- **Draw Distance** - Increases the level of detail and draw distances past the game's values: 2x, 4x, 8x, 12x or 16x.
- **Ambient Occlusion** - Adds screen space ambient occlusion, creating soft contact shadows where surfaces meet. Choose between SSAO and GTAO.
- **Translucent Surface Shading** - Keeps ambient occlusion and fog from incorrectly darkening or tinting translucent surfaces.
- **Optional HD Texture Packs** - Loads compatible replacements from a `textures` folder beside `BLACK.exe`; absent or invalid replacements fall back to the original game texture.
- **Reload Blur** - Toggles the blur effect while reloading.
- **Light Glow Alignment** - Adjusts the final glow composite so bright effects line up more closely with their light sources.

### Input
- **Mouse and Keyboard** - Full keyboard and mouse support with raw input, adjustable sensitivity, inverted look and rebindable controls.
- **Controller** - Controller with SDL3 support and additional features.
- **Controller Options** - Configure controller aim behavior, sensitivity, and rumble from the in-game options.
- **Aim Assist** - Turn the game's controller aim assistance on or off. It remains disabled while keyboard and mouse are controlling the game.
- **Zoom Sensitivity** - Keeps mouse look sensitivity consistent with the current view FOV while aiming.
- **Menu Animations** - Toggle the game's menu transitions.

### Audio
- **XAudio2 Output Queue** - Uses a deeper audio output queue to help reduce crackling.
- **Audio Output Modes** - Supports stereo, mono, and 5.1 output with game-compatible downmixing and in-game panning.

### Weapons
- **Weapons Page** - Adds a page to the F10 settings panel for selecting any of the game's weapons in a level and filling ammunition. This is an optional testing and gameplay utility.

### General
- **Skip Intro** - Skips the intro logos.
- **Pause On Focus Loss** - Pauses the game and its sound while the window is in the background or minimized.

### Miscellaneous
- **Custom Menu Cursor** - Place a `cursor.png` (or `cursor.dds`) beside the exe to replace the Windows arrow in the game's menus.
- **HD texture pack files** - Put replacement files in `textures/` beside `BLACK.exe`. The game log reports the fingerprint filename for supported source textures that do not have a replacement. Use that exact 16-digit hexadecimal name with a `.dds` extension. Accepted files are BC3/DXT5 DDS with a valid mip chain, or single-level 32-bit BGRA DDS for sharp edges such as font atlases. No texture files are bundled; invalid or missing replacements use the original texture.

### Community Changes in This Fork
- Runtime language selection, manifest-based language downloads and updates, translated game/recomp menus, cutscene subtitles, and subtitle display settings.
- Portuguese subtitle glyph compatibility: missing tilde glyphs in the Portuguese font atlas are mapped to the corresponding supported umlaut glyphs, so `ã/Ã` and `õ/Õ` render instead of missing-character boxes.
- Subtitle positioning and preview fixes: cutscene text is centered to the output, long lines wrap within the screen width, and the preview uses the game's subtitle font and background settings.
- Save preservation: save data is kept in the per-user persistent HDD location, existing profiles are validated before use, and verified profiles in legacy or preserved AppData folders can be recovered without deleting the source.
- A deeper XAudio2 output queue to reduce audio crackling.
- A weapons page in the F10 panel.
- Field-of-view limits and rendering adjustments to reduce first-person weapon clipping.
- A D3D12 light-glow alignment adjustment.
- Optional fingerprint-based HD texture replacement, with validation and original-texture fallback.
- Controller settings, menu animation control, improved audio output handling, and 30 FPS timing fixes for selected effects.
- A controller aim-assist toggle and a 4:3 exception for the EA opening logo film.
- Corrected profile discovery so a missing `Profile` in an otherwise valid HDD is not mistaken for a corrupt save.

See [the upstream and feature audit](docs/UPSTREAM_FEATURE_AUDIT_2026-10-18.md) for the per-commit status and the features that remain candidates for later work.

## System Requirements
### Minimum
- **OS**
  - Windows 10 64-bit
- **CPU** with 2 cores and support for SSE4.2 and POPCNT
  - Intel Nehalem (Intel Core 1st Generation)
  - AMD Bulldozer (AMD FX series)
- **GPU** with support for Direct3D 12
  - NVIDIA GeForce GT 630 (Kepler)
  - AMD Radeon HD 7750
  - Intel HD Graphics 510 (Skylake)
- **Memory**
  - 4 GB
- **Storage**
  - 2 GB for the game files and saves

## Building
### 1. Clone the Repository
Clone this repository using [Git](https://git-scm.com/).
```
git clone https://github.com/soldieg/BLACK-XBERecomp.git
```

### 2. Add the Required Game Files
Place your extracted game data files in the `assets` folder. `default.xbe` from the USA release must be there, with the rest of the game's files beside it.


### 3. Install Dependencies
Install [Visual Studio 2022](https://visualstudio.microsoft.com/downloads/) and select the following **Workloads** and **Individual components**:
- Desktop development with C++
- C++ Clang tools for Windows
- C++ CMake tools for Windows

> [!TIP]
> `build.bat` offers to install the Clang and CMake components for you if they're missing.

### 4. Build the Project
- Run `build.bat`. The first build recompiles the entire game and can take 30 to 90 minutes on weaker systems.
- When it finishes, run `out\build\release\BLACK.exe`.
