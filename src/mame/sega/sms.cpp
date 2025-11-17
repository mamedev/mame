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
 - On sms1kr (without FM), verify if Rapid uses C-Sync and PSG can be muted like on smsj
 - Accurate GG vertical scaling in SMS mode (h-scaling seems right for subpixel rendering)
 - Software compatibility flags, by region and/or BIOS
 - Samsung modem for Gam*Boy Securities Information Service System
 - Sega Demo Unit II (SMS kiosk-like expansion device)
 - SMS 8 slot game changer (kiosk-like expansion device)
 - Sega Floppy Disc Unit (SMS expansion device) - unreleased
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
- Few games of the ones with FM support need to detect the system region as
  Japanese to play FM sound;
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
#include "sms.h"

#include "bus/sms_ctrl/controllers.h"
#include "cpu/z80/z80.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "sms1.lh"


#define MASTER_CLOCK_GG     32215905.0
#define MASTER_CLOCK_PALN   10746168.0
// The clocks for PAL and PAL-M used here differ from their nominal values,
// because with the latter errors will occur on Flubba's VDPTest, probably
// due to rounding issues in the core.
// Nominal XTAL value for SMS PAL-M (Brazil) is 10726834.
#define MASTER_CLOCK_PALM   10726833.0
// Nominal XTAL value for SMS PAL is 53203424.
#define MASTER_CLOCK_PAL    53203425.0  /* 12 * subcarrier freq. (4.43361875MHz) */


void sms1_state::sms1_mem(address_map &map)
{
	map(0x0000, 0xbfff).w(FUNC(sms1_state::write_cart));
	map(0x0000, 0x3fff).r(FUNC(sms1_state::read_0000));
	map(0x4000, 0x7fff).r(FUNC(sms1_state::read_4000));
	map(0x8000, 0xbfff).r(FUNC(sms1_state::read_8000));
	map(0xc000, 0xfff7).rw(FUNC(sms1_state::read_ram), FUNC(sms1_state::write_ram));
	map(0xfff8, 0xfffb).rw(FUNC(sms1_state::sscope_r), FUNC(sms1_state::sscope_w));             /* 3-D glasses */
	map(0xfffc, 0xffff).rw(FUNC(sms1_state::sms_mapper_r), FUNC(sms1_state::sms_mapper_w));     /* Bankswitch control */
}

void sms_state::sms_mem(address_map &map)
{
	map(0x0000, 0xbfff).w(FUNC(sms_state::write_cart));
	map(0x0000, 0x3fff).r(FUNC(sms_state::read_0000));
	map(0x4000, 0x7fff).r(FUNC(sms_state::read_4000));
	map(0x8000, 0xbfff).r(FUNC(sms_state::read_8000));
	map(0xc000, 0xfffb).rw(FUNC(sms_state::read_ram), FUNC(sms_state::write_ram));
	map(0xfffc, 0xffff).rw(FUNC(sms_state::sms_mapper_r), FUNC(sms_state::sms_mapper_w));       /* Bankswitch control */
}

void smssdisp_state::sms_store_mem(address_map &map)
{
	map(0x0000, 0x3fff).rom();                     /* BIOS */
	map(0x4000, 0x47ff).ram();                     /* RAM */
	map(0x6000, 0x7fff).r(FUNC(smssdisp_state::store_cart_peek));
	map(0x8000, 0x8000).portr("DSW").w(FUNC(smssdisp_state::sms_store_control_w)); /* Control */
	map(0xc000, 0xc000).rw(FUNC(smssdisp_state::sms_store_cart_select_r), FUNC(smssdisp_state::sms_store_cart_select_w)); /* cartridge/card slot selector */
	map(0xd800, 0xd800).portr("GAMESEL1");         /* Game selector port #1 */
	map(0xdc00, 0xdc00).portr("GAMESEL2");         /* Game selector port #2 */
}

// I/O ports $3E and $3F do not exist on Mark III
void sg1000m3_state::sg1000m3_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();

	map(0x40, 0x7f).r(FUNC(sg1000m3_state::sms_count_r)).w(m_vdp, FUNC(sega315_5124_device::psg_w));
	map(0x80, 0x80).mirror(0x3e).rw(m_vdp, FUNC(sega315_5124_device::data_read), FUNC(sega315_5124_device::data_write));
	map(0x81, 0x81).mirror(0x3e).rw(m_vdp, FUNC(sega315_5124_device::control_read), FUNC(sega315_5124_device::control_write));
	map(0xc0, 0xc7).mirror(0x38).rw(FUNC(sg1000m3_state::sg1000m3_peripheral_r), FUNC(sg1000m3_state::sg1000m3_peripheral_w));
}

void sms_state::sms_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();

	map(0x00, 0x00).mirror(0x3e).w(FUNC(sms_state::sms_mem_control_w));
	map(0x01, 0x01).mirror(0x3e).w(FUNC(sms_state::sms_io_control_w));
	map(0x40, 0x7f).r(FUNC(sms_state::sms_count_r)).w(m_vdp, FUNC(sega315_5124_device::psg_w));
	map(0x80, 0x80).mirror(0x3e).rw(m_vdp, FUNC(sega315_5124_device::data_read), FUNC(sega315_5124_device::data_write));
	map(0x81, 0x81).mirror(0x3e).rw(m_vdp, FUNC(sega315_5124_device::control_read), FUNC(sega315_5124_device::control_write));
	map(0xc0, 0xc0).mirror(0x3e).r(FUNC(sms_state::sms_input_port_dc_r));
	map(0xc1, 0xc1).mirror(0x3e).r(FUNC(sms_state::sms_input_port_dd_r));
}


// It seems the Korean versions do some more strict decoding on the I/O addresses.
// At least the mirrors for I/O ports $3E/$3F don't seem to exist there.
// Leaving the mirrors breaks the Korean cartridge bublboky.
void sms_state::smskr_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();

	map(0x3e, 0x3e).w(FUNC(sms_state::sms_mem_control_w));
	map(0x3f, 0x3f).w(FUNC(sms_state::sms_io_control_w));
	map(0x40, 0x7f).r(FUNC(sms_state::sms_count_r)).w(m_vdp, FUNC(sega315_5124_device::psg_w));
	map(0x80, 0x80).mirror(0x3e).rw(m_vdp, FUNC(sega315_5124_device::data_read), FUNC(sega315_5124_device::data_write));
	map(0x81, 0x81).mirror(0x3e).rw(m_vdp, FUNC(sega315_5124_device::control_read), FUNC(sega315_5124_device::control_write));
	map(0xc0, 0xc0).mirror(0x3e).r(FUNC(sms_state::sms_input_port_dc_r));
	map(0xc1, 0xc1).mirror(0x3e).r(FUNC(sms_state::sms_input_port_dd_r));
}


