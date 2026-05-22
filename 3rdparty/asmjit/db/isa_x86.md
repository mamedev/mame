X86 ISA - JSON Database Documentation
-------------------------------------

This file provides a documentation of isa_x86.json file, which contains a compact X86 and X86_64 instruction database. The database has been created from several manuals from Intel and AMD, which describes the encoding of instructions and other properties. The database tries to keep the instruction and opcode formats compatible with the database, however, in cases where applicable it tries to simplify it (for example merging instructions that all use either XMM, YMM, or ZMM registers, etc...).

## Instruction Category

Each set of instructions start with category group, which looks like the following:

```json
{"category": "<LIST_OF_CATEGORIES>", "ext": "<LIST_OF_EXTENSIONS>", /* flags, */ "instructions": [
  /* (instruction records) */
]}
```

Fields:

  - `category` - describes a single or multiple categories this instruction belongs to. In general the database is written in a way to provide "baseline" and "extended" categories, so have the following categories used to distinguish between different set of instructions:
    - `GP` - General purpose instructions.
    - `GP_EXT` - Extensions to general purpose instructions (such as BMI, POPCNT, etc...).
    - `MMX` - MMX and 3DNOW instructions (in general instructions using MM registers).
    - `SSE` - SSE to SSE4.2 instructions, including other extensions that don't use VEX/EVEX prefixes.
    - `AVX` - AVX, AVX2, and other instructions that use SIMD registers (XMM, YMM) and VEX prefix.
    - `AVX512` - AVX512 instructions.

  - `ext` (optional) - describes extensions required to execute the instruction. When multiple extensions are in the list it means ALL of them must be available. For example `AESNI` means that `AESNI` extension is required, `AVX AESNI` means that both `AVX` and `AESNI` extensions are required.

  - `deprecated` (optional) - deprecation flag (either `true` or `false`), by default instructions are not deprecated.

  - `volatile` (optional) - volatility flag (either `true` or `false`), by default instructions are not volatile (note that volatile flag is used for code generators and in general it says that the instruction should not be reordered and nothing above it should move below and vice versa).

  - `instructions` - array of instruction records, one line per instruction.

## Instruction Record

A single instruction record looks like the following:

```json
{"<ARCH>": "<INSTRUCTION_SIGNATURE>", "op": "[<ENCODING_OPS>] <ENCODING_DATA>" /*, <ADDITIONAL_DATA>*/ }
```

### Architecture Field

`<ARCH>` field describes the architecture and optionally `APX_F` promoted instruction:

  - `any` - The instruction can run in both 32-bit and 64-bit mode and shares the encoding.
  - `x86` - The instruction can only run in 32-bit mode. This in general may be an indication that the instruction was deprecated in 64-bit mode or that the instruction has a different signature in 64-bit mode (some instructions require 32-bit registers in 32-bit mode and 64-bit registers in 64-bit mode, for example)
  - `x64` - The instruction can only run in 64-bit mode.
  - `apx` - The instruction can only run in 64-bit mode with `APX_F` extension present. This notation is only used for `APX_F` instructions that provide alternative `EVEX` encoding to existing instructions, but not new `APX_F` only instructions. For example `jmpabs` instruction is a new `APX_F` instruction, thus the arch notation it uses is `x64`.

Explanation: The database uses this "variable" field name to save space and to avoid defining the architecture elsewhere.

### Instruction Signature

Instruction signature is composed of the following components:

