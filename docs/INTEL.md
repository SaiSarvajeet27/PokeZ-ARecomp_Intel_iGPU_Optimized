# Intel integrated graphics and PC controls (community fork)

This community fork is the ZA Recomp v0.1.23 source (github.com/Hexveil404/PokeZ-ARecomp, tag v0.1.23) with the
changes described here (`git diff v0.1.23` shows all of them). It was tuned and measured on an i3-1215U with Intel
UHD Graphics (driver 32.0.101.7092, Vulkan 1.4.323), at native 1920x1080 ("Match display").

Nothing here comes with game files: as upstream, you need your own copy of the game, imported on the start screen.
The game code library (`extracted/lasgame.dll`) doesn't change with these runtime changes: an install set up with
v0.1.23 keeps it. The first start after switching rebuilds the shader caches (a black window for a few minutes).

## Version and updates

This fork's builds are **v0.1.23a** (a letter after the upstream version they're based on; the next would be v0.1.23b).
The start screen's update check, project page and "Report a problem" use this fork's repository, not upstream's: an
upstream release would replace the build with one without these changes. The version check ranks
v0.1.23 < v0.1.23a < v0.1.23b < v0.1.24. Changing the version rebuilds the shader caches once at the next start.

## On other hardware

- **NVIDIA / AMD**: the render passes, robust access 2 and (on NVIDIA) quad control stay as upstream has them. The depth
  copies as one draw, depth-only snapshots and the shader translation changes apply to every GPU: they give the same
  results with less work (`tools/test_translate.sh` checks every translation still compiles and validates; the Metal
  output is byte-identical to v0.1.23). Only Intel hardware was measured.
- **Other Intel GPUs** (Iris Xe, Arc, older UHD): expected to gain the same way, but only the UHD above was measured.
- **If something looks wrong**, each change has a switch to turn it off (the right-hand column below): set it as an
  environment variable before starting, e.g. in a `.bat` next to `lasrecomp.exe`:
  `set LARECOMP_SHADER_FLAT_LOOPS=0` then `lasrecomp.exe`. Please report which switch fixed it.

## What changed (all renderer code; no settings, no lower quality)