// Mirrors for I/O ports $3E/$3F don't exist on the Japanese SMS.
// Also, $C0/$C1 are the only mirrors for I/O ports $DC/$DD.
void sms_state::smsj_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();

	map(0x3e, 0x3e).w(FUNC(sms_state::sms_mem_control_w));
	map(0x3f, 0x3f).w(FUNC(sms_state::sms_io_control_w));
	map(0x40, 0x7f).r(FUNC(sms_state::sms_count_r)).w(m_vdp, FUNC(sega315_5124_device::psg_w));
	map(0x80, 0x80).mirror(0x3e).rw(m_vdp, FUNC(sega315_5124_device::data_read), FUNC(sega315_5124_device::data_write));
	map(0x81, 0x81).mirror(0x3e).rw(m_vdp, FUNC(sega315_5124_device::control_read), FUNC(sega315_5124_device::control_write));
	map(0xc0, 0xc0).r(FUNC(sms_state::sms_input_port_dc_r));
	map(0xc1, 0xc1).r(FUNC(sms_state::sms_input_port_dd_r));
	map(0xdc, 0xdc).r(FUNC(sms_state::sms_input_port_dc_r));
	map(0xdd, 0xdd).r(FUNC(sms_state::sms_input_port_dd_r));
	map(0xf0, 0xf0).w(FUNC(sms_state::smsj_ym2413_register_port_w));
	map(0xf1, 0xf1).w(FUNC(sms_state::smsj_ym2413_data_port_w));
	map(0xf2, 0xf2).rw(FUNC(sms_state::smsj_audio_control_r), FUNC(sms_state::smsj_audio_control_w));
}


// It seems the mirrors for I/O ports $3E/$3F also don't seem to exist on the
// Game Gear. Leaving the mirrors breaks 'gloc' (it freezes after 1st stage).
void gamegear_state::gg_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();

	map(0x3e, 0x3e).w(FUNC(gamegear_state::sms_mem_control_w)); // TODO: only really exists in Master System mode
	map(0x40, 0x7f).r(FUNC(gamegear_state::sms_count_r)).w(m_vdp, FUNC(sega315_5124_device::psg_w));
	map(0x80, 0x80).mirror(0x3e).rw(m_vdp, FUNC(sega315_5124_device::data_read), FUNC(sega315_5124_device::data_write));
	map(0x81, 0x81).mirror(0x3e).rw(m_vdp, FUNC(sega315_5124_device::control_read), FUNC(sega315_5124_device::control_write));
	map(0xc0, 0xc0).r(FUNC(gamegear_state::gg_input_port_dc_r));
	map(0xc1, 0xc1).r(FUNC(gamegear_state::gg_input_port_dd_r));
	map(0xdc, 0xdc).r(FUNC(gamegear_state::gg_input_port_dc_r));
	map(0xdd, 0xdd).r(FUNC(gamegear_state::gg_input_port_dd_r));

	map(0x00, 0x3f).view(m_io_view);

	// Game Gear mode
	m_io_view[0](0x00, 0x00).r(FUNC(gamegear_state::gg_input_port_00_r));
	m_io_view[0](0x01, 0x01).rw(m_gg_ioport, FUNC(gamegear_io_port_device::data_r), FUNC(gamegear_io_port_device::data_w));
	m_io_view[0](0x02, 0x02).rw(m_gg_ioport, FUNC(gamegear_io_port_device::ctrl_r), FUNC(gamegear_io_port_device::ctrl_w));
	m_io_view[0](0x03, 0x03).rw(m_gg_ioport, FUNC(gamegear_io_port_device::txdata_r), FUNC(gamegear_io_port_device::txdata_w));
	m_io_view[0](0x04, 0x04).r(m_gg_ioport, FUNC(gamegear_io_port_device::rxdata_r));
	m_io_view[0](0x05, 0x05).rw(m_gg_ioport, FUNC(gamegear_io_port_device::s_ctrl_r), FUNC(gamegear_io_port_device::s_ctrl_w));
	m_io_view[0](0x06, 0x06).w(m_vdp, FUNC(sega315_5124_device::psg_stereo_w));

	// Master System mode
	m_io_view[1](0x3f, 0x3f).w(FUNC(gamegear_state::gg_io_control_w));
}


static INPUT_PORTS_START( sms )
	PORT_START("PAUSE")
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME(DEF_STR(Pause)) PORT_CODE(KEYCODE_1) PORT_WRITE_LINE_DEVICE_MEMBER("sms_vdp", FUNC(sega315_5124_device::n_nmi_in_write))
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

	PORT_START("RAPID")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Rapid Button")
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
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START ) PORT_NAME("Start")

	PORT_START("PERSISTENCE")
	PORT_CONFNAME( 0x01, 0x01, "LCD Persistence Hack" )
	PORT_CONFSETTING( 0x00, DEF_STR( Off ) )
	PORT_CONFSETTING( 0x01, DEF_STR( On ) )
INPUT_PORTS_END


void sms_state::sms_base(machine_config &config)
{
	SPEAKER(config, "mono").front_center();

	SMS_CART_SLOT(config, "slot", sms_cart, nullptr);

	SOFTWARE_LIST(config, "cart_list").set_original("sms");

	SMS_CONTROL_PORT(config, m_port_ctrl1, sms_control_port_devices, SMS_CTRL_OPTION_JOYPAD);
	m_port_ctrl1->set_screen(m_main_scr);
	m_port_ctrl1->th_handler().set(FUNC(sms_state::sms_ctrl1_th_input));

	SMS_CONTROL_PORT(config, m_port_ctrl2, sms_control_port_devices, SMS_CTRL_OPTION_JOYPAD);
	m_port_ctrl2->set_screen(m_main_scr);
	m_port_ctrl2->th_handler().set(FUNC(sms_state::sms_ctrl2_th_input));
}

