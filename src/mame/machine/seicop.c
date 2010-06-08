
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

  to do:
  clean it up, consolidate code, make it work!

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
if ((cop_clearfill_lasttrigger==0x14) || (cop_clearfill_lasttrigger==0x15)) return;

----

Other Protections?

Denjin Makai seems to rely on a byteswapped mirror to write the palette.
Various other ports go through the COP area, and get mapped to inputs / sounds / video registers, this adds to
the confusion and makes it less clear what is / isn't protection related
Raiden 2 / Zero Team banking doesn't make much sense, a bank address has been found through testing on real
hardware, but the game never writes directly to it.

 */
#include "emu.h"
#include "audio/seibu.h"
#include "includes/legionna.h"
#include "includes/raiden2.h"
#include "machine/seicop.h"

#define seibu_cop_log logerror

UINT16 *cop_mcu_ram;

static UINT16 copd2_table[0x100];
static UINT16 copd2_table_2[0x100/8];
static UINT16 copd2_table_3[0x100/8];
static UINT16 copd2_table_4[0x100/8];

static UINT16 cop_438;
static UINT16 cop_43a;
static UINT16 cop_43c;

static UINT16 cop_clearfill_address[0x200];
static UINT16 cop_clearfill_length[0x200];
static UINT16 cop_clearfill_value[0x200];
static UINT16 cop_clearfill_lasttrigger = 0;


static UINT16 copd2_offs = 0;

static void copd2_set_tableoffset(running_machine *machine, UINT16 data)
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
        sprintf(filename,"copdat_%s.table2", machine->gamedrv->name);
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
        sprintf(filename,"copdat_%s.table3", machine->gamedrv->name);
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
        sprintf(filename,"copdat_%s.table4", machine->gamedrv->name);
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

static void copd2_set_tabledata(running_machine *machine, UINT16 data)
{
	copd2_table[copd2_offs] = data;
	//logerror("mcu_data %04x\n", data);
#if 0
    {
        FILE *fp;
        char filename[256];
        sprintf(filename,"copdat_%s.data", machine->gamedrv->name);
        fp=fopen(filename, "w+b");
        if (fp)
        {
            fwrite(copd2_table, 0x200, 1, fp);
            fclose(fp);
        }
    }
#endif
}


/*Movement protection*//*Legionnaire,Heated Barrel*/
static UINT32 cop_register[6];
/*Sprite DMA protection*//*SD Gundam*/
static UINT8 dma_status;
static UINT32 dma_src;
static UINT16 prot_data[2],dma_size;
/*Number protection*//*Heated Barrel,SD Gundam,Godzilla,Denjin Makai*/
//static UINT32 prot_bcd[4];
/*Hit check protection*//*Legionnaire,Heated Barrel,SD Gundam*/
static UINT8 xy_check;




#define CRT_MODE(_x_,_y_,_flip_) \
	{ \
	rectangle visarea; \
	visarea.min_x = 0; \
	visarea.max_x = _x_-1; \
	visarea.min_y = 0; \
	visarea.max_y = _y_-1; \
	space->machine->primary_screen->configure(_x_, _y_, visarea, space->machine->primary_screen->frame_period().attoseconds ); \
	flip_screen_set(space->machine, _flip_); \
	} \

/*TODO: -move x-axis limits,to calculate basing on the screen xy-axis values*/
/*      -the second value should be end of calculation (in other words,check everything between the two values) */
#define PLAYER 0
#define ENEMY 1
static void protection_move_jsr(const address_space *space,UINT32 work_ram,UINT8 k)
{
	static UINT32 move_data,x_data,y_data;
	/*Read the movement data to execute*/
	move_data = ((memory_read_word(space, work_ram+0x34)<<16) & 0xffff0000) |
	             (memory_read_word(space, work_ram+0x36) & 0xffff);

	/*Read the x/y axis of the sprite to change*/
	x_data = (memory_read_word(space, work_ram+0x8));
	y_data = (memory_read_word(space, work_ram+0x4));
	/*it's bit sensitive AFAIK*/
	/*move_data hi-word on player
      $17 = walk floor
      $1b = jump
      $30 = ?
    */
	/*Check the kind of movement that we need to execute*/
	switch(k & 1)
	{
		case PLAYER:
			switch(move_data & 0xffff)
			{
				case 0x0000: x_data++; break; //right
				case 0x0040: y_data++; break; //down
				case 0x00c0: y_data--; break; //up
				case 0x0080: x_data--; break; //left
				case 0x00a0: y_data--; x_data--; break; //up-left
				case 0x0060: y_data++; x_data--; break; //down-left
				case 0x0020: y_data++; x_data++; break; //down-right
				case 0x00e0: y_data--; x_data++; break; //up-right
			}
			break;
		/*wrong...*/
		case ENEMY:
			switch(move_data & 0xffff)
			{
				case 0x0000: x_data++; break; //right
				case 0x0040: y_data++; break; //down
				case 0x00c0: y_data--; break; //up
				case 0x0080: x_data--; break; //left
				case 0x00a0: y_data--; x_data--; break; //up-left
				case 0x0060: y_data++; x_data--; break; //down-left
				case 0x0020: y_data++; x_data++; break; //down-right
				case 0x00e0: y_data--; x_data++; break; //up-right
				default:
					popmessage("%08x",move_data);
				break;
			}
			break;
	}
	/*Write the new values to the sprite x/y data*/
	memory_write_word(space, work_ram+0x8,x_data);
	memory_write_word(space, work_ram+0x4,y_data);
}


static UINT16 hit_check;

static void protection_hit_jsr(const address_space *space,UINT32 work_ram1,UINT32 work_ram2)
{
	int x1,y1,x2,y2/*,hit1,hit2*/;
	x1 = (memory_read_word(space, work_ram1+0x8));
	y1 = (memory_read_word(space, work_ram1+0x4));
	//hit1 = (memory_read_word(space, work_ram1-));//this sprite is attacking
	x2 = (memory_read_word(space, work_ram2+0x8));
	y2 = (memory_read_word(space, work_ram2+0x4));
	//hit2 = (memory_read_word(space));

	popmessage("%08x:x=%04x y=%04x %08x:x=%04x y=%04x",work_ram1,x1,y1,work_ram2,x2,y2);

	if((x1 >= (x2-0x80)) &&
	   (x1 <= (x2+0x80)) &&
	   (y1 >= (y2-3))  &&
	   (y1 <= (y2+3)))
		hit_check = 0;
	else
		hit_check = 0xffff;
}

/*directional movement protection*/
static void moveprot_jsr(const address_space *space)
{
	static INT16 x_axis,y_axis;
	static UINT16 move_data,distance,move_type;
	move_data = memory_read_word(space, cop_register[0]+0x36);
	x_axis = memory_read_word(space, cop_register[0]+0x08);
	y_axis = memory_read_word(space, cop_register[0]+0x04);

	distance = (move_data & 0xf);
	move_type = (move_data & 0xf0)>>4;
	switch(move_type)
	{
		case 0x0f://right
			memory_write_word(space, cop_register[0]+0x08,x_axis+distance);
			//memory_write_word(space, 0x110004,);
			break;
		case 0x0b://up
			memory_write_word(space, cop_register[0]+0x04,y_axis-distance);
			break;
		case 0x07://left
			memory_write_word(space, cop_register[0]+0x08,x_axis-distance);
			break;
		case 0x03://down
			memory_write_word(space, cop_register[0]+0x04,y_axis+distance);
			break;
		case 0x0d://up-right
			memory_write_word(space, cop_register[0]+0x08,x_axis+distance);
			memory_write_word(space, cop_register[0]+0x04,y_axis-distance);
			break;
		case 0x09://up-left
			memory_write_word(space, cop_register[0]+0x04,y_axis-distance);
			memory_write_word(space, cop_register[0]+0x08,x_axis-distance);
			break;
		case 0x01://down-right
			memory_write_word(space, cop_register[0]+0x04,y_axis+distance);
			memory_write_word(space, cop_register[0]+0x08,x_axis+distance);
			break;
		case 0x05://down-left
			memory_write_word(space, cop_register[0]+0x04,y_axis+distance);
			memory_write_word(space, cop_register[0]+0x08,x_axis-distance);
			break;
		default:
			logerror("Warning: \"0x205\" command called with move_type parameter = %02x\n",move_type);

			//down-right
			//down-left
	}
	//memory_write_word(space, 0x110008,x_axis+tmp);
	//memory_write_word(space, 0x110004,y_axis+tmp);

	//memory_write_word(space, 0x110008,x_axis);
	//memory_write_word(space, 0x110004,y_axis);
}

