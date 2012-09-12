
/* Seibu 'COP' (co-processor)  protection

  there appear to be 3 revisions of this protection (based on the external rom)

  COPX-D1 - Seibu Cup Soccer / Olympic Soccer '92
          - Legionnaire
  COPX-D2 - Heated Barrel
          - Godzilla
          - SD Gundam Sangokushi Rainbow Tairiku Senki
          - Denjin Makai
          - Raiden 2
          - Raiden DX
          - Zero Team
  COPX-D3 - Raiden 2/DX New (V33 PCB version)
          - New Zero Team

  COPX / COPX-D2 based games appear to function in a similar way to each other
  while the games using COPX-D3 appears to access the protection device very
  differently to the others.

  As this is only the external rom it isn't confirmed that the actual protection
  devices are identical even where the external rom matches.

  Protection features include BCD math protection, 'command sequences', DMA.
  memory clearing etc.

  it is not confirmed which custom Seibu chip contains the actual co-processor,
  nor if the co-processor is a real MCU with internal code, or a custom designed
  'blitter' like device.

  I suspect that for the earlier games it's part of the COP300 or COP1000 chips,
  for the COPX-D3 based games it's probably inside the system controller SEI333

  the external COP rom is probably used as a lookup for maths operations.

  there should probably only be a single cop2_r / cop2_w function, the chip
  looks to be configurable via the table uploaded to
  0x432 / 0x434 / 0x438 / 0x43a / 0x43c, with 'macro' commands triggered via
  writes to 0x500.

  this simulation is incomplete

  COP TODO list:
  - collision detection, hitbox parameter is a complete mystery;
  - collision detection, unknown usage of reads at 0x582-0x588;
  - The RNG relies on the master CPU clock cycles, but without accurate waitstate support is
    basically impossible to have a 1:1 matchup. Seibu Cup Soccer Selection for instance should show the
    first match as Germany vs. USA at twilight, right now is Spain vs. Brazil at sunset.
  - (MANY other things that needs to be listed here)

  Protection information

ALL games using COPX-D/D2 upload a series of command tables, using the following upload pattern.
This happens ONCE at startup.

Each table is 8 words long, and the table upload offsets aren't always in sequential order, as
you can see from this example (cupsocs) it uploads in the following order

00 - 07, 08 - 0f, 10 - 17, 18 - 1f, 28 - 2f, 60 - 67, 80 - 87, 88 - 8f
90 - 97, 98 - 9f, 20 - 27, 30 - 37, 38 - 3f, 40 - 47, 48 - 4f, 68 - 6f
c0 - c7, a0 - a7, a8 - af, b0 - b7, b8 - bf, c8 - cf, d0 - d7, d8 - df
e0 - e7, e8 - ef, 50 - 57, 58 - 5f, 78 - 7f, f0 - f7

table data is never overwritten, and in this case no data is uploaded
in the 70-77 or f8 - ff region.

It is assumed that the data written before each part of the table is associated
with that table.

000620:  write data 0205 at offset 003c <- 'trigger' associated with this table
000624:  write data 0006 at offset 0038 <- 'unknown1 (4-bit)'
000628:  write data ffeb at offset 003a <- 'unknown2'

000632:  write data 0000 at offset 0034 <- 'table offset'
000638:  write data 0188 at offset 0032 <- '12-bit data' for this offset
000632:  write data 0001 at offset 0034
000638:  write data 0282 at offset 0032
000632:  write data 0002 at offset 0034
000638:  write data 0082 at offset 0032
000632:  write data 0003 at offset 0034
000638:  write data 0b8e at offset 0032
000632:  write data 0004 at offset 0034
000638:  write data 098e at offset 0032
000632:  write data 0005 at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 0006 at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 0007 at offset 0034
000638:  write data 0000 at offset 0032
--------------------------------------------
000620:  write data 0905 at offset 003c
000624:  write data 0006 at offset 0038
000628:  write data fbfb at offset 003a

000632:  write data 0008 at offset 0034
000638:  write data 0194 at offset 0032
000632:  write data 0009 at offset 0034
000638:  write data 0288 at offset 0032
000632:  write data 000a at offset 0034
000638:  write data 0088 at offset 0032
000632:  write data 000b at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 000c at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 000d at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 000e at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 000f at offset 0034
000638:  write data 0000 at offset 0032
--------------------------------------------
000620:  write data 138e at offset 003c
000624:  write data 0005 at offset 0038
000628:  write data bf7f at offset 003a

000632:  write data 0010 at offset 0034
000638:  write data 0984 at offset 0032
000632:  write data 0011 at offset 0034
000638:  write data 0aa4 at offset 0032
000632:  write data 0012 at offset 0034
000638:  write data 0d82 at offset 0032
000632:  write data 0013 at offset 0034
000638:  write data 0aa2 at offset 0032
000632:  write data 0014 at offset 0034
000638:  write data 039b at offset 0032
000632:  write data 0015 at offset 0034
000638:  write data 0b9a at offset 0032
000632:  write data 0016 at offset 0034
000638:  write data 0b9a at offset 0032
000632:  write data 0017 at offset 0034
000638:  write data 0a9a at offset 0032
--------------------------------------------
000620:  write data 1905 at offset 003c
000624:  write data 0006 at offset 0038
000628:  write data fbfb at offset 003a

000632:  write data 0018 at offset 0034
000638:  write data 0994 at offset 0032
000632:  write data 0019 at offset 0034
000638:  write data 0a88 at offset 0032
000632:  write data 001a at offset 0034
000638:  write data 0088 at offset 0032
000632:  write data 001b at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 001c at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 001d at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 001e at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 001f at offset 0034
000638:  write data 0000 at offset 0032
--------------------------------------------
000620:  write data 2a05 at offset 003c
000624:  write data 0006 at offset 0038
000628:  write data ebeb at offset 003a

000632:  write data 0028 at offset 0034
000638:  write data 09af at offset 0032
000632:  write data 0029 at offset 0034
000638:  write data 0a82 at offset 0032
000632:  write data 002a at offset 0034
000638:  write data 0082 at offset 0032
000632:  write data 002b at offset 0034
000638:  write data 0a8f at offset 0032
000632:  write data 002c at offset 0034
000638:  write data 018e at offset 0032
000632:  write data 002d at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 002e at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 002f at offset 0034
000638:  write data 0000 at offset 0032
--------------------------------------------
000620:  write data 6200 at offset 003c
000624:  write data 0008 at offset 0038
000628:  write data f3e7 at offset 003a

000632:  write data 0060 at offset 0034
000638:  write data 03a0 at offset 0032
000632:  write data 0061 at offset 0034
000638:  write data 03a6 at offset 0032
000632:  write data 0062 at offset 0034
000638:  write data 0380 at offset 0032
000632:  write data 0063 at offset 0034
000638:  write data 0aa0 at offset 0032
000632:  write data 0064 at offset 0034
000638:  write data 02a6 at offset 0032
000632:  write data 0065 at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 0066 at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 0067 at offset 0034
000638:  write data 0000 at offset 0032
--------------------------------------------
000620:  write data 8100 at offset 003c
000624:  write data 0007 at offset 0038
000628:  write data fdfb at offset 003a

000632:  write data 0080 at offset 0034
000638:  write data 0b9a at offset 0032
000632:  write data 0081 at offset 0034
000638:  write data 0b88 at offset 0032
000632:  write data 0082 at offset 0034
000638:  write data 0888 at offset 0032
000632:  write data 0083 at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 0084 at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 0085 at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 0086 at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 0087 at offset 0034
000638:  write data 0000 at offset 0032
--------------------------------------------
000620:  write data 8900 at offset 003c
000624:  write data 0007 at offset 0038
000628:  write data fdfb at offset 003a

000632:  write data 0088 at offset 0034
000638:  write data 0b9a at offset 0032
000632:  write data 0089 at offset 0034
000638:  write data 0b8a at offset 0032
000632:  write data 008a at offset 0034
000638:  write data 088a at offset 0032
000632:  write data 008b at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 008c at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 008d at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 008e at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 008f at offset 0034
000638:  write data 0000 at offset 0032
--------------------------------------------
000620:  write data 9180 at offset 003c
000624:  write data 0007 at offset 0038
000628:  write data f8f7 at offset 003a

000632:  write data 0090 at offset 0034
000638:  write data 0b80 at offset 0032
000632:  write data 0091 at offset 0034
000638:  write data 0b94 at offset 0032
000632:  write data 0092 at offset 0034
000638:  write data 0b94 at offset 0032
000632:  write data 0093 at offset 0034
000638:  write data 0894 at offset 0032
000632:  write data 0094 at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 0095 at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 0096 at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 0097 at offset 0034
000638:  write data 0000 at offset 0032
--------------------------------------------
000620:  write data 9980 at offset 003c
000624:  write data 0007 at offset 0038
000628:  write data f8f7 at offset 003a

000632:  write data 0098 at offset 0034
000638:  write data 0b80 at offset 0032
000632:  write data 0099 at offset 0034
000638:  write data 0b94 at offset 0032
000632:  write data 009a at offset 0034
000638:  write data 0b94 at offset 0032
000632:  write data 009b at offset 0034
000638:  write data 0896 at offset 0032
000632:  write data 009c at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 009d at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 009e at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 009f at offset 0034
000638:  write data 0000 at offset 0032
--------------------------------------------
000620:  write data 2288 at offset 003c
000624:  write data 0005 at offset 0038
000628:  write data f5df at offset 003a

000632:  write data 0020 at offset 0034
000638:  write data 0f8a at offset 0032
000632:  write data 0021 at offset 0034
000638:  write data 0b8a at offset 0032
000632:  write data 0022 at offset 0034
000638:  write data 0388 at offset 0032
000632:  write data 0023 at offset 0034
000638:  write data 0b9a at offset 0032
000632:  write data 0024 at offset 0034
000638:  write data 0b9a at offset 0032
000632:  write data 0025 at offset 0034
000638:  write data 0a9a at offset 0032
000632:  write data 0026 at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 0027 at offset 0034
000638:  write data 0000 at offset 0032
--------------------------------------------
000620:  write data 338e at offset 003c
000624:  write data 0005 at offset 0038
000628:  write data bf7f at offset 003a

000632:  write data 0030 at offset 0034
000638:  write data 0984 at offset 0032
000632:  write data 0031 at offset 0034
000638:  write data 0aa4 at offset 0032
000632:  write data 0032 at offset 0034
000638:  write data 0d82 at offset 0032
000632:  write data 0033 at offset 0034
000638:  write data 0aa2 at offset 0032
000632:  write data 0034 at offset 0034
000638:  write data 039c at offset 0032
000632:  write data 0035 at offset 0034
000638:  write data 0b9c at offset 0032
000632:  write data 0036 at offset 0034
000638:  write data 0b9c at offset 0032
000632:  write data 0037 at offset 0034
000638:  write data 0a9a at offset 0032
--------------------------------------------
000620:  write data 3bb0 at offset 003c
000624:  write data 0004 at offset 0038
000628:  write data 007f at offset 003a

000632:  write data 0038 at offset 0034
000638:  write data 0f9c at offset 0032
000632:  write data 0039 at offset 0034
000638:  write data 0b9c at offset 0032
000632:  write data 003a at offset 0034
000638:  write data 0b9c at offset 0032
000632:  write data 003b at offset 0034
000638:  write data 0b9c at offset 0032
000632:  write data 003c at offset 0034
000638:  write data 0b9c at offset 0032
000632:  write data 003d at offset 0034
000638:  write data 0b9c at offset 0032
000632:  write data 003e at offset 0034
000638:  write data 0b9c at offset 0032
000632:  write data 003f at offset 0034
000638:  write data 099c at offset 0032
--------------------------------------------
000620:  write data 42c2 at offset 003c
000624:  write data 0005 at offset 0038
000628:  write data fcdd at offset 003a

000632:  write data 0040 at offset 0034
000638:  write data 0f9a at offset 0032
000632:  write data 0041 at offset 0034
000638:  write data 0b9a at offset 0032
000632:  write data 0042 at offset 0034
000638:  write data 0b9c at offset 0032
000632:  write data 0043 at offset 0034
000638:  write data 0b9c at offset 0032
000632:  write data 0044 at offset 0034
000638:  write data 0b9c at offset 0032
000632:  write data 0045 at offset 0034
000638:  write data 029c at offset 0032
000632:  write data 0046 at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 0047 at offset 0034
000638:  write data 0000 at offset 0032
--------------------------------------------
000620:  write data 4aa0 at offset 003c
000624:  write data 0005 at offset 0038
000628:  write data fcdd at offset 003a

000632:  write data 0048 at offset 0034
000638:  write data 0f9a at offset 0032
000632:  write data 0049 at offset 0034
000638:  write data 0b9a at offset 0032
000632:  write data 004a at offset 0034
000638:  write data 0b9c at offset 0032
000632:  write data 004b at offset 0034
000638:  write data 0b9c at offset 0032
000632:  write data 004c at offset 0034
000638:  write data 0b9c at offset 0032
000632:  write data 004d at offset 0034
000638:  write data 099b at offset 0032
000632:  write data 004e at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 004f at offset 0034
000638:  write data 0000 at offset 0032
--------------------------------------------
000620:  write data 6880 at offset 003c
000624:  write data 000a at offset 0038
000628:  write data fff3 at offset 003a

000632:  write data 0068 at offset 0034
000638:  write data 0b80 at offset 0032
000632:  write data 0069 at offset 0034
000638:  write data 0ba0 at offset 0032
000632:  write data 006a at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 006b at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 006c at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 006d at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 006e at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 006f at offset 0034
000638:  write data 0000 at offset 0032
--------------------------------------------
000620:  write data c480 at offset 003c
000624:  write data 000a at offset 0038
000628:  write data ff00 at offset 003a

000632:  write data 00c0 at offset 0034
000638:  write data 0080 at offset 0032
000632:  write data 00c1 at offset 0034
000638:  write data 0882 at offset 0032
000632:  write data 00c2 at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 00c3 at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 00c4 at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 00c5 at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 00c6 at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 00c7 at offset 0034
000638:  write data 0000 at offset 0032
--------------------------------------------
000620:  write data a180 at offset 003c
000624:  write data 0000 at offset 0038
000628:  write data ffff at offset 003a

000632:  write data 00a0 at offset 0034
000638:  write data 0b80 at offset 0032
000632:  write data 00a1 at offset 0034
000638:  write data 0b82 at offset 0032
000632:  write data 00a2 at offset 0034
000638:  write data 0b84 at offset 0032
000632:  write data 00a3 at offset 0034
000638:  write data 0b86 at offset 0032
000632:  write data 00a4 at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 00a5 at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 00a6 at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 00a7 at offset 0034
000638:  write data 0000 at offset 0032
--------------------------------------------
000620:  write data a980 at offset 003c
000624:  write data 000f at offset 0038
000628:  write data ffff at offset 003a

000632:  write data 00a8 at offset 0034
000638:  write data 0ba0 at offset 0032
000632:  write data 00a9 at offset 0034
000638:  write data 0ba2 at offset 0032
000632:  write data 00aa at offset 0034
000638:  write data 0ba4 at offset 0032
000632:  write data 00ab at offset 0034
000638:  write data 0ba6 at offset 0032
000632:  write data 00ac at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 00ad at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 00ae at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 00af at offset 0034
000638:  write data 0000 at offset 0032
--------------------------------------------
000620:  write data b100 at offset 003c
000624:  write data 0009 at offset 0038
000628:  write data ffff at offset 003a

000632:  write data 00b0 at offset 0034
000638:  write data 0b40 at offset 0032
000632:  write data 00b1 at offset 0034
000638:  write data 0bc0 at offset 0032
000632:  write data 00b2 at offset 0034
000638:  write data 0bc2 at offset 0032
000632:  write data 00b3 at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 00b4 at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 00b5 at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 00b6 at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 00b7 at offset 0034
000638:  write data 0000 at offset 0032
--------------------------------------------
000620:  write data b900 at offset 003c
000624:  write data 0006 at offset 0038
000628:  write data ffff at offset 003a

000632:  write data 00b8 at offset 0034
000638:  write data 0b60 at offset 0032
000632:  write data 00b9 at offset 0034
000638:  write data 0be0 at offset 0032
000632:  write data 00ba at offset 0034
000638:  write data 0be2 at offset 0032
000632:  write data 00bb at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 00bc at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 00bd at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 00be at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 00bf at offset 0034
000638:  write data 0000 at offset 0032
--------------------------------------------
000620:  write data cb8f at offset 003c
000624:  write data 0005 at offset 0038
000628:  write data bf7f at offset 003a

000632:  write data 00c8 at offset 0034
000638:  write data 0984 at offset 0032
000632:  write data 00c9 at offset 0034
000638:  write data 0aa4 at offset 0032
000632:  write data 00ca at offset 0034
000638:  write data 0d82 at offset 0032
000632:  write data 00cb at offset 0034
000638:  write data 0aa2 at offset 0032
000632:  write data 00cc at offset 0034
000638:  write data 039b at offset 0032
000632:  write data 00cd at offset 0034
000638:  write data 0b9a at offset 0032
000632:  write data 00ce at offset 0034
000638:  write data 0b9a at offset 0032
000632:  write data 00cf at offset 0034
000638:  write data 0a9f at offset 0032
--------------------------------------------
000620:  write data d104 at offset 003c
000624:  write data 0005 at offset 0038
000628:  write data fffb at offset 003a

000632:  write data 00d0 at offset 0034
000638:  write data 0ac2 at offset 0032
000632:  write data 00d1 at offset 0034
000638:  write data 09e0 at offset 0032
000632:  write data 00d2 at offset 0034
000638:  write data 00a2 at offset 0032
000632:  write data 00d3 at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 00d4 at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 00d5 at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 00d6 at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 00d7 at offset 0034
000638:  write data 0000 at offset 0032
--------------------------------------------
000620:  write data dde5 at offset 003c
000624:  write data 0005 at offset 0038
000628:  write data 7ff7 at offset 003a

000632:  write data 00d8 at offset 0034
000638:  write data 0f80 at offset 0032
000632:  write data 00d9 at offset 0034
000638:  write data 0aa2 at offset 0032
000632:  write data 00da at offset 0034
000638:  write data 0984 at offset 0032
000632:  write data 00db at offset 0034
000638:  write data 00c2 at offset 0032
000632:  write data 00dc at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 00dd at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 00de at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 00df at offset 0034
000638:  write data 0000 at offset 0032
--------------------------------------------
000620:  write data e38e at offset 003c
000624:  write data 0005 at offset 0038
000628:  write data b07f at offset 003a

000632:  write data 00e0 at offset 0034
000638:  write data 0984 at offset 0032
000632:  write data 00e1 at offset 0034
000638:  write data 0ac4 at offset 0032
000632:  write data 00e2 at offset 0034
000638:  write data 0d82 at offset 0032
000632:  write data 00e3 at offset 0034
000638:  write data 0ac2 at offset 0032
000632:  write data 00e4 at offset 0034
000638:  write data 039b at offset 0032
000632:  write data 00e5 at offset 0034
000638:  write data 0b9a at offset 0032
000632:  write data 00e6 at offset 0034
000638:  write data 0b9a at offset 0032
000632:  write data 00e7 at offset 0034
000638:  write data 0a9a at offset 0032
--------------------------------------------
000620:  write data eb8e at offset 003c
000624:  write data 0005 at offset 0038
000628:  write data b07f at offset 003a

000632:  write data 00e8 at offset 0034
000638:  write data 0984 at offset 0032
000632:  write data 00e9 at offset 0034
000638:  write data 0ac4 at offset 0032
000632:  write data 00ea at offset 0034
000638:  write data 0d82 at offset 0032
000632:  write data 00eb at offset 0034
000638:  write data 0ac2 at offset 0032
000632:  write data 00ec at offset 0034
000638:  write data 039b at offset 0032
000632:  write data 00ed at offset 0034
000638:  write data 0b9a at offset 0032
000632:  write data 00ee at offset 0034
000638:  write data 0b9a at offset 0032
000632:  write data 00ef at offset 0034
000638:  write data 0a9f at offset 0032
--------------------------------------------
000620:  write data 5105 at offset 003c
000624:  write data 0005 at offset 0038
000628:  write data fefb at offset 003a

000632:  write data 0050 at offset 0034
000638:  write data 0a80 at offset 0032
000632:  write data 0051 at offset 0034
000638:  write data 0984 at offset 0032
000632:  write data 0052 at offset 0034
000638:  write data 0082 at offset 0032
000632:  write data 0053 at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 0054 at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 0055 at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 0056 at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 0057 at offset 0034
000638:  write data 0000 at offset 0032
--------------------------------------------
000620:  write data 5905 at offset 003c
000624:  write data 0005 at offset 0038
000628:  write data fffb at offset 003a

000632:  write data 0058 at offset 0034
000638:  write data 09c8 at offset 0032
000632:  write data 0059 at offset 0034
000638:  write data 0a84 at offset 0032
000632:  write data 005a at offset 0034
000638:  write data 00a2 at offset 0032
000632:  write data 005b at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 005c at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 005d at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 005e at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 005f at offset 0034
000638:  write data 0000 at offset 0032
--------------------------------------------
000620:  write data 7905 at offset 003c
000624:  write data 0006 at offset 0038
000628:  write data fffb at offset 003a

000632:  write data 0078 at offset 0034
000638:  write data 01a2 at offset 0032
000632:  write data 0079 at offset 0034
000638:  write data 02c2 at offset 0032
000632:  write data 007a at offset 0034
000638:  write data 00a2 at offset 0032
000632:  write data 007b at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 007c at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 007d at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 007e at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 007f at offset 0034
000638:  write data 0000 at offset 0032
--------------------------------------------
000620:  write data f105 at offset 003c
000624:  write data 0005 at offset 0038
000628:  write data fefb at offset 003a

000632:  write data 00f0 at offset 0034
000638:  write data 0a88 at offset 0032
000632:  write data 00f1 at offset 0034
000638:  write data 0994 at offset 0032
000632:  write data 00f2 at offset 0034
000638:  write data 0088 at offset 0032
000632:  write data 00f3 at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 00f4 at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 00f5 at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 00f6 at offset 0034
000638:  write data 0000 at offset 0032
000632:  write data 00f7 at offset 0034
000638:  write data 0000 at offset 0032

These uploads appear to form the basis of one part of the protection; command lists.

The games upload these tables

cupsoc, cupsoca, cupsocs, cupsocs2, olysoc92
t    u1  u2     trg    tbl
00 | 6 | ffeb | 0205 | 188 282 082 b8e 98e
01 | 6 | fbfb | 0905 | 194 288 088
02 | 5 | bf7f | 138e | 984 aa4 d82 aa2 39b b9a b9a a9a
03 | 6 | fbfb | 1905 | 994 a88 088
04 | 5 | f5df | 2288 | f8a b8a 388 b9a b9a a9a
05 | 6 | ebeb | 2a05 | 9af a82 082 a8f 18e
06 | 5 | bf7f | 338e | 984 aa4 d82 aa2 39c b9c b9c a9a
07 | 4 | 007f | 3bb0 | f9c b9c b9c b9c b9c b9c b9c 99c
08 | 5 | fcdd | 42c2 | f9a b9a b9c b9c b9c 29c
09 | 5 | fcdd | 4aa0 | f9a b9a b9c b9c b9c 99b
0a | 5 | fefb | 5105 | a80 984 082
0b | 5 | fffb | 5905 | 9c8 a84 0a2
0c | 8 | f3e7 | 6200 | 3a0 3a6 380 aa0 2a6
0d | a | fff3 | 6880 | b80 ba0
0e | 0 | 0000 | 0000 |
0f | 6 | fffb | 7905 | 1a2 2c2 0a2
10 | 7 | fdfb | 8100 | b9a b88 888
11 | 7 | fdfb | 8900 | b9a b8a 88a
12 | 7 | f8f7 | 9180 | b80 b94 b94 894
13 | 7 | f8f7 | 9980 | b80 b94 b94 896
14 | 0 | ffff | a180 | b80 b82 b84 b86
15 | f | ffff | a980 | ba0 ba2 ba4 ba6
16 | 9 | ffff | b100 | b40 bc0 bc2
17 | 6 | ffff | b900 | b60 be0 be2
18 | a | ff00 | c480 | 080 882
19 | 5 | bf7f | cb8f | 984 aa4 d82 aa2 39b b9a b9a a9f
1a | 5 | fffb | d104 | ac2 9e0 0a2
1b | 5 | 7ff7 | dde5 | f80 aa2 984 0c2
1c | 5 | b07f | e38e | 984 ac4 d82 ac2 39b b9a b9a a9a
1d | 5 | b07f | eb8e | 984 ac4 d82 ac2 39b b9a b9a a9f
1e | 5 | fefb | f105 | a88 994 088
1f | 0 | 0000 | 0000 |

heatbrl, heatbrl2, heatbrlo, heatbrlu
t    u1  u2     trg    tbl
00 | 6 | ffeb | 0205 | 188 282 082 b8e 98e
01 | 6 | fbfb | 0905 | 194 288 088
02 | 5 | bf7f | 138e | 984 aa4 d82 aa2 39b b9a b9a b9a
03 | 6 | fbfb | 1905 | 994 a88 088
04 | 5 | f5df | 2288 | f8a b8a 388 b9c b9a a9a
05 | 6 | ebeb | 2a05 | 9af a82 082 a8f 18e
06 | 5 | bf7f | 338e | 984 aa4 d82 aa2 39c b9c b9c a9a
07 | 4 | 007f | 3bb0 | f9c b9c b9c b9c b9c b9c b9c 99c
08 | 5 | fcdd | 42c2 | f9a b9a b9c b9c b9c 29c
09 | 5 | fcdd | 4aa0 | f9a b9a b9c b9c b9c 99b
0a | 0 | 0000 | 0000 |
0b | 0 | 0000 | 0000 |
0c | 8 | f3e7 | 6200 | 380 39a 380 a80 29a
0d | a | fff3 | 6880 | b80 ba0
0e | 0 | 0000 | 0000 |
0f | 0 | 0000 | 0000 |
10 | 7 | fdfb | 8100 | b9a b88 888
11 | 7 | fdfb | 8900 | b9a b8a 88a
12 | 7 | f8f7 | 9180 | b80 b94 b94 894
13 | 7 | f8f7 | 9980 | b80 b96 b96 896
14 | 0 | ffff | a100 | b80 b82 b84 b86
15 | f | ffff | a900 | ba0 ba2 ba4 ba6
16 | 9 | ffff | b080 | b40 bc0 bc2
17 | 6 | ffff | b880 | b60 be0 be2
18 | a | ff00 | c480 | 080 882
19 | 0 | 0000 | 0000 |
1a | 0 | 0000 | 0000 |
1b | 0 | 0000 | 0000 |
1c | 0 | 0000 | 0000 |
1d | 0 | 0000 | 0000 |
1e | 0 | 0000 | 0000 |
1f | 0 | 0000 | 0000 |

legionna, legionnau (commands are the same as heatbrl, triggers are different)
t    u1  u2     trg    tbl
00 | 6 | ffeb | 0205 | 188 282 082 b8e 98e
01 | 6 | fbfb | 0905 | 194 288 088
02 | 5 | bf7f | 138e | 984 aa4 d82 aa2 39b b9a b9a b9a
03 | 6 | fbfb | 1905 | 994 a88 088
04 | 5 | f5df | 2288 | f8a b8a 388 b9c b9a a9a
05 | 6 | ebeb | 2a05 | 9af a82 082 a8f 18e
06 | 5 | bf7f | 338e | 984 aa4 d82 aa2 39c b9c b9c a9a
07 | 4 | 007f | 3bb0 | f9c b9c b9c b9c b9c b9c b9c 99c
08 | 5 | fcdd | 42c2 | f9a b9a b9c b9c b9c 29c
09 | 5 | fcdd | 4aa0 | f9a b9a b9c b9c b9c 99b
0a | 0 | 0000 | 0000 |
0b | 0 | 0000 | 0000 |
0c | 8 | f3e7 | 6200 | 380 39a 380 a80 29a
0d | a | fff3 | 6880 | b80 ba0
0e | 0 | 0000 | 0000 |
0f | 0 | 0000 | 0000 |
10 | 7 | fdfb | 8100 | b9a b88 888
11 | 7 | fdfb | 8900 | b9a b8a 88a
12 | 7 | f8f7 | 9180 | b80 b94 b94 894
13 | 7 | f8f7 | 9980 | b80 b96 b96 896
14 | 0 | ffff | a180 | b80 b82 b84 b86
15 | f | ffff | a980 | ba0 ba2 ba4 ba6
16 | 9 | ffff | b100 | b40 bc0 bc2
17 | 6 | ffff | b900 | b60 be0 be2
18 | a | ff00 | c480 | 080 882
19 | 0 | 0000 | 0000 |
1a | 0 | 0000 | 0000 |
1b | 0 | 0000 | 0000 |
1c | 0 | 0000 | 0000 |
1d | 0 | 0000 | 0000 |
1e | 0 | 0000 | 0000 |
1f | 0 | 0000 | 0000 |

godzilla, denjinmk
(denjinmk doesn't actually make use of the table, it never writes to the execute trigger)
t    u1  u2     trg    tbl
00 | 6 | ffeb | 0205 | 188 282 082 b8e 98e
01 | 6 | fbfb | 0905 | 194 288 088
02 | 5 | bf7f | 138e | 984 aa4 d82 aa2 39b b9a b9a a9a
03 | 6 | fbfb | 1905 | 994 a88 088
04 | 5 | f5df | 2288 | f8a b8a 388 b9a b9a a9a
05 | 6 | ebeb | 2a05 | 9af a82 082 a8f 18e
06 | 5 | bf7f | 338e | 984 aa4 d82 aa2 39c b9c b9c a9a
07 | 4 | 007f | 3bb0 | f9c b9c b9c b9c b9c b9c b9c 99c
08 | 5 | fcdd | 42c2 | f9a b9a b9c b9c b9c 29c
09 | 5 | fcdd | 4aa0 | f9a b9a b9c b9c b9c 99b
0a | 0 | 0000 | 0000 |
0b | 0 | 0000 | 0000 |
0c | 8 | f3e7 | 6200 | 380 39a 380 a80 29a
0d | a | fff3 | 6880 | b80 ba0
0e | 0 | 0000 | 0000 |
0f | 0 | 0000 | 0000 |
10 | 7 | fdfb | 8100 | b9a b88 888
11 | 7 | fdfb | 8900 | b9a b8a 88a
12 | 7 | f8f7 | 9180 | b80 b94 b94 894
13 | 7 | f8f7 | 9980 | b80 b94 b94 896
14 | 0 | ffff | a180 | b80 b82 b84 b86
15 | f | ffff | a980 | ba0 ba2 ba4 ba6
16 | 9 | ffff | b100 | b40 bc0 bc2
17 | 6 | ffff | b900 | b60 be0 be2
18 | a | ff00 | c480 | 080 882
19 | 0 | 0000 | 0000 |
1a | 0 | 0000 | 0000 |
1b | 0 | 0000 | 0000 |
1c | 0 | 0000 | 0000 |
1d | 0 | 0000 | 0000 |
1e | 0 | 0000 | 0000 |
1f | 0 | 0000 | 0000 |

grainbow
t    u1  u2     trg    tbl
00 | 6 | ffeb | 0205 | 188 282 082 b8e 98e
01 | 6 | fbfb | 0905 | 194 288 088
02 | 5 | bf7f | 138e | 984 aa4 d82 aa2 39b b9a b9a a9a
03 | 6 | fbfb | 1905 | 994 a88 088
04 | 5 | f5df | 2288 | f8a b8a 388 b9a b9a a9a
05 | 6 | ebeb | 2a05 | 9af a82 082 a8f 18e
06 | 5 | bf7f | 338e | 984 aa4 d82 aa2 39c b9c b9c a9a
07 | 4 | 007f | 3bb0 | f9c b9c b9c b9c b9c b9c b9c 99c
08 | 5 | fcdd | 42c2 | f9a b9a b9c b9c b9c 29c
09 | 5 | fcdd | 4aa0 | f9a b9a b9c b9c b9c 99b
0a | 5 | fefb | 5105 | a80 984 082
0b | 5 | fffb | 5905 | 9c8 a84 0a2
0c | 8 | f3e7 | 6200 | 380 39a 380 a80 29a
0d | a | fff3 | 6980 | b80 ba0
0e | 0 | 0000 | 0000 |
0f | 6 | fffb | 7905 | 1a2 2c2 0a2
10 | 7 | fdfb | 8100 | b9a b88 888
11 | 7 | fdfb | 8900 | b9a b8a 88a
12 | 7 | f8f7 | 9180 | b80 b94 b94 894
13 | 7 | f8f7 | 9980 | b80 b94 b94 896
14 | 0 | 02ff | a180 | b80 b82 b84 b86
15 | f | 02ff | a980 | ba0 ba2 ba4 ba6
16 | 9 | ffff | b100 | b40 bc0 bc2
17 | 6 | ffff | b900 | b60 be0 be2
18 | a | ff00 | c480 | 080 882
19 | 5 | bf7f | cb8f | 984 aa4 d82 aa2 39b b9a b9a a9f
1a | 5 | fffb | d104 | ac2 9e0 0a2
1b | 5 | 7ff7 | dde5 | f80 aa2 984 0c2
1c | 5 | b07f | e38e | 984 ac4 d82 ac2 39b b9a b9a a9a
1d | 5 | b07f | eb8e | 984 ac4 d82 ac2 39b b9a b9a a9f
1e | 5 | fefb | f105 | a88 994 088
1f | 0 | 0000 | 0000 |

raiden2, raiden2a, raiden2b, raiden2c, raiden2d, raiden2e, raiden2f
t    u1  u2     trg    tbl
00 | 6 | ffeb | 0205 | 188 282 082 b8e 98e
01 | 6 | fbfb | 0905 | 194 288 088
02 | 5 | bf7f | 130e | 984 aa4 d82 aa2 39b b9a b9a a9a
03 | 6 | fbfb | 1905 | 994 a88 088
04 | 5 | f5df | 2208 | f8a b8a 388 b9a b9a a9a
05 | 6 | ebeb | 2a05 | 9af a82 082 a8f 18e
06 | 5 | bf7f | 338e | 984 aa4 d82 aa2 39c b9c b9c a9a
07 | 4 | 007f | 3bb0 | f9c b9c b9c b9c b9c b9c b9c 99c
08 | 5 | fcdd | 42c2 | f9a b9a b9c b9c b9c 29c
09 | 5 | fcdd | 4aa0 | f9a b9a b9c b9c b9c 99b
0a | 6 | fff7 | 5205 | 180 2e0 3a0 0a0 3a0
0b | 6 | fff7 | 5a05 | 180 2e0 3a0 0a0 3a0
0c | 8 | f3e7 | 6200 | 380 39a 380 a80 29a
0d | 0 | 0000 | 0000 |
0e | 0 | 0000 | 0000 |
0f | 0 | 0000 | 0000 |
10 | 7 | fdfb | 8100 | b9a b88 888
11 | 7 | fdfb | 8900 | b9a b8a 88a
12 | 7 | fefb | 9100 | b80 b94 894
13 | 7 | fefb | 9900 | b80 b94 896
14 | 0 | 00ff | a100 | b80 b82 b84 b86
15 | f | 00ff | a900 | ba0 ba2 ba4 ba6
16 | 9 | ffff | b100 | b40 bc0 bc2
17 | 6 | ffff | b900 | b60 be0 be2
18 | 0 | 0000 | 0000 |
19 | 0 | 0000 | 0000 |
1a | 0 | 0000 | 0000 |
1b | 0 | 0000 | 0000 |
1c | 0 | 0000 | 0000 |
1d | 0 | 0000 | 0000 |
1e | 6 | fff7 | f205 | 182 2e0 3c0 0c0 3c0
1f | 0 | 0000 | 0000 |

raidndx, raidndxj, raidndxm, raidndxt
(the same as raiden2, but adds an extra command with trigger 7e05)
t    u1  u2     trg    tbl
00 | 6 | ffeb | 0205 | 188 282 082 b8e 98e
01 | 6 | fbfb | 0905 | 194 288 088
02 | 5 | bf7f | 130e | 984 aa4 d82 aa2 39b b9a b9a a9a
03 | 6 | fbfb | 1905 | 994 a88 088
04 | 5 | f5df | 2208 | f8a b8a 388 b9a b9a a9a
05 | 6 | ebeb | 2a05 | 9af a82 082 a8f 18e
06 | 5 | bf7f | 338e | 984 aa4 d82 aa2 39c b9c b9c a9a
07 | 4 | 007f | 3bb0 | f9c b9c b9c b9c b9c b9c b9c 99c
08 | 5 | fcdd | 42c2 | f9a b9a b9c b9c b9c 29c
09 | 5 | fcdd | 4aa0 | f9a b9a b9c b9c b9c 99b
0a | 6 | fff7 | 5205 | 180 2e0 3a0 0a0 3a0
0b | 6 | fff7 | 5a05 | 180 2e0 3a0 0a0 3a0
0c | 8 | f3e7 | 6200 | 380 39a 380 a80 29a
0d | 0 | 0000 | 0000 |
0e | 0 | 0000 | 0000 |
0f | 6 | fffb | 7e05 | 180 282 080 180 282
10 | 7 | fdfb | 8100 | b9a b88 888
11 | 7 | fdfb | 8900 | b9a b8a 88a
12 | 7 | fefb | 9100 | b80 b94 894
13 | 7 | fefb | 9900 | b80 b94 896
14 | 0 | 00ff | a100 | b80 b82 b84 b86
15 | f | 00ff | a900 | ba0 ba2 ba4 ba6
16 | 9 | ffff | b100 | b40 bc0 bc2
17 | 6 | ffff | b900 | b60 be0 be2
18 | 0 | 0000 | 0000 |
19 | 0 | 0000 | 0000 |
1a | 0 | 0000 | 0000 |
1b | 0 | 0000 | 0000 |
1c | 0 | 0000 | 0000 |
1d | 0 | 0000 | 0000 |
1e | 6 | fff7 | f205 | 182 2e0 3c0 0c0 3c0
1f | 0 | 0000 | 0000 |


zeroteam, zeroteama, zeroteamb, zeroteamc, zeroteams, xsedae
t    u1  u2     trg    tbl
00 | 6 | ffeb | 0205 | 188 282 082 b8e 98e
01 | 6 | fbfb | 0905 | 194 288 088
02 | 5 | bf7f | 130e | 984 aa4 d82 aa2 39b b9a b9a a9a
03 | 6 | fbfb | 1905 | 994 a88 088
04 | 5 | f5df | 2208 | f8a b8a 388 b9a b9a a9a
05 | 6 | ebeb | 2a05 | 9af a82 082 a8f 18e
06 | 5 | bf7f | 330e | 984 aa4 d82 aa2 39c b9c b9c a9a
07 | 4 | 007f | 3b30 | f9c b9c b9c b9c b9c b9c b9c 99c
08 | 5 | fcdd | 42c2 | f9a b9a b9c b9c b9c 29c
09 | 5 | fcdd | 4aa0 | f9a b9a b9c b9c b9c 99b
0a | 6 | fffb | 5105 | 180 2e0 0a0
0b | 6 | ffdb | 5a85 | 180 2e0 0a0 182 2e0 0c0 3c0
0c | 8 | f3e7 | 6200 | 380 39a 380 a80 29a
0d | a | fff3 | 6980 | b80 ba0
0e | 8 | fdfd | 7100 | b80 a80 b80
0f | 0 | 0000 | 0000 |
10 | 7 | fdfb | 8100 | b9a b88 888
11 | 7 | fdfb | 8900 | b9a b8a 88a
12 | 7 | f8f7 | 9100 | b80 b94 b94 894
13 | 7 | f8f7 | 9900 | b80 b94 b94 896
14 | 0 | ffff | a100 | b80 b82 b84 b86
15 | f | ffff | a900 | ba0 ba2 ba4 ba6
16 | 9 | ffff | b100 | b40 bc0 bc2
17 | 6 | ffff | b900 | b60 be0 be2
18 | a | ff00 | 7c80 | 080 882
19 | 0 | 0000 | 0000 |
1a | 0 | 0000 | 0000 |
1b | 0 | 0000 | 0000 |
1c | 5 | 06fb | e105 | a88 994 088
1d | 5 | 05f7 | ede5 | f88 a84 986 08a
1e | 4 | 00ff | f790 | f80 b84 b84 b84 b84 b84 b84 b84
1f | 6 | 00ff | fc84 | 182 280

as you can see, there are a lot of command 'command lists' between the games.
(todo, comment ones which seem to have a known function)

executing these seems to cause various operations to occur, be it memory transfer, collision checking, or movement checking.

each 12-bit entry in the tables is probably some kind of 'opcode' which runs and processes data placed in registers / memory.

If we rearrange these tables a bit, so that we can see which are common between games we get the following, take note of
the ones with slight changes, these could be important to figuring out how this works!


Game       |u1 |u2    | trig | macrolist

Table 00 - Same on All games
(grainbow) | 6 | ffeb | 0205 | 188 282 082 b8e 98e
(cupsoc)   | 6 | ffeb | 0205 | 188 282 082 b8e 98e
(legionna) | 6 | ffeb | 0205 | 188 282 082 b8e 98e
(godzilla) | 6 | ffeb | 0205 | 188 282 082 b8e 98e
(heatbrl)  | 6 | ffeb | 0205 | 188 282 082 b8e 98e
(zeroteam) | 6 | ffeb | 0205 | 188 282 082 b8e 98e
(raiden2)  | 6 | ffeb | 0205 | 188 282 082 b8e 98e
(raidndx)  | 6 | ffeb | 0205 | 188 282 082 b8e 98e

Table 01 - Same on All games
(grainbow) | 6 | fbfb | 0905 | 194 288 088
(cupsoc)   | 6 | fbfb | 0905 | 194 288 088
(legionna) | 6 | fbfb | 0905 | 194 288 088
(godzilla) | 6 | fbfb | 0905 | 194 288 088
(heatbrl)  | 6 | fbfb | 0905 | 194 288 088
(zeroteam) | 6 | fbfb | 0905 | 194 288 088
(raiden2)  | 6 | fbfb | 0905 | 194 288 088
(raidndx)  | 6 | fbfb | 0905 | 194 288 088

Table 02 - grainbow and heatbrl have different last entry.  triggers differ on v30 hw
(grainbow) | 5 | bf7f | 138e | 984 aa4 d82 aa2 39b b9a b9a b9a
(cupsoc)   | 5 | bf7f | 138e | 984 aa4 d82 aa2 39b b9a b9a a9a
(legionna) | 5 | bf7f | 138e | 984 aa4 d82 aa2 39b b9a b9a a9a
(godzilla) | 5 | bf7f | 138e | 984 aa4 d82 aa2 39b b9a b9a a9a
(heatbrl)  | 5 | bf7f | 138e | 984 aa4 d82 aa2 39b b9a b9a b9a
(zeroteam) | 5 | bf7f | 130e | 984 aa4 d82 aa2 39b b9a b9a a9a
(raiden2)  | 5 | bf7f | 130e | 984 aa4 d82 aa2 39b b9a b9a a9a
(raidndx)  | 5 | bf7f | 130e | 984 aa4 d82 aa2 39b b9a b9a a9a

Table 03 - Same on All games
(grainbow) | 6 | fbfb | 1905 | 994 a88 088
(cupsoc)   | 6 | fbfb | 1905 | 994 a88 088
(legionna) | 6 | fbfb | 1905 | 994 a88 088
(godzilla) | 6 | fbfb | 1905 | 994 a88 088
(heatbrl)  | 6 | fbfb | 1905 | 994 a88 088
(zeroteam) | 6 | fbfb | 1905 | 994 a88 088
(raiden2)  | 6 | fbfb | 1905 | 994 a88 088
(raidndx)  | 6 | fbfb | 1905 | 994 a88 088

Table 04 - grainbow and heatbrl have a b9c in the 4th slot, triggers differ on v30 hw
(grainbow) | 5 | f5df | 2288 | f8a b8a 388 b9c b9a a9a
(cupsoc)   | 5 | f5df | 2288 | f8a b8a 388 b9a b9a a9a
(legionna) | 5 | f5df | 2288 | f8a b8a 388 b9a b9a a9a
(godzilla) | 5 | f5df | 2288 | f8a b8a 388 b9a b9a a9a
(heatbrl)  | 5 | f5df | 2288 | f8a b8a 388 b9c b9a a9a
(zeroteam) | 5 | f5df | 2208 | f8a b8a 388 b9a b9a a9a
(raiden2)  | 5 | f5df | 2208 | f8a b8a 388 b9a b9a a9a
(raidndx)  | 5 | f5df | 2208 | f8a b8a 388 b9a b9a a9a

Table 05 - Same on All games
(grainbow) | 6 | ebeb | 2a05 | 9af a82 082 a8f 18e
(cupsoc)   | 6 | ebeb | 2a05 | 9af a82 082 a8f 18e
(legionna) | 6 | ebeb | 2a05 | 9af a82 082 a8f 18e
(godzilla) | 6 | ebeb | 2a05 | 9af a82 082 a8f 18e
(heatbrl)  | 6 | ebeb | 2a05 | 9af a82 082 a8f 18e
(zeroteam) | 6 | ebeb | 2a05 | 9af a82 082 a8f 18e
(raiden2)  | 6 | ebeb | 2a05 | 9af a82 082 a8f 18e
(raidndx)  | 6 | ebeb | 2a05 | 9af a82 082 a8f 18e

Table 06 - different trigger on zeroteam (330e)
(grainbow) | 5 | bf7f | 338e | 984 aa4 d82 aa2 39c b9c b9c a9a
(cupsoc)   | 5 | bf7f | 338e | 984 aa4 d82 aa2 39c b9c b9c a9a
(legionna) | 5 | bf7f | 338e | 984 aa4 d82 aa2 39c b9c b9c a9a
(godzilla) | 5 | bf7f | 338e | 984 aa4 d82 aa2 39c b9c b9c a9a
(heatbrl)  | 5 | bf7f | 338e | 984 aa4 d82 aa2 39c b9c b9c a9a
(zeroteam) | 5 | bf7f | 330e | 984 aa4 d82 aa2 39c b9c b9c a9a
(raiden2)  | 5 | bf7f | 338e | 984 aa4 d82 aa2 39c b9c b9c a9a
(raidndx)  | 5 | bf7f | 338e | 984 aa4 d82 aa2 39c b9c b9c a9a

Table 07 - different trigger on zeroteam (3b30)
(grainbow) | 4 | 007f | 3bb0 | f9c b9c b9c b9c b9c b9c b9c 99c
(cupsoc)   | 4 | 007f | 3bb0 | f9c b9c b9c b9c b9c b9c b9c 99c
(legionna) | 4 | 007f | 3bb0 | f9c b9c b9c b9c b9c b9c b9c 99c
(godzilla) | 4 | 007f | 3bb0 | f9c b9c b9c b9c b9c b9c b9c 99c
(heatbrl)  | 4 | 007f | 3bb0 | f9c b9c b9c b9c b9c b9c b9c 99c
(zeroteam) | 4 | 007f | 3b30 | f9c b9c b9c b9c b9c b9c b9c 99c
(raiden2)  | 4 | 007f | 3bb0 | f9c b9c b9c b9c b9c b9c b9c 99c
(raidndx)  | 4 | 007f | 3bb0 | f9c b9c b9c b9c b9c b9c b9c 99c

Table 08 - Same on All games
(grainbow) | 5 | fcdd | 42c2 | f9a b9a b9c b9c b9c 29c
(cupsoc)   | 5 | fcdd | 42c2 | f9a b9a b9c b9c b9c 29c
(legionna) | 5 | fcdd | 42c2 | f9a b9a b9c b9c b9c 29c
(godzilla) | 5 | fcdd | 42c2 | f9a b9a b9c b9c b9c 29c
(heatbrl)  | 5 | fcdd | 42c2 | f9a b9a b9c b9c b9c 29c
(zeroteam) | 5 | fcdd | 42c2 | f9a b9a b9c b9c b9c 29c
(raiden2)  | 5 | fcdd | 42c2 | f9a b9a b9c b9c b9c 29c
(raidndx)  | 5 | fcdd | 42c2 | f9a b9a b9c b9c b9c 29c

Table 09 - Same on All games
(grainbow) | 5 | fcdd | 4aa0 | f9a b9a b9c b9c b9c 99b
(cupsoc)   | 5 | fcdd | 4aa0 | f9a b9a b9c b9c b9c 99b
(legionna) | 5 | fcdd | 4aa0 | f9a b9a b9c b9c b9c 99b
(godzilla) | 5 | fcdd | 4aa0 | f9a b9a b9c b9c b9c 99b
(heatbrl)  | 5 | fcdd | 4aa0 | f9a b9a b9c b9c b9c 99b
(zeroteam) | 5 | fcdd | 4aa0 | f9a b9a b9c b9c b9c 99b
(raiden2)  | 5 | fcdd | 4aa0 | f9a b9a b9c b9c b9c 99b
(raidndx)  | 5 | fcdd | 4aa0 | f9a b9a b9c b9c b9c 99b

Table 0a - Game specific
(grainbow) | 5 | fefb | 5105 | a80 984 082
(cupsoc)   | 5 | fefb | 5105 | a80 984 082
(legionna) | 0 | 0000 | 0000 |
(godzilla) | 0 | 0000 | 0000 |
(heatbrl)  | 0 | 0000 | 0000 |
(zeroteam) | 6 | fffb | 5105 | 180 2e0 0a0
(raiden2)  | 6 | fff7 | 5205 | 180 2e0 3a0 0a0 3a0
(raidndx)  | 6 | fff7 | 5205 | 180 2e0 3a0 0a0 3a0

Table 0b - Game specific
(grainbow) | 5 | fffb | 5905 | 9c8 a84 0a2
(cupsoc)   | 5 | fffb | 5905 | 9c8 a84 0a2
(legionna) | 0 | 0000 | 0000 |
(godzilla) | 0 | 0000 | 0000 |
(heatbrl)  | 0 | 0000 | 0000 |
(zeroteam) | 6 | ffdb | 5a85 | 180 2e0 0a0 182 2e0 0c0 3c0
(raiden2)  | 6 | fff7 | 5a05 | 180 2e0 3a0 0a0 3a0
(raidndx)  | 6 | fff7 | 5a05 | 180 2e0 3a0 0a0 3a0

Table 0c - cupsoc has various modifications
notice how *80 is replaced by *a0 and *9a is replaced with *a6, maybe it has the same function, but on different registers?
(grainbow) | 8 | f3e7 | 6200 | 380 39a 380 a80 29a
(cupsoc)   | 8 | f3e7 | 6200 | 3a0 3a6 380 aa0 2a6
(legionna) | 8 | f3e7 | 6200 | 380 39a 380 a80 29a
(godzilla) | 8 | f3e7 | 6200 | 380 39a 380 a80 29a
(heatbrl)  | 8 | f3e7 | 6200 | 380 39a 380 a80 29a
(zeroteam) | 8 | f3e7 | 6200 | 380 39a 380 a80 29a
(raiden2)  | 8 | f3e7 | 6200 | 380 39a 380 a80 29a
(raidndx)  | 8 | f3e7 | 6200 | 380 39a 380 a80 29a

Table 0d - Zero team uses different trigger, doesn't exist on raiden2/dx
(grainbow) | a | fff3 | 6980 | b80 ba0
(cupsoc)   | a | fff3 | 6880 | b80 ba0
(legionna) | a | fff3 | 6880 | b80 ba0
(godzilla) | a | fff3 | 6880 | b80 ba0
(heatbrl)  | a | fff3 | 6880 | b80 ba0
(zeroteam) | a | fff3 | 6980 | b80 ba0
(raiden2)  | 0 | 0000 | 0000 |
(raidndx)  | 0 | 0000 | 0000 |

Table 0e - Zero Team only
(grainbow) | 0 | 0000 | 0000 |
(cupsoc)   | 0 | 0000 | 0000 |
(legionna) | 0 | 0000 | 0000 |
(godzilla) | 0 | 0000 | 0000 |
(heatbrl)  | 0 | 0000 | 0000 |
(zeroteam) | 8 | fdfd | 7100 | b80 a80 b80
(raiden2)  | 0 | 0000 | 0000 |
(raidndx)  | 0 | 0000 | 0000 |

Table 0f - Same on grainbow/cupsoc, different on raidndx (added compared to raiden2)
(grainbow) | 6 | fffb | 7905 | 1a2 2c2 0a2
(cupsoc)   | 6 | fffb | 7905 | 1a2 2c2 0a2
(legionna) | 0 | 0000 | 0000 |
(godzilla) | 0 | 0000 | 0000 |
(heatbrl)  | 0 | 0000 | 0000 |
(zeroteam) | 0 | 0000 | 0000 |
(raiden2)  | 0 | 0000 | 0000 |
(raidndx)  | 6 | fffb | 7e05 | 180 282 080 180 282

Table 10 - Same on all games
(grainbow) | 7 | fdfb | 8100 | b9a b88 888
(cupsoc)   | 7 | fdfb | 8100 | b9a b88 888
(legionna) | 7 | fdfb | 8100 | b9a b88 888
(godzilla) | 7 | fdfb | 8100 | b9a b88 888
(heatbrl)  | 7 | fdfb | 8100 | b9a b88 888
(zeroteam) | 7 | fdfb | 8100 | b9a b88 888
(raiden2)  | 7 | fdfb | 8100 | b9a b88 888
(raidndx)  | 7 | fdfb | 8100 | b9a b88 888

Table 11 - Same on all games
(grainbow) | 7 | fdfb | 8900 | b9a b8a 88a
(cupsoc)   | 7 | fdfb | 8900 | b9a b8a 88a
(legionna) | 7 | fdfb | 8900 | b9a b8a 88a
(godzilla) | 7 | fdfb | 8900 | b9a b8a 88a
(heatbrl)  | 7 | fdfb | 8900 | b9a b8a 88a
(zeroteam) | 7 | fdfb | 8900 | b9a b8a 88a
(raiden2)  | 7 | fdfb | 8900 | b9a b8a 88a
(raidndx)  | 7 | fdfb | 8900 | b9a b8a 88a

Table 12 - Raiden2/DX differ from others (list and trigger)
(grainbow) | 7 | f8f7 | 9180 | b80 b94 b94 894
(cupsoc)   | 7 | f8f7 | 9180 | b80 b94 b94 894
(legionna) | 7 | f8f7 | 9180 | b80 b94 b94 894
(godzilla) | 7 | f8f7 | 9180 | b80 b94 b94 894
(heatbrl)  | 7 | f8f7 | 9180 | b80 b94 b94 894
(zeroteam) | 7 | f8f7 | 9100 | b80 b94 b94 894
(raiden2)  | 7 | fefb | 9100 | b80 b94 894
(raidndx)  | 7 | fefb | 9100 | b80 b94 894

Table 13 - Raiden2/DX differ from others , slight changes on legionna and hearbrl too
           (*94 replaced with *96, to operate on a different register?)
(grainbow) | 7 | f8f7 | 9980 | b80 b94 b94 896
(cupsoc)   | 7 | f8f7 | 9980 | b80 b94 b94 896
(legionna) | 7 | f8f7 | 9980 | b80 b96 b96 896
(godzilla) | 7 | f8f7 | 9980 | b80 b94 b94 896
(heatbrl)  | 7 | f8f7 | 9980 | b80 b96 b96 896
(zeroteam) | 7 | f8f7 | 9900 | b80 b94 b94 896
(raiden2)  | 7 | fefb | 9900 | b80 b94 896
(raidndx)  | 7 | fefb | 9900 | b80 b94 896

Table 14 - Trigger differs on heatbrl + v30 games, unknown param differs on grainbow + v30 games
(grainbow) | 0 | 02ff | a180 | b80 b82 b84 b86
(cupsoc)   | 0 | ffff | a180 | b80 b82 b84 b86
(legionna) | 0 | ffff | a180 | b80 b82 b84 b86
(godzilla) | 0 | ffff | a180 | b80 b82 b84 b86
(heatbrl)  | 0 | ffff | a100 | b80 b82 b84 b86
(zeroteam) | 0 | ffff | a100 | b80 b82 b84 b86
(raiden2)  | 0 | 00ff | a100 | b80 b82 b84 b86
(raidndx)  | 0 | 00ff | a100 | b80 b82 b84 b86

Table 15 - Trigger differs on heatbrl + v30 games, unknown param differs on grainbow + v30 games
(grainbow) | f | 02ff | a980 | ba0 ba2 ba4 ba6
(cupsoc)   | f | ffff | a980 | ba0 ba2 ba4 ba6
(legionna) | f | ffff | a980 | ba0 ba2 ba4 ba6
(godzilla) | f | ffff | a980 | ba0 ba2 ba4 ba6
(heatbrl)  | f | ffff | a900 | ba0 ba2 ba4 ba6
(zeroteam) | f | ffff | a900 | ba0 ba2 ba4 ba6
(raiden2)  | f | 00ff | a900 | ba0 ba2 ba4 ba6
(raidndx)  | f | 00ff | a900 | ba0 ba2 ba4 ba6

Table 16 - Trigger differs on heatbrl
(grainbow) | 9 | ffff | b100 | b40 bc0 bc2
(cupsoc)   | 9 | ffff | b100 | b40 bc0 bc2
(legionna) | 9 | ffff | b100 | b40 bc0 bc2
(godzilla) | 9 | ffff | b100 | b40 bc0 bc2
(heatbrl)  | 9 | ffff | b080 | b40 bc0 bc2
(zeroteam) | 9 | ffff | b100 | b40 bc0 bc2
(raiden2)  | 9 | ffff | b100 | b40 bc0 bc2
(raidndx)  | 9 | ffff | b100 | b40 bc0 bc2

Table 17 - Trigger differs on heatbrl
(grainbow) | 6 | ffff | b900 | b60 be0 be2
(cupsoc)   | 6 | ffff | b900 | b60 be0 be2
(legionna) | 6 | ffff | b900 | b60 be0 be2
(godzilla) | 6 | ffff | b900 | b60 be0 be2
(heatbrl)  | 6 | ffff | b880 | b60 be0 be2
(zeroteam) | 6 | ffff | b900 | b60 be0 be2
(raiden2)  | 6 | ffff | b900 | b60 be0 be2
(raidndx)  | 6 | ffff | b900 | b60 be0 be2

Table 18 - Same for all 68k games, zero team has different trigger, not on Raiden2/DX
(grainbow) | a | ff00 | c480 | 080 882
(cupsoc)   | a | ff00 | c480 | 080 882
(legionna) | a | ff00 | c480 | 080 882
(godzilla) | a | ff00 | c480 | 080 882
(heatbrl)  | a | ff00 | c480 | 080 882
(zeroteam) | a | ff00 | 7c80 | 080 882
(raiden2)  | 0 | 0000 | 0000 |
(raidndx)  | 0 | 0000 | 0000 |

Table 19 - grainbow / cupsoc only
(grainbow) | 5 | bf7f | cb8f | 984 aa4 d82 aa2 39b b9a b9a a9f
(cupsoc)   | 5 | bf7f | cb8f | 984 aa4 d82 aa2 39b b9a b9a a9f
(legionna) | 0 | 0000 | 0000 |
(godzilla) | 0 | 0000 | 0000 |
(heatbrl)  | 0 | 0000 | 0000 |
(zeroteam) | 0 | 0000 | 0000 |
(raiden2)  | 0 | 0000 | 0000 |
(raidndx)  | 0 | 0000 | 0000 |

Table 1a - grainbow / cupsoc only
(grainbow) | 5 | fffb | d104 | ac2 9e0 0a2
(cupsoc)   | 5 | fffb | d104 | ac2 9e0 0a2
(legionna) | 0 | 0000 | 0000 |
(godzilla) | 0 | 0000 | 0000 |
(heatbrl)  | 0 | 0000 | 0000 |
(zeroteam) | 0 | 0000 | 0000 |
(raiden2)  | 0 | 0000 | 0000 |
(raidndx)  | 0 | 0000 | 0000 |

Table 1b - grainbow / cupsoc only
(grainbow) | 5 | 7ff7 | dde5 | f80 aa2 984 0c2
(cupsoc)   | 5 | 7ff7 | dde5 | f80 aa2 984 0c2
(legionna) | 0 | 0000 | 0000 |
(godzilla) | 0 | 0000 | 0000 |
(heatbrl)  | 0 | 0000 | 0000 |
(zeroteam) | 0 | 0000 | 0000 |
(raiden2)  | 0 | 0000 | 0000 |
(raidndx)  | 0 | 0000 | 0000 |

Table 1c - grainbow / cupsoc are the same, different on zero team
(grainbow) | 5 | b07f | e38e | 984 ac4 d82 ac2 39b b9a b9a a9a
(cupsoc)   | 5 | b07f | e38e | 984 ac4 d82 ac2 39b b9a b9a a9a
(legionna) | 0 | 0000 | 0000 |
(godzilla) | 0 | 0000 | 0000 |
(heatbrl)  | 0 | 0000 | 0000 |
(zeroteam) | 5 | 06fb | e105 | a88 994 088
(raiden2)  | 0 | 0000 | 0000 |
(raidndx)  | 0 | 0000 | 0000 |

Table 1d - grainbow / cupsoc are the same, different on zero team
(grainbow) | 5 | b07f | eb8e | 984 ac4 d82 ac2 39b b9a b9a a9f
(cupsoc)   | 5 | b07f | eb8e | 984 ac4 d82 ac2 39b b9a b9a a9f
(legionna) | 0 | 0000 | 0000 |
(godzilla) | 0 | 0000 | 0000 |
(heatbrl)  | 0 | 0000 | 0000 |
(zeroteam) | 5 | 05f7 | ede5 | f88 a84 986 08a
(raiden2)  | 0 | 0000 | 0000 |
(raidndx)  | 0 | 0000 | 0000 |

Table 1e - grainbow / cupsoc are the same, different on zero team, different on raiden2/dx
(grainbow) | 5 | fefb | f105 | a88 994 088
(cupsoc)   | 5 | fefb | f105 | a88 994 088
(legionna) | 0 | 0000 | 0000 |
(godzilla) | 0 | 0000 | 0000 |
(heatbrl)  | 0 | 0000 | 0000 |
(zeroteam) | 4 | 00ff | f790 | f80 b84 b84 b84 b84 b84 b84 b84
(raiden2)  | 6 | fff7 | f205 | 182 2e0 3c0 0c0 3c0
(raidndx)  | 6 | fff7 | f205 | 182 2e0 3c0 0c0 3c0

Table 1f - zeroteam specific
(grainbow) | 0 | 0000 | 0000 |
(cupsoc)   | 0 | 0000 | 0000 |
(legionna) | 0 | 0000 | 0000 |
(godzilla) | 0 | 0000 | 0000 |
(heatbrl)  | 0 | 0000 | 0000 |
(zeroteam) | 6 | 00ff | fc84 | 182 280
(raiden2)  | 0 | 0000 | 0000 |
(raidndx)  | 0 | 0000 | 0000 |



typically the games write data for use with the commands at MCUBASE+0xa0 - 0xaf  and MCUBASE+0xc0 - 0xcf before triggering
the operation by writing to MCUBASE+0x100 (or MCUBASE+0x102) with the trigger value.  I believe the commands can change
both COP registers and system memory.

(MCUBASE typically being 0x100400 in the 68k games, and 0x400 in the v30 games)

Seibu Cup Soccer sometimes attempts to use a trigger value which wasn't defined in the table, I don't know what should
happen in that case!

----

Protection Part 2: BCD Maths

some additional registers serve as a math box type device, converting numbers + other functions.  Godzilla seems to use
this for a protection check, other games (Denjin Makai, Raiden 2) use it for scoring:

----

Protection Part 3: Private Buffer DMA + RAM Clear
(todo, expand on this)

address ranges can be specified which allows DMA Fill / Clear operations to be performed, as well as transfering
tilemap+palette data to private buffers for rendering.  If you don't use these nothing gets updated on the real
hardware!.  These don't currently make much sense because the hardware specifies ranges which aren't mapped, or
contain nothing.  It's possible the original hardware has mirroring which this function relies on.

the DMA to private buffer operations are currently ignored due to
if ((cop_dma_trigger==0x14) || (cop_dma_trigger==0x15)) return;

----

Other Protections?

Denjin Makai seems to rely on a byteswapped mirror to write the palette.
Various other ports go through the COP area, and get mapped to inputs / sounds / video registers, this adds to
the confusion and makes it less clear what is / isn't protection related

=================================================================================================================
Seibu COP memory map

DMA mode partial documentation:
0x476
???? ???? ???? ???? SRC table value used for palette DMAs, val << 10

0x478
xxxx xxxx xxxx xxxx SRC address register, val << 6

0x47a
xxxx xxxx xxxx xxxx length register, val << 5

0x47c
xxxx xxxx xxxx xxxx DST address register, val << 6

0x47e
---- ---x ---x ---- DMA mode (00: DMA, work RAM to work RAM 01: DMA, work RAM to private buffers / 10 <unknown> / 11: fill work RAM)
---- ---- x--- ---- palette DMA mode (used for brightness effects)
---- ---- ---- x--- Transfer type (0: word 1:dword)
---- ---- ---- -xxx Channel #

- channels 0x4 and 0x5 are always used to transfer respectively the VRAM and the palette data to the private buffers.
  It isn't know at current stage if it's just a design choice or they are dedicated DMAs.

- Some games (Heated Barrel start-up menu, Olympic Soccer '92 OBJ test) sets up the layer clearance in the midst of the
  frame interval. It might indicate that it delays those DMAs inside the buffers at vblank time.

- Raiden 2 / Raiden DX sets 0x14 with DST = 0xfffe and size as a sprite limit behaviour. The former is probably used to change the
  order of the loaded tables (they are the only known cases where spriteram is smallest address-wise).

- Reading here is probably used for DMA status of the individual channels or just for read-back of the register, but nothing seems to rely
  on it so far so nothing is really known about it.

0x6fc
???? ???? ???? ???? triggers DMA loaded in registers, value looks meaningless

Miscellaneous registers:
0x470
???? ???? ???? ???? External pin register, used by some games for prg/gfx banking (per-game specific)

*/

