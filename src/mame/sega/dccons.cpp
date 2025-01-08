// license:LGPL-2.1+
// copyright-holders:Angelo Salese, R. Belmont
/*
    dccons.cpp - Sega Dreamcast driver
    by R. Belmont & Angelo Salese

    SH-4 @ 200 MHz
    ARM7DI @ 2.8223 MHz (no T or M extensions)
    PowerVR 3D video
    AICA audio
    GD-ROM drive (modified ATAPI interface)

            NTSC/N  NTSC/I   PAL/N   PAL/I   VGA
        (x/240) (x/480) (x/240)  (x/480) (640x480)
    VTOTAL   262     524      312     624    524
    HTOTAL   857     857      863     863    857

    PCLKs = 26917135 (NTSC 480 @ 59.94), 26944080 (VGA 480 @ 60.0), 13458568 (NTSC 240 @ 59.94),
            25925600 (PAL 480 @ 50.00), 13462800 (PAL 240 @ 50.00)

    TODO:
    - cfr. naomi.cpp header for general DC notes;
    - https://github.com/flyinghead/flycast/blob/master/docs/Notable%20game%20bugs.md
      For a comprehensive list of issues to be verified;
    - Fix Check-GD disk usability, cfr. notes in dc.xml;
    - G1 i/f for GD-ROM needs to be converted in device class and hunted for unsupported features;
    - VMU;
    - Modem;
    - regtest dc.xml against region setting, find the bad chance that a SW item
      managed to brick a dcjp and turning into PAL. Notable symptoms:
      - crzytaxi2j will start with 50Hz/60Hz screen selector;
      - jojobaj will run in PAL mode;

    Notes:
    - RTC error pops up at start-up. (btanb)
      System saves to flash the AICA RTC timestamp and prompt user to change the clock in
      case that drifts out significantly during boot.
      Solution is to insert the actual host PC timestamp and have MAME nvram_save on,
      that will lift the RTC prompt at next boot.

    Old TODO (to be rechecked and moved in XML notes):
    - Inputs doesn't work most of the time;
    - Candy Stripe: fills the log with "ATAPI_FEATURES_FLAG_OVL not supported", black screen
    - Close To: Hangs at FMV
    - Power Stone: hangs at Capcom logo;
    - Sega GT: no cursor on main menu;
    - Tetris 4D: hangs at BPS FMV (bp 0C0B0C4E)

    Notes:
    - 0x1a002 of flash ROM returns the region type (0x30=Japan, 0x31=USA, 0x32=Europe). Amusingly, if the value
      on a non-jp console is different than these ones, the system shows a black swirl (and nothing boots).

*/

#include "emu.h"
#include "dccons.h"

#include "dc-ctrl.h"

#include "bus/ata/gdrom.h"
#include "cpu/arm7/arm7.h"
#include "cpu/sh/sh4.h"
#include "imagedev/cdromimg.h"
#include "machine/aicartc.h"

//#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "softlist.h"

#define CPU_CLOCK (200000000)
// cfr. sh4.cpp m_mmuhack
#define DC_MMU_HACK_MODE (1)

void dc_cons_state::init_dc()
{
	m_maincpu->sh2drc_set_options(SH2DRC_STRICT_VERIFY | SH2DRC_STRICT_PCREL);
	m_maincpu->sh2drc_add_fastram(0x00000000, 0x001fffff, true, memregion("maincpu")->base());
	m_maincpu->sh2drc_add_fastram(0x0c000000, 0x0cffffff, false, dc_ram);
	dreamcast_atapi_init();
}

void dc_cons_state::init_tream()
{
	// Modchip connected to BIOS ROM chip changes 4 bytes (actually bits) as shown below, which allow to boot any region games.
	u8 *rom = (u8 *)memregion("maincpu")->base();
	rom[0x503] |= 0x40;
	rom[0x50f] |= 0x40;
	rom[0x523] |= 0x40;
	rom[0x531] |= 0x40;

	init_dc();
}

uint64_t dc_cons_state::dc_pdtra_r()
{
	uint64_t out = PCTRA<<32;

	out |= PDTRA & ~0x0303;

	// if both bits are inputs
	if (!(PCTRA & 0x5))
	{
		out |= 0x03;
	}

	// one's input one's output, always pull up both bits
	if (((PCTRA & 5) == 1) || ((PCTRA & 5) == 4))
	{
		if (PDTRA & 3)
		{
			out |= 0x03;
		}
	}


	// cable setting, (0) VGA, (2) TV RGB (3) TV VBS/Y + S/C.
	// Note: several games doesn't like VGA setting,
	// default to composite for max possible compatibility
	// (i.e. Idol Janshi wo Tsukucchaou, Airforce Delta)
	// TODO: identify via script
	out |= ioport("SCREEN_TYPE")->read() << 8;

	return out;
}