/*
00454E: 3028 0008                move.w  ($8,A0), D0 ;player
004552: 3228 000C                move.w  ($c,A0), D1
004556: 342B 0008                move.w  ($8,A3), D2 ;enemy
00455A: 362B 000C                move.w  ($c,A3), D3
00455E: 33C0 0011 0048           move.w  D0, $110048.l ;player x
004564: 33C1 0011 0044           move.w  D1, $110044.l ;player y
00456A: 33C2 0011 0008           move.w  D2, $110008.l ;enemy x
004570: 33C3 0011 0004           move.w  D3, $110004.l ;enemy y

*/
/*sprite "look" protection*/
static void move2prot_jsr(const address_space *space)
{
	static INT16 x_pl,y_pl,x_en,y_en,res;
	x_pl = memory_read_word(space, cop_register[1]+0x8);
	y_pl = memory_read_word(space, cop_register[1]+0x4);
	x_en = memory_read_word(space, cop_register[0]+0x8);
	y_en = memory_read_word(space, cop_register[0]+0x4);

	res = 0;
	if(x_en > x_pl)
		res|=0x80;

	if((x_en > x_pl-0x20) && (x_en < x_pl+0x20))
		res|=0x40;
//...
	//if(y_en > y_pl)
	//  res|=0x40;

	memory_write_word(space, cop_register[0]+0x36,res);
}

#ifdef UNUSED_FUNCTION
/*"To point" movement protection*/
static void move3x_prot_jsr(const address_space *space)
{
	static INT16 x_pl,x_en,x_dis;
	x_pl = memory_read_word(space, cop_register[1]+0x8);
	x_en = memory_read_word(space, cop_register[0]+0x8);
	x_dis = ((memory_read_word(space, cop_register[0]+0x34) & 0xf0) >> 4);

	if(x_en > x_pl)
		x_dis^=0xffff;

	memory_write_word(space, cop_register[0]+0x36,-0x40);/*enable command*/
	memory_write_word(space, cop_register[0]+0x14,x_dis);
}

static void move3y_prot_jsr(const address_space *space)
{
	static INT16 y_pl,y_en,y_dis;
	y_pl = memory_read_word(space, cop_register[1]+0x4);
	y_en = memory_read_word(space, cop_register[0]+0x4);
	y_dis = (memory_read_word(space, cop_register[0]+0x34) & 0xf);

	if(y_en > y_pl)
		y_dis^=0xffff;

	memory_write_word(space, cop_register[0]+0x36,-0x80);/*enable command*/
	memory_write_word(space, cop_register[0]+0x10,y_dis);
}
#endif






/*
player-1 priorities list:
1086d8: show this sprite (bit 15)
1086dc: lives (BCD,bits 3,2,1,0)
1086de: energy bar (upper byte)
1086e0: walk animation (lower byte)
1086ec: "death" status (bit 15)
1086f4: sprite y axis
1086f0: sprite x axis

Sprite DMA TODO:
-sprite priorities,likely to be a protection issue because in-game sprites MUST
 be behind the foreground layer but the attract mode logos (the ones used when
 the story is explained) should be above it.They both use 0 as priority number.
-sprites at the very left border disappears.
-some bullets STILL remains on screen?
-3rd mid-boss disappears (0x1f0),2nd boss lasers wrong positioning

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
001E2E: 317C A180 0100           move.w  #$a180, ($100,A0) ;<-DMA parameters!?
001E34: 317C 6980 0102           move.w  #$6980, ($102,A0) ;<-""
001E3A: 317C C480 0102           move.w  #$c480, ($102,A0) ;<-""
001E40: 317C 0000 0010           move.w  #$0, ($10,A0)     ;<-""
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
//Note: I believe the above program is there just for protection copy
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

*/

static UINT16 s_i;

static void dma_transfer(const address_space *space)
{
	static UINT16 rel_xy;
	static UINT16 abs_x,abs_y;
	static UINT16 param;

	//for(s_i = dma_size;s_i > 0;s_i--)
	{
		/*Sprite Color*/
		param = memory_read_word(space, 0x100400) & 0x3f;
		/*Write the entire parameters [offs+0]*/
		memory_write_word(space, cop_register[5]+4,memory_read_word(space, dma_src) + param);
		/*Sprite Priority (guess)*/
		//param = ((memory_read_word(space, 0x100400) & 0x40) ? 0x4000 : 0);
		/*Write the sprite number [offs+1]*/
		memory_write_word(space, cop_register[5]+6,memory_read_word(space, dma_src+2));
		/*Sprite Relative x/y coords*/
		rel_xy = memory_read_word(space, dma_src+4); /*???*/
		/*temporary hardwired,it should point to 0x4c0/0x4a0*/
		abs_x = (memory_read_word(space, 0x110008) - memory_read_word(space, 0x10048e));
		abs_y = (memory_read_word(space, 0x110004) - memory_read_word(space, 0x10048c));
		memory_write_word(space, cop_register[5]+8,((rel_xy & 0x7f) + (abs_x) - ((rel_xy & 0x80) ? 0x80 : 0)) & 0x1ff);
		memory_write_word(space, cop_register[5]+10,(((rel_xy & 0x7f00) >> 8) + (abs_y) + (0x10) - ((rel_xy & 0x8000) ? 0x80 : 0)) & 0x1ff);
		cop_register[5]+=8;
		dma_src+=6;
	}
}


/*
    switch(memory_read_word(space, cop_register[2]))
    {
        case 0xb4: xparam = 0x0c/2; break;
        case 0xb8: xparam = 0x10/2; break;
        case 0xbc: xparam = 0x14/2; break;
        case 0xc0: xparam = 0x18/2; break;
        case 0xc4: xparam = 0x1c/2; break;
        case 0xd4: xparam = 0x20/2; break;
        ...
        case 0xb0: xparam = 0x08/2;
        case 0xac: xparam = 0x04/2;
    }
*/
static UINT16 check_calc(UINT16 param)
{
	UINT16 num,i;

	i = param;
	i-=0xac;
	i/=4;
	num = (0x4/2);
	for(;i>0;i--)
		num+=(0x4/2);

	return num;
}

static UINT16 hit_check_jsr(const address_space *space)
{
	static INT16 xsrc,xdst,ysrc,ydst,xparam,yparam;
	xsrc = (memory_read_word(space, 0x110008));
	ysrc = (memory_read_word(space, 0x110004));
	xdst = (memory_read_word(space, 0x110048));
	ydst = (memory_read_word(space, 0x110044));

	/*Here we check the destination sprite width*/
	/*0x4a4/0x4c4*/
	xparam = check_calc(memory_read_word(space, cop_register[2]));
	/*Here we check the destination sprite height*/
	/*0x4a6/0x4c6*/
	yparam = check_calc(memory_read_word(space, cop_register[3]));

	if(!xparam || !yparam)
		popmessage("SRC:%04x %04x DST:%04x %04x V:%08x %08x",xsrc,ysrc,xdst,ydst,memory_read_word(space, cop_register[2]),memory_read_word(space, cop_register[3]));
	if(xdst >= (xsrc-xparam) && ydst >= (ysrc-yparam) &&
	   xdst <= (xsrc+xparam) && ydst <= (ysrc+yparam))
		return 0;//sprites collide
	else
		return 3;//sprites do not collide
}

#define UP			0xc0
#define UP_RIGHT    0xe0
#define RIGHT       0x00
#define DOWN_RIGHT	0x20
#define DOWN		0x40
#define DOWN_LEFT	0x60
#define LEFT		0x80
#define UP_LEFT		0xa0

/*Heated Barrel*/
/*command 0x8100 will check for the direction of the sprite*/
/*command 0x8900 will check the "point" movement*/
static void cop2_move3_prot(const address_space *space)
{
	static INT16 x_pl,x_en;
	static INT16 y_pl,y_en;
	static INT16 x_dis,y_dis;
	static INT16 dir,dis;
	x_pl = memory_read_word(space, cop_register[1]+0x8);
	x_en = memory_read_word(space, cop_register[0]+0x8);
	dis = ((memory_read_word(space, cop_register[0]+0x34) & 0xf0) >> 4);
	y_pl = memory_read_word(space, cop_register[1]+0x4);
	y_en = memory_read_word(space, cop_register[0]+0x4);

	/*
    xxxx ---- select the direction of the enemy sprite

                        0xc0 up
          up-left   0xa0  |  0xe0 up-right
         left    0x80   <-o->  0x00 right
        down-left   0x60  |  0x20 down-right
                        0x40 down
    */

	if(x_en >= x_pl)
	{
		if((y_en >= (y_pl-0x10)) && (y_en <= (y_pl+0x10)))
			dir = LEFT;
		else if(y_en < (y_pl-0x10))
			dir = DOWN_LEFT;
		else if(y_en > (y_pl+0x10))
			dir = UP_LEFT;
	}
	else if(x_en < x_pl)
	{
		if((y_en >= (y_pl-0x10)) && (y_en <= (y_pl+0x10)))
			dir = RIGHT;
		else if(y_en < (y_pl-0x10))
			dir = DOWN_RIGHT;
		else if(y_en > (y_pl+0x10))
			dir = UP_RIGHT;
	}
	/*UP DOWN cases*/
	if((x_en >= (x_pl-0x10)) && (x_en <= (x_pl+0x10)))
	{
		if(y_en >= y_pl)
			dir = UP;
		else if(y_en < y_pl)
			dir = DOWN;
	}

	memory_write_word(space, cop_register[0]+0x36,dir);

	/*TODO*/
	x_dis = (x_pl-x_en);
	y_dis = (y_pl-y_en);

	if(x_dis > 4)
		x_dis = 4;

	if(x_dis < -4)
		x_dis = -4;

	if(y_dis > 4)
		y_dis = 4;

	if(y_dis < -4)
		y_dis = -4;

	//if(y_en > y_pl)
	//  y_dis^=0xffff;

	//if(x_en > x_pl)
	//  x_dis^=0xffff;

	memory_write_word(space, cop_register[0]+0x10,y_dis);
	memory_write_word(space, cop_register[0]+0x14,x_dis);
}

