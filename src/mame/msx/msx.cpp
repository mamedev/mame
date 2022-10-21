// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*
** msx.cpp : driver for MSX family machines
**
** Special usage notes:
**
** ax350iif:
**  The machine has a French keyboard so to select firmware options you
**  need to enter numbers as shift + number key.
**
** cpc300:
**  To get out of the MSX Tutor press the SELECT key. Entering SET SYSTEM 1 should
**  disable the MSX Tutor on next boot and SET SYSTEM 0 should enable.
**
** hx21, hx22:
**  To start the firmware, mount a HX-M200 cartridge and type: CALL JWP.
**
** tpp311:
**  This machine is supposed to boot into logo; it was made to only run logo.
**
** tps312:
**  - To get into MSX-WRITE type: CALL WRITE
**  - To get into MSX-PLAN type: CALL MSXPLAN
**
**
**
** Todo/known issues:
** - Get rid of trampolines and code duplication between msx_slot and msx_cart (eg, kanji roms, disk interfaces)
** - general: - Add support for kana lock
** -          - Expansion slots not emulated
** - kanji: The direct rom dump from FS-A1FX shows that the kanji font roms are accessed slightly differently. Most
**          existing kanji font roms may haven been dumped from inside a running machine. Are all other kanji font
**          roms bad? We need more direct rom dumps to know for sure.
** - rs232 support:
**   - mlg3 (working, how does the rs232 switch work?)
**   - mlg30_2 (working, how does the rs232 switch work?)
**   - hbg900ap (not working, switch for terminal/moden operation)
**   - hbg900p (not working)
**   - victhc90 (cannot test, system config not emulated)
**   - victhc95 (cannot test, system config not emulated)
**   - victhc95a (cannot test, system config not emulated)
**   - y805256 (cannot test, rs232 rom has not been dumped?)
** - inputs:
**     bruc100: Not all keypad keys hooked up yet
**     fs4000: Is the keypad enter exactly the same as the normal enter key? There does not appear to be a separate mapping for it.
**     piopx7: The keyboard responds like a regular international keyboard, not a japanese keyboard.
**     svi728/svi728es: How are the keypad keys mapped?
** - ax230: Some builtin games show bad graphics, example: Press 5 in main firmware screen and choose the first game
** - piopx7/piopx7uk/piopxv60: Pioneer System Remote (home entertainment/Laserdisc control) not implemented
** - spc800: How to test operation of the han rom?
** - mbh1, mbh1e, mbh2, mbh25: speed controller
** - cx5m128, yis503iir, yis604: yamaha mini cartridge slot
** - cpg120: turbo button
** - fs4000: Internal thermal printer not emulated
** - fs4500, fs4600f, fs4700:
**   - switch to bypass firmware
**   - switch to switch between internal and external printer
**   - switch to switch betweem jp50on and jis layout
**   - internal printer
** - fs5500f1, fs5500f2:
**   - switch to switch between jp50on and jis layout
**   - switcn to bypass firmware
** - fsa1fm:
**   - Firmware and modem partially emulated
** - fsa1mk2: pause button
** - nms8260: HDD not emulated
** - phc77: builtin printer, switch to turn off firmware
** - hbf1: pause button
** - hbf1ii: rensha turbo slider
** - hbf1xd, hbf1xdj, hbf1xv
**   - pause button
**   - speed controller slider
**   - rensha turbo slider
** - victhc90, victhc95, victhc95a: Turbo/2nd cpu not supported. Firmware not working.
** - fsa1fx, fsa1wsx, fsa1wx
**   - rensha turbo slider
**   - pause button
**   - firmware switch
** - phc35j, phc70fd: rensha turbo slider, pause button
** - y503iir, y503iir2: Switch for teacher/student mode, net not emulated
** - cpc300: Config for MSX Tutor ON/OFF is not saved
** - cpc300e: Remove joystick connectors
** - nms8280, nms8280g, and others: Digitizer functionality not emulated
** - expert3i: IDE not emulated
** - expert3t: Turbo not emulated
** - expertac: Does not boot
** - fsa1gt, fsa1st: Add Turbo-R support
** - cpc50a/cpc50b: Remove keyboard; and add an external keyboard??
** - cpc51/cpc61: Remove keyboard and add a keyboard connector
** - mbh3: touch pad not emulated
** - mbh70: Verify firmware operation
** - y805*: Floppy support broken
** - y8805128r2/e: Firmware not working
** - cpg120: Remove ports
**
** TODO:
** - Add T6950 support. T6950 is selectable between pal and ntsc by a pin.
**
** Possibly missing machines:
** - Sanyo MPC-1 (T6950)
** - Sony HB-101 (TMS9118)
************************************************************************

This following list is probably incomplete. Corrections are welcome.
Entries marked with * still need to be processed.

*Ascii MSXPLAYer 2003
*Ascii One Chip MSX
ACT DPC-200 - MSX1 - See Fenner DPC-200 (they are the same machine)
Canon V-8 - MSX1 - canonv8
Canon V-10 - MSX1 - canonv10
Canon V-20 - MSX1 - canonv20
Canon V-20E - MSX1 - canonv20e
Canon V-20F - MSX1 - canonv20f
Canon V-20G - MSX1 - canonv20g
Canon V-20S - MSX1 - canonv20s
Canon V-25 - MSX2 - canonv25
Canon V-30F - MSX2 - canonv30f
Casio MX-10 - MSX1 - mx10
Casio MX-15 - MSX1 - mx15
Casio MX-101 - MSX1 - mx101
Casio PV-7 - MSX1 - pv7
*Casio PV-7 + KB-7
Casio PV-16 - MSX1 - pv16
Ciel Expert 3 IDE - MSX2+ - expert3i
Ciel Expert 3 Turbo = MSX2+ - expert3t
======================================
This one is a full motherboard by CIEL (not an upgrade kit), created to replace the motherboard of a Gradiente Expert (which means that only the case, the analog boards and the keyboard remains Gradiente). This new motherboard has the following built-in features:

1) MSX2+
2) Support either 3.57MHz or 7.14MHz natively, switched either by software (*1) or by a hardware-switch on the front panel. Turbo-led included.
3) Up to 4MB of Memory Mapper (1MB is the most common configuration)
4) MSX-Music
5) 4 expansion slots (two external on the front panel, two internal)
6) Stereo sound (YM2413 channels 0-6 on right, PSG+YM2413 channels 7-9 on left)
7) Support the V9938 instead of the V9958 by switching some jumpers
8) The main-ram can be placed on slot 2 or slot 3, using jumpers (slot 2 is the default)


*1: A routine hidden inside the BIOS frame-0 is used to switch the turbo.

Daewoo CPC-88 - MSX1 - cpc88
Daewoo CPC-300 - MSX2 - cpc300
Daewoo CPC-300E - MSX2 - cpc300e
Daewoo CPC-330K - MSX2 - cpc330k
Daewoo CPC-400 - MSX2 - cpc400
Daewoo CPC-400S - MSX2 cpc400s
Daewoo DPC-100 - MSX1 - dpc100
Daewoo DPC-180 - MSX1 - dpc180
Daewoo DPC-200 - MSX1 - dpc200
Daewoo DPC-200E - MSX1 - dpc200e
Daewoo Zemmix CPC-50A - MSX1 - cpc50a
Daewoo Zemmix CPC-50B - MSX1 - cpc50b
Daewoo Zemmix CPC-51 - MSX1 - cpc51
Daewoo Zemmix CPC-61 - MSX2 - cpc61
Daewoo Zemmix CPG-120 Normal (no clock chip, no printer port) - MSX2 - cpg120
*Daewoo Zemmix CPG-120 Turbo - MSX2 - cpg120t
Fenner DPC-200 - MSX1 - fdpc200
Fenner FPC-500 - MSX1 - fpc500
Fenner FPC-900 - MSX2 - fpc900
Fenner SPC-800 - MSX1 - fspc800
Frael Bruc 100-1 - MSX1 - bruc100
Fujitsu FM-X - MSX1 - fmx
Goldstar FC-80U - MSX1 - gsfc80u
Goldstar FC-200 - MSX1 - gsfc200
Goldstar GFC-1080 - MSX1 - gfc1080
Goldstar GFC-1080A - MSX1 - gfc1080a
Gradiente Expert XP-800 (1.0) - MSX1 - expert10
Gradiente Expert XP-800 (1.1) / Expert GPC-1 - MSX1 - expert11
Gradiente Expert 1.3 - MSX1 - expert13
Gradiente Expert 2.0 - MSX2 - expert20
Gradiente Expert AC88+ - MSX2+ - expertac
Gradiente Expert DDPlus - MSX1 - expertdp
Gradiente Expert DDX+ - MSX2+ - expertdx
Gradiente Expert Plus - MSX1 - expertpl
*Haesung Virtual Console
Hitachi MB-H1 - MSX1 - mbh1
Hitachi MB-H1E - MSX1 - mbh1e
Hitachi MB-H2 - MSX1 - mbh2
*Hitachi MB-H3 - MSX2 (64KB VRAM)
*Hitachi MB-H21 - MSX1
Hitachi MB-H25 - MSX1 - mbh25
Hitachi MB-H50 - MSX1 - mbh50
Hitachi MB-H70 - MSX2 - mbh70
*Hitachi MB-H80 - MSX1
JVC HC-7E / HC-7GB - MSX1 -jvchc7gb
*Jotan Holland Bingo - MSX1
Kawai KMC-5000 - MSX2 - kmc5000
*Misawa-Van CX-5
Mitsubishi ML-8000 - MSX1 - ml8000
Mitsubishi ML-F48 - MSX1 - mlf48
Mitsubishi ML-F80 - MSX1 - mlf80
Mitsubishi ML-F110 - MSX1 - mlf110
Mitsubishi ML-F120 - MSX1 - mlf120
Mitsubishi ML-FX1 - MSX1 - mlfx1
Mitsubishi ML-G1 - MSX2 - mlg1
Mitsubishi ML-G3 - MSX2 - mlg3
Mitsubishi ML-G10 - MSX2 - mlg10
Mitsubishi ML-G30 Model 1 - MSX2 - mlg30
Mitsubishi ML-G30 Model 2 - MSX2 - mlg30_2
National CF-1200 - MSX1 - cf1200
National CF-2000 - MSX1 - cf2000
National CF-2700 - MSX1 - cf2700
National CF-3000 - MSX1 - cf3000
National CF-3300 - MSX1 - cf3300
National FS-1300 - MSX1 - fs1300
National FS-4000 - MSX1 - fs4000
National FS-4000 (alt) - MSX1 - fs4000a
National FS-4500 - MSX2 - fs4500
National FS-4600 - MSX2 - fs4600f
National FS-4700 - MSX2 - fs4700f
National FS-5000F2 - MSX2 - fs5000F2
National FS-5500F1 - MSX2 - fs5500
National FS-5500F2 - MSX2 - fs5500
Olympia PHC-2 - MSX1 - phc2
Olympia PHC-28 - MSX1 - phc28
Panasonic CF-2700 (Germany) - MSX1 - cf2700g
Panasonic CF-2700 (UK) - MSX1 - cf2700uk
*Panasonic FS-3900 - MSX1
Panasonic FS-A1 - MSX2 - fsa1 / fsa1a
Panasonic FS-A1 MK2 - MSX2 - fsa1mk2
Panasonic FS-A1F - MSX2 - fsa1f
Panasonic FS-A1FM - MSX2 - fsa1fm
Panasonic FS-A1FX - MSX2+ - fsa1fx
Panasonic FS-A1GT - MSX Turbo-R - fsa1gt
Panasonic FS-A1ST - MSX Turbo-R - fsa1st
Panasonic FS-A1WSX - MSX2+ - fsa1wsx
Panasonic FS-A1WX - MSX2+ - fsa1wx / fsa1wxa
Perfect Perfect1 - MSX1 - perfect1
*Perfect Perfect2 - MSX2
Philips NMS 801 - MSX1 - nms801
Philips NMS 8220 - MSX2 - nms8220 / nms8220a
Philips NMS 8245 - MSX2 - nms8245
Philips NMS 8245F - MSX2 - nms8245f
Philips NMS 8250 - MSX2 - nms8250
Philips NMS 8250F - MSX2 - nms8250f
Philips NMS 8255 - MSX2 - nms8255
Philips NMS 8255F - MSX2 - nms8255f
Philips NMS 8260 - MSX2 - nms8260
Philips NMS 8280 - MSX2 - nms8280
Philips NMS 8280F - MSX2 - nms8280f
Philips NMS 8280G - MSX2 - nms8280g
*Philips PTC MSX PC
Philips VG-8000 - MSX1 - vg8000
Philips VG-8010 / VG-8010/00 - MSX1 - vg8010
Philips VG-8010F / VG-8010/19 - MSX1 - vg8010f
Philips VG-8020/00 - MSX1 - vg802000
Philips VG-8020/20 - MSX1 - vg802020
*Philips VG-8020/29 - MSX1
*Philips VG-8020/40 - MSX1
Philips VG-8020F - MSX1 - vg8020f
Philips VG-8230 - MSX2 - vg8230
Philips VG-8235 - MSX2 - vg8235
Philips VG-8235F - MSX2 - vg8235f
Philips VG-8240 - MSX2 - vg8240
===============================

PCB Layout missing


Pioneer PX-7 - MSX1 - piopx7
============================

|---------------------------------------|
|  CN1     CN2                          |
|                                       |
|                                       |
|  IC33                                 |---------------------------------|
|                                                      CN3                |
|   IC32   IC34            IC38  IC40                                     |
|                                                               IC20      |
|   IC15   IC18  IC43      IC8   IC35   IC6     |----IC3---|              |
|                                               |----------|    IC21      |
|   IC16   IC19  |---IC13---|    IC7    IC10                              |
|                |----------|                   IC36  IC29      ---       |
|   IC17   IC14                                     X2          | |       |
|                |--IC12---|     |----IC1-----|       IC37      |I|       |
|   IC28   IC11  |---------|     |------------|   X1            |C|       |
|                                                               |2|       |
|  |----IC4----| |----IC5----|   IC39  IC9      IC42  IC44      | |       |
|  |-----------| |-----------|                                  ---       |
|                                                                         |
|       IC45   IC31    IC30      IC41                                     |
|                                                                         |
|  CN4 CN5  CN6  CN7                                  CN8                 |
|-------------------------------------------------------------------------|

Notes:
  X1 - 3.579MHz
  X2 - 500kHz
  IC1 - Sharp LH0080A Z80A-CPU-D
  IC2 - TMS91289NL
  IC3 - MB111S112  Z10 (500kHz)
  IC4  - M5L8255AP-5
  IC5  - YM2149F
  IC6,IC7,IC8,IC10,IC45 - SN74LS367AN
  IC9 - SN74LS245N
  IC11,IC34 - SN74LS139N
  IC12 - YM2301-23908 / 53 18 85 A (might indicate a version)
  IC13 - Pioneer PD5031 2364-213 514100 (M5L2764-213)
  IC14,IC17,IC30,IC31 - SN74LS157N
  IC15-IC19 - MB81416-12
  IC20,IC21 - TMS4416-I5NL
  IC28 - SN74LS153N
  IC29 - SN74LS02N
  IC32 - SN74LS374N
  IC33 - M5218P
  IC35 - SN74LS74AN
  IC36 - SN74LS30N
  IC37-IC39 - SN74LS04N
  IC40,IC41 - SN74LS05N
  IC42 - SN74LS08N
  IC43,IC44 - SN74LS32N
  CN1 - Printer
  CN2 - Cassette recorder
  CN3 - Expansion slot
  CN4 - Keyboard
  CN5 - Keyboard
  CN6 - Controller #1
  CN7 - Controller #2
  CN8 - Expansion slot


Pioneer PX-7UK - MSX1 - piopx7uk
Pioneer PX-V60 - MSX1 - piopxv60
*Sakhr AH-200 - MSX1
*Sakhr AX-100 - MSX1
Sakhr AX-150 - MSX1 - ax150
Sakhr AX-170 - MSX1 - ax170
Sakhr AX-200 (Arabic/English) - MSX1 - ax200
*Sakhr AX-200 (Arabic/French) - MSX1
Sakhr AX-200M - MSX1 - ax200m
Sakhr AX-230 - MSX1 - ax230
Sakhr AX-350II - MSX2 - ax350
Sakhr AX-370 - MSX2 - ax370
Sakhr AX-500 - MSX2 - ax500
Samsung SPC-800 MSX1 - spc800
Sanyo MPC-64 - MSX1 - mpc64
Sanyo MPC-100 - MSX1 - mpc100
Sanyo MPC-200 - MSX1 - mpc200
Sanyo MPC-200SP - MSX1 - mpc200sp
Sanyo MPC-2300 - MSX2 - mpc2300
Sanyo MPC-2500FD - MSX2 - mpc2500f
Sanyo PHC-28L - MSX1 - phc28l
Sanyo PHC-28S - MSX1 - phc28s
Sanyo Wavy MPC-10 - MSX1 - mpc10
Sanyo Wavy MPC-25FD - MSX2 - mpc25fd
Sanyo Wavy MPC-27 - MSX2 - mpc27
Sanyo Wavy PHC-23 - MSX2 - phc23
Sanyo Wavy PHC-35J - MSX2+ - phc35j
Sanyo Wavy PHC-55FD2 - MSX2 - phc55fd2
Sanyo Wavy PHC-70FD1 - MSX2+ - phc70fd
Sanyo Wavy PHC-70FD2 - MSX2+ - phc70fd2
Sanyo Wavy PHC-77 - MSX2 - phc77
Sharp Epcom HB-8000 (HotBit) - MSX1 - hb8000
Sharp Epcom HotBit 1.3b - MSX1 - hotbi13b
Sharp Epcom HotBit 1.3p - MSX1 - hotbi13p
Sharp Epcom HotBit 2.0 - MSX2 - hotbit20
Sony HB-10 - MSX1 - hb10
*Sony HB-10D - MSX1
Sony HB-10P - MSX1 - hb10p
Sony HB-20P - MSX1 - hb20p
Sony HB-55 Version 1 - MSX1 - hb55
Sony HB-55D - MSX1 - hb55d
Sony HB-55P - MSX1 - hb55p
Sony HB-75 - MSX1 - hb75
Sony HB-75D - MSX1 - hb75d
Sony HB-75P - MSX1 - hb75p
Sony HB-101 - MSX1 - hb101
Sony HB-101P - MSX1 - hb101p
Sony HB-201 - MSX1 - hp201
Sony HB-201P - MSX1 - hb201p
Sony HB-501P - MSX1 - hb501p
Sony HB-701FD - MSX1 - hb701fd
Sony HB-F1 - MSX2 - hbf1
Sony HB-F1II - MSX2 - hbf1ii
Sony HB-F1XD / HB-F1XDmk2 - MSX2 - hbf1xd
Sony HB-F1XDJ - MSX2+ - hbf1xdj
Sony HB-F1XV - MSX2+ - hbf1xv
Sony HB-F5 - MSX2 - hbf5
Sony HB-F500 - MSX2 - hbf500
Sony HB-F500F - MSX2 - hbf500f
Sony HB-F500P - MSX2 - hbf500p
Sony HB-F700D - MSX2 - hbf700d
Sony HB-F700F - MSX2 - hbf700f
Sony HB-F700P - MSX2 - hbf700p
Sony HB-F700S - MSX2 - hbf700s
*Sony HB-F750+
Sony HB-F900 - MSX2 - hbf900 / hbf900a
Sony HB-F9P - MSX2 - hbf9p
Sony HB-F9P Russian - MSX2 - hbf9pr
Sony HB-F9S - MSX2 - hbf9s
Sony HB-G900AP - MSX2 - hbg900ap
Sony HB-G900P - MSX2 - hbg900p
*Sony HB-T7
Spectravideo SVI-728 - MSX1 - svi728
Spectravideo SVI-738 - MSX1 - svi738
Spectravideo SVI-738 Arabic - MSX1 - svi738ar
Spectravideo SVI-738 Danish - MSX1 - svi738dk
Spectravideo SVI-738 Polish - MSX1 - svi738pl
Spectravideo SVI-738 Spanish - MSX1 - svi738sp
Spectravideo SVI-738 Swedish - MSX1 - svi738sw
Talent DPC-200 - MSX1 - tadpc200 / tadpc200b
Talent DPC-200A - MSX1 - tadpc200a
Talent TPC-310 - MSX2 - tpc310
Talent TPP-311 - MSX2 - tpp311
Talent TPS-312 - MSX2 - tps312
==============================

PCB Layouts missing


Toshiba HX-10 - MSX1 - hx10
===========================

Code on PCB: MSX TUK
        |---------------------------|-------------------|-------------|
        |   CN1  CN2  CN3  CN4               CN5                      |
        |                        |---------------------------|        |
        |                        |---------------------------|        |
        |                                    CN6                      |
        |                        IC40                                 |
        |                                                         CN7 |
        |                      IC38     IC32     IC33     IC37        |
        |                                                             |
        |                      Q2       IC31     IC34     IC35        |
        |    Q1                                                   CN8 |
        |                                                 IC39        |
        |   |--IC15------| |--IC2----|   |----IC1-----|               |
        |   |------------| |---------|   |------------|               |
        |                                                 IC30        |
        |                 IC3    IC4                              CN9 |
        |                               |-----IC15-------|            |
        |  IC17   IC18    IC7    IC8    |----------------|            |
        |                                                 IC27        |
        |  IC19   IC20    IC9    IC10   |----IC25------|              |
|----|  |                               |--------------|  IC26        |
| Q  |  |  IC21   IC22    IC11   IC12                                 |
|    |  |                                                             |
| S  |  |  IC23   IC24    IC13   IC14    IC29             IC28        |
|  L |  |                                                             |
|    |  |                                 CN11   CN10                 |
|----|  |-------------------------------------------------------------|

Notes:
  Mainboard components:
   IC1               - Sharp LH0080A Z80A-CPU-D
   IC2               - MB83256
   IC3,IC4,IC27,IC28 - Texas Instruments SN74LS157N
   IC7-IC14          - HM4864AP
   IC15              - Toshiba TCX-1007 (64pin custom chip)
   IC16              - 40pin chip covered with some kind of heatsink(?), probably TMS9929A
   IC17-IC24         - 4116-3
   IC25              - AY-3-8910A
   IC26              - SN74LS09N
   IC29              - HD74LS145P
   IC30-IC34         - M74LS367AP
   IC35              - MB74LS74A
   IC37              - HD74LS373P
   IC38              - Toshiba TC74HCU04P
   IC39              - HD74LS08P
   IC40              - TA75559P
   Q1                - 10687.5
   Q2                - 3579545
   CN1               - Cassette connector
   CN2               - RF connector
   CN3               - Audio connector
   CN4               - Video connector
   CN5               - Expansion connector
   CN6               - Cartridge connector
   CN7               - Printer connector
   CN8               - Joystick 2 connector
   CN9               - Joystick 1 connector
   CN10              - Keyboard connector 1
   CN11              - Keyboard connector 2

  Extra pcb (video related?) components::
   Q - 4.433619
   S - 74LS04
   L - LVA510


Toshiba HX-10D - MSX1 - hx10d
Toshiba HX-10DP - MSX1 - hx10dp
Toshiba HX-10E - MSX1 - hx10e
Toshiba HX-10F - MSX1 - hx10f
Toshiba HX-10S - MSX1 - hx10s
Toshiba HX-10SA - MSX1 - hx10sa
Toshiba HX-20 - MSX1 - hx20
Toshiba HX-20E - MSX1 - hx20e
Toshiba HX-20I - MSX1 - hx20i
Toshiba HX-21 - MXS1 - -hx21
Toshiba HX-21F - MSX1 - hx21f
Toshiba HX-22 - MSX1 - hx22
Toshiba HX-22I - MSX1 - hx22i
Toshiba HX-23 - MSX2 - hx23
Toshiba HX-23F - MSX2 - hx23f
Toshiba HX-32 - MSX1 - hx32
Toshiba HX-33 - MSX2 - hx33
Toshiba HX-34 - MSX2 - hx34
Toshiba HX-34I - MSX2 - hx34i
Toshiba FS-TM1 - MSX2 - fstm1
Victor HC-5 - MSX1 - hc5
Victor HC-6 - MSX1 - hc6
*Victor HC-6AV - MSX1
Victor HC-7 - MSX1 - hc7
*Victor HC-7E - MSX1
*Victor HC-7GB - MSX1
*Victor HC-9S - MSX2
*Victor HC-80 - MSX2
Victor HC-90 - MSX2 - victhc90
Victor HC-95 - MSX2 - victhc95
Victor HC-95A - MSX2 - victhc95a
*Victor HC-95T - MSX2
*Victor HC-95V - MSX2
Yamaha CX5F-1 - MSX1 - cx5f1
Yamaha CX5F-2 - MSX1 - cx5f
Yamaha CX5MU - MSX1 - cx5mu
Yamaha CX5MII-128 - MSX1 - cx5m128
Yamaha CX5MII - MSX1 - cx5m2
Yamaha CX7M - MSX2 - cx7m
Yamaha CX7M/128 - MSX2 - cx7m128
Yamaha SX-100 - MSX1 - sx100
Yamaha YIS-303 - MSX1 - yis303
Yamaha YIS-503 - MSX1 - yis503
Yamaha YIS-503F - MSX1 - yis503f
Yamaha YIS-503II - MSX1 - yis503ii
Yamaha YIS-503IIR (Russian) - MSX1 - y503iir
Yamaha YIS-503IIR (Estonian) - MSX1 - y503iir2
Yamaha YIS-503M - MSX1 - yis503m
Yamaha YIS-503IIIR - MSX2 - y503iiir
Yamaha YIS-503IIIR Estonian - MSX2 - y503iiire
Yamaha YIS604 - MSX2 - yis60464
Yamaha YIS604-128 - MSX2 - yis604
Yamaha YIS805-128 - MSX2 - y805128
Yamaha YIS805-128R2 - MSX2 - y805128r2
Yamaha YIS805-128R2 Estonian - MSX2 - y805128r2e
Yamaha YIS805-256 - MSX2 - y805256
*Yamaha YIS805-256 2+ - MSX2+ - was this a real machine?
Yashica YC-64 - MSX1 - yc64
Yeno DPC-64 (same bios as Olympia PHC-2)
Yeno MX64 - MSX1 - mx64
=============

PCB Layouts missing


*/


#include "emu.h"

#include "bus/centronics/ctronics.h"
#include "bus/msx_slot/slot.h"
#include "bus/msx_slot/rom.h"
#include "bus/msx_slot/ram.h"
#include "bus/msx_slot/ax230.h"
#include "bus/msx_slot/bruc100.h"
#include "bus/msx_slot/bunsetsu.h"
#include "bus/msx_slot/cartridge.h"
#include "bus/msx_slot/disk.h"
#include "bus/msx_slot/fs4600.h"
#include "bus/msx_slot/fsa1fm.h"
#include "bus/msx_slot/msx_write.h"
#include "bus/msx_slot/music.h"
#include "bus/msx_slot/panasonic08.h"
#include "bus/msx_slot/ram_mm.h"
#include "bus/msx_slot/msx_rs232.h"
#include "bus/msx_slot/sony08.h"
#include "cpu/z80/r800.h"
#include "cpu/z80/z80.h"
#include "formats/dsk_dsk.h"
#include "formats/dmk_dsk.h"
#include "formats/fmsx_cas.h"
#include "formats/msx_dsk.h"
#include "hashfile.h"
#include "imagedev/cassette.h"
#include "imagedev/floppy.h"
#include "machine/i8255.h"
#include "machine/rp5c01.h"
#include "machine/buffer.h"
#include "machine/input_merger.h"
#include "machine/wd_fdc.h"
#include "msx_kanji12.h"
#include "msx_matsushita.h"
#include "msx_s1985.h"
#include "msx_switched.h"
#include "msx_systemflags.h"
#include "screen.h"
#include "softlist_dev.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "sound/ymopl.h"
#include "speaker.h"
#include "video/v9938.h"
#include "video/tms9928a.h"


#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"


namespace {

class msx_hw_def
{
public:
	msx_hw_def() {}
	bool has_cassette() const { return m_has_cassette; }
	bool has_printer_port() const { return m_has_printer_port; }
	bool has_cartslot() const { return m_has_cartslot; }
	bool has_fdc() const { return m_has_fdc; }
	msx_hw_def &has_cassette(bool has_cassette) { m_has_cassette = has_cassette; return *this;}
	msx_hw_def &has_printer_port(bool has_printer_port) { m_has_printer_port = has_printer_port; return *this; }
	msx_hw_def &has_cartslot(bool has_cartslot) { m_has_cartslot = has_cartslot; return *this; }
	msx_hw_def &has_fdc(bool has_fdc) { m_has_fdc = has_fdc; return *this; }

private:
	bool m_has_cassette = true;
	bool m_has_printer_port = true;
	bool m_has_cartslot = false;
	bool m_has_fdc = false;
};

class msx_state : public driver_device
{
public:
	msx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cassette(*this, "cassette")
		, m_ay8910(*this, "ay8910")
		, m_dac(*this, "dac")
		, m_ppi(*this, "ppi8255")
		, m_tms9928a(*this, "tms9928a")
		, m_cent_status_in(*this, "cent_status_in")
		, m_cent_ctrl_out(*this, "cent_ctrl_out")
		, m_cent_data_out(*this, "cent_data_out")
		, m_centronics(*this, "centronics")
		, m_speaker(*this, "speaker")
		, m_mainirq(*this, "mainirq")
		, m_screen(*this, "screen")
		, m_region_kanji(*this, "kanji")
		, m_io_joy(*this, "JOY%u", 0U)
		, m_io_dsw(*this, "DSW")
		, m_io_mouse(*this, "MOUSE%u", 0U)
		, m_io_key(*this, "KEY%u", 0U)
		, m_leds(*this, "led%u", 1U)
		, m_view_page0(*this, "view0")
		, m_view_page1(*this, "view1")
		, m_view_page2(*this, "view2")
		, m_view_page3(*this, "view3")
		, m_view_slot0_page0(*this, "view0_0")
		, m_view_slot0_page1(*this, "view0_1")
		, m_view_slot0_page2(*this, "view0_2")
		, m_view_slot0_page3(*this, "view0_3")
		, m_view_slot1_page0(*this, "view1_0")
		, m_view_slot1_page1(*this, "view1_1")
		, m_view_slot1_page2(*this, "view1_2")
		, m_view_slot1_page3(*this, "view1_3")
		, m_view_slot2_page0(*this, "view2_0")
		, m_view_slot2_page1(*this, "view2_1")
		, m_view_slot2_page2(*this, "view2_2")
		, m_view_slot2_page3(*this, "view2_3")
		, m_view_slot3_page0(*this, "view3_0")
		, m_view_slot3_page1(*this, "view3_1")
		, m_view_slot3_page2(*this, "view3_2")
		, m_view_slot3_page3(*this, "view3_3")
		, m_use_exp_views(false)
		, m_expanded(-1)
		, m_psg_b(0)
		, m_kanji_latch(0)
		, m_slot_expanded{false, false, false, false}
		, m_primary_slot(0)
		, m_secondary_slot{0, 0, 0, 0}
		, m_port_c_old(0)
		, m_keylatch(0)
	{
		m_mouse[0] = m_mouse[1] = 0;
		m_mouse_stat[0] = m_mouse_stat[1] = 0;
		m_cartslot[0] = nullptr;
		m_cartslot[1] = nullptr;
		m_generic_internal = nullptr;
		m_view[0] = &m_view_page0;
		m_view[1] = &m_view_page1;
		m_view[2] = &m_view_page2;
		m_view[3] = &m_view_page3;
		m_exp_view[0][0] = &m_view_slot0_page0;
		m_exp_view[0][1] = &m_view_slot0_page1;
		m_exp_view[0][2] = &m_view_slot0_page2;
		m_exp_view[0][3] = &m_view_slot0_page3;
		m_exp_view[1][0] = &m_view_slot1_page0;
		m_exp_view[1][1] = &m_view_slot1_page1;
		m_exp_view[1][2] = &m_view_slot1_page2;
		m_exp_view[1][3] = &m_view_slot1_page3;
		m_exp_view[2][0] = &m_view_slot2_page0;
		m_exp_view[2][1] = &m_view_slot2_page1;
		m_exp_view[2][2] = &m_view_slot2_page2;
		m_exp_view[2][3] = &m_view_slot2_page3;
		m_exp_view[3][0] = &m_view_slot3_page0;
		m_exp_view[3][1] = &m_view_slot3_page1;
		m_exp_view[3][2] = &m_view_slot3_page2;
		m_exp_view[3][3] = &m_view_slot3_page3;
	}

	void ax150(machine_config &config);
	void ax170(machine_config &config);
	void ax230(machine_config &config);
	void canonv8(machine_config &config);
	void canonv10(machine_config &config);
	void canonv20(machine_config &config);
	void canonv20e(machine_config &config);
	void canonv25(machine_config &config);
	void cf1200(machine_config &config);
	void cf2000(machine_config &config);
	void cf2700(machine_config &config);
	void cf2700g(machine_config &config);
	void cf2700uk(machine_config &config);
	void cf3000(machine_config &config);
	void cf3300(machine_config &config);
	void cpc50a(machine_config &config);
	void cpc50b(machine_config &config);
	void cpc51(machine_config &config);
	void cpc88(machine_config &config);
	void cx5f(machine_config &config);
	void cx5f1(machine_config &config);
	void cx5mu(machine_config &config);
	void dgnmsx(machine_config &config);
	void dpc100(machine_config &config);
	void dpc180(machine_config &config);
	void dpc200(machine_config &config);
	void dpc200e(machine_config &config);
	void expert10(machine_config &config);
	void expert11(machine_config &config);
	void expert13(machine_config &config);
	void expertdp(machine_config &config);
	void expertpl(machine_config &config);
	void fmx(machine_config &config);
	void fdpc200(machine_config &config);
	void fpc500(machine_config &config);
	void fs1300(machine_config &config);
	void fs4000(machine_config &config);
	void fs4000a(machine_config &config);
	void fspc800(machine_config &config);
	void gfc1080(machine_config &config);
	void gfc1080a(machine_config &config);
	void gsfc80u(machine_config &config);
	void gsfc200(machine_config &config);
	void hb10(machine_config &config);
	void hb10p(machine_config &config);
	void hb20p(machine_config &config);
	void hb55(machine_config &config);
	void hb55d(machine_config &config);
	void hb55p(machine_config &config);
	void hb75(machine_config &config);
	void hb75d(machine_config &config);
	void hb75p(machine_config &config);
	void hb101(machine_config &config);
	void hb101p(machine_config &config);
	void hb201(machine_config &config);
	void hb201p(machine_config &config);
	void hb501p(machine_config &config);
	void hb701fd(machine_config &config);
	void hb8000(machine_config &config);
	void hc5(machine_config &config);
	void hc6(machine_config &config);
	void hc7(machine_config &config);
	void hotbi13b(machine_config &config);
	void hotbi13p(machine_config &config);
	void hx10(machine_config &config);
	void hx10d(machine_config &config);
	void hx10dp(machine_config &config);
	void hx10e(machine_config &config);
	void hx10f(machine_config &config);
	void hx10s(machine_config &config);
	void hx10sa(machine_config &config);
	void hx20(machine_config &config);
	void hx20e(machine_config &config);
	void hx20i(machine_config &config);
	void hx21(machine_config &config);
	void hx21f(machine_config &config);
	void hx22(machine_config &config);
	void hx22i(machine_config &config);
	void hx32(machine_config &config);
	void hx51i(machine_config &config);
	void jvchc7gb(machine_config &config);
	void mbh1(machine_config &config);
	void mbh1e(machine_config &config);
	void mbh2(machine_config &config);
	void mbh25(machine_config &config);
	void mbh50(machine_config &config);
	void ml8000(machine_config &config);
	void mlf48(machine_config &config);
	void mlf80(machine_config &config);
	void mlf110(machine_config &config);
	void mlf120(machine_config &config);
	void mlfx1(machine_config &config);
	void mpc10(machine_config &config);
	void mpc64(machine_config &config);
	void mpc100(machine_config &config);
	void mpc200(machine_config &config);
	void mpc200sp(machine_config &config);
	void mx10(machine_config &config);
	void mx15(machine_config &config);
	void mx64(machine_config &config);
	void mx101(machine_config &config);
	void nms801(machine_config &config);
	void perfect1(machine_config &config);
	void phc2(machine_config &config);
	void phc28(machine_config &config);
	void phc28l(machine_config &config);
	void phc28s(machine_config &config);
	void piopx7(machine_config &config);
	void piopx7uk(machine_config &config);
	void piopxv60(machine_config &config);
	void pv7(machine_config &config);
	void pv16(machine_config &config);
	void spc800(machine_config &config);
	void svi728(machine_config &config);
	void sx100(machine_config &config);
	void tadpc200(machine_config &config);
	void vg8000(machine_config &config);
	void vg8010(machine_config &config);
	void vg8010f(machine_config &config);
	void vg802000(machine_config &config);
	void vg802020(machine_config &config);
	void vg8020f(machine_config &config);
	void yc64(machine_config &config);
	void yis303(machine_config &config);
	void yis503(machine_config &config);
	void yis503f(machine_config &config);

protected:
	template<typename AY8910Type> void msx_base(AY8910Type &ay8910_type, machine_config &config, XTAL xtal, int cpu_divider);
	template<typename AY8910Type, typename T, typename Ret, typename... Params> void msx_base(AY8910Type &ay8910_type, machine_config &config, XTAL xtal, int cpu_divider, Ret (T::*func)(Params...));
	template<typename VDPType, typename AY8910Type> void msx1(VDPType &vdp_type, AY8910Type &ay8910_type, machine_config &config);
	template<typename VDPType, typename AY8910Type, typename T, typename Ret, typename... Params> void msx1(VDPType &vdp_type, AY8910Type &ay8910_type, machine_config &config, Ret (T::*func)(Params...));

	void msx_fd1793_force_ready(machine_config &config);
	void msx_fd1793(machine_config &config);
	void msx_wd2793_force_ready(machine_config &config);
	void msx_wd2793(machine_config &config);
	void msx_mb8877a(machine_config &config);
	void msx_microsol(machine_config &config);
	void msx_1_35_ssdd_drive(machine_config &config);
	void msx_1_35_dd_drive(machine_config &config);
	void msx_2_35_dd_drive(machine_config &config);
	template <u8 Game_port>
	u8 game_port_r();

	// configuration helpers
	template <typename T, typename U>
	auto &add_base_slot(machine_config &config, T &&type, U &&tag, u8 prim, bool expanded, u8 sec, u8 page, u8 numpages)
	{
		auto &device(std::forward<T>(type)(config, std::forward<U>(tag), 0U));
		device.set_memory_space(m_maincpu, AS_PROGRAM);
		device.set_io_space(m_maincpu, AS_IO);
		device.set_maincpu(m_maincpu);
		m_internal_slots.push_back(std::make_tuple(prim, expanded, sec, page, numpages, &device));
		return device;
	}
	template <typename T, typename U>
	auto &add_internal_slot(machine_config &config, T &&type, U &&tag, u8 prim, bool expanded, u8 sec, u8 page, u8 numpages)
	{
		auto &device = add_base_slot(config, type, tag, prim, expanded, sec, page, numpages);
		device.set_start_address(page * 0x4000);
		device.set_size(numpages * 0x4000);
		return device;
	}
	template <typename T, typename U>
	auto &add_internal_slot(machine_config &config, T &&type, U &&tag, u8 prim, u8 sec, u8 page, u8 numpages)
	{
		return add_internal_slot(config, type, tag, prim, true, sec, page, numpages);
	}
	template <typename T, typename U>
	auto &add_internal_slot(machine_config &config, T &&type, U &&tag, u8 prim, u8 page, u8 numpages)
	{
		return add_internal_slot(config, type, tag, prim, false, 0, page, numpages);
	}
	template <typename T, typename U>
	auto &add_internal_slot(machine_config &config, T &&type, U &&tag, u8 prim, bool expanded, u8 sec, u8 page, u8 numpages, const char *region, u32 offset = 0)
	{
		auto &device = add_internal_slot(config, type, tag, prim, expanded, sec, page, numpages);
		device.set_rom_start(region, offset);
		return device;
	}
	template <typename T, typename U>
	auto &add_internal_slot(machine_config &config, T &&type, U &&tag, u8 prim, u8 sec, u8 page, u8 numpages, const char *region, u32 offset = 0)
	{
		return add_internal_slot(config, type, tag, prim, true, sec, page, numpages, region, offset);
	}
	template <typename T, typename U>
	auto &add_internal_slot(machine_config &config, T &&type, U &&tag, u8 prim, u8 page, u8 numpages, const char *region, u32 offset = 0)
	{
		return add_internal_slot(config, type, tag, prim, false, 0, page, numpages, region, offset);
	}
	template <typename T, typename U>
	auto &add_internal_disk(machine_config &config, T &&type, U &&tag, u8 prim, bool expanded, u8 sec, u8 page, u8 numpages, const char *region, u32 offset = 0)
	{
		auto &device = add_internal_slot(config, type, tag, prim, true, sec, page, numpages, region, offset);
		m_hw_def.has_fdc(true);
		return device;
	}
	template <typename T, typename U>
	auto &add_internal_disk(machine_config &config, T &&type, U &&tag, u8 prim, u8 sec, u8 page, u8 numpages, const char *region, u32 offset = 0)
	{
		return add_internal_disk(config, type, tag, prim, true, sec, page, numpages, region, offset);
	}
	template <typename T, typename U>
	auto &add_internal_disk(machine_config &config, T &&type, U &&tag, u8 prim, u8 page, u8 numpages, const char *region, u32 offset = 0)
	{
		return add_internal_disk(config, type, tag, prim, false, 0, page, numpages, region, offset);
	}
	template <int N, typename T, typename U>
	auto &add_internal_slot_irq(machine_config &config, T &&type, U &&tag, u8 prim, bool expanded, u8 sec, u8 page, u8 numpages, const char *region, u32 offset = 0)
	{
		auto &device = add_internal_slot(config, type, tag, prim, expanded, sec, page, numpages, region, offset);
		device.irq_handler().set(m_mainirq, FUNC(input_merger_device::in_w<N>));
		return device;
	}
	template <int N, typename T, typename U>
	auto &add_internal_slot_irq(machine_config &config, T &&type, U &&tag, u8 prim, u8 sec, u8 page, u8 numpages, const char *region, u32 offset = 0)
	{
		return add_internal_slot_irq<N>(config, type, tag, prim, true, sec, page, numpages, region, offset);
	}
	template <int N, typename T, typename U>
	auto &add_internal_slot_irq(machine_config &config, T &&type, U &&tag, u8 prim, u8 page, u8 numpages, const char *region, u32 offset = 0)
	{
		return add_internal_slot_irq<N>(config, type, tag, prim, false, 0, page, numpages, region, offset);
	}
	template <int N, typename T, typename U>
	auto &add_internal_slot_irq_mirrored(machine_config &config, T &&type, U &&tag, u8 prim, u8 sec, u8 page, u8 numpages, const char *region, u32 offset = 0)
	{
		auto &device = add_internal_slot_irq<N>(config, type, tag, prim, sec, page, numpages, region, offset);
		device.set_size(0x4000);
		return device;
	}
	template <typename T, typename U>
	auto &add_internal_disk_mirrored(machine_config &config, T &&type, U &&tag, u8 prim, bool expanded, u8 sec, u8 page, u8 numpages, const char *region, u32 offset = 0)
	{
		// Memory mapped FDC registers are also accessible through page 2
		auto &device = add_internal_disk(config, type, tag, prim, expanded, sec, page, numpages, region, offset);
		device.set_size(0x4000);
		return device;
	}
	template <typename T, typename U>
	auto &add_internal_disk_mirrored(machine_config &config, T &&type, U &&tag, u8 prim, u8 sec, u8 page, u8 numpages, const char *region, u32 offset = 0)
	{
		return add_internal_disk_mirrored(config, type, tag, prim, true, sec, page, numpages, region, offset);
	}
	template <typename T, typename U>
	auto &add_internal_disk_mirrored(machine_config &config, T &&type, U &&tag, u8 prim, u8 page, u8 numpages, const char *region, u32 offset = 0)
	{
		return add_internal_disk_mirrored(config, type, tag, prim, false, 0, page, numpages, region, offset);
	}
	template <int N, typename T, typename U, typename V>
	auto &add_cartridge_slot(machine_config &config, T &&type, U &&tag, u8 prim, bool expanded, u8 sec, V &&intf, const char *deft)
	{
		auto &device = add_base_slot(config, type, tag, prim, expanded, sec, 0, 4);
		device.option_reset();
		intf(device);
		device.set_default_option(deft);
		device.set_fixed(false);
		device.irq_handler().set(m_mainirq, FUNC(input_merger_device::in_w<N>));
		m_hw_def.has_cartslot(true);
		return device;
	}
	template <int N, typename T, typename U, typename V>
	auto &add_cartridge_slot(machine_config &config, T &&type, U &&tag, u8 prim, u8 sec, V &&intf, const char *deft)
	{
		return add_cartridge_slot<N>(config, type, tag, prim, true, sec, intf, deft);
	}
	template <int N, typename T, typename U, typename V>
	auto &add_cartridge_slot(machine_config &config, T &&type, U &&tag, u8 prim, V &&intf, const char *deft)
	{
		return add_cartridge_slot<N>(config, type, tag, prim, false, 0, intf, deft);
	}
	template <int N>
	auto &add_cartridge_slot(machine_config &config, u8 prim, u8 sec)
	{
		std::string tag = "cartslot";
		tag += std::to_string(N);
		return add_cartridge_slot<N>(config, MSX_SLOT_CARTRIDGE, tag.c_str(), prim, true, sec, msx_cart, nullptr);
	}
	template <int N>
	auto &add_cartridge_slot(machine_config &config, u8 prim)
	{
		std::string tag = "cartslot";
		tag += std::to_string(N);
		return add_cartridge_slot<N>(config, MSX_SLOT_CARTRIDGE, tag.c_str(), prim, false, 0, msx_cart, nullptr);
	}
	virtual void driver_start() override;
	virtual void machine_start() override;
	virtual void machine_reset() override;

	void expanded_slot_w(offs_t offset, u8 data);
	u8 expanded_slot_r(offs_t offset);
	u8 kanji_r(offs_t offset);
	void kanji_w(offs_t offset, u8 data);
	void ppi_port_a_w(u8 data);
	void ppi_port_c_w(u8 data);
	u8 ppi_port_b_r();
	u8 psg_port_a_r();
	u8 psg_port_b_r();
	void psg_port_a_w(u8 data);
	void psg_port_b_w(u8 data);

	void msx_io_map(address_map &map);
	void memory_map(address_map &map);
	void memory_expand_slot(int slot);
	memory_view::memory_view_entry *get_view(int page, int prim, int sec);

	required_device<z80_device> m_maincpu;
	optional_device<cassette_image_device> m_cassette;
	required_device<ay8910_device> m_ay8910;
	required_device<dac_bit_interface> m_dac;
	required_device<i8255_device> m_ppi;
	optional_device<tms9928a_device> m_tms9928a;
	optional_device<input_buffer_device> m_cent_status_in;
	optional_device<output_latch_device> m_cent_ctrl_out;
	optional_device<output_latch_device> m_cent_data_out;
	optional_device<centronics_device> m_centronics;
	required_device<speaker_device> m_speaker;
	required_device<input_merger_any_high_device> m_mainirq;
	required_device<screen_device> m_screen;
	optional_memory_region m_region_kanji;
	required_ioport_array<2> m_io_joy;
	required_ioport m_io_dsw;
	required_ioport_array<2> m_io_mouse;
	required_ioport_array<11> m_io_key;
	output_finder<2> m_leds;
	msx_hw_def m_hw_def;
	// This is here until more direct rom dumps from kanji font roms become available.
	bool m_kanji_fsa1fx = false;
	memory_view m_view_page0;
	memory_view m_view_page1;
	memory_view m_view_page2;
	memory_view m_view_page3;
	memory_view *m_view[4];
	// There must be a better way to do this
	memory_view m_view_slot0_page0;
	memory_view m_view_slot0_page1;
	memory_view m_view_slot0_page2;
	memory_view m_view_slot0_page3;
	memory_view m_view_slot1_page0;
	memory_view m_view_slot1_page1;
	memory_view m_view_slot1_page2;
	memory_view m_view_slot1_page3;
	memory_view m_view_slot2_page0;
	memory_view m_view_slot2_page1;
	memory_view m_view_slot2_page2;
	memory_view m_view_slot2_page3;
	memory_view m_view_slot3_page0;
	memory_view m_view_slot3_page1;
	memory_view m_view_slot3_page2;
	memory_view m_view_slot3_page3;
	memory_view *m_exp_view[4][4];
	bool m_use_exp_views;
	int8_t m_expanded;
	msx_slot_cartridge_device *m_cartslot[2];
	msx_internal_slot_interface *m_generic_internal;
	std::vector<std::tuple<int, bool, int, int, int, msx_internal_slot_interface *>> m_internal_slots;

private:
	static void floppy_formats(format_registration &fr);

	INTERRUPT_GEN_MEMBER(msx_interrupt);

	// PSG
	u8 m_psg_b = 0;
	// mouse
	u16 m_mouse[2]{};
	s8 m_mouse_stat[2]{};
	// kanji
	u32 m_kanji_latch = 0;
	// memory
	bool m_slot_expanded[4];
	u8 m_primary_slot = 0;
	u8 m_secondary_slot[4];
	u8 m_port_c_old = 0;
	u8 m_keylatch = 0;
};


class bruc100_state : public msx_state
{
public:
	bruc100_state(const machine_config &mconfig, device_type type, const char *tag)
		: msx_state(mconfig, type, tag)
		, m_bruc100_firm(*this, "firm")
	{
	}

	void bruc100(machine_config &config);
	void bruc100a(machine_config &config);

private:
	required_device<msx_slot_bruc100_device> m_bruc100_firm;

	void io_map(address_map &map);
	void port90_w(u8 data)
	{
		m_bruc100_firm->select_bank(BIT(data, 7));
		m_cent_ctrl_out->write(data);
	}
};

class msx1_v9938_state : public msx_state
{
public:
	msx1_v9938_state(const machine_config &mconfig, device_type type, const char *tag)
		: msx_state(mconfig, type, tag)
		, m_v9938(*this, "v9938")
	{
	}

	void ax200(machine_config &mconfig);
	void ax200m(machine_config &mconfig);
	void cx5m128(machine_config &config);
	void cx5miib(machine_config &config);
	void svi738(machine_config &config);
	void svi738ar(machine_config &config);
	void tadpc200a(machine_config &config);
	void y503iir(machine_config &config);
	void y503iir2(machine_config &config);
	void yis503ii(machine_config &config);

private:
	template<typename AY8910Type> void msx1_v9938(AY8910Type &ay8910_type, machine_config &config);
	template<typename AY8910Type, typename T, typename Ret, typename... Params> void msx1_v9938(AY8910Type &ay8910_type, machine_config &config, Ret (T::*func)(Params...));
	template<typename AY8910Type> void msx1_v9938_pal(AY8910Type &ay8910_type, machine_config &config);
	template<typename AY8910Type, typename T, typename Ret, typename... Params> void msx1_v9938_pal(AY8910Type &ay8910_type, machine_config &config, Ret (T::*func)(Params...));

	void io_map(address_map &map);

	optional_device<v9938_device> m_v9938;
};

class msx2_state : public msx_state
{
public:
	msx2_state(const machine_config &mconfig, device_type type, const char *tag)
		: msx_state(mconfig, type, tag)
		, m_v9938(*this, "v9938")
		, m_v9958(*this, "v9958")
		, m_rtc(*this, "rtc")
		, m_rtc_latch(0)
	{
	}

	void ax350(machine_config &config);
	void ax350ii(machine_config &config);
	void ax350iif(machine_config &config);
	void ax370(machine_config &config);
	void ax500(machine_config &config);
	void canonv25(machine_config &config);
	void canonv30(machine_config &config);
	void canonv30f(machine_config &config);
	void cpc300(machine_config &config);
	void cpc300e(machine_config &config);
	void cpc330k(machine_config &config);
	void cpc400(machine_config &config);
	void cpc400s(machine_config &config);
	void cpc61(machine_config &config);
	void cpg120(machine_config &config);
	void cx7128(machine_config &config);
	void cx7m128(machine_config &config);
	void expert20(machine_config &config);
	void expert3i(machine_config &config);
	void expert3t(machine_config &config);
	void expertac(machine_config &config);
	void expertdx(machine_config &config);
	void fpc900(machine_config &config);
	void kmc5000(machine_config &config);
	void mbh70(machine_config &config);
	void mlg1(machine_config &config);
	void mlg3(machine_config &config);
	void mlg10(machine_config &config);
	void mlg30(machine_config &config);
	void mlg30_2(machine_config &config);
	void mpc2300(machine_config &config);
	void mpc2500f(machine_config &config);
	void mpc25fd(machine_config &config);
	void mpc25fs(machine_config &config);
	void mpc27(machine_config &config);
	void fs4500(machine_config &config);
	void fs4600f(machine_config &config);
	void fs4700f(machine_config &config);
	void fs5000f2(machine_config &config);
	void fs5500f1(machine_config &config);
	void fs5500f2(machine_config &config);
	void fsa1(machine_config &config);
	void fsa1a(machine_config &config);
	void fsa1f(machine_config &config);
	void fsa1fm(machine_config &config);
	void fsa1fx(machine_config &config);
	void fsa1gt(machine_config &config);
	void fsa1st(machine_config &config);
	void fsa1mk2(machine_config &config);
	void fsa1wsx(machine_config &config);
	void fsa1wx(machine_config &config);
	void fsa1wxa(machine_config &config);
	void fstm1(machine_config &config);
	void hbf1(machine_config &config);
	void hbf1ii(machine_config &config);
	void hbf1xd(machine_config &config);
	void hbf1xdj(machine_config &config);
	void hbf1xv(machine_config &config);
	void hbf5(machine_config &config);
	void hbf500(machine_config &config);
	void hbf500_2(machine_config &config);
	void hbf500f(machine_config &config);
	void hbf500p(machine_config &config);
	void hbf700d(machine_config &config);
	void hbf700f(machine_config &config);
	void hbf700p(machine_config &config);
	void hbf700s(machine_config &config);
	void hbf900(machine_config &config);
	void hbf900a(machine_config &config);
	void hbf9p(machine_config &config);
	void hbf9pr(machine_config &config);
	void hbf9s(machine_config &config);
	void hbg900ap(machine_config &config);
	void hbg900p(machine_config &config);
	void hotbit20(machine_config &config);
	void hx23(machine_config &config);
	void hx23f(machine_config &config);
	void hx33(machine_config &config);
	void hx34(machine_config &config);
	void mbh3(machine_config &config);
	void nms8220(machine_config &config);
	void nms8245(machine_config &config);
	void nms8245f(machine_config &config);
	void nms8250(machine_config &config);
	void nms8255(machine_config &config);
	void nms8255f(machine_config &config);
	void nms8260(machine_config &config);
	void nms8280(machine_config &config);
	void nms8280f(machine_config &config);
	void nms8280g(machine_config &config);
	void phc23(machine_config &config);
	void phc23jb(machine_config &config);
	void phc35j(machine_config &config);
	void phc55fd2(machine_config &config);
	void phc70fd(machine_config &config);
	void phc70fd2(machine_config &config);
	void phc77(machine_config &config);
	void tpc310(machine_config &config);
	void tpp311(machine_config &config);
	void tps312(machine_config &config);
	void ucv102(machine_config &config);
	void vg8230(machine_config &config);
	void vg8235(machine_config &config);
	void vg8235f(machine_config &config);
	void vg8240(machine_config &config);
	void victhc80(machine_config &config);
	void victhc90(machine_config &config);
	void victhc95(machine_config &config);
	void victhc95a(machine_config &config);
	void y503iiir(machine_config &config);
	void y503iiire(machine_config &config);
	void yis604(machine_config &config);
	void y805128(machine_config &config);
	void y805128r2(machine_config &config);
	void y805128r2e(machine_config &config);
	void y805256(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	template<typename AY8910Type> void msx2_base(AY8910Type &ay8910_type, machine_config &config);
	template<typename AY8910Type> void msx2(AY8910Type &ay8910_type, machine_config &config);
	template<typename AY8910Type> void msx2_pal(AY8910Type &ay8910_type, machine_config &config);
	template<typename AY8910Type> void msx2plus_base(AY8910Type &ay8910_type, machine_config &config);
	template<typename AY8910Type> void msx2plus(AY8910Type &ay8910_type, machine_config &config);
	template<typename AY8910Type> void msx2plus_pal(AY8910Type &ay8910_type, machine_config &config);
	template<typename AY8910Type> void turbor(AY8910Type &ay8910_type, machine_config &config);

	void msx_ym2413(machine_config &config);
	void msx2_64kb_vram(machine_config &config);

	u8 rtc_reg_r();
	void rtc_reg_w(u8 data);
	void rtc_latch_w(u8 data);
	u8 switched_r(offs_t offset);
	void switched_w(offs_t offset, u8 data);
	DECLARE_WRITE_LINE_MEMBER(turbo_w);

	void msx2_io_map(address_map &map);
	void msx2plus_io_map(address_map &map);

	std::vector<msx_switched_interface *> m_switched;

	optional_device<v9938_device> m_v9938;
	optional_device<v9958_device> m_v9958;
	required_device<rp5c01_device> m_rtc;

	// rtc
	u8 m_rtc_latch = 0;
};


void msx_state::memory_expand_slot(int slot)
{
	if (slot < 0 || slot > 3)
	{
		fatalerror("Invalid slot %d to expand\n", slot);
	}
	if (m_slot_expanded[slot])
		return;

	m_view_page0[slot](0x0000, 0x3fff).view(*m_exp_view[slot][0]);
	m_view_page1[slot](0x4000, 0x7fff).view(*m_exp_view[slot][1]);
	m_view_page2[slot](0x8000, 0xbfff).view(*m_exp_view[slot][2]);
	m_view_page3[slot](0xc000, 0xffff).view(*m_exp_view[slot][3]);
	m_view_page3[slot](0xffff, 0xffff).rw(FUNC(msx_state::expanded_slot_r), FUNC(msx_state::expanded_slot_w));
	for (int i = 0; i < 4; i++)
	{
		(*m_exp_view[slot][0])[i];
		(*m_exp_view[slot][1])[i];
		(*m_exp_view[slot][2])[i];
		(*m_exp_view[slot][3])[i];
	}
	m_slot_expanded[slot] = true;
}


void msx_state::memory_map(address_map &map)
{
	map.unmap_value_high();

	map(0x0000, 0x3fff).view(m_view_page0);
	map(0x4000, 0x7fff).view(m_view_page1);
	map(0x8000, 0xbfff).view(m_view_page2);
	map(0xc000, 0xffff).view(m_view_page3);

	// setup defaults
	for (int i = 0; i < 4; i++)
	{
		m_view_page0[i];
		m_view_page1[i];
		m_view_page2[i];
		m_view_page3[i];
	}

	// Look for expanded slots
	for (const auto& tuple : m_internal_slots)
	{
		int prim, sec, page, numpages;
		bool is_expanded;
		msx_internal_slot_interface *internal_slot;
		std::tie(prim, is_expanded, sec, page, numpages, internal_slot) = tuple;
		if (is_expanded)
		{
			memory_expand_slot(prim);
		}
	}

	for (const auto& tuple : m_internal_slots)
	{
		int prim, sec, page, numpages;
		bool is_expanded;
		msx_internal_slot_interface *internal_slot;
		std::tie(prim, is_expanded, sec, page, numpages, internal_slot) = tuple;

		memory_view::memory_view_entry *view[4] = {nullptr, nullptr, nullptr, nullptr};
		for (int i = 0; i < numpages; i++)
		{
			view[page + i] = get_view(page + i, prim, sec);
		}
		internal_slot->install(view[0], view[1], view[2], view[3]);
	}
}

memory_view::memory_view_entry *msx_state::get_view(int page, int prim, int sec)
{
	switch (page)
	{
	case 0:
		return m_slot_expanded[prim] ? &(*m_exp_view[prim][0])[sec] : &m_view_page0[prim];
	case 1:
		return m_slot_expanded[prim] ? &(*m_exp_view[prim][1])[sec] : &m_view_page1[prim];
	case 2:
		return m_slot_expanded[prim] ? &(*m_exp_view[prim][2])[sec] : &m_view_page2[prim];
	case 3:
		return m_slot_expanded[prim] ? &(*m_exp_view[prim][3])[sec] : &m_view_page3[prim];
	}
	return nullptr;
}

void msx_state::msx_io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	// 0x7c - 0x7d : MSX-MUSIC/FM-PAC write port. Handlers will be installed if MSX-MUSIC is present in a system
	if (m_hw_def.has_printer_port())
	{
		map(0x90, 0x90).r(m_cent_status_in, FUNC(input_buffer_device::read));
		map(0x90, 0x90).w(m_cent_ctrl_out, FUNC(output_latch_device::write));
		map(0x91, 0x91).w(m_cent_data_out, FUNC(output_latch_device::write));
	}
	map(0xa0, 0xa7).rw(m_ay8910, FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_data_w));
	map(0xa8, 0xab).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x98, 0x99).rw(m_tms9928a, FUNC(tms9928a_device::read), FUNC(tms9928a_device::write));
	map(0xd8, 0xd9).w(FUNC(msx_state::kanji_w));
	map(0xd9, 0xd9).r(FUNC(msx_state::kanji_r));
	// 0xfc - 0xff : Memory mapper I/O ports. I/O handlers will be installed if a memory mapper is present in a system
}


void bruc100_state::io_map(address_map &map)
{
	msx_io_map(map);
	map(0x90, 0x90).w(FUNC(bruc100_state::port90_w));
}

void msx1_v9938_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	// 0x7c - 0x7d : MSX-MUSIC/FM-PAC write port. Handlers will be installed if MSX-MUSIC is present in a system
	if (m_hw_def.has_printer_port())
	{
		map(0x90, 0x90).r(m_cent_status_in, FUNC(input_buffer_device::read));
		map(0x90, 0x90).w(m_cent_ctrl_out, FUNC(output_latch_device::write));
		map(0x91, 0x91).w(m_cent_data_out, FUNC(output_latch_device::write));
	}
	map(0xa0, 0xa7).rw(m_ay8910, FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_data_w));
	map(0xa8, 0xab).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x98, 0x9b).rw(m_v9938, FUNC(v9938_device::read), FUNC(v9938_device::write));
	map(0xd8, 0xd9).w(FUNC(msx1_v9938_state::kanji_w));
	map(0xd9, 0xd9).r(FUNC(msx1_v9938_state::kanji_r));
	// 0xfc - 0xff : Memory mapper I/O ports. I/O handlers will be installed if a memory mapper is present in a system
}

void msx2_state::msx2_io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x40, 0x4f).rw(FUNC(msx2_state::switched_r), FUNC(msx2_state::switched_w));
	// 0x7c - 0x7d : MSX-MUSIC/FM-PAC write port. Handlers will be installed if MSX-MUSIC is present in a system
	if (m_hw_def.has_printer_port())
	{
		map(0x90, 0x90).r(m_cent_status_in, FUNC(input_buffer_device::read));
		map(0x90, 0x90).w(m_cent_ctrl_out, FUNC(output_latch_device::write));
		map(0x91, 0x91).w(m_cent_data_out, FUNC(output_latch_device::write));
	}
	map(0xa0, 0xa7).rw(m_ay8910, FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_data_w));
	// TODO: S-3527 mirrors ac-af
	map(0xa8, 0xab).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
	// TODO: S-1985 mirrors 9c-9f
	map(0x98, 0x9b).rw(m_v9938, FUNC(v9938_device::read), FUNC(v9938_device::write));
	map(0xb4, 0xb4).w(FUNC(msx2_state::rtc_latch_w));
	map(0xb5, 0xb5).rw(FUNC(msx2_state::rtc_reg_r), FUNC(msx2_state::rtc_reg_w));
//	// Sanyo optical pen interface (not emulated)
//	map(0xb8, 0xbb).noprw();
	map(0xd8, 0xd9).w(FUNC(msx2_state::kanji_w));
	map(0xd9, 0xd9).r(FUNC(msx2_state::kanji_r));
	// 0xfc - 0xff : Memory mapper I/O ports. I/O handlers will be installed if a memory mapper is present in a system
}


void msx2_state::msx2plus_io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x40, 0x4f).rw(FUNC(msx2_state::switched_r), FUNC(msx2_state::switched_w));
	// 0x7c - 0x7d : MSX-MUSIC/FM-PAC write port. Handlers will be installed if MSX-MUSIC is present in a system
	if (m_hw_def.has_printer_port())
	{
		map(0x90, 0x90).r(m_cent_status_in, FUNC(input_buffer_device::read));
		map(0x90, 0x90).w(m_cent_ctrl_out, FUNC(output_latch_device::write));
		map(0x91, 0x91).w(m_cent_data_out, FUNC(output_latch_device::write));
	}
	map(0xa0, 0xa7).rw(m_ay8910, FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_data_w));
	map(0xa8, 0xab).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x98, 0x9b).rw(m_v9958, FUNC(v9958_device::read), FUNC(v9958_device::write));
	map(0xb4, 0xb4).w(FUNC(msx2_state::rtc_latch_w));
	map(0xb5, 0xb5).rw(FUNC(msx2_state::rtc_reg_r), FUNC(msx2_state::rtc_reg_w));
	map(0xd8, 0xd9).w(FUNC(msx2_state::kanji_w));
	map(0xd9, 0xd9).r(FUNC(msx2_state::kanji_r));
	// 0xfc - 0xff : Memory mapper I/O ports. I/O handlers will be installed if a memory mapper is present in a system
}


void msx_state::machine_reset()
{
	m_primary_slot = 0;
	m_secondary_slot[0] = 0;
	m_view_page0.select(0);
	m_view_page1.select(0);
	m_view_page2.select(0);
	m_view_page3.select(0);
	if (m_slot_expanded[0])
	{
		m_view_slot0_page0.select(0);
		m_view_slot0_page0.select(0);
		m_view_slot0_page0.select(0);
		m_view_slot0_page0.select(0);
	}
}


void msx_state::machine_start()
{
	m_leds.resolve();
	m_port_c_old = 0xff;
}


void msx2_state::machine_start()
{
	msx_state::machine_start();

	for (msx_switched_interface &switched : device_interface_enumerator<msx_switched_interface>(*this))
		m_switched.push_back(&switched);

	save_item(NAME(m_rtc_latch));
}

/* A hack to add 1 wait cycle in each opcode fetch.
   Possibly worth not to use custom table at all but adjust desired icount
   directly in m_opcodes.read_byte handler. */
static const u8 cc_op[0x100] = {
	4+1,10+1, 7+1, 6+1, 4+1, 4+1, 7+1, 4+1, 4+1,11+1, 7+1, 6+1, 4+1, 4+1, 7+1, 4+1,
	8+1,10+1, 7+1, 6+1, 4+1, 4+1, 7+1, 4+1,12+1,11+1, 7+1, 6+1, 4+1, 4+1, 7+1, 4+1,
	7+1,10+1,16+1, 6+1, 4+1, 4+1, 7+1, 4+1, 7+1,11+1,16+1, 6+1, 4+1, 4+1, 7+1, 4+1,
	7+1,10+1,13+1, 6+1,11+1,11+1,10+1, 4+1, 7+1,11+1,13+1, 6+1, 4+1, 4+1, 7+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1,
	7+1, 7+1, 7+1, 7+1, 7+1, 7+1, 4+1, 7+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1,
	5+1,10+1,10+1,10+1,10+1,11+1, 7+1,11+1, 5+1,10+1,10+1, 4+1,10+1,17+1, 7+1,11+1,
	5+1,10+1,10+1,11+1,10+1,11+1, 7+1,11+1, 5+1, 4+1,10+1,11+1,10+1, 4+1, 7+1,11+1,
	5+1,10+1,10+1,19+1,10+1,11+1, 7+1,11+1, 5+1, 4+1,10+1, 4+1,10+1, 4+1, 7+1,11+1,
	5+1,10+1,10+1, 4+1,10+1,11+1, 7+1,11+1, 5+1, 6+1,10+1, 4+1,10+1, 4+1, 7+1,11+1
};

static const u8 cc_cb[0x100] = {
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 8+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 8+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 8+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 8+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 8+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 8+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 8+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 8+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1
};

static const u8 cc_ed[0x100] = {
	 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,
	 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,
	 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,
	 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,
	 8+1, 8+1,15+2,16+1, 4+1,14+2, 4+1, 5+1, 8+1, 8+1,15+2,16+1, 4+1,14+2, 4+1, 5+1,
	 8+1, 8+1,15+2,16+1, 4+1,14+2, 4+1, 5+1, 8+1, 8+1,15+2,16+1, 4+1,14+2, 4+1, 5+1,
	 8+1, 8+1,15+2,16+1, 4+1,14+2, 4+1,14+1, 8+1, 8+1,15+2,16+1, 4+1,14+2, 4+1,14+1,
	 8+1, 8+1,15+2,16+1, 4+1,14+2, 4+1, 4+1, 8+1, 8+1,15+2,16+1, 4+1,14+2, 4+1, 4+1,
	 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,
	 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,
	12+1,12+1,12+1,12+1, 4+1, 4+1, 4+1, 4+1,12+1,12+1,12+1,12+1, 4+1, 4+1, 4+1, 4+1,
	12+1,12+1,12+1,12+1, 4+1, 4+1, 4+1, 4+1,12+1,12+1,12+1,12+1, 4+1, 4+1, 4+1, 4+1,
	 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,
	 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,
	 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,
	 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1
};

static const u8 cc_xy[0x100] = {
	 4+1,10+1, 7+1, 6+1, 4+1, 4+1, 7+1, 4+1, 4+1,11+1, 7+1, 6+1, 4+1, 4+1, 7+1, 4+1,
	 8+1,10+1, 7+1, 6+1, 4+1, 4+1, 7+1, 4+1,12+1,11+1, 7+1, 6+1, 4+1, 4+1, 7+1, 4+1,
	 7+1,10+1,16+1, 6+1, 4+1, 4+1, 7+1, 4+1, 7+1,11+1,16+1, 6+1, 4+1, 4+1, 7+1, 4+1,
	 7+1,10+1,13+1, 6+1,19+1,19+1,15+1, 4+1, 7+1,11+1,13+1, 6+1, 4+1, 4+1, 7+1, 4+1,
	 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,15+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,15+1, 4+1,
	 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,15+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,15+1, 4+1,
	 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,15+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,15+1, 4+1,
	15+1,15+1,15+1,15+1,15+1,15+1, 4+1,15+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,15+1, 4+1,
	 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,15+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,15+1, 4+1,
	 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,15+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,15+1, 4+1,
	 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,15+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,15+1, 4+1,
	 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,15+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,15+1, 4+1,
	 5+1,10+1,10+1,10+1,10+1,11+1, 7+1,11+1, 5+1,10+1,10+1, 7+1,10+1,17+1, 7+1,11+1,
	 5+1,10+1,10+1,11+1,10+1,11+1, 7+1,11+1, 5+1, 4+1,10+1,11+1,10+1, 4+1, 7+1,11+1,
	 5+1,10+1,10+1,19+1,10+1,11+1, 7+1,11+1, 5+1, 4+1,10+1, 4+1,10+1, 4+1, 7+1,11+1,
	 5+1,10+1,10+1, 4+1,10+1,11+1, 7+1,11+1, 5+1, 6+1,10+1, 4+1,10+1, 4+1, 7+1,11+1
};

/* extra cycles if jr/jp/call taken and 'interrupt latency' on rst 0-7 */
static const u8 cc_ex[0x100] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* DJNZ */
	5, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, /* JR NZ/JR Z */
	5, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, /* JR NC/JR C */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	5, 5, 5, 5, 0, 0, 0, 0, 5, 5, 5, 5, 0, 0, 0, 0, /* LDIR/CPIR/INIR/OTIR LDDR/CPDR/INDR/OTDR */
	6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2,
	6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2,
	6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2,
	6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2+1
};

void msx_state::driver_start()
{
	m_maincpu->set_input_line_vector(0, 0xff); // Z80

	m_maincpu->z80_set_cycle_tables(cc_op, cc_cb, cc_ed, cc_xy, nullptr, cc_ex);

	save_item(NAME(m_psg_b));
	save_item(NAME(m_mouse));
	save_item(NAME(m_mouse_stat));
	save_item(NAME(m_kanji_latch));
	save_item(NAME(m_kanji_fsa1fx));
	save_item(NAME(m_slot_expanded));
	save_item(NAME(m_primary_slot));
	save_item(NAME(m_secondary_slot));
	save_item(NAME(m_port_c_old));
	save_item(NAME(m_keylatch));
}

INTERRUPT_GEN_MEMBER(msx_state::msx_interrupt)
{
	m_mouse[0] = m_io_mouse[0]->read();
	m_mouse_stat[0] = -1;
	m_mouse[1] = m_io_mouse[1]->read();
	m_mouse_stat[1] = -1;
}

/*
** The I/O functions
*/


template <u8 Game_port>
u8 msx_state::game_port_r()
{
	u8 inp = m_io_joy[Game_port]->read();
	if (!(inp & 0x80))
	{
		// joystick
		return (inp & 0x7f);
	}
	else
	{
		// mouse
		u8 data = (inp & 0x70);
		if (m_mouse_stat[Game_port] < 0)
			data |= 0xf;
		else
			data |= ~(m_mouse[Game_port] >> (4 * m_mouse_stat[Game_port])) & 0x0f;
		return data;
	}
}

u8 msx_state::psg_port_a_r()
{
	u8 data = 0x80;
	if (m_cassette)
	{
		data = (m_cassette->input() > 0.0038 ? 0x80 : 0);
	}

	if ((m_psg_b ^ m_io_dsw->read()) & 0x40)
	{
		// game port 2
		data |= game_port_r<1>();
	}
	else
	{
		// game port 1
		data |= game_port_r<0>();
	}

	return data;
}

u8 msx_state::psg_port_b_r()
{
	return m_psg_b;
}

void msx_state::psg_port_a_w(u8 data)
{
}

void msx_state::psg_port_b_w(u8 data)
{
	// Arabic or kana mode led
	if ((data ^ m_psg_b) & 0x80)
		m_leds[1] = BIT(~data, 7);

	if ((m_psg_b ^ data) & 0x10)
	{
		if (++m_mouse_stat[0] > 3)
			m_mouse_stat[0] = -1;
	}
	if ((m_psg_b ^ data) & 0x20)
	{
		if (++m_mouse_stat[1] > 3)
			m_mouse_stat[1] = -1;
	}

	m_psg_b = data;
}


/*
** RTC functions
*/

void msx2_state::rtc_latch_w(u8 data)
{
	m_rtc_latch = data & 15;
}

void msx2_state::rtc_reg_w(u8 data)
{
	m_rtc->write(m_rtc_latch, data);
}

u8 msx2_state::rtc_reg_r()
{
	return m_rtc->read(m_rtc_latch);
}


/*
** The PPI functions
*/

void msx_state::ppi_port_a_w(u8 data)
{
	m_primary_slot = data;

	LOG("write to primary slot select: %02x\n", m_primary_slot);
	m_view_page0.select((data >> 0) & 0x03);
	m_view_page1.select((data >> 2) & 0x03);
	m_view_page2.select((data >> 4) & 0x03);
	m_view_page3.select((data >> 6) & 0x03);
}

void msx_state::ppi_port_c_w(u8 data)
{
	m_keylatch = data & 0x0f;

	// caps lock
	if (BIT(m_port_c_old ^ data, 6))
		m_leds[0] = BIT(~data, 6);

	// key click
	if (BIT(m_port_c_old ^ data, 7))
		m_dac->write(BIT(data, 7));

	// cassette motor on/off
	if (BIT(m_port_c_old ^ data, 4) && m_cassette)
		m_cassette->change_state(BIT(data, 4) ? CASSETTE_MOTOR_DISABLED : CASSETTE_MOTOR_ENABLED, CASSETTE_MASK_MOTOR);

	// cassette signal write
	if (BIT(m_port_c_old ^ data, 5) && m_cassette)
		m_cassette->output(BIT(data, 5) ? -1.0 : 1.0);

	m_port_c_old = data;
}

u8 msx_state::ppi_port_b_r()
{
	u8 result = 0xff;

	if (m_keylatch <= 10)
	{
		return m_io_key[m_keylatch]->read();
	}
	return result;
}

void msx_state::expanded_slot_w(offs_t offset, u8 data)
{
	const int slot = (m_primary_slot >> 6) & 0x03;
	m_secondary_slot[slot] = data;
	LOG("write to expanded slot select: %02x\n", m_secondary_slot[slot]);
	m_exp_view[slot][0]->select((data >> 0) & 0x03);
	m_exp_view[slot][1]->select((data >> 2) & 0x03);
	m_exp_view[slot][2]->select((data >> 4) & 0x03);
	m_exp_view[slot][3]->select((data >> 6) & 0x03);
}

u8 msx_state::expanded_slot_r(offs_t offset)
{
	const int slot = (m_primary_slot >> 6) & 0x03;
	return ~m_secondary_slot[slot];
}

u8 msx_state::kanji_r(offs_t offset)
{
	u8 result = 0xff;

	if (m_region_kanji)
	{
		u32 latch = m_kanji_fsa1fx ? bitswap<17>(m_kanji_latch, 4, 3, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 2, 1, 0) : m_kanji_latch;
		result = m_region_kanji->as_u8(latch);

		if (!machine().side_effects_disabled())
		{
			m_kanji_latch = (m_kanji_latch & ~0x1f) | ((m_kanji_latch + 1) & 0x1f);
		}
	}
	return result;
}

void msx_state::kanji_w(offs_t offset, u8 data)
{
	if (offset)
		m_kanji_latch = (m_kanji_latch & 0x007E0) | ((data & 0x3f) << 11);
	else
		m_kanji_latch = (m_kanji_latch & 0x1f800) | ((data & 0x3f) << 5);
}

u8 msx2_state::switched_r(offs_t offset)
{
	u8 data = 0xff;

	for (int i = 0; i < m_switched.size(); i++)
	{
		data &= m_switched[i]->switched_read(offset);
	}

	return data;
}

void msx2_state::switched_w(offs_t offset, u8 data)
{
	for (int i = 0; i < m_switched.size(); i++)
	{
		m_switched[i]->switched_write(offset, data);
	}
}


static INPUT_PORTS_START(msx_dips)
	PORT_START("JOY0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_DIPNAME(0x80, 0, "Game port 1")
	PORT_DIPSETTING(0x00, DEF_STR(Joystick))
	PORT_DIPSETTING(0x80, "Mouse")

	PORT_START("JOY1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1)        PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2)        PORT_PLAYER(2)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_DIPNAME(0x80, 0, "Game port 2")
	PORT_DIPSETTING(0x00, DEF_STR(Joystick))
	PORT_DIPSETTING(0x80, "Mouse")

	PORT_START("DSW")
	PORT_DIPNAME(0x40, 0, "Swap game port 1 and 2")
	PORT_DIPSETTING(0, DEF_STR(No))
	PORT_DIPSETTING(0x40, DEF_STR(Yes))

	PORT_START("MOUSE0")
	PORT_BIT(0xff00, 0x00, IPT_TRACKBALL_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(1)
	PORT_BIT(0x00ff, 0x00, IPT_TRACKBALL_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(1)

	PORT_START("MOUSE1")
	PORT_BIT(0xff00, 0x00, IPT_TRACKBALL_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(2)
	PORT_BIT(0x00ff, 0x00, IPT_TRACKBALL_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(2)
INPUT_PORTS_END


/* 2008-05 FP: About keyboards

Even if some later Philips (and maybe others) models started to use a layout similar to current
PC keyboards, common MSX keyboards have a couple of keys which do not fit usual mapping
- the key in the 1st row before 'Backspace', 3rd key from '0', here re-mapped to KEYCODE_BACKSLASH2
- the last key in the 4th row, 4th key from 'M' (not counting Shift), here re-mapped to KEYCODE_TILDE

These keys corresponds to the following symbols

    input_port  | msx   | msxuk | msxjp | msxkr |hotbit |expert |
    -------------------------------------------------------------
    BACKSLASH2  |  \ |  |  \ |  |  ? |  | won | |  \ ^  |  { }  |
    -------------------------------------------------------------
    TILDE       |  DK*  |  DK*  |  _    |  _    |  < >  |  / ?  |

* DK = "Dead Key"
Notice that 'expert' input_ports covers both versions 1.0 and 1.1.
msx2 input_ports have the same symbols as their msx counterparts.

TO DO:
- check Expert 1.0 layout with the real thing
- check Korean layout
- fix natural support in systems using msx inputs but with different mapping
(these systems could have different uses for keys mapped at the following
locations: COLON, QUOTE, BACKSLASH, OPENBRACE, CLOSEBRACE, BACKSLASH2, TILDE.
The corresponding symbols would not work properly in -natural mode).

Additional note about natural keyboard support: currently,
- "Graph" is mapped to 'F6' (this key could be labeled "L Graph")
- "Code" is mapped to 'F7' (this key could be labeled "R Graph", "Kana" or "Hangul")
- "Stop" is mapped to 'F8'
- "Select" is mapped to 'F9'
*/

#define KEYB_ROW0   \
	PORT_START("KEY0") \
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')') \
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!') \
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@') \
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#') \
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$') \
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%') \
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^') \
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')

#define KEYB_JP_ROW0   \
	PORT_START("KEY0") \
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0')                 \
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')  \
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')  \
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')  \
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')  \
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')  \
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')  \
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')

#define KEYB_FR_ROW0   \
	PORT_START("KEY0") \
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR(0xe0) PORT_CHAR('0') /* U+00E0, a with grave */ \
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('&')  PORT_CHAR('1') \
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR(0xe9) PORT_CHAR('2') /* U+00E9, e with acute */ \
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('"')  PORT_CHAR('3') \
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('\'') PORT_CHAR('4') \
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('(')  PORT_CHAR('5') \
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR(0xa7) PORT_CHAR('6') /* U+00A7, section sign */ \
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR(0xe8) PORT_CHAR('7') /* U+00E8, e with grave */

#define KEYB_DE_ROW0   \
	PORT_START("KEY0") \
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('=') \
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!') \
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"') \
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR(0xa7) /* U+00A7, section sign */ \
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$') \
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%') \
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&') \
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('/')

#define KEYB_SC_ROW0 \
	PORT_START("KEY0") \
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('=') \
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!') \
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"') \
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#') \
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$') \
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%') \
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&') \
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('/')

#define KEYB_RU_ROW0 \
	PORT_START("KEY0") \
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR(')') PORT_CHAR('9') \
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('+') PORT_CHAR(';') \
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('!') PORT_CHAR('1') \
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('"') PORT_CHAR('2') \
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('#') PORT_CHAR('3') \
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) /*PORT_CHAR('?')*/ PORT_CHAR('4') /* No idea what character that is, ring with 4 spikes in the corners */ \
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('%') PORT_CHAR('5') \
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('&') PORT_CHAR('6')

#define KEYB_ROW1   \
	PORT_START("KEY1") \
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('8')  PORT_CHAR('*') \
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)          PORT_CHAR('9')  PORT_CHAR('(') \
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-')  PORT_CHAR('_') \
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('=')  PORT_CHAR('+') \
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('\\') PORT_CHAR('|') \
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[')  PORT_CHAR('{') \
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']')  PORT_CHAR('}') \
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';')  PORT_CHAR(':')

#define KEYB_JP_ROW1   \
	PORT_START("KEY1") \
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('8')  PORT_CHAR('(') \
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)          PORT_CHAR('9')  PORT_CHAR(')') \
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-')  PORT_CHAR('=') \
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('^')  PORT_CHAR('~') \
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR(0xa5) PORT_CHAR('|') /* U+00A5 - Yen */ \
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('@')  PORT_CHAR('`') \
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('[')  PORT_CHAR('{') \
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';')  PORT_CHAR('+')

#define KEYB_KR_ROW1   \
	PORT_START("KEY1") \
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('8')    PORT_CHAR('(') \
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)          PORT_CHAR('9')    PORT_CHAR(')') \
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-')    PORT_CHAR('=') \
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('^')    PORT_CHAR('~') \
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR(0xffe6) PORT_CHAR('|') /* U+FFE6 Won sign */ \
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('@')    PORT_CHAR('`') \
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('[')    PORT_CHAR('{') \
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';')    PORT_CHAR('+')

#define KEYB_FR_ROW1   \
	PORT_START("KEY1") \
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('!')  PORT_CHAR('8') \
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)          PORT_CHAR(0xe7) PORT_CHAR('9') /* U+00E7, c with cedilla */ \
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR(')')  PORT_CHAR(0xb0) /* U+00B0, degree */ \
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('-')  PORT_CHAR('_') \
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('#')  PORT_CHAR(0xa3) /* U+00A3 - GBP sign */ \
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_NAME("Dead key") \
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('$')  PORT_CHAR('*') \
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR('m')  PORT_CHAR('M')

#define KEYB_DE_ROW1   \
	PORT_START("KEY1") \
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('8')  PORT_CHAR('(') \
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)          PORT_CHAR('9')  PORT_CHAR(')') \
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-')  PORT_CHAR(0xdf) /* U+00DF, sharp s */ \
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_NAME("Dead key") \
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('<')  PORT_CHAR('>') \
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR(0xfc) PORT_CHAR(0xdc) /* U+00FC, u with diaeresis, and U+00DC, U with diaeresis */\
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('+')  PORT_CHAR('*') \
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(0xf6) PORT_CHAR(0xd6) /* U+00F6, o with diaeresis, and U+00D6, O with diaeresis */

#define KEYB_SP_ROW1   \
	PORT_START("KEY1") \
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('8')  PORT_CHAR('*') \
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)          PORT_CHAR('9')  PORT_CHAR('(') \
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-')  PORT_CHAR('_') \
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('=')  PORT_CHAR('+') \
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('\\') PORT_CHAR('|') \
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[')  PORT_CHAR('{') \
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']')  PORT_CHAR('}') \
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(0xf1) PORT_CHAR(0xd1) /* U+00F1 and U+00D1 */

#define KEYB_RU_ROW1 \
	PORT_START("KEY1") \
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('\'') PORT_CHAR('7') \
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)          PORT_CHAR('(')  PORT_CHAR('8') \
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('$')  PORT_CHAR('0') \
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('=')  PORT_CHAR('_') \
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('-')  PORT_CHAR('^') \
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('h')  PORT_CHAR('H') \
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('*')  PORT_CHAR(':') \
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR('v')  PORT_CHAR('V')

#define KEYB_ROW2   \
	PORT_START("KEY2") \
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)     PORT_CHAR('\'')          PORT_CHAR('"') \
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('`')           PORT_CHAR('~') \
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)     PORT_CHAR(',')           PORT_CHAR('<') \
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)      PORT_CHAR('.')           PORT_CHAR('>') \
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)     PORT_CHAR('/')           PORT_CHAR('?') \
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Dead Key")        PORT_CODE(KEYCODE_TILDE)                \
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)         PORT_CHAR('a')           PORT_CHAR('A') \
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)         PORT_CHAR('b')           PORT_CHAR('B')

#define KEYB_JP_ROW2   \
	PORT_START("KEY2") \
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                PORT_CODE(KEYCODE_QUOTE)     PORT_CHAR(':') PORT_CHAR('*') \
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR(']') PORT_CHAR('}') \
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                PORT_CODE(KEYCODE_COMMA)     PORT_CHAR(',') PORT_CHAR('<') \
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                PORT_CODE(KEYCODE_STOP)      PORT_CHAR('.') PORT_CHAR('>') \
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                PORT_CODE(KEYCODE_SLASH)     PORT_CHAR('/') PORT_CHAR('?') \
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("_") PORT_CODE(KEYCODE_TILDE)     PORT_CHAR('_')                \
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                PORT_CODE(KEYCODE_A)         PORT_CHAR('a') PORT_CHAR('A') \
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                PORT_CODE(KEYCODE_B)         PORT_CHAR('b') PORT_CHAR('B')

#define KEYB_UK_ROW2   \
	PORT_START("KEY2") \
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                       PORT_CODE(KEYCODE_QUOTE)     PORT_CHAR('\'') PORT_CHAR('"') \
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                       PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR(0xa3) PORT_CHAR('~') /* U+00A3 - GBP sign */ \
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                       PORT_CODE(KEYCODE_COMMA)     PORT_CHAR(',')  PORT_CHAR('<') \
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                       PORT_CODE(KEYCODE_STOP)      PORT_CHAR('.')  PORT_CHAR('>') \
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                       PORT_CODE(KEYCODE_SLASH)     PORT_CHAR('/')  PORT_CHAR('?') \
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Dead Key") PORT_CODE(KEYCODE_TILDE) \
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                       PORT_CODE(KEYCODE_A)         PORT_CHAR('a')  PORT_CHAR('A') \
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                       PORT_CODE(KEYCODE_B)         PORT_CHAR('b')  PORT_CHAR('B')

#define KEYB_FR_ROW2   \
	PORT_START("KEY2") \
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)     PORT_CHAR(0xf9)          PORT_CHAR('%') /* U+00F9 u grave */ \
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('<')           PORT_CHAR('>') \
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)     PORT_CHAR(';')           PORT_CHAR('.') \
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)      PORT_CHAR(':')           PORT_CHAR('/') \
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)     PORT_CHAR('=')           PORT_CHAR('+') \
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Dead Key")        PORT_CODE(KEYCODE_TILDE)                \
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)         PORT_CHAR('q')           PORT_CHAR('Q') \
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)         PORT_CHAR('b')           PORT_CHAR('B')

#define KEYB_DE_ROW2   \
	PORT_START("KEY2") \
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)     PORT_CHAR(0xe4)          PORT_CHAR(0xc4) /* U+00E4, a with diaeresis, and U+00C4, A with diaeresis */ \
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('#')           PORT_CHAR('^') \
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)     PORT_CHAR(',')           PORT_CHAR(';') \
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)      PORT_CHAR('.')           PORT_CHAR(':') \
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)     PORT_CHAR('-')           PORT_CHAR('_') \
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Dead Key")        PORT_CODE(KEYCODE_TILDE)                \
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)         PORT_CHAR('a')           PORT_CHAR('A') \
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)         PORT_CHAR('b')           PORT_CHAR('B')

#define KEYB_SP_ROW2   \
	PORT_START("KEY2") \
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)     PORT_CHAR('\'')          PORT_CHAR('"') \
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR(';')           PORT_CHAR(':') \
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)     PORT_CHAR(',')           PORT_CHAR('<') \
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)      PORT_CHAR('.')           PORT_CHAR('>') \
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)     PORT_CHAR('/')           PORT_CHAR('?') \
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Dead Key")        PORT_CODE(KEYCODE_TILDE)                \
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)         PORT_CHAR('a')           PORT_CHAR('A') \
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)         PORT_CHAR('b')           PORT_CHAR('B')

#define KEYB_RU_ROW2 \
	PORT_START("KEY2") \
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)     PORT_CHAR('\\') \
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('>')  PORT_CHAR('.') \
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)     PORT_CHAR('b')  PORT_CHAR('B') \
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)      PORT_CHAR('@') \
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)     PORT_CHAR('<')  PORT_CHAR(',') \
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)     PORT_CHAR('?')  PORT_CHAR('/') \
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)         PORT_CHAR('f')  PORT_CHAR('F') \
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)         PORT_CHAR('i')  PORT_CHAR('I')

#define KEYB_ROW3   \
	PORT_START("KEY3") \
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C') \
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D') \
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E') \
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F') \
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G') \
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H') \
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I') \
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')

#define KEYB_RU_ROW3 \
	PORT_START("KEY3") \
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('s') PORT_CHAR('S') \
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('w') PORT_CHAR('W') \
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('u') PORT_CHAR('U') \
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('a') PORT_CHAR('A') \
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('p') PORT_CHAR('P') \
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('r') PORT_CHAR('R') \
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('[') PORT_CHAR('{') \
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('o') PORT_CHAR('O')

#define KEYB_ROW4   \
	PORT_START("KEY4") \
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K') \
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L') \
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M') \
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N') \
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O') \
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P') \
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q') \
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')

#define KEYB_FR_ROW4   \
	PORT_START("KEY4") \
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K') \
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L') \
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR(',') PORT_CHAR('?') \
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N') \
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O') \
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P') \
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('a') PORT_CHAR('A') \
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')

#define KEYB_RU_ROW4 \
	PORT_START("KEY4") \
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('l') PORT_CHAR('L') \
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('d') PORT_CHAR('D') \
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('x') PORT_CHAR('X') \
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('t') PORT_CHAR('T') \
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR(']') PORT_CHAR('}') \
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('z') PORT_CHAR('Z') \
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('j') PORT_CHAR('J') \
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('k') PORT_CHAR('K')

#define KEYB_ROW5   \
	PORT_START("KEY5") \
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S') \
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T') \
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U') \
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V') \
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W') \
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X') \
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y') \
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')

#define KEYB_DE_ROW5   \
	PORT_START("KEY5") \
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S') \
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T') \
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U') \
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V') \
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W') \
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X') \
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('z') PORT_CHAR('Z') \
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('y') PORT_CHAR('Y')

#define KEYB_FR_ROW5   \
	PORT_START("KEY5") \
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S') \
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T') \
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U') \
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V') \
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('z') PORT_CHAR('Z') \
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X') \
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y') \
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('w') PORT_CHAR('W')

#define KEYB_RU_ROW5 \
	PORT_START("KEY5") \
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('y') PORT_CHAR('Y') \
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('e') PORT_CHAR('E') \
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('g') PORT_CHAR('G') \
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('m') PORT_CHAR('M') \
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('c') PORT_CHAR('C') \
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('|') PORT_CHAR('~') \
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('n') PORT_CHAR('N') \
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('q') PORT_CHAR('Q')

#define KEYB_ROW6   \
	PORT_START("KEY6") \
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                     PORT_CODE(KEYCODE_LSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)           \
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                     PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)           \
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("GRAPH")  PORT_CODE(KEYCODE_PGUP)     PORT_CHAR(UCHAR_MAMEKEY(F6))       \
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CAPS")   PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) \
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CODE")   PORT_CODE(KEYCODE_PGDN)     PORT_CHAR(UCHAR_MAMEKEY(F7))       \
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1  F6") PORT_CODE(KEYCODE_F1)       PORT_CHAR(UCHAR_MAMEKEY(F1))       \
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2  F7") PORT_CODE(KEYCODE_F2)       PORT_CHAR(UCHAR_MAMEKEY(F2))       \
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3  F8") PORT_CODE(KEYCODE_F3)       PORT_CHAR(UCHAR_MAMEKEY(F3))

#define KEYB_JP_ROW6 \
	PORT_START("KEY6") \
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                     PORT_CODE(KEYCODE_LSHIFT)   PORT_CHAR(UCHAR_SHIFT_1) \
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                     PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2) \
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("GRAPH")  PORT_CODE(KEYCODE_PGUP)     PORT_CHAR(UCHAR_MAMEKEY(F6)) \
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CAPS")   PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) \
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KANA")   PORT_CODE(KEYCODE_PGDN)     PORT_CHAR(UCHAR_MAMEKEY(F7)) \
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1  F6") PORT_CODE(KEYCODE_F1)       PORT_CHAR(UCHAR_MAMEKEY(F1)) \
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2  F7") PORT_CODE(KEYCODE_F2)       PORT_CHAR(UCHAR_MAMEKEY(F2)) \
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3  F8") PORT_CODE(KEYCODE_F3)       PORT_CHAR(UCHAR_MAMEKEY(F3))

#define KEYB_KR_ROW6 \
	PORT_START("KEY6") \
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD)                     PORT_CODE(KEYCODE_LSHIFT)   PORT_CHAR(UCHAR_SHIFT_1) \
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD)                     PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2) \
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("GRAPH")  PORT_CODE(KEYCODE_PGUP)     PORT_CHAR(UCHAR_MAMEKEY(F6)) \
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CAPS")   PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) \
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Hangul") PORT_CODE(KEYCODE_PGDN)     PORT_CHAR(UCHAR_MAMEKEY(F7)) \
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1  F6") PORT_CODE(KEYCODE_F1)       PORT_CHAR(UCHAR_MAMEKEY(F1)) \
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2  F7") PORT_CODE(KEYCODE_F2)       PORT_CHAR(UCHAR_MAMEKEY(F2)) \
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3  F8") PORT_CODE(KEYCODE_F3)       PORT_CHAR(UCHAR_MAMEKEY(F3))

#define KEYB_EXPERT11_ROW6   \
	PORT_START("KEY6") \
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                        PORT_CODE(KEYCODE_LSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)           \
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CONTROL")   PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)           \
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L GRA")     PORT_CODE(KEYCODE_PGUP)     PORT_CHAR(UCHAR_MAMEKEY(F6))       \
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CAPS LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) \
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R GRA")     PORT_CODE(KEYCODE_PGDN)     PORT_CHAR(UCHAR_MAMEKEY(F7))       \
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1  F6")    PORT_CODE(KEYCODE_F1)       PORT_CHAR(UCHAR_MAMEKEY(F1))       \
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2  F7")    PORT_CODE(KEYCODE_F2)       PORT_CHAR(UCHAR_MAMEKEY(F2))       \
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3  F8")    PORT_CODE(KEYCODE_F3)       PORT_CHAR(UCHAR_MAMEKEY(F3))

#define KEYB_ROW7   \
	PORT_START("KEY7") \
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F4  F9")  PORT_CODE(KEYCODE_F4)        PORT_CHAR(UCHAR_MAMEKEY(F4))  \
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F5  F10") PORT_CODE(KEYCODE_F5)        PORT_CHAR(UCHAR_MAMEKEY(F5))  \
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                      PORT_CODE(KEYCODE_ESC)       PORT_CHAR(UCHAR_MAMEKEY(ESC)) \
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                      PORT_CODE(KEYCODE_TAB)       PORT_CHAR('\t')               \
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("STOP")    PORT_CODE(KEYCODE_RCONTROL)  PORT_CHAR(UCHAR_MAMEKEY(F8))  \
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                      PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)                  \
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SELECT")  PORT_CODE(KEYCODE_END)       PORT_CHAR(UCHAR_MAMEKEY(F9))  \
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                      PORT_CODE(KEYCODE_ENTER)     PORT_CHAR(13)

#define KEYB_ROW8   \
	PORT_START("KEY8") \
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                  PORT_CODE(KEYCODE_SPACE)  PORT_CHAR(' ')                   \
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                  PORT_CODE(KEYCODE_HOME)   PORT_CHAR(UCHAR_MAMEKEY(HOME))   \
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                  PORT_CODE(KEYCODE_INSERT) PORT_CHAR(UCHAR_MAMEKEY(INSERT)) \
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DEL") PORT_CODE(KEYCODE_DEL)    PORT_CHAR(UCHAR_MAMEKEY(DEL))    \
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                  PORT_CODE(KEYCODE_LEFT)   PORT_CHAR(UCHAR_MAMEKEY(LEFT))   \
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                  PORT_CODE(KEYCODE_UP)     PORT_CHAR(UCHAR_MAMEKEY(UP))     \
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                  PORT_CODE(KEYCODE_DOWN)   PORT_CHAR(UCHAR_MAMEKEY(DOWN))   \
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                  PORT_CODE(KEYCODE_RIGHT)  PORT_CHAR(UCHAR_MAMEKEY(RIGHT))

#define KEYB_ROW9   \
	PORT_START("KEY9") \
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK)  PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))  \
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD)  PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))  \
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD) PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD)) \
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)     PORT_CHAR(UCHAR_MAMEKEY(0_PAD))     \
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)     PORT_CHAR(UCHAR_MAMEKEY(1_PAD))     \
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)     PORT_CHAR(UCHAR_MAMEKEY(2_PAD))     \
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)     PORT_CHAR(UCHAR_MAMEKEY(3_PAD))     \
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)     PORT_CHAR(UCHAR_MAMEKEY(4_PAD))

/* 2008-05 FP: I guess these belong to the keypad */
#define KEYB_EXPERT11_ROW9  \
	PORT_START("KEY9") \
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD)  PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))  \
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD)) \
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK)  PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))  \
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD) PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD)) \
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

#define KEYB_ROW10  \
	PORT_START("KEY10") \
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)     PORT_CHAR(UCHAR_MAMEKEY(5_PAD))     \
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)     PORT_CHAR(UCHAR_MAMEKEY(6_PAD))     \
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)     PORT_CHAR(UCHAR_MAMEKEY(7_PAD))     \
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)     PORT_CHAR(UCHAR_MAMEKEY(8_PAD))     \
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)     PORT_CHAR(UCHAR_MAMEKEY(9_PAD))     \
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD)) \
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(UCHAR_MAMEKEY(COMMA_PAD)) \
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD)   PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))

static INPUT_PORTS_START(msx)
	KEYB_ROW0
	KEYB_ROW1
	KEYB_ROW2
	KEYB_ROW3
	KEYB_ROW4
	KEYB_ROW5
	KEYB_ROW6
	KEYB_ROW7
	KEYB_ROW8

	PORT_START("KEY9")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY10")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_INCLUDE(msx_dips)
INPUT_PORTS_END

static INPUT_PORTS_START(msxde)
	KEYB_DE_ROW0
	KEYB_DE_ROW1
	KEYB_DE_ROW2
	KEYB_ROW3
	KEYB_ROW4
	KEYB_DE_ROW5
	KEYB_ROW6
	KEYB_ROW7
	KEYB_ROW8

	PORT_START("KEY9")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY10")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_INCLUDE(msx_dips)
INPUT_PORTS_END

static INPUT_PORTS_START(msxfr)
	KEYB_FR_ROW0
	KEYB_FR_ROW1
	KEYB_FR_ROW2
	KEYB_ROW3
	KEYB_FR_ROW4
	KEYB_FR_ROW5
	KEYB_ROW6
	KEYB_ROW7
	KEYB_ROW8

	PORT_START("KEY9")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY10")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_INCLUDE(msx_dips)
INPUT_PORTS_END

static INPUT_PORTS_START(msxjp)
	KEYB_JP_ROW0
	KEYB_JP_ROW1
	KEYB_JP_ROW2
	KEYB_ROW3
	KEYB_ROW4
	KEYB_ROW5
	KEYB_JP_ROW6
	KEYB_ROW7
	KEYB_ROW8

	PORT_START("KEY9")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY10")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_INCLUDE(msx_dips)
INPUT_PORTS_END

static INPUT_PORTS_START(msxkr)
	KEYB_JP_ROW0
	KEYB_KR_ROW1
	KEYB_JP_ROW2
	KEYB_ROW3
	KEYB_ROW4
	KEYB_ROW5
	KEYB_KR_ROW6
	KEYB_ROW7
	KEYB_ROW8

	PORT_START("KEY9")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY10")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_INCLUDE(msx_dips)
INPUT_PORTS_END

static INPUT_PORTS_START(msxru)
	KEYB_RU_ROW0
	KEYB_RU_ROW1
	KEYB_RU_ROW2
	KEYB_RU_ROW3
	KEYB_RU_ROW4
	KEYB_RU_ROW5
	KEYB_ROW6
	KEYB_ROW7
	KEYB_ROW8

	PORT_START("KEY9")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY10")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_INCLUDE(msx_dips)
INPUT_PORTS_END

static INPUT_PORTS_START(msxsp)
	KEYB_ROW0
	KEYB_SP_ROW1
	KEYB_SP_ROW2
	KEYB_ROW3
	KEYB_ROW4
	KEYB_ROW5
	KEYB_ROW6
	KEYB_ROW7
	KEYB_ROW8

	PORT_START("KEY9")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY10")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_INCLUDE(msx_dips)
INPUT_PORTS_END

static INPUT_PORTS_START(msxuk)
	KEYB_ROW0
	KEYB_ROW1
	KEYB_UK_ROW2
	KEYB_ROW3
	KEYB_ROW4
	KEYB_ROW5
	KEYB_ROW6
	KEYB_ROW7
	KEYB_ROW8

	PORT_START("KEY9")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY10")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_INCLUDE(msx_dips)
INPUT_PORTS_END

static INPUT_PORTS_START(bruc100)
	KEYB_ROW0
	KEYB_ROW1
	KEYB_ROW2
	KEYB_ROW3
	KEYB_ROW4
	KEYB_ROW5
	KEYB_ROW6
	KEYB_ROW7

	KEYB_ROW8

	PORT_START("KEY9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD)  PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD)  PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK)  PORT_NAME("Keypad *")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD)  PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD)  PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CUT LINE/DEL CUT?") // DEL CUT / CUT LINE?
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\xe2\x86\x90WORD") // <-WORD
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\xe2\x86\x92WORD") // ->WORD

	PORT_START("KEY10")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("END LINE")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('$')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('"')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('?')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('>')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('<')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(')')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('(')

	PORT_INCLUDE(msx_dips)
INPUT_PORTS_END

static INPUT_PORTS_START(cf3000)
	KEYB_JP_ROW0
	KEYB_JP_ROW1
	KEYB_JP_ROW2
	KEYB_ROW3
	KEYB_ROW4
	KEYB_ROW5
	KEYB_JP_ROW6
	KEYB_ROW7
	KEYB_ROW8

	PORT_START("KEY9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)     PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD) PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)     PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)     PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)     PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)     PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)     PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)     PORT_CHAR(UCHAR_MAMEKEY(3_PAD))

	PORT_START("KEY10")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)      PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK)   PORT_NAME("Keypad *")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD)  PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD)   PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)      PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)      PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD)    PORT_NAME("Keypad .")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS_PAD) PORT_CHAR(UCHAR_MAMEKEY(EQUALS_PAD))

	PORT_INCLUDE(msx_dips)
INPUT_PORTS_END

static INPUT_PORTS_START(expert10)
	KEYB_ROW0
	KEYB_ROW1

	PORT_START("KEY2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)     PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR(0xc7) PORT_CHAR(0xe7)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)     PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)      PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)     PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Dead Key")        PORT_CODE(KEYCODE_TILDE)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)         PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)         PORT_CHAR('b') PORT_CHAR('B')

	KEYB_ROW3
	KEYB_ROW4
	KEYB_ROW5
	KEYB_EXPERT11_ROW6
	KEYB_ROW7
	KEYB_ROW8
	KEYB_EXPERT11_ROW9

	PORT_START("KEY10")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_INCLUDE(msx_dips)
INPUT_PORTS_END

static INPUT_PORTS_START(expert11)
	PORT_START("KEY0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')

	PORT_START("KEY1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('8')  PORT_CHAR('\'')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)          PORT_CHAR('9')  PORT_CHAR('(')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-')  PORT_CHAR('_')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('=')  PORT_CHAR('+')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('{')  PORT_CHAR('}')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('\'') PORT_CHAR('`')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('[')  PORT_CHAR(']')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR('~')  PORT_CHAR('^')

	PORT_START("KEY2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('*')  PORT_CHAR('@')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)     PORT_CHAR(0xc7) PORT_CHAR(0xe7)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)     PORT_CHAR(',')  PORT_CHAR('<')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)      PORT_CHAR('.')  PORT_CHAR('>')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)     PORT_CHAR('/')  PORT_CHAR('?')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)     PORT_CHAR(';')  PORT_CHAR(':')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)         PORT_CHAR('a')  PORT_CHAR('A')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)         PORT_CHAR('b')  PORT_CHAR('B')

	KEYB_ROW3
	KEYB_ROW4
	KEYB_ROW5
	KEYB_EXPERT11_ROW6
	KEYB_ROW7
	KEYB_ROW8
	KEYB_EXPERT11_ROW9

	PORT_START("KEY10")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_INCLUDE(msx_dips)
INPUT_PORTS_END

static INPUT_PORTS_START(fs4000)
	KEYB_JP_ROW0
	KEYB_JP_ROW1
	KEYB_JP_ROW2
	KEYB_ROW3
	KEYB_ROW4
	KEYB_ROW5
	KEYB_JP_ROW6
	KEYB_ROW7
	KEYB_ROW8

	PORT_START("KEY9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)  // PORT_CODE(KEYCODE_ASTERISK)  PORT_NAME("Keypad *") /* Mapped by the bios but there is actually no key for this */
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)  // PORT_CODE(KEYCODE_PLUS_PAD)  PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD)) /* Mapped by the bios but there is actually no key for this */
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)  // PORT_CODE(KEYCODE_SLASH_PAD) PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD)) /* Mapped by the bios but there is actually no key for this */
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)     PORT_CHAR(UCHAR_MAMEKEY(0_PAD)) //
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)     PORT_CHAR(UCHAR_MAMEKEY(1_PAD)) //
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)     PORT_CHAR(UCHAR_MAMEKEY(2_PAD)) //
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)     PORT_CHAR(UCHAR_MAMEKEY(3_PAD)) //
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)     PORT_CHAR(UCHAR_MAMEKEY(4_PAD)) //

	PORT_START("KEY10")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)     PORT_CHAR(UCHAR_MAMEKEY(5_PAD)) //
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)     PORT_CHAR(UCHAR_MAMEKEY(6_PAD)) //
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)     PORT_CHAR(UCHAR_MAMEKEY(7_PAD)) //
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)     PORT_CHAR(UCHAR_MAMEKEY(8_PAD)) //
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)     PORT_CHAR(UCHAR_MAMEKEY(9_PAD)) //
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD)) //
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA_PAD) PORT_CHAR(UCHAR_MAMEKEY(COMMA_PAD)) //
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD)   PORT_NAME("Keypad .") //

	PORT_INCLUDE(msx_dips)
INPUT_PORTS_END

static INPUT_PORTS_START(hotbit)
	PORT_START("KEY0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('"')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')

	PORT_START("KEY1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('8')  PORT_CHAR('*')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)          PORT_CHAR('9')  PORT_CHAR('(')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-')  PORT_CHAR('_')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('=')  PORT_CHAR('+')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('\\') PORT_CHAR('^')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('\'') PORT_CHAR('`')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('"')  PORT_CHAR('`')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(0xc7) PORT_CHAR(0xe7)

	PORT_START("KEY2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)     PORT_CHAR('~') PORT_CHAR('^')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('[') PORT_CHAR(']')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)     PORT_CHAR(',') PORT_CHAR(';')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)      PORT_CHAR('.') PORT_CHAR(':')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)     PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)     PORT_CHAR('<') PORT_CHAR('>')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)         PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)         PORT_CHAR('b') PORT_CHAR('B')

	KEYB_ROW3
	KEYB_ROW4
	KEYB_ROW5
	KEYB_ROW6
	KEYB_ROW7
	KEYB_ROW8

	PORT_START("KEY9")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY10")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_INCLUDE(msx_dips)
INPUT_PORTS_END

static INPUT_PORTS_START(mlfx1)
	KEYB_ROW0
	KEYB_ROW1
	KEYB_UK_ROW2
	KEYB_ROW3
	KEYB_ROW4
	KEYB_ROW5
	KEYB_ROW6
	KEYB_ROW7
	KEYB_ROW8

	PORT_START("KEY9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD)  PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD) PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK)  PORT_NAME("Keypad *")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)     PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)     PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)     PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)     PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)     PORT_CHAR(UCHAR_MAMEKEY(4_PAD))

	PORT_START("KEY10")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)     PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)     PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)     PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)     PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)     PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(UCHAR_MAMEKEY(COMMA_PAD))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD)   PORT_NAME("Keypad .")

	PORT_INCLUDE(msx_dips)
INPUT_PORTS_END

static INPUT_PORTS_START(svi728)
	KEYB_ROW0
	KEYB_ROW1
	KEYB_ROW2
	KEYB_ROW3
	KEYB_ROW4
	KEYB_ROW5
	KEYB_ROW6
	KEYB_ROW7
	KEYB_ROW8

	PORT_START("KEY9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD)  PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK)  PORT_NAME("Keypad *")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD) PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)   // ?? PORT_CODE(KEYCODE_1_PAD)     PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)   // ?? PORT_CODE(KEYCODE_2_PAD)     PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)   // ?? PORT_CODE(KEYCODE_3_PAD)     PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)   // ?? PORT_CODE(KEYCODE_4_PAD)     PORT_CHAR(UCHAR_MAMEKEY(4_PAD))

	PORT_START("KEY10")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)   // ?? PORT_CODE(KEYCODE_5_PAD)     PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)   // ?? PORT_CODE(KEYCODE_6_PAD)     PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)   // ?? PORT_CODE(KEYCODE_7_PAD)     PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)   // ?? PORT_CODE(KEYCODE_8_PAD)     PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)   // ?? PORT_CODE(EYCODE_9_PAD)      PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)   // ?? PORT_CODE(KEYCODE_0_PAD)     PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)   // ?? PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(UCHAR_MAMEKEY(COMMA_PAD))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)   // ?? PORT_CODE(KEYCODE_DEL_PAD)   PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))

	PORT_INCLUDE(msx_dips)
INPUT_PORTS_END

static INPUT_PORTS_START(svi728sp)
	KEYB_ROW0
	KEYB_SP_ROW1
	KEYB_SP_ROW2
	KEYB_ROW3
	KEYB_ROW4
	KEYB_ROW5
	KEYB_ROW6
	KEYB_ROW7
	KEYB_ROW8

	PORT_START("KEY9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD)  PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK)  PORT_NAME("Keypad *")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD) PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)   // ?? PORT_CODE(KEYCODE_1_PAD)     PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)   // ?? PORT_CODE(KEYCODE_2_PAD)     PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)   // ?? PORT_CODE(KEYCODE_3_PAD)     PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)   // ?? PORT_CODE(KEYCODE_4_PAD)     PORT_CHAR(UCHAR_MAMEKEY(4_PAD))

	PORT_START("KEY10")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)   // ?? PORT_CODE(KEYCODE_5_PAD)     PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)   // ?? PORT_CODE(KEYCODE_6_PAD)     PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)   // ?? PORT_CODE(KEYCODE_7_PAD)     PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)   // ?? PORT_CODE(KEYCODE_8_PAD)     PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)   // ?? PORT_CODE(EYCODE_9_PAD)      PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)   // ?? PORT_CODE(KEYCODE_0_PAD)     PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)   // ?? PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(UCHAR_MAMEKEY(COMMA_PAD))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)   // ?? PORT_CODE(KEYCODE_DEL_PAD)   PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))

	PORT_INCLUDE(msx_dips)
INPUT_PORTS_END

static INPUT_PORTS_START(svi738dk)
	KEYB_SC_ROW0

	PORT_START("KEY1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('8')  PORT_CHAR('(')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)          PORT_CHAR('9')  PORT_CHAR(')')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('+')  PORT_CHAR('?')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('\'') PORT_CHAR('`')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('<')  PORT_CHAR('>')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR(0xe5) PORT_CHAR(0xc5) // U+00E5 and U+00C5, a/A with ring above
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('@')  PORT_CHAR('^')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(0xe6) PORT_CHAR(0xc6) // U+00E6 and U+00C6, ae/AE

	PORT_START("KEY2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)     PORT_CHAR(0xf8)          PORT_CHAR(0xd8) // U+00F8 and U+00D8, o/O with stroke
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('~')           PORT_CHAR('*')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)     PORT_CHAR(',')           PORT_CHAR(';')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)      PORT_CHAR('.')           PORT_CHAR(':')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)     PORT_CHAR('-')           PORT_CHAR('_')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Dead Key")        PORT_CODE(KEYCODE_TILDE)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)         PORT_CHAR('a')           PORT_CHAR('A')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)         PORT_CHAR('b')           PORT_CHAR('B')

	KEYB_ROW3
	KEYB_ROW4
	KEYB_ROW5
	KEYB_ROW6
	KEYB_ROW7
	KEYB_ROW8

	PORT_START("KEY9")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY10")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_INCLUDE(msx_dips)
INPUT_PORTS_END

static INPUT_PORTS_START(svi738sw)
	KEYB_SC_ROW0

	PORT_START("KEY1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('8')  PORT_CHAR('(')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)          PORT_CHAR('9')  PORT_CHAR(')')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('+')  PORT_CHAR('?')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR(0xe9) PORT_CHAR(0xc9) // U+00E9 and U+00C9, e/E with acute
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('<')  PORT_CHAR('>')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR(0xe5) PORT_CHAR(0xc5) // U+00E5 and U+00C5, a/A with ring above
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(0xfc) PORT_CHAR(0xdc) // U+00FC and U+00DC, u/U with diaeresis
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(0xf6) PORT_CHAR(0xd6) // U+00F6 and U+00D6, o/O with diaeresis

	PORT_START("KEY2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)     PORT_CHAR(0xe4)          PORT_CHAR(0xc4) // U+00F8 and U+00D8, a/A with diaeresis
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\'')          PORT_CHAR('*')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)     PORT_CHAR(',')           PORT_CHAR(';')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)      PORT_CHAR('.')           PORT_CHAR(':')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)     PORT_CHAR('-')           PORT_CHAR('_')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Dead Key")        PORT_CODE(KEYCODE_TILDE)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)         PORT_CHAR('a')           PORT_CHAR('A')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)         PORT_CHAR('b')           PORT_CHAR('B')

	KEYB_ROW3
	KEYB_ROW4
	KEYB_ROW5
	KEYB_ROW6
	KEYB_ROW7
	KEYB_ROW8

	PORT_START("KEY9")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY10")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_INCLUDE(msx_dips)
INPUT_PORTS_END

static INPUT_PORTS_START(vg8010)
	PORT_START("KEY0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',')  PORT_CHAR('<')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)          PORT_CHAR('x')  PORT_CHAR('X')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)          PORT_CHAR('s')  PORT_CHAR('S')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)          PORT_CHAR('1')  PORT_CHAR('!')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Dead Key")         PORT_CODE(KEYCODE_TILDE)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)          PORT_CHAR('l')  PORT_CHAR('L')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)          PORT_CHAR('b')  PORT_CHAR('B')

	PORT_START("KEY1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)         PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)         PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)         PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)         PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)    PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)         PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)         PORT_CHAR('v') PORT_CHAR('V')

	PORT_START("KEY2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)          PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)          PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)          PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)          PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)          PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)          PORT_CHAR('g') PORT_CHAR('G')

	PORT_START("KEY3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)     PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)     PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)     PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)     PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)     PORT_CHAR('t') PORT_CHAR('T')

	PORT_START("KEY4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)     PORT_CHAR('h')  PORT_CHAR('H')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)     PORT_CHAR('7')  PORT_CHAR('&')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)     PORT_CHAR('r')  PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)     PORT_CHAR('3')  PORT_CHAR('#')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)     PORT_CHAR('p')  PORT_CHAR('P')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)     PORT_CHAR('k')  PORT_CHAR('K')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)     PORT_CHAR('6')  PORT_CHAR('^')

	PORT_START("KEY5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)      PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)         PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)         PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)         PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)         PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)         PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)         PORT_CHAR('5') PORT_CHAR('%')

	KEYB_ROW6
	KEYB_ROW7
	KEYB_ROW8

	PORT_START("KEY9")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY10")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_INCLUDE(msx_dips)
INPUT_PORTS_END

static INPUT_PORTS_START(vg8010f)
	PORT_START("KEY0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(';')  PORT_CHAR('.')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)          PORT_CHAR('x')  PORT_CHAR('X')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)          PORT_CHAR('s')  PORT_CHAR('S')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)          PORT_CHAR('&')  PORT_CHAR('1')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('#')  PORT_CHAR(0xa3) /* U+00A3 - GBP sign */
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Dead Key")         PORT_CODE(KEYCODE_TILDE)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)          PORT_CHAR('l')  PORT_CHAR('L')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)          PORT_CHAR('b')  PORT_CHAR('B')

	PORT_START("KEY1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)         PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)         PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)         PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)         PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('<') PORT_CHAR('>')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)    PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)         PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)         PORT_CHAR('v') PORT_CHAR('V')

	PORT_START("KEY2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)          PORT_CHAR(',')  PORT_CHAR('?')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)          PORT_CHAR('w')  PORT_CHAR('W')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)          PORT_CHAR('f')  PORT_CHAR('F')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)          PORT_CHAR('z')  PORT_CHAR('Z')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('$')  PORT_CHAR('*')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)          PORT_CHAR(0xe0) PORT_CHAR('0') /* U+00E0, a with grave */
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('!')  PORT_CHAR('8')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)          PORT_CHAR('g')  PORT_CHAR('G')

	PORT_START("KEY3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('=')  PORT_CHAR('+')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)     PORT_CHAR('u')  PORT_CHAR('U')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)     PORT_CHAR('d')  PORT_CHAR('D')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)     PORT_CHAR(0xe9) PORT_CHAR('2') /* U+00E9, e with acute */
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR('m')  PORT_CHAR('M')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR(')')  PORT_CHAR(0xb0) /* U+00B0, degree */
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)     PORT_CHAR('i')  PORT_CHAR('I')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)     PORT_CHAR('t')  PORT_CHAR('T')

	PORT_START("KEY4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)     PORT_CHAR('h')  PORT_CHAR('H')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)     PORT_CHAR(0xe8) PORT_CHAR('7') /* U+00E8, e with grave */
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)     PORT_CHAR('r')  PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)     PORT_CHAR('"')  PORT_CHAR('3')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(0xf9) PORT_CHAR('%') /* U+00F9 u grave */
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)     PORT_CHAR('p')  PORT_CHAR('P')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)     PORT_CHAR('k')  PORT_CHAR('K')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)     PORT_CHAR(0xa7) PORT_CHAR('6') /* U+00A7, section sign */

	PORT_START("KEY5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)      PORT_CHAR(':')  PORT_CHAR('/')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)         PORT_CHAR('y')  PORT_CHAR('Y')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)         PORT_CHAR('e')  PORT_CHAR('E')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)         PORT_CHAR('\'') PORT_CHAR('4')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("Dead key")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)         PORT_CHAR(0xe7) PORT_CHAR('9') /* U+00E7, c with cedilla */
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)         PORT_CHAR('o')  PORT_CHAR('O')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)         PORT_CHAR('(')  PORT_CHAR('5')

	KEYB_ROW6
	KEYB_ROW7
	KEYB_ROW8

	PORT_START("KEY9")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY10")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_INCLUDE(msx_dips)
INPUT_PORTS_END

static INPUT_PORTS_START(y503iir2)
	PORT_START("KEY0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('=')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('/')

	PORT_START("KEY1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('8')  PORT_CHAR('(')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)          PORT_CHAR('9')  PORT_CHAR(')')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('+')  PORT_CHAR('?')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('\'') PORT_CHAR('`')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('\'') PORT_CHAR('*')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[')  PORT_CHAR('{')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']')  PORT_CHAR('}')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR('\\')  PORT_CHAR('|')

	PORT_START("KEY2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)     PORT_CHAR('<')           PORT_CHAR('>')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('^')           PORT_CHAR('~')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)     PORT_CHAR(',')           PORT_CHAR(';')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)      PORT_CHAR('.')           PORT_CHAR(':')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)     PORT_CHAR('-')           PORT_CHAR('_')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Dead Key")        PORT_CODE(KEYCODE_TILDE)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)         PORT_CHAR('a')           PORT_CHAR('A')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)         PORT_CHAR('b')           PORT_CHAR('B')

	KEYB_ROW3
	KEYB_ROW4
	KEYB_ROW5
	KEYB_ROW6
	KEYB_ROW7
	KEYB_ROW8

	PORT_START("KEY9")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY10")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_INCLUDE(msx_dips)
INPUT_PORTS_END

static INPUT_PORTS_START(msx2)
	KEYB_ROW0
	KEYB_ROW1
	KEYB_ROW2
	KEYB_ROW3
	KEYB_ROW4
	KEYB_ROW5
	KEYB_ROW6
	KEYB_ROW7
	KEYB_ROW8
	KEYB_ROW9
	KEYB_ROW10

	PORT_INCLUDE(msx_dips)
INPUT_PORTS_END

static INPUT_PORTS_START(msx2de)
	KEYB_DE_ROW0
	KEYB_DE_ROW1
	KEYB_DE_ROW2
	KEYB_ROW3
	KEYB_ROW4
	KEYB_DE_ROW5
	KEYB_ROW6
	KEYB_ROW7
	KEYB_ROW8
	KEYB_ROW9
	KEYB_ROW10

	PORT_INCLUDE(msx_dips)
INPUT_PORTS_END

static INPUT_PORTS_START(msx2fr)
	KEYB_FR_ROW0
	KEYB_FR_ROW1
	KEYB_FR_ROW2
	KEYB_ROW3
	KEYB_FR_ROW4
	KEYB_FR_ROW5
	KEYB_ROW6
	KEYB_ROW7
	KEYB_ROW8
	KEYB_ROW9
	KEYB_ROW10

	PORT_INCLUDE(msx_dips)
INPUT_PORTS_END

static INPUT_PORTS_START(msx2jp)
	KEYB_JP_ROW0
	KEYB_JP_ROW1
	KEYB_JP_ROW2
	KEYB_ROW3
	KEYB_ROW4
	KEYB_ROW5
	KEYB_JP_ROW6
	KEYB_ROW7
	KEYB_ROW8
	KEYB_ROW9
	KEYB_ROW10

	PORT_INCLUDE(msx_dips)
INPUT_PORTS_END

static INPUT_PORTS_START(msx2kr)
	KEYB_JP_ROW0
	KEYB_KR_ROW1
	KEYB_JP_ROW2
	KEYB_ROW3
	KEYB_ROW4
	KEYB_ROW5
	KEYB_KR_ROW6
	KEYB_ROW7
	KEYB_ROW8

	PORT_START("KEY9")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY10")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_INCLUDE(msx_dips)
INPUT_PORTS_END

static INPUT_PORTS_START(msx2ru)
	KEYB_RU_ROW0
	KEYB_RU_ROW1
	KEYB_RU_ROW2
	KEYB_RU_ROW3
	KEYB_RU_ROW4
	KEYB_RU_ROW5
	KEYB_ROW6
	KEYB_ROW7
	KEYB_ROW8
	KEYB_ROW9
	KEYB_ROW10

	PORT_INCLUDE(msx_dips)
INPUT_PORTS_END

static INPUT_PORTS_START(msx2sp)
	KEYB_ROW0
	KEYB_SP_ROW1
	KEYB_SP_ROW2
	KEYB_ROW3
	KEYB_ROW4
	KEYB_ROW5
	KEYB_ROW6
	KEYB_ROW7
	KEYB_ROW8
	KEYB_ROW9
	KEYB_ROW10

	PORT_INCLUDE(msx_dips)
INPUT_PORTS_END

static INPUT_PORTS_START(msx2uk)
	KEYB_ROW0
	KEYB_ROW1
	KEYB_UK_ROW2
	KEYB_ROW3
	KEYB_ROW4
	KEYB_ROW5
	KEYB_ROW6
	KEYB_ROW7
	KEYB_ROW8
	KEYB_ROW9
	KEYB_ROW10

	PORT_INCLUDE(msx_dips)
INPUT_PORTS_END


// Some MSX2+ can switch the z80 clock between 3.5 and 5.3 MHz
WRITE_LINE_MEMBER(msx2_state::turbo_w)
{
	// 0 - 5.369317 MHz
	// 1 - 3.579545 MHz
	m_maincpu->set_unscaled_clock(21.477272_MHz_XTAL / (state ? 6 : 4));
}


void msx_state::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_MSX_FORMAT);
	fr.add(FLOPPY_DMK_FORMAT);
}

static void msx_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
	device.option_add("35ssdd", FLOPPY_35_SSDD);
}

void msx_state::msx_fd1793_force_ready(machine_config &config)
{
	fd1793_device& fdc(FD1793(config, "fdc", 4_MHz_XTAL / 4));
	fdc.set_force_ready(true);
}

void msx_state::msx_fd1793(machine_config &config)
{
	FD1793(config, "fdc", 4_MHz_XTAL / 4);
}

void msx_state::msx_wd2793_force_ready(machine_config &config)
{
	// From NMS8245 schematics:
	// READY + HLT - pulled high
	// SSO/-ENMF + -DDEN + ENP + -5/8 - pulled low
	wd2793_device& fdc(WD2793(config, "fdc", 4_MHz_XTAL / 4));
	fdc.set_force_ready(true);
}

void msx_state::msx_wd2793(machine_config &config)
{
	WD2793(config, "fdc", 4_MHz_XTAL / 4);
}

void msx_state::msx_mb8877a(machine_config & config)
{
	// From CF-3300 FDC schematic:
	// READY + HLT - pulled high
	// -DDEN - pulled low
	mb8877_device& fdc(MB8877(config, "fdc", 4_MHz_XTAL / 4));
	fdc.set_force_ready(true);
}

void msx_state::msx_microsol(machine_config &config)
{
	wd2793_device& fdc(WD2793(config, "fdc", 4_MHz_XTAL / 4));
	fdc.set_force_ready(true);
}

void msx_state::msx_1_35_ssdd_drive(machine_config &config)
{
	FLOPPY_CONNECTOR(config, "fdc:0", msx_floppies, "35ssdd", msx_state::floppy_formats);
}

void msx_state::msx_1_35_dd_drive(machine_config &config)
{
	FLOPPY_CONNECTOR(config, "fdc:0", msx_floppies, "35dd", msx_state::floppy_formats);
}

void msx_state::msx_2_35_dd_drive(machine_config &config)
{
	FLOPPY_CONNECTOR(config, "fdc:0", msx_floppies, "35dd", msx_state::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", msx_floppies, "35dd", msx_state::floppy_formats);
}

void msx2_state::msx_ym2413(machine_config &config)
{
	YM2413(config, "ym2413", 21.477272_MHz_XTAL / 6).add_route(ALL_OUTPUTS, m_speaker, 0.4);
}

void msx2_state::msx2_64kb_vram(machine_config &config)
{
	m_v9938->set_vram_size(0x10000);
}

template<typename AY8910Type, typename T, typename Ret, typename... Params>
void msx_state::msx_base(AY8910Type &ay8910_type, machine_config &config, XTAL xtal, int cpu_divider, Ret (T::*func)(Params...))
{
	// basic machine hardware
	Z80(config, m_maincpu, xtal / cpu_divider);         // 3.579545 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, func);
	config.set_maximum_quantum(attotime::from_hz(60));

	INPUT_MERGER_ANY_HIGH(config, m_mainirq).output_handler().set_inputline("maincpu", INPUT_LINE_IRQ0);

	I8255(config, m_ppi);
	m_ppi->out_pa_callback().set(FUNC(msx_state::ppi_port_a_w));
	m_ppi->in_pb_callback().set(FUNC(msx_state::ppi_port_b_r));
	m_ppi->out_pc_callback().set(FUNC(msx_state::ppi_port_c_w));

	// Video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);

	// sound hardware
	SPEAKER(config, m_speaker).front_center();
	DAC_1BIT(config, m_dac, 0).add_route(ALL_OUTPUTS, m_speaker, 0.1);

	ay8910_type(config, m_ay8910, xtal / cpu_divider / 2);
	m_ay8910->set_flags(AY8910_SINGLE_OUTPUT);
	m_ay8910->port_a_read_callback().set(FUNC(msx2_state::psg_port_a_r));
	m_ay8910->port_b_read_callback().set(FUNC(msx2_state::psg_port_b_r));
	m_ay8910->port_a_write_callback().set(FUNC(msx2_state::psg_port_a_w));
	m_ay8910->port_b_write_callback().set(FUNC(msx2_state::psg_port_b_w));
	m_ay8910->add_route(ALL_OUTPUTS, m_speaker, 0.3);

	if (m_hw_def.has_printer_port())
	{
		// printer
		CENTRONICS(config, m_centronics, centronics_devices, "printer");
		m_centronics->busy_handler().set(m_cent_status_in, FUNC(input_buffer_device::write_bit1));

		OUTPUT_LATCH(config, m_cent_data_out);
		m_centronics->set_output_latch(*m_cent_data_out);
		INPUT_BUFFER(config, m_cent_status_in);

		OUTPUT_LATCH(config, m_cent_ctrl_out);
		m_cent_ctrl_out->bit_handler<1>().set(m_centronics, FUNC(centronics_device::write_strobe));
	}

	if (m_hw_def.has_cassette())
	{
		// cassette
		CASSETTE(config, m_cassette);
		m_cassette->set_formats(fmsx_cassette_formats);
		m_cassette->set_default_state(CASSETTE_PLAY);
		m_cassette->add_route(ALL_OUTPUTS, m_speaker, 0.05);
		m_cassette->set_interface("msx_cass");
	}
}

template<typename AY8910Type>
void msx_state::msx_base(AY8910Type &ay8910_type, machine_config &config, XTAL xtal, int cpu_divider)
{
	// basic machine hardware
	Z80(config, m_maincpu, xtal / cpu_divider);         // 3.579545 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &msx_state::memory_map);
	config.set_maximum_quantum(attotime::from_hz(60));

	INPUT_MERGER_ANY_HIGH(config, m_mainirq).output_handler().set_inputline("maincpu", INPUT_LINE_IRQ0);

	I8255(config, m_ppi);
	m_ppi->out_pa_callback().set(FUNC(msx_state::ppi_port_a_w));
	m_ppi->in_pb_callback().set(FUNC(msx_state::ppi_port_b_r));
	m_ppi->out_pc_callback().set(FUNC(msx_state::ppi_port_c_w));

	// Video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);

	// sound hardware
	SPEAKER(config, m_speaker).front_center();
	DAC_1BIT(config, m_dac, 0).add_route(ALL_OUTPUTS, m_speaker, 0.1);

	ay8910_type(config, m_ay8910, xtal / cpu_divider / 2);
	m_ay8910->set_flags(AY8910_SINGLE_OUTPUT);
	m_ay8910->port_a_read_callback().set(FUNC(msx2_state::psg_port_a_r));
	m_ay8910->port_b_read_callback().set(FUNC(msx2_state::psg_port_b_r));
	m_ay8910->port_a_write_callback().set(FUNC(msx2_state::psg_port_a_w));
	m_ay8910->port_b_write_callback().set(FUNC(msx2_state::psg_port_b_w));
	m_ay8910->add_route(ALL_OUTPUTS, m_speaker, 0.3);

	if (m_hw_def.has_printer_port())
	{
		// printer
		CENTRONICS(config, m_centronics, centronics_devices, "printer");
		m_centronics->busy_handler().set(m_cent_status_in, FUNC(input_buffer_device::write_bit1));

		OUTPUT_LATCH(config, m_cent_data_out);
		m_centronics->set_output_latch(*m_cent_data_out);
		INPUT_BUFFER(config, m_cent_status_in);

		OUTPUT_LATCH(config, m_cent_ctrl_out);
		m_cent_ctrl_out->bit_handler<1>().set(m_centronics, FUNC(centronics_device::write_strobe));
	}

	if (m_hw_def.has_cassette())
	{
		// cassette
		CASSETTE(config, m_cassette);
		m_cassette->set_formats(fmsx_cassette_formats);
		m_cassette->set_default_state(CASSETTE_PLAY);
		m_cassette->add_route(ALL_OUTPUTS, m_speaker, 0.05);
		m_cassette->set_interface("msx_cass");
	}
}

template<typename VDPType, typename AY8910Type, typename T, typename Ret, typename... Params>
void msx_state::msx1(VDPType &vdp_type, AY8910Type &ay8910_type, machine_config &config, Ret (T::*func)(Params...))
{
	msx_base(ay8910_type, config, 10.738635_MHz_XTAL, 3, func);

	m_maincpu->set_addrmap(AS_IO, &msx_state::msx_io_map);
	m_maincpu->set_vblank_int("screen", FUNC(msx_state::msx_interrupt)); /* Needed for mouse updates */

	vdp_type(config, m_tms9928a, 10.738635_MHz_XTAL);
	m_tms9928a->set_screen(m_screen);
	m_tms9928a->set_vram_size(0x4000);
	m_tms9928a->int_callback().set(m_mainirq, FUNC(input_merger_device::in_w<0>));

	// Software lists
	if (m_hw_def.has_cassette())
	{
		SOFTWARE_LIST(config, "cass_list").set_original("msx1_cass");
	}

	if (m_hw_def.has_cartslot())
	{
		SOFTWARE_LIST(config, "cart_list").set_original("msx1_cart");
	}

	if (m_hw_def.has_fdc())
	{
		SOFTWARE_LIST(config, "flop_list").set_original("msx1_flop");
	}
}

template<typename VDPType, typename AY8910Type>
void msx_state::msx1(VDPType &vdp_type, AY8910Type &ay8910_type, machine_config &config)
{
	msx_base(ay8910_type, config, 10.738635_MHz_XTAL, 3);

	m_maincpu->set_addrmap(AS_IO, &msx_state::msx_io_map);
	m_maincpu->set_vblank_int("screen", FUNC(msx_state::msx_interrupt)); /* Needed for mouse updates */

	vdp_type(config, m_tms9928a, 10.738635_MHz_XTAL);
	m_tms9928a->set_screen(m_screen);
	m_tms9928a->set_vram_size(0x4000);
	m_tms9928a->int_callback().set(m_mainirq, FUNC(input_merger_device::in_w<0>));

	// Software lists
	if (m_hw_def.has_cassette())
	{
		SOFTWARE_LIST(config, "cass_list").set_original("msx1_cass");
	}

	if (m_hw_def.has_cartslot())
	{
		SOFTWARE_LIST(config, "cart_list").set_original("msx1_cart");
	}

	if (m_hw_def.has_fdc())
	{
		SOFTWARE_LIST(config, "flop_list").set_original("msx1_flop");
	}
}

template<typename AY8910Type>
void msx1_v9938_state::msx1_v9938(AY8910Type &ay8910_type, machine_config &config)
{
	msx_base(ay8910_type, config, 21.477272_MHz_XTAL, 6);

	m_maincpu->set_addrmap(AS_IO, &msx1_v9938_state::io_map);

	// video hardware
	V9938(config, m_v9938, 21.477272_MHz_XTAL);
	m_v9938->set_screen_ntsc(m_screen);
	m_v9938->set_vram_size(0x4000);
	m_v9938->int_cb().set(m_mainirq, FUNC(input_merger_device::in_w<0>));

	// Software lists
	if (m_hw_def.has_cassette())
	{
		SOFTWARE_LIST(config, "cass_list").set_original("msx1_cass");
	}

	if (m_hw_def.has_cartslot())
	{
		SOFTWARE_LIST(config, "cart_list").set_original("msx1_cart");
	}

	if (m_hw_def.has_fdc())
	{
		SOFTWARE_LIST(config, "flop_list").set_original("msx1_flop");
	}
}

template<typename AY8910Type, typename T, typename Ret, typename... Params>
void msx1_v9938_state::msx1_v9938(AY8910Type &ay8910_type, machine_config &config, Ret (T::*func)(Params...))
{
	msx_base(ay8910_type, config, 21.477272_MHz_XTAL, 6, func);

	m_maincpu->set_addrmap(AS_IO, &msx1_v9938_state::io_map);

	// video hardware
	V9938(config, m_v9938, 21.477272_MHz_XTAL);
	m_v9938->set_screen_ntsc(m_screen);
	m_v9938->set_vram_size(0x4000);
	m_v9938->int_cb().set(m_mainirq, FUNC(input_merger_device::in_w<0>));

	// Software lists
	if (m_hw_def.has_cassette())
	{
		SOFTWARE_LIST(config, "cass_list").set_original("msx1_cass");
	}

	if (m_hw_def.has_cartslot())
	{
		SOFTWARE_LIST(config, "cart_list").set_original("msx1_cart");
	}

	if (m_hw_def.has_fdc())
	{
		SOFTWARE_LIST(config, "flop_list").set_original("msx1_flop");
	}
}

template<typename AY8910Type>
void msx1_v9938_state::msx1_v9938_pal(AY8910Type &ay8910_type, machine_config &config)
{
	msx1_v9938(ay8910_type, config);
	m_v9938->set_screen_pal(m_screen);
}

template<typename AY8910Type, typename T, typename Ret, typename... Params>
void msx1_v9938_state::msx1_v9938_pal(AY8910Type &ay8910_type, machine_config &config, Ret (T::*func)(Params...))
{
	msx1_v9938(ay8910_type, config, func);
	m_v9938->set_screen_pal(m_screen);
}

template<typename AY8910Type>
void msx2_state::msx2_base(AY8910Type &ay8910_type, machine_config &config)
{
	msx_base(ay8910_type, config, 21.477272_MHz_XTAL, 6);

	// real time clock
	RP5C01(config, m_rtc, 32.768_kHz_XTAL);
}

template<typename AY8910Type>
void msx2_state::msx2(AY8910Type &ay8910_type, machine_config &config)
{
	msx2_base(ay8910_type, config);

	m_maincpu->set_addrmap(AS_IO, &msx2_state::msx2_io_map);

	// video hardware
	V9938(config, m_v9938, 21.477272_MHz_XTAL);
	m_v9938->set_screen_ntsc(m_screen);
	m_v9938->set_vram_size(0x20000);
	m_v9938->int_cb().set(m_mainirq, FUNC(input_merger_device::in_w<0>));

	// Software lists
	if (m_hw_def.has_cassette())
	{
		SOFTWARE_LIST(config, "cass_list").set_original("msx2_cass");
		SOFTWARE_LIST(config, "msx1_cas_l").set_compatible("msx1_cass");
	}

	if (m_hw_def.has_cartslot())
	{
		SOFTWARE_LIST(config, "cart_list").set_original("msx2_cart");
		SOFTWARE_LIST(config, "msx1_crt_l").set_compatible("msx1_cart");
	}

	if (m_hw_def.has_fdc())
	{
		SOFTWARE_LIST(config, "flop_list").set_original("msx2_flop");
		SOFTWARE_LIST(config, "msx1_flp_l").set_compatible("msx1_flop");
	}
}

template<typename AY8910Type>
void msx2_state::msx2_pal(AY8910Type &ay8910_type, machine_config &config)
{
	msx2(ay8910_type, config);
	m_v9938->set_screen_pal(m_screen);
}

template<typename AY8910Type>
void msx2_state::msx2plus_base(AY8910Type &ay8910_type, machine_config &config)
{
	msx2_base(ay8910_type, config);

	m_maincpu->set_addrmap(AS_IO, &msx2_state::msx2plus_io_map);

	// video hardware
	V9958(config, m_v9958, 21.477272_MHz_XTAL);
	m_v9958->set_screen_ntsc(m_screen);
	m_v9958->set_vram_size(0x20000);
	m_v9958->int_cb().set(m_mainirq, FUNC(input_merger_device::in_w<0>));
}

template<typename AY8910Type>
void msx2_state::msx2plus(AY8910Type &ay8910_type, machine_config &config)
{
	msx2plus_base(ay8910_type, config);

	// Software lists
	if (m_hw_def.has_cassette())
	{
		SOFTWARE_LIST(config, "cass_list").set_original("msx2_cass");
		SOFTWARE_LIST(config, "msx1_cas_l").set_compatible("msx1_cass");
	}

	if (m_hw_def.has_cartslot())
	{
		SOFTWARE_LIST(config, "cart_list").set_original("msx2_cart");
		SOFTWARE_LIST(config, "msx1_crt_l").set_compatible("msx1_cart");
	}

	if (m_hw_def.has_fdc())
	{
		SOFTWARE_LIST(config, "flop_list").set_original("msx2p_flop");
		SOFTWARE_LIST(config, "msx2_flp_l").set_compatible("msx2_flop");
		SOFTWARE_LIST(config, "msx1_flp_l").set_compatible("msx1_flop");
	}
}

template<typename AY8910Type>
void msx2_state::msx2plus_pal(AY8910Type &ay8910_type, machine_config &config)
{
	msx2plus(ay8910_type, config);
	m_v9958->set_screen_pal(m_screen);
}

template<typename AY8910Type>
void msx2_state::turbor(AY8910Type &ay8910_type, machine_config &config)
{
	msx2plus_base(ay8910_type, config);

	R800(config.replace(), m_maincpu, 28.636363_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &msx2_state::memory_map);
	m_maincpu->set_addrmap(AS_IO, &msx2_state::msx2plus_io_map);

	// Software lists
	if (m_hw_def.has_cassette())
	{
		SOFTWARE_LIST(config, "cass_list").set_original("msx2_cass");
		SOFTWARE_LIST(config, "msx1_cas_l").set_compatible("msx1_cass");
	}

	if (m_hw_def.has_cartslot())
	{
		SOFTWARE_LIST(config, "cart_list").set_original("msx2_cart");
		SOFTWARE_LIST(config, "msx1_crt_l").set_compatible("msx1_cart");
	}

	if (m_hw_def.has_fdc())
	{
		SOFTWARE_LIST(config, "flop_list").set_original("msxr_flop");
		SOFTWARE_LIST(config, "msx2p_flp_l").set_compatible("msx2p_flop");
		SOFTWARE_LIST(config, "msx2_flp_l").set_compatible("msx2_flop");
		SOFTWARE_LIST(config, "msx1_flp_l").set_compatible("msx1_flop");
	}
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

/********************************  MSX 1 **********************************/

/* MSX - Al Fateh 100 - rebranded Sakhr / Al Alamiah AX-170, dump needed to verify */

/* MSX - Al Fateh 123 - rebranded Sakhr / Al Alamiah AX-230, dump needed to verify */

/* MSX - AVT DPC-200 - See Fenner DPC-200 (they are the same machine, same roms) */

/* MSX - AVT FC-200 - probably same as Goldstar FC-80, dump needed to verify */

/* MSX - Bawareth Perfect MSX1 (DPC-200CD) */

ROM_START(perfect1)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("perfect1bios.rom", 0x0000, 0x8000, CRC(a317e6b4) SHA1(e998f0c441f4f1800ef44e42cd1659150206cf79))

	ROM_REGION(0x8000, "firmware", 0)
	ROM_SYSTEM_BIOS(0, "v1990", "1990 Firmware")
	ROMX_LOAD("cpc-200bw_v1_0", 0x0000, 0x8000, CRC(d6373270) SHA1(29a9169b605b5881e4a15fcfd65209a4e8679285), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v1987", "1987 Firmware (v3.21)")
	ROMX_LOAD("perfect1arab.rom", 0x0000, 0x8000, CRC(6db04a4d) SHA1(01012a0e2738708861f66b6921b2e2108f2edb54), ROM_BIOS(1))
ROM_END

void msx_state::perfect1(machine_config &config)
{
	// GSS Z8400A PS cpu
	// AY-3-8910
	// TMS9129
	// DW64MX1
	// FDC: None, 0 drives
	// 1 Cartridge slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 0, 1, 1, 2, "firmware");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 2, 0, 4); // 64KB RAM
	add_cartridge_slot<1>(config, 1);
	// expansion slot in slot #2

	msx1(TMS9129, AY8910, config);
}

/* MSX - Canon V-8 */

ROM_START(canonv8)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("v8bios.rom", 0x0000, 0x8000, CRC(e941b08e) SHA1(97f9a0b45ee4b34d87ca3f163df32e1f48b0f09c))
ROM_END

void msx_state::canonv8(machine_config &config)
{
	// NEC D780C cpu
	// YM2149 in S3527
	// FDC: None, 0 drives
	// 1 Cartridge slots
	// S3527
	// No printer port

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 3, 1);

	m_hw_def.has_printer_port(false);
	msx1(TMS9118, YM2149, config);
}

/* MSX - Canon V-10 */

ROM_START(canonv10)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("v10bios.rom", 0x0000, 0x8000, CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821))
ROM_END

void msx_state::canonv10(machine_config &config)
{
	// Zilog Z80A
	// YM2149
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 3, 1);   // 16KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);

	msx1(TMS9918A, YM2149, config);
}

/* MSX - Canon V-20 */

ROM_START(canonv20)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("v20bios.rom", 0x0000, 0x8000, CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821))
ROM_END

void msx_state::canonv20(machine_config &config)
{
	// NEC D780C cpu
	// XTAL: 14.31818(Z80/PSG) + 10.6875(VDP)
	// YM2149
	// TMS9918ANL
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 4);  // 64KB RAM

	msx1(TMS9918A, YM2149, config);
}

/* MSX - Canon V-20E */

ROM_START(canonv20e)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("3256m67-5a3_z-2_uk.u20", 0x0000, 0x8000, CRC(e9ccd789) SHA1(8963fc041975f31dc2ab1019cfdd4967999de53e))
ROM_END

void msx_state::canonv20e(machine_config &config)
{
	// Zilog Z8400A PS Z80A cpu
	// XTAL: 14.31818(Z80/PSG) + 10.6875(VDP)
	// YM2149
	// TMS9929ANL
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 4);  // 64KB RAM

	msx1(TMS9929A, YM2149, config);
}

/* MSX - Canon V-20F */

ROM_START(canonv20f)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("v20fbios.rom", 0x0000, 0x8000, BAD_DUMP CRC(e0e894b7) SHA1(d99eebded5db5fce1e072d08e642c0909bc7efdd)) // need verification
ROM_END

/* MSX - Canon V-20G */

ROM_START(canonv20g)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("v20gbios.rom", 0x0000, 0x8000, BAD_DUMP CRC(d6e704ad) SHA1(d67be6d7d56d7229418f4e122f2ec27990db7d19)) // need verification
ROM_END

/* MSX - Canon V-20S */

ROM_START(canonv20s)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("v20sbios.rom", 0x0000, 0x8000, BAD_DUMP CRC(c72b186e) SHA1(9fb289ea5c11d497ee00703f64e82575d1c59923)) // need verification
ROM_END

/* MSX - Casio MX-10 */

ROM_START(mx10)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("3256d19-5k3_z-1.g2", 0x0000, 0x8000, CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821))
ROM_END

void msx_state::mx10(machine_config &config)
{
	// Z80: uPD780C-1
	// AY-3-8910
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// 1 Expansion slot for KB-10 to add a printer port and 2 more cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 3, 1); // 16KB RAM
	add_cartridge_slot<1>(config, 1);

	m_hw_def.has_cassette(false).has_printer_port(false);
	msx1(TMS9118, AY8910, config);
}

/* MSX - Casio MX-15 */

ROM_START(mx15)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("mx15bios.rom", 0x0000, 0x8000, CRC(6481230f) SHA1(a7ed5fd940f4e3a33e676840c0a83ac7ee54d972))
ROM_END

void msx_state::mx15(machine_config &config)
{
	// AY-3-8910
	// T6950
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// 1 Expansion slot for KB-15 to add a printer port and 2 more cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 3, 1); // 16KB RAM
	add_cartridge_slot<1>(config, 1);

	m_hw_def.has_printer_port(false);
	msx1(TMS9928A, AY8910, config);
}

/* MSX - Casio MX-101 */

ROM_START(mx101)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("3256d19-5k3_z-1", 0x0000, 0x8000, BAD_DUMP CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821)) // need verification
ROM_END

void msx_state::mx101(machine_config &config)
{
	// Z80: uPD780C-1
	// AY-3-8910
	// FDC: None, 0 drives
	// 1 Cartridge slots
	// 1 Expansion slot for KB-10 to add a printer port and 2 more cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 3, 1); // 16KB RAM
	add_cartridge_slot<1>(config, 1);

	m_hw_def.has_cassette(false).has_printer_port(false);
	msx1(TMS9118, AY8910, config);
}

/* MSX - Casio PV-7 */

ROM_START(pv7)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("pv7bios.rom", 0x0000, 0x8000, CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821))
ROM_END

void msx_state::pv7(machine_config &config)
{
	// AY8910?
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// Non-standard cassette port
	// No printer port
	// 1 Expansion slot for KB-7 to add a printer port, 2 more cartridge slots, and 8KB RAM
	// Z80: uPD780C-1

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 3, 1).force_start_address(0xe000);   // 8KB RAM
	add_cartridge_slot<1>(config, 1);

	m_hw_def.has_printer_port(false);
	msx1(TMS9118, AY8910, config);
}

/* MSX - Casio PV-16 */

ROM_START(pv16)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("3256d19-5k3_z-1", 0x0000, 0x8000, CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821))
ROM_END

void msx_state::pv16(machine_config &config)
{
	// NEC UPD780C
	// AY-3-8910
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// No printer port
	// 1 Expansion slot for KB-7 to add a printer port, 2 more cartridge slots, and 8KB RAM

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 3, 1);   // 16KB RAM
	add_cartridge_slot<1>(config, 1);

	m_hw_def.has_printer_port(false);
	msx1(TMS9118, AY8910, config);
}

/* MSX - CE-TEC MPC-80, German version of Daewoo DPC-200, dump needed to verify */

/* MSX - Daewoo CPC-200 */

/* MSX - Daewoo CPC-88 */

ROM_START(cpc88)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("88bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(3ab0cd3b) SHA1(171b587bd5a947a13f3114120b6e7baca3b57d78)) // need verification

	ROM_REGION(0x4000, "hangul", 0)
	ROM_LOAD("88han.rom",  0x0000, 0x2000, BAD_DUMP CRC(938db440) SHA1(d41676fde0a3047792f93c4a41509b8749e55e74)) // need verification
	ROM_RELOAD(0x2000, 0x2000) // Are the contents really mirrored?
ROM_END

void msx_state::cpc88(machine_config &config)
{
	// AY-3-8910A
	// TMS9928A ?
	// FDC: None, 0 drives
	// 0 Cartridge slots
	// Expansion slot allows addition of cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "hangul", 0, 2, 1, "hangul");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 0, 4);   // 64KB RAM 

	m_hw_def.has_printer_port(false);
	msx1(TMS9928A, AY8910, config);
}

/* MSX - Daewoo DPC-100 */

ROM_START(dpc100)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("100bios.rom", 0x0000, 0x8000, CRC(3ab0cd3b) SHA1(171b587bd5a947a13f3114120b6e7baca3b57d78))

	ROM_REGION(0x4000, "hangul", 0)
	// should be 0x2000?
	ROM_LOAD("100han.rom",  0x0000, 0x4000, CRC(97478efb) SHA1(4421fa2504cbce18f7c84b5ea97f04e017007f07))
ROM_END

void msx_state::dpc100(machine_config &config)
{
	// GSS Z8400A PS
	// AY-3-8910A
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// 1 Expansion slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "hangul", 0, 2, 1, "hangul");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 3, 1);   // 16KB RAM
	// expansion slot is in slot #3

	msx1(TMS9918A, AY8910, config);
}

/* MSX - Daewoo DPC-180 */

ROM_START(dpc180)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("180bios.rom", 0x0000, 0x8000, CRC(3ab0cd3b) SHA1(171b587bd5a947a13f3114120b6e7baca3b57d78))

	ROM_REGION(0x4000, "hangul", 0)
	// should be 0x2000?
	ROM_LOAD("180han.rom",  0x0000, 0x4000, CRC(97478efb) SHA1(4421fa2504cbce18f7c84b5ea97f04e017007f07))
ROM_END

void msx_state::dpc180(machine_config &config)
{
	// AY-3-8910A
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// 1 Expansion slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "hangul", 0, 2, 1, "hangul");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 0, 4);   // 64KB RAM
	// Expansion slot is in slot #3

	msx1(TMS9918A, AY8910, config);
}

/* MSX - Daewoo DPC-200 */

ROM_START(dpc200)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("200bios.rom", 0x0000, 0x8000, CRC(3ab0cd3b) SHA1(171b587bd5a947a13f3114120b6e7baca3b57d78))

	ROM_REGION(0x4000, "hangul", 0)
	// should be 0x2000?
	ROM_LOAD("200han.rom",  0x0000, 0x4000, CRC(97478efb) SHA1(4421fa2504cbce18f7c84b5ea97f04e017007f07))
ROM_END

void msx_state::dpc200(machine_config &config)
{
	// GSS Z8400A PS cpu
	// AY-3-8910A
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// 1 Expansion slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "hangul", 0, 2, 1, "hangul");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 0, 4);  // 64KB RAM
	// Expansion slot is in slot #3

	msx1(TMS9918A, AY8910, config);
}

/* MSX - Daewoo DPC-200E (France) */

ROM_START(dpc200e)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("dpc200ebios.rom", 0x0000, 0x8000, CRC(d2110d66) SHA1(d3af963e2529662eae63f04a2530454685a1989f))
ROM_END

void msx_state::dpc200e(machine_config &config)
{
	// AY-3-8910
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// 1 Expansion slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 1, 0, 4);  // 64KB RAM
	add_cartridge_slot<1>(config, 2);
	// Expansion slot is in slot #3

	msx1(TMS9129, AY8910, config);
}

/* MSX - Daewoo Zemmix CPC-50 */

/* MSX - Daewoo Zemmix CPC-50A */

ROM_START(cpc50a)
	ROM_REGION(0x8000, "mainrom", 0)
	// HM6264LP-15 / U0422880 (ic4)
	// GMCE? VER1.01 (ic5)
	ROM_LOAD("50abios.rom", 0x0000, 0x8000, BAD_DUMP CRC(c3a868ef) SHA1(a08a940aa87313509e00bc5ac7494d53d8e03492)) // need verification
ROM_END

void msx_state::cpc50a(machine_config &config)
{
	// NEC uPD780C cpu
	// AY-3-8910A
	// DW64MX1
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// No keyboard
	// No cassette port
	// No printer port

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 3, 1).force_start_address(0xe000);  // 8KB RAM

	m_hw_def.has_cassette(false)
		.has_printer_port(false);
	msx1(TMS9118, AY8910, config);
}

/* MSX - Daewoo Zemmix CPC-50B */

ROM_START(cpc50b)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("50bbios.rom", 0x0000, 0x8000, BAD_DUMP CRC(c3a868ef) SHA1(a08a940aa87313509e00bc5ac7494d53d8e03492)) // need verification
ROM_END

void msx_state::cpc50b(machine_config &config)
{
	// AY-3-8910A
	// DW64MX1
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// No keyboard
	// No cassette port
	// No printer port

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 3, 1);  // 16KB RAM

	m_hw_def.has_cassette(false)
		.has_printer_port(false);
	msx1(TMS9118, AY8910, config);
}

/* MSX - Daewoo Zemmix CPC-51 */

ROM_START(cpc51)
	ROM_REGION(0x8000, "mainrom", 0)
	// Sticker: CPC-51 V 1.01 (ic05)
	ROM_LOAD("cpc-51_v_1_01.ic05", 0x0000, 0x8000, CRC(c3a868ef) SHA1(a08a940aa87313509e00bc5ac7494d53d8e03492))
ROM_END

void msx_state::cpc51(machine_config &config)
{
	// GSS Z8400A PS cpu
	// AY-3-8910A
	// FDC: None, 0 drives
	// DW64MX1
	// 1 Cartridge slot
	// No keyboard, just a keyboard connector
	// No cassette port
	// No printer port

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 0, 4);  // 64KB RAM

	m_hw_def.has_cassette(false)
		.has_printer_port(false);
	msx1(TMS9118, AY8910, config);
}

/* MSX - Daewoo Zemmix DTX-1493FW */

/* MSX - Dragon MSX-64 */

ROM_START(dgnmsx)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("uk1msx048.ic37", 0x0000, 0x4000, BAD_DUMP CRC(24c198be) SHA1(7f8c94cb8913db32a696dec80ffc78e46693f1b7)) // need verification
	ROM_LOAD("uk2msx058.ic6",  0x4000, 0x4000, BAD_DUMP CRC(e516e7e5) SHA1(05fedd4b9bfcf4949020c79d32c4c3f03a54fb62)) // need verification
ROM_END

void msx_state::dgnmsx(machine_config &config)
{
	// Sharp LH0080A cpu
	// AY-3-8910
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 0, 4);  // 64KB RAM
	add_cartridge_slot<2>(config, 3);

	msx1(TMS9129, AY8910, config);
}

/* MSX - Dynadata DPC-200 */
// GSS Z8400A PS
// AY-3-8910A
// 1 Cartridge slot
// 1 Expansion slot
// TMS9929A

/* MSX - Fenner DPC-200 */

ROM_START(fdpc200)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("dpc200bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(8205795e) SHA1(829c00c3114f25b3dae5157c0a238b52a3ac37db)) // need verification
ROM_END

void msx_state::fdpc200(machine_config &config)
{
	// AY-3-8910
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// 1 Expansion slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 1, 0, 4);  // 64KB RAM
	add_cartridge_slot<1>(config, 2);
	// Expansion slot

	msx1(TMS9929A, AY8910, config);
}

/* MSX - Fenner FPC-500 */

ROM_START(fpc500)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("fpc500bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(8205795e) SHA1(829c00c3114f25b3dae5157c0a238b52a3ac37db)) // need verification
ROM_END

void msx_state::fpc500(machine_config &config)
{
	// AY-3-8910
	// T6950 vdp
	// T7775 msx engine
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 4);  // 64KB RAM

	msx1(TMS9929A, AY8910, config);
}

/* MSX - Fenner SPC-800 */

ROM_START(fspc800)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("spc800bios.u7", 0x0000, 0x8000, CRC(e9ccd789) SHA1(8963fc041975f31dc2ab1019cfdd4967999de53e))
ROM_END

void msx_state::fspc800(machine_config &config)
{
	// GSS Z8400A
	// AY-3-8910A
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 4);  // 64KB RAM

	msx1(TMS9129, AY8910, config);
}

/* MSX - Frael Bruc 100-1 */

ROM_START(bruc100)
	ROM_REGION(0x10000, "mainrom", 0)
	ROM_LOAD("v1_1_mcl", 0x0000, 0x8000, CRC(c7bc4298) SHA1(3abca440cba16ac5e162b602557d30169f77adab))
	ROM_LOAD("f_v1_0", 0x8000, 0x2000, CRC(707a62b6) SHA1(e4ffe02abbda17986cb161c332e9e54d24fd053c))
	ROM_RELOAD(0xa000, 0x2000)
ROM_END

void bruc100_state::bruc100(machine_config &config)
{
	// AY-3-8910
	// FDC: None, 0 drives
	// Non-standard cassette port
	// 0 Cartridge slots
	// 1 Expansion slot

	add_internal_slot(config, MSX_SLOT_BRUC100, "firm", 0, 0, 2, "mainrom");
	// Expansion slot in slot 1
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 0, 4).set_total_size(0x10000);   // 64KB RAM

	msx1(TMS9129, AY8910, config);
	m_maincpu->set_addrmap(AS_IO, &bruc100_state::io_map);
}

/* MSX - Frael Bruc 100-2 */
ROM_START(bruc100a)
	ROM_REGION(0x10000, "mainrom", 0)
	ROM_LOAD("bruc100-2bios.rom", 0x0000, 0x8000, CRC(24464a7b) SHA1(88611b54cdbb79aa5380570f3dfef8b3a1cc2057))
	// v1.3
	ROM_SYSTEM_BIOS(0, "v13", "v1.3 firmware")
	ROMX_LOAD("bruc100_2_firmware1.3.rom", 0x8000, 0x8000, CRC(286fd001) SHA1(85ab6946950d4e329d5703b5defcce46cd96a50e), ROM_BIOS(0))
	// v1.2
	ROM_SYSTEM_BIOS(1, "v12", "v1.2 firmware")
	ROMX_LOAD("c_v1_2.u8", 0x8000, 0x8000, CRC(de12f81d) SHA1(f92d3aaa314808b7a9a14c40871d24f0d558ea00), ROM_BIOS(1))
	// v1.1
	ROM_SYSTEM_BIOS(2, "v11", "v1.1 firmware")
	// Firmware 1.1 - 8kb bootlogo rom (mirrored to 2000-3fff)
	ROMX_LOAD("bruc100_2_firmware1.2.rom", 0x8000, 0x2000, CRC(54d60863) SHA1(b4c9a06054cda5fd31311a79cc06e6f018cf828f), ROM_BIOS(2))
	ROMX_LOAD("bruc100_2_firmware1.2.rom", 0xa000, 0x2000, CRC(54d60863) SHA1(b4c9a06054cda5fd31311a79cc06e6f018cf828f), ROM_BIOS(2))
ROM_END

void bruc100_state::bruc100a(machine_config &config)
{
	// AY-3-8910
	// FDC: None, 0 drives
	// Non-standard cassette port
	// 1 Cartridge slot
	// 1 Expansion slot

	add_internal_slot(config, MSX_SLOT_BRUC100, "firm", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 1, 0, 4);   // 64KB RAM
	add_cartridge_slot<1>(config, 2);
	// Expansion slot in slot 3

	msx1(TMS9129, AY8910, config);
	m_maincpu->set_addrmap(AS_IO, &bruc100_state::io_map);
}

/* MSX - Fujitsu FM-X */

ROM_START(fmx)
	ROM_REGION(0x8000, "mainrom", 0)
	// mb62h010 ?
	ROM_LOAD("fmxbios.rom", 0x0000, 0x8000, CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821))
ROM_END

void msx_state::fmx(machine_config &config)
{
	// 21.4772Mhz
	// AY-3-8910A
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// 1 "Fujitsu expansion" slot for MB22450 (to connect FM-X to FM7) or MB22451 (printer port + 16KB ram)

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 3, 1); // 16KB RAM
	// Fujitsu expansion slot #1 in slot 1
	add_cartridge_slot<1>(config, 2);

	m_hw_def.has_printer_port(false);
	msx1(TMS9928A, AY8910, config);
}

/* MSX - General PCT-50 */

/* MSX - General PCT-55 */

/* MSX - Goldstar FC-80 */

/* MSX - Goldstar FC-80U */

ROM_START(gsfc80u)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("fc80ubios.rom", 0x0000, 0x8000, CRC(3ab0cd3b) SHA1(171b587bd5a947a13f3114120b6e7baca3b57d78))

	ROM_REGION(0x4000, "hangul", 0)
	ROM_LOAD("fc80uhan.rom",  0x0000, 0x2000, CRC(0cdb8501) SHA1(58dbe73ae80c2c409e766c3ace730ecd7bec89d0))
	ROM_RELOAD(0x2000, 0x2000)
ROM_END

void msx_state::gsfc80u(machine_config &config)
{
	// AY-3-8910A
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// 1 Expansion slot
	// Hangul LED

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "hangul", 0, 2, 1, "hangul");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 0, 4);  // 64KB RAM
	// Expansion slot in slot #3

	msx1(TMS9918A, AY8910, config);
}

/* MSX - Goldstar FC-200 */

ROM_START(gsfc200)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("fc200bios.rom.u5a", 0x0000, 0x4000, CRC(61f473fb) SHA1(c425750bbb2ae1d278216b45029d303e37d8df2f))
	ROM_LOAD("fc200bios.rom.u5b", 0x4000, 0x4000, CRC(1a99b1a1) SHA1(e18f72271b64693a2a2bc226e1b9ebd0448e07c0))
ROM_END

void msx_state::gsfc200(machine_config &config)
{
	// AY-3-8910A
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// 1 Expansion slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 0, 4);  // 64KB RAM
	// Expansion slot in slot #3

	msx1(TMS9129, AY8910, config);
}

/* MSX - Goldstar GFC-1080 */

ROM_START(gfc1080)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("gfc1080bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(d9cdd4a6) SHA1(6b0be712b9c95c1e912252ab5703e1c0bc457d9e)) // need verification

	ROM_REGION(0x4000, "hangul", 0)
	ROM_LOAD("gfc1080han.rom", 0x0000, 0x4000, BAD_DUMP CRC(f209448c) SHA1(141b44212ba28e7d03e0b54126fedd9e0807dc42)) // need verification

	ROM_REGION(0x4000, "pasocalc", 0)
	ROM_LOAD("gfc1080pasocalc.rom", 0x0000, 0x4000, BAD_DUMP CRC(4014f7ea) SHA1(a5581fa3ce10f90f15ba3dc53d57b02d6e4af172)) // need verification
ROM_END

void msx_state::gfc1080(machine_config &config)
{
	// AY8910/YM2149?
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// 1 Expansion slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "hangul", 0, 2, 1, "hangul");
	add_internal_slot(config, MSX_SLOT_ROM, "pasocalc", 0, 3, 1, "pasocalc");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 0, 4); // 64KB RAM
	// Expansion slot in slot #3

	msx1(TMS9928A, AY8910, config);
}

/* MSX - Goldstar GFC-1080A */

ROM_START(gfc1080a)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("gfc1080abios.rom", 0x0000, 0x8000, BAD_DUMP CRC(3ab0cd3b) SHA1(171b587bd5a947a13f3114120b6e7baca3b57d78)) // need verification

	ROM_REGION(0x4000, "hangul", 0)
	ROM_LOAD("gfc1080ahan.rom", 0x0000, 0x2000, BAD_DUMP CRC(0cdb8501) SHA1(58dbe73ae80c2c409e766c3ace730ecd7bec89d0)) // need verification
	ROM_RELOAD(0x2000, 0x2000)
ROM_END

void msx_state::gfc1080a(machine_config &config)
{
	// AY-3-8910A
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// 1 Expansion slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "hangul", 0, 2, 1, "hangul");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 0, 4); // 64KB RAM
	// Expansion slot in slot #3

	msx1(TMS9918A, AY8910, config);
}

/* MSX - Gradiente Expert 1.3 - source? */

ROM_START(expert13)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("expbios13.rom", 0x0000, 0x8000, BAD_DUMP CRC(5638bc38) SHA1(605f5af3f358c6811f54e0173bad908614a198c0)) // need verification
ROM_END

void msx_state::expert13(machine_config &config)
{
	// AY8910/YM2149?
	// FDC: None, 0 drives
	// 2 Cartridge slots?

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 2, 0, 4).set_total_size(0x10000);   // 64KB Mapper RAM
	add_cartridge_slot<2>(config, 3);

	msx1(TMS9928A, AY8910, config);
}

/* MSX - Gradiente Expert DDPlus */
ROM_START(expertdp)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("eddpbios.rom", 0x0000, 0x8000, CRC(efb4b972) SHA1(d6720845928ee848cfa88a86accb067397685f02))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("eddpdisk.rom", 0x0000, 0x4000, CRC(549f1d90) SHA1(f1525de4e0b60a6687156c2a96f8a8b2044b6c56))
ROM_END

void msx_state::expertdp(machine_config &config)
{
	// T7766A (AY-3-8910A compatible, integrated in T7937A)
	// FDC: mb8877a, 1 3.5" DSDD drive
	// non-standard cassette port
	// non-standard printer port
	// 2 Cartridge slots
	// T6950 (integrated in T7937A)
	// MSX Engine T7937A (also VDP)

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 0, 4);  // 64KB RAM
	add_internal_disk_mirrored(config, MSX_SLOT_DISK2, "diskrom", 3, 3, 1, 2, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");

	msx_mb8877a(config);
	msx_1_35_dd_drive(config);
	msx1(TMS9928A, AY8910, config);
}

/* MSX - Gradiente Expert Plus */

ROM_START(expertpl)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("exppbios.rom", 0x0000, 0x8000, CRC(efb4b972) SHA1(d6720845928ee848cfa88a86accb067397685f02))

	ROM_REGION(0x4000, "demo", 0)
	ROM_LOAD("exppdemo.rom", 0x0000, 0x4000, CRC(a9bbef64) SHA1(d4cea8c815f3eeabe0c6a1c845f902ec4318bf6b))
ROM_END

void msx_state::expertpl(machine_config &config)
{
	// T7766A (AY-3-8910 compatible in T7937A)
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// MSX Engine T7937A (with T6950 VDP and T7766A psg)

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 0, 4);  // 64KB RAM
	add_internal_slot(config, MSX_SLOT_ROM, "demo", 3, 3, 2, 1, "demo");

	msx1(TMS9928A, AY8910, config);
}

/* MSX - Gradiente Expert XP-800 (1.0) */

ROM_START(expert10)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("expbios.rom", 0x0000, 0x8000, CRC(07610d77) SHA1(ef3e010eb57e4476700a3bbff9d2119ab3acdf62))
ROM_END

void msx_state::expert10(machine_config &config)
{
	// AY8910/YM2149?
	// FDC: None, 0 drives
	// non-standard cassette port
	// non-standard printer port
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 0, 4);  // 64KB RAM
	// when no cartridge is inserted the expansion slot can be used in this slot
	add_cartridge_slot<2>(config, 3);

	msx1(TMS9128, AY8910, config);
}

/* MSX - Gradiente Expert XP-800 (1.1) / GPC-1 */
ROM_START(expert11)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("expbios11.rom", 0x0000, 0x8000, CRC(efb4b972) SHA1(d6720845928ee848cfa88a86accb067397685f02))
ROM_END

void msx_state::expert11(machine_config &config)
{
	// AY8910/YM2149?
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 0, 4);  // 64KB RAM
	// when no cartridge is inserted the expansion slot can be used in this slot
	add_cartridge_slot<2>(config, 3);

	msx1(TMS9128, AY8910, config);
}

/* MSX - Hitachi MB-H1 */

ROM_START(mbh1)
	ROM_REGION(0xc000, "mainrom", 0)
	ROM_LOAD("mbh1bios.rom", 0x0000, 0x8000, CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821))

	ROM_REGION(0x4000, "firmware", 0)
	ROM_LOAD("mbh1firm.rom", 0x0000, 0x2000, CRC(83f5662b) SHA1(3e005832138ffde8b1c36025754f81c2112b236d))
ROM_END

void msx_state::mbh1(machine_config &config)
{
	// AY-3-8910
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// Speed controller (normal, slow 1, slow 2)
	// Firmware should be bypassed when a cartridge is inserted

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 2, 2); // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 3, 1, 1, "firmware");

	msx1(TMS9918A, AY8910, config);
}

/* MSX - Hitachi MB-H1E */

ROM_START(mbh1e)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("mbh1bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821)) // need verification
ROM_END

void msx_state::mbh1e(machine_config &config)
{
	// AY-3-8910
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// Speed controller (normal, slow 1, slow 2)

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 3, 1); // 16KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);

	msx1(TMS9918A, AY8910, config);
}

/* MSX - Hitachi MB-H2 */

ROM_START(mbh2)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("mbh2bios.rom", 0x0000, 0x8000, CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821))

	ROM_REGION(0x4000, "firmware", 0)
	ROM_LOAD("mbh2firm.rom", 0x0000, 0x4000, CRC(4f03c947) SHA1(e2140fa2e8e59090ecccf55b62323ea9dcc66d0b))
ROM_END

void msx_state::mbh2(machine_config &config)
{
	// AY-3-8910
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// Builtin cassette player
	// Speed controller (normal, slow 1, slow 2)

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 0, 2, 1, "firmware");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 4); // 64KB RAM

	msx1(TMS9928A, AY8910, config);
}

/* MSX - Hitachi MB-H21 */

/* MSX - Hitachi MB-H25 */

ROM_START(mbh25)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("mbh25bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821)) // need verification
ROM_END

void msx_state::mbh25(machine_config &config)
{
	// AY-3-8910
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// Speed controller (normal, slow 1, slow 2)

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 2, 2); // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);

	msx1(TMS9118, AY8910, config);
}

/* MSX - Hitachi MB-H50 */

ROM_START(mbh50)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("mbh50bios.rom", 0x0000, 0x8000, CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821))
ROM_END

void msx_state::mbh50(machine_config &config)
{
	// YM2149 in S3527
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// T6950A
	// MSX-Engine S3527

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 2, 0, 4); // 64KB RAM

	msx1(TMS9928A, YM2149, config);
}

/* MSX - Hitachi MB-H80 (unreleased) */

/* MSX - In Tensai DPC-200CD */

/* MSX - JVC HC-7E / HC-7GB (different power supplies) */

ROM_START(jvchc7gb)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hc7gbbios.rom", 0x0000, 0x8000, CRC(e9ccd789) SHA1(8963fc041975f31dc2ab1019cfdd4967999de53e))
ROM_END

void msx_state::jvchc7gb(machine_config &config)
{
	// AY-3-8910A
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 0, 4);  // 64KB RAM
	add_cartridge_slot<2>(config, 3);

	msx1(TMS9929A, AY8910, config);
}

/* MSX - Jotan Holland Bingo */

/* MSX - Misawa-Van CX-5 */

/* MSX - Mitsubishi ML-8000 */

ROM_START(ml8000)
	ROM_REGION(0x8000, "mainrom", 0)
	// Same contents as standard 32kb bios with sha1 302afb5d8be26c758309ca3df611ae69cced2821
	// split across 4 eeproms
	ROM_LOAD("1.ic56", 0x0000, 0x2000, BAD_DUMP CRC(782e39fd) SHA1(ad20865df0d33ee5379b69be984302fb85d74c5a)) // need verification
	ROM_LOAD("2.ic32", 0x2000, 0x2000, BAD_DUMP CRC(d859cf14) SHA1(b6894ed3cd5be5a73a93fd02d275d6c1237310e6)) // need verification
	ROM_LOAD("3.ic26", 0x4000, 0x2000, BAD_DUMP CRC(b3211adf) SHA1(fad7be46d1137f636c23dc01e498cc38cff486e3)) // need verification
	ROM_LOAD("4.ic22", 0x6000, 0x2000, BAD_DUMP CRC(24e09092) SHA1(d6fdff0fa86a248ce319888275d1c634480b58c4)) // need verification
ROM_END

void msx_state::ml8000(machine_config &config)
{
	// NEC uPD780C
	// AY-3-8910
	// FDC: None, 0 drives
	// 1 Cartridge slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 2, 2); // 32KB RAM
	add_cartridge_slot<1>(config, 1);

	msx1(TMS9918A, AY8910, config);
}

/* MSX - Mitsubishi ML-F48 */

ROM_START(mlf48)
	ROM_REGION(0x8000, "mainrom", 0)
	// 4j3 hn613256p m82 ?
	ROM_LOAD("mlf48bios.ic2d", 0x0000, 0x8000, BAD_DUMP CRC(e9ccd789) SHA1(8963fc041975f31dc2ab1019cfdd4967999de53e)) // needs verification
ROM_END

void msx_state::mlf48(machine_config &config)
{
	// NEC uPD780C
	// AY-3-8910A
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 1, 2, 2); // 32KB RAM
	add_cartridge_slot<1>(config, 2);
	add_cartridge_slot<2>(config, 3);

	msx1(TMS9929A, AY8910, config); // videochip needs verification
}

/* MSX - Mitsubishi ML-F80 */

ROM_START(mlf80)
	ROM_REGION(0x8000, "mainrom", 0)
	// 4j1 hn613256p m82 ?
	ROM_LOAD("mlf80bios.ic2d", 0x0000, 0x8000, CRC(e9ccd789) SHA1(8963fc041975f31dc2ab1019cfdd4967999de53e))
ROM_END

void msx_state::mlf80(machine_config &config)
{
	// 21.3750MHz
	// AY-3-8910A
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 1, 0, 4);  // 64KB RAM
	add_cartridge_slot<1>(config, 2);
	add_cartridge_slot<2>(config, 3);

	msx1(TMS9929A, AY8910, config); // videochip needs verification
}

/* MSX - Mitsubishi ML-F110 */

ROM_START(mlf110)
	ROM_REGION(0x8000, "mainrom", 0)
	// 4j1 mn613256p d35
	ROM_LOAD("hn613256p.ic6d", 0x0000, 0x8000, CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821))
ROM_END

void msx_state::mlf110(machine_config &config)
{
	// NEC uPD780C
	// AY-3-8910A
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 1, 3, 1); // 16KB RAM
	add_cartridge_slot<1>(config, 2);
	add_cartridge_slot<2>(config, 3);

	msx1(TMS9918A, AY8910, config);
}

/* MSX - Mitsubishi ML-F120 / ML-F120D */

ROM_START(mlf120)
	ROM_REGION(0x8000, "mainrom", 0)
	// 4k3 hn613256p d35 ?
	// 904p874h11 ?
	ROM_LOAD("hn613256p.ic6d", 0x0000, 0x8000, CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821))

	ROM_REGION(0x4000, "firmware", 0)
	ROM_LOAD("t704p890h11.ic8d", 0x0000, 0x4000, CRC(4b5f3173) SHA1(21a9f60cb6370d0617ce54c42bb7d8e40a4ab560))
ROM_END

void msx_state::mlf120(machine_config &config)
{
	// NEC uPD780C
	// AY-3-8910A
	// FDC: None, 0 drives
	// 2 Cartridge slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 2, 2); // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 3, 1, 1, "firmware");

	msx1(TMS9918A, AY8910, config);
}

/* MSX - Mitsubishi ML-F120D (functionality wise same as ML-F120 but with RGB out instead of composite)
Different PCB from ML-F120:
21.4772Mhz
TMS9928A
AY-3-8910
3 ROMs? - labels unreadable, locations ic5?, ic7f, ic10f
 */

/* MSX - Mitsubishi ML-FX1 */

ROM_START(mlfx1)
	ROM_REGION(0x8000, "mainrom", 0)
	// 5c1 hn613256p t21 t704p874h21-u
	ROM_LOAD("mlfx1bios.ic6c", 0x0000, 0x8000, CRC(62867dce) SHA1(0cbe0df4af45e8f531e9c761403ac9e71808f20c))
ROM_END

void msx_state::mlfx1(machine_config &config)
{
	// NEC uPD780C
	// YM2149 (in S3527 MSX Engine)
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// MSX-Engine S3527

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 2, 0, 4);  // 64KB RAM

	msx1(TMS9929A, YM2149, config);
}

/* MSX - Mitsubishi ML-FX2 */

/* MSX - Mitsubishi ML-TS1 */

/* MSX - National CF-1200 */

ROM_START(cf1200)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("1200bios.rom", 0x0000, 0x8000, CRC(5ad03407) SHA1(c7a2c5baee6a9f0e1c6ee7d76944c0ab1886796c))
ROM_END

void msx_state::cf1200(machine_config &config)
{
	// AY8910, needs verification
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 3, 1);   // 16KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);

	msx1(TMS9918A, AY8910, config); // soundchip needs verification
}

/* MSX - National CF-2000 */

ROM_START(cf2000)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("2000bios.rom", 0x0000, 0x8000, CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821))
ROM_END

void msx_state::cf2000(machine_config &config)
{
	// AY-3-8910A, needs verification
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 3, 1);   // 16KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);

	msx1(TMS9928A, AY8910, config); // soundchip needs verification
}

/* MSX - National CF-2700 */

ROM_START(cf2700)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("2700bios.rom.ic32", 0x0000, 0x8000, CRC(5ad03407) SHA1(c7a2c5baee6a9f0e1c6ee7d76944c0ab1886796c))
ROM_END

void msx_state::cf2700(machine_config &config)
{
	// NEC upD780C
	// AY-3-8910A
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 2, 2);   // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);

	msx1(TMS9918A, AY8910, config);
}

/* MSX - National CF-3000 */

ROM_START(cf3000)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("3000bios.rom", 0x0000, 0x8000, CRC(5ad03407) SHA1(c7a2c5baee6a9f0e1c6ee7d76944c0ab1886796c))
ROM_END

void msx_state::cf3000(machine_config &config)
{
	// AY-3-8910
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 1, 0, 4);  // 64KB RAM
	add_cartridge_slot<1>(config, 2);
	add_cartridge_slot<2>(config, 3);

	msx1(TMS9928A, AY8910, config);
}

/* MSX - National CF-3300 */

ROM_START(cf3300)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("3300bios.rom", 0x0000, 0x8000, CRC(5ad03407) SHA1(c7a2c5baee6a9f0e1c6ee7d76944c0ab1886796c))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("3300disk.rom", 0x0000, 0x4000, CRC(549f1d90) SHA1(f1525de4e0b60a6687156c2a96f8a8b2044b6c56))
ROM_END

void msx_state::cf3300(machine_config &config)
{
	// AY8910/YM2149?
	// FDC: mb8877a, 1 3.5" SSDD drive
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 0, 4);  // 64KB RAM
	add_internal_disk_mirrored(config, MSX_SLOT_DISK2, "diskrom", 3, 1, 1, 2, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");

	msx_mb8877a(config);
	msx_1_35_ssdd_drive(config);
	msx1(TMS9928A, AY8910, config);
}

/* MSX - National FS-1300 */

ROM_START(fs1300)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("1300bios.rom", 0x0000, 0x8000, CRC(5ad03407) SHA1(c7a2c5baee6a9f0e1c6ee7d76944c0ab1886796c))
ROM_END

void msx_state::fs1300(machine_config &config)
{
	// AY8910/YM2149?
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 4);  // 64KB RAM

	msx1(TMS9918A, AY8910, config);
}

/* MSX - National FS-4000 */

ROM_START(fs4000)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("4000bios.rom",  0x0000, 0x8000, CRC(071135e0) SHA1(df48902f5f12af8867ae1a87f255145f0e5e0774))

	ROM_REGION(0x8000, "word", 0)
	ROM_LOAD("4000word.rom",  0x0000, 0x8000, CRC(950b6c87) SHA1(931d6318774bd495a32ec3dabf8d0edfc9913324))

	ROM_REGION(0x8000, "kdr", 0)
	ROM_LOAD("4000kdr.rom",  0x0000, 0x8000, CRC(ebaa5a1e) SHA1(77bd67d5d10d459d343e79eafcd8e17eb0f209dd))

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("4000kfn.rom", 0, 0x20000, CRC(956dc96d) SHA1(9ed3ab6d893632b9246e91b412cd5db519e7586b))
ROM_END

void msx_state::fs4000(machine_config &config)
{
	// YM2149 in S3527
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// MSX-Engine S3527

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "word", 3, 0, 0, 2, "word");
	add_internal_slot(config, MSX_SLOT_ROM, "kdr", 3, 1, 1, 2, "kdr");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 2, 0, 4);  // 64KB RAM

	msx1(TMS9128, YM2149, config);
}

/* MSX - National FS-4000 (Alt) */

ROM_START(fs4000a)
	ROM_REGION(0x8000 ,"mainrom", 0)
	ROM_LOAD("4000bios.rom",  0x0000, 0x8000, BAD_DUMP CRC(071135e0) SHA1(df48902f5f12af8867ae1a87f255145f0e5e0774)) // need verification

	ROM_REGION(0x8000, "word", 0)
	ROM_LOAD("4000wora.rom",  0x0000, 0x8000, BAD_DUMP CRC(52f4cdf7) SHA1(acbac3cb5b700254bed2cacc19fa54f1950f371d)) // need verification

	ROM_REGION(0x8000, "kdr", 0)
	ROM_LOAD("4000kdra.rom", 0x0000, 0x8000, BAD_DUMP CRC(b2db6bf5) SHA1(3a9a942ed888dd641cddf8deada1879c454df3c6)) // need verification

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("4000kfn.rom", 0, 0x20000, BAD_DUMP CRC(956dc96d) SHA1(9ed3ab6d893632b9246e91b412cd5db519e7586b)) // need verification
ROM_END

void msx_state::fs4000a(machine_config &config)
{
	// YM2149 in S3527
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// MSX-Engine S3527

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "word", 3, 0, 0, 2, "word");
	add_internal_slot(config, MSX_SLOT_ROM, "kdr", 3, 1, 1, 2, "kdr");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 2, 0, 4);  // 64KB RAM

	msx1(TMS9128, YM2149, config);
}

/* MSX - Network DPC-200 */

/* MSX - Olympia DPC-200 */
// GSS Z8400A PS
// AY-3-8910A
// ic8 ROM 9256C-0047 R09256C-INT

/* MSX - Olympia PHC-2 */

ROM_START(phc2)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("r09256c-fra.ic8", 0x0000, 0x8000, CRC(d2110d66) SHA1(d3af963e2529662eae63f04a2530454685a1989f))
ROM_END

void msx_state::phc2(machine_config &config)
{
	// AY-3-8910
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// 1 Expansion slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 1, 0, 4);  // 64KB RAM
	add_cartridge_slot<1>(config, 2);
	// Expansion slot in slot #3

	msx1(TMS9129, AY8910, config);
}

/* MSX - Olympia PHC-28 */

ROM_START(phc28)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("phc28bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(eceb2802) SHA1(195950173701abeb460a1a070d83466f3f53b337)) // need verification
ROM_END

void msx_state::phc28(machine_config &config)
{
	// AY8910/YM2149?
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// 1 Expansion slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 3, 1);   // 16KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	// Expansion slot in slot #3

	msx1(TMS9929A, AY8910, config); // soundchip and videochip need verification
}

/* MSX - Panasonic CF-2700 (Germany) */

ROM_START(cf2700g)
	ROM_REGION(0x8000, "mainrom", 0)
	// mn23257rfa
	ROM_LOAD("cf2700g.ic32", 0x0000, 0x8000, CRC(4aa194f4) SHA1(69bf27b610e11437dad1f7a1c37a63179a293d12))
ROM_END

void msx_state::cf2700g(machine_config &config)
{
	// NEC uPD780C
	// AY-3-8910A
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 1, 0, 4);  // 64KB RAM
	add_cartridge_slot<1>(config, 2);
	add_cartridge_slot<2>(config, 3);

	msx1(TMS9129, AY8910, config);
}

/* MSX - Panasonic CF-2700 (UK) */

ROM_START(cf2700uk)
	ROM_REGION(0x8000, "mainrom", 0)
	// mn23257rfa
	ROM_LOAD("cf2700uk.ic32", 0x0000, 0x8000, CRC(15e503de) SHA1(5e6b1306a30bbb46af61487d1a3cc1b0a69004c3))
ROM_END

void msx_state::cf2700uk(machine_config &config)
{
	// NEC uPD780C
	// AY-3-8910A
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 1, 0, 4);  // 64KB RAM
	add_cartridge_slot<1>(config, 2);
	add_cartridge_slot<2>(config, 3);

	msx1(TMS9129, AY8910, config);
}

/* MSX - Panasonic FS-3900 */

/* MSX - Philips NMS 800 */
// SGS Z8400AB1
// AY-3-8910A
// TMS9129
// U3 - ST27256-25CP
// 0 Cartridge slots
// No printer port

/* MSX - Philips NMS 801 */

ROM_START(nms801)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("801bios.rom", 0x0000, 0x8000, CRC(fa089461) SHA1(21329398c0f350e330b353f45f21aa7ba338fc8d))
ROM_END

void msx_state::nms801(machine_config &config)
{
	// SGS Z8400AB1
	// AY-3-8910A
	// FDC: None, 0 drives
	// 0 Cartridge slots
	// No printer port

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 4);  // 64KB RAM

	m_hw_def.has_printer_port(false);
	msx1(TMS9129, AY8910, config);
}

/* MSX - Philips VG-8000 */

ROM_START(vg8000)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("8000bios.rom", 0x0000, 0x8000, CRC(efd970b0) SHA1(42252cf87deeb58181a7bfec7c874190a1351779))
ROM_END

void msx_state::vg8000(machine_config &config)
{
	// SGS Z8400AB1
	// AY-3-8910
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// No printer port

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 3, 1);   // 16KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);

	m_hw_def.has_printer_port(false);
	msx1(TMS9129, AY8910, config);
}

/* MSX - Philips VG-8010 / VG-8010/00 */

ROM_START(vg8010)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("vg8000v1_0.663", 0x0000, 0x8000, CRC(efd970b0) SHA1(42252cf87deeb58181a7bfec7c874190a1351779))
ROM_END

void msx_state::vg8010(machine_config &config)
{
	// SGS Z8400AB1
	// AY-3-8910
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// No printer port

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 2, 2);   // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);

	m_hw_def.has_printer_port(false);
	msx1(TMS9129, AY8910, config);
}

/* MSX - Philips VG-8010F / VG-8010/19 */

ROM_START(vg8010f)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("8010fbios.663", 0x0000, 0x8000, CRC(df57c9ca) SHA1(898630ad1497dc9a329580c682ee55c4bcb9c30c))
ROM_END

void msx_state::vg8010f(machine_config &config)
{
	// SGS Z8400AB1
	// AY-3-8910
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// No printer port

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 2, 2);   // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);

	m_hw_def.has_printer_port(false);
	msx1(TMS9129, AY8910, config);
}

/* MSX - Philips VG-8020-00 */

ROM_START(vg802000)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("8020-00bios.rom", 0x0000, 0x8000, CRC(8205795e) SHA1(829c00c3114f25b3dae5157c0a238b52a3ac37db))
ROM_END

void msx_state::vg802000(machine_config &config)
{
	// YM2149
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 4);  // 64KB RAM

	msx1(TMS9929A, YM2149, config);
}

/* MSX - Philips VG-8020/19 / VG-8020F */

ROM_START(vg8020f)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("8020-19bios.u11", 0x0000, 0x8000, CRC(70ce2d45) SHA1(ae4a6632d4456ef44603e72f5acd5bbcd6c0d124))
ROM_END

void msx_state::vg8020f(machine_config &config)
{
	// YM2149 (in S-3527 MSX Engine)
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// S-3527 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 4);   // 64KB RAM

	msx1(TMS9929A, YM2149, config);
}

/* MSX - Philips VG-8020/20 */

ROM_START(vg802020)
	ROM_REGION(0x8000, "mainrom", 0)
	// m38256-k5 z-4 int rev1
	ROM_LOAD("8020-20bios.u11", 0x0000, 0x8000, CRC(a317e6b4) SHA1(e998f0c441f4f1800ef44e42cd1659150206cf79))
ROM_END

void msx_state::vg802020(machine_config &config)
{
	// Zilog Z8400APS
	// YM2149 (in S-3527 MSX Engine)
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// S-3527 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 4);   // 64KB RAM

	msx1(TMS9129, YM2149, config);
}

/* MSX - Phonola VG-8000 (Italian market, mostly likely same as Philips VG-8000) */

/* MSX - Phonola VG-8010 (Italian market, mostly likely same as Philips VG-8010) */

/* MSX - Phonola VG-8020 (Italian market, mostly likely same as Philips VG-8020) */

/* MSX - Pioneer PX-7 */

// BIOS is for an international keyboard while the machine has a Japanese layout
ROM_START(piopx7)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("ym2301.ic12", 0x0000, 0x8000, BAD_DUMP CRC(e9ccd789) SHA1(8963fc041975f31dc2ab1019cfdd4967999de53e))

	ROM_REGION(0x4000, "pbasic", 0)
	ROM_LOAD("pd5031.ic13", 0x0000, 0x2000, CRC(a5d0c0b9) SHA1(665d805f96616e1037f1823050657b7849899283)) // v1.0
	ROM_FILL(0x2000, 0x2000, 0x6e)
ROM_END

void msx_state::piopx7(machine_config &config)
{
	// TMS9928ANL VDP with sync/overlay interface
	// AY-3-8910 PSG
	// Pioneer System Remote (SR) system control interface
	// FDC: None, 0 drives
	// 2 Cartridge slots

	// Line-level stereo audio input can be mixed with sound output, balance controlled with slider on front panel
	// Front-panel switch allows audio input to be passed through bypassing the mixing circuit
	// Line input can be muted under software control, e.g. when loading data from Laserdisc
	// Right channel of line input is additionally routed via some signal processing to the cassette input for loading data from Laserdisc

	// PSG port B bits 0-5 can be used to drive controller pins 1-6, 1-7, 2-6, 2-7, 1-8 and 2-8 low if 0 is written

	// Slot #2 7FFE is the SR control register LCON
	// Bit 7 R = /ACK (significant with acknowledge 1->0 with respect to remote control signal transmission)
	// Bit 0 R = RMCLK (clock produced by dividing CLK1/CLK2 frequency by 128)
	// Bit 0 W = /REM (high output with bit serial data output generated in synchronisation with RMCLK)

	// Slot #2 7FFF is the video overlay control register VCON
	// Bit 7 R = /EXTV (low when external video input available; high when not available)
	// Bit 7 W = Mute (line input signal muting)
	// Bit 0 R = INTEXV (interrupt available when external video signal OFF, reset on read)
	// Bit 0 W = /OVERLAY (0 = superimpose, 1 = non-superimpose)

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 2, 2);   // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_ROM, "pbasic", 2, 1, 1, "pbasic");
	add_cartridge_slot<2>(config, 3);

	msx1(TMS9928A, AY8910, config);
}

/* MSX - Pioneer PX-7UK */

ROM_START(piopx7uk)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("px7ukbios.rom",   0x0000, 0x8000, CRC(e9ccd789) SHA1(8963fc041975f31dc2ab1019cfdd4967999de53e))

	ROM_REGION(0x4000, "pbasic", 0)
	ROM_LOAD("px7ukpbasic.rom", 0x0000, 0x2000, CRC(91e0df72) SHA1(4f0102cdc27216fd9bcdb9663db728d2ccd8ca6d)) // v1,1
	ROM_FILL(0x2000, 0x2000, 0x6e)

// Is this a cartridge that came with the machine?
// ROM_REGION(0x8000, "videoart", 0)
// ROM_LOAD("videoart.rom", 0x0000, 0x8000, CRC(0ba148dc) SHA1(b7b4e4cd40a856bb071976e6cf0f5e546fc86a78))
ROM_END

void msx_state::piopx7uk(machine_config &config)
{
	// YM2149
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 2, 2);   // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_ROM, "pbasic", 2, 1, 1, "pbasic");
	add_cartridge_slot<2>(config, 3);

	msx1(TMS9129, YM2149, config);
}

/* MSX - Pioneer PX-V60 */

ROM_START(piopxv60)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("pxv60bios.rom",   0x0000, 0x8000, CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821))

	ROM_REGION(0x4000, "pbasic", 0)
	ROM_LOAD("pd5031.rom", 0x0000, 0x2000, CRC(91e0df72) SHA1(4f0102cdc27216fd9bcdb9663db728d2ccd8ca6d)) // v1.1
	ROM_FILL(0x2000, 0x2000, 0x6E)
ROM_END

void msx_state::piopxv60(machine_config &config)
{
	// Sharp LH0080A
	// YM2149 in S3527
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// S3527

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 2, 2);   // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_ROM, "pbasic", 2, 1, 1, "pbasic");
	add_cartridge_slot<2>(config, 3);

	msx1(TMS9128, YM2149, config);
}

/* MSX - Pioneer PX-V7 */

/* MSX - Radiola MK 180 */

/* MSX - Sakhr AH-200 */

/* MSX - Sakhr AX-100 */

/* MSX - Sakhr AX-150 */

ROM_START(ax150)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("ax150bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(bd95c436) SHA1(5e094fca95ab8e91873ee372a3f1239b9a48a48d)) // need verification

	ROM_REGION(0x8000, "arabic", 0)
	ROM_LOAD("ax150arab.rom", 0x0000, 0x8000, BAD_DUMP CRC(339cd1aa) SHA1(0287b2ec897b9196788cd9f10c99e1487d7adbbb)) // need verification
ROM_END

void msx_state::ax150(machine_config &config)
{
	// YM2149 in S3527
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// YM2220 (compatible with TMS9918)
	// MSX-Engine S3527

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram1", 0, 0, 2, 2); // 32KB RAM
	add_internal_slot(config, MSX_SLOT_ROM, "arabic", 0, 1, 1, 2, "arabic");
	add_internal_slot(config, MSX_SLOT_RAM, "ram2", 0, 2, 0, 2); // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);

	msx1(TMS9918, YM2149, config);
}

/* MSX - Sakhr AX-170 */

ROM_START (ax170)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("ax170bios.rom", 0x0000, 0x8000, CRC(bd95c436) SHA1(5e094fca95ab8e91873ee372a3f1239b9a48a48d))

	ROM_REGION(0x8000, "arabic", 0)
	ROM_LOAD("ax170arab.rom", 0x0000, 0x8000, CRC(339cd1aa) SHA1(0287b2ec897b9196788cd9f10c99e1487d7adbbb))
ROM_END

void msx_state::ax170(machine_config &config)
{
	// AY-3-8910 in T7937
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// T6950 in T7937
	// T7937 (in ax170mk2)

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "arabic", 1, 1, 2, "arabic");
	add_cartridge_slot<1>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 0, 4); // 64KB RAM
	add_cartridge_slot<2>(config, 3, 3);

	msx1(TMS9929A, AY8910, config);
}

/* MSX - Sakhr AX-170F */

/* MSX - Sakhr AX-200 (Arabic/English) */

ROM_START (ax200)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("ax200bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(cae98b30) SHA1(079c018739c37485f3d64ef2145a0267fce6e20e)) // need verification

	ROM_REGION(0x8000, "arabic", 0)
	ROM_LOAD("ax200arab.rom", 0x0000, 0x8000, BAD_DUMP CRC(b041e610) SHA1(7574cc5655805ea316011a8123b064917f06f83c)) // need verification
ROM_END

void msx1_v9938_state::ax200(machine_config &config)
{
	// YM2149 in S3527
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// V9938
	// MSX Engine S3527

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram1", 0, 0, 2, 2); // 32KB RAM
	add_internal_slot(config, MSX_SLOT_RAM, "ram2", 0, 2, 0, 2); // 32KB RAM
	add_internal_slot(config, MSX_SLOT_ROM, "arabic", 0, 3, 1, 2, "arabic");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_cartridge_slot<3>(config, MSX_SLOT_YAMAHA_EXPANSION, "expansion", 3, msx_yamaha_60pin, nullptr);

	msx1_v9938_pal(YM2149, config);
}

/* MSX - Sakhr AX-200 (Arabic/French) */

/* MSX - Sakhr AX-200M (Arabic/English) */

ROM_START (ax200m)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("ax200bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(cae98b30) SHA1(079c018739c37485f3d64ef2145a0267fce6e20e)) // need verification

	ROM_REGION(0x8000, "arabic", 0)
	ROM_LOAD("ax200arab.rom", 0x0000, 0x8000, BAD_DUMP CRC(b041e610) SHA1(7574cc5655805ea316011a8123b064917f06f83c)) // need verification
ROM_END

void msx1_v9938_state::ax200m(machine_config &config)
{
	// YM2149 in S3527
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// V9938
	// MSX Engine S3527

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram1", 0, 0, 2, 2); // 32KB RAM
	add_internal_slot(config, MSX_SLOT_RAM, "ram2", 0, 2, 0, 2); // 32KB RAM
	add_internal_slot(config, MSX_SLOT_ROM, "arabic", 0, 3, 1, 2, "arabic");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	// Dumped unit had a SFG05 with version M5.00.011 rom
	add_cartridge_slot<3>(config, MSX_SLOT_YAMAHA_EXPANSION, "expansion", 3, msx_yamaha_60pin, "sfg05");

	msx1_v9938_pal(YM2149, config);
}

/* MSX - Sakhr AX-230 */

ROM_START (ax230)
	ROM_REGION(0x20000, "mainrom", 0)
	ROM_LOAD("qxxca0259.ic125", 0x0000, 0x20000, CRC(f1a3e650) SHA1(0340707c5de2310dcf5e569b7db4c6a6a5590cb7))

	ROM_REGION(0x100000, "games", 0)
	ROM_LOAD("qxxca0270.ic127", 0x00000, 0x100000, CRC(103c11c4) SHA1(620a209bdfdb65a22380031fce654bd1df61def2))
ROM_END

void msx_state::ax230(machine_config &config)
{
	// AY-3-8910 in T7937
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// T6950 in T7937
	// T7937

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	// TODO: is and if so, how is the rest of ic125 accessed?
	add_internal_slot(config, MSX_SLOT_ROM, "arabic1", 1, 1, 1, "mainrom", 0xc000);
	add_internal_slot(config, MSX_SLOT_ROM, "arabic2", 1, 2, 1, "mainrom", 0x8000);
	add_cartridge_slot<1>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 0, 4); // 64KB RAM
	add_internal_slot(config, MSX_SLOT_AX230, "games", 3, 3, 1, 2, "games");

	msx1(TMS9918, AY8910, config);
}

/* MSX - Sakhr AX-330 */

/* MSX - Sakhr AX-660 */

/* MSX - Sakhr AX-990 */

/* MSX - Salora MSX (prototypes) */

/* MSX - Samsung SPC-800 */

ROM_START(spc800)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("spc800bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(3ab0cd3b) SHA1(171b587bd5a947a13f3114120b6e7baca3b57d78)) // need verification

	ROM_REGION(0x4000, "hangul", 0)
	ROM_LOAD("spc800han.rom",  0x0000, 0x4000, BAD_DUMP CRC(5ae2b013) SHA1(1e7616261a203580c1044205ad8766d104f1d874)) // need verification
ROM_END

void msx_state::spc800(machine_config &config)
{
	// AY-3-8910
	// FDC: None, 0 drives
	// 1 Cartridge slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "hangul", 0, 2, 1, "hangul");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 0, 4);  // 64KB?? RAM
	// Expansion slot in slot #3

	msx1(TMS9118, AY8910, config);
}

/* MSX - Sanno PHC-SPC */

/* MSX - Sanno SPCmk-II */

/* MSX - Sanno SPCmk-III */

/* MSX - Sanyo MPC-1 / Wavy1 */

/* MSX - Sanyo MPC-10 / Wavy10 */

/* MSX - Sanyo MPC-64 */

ROM_START(mpc64)
	ROM_REGION(0x8000, "mainrom", 0)
	// hn613256p t22 5c1 ?
	ROM_LOAD("mpc64bios.ic111", 0x0000, 0x8000, BAD_DUMP CRC(d6e704ad) SHA1(d67be6d7d56d7229418f4e122f2ec27990db7d19)) // needs verification
ROM_END

void msx_state::mpc64(machine_config &config)
{
	// AY8910
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 4);  // 64KB RAM

	msx1(TMS9928A, AY8910, config);
}

/* MSX - Sanyo MPC-100 */

ROM_START(mpc100)
	ROM_REGION(0x8000, "mainrom", 0)
	// hn613256p 4j1 ?
	ROM_LOAD("mpc100bios.ic122", 0x0000, 0x8000, CRC(e9ccd789) SHA1(8963fc041975f31dc2ab1019cfdd4967999de53e))
ROM_END

void msx_state::mpc100(machine_config &config)
{
	// AY8910
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 4);  // 64KB RAM

	msx1(TMS9929A, AY8910, config);
}

/* MSX - Sanyo MPC-200 */

ROM_START(mpc200)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("mpc200bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(e9ccd789) SHA1(8963fc041975f31dc2ab1019cfdd4967999de53e)) // need verification
ROM_END

void msx_state::mpc200(machine_config &config)
{
	// AY8910
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// T6950
	// T7775 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "bios", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 4); // 64KB RAM

	msx1(TMS9929A, AY8910, config);
}

/* MSX - Sanyo MPC-200SP (same as Sanyo MPC-200 ?) */

ROM_START(mpc200sp)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("mpcsp200bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(bcd79900) SHA1(fc8c2b69351e60dc902add232032c2d69f00e41e)) // need verification
ROM_END

void msx_state::mpc200sp(machine_config &config)
{
	// AY8910/YM2149?
	// FDC: None, 0 drives
	// 2? Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 4); // 64KB RAM

	msx1(TMS9929A, AY8910, config);
}

/* MSX - Sanyo PHC-28L */

ROM_START(phc28l)
	ROM_REGION(0x8000, "mainrom", 0)
	// 5a1 hn613256p g42
	ROM_LOAD("28lbios.ic20", 0x0000, 0x8000, CRC(d2110d66) SHA1(d3af963e2529662eae63f04a2530454685a1989f))
ROM_END

void msx_state::phc28l(machine_config &config)
{
	// YM2149
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 4);  // 64KB RAM

	msx1(TMS9929A, YM2149, config);
}

/* MSX - Sanyo PHC-28S */

ROM_START(phc28s)
	ROM_REGION(0x8000, "mainrom", 0)
	// 4l1 hn613256p m74
	ROM_LOAD("28sbios.ic2", 0x0000, 0x8000, CRC(e5cf6b3c) SHA1(b1cce60ef61c058f5e42ef7ac635018d1a431168))
ROM_END

void msx_state::phc28s(machine_config &config)
{
	// 10.738MHz and 3.574545MHz
	// AY8910
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 2, 2);   // 32KB RAM

	msx1(TMS9929A, AY8910, config);
}

/* MSX - Sanyo Wavy MPC-10 */

ROM_START(mpc10)
	ROM_REGION(0x8000, "mainrom", 0)
	// Split across 2 roms? (image hard to read)
	// ic14? hn613256p d24 ?
	// ic15? hn613256p d25 ?
	ROM_LOAD("mpc10.rom", 0x0000, 0x8000, CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821))
ROM_END

void msx_state::mpc10(machine_config &config)
{
	// AY8910
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// 1 Expansion slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	// Expansion slot in slot #2
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 2, 2);   // 32KB RAM

	msx1(TMS9918, AY8910, config);
}

/* MSX - Schneider MC 810 */

/* MSX - Sharp Epcom HB-8000 (HotBit) */

ROM_START(hb8000)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_SYSTEM_BIOS(0, "v12", "v1.2")
	ROMX_LOAD("hotbit12.ic28", 0x0000, 0x8000, CRC(f59a4a0c) SHA1(9425815446d468058705bae545ffa13646744a87), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v11", "v1.1")
	ROMX_LOAD("hotbit11.ic28", 0x0000, 0x8000, CRC(b6942694) SHA1(663f8c512d04d213fa616b0db5eefe3774012a4b), ROM_BIOS(1))
	// v1.0 missing
ROM_END

void msx_state::hb8000(machine_config &config)
{
	// AY8910 and YM2149
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 4);  // 64KB RAM

	msx1(TMS9128, AY8910, config);
}

/* MSX - Sharp Epcom HB-8000 (HotBit 1.3b) */

ROM_START(hotbi13b)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hotbit13b.rom", 0x0000, 0x8000, BAD_DUMP CRC(7a19820e) SHA1(e0c2bfb078562d15acabc5831020a2370ea87052)) // need verification
ROM_END

void msx_state::hotbi13b(machine_config &config)
{
	// AY8910/YM2149?
	// FDC: None, 0 drives
	// 2 Cartridge slots?

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 0, 4).set_total_size(0x10000);   // 64KB Mapper RAM

	msx1(TMS9928A, AY8910, config);
}

/* MSX - Sharp Epcom HB-8000 (HotBit 1.3p) */

ROM_START(hotbi13p)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hotbit13p.rom", 0x0000, 0x8000, BAD_DUMP CRC(150e239c) SHA1(942f9507d206cd8156f15601fe8032fcf0e3875b)) // need verification
ROM_END

void msx_state::hotbi13p(machine_config &config)
{
	// AY8910/YM2149?
	// FDC: None, 0 drives
	// 2 Cartridge slots?

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 0, 4).set_total_size(0x10000);   // 64KB Mapper RAM

	msx1(TMS9928A, AY8910, config);
}

/* MSX - Sincorp SBX (Argentina, homebrew) */

/* MSX - Sony HB-10 */

ROM_START(hb10)
	ROM_REGION(0x8000, "mainrom", 0)
	// 5h3 hn613256p c78
	// 3l1 hn613256p c78 ?
	ROM_LOAD("hb10bios.ic12", 0x0000, 0x8000, CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821))
ROM_END

void msx_state::hb10(machine_config &config)
{
	// YM2149 (in S-S3527 MSX-Engine)
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// S3527 MSX-Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 3, 1);  // 16KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);

	msx1(TMS9118, YM2149, config);
}

/* MSX - Sony HB-10B */

/* MSX - Sony HB-10D */
// ic12 - tmm23256p 

/* MSX - Sony HB-10P */

ROM_START(hb10p)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("10pbios.ic12", 0x0000, 0x8000, CRC(0f488dd8) SHA1(5e7c8eab238712d1e18b0219c0f4d4dae180420d))
ROM_END

void msx_state::hb10p(machine_config &config)
{
	// XTAL: 3.579545 + 22.168(VDP)
	// YM2149 (in S3527 MSX-Engine)
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// T6950

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	// A mirror of RAM appears in slot #0, page #3
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 4);  // 64KB RAM

	msx1(TMS9929A, YM2149, config);
}

/* MSX - Sony HB-20P */

ROM_START(hb20p)
	ROM_REGION(0x8000, "mainrom", 0)
	// lh2359z3
	ROM_LOAD("20pbios.ic12", 0x0000, 0x8000, CRC(15ddeb5c) SHA1(63050d2d21214a721cc55f152c22b7be8061ac33))
ROM_END

void msx_state::hb20p(machine_config &config)
{
	// YM2149
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// T6950A

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	// A mirror of RAM appears in slot #0, page #3
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 4);  // 64KB RAM

	msx1(TMS9929A, YM2149, config);
}

/* MSX - Sony HB-201 */

ROM_START(hb201)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("201bios.ic9", 0x0000, 0x8000, CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821))

	ROM_REGION(0x4000, "firmware", 0)
	ROM_LOAD("201note.ic8", 0x0000, 0x4000, CRC(74567244) SHA1(0f4f09f1a6ef7535b243afabfb44a3a0eb0498d9))
ROM_END

void msx_state::hb201(machine_config &config)
{
	// YM2149
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 0, 2, 1, "firmware");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 4);  // 64KB RAM

	msx1(TMS9118, YM2149, config);
}

/* MSX - Sony HB-201P */

ROM_START(hb201p)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("201pbios.rom.ic9", 0x0000, 0x8000, CRC(0f488dd8) SHA1(5e7c8eab238712d1e18b0219c0f4d4dae180420d))

	ROM_REGION(0x4000, "firmware", 0)
	ROM_LOAD("201pnote.rom.ic8", 0x0000, 0x4000, CRC(1ff9b6ec) SHA1(e84d3ec7a595ee36b50e979683c84105c1871857))
ROM_END

void msx_state::hb201p(machine_config &config)
{
	// YM2149
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 0, 2, 1, "firmware");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 4);  // 64KB RAM

	msx1(TMS9129, YM2149, config);
}

/* MSX - Sony HB-501F */
// ic1 - tmm23256p
// YM2149
// TMS9129
// S3527

/* MSX - Sony HB-501P */

ROM_START(hb501p)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("501pbios.rom", 0x0000, 0x8000, CRC(0f488dd8) SHA1(5e7c8eab238712d1e18b0219c0f4d4dae180420d))
ROM_END

void msx_state::hb501p(machine_config &config)
{
	// YM2149
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// T6950
	// S3527

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 4);  // 64KB RAM

	msx1(TMS9929A, YM2149, config);
}

/* MSX - Sony HB-55 (Version 1) */

ROM_START(hb55)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hb55bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821)) // need verification

	ROM_REGION(0x4000, "firmware", 0)
	ROM_LOAD("hb55note.rom", 0x0000, 0x2000, BAD_DUMP CRC(5743ab55) SHA1(b9179db93608c4da649532e704f072e0a3ea1b22)) // need verification
ROM_END

void msx_state::hb55(machine_config &config)
{
	// AY8910
	// FDC: None, 0 drives
	// 1 Cartridge slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 0, 2, 1, "firmware");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 3, 1);   // 16KB RAM
	add_cartridge_slot<1>(config, 1);
	// Expansion slot in slot #3

	msx1(TMS9918A, AY8910, config);
}

/* MSX - Sony HB-55D, is this HB-55 2nd version? */

ROM_START(hb55d)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("55dbios.rom", 0x0000, 0x8000, BAD_DUMP CRC(7e2b32dd) SHA1(38a645febd0e0fe86d594f27c2d14be995acc730)) // need verification

	ROM_REGION(0x4000, "firmware", 0)
	ROM_LOAD("55dnote.rom", 0x0000, 0x4000, BAD_DUMP CRC(8aae0494) SHA1(97ce59892573cac3c440efff6d74c8a1c29a5ad3)) // need verification
ROM_END

void msx_state::hb55d(machine_config &config)
{
	// AY8910/YM2149?
	// FDC: None, 0 drives
	// 2 Cartridge slots?

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 0, 2, 1, "firmware", 0x8000);
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 3, 1);   // 16KB RAM
	add_cartridge_slot<2>(config, 3);

	msx1(TMS9929A, AY8910, config);
}

/* MSX - Sony HB-55P */

ROM_START(hb55p)
	ROM_REGION(0x8000, "mainrom", 0)
	// Are there machines with ic42 and ic43 populated like this?
	// Image on msx.org shows only ic42 and ic44 populated (for hb-55)
//	ROM_LOAD("55pbios.ic42", 0x0000, 0x4000, CRC(24c198be) SHA1(7f8c94cb8913db32a696dec80ffc78e46693f1b7))
//	ROM_LOAD("55pbios.ic43", 0x4000, 0x4000, CRC(e516e7e5) SHA1(05fedd4b9bfcf4949020c79d32c4c3f03a54fb62))
	ROM_LOAD("55pbios.ic42", 0x0000, 0x8000, CRC(e9ccd789) SHA1(8963fc041975f31dc2ab1019cfdd4967999de53e))

	ROM_REGION(0x4000, "firmware", 0)
	ROM_LOAD("55pnote.ic44", 0x0000, 0x4000, CRC(492b12f8) SHA1(b262aedc71b445303f84efe5e865cbb71fd7d952))
ROM_END

void msx_state::hb55p(machine_config &config)
{
	// AY8910
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 0, 2, 1, "firmware");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 3, 1);   // 16KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 3);

	msx1(TMS9929A, AY8910, config);
}

/* MSX - Sony HB-75 */

ROM_START(hb75)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("75bios.rom", 0x0000, 0x8000, CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821))

	ROM_REGION(0x4000, "firmware", 0)
	ROM_LOAD("75note.rom", 0x0000, 0x4000, CRC(2433dd0b) SHA1(5f26319aec3354a94e2a98e07b2c70046bc45417))
ROM_END

void msx_state::hb75(machine_config &config)
{
	// AY8910/YM2149?
	// FDC: None, 0 drives
	// 1 Cartridge slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 0, 2, 1, "firmware");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 0, 4);  // 64KB RAM
	// Expansion slot in slot #3

	msx1(TMS9918A, AY8910, config);
}

/* MSX - Sony HB-75AS */

/* MSX - Sony HB-75B */

/* MSX - Sony HB-75D */

ROM_START(hb75d)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("75dbios.rom", 0x0000, 0x8000, CRC(7e2b32dd) SHA1(38a645febd0e0fe86d594f27c2d14be995acc730))

	ROM_REGION(0x4000, "firmware", 0)
	ROM_LOAD("75dnote.rom", 0x0000, 0x4000, CRC(8aae0494) SHA1(97ce59892573cac3c440efff6d74c8a1c29a5ad3))
ROM_END

void msx_state::hb75d(machine_config &config)
{
	// AY8910/YM2149?
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 0, 2, 1, "firmware");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 0, 4);  // 64KB RAM
	add_cartridge_slot<2>(config, 3);

	msx1(TMS9929A, AY8910, config);
}

/* MSX - Sony HB-75F */

/* MSX - Sony HB-75P */

ROM_START(hb75p)
	ROM_REGION(0x8000, "mainrom", 0)
	// Are there machines with ic42 and ic43 populated like this?
	// HB-75P internal image on msx.org only has 2 roms populated (ic42 and ic44)
//	ROM_LOAD("75pbios.ic42", 0x0000, 0x4000, CRC(24c198be) SHA1(7f8c94cb8913db32a696dec80ffc78e46693f1b7))
//	ROM_LOAD("75pbios.ic43", 0x4000, 0x4000, CRC(e516e7e5) SHA1(05fedd4b9bfcf4949020c79d32c4c3f03a54fb62))
	ROM_LOAD("75pbios.ic42", 0x0000, 0x8000, CRC(e9ccd789) SHA1(8963fc041975f31dc2ab1019cfdd4967999de53e))

	ROM_REGION(0x4000, "firmware", 0)
	ROM_LOAD("75pnote.ic44", 0x0000, 0x4000, CRC(492b12f8) SHA1(b262aedc71b445303f84efe5e865cbb71fd7d952))
ROM_END

void msx_state::hb75p(machine_config &config)
{
	// AY8910
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 0, 2, 1, "firmware");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 0, 4);  // 64KB RAM
	add_cartridge_slot<2>(config, 3);

	msx1(TMS9929A, AY8910, config);
}

/* MSX - Sony HB-101 */

ROM_START(hb101)
	ROM_REGION(0x8000, "mainrom", 0)
	// 4l1 hn613256p d78
	ROM_LOAD("101pbios.ic108", 0x0000, 0x8000, CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821))

	ROM_REGION(0x4000, "firmware", 0)
	// m38128a-91 4202
	ROM_LOAD("101pnote.ic111", 0x0000, 0x4000, CRC(f62e75f6) SHA1(64adb7fcf9b86f59d8658badb02f58e61bb15712))
ROM_END

void msx_state::hb101(machine_config &config)
{
	// YM2149
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 3, 1);   // 16KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 3, 1, 1, "firmware");

	msx1(TMS9118, YM2149, config);
}

/* MSX - Sony HB-101P */

ROM_START(hb101p)
	ROM_REGION(0x8000, "mainrom", 0)
	// m38256-7b 5411
	ROM_LOAD("101pbios.ic9", 0x0000, 0x8000, CRC(0f488dd8) SHA1(5e7c8eab238712d1e18b0219c0f4d4dae180420d))

	ROM_REGION(0x4000, "firmware", 0)
	// m38128a-f6 5501
	ROM_LOAD("101pnote.ic8", 0x0000, 0x4000, CRC(525017c2) SHA1(8ffc24677fd9d2606a79718764261cdf02434f0a))
ROM_END

void msx_state::hb101p(machine_config &config)
{
	// YM2149
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 2, 2);   // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 3, 1, 1, "firmware");

	msx1(TMS9929A, YM2149, config);
}

/* MSX - Sony HB-701 */

/* MSX - Sony HB-701FD */

ROM_START(hb701fd)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hb701fdbios.rom", 0x0000, 0x8000, BAD_DUMP CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821)) // need verification

	ROM_REGION(0x4000, "diskrom", 0)
	// FDC register contents at 3ff8-3fff
	ROM_LOAD("hb701fddisk.rom", 0x0000, 0x4000, BAD_DUMP CRC(71961d9d) SHA1(2144036d6573d666143e890e5413956bfe8f66c5)) // need verification
ROM_END

void msx_state::hb701fd(machine_config &config)
{
	// YM2149
	// FDC: WD2793, 1 3.5" SSDD drive
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 0, 4);  // 64KB RAM
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1, "disk", 3, 1, 1, 2, "diskrom").set_tags("fdc", "fdc:0", "fdc:1"); // Is this correct??

	msx_wd2793_force_ready(config);
	msx_1_35_ssdd_drive(config);
	msx1(TMS9928A, YM2149, config);
}

/* MSX - Spectravideo SVI-728 */

ROM_START(svi728)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("728bios.rom", 0x0000, 0x8000, CRC(1ce9246c) SHA1(ea6a82cf8c6e65eb30b98755c8577cde8d9186c0))
ROM_END

void msx_state::svi728(machine_config &config)
{
	// AY8910
	// FDC: None, 0 drives
	// 1 Cartridge slots, 1 Expansion slot (eg for SVI-707)

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "mainmirror", 0, 2, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 1, 0, 4);  // 64KB RAM
	add_cartridge_slot<1>(config, 2);
	// Expansion slot (for eg SVI-707) in slot #3

	msx1(TMS9129, AY8910, config);
}

/* MSX - Spectravideo SVI-728 (Arabic) */

/* MSX - Spectravideo SVI-728 (Danish/Norwegian) */

/* MSX - Spectravideo SVI-728 (Spanish) */

ROM_START(svi728es)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("728esbios.rom", 0x0000, 0x8000, CRC(76c5e381) SHA1(82415ee031721d1954bfa42e1c6dd79d71c692d6))
ROM_END

/* MSX - Spectravideo SVI-728 (Swedish/Finnish) */

/* MSX - Spectravideo SVI-738 */

ROM_START(svi738)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("738bios.rom", 0x0000, 0x8000, CRC(ad007d62) SHA1(c53b3f2c00f31683914f7452f3f4d94ae2929c0d))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("738disk.rom", 0x0000, 0x4000, CRC(acd27a36) SHA1(99a40266bc296cef1d432cb0caa8df1a7e570be4))

	ROM_REGION(0x4000, "rs232rom", ROMREGION_ERASEFF)
	ROM_LOAD("738232c.rom", 0x0000, 0x2000, CRC(3353dcc6) SHA1(4e9384c9d137f0ab65ffc5a78f04cd8c9df6c8b7))
ROM_END

void msx1_v9938_state::svi738(machine_config &config)
{
	// AY8910
	// FDC: fd1793, 1 3.5" SSDD drive
	// 1 Cartridge slot
	// builtin 80 columns card (V9938)
	// RS-232C interface

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 1, 0, 4);  // 64KB RAM
	add_cartridge_slot<1>(config, 2);
	add_internal_slot_irq<2>(config, MSX_SLOT_RS232_SVI738, "rs232", 3, 0, 1, 1, "rs232rom");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK2, "disk", 3, 1, 1, 2, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");

	msx_fd1793_force_ready(config);
	msx_1_35_ssdd_drive(config);
	msx1_v9938_pal(AY8910, config);
}

/* MSX - Spectravideo SVI-738 Arabic */

ROM_START(svi738ar)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("738arbios.rom", 0x0000, 0x8000, BAD_DUMP CRC(ad007d62) SHA1(c53b3f2c00f31683914f7452f3f4d94ae2929c0d)) // need verification

	ROM_REGION(0x8000, "arab", 0)
	ROM_LOAD("738arab.rom",  0x0000, 0x8000, BAD_DUMP CRC(339cd1aa) SHA1(0287b2ec897b9196788cd9f10c99e1487d7adbbb)) // need verification

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("738ardisk.rom", 0x0000, 0x4000, BAD_DUMP CRC(acd27a36) SHA1(99a40266bc296cef1d432cb0caa8df1a7e570be4)) // meed verification

	ROM_REGION(0x4000, "rs232rom", ROMREGION_ERASEFF)
	ROM_LOAD("738ar232c.rom", 0x0000, 0x2000, BAD_DUMP CRC(3353dcc6) SHA1(4e9384c9d137f0ab65ffc5a78f04cd8c9df6c8b7)) // need verification
ROM_END

void msx1_v9938_state::svi738ar(machine_config &config)
{
	svi738(config);
	add_internal_slot(config, MSX_SLOT_ROM, "arab", 3, 3, 1, 2, "arab");
}

/* MSX - Spectravideo SVI-738 Danish/Norwegian */

ROM_START(svi738dk)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("738dkbios.rom", 0x0000, 0x8000, BAD_DUMP CRC(88720320) SHA1(1bda5af20cb86565bdc1ebd1e59a691fed7f9256)) // need verification

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("738dkdisk.rom", 0x0000, 0x4000, BAD_DUMP CRC(fb884df4) SHA1(6d3a530ae822ec91f6444c681c9b08b9efadc7e7)) // need verification

	ROM_REGION(0x4000, "rs232rom", ROMREGION_ERASEFF)
	ROM_LOAD("738dk232c.rom", 0x0000, 0x2000, BAD_DUMP CRC(3353dcc6) SHA1(4e9384c9d137f0ab65ffc5a78f04cd8c9df6c8b7)) // need verification
ROM_END

/* MSX - Spectravideo SVI-738 German */

/* MSX - Spectravideo SVI-738 Polish */

ROM_START(svi738pl)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("738plbios.rom", 0x0000, 0x8000, BAD_DUMP CRC(431b8bf5) SHA1(c90077ed84133a947841e07856e71133ba779da6)) // IC51 on board, need verification

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("738disk.rom",   0x0000, 0x4000, BAD_DUMP CRC(acd27a36) SHA1(99a40266bc296cef1d432cb0caa8df1a7e570be4)) // need verification

	ROM_REGION(0x4000, "rs232rom", ROMREGION_ERASEFF)
	ROM_LOAD("738232c.rom",   0x0000, 0x2000, BAD_DUMP CRC(3353dcc6) SHA1(4e9384c9d137f0ab65ffc5a78f04cd8c9df6c8b7)) // need verification
ROM_END

/* MSX - Spectravideo SVI-738 Spanish */

ROM_START(svi738sp)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("738spbios.rom", 0x0000, 0x8000, BAD_DUMP CRC(f0c0cbb9) SHA1(5f04d5799ed72ea4993e7c4302a1dd55ac1ea8cd)) // need verification

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("738spdisk.rom", 0x0000, 0x4000, BAD_DUMP CRC(fb884df4) SHA1(6d3a530ae822ec91f6444c681c9b08b9efadc7e7)) // need verification

	ROM_REGION(0x4000, "rs232rom", ROMREGION_ERASEFF)
	ROM_LOAD("738sp232c.rom", 0x0000, 0x2000, BAD_DUMP CRC(3353dcc6) SHA1(4e9384c9d137f0ab65ffc5a78f04cd8c9df6c8b7)) // need verification
ROM_END

/* MSX - Spectravideo SVI-738 Swedish/Finnish */

ROM_START(svi738sw)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("738sebios.rom", 0x0000, 0x8000, CRC(c8ccdaa0) SHA1(87f4d0fa58cfe9cef818a3185df2735e6da6168c))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("738sedisk.rom", 0x0000, 0x4000, CRC(fb884df4) SHA1(6d3a530ae822ec91f6444c681c9b08b9efadc7e7))

	ROM_REGION(0x4000, "rs232rom", ROMREGION_ERASEFF)
	ROM_LOAD("738se232c.rom", 0x0000, 0x2000, CRC(3353dcc6) SHA1(4e9384c9d137f0ab65ffc5a78f04cd8c9df6c8b7))
ROM_END

/* MSX - Talent DPC-200 */

// Spanish keyboard
ROM_START(tadpc200)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("dpc200bios.rom", 0x0000, 0x8000, CRC(01ad4fab) SHA1(d5bf4814ea694481c8badbb8de8d56a08ee03cc0))
ROM_END

// International keyboard
ROM_START(tadpc200b)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("dpc200altbios.rom", 0x0000, 0x8000, BAD_DUMP CRC(8205795e) SHA1(829c00c3114f25b3dae5157c0a238b52a3ac37db)) // need verification
ROM_END

void msx_state::tadpc200(machine_config &config)
{
	// AY8910
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// DW64MX1

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 1, 0, 4);  // 64KB RAM
	add_cartridge_slot<1>(config, 2);
	// Expansion slot in slot #3

	msx1(TMS9129, AY8910, config);
}

/* MSX - Talent DPC-200A */

ROM_START(tadpc200a)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("dpc200abios.rom", 0x0000, 0x8000, BAD_DUMP CRC(8205795e) SHA1(829c00c3114f25b3dae5157c0a238b52a3ac37db)) // need verification
ROM_END

void msx1_v9938_state::tadpc200a(machine_config &config)
{
	// YM2149
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// S1985

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 1, 0, 4);  // 64KB RAM
	add_cartridge_slot<1>(config, 2);
	// Expansion slot

	msx1_v9938_pal(YM2149, config);
}

/* MSX - Talent DPS-201 */

/* MSX - Toshiba HX-10 / HX-10P */

ROM_START(hx10)
	ROM_REGION(0x8000, "mainrom", 0)
	// tmm23256p
	ROM_LOAD("hx10bios.ic2", 0x0000, 0x8000, CRC(5486b711) SHA1(4dad9de7c28b452351cc12910849b51bd9a37ab3))
ROM_END

void msx_state::hx10(machine_config &config)
{
	// AY8910
	// FDC: None, 0 drives
	// 1 Cartridge slot, 1 Toshiba Expension slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 0, 4);  // 64KB RAM
	// Expansion slot in slot #3

	msx1(TMS9929A, AY8910, config);
}

/* MSX - Toshiba HX-10AA */

/* MSX - Toshiba HX-10D */

ROM_START(hx10d)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hx10dbios.ic5", 0x0000, 0x8000, CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821))
ROM_END

void msx_state::hx10d(machine_config &config)
{
	// AY8910
	// FDC: None, 0 drives
	// 1 Cartridge slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 0, 4);   // 64KB RAM
	// Expansion slot in slot #3

	m_hw_def.has_printer_port(false);
	msx1(TMS9918A, AY8910, config);
}

/* MSX - Toshiba HX-10DP */

ROM_START(hx10dp)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hx10dpbios.ic2", 0x0000, 0x8000, BAD_DUMP CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821)) // need verification
ROM_END

void msx_state::hx10dp(machine_config &config)
{
	// AY8910?
	// FDC: None, 0 drives
	// 1 Cartridge slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 0, 4);   // 64KB RAM
	// Expansion slot in slot #3

	msx1(TMS9918A, AY8910, config);
}

/* MSX - Toshiba HX-10DPN */

/* MSX - Toshiba HX-10E */

ROM_START(hx10e)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hx10ebios.rom", 0x0000, 0x8000, BAD_DUMP CRC(5486b711) SHA1(4dad9de7c28b452351cc12910849b51bd9a37ab3)) // need verification
ROM_END

void msx_state::hx10e(machine_config &config)
{
	// AY8910/YM2149?
	// FDC: None, 0 drives
	// 1 Cartridge slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 0, 4);   // 64KB RAM
	// Expansion slot in slot #3

	msx1(TMS9929A, AY8910, config);
}

/* MSX - Toshiba HX-10F */

ROM_START(hx10f)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hx10fbios.rom", 0x0000, 0x8000, BAD_DUMP CRC(e0e894b7) SHA1(d99eebded5db5fce1e072d08e642c0909bc7efdd)) // need verification
ROM_END

void msx_state::hx10f(machine_config &config)
{
	// AY8910?
	// FDC: None, 0 drives
	// 1 Cartridge slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 0, 4);   // 64KB RAM
	// Expansion slot in slot #3

	msx1(TMS9929A, AY8910, config);
}

/* MSX - Toshiba HX-10I */

/* MSX - Toshiba HX-10S */

// BIOS is for an international keyboard but the machine had a japanese keyboard layout so that cannot be correct
ROM_START(hx10s)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hx10sbios.rom", 0x0000, 0x8000, BAD_DUMP CRC(5486b711) SHA1(4dad9de7c28b452351cc12910849b51bd9a37ab3)) // need verification
ROM_END

void msx_state::hx10s(machine_config &config)
{
	// AY8910?
	// FDC: None, 0 drives
	// 1 Cartridge slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 3, 1);   // 16KB RAM
	add_cartridge_slot<1>(config, 1);
	// Expansion slot in slot #3

	m_hw_def.has_printer_port(false);
	msx1(TMS9918A, AY8910, config);
}

/* MSX - Toshiba HX-10SA */

ROM_START(hx10sa)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hx10sabios.rom", 0x0000, 0x8000, BAD_DUMP CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821)) // need verification
ROM_END

void msx_state::hx10sa(machine_config &config)
{
	// AY8910?
	// FDC: None, 0 drives
	// 1 Cartridge slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 3, 1);   // 16KB RAM
	add_cartridge_slot<1>(config, 1);
	// Expansion slot in slot #3

	m_hw_def.has_printer_port(false);
	msx1(TMS9918A, AY8910, config);
}

/* MSX - Toshiba HX-10SF */

/* MSX - Toshiba HX-20 */

// BIOS is for an internation keyboard but the machine had japanese keyboard layout
// Firmware is in English
ROM_START(hx20)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hx20bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(8205795e) SHA1(829c00c3114f25b3dae5157c0a238b52a3ac37db)) // need verification

	ROM_REGION(0x8000, "firmware", 0)
	ROM_LOAD("hx20word.rom", 0x0000, 0x8000, BAD_DUMP CRC(39b3e1c0) SHA1(9f7cfa932bd7dfd0d9ecaadc51655fb557c2e125)) // need verification
ROM_END

void msx_state::hx20(machine_config &config)
{
	// AY8910/YM2149?
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// HX-R701 RS-232 optional
	// T6950

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram1", 0, 2, 2);   // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram2", 3, 0, 0, 2);   // 32KB RAM
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 3, 3, 1, 2, "firmware");

	msx1(TMS9129, YM2149, config);
}

/* MSX - Toshiba HX-20AR */
// TMS9128

/* MSX - Toahiba HX-20E */

ROM_START(hx20e)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("tmm23256p_2023.ic2", 0x0000, 0x8000, CRC(8205795e) SHA1(829c00c3114f25b3dae5157c0a238b52a3ac37db))

	ROM_REGION(0x8000, "firmware", 0)
	ROM_LOAD("tc53257p_2047.ic3", 0x0000, 0x8000, CRC(39b3e1c0) SHA1(9f7cfa932bd7dfd0d9ecaadc51655fb557c2e125))
ROM_END

void msx_state::hx20e(machine_config &config)
{
	// YM2149
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// T6950

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram1", 0, 2, 2);   // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram2", 3, 0, 0, 2);   // 32KB RAM
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 3, 3, 1, 2, "firmware");

	msx1(TMS9129, YM2149, config);
}

/* MSX - Toshiba HX-20I */

ROM_START(hx20i)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hx20ibios.rom", 0x0000, 0x8000, BAD_DUMP CRC(8205795e) SHA1(829c00c3114f25b3dae5157c0a238b52a3ac37db)) // need verification

	ROM_REGION(0x8000, "firmware", 0)
	ROM_LOAD("hx20iword.rom", 0x0000, 0x8000, BAD_DUMP CRC(39b3e1c0) SHA1(9f7cfa932bd7dfd0d9ecaadc51655fb557c2e125)) // need verification
ROM_END

void msx_state::hx20i(machine_config &config)
{
	// AY8910/YM2149?
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// T6950

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram1", 0, 2, 2);   // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram2", 3, 0, 0, 2);   // 32KB RAM
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 3, 3, 1, 2, "firmware");

	msx1(TMS9129, YM2149, config);
}

/* MSX - Toshiba HX-21 */

ROM_START(hx21)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("tmm23256p_2011.ic2", 0x0000, 0x8000, CRC(83ba6fde) SHA1(01600d06d83072d4e757b29728555efde2c79705))

	ROM_REGION(0x8000, "firmware", 0)
	ROM_LOAD("tmm23256p_2014.ic3", 0x0000, 0x8000, CRC(87508e78) SHA1(4e2ec9c0294a18a3ab463f182f9333d2af68cdca))
ROM_END

void msx_state::hx21(machine_config &config)
{
	// AY8910
	// FDC: None, 0 drives
	// HX-R701 RS-232 optional
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram2", 3, 0, 0, 4);   // 64KB RAM
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 3, 3, 1, 2, "firmware");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware_mirror1", 3, 3, 0, 1, "firmware");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware_mirror2", 3, 3, 3, 1, "firmware", 0x4000);

	msx1(TMS9928A, AY8910, config);
}

/* MSX - Toshiba HX-21F */

ROM_START(hx21f)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hx21fbios.rom", 0x0000, 0x8000, BAD_DUMP CRC(8205795e) SHA1(829c00c3114f25b3dae5157c0a238b52a3ac37db)) // need verification

	ROM_REGION(0x8000, "firmware", 0)
	ROM_LOAD("hx21fword.rom", 0x0000, 0x8000, BAD_DUMP CRC(f9e29c66) SHA1(3289336b2c12161fd926a7e5ce865770ae7038af)) // need verification
ROM_END

void msx_state::hx21f(machine_config &config)
{
	// AY8910/YM2149?
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram1", 0, 2, 2);   // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram2", 3, 0, 2, 2);   // 32KB RAM
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 3, 3, 1, 2, "firmware");

	msx1(TMS9929A, AY8910, config);
}

/* MSX - Toshiba HX-22 */

ROM_START(hx22)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("tms23256p_2011.ic2", 0x0000, 0x8000, CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821))

	ROM_REGION(0x8000, "firmware", 0)
	ROM_LOAD("tms23256p_2014.ic3", 0x0000, 0x8000, CRC(87508e78) SHA1(4e2ec9c0294a18a3ab463f182f9333d2af68cdca))
ROM_END

void msx_state::hx22(machine_config &config)
{
	// AY8910
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// RS232C builtin

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram2", 3, 0, 0, 4);   // 64KB RAM
	add_internal_slot_irq<3>(config, MSX_SLOT_RS232_TOSHIBA, "firmware", 3, 3, 1, 2, "firmware");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware_mirror1", 3, 3, 0, 1, "firmware");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware_mirror2", 3, 3, 3, 1, "firmware", 0x4000);

	msx1(TMS9928A, AY8910, config);
}

/* MSX - Toshiba HX-22CH */

/* MSX - Toshibe HX-22GB */

/* MSX - Toshiba HX-22I */

ROM_START(hx22i)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("tmm23256p_2023.ic2", 0x0000, 0x8000, CRC(8205795e) SHA1(829c00c3114f25b3dae5157c0a238b52a3ac37db))

	ROM_REGION(0x8000, "firmware", 0)
	ROM_LOAD("tmm23256p_2046.ic3", 0x0000, 0x8000, CRC(f9e29c66) SHA1(3289336b2c12161fd926a7e5ce865770ae7038af))
ROM_END

void msx_state::hx22i(machine_config &config)
{
	// AY8910
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// RS232C builtin
	// Z80: LH0080A

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram1", 0, 2, 2);   // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram2", 3, 0, 2, 2);   // 32KB RAM
	add_internal_slot_irq<3>(config, MSX_SLOT_RS232_TOSHIBA, "firmware", 3, 3, 1, 2, "firmware");

	msx1(TMS9929A, AY8910, config);
}

/* MSX - Toshiba HX-30 */

/* MSX - Tosbiba HX-31 */

/* MSX - Toshiba HX-32 */

ROM_START(hx32)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hx32bios.ic3", 0x0000, 0x8000, CRC(83ba6fde) SHA1(01600d06d83072d4e757b29728555efde2c79705))

	ROM_REGION(0x8000, "firmware", 0)
	ROM_LOAD("hx32firm.ic2", 0x0000, 0x8000, CRC(efc3aca7) SHA1(ed589da7f359a4e139a23cd82d9a6a6fa3d70db0))

	// Same as HX-M200 Kanji cartridge
	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("hx32kfn.rom", 0x0000, 0x20000, CRC(d23d4d2d) SHA1(db03211b7db46899df41db2b1dfbec972109a967))
ROM_END

void msx_state::hx32(machine_config &config)
{
	// YM2149
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// HX-R701 RS-232 optional
	// T6950

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram2", 3, 0, 0, 4);   // 64KB RAM
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 3, 3, 1, 2, "firmware");

	msx1(TMS9928A, YM2149, config);
}

/* MSX - Toshiba HX-51I */

ROM_START(hx51i)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("tmm23256ad_2023.ic8", 0x0000, 0x8000, CRC(8205795e) SHA1(829c00c3114f25b3dae5157c0a238b52a3ac37db))
ROM_END

void msx_state::hx51i(machine_config &config)
{
	// AY8910 in T7937
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// T6950 in T7937
	// T7937

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram1", 0, 2, 2);   // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram2", 3, 0, 2);   // 32KB RAM

	msx1(TMS9918, AY8910, config);
}

/* MSX - Toshiba HX-52 */

/* MSX - Triton PC64 */

/* MSX - Vestel FC-200 */

/* MSX - Victor HC-5 */

ROM_START(hc5)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hc5bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821)) // need verification
ROM_END

void msx_state::hc5(machine_config &config)
{
	// YM2149
	// FDC: None, 0 drives,
	// 1 Cartridge slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 3, 1); // 16KB RAM
	add_cartridge_slot<1>(config, 1);
	// Expansion slot in slot #2
	// Module slot in slot #3

	msx1(TMS9928A, YM2149, config);
}

/* MSX - Victor HC-6 */

ROM_START(hc6)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hc6bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821)) // need verification
ROM_END

void msx_state::hc6(machine_config &config)
{
	// YM2149
	// FDC: None, 0 drives,
	// 1 Cartridge slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 2, 2); // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	// Expansion slot in slot #2
	// Module slot in slot #3

	msx1(TMS9928A, YM2149, config);
}

/* MSX - Victor HC-7 */

ROM_START(hc7)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hc7bios.rom", 0x0000, 0x8000, CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821))

	ROM_REGION(0x4000, "firmware", 0)
	ROM_LOAD("hc7firm.rom", 0x0000, 0x4000, CRC(7d62951d) SHA1(e211534064ea6f436f2e7458ded35c398f17b761))
ROM_END

void msx_state::hc7(machine_config &config)
{
	// AY8910
	// FDC: None, 0 drives,
	// 2 Cartridge slots?

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 0, 3, 1, "firmware");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 0, 4); // 64KB RAM
	add_cartridge_slot<2>(config, 3);

	msx1(TMS9928A, AY8910, config);
}

/* MSX - Victor HC-30 */

/* MSX - Victor HC-60 */

/* MSX - Wandy DPC-200 */

/* MSX - Yamaha CX5 */

/* MSX - Yamaha CX5F (with SFG01) */

ROM_START(cx5f1)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("cx5fbios.rom", 0x0000, 0x8000, CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821))
//	ROM_LOAD("cx5fbios.rom", 0x0000, 0x8000, CRC(dc662057) SHA1(36d77d357a5fd15af2ab266ee66e5091ba4770a3))
ROM_END

void msx_state::cx5f1(machine_config &config)
{
	// YM2149
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// 1 Yamaha expansion slot (50 pins)
	// 1 Yamaha module slot (60 pins)
	// S-5327 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 2, 2); // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	// Expansion bus in slot #2
	add_cartridge_slot<3>(config, MSX_SLOT_YAMAHA_EXPANSION, "module", 3, msx_yamaha_60pin, "sfg01");

	msx1(TMS9928A, YM2149, config);
}

/* MSX - Yamaha CX5F (with SFG05) */

ROM_START(cx5f)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("cx5fbios.rom", 0x0000, 0x8000, CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821))
//	ROM_LOAD("cx5fbios.rom", 0x0000, 0x8000, CRC(dc662057) SHA1(36d77d357a5fd15af2ab266ee66e5091ba4770a3))
ROM_END

void msx_state::cx5f(machine_config &config)
{
	// YM2149
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// 1 Yamaha expansion slot (50 pins)
	// 1 Yamaha module slot (60 pins)
	// S-5327 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 2, 2); // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	// Expansion bus in slot #2
	add_cartridge_slot<3>(config, MSX_SLOT_YAMAHA_EXPANSION, "module", 3, msx_yamaha_60pin, "sfg05");

	msx1(TMS9928A, YM2149, config);
}

/* MSX - Yamaha CX5MA (Australia / New Zealand */

/* MSX - Yamaha CX5MC (Canada) */

/* MSX - Yamaha CX5ME (UK) */

/* MSX - Yamaha CX5MF (France) */

/* MSX - Yamaha CX5MG (Germany) */

/* MSX - Yamaha CX5MS (Scandinavia) */

/* MSX - Yamaha CX5MU (USA) */

ROM_START(cx5mu)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("ym2301-23907.tmm23256p-2011", 0x0000, 0x8000, CRC(e2242b53) SHA1(706dd67036baeec7127e4ccd8c8db8f6ce7d0e4c))
ROM_END

void msx_state::cx5mu(machine_config &config)
{
	// YM2149
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// 1 Expansion slot (50 pins)
	// 1 Module slot (60 pins interface instead of regular 50 pin cartridge interface)

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 2, 2);   // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	// Expansion slot in slot #2
	add_cartridge_slot<3>(config, MSX_SLOT_YAMAHA_EXPANSION, "module", 3, msx_yamaha_60pin, "sfg01");

	msx1(TMS9918A, YM2149, config);
}

/* MSX - Yamaha CX5MII-128A (Australia, New Zealand) */

/* MSX - Yamaha CX5MII-128B (Italy) */

// Exact region unknown
ROM_START(cx5m128)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("cx5m128bios.rom", 0x0000, 0x8000, CRC(7973e080) SHA1(ea4a723cf098be7d7b40f23a7ab831cf5e2190d7))

	ROM_REGION(0x4000, "subrom", ROMREGION_ERASEFF)
	ROM_LOAD("cx5m128sub.rom",  0x0000, 0x2000, CRC(b17a776d) SHA1(c2340313bfda751181e8a5287d60f77bc6a2f3e6))

	ROM_REGION(0x4000, "minicart", 0)
	ROM_LOAD("yrm502.rom", 0x0000, 0x4000, CRC(5412d5dc) SHA1(30747a56f45389be76362f7fc55d673f1bff8312))
ROM_END

void msx1_v9938_state::cx5m128(machine_config &config)
{
	// YM2149 in S3527
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// 1 Mini cart slot (with YRM-502)
	// 1 Yamaha Module slot
	// S3527

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 1, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "minicart", 3, 1, 1, 1, "minicart"); /* YRM-502 */
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000);   // 128KB Mapper RAM
	add_cartridge_slot<3>(config, MSX_SLOT_YAMAHA_EXPANSION, "module", 3, 3, msx_yamaha_60pin, "sfg05");

	msx1_v9938_pal(YM2149, config);
}

/* MSX - Yamaha CX5MII-128 C (Canada) */

/* MSX - Yamaha CX5MII-128 E (UK) */

/* MSX - Yamaha CX5MII-128 F (France) */

/* MSX - Yamaha CX5MII-128 G (Germany) */

/* MSX - Yamaha CX5MII-128 P (Spain) */

/* MSX - Yamaha CX5MII-128 S (Scandinavia) */

/* MSX - Yamaha CX5MII-128 U (USA) */

/* MSX - Yamaha CX5MIIA (Australia, New Zealand) */

/* MSX - Yamaha CX5MIIB (Italy) */

ROM_START(cx5miib)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("cx5mii_basic-bios1.rom", 0x0000, 0x8000, CRC(507b2caa) SHA1(0dde59e8d98fa524961cd37b0e100dbfb42cf576))

	ROM_REGION(0x4000, "subrom", 0)
	// overdump?
	ROM_LOAD("cx5mii_sub.rom",  0x0000, 0x4000, BAD_DUMP CRC(317f9bb5) SHA1(0ce800666c0d66bc2aa0b73a16f228289b9198be))

	ROM_REGION(0x4000, "minicart", 0)
	ROM_LOAD("yrm502.rom", 0x0000, 0x4000, CRC(5330fe21) SHA1(7b1798561ee1844a7d6432924fbee9b4fc591c19))
ROM_END

void msx1_v9938_state::cx5miib(machine_config &config)
{
	// YM2149 in S3527
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// 1 Mini cartridge slot (with YRM-502)
	// 1 Module slot
	// S3527

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 1, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "minicart", 3, 1, 1, 1, "minicart"); /* YRM-502 */
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x10000);   // 64KB Mapper RAM
	add_cartridge_slot<3>(config, MSX_SLOT_YAMAHA_EXPANSION, "module", 3, 3, msx_yamaha_60pin, "sfg05");

	msx1_v9938_pal(YM2149, config);
}

/* MSX - Yamaha CX5MIIC (Canada) */

/* MSX - Yamaha CX5MIIE (UK) */

/* MSX - Yamaha CX5MIIF (France) */

/* MSX - Yamaha CX5MIIG (Germany) */

/* MSX - Yamaha CX5MIIP (Spain) */

/* MSX - Yamaha CX5MIIS (Scandinavia) */

/* MSX - Yamaha CX5MIIU (USA) */

/* MSX - Yamaha SX-100 */

ROM_START(sx100)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("sx100bios.rom", 0x0000, 0x8000, CRC(3b08dc03) SHA1(4d0c37ad722366ac7aa3d64291c3db72884deb2d))
ROM_END

void msx_state::sx100(machine_config &config)
{
	// YM2149 in S3527
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// YM2220, TMS9918 compatible
	// S3527

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 2, 2);   // 32KB RAM
	add_cartridge_slot<1>(config, 1);

	msx1(TMS9918, YM2149, config);
}

/* MSX - Yamaha YIS-303 */

// BIOS is for an international keyboard while the machine has a Japanese layout
ROM_START(yis303)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("ym2301-23907.tmm23256p-2011", 0x0000, 0x8000, BAD_DUMP CRC(e2242b53) SHA1(706dd67036baeec7127e4ccd8c8db8f6ce7d0e4c))
ROM_END

void msx_state::yis303(machine_config &config)
{
	// YM2149
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// 1 Yamaha expansion slot (50 pin)
	// 1 Yamaha module slot (60 pin)

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 3, 1);   // 16KB RAM
	add_cartridge_slot<1>(config, 1);
	// Expansion slot in slot #2
	// mounting sfg01 works, sfg05 does not
	add_cartridge_slot<3>(config, MSX_SLOT_YAMAHA_EXPANSION, "module", 3, 0, msx_yamaha_60pin, nullptr);

	m_hw_def.has_printer_port(false);
	msx1(TMS9918A, YM2149, config);
}

/* MSX - Yamaha YIS-503 */

ROM_START(yis503)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("yis503bios.rom", 0x0000, 0x8000, CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821))
ROM_END

void msx_state::yis503(machine_config &config)
{
	// YM2149
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// 1 Yamaha expansion slot
	// 1 Yamaha module slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 2, 2);   // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	// Expansion slot in slot #2
	add_cartridge_slot<3>(config, MSX_SLOT_YAMAHA_EXPANSION, "module", 3, msx_yamaha_60pin, nullptr);

	msx1(TMS9928A, YM2149, config);
}

/* MSX - Yamaha YIS-503F */

ROM_START(yis503f)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("yis503f.rom", 0x0000, 0x8000, CRC(e9ccd789) SHA1(8963fc041975f31dc2ab1019cfdd4967999de53e))
ROM_END

void msx_state::yis503f(machine_config &config)
{
	// YM2149
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// 1 Yamaha expansion slot (50 pin)
	// 1 Yamaha module slot (60 pin)

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 2, 2);  // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	// Expansion slot in slot #2
	add_cartridge_slot<3>(config, MSX_SLOT_YAMAHA_EXPANSION, "module", 3, msx_yamaha_60pin, nullptr);

	msx1(TMS9929A, YM2149, config);
}

/* MSX - Yamaha YIS-503II */

ROM_START(yis503ii)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("yis503iibios.rom", 0x0000, 0x8000, CRC(3b08dc03) SHA1(4d0c37ad722366ac7aa3d64291c3db72884deb2d))
ROM_END

void msx1_v9938_state::yis503ii(machine_config &config)
{
	// YM2149 in S3527
	// FDC: None, 0 drives
	// S3527
	// 2 Cartridge slots
	// 1 Yamaha module slot (60 pin)

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 2, 0, 4);  // 64KB RAM
	add_cartridge_slot<3>(config, MSX_SLOT_YAMAHA_EXPANSION, "module", 3, 3, msx_yamaha_60pin, nullptr);

	msx1_v9938(YM2149, config);
}

/* MSX - Yamaha YIS503-IIR Russian */

ROM_START(y503iir)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("yis503iirbios.rom", 0x0000, 0x8000, CRC(e751d55c) SHA1(807a823d4cac527c9f3758ed412aa2584c7f6d37))
// This is in the module slot by default
// ROM_LOAD("yis503iirnet.rom",  0xc000, 0x2000, CRC(0731db3f) SHA1(264fbb2de69fdb03f87dc5413428f6aa19511a7f))
ROM_END

void msx1_v9938_state::y503iir(machine_config &config)
{
	// YM2149 (in S-3527 MSX Engine)
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// 1 Mini cartridge slot
	// 1 Yamaha module slot
	// S-3527 MSX Engine
	// V9938 VDP

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	//  Mini cartridge slot in slot #3-1
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 2, 0, 4);  // 64KB RAM
	// This should have a serial network interface by default
	add_cartridge_slot<3>(config, MSX_SLOT_YAMAHA_EXPANSION, "module", 3, 3, msx_yamaha_60pin, nullptr);

	msx1_v9938_pal(YM2149, config);
}

/* MSX - Yamaha YIS503-IIR Estonian */

ROM_START(y503iir2)
	ROM_REGION(0x10000, "mainrom", 0)
	ROM_LOAD("yis503ii2bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(1548cee3) SHA1(42c7fff25b1bd90776ac0aea971241aedce8947d)) // need verification
// This is in the module slot by default
// ROM_LOAD("yis503iirnet.rom",  0xc000, 0x2000, CRC(0731db3f) SHA1(264fbb2de69fdb03f87dc5413428f6aa19511a7f))
ROM_END

void msx1_v9938_state::y503iir2(machine_config &config)
{
	// YM2149 (in S-3527 MSX Engine)
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// 1 Mini cartridge slot
	// 1 Yamaha module slot
	// S-3527 MSX Engine
	// V9938 VDP

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	//  Mini cartridge slot in slot #3-1
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 2, 0, 4);  // 64KB RAM
	// This should have a serial network interface by default
	add_cartridge_slot<3>(config, MSX_SLOT_YAMAHA_EXPANSION, "module", 3, 3, msx_yamaha_60pin, nullptr);

	msx1_v9938_pal(YM2149, config);
}

/* MSX - Yamaha YIS-603 */

/* MSX - Yashica YC-64 */

ROM_START(yc64)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("yc64bios.u20", 0x0000, 0x8000, CRC(e9ccd789) SHA1(8963fc041975f31dc2ab1019cfdd4967999de53e))
ROM_END

void msx_state::yc64(machine_config &config)
{
	// YM2149
	// FDC: None, 0 drives
	// 1 Cartridge slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 4);  // 64KB RAM

	msx1(TMS9929A, YM2149, config);
}

/* MSX - Yeno DPC-64 */

/* MSX - Yeno MX64 */

ROM_START(mx64)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("tmm23256p_7953.ic2d", 0x0000, 0x8000, BAD_DUMP CRC(e0e894b7) SHA1(d99eebded5db5fce1e072d08e642c0909bc7efdd)) // need verification
ROM_END

void msx_state::mx64(machine_config &config)
{
	// AY8910
	// FDC: None, 0 drives
	// 2 Cartridge slots

	// slot layout needs verification
	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 0, 4);  // 64KB RAM
	add_cartridge_slot<2>(config, 3);

	msx1(TMS9928A, AY8910, config);
}


/********************************  MSX 2 **********************************/

/* MSX2 - AVT CPC-300 (prototype) */

/* MSX2 - Bawareth Perfect MSX2 */

/* MSX2 - Canon V-25 */

ROM_START(canonv25)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("v25bios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("v25ext.rom", 0x0000, 0x4000, CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33))
ROM_END

void msx2_state::canonv25(machine_config &config)
{
	// YM2149 (in S3527 MSX Engine)
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// S3527
	// 64KB VRAM

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x10000); // 64KB Mapper RAM

	msx2(YM2149, config);
	msx2_64kb_vram(config);
}

/* MSX2 - Canon V-30F */

ROM_START(canonv30f)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("v30fbios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("v30fext.rom",  0x0000, 0x4000, CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("v30fdisk.rom", 0x0000, 0x4000, CRC(b499277c) SHA1(33117c47543a4eb00392cb9ff4e503004999a97a))
ROM_END

void msx2_state::canonv30f(machine_config &config)
{
	// YM2149 (in S3527 MSX Engine)
	// FDC: mb8877a, 1 3.5" DSDD drive
	// 2 Cartridge slots
	// S3527 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom"); // EXT
	add_internal_disk_mirrored(config, MSX_SLOT_DISK7, "disk", 3, 1, 1, 2, "diskrom").set_tags("fdc", "fdc:0", "fdc:1"); // DISK
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000); // 128KB Mapper RAM

	msx_mb8877a(config);
	msx_1_35_dd_drive(config);
	msx2(YM2149, config);
}

/* MSX2 - Daewoo CPC-300 */

ROM_START(cpc300)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("300bios.rom", 0x0000, 0x8000, CRC(53850907) SHA1(affa3c5cd8db79a1450ad8a7f405a425b251653d))

	ROM_REGION(0x8000, "subrom", 0)
	ROM_LOAD("300ext.rom", 0x0000, 0x8000, CRC(d64da39c) SHA1(fb51c505adfbc174df94289fa894ef969f5357bc))

	ROM_REGION(0x8000, "hangul", 0)
	ROM_LOAD("300han.rom", 0x0000, 0x8000, CRC(e78cd87f) SHA1(47a9d9a24e4fc6f9467c6e7d61a02d45f5a753ef))
ROM_END

void msx2_state::cpc300(machine_config &config)
{
	// YM2149 (in S-1985 MSX Engine)
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// 1 Expansion slot
	// S-1985 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "hangul", 0, 1, 1, 2, "hangul");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 0, 2, 0, 4).set_total_size(0x20000).set_ramio_bits(0x80);   // 128KB Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 3, 0, 2, "subrom");
	add_cartridge_slot<1>(config, 1);
	// Expansion slot in slot #3

	MSX_S1985(config, "s1985", 0);
	msx2(YM2149, config);
}

/* MSX2 - Daewoo CPC-300E */

ROM_START(cpc300e)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("300ebios.rom", 0x0000, 0x8000, CRC(53850907) SHA1(affa3c5cd8db79a1450ad8a7f405a425b251653d))

	ROM_REGION(0x8000, "subrom", 0)
	ROM_LOAD("300eext.rom",  0x0000, 0x8000, CRC(c52bddda) SHA1(09f7d788698a23aa7eec140237e907d4c37cbfe0))

	ROM_REGION(0x8000, "hangul", 0)
	ROM_LOAD("300ehan.rom", 0x0000, 0x8000, CRC(e78cd87f) SHA1(47a9d9a24e4fc6f9467c6e7d61a02d45f5a753ef))
ROM_END

void msx2_state::cpc300e(machine_config &config)
{
	// YM2149 in S1985
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// 1 Expansion slot
	// No clockchip
	// No joystick ports
	// S1985

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "hangul", 0, 1, 1, 2, "hangul");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 0, 2, 0, 4).set_total_size(0x20000).set_ramio_bits(0x80);   // 128KB Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 3, 0, 2, "subrom");
	add_cartridge_slot<1>(config, 1);
	// Expansion slot in slot #3

	MSX_S1985(config, "s1985", 0);
	msx2(YM2149, config);
}

/* MSX2 - Daewoo CPC-330K */

ROM_START(cpc330k)
	ROM_REGION(0x18000, "mainrom", 0)
	ROM_LOAD("330kbios.rom", 0x0000, 0x8000, BAD_DUMP CRC(53850907) SHA1(affa3c5cd8db79a1450ad8a7f405a425b251653d)) // need verification

	ROM_REGION(0x8000, "subrom", 0)
	ROM_LOAD("330kext.rom",  0x0000, 0x8000, BAD_DUMP CRC(5d685cca) SHA1(97afbadd8fe34ab658cce8222a27cdbe19bcef39)) // need verification

	ROM_REGION(0x8000, "hangul", 0)
	ROM_LOAD("330khan.rom", 0x0000, 0x4000, BAD_DUMP CRC(3d6dd335) SHA1(d2b058989a700ca772b9591f42c01ed0f45f74d6)) // need verification
ROM_END

void msx2_state::cpc330k(machine_config &config)
{
	// AY8910
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// 1 Expansion slot
	// DW64MX1
	// Ergonomic keyboard, 2 builtin game controllers

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "hangul", 0, 1, 1, 2, "hangul");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 0, 2, 0, 4).set_total_size(0x20000);   // 128KB Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 3, 0, 2, "subrom");
	add_cartridge_slot<1>(config, 1);

	msx2(AY8910, config);
}

/* MSX2 - Daewoo CPC-400 */

ROM_START(cpc400)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("400bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(53850907) SHA1(affa3c5cd8db79a1450ad8a7f405a425b251653d)) // need verification

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("400disk.rom", 0x0000, 0x4000, BAD_DUMP CRC(5fa517df) SHA1(914f6ccb25d78621186001f2f5e2aaa2d628cd0c)) // need verification

	ROM_REGION(0x8000, "subrom", 0)
	ROM_LOAD("400ext.rom",  0x0000, 0x8000, BAD_DUMP CRC(2ba104a3) SHA1(b6d3649a6647fa9f6bd61efc317485a20901128f)) // need verification

	ROM_REGION(0x8000, "hangul", 0)
	ROM_LOAD("400han.rom", 0x0000, 0x8000, BAD_DUMP CRC(a8ead5e3) SHA1(87936f808423dddfd00629056d6807b4be1dc63e)) // need verification

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("400kfn.rom", 0, 0x20000, BAD_DUMP CRC(b663c605) SHA1(965f4982790f1817bcbabbb38c8777183b231a55)) // need verification
ROM_END

void msx2_state::cpc400(machine_config &config)
{
	// AY8910
	// FDC: mb8877a, 1 3.5" DS?DD drive
	// 1 Cartridge slot
	// 1 Expansion slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "hangul", 0, 1, 1, 2, "hangul");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 0, 2, 0, 4).set_total_size(0x20000).set_ramio_bits(0x80);   // 128KB Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 3, 0, 2, "subrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_disk_mirrored(config, MSX_SLOT_DISK2, "disk", 2, 1, 2, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");
	// Expansion slot in slot #3

	msx_mb8877a(config);
	msx_1_35_dd_drive(config);
	msx2(AY8910, config);
}

/* MSX2 - Daewoo CPC-400S */

ROM_START(cpc400s)
	ROM_REGION(0x10000, "mainrom", 0)
	// bios, disk, and sub
	ROM_LOAD("400sbios.u38", 0x0000, 0x10000, CRC(dbb0f26d) SHA1(75f5f0a5a2e8f0935f33bb3cf07b83dd3e5f3347))

	ROM_REGION(0x10000, "hangul", 0)
	// hangul and kanji driver
	ROM_LOAD("400shan.u44", 0x0000, 0x10000, CRC(5fdb493c) SHA1(6b640c1d8cbeda6ca7d6facd16a206b62e059eee))

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("400skfn.rom", 0, 0x20000, CRC(fa85368c) SHA1(30fff22e3e3d464993707488442721a5e56a9707))
ROM_END

void msx2_state::cpc400s(machine_config &config)
{
	// AY8910
	// FDC: mb8877a, 1 3.5" DS?DD drive
	// 1 Cartridge slot
	// DW64MX1

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "hangul1", 0, 1, 1, 1, "hangul", 0x4000);
	add_internal_slot(config, MSX_SLOT_ROM, "kfn", 0, 1, 2, 1, "hangul", 0xc000);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 0, 2, 0, 4).set_total_size(0x20000).set_ramio_bits(0x80);   // 128KB Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 3, 0, 1, "mainrom", 0xc000);
	add_internal_slot(config, MSX_SLOT_ROM, "hangul2", 0, 3, 1, 1, "hangul");
	add_cartridge_slot<1>(config, 1);
	add_internal_disk_mirrored(config, MSX_SLOT_DISK2, "disk", 2, 1, 2, "mainrom", 0x8000).set_tags("fdc", "fdc:0", "fdc:1");
	// Expansion slot in slot #3

	msx_mb8877a(config);
	msx_1_35_dd_drive(config);
	msx2(AY8910, config);
}

/* MSX2 - Daewoo Zemmix CPC-61 */

ROM_START(cpc61)
	ROM_REGION(0x10000, "mainrom", 0)
	ROM_LOAD("cpc64122.rom", 0x0000, 0x10000, CRC(914dcad9) SHA1(dd3c39c8cfa06ec69f54c95c3b2291e3da7bd4f2))
ROM_END

void msx2_state::cpc61(machine_config &config)
{
	// AY8910
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// DW64MX1
	// No keyboard, but a keyboard connector
	// No printer port
	// No cassette port

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "hangul", 0, 1, 1, 1, "mainrom", 0xc000);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 0, 2, 0, 4).set_total_size(0x10000); // 64KB Mapper RAM?
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 3, 0, 1, "mainrom", 0x8000);
	add_cartridge_slot<1>(config, 1);

	m_hw_def.has_cassette(false)
		.has_printer_port(false);
	msx2(AY8910, config);
}

/* MSX2 - Daewoo Zemmix CPG-120 Normal */

ROM_START(cpg120)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("cpg120bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(b80c8e45) SHA1(310a02a9746bc062834e0cf2fabf7f3e0f7e829e)) // need verification

	ROM_REGION(0x8000, "subrom", 0)
	ROM_LOAD("cpg120ext.rom", 0x0000, 0x8000, BAD_DUMP CRC(b3d740b4) SHA1(7121c3c5ee6e4931fceda14a06f4c0e3b8eda437)) // need verification

	ROM_REGION(0x4000, "music", 0)
	ROM_LOAD("cpg128music.rom", 0x0000, 0x4000, BAD_DUMP CRC(73491999) SHA1(b9ee4f30a36e283a2b1b9a28a70ab9b9831570c6)) // need verification
ROM_END

void msx2_state::cpg120(machine_config &config)
{
	// By pressing the turbo button the CPU can switched between 3.579545 and 5.369317 MHz
	// YM2149 (in S1985)
	// FDC: None, 0 drives
	// 2 Cartridge slots?
	// S-1985 MSX Engine
	// V9958 VDP
	// FM built in
	// No keyboard, bot a keyboard connector?
	// No clock chip?
	// No printer port

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 0, 2, 0, 4).set_total_size(0x20000); // 128KB Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "saubrom", 0, 3, 0, 2, "subrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_MUSIC, "mus", 2, 1, 1, "music").set_ym2413_tag("ym2413");
	add_cartridge_slot<2>(config, 3);

	MSX_S1985(config, "s1985", 0);

	msx_ym2413(config);
	m_hw_def.has_printer_port(false);
	msx2plus(AY8910, config);
}

/* MSX2 - Daisen Sangyo MX-2021 */

/* MSX2 - Fenner FPC-900 */

ROM_START(fpc900)
	ROM_REGION(0x10000, "mainrom", 0)
	ROM_LOAD("fpc900bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(6cdaf3a5) SHA1(6103b39f1e38d1aa2d84b1c3219c44f1abb5436e)) // need verification

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("fpc900ext.rom", 0x0000, 0x4000, BAD_DUMP CRC(66237ecf) SHA1(5c1f9c7fb655e43d38e5dd1fcc6b942b2ff68b02)) // need verification

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("fpc900disk.rom", 0x0000, 0x4000, BAD_DUMP CRC(ca3307d3) SHA1(c3efedda7ab947a06d9345f7b8261076fa7ceeef)) // need verification
ROM_END

void msx2_state::fpc900(machine_config &config)
{
	// YM2149 (in S-3527 MSX Engine)
	// FDC: WD2793?, 1 3.5" DSDD drive
	// 2? Cartridge slots
	// S-3527 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x40000); // 256KB? Mapper RAM
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1, "disk", 3, 3, 1, 2, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");

	msx_wd2793_force_ready(config);
	msx_1_35_dd_drive(config);
	msx2_pal(YM2149, config);
}

/* MSX2 - GR8Bit (should probably be a separate driver) */

/* MSX2 - Gradiente Expert 2.0 */

ROM_START(expert20)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("exp20bios.rom", 0x0000, 0x8000, CRC(6bacdce4) SHA1(9c43106dba3ae2829e9a11dffa9d000ed6d6454c))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("exp20ext.rom", 0x0000, 0x4000, CRC(08ced880) SHA1(4f2a7e0172f0214f025f23845f6e053d0ffd28e8))

	ROM_REGION(0x4000, "xbasic", 0)
	ROM_LOAD("xbasic2.rom", 0x0000, 0x4000, CRC(2825b1a0) SHA1(47370bec7ca1f0615a54eda548b07fbc0c7ef398))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("microsoldisk.rom", 0x0000, 0x4000, CRC(6704ef81) SHA1(a3028515ed829e900cc8deb403e17b09a38bf9b0))
ROM_END

void msx2_state::expert20(machine_config &config)
{
	// AY8910/YM2149?
	// FDC: microsol, 1? 3.5"? DS?DD drive
	// 2 Cartridge slots?

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 1, 1, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "xbasic", 1, 1, 1, 1, "xbasic");
	add_internal_disk(config, MSX_SLOT_DISK5, "disk", 1, 3, 1, 1, "diskrom").set_tags("fdc", "fdc:0", "fdc:1", "fdc:2", "fdc:3");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 2, 0, 0, 4).set_total_size(0x20000).set_ramio_bits(0x80);   // 128KB Mapper RAM
	add_cartridge_slot<2>(config, 3);

	msx_microsol(config);
	msx_1_35_dd_drive(config);
	msx2_pal(AY8910, config);
}

/* MSX2 - Hitachi MB-H3 */

ROM_START(mbh3)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("mbh3bios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("mbh3sub.rom", 0x000, 0x4000, CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33))

	ROM_REGION(0x8000, "firmware", 0)
	ROM_LOAD("mbh3firm.rom", 0x0000, 0x8000, CRC(8c844173) SHA1(74ee82cc09ffcf78f6e9a3f0d993f8f80d81444c))
ROM_END

void msx2_state::mbh3(machine_config &config)
{
	// YM2149 in S3527
	// FDC: None, 0 drives
	// 64KB VRAM
	// Touchpad
	// 2 Cartrdige slots
	// S3527

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0 ,1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 3, 0, 1, 2, "firmware");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000); // 128KB Mapper RAM

	msx2(YM2149, config);
	msx2_64kb_vram(config);
}

/* MSX2 - Hitachi MB-H70 */

ROM_START(mbh70)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("mbh70bios.rom" , 0x0000, 0x8000, BAD_DUMP CRC(a27c563d) SHA1(c1e46c00f1e38fc9e0ab487bf0513bd93ce61f3f)) // need verification

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("mbh70ext.rom", 0x0000, 0x4000, BAD_DUMP CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33)) // need verification

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("mbh70disk.rom", 0x0000, 0x4000, BAD_DUMP CRC(05661a3f) SHA1(e695fc0c917577a3183901a08ca9e5f9c60b8317)) // need verification

	ROM_REGION(0x100000, "firmware", 0)
	ROM_LOAD("mbh70halnote.rom", 0x0000, 0x100000, BAD_DUMP CRC(40313fec) SHA1(1af617bfd11b10a71936c606174a80019762ea71)) // need verification

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("mbh70kfn.rom", 0x0000, 0x20000, BAD_DUMP CRC(d23d4d2d) SHA1(db03211b7db46899df41db2b1dfbec972109a967)) // need verification
ROM_END

void msx2_state::mbh70(machine_config &config)
{
	// YM2149 (in S-1985)
	// FDC: WD2793?, 2? 3.5" DSDD drives
	// S-1985 MSX Engine
	// 3 Cartridge slots?

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_SONY08, "firmware", 0, 3, 0, 4, "firmware");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1, "disk", 3, 0, 1, 2, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000); // 128KB Mapper RAM

	MSX_S1985(config, "s1985", 0);

	msx_wd2793(config);
	msx_2_35_dd_drive(config);
	msx2(YM2149, config);
}

/* MSX2 - Kawai KMC-5000 */

ROM_START(kmc5000)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("kmc5000bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd)) // need verification

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("kmc5000ext.rom", 0x0000, 0x4000, BAD_DUMP CRC(43e7a7fc) SHA1(0fbd45ef3dd7bb82d4c31f1947884f411f1ca344)) // need verification

	ROM_REGION(0x4000, "diskrom", 0)
	// FDC register contents at 3ff8-3fff
	ROM_LOAD("kmc5000disk.rom", 0x0000, 0x4000, BAD_DUMP CRC(e25cacca) SHA1(607cfca605eaf82e3efa33459d6583efb7ecc13b)) // need verification

	ROM_REGION(0x8000, "kdr", 0)
	ROM_LOAD("kmc5000kdr.rom", 0x0000, 0x8000, BAD_DUMP CRC(2dbea5ec) SHA1(ea35cc2cad9cfdf56cae224d8ee41579de37f000)) // need verification

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("kmc5000kfn.rom", 0, 0x20000, BAD_DUMP CRC(c61ddc5d) SHA1(5e872d5853698731a0ed22fb72dbcdfd59cd19c3)) // need verification
ROM_END

void msx2_state::kmc5000(machine_config &config)
{
	// YM2149 (in S-1985)
	// FDC: TC8566AF?, 1? 3.5" DSDD drive
	// S-1985 MSX Engine
	// 2? Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 0, 0, 4).set_total_size(0x10000);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 1, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "kdr", 3, 1, 1, 2, "kdr");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK3, "disk", 3, 2, 1, 2, "diskrom");

	MSX_S1985(config, "s1985", 0);

	msx2(YM2149, config);
}

/* MSX2 - Laser MSX2 (unreleased) */

/* MSX2 - Mitsubishi ML-G1 */

ROM_START(mlg1)
	// ic8f - hn27256g-25 (paint?)
	// ic10f - hn613128pn33-sp2 - t704p874h52 (sub?)
	// ic11f - hn613256pdc1 - t704p874h50 (bios?)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("mlg1bios.rom", 0x0000, 0x8000, CRC(0cc7f817) SHA1(e4fdf518a8b9c8ab4290c21b83be2c347965fc24))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("mlg1ext.rom", 0x0000, 0x4000, CRC(dc0951bd) SHA1(1e9a955943aeea9b1807ddf1250ba6436d8dd276))

	ROM_REGION(0x8000, "firmware", 0)
	ROM_LOAD("mlg1paint.rom", 0x0000, 0x8000, CRC(64df1750) SHA1(5cf0abca6dbcf940bc33c433ecb4e4ada02fbfe6))
ROM_END

void msx2_state::mlg1(machine_config &config)
{
	// YM2149 (in S3527)
	// FDC: None, 0 drives
	// S3527 MSX Engine
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x10000); // 64KB Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 3, 3, 0, 2, "firmware");

	msx2_pal(YM2149, config);
}

/* MSX2 - Mitsubishi ML-G3 */

ROM_START(mlg3)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("mlg3bios.rom", 0x0000, 0x8000, CRC(0cc7f817) SHA1(e4fdf518a8b9c8ab4290c21b83be2c347965fc24))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("mlg3ext.rom", 0x0000, 0x4000, CRC(dc0951bd) SHA1(1e9a955943aeea9b1807ddf1250ba6436d8dd276))

	ROM_REGION(0x4000, "diskrom", 0)
	// FDC register contents at 3ff8-3fff
	ROM_LOAD("mlg3disk.rom", 0x0000, 0x4000, BAD_DUMP CRC(b94ebc7a) SHA1(30ba1144c872a0bb1c91768e75a2c28ab1f4e3c6))

	ROM_REGION(0x4000, "rs232", 0)
	ROM_LOAD("mlg3rs232c.rom", 0x0000, 0x2000, CRC(849b325e) SHA1(b1ac74c2550d553579c1176f5dfde814218ec311))
	ROM_RELOAD(0x2000, 0x2000)
ROM_END

void msx2_state::mlg3(machine_config &config)
{
	// YM2149 (in S3527)
	// FDC: wd2793?, 1 3.5" DSDD drive
	// S-1985 MSX Engine
	// 4 Cartridge slots (3 internal, 1 taken by RS232 board)
	// S3527
	// RS232 with switch. What does the switch do?

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1, "disk", 3, 0, 1, 1, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");
	add_cartridge_slot<3>(config, 3, 1);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x10000); // 64KB Mapper RAM
	add_internal_slot_irq<4>(config, MSX_SLOT_RS232_MITSUBISHI, "rs232", 3, 3, 1, 1, "rs232");

	msx_wd2793(config);
	msx_1_35_dd_drive(config);
	msx2_pal(YM2149, config);
}

/* MSX2 - Mitsubishi ML-G10 */

ROM_START(mlg10)
	// ic8f - 00000210 - hn27256g-25
	// ic10f - 5j1 - hn613128p - m50 - t704p874h42-g
	// ic11f - 5j1 - hn613256p - cn9 - t704p874h40-g
	ROM_REGION(0xc000, "mainrom", 0)
	ROM_LOAD("mlg10bios.rom", 0x0000, 0x8000, CRC(a27c563d) SHA1(c1e46c00f1e38fc9e0ab487bf0513bd93ce61f3f))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("mlg10ext.rom", 0x0000, 0x4000, CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33))

	ROM_REGION(0x8000, "firmware", 0)
	ROM_LOAD("g10firm.rom", 0x0000, 0x8000, CRC(dd4007f9) SHA1(789bb6cdb2d1ed0348f36336da11b149d74e4d9f))

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("mlg10kfn.rom", 0, 0x20000, CRC(d23d4d2d) SHA1(db03211b7db46899df41db2b1dfbec972109a967))
ROM_END

void msx2_state::mlg10(machine_config &config)
{
	// YM2149 (in S3527)
	// FDC: None, 0 drives
	// S3527 MSX Engine
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x10000); // 64KB Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 3, 3, 0, 2, "firmware");

	msx2(YM2149, config);
}

/* MSX2 - Mitsubishi ML-G30 Model 1 */

ROM_START(mlg30)
	ROM_REGION(0x10000, "mainrom", 0)
	ROM_LOAD("g30bios.rom", 0x0000, 0x8000, CRC(a27c563d) SHA1(c1e46c00f1e38fc9e0ab487bf0513bd93ce61f3f))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("g30ext.rom", 0x0000, 0x4000, CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("g30disk.rom", 0x0000, 0x4000, CRC(a90be8d5) SHA1(f7c3ac138918a493eb91628ed88cf37999059579))

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("g30kfn.rom", 0x0000, 0x20000, CRC(d23d4d2d) SHA1(db03211b7db46899df41db2b1dfbec972109a967))
ROM_END

void msx2_state::mlg30(machine_config &config)
{
	// YM2149 (in S3527)
	// FDC: wd2793/tc8566af?, 1 3.5" DSDD drives
	// 4 Cartridge slots
	// S3527

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1, "disk", 3, 0, 1, 2, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");
	add_cartridge_slot<3>(config, 3, 1);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000);   // 128KB Mapper RAM
	add_cartridge_slot<4>(config, 3, 3);

	msx_wd2793(config);
	msx_1_35_dd_drive(config);
	msx2(YM2149, config);
}

/* MSX2 - Mitsubishi ML-G30 Model 2 */

ROM_START(mlg30_2)
	ROM_REGION(0x10000, "mainrom", 0)
	ROM_LOAD("g30bios.rom", 0x0000, 0x8000, CRC(a27c563d) SHA1(c1e46c00f1e38fc9e0ab487bf0513bd93ce61f3f))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("g30ext.rom", 0x0000, 0x4000, CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("g30disk.rom", 0x0000, 0x4000, CRC(995e6bf6) SHA1(6069d63c68b03fa56de040fb5f52eeadbffe2a2c))

	ROM_REGION(0x4000, "rs232", 0)
	ROM_LOAD("g30rs232c.rom", 0x0000, 0x2000, CRC(15d6ba9e) SHA1(782e54cf88eb4a974631eaa707aad97d3eb1ea14))
	ROM_RELOAD(0x2000, 0x2000)

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("g30kfn.rom", 0x0000, 0x20000, CRC(d23d4d2d) SHA1(db03211b7db46899df41db2b1dfbec972109a967))
ROM_END

void msx2_state::mlg30_2(machine_config &config)
{
	// YM2149 (in S3527)
	// FDC: wd2793/tc8566af?, 2 3.5" DSDD drives
	// 4 Cartridge slots (1 taken by RS232 board)
	// S3527
	// RS232 with switch. What does the switch do?

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1, "disk", 3, 0, 1, 2, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");
	add_cartridge_slot<3>(config, 3, 1);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000);   // 128KB Mapper RAM
	add_internal_slot_irq<4>(config, MSX_SLOT_RS232_MITSUBISHI, "rs232", 3, 3, 1, 1, "rs232");

	msx_wd2793(config);
	msx_2_35_dd_drive(config);
	msx2(YM2149, config);
}

/* MSX2 - ML-TS100 (should be a separate driver) */

/* MSX2 - ML-TS100M2 (should be a separate driver) */

/* MSX2 - ML-TS2 */

/* MSX2 - NTT Captain Multi-Station (should be a separate driver due to the added V99C37-F) */

/* MSX2 - National FS-4500 */

ROM_START(fs4500)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("4500bios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("4500ext.rom", 0x0000, 0x4000, CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33))

	ROM_REGION(0x4000, "font", 0)
	ROM_LOAD("4500font.rom", 0x0000, 0x4000, CRC(4bd54f95) SHA1(3ce8e35790eb4689b21e14c7ecdd4b63943ee158))

	ROM_REGION(0x8000, "jukugo", 0)
	ROM_LOAD("4500buns.rom", 0x0000, 0x8000, CRC(c9398e11) SHA1(e89ea1e8e583392e2dd9debb8a4b6a162f58ba91))

	ROM_REGION(0x8000, "jusho", 0)
	ROM_LOAD("4500jush.rom", 0x0000, 0x8000, CRC(4debfd2d) SHA1(6442c1c5cece64c6dae90cc6ae3675f070d93e06))

	ROM_REGION(0xc000, "wordpro1", 0)
	ROM_LOAD("4500wor1.rom", 0x0000, 0xc000, CRC(0c8b5cfb) SHA1(3f047469b62d93904005a0ea29092e892724ce0b))

	ROM_REGION(0xc000, "wordpro2", 0)
	ROM_LOAD("4500wor2.rom", 0x0000, 0xc000, CRC(d9909451) SHA1(4c8ea05c09b40c41888fa18db065575a317fda16))

	ROM_REGION(0x4000, "kdr1", 0)
	ROM_LOAD("4500kdr1.rom", 0x0000, 0x4000, CRC(f8c7f0db) SHA1(df07e89fa0b1c7874f9cdf184c136f964fea4ff4))

	ROM_REGION(0x4000, "kdr2", 0)
	ROM_LOAD("4500kdr2.rom", 0x0000, 0x4000, CRC(69e87c31) SHA1(c63db26660da96af56f8a7d3ea18544b9ae5a37c))

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("4500kfn.rom", 0, 0x20000, CRC(956dc96d) SHA1(9ed3ab6d893632b9246e91b412cd5db519e7586b))

	ROM_REGION(0x20000, "bunsetsu", 0)
	ROM_LOAD("4500budi.rom", 0, 0x20000, CRC(f94590f8) SHA1(1ebb06062428fcdc66808a03761818db2bba3c73))
ROM_END

void msx2_state::fs4500(machine_config &config)
{
	// YM2149 (in S3527 MSX Engine)
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// S3527 MSX Engine
	// Matsushita switched device
	// 8KB SRAM (NEC D4364C-15L)
	// Builtin thermal printer
	// Switch to switch between JP50on and JIS keyboard layout
	// Switch to switch between internal and external printer
	// Switch to bypass the firmware

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 1, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "font", 0, 2, 0, 1, "font");
	add_internal_slot(config, MSX_SLOT_BUNSETSU, "jukugo", 0, 2, 1, 2, "jukugo").set_bunsetsu_region_tag("bunsetsu");
	add_internal_slot(config, MSX_SLOT_ROM, "jusho", 0, 3, 1, 2, "jusho");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "wordpro1", 3, 0, 0, 3, "wordpro1");
	add_internal_slot(config, MSX_SLOT_ROM, "kdr1", 3, 0, 3, 1, "kdr1");
	add_internal_slot(config, MSX_SLOT_ROM, "wordpro2", 3, 1, 0, 3, "wordpro2");
	add_internal_slot(config, MSX_SLOT_ROM, "kdr2", 3, 1, 3, 1, "kdr2");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 2, 0, 4);  // 64KB RAM

	MSX_MATSUSHITA(config, "matsushita", 0);
	msx2(YM2149, config);
}

/* MSX2 - National FS-4600F */

ROM_START(fs4600f)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("4600bios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("4600ext.rom", 0x0000, 0x4000, CRC(43e7a7fc) SHA1(0fbd45ef3dd7bb82d4c31f1947884f411f1ca344))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("4600disk.rom", 0x0000, 0x4000, CRC(ae4e65b7) SHA1(073feb8bb645d935e099afaf61e6f04f52adee42))

	ROM_REGION(0x4000, "font1", 0)
	ROM_LOAD("4600fon1.rom", 0x0000, 0x4000, CRC(7391389b) SHA1(31292b9ca9fe7d1d8833530f44c0a5671bfefe4e))

	ROM_REGION(0x4000, "font2", 0)
	ROM_LOAD("4600fon2.rom", 0x0000, 0x4000, CRC(c3a6b445) SHA1(02155fc25c9bd23e1654fe81c74486351e1ecc28))

	ROM_REGION(0x8000, "kdr", 0)
	ROM_LOAD("4600kdr.rom", 0x0000, 0x8000, CRC(b2db6bf5) SHA1(3a9a942ed888dd641cddf8deada1879c454df3c6))

	ROM_REGION(0x100000, "firmware", 0)
	ROM_LOAD("4600firm.rom", 0x0000, 0x100000, CRC(1df57472) SHA1(005794c10a4237de3907ba4a44d436078d3c06c2))

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("4600kfn.rom", 0, 0x20000, CRC(c61ddc5d) SHA1(5e872d5853698731a0ed22fb72dbcdfd59cd19c3))

	/* Matsushita 12 dots Kanji ROM must be emulated */
	ROM_REGION(0x20000, "kanji12", 0)
	ROM_LOAD("4600kf12.rom", 0, 0x20000, CRC(340d1ef7) SHA1(a7a23dc01314e88381eee88b4878b39931ab4818))
ROM_END

void msx2_state::fs4600f(machine_config &config)
{
	// YM2149 (in S-1985 MSX Engine)
	// FDC: mb8877a, 1 3.5" DSDD drive
	// 2 Cartridge slots
	// S-1985 MSX Engine
	// 8KB SRAM (NEC D4364C-15L)
	// Builtin thermal printer
	// Switch to switch between JP50on and JIS keyboard layout
	// Switch to switch between internal and external printer
	// Switch to bypass the firmware

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 1, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "font1", 0, 2, 0, 1, "font1");
	add_internal_slot(config, MSX_SLOT_ROM, "kdr", 0, 2, 1, 2, "kdr");
	add_internal_slot(config, MSX_SLOT_ROM, "font2", 0, 3, 0, 1, "font2");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_FS4600, "firmware", 3, 1, 0, 4, "firmware");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000).set_ramio_bits(0x80);   // 128KB Mapper RAM
	add_internal_disk_mirrored(config, MSX_SLOT_DISK2, "disk", 3, 3, 1, 2, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");

	msx_kanji12_device &kanji12(MSX_KANJI12(config, "kanji12", 0));
	kanji12.set_rom_start("kanji12");

	MSX_S1985(config, "s1985", 0);

	msx_mb8877a(config);
	msx_1_35_dd_drive(config);
	msx2(YM2149, config);
}

/* MSX2 - National FS-4700 */

ROM_START(fs4700f)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("4700bios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("4700ext.rom", 0x0000, 0x4000, CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33))

	ROM_REGION(0x4000, "diskrom", 0)
	// ic403 - disk basic - copyright mei - 1986 das4700j1
	ROM_LOAD("4700disk.rom",  0x0000, 0x4000, CRC(1e7d6512) SHA1(78cd7f847e77fd8cd51a647efb2725ba93f4c471))

	ROM_REGION(0x4000, "font", 0)
	ROM_LOAD("4700font.rom", 0x0000, 0x4000, CRC(4bd54f95) SHA1(3ce8e35790eb4689b21e14c7ecdd4b63943ee158))

	ROM_REGION(0x8000, "jukugo", 0)
	ROM_LOAD("4700buns.rom", 0x0000, 0x8000, CRC(c9398e11) SHA1(e89ea1e8e583392e2dd9debb8a4b6a162f58ba91))

	ROM_REGION(0x8000, "jusho", 0)
	ROM_LOAD("4700jush.rom", 0x0000, 0x8000, CRC(4debfd2d) SHA1(6442c1c5cece64c6dae90cc6ae3675f070d93e06))

	ROM_REGION(0xc000, "wordpro1", 0)
	ROM_LOAD("4700wor1.rom", 0x0000, 0xc000, CRC(5f39a727) SHA1(f5af1d2a8bcf247f78847e1a9d995e581df87e8e))

	ROM_REGION(0xc000, "wordpro2", 0)
	ROM_LOAD("4700wor2.rom", 0x0000, 0xc000, CRC(d9909451) SHA1(4c8ea05c09b40c41888fa18db065575a317fda16))

	ROM_REGION(0x4000, "kdr1", 0)
	ROM_LOAD("4700kdr1.rom", 0x0000, 0x4000, CRC(f8c7f0db) SHA1(df07e89fa0b1c7874f9cdf184c136f964fea4ff4))

	ROM_REGION(0x4000, "kdr2", 0)
	ROM_LOAD("4700kdr2.rom", 0x0000, 0x4000, CRC(69e87c31) SHA1(c63db26660da96af56f8a7d3ea18544b9ae5a37c))

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("4700kfn.rom", 0, 0x20000, CRC(956dc96d) SHA1(9ed3ab6d893632b9246e91b412cd5db519e7586b))

	ROM_REGION(0x20000, "bunsetsu", 0)
	ROM_LOAD("4700budi.rom", 0, 0x20000, CRC(f94590f8) SHA1(1ebb06062428fcdc66808a03761818db2bba3c73))
ROM_END

void msx2_state::fs4700f(machine_config &config)
{
	// YM2149 (in S3527 MSX Engine)
	// FDC: mb8877a, 1 3.5" DSDD drive
	// 2 Cartridge slots
	// S3527 MSX Engine
	// Matsushita switched device
	// 8KB SRAM (NEC D4364C-15L)
	// Builtin thermal printer
	// Switch to switch between JP50on and JIS keyboard layout
	// Switch to switch between internal and external printer
	// Switch to bypass the firmware

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 1, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "font", 0, 2, 0, 1, "font");
	add_internal_slot(config, MSX_SLOT_BUNSETSU, "buns", 0, 2, 1, 2, "jukugo").set_bunsetsu_region_tag("bunsetsu");
	add_internal_slot(config, MSX_SLOT_ROM, "jusho", 0, 3, 1, 2, "jusho");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "wordpro1", 3, 0, 0, 3, "wordpro1");
	add_internal_slot(config, MSX_SLOT_ROM, "kdr1", 3, 0, 3, 1, "kdr1");
	add_internal_slot(config, MSX_SLOT_ROM, "wordpro2", 3, 1, 0, 3, "wordpro2");
	add_internal_slot(config, MSX_SLOT_ROM, "kdr2", 3, 1, 3, 1, "kdr2");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 2, 0, 4);  // 64KB RAM
	add_internal_disk_mirrored(config, MSX_SLOT_DISK2, "disk", 3, 3, 1, 2, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");

	MSX_S1985(config, "s1985", 0);

	MSX_MATSUSHITA(config, "matsushita", 0);

	msx_mb8877a(config);
	msx_1_35_dd_drive(config);
	msx2(YM2149, config);
}

/* MSX2 - National FS-5000F2 */

ROM_START(fs5000f2)
	ROM_REGION(0x18000, "mainrom", ROMREGION_ERASEFF)
	ROM_LOAD("5000bios.rom", 0x0000, 0x8000, CRC(a44ea707) SHA1(59967765d6e9328909dee4dac1cbe4cf9d47d315))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("5000ext.rom",  0x0000, 0x4000, CRC(43e7a7fc) SHA1(0fbd45ef3dd7bb82d4c31f1947884f411f1ca344))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("5000disk.rom", 0x0000, 0x4000, CRC(ae4e65b7) SHA1(073feb8bb645d935e099afaf61e6f04f52adee42))

	ROM_REGION(0x8000, "setup", 0)
	ROM_LOAD("5000rtc.rom", 0x0000, 0x8000, CRC(03351598) SHA1(98bbfa3ab07b7a5cad55d7ddf7cbd9440caa2a86))

	ROM_REGION(0x8000, "kdr", 0)
	ROM_LOAD("5000kdr.rom", 0x0000, 0x8000, CRC(b2db6bf5) SHA1(3a9a942ed888dd641cddf8deada1879c454df3c6))

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("5000kfn.rom", 0, 0x20000, CRC(c61ddc5d) SHA1(5e872d5853698731a0ed22fb72dbcdfd59cd19c3))
ROM_END

void msx2_state::fs5000f2(machine_config &config)
{
	// YM2149 (in S-1985 MSX Engine)
	// FDC: wd2793, 2 3.5" DSDD drives
	// 3 Expansion slots
	// 2 Cartridge slots
	// S-1985 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "cn904", 0, 1, 0, 4, "mainrom", 0x8000); // expansion slot
	add_internal_slot(config, MSX_SLOT_ROM, "cn906", 0, 2, 0, 4, "mainrom", 0x8000); // expansion slot
	add_internal_slot(config, MSX_SLOT_ROM, "cn907", 0, 3, 0, 4, "mainrom", 0x8000); // expansion slot
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "kdr", 3, 0, 1, 2, "kdr");
	add_internal_slot(config, MSX_SLOT_ROM, "rtcrom", 3, 1, 1, 2, "setup");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000).set_ramio_bits(0x80);   // 128KB Mapper RAM
	add_internal_disk_mirrored(config, MSX_SLOT_DISK2, "disk", 3, 3, 1, 2, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");

	MSX_S1985(config, "s1985", 0);

	msx_wd2793_force_ready(config);
	msx_2_35_dd_drive(config);
	msx2(YM2149, config);
}

/* MSX2 - National FS-5500F1 */

ROM_START(fs5500f1)
	// ic206
	// OS1 ROM
	// MATSUSHITA
	// 1985 DAS5500A1
	// DFQT9062ZA
	// MSL27256K
	//
	// ic208
	// HKN ROM
	// MASTUSHITA
	// 1985 DAS5500E1
	// DFQT9068ZA
	//
	// ic209
	// API ROM
	// MATSUSHITA
	// 1985 DAS500C1
	// DFQT9064ZA
	// MSL27256K
	//
	// ic210
	// OS2 ROM
	// MATSUSHITA
	// 1985 DAS5500B1
	// DFQT9063ZA
	// HN4827128G-25
	//
	// ic211
	// FDC ROM
	// MATSUSHITA
	// 1985 DAS5500F1
	// DFQT9067ZA
	// HN4827128G-25
	ROM_REGION(0x18000, "mainrom", ROMREGION_ERASEFF)
	ROM_LOAD("5500bios.rom", 0x0000, 0x8000, CRC(5bf38e13) SHA1(44e0dd215b2a9f0770dd76fb49187c05b083eed9))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("5500ext.rom", 0x0000, 0x4000, CRC(3c42c367) SHA1(4be8371f3b03e70ddaca495958345f3c4f8e2d36))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("5500disk.rom", 0x0000, 0x4000, CRC(1e7d6512) SHA1(78cd7f847e77fd8cd51a647efb2725ba93f4c471))

	ROM_REGION(0x8000, "impose", 0)
	ROM_LOAD("5500imp.rom", 0x0000, 0x8000, CRC(6173a88c) SHA1(b677a861b67e8763a11d5dcf52416b42493ade57))

	ROM_REGION(0x8000, "kdr", 0)
	ROM_LOAD("5500kdr.rom", 0x0000, 0x8000, CRC(b2db6bf5) SHA1(3a9a942ed888dd641cddf8deada1879c454df3c6))

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("tc531000p_6611.ic212", 0, 0x20000, CRC(956dc96d) SHA1(9ed3ab6d893632b9246e91b412cd5db519e7586b))
ROM_END

void msx2_state::fs5500f1(machine_config &config)
{
	// YM2149 in (S3527 MSX Engine)
	// FDC: mb8877a, 1 3.5" DSDD drive
	// 3 Expansion slots
	// 2 Cartridge slots
	// S3527 MSX Engine
	// Matsushita switched device
	// Switch to switch between JP50on and JIS keyboard layout
	// Switch to bypass the firmware

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "cn904", 0, 1, 0, 4, "mainrom", 0x8000); // expansion slot
	add_internal_slot(config, MSX_SLOT_ROM, "cn906", 0, 2, 0, 4, "mainrom", 0x8000); // expansion slot
	add_internal_slot(config, MSX_SLOT_ROM, "cn908", 0, 3, 0, 4, "mainrom", 0x8000); // expansion slot
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "kdr", 3, 0, 1, 2, "kdr");
	add_internal_slot(config, MSX_SLOT_ROM, "impose", 3, 1, 1, 2, "impose");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 2, 0, 4);  // 64KB RAM
	add_internal_disk_mirrored(config, MSX_SLOT_DISK2, "disk", 3, 3, 1, 2, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");

	MSX_MATSUSHITA(config, "matsushita", 0);

	msx_mb8877a(config);
	msx_1_35_dd_drive(config);
	msx2(YM2149, config);
}

/* MSX2 - National FS-5500F2 */

ROM_START(fs5500f2)
	ROM_REGION(0x18000, "mainrom", ROMREGION_ERASEFF)
	ROM_LOAD("5500bios.rom", 0x0000, 0x8000, CRC(5bf38e13) SHA1(44e0dd215b2a9f0770dd76fb49187c05b083eed9))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("5500ext.rom", 0x0000, 0x4000, CRC(3c42c367) SHA1(4be8371f3b03e70ddaca495958345f3c4f8e2d36))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("5500disk.rom", 0x0000, 0x4000, CRC(1e7d6512) SHA1(78cd7f847e77fd8cd51a647efb2725ba93f4c471))

	ROM_REGION(0x8000, "impose", 0)
	ROM_LOAD("5500imp.rom", 0x0000, 0x8000, CRC(6173a88c) SHA1(b677a861b67e8763a11d5dcf52416b42493ade57))

	ROM_REGION(0x8000, "kdr", 0)
	ROM_LOAD("5500kdr.rom", 0x0000, 0x8000, CRC(b2db6bf5) SHA1(3a9a942ed888dd641cddf8deada1879c454df3c6))

	ROM_REGION(0x20000, "kanji", 0)
	// ic212 - tc531000p
	ROM_LOAD("5500kfn.rom", 0, 0x20000, CRC(956dc96d) SHA1(9ed3ab6d893632b9246e91b412cd5db519e7586b))
ROM_END

void msx2_state::fs5500f2(machine_config &config)
{
	// YM2149 in (S3527 MSX Engine)
	// FDC: mb8877a, 2 3.5" DSDD drives
	// 3 Expansion slots
	// 2 Cartridge slots
	// S3527 MSX Engine
	// Matsushita switched device
	// Switch to switch between JP50on and JIS keyboard layout
	// Switch to bypass the firmware

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "cn904", 0, 1, 0, 4, "mainrom", 0x8000); // expansion slot
	add_internal_slot(config, MSX_SLOT_ROM, "cn906", 0, 2, 0, 4, "mainrom", 0x8000); // expansion slot
	add_internal_slot(config, MSX_SLOT_ROM, "cn908", 0, 3, 0, 4, "mainrom", 0x8000); // expansion slot
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "kdr", 3, 0, 1, 2, "kdr");
	add_internal_slot(config, MSX_SLOT_ROM, "impose", 3, 1, 1, 2, "impose");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 2, 0, 4);  // 64KB RAM
	add_internal_disk_mirrored(config, MSX_SLOT_DISK2, "disk", 3, 3, 1, 2, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");

	MSX_MATSUSHITA(config, "matsushita", 0);

	msx_mb8877a(config);
	msx_2_35_dd_drive(config);
	msx2(YM2149, config);
}

/* MSX2 - Panasonic FS-A1 */

ROM_START(fsa1)
	ROM_REGION(0x20000, "mainrom", 0)
	ROM_LOAD("fsa1.ic3", 0x00000, 0x20000, CRC(4d6dae42) SHA1(7bbe3f355d3129592268ae87f40ea7e3ced88f98))
ROM_END

void msx2_state::fsa1(machine_config &config)
{
	// YM2149 (in S1985)
	// FDC: None, 0 drives
	// S1985
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 0, 4);  // 64 KB RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 1, 0, 2, "mainrom", 0x8000);
	add_internal_slot(config, MSX_SLOT_ROM, "deskpac1", 3, 2, 1, 2, "mainrom", 0x10000);
	add_internal_slot(config, MSX_SLOT_ROM, "deskpac2", 3, 3, 1, 2, "mainrom", 0x18000);

	MSX_S1985(config, "s1985", 0);

	msx2(YM2149, config);
}

/* MSX2 - Panasonic FS-A1 (a) */

ROM_START(fsa1a)
	ROM_REGION(0x1c000, "maincpu", 0)
	ROM_LOAD("a1bios.rom",   0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))
	ROM_LOAD("a1ext.rom",    0x8000, 0x4000, CRC(43e7a7fc) SHA1(0fbd45ef3dd7bb82d4c31f1947884f411f1ca344))
	ROM_LOAD("a1desk1a.rom", 0xc000, 0x8000, CRC(25b5b170) SHA1(d9307bfdaab1312d25e38af7c0d3a7671a9f716b))
	ROM_LOAD("a1desk2.rom", 0x14000, 0x8000, CRC(7f6f4aa1) SHA1(7f5b76605e3d898cc4b5aacf1d7682b82fe84353))
ROM_END

void msx2_state::fsa1a(machine_config &config)
{
	// YM2149 (in S1985)
	// FDC: None, 0 drives
	// S1985
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "bios", 0, 0, 2, "maincpu");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 0, 4);  // 64KB RAM
	add_internal_slot(config, MSX_SLOT_ROM, "ext", 3, 1, 0, 1, "maincpu", 0x8000);
	add_internal_slot(config, MSX_SLOT_ROM, "desk1", 3, 2, 1, 2, "maincpu", 0xc000);
	add_internal_slot(config, MSX_SLOT_ROM, "desk2", 3, 3, 1, 2, "maincpu", 0x14000);

	MSX_S1985(config, "s1985", 0);

	msx2(YM2149, config);
}

/* MSX2 - Panasonic FS-A1F */

ROM_START(fsa1f)
	ROM_REGION(0x20000, "mainrom", 0)
	ROM_LOAD("da1024d0365r.ic18", 0x00000, 0x20000, CRC(64a53ec8) SHA1(9a62d7a5ccda974261f7c0600476d85e10deb99b))

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("da531p6616_0.ic17", 0, 0x20000, CRC(c61ddc5d) SHA1(5e872d5853698731a0ed22fb72dbcdfd59cd19c3))	// da531p6616-0 - tc531000ap-6616
ROM_END

void msx2_state::fsa1f(machine_config &config)
{
	// YM2149 (in S1985)
	// FDC: tc8566af, 1 3.5" DSDD drive
	// S1985
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 0, 0, 4).set_total_size(0x10000).set_ramio_bits(0x80);   // 64KB Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 1, 0, 1, "mainrom", 0x8000);
	add_internal_slot(config, MSX_SLOT_ROM, "kdr", 3, 1, 1, 2, "mainrom", 0x10000);
	add_internal_disk_mirrored(config, MSX_SLOT_DISK3, "disk", 3, 2, 1, 2, "mainrom", 0xc000);
	add_internal_slot(config, MSX_SLOT_ROM, "cockpit", 3, 3, 1, 2, "mainrom", 0x18000);

	MSX_S1985(config, "s1985", 0);

	msx2(YM2149, config);
}

/* MSX2 - Panasonic FS-A1FM */

ROM_START(fsa1fm)
	// Everything should be in one rom??
	// da534p66220 - tc534000p-6620 - ic12 - 4mbit rom
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("a1fmbios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("a1fmext.rom", 0x0000, 0x4000, CRC(ad295b5d) SHA1(d552319a19814494e3016de4b8f010e8f7b97e02))

	ROM_REGION(0x4000, "diskrom", 0)
	// FDC register contents at 3ff8-3fff
	ROM_LOAD("a1fmdisk.rom", 0x0000, 0x4000, BAD_DUMP CRC(e25cacca) SHA1(607cfca605eaf82e3efa33459d6583efb7ecc13b))

	ROM_REGION(0x100000, "firmware", 0)
	ROM_LOAD("a1fmfirm.rom", 0x000000, 0x100000, CRC(8ce0ece7) SHA1(f89e3d8f3b6855c29d71d3149cc762e0f6918ad5))

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("a1fmkfn.rom", 0, 0x20000, CRC(c61ddc5d) SHA1(5e872d5853698731a0ed22fb72dbcdfd59cd19c3))

	ROM_REGION(0x20000, "kanji12", 0)
	ROM_LOAD("a1fmkf12.rom", 0, 0x20000, CRC(340d1ef7) SHA1(a7a23dc01314e88381eee88b4878b39931ab4818))
ROM_END

void msx2_state::fsa1fm(machine_config &config)
{
	// AY8910 (in T9769)
	// FDC: tc8566af, 1 3.5" DSDD drive
	// 2 Cartridge slots
	// T9769
	// 8KB SRAM
	// Integrated 1200baud modem

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 0, 0, 4).set_total_size(0x10000).set_ramio_bits(0x80);   // 64KB Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 1, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_FSA1FM, "modem", 3, 1, 1, 1, "firmware");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK3, "disk", 3, 2, 1, 2, "diskrom");
	add_internal_slot(config, MSX_SLOT_FSA1FM2, "firmware", 3, 3, 0, 3, "firmware");

	msx_kanji12_device &kanji12(MSX_KANJI12(config, "kanji12", 0));
	kanji12.set_rom_start("kanji12");

	msx2(AY8910, config);
}

/* MSX2 - Panasonic FS-A1MK2 */

ROM_START(fsa1mk2)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("a1mkbios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("a1mk2ext.rom", 0x0000, 0x4000, CRC(43e7a7fc) SHA1(0fbd45ef3dd7bb82d4c31f1947884f411f1ca344))

	ROM_REGION(0x8000, "deskpac1", 0)
	ROM_LOAD("a1mkcoc1.rom", 0x0000, 0x8000, CRC(0eda3f57) SHA1(2752cd89754c05abdf7c23cba132d38e3ef0f27d))

	ROM_REGION(0x4000, "deskpac2", 0)
	ROM_LOAD("a1mkcoc2.rom", 0x0000, 0x4000, CRC(756d7128) SHA1(e194d290ebfa4595ce0349ea2fc15442508485b0))

	ROM_REGION(0x8000, "deskpac3", 0)
	ROM_LOAD("a1mkcoc3.rom", 0x0000, 0x8000, CRC(c1945676) SHA1(a3f4e2e4934074925d775afe30ac72f150ede543))
ROM_END

void msx2_state::fsa1mk2(machine_config &config)
{
	// YM2149 (in S1985)
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// S1985
	// pause button

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 0, 0, 4).set_total_size(0x10000).set_ramio_bits(0x80);   // 64 KB Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 1, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "deskpac1", 3, 1, 1, 2, "deskpac1");
	add_internal_slot(config, MSX_SLOT_ROM, "deskpac2", 3, 2, 1, 1, "deskpac2");
	add_internal_slot(config, MSX_SLOT_ROM, "deskpac3", 3, 3, 1, 2, "deskpac3");

	MSX_S1985(config, "s1985", 0);

	msx2(YM2149, config);
}

/* MSX2 - Philips HCS 280 */

/* MSX2 - Philips NMS 8220 - 2 possible sets (/00 /16) */

ROM_START(nms8220)
	ROM_REGION(0x20000, "mainrom", 0)
	ROM_LOAD("m531000-52_68503.u14", 0x0000, 0x20000, CRC(f506d7ab) SHA1(e761e7081c613ad4893a664334ce105841d0e80e))

	ROM_REGION(0x4000, "designer", 0)
	ROM_SYSTEM_BIOS(0, "v1308", "13-08-1986 Designer")
	ROMX_LOAD("8220pena.u13", 0x0000, 0x4000, CRC(17817b5a) SHA1(5df95d033ae70b107697b69470126ce1b7ae9eb5), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v1206", "12-06-1986 Designer")
	ROMX_LOAD("8220pen.u13",  0x0000, 0x4000, CRC(3d38c53e) SHA1(cb754aed85b3e97a7d3c5894310df7ca18f89f41), ROM_BIOS(1))
ROM_END

void msx2_state::nms8220(machine_config &config)
{
	// YM2149 (in S-3527 MSX Engine)
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// S-3527 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom", 0x10000);
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "mainrom", 0x18000);
	// Memory mapper blocks mirrored every 8 blocks: 4x ram, 4x empty, 4x ram, 4x empty, etc
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x10000).set_ramio_bits(0xf8);   // 64KB Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "designer", 3, 3, 1, 1, "designer");
	add_internal_slot(config, MSX_SLOT_ROM, "designer_mirror", 3, 3, 2, 1, "designer");

	msx2_pal(YM2149, config);
}

/* MSX2 - Philips NMS 8245 - 2 possible sets (/00 /16) */
/* /00 - A16 = 0 */
/* /16 - A16 = 1 */
/* /19 - Azerty keyboard */

ROM_START(nms8245)
	ROM_REGION(0x10000, "mainrom", 0)
	ROM_SYSTEM_BIOS(0, "v1.08", "v1.08")
	ROMX_LOAD("v1_08.u7", 0x00000, 0x10000, CRC(69d5cbe6) SHA1(cc57c1dcd7249ea9f8e2547244592e7d97308ed0), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v1.06", "v1.06")
	ROMX_LOAD("v1_06.u7", 0x00000, 0x10000, CRC(be4ae17e) SHA1(cc57c1dcd7249ea9f8e2547244592e7d97308ed0), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v1.05", "v1.05")
	ROMX_LOAD("v1_05.u7", 0x00000, 0x10000, CRC(cef8895d) SHA1(cc57c1dcd7249ea9f8e2547244592e7d97308ed0), ROM_BIOS(2))
ROM_END

void msx2_state::nms8245(machine_config &config)
{
	// XTAL: 21328.1 (different from default)
	// YM2149 (in S-3527 MSX Engine)
	// FDC: wd2793, 1 3.5" DSDD drive
	// 2 Cartridge slots
	// S-3527 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "mainrom", 0x8000);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000).set_ramio_bits(0xf8);   // 128KB Mapper RAM
	// Disk rom is not mirrored, but FDC registers are
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1, "disk", 3, 3, 1, 2, "mainrom", 0xc000).set_tags("fdc", "fdc:0", "fdc:1");

	msx_wd2793_force_ready(config);
	msx_1_35_dd_drive(config);
	msx2_pal(YM2149, config);
}

/* MSX2 - Philips NMS 8245/19 */

ROM_START(nms8245f)
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_LOAD("nms8245.u7", 0x0000, 0x20000, BAD_DUMP CRC(0c827d5f) SHA1(064e706cb1f12b99b329944ceeedc0efc3b2d9be))
ROM_END

void msx2_state::nms8245f(machine_config &config)
{
	// YM2149 (in S-3527 MSX Engine)
	// FDC: wd2793, 1 3.5" DSDD drive
	// 2 Cartridge slots
	// S-3527 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "bios", 0, 0, 2, "maincpu", 0x10000);
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "ext", 3, 0, 0, 1, "maincpu", 0x18000);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000).set_ramio_bits(0xf8);   // 128KB Mapper RAM
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1, "disk", 3, 3, 1, 2, "maincpu", 0x1c000).set_tags("fdc", "fdc:0", "fdc:1");

	msx_wd2793_force_ready(config);
	msx_1_35_dd_drive(config);
	msx2_pal(YM2149, config);
}

/* MSX2 - Philips NMS 8245 Home Banking (Italy) */

/* MSX2 - Philips NMS 8250/00 */

ROM_START(nms8250)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("d23c256eac.ic119", 0x0000, 0x8000, CRC(6cdaf3a5) SHA1(6103b39f1e38d1aa2d84b1c3219c44f1abb5436e))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("d23128ec.ic118", 0x0000, 0x4000, CRC(66237ecf) SHA1(5c1f9c7fb655e43d38e5dd1fcc6b942b2ff68b02))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_SYSTEM_BIOS(0, "v1.08", "v1.08 diskrom")
	ROMX_LOAD("v1.08.ic117", 0x0000, 0x4000, CRC(61f6fcd3) SHA1(dab3e6f36843392665b71b04178aadd8762c6589), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "unknown", "Unknown version diskrom")
	ROMX_LOAD("jq00014.ic117", 0x0000, 0x4000, CRC(ca3307d3) SHA1(c3efedda7ab947a06d9345f7b8261076fa7ceeef), ROM_BIOS(1))
ROM_END

void msx2_state::nms8250(machine_config &config)
{
	// YM2149 (in S-3527 MSX Engine)
	// FDC: wd2793, 1 3.5" DSDD drive
	// 2 Cartridge slots
	// S-3527 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	// ROM is mirrored in all 4 pages
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom1", 3, 0, 1, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom2", 3, 0, 2, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom3", 3, 0, 3, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000).set_ramio_bits(0xf8);   // 128KB Mapper RAM
	// ROM is not mirrored but the FDC registers are in all pages
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1, "disk", 3, 3, 1, 2, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");

	msx_wd2793_force_ready(config);
	msx_1_35_dd_drive(config);
	msx2_pal(YM2149, config);
}

/* MSX2 - Philips NMS 8250/16 */

ROM_START(nms8250_16)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("bios.ic119", 0x0000, 0x8000, CRC(5e3caf18) SHA1(ee0d8ccfc247368078d27183c34b3a5c0f4ae0f1))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("sub.ic118", 0x0000, 0x4000, CRC(0a0aeb2f) SHA1(b83770cca8453a153d7e260070a3a3c059d64ed3))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("jq00014.ic117", 0x0000, 0x4000, CRC(ca3307d3) SHA1(c3efedda7ab947a06d9345f7b8261076fa7ceeef))
ROM_END

/* MSX2 - Philips NMS 8250/19 */

ROM_START(nms8250_19)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("nms8250fbios.ic119", 0x0000, 0x8000, CRC(bee21533) SHA1(d18694e9e7040b2851fe985cefb89766868a2fd3))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("nms8250fext.ic118",  0x0000, 0x4000, CRC(781ba055) SHA1(fd4bcc81a8160a1dea06036c5f79d200f948f4d6))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("nms8250fdisk.ic117", 0x0000, 0x4000, CRC(ca3307d3) SHA1(c3efedda7ab947a06d9345f7b8261076fa7ceeef))
ROM_END

/* MSX2 - Philips NMS 8255 */

ROM_START(nms8255)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("8255bios.rom.ic119", 0x0000, 0x8000, CRC(6cdaf3a5) SHA1(6103b39f1e38d1aa2d84b1c3219c44f1abb5436e))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("8255ext.rom.ic118",  0x0000, 0x4000, CRC(66237ecf) SHA1(5c1f9c7fb655e43d38e5dd1fcc6b942b2ff68b02))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("8255disk.rom.ic117", 0x0000, 0x4000, CRC(ca3307d3) SHA1(c3efedda7ab947a06d9345f7b8261076fa7ceeef))
ROM_END

void msx2_state::nms8255(machine_config &config)
{
	// YM2149 (in S-3527 MSX Engine)
	// FDC: wd2793, 2 3.5" DSDD drives
	// 2 Cartridge slots
	// S-3527 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	// ROM is mirrored in all 4 pages
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom1", 3, 0, 1, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom2", 3, 0, 2, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom3", 3, 0, 3, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000).set_ramio_bits(0xf8);   // 128KB Mapper RAM
	// ROM is not mirrored but the FDC registers are in all pages
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1, "disk", 3, 3, 1, 2, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");

	msx_wd2793_force_ready(config);
	msx_2_35_dd_drive(config);
	msx2_pal(YM2149, config);
}

/* MSX2 - Philips NMS 8255/19 */

ROM_START(nms8255f)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("nms8255fbios.rom", 0x0000, 0x8000, BAD_DUMP CRC(5cd35ced) SHA1(b034764e6a8978db60b1d652917f5e24a66a7925)) // need verification

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("nms8255fext.rom",  0x0000, 0x4000, BAD_DUMP CRC(781ba055) SHA1(fd4bcc81a8160a1dea06036c5f79d200f948f4d6)) // need verification

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("nms8255fdisk.rom", 0x0000, 0x4000, BAD_DUMP CRC(13b60725) SHA1(58ba1887e8fd21c912b6859cae6514bd874ffcca)) // need verification
ROM_END

void msx2_state::nms8255f(machine_config &config)
{
	// YM2149 (in S-3527 MSX Engine)
	// FDC: wd2793, 2 3.5" DSDD drives
	// 2 Cartridge slots
	// S-3527 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000).set_ramio_bits(0xf8);   // 128KB Mapper RAM
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1, "disk", 3, 3, 1, 2, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");

	msx_wd2793_force_ready(config);
	msx_2_35_dd_drive(config);
	msx2_pal(YM2149, config);
}

/* MSX2 - Philips NMS 8260 */
/* Prototype created by JVC for Philips. Based on an NMS-8250 with the floppy drive removed and replaced with a 20MB JVC harddisk */

ROM_START(nms8260)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("nms8260bios.rom", 0x0000, 0x8000, CRC(6cdaf3a5) SHA1(6103b39f1e38d1aa2d84b1c3219c44f1abb5436e))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("nms8260ext.rom",  0x0000, 0x4000, CRC(66237ecf) SHA1(5c1f9c7fb655e43d38e5dd1fcc6b942b2ff68b02))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("nms8260disk.rom", 0x0000, 0x4000, CRC(ca3307d3) SHA1(c3efedda7ab947a06d9345f7b8261076fa7ceeef))

	ROM_REGION(0x4000, "hddrom", 0)
	ROM_LOAD("nms8260hdd.rom", 0x0000, 0x4000, CRC(0051afc3) SHA1(77f9fe964f6d8cb8c4af3b5fe63ce6591d5288e6))
ROM_END

void msx2_state::nms8260(machine_config &config)
{
	// YM2149 (in S-3527 MSX Engine)
	// FDC: wd2793, 0 3.5" DSDD drives
	// 2 Cartridge slots
	// S-3527 MSX Engine
	// HDD

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_ROM, "hddrom", 2, 1, 1, "hddrom");
	// ROM is mirrored in all 4 pages
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom1", 3, 0, 1, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom2", 3, 0, 2, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom3", 3, 0, 3, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000).set_ramio_bits(0xf8);   // 128KB Mapper RAM
	// ROM is not mirrored but the FDC registers are in all pages
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1, "disk", 3, 3, 1, 2, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");

	// There is actually only an FDC inside with a floppy controller to attach an external floppy drive
	msx_wd2793_force_ready(config);
	msx2_pal(YM2149, config);
}

/* MSX2 - Philips NMS 8280 - 5 possible sets (/00 /02 /09 /16 /19) */

ROM_START(nms8280)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("8280bios.rom.ic119", 0x0000, 0x8000, BAD_DUMP CRC(6cdaf3a5) SHA1(6103b39f1e38d1aa2d84b1c3219c44f1abb5436e)) // need verification

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("8280ext.rom.ic118",  0x0000, 0x4000, BAD_DUMP CRC(66237ecf) SHA1(5c1f9c7fb655e43d38e5dd1fcc6b942b2ff68b02)) // need verification

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("8280disk.rom.ic117", 0x0000, 0x4000, BAD_DUMP CRC(ca3307d3) SHA1(c3efedda7ab947a06d9345f7b8261076fa7ceeef)) // need verification
ROM_END

void msx2_state::nms8280(machine_config &config)
{
	// YM2149 (in S3527)
	// FDC: wd2793, 2 3.5" DSDD drives
	// 2 Cartridge slots
	// S3527

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000).set_ramio_bits(0xf8);   // 128KB Mapper RAM
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1, "disk", 3, 3, 1, 2, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");

	msx_wd2793_force_ready(config);
	msx_2_35_dd_drive(config);
	msx2_pal(YM2149, config);
}

/* MSX2 - Philips NMS 8280F */

ROM_START(nms8280f)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("8280fbios.rom", 0x0000, 0x8000, BAD_DUMP CRC(5cd35ced) SHA1(b034764e6a8978db60b1d652917f5e24a66a7925)) // need verification

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("8280fext.rom",  0x0000, 0x4000, BAD_DUMP CRC(781ba055) SHA1(fd4bcc81a8160a1dea06036c5f79d200f948f4d6)) // need verification

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("8280fdisk.rom", 0x0000, 0x4000, BAD_DUMP CRC(13b60725) SHA1(58ba1887e8fd21c912b6859cae6514bd874ffcca)) // need verification
ROM_END

void msx2_state::nms8280f(machine_config &config)
{
	// YM2149 (in S3527)
	// FDC: wd2793, 2 3.5" DSDD drives
	// 2 Cartridge slots
	// S3527

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000).set_ramio_bits(0xf8);   // 128KB Mapper RAM
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1, "disk", 3, 3, 1, 2, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");

	msx_wd2793_force_ready(config);
	msx_2_35_dd_drive(config);
	msx2_pal(YM2149, config);
}

/* MSX2 - Philips NMS 8280G */

ROM_START(nms8280g)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("8280gbios.rom.ic119", 0x0000, 0x8000, BAD_DUMP CRC(8fa060e2) SHA1(b17d9bea0eb16a1aa2d0ccbd7c9488da9f57698e)) // need verification

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("8280gext.rom.ic118", 0x0000, 0x4000, BAD_DUMP CRC(41e36d03) SHA1(4ab7b2030d022f5486abaab22aaeaf8aa23e05f3)) // need verification

	ROM_REGION(0x4000, "diskrom", 0)
	// FDC register contents at 3ff8-3fff
	ROM_LOAD("8280gdisk.rom.ic117", 0x0000, 0x4000, BAD_DUMP CRC(d0beebb8) SHA1(d1001f93c87ff7fb389e418e33bf7bc81bdbb65f)) // need verification
ROM_END

void msx2_state::nms8280g(machine_config &config)
{
	// YM2149 (in S3527)
	// FDC: wd2793, 2 3.5" DSDD drives
	// 2 Cartridge slots
	// S3527

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000).set_ramio_bits(0xf8);   // 128KB Mapper RAM
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1, "disk", 3, 3, 1, 2, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");

	msx_wd2793_force_ready(config);
	msx_2_35_dd_drive(config);
	msx2_pal(YM2149, config);
}

/* MSX2 - Philips VG-8230 (u11 - exp, u12 - basic, u13 - disk */

ROM_START(vg8230)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("8230bios.rom.u12", 0x0000, 0x8000, CRC(b31c851d) SHA1(0de3c802057560560a03d7965fcc4cff69f8575c))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("8230ext.rom.u11",  0x0000, 0x4000, CRC(8f84f783) SHA1(3288894e1be6af705871499b23c85732dbc40993))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("8230disk.rom.u13", 0x0000, 0x4000, CRC(7639758a) SHA1(0f5798850d11b316a4254b222ca08cc4ad6d4da2))
ROM_END

void msx2_state::vg8230(machine_config &config)
{
	// YM2149 (in S-3527 MSX Engine)
	// FDC: wd2793, 1 3.5" SSDD drive
	// 2 Cartridge slots
	// S-3527 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 2, 0, 4);   // 64KB RAM
	add_internal_disk(config, MSX_SLOT_DISK1, "disk", 3, 3, 1, 1, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");

	msx_wd2793_force_ready(config);
	msx_1_35_ssdd_drive(config);
	msx2_pal(YM2149, config);
}

/* MSX2 - Philips VG-8235 (/00 and /20) */
/* 9 versions:
 * /00 NL,BE QWERTY        2.0
 * /02 DE    QWERTZ        2.0
 * /16 ES    QWERTY with ñ 2.0
 * /19 FR,BE AZERTY        2.0
 * /20 NL,BE QWERTY        2.1
 * /22 DE    QWERTZ        2.1
 * /29 DE    QWERTZ        2.1
 * /36 ES    QWERTY with ñ 2.1
 * /39 FR,BE AZERTY        2.1
 */ 

ROM_START(vg8235)
	ROM_SYSTEM_BIOS(0, "r20", "VG8235/20")
	ROM_SYSTEM_BIOS(1, "r00", "VG8235/00")

	ROM_REGION(0x8000, "mainrom", 0)
	ROMX_LOAD("8235_20.u48", 0x0000, 0x8000, CRC(6cdaf3a5) SHA1(6103b39f1e38d1aa2d84b1c3219c44f1abb5436e), ROM_BIOS(0))
	ROMX_LOAD("8235_00.u48", 0x0000, 0x8000, CRC(f05ed518) SHA1(5e1a4bd6826b29302a1eb88c340477e7cbd0b50a), ROM_BIOS(1))

	ROM_REGION(0x4000, "subrom", 0)
	ROMX_LOAD("8235_20.u49", 0x0000, 0x4000, CRC(66237ecf) SHA1(5c1f9c7fb655e43d38e5dd1fcc6b942b2ff68b02), ROM_BIOS(0))
	ROMX_LOAD("8235_00.u49", 0x0000, 0x4000, CRC(474439d1) SHA1(c289dad246364e2dd716c457ca5eecf98e76c9ab), ROM_BIOS(1))

	ROM_REGION(0x4000, "diskrom", 0)
	// Different versions might exist and mixed between /00 and /20
	ROMX_LOAD("8235_20.u50", 0x0000, 0x4000, CRC(0efbea8a) SHA1(1ee2abc68a81ae7e39548985021b6532f31171b2), ROM_BIOS(0))
	ROMX_LOAD("8235_00.u50", 0x0000, 0x4000, CRC(f39342ce) SHA1(7ce255ab63ba79f81d8b83d66f1108062d0b61f1), ROM_BIOS(1))
ROM_END

void msx2_state::vg8235(machine_config &config)
{
	// YM2149 (in S-3527 MSX Engine)
	// FDC: wd2793, 1 3.5" SSDD drive
	// 2 Cartridge slots
	// S-3527 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000).set_ramio_bits(0xf8);   // 128KB Mapper RAM
	// Only the FDC registers are mirrored
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1, "disk", 3, 3, 1, 2, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");

	msx_wd2793_force_ready(config);
	msx_1_35_ssdd_drive(config);
	msx2_pal(YM2149, config);
}

/* MSX2 - Philips VG-8235F (/19 and /39) */

ROM_START(vg8235f)
	ROM_SYSTEM_BIOS(0, "r39", "VG8235/39")
	ROM_SYSTEM_BIOS(1, "r19", "VG8235/19")

	ROM_REGION(0x8000, "mainrom", 0)
	ROMX_LOAD("8235_39.u48", 0x0000, 0x8000, CRC(5cd35ced) SHA1(b034764e6a8978db60b1d652917f5e24a66a7925), ROM_BIOS(0))
	ROMX_LOAD("8235_19.u48", 0x0000, 0x8000, BAD_DUMP CRC(c0577a50) SHA1(3926cdd91fa89657a811463e48cfbdb350676e51), ROM_BIOS(1)) // need verification

	ROM_REGION(0x4000, "subrom", 0)
	ROMX_LOAD("8235_39.u49", 0x0000, 0x4000, CRC(781ba055) SHA1(fd4bcc81a8160a1dea06036c5f79d200f948f4d6), ROM_BIOS(0))
	ROMX_LOAD("8235_19.u49", 0x0000, 0x4000, BAD_DUMP CRC(e235d5c8) SHA1(792e6b2814ab783d06c7576c1e3ccd6a9bbac34a), ROM_BIOS(1)) // need verification

	ROM_REGION(0x4000, "diskrom", 0)
	ROMX_LOAD("8235_39.u50", 0x0000, 0x4000, CRC(768549a9) SHA1(959060445e92eb980ddd9df3ef9cfefcae2de1d0), ROM_BIOS(0))
	ROMX_LOAD("8235_19.u50", 0x0000, 0x4000, BAD_DUMP CRC(768549a9) SHA1(959060445e92eb980ddd9df3ef9cfefcae2de1d0), ROM_BIOS(1)) // need verification
ROM_END

void msx2_state::vg8235f(machine_config &config)
{
	// YM2149 (in S-3527 MSX Engine)
	// FDC: wd2793, 1 3.5" SSDD drive
	// 2 Cartridge slots
	// S-3527 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000).set_ramio_bits(0xf8);   // 128KB Mapper RAM
	// Only the FDC registers are mirrored
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1, "disk", 3, 3, 1, 2, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");

	msx_wd2793_force_ready(config);
	msx_1_35_ssdd_drive(config);
	msx2_pal(YM2149, config);
}

/* MSX2 - Philips VG-8240 (unreleased) */

ROM_START(vg8240)
	ROM_REGION(0x10000, "mainrom", 0)
	ROM_LOAD("8240bios.rom", 0x0000, 0x8000, CRC(6cdaf3a5) SHA1(6103b39f1e38d1aa2d84b1c3219c44f1abb5436e))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("8240ext.rom",  0x0000, 0x4000, CRC(66237ecf) SHA1(5c1f9c7fb655e43d38e5dd1fcc6b942b2ff68b02))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("8240disk.rom", 0x0000, 0x4000, CRC(ca3307d3) SHA1(c3efedda7ab947a06d9345f7b8261076fa7ceeef))
ROM_END

void msx2_state::vg8240(machine_config &config)
{
	// YM2149 (in S3527)
	// FDC: wd2793, 1 3.5" DSDD drive
	// 2 Cartridge slots?
	// S3527

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 2, 0, 4);   // 64KB RAM
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1, "disk", 3, 3, 1, 2, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");

	msx_wd2793_force_ready(config);
	msx_1_35_dd_drive(config);
	msx2_pal(YM2149, config);
}

/* MSX2 - Phonola NMS 8245 */

/* MSX2 - Phonola NMS 8280 */

/* MSX2 - Phonola VG-8235 */

/* MSX2 - Pioneer UC-V102 */

ROM_START(ucv102)
	ROM_REGION(0x8000, "mainrom", 0)
	// Machine is not supposed to display the MSX logo on startup
	ROM_LOAD("uc-v102bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(a27c563d) SHA1(c1e46c00f1e38fc9e0ab487bf0513bd93ce61f3f))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("uc-v102sub.rom", 0x0000, 0x4000, BAD_DUMP CRC(43e7a7fc) SHA1(0fbd45ef3dd7bb82d4c31f1947884f411f1ca344))

	ROM_REGION(0x4000, "rs232", ROMREGION_ERASEFF)
	ROM_LOAD("uc-v102rs232.rom", 0x0000, 0x2000, CRC(7c6790fc) SHA1(a4f19371fd09b73f2776cb637b0e9cbd8415f8eb))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("uc-v102disk.rom", 0x0000, 0x4000, CRC(a90be8d5) SHA1(f7c3ac138918a493eb91628ed88cf37999059579))

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("kanjifont.rom", 0x0000, 0x20000, CRC(d23d4d2d) SHA1(db03211b7db46899df41db2b1dfbec972109a967))
ROM_END

void msx2_state::ucv102(machine_config &config)
{
	// YM2149 (in S1985)
	// FDC: wd1793, 2 3.5" DSDD drives
	// 1 Cartridge slots
	// S1985
	// RS232

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 1, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 0, 2, 0, 4).set_total_size(0x10000);   // 64KB Mapper RAM
	add_cartridge_slot<1>(config, 1);
	add_internal_slot_irq<2>(config, MSX_SLOT_RS232, "rs232", 2, 1, 1, "rs232");
	// Expansion slot 1 connects to slots 2-1 and 3-1 (2x 50 pin)
	// Expansion slot 2 connects to slots 2-2 and 3-2 (2x 50 pin)
	// Expansion slot 3 connects to slots 2-3 and 3-3 (2x 50 pin)
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1, "disk", 3, 1, 2, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");

	MSX_S1985(config, "s1985", 0);

	msx_fd1793(config); // Mitsubishi MSW1793
	msx_1_35_dd_drive(config);
	m_hw_def.has_cassette(false);
	msx2(YM2149, config);
}

/* MSX2 - Sakhr AX-350 */

ROM_START(ax350)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("ax350bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(ea306155) SHA1(35195ab67c289a0b470883464df66bc6ea5b00d3)) // need verification

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("ax350ext.rom", 0x0000, 0x4000, BAD_DUMP CRC(7c7540b7) SHA1(ebb76f9061e875365023523607db610f2eda1d26)) // need verification

	ROM_REGION(0x8000, "arabic", 0)
	ROM_LOAD("ax350arab.rom", 0x0000, 0x8000, BAD_DUMP CRC(c0d8fc85) SHA1(2c9600c6e0025fee10d249e97448ecaa37e38c42)) // need verification

	ROM_REGION(0x8000, "swp", 0)
	ROM_LOAD("ax350swp.rom", 0x0000, 0x8000, BAD_DUMP CRC(076f40fc) SHA1(4b4508131dca6d811694ae6379f41364c477de58)) // need verification

	ROM_REGION(0x10000, "painter", 0)
	ROM_LOAD("ax350paint.rom", 0x0000, 0x10000, BAD_DUMP CRC(18956e3a) SHA1(ace202e87337fbc54fea21e22c0b3af0abe6f4ae)) // need verification

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("ax350disk.rom", 0x0000, 0x4000, BAD_DUMP CRC(1e7d6512) SHA1(78cd7f847e77fd8cd51a647efb2725ba93f4c471)) // need verification
ROM_END

void msx2_state::ax350(machine_config &config)
{
	// YM2149 (in S1985)
	// FDC: wd2793/tc8566af?, 1 3.5" DSDD drive
	// 2 Cartridge slots
	// S1985 

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 1, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "arabic", 0, 1, 1, 2, "arabic");
	add_internal_slot(config, MSX_SLOT_ROM, "swp", 0, 2, 1, 2, "swp");
	add_internal_slot(config, MSX_SLOT_ROM, "painter", 0, 3, 0, 4, "painter");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_disk_mirrored(config, MSX_SLOT_DISK2, "disk", 3, 1, 1, 2, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000).set_ramio_bits(0xf8);   // 128KB Mapper RAM

	MSX_S1985(config, "s1985", 0);

	msx_wd2793_force_ready(config);
	msx_1_35_dd_drive(config);
	msx2_pal(YM2149, config);
}

/* MSX2 - Sakhr AX-350 II */

ROM_START(ax350ii)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("ax350iibios.rom", 0x0000, 0x8000, CRC(ea306155) SHA1(35195ab67c289a0b470883464df66bc6ea5b00d3))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("ax350iiext.rom", 0x0000, 0x4000, CRC(7c7540b7) SHA1(ebb76f9061e875365023523607db610f2eda1d26))

	ROM_REGION(0x8000, "arabic", 0)
	ROM_LOAD("ax350iiarab.rom", 0x0000, 0x8000, CRC(e62f9bc7) SHA1(f8cd4c05083decfc098cff077e055a4ae1e91a73))

	ROM_REGION(0x8000, "swp", 0)
	ROM_LOAD("ax350iiword.rom", 0x0000, 0x8000, CRC(307ae37c) SHA1(3a74e73b94d066b0187feb743c5eceddf0c61c2b))

	ROM_REGION(0x10000, "painter", 0)
	ROM_LOAD("ax350iipaint.rom", 0x0000, 0x10000, CRC(18956e3a) SHA1(ace202e87337fbc54fea21e22c0b3af0abe6f4ae))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("ax350iidisk.rom", 0x0000, 0x4000, CRC(d07782a6) SHA1(358e69f427390041b5aa28018550a88f996bddb6))
ROM_END

void msx2_state::ax350ii(machine_config &config)
{
	// YM2149 (in S1985)
	// FDC: wd2793/tc8566af?, 1 3.5" DSDD drive (mb8877a in pcb picture)
	// 2 Cartridge slots
	// S1985

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 1, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "arabic", 0, 1, 1, 2, "arabic");
	add_internal_slot(config, MSX_SLOT_ROM, "swp", 0, 2, 1, 2, "swp");
	add_internal_slot(config, MSX_SLOT_ROM, "painter", 0, 3, 0, 4, "painter");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	// mirroring not confirmed
	add_internal_disk_mirrored(config, MSX_SLOT_DISK8, "disk", 3, 1, 1, 2, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000).set_ramio_bits(0xf8);   // 128KB Mapper RAM

	MSX_S1985(config, "s1985", 0);

	msx_mb8877a(config);
	msx_1_35_dd_drive(config);
	msx2_pal(YM2149, config);
}

/* MSX2 - Sakhr AX-350 II F */

ROM_START(ax350iif)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("ax350iifbios.rom", 0x0000, 0x8000, CRC(5cd35ced) SHA1(b034764e6a8978db60b1d652917f5e24a66a7925))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("ax350iifext.rom", 0x0000, 0x4000, CRC(16afa2e9) SHA1(4cbceba8f37f08272b612b6fc212eeaf379da9c3))

	ROM_REGION(0x8000, "arabic", 0)
	ROM_LOAD("ax350iifarab.rom", 0x0000, 0x8000, CRC(a64c3192) SHA1(5077b9c86ce1dc0a22c71782dac7fb3ca2a467e0))

	ROM_REGION(0x8000, "swp", 0)
	ROM_LOAD("ax350iifword.rom", 0x0000, 0x8000, CRC(097fd8ca) SHA1(54ff13b58868018fcd43c916b8d7c7200ebdcabe))

	ROM_REGION(0x10000, "painter", 0)
	ROM_LOAD("ax350iifpaint.rom", 0x0000, 0x10000, CRC(18956e3a) SHA1(ace202e87337fbc54fea21e22c0b3af0abe6f4ae))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("ax350iifdisk.rom", 0x0000, 0x4000, CRC(5eb46cd5) SHA1(bd0ad648d728c691fcee08eaaaa95e15e29c0d0d))
ROM_END

void msx2_state::ax350iif(machine_config &config)
{
	// YM2149 (in S1985)
	// FDC: wd2793/tc8566af?, 1 3.5" DSDD drive (mb8877a in pcb picture)
	// 2 Cartridge slots
	// S1985 

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 1, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "arabic", 0, 1, 1, 2, "arabic");
	add_internal_slot(config, MSX_SLOT_ROM, "swp", 0, 2, 1, 2, "swp");
	add_internal_slot(config, MSX_SLOT_ROM, "painter", 0, 3, 0, 4, "painter");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	// mirroring not confirmed
	add_internal_disk_mirrored(config, MSX_SLOT_DISK8, "disk", 3, 1, 1, 2, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000).set_ramio_bits(0xf8);   // 128KB Mapper RAM

	MSX_S1985(config, "s1985", 0);

	msx_mb8877a(config);
	msx_1_35_dd_drive(config);
	msx2_pal(YM2149, config);
}

/* MSX2 - Sakhr AX-370 */

ROM_START(ax370)
	ROM_REGION(0x30000, "mainrom", 0)
	ROM_LOAD("ax370bios.rom", 0x0000, 0x8000, CRC(ea306155) SHA1(35195ab67c289a0b470883464df66bc6ea5b00d3))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("ax370ext.rom", 0x0000, 0x4000, CRC(3c011d12) SHA1(ee9c6a073766bef2220a57372f5c0dbfc6e55c8c))

	ROM_REGION(0x8000, "arabic", 0)
	ROM_LOAD("ax370arab.rom", 0x0000, 0x8000, CRC(66c2c71e) SHA1(0c08e799a7cf130ae2b9bc93f28bd4959cee6fdc))

	ROM_REGION(0x8000, "swp", 0)
	ROM_LOAD("ax370swp.rom", 0x0000, 0x8000, CRC(076f40fc) SHA1(4b4508131dca6d811694ae6379f41364c477de58))

	ROM_REGION(0x8000, "sakhr", 0)
	ROM_LOAD("ax370sakhr.rom", 0x0000, 0x8000, CRC(71a356ce) SHA1(8167117a003824220c36775682acbb36b3733c5e))

	ROM_REGION(0x10000, "painter", 0)
	ROM_LOAD("ax370paint.rom", 0x00000, 0x10000, CRC(0394cb41) SHA1(1c9a5867d39f6f02a0a4ef291904623e2521c2c5))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("ax370disk.rom", 0x0000, 0x4000, CRC(db7f1125) SHA1(9efa744be8355675e7bfdd3976bbbfaf85d62e1d))
ROM_END

void msx2_state::ax370(machine_config &config)
{
	// AY8910 (in T9769B)
	// FDC: tc8566af, 1 3.5" DSDD drive
	// 2 Cartridge slots
	// V9958
	// T9769B

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "arabic", 0, 1, 1, 2, "arabic");
	add_internal_slot(config, MSX_SLOT_ROM, "swp", 0, 2, 1, 2, "swp");
	add_internal_slot(config, MSX_SLOT_ROM, "sakhr", 0, 3, 1, 2, "sakhr");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK3, "disk", 3, 0, 1, 2, "diskrom");
	add_internal_slot(config, MSX_SLOT_ROM, "painter", 3, 1, 0, 4, "painter");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 3, 0, 4).set_total_size(0x20000).set_ramio_bits(0xf8);   // 128KB Mapper RAM

	msx2plus_pal(AY8910, config);
}

/* MSX2 - Sakhr AX-500 */

ROM_START(ax500)
	ROM_REGION(0x30000, "mainrom", 0)
	ROM_LOAD("ax500bios.rom", 0x0000, 0x8000, CRC(0a6e2e13) SHA1(dd1b577ea3ea69de84a68d311261392881f9eac3))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("ax500ext.rom", 0x0000, 0x4000, CRC(c9186a21) SHA1(7f86af13e81259a0db8f70d8a7e026fb918ee652))

	ROM_REGION(0x8000, "arabic", 0)
	ROM_LOAD("ax500arab.rom", 0x0000, 0x8000, CRC(11830686) SHA1(92bac0b2995f54f0eebf167cd447361a6a4923eb))

	ROM_REGION(0x8000, "swp", 0)
	ROM_LOAD("ax500swp.rom", 0x0000, 0x8000, CRC(17ed0610) SHA1(8674d000a52ec01fd80c8cb7cbaa66d4c3ca5cf7))

	ROM_REGION(0xc000, "sakhr", 0)
	ROM_LOAD("ax500sakhr.rom", 0x0000, 0xc000, CRC(bee11490) SHA1(8e889999ecec302f05d3bd0a0f127b489fcf3739))

	ROM_REGION(0x10000, "painter", 0)
	ROM_LOAD("ax500paint.rom", 0x00000, 0x10000, CRC(c32ea7ec) SHA1(80872d997d18e1a633e70b9da35a0d28113073e5))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("ax500disk.rom", 0x0000, 0x4000, CRC(a7d7746e) SHA1(a953bef071b603d6280bdf7ab6249c2e6f1a4cd8))
ROM_END

void msx2_state::ax500(machine_config &config)
{
	// YM2149 (in S9185)
	// FDC: wd2793?, 2 3.5" DSDD drives
	// 2 Cartridge slots
	// S1985

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "arabic", 0, 1, 1, 2, "arabic");
	add_internal_slot(config, MSX_SLOT_ROM, "swp", 0, 2, 1, 2, "swp");
	add_internal_slot(config, MSX_SLOT_ROM, "sakhr", 0, 3, 1, 2, "sakhr");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK8, "disk", 3, 0, 1, 2, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");
	add_internal_slot(config, MSX_SLOT_ROM, "painter", 3, 1, 0, 4, "painter");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x40000).set_ramio_bits(0xf8);   // 256KB Mapper RAM
	add_cartridge_slot<3>(config, MSX_SLOT_YAMAHA_EXPANSION, "module", 3, 3, msx_yamaha_60pin, nullptr);

	MSX_S1985(config, "s1985", 0);

	msx_wd2793_force_ready(config);
	msx_2_35_dd_drive(config);
	msx2_pal(YM2149, config);
}

/* MSX2 - Sanyo MPC-2300 */

ROM_START(mpc2300)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("2300bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(e7d08e29) SHA1(0f851ee7a1cf79819f61cc89e9948ee72a413802)) // need verification

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("2300ext.rom", 0x0000, 0x4000, BAD_DUMP CRC(3d7dc718) SHA1(e1f834b28c3ee7c9f79fe6fbf2b23c8a0617892b)) // need verification
ROM_END

void msx2_state::mpc2300(machine_config &config)
{
	// YM2149 (in S1985)
	// FDC: None, 0 drives
	// 2 Cartridge slots?
	// S1985

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 0, 0, 4).set_total_size(0x20000);   // 128KB?? Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 1, 0, 1, "subrom");

	MSX_S1985(config, "s1985", 0);

	msx2(YM2149, config);
}

/* MSX2 - Sanyo MPC-2500FD */

ROM_START(mpc2500f)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("mpc2500fdbios.rom", 0x0000, 0x8000, BAD_DUMP CRC(e7d08e29) SHA1(0f851ee7a1cf79819f61cc89e9948ee72a413802)) // need verification

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("mpc2500fdext.rom",  0x0000, 0x4000, BAD_DUMP CRC(3d7dc718) SHA1(e1f834b28c3ee7c9f79fe6fbf2b23c8a0617892b)) // need verification

	ROM_REGION(0x4000, "diskrom", 0)
	// FDC register contents at 3ff8-3fff
	ROM_LOAD("mpc2500fddisk.rom", 0x0000, 0x4000, BAD_DUMP CRC(38454059) SHA1(58ac78bba29a06645ca8d6a94ef2ac68b743ad32)) // need verification
ROM_END

void msx2_state::mpc2500f(machine_config &config)
{
	// YM2149
	// FDC: wd2793?, 1? 3.5" DSDD drive?
	// 2 Cartridge slots?
	// S-3527 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 2, 0, 1, "subrom");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1, "disk", 3, 1, 1, 2, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x10000);   // 64KB?? Mapper RAM

	msx_wd2793_force_ready(config);
	msx_1_35_dd_drive(config);
	msx2(YM2149, config);
}

/* MSX2 - Sanyo MPC-25F */

/* MSX2 - Sanyo MPC-25FD */

ROM_START(mpc25fd)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("25fdbios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("25fdext.rom",  0x0000, 0x4000, CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("25fddisk.rom", 0x0000, 0x4000, CRC(1a91f241) SHA1(bdbc75aacba4ea0f359694f304ae436914733460))
ROM_END

void msx2_state::mpc25fd(machine_config &config)
{
	// YM2149 (in S-3527 MSX Engine)
	// FDC: wd2793, 1 drive
	// 1 Cartridge slot (slot 1)
	// S-3527 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 2, 3, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom_mirror1", 2, 3, 1, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom_mirror2", 2, 3, 2, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom_mirror3", 2, 3, 3, 1, "subrom");
	// Mirrored in all 4 pages (only rom or also registers?)
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1, "disk", 3, 1, 1, 2, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 2, 0, 4);   // 64KB RAM

	msx_wd2793_force_ready(config);
	msx_1_35_dd_drive(config);
	msx2(YM2149, config);
}

/* MSX2 - Sanyo MPC-25FK */

/* MSX2 - Sanyo MPC-25FS */

ROM_START(mpc25fs)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("25fdbios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("25fdext.rom",  0x0000, 0x4000, CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("25fddisk.rom", 0x0000, 0x4000, CRC(0fa7b10e) SHA1(50f4098a77e7af7093e29cc8683d2b34b2d07b13))
ROM_END

void msx2_state::mpc25fs(machine_config &config)
{
	// YM2149 (in S-3527 MSX Engine)
	// FDC: wd2793, 1 drive
	// 1 Cartridge slot (slot 1)
	// S-3527 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 2, 3, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom_mirror1", 2, 3, 1, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom_mirror2", 2, 3, 2, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom_mirror3", 2, 3, 3, 1, "subrom");
	// Mirrored in all 4 pages (only rom or also registers?)
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1, "disk", 3, 1, 1, 2, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 2, 0, 4);   // 64KB RAM

	msx_wd2793_force_ready(config);
	msx_1_35_ssdd_drive(config);
	msx2(YM2149, config);
}

/* MSX2 - Sanyo Wavy MPC-27 */

ROM_START(mpc27)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("mpc27bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(ba81b3dd) SHA1(4ce41fcc1a603411ec4e99556409c442078f0ecf)) // need verification

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("mpc27ext.rom", 0x0000, 0x4000, BAD_DUMP CRC(90ca25b5) SHA1(fd9fa78bac25aa3c0792425b21d14e364cf7eea4)) // need verificaiton

	ROM_REGION(0x4000, "diskrom", 0)
	// FDC register contents at 3ff8-3fff
	ROM_LOAD("mpc27disk.rom", 0x0000, 0x4000, BAD_DUMP CRC(38454059) SHA1(58ac78bba29a06645ca8d6a94ef2ac68b743ad32)) // need verification

	ROM_REGION(0x4000, "lpen", 0)
	ROM_LOAD("mlp27.rom", 0x0000, 0x2000, BAD_DUMP CRC(8f9e6ba0) SHA1(c3a47480c9dd2235f40f9a53dab68e3c48adca01)) // need verification
	ROM_RELOAD(0x2000, 0x2000)
ROM_END

void msx2_state::mpc27(machine_config &config)
{
	// YM2149 (in S-3527 MSX Engine)
	// FDC: wd2793?, 1 drive
	// 2 Cartridge slots?
	// S-3527 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 0, 0, 4).set_total_size(0x20000);   // 128KB?? RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 1, 0, 1, "subrom");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1, "disk", 3, 2, 1, 2, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");
	add_internal_slot(config, MSX_SLOT_ROM, "lpen", 3, 3, 1, 1, "lpen");

	msx_wd2793_force_ready(config);
	msx_1_35_ssdd_drive(config);
	msx2(YM2149, config);
}

/* MSX2 - Sanyo PCT-100 */

/* MSX2 - Sanyo PHC-23 - "Wavy23" (and PHC-23J with different keyboard layout) */

ROM_START(phc23)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("23bios.rom", 0x0000, 0x8000, CRC(ba81b3dd) SHA1(4ce41fcc1a603411ec4e99556409c442078f0ecf))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("23ext.rom", 0x0000, 0x4000, CRC(90ca25b5) SHA1(fd9fa78bac25aa3c0792425b21d14e364cf7eea4))
ROM_END

void msx2_state::phc23(machine_config &config)
{
	// YM2149 (in S3527 MSX Engine)
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// S3527 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 2, 0, 4);  // 64KB RAM

	msx2(YM2149, config);
}

/* MSX2 - Sanyo PHC-23J(B) / PHC-23J(GR) - "Wavy23" */

ROM_START(phc23jb)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("23bios.rom", 0x0000, 0x8000, CRC(ba81b3dd) SHA1(4ce41fcc1a603411ec4e99556409c442078f0ecf))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("23ext.rom", 0x0000, 0x4000, CRC(90ca25b5) SHA1(fd9fa78bac25aa3c0792425b21d14e364cf7eea4))
ROM_END

void msx2_state::phc23jb(machine_config &config)
{
	// YM2149 (in S-1985 MSX Engine)
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// S-1985 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 2, 0, 4);  // 64KB RAM

	MSX_S1985(config, "s1985", 0);
	msx2(YM2149, config);
}

/* MSX2 - Sanyo Wavy PHC-55FD2 */

ROM_START(phc55fd2)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("phc55fd2bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(ba81b3dd) SHA1(4ce41fcc1a603411ec4e99556409c442078f0ecf)) // need verification

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("phc55fd2ext.rom", 0x0000, 0x4000, BAD_DUMP CRC(90ca25b5) SHA1(fd9fa78bac25aa3c0792425b21d14e364cf7eea4)) // need verification

	ROM_REGION(0x4000, "diskrom", 0)
	// FDC register contents at 3ff8-3fff
	ROM_LOAD("phc55fd2disk.rom", 0x0000, 0x4000, BAD_DUMP CRC(38454059) SHA1(58ac78bba29a06645ca8d6a94ef2ac68b743ad32)) // need verification
ROM_END

void msx2_state::phc55fd2(machine_config &config)
{
	// YM2149 (in S-1985 MSX Engine)
	// FDC: wd2793?, 2 3.5" DSDD drives
	// 2 Cartridge slots
	// S-1985 MSX Engine
	// T9763

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 0, 0, 4).set_total_size(0x20000);   // 128KB RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 1, 0, 1, "subrom");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1, "disk", 3, 2, 1, 2, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");

	MSX_S1985(config, "s1985", 0);

	msx_wd2793_force_ready(config);
	msx_2_35_dd_drive(config);
	msx2(YM2149, config);
}

/* MSX2 - Sanyo Wavy PHC-77 */

ROM_START(phc77)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("phc77bios.rom", 0x0000, 0x8000, CRC(ba81b3dd) SHA1(4ce41fcc1a603411ec4e99556409c442078f0ecf))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("phc77ext.rom", 0x0000, 0x4000, CRC(43e7a7fc) SHA1(0fbd45ef3dd7bb82d4c31f1947884f411f1ca344))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("phc77disk.rom", 0x0000, 0x4000, BAD_DUMP CRC(7c79759a) SHA1(a427b0c9a344c87b587568ecca7fee0abbe72189)) // Floppy registers visible in dump

	ROM_REGION(0x80000, "msxwrite", 0)
	ROM_LOAD("phc77msxwrite.rom", 0x00000, 0x80000, CRC(ef02e4f3) SHA1(4180544158a57c99162269e33e4f2c77c9fce84e))

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("phc77kfn.rom", 0x0000, 0x20000, CRC(d23d4d2d) SHA1(db03211b7db46899df41db2b1dfbec972109a967))
ROM_END

void msx2_state::phc77(machine_config &config)
{
	// YM2149 (in S-1985 MSX Engine)
	// FDC: wd2793?, 1 drive; looks like mb8877a
	// 1 Cartridge slot
	// S-1985 MSX Engine
	// SRAM?
	// Builtin printer
	// Switch to turn off firmware

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_MSX_WRITE, "msxwrite", 2, 1, 2, "msxwrite");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_disk(config, MSX_SLOT_DISK9, "disk", 3, 0, 1, 1, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");
	add_cartridge_slot<2>(config, 3, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 3, 0, 4);   // 64KB RAM

	MSX_S1985(config, "s1985", 0);

	msx_wd2793(config);
	msx_1_35_dd_drive(config);
	msx2(YM2149, config);
}

/* MSX2 - Sharp Epcom HotBit 2.0 - is this an officially released machine? */

ROM_START(hotbit20)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hb2bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(0160e8c9) SHA1(d0cfc35f22b150a1cb10decae4841dfe63b78251)) // need verification

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("hb2ext.rom", 0x0000, 0x4000, BAD_DUMP CRC(08ced880) SHA1(4f2a7e0172f0214f025f23845f6e053d0ffd28e8)) // need verification

	ROM_REGION(0x4000, "xbasic", 0)
	ROM_LOAD("xbasic2.rom", 0x0000, 0x4000, BAD_DUMP CRC(2825b1a0) SHA1(47370bec7ca1f0615a54eda548b07fbc0c7ef398)) // need verification

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("microsoldisk.rom", 0x0000, 0x4000, BAD_DUMP CRC(6704ef81) SHA1(a3028515ed829e900cc8deb403e17b09a38bf9b0)) // need verification
ROM_END

void msx2_state::hotbit20(machine_config &config)
{
	// AY8910/YM2149?
	// FDC: microsol, 1 or 2 drives?
	// 2 Cartridge slots?

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1, 0);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 1, 1, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "xbasic", 1, 1, 1, 1, "xbasic");
	add_internal_disk(config, MSX_SLOT_DISK5, "disk", 1, 3, 1, 1, "diskrom").set_tags("fdc", "fdc:0", "fdc:1", "fdc:2", "fdc:3");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 2, 0, 4).set_total_size(0x20000).set_ramio_bits(0x80);   // 128KB Mapper RAM
	add_cartridge_slot<2>(config, 3);

	msx_microsol(config);
	msx_1_35_dd_drive(config);
	msx2_pal(AY8910, config);
}

/* MSX2 - Sony HB-F1 */

ROM_START(hbf1)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("f1bios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("f1ext.rom", 0x0000, 0x4000, CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33))

	ROM_REGION(0x4000, "firmware1", 0)
	ROM_LOAD("f1note1.rom", 0x0000, 0x4000, CRC(84810ea8) SHA1(9db72bb78792595a12499c821048504dc96ef848))

	ROM_REGION(0x8000, "firmware2", 0)
	ROM_LOAD("f1note2.rom", 0x0000, 0x8000, CRC(e32e5ee0) SHA1(aa78fc9bcd2343f84cf790310a768ee47f90c841))

	ROM_REGION(0x8000, "firmware3", 0)
	ROM_LOAD("f1note3.rom", 0x0000, 0x8000, CRC(73eb9329) SHA1(58accf41a90693874b86ce98d8d43c27beb8b6dc))
ROM_END

void msx2_state::hbf1(machine_config &config)
{
	// YM2149 (in S1985)
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// S1985
	// pause button

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware1", 3, 0, 1, 1, "firmware1");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware2", 3, 1, 1, 2, "firmware2");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware3", 3, 2, 1, 2, "firmware3");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 3, 0, 4);  // 64KB RAM

	MSX_S1985(config, "s1985", 0);

	msx2(YM2149, config);
}

/* MSX2 - Sony HB-F1II */

ROM_START(hbf1ii)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("f12bios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("f12ext.rom", 0x0000, 0x4000, CRC(43e7a7fc) SHA1(0fbd45ef3dd7bb82d4c31f1947884f411f1ca344))

	ROM_REGION(0x4000, "firmware1", 0)
	ROM_LOAD("f12note1.rom", 0x0000, 0x4000, CRC(dcacf970) SHA1(30d914cda2180889a40a3328e0a0c1327f4eaa10))

	ROM_REGION(0x8000, "firmware2", 0)
	ROM_LOAD("f12note2.rom", 0x0000, 0x8000, CRC(b0241a61) SHA1(ed2fea5c2a3c2e58d4f69f9d636e08574486a2b1))

	ROM_REGION(0x8000, "firmware3", 0)
	ROM_LOAD("f12note3.rom", 0x0000, 0x8000, CRC(44a10e6a) SHA1(917d1c079e03c4a44de864f123d03c4e32c8daae))
ROM_END

void msx2_state::hbf1ii(machine_config &config)
{
	// YM2149 (in S1985)
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// S1985
	// rensha-turbo slider

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware1", 3, 0, 1, 1, "firmware1");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware2", 3, 1, 1, 2, "firmware2");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware3", 3, 2, 1, 2, "firmware3");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 3, 0, 4);   // 64KB RAM

	MSX_S1985(config, "s1985", 0);

	msx2(YM2149, config);
}

/* MSX2 - Sony HB-F1XD  / HB-F1XDmk2 */
/* HB-F1XDmk2 is a cost-reduced version of HB-F1XD but identical in emulation */

ROM_START(hbf1xd)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("f1xdbios.rom.ic27", 0x0000, 0x8000, CRC(ba81b3dd) SHA1(4ce41fcc1a603411ec4e99556409c442078f0ecf))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("f1xdext.rom.ic27", 0x0000, 0x4000, CRC(43e7a7fc) SHA1(0fbd45ef3dd7bb82d4c31f1947884f411f1ca344))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("f1xddisk.rom.ic27", 0x0000, 0x4000, CRC(54c73ad6) SHA1(12f2cc79b3d09723840bae774be48c0d721ec1c6))
ROM_END

void msx2_state::hbf1xd(machine_config &config)
{
	// YM2149 (in S-1895 MSX Engine)
	// FDC: wd2793, 1 3.5" DSDD drive
	// 2 Cartridge slots
	// S-1985 MSX Engine
	// pause button
	// speed controller slider
	// rensha turbo slider

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1, "disk", 3, 0, 1, 2, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 3, 0, 4);   // 64KB RAM

	MSX_S1985(config, "s1985", 0);

	msx_wd2793(config);
	msx_1_35_dd_drive(config);
	msx2(YM2149, config);
}

/* MSX2 - Sony HB-F5 */

ROM_START(hbf5)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hbf5bios.ic25", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("hbf5ext.rom", 0x0000, 0x4000, CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33))

	ROM_REGION(0x4000, "firmware", 0)
	ROM_LOAD("hbf5note.rom", 0x0000, 0x4000, CRC(0cdc0777) SHA1(06ba91d6732ee8a2ecd5dcc38b0ce42403d86708))
ROM_END

void msx2_state::hbf5(machine_config &config)
{
	// YM2149
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 1, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 0, 1, 1, 1, "firmware");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 2, 0, 4);   // 64KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);

	msx2_pal(YM2149, config);
}

/* MSX2 - Sony HB-F500 */

ROM_START(hbf500)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("f500bios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("f500ext.rom", 0x0000, 0x4000, CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("f500disk.rom", 0x0000, 0x4000, CRC(f7f5b0ea) SHA1(e93b8da1e8dddbb3742292b0e5e58731b90e9313))

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("f500kfn.rom", 0, 0x20000, CRC(5a59926e) SHA1(6acaf2eeb57f65f7408235d5e07b7563229de799))
ROM_END

void msx2_state::hbf500(machine_config &config)
{
	// YM2149 (in S3527)
	// FDC: wd2793, 1 3.5" DSDD drive
	// 2 Cartridge slots
	// S3527

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram1", 0, 0, 2, 2);   // 32KB RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 1, 0, 1, "subrom");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1, "disk", 0, 1, 1, 2, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");
	add_internal_slot(config, MSX_SLOT_RAM, "ram2", 0, 2, 0, 2);   // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);

	msx_wd2793_force_ready(config);
	msx_1_35_dd_drive(config);
	msx2(YM2149, config);
}

/* MSX2 - Sony HB-F500 2nd version (slot layout is different) */

ROM_START(hbf500_2)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("f500bios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("f500ext.rom", 0x0000, 0x4000, CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("f500disk.rom", 0x0000, 0x4000, CRC(f7f5b0ea) SHA1(e93b8da1e8dddbb3742292b0e5e58731b90e9313))

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("f500kfn.rom", 0, 0x20000, CRC(5a59926e) SHA1(6acaf2eeb57f65f7408235d5e07b7563229de799))
ROM_END

void msx2_state::hbf500_2(machine_config &config)
{
	// YM2149 (in S3527)
	// FDC: wd2793, 1 3.5" DSDD drive
	// 2 Cartridge slots
	// S3527

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 1, 0, 1, "subrom");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1, "disk", 3, 1, 1, 2, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 2, 0, 4);   // 64KB RAM

	msx_wd2793_force_ready(config);
	msx_1_35_dd_drive(config);
	msx2(YM2149, config);
}

/* MSX2 - Sony HB-F500F */

ROM_START(hbf500f)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hbf500fbios.rom", 0x0000, 0x8000, BAD_DUMP CRC(440dae3c) SHA1(fedd9b682d056ddd1e9b3d281723e12f859b2e69)) // need verification

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("hbf500fext.rom", 0x0000, 0x4000, BAD_DUMP CRC(e235d5c8) SHA1(792e6b2814ab783d06c7576c1e3ccd6a9bbac34a)) // need verification

	ROM_REGION(0x4000, "diskrom", 0)
	// FDC register contents at 3ff8-3fff
	ROM_LOAD("hbf500fdisk.rom", 0x0000, 0x4000, BAD_DUMP CRC(6e718f5c) SHA1(0e081572f84555dc13bdb0c7044a19d6c164d985)) // need verification
ROM_END

void msx2_state::hbf500f(machine_config &config)
{
	// YM2149 (in S3527)
	// FDC: wd2793, 1 3.5" DSDD drive
	// 3 Cartridge slots
	// S3527

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram1", 0, 0, 2, 2);   // 32KB RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 1, 0, 1, "subrom");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1, "disk", 0, 1, 1, 2, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");
	add_internal_slot(config, MSX_SLOT_RAM, "ram2", 0, 2, 0, 2);   // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_cartridge_slot<3>(config, 3);

	msx_wd2793(config);
	msx_1_35_dd_drive(config);
	msx2_pal(YM2149, config);
}

/* MSX2 - Sony HB-F500P */

ROM_START(hbf500p)
	ROM_REGION(0x10000, "mainrom", 0)
	ROM_LOAD("500pbios.rom.ic41", 0x0000, 0x8000, CRC(b31c851d) SHA1(0de3c802057560560a03d7965fcc4cff69f8575c))
	// FDC register contents at 7ff8-7fff
	ROM_LOAD("500pext.ic47",      0x8000, 0x8000, BAD_DUMP CRC(cdd4824a) SHA1(505031f1e8396a6e0cb11c1540e6e7f6999d1191))
ROM_END

void msx2_state::hbf500p(machine_config &config)
{
	// YM2149 (in S3527)
	// FDC: wd2793, 1 3.5" DSDD drive
	// 3 Cartridge slots
	// S3527

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram1", 0, 0, 2, 2);   // 32KB RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 1, 0, 1, "mainrom", 0x8000);
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1, "disk", 0, 1, 1, 2, "mainrom", 0xc000).set_tags("fdc", "fdc:0", "fdc:1");
	add_internal_slot(config, MSX_SLOT_RAM, "ram2", 0, 2, 0, 2);   // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_cartridge_slot<3>(config, 3);

	msx_wd2793(config);
	msx_1_35_dd_drive(config);
	msx2_pal(YM2149, config);
}

/* MSX2 - Sony HB-F700D */

ROM_START(hbf700d)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("700dbios.rom.ic5", 0x0000, 0x8000, CRC(e975aa79) SHA1(cef16eb95502ba6ab2265fcafcedde470a101541))

	ROM_REGION(0x8000, "extrom", 0)
	// dumps as listed in openMSX and blueMSX
	//	ROM_LOAD("700dsub.ic6", 0x0000, 0x4000, CRC(8f84f783) SHA1(3288894e1be6af705871499b23c85732dbc40993))
	//	ROM_LOAD("700ddisk.ic6", 0x4000, 0x4000, CRC(54c73ad6) SHA1(12f2cc79b3d09723840bae774be48c0d721ec1c6))
	//
	// however according to the service manual these should be in the same rom chip
	// concatenation of 3288894e1be6af705871499b23c85732dbc40993 and 12f2cc79b3d09723840bae774be48c0d721ec1c6
	ROM_LOAD("700dext.ic6", 0x0000, 0x8000, BAD_DUMP CRC(2aba42dc) SHA1(9dee68aab6c921b0b20862a3f2f4e38ff8d155c0)) // to be verified with direct dump
ROM_END

void msx2_state::hbf700d(machine_config &config)
{
	// YM2149 (in S-1985 MSX Engine)
	// FDC: wd2793, 1 3.5" DSDD drive
	// 2 Cartridge slots
	// S-1985 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_disk(config, MSX_SLOT_DISK1, "disk", 3, 0, 0, 2, "extrom").set_tags("fdc", "fdc:0", "fdc:1");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 3, 0, 4).set_total_size(0x40000).set_ramio_bits(0x80);   // 256KB Mapper RAM

	MSX_S1985(config, "s1985", 0);

	msx_wd2793(config);
	msx_1_35_dd_drive(config);
	msx2_pal(YM2149, config);
}

/* MSX2 - Sony HB-F700F */

ROM_START(hbf700f)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("700fbios.ic5", 0x0000, 0x8000, CRC(440dae3c) SHA1(fedd9b682d056ddd1e9b3d281723e12f859b2e69))

	ROM_REGION(0x8000, "extrom", 0)
	// dumps as listed in openMSX and blueMSX
	//	ROM_LOAD("700fsub.ic6", 0x0000, 0x4000, CRC(e235d5c8) SHA1(792e6b2814ab783d06c7576c1e3ccd6a9bbac34a))
	//	ROM_LOAD("700fdisk.ic6", 0x4000, 0x4000, CRC(54c73ad6) SHA1(12f2cc79b3d09723840bae774be48c0d721ec1c6))
	//
	// however according to the service manual these should be in the same rom chip
	// concatenation of 792e6b2814ab783d06c7576c1e3ccd6a9bbac34a and 12f2cc79b3d09723840bae774be48c0d721ec1c6
	ROM_LOAD("700fext.ic6",  0x0000, 0x8000, BAD_DUMP CRC(463db23b) SHA1(2ab5be13b356692e75a5d76a23f8e4cfc094b3df)) // to be verified with direct dump
ROM_END

void msx2_state::hbf700f(machine_config &config)
{
	// YM2149 (in S1985)
	// FDC: wd2793, 1 3.5" DSDD drive
	// 2 Cartridge slots
	// S1985

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_disk(config, MSX_SLOT_DISK1, "disk", 3, 0, 0, 2, "extrom").set_tags("fdc", "fdc:0", "fdc:1");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 3, 0, 4).set_total_size(0x40000).set_ramio_bits(0x80);   // 256KB Mapper RAM

	MSX_S1985(config, "s1985", 0);

	msx_wd2793(config);
	msx_1_35_dd_drive(config);
	msx2_pal(YM2149, config);
}

/* MSX2 - Sony HB-F700P */

ROM_START(hbf700p)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("700pbios.rom.ic5", 0x0000, 0x8000, CRC(b31c851d) SHA1(0de3c802057560560a03d7965fcc4cff69f8575c))

	ROM_REGION(0x8000, "extrom", 0)
	// dumps as listed in openMSX / blueMSX
	// openMSX also lists 24624c5fa3a8069b1d865cdea8a029f15c1955ea for the subrom but the disk rom
	// part of that machine is 'certainly not original' so this may also not be original.
	//	ROM_LOAD("700psub.ic6", 0x0000, 0x4000, CRC(8f84f783) SHA1(3288894e1be6af705871499b23c85732dbc40993))
	//	ROM_LOAD("700pdisk.ic6", 0x4000, 0x4000, CRC(1d9cc7f6) SHA1(3376cf9dd2b1ac9b41bf6bf6598b33136e86f9d5))
	//
	// however according to the service manual these should be in the same rom chip
	// concatenation of 3288894e1be6af705871499b23c85732dbc40993 and 3376cf9dd2b1ac9b41bf6bf6598b33136e86f9d5
	ROM_LOAD("700pext.ic6", 0x0000, 0x8000, BAD_DUMP CRC(63e1bffc) SHA1(496698a60432490dc1306c8cc1d4a6ded275261a)) // to be verified with direct dump
ROM_END

void msx2_state::hbf700p(machine_config &config)
{
	// YM2149 (in S-1985 MSX Engine)
	// FDC: wd2793, 1 3.5" DSDD drive
	// 2 Cartridge slots
	// S-1985 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_disk(config, MSX_SLOT_DISK1, "disk", 3, 0, 0, 2, "extrom").set_tags("fdc", "fdc:0", "fdc:1");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 3, 0, 4).set_total_size(0x40000).set_ramio_bits(0x80);   // 256KB Mapper RAM

	MSX_S1985(config, "s1985", 0);

	msx_wd2793_force_ready(config);
	msx_1_35_dd_drive(config);
	msx2_pal(YM2149, config);
}

/* MSX2 - Sony HB-F700S */

ROM_START(hbf700s)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("700sbios.rom.ic5", 0x0000, 0x8000, CRC(c2b889a5) SHA1(4811956f878c3e03da46317f787cdc4bebc86f47))

	ROM_REGION(0x8000, "extrom", 0)
	// dumps as listed in openMSX / blueMSX
	//	ROM_LOAD("700ssub.ic6", 0x0000, 0x4000, CRC(dc0951bd) SHA1(1e9a955943aeea9b1807ddf1250ba6436d8dd276))
	//	ROM_LOAD("700sdisk.ic6", 0x4000, 0x4000, CRC(1d9cc7f6) SHA1(3376cf9dd2b1ac9b41bf6bf6598b33136e86f9d5))
	//
	// however according to the service manual these should be in the same rom chip
	// concatenation of 1e9a955943aeea9b1807ddf1250ba6436d8dd276 and 3376cf9dd2b1ac9b41bf6bf6598b33136e86f9d5
	ROM_LOAD("700sext.ic6", 0x0000, 0x8000, BAD_DUMP CRC(28d1badf) SHA1(ae3ed88a2d7034178e08f7bdf5409f462bf67fc9)) // to be verified with direct dump
ROM_END

void msx2_state::hbf700s(machine_config &config)
{
	// YM2149 (in S1985)
	// FDC: wd2793, 1 3.5" DSDD drive
	// 2 Cartridge slots
	// S1985

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_disk(config, MSX_SLOT_DISK1, "disk", 3, 0, 0, 2, "extrom").set_tags("fdc", "fdc:0", "fdc:1");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 3, 0, 4).set_total_size(0x40000).set_ramio_bits(0x80);   // 256KB Mapper RAM

	MSX_S1985(config, "s1985", 0);

	msx_wd2793_force_ready(config);
	msx_1_35_dd_drive(config);
	msx2_pal(YM2149, config);
}

/* MSX2 - Sony HB-F750 (prototype) */

/* MSX2 - Sony HB-F900 */

ROM_START(hbf900)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("f900bios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("f900ext.rom", 0x0000, 0x4000, CRC(43e7a7fc) SHA1(0fbd45ef3dd7bb82d4c31f1947884f411f1ca344))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("f900disk.rom", 0x0000, 0x4000, CRC(f83d0ea6) SHA1(fc760d1d7b16370abc7eea39955f230b95b37df6))

	ROM_REGION(0x4000, "firmware", 0)
	ROM_LOAD("f900util.rom", 0x0000, 0x4000, CRC(bc6c7c66) SHA1(558b7383544542cf7333700ff90c3efbf93ba2a3))

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("f900kfn.rom", 0, 0x20000, CRC(5a59926e) SHA1(6acaf2eeb57f65f7408235d5e07b7563229de799))
ROM_END

void msx2_state::hbf900(machine_config &config)
{
	// YM2149 (in S1985)
	// FDC: wd2793, 2 3.5" DSDD drives
	// 2 Cartridge slots
	// S1985

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 1, 0, 4).set_total_size(0x40000).set_ramio_bits(0x80);   // 256KB Mapper RAM
	add_internal_disk(config, MSX_SLOT_DISK1, "disk", 3, 2, 1, 1, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 3, 3, 1, 1, "firmware");

	MSX_S1985(config, "s1985", 0);

	msx_wd2793_force_ready(config);
	msx_2_35_dd_drive(config);
	msx2(YM2149, config);
}

/* MSX2 - Sony HB-F900 (a) */

ROM_START(hbf900a)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("f900bios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("f900ext.rom", 0x0000, 0x4000, CRC(43e7a7fc) SHA1(0fbd45ef3dd7bb82d4c31f1947884f411f1ca344))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("f900disa.rom", 0x0000, 0x4000, CRC(54c73ad6) SHA1(12f2cc79b3d09723840bae774be48c0d721ec1c6))

	ROM_REGION(0x4000, "firmware", 0)
	ROM_LOAD("f900util.rom", 0x0000, 0x4000, CRC(bc6c7c66) SHA1(558b7383544542cf7333700ff90c3efbf93ba2a3))

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("f900kfn.rom", 0, 0x20000, CRC(5a59926e) SHA1(6acaf2eeb57f65f7408235d5e07b7563229de799))
ROM_END

/* MSX2 - Sony HB-F9P */

ROM_START(hbf9p)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("f9pbios.rom.ic11", 0x0000, 0x8000, CRC(b31c851d) SHA1(0de3c802057560560a03d7965fcc4cff69f8575c))

	ROM_REGION(0x8000, "subfirm", 0)
	// dumps as listed in openMSX / blueMSX
	// ROM_LOAD("f9psub.rom", 0x0000, 0x4000, CRC(7c456c8b) SHA1(7b4a96402847decfc110ff9eda713bdcd218bd83))
	// ROM_LOAD("f9pfirm2.rom", 0x0000, 0x4000, CRC(dea2cb50) SHA1(8cc1f7ceeef745bb34e80253971e137213671486))
	// concatenation of 7b4a96402847decfc110ff9eda713bdcd218bd83 and 8cc1f7ceeef745bb34e80253971e137213671486
	ROM_LOAD("f9pfirm1.ic12", 0x0000, 0x8000, BAD_DUMP CRC(524f67aa) SHA1(41a186afced50ca6312cb5b6c4adb684faca6232))

	ROM_REGION(0x8000, "firmware", 0)
	// like in HB-F9S, the halves should be swapped?
	ROM_LOAD("f9pfirm2.rom.ic13", 0x0000, 0x8000, BAD_DUMP CRC(ea97069f) SHA1(2d1880d1f5a6944fcb1b198b997a3d90ecd1903d))
ROM_END

void msx2_state::hbf9p(machine_config &config)
{
	// YM2149 (in S-1985 MSX Engine)
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// S-1985 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subfirm", 3, 0, 0, 2, "subfirm");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 3, 1, 1, 2, "firmware");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000).set_ramio_bits(0x80);   // 128KB Mapper RAM

	MSX_S1985(config, "s1985", 0);
	msx2_pal(YM2149, config);
}

/* MSX2 - Sony HB-F9P Russian */

ROM_START(hbf9pr)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("f9prbios.rom", 0x0000, 0x8000, CRC(f465311b) SHA1(7f440ec7295d889b097e1b66bf9bc5ce086f59aa))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("f9prext.rom", 0x0000, 0x4000, CRC(d701adac) SHA1(a6d7b1fd4ee896ca7513d02c033fc9a8aa065235))
ROM_END

void msx2_state::hbf9pr(machine_config &config)
{
	// YM2149 (in S-1985 MSX Engine)
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// S-1985 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "ext", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "ext_mirror", 3, 0, 1, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000);   // 128KB Mapper RAM

	MSX_S1985(config, "s1985", 0);
	msx2_pal(YM2149, config);
}

/* MSX2 - Sony HB-F9S */

ROM_START(hbf9s)
	ROM_REGION(0x18000, "mainrom", 0)
	ROM_LOAD("f9sbios.ic11", 0x0000, 0x8000, CRC(c2b889a5) SHA1(4811956f878c3e03da46317f787cdc4bebc86f47))

	ROM_REGION(0x8000, "subfirm", 0)
	ROM_LOAD("f9sfirm1.ic12", 0x0000, 0x8000, CRC(cf39620b) SHA1(1166a93d7185ba024bdf2bfa9a30e1c447fb6db1))

	ROM_REGION(0x8000, "firmware", 0)
	ROM_LOAD("f9sfirm2.ic13", 0x0000, 0x8000, CRC(4a271395) SHA1(7efac54dd8f580f3b7809ab35db4ae58f0eb84d1))
ROM_END

void msx2_state::hbf9s(machine_config &config)
{
	// YM2149 (in S-1985 MSX Engine)
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// S-1985 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subfirm", 3, 0, 0, 2, "subfirm");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware_lo", 3, 1, 1, 1, "firmware", 0x4000);
	add_internal_slot(config, MSX_SLOT_ROM, "firmware_hi", 3, 1, 2, 1, "firmware");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000).set_ramio_bits(0x80);   // 128KB Mapper RAM

	MSX_S1985(config, "s1985", 0);
	msx2_pal(YM2149, config);
}

/* MSX2 - Sony HB-G900AP */

/* IC109 - 32KB Basic ROM SLOT#00 0000-7FFF */
/* IC112 - 16KB Basic ROM SLOT#01 0000-3FFF */
/* IC117 - 16KB Disk ROM SLOT#01 4000-7FFF */
/* IC123 - 32KB ROM RS232C ROM SLOT#02 4000-7FFF / Video Utility ROM SLOT#03 4000-7FFF */

/* MSX2 - Sony HB-G900AP */
ROM_START(hbg900ap)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("g900bios.ic109",  0x0000, 0x8000, CRC(b31c851d) SHA1(0de3c802057560560a03d7965fcc4cff69f8575c))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("g900ext.ic112", 0x0000, 0x4000, CRC(8f84f783) SHA1(3288894e1be6af705871499b23c85732dbc40993))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("g900disk.ic117", 0x0000, 0x4000, CRC(54c73ad6) SHA1(12f2cc79b3d09723840bae774be48c0d721ec1c6))

	ROM_REGION(0x4000, "rs232", 0)
	// Contents very likely to be ok, but should be inside a single rom together with firmware
	ROM_LOAD("g900232c.rom", 0x0000, 0x2000, BAD_DUMP CRC(be88e5f7) SHA1(b2776159a7b92d74308b434a6b3e5feba161e2b7))

	ROM_REGION(0x4000, "firmware", 0)
	// Contents very likely to be ok, but should be inside a single rom together with rs232 code
	ROM_LOAD("g900util.rom", 0x0000, 0x4000, BAD_DUMP CRC(ecf6abcf) SHA1(6bb18cd2d69f124ad0c7c23a13eb0d2139037696))
ROM_END

void msx2_state::hbg900ap(machine_config &config)
{
	// YM2149 (in S1985)
	// FDC: wd2793, 1 3.5" DSDD drive
	// 2 Cartridge slots
	// S1985
	// rs232 switch for terminal / modem operation
	// rs232 2kb ram

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 1, 0, 1, "subrom");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1, "disk", 0, 1, 1, 2, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");
	add_internal_slot_irq_mirrored<3>(config, MSX_SLOT_RS232_SONY, "rs232", 0, 2, 1, 2, "rs232");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 0, 3, 1, 1, "firmware");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	// slot #3 is expanded
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 0, 0, 4).set_total_size(0x80000).set_ramio_bits(0x80);   // 512KB Mapper RAM

	msx_wd2793(config);
	msx_1_35_dd_drive(config);
	msx2_pal(YM2149, config);
}

/* MSX2 - Sony HB-G900D */

/* MSX2 - Sony HB-G900F */

/* MSX2 - Sony HB-G900P - 3x 32KB ROMs */

ROM_START(hbg900p)
	ROM_REGION(0x18000, "mainrom", 0)
	ROM_LOAD("g900bios.rom", 0x0000, 0x8000, CRC(b31c851d) SHA1(0de3c802057560560a03d7965fcc4cff69f8575c))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("g900ext.rom", 0x0000, 0x4000, CRC(8f84f783) SHA1(3288894e1be6af705871499b23c85732dbc40993))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("g900disk.rom", 0x0000, 0x4000, CRC(54c73ad6) SHA1(12f2cc79b3d09723840bae774be48c0d721ec1c6))

	ROM_REGION(0x4000, "rs232", 0)
	ROM_LOAD("g900232c.rom", 0x0000, 0x2000, CRC(be88e5f7) SHA1(b2776159a7b92d74308b434a6b3e5feba161e2b7))

	ROM_REGION(0x4000, "firmware", 0)
	ROM_LOAD("g900util.rom", 0x0000, 0x4000, CRC(d0417c20) SHA1(8779b004e7605a3c419825f0373a5d8fa84e1d5b))
ROM_END

void msx2_state::hbg900p(machine_config &config)
{
	// AY8910
	// FDC: wd2793, 1 3.5" DSDD drive
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 1, 0, 1, "subrom");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1, "diskrom", 0, 1, 1, 2, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");
	add_internal_slot_irq_mirrored<3>(config, MSX_SLOT_RS232_SONY, "rs232", 0, 2, 1, 2, "rs232");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 0, 3, 1, 1, "firmware");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 4);   // 64KB RAM

	msx_wd2793(config);
	msx_1_35_dd_drive(config);
	msx2_pal(AY8910, config);
}

/* MSX2 - Sony HB-T600 */

/* MSX2 - Sony HB-T7 */

/* MSX2 - Talent DPC-300 */

/* MSX2 - Talent TPC-310 */

ROM_START(tpc310)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("tpc310bios.rom", 0x0000, 0x8000, CRC(8cd3e845) SHA1(7bba23669b7abfb6a142f9e1735b847d6e4e8267))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("tpc310ext.rom", 0x0000, 0x4000, CRC(094a9e7a) SHA1(39dfc46260f99b670916b1e55f67a5d4136e6e54))

	ROM_REGION(0x4000, "turbo", 0)
	ROM_LOAD("tpc310turbo.rom", 0x0000, 0x4000, CRC(0ea62a4d) SHA1(181bf58da7184e128cd419da3109b93344a543cf))

	ROM_REGION(0x8000, "firmware", 0)
	ROM_LOAD("tpc310acc.rom", 0x0000, 0x8000, CRC(4fb8fab3) SHA1(cdeb0ed8adecaaadb78d5a5364fd603238591685))
ROM_END

void msx2_state::tpc310(machine_config &config)
{
	// YM2149 (in S-1985 MSX Engine)
	// FDC: None, 0 drives
	// 1 Cartridge slot (slot 2)
	// S-1985 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 1, 0, 4).set_total_size(0x20000).set_ramio_bits(0x80);   // 128KB Mapper RAM
	add_cartridge_slot<1>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "turbo", 3, 0, 1, 1, "turbo");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 3, 1, 1, 2, "firmware");
	// Expansion slot in slot #3-2

	MSX_S1985(config, "s1985", 0);

	msx_mb8877a(config);
	msx_1_35_dd_drive(config);
	msx2_pal(YM2149, config);
}

/* MSX2 - Talent TPP-311 */

ROM_START(tpp311)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("311bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(8cd3e845) SHA1(7bba23669b7abfb6a142f9e1735b847d6e4e8267)) // need verification

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("311ext.rom", 0x0000, 0x4000, BAD_DUMP CRC(094a9e7a) SHA1(39dfc46260f99b670916b1e55f67a5d4136e6e54)) // need verification

	ROM_REGION(0x8000, "logo", 0)
	ROM_LOAD("311logo.rom", 0x0000, 0x8000, BAD_DUMP CRC(0e6ecb9f) SHA1(e45ddc5bf1a1e63756d11fb43fc50276ca35cab0)) // need verification
ROM_END

void msx2_state::tpp311(machine_config &config)
{
	// YM2149 (in S1985)
	// FDC: None, 0 drives
	// 0 Cartridge slots?
	// S1985
	// 64KB VRAM

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 1, 0, 4).set_total_size(0x10000);   // 64KB?? Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "logo", 2, 1, 2, "logo");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 1, "subrom");

	MSX_S1985(config, "s1985", 0);

	msx2_pal(YM2149, config);
	msx2_64kb_vram(config);
}

/* MSX2 - Talent TPS-312 */

ROM_START(tps312)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("312bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(8cd3e845) SHA1(7bba23669b7abfb6a142f9e1735b847d6e4e8267)) // need verification

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("312ext.rom", 0x0000, 0x4000, BAD_DUMP CRC(094a9e7a) SHA1(39dfc46260f99b670916b1e55f67a5d4136e6e54)) // need verification

	ROM_REGION(0x8000, "plan", 0)
	ROM_LOAD("312plan.rom", 0x0000, 0x8000, BAD_DUMP CRC(b3a6aaf6) SHA1(6de80e863cdd7856ab7aac4c238224a5352bda3b)) // need verification

	ROM_REGION(0x4000, "write", 0)
	ROM_LOAD("312write.rom", 0x0000, 0x4000, BAD_DUMP CRC(63c6992f) SHA1(93682f5baba7697c40088e26f99ee065c78e83b8)) // need verification
ROM_END

void msx2_state::tps312(machine_config &config)
{
	// YM2149 (in S1985)
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// 64KB VRAM
	// S1985

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 1, 0, 4).set_total_size(0x10000);   // 64KB Mapper RAM
	add_cartridge_slot<1>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "write", 3, 0, 1, 1, "write");
	add_internal_slot(config, MSX_SLOT_ROM, "plan", 3, 1, 0, 2, "plan");
	add_internal_slot(config, MSX_SLOT_ROM, "plan_mirror", 3, 1, 2, 2, "plan");
	// Expansion slot in slot #3-2

	MSX_S1985(config, "s1985", 0);

	msx2_pal(YM2149, config);
	msx2_64kb_vram(config);
}

/* MSX2 - Toshiba FS-TM1 */

ROM_START(fstm1)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("fstm1bios.rom", 0x0000, 0x8000, CRC(d1e11d52) SHA1(7a69e9b9595f3b0060155f4b419c915d4d9d8ca1))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("fstm1ext.rom", 0x0000, 0x4000, CRC(4eebe9b1) SHA1(a4bdbdb20bf9fd3c492a890fbf541bf092eaa8e1))

	ROM_REGION(0x8000, "deskpac1", 0)
	ROM_LOAD("fstm1desk1.rom", 0x0000, 0x8000, CRC(8b802086) SHA1(30737040d90c136d34dd409fe579bc4cca11c469))

	ROM_REGION(0x8000, "deskpac2", 0)
	ROM_LOAD("fstm1desk2.rom", 0x0000, 0x8000, CRC(304820ea) SHA1(ff6e07d3976b0874164fae680ae028d598752049))
ROM_END

void msx2_state::fstm1(machine_config &config)
{
	// YM2149 (in S-1985)
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// S-1985 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 0, 0, 4).set_total_size(0x10000); // 64KB Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 1, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "deskpac1", 3, 1, 1, 2, "deskpac1");
	add_internal_slot(config, MSX_SLOT_ROM, "deskpac2", 3, 3, 1, 2, "deskpac2");

	MSX_S1985(config, "s1985", 0);
	msx2_pal(YM2149, config);
}

/* MSX2 - Toshiba HX-23 */

ROM_START(hx23)
	// roms from hx23f, assumed to be the same for hx23 but need verification
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hx23bios.ic2", 0x0000, 0x8000, BAD_DUMP CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd)) // need verification

	ROM_REGION(0x8000, "subjwp", 0)
	ROM_LOAD("hx23subjwp.ic52", 0x0000, 0x8000, BAD_DUMP CRC(478016bf) SHA1(6ecf73a1dd55b363c2e68cc6245ece979aec1fc5)) // need verification

	ROM_REGION(0x8000, "rs232jwp", 0)
	ROM_LOAD("hx23rs232jwp.ic3", 0x0000, 0x8000, BAD_DUMP CRC(60160d3b) SHA1(0958361ac9b19782cf7017b2e762b416e0203f37)) // need verification
ROM_END

void msx2_state::hx23(machine_config &config)
{
	// AY8910
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// 64KB VRAM
	// HX-R701 RS-232 optional
	// TCX-1012

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram1", 0, 2, 2);   // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram2", 3, 0, 0, 2);   // 32KB RAM
	add_internal_slot(config, MSX_SLOT_ROM, "sub", 3, 1, 0, 1, "subjwp");
	add_internal_slot(config, MSX_SLOT_ROM, "jwp", 3, 1, 2, 1, "subjwp", 0x4000);
	add_internal_slot(config, MSX_SLOT_ROM, "rs232jwp", 3, 3, 1, 2, "rs232jwp");

	msx2_pal(AY8910, config);
	msx2_64kb_vram(config);
}

/* MSX2 - Toshiba HX-23F */

ROM_START(hx23f)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hx23bios.ic2", 0x0000, 0x8000, BAD_DUMP CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd)) // need verification

	ROM_REGION(0x8000, "subjwp", 0)
	ROM_LOAD("hx23subjwp.ic52", 0x0000, 0x8000, BAD_DUMP CRC(478016bf) SHA1(6ecf73a1dd55b363c2e68cc6245ece979aec1fc5)) // need verification

	ROM_REGION(0x8000, "rs232jwp", 0)
	ROM_LOAD("hx23rs232jwp.ic3", 0x0000, 0x8000, BAD_DUMP CRC(60160d3b) SHA1(0958361ac9b19782cf7017b2e762b416e0203f37)) // need verification
ROM_END

void msx2_state::hx23f(machine_config &config)
{
	// AY8910
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// HX-R701 RS-232 optional
	// TCX-1012

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram1", 0, 2, 2);   // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram2", 3, 0, 0, 2);   // 32KB RAM
	add_internal_slot(config, MSX_SLOT_ROM, "sub", 3, 1, 0, 1, "subjwp");
	add_internal_slot(config, MSX_SLOT_ROM, "jwp", 3, 1, 2, 1, "subjwp", 0x4000);
	add_internal_slot(config, MSX_SLOT_ROM, "rs232jwp", 3, 3, 1, 2, "rs232jwp");

	msx2_pal(AY8910, config);
}

/* MSX2 - Toshiba HX-33 */

ROM_START(hx33)
	ROM_REGION(0x20000, "mainrom", 0)
	ROM_LOAD("hx33bios.ic7", 0x0000, 0x20000, CRC(8dd5502b) SHA1(5e057526fe39d79e88e7ff1ce02ed669bd38929e))

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("hx33kfn.rom", 0x0000, 0x20000, CRC(d23d4d2d) SHA1(db03211b7db46899df41db2b1dfbec972109a967))
ROM_END

void msx2_state::hx33(machine_config &config)
{
	// YM2149
	// FDC: None, 0, drives
	// 2 Cartridge slots
	// TCX-2001 + TCX-2002
	// HX-R702 RS-232 optional
	// 2KB SRAM
	// copy button

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 0, 0, 4).set_total_size(0x10000); // 64KB Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 1, 0, 1, "mainrom", 0x8000);
	add_internal_slot(config, MSX_SLOT_RS232_TOSHIBA_HX3X, "firmware", 3, 3, 1, 2, "mainrom", 0xc000);

	msx2(YM2149, config);
	msx2_64kb_vram(config);
}

/* MSX@ - Toshiba HX-34 */

ROM_START(hx34)
	ROM_REGION(0x20000, "mainrom", 0)
	ROM_LOAD("hx33bios.ic7", 0x0000, 0x20000, CRC(8dd5502b) SHA1(5e057526fe39d79e88e7ff1ce02ed669bd38929e))

	ROM_REGION(0x4000, "diskrom", 0)
	// hx34disk.rom has contents of floppy registers at offset 3ff0-3ff7 and mirrored at 3ff8-3fff
	ROM_LOAD("hx34disk.rom", 0x0000, 0x4000, BAD_DUMP CRC(626b719d) SHA1(c88ef953b21370cbaef5e82575d093d6f9047ec6))

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("hx34kfn.rom", 0x0000, 0x20000, CRC(d23d4d2d) SHA1(db03211b7db46899df41db2b1dfbec972109a967))
ROM_END

void msx2_state::hx34(machine_config &config)
{
	// YM2149
	// FDC: wd2793??, 1 3.5" DSDD drive
	// 2 Cartridge slots
	// TCX-2001 + TCX-2002
	// HX-R703 RS232 optional
	// 2KB SRAM
	// copy button

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 0, 0, 4).set_total_size(0x10000); // 64KB Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 1, 0, 1, "mainrom", 0x8000);
	add_internal_disk(config, MSX_SLOT_DISK6, "disk", 3, 2, 1, 1, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");
	add_internal_slot(config, MSX_SLOT_RS232_TOSHIBA_HX3X, "firmware", 3, 3, 1, 2, "mainrom", 0xc000);

	msx_wd2793(config);
	msx_1_35_dd_drive(config);
	msx2(YM2149, config);
}

/* MSX2 - Victor HC-80 */

ROM_START(victhc80)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hc80bios.rom",  0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("hc80ext.rom", 0x0000, 0x4000, CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33))

	ROM_REGION(0x4000, "firmware", 0)
	ROM_LOAD("hc80firm.rom", 0x0000, 0x4000, CRC(30e8c08d) SHA1(7f498db2f431b9c0b42dac1c7ca46a236b780228))
ROM_END

void msx2_state::victhc80(machine_config &config)
{
	// YM2149 (in S3527)
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// S3527 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram1", 0, 0, 2, 2); // 32KB RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 1, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram2", 0, 2, 0, 2); // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 3, 2, 1, "firmware");

	msx2(YM2149, config);
}

/* MSX2 - Victor HC-90 */

ROM_START(victhc90)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hc90bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd)) // need verification

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("hc90ext.rom", 0x0000, 0x4000, BAD_DUMP CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33)) // need verification

	ROM_REGION(0x4000, "diskrom", 0)
	// FDC register contents at 3ff8-3fff
	ROM_LOAD("hc90disk.rom", 0x0000, 0x4000, BAD_DUMP CRC(11bca2ed) SHA1(a7a34671bddb48fa6c74182e2977f9129558ec32)) // need verification

	ROM_REGION(0x4000, "firmware", 0)
	ROM_LOAD("hc90firm.rom", 0x0000, 0x4000, BAD_DUMP CRC(53791d91) SHA1(caeffdd654394726c8c0824b21af7ff51c0b1031)) // need verification

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("hc90kfn.rom", 0x0000, 0x20000, BAD_DUMP CRC(d23d4d2d) SHA1(db03211b7db46899df41db2b1dfbec972109a967)) // need verification
ROM_END

void msx2_state::victhc90(machine_config &config)
{
	// YM2149
	// FDC: mb8877a?, 1 3.5" DSDD drive
	// RS232C builtin
	// 2nd CPU HD-64B180 @ 6.144 MHz
	// 1 Cartridge slot (slot 1 or 2?)

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 1, 0, 1, "subrom");
	add_internal_slot_irq<2>(config, MSX_SLOT_RS232, "firmware", 0, 1, 1, 1, "firmware");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 0, 2, 0, 4).set_total_size(0x10000); // 64KB Mapper RAM
	add_cartridge_slot<1>(config, 1);
	add_internal_disk(config, MSX_SLOT_DISK10, "disk", 3, 1, 1, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");

	msx_mb8877a(config);
	msx_1_35_dd_drive(config);
	msx2(YM2149, config);
}

/* MSX2 - Victor HC-90(A) */

/* MSX2 - Victor HC-90(B) */

/* MSX2 - Victor HC-90(V) */

/* MSX2 - Victor HC-90(T) */

/* MSX2 - Victor HC-95 */

ROM_START(victhc95)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hc95bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd)) // need verification

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("hc95ext.rom", 0x0000, 0x4000, BAD_DUMP CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33)) // need verification

	ROM_REGION(0x4000, "diskrom", 0)
	// FDC register contents at 3ff8-3fff
	ROM_LOAD("hc95disk.rom", 0x0000, 0x4000, BAD_DUMP CRC(11bca2ed) SHA1(a7a34671bddb48fa6c74182e2977f9129558ec32)) // need verification

	ROM_REGION(0x4000, "firmware", 0)
	ROM_LOAD("hc95firm.rom", 0x0000, 0x4000, BAD_DUMP CRC(53791d91) SHA1(caeffdd654394726c8c0824b21af7ff51c0b1031)) // need verification

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("hc95kfn.rom", 0x0000, 0x20000, BAD_DUMP CRC(d23d4d2d) SHA1(db03211b7db46899df41db2b1dfbec972109a967)) // need verification
ROM_END

void msx2_state::victhc95(machine_config &config)
{
	// YM2149
	// FDC: mb8877a, 2 3.5" DSDD drive
	// RS232C builtin
	// 2nd CPU HD-64B180 @ 6.144 MHz
	// 1 Cartridge slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 1, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 0, 1, 1, 1, "firmware");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 0, 2, 0, 4).set_total_size(0x10000); // 64KB Mapper RAM
	// 96 pin expansion bus in slot #0-3
	add_cartridge_slot<1>(config, 1);
	// 96 pin expansion bus in slot #2
	add_internal_disk(config, MSX_SLOT_DISK10, "disk", 3, 1, 1, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");

	msx_mb8877a(config);
	msx_2_35_dd_drive(config);
	msx2(YM2149, config);
}

/* MSX2 - Victor HC-95A */

ROM_START(victhc95a)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hc95abios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("hc95aext.rom", 0x0000, 0x4000, CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33))

	ROM_REGION(0x4000, "diskrom", 0)
	// FDC register contents at 3ff8-3fff
	ROM_LOAD("hc95adisk.rom", 0x0000, 0x4000, BAD_DUMP CRC(11bca2ed) SHA1(a7a34671bddb48fa6c74182e2977f9129558ec32))

	ROM_REGION(0x4000, "firmware", 0)
	ROM_LOAD("hc95afirm.rom", 0x0000, 0x4000, CRC(53791d91) SHA1(caeffdd654394726c8c0824b21af7ff51c0b1031))

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("hc95akfn.rom", 0x0000, 0x20000, CRC(d23d4d2d) SHA1(db03211b7db46899df41db2b1dfbec972109a967))
ROM_END

void msx2_state::victhc95a(machine_config &config)
{
	// YM2149
	// FDC: mb8877a, 2 3.5" DSDD drive
	// RS232C builtin
	// 2nd CPU HD-64B180 @ 6.144 MHz
	// 1 Cartridge slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 1, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 0, 1, 1, 1, "firmware");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 0, 2, 0, 4).set_total_size(0x10000); // 64KB Mapper RAM
	// 96 pin expansion bus in slot #0-3
	add_cartridge_slot<1>(config, 1);
	// 96 pin expansion bus in slot #2
	add_internal_disk(config, MSX_SLOT_DISK10, "disk", 3, 1, 1, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");

	msx_mb8877a(config);
	msx_2_35_dd_drive(config);
	msx2(YM2149, config);
}

/* MSX2 - Wandy CPC-300 */

/* MSX2 - Yamaha CX7/128 */

ROM_START(cx7128)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("cx7mbios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("cx7mext.rom", 0x0000, 0x4000, CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33))
ROM_END

void msx2_state::cx7128(machine_config &config)
{
	// YM2149 (in S3527)
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// S3527

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	// mini cartridge port in slot #3-1
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000).set_ramio_bits(0x80);   // 64KB Mapper RAM
	add_cartridge_slot<3>(config, MSX_SLOT_YAMAHA_EXPANSION, "expansion", 3, 3, msx_yamaha_60pin, nullptr);

	msx2(YM2149, config);
}

/* MSX2 - Yamaha CX7M/128 */

ROM_START(cx7m128)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("cx7mbios.rom", 0x0000, 0x8000, BAD_DUMP CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd)) // needs verification

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("cx7mext.rom", 0x0000, 0x4000, BAD_DUMP CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33)) // needs verification

	ROM_REGION(0x4000, "minicart", 0)
	ROM_LOAD("yrm502.rom", 0x0000, 0x4000, CRC(51f7ddd1) SHA1(2a4b4a4657e3077df8a88f98210b76883d3702b1))
ROM_END

void msx2_state::cx7m128(machine_config &config)
{
	// YM2149 (in S3527)
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// S3527

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	// mini cartridge port in slot #3-1
	add_internal_slot(config, MSX_SLOT_ROM, "minicart", 3, 1, 1, 1, "minicart");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000).set_ramio_bits(0x80);   // 64KB Mapper RAM
	add_cartridge_slot<3>(config, MSX_SLOT_YAMAHA_EXPANSION, "expansion", 3, 3, msx_yamaha_60pin, "sfg05");

	msx2(YM2149, config);
}

/* MSX2 - Yamaha YIS-503 III R (student) */

ROM_START(y503iiir)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("yis503iiirbios.rom", 0x0000, 0x8000, CRC(e7d08e29) SHA1(0f851ee7a1cf79819f61cc89e9948ee72a413802))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("yis503iiirext.rom", 0x0000, 0x4000, CRC(34d21778) SHA1(03bf6d2ac86f5c9ab618e155442787c700f99fed))

	ROM_REGION(0x4000, "cpm", 0)
	ROM_LOAD("yis503iiircpm.rom", 0x0000, 0x4000, CRC(417bf00e) SHA1(f4f7a54cdf5a9dd6c59f7cb219c2c5eb0a00fa8a))

	ROM_REGION(0x8000, "network", 0)
	// TODO: Which one is correct??
	// has 2 * 2KB RAM?
	ROM_LOAD("yis503iiirnet.rom", 0x0000, 0x8000, CRC(0e345b43) SHA1(e8fd2bbc1bdab12c73a0fec178a190f9063547bb))
	ROM_LOAD("yis503iiirnet.rom", 0x0000, 0x8000, CRC(75331cac) SHA1(307a7be064442feb4ab2e1a2bc971b138c1a1169))
ROM_END

void msx2_state::y503iiir(machine_config &config)
{
	// YM2149 (in S-3527)
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// Networking builtin
	// S-3527 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "cpm", 3, 0, 1, 1, "cpm");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000); // 128KB Mapper RAM
	// Yamaha expansion slot in slot #3-3
	add_internal_slot(config, MSX_SLOT_ROM, "network", 3, 3, 1, 2, "network");

	msx2_pal(YM2149, config);
}

/* MSX2 - Yamaha YIS-503 III R Estonian */

ROM_START(y503iiire)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("yis503iiirebios.rom", 0x0000, 0x8000, CRC(d0c20f54) SHA1(ebb7eb540a390509edfd36c84288ba85e63f2d1f))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("yis503iiireext.rom", 0x0000, 0x4000, CRC(34d21778) SHA1(03bf6d2ac86f5c9ab618e155442787c700f99fed))

	ROM_REGION(0x4000, "cpm", 0)
	ROM_LOAD("yis503iiirecpm.rom", 0x0000, 0x4000, CRC(417bf00e) SHA1(f4f7a54cdf5a9dd6c59f7cb219c2c5eb0a00fa8a))

	ROM_REGION(0x8000, "network", 0)
	// TODO: Which one is correct??
	ROM_LOAD("yis503iiirnet.rom", 0x0000, 0x8000, CRC(0e345b43) SHA1(e8fd2bbc1bdab12c73a0fec178a190f9063547bb))
	ROM_LOAD("yis503iiirnet.rom", 0x0000, 0x8000, CRC(75331cac) SHA1(307a7be064442feb4ab2e1a2bc971b138c1a1169))
ROM_END

/* MSX2 - Yamaha YIS604/128 */

ROM_START(yis604)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("yis604bios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("yis604ext.rom", 0x0000, 0x4000, CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33))
ROM_END

void msx2_state::yis604(machine_config &config)
{
	// YM2149 (in S-3527)
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// S-3527 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 1, 0, 1, "subrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	// Mini cartridge slot in slot #3-1
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000); // 128KB Mapper RAM
	add_cartridge_slot<3>(config, MSX_SLOT_YAMAHA_EXPANSION, "expansion", 3, 3, msx_yamaha_60pin, nullptr);

	msx2(YM2149, config);
}

/* MSX2 - Yamaha YIS-805/128 */

ROM_START(y805128)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("yis805128bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd)) // need verification

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("yis805128ext.rom", 0x0000, 0x4000, BAD_DUMP CRC(43e7a7fc) SHA1(0fbd45ef3dd7bb82d4c31f1947884f411f1ca344)) // need verification

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("yis805128disk.rom", 0x0000, 0x4000, BAD_DUMP CRC(ab94a273) SHA1(4b08a057e5863ade179dcf8bc9377e90940e6d61)) // need verification

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("yis805128kfn.rom", 0x0000, 0x20000, BAD_DUMP CRC(5a59926e) SHA1(6acaf2eeb57f65f7408235d5e07b7563229de799)) // need verification
ROM_END

void msx2_state::y805128(machine_config &config)
{
	// YM2149 (in S1985)
	// FDC: wd2793?, 1 3.5" DSDD drive
	// 2 Cartridge slots
	// S1985 MSX Engine
	// 2KB SRAM
	// no mini cartridge slot?

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 1, 0, 1, "subrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_disk_mirrored(config, MSX_SLOT_DISK11, "disk", 3, 0, 1, 1, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000); // 128KB Mapper RAM
	// Default: SKW-05
	add_cartridge_slot<3>(config, MSX_SLOT_YAMAHA_EXPANSION, "expansion", 3, 3, msx_yamaha_60pin, nullptr);

	MSX_S1985(config, "s1985", 0);
	msx_wd2793_force_ready(config);
	msx_1_35_dd_drive(config);
	msx2(YM2149, config);
}

/* MSX2 - Yamaha YIS-805/256 */

ROM_START(y805256)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("yis805256bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd)) // need verification

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("yis805256ext.rom", 0x0000, 0x4000, BAD_DUMP CRC(43e7a7fc) SHA1(0fbd45ef3dd7bb82d4c31f1947884f411f1ca344)) // need verification

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("yis805256disk.rom", 0x0000, 0x4000, BAD_DUMP CRC(ab94a273) SHA1(4b08a057e5863ade179dcf8bc9377e90940e6d61)) // need verification

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("yis805256kfn.rom", 0x0000, 0x20000, BAD_DUMP CRC(5a59926e) SHA1(6acaf2eeb57f65f7408235d5e07b7563229de799)) // need verification
ROM_END

void msx2_state::y805256(machine_config &config)
{
	// YM2149 (in S1985)
	// FDC: wd2793?, 2 3.5" DSDD drives
	// 2 Cartridge slots
	// S1985 MSX Engine
	// 2KB SRAM
	// RS232C
	// no mini cartridge slot?

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 1, 0, 1, "subrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1, "disk", 3, 0, 1, 2, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x40000); // 256KB Mapper RAM
	// Default: SKW-05
	add_cartridge_slot<3>(config, MSX_SLOT_YAMAHA_EXPANSION, "expansion", 3, 3, msx_yamaha_60pin, nullptr);

	MSX_S1985(config, "s1985", 0);
	msx_wd2793_force_ready(config);
	msx_2_35_dd_drive(config);
	msx2(YM2149, config);
}

/* MSX2 - Yamaha YIS-805/128R2 (teacher) */

ROM_START(y805128r2)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("yis805128r2bios.rom", 0x0000, 0x8000, CRC(e7d08e29) SHA1(0f851ee7a1cf79819f61cc89e9948ee72a413802))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("yis805128r2ext.rom", 0x0000, 0x4000, CRC(34d21778) SHA1(03bf6d2ac86f5c9ab618e155442787c700f99fed))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("yis805128r2disk.rom", 0x0000, 0x4000, CRC(9eb7e24d) SHA1(3a481c7b7e4f0406a55952bc5b9f8cf9d699376c))

	ROM_REGION(0x8000, "network", 0)
	// has 2 * 2KB RAM ?
	ROM_LOAD("yis805128r2net.rom", 0x0000, 0x8000, CRC(0e345b43) SHA1(e8fd2bbc1bdab12c73a0fec178a190f9063547bb))

	ROM_REGION(0x10000, "firmware", 0)
	ROM_LOAD("yis805128r2paint.rom", 0x00000, 0x10000, CRC(1bda68a3) SHA1(7fd2a28c4fdaeb140f3c8c8fb90271b1472c97b9))
ROM_END

void msx2_state::y805128r2(machine_config &config)
{
	// YM2149 (in S1985)
	// FDC: wd2793?, 1 3.5" DSDD drive
	// 2 Cartridge slots
	// S1985 MSX Engine
	// Networking built in

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 3, 0, 0, 4, "firmware");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 1, 0, 1, "subrom");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1, "disk", 3, 1, 1, 2, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000); // 128KB Mapper RAM
	// This is actually the module slot
	add_internal_slot(config, MSX_SLOT_ROM, "network", 3, 3, 0, 2, "network", 0x00000);

	MSX_S1985(config, "s1985", 0);
	msx_wd2793_force_ready(config);
	msx_1_35_dd_drive(config);
	msx2_pal(YM2149, config);
}

/* MSX2 - Yamaha YIS-805/128R2 Estonian */

ROM_START(y805128r2e)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("yis805128r2ebios.rom", 0x0000, 0x8000, CRC(d0c20f54) SHA1(ebb7eb540a390509edfd36c84288ba85e63f2d1f))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("yis805128r2eext.rom", 0x0000, 0x4000, CRC(34d21778) SHA1(03bf6d2ac86f5c9ab618e155442787c700f99fed))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("yis805128r2edisk.rom", 0x0000, 0x4000, CRC(9eb7e24d) SHA1(3a481c7b7e4f0406a55952bc5b9f8cf9d699376c))

	ROM_REGION(0x8000, "network", 0)
	ROM_LOAD("yis805128r2enet.rom", 0x0000, 0x8000, CRC(0e345b43) SHA1(e8fd2bbc1bdab12c73a0fec178a190f9063547bb))

	ROM_REGION(0x10000, "firmware", 0)
	ROM_LOAD("yis805128r2epaint.rom", 0x00000, 0x10000, CRC(1bda68a3) SHA1(7fd2a28c4fdaeb140f3c8c8fb90271b1472c97b9))
ROM_END

/********************************  MSX 2+ **********************************/

/* MSX2+ - Ciel Expert 3 IDE */

ROM_START(expert3i)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("exp30bios.rom", 0x0000,  0x8000, CRC(a10bb1ce) SHA1(5029cf47031b22bd5d1f68ebfd3be6d6da56dfe9))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("exp30ext.rom", 0x0000, 0x4000, CRC(6bcf4100) SHA1(cc1744c6c513d6409a142b4fb42fbe70a95d9b7f))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("cieldisk.rom", 0x0000, 0x4000, CRC(bb550b09) SHA1(0274dd9b5096065a7f4ed019101124c9bd1d56b8))

	ROM_REGION(0x4000, "music", 0)
	ROM_LOAD("exp30mus.rom", 0x0000, 0x4000, CRC(9881b3fd) SHA1(befebc916bfdb5e8057040f0ae82b5517a7750db))

	ROM_REGION(0x10000, "ide", 0)
	ROM_LOAD("ide240a.rom", 0x00000, 0x10000, CRC(7adf857f) SHA1(8a919dbeed92db8c06a611279efaed8552810239))
ROM_END

void msx2_state::expert3i(machine_config &config)
{
	// AY8910/YM2149?
	// FDC: wd2793, 1 or 2? drives
	// 2 Cartridge slots?

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 1, 1, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_MUSIC, "music", 1, 1, 1, 1, "music").set_ym2413_tag("ym2413");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1, "disk", 1, 2, 1, 2, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");
	add_internal_slot(config, MSX_SLOT_ROM, "ide", 1, 3, 0, 4, "ide");         /* IDE hardware needs to be emulated */
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 2, 0, 4).set_total_size(0x40000);       // 256KB?? Mapper RAM
	add_cartridge_slot<2>(config, 3);

	msx_ym2413(config);

	msx_wd2793_force_ready(config);
	msx_1_35_dd_drive(config);
	msx2plus(AY8910, config);
}

/* MSX2+ - Ciel Expert 3 Turbo */

/* Uses a Z84C0010 - CMOS processor working at 7 MHz */
ROM_START(expert3t)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("exp30bios.rom", 0x0000, 0x8000, CRC(a10bb1ce) SHA1(5029cf47031b22bd5d1f68ebfd3be6d6da56dfe9))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("exp30ext.rom", 0x0000, 0x4000, CRC(6bcf4100) SHA1(cc1744c6c513d6409a142b4fb42fbe70a95d9b7f))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("cieldisk.rom", 0x0000, 0x4000, CRC(bb550b09) SHA1(0274dd9b5096065a7f4ed019101124c9bd1d56b8))

	ROM_REGION(0x4000, "music", 0)
	ROM_LOAD("exp30mus.rom", 0x0000, 0x4000, CRC(9881b3fd) SHA1(befebc916bfdb5e8057040f0ae82b5517a7750db))

	ROM_REGION(0x4000, "turbo", 0)
	ROM_LOAD("turbo.rom", 0x0000, 0x4000, CRC(ab528416) SHA1(d468604269ae7664ac739ea9f922a05e14ffa3d1))
ROM_END

void msx2_state::expert3t(machine_config &config)
{
	// AY8910
	// FDC: wd2793?, 1 or 2? drives
	// 4 Cartridge/Expansion slots?
	// FM/YM2413 built-in

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1, 0);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 1, 1, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_MUSIC, "music", 1, 1, 1, 1, "music").set_ym2413_tag("ym2413");
	add_internal_slot(config, MSX_SLOT_ROM, "turbo", 1, 2, 1, 1, "turbo");          /* Turbo hardware needs to be emulated */
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1, "disk", 1, 3, 1, 2, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 2, 0, 4).set_total_size(0x40000);       // 256KB Mapper RAM
	add_cartridge_slot<2>(config, 3);

	msx_ym2413(config);

	msx_wd2793_force_ready(config);
	msx_1_35_dd_drive(config);
	msx2plus(AY8910, config);
}

/* MSX2+ - Gradiente Expert AC88+ */

ROM_START(expertac)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("ac88bios.rom", 0x0000, 0x8000, CRC(9ce0da44) SHA1(1fc2306911ab6e1ebdf7cb8c3c34a7f116414e88))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("ac88ext.rom", 0x0000, 0x4000, CRC(c74c005c) SHA1(d5528825c7eea2cfeadd64db1dbdbe1344478fc6))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("panadisk.rom", 0x0000, 0x4000, CRC(17fa392b) SHA1(7ed7c55e0359737ac5e68d38cb6903f9e5d7c2b6))

	ROM_REGION(0x4000, "asm", 0)
	ROM_LOAD("ac88asm.rom", 0x0000, 0x4000, CRC(a8a955ae) SHA1(91e522473a8470511584df3ee5b325ea5e2b81ef))

	ROM_REGION(0x4000, "xbasic", 0)
	ROM_LOAD("xbasic2.rom", 0x0000, 0x4000, CRC(2825b1a0) SHA1(47370bec7ca1f0615a54eda548b07fbc0c7ef398))
ROM_END

void msx2_state::expertac(machine_config &config)
{
	// AY8910/YM2149?
	// FDC: wd2793?, 1 or 2? drives
	// 2 Cartridge slots?

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 0, 0, 4).set_total_size(0x10000);   // 64KB Mapper RAM??
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 1, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "asm", 3, 1, 1, 1, "asm");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1, "disk", 3, 2, 1, 2, "diskrom").set_tags("fdc", "fdc:0", "fdc:1");
	add_internal_slot(config, MSX_SLOT_ROM, "xbasic", 3, 3, 1, 1, "xbasic");

	msx_wd2793_force_ready(config);
	msx_1_35_dd_drive(config);
	msx2plus(AY8910, config);
}

/* MSX2+ - Gradiente Expert DDX+ */

ROM_START(expertdx)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("ddxbios.rom", 0x0000, 0x8000, CRC(e00af3dc) SHA1(5c463dd990582e677c8206f61035a7c54d8c67f0))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("ddxext.rom", 0x0000, 0x4000, CRC(b8ba44d3) SHA1(fe0254cbfc11405b79e7c86c7769bd6322b04995))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("panadisk.rom", 0x0000, 0x4000, CRC(17fa392b) SHA1(7ed7c55e0359737ac5e68d38cb6903f9e5d7c2b6))

	ROM_REGION(0x4000, "xbasic", 0)
	ROM_LOAD("xbasic2.rom", 0x0000, 0x4000, CRC(2825b1a0) SHA1(47370bec7ca1f0615a54eda548b07fbc0c7ef398))

	ROM_REGION(0x8000, "kanjirom", 0)
	ROM_LOAD("kanji.rom", 0x0000, 0x8000, CRC(b4fc574d) SHA1(dcc3a67732aa01c4f2ee8d1ad886444a4dbafe06))
ROM_END

void msx2_state::expertdx(machine_config &config)
{
	// AY8910/YM2149?
	// FDC: tc8566af, 1 3.5" DSDD drive?
	// 2 Cartridge slots?

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1, 0);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 1, 1, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "xbasic", 1, 2, 1, 1, "xbasic");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK3, "disk", 1, 3, 1, 2, "diskrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 2, 0, 4).set_total_size(0x10000);   // 64KB Mapper RAM??
	add_cartridge_slot<2>(config, 3);
	/* Kanji? */

	msx2plus(AY8910, config);
}

/* MSX2+ - Panasonic FS-A1FX */

ROM_START(fsa1fx)
	ROM_REGION(0x40000, "maincpu", 0)
	ROM_LOAD("a1fx.ic16", 0, 0x40000, CRC(c0b2d882) SHA1(623cbca109b6410df08ee7062150a6bda4b5d5d4))

	// Kanji rom contents are the first half of the single rom
//	ROM_REGION(0x20000, "kanji", 0)
	ROM_REGION(0x40000, "kanji", 0)
	ROM_LOAD("a1fx.ic16", 0, 0x40000, CRC(c0b2d882) SHA1(623cbca109b6410df08ee7062150a6bda4b5d5d4))
ROM_END

void msx2_state::fsa1fx(machine_config &config)
{
	// AY8910 (in T9769)
	// FDC: tc8566af, 1 3.5" DSDD drive
	// 2 Cartridge slots
	// T9769(B)
	// ren-sha turbo slider
	// pause button
	// firmware switch

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "maincpu", 0x30000);
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 0, 0, 4).set_total_size(0x10000).set_ramio_bits(0x80);   // 64KB Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 1, 0, 1, "maincpu", 0x38000);
	add_internal_slot(config, MSX_SLOT_ROM, "kdr", 3, 1, 1, 2, "maincpu", 0x28000);
	add_internal_disk_mirrored(config, MSX_SLOT_DISK3, "disk", 3, 2, 1, 2, "maincpu", 0x3c000);
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 3, 3, 1, 2, "maincpu", 0x20000);

	msx_matsushita_device &matsushita(MSX_MATSUSHITA(config, "matsushita", 0));
	matsushita.turbo_callback().set(FUNC(msx2_state::turbo_w));

	MSX_SYSTEMFLAGS(config, "sysflags", m_maincpu, 0xff);

	m_kanji_fsa1fx = true;
	msx2plus(AY8910, config);
}

/* MSX2+ - Panasonic FS-A1WSX */

ROM_START(fsa1wsx)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("a1wsbios.rom", 0x0000, 0x8000, CRC(358ec547) SHA1(f4433752d3bf876bfefb363c749d4d2e08a218b6))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("a1wsext.rom", 0x0000, 0x4000, CRC(b8ba44d3) SHA1(fe0254cbfc11405b79e7c86c7769bd6322b04995))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("a1wsdisk.rom", 0x0000, 0x4000, CRC(17fa392b) SHA1(7ed7c55e0359737ac5e68d38cb6903f9e5d7c2b6))

	ROM_REGION(0x8000, "kdr", 0)
	ROM_LOAD("a1wskdr.rom", 0x0000, 0x8000, CRC(b4fc574d) SHA1(dcc3a67732aa01c4f2ee8d1ad886444a4dbafe06))

	ROM_REGION(0x4000, "msxmusic", 0)
	ROM_LOAD("a1wsmusp.rom", 0x0000, 0x4000, CRC(5c32eb29) SHA1(aad42ba4289b33d8eed225d42cea930b7fc5c228))

	ROM_REGION(0x200000, "firmware", 0)
	ROM_LOAD("a1wsfirm.rom", 0x000000, 0x200000, CRC(e363595d) SHA1(3330d9b6b76e3c4ccb7cf252496ed15d08b95d3f))

	ROM_REGION(0x40000, "kanji", 0)
	ROM_LOAD("a1wskfn.rom", 0, 0x40000, CRC(1f6406fb) SHA1(5aff2d9b6efc723bc395b0f96f0adfa83cc54a49))
ROM_END

void msx2_state::fsa1wsx(machine_config &config)
{
	// AY8910 (in T9769)
	// FDC: tc8566af, 1 3.5" DSDD drive
	// 2 Cartridge slots
	// FM built-in
	// T9769(C)
	// ren-sha turbo slider
	// firmware switch
	// pause button

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_MUSIC, "msxmusic", 0, 2, 1, 1, "msxmusic").set_ym2413_tag("ym2413");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 0, 0, 4).set_total_size(0x10000).set_ramio_bits(0x80);   // 64KB Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 1, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "kdr", 3, 1, 1, 2, "kdr");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK3, "disk", 3, 2, 1, 2, "diskrom");
	add_internal_slot(config, MSX_SLOT_PANASONIC08, "firmware", 3, 3, 0, 4, "firmware");

	msx_matsushita_device &matsushita(MSX_MATSUSHITA(config, "matsushita", 0));
	matsushita.turbo_callback().set(FUNC(msx2_state::turbo_w));

	MSX_SYSTEMFLAGS(config, "sysflags", m_maincpu, 0xff);

	msx_ym2413(config);

	msx2plus(AY8910, config);
}

/* MSX2+ - Panasonic FS-A1WX */

ROM_START(fsa1wx)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("a1wxbios.rom", 0x0000, 0x8000, CRC(19771608) SHA1(e90f80a61d94c617850c415e12ad70ac41e66bb7))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("a1wxext.rom", 0x0000, 0x4000, CRC(b8ba44d3) SHA1(fe0254cbfc11405b79e7c86c7769bd6322b04995))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("a1wxdisk.rom", 0x0000, 0x4000, CRC(905daa1b) SHA1(bb59c849898d46a23fdbd0cc04ab35088e74a18d))

	ROM_REGION(0x8000, "kdr", 0)
	ROM_LOAD("a1wxkdr.rom", 0x0000, 0x8000, CRC(a068cba9) SHA1(1ef3956f7f918873fb9b031339bba45d1e5e5878))

	ROM_REGION(0x4000, "msxmusic", 0)
	ROM_LOAD("a1wxmusp.rom", 0x0000, 0x4000, CRC(456e494e) SHA1(6354ccc5c100b1c558c9395fa8c00784d2e9b0a3))

	ROM_REGION(0x200000, "firmware", 0)
	ROM_LOAD("a1wxfirm.rom", 0x000000, 0x200000, CRC(283f3250) SHA1(d37ab4bd2bfddd8c97476cbe7347ae581a6f2972))

	ROM_REGION(0x40000, "kanji", 0)
	ROM_LOAD("a1wxkfn.rom", 0, 0x40000, CRC(1f6406fb) SHA1(5aff2d9b6efc723bc395b0f96f0adfa83cc54a49))
ROM_END

void msx2_state::fsa1wx(machine_config &config)
{
	// AY8910 (in T9769)
	// FDC: tc8566af, 1 3.5" DSDD drive
	// 2 Cartridge slots
	// FM built-in
	// MSX Engine T9769A/B
	// ren-sha turbo slider
	// firmware switch
	// pause button

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_MUSIC, "msxmusic", 0, 2, 1, 1, "msxmusic").set_ym2413_tag("ym2413");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 0, 0, 4).set_total_size(0x10000).set_ramio_bits(0x80);   // 64KB Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 1, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "kdr", 3, 1, 1, 2, "kdr");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK3, "disk", 3, 2, 1, 2, "diskrom");
	add_internal_slot(config, MSX_SLOT_PANASONIC08, "firmware", 3, 3, 0, 4, "firmware");

	msx_matsushita_device &matsushita(MSX_MATSUSHITA(config, "matsushita", 0));
	matsushita.turbo_callback().set(FUNC(msx2_state::turbo_w));

	MSX_SYSTEMFLAGS(config, "sysflags", m_maincpu, 0xff);

	msx_ym2413(config);

	msx2plus(AY8910, config);
}

/* MSX2+ - Panasonic FS-A1WX (a) */

ROM_START(fsa1wxa)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("a1wxbios.rom", 0x0000, 0x8000, CRC(19771608) SHA1(e90f80a61d94c617850c415e12ad70ac41e66bb7))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("a1wxext.rom", 0x0000, 0x4000, CRC(b8ba44d3) SHA1(fe0254cbfc11405b79e7c86c7769bd6322b04995))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("a1wxdisk.rom", 0x0000, 0x4000, CRC(2bda0184) SHA1(2a0d228afde36ac7c5d3c2aac9c7c664dd071a8c))

	ROM_REGION(0x8000, "kdr", 0)
	ROM_LOAD("a1wxkdr.rom", 0x0000, 0x8000, CRC(a068cba9) SHA1(1ef3956f7f918873fb9b031339bba45d1e5e5878))

	ROM_REGION(0x4000, "msxmusic", 0)
	ROM_LOAD("a1wxmusp.rom", 0x0000, 0x4000, CRC(456e494e) SHA1(6354ccc5c100b1c558c9395fa8c00784d2e9b0a3))

	ROM_REGION(0x200000, "firmware", 0)
	ROM_LOAD("a1wxfira.rom", 0x000000, 0x200000, CRC(58440a8e) SHA1(8e0d4a77e7d5736e8225c2df4701509363eb230f))

	ROM_REGION(0x40000, "kanji", 0)
	ROM_LOAD("a1wxkfn.rom", 0, 0x40000, CRC(1f6406fb) SHA1(5aff2d9b6efc723bc395b0f96f0adfa83cc54a49))
ROM_END

/* MSX2+ - Sanyo Wavy PHC-35J */

ROM_START(phc35j)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("35jbios.rom", 0x0000, 0x8000, CRC(358ec547) SHA1(f4433752d3bf876bfefb363c749d4d2e08a218b6))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("35jext.rom", 0x0000, 0x4000, CRC(b8ba44d3) SHA1(fe0254cbfc11405b79e7c86c7769bd6322b04995))

	ROM_REGION(0x8000, "kdr", 0)
	ROM_LOAD("35jkdr.rom", 0x0000, 0x8000, CRC(b4fc574d) SHA1(dcc3a67732aa01c4f2ee8d1ad886444a4dbafe06))

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("35jkfn.rom", 0, 0x20000, CRC(c9651b32) SHA1(84a645becec0a25d3ab7a909cde1b242699a8662))
ROM_END

void msx2_state::phc35j(machine_config &config)
{
	// AY8910 (in T9769)
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// T9769A
	// ren-sha turbo slider
	// pause button

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 0, 0, 4).set_total_size(0x10000).set_ramio_bits(0x80);   // 64KB Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 1, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "kdr", 3, 1, 1, 2, "kdr");

	MSX_SYSTEMFLAGS(config, "sysflags", m_maincpu, 0xff);
	msx2plus(AY8910, config);
}

/* MSX2+ - Sanyo Wavy PHC-70FD */

ROM_START(phc70fd)
	ROM_REGION(0x20000, "mainrom", 0)
	ROM_LOAD("phc-70fd.rom", 0x0000, 0x20000, CRC(d2307ddf) SHA1(b6f2ca2e8a18d6c7cd326cb8d1a1d7d747f23059))
//	ROM_LOAD("70fdbios.rom", 0x0000, 0x8000, CRC(19771608) SHA1(e90f80a61d94c617850c415e12ad70ac41e66bb7))
//	ROM_LOAD("70fdext.rom",  0x8000, 0x4000, CRC(b8ba44d3) SHA1(fe0254cbfc11405b79e7c86c7769bd6322b04995))
//	ROM_LOAD("70fddisk.rom", 0xc000, 0x4000, CRC(db7f1125) SHA1(9efa744be8355675e7bfdd3976bbbfaf85d62e1d))
//	ROM_LOAD("70fdkdr.rom", 0x10000, 0x8000, CRC(a068cba9) SHA1(1ef3956f7f918873fb9b031339bba45d1e5e5878))
//	ROM_LOAD("70fdmus.rom", 0x18000, 0x4000, CRC(5c32eb29) SHA1(aad42ba4289b33d8eed225d42cea930b7fc5c228))
//	ROM_LOAD("70fdbas.rom", 0x1c000, 0x4000, CRC(da7be246) SHA1(22b3191d865010264001b9d896186a9818478a6b))

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("70fdkfn.rom", 0, 0x20000, CRC(c9651b32) SHA1(84a645becec0a25d3ab7a909cde1b242699a8662))
ROM_END

void msx2_state::phc70fd(machine_config &config)
{
	// AY8910 (in T9769)
	// FDC: tc8566af, 1 3.5" DSDD drive
	// 2 Cartridge slots
	// T9769
	// FM built-in
	// ren-sha turbo slider
	// pause button

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom", 0x10000);
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 0, 0, 4).set_total_size(0x10000).set_ramio_bits(0x80);   // 64KB Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 1, 0, 1, "mainrom", 0x18000);
	add_internal_slot(config, MSX_SLOT_ROM, "kdr", 3, 1, 1, 2, "mainrom", 0x8000);
	add_internal_disk_mirrored(config, MSX_SLOT_DISK3, "disk", 3, 2, 1, 2, "mainrom", 0x1c000);
	add_internal_slot(config, MSX_SLOT_MUSIC, "msxmusic", 3, 3, 1, 1, "mainrom", 0x00000).set_ym2413_tag("ym2413");
	add_internal_slot(config, MSX_SLOT_ROM, "basickun", 3, 3, 2, 1, "mainrom", 0x04000);

	MSX_SYSTEMFLAGS(config, "sysflags", m_maincpu, 0xff);

	msx_ym2413(config);

	msx2plus(AY8910, config);
}

/* MSX2+ - Sanyo Wavy PHC-70FD2 */

ROM_START(phc70fd2)
	ROM_REGION(0x20000, "mainrom", 0)
	ROM_LOAD("70f2bios.rom", 0x0000, 0x8000, CRC(19771608) SHA1(e90f80a61d94c617850c415e12ad70ac41e66bb7))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("70f2ext.rom", 0x0000, 0x4000, CRC(b8ba44d3) SHA1(fe0254cbfc11405b79e7c86c7769bd6322b04995))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("70f2disk.rom", 0x0000, 0x4000, CRC(db7f1125) SHA1(9efa744be8355675e7bfdd3976bbbfaf85d62e1d))

	ROM_REGION(0x8000, "kdr", 0)
	ROM_LOAD("70f2kdr.rom", 0x0000, 0x8000, CRC(a068cba9) SHA1(1ef3956f7f918873fb9b031339bba45d1e5e5878))

	ROM_REGION(0x4000, "msxmusic", 0)
	ROM_LOAD("70f2mus.rom", 0x0000, 0x4000, CRC(5c32eb29) SHA1(aad42ba4289b33d8eed225d42cea930b7fc5c228))

	ROM_REGION(0x4000, "basickun", 0)
	ROM_LOAD("70f2bas.rom", 0x0000, 0x4000, CRC(da7be246) SHA1(22b3191d865010264001b9d896186a9818478a6b))

	ROM_REGION(0x40000, "kanji", 0)
	ROM_LOAD("70f2kfn.rom", 0, 0x40000, CRC(9a850db9) SHA1(bcdb4dae303dfe5234f372d70a5e0271d3202c36))
ROM_END

void msx2_state::phc70fd2(machine_config &config)
{
	// AY8910
	// FDC: tc8566af, 2 3.5" DSDD drives
	// 2 Cartridge slots
	// FM built-in
	// T9769

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 0, 0, 4).set_total_size(0x10000).set_ramio_bits(0x80);   // 64KB Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 1, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "kdr", 3, 1, 1, 2, "kdr");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK3_2_DRIVES, "disk", 3, 2, 1, 2, "diskrom");
	add_internal_slot(config, MSX_SLOT_MUSIC, "msxmusic", 3, 3, 1, 1, "msxmusic").set_ym2413_tag("ym2413");
	add_internal_slot(config, MSX_SLOT_ROM, "basickun", 3, 3, 2, 1, "basickun");

	MSX_SYSTEMFLAGS(config, "sysflags", m_maincpu, 0xff);

	msx_ym2413(config);

	msx2plus(AY8910, config);
}

/* MSX2+ - Sony HB-F1XDJ */

ROM_START(hbf1xdj)
	ROM_REGION(0x20000, "mainrom", 0)
	ROM_LOAD("hb-f1xdj_main.rom", 0x0000, 0x20000, CRC(d89bab74) SHA1(f2a1d326d72d4c70ea214d7883838de8847a82b7))

	ROM_REGION(0x100000, "firmware", 0)
	ROM_LOAD("f1xjfirm.rom", 0x000000, 0x100000, CRC(77be583f) SHA1(ade0c5ba5574f8114d7079050317099b4519e88f))

	ROM_REGION(0x40000, "kanji", 0)
	ROM_LOAD("f1xjkfn.rom", 0, 0x40000, CRC(7016dfd0) SHA1(218d91eb6df2823c924d3774a9f455492a10aecb))
ROM_END

void msx2_state::hbf1xdj(machine_config &config)
{
	// YM2149 (in S-1985 MSX Engine)
	// FDC: MB89311, 1 3.5" DSDD drive
	// 2 Cartridge slots
	// FM built-in
	// S-1985 MSX Engine
	// speed controller
	// pause button
	// ren-sha turbo

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_SONY08, "firmware", 0, 3, 0, 4, "firmware");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 0, 0, 4).set_total_size(0x10000).set_ramio_bits(0x80);   // 64KB Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 1, 0, 1, "mainrom", 0x8000);
	add_internal_slot(config, MSX_SLOT_ROM, "kdr", 3, 1, 1, 2, "mainrom", 0x10000);
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1, "disk", 3, 2, 1, 2, "mainrom", 0xc000).set_tags("fdc", "fdc:0", "fdc:1");
	add_internal_slot(config, MSX_SLOT_MUSIC, "msxmusic", 3, 3, 1, 1, "mainrom", 0x18000).set_ym2413_tag("ym2413");

	MSX_SYSTEMFLAGS(config, "sysflags", m_maincpu, 0x00);

	MSX_S1985(config, "s1985", 0);

	msx_ym2413(config);

	msx_wd2793(config);
	msx_1_35_dd_drive(config);
	msx2plus(YM2149, config);
}

/* MSX2+ - Sony HB-F1XV */

ROM_START(hbf1xv)
	ROM_REGION(0x20000, "mainrom", 0)
	ROM_LOAD("hb-f1xdj_main.rom", 0x0000, 0x20000, CRC(d89bab74) SHA1(f2a1d326d72d4c70ea214d7883838de8847a82b7))

	ROM_REGION(0x100000, "firmware", 0)
	ROM_LOAD("f1xvfirm.rom", 0x0, 0x100000, CRC(77be583f) SHA1(ade0c5ba5574f8114d7079050317099b4519e88f))

	ROM_REGION(0x40000, "kanji", 0)
	ROM_LOAD("f1xvkfn.rom", 0, 0x40000, CRC(7016dfd0) SHA1(218d91eb6df2823c924d3774a9f455492a10aecb))
ROM_END

void msx2_state::hbf1xv(machine_config &config)
{
	// YM2149 (in S-1985 MSX Engine)
	// FDC: mb89311, 1 3.5" DSDD drives
	// 2 Cartridge slots
	// FM built-in
	// S-1985 MSX Engine
	// speed controller
	// pause button
	// ren-sha turbo

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_SONY08, "firmware", 0, 3, 0, 4, "firmware");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 0, 0, 4).set_total_size(0x10000).set_ramio_bits(0x80);   // 64KB Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 1, 0, 1, "mainrom", 0x8000);
	add_internal_slot(config, MSX_SLOT_ROM, "kdr", 3, 1, 1, 2, "mainrom", 0x10000);
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1, "disk", 3, 2, 1, 2, "mainrom", 0xc000).set_tags("fdc", "fdc:0", "fdc:1");
	add_internal_slot(config, MSX_SLOT_MUSIC, "msxmusic", 3, 3, 1, 1, "mainrom", 0x18000).set_ym2413_tag("ym2413");

	MSX_SYSTEMFLAGS(config, "sysflags", m_maincpu, 0x00);

	MSX_S1985(config, "s1985", 0);

	msx_ym2413(config);

	msx_wd2793(config);
	msx_1_35_dd_drive(config);
	msx2plus(YM2149, config);
}

/* MSX Turbo-R - Panasonic FS-A1GT */

ROM_START(fsa1gt)
	ROM_REGION(0x46c000, "maincpu", 0)
	ROM_LOAD("a1gtbios.rom",  0x0000,   0x8000, CRC(937c8dbb) SHA1(242e73d8284a012b275c0a266844ebbc4269d787))
	ROM_LOAD("a1gtext.rom",   0x8000,   0x4000, CRC(70aea0fe) SHA1(018d7a5222f28514908fb1b1513286a6558a6d05))
	ROM_LOAD("a1gtdos.rom",   0xc000,  0x10000, CRC(bb2a0eae) SHA1(4880bf34f1c86fff5456ec2b4cf70d02339e2caa))
	ROM_LOAD("a1gtkdr.rom",  0x1c000,   0x8000, CRC(eaf0d125) SHA1(5b39c1ccd3a213b78e02927f56a9abc72cd8c28d))
	ROM_LOAD("a1gtmus.rom",  0x24000,   0x4000, CRC(f5f93437) SHA1(6aea1aef5ec31c1826c22edf580525f93baad425))
	ROM_LOAD("a1gtopt.rom",  0x28000,   0x4000, CRC(50d11f60) SHA1(b4433a3975c57dd440d6bf12dbd28b2ac1b90ef4))
	ROM_LOAD("a1gtkfn.rom",  0x2c000,  0x40000, CRC(1f6406fb) SHA1(5aff2d9b6efc723bc395b0f96f0adfa83cc54a49))
	ROM_LOAD("a1gtfirm.rom", 0x6c000, 0x400000, CRC(feefeadc) SHA1(e779c338eb91a7dea3ff75f3fde76b8af22c4a3a))
ROM_END

void msx2_state::fsa1gt(machine_config &config)
{
	// AY8910 (in T9769)
	// FDC: tc8566af, 1 3.5" DSDD drive
	// 2 Cartridge slots
	// T9769C + S1990
	// FM built-in
	// Microphone
	// MIDI-in
	// MIDI-out
	// firmware switch
	// pause button
	// ren-sha turbo slider

	add_internal_slot(config, MSX_SLOT_ROM, "bios", 0, 0, 0, 2, "maincpu");
	add_internal_slot(config, MSX_SLOT_MUSIC, "mus", 0, 2, 1, 1, "maincpu", 0x24000).set_ym2413_tag("ym2413");
	add_internal_slot(config, MSX_SLOT_ROM, "opt", 0, 3, 1, 1, "maincpu", 0x28000);
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 0, 0, 4).set_total_size(0x20000);   // 128KB?? Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "ext", 3, 1, 0, 1, "maincpu", 0x8000);
	add_internal_slot(config, MSX_SLOT_ROM, "kdr", 3, 1, 1, 2, "maincpu", 0x1c000);
	add_internal_disk(config, MSX_SLOT_DISK4, "dos", 3, 2, 1, 3, "maincpu", 0xc000);
	add_internal_slot(config, MSX_SLOT_ROM, "firm", 3, 3, 0, 4, "maincpu", 0x6c000);

	MSX_SYSTEMFLAGS(config, "sysflags", m_maincpu, 0x00);

	msx_ym2413(config);

	turbor(AY8910, config);
}

/* MSX Turbo-R - Panasonic FS-A1ST */

ROM_START(fsa1st)
	ROM_REGION(0x46c000, "maincpu", 0)
	ROM_LOAD("a1stbios.rom",  0x0000,   0x8000, CRC(77b94ae0) SHA1(f078b5ec56884bfb81481d45c7151418770bff5a))
	ROM_LOAD("a1stext.rom",   0x8000,   0x4000, CRC(2c2c77a4) SHA1(373412f9c32762de1c3a7e27fc3d80614e0a0c8e))
	ROM_LOAD("a1stdos.rom",   0xc000,  0x10000, CRC(1fc71407) SHA1(5d2186658adcf4ce0c2d3232384b5712341108e5))
	ROM_LOAD("a1stkdr.rom",  0x1c000,   0x8000, CRC(eaf0d125) SHA1(5b39c1ccd3a213b78e02927f56a9abc72cd8c28d))
	ROM_LOAD("a1stmus.rom",  0x24000,   0x4000, CRC(fd7dec41) SHA1(e002a9b426732e6c2d31e548c40cf7c122348ce3))
	ROM_LOAD("a1stopt.rom",  0x28000,   0x4000, CRC(c6a4a2a1) SHA1(cb06dea7b025745f9d2b87dcf03ded615287ead3))
	ROM_LOAD("a1stkfn.rom",  0x2c000,  0x40000, CRC(1f6406fb) SHA1(5aff2d9b6efc723bc395b0f96f0adfa83cc54a49))
	ROM_LOAD("a1stfirm.rom", 0x6c000, 0x400000, CRC(139ac99c) SHA1(c212b11fda13f83dafed688c54d098e7e47ab225))
ROM_END

void msx2_state::fsa1st(machine_config &config)
{
	// AY8910 (in T9769)
	// FDC: tc8566af, 1 3.5" DSDD drive
	// T9769C + S1990
	// 2 Cartridge slots
	// FM built-in
	// microphone
	// firmware switch
	// pause button
	// ren-sha turbo slider

	add_internal_slot(config, MSX_SLOT_ROM, "bios", 0, 0, 0, 2, "maincpu");
	add_internal_slot(config, MSX_SLOT_MUSIC, "mus", 0, 2, 1, 1, "maincpu", 0x24000).set_ym2413_tag("ym2413");
	add_internal_slot(config, MSX_SLOT_ROM, "opt", 0, 3, 1, 1, "maincpu", 0x28000);
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 0, 0, 4).set_total_size(0x20000);   // 128KB?? Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "ext", 3, 1, 0, 1, "maincpu", 0x8000);
	add_internal_slot(config, MSX_SLOT_ROM, "kdr", 3, 1, 1, 2, "maincpu", 0x1c000);
	add_internal_disk(config, MSX_SLOT_DISK4, "dos", 3, 2, 1, 3, "maincpu", 0xc000);
	add_internal_slot(config, MSX_SLOT_ROM, "firm", 3, 3, 0, 4, "maincpu", 0x6c000);

	MSX_SYSTEMFLAGS(config, "sysflags", m_maincpu, 0x00);

	msx_ym2413(config);

	turbor(AY8910, config);
}

} // anonymous namespace


/*   YEAR  NAME        PARENT    COMPAT MACHINE     INPUT     CLASS      INIT        COMPANY       FULLNAME */
/* MSX1 */
COMP(1986, perfect1,   0,        0,     perfect1,   msx,      msx_state, empty_init, "Bawareth", "Perfect MSX1 (Middle East) (MSX1)", 0)
COMP(1985, canonv8,    0,        0,     canonv8,    msxjp,    msx_state, empty_init, "Canon", "V-8 (Japan) (MSX1)", 0)
COMP(1984, canonv10,   canonv20, 0,     canonv10,   msxjp,    msx_state, empty_init, "Canon", "V-10 (Japan) (MSX1)", 0)
COMP(1984, canonv20,   0,        0,     canonv20,   msxjp,    msx_state, empty_init, "Canon", "V-20 (Japan) (MSX1)", 0)
COMP(1985, canonv20e,  canonv20, 0,     canonv20e,  msxuk,    msx_state, empty_init, "Canon", "V-20E (UK) (MSX1)", 0)
COMP(1985, canonv20f,  canonv20, 0,     canonv20e,  msxfr,    msx_state, empty_init, "Canon", "V-20F (France) (MSX1)", 0)
COMP(198?, canonv20g,  canonv20, 0,     canonv20e,  msxde,    msx_state, empty_init, "Canon", "V-20G (MSX1)", 0)
COMP(198?, canonv20s,  canonv20, 0,     canonv20e,  msxsp,    msx_state, empty_init, "Canon", "V-20S (Spain) (MSX1)", 0)
COMP(1985, mx10,       mx15,     0,     mx10,       msxjp,    msx_state, empty_init, "Casio", "MX-10 (Japan) (MSX1)", 0)
COMP(1987, mx101,      mx15,     0,     mx101,      msxjp,    msx_state, empty_init, "Casio", "MX-101 (Japan) (MSX1)", 0)
COMP(1986, mx15,       0,        0,     mx15,       msx,      msx_state, empty_init, "Casio", "MX-15 (International) (MSX1)", 0)
COMP(1984, pv7,        pv16,     0,     pv7,        msxjp,    msx_state, empty_init, "Casio", "PV-7 (Japan) (MSX1)", 0)
COMP(1985, pv16,       0,        0,     pv16,       msxjp,    msx_state, empty_init, "Casio", "PV-16 (Japan) (MSX1)", 0)
COMP(1984, cpc88,      0,        0,     cpc88,      msxkr,    msx_state, empty_init, "Daewoo", "CPC-88 (Korea) (MSX1)", 0)
COMP(1984, dpc100,     dpc200,   0,     dpc100,     msxkr,    msx_state, empty_init, "Daewoo", "IQ-1000 DPC-100 (Korea) (MSX1)", 0)
COMP(1984, dpc180,     dpc200,   0,     dpc180,     msxkr,    msx_state, empty_init, "Daewoo", "IQ-1000 DPC-180 (Korea) (MSX1)", 0)
COMP(1984, dpc200,     0,        0,     dpc200,     msxkr,    msx_state, empty_init, "Daewoo", "IQ-1000 DPC-200 (Korea) (MSX1)", 0)
COMP(1985, dpc200e,    0,        0,     dpc200e,    msxfr,    msx_state, empty_init, "Daewoo", "DPC-200E (French) (MSX1)", 0)
COMP(1986, cpc50a,     cpc51,    0,     cpc50a,     msxkr,    msx_state, empty_init, "Daewoo", "Zemmix CPC-50A (Korea) (MSX1)", 0)
COMP(1987, cpc50b,     cpc51,    0,     cpc50b,     msxkr,    msx_state, empty_init, "Daewoo", "Zemmix CPC-50B (Korea) (MSX1)", 0)
COMP(1988, cpc51,      0,        0,     cpc51,      msxkr,    msx_state, empty_init, "Daewoo", "Zemmix CPC-51 (Korea) (MSX1)", 0)
COMP(1985, dgnmsx,     0,        0,     dgnmsx,     msxuk,    msx_state, empty_init, "Eurohard S.A.", "Dragon MSX-64 (Spain) (MSX1)", 0)
COMP(1985, fdpc200,    0,        0,     fdpc200,    msx,      msx_state, empty_init, "Fenner", "DPC-200 (Italy) (MSX1)", 0)
COMP(1985, fpc500,     0,        0,     fpc500,     msx,      msx_state, empty_init, "Fenner", "FPC-500 (Italy) (MSX1)", 0)
COMP(1984, fspc800,    0,        0,     fspc800,    msxuk,    msx_state, empty_init, "Fenner", "SPC-800 (Italy) (MSX1)", 0)
COMP(1987, bruc100,    0,        0,     bruc100,    bruc100,  bruc100_state, empty_init, "Frael", "Bruc 100-1 (Italy) (MSX1)", 0)
COMP(1988, bruc100a,   bruc100,  0,     bruc100a,   bruc100,  bruc100_state, empty_init, "Frael", "Bruc 100-2 (Italy) (MSX1)", 0)
COMP(1983, fmx,        0,        0,     fmx,        msxjp,    msx_state, empty_init, "Fujitsu", "FM-X (Japan) (MSX1)", 0)
COMP(1984, gsfc80u,    0,        0,     gsfc80u,    msxkr,    msx_state, empty_init, "Goldstar", "FC-80U (Korea) (MSX1)", 0)
COMP(1984, gsfc200,    0,        0,     gsfc200,    msx,      msx_state, empty_init, "Goldstar", "FC-200 (Europe) (MSX1)", 0)
COMP(1985, gfc1080,    0,        0,     gfc1080,    msxkr,    msx_state, empty_init, "Goldstar", "GFC-1080 (Koera) (MSX1)", 0)
COMP(1985, gfc1080a,   0,        0,     gfc1080a,   msxkr,    msx_state, empty_init, "Goldstar", "GFC-1080A (Korea) (MSX1)", 0)
COMP(1985, expert10,   expert13, 0,     expert10,   expert10, msx_state, empty_init, "Gradiente", "Expert XP-800 (1.0) (Brazil) (MSX1)", 0)
COMP(198?, expert11,   expert13, 0,     expert11,   expert11, msx_state, empty_init, "Gradiente", "Expert XP-800 (1.1) / Expert GPC-1 (Brazil) (MSX1)", 0)
COMP(198?, expert13,   0,        0,     expert13,   expert11, msx_state, empty_init, "Gradiente", "Expert 1.3 (Brazil) (MSX1)", 0)
COMP(1989, expertdp,   0,        0,     expertdp,   expert11, msx_state, empty_init, "Gradiente", "Expert DDPlus (Brazil) (MSX1)", 0)
COMP(1989, expertpl,   0,        0,     expertpl,   expert11, msx_state, empty_init, "Gradiente", "Expert Plus (Brazil) (MSX1)", 0)
COMP(1983, mbh1,       0,        0,     mbh1,       msxjp,    msx_state, empty_init, "Hitachi", "MB-H1 (Japan) (MSX1)", 0)
COMP(1984, mbh1e,      mbh1,     0,     mbh1e,      msxjp,    msx_state, empty_init, "Hitachi", "MB-H1E (Japan) (MSX1)", 0)
COMP(1984, mbh2,       0,        0,     mbh2,       msxjp,    msx_state, empty_init, "Hitachi", "MB-H2 (Japan) (MSX1)", 0)
COMP(1988, mbh25,      0,        0,     mbh25,      msxjp,    msx_state, empty_init, "Hitachi", "MB-H25 (Japan) (MSX1)", 0)
COMP(1986, mbh50,      0,        0,     mbh50,      msxjp,    msx_state, empty_init, "Hitachi", "MB-H50 (Japan) (MSX1)", 0)
COMP(1985, jvchc7gb,   0,        0,     jvchc7gb,   msxuk,    msx_state, empty_init, "JVC", "HC-7E / HC-7GB (Europe) (MSX1)", 0)
COMP(1983, ml8000,     0,        0,     ml8000,     msxjp,    msx_state, empty_init, "Mitsubishi", "ML-8000 (Japan) (MSX1)", 0)
COMP(1984, mlf48,      0,        0,     mlf48,      msxuk,    msx_state, empty_init, "Mitsubishi", "ML-F48 (UK) (MSX1)", 0)
COMP(1984, mlf80,      0,        0,     mlf80,      msxuk,    msx_state, empty_init, "Mitsubishi", "ML-F80 (UK) (MSX1)", 0)
COMP(1984, mlf110,     0,        0,     mlf110,     msxjp,    msx_state, empty_init, "Mitsubishi", "ML-F110 (Japan) (MSX1)", 0)
COMP(1984, mlf120,     0,        0,     mlf120,     msxjp,    msx_state, empty_init, "Mitsubishi", "ML-F120 (Japan) (MSX1)", 0)
COMP(1986, mlfx1,      0,        0,     mlfx1,      mlfx1,    msx_state, empty_init, "Mitsubishi", "ML-FX1 (Spain) (MSX1)", 0)
COMP(1985, cf1200,     0,        0,     cf1200,     msxjp,    msx_state, empty_init, "National", "CF-1200 (Japan) (MSX1)", 0)
COMP(1983, cf2000,     0,        0,     cf2000,     msxjp,    msx_state, empty_init, "National", "CF-2000 (Japan) (MSX1)", 0)
COMP(1984, cf2700,     0,        0,     cf2700,     msxjp,    msx_state, empty_init, "National", "CF-2700 (Japan) (MSX1)", 0)
COMP(1984, cf3000,     0,        0,     cf3000,     cf3000,   msx_state, empty_init, "National", "CF-3000 (Japan) (MSX1)", 0)
COMP(1985, cf3300,     0,        0,     cf3300,     cf3000,   msx_state, empty_init, "National", "CF-3300 (Japan) (MSX1)", 0)
COMP(1985, fs1300,     0,        0,     fs1300,     msxjp,    msx_state, empty_init, "National", "FS-1300 (Japan) (MSX1)", 0)
COMP(1985, fs4000,     0,        0,     fs4000,     fs4000,   msx_state, empty_init, "National", "FS-4000 (Japan) (MSX1)", 0)
COMP(1985, fs4000a,    fs4000,   0,     fs4000a,    fs4000,   msx_state, empty_init, "National", "FS-4000 (alt) (Japan) (MSX1)", 0)
COMP(1985, phc2,       0,        0,     phc2,       msxfr,    msx_state, empty_init, "Olympia", "PHC-2 (France) (MSX1)" , 0)
COMP(1984, phc28,      0,        0,     phc28,      msxfr,    msx_state, empty_init, "Olympia", "PHC-28 (France) (MSX1)", 0)
COMP(1985, cf2700g,    cf2700uk, 0,     cf2700g,    msxde,    msx_state, empty_init, "Panasonic", "CF-2700 (Germany) (MSX1)", 0)
COMP(1985, cf2700uk,   0,        0,     cf2700uk,   msxuk,    msx_state, empty_init, "Panasonic", "CF-2700 (UK) (MSX1)", 0)
COMP(1989, nms801,     0,        0,     nms801,     msx,      msx_state, empty_init, "Philips", "NMS-801 (Italy) (MSX1)", 0)
COMP(1984, vg8000,     vg8010,   0,     vg8000,     vg8010,   msx_state, empty_init, "Philips", "VG-8000 (Europe) (MSX1)", 0)
COMP(1985, vg8010,     0,        0,     vg8010,     vg8010,   msx_state, empty_init, "Philips", "VG-8010 / VG-8010/00 (Europe) (MSX1)", 0)
COMP(1985, vg8010f,    vg8010,   0,     vg8010f,    vg8010f,  msx_state, empty_init, "Philips", "VG-8010F / VG-8010/19 (French) (MSX1)" , 0)
COMP(1985, vg802000,   vg802020, 0,     vg802000,   msx,      msx_state, empty_init, "Philips", "VG-8020/00 (Europe) (MSX1)", 0)
COMP(1985, vg8020f,    vg802020, 0,     vg8020f,    msxfr,    msx_state, empty_init, "Philips", "VG-8020/19 / VG-8020F (French) (MSX1)", 0)
COMP(1985, vg802020,   0,        0,     vg802020,   msx,      msx_state, empty_init, "Philips", "VG-8020/20 (Europe) (MSX1)", 0)
COMP(1984, piopx7,     0,        0,     piopx7,     msxjp,    msx_state, empty_init, "Pioneer", "PX-07 Palcom (Japan) (MSX1)", 0)
COMP(1985, piopx7uk,   piopx7,   0,     piopx7uk,   msxuk,    msx_state, empty_init, "Pioneer", "PX-07UK Palcom (UK) (MSX1)", 0)
COMP(1986, piopxv60,   piopx7,   0,     piopxv60,   msxjp,    msx_state, empty_init, "Pioneer", "PX-V60 (Japan) (MSX1)", 0)
COMP(1986, ax150,      0,        0,     ax150,      msx,      msx_state, empty_init, "Sakhr", "AX-150 (Arabic) (MSX1)", 0)
COMP(1986, ax170,      0,        0,     ax170,      msx,      msx_state, empty_init, "Sakhr", "AX-170 (Arabic) (MSX1)", 0)
COMP(1986, ax200,      0,        0,     ax200,      msx,      msx1_v9938_state, empty_init, "Sakhr", "AX-200 (Arabic/English) (MSX1)", 0)
COMP(1986, ax200m,     ax200,    0,     ax200m,     msx,      msx1_v9938_state, empty_init, "Sakhr", "AX-200M (Arabic/English) (MSX1)", 0)
COMP(1986, ax230,      0,        0,     ax230,      msx,      msx_state, empty_init, "Sakhr", "AX-230 (Arabic) (MSX1)", MACHINE_IMPERFECT_GRAPHICS)
COMP(1984, spc800,     0,        0,     spc800,     msxkr,    msx_state, empty_init, "Samsung", "SPC-800 (Korea) (MSX1)", 0)
COMP(1983, mpc10,      0,        0,     mpc10,      msxjp,    msx_state, empty_init, "Sanyo", "MPC-10 / Wavy10 (Japan) (MSX1)", 0)
COMP(1985, mpc64,      0,        0,     mpc64,      msxde,    msx_state, empty_init, "Sanyo", "MPC-64 (Germany) (MSX1)", 0)
COMP(1985, mpc100,     0,        0,     mpc100,     msxuk,    msx_state, empty_init, "Sanyo", "MPC-100 (UK) (MSX1)", 0)
COMP(1985, mpc200,     0,        0,     mpc200,     msxuk,    msx_state, empty_init, "Sanyo", "MPC-200 (UK) (MSX1)", 0) // Is this the same as MPC-200SP, is it even real ?
COMP(1985, mpc200sp,   mpc200,   0,     mpc200sp,   msxsp,    msx_state, empty_init, "Sanyo", "MPC-200SP (Spain) (MSX1)", 0)
COMP(1985, phc28l,     0,        0,     phc28l,     msxfr,    msx_state, empty_init, "Sanyo", "PHC-28L (France) (MSX1)", 0)
COMP(1984, phc28s,     0,        0,     phc28s,     msxuk,    msx_state, empty_init, "Sanyo", "PHC-28S (France) (MSX1)", 0)
COMP(1985, hb8000,     0,        0,     hb8000,     hotbit,   msx_state, empty_init, "Sharp / Epcom", "HB-8000 (Hotbit) (Brazil) (MSX1)", 0)
COMP(198?, hotbi13b,   hotbi13p, 0,     hotbi13b,   hotbit,   msx_state, empty_init, "Sharp / Epcom", "HB-8000 Hotbit 1.3b (Brazil) (MSX1)", 0)
COMP(198?, hotbi13p,   0,        0,     hotbi13p,   hotbit,   msx_state, empty_init, "Sharp / Epcom", "HB-8000 Hotbit 1.3p (Brazil) (MSX1)", 0)
COMP(1985, hb10,       hb10p,    0,     hb10,       msxjp,    msx_state, empty_init, "Sony", "HB-10 (Japan) (MSX1)", 0)
COMP(1986, hb10p,      0,        0,     hb10p,      msxuk,    msx_state, empty_init, "Sony", "HB-10P (Netherlands) (MSX1)", 0)
COMP(1984, hb101,      hb101p,   0,     hb101,      msxjp,    msx_state, empty_init, "Sony", "HB-101 (Japan) (MSX1)", 0)
COMP(1984, hb101p,     0,        0,     hb101p,     msxuk,    msx_state, empty_init, "Sony", "HB-101P (Europe) (MSX1)", 0)
COMP(1986, hb20p,      0,        0,     hb20p,      msxsp,    msx_state, empty_init, "Sony", "HB-20P (Spain) (MSX1)", 0)
COMP(1985, hb201,      hb201p,   0,     hb201,      msxjp,    msx_state, empty_init, "Sony", "HB-201 (Japan) (MSX1)", 0)
COMP(1985, hb201p,     0,        0,     hb201p,     msxuk,    msx_state, empty_init, "Sony", "HB-201P (Europe) (MSX1)", 0)
COMP(1984, hb501p,     0,        0,     hb501p,     msxuk,    msx_state, empty_init, "Sony", "HB-501P (Europe) (MSX1)", 0)
COMP(1983, hb55,       hb55p,    0,     hb55,       msxjp,    msx_state, empty_init, "Sony", "HB-55 (Japan) (MSX1)", 0)
COMP(1984, hb55d,      hb55p,    0,     hb55d,      msxde,    msx_state, empty_init, "Sony", "HB-55D (Germany) (MSX1)", 0)
COMP(1984, hb55p,      0,        0,     hb55p,      msxuk,    msx_state, empty_init, "Sony", "HB-55P (Europe) (MSX1)", 0)
COMP(1984, hb701fd,    0,        0,     hb701fd,    msxjp,    msx_state, empty_init, "Sony", "HB-701FD (Japan) (MSX1)", 0)
COMP(1984, hb75,       hb75p,    0,     hb75,       msxjp,    msx_state, empty_init, "Sony", "HB-75 (Japan) (MSX1)", 0)
COMP(1984, hb75d,      hb75p,    0,     hb75d,      msxde,    msx_state, empty_init, "Sony", "HB-75D (Germany) (MSX1)", 0)
COMP(1984, hb75p,      0,        0,     hb75p,      msxuk,    msx_state, empty_init, "Sony", "HB-75P (Europe) (MSX1)", 0)
COMP(1984, svi728,     0,        0,     svi728,     svi728,   msx_state, empty_init, "Spectravideo", "SVI-728 (International) (MSX1)", 0)
COMP(1984, svi728es,   svi728,   0,     svi728,     svi728sp, msx_state, empty_init, "Spectravideo", "SVI-728 (Spanish) (MSX1)", 0)
COMP(1985, svi738,     0,        0,     svi738,     msx,      msx1_v9938_state, empty_init, "Spectravideo", "SVI-738 (International) (MSX1)", 0)
COMP(1987, svi738ar,   svi738,   0,     svi738ar,   msx,      msx1_v9938_state, empty_init, "Spectravideo", "SVI-738 (Arabic) (MSX1)", 0)
COMP(1985, svi738dk,   svi738,   0,     svi738,     svi738dk, msx1_v9938_state, empty_init, "Spectravideo", "SVI-738 (Denmark, Norway) (MSX1)", 0)
COMP(1986, svi738pl,   svi738,   0,     svi738,     msx,      msx1_v9938_state, empty_init, "Spectravideo", "SVI-738 (Poland) (MSX1)", 0)
COMP(1985, svi738sp,   svi738,   0,     svi738,     msxsp,    msx1_v9938_state, empty_init, "Spectravideo", "SVI-738 (Spain) (MSX1)", 0)
COMP(1985, svi738sw,   svi738,   0,     svi738,     svi738sw, msx1_v9938_state, empty_init, "Spectravideo", "SVI-738 (Finland, Sweden) (MSX1)", 0)
COMP(1986, tadpc200,   dpc200,   0,     tadpc200,   msxsp,    msx_state, empty_init, "Talent", "DPC-200 (Argentina) (MSX1, Spanish keyboard)", 0)
COMP(1986, tadpc200b,  dpc200,   0,     tadpc200,   msx,      msx_state, empty_init, "Talent", "DPC-200 (Argentina) (MSX1, international keyboard)", 0)
COMP(1988, tadpc200a,  dpc200,   0,     tadpc200a,  msx,      msx1_v9938_state, empty_init, "Talent", "DPC-200A (Argentina) (MSX1)", 0) // Should have a Spanish keyboard layout?
COMP(1984, hx10,       0,        0,     hx10,       msx,      msx_state, empty_init, "Toshiba", "HX-10AA (Europe) (MSX1)", 0)
COMP(1983, hx10d,      hx10,     0,     hx10d,      msxjp,    msx_state, empty_init, "Toshiba", "HX-10D (Japan) (MSX1)", 0)
COMP(1984, hx10dp,     hx10,     0,     hx10dp,     msxjp,    msx_state, empty_init, "Toshiba", "HX-10DP (Japan) (MSX1)", 0)
COMP(1984, hx10e,      hx10,     0,     hx10e,      msx,      msx_state, empty_init, "Toshiba", "HX-10E (Spain) (MSX1)", 0)
COMP(1984, hx10f,      hx10,     0,     hx10f,      msx,      msx_state, empty_init, "Toshiba", "HX-10F (France) (MSX1)", 0)
COMP(1983, hx10s,      hx10,     0,     hx10s,      msxjp,    msx_state, empty_init, "Toshiba", "HX-10S (Japan) (MSX1)", 0)
COMP(1984, hx10sa,     hx10,     0,     hx10sa,     msxjp,    msx_state, empty_init, "Toshiba", "HX-10SA (Japan) (MSX1)", 0)
COMP(1984, hx20,       0,        0,     hx20,       msxjp,    msx_state, empty_init, "Toshiba", "HX-20 (Japan) (MSX1)", 0)
COMP(1985, hx20e,      hx20,     0,     hx20e,      msx,      msx_state, empty_init, "Toshiba", "HX-20E (Spain) (MSX1)", 0)
COMP(1985, hx20i,      hx20,     0,     hx20i,      msx,      msx_state, empty_init, "Toshiba", "HX-20I (Italy) (MSX1)", 0)
COMP(1984, hx21,       0,        0,     hx21,       msxjp,    msx_state, empty_init, "Toshiba", "HX-21 (Japan) (MSX1)", 0)
COMP(1985, hx21f,      hx21,     0,     hx21f,      msx,      msx_state, empty_init, "Toshiba", "HX-21F (France) (MSX1)", 0)
COMP(1984, hx22,       0,        0,     hx22,       msxjp,    msx_state, empty_init, "Toshiba", "HX-22 (Japan) (MSX1)", 0)
COMP(1985, hx22i,      hx22,     0,     hx22i,      msx,      msx_state, empty_init, "Toshiba", "HX-22I (Italy) (MSX1)", 0)
COMP(1985, hx32,       0,        0,     hx32,       msxjp,    msx_state, empty_init, "Toshiba", "HX-32 (Japan) (MSX1)", 0)
COMP(1985, hx51i,      0,        0,     hx51i,      msx,      msx_state, empty_init, "Toshiba", "HX-51I (Italy, Spain) (MSX1)", 0)
COMP(1983, hc5,        hc7,      0,     hc5,        msxjp,    msx_state, empty_init, "Victor", "HC-5 (Japan) (MSX1)", 0)
COMP(1984, hc6,        hc7,      0,     hc6,        msxjp,    msx_state, empty_init, "Victor", "HC-6 (Japan) (MSX1)", 0)
COMP(1985, hc7,        0,        0,     hc7,        msxjp,    msx_state, empty_init, "Victor", "HC-7 (Japan) (MSX1)", 0)
COMP(1984, cx5f1,      cx5f,     0,     cx5f1,      msxjp,    msx_state, empty_init, "Yamaha", "CX5F (w/SFG01) (Japan) (MSX1)", 0)
COMP(1984, cx5f,       0,        0,     cx5f,       msxjp,    msx_state, empty_init, "Yamaha", "CX5F (w/SFG05) (Japan) (MSX1)", 0)
COMP(1984, cx5mu,      0,        0,     cx5mu,      msx,      msx_state, empty_init, "Yamaha", "CX5MU (USA) (MSX1)", 0)
COMP(1984, cx5m128,    0,        0,     cx5m128,    msx,      msx1_v9938_state, empty_init, "Yamaha", "CX5M-128 (MSX1)", 0)
COMP(1984, cx5miib,    cx5m128,  0,     cx5miib,    msx,      msx1_v9938_state, empty_init, "Yamaha", "CX5MIIB (Italy) (MSX1)", 0)
COMP(1985, sx100,      0,        0,     sx100,      msxjp,    msx_state, empty_init, "Yamaha", "SX-100 (Japan) (MSX1)", 0)
COMP(1984, yis303,     0,        0,     yis303,     msxjp,    msx_state, empty_init, "Yamaha", "YIS303 (Japan) (MSX1)", 0)
COMP(1984, yis503,     0,        0,     yis503,     msxjp,    msx_state, empty_init, "Yamaha", "YIS503 (Japan) (MSX1)", 0)
COMP(1984, yis503f,    yis503,   0,     yis503f,    msxuk,    msx_state, empty_init, "Yamaha", "YIS503F (French) (MSX1)", 0)
COMP(1985, yis503ii,   yis503,   0,     yis503ii,   msxjp,    msx1_v9938_state, empty_init, "Yamaha", "YIS503II (Japan) (MSX1)", 0)
COMP(1985, y503iir,    yis503,   0,     y503iir,    msxru,    msx1_v9938_state, empty_init, "Yamaha", "YIS503IIR (USSR) (MSX1)", 0)
COMP(1986, y503iir2,   yis503,   0,     y503iir2,   y503iir2, msx1_v9938_state, empty_init, "Yamaha", "YIS503IIR (Estonian) (MSX1)", 0)
COMP(1984, yc64,       0,        0,     yc64,       msxuk,    msx_state, empty_init, "Yashica", "YC-64 (Europe) (MSX1)", 0)
COMP(1985, mx64,       0,        0,     mx64,       msxfr,    msx_state, empty_init, "Yeno", "MX64 (France) (MSX1)", 0)

/* MSX2 */
COMP(1985, canonv25,   0,        0,     canonv25,   msxjp,    msx2_state, empty_init, "Canon", "V-25 (Japan) (MSX2)", 0)
COMP(1985, canonv30f,  0,        0,     canonv30f,  msx2,     msx2_state, empty_init, "Canon", "V-30F (Japan) (MSX2)", 0)
COMP(1986, cpc300,     0,        0,     cpc300,     msx2kr,   msx2_state, empty_init, "Daewoo", "IQ-2000 CPC-300 (Korea) (MSX2)", 0)
COMP(1987, cpc300e,    0,        0,     cpc300e,    msx2kr,   msx2_state, empty_init, "Daewoo", "IQ-2000 CPC-300E (Korea) (MSX2)", 0)
COMP(1988, cpc330k,    0,        0,     cpc330k,    msx2kr,   msx2_state, empty_init, "Daewoo", "CPC-330K KOBO (Korea) (MSX2)", 0)
COMP(1987, cpc400,     0,        0,     cpc400,     msx2kr,   msx2_state, empty_init, "Daewoo", "X-II CPC-400 (Korea) (MSX2)", 0)
COMP(1988, cpc400s,    0,        0,     cpc400s,    msx2kr,   msx2_state, empty_init, "Daewoo", "X-II CPC-400S (Korea) (MSX2)", 0)
COMP(1990, cpc61,      0,        0,     cpc61,      msxkr,    msx2_state, empty_init, "Daewoo", "Zemmix CPC-61 (Korea) (MSX2)", 0)
COMP(1991, cpg120,     0,        0,     cpg120,     msx2kr,   msx2_state, empty_init, "Daewoo", "Zemmix CPG-120 Normal (Korea) (MSX2)", 0)
COMP(1986, fpc900,     0,        0,     fpc900,     msx2,     msx2_state, empty_init, "Fenner", "FPC-900 (Italy) (MSX2)", 0)
COMP(1986, expert20,   0,        0,     expert20,   msx2,     msx2_state, empty_init, "Gradiente", "Expert 2.0 (Brazil) (MSX2)", 0)
COMP(1985, mbh3,       0,        0,     mbh3,       msx2jp,   msx2_state, empty_init, "Hitachi", "MB-H3 (Japan) (MSX2)", 0)
COMP(1986, mbh70,      0,        0,     mbh70,      msx2jp,   msx2_state, empty_init, "Hitachi", "MB-H70 (Japan) (MSX2)", MACHINE_NOT_WORKING) // How to enter/use the firmware?
COMP(1987, kmc5000,    0,        0,     kmc5000,    msx2jp,   msx2_state, empty_init, "Kawai", "KMC-5000 (Japan) (MSX2)", 0)
COMP(1986, mlg1,       0,        0,     mlg1,       msx2sp,   msx2_state, empty_init, "Mitsubishi", "ML-G1 (Spain) (MSX2)", 0)
COMP(1986, mlg3,       0,        0,     mlg3,       msx2sp,   msx2_state, empty_init, "Mitsubishi", "ML-G3 (Spain) (MSX2)", 0)
COMP(1985, mlg10,      0,        0,     mlg10,      msx2jp,   msx2_state, empty_init, "Mitsubishi", "ML-G10 (Japan) (MSX2)", 0)
COMP(1985, mlg30,      0,        0,     mlg30,      msx2jp,   msx2_state, empty_init, "Mitsubishi", "ML-G30 Model 1 (Japan) (MSX2)", 0)
COMP(1985, mlg30_2,    0,        0,     mlg30_2,    msx2jp,   msx2_state, empty_init, "Mitsubishi", "ML-G30 Model 2 (Japan) (MSX2)", 0)
COMP(1986, fs4500,     0,        0,     fs4500,     msx2jp,   msx2_state, empty_init, "National", "FS-4500 (Japan) (MSX2)", 0)
COMP(1986, fs4600f,    0,        0,     fs4600f,    msx2jp,   msx2_state, empty_init, "National", "FS-4600F (Japan) (MSX2)", 0)
COMP(1986, fs4700f,    0,        0,     fs4700f,    msx2jp,   msx2_state, empty_init, "National", "FS-4700F (Japan) (MSX2)", 0)
COMP(1986, fs5000f2,   0,        0,     fs5000f2,   msx2jp,   msx2_state, empty_init, "National", "FS-5000F2 (Japan) (MSX2)", 0)
COMP(1985, fs5500f1,   fs5500f2, 0,     fs5500f1,   msx2jp,   msx2_state, empty_init, "National", "FS-5500F1 (Japan) (MSX2)", 0)
COMP(1985, fs5500f2,   0,        0,     fs5500f2,   msx2jp,   msx2_state, empty_init, "National", "FS-5500F2 (Japan) (MSX2)", 0)
COMP(1986, fsa1,       fsa1a,    0,     fsa1,       msxjp,    msx2_state, empty_init, "Panasonic", "FS-A1 / 1st released version (Japan) (MSX2)", 0)
COMP(1986, fsa1a,      0,        0,     fsa1a,      msxjp,    msx2_state, empty_init, "Panasonic", "FS-A1 / 2nd released version (Japan) (MSX2)", 0)
COMP(1987, fsa1mk2,    0,        0,     fsa1mk2,    msx2jp,   msx2_state, empty_init, "Panasonic", "FS-A1MK2 (Japan) (MSX2)", 0)
COMP(1987, fsa1f,      0,        0,     fsa1f,      msx2jp,   msx2_state, empty_init, "Panasonic", "FS-A1F (Japan) (MSX2)", 0)
COMP(1988, fsa1fm,     0,        0,     fsa1fm,     msx2jp,   msx2_state, empty_init, "Panasonic", "FS-A1FM (Japan) (MSX2)", MACHINE_NOT_WORKING) // Modem not emulated, firmware partially working
COMP(1987, nms8220,    0,        0,     nms8220,    msx,      msx2_state, empty_init, "Philips", "NMS 8220 (Europe) (MSX2)", 0)
COMP(1987, nms8245,    0,        0,     nms8245,    msx,      msx2_state, empty_init, "Philips", "NMS 8245 (Europe) (MSX2)", 0)
COMP(1987, nms8245f,   nms8245,  0,     nms8245f,   msxfr,    msx2_state, empty_init, "Philips", "NMS 8245F (France) (MSX2)", 0)
COMP(1987, nms8250,    nms8255,  0,     nms8250,    msx2,     msx2_state, empty_init, "Philips", "NMS 8250 (Europe) (MSX2)", 0)
COMP(1987, nms8250_16, nms8255,  0,     nms8250,    msx2sp,   msx2_state, empty_init, "Philips", "NMS 8250/16 (Spain) (MSX2)", 0)
COMP(1987, nms8250_19, nms8255,  0,     nms8250,    msx2fr,   msx2_state, empty_init, "Philips", "NMS 8250/19 (France) (MSX2)", 0)
COMP(1987, nms8255,    0,        0,     nms8255,    msx2,     msx2_state, empty_init, "Philips", "NMS 8255 (Europe) (MSX2)", 0)
COMP(1987, nms8255f,   nms8255,  0,     nms8255f,   msx2fr,   msx2_state, empty_init, "Philips", "NMS 8255F (France) (MSX2)", 0)
COMP(1987, nms8260,    0,        0,     nms8260,    msx2,     msx2_state, empty_init, "Philips", "NMS 8260 (Prototype) (MSX2)", MACHINE_NOT_WORKING)
COMP(1987, nms8280,    0,        0,     nms8280,    msx2,     msx2_state, empty_init, "Philips", "NMS 8280 (Europe) (MSX2)", 0)
COMP(1986, nms8280f,   nms8280,  0,     nms8280f,   msx2fr,   msx2_state, empty_init, "Philips", "NMS 8280F (France) (MSX2)", 0)
COMP(1986, nms8280g,   nms8280,  0,     nms8280g,   msx2de,   msx2_state, empty_init, "Philips", "NMS 8280G (Germany) (MSX2)", 0)
COMP(1986, vg8230,     0,        0,     vg8230,     msx,      msx2_state, empty_init, "Philips", "VG-8230 (Netherlands) (MSX2)", 0)
COMP(1986, vg8235,     0,        0,     vg8235,     msx,      msx2_state, empty_init, "Philips", "VG-8235 (Europe) (MSX2)", 0)
COMP(1986, vg8235f,    vg8235,   0,     vg8235f,    msxfr,    msx2_state, empty_init, "Philips", "VG-8235F (France) (MSX2)", 0)
COMP(1986, vg8240,     0,        0,     vg8240,     msx,      msx2_state, empty_init, "Philips", "VG-8240 (Prototype) (MSX2)", 0)
COMP(1987, ucv102,     0,        0,     ucv102,     msx2jp,   msx2_state, empty_init, "Pioneer", "UC-V102 (Japan) (MSX2)", 0)
COMP(1987, ax350,      ax350ii,  0,     ax350,      msx,      msx2_state, empty_init, "Sakhr", "AX-350 (Arabic) (MSX2)", 0)
COMP(1987, ax350ii,    0,        0,     ax350ii,    msx,      msx2_state, empty_init, "Sakhr", "AX-350 II (Arabic) (MSX2)", 0)
COMP(1987, ax350iif,   ax350ii,  0,     ax350iif,   msxfr,    msx2_state, empty_init, "Sakhr", "AX-350 II F (Arabic) (MSX2)", 0)
COMP(1988, ax370,      0,        0,     ax370,      msx2,     msx2_state, empty_init, "Sakhr", "AX-370 (Arabic) (MSX2)", 0)
COMP(1987, ax500,      0,        0,     ax500,      msx2,     msx2_state, empty_init, "Sakhr", "AX-500 (Arabic) (MSX2)", 0)
COMP(1987, mpc2300,    0,        0,     mpc2300,    msxru,    msx2_state, empty_init, "Sanyo", "MPC-2300 (USSR) (MSX2)", 0)
COMP(1987, mpc2500f,   0,        0,     mpc2500f,   msx2ru,   msx2_state, empty_init, "Sanyo", "MPC-2500FD (USSR) (MSX2)", 0)
COMP(1985, mpc25fd,    0,        0,     mpc25fd,    msx2jp,   msx2_state, empty_init, "Sanyo", "MPC-25FD (Japan) (MSX2)", 0)
COMP(1985, mpc25fs,    0,        0,     mpc25fs,    msx2jp,   msx2_state, empty_init, "Sanyo", "MPC-25FS (Japan) (MSX2)", 0)
COMP(1985, mpc27,      0,        0,     mpc27,      msx2jp,   msx2_state, empty_init, "Sanyo", "MPC-27 (Japan) (MSX2)", MACHINE_NOT_WORKING) // Light pen not emulated
COMP(1986, phc23,      0,        0,     phc23,      msx2jp,   msx2_state, empty_init, "Sanyo", "PHC-23 / Wavy23 (Japan) (MSX2)", 0)
//COMP(1987, phc23j,     0,        0,     phc23,      msx2jp,   msx2_state, empty_init, "Sanyo", "PHC-23J / Wavy23 (Japan) (MSX2)", 0) // different keyboard layout
COMP(1987, phc23jb,    0,        0,     phc23jb,    msx2jp,   msx2_state, empty_init, "Sanyo", "PHC-23JB / Wavy23 (Japan) (MSX2)", 0)
COMP(1988, phc55fd2,   0,        0,     phc55fd2,   msx2jp,   msx2_state, empty_init, "Sanyo", "PHC-55FD2 / Wavy55FD2 (Japan) (MSX2)", 0)
COMP(1987, phc77,      0,        0,     phc77,      msx2jp,   msx2_state, empty_init, "Sanyo", "PHC-77 / Wavy77 (Japan) (MSX2)", 0)
COMP(1986, hotbit20,   0,        0,     hotbit20,   msx2,     msx2_state, empty_init, "Sharp / Epcom", "HB-8000 Hotbit 2.0 (MSX2)", 0)
COMP(1986, hbf1,       hbf1xd,   0,     hbf1,       msxjp,    msx2_state, empty_init, "Sony", "HB-F1 (Japan) (MSX2)", 0)
COMP(1987, hbf1ii,     hbf1xd,   0,     hbf1ii,     msxjp,    msx2_state, empty_init, "Sony", "HB-F1II (Japan) (MSX2)", 0)
COMP(1987, hbf1xd,     0,        0,     hbf1xd,     msx2jp,   msx2_state, empty_init, "Sony", "HB-F1XD (Japan) (MSX2)", 0)
COMP(1985, hbf5,       0,        0,     hbf5,       msx2jp,   msx2_state, empty_init, "Sony", "HB-F5 (Japan) (MSX2)", 0)
COMP(1986, hbf9p,      0,        0,     hbf9p,      msx2uk,   msx2_state, empty_init, "Sony", "HB-F9P (Europe) (MSX2)", 0)
COMP(19??, hbf9pr,     hbf9p,    0,     hbf9pr,     msx2ru,   msx2_state, empty_init, "Sony", "HB-F9P (Russian) (MSX2, prototype)", 0)
COMP(1986, hbf9s,      hbf9p,    0,     hbf9s,      msx2sp,   msx2_state, empty_init, "Sony", "HB-F9S (Spain) (MSX2)", 0)
COMP(1986, hbf500,     hbf500p,  0,     hbf500,     msx2jp,   msx2_state, empty_init, "Sony", "HB-F500 (Japan) (MSX2)", 0)
COMP(1986, hbf500_2,   hbf500p,  0,     hbf500_2,   msx2jp,   msx2_state, empty_init, "Sony", "HB-F500 2nd version (Japan) (MSX2)", 0)
COMP(1986, hbf500f,    hbf500p,  0,     hbf500f,    msx2fr,   msx2_state, empty_init, "Sony", "HB-F500F (France) (MSX2)", 0)
COMP(1986, hbf500p,    0,        0,     hbf500p,    msx2,     msx2_state, empty_init, "Sony", "HB-F500P (Europe) (MSX2)", 0)
COMP(1986, hbf700d,    hbf700p,  0,     hbf700d,    msx2de,   msx2_state, empty_init, "Sony", "HB-F700D (Germany) (MSX2)", 0)
COMP(1986, hbf700f,    hbf700p,  0,     hbf700f,    msx2fr,   msx2_state, empty_init, "Sony", "HB-F700F (France) (MSX2)", 0)
COMP(1986, hbf700p,    0,        0,     hbf700p,    msx2uk,   msx2_state, empty_init, "Sony", "HB-F700P (Europe) (MSX2)", 0)
COMP(1986, hbf700s,    hbf700p,  0,     hbf700s,    msx2sp,   msx2_state, empty_init, "Sony", "HB-F700S (Spain) (MSX2)", 0)
COMP(1986, hbf900,     hbf900a,  0,     hbf900,     msx2jp,   msx2_state, empty_init, "Sony", "HB-F900 (Japan) (MSX2)", 0)
COMP(1986, hbf900a,    0,        0,     hbf900,     msx2jp,   msx2_state, empty_init, "Sony", "HB-F900 (alt) (Japan) (MSX2)", 0)
COMP(1987, hbg900ap,   hbg900p,  0,     hbg900ap,   msx2uk,   msx2_state, empty_init, "Sony", "HB-G900AP (Europe) (MSX2)", MACHINE_NOT_WORKING) // rs232 not communicating
COMP(1986, hbg900p,    0,        0,     hbg900p,    msx2uk,   msx2_state, empty_init, "Sony", "HB-G900P (Europe) (MSX2)", MACHINE_NOT_WORKING) // rs232 not communicating
COMP(1987, tpc310,     0,        0,     tpc310,     msxsp,    msx2_state, empty_init, "Talent", "TPC-310 (Argentina) (MSX2)", 0)
COMP(1987, tpp311,     0,        0,     tpp311,     msxsp,    msx2_state, empty_init, "Talent", "TPP-311 (Argentina) (MSX2)", 0)
COMP(1987, tps312,     0,        0,     tps312,     msxsp,    msx2_state, empty_init, "Talent", "TPS-312 (Argentina) (MSX2)", 0)
COMP(1985, hx23,       hx23f,    0,     hx23,       msxjp,    msx2_state, empty_init, "Toshiba", "HX-23 (Japan) (MSX2)", MACHINE_NOT_WORKING) // firmware goes into an infinite loop on the title screen
COMP(1985, hx23f,      0,        0,     hx23f,      msxjp,    msx2_state, empty_init, "Toshiba", "HX-23F (Japan) (MSX2)", MACHINE_NOT_WORKING) // firmware goes into an infinite loop on the title screen
COMP(1985, hx33,       hx34,     0,     hx33,       msxjp,    msx2_state, empty_init, "Toshiba", "HX-33 w/HX-R702 (Japan) (MSX2)", MACHINE_NOT_WORKING) // half the pixels are missing in the firmware?
COMP(1985, hx34,       0,        0,     hx34,       msx2jp,   msx2_state, empty_init, "Toshiba", "HX-34 w/HX-R703 (Japan) (MSX2)", 0)
COMP(1986, fstm1,      0,        0,     fstm1,      msx,      msx2_state, empty_init, "Toshiba", "FS-TM1 (Italy) (MSX2)", 0)
COMP(1986, victhc80,   0,        0,     victhc80,   msxjp,    msx2_state, empty_init, "Victor", "HC-80 (Japan) (MSX2)", 0)
COMP(1986, victhc90,   victhc95, 0,     victhc90,   msx2jp,   msx2_state, empty_init, "Victor", "HC-90 (Japan) (MSX2)", MACHINE_NOT_WORKING) // 2nd cpu/turbo not emulated, firmware won't start
COMP(1986, victhc95,   0,        0,     victhc95,   msx2jp,   msx2_state, empty_init, "Victor", "HC-95 (Japan) (MSX2)", MACHINE_NOT_WORKING) // 2nd cpu/turbo not emulated, firmware won't start
COMP(1986, victhc95a,  victhc95, 0,     victhc95a,  msx2jp,   msx2_state, empty_init, "Victor", "HC-95A (Japan) (MSX2)", MACHINE_NOT_WORKING) // 2nd cpu/turbo not emulated, firmware won't start
COMP(1985, cx7128,     cx7m128,  0,     cx7128,     msxjp,    msx2_state, empty_init, "Yamaha", "CX7/128 (Japan) (MSX2)", 0)
COMP(1985, cx7m128,    0,        0,     cx7m128,    msxjp,    msx2_state, empty_init, "Yamaha", "CX7M/128 (Japan) (MSX2)", 0)
COMP(1985, y503iiir,   0,        0,     y503iiir,   msxru,    msx2_state, empty_init, "Yamaha", "YIS-503 III R (USSR) (MSX2)", MACHINE_NOT_WORKING) // network not implemented
COMP(198?, y503iiire,  y503iiir, 0,     y503iiir,   msx2,     msx2_state, empty_init, "Yamaha", "YIS-503 III R (Estonian) (MSX2)", MACHINE_NOT_WORKING) // network not implemented
COMP(1985, yis604,     0,        0,     yis604,     msx2jp,   msx2_state, empty_init, "Yamaha", "YIS604/128 (Japan) (MSX2)", 0)
COMP(1986, y805128,    y805256,  0,     y805128,    msx2jp,   msx2_state, empty_init, "Yamaha", "YIS805/128 (Japan) (MSX2)", MACHINE_NOT_WORKING) // Floppy support broken
COMP(1986, y805128r2,  y805256,  0,     y805128r2,  msx2,     msx2_state, empty_init, "Yamaha", "YIS805/128R2 (USSR) (MSX2)", MACHINE_NOT_WORKING) // Floppy support broken, network not implemented
COMP(198?, y805128r2e, y805256,  0,     y805128r2,  y503iir2, msx2_state, empty_init, "Yamaha", "YIS805/128R2 (Estonian) (MSX2)", MACHINE_NOT_WORKING) // Floppy support broken, network not implemented
COMP(198?, y805256,    0,        0,     y805256,    msx2jp,   msx2_state, empty_init, "Yamaha", "YIS805/256 (Japan) (MSX2)", MACHINE_NOT_WORKING) // Floppy support broken?

/* MSX2+ */
COMP(19??, expert3i,   0,        0,     expert3i,   msx2,     msx2_state, empty_init, "Ciel", "Expert 3 IDE (Brazil) (MSX2+)", MACHINE_NOT_WORKING) // Some hardware not emulated
COMP(1996, expert3t,   0,        0,     expert3t,   msx2,     msx2_state, empty_init, "Ciel", "Expert 3 Turbo (Brazil) (MSX2+)", MACHINE_NOT_WORKING) // Some hardware not emulated
COMP(19??, expertac,   0,        0,     expertac,   msx2,     msx2_state, empty_init, "Gradiente", "Expert AC88+ (Brazil) (MSX2+)", MACHINE_NOT_WORKING) // Some hardware not emulated
COMP(19??, expertdx,   0,        0,     expertdx,   msx2,     msx2_state, empty_init, "Gradiente", "Expert DDX+ (Brazil) (MSX2+)", MACHINE_NOT_WORKING) // Some hardware not emulated
COMP(1988, fsa1fx,     0,        0,     fsa1fx,     msx2jp,   msx2_state, empty_init, "Panasonic", "FS-A1FX (Japan) (MSX2+)", 0)
COMP(1989, fsa1wsx,    0,        0,     fsa1wsx,    msx2jp,   msx2_state, empty_init, "Panasonic", "FS-A1WSX (Japan) (MSX2+)", 0)
COMP(1988, fsa1wx,     fsa1wxa,  0,     fsa1wx,     msx2jp,   msx2_state, empty_init, "Panasonic", "FS-A1WX / 1st released version (Japan) (MSX2+)", 0)
COMP(1988, fsa1wxa,    0,        0,     fsa1wx,     msx2jp,   msx2_state, empty_init, "Panasonic", "FS-A1WX / 2nd released version (Japan) (MSX2+)", 0)
COMP(1988, phc70fd,    phc70fd2, 0,     phc70fd,    msx2jp,   msx2_state, empty_init, "Sanyo", "PHC-70FD / Wavy70FD (Japan) (MSX2+)", 0)
COMP(1989, phc70fd2,   0,        0,     phc70fd2,   msx2jp,   msx2_state, empty_init, "Sanyo", "PHC-70FD2 / Wavy70FD2(Japan) (MSX2+)", 0)
COMP(1989, phc35j,     0,        0,     phc35j,     msx2jp,   msx2_state, empty_init, "Sanyo", "PHC-35J / Wavy35 (Japan) (MSX2+)", 0)
COMP(1988, hbf1xdj,    0,        0,     hbf1xdj,    msx2jp,   msx2_state, empty_init, "Sony", "HB-F1XDJ (Japan) (MSX2+)", 0)
COMP(1989, hbf1xv,     0,        0,     hbf1xv,     msx2jp,   msx2_state, empty_init, "Sony", "HB-F1XV (Japan) (MSX2+)", 0)

/* MSX Turbo-R */
/* Temporary placeholders, Turbo-R hardware is not supported yet */
COMP(1991, fsa1gt,     0,        0,     fsa1gt,     msx2jp,   msx2_state, empty_init, "Panasonic", "FS-A1GT (Japan) (MSX Turbo-R)", MACHINE_NOT_WORKING)
COMP(1991, fsa1st,     0,        0,     fsa1st,     msx2jp,   msx2_state, empty_init, "Panasonic", "FS-A1ST (Japan) (MSX Turbo-R)", MACHINE_NOT_WORKING)