void dc_cons_state::dc_pdtra_w(uint64_t data)
{
	PCTRA = (data>>16) & 0xffff;
	PDTRA = (data & 0xffff);
}

uint8_t dc_cons_state::dc_flash_r(offs_t offset)
{
	return m_dcflash->read(offset+0x20000);
}

void dc_cons_state::dc_flash_w(offs_t offset, uint8_t data)
{
	m_dcflash->write(offset+0x20000,data);
}

void dc_cons_state::dc_map(address_map &map)
{
	map(0x00000000, 0x001fffff).rom().nopw();             // BIOS
	map(0x00200000, 0x0021ffff).rw(FUNC(dc_cons_state::dc_flash_r), FUNC(dc_cons_state::dc_flash_w));
	map(0x005f6800, 0x005f69ff).rw(FUNC(dc_cons_state::dc_sysctrl_r), FUNC(dc_cons_state::dc_sysctrl_w));
	map(0x005f6c00, 0x005f6cff).m(m_maple, FUNC(maple_dc_device::amap));
	map(0x005f7000, 0x005f701f).rw(m_ata, FUNC(ata_interface_device::cs1_r), FUNC(ata_interface_device::cs1_w)).umask64(0x0000ffff0000ffff);
	map(0x005f7080, 0x005f709f).rw(m_ata, FUNC(ata_interface_device::cs0_r), FUNC(ata_interface_device::cs0_w)).umask64(0x0000ffff0000ffff);
	map(0x005f7400, 0x005f74ff).rw(FUNC(dc_cons_state::dc_mess_g1_ctrl_r), FUNC(dc_cons_state::dc_mess_g1_ctrl_w));
	map(0x005f7800, 0x005f78ff).m(m_g2if, FUNC(dc_g2if_device::amap));
	map(0x005f7c00, 0x005f7cff).m(m_powervr2, FUNC(powervr2_device::pd_dma_map));
	map(0x005f8000, 0x005f9fff).m(m_powervr2, FUNC(powervr2_device::ta_map));
	map(0x00600000, 0x006007ff).rw(FUNC(dc_cons_state::dc_modem_r), FUNC(dc_cons_state::dc_modem_w));
	map(0x00700000, 0x00707fff).rw(FUNC(dc_cons_state::dc_aica_reg_r), FUNC(dc_cons_state::dc_aica_reg_w));
	map(0x00710000, 0x0071000f).mirror(0x02000000).rw("aicartc", FUNC(aicartc_device::read), FUNC(aicartc_device::write)).umask64(0x0000ffff0000ffff);
	map(0x00800000, 0x009fffff).mirror(0x02000000).rw(FUNC(dc_cons_state::soundram_r), FUNC(dc_cons_state::soundram_w));
//  map(0x01000000, 0x01ffffff) G2 Ext Device #1
//  map(0x02700000, 0x02707fff) AICA reg mirror
//  map(0x02800000, 0x02ffffff) AICA wave mem mirror (loopchk g2 bus DMA test)

//  map(0x03000000, 0x03ffffff) G2 Ext Device #2

	/* Area 1 */
	map(0x04000000, 0x04ffffff).ram().share("dc_texture_ram");      // texture memory 64 bit access
	map(0x05000000, 0x05ffffff).ram().share("frameram"); // apparently this actually accesses the same memory as the 64-bit texture memory access, but in a different format, keep it apart for now

	/* Area 3 */
	map(0x0c000000, 0x0cffffff).ram().share("dc_ram");
	map(0x0d000000, 0x0dffffff).ram().share("dc_ram");// extra ram on Naomi (mirror on DC)
	map(0x0e000000, 0x0effffff).ram().share("dc_ram");// mirror
	map(0x0f000000, 0x0fffffff).ram().share("dc_ram");// mirror

	/* Area 4 */
	map(0x10000000, 0x107fffff).w(m_powervr2, FUNC(powervr2_device::ta_fifo_poly_w));
	map(0x10800000, 0x10ffffff).w(m_powervr2, FUNC(powervr2_device::ta_fifo_yuv_w));
	map(0x11000000, 0x117fffff).w(m_powervr2, FUNC(powervr2_device::ta_texture_directpath0_w)).mirror(0x00800000);  // access to texture / framebuffer memory (either 32-bit or 64-bit area depending on SB_LMMODE0 register - cannot be written directly, only through dma / store queue

	map(0x12000000, 0x127fffff).w(m_powervr2, FUNC(powervr2_device::ta_fifo_poly_w));
	map(0x12800000, 0x12ffffff).w(m_powervr2, FUNC(powervr2_device::ta_fifo_yuv_w));
	map(0x13000000, 0x137fffff).w(m_powervr2, FUNC(powervr2_device::ta_texture_directpath1_w)).mirror(0x00800000); // access to texture / framebuffer memory (either 32-bit or 64-bit area depending on SB_LMMODE1 register - cannot be written directly, only through dma / store queue

//  map(0x14000000, 0x17ffffff) G2 Ext Device #3

	map(0x8c000000, 0x8cffffff).ram().share("dc_ram");  // another RAM mirror

	map(0xa0000000, 0xa01fffff).rom().region("maincpu", 0);

	map(0xf4000000, 0xf4003fff).noprw(); // SH-4 operand cache address array
}

