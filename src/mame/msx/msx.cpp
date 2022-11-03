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
Entries marked with * are missing.

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

#include "cpu/z80/r800.h"
#include "formats/fmsx_cas.h"
#include "hashfile.h"
#include "msx.h"
#include "screen.h"
#include "softlist_dev.h"


//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"


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
		// Ensure that the views will exist
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


void msx2_base_state::msx2_io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x40, 0x4f).rw(FUNC(msx2_base_state::switched_r), FUNC(msx2_base_state::switched_w));
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
	map(0xb4, 0xb4).w(FUNC(msx2_base_state::rtc_latch_w));
	map(0xb5, 0xb5).rw(FUNC(msx2_base_state::rtc_reg_r), FUNC(msx2_base_state::rtc_reg_w));
//	// Sanyo optical pen interface (not emulated)
//	map(0xb8, 0xbb).noprw();
	map(0xd8, 0xd9).w(FUNC(msx2_base_state::kanji_w));
	map(0xd9, 0xd9).r(FUNC(msx2_base_state::kanji_r));
	// 0xfc - 0xff : Memory mapper I/O ports. I/O handlers will be installed if a memory mapper is present in a system
}


void msx2_base_state::msx2plus_io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x40, 0x4f).rw(FUNC(msx2_base_state::switched_r), FUNC(msx2_base_state::switched_w));
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
	map(0xb4, 0xb4).w(FUNC(msx2_base_state::rtc_latch_w));
	map(0xb5, 0xb5).rw(FUNC(msx2_base_state::rtc_reg_r), FUNC(msx2_base_state::rtc_reg_w));
	map(0xd8, 0xd9).w(FUNC(msx2_base_state::kanji_w));
	map(0xd9, 0xd9).r(FUNC(msx2_base_state::kanji_r));
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
		m_view_slot0_page1.select(0);
		m_view_slot0_page2.select(0);
		m_view_slot0_page3.select(0);
	}
}


void msx_state::machine_start()
{
	m_leds.resolve();
	m_port_c_old = 0xff;
}


void msx2_base_state::machine_start()
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

void msx2_base_state::rtc_latch_w(u8 data)
{
	m_rtc_latch = data & 15;
}

void msx2_base_state::rtc_reg_w(u8 data)
{
	m_rtc->write(m_rtc_latch, data);
}

u8 msx2_base_state::rtc_reg_r()
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

u8 msx2_base_state::switched_r(offs_t offset)
{
	u8 data = 0xff;

	for (int i = 0; i < m_switched.size(); i++)
	{
		data &= m_switched[i]->switched_read(offset);
	}

	return data;
}

void msx2_base_state::switched_w(offs_t offset, u8 data)
{
	for (int i = 0; i < m_switched.size(); i++)
	{
		m_switched[i]->switched_write(offset, data);
	}
}

// Some MSX2+ can switch the z80 clock between 3.5 and 5.3 MHz
WRITE_LINE_MEMBER(msx2_base_state::turbo_w)
{
	// 0 - 5.369317 MHz
	// 1 - 3.579545 MHz
	m_maincpu->set_unscaled_clock(21.477272_MHz_XTAL / (state ? 6 : 4));
}

void msx2_base_state::msx_ym2413(machine_config &config)
{
	YM2413(config, "ym2413", 21.477272_MHz_XTAL / 6).add_route(ALL_OUTPUTS, m_speaker, 0.4);
}

void msx2_base_state::msx2_64kb_vram(machine_config &config)
{
	m_v9938->set_vram_size(0x10000);
}

void msx_state::msx_base(ay8910_type ay8910_type, machine_config &config, XTAL xtal, int cpu_divider)
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

	if (ay8910_type == SND_AY8910)
		AY8910(config, m_ay8910, xtal / cpu_divider / 2);
	if (ay8910_type == SND_YM2149)
		YM2149(config, m_ay8910, xtal / cpu_divider / 2);
	m_ay8910->set_flags(AY8910_SINGLE_OUTPUT);
	m_ay8910->port_a_read_callback().set(FUNC(msx2_base_state::psg_port_a_r));
	m_ay8910->port_b_read_callback().set(FUNC(msx2_base_state::psg_port_b_r));
	m_ay8910->port_a_write_callback().set(FUNC(msx2_base_state::psg_port_a_w));
	m_ay8910->port_b_write_callback().set(FUNC(msx2_base_state::psg_port_b_w));
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

