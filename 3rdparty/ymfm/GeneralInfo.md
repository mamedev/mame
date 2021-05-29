# ymfm: One FM core to rule them all

The ymfm emulator ws written from the ground-up using the analysis and deduction by Nemesis as a starting point, particularly in [this thread](https://gendev.spritesmind.net/forum/viewtopic.php?f=24&t=386).

The core assumption is that these details apply to all FM variants unless otherwise proven incorrect.

The fine details of this implementation have also been cross-checked against Nemesis' implementation in his [Exodus emulator](https://www.exodusemulator.com/), as well as Alexey Khokholov's ["Nuked" implementations](https://github.com/nukeykt/Nuked-OPN2) based off die shots.

Operator and channel summing/mixing code for OPM and OPN is largely based off of research done by [David Viens](https://twitter.com/plgDavid) and Hubert Lamontagne.

## Families

The Yamaha FM chips can be broadly categoried into families:

* OPM (YM2151)
	* OPP (YM2164)
* OPN (YM2203)
	* OPNA/OPNB/OPN2 (YM2608, YM2610, YM2610B, YM2612, YM3438, YMF276, YMF288)
* OPL (YM3526)
	* OPL2 (YM3812)
	* OPLL (YM2413, YM2423, YMF281, DS1001, and others)
	* OPL3 (YMF262, YMF289B)
	* OPL4 (YMF278)

Additionally, several lesser-documented variants exist exclusively in the employ of Yamaha synthesizers:

* OPQ (YM3608)
* OPZ (YM2414)

All of these families are very closely related, and the ymfm engine is designed to be universal to work across all of
these families.

Of course, each variant has its own register maps, features, and implementation details which need to be sorted out.
Thus, each significant variant listed above is represented by a register class.
The register class contains:

* constants describing core parameters and features
* mappers between operators and channels
* generic fetchers that return normalized values across families
* family-specific implementations of LFO and phase calculations

## Family History

This history outlines the progress of adding/removing features across the three main families (OPM, OPN, OPL):

OPM started it all off, featuring:
* 8 FM channels, 4 operators each
* LFO and noise support
* Stereo output

OPM -> OPN changes:
* Reduced to 3 FM channels, 4 operators each
* Removed LFO and noise support
* Mono output
* Integrated AY-8910 compatible PSG
* Added SSG-EG envelope mode
* Added multi-frequency mode: ch. 3 operators can have separate frequencies
* Software controlled clock divider

OPN -> OPNA changes:
* Increased to 6 FM channels, 4 operators each
* Added back (a cut-down) LFO
* Stereo output again
* Removed software controlled divider on later versions (OPNB/OPN2)
* Removed PSG on OPN2 models

OPNA -> OPL changes:
* Increased to 9 FM channels, but only 2 operators each
* Even more simplified LFO
* Mono output
* Removed PSG
* Removed SSG-EG envelope modes
* Removed multi-frequency modes
* Fixed clock divider
* Built-in ryhthm generation

OPL -> OPL2 changes:
* Added 4 selectable waveforms

OPL2 -> OPLL changes:
* Vastly simplified register map
* 15 built-in instruments, plus built-in rhythm instruments
* 1 user-controlled instrument

OPL2 -> OPL3 changes:
* Increased to 18 FM channels, 2 operators each
* 4 output channels
* Increased to 8 selectable waveforms
* 6 channels can be configured to use 4 operators

## Channels and Operators

The polyphony of a given chip is determined by the number of channels it supports.
This number ranges from as low as 3 to as high as 18.
Each channel has either 2 or 4 operators that can be combined in a myriad of ways.
On most chips the number of operators per channel is fixed; however, some later OPL chips allow this to be toggled between 2 and 4 at runtime.

The base ymfm engine class maintains an array of channels and operators, while the relationship between the two is described by the register class.

## Registers

Registers on the Yamaha chips are generally write-only, and can be divided into three distinct categories:

* system-wide registers
* channel-specific registers
* operator-specific registers

For maximum flexibility, most parameters can be configured at the operator level, with channel-level registers controlling details such as how to combine the operators into the final output.
System-wide registers are used to control chip-wide modes and manage onboard timer functions.

Note that since registers are write-only, some ymfm register classes will use "holes" in the register space to store additional values that may be needed.

## Attenuation

Most of the computations of the FM engines are done in terms of attenuation, and thus are logarithmic in nature.
The maximum resolution used internally is 12 bits, as returned by the sin table:

Bit | 11 | 10 |  9 |  8 |  7 |  6 |   5 |    4 |     3 |      2 |       1 |        0
----|----|----|----|----|----|----|-----|------|-------|--------|---------|---------
 dB | -96| -48| -24| -12|  -6|  -3| -1.5| -0.75| -0.375| -0.1875| -0.09375| -0.046875

The envelope generator internally uses 10 bits:

Bit |  9 |  8 |  7 |  6 |  5 |   4 |    3 |     2 |      1 |       0 |
----|----|----|----|----|----|-----|------|-------|--------|---------|
 dB | -48| -24| -12|  -6|  -3| -1.5| -0.75| -0.375| -0.1875| -0.09375|

Total level for operators is usually represented by 7 bits:

Bit |  6 |  5 |  4 |  3 |  2 |   1 |    0 |
----|----|----|----|----|----|-----|------|
 dB | -48| -24| -12|  -6|  -3| -1.5| -0.75|

Sustain level in the envelope generator is usually represented by 4 bits:

Bit |  3 |  2 |  1 |  0 |
----|----|----|----|----|
 dB | -24| -12|  -6|  -3|

## Status and Timers

Generically, all chips (except OPLL) support two timers that can be programmed to fire and signal IRQs.
These timers also set bits in the status register.
The behavior of these bits is shared across all implementations, even if the exact bit positions shift (this is controlled by constants in the registers class).

In addition, several chips incorporate ADPCM decoders which also may set bits in the same status register.
For this reason, it is possible to control various bits in the status register via the `set_reset_status()` function directly.
Any active bits that are set and which are not masked (mask is controlled by `set_irq_mask()`), lead to an IRQ being signalled.

Thus, it is possible for the chip-specific implementations to set the mask and control the status register bits such that IRQs are signalled via the same mechanism as timer signals.

In addition, the OPM and OPN families have a "busy" flag, which is set after each write, indicating that another write should not be performed.
Historically, the duration of this flag was constant and had nothing to do with the internals of the chip.
However, since the details can potentially vary chip-to-chip, it is the chip's responsibility to insert the busy flag into the status before returning it to the caller.

## Clocking

Each of the Yamaha chips works by cycling through all operators one at a time.
Thus, the effective output rate of the chips is related to the input clock divided by the number of operators.
In addition, the input clock is prescaled by an amount.
Generally, this is a fixed value, though some early OPN chips allow this to be selected at runtime from a small
number of values.

## Channel Frequencies

One major difference between OPM and later families is in how frequencies are specified.
OPM specifies frequency via a 3-bit 'block' (aka octave), combined with a 4-bit 'key code' (note number) and a 6-bit 'key fraction'.
The key code and fraction are converted on the chip into an x.11 fixed-point value and then shifted by the block to produce the final step value for the phase.

Later families, on the other hand, specify frequencies via a 3-bit 'block' just as on OPM, but combined with a 9-12-bit 'frequency number' or 'fnum', which is directly shifted by the block to produce the step value.
So essentially, later chips make the user do the conversion from note value to phase increment, while OPM is programmed in a more 'musical' way, specifying notes and cents.

Internally, this is abstracted away into a 'block_freq' value, which is a 16-bit value containing the block and frequency info concatenated together as follows:

* OPM:  `[3-bit block]:[4-bit keycode]:[6-bit fraction] = 13 bits total`

* OPZ:  `[3-bit block]:[12-bit fnum]     = 15 bits total`
* OPN:  `[3-bit block]:[11-bit fnum] 0   = 15 bits total`
* OPL:  `[3-bit block]:[10-bit fnum]:00  = 15 bits total`
* OPLL: `[3-bit block]:[ 9-bit fnum]:000 = 15 bits total`

The register classes handle the raw format directly and convert it into a phase increment which can be used by the generic engine.

## Low Frequency Oscillator (LFO)

The LFO engines are different in several key ways.
The OPM LFO engine is fairly intricate.
It has a 4.4 floating-point rate which allows for a huge range of frequencies, and can select between four different waveforms (sawtooth, square, triangle, or noise).
Separate 7-bit depth controls for AM and PM control the amount of modulation applied in each case.
This global LFO value is then further controlled at the channel level by a 2-bit AM sensitivity and a 3-bit PM sensitivity, and each operator has a 1-bit AM on/off switch.

For OPN the LFO engine was removed entirely, but a limited version was put back in OPNA and later chips.
This stripped-down version offered only a 3-bit rate setting (versus the 4.4 floating-point rate in OPN), and no
global depth control.
It did bring back the channel-level sensitivity controls and the operator-level on/off control.

For OPL, the LFO is simplified again, with AM and PM running at fixed frequencies, and simple enable flags at the operator level for each controlling their application.

## Differences Between Families

The table below provides some high level functional differences between the differnet families:

subfamily:   |   OPM  |   OPN  |  OPNA  |   OPL  |  OPL2  |  OPLL  |  OPL3  |
------------:|:------:|:------:|:------:|:------:|:------:|:------:|:------:|
outputs:     |    2   |    1   |    2   |    1   |    1   |    1   |    4   |
channels:    |    8   |    3   |    6   |    9   |    9   |    9   |   18   |
operators:   |   32   |   12   |   24   |   18   |   18   |   18   |   36   |
waveforms:   |    1   |    1   |    1   |    1   |    4   |    2   |    8   |
instruments: |   no   |   no   |   no   |   yes  |   yes  |   yes  |   yes  |
ryhthm:      |   no   |   no   |   no   |   no   |   no   |   yes  |   no   |
dynamic ops: |   no   |   no   |   no   |   no   |   no   |   no   |   yes  |
prescale:    |    2   |  2/3/6 |  2/3/6 |    4   |    4   |    4   |    8   |
EG divider:  |    3   |    3   |    3   |    1   |    1   |    1   |    1   |
EG DP:       |   no   |   no   |   no   |   no   |   no   |   yes  |   no   |
EG SSG:      |   no   |   yes  |   yes  |   no   |   no   |   no   |   no   |
mod delay:   |   no   |   no   |   no   |   yes  |   yes  |   yes? |   no   |
CSM:         |   yes  |  ch 2  |  ch 2  |   yes  |   yes  |   yes  |   no   |
LFO:         |   yes  |   no   |   yes  |   yes  |   yes  |   yes  |   yes  |
noise:       |   yes  |   no   |   no   |   no   |   no   |   no   |   no   |

* Outputs represents the number of output channels: 1=mono, 2=stereo, 4=stereo+.
* Channels represents the number of independent FM channels.
* Operators represents the number of operators, or "slots" which are assembled into the channels.
* Waveforms represents the number of different sine-derived waveforms available.
* Instruments indicates whether the family has built-in instruments.
* Rhythm indicates whether the family has a built-in rhythm
* Dynamic ops indicates whether it is possible to switch between 2-operator and 4-operator modes dynamically.
* Prescale specifies the default clock divider; some chips allow this to be controlled via register writes.
* EG divider represents the divider applied to the envelope generator clock.
* EG DP indicates whether the envelope generator includes a DP (depress?) phase at the beginning of each key on.
* SSG EG indicates whether the envelope generator has SSG-style support.
* Mod delay indicates whether the connection to the first modulator's input is delayed by 1 sample.
* CSM indicates whether CSM mode is supported, triggered by timer A.
* LFO indicates whether LFO is supported.
* Noise indicates whether one of the operators can be replaced with a noise source.

## Chip Specifics

While OPM is its own thing, the OPN and OPL families have quite a few specific
implementations, with many differing details beyond the core FM parts. Here are
some details on the OPN family:

chip ID:  | YM2203 | YM2608 | YMF288 | YM2610 | YM2610B | YM2612 | YM3438 | YMF276 |
---------:|:------:|:------:|:------:|:------:|:-------:|:------:|:------:|:------:|
aka:      |   OPN  |  OPNA  |  OPN3L |  OPNB  |  OPNB2  |  OPN2  |  OPN2C |  OPN2L |
FM:       |    3   |    6   |    6   |    4   |    6    |    6   |    6   |    6   |
AY-8910:  |    3   |    1   |    1   |    1   |    1    |    -   |    -   |    -   |
ADPCM-A:  |    -   |  6 int |  6 int |  6 ext |  6 ext  |    -   |    -   |    -   |
ADPCM-B:  |    -   |  1 ext |    -   |  1 ext |  1 ext  |    -   |    -   |    -   |
DAC:      |   no   |   no   |   no   |   no   |   no    |   yes  |   yes  |   yes  |
output:   | 10.3fp | 16-bit | 16-bit | 16-bit |  16-bit |  9-bit |  9-bit | 16-bit |
summing:  |  adder |  adder |  adder |  adder |  adder  |  muxer |  muxer |  adder |

* FM represents the number of FM channels available.
* AY-8910 represents the number of AY-8910-compatible outputs.
* ADPCM-A represents the number of internal/external ADPCM-A channels present.
* ADPCM-B represents the number of internal/external ADPCM-B channels present.
* DAC indicates if a directly-accessible DAC output exists, replacing one channel.
* Output indicates the output format to the final DAC.
* Summing indicates whether channels are added or time divided in the output.

OPL has a similar trove of chip variants:

chip ID:     | YM3526 |  Y8950  | YM3812 | YM2413 | YMF262 | YMF289B | YMF278B |
------------:|:------:|:-------:|:------:|:------:|:------:|:-------:|:-------:|
aka:         |   OPL  |MSX-AUDIO|  OPL2  |  OPLL  |  OPL3  |  OPL3L  |   OPL4  |
FM:          |    9   |    9    |    9   |    9   |   18   |   18    |    18   |
ADPCM-B:     |    -   |  1 ext  |    -   |    -   |    -   |    -    |    -    |
wavetable:   |    -   |    -    |    -   |    -   |    -   |    -    |    24   |
instruments: |   no   |    no   |   no   |   yes  |   no   |   no    |    no   |
output:      | 10.3fp |  10.3fp | 10.3fp |  9-bit | 16-bit | 16-bit  | 16-bit  |
summing:     |  adder |  adder  |  adder |  muxer |  adder |  adder  |  adder  |

* FM represents the number of FM channels available.
* ADPCM-B represents the number of external ADPCM-B channels present.
* Wavetable indicates the number of wavetable channels present.
* Instruments indicates that the chip has built-in instrument selection.
* Output indicates the output format to the final DAC.
* Summing indicates whether channels are added or time divided in the output.

There are several close variants of the YM2413 with different sets of built-in instruments.
These include the YM2423, YMF281, and DS1001 (aka Konami VRC7).
