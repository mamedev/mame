// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Charles MacDonald,Mathis Rosenhauer,Brad Oliver,Michael Luong,Fabio Priuli,Enik Land
/******************************************************************************
 Contributors:

    Marat Fayzullin (MG source)
    Charles MacDonald
    Mathis Rosenhauer
    Brad Oliver
    Michael Luong
    Wilbert Pol
    Fabio Priuli
    Enik Land

 To do:

 - SIO interface for Game Gear (needs netplay, I guess)
 - Support for other DE-9 compatible controllers, like the Mega Drive 6-Button
   that has homebrew software support
 - Sega SK-1100 keyboard support for Sega Mark III (sg1000m3 driver)
 - Link between two Mark III's through keyboard, supported by F-16 Fighting Falcon
 - Mark III expansion slot, used by keyboard and FM Sound Unit
 - Verify if the SN76489 chip can be disabled on sg1000m3 and sms1krfm consoles
 - Rapid button of smsj, sms1krfm and sms1kr consoles
 - Software compatibility flags, by region and/or BIOS
 - Samsung modem for Gam*Boy Securities Information Service System
 - Sega Demo Unit II (kiosk expansion device)
 - SMS 8 slot game changer (kiosk expansion device)
 - SMS Disk System (floppy disk drive expansion device) - unreleased
 - Emulate SRAM cartridges? (for use with Bock's dump tool)

 The Game Gear SIO hardware is not emulated but has some
 placeholders in 'machine/sms.c'

 Changes:
    Apr 02 - Added raster palette effects for SMS & GG (ML)
                 - Added sprite collision (ML)
                 - Added zoomed sprites (ML)
    May 02 - Fixed paging bug (ML)
                 - Fixed sprite and tile priority bug (ML)
                 - Fixed bug #66 (ML)
                 - Fixed bug #78 (ML)
                 - try to implement LCD persistence emulation for GG (ML)
    Jun 10, 02 - Added bios emulation (ML)
    Jun 12, 02 - Added PAL & NTSC systems (ML)
    Jun 25, 02 - Added border emulation (ML)
    Jun 27, 02 - Version bits for Game Gear (bits 6 of port 00) (ML)
    Nov-Dec, 05 - Numerous cleanups, fixes, updates (WP)
    Mar, 07 - More cleanups, fixes, mapper additions, etc (WP)

--------------------------------------------------------------------------------

General compatibility issues on real hardware (not emulation bugs):

- Some ROMs have issues or don't work when running on a console of different
  region;
- Many Japanese, Korean and homebrew ROMs don't have the signature required by
  BIOSes of consoles sold overseas;
- Paddle games need to detect the system region as Japanese to work with the
  Paddle controller;
- Few games of the ones with FM support need to detect the system region as
  Japanese to play FM sound;
- The Light Phaser gun doesn't work with the Japanese SMS;
- There are reports about Light Phaser working on the second Korean SMS
  version, and a Korean advert shows support on the first version (Gam*Boy I,
  although based on Japanese SMS);
- The Korean SMS versions have Japanese-format cartridge slot, but only on the
  first (Gam*Boy I) the region is detected as Japanese;
- Some SMS ROMs don't run when are plugged-in to SMS expansion slot, through
  the gender adapter;
- Some SMS ROMs don't run or have issues when are plugged-in to a Game Gear,
  through the Master Gear adapter;

--------------------------------------------------------------------------------

Sega Master System II
Sega 1990

This particular version was manufactured between July 1991 and October 1992
The PCB is stamped '17 AUG 1991'

PCB Layout
----------
171-5922A
(C) SEGA 1990
IC BD M4Jr. PAL
|----------|------|-----------------------|
| POWER_IN |RF_OUT|       IC1  IC2   IC3  |
|          |      |       Z80  BIOS  6264 |
|          |      | IC9                   |
|          |------| CXA1145               |
|                                         |
|                                         |
|IC8                                      |
|7805                                     |
|                   |---------------|     |
|                   |   CART SLOT   |     |
|                   |---------------|     |
|                                         |
|                                         |
| PAUSE_SW                                |
|                                         |
|      IC6       IC5            IC4       |
|      D4168     315-5246       315-5237  |
|                                         |
|      IC7                                |
|      D4168                              |
|                                         |
| POWER_SW           JOY1 JOY2  53.2034MHz|
|-----------------------------------------|
Notes: (All ICs shown)
       IC1 Z80      - Z0840004PSC Z80 CPU (DIP40). Clock input 3.54689MHz (53.2034/15)
       IC2 BIOS     - 1M 28-pin mask ROM marked MPR-12808 (DIP28)
       IC3 6264     - Samsung KM6264 8k x8 SRAM. Some models have NEC D4168 or Sanyo LC3664 which are compatible (DIP28)
       IC4 315-5237 - Custom Sega I/O controller IC (DIP48)
                      Clocks - Pin 43 - master clock input 53.2034MHz from OSC
                               Pin 41 - 10.6406MHz (53.2034/5)
                               Pin 42 - 4.43361MHz (53.2034/12)
       IC5 315-5246 - Custom Sega Video Display Processor (VDP) (64 pin flat pack)
                      The VDP also contains a Texas Instruments SN76489 sound chip
                      Clocks - Pin 33 - 3.54689MHz (53.2034/15)
                               Pin 34 - 10.6406MHz (53.2034/5)
                               Pin 35 - 10.6406MHz (53.2034/5)
                               Pin 39 - 2.66017MHz (53.2034/20)
       IC6/IC7 D4168- NEC D4168 8k x8 SRAM (DIP28)
       IC8 7805     - Motorola MC7805 voltage regulator (7v to 25v input, 5v output)
       IC9 CXA1145  - Sony CXA1145 RGB to composite video encoder IC (DIP24)
       POWER_IN     - Power input from AC/DC power pack. System requires 9VDC at 500mA. Center pin is negative and
                      outer barrel is positive. Note this is opposite to regular DC power packs
       RF_OUT       - RF modulator with RF signal output to TV
       POWER_SW     - Power on/off switch
       PAUSE_SW     - Push button used to pause the game
       JOY1/JOY2    - Joystick connectors (DB9)
       HSync        - 15.5565kHz
       VSync        - 49.7015Hz


Cart PCB Examples
-----------------
Note! There are many more types of PCBs & custom chip matching variations. This document
is not meant to provide all details of every type in existence. Some games have been
found on different types of ROM boards with and without bankswitching hardware.

Type with no bankswitching hardware

171-5519 \ no visible difference?
171-5519D/
|------------------|
||----------|      |
||  DIP28   |      |
||----------|      |
|IC1               |
|------------------|
Notes:
      DIP28 - 1M mask ROM (DIP28) at location IC1. Actual ROM type is Fujitsu MB831001
              This ROM has a built-in mapper so no bankswitching chip is required.
              The CPU uses pin 22 of the ROM to bankswitch it.
              Found in....
                          Game Name         Sega ROM ID
                          -----------------------------
                          World Grand Prix  MPR-11074
                          Black Belt        MPR-10150
                          Ghost House       MPR-12586


Types with bankswitching hardware

171-5713D (uses 315-5235)
171-5838 (uses 315-5365)
171-5893 (uses 315-5365)
|------------------|
|                  |
|  |------------|  |
|  |   SDIP42   |  |
|  |------------|  |
|               IC1|
||----------|      |
||  DIP32   |      |
||----------|      |
|IC2               |
|------------------|
Notes:
     SDIP42 - Custom Sega bankswitch chip at location IC1. There are several different
              types of these chips with different 315-xxxx numbers
              These include 315-5235 (DIP42), 315-5208 (DIP28) and 315-5365 (DIP42) and possibly others.
      DIP32 - 1MBit/2MBit/4MBit mask ROM (DIP32) at location IC2
              Actual ROM type can be 831000, 831001, 832011, 834000, 834011
              Found in....
                          Game Name         Sega ROM ID   Bank Chip
                          -----------------------------------------
                          Spellcaster       MPR-12532-T   315-5365
                          Altered Beast     MPR-12534     315-5235
                          Bubble Bobble     MPR-14177     315-5365

171-5442 (uses 315-5235)
|--------------------------|
|  |----------------|      |
|  |     DIP40      |      |
|  |----------------|IC2   |
|                          |
|      |------------|      |
|      |   SDIP42   |      |
|      |------------|IC1   |
|----|                |----|
     |                |
     |----------------|
Notes:
      SDIP42 - Custom Sega bankswitch chip at location IC1. There are several different
               types of these chips with different 315-xxxx numbers
               These include 315-5235 (DIP42), 315-5208 (DIP28) and 315-5365 (DIP42) and possibly others.
       DIP40 - 2MBit/4Mbit 16-bit mask ROM (DIP40) at location IC2
               Found in....
                           Game Name         Sega ROM ID   Bank Chip
                           -----------------------------------------
                           Space Harrier     MPR-10410     315-5235

Another ROM board 171-5497 used by Monopoly has 315-5235 DIP42 mapper chip, DIP28 mask ROM, a DIP8 chip (unknown),
DIP28 SRAM (likely 8k) and a 3V coin battery.
Yet another type of ROM board with unknown PCB number used by Phantasy Star has 315-5235 DIP42 mapper chip, DIP32 mask
ROM and DIP28 SRAM (likely 8k) and a 3V coin battery.
Unfortunatley the majority of these ROM boards, ROM types and MPR-xxxxx Sega part numbers are undocumented because they
were mostly dumped from the edge connector without being opened.
Some additional info can be found at http://www.smspower.org/Development/Index
Some excellent SMS documentation can be found at http://cgfm2.emuviews.com/sms.php

--------------------------------------------------------------------------------
SMS Store Unit memory map for the second CPU:

0000-3FFF - BIOS
4000-47FF - RAM
8000      - System Control Register (R/W)
            Reading:
            bit7      - ready (0 = ready, 1 = not ready)
            bit6      - active timer bit switch (0 = timer 2, 1 = timer 1)
            bit5      - unknown
            bit4-bit3 - timer 2 length bit switches (10s-25s)
            bit2-bit0 - timer 1 length bit switches (30s-135s)
            Writing:
            bit7-bit4 - led of selected game to set
            bit3      - unknown, 1 seems to be written all the time
            bit2      - unknown, 1 seems to be written all the time
            bit1      - reset signal for sms cpu, 0 = reset low, 1 = reset high
            bit0      - which cpu receives interrupt signals, 0 = sms cpu, 1 = controlling cpu
C000      - Card/Cartridge selction register (W)
            bit7-bit4 - slot to select
            bit3      - slot type (0 = cartridge, 1 = card ?)
            bit2-bit0 - unknown
C400      - ???? (used once)
D800      - Selection buttons #1, 1-8 (R)
DC00      - Selection buttons #2, 9-16 (R)

 ******************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/sn76496.h"
#include "sound/2413intf.h"
#include "video/315_5124.h"
#include "includes/sms.h"
#include "softlist.h"

#include "sms1.lh"

#define MASTER_CLOCK_PAL    53203425    /* 12 * subcarrier freq. (4.43361875MHz) */