void sms_state::sms_ntsc_base(machine_config &config)
{
	sms_base(config);

	Z80(config, m_maincpu, XTAL(10'738'635)/3);
	m_maincpu->set_addrmap(AS_PROGRAM, &sms_state::sms_mem);
	m_maincpu->set_addrmap(AS_IO, &sms_state::sms_io);

	config.set_maximum_quantum(attotime::from_hz(60));
}

/*
    For SMS drivers, the ratio between CPU and pixel clocks, set through dividers, is 2/3. The
    division that sets the pixel clock, in screen.set_raw(), results in a remainder
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
    the remainder of the division. If the core is fixed in the future, the screen.set_refresh_hz
    lines after each screen.set_raw call below can be removed.
*/

template <typename X>
void sms_state::screen_sms_pal_raw_params(screen_device &screen, X &&pixelclock)
{
	screen.set_raw(pixelclock,
		sega315_5124_device::WIDTH,
		sega315_5124_device::LBORDER_START + sega315_5124_device::LBORDER_WIDTH - 2,
		sega315_5124_device::LBORDER_START + sega315_5124_device::LBORDER_WIDTH + 256 + 10,
		sega315_5124_device::HEIGHT_PAL,
		sega315_5124_device::TBORDER_START + sega315_5124_device::PAL_240_TBORDER_HEIGHT,
		sega315_5124_device::TBORDER_START + sega315_5124_device::PAL_240_TBORDER_HEIGHT + 240);
	screen.set_refresh_hz(pixelclock / (sega315_5124_device::WIDTH * sega315_5124_device::HEIGHT_PAL));
}

template <typename X>
void sms_state::screen_sms_ntsc_raw_params(screen_device &screen, X &&pixelclock)
{
	screen.set_raw(pixelclock,
		sega315_5124_device::WIDTH,
		sega315_5124_device::LBORDER_START + sega315_5124_device::LBORDER_WIDTH - 2,
		sega315_5124_device::LBORDER_START + sega315_5124_device::LBORDER_WIDTH + 256 + 10,
		sega315_5124_device::HEIGHT_NTSC,
		sega315_5124_device::TBORDER_START + sega315_5124_device::NTSC_224_TBORDER_HEIGHT,
		sega315_5124_device::TBORDER_START + sega315_5124_device::NTSC_224_TBORDER_HEIGHT + 224);
	screen.set_refresh_hz(pixelclock / (sega315_5124_device::WIDTH * sega315_5124_device::HEIGHT_NTSC));
}

template <typename X>
void gamegear_state::screen_gg_raw_params(screen_device &screen, X &&pixelclock)
{
	screen.set_raw(pixelclock,
		sega315_5124_device::WIDTH,
		sega315_5124_device::LBORDER_START + sega315_5124_device::LBORDER_WIDTH + 6*8,
		sega315_5124_device::LBORDER_START + sega315_5124_device::LBORDER_WIDTH + 26*8,
		sega315_5124_device::HEIGHT_NTSC,
		sega315_5124_device::TBORDER_START + sega315_5124_device::NTSC_192_TBORDER_HEIGHT + 3*8,
		sega315_5124_device::TBORDER_START + sega315_5124_device::NTSC_192_TBORDER_HEIGHT + 21*8 );
	screen.set_refresh_hz(pixelclock / (sega315_5124_device::WIDTH * sega315_5124_device::HEIGHT_NTSC));
}


void sms_state::sms2_ntsc(machine_config &config)
{
	sms_ntsc_base(config);
	/* video hardware */
	SCREEN(config, m_main_scr, SCREEN_TYPE_RASTER);
	screen_sms_ntsc_raw_params(*m_main_scr, XTAL(10'738'635)/2);
	m_main_scr->set_screen_update(FUNC(sms_state::screen_update_sms));

	SEGA315_5246(config, m_vdp, XTAL(10'738'635));
	m_vdp->set_screen(m_main_scr);
	m_vdp->set_is_pal(false);
	m_vdp->n_int().set_inputline(m_maincpu, 0);
	m_vdp->n_nmi().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_vdp->add_route(ALL_OUTPUTS, "mono", 1.00);

	m_has_bios_full = true;
}


void sms1_state::sms1_ntsc(machine_config &config)
{
	sms_ntsc_base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &sms1_state::sms1_mem);  // This adds the SegaScope handlers for 3-D glasses

	/* video hardware */
	SCREEN(config, m_main_scr, SCREEN_TYPE_RASTER);
	screen_sms_ntsc_raw_params(*m_main_scr, XTAL(10'738'635)/2);
	m_main_scr->set_screen_update(FUNC(sms1_state::screen_update_sms));

	SCREEN(config, m_left_lcd, SCREEN_TYPE_LCD);    // This is needed for SegaScope Left LCD
	screen_sms_ntsc_raw_params(*m_left_lcd, XTAL(10'738'635)/2);
	m_left_lcd->set_screen_update(FUNC(sms1_state::screen_update_left));

	SCREEN(config, m_right_lcd, SCREEN_TYPE_LCD);   // This is needed for SegaScope Right LCD
	screen_sms_ntsc_raw_params(*m_right_lcd, XTAL(10'738'635)/2);
	m_right_lcd->set_screen_update(FUNC(sms1_state::screen_update_right));

	m_main_scr->screen_vblank().set(FUNC(sms1_state::sscope_vblank));

	config.set_default_layout(layout_sms1);

	SEGA315_5124(config, m_vdp, XTAL(10'738'635));
	m_vdp->set_screen(m_main_scr);
	m_vdp->set_is_pal(false);
	m_vdp->n_int().set_inputline(m_maincpu, 0);
	m_vdp->n_nmi().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_vdp->add_route(ALL_OUTPUTS, "mono", 1.00);

	// card and expansion slots, not present in Master System II
	SMS_CARD_SLOT(config, "mycard", sms_cart, nullptr);
	SMS_EXPANSION_SLOT(config, "smsexp", sms_expansion_devices, nullptr);

	m_has_bios_full = true;
	m_has_pwr_led = true;
}