#include "emu.h"
#include "audio/seibu.h"
#include "includes/legionna.h"
#include "includes/raiden2.h"
#include "machine/seicop.h"

#define seibu_cop_log logerror
#define LOG_CMDS 1

UINT16 *cop_mcu_ram;

static UINT16 copd2_table[0x100];
static UINT16 copd2_table_2[0x100/8];
static UINT16 copd2_table_3[0x100/8];
static UINT16 copd2_table_4[0x100/8];

static UINT16 cop_438;
static UINT16 cop_43a;
static UINT16 cop_43c;

static UINT16 cop_dma_src[0x200];
static UINT16 cop_dma_size[0x200];
static UINT16 cop_dma_dst[0x200];
static UINT16 cop_dma_fade_table;
static UINT16 cop_dma_trigger = 0;
static UINT16 cop_scale;

static UINT8 cop_rng_max_value;

static UINT16 copd2_offs = 0;

static void copd2_set_tableoffset(running_machine &machine, UINT16 data)
{
	//logerror("mcu_offs %04x\n", data);
	copd2_offs = data;
	if (copd2_offs>0xff)
	{
		logerror("copd2 offs > 0x100\n");
	}

	copd2_table_2[copd2_offs/8] = cop_438;
	copd2_table_3[copd2_offs/8] = cop_43a;
	copd2_table_4[copd2_offs/8] = cop_43c;
#if 0

    {
        FILE *fp;
        char filename[256];
        sprintf(filename,"copdat_%s.table2", machine.system().name);
        fp=fopen(filename, "w+b");
        if (fp)
        {
            fwrite(copd2_table_2, 0x200/8, 1, fp);
            fclose(fp);
        }
    }
    {
        FILE *fp;
        char filename[256];
        sprintf(filename,"copdat_%s.table3", machine.system().name);
        fp=fopen(filename, "w+b");
        if (fp)
        {
            fwrite(copd2_table_3, 0x200/8, 1, fp);
            fclose(fp);
        }
    }
    {
        FILE *fp;
        char filename[256];
        sprintf(filename,"copdat_%s.table4", machine.system().name);
        fp=fopen(filename, "w+b");
        if (fp)
        {
            fwrite(copd2_table_4, 0x200/8, 1, fp);
            fclose(fp);
        }
    }

	{
		int i;

		printf("start\n");

		for (i=0;i<0x20;i++)
		{
			int ii;
			printf("%02x | %01x | %04x | %04x | ", i, copd2_table_2[i], copd2_table_3[i], copd2_table_4[i]);


			for (ii=0;ii<0x8;ii++)
			{
				printf("%03x ", copd2_table[i*8 + ii]);

			}
			printf("\n");
		}

	}
#endif

}