static ADDRESS_MAP_START( sms1_mem, AS_PROGRAM, 8, sms_state )
	AM_RANGE(0x0000, 0xbfff) AM_WRITE(write_cart)
	AM_RANGE(0x0000, 0x3fff) AM_READ(read_0000)
	AM_RANGE(0x4000, 0x7fff) AM_READ(read_4000)
	AM_RANGE(0x8000, 0xbfff) AM_READ(read_8000)
	AM_RANGE(0xc000, 0xfff7) AM_READWRITE(read_ram, write_ram)
	AM_RANGE(0xfff8, 0xfffb) AM_READWRITE(sms_sscope_r, sms_sscope_w)       /* 3-D glasses */
	AM_RANGE(0xfffc, 0xffff) AM_READWRITE(sms_mapper_r, sms_mapper_w)       /* Bankswitch control */
ADDRESS_MAP_END

static ADDRESS_MAP_START( sms_mem, AS_PROGRAM, 8, sms_state )
	AM_RANGE(0x0000, 0xbfff) AM_WRITE(write_cart)
	AM_RANGE(0x0000, 0x3fff) AM_READ(read_0000)
	AM_RANGE(0x4000, 0x7fff) AM_READ(read_4000)
	AM_RANGE(0x8000, 0xbfff) AM_READ(read_8000)
	AM_RANGE(0xc000, 0xfff7) AM_READWRITE(read_ram, write_ram)
	AM_RANGE(0xfffc, 0xffff) AM_READWRITE(sms_mapper_r, sms_mapper_w)       /* Bankswitch control */
ADDRESS_MAP_END