void smssdisp_state::sms_sdisp(machine_config &config)
{
	sms1_ntsc(config);

	m_vdp->n_int().set(FUNC(smssdisp_state::sms_store_int_callback));

	Z80(config, m_control_cpu, XTAL(10'738'635)/3);
	m_control_cpu->set_addrmap(AS_PROGRAM, &smssdisp_state::sms_store_mem);
	/* Both CPUs seem to communicate with the VDP etc? */
	m_control_cpu->set_addrmap(AS_IO, &smssdisp_state::sms_io);

	config.device_remove("mycard");
	config.device_remove("smsexp");

	for (int i = 1; i < 16; i++)
		SMS_CART_SLOT(config, m_slots[i], sms_cart, nullptr);
	for (int i = 0; i < 16; i++)
		SMS_CARD_SLOT(config, m_cards[i], sms_cart, nullptr);

	m_has_bios_full = false;
	m_has_pwr_led = false;
}

void sms_state::sms_pal_base(machine_config &config)
{
	sms_base(config);
	/* basic machine hardware */
	Z80(config, m_maincpu, MASTER_CLOCK_PAL/15);
	m_maincpu->set_addrmap(AS_PROGRAM, &sms_state::sms_mem);
	m_maincpu->set_addrmap(AS_IO, &sms_state::sms_io);

	config.set_maximum_quantum(attotime::from_hz(50));
}

void sms_state::sms2_pal(machine_config &config)
{
	sms_pal_base(config);

	/* video hardware */
	SCREEN(config, m_main_scr, SCREEN_TYPE_RASTER);
	screen_sms_pal_raw_params(*m_main_scr, MASTER_CLOCK_PAL/10);
	m_main_scr->set_screen_update(FUNC(sms_state::screen_update_sms));

	SEGA315_5246(config, m_vdp, MASTER_CLOCK_PAL/5);
	m_vdp->set_screen(m_main_scr);
	m_vdp->set_is_pal(true);
	m_vdp->n_int().set_inputline(m_maincpu, 0);
	m_vdp->n_nmi().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_vdp->add_route(ALL_OUTPUTS, "mono", 1.00);

	m_has_bios_full = true;
}

void sms1_state::sms1_pal(machine_config &config)
{
	sms_pal_base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &sms1_state::sms1_mem);  // This adds the SegaScope handlers for 3-D glasses

	/* video hardware */
	SCREEN(config, m_main_scr, SCREEN_TYPE_RASTER);
	screen_sms_pal_raw_params(*m_main_scr, MASTER_CLOCK_PAL/10);
	m_main_scr->set_screen_update(FUNC(sms1_state::screen_update_sms));

	SCREEN(config, m_left_lcd, SCREEN_TYPE_LCD);    // This is needed for SegaScope Left LCD
	screen_sms_pal_raw_params(*m_left_lcd, MASTER_CLOCK_PAL/10);
	m_left_lcd->set_screen_update(FUNC(sms1_state::screen_update_left));

	SCREEN(config, m_right_lcd, SCREEN_TYPE_LCD);   // This is needed for SegaScope Right LCD
	screen_sms_pal_raw_params(*m_right_lcd, MASTER_CLOCK_PAL/10);
	m_right_lcd->set_screen_update(FUNC(sms1_state::screen_update_right));

	m_main_scr->screen_vblank().set(FUNC(sms1_state::sscope_vblank));

	config.set_default_layout(layout_sms1);

	SEGA315_5124(config, m_vdp, MASTER_CLOCK_PAL/5);
	m_vdp->set_screen(m_main_scr);
	m_vdp->set_is_pal(true);
	m_vdp->n_int().set_inputline(m_maincpu, 0);
	m_vdp->n_nmi().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_vdp->add_route(ALL_OUTPUTS, "mono", 1.00);

	// card and expansion slots, not present in Master System II
	SMS_CARD_SLOT(config, "mycard", sms_cart, nullptr);
	SMS_EXPANSION_SLOT(config, "smsexp", sms_expansion_devices, nullptr);

	m_has_bios_full = true;
	m_has_pwr_led = true;
}


void sms_state::sms_paln_base(machine_config &config)
{
	sms_base(config);
	/* basic machine hardware */
	Z80(config, m_maincpu, MASTER_CLOCK_PALN/3);
	m_maincpu->set_addrmap(AS_PROGRAM, &sms_state::sms_mem);
	m_maincpu->set_addrmap(AS_IO, &sms_state::sms_io);

	config.set_maximum_quantum(attotime::from_hz(50));
}

void sms_state::sms3_paln(machine_config &config)
{
	sms_paln_base(config);

	/* video hardware */
	SCREEN(config, m_main_scr, SCREEN_TYPE_RASTER);
	screen_sms_pal_raw_params(*m_main_scr, MASTER_CLOCK_PALN/2);
	m_main_scr->set_screen_update(FUNC(sms_state::screen_update_sms));

	SEGA315_5246(config, m_vdp, MASTER_CLOCK_PALN);
	m_vdp->set_screen(m_main_scr);
	m_vdp->set_is_pal(true);
	m_vdp->n_int().set_inputline(m_maincpu, 0);
	m_vdp->n_nmi().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_vdp->add_route(ALL_OUTPUTS, "mono", 1.00);

	m_has_bios_full = true;
}

void sms1_state::sms1_paln(machine_config &config)
{
	sms_paln_base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &sms1_state::sms1_mem);  // This adds the SegaScope handlers for 3-D glasses

	/* video hardware */
	SCREEN(config, m_main_scr, SCREEN_TYPE_RASTER);
	screen_sms_pal_raw_params(*m_main_scr, MASTER_CLOCK_PALN/2);
	m_main_scr->set_screen_update(FUNC(sms1_state::screen_update_sms));

	SCREEN(config, m_left_lcd, SCREEN_TYPE_LCD);    // This is needed for SegaScope Left LCD
	screen_sms_pal_raw_params(*m_left_lcd, MASTER_CLOCK_PALN/2);
	m_left_lcd->set_screen_update(FUNC(sms1_state::screen_update_left));

	SCREEN(config, m_right_lcd, SCREEN_TYPE_LCD);   // This is needed for SegaScope Right LCD
	screen_sms_pal_raw_params(*m_right_lcd, MASTER_CLOCK_PALN/2);
	m_right_lcd->set_screen_update(FUNC(sms1_state::screen_update_right));

	m_main_scr->screen_vblank().set(FUNC(sms1_state::sscope_vblank));

	config.set_default_layout(layout_sms1);

	SEGA315_5124(config, m_vdp, MASTER_CLOCK_PALN);
	m_vdp->set_screen(m_main_scr);
	m_vdp->set_is_pal(true);
	m_vdp->n_int().set_inputline(m_maincpu, 0);
	m_vdp->n_nmi().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_vdp->add_route(ALL_OUTPUTS, "mono", 1.00);

	// card and expansion slots, not present in Tec Toy Master System III
	SMS_CARD_SLOT(config, "mycard", sms_cart, nullptr);
	SMS_EXPANSION_SLOT(config, "smsexp", sms_expansion_devices, nullptr);

	m_has_bios_full = true;
	m_has_pwr_led = true;
}


