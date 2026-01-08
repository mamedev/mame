# SAM8905 Programmer's Guide

---

## 1. Purpose

This section describes how to use the SAM chip from a software programmer point of view.

---

## 2. Introduction

The SAM is a powerful signal processor oriented towards music. It generates sound by computing the samples in an "algorithm", which is a succession of micro-instructions.

The SAM is not predefined for any particular synthesis: the algorithms are stored in a built-in RAM, which has to be loaded at power-up by the main processor. This makes it very flexible to adapt to different products. Furthermore, algorithms can be loaded while the synthesis is in progress.

The SAM has a built-in sinus/ramp generator. However, for application involving sampling, up to 1M words of 12 bits sampling memory (RAM or ROM) can be directly connected. With a few external circuits, up to 256 mega words of DRAM sampling memory can be connected (ref: application note, using the SAM with dynamic RAMs).

### Using the SAM involves:

**1- Defining and coding the algorithms:**

For this purpose a language, a compiler and a "code view" debugger are available, as part of a whole package for product development.

**2- Controlling the SAM in a real time environment:**

- Algorithms down-loading
- Algorithms parameters control

---

## 3. SAM Basic Operation

The following assumes that the SAM is used with a 45.1584 MHz quartz.

The SAM can operate at two different sampling rates: 44.1 kHz (CD standard) or 22.05 kHz. The choice of the sampling rate has to be done when defining the product as it greatly influences algorithm sizes, number of simultaneous algorithms, polyphony and sound quality.

In the following, we will assume that the 44.1 kHz sampling rate is used, brackets [] will be used for the corresponding 22.05 kHz figures.

The SAM has two built-in RAMs: The micro-program RAM, referred as A-RAM (256 x 15), which holds the algorithms code and the parameter RAM, referred as D-RAM (256 x 19) which holds the algorithms parameters.

The A-RAM is divided into 8 [4] blocks, each block storing a single algorithm of 32 [64] micro-instructions.

The D-RAM is divided into 16 blocks of 16 words (19bits width), each block holding the parameters of a specific algorithm. The algorithm number related to a particular block is stored at address 15 of the block.

The 44.1 kHz [22.05 kHz] sampling rate corresponds to a time of 22.68 /us [45.35 /us], known as a "frame".

---

## 4. Parameter RAM (D-RAM) Format

The "frame" is divided into 16 "slots", each slot addressing sequentially one block of the D-RAM.

Each slot is divided into 32 [64] cycles, one micro-instruction from an algorithm being executed each cycle.

A micro-program starts at address 0 in each slot, addresses 30 and 31 [62 and 63] are reserved for micro-processor access and algorithm change and the corresponding micro-instructions are not processed.

A micro-program accumulates results of computations into a left channel and a right channel accumulator. At the end of a frame, the content of the accumulators is transferred to a final shift register. This shift register is directly compatible with a standard 16 bits stereo serial DAC.

Any of the 16 x 19 bits words of a D-RAM block can be addressed during a slot by the micro-program. Only address 15 of a D-RAM should hold a specific information, as follows:

```
| 18 |    |    |    | 7  |    |    |    |    |    |    |    |    | 0  |
|----|----|----|----|----|----|----|----|----|----|----|----|----|----|
| X  | X  | X  | X  | I  | ALG     | X  | X  | X  | X  | X  | X  | X  |
```

- **X:** free for other information

- **M:** Interrupt mask, when set, this slot will not generate interrupts.
- **I:** Idle, when set, this slot will execute a no-op algorithm, independently of the selected algorithm.
- **ALG:** selected algorithm, will select the block in the A-RAM. Execution of the corresponding algorithm will start at address:

```
| 7  |    |    |    |    | 0  |
|----|----|----|----|----|----| 
|ALG |    |    | 0  |    |    |     44.1 kHz sampling
```

```
| 7  |    |    |    |    | 0  |
|----|----|----|----|----|----| 
|ALG |    | 0  |    |    |    |     22.05 kHz sampling
```

**Note:** For 22.05 kHz sampling, only the 2 upper ALG bits are used to select the algorithm.

The other addresses of the D-RAM are not assigned to any specific information. However, the word format of the D-RAM depends on the type of information, as follows:

```
| 18 |    |    | 10 |    | 7  |    |    |    |    | 0  |
|----|----|----|----|----|----|----|----|----|----|----|
|         PHI          |              X               |
|       PHIWF          |              X               |
|       PHIREG         |              X               |
```

**Typical micro-instructions: WA, WB, WPHI**