static ADDRESS_MAP_START( sms_store_mem, AS_PROGRAM, 8, smssdisp_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM                     /* BIOS */
	AM_RANGE(0x4000, 0x47ff) AM_RAM                     /* RAM */
	AM_RANGE(0x6000, 0x7fff) AM_READ(store_cart_peek)
	AM_RANGE(0x8000, 0x8000) AM_READ_PORT("DSW") AM_WRITE(sms_store_control_w) /* Control */
	AM_RANGE(0xc000, 0xc000) AM_READWRITE(sms_store_cart_select_r, sms_store_cart_select_w) /* cartridge/card slot selector */
	AM_RANGE(0xd800, 0xd800) AM_READ_PORT("GAMESEL1")         /* Game selector port #1 */
	AM_RANGE(0xdc00, 0xdc00) AM_READ_PORT("GAMESEL2")         /* Game selector port #2 */
ADDRESS_MAP_END

// I/O ports $3E and $3F do not exist on Mark III
static ADDRESS_MAP_START( sg1000m3_io, AS_IO, 8, sms_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x40, 0x7f)                 AM_READWRITE(sms_count_r, sms_psg_w)
	AM_RANGE(0x80, 0x80) AM_MIRROR(0x3e) AM_DEVREADWRITE("sms_vdp", sega315_5124_device, vram_read, vram_write)
	AM_RANGE(0x81, 0x81) AM_MIRROR(0x3e) AM_DEVREADWRITE("sms_vdp", sega315_5124_device, register_read, register_write)
	AM_RANGE(0xc0, 0xc0) AM_MIRROR(0x1e) AM_READ(sms_input_port_dc_r)
	AM_RANGE(0xc1, 0xc1) AM_MIRROR(0x1e) AM_READ(sms_input_port_dd_r)
	AM_RANGE(0xe0, 0xe0) AM_MIRROR(0x0e) AM_READ(sms_input_port_dc_r)
	AM_RANGE(0xe1, 0xe1) AM_MIRROR(0x0e) AM_READ(sms_input_port_dd_r)
	AM_RANGE(0xf0, 0xf0)                 AM_READWRITE(sms_input_port_dc_r, sms_ym2413_register_port_w)
	AM_RANGE(0xf1, 0xf1)                 AM_READWRITE(sms_input_port_dd_r, sms_ym2413_data_port_w)
	AM_RANGE(0xf2, 0xf2)                 AM_READWRITE(sms_audio_control_r, sms_audio_control_w)
	AM_RANGE(0xf3, 0xf3)                 AM_READ(sms_input_port_dd_r)
	AM_RANGE(0xf4, 0xf4) AM_MIRROR(0x02) AM_READ(sms_input_port_dc_r)
	AM_RANGE(0xf5, 0xf5) AM_MIRROR(0x02) AM_READ(sms_input_port_dd_r)
	AM_RANGE(0xf8, 0xf8) AM_MIRROR(0x06) AM_READ(sms_input_port_dc_r)
	AM_RANGE(0xf9, 0xf9) AM_MIRROR(0x06) AM_READ(sms_input_port_dd_r)
ADDRESS_MAP_END


static ADDRESS_MAP_START( sms_io, AS_IO, 8, sms_state )
	AM_IMPORT_FROM(sg1000m3_io)
	AM_RANGE(0x00, 0x00) AM_MIRROR(0x3e) AM_WRITE(sms_mem_control_w)
	AM_RANGE(0x01, 0x01) AM_MIRROR(0x3e) AM_WRITE(sms_io_control_w)
ADDRESS_MAP_END


// It seems the Korean versions do some more strict decoding on the I/O
// addresses.
// At least the mirrors for I/O ports $3E/$3F don't seem to exist there.
// Enri's doc ( http://www43.tok2.com/home/cmpslv/Sms/EnrSms.htm ) about
// the Japanese SMS also doesn't mention them.
// Leaving the mirrors breaks the Korean cartridge bublboky.
static ADDRESS_MAP_START( smsj_io, AS_IO, 8, sms_state )
	AM_IMPORT_FROM(sg1000m3_io)
	AM_RANGE(0x3e, 0x3e)                 AM_WRITE(sms_mem_control_w)
	AM_RANGE(0x3f, 0x3f)                 AM_WRITE(sms_io_control_w)
ADDRESS_MAP_END


// The first Korean SMS version also seems to lack I/O port $3F. Games execute
// a region detection procedure that, through that port, sets the mode used by
// the TH bits of port $DD and tests their behavior. The region of the first SMS
// version is detected as Japanese (opposite to the second version). However,
// as it supports Light Phaser games, it doesn't have the same behavior of the
// Japanese SMS. If it had the behavior of other SMS versions, the system
// region would be detected as Export, so it probably lacks the port.
static ADDRESS_MAP_START( sms1kr_io, AS_IO, 8, sms_state )
	AM_IMPORT_FROM(sg1000m3_io)
	AM_RANGE(0x3e, 0x3e)                 AM_WRITE(sms_mem_control_w)
ADDRESS_MAP_END


// It seems the mirrors for I/O ports $3E/$3F also don't seem to exist on the
// Game Gear. Leaving the mirrors breaks 'gloc' (it freezes after 1st stage).
static ADDRESS_MAP_START( gg_io, AS_IO, 8, sms_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00, 0x00)                 AM_READ(gg_input_port_00_r)
	AM_RANGE(0x01, 0x05)                 AM_READWRITE(gg_sio_r, gg_sio_w)
	AM_RANGE(0x06, 0x06)                 AM_WRITE(gg_psg_stereo_w)
	AM_RANGE(0x3e, 0x3e)                 AM_WRITE(sms_mem_control_w)
	AM_RANGE(0x3f, 0x3f)                 AM_WRITE(sms_io_control_w)
	AM_RANGE(0x40, 0x7f)                 AM_READWRITE(sms_count_r, gg_psg_w)
	AM_RANGE(0x80, 0x80) AM_MIRROR(0x3e) AM_DEVREADWRITE("sms_vdp", sega315_5124_device, vram_read, vram_write)
	AM_RANGE(0x81, 0x81) AM_MIRROR(0x3e) AM_DEVREADWRITE("sms_vdp", sega315_5124_device, register_read, register_write)
	AM_RANGE(0xc0, 0xc0)                 AM_READ(sms_input_port_dc_r)
	AM_RANGE(0xc1, 0xc1)                 AM_READ(sms_input_port_dd_r)
	AM_RANGE(0xdc, 0xdc)                 AM_READ(sms_input_port_dc_r)
	AM_RANGE(0xdd, 0xdd)                 AM_READ(sms_input_port_dd_r)
ADDRESS_MAP_END


static INPUT_PORTS_START( sms )
	PORT_START("PAUSE")
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME(DEF_STR(Pause)) PORT_CODE(KEYCODE_1)
INPUT_PORTS_END

static INPUT_PORTS_START( sg1000m3 )
	PORT_INCLUDE( sms )

	PORT_START("SEGASCOPE")
	PORT_CONFNAME( 0x01, 0x00, "SegaScope (3-D Glasses)" )
	PORT_CONFSETTING( 0x00, DEF_STR( Off ) )
	PORT_CONFSETTING( 0x01, DEF_STR( On ) )

	PORT_START("SSCOPE_BINOCULAR")
	PORT_CONFNAME( 0x03, 0x00, "SegaScope - Binocular Hack" ) PORT_CONDITION("SEGASCOPE", 0x01, EQUALS, 0x01)
	PORT_CONFSETTING( 0x00, DEF_STR( Off ) )
	PORT_CONFSETTING( 0x01, "Left Lens" )
	PORT_CONFSETTING( 0x02, "Right Lens" )
	PORT_CONFSETTING( 0x03, "Both Lens" )
	PORT_BIT( 0x03, 0x00, IPT_UNUSED ) PORT_CONDITION("SEGASCOPE", 0x01, EQUALS, 0x00)
INPUT_PORTS_END

static INPUT_PORTS_START( sms1 )
	PORT_INCLUDE( sg1000m3 )

	PORT_START("RESET")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Reset Button")
INPUT_PORTS_END

static INPUT_PORTS_START( smsj )
	PORT_INCLUDE( sg1000m3 )

	//PORT_START("RAPID")
	//PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Rapid Button") /* Not implemented */
INPUT_PORTS_END

static INPUT_PORTS_START( smssdisp )
	// For each peripheral port (for controllers or 3-D glasses), there are sets
	// of two connectors wired in parallel on the real hardware. This allows to
	// have different controllers, like a pad and a Light Phaser, plugged together
	// for a player input, what avoids having to re-plug them every time a game is
	// changed to another that requires a different controller. Also, this allows
	// 3-D games to be properly watched by two persons at same time.
	// For now the driver just uses single input ports.
	PORT_INCLUDE( sms1 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x07, 0x07, "Timer 1 length" )
	PORT_DIPSETTING( 0x00, "135s" )
	PORT_DIPSETTING( 0x01, "120s" )
	PORT_DIPSETTING( 0x02, "105s" )
	PORT_DIPSETTING( 0x03, "90s" )
	PORT_DIPSETTING( 0x04, "75s" )
	PORT_DIPSETTING( 0x05, "60s" )
	PORT_DIPSETTING( 0x06, "45s" )
	PORT_DIPSETTING( 0x07, "30s" )
	PORT_DIPNAME( 0x18, 0x18, "Timer 2 length" )
	PORT_DIPSETTING( 0x00, "25s" )
	PORT_DIPSETTING( 0x08, "20s" )
	PORT_DIPSETTING( 0x10, "15s" )
	PORT_DIPSETTING( 0x18, "10s" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPSETTING( 0x20, DEF_STR( Off ) )
	PORT_DIPNAME( 0x40, 0x40, "Select Timer" )
	PORT_DIPSETTING( 0x00, "Timer 2" )
	PORT_DIPSETTING( 0x40, "Timer 1" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )   // READY, must be high

	PORT_START("GAMESEL1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Game 03") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Game 02") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Game 01") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Game 00") PORT_CODE(KEYCODE_6)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Game 07") PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Game 06") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Game 05") PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Game 04") PORT_CODE(KEYCODE_7)

	PORT_START("GAMESEL2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Game 11") PORT_CODE(KEYCODE_COMMA)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Game 10") PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Game 09") PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Game 08") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Game 15") PORT_CODE(KEYCODE_STOP)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Game 14") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Game 13") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Game 12") PORT_CODE(KEYCODE_9)