static void copd2_set_tabledata(running_machine &machine, UINT16 data)
{
	copd2_table[copd2_offs] = data;
	//logerror("mcu_data %04x\n", data);
#if 0
    {
        FILE *fp;
        char filename[256];
        sprintf(filename,"copdat_%s.data", machine.system().name);
        fp=fopen(filename, "w+b");
        if (fp)
        {
            fwrite(copd2_table, 0x200, 1, fp);
            fclose(fp);
        }
    }
#endif
}

static UINT32 cop_register[8];
static UINT16 seibu_vregs[0x50/2];

static WRITE16_HANDLER( seibu_common_video_regs_w )
{
	legionna_state *state = space->machine().driver_data<legionna_state>();
	COMBINE_DATA(&seibu_vregs[offset]);

	switch(offset)
	{
		case (0x01a/2): { state->flip_screen_set(seibu_vregs[offset] & 0x01); break; }
		case (0x01c/2): { state->m_layer_disable =  seibu_vregs[offset]; break; }
		case (0x020/2): { state->m_scrollram16[0] = seibu_vregs[offset]; break; }
		case (0x022/2): { state->m_scrollram16[1] = seibu_vregs[offset]; break; }
		case (0x024/2): { state->m_scrollram16[2] = seibu_vregs[offset]; break; }
		case (0x026/2): { state->m_scrollram16[3] = seibu_vregs[offset]; break; }
		case (0x028/2): { state->m_scrollram16[4] = seibu_vregs[offset]; break; }
		case (0x02a/2): { state->m_scrollram16[5] = seibu_vregs[offset]; break; }
//		case (0x03a/2): Godzilla sets this up to be 0x1ef / 0x1eb, presumably bit 2 is vertical wrap-around on/off?
		default: { logerror("seibu_common_video_regs_w unhandled offset %02x %04x\n",offset*2,data); break; }
	}
}


