/*
** msx.c : driver for MSX
**
** Todo:
** - Add Turbo-R support
** - Add support for other MSX models (de,fr,jp,ru etc.)

This following list is probably incomplete. Corrections are welcome.
+
Al Alamiah AX-170 - MSX1 - ax170
Al Alamiah AX-350 - MSX2 - ax350
Al Alamiah AX-370 - MSX2 - ax370
Canon V-10 - MSX1 - canonv10
Canon V-20 - MSX1 - canonv20
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

Daewoo CPC-300 - MSX2 - cpc300
Daewoo CPC-300E - MSX2 - cpc300e
Daewoo CPC-400 - MSX2 - cpc400
Daewoo CPC-400S - MSX2 cpc400s
Daewoo DPC-100 - MSX1 - dpc100
Daewoo DPC-180 - MSX1 - dpc180
Daewoo DPC-200 - MSX1 - dpc200
Daewoo DPC-200E - MSX1 - See Talent DPC-200
Frael Bruc 100-1 - MSX1 - bruc100
Goldstar FC-200 - MSX1 - gsfc200
Gradiente Expert 1.0 - MSX1 - expert10
Gradiente Expert 1.1 - MSX1 - expert11
Gradiente Expert 1.3 - MSX1 - expert13
Gradiente Expert 2.0 - MSX2 - expert20
Gradiente Expert AC88+ - MSX2+ - expertac
Gradiente Expert DDPlus - MSX1 - expertdp
Gradiente Expert DDX+ - MSX2+ - expertdx
Gradiente Expert Plus - MSX1 - expertpl
JVC HC-7GB - MSX1 -jvchc7gb
Mitsubishi ML-F80 - MSX1 - mlf80
Mitsubishi ML-FX1 - MSX1 - mlfx1
Mitsubishi ML-G30 Model 1 - MSX2 - mlg30
Mitsubishi ML-G30 Model 2 - MSX2 - See Mitsubishi ML-G30 Model 1
National CF-1200 - MSX1 - cf1200
National CF-2000 - MSX1 - cf2000
National CF-2700 - MSX1 - cf2700
National CF-3000 - MSX1 - cf3000
National CF-3300 - MSX1 - cf3300
National FS-1300 - MSX1 - fs1300
National FS-4000 - MSX1 - fs4000
National FS-4500 - MSX2 - fs4500
National FS-4600 - MSX2 - fs4600
National FS-4700 - MSX2 - fs4700
National FS-5000F2 - MSX2 - fs5000
National FS-5500F1 - MSX2 - fs5500
National FS-5500F2 - MSX2 - fs5500
Olympia PHC-2 - MSX1 - phc2
Olympia PHC-28 - MSX1 - phc28
Panasonic CF-2700G - MSX1 - cf2700g
Panasonic FS-A1 - MSX2 - fsa1 / fsa1a
Panasonic FS-A1 MK2 - MSX2 - fsa1mk2
Panasonic FS-A1F - MSX2 - fsa1f
Panasonic FS-A1FM - MSX2 - fsa1fm
Panasonic FS-A1FX - MSX2+ - fsa1fx
Panasonic FS-A1GT - MSX Turbo-R - fsa1gt
Panasonic FS-A1ST - MSX Turbo-R - fsa1st
Panasonic FS-A1WSX - MSX2+ - fsa1wsx
Panasonic FS-A1WX - MSX2+ - fsa1wx / fsa1wxa
Philips NMS-801 - MSX1 - nms801
Philips NMS-8220 - MSX2 - nms8220 / nms8220a
Philips NMS-8245 - MSX2 - nms8245
Philips NMS-8245F - MSX2 - nms8245f
Philips NMS-8250 - MSX2 - nms8250
Philips NMS-8250J - MSX2 - nms8250j
Philips NMS-8255 - MSX2 - nms8255
Philips NMS-8250J - MSX2 - nms8250j
Philips NMS-8280 - MSX2 - nms8280
Philips NMS-8280G - MSX2 - nms8280g
Philips VG-8000 - MSX1 - vg8000
Philips VG-8010 - MSX1 - vg8010
Philips VG-8010F - MSX1 - vg8010f
Philips VG-8020-00 - MSX1 - vg802000
Philips VG-8020-20 - MSX1 - vg802020
Philips VG-8020-40 - MSX1 -
Philips VG-8020F - MSX1 - vg8020f
Philips VG-8230 - MSX2 - vg8230
Philips VG-8230J - MSX2 - vg8230j
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


Samsung SPC-800 MSX1 - spc800
Sanyo MPC-64 - MSX1 - mpc64
Sanyo MPC-100 - MSX1 - mpc100
Sanyo MPC-2300 - MSX2 - mpc2300
Sanyo PHC-28L - MSX1 - phc28l
Sanyo PHC-28S - MSX1 - phc28s
Sanyo Wavy MPC-10 - MSX1 - mpc10
Sanyo Wavy MPC-25FD - MSX2 - mpc25fd
Sanyo Wavy PHC-23 - MSX2 - phc23
Sanyo Wavy PHC-35J - MSX2+ - phc35j
Sanyo Wavy PHC70FD1 - MSX2+ - phc70fd
Sanyo Wavy PHC70FD2 - MSX2+ - phc70fd2
Sharp Epcom HotBit 1.1 - MSX1 - hotbit11
Sharp Epcom HotBit 1.2 - MSX1 - hotbit12
Sharp Epcom HotBit 1.3b - MSX1 - hotbi13b
Sharp Epcom HotBit 1.3p - MSX1 - hotbi13p
Sharp Epcom HotBit 2.0 - MSX2 - hotbit20
Sony HB-10P - MSX1 - hb10p
Sony HB-20P - MSX1 - hb20p
Sony HB-201 - MSX1 - hp201
Sony HB-201P - MSX1 - hp201p
Sony HB-501P - MSX1 - hp501p
Sony HB-55D - MSX1 - hp55d
Sony HB-55P - MSX1 - hp55p
Sony HB-75D - MSX1 - hb75d
Sony HB-75P - MSX1 - hb75p
Sony HB-F1 - MSX2 - hbf1
Sony HB-F1II - MSX2 - hbf12
Sony HB-F1XD - MSX2 - hbf1xd
Sony HB-F1XD MK2 - MSX2 - hbf1xdm2
Sony HB-F1XDJ - MSX2+ - hbf1xdj
Sony HB-F1XV - MSX2+ - hbf1xv
Sony HB-F5 - MSX2 - hbf5
Sony HB-F500 - MSX2 - hbf500
Sony HB-F500P - MSX2 - hbf500p
Sony HB-F700D - MSX2 - hbf700d
Sony HB-F700F - MSX2 - hbf700f
Sony HB-F700P - MSX2 - hbf700p
Sony HB-F700S - MSX2 - hbf700s
Sony HB-F900 - MSX2 - hbf900 / hbf900a
Sony HB-F9P - MSX2 - hbf9p
Sony HB-F9P Russian - MSX2 - hbf9pr
Sony HB-F9S - MSX2 - hbf9s
Sony HB-F9S+ - MSX2+ - hbf9sp
Sony HB-G900AP - MSX2 - hbg900ap
Sony HB-G900P - MSX2 - hbg900p
Sony HB-T7
Spectravideo SVI-728 - MSX1 - svi728
Spectravideo SVI-738 - MSX1 - svi738 / svi738sw / svi738pl
Talent DPC-200 - MSX1 - tadpc200
Talent DPC-200A - MSX1 - tadpc20a
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
   CN1               - Cassette connecter
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


Toshiba HX-10S - MSX1 - hx10s
Toshiba HX-20 - MSX1 - hx20
Toshiba HX-23 - MSX2 - hx23
Toshiba HX-23F - MSX2 - hx23f
Yamaha CX5M - MSX1 - cx5m
Yamaha CX5MII-128 - MSX1 - cx5m128
Yamaha CX5MII - MSX1 - cx5m2
Yamaha CX7M - MSX2 - cx7m
Yamaha CX7M-128 - MSX2 - cx7m128
Yamaha YIS-303 - MSX1 - yis303
Yamaha YIS-503 - MSX1 - yis503
Yamaha YIS-503F - MSX1 - yis503f
Yamaha YIS-503II - MSX1 - yis503ii
Yamaha YIS-503IIR (Russian) - MSX1 - y503iir
Yamaha YIS-503IIR (Estonian) - MSX1 - y503iir2
Yamaha YIS-503M - MSX1 - yis503m
Yashica YC-64 - MSX1 - yc64
Yeno DPC-64 (same bios as Olympia PHC-2)
Yeno MX64 - MSX1 - mx64
=============

PCB Layouts missing


*/


#include "includes/msx.h"

static ADDRESS_MAP_START ( msx_memory_map, AS_PROGRAM, 8, msx_state )
	AM_RANGE( 0x0000, 0x1fff) AM_READ_BANK("bank1") AM_WRITE(msx_page0_w)
	AM_RANGE( 0x2000, 0x3fff) AM_READ_BANK("bank2") AM_WRITE(msx_page0_1_w)
	AM_RANGE( 0x4000, 0x5fff) AM_READ_BANK("bank3") AM_WRITE(msx_page1_w)
	AM_RANGE( 0x6000, 0x7ff7) AM_READ_BANK("bank4") AM_WRITE(msx_page1_1_w)
	AM_RANGE( 0x7ff8, 0x7fff) AM_READ_BANK("bank5") AM_WRITE(msx_page1_2_w)
	AM_RANGE( 0x8000, 0x97ff) AM_READ_BANK("bank6") AM_WRITE(msx_page2_w)
	AM_RANGE( 0x9800, 0x9fff) AM_READ_BANK("bank7") AM_WRITE(msx_page2_1_w)
	AM_RANGE( 0xa000, 0xb7ff) AM_READ_BANK("bank8") AM_WRITE(msx_page2_2_w)
	AM_RANGE( 0xb800, 0xbfff) AM_READ_BANK("bank9") AM_WRITE(msx_page2_3_w)
	AM_RANGE( 0xc000, 0xdfff) AM_READ_BANK("bank10") AM_WRITE(msx_page3_w)
	AM_RANGE( 0xe000, 0xfffe) AM_READ_BANK("bank11") AM_WRITE(msx_page3_1_w)
	AM_RANGE( 0xffff, 0xffff) AM_READWRITE(msx_sec_slot_r, msx_sec_slot_w)
ADDRESS_MAP_END


WRITE8_MEMBER(msx_state::msx_ay8910_w)
{
	device_t *device = machine().device("ay8910");
	if ( offset & 1 )
		ay8910_data_w( device, space, offset, data );
	else
		ay8910_address_w( device, space, offset, data );
}