INPUT_PORTS_END

static INPUT_PORTS_START( gg )
	PORT_START("GG_PORT_DC")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("START")
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START ) PORT_NAME("Start") /* Game Gear START */

	PORT_START("PERSISTENCE")
	PORT_CONFNAME( 0x01, 0x01, "LCD Persistence Hack" )
	PORT_CONFSETTING( 0x00, DEF_STR( Off ) )
	PORT_CONFSETTING( 0x01, DEF_STR( On ) )
INPUT_PORTS_END


WRITE_LINE_MEMBER(sms_state::sms_int_callback)
{
	m_maincpu->set_input_line(0, state);
}


static MACHINE_CONFIG_START( sms_ntsc_base, sms_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_53_693175MHz/15)
	MCFG_CPU_PROGRAM_MAP(sms_mem)
	MCFG_CPU_IO_MAP(sms_io)

	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_MACHINE_START_OVERRIDE(sms_state,sms)
	MCFG_MACHINE_RESET_OVERRIDE(sms_state,sms)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("segapsg", SEGAPSG, XTAL_53_693175MHz/15)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	MCFG_SMS_CARTRIDGE_ADD("slot", sms_cart, NULL)

	MCFG_SOFTWARE_LIST_ADD("cart_list","sms")

	MCFG_SMS_CONTROL_PORT_ADD(CONTROL1_TAG, sms_control_port_devices, "joypad")
	MCFG_SMS_CONTROL_PORT_TH_INPUT_HANDLER(WRITELINE(sms_state, sms_ctrl1_th_input))
	MCFG_SMS_CONTROL_PORT_PIXEL_HANDLER(READ32(sms_state, sms_pixel_color))

	MCFG_SMS_CONTROL_PORT_ADD(CONTROL2_TAG, sms_control_port_devices, "joypad")
	MCFG_SMS_CONTROL_PORT_TH_INPUT_HANDLER(WRITELINE(sms_state, sms_ctrl2_th_input))
	MCFG_SMS_CONTROL_PORT_PIXEL_HANDLER(READ32(sms_state, sms_pixel_color))
MACHINE_CONFIG_END

/*
    For SMS drivers, the ratio between CPU and pixel clocks, set through dividers, is 2/3. The
    division that sets the pixel clock, in MCFG_SCREEN_RAW_PARAMS(), results in a remainder
    that is discarded internally. Due to this rounding, the cycle time and the screen pixel
    time, derived from their clocks, do not longer match (inversely) the exact original ratio
    of these clocks. The SMS VDP emulation controls some properties (counters/flags) through
    screen timing, that the core calculates based on the emulation time. The VDP properties
    are read in the CPU timeslice. When a CPU operation that access the VDP is executed, the
    elapsed emulation time is also based on how many CPU cycles have elapsed since start of
    the current timeslice. Depending on this amount of CPU cycles, when the core divides the
    elapsed time by the pixel time, the obtained pixel count may be less than expected. Flubba's
    VDPTest ROM relies on exact results. A workaround is to use an additional macro, for each
    driver, that resets the refresh rate, and by consequence the pixel time, without discarding
    the remainder of the division. If the core is fixed in the future, the MCFG_SCREEN_REFRESH_RATE
    lines after each MCFG_SCREEN_RAW_PARAMS call below can be removed.
*/