- **PHI:** full phase of a period of a given waveform (unsigned integer)
- **PHIWF:** Portion of phase recommended to access external waveforms (512 samples/period). Up to 12 bits can be used (4096 samples/period with the same frequency definition as the built-in sinus)
- **PHIREG:** Portion of phase loaded at WPHI time, allows to address the internal 4096 samples/period sinus/ramp.

---

## Phase and Frequency (DPHI)

```
| 18 |    |    |    |    |    |    |    |    |    | 0  |
|----|----|----|----|----|----|----|----|----|----|----|
|                       DPHI                          |
```

**Typical micro-instructions: WA, WB**

**DPHI:** Phase interval, signed integer, determines the basic frequency of a signal as follows (standardized to the built-in sinus):

```
f = 0.084114 x DPHI  (Hz)
[f = 0.042057 x DPHI (Hz)]
```

Negative values of DPHI are mainly used for envelope applications (see later)

### Wave Format

```
| 18 |    |    | 9  |    |    |    |    |    |    | 0  |
|----|----|----|----|----|----|----|----|----|----|----|
| E  |     WAVE      |        FINAL WAVE             |
| 0  | ext mem wave  |                               |
| I  |I |R |I |SEL|Z |              X                |
```

**Typical micro-instructions: WA WSP, WWF**

This format is mostly used in sampling applications, involving an external wave-form memory. Specific micro-instructions allow to selectively increment WAVE at the end of a period and to check if WAVE reaches FINALWAVE. Subsequent actions can be taken depending on the state of the E bit (END), like looping on a period or stopping the sampling process.

The upper bit of WAVE indicates an external (0) or internal (1) wave. For internal waves, the remaining bits have the following meaning:

| Field | Description |
|-------|-------------|
| **R:** | 0- Select sinus wave (internal 4k x 12 sinus). In this case the other bits should be zero. |
|        | 1- Select ramp or constant |
| **I:** | 0- direct, 1- two's complement |
| **SEL:** | 00- 2 x PHI ramp |
|          | 01- Constant from micro-instruction |
|          | 10- PHI ramp |
|          | 11- PHI/2 ramp |
| **Z:** | 1- select constant zero as waveform |

See appendix I for more informations about sinus, ramps and constants.

### External Wave Format

When the upper bit of wave is 1, the remaining 8 bits, together with the 12 upper phase bits define the external sampling memory address.

```
| 18 |    |    |    |    |    | 7  |    |    |    | 0  |
|----|----|----|----|----|----|----|----|----|----|----|
|           AMP            | X |MIXL|MIXR|              |
```

**Typical micro-instructions: WXY, WXY WSP, WA, WB**

This format allows to define an amplitude and an optional left and right mix. The amplitude is coded on 12 bits and ranges from -1 to 1-2⁻¹¹.

MIXL and MIXR define the amount of signal to be send resp. to the left and right channels according to the following table:

---

## 5. Algorithm RAM (A-RAM) Format


### MIXL/MIXR Output Attenuation

| Code | Attenuation |
|------|-------------|
| 000  | no signal to output |
| 001  | -36 dB |
| 010  | -30 dB |
| 011  | -24 dB |
| 100  | -18 dB |
| 101  | -12 dB |
| 110  | -6 dB |
| 111  | 0 dB |

**Note:** As the Mix is only updated by a UXY USP, it can be handled in a very flexible way. The minimum requirement is to initialize this information.

### Micro-Instruction Philosophy

The philosophy of a micro instruction is to put one emitter of the bus in relationship with several receivers and/or modifiers and/or do a special action.

The micro-instruction consists of 3 fields:

```
Emitter, MAD, < Receiver0,receiver1,... ,modifier, special action>
```

**The emitter field** allows information to drive the internal 19 bits bus. Obviously, only one emitter can be active at a time. Emitters are defined with an "R" like "read". Possible emitters are the adder (RADD), the D-RAM (RM MAD), the multiplier output (RP), and "nothing" (RSP), which will leave the bus indeterminate but is used to write to the external memory (see later).

