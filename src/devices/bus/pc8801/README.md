# PC-8801 EXPansion bus

## List of known cards
- NEC PC-8801-10
  > MIDI interface
- NEC PC-8801-11
  > "Sound Board", single YM2203C OPN, single joy port, mono out
- NEC PC-8801-12
  > Modem board, full duplex 300bps
- NEC PC-8801-13
  > Parallel I/F board
- NEC PC-8801-16
  > extra μPD8086 CPU + 8253, 8255 and 8259 i/f, able to run MS-DOS 1.25, requires extra ROMs for both sides
- NEC PC-8801-17
- NEC PC-8801-18
  > VTR capture card "Video art board" / "Video digitizing unit", 16-bit color
- NEC PC-8801-21
  > CMT i/f board
- NEC PC-8801-22
  > "Multi board B", upgrades a FH to MH and FA to MA (?)
- NEC PC-8801-23
- NEC PC-8801-24
- NEC PC-8801-25
  > "Sound Board 2", single YM2608 OPNA, single joy port, stereo out. -24 is the internal FH / MH version, -25 is the internal FE / FE2 with YM2608B. Standard and on main board instead for FA / MA and onward;
- NEC PC-8801-30
- NEC PC-8801-31
  > -31 is CD-ROM SCSI i/f (built-in for MC, optional for MA+), -30 is virtually same to PC Engine CD-ROM² drive
- NEC PC-8864
  > Network board mapping at $a0-$a3
- HAL PCG-8100
  > PCG and 3x DAC_1BIT at I/O $01, $02. PIT at $0c-$0f
- HAL GSX-8800
  > 2x PSG at I/O $a0-$a3, mono out. Has goofy extra connector on top and a couple jumpers, guess it may cascade with another board for 2x extra PSGs at $a4-$a7
- HIBIKI-8800
  > YM2151 OPM + YM3802-X MIDI controller, stereo out, has own internal XTAL @ 4MHz. Has an undumped PAL/PROM labeled "HAL-881"
- HAL HMB-20
  > earlier version of HIBIKI-8800 board with no MIDI? cfr. GH [#10703](https://github.com/mamedev/mame/issues/10703)
- JMB-X1
  > "Sound Board X", 2x OPM + 1x SSG. Used by NRTDRV88J, more info at GH [#8709](https://github.com/mamedev/mame/issues/8709)