/**/
static UINT16 cop2_hit_prot(const address_space *space)
{
	static INT16 xsrc,xdst;
	static INT16 ysrc,ydst;
	static INT16 xp,yp;
	static INT16 param1,param2;
	static INT16 val;

	param1 = memory_read_word(space, cop_register[2]);
	param2 = memory_read_word(space, cop_register[3]);

	xsrc = memory_read_word(space, cop_register[0]+0x8) + memory_read_word(space, cop_register[0]+0x14);
	ysrc = memory_read_word(space, cop_register[0]+0x4) + memory_read_word(space, cop_register[0]+0x10);
	xdst = memory_read_word(space, cop_register[1]+0x8) + memory_read_word(space, cop_register[1]+0x14);
	ydst = memory_read_word(space, cop_register[1]+0x4) + memory_read_word(space, cop_register[1]+0x10);

//  xp = (param1 & 0x00f0) >> 4;
//  yp = (param1 & 0x0f00) >> 8;

//  popmessage("%04x %04x",param1,param2);

	xp = 0;
	yp = 0;
	for(val = ((param1 & 0xff0) >> 4); val > 0; val-=5)
		xp++;

	for(val = ((param1 & 0xff0) >> 4); val > 0; val-=3)
		yp++;

	/*TODO*/
//  xp+=4;
//  yp+=4;

	if(xsrc >= xdst && xsrc <= xdst+xp && ysrc >= xdst && ysrc <= ydst+yp)
		return 0;
	else
		return 3;
}

static void cop2_move2_prot(const address_space *space)
{
	static INT16 xsrc,ysrc;
	static INT16 param2;

	xsrc = memory_read_word(space, cop_register[0]+0x14);
	ysrc = memory_read_word(space, cop_register[0]+0x10);
	param2 = memory_read_word(space, cop_register[3]);

	switch(param2)
	{
		case 0x10:	xsrc++; break; //right
		case 0x30:	xsrc--; break; //left
		case 0x40:  ysrc--; break; //up
		case 0x60:	ysrc++; break; //down
		case 0x08:  ysrc--; xsrc++; break; //up-right
		case 0x38:	ysrc--; xsrc--; break; //up-left
		case 0x28:	ysrc++; xsrc--; break; //down-left
		case 0x18:  ysrc++; xsrc++; break; //down-right
	}

	memory_write_word(space, cop_register[0]+0x14,xsrc);
	memory_write_word(space, cop_register[0]+0x10,ysrc);
}




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
			logerror("%06x: COPX unhandled read returning %04x from offset %04x\n", cpu_get_pc(space->cpu), retvalue, offset*2);
			return retvalue;
		}

		//case (0x47e/2):
		//case (0x5b0/2):
		//case (0x5b4/2):
		//  return cop_mcu_ram[offset];

		case (0x700/2): return input_port_read(space->machine, "DSW1");
		case (0x704/2):	return input_port_read(space->machine, "PLAYERS12");
		case (0x708/2):	return input_port_read(space->machine, "PLAYERS34");
		case (0x70c/2):	return input_port_read(space->machine, "SYSTEM");
		case (0x71c/2): return input_port_read(space->machine, "DSW2");
	}
}

WRITE16_HANDLER( copdxbl_0_w )
{
	COMBINE_DATA(&cop_mcu_ram[offset]);

	switch(offset)
	{

		default:
		{
			logerror("%06x: COPX unhandled write data %04x at offset %04x\n", cpu_get_pc(space->cpu), data, offset*2);
			break;
		}

		/*********************************************************************
        400-5ff -  Protection writes
        *********************************************************************/


		case (0x4a0/2):
		case (0x4a2/2):
		case (0x4a4/2):
		case (0x4a6/2):
		case (0x4a8/2):
		case (0x4aa/2):
		case (0x4ac/2):
		case (0x4ae/2):
		case (0x4c0/2):
		case (0x4c2/2):
		case (0x4c4/2):
		case (0x4c6/2):
		case (0x4c8/2):
		case (0x4ca/2):
		case (0x4cc/2):
		case (0x4ce/2):
			//cop_reg_w(cop_mcu_ram[offset],offset & 0x000f, (offset < (0x4b0/2)) ? 1 : 0);
			break;
		/*layer clearance,but the bootleg doesn't send values,so this function
          is an original left-over.*/
		case (0x478/2):
		{
			/*
    AM_RANGE(0x100800, 0x100fff) AM_RAM_WRITE(legionna_background_w) AM_BASE(&legionna_back_data)
    AM_RANGE(0x101000, 0x1017ff) AM_RAM_WRITE(legionna_foreground_w) AM_BASE(&legionna_fore_data)
    AM_RANGE(0x101800, 0x101fff) AM_RAM_WRITE(legionna_midground_w) AM_BASE(&legionna_mid_data)
    AM_RANGE(0x102000, 0x102fff) AM_RAM_WRITE(legionna_text_w) AM_BASE(&legionna_textram)
            */
			break;
		}
		case (0x500/2):
		{
			//cop_fct = cop_mcu_ram[offset];
			//cop_run();
			break;
		}
		case (0x604/2):
		{
			//C.R.T. Controller
			/*
            data = setting
            0x01e = 320x256         ---- ---x xxx-
            0x0e1 = 320x256 REVERSE ---- xxx- ---x
            0x016 = 320x240         ---- ---x -xx-
            0x0e9 = 320x240 REVERSE ---- xxx- x--x
            0x004 = 320x224         ---- ---- -x--
            0x10b = 320x224 REVERSE ---x ---- x-xx
            For now we use this by cases and not per bits.
            */

			switch(data)
			{
				case 0x0000:
				case 0x001e: CRT_MODE(320,256,0); break;
				case 0x00e1: CRT_MODE(320,256,1); break;
				case 0x0016: CRT_MODE(320,240,0); break;
				case 0x00e9: CRT_MODE(320,240,1); break;
				case 0x0004: CRT_MODE(320,224,0); break;
				case 0x010b: CRT_MODE(320,224,1); break;
				default:
				#ifdef MAME_DEBUG
				popmessage("Warning: Undefined CRT Mode %04x",data);
				#endif
				CRT_MODE(320,256,0);
			}
			break;
		}
		/*TODO: kludge on x-axis.*/
		case (0x660/2): { legionna_scrollram16[0] = cop_mcu_ram[offset] - 0x1f0; break; }
		case (0x662/2): { legionna_scrollram16[1] = cop_mcu_ram[offset]; break; }
		case (0x664/2): { legionna_scrollram16[2] = cop_mcu_ram[offset] - 0x1f0; break; }
		case (0x666/2): { legionna_scrollram16[3] = cop_mcu_ram[offset]; break; }
		case (0x668/2): { legionna_scrollram16[4] = cop_mcu_ram[offset] - 0x1f0; break; }
		case (0x66a/2): { legionna_scrollram16[5] = cop_mcu_ram[offset]; break; }
		case (0x66c/2): { legionna_scrollram16[6] = cop_mcu_ram[offset] - 0x1f0; break; }
		case (0x66e/2): { legionna_scrollram16[7] = cop_mcu_ram[offset]; break; }

		case (0x740/2):
		{
			soundlatch_w(space, 0, data & 0xff);
			cputag_set_input_line(space->machine, "audiocpu", INPUT_LINE_NMI, PULSE_LINE );
			break;
		}
		/*video regs (not scrollram,something else)*/
		//case (0x660/2):
		//case (0x662/2):
		//case (0x664/2):
		//case (0x666/2):
		//case (0x668/2):
		//case (0x66a/2):
		//case (0x66c/2):
		//case (0x66e/2):
		//  break;
		/*bootleg sound HW*/
		/*case (0x740/2):
        {
            soundlatch_w(space, 1, data & 0x00ff);
            cputag_set_input_line(space->machine, "audiocpu", INPUT_LINE_NMI, PULSE_LINE );
            break;
        }*/
	}
}