static MACHINE_CONFIG_DERIVED( sms2_ntsc, sms_ntsc_base )
	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_53_693175MHz/10, \
		SEGA315_5124_WIDTH , SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH - 2, SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH + 256 + 10, \
		SEGA315_5124_HEIGHT_NTSC, SEGA315_5124_TBORDER_START + SEGA315_5124_NTSC_224_TBORDER_HEIGHT, SEGA315_5124_TBORDER_START + SEGA315_5124_NTSC_224_TBORDER_HEIGHT + 224)
	MCFG_SCREEN_REFRESH_RATE((double) XTAL_53_693175MHz/10 / (SEGA315_5124_WIDTH * SEGA315_5124_HEIGHT_NTSC))
	MCFG_SCREEN_UPDATE_DRIVER(sms_state, screen_update_sms)

	MCFG_DEVICE_ADD("sms_vdp", SEGA315_5246, 0)
	MCFG_SEGA315_5246_SET_SCREEN("screen")
	MCFG_SEGA315_5246_IS_PAL(false)
	MCFG_SEGA315_5246_INT_CB(WRITELINE(sms_state, sms_int_callback))
	MCFG_SEGA315_5246_PAUSE_CB(WRITELINE(sms_state, sms_pause_callback))
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( sms1_ntsc, sms_ntsc_base )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(sms1_mem)  // This adds the SegaScope handlers for 3-D glasses
	MCFG_CPU_IO_MAP(sms_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_53_693175MHz/10, \
		SEGA315_5124_WIDTH, SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH - 2, SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH + 256 + 10, \
		SEGA315_5124_HEIGHT_NTSC, SEGA315_5124_TBORDER_START + SEGA315_5124_NTSC_224_TBORDER_HEIGHT, SEGA315_5124_TBORDER_START + SEGA315_5124_NTSC_224_TBORDER_HEIGHT + 224)
	MCFG_SCREEN_REFRESH_RATE((double) XTAL_53_693175MHz/10 / (SEGA315_5124_WIDTH * SEGA315_5124_HEIGHT_NTSC))
	MCFG_SCREEN_UPDATE_DRIVER(sms_state, screen_update_sms1)

	MCFG_SCREEN_ADD("left_lcd", LCD)    // This is needed for SegaScope Left LCD
	MCFG_SCREEN_RAW_PARAMS(XTAL_53_693175MHz/10, \
		SEGA315_5124_WIDTH, SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH - 2, SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH + 256 + 10, \
		SEGA315_5124_HEIGHT_NTSC, SEGA315_5124_TBORDER_START + SEGA315_5124_NTSC_224_TBORDER_HEIGHT, SEGA315_5124_TBORDER_START + SEGA315_5124_NTSC_224_TBORDER_HEIGHT + 224)
	MCFG_SCREEN_REFRESH_RATE((double) XTAL_53_693175MHz/10 / (SEGA315_5124_WIDTH * SEGA315_5124_HEIGHT_NTSC))
	MCFG_SCREEN_UPDATE_DRIVER(sms_state, screen_update_sms1)

	MCFG_SCREEN_ADD("right_lcd", LCD)   // This is needed for SegaScope Right LCD
	MCFG_SCREEN_RAW_PARAMS(XTAL_53_693175MHz/10, \
		SEGA315_5124_WIDTH, SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH - 2, SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH + 256 + 10, \
		SEGA315_5124_HEIGHT_NTSC, SEGA315_5124_TBORDER_START + SEGA315_5124_NTSC_224_TBORDER_HEIGHT, SEGA315_5124_TBORDER_START + SEGA315_5124_NTSC_224_TBORDER_HEIGHT + 224)
	MCFG_SCREEN_REFRESH_RATE((double) XTAL_53_693175MHz/10 / (SEGA315_5124_WIDTH * SEGA315_5124_HEIGHT_NTSC))
	MCFG_SCREEN_UPDATE_DRIVER(sms_state, screen_update_sms1)

	MCFG_SCREEN_VBLANK_DRIVER(sms_state, screen_vblank_sms1)

	MCFG_DEFAULT_LAYOUT(layout_sms1)

	MCFG_VIDEO_START_OVERRIDE(sms_state,sms1)
	MCFG_VIDEO_RESET_OVERRIDE(sms_state,sms1)

	MCFG_DEVICE_ADD("sms_vdp", SEGA315_5124, 0)
	MCFG_SEGA315_5124_SET_SCREEN("screen")
	MCFG_SEGA315_5124_IS_PAL(false)
	MCFG_SEGA315_5124_INT_CB(WRITELINE(sms_state, sms_int_callback))
	MCFG_SEGA315_5124_PAUSE_CB(WRITELINE(sms_state, sms_pause_callback))

	// card and expansion slots, not present in Master System II
	MCFG_SMS_CARD_ADD("mycard", sms_cart, NULL)
	MCFG_SMS_EXPANSION_ADD("exp", sms_expansion_devices, NULL)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED_CLASS( sms_sdisp, sms1_ntsc, smssdisp_state )

	MCFG_DEVICE_MODIFY("sms_vdp")
	MCFG_SEGA315_5124_INT_CB(WRITELINE(smssdisp_state, sms_store_int_callback))

	MCFG_CPU_ADD("control", Z80, XTAL_53_693175MHz/15)
	MCFG_CPU_PROGRAM_MAP(sms_store_mem)
	/* Both CPUs seem to communicate with the VDP etc? */
	MCFG_CPU_IO_MAP(sms_io)

	MCFG_DEVICE_REMOVE("mycard")
	MCFG_DEVICE_REMOVE("exp")

	MCFG_SMS_CARTRIDGE_ADD("slot2", sms_cart, NULL)
	MCFG_SMS_CARTRIDGE_ADD("slot3", sms_cart, NULL)
	MCFG_SMS_CARTRIDGE_ADD("slot4", sms_cart, NULL)
	MCFG_SMS_CARTRIDGE_ADD("slot5", sms_cart, NULL)
	MCFG_SMS_CARTRIDGE_ADD("slot6", sms_cart, NULL)
	MCFG_SMS_CARTRIDGE_ADD("slot7", sms_cart, NULL)
	MCFG_SMS_CARTRIDGE_ADD("slot8", sms_cart, NULL)
	MCFG_SMS_CARTRIDGE_ADD("slot9", sms_cart, NULL)
	MCFG_SMS_CARTRIDGE_ADD("slot10", sms_cart, NULL)
	MCFG_SMS_CARTRIDGE_ADD("slot11", sms_cart, NULL)
	MCFG_SMS_CARTRIDGE_ADD("slot12", sms_cart, NULL)
	MCFG_SMS_CARTRIDGE_ADD("slot13", sms_cart, NULL)
	MCFG_SMS_CARTRIDGE_ADD("slot14", sms_cart, NULL)
	MCFG_SMS_CARTRIDGE_ADD("slot15", sms_cart, NULL)
	MCFG_SMS_CARTRIDGE_ADD("slot16", sms_cart, NULL)

	MCFG_SMS_CARD_ADD("slot17", sms_cart, NULL)
	MCFG_SMS_CARD_ADD("slot18", sms_cart, NULL)
	MCFG_SMS_CARD_ADD("slot19", sms_cart, NULL)
	MCFG_SMS_CARD_ADD("slot20", sms_cart, NULL)
	MCFG_SMS_CARD_ADD("slot21", sms_cart, NULL)
	MCFG_SMS_CARD_ADD("slot22", sms_cart, NULL)
	MCFG_SMS_CARD_ADD("slot23", sms_cart, NULL)
	MCFG_SMS_CARD_ADD("slot24", sms_cart, NULL)
	MCFG_SMS_CARD_ADD("slot25", sms_cart, NULL)
	MCFG_SMS_CARD_ADD("slot26", sms_cart, NULL)
	MCFG_SMS_CARD_ADD("slot27", sms_cart, NULL)
	MCFG_SMS_CARD_ADD("slot28", sms_cart, NULL)
	MCFG_SMS_CARD_ADD("slot29", sms_cart, NULL)
	MCFG_SMS_CARD_ADD("slot30", sms_cart, NULL)
	MCFG_SMS_CARD_ADD("slot31", sms_cart, NULL)
	MCFG_SMS_CARD_ADD("slot32", sms_cart, NULL)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( sms_pal_base, sms_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MASTER_CLOCK_PAL/15)
	MCFG_CPU_PROGRAM_MAP(sms_mem)
	MCFG_CPU_IO_MAP(sms_io)

	MCFG_QUANTUM_TIME(attotime::from_hz(50))

	MCFG_MACHINE_START_OVERRIDE(sms_state,sms)
	MCFG_MACHINE_RESET_OVERRIDE(sms_state,sms)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("segapsg", SEGAPSG, MASTER_CLOCK_PAL/15)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	MCFG_SMS_CARTRIDGE_ADD("slot", sms_cart, NULL)

	MCFG_SOFTWARE_LIST_ADD("cart_list","sms")

	MCFG_SMS_CONTROL_PORT_ADD(CONTROL1_TAG, sms_control_port_devices, "joypad")
	MCFG_SMS_CONTROL_PORT_TH_INPUT_HANDLER(WRITELINE(sms_state, sms_ctrl1_th_input))
	MCFG_SMS_CONTROL_PORT_PIXEL_HANDLER(READ32(sms_state, sms_pixel_color))

	MCFG_SMS_CONTROL_PORT_ADD(CONTROL2_TAG, sms_control_port_devices, "joypad")
	MCFG_SMS_CONTROL_PORT_TH_INPUT_HANDLER(WRITELINE(sms_state, sms_ctrl2_th_input))
	MCFG_SMS_CONTROL_PORT_PIXEL_HANDLER(READ32(sms_state, sms_pixel_color))
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( sms2_pal, sms_pal_base )

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(MASTER_CLOCK_PAL/10, \
		SEGA315_5124_WIDTH, SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH - 2, SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH + 256 + 10, \
		SEGA315_5124_HEIGHT_PAL, SEGA315_5124_TBORDER_START + SEGA315_5124_PAL_240_TBORDER_HEIGHT, SEGA315_5124_TBORDER_START + SEGA315_5124_PAL_240_TBORDER_HEIGHT + 240)
	MCFG_SCREEN_REFRESH_RATE((double) MASTER_CLOCK_PAL/10 / (SEGA315_5124_WIDTH * SEGA315_5124_HEIGHT_PAL))
	MCFG_SCREEN_UPDATE_DRIVER(sms_state, screen_update_sms)

	MCFG_DEVICE_ADD("sms_vdp", SEGA315_5246, 0)
	MCFG_SEGA315_5246_SET_SCREEN("screen")
	MCFG_SEGA315_5246_IS_PAL(true)
	MCFG_SEGA315_5246_INT_CB(WRITELINE(sms_state, sms_int_callback))
	MCFG_SEGA315_5246_PAUSE_CB(WRITELINE(sms_state, sms_pause_callback))
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( sms1_pal, sms_pal_base )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(sms1_mem)  // This adds the SegaScope handlers for 3-D glasses
	MCFG_CPU_IO_MAP(sms_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(MASTER_CLOCK_PAL/10, \
		SEGA315_5124_WIDTH, SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH - 2, SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH + 256 + 10, \
		SEGA315_5124_HEIGHT_PAL, SEGA315_5124_TBORDER_START + SEGA315_5124_PAL_240_TBORDER_HEIGHT, SEGA315_5124_TBORDER_START + SEGA315_5124_PAL_240_TBORDER_HEIGHT + 240)
	MCFG_SCREEN_REFRESH_RATE((double) MASTER_CLOCK_PAL/10 / (SEGA315_5124_WIDTH * SEGA315_5124_HEIGHT_PAL))
	MCFG_SCREEN_UPDATE_DRIVER(sms_state, screen_update_sms1)

	MCFG_SCREEN_ADD("left_lcd", LCD)    // This is needed for SegaScope Left LCD
	MCFG_SCREEN_RAW_PARAMS(MASTER_CLOCK_PAL/10, \
		SEGA315_5124_WIDTH, SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH - 2, SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH + 256 + 10, \
		SEGA315_5124_HEIGHT_PAL, SEGA315_5124_TBORDER_START + SEGA315_5124_PAL_240_TBORDER_HEIGHT, SEGA315_5124_TBORDER_START + SEGA315_5124_PAL_240_TBORDER_HEIGHT + 240)
	MCFG_SCREEN_REFRESH_RATE((double) MASTER_CLOCK_PAL/10 / (SEGA315_5124_WIDTH * SEGA315_5124_HEIGHT_PAL))
	MCFG_SCREEN_UPDATE_DRIVER(sms_state, screen_update_sms1)

	MCFG_SCREEN_ADD("right_lcd", LCD)   // This is needed for SegaScope Right LCD
	MCFG_SCREEN_RAW_PARAMS(MASTER_CLOCK_PAL/10, \
		SEGA315_5124_WIDTH, SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH - 2, SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH + 256 + 10, \
		SEGA315_5124_HEIGHT_PAL, SEGA315_5124_TBORDER_START + SEGA315_5124_PAL_240_TBORDER_HEIGHT, SEGA315_5124_TBORDER_START + SEGA315_5124_PAL_240_TBORDER_HEIGHT + 240)
	MCFG_SCREEN_REFRESH_RATE((double) MASTER_CLOCK_PAL/10 / (SEGA315_5124_WIDTH * SEGA315_5124_HEIGHT_PAL))
	MCFG_SCREEN_UPDATE_DRIVER(sms_state, screen_update_sms1)

	MCFG_SCREEN_VBLANK_DRIVER(sms_state, screen_vblank_sms1)

	MCFG_DEFAULT_LAYOUT(layout_sms1)

	MCFG_VIDEO_START_OVERRIDE(sms_state,sms1)
	MCFG_VIDEO_RESET_OVERRIDE(sms_state,sms1)

	MCFG_DEVICE_ADD("sms_vdp", SEGA315_5124, 0)
	MCFG_SEGA315_5124_SET_SCREEN("screen")
	MCFG_SEGA315_5124_IS_PAL(true)
	MCFG_SEGA315_5124_INT_CB(WRITELINE(sms_state, sms_int_callback))
	MCFG_SEGA315_5124_PAUSE_CB(WRITELINE(sms_state, sms_pause_callback))

	// card and expansion slots, not present in Master System II
	MCFG_SMS_CARD_ADD("mycard", sms_cart, NULL)
	MCFG_SMS_EXPANSION_ADD("exp", sms_expansion_devices, NULL)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( sms1_kr, sms1_ntsc )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(sms1kr_io)

	MCFG_DEVICE_REMOVE("slot")
	MCFG_SG1000MK3_CARTRIDGE_ADD("slot", sg1000mk3_cart, NULL)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( sms1_krfm, sms1_kr )
	MCFG_SOUND_ADD("ym2413", YM2413, XTAL_53_693175MHz/15)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( smsj, sms1_krfm )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(smsj_io)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( sg1000m3, smsj )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(sg1000m3_io)

	// Mark III does not have TH connected.
	MCFG_SMS_CONTROL_PORT_MODIFY(CONTROL1_TAG)
	MCFG_SMS_CONTROL_PORT_TH_INPUT_HANDLER(NULL)
	MCFG_SMS_CONTROL_PORT_MODIFY(CONTROL2_TAG)
	MCFG_SMS_CONTROL_PORT_TH_INPUT_HANDLER(NULL)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( sms2_kr, sms2_ntsc )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(smsj_io)

	MCFG_DEVICE_REMOVE("slot")
	MCFG_SG1000MK3_CARTRIDGE_ADD("slot", sg1000mk3_cart, NULL)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( gamegear, sms_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_53_693175MHz/15)
	MCFG_CPU_PROGRAM_MAP(sms_mem)
	MCFG_CPU_IO_MAP(gg_io)

	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_MACHINE_START_OVERRIDE(sms_state,sms)
	MCFG_MACHINE_RESET_OVERRIDE(sms_state,sms)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_RAW_PARAMS(XTAL_53_693175MHz/10, \
		SEGA315_5124_WIDTH, SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH + 6*8, SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH + 26*8, \
		SEGA315_5124_HEIGHT_NTSC, SEGA315_5124_TBORDER_START + SEGA315_5124_NTSC_192_TBORDER_HEIGHT + 3*8, SEGA315_5124_TBORDER_START + SEGA315_5124_NTSC_192_TBORDER_HEIGHT + 21*8 )
	MCFG_SCREEN_REFRESH_RATE((double) XTAL_53_693175MHz/10 / (SEGA315_5124_WIDTH * SEGA315_5124_HEIGHT_NTSC))
	MCFG_SCREEN_UPDATE_DRIVER(sms_state, screen_update_gamegear)

	MCFG_VIDEO_START_OVERRIDE(sms_state,gamegear)
	MCFG_VIDEO_RESET_OVERRIDE(sms_state,gamegear)

	MCFG_DEVICE_ADD("sms_vdp", SEGA315_5378, 0)
	MCFG_SEGA315_5378_SET_SCREEN("screen")
	MCFG_SEGA315_5378_IS_PAL(false)
	MCFG_SEGA315_5378_INT_CB(WRITELINE(sms_state, sms_int_callback))
	MCFG_SEGA315_5378_PAUSE_CB(WRITELINE(sms_state, sms_pause_callback))

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker","rspeaker")

	MCFG_SOUND_ADD("gamegear", GAMEGEAR, XTAL_53_693175MHz/15)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.00)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.00)

	/* cartridge */
	MCFG_GG_CARTRIDGE_ADD("slot", gg_cart, NULL)

	MCFG_SOFTWARE_LIST_ADD("cart_list", "gamegear")

	MCFG_GG_EXT_PORT_ADD("ext", gg_ext_port_devices, NULL)
	MCFG_GG_EXT_PORT_TH_INPUT_HANDLER(WRITELINE(sms_state, gg_ext_th_input))
	//MCFG_GG_EXT_PORT_PIXEL_HANDLER(READ32(sms_state, sms_pixel_color)) // only for GG-TV mod