void dc_cons_state::dc_port(address_map &map)
{
	map(0x00000000, 0x00000007).rw(FUNC(dc_cons_state::dc_pdtra_r), FUNC(dc_cons_state::dc_pdtra_w));
}

void dc_cons_state::dc_audio_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000000, 0x001fffff).rw(FUNC(dc_cons_state::soundram_r), FUNC(dc_cons_state::soundram_w));        /* shared with SH-4 */
	map(0x00800000, 0x00807fff).rw(FUNC(dc_cons_state::dc_arm_aica_r), FUNC(dc_cons_state::dc_arm_aica_w));
}

void dc_cons_state::aica_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x1fffff).ram().share("dc_sound_ram");
}

static INPUT_PORTS_START( dc )
	PORT_START("P1:0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1) PORT_NAME("P1 RIGHT")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1) PORT_NAME("P1 LEFT")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1) PORT_NAME("P1 DOWN")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1) PORT_NAME("P1 UP")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("P1 START")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 A")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED ) //PORT_NAME("P1 C")

	PORT_START("P1:1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) // 2nd directional pad
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED ) //PORT_NAME("P1 D")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 X")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("P1 Y")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED ) //PORT_NAME("P1 Z")

	PORT_START("P1:A0")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_CENTERDELTA(10) PORT_NAME("P1 R") PORT_PLAYER(1)

	PORT_START("P1:A1")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_CENTERDELTA(10) PORT_NAME("P1 L") PORT_PLAYER(1)

	PORT_START("P1:A2")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_CENTERDELTA(10) PORT_PLAYER(1)

	PORT_START("P1:A3")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_CENTERDELTA(10) PORT_PLAYER(1)

	//A4 - A5, second analog stick, unused on DC

	PORT_START("P2:0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2) PORT_NAME("P2 RIGHT")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2) PORT_NAME("P2 LEFT")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2) PORT_NAME("P2 DOWN")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2) PORT_NAME("P2 UP")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME("P2 START")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 A")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED ) //PORT_NAME("P2 C")

	PORT_START("P2:1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) // 2nd directional pad
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED ) //PORT_NAME("P1 D")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 X")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("P2 Y")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED ) //PORT_NAME("P1 Z")

	PORT_START("P2:A0")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_CENTERDELTA(10) PORT_NAME("P2 R") PORT_PLAYER(2)

	PORT_START("P2:A1")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_CENTERDELTA(10) PORT_NAME("P2 L") PORT_PLAYER(2)

	PORT_START("P2:A2")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_CENTERDELTA(10) PORT_PLAYER(2)

	PORT_START("P2:A3")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_CENTERDELTA(10) PORT_PLAYER(2)

	//A4 - A5, second analog stick, unused on DC

	PORT_START("P3:0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3) PORT_NAME("P3 RIGHT")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3) PORT_NAME("P3 LEFT")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3) PORT_NAME("P3 DOWN")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3) PORT_NAME("P3 UP")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START3 ) PORT_NAME("P3 START")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3) PORT_NAME("P3 A")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3) PORT_NAME("P3 B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED ) //PORT_NAME("P3 C")

	PORT_START("P3:1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) // 2nd directional pad
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED ) //PORT_NAME("P3 D")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3) PORT_NAME("P3 X")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(3) PORT_NAME("P3 Y")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED ) //PORT_NAME("P3 Z")

	PORT_START("P3:A0")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_CENTERDELTA(10) PORT_NAME("P3 R") PORT_PLAYER(3)

	PORT_START("P3:A1")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_CENTERDELTA(10) PORT_NAME("P3 L") PORT_PLAYER(3)

	PORT_START("P3:A2")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_CENTERDELTA(10) PORT_PLAYER(3)

	PORT_START("P3:A3")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_CENTERDELTA(10) PORT_PLAYER(3)

	//A4 - A5, second analog stick, unused on DC

	PORT_START("P4:0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4) PORT_NAME("P4 RIGHT")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4) PORT_NAME("P4 LEFT")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4) PORT_NAME("P4 DOWN")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4) PORT_NAME("P4 UP")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START4 ) PORT_NAME("P4 START")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4) PORT_NAME("P4 A")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4) PORT_NAME("P4 B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED ) //PORT_NAME("P4 C")

	PORT_START("P4:1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) // 2nd directional pad
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED ) //PORT_NAME("P3 D")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4) PORT_NAME("P4 X")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(4) PORT_NAME("P4 Y")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED ) //PORT_NAME("P3 Z")

	PORT_START("P4:A0")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_CENTERDELTA(10) PORT_NAME("P4 R") PORT_PLAYER(4)

	PORT_START("P4:A1")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_CENTERDELTA(10) PORT_NAME("P4 L") PORT_PLAYER(4)

	PORT_START("P4:A2")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_CENTERDELTA(10) PORT_PLAYER(4)

	PORT_START("P4:A3")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_CENTERDELTA(10) PORT_PLAYER(4)

	//A4 - A5, second analog stick, unused on DC

	PORT_START("SCREEN_TYPE")
	PORT_CONFNAME( 0x03, 0x03, "Screen Connection Type" )
	PORT_CONFSETTING(    0x00, "VGA" )
	PORT_CONFSETTING(    0x02, "Composite" )
	PORT_CONFSETTING(    0x03, "S-Video" )