**The MAD field** allows to select the address (0 to 15) of the D-RAM into the current block. This field is blank if no D-RAM operation is involved (Don't forget the coma in this case).

**The receiver /modifier/special action field:**

A receiver stores the information of the 19 bits internal bus. Several receivers can be active at a time. Receivers are defined with a "W" like "write". Possible receivers are the A register (WA), the B register (WB), the D-RAM (WM), the phase register (WPHI), the X and Y registers (WXY) and the wave-form register (WWF).

**The Modifier** (WSP), modifies a receiver action.

**The special actions** as clearB which sets the B register to zero and WACC which accumulates the product into the left and right accumulators through the attenuators.

The general micro-instruction format is then:

```
{ RM    }
{ RADD  }   { MAD }  , <[WA] [WB] [WM] [WWF] [WPHI] [WXY] [clearB] [WACC] [WSP] >
{ RP    }
{ RSP   }
```

RM and WM are exclusive

The WSP field, when present, will modify some receiver actions as follows:

### WPHI WSP

The PHI register is loaded normally, but the WF register is set to 100H, which will select the sinus internal wave-form.

### WXY WSP

The X and Y registers are loaded normally, but the MIXL and MIXR fields of the bus are also loaded into the respective output attenuators.

---

## 6. Timing Constraints

### WA WSP and WM WSP

WA WSP and WM WSP will be described later in detail.

Normally, the result of an operation is available on the cycle following the loading of the operands. For example, the operation:

```
RM  PHI,  <WA>
```

can be immediately followed by

```
RADD  ,<WPHI>
```

The result of the addition A+B being available one cycle after the A and B operands have been loaded.

### Exceptions are:

1. Availability of the product (WXY to RP): 2 cycles
2. Availability of the product for Accumulation (WXY to WACC): 3 cycles
3. Availability of the wave sample (WWF or WPHI to WXY):
   - 1- Internal ramp or constant: 1 cycle
   - 2- Internal sinus: 2 cycles
   - 3- External wave memory: (t_acc + 44.2)/44.2 cycles, t_acc being the access time of the external memory. For example, assume a 250ns access time, then 7 cycles are needed.

---

## 7. Simple Algorithm Examples

### Example 1: Sinus Oscillator

```
PHI=0                      ;assign phase to address 0 of RAM block
DPHI=1                     ;assign phase angle (frequency) to address 1
AMP=2                      ;assign amplitude and mix to address 2 (see format under §4)

RM      PHI,    <WA,WPHI,WSP>      ;Areg=PHIreg=D-RAM(PHI) WFreg=100H (sinus)
RM      DPHI,   <WB>               ;Breg=D-RAM(DPHI)
RM      AMP,    <WXY,WSP>          ;X=sin(PHI) Y=AMP mix updated
RADD    PHI,    <WM>               ;D-RAM(PHI)= Areg + Breg (PHI+DPHI)
RSP                                ;NOP to wait 3 cycles from WXY
                  ,<WACC>          ;accumulate AMP x sin(PHI) through mix
;This statement will fill the rest of the algorithm with no-ops until the requested
;size (32 or [64]) as defined by parameter SIZE
FIN
```

### Example 2: External Memory Wave Oscillator (250 ns access time)

```
PHI=0                      ;assign phase to address 0
WF=1                       ;assign wave to address 1 (see format under §4), E and final wave are
                           ;don't care
DPHI=2                     ;assign phase angle to address 2
AMP=3                      ;assign AMP and mix to address 3 (see format under §4)

RM      WF,     <WWF>              ;WFreg=wave select
RM      PHI,    <WA,WPHI>          ;Areg=PHIreg=D-RAM(PHI)
RSP                                ;nops to ensure 7 cycles from WPHI to WXY
RSP
RSP
RSP
RSP
RM      DPHI,   <WB>               ;Breg=D-RAM(DPHI)
RADD    PHI,    <WM>               ;D-RAM(PHI)= Areg + Breg (PHI+DPHI)
RM      AMP,    <WXY,WSP>          ;X=wave(PHI) Y=AMP mix update
RSP                                ;wait multiplier result
RSP                                ;nop to wait multiply
RSP                                ;nop to wait multiply
RSP                   ,<WACC>      ;accumulate
                                   ;fill with nops til size of algorithm
FIN
```

---

## Polyphony Notes

**Note:** The nop area can in most cases be used to implement modulations or overlap with the next computation.

As it can be seen from the previous examples, a single algorithm can implement up to 7 [15] sinus oscillators, which means that the polyphony of the SAM is 112 [240] simple sinus oscillators. Also, the polyphony achieved when accessing external memory greatly depends upon the external memory access time. With the example given (250 ns access time), the wave polyphony is from 3 [7] for one algorithm, for a total SAM wave polyphony of 48 [112]. Using 100ns external memory will increase this polyphony to 80 [160].

With the SAM features described, one can implement all types of formulae synthesis, i.e. synthesis which can be defined with mathematical expressions involving multiply and add without limit conditions.

For example, the synthesis S=A sin(w₁ + M sin w₂t) is easily achieved (FM). Also possible are multiple FM, FM feedback, phase modulation, amplitude modulation, ring modulation, exponential envelope generation, digital filtering, etc. etc. A corresponding algorithm library is provided with the SAM toolkit. However, until now, synthesis invoking conditions, like sampling (loop when WF=final WF), MACROSampling™, formant synthesis (trigger a function with an excitation), envelope generation with a given envelope shape, etc. are not described. The purpose of the next chapter is to describe the SAM micro-instructions allowing such synthesis.

---

## 8. Micro-Instructions for Conditional Synthesis: WA WSP and WM WSP

### 8-1. The CARRY from the A+B Adder

The carry of the adder has a different meaning depending on the sign of the data in the B register:

- **B register positive (bit 18 from Breg=0)**
  Then the CARRY is the normal carry of an addition (bit 19 of the result of the A+B addition in twos complement).

- **B register negative (bit 18 from Breg=1)**
  Then the CARRY is the complement of the sign bit of the addition (complement of bit 18 of A+B).

### Examples

| A | B | CARRY | A+B (RADD) |
|---|---|-------|------------|
| 1000H | 2000H | 0 | 3000H |
| 7FFFFH | 1 | 1 | 0 |
| 7FFFFH | 7FFFFH | 0 | 7FFFEH (-2) |
| 40000H | 7FFFFH | 1 | 3FFFFM |

This carry mechanism allows to deal with one half of a signal period.

### 8-2. The CLEARRQST and INTMOD Flip-Flops

This flip-flops are updated by WA and WA WSP and are tested by WM WSP, allowing conditional write to the D-RAM and corresponding interrupt request.

### 8-3. The WA WSP Micro-Instruction

WA (without WSP) always sets the CLEARRQST flip-flop and clears the INTMOD flip-flop.

WPHI WSP takes priority other WA WSP giving a normal WA

WA WSP expects an E|WAVE|finalWAVE format on the bus (see §4)

| WSP | CARRY | WF=finalWF | END | value loaded in A | CLEARRQST | INTMOD |
|-----|-------|------------|-----|-------------------|-----------|--------|
| no  | X     | X          | X   | 19 bits bus       | 1         | 0      |
| yes | 0     | X          | X   | 00000H            | 0         | 1      |
| yes | 1     | no         | X   | 00200H            | 0         | 1      |
| yes | 1     | yes        | 0   | 00000H            | 0         | 1      |
| yes | 1     | yes        | 1   | 00000H            | 1         | 1      |

### 8-4. The WM WSP Micro-Instruction

This micro-instruction allows to conditionally write to the D-RAM, according to CLEARRQST and CARRY. Also, an interrupt may be requested to the external processor, assuming that the interrupt mask bit at D-RAM address 15 is reset (see §4).

| CLEARRQST | CARRY | INTMOD | write bus to D-RAM | Request Interrupt |
|-----------|-------|--------|--------------------|--------------------|
| no        | X     | X      | no                 | no                 |
| yes       | 0     | 0      | yes                | no                 |
| yes       | 0     | 1      | yes                | yes                |
| yes       | 1     | X      | no                 | yes                |

### 8-5. Using WA WSP and WM WSP

#### 8-5-1. Increment of the Waveform Number at the End of a Period

This is used mainly for sampling, but can find other applications like slow linear ramps, etc. The following example assumes a 200 ns external sampling memory.

```
PHI=0                      ;period phase
WF=1                       ;initial and final sampling periods, E should be set if the AMP should
                           ;drop to 0 when WAVE=finalWAVE (see §4 for format)
DPHI=2                     ;frequency
AMP=3                      ;amplitude and mix

RM      WF,     <WWF>              ;WFreg=WF
RM      PHI,    <WA,WPHI>          ;Areg=PHIreg=PHI
                                   ;Breg=DPHI
RM      DPHI,   <WB>               ;PHI=PHI+DPHI
RADD    PHI,    <WM>               ;Breg=WF, A set to 0 if no carry or
RM      WF,     <WB,WA,WSP>        ;WF=finalWF, A set to 100H otherwise,
                                   ;CIrqst set if carry and WF=finalWF and
                                   ;end=1, INTMOD set.
RADD    WF,     <WM,clearB>        ;Breg=0, WF=WF/0 or WF+1
RADD            <WM,WSP>           ;AMP=0 and interrupt request if WF=finalWF
                                   ;and end=1
RM      AMP,    <WXY,WSP>          ;X=mem(WF,PHI) Y=AMP mix update
RSP                                ;wait multiply
RSP                                ;wait multiply
RSP                   ,<WACC>      ;accumulate
FIN
```

#### 8-5-2. One-Shot Wave-Form Period

It is often necessary to generate only one period or a portion of a wave-form period. This is useful for envelope generation, but also for synthesis like FOF, where the shape of the pattern generated should be independent of the repetition rate of the pattern. It is also useful for transients generation, like clicks, frequency transients, etc.

The next example shows the generation of a single sinus period; an interrupt to the main computer will be generated at the end of the period if the interrupt mask is resetted.

```
PHI1=0                     ;phase of sinus
DPHI1=1                    ;frequency of single period
AMP=2                      ;amplitude and mix

RM      PHI1,   <WA>               ;Areg=PHI2
RM      DPHI2,  <WB>               ;Breg=DPHI2
RADD    PHI2,   <WM>               ;PHI2=PHI2+DPHI2
RM      K,      <WA, clearB, WSP>  ;Areg=Breg=0 clrrqst=1 if cy=1
RADD    PHI1,   <WM,WSP>           ;clear PHI1 if clrrqst (once every period
                                   ;of PHI2)
; one shot period
RM      PHI1,   <WA, WPHI, WSP>    ;Areg=PHIreg=PHI1 Clrrqst=1
RM            , <WB>               ;Breg=DPHI1
RADD    DPHI1,  <WB>               ;write to D-RAM only if carry=0
RADD    PHI1,   <WM,WSP>           ;X=sin(PHI1) Y=amp Clrrqst=1
RM      AMP,    <WXY,WSP>          ;accumulate
RSP                                ;wait multiply
RSP                                ;wait multiply
RSP                   ,<WACC>      ;accumulate
FIN
```

More examples can be found in the algorithm library (SAM toolkit)

---

## 9. Writing to an External RAM Sampling Memory

There is no direct way for the main computer to read or write from/to an external memory connected to the SAM. However, defining an algorithm which will read the external memory in a block basis into the D-RAM is easy, and the computer can then read the D-RAM.

Writing data to an external RAM is done also with a specific algorithm, but specific micro-instructions are needed to control the memory signals as follows:

```
PHI=0                      ;lower part of memory address (upper 9 bits)
WF=1                       ;0(2)|WF(8)|X(9) upper part of memory address
DATA=2                     ;data to be written in memory

RM      WF,     <WWF>              ;activate memory chip select (WCS/)
RM      PHI,    <WPHI>             ;PHIreg=PHI
RM      DATA,   <WXY>              ;prepare to output data on WD pins
RSP             <ClearB,WSP>       ;WOE/=1 (disable outputs from external mem)
RSP             <ClearB,WSP>       ;WOE/=1, apply data to WD pins
RSP             <ClearB>           ;WOE/=0 (write pulse)
RSP             <ClearB,WSP>       ;WOE/=1
RSP             <ClearB,WSP>       ;WOE/=1 data still applied
                                   ;data removed
                                   ;WOE/=0 (back to normal operation)
```

The complete write cycle takes 443 ns. This algorithm can be improved to transfer several words and generate an interrupt at transfer end.

---

## 10. SAM Micro-Processor Interface

**ON CHIP SYSTEMS**

### 10-1. Control Byte

The control byte is write only. It is updated when a computer write occurs to the chip with A2=1.

**Control byte format:**

```
| X | X | X | X | SSR | IDL | SEL | WR |
```

| Field | Description |
|-------|-------------|
| **SSR** | 0- Select 44.1 kHz sampling rate |
|         | 1- Select 22.05 kHz sampling rate |
| **IDL** | 0- Normal operation |
|         | 1- Force all 16 slots to idle algorithm |
| **SEL** | 0- Select access to D-RAM |
|         | 1- Select access to A-RAM |
| **WR**  | 0- Request a read from D-RAM or A-RAM |
|         | 1- Request a write to D-RAM or A-RAM |

**Note:** When IDL is changed from 0 to 1, it takes up to 1.5 /us [3 /us] for the SAM to actually go into an idle state.

### 10-2. Address, Data and Interrupt Bytes

A write to the chip with A2=A1=A0=0 selects the internal RAM address

| V3 | V2 | V1 | V0 | MAD3 | MAD2 | MAD1 | MAD0 | → D-RAM |
|----|----|----|----|----|----|----|----|----|
| AL2 | AL1 | AL0 | PC4 | PC3 | PC2 | PC1 | PC0 | → A-RAM with SSR=0 |
| AL2 | AL1 | PC5 | PC4 | PC3 | PC2 | PC1 | PC0 | → A-RAM with SSR=1 |

| Field | Description |
|-------|-------------|
| **V:** | slot number |
| **MAD:** | D-RAM address inside slot (MAD from micro-instruction) |
| **AL:** | algorithm number |
| **PC:** | micro-instruction location counter inside algorithm |

Write or read to the chip with A2=0 and A1A0 <> 00 allows to write/read data td/from the internal RAMs as follows:

```
|    A1=1 A0=1    |       A1=1 A0=0       |    A1=0 A0=1    |
| X | X | X | X | X | B18|B17|B16|B15|B14|B13|B12|B11|B10|B9 | B8 | B7 | B6 | B5 | B4 | B3 | B2 | B1 | B0 |
```

Write or read to D-RAM

```
|    A1=1 A0=1    |       A1=1 A0=0       |    A1=0 A0=1    |
| X | X | X | X | X | X |I14|I13|I12|I11|I10|I9 | I8 | I7 | I6 | I5 | I4 | I3 | I2 | I1 | I0 |
```

Write or read to A-RAM

**B** is the 19 bits word of the D-RAM, **I** is the 15 bits micro-instruction word of the A-RAM.

### 10-3. Operation

**ON CHIP SYSTEMS**

**Warning:** before writing to the A-RAM, to avoid transient sounds at the output, care must be taken that the corresponding algorithm is not active, which can be done by setting the SAM idle in the control byte or making sure that all slots that reference the algorithm are in the idle state.

#### 10-3-1. Writing to the D-RAM or the A-RAM

1. Write address
2. Write data
3. Write control byte with WR=1
4. Allow 1.5 /us [3 /us] recovery time for the write to perform

**Notes:**
The data bytes will retain their value from one write to another, which means that to write repetitive data, the write data step can be omitted.
Step 4 can be omitted if the SAM is in the idle state in the control byte.

#### 10-3-2. Reading the D-RAM or the A-RAM

1. Write address
2. Write control byte with WR=0
3. Wait 1.5 /us [3 /us] for the read to perform
4. Read the data bytes

**Notes:**
X portions of the data bytes will give indeterminate results
Step 3 can be omitted if the SAM was in the idle state in the control byte

### 10-4. Interrupt Byte

The WM WSP micro-instruction can generate an interrupt (see §8-4). The processor can then read which memory location of the D-RAM caused the interrupt by reading the SAM with A2=A1=A0=0.

```
| V3 | V2 | V1 | V0 | MAD3 | MAD2 | MAD1 | MAD0 | → D-RAM |
```

This will not reset the interrupt cause. The interrupt cause can be removed by changing the corresponding slot parameters in the D-RAM or masking the interrupt of the corresponding slot (address V|15).

In any case, once the interrupt cause has been removed, a dummy read interrupt should be issued to remove a possible pending interrupt with the same cause:

1. read interrupt byte
2. remove interrupt cause
3. read interrupt byte and discard result

---

## Figure 1: SAM Simplified Diagram (Data Path)


```
    ┌───┐     ┌───┐
    │ A │<─WA │ B │<─WB
    └───┘     └───┘
      │         │
      └───┬─────┘
        Adder <───RADD
          │
         MAD─────┐
                 │
    ┌───┐  ┌────┐│   ┌───┐
    │WWF│  │WPHI││   │WXY│      RP
    └───┘  └────┘│   └───┘       │
      │      │   │     │         │
     WF     PHI  │    X  Y       │
      │      │   │     └──┬──────┘
      └──────┴───┘        │
                    ┌─────┴─────┐
    Sinus/Ramp ROM  │ Multiplier│
    External RAM/ROM└───────────┘
                          │
                     ┌────┴────┐
                     │    P    │────LACC───┐
                     │         │           │
                     │    P    │────RACC───┤    SR
                     └─────────┘           │
                                      ┌────┴────┐
                                      │  WACO   │
                                      └─────────┘
                                      
      ┌───┐       ┌───┐                            ┌───────┐
      │ A │─WA    │ B │─WB                     V───│       │───WM
      └─┬─┘       └─┬─┘                      MAD───│  RAM  │
        └─────┬─────┘                              │       │───RM
          ┌───┴───┐                                └───┬───┘
          │ Adder │───RADD                             │
          └───┬───┘                                    │
══════════════╪════════════════════════════════════════╪══════════════════
    BUS       │                                        │
              │                                        │
    WWF      WPHI         WXY              RP                    WACC
     │        │            │               │                      │
     ▼        ▼            ▼               │                      ▼
  ┌────┐  ┌─────┐      ┌───┬───┐          /\              ┌──┐  ┌──────┐
  │ WF │  │ PHI │      │ X │ Y │─────────/  \─────────┬───│L │──│ LACC │──┐
  └──┬─┘  └──┬──┘      └───┴───┘        / RP \        │   └──┘  └──────┘  │
     │       │             │            \    /        │                  │  ┌────┐
     │       │             ▼             \  /         │   ┌──┐  ┌──────┐ ├──│ SR │
     │       │     ┌────────────┐         \/          └───│R │──│ RACC │─┘  └────┘
     │       └─────┤ Multiplier ├─────────┘               └──┘  └──────┘
     │             └────────────┘
     ▼                   │
┌────────────────┐       │
│ Sinus/Ramp ROM │───────┘
│External RAM/ROM│
└────────────────┘                                      
```

**Bus Widths:**
- 19 bits emitters: D-RAM (RM), Adder (RADD), Multiplier (RP)
- 19 bits receivers: D-RAM (WM), Areg (WA), Breg (WB)
- 12 bits receivers: PHIreg (WPHI), Yreg (WXY)
- 9 bits receiver: WFreg (WWF)
- 3 bits receivers: BL and BR, right and left attenuators (WXY WSP)

---

## Appendix I - Sinus, Ramps and Constants Specifications

"PHI" relates to the 12 upper bits of the phase considered as an unsigned integer (0 to 4095)

"X" relates to the corresponding input of the multiplier, considered as a fractional number between -1 and 1-2⁻¹¹.

Then the sinus is defined as:

```
X = .71875 sin ( (PI/2048) x PHI + PI/4096)
```

The ramp "PHI" is defined as:

| Range          | Formula          |
|----------------|------------------|
| 0<=PHI<2048    | X = PHI/2048     |
| 2048<=PHI<4095 | X = PHI/2048 - 2 |

The ramp "2 x PHI" is defined as:

| Range          | Formula          |
|----------------|------------------|
| 0<=PHI<1024    | X = PHI/1024     |
| 1024<=PHI<3072 | X = PHI/1024 - 2 |
| 3072<=PHI<4095 | X = PHI/1024 - 4 |

The ramp "PHI/2" is defined as:

| Range          | Formula          |
|----------------|------------------|
| 0<=PHI<2048    | X = PHI/4096     |
| 2048<=PHI<4095 | X = PHI/4096 - 1 |

The constants are derived from the MAD part of the micro-instruction, as defined in the corresponding fields of RM(MAD), WM(MAD), AD(MAD) as follows:

| MAD  | corresponding X value |
|------|-----------------------|
| 0000 | .0004883              |
| 0001 | .06299                |
| 0010 | .12549                |
| 0011 | .18799                |
| 0100 | .25049                |
| 0101 | .31299                |
| 0110 | .37549                |
| 0111 | .43799                |
| 1000 | .50049                |
| 1001 | .56299                |
| 1010 | .62549                |
| 1011 | .68799                |
| 1100 | .75049                |
| 1101 | .81299                |
| 1110 | .87549                |
| 1111 | .93799                |

---

## Appendix II - Summary of Instructions and D-RAM Formats

### Emitters:

| Emitter  | Description                                 |
|----------|---------------------------------------------|
| **RM**   | put the content of D-RAM [MAD] on the bus   |
| **RADD** | put the result of the adder on the bus      |
| **RP**   | put the result of the multiplier on the bus |
| **RSP**  | nop, put nothing on the bus                 |

### Receivers:

| Receiver | Description                                                                                                                   |
|----------|-------------------------------------------------------------------------------------------------------------------------------|
| **WA**   | write the bus to A (19 bits). WA (without WSP) always sets the internals CLEARRQST flip-flop and clears the INTMOD flip-flop. |
| **WB**   | write the bus to B (19 bits)                                                                                                  |
| **WM**   | write the bus to D-RAM[MAD]                                                                                                   |
| **WWF**  | write the bus to WF (9 bits, B9-B17)                                                                                          |
| **WPHI** | write the bus to PHI (12 leftmost bits, B7-B18)                                                                               |
| **WXY**  | write the bus to Y (12 leftmost bits), write the WF data to X                                                                 |

### Modifier:

| Modifier | Description |
|----------|-------------|
| **WSP** | modifier used in combination with WPHI, WXY, WA, WM |

### Modified Receivers:

| Receiver | Modification                                                                                                        |
|----------|---------------------------------------------------------------------------------------------------------------------|
| **WPHI** | WSP: PHI loaded but WF forced to 1 (select internal sinus). WPHI WSP takes priority other WA WSP giving a normal WA |
| **WXY**  | WSP: Y and MIX loaded                                                                                               |
| **WA**   | WSP: expects an E\|WAVE\|finalWAVE format on the bus (see §4)                                                       |

### WA WSP Truth Table:

| WSP | CARRY | WF=finalWF | END | value loaded in A | CLEARRQST | INTMOD |
|-----|-------|------------|-----|-------------------|-----------|--------|
| no  | X     | X          | X   | 19 bits bus       | 1         | 0      |
| yes | 0     | X          | X   | 00000H            | 0         | 1      |
| yes | 1     | no         | X   | 00200H            | 0         | 1      |
| yes | 1     | yes        | 0   | 00000H            | 0         | 1      |
| yes | 1     | yes        | 1   | 00000H            | 1         | 1      |

### WM WSP Truth Table:

| CLEARRQST | CARRY | INTMOD | write bus to D-RAM | Request Interrupt |
|-----------|-------|--------|--------------------|-------------------|
| no        | X     | X      | no                 | no                |
| yes       | 0     | 0      | yes                | no                |
| yes       | 0     | 1      | yes                | yes               |
| yes       | 1     | X      | no                 | yes               |

### Special Actions:

| Action     | Description                                         |
|------------|-----------------------------------------------------|
| **clearB** | write 0 in B, used also to write in external memory |
| **WACC**   | send the multiplier result to final accumulator     |

---

## D-RAM Formats:

```
| 18 | 17 | 16 | 15 | 14 | 13 | 12 | 11 | 10 | 9  | 8  | 7  | 6  | 5  | 4  | 3  | 2  | 1  | 0  |
|----|----|----|----|----|----|----|----|----|----|----|----|----|----|----|----|----|----|----|
|                          PHI                                                                 |
|            PHIWF                                |  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX  |
|           PHIREG                                          | XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX  |
|             DPHI                                                                             |
|       AMP                                                 | XX | MIXL         | MIXR         |
| E  |     WAVE                                        |  FINAL WAVE                           |
|    | 0  | ext mem wave                               |                                       |
| .  | 1  | R  | I  | SEL     | Z  | XXXXXXXXXXXXXXXXX |                                       |
|  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX | I  | ALG      | H | XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX |
```

---

## Appendix III - Micro-Instruction Coding

```
| I14 | I13 | I12 | I11 | I10 | I9 | I8 | I7 | I6 | I5 | I4 | I3 | I2 | I1 | I0 |
|-----|-----|-----|-----|-----|----|----|----|----|----|----|----|----|----|----|
|         MAD           | 00- RM   |WSP | WA | WB | WM |WPHI|WXY |clea|WWF |WACC|
|                       | 01- RADD |    |    |    |    |    |    |rB  |    |    |
|                       | 10- RP   |    |    |    |    |    |    |    |    |    |
|                       | 11- RSP  |    |    |    |    |    |    |    |    |    |
```

All Wxx and clearB are active 0 except WSP

---

## Appendix IV - Implementation Notes

### Carry Calculation

The carry logic described in Section 8-1 is critical for envelope processing. The key insight is that the "complement" when B is negative must be implemented correctly:

```cpp
bool b_neg = BIT(slot.b, 18);
uint32_t result = (slot.a + slot.b) & MASK19;
bool carry;
if (!b_neg)
    carry = ((uint64_t)slot.a + slot.b) > MASK19;  // bit 19 overflow
else
    carry = !BIT(result, 18);  // COMPLEMENT of bit 18
```

**Why this matters for envelopes:**

When B contains a negative delta (for amplitude decay):
- `carry = 1` while the result (A+B) is still positive
- `carry = 0` when the result goes negative (crossed zero)

The WM WSP truth table shows that writes are blocked when `carry = 1`. So during decay:
- While amplitude > 0: carry=1, WM WSP blocks the write, envelope keeps decaying
- When amplitude crosses 0: carry=0, WM WSP allows the final write, stopping the decay

**Common bug:** Implementing carry as simply `BIT(result, 18)` without the complement when B is negative will break note release - notes will cut off abruptly instead of decaying smoothly.

---