void sms_state::sms_br_base(machine_config &config)
{
	sms_base(config);
	/* basic machine hardware */
	Z80(config, m_maincpu, MASTER_CLOCK_PALM/3);
	m_maincpu->set_addrmap(AS_PROGRAM, &sms_state::sms_mem);
	m_maincpu->set_addrmap(AS_IO, &sms_state::sms_io);

	// PAL-M has near the same frequency of NTSC
	config.set_maximum_quantum(attotime::from_hz(60));
}

void sms_state::sms3_br(machine_config &config)
{
	sms_br_base(config);
	/* video hardware */
	// PAL-M height/width parameters are the same of NTSC screens.
	SCREEN(config, m_main_scr, SCREEN_TYPE_RASTER);
	screen_sms_ntsc_raw_params(*m_main_scr, MASTER_CLOCK_PALM/2);
	m_main_scr->set_screen_update(FUNC(sms_state::screen_update_sms));

	SEGA315_5246(config, m_vdp, MASTER_CLOCK_PALM);
	m_vdp->set_screen(m_main_scr);
	m_vdp->set_is_pal(false); // PAL-M has same line count of NTSC
	m_vdp->n_int().set_inputline(m_maincpu, 0);
	m_vdp->n_nmi().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_vdp->add_route(ALL_OUTPUTS, "mono", 1.00);

	m_has_bios_full = true;
}

void sms1_state::sms1_br(machine_config &config)
{
	sms_br_base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &sms1_state::sms1_mem);  // This adds the SegaScope handlers for 3-D glasses

	/* video hardware */
	// PAL-M height/width parameters are the same of NTSC screens.
	SCREEN(config, m_main_scr, SCREEN_TYPE_RASTER);
	screen_sms_ntsc_raw_params(*m_main_scr, MASTER_CLOCK_PALM/2);
	m_main_scr->set_screen_update(FUNC(sms1_state::screen_update_sms));

	SCREEN(config, m_left_lcd, SCREEN_TYPE_LCD);    // This is needed for SegaScope Left LCD
	screen_sms_ntsc_raw_params(*m_left_lcd, MASTER_CLOCK_PALM/2);
	m_left_lcd->set_screen_update(FUNC(sms1_state::screen_update_left));

	SCREEN(config, m_right_lcd, SCREEN_TYPE_LCD);   // This is needed for SegaScope Right LCD
	screen_sms_ntsc_raw_params(*m_right_lcd, MASTER_CLOCK_PALM/2);
	m_right_lcd->set_screen_update(FUNC(sms1_state::screen_update_right));

	m_main_scr->screen_vblank().set(FUNC(sms1_state::sscope_vblank));

	config.set_default_layout(layout_sms1);

	SEGA315_5124(config, m_vdp, MASTER_CLOCK_PALM);
	m_vdp->set_screen(m_main_scr);
	m_vdp->set_is_pal(false); // PAL-M has same line count of NTSC
	m_vdp->n_int().set_inputline(m_maincpu, 0);
	m_vdp->n_nmi().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_vdp->add_route(ALL_OUTPUTS, "mono", 1.00);

	// card and expansion slots, not present in Tec Toy Master System III
	SMS_CARD_SLOT(config, "mycard", sms_cart, nullptr);
	SMS_EXPANSION_SLOT(config, "smsexp", sms_expansion_devices, nullptr);

	m_has_bios_full = true;
	m_has_pwr_led = true;
}


void sms_state::sms2_kr(machine_config &config)
{
	sms2_ntsc(config);

	m_maincpu->set_addrmap(AS_IO, &sms_state::smskr_io);

	config.device_remove("slot");
	SG1000MK3_CART_SLOT(config, "slot", sg1000mk3_cart, nullptr);
	SOFTWARE_LIST(config, "cart_list2").set_original("sg1000");

	// Despite having a Japanese cartridge slot, this version is detected as Export region.
	m_has_jpn_sms_cart_slot = true;
}

void sms1_state::sms1_kr(machine_config &config)
{
	sms1_ntsc(config);

	m_maincpu->set_addrmap(AS_IO, &sms1_state::smskr_io);

	// need to replace the cartridge slot with the Japanese version, so to
	// keep the usual media order, remove and reinsert all of them.
	config.device_remove("slot");
	config.device_remove("mycard");
	config.device_remove("smsexp");
	SG1000MK3_CART_SLOT(config, "slot", sg1000mk3_cart, nullptr);
	SMS_CARD_SLOT(config, "mycard", sms_cart, nullptr);
	SMS_EXPANSION_SLOT(config, "smsexp", sms_expansion_devices, nullptr);

	SOFTWARE_LIST(config, "cart_list2").set_original("sg1000");

	m_vdp->n_csync().set(FUNC(sms1_state::rapid_n_csync_callback));

	m_has_bios_full = false;
	m_has_bios_2000 = true;
	m_ioctrl_region_is_japan = true;
	m_has_jpn_sms_cart_slot = true;
}

void sms1_state::smsj(machine_config &config)
{
	sms1_kr(config);

	m_maincpu->set_addrmap(AS_IO, &sms1_state::smsj_io);

	YM2413(config, m_ym, XTAL(10'738'635)/3);
	// if this output gain is changed, the gain set when unmute the output need
	// to be changed too, probably along the gain set for the Mark III FM Unit.
	m_ym->add_route(ALL_OUTPUTS, "mono", 1.00);

	m_is_smsj = true;
}