/*
"The area of ASM snippets"

player-1 priorities list:
1086d8: show this sprite (bit 15)
1086dc: lives (BCD,bits 3,2,1,0)
1086de: energy bar (upper byte)
1086e0: walk animation (lower byte)
1086ec: "death" status (bit 15)
1086f4: sprite y axis
1086f0: sprite x axis

Sprite DMA TODO:
- various bits not yet understood in the sprite src tables and in the 0x400/0x402 sprite param;

spriteram DMA [1]
001DE4: 3086                     move.w  D6, (A0) ;$100400,color + other stuff
001DE6: 2440                     movea.l D0, A2
001DE8: 0269 0004 0002           andi.w  #$4, ($2,A1)
001DEE: 3152 000C                move.w  (A2), ($c,A0) ;DMA size
001DF2: 3145 0002                move.w  D5, ($2,A0)
001DF6: 0245 0040                andi.w  #$40, D5
001DFA: 2009                     move.l  A1, D0
001DFC: 3140 00C0                move.w  D0, ($c0,A0) ;RAM -> $1004c0 (work ram index?)
001E00: 4840                     swap    D0
001E02: 3140 00A0                move.w  D0, ($a0,A0) ;RAM -> $1004a0
001E06: 200A                     move.l  A2, D0
001E08: 3140 0014                move.w  D0, ($14,A0) ;$ROM lo -> $100414 src
001E0C: 4840                     swap    D0
001E0E: 3140 0012                move.w  D0, ($12,A0) ;$ROM hi -> $100412
001E12: 2679 0010 8116           movea.l $108116.l, A3 ;points to dst spriteram
001E18: 3839 0010 810A           move.w  $10810a.l, D4 ;spriteram index
001E1E: 260B                     move.l  A3, D3
001E20: 3143 00C8                move.w  D3, ($c8,A0) ;sets the dst spriteram
001E24: 4843                     swap    D3
001E26: 3143 00A8                move.w  D3, ($a8,A0)
001E2A: 45EA 0004                lea     ($4,A2), A2
//at this point we're ready for DMAing
001E2E: 317C A180 0100           move.w  #$a180, ($100,A0) ;<-get x/y from sprite
001E34: 317C 6980 0102           move.w  #$6980, ($102,A0) ;<-adjust sprite x/y
001E3A: 317C C480 0102           move.w  #$c480, ($102,A0) ;<-load sprite offset
001E40: 317C 0000 0010           move.w  #$0, ($10,A0)     ;<-do the job?
001E46: 302A 0002                move.w  ($2,A2), D0
001E4A: 816B 0006                or.w    D0, ($6,A3)
001E4E: 45EA 0006                lea     ($6,A2), A2
001E52: 302B 0008                move.w  ($8,A3), D0
001E56: B079 0010 8112           cmp.w   $108112.l, D0
001E5C: 6E00 0054                bgt     $1eb2
001E60: B079 0010 8110           cmp.w   $108110.l, D0
001E66: 6D00 004A                blt     $1eb2
001E6A: 026B 7FFF 000A           andi.w  #$7fff, ($a,A3)
001E70: 8B6B 0004                or.w    D5, ($4,A3)
001E74: 47EB 0008                lea     ($8,A3), A3
001E78: 260B                     move.l  A3, D3
001E7A: 3143 00C8                move.w  D3, ($c8,A0)
001E7E: 4843                     swap    D3
001E80: 3143 00A8                move.w  D3, ($a8,A0)
001E84: 5244                     addq.w  #1, D4
001E86: B879 0010 8114           cmp.w   $108114.l, D4
001E8C: 6500 000C                bcs     $1e9a
001E90: 0069 0002 0002           ori.w   #$2, ($2,A1)
001E96: 6000 000C                bra     $1ea4
001E9A: 3028 01B0                move.w  ($1b0,A0), D0 ;bit 1 = DMA job finished
001E9E: 0240 0002                andi.w  #$2, D0
001EA2: 6790                     beq     $1e34
001EA4: 33C4 0010 810A           move.w  D4, $10810a.l
001EAA: 23CB 0010 8116           move.l  A3, $108116.l
001EB0: 4E75                     rts

x/y check [2]
002030: E58D                     lsl.l   #2, D5
002032: 0685 0003 0000           addi.l  #$30000, D5
002038: 33C5 0010 04C4           move.w  D5, $1004c4.l
00203E: 4845                     swap    D5
002040: 33C5 0010 04A4           move.w  D5, $1004a4.l
002046: E58E                     lsl.l   #2, D6
002048: 0686 0003 0000           addi.l  #$30000, D6
00204E: 33C6 0010 04C6           move.w  D6, $1004c6.l
002054: 4846                     swap    D6
002056: 33C6 0010 04A6           move.w  D6, $1004a6.l
00205C: 33FC A180 0010 0500      move.w  #$a180, $100500.l
002064: 33FC B100 0010 0500      move.w  #$b100, $100500.l
00206C: 33FC A980 0010 0500      move.w  #$a980, $100500.l
002074: 33FC B900 0010 0500      move.w  #$b900, $100500.l
00207C: 4E75                     rts
[...]
//then reads at $580

sine cosine has a weird math problem, it needs that the amp is multiplied by two when the direction is TOTALLY left or TOTALLY up.
No known explanation to this so far ...

003306: move.w  #$8100, ($100,A0)
00330C: move.w  #$8900, ($100,A0)
003312: cmpi.w  #$80, ($36,A1) ;checks if angle is equal to 0x80 (left direction of objects)
003318: bne     $332a
00331C: move.l  ($14,A1), D0 ;divide by two if so
003320: asr.l   #1, D0
003322: move.l  D0, ($14,A1)
003326: bra     $333e
00332A: cmpi.w  #$c0, ($36,A1) ;checks if angle is equal to 0xc0 (up direction of objects)
003330: bne     $333e
003334: move.l  ($10,A1), D0 ;divide by two if so
003338: asr.l   #1, D0
00333A: move.l  D0, ($10,A1)
00333E: movem.l (A7)+, D0/A0-A1
003342: rts

*/

