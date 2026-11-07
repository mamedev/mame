# MK4 v17 lighting experiment, 9 October 2026

The `mk4` (version 3.0) driver carries the material response and pixel arithmetic
from the blahm1d MK4 MiSTer v17 renderer. Other Zeus games and MK4 revisions keep
the preceding general response-table model. This is an experimental approximation,
not a recovered Zeus microcode implementation or a claim of complete PCB fidelity.

The common model uses the game's packed signed 10-bit normals, current matrix,
FIFO light vector, and uploaded 128-byte response table:

```
index = clamp(floor(dot(N, transpose(M) * L) / 2^26), -64, 63) & 127
intensity = table[index] * 256
```

The MK4 extension retains v17's existing asset filters, fighter contrast,
material alpha contrast, selected view-dependent rim response, additive-particle
modulation, nearest RGB555 packing, and Well shaft depth fade. It adds 23 fixed
gain/bias pairs identified by response pointer and the first table word, plus
register 4C modulation on the tested direct rectangle path. These selectors and
coefficients are empirical. They must not be interpreted as documented hardware
flags, universal material rules, or evidence for special handling in original
Zeus silicon. Unknown material signatures retain the prior response path.

The normal, light, table and texture data are unchanged. No PCB image lookup or
screen-coordinate correction runs in MAME. The selected filters protect the
existing short-quad HUD path; background changes can still affect additive HUD
composition. Table reads use MAME's endian-aware waveram accessor.

## PCB comparison method

The supplied nine-arena capture has SHA-256
`96334c6019e5def1bbd685627c10d2d1e3a8e2c96ed0b8687bddb81b1b60af9c`.
It is 2374 frames at 60 frames/s. One fixed per-channel affine transfer uses
native white interiors from eight Kombat Kode announcements and blank-frame
black. Common alignment comes from HUD/text, rather than fitted fighter or
background brightness. The same transfer is applied to every arena.

Capture white BGR is approximately (226.22, 225.24, 227.33), not 255.
Black BGR is (13, 12, 15). Gains BGR are
(0.8361431373, 0.8362333333, 0.8326647059).
Calibration file SHA-256:
`3b09a8749cc957042c20807a6f38e0e1429c6aae20b95bd556b0e1fcfe85df20`.
This endpoint calibration does not recover capture gamma or filtering.

The comparison GIF alternates calibrated PCB frames with actual v17 RTL replay
images across all nine arenas. It is not compiled-MAME or cabinet footage.
Both states share each pair's GIF palette; unquantized PNGs are supplied separately.
Poses, HUD values and effect phases differ between runs. There is no matched PCB
falling sequence in this capture, so the v16/v17 shaft comparison is separate.

## Measured stage response

Values are pixel-weighted absolute errors of selected material-region mean
luminance, on a 0-255 scale. They are not whole-image pixel MAE. Later windows were
excluded from coefficient fitting; their stable-region selection may differ.

| Arena | Initial v16 | Initial v17 | Later v16 | Later v17 |
|---|---:|---:|---:|---:|
| Tomb | 3.99 | 0.95 | 4.06 | 0.84 |
| Elder Gods | 2.90 | 1.18 | 3.06 | 1.20 |
| Living Forest | 1.55 | 0.88 | 1.52 | 0.91 |
| Goro's Lair | 2.47 | 1.32 | 2.46 | 1.31 |
| Prison | 11.11 | 1.93 | 11.79 | 1.98 |
| Reptile's Lair | 6.55 | 0.69 | 6.07 | 0.76 |
| Shaolin | 7.02 | 1.19 | 6.85 | 1.25 |
| Well | 4.79 | 1.19 | 5.26 | 1.50 |
| Wind World | 1.59 | 0.61 | 5.65 | 3.94 |

Character leg/shape lighting, dark-tone quantization, some channel errors and
animated effects remain imperfect. Improving these regional means does not prove
the original lighting equation or exact frame correspondence.

## Reproduction and scope

The portable arithmetic is in `src/mame/williams/midzeus_mk4_v17.h`.
Build the asset-free test with a C++17 compiler:

```
g++ -std=c++17 -O2 scripts/tests/midzeus_mk4_v17.cpp -o mk4-v17-test
./mk4-v17-test
```

`scripts/tests/midzeus_mk4_v17.lua` is a compiled-MAME register integration test.
Supply your own legally obtained MK4 ROMs, set `ZEUS_TEST_OUT` to a disposable
output directory, and use `-autoboot_script` with separate cfg/nvram directories.
It writes only synthetic tables/geometry, checks response reload and state restore,
and exits with a status file. No ROM or saved game-state payload is distributed.

Cross-checking the portable functions against v17's actual Verilated frontend and
pixel pipeline is separate from compiled-MAME integration. Rasterization, clipping
and interpolation differences still prevent claiming every MAME pixel equals RTL.
The validation receipt records the exact header hash, checked cases and counts.
The 36-frame cross-check covers all nine arenas, two Continue/falling windows and
a UI window: 433,844 vertex results and 5,243,616 pixel operations match with zero
mismatches. Instrumented replay color/depth hashes match the frozen v17 outputs.

The general model and original three-vertex clipping repair remain in the branch.
No new persistent state is added by the MK4 extension. Existing lighting state is
registered for save/load. This fork is not made or supported by the MAME team;
please report fork-specific issues to its publisher.