void sg1000m3_state::sg1000m3(machine_config &config)
{
	sms1_ntsc(config);

	m_maincpu->set_addrmap(AS_IO, &sg1000m3_state::sg1000m3_io);

	// Remove and reinsert all media slots, as done with the sms1_kr config,
	// and also replace the expansion slot with the SG-1000 version.
	config.device_remove("slot");
	config.device_remove("mycard");
	config.device_remove("smsexp");
	SG1000MK3_CART_SLOT(config, "slot", sg1000mk3_cart, nullptr);
	SMS_CARD_SLOT(config, "mycard", sms_cart, nullptr);
	SG1000_EXPANSION_SLOT(config, m_sgexpslot, sg1000_expansion_devices, nullptr, false);

	SOFTWARE_LIST(config, "cart_list2").set_original("sg1000");

	// Mark III does not have TH connected.
	m_port_ctrl1->th_handler().set_nop();
	m_port_ctrl2->th_handler().set_nop();

	m_has_bios_full = false;
	m_is_mark_iii = true;
	m_has_jpn_sms_cart_slot = true;
}

void gamegear_state::gamegear(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, MASTER_CLOCK_GG/9);
	m_maincpu->set_addrmap(AS_PROGRAM, &gamegear_state::sms_mem);
	m_maincpu->set_addrmap(AS_IO, &gamegear_state::gg_io);

	config.set_maximum_quantum(attotime::from_hz(60));

	/* video hardware */
	SCREEN(config, m_main_scr, SCREEN_TYPE_LCD);
	screen_gg_raw_params(*m_main_scr, MASTER_CLOCK_GG/6);
	m_main_scr->set_screen_update(FUNC(gamegear_state::screen_update_gamegear));

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	/* VDP chip of the Gamegear 2 ASIC version */
	SEGA315_5377(config, m_vdp, MASTER_CLOCK_GG/3);
	m_vdp->set_screen(m_main_scr);
	m_vdp->set_is_pal(false);
	m_vdp->n_int().set_inputline(m_maincpu, 0);
	m_vdp->vblank().set(FUNC(gamegear_state::gg_pause_callback));
	m_vdp->add_route(0, "speaker", 1.00, 0);
	m_vdp->add_route(1, "speaker", 1.00, 1);

	/* cartridge */
	GAMEGEAR_CART_SLOT(config, "slot", gg_cart, nullptr).set_must_be_loaded(true);

	SOFTWARE_LIST(config, "cart_list").set_original("gamegear");

	GAMEGEAR_IO_PORT(config, m_gg_ioport, 0);
	m_gg_ioport->set_in_handler(m_port_gg_ext, FUNC(sms_control_port_device::in_r));
	m_gg_ioport->set_out_handler(m_port_gg_ext, FUNC(sms_control_port_device::out_w));
	m_gg_ioport->hl_handler().set(FUNC(gamegear_state::gg_nmi));

	SMS_CONTROL_PORT(config, m_port_gg_ext, sms_control_port_devices, nullptr);
	m_port_gg_ext->set_screen(m_main_scr);
	m_port_gg_ext->th_handler().set(FUNC(gamegear_state::gg_ext_th_input));

	m_is_gamegear = true;
	m_has_bios_0400 = true;
	m_has_pwr_led = true;
}

void gamegear_state::gamegeaj(machine_config &config)
{
	gamegear(config);
	m_ioctrl_region_is_japan = true;
}


ROM_START(sms1)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_FILL(0x0000, 0x4000, 0xff)

	ROM_REGION(0x20000, "user1", 0)
	ROM_SYSTEM_BIOS( 0, "bios13", "US/European BIOS v1.3 (1986)" )
	ROMX_LOAD("mpr-10052.rom", 0x0000, 0x2000, CRC(0072ed54) SHA1(c315672807d8ddb8d91443729405c766dd95cae7), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "hangonsh", "US/European BIOS v2.4 with Hang On and Safari Hunt (1988)" )
	ROMX_LOAD("mpr-11459a.rom", 0x0000, 0x20000, CRC(91e93385) SHA1(9e179392cd416af14024d8f31c981d9ee9a64517), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 2, "hangon", "US/European BIOS v3.4 with Hang On (1988)" )
	ROMX_LOAD("mpr-11458.rom", 0x0000, 0x20000, CRC(8edf7ac6) SHA1(51fd6d7990f62cd9d18c9ecfc62ed7936169107e), ROM_BIOS(2))
	ROM_SYSTEM_BIOS( 3, "missiled", "US/European BIOS v4.4 with Missile Defense 3D (1988)" )
	ROMX_LOAD("missiled.rom", 0x0000, 0x20000, CRC(e79bb689) SHA1(aa92ae576ca670b00855e278378d89e9f85e0351), ROM_BIOS(3))
	ROM_SYSTEM_BIOS( 4, "v10", "US Master System BIOS v1.0 (prototype)" )
	ROMX_LOAD("v1.0.bin", 0x0000, 0x2000, CRC(72bec693) SHA1(29091ff60ef4c22b1ee17aa21e0e75bac6b36474), ROM_BIOS(4))
	ROM_SYSTEM_BIOS( 5, "proto", "US Master System Prototype BIOS" )
	ROMX_LOAD("m404prot.rom", 0x0000, 0x2000, CRC(1a15dfcc) SHA1(4a06c8e66261611dce0305217c42138b71331701), ROM_BIOS(5))
ROM_END

ROM_START(sms)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_FILL(0x0000, 0x4000, 0xff)

	ROM_REGION(0x20000, "user1", 0)
	ROM_SYSTEM_BIOS( 0, "alexkidd", "US/European BIOS with Alex Kidd in Miracle World (1990)" )
	ROMX_LOAD("mpr-12808.ic2", 0x0000, 0x20000, CRC(cf4a09ea) SHA1(3af7b66248d34eb26da40c92bf2fa4c73a46a051), ROM_BIOS(0)) /* "SEGA // MPR-12808 W63 // 9114E9004" @ IC2 */
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
	ROMX_LOAD("mpr-10052.rom", 0x0000, 0x2000, CRC(0072ed54) SHA1(c315672807d8ddb8d91443729405c766dd95cae7), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "bios20", "European BIOS v2.0 (1987?)" ) //Chinese (PAL-D)?
	ROMX_LOAD("mpr-10883.rom", 0x0000, 0x2000, CRC(b3d854f8) SHA1(fc7eb9141f38c92bf98d9134816f64b45e811112), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 2, "hangonsh", "US/European BIOS v2.4 with Hang On and Safari Hunt (1988)" )
	ROMX_LOAD("mpr-11459a.rom", 0x0000, 0x20000, CRC(91e93385) SHA1(9e179392cd416af14024d8f31c981d9ee9a64517), ROM_BIOS(2))
	ROM_SYSTEM_BIOS( 3, "hangon", "Sega Master System - US/European BIOS v3.4 with Hang On (1988)" )
	ROMX_LOAD("mpr-11458.rom", 0x0000, 0x20000, CRC(8edf7ac6) SHA1(51fd6d7990f62cd9d18c9ecfc62ed7936169107e), ROM_BIOS(3))
	ROM_SYSTEM_BIOS( 4, "missiled", "US/European BIOS v4.4 with Missile Defense 3D (1988)" )
	ROMX_LOAD("missiled.rom", 0x0000, 0x20000, CRC(e79bb689) SHA1(aa92ae576ca670b00855e278378d89e9f85e0351), ROM_BIOS(4))