MACHINE_CONFIG_END


ROM_START(sms1)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_FILL(0x0000, 0x4000, 0xff)

	ROM_REGION(0x20000, "user1", 0)
	ROM_SYSTEM_BIOS( 0, "bios13", "US/European BIOS v1.3 (1986)" )
	ROMX_LOAD("mpr-10052.rom", 0x0000, 0x2000, CRC(0072ed54) SHA1(c315672807d8ddb8d91443729405c766dd95cae7), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "hangonsh", "US/European BIOS v2.4 with Hang On and Safari Hunt (1988)" )
	ROMX_LOAD("mpr-11459a.rom", 0x0000, 0x20000, CRC(91e93385) SHA1(9e179392cd416af14024d8f31c981d9ee9a64517), ROM_BIOS(2))
	ROM_SYSTEM_BIOS( 2, "hangon", "US/European BIOS v3.4 with Hang On (1988)" )
	ROMX_LOAD("mpr-11458.rom", 0x0000, 0x20000, CRC(8edf7ac6) SHA1(51fd6d7990f62cd9d18c9ecfc62ed7936169107e), ROM_BIOS(3))
	ROM_SYSTEM_BIOS( 3, "missiled", "US/European BIOS v4.4 with Missile Defense 3D (1988)" )
	ROMX_LOAD("missiled.rom", 0x0000, 0x20000, CRC(e79bb689) SHA1(aa92ae576ca670b00855e278378d89e9f85e0351), ROM_BIOS(4))
	ROM_SYSTEM_BIOS( 4, "v10", "US Master System BIOS v1.0 (prototype)" )
	ROMX_LOAD("v1.0.bin", 0x0000, 0x2000, CRC(72bec693) SHA1(29091ff60ef4c22b1ee17aa21e0e75bac6b36474), ROM_BIOS(5))
	ROM_SYSTEM_BIOS( 5, "proto", "US Master System Prototype BIOS" )
	ROMX_LOAD("m404prot.rom", 0x0000, 0x2000, CRC(1a15dfcc) SHA1(4a06c8e66261611dce0305217c42138b71331701), ROM_BIOS(6))