// this still probably contains some useful information, but we should handle
// things as generically as possible
#ifdef UNUSED_FUNCTION
/********************************************************************************************

  COPX-D2 simulation
    - Raiden 2
    - Zero Team

 *******************************************************************************************/


/* Raiden 2 COP2 handling.  Note, some important details about table upload in here that the
   other simulations are missing */

//  COPX functions, terribly incomplete

typedef struct _cop_state cop_state;
struct _cop_state
{
	UINT16		offset;						/* last write offset */
	UINT16		ram[0x200/2];				/* RAM from 0x400-0x5ff */

	UINT32		reg[4];						/* registers */

	UINT16		func_trigger[0x100/8];		/* function trigger */
	UINT16		func_value[0x100/8];		/* function value (?) */
	UINT16		func_mask[0x100/8];			/* function mask (?) */
	UINT16		program[0x100];				/* program "code" */
};

static cop_state cop_data;


#define VERBOSE 1
#define COP_LOG(x)	do { if (VERBOSE) logerror x; } while (0)



INLINE UINT16 cop_ram_r(cop_state *cop, UINT16 offset)
{
	return cop->ram[(offset - 0x400) / 2];
}

INLINE void cop_ram_w(cop_state *cop, UINT16 offset, UINT16 data)
{
	cop->ram[(offset - 0x400) / 2] = data;
}

INLINE UINT32 r32(offs_t address)
{
	return	(memory_read_word(space, address + 0) << 0) |
			(memory_read_word(space, address + 2) << 16);
}

INLINE void w32(offs_t address, UINT32 data)
{
	memory_write_word(space, address + 0, data >> 0);
	memory_write_word(space, address + 2, data >> 16);
}


void cop_init(void)
{
	memset(&cop_data, 0, sizeof(cop_data));
}

WRITE16_HANDLER( raiden2_cop2_w )
{
	cop_state *cop = &cop_data;
	UINT32 temp32;
	UINT8 regnum;
	int func;

	/* all COP data writes are word-length (?) */
	data = COMBINE_DATA(&cop->ram[offset]);

	/* handle writes */
	switch (offset + (0x400/2))
	{
		/* ----- BCD conversion ----- */

		case 0x420/2:		/* LSW of number */
		case 0x422/2:		/* MSW of number */
			temp32 = cop_ram_r(cop, 0x420) | (cop_ram_r(cop, 0x422) << 16);
			cop_ram_w(cop, 0x590, ((temp32 / 1) % 10) + (((temp32 / 10) % 10) << 8) + 0x3030);
			cop_ram_w(cop, 0x592, ((temp32 / 100) % 10) + (((temp32 / 1000) % 10) << 8) + 0x3030);
			cop_ram_w(cop, 0x594, ((temp32 / 10000) % 10) + (((temp32 / 100000) % 10) << 8) + 0x3030);
			cop_ram_w(cop, 0x596, ((temp32 / 1000000) % 10) + (((temp32 / 10000000) % 10) << 8) + 0x3030);
			cop_ram_w(cop, 0x598, ((temp32 / 100000000) % 10) + (((temp32 / 1000000000) % 10) << 8) + 0x3030);
			break;

		/* ----- program upload registers ----- */

		case 0x432/2:		/* COP program data */
			COP_LOG(("%05X:COP Prog Data = %04X\n", cpu_get_pc(space->cpu), data));
			cop->program[cop_ram_r(cop, 0x434)] = data;
			break;

		case 0x434/2:		/* COP program address */
			COP_LOG(("%05X:COP Prog Addr = %04X\n", cpu_get_pc(space->cpu), data));
			assert((data & ~0xff) == 0);
			temp32 = (data & 0xff) / 8;
			cop->func_value[temp32] = cop_ram_r(cop, 0x438);
			cop->func_mask[temp32] = cop_ram_r(cop, 0x43a);
			cop->func_trigger[temp32] = cop_ram_r(cop, 0x43c);

			break;

		case 0x438/2:		/* COP program entry value (0,4,5,6,7,8,9,F) */
			COP_LOG(("%05X:COP Prog Val  = %04X\n", cpu_get_pc(space->cpu), data));
			break;

		case 0x43a/2:		/* COP program entry mask */
			COP_LOG(("%05X:COP Prog Mask = %04X\n", cpu_get_pc(space->cpu), data));
			break;

		case 0x43c/2:		/* COP program trigger value */
			COP_LOG(("%05X:COP Prog Trig = %04X\n", cpu_get_pc(space->cpu), data));
			break;

		/* ----- ???? ----- */

		case 0x47a/2:		/* clear RAM */
			if (cop_ram_r(cop, 0x47e) == 0x118)
			{
				UINT32 addr = cop_ram_r(cop, 0x478) << 6;
				int count = (cop_ram_r(cop, 0x47a) + 1) << 5;
				COP_LOG(("%05X:COP RAM clear from %05X to %05X\n", cpu_get_pc(space->cpu), addr, addr + count));
				while (count--)
					memory_write_byte(space, addr++, 0);
			}
			else
			{
				COP_LOG(("%05X:COP Unknown RAM clear(%04X) = %04X\n", cpu_get_pc(space->cpu), cop_ram_r(cop, 0x47e), data));
			}
			break;

		/* ----- program data registers ----- */

		case 0x4a0/2:		/* COP register high word */
		case 0x4a2/2:		/* COP register high word */
		case 0x4a4/2:		/* COP register high word */
		case 0x4a6/2:		/* COP register high word */
			regnum = (offset) % 4;
			COP_LOG(("%05X:COP RegHi(%d) = %04X\n", cpu_get_pc(space->cpu), regnum, data));
			cop->reg[regnum] = (cop->reg[regnum] & 0x0000ffff) | (data << 16);
			break;

		case 0x4c0/2:		/* COP register low word */
		case 0x4c2/2:		/* COP register low word */
		case 0x4c4/2:		/* COP register low word */
		case 0x4c6/2:		/* COP register low word */
			regnum = (offset) % 4;
			COP_LOG(("%05X:COP RegLo(%d) = %04X\n", cpu_get_pc(space->cpu), regnum, data));
			cop->reg[regnum] = (cop->reg[regnum] & 0xffff0000) | data;
			break;

		/* ----- program trigger register ----- */

		case 0x500/2:		/* COP trigger */
			COP_LOG(("%05X:COP Trigger = %04X\n", cpu_get_pc(space->cpu), data));
			for (func = 0; func < ARRAY_LENGTH(cop->func_trigger); func++)
				if (cop->func_trigger[func] == data)
				{
					int offs;

					COP_LOG(("  Execute:"));
					for (offs = 0; offs < 8; offs++)
					{
						if (cop->program[func * 8 + offs] == 0)
							break;
						COP_LOG((" %04X", cop->program[func * 8 + offs]));
					}
					COP_LOG(("\n"));

					/* special cases for now */
					if (data == 0x5205 || data == 0x5a05)
					{
						COP_LOG(("  Copy 32 bits from %05X to %05X\n", cop->reg[0], cop->reg[1]));
						w32(cop->reg[1], r32(cop->reg[0]));
					}
					else if (data == 0xf205)
					{
						COP_LOG(("  Copy 32 bits from %05X to %05X\n", cop->reg[0] + 4, cop->reg[1]));
						w32(cop->reg[2], r32(cop->reg[0] + 4));
					}
					break;
				}
			logerror("%05X:COP Warning - can't find command - func != ARRAY_LENGTH(cop->func_trigger)\n",  cpu_get_pc(space->cpu));
			break;

		/* ----- other stuff ----- */

		default:		/* unknown */
			COP_LOG(("%05X:COP Unknown(%04X) = %04X\n", cpu_get_pc(space->cpu), offset*2 + 0x400, data));
			break;
	}
}


READ16_HANDLER( raiden2_cop2_r )
{
	cop_state *cop = &cop_data;
	COP_LOG(("%05X:COP Read(%04X) = %04X\n", cpu_get_pc(space->cpu), offset*2 + 0x400, cop->ram[offset]));
	return cop->ram[offset];
}
#endif





/* Generic COP functions
  -- the game specific handlers fall through to these if there
     isn't a specific case for them.  these implement behavior
     which seems common to all the agmes
*/

static READ16_HANDLER( generic_cop_r )
{
	UINT16 retvalue;
	retvalue = cop_mcu_ram[offset];


	switch (offset)
	{
		default:
			seibu_cop_log("%06x: COPX unhandled read returning %04x from offset %04x\n", cpu_get_pc(space->cpu), retvalue, offset*2);
			return retvalue;
	}
}