/********************************************************************************************

  COPX bootleg simulation
    - Seibu Cup Soccer (bootleg)

 *******************************************************************************************/

READ16_HANDLER( copdxbl_0_r )
{
	UINT16 retvalue = cop_mcu_ram[offset];

	switch(offset)
	{
		default:
		{
			logerror("%06x: COPX unhandled read returning %04x from offset %04x\n", space->device().safe_pc(), retvalue, offset*2);
			return retvalue;
		}

		//case (0x47e/2):
		//case (0x5b0/2):
		//case (0x5b4/2):
		//  return cop_mcu_ram[offset];

		case (0x700/2): return space->machine().root_device().ioport("DSW1")->read();
		case (0x704/2):	return space->machine().root_device().ioport("PLAYERS12")->read();
		case (0x708/2):	return space->machine().root_device().ioport("PLAYERS34")->read();
		case (0x70c/2):	return space->machine().root_device().ioport("SYSTEM")->read();
		case (0x71c/2): return space->machine().root_device().ioport("DSW2")->read();
	}
}

WRITE16_HANDLER( copdxbl_0_w )
{
	legionna_state *state = space->machine().driver_data<legionna_state>();
	COMBINE_DATA(&cop_mcu_ram[offset]);

	switch(offset)
	{
		default:
		{
			logerror("%06x: COPX unhandled write data %04x at offset %04x\n", space->device().safe_pc(), data, offset*2);
			break;
		}

		/*TODO: kludge on x-axis.*/
		case (0x660/2): { state->m_scrollram16[0] = cop_mcu_ram[offset] - 0x1f0; break; }
		case (0x662/2): { state->m_scrollram16[1] = cop_mcu_ram[offset]; break; }
		case (0x664/2): { state->m_scrollram16[2] = cop_mcu_ram[offset] - 0x1f0; break; }
		case (0x666/2): { state->m_scrollram16[3] = cop_mcu_ram[offset]; break; }
		case (0x668/2): { state->m_scrollram16[4] = cop_mcu_ram[offset] - 0x1f0; break; }
		case (0x66a/2): { state->m_scrollram16[5] = cop_mcu_ram[offset]; break; }
		case (0x66c/2): { state->m_scrollram16[6] = cop_mcu_ram[offset] - 0x1f0; break; }
		case (0x66e/2): { state->m_scrollram16[7] = cop_mcu_ram[offset]; break; }

		case (0x740/2):
		{
			state->soundlatch_byte_w(*space, 0, data & 0xff);
			space->machine().device("audiocpu")->execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE );
			break;
		}
	}
}

/* Generic COP functions
  -- the game specific handlers fall through to these if there
     isn't a specific case for them.  these implement behavior
     which seems common to all the agmes
*/

static UINT16 cop_status,cop_dist,cop_angle;
static UINT16 cop_hit_status;
static INT16 cop_hit_val_x,cop_hit_val_y,cop_hit_val_z,cop_hit_val_unk;
static UINT32 cop_sort_lookup,cop_sort_ram_addr,cop_sort_param;
static INT8 cop_angle_compare;
static UINT8 cop_angle_mod_val;
static struct
{
	int x,y;
	INT16 min_x,min_y,max_x,max_y;
	UINT16 hitbox;
	UINT16 hitbox_x,hitbox_y;
}cop_collision_info[2];

/* RE from Seibu Cup Soccer bootleg */
static const UINT8 fade_table(int v)
{
    int low  = v & 0x001f;
    int high = v & 0x03e0;

    return (low * (high | (high >> 5)) + 0x210) >> 10;
}

static UINT16 u1,u2;

#define COP_CMD(_1_,_2_,_3_,_4_,_5_,_6_,_7_,_8_,_u1_,_u2_) \
	(copd2_table[command+0] == _1_ && copd2_table[command+1] == _2_ && copd2_table[command+2] == _3_ && copd2_table[command+3] == _4_ && \
	copd2_table[command+4] == _5_ && copd2_table[command+5] == _6_ && copd2_table[command+6] == _7_ && copd2_table[command+7] == _8_ && \
	u1 == _u1_ && u2 == _u2_) \