static ADDRESS_MAP_START ( msx_io_map, AS_IO, 8, msx_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x77, 0x77) AM_WRITE(msx_90in1_w)
	AM_RANGE( 0x7c, 0x7d) AM_WRITE(msx_fmpac_w)
	AM_RANGE( 0x90, 0x90) AM_READWRITE(msx_printer_status_r, msx_printer_strobe_w)
	AM_RANGE( 0x91, 0x91) AM_WRITE(msx_printer_data_w)
	AM_RANGE( 0xa0, 0xa7) AM_DEVREAD_LEGACY("ay8910", ay8910_r) AM_WRITE(msx_ay8910_w)
	AM_RANGE( 0xa8, 0xab) AM_DEVREADWRITE("ppi8255", i8255_device, read, write)
	AM_RANGE( 0x98, 0x98) AM_DEVREADWRITE("tms9928a", tms9928a_device, vram_read, vram_write)
	AM_RANGE( 0x99, 0x99) AM_DEVREADWRITE("tms9928a", tms9928a_device, register_read, register_write)
	AM_RANGE( 0xd8, 0xd9) AM_READWRITE(msx_kanji_r, msx_kanji_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START ( msx2_io_map, AS_IO, 8, msx_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x77, 0x77) AM_WRITE(msx_90in1_w)
	AM_RANGE( 0x7c, 0x7d) AM_WRITE(msx_fmpac_w)
	AM_RANGE( 0x90, 0x90) AM_READWRITE(msx_printer_status_r, msx_printer_strobe_w)
	AM_RANGE( 0x91, 0x91) AM_WRITE(msx_printer_data_w)
	AM_RANGE( 0xa0, 0xa7) AM_DEVREAD_LEGACY("ay8910", ay8910_r) AM_WRITE(msx_ay8910_w)
	AM_RANGE( 0xa8, 0xab) AM_DEVREADWRITE("ppi8255", i8255_device, read, write)
	AM_RANGE( 0x98, 0x9b) AM_DEVREADWRITE("v9938", v9938_device, read, write)
	AM_RANGE( 0xb4, 0xb4) AM_WRITE(msx_rtc_latch_w)
	AM_RANGE( 0xb5, 0xb5) AM_READWRITE(msx_rtc_reg_r, msx_rtc_reg_w)
	AM_RANGE( 0xd8, 0xd9) AM_READWRITE(msx_kanji_r, msx_kanji_w)
	AM_RANGE( 0xfc, 0xff) AM_READWRITE(msx_ram_mapper_r, msx_ram_mapper_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( msx_dips )
	PORT_START("JOY0")
	PORT_BIT (0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)
	PORT_BIT (0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)
	PORT_BIT (0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)
	PORT_BIT (0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT)
	PORT_BIT (0x10, IP_ACTIVE_LOW, IPT_BUTTON1)
	PORT_BIT (0x20, IP_ACTIVE_LOW, IPT_BUTTON2)
	PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_DIPNAME( 0x80, 0, "Game port 1")
	PORT_DIPSETTING(    0x00, DEF_STR( Joystick ))
	PORT_DIPSETTING(    0x80, "Mouse")

	PORT_START("JOY1")
	PORT_BIT (0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)     PORT_PLAYER(2)
	PORT_BIT (0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)     PORT_PLAYER(2)
	PORT_BIT (0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)     PORT_PLAYER(2)
	PORT_BIT (0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT)     PORT_PLAYER(2)
	PORT_BIT (0x10, IP_ACTIVE_LOW, IPT_BUTTON1)     PORT_PLAYER(2)
	PORT_BIT (0x20, IP_ACTIVE_LOW, IPT_BUTTON2)     PORT_PLAYER(2)
	PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_DIPNAME( 0x80, 0, "Game port 2")
	PORT_DIPSETTING( 0x00, DEF_STR( Joystick ))
	PORT_DIPSETTING( 0x80, "Mouse")

	PORT_START("DSW")
	PORT_DIPNAME( 0x40, 0, "Swap game port 1 and 2")
	PORT_DIPSETTING( 0, DEF_STR( No ) )
	PORT_DIPSETTING( 0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0, "SIMPL")
	PORT_DIPSETTING( 0x00, DEF_STR ( Off ) )
	PORT_DIPSETTING( 0x80, DEF_STR ( On ) )
	PORT_DIPNAME ( 0x03, 0, "Render resolution")
	PORT_DIPSETTING( 0, DEF_STR( High ))
	PORT_DIPSETTING( 1, DEF_STR( Low ))
	PORT_DIPSETTING( 2, "Auto" )

	PORT_START("MOUSE0")
	PORT_BIT( 0xff00, 0x00, IPT_TRACKBALL_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(1)
	PORT_BIT( 0x00ff, 0x00, IPT_TRACKBALL_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(1)

	PORT_START("MOUSE1")
	PORT_BIT( 0xff00, 0x00, IPT_TRACKBALL_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(2)
	PORT_BIT( 0x00ff, 0x00, IPT_TRACKBALL_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(2)
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
- "Keypad ," is not mapped
- "Graph" is mapped to 'F6' (this key could be labeled "L Graph")
- "Code" is mapped to 'F7' (this key could be labeled "R Graph", "Kana" or "Hangul")
- "Stop" is mapped to 'F8'
- "Select" is mapped to 'F9'
*/

#define KEYB_ROW0   \
	PORT_BIT (0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')   \
	PORT_BIT (0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')   \
	PORT_BIT (0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')   \
	PORT_BIT (0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')   \
	PORT_BIT (0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')   \
	PORT_BIT (0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')   \
	PORT_BIT (0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^')   \
	PORT_BIT (0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')

#define KEYB_EXPERT11_ROW0   \
	PORT_BIT (0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')   \
	PORT_BIT (0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')   \
	PORT_BIT (0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')   \
	PORT_BIT (0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')   \
	PORT_BIT (0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')   \
	PORT_BIT (0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')   \
	PORT_BIT (0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^')   \
	PORT_BIT (0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')

#define KEYB_HOTBIT_ROW0     \
	PORT_BIT (0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')   \
	PORT_BIT (0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')   \
	PORT_BIT (0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')   \
	PORT_BIT (0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')   \
	PORT_BIT (0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')   \
	PORT_BIT (0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')   \
	PORT_BIT (0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('"')   \
	PORT_BIT (0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')

#define KEYB_ROW1   \
	PORT_BIT (0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)             PORT_CHAR('8') PORT_CHAR('*')   \
	PORT_BIT (0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)             PORT_CHAR('9') PORT_CHAR('(')   \
	PORT_BIT (0x0400, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)         PORT_CHAR('-') PORT_CHAR('_')   \
	PORT_BIT (0x0800, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)        PORT_CHAR('=') PORT_CHAR('+')   \
	PORT_BIT (0x1000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2)    PORT_CHAR('\\') PORT_CHAR('|')  \
	PORT_BIT (0x2000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)     PORT_CHAR('[') PORT_CHAR('{')   \
	PORT_BIT (0x4000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)    PORT_CHAR(']') PORT_CHAR('}')   \
	PORT_BIT (0x8000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)         PORT_CHAR(';') PORT_CHAR(':')

#define KEYB_HOTBIT_ROW1     \
	PORT_BIT (0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)             PORT_CHAR('8') PORT_CHAR('*')   \
	PORT_BIT (0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)             PORT_CHAR('9') PORT_CHAR('(')   \
	PORT_BIT (0x0400, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)         PORT_CHAR('-') PORT_CHAR('_')   \
	PORT_BIT (0x0800, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)        PORT_CHAR('=') PORT_CHAR('+')   \
	PORT_BIT (0x1000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2)    PORT_CHAR('\\') PORT_CHAR('^')  \
	PORT_BIT (0x2000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)     PORT_CHAR('\'') PORT_CHAR('`')  \
	PORT_BIT (0x4000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)    PORT_CHAR('"') PORT_CHAR('`')   \
	PORT_BIT (0x8000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)         PORT_CHAR('\xC7') PORT_CHAR('\xE7')

#define KEYB_EXPERT11_ROW1   \
	PORT_BIT (0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)             PORT_CHAR('8') PORT_CHAR('\'')  \
	PORT_BIT (0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)             PORT_CHAR('9') PORT_CHAR('(')   \
	PORT_BIT (0x0400, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)         PORT_CHAR('-') PORT_CHAR('_')   \
	PORT_BIT (0x0800, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)        PORT_CHAR('=') PORT_CHAR('+')   \
	PORT_BIT (0x1000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2)    PORT_CHAR('{') PORT_CHAR('}')   \
	PORT_BIT (0x2000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)     PORT_CHAR('\'') PORT_CHAR('`')  \
	PORT_BIT (0x4000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)    PORT_CHAR('[') PORT_CHAR(']')   \
	PORT_BIT (0x8000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)         PORT_CHAR('~') PORT_CHAR('^')

#define KEYB_ROW2   \
	PORT_BIT (0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)     PORT_CHAR('\'') PORT_CHAR('"')  \
	PORT_BIT (0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('`') PORT_CHAR('~')   \
	PORT_BIT (0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)     PORT_CHAR(',') PORT_CHAR('<')   \
	PORT_BIT (0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)      PORT_CHAR('.') PORT_CHAR('>')   \
	PORT_BIT (0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)     PORT_CHAR('/') PORT_CHAR('?')   \
	PORT_BIT (0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Dead Key") PORT_CODE(KEYCODE_TILDE)               \
	PORT_BIT (0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)         PORT_CHAR('a') PORT_CHAR('A')   \
	PORT_BIT (0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)         PORT_CHAR('b') PORT_CHAR('B')

#define KEYB_HOTBIT_ROW2     \
	PORT_BIT (0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)         PORT_CHAR('~') PORT_CHAR('^')   \
	PORT_BIT (0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)     PORT_CHAR('[') PORT_CHAR(']')   \
	PORT_BIT (0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)         PORT_CHAR(',') PORT_CHAR(';')   \
	PORT_BIT (0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)          PORT_CHAR('.') PORT_CHAR(':')   \
	PORT_BIT (0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)         PORT_CHAR('/') PORT_CHAR('?')   \
	PORT_BIT (0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)         PORT_CHAR('<') PORT_CHAR('>')   \
	PORT_BIT (0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)             PORT_CHAR('a') PORT_CHAR('A')   \
	PORT_BIT (0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)             PORT_CHAR('b') PORT_CHAR('B')

#define KEYB_EXPERT10_ROW2   \
	PORT_BIT (0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)     PORT_CHAR('\'') PORT_CHAR('"')  \
	PORT_BIT (0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\xC7') PORT_CHAR('\xE7') \
	PORT_BIT (0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)     PORT_CHAR(',') PORT_CHAR('<')   \
	PORT_BIT (0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)      PORT_CHAR('.') PORT_CHAR('>')   \
	PORT_BIT (0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)     PORT_CHAR('/') PORT_CHAR('?')   \
	PORT_BIT (0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Dead Key") PORT_CODE(KEYCODE_TILDE)               \
	PORT_BIT (0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)         PORT_CHAR('a') PORT_CHAR('A')   \
	PORT_BIT (0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)         PORT_CHAR('b') PORT_CHAR('B')

#define KEYB_EXPERT11_ROW2   \
	PORT_BIT (0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('*') PORT_CHAR('@')   \
	PORT_BIT (0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)     PORT_CHAR('\xC7') PORT_CHAR('\xE7') \
	PORT_BIT (0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)     PORT_CHAR(',') PORT_CHAR('<')   \
	PORT_BIT (0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)      PORT_CHAR('.') PORT_CHAR('>')   \
	PORT_BIT (0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)     PORT_CHAR('/') PORT_CHAR('?')   \
	PORT_BIT (0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)     PORT_CHAR(';') PORT_CHAR(':')   \
	PORT_BIT (0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)         PORT_CHAR('a') PORT_CHAR('A')   \
	PORT_BIT (0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)         PORT_CHAR('b') PORT_CHAR('B')

#define KEYB_ROW3   \
	PORT_BIT (0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')   \
	PORT_BIT (0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')   \
	PORT_BIT (0x0400, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')   \
	PORT_BIT (0x0800, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')   \
	PORT_BIT (0x1000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')   \
	PORT_BIT (0x2000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')   \
	PORT_BIT (0x4000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')   \
	PORT_BIT (0x8000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')

#define KEYB_ROW4   \
	PORT_BIT (0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')   \
	PORT_BIT (0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')   \
	PORT_BIT (0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')   \
	PORT_BIT (0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')   \
	PORT_BIT (0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')   \
	PORT_BIT (0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')   \
	PORT_BIT (0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')   \
	PORT_BIT (0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')

#define KEYB_ROW5   \
	PORT_BIT (0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')   \
	PORT_BIT (0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')   \
	PORT_BIT (0x0400, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')   \
	PORT_BIT (0x0800, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')   \
	PORT_BIT (0x1000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')   \
	PORT_BIT (0x2000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')   \
	PORT_BIT (0x4000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')   \
	PORT_BIT (0x8000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')

#define KEYB_ROW6   \
	PORT_BIT (0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)                        PORT_CHAR(UCHAR_SHIFT_1)            \
	PORT_BIT (0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)                      PORT_CHAR(UCHAR_SHIFT_2)            \
	PORT_BIT (0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("GRAPH") PORT_CODE(KEYCODE_PGUP)       PORT_CHAR(UCHAR_MAMEKEY(F6))        \
	PORT_BIT (0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CAPS") PORT_CODE(KEYCODE_CAPSLOCK)    PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))  \
	PORT_BIT (0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CODE") PORT_CODE(KEYCODE_PGDN)        PORT_CHAR(UCHAR_MAMEKEY(F7))        \
	PORT_BIT (0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1  F6") PORT_CODE(KEYCODE_F1)        PORT_CHAR(UCHAR_MAMEKEY(F1))        \
	PORT_BIT (0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2  F7") PORT_CODE(KEYCODE_F2)        PORT_CHAR(UCHAR_MAMEKEY(F2))        \
	PORT_BIT (0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3  F8") PORT_CODE(KEYCODE_F3)        PORT_CHAR(UCHAR_MAMEKEY(F3))

#define KEYB_EXPERT11_ROW6   \
	PORT_BIT (0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)                            PORT_CHAR(UCHAR_SHIFT_1)            \
	PORT_BIT (0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CONTROL") PORT_CODE(KEYCODE_LCONTROL)     PORT_CHAR(UCHAR_SHIFT_2)            \
	PORT_BIT (0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L GRA") PORT_CODE(KEYCODE_PGUP)           PORT_CHAR(UCHAR_MAMEKEY(F6))        \
	PORT_BIT (0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CAPS LOCK") PORT_CODE(KEYCODE_CAPSLOCK)   PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))  \
	PORT_BIT (0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R GRA") PORT_CODE(KEYCODE_PGDN)           PORT_CHAR(UCHAR_MAMEKEY(F7))        \
	PORT_BIT (0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1  F6") PORT_CODE(KEYCODE_F1)            PORT_CHAR(UCHAR_MAMEKEY(F1))        \
	PORT_BIT (0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2  F7") PORT_CODE(KEYCODE_F2)            PORT_CHAR(UCHAR_MAMEKEY(F2))        \
	PORT_BIT (0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3  F8") PORT_CODE(KEYCODE_F3)            PORT_CHAR(UCHAR_MAMEKEY(F3))

#define KEYB_ROW7   \
	PORT_BIT (0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F4  F9") PORT_CODE(KEYCODE_F4)        PORT_CHAR(UCHAR_MAMEKEY(F4))    \
	PORT_BIT (0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F5  F10") PORT_CODE(KEYCODE_F5)       PORT_CHAR(UCHAR_MAMEKEY(F5))    \
	PORT_BIT (0x0400, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)                           PORT_CHAR(UCHAR_MAMEKEY(ESC))   \
	PORT_BIT (0x0800, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)                           PORT_CHAR('\t')                 \
	PORT_BIT (0x1000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("STOP") PORT_CODE(KEYCODE_RCONTROL)    PORT_CHAR(UCHAR_MAMEKEY(F8))    \
	PORT_BIT (0x2000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)                     PORT_CHAR(8)                    \
	PORT_BIT (0x4000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SELECT") PORT_CODE(KEYCODE_END)       PORT_CHAR(UCHAR_MAMEKEY(F9))    \
	PORT_BIT (0x8000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)                         PORT_CHAR(13)

#define KEYB_ROW8   \
	PORT_BIT (0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)                 PORT_CHAR(' ')                  \
	PORT_BIT (0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME)                  PORT_CHAR(UCHAR_MAMEKEY(HOME))  \
	PORT_BIT (0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_INSERT)                PORT_CHAR(UCHAR_MAMEKEY(INSERT))\
	PORT_BIT (0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DEL") PORT_CODE(KEYCODE_DEL)  PORT_CHAR(UCHAR_MAMEKEY(DEL))   \
	PORT_BIT (0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)                  PORT_CHAR(UCHAR_MAMEKEY(LEFT))  \
	PORT_BIT (0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)                    PORT_CHAR(UCHAR_MAMEKEY(UP))    \
	PORT_BIT (0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)                  PORT_CHAR(UCHAR_MAMEKEY(DOWN))  \
	PORT_BIT (0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)                 PORT_CHAR(UCHAR_MAMEKEY(RIGHT))

#define KEYB_ROW9   \
	PORT_BIT (0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK)  PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))  \
	PORT_BIT (0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD)  PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))  \
	PORT_BIT (0x0400, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD) PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD)) \
	PORT_BIT (0x0800, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)     PORT_CHAR(UCHAR_MAMEKEY(0_PAD))     \
	PORT_BIT (0x1000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)     PORT_CHAR(UCHAR_MAMEKEY(1_PAD))     \
	PORT_BIT (0x2000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)     PORT_CHAR(UCHAR_MAMEKEY(2_PAD))     \
	PORT_BIT (0x4000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)     PORT_CHAR(UCHAR_MAMEKEY(3_PAD))     \
	PORT_BIT (0x8000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)     PORT_CHAR(UCHAR_MAMEKEY(4_PAD))

#define KEYB_ROW10  \
	PORT_BIT (0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)     PORT_CHAR(UCHAR_MAMEKEY(5_PAD))     \
	PORT_BIT (0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)     PORT_CHAR(UCHAR_MAMEKEY(6_PAD))     \
	PORT_BIT (0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)     PORT_CHAR(UCHAR_MAMEKEY(7_PAD))     \
	PORT_BIT (0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)     PORT_CHAR(UCHAR_MAMEKEY(8_PAD))     \
	PORT_BIT (0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)     PORT_CHAR(UCHAR_MAMEKEY(9_PAD))     \
	PORT_BIT (0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD)) \
	PORT_BIT (0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad ,") PORT_CODE(KEYCODE_ENTER_PAD)               \
	PORT_BIT (0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD)   PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))

static INPUT_PORTS_START( msx )
	PORT_START("KEY0")
	KEYB_ROW0
	KEYB_ROW1

	PORT_START("KEY1")
	KEYB_ROW2
	KEYB_ROW3

	PORT_START("KEY2")
	KEYB_ROW4
	KEYB_ROW5

	PORT_START("KEY3")
	KEYB_ROW6
	KEYB_ROW7

	PORT_START("KEY4")
	KEYB_ROW8
	PORT_BIT (0xff00, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY5")
	PORT_BIT (0xffff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_INCLUDE( msx_dips )
INPUT_PORTS_END

#ifdef UNREFERENCED_CODE
static INPUT_PORTS_START( msxuk )
	PORT_START("KEY0")
	KEYB_ROW0
	KEYB_ROW1

	PORT_START("KEY1")
	PORT_BIT (0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)         PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT (0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)     PORT_CHAR('\xA3') PORT_CHAR('~')
	PORT_BIT (0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)         PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT (0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)          PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT (0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)         PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT (0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Dead Key") PORT_CODE(KEYCODE_TILDE)
	PORT_BIT (0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)             PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT (0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)             PORT_CHAR('b') PORT_CHAR('B')
	KEYB_ROW3

	PORT_START("KEY2")
	KEYB_ROW4
	KEYB_ROW5

	PORT_START("KEY3")
	KEYB_ROW6
	KEYB_ROW7

	PORT_START("KEY4")
	KEYB_ROW8
	PORT_BIT (0xff00, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY5")
	PORT_BIT (0xffff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_INCLUDE( msx_dips )
INPUT_PORTS_END
#endif

#define KEYB_JAP_ROW0   \
	PORT_BIT (0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0')                  \
	PORT_BIT (0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')   \
	PORT_BIT (0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')   \
	PORT_BIT (0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')   \
	PORT_BIT (0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')   \
	PORT_BIT (0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')   \
	PORT_BIT (0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')   \
	PORT_BIT (0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')

#define KEYB_JAP_ROW1   \
	PORT_BIT (0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)             PORT_CHAR('8') PORT_CHAR('(')   \
	PORT_BIT (0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)             PORT_CHAR('9') PORT_CHAR(')')   \
	PORT_BIT (0x0400, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)         PORT_CHAR('-') PORT_CHAR('=')   \
	PORT_BIT (0x0800, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)        PORT_CHAR('^') PORT_CHAR('~')   \
	PORT_BIT (0x1000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2)    PORT_CHAR('\xA5') PORT_CHAR('|')\
	PORT_BIT (0x2000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)     PORT_CHAR('@') PORT_CHAR('`')   \
	PORT_BIT (0x4000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)    PORT_CHAR('[') PORT_CHAR('{')   \
	PORT_BIT (0x8000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)         PORT_CHAR(';') PORT_CHAR('+')

#define KEYB_KOR_ROW1   \
	PORT_BIT (0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)             PORT_CHAR('8') PORT_CHAR('(')   \
	PORT_BIT (0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)             PORT_CHAR('9') PORT_CHAR(')')   \
	PORT_BIT (0x0400, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)         PORT_CHAR('-') PORT_CHAR('=')   \
	PORT_BIT (0x0800, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)        PORT_CHAR('^') PORT_CHAR('~')   \
	PORT_BIT (0x1000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\xE2\x82\xA9 |") PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR(0xffe6) PORT_CHAR('|') \
	PORT_BIT (0x2000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)     PORT_CHAR('@') PORT_CHAR('`')   \
	PORT_BIT (0x4000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)    PORT_CHAR('[') PORT_CHAR('{')   \
	PORT_BIT (0x8000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)         PORT_CHAR(';') PORT_CHAR('+')

#define KEYB_JAP_ROW2   \
	PORT_BIT (0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)     PORT_CHAR(':') PORT_CHAR('*')   \
	PORT_BIT (0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR(']') PORT_CHAR('}')   \
	PORT_BIT (0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)     PORT_CHAR(',') PORT_CHAR('<')   \
	PORT_BIT (0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)      PORT_CHAR('.') PORT_CHAR('>')   \
	PORT_BIT (0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)     PORT_CHAR('/') PORT_CHAR('?')   \
	PORT_BIT (0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("_") PORT_CODE(KEYCODE_TILDE) PORT_CHAR('_')       \
	PORT_BIT (0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)         PORT_CHAR('a') PORT_CHAR('A')   \
	PORT_BIT (0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)         PORT_CHAR('b') PORT_CHAR('B')

static INPUT_PORTS_START( msxjp )
	PORT_START("KEY0")
	KEYB_JAP_ROW0
	KEYB_JAP_ROW1

	PORT_START("KEY1")
	KEYB_JAP_ROW2
	KEYB_ROW3

	PORT_START("KEY2")
	KEYB_ROW4
	KEYB_ROW5

	PORT_START("KEY3")
	PORT_BIT (0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)                        PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT (0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)                      PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT (0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("GRAPH") PORT_CODE(KEYCODE_PGUP)       PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT (0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CAPS") PORT_CODE(KEYCODE_CAPSLOCK)    PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT (0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KANA") PORT_CODE(KEYCODE_PGDN)        PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT (0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1  F6") PORT_CODE(KEYCODE_F1)        PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT (0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2  F7") PORT_CODE(KEYCODE_F2)        PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT (0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3  F8") PORT_CODE(KEYCODE_F3)        PORT_CHAR(UCHAR_MAMEKEY(F3))
	KEYB_ROW7

	PORT_START("KEY4")
	KEYB_ROW8
	PORT_BIT (0xff00, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY5")
	PORT_BIT (0xffff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_INCLUDE( msx_dips )
INPUT_PORTS_END

static INPUT_PORTS_START( msxkr )
	PORT_START("KEY0")
	KEYB_JAP_ROW0
	KEYB_KOR_ROW1

	PORT_START("KEY1")
	KEYB_JAP_ROW2
	KEYB_ROW3

	PORT_START("KEY2")
	KEYB_ROW4
	KEYB_ROW5

	PORT_START("KEY3")
	PORT_BIT (0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)                        PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT (0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)                      PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT (0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("GRAPH") PORT_CODE(KEYCODE_PGUP)       PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT (0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CAPS") PORT_CODE(KEYCODE_CAPSLOCK)    PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT (0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Hangul") PORT_CODE(KEYCODE_PGDN)      PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT (0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1  F6") PORT_CODE(KEYCODE_F1)        PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT (0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2  F7") PORT_CODE(KEYCODE_F2)        PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT (0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3  F8") PORT_CODE(KEYCODE_F3)        PORT_CHAR(UCHAR_MAMEKEY(F3))
	KEYB_ROW7

	PORT_START("KEY4")
	KEYB_ROW8
	PORT_BIT (0xff00, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY5")
	PORT_BIT (0xffff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_INCLUDE( msx_dips )
INPUT_PORTS_END

static INPUT_PORTS_START( hotbit )
	PORT_START("KEY0")
	KEYB_HOTBIT_ROW0
	KEYB_HOTBIT_ROW1

	PORT_START("KEY1")
	KEYB_HOTBIT_ROW2
	KEYB_ROW3

	PORT_START("KEY2")
	KEYB_ROW4
	KEYB_ROW5

	PORT_START("KEY3")
	KEYB_ROW6
	KEYB_ROW7

	PORT_START("KEY4")
	KEYB_ROW8
	PORT_BIT (0xff00, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY5")
	PORT_BIT (0xffff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_INCLUDE( msx_dips )
INPUT_PORTS_END

/* 2008-05 FP: I guess these belong to the keypad */
#define KEYB_EXPERT11_ROW9  \
	PORT_BIT (0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD)      PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))  \
	PORT_BIT (0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD)     PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD)) \
	PORT_BIT (0x0400, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK)      PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))  \
	PORT_BIT (0x0800, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD)     PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD)) \
	PORT_BIT (0xf000, IP_ACTIVE_LOW, IPT_UNUSED)    \

static INPUT_PORTS_START( expert11 )
	PORT_START("KEY0")
	KEYB_EXPERT11_ROW0
	KEYB_EXPERT11_ROW1

	PORT_START("KEY1")
	KEYB_EXPERT11_ROW2
	KEYB_ROW3

	PORT_START("KEY2")
	KEYB_ROW4
	KEYB_ROW5

	PORT_START("KEY3")
	KEYB_EXPERT11_ROW6
	KEYB_ROW7

	PORT_START("KEY4")
	KEYB_ROW8
	KEYB_EXPERT11_ROW9

	PORT_START("KEY5")
	PORT_BIT (0xffff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_INCLUDE( msx_dips )
INPUT_PORTS_END

static INPUT_PORTS_START( expert10 )
	PORT_START("KEY0")
	KEYB_ROW0
	KEYB_ROW1

	PORT_START("KEY1")
	KEYB_EXPERT10_ROW2
	KEYB_ROW3

	PORT_START("KEY2")
	KEYB_ROW4
	KEYB_ROW5

	PORT_START("KEY3")
	KEYB_EXPERT11_ROW6
	KEYB_ROW7

	PORT_START("KEY4")
	KEYB_ROW8
	KEYB_EXPERT11_ROW9

	PORT_START("KEY5")
	PORT_BIT (0xffff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_INCLUDE( msx_dips )
INPUT_PORTS_END

static INPUT_PORTS_START( msx2 )
	PORT_START("KEY0")
	KEYB_ROW0
	KEYB_ROW1

	PORT_START("KEY1")
	KEYB_ROW2
	KEYB_ROW3

	PORT_START("KEY2")
	KEYB_ROW4
	KEYB_ROW5

	PORT_START("KEY3")
	KEYB_ROW6
	KEYB_ROW7

	PORT_START("KEY4")
	KEYB_ROW8
	KEYB_ROW9

	PORT_START("KEY5")
	KEYB_ROW10

	PORT_INCLUDE( msx_dips )
INPUT_PORTS_END

static INPUT_PORTS_START( msx2jp )
	PORT_START("KEY0")
	KEYB_JAP_ROW0
	KEYB_JAP_ROW1

	PORT_START("KEY1")
	KEYB_JAP_ROW2
	KEYB_ROW3

	PORT_START("KEY2")
	KEYB_ROW4
	KEYB_ROW5

	PORT_START("KEY3")
	PORT_BIT (0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)                        PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT (0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)                      PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT (0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("GRAPH") PORT_CODE(KEYCODE_PGUP)       PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT (0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CAPS") PORT_CODE(KEYCODE_CAPSLOCK)    PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT (0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("KANA") PORT_CODE(KEYCODE_PGDN)        PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT (0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1  F6") PORT_CODE(KEYCODE_F1)        PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT (0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2  F7") PORT_CODE(KEYCODE_F2)        PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT (0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3  F8") PORT_CODE(KEYCODE_F3)        PORT_CHAR(UCHAR_MAMEKEY(F3))
	KEYB_ROW7

	PORT_START("KEY4")
	KEYB_ROW8
	KEYB_ROW9

	PORT_START("KEY5")
	KEYB_ROW10

	PORT_INCLUDE( msx_dips )
INPUT_PORTS_END

static INPUT_PORTS_START( msx2kr )
	PORT_START("KEY0")
	KEYB_JAP_ROW0
	KEYB_KOR_ROW1

	PORT_START("KEY1")
	KEYB_JAP_ROW2
	KEYB_ROW3

	PORT_START("KEY2")
	KEYB_ROW4
	KEYB_ROW5

	PORT_START("KEY3")
	PORT_BIT (0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)                        PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT (0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)                      PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT (0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("GRAPH") PORT_CODE(KEYCODE_PGUP)       PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT (0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CAPS") PORT_CODE(KEYCODE_CAPSLOCK)    PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT (0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Hangul") PORT_CODE(KEYCODE_PGDN)      PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT (0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1  F6") PORT_CODE(KEYCODE_F1)        PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT (0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2  F7") PORT_CODE(KEYCODE_F2)        PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT (0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3  F8") PORT_CODE(KEYCODE_F3)        PORT_CHAR(UCHAR_MAMEKEY(F3))
	KEYB_ROW7

	PORT_START("KEY4")
	KEYB_ROW8
	PORT_BIT (0xff00, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY5")
	PORT_BIT (0xffff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_INCLUDE( msx_dips )
INPUT_PORTS_END

static const ay8910_interface msx_ay8910_interface =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_DRIVER_MEMBER(msx_state, msx_psg_port_a_r),
	DEVCB_DRIVER_MEMBER(msx_state, msx_psg_port_b_r),
	DEVCB_DRIVER_MEMBER(msx_state, msx_psg_port_a_w),
	DEVCB_DRIVER_MEMBER(msx_state, msx_psg_port_b_w)
};

#define MSX_XBORDER_PIXELS      15
#define MSX_YBORDER_PIXELS      27
#define MSX_TOTAL_XRES_PIXELS       256 + (MSX_XBORDER_PIXELS * 2)
#define MSX_TOTAL_YRES_PIXELS       192 + (MSX_YBORDER_PIXELS * 2)
#define MSX_VISIBLE_XBORDER_PIXELS  8
#define MSX_VISIBLE_YBORDER_PIXELS  24

static const cassette_interface msx_cassette_interface =
{
	fmsx_cassette_formats,
	NULL,
	(cassette_state)(CASSETTE_PLAY),
	NULL,
	NULL
};

static const floppy_interface msx_floppy_interface =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	FLOPPY_STANDARD_5_25_DSHD,
	LEGACY_FLOPPY_OPTIONS_NAME(msx),
	NULL,
	NULL
};

static MACHINE_CONFIG_FRAGMENT( msx_cartslot )
	MCFG_CARTSLOT_ADD("cart1")
	MCFG_CARTSLOT_EXTENSION_LIST("mx1,rom")
	MCFG_CARTSLOT_NOT_MANDATORY
	MCFG_CARTSLOT_INTERFACE("msx_cart")
	MCFG_CARTSLOT_LOAD(msx_cart)
	MCFG_CARTSLOT_UNLOAD(msx_cart)

	MCFG_CARTSLOT_ADD("cart2")
	MCFG_CARTSLOT_EXTENSION_LIST("mx1,rom")
	MCFG_CARTSLOT_NOT_MANDATORY
	MCFG_CARTSLOT_INTERFACE("msx_cart")
	MCFG_CARTSLOT_LOAD(msx_cart)
	MCFG_CARTSLOT_UNLOAD(msx_cart)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( msx, msx_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_10_738635MHz/3)         /* 3.579545 MHz */
	MCFG_CPU_PROGRAM_MAP(msx_memory_map)
	MCFG_CPU_IO_MAP(msx_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", msx_state,  msx_interrupt) /* Needed for mouse updates */
	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_MACHINE_START_OVERRIDE(msx_state, msx )
	MCFG_MACHINE_RESET_OVERRIDE(msx_state, msx )

	MCFG_I8255_ADD( "ppi8255", msx_ppi8255_interface )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, CASSETTE_TAG)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
	MCFG_SOUND_ADD("ay8910", AY8910, XTAL_10_738635MHz/3/2)
	MCFG_SOUND_CONFIG(msx_ay8910_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)
	MCFG_SOUND_ADD("k051649", K051649, XTAL_10_738635MHz/3/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
	MCFG_SOUND_ADD("ym2413", YM2413, XTAL_10_738635MHz/3)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)

	/* printer */
	MCFG_CENTRONICS_PRINTER_ADD("centronics", standard_centronics)

	MCFG_CASSETTE_ADD( CASSETTE_TAG, msx_cassette_interface )

	MCFG_FD1793_ADD("wd179x", msx_wd17xx_interface ) // TODO confirm type

	MCFG_LEGACY_FLOPPY_2_DRIVES_ADD(msx_floppy_interface)

	MCFG_FRAGMENT_ADD(msx_cartslot)

	/* Software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list","msx1_cart")
MACHINE_CONFIG_END


static TMS9928A_INTERFACE(msx_tms9928a_interface)
{
	"screen",
	0x4000,
	DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_IRQ0)
};


static MACHINE_CONFIG_DERIVED( msx_ntsc, msx )
	/* Video hardware */
	MCFG_TMS9928A_ADD( "tms9928a", TMS9928A, msx_tms9928a_interface )
	MCFG_TMS9928A_SCREEN_ADD_NTSC( "screen" )
	MCFG_SCREEN_UPDATE_DEVICE("tms9928a", tms9928a_device, screen_update)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( msx_pal, msx )
	/* Video hardware */
	MCFG_TMS9928A_ADD( "tms9928a", TMS9929A, msx_tms9928a_interface )
	MCFG_TMS9928A_SCREEN_ADD_PAL( "screen" )
	MCFG_SCREEN_UPDATE_DEVICE("tms9928a", tms9928a_device, screen_update)
MACHINE_CONFIG_END


#define MSX2_XBORDER_PIXELS     16
#define MSX2_YBORDER_PIXELS     28
#define MSX2_TOTAL_XRES_PIXELS      256 * 2 + (MSX2_XBORDER_PIXELS * 2)
#define MSX2_TOTAL_YRES_PIXELS      212 * 2 + (MSX2_YBORDER_PIXELS * 2)
#define MSX2_VISIBLE_XBORDER_PIXELS 8 * 2
#define MSX2_VISIBLE_YBORDER_PIXELS 14 * 2

static RP5C01_INTERFACE( rtc_intf )
{
	DEVCB_NULL
};

static MACHINE_CONFIG_START( msx2, msx_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_21_4772MHz/6)       /* 3.579545 MHz */
	MCFG_CPU_PROGRAM_MAP(msx_memory_map)
	MCFG_CPU_IO_MAP(msx2_io_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", msx_state, msx2_interrupt, "screen", 0, 2)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_MACHINE_START_OVERRIDE(msx_state, msx2 )
	MCFG_MACHINE_RESET_OVERRIDE(msx_state, msx2 )

	MCFG_I8255_ADD( "ppi8255", msx_ppi8255_interface )

	/* video hardware */
	MCFG_V9938_ADD("v9938", "screen", 0x20000)
	MCFG_V99X8_INTERRUPT_CALLBACK_STATIC(msx_vdp_interrupt)

	MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DEVICE("v9938", v9938_device, screen_update)
	MCFG_SCREEN_SIZE(MSX2_TOTAL_XRES_PIXELS, 262*2)
	MCFG_SCREEN_VISIBLE_AREA(MSX2_XBORDER_PIXELS - MSX2_VISIBLE_XBORDER_PIXELS, MSX2_TOTAL_XRES_PIXELS - MSX2_XBORDER_PIXELS + MSX2_VISIBLE_XBORDER_PIXELS - 1, MSX2_YBORDER_PIXELS - MSX2_VISIBLE_YBORDER_PIXELS, MSX2_TOTAL_YRES_PIXELS - MSX2_YBORDER_PIXELS + MSX2_VISIBLE_YBORDER_PIXELS - 1)

	MCFG_PALETTE_LENGTH(512)
	MCFG_PALETTE_INIT(v9938)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, CASSETTE_TAG)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
	MCFG_SOUND_ADD("ay8910", AY8910, XTAL_21_4772MHz/6/2)
	MCFG_SOUND_CONFIG(msx_ay8910_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)
	MCFG_SOUND_ADD("k051649", K051649, XTAL_21_4772MHz/6/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
	MCFG_SOUND_ADD("ym2413", YM2413, XTAL_21_4772MHz/6)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)

	/* printer */
	MCFG_CENTRONICS_PRINTER_ADD("centronics", standard_centronics)

	/* cassette */
	MCFG_CASSETTE_ADD( CASSETTE_TAG, msx_cassette_interface )

	/* real time clock */
	MCFG_RP5C01_ADD("rtc", XTAL_32_768kHz, rtc_intf)

	MCFG_FD1793_ADD("wd179x", msx_wd17xx_interface ) // TODO confirm type

	MCFG_LEGACY_FLOPPY_2_DRIVES_ADD(msx_floppy_interface)

	MCFG_FRAGMENT_ADD(msx_cartslot)

	/* Software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list","msx2_cart")
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( msx2_pal, msx2 )
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_SIZE(MSX2_TOTAL_XRES_PIXELS, 313*2)
	MCFG_SCREEN_REFRESH_RATE(50)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

/********************************  MSX 1 **********************************/

/* MSX */

ROM_START (msx)
	ROM_REGION (0x8000, "maincpu", 0)
	ROM_LOAD ("msx.rom", 0x0000, 0x8000, CRC(8205795e) SHA1(829c00c3114f25b3dae5157c0a238b52a3ac37db))
ROM_END

MSX_LAYOUT_INIT (msx)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB RAM */
MSX_LAYOUT_END

/* MSX - Al Alamiah AX-170 */

ROM_START (ax170)
	ROM_REGION (0x10000, "maincpu", 0)
	ROM_LOAD ("ax170bios.rom", 0x0000, 0x8000, CRC(bd95c436) SHA1(5e094fca95ab8e91873ee372a3f1239b9a48a48d))
	ROM_LOAD ("ax170arab.rom", 0x8000, 0x8000, CRC(339cd1aa) SHA1(0287b2ec897b9196788cd9f10c99e1487d7adbbb))
ROM_END

MSX_LAYOUT_INIT (ax170)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 1, 2, ROM, 0x8000, 0x8000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 2, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB RAM */
MSX_LAYOUT_END

/* MSX - Canon V-10 */

ROM_START (canonv10)
	ROM_REGION (0x8000, "maincpu", 0)
	ROM_LOAD ("v10bios.rom", 0x0000, 0x8000, CRC(e9ccd789) SHA1(8963fc041975f31dc2ab1019cfdd4967999de53e))
ROM_END

MSX_LAYOUT_INIT (canonv10)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 3, 1, RAM, 0x4000, 0xC000)   /* 16KB RAM */
	MSX_LAYOUT_SLOT (3, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
MSX_LAYOUT_END

/* MSX - Canon V-20 */
ROM_START (canonv20)
	ROM_REGION (0x8000, "maincpu", 0)
	ROM_LOAD ("v20bios.rom", 0x0000, 0x8000, CRC(e9ccd789) SHA1(8963fc041975f31dc2ab1019cfdd4967999de53e))
ROM_END

MSX_LAYOUT_INIT (canonv20)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB RAM */
	MSX_LAYOUT_SLOT (3, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
MSX_LAYOUT_END

/* MSX - Daewoo DPC-100 */

ROM_START (dpc100)
	ROM_REGION (0xc000, "maincpu", 0)
	ROM_LOAD ("100bios.rom", 0x0000, 0x8000, CRC(3ab0cd3b) SHA1(171b587bd5a947a13f3114120b6e7baca3b57d78))
	ROM_LOAD ("100han.rom", 0x8000, 0x4000, CRC(97478efb) SHA1(4421fa2504cbce18f7c84b5ea97f04e017007f07))
ROM_END

MSX_LAYOUT_INIT (dpc100)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (0, 0, 2, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 3, 1, RAM, 0x4000, 0xC000)   /* 16KB RAM */
	MSX_LAYOUT_SLOT (3, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
MSX_LAYOUT_END

/* MSX - Daewoo DPC-180 */

ROM_START (dpc180)
	ROM_REGION (0xc000, "maincpu", 0)
	ROM_LOAD ("180bios.rom", 0x0000, 0x8000, CRC(3ab0cd3b) SHA1(171b587bd5a947a13f3114120b6e7baca3b57d78))
	ROM_LOAD ("180han.rom", 0x8000, 0x4000, CRC(97478efb) SHA1(4421fa2504cbce18f7c84b5ea97f04e017007f07))
ROM_END

MSX_LAYOUT_INIT (dpc180)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (0, 0, 2, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 2, 2, RAM, 0x8000, 0x8000)   /* 32KB RAM */
	MSX_LAYOUT_SLOT (3, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
MSX_LAYOUT_END

/* MSX - Daewoo DPC-200 */

ROM_START (dpc200)
	ROM_REGION (0xc000, "maincpu", 0)
	ROM_LOAD ("200bios.rom", 0x0000, 0x8000, CRC(3ab0cd3b) SHA1(171b587bd5a947a13f3114120b6e7baca3b57d78))
	ROM_LOAD ("200han.rom", 0x8000, 0x4000, CRC(97478efb) SHA1(4421fa2504cbce18f7c84b5ea97f04e017007f07))
ROM_END

MSX_LAYOUT_INIT (dpc200)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (0, 0, 2, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB RAM */
	MSX_LAYOUT_SLOT (3, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
MSX_LAYOUT_END

/* MSX - Goldstar FC-200 */

ROM_START (gsfc200)
	ROM_REGION (0x8000, "maincpu", 0)
	ROM_LOAD ("fc200bios.rom.u5a", 0x0000, 0x4000, CRC(61f473fb) SHA1(c425750bbb2ae1d278216b45029d303e37d8df2f))
	ROM_LOAD ("fc200bios.rom.u5b", 0x4000, 0x4000, CRC(1a99b1a1) SHA1(e18f72271b64693a2a2bc226e1b9ebd0448e07c0))
ROM_END

MSX_LAYOUT_INIT (gsfc200)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB RAM */
	MSX_LAYOUT_SLOT (3, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
MSX_LAYOUT_END

/* MSX - Gradiente Expert 1.0 */

ROM_START (expert10)
	ROM_REGION (0x8000, "maincpu",0)
	ROM_LOAD ("expbios.rom", 0x0000, 0x8000, CRC(07610d77) SHA1(ef3e010eb57e4476700a3bbff9d2119ab3acdf62))
ROM_END

MSX_LAYOUT_INIT (expert10)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB RAM */
	MSX_LAYOUT_SLOT (3, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
MSX_LAYOUT_END

/* MSX - Gradiente Expert 1.1 */
ROM_START (expert11)
	ROM_REGION (0xc000, "maincpu",0)
	ROM_LOAD ("expbios11.rom", 0x0000, 0x8000, CRC(efb4b972) SHA1(d6720845928ee848cfa88a86accb067397685f02))
ROM_END

MSX_LAYOUT_INIT (expert11)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB RAM */
	MSX_LAYOUT_SLOT (3, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
MSX_LAYOUT_END

/* MSX - Gradiente Expert 1.3 */
ROM_START (expert13)
	ROM_REGION (0x8000, "maincpu",0)
	ROM_LOAD ("expbios13.rom", 0x0000, 0x8000, CRC(5638bc38) SHA1(605f5af3f358c6811f54e0173bad908614a198c0))
ROM_END

MSX_LAYOUT_INIT (expert13)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, RAM_MM, 0x10000, 0x0000)   /* 64KB Mapper RAM */
	MSX_LAYOUT_SLOT (3, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
MSX_LAYOUT_END

/* MSX - Gradiente Expert DDPlus */
ROM_START (expertdp)
	ROM_REGION (0xc000, "maincpu",0)
	ROM_LOAD ("eddpbios.rom", 0x0000, 0x8000, CRC(efb4b972) SHA1(d6720845928ee848cfa88a86accb067397685f02))
	ROM_LOAD ("eddpdisk.rom", 0x8000, 0x4000, CRC(549f1d90) SHA1(f1525de4e0b60a6687156c2a96f8a8b2044b6c56))
ROM_END

MSX_LAYOUT_INIT (expertdp)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB RAM */
	MSX_LAYOUT_SLOT (3, 3, 1, 2, DISK_ROM2, 0x4000, 0x8000)
MSX_LAYOUT_END

/* MSX - Gradiente Expert Plus */

ROM_START (expertpl)
	ROM_REGION (0xc000, "maincpu",0)
	ROM_LOAD ("exppbios.rom", 0x0000, 0x8000, CRC(efb4b972) SHA1(d6720845928ee848cfa88a86accb067397685f02))
	ROM_LOAD ("exppdemo.rom", 0x8000, 0x4000, CRC(a9bbef64) SHA1(d4cea8c815f3eeabe0c6a1c845f902ec4318bf6b))
ROM_END

MSX_LAYOUT_INIT (expertpl)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB RAM */
	MSX_LAYOUT_SLOT (3, 3, 2, 1, ROM, 0x4000, 0x8000)
MSX_LAYOUT_END

/* MSX - JVC HC-7GB */

ROM_START (jvchc7gb)
	ROM_REGION (0x8000, "maincpu", 0)
	ROM_LOAD ("hc7gbbios.rom", 0x0000, 0x8000, CRC(e9ccd789) SHA1(8963fc041975f31dc2ab1019cfdd4967999de53e))
ROM_END

MSX_LAYOUT_INIT (jvchc7gb)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB RAM */
	MSX_LAYOUT_SLOT (3, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
MSX_LAYOUT_END

/* MSX - Mitsubishi ML-F80 */

ROM_START (mlf80)
	ROM_REGION (0x8000, "maincpu", 0)
	ROM_LOAD ("mlf80bios.rom", 0x0000, 0x8000, CRC(e9ccd789) SHA1(8963fc041975f31dc2ab1019cfdd4967999de53e))
ROM_END

MSX_LAYOUT_INIT (mlf80)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB RAM */
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
MSX_LAYOUT_END

/* MSX - Mitsubishi ML-FX1 */

ROM_START (mlfx1)
	ROM_REGION (0x8000, "maincpu", 0)
	ROM_LOAD ("mlfx1bios.rom", 0x0000, 0x8000, CRC(62867dce) SHA1(0cbe0df4af45e8f531e9c761403ac9e71808f20c))
ROM_END

MSX_LAYOUT_INIT (mlfx1)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 2, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB RAM */
MSX_LAYOUT_END

/* MSX - National CF-1200 */

ROM_START (cf1200)
	ROM_REGION (0x8000, "maincpu", 0)
	ROM_LOAD ("1200bios.rom", 0x0000, 0x8000, CRC(5ad03407) SHA1(c7a2c5baee6a9f0e1c6ee7d76944c0ab1886796c))
ROM_END

MSX_LAYOUT_INIT (cf1200)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (0, 0, 3, 1, RAM, 0x4000, 0xC000)   /* 16KB RAM */
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
MSX_LAYOUT_END

/* MSX - National CF-2000 */

ROM_START (cf2000)
	ROM_REGION (0x8000, "maincpu", 0)
	ROM_LOAD ("2000bios.rom", 0x0000, 0x8000, CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821))
ROM_END

MSX_LAYOUT_INIT (cf2000)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (0, 0, 3, 1, RAM, 0x4000, 0xC000)   /* 16KB RAM */
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
MSX_LAYOUT_END

/* MSX - National CF-2700 */
ROM_START (cf2700)
	ROM_REGION (0x8000, "maincpu", 0)
	ROM_LOAD ("2700bios.rom.ic32", 0x0000, 0x8000, CRC(5ad03407) SHA1(c7a2c5baee6a9f0e1c6ee7d76944c0ab1886796c))
ROM_END

MSX_LAYOUT_INIT (cf2700)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (0, 0, 2, 2, RAM, 0x8000, 0x8000)   /* 32KB RAM */
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
MSX_LAYOUT_END

/* MSX - National CF-3000 */

ROM_START (cf3000)
	ROM_REGION (0x8000, "maincpu", 0)
	ROM_LOAD ("3000bios.rom", 0x0000, 0x8000, CRC(5ad03407) SHA1(c7a2c5baee6a9f0e1c6ee7d76944c0ab1886796c))
ROM_END

MSX_LAYOUT_INIT (cf3000)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB RAM */
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
MSX_LAYOUT_END

/* MSX - National CF-3300 */
ROM_START (cf3300)
	ROM_REGION (0xc000, "maincpu", 0)
	ROM_LOAD ("3300bios.rom", 0x0000, 0x8000, CRC(5ad03407) SHA1(c7a2c5baee6a9f0e1c6ee7d76944c0ab1886796c))
	ROM_LOAD ("3300disk.rom", 0x8000, 0x4000, CRC(549f1d90) SHA1(f1525de4e0b60a6687156c2a96f8a8b2044b6c56))
ROM_END

MSX_LAYOUT_INIT (cf3300)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB RAM */
	MSX_LAYOUT_SLOT (3, 1, 1, 2, DISK_ROM2, 0x4000, 0x8000)
MSX_LAYOUT_END

/* MSX - National FS-1300 */

ROM_START (fs1300)
	ROM_REGION (0x8000, "maincpu", 0)
	ROM_LOAD ("1300bios.rom", 0x0000, 0x8000, CRC(5ad03407) SHA1(c7a2c5baee6a9f0e1c6ee7d76944c0ab1886796c))
ROM_END

MSX_LAYOUT_INIT (fs1300)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB RAM */
MSX_LAYOUT_END

/* MSX - National FS-4000 */
ROM_START (fs4000)
	ROM_REGION (0x38000 ,"maincpu", 0)
	ROM_LOAD ("4000bios.rom", 0x0000, 0x8000, CRC(071135e0) SHA1(df48902f5f12af8867ae1a87f255145f0e5e0774))
	ROM_LOAD ("4000word.rom", 0x8000, 0x8000, CRC(950b6c87) SHA1(931d6318774bd495a32ec3dabf8d0edfc9913324))
	ROM_LOAD ("4000kdr.rom", 0x10000, 0x8000, CRC(ebaa5a1e) SHA1(77bd67d5d10d459d343e79eafcd8e17eb0f209dd))
	ROM_LOAD ("4000kfn.rom", 0x18000, 0x20000, CRC(956dc96d) SHA1(9ed3ab6d893632b9246e91b412cd5db519e7586b))
ROM_END

MSX_LAYOUT_INIT (fs4000)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 2, ROM, 0x8000, 0x8000)
	MSX_LAYOUT_SLOT (3, 1, 1, 2, ROM, 0x8000, 0x10000)
	MSX_LAYOUT_SLOT (3, 2, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB RAM */
	MSX_LAYOUT_KANJI (0x18000)
MSX_LAYOUT_END

/*MSX - Olympia PHC-2*/

ROM_START (phc2)
	ROM_REGION (0x8000, "maincpu", 0)
	ROM_LOAD ("phc2bios.rom", 0x0000, 0x8000, CRC(4f7bb04b) SHA1(ab0177624d46dd77ab4f50ffcb983c3ba88223f4))
ROM_END

MSX_LAYOUT_INIT (phc2)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB RAM */
MSX_LAYOUT_END

/* MSX - Olympia PHC-28 */

ROM_START (phc28)
	ROM_REGION (0x8000, "maincpu", 0)
	ROM_LOAD ("phc28bios.rom", 0x0000, 0x8000, CRC(eceb2802) SHA1(195950173701abeb460a1a070d83466f3f53b337))
ROM_END

MSX_LAYOUT_INIT (phc28)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 2, 4, RAM, 0x8000, 0x0000)   /* 32KB RAM */
MSX_LAYOUT_END

/* MSX - Panasonic CF-2700G */

ROM_START (cf2700g)
	ROM_REGION (0x8000, "maincpu", 0)
	ROM_LOAD ("cf2700g.rom", 0x0000, 0x8000, CRC(4aa194f4) SHA1(69bf27b610e11437dad1f7a1c37a63179a293d12))
ROM_END

MSX_LAYOUT_INIT (cf2700g)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB?? RAM */
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
MSX_LAYOUT_END

/* MSX - Philips NMS-801 */

ROM_START (nms801)
	ROM_REGION (0x8000, "maincpu", 0)
	ROM_LOAD ("801bios.rom", 0x0000, 0x8000, CRC(fa089461) SHA1(21329398c0f350e330b353f45f21aa7ba338fc8d))
ROM_END

MSX_LAYOUT_INIT (nms801)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB RAM */
MSX_LAYOUT_END

/* MSX - Philips VG-8000 */

ROM_START (vg8000)
	ROM_REGION (0x8000, "maincpu", 0)
	ROM_LOAD ("8000bios.rom", 0x0000, 0x8000, CRC(efd970b0) SHA1(42252cf87deeb58181a7bfec7c874190a1351779))
ROM_END

MSX_LAYOUT_INIT (vg8000)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, RAM, 0x4000, 0xC000)   /* 16KB RAM */
MSX_LAYOUT_END

/* MSX - Philips VG-8010 */

ROM_START (vg8010)
	ROM_REGION (0x8000, "maincpu", 0)
	ROM_LOAD ("8010bios.rom", 0x0000, 0x8000, CRC(efd970b0) SHA1(42252cf87deeb58181a7bfec7c874190a1351779))
ROM_END

MSX_LAYOUT_INIT (vg8010)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, RAM, 0x8000, 0x8000)   /* 32KB RAM */
MSX_LAYOUT_END

/* MSX - Philips VG-8010F */

ROM_START (vg8010f)
	ROM_REGION (0x8000, "maincpu", 0)
	ROM_LOAD ("8010fbios.rom", 0x0000, 0x8000, CRC(df57c9ca) SHA1(898630ad1497dc9a329580c682ee55c4bcb9c30c))
ROM_END

MSX_LAYOUT_INIT (vg8010f)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, RAM, 0x8000, 0x8000)   /* 32KB RAM */
MSX_LAYOUT_END

/* MSX - Philips VG-8020-00 */

ROM_START (vg802000)
	ROM_REGION (0x8000, "maincpu", 0)
	ROM_LOAD ("8020-00bios.rom", 0x0000, 0x8000, CRC(8205795e) SHA1(829c00c3114f25b3dae5157c0a238b52a3ac37db))
ROM_END

MSX_LAYOUT_INIT (vg802000)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB RAM */
MSX_LAYOUT_END

/* MSX - Philips VG-8020-20 */

ROM_START (vg802020)
	ROM_REGION (0x8000, "maincpu", 0)
	ROM_LOAD ("8020-20bios.rom", 0x0000, 0x8000, CRC(A317E6B4) SHA1(E998F0C441F4F1800EF44E42CD1659150206CF79))
ROM_END

MSX_LAYOUT_INIT (vg802020)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 2, 0, 4, RAM_MM, 0x10000, 0x0000)   /* 64KB Mapper RAM */
MSX_LAYOUT_END

/* MSX - Philips VG-8020F */

ROM_START (vg8020f)
	ROM_REGION (0x8000, "maincpu", 0)
	ROM_LOAD ("vg8020f.rom", 0x0000, 0x8000, CRC(6e692fa1) SHA1(9eaad185efc8e224368d1db4949eb9659c26fb2c))
ROM_END

MSX_LAYOUT_INIT (vg8020f)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 2, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB?? RAM */
MSX_LAYOUT_END

/* MSX - Pioneer PX-7 */

ROM_START (piopx7)
	ROM_REGION (0xc000, "maincpu", 0)
	ROM_LOAD ("ym2301.ic12", 0x0000, 0x8000, CRC(e9ccd789) SHA1(8963fc041975f31dc2ab1019cfdd4967999de53e))
	ROM_LOAD ("pd5031.ic13", 0x8000, 0x2000, CRC(91e0df72) SHA1(4f0102cdc27216fd9bcdb9663db728d2ccd8ca6d))
	ROM_FILL( 0xa000, 0x2000, 0x6E )
ROM_END

MSX_LAYOUT_INIT (piopx7)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (0, 0, 2, 2, RAM, 0x8000, 0x8000)   /* 32KB RAM */
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 1, 2, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
MSX_LAYOUT_END

/* MSX - Samsung SPC-800 */

ROM_START (spc800)
	ROM_REGION (0xc000, "maincpu", 0)
	ROM_LOAD ("spc800bios.rom", 0x0000, 0x8000, CRC(3ab0cd3b) SHA1(171b587bd5a947a13f3114120b6e7baca3b57d78))
	ROM_LOAD ("spc800han.rom", 0x8000, 0x4000, CRC(5ae2b013) SHA1(1e7616261a203580c1044205ad8766d104f1d874))
ROM_END

MSX_LAYOUT_INIT (spc800)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (0, 0, 4, 2, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB?? RAM */
	MSX_LAYOUT_SLOT (3, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
MSX_LAYOUT_END

/* MSX - Sanyo MPC-64 */

ROM_START (mpc64)
	ROM_REGION (0x8000, "maincpu", 0)
	ROM_LOAD ("mpc64bios.rom", 0x0000, 0x8000, CRC(d6e704ad) SHA1(d67be6d7d56d7229418f4e122f2ec27990db7d19))
ROM_END

MSX_LAYOUT_INIT (mpc64)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB RAM */
MSX_LAYOUT_END

/* MSX - Sanyo MPC-100 */

ROM_START (mpc100)
	ROM_REGION (0x8000, "maincpu", 0)
	ROM_LOAD ("mpc100bios.rom", 0x0000, 0x8000, CRC(e9ccd789) SHA1(8963fc041975f31dc2ab1019cfdd4967999de53e))
ROM_END

MSX_LAYOUT_INIT (mpc100)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB RAM */
MSX_LAYOUT_END

/* MSX - Sanyo PHC-28L */

ROM_START (phc28l)
	ROM_REGION( 0x8000, "maincpu", 0)
	ROM_LOAD ("28lbios.rom", 0x0000, 0x8000, CRC(d2110d66) SHA1(d3af963e2529662eae63f04a2530454685a1989f))
ROM_END

MSX_LAYOUT_INIT (phc28l)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 0, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 0, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB RAM */
MSX_LAYOUT_END

/* MSX - Sanyo PHC-28S */

ROM_START (phc28s)
	ROM_REGION (0x8000, "maincpu", 0)
	ROM_LOAD ("28sbios.rom", 0x0000, 0x8000, CRC(e5cf6b3c) SHA1(b1cce60ef61c058f5e42ef7ac635018d1a431168))
ROM_END

MSX_LAYOUT_INIT (phc28s)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 2, 4, RAM, 0x8000, 0x0000)   /* 32KB RAM */
MSX_LAYOUT_END

/* MSX - Sanyo Wavy MPC-10 */

ROM_START (mpc10)
	ROM_REGION (0x8000, "maincpu", 0)
	ROM_LOAD ("mpc10.rom", 0x0000, 0x8000, CRC(e9ccd789) SHA1(8963fc041975f31dc2ab1019cfdd4967999de53e))
ROM_END

MSX_LAYOUT_INIT (mpc10)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 2, 2, RAM, 0x8000, 0x8000)   /* 32KB RAM */
MSX_LAYOUT_END

/* MSX - Sharp Epcom HotBit 1.1 */

ROM_START (hotbit11)
	ROM_REGION (0x8000, "maincpu",0)
	ROM_LOAD ("hotbit11.rom", 0x0000, 0x8000, CRC(b6942694) SHA1(663f8c512d04d213fa616b0db5eefe3774012a4b))
ROM_END

MSX_LAYOUT_INIT (hotbit11)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB RAM */
MSX_LAYOUT_END

/* MSX - Sharp Epcom HotBit 1.2 */

ROM_START (hotbit12)
	ROM_REGION (0x8000, "maincpu",0)
	ROM_LOAD ("hotbit12.rom", 0x0000, 0x8000, CRC(f59a4a0c) SHA1(9425815446d468058705bae545ffa13646744a87))
ROM_END

MSX_LAYOUT_INIT (hotbit12)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB RAM */
MSX_LAYOUT_END

/* MSX - Sharp Epcom HotBit 1.3b */

ROM_START (hotbi13b)
	ROM_REGION (0x8000, "maincpu",0)
	ROM_LOAD ("hotbit13b.rom", 0x0000, 0x8000, CRC(7a19820e) SHA1(e0c2bfb078562d15acabc5831020a2370ea87052))
ROM_END

MSX_LAYOUT_INIT (hotbi13b)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, RAM_MM, 0x10000, 0x0000)   /* 64KB Mapper RAM */
MSX_LAYOUT_END

/* MSX - Sharp Epcom HotBit 1.3p */

ROM_START (hotbi13p)
	ROM_REGION (0x8000, "maincpu",0)
	ROM_LOAD ("hotbit13p.rom", 0x0000, 0x8000, CRC(150e239c) SHA1(942f9507d206cd8156f15601fe8032fcf0e3875b))
ROM_END

MSX_LAYOUT_INIT (hotbi13p)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, RAM_MM, 0x10000, 0x0000)   /* 64KB Mapper RAM */
MSX_LAYOUT_END

/* MSX - Sony HB-10P */

ROM_START (hb10p)
	ROM_REGION (0x10000, "maincpu", 0)
	ROM_LOAD ("10pbios.rom", 0x0000, 0x8000, CRC(0f488dd8) SHA1(5e7c8eab238712d1e18b0219c0f4d4dae180420d))
ROM_END

MSX_LAYOUT_INIT (hb10p)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB RAM */
MSX_LAYOUT_END

/* MSX - Sony HB-20P */

ROM_START (hb20p)
	ROM_REGION (0x10000, "maincpu", 0)
	ROM_LOAD ("20pbios.rom", 0x0000, 0x8000, CRC(21af423f) SHA1(365c93d7652c9f727221689bcc348652832a7b7a))
ROM_END

MSX_LAYOUT_INIT (hb20p)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB RAM */
MSX_LAYOUT_END

/* MSX - Sony HB-201 */

ROM_START (hb201)
	ROM_REGION (0x10000, "maincpu", 0)
	ROM_LOAD ("201bios.rom.ic9", 0x0000, 0x8000, CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821))
	ROM_LOAD ("201note.rom.ic8", 0x8000, 0x4000, CRC(74567244) SHA1(0f4f09f1a6ef7535b243afabfb44a3a0eb0498d9))
	ROM_FILL( 0xc000, 0x4000, 0xff )
ROM_END

MSX_LAYOUT_INIT (hb201)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (0, 0, 2, 2, ROM, 0x8000, 0x8000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB RAM */
MSX_LAYOUT_END

/* MSX - Sony HB-201P */

ROM_START (hb201p)
	ROM_REGION (0xc000, "maincpu", 0)
	ROM_LOAD ("201pbios.rom.ic9", 0x0000, 0x8000, CRC(0f488dd8) SHA1(5e7c8eab238712d1e18b0219c0f4d4dae180420d))
	ROM_LOAD ("201pnote.rom.ic8", 0x8000, 0x4000, CRC(1ff9b6ec) SHA1(e84d3ec7a595ee36b50e979683c84105c1871857))
ROM_END

MSX_LAYOUT_INIT (hb201p)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (0, 0, 2, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB RAM */
MSX_LAYOUT_END

/* MSX - Sony HB-501P */

ROM_START (hb501p)
	ROM_REGION (0x8000, "maincpu", 0)
	ROM_LOAD ("501pbios.rom", 0x0000, 0x8000, CRC(0f488dd8) SHA1(5e7c8eab238712d1e18b0219c0f4d4dae180420d))
ROM_END

MSX_LAYOUT_INIT (hb501p)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB RAM */
MSX_LAYOUT_END

/* MSX - Sony HB-55D */

ROM_START (hb55d)
	ROM_REGION (0xc000, "maincpu", 0)
	ROM_LOAD ("55dbios.rom", 0x0000, 0x8000, CRC(7e2b32dd) SHA1(38a645febd0e0fe86d594f27c2d14be995acc730))
	ROM_LOAD ("55dnote.rom", 0x8000, 0x4000, CRC(8aae0494) SHA1(97ce59892573cac3c440efff6d74c8a1c29a5ad3))
ROM_END

MSX_LAYOUT_INIT (hb55d)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (0, 0, 2, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 3, 1, RAM, 0x4000, 0xC000)   /* 16KB RAM */
	MSX_LAYOUT_SLOT (3, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
MSX_LAYOUT_END

/* MSX - Sony HB-55P */

ROM_START (hb55p)
	ROM_REGION (0xc000, "maincpu", 0)
	ROM_LOAD ("55pbios.ic42", 0x0000, 0x4000, CRC(24c198be) SHA1(7f8c94cb8913db32a696dec80ffc78e46693f1b7))
	ROM_LOAD ("55pbios.ic43", 0x4000, 0x4000, CRC(e516e7e5) SHA1(05fedd4b9bfcf4949020c79d32c4c3f03a54fb62))
	ROM_LOAD ("55pnote.ic44", 0x8000, 0x4000, CRC(492b12f8) SHA1(b262aedc71b445303f84efe5e865cbb71fd7d952))
ROM_END

MSX_LAYOUT_INIT (hb55p)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (0, 0, 2, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 3, 1, RAM, 0x4000, 0xC000)   /* 16KB RAM */
	MSX_LAYOUT_SLOT (3, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
MSX_LAYOUT_END

/* MSX - Sony HB-75D */

ROM_START (hb75d)
	ROM_REGION (0xc000, "maincpu", 0)
	ROM_LOAD ("75dbios.rom", 0x0000, 0x8000, CRC(7e2b32dd) SHA1(38a645febd0e0fe86d594f27c2d14be995acc730))
	ROM_LOAD ("75dnote.rom", 0x8000, 0x4000, CRC(8aae0494) SHA1(97ce59892573cac3c440efff6d74c8a1c29a5ad3))
ROM_END

MSX_LAYOUT_INIT (hb75d)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (0, 0, 2, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB RAM */
	MSX_LAYOUT_SLOT (3, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
MSX_LAYOUT_END

/* MSX - Sony HB-75P */

ROM_START (hb75p)
	ROM_REGION (0xc000, "maincpu", 0)
	ROM_LOAD ("75pbios.ic42", 0x0000, 0x4000, CRC(24c198be) SHA1(7f8c94cb8913db32a696dec80ffc78e46693f1b7))
	ROM_LOAD ("75pbios.ic43", 0x4000, 0x4000, CRC(e516e7e5) SHA1(05fedd4b9bfcf4949020c79d32c4c3f03a54fb62))
	ROM_LOAD ("75pnote.ic44", 0x8000, 0x4000, CRC(492b12f8) SHA1(b262aedc71b445303f84efe5e865cbb71fd7d952))
ROM_END

MSX_LAYOUT_INIT (hb75p)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (0, 0, 2, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB RAM */
	MSX_LAYOUT_SLOT (3, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
MSX_LAYOUT_END

/* MSX - Spectravideo SVI-728 */

ROM_START (svi728)
	ROM_REGION (0xc000, "maincpu", 0)
	ROM_LOAD ("728bios.rom", 0x0000, 0x8000, CRC(1ce9246c) SHA1(ea6a82cf8c6e65eb30b98755c8577cde8d9186c0))
	ROM_LOAD ("707disk.rom", 0x8000, 0x4000, CRC(f9978853) SHA1(6aa856cc56eb98863c9da7a566571605682b5c6b))
ROM_END

MSX_LAYOUT_INIT (svi728)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB RAM */
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 1, 1, DISK_ROM2, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 1, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
MSX_LAYOUT_END

/* MSX - Spectravideo SVI-738 */

ROM_START (svi738)
	ROM_REGION (0x10000, "maincpu", 0)
	ROM_LOAD ("738bios.rom", 0x0000, 0x8000, CRC(ad007d62) SHA1(c53b3f2c00f31683914f7452f3f4d94ae2929c0d))
	ROM_LOAD ("738disk.rom", 0x8000, 0x4000, CRC(acd27a36) SHA1(99a40266bc296cef1d432cb0caa8df1a7e570be4))
	ROM_LOAD ("738232c.rom", 0xc000, 0x2000, CRC(3353dcc6) SHA1(4e9384c9d137f0ab65ffc5a78f04cd8c9df6c8b7))
	ROM_FILL (0xe000, 0x2000, 0xff)
ROM_END

MSX_LAYOUT_INIT (svi738)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB RAM */
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 1, 1, ROM, 0x4000, 0xc000)
	MSX_LAYOUT_SLOT (3, 1, 1, 2, DISK_ROM2, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 2, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
MSX_LAYOUT_END

/* MSX - Spectravideo SVI-738 Swedish */

ROM_START (svi738sw)
	ROM_REGION (0x10000, "maincpu", 0)
	ROM_LOAD ("738sebios.rom", 0x0000, 0x8000, CRC(c8ccdaa0) SHA1(87f4d0fa58cfe9cef818a3185df2735e6da6168c))
	ROM_LOAD ("738sedisk.rom", 0x8000, 0x4000, CRC(fb884df4) SHA1(6d3a530ae822ec91f6444c681c9b08b9efadc7e7))
	ROM_LOAD ("738se232c.rom", 0xc000, 0x2000, CRC(3353dcc6) SHA1(4e9384c9d137f0ab65ffc5a78f04cd8c9df6c8b7))
	ROM_FILL (0xe000, 0x2000, 0xff)
ROM_END

MSX_LAYOUT_INIT (svi738sw)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB RAM */
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 1, 1, ROM, 0x4000, 0xc000)
	MSX_LAYOUT_SLOT (3, 1, 1, 2, DISK_ROM2, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 2, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
MSX_LAYOUT_END

/* MSX - Spectravideo SVI-738 Poland*/

ROM_START (svi738pl)
	ROM_REGION (0x10000, "maincpu", 0)
	ROM_LOAD ("738plbios.rom", 0x0000, 0x8000, CRC(431b8bf5) SHA1(c90077ed84133a947841e07856e71133ba779da6)) // IC51 on board
	ROM_LOAD ("738disk.rom", 0x8000, 0x4000, CRC(acd27a36) SHA1(99a40266bc296cef1d432cb0caa8df1a7e570be4))
	ROM_LOAD ("738232c.rom", 0xc000, 0x2000, CRC(3353dcc6) SHA1(4e9384c9d137f0ab65ffc5a78f04cd8c9df6c8b7))
	ROM_FILL (0xe000, 0x2000, 0xff)
ROM_END

MSX_LAYOUT_INIT (svi738pl)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB RAM */
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 1, 1, ROM, 0x4000, 0xc000)
	MSX_LAYOUT_SLOT (3, 1, 1, 2, DISK_ROM2, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 2, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
MSX_LAYOUT_END

/* MSX - Talent DPC-200 / Daewoo DPC-200E */

ROM_START (tadpc200)
	ROM_REGION (0x8000, "maincpu", 0)
	ROM_LOAD ("dpc200bios.rom", 0x0000, 0x8000, CRC(8205795e) SHA1(829c00c3114f25b3dae5157c0a238b52a3ac37db))
ROM_END

MSX_LAYOUT_INIT (tadpc200)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB RAM */
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
MSX_LAYOUT_END


/* MSX - Talent DPC-200A */

ROM_START (tadpc20a)
	ROM_REGION (0x8000, "maincpu", 0)
	ROM_LOAD ("dpc200abios.rom", 0x0000, 0x8000, CRC(8205795e) SHA1(829c00c3114f25b3dae5157c0a238b52a3ac37db))
ROM_END

MSX_LAYOUT_INIT (tadpc20a)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB RAM */
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
MSX_LAYOUT_END

/* MSX - Toshiba HX-10 */
/* The BIOS on the Toshiba HX-10 is inside a big 64pin Toshiba chip label TCX-1007 */

ROM_START (hx10)
	ROM_REGION (0x8000, "maincpu",0)
	ROM_LOAD ("tcx-1007.ic15", 0x0000, 0x8000, CRC(5486b711) SHA1(4dad9de7c28b452351cc12910849b51bd9a37ab3))
ROM_END

MSX_LAYOUT_INIT (hx10)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB RAM */
	MSX_LAYOUT_SLOT (3, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
MSX_LAYOUT_END

/* MSX - Toshiba HX-10S */

ROM_START (hx10s)
	ROM_REGION (0x8000, "maincpu",0)
	ROM_LOAD ("hx10sbios.rom", 0x0000, 0x8000, CRC(5486b711) SHA1(4dad9de7c28b452351cc12910849b51bd9a37ab3))
ROM_END

MSX_LAYOUT_INIT (hx10s)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 3, 1, RAM, 0x4000, 0xC000)   /* 16KB RAM */
	MSX_LAYOUT_SLOT (3, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
MSX_LAYOUT_END

/* MSX - Toshiba HX-20 */

ROM_START (hx20)
	ROM_REGION (0x10000, "maincpu",0)
	ROM_LOAD ("hx20bios.rom", 0x0000, 0x8000, CRC(8205795e) SHA1(829c00c3114f25b3dae5157c0a238b52a3ac37db))
	ROM_LOAD ("hx20word.rom", 0x8000, 0x8000, CRC(39b3e1c0) SHA1(9f7cfa932bd7dfd0d9ecaadc51655fb557c2e125))
ROM_END

MSX_LAYOUT_INIT (hx20)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (0, 0, 2, 2, RAM, 0x8000, 0x8000)   /* 32KB RAM */
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 2, 0, RAM, 0x8000, 0x0000)   /* 32KB RAM */
	MSX_LAYOUT_SLOT (3, 3, 1, 2, ROM, 0x8000, 0x8000)
MSX_LAYOUT_END

/* MSX - Yamaha CX5M / Yamaha CX5M-2 */

ROM_START (cx5m)
	ROM_REGION (0x10000, "maincpu",0)
	ROM_LOAD ("cx5mbios.rom", 0x0000, 0x8000, CRC(e2242b53) SHA1(706dd67036baeec7127e4ccd8c8db8f6ce7d0e4c))
	ROM_LOAD ("sfg05m.rom",         0x8000, 0x8000, CRC(6c2545c9) SHA1(bc4b242647116f4886bb92e86421f97b1be51938))
ROM_END

MSX_LAYOUT_INIT (cx5m)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 2, ROM, 0x8000, 0x8000)
	MSX_LAYOUT_SLOT (3, 0, 2, 2, RAM, 0x8000, 0x8000)   /* 32KB RAM */
MSX_LAYOUT_END

/* MSX - Yamaha CX5M-128 */

ROM_START (cx5m128)
	ROM_REGION (0x18000, "maincpu",0)
	ROM_LOAD ("cx5m128bios.rom", 0x0000, 0x8000, CRC(507b2caa) SHA1(0dde59e8d98fa524961cd37b0e100dbfb42cf576))
	ROM_LOAD ("cx5m128ext.rom",  0x8000, 0x4000, CRC(feada82e) SHA1(48b0c2ff1f1e407cc44394219f7b3878efaa919f))
	ROM_LOAD ("sfg05.rom",      0xc000, 0x8000, CRC(2425c279) SHA1(d956167e234f60ad916120437120f86fc8c3c321))
	ROM_LOAD ("yrm502.rom",     0x14000,0x4000, CRC(5330fe21) SHA1(7b1798561ee1844a7d6432924fbee9b4fc591c19))
ROM_END

MSX_LAYOUT_INIT (cx5m128)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 1, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 1, 1, 1, ROM, 0x4000, 0x14000) /* YRM-502 */
	MSX_LAYOUT_SLOT (3, 2, 0, 4, RAM_MM, 0x20000, 0x0000)   /* 128KB Mapper RAM */
	MSX_LAYOUT_SLOT (3, 3, 0, 2, ROM, 0x8000, 0xc000) /* SFG-05 */
MSX_LAYOUT_END

/* MSX - Yamaha CX5MII */

ROM_START (cx5m2)
	ROM_REGION (0x14000, "maincpu",0)
	ROM_LOAD ("cx5m2bios.rom", 0x0000, 0x8000, CRC(507b2caa) SHA1(0dde59e8d98fa524961cd37b0e100dbfb42cf576))
	ROM_LOAD ("cx5m2ext.rom",  0x8000, 0x4000, CRC(feada82e) SHA1(48b0c2ff1f1e407cc44394219f7b3878efaa919f))
	ROM_LOAD ("sfg05.rom",       0xc000, 0x8000, CRC(2425c279) SHA1(d956167e234f60ad916120437120f86fc8c3c321))
ROM_END

MSX_LAYOUT_INIT (cx5m2)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 1, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 2, 0, 4, RAM_MM, 0x10000, 0x0000)   /* 64KB Mapper RAM */
	MSX_LAYOUT_SLOT (3, 3, 0, 2, ROM, 0x8000, 0xc000)
MSX_LAYOUT_END

/* MSX - Yamaha YIS303 */

ROM_START (yis303)
	ROM_REGION (0x14000, "maincpu",0)
	ROM_LOAD ("yis303bios.rom", 0x0000, 0x8000, CRC(e2242b53) SHA1(706dd67036baeec7127e4ccd8c8db8f6ce7d0e4c))
	ROM_FILL( 0x8000, 0xc000, 0xff )
ROM_END

MSX_LAYOUT_INIT (yis303)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 3, ROM, 0xC000, 0x0000)   /* Fill FF */
	MSX_LAYOUT_SLOT (3, 0, 3, 1, RAM, 0x4000, 0xC000)   /* 16KB RAM */
MSX_LAYOUT_END

/* MSX - Yamaha YIS503 */

ROM_START (yis503)
	ROM_REGION (0x10000, "maincpu",0)
	ROM_LOAD ("yis503bios.rom", 0x0000, 0x8000, CRC(e2242b53) SHA1(706dd67036baeec7127e4ccd8c8db8f6ce7d0e4c))
	ROM_FILL( 0x8000, 0x8000, 0xff )
ROM_END

MSX_LAYOUT_INIT (yis503)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 2, ROM, 0xC000, 0x0000)   /* Fill FF */
	MSX_LAYOUT_SLOT (3, 0, 2, 2, RAM, 0x8000, 0x8000)   /* 32KB RAM */
MSX_LAYOUT_END

/* MSX - Yamaha YIS503F */

ROM_START (yis503f)
	ROM_REGION (0x8000, "maincpu", 0)
	ROM_LOAD ("yis503f.rom", 0x0000, 0x8000, CRC(e9ccd789) SHA1(8963fc041975f31dc2ab1019cfdd4967999de53e))
ROM_END

MSX_LAYOUT_INIT (yis503f)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB?? RAM */
MSX_LAYOUT_END

/* MSX - Yamaha YIS503II */

ROM_START (yis503ii)
	ROM_REGION (0x8000, "maincpu",0)
	ROM_LOAD ("yis503iibios.rom", 0x0000, 0x8000, CRC(e2242b53) SHA1(706dd67036baeec7127e4ccd8c8db8f6ce7d0e4c))
ROM_END

MSX_LAYOUT_INIT (yis503ii)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB RAM */
MSX_LAYOUT_END

/* MSX - Yamaha YIS503IIR Russian */

ROM_START (y503iir)
	ROM_REGION (0xe000, "maincpu",0)
	ROM_LOAD ("yis503iirbios.rom",  0x0000, 0x8000, CRC(225a4f9e) SHA1(5173ac403e26c462f904f85c9ef5e7b1e19253e7))
	ROM_LOAD ("yis503iirdisk.rom", 0x8000, 0x4000, CRC(9eb7e24d) SHA1(3a481c7b7e4f0406a55952bc5b9f8cf9d699376c))
	ROM_LOAD ("yis503iirnet.rom",  0xc000, 0x2000, CRC(0731db3f) SHA1(264fbb2de69fdb03f87dc5413428f6aa19511a7f))
ROM_END

MSX_LAYOUT_INIT (y503iir)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 1, 1, 2, DISK_ROM2, 0x4000, 0x8000) /* National disk */
	MSX_LAYOUT_SLOT (3, 2, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB RAM */
	MSX_LAYOUT_SLOT (3, 3, 1, 1, ROM, 0x2000, 0xc000)   /* Net */
MSX_LAYOUT_END

/* MSX - Yamaha YIS503IIR Estonian */

ROM_START (y503iir2)
	ROM_REGION (0xe000, "maincpu",0)
	ROM_LOAD ("yis503ii2bios.rom",  0x0000, 0x8000, CRC(1548cee3) SHA1(42c7fff25b1bd90776ac0aea971241aedce8947d))
	ROM_LOAD ("yis503iirdisk.rom", 0x8000, 0x4000, CRC(9eb7e24d) SHA1(3a481c7b7e4f0406a55952bc5b9f8cf9d699376c))
	ROM_LOAD ("yis503iirnet.rom",  0xc000, 0x2000, CRC(0731db3f) SHA1(264fbb2de69fdb03f87dc5413428f6aa19511a7f))
ROM_END

MSX_LAYOUT_INIT (y503iir2)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 1, 1, 2, DISK_ROM2, 0x4000, 0x8000) /* National disk */
	MSX_LAYOUT_SLOT (3, 2, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB RAM */
	MSX_LAYOUT_SLOT (3, 3, 1, 1, ROM, 0x2000, 0xc000)   /* Net */
MSX_LAYOUT_END

/* MSX - Yamaha YIS503M */

ROM_START (yis503m)
	ROM_REGION (0x10000, "maincpu",0)
	ROM_LOAD ("yis503mbios.rom", 0x0000, 0x8000, CRC(e2242b53) SHA1(706dd67036baeec7127e4ccd8c8db8f6ce7d0e4c))
	ROM_LOAD ("sfg05m.rom",        0x8000, 0x8000, CRC(6c2545c9) SHA1(bc4b242647116f4886bb92e86421f97b1be51938))
ROM_END

MSX_LAYOUT_INIT (yis503m)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 2, ROM, 0x8000, 0x8000)
	MSX_LAYOUT_SLOT (3, 0, 2, 2, RAM, 0x8000, 0x8000)   /* 32KB RAM */
MSX_LAYOUT_END

/* MSX - Yashica YC-64 */

ROM_START (yc64)
	ROM_REGION (0x8000, "maincpu",0)
	ROM_LOAD ("yc64bios.rom", 0x0000, 0x8000, CRC(e9ccd789) SHA1(8963fc041975f31dc2ab1019cfdd4967999de53e))
ROM_END

MSX_LAYOUT_INIT (yc64)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB RAM */
MSX_LAYOUT_END

/* MSX - Yeno MX64 */

ROM_START (mx64)
	ROM_REGION (0x8000, "maincpu",0)
	ROM_LOAD ("mx64bios.rom", 0x0000, 0x8000, CRC(e0e894b7) SHA1(d99eebded5db5fce1e072d08e642c0909bc7efdd))
ROM_END

MSX_LAYOUT_INIT (mx64)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB RAM */
	MSX_LAYOUT_SLOT (3, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
MSX_LAYOUT_END

/* MSX - Frael Bruc 100-1 */

ROM_START (bruc100)
	ROM_REGION (0x8000, "maincpu",0)
		ROM_LOAD( "bruc100-1bios.rom", 0x0000, 0x8000, CRC(c7bc4298) SHA1(3abca440cba16ac5e162b602557d30169f77adab))
ROM_END

MSX_LAYOUT_INIT (bruc100)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, RAM_MM, 0x10000, 0x0000)   /* 64KB RAM */
MSX_LAYOUT_END


/********************************  MSX 2 **********************************/

/* MSX2 */

ROM_START (msx2)
	ROM_REGION (0x20000, "maincpu",0)
	ROM_LOAD ("msx2.rom", 0x0000, 0x8000, CRC(f05ed518) SHA1(5e1a4bd6826b29302a1eb88c340477e7cbd0b50a))
	ROM_LOAD ("msx2ext.rom", 0x8000, 0x4000, CRC(95db2959) SHA1(e7905d16d2ccd57a013c122dc432106cd59ef52c))
	ROM_LOAD_OPTIONAL ("disk.rom", 0xc000, 0x4000, CRC(b7c58fad) SHA1(bc517b4a248c3a1338c5efc937b0128b6a783808))
	ROM_LOAD_OPTIONAL ("fmpac.rom", 0x10000, 0x10000, CRC(0e84505d) SHA1(9d789166e3caf28e4742fe933d962e99618c633d))
ROM_END

MSX_LAYOUT_INIT (msx2)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 0, 1, 2, DISK_ROM, 0x4000, 0xc000)
	MSX_LAYOUT_SLOT (3, 3, 0, 4, RAM_MM, 0x20000, 0x0000)   /* 128KB Mapper RAM */
	MSX_LAYOUT_RAMIO_SET_BITS (0xf8)
MSX_LAYOUT_END

/* MSX2 - Al Alamiah AX-350 */

ROM_START (ax350)
	ROM_REGION (0x44000, "maincpu", 0)
	ROM_LOAD ("ax350bios.rom",  0x0000,  0x8000, CRC(ea306155) SHA1(35195ab67c289a0b470883464df66bc6ea5b00d3))
	ROM_LOAD ("ax350ext.rom",   0x8000,  0x4000, CRC(7c7540b7) SHA1(ebb76f9061e875365023523607db610f2eda1d26))
	/* 0x0c000 - 0x1ffff reserved for optional disk and fmpac roms from msx2 parent set */
	ROM_FILL (0xc000, 0x14000, 0)
	ROM_LOAD ("ax350arab.rom", 0x20000,  0x8000, CRC(c0d8fc85) SHA1(2c9600c6e0025fee10d249e97448ecaa37e38c42))
	ROM_LOAD ("ax350swp.rom",  0x28000,  0x8000, CRC(076f40fc) SHA1(4b4508131dca6d811694ae6379f41364c477de58))
	ROM_LOAD ("ax350paint.rom",0x30000, 0x10000, CRC(18956e3a) SHA1(ace202e87337fbc54fea21e22c0b3af0abe6f4ae))
	ROM_LOAD ("ax350disk.rom", 0x40000,  0x4000, CRC(1e7d6512) SHA1(78cd7f847e77fd8cd51a647efb2725ba93f4c471))
ROM_END

MSX_LAYOUT_INIT (ax350)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000,  0x0000)  /* Bios */
	MSX_LAYOUT_SLOT (0, 1, 0, 1, ROM, 0x4000,  0x8000)  /* Ext */
	MSX_LAYOUT_SLOT (0, 1, 1, 2, ROM, 0x8000,  0x20000)  /* Arab */
	MSX_LAYOUT_SLOT (0, 2, 1, 2, ROM, 0x8000,  0x28000) /* SWP */
	MSX_LAYOUT_SLOT (0, 3, 0, 4, ROM, 0x10000, 0x30000)  /* Paint */
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 1, 1, 2, DISK_ROM2, 0x4000, 0x40000) /* Disk */
	MSX_LAYOUT_SLOT (3, 2, 0, 4, RAM_MM, 0x20000, 0x0000)   /* 128KB Mapper RAM */
	MSX_LAYOUT_RAMIO_SET_BITS (0xf8)
MSX_LAYOUT_END

/* MSX2 - Al Alamiah AX-370 */

ROM_START (ax370)
	ROM_REGION (0x44000, "maincpu", 0)
	ROM_LOAD ("ax370bios.rom",  0x0000,  0x8000, CRC(ea306155) SHA1(35195ab67c289a0b470883464df66bc6ea5b00d3))
	ROM_LOAD ("ax370ext.rom",   0x8000,  0x4000, CRC(7c7540b7) SHA1(ebb76f9061e875365023523607db610f2eda1d26))
	/* 0x0c000 - 0x1ffff reserved for optional disk and fmpac roms from msx2 parent set */
	ROM_FILL (0xc000, 0x14000, 0)
	ROM_LOAD ("ax370arab.rom", 0x20000,  0x8000, CRC(c0d8fc85) SHA1(2c9600c6e0025fee10d249e97448ecaa37e38c42))
	ROM_LOAD ("ax370swp.rom",  0x28000,  0x8000, CRC(076f40fc) SHA1(4b4508131dca6d811694ae6379f41364c477de58))
	ROM_LOAD ("ax370paint.rom",0x30000, 0x10000, CRC(18956e3a) SHA1(ace202e87337fbc54fea21e22c0b3af0abe6f4ae))
	ROM_LOAD ("ax370disk.rom", 0x40000,  0x4000, CRC(60f8baba) SHA1(95de8809d2758fc0a743390ea5085b602e59e101))
ROM_END

MSX_LAYOUT_INIT (ax370)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000,  0x0000)  /* Bios */
	MSX_LAYOUT_SLOT (0, 2, 1, 2, ROM, 0x8000,  0x28000) /* SWP */
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, RAM_MM, 0x20000, 0x0000)   /* 128KB Mapper RAM */
	MSX_LAYOUT_SLOT (3, 1, 0, 1, ROM, 0x4000,  0x8000)  /* Ext */
	MSX_LAYOUT_SLOT (3, 1, 1, 2, ROM, 0x8000,  0x20000)  /* Arab */
	//MSX_LAYOUT_SLOT (3, 2, 1, 2, DISK_ROM, 0x4000, 0x40000) /* TC8566AF Disk controller*/
	MSX_LAYOUT_SLOT (3, 3, 0, 4, ROM, 0x10000, 0x30000)  /* Paint */
	MSX_LAYOUT_RAMIO_SET_BITS (0xf8)
MSX_LAYOUT_END

/* MSX2 - Daewoo CPC-300 */

ROM_START (cpc300)
	ROM_REGION (0x30000, "maincpu", 0)
	ROM_LOAD ("300bios.rom", 0x0000, 0x8000, CRC(53850907) SHA1(affa3c5cd8db79a1450ad8a7f405a425b251653d))
	/* 0x0c000 - 0x1ffff reserved for optional disk and fmpac roms from msx2 parent set */
	ROM_FILL (0xc000, 0x14000, 0)
	ROM_LOAD ("300ext.rom", 0x20000, 0x8000, CRC(d64da39c) SHA1(fb51c505adfbc174df94289fa894ef969f5357bc))
	ROM_LOAD ("300han.rom", 0x28000, 0x8000, CRC(e78cd87f) SHA1(47a9d9a24e4fc6f9467c6e7d61a02d45f5a753ef))
ROM_END

MSX_LAYOUT_INIT (cpc300)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (0, 1, 1, 2, ROM, 0x8000, 0x28000)
	MSX_LAYOUT_SLOT (0, 2, 0, 4, RAM_MM, 0x20000, 0x0000)   /* 128KB Mapper RAM */
	MSX_LAYOUT_SLOT (0, 3, 0, 2, ROM, 0x8000, 0x20000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_RAMIO_SET_BITS (0x80)
MSX_LAYOUT_END

/* MSX2 - Daewoo CPC-300E */

ROM_START (cpc300e)
	ROM_REGION (0x30000, "maincpu", 0)
	ROM_LOAD ("300ebios.rom", 0x0000, 0x8000, CRC(53850907) SHA1(affa3c5cd8db79a1450ad8a7f405a425b251653d))
	/* 0x0c000 - 0x1ffff reserved for optional disk and fmpac roms from msx2 parent set */
	ROM_FILL (0xc000, 0x14000, 0)
	ROM_LOAD ("300eext.rom", 0x20000, 0x8000, CRC(d64da39c) SHA1(fb51c505adfbc174df94289fa894ef969f5357bc))
	ROM_LOAD ("300ehan.rom", 0x28000, 0x4000, CRC(5afea78d) SHA1(f08c91f8c78d681e1f02eaaaaafb87ad81112b60))
ROM_END

MSX_LAYOUT_INIT (cpc300e)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (0, 1, 1, 1, ROM, 0x4000, 0x28000)
	MSX_LAYOUT_SLOT (0, 2, 0, 4, RAM_MM, 0x20000, 0x0000)   /* 128KB Mapper RAM */
	MSX_LAYOUT_SLOT (0, 3, 0, 2, ROM, 0x8000, 0x20000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_RAMIO_SET_BITS (0x80)
MSX_LAYOUT_END

/* MSX2 - Daewoo CPC-400 */
ROM_START (cpc400)
	ROM_REGION (0x50000, "maincpu", 0)
	ROM_LOAD ("400bios.rom", 0x0000, 0x8000, CRC(53850907) SHA1(affa3c5cd8db79a1450ad8a7f405a425b251653d))
	ROM_LOAD ("400disk.rom", 0x8000, 0x4000, CRC(5fa517df) SHA1(914f6ccb25d78621186001f2f5e2aaa2d628cd0c))
	/* 0x0c000 - 0x1ffff reserved for optional disk and fmpac roms from msx2 parent set */
	ROM_FILL (0xc000, 0x14000, 0)
	ROM_LOAD ("400ext.rom", 0x20000, 0x8000, CRC(2ba104a3) SHA1(b6d3649a6647fa9f6bd61efc317485a20901128f))
	ROM_LOAD ("400han.rom", 0x28000, 0x8000, CRC(a8ead5e3) SHA1(87936f808423dddfd00629056d6807b4be1dc63e))
	ROM_LOAD ("400kfn.rom", 0x30000, 0x20000, CRC(b663c605) SHA1(965f4982790f1817bcbabbb38c8777183b231a55))
ROM_END

MSX_LAYOUT_INIT (cpc400)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (0, 1, 1, 2, ROM, 0x8000, 0x28000)
	MSX_LAYOUT_SLOT (0, 2, 0, 4, RAM_MM, 0x20000, 0x0000)   /* 128KB Mapper RAM */
	MSX_LAYOUT_SLOT (0, 3, 0, 2, ROM, 0x8000, 0x20000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 1, 2, DISK_ROM2, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_KANJI (0x30000)
	MSX_LAYOUT_RAMIO_SET_BITS (0x80)
MSX_LAYOUT_END

/* MSX2 - Daewoo CPC-400S */

ROM_START (cpc400s)
	ROM_REGION (0x50000, "maincpu", 0)
	ROM_LOAD ("400sbios.rom", 0x0000, 0x8000, CRC(53850907) SHA1(affa3c5cd8db79a1450ad8a7f405a425b251653d))
	ROM_LOAD ("400sdisk.rom", 0x8000, 0x4000, CRC(5fa517df) SHA1(914f6ccb25d78621186001f2f5e2aaa2d628cd0c))
	/* 0x0c000 - 0x1ffff reserved for optional disk and fmpac roms from msx2 parent set */
	ROM_FILL (0xc000, 0x14000, 0)
	ROM_LOAD ("400sext.rom", 0x20000, 0x8000, CRC(2ba104a3) SHA1(b6d3649a6647fa9f6bd61efc317485a20901128f))
	ROM_LOAD ("400shan.rom", 0x28000, 0x8000, CRC(975e7a31) SHA1(6a50295ea35e720ba6f4ba5616c3441128b384ed))
	ROM_LOAD ("400skfn.rom", 0x30000, 0x20000, CRC(fa85368c) SHA1(30fff22e3e3d464993707488442721a5e56a9707))
ROM_END

MSX_LAYOUT_INIT (cpc400s)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (0, 1, 1, 2, ROM, 0x8000, 0x28000)
	MSX_LAYOUT_SLOT (0, 2, 0, 4, RAM_MM, 0x20000, 0x0000)   /* 128KB Mapper RAM */
	MSX_LAYOUT_SLOT (0, 3, 0, 2, ROM, 0x8000, 0x20000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 1, 2, DISK_ROM2, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_KANJI (0x30000)
	MSX_LAYOUT_RAMIO_SET_BITS (0x80)
MSX_LAYOUT_END

/* MSX2 - Gradiente Expert 2.0 */

ROM_START (expert20)
	ROM_REGION (0x28000, "maincpu", 0)
	ROM_LOAD ("exp20bios.rom", 0x0000, 0x8000, CRC(6BACDCE4) SHA1(9c43106dba3ae2829e9a11dffa9d000ed6d6454c))
	ROM_LOAD ("exp20ext.rom",  0x8000, 0x4000, CRC(08CED880) SHA1(4f2a7e0172f0214f025f23845f6e053d0ffd28e8))
	/* 0x0c000 - 0x1ffff reserved for optional disk and fmpac roms from msx2 parent set */
	ROM_FILL (0xc000, 0x14000, 0)
	ROM_LOAD ("xbasic2.rom", 0x20000, 0x4000, CRC(2825b1a0) SHA1(47370bec7ca1f0615a54eda548b07fbc0c7ef398))
	ROM_LOAD ("microsoldisk.rom", 0x24000, 0x4000, CRC(6704ef81) SHA1(a3028515ed829e900cc8deb403e17b09a38bf9b0))
ROM_END

MSX_LAYOUT_INIT (expert20)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (1, 1, 0, 1, ROM, 0x4000, 0x8000) /* EXT */
	MSX_LAYOUT_SLOT (1, 1, 1, 1, ROM, 0x4000, 0x20000) /* BASIC */
	MSX_LAYOUT_SLOT (1, 3, 1, 1, DISK_ROM, 0x4000, 0x24000) /* Microsol controller */
	MSX_LAYOUT_SLOT (2, 0, 0, 4, RAM_MM, 0x20000, 0x0000)   /* 128KB Mapper RAM */
	MSX_LAYOUT_SLOT (3, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_RAMIO_SET_BITS (0x80)
MSX_LAYOUT_END

/* MSX2 - Mitsubishi ML-G30 Model 1/Model 2 */

ROM_START (mlg30)
	ROM_REGION (0x40000, "maincpu", 0)
	ROM_LOAD ("g30bios.rom", 0x0000, 0x8000, CRC(a27c563d) SHA1(c1e46c00f1e38fc9e0ab487bf0513bd93ce61f3f))
	ROM_LOAD ("g30ext.rom", 0x8000, 0x4000, CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33))
	ROM_LOAD ("g30disk.rom", 0xc000, 0x4000, CRC(05661a3f) SHA1(e695fc0c917577a3183901a08ca9e5f9c60b8317))
	ROM_LOAD ("g30kfn.rom", 0x20000, 0x20000, CRC(d23d4d2d) SHA1(db03211b7db46899df41db2b1dfbec972109a967))
ROM_END

MSX_LAYOUT_INIT (mlg30)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)    /* Slot 2 subslot 0 */
	MSX_LAYOUT_SLOT (3, 0, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 0, 1, 2, DISK_ROM, 0x4000, 0xc000)
	MSX_LAYOUT_SLOT (3, 2, 0, 4, RAM_MM, 0x10000, 0x0000)   /* 64KB Mapper RAM */
MSX_LAYOUT_END

/* MSX2 - National FS-4500 */

ROM_START (fs4500)
	ROM_REGION (0x94000, "maincpu",0)
	ROM_LOAD ("4500bios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))
	ROM_LOAD ("4500ext.rom", 0x8000, 0x4000, CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33))
	/* 0x0c000 - 0x1ffff reserved for optional disk and fmpac roms from msx2 parent set */
	ROM_FILL (0xc000, 0x14000, 0)
	ROM_LOAD ("4500font.rom", 0x20000, 0x4000, CRC(4bd54f95) SHA1(3ce8e35790eb4689b21e14c7ecdd4b63943ee158))
	ROM_LOAD ("4500buns.rom", 0x24000, 0x8000, CRC(c9398e11) SHA1(e89ea1e8e583392e2dd9debb8a4b6a162f58ba91))
	ROM_LOAD ("4500jush.rom", 0x2c000, 0x8000, CRC(4debfd2d) SHA1(6442c1c5cece64c6dae90cc6ae3675f070d93e06))
	ROM_LOAD ("4500wor1.rom", 0x34000, 0xc000, CRC(0c8b5cfb) SHA1(3f047469b62d93904005a0ea29092e892724ce0b))
	ROM_LOAD ("4500wor2.rom", 0x40000, 0xc000, CRC(d9909451) SHA1(4c8ea05c09b40c41888fa18db065575a317fda16))
	ROM_LOAD ("4500kdr1.rom", 0x4c000, 0x4000, CRC(f8c7f0db) SHA1(df07e89fa0b1c7874f9cdf184c136f964fea4ff4))
	ROM_LOAD ("4500kdr2.rom", 0x50000, 0x4000, CRC(69e87c31) SHA1(c63db26660da96af56f8a7d3ea18544b9ae5a37c))
	ROM_LOAD ("4500kfn.rom", 0x54000, 0x20000, CRC(956dc96d) SHA1(9ed3ab6d893632b9246e91b412cd5db519e7586b))
	ROM_LOAD ("4500budi.rom", 0x74000, 0x20000, CRC(f94590f8) SHA1(1ebb06062428fcdc66808a03761818db2bba3c73))
ROM_END

MSX_LAYOUT_INIT (fs4500)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (0, 1, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (0, 2, 0, 1, ROM, 0x4000, 0x20000)
	MSX_LAYOUT_SLOT (0, 3, 1, 2, ROM, 0x8000, 0x2c000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 3, ROM, 0xc000, 0x34000)
	MSX_LAYOUT_SLOT (3, 0, 3, 1, ROM, 0x4000, 0x4c000)
	MSX_LAYOUT_SLOT (3, 1, 0, 3, ROM, 0xc000, 0x40000)
	MSX_LAYOUT_SLOT (3, 1, 3, 1, ROM, 0x4000, 0x50000)
	MSX_LAYOUT_SLOT (3, 2, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB RAM */
	MSX_LAYOUT_KANJI (0x54000)
/*  MSX_LAYOUT_BUNSETSU (0x74000) */ /* Matsushita Bunsetsu Henkan ROM must be emulated */
MSX_LAYOUT_END

/* MSX2 - National FS-4600 */

ROM_START (fs4600)
	ROM_REGION (0x170000, "maincpu",0)
	ROM_LOAD ("4600bios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))
	ROM_LOAD ("4600ext.rom", 0x8000, 0x4000, CRC(43e7a7fc) SHA1(0fbd45ef3dd7bb82d4c31f1947884f411f1ca344))
	ROM_LOAD ("4600disk.rom", 0xc000, 0x4000, CRC(ae4e65b7) SHA1(073feb8bb645d935e099afaf61e6f04f52adee42))
	/* 0x10000 - 0x1ffff reserved for optional fmpac roms from msx2 parent set */
	ROM_FILL (0x10000, 0x10000, 0)
	ROM_LOAD ("4600fon1.rom", 0x20000, 0x4000, CRC(7391389b) SHA1(31292b9ca9fe7d1d8833530f44c0a5671bfefe4e))
	ROM_LOAD ("4600fon2.rom", 0x24000, 0x4000, CRC(c3a6b445) SHA1(02155fc25c9bd23e1654fe81c74486351e1ecc28))
	ROM_LOAD ("4600kdr.rom", 0x28000, 0x8000, CRC(b2db6bf5) SHA1(3a9a942ed888dd641cddf8deada1879c454df3c6))
	ROM_LOAD ("4600kfn.rom", 0x30000, 0x20000, CRC(c61ddc5d) SHA1(5e872d5853698731a0ed22fb72dbcdfd59cd19c3))
	ROM_LOAD ("4600kf12.rom", 0x50000, 0x20000, CRC(340d1ef7) SHA1(a7a23dc01314e88381eee88b4878b39931ab4818))
	ROM_LOAD ("4600firm.rom", 0x70000, 0x100000, CRC(1df57472) SHA1(005794c10a4237de3907ba4a44d436078d3c06c2))
ROM_END

MSX_LAYOUT_INIT (fs4600)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (0, 1, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (0, 2, 0, 1, ROM, 0x4000, 0x20000)
	MSX_LAYOUT_SLOT (0, 2, 1, 2, ROM, 0x8000, 0x28000)
	MSX_LAYOUT_SLOT (0, 3, 0, 1, ROM, 0x4000, 0x24000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 1, 0, 4, ASCII16, 0x100000, 0x70000) /* National FS-4600 Mapper must be emulated */
	MSX_LAYOUT_SLOT (3, 2, 0, 4, RAM_MM, 0x20000, 0x0000)   /* 128KB Mapper RAM */
	MSX_LAYOUT_SLOT (3, 3, 1, 2, DISK_ROM2, 0x4000, 0xc000)
	MSX_LAYOUT_KANJI (0x30000)
/*  MSX_LAYOUT_KANJI_12 (0x50000) */ /* Matsushita 12 dots Kanji ROM must be emulated */
	MSX_LAYOUT_RAMIO_SET_BITS (0x80)
MSX_LAYOUT_END

/* MSX2 - National FS-4700 */

ROM_START (fs4700)
	ROM_REGION (0x94000, "maincpu",0)
	ROM_LOAD ("4700bios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))
	ROM_LOAD ("4700ext.rom", 0x8000, 0x4000, CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33))
	ROM_LOAD ("4700disk.rom", 0xc000, 0x4000, CRC(1e7d6512) SHA1(78cd7f847e77fd8cd51a647efb2725ba93f4c471))
	/* 0x10000 - 0x1ffff reserved for optional fmpac roms from msx2 parent set */
	ROM_FILL (0x10000, 0x10000, 0)
	ROM_LOAD ("4700font.rom", 0x20000, 0x4000, CRC(4bd54f95) SHA1(3ce8e35790eb4689b21e14c7ecdd4b63943ee158))
	ROM_LOAD ("4700buns.rom", 0x24000, 0x8000, CRC(c9398e11) SHA1(e89ea1e8e583392e2dd9debb8a4b6a162f58ba91))
	ROM_LOAD ("4700jush.rom", 0x2c000, 0x8000, CRC(4debfd2d) SHA1(6442c1c5cece64c6dae90cc6ae3675f070d93e06))
	ROM_LOAD ("4700wor1.rom", 0x34000, 0xc000, CRC(5f39a727) SHA1(f5af1d2a8bcf247f78847e1a9d995e581df87e8e))
	ROM_LOAD ("4700wor2.rom", 0x40000, 0xc000, CRC(d9909451) SHA1(4c8ea05c09b40c41888fa18db065575a317fda16))
	ROM_LOAD ("4700kdr1.rom", 0x4c000, 0x4000, CRC(f8c7f0db) SHA1(df07e89fa0b1c7874f9cdf184c136f964fea4ff4))
	ROM_LOAD ("4700kdr2.rom", 0x50000, 0x4000, CRC(69e87c31) SHA1(c63db26660da96af56f8a7d3ea18544b9ae5a37c))
	ROM_LOAD ("4700kfn.rom", 0x54000, 0x20000, CRC(956dc96d) SHA1(9ed3ab6d893632b9246e91b412cd5db519e7586b))
	ROM_LOAD ("4700budi.rom", 0x74000, 0x20000, CRC(f94590f8) SHA1(1ebb06062428fcdc66808a03761818db2bba3c73))
ROM_END

MSX_LAYOUT_INIT (fs4700)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (0, 1, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (0, 2, 0, 1, ROM, 0x4000, 0x20000)
	MSX_LAYOUT_SLOT (0, 3, 1, 2, ROM, 0x8000, 0x2c000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 3, ROM, 0xc000, 0x34000)
	MSX_LAYOUT_SLOT (3, 0, 3, 1, ROM, 0x4000, 0x4c000)
	MSX_LAYOUT_SLOT (3, 1, 0, 3, ROM, 0xc000, 0x40000)
	MSX_LAYOUT_SLOT (3, 1, 3, 1, ROM, 0x4000, 0x50000)
	MSX_LAYOUT_SLOT (3, 2, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB RAM */
	MSX_LAYOUT_SLOT (3, 3, 1, 2, DISK_ROM2, 0x4000, 0xc000)
	MSX_LAYOUT_KANJI (0x54000)
/*  MSX_LAYOUT_BUNSETSU (0x74000) */ /* Matsushita Bunsetsu Henkan ROM must be emulated */
MSX_LAYOUT_END

/* MSX2 - National FS-5000 */

ROM_START (fs5000)
	ROM_REGION (0x50000, "maincpu",0)
	ROM_LOAD ("5000bios.rom", 0x0000, 0x8000, CRC(a44ea707) SHA1(59967765d6e9328909dee4dac1cbe4cf9d47d315))
	ROM_LOAD ("5000ext.rom", 0x8000, 0x4000, CRC(43e7a7fc) SHA1(0fbd45ef3dd7bb82d4c31f1947884f411f1ca344))
	ROM_LOAD ("5000disk.rom", 0xc000, 0x4000, CRC(ae4e65b7) SHA1(073feb8bb645d935e099afaf61e6f04f52adee42))
	/* 0x10000 - 0x1ffff reserved for optional fmpac roms from msx2 parent set */
	ROM_FILL (0x10000, 0x10000, 0xff)
	ROM_LOAD ("5000rtc.rom", 0x20000, 0x8000, CRC(03351598) SHA1(98bbfa3ab07b7a5cad55d7ddf7cbd9440caa2a86))
	ROM_LOAD ("5000kdr.rom", 0x28000, 0x8000, CRC(b2db6bf5) SHA1(3a9a942ed888dd641cddf8deada1879c454df3c6))
	ROM_LOAD ("5000kfn.rom", 0x30000, 0x20000, CRC(c61ddc5d) SHA1(5e872d5853698731a0ed22fb72dbcdfd59cd19c3))
ROM_END

MSX_LAYOUT_INIT (fs5000)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (0, 1, 0, 4, ROM, 0x10000, 0x10000)
	MSX_LAYOUT_SLOT (0, 2, 0, 4, ROM, 0x10000, 0x10000)
	MSX_LAYOUT_SLOT (0, 3, 0, 4, ROM, 0x10000, 0x10000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 0, 1, 2, ROM, 0x8000, 0x28000)
	MSX_LAYOUT_SLOT (3, 1, 1, 1, ROM, 0x8000, 0x20000)
	MSX_LAYOUT_SLOT (3, 2, 0, 4, RAM_MM, 0x20000, 0x0000)   /* 128KB Mapper RAM */
	MSX_LAYOUT_SLOT (3, 3, 1, 2, DISK_ROM2, 0x4000, 0xc000)
	MSX_LAYOUT_KANJI (0x30000)
	MSX_LAYOUT_RAMIO_SET_BITS (0x80)
MSX_LAYOUT_END

/* MSX2 - National FS-5500F1/F2*/

ROM_START (fs5500)
	ROM_REGION (0x50000, "maincpu",0)
	ROM_LOAD ("5500bios.rom", 0x0000, 0x8000, CRC(5bf38e13) SHA1(44e0dd215b2a9f0770dd76fb49187c05b083eed9))
	ROM_LOAD ("5500ext.rom", 0x8000, 0x4000, CRC(3c42c367) SHA1(4be8371f3b03e70ddaca495958345f3c4f8e2d36))
	ROM_LOAD ("5500disk.rom", 0xc000, 0x4000, CRC(1e7d6512) SHA1(78cd7f847e77fd8cd51a647efb2725ba93f4c471))
	/* 0x10000 - 0x1ffff reserved for optional fmpac roms from msx2 parent set */
	ROM_FILL (0x10000, 0x10000, 0xff)
	ROM_LOAD ("5500imp.rom", 0x20000, 0x8000, CRC(6173a88c) SHA1(b677a861b67e8763a11d5dcf52416b42493ade57))
	ROM_LOAD ("5500kdr.rom", 0x28000, 0x8000, CRC(b2db6bf5) SHA1(3a9a942ed888dd641cddf8deada1879c454df3c6))
	ROM_LOAD ("5500kfn.rom", 0x30000, 0x20000, CRC(956dc96d) SHA1(9ed3ab6d893632b9246e91b412cd5db519e7586b))
ROM_END

MSX_LAYOUT_INIT (fs5500)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (0, 1, 0, 4, ROM, 0x10000, 0x10000)
	MSX_LAYOUT_SLOT (0, 2, 0, 4, ROM, 0x10000, 0x10000)
	MSX_LAYOUT_SLOT (0, 3, 0, 4, ROM, 0x10000, 0x10000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 0, 1, 2, ROM, 0x8000, 0x28000)
	MSX_LAYOUT_SLOT (3, 1, 1, 1, ROM, 0x8000, 0x20000)
	MSX_LAYOUT_SLOT (3, 2, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB RAM */
	MSX_LAYOUT_SLOT (3, 3, 1, 2, DISK_ROM2, 0x4000, 0xc000)
	MSX_LAYOUT_KANJI (0x30000)
MSX_LAYOUT_END


/* MSX2 - Panasonic FS-A1 */

ROM_START (fsa1)
	ROM_REGION (0x30000, "maincpu",0)
	ROM_LOAD ("a1bios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))
	ROM_LOAD ("a1ext.rom", 0x8000, 0x4000, CRC(43e7a7fc) SHA1(0fbd45ef3dd7bb82d4c31f1947884f411f1ca344))
	/* 0x0c000 - 0x1ffff reserved for optional disk and fmpac roms from msx2 parent set */
	ROM_FILL (0xc000, 0x14000, 0)
	ROM_LOAD ("a1desk1.rom", 0x20000, 0x8000, CRC(99c48147) SHA1(63098f27beac9eca6b39d837d2a552395df33fe1))
	ROM_LOAD ("a1desk2.rom", 0x28000, 0x8000, CRC(7f6f4aa1) SHA1(7f5b76605e3d898cc4b5aacf1d7682b82fe84353))
ROM_END

MSX_LAYOUT_INIT (fsa1)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, RAM, 0x10000, 0x0000)  /* 64 KB RAM */
	MSX_LAYOUT_SLOT (3, 1, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 2, 1, 2, ROM, 0x8000, 0x20000)
	MSX_LAYOUT_SLOT (3, 3, 1, 2, ROM, 0x8000, 0x28000)
MSX_LAYOUT_END

/* MSX2 - Panasonic FS-A1 (a) */

ROM_START (fsa1a)
	ROM_REGION (0x30000, "maincpu",0)
	ROM_LOAD ("a1bios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))
	ROM_LOAD ("a1ext.rom", 0x8000, 0x4000, CRC(43e7a7fc) SHA1(0fbd45ef3dd7bb82d4c31f1947884f411f1ca344))
	/* 0x0c000 - 0x1ffff reserved for optional disk and fmpac roms from msx2 parent set */
	ROM_FILL (0xc000, 0x14000, 0)
	ROM_LOAD ("a1desk1a.rom", 0x20000, 0x8000, CRC(25b5b170) SHA1(d9307bfdaab1312d25e38af7c0d3a7671a9f716b))
	ROM_LOAD ("a1desk2.rom", 0x28000, 0x8000, CRC(7f6f4aa1) SHA1(7f5b76605e3d898cc4b5aacf1d7682b82fe84353))
ROM_END

MSX_LAYOUT_INIT (fsa1a)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB RAM */
	MSX_LAYOUT_SLOT (3, 1, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 2, 1, 2, ROM, 0x8000, 0x20000)
	MSX_LAYOUT_SLOT (3, 3, 1, 2, ROM, 0x8000, 0x28000)
MSX_LAYOUT_END

/* MSX2 - Panasonic FS-A1F */

ROM_START (fsa1f)
	ROM_REGION (0x50000, "maincpu",0)
	ROM_LOAD ("a1fbios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))
	ROM_LOAD ("a1fext.rom", 0x8000, 0x4000, CRC(43e7a7fc) SHA1(0fbd45ef3dd7bb82d4c31f1947884f411f1ca344))
	ROM_LOAD ("a1fdisk.rom", 0xc000, 0x4000, CRC(e25cacca) SHA1(607cfca605eaf82e3efa33459d6583efb7ecc13b))
	/* 0x10000 - 0x1ffff reserved for optional fmpac roms from msx2 parent set */
	ROM_FILL (0x10000, 0x10000, 0)
	ROM_LOAD ("a1fkdr.rom", 0x20000, 0x8000, CRC(2dbea5ec) SHA1(ea35cc2cad9cfdf56cae224d8ee41579de37f000))
	ROM_LOAD ("a1fcock.rom", 0x28000, 0x8000, CRC(5c2948cd) SHA1(4a99f2444f29c2b642efd6f084081d6fd96bfa9b))
	ROM_LOAD ("a1fkfn.rom", 0x30000, 0x20000, CRC(c61ddc5d) SHA1(5e872d5853698731a0ed22fb72dbcdfd59cd19c3))
ROM_END

MSX_LAYOUT_INIT (fsa1f)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, RAM_MM, 0x10000, 0x0000)   /* 64KB Mapper RAM */
	MSX_LAYOUT_SLOT (3, 1, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 1, 1, 2, ROM, 0x8000, 0x20000)
/*  MSX_LAYOUT_SLOT (3, 2, 1, 2, DISK_ROM, 0x4000, 0xc000) */ /* FDC Emulation of TC8566AF must be emulated */
	MSX_LAYOUT_SLOT (3, 3, 1, 2, ROM, 0x8000, 0x28000)
	MSX_LAYOUT_KANJI (0x30000)
	MSX_LAYOUT_RAMIO_SET_BITS (0x80)
MSX_LAYOUT_END

/* MSX2 - Panasonic FS-A1FM */

ROM_START (fsa1fm)
	ROM_REGION (0x180000, "maincpu",0)
	ROM_LOAD ("a1fmbios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))
	ROM_LOAD ("a1fmext.rom", 0x8000, 0x4000, CRC(ad295b5d) SHA1(d552319a19814494e3016de4b8f010e8f7b97e02))
	ROM_LOAD ("a1fmdisk.rom", 0xc000, 0x4000, CRC(e25cacca) SHA1(607cfca605eaf82e3efa33459d6583efb7ecc13b))
	/* 0x10000 - 0x1ffff reserved for optional fmpac roms from msx2 parent set */
	ROM_FILL (0x10000, 0x10000, 0)
	ROM_LOAD ("a1fmkfn.rom", 0x40000, 0x20000, CRC(c61ddc5d) SHA1(5e872d5853698731a0ed22fb72dbcdfd59cd19c3))
	ROM_LOAD ("a1fmkf12.rom", 0x60000, 0x20000, CRC(340d1ef7) SHA1(a7a23dc01314e88381eee88b4878b39931ab4818))
	ROM_LOAD ("a1fmfirm.rom", 0x80000, 0x100000, CRC(8ce0ece7) SHA1(f89e3d8f3b6855c29d71d3149cc762e0f6918ad5))
ROM_END

MSX_LAYOUT_INIT (fsa1fm)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, RAM_MM, 0x10000, 0x0000)   /* 64KB Mapper RAM */
	MSX_LAYOUT_SLOT (3, 1, 0, 1, ROM, 0x4000, 0x8000)
/*  MSX_LAYOUT_SLOT (3, 1, 1, 2, MODEM_ROM, 0x20000, 0x20000) */ /* Modem Mapper of FS-CM1/A1FM must be emulated */
/*  MSX_LAYOUT_SLOT (3, 2, 1, 2, DISK_ROM, 0x4000, 0xc000) */ /* FDC Emulation of TC8566AF must be emulated */
/*  MSX_LAYOUT_SLOT (3, 3, 0, 4, FSA1FM_ROM, 0x100000, 0x80000) */ /* Panasonic FS-A1FM Mapper must be emulated */
	MSX_LAYOUT_KANJI (0x40000)
/*  MSX_LAYOUT_KANJI_12 (0x60000) */ /* Matsushita 12 dots Kanji ROM must be emulated */
	MSX_LAYOUT_RAMIO_SET_BITS (0x80)
MSX_LAYOUT_END

/* MSX2 - Panasonic FS-A1MK2 */

ROM_START (fsa1mk2)
	ROM_REGION (0x34000, "maincpu",0)
	ROM_LOAD ("a1mkbios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))
	ROM_LOAD ("a1mk2ext.rom", 0x8000, 0x4000, CRC(43e7a7fc) SHA1(0fbd45ef3dd7bb82d4c31f1947884f411f1ca344))
	/* 0x0c000 - 0x1ffff reserved for optional disk and fmpac roms from msx2 parent set */
	ROM_FILL (0xc000, 0x14000, 0)
	ROM_LOAD ("a1mkcoc1.rom", 0x20000, 0x8000, CRC(0eda3f57) SHA1(2752cd89754c05abdf7c23cba132d38e3ef0f27d))
	ROM_LOAD ("a1mkcoc2.rom", 0x28000, 0x4000, CRC(756d7128) SHA1(e194d290ebfa4595ce0349ea2fc15442508485b0))
	ROM_LOAD ("a1mkcoc3.rom", 0x2c000, 0x8000, CRC(c1945676) SHA1(a3f4e2e4934074925d775afe30ac72f150ede543))
ROM_END

MSX_LAYOUT_INIT (fsa1mk2)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, RAM_MM, 0x10000, 0x0000)   /* 64 KB Mapper RAM */
	MSX_LAYOUT_SLOT (3, 1, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 1, 1, 2, ROM, 0x8000, 0x20000)
	MSX_LAYOUT_SLOT (3, 2, 1, 1, ROM, 0x4000, 0x28000)
	MSX_LAYOUT_SLOT (3, 3, 1, 2, ROM, 0x8000, 0x2c000)
	MSX_LAYOUT_RAMIO_SET_BITS (0x80)
MSX_LAYOUT_END

/* MSX2 - Philips NMS-8220 - 2 possible sets (/00 /16) */

ROM_START (nms8220)
	ROM_REGION (0x20000, "maincpu", 0)
	ROM_LOAD ("8220bios.rom.u14", 0x0000, 0x8000, BAD_DUMP CRC(6cdaf3a5) SHA1(6103b39f1e38d1aa2d84b1c3219c44f1abb5436e))
	ROM_LOAD ("8220ext.rom.u14", 0x8000, 0x4000, BAD_DUMP CRC(06e4f5e6) SHA1(f5eb0a396097572589f2a6efeed045044e9425e4))
	ROM_LOAD ("8220pen.rom.u13", 0xc000, 0x4000, CRC(3d38c53e) SHA1(cb754aed85b3e97a7d3c5894310df7ca18f89f41))
	/* 0x10000 - 0x1ffff reserved for optional fmpac roms from msx2 parent set */
	ROM_FILL (0x10000, 0x10000, 0)
ROM_END

MSX_LAYOUT_INIT (nms8220)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 2, 0, 4, RAM_MM, 0x10000, 0x0000)   /* 64KB Mapper RAM */
	MSX_LAYOUT_SLOT (3, 3, 1, 1, ROM, 0x4000, 0xc000)
	MSX_LAYOUT_RAMIO_SET_BITS (0xf8)
MSX_LAYOUT_END

/* MSX2 - Philips NMS-8220 (a) */

ROM_START (nms8220a)
	ROM_REGION (0x20000, "maincpu", 0)
	ROM_LOAD ("8220bios.rom.u14", 0x0000, 0x8000, BAD_DUMP CRC(6cdaf3a5) SHA1(6103b39f1e38d1aa2d84b1c3219c44f1abb5436e))
	ROM_LOAD ("8220ext.rom.u14", 0x8000, 0x4000, BAD_DUMP CRC(06e4f5e6) SHA1(f5eb0a396097572589f2a6efeed045044e9425e4))
	ROM_LOAD ("8220pena.rom.u13", 0xc000, 0x4000, CRC(17817b5a) SHA1(5df95d033ae70b107697b69470126ce1b7ae9eb5))
	/* 0x10000 - 0x1ffff reserved for optional fmpac rom from msx2 parent set */
	ROM_FILL (0x10000, 0x10000, 0)
ROM_END

MSX_LAYOUT_INIT (nms8220a)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 2, 0, 4, RAM_MM, 0x10000, 0x0000)   /* 64KB Mapper RAM */
	MSX_LAYOUT_SLOT (3, 3, 1, 1, ROM, 0x4000, 0xc000)
	MSX_LAYOUT_RAMIO_SET_BITS (0xf8)
MSX_LAYOUT_END

/* MSX2 - Philips NMS-8245 - 2 possible sets (/00 /16) */

ROM_START (nms8245)
	ROM_REGION (0x40000, "maincpu", 0)
	/* 0x10000 - 0x1ffff reserved for optional fmpac rom from msx2 parent set */
	ROM_FILL (0x10000, 0x10000, 0)
	ROM_LOAD ("nms8245.u7", 0x20000, 0x20000, BAD_DUMP CRC(0c827d5f) SHA1(064e706cb1f12b99b329944ceeedc0efc3b2d9be))
ROM_END

MSX_LAYOUT_INIT (nms8245)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x20000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 1, ROM, 0x4000, 0x28000)
	MSX_LAYOUT_SLOT (3, 2, 0, 4, RAM_MM, 0x20000, 0x0000)   /* 128KB Mapper RAM */
	MSX_LAYOUT_SLOT (3, 3, 1, 2, DISK_ROM, 0x4000, 0x2c000)
	MSX_LAYOUT_RAMIO_SET_BITS (0xf8)
MSX_LAYOUT_END

/* MSX2 - Philips NMS-8245F */

ROM_START (nms8245f)
	ROM_REGION (0x40000, "maincpu", 0)
	/* 0x10000 - 0x1ffff reserved for optional fmpac rom from msx2 parent set */
	ROM_FILL (0x10000, 0x10000, 0)
	ROM_LOAD ("nms8245.u7", 0x20000, 0x20000, BAD_DUMP CRC(0c827d5f) SHA1(064e706cb1f12b99b329944ceeedc0efc3b2d9be))
ROM_END

MSX_LAYOUT_INIT (nms8245f)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x30000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 1, ROM, 0x4000, 0x38000)
	MSX_LAYOUT_SLOT (3, 2, 0, 4, RAM_MM, 0x20000, 0x0000)   /* 128KB Mapper RAM */
	MSX_LAYOUT_SLOT (3, 3, 1, 2, DISK_ROM, 0x4000, 0x3c000)
	MSX_LAYOUT_RAMIO_SET_BITS (0xf8)
MSX_LAYOUT_END

/* MSX2 - Philips NMS-8250 */
/* Labels taken from an NMS-8250/00 */

ROM_START (nms8250)
	ROM_REGION (0x20000, "maincpu", 0)
	ROM_LOAD ("d23c256eac.ic119", 0x0000, 0x8000, CRC(6cdaf3a5) SHA1(6103b39f1e38d1aa2d84b1c3219c44f1abb5436e))
	ROM_LOAD ("d23128ec.ic118", 0x8000, 0x4000, CRC(66237ecf) SHA1(5c1f9c7fb655e43d38e5dd1fcc6b942b2ff68b02))
	ROM_LOAD ("jq00014.ic117", 0xc000, 0x04000, CRC(ca3307d3) SHA1(c3efedda7ab947a06d9345f7b8261076fa7ceeef))
	/* 0x10000 - 0x1ffff reserved for optional fmpac roms from msx2 parent set */
	ROM_FILL (0x10000, 0x10000, 0)
ROM_END

MSX_LAYOUT_INIT (nms8250)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 2, 0, 4, RAM_MM, 0x20000, 0x0000)   /* 128KB Mapper RAM */
	MSX_LAYOUT_SLOT (3, 3, 1, 2, DISK_ROM, 0x4000, 0xc000)
	MSX_LAYOUT_RAMIO_SET_BITS (0xf8)
MSX_LAYOUT_END

/* MSX2 - Philips NMS-8250J */

ROM_START (nms8250j)
	ROM_REGION (0x240000, "maincpu", 0)
	ROM_LOAD ("8250jbios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))
	ROM_LOAD ("8250jext.rom", 0x8000, 0x4000, CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33))
	ROM_LOAD ("8250jdisk.rom", 0xc000, 0x4000, CRC(ca3307d3) SHA1(c3efedda7ab947a06d9345f7b8261076fa7ceeef))
	ROM_LOAD ("8250jkfn.rom", 0x20000, 0x20000, CRC(5a59926e) SHA1(6acaf2eeb57f65f7408235d5e07b7563229de799))
ROM_END

MSX_LAYOUT_INIT (nms8250j)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 2, 0, 4, RAM_MM, 0x20000, 0x0000)   /* 128KB Mapper RAM */
	MSX_LAYOUT_SLOT (3, 3, 1, 2, DISK_ROM, 0x4000, 0xc000)
MSX_LAYOUT_END

/* MSX2 - Philips NMS-8255 */

ROM_START (nms8255)
	ROM_REGION (0x20000, "maincpu", 0)
	ROM_LOAD ("8255bios.rom.ic119", 0x0000, 0x8000, CRC(6cdaf3a5) SHA1(6103b39f1e38d1aa2d84b1c3219c44f1abb5436e))
	ROM_LOAD ("8255ext.rom.ic118", 0x8000, 0x4000, CRC(66237ecf) SHA1(5c1f9c7fb655e43d38e5dd1fcc6b942b2ff68b02))
	ROM_LOAD ("8255disk.rom.ic117", 0xc000, 0x04000, CRC(ca3307d3) SHA1(c3efedda7ab947a06d9345f7b8261076fa7ceeef))
	/* 0x10000 - 0x1ffff reserved for optional fmpac roms from msx2 parent set */
	ROM_FILL (0x10000, 0x10000, 0)
ROM_END

MSX_LAYOUT_INIT (nms8255)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 2, 0, 4, RAM_MM, 0x20000, 0x0000)   /* 128KB Mapper RAM */
	MSX_LAYOUT_SLOT (3, 3, 1, 2, DISK_ROM, 0x4000, 0xc000)
	MSX_LAYOUT_RAMIO_SET_BITS (0xf8)
MSX_LAYOUT_END

/* MSX2 - Philips NMS-8280 - 2 possible sets (/00 /16) */

ROM_START (nms8280)
	ROM_REGION (0x20000, "maincpu", 0)
	ROM_LOAD ("8280bios.rom.ic119", 0x0000, 0x8000, CRC(6cdaf3a5) SHA1(6103b39f1e38d1aa2d84b1c3219c44f1abb5436e))
	ROM_LOAD ("8280ext.rom.ic118", 0x8000, 0x4000, CRC(66237ecf) SHA1(5c1f9c7fb655e43d38e5dd1fcc6b942b2ff68b02))
	ROM_LOAD ("8280disk.rom.ic117", 0xc000, 0x04000, CRC(ca3307d3) SHA1(c3efedda7ab947a06d9345f7b8261076fa7ceeef))
	/* 0x10000 - 0x1ffff reserved for optional fmpac roms from msx2 parent set */
	ROM_FILL (0x10000, 0x10000, 0)
ROM_END

MSX_LAYOUT_INIT (nms8280)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 2, 0, 4, RAM_MM, 0x20000, 0x0000)   /* 128KB Mapper RAM */
	MSX_LAYOUT_SLOT (3, 3, 1, 2, DISK_ROM, 0x4000, 0xc000)
	MSX_LAYOUT_RAMIO_SET_BITS (0xf8)
MSX_LAYOUT_END

/* MSX2 - Philips NMS-8280G */

ROM_START (nms8280g)
	ROM_REGION (0x20000, "maincpu", 0)
	ROM_LOAD ("8280gbios.rom.ic119", 0x0000, 0x8000, CRC(8fa060e2) SHA1(b17d9bea0eb16a1aa2d0ccbd7c9488da9f57698e))
	ROM_LOAD ("8280gext.rom.ic118", 0x8000, 0x4000, CRC(41e36d03) SHA1(4ab7b2030d022f5486abaab22aaeaf8aa23e05f3))
	ROM_LOAD ("8280gdisk.rom.ic117", 0xc000, 0x04000, CRC(d0beebb8) SHA1(d1001f93c87ff7fb389e418e33bf7bc81bdbb65f))
	/* 0x10000 - 0x1ffff reserved for optional fmpac roms from msx2 parent set */
	ROM_FILL (0x10000, 0x10000, 0)
ROM_END

MSX_LAYOUT_INIT (nms8280g)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 2, 0, 4, RAM_MM, 0x20000, 0x0000)   /* 128KB Mapper RAM */
	MSX_LAYOUT_SLOT (3, 3, 1, 2, DISK_ROM, 0x4000, 0xc000)
	MSX_LAYOUT_RAMIO_SET_BITS (0xf8)
MSX_LAYOUT_END

/* MSX2 - Philips VG-8230 (u11 - exp, u12 - basic, u13 - disk */

ROM_START (vg8230)
	ROM_REGION (0x20000, "maincpu", 0)
	ROM_LOAD ("8230bios.rom.u12", 0x0000, 0x8000, CRC(b31c851d) SHA1(0de3c802057560560a03d7965fcc4cff69f8575c))
	ROM_LOAD ("8230ext.rom.u11", 0x8000, 0x4000, CRC(8f84f783) SHA1(3288894e1be6af705871499b23c85732dbc40993))
	ROM_LOAD ("8230disk.rom.u13", 0xc000, 0x4000, CRC(77c4e5bc) SHA1(849f93867ff7846b27f84d0be418569faf058ac2))
	/* 0x10000 - 0x1ffff reserved for optional fmpac rom from msx2 parent set */
	ROM_FILL (0x10000, 0x10000, 0)
ROM_END

MSX_LAYOUT_INIT (vg8230)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 2, 0, 4, RAM_MM, 0x10000, 0x0000)   /* 64KB Mapper RAM */
	MSX_LAYOUT_SLOT (3, 3, 1, 2, DISK_ROM, 0x4000, 0xc000)
	MSX_LAYOUT_RAMIO_SET_BITS (0xf8)
MSX_LAYOUT_END

/* MSX2 - Philips VG-8230J */

ROM_START (vg8230j)
	ROM_REGION (0x40000, "maincpu", 0)
	ROM_LOAD ("8230jbios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))
	ROM_LOAD ("8230jext.rom", 0x8000, 0x4000, CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33))
	ROM_LOAD ("8230jdisk.rom", 0xc000, 0x4000, CRC(7639758a) SHA1(0f5798850d11b316a4254b222ca08cc4ad6d4da2))
	ROM_LOAD ("8230jkfn.rom", 0x20000, 0x20000, CRC(5a59926e) SHA1(6acaf2eeb57f65f7408235d5e07b7563229de799))
ROM_END

MSX_LAYOUT_INIT (vg8230j)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 2, 0, 4, RAM_MM, 0x20000, 0x0000)   /* 128KB Mapper RAM */
	MSX_LAYOUT_SLOT (3, 3, 1, 2, DISK_ROM, 0x4000, 0xc000)
MSX_LAYOUT_END

/* MSX2 - Philips VG-8235 3 psosible basic and ext roms (/00 /02 /19) */

ROM_START (vg8235)
	ROM_REGION (0x20000, "maincpu", 0)
	ROM_LOAD ("8235bios.rom.u48", 0x0000, 0x8000, CRC(6cdaf3a5) SHA1(6103b39f1e38d1aa2d84b1c3219c44f1abb5436e))
	ROM_LOAD ("8235ext.rom.u49", 0x8000, 0x4000, CRC(66237ecf) SHA1(5c1f9c7fb655e43d38e5dd1fcc6b942b2ff68b02))
	ROM_LOAD ("8235disk.rom.u50", 0xc000, 0x4000, CRC(51daeb25) SHA1(8954e59aa79310c7b719ecf0cde1e82fb731dcd1))
	/* 0x10000 - 0x1ffff reserved for optional fmpac rom from msx2 parent set */
	ROM_FILL (0x10000, 0x10000, 0)
ROM_END

MSX_LAYOUT_INIT (vg8235)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 2, 0, 4, RAM_MM, 0x20000, 0x0000)   /* 128KB Mapper RAM */
	MSX_LAYOUT_SLOT (3, 3, 1, 2, DISK_ROM, 0x4000, 0xc000)
	MSX_LAYOUT_RAMIO_SET_BITS (0xf8)
MSX_LAYOUT_END

/* MSX2 - Philips VG-8235F */

ROM_START (vg8235f)
	ROM_REGION (0x20000, "maincpu", 0)
	ROM_LOAD ("8235fbios.rom.u48", 0x0000, 0x8000, CRC(c0577a50) SHA1(3926cdd91fa89657a811463e48cfbdb350676e51))
	ROM_LOAD ("8235fext.rom.u49", 0x8000, 0x4000, CRC(e235d5c8) SHA1(792e6b2814ab783d06c7576c1e3ccd6a9bbac34a))
	ROM_LOAD ("8235fdisk.rom.u50", 0xc000, 0x4000, CRC(77c4e5bc) SHA1(849f93867ff7846b27f84d0be418569faf058ac2))
	/* 0x10000 - 0x1ffff reserved for optional fmpac rom from msx2 parent set */
	ROM_FILL (0x10000, 0x10000, 0)
ROM_END

MSX_LAYOUT_INIT (vg8235f)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 2, 0, 4, RAM_MM, 0x20000, 0x0000)   /* 128KB Mapper RAM */
	MSX_LAYOUT_SLOT (3, 3, 1, 2, DISK_ROM, 0x4000, 0xc000)
	MSX_LAYOUT_RAMIO_SET_BITS (0xf8)
MSX_LAYOUT_END

/* MSX2 - Philips VG-8240 */

ROM_START (vg8240)
	ROM_REGION (0x20000, "maincpu", 0)
	ROM_LOAD ("8240bios.rom", 0x0000, 0x8000, CRC(6cdaf3a5) SHA1(6103b39f1e38d1aa2d84b1c3219c44f1abb5436e))
	ROM_LOAD ("8240ext.rom", 0x8000, 0x4000, CRC(66237ecf) SHA1(5c1f9c7fb655e43d38e5dd1fcc6b942b2ff68b02))
	ROM_LOAD ("8240disk.rom", 0xc000, 0x4000, CRC(ca3307d3) SHA1(c3efedda7ab947a06d9345f7b8261076fa7ceeef))
	/* 0x10000 - 0x1ffff reserved for optional fmpac rom from msx2 parent set */
	ROM_FILL (0x10000, 0x10000, 0)
ROM_END

MSX_LAYOUT_INIT (vg8240)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 2, 0, 4, RAM_MM, 0x10000, 0x0000)   /* 64KB Mapper RAM */
	MSX_LAYOUT_SLOT (3, 3, 1, 2, DISK_ROM, 0x4000, 0xc000)
	MSX_LAYOUT_RAMIO_SET_BITS (0xf8)
MSX_LAYOUT_END

/* MSX2 - Sanyo MPC-2300 */

ROM_START (mpc2300)
	ROM_REGION (0x20000, "maincpu", 0)
	ROM_LOAD ("2300bios.rom", 0x0000, 0x8000, CRC(e7d08e29) SHA1(0f851ee7a1cf79819f61cc89e9948ee72a413802))
	ROM_LOAD ("2300ext.rom", 0x8000, 0x4000, CRC(3d7dc718) SHA1(e1f834b28c3ee7c9f79fe6fbf2b23c8a0617892b))
	/* 0x0c000 - 0x1ffff reserved for optional disk and fmpac roms from msx2 parent set */
	ROM_FILL (0xc000, 0x14000, 0)
ROM_END

MSX_LAYOUT_INIT (mpc2300)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, RAM_MM, 0x20000, 0x0000)   /* 128KB?? Mapper RAM */
	MSX_LAYOUT_SLOT (3, 1, 0, 1, ROM, 0x4000, 0x8000)
MSX_LAYOUT_END

/* MSX2 - Sanyo Wavy MPC-25FD */

ROM_START (mpc25fd)
	ROM_REGION (0x20000, "maincpu", 0)
	ROM_LOAD ("25fdbios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))
	ROM_LOAD ("25fdext.rom", 0x8000, 0x4000, CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33))
	ROM_LOAD ("25fddisk.rom", 0xc000, 0x4000, CRC(38454059) SHA1(58ac78bba29a06645ca8d6a94ef2ac68b743ad32))
ROM_END

MSX_LAYOUT_INIT (mpc25fd)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 3, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 1, 1, 1, DISK_ROM, 0x4000, 0xc000)
	MSX_LAYOUT_SLOT (3, 2, 0, 0, RAM_MM, 0x10000, 0x0000)   /* 128KB?? RAM */
MSX_LAYOUT_END

/* MSX2 - Sanyo Wavy PHC-23 */

ROM_START (phc23)
	ROM_REGION (0x20000, "maincpu",0)
	ROM_LOAD ("23bios.rom", 0x0000, 0x8000, CRC(ba81b3dd) SHA1(4ce41fcc1a603411ec4e99556409c442078f0ecf))
	ROM_LOAD ("23ext.rom", 0x8000, 0x4000, CRC(90ca25b5) SHA1(fd9fa78bac25aa3c0792425b21d14e364cf7eea4))
	/* 0x0c000 - 0x1ffff reserved for optional disk and fmpac roms from msx2 parent set */
	ROM_FILL (0xc000, 0x14000, 0)
ROM_END

MSX_LAYOUT_INIT (phc23)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 2, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB RAM */
MSX_LAYOUT_END

/* MSX2 - Sharp Epcom HotBit 2.0 */

ROM_START (hotbit20)
	ROM_REGION (0x28000, "maincpu", 0)
	ROM_LOAD ("hb2bios.rom", 0x0000, 0x8000, CRC(0160e8c9) SHA1(d0cfc35f22b150a1cb10decae4841dfe63b78251))
	ROM_LOAD ("hb2ext.rom",  0x8000, 0x4000, CRC(08ced880) SHA1(4f2a7e0172f0214f025f23845f6e053d0ffd28e8))
	/* 0x0c000 - 0x1ffff reserved for optional disk and fmpac roms from msx2 parent set */
	ROM_FILL (0xc000, 0x14000, 0)
	ROM_LOAD ("xbasic2.rom", 0x20000, 0x4000, CRC(2825b1a0) SHA1(47370bec7ca1f0615a54eda548b07fbc0c7ef398))
	ROM_LOAD ("microsoldisk.rom", 0x24000, 0x4000, CRC(6704ef81) SHA1(a3028515ed829e900cc8deb403e17b09a38bf9b0))
ROM_END

MSX_LAYOUT_INIT (hotbit20)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (1, 1, 0, 1, ROM, 0x4000, 0x8000) /* EXT */
	MSX_LAYOUT_SLOT (1, 1, 1, 1, ROM, 0x4000, 0x20000) /* BASIC */
	MSX_LAYOUT_SLOT (1, 3, 1, 1, DISK_ROM, 0x4000, 0x24000) /* Microsol controller */
	MSX_LAYOUT_SLOT (2, 0, 0, 4, RAM_MM, 0x20000, 0x0000)   /* 128KB Mapper RAM */
	MSX_LAYOUT_SLOT (3, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_RAMIO_SET_BITS (0x80)
MSX_LAYOUT_END

/* MSX2 - Sony HB-F1 */

ROM_START (hbf1)
	ROM_REGION (0x34000, "maincpu",0)
	ROM_LOAD ("f1bios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))
	ROM_LOAD ("f1ext.rom", 0x8000, 0x4000, CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33))
	/* 0x0c000 - 0x1ffff reserved for optional disk and fmpac roms from msx2 parent set */
	ROM_FILL (0xc000, 0x14000, 0)
	ROM_LOAD ("f1note1.rom", 0x20000, 0x4000, CRC(84810ea8) SHA1(9db72bb78792595a12499c821048504dc96ef848))
	ROM_LOAD ("f1note2.rom", 0x24000, 0x8000, CRC(e32e5ee0) SHA1(aa78fc9bcd2343f84cf790310a768ee47f90c841))
	ROM_LOAD ("f1note3.rom", 0x2c000, 0x8000, CRC(73eb9329) SHA1(58accf41a90693874b86ce98d8d43c27beb8b6dc))
ROM_END

MSX_LAYOUT_INIT (hbf1)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 0, 1, 1, ROM, 0x4000, 0x20000)
	MSX_LAYOUT_SLOT (3, 1, 1, 2, ROM, 0x8000, 0x24000)
	MSX_LAYOUT_SLOT (3, 2, 1, 2, ROM, 0x8000, 0x2c000)
	MSX_LAYOUT_SLOT (3, 3, 0, 4, RAM, 0x10000, 0x0000)  /* 64KB RAM */
MSX_LAYOUT_END

/* MSX2 - Sony HB-F1II */

ROM_START (hbf12)
	ROM_REGION (0x34000, "maincpu",0)
	ROM_LOAD ("f12bios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))
	ROM_LOAD ("f12ext.rom", 0x8000, 0x4000, CRC(43e7a7fc) SHA1(0fbd45ef3dd7bb82d4c31f1947884f411f1ca344))
	/* 0x0c000 - 0x1ffff reserved for optional disk and fmpac roms from msx2 parent set */
	ROM_FILL (0xc000, 0x14000, 0)
	ROM_LOAD ("f12note1.rom", 0x20000, 0x4000, CRC(dcacf970) SHA1(30d914cda2180889a40a3328e0a0c1327f4eaa10))
	ROM_LOAD ("f12note2.rom", 0x24000, 0x8000, CRC(b0241a61) SHA1(ed2fea5c2a3c2e58d4f69f9d636e08574486a2b1))
	ROM_LOAD ("f12note3.rom", 0x2c000, 0x8000, CRC(44a10e6a) SHA1(917d1c079e03c4a44de864f123d03c4e32c8daae))
ROM_END

MSX_LAYOUT_INIT (hbf12)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 0, 1, 1, ROM, 0x4000, 0x20000)
	MSX_LAYOUT_SLOT (3, 1, 1, 2, ROM, 0x8000, 0x24000)
	MSX_LAYOUT_SLOT (3, 2, 1, 2, ROM, 0x8000, 0x2c000)
	MSX_LAYOUT_SLOT (3, 3, 0, 4, RAM_MM, 0x10000, 0x0000)   /* 64KB Mapper RAM */
	MSX_LAYOUT_RAMIO_SET_BITS (0x80)
MSX_LAYOUT_END

/* MSX2 - Sony HB-F1XD */

ROM_START (hbf1xd)
	ROM_REGION (0x20000, "maincpu",0)
	ROM_LOAD ("f1xdbios.rom.ic27", 0x0000, 0x8000, BAD_DUMP CRC(ba81b3dd) SHA1(4ce41fcc1a603411ec4e99556409c442078f0ecf))
	ROM_LOAD ("f1xdext.rom.ic27", 0x8000, 0x4000, BAD_DUMP CRC(43e7a7fc) SHA1(0fbd45ef3dd7bb82d4c31f1947884f411f1ca344))
	ROM_LOAD ("f1xddisk.rom.ic27", 0xc000, 0x4000, BAD_DUMP CRC(54c73ad6) SHA1(12f2cc79b3d09723840bae774be48c0d721ec1c6))
	/* 0x10000 - 0x1ffff reserved for optional fmpac roms from msx2 parent set */
	ROM_FILL (0x10000, 0x10000, 0)
ROM_END

MSX_LAYOUT_INIT (hbf1xd)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 0, 1, 2, DISK_ROM, 0x4000, 0xc000)
	MSX_LAYOUT_SLOT (3, 3, 0, 4, RAM_MM, 0x10000, 0x0000)   /* 64KB Mapper RAM */
	MSX_LAYOUT_RAMIO_SET_BITS (0x80)
MSX_LAYOUT_END

/* MSX2 - Sony HB-F1XDMK2 */

ROM_START (hbf1xdm2)
	ROM_REGION (0x20000, "maincpu",0)
	ROM_LOAD ("f1m2bios.rom.ic27", 0x0000, 0x8000, BAD_DUMP CRC(ba81b3dd) SHA1(4ce41fcc1a603411ec4e99556409c442078f0ecf))
	ROM_LOAD ("f1m2ext.rom.ic27", 0x8000, 0x4000, BAD_DUMP CRC(43e7a7fc) SHA1(0fbd45ef3dd7bb82d4c31f1947884f411f1ca344))
	ROM_LOAD ("f1m2disk.rom.ic27", 0xc000, 0x4000, BAD_DUMP CRC(54c73ad6) SHA1(12f2cc79b3d09723840bae774be48c0d721ec1c6))
	/* 0x10000 - 0x1ffff reserved for optional fmpac roms from msx2 parent set */
	ROM_FILL (0x10000, 0x10000, 0)
ROM_END

MSX_LAYOUT_INIT (hbf1xdm2)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 0, 1, 2, DISK_ROM, 0x4000, 0xc000)
	MSX_LAYOUT_SLOT (3, 3, 0, 4, RAM_MM, 0x10000, 0x0000)   /* 64KB Mapper RAM */
	MSX_LAYOUT_RAMIO_SET_BITS (0x80)
MSX_LAYOUT_END

/* MSX2 - Sony HB-F5 */

ROM_START (hbf5)
	ROM_REGION (0x20000, "maincpu", 0)
	ROM_LOAD ("hbf5bios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))
	ROM_LOAD ("hbf5ext.rom", 0x8000, 0x4000, CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33))
	ROM_LOAD ("hbf5note.rom", 0xc000, 0x4000, CRC(0cdc0777) SHA1(06ba91d6732ee8a2ecd5dcc38b0ce42403d86708))
ROM_END

MSX_LAYOUT_INIT (hbf5)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (0, 1, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (0, 1, 2, 1, ROM, 0x4000, 0xc000)
	MSX_LAYOUT_SLOT (0, 2, 0, 4, RAM_MM, 0x10000, 0x0000)   /* 64KB?? Mapper RAM */
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
MSX_LAYOUT_END

/* MSX2 - Sony HB-F500 */

ROM_START (hbf500)
	ROM_REGION (0x40000, "maincpu", 0)
	ROM_LOAD ("f500bios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))
	ROM_LOAD ("f500ext.rom", 0x8000, 0x4000, CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33))
	ROM_LOAD ("f500disk.rom", 0xc000, 0x4000, CRC(f7f5b0ea) SHA1(e93b8da1e8dddbb3742292b0e5e58731b90e9313))
	/* 0x10000 - 0x1ffff reserved for optional fmpac roms from msx2 parent set */
	ROM_FILL (0x10000, 0x10000, 0)
	ROM_LOAD ("f500kfn.rom", 0x20000, 0x20000, CRC(5a59926e) SHA1(6acaf2eeb57f65f7408235d5e07b7563229de799))
ROM_END

MSX_LAYOUT_INIT (hbf500)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (0, 0, 2, 2, RAM, 0x8000, 0x8000)   /* 32KB RAM */
	MSX_LAYOUT_SLOT (0, 1, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (0, 1, 1, 2, DISK_ROM, 0x4000, 0xc000)
	MSX_LAYOUT_SLOT (0, 2, 0, 2, RAM, 0x8000, 0x0000)   /* 32KB RAM */
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_KANJI (0x20000)
MSX_LAYOUT_END

/* MSX2 - Sony HB-F500P */

ROM_START (hbf500p)
	ROM_REGION (0x20000, "maincpu", 0)
	ROM_LOAD ("500pbios.rom.ic41", 0x0000, 0x8000, CRC(b31c851d) SHA1(0de3c802057560560a03d7965fcc4cff69f8575c))
	ROM_LOAD ("500pext.ic47", 0x8000, 0x8000, CRC(cdd4824a) SHA1(505031f1e8396a6e0cb11c1540e6e7f6999d1191))
	/* 0x10000 - 0x1ffff reserved for optional fmpac roms from msx2 parent set */
	ROM_FILL (0x10000, 0x10000, 0)
ROM_END

MSX_LAYOUT_INIT (hbf500p)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (0, 0, 2, 2, RAM, 0x8000, 0x8000)   /* 32KB RAM */
	MSX_LAYOUT_SLOT (0, 1, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (0, 1, 1, 2, DISK_ROM, 0x4000, 0xc000)
	MSX_LAYOUT_SLOT (0, 2, 0, 2, RAM, 0x8000, 0x0000)   /* 32KB RAM */
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, ROM, 0x10000, 0x10000)
MSX_LAYOUT_END

/* MSX2 - Sony HB-F700D */

ROM_START (hbf700d)
	ROM_REGION (0x20000, "maincpu", 0)
	ROM_LOAD ("700dbios.rom.ic5", 0x0000, 0x8000, CRC(e975aa79) SHA1(cef16eb95502ba6ab2265fcafcedde470a101541))
	ROM_LOAD ("700dext.ic6", 0x8000, 0x8000, CRC(100cf756) SHA1(317722fa36c2ed31c07c5218b43490fd5badf1f8))
	/* 0x10000 - 0x1ffff reserved for optional fmpac roms from msx2 parent set */
	ROM_FILL (0x10000, 0x10000, 0)
ROM_END

MSX_LAYOUT_INIT (hbf700d)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 0, 1, 2, DISK_ROM, 0x4000, 0xc000)
	MSX_LAYOUT_SLOT (3, 3, 0, 4, RAM_MM, 0x40000, 0x0000)   /* 256KB Mapper RAM */
	MSX_LAYOUT_RAMIO_SET_BITS (0x80)
MSX_LAYOUT_END

/* MSX2 - Sony HB-F700F */

ROM_START (hbf700f)
	ROM_REGION (0x20000, "maincpu", 0)
	ROM_LOAD ("700fbios.ic5", 0x0000, 0x8000, CRC(440dae3c) SHA1(fedd9b682d056ddd1e9b3d281723e12f859b2e69))
	ROM_LOAD ("700fext.ic6", 0x8000, 0x8000, CRC(7c8b07b1) SHA1(ecacb20ba0a9bbd25e8c0f128d64dd66f8cd8bee))
	/* 0x10000 - 0x1ffff reserved for optional fmpac roms from msx2 parent set */
	ROM_FILL (0x10000, 0x10000, 0)
ROM_END

MSX_LAYOUT_INIT (hbf700f)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 0, 1, 2, DISK_ROM, 0x4000, 0xc000)
	MSX_LAYOUT_SLOT (3, 3, 0, 4, RAM_MM, 0x40000, 0x0000)   /* 256KB Mapper RAM */
	MSX_LAYOUT_RAMIO_SET_BITS (0x80)
MSX_LAYOUT_END

/* MSX2 - Sony HB-F700P */

ROM_START (hbf700p)
	ROM_REGION (0x20000, "maincpu", 0)
	ROM_LOAD ("700pbios.rom.ic5", 0x0000, 0x8000, CRC(b31c851d) SHA1(0de3c802057560560a03d7965fcc4cff69f8575c))
	ROM_LOAD ("700pext.ic6", 0x8000, 0x8000, CRC(63e1bffc) SHA1(496698a60432490dc1306c8cc1d4a6ded275261a))
	/* 0x10000 - 0x1ffff reserved for optional fmpac roms from msx2 parent set */
	ROM_FILL (0x10000, 0x10000, 0)
ROM_END

MSX_LAYOUT_INIT (hbf700p)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 0, 1, 2, DISK_ROM, 0x4000, 0xc000)
	MSX_LAYOUT_SLOT (3, 3, 0, 4, RAM_MM, 0x40000, 0x0000)   /* 256KB Mapper RAM */
	MSX_LAYOUT_RAMIO_SET_BITS (0x80)
MSX_LAYOUT_END

/* MSX2 - Sony HB-F700S */

ROM_START (hbf700s)
	ROM_REGION (0x20000, "maincpu", 0)
	ROM_LOAD ("700sbios.rom.ic5", 0x0000, 0x8000, CRC(c2b889a5) SHA1(4811956f878c3e03da46317f787cdc4bebc86f47))
	ROM_LOAD ("700sext.ic6", 0x8000, 0x8000, CRC(28d1badf) SHA1(ae3ed88a2d7034178e08f7bdf5409f462bf67fc9))
	/* 0x10000 - 0x1ffff reserved for optional fmpac roms from msx2 parent set */
	ROM_FILL (0x10000, 0x10000, 0)
ROM_END

MSX_LAYOUT_INIT (hbf700s)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 0, 1, 2, DISK_ROM, 0x4000, 0xc000)
	MSX_LAYOUT_SLOT (3, 3, 0, 4, RAM_MM, 0x40000, 0x0000)   /* 256KB Mapper RAM */
	MSX_LAYOUT_RAMIO_SET_BITS (0x80)
MSX_LAYOUT_END

/* MSX2 - Sony HB-F900 */
ROM_START (hbf900)
	ROM_REGION (0x44000, "maincpu", 0)
	ROM_LOAD ("f900bios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))
	ROM_LOAD ("f900ext.rom", 0x8000, 0x4000, CRC(43e7a7fc) SHA1(0fbd45ef3dd7bb82d4c31f1947884f411f1ca344))
	ROM_LOAD ("f900disk.rom", 0xc000, 0x4000, CRC(f83d0ea6) SHA1(fc760d1d7b16370abc7eea39955f230b95b37df6))
	/* 0x10000 - 0x1ffff reserved for optional fmpac roms from msx2 parent set */
	ROM_FILL (0x10000, 0x10000, 0)
	ROM_LOAD ("f900util.rom", 0x20000, 0x4000, CRC(bc6c7c66) SHA1(558b7383544542cf7333700ff90c3efbf93ba2a3))
	ROM_LOAD ("f900kfn.rom", 0x24000, 0x20000, CRC(5a59926e) SHA1(6acaf2eeb57f65f7408235d5e07b7563229de799))
ROM_END

MSX_LAYOUT_INIT (hbf900)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 0, 1, 2, DISK_ROM, 0x4000, 0xc000)
	MSX_LAYOUT_SLOT (3, 1, 0, 4, RAM_MM, 0x40000, 0x0000)   /* 256KB Mapper RAM */
	MSX_LAYOUT_SLOT (3, 3, 1, 1, ROM, 0x4000, 0x10000)
	MSX_LAYOUT_KANJI (0x24000)
	MSX_LAYOUT_RAMIO_SET_BITS (0x80)
MSX_LAYOUT_END

/* MSX2 - Sony HB-F900 (a) */
ROM_START (hbf900a)
	ROM_REGION (0x44000, "maincpu", 0)
	ROM_LOAD ("f900bios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))
	ROM_LOAD ("f900ext.rom", 0x8000, 0x4000, CRC(43e7a7fc) SHA1(0fbd45ef3dd7bb82d4c31f1947884f411f1ca344))
	ROM_LOAD ("f900disa.rom", 0xc000, 0x4000, CRC(54c73ad6) SHA1(12f2cc79b3d09723840bae774be48c0d721ec1c6))
	/* 0x10000 - 0x1ffff reserved for optional fmpac roms from msx2 parent set */
	ROM_FILL (0x10000, 0x10000, 0)
	ROM_LOAD ("f900util.rom", 0x20000, 0x4000, CRC(bc6c7c66) SHA1(558b7383544542cf7333700ff90c3efbf93ba2a3))
	ROM_LOAD ("f900kfn.rom", 0x24000, 0x20000, CRC(5a59926e) SHA1(6acaf2eeb57f65f7408235d5e07b7563229de799))
ROM_END

MSX_LAYOUT_INIT (hbf900a)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 0, 1, 2, DISK_ROM, 0x4000, 0xc000)
	MSX_LAYOUT_SLOT (3, 1, 0, 4, RAM_MM, 0x40000, 0x0000)   /* 256KB Mapper RAM */
	MSX_LAYOUT_SLOT (3, 3, 1, 1, ROM, 0x4000, 0x10000)
	MSX_LAYOUT_KANJI (0x24000)
	MSX_LAYOUT_RAMIO_SET_BITS (0x80)
MSX_LAYOUT_END

/* MSX2 - Sony HB-F9P */

ROM_START (hbf9p)
	ROM_REGION (0x28000, "maincpu", 0)
	ROM_LOAD ("f9pbios.rom.ic11", 0x0000, 0x8000, CRC(b31c851d) SHA1(0de3c802057560560a03d7965fcc4cff69f8575c))
	ROM_LOAD ("f9pfirm1.ic12", 0x8000, 0x8000, CRC(524f67aa) SHA1(41a186afced50ca6312cb5b6c4adb684faca6232))
	/* 0x10000 - 0x1ffff reserved for optional fmpac roms from msx2 parent set */
	ROM_FILL (0x10000, 0x10000, 0)
	ROM_LOAD ("f9pfirm2.rom.ic13", 0x20000, 0x8000, CRC(ea97069f) SHA1(2d1880d1f5a6944fcb1b198b997a3d90ecd1903d))
ROM_END

MSX_LAYOUT_INIT (hbf9p)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 0, 1, 1, ROM, 0x4000, 0xc000)
	MSX_LAYOUT_SLOT (3, 1, 1, 2, ROM, 0x8000, 0x20000)
	MSX_LAYOUT_SLOT (3, 2, 0, 4, RAM_MM, 0x20000, 0x0000)   /* 128KB Mapper RAM */
	MSX_LAYOUT_RAMIO_SET_BITS (0x80)
MSX_LAYOUT_END

/* MSX2 - Sony HB-F9P Russian */

ROM_START (hbf9pr)
	ROM_REGION (0x20000, "maincpu", 0)
	ROM_LOAD ("f9prbios.rom", 0x0000, 0x8000, CRC(39d7674a) SHA1(47642bb0a2c46a82100543dc3970d0a49fc53b69))
	ROM_LOAD ("f9prext.rom", 0x8000, 0x4000, CRC(8b966f50) SHA1(65253cb38ab11084f355a2d4ad78fa6c64cbe660))
ROM_END

MSX_LAYOUT_INIT (hbf9pr)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 2, 0, 4, RAM_MM, 0x10000, 0x0000)   /* 64KB?? Mapper RAM */
MSX_LAYOUT_END

/* MSX2 - Sony HB-F9S */

ROM_START (hbf9s)
	ROM_REGION (0x28000, "maincpu", 0)
	ROM_LOAD ("f9sbios.ic11", 0x0000, 0x8000, CRC(c2b889a5) SHA1(4811956f878c3e03da46317f787cdc4bebc86f47))
	ROM_LOAD ("f9sfirm1.ic12", 0x8000, 0x8000, CRC(cf39620b) SHA1(1166a93d7185ba024bdf2bfa9a30e1c447fb6db1))
	/* 0x10000 - 0x1ffff reserved for optional fmpac roms from msx2 parent set */
	ROM_FILL (0x10000, 0x10000, 0)
	ROM_LOAD ("f9sfirm2.ic13", 0x20000, 0x8000, CRC(ea97069f) SHA1(2d1880d1f5a6944fcb1b198b997a3d90ecd1903d))
ROM_END

MSX_LAYOUT_INIT (hbf9s)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 0, 1, 1, ROM, 0x4000, 0xc000)
	MSX_LAYOUT_SLOT (3, 1, 1, 2, ROM, 0x8000, 0x20000)
	MSX_LAYOUT_SLOT (3, 2, 0, 4, RAM_MM, 0x20000, 0x0000)   /* 128KB Mapper RAM */
	MSX_LAYOUT_RAMIO_SET_BITS (0x80)
MSX_LAYOUT_END

/* MSX2 - Sony HB-G900AP */

/* IC109 - 32KB Basic ROM SLOT#00 0000-7FFF */
/* IC112 - 16KB Basic ROM SLOT#01 0000-3FFF */
/* IC117 - 16KB Disk ROM SLOT#01 4000-7FFF */
/* IC123 - 32KB ROM RS232C ROM SLOT#02 4000-7FFF / Video Utility ROM SLOT#03 4000-7FFF */

/* MSX2 - Sony HB-G900AP */
ROM_START (hbg900ap)
	ROM_REGION (0x28000, "maincpu", 0)
	ROM_LOAD ("g900bios.rom", 0x0000, 0x8000, CRC(b31c851d) SHA1(0de3c802057560560a03d7965fcc4cff69f8575c))
	ROM_LOAD ("g900ext.rom", 0x8000, 0x4000, CRC(8f84f783) SHA1(3288894e1be6af705871499b23c85732dbc40993))
	ROM_LOAD ("g900disk.rom", 0xc000, 0x4000, CRC(54c73ad6) SHA1(12f2cc79b3d09723840bae774be48c0d721ec1c6))
	/* 0x10000 - 0x1ffff reserved for optional fmpac roms from msx2 parent set */
	ROM_FILL (0x10000, 0x10000, 0)
	ROM_LOAD ("g900232c.rom", 0x20000, 0x4000, CRC(06cf1da6) SHA1(373aa82d0426830880a7344ef98f7309d93814c7))
	ROM_LOAD ("g900util.rom", 0x24000, 0x4000, CRC(d0417c20) SHA1(8779b004e7605a3c419825f0373a5d8fa84e1d5b))
ROM_END

MSX_LAYOUT_INIT (hbg900ap)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (0, 1, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (0, 1, 1, 2, DISK_ROM, 0x4000, 0xc000)
/*  MSX_LAYOUT_SLOT (0, 2, 1, 1, ROM, 0x4000, 0x20000) */ /* RS232C must be emulated */
	MSX_LAYOUT_SLOT (0, 3, 1, 1, ROM, 0x4000, 0x24000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, RAM_MM, 0x80000, 0x0000)   /* 512KB Mapper RAM */
	MSX_LAYOUT_RAMIO_SET_BITS (0x80)
MSX_LAYOUT_END


/* MSX2 - Sony HB-G900P - 3x 32KB ROMs */

ROM_START (hbg900p)
	ROM_REGION (0x28000, "maincpu", 0)
	ROM_LOAD ("g900bios.rom", 0x0000, 0x8000, CRC(b31c851d) SHA1(0de3c802057560560a03d7965fcc4cff69f8575c))
	ROM_LOAD ("g900ext.rom", 0x8000, 0x4000, CRC(8f84f783) SHA1(3288894e1be6af705871499b23c85732dbc40993))
	ROM_LOAD ("g900disk.rom", 0xc000, 0x4000, CRC(54c73ad6) SHA1(12f2cc79b3d09723840bae774be48c0d721ec1c6))
	/* 0x10000 - 0x1ffff reserved for optional fmpac roms from msx2 parent set */
	ROM_FILL (0x10000, 0x10000, 0)
	ROM_LOAD ("g900232c.rom", 0x20000, 0x4000, CRC(06cf1da6) SHA1(373aa82d0426830880a7344ef98f7309d93814c7))
	ROM_LOAD ("g900util.rom", 0x24000, 0x4000, CRC(d0417c20) SHA1(8779b004e7605a3c419825f0373a5d8fa84e1d5b))
ROM_END

MSX_LAYOUT_INIT (hbg900p)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (0, 1, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (0, 1, 1, 2, DISK_ROM, 0x4000, 0xc000)
/*  MSX_LAYOUT_SLOT (0, 2, 1, 1, ROM, 0x4000, 0x20000) */ /* RS232C must be emulated */
	MSX_LAYOUT_SLOT (0, 3, 1, 1, ROM, 0x4000, 0x24000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, RAM_MM, 0x10000, 0x0000)   /* 64KB Mapper RAM */
	MSX_LAYOUT_RAMIO_SET_BITS (0x80)
MSX_LAYOUT_END

/* MSX2 - Talent TPC-310 */
ROM_START (tpc310)
	ROM_REGION (0x2c000, "maincpu", 0)
	ROM_LOAD ("tpc310bios.rom",  0x0000, 0x8000, CRC(8cd3e845) SHA1(7bba23669b7abfb6a142f9e1735b847d6e4e8267))
	ROM_LOAD ("tpc310ext.rom",   0x8000, 0x4000, CRC(094a9e7a) SHA1(39dfc46260f99b670916b1e55f67a5d4136e6e54))
	ROM_LOAD ("dpf550disk.rom",  0xc000, 0x4000, CRC(347b1b44) SHA1(c1d83c559e1e6a6da961eafa55aab105681c634c))
	/* 0x10000 - 0x1ffff reserved for optional fmpac roms from msx2 parent set */
	ROM_FILL (0x10000, 0x10000, 0)
	ROM_LOAD ("tpc310turbo.rom",0x20000, 0x4000, CRC(0ea62a4d) SHA1(181bf58da7184e128cd419da3109b93344a543cf))
	ROM_LOAD ("tpc310acc.rom",  0x24000, 0x8000, CRC(4fb8fab3) SHA1(cdeb0ed8adecaaadb78d5a5364fd603238591685))
ROM_END

MSX_LAYOUT_INIT (tpc310)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, RAM_MM, 0x20000, 0x0000)   /* 128KB Mapper RAM */
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 0, 1, 1, ROM, 0x4000, 0x20000)
	MSX_LAYOUT_SLOT (3, 1, 1, 2, ROM, 0x8000, 0x24000)
	MSX_LAYOUT_SLOT (3, 2, 1, 1, DISK_ROM2, 0x4000, 0xc000)
	MSX_LAYOUT_SLOT (3, 3, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_RAMIO_SET_BITS (0x80)
MSX_LAYOUT_END

/* MSX2 - Talent TPP-311 */

ROM_START (tpp311)
	ROM_REGION (0x28000, "maincpu", 0)
	ROM_LOAD ("311bios.rom", 0x0000, 0x8000, CRC(8cd3e845) SHA1(7bba23669b7abfb6a142f9e1735b847d6e4e8267))
	ROM_LOAD ("311ext.rom", 0x8000, 0x4000, CRC(094a9e7a) SHA1(39dfc46260f99b670916b1e55f67a5d4136e6e54))
	ROM_LOAD ("311logo.rom", 0x20000, 0x8000, CRC(0e6ecb9f) SHA1(e45ddc5bf1a1e63756d11fb43fc50276ca35cab0))
ROM_END

MSX_LAYOUT_INIT (tpp311)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, RAM_MM, 0x10000, 0x0000)   /* 64KB?? Mapper RAM */
	MSX_LAYOUT_SLOT (2, 0, 0, 2, ROM, 0x8000, 0x20000)
	MSX_LAYOUT_SLOT (3, 0, 0, 1, ROM, 0x4000, 0x8000)
MSX_LAYOUT_END

/* MSX2 - Talent TPS-312 */

ROM_START (tps312)
	ROM_REGION (0x2c000, "maincpu", 0)
	ROM_LOAD ("312bios.rom", 0x0000, 0x8000, CRC(8cd3e845) SHA1(7bba23669b7abfb6a142f9e1735b847d6e4e8267))
	ROM_LOAD ("312ext.rom", 0x8000, 0x4000, CRC(094a9e7a) SHA1(39dfc46260f99b670916b1e55f67a5d4136e6e54))
	ROM_LOAD ("312plan.rom", 0x20000, 0x8000, CRC(b3a6aaf6) SHA1(6de80e863cdd7856ab7aac4c238224a5352bda3b))
	ROM_LOAD ("312write.rom", 0x28000, 0x4000, CRC(63c6992f) SHA1(93682f5baba7697c40088e26f99ee065c78e83b8))
ROM_END

MSX_LAYOUT_INIT (tps312)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, RAM_MM, 0x20000, 0x0000)   /* 128KB?? Mapper RAM */
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 1, 0, 1, ROM, 0x4000, 0x28000)
	MSX_LAYOUT_SLOT (3, 2, 0, 2, ROM, 0x8000, 0x20000)
	MSX_LAYOUT_SLOT (3, 3, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
MSX_LAYOUT_END

/* MSX2 - Toshiba HX-23 */

ROM_START (hx23)
	ROM_REGION (0x30000, "maincpu", 0)
	ROM_LOAD ("hx23bios.rom",  0x0000, 0x8000, CRC(6cdaf3a5) SHA1(6103b39f1e38d1aa2d84b1c3219c44f1abb5436e))
	ROM_LOAD ("hx23ext.rom",   0x8000, 0x4000, CRC(06e4f5e6) SHA1(f5eb0a396097572589f2a6efeed045044e9425e4))
	/* 0xc000 - 0x1ffff reserved for optional fmpac roms from msx2 parent set */
	ROM_FILL (0xc000, 0x14000, 0)
	ROM_LOAD ("hx23word.rom", 0x20000, 0x8000, CRC(39b3e1c0) SHA1(9f7cfa932bd7dfd0d9ecaadc51655fb557c2e125))
	ROM_FILL (0x28000, 0x8000, 0)
ROM_END

MSX_LAYOUT_INIT (hx23)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (0, 0, 2, 2, RAM, 0x8000, 0x8000)   /* 32KB RAM */
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 2, RAM, 0x8000, 0x0000)   /* 32KB RAM */
	MSX_LAYOUT_SLOT (3, 1, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 3, 0, 4, ROM, 0x10000, 0x20000)
	MSX_LAYOUT_RAMIO_SET_BITS (0x80)
MSX_LAYOUT_END

/* MSX2 - Toshiba HX-23F */

ROM_START (hx23f)
	ROM_REGION (0x30000, "maincpu", 0)
	ROM_LOAD ("hx23fbios.rom",  0x0000, 0x8000, CRC(6cdaf3a5) SHA1(6103b39f1e38d1aa2d84b1c3219c44f1abb5436e))
	ROM_LOAD ("hx23fext.rom",   0x8000, 0x4000, CRC(06e4f5e6) SHA1(f5eb0a396097572589f2a6efeed045044e9425e4))
	/* 0xc000 - 0x1ffff reserved for optional fmpac roms from msx2 parent set */
	ROM_FILL (0xc000, 0x14000, 0)
	ROM_LOAD ("hx23fword.rom", 0x20000, 0x8000, CRC(39b3e1c0) SHA1(9f7cfa932bd7dfd0d9ecaadc51655fb557c2e125))
	ROM_FILL (0x28000, 0x8000, 0)
ROM_END

MSX_LAYOUT_INIT (hx23f)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, RAM_MM, 0x20000, 0x0000)   /* 128KB Mapper RAM */
	MSX_LAYOUT_SLOT (3, 1, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 3, 0, 4, ROM, 0x10000, 0x20000)
	MSX_LAYOUT_RAMIO_SET_BITS (0x80)
MSX_LAYOUT_END

/* MSX2 - Yamaha CX7M */

ROM_START (cx7m)
	ROM_REGION (0x28000, "maincpu", 0)
	ROM_LOAD ("cx7mbios.rom",  0x0000, 0x8000, CRC(6cdaf3a5) SHA1(6103b39f1e38d1aa2d84b1c3219c44f1abb5436e))
	ROM_LOAD ("cx7mext.rom",   0x8000, 0x4000, CRC(66237ecf) SHA1(5c1f9c7fb655e43d38e5dd1fcc6b942b2ff68b02))
	/* 0xc000 - 0x1ffff reserved for optional fmpac roms from msx2 parent set */
	ROM_FILL (0xc000, 0x14000, 0)
	ROM_LOAD ("sfg05m.rom", 0x20000, 0x8000, CRC(6c2545c9) SHA1(bc4b242647116f4886bb92e86421f97b1be51938))
ROM_END

MSX_LAYOUT_INIT (cx7m)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (0, 1, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 2, 0, 4, RAM_MM, 0x10000, 0x0000)   /* 64KB Mapper RAM */
	MSX_LAYOUT_SLOT (3, 3, 0, 2, ROM, 0x8000, 0x20000)
	MSX_LAYOUT_RAMIO_SET_BITS (0x80)
MSX_LAYOUT_END

/* MSX2 - Yamaha CX7M-128 */

ROM_START (cx7m128)
	ROM_REGION (0x2c000, "maincpu", 0)
	ROM_LOAD ("cx7mbios.rom",  0x0000, 0x8000, CRC(6cdaf3a5) SHA1(6103b39f1e38d1aa2d84b1c3219c44f1abb5436e))
	ROM_LOAD ("cx7mext.rom",   0x8000, 0x4000, CRC(66237ecf) SHA1(5c1f9c7fb655e43d38e5dd1fcc6b942b2ff68b02))
	/* 0xc000 - 0x1ffff reserved for optional fmpac roms from msx2 parent set */
	ROM_FILL (0xc000, 0x14000, 0)
	ROM_LOAD ("sfg05m.rom", 0x20000, 0x8000, CRC(6c2545c9) SHA1(bc4b242647116f4886bb92e86421f97b1be51938))
	ROM_LOAD ("yrm502.rom", 0x28000, 0x4000, CRC(51f7ddd1) SHA1(2a4b4a4657e3077df8a88f98210b76883d3702b1))
ROM_END

MSX_LAYOUT_INIT (cx7m128)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (0, 1, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 1, 1, 1, ROM, 0x4000, 0x28000)
	MSX_LAYOUT_SLOT (3, 2, 0, 4, RAM_MM, 0x10000, 0x0000)   /* 64KB Mapper RAM */
	MSX_LAYOUT_SLOT (3, 3, 0, 2, ROM, 0x8000, 0x20000)
	MSX_LAYOUT_RAMIO_SET_BITS (0x80)
MSX_LAYOUT_END

/********************************  MSX 2+ **********************************/

/* MSX2+ */

ROM_START (msx2p)
	ROM_REGION (0x48000, "maincpu",0)
	ROM_LOAD ("msx2p.rom", 0x0000, 0x8000, CRC(00870134) SHA1(e2fbd56e42da637609d23ae9df9efd1b4241b18a))
	ROM_LOAD ("msx2pext.rom", 0x8000, 0x4000, CRC(b8ba44d3) SHA1(fe0254cbfc11405b79e7c86c7769bd6322b04995))
	ROM_LOAD_OPTIONAL ("disk.rom", 0xc000, 0x4000, CRC(b7c58fad) SHA1(bc517b4a248c3a1338c5efc937b0128b6a783808))
	ROM_LOAD_OPTIONAL ("fmpac.rom", 0x10000, 0x10000, CRC(0e84505d) SHA1(9d789166e3caf28e4742fe933d962e99618c633d))
	ROM_LOAD ("msx2pkdr.rom", 0x20000, 0x8000, CRC(a068cba9) SHA1(1ef3956f7f918873fb9b031339bba45d1e5e5878))
	ROM_LOAD ("msx2pkfn.rom", 0x28000, 0x20000, CRC(b244f6cf) SHA1(e0e99cd91e88ce2676445663f832c835d74d6fd4))
ROM_END

MSX_LAYOUT_INIT (msx2p)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, RAM_MM, 0x20000, 0x0000)   /* 128KB Mapper RAM */
	MSX_LAYOUT_SLOT (3, 1, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 1, 1, 2, ROM, 0x8000, 0x20000)
	MSX_LAYOUT_SLOT (3, 2, 1, 2, DISK_ROM, 0x4000, 0xc000)
	MSX_LAYOUT_KANJI (0x28000)
	MSX_LAYOUT_RAMIO_SET_BITS (0x80)
MSX_LAYOUT_END

/* MSX2+ - Ciel Expert 3 IDE */

ROM_START (expert3i )
	ROM_REGION (0x30000, "maincpu",0)
	ROM_LOAD ("exp30bios.rom", 0x0000, 0x8000, CRC(a10bb1ce) SHA1(5029cf47031b22bd5d1f68ebfd3be6d6da56dfe9))
	ROM_LOAD ("exp30ext.rom", 0x8000, 0x4000, CRC(6bcf4100) SHA1(cc1744c6c513d6409a142b4fb42fbe70a95d9b7f))
	ROM_LOAD ("cieldisk.rom", 0xc000, 0x4000, CRC(bb550b09) SHA1(0274dd9b5096065a7f4ed019101124c9bd1d56b8))
	ROM_LOAD ("exp30mus.rom", 0x10000, 0x4000, CRC(9881b3fd) SHA1(befebc916bfdb5e8057040f0ae82b5517a7750db))
	ROM_LOAD ("ide240a.rom", 0x20000, 0x10000, CRC(7adf857f) SHA1(8a919dbeed92db8c06a611279efaed8552810239))
ROM_END

MSX_LAYOUT_INIT (expert3i)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (1, 1, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (1, 1, 1, 2, ROM, 0x4000, 0x10000)
	MSX_LAYOUT_SLOT (1, 2, 1, 2, DISK_ROM, 0x4000, 0xc000)
	MSX_LAYOUT_SLOT (1, 3, 0, 4, ROM, 0x10000, 0x20000)         /* IDE hardware needs to be emulated */
	MSX_LAYOUT_SLOT (2, 0, 0, 4, RAM_MM, 0x40000, 0x0000)       /* 256KB?? Mapper RAM */
	MSX_LAYOUT_SLOT (3, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
MSX_LAYOUT_END

/* MSX2+ - Ciel Expert 3 Turbo */

/* Uses a Z84C0010 - CMOS processor working at 7 MHz */
ROM_START (expert3t )
	ROM_REGION (0x30000, "maincpu",0)
	ROM_LOAD ("exp30bios.rom", 0x0000, 0x8000, CRC(a10bb1ce) SHA1(5029cf47031b22bd5d1f68ebfd3be6d6da56dfe9))
	ROM_LOAD ("exp30ext.rom", 0x8000, 0x4000, CRC(6bcf4100) SHA1(cc1744c6c513d6409a142b4fb42fbe70a95d9b7f))
	ROM_LOAD ("cieldisk.rom", 0xc000, 0x4000, CRC(bb550b09) SHA1(0274dd9b5096065a7f4ed019101124c9bd1d56b8))
	ROM_LOAD ("exp30mus.rom", 0x10000, 0x4000, CRC(9881b3fd) SHA1(befebc916bfdb5e8057040f0ae82b5517a7750db))
	ROM_LOAD ("turbo.rom", 0x20000, 0x4000, CRC(ab528416) SHA1(d468604269ae7664ac739ea9f922a05e14ffa3d1))
ROM_END

MSX_LAYOUT_INIT (expert3t)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (1, 1, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (1, 1, 1, 2, ROM, 0x4000, 0x10000)
	MSX_LAYOUT_SLOT (1, 2, 1, 2, ROM, 0x4000, 0x20000)          /* Turbo hardware needs to be emulated */
	MSX_LAYOUT_SLOT (1, 3, 1, 2, DISK_ROM, 0x4000, 0xc000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, RAM_MM, 0x40000, 0x0000)       /* 256KB Mapper RAM */
	MSX_LAYOUT_SLOT (3, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
MSX_LAYOUT_END

/* MSX2+ - Gradiente Expert AC88+ */

ROM_START (expertac)
	ROM_REGION(0x30000, "maincpu", 0)
	ROM_LOAD ("ac88bios.rom", 0x0000, 0x8000, CRC(9ce0da44) SHA1(1fc2306911ab6e1ebdf7cb8c3c34a7f116414e88))
	ROM_LOAD ("ac88ext.rom", 0x8000, 0x4000, CRC(c74c005c) SHA1(d5528825c7eea2cfeadd64db1dbdbe1344478fc6))
	ROM_LOAD ("panadisk.rom", 0xc000, 0x4000, CRC(17fa392b) SHA1(7ed7c55e0359737ac5e68d38cb6903f9e5d7c2b6))
	ROM_LOAD ("ac88asm.rom", 0x20000, 0x4000, CRC(a8a955ae) SHA1(91e522473a8470511584df3ee5b325ea5e2b81ef))
	ROM_LOAD ("xbasic2.rom", 0x24000, 0x4000, CRC(2825b1a0) SHA1(47370bec7ca1f0615a54eda548b07fbc0c7ef398))
ROM_END

MSX_LAYOUT_INIT (expertac)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, RAM_MM, 0x10000, 0x0000)   /* 64KB Mapper RAM?? */
	MSX_LAYOUT_SLOT (3, 1, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 1, 1, 2, ROM, 0x4000, 0x20000)
	MSX_LAYOUT_SLOT (3, 2, 1, 2, DISK_ROM, 0x4000, 0xc000)
	MSX_LAYOUT_SLOT (3, 3, 1, 2, ROM, 0x4000, 0x24000)
MSX_LAYOUT_END

/* MSX2+ - Gradiente Expert DDX+ */

ROM_START (expertdx)
	ROM_REGION(0x38000, "maincpu", 0)
	ROM_LOAD ("ddxbios.rom", 0x0000, 0x8000, CRC(e00af3dc) SHA1(5c463dd990582e677c8206f61035a7c54d8c67f0))
	ROM_LOAD ("ddxext.rom", 0x8000, 0x4000, CRC(b8ba44d3) SHA1(fe0254cbfc11405b79e7c86c7769bd6322b04995))
	ROM_LOAD ("panadisk.rom", 0xc000, 0x4000, CRC(17fa392b) SHA1(7ed7c55e0359737ac5e68d38cb6903f9e5d7c2b6))
	ROM_LOAD ("xbasic2.rom", 0x20000, 0x4000, CRC(2825b1a0) SHA1(47370bec7ca1f0615a54eda548b07fbc0c7ef398))
	ROM_LOAD ("kanji.rom", 0x30000, 0x8000, CRC(b4fc574d) SHA1(dcc3a67732aa01c4f2ee8d1ad886444a4dbafe06))
ROM_END

MSX_LAYOUT_INIT (expertdx)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (1, 1, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (1, 2, 1, 2, ROM, 0x4000, 0x20000)
	MSX_LAYOUT_SLOT (1, 3, 1, 2, DISK_ROM, 0x4000, 0xc000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, RAM_MM, 0x10000, 0x0000)   /* 64KB Mapper RAM?? */
	MSX_LAYOUT_SLOT (3, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	/* Kanji? */
MSX_LAYOUT_END

/* MSX2+ - Panasonic FS-A1FX */

ROM_START (fsa1fx)
	ROM_REGION (0x50000, "maincpu",0)
	ROM_LOAD ("a1fxbios.rom", 0x0000, 0x8000, CRC(19771608) SHA1(e90f80a61d94c617850c415e12ad70ac41e66bb7))
	ROM_LOAD ("a1fxext.rom", 0x8000, 0x4000, CRC(b8ba44d3) SHA1(fe0254cbfc11405b79e7c86c7769bd6322b04995))
	ROM_LOAD ("a1fxdisk.rom", 0xc000, 0x4000, CRC(2bda0184) SHA1(2a0d228afde36ac7c5d3c2aac9c7c664dd071a8c))
	/* 0x10000 - 0x1ffff reserved for optional fmpac roms from msx2p parent set */
	ROM_FILL (0x10000, 0x10000, 0)
	ROM_LOAD ("a1fxkdr.rom", 0x20000, 0x8000, CRC(a068cba9) SHA1(1ef3956f7f918873fb9b031339bba45d1e5e5878))
	ROM_LOAD ("a1fxcock.rom", 0x28000, 0x8000, CRC(f662e6eb) SHA1(9d67fab55b85f4ac4f5924323a70020eb8589057))
	ROM_LOAD ("a1fxkfn.rom", 0x30000, 0x20000, CRC(b244f6cf) SHA1(e0e99cd91e88ce2676445663f832c835d74d6fd4))
ROM_END

MSX_LAYOUT_INIT (fsa1fx)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, RAM_MM, 0x10000, 0x0000)   /* 64KB Mapper RAM */
	MSX_LAYOUT_SLOT (3, 1, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 1, 1, 2, ROM, 0x8000, 0x20000)
/*  MSX_LAYOUT_SLOT (3, 2, 1, 2, DISK_ROM, 0x4000, 0xc000) */ /* FDC Emulation of TC8566AF must be emulated */
	MSX_LAYOUT_SLOT (3, 3, 1, 2, ROM, 0x8000, 0x28000)
	MSX_LAYOUT_KANJI (0x30000)
	MSX_LAYOUT_RAMIO_SET_BITS (0x80)
MSX_LAYOUT_END

/* MSX2+ - Panasonic FS-A1WSX */

ROM_START (fsa1wsx)
	ROM_REGION (0x270000, "maincpu",0)
	ROM_LOAD ("a1wsbios.rom", 0x0000, 0x8000, CRC(358ec547) SHA1(f4433752d3bf876bfefb363c749d4d2e08a218b6))
	ROM_LOAD ("a1wsext.rom", 0x8000, 0x4000, CRC(b8ba44d3) SHA1(fe0254cbfc11405b79e7c86c7769bd6322b04995))
	ROM_LOAD ("a1wsdisk.rom", 0xc000, 0x4000, CRC(ac7d92b4) SHA1(b7068e2aab02072852ca249596b7550ac632c4c2))
	/* 0x10000 - 0x1ffff reserved for optional fmpac roms from msx2p parent set */
	ROM_FILL (0x10000, 0x10000, 0)
	ROM_LOAD ("a1wskdr.rom", 0x20000, 0x8000, CRC(b4fc574d) SHA1(dcc3a67732aa01c4f2ee8d1ad886444a4dbafe06))
	ROM_LOAD ("a1wsmusp.rom", 0x28000, 0x4000, CRC(456e494e) SHA1(6354ccc5c100b1c558c9395fa8c00784d2e9b0a3))
	ROM_FILL (0x2c000, 0x4000, 0)
	ROM_LOAD ("a1wskfn.rom", 0x30000, 0x40000, CRC(1f6406fb) SHA1(5aff2d9b6efc723bc395b0f96f0adfa83cc54a49))
	ROM_LOAD ("a1wsfirm.rom", 0x70000, 0x200000, CRC(e363595d) SHA1(3330d9b6b76e3c4ccb7cf252496ed15d08b95d3f))
ROM_END

MSX_LAYOUT_INIT (fsa1wsx)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (0, 2, 1, 1, ROM, 0x4000, 0x28000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, RAM_MM, 0x10000, 0x0000)   /* 64KB Mapper RAM */
	MSX_LAYOUT_SLOT (3, 1, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 1, 1, 2, ROM, 0x8000, 0x20000)
/*  MSX_LAYOUT_SLOT (3, 2, 1, 2, DISK_ROM, 0x4000, 0xc000) */ /* FDC Emulation of TC8566AF must be emulated */
/*  MSX_LAYOUT_SLOT (3, 3, 1, 4, PANASONIC08, 0x200000, 0x70000) */ /* Panasonic 08KB Mapper must be emulated */
	MSX_LAYOUT_KANJI (0x30000)
	MSX_LAYOUT_RAMIO_SET_BITS (0x80)
MSX_LAYOUT_END

/* MSX2+ - Panasonic FS-A1WX */

ROM_START (fsa1wx)
	ROM_REGION (0x270000, "maincpu",0)
	ROM_LOAD ("a1wxbios.rom", 0x0000, 0x8000, CRC(19771608) SHA1(e90f80a61d94c617850c415e12ad70ac41e66bb7))
	ROM_LOAD ("a1wxext.rom", 0x8000, 0x4000, CRC(b8ba44d3) SHA1(fe0254cbfc11405b79e7c86c7769bd6322b04995))
	ROM_LOAD ("a1wxdisk.rom", 0xc000, 0x4000, CRC(2bda0184) SHA1(2a0d228afde36ac7c5d3c2aac9c7c664dd071a8c))
	/* 0x10000 - 0x1ffff reserved for optional fmpac roms from msx2p parent set */
	ROM_FILL (0x10000, 0x10000, 0)
	ROM_LOAD ("a1wxkdr.rom", 0x20000, 0x8000, CRC(a068cba9) SHA1(1ef3956f7f918873fb9b031339bba45d1e5e5878))
	ROM_LOAD ("a1wxmusp.rom", 0x28000, 0x4000, CRC(456e494e) SHA1(6354ccc5c100b1c558c9395fa8c00784d2e9b0a3))
	ROM_FILL (0x2c000, 0x4000, 0)
	ROM_LOAD ("a1wxkfn.rom", 0x30000, 0x40000, CRC(1f6406fb) SHA1(5aff2d9b6efc723bc395b0f96f0adfa83cc54a49))
	ROM_LOAD ("a1wxfirm.rom", 0x70000, 0x200000, CRC(283f3250) SHA1(d37ab4bd2bfddd8c97476cbe7347ae581a6f2972))
ROM_END

MSX_LAYOUT_INIT (fsa1wx)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (0, 2, 1, 1, ROM, 0x4000, 0x28000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, RAM_MM, 0x10000, 0x0000)   /* 64KB Mapper RAM */
	MSX_LAYOUT_SLOT (3, 1, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 1, 1, 2, ROM, 0x8000, 0x20000)
/*  MSX_LAYOUT_SLOT (3, 2, 1, 2, DISK_ROM, 0x4000, 0xc000) */ /* FDC Emulation of TC8566AF must be emulated */
/*  MSX_LAYOUT_SLOT (3, 3, 1, 4, PANASONIC08, 0x200000, 0x70000) */ /* Panasonic 08KB Mapper must be emulated */
	MSX_LAYOUT_KANJI (0x30000)
	MSX_LAYOUT_RAMIO_SET_BITS (0x80)
MSX_LAYOUT_END

/* MSX2+ - Panasonic FS-A1WX (a) */
ROM_START (fsa1wxa)
	ROM_REGION (0x270000, "maincpu",0)
	ROM_LOAD ("a1wxbios.rom", 0x0000, 0x8000, CRC(19771608) SHA1(e90f80a61d94c617850c415e12ad70ac41e66bb7))
	ROM_LOAD ("a1wxext.rom", 0x8000, 0x4000, CRC(b8ba44d3) SHA1(fe0254cbfc11405b79e7c86c7769bd6322b04995))
	ROM_LOAD ("a1wxdisk.rom", 0xc000, 0x4000, CRC(2bda0184) SHA1(2a0d228afde36ac7c5d3c2aac9c7c664dd071a8c))
	/* 0x10000 - 0x1ffff reserved for optional fmpac roms from msx2p parent set */
	ROM_FILL (0x10000, 0x10000, 0)
	ROM_LOAD ("a1wxkdr.rom", 0x20000, 0x8000, CRC(a068cba9) SHA1(1ef3956f7f918873fb9b031339bba45d1e5e5878))
	ROM_LOAD ("a1wxmusp.rom", 0x28000, 0x4000, CRC(456e494e) SHA1(6354ccc5c100b1c558c9395fa8c00784d2e9b0a3))
	ROM_FILL (0x2c000, 0x4000, 0)
	ROM_LOAD ("a1wxkfn.rom", 0x30000, 0x40000, CRC(1f6406fb) SHA1(5aff2d9b6efc723bc395b0f96f0adfa83cc54a49))
	ROM_LOAD ("a1wxfira.rom", 0x70000, 0x200000, CRC(58440a8e) SHA1(8e0d4a77e7d5736e8225c2df4701509363eb230f))
ROM_END

MSX_LAYOUT_INIT (fsa1wxa)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (0, 2, 1, 1, ROM, 0x4000, 0x28000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, RAM_MM, 0x10000, 0x0000)   /* 64KB Mapper RAM */
	MSX_LAYOUT_SLOT (3, 1, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 1, 1, 2, ROM, 0x8000, 0x20000)
/*  MSX_LAYOUT_SLOT (3, 2, 1, 2, DISK_ROM, 0x4000, 0xc000) */ /* FDC Emulation of TC8566AF must be emulated */
/*  MSX_LAYOUT_SLOT (3, 3, 1, 4, PANASONIC08, 0x200000, 0x70000) */ /* Panasonic 08KB Mapper must be emulated */
	MSX_LAYOUT_KANJI (0x30000)
	MSX_LAYOUT_RAMIO_SET_BITS (0x80)
MSX_LAYOUT_END

/* MSX2+ - Sanyo Wavy PHC-35J */

ROM_START (phc35j)
	ROM_REGION (0x48000, "maincpu",0)
	ROM_LOAD ("35jbios.rom", 0x0000, 0x8000, CRC(358ec547) SHA1(f4433752d3bf876bfefb363c749d4d2e08a218b6))
	ROM_LOAD ("35jext.rom", 0x8000, 0x4000, CRC(b8ba44d3) SHA1(fe0254cbfc11405b79e7c86c7769bd6322b04995))
	/* 0x0c000 - 0x1ffff reserved for optional disk and fmpac roms from msx2p parent set */
	ROM_FILL (0xc000, 0x14000, 0)
	ROM_LOAD ("35jkdr.rom", 0x20000, 0x8000, CRC(b4fc574d) SHA1(dcc3a67732aa01c4f2ee8d1ad886444a4dbafe06))
	ROM_LOAD ("35jkfn.rom", 0x28000, 0x20000, CRC(c9651b32) SHA1(84a645becec0a25d3ab7a909cde1b242699a8662))
ROM_END

MSX_LAYOUT_INIT (phc35j)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, RAM_MM, 0x10000, 0x0000)   /* 64KB Mapper RAM */
	MSX_LAYOUT_SLOT (3, 1, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 1, 1, 2, ROM, 0x8000, 0x20000)
	MSX_LAYOUT_KANJI (0x28000)
	MSX_LAYOUT_RAMIO_SET_BITS (0x80)
MSX_LAYOUT_END

/* MSX2+ - Sanyo Wavy PHC-70FD1 */

ROM_START (phc70fd)
	ROM_REGION (0x50000, "maincpu",0)
	ROM_LOAD ("70fdbios.rom", 0x0000, 0x8000, CRC(19771608) SHA1(e90f80a61d94c617850c415e12ad70ac41e66bb7))
	ROM_LOAD ("70fdext.rom", 0x8000, 0x4000, CRC(b8ba44d3) SHA1(fe0254cbfc11405b79e7c86c7769bd6322b04995))
	ROM_LOAD ("70fddisk.rom", 0xc000, 0x4000, CRC(db7f1125) SHA1(9efa744be8355675e7bfdd3976bbbfaf85d62e1d))
	/* 0x10000 - 0x1ffff reserved for optional fmpac roms from msx2p parent set */
	ROM_FILL (0x10000, 0x10000, 0)
	ROM_LOAD ("70fdkdr.rom", 0x20000, 0x8000, CRC(a068cba9) SHA1(1ef3956f7f918873fb9b031339bba45d1e5e5878))
	ROM_LOAD ("70fdmus.rom", 0x28000, 0x4000, CRC(5c32eb29) SHA1(aad42ba4289b33d8eed225d42cea930b7fc5c228))
	ROM_LOAD ("70fdbas.rom", 0x2c000, 0x4000, CRC(da7be246) SHA1(22b3191d865010264001b9d896186a9818478a6b))
	ROM_LOAD ("70fdkfn.rom", 0x30000, 0x20000, CRC(c9651b32) SHA1(84a645becec0a25d3ab7a909cde1b242699a8662))
ROM_END

MSX_LAYOUT_INIT (phc70fd)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, RAM_MM, 0x10000, 0x0000)   /* 64KB Mapper RAM */
	MSX_LAYOUT_SLOT (3, 1, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 1, 1, 2, ROM, 0x8000, 0x20000)
/*  MSX_LAYOUT_SLOT (3, 2, 1, 2, DISK_ROM, 0x4000, 0xc000) */ /* FDC Emulation of TC8566AF must be emulated */
	MSX_LAYOUT_SLOT (3, 3, 1, 1, ROM, 0x4000, 0x28000)
	MSX_LAYOUT_SLOT (3, 3, 2, 1, ROM, 0x4000, 0x2c000)
	MSX_LAYOUT_KANJI (0x30000)
	MSX_LAYOUT_RAMIO_SET_BITS (0x80)
MSX_LAYOUT_END

/* MSX2+ - Sanyo Wavy PHC-70FD2 */
ROM_START (phc70fd2)
	ROM_REGION (0x70000, "maincpu",0)
	ROM_LOAD ("70f2bios.rom", 0x0000, 0x8000, CRC(19771608) SHA1(e90f80a61d94c617850c415e12ad70ac41e66bb7))
	ROM_LOAD ("70f2ext.rom", 0x8000, 0x4000, CRC(b8ba44d3) SHA1(fe0254cbfc11405b79e7c86c7769bd6322b04995))
	ROM_LOAD ("70f2disk.rom", 0xc000, 0x4000, CRC(db7f1125) SHA1(9efa744be8355675e7bfdd3976bbbfaf85d62e1d))
	/* 0x10000 - 0x1ffff reserved for optional fmpac roms from msx2p parent set */
	ROM_FILL (0x10000, 0x10000, 0)
	ROM_LOAD ("70f2kdr.rom", 0x20000, 0x8000, CRC(a068cba9) SHA1(1ef3956f7f918873fb9b031339bba45d1e5e5878))
	ROM_LOAD ("70f2mus.rom", 0x28000, 0x4000, CRC(5c32eb29) SHA1(aad42ba4289b33d8eed225d42cea930b7fc5c228))
	ROM_LOAD ("70f2bas.rom", 0x2c000, 0x4000, CRC(da7be246) SHA1(22b3191d865010264001b9d896186a9818478a6b))
	ROM_LOAD ("70f2kfn.rom", 0x30000, 0x40000, CRC(9a850db9) SHA1(bcdb4dae303dfe5234f372d70a5e0271d3202c36))
ROM_END

MSX_LAYOUT_INIT (phc70fd2)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, RAM_MM, 0x10000, 0x0000)   /* 64KB Mapper RAM */
	MSX_LAYOUT_SLOT (3, 1, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 1, 1, 2, ROM, 0x8000, 0x20000)
/*  MSX_LAYOUT_SLOT (3, 2, 1, 2, DISK_ROM, 0x4000, 0xc000) */ /* FDC Emulation of TC8566AF must be emulated */
	MSX_LAYOUT_SLOT (3, 3, 1, 1, ROM, 0x4000, 0x28000)
	MSX_LAYOUT_SLOT (3, 3, 2, 1, ROM, 0x4000, 0x2c000)
	MSX_LAYOUT_KANJI (0x30000)
	MSX_LAYOUT_RAMIO_SET_BITS (0x80)
MSX_LAYOUT_END

/* MSX2+ - Sony HB-F1XDJ */

ROM_START (hbf1xdj)
	ROM_REGION (0x170000, "maincpu",0)
	ROM_LOAD ("f1xjbios.rom", 0x0000, 0x8000, CRC(00870134) SHA1(e2fbd56e42da637609d23ae9df9efd1b4241b18a))
	ROM_LOAD ("f1xjext.rom", 0x8000, 0x4000, CRC(b8ba44d3) SHA1(fe0254cbfc11405b79e7c86c7769bd6322b04995))
	ROM_LOAD ("f1xjdisk.rom", 0xc000, 0x4000, CRC(a21f5266) SHA1(c1bb307a570ab833e3bfcc4a58a4f4e12dc1df0f))
	/* 0x10000 - 0x1ffff reserved for optional fmpac roms from msx2p parent set */
	ROM_FILL (0x10000, 0x10000, 0)
	ROM_LOAD ("f1xjkdr.rom", 0x20000, 0x8000, CRC(a068cba9) SHA1(1ef3956f7f918873fb9b031339bba45d1e5e5878))
	ROM_LOAD ("f1xjmus.rom", 0x28000, 0x4000, CRC(5c32eb29) SHA1(aad42ba4289b33d8eed225d42cea930b7fc5c228))
	ROM_FILL (0x2c000, 0x4000, 0)
	ROM_LOAD ("f1xjkfn.rom", 0x30000, 0x40000, CRC(7016dfd0) SHA1(218d91eb6df2823c924d3774a9f455492a10aecb))
	ROM_LOAD ("f1xjfirm.rom", 0x70000, 0x100000, CRC(77be583f) SHA1(ade0c5ba5574f8114d7079050317099b4519e88f))
ROM_END

MSX_LAYOUT_INIT (hbf1xdj)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
/*  MSX_LAYOUT_SLOT (0, 3, 1, 4, SONY08, 0x100000, 0x70000) */ /* Sony 08KB MSX-JE Mapper must be emulated */
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, RAM_MM, 0x10000, 0x0000)   /* 64KB Mapper RAM */
	MSX_LAYOUT_SLOT (3, 1, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 1, 1, 2, ROM, 0x8000, 0x20000)
	MSX_LAYOUT_SLOT (3, 2, 1, 2, DISK_ROM, 0x4000, 0xc000)
	MSX_LAYOUT_SLOT (3, 3, 1, 1, ROM, 0x4000, 0x28000)
	MSX_LAYOUT_KANJI (0x30000)
	MSX_LAYOUT_RAMIO_SET_BITS (0x80)
MSX_LAYOUT_END

/* MSX2+ - Sony HB-F1XV */

ROM_START (hbf1xv)
	ROM_REGION (0x170000, "maincpu",0)
	ROM_LOAD ("f1xvbios.rom", 0x0000, 0x8000, CRC(2c7ed27b) SHA1(174c9254f09d99361ff7607630248ff9d7d8d4d6))
	ROM_LOAD ("f1xvext.rom", 0x8000, 0x4000, CRC(b8ba44d3) SHA1(fe0254cbfc11405b79e7c86c7769bd6322b04995))
	ROM_LOAD ("f1xvdisk.rom", 0xc000, 0x4000, CRC(04e4e533) SHA1(5a4e7dbbfb759109c7d2a3b38bda9c60bf6ffef5))
	/* 0x10000 - 0x1ffff reserved for optional fmpac roms from msx2p parent set */
	ROM_FILL (0x10000, 0x10000, 0)
	ROM_LOAD ("f1xvkdr.rom", 0x20000, 0x8000, CRC(b4fc574d) SHA1(dcc3a67732aa01c4f2ee8d1ad886444a4dbafe06))
	ROM_LOAD ("f1xvmus.rom", 0x28000, 0x4000, CRC(5c32eb29) SHA1(aad42ba4289b33d8eed225d42cea930b7fc5c228))
	ROM_FILL (0x2c000, 0x4000, 0)
	ROM_LOAD ("f1xvkfn.rom", 0x30000, 0x40000, CRC(7016dfd0) SHA1(218d91eb6df2823c924d3774a9f455492a10aecb))
	ROM_LOAD ("f1xvfirm.rom", 0x70000, 0x100000, CRC(77be583f) SHA1(ade0c5ba5574f8114d7079050317099b4519e88f))
ROM_END

MSX_LAYOUT_INIT (hbf1xv)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
/*  MSX_LAYOUT_SLOT (0, 3, 1, 4, SONY08, 0x100000, 0x70000) */ /* Sony 08KB MSX-JE Mapper must be emulated */
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, RAM_MM, 0x10000, 0x0000)   /* 64KB Mapper RAM */
	MSX_LAYOUT_SLOT (3, 1, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 1, 1, 2, ROM, 0x8000, 0x20000)
	MSX_LAYOUT_SLOT (3, 2, 1, 2, DISK_ROM, 0x4000, 0xc000)
	MSX_LAYOUT_SLOT (3, 3, 1, 1, ROM, 0x4000, 0x28000)
	MSX_LAYOUT_KANJI (0x30000)
	MSX_LAYOUT_RAMIO_SET_BITS (0x80)
MSX_LAYOUT_END

/* MSX2+ - Sony HB-F9S+ */

ROM_START (hbf9sp)
	ROM_REGION (0x2c000, "maincpu", 0)
	ROM_LOAD ("f9spbios.rom", 0x0000, 0x8000, CRC(994d3a80) SHA1(03556d380a9bd413faf1b9e3cbd7da47c7238775))
	ROM_LOAD ("f9spext.rom", 0x8000, 0x4000, CRC(b8ba44d3) SHA1(fe0254cbfc11405b79e7c86c7769bd6322b04995))
	ROM_LOAD ("f9psfrm1.rom", 0x20000, 0x4000, CRC(43d4cef1) SHA1(8948704bad9ff27873fa9ccd0ef89868e2bd6479))
	ROM_LOAD ("f9spfrm2.rom", 0x24000, 0x8000, CRC(ea97069f) SHA1(2d1880d1f5a6944fcb1b198b997a3d90ecd1903d))
ROM_END

MSX_LAYOUT_INIT (hbf9sp)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (1, 0, 0, 4, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 4, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 0, 2, 1, ROM, 0x4000, 0x20000)
	MSX_LAYOUT_SLOT (3, 1, 2, 4, ROM, 0x8000, 0x24000)
	MSX_LAYOUT_SLOT (3, 2, 0, 4, RAM_MM, 0x10000, 0x0000)   /* 64KB?? Mapper RAM */
MSX_LAYOUT_END

/* MSX Turbo-R - Panasonic FS-A1GT */

ROM_START (fsa1gt)
	ROM_REGION (0x480000, "maincpu", 0)
	ROM_LOAD ("a1gtbios.rom", 0x0000, 0x8000, CRC(937c8dbb) SHA1(242e73d8284a012b275c0a266844ebbc4269d787))
	ROM_LOAD ("a1gtext.rom", 0x8000, 0x4000, CRC(70aea0fe) SHA1(018d7a5222f28514908fb1b1513286a6558a6d05))
	ROM_LOAD ("a1gtdos.rom", 0x20000, 0x10000, CRC(bb2a0eae) SHA1(4880bf34f1c86fff5456ec2b4cf70d02339e2caa))
	ROM_LOAD ("a1gtkdr.rom", 0x30000, 0x8000, CRC(eaf0d125) SHA1(5b39c1ccd3a213b78e02927f56a9abc72cd8c28d))
	ROM_LOAD ("a1gtmus.rom", 0x38000, 0x4000, CRC(f5f93437) SHA1(6aea1aef5ec31c1826c22edf580525f93baad425))
	ROM_LOAD ("a1gtopt.rom", 0x3c000, 0x4000, CRC(50d11f60) SHA1(b4433a3975c57dd440d6bf12dbd28b2ac1b90ef4))
	ROM_LOAD ("a1gtkfn.rom", 0x40000, 0x40000, CRC(1f6406fb) SHA1(5aff2d9b6efc723bc395b0f96f0adfa83cc54a49))
	ROM_LOAD ("a1gtfirm.rom", 0x80000, 0x400000, CRC(feefeadc) SHA1(e779c338eb91a7dea3ff75f3fde76b8af22c4a3a))
ROM_END

MSX_LAYOUT_INIT (fsa1gt)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (0, 2, 1, 1, ROM, 0x4000, 0x38000)
	MSX_LAYOUT_SLOT (0, 3, 1, 1, ROM, 0x4000, 0x3c000)
	MSX_LAYOUT_SLOT (1, 0, 0, 0, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 0, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, RAM_MM, 0x20000, 0x0000)   /* 128KB?? Mapper RAM */
	MSX_LAYOUT_SLOT (3, 1, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 1, 1, 2, ROM, 0x8000, 0x30000)
	MSX_LAYOUT_SLOT (3, 2, 1, 4, ROM, 0xc000, 0x20000)
	MSX_LAYOUT_SLOT (3, 3, 0, 4, ROM, 0x10000, 0x80000)
MSX_LAYOUT_END

/* MSX Turbo-R - Panasonic FS-A1ST */

ROM_START (fsa1st)
	ROM_REGION (0x480000, "maincpu", 0)
	ROM_LOAD ("a1stbios.rom", 0x0000, 0x8000, CRC(77b94ae0) SHA1(f078b5ec56884bfb81481d45c7151418770bff5a))
	ROM_LOAD ("a1stext.rom", 0x8000, 0x4000, CRC(2c2c77a4) SHA1(373412f9c32762de1c3a7e27fc3d80614e0a0c8e))
	ROM_LOAD ("a1stdos.rom", 0x20000, 0x10000, CRC(1fc71407) SHA1(5d2186658adcf4ce0c2d3232384b5712341108e5))
	ROM_LOAD ("a1stkdr.rom", 0x30000, 0x8000, CRC(eaf0d125) SHA1(5b39c1ccd3a213b78e02927f56a9abc72cd8c28d))
	ROM_LOAD ("a1stmus.rom", 0x38000, 0x4000, CRC(fd7dec41) SHA1(e002a9b426732e6c2d31e548c40cf7c122348ce3))
	ROM_LOAD ("a1stopt.rom", 0x3c000, 0x4000, CRC(c6a4a2a1) SHA1(cb06dea7b025745f9d2b87dcf03ded615287ead3))
	ROM_LOAD ("a1stkfn.rom", 0x40000, 0x40000, CRC(1f6406fb) SHA1(5aff2d9b6efc723bc395b0f96f0adfa83cc54a49))
	ROM_LOAD ("a1stfirm.rom", 0x80000, 0x400000, CRC(139ac99c) SHA1(c212b11fda13f83dafed688c54d098e7e47ab225))
ROM_END

MSX_LAYOUT_INIT (fsa1st)
	MSX_LAYOUT_SLOT (0, 0, 0, 2, ROM, 0x8000, 0x0000)
	MSX_LAYOUT_SLOT (0, 2, 1, 1, ROM, 0x4000, 0x38000)
	MSX_LAYOUT_SLOT (0, 3, 1, 1, ROM, 0x4000, 0x3c000)
	MSX_LAYOUT_SLOT (1, 0, 0, 0, CARTRIDGE1, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (2, 0, 0, 0, CARTRIDGE2, 0x0000, 0x0000)
	MSX_LAYOUT_SLOT (3, 0, 0, 4, RAM_MM, 0x20000, 0x0000)   /* 128KB?? Mapper RAM */
	MSX_LAYOUT_SLOT (3, 1, 0, 1, ROM, 0x4000, 0x8000)
	MSX_LAYOUT_SLOT (3, 1, 1, 2, ROM, 0x8000, 0x30000)
	MSX_LAYOUT_SLOT (3, 2, 1, 4, ROM, 0xc000, 0x20000)
	MSX_LAYOUT_SLOT (3, 3, 0, 4, ROM, 0x10000, 0x80000)
MSX_LAYOUT_END


MSX_DRIVER_LIST
	/* MSX 1 */
	MSX_DRIVER (msx)
	MSX_DRIVER (ax170)
	MSX_DRIVER (canonv10)
	MSX_DRIVER (canonv20)
	MSX_DRIVER (dpc100)
	MSX_DRIVER (dpc180)
	MSX_DRIVER (dpc200)
	MSX_DRIVER (gsfc200)
	MSX_DRIVER (expert10)
	MSX_DRIVER (expert11)
	MSX_DRIVER (expert13)
	MSX_DRIVER (expert20)
	MSX_DRIVER (expertdp)
	MSX_DRIVER (expertpl)
	MSX_DRIVER (jvchc7gb)
	MSX_DRIVER (mlf80)
	MSX_DRIVER (mlfx1)
	MSX_DRIVER (cf1200)
	MSX_DRIVER (cf2000)
	MSX_DRIVER (cf2700)
	MSX_DRIVER (cf3000)
	MSX_DRIVER (cf3300)
	MSX_DRIVER (fs1300)
	MSX_DRIVER (fs4000)
	MSX_DRIVER (phc2)
	MSX_DRIVER (phc28)
	MSX_DRIVER (cf2700g)
	MSX_DRIVER (nms801)
	MSX_DRIVER (vg802000)
	MSX_DRIVER (vg802020)
	MSX_DRIVER (vg8020f)
	MSX_DRIVER (piopx7)
	MSX_DRIVER (spc800)
	MSX_DRIVER (mpc64)
	MSX_DRIVER (mpc100)
	MSX_DRIVER (phc28l)
	MSX_DRIVER (phc28s)
	MSX_DRIVER (mpc10)
	MSX_DRIVER (hotbit11)
	MSX_DRIVER (hotbit12)
	MSX_DRIVER (hotbi13b)
	MSX_DRIVER (hotbi13p)
	MSX_DRIVER (hotbit20)
	MSX_DRIVER (hb10p)
	MSX_DRIVER (hb20p)
	MSX_DRIVER (hb201)
	MSX_DRIVER (hb201p)
	MSX_DRIVER (hb501p)
	MSX_DRIVER (hb55d)
	MSX_DRIVER (hb55p)
	MSX_DRIVER (hb75d)
	MSX_DRIVER (hb75p)
	MSX_DRIVER (svi728)
	MSX_DRIVER (svi738)
	MSX_DRIVER (svi738sw)
	MSX_DRIVER (svi738pl)
	MSX_DRIVER (tadpc200)
	MSX_DRIVER (tadpc20a)
	MSX_DRIVER (hx10)
	MSX_DRIVER (hx10s)
	MSX_DRIVER (hx20)
	MSX_DRIVER (cx5m)
	MSX_DRIVER (cx5m128)
	MSX_DRIVER (cx5m2)
	MSX_DRIVER (yis303)
	MSX_DRIVER (yis503)
	MSX_DRIVER (yis503f)
	MSX_DRIVER (yis503ii)
	MSX_DRIVER (y503iir)
	MSX_DRIVER (y503iir2)
	MSX_DRIVER (yis503m)
	MSX_DRIVER (yc64)
	MSX_DRIVER (mx64)
	MSX_DRIVER (bruc100)



	MSX_DRIVER (msx2)
	MSX_DRIVER (ax350)
	MSX_DRIVER (ax370)
	MSX_DRIVER (nms8220)
	MSX_DRIVER (nms8220a)
	MSX_DRIVER (vg8230)
	MSX_DRIVER (vg8230j)
	MSX_DRIVER (vg8235)
	MSX_DRIVER (vg8235f)
	MSX_DRIVER (vg8240)
	MSX_DRIVER (nms8245)
	MSX_DRIVER (nms8245f)
	MSX_DRIVER (nms8250)
	MSX_DRIVER (nms8250j)
	MSX_DRIVER (nms8255)
	MSX_DRIVER (nms8280)
	MSX_DRIVER (nms8280g)
	MSX_DRIVER (hbf5)
	MSX_DRIVER (hbf9p)
	MSX_DRIVER (hbf9pr)
	MSX_DRIVER (hbf9s)
	MSX_DRIVER (hbf500p)
	MSX_DRIVER (hbf700d)
	MSX_DRIVER (hbf700f)
	MSX_DRIVER (hbf700p)
	MSX_DRIVER (hbf700s)
	MSX_DRIVER (hbg900ap)
	MSX_DRIVER (hbg900p)
	MSX_DRIVER (mlg30)
	MSX_DRIVER (fs5500)
	MSX_DRIVER (fs4500)
	MSX_DRIVER (fs4700)
	MSX_DRIVER (fs5000)
	MSX_DRIVER (fs4600)
	MSX_DRIVER (fsa1)
	MSX_DRIVER (fsa1a)
	MSX_DRIVER (fsa1mk2)
	MSX_DRIVER (fsa1f)
	MSX_DRIVER (fsa1fm)
	MSX_DRIVER (hbf500)
	MSX_DRIVER (hbf900)
	MSX_DRIVER (hbf900a)
	MSX_DRIVER (hbf1)
	MSX_DRIVER (hbf12)
	MSX_DRIVER (hbf1xd)
	MSX_DRIVER (hbf1xdm2)
	MSX_DRIVER (mpc2300)
	MSX_DRIVER (mpc25fd)
	MSX_DRIVER (phc23)
	MSX_DRIVER (cpc300)
	MSX_DRIVER (cpc300e)
	MSX_DRIVER (cpc400)
	MSX_DRIVER (cpc400s)

	MSX_DRIVER (msx2p)
	MSX_DRIVER (expert3i)
	MSX_DRIVER (expert3t)
	MSX_DRIVER (expertac)
	MSX_DRIVER (expertdx)
	MSX_DRIVER (fsa1fx)
	MSX_DRIVER (fsa1wx)
	MSX_DRIVER (fsa1wxa)
	MSX_DRIVER (fsa1wsx)
	MSX_DRIVER (hbf1xdj)
	MSX_DRIVER (hbf1xv)
	MSX_DRIVER (phc70fd)
	MSX_DRIVER (phc70fd2)
	MSX_DRIVER (phc35j)
	MSX_DRIVER (hbf9sp)

	MSX_DRIVER (tpc310)
	MSX_DRIVER (tpp311)
	MSX_DRIVER (tps312)
	MSX_DRIVER (hx23)
	MSX_DRIVER (hx23f)
	MSX_DRIVER (cx7m)
	MSX_DRIVER (cx7m128)

MSX_DRIVER_END


/*    YEAR  NAME      PARENT    COMPAT  MACHINE   INPUT     INIT         COMPANY              FULLNAME */
COMP(1983, msx,       0,        0,      msx_pal,  msx, msx_state,      msx,     "ASCII & Microsoft", "MSX" , 0)

COMP(1983, ax170,     msx,      0,      msx_pal,  msx, msx_state,      msx,     "Al Alamiah", "AX-170" , 0)
COMP(1983, canonv10,  msx,      0,      msx_pal,  msx, msx_state,      msx,     "Canon", "V-10" , 0)
COMP(1983, canonv20,  msx,      0,      msx_pal,  msx, msx_state,      msx,     "Canon", "V-20" , 0)
COMP(1984, dpc100,    msx,      0,      msx_ntsc, msxkr, msx_state,    msx,     "Daewoo", "IQ-1000 DPC-100 (Korea)" , 0)
COMP(1984, dpc180,    msx,      0,      msx_ntsc, msxkr, msx_state,    msx,     "Daewoo", "IQ-1000 DPC-180 (Korea)" , 0)
COMP(1984, dpc200,    msx,      0,      msx_ntsc, msxkr, msx_state,    msx,     "Daewoo", "IQ-1000 DPC-200 (Korea)" , 0)
COMP(1983, gsfc200,   msx,      0,      msx_pal,  msx, msx_state,      msx,     "Goldstar", "FC-200" , 0)
COMP(1983, expert10,  msx,      0,      msx_ntsc, expert10, msx_state, msx,     "Gradiente", "Expert 1.0 (Brazil)" , 0)
COMP(1984, expert11,  msx,      0,      msx_ntsc, expert11, msx_state, msx,     "Gradiente", "Expert 1.1 (Brazil)" , 0)
COMP(1984, expert13,  msx,      0,      msx_ntsc, expert11, msx_state, msx,     "Gradiente", "Expert 1.3 (Brazil)" , 0)
COMP(1985, expertdp,  msx,      0,      msx_ntsc, expert11, msx_state, msx,     "Gradiente", "Expert DDPlus (Brazil)", 0 )
COMP(1984, expertpl,  msx,      0,      msx_ntsc, expert11, msx_state, msx,     "Gradiente", "Expert Plus (Brazil)" , 0)
COMP(1983, jvchc7gb,  msx,      0,      msx_pal,  msx, msx_state,      msx,     "JVC", "HC-7GB" , 0)
COMP(1983, mlf80,     msx,      0,      msx_pal,  msx, msx_state,      msx,     "Mitsubishi", "ML-F80" , 0)
COMP(1983, mlfx1,     msx,      0,      msx_pal,  msx, msx_state,      msx,     "Mitsubishi", "ML-FX1" , 0)
COMP(1984, cf1200,    msx,      0,      msx_ntsc, msxjp, msx_state,    msx,     "National / Matsushita", "CF-1200 (Japan)" , 0)
COMP(1983, cf2000,    msx,      0,      msx_ntsc, msxjp, msx_state,    msx,     "National / Matsushita", "CF-2000 (Japan)" , 0)
COMP(1984, cf2700,    msx,      0,      msx_ntsc, msxjp, msx_state,    msx,     "National / Matsushita", "CF-2700 (Japan)" , 0)
COMP(1984, cf3000,    msx,      0,      msx_ntsc, msxjp, msx_state,    msx,     "National / Matsushita", "CF-3000 (Japan)" , 0)
COMP(1985, cf3300,    msx,      0,      msx_ntsc, msxjp, msx_state,    msx,     "National / Matsushita", "CF-3300 (Japan)", 0 )
COMP(1985, fs1300,    msx,      0,      msx_ntsc, msxjp, msx_state,    msx,     "National / Matsushita", "FS-1300 (Japan)" , 0)
COMP(1985, fs4000,    msx,      0,      msx_ntsc, msxjp, msx_state,    msx,     "National / Matsushita", "FS-4000 (Japan)" , 0)
COMP(1983, phc2,      msx,      0,      msx_pal,  msx, msx_state,      msx,     "Olympia", "PHC-2" , 0)
COMP(19??, phc28,     msx,      0,      msx_pal,  msx, msx_state,      msx,     "Olympia", "PHC-28", GAME_NOT_WORKING)
COMP(1984, cf2700g,   msx,      0,      msx_pal,  msx, msx_state,      msx,     "Panasonic", "CF-2700G (Germany)", GAME_NOT_WORKING)
COMP(1983, nms801,    msx,      0,      msx_pal,  msx, msx_state,      msx,     "Philips", "NMS-801" , 0)
COMP(1984, vg8000,    msx,      0,      msx_pal,  msx, msx_state,      msx,     "Philips", "VG-8000" , GAME_NOT_WORKING)
COMP(1984, vg8010,    msx,      0,      msx_pal,  msx, msx_state,      msx,     "Philips", "VG-8010" , GAME_NOT_WORKING)
COMP(1984, vg8010f,   msx,      0,      msx_pal,  msx, msx_state,      msx,     "Philips", "VG-8010F" , GAME_NOT_WORKING)
COMP(1985, vg802000,  msx,      0,      msx_pal,  msx, msx_state,      msx,     "Philips", "VG-8020-00" , 0)
COMP(1985, vg802020,  msx,      0,      msx_pal,  msx, msx_state,      msx,     "Philips", "VG-8020-20" , 0)
COMP(19??, vg8020f,   msx,      0,      msx_pal,  msx, msx_state,      msx,     "Philips",  "VG-8020F", GAME_NOT_WORKING)
COMP(1985, piopx7,    msx,      0,      msx_pal,  msx, msx_state,      msx,     "Pioneer", "PX-07" , 0)
COMP(19??, spc800,    msx,      0,      msx_ntsc, msx, msx_state,      msx,     "Samsung",  "SPC-800", GAME_NOT_WORKING)
COMP(1985, mpc64,     msx,      0,      msx_ntsc, msxjp, msx_state,    msx,     "Sanyo", "MPC-64" , 0)
COMP(1985, mpc100,    msx,      0,      msx_pal,  msx, msx_state,      msx,     "Sanyo", "MPC-100" , 0)
COMP(1983, phc28l,    msx,      0,      msx_pal,  msx, msx_state,      msx,     "Sanyo", "PHC-28L", GAME_NOT_WORKING)
COMP(1983, phc28s,    msx,      0,      msx_pal,  msx, msx_state,      msx,     "Sanyo", "PHC-28S", GAME_NOT_WORKING)
COMP(19??, mpc10,     msx,      0,      msx_pal,  msx, msx_state,      msx,     "Sanyo", "Wavy MPC-10", GAME_NOT_WORKING)
COMP(1985, hotbit11,  msx,      0,      msx_ntsc, hotbit, msx_state,   msx,     "Sharp / Epcom", "HB-8000 Hotbit 1.1" , 0)
COMP(1985, hotbit12,  msx,      0,      msx_ntsc, hotbit, msx_state,   msx,     "Sharp / Epcom", "HB-8000 Hotbit 1.2" , 0)
COMP(1985, hotbi13b,  msx,      0,      msx_ntsc, hotbit, msx_state,   msx,     "Sharp / Epcom", "HB-8000 Hotbit 1.3b" , 0)
COMP(1985, hotbi13p,  msx,      0,      msx_ntsc, hotbit, msx_state,   msx,     "Sharp / Epcom", "HB-8000 Hotbit 1.3p" , 0)
COMP(1985, hb10p,     msx,      0,      msx_pal,  msx, msx_state,      msx,     "Sony", "HB-10P" , 0)
COMP(1985, hb20p,     msx,      0,      msx_pal,  msx, msx_state,      msx,     "Sony", "HB-20P (Spanish)" , 0)
COMP(1985, hb201,     msx,      0,      msx_ntsc, msxjp, msx_state,    msx,     "Sony", "HB-201 (Japan)" , 0)
COMP(1985, hb201p,    msx,      0,      msx_pal,  msx, msx_state,      msx,     "Sony", "HB-201P" , 0)
COMP(1984, hb501p,    msx,      0,      msx_pal,  msx, msx_state,      msx,     "Sony", "HB-501P" , 0)
COMP(1983, hb55d,     msx,      0,      msx_pal,  msx, msx_state,      msx,     "Sony", "HB-55D (Germany)" , 0)
COMP(1983, hb55p,     msx,      0,      msx_pal,  msx, msx_state,      msx,     "Sony", "HB-55P" , 0)
COMP(1983, hb75d,     msx,      0,      msx_pal,  msx, msx_state,      msx,     "Sony", "HB-75D (Germany)" , 0)
COMP(1983, hb75p,     msx,      0,      msx_pal,  msx, msx_state,      msx,     "Sony", "HB-75P" , 0)
COMP(1985, svi728,    msx,      0,      msx_pal,  msx, msx_state,      msx,     "Spectravideo", "SVI-728", 0 )
COMP(1985, svi738,    msx,      0,      msx_pal,  msx, msx_state,      msx,     "Spectravideo", "SVI-738", 0 )
COMP(1985, svi738sw,  msx,      0,      msx_pal,  msx, msx_state,      msx,     "Spectravideo", "SVI-738 (Swedish)", 0 )
COMP(1985, svi738pl,  msx,      0,      msx_pal,  msx, msx_state,      msx,     "Spectravideo", "SVI-738 (Poland)", 0 )
COMP(1983, tadpc200,  msx,      0,      msx_pal,  msx, msx_state,      msx,     "Talent", "DPC-200" , 0)
COMP(1983, tadpc20a,  msx,      0,      msx_pal,  msx, msx_state,      msx,     "Talent", "DPC-200A" , 0)
COMP(1984, hx10,      msx,      0,      msx_pal,  msx, msx_state,      msx,     "Toshiba", "HX-10" , 0)
COMP(1984, hx10s,     msx,      0,      msx_pal,  msx, msx_state,      msx,     "Toshiba", "HX-10S" , 0)
COMP(1984, hx20,      msx,      0,      msx_pal,  msx, msx_state,      msx,     "Toshiba", "HX-20" , 0)
COMP(1984, cx5m,      msx,      0,      msx_pal,  msx, msx_state,      msx,     "Yamaha", "CX5M" , GAME_NOT_WORKING)
COMP(1984, cx5m128,   msx,      0,      msx_pal,  msx, msx_state,      msx,     "Yamaha", "CX5M-128" , GAME_NOT_WORKING)
COMP(1984, cx5m2,     msx,      0,      msx_pal,  msx, msx_state,      msx,     "Yamaha", "CX5MII" , GAME_NOT_WORKING)
COMP(1984, yis303,    msx,      0,      msx_pal,  msx, msx_state,      msx,     "Yamaha", "YIS303" , 0)
COMP(1984, yis503,    msx,      0,      msx_pal,  msx, msx_state,      msx,     "Yamaha", "YIS503" , 0)
COMP(19??, yis503f,   msx,      0,      msx_pal,  msx, msx_state,      msx,     "Yamaha",   "YIS503F", GAME_NOT_WORKING)
COMP(1984, yis503ii,  msx,      0,      msx_pal,  msx, msx_state,      msx,     "Yamaha", "YIS503II" , 0)
COMP(1986, y503iir,   msx,      0,      msx_pal,  msx, msx_state,      msx,     "Yamaha", "YIS503IIR (Russian)" , 0)
COMP(1986, y503iir2,  msx,      0,      msx_pal,  msx, msx_state,      msx,     "Yamaha", "YIS503IIR (Estonian)" , 0)
COMP(1984, yis503m,   msx,      0,      msx_pal,  msx, msx_state,      msx,     "Yamaha", "YIS503M" , GAME_NOT_WORKING)
COMP(1984, yc64,      msx,      0,      msx_pal,  msx, msx_state,      msx,     "Yashica", "YC-64" , 0)
COMP(1984, mx64,      msx,      0,      msx_ntsc, msxkr, msx_state,    msx,     "Yeno", "MX64" , 0)
COMP(1984, bruc100,   msx,      0,      msx_pal,  msx, msx_state,      msx,     "Frael", "Bruc 100-1" , 0)

COMP(1985, msx2,      0,        msx,    msx2_pal, msx2, msx_state,     msx,     "ASCII & Microsoft", "MSX2", 0)
COMP(1986, ax350,     msx2,     0,      msx2_pal, msx2, msx_state,     msx,     "Al Alamiah", "AX-350", 0)
COMP(1986, ax370,     msx2,     0,      msx2_pal, msx2, msx_state,     msx,     "Al Alamiah", "AX-370", 0)
COMP(1986, expert20,  msx2,     0,      msx2_pal, msx2, msx_state,     msx,     "Gradiente", "Expert 2.0 (Brazil)" , GAME_NOT_WORKING)
COMP(1986, nms8220,   msx2,     0,      msx2_pal, msx2, msx_state,     msx,     "Philips", "NMS-8220 (12-jun-1986)", 0)
COMP(1986, nms8220a,  msx2,     0,      msx2_pal, msx2, msx_state,     msx,     "Philips", "NMS-8220 (13-aug-1986)", 0)
COMP(1986, vg8230,    msx2,     0,      msx2_pal, msx2, msx_state,     msx,     "Philips", "VG-8230", 0)
COMP(1986, vg8235,    msx2,     0,      msx2_pal, msx2, msx_state,     msx,     "Philips", "VG-8235", 0)
COMP(1986, vg8235f,   msx2,     0,      msx2_pal, msx2, msx_state,     msx,     "Philips", "VG-8235F", 0)
COMP(1986, vg8240,    msx2,     0,      msx2_pal, msx2, msx_state,     msx,     "Philips", "VG-8240", 0)
COMP(1986, nms8245,   msx2,     0,      msx2_pal, msx2, msx_state,     msx,     "Philips", "NMS-8245", 0)
COMP(1986, nms8245f,  msx2,     0,      msx2_pal, msx2, msx_state,     msx,     "Philips", "NMS-8245F", 0)
COMP(1986, nms8250,   msx2,     0,      msx2_pal, msx2, msx_state,     msx,     "Philips", "NMS-8250", 0)
COMP(1986, nms8255,   msx2,     0,      msx2_pal, msx2, msx_state,     msx,     "Philips", "NMS-8255", 0)
COMP(1986, nms8280,   msx2,     0,      msx2_pal, msx2, msx_state,     msx,     "Philips", "NMS-8280", 0)
COMP(1986, nms8280g,  msx2,     0,      msx2_pal, msx2, msx_state,     msx,     "Philips", "NMS-8280G", 0)
COMP(19??, hbf5,      msx2,     0,      msx2_pal, msx2, msx_state,     msx,     "Sony", "HB-F5", GAME_NOT_WORKING)
COMP(1985, hbf9p,     msx2,     0,      msx2_pal, msx2, msx_state,     msx,     "Sony", "HB-F9P" , 0)
COMP(19??, hbf9pr,    msx2,     0,      msx2_pal, msx2, msx_state,     msx,     "Sony", "HB-F9P Russion", GAME_NOT_WORKING)
COMP(1985, hbf9s,     msx2,     0,      msx2_pal, msx2, msx_state,     msx,     "Sony", "HB-F9S" , 0)
COMP(1985, hbf500p,   msx2,     0,      msx2_pal, msx2, msx_state,     msx,     "Sony", "HB-F500P", 0)
COMP(1985, hbf700d,   msx2,     0,      msx2_pal, msx2, msx_state,     msx,     "Sony", "HB-F700D (Germany)" , 0)
COMP(1985, hbf700f,   msx2,     0,      msx2_pal, msx2, msx_state,     msx,     "Sony", "HB-F700F" , 0)
COMP(1985, hbf700p,   msx2,     0,      msx2_pal, msx2, msx_state,     msx,     "Sony", "HB-F700P" , 0)
COMP(1985, hbf700s,   msx2,     0,      msx2_pal, msx2, msx_state,     msx,     "Sony", "HB-F700S (Spain)", 0)
COMP(1986, hbg900ap,  msx2,     0,      msx2_pal, msx2, msx_state,     msx,     "Sony", "HB-G900AP", 0 )
COMP(1986, hbg900p,   msx2,     0,      msx2_pal, msx2, msx_state,     msx,     "Sony", "HB-G900P", 0 )
COMP(1986, hotbit20,  msx2,     0,      msx2_pal, msx2, msx_state,     msx,     "Sharp / Epcom", "HB-8000 Hotbit 2.0" , GAME_NOT_WORKING)
COMP(1986, tpc310,    msx2,     0,      msx2_pal, msx2, msx_state,     msx,     "Talent", "TPC-310", 0)
COMP(19??, tpp311,    msx2,     0,      msx2_pal, msx2, msx_state,     msx,     "Talent", "TPP-311", GAME_NOT_WORKING)
COMP(19??, tps312,    msx2,     0,      msx2_pal, msx2, msx_state,     msx,     "Talent", "TPS-312", GAME_NOT_WORKING)
COMP(1986, hx23,      msx2,     0,      msx2_pal, msx2, msx_state,     msx,     "Toshiba", "HX-23", 0)
COMP(1986, hx23f,     msx2,     0,      msx2_pal, msx2, msx_state,     msx,     "Toshiba", "HX-23F", 0)
COMP(1986, cx7m,      msx2,     0,      msx2_pal, msx2, msx_state,     msx,     "Yamaha", "CX7M" , GAME_NOT_WORKING)
COMP(1986, cx7m128,   msx2,     0,      msx2_pal, msx2, msx_state,     msx,     "Yamaha", "CX7M-128" , GAME_NOT_WORKING)
COMP(1983, mlg30,     msx2,     0,      msx2,     msx2, msx_state,     msx,     "Mistubishi", "ML-G30", GAME_NOT_WORKING)
COMP(1985, fs5500,    msx2,     0,      msx2,     msx2jp, msx_state,   msx,     "National / Matsushita", "FS-5500F1/F2 (Japan)", 0 )
COMP(1986, fs4500,    msx2,     0,      msx2,     msx2jp, msx_state,   msx,     "National / Matsushita", "FS-4500 (Japan)", 0 )
COMP(1986, fs4700,    msx2,     0,      msx2,     msx2jp, msx_state,   msx,     "National / Matsushita", "FS-4700 (Japan)", 0 )
COMP(1986, fs5000,    msx2,     0,      msx2,     msx2jp, msx_state,   msx,     "National / Matsushita", "FS-5000F2 (Japan)", 0 )
COMP(1986, fs4600,    msx2,     0,      msx2,     msx2jp, msx_state,   msx,     "National / Matsushita", "FS-4600 (Japan)", 0 )
COMP(1986, fsa1,      msx2,     0,      msx2,     msx2jp, msx_state,   msx,     "Panasonic / Matsushita", "FS-A1 / 1st released version (Japan)", 0)
COMP(1986, fsa1a,     msx2,     0,      msx2,     msx2jp, msx_state,   msx,     "Panasonic / Matsushita", "FS-A1 / 2nd released version (Japan)", 0)
COMP(1987, fsa1mk2,   msx2,     0,      msx2,     msx2jp, msx_state,   msx,     "Panasonic / Matsushita", "FS-A1MK2 (Japan)", 0)
COMP(1987, fsa1f,     msx2,     0,      msx2,     msx2jp, msx_state,   msx,     "Panasonic / Matsushita", "FS-A1F (Japan)", 0 )
COMP(1987, fsa1fm,    msx2,     0,      msx2,     msx2jp, msx_state,   msx,     "Panasonic / Matsushita", "FS-A1FM (Japan)", 0 )
COMP(19??, nms8250j,  msx2,     0,      msx2,     msx2jp, msx_state,   msx,     "Philips", "NMS-8250J", GAME_NOT_WORKING)
COMP(19??, vg8230j,   msx2,     0,      msx2,     msx2jp, msx_state,   msx,     "Philips", "VG-8230J", GAME_NOT_WORKING)
COMP(1986, hbf500,    msx2,     0,      msx2,     msx2jp, msx_state,   msx,     "Sony", "HB-F500 (Japan)", 0)
COMP(1986, hbf900,    msx2,     0,      msx2,     msx2jp, msx_state,   msx,     "Sony", "HB-F900 / 1st released version (Japan)", 0)
COMP(1986, hbf900a,   msx2,     0,      msx2,     msx2jp, msx_state,   msx,     "Sony", "HB-F900 / 2nd released version (Japan)", 0)
COMP(1986, hbf1,      msx2,     0,      msx2,     msx2jp, msx_state,   msx,     "Sony", "HB-F1 (Japan)", GAME_NOT_WORKING )
COMP(1987, hbf12,     msx2,     0,      msx2,     msx2jp, msx_state,   msx,     "Sony", "HB-F1II (Japan)", GAME_NOT_WORKING )
COMP(1987, hbf1xd,    msx2,     0,      msx2,     msx2jp, msx_state,   msx,     "Sony", "HB-F1XD (Japan)", 0)
COMP(1988, hbf1xdm2,  msx2,     0,      msx2,     msx2jp, msx_state,   msx,     "Sony", "HB-F1XDMK2 (Japan)", 0)
COMP(19??, mpc2300,   msx2,     0,      msx2,     msx2, msx_state,     msx,     "Sanyo", "MPC-2300", GAME_NOT_WORKING)
COMP(19??, mpc25fd,   msx2,     0,      msx2,     msx2, msx_state,     msx,     "Sanyo", "Wavy MPC-25FD", GAME_NOT_WORKING)
COMP(1988, phc23,     msx2,     0,      msx2,     msx2jp, msx_state,   msx,     "Sanyo", "Wavy PHC-23 (Japan)", 0)
COMP(1986, cpc300,    msx2,     0,      msx2,     msx2kr, msx_state,   msx,     "Daewoo", "IQ-2000 CPC-300 (Korea)", 0)
COMP(1986, cpc300e,   msx2,     0,      msx2,     msx2kr, msx_state,   msx,     "Daewoo", "IQ-2000 CPC-300E (Korea)", 0)
COMP(1988, cpc400,    msx2,     0,      msx2,     msx2kr, msx_state,   msx,     "Daewoo", "X-II CPC-400 (Korea)", 0 )
COMP(1988, cpc400s,   msx2,     0,      msx2,     msx2kr, msx_state,   msx,     "Daewoo", "X-II CPC-400S (Korea)", 0 )

COMP(1988, msx2p,     0,        msx,    msx2,     msx2jp, msx_state,   msx,     "ASCII & Microsoft", "MSX2+", 0)
COMP(19??, expert3i,  msx2p,    0,      msx2,     msx2, msx_state,     msx,     "Ciel", "Expert 3 IDE", GAME_NOT_WORKING )
COMP(1996, expert3t,  msx2p,    0,      msx2,     msx2, msx_state,     msx,     "Ciel", "Expert 3 Turbo", GAME_NOT_WORKING )
COMP(19??, expertac,  msx2p,    0,      msx2,     msx2, msx_state,     msx,     "Gradiente", "Expert AC88+", GAME_NOT_WORKING )
COMP(19??, expertdx,  msx2p,    0,      msx2,     msx2, msx_state,     msx,     "Gradiente", "Expert DDX+", GAME_NOT_WORKING )
COMP(1988, fsa1fx,    msx2p,    0,      msx2,     msx2jp, msx_state,   msx,     "Panasonic / Matsushita", "FS-A1FX (Japan)", 0 )
COMP(1988, fsa1wx,    msx2p,    0,      msx2,     msx2jp, msx_state,   msx,     "Panasonic / Matsushita", "FS-A1WX / 1st released version (Japan)", 0 )
COMP(1988, fsa1wxa,   msx2p,    0,      msx2,     msx2jp, msx_state,   msx,     "Panasonic / Matsushita", "FS-A1WX / 2nd released version (Japan)", 0 )
COMP(1989, fsa1wsx,   msx2p,    0,      msx2,     msx2jp, msx_state,   msx,     "Panasonic / Matsushita", "FS-A1WSX (Japan)", 0 )
COMP(1988, hbf1xdj,   msx2p,    0,      msx2,     msx2jp, msx_state,   msx,     "Sony", "HB-F1XDJ (Japan)", 0 )
COMP(1989, hbf1xv,    msx2p,    0,      msx2,     msx2jp, msx_state,   msx,     "Sony", "HB-F1XV (Japan)", 0 )
COMP(1988, phc70fd,   msx2p,    0,      msx2,     msx2jp, msx_state,   msx,     "Sanyo", "WAVY PHC-70FD (Japan)", 0 )
COMP(1988, phc70fd2,  msx2p,    0,      msx2,     msx2jp, msx_state,   msx,     "Sanyo", "WAVY PHC-70FD2 (Japan)", 0 )
COMP(1989, phc35j,    msx2p,    0,      msx2,     msx2jp, msx_state,   msx,     "Sanyo", "WAVY PHC-35J (Japan)", 0)
COMP(19??, hbf9sp,    msx2p,    0,      msx2,     msx2jp, msx_state,   msx,     "Sony", "HB-F9S+", GAME_NOT_WORKING)

/* Temporary placeholders */
COMP(19??, fsa1gt,    msx2p,    0,      msx2,     msx2jp, msx_state,   msx,     "Panasonic", "FS-A1GT", GAME_NOT_WORKING)
COMP(19??, fsa1st,    msx2p,    0,      msx2,     msx2jp, msx_state,   msx,     "Panasonic", "FS-A1ST", GAME_NOT_WORKING)