static WRITE16_HANDLER( generic_cop_w )
{
	static UINT32 temp32;

	switch (offset)
	{
		default:
			seibu_cop_log("%06x: COPX unhandled write data %04x at offset %04x\n", cpu_get_pc(space->cpu), data, offset*2);
			break;

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
            SD Gundam sets 0 (maybe there's a mirror somewhere else? 0x01e for example is setted with an 8)
            */
			break;

		/* Command tables for 0x500 / 0x502 commands */
		case (0x032/2): { copd2_set_tabledata(space->machine, data); break; }
		case (0x034/2): { copd2_set_tableoffset(space->machine, data); break; }
		case (0x038/2):	{ cop_438 = data; break; }
		case (0x03a/2):	{ cop_43a = data; break; }
		case (0x03c/2): { cop_43c = data; break; }

		/* Layer Clearing */
		case (0x078/2): /* clear address */
		{
			cop_clearfill_address[cop_clearfill_lasttrigger] = data; // << 6 to get actual address
			seibu_cop_log("%06x: COPX set layer clear address to %04x (actual %08x)\n", cpu_get_pc(space->cpu), data, data<<6);
			break;
		}

		case (0x07a/2): /* clear length */
		{
			cop_clearfill_length[cop_clearfill_lasttrigger] = data;
			seibu_cop_log("%06x: COPX set layer clear length to %04x (actual %08x)\n", cpu_get_pc(space->cpu), data, data<<5);

			break;
		}

		case (0x07c/2): /* clear value? */
		{
			cop_clearfill_value[cop_clearfill_lasttrigger] = data;
			seibu_cop_log("%06x: COPX set layer clear value to %04x (actual %08x)\n", cpu_get_pc(space->cpu), data, data<<6);
			break;
		}

		/* unknown, related to clears? / DMA? */
		case (0x07e/2):
		{
			cop_clearfill_lasttrigger = data;
			seibu_cop_log("%06x: COPX set layer clear trigger? to %04x\n", cpu_get_pc(space->cpu), data);
			if (data>=0x1ff)
			{
				seibu_cop_log("invalid!, >0x1ff\n");
				cop_clearfill_lasttrigger = 0;
			}

			break;
		}

		/* Registers */
		case (0x0a0/2): { cop_register[0] = (cop_register[0]&0x0000ffff)|(cop_mcu_ram[offset]<<16); break; }
		case (0x0c0/2): { cop_register[0] = (cop_register[0]&0xffff0000)|(cop_mcu_ram[offset]<<0);  break; }

		case (0x0a2/2): { cop_register[1] = (cop_register[1]&0x0000ffff)|(cop_mcu_ram[offset]<<16); break; }
		case (0x0c2/2): { cop_register[1] = (cop_register[1]&0xffff0000)|(cop_mcu_ram[offset]<<0);   break; }

		case (0x0a4/2): { cop_register[2] = (cop_register[2]&0x0000ffff)|(cop_mcu_ram[offset]<<16); break; }
		case (0x0c4/2): { cop_register[2] = (cop_register[2]&0xffff0000)|(cop_mcu_ram[offset]<<0);  break; }

		case (0x0a6/2): { cop_register[3] = (cop_register[3]&0x0000ffff)|(cop_mcu_ram[offset]<<16); break; }
		case (0x0c6/2): { cop_register[3] = (cop_register[3]&0xffff0000)|(cop_mcu_ram[offset]<<0);   break; }

		/* was dma_dst */
		case (0x0a8/2): { cop_register[4] = (cop_register[4]&0x0000ffff)|(cop_mcu_ram[offset]<<16); break; }
		case (0x0c8/2): { cop_register[4] = (cop_register[4]&0xffff0000)|(cop_mcu_ram[offset]<<0);   break; }

		case (0x100/2):
		{
			int i;
			int command;

			seibu_cop_log("%06x: COPX execute table macro command %04x %04x | regs %08x %08x %08x %08x %08x\n", cpu_get_pc(space->cpu), data, cop_mcu_ram[offset], cop_register[0], cop_register[1], cop_register[2], cop_register[3], cop_register[4]);

			command = -1;
			/* search the uploaded 'trigger' table for a matching trigger*/
			/* note, I don't know what the 'mask' or 'value' tables are... probably important, might determine what actually gets executed! */
			for (i=0;i<32;i++)
			{
				if (cop_mcu_ram[offset]==copd2_table_4[i])
				{
					seibu_cop_log("    Cop Command %04x found in slot %02x with other params %04x %04x\n", cop_mcu_ram[offset], i, copd2_table_2[i], copd2_table_3[i]);
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
				int j;
				command*=0x8;
				seibu_cop_log("     Sequence: ");
				for (j=0;j<0x8;j++)
				{
					seibu_cop_log("%04x ", copd2_table[command+j]);
				}
				seibu_cop_log("\n");
			}


			break;
		}


		/* hmm, this would be strange the 6xx range should be video regs?? */
		case (0x2fc/2):
		{
			seibu_cop_log("%06x: COPX execute current layer clear??? %04x\n", cpu_get_pc(space->cpu), data);

			// I think the value it writes here must match the other value for anything to happen.. maybe */
			//if (data!=cop_clearfill_value[cop_clearfill_lasttrigger]) break;
			if ((cop_clearfill_lasttrigger==0x14) || (cop_clearfill_lasttrigger==0x15)) return;

			/* do the fill  */
			if (cop_clearfill_value[cop_clearfill_lasttrigger]==0x0000)
			{
				UINT32 length, address;
				int i;
				address = cop_clearfill_address[cop_clearfill_lasttrigger] << 6;
				length = (cop_clearfill_length[cop_clearfill_lasttrigger]+1) << 5;

				for (i=address;i<address+length;i+=2)
				{
					memory_write_word(space, i, 0x0000);
				}
			}
			break;
		}
	}
}

/**********************************************************************************************
  Heated Barrel
**********************************************************************************************/

READ16_HANDLER( heatbrl_mcu_r )
{
	switch (offset)
	{
		default:
			return generic_cop_r(space, offset, mem_mask);

	    /*********************************************************************
        400-5ff -  Protection reads
        *********************************************************************/

		case (0x180/2):	{ return xy_check; } /*hit protection*/
		case (0x182/2):	{ if(input_code_pressed(space->machine, KEYCODE_X)) { return 0; } else { return 3; } } /*---- ---- ---- --xx used bits*/
		case (0x184/2):	{ if(input_code_pressed(space->machine, KEYCODE_C)) { return 0; } else { return 3; } } /*---- ---- ---- --xx used bits*/

	    case (0x1b0/2): return (0xffff); /* bit 15 is branched on a few times in the $1938 area */
		case (0x1b4/2):	return (0xffff); /* read at $1932 and stored in ram before +0x5b0 bit 15 tested */

		/*********************************************************************
        700-7ff - Non-protection reads
        *********************************************************************/

		/* Seibu Sound System */
		case (0x3c8/2):	return seibu_main_word_r(space,2,0xffff);
		case (0x3cc/2):	return seibu_main_word_r(space,3,0xffff);
		case (0x3d4/2): return seibu_main_word_r(space,5,0xffff);

		/* Inputs */
		case (0x340/2): return input_port_read(space->machine, "DSW1");
		case (0x344/2):	return input_port_read(space->machine, "PLAYERS12");
		case (0x348/2): return input_port_read(space->machine, "PLAYERS34");
		case (0x34c/2): return input_port_read(space->machine, "SYSTEM");
	}
}

WRITE16_HANDLER( heatbrl_mcu_w )
{
	COMBINE_DATA(&cop_mcu_ram[offset]);

	switch (offset)
	{
		default:
			generic_cop_w(space, offset, data, mem_mask);
			break;

		/*********************************************************************
        400-5ff -  Protection writes
        *********************************************************************/

		/* Odd, this is a video register */
		case (0x070/2): { heatbrl_setgfxbank( cop_mcu_ram[offset] ); break; }

#if 1 // turn off to get the generic sequence logging
		/* Macros Command Trigger */
		case (0x100/2):
		{
			switch(cop_mcu_ram[offset])
			{
				case 0x8100:
					break;
				case 0x8900:
				{
					cop2_move3_prot(space);
					break;
				}
				case 0x205:
				{
					cop2_move2_prot(space);
					break;
				}
				case 0xa100:
					break;
				case 0xb080:
					break;
				case 0xa900:
					break;
				case 0xb880:
				{
					xy_check = cop2_hit_prot(space);
					break;
				}
				default:
					seibu_cop_log("DMA CMD 0x500 with parameter = %04x PC = %08x\n",cop_mcu_ram[offset],cpu_get_previouspc(space->cpu));
			}
			break;
		}
#endif

		/*********************************************************************
        600-6ff - Video Registers
        *********************************************************************/

		// 65a bit 0 is flipscreen
		case (0x25c/2): { legionna_layer_disable = cop_mcu_ram[offset]; break; } // 65c probably layer disables, like Dcon? Used on screen when you press P1-4 start (values 13, 11, 0 seen)
		// 660 - 66a scroll control;  is there a layer priority switch...?
		case (0x260/2): { legionna_scrollram16[0] = cop_mcu_ram[offset]; break; }
		case (0x262/2): { legionna_scrollram16[1] = cop_mcu_ram[offset]; break; }
		case (0x264/2): { legionna_scrollram16[2] = cop_mcu_ram[offset]; break; }
		case (0x266/2): { legionna_scrollram16[3] = cop_mcu_ram[offset]; break; }
		case (0x268/2): { legionna_scrollram16[4] = cop_mcu_ram[offset]; break; }
		case (0x26a/2): { legionna_scrollram16[5] = cop_mcu_ram[offset]; break; }

		/*********************************************************************
        700-7ff - Output (Seibu Sound System)
        *********************************************************************/

		case (0x3c0/2):	{ seibu_main_word_w(space,0,cop_mcu_ram[offset],0x00ff); break; }
		case (0x3c4/2):	{ seibu_main_word_w(space,1,cop_mcu_ram[offset],0x00ff); break; }
		case (0x3d0/2):	{ seibu_main_word_w(space,4,cop_mcu_ram[offset],0x00ff); break; }
		case (0x3d8/2):	{ seibu_main_word_w(space,6,cop_mcu_ram[offset],0x00ff); break; }
	}
}


/**********************************************************************************************
  Seibu Cup Soccer
**********************************************************************************************/

READ16_HANDLER( cupsoc_mcu_r )
{
	switch (offset)
	{
		default:
			return generic_cop_r(space, offset, mem_mask);

		//case (0x07e/2):
		//case (0x1b0/2):
		//case (0x1b4/2):
		//  return cop_mcu_ram[offset];

		/* returning 0xffff for some inputs for now, breaks coinage but
           allows cupsoc to boot */
		case (0x300/2): return input_port_read(space->machine, "DSW1");
		case (0x304/2): return input_port_read(space->machine, "PLAYERS12");
		case (0x308/2): return input_port_read(space->machine, "PLAYERS34");
		case (0x30c/2): return input_port_read(space->machine, "SYSTEM");
		case (0x314/2): return 0xffff;
		case (0x31c/2): return input_port_read(space->machine, "DSW2");

		case (0x340/2): return 0xffff;
		case (0x344/2): return 0xffff;
		case (0x348/2):	return 0xffff;//seibu_main_word_r(space,2,0xffff);
		case (0x34c/2): return 0xffff;//seibu_main_word_r(space,3,0xffff);
		case (0x354/2): return 0xffff;//seibu_main_word_r(space,5,0xffff);
		case (0x35c/2): return 0xffff;
	}
}

WRITE16_HANDLER( cupsoc_mcu_w )
{
	COMBINE_DATA(&cop_mcu_ram[offset]);

	seibu_cop_log("%06x: Legionna write data %04x at offset %04x\n", cpu_get_pc(space->cpu), data, offset*2);

	switch (offset)
	{
		default:
			generic_cop_w(space, offset, data, mem_mask);
			break;

		/*********************************************************************
        400-5ff -  Protection writes
        *********************************************************************/

		/* Trigger Macro Command */
		case (0x100/2):
		{
			switch(cop_mcu_ram[offset])
			{
				/*???*/
				case 0x8100:
				{
					UINT32 src = cop_register[0];
					memory_write_word(space, src+0x36,0xffc0);
					break;
				}
				case 0x8900:
				{
					UINT32 src = cop_register[0];
					memory_write_word(space, src+0x36,0xff80);
					break;
				}
				/*Right*/
				case 0x0205:
				{
					UINT32 src = cop_register[0];
					INT16 y = memory_read_word(space, src+0x4);
					INT16 x = memory_read_word(space, src+0x8);
					INT16 y_rel = memory_read_word(space, src+0x10);
					INT16 x_rel = memory_read_word(space, src+0x14);
					memory_write_word(space, src+0x4,(y+y_rel));
					memory_write_word(space, src+0x8,(x+x_rel));
					/*logerror("%08x %08x %08x %08x %08x\n",cop_register[0],
                                                   memory_read_word(space, cop_reg[0]+0x4),
                                                   memory_read_word(space, cop_reg[0]+0x8),
                                                   memory_read_word(space, cop_reg[0]+0x10),
                                                   memory_read_word(space, cop_reg[0]+0x14));*/
					break;
				}
				/*???*/
				case 0x3bb0:
				{
					//UINT32 dst = cop_register[0];
					//UINT32 dst = cop_register[1];
					//memory_write_word(space, dst,  mame_rand(space->machine)/*memory_read_word(space, src)*/);
					//memory_write_word(space, dst+2,mame_rand(space->machine)/*memory_read_word(space, src+2)*/);
					//memory_write_word(space, dst+4,mame_rand(space->machine)/*memory_read_word(space, src+4)*/);
					//memory_write_word(space, dst+6,mame_rand(space->machine)/*memory_read_word(space, src+6)*/);
					//logerror("%04x\n",cop_register[0]);
					break;
				}
				default:
					//logerror("%04x\n",data);
					break;
			}
			break;
		}

		/* Video Regs */
		case (0x204/2):
		{
			//C.R.T. Controller
			/*
            data = setting
            0x01e = 320x256         ---- ---x xxx-
            0x0e1 = 320x256 REVERSE ---- xxx- ---x
            0x016 = 320x240         ---- ---x -xx-
            0x0e9 = 320x240 REVERSE ---- xxx- x--x
            0x004 = 320x224         ---- ---- -x--
            0x10b = 320x224 REVERSE ---x ---- x-xx
            For now we use this by cases and not per bits.
            */

			switch(data)
			{
				case 0x0000:
				case 0x001e: CRT_MODE(320,256,0); break;
				case 0x00e1: CRT_MODE(320,256,1); break;
				case 0x0016: CRT_MODE(320,240,0); break;
				case 0x00e9: CRT_MODE(320,240,1); break;
				case 0x0004: CRT_MODE(320,224,0); break;
				case 0x010b: CRT_MODE(320,224,1); break;
				default:
				#ifdef MAME_DEBUG
				popmessage("Warning: Undefined CRT Mode %04x",data);
				#endif
				CRT_MODE(320,256,0);
			}
			break;
		}
		/*TODO: what's going on here,some scroll values aren't sent in these locations
                but somewhere else?*/
		case (0x22c/2): { legionna_scrollram16[0] = cop_mcu_ram[offset]; break; }
		case (0x22e/2): { legionna_scrollram16[1] = cop_mcu_ram[offset]; break; }
		case (0x230/2): { legionna_scrollram16[2] = cop_mcu_ram[offset]; break; }
		case (0x232/2): { legionna_scrollram16[3] = cop_mcu_ram[offset]; break; }
		case (0x234/2): { legionna_scrollram16[4] = cop_mcu_ram[offset]; break; }
		case (0x236/2): { legionna_scrollram16[5] = cop_mcu_ram[offset]; break; }
		case (0x238/2): { legionna_scrollram16[6] = cop_mcu_ram[offset]; break; }
		case (0x23a/2): { legionna_scrollram16[7] = cop_mcu_ram[offset]; break; }

		case (0x340/2):	{ seibu_main_word_w(space,0,cop_mcu_ram[offset],0x00ff); break; }
		case (0x344/2):	{ seibu_main_word_w(space,1,cop_mcu_ram[offset],0x00ff); break; }
		case (0x350/2):	{ seibu_main_word_w(space,4,cop_mcu_ram[offset],0x00ff); break; }
		case (0x358/2):	{ seibu_main_word_w(space,6,cop_mcu_ram[offset],0x00ff); break; }
	}
}

/**********************************************************************************************
  Godzilla
**********************************************************************************************/

READ16_HANDLER( godzilla_mcu_r )
{
	switch (offset)
	{
		default:
			return generic_cop_r(space, offset, mem_mask);

		/* Non-protection reads */
		case (0x308/2):	return seibu_main_word_r(space,2,0xffff);
		case (0x30c/2):	return seibu_main_word_r(space,3,0xffff);
		case (0x314/2):	return seibu_main_word_r(space,5,0xffff);

		/* Inputs */
		case (0x340/2): return input_port_read(space->machine, "DSW1");
		case (0x344/2): return input_port_read(space->machine, "PLAYERS12");
		case (0x348/2): return input_port_read(space->machine, "PLAYERS34");
		case (0x34c/2): return input_port_read(space->machine, "SYSTEM");
	}
}

WRITE16_HANDLER( godzilla_mcu_w )
{
	COMBINE_DATA(&cop_mcu_ram[offset]);

	switch (offset)
	{
		default:
			generic_cop_w(space, offset, data, mem_mask);
			break;


		case (0x220/2): { legionna_scrollram16[0] = cop_mcu_ram[offset]; break; }
		case (0x222/2): { legionna_scrollram16[1] = cop_mcu_ram[offset]; break; }
		case (0x224/2): { legionna_scrollram16[2] = cop_mcu_ram[offset]; break; }
		case (0x226/2): { legionna_scrollram16[3] = cop_mcu_ram[offset]; break; }
		case (0x228/2): { legionna_scrollram16[4] = cop_mcu_ram[offset]; break; }
		case (0x22a/2): { legionna_scrollram16[5] = cop_mcu_ram[offset]; break; }

		case (0x300/2):	{ seibu_main_word_w(space,0,cop_mcu_ram[offset],0x00ff); break; }
		case (0x304/2):	{ seibu_main_word_w(space,1,cop_mcu_ram[offset],0x00ff); break; }
		case (0x310/2):	{ seibu_main_word_w(space,4,cop_mcu_ram[offset],0x00ff); break; }
		case (0x318/2):	{ seibu_main_word_w(space,6,cop_mcu_ram[offset],0x00ff); break; }
	}
}

/**********************************************************************************************
  Denjin Makai
**********************************************************************************************/

READ16_HANDLER( denjinmk_mcu_r )
{
	switch (offset)
	{
		default:
			return generic_cop_r(space, offset, mem_mask);

		/* Non-protection reads */

		case (0x308/2):	return seibu_main_word_r(space,2,0xffff);
		case (0x30c/2):	return seibu_main_word_r(space,3,0xffff);
		case (0x314/2): return seibu_main_word_r(space,5,0xffff);

		/* Inputs */
		case (0x340/2): return input_port_read(space->machine, "DSW1");
		case (0x344/2):	return input_port_read(space->machine, "PLAYERS12");
		case (0x348/2): return input_port_read(space->machine, "PLAYERS34");
		case (0x34c/2): return input_port_read(space->machine, "SYSTEM");
		case (0x35c/2): return input_port_read(space->machine, "DSW2");
	}
}

WRITE16_HANDLER( denjinmk_mcu_w )
{
	COMBINE_DATA(&cop_mcu_ram[offset]);

	switch (offset)
	{
		default:
			generic_cop_w(space, offset, data, mem_mask);
			break;

		//case (0x05a/2): { /* brightness?? */ break; }
		case (0x070/2): { denjinmk_setgfxbank(cop_mcu_ram[offset]); break; }

		case (0x21c/2): { legionna_layer_disable = cop_mcu_ram[offset]; break; }

		case (0x220/2): { legionna_scrollram16[0] = cop_mcu_ram[offset]; break; }
		case (0x222/2): { legionna_scrollram16[1] = cop_mcu_ram[offset]; break; }
		case (0x224/2): { legionna_scrollram16[2] = cop_mcu_ram[offset]; break; }
		case (0x226/2): { legionna_scrollram16[3] = cop_mcu_ram[offset]; break; }
		case (0x228/2): { legionna_scrollram16[4] = cop_mcu_ram[offset]; break; }
		case (0x22a/2): { legionna_scrollram16[5] = cop_mcu_ram[offset]; break; }


		//case (0x280/2): { /* trigger.. something */ break; }

		case (0x300/2):	{ seibu_main_word_w(space,0,cop_mcu_ram[offset],0x00ff); break; }
		case (0x304/2):	{ seibu_main_word_w(space,1,cop_mcu_ram[offset],0x00ff); break; }
		case (0x310/2):	{ seibu_main_word_w(space,4,cop_mcu_ram[offset],0x00ff); break; }
		case (0x318/2):	{ seibu_main_word_w(space,6,cop_mcu_ram[offset],0x00ff); break; }

	}
}

/**********************************************************************************************
  SD Gundam Rainbow Trout
**********************************************************************************************/

READ16_HANDLER( grainbow_mcu_r )
{
	switch (offset)
	{
		default:
			return generic_cop_r(space, offset, mem_mask);

		/*hit protection*/
		case (0x180/2): { return xy_check; }

		case (0x1b0/2):
			return 2;
			// FIXME: this code is never reached
			/*check if the DMA has been finished*/
			if(dma_status == 1)
			{
				dma_status = 0;
				return 2;
			}
			return cop_mcu_ram[offset];

		/* Non-protection reads */
		case (0x308/2): return seibu_main_word_r(space,2,0xffff);
		case (0x30c/2): return seibu_main_word_r(space,3,0xffff);
		case (0x314/2): return seibu_main_word_r(space,5,0xffff);

		/* Inputs */
		case (0x340/2): return input_port_read(space->machine, "DSW1");
		case (0x344/2):	return input_port_read(space->machine, "PLAYERS12");
		case (0x348/2): return input_port_read(space->machine, "PLAYERS34");
		case (0x34c/2): return input_port_read(space->machine, "SYSTEM");
		case (0x35c/2): return input_port_read(space->machine, "DSW2");
	}
}


WRITE16_HANDLER( grainbow_mcu_w )
{
	COMBINE_DATA(&cop_mcu_ram[offset]);

	switch (offset)
	{
		default:
			generic_cop_w(space, offset, data, mem_mask);
			break;

		/*********************************************************************
        400-5ff -  Protection writes
        *********************************************************************/

		case (0x00c/2): { dma_size = cop_mcu_ram[offset]; break; }

		/*DMA source address*/
		case (0x012/2): { prot_data[1] = cop_mcu_ram[offset]; dma_src = (prot_data[0]&0xffff)|((prot_data[1]&0xffff)<<16); break; }
		case (0x014/2): { prot_data[0] = cop_mcu_ram[offset]; dma_src = (prot_data[0]&0xffff)|((prot_data[1]&0xffff)<<16); break; }

		/* Execute Macro Command */
		case (0x100/2):
		{
			switch(cop_mcu_ram[offset])
			{
				case 0xa180:/*do the job [1]*/
				{
					//popmessage("%08x %08x %04x",dma_src,cop_register[5]+4,dma_size);
					/*fix the offset for easier reading*/
					dma_src+=4;
					//cop_register[5]+=4;
					s_i = dma_size;
					//cop_register[5]+=((memory_read_word(space, 0x110000) & 0x000f) * 8);
					//memory_write_word(space, 0x1004c8,cop_register[5] & 0xffff);
					//dma_status = 1;
					break;
				}
				case 0xb100:
					break;
				case 0xa980:
					break;
				case 0xb900:
				{
					xy_check = hit_check_jsr(space);
					break;
				}
				/*bullet movement protection,conflicts with [3],will be worked out*/
				case 0x8100:
				{
					//move3x_prot_jsr(space);
					break;
				}
				case 0x8900:
				{
					//move3y_prot_jsr(space);
					break;
				}
				case 0x0205:/*do the job [3]*/
				{
					moveprot_jsr(space);
					break;
				}
				case 0x138e:/*do the job [4]*/
					break;
				case 0x3bb0:
				{
					move2prot_jsr(space);
					break;
				}
				default:
					seibu_cop_log("DMA CMD 0x500 with parameter = %04x PC = %08x\n",cop_mcu_ram[offset],cpu_get_previouspc(space->cpu));
			}
			break;
		}

		case (0x102/2):
		{
			if(cop_mcu_ram[offset] == 0xc480)
			{
				dma_transfer(space);
				s_i--;
				if(s_i == 0)
					dma_status = 1;
			}
			break;
		}

		/*Layer Enable,bit wise active low*/
		case (0x21c/2):
		{
			/*
            ---x ---- (used in test mode)
            ---- x--- Text Layer
            ---- -x-- Foreground Layer
            ---- --x- Midground Layer
            ---- ---x Background Layer
            */
			grainbow_pri_n = cop_mcu_ram[offset] & 0xf;
			break;
		}

		/* TODO: tilemaps x-axis are offset,we use a temporary kludge for now */
		case (0x220/2):	{ legionna_scrollram16[0] = 0x10 + cop_mcu_ram[offset]; break; }
		case (0x222/2): { legionna_scrollram16[1] = cop_mcu_ram[offset]; break; }
		case (0x224/2): { legionna_scrollram16[2] = 0x10 + cop_mcu_ram[offset]; break; }
		case (0x226/2): { legionna_scrollram16[3] = cop_mcu_ram[offset]; break; }
		case (0x228/2): { legionna_scrollram16[4] = 0x10 + cop_mcu_ram[offset]; break; }
		case (0x22a/2): { legionna_scrollram16[5] = cop_mcu_ram[offset]; break; }

		/* scroll mirrors? */
		case (0x22c/2):
		case (0x22e/2):
		case (0x230/2):
		case (0x232/2):
		case (0x234/2):
		case (0x236/2):
			break;


		/* Text Layer scroll registers */
		case (0x238/2): { legionna_scrollram16[6] = 0x38 + cop_mcu_ram[offset]; break; }
		case (0x23a/2): { legionna_scrollram16[7] = cop_mcu_ram[offset]; break; }
		/*C.R.T. Controller (note:game calls it OBJ register)*/
		case (0x244/2):
			{
				/*
                data = setting
                0x01e = 320x256
                0x0e1 = 320x256 REVERSE
                0x016 = 320x240
                0x0e9 = 320x240 REVERSE
                0x004 = 320x224
                0x10b = 320x224 REVERSE
                It is like to be per cases and not per bits.
                */
				switch(data)
				{
					case 0x0000:
					case 0x0003:
					case 0x001e: CRT_MODE(320,224,0); break;
					case 0x00e1: CRT_MODE(320,224,1); break;
					case 0x0016: CRT_MODE(320,256,0); break;
					case 0x00e9: CRT_MODE(320,256,1); break;
					case 0x0004: CRT_MODE(320,240,0); break;
					case 0x00fb: CRT_MODE(320,240,1); break;
					default:
					#ifdef MAME_DEBUG
					popmessage("Warning: Undefined CRT Mode %04x",data);
					#endif
					CRT_MODE(320,256,0);
				}
			}
			break;

		/* Seems a mirror for the choices in the test menu... */
		//case (0x27c/2): break;
		//case (0x280/2): break;
		//case (0x6fc/2): break;

		case (0x300/2):	{ seibu_main_word_w(space,0,cop_mcu_ram[offset],0x00ff); break; }
		case (0x304/2):	{ seibu_main_word_w(space,1,cop_mcu_ram[offset],0x00ff); break; }
		case (0x310/2):	{ seibu_main_word_w(space,4,cop_mcu_ram[offset],0x00ff); break; }
		case (0x318/2):	{ seibu_main_word_w(space,6,cop_mcu_ram[offset],0x00ff); break; }
	}
}

/**********************************************************************************************
  Legionnaire
**********************************************************************************************/


READ16_HANDLER( legionna_mcu_r )
{
	switch (offset)
	{
		default:
			return generic_cop_r(space, offset, mem_mask);

		/*********************************************************************
        400-5ff -  Protection reads
        *********************************************************************/

		case (0x070/2):	return (mame_rand(space->machine) &0xffff); /* read PC $110a, could be some sort of control word:  sometimes a bit is changed then it's poked back in... */
		case (0x182/2):	return (0); /* read PC $3594 */
		case (0x184/2):	return (0); /* read PC $3588 */
		case (0x186/2):	return (0); /* read PC $35a0 */
		case (0x188/2):	return hit_check; /* read PC $3580 */
		case (0x1b0/2):	return (0); /* bit 15 is branched on a few times in the $3300 area */
		case (0x1b4/2):	return (0); /* read and stored in ram before +0x5b0 bit 15 tested */


		/*********************************************************************
        700-7ff - Non-protection reads
        *********************************************************************/

		/* Seibu Sound System */
		case (0x308/2):	return seibu_main_word_r(space,2,0xffff);
		case (0x30c/2):	return seibu_main_word_r(space,3,0xffff);
		case (0x314/2): return seibu_main_word_r(space,5,0xffff);

		/* Inputs */
		case (0x340/2): return input_port_read(space->machine, "DSW1");
		case (0x344/2):	return input_port_read(space->machine, "PLAYERS12");
		case (0x348/2):	return input_port_read(space->machine, "COIN");
		case (0x34c/2):	return input_port_read(space->machine, "SYSTEM");

	}
}


WRITE16_HANDLER( legionna_mcu_w )
{
	COMBINE_DATA(&cop_mcu_ram[offset]);

	seibu_cop_log("%06x: Legionna write data %04x at offset %04x\n", cpu_get_pc(space->cpu), data, offset*2);


	switch (offset)
	{
		default:
			generic_cop_w(space, offset, data, mem_mask);
			break;

		/* Execute Macro command from table */
		case (0x100/2):
		{
			/*Movement protection*/
			if(cop_mcu_ram[offset] == 0x8900 || cop_mcu_ram[offset] == 0x0205)
			{
				static UINT16 xy_data[2];
				static UINT8 k;
				xy_data[0] = memory_read_word(space, cop_register[2]);
				xy_data[1] = memory_read_word(space, cop_register[3]);
				k = (cop_mcu_ram[offset] == 0x0205) ? ENEMY : PLAYER;
				protection_move_jsr(space,cop_register[0],k);
				//protection_move_jsr(space,cop_register[1]); //???
				//popmessage("%08x %08x %04x %04x",cop_register[0],cop_register[1],xy_data[0],xy_data[1]);
			}
			else if(cop_mcu_ram[offset] == 0x3bb0 || cop_mcu_ram[offset] == 0x138e)
			{
				protection_hit_jsr(space,cop_register[0],cop_register[1]);
			}
			break;
		}

		/*********************************************************************
        600-6ff - Video Registers
        *********************************************************************/

		// 61a bit 0 is flipscreen
		// 61c probably layer disables, like Dcon

		case (0x220/2): { legionna_scrollram16[0] = cop_mcu_ram[offset]; break; }
		case (0x222/2): { legionna_scrollram16[1] = cop_mcu_ram[offset]; break; }
		case (0x224/2): { legionna_scrollram16[2] = cop_mcu_ram[offset]; break; }
		case (0x226/2): { legionna_scrollram16[3] = cop_mcu_ram[offset]; break; }
		case (0x228/2): { legionna_scrollram16[4] = cop_mcu_ram[offset]; break; }
		case (0x22a/2): { legionna_scrollram16[5] = cop_mcu_ram[offset]; break; }

		/*********************************************************************
        700-7ff - Output (Seibu Sound System)
        *********************************************************************/

		case (0x300/2):	{ seibu_main_word_w(space,0,cop_mcu_ram[offset],0x00ff); break; }
		case (0x304/2):	{ seibu_main_word_w(space,1,cop_mcu_ram[offset],0x00ff); break; }
		case (0x310/2):	{ seibu_main_word_w(space,4,cop_mcu_ram[offset],0x00ff); break; }
		case (0x318/2):	{ seibu_main_word_w(space,6,cop_mcu_ram[offset],0x00ff); break; }
	}
}


/**********************************************************************************************
  Raiden 2 / Zero Team
**********************************************************************************************/

READ16_HANDLER( raiden2_mcu_r )
{
	switch (offset)
	{
		default:
			return generic_cop_r(space, offset, mem_mask);

		case (0x340/2): return input_port_read(space->machine, "DSWA") | (input_port_read(space->machine, "DSWB") << 8);
		case (0x344/2): return input_port_read(space->machine, "P1") | (input_port_read(space->machine, "P2") << 8);
		case (0x34c/2): return input_port_read(space->machine, "SYSTEM") | 0xff00;

		/* Inputs */
		case (0x308/2):	return seibu_main_word_r(space,2,0xffff);
		case (0x30c/2):	return seibu_main_word_r(space,3,0xffff);
		case (0x314/2): return seibu_main_word_r(space,5,0xffff);

	}
}

WRITE16_HANDLER( raiden2_mcu_w )
{
	COMBINE_DATA(&cop_mcu_ram[offset]);

	seibu_cop_log("%06x: raiden2 write data %04x at offset %04x\n", cpu_get_pc(space->cpu), data, offset*2);

	switch (offset)
	{
		default:
			generic_cop_w(space, offset, data, mem_mask);
			break;

		case (0x2a0/2): sprcpt_val_1_w(space,offset,data,mem_mask); break;
		case (0x2a2/2): sprcpt_val_1_w(space,offset,data,mem_mask); break;
		case (0x2a4/2): sprcpt_data_3_w(space,offset,data,mem_mask); break;
		case (0x2a6/2): sprcpt_data_3_w(space,offset,data,mem_mask); break;
		case (0x2a8/2): sprcpt_data_4_w(space,offset,data,mem_mask); break;
		case (0x2aa/2): sprcpt_data_4_w(space,offset,data,mem_mask); break;
		case (0x2ac/2): sprcpt_flags_1_w(space,offset,data,mem_mask); break;
		case (0x2ae/2): sprcpt_flags_1_w(space,offset,data,mem_mask); break;
		case (0x2b0/2): sprcpt_data_1_w(space,offset,data,mem_mask); break;
		case (0x2b2/2): sprcpt_data_1_w(space,offset,data,mem_mask); break;
		case (0x2b4/2): sprcpt_data_2_w(space,offset,data,mem_mask); break;
		case (0x2b6/2): sprcpt_data_2_w(space,offset,data,mem_mask); break;
		case (0x2b8/2): sprcpt_val_2_w(space,offset,data,mem_mask); break;
		case (0x2ba/2): sprcpt_val_2_w(space,offset,data,mem_mask); break;
		case (0x2bc/2): sprcpt_adr_w(space,offset,data,mem_mask); break;
		case (0x2be/2): sprcpt_adr_w(space,offset,data,mem_mask); break;
		case (0x2ce/2): sprcpt_flags_2_w(space,offset,data,mem_mask); break;

		case (0x300/2):	{ seibu_main_word_w(space,0,cop_mcu_ram[offset],0x00ff); break; }
		case (0x304/2):	{ seibu_main_word_w(space,1,cop_mcu_ram[offset],0x00ff); break; }
		case (0x310/2):	{ seibu_main_word_w(space,4,cop_mcu_ram[offset],0x00ff); break; }
		case (0x318/2):	{ seibu_main_word_w(space,6,cop_mcu_ram[offset],0x00ff); break; }


	}
}
