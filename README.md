<div align="center">

<img src="docs/icon/icon_1024.png" alt="ZA Recomp" width="128">

# ZA Recomp · Intel iGPU Edition

**Pokémon Legends: Z-A as a native PC game, tuned for laptops with Intel integrated graphics.**

A community fork of [ZA Recomp](https://github.com/Hexveil404/PokeZ-ARecomp) v0.1.23 · current version **v0.1.23a**

[**Download**](https://github.com/SaiSarvajeet27/PokeZ-ARecomp_Intel_iGPU_Optimized/releases/latest) ·
[What's different](#whats-different) ·
[Install](#install) ·
[Controls](#controls) ·
[Report a problem](https://github.com/SaiSarvajeet27/PokeZ-ARecomp_Intel_iGPU_Optimized/issues)

</div>

---

ZA Recomp turns the game's Nintendo Switch code into a native Windows program: the ARM64 code is translated ahead of
time into C and compiled for your PC, and the Switch's system and GPU are rebuilt on top of Vulkan. No emulator runs
while you play. This edition takes it further on the hardware most laptops have - Intel integrated graphics - without
lowering a single quality setting, and makes it play like a PC game.

> [!IMPORTANT]
> **You need your own copy of the game.** Nothing here contains game files, game code, assets or fonts: you set the
> game up from your own dump on the start screen, and its code is compiled on your PC.

## Contents

- [What's different](#whats-different)
- [Install](#install) ([upgrading an existing install](#upgrading-an-existing-za-recomp-install), [DLC and Switch 2 Edition](#dlc-switch-2-edition-and-nup))
- [Requirements](#requirements)
- [Controls](#controls)
- [Getting the best performance](#getting-the-best-performance)
- [Saves, mods and multiplayer](#saves-mods-and-multiplayer)
- [Troubleshooting](#troubleshooting)
- [Building from source](#building-from-source)
- [Credits](#credits) and [Legal](#legal)

## What's different

### Locked 30 fps on Intel integrated graphics, at native 1080p

Measured on an Intel Core i3-1215U with Intel UHD Graphics (driver 32.0.101.7092), native 1920x1080, the same save and
the same moment in the game each time:

| Scene | ZA Recomp v0.1.23 | **This edition** |
|---|---|---|
| Lumiose City, crowded courtyard | 14.1 fps | **30 fps locked** (GPU 30.5 ms of a 33.3 ms frame) |
| Hotel lobby | 25.7 fps (with upstream's binary patch) | **30 fps locked** (GPU 26 ms) |

Nothing was turned down: every change was checked with screenshots of the same frames with it on and off. The speed
comes from the renderer and the shader translator doing the same work in less time:

- **Render passes Intel's driver runs faster** - the game splits a frame into ~105 passes.
- **Depth-pyramid copies as one draw each** instead of 5 transfers and 4 full GPU barriers, which cost ~5.5 ms a frame.
- **Shaders with loops translated as plain code** instead of a state machine that shuffled 32 variables on every
  branch: ambient occlusion 2.37 → 1.63 ms, GPU culling and animation 1.12 → 0.34 ms.
- **Branch-free texture reads** in translated shaders, so Intel's compiler can issue a shader's reads together.
- **The Switch's NaN-safe min / max / clamp as single instructions** (about 150 fewer in the lighting shader).
- **Lighter bounds checks** on Intel, and **depth snapshots** that copy only what is read.

Every change has its own off switch: see [docs/INTEL.md](docs/INTEL.md) for the details, measurements and switches.

### Plays like a PC game

One layout for mouse and keyboard, set on a fresh install (an existing install switches with one click: **F1 >
Controls > Keys: PC layout**):

- **WASD** to move, the **mouse** turns the camera
- **Left click** throws a Poké Ball, **right click** locks on
- **F** talks, **Shift** dashes, **Space** rolls, **Tab** opens the menu, **M** the map
- **Mouse wheel** or **Q / E** to pick your Pokémon
- **Enter** and **Backspace** confirm and go back in menus

The game's own button icons show your keys - and **LMB / RMB** for the mouse buttons. Full table: [Controls](#controls).

### Its own updates

The start screen checks **this** repository for new versions, so an upstream release never replaces this edition with
a build that doesn't have these changes. Versions are named after the upstream version they're based on plus a letter:
v0.1.23**a**, v0.1.23**b**...

### Tested

- 20,000 randomly generated shaders translate exactly as before (no new failures; every translation compiles and
  validates).
- Upstream's settings and controller tests pass, plus a new test for the control bindings
  ([tools/test_key2.c](tools/test_key2.c)).
- Played and scripted runs on the hardware above: menus, map, battles, switching Pokémon, dashing, rolling, crouching.

## Install

1. **Download** `ZA-Recomp-windows.zip` from the
   [latest release](https://github.com/SaiSarvajeet27/PokeZ-ARecomp_Intel_iGPU_Optimized/releases/latest) and extract it
   to a folder of your choice, for example `C:\Games\ZA Recomp` (not inside `Program Files`: the game keeps its saves
   and settings next to itself).
2. **Prepare your own copy of the game** in a folder, in one of two ways:
   - **Extracted game**: the `exefs` and `romfs` folders, as an emulator or your Switch extracts them. No keys needed.
   - **Dump**: the base game (`.xci`, `.nsp` or `.nup`), optionally the update and the *Mega Dimension* DLC (`.nsp`).
     `.xci` / `.nsp` packages need the `prod.keys` from your own Switch; a `.nup` needs no keys.
3. **Run `lasrecomp.exe`.** On the start screen, choose that folder (or drag it onto the window) and click
   **Set up the game**.
4. **Wait for the one-time setup.** The game's files are read and its code is compiled for your PC: 10 to 30 minutes
   depending on your CPU, **only once**.
5. **Click Play.** The very first start also prepares the game's shaders for your GPU: the window can stay black for a
   few minutes. After that, starts are quick.

Your own game files are never changed. Optionally, import your console's fonts on the start screen
(**Settings > Files**) for the original text; without them a substitute font is used.

### Upgrading an existing ZA Recomp install

Already playing ZA Recomp? Close the game, extract the zip **over your install folder** and allow it to replace the
files. Your saves (`save/`), settings (`larecomp.ini`), mods and the set-up game (`extracted/`) are kept: the zip
contains none of them.

- **From upstream v0.1.23**: nothing to set up again. The first start prepares the shaders once (a few minutes of black
  window).
- **From another upstream version** (v0.1.24 and later): the start screen may compile the game code again (10-30
  minutes, once). Your saves still work.
- **Your keys stay as they were.** For this edition's layout: **F1 > Controls > Keyboard > Keys: PC layout**.

To go back to upstream, download its release and extract it over the folder the same way.

### DLC, Switch 2 Edition and `.nup`

- **Mega Dimension DLC**: use the dump route (the extracted `exefs` / `romfs` folders don't include it). Put the base
  game, the update (2.0.0 or newer), the DLC `.nsp` and your `prod.keys` (plus `title.keys` if asked) in one folder,
  choose it and click **Set up the game**. The start screen shows **+ DLC** when it was found. Already set up without
  it? **Change game folder**, choose the folder with the DLC added and set it up again; saves and settings are kept.
- **Switch 2 Edition / `.nup`**: choose the folder with the `.nup` directly - no `prod.keys` needed. Use the **2.0.2**
  update for Lumiose Online. The *Mega Dimension* DLC is separate (its `.nsp` and your keys in the same folder).

## Requirements

| | |
|---|---|
| **OS** | Windows 10 (1903 or newer) or Windows 11, 64-bit |
| **CPU** | x86-64 with AVX2 (2013 or newer). Older CPUs with SSE4.2 also work, but compile and run slower |
| **GPU** | Vulkan 1.2 or newer (1.3 recommended) with an up-to-date driver. Tuned on Intel UHD Graphics; NVIDIA and AMD run as on upstream |
| **RAM** | 8 GB (16 GB recommended) |
| **Disk** | About 12 GB free |
| **Game** | Pokémon Legends: Z-A **1.0.0, 2.0.0 or 2.0.2**, optionally with the *Mega Dimension* DLC; a `.nup` whose update is one of these also works |

**Intel graphics:** keep your driver up to date from [Intel's driver page](https://www.intel.com/content/www/us/en/download-center/home.html)
(the results above were measured with 32.0.101.7092). Only the Intel UHD Graphics of an i3-1215U was measured; other
Intel GPUs (Iris Xe, Arc, older UHD) are expected to gain the same way - reports are welcome.

Linux and macOS are not covered by this edition: see upstream's
[Linux notes](docs/linux.md) and [MonZaMac](https://github.com/lucabravi/MonZaMac) for macOS.

## Controls

**Mouse and keyboard** (the PC layout - new installs start with it; **F1 > Controls > Keys: PC layout** sets it any time):

| Key | Action | Switch button |
|---|---|---|
| **W A S D** | Move | Left stick |
| **Mouse** | Camera (locked on: change target) | Right stick |
| **I J K L** | Camera from the keyboard | Right stick |
| **Left click** | Throw a Poké Ball (hold: aim) | ZR |
| **Right click** (hold) | Lock on - then your moves are **F / Shift / Tab / Space** | ZL |
| **F** or **Enter** | Talk, investigate, confirm | A |
| **Left Shift** or **Backspace** | Dash; back / close | B |
| **Space** | Roll | Y |
| **Tab** | Main menu | X |
| **M** | Map (locked on: Plus moves on / off) | + |
| **P** | Photo mode | − |
| **Q / E**, **mouse wheel** or **← / →** | Previous / next Pokémon | D-pad left / right |
| **↑** or **R** | Send out your Pokémon | D-pad up |
| **↓** or **G** | Call it back | D-pad down |
| **C** | Crouch / stand | Left stick press |
| **V** | Mega Evolve | Right stick press |
| **Z / X** | Reset the camera / turn it while locked on; menu tabs | L / R |
| **Esc** or **F1** | Settings menu | - |
| **F11** or **Alt+Enter** | Fullscreen | - |
| **F10** | Screenshot (`screenshots/`) | - |

Send out and call back show the arrow keys on screen: the game draws "call back" as the "send out" icon turned upside
down, which only an arrow survives. R and G do the same.

**Controllers**: any SDL3 controller works - Xbox, PlayStation (DualShock 4 / DualSense), Switch Pro, Steam Deck and
more - with icons that match it. Every button and key can be changed in **F1 > Controls > Configure inputs**; the mouse
buttons and wheel in **F1 > Controls > Mouse**.

## Getting the best performance

- **Frame rate**: **F1 > Video**: 30 fps gives the steadiest result on integrated graphics (the numbers above are at
  30 fps). With 60 fps, **Drop to 30 fps when needed** avoids slow motion when the PC can't keep up.
- **Resolution**: **Match display** renders at your screen's resolution (native 1080p on a 1080p laptop). If a scene
  still drops below 30, try **Dynamic** resolution or a lower internal resolution before turning anything else off.
- **Power**: play plugged in, with Windows' power mode on **Best performance** - integrated GPUs share the laptop's
  power budget with the CPU.
- **Drivers**: an up-to-date Intel graphics driver matters more than any setting.
- **Optional effects** (ambient occlusion, indirect light, water reflections, HD textures) cost GPU time; they are off
  by default.

## Saves, mods and multiplayer

- **Saves** are the game's own files, in `save/save_0/` next to `lasrecomp.exe`, the same as on the Switch. The
  **Saves** page on the start screen imports, exports and restores backups. Switch → PC: back up with **JKSV** on the
  Switch and import the folder (it holds a file named `main`). A 2.0.x save needs the game set up with the 2.0.x
  update; 2.0.0 converts a 1.0.0 save the first time it saves, so keep a backup.
- **Mods** that replace game files (a `romfs` folder: translations, textures, models) install from the **Mods** page or
  by dropping them on the window. Mods that change the game's code (`exefs`, IPS patches, cheat plugins) don't work
  with a recompiled game.
- **Link Play** (trades and private battles, a test feature) works between two copies, on the same network or over the
  internet with **F1 > Online** codes - no server, no account. Upstream's
  [multiplayer notes](https://github.com/Hexveil404/PokeZ-ARecomp/tree/v0.1.23#multiplayer-test) explain how it works
  and what it sends.
- **DirectX 12** (**F1 > Video > Graphics API**) runs the same renderer through Mesa's Dozen for cards whose Vulkan
  driver crashes the game. This edition was measured on Vulkan; Vulkan is the right choice on Intel.

## Troubleshooting

- **Logs**: each session writes `logs/play.log` next to the exe (the previous one as `logs/play-anterior.log`). Attach
  it to any report.
- **Something looks wrong or crashes in this edition but not upstream**: each change can be turned off on its own.
  Make a file `play-safe.bat` next to `lasrecomp.exe` with, for example:

  ```bat
  set LARECOMP_SHADER_FLAT_LOOPS=0
  set LARECOMP_SHADER_RS_SELECT=0
  start "" lasrecomp.exe
  ```

  and start the game with it. The full list of switches is in [docs/INTEL.md](docs/INTEL.md). Please report which one
  fixed it.
- **Graphics driver errors** ("Your graphics driver crashed", a GPU page fault): **Settings > Video > Compatibility**,
  turn the options off one at a time (they apply from the next start).
- **Black window for a few minutes after installing or updating**: the shaders are being prepared for your GPU. It
  happens once per version.
- **Button icons still show the old keys**: they are redrawn in the background after a change and show from the next
  start.
- **Report a problem**: [open an issue here](https://github.com/SaiSarvajeet27/PokeZ-ARecomp_Intel_iGPU_Optimized/issues)
  (not on the upstream project) with your CPU, GPU, driver version, game version and `logs/play.log`.

## Building from source

Windows, with MSYS2 (UCRT64) and Python - the full setup is in [docs/BUILD-windows.md](docs/BUILD-windows.md). From
an MSYS2 UCRT64 shell in the repository:

```bash
tools/build_deps_windows.sh     # once: the LGPL FFmpeg the release links against (TMPDIR=/tmp)
make -j8 B=build/za-dist DEPS="$PWD/build/deps-win" RELEASE=1 GENARCH=-march=x86-64-v3 \
     GENOPT="-O1 -DGUEST_VOLATILE" build/za-dist/lasrecomp.exe
```

Copy `build/za-dist/lasrecomp.exe` and `lasrecomp.dll.a` over an install. The button-icon helper (`zaprompts`) is
rebuilt with PyInstaller (`tools/zaprompts.spec`); `bash tools/za/dist.sh` builds the whole release folder
(`ZIP=1` for the zip). Steps, tests and where each change lives: [docs/INTEL.md](docs/INTEL.md#building).

Never commit or attach Nintendo files (game files, extracted data, generated code, fonts, saves).

## Credits

- **[SaiSarvajeet27](https://github.com/SaiSarvajeet27)**: this Intel iGPU Edition - the renderer and shader
  translator optimizations, the PC controls and their tooling.
- **[Peiikai](https://github.com/peiikai)**: creator of [LAS Recomp](https://github.com/peiikai/LASRecomp), the static
  recompiler, runtime, renderers and shader translator everything here is built on.
- **[Hexveil404](https://github.com/Hexveil404)**: [ZA Recomp](https://github.com/Hexveil404/PokeZ-ARecomp), the
  Pokémon Legends: Z-A port this edition is a fork of.
- **ZA Recomp contributors**: [@Primus92](https://github.com/Primus92), [@Atterax](https://github.com/Atterax),
  [@wimw85](https://github.com/wimw85), [@lucabravi](https://github.com/lucabravi),
  [@gabrielsem16-bot](https://github.com/gabrielsem16-bot), [@gdsistemas-web](https://github.com/gdsistemas-web) and
  everyone who reported issues and tested builds. **LAS Recomp contributors**: see
  [docs/LASRecomp-README.md](docs/LASRecomp-README.md#credits).
- **[yuzu](https://github.com/yuzu-emu)**'s shader recompiler, whose model of Maxwell's branch stack (one stack,
  unwound by SYNC / BRK) the loop translation follows (no code taken); **[Ryujinx](https://github.com/Ryujinx/Ryujinx)**,
  **[envytools](https://github.com/envytools/envytools)** and **[Switchbrew](https://switchbrew.org)** for reference.
- **Libraries**: [SDL 3](https://libsdl.org), [FFmpeg](https://ffmpeg.org), [Dear ImGui](https://github.com/ocornut/imgui),
  [Vulkan Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator),
  [shaderc](https://github.com/google/shaderc), [SMAA](https://github.com/iryoku/smaa),
  [AMD FidelityFX FSR](https://github.com/GPUOpen-Effects), [XeGTAO](https://github.com/GameTechDev/XeGTAO),
  [LibHac](https://github.com/Thealexbarney/LibHac), [Zig](https://ziglang.org) and the others listed in
  [docs/LASRecomp-README.md](docs/LASRecomp-README.md#license).

## Legal

This is a fan project and a community fork. It is not affiliated with, endorsed or sponsored by Nintendo, Game Freak,
Creatures, The Pokémon Company, or the upstream ZA Recomp / LAS Recomp projects. *Pokémon* and *Pokémon Legends: Z-A*
are trademarks of their respective owners. No game files, code, assets or fonts are included; you must provide your own
legally obtained copy of the game.

The code is released under the [MIT License](LICENSE) (LAS Recomp © Peiikai). Third-party components keep their own
licenses; see [docs/LASRecomp-README.md](docs/LASRecomp-README.md#license). The release's DirectX 12 files are
[Mesa](https://mesa3d.org)'s Dozen (MIT, built with `tools/dx12/dozen-za.patch`) with Microsoft's redistributable
`dxil.dll` and Direct3D 12 Agility SDK; the button-icon helper ships with ooz (GPLv3, its licence and source notes in
`zaprompts/`); FFmpeg is LGPL, built from source by `tools/build_deps_windows.sh`. NVIDIA DLAA uses NVIDIA's
`nvngx_dlss.dll`, which isn't included.