void msx_state::msx1_add_softlists(machine_config &config)
{
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

void msx2_base_state::msx2_add_softlists(machine_config &config)
{
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

void msx2_base_state::msx2plus_add_softlists(machine_config &config)
{
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

void msx2_base_state::turbor_add_softlists(machine_config &config)
{
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

void msx_state::msx1(vdp_type vdp_type, ay8910_type ay8910_type, machine_config &config)
{
	msx_base(ay8910_type, config, 10.738635_MHz_XTAL, 3);

	m_maincpu->set_addrmap(AS_IO, &msx_state::msx_io_map);
	m_maincpu->set_vblank_int("screen", FUNC(msx_state::msx_interrupt)); /* Needed for mouse updates */

	if (vdp_type == VDP_TMS9118)
		TMS9118(config, m_tms9928a, 10.738635_MHz_XTAL);
	if (vdp_type == VDP_TMS9128)
		TMS9128(config, m_tms9928a, 10.738635_MHz_XTAL);
	if (vdp_type == VDP_TMS9129)
		TMS9129(config, m_tms9928a, 10.738635_MHz_XTAL);
	if (vdp_type == VDP_TMS9918)
		TMS9918(config, m_tms9928a, 10.738635_MHz_XTAL);
	if (vdp_type == VDP_TMS9918A)
		TMS9918A(config, m_tms9928a, 10.738635_MHz_XTAL);
	if (vdp_type == VDP_TMS9928A)
		TMS9928A(config, m_tms9928a, 10.738635_MHz_XTAL);
	if (vdp_type == VDP_TMS9929A)
		TMS9929A(config, m_tms9928a, 10.738635_MHz_XTAL);
	m_tms9928a->set_screen(m_screen);
	m_tms9928a->set_vram_size(0x4000);
	m_tms9928a->int_callback().set(m_mainirq, FUNC(input_merger_device::in_w<0>));

	// Software lists
	msx1_add_softlists(config);
}

void msx2_base_state::msx2_base(ay8910_type ay8910_type, machine_config &config)
{
	msx_base(ay8910_type, config, 21.477272_MHz_XTAL, 6);

	// real time clock
	RP5C01(config, m_rtc, 32.768_kHz_XTAL);
}

void msx2_base_state::msx2(ay8910_type ay8910_type, machine_config &config)
{
	msx2_base(ay8910_type, config);

	m_maincpu->set_addrmap(AS_IO, &msx2_base_state::msx2_io_map);

	// video hardware
	V9938(config, m_v9938, 21.477272_MHz_XTAL);
	m_v9938->set_screen_ntsc(m_screen);
	m_v9938->set_vram_size(0x20000);
	m_v9938->int_cb().set(m_mainirq, FUNC(input_merger_device::in_w<0>));

	// Software lists
	msx2_add_softlists(config);
}

void msx2_base_state::msx2_pal(ay8910_type ay8910_type, machine_config &config)
{
	msx2(ay8910_type, config);
	m_v9938->set_screen_pal(m_screen);
}

void msx2_base_state::msx2plus_base(ay8910_type ay8910_type, machine_config &config)
{
	msx2_base(ay8910_type, config);

	m_maincpu->set_addrmap(AS_IO, &msx2_base_state::msx2plus_io_map);

	// video hardware
	V9958(config, m_v9958, 21.477272_MHz_XTAL);
	m_v9958->set_screen_ntsc(m_screen);
	m_v9958->set_vram_size(0x20000);
	m_v9958->int_cb().set(m_mainirq, FUNC(input_merger_device::in_w<0>));
}

void msx2_base_state::msx2plus(ay8910_type ay8910_type, machine_config &config)
{
	msx2plus_base(ay8910_type, config);

	// Software lists
	msx2plus_add_softlists(config);
}

void msx2_base_state::msx2plus_pal(ay8910_type ay8910_type, machine_config &config)
{
	msx2plus(ay8910_type, config);
	m_v9958->set_screen_pal(m_screen);
}

void msx2_base_state::turbor(ay8910_type ay8910_type, machine_config &config)
{
	msx2plus_base(ay8910_type, config);

	R800(config.replace(), m_maincpu, 28.636363_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &msx2_base_state::memory_map);
	m_maincpu->set_addrmap(AS_IO, &msx2_base_state::msx2plus_io_map);

	// Software lists
	turbor_add_softlists(config);
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