ROM_END

ROM_START(sms)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_FILL(0x0000, 0x4000, 0xff)

	ROM_REGION(0x20000, "user1", 0)
	ROM_SYSTEM_BIOS( 0, "alexkidd", "US/European BIOS with Alex Kidd in Miracle World (1990)" )
	ROMX_LOAD("mpr-12808.ic2", 0x0000, 0x20000, CRC(cf4a09ea) SHA1(3af7b66248d34eb26da40c92bf2fa4c73a46a051), ROM_BIOS(1)) /* "SEGA // MPR-12808 W63 // 9114E9004" @ IC2 */
ROM_END

ROM_START(smssdisp)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_FILL(0x0000, 0x4000, 0x00)

	ROM_REGION(0x4000, "user1", 0)
	ROM_FILL(0x0000, 0x4000, 0xff)

	ROM_REGION(0x4000, "control", 0)
	ROM_LOAD("smssdisp.rom", 0x0000, 0x4000, CRC(ee2c29ba) SHA1(fc465122134d95363112eb51b9ab71db3576cefd))
ROM_END

ROM_START(sms1pal)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_FILL(0x0000, 0x4000, 0xff)

	ROM_REGION(0x20000, "user1", 0)
	ROM_SYSTEM_BIOS( 0, "bios13", "US/European BIOS v1.3 (1986)" )
	ROMX_LOAD("mpr-10052.rom", 0x0000, 0x2000, CRC(0072ed54) SHA1(c315672807d8ddb8d91443729405c766dd95cae7), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "bios20", "European BIOS v2.0 (198?)" )
	ROMX_LOAD("mpr-10883.rom", 0x0000, 0x2000, CRC(b3d854f8) SHA1(fc7eb9141f38c92bf98d9134816f64b45e811112), ROM_BIOS(2))
	ROM_SYSTEM_BIOS( 2, "hangonsh", "US/European BIOS v2.4 with Hang On and Safari Hunt (1988)" )
	ROMX_LOAD("mpr-11459a.rom", 0x0000, 0x20000, CRC(91e93385) SHA1(9e179392cd416af14024d8f31c981d9ee9a64517), ROM_BIOS(3))
	ROM_SYSTEM_BIOS( 3, "hangon", "Sega Master System - US/European BIOS v3.4 with Hang On (1988)" )
	ROMX_LOAD("mpr-11458.rom", 0x0000, 0x20000, CRC(8edf7ac6) SHA1(51fd6d7990f62cd9d18c9ecfc62ed7936169107e), ROM_BIOS(4))
	ROM_SYSTEM_BIOS( 4, "missiled", "US/European BIOS v4.4 with Missile Defense 3D (1988)" )
	ROMX_LOAD("missiled.rom", 0x0000, 0x20000, CRC(e79bb689) SHA1(aa92ae576ca670b00855e278378d89e9f85e0351), ROM_BIOS(5))
ROM_END

ROM_START(smspal)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_FILL(0x0000, 0x4000, 0xff)

	ROM_REGION(0x40000, "user1", 0)
	ROM_SYSTEM_BIOS( 0, "alexkidd", "US/European BIOS with Alex Kidd in Miracle World (1990)" ) /* PCB Label: SEGA // IC BD M4Jr. PAL" Master System II with 314-5246 (ZIP) VDP and 314-5237 (DIP48) IO */
	ROMX_LOAD("mpr-12808.ic2", 0x0000, 0x20000, CRC(cf4a09ea) SHA1(3af7b66248d34eb26da40c92bf2fa4c73a46a051), ROM_BIOS(1)) /* "SEGA // MPR-12808 W63 // 9114E9004" @ IC2 */
	ROM_SYSTEM_BIOS( 1, "sonic", "European/Brazilian BIOS with Sonic the Hedgehog (1991)" )
	ROMX_LOAD("sonbios.rom", 0x0000, 0x40000, CRC(81c3476b) SHA1(6aca0e3dffe461ba1cb11a86cd4caf5b97e1b8df), ROM_BIOS(2))
ROM_END

ROM_START(sg1000m3)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_FILL(0x0000, 0x4000, 0x00)
ROM_END

ROM_START(smsj) /* PCB Label: "SEGA(R) IC BOARD M4J MAIN // 837-6418"; has "YM2413 // 78 04 71 G" at IC10; Back of pcb has traces marked "171-5541 (C)SEGA 1987 MADE IN JAPAN"
    see http://www.smspower.org/Development/JapaneseSMS837-6418 */
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_FILL(0x0000, 0x4000, 0xff)

	ROM_REGION(0x4000, "user1", 0)
	ROM_SYSTEM_BIOS( 0, "jbios21", "Japanese BIOS v2.1 (1987)" )
	ROMX_LOAD("mpr-11124.ic2", 0x0000, 0x2000, CRC(48d44a13) SHA1(a8c1b39a2e41137835eda6a5de6d46dd9fadbaf2), ROM_BIOS(1)) /* "SONY 7J06 // MPR-11124 // JAPAN 021" @ IC2 */
