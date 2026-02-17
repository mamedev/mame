# **src/mame/pc**

Generic folder with MAME drivers for IBM PC, PC compatibles, or PC-based arcade hardware.

## POST error codes

ISA debug port $80, `wpiset 0x80,1,w,1,{printf "%02x",wpdata;g}`.
- [Phoenix BIOS enum in pcipc](pcipc.cpp#L100)
- [Phoenix v4.0 BIOS enum in pcipc](pcipc.cpp#L223)
- [Award BIOS enum in pcipc](pcipc.cpp#L395)

### Quadtel BIOS

| code | meaning |
|-|-|
| 02 | Flag test |
| 04 | Register test |
| 06 | System hardware initialization |
| 08 | Initialize chip set registers |
| 0A | BIOS ROM checksum |
| 0C | DMA page register test |
| 0E | 8254 timer test |
| 10 | 8254 timer initialization |
| 12 | 8237 DMA controller test |
| 14 | 8237 DMA initialization |
| 16 | Initialize 8259/Reset coprocessor |
| 18 | 8259 interrupt controller test |
| 1A | Memory refresh test |
| 1C | Base 64(0?)KB address test |
| 1E | Base 64(0?)KB memory test |
| 20 | Base 64(0?)KB test (upper 16 bits) |
| 22 | 8742 Keyboard self test |
| 24 | MC146818 CMOS test |
| 26 | Start first protected mode test |
| 28 | Memory Sizing test |
| 2A | Autosize memory chips |
| 2C | Chip interleave enable test |
| 2E | First protected mode test exit |
| 30 | Unexpected shutdown |
| 32 | System board memory size |
| 34 | Relocate shadow RAM if configured |
| 36 | Configure EMS system |
| 38 | Configure wait states |