/*
Godzilla 0x12c0 X = 0x21ed Y = 0x57da
Megaron  0x12d0 X = 0x1ef1 Y = 0x55db
King Ghidorah 0x12c8 X = 0x26eb Y = 0x55dc
Mecha Ghidorah 0x12dc X = 0x24ec Y = 0x55dc
Mecha Godzilla 0x12d4 X = 0x1cf1 Y = 0x52dc
Gigan 0x12cc X = 0x23e8 Y = 0x55db
*/
static void cop_take_hit_box_params(UINT8 offs)
{
	INT16 start_x,start_y,end_x,end_y;

	start_x = INT8(cop_collision_info[offs].hitbox_x);
	start_y = INT8(cop_collision_info[offs].hitbox_y);

	end_x = INT8(cop_collision_info[offs].hitbox_x >> 8);
	end_y = INT8(cop_collision_info[offs].hitbox_y >> 8);

	cop_collision_info[offs].min_x = start_x + (cop_collision_info[offs].x >> 16);
	cop_collision_info[offs].min_y = start_y + (cop_collision_info[offs].y >> 16);
	cop_collision_info[offs].max_x = end_x + (cop_collision_info[offs].x >> 16);
	cop_collision_info[offs].max_y = end_y + (cop_collision_info[offs].y >> 16);
}


static UINT8 cop_calculate_collsion_detection(running_machine &machine)
{
	static UINT8 res;

	res = 3;

	/* outbound X check */
	if(cop_collision_info[0].max_x >= cop_collision_info[1].min_x && cop_collision_info[0].min_x <= cop_collision_info[1].max_x)
		res &= ~2;

	/* outbound Y check */
	if(cop_collision_info[0].max_y >= cop_collision_info[1].min_y && cop_collision_info[0].min_y <= cop_collision_info[1].max_y)
		res &= ~1;

	cop_hit_val_x = (cop_collision_info[0].x - cop_collision_info[1].x) >> 16;
	cop_hit_val_y = (cop_collision_info[0].y - cop_collision_info[1].y) >> 16;
	cop_hit_val_z = 1;
	cop_hit_val_unk = res; // TODO: there's also bit 2 and 3 triggered in the tests, no known meaning


	//popmessage("%d %d %04x %04x %04x %04x",cop_hit_val_x,cop_hit_val_y,cop_collision_info[0].hitbox_x,cop_collision_info[0].hitbox_y,cop_collision_info[1].hitbox_x,cop_collision_info[1].hitbox_y);

	//if(res == 0)
	//popmessage("0:%08x %08x %08x 1:%08x %08x %08x\n",cop_collision_info[0].x,cop_collision_info[0].y,cop_collision_info[0].hitbox,cop_collision_info[1].x,cop_collision_info[1].y,cop_collision_info[1].hitbox);

	return res;
}

static READ16_HANDLER( generic_cop_r )
{
	UINT16 retvalue;
	retvalue = cop_mcu_ram[offset];


	switch (offset)
	{
		/* RNG max value readback, trusted */
		case 0x02c/2:
			return retvalue;

		/* DMA mode register readback, trusted */
		case 0x07e/2:
			return retvalue;

		case 0x180/2:
			return cop_hit_status;

		/* these two controls facing direction in Godzilla opponents (only vs.) - x value compare? */
		case 0x182/2:
			return (cop_hit_val_y);

		case 0x184/2:
			return (cop_hit_val_x);

		/* Legionnaire only - z value compare? */
		case 0x186/2:
			return (cop_hit_val_z);

		case 0x188/2:
			return cop_hit_val_unk;

		/* BCD */
		case 0x190/2:
		case 0x192/2:
		case 0x194/2:
		case 0x196/2:
		case 0x198/2:
			return retvalue;

		/* RNG, trusted */
		case 0x1a0/2:
		case 0x1a2/2:
		case 0x1a4/2:
		case 0x1a6/2:
			return space->machine().firstcpu->total_cycles() % (cop_rng_max_value+1);

		case 0x1b0/2:
			return cop_status;

		case 0x1b2/2:
			return cop_dist;

		case 0x1b4/2:
			return cop_angle;

		default:
			seibu_cop_log("%06x: COPX unhandled read returning %04x from offset %04x\n", space->device().safe_pc(), retvalue, offset*2);
			return retvalue;
	}
}

static UINT32 fill_val;
static UINT8 pal_brightness_val,pal_brightness_mode;
static UINT32 cop_sprite_dma_src;
static int cop_sprite_dma_abs_x,cop_sprite_dma_abs_y,cop_sprite_dma_size;
static UINT32 cop_sprite_dma_param;