ROM_END

ROM_START(smskr)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_FILL(0x0000, 0x4000, 0xff)

	ROM_REGION(0x20000, "user1", 0)
	ROM_SYSTEM_BIOS( 0, "akbioskr", "Samsung Gam*Boy II with Alex Kidd in Miracle World (1990)" )
	ROMX_LOAD("akbioskr.rom", 0x000, 0x20000, CRC(9c5bad91) SHA1(2feafd8f1c40fdf1bd5668f8c5c02e5560945b17), ROM_BIOS(1))
ROM_END

ROM_START(gamegear)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_FILL(0x0000, 0x4000, 0x00)

	ROM_REGION(0x0400, "user1", 0)
	ROM_SYSTEM_BIOS( 0, "none", "No BIOS" ) /* gamegear */
	ROM_SYSTEM_BIOS( 1, "majesco", "Majesco BIOS" ) /* gamg */
	ROMX_LOAD("majbios.rom", 0x0000, 0x0400, CRC(0ebea9d4) SHA1(914aa165e3d879f060be77870d345b60cfeb4ede), ROM_BIOS(2))
ROM_END

#define rom_gamegeaj rom_gamegear
#define rom_sms1krfm rom_smsj
#define rom_sms1kr rom_smsj

/***************************************************************************

  Game driver(s)

  US
   - Sega Master System (I) (sms1)
     - prototype (M404) bios - 1986
     - without built-in game v1.3 - 1986
     - built-in Hang On/Safari Hunt v2.4 - 1988
     - built-in Hang On v3.4 - 1988
     - built-in Missile Defense 3-D v4.4 - 1988
     - built-in Hang On/Astro Warrior - 19??
   - Sega Master System II (sms)
     - built-in Alex Kidd in Miracle World - 1990

  JP
   - Sega SG-1000 Mark III (sg1000m3)
     - no bios - 1985
   - Sega Master System (I) (smsj)
     - without built-in game v2.1 - 1987

  KR
   - Samsung Gam*Boy (I) - with FM Chip (sms1krfm)
     - without built-in game v2.1 - 1989
   - Samsung Gam*Boy (I) (sms1kr)
     - without built-in game v2.1 - 19??
   - Samsung Gam*Boy II / Aladdin Boy (smskr)
     - built-in Alex Kidd in Miracle World (Korean) - 1991 (GB II) / 1992 (AB)
  Note about KR:
     - units of Gam*Boy (I) with plug-in AC adaptor have FM and the ones with
       built-in AC adaptor do not.

  EU
   - Sega Master System (I) (sms1pal)
     - without built-in game v1.3 - 1986
     - built-in Hang On/Safari Hunt v2.4 - 1988
     - built-in Hang On v3.4 - 1988
     - built-in Missile Defense 3-D v4.4 - 1988
     - built-in Hang On/Astro Warrior - 19??
   - Sega Master System II (smspal)
     - built-in Alex Kidd in Miracle World - 1990
     - built-in Sonic the Hedgehog - 1991

  BR
   - Tec Toy Master System (I) (same as sms1)
     - built-in Hang On/Safari Hunt v2.4 - 1989
   - Tec Toy Master System II (same as sms1)
     - built-in Alex Kidd in Miracle World - 1991
   - Tec Toy Master System III Compact (same as sms)
     - built-in Alex Kidd in Miracle World - 1992
     - built-in Sonic the Hedgehog - 1993
     - built-in World Cup Italia '90 (Super Futebol II) - 1994
     - built-in Hang On/Safari Hunt v2.4 (blue L.Phaser pack) - 1995
   - Tec Toy Master System Super Compact (no driver)
     - built-in Alex Kidd in Miracle World - 1993
     - built-in Sonic the Hedgehog - 1993
     - built-in World Cup Italia '90 (Super Futebol II) - 1994
   - Tec Toy Master System Girl (no driver)
     - built-in Monica no Castelo do Dragao - 1994
     - built-in Sonic the Hedgehog (T. Monica em O Resgate pack) - 199?
  Notes about BR:
   - PAL-M has same frequency and line count of NTSC
   - Tec Toy later changed its logo twice and its name to Tectoy
   - 20XX models (Handy, Collection, Evolution...) likely have SoC hardware

  These are coin-operated machines (stuff for MAME):

   - Sega Game Box 9
   - Sega Mark III Soft Desk 5
   - Sega Mark III Soft Desk 10
   - Sega Shooting Zone

   The SMS Store Display Unit is labeled PD-W UNIT. Pictures found on Internet
   show cartridges with a label where a not-for-sale message is written along
   the information that it is for use on the Product Display-Working Unit.

***************************************************************************/

/*    YEAR  NAME        PARENT      COMPAT  MACHINE      INPUT     CLASS           INIT      COMPANY     FULLNAME                            FLAGS */
CONS( 1985, sg1000m3,   sms,        0,      sg1000m3,    sg1000m3, sms_state,      sg1000m3, "Sega",     "SG-1000 Mark III",                 MACHINE_SUPPORTS_SAVE )
CONS( 1986, sms1,       sms,        0,      sms1_ntsc,   sms1,     sms_state,      sms1,     "Sega",     "Master System I",                  MACHINE_SUPPORTS_SAVE )
CONS( 1986, sms1pal,    sms,        0,      sms1_pal,    sms1,     sms_state,      sms1,     "Sega",     "Master System I (PAL)" ,           MACHINE_SUPPORTS_SAVE )
CONS( 1986, smssdisp,   sms,        0,      sms_sdisp,   smssdisp, smssdisp_state, smssdisp, "Sega",     "Master System Store Display Unit", MACHINE_SUPPORTS_SAVE )
CONS( 1987, smsj,       sms,        0,      smsj,        smsj,     sms_state,      smsj,     "Sega",     "Master System (Japan)",            MACHINE_SUPPORTS_SAVE )
CONS( 1990, sms,        0,          0,      sms2_ntsc,   sms,      sms_state,      sms1,     "Sega",     "Master System II",                 MACHINE_SUPPORTS_SAVE )
CONS( 1990, smspal,     sms,        0,      sms2_pal,    sms,      sms_state,      sms1,     "Sega",     "Master System II (PAL)",           MACHINE_SUPPORTS_SAVE )
CONS( 1989, sms1krfm,   sms,        0,      sms1_krfm,   smsj,     sms_state,      sms1krfm, "Samsung",  "Gam*Boy I - FM (Korea)",           MACHINE_SUPPORTS_SAVE )
CONS( 19??, sms1kr,     sms,        0,      sms1_kr,     smsj,     sms_state,      sms1kr,   "Samsung",  "Gam*Boy I (Korea)",                MACHINE_SUPPORTS_SAVE )
CONS( 1991, smskr,      sms,        0,      sms2_kr,     sms,      sms_state,      smskr,    "Samsung",  "Gam*Boy II (Korea)",               MACHINE_SUPPORTS_SAVE )

CONS( 1991, gamegear,   0,          sms,    gamegear,    gg,       sms_state,      gamegear, "Sega",     "Game Gear (Europe/America)",       MACHINE_SUPPORTS_SAVE )
CONS( 1990, gamegeaj,   gamegear,   0,      gamegear,    gg,       sms_state,      gamegeaj, "Sega",     "Game Gear (Japan)",                MACHINE_SUPPORTS_SAVE )