| Change | Why | Kill switch |
|---|---|---|
| Classic render passes on every Intel driver | Intel's driver runs Z-A's ~105 passes a frame much faster this way | `LARECOMP_VK_INTEL_RENDER_PASSES=0` |
| Quad control on Intel | fixes the eyes' iris drawn as magenta noise | `LARECOMP_SHADER_QUAD_CONTROL=0` |
| No throwaway colour target on depth-only passes (only old Intel drivers that crash need it) | it forced fragment shaders to run in shadow/depth passes | `LARECOMP_VK_PLACEHOLDER_COLOR=1` |
| Scaled depth copies (the depth pyramid) as one draw each | they went through 5 transfers + 4 full barriers via a buffer and two colour images: ~5.5 ms a frame | `LARECOMP_VK_DEPTH_COPY_DRAW=0` |
| Depth snapshots copy only the depth they're read as | the stencil was copied along and never read | `LARECOMP_SNAP_ALL_ASPECTS=1` |
| Robust access 2 off on Intel (buffers keep robustBufferAccess, images get robust access 1) | the strict bounds checks cost ~13% in Z-A's constant-heavy shaders | `LARECOMP_VK_ROBUST2=1`, or F1 > Compatibility > Robust GPU access |
| Translated shaders use SPIR-V NMin / NMax / NClamp for the Switch's NaN-safe min / max / saturate | the same results in one instruction instead of ~5 (about 150 in the lighting shader) | `LARECOMP_SHADER_NMINMAX=0` |
| Render-scale helpers in translated shaders (rsc / uvs / rscf) select instead of branch | each texture read sat behind its own branch, so Intel's compiler couldn't issue a shader's reads together: GPU 34.3 -> 32.9 ms a frame outdoors | `LARECOMP_SHADER_RS_SELECT=0` |
| Shaders with loops translated as plain code (`for (;;)` around each loop) instead of the `switch (pc)` state machine, which kept two 16-deep branch stacks as 32 variables shifted on every push / pop. Branch stacks resolved at translation time, with the hardware's one-stack model (BRK / SYNC unwind, as yuzu models it) as a second try, and computed-branch tables cut where the next switch's table starts | ambient occlusion 2.37 -> 1.63 ms, GPU culling / animation compute 1.12 -> 0.34 ms (+ two smaller ones): GPU 32.1 -> 30.5 ms a frame outdoors | `LARECOMP_SHADER_FLAT_LOOPS=0` |
| Profiler: `LARECOMP_GPUPROF_DRAWS=1` (a row per draw), GLSL dumps named by shader hash; `LARECOMP_VK_PIPE_STATS=1` (Intel's SIMD width / instruction counts per pipeline, `[pstat]` in the log); `LARECOMP_GLSL_OVERRIDE=<folder>` (load hand-edited shaders); `LARECOMP_COPY_TRACE` lines carry each draw's depth / stencil state; `LARECOMP_SHADER_FLAT_LOG=1` says why a shader keeps the state machine | tools used to find the above | - |

## Measured (native 1080p, same saves, same moment, 35 s averages)

| Scene | v0.1.23 as shipped | binary patch (render passes + eyes) | this build |
|---|---|---|---|
| Hotel lobby (indoors) | - | 25.7 fps | 30.0 fps locked (GPU 26 ms a frame) |
| Vert District, crowded (outdoors) | 14.1 fps | 21.8 fps | 30.0 fps locked: 30.0 average and minimum over every 5 s, worst frame 34-35 ms, GPU 30.3-30.6 ms a frame (loops as state machines: 29.6-29.8, worst frames 38-45 ms) |

A 7-minute run turning around the courtyard (the story keeps the player there on that save) held 30.0 except three
single frames of 50-64 ms (GPU spikes) and the test's own screenshots. Outdoor runs pin the game's clock
(`LARECOMP_CLOCK_START`): the day / night lighting follows the real time and changes the GPU load.

Screenshots of the same frames with each change on and off differ only where things move with the clock
(the player's idle pose, foliage in the wind), as two runs of the same build do.

Tried and left out: a stencil write mask of 0 for KEEP-only faces, and early depth / stencil tests forced on the
202 point-light volumes - the light volumes cost the same (Intel already tests them first). Larger GPU submissions
(32 passes instead of 8): no difference. The lighting and ambient-occlusion shaders stay at SIMD8 on Intel: their
register needs, not the translation (the same with every translation change tried).

## Controls: a PC layout (mouse and keyboard)

F1 > Controls > Keyboard > **Keys: PC layout** sets it in one go (the defaults stay upstream's, so an existing install
keeps its keys until you choose it). The game's on-screen button icons name these keys - and LMB / RMB for the mouse
buttons - from the next start.

| Key | Game (Switch) | Action |
|---|---|---|
| W A S D | left stick | move |
| Mouse | right stick | camera (when locked on: change target) |
| I J K L | right stick | camera from the keyboard |
| Left click | ZR | throw a Poké Ball (hold: aim) |
| Right click (hold) | ZL | lock on; then the moves are F / Shift / Tab / Space |
| F, or Enter | A | talk, investigate, confirm |
| Left Shift, or Backspace | B | dash, back / close |
| Space | Y | roll |
| Tab | X | main menu |
| M | + | map (locked on: Plus moves on / off) |
| P | - | photo mode |
| Q / E, or the mouse wheel, or Left / Right | d-pad left / right | previous / next Pokémon |
| Up arrow, or R | d-pad up | send out the Pokémon |
| Down arrow, or G | d-pad down | call it back |
| C | left stick press | crouch / stand |
| V | right stick press | Mega Evolve |
| Z / X | L / R | reset the camera / turn it while locked on; menu tabs |
| Esc or F1 | - | settings menu |

Send out / call back show arrow keys on screen because the game draws the "call back" badge as the "send out" icon
turned upside down: only an arrow reads right both ways (a letter came out upside down, naming the wrong key).

What the code adds for it (runtime/hle/input.c, bindings.c, ui/controls_ui.mm, ui/launcher.mm, tools/za/prompt_textures.py):
- a second key per input (`bind_key2_<input>` lines in `larecomp.ini`, shown as "F / Return" in Configure inputs):
  Enter as well as F, Backspace as well as Shift, the arrows as well as Q / E / R / G. One that would clash with
  another key, or Esc / F1 / F9-F11, is left out when the file is read (the log says so);
- the mouse wheel as two more mouse "buttons" (`bind_mouse_wheel_up / wheel_down`; F1 > Controls > Mouse): each notch
  one press, so a fast turn still moves one Pokémon a notch;
- the icon maker (`zaprompts`) names an input on a mouse button by the button (LMB, RMB, MMB, M4, M5), and the icons
  are made again when a mouse binding changes. **It has to be rebuilt** (below): an older `zaprompts` still works but
  shows the keys 1 / 3 for the mouse buttons.

`tools/test_key2.c` tests the second keys, the wheel's bindings and the preset.

## Building

As upstream ([BUILD-windows.md](BUILD-windows.md)). From an MSYS2 UCRT64 shell in the repository:

    tools/build_deps_windows.sh     # once: the LGPL FFmpeg the release links against (TMPDIR=/tmp)
    make -j8 B=build/za-dist DEPS="$PWD/build/deps-win" RELEASE=1 GENARCH=-march=x86-64-v3 \
         GENOPT="-O1 -DGUEST_VOLATILE" build/za-dist/lasrecomp.exe

then copy `build/za-dist/lasrecomp.exe` and `lasrecomp.dll.a` over a v0.1.23 install. The icon maker, from a
Python 3.11 with pyinstaller, pillow, numpy, fonttools and cffsubr (`build/ooz/ooz.dll`: built by `tools/za/dist.sh`,
or copied from a release's `zaprompts/_internal`):

    python -m PyInstaller --noconfirm --distpath build/helper --workpath build/helper-work tools/zaprompts.spec

then copy `build/helper/zaprompts/zaprompts.exe` and `_internal/` into the install's `zaprompts/` (keep `oodec.exe`).

Tests run for this fork: `tools/test_translate.sh 20000 6 both` (with `REF=v0.1.23` on a shallow clone),
`tools/test_settings.py`, `tools/test_first_start_pad.c`, `tools/test_key2.c`.