static WRITE16_HANDLER( generic_cop_w )
{
	UINT32 temp32;

	switch (offset)
	{
		default:
			seibu_cop_log("%06x: COPX unhandled write data %04x at offset %04x\n", space->device().safe_pc(), data, offset*2);
			break;

		/* Sprite DMA */
		case (0x000/2):
		case (0x002/2):
			cop_sprite_dma_param = (cop_mcu_ram[0x000/2]) | (cop_mcu_ram[0x002/2] << 16);
			//popmessage("%08x",cop_sprite_dma_param & 0xffffffc0);
			break;

		case (0x00c/2): { cop_sprite_dma_size = cop_mcu_ram[offset]; break; }
		case (0x010/2):
		{
			if(data)
				printf("Warning: COP RAM 0x410 used with %04x\n",data);
			else
			{
				/* guess */
				cop_register[4]+=8;
				cop_sprite_dma_src+=6;

				cop_sprite_dma_size--;

				if(cop_sprite_dma_size > 0)
					cop_status &= ~2;
				else
					cop_status |= 2;
			}
			break;
		}

		case (0x012/2):
		case (0x014/2):
			cop_sprite_dma_src = (cop_mcu_ram[0x014/2]) | (cop_mcu_ram[0x012/2] << 16);
			break;

		/* triggered before 0x6200 in Seibu Cup, looks like an angle value ... */
		case (0x01c/2): cop_angle_compare = cop_mcu_ram[0x1c/2] & 0xff;	break;
		case (0x01e/2): cop_angle_mod_val = cop_mcu_ram[0x1e/2] & 0xff; break;

		case (0x08c/2): cop_sprite_dma_abs_y = (cop_mcu_ram[0x08c/2]); break;
		case (0x08e/2): cop_sprite_dma_abs_x = (cop_mcu_ram[0x08e/2]); break;

		/* BCD Protection */
		case (0x020/2):
		case (0x022/2):
			temp32 = (cop_mcu_ram[0x020/2]) | (cop_mcu_ram[0x022/2] << 16);
			cop_mcu_ram[0x190/2] = (((temp32 / 1) % 10) + (((temp32 / 10) % 10) << 8) + 0x3030);
			cop_mcu_ram[0x192/2] = (((temp32 / 100) % 10) + (((temp32 / 1000) % 10) << 8) + 0x3030);
			cop_mcu_ram[0x194/2] = (((temp32 / 10000) % 10) + (((temp32 / 100000) % 10) << 8) + 0x3030);
			cop_mcu_ram[0x196/2] = (((temp32 / 1000000) % 10) + (((temp32 / 10000000) % 10) << 8) + 0x3030);
			cop_mcu_ram[0x198/2] = (((temp32 / 100000000) % 10) + (((temp32 / 1000000000) % 10) << 8) + 0x3030);
			break;
		case (0x024/2):
			/*
            This looks like a register for the BCD...
            Godzilla and Heated Barrel sets 3
            Denjin Makai sets 3 at start-up and toggles between 2 and 3 during gameplay on the BCD subroutine
            SD Gundam sets 0
            */
			break;

		case (0x028/2):
		case (0x02a/2):
			fill_val = (cop_mcu_ram[0x028/2]) | (cop_mcu_ram[0x02a/2] << 16);
			break;

		/* Command tables for 0x500 / 0x502 commands */
		case (0x032/2): { copd2_set_tabledata(space->machine(), data); break; }
		case (0x034/2): { copd2_set_tableoffset(space->machine(), data); break; }
		case (0x038/2):	{ cop_438 = data; break; }
		case (0x03a/2):	{ cop_43a = data; break; }
		case (0x03c/2): { cop_43c = data; break; }
		case (0x03e/2):
			/*
			0 in all 68k based games
			0xffff in raiden2 / raidendx
			0x2000 in zeroteam / xsedae
			it's always setted up just before the 0x474 register
			*/
			break;

		/* brightness control */
		case (0x05a/2): pal_brightness_val = data & 0xff; break;
		case (0x05c/2): pal_brightness_mode = data & 0xff; break;

		/* DMA / layer clearing section */
		case (0x074/2):
			/*
			This sets up a DMA mode of some sort
				0x0e00: grainbow, cupsoc
				0x0a00: legionna, godzilla, denjinmk
				0x0600: heatbrl
				0x1e00: zeroteam, xsedae
			raiden2 and raidendx doesn't set this up, this could indicate that this is related to the non-private buffer DMAs
			(both only uses 0x14 and 0x15 as DMAs)
			*/
			break;

		/* used in palette DMAs, for fading effects */
		case (0x076/2):
			cop_dma_fade_table = data;
			break;

		case (0x078/2): /* DMA source address */
		{
			cop_dma_src[cop_dma_trigger] = data; // << 6 to get actual address
			//seibu_cop_log("%06x: COPX set layer clear address to %04x (actual %08x)\n", space->device().safe_pc(), data, data<<6);
			break;
		}

		case (0x07a/2): /* DMA length */
		{
			cop_dma_size[cop_dma_trigger] = data;
			//seibu_cop_log("%06x: COPX set layer clear length to %04x (actual %08x)\n", space->device().safe_pc(), data, data<<5);
			break;
		}

		case (0x07c/2): /* DMA destination */
		{
			cop_dma_dst[cop_dma_trigger] = data;
			//seibu_cop_log("%06x: COPX set layer clear value to %04x (actual %08x)\n", space->device().safe_pc(), data, data<<6);
			break;
		}

		case (0x07e/2): /* DMA parameter */
		{
			cop_dma_trigger = data;
			//seibu_cop_log("%06x: COPX set layer clear trigger? to %04x\n", space->device().safe_pc(), data);
			if (data>=0x1ff)
			{
				seibu_cop_log("invalid DMA trigger!, >0x1ff\n");
				cop_dma_trigger = 0;
			}

			break;
		}

		/* max possible value returned by the RNG at 0x5a*, trusted */
		case (0x02c/2): cop_rng_max_value = cop_mcu_ram[0x2c/2] & 0xff; break;

		case (0x044/2):
		{
			/*TODO: this appears to control trigonometry maths, but all games here doesn't seem to like current implementation ... */
			cop_scale = 1;
			if(data == 4)
				cop_scale = 0;
			break;
		}

		/* Registers */
		case (0x0a0/2): { cop_register[0] = (cop_register[0]&0x0000ffff)|(cop_mcu_ram[offset]<<16); break; }
		case (0x0c0/2): { cop_register[0] = (cop_register[0]&0xffff0000)|(cop_mcu_ram[offset]<<0);  break; }

		case (0x0a2/2): { cop_register[1] = (cop_register[1]&0x0000ffff)|(cop_mcu_ram[offset]<<16); break; }
		case (0x0c2/2): { cop_register[1] = (cop_register[1]&0xffff0000)|(cop_mcu_ram[offset]<<0);  break; }

		case (0x0a4/2): { cop_register[2] = (cop_register[2]&0x0000ffff)|(cop_mcu_ram[offset]<<16); break; }
		case (0x0c4/2): { cop_register[2] = (cop_register[2]&0xffff0000)|(cop_mcu_ram[offset]<<0);  break; }

		case (0x0a6/2): { cop_register[3] = (cop_register[3]&0x0000ffff)|(cop_mcu_ram[offset]<<16); break; }
		case (0x0c6/2): { cop_register[3] = (cop_register[3]&0xffff0000)|(cop_mcu_ram[offset]<<0);  break; }

		case (0x0a8/2): { cop_register[4] = (cop_register[4]&0x0000ffff)|(cop_mcu_ram[offset]<<16); break; }
		case (0x0c8/2): { cop_register[4] = (cop_register[4]&0xffff0000)|(cop_mcu_ram[offset]<<0);  break; }

		case (0x0aa/2): { cop_register[5] = (cop_register[5]&0x0000ffff)|(cop_mcu_ram[offset]<<16); break; }
		case (0x0ca/2): { cop_register[5] = (cop_register[5]&0xffff0000)|(cop_mcu_ram[offset]<<0);  break; }

		case (0x0ac/2): { cop_register[6] = (cop_register[6]&0x0000ffff)|(cop_mcu_ram[offset]<<16); break; }
		case (0x0cc/2): { cop_register[6] = (cop_register[6]&0xffff0000)|(cop_mcu_ram[offset]<<0);  break; }


		case (0x100/2):
		case (0x102/2):
		case (0x104/2):
		{
			int i;
			int command;

			#if LOG_CMDS
			seibu_cop_log("%06x: COPX execute table macro command %04x %04x | regs %08x %08x %08x %08x %08x\n", space->device().safe_pc(), data, cop_mcu_ram[offset], cop_register[0], cop_register[1], cop_register[2], cop_register[3], cop_register[4]);
			#endif

			command = -1;
			/* search the uploaded 'trigger' table for a matching trigger*/
			/* note, I don't know what the 'mask' or 'value' tables are... probably important, might determine what actually gets executed! */
			/* note: Zero Team triggers macro 0x904 instead of 0x905, Seibu Cup Soccer triggers 0xe30e instead of 0xe38e. I highly doubt that AT LEAST
               it isn't supposed to do anything, especially in the former case (it definitely NEED that sprites have an ark movement when they are knocked down). */
			for (i=0;i<32;i++)
			{
				if ((cop_mcu_ram[offset] & 0xff00) == (copd2_table_4[i] & 0xff00))
				{
					#if LOG_CMDS
					seibu_cop_log("    Cop Command %04x found in slot %02x with other params %04x %04x\n", cop_mcu_ram[offset], i, copd2_table_2[i], copd2_table_3[i]);
					#endif

					u1 = copd2_table_2[i] & 0x000f;
					u2 = copd2_table_3[i] & 0xffff;
					command = i;
				}
			}

			if (command==-1)
			{
				seibu_cop_log("    Cop Command %04x NOT IN TABLE!\n", cop_mcu_ram[offset]);
				break;
			}
			else
			{
				command*=0x8;
				#if LOG_CMDS
				{
					int j;
					seibu_cop_log("     Sequence: ");
					for (j=0;j<0x8;j++)
					{
						seibu_cop_log("%04x ", copd2_table[command+j]);
					}
					seibu_cop_log("\n");
				}
				#endif
			}

			//printf("%04x %04x %04x\n",cop_mcu_ram[offset],u1,u2);

			cop_status &= 0x7fff;

			/*
            Macro notes:
            - endianess changes from/to Raiden 2:
              dword ^= 0
              word ^= 2
              byte ^= 3
            - some macro commands here have a commented algorithm, it's how Seibu Cup Bootleg version handles maths inside the 14/15 roms.
              The ROMs map tables in the following arrangement:
              0x00000 - 0x1ffff Sine math results
              0x20000 - 0x3ffff Cosine math results
              0x40000 - 0x7ffff Division math results
              0x80000 - 0xfffff Pythagorean theorem, hypotenuse length math results
              Surprisingly atan maths are nowhere to be found from the roms.
            */

			/* "automatic" movement */
			if(COP_CMD(0x188,0x282,0x082,0xb8e,0x98e,0x000,0x000,0x000,6,0xffeb))
			{
				UINT8 offs;

				offs = (offset & 3) * 4;

				space->write_dword(cop_register[0] + 4 + offs, space->read_dword(cop_register[0] + 4 + offs) + space->read_dword(cop_register[0] + 16 + offs));
				return;
			}

			/* "automatic" movement, for arks in Legionnaire / Zero Team (expression adjustment) */
			if(COP_CMD(0x194,0x288,0x088,0x000,0x000,0x000,0x000,0x000,6,0xfbfb))
			{
				UINT8 offs;

				offs = (offset & 3) * 4;

				//popmessage("%d %d",space->read_dword(cop_register[0] + 0x2c + 0),space->read_dword(cop_register[0] + 0x2c + 4));

				space->write_dword(cop_register[0] + 16 + offs, space->read_dword(cop_register[0] + 16 + offs) + space->read_dword(cop_register[0] + 0x28 + offs));
				return;
			}

			/* SINE math - 0x8100 */
			/* FIXME: cop scale is unreliable */
			/*
                 00000-0ffff:
                   amp = x/256
                   ang = x & 255
                   s = sin(ang*2*pi/256)
                   val = trunc(s*amp)
                   if(s<0)
                     val--
                   if(s == 192)
                     val = -2*amp
            */
			if(COP_CMD(0xb9a,0xb88,0x888,0x000,0x000,0x000,0x000,0x000,7,0xfdfb))
			{
				int raw_angle = (space->read_word(cop_register[0]+(0x34^2)) & 0xff);
				double angle = raw_angle * M_PI / 128;
				double amp = 65536*(space->read_word(cop_register[0]+(0x36^2)) & 0xff);

				/* TODO: up direction, why? */
				if(raw_angle == 0xc0)
					amp*=2;

				space->write_dword(cop_register[0] + 16, int(amp*sin(angle)) >> (5-cop_scale));
				return;
			}

			/* COSINE math - 0x8900 */
			/* FIXME: cop scale is unreliable */
			/*
             10000-1ffff:
               amp = x/256
               ang = x & 255
               s = cos(ang*2*pi/256)
               val = trunc(s*amp)
               if(s<0)
                 val--
               if(s == 128)
                 val = -2*amp
            */
			if(COP_CMD(0xb9a,0xb8a,0x88a,0x000,0x000,0x000,0x000,0x000,7,0xfdfb))
			{
				int raw_angle = (space->read_word(cop_register[0]+(0x34^2)) & 0xff);
				double angle = raw_angle * M_PI / 128;
				double amp = 65536*(space->read_word(cop_register[0]+(0x36^2)) & 0xff);

				/* TODO: left direction, why? */
				if(raw_angle == 0x80)
					amp*=2;

				space->write_dword(cop_register[0] + 20, int(amp*cos(angle)) >> (5-cop_scale));
				return;
			}

			/* 0x130e / 0x138e */
			if(COP_CMD(0x984,0xaa4,0xd82,0xaa2,0x39b,0xb9a,0xb9a,0xa9a,5,0xbf7f))
			{
				int dx = space->read_dword(cop_register[1]+4) - space->read_dword(cop_register[0]+4);
				int dy = space->read_dword(cop_register[1]+8) - space->read_dword(cop_register[0]+8);

				if(!dy) {
					cop_status |= 0x8000;
					cop_angle = 0;
				} else {
					cop_angle = atan(double(dx)/double(dy)) * 128 / M_PI;
					if(dy<0)
						cop_angle += 0x80;
				}

				space->write_byte(cop_register[0]+(0x34^3), cop_angle);
				return;
			}

			/* Pythagorean theorem, hypotenuse direction - 130e / 138e */
			//(heatbrl)  | 5 | bf7f | 138e | 984 aa4 d82 aa2 39b b9a b9a b9a
			if(COP_CMD(0x984,0xaa4,0xd82,0xaa2,0x39b,0xb9a,0xb9a,0xb9a,5,0xbf7f))
			{
				int dx = space->read_dword(cop_register[1]+4) - space->read_dword(cop_register[0]+4);
				int dy = space->read_dword(cop_register[1]+8) - space->read_dword(cop_register[0]+8);
				if(!dy) {
					cop_status |= 0x8000;
					cop_angle = 0;
				} else {
					cop_angle = atan(double(dx)/double(dy)) * 128 / M_PI;
					if(dy<0)
						cop_angle += 0x80;
				}

				/* TODO: bit 7 of macro command says if we have to write on work RAM */
				//if(0)
					space->write_byte(cop_register[0]+(0x34^3), cop_angle);
				return;
			}

			/* Pythagorean theorem, hypotenuse length - 0x3bb0 */
			//07 | 4 | 007f | 3bb0 | f9c b9c b9c b9c b9c b9c b9c 99c
			//(grainbow) | 4 | 007f | 3bb0 | f9c b9c b9c b9c b9c b9c b9c 99c
			/*
             40000-7ffff:
               v1 = (x / 32768)*64
               v2 = (x & 255)*32767/255
               val = sqrt(v1*v1+v2*v2) (unsigned)
            */
			if(COP_CMD(0xf9c,0xb9c,0xb9c,0xb9c,0xb9c,0xb9c,0xb9c,0x99c,4,0x007f))
			{
				int dx = space->read_dword(cop_register[1]+4) - space->read_dword(cop_register[0]+4);
				int dy = space->read_dword(cop_register[1]+8) - space->read_dword(cop_register[0]+8);

				dx = dx >> 16;
				dy = dy >> 16;
				cop_dist = sqrt((double)(dx*dx+dy*dy));

				/* TODO: bit 7 of macro command says if we have to write on work RAM */
				space->write_word(cop_register[0]+(0x38^2), cop_dist);
				return;
			}

			/* Division - 0x42c2 */
			/*
             20000-2ffff:
               v1 = x / 1024
               v2 = x & 1023
               val = !v1 ? 32767 : trunc(v2/v1+0.5)
             30000-3ffff:
               v1 = x / 1024
               v2 = (x & 1023)*32
               val = !v1 ? 32767 : trunc(v2/v1+0.5)
            */
			if(COP_CMD(0xf9a,0xb9a,0xb9c,0xb9c,0xb9c,0x29c,0x000,0x000,5,0xfcdd))
			{
				int div = space->read_word(cop_register[0]+(0x36^2));
				if(!div)
					div = 1;

				space->write_word(cop_register[0]+(0x38^2), (space->read_word(cop_register[0]+(0x38^2)) << (5-cop_scale)) / div);
				return;
			}

			/*
                collision detection:

                int dy_0 = space->read_dword(cop_register[0]+4);
                int dx_0 = space->read_dword(cop_register[0]+8);
                int dy_1 = space->read_dword(cop_register[1]+4);
                int dx_1 = space->read_dword(cop_register[1]+8);
                int hitbox_param1 = space->read_dword(cop_register[2]);
                int hitbox_param2 = space->read_dword(cop_register[3]);

                TODO: we are ignoring the u1 / u2 params for now
            */

			if(COP_CMD(0xb80,0xb82,0xb84,0xb86,0x000,0x000,0x000,0x000,u1,u2))
			{
				cop_collision_info[0].y = (space->read_dword(cop_register[0]+4));
				cop_collision_info[0].x = (space->read_dword(cop_register[0]+8));
				return;
			}

			//(heatbrl)  | 9 | ffff | b080 | b40 bc0 bc2
			if(COP_CMD(0xb40,0xbc0,0xbc2,0x000,0x000,0x000,0x000,0x000,u1,u2))
			{
				cop_collision_info[0].hitbox = space->read_word(cop_register[2]);
				cop_collision_info[0].hitbox_y = space->read_word((cop_register[2]&0xffff0000)|(cop_collision_info[0].hitbox));
				cop_collision_info[0].hitbox_x = space->read_word(((cop_register[2]&0xffff0000)|(cop_collision_info[0].hitbox))+2);

				/* do the math */
				cop_take_hit_box_params(0);
				cop_hit_status = cop_calculate_collsion_detection(space->machine());

				return;
			}

			if(COP_CMD(0xba0,0xba2,0xba4,0xba6,0x000,0x000,0x000,0x000,u1,u2))
			{
				cop_collision_info[1].y = (space->read_dword(cop_register[1]+4));
				cop_collision_info[1].x = (space->read_dword(cop_register[1]+8));
				return;
			}

			//(heatbrl)  | 6 | ffff | b880 | b60 be0 be2
			if(COP_CMD(0xb60,0xbe0,0xbe2,0x000,0x000,0x000,0x000,0x000,u1,u2))
			{
				cop_collision_info[1].hitbox = space->read_word(cop_register[3]);
				cop_collision_info[1].hitbox_y = space->read_word((cop_register[3]&0xffff0000)|(cop_collision_info[1].hitbox));
				cop_collision_info[1].hitbox_x = space->read_word(((cop_register[3]&0xffff0000)|(cop_collision_info[1].hitbox))+2);

				/* do the math */
				cop_take_hit_box_params(1);
				cop_hit_status = cop_calculate_collsion_detection(space->machine());
				return;
			}

			// grainbow 0d | a | fff3 | 6980 | b80 ba0
			if(COP_CMD(0xb80,0xba0,0x000,0x000,0x000,0x000,0x000,0x000,10,0xfff3))
			{
				UINT8 offs;
				int abs_x,abs_y,rel_xy;

				offs = (offset & 3) * 4;

				/* TODO: I really suspect that following two are actually taken from the 0xa180 macro command then internally loaded */
				abs_x = space->read_word(cop_register[0] + 8) - cop_sprite_dma_abs_x;
				abs_y = space->read_word(cop_register[0] + 4) - cop_sprite_dma_abs_y;
				rel_xy = space->read_word(cop_sprite_dma_src + 4 + offs);

				//if(rel_xy & 0x0706)
				//  printf("sprite rel_xy = %04x\n",rel_xy);

				if(rel_xy & 1)
					space->write_word(cop_register[4] + offs + 4,0xc0 + abs_x - (rel_xy & 0xf8));
				else
					space->write_word(cop_register[4] + offs + 4,(((rel_xy & 0x78) + (abs_x) - ((rel_xy & 0x80) ? 0x80 : 0))));

				space->write_word(cop_register[4] + offs + 6,(((rel_xy & 0x7800) >> 8) + (abs_y) - ((rel_xy & 0x8000) ? 0x80 : 0)));
				return;
			}

			// grainbow 18 | a | ff00 | c480 | 080 882
			if(COP_CMD(0x080,0x882,0x000,0x000,0x000,0x000,0x000,0x000,10,0xff00))
			{
				UINT8 offs;

				offs = (offset & 3) * 4;

				space->write_word(cop_register[4] + offs + 0,space->read_word(cop_sprite_dma_src + offs) + (cop_sprite_dma_param & 0x3f));
				//space->write_word(cop_register[4] + offs + 2,space->read_word(cop_sprite_dma_src+2 + offs));
				return;
			}

			// cupsoc 1b | 5 | 7ff7 | dde5 | f80 aa2 984 0c2
			/* radar x/y positions */
			/* FIXME: x/ys are offsetted */
			if(COP_CMD(0xf80,0xaa2,0x984,0x0c2,0x000,0x000,0x000,0x000,5,0x7ff7))
			{
				UINT8 offs;
				int div;
//              INT16 offs_val;

				//printf("%08x %08x %08x %08x %08x %08x %08x\n",cop_register[0],cop_register[1],cop_register[2],cop_register[3],cop_register[4],cop_register[5],cop_register[6]);

				offs = (offset & 3) * 4;

				div = space->read_word(cop_register[4] + offs) + 1;
//              offs_val = space->read_word(cop_register[3] + offs);
				//420 / 180 = 500 : 400 = 30 / 50 = 98 / 18

				if(div == 0) { div = 1; }

				space->write_word((cop_register[6] + offs + 4), ((space->read_word(cop_register[5] + offs + 4)) / div));
				return;
			}

			//(cupsoc)   | 8 | f3e7 | 6200 | 3a0 3a6 380 aa0 2a6
			if(COP_CMD(0x3a0,0x3a6,0x380,0xaa0,0x2a6,0x000,0x000,0x000,8,0xf3e7))
			{
				INT8 cur_angle;

				cur_angle = space->read_byte(cop_register[1] + (0xc ^ 3));
				space->write_byte(cop_register[1] + (0^3),space->read_byte(cop_register[1] + (0^3)) & 0xfb); //correct?

				if(cur_angle >= cop_angle_compare)
				{
					cur_angle -= cop_angle_mod_val;
					if(cur_angle <= cop_angle_compare)
					{
						cur_angle = cop_angle_compare;
						space->write_byte(cop_register[1] + (0^3),space->read_byte(cop_register[1] + (0^3)) | 2);
					}
				}
				else if(cur_angle <= cop_angle_compare)
				{
					cur_angle += cop_angle_mod_val;
					if(cur_angle >= cop_angle_compare)
					{
						cur_angle = cop_angle_compare;
						space->write_byte(cop_register[1] + (0^3),space->read_byte(cop_register[1] + (0^3)) | 2);
					}
				}

				space->write_byte(cop_register[1] + (0xc ^ 3),cur_angle);
				return;
			}

			//(grainbow) | 8 | f3e7 | 6200 | 380 39a 380 a80 29a
			/* search direction, used on SD Gundam homing weapon */
			/* FIXME: still doesn't work ... */
			if(COP_CMD(0x380,0x39a,0x380,0xa80,0x29a,0x000,0x000,0x000,8,0xf3e7))
			{
				INT8 cur_angle;

				cur_angle = space->read_byte(cop_register[0] + (0x34 ^ 3));
				//space->write_byte(cop_register[0] + (0^3),space->read_byte(cop_register[0] + (0^3)) & 0xfb); //correct?

				if(cur_angle >= cop_angle_compare)
				{
					cur_angle -= cop_angle_mod_val;

					if(cur_angle <= cop_angle_compare)
					{
						cur_angle = cop_angle_compare;
						//space->write_byte(cop_register[0] + (0^3),space->read_byte(cop_register[0] + (0^3)) | 2);
					}
				}
				else if(cur_angle <= cop_angle_compare)
				{
					cur_angle += cop_angle_mod_val;

					if(cur_angle >= cop_angle_compare)
					{
						cur_angle = cop_angle_compare;
						//space->write_byte(cop_register[0] + (0^3),space->read_byte(cop_register[0] + (0^3)) | 2);
					}
				}

				space->write_byte(cop_register[0] + (0x34 ^ 3),cur_angle);
				return;
			}

			printf("%04x\n",cop_mcu_ram[offset]);
			break;
		}

		/* DMA go register */
		case (0x2fc/2):
		{
			//seibu_cop_log("%06x: COPX execute current layer clear??? %04x\n", space->device().safe_pc(), data);

			if (cop_dma_trigger >= 0x80 && cop_dma_trigger <= 0x87)
			{
				UINT32 src,dst,size,i;

				/*
                Apparently all of those are just different DMA channels, brightness effects are done through a RAM table and the pal_brightness_val / mode
                0x80 is used by Legionnaire
                0x81 is used by SD Gundam and Godzilla
                0x82 is used by Zero Team and X Se Dae
                0x86 is used by Seibu Cup Soccer
                0x87 is used by Denjin Makai

                TODO:
                - Denjin Makai mode 4 is totally guessworked.
                - SD Gundam doesn't fade colors correctly, it should have the text layer / sprites with normal gradient and the rest dimmed in most cases,
                  presumably bad RAM table or bad algorithm
                */

				//if(dma_trigger != 0x87)
				//printf("SRC: %08x %08x DST:%08x SIZE:%08x TRIGGER: %08x %02x %02x\n",cop_dma_src[cop_dma_trigger] << 6,cop_dma_fade_table * 0x400,cop_dma_dst[cop_dma_trigger] << 6,cop_dma_size[cop_dma_trigger] << 5,cop_dma_trigger,pal_brightness_val,pal_brightness_mode);

				src = (cop_dma_src[cop_dma_trigger] << 6);
				dst = (cop_dma_dst[cop_dma_trigger] << 6);
				size = ((cop_dma_size[cop_dma_trigger] << 5) - (cop_dma_dst[cop_dma_trigger] << 6) + 0x20)/2;

				for(i = 0;i < size;i++)
				{
					UINT16 pal_val;
					int r,g,b;
					int rt,gt,bt;

					if(pal_brightness_mode == 5)
					{
						bt = ((space->read_word(src + (cop_dma_fade_table * 0x400))) & 0x7c00) >> 5;
						bt = fade_table(bt|(pal_brightness_val ^ 0));
						b = ((space->read_word(src)) & 0x7c00) >> 5;
						b = fade_table(b|(pal_brightness_val ^ 0x1f));
						pal_val = ((b + bt) & 0x1f) << 10;
						gt = ((space->read_word(src + (cop_dma_fade_table * 0x400))) & 0x03e0);
						gt = fade_table(gt|(pal_brightness_val ^ 0));
						g = ((space->read_word(src)) & 0x03e0);
						g = fade_table(g|(pal_brightness_val ^ 0x1f));
						pal_val |= ((g + gt) & 0x1f) << 5;
						rt = ((space->read_word(src + (cop_dma_fade_table * 0x400))) & 0x001f) << 5;
						rt = fade_table(rt|(pal_brightness_val ^ 0));
						r = ((space->read_word(src)) & 0x001f) << 5;
						r = fade_table(r|(pal_brightness_val ^ 0x1f));
						pal_val |= ((r + rt) & 0x1f);
					}
					else if(pal_brightness_mode == 4) //Denjin Makai
					{
						bt =(space->read_word(src + (cop_dma_fade_table * 0x400)) & 0x7c00) >> 10;
						b = (space->read_word(src) & 0x7c00) >> 10;
						gt =(space->read_word(src + (cop_dma_fade_table * 0x400)) & 0x03e0) >> 5;
						g = (space->read_word(src) & 0x03e0) >> 5;
						rt =(space->read_word(src + (cop_dma_fade_table * 0x400)) & 0x001f) >> 0;
						r = (space->read_word(src) & 0x001f) >> 0;

						if(pal_brightness_val == 0x10)
							pal_val = bt << 10 | gt << 5 | rt << 0;
						else if(pal_brightness_val == 0xff) // TODO: might be the back plane or it still doesn't do any mod, needs PCB tests
							pal_val = 0;
						else
						{
							bt = fade_table(bt<<5|((pal_brightness_val*2) ^ 0));
							b =  fade_table(b<<5|((pal_brightness_val*2) ^ 0x1f));
							pal_val = ((b + bt) & 0x1f) << 10;
							gt = fade_table(gt<<5|((pal_brightness_val*2) ^ 0));
							g =  fade_table(g<<5|((pal_brightness_val*2) ^ 0x1f));
							pal_val |= ((g + gt) & 0x1f) << 5;
							rt = fade_table(rt<<5|((pal_brightness_val*2) ^ 0));
							r =  fade_table(r<<5|((pal_brightness_val*2) ^ 0x1f));
							pal_val |= ((r + rt) & 0x1f);
						}
					}
					else
					{
						printf("Warning: palette DMA used with mode %02x!\n",pal_brightness_mode);
						pal_val = space->read_word(src);
					}

					space->write_word(dst, pal_val);
					src+=2;
					dst+=2;
				}

				return;
			}

			/* Seibu Cup Soccer trigger this*/
			if (cop_dma_trigger == 0x0e)
			{
				UINT32 src,dst,size,i;

				src = (cop_dma_src[cop_dma_trigger] << 6);
				dst = (cop_dma_dst[cop_dma_trigger] << 6);
				size = ((cop_dma_size[cop_dma_trigger] << 5) - (cop_dma_dst[cop_dma_trigger] << 6) + 0x20)/2;

				for(i = 0;i < size;i++)
				{
					space->write_word(dst, space->read_word(src));
					src+=2;
					dst+=2;
				}

				return;
			}

			/* do the fill  */
			if (cop_dma_trigger >= 0x118 && cop_dma_trigger <= 0x11f)
			{
				UINT32 length, address;
				int i;
				if(cop_dma_dst[cop_dma_trigger] != 0x0000) // Invalid?
					return;

				address = (cop_dma_src[cop_dma_trigger] << 6);
				length = (cop_dma_size[cop_dma_trigger]+1) << 5;

				//printf("%08x %08x\n",address,length);

				for (i=address;i<address+length;i+=4)
				{
					space->write_dword(i, fill_val);
				}

				return;
			}

			/* Godzilla specific */
			if (cop_dma_trigger == 0x116)
			{
				UINT32 length, address;
				int i;

				//if(cop_dma_dst[cop_dma_trigger] != 0x0000) // Invalid?
				//  return;

				address = (cop_dma_src[cop_dma_trigger] << 6);
				length = ((cop_dma_size[cop_dma_trigger]+1) << 4);

				for (i=address;i<address+length;i+=4)
				{
					space->write_dword(i, fill_val);
				}

				return;
			}

			/* private buffer copies */
			if ((cop_dma_trigger==0x14) || (cop_dma_trigger==0x15)) return;

			printf("SRC: %08x %08x DST:%08x SIZE:%08x TRIGGER: %08x\n",cop_dma_src[cop_dma_trigger] << 6,cop_dma_fade_table,cop_dma_dst[cop_dma_trigger] << 6,cop_dma_size[cop_dma_trigger] << 5,cop_dma_trigger);

			break;
		}

		/* sort-DMA, oh my ... */
		case (0x054/2): { cop_sort_lookup = (cop_sort_lookup&0x0000ffff)|(cop_mcu_ram[offset]<<16); break; }
		case (0x056/2): { cop_sort_lookup = (cop_sort_lookup&0xffff0000)|(cop_mcu_ram[offset]<<0);  break; }
		case (0x050/2): { cop_sort_ram_addr = (cop_sort_ram_addr&0x0000ffff)|(cop_mcu_ram[offset]<<16); break; }
		case (0x052/2): { cop_sort_ram_addr = (cop_sort_ram_addr&0xffff0000)|(cop_mcu_ram[offset]<<0);  break; }
		case (0x058/2): { cop_sort_param = cop_mcu_ram[offset]; break; }

		case (0x2fe/2):
		{
			UINT16 sort_size;

			sort_size = cop_mcu_ram[offset];

			{
				int i,j;
				UINT8 xchg_flag;
				UINT32 addri,addrj;
				UINT16 vali,valj;

				/* TODO: use a better algorithm than bubble sort! */
				for(i=2;i<sort_size;i+=2)
				{
					for(j=i-2;j<sort_size;j+=2)
					{
						addri = cop_sort_ram_addr+space->read_word(cop_sort_lookup+i);
						addrj = cop_sort_ram_addr+space->read_word(cop_sort_lookup+j);

						vali = space->read_word(addri);
						valj = space->read_word(addrj);

						//printf("%08x %08x %04x %04x\n",addri,addrj,vali,valj);

						switch(cop_sort_param)
						{
							case 2:	xchg_flag = (vali > valj); break;
							case 1: xchg_flag = (vali < valj); break;
							default: xchg_flag = 0; printf("Warning: sort-DMA used with param %02x\n",cop_sort_param); break;
						}

						if(xchg_flag)
						{
							UINT16 xch_val;

							xch_val = space->read_word(cop_sort_lookup+i);
							space->write_word(cop_sort_lookup+i,space->read_word(cop_sort_lookup+j));
							space->write_word(cop_sort_lookup+j,xch_val);
						}
					}
				}
			}
			//else

			break;
		}
	}
}