ROM_END

ROM_START(smspal)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_FILL(0x0000, 0x4000, 0xff)

	ROM_REGION(0x40000, "user1", 0)
	ROM_SYSTEM_BIOS( 0, "alexkidd", "US/European BIOS with Alex Kidd in Miracle World (1990)" ) /* PCB Label: SEGA // IC BD M4Jr. PAL" Master System II with 314-5246 (ZIP) VDP and 314-5237 (DIP48) IO */
	ROMX_LOAD("mpr-12808.ic2", 0x0000, 0x20000, CRC(cf4a09ea) SHA1(3af7b66248d34eb26da40c92bf2fa4c73a46a051), ROM_BIOS(0)) /* "SEGA // MPR-12808 W63 // 9114E9004" @ IC2 */
	ROM_SYSTEM_BIOS( 1, "sonic", "European/Brazilian BIOS with Sonic the Hedgehog (1991)" )
	ROMX_LOAD("sonbios.rom", 0x0000, 0x40000, CRC(81c3476b) SHA1(6aca0e3dffe461ba1cb11a86cd4caf5b97e1b8df), ROM_BIOS(1))
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
	ROMX_LOAD("mpr-11124.ic2", 0x0000, 0x2000, CRC(48d44a13) SHA1(a8c1b39a2e41137835eda6a5de6d46dd9fadbaf2), ROM_BIOS(0)) /* "SONY 7J06 // MPR-11124 // JAPAN 021" @ IC2 */
ROM_END

ROM_START(smskr)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_FILL(0x0000, 0x4000, 0xff)

	ROM_REGION(0x20000, "user1", 0)
	ROM_SYSTEM_BIOS( 0, "akbioskr", "Samsung Gam*Boy II with Alex Kidd in Miracle World (1990)" )
	ROMX_LOAD("akbioskr.rom", 0x000, 0x20000, CRC(9c5bad91) SHA1(2feafd8f1c40fdf1bd5668f8c5c02e5560945b17), ROM_BIOS(0))
ROM_END

ROM_START(sms1br)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_FILL(0x0000, 0x4000, 0xff)

	ROM_REGION(0x20000, "user1", 0)
	ROM_SYSTEM_BIOS( 0, "hangonsh", "US/European BIOS v2.4 with Hang On and Safari Hunt (1988)" )
	ROMX_LOAD("mpr-11459a.rom", 0x0000, 0x20000, CRC(91e93385) SHA1(9e179392cd416af14024d8f31c981d9ee9a64517), ROM_BIOS(0))
ROM_END

ROM_START(sms2br)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_FILL(0x0000, 0x4000, 0xff)

	ROM_REGION(0x40000, "user1", 0)
	ROM_SYSTEM_BIOS( 0, "alexkidd", "US/European BIOS with Alex Kidd in Miracle World (1990)" )
	ROMX_LOAD("mpr-12808.ic2", 0x0000, 0x20000, CRC(cf4a09ea) SHA1(3af7b66248d34eb26da40c92bf2fa4c73a46a051), ROM_BIOS(0))
ROM_END

ROM_START(smsbr)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_FILL(0x0000, 0x4000, 0xff)

	ROM_REGION(0x40000, "user1", 0)
	ROM_SYSTEM_BIOS( 0, "alexkidd", "US/European BIOS with Alex Kidd in Miracle World (1990)" )
	ROMX_LOAD("mpr-12808.ic2", 0x0000, 0x20000, CRC(cf4a09ea) SHA1(3af7b66248d34eb26da40c92bf2fa4c73a46a051), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "sonic", "European/Brazilian BIOS with Sonic the Hedgehog (1991)" )
	ROMX_LOAD("sonbios.rom", 0x0000, 0x40000, CRC(81c3476b) SHA1(6aca0e3dffe461ba1cb11a86cd4caf5b97e1b8df), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 2, "hangonsh", "US/European BIOS v2.4 with Hang On and Safari Hunt (1988)" )
	ROMX_LOAD("mpr-11459a.rom", 0x0000, 0x20000, CRC(91e93385) SHA1(9e179392cd416af14024d8f31c981d9ee9a64517), ROM_BIOS(2))
ROM_END

ROM_START(sms2paln)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_FILL(0x0000, 0x4000, 0xff)

	ROM_REGION(0x40000, "user1", 0)
	ROM_SYSTEM_BIOS( 0, "alexkidd", "US/European BIOS with Alex Kidd in Miracle World (1990)" )
	ROMX_LOAD("mpr-12808.ic2", 0x0000, 0x20000, CRC(cf4a09ea) SHA1(3af7b66248d34eb26da40c92bf2fa4c73a46a051), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "missiled", "US/European BIOS v4.4 with Missile Defense 3D (1988)" )
	ROMX_LOAD("missiled.rom", 0x0000, 0x20000, CRC(e79bb689) SHA1(aa92ae576ca670b00855e278378d89e9f85e0351), ROM_BIOS(1))
ROM_END

ROM_START(smspaln)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_FILL(0x0000, 0x4000, 0xff)

	ROM_REGION(0x40000, "user1", 0)
	ROM_SYSTEM_BIOS( 0, "alexkidd", "US/European BIOS with Alex Kidd in Miracle World (1990)" )
	ROMX_LOAD("mpr-12808.ic2", 0x0000, 0x20000, CRC(cf4a09ea) SHA1(3af7b66248d34eb26da40c92bf2fa4c73a46a051), ROM_BIOS(0))
