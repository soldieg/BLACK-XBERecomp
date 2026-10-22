# BLACK — XBE Recomp (fork comunitário)

Recompilação para PC de BLACK (Xbox original), com recursos comunitários. Este fork não é oficial. O jogo não acompanha o código nem os releases: use arquivos de uma cópia legal própria.

## Instalação — Build V2.0

1. Extraia o pacote do release em uma pasta.
2. Coloque em Assets/ os arquivos extraídos do jogo. O release inclui essa pasta vazia como destino; não inclui ISO, XBE ou dados originais.
3. Opcional: extraia textures.zip ao lado de BLACK.exe, mantendo a pasta textures/. O carregador usa DDS compatível e volta à textura original se uma substituição faltar ou for inválida.
4. Execute BLACK.exe. Não copie HDD, logs ou black.toml de outra instalação: eles podem conter saves e preferências pessoais.

### Idiomas e legendas

O menu Language oferece inglês, francês, alemão, italiano, espanhol, português brasileiro, indonésio e russo. Pacotes e atualizações são obtidos pelo updater do [BLACK-SUBTITLES](https://github.com/soldieg/BLACK-SUBTITLES), não vêm embutidos no executável. Legendas de cutscenes têm opções de tamanho, negrito, cor, sombra e fundo; a correção de fonte portuguesa cobre ã/Ã e õ/Õ.

## Recursos

- Vídeo e gráficos: tela cheia/janela, resolução, escala de renderização, FOV, limite de FPS, V-Sync, HDR, escala de FMV, MSAA, SMAA, filtragem anisotrópica, distância de desenho, SSAO/GTAO e substituição opcional de texturas HD.
- Upscaling: opções DLSS/DLAA e FSR com runtimes no pacote. Ver o modo no menu ou detectar uma DLL não prova que o upscale esteja ativo; isso depende do hardware, driver e validação visual.
- Controles: mouse/teclado configurável, suporte a controle, assistência de mira, sensibilidade de zoom e animações do menu.
- Áudio: modos estéreo/mono/5.1 e fila de saída XAudio2 ampliada.
- Gameplay V2.0: corrida ajustável, recarga acelerada, cozimento de granadas, kit médico automático, inclinação tática configurável e multiplicador de dano aos inimigos.
- F10/Weapons: seleção de armas, reabastecimento de munição, munição infinita, armas prateadas, Modo Deus e reposição de vida/kits.
- HUD V2.0: mira adicional configurável, porcentagem de vida, hit/death marker, aviso direcional de granada, avisos de munição/recarga e marcadores de inimigos.
- Legendas e saves: idiomas selecionáveis, legendas de cutscene centralizadas e recuperação de saves existentes sem descartar a origem.

### Limites conhecidos da Build V2.0

Recolorir sangue não está implementado; cozimento de granadas em controle físico não foi validado. Esta build não inclui mod loader, falas em tempo real, SSR ou sombras dinâmicas. Os recursos novos estão presentes na build, mas seus efeitos de gameplay ainda exigem validação visual; runtimes DLSS/FSR não confirmam upscale efetivo.

## Compilar

1. Clone o repositório e adicione seus arquivos do jogo em assets/.
2. Instale Visual Studio 2022 com Desktop development with C++, Clang tools e CMake tools.
3. Execute build.bat e depois out/build/release/BLACK.exe.

---

# BLACK — XBE Recomp (community fork)

A community PC recompilation of BLACK for the original Xbox. This fork is unofficial. The game is not included in the source or releases; use files from your own legally obtained copy.

## Installation — Build V2.0

1. Extract the release package to a folder.
2. Put the extracted game files in Assets/. The release provides an empty destination folder; it does not include the ISO, XBE, or original game data.
3. Optional: extract textures.zip beside BLACK.exe, keeping the textures/ folder created by the archive. The loader accepts compatible DDS files and falls back to the original texture if a replacement is missing or invalid.
4. Run BLACK.exe. Do not copy another installation's HDD, logs, or black.toml; they may contain personal saves and settings.

### Languages and subtitles

The Language menu offers English, French, German, Italian, Spanish, Brazilian Portuguese, Indonesian, and Russian. Language and translation packages are retrieved by the [BLACK-SUBTITLES](https://github.com/soldieg/BLACK-SUBTITLES) updater and are not bundled with the executable. Cutscene subtitles support size, bold, color, shadow, and background settings; Portuguese font compatibility covers ã/Ã and õ/Õ.

## Features

- Display and graphics: fullscreen/windowed modes, resolution, render scale, FOV, frame cap, V-Sync, HDR, FMV scaling, MSAA, SMAA, anisotropic filtering, draw distance, SSAO/GTAO, and optional HD texture replacements.
- Upscaling: DLSS/DLAA and FSR options with runtimes in the package. A menu selection or detected DLL does not prove upscaling is active; hardware, driver, and visual validation matter.
- Input: configurable mouse/keyboard, controller support, aim assist, zoom sensitivity, and menu animation controls.
- Audio: stereo/mono/5.1 modes and a deeper XAudio2 output queue.
- Build V2.0 gameplay: adjustable sprint, faster reload, grenade cooking, automatic health pack, configurable tactical lean, and enemy damage multiplier.
- F10/Weapons: weapon selection, ammo refill, infinite ammo, silver weapons, God Mode, and health/kit refill.
- Build V2.0 HUD: configurable extra crosshair, health percentage, hit/death marker, directional grenade warning, ammo/reload prompts, and enemy markers.
- Subtitles and saves: selectable languages, centered cutscene subtitles, and recovery of existing saves without discarding their source.

### Known Build V2.0 limits

Blood recoloring is not implemented. Grenade cooking with a physical controller has not been validated. This build does not include a mod loader, real-time gameplay dialogue subtitles, SSR, or dynamic shadows. New gameplay effects still need visual in-game validation; DLSS/FSR runtime files alone do not confirm effective upscaling.

## Build from source

1. Clone this repository and add your game files to assets/.
2. Install Visual Studio 2022 with Desktop development with C++, C++ Clang tools, and CMake tools.
3. Run build.bat, then launch out/build/release/BLACK.exe.