/**********************************************************************************************
  Heated Barrel
**********************************************************************************************/

READ16_HANDLER( heatbrl_mcu_r )
{
	if(offset >= 0x3c0/2 && offset <= 0x3df/2)
		return seibu_main_word_r(space,(offset >> 1) & 7,0xffff);

	if(offset >= 0x340/2 && offset <= 0x34f/2)
	{
		static const char *const portnames[] = { "DSW1", "PLAYERS12", "PLAYERS34", "SYSTEM" };

		return space->machine().root_device().ioport(portnames[(offset >> 1) & 3])->read();
	}

	return generic_cop_r(space, offset, mem_mask);
}

WRITE16_HANDLER( heatbrl_mcu_w )
{
	COMBINE_DATA(&cop_mcu_ram[offset]);

	/* external pin register, used for banking */
	if(offset == 0x070/2)
	{
		heatbrl_setgfxbank(space->machine(), cop_mcu_ram[offset]);
		return;
	}

	if(offset == 0x200/2) //irq ack / sprite buffering?
		return;

	if(offset >= 0x240/2 && offset <= 0x28f/2)
	{
		seibu_common_video_regs_w(space,offset-0x240/2,cop_mcu_ram[offset],mem_mask);
		return;
	}

	if(offset >= 0x3c0/2 && offset <= 0x3df/2)
	{
		seibu_main_word_w(space,(offset >> 1) & 7,cop_mcu_ram[offset],0x00ff);
		return;
	}

	generic_cop_w(space, offset, data, mem_mask);

}


/**********************************************************************************************
  Seibu Cup Soccer
**********************************************************************************************/

READ16_HANDLER( cupsoc_mcu_r )
{
	if(offset >= 0x300/2 && offset <= 0x31f/2)
		return seibu_main_word_r(space,(offset >> 1) & 7,0xffff);

	if(offset >= 0x340/2 && offset <= 0x34f/2)
	{
		static const char *const portnames[] = { "DSW1", "PLAYERS12", "PLAYERS34", "SYSTEM" };

		return space->machine().root_device().ioport(portnames[(offset >> 1) & 3])->read();
	}

	if(offset == 0x35c/2)
	{
		return space->machine().root_device().ioport("DSW2")->read();
	}

	return generic_cop_r(space, offset, mem_mask);
}

WRITE16_HANDLER( cupsoc_mcu_w )
{
	COMBINE_DATA(&cop_mcu_ram[offset]);

	if(offset == 0x280/2) //irq ack / sprite buffering?
		return;

	if(offset >= 0x200/2 && offset <= 0x24f/2)
	{
		seibu_common_video_regs_w(space,offset-0x200/2,cop_mcu_ram[offset],mem_mask);
		return;
	}

	if(offset >= 0x300/2 && offset <= 0x31f/2)
	{
		seibu_main_word_w(space,(offset >> 1) & 7,cop_mcu_ram[offset],0x00ff);
		return;
	}

	generic_cop_w(space, offset, data, mem_mask);
}

READ16_HANDLER( cupsocs_mcu_r )
{
	if(offset >= 0x340/2 && offset <= 0x35f/2)
		return seibu_main_word_r(space,(offset >> 1) & 7,0xffff);

	if(offset >= 0x300/2 && offset <= 0x30f/2)
	{
		static const char *const portnames[] = { "DSW1", "PLAYERS12", "PLAYERS34", "SYSTEM" };

		return space->machine().root_device().ioport(portnames[(offset >> 1) & 3])->read();
	}

	if(offset == 0x31c/2)
	{
		return space->machine().root_device().ioport("DSW2")->read();
	}

	return generic_cop_r(space, offset, mem_mask);
}

WRITE16_HANDLER( cupsocs_mcu_w )
{
	COMBINE_DATA(&cop_mcu_ram[offset]);

	if(offset == 0x280/2) //irq ack / sprite buffering?
		return;

	if(offset >= 0x240/2 && offset <= 0x27f/2)
	{
		seibu_common_video_regs_w(space,offset-0x240/2,cop_mcu_ram[offset],mem_mask);
		return;
	}

	if(offset >= 0x200/2 && offset <= 0x20f/2)
	{
		seibu_common_video_regs_w(space,(offset-0x200/2)+(0x40/2),cop_mcu_ram[offset],mem_mask);
		return;
	}

	if(offset >= 0x340/2 && offset <= 0x35f/2)
	{
		seibu_main_word_w(space,(offset >> 1) & 7,cop_mcu_ram[offset],0x00ff);
		return;
	}

	generic_cop_w(space, offset, data, mem_mask);
}

/**********************************************************************************************
  Godzilla
**********************************************************************************************/

READ16_HANDLER( godzilla_mcu_r )
{
	if(offset >= 0x300/2 && offset <= 0x31f/2)
		return seibu_main_word_r(space,(offset >> 1) & 7,0xffff);

	if(offset >= 0x340/2 && offset <= 0x34f/2)
	{
		static const char *const portnames[] = { "DSW1", "PLAYERS12", "PLAYERS34", "SYSTEM" };

		return space->machine().root_device().ioport(portnames[(offset >> 1) & 3])->read();
	}

	return generic_cop_r(space, offset, mem_mask);
}

WRITE16_HANDLER( godzilla_mcu_w )
{
	COMBINE_DATA(&cop_mcu_ram[offset]);

	if(offset == 0x070/2)
	{
		denjinmk_setgfxbank(space->machine(), cop_mcu_ram[offset]);
		return;
	}

	if(offset == 0x280/2) //irq ack / sprite buffering?
		return;

	if(offset >= 0x200/2 && offset <= 0x24f/2)
	{
		seibu_common_video_regs_w(space,offset-0x200/2,cop_mcu_ram[offset],mem_mask);
		return;
	}

	if(offset >= 0x300/2 && offset <= 0x31f/2)
	{
		seibu_main_word_w(space,(offset >> 1) & 7,cop_mcu_ram[offset],0x00ff);
		return;
	}

	generic_cop_w(space, offset, data, mem_mask);
}

/**********************************************************************************************
  Denjin Makai
**********************************************************************************************/

READ16_HANDLER( denjinmk_mcu_r )
{
	if(offset >= 0x300/2 && offset <= 0x31f/2)
		return seibu_main_word_r(space,(offset >> 1) & 7,0xffff);

	if(offset >= 0x340/2 && offset <= 0x34f/2)
	{
		static const char *const portnames[] = { "DSW1", "PLAYERS12", "PLAYERS34", "SYSTEM" };

		return space->machine().root_device().ioport(portnames[(offset >> 1) & 3])->read();
	}

	if(offset == 0x35c/2)
	{
		return space->machine().root_device().ioport("DSW2")->read();
	}

	return generic_cop_r(space, offset, mem_mask);
}

WRITE16_HANDLER( denjinmk_mcu_w )
{
	COMBINE_DATA(&cop_mcu_ram[offset]);

	if(offset == 0x280/2) //irq ack / sprite buffering?
		return;

	if(offset == 0x070/2)
	{
		denjinmk_setgfxbank(space->machine(), cop_mcu_ram[offset]);
		return;
	}

	if(offset >= 0x200/2 && offset <= 0x24f/2)
	{
		seibu_common_video_regs_w(space,offset-0x200/2,cop_mcu_ram[offset],mem_mask);
		return;
	}

	if(offset >= 0x300/2 && offset <= 0x31f/2)
	{
		seibu_main_word_w(space,(offset >> 1) & 7,cop_mcu_ram[offset],0x00ff);
		return;
	}

	generic_cop_w(space, offset, data, mem_mask);
}

/**********************************************************************************************
  SD Gundam Sangokushi Rainbow Tairiku Senki
**********************************************************************************************/

READ16_HANDLER( grainbow_mcu_r )
{
	if(offset >= 0x300/2 && offset <= 0x31f/2)
		return seibu_main_word_r(space,(offset >> 1) & 7,0xffff);

	if(offset >= 0x340/2 && offset <= 0x34f/2)
	{
		static const char *const portnames[] = { "DSW1", "PLAYERS12", "PLAYERS34", "SYSTEM" };

		return space->machine().root_device().ioport(portnames[(offset >> 1) & 3])->read();
	}

	if(offset == 0x35c/2)
	{
		return space->machine().root_device().ioport("DSW2")->read();
	}

	return generic_cop_r(space, offset, mem_mask);
}


WRITE16_HANDLER( grainbow_mcu_w )
{
	COMBINE_DATA(&cop_mcu_ram[offset]);

	if(offset == 0x280/2) //irq ack / sprite buffering?
		return;

	if(offset >= 0x200/2 && offset <= 0x24f/2)
	{
		seibu_common_video_regs_w(space,offset-0x200/2,cop_mcu_ram[offset],mem_mask);
		return;
	}

	if(offset >= 0x300/2 && offset <= 0x31f/2)
	{
		seibu_main_word_w(space,(offset >> 1) & 7,cop_mcu_ram[offset],0x00ff);
		return;
	}

	generic_cop_w(space, offset, data, mem_mask);
}

/**********************************************************************************************
  Legionnaire
**********************************************************************************************/


READ16_HANDLER( legionna_mcu_r )
{
	if(offset >= 0x300/2 && offset <= 0x31f/2)
		return seibu_main_word_r(space,(offset >> 1) & 7,0xffff);

	if(offset >= 0x340/2 && offset <= 0x34f/2)
	{
		static const char *const portnames[] = { "DSW1", "PLAYERS12", "UNK", "SYSTEM" };

		return space->machine().root_device().ioport(portnames[(offset >> 1) & 3])->read();
	}

	return generic_cop_r(space, offset, mem_mask);
}

WRITE16_HANDLER( legionna_mcu_w )
{
	COMBINE_DATA(&cop_mcu_ram[offset]);

	if(offset == 0x070/2) //external pin: puts bit 13 high, delay, reads 0x748, writes bit 13 low
		return;

	if(offset == 0x280/2) //irq ack / sprite buffering?
		return;

	if(offset >= 0x200/2 && offset <= 0x24f/2)
	{
		seibu_common_video_regs_w(space,offset-0x200/2,cop_mcu_ram[offset],mem_mask);
		return;
	}

	if(offset >= 0x300/2 && offset <= 0x31f/2)
	{
		seibu_main_word_w(space,(offset >> 1) & 7,cop_mcu_ram[offset],0x00ff);
		return;
	}

	generic_cop_w(space, offset, data, mem_mask);
}