```
[<PREFIX>|<OPTIONS>] <INSTRUCTION_NAME>|<ALIASES> <OPERANDS>
```

  - `[<PREFIX>|<OPTIONS>]` (optional) - prefixes and other options:
    - `[bnd]`       - instruction supports `bnd` prefix (deprecated).
    - `[lock]`      - instruction supports `lock` prefix.
    - `[xacquire]`  - instruction supports `xacquire` prefix.
    - `[xacqrel]`   - instruction supports both `xacquire` and `xrelease` prefixes.
    - `[xrelease]`  - instruction supports `xrelease` prefix.
    - `[rep]`       - instruction supports `rep` prefix (or `repe`).
    - `[repne]`     - instruction supports `repne` prefix.
    - `[repIgnore]` - instruction supports `rep` prefix, which is ignored during execution (to support for example `rep ret`).

  - `<INSTRUCTION_NAME>|<ALIASES>` (required) - the name of the instruction possibly followed by aliases if the instruction has them
    - if the instruction has aliases, they will be recognized in AsmJit API (the API will provide aliases), but will not have a separate instruction identifier
    - for example `cmovz|cmove` is a `cmovz` instruction that has a `cmove` alias.

  - `<OPERANDS>` (optional):
    - When an instruction has multiple operands, they are separated by comma.
      - Registers:
        - `r8` - 8-bit general purpose register.
        - `r16` - 16-bit general purpose register.
        - `r32` - 32-bit general purpose register.
        - `r64` - 64-bit general purpose register (x64 only).
        - `mm` - 64-bit MM register.
        - `xmm` - 128-bit XMM register (SSE).
        - `ymm` - 256-bit YMM register (AVX)
        - `zmm` - 512-bit ZMM register (AVX512/AVX10).
        - `k` - 64-bit K register (AVX512/AVX10).
        - `tmm` - TMM register (AMX).
        - `creg` - Control register.
        - `dreg` - Debug register.
        - `st(x)` - FPU register.
        - `rip` - Instruction pointer register (used during addressing).
        - `bnd` - Bounds register (deprecated).
      - Memory:
        - `mem` - Memory operand without size specified.
        - `m8-m512` - Memory operand of a specified size.
      - Immediate values
        - `imm4` - Signed or unsigned 4-bit immediate value (only used by instructions where the other 4 bits are used to encode additional register operand).
        - `imm8` - Signed or unsigned 8-bit immediate value.
        - `imm16` - Signed or unsigned 16-bit immediate value.
        - `imm32` - Signed or unsigned 32-bit immediate value.
        - `imm64` - Signed or unsigned 64-bit immediate value.
        - `imms8` - Signed 8-bit immediate value (instructions that sign-extend the immediate value to 32 or 64 bits).
        - `immu8` - Unsigned 8-bit immediate value (instructions that zero-extend the immediate value to 32 or 64 bits).
        - `immu16` - Unsigned 16-bit immediate value (instructions that zero-extend the immediate value to 32 or 64 bits).
        - `imms32` - Signed 32-bit immediate value (instructions that sign-extend the immediate value to 64 bits).
        - `immu32` - Unsigned 32-bit immediate value (instructions that zero-extend the immediate value to 64 bits).
      - Relative operands (addressing labels and relative locations in code):
        - `rel8` - 8-bit signed relative displacement.
        - `rel16` - 16-bit signed relative displacement.
        - `rel32` - 32-bit signed relative displacement.
    - Combining:
      - Register and memory operands can be combined, for example `r8/m8` means either 8-bit register or 8-bit memory operand
    - Read/Write access:
      - The first operand must be decorated by access in a form `[r|w|x]: <OPERAND>`:
        - `R:` - read access.
        - `W:` - write access (overwrites the whole register).
        - `w:` - write access (only overwrites an addressed part of the register - 8-bit and 16-bit GP registers use this access).
        - `X:` - read/write access (the final write operation overwrites the whole register).
        - `x:` - read/write access (the final write operation only overwrites the addressed part of the register - 16-bit GP access).
      - The access of the second operand and following operands is implied to be read-only, when not specified explicitly.
    - Grouping:
      - If the encoding used for multiple register combinations is the same, multiple instructions can be grouped into a single record.
      - `GP` Instructions:
        - `ry` - either `r32` or `r64`.
        - `my` - either `m32` or `m64`.
        - `rv` - either `r16`, `r32`, or `r64`.
        - `mv` - either `m16`, `m32`, or `m64`.
        - `immv` - either `imm16`, `imm32`, or `simm32`.
        - All registers must match, example: `add x:rv, rv/mv` - either all `r16/m16`, `r32/m32`, or `r64/m64`.
        - When `rv/mv/immv` is used, the instruction must use `66h` prefix (or 66h part of `VEX/EVEX`) when a 16-bit `r16/m16` is used, and `REX.W` (or `W` part of `VEX/EVEX`) when a 64-bit `r64/m64` is used. The immediate expands to `imm16` for instructions working with 16-bit registers, to `imm32` for instructions working with 32-bit registers, and to `simm32` for instructions working with 64-bit registers (32-bit immediate sign extended to 64 bits).
        - When `rv/mv` is used with `w` and `x` access option, it only applies to a 16-bit operation, wider operations would use `W` or `X` (this is an architectural constraint that the tool processing the database must be aware of)
        - Why `y` and `v`? `ry/my` and `rv/mv` appeared in initial manuals describing `APX_F` extension, so this database is using exactly this notation to group multiple instructions into a single entry.
      - `AVX` Instructions:
        - `xy/mxy`  - either `xmm/m128` or `ymm/m256` register operand.
        - All registers must match, example: `vfmadd132pd X:xy, xy, xy/mxy` - either all `xmm/m128` or all `ymm/m256`.
        - Why `xy/mxy`? This notation is not used by instruction manuals, but we have found it easy to use and understand.
      - `AVX512` and `AVX10` Instructions:
        - `xxx/mxxx`  - either `xmm[31:0]/m32`, `xmm[63:0]/m64`, or `xmm/m128` register operand.
        - `xxy/mxxy`  - either `xmm[63:0]/m64`, `xmm/m128`, or `ymm/m256` register operand.
        - `xyz/mxyz`  - either `xmm/m128`, `ymm/m256`, or `zmm/m512` register operand.
        - All registers must match, example: `vvfmadd132pd X:xyz {kz}, xyz, xyz/mxyz/b64 {er}` - either all `xmm/m128`, `ymm/m256`, or `zmm/m512`.
        - Embedded rounding `{er}` and `{sae}` are grouped - in AVX512 case only 512-bit operations can use `{er}/{sae}`; in AVX10.2 case both 256-bit and 512-bit operations can use `{er}/{sae}`, but not 128-bit operations - the assembler or the tool processing this data must be aware of this architectural constraint.
        - Why `xyz/mxyz`? This notation is not used by instruction manuals, but we have found it easy to use and understand.