INPUT_PORTS_END

static INPUT_PORTS_START( dcfish )
	PORT_START("P1:0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("CONFIG")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("A")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SCREEN_TYPE")
	PORT_CONFNAME( 0x03, 0x03, "Screen Connection Type" )
	PORT_CONFSETTING(    0x00, "VGA" )
	PORT_CONFSETTING(    0x02, "Composite" )
	PORT_CONFSETTING(    0x03, "S-Video" )
INPUT_PORTS_END

void dc_cons_state::gdrom_config(device_t *device)
{
	cdda_device *cdda = device->subdevice<cdda_device>("cdda");
	cdda->audio_end_cb().set(*device, FUNC(gdrom_device::cdda_end_mark_cb));
	cdda->add_route(0, "^^aica", 1.0);
	cdda->add_route(1, "^^aica", 1.0);
}

void dc_cons_state::dc_base(machine_config &config)
{
	/* basic machine hardware */
	SH4LE(config, m_maincpu, CPU_CLOCK);
	m_maincpu->set_md(0, 1);
	m_maincpu->set_md(1, 0);
	m_maincpu->set_md(2, 1);
	m_maincpu->set_md(3, 0);
	m_maincpu->set_md(4, 0);
	m_maincpu->set_md(5, 1);
	m_maincpu->set_md(6, 0);
	m_maincpu->set_md(7, 1);
	m_maincpu->set_md(8, 0);
	m_maincpu->set_sh4_clock(CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &dc_cons_state::dc_map);
	m_maincpu->set_addrmap(AS_IO, &dc_cons_state::dc_port);
	m_maincpu->set_mmu_hacktype(DC_MMU_HACK_MODE);

	TIMER(config, "scantimer").configure_scanline(FUNC(dc_state::dc_scanline), "screen", 0, 1);

	system_bus_config(config, "maincpu");

	ARM7(config, m_soundcpu, ((XTAL(33'868'800)*2)/3)/8);   // AICA bus clock is 2/3rds * 33.8688.  ARM7 gets 1 bus cycle out of each 8.
	m_soundcpu->set_addrmap(AS_PROGRAM, &dc_cons_state::dc_audio_map);

	MCFG_MACHINE_RESET_OVERRIDE(dc_cons_state,dc_console )

	FUJITSU_29LV002TC(config, "dcflash");

	MAPLE_DC(config, m_maple, 0, m_maincpu);
	m_maple->irq_callback().set(FUNC(dc_state::maple_irq));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	// TODO: find exact pclk source
	screen.set_raw(13458568*2, 857, 0, 640, 524, 0, 480);
	screen.set_screen_update("powervr2", FUNC(powervr2_device::screen_update));

	POWERVR2(config, m_powervr2, 0);
	m_powervr2->irq_callback().set(FUNC(dc_state::pvr_irq));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	AICA(config, m_aica, (XTAL(33'868'800)*2)/3); // 67.7376MHz(2*33.8688MHz), div 3 for audio block
	m_aica->irq().set(FUNC(dc_state::aica_irq));
	m_aica->main_irq().set(FUNC(dc_state::sh4_aica_irq));
	m_aica->set_addrmap(0, &dc_cons_state::aica_map);
	m_aica->add_route(0, "lspeaker", 0.4);
	m_aica->add_route(1, "rspeaker", 0.4);

	AICARTC(config, "aicartc", XTAL(32'768));

	ATA_INTERFACE(config, m_ata, 0);
	m_ata->irq_handler().set(FUNC(dc_cons_state::ata_interrupt));

	ata_slot_device &ata_0(*subdevice<ata_slot_device>("ata:0"));
	ata_0.option_add("gdrom", ATAPI_GDROM);
	ata_0.set_option_machine_config("gdrom", gdrom_config);
	ata_0.set_default_option("gdrom");
}

void dc_cons_state::dc(machine_config &config)
{
	dc_base(config);

	dc_controller_device &dcctrl0(DC_CONTROLLER(config, "dcctrl0", 0, m_maple, 0));
	dcctrl0.set_port_tags("P1:0", "P1:1", "P1:A0", "P1:A1", "P1:A2", "P1:A3", "P1:A4", "P1:A5");
	dc_controller_device &dcctrl1(DC_CONTROLLER(config, "dcctrl1", 0, m_maple, 1));
	dcctrl1.set_port_tags("P2:0", "P2:1", "P2:A0", "P2:A1", "P2:A2", "P2:A3", "P2:A4", "P2:A5");
	dc_controller_device &dcctrl2(DC_CONTROLLER(config, "dcctrl2", 0, m_maple, 2));
	dcctrl2.set_port_tags("P3:0", "P3:1", "P3:A0", "P3:A1", "P3:A2", "P3:A3", "P3:A4", "P3:A5");
	dc_controller_device &dcctrl3(DC_CONTROLLER(config, "dcctrl3", 0, m_maple, 3));
	dcctrl3.set_port_tags("P4:0", "P4:1", "P4:A0", "P4:A1", "P4:A2", "P4:A3", "P4:A4", "P4:A5");

	SOFTWARE_LIST(config, "cd_list").set_original("dc");
}

void dc_cons_state::dc_fish(machine_config &config)
{
	dc_base(config);

	dc_controller_device &dcctrl0(DC_CONTROLLER(config, "dcctrl0", 0, m_maple, 0));
	dcctrl0.set_port_tag<0>("P1:0");
}

#define ROM_LOAD_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_BIOS(bios))

// known undumped or private BIOS revisions:
// "MPR-21068 SEGA JAPAN / 9850 D" from VA0 837-13392-02 (171-7782B) NTSC-J unit
// KABUTO Ver.1.011 CRC 34DA5C88 from pre-release US unit (private)

// MPR-21933 (5v/VA0) confirmed match MPR-21931 (3.3v/VA1) - v1.01d
// actual mask rom labels may have -X1 or -X2 added depending on chip manufacturer, contents is the same

#define DREAMCAST_COMMON_BIOS \
	ROM_REGION(0x200000, "maincpu", 0) \
	ROM_SYSTEM_BIOS(0, "101d", "v1.01d (World)") \
	ROM_LOAD_BIOS(0, "mpr-21931.ic501", 0x000000, 0x200000, CRC(89f2b1a1) SHA1(8951d1bb219ab2ff8583033d2119c899cc81f18c) ) \
	ROM_SYSTEM_BIOS(1, "1022", "v1.022 (World)") \
	ROM_LOAD_BIOS(1, "mpr-23588.ic501", 0x000000, 0x200000, CRC(786168f9) SHA1(ba8bbb90fdb29525f24f17055dc2c7b2d7674437) ) \
	ROM_SYSTEM_BIOS(2, "101c", "v1.01c (World)") \
	ROM_LOAD_BIOS(2, "mpr-21871.ic501", 0x000000, 0x200000, CRC(2f551bc5) SHA1(1ede8d5be49116a4c6f3fe0961175469537a0434) ) \
	ROM_SYSTEM_BIOS(3, "101dch", "v1.01d (Chinese hack)") \
	ROM_LOAD_BIOS(3, "dc101d_ch.bin",   0x000000, 0x200000, CRC(a2564fad) SHA1(edc5d3d70a93c935703d26119b37731fd317d2bf) )
// ^^^ dc101d_ch.bin ^^^ is selfmade Chinese translation, doesn't work on real hardware, does it must be here at all ?

/* note: Dreamcast Flash ROMs actually 256KB MBM29F002TC (5v/VA0) or MBM29LV002TC (3.3v/VA1) devices, only 2nd 128KB half is used, A17 pin tied to VCC
   sector SA5 (1A000 - 1BFFF) is read-only, contain information written during manufacture or repair, fully generated by software tool (except predefined list of creators)
struct factory_sector
{
    struct factory_record {
        // everything 'char' below is decimal numbers in ASCII, unless noted else
        char machine_code1; // '0' - Dreamcast, 0xFF - dev.box
        char machine_code2; // '0' - Dreamcast, 0xFF - dev.box
        char country_code;  // 0 - Japan, 1 - America, 2 - Europe
        char language;      // 0 - Japanese, 1 - English, etc
        char broadcast_format;  // 0 - NTSC, 1 - PAL, 2 - PAL-M, 3 - PAL-N
        char machine_name[32];  // ASCII text 'Dreamcast', trail is 0x20 filled
        char tool_number[4];    // software tool #
        char tool_version[2];   // software tool version
        char tool_type[2];  // software tool type: 0 - for MP(mass production?), 1 - for Repair, 2 - for PP
        char year[4];
        char month[2];
        char day[2];
        char hour[2];
        char min[2];
        char serial_number[8];
        char factory_code[4];
        char total_number[16];
        uint8_t sum;        // byte sum of above
        struct {
            uint8_t sum_inv;    // ~(UID byte sum)
            uint8_t sum;        // UID byte sum
            uint8_t id[6];      // UID
        } machine_id;
        uint8_t machine_type;   // FF - Dreamcast
        uint8_t machine_version;// FF - VA0, FE - VA1, FD - VA2, NOTE: present in 1st factory record only, in 2nd always FF
        uint8_t unused[0x40]    // FF filled
    } factory_records[2];       // 2 copies
    uint8_t unused_0[0x36];     // FF filled
    uint8_t unk_version;        // not clear if hardware or bios version, A0 - VA0, 9F - VA1, 9E - VA2
    uint8_t unused_1[9];        // FF filled
    char staff_roll[0xca0];     // list of creators
    uint8_t unused_2[0x420];    // FF filled
    uint8_t random[0xdc0];      // output of RNG {static u32 seed; seed=(seed*0x83d+0x2439)&0x7fff; return (u16)(seed+0xc000);}, where initial seed value is serial_number[7] & 0xf
};

Besides factory sector, each new Dreamcast have "Flash Partition 2" header in SA6 (@1C000) followed by "CID" record:
struct cid_record
{
    uint16_t record_type;           // 0, can be 0-4
    struct cid_data
    {
        uint8_t date[4];            // BCD YYYY/MM/DD
        char t_inferior_code[4];    // '0'-filled in all dumps we have
        char repair_voucher_no[8];  // '0'-filled in all dumps we have
        uint8_t serial_no[8];
        uint8_t factory_code;
        uint8_t order_no[5];
    } cid[2];
    uint16_t crc16;
};
*/

ROM_START(dc)
	DREAMCAST_COMMON_BIOS

	ROM_REGION64_LE(0x040000, "dcflash", ROMREGION_ERASEFF)
	ROM_LOAD( "dcus_ntsc.bin", 0x020000, 0x020000, CRC(4136c25b) SHA1(1efa00ab9d8357a9f91e5be931a3efd6236f2b79) )  // dumped from VA2.4 mobo with 1.022 BIOS
ROM_END

ROM_START( dceu )
	DREAMCAST_COMMON_BIOS

	ROM_REGION64_LE(0x040000, "dcflash", ROMREGION_ERASEFF)
	ROM_LOAD( "dceu_pal.bin",  0x020000, 0x020000, CRC(7a102d05) SHA1(13e444e613dffe0a8bce073a01efa9a1d4626ba7) ) // VA1
	ROM_LOAD( "dceu_pala.bin", 0x020000, 0x020000, CRC(2e8dfa07) SHA1(ca5fd977bbf8f48c28c1027a023b038123d57d39) ) // from VA1 with 1.01d BIOS
ROM_END

ROM_START( dcjp )
	DREAMCAST_COMMON_BIOS
	ROM_SYSTEM_BIOS(4, "1004", "v1.004 (Japan)")    // oldest known mass production version, supports Japan region only
	ROM_LOAD_BIOS(4, "mpr-21068.ic501", 0x000000, 0x200000, CRC(5454841f) SHA1(1ea132c0fbbf07ef76789eadc07908045c089bd6) )

	ROM_REGION64_LE(0x040000, "dcflash", ROMREGION_ERASEFF)
	ROM_LOAD( "dcjp_ntsc.bin", 0x020000, 0x020000, CRC(306023ab) SHA1(5fb66adb6d1b54a552fe9c2bb736e4c6960e447d) ) // from refurbished VA0 with 1.004 BIOS
ROM_END

// unauthorised portable modification
ROM_START( dctream )
	ROM_REGION(0x200000, "maincpu", 0)
	// uses regular mpr-21931 BIOS chip, have region-free mod-chip installed, see driver init.
	ROM_LOAD( "mpr-21931.ic501", 0x000000, 0x200000, CRC(89f2b1a1) SHA1(8951d1bb219ab2ff8583033d2119c899cc81f18c) )

	ROM_REGION64_LE(0x040000, "dcflash", ROMREGION_ERASEFF)
	ROM_LOAD( "dc_flash.bin", 0x020000, 0x020000, CRC(9d5515c4) SHA1(78a86fd4e8b58fc9d3535eef6591178f1b97ecf9) ) // VA1 NTSC-US
ROM_END

// normally, with DIP switch 4 off, HKT-0100/0110/0120 AKA "Katana Set 5.xx", will be booted from flash ROM IC507 (first 2 dumps below)
// otherwise it boots from EPROM which contain system checker software (last 2 dumps)
ROM_START( dcdev )
	ROM_REGION(0x200000, "maincpu", 0)
	ROM_SYSTEM_BIOS(0, "1011", "Katana Set5 v1.011 (World)")    // BOOT flash rom update from Katana SDK R9-R11, WinCE SDK v2.1
	ROM_LOAD_BIOS(0, "set5v1.011.ic507", 0x000000, 0x200000, CRC(2186e0e5) SHA1(6bd18fb83f8fdb56f1941e079580e5dd672a6dad) )
	ROM_SYSTEM_BIOS(1, "1001", "Katana Set5 v1.001 (Japan)")    // BOOT flash rom update from Katana SDK 1.42J and WinCE SDK v1.0
	ROM_LOAD_BIOS(1, "set5v1.001.ic507", 0x000000, 0x200000, CRC(5702d38f) SHA1(ea7a3ae1de73683008dd795c252941a4fc81b42e) )
	ROM_SYSTEM_BIOS(2, "0976", "Katana Set5 v0.976 (Japan)")    // BOOT flash rom update from Katana SDK 1.20J
	ROM_LOAD_BIOS(2, "set5v0.976.ic507", 0x000000, 0x200000, CRC(dcb2e86f) SHA1(c88b4b6704811e3a428ee225727e4f7df467a3b5) )
	ROM_SYSTEM_BIOS(3, "0972", "Katana Set5 v0.972 (Japan)")    // BOOT flash rom update from Katana SDK 1.00b2
	ROM_LOAD_BIOS(3, "set5v0.972.ic507", 0x000000, 0x200000, CRC(1a2f2a91) SHA1(08df891f02cf959189bc9b7c4ac1a4e6a4475b50) )

	// 27C160 EPROM (DIP42) IC??? labeled
	// SET5 7676
	// V0.71 98/11/13
	ROM_SYSTEM_BIOS(4, "071", "Katana Set5 Checker v0.71")
	ROM_LOAD_BIOS(4, "set5v0.71.bin", 0x000000, 0x200000, CRC(52d01969) SHA1(28aec4a01419d2d2a664c540bef30ea289ca0644) )
	// SET5 FC52
	// V0.41 98/08/27
	ROM_SYSTEM_BIOS(5, "041", "Katana Set5 Checker v0.41")
	ROM_LOAD_BIOS(5, "set5v0.41.bin", 0x000000, 0x200000, CRC(485877bd) SHA1(dc1af1f1248ffa87d57bc5ef2ea41aac95ecfc5e) )

	ROM_REGION64_LE(0x040000, "dcflash", ROMREGION_ERASEFF)
	// Dev.Boxes have empty (FF filled) flash ROM
ROM_END

/*    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT  CLASS          INIT       COMPANY FULLNAME */
CONS( 1999, dc,      dcjp,   0,      dc,      dc,    dc_cons_state, init_dc,   "Sega", "Dreamcast (USA, NTSC)", MACHINE_NOT_WORKING )
CONS( 1998, dcjp,    0,      0,      dc,      dc,    dc_cons_state, init_dc,   "Sega", "Dreamcast (Japan, NTSC)", MACHINE_NOT_WORKING )
CONS( 1999, dceu,    dcjp,   0,      dc,      dc,    dc_cons_state, init_dc,   "Sega", "Dreamcast (Europe, PAL)", MACHINE_NOT_WORKING )
CONS( 200?, dctream, dcjp,   0,      dc,      dc,    dc_cons_state, init_tream,"<unknown>", "Treamcast", MACHINE_NOT_WORKING )
CONS( 1998, dcdev,   0,      0,      dc,      dc,    dc_cons_state, init_dc,   "Sega", "HKT-0120 Sega Dreamcast Development Box", MACHINE_NOT_WORKING )

/*
Fish Life - interactive aquarium simulator
Consists of HKS-0300 main unit and HKS-0100 LCD with touch screen

  HKS-0300
  Fish Life
  670-14239A
  (c) 2000 Sega
  components:
    Dreamcast VA1 motherboard
    GD-ROM drive
    PSU
    173-8100B / 837-14049 IC BD SW FL
      backplate with up/down/left/right/A/B/Start buttons
    171-8097B / 837-14046 IC BD FL
      main components:
        315-6211-AB - Dreamcast game controller IC
        315-6182    - Dreamcast microphone controller IC
        connectors

 HKS-0100 touch screen wired to SH-4's SCIF serial port. Communication is one-way, touch packet format is:
 0100000T 00xxxxxx 00XXXXXX 00yyyyyy 00YYYYYY 00--zzzz 00------
  T: 1 - touch, 0 - release
  x: X value low bits
  X: X value high bits
  y: Y value low bits
  Y: Y value high bits
  z: unused
 X/Y values range seems 3000x2294

 HKS-0200 software GD-ROMs:
  HDR-0093 673-01613 Fish Life Red Sea Playful Edition
 *HDR-0094 673-01672 Fish Life Amazon Playful Edition
  HDR-0095 673-01??? Fish Life Episode 1 Basic Edition
  HDR-0096 673-01??? Fish Life Episode 2 Basic Edition
  HDR-0097 673-01??? Fish Life Episode 3 Basic Edition
  MSD-0001 ???-????? Fish Life Red Sea & Amazon PDP Ver.
 * denotes these games are archived.

 Machines high likely based on Fish Life:
  タッチであそぼ！ / Play with a touch! (2001) - touch screen cabinet for McDonald's Japan https://www.famitsu.com/game/news/2001/09/13/103,1000362656,1276,0,0.html
  タッチでポン！ / Pong by touch! (2001) - sushi ordering system https://web.archive.org/web/20180421214402/sega.jp/fb/creators/vol_13/1.html

 notes:
  Some sources claims Playful and Basic editions hardware is not the same, has to be verified.
  Press down+B for Test Mode
*/
ROM_START( dcfish )
	ROM_REGION64_LE(0x200000, "maincpu", 0)
	ROM_LOAD( "mpr-21931.ic501", 0x000000, 0x200000, CRC(89f2b1a1) SHA1(8951d1bb219ab2ff8583033d2119c899cc81f18c) ) // regular v1.0d 3.3v BIOS

	// similar Dreamcast flashes, machine_name was changed to "Fish Life", machine_code2 is 0xff (verified  by software)
	ROM_REGION64_LE(0x040000, "dcflash", ROMREGION_ERASEFF)
	ROM_LOAD( "fish_flash.bin", 0x020000, 0x020000, CRC(f7f36b7b) SHA1(f49d18de85c519c16d5447ca8ae39b62d1b8e483) ) // VA1 NTSC-JP

	DISK_REGION( "ata:0:gdrom" )
	DISK_IMAGE_READONLY( "fish_life_amazon", 0, SHA1(2cbba727b219bbbeddf551d0f3e80c5f8ecbe21f) ) // HDR-0094
ROM_END

/*    YEAR  NAME     PARENT  MACHINE  INPUT  CLASS          INIT       ROT   COMPANY FULLNAME */
GAME( 2000, dcfish,  0,      dc_fish, dcfish,dc_cons_state, init_dc, ROT0, "Sega", "Fish Life Amazon Playful Edition (Japan)", MACHINE_NOT_WORKING ) // requires SH-4 touch screen, crashes on attract mode with DRC