ROM_END

ROM_START(gamegear)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_FILL(0x0000, 0x4000, 0x00)

	ROM_REGION(0x0400, "user1", 0)
	ROM_SYSTEM_BIOS( 0, "none", "No BIOS" )
	ROM_SYSTEM_BIOS( 1, "majesco", "Majesco BIOS" )
	ROMX_LOAD("majbios.rom", 0x0000, 0x0400, CRC(0ebea9d4) SHA1(914aa165e3d879f060be77870d345b60cfeb4ede), ROM_BIOS(1))
ROM_END

#define rom_gamegeaj rom_gamegear
#define rom_sms1krfm rom_smsj
#define rom_sms1kr rom_smsj
#define rom_sms1paln rom_sms1br

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
   - Tec Toy Master System (I) (sms1br)
     - built-in Hang On/Safari Hunt v2.4 - 1989
   - Tec Toy Master System II (sms2br)
     - built-in Alex Kidd in Miracle World - 1991
   - Tec Toy Master System III Compact (smsbr)
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
   - PAL-M has the same line count and near the same frequency of NTSC
   - Tec Toy later changed its logo twice and its name to Tectoy
   - 20XX models (Handy, Collection, Evolution...) likely have SoC hardware

  PAL-N (Argentina, Paraguay, Uruguay)
   - Tec Toy Master System (I) (sms1paln)
     - built-in Hang On/Safari Hunt v2.4
   - Tec Toy Master System II (sms2paln)
     - built-in Alex Kidd in Miracle World
     - built-in Missile Defense 3-D v4.4
   - Tec Toy Master System III Compact (smspaln)
     - built-in Alex Kidd in Miracle World
  Notes:
   - Distributed by: Gameland (Argentina), Forstar (Uruguay)

  These are coin-operated machines (stuff for MAME):

   - Sega Game Box 9
   - Sega Mark III Soft Desk 5
   - Sega Mark III Soft Desk 10
   - Sega Shooting Zone

   The SMS Store Display Unit is labeled PD-W UNIT. Pictures found on Internet
   show cartridges with a label where a not-for-sale message is written along
   the information that it is for use on the Product Display-Working Unit.

***************************************************************************/

/*    YEAR  NAME      PARENT    COMPAT  MACHINE    INPUT     CLASS           INIT           COMPANY    FULLNAME                              FLAGS */
CONS( 1985, sg1000m3, sms,      0,      sg1000m3,  sg1000m3, sg1000m3_state, empty_init,    "Sega",    "Mark III",                           MACHINE_SUPPORTS_SAVE )
CONS( 1986, sms1,     sms,      0,      sms1_ntsc, sms1,     sms1_state,     empty_init,    "Sega",    "Master System",                      MACHINE_SUPPORTS_SAVE )
CONS( 1986, sms1pal,  sms,      0,      sms1_pal,  sms1,     sms1_state,     empty_init,    "Sega",    "Master System (PAL)" ,               MACHINE_SUPPORTS_SAVE )
CONS( 1986, smssdisp, sms,      0,      sms_sdisp, smssdisp, smssdisp_state, empty_init,    "Sega",    "Master System Store Display Unit",   MACHINE_SUPPORTS_SAVE )
CONS( 1987, smsj,     sms,      0,      smsj,      smsj,     sms1_state,     empty_init,    "Sega",    "Master System (Japan)",              MACHINE_SUPPORTS_SAVE )
CONS( 1990, sms,      0,        0,      sms2_ntsc, sms,      sms_state,      empty_init,    "Sega",    "Master System II",                   MACHINE_SUPPORTS_SAVE )
CONS( 1990, smspal,   sms,      0,      sms2_pal,  sms,      sms_state,      empty_init,    "Sega",    "Master System II (PAL)",             MACHINE_SUPPORTS_SAVE )
CONS( 1989, sms1krfm, sms,      0,      smsj,      smsj,     sms1_state,     empty_init,    "Samsung", "Gam*Boy (Korea) (FM)",               MACHINE_SUPPORTS_SAVE )
CONS( 19??, sms1kr,   sms,      0,      sms1_kr,   smsj,     sms1_state,     empty_init,    "Samsung", "Gam*Boy (Korea)",                    MACHINE_SUPPORTS_SAVE )
CONS( 1991, smskr,    sms,      0,      sms2_kr,   sms,      sms_state,      empty_init,    "Samsung", "Gam*Boy II (Korea)",                 MACHINE_SUPPORTS_SAVE )
CONS( 1989, sms1br,   sms,      0,      sms1_br,   sms1,     sms1_state,     empty_init,    "Tec Toy", "Master System (Brazil)",             MACHINE_SUPPORTS_SAVE )
CONS( 1991, sms2br,   sms,      0,      sms1_br,   sms1,     sms1_state,     empty_init,    "Tec Toy", "Master System II (Brazil)",          MACHINE_SUPPORTS_SAVE )
CONS( 1992, smsbr,    sms,      0,      sms3_br,   sms,      sms_state,      empty_init,    "Tec Toy", "Master System III Compact (Brazil)", MACHINE_SUPPORTS_SAVE )
CONS( 19??, sms1paln, sms,      0,      sms1_paln, sms1,     sms1_state,     empty_init,    "Tec Toy", "Master System (PAL-N)",              MACHINE_SUPPORTS_SAVE )
CONS( 19??, sms2paln, sms,      0,      sms1_paln, sms1,     sms1_state,     empty_init,    "Tec Toy", "Master System II (PAL-N)",           MACHINE_SUPPORTS_SAVE )
CONS( 19??, smspaln,  sms,      0,      sms3_paln, sms,      sms_state,      empty_init,    "Tec Toy", "Master System III Compact (PAL-N)",  MACHINE_SUPPORTS_SAVE )

CONS( 1991, gamegear, 0,        sms,    gamegear,  gg,       gamegear_state, empty_init,    "Sega",    "Game Gear (Europe/America)",         MACHINE_SUPPORTS_SAVE )
CONS( 1990, gamegeaj, gamegear, 0,      gamegeaj,  gg,       gamegear_state, empty_init,    "Sega",    "Game Gear (Japan)",                  MACHINE_SUPPORTS_SAVE )
