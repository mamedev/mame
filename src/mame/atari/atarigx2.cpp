// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari GX2 hardware

    driver by Aaron Giles

    Moto Frenzy and Space Lords protection reverse engineered by:
        Morten Shearman Kirkegaard, Samuel Neves, Peter Wilhelmsen

    Games supported:
        * Space Lords (1992)
        * Moto Frenzy (1992)
        * Road Riot's Revenge Rally (1993)

    Known bugs:
        * Protection for Road Riot's Revenge
        * Missing DSPCOM board (Moto Frenzy SD) and Clarn board (Road Riot's Revenge, Space Lords and Moto Frenzy mini dx), both ADSP2105 based.

****************************************************************************

    Memory map (TBA)

***************************************************************************/


#include "emu.h"
#include "atarigx2.h"

#include "cpu/m68000/m68020.h"
#include "machine/eeprompar.h"
#include "emupal.h"
#include "speaker.h"



/*************************************
 *
 *  Initialization
 *
 *************************************/

void atarigx2_state::video_int_ack_w(uint32_t data)
{
	m_maincpu->set_input_line(M68K_IRQ_4, CLEAR_LINE);
}



/*************************************
 *
 *  I/O read dispatch.
 *
 *************************************/

uint32_t atarigx2_state::special_port2_r()
{
	int const temp = m_io_service->read();
	return (temp << 16) | temp;
}


uint32_t atarigx2_state::special_port3_r()
{
	int const temp = m_io_special->read();
	return (temp << 16) | temp;
}



uint8_t atarigx2_state::a2d_data_r(offs_t offset)
{
	uint8_t const result = m_adc->data_r();
	if (!machine().side_effects_disabled())
		m_adc->address_offset_start_w(offset, 0);
	return result;
}


void atarigx2_state::latch_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	/*
	    D13 = 68.DISA
	    D12 = ERASE
	    D11 = /MOGO
	    D8  = VCR
	    D5  = /XRESET
	    D4  = /SNDRES
	    D3  = CC.L
	    D0  = CC.R
	*/

	logerror("latch_w(%08X) & %08X\n", data, mem_mask);

	/* upper byte */
	if (ACCESSING_BITS_24_31)
	{
		/* bits 13-11 are the MO control bits */
		m_rle->control_write((data >> 27) & 7);
	}

	/* lower byte */
	if (ACCESSING_BITS_16_23)
		m_jsa->soundcpu().set_input_line(INPUT_LINE_RESET, BIT(data, 20) ? CLEAR_LINE : ASSERT_LINE);
}


void atarigx2_state::mo_command_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(m_mo_command);
	if (ACCESSING_BITS_0_15)
		m_rle->command_write(((data & 0xffff) == 2) ? atari_rle_objects_device::COMMAND_CHECKSUM : atari_rle_objects_device::COMMAND_DRAW);
}

/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void atarigx2_state::main_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x07ffff).rom();
	map(0xc80000, 0xc80fff).ram();
	map(0xd00000, 0xd0000f).r(FUNC(atarigx2_state::a2d_data_r)).umask32(0xff00ff00);
	map(0xd20000, 0xd20fff).rw("eeprom", FUNC(eeprom_parallel_28xx_device::read), FUNC(eeprom_parallel_28xx_device::write)).umask32(0xff00ff00);
	map(0xd40000, 0xd40fff).ram().w("palette", FUNC(palette_device::write32)).share("palette");
	map(0xd70000, 0xd71fff).ram();
	map(0xd72000, 0xd75fff).ram().w(m_playfield_tilemap, FUNC(tilemap_device::write32)).share("playfield");
	map(0xd76000, 0xd76fff).ram().w(m_alpha_tilemap, FUNC(tilemap_device::write32)).share("alpha");
	map(0xd77000, 0xd77fff).ram();
	map(0xd78000, 0xd78fff).ram().share("rle");
	map(0xd79000, 0xd7a1ff).ram();
	map(0xd7a200, 0xd7a203).ram().w(FUNC(atarigx2_state::mo_command_w)).share(m_mo_command);
	map(0xd7a204, 0xd7ffff).ram();
	map(0xd80000, 0xd9ffff).w("eeprom", FUNC(eeprom_parallel_28xx_device::unlock_write32));
	map(0xe06000, 0xe06000).w(m_jsa, FUNC(atari_jsa_iiis_device::main_command_w));
	map(0xe08000, 0xe08003).w(FUNC(atarigx2_state::latch_w));
	map(0xe0c000, 0xe0c003).w(FUNC(atarigx2_state::video_int_ack_w));
	map(0xe0e000, 0xe0e003).nopw();//watchdog_reset_w },
	map(0xe80000, 0xe80003).portr("P1_P2");
	map(0xe82000, 0xe82003).r(FUNC(atarigx2_state::special_port2_r));
	map(0xe82004, 0xe82007).r(FUNC(atarigx2_state::special_port3_r));
	map(0xe86000, 0xe86000).r(m_jsa, FUNC(atari_jsa_iiis_device::main_response_r));
	map(0xff8000, 0xffffff).ram();
}




/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( spclords )
	PORT_START("P1_P2")     /* 68.SW (A1=0,1) */
	PORT_BIT( 0x000000ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_START1 )                   /* Pilot RED button (P1 Start / Hyperspace) */
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)   /* Co-Pilot Right Thumb (P2 Nuke) */
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)   /* Co-Pilot Right Trigger (P2 Laser)*/
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)   /* Pilot Throttle reverse */
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_CUSTOM )                  /* TODO: RIGHT of DOUBLE */
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_CUSTOM )                  /* TODO: DOUBLE SYSTEM */
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_CUSTOM )                  /* TODO: 4 COIN COUNTERS */
	PORT_BIT( 0x00ff0000, IP_ACTIVE_LOW, IPT_UNUSED )                   /* Freeze / Unfreeze */
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_START2 )                   /* Co-Pilot BLUE button (P2 Start / Cloak) */
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)   /* Pilot Left Thumb (P1 Nuke) */
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)   /* Pilot Left Trigger (P1 Laser) */
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)   /* Pilot Throttle forward */
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)   /* Pilot Rearview */
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SERVICE")      /* 68.STATUS (A2=0) */
	PORT_BIT( 0x0007, IP_ACTIVE_LOW, IPT_CUSTOM )  /* +5V */
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("adc", FUNC(adc0808_device::eoc_r)) // A2D.EOC
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_SOUND_TO_MAIN_READY("jsa") // /AUDIRQ
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_MAIN_TO_SOUND_READY("jsa") // /AUDFULL
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SPECIAL")     /* 68.STATUS (A2=1) */
	PORT_BIT( 0x0003, IP_ACTIVE_LOW, IPT_CUSTOM )  /* +5V */
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_CUSTOM )  /* /XIRQ */
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_CUSTOM )  /* /XFULL */
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_CUSTOM )  /* /SERVICER */
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_CUSTOM )  /* /SER.L */
	PORT_BIT( 0x00c0, IP_ACTIVE_LOW, IPT_CUSTOM )  /* +5V */
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("A2D0")      /* A2D @ 0xD00000 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("A2D1")      /* A2D @ 0xD00002 */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1) // Co-Pilot U/D

	PORT_START("A2D2")      /* A2D @ 0xD00004 */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2) // Pilot L/R

	PORT_START("A2D3")      /* A2D @ 0xD00006 */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2) // Pilot U/D

	PORT_START("A2D4")      /* A2D @ 0xD00008 */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1) // Co-Pilot L/R

	PORT_START("A2D5")      /* A2D @ 0xD0000A */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("A2D6")      /* A2D @ 0xD0000C */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("A2D7")      /* A2D @ 0xD0000E */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( motofren )
	PORT_START("P1_P2")     /* 68.SW (A1=0,1) */
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00ff0000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_START1 )                       /* Start/fire */
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("AUX3")
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("AUX2")
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("AUX1")
	PORT_BIT( 0xf0000000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SERVICE")       /* 68.STATUS (A2=0) */
	PORT_BIT( 0x0007, IP_ACTIVE_LOW, IPT_UNUSED )   /* +5V */
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("adc", FUNC(adc0808_device::eoc_r)) // A2D.EOC
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_SOUND_TO_MAIN_READY("jsa") // /AUDIRQ
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_MAIN_TO_SOUND_READY("jsa") // /AUDFULL
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SPECIAL")       /* 68.STATUS (A2=1) */
	PORT_BIT( 0x0003, IP_ACTIVE_LOW, IPT_CUSTOM )  /* +5V */
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_CUSTOM )  /* /XIRQ */
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_CUSTOM )  /* /XFULL */
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_CUSTOM )  /* /SERVICER */
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_CUSTOM )  /* /SER.L */
	PORT_BIT( 0x00c0, IP_ACTIVE_LOW, IPT_CUSTOM )  /* +5V */
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("A2D0")      /* A2D @ 0xD00000 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("A2D1")      /* A2D @ 0xD00002 */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_REVERSE

	PORT_START("A2D2")      /* A2D @ 0xD00004 */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(16) PORT_NAME("Throttle") PORT_REVERSE

	PORT_START("A2D3")      /* A2D @ 0xD00006 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("A2D4")      /* A2D @ 0xD00008 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("A2D5")      /* A2D @ 0xD0000A */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("A2D6")      /* A2D @ 0xD0000C */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("A2D7")      /* A2D @ 0xD0000E */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( rrreveng )
	PORT_START("P1_P2")     /* 68.SW (A1=0,1) */
	PORT_BIT( 0x000000ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0000fe00, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00ff0000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_A) PORT_PLAYER(1)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_S) PORT_PLAYER(1)
	PORT_BIT( 0x30000000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SERVICE")       /* 68.STATUS (A2=0) */
	PORT_BIT( 0x0007, IP_ACTIVE_LOW, IPT_CUSTOM )  /* +5V */
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("adc", FUNC(adc0808_device::eoc_r)) // A2D.EOC
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_SOUND_TO_MAIN_READY("jsa") // /AUDIRQ
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_MAIN_TO_SOUND_READY("jsa") // /AUDFULL
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SPECIAL")       /* 68.STATUS (A2=1) */
	PORT_BIT( 0x0003, IP_ACTIVE_LOW, IPT_CUSTOM )  /* +5V */
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_CUSTOM )  /* /XIRQ */
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_CUSTOM )  /* /XFULL */
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_CUSTOM )  /* /SERVICER */
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_CUSTOM )  /* /SER.L */
	PORT_BIT( 0x00c0, IP_ACTIVE_LOW, IPT_CUSTOM )  /* +5V */
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("A2D0")      /* A2D @ 0xD00000 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("A2D1")      /* A2D @ 0xD00002 */
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)

	PORT_START("A2D2")      /* A2D @ 0xD00004 */
	PORT_BIT( 0xff, 0x10, IPT_PEDAL ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)

	PORT_START("A2D3")      /* A2D @ 0xD00006 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("A2D4")      /* A2D @ 0xD00008 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("A2D5")      /* A2D @ 0xD0000A */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("A2D6")      /* A2D @ 0xD0000C */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("A2D7")      /* A2D @ 0xD0000E */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout pflayout =
{
	8,8,
	RGN_FRAC(1,3),
	5,
	{ 0, 0, 1, 2, 3 },
	{ RGN_FRAC(1,3)+0, RGN_FRAC(1,3)+4, 0, 4, RGN_FRAC(1,3)+8, RGN_FRAC(1,3)+12, 8, 12 },
	{ 0*8, 2*8, 4*8, 6*8, 8*8, 10*8, 12*8, 14*8 },
	16*8
};

static const gfx_layout pftoplayout =
{
	8,8,
	RGN_FRAC(1,3),
	6,
	{ RGN_FRAC(2,3)+0, RGN_FRAC(2,3)+4, 0, 0, 0, 0 },
	{ 3, 2, 1, 0, 11, 10, 9, 8 },
	{ 0*8, 2*8, 4*8, 6*8, 8*8, 10*8, 12*8, 14*8 },
	16*8
};

static GFXDECODE_START( gfx_atarigx2 )
	GFXDECODE_ENTRY( "tiles", 0, pflayout,             0x000, 64 )
	GFXDECODE_ENTRY( "chars", 0, gfx_8x8x4_packed_msb, 0x000, 16 )
	GFXDECODE_ENTRY( "tiles", 0, pftoplayout,          0x000, 64 )
GFXDECODE_END

static const atari_rle_objects_config modesc_0x200 =
{
	0,          /* left clip coordinate */
	0,          /* right clip coordinate */
	0x200,      /* base palette entry */

	{{ 0x7fff,0,0,0,0,0,0,0 }}, /* mask for the code index */
	{{ 0,0x01f0,0,0,0,0,0,0 }}, /* mask for the color */
	{{ 0,0,0xffc0,0,0,0,0,0 }}, /* mask for the X position */
	{{ 0,0,0,0xffc0,0,0,0,0 }}, /* mask for the Y position */
	{{ 0,0,0,0,0xffff,0,0,0 }}, /* mask for the scale factor */
	{{ 0x8000,0,0,0,0,0,0,0 }}, /* mask for the horizontal flip */
	{{ 0,0,0,0,0,0,0x00ff,0 }}, /* mask for the order */
	{{ 0,0x0e00,0,0,0,0,0,0 }}, /* mask for the priority */
	{{ 0 }}                     /* mask for the VRAM target */
};

static const atari_rle_objects_config modesc_0x400 =
{
	0,          /* left clip coordinate */
	0,          /* right clip coordinate */
	0x400,      /* base palette entry */

	{{ 0x7fff,0,0,0,0,0,0,0 }}, /* mask for the code index */
	{{ 0,0x03f0,0,0,0,0,0,0 }}, /* mask for the color */
	{{ 0,0,0xffc0,0,0,0,0,0 }}, /* mask for the X position */
	{{ 0,0,0,0xffc0,0,0,0,0 }}, /* mask for the Y position */
	{{ 0,0,0,0,0xffff,0,0,0 }}, /* mask for the scale factor */
	{{ 0x8000,0,0,0,0,0,0,0 }}, /* mask for the horizontal flip */
	{{ 0,0,0,0,0,0,0x00ff,0 }}, /* mask for the order */
	{{ 0,0x0e00,0,0,0,0,0,0 }}, /* mask for the priority */
	{{ 0 }}                     /* mask for the VRAM target */
};



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void atarigx2_state::atarigx2(machine_config &config)
{
	/* basic machine hardware */
	M68EC020(config, m_maincpu, 14.318181_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &atarigx2_state::main_map);

	TIMER(config, "scantimer").configure_scanline(FUNC(atarigx2_state::scanline_update), m_screen, 0, 8);

	ADC0809(config, m_adc, 14.318181_MHz_XTAL/16);
	m_adc->in_callback<0>().set_ioport("A2D0");
	m_adc->in_callback<1>().set_ioport("A2D1");
	m_adc->in_callback<2>().set_ioport("A2D2");
	m_adc->in_callback<3>().set_ioport("A2D3");
	m_adc->in_callback<4>().set_ioport("A2D4");
	m_adc->in_callback<5>().set_ioport("A2D5");
	m_adc->in_callback<6>().set_ioport("A2D6");
	m_adc->in_callback<7>().set_ioport("A2D7");

	EEPROM_2816(config, "eeprom").lock_after_write(true);

	/* video hardware */
	GFXDECODE(config, m_gfxdecode, "palette", gfx_atarigx2);
	PALETTE(config, "palette").set_format(palette_device::IRGB_1555, 2048);

	TILEMAP(config, m_playfield_tilemap, m_gfxdecode, 2, 8,8);
	m_playfield_tilemap->set_layout(FUNC(atarigx2_state::atarigx2_playfield_scan), 128,64);
	m_playfield_tilemap->set_info_callback(FUNC(atarigx2_state::get_playfield_tile_info));
	TILEMAP(config, m_alpha_tilemap, m_gfxdecode, 2, 8,8, TILEMAP_SCAN_ROWS, 64,32, 0).set_info_callback(FUNC(atarigx2_state::get_alpha_tile_info));

	SCREEN(config, m_screen);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	/* note: these parameters are from published specs, not derived */
	/* the board uses a pair of GALs to determine H and V parameters */
	m_screen->set_raw(14.318181_MHz_XTAL/2, 456, 0, 336, 262, 0, 240);
	m_screen->set_screen_update(FUNC(atarigx2_state::screen_update));
	m_screen->set_palette("palette");
	m_screen->screen_vblank().set_inputline(m_maincpu, M68K_IRQ_4, ASSERT_LINE);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	ATARI_JSA_IIIS(config, m_jsa);
	m_jsa->main_int_cb().set_inputline(m_maincpu, M68K_IRQ_5);
	m_jsa->test_read_cb().set_ioport("SERVICE").bit(6);
	m_jsa->add_route(0, "speaker", 0.7, 0);
	m_jsa->add_route(1, "speaker", 0.7, 1);
}

void atarigx2_state::atarigx2_0x200(machine_config &config)
{
	atarigx2(config);
	ATARI_136094_0072(config, m_xga);
	ATARI_RLE_OBJECTS(config, m_rle, modesc_0x200);
}

void atarigx2_state::atarigx2_0x400(machine_config &config)
{
	atarigx2(config);
	ATARI_136095_0072(config, m_xga);
	ATARI_RLE_OBJECTS(config, m_rle, modesc_0x400);
}



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( spclords )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 8*64k for 68000 code */
	ROM_LOAD32_BYTE( "main0rc.095",  0x000000, 0x020000, CRC(82ddf575) SHA1(e2821b2f576694bce4590cd03b944b7991ecbc27) )
	ROM_LOAD32_BYTE( "main1rc.095",  0x000001, 0x020000, CRC(69d64819) SHA1(e9cb99b0ba2a0e23e7699a61130e5e8a4b632db4) )
	ROM_LOAD32_BYTE( "main2rc.095",  0x000002, 0x020000, CRC(49d30630) SHA1(2d0f2abe5d17b4cf575f80687502fac33c7f3206) )
	ROM_LOAD32_BYTE( "main3rc.095",  0x000003, 0x020000, CRC(3872424c) SHA1(db08ad9386dfe8fa4e2a83a2505118a636247279) )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) /* 64k for 6502 code */
	ROM_LOAD( "136095.80a", 0x00000, 0x10000, CRC(33bc0ede) SHA1(2ee30d9125057cdfbdb83e4dbf28306c35a9c233) )

	ROM_REGION( 0x60000, "tiles", 0 )
	ROM_LOAD( "136095.30a", 0x00000, 0x20000, CRC(27e0cfec) SHA1(03df57757d091f9a0b8c8d98d091dd759f570788) ) /* playfield, planes 0-1 */
	ROM_LOAD( "136095.31a", 0x20000, 0x20000, CRC(5529cdc7) SHA1(8aff8a42fb2a86b7e4666940da4c1ee19dab6281) ) /* playfield, planes 2-3 */
	ROM_FILL(               0x40000, 0x20000, 0x00 )          /* playfield, planes 4-5 */

	ROM_REGION( 0x020000, "chars", 0 )
	ROM_LOAD( "136095.25a", 0x000000, 0x20000, CRC(1669496e) SHA1(005deaafd6156505e3a27966123e58928837ad9f) ) /* alphanumerics */

	ROM_REGION16_BE( 0x600000, "rle", 0 )
	ROM_LOAD16_BYTE( "136095.41b", 0x000000, 0x80000, CRC(02ce7e07) SHA1(a48a6930c8ca4e2d0e4bc77a558730fa790be3b5) )
	ROM_LOAD16_BYTE( "136095.40b", 0x000001, 0x80000, CRC(abb80720) SHA1(0e02688454f59b90d38d548808f794294f5c9e7e) )
	ROM_LOAD16_BYTE( "136095.43b", 0x100000, 0x80000, CRC(26526345) SHA1(30ef83f63aca3a846dfc2828e3147208e3e350ca) )
	ROM_LOAD16_BYTE( "136095.42b", 0x100001, 0x80000, CRC(c7a163df) SHA1(deb2c1fe57b6673d98ea9880a16e26751cc6e3a5) )
	ROM_LOAD16_BYTE( "136095.45b", 0x200000, 0x80000, CRC(53d01714) SHA1(6ef584eb6c723d4843dad9e00da132e65be5ce25) )
	ROM_LOAD16_BYTE( "136095.44b", 0x200001, 0x80000, CRC(60a16e4d) SHA1(13629b7039e77a654e22d7efb3bc01413b7ec187) )
	ROM_LOAD16_BYTE( "136095.47b", 0x300000, 0x80000, CRC(41c873a3) SHA1(87c4e35a198ba5f73a545ccd176e9c5d454d194a) )
	ROM_LOAD16_BYTE( "136095.46b", 0x300001, 0x80000, CRC(e885aece) SHA1(f02d2028b74ad26447e4d4c9d4d2d8136e0be8f5) )
	ROM_LOAD16_BYTE( "136095.49b", 0x400000, 0x80000, CRC(7af90faf) SHA1(14629961f70ed969525d9aba49b9637115d26f44) )
	ROM_LOAD16_BYTE( "136095.48b", 0x400001, 0x80000, CRC(6c553406) SHA1(b5a596dc69620d935eab9d9afc45377ad98ba77e) )
	ROM_LOAD16_BYTE( "136095.51b", 0x500000, 0x80000, CRC(97541074) SHA1(f9f75bfc4af9587f4a9630ad93d9cd0efd89e4f4) )
	ROM_LOAD16_BYTE( "136095.50b", 0x500001, 0x80000, CRC(a1c11ae8) SHA1(53fb2f376aae0aa346f9f911d6d8a73753c67d6e) )

	ROM_REGION( 0x80000, "jsa:oki1", 0 )
	ROM_LOAD( "136095.81a",  0x00000, 0x80000, CRC(212560dd) SHA1(9d90bca5b478050d640b2393c9d3d59a4bd493dd) )

	ROM_REGION( 0x80000, "jsa:oki2", 0 )
	ROM_LOAD( "136095.81a",  0x00000, 0x80000, CRC(212560dd) SHA1(9d90bca5b478050d640b2393c9d3d59a4bd493dd) )

	ROM_REGION( 0x0600, "proms", 0 )    /* microcode for growth renderer */
	ROM_LOAD( "136095-001a.bin",  0x0000, 0x0200, BAD_DUMP CRC(a70ade3f) SHA1(f4a558b17767eed2683c768d1b441e75edcff967) ) /* Not dumped from actual PCB, but seems common to the platform */
	ROM_LOAD( "136095-002a.bin",  0x0200, 0x0200, BAD_DUMP CRC(f4768b4d) SHA1(a506fa5386ab0ea2851ff1f8474d4bfc66deaa70) ) /* Confirmed for Moto Frenzy & Road Riot's Revenge */
	ROM_LOAD( "136095-003a.bin",  0x0400, 0x0200, BAD_DUMP CRC(22a76ad4) SHA1(ce840c283bbd3a5f19dc8d91b19d1571eff51ff4) )
ROM_END


ROM_START( spclordsb )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 8*64k for 68000 code */
	ROM_LOAD32_BYTE( "136095.21b", 0x00000, 0x20000, CRC(2ba99ce2) SHA1(5d8d138698c29838a85da1721c3400c666a14e18) )
	ROM_LOAD32_BYTE( "136095.22b", 0x00001, 0x20000, CRC(631c5009) SHA1(6b2ea907087e411579f55dff60724ba33afa8a06) )
	ROM_LOAD32_BYTE( "136095.23b", 0x00002, 0x20000, CRC(bc64ab63) SHA1(999851a39123f6a01cb83d97ea744e12590b6e7e) )
	ROM_LOAD32_BYTE( "136095.24b", 0x00003, 0x20000, CRC(7284a01a) SHA1(afa866c97b4c3df7fda3c196072231096beaa0db) )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) /* 64k for 6502 code */
	ROM_LOAD( "136095.80a", 0x00000, 0x10000, CRC(33bc0ede) SHA1(2ee30d9125057cdfbdb83e4dbf28306c35a9c233) )

	ROM_REGION( 0x60000, "tiles", 0 )
	ROM_LOAD( "136095.30a", 0x00000, 0x20000, CRC(27e0cfec) SHA1(03df57757d091f9a0b8c8d98d091dd759f570788) ) /* playfield, planes 0-1 */
	ROM_LOAD( "136095.31a", 0x20000, 0x20000, CRC(5529cdc7) SHA1(8aff8a42fb2a86b7e4666940da4c1ee19dab6281) ) /* playfield, planes 2-3 */
	ROM_FILL(               0x40000, 0x20000, 0x00 )          /* playfield, planes 4-5 */

	ROM_REGION( 0x020000, "chars", 0 )
	ROM_LOAD( "136095.25a", 0x000000, 0x20000, CRC(1669496e) SHA1(005deaafd6156505e3a27966123e58928837ad9f) ) /* alphanumerics */

	ROM_REGION16_BE( 0x600000, "rle", 0 )
	ROM_LOAD16_BYTE( "136095.41b", 0x000000, 0x80000, CRC(02ce7e07) SHA1(a48a6930c8ca4e2d0e4bc77a558730fa790be3b5) )
	ROM_LOAD16_BYTE( "136095.40b", 0x000001, 0x80000, CRC(abb80720) SHA1(0e02688454f59b90d38d548808f794294f5c9e7e) )
	ROM_LOAD16_BYTE( "136095.43b", 0x100000, 0x80000, CRC(26526345) SHA1(30ef83f63aca3a846dfc2828e3147208e3e350ca) )
	ROM_LOAD16_BYTE( "136095.42b", 0x100001, 0x80000, CRC(c7a163df) SHA1(deb2c1fe57b6673d98ea9880a16e26751cc6e3a5) )
	ROM_LOAD16_BYTE( "136095.45b", 0x200000, 0x80000, CRC(53d01714) SHA1(6ef584eb6c723d4843dad9e00da132e65be5ce25) )
	ROM_LOAD16_BYTE( "136095.44b", 0x200001, 0x80000, CRC(60a16e4d) SHA1(13629b7039e77a654e22d7efb3bc01413b7ec187) )
	ROM_LOAD16_BYTE( "136095.47b", 0x300000, 0x80000, CRC(41c873a3) SHA1(87c4e35a198ba5f73a545ccd176e9c5d454d194a) )
	ROM_LOAD16_BYTE( "136095.46b", 0x300001, 0x80000, CRC(e885aece) SHA1(f02d2028b74ad26447e4d4c9d4d2d8136e0be8f5) )
	ROM_LOAD16_BYTE( "136095.49b", 0x400000, 0x80000, CRC(7af90faf) SHA1(14629961f70ed969525d9aba49b9637115d26f44) )
	ROM_LOAD16_BYTE( "136095.48b", 0x400001, 0x80000, CRC(6c553406) SHA1(b5a596dc69620d935eab9d9afc45377ad98ba77e) )
	ROM_LOAD16_BYTE( "136095.51b", 0x500000, 0x80000, CRC(97541074) SHA1(f9f75bfc4af9587f4a9630ad93d9cd0efd89e4f4) )
	ROM_LOAD16_BYTE( "136095.50b", 0x500001, 0x80000, CRC(a1c11ae8) SHA1(53fb2f376aae0aa346f9f911d6d8a73753c67d6e) )

	ROM_REGION( 0x80000, "jsa:oki1", 0 )
	ROM_LOAD( "136095.81a",  0x00000, 0x80000, CRC(212560dd) SHA1(9d90bca5b478050d640b2393c9d3d59a4bd493dd) )

	ROM_REGION( 0x80000, "jsa:oki2", 0 )
	ROM_LOAD( "136095.81a",  0x00000, 0x80000, CRC(212560dd) SHA1(9d90bca5b478050d640b2393c9d3d59a4bd493dd) )

	ROM_REGION( 0x0600, "proms", 0 )    /* microcode for growth renderer */
	ROM_LOAD( "136095-001a.bin",  0x0000, 0x0200, BAD_DUMP CRC(a70ade3f) SHA1(f4a558b17767eed2683c768d1b441e75edcff967) ) /* Not dumped from actual PCB, but seems common to the platform */
	ROM_LOAD( "136095-002a.bin",  0x0200, 0x0200, BAD_DUMP CRC(f4768b4d) SHA1(a506fa5386ab0ea2851ff1f8474d4bfc66deaa70) ) /* Confirmed for Moto Frenzy & Road Riot's Revenge */
	ROM_LOAD( "136095-003a.bin",  0x0400, 0x0200, BAD_DUMP CRC(22a76ad4) SHA1(ce840c283bbd3a5f19dc8d91b19d1571eff51ff4) )
ROM_END


ROM_START( spclordsg )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 8*64k for 68000 code */
	ROM_LOAD32_BYTE( "german0.095",  0x000000, 0x020000, CRC(5a885f8e) SHA1(ec7ef0d76320099a6f3fad80c9ec404ce8602557) )
	ROM_LOAD32_BYTE( "german1.095",  0x000001, 0x020000, CRC(56f8d517) SHA1(4bcd2d368d48e7492a739aa3041a40e1518b8c94) )
	ROM_LOAD32_BYTE( "german2.095",  0x000002, 0x020000, CRC(9527df10) SHA1(c18434c1f40fa23a6cc78df7104c7e2e6888d189) )
	ROM_LOAD32_BYTE( "german3.095",  0x000003, 0x020000, CRC(0aaaad66) SHA1(382b859be652d7d83319907d354d294643cef2b4) )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) /* 64k for 6502 code */
	ROM_LOAD( "136095.80a", 0x00000, 0x10000, CRC(33bc0ede) SHA1(2ee30d9125057cdfbdb83e4dbf28306c35a9c233) )

	ROM_REGION( 0x60000, "tiles", 0 )
	ROM_LOAD( "136095.30a", 0x00000, 0x20000, CRC(27e0cfec) SHA1(03df57757d091f9a0b8c8d98d091dd759f570788) ) /* playfield, planes 0-1 */
	ROM_LOAD( "136095.31a", 0x20000, 0x20000, CRC(5529cdc7) SHA1(8aff8a42fb2a86b7e4666940da4c1ee19dab6281) ) /* playfield, planes 2-3 */
	ROM_FILL(               0x40000, 0x20000, 0x00 )          /* playfield, planes 4-5 */

	ROM_REGION( 0x020000, "chars", 0 )
	ROM_LOAD( "136095.25a", 0x000000, 0x20000, CRC(1669496e) SHA1(005deaafd6156505e3a27966123e58928837ad9f) ) /* alphanumerics */

	ROM_REGION16_BE( 0x600000, "rle", 0 )
	ROM_LOAD16_BYTE( "136095.41a", 0x000000, 0x80000, CRC(5f9743ee) SHA1(fa521572f8dd2eda566f90d1345adba3d0b8c48f) )
	ROM_LOAD16_BYTE( "136095.40a", 0x000001, 0x80000, CRC(99b26863) SHA1(21682771d310c73d4431dde5e72398a69a6f3d53) )
	ROM_LOAD16_BYTE( "136095.43a", 0x100000, 0x80000, CRC(9c0e09a5) SHA1(039dc52318935f686230f57a7b39b9c62280cbf9) )
	ROM_LOAD16_BYTE( "136095.42a", 0x100001, 0x80000, CRC(523bbb39) SHA1(635e859e634f6ef0ba26775653bd38a5ca9ddbbc) )
	ROM_LOAD16_BYTE( "136095.45a", 0x200000, 0x80000, CRC(ac9bf600) SHA1(7da51103c419c7e09992bea67b5413bd2c9a0bb6) )
	ROM_LOAD16_BYTE( "136095.44a", 0x200001, 0x80000, CRC(58949b04) SHA1(c05cc0b691e110532a04e5de43564ddef6c65769) )
	ROM_LOAD16_BYTE( "136095.47a", 0x300000, 0x80000, CRC(675fee50) SHA1(2acbebce99d84ba6029baba48aa0271acb465c1c) )
	ROM_LOAD16_BYTE( "136095.46a", 0x300001, 0x80000, CRC(c948c7b6) SHA1(c5fc684bc8b38370221fe126e6d1461533908cfd) )
	ROM_LOAD16_BYTE( "136095.49a", 0x400000, 0x80000, CRC(04af9a77) SHA1(a985ef7617480e3606068369a1ddb7d90739aeb2) )
	ROM_LOAD16_BYTE( "136095.48a", 0x400001, 0x80000, CRC(5ad113aa) SHA1(71bb40520578447bef1eb78bb60f97a12720ecd9) )
	ROM_LOAD16_BYTE( "136095.51a", 0x500000, 0x80000, CRC(4635c534) SHA1(7261508052e3b17a552b43bc3d4ad7cd2d1f6af9) )
	ROM_LOAD16_BYTE( "136095.50a", 0x500001, 0x80000, CRC(94bde47d) SHA1(dde8f0184a2d7e9f7eb961af2d9d016399ec18fc) )

	ROM_REGION( 0x80000, "jsa:oki1", 0 )
	ROM_LOAD( "136095.81a",  0x00000, 0x80000, CRC(212560dd) SHA1(9d90bca5b478050d640b2393c9d3d59a4bd493dd) )

	ROM_REGION( 0x80000, "jsa:oki2", 0 )
	ROM_LOAD( "136095.81a",  0x00000, 0x80000, CRC(212560dd) SHA1(9d90bca5b478050d640b2393c9d3d59a4bd493dd) )

	ROM_REGION( 0x0600, "proms", 0 )    /* microcode for growth renderer */
	ROM_LOAD( "136095-001a.bin",  0x0000, 0x0200, BAD_DUMP CRC(a70ade3f) SHA1(f4a558b17767eed2683c768d1b441e75edcff967) ) /* Not dumped from actual PCB, but seems common to the platform */
	ROM_LOAD( "136095-002a.bin",  0x0200, 0x0200, BAD_DUMP CRC(f4768b4d) SHA1(a506fa5386ab0ea2851ff1f8474d4bfc66deaa70) ) /* Confirmed for Moto Frenzy & Road Riot's Revenge */
	ROM_LOAD( "136095-003a.bin",  0x0400, 0x0200, BAD_DUMP CRC(22a76ad4) SHA1(ce840c283bbd3a5f19dc8d91b19d1571eff51ff4) )
ROM_END


ROM_START( spclordsa )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 8*64k for 68000 code */
	ROM_LOAD32_BYTE( "136095.21a", 0x00000, 0x20000, CRC(fe8edb0b) SHA1(ae50a637df476c62f8194577cdca2677f9b5cbd0) )
	ROM_LOAD32_BYTE( "136095.22a", 0x00001, 0x20000, CRC(c2d2867b) SHA1(481fe54d6cd8698bfd2776e2af6f51332304b7ba) )
	ROM_LOAD32_BYTE( "136095.23a", 0x00002, 0x20000, CRC(20a0e443) SHA1(54597342901d6b38dddbe754f41ceeddcc4e5289) )
	ROM_LOAD32_BYTE( "136095.24a", 0x00003, 0x20000, CRC(d3f0439c) SHA1(f9245f448b77187b4cd5d9436b5caebd2800be5d))

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) /* 64k for 6502 code */
	ROM_LOAD( "136095.80a", 0x00000, 0x10000, CRC(33bc0ede) SHA1(2ee30d9125057cdfbdb83e4dbf28306c35a9c233) )

	ROM_REGION( 0x60000, "tiles", 0 )
	ROM_LOAD( "136095.30a", 0x00000, 0x20000, CRC(27e0cfec) SHA1(03df57757d091f9a0b8c8d98d091dd759f570788) ) /* playfield, planes 0-1 */
	ROM_LOAD( "136095.31a", 0x20000, 0x20000, CRC(5529cdc7) SHA1(8aff8a42fb2a86b7e4666940da4c1ee19dab6281) ) /* playfield, planes 2-3 */
	ROM_FILL(               0x40000, 0x20000, 0x00 )          /* playfield, planes 4-5 */

	ROM_REGION( 0x020000, "chars", 0 )
	ROM_LOAD( "136095.25a", 0x000000, 0x20000, CRC(1669496e) SHA1(005deaafd6156505e3a27966123e58928837ad9f) ) /* alphanumerics */

	ROM_REGION16_BE( 0x600000, "rle", 0 )
	ROM_LOAD16_BYTE( "136095.41a", 0x000000, 0x80000, CRC(5f9743ee) SHA1(fa521572f8dd2eda566f90d1345adba3d0b8c48f) )
	ROM_LOAD16_BYTE( "136095.40a", 0x000001, 0x80000, CRC(99b26863) SHA1(21682771d310c73d4431dde5e72398a69a6f3d53) )
	ROM_LOAD16_BYTE( "136095.43a", 0x100000, 0x80000, CRC(9c0e09a5) SHA1(039dc52318935f686230f57a7b39b9c62280cbf9) )
	ROM_LOAD16_BYTE( "136095.42a", 0x100001, 0x80000, CRC(523bbb39) SHA1(635e859e634f6ef0ba26775653bd38a5ca9ddbbc) )
	ROM_LOAD16_BYTE( "136095.45a", 0x200000, 0x80000, CRC(ac9bf600) SHA1(7da51103c419c7e09992bea67b5413bd2c9a0bb6) )
	ROM_LOAD16_BYTE( "136095.44a", 0x200001, 0x80000, CRC(58949b04) SHA1(c05cc0b691e110532a04e5de43564ddef6c65769) )
	ROM_LOAD16_BYTE( "136095.47a", 0x300000, 0x80000, CRC(675fee50) SHA1(2acbebce99d84ba6029baba48aa0271acb465c1c) )
	ROM_LOAD16_BYTE( "136095.46a", 0x300001, 0x80000, CRC(c948c7b6) SHA1(c5fc684bc8b38370221fe126e6d1461533908cfd) )
	ROM_LOAD16_BYTE( "136095.49a", 0x400000, 0x80000, CRC(04af9a77) SHA1(a985ef7617480e3606068369a1ddb7d90739aeb2) )
	ROM_LOAD16_BYTE( "136095.48a", 0x400001, 0x80000, CRC(5ad113aa) SHA1(71bb40520578447bef1eb78bb60f97a12720ecd9) )
	ROM_LOAD16_BYTE( "136095.51a", 0x500000, 0x80000, CRC(4635c534) SHA1(7261508052e3b17a552b43bc3d4ad7cd2d1f6af9) )
	ROM_LOAD16_BYTE( "136095.50a", 0x500001, 0x80000, CRC(94bde47d) SHA1(dde8f0184a2d7e9f7eb961af2d9d016399ec18fc) )

	ROM_REGION( 0x80000, "jsa:oki1", 0 )
	ROM_LOAD( "136095.81a",  0x00000, 0x80000, CRC(212560dd) SHA1(9d90bca5b478050d640b2393c9d3d59a4bd493dd) )

	ROM_REGION( 0x80000, "jsa:oki2", 0 )
	ROM_LOAD( "136095.81a",  0x00000, 0x80000, CRC(212560dd) SHA1(9d90bca5b478050d640b2393c9d3d59a4bd493dd) )

	ROM_REGION( 0x0600, "proms", 0 )    /* microcode for growth renderer */
	ROM_LOAD( "136095-001a.bin",  0x0000, 0x0200, BAD_DUMP CRC(a70ade3f) SHA1(f4a558b17767eed2683c768d1b441e75edcff967) ) /* Not dumped from actual PCB, but seems common to the platform */
	ROM_LOAD( "136095-002a.bin",  0x0200, 0x0200, BAD_DUMP CRC(f4768b4d) SHA1(a506fa5386ab0ea2851ff1f8474d4bfc66deaa70) ) /* Confirmed for Moto Frenzy & Road Riot's Revenge */
	ROM_LOAD( "136095-003a.bin",  0x0400, 0x0200, BAD_DUMP CRC(22a76ad4) SHA1(ce840c283bbd3a5f19dc8d91b19d1571eff51ff4) )
ROM_END


ROM_START( motofren )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 8*64k for 68000 code */
	ROM_LOAD32_BYTE( "136094-0021i.23e", 0x000000, 0x020000, CRC(2c6ec446) SHA1(d83fee26b384e6fd783104746e6560504ae43ca6) )
	ROM_LOAD32_BYTE( "136094-0022i.23j", 0x000001, 0x020000, CRC(e7163e7b) SHA1(7ea8a7a63bd1befee4cf9e708949fca7f06572c1) )
	ROM_LOAD32_BYTE( "136094-0023i.37e", 0x000002, 0x020000, CRC(6b1c7626) SHA1(b318a5856bcbd6a8fc7eb92e4b9a576b8c16cbf3) )
	ROM_LOAD32_BYTE( "136094-0024i.37j", 0x000003, 0x020000, CRC(44c3cd2a) SHA1(a16046586cbaa000e056115c92b5f22bf49869ad) )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) /* 64k for 6502 code */
	ROM_LOAD( "136094-0080a.12c", 0x00000, 0x10000, CRC(0b1e565c) SHA1(03bdeafd8cf680f76bbd1f9aba6efac27f19a93c) )

	ROM_REGION( 0x180000, "tiles", 0 )
	ROM_LOAD( "136094-0036a.2d", 0x000000, 0x80000, CRC(1b63b493) SHA1(980141fec011fa2b5cb020eeecb4784d31679dba) ) /* playfield, planes 0-1 */
	ROM_LOAD( "136094-0037a.5d", 0x080000, 0x80000, CRC(6d290056) SHA1(fa32dbe5ac5e735d700d086353461eaa2c1dee55) ) /* playfield, planes 2-3 */
	ROM_LOAD( "136094-0038a.8d", 0x100000, 0x80000, CRC(38197c88) SHA1(dc5d4d878759503b8500e8e3a032f499bfeedcb1) ) /* playfield, planes 4-5 */

	ROM_REGION( 0x020000, "chars", 0 )
	ROM_LOAD( "136094-0025a.13n", 0x000000, 0x20000, CRC(6ab762ad) SHA1(c52dd207ff5adaffa458e020e7d452a1d1e65194) ) /* alphanumerics */

	ROM_REGION16_BE( 0x700000, "rle", 0 )
	ROM_LOAD16_BYTE( "136094-0041a.31n", 0x000000, 0x80000, CRC(474770e9) SHA1(507dac654d1c350ab530892e3ec19793629d3a07) )
	ROM_LOAD16_BYTE( "136094-0040a.31l", 0x000001, 0x80000, CRC(cb777468) SHA1(aa199bc02ab966b9f270057857aec50add8d684c) )
	ROM_LOAD16_BYTE( "136094-0043a.33n", 0x100000, 0x80000, CRC(353d2dc3) SHA1(82c2c862404ea4c94c9baee1d0ac32696fcf78bd) )
	ROM_LOAD16_BYTE( "136094-0042a.33l", 0x100001, 0x80000, CRC(17d49f77) SHA1(7ed85035128f3c9dc2bc3d9f387dba01a882e3f4) )
	ROM_LOAD16_BYTE( "136094-0045a.35n", 0x200000, 0x80000, CRC(13d89355) SHA1(becd43a51c643732025386d7781dcf784b20d031) )
	ROM_LOAD16_BYTE( "136094-0044a.35l", 0x200001, 0x80000, CRC(924c817e) SHA1(2c23fd8a85833875cfb2d60e1072bd59cea57e0c) )
	ROM_LOAD16_BYTE( "136094-0047a.37n", 0x300000, 0x80000, CRC(43ee7453) SHA1(3549f62e6b2f50d663fbb4068cb8907b5073ef5e) )
	ROM_LOAD16_BYTE( "136094-0046a.37l", 0x300001, 0x80000, CRC(980b9b92) SHA1(6a5cfb77b65c0a28dba705d4738c883be983cd4d) )
	ROM_LOAD16_BYTE( "136094-0049a.31t", 0x400000, 0x80000, CRC(25ac33af) SHA1(e76af04d2e2cbf5769c1f53e2eac4f85e57003ba) )
	ROM_LOAD16_BYTE( "136094-0048a.31r", 0x400001, 0x80000, CRC(d725a27d) SHA1(9d8d332a6c1d773888c2d9ef1d7ea1e865f6dd02) )
	ROM_LOAD16_BYTE( "136094-0051a.33t", 0x500000, 0x80000, CRC(a0fc90b2) SHA1(08ac2510287e8c3e8e52995d8f741b2a58e5b37b) )
	ROM_LOAD16_BYTE( "136094-0050a.33r", 0x500001, 0x80000, CRC(dcc206cf) SHA1(f0b7d5d289f4c0de99b536dd2874ffdabf648cb2) )
	ROM_LOAD16_BYTE( "136094-0053a.35t", 0x600000, 0x80000, CRC(74320763) SHA1(9cbf61c51dd96dc3e4a4227f3080766b9482a16a) )
	ROM_LOAD16_BYTE( "136094-0052a.35r", 0x600001, 0x80000, CRC(a7f9df2e) SHA1(c3e0c67081cf8f7b24350abf5a9adbb544ab44a7) )

	ROM_REGION( 0x80000, "jsa:oki1", 0 )
	ROM_LOAD( "136094-0082a.19e",  0x00000, 0x80000, CRC(fde543c4) SHA1(7d36d7f2f30d0ac40da77a36a47488d75474caaf) )

	ROM_REGION( 0x80000, "jsa:oki2", 0 )
	ROM_LOAD( "136094-0082a.19e",  0x00000, 0x80000, CRC(fde543c4) SHA1(7d36d7f2f30d0ac40da77a36a47488d75474caaf) )

	ROM_REGION( 0x0600, "proms", 0 )    /* microcode for growth renderer */
	ROM_LOAD( "136094-001a.22s",  0x0000, 0x0200, CRC(a70ade3f) SHA1(f4a558b17767eed2683c768d1b441e75edcff967) )
	ROM_LOAD( "136094-002a.21s",  0x0200, 0x0200, CRC(f4768b4d) SHA1(a506fa5386ab0ea2851ff1f8474d4bfc66deaa70) )
	ROM_LOAD( "136094-003a.20s",  0x0400, 0x0200, CRC(22a76ad4) SHA1(ce840c283bbd3a5f19dc8d91b19d1571eff51ff4) )
ROM_END


ROM_START( motofrenmd )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 8*64k for 68000 code */
	ROM_LOAD32_BYTE( "136094-0221a.23e", 0x00000, 0x20000, CRC(134e9ff0) SHA1(801b817bf49b4317a7518192025a878b9cd13f7f) )
	ROM_LOAD32_BYTE( "136094-0222a.23j", 0x00001, 0x20000, CRC(f6df65c7) SHA1(0a2092a509ae8c61e3f55c30c47bf39c71e2aa6e) )
	ROM_LOAD32_BYTE( "136094-0223a.37e", 0x00002, 0x20000, CRC(cdb04a4a) SHA1(ee342bdb5654e8b841b1f60e46d1bcae7c4e5cd2) )
	ROM_LOAD32_BYTE( "136094-0224a.37j", 0x00003, 0x20000, CRC(f3a9949f) SHA1(d3fa68fc63c505dd4c9d0e0c7f0625cc24ac9571) )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) /* 64k for 6502 code */
	ROM_LOAD( "136094-0080b.12c", 0x00000, 0x10000, CRC(5e542608) SHA1(8a10b5fac6ac120c7aae2edaa12413c9b8345d87) )

	ROM_REGION( 0x180000, "tiles", 0 ) /* Although verified, the manual states the label codes as 136094-0030 through 136094-0032 */
	ROM_LOAD( "136094-0036a.2d", 0x000000, 0x80000, CRC(1b63b493) SHA1(980141fec011fa2b5cb020eeecb4784d31679dba) ) /* playfield, planes 0-1 */
	ROM_LOAD( "136094-0037a.5d", 0x080000, 0x80000, CRC(6d290056) SHA1(fa32dbe5ac5e735d700d086353461eaa2c1dee55) ) /* playfield, planes 2-3 */
	ROM_LOAD( "136094-0038a.8d", 0x100000, 0x80000, CRC(38197c88) SHA1(dc5d4d878759503b8500e8e3a032f499bfeedcb1) ) /* playfield, planes 4-5 */

	ROM_REGION( 0x020000, "chars", 0 )
	ROM_LOAD( "136094-0025a.13n", 0x000000, 0x20000, CRC(6ab762ad) SHA1(c52dd207ff5adaffa458e020e7d452a1d1e65194) ) /* alphanumerics */

	ROM_REGION16_BE( 0x700000, "rle", 0 )
	ROM_LOAD16_BYTE( "136094-0041a.31n", 0x000000, 0x80000, CRC(474770e9) SHA1(507dac654d1c350ab530892e3ec19793629d3a07) )
	ROM_LOAD16_BYTE( "136094-0040a.31l", 0x000001, 0x80000, CRC(cb777468) SHA1(aa199bc02ab966b9f270057857aec50add8d684c) )
	ROM_LOAD16_BYTE( "136094-0043a.33n", 0x100000, 0x80000, CRC(353d2dc3) SHA1(82c2c862404ea4c94c9baee1d0ac32696fcf78bd) )
	ROM_LOAD16_BYTE( "136094-0042a.33l", 0x100001, 0x80000, CRC(17d49f77) SHA1(7ed85035128f3c9dc2bc3d9f387dba01a882e3f4) )
	ROM_LOAD16_BYTE( "136094-0045a.35n", 0x200000, 0x80000, CRC(13d89355) SHA1(becd43a51c643732025386d7781dcf784b20d031) )
	ROM_LOAD16_BYTE( "136094-0044a.35l", 0x200001, 0x80000, CRC(924c817e) SHA1(2c23fd8a85833875cfb2d60e1072bd59cea57e0c) )
	ROM_LOAD16_BYTE( "136094-0047a.37n", 0x300000, 0x80000, CRC(43ee7453) SHA1(3549f62e6b2f50d663fbb4068cb8907b5073ef5e) )
	ROM_LOAD16_BYTE( "136094-0046a.37l", 0x300001, 0x80000, CRC(980b9b92) SHA1(6a5cfb77b65c0a28dba705d4738c883be983cd4d) )
	ROM_LOAD16_BYTE( "136094-0049a.31t", 0x400000, 0x80000, CRC(25ac33af) SHA1(e76af04d2e2cbf5769c1f53e2eac4f85e57003ba) )
	ROM_LOAD16_BYTE( "136094-0048a.31r", 0x400001, 0x80000, CRC(d725a27d) SHA1(9d8d332a6c1d773888c2d9ef1d7ea1e865f6dd02) )
	ROM_LOAD16_BYTE( "136094-0051a.33t", 0x500000, 0x80000, CRC(a0fc90b2) SHA1(08ac2510287e8c3e8e52995d8f741b2a58e5b37b) )
	ROM_LOAD16_BYTE( "136094-0050a.33r", 0x500001, 0x80000, CRC(dcc206cf) SHA1(f0b7d5d289f4c0de99b536dd2874ffdabf648cb2) )
	ROM_LOAD16_BYTE( "136094-0053a.35t", 0x600000, 0x80000, CRC(74320763) SHA1(9cbf61c51dd96dc3e4a4227f3080766b9482a16a) )
	ROM_LOAD16_BYTE( "136094-0052a.35r", 0x600001, 0x80000, CRC(a7f9df2e) SHA1(c3e0c67081cf8f7b24350abf5a9adbb544ab44a7) )

	ROM_REGION( 0x80000, "jsa:oki1", 0 )
	ROM_LOAD( "136094-0082a.19e",  0x00000, 0x80000, CRC(fde543c4) SHA1(7d36d7f2f30d0ac40da77a36a47488d75474caaf) )

	ROM_REGION( 0x80000, "jsa:oki2", 0 )
	ROM_LOAD( "136094-0082a.19e",  0x00000, 0x80000, CRC(fde543c4) SHA1(7d36d7f2f30d0ac40da77a36a47488d75474caaf) )

	ROM_REGION( 0x0600, "proms", 0 )    /* microcode for growth renderer */
	ROM_LOAD( "136094-001a.22s",  0x0000, 0x0200, CRC(a70ade3f) SHA1(f4a558b17767eed2683c768d1b441e75edcff967) )
	ROM_LOAD( "136094-002a.21s",  0x0200, 0x0200, CRC(f4768b4d) SHA1(a506fa5386ab0ea2851ff1f8474d4bfc66deaa70) )
	ROM_LOAD( "136094-003a.20s",  0x0400, 0x0200, CRC(22a76ad4) SHA1(ce840c283bbd3a5f19dc8d91b19d1571eff51ff4) )

	ROM_REGION( 0x10000, "clarn", 0 )   /* ADSP2105 (40 MHz) CPU code for communications / Game Link with another PCB */
	ROM_LOAD( "136094-0071a.1j",  0x00000, 0x10000, CRC(089bc0a4) SHA1(677f95aac18fecfc6067d93f488999775889be4c) )
ROM_END


#ifdef UNUSED_DEFINITION
ROM_START( motofrei )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 8*64k for 68000 code */
	ROM_LOAD32_BYTE( "136094-motoi0.23e",   0x000000, 0x020000, CRC(a2ed9656) SHA1(7473400ee26e72d8ca51dd1f84a6c3bf0c5a72e9) )
	ROM_LOAD32_BYTE( "136094-motoi1.23j",   0x000001, 0x020000, CRC(5ded2f8d) SHA1(df146f110abf3d53f1c968baac1a6fc6e1871aa0) )
	ROM_LOAD32_BYTE( "136094-motoi2.37e",   0x000002, 0x020000, CRC(7a26217f) SHA1(1271a000e2976480a3b959609a5597498886be4f) )
	ROM_LOAD32_BYTE( "136094-motoi3.37j",   0x000003, 0x020000, CRC(ff5ca6ad) SHA1(1e26db56940ce1db819d2179f4ce3962e0b5b732) )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) /* 64k for 6502 code */
	ROM_LOAD( "136094-0080a.12c", 0x00000, 0x10000, CRC(0b1e565c) SHA1(03bdeafd8cf680f76bbd1f9aba6efac27f19a93c) )

	ROM_REGION( 0x180000, "tiles", 0 )
	ROM_LOAD( "136094-0036a.2d", 0x000000, 0x80000, CRC(1b63b493) SHA1(980141fec011fa2b5cb020eeecb4784d31679dba) ) /* playfield, planes 0-1 */
	ROM_LOAD( "136094-0037a.5d", 0x080000, 0x80000, CRC(6d290056) SHA1(fa32dbe5ac5e735d700d086353461eaa2c1dee55) ) /* playfield, planes 2-3 */
	ROM_LOAD( "136094-0038a.8d", 0x100000, 0x80000, CRC(38197c88) SHA1(dc5d4d878759503b8500e8e3a032f499bfeedcb1) ) /* playfield, planes 4-5 */

	ROM_REGION( 0x020000, "chars", 0 )
	ROM_LOAD( "136094-0025a.13n", 0x000000, 0x20000, CRC(6ab762ad) SHA1(c52dd207ff5adaffa458e020e7d452a1d1e65194) ) /* alphanumerics */

	ROM_REGION16_BE( 0x700000, "rle", 0 )
	ROM_LOAD16_BYTE( "136094-0041a.31n", 0x000000, 0x80000, CRC(474770e9) SHA1(507dac654d1c350ab530892e3ec19793629d3a07) )
	ROM_LOAD16_BYTE( "136094-0040a.31l", 0x000001, 0x80000, CRC(cb777468) SHA1(aa199bc02ab966b9f270057857aec50add8d684c) )
	ROM_LOAD16_BYTE( "136094-0043a.33n", 0x100000, 0x80000, CRC(353d2dc3) SHA1(82c2c862404ea4c94c9baee1d0ac32696fcf78bd) )
	ROM_LOAD16_BYTE( "136094-0042a.33l", 0x100001, 0x80000, CRC(17d49f77) SHA1(7ed85035128f3c9dc2bc3d9f387dba01a882e3f4) )
	ROM_LOAD16_BYTE( "136094-0045a.35n", 0x200000, 0x80000, CRC(13d89355) SHA1(becd43a51c643732025386d7781dcf784b20d031) )
	ROM_LOAD16_BYTE( "136094-0044a.35l", 0x200001, 0x80000, CRC(924c817e) SHA1(2c23fd8a85833875cfb2d60e1072bd59cea57e0c) )
	ROM_LOAD16_BYTE( "136094-0047a.37n", 0x300000, 0x80000, CRC(43ee7453) SHA1(3549f62e6b2f50d663fbb4068cb8907b5073ef5e) )
	ROM_LOAD16_BYTE( "136094-0046a.37l", 0x300001, 0x80000, CRC(980b9b92) SHA1(6a5cfb77b65c0a28dba705d4738c883be983cd4d) )
	ROM_LOAD16_BYTE( "136094-0049a.31t", 0x400000, 0x80000, CRC(25ac33af) SHA1(e76af04d2e2cbf5769c1f53e2eac4f85e57003ba) )
	ROM_LOAD16_BYTE( "136094-0048a.31r", 0x400001, 0x80000, CRC(d725a27d) SHA1(9d8d332a6c1d773888c2d9ef1d7ea1e865f6dd02) )
	ROM_LOAD16_BYTE( "136094-0051a.33t", 0x500000, 0x80000, CRC(a0fc90b2) SHA1(08ac2510287e8c3e8e52995d8f741b2a58e5b37b) )
	ROM_LOAD16_BYTE( "136094-0050a.33r", 0x500001, 0x80000, CRC(dcc206cf) SHA1(f0b7d5d289f4c0de99b536dd2874ffdabf648cb2) )
	ROM_LOAD16_BYTE( "136094-0053a.35t", 0x600000, 0x80000, CRC(74320763) SHA1(9cbf61c51dd96dc3e4a4227f3080766b9482a16a) )
	ROM_LOAD16_BYTE( "136094-0052a.35r", 0x600001, 0x80000, CRC(a7f9df2e) SHA1(c3e0c67081cf8f7b24350abf5a9adbb544ab44a7) )

	ROM_REGION( 0x80000, "jsa:oki1", 0 )
	ROM_LOAD( "136094-0082a.19e",  0x00000, 0x80000, CRC(fde543c4) SHA1(7d36d7f2f30d0ac40da77a36a47488d75474caaf) )

	ROM_REGION( 0x80000, "jsa:oki2", 0 )
	ROM_LOAD( "136094-0082a.19e",  0x00000, 0x80000, CRC(fde543c4) SHA1(7d36d7f2f30d0ac40da77a36a47488d75474caaf) )

	ROM_REGION( 0x0600, "proms", 0 )    /* microcode for growth renderer */
	ROM_LOAD( "136094-001a.22s",  0x0000, 0x0200, CRC(a70ade3f) SHA1(f4a558b17767eed2683c768d1b441e75edcff967) )
	ROM_LOAD( "136094-002a.21s",  0x0200, 0x0200, CRC(f4768b4d) SHA1(a506fa5386ab0ea2851ff1f8474d4bfc66deaa70) )
	ROM_LOAD( "136094-003a.20s",  0x0400, 0x0200, CRC(22a76ad4) SHA1(ce840c283bbd3a5f19dc8d91b19d1571eff51ff4) )
ROM_END
#endif

#ifdef UNUSED_DEFINITION
ROM_START( motofreg )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 8*64k for 68000 code */
	ROM_LOAD32_BYTE( "136094-motog0.23e",   0x000000, 0x020000, CRC(1b205eed) SHA1(e5bcabd6b0b8e7f06e9be0f7f66c86d09840d876) )
	ROM_LOAD32_BYTE( "136094-motog1.23j",   0x000001, 0x020000, CRC(f28e6634) SHA1(2d5d151cbbebdb8691b01398e0be6a08b2bc65ac) )
	ROM_LOAD32_BYTE( "136094-motog2.37e",   0x000002, 0x020000, CRC(01400d54) SHA1(cd539497465857a804b5bc228bb0c93afd1e684e) )
	ROM_LOAD32_BYTE( "136094-motog3.37j",   0x000003, 0x020000, CRC(c467c136) SHA1(9407bdf65ee6261e30227e6b87e2a35da8ee124e) )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) /* 64k for 6502 code */
	ROM_LOAD( "136094-0080a.12c", 0x00000, 0x10000, CRC(0b1e565c) SHA1(03bdeafd8cf680f76bbd1f9aba6efac27f19a93c) )

	ROM_REGION( 0x180000, "tiles", 0 )
	ROM_LOAD( "136094-0036a.2d", 0x000000, 0x80000, CRC(1b63b493) SHA1(980141fec011fa2b5cb020eeecb4784d31679dba) ) /* playfield, planes 0-1 */
	ROM_LOAD( "136094-0037a.5d", 0x080000, 0x80000, CRC(6d290056) SHA1(fa32dbe5ac5e735d700d086353461eaa2c1dee55) ) /* playfield, planes 2-3 */
	ROM_LOAD( "136094-0038a.8d", 0x100000, 0x80000, CRC(38197c88) SHA1(dc5d4d878759503b8500e8e3a032f499bfeedcb1) ) /* playfield, planes 4-5 */

	ROM_REGION( 0x020000, "chars", 0 )
	ROM_LOAD( "136094-0025a.13n", 0x000000, 0x20000, CRC(6ab762ad) SHA1(c52dd207ff5adaffa458e020e7d452a1d1e65194) ) /* alphanumerics */

	ROM_REGION16_BE( 0x700000, "rle", 0 )
	ROM_LOAD16_BYTE( "136094-0041a.31n", 0x000000, 0x80000, CRC(474770e9) SHA1(507dac654d1c350ab530892e3ec19793629d3a07) )
	ROM_LOAD16_BYTE( "136094-0040a.31l", 0x000001, 0x80000, CRC(cb777468) SHA1(aa199bc02ab966b9f270057857aec50add8d684c) )
	ROM_LOAD16_BYTE( "136094-0043a.33n", 0x100000, 0x80000, CRC(353d2dc3) SHA1(82c2c862404ea4c94c9baee1d0ac32696fcf78bd) )
	ROM_LOAD16_BYTE( "136094-0042a.33l", 0x100001, 0x80000, CRC(17d49f77) SHA1(7ed85035128f3c9dc2bc3d9f387dba01a882e3f4) )
	ROM_LOAD16_BYTE( "136094-0045a.35n", 0x200000, 0x80000, CRC(13d89355) SHA1(becd43a51c643732025386d7781dcf784b20d031) )
	ROM_LOAD16_BYTE( "136094-0044a.35l", 0x200001, 0x80000, CRC(924c817e) SHA1(2c23fd8a85833875cfb2d60e1072bd59cea57e0c) )
	ROM_LOAD16_BYTE( "136094-0047a.37n", 0x300000, 0x80000, CRC(43ee7453) SHA1(3549f62e6b2f50d663fbb4068cb8907b5073ef5e) )
	ROM_LOAD16_BYTE( "136094-0046a.37l", 0x300001, 0x80000, CRC(980b9b92) SHA1(6a5cfb77b65c0a28dba705d4738c883be983cd4d) )
	ROM_LOAD16_BYTE( "136094-0049a.31t", 0x400000, 0x80000, CRC(25ac33af) SHA1(e76af04d2e2cbf5769c1f53e2eac4f85e57003ba) )
	ROM_LOAD16_BYTE( "136094-0048a.31r", 0x400001, 0x80000, CRC(d725a27d) SHA1(9d8d332a6c1d773888c2d9ef1d7ea1e865f6dd02) )
	ROM_LOAD16_BYTE( "136094-0051a.33t", 0x500000, 0x80000, CRC(a0fc90b2) SHA1(08ac2510287e8c3e8e52995d8f741b2a58e5b37b) )
	ROM_LOAD16_BYTE( "136094-0050a.33r", 0x500001, 0x80000, CRC(dcc206cf) SHA1(f0b7d5d289f4c0de99b536dd2874ffdabf648cb2) )
	ROM_LOAD16_BYTE( "136094-0053a.35t", 0x600000, 0x80000, CRC(74320763) SHA1(9cbf61c51dd96dc3e4a4227f3080766b9482a16a) )
	ROM_LOAD16_BYTE( "136094-0052a.35r", 0x600001, 0x80000, CRC(a7f9df2e) SHA1(c3e0c67081cf8f7b24350abf5a9adbb544ab44a7) )

	ROM_REGION( 0x80000, "jsa:oki1", 0 )
	ROM_LOAD( "136094-0082a.19e",  0x00000, 0x80000, CRC(fde543c4) SHA1(7d36d7f2f30d0ac40da77a36a47488d75474caaf) )

	ROM_REGION( 0x80000, "jsa:oki2", 0 )
	ROM_LOAD( "136094-0082a.19e",  0x00000, 0x80000, CRC(fde543c4) SHA1(7d36d7f2f30d0ac40da77a36a47488d75474caaf) )

	ROM_REGION( 0x0600, "proms", 0 )    /* microcode for growth renderer */
	ROM_LOAD( "136094-001a.22s",  0x0000, 0x0200, CRC(a70ade3f) SHA1(f4a558b17767eed2683c768d1b441e75edcff967) )
	ROM_LOAD( "136094-002a.21s",  0x0200, 0x0200, CRC(f4768b4d) SHA1(a506fa5386ab0ea2851ff1f8474d4bfc66deaa70) )
	ROM_LOAD( "136094-003a.20s",  0x0400, 0x0200, CRC(22a76ad4) SHA1(ce840c283bbd3a5f19dc8d91b19d1571eff51ff4) )
ROM_END
#endif

#ifdef UNUSED_DEFINITION
ROM_START( motofmdg )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 8*64k for 68000 code */
	ROM_LOAD32_BYTE( "136094-mdg0.23e", 0x000000, 0x020000, CRC(5839b940) SHA1(0b331869e938a7e344d4a514a8b70d26f5d1fc14) )
	ROM_LOAD32_BYTE( "136094-mdg1.23j", 0x000001, 0x020000, CRC(c46a4104) SHA1(76543fefff535938f11ba7b68e97a786d35f5b82) )
	ROM_LOAD32_BYTE( "136094-mdg2.37e", 0x000002, 0x020000, CRC(0b8bfe6e) SHA1(7220032a07928fd8a887c63ffcab4ec526733cae) )
	ROM_LOAD32_BYTE( "136094-mdg3.37j", 0x000003, 0x020000, CRC(1dcd0d09) SHA1(0f6801694498688ed94588ac4b828ac56f3a16ec) )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) /* 64k for 6502 code */
	ROM_LOAD( "136094-0080a.12c", 0x00000, 0x10000, CRC(0b1e565c) SHA1(03bdeafd8cf680f76bbd1f9aba6efac27f19a93c) )

	ROM_REGION( 0x180000, "tiles", 0 )
	ROM_LOAD( "136094-0036a.2d", 0x000000, 0x80000, CRC(1b63b493) SHA1(980141fec011fa2b5cb020eeecb4784d31679dba) ) /* playfield, planes 0-1 */
	ROM_LOAD( "136094-0037a.5d", 0x080000, 0x80000, CRC(6d290056) SHA1(fa32dbe5ac5e735d700d086353461eaa2c1dee55) ) /* playfield, planes 2-3 */
	ROM_LOAD( "136094-0038a.8d", 0x100000, 0x80000, CRC(38197c88) SHA1(dc5d4d878759503b8500e8e3a032f499bfeedcb1) ) /* playfield, planes 4-5 */

	ROM_REGION( 0x020000, "chars", 0 )
	ROM_LOAD( "136094-0025a.13n", 0x000000, 0x20000, CRC(6ab762ad) SHA1(c52dd207ff5adaffa458e020e7d452a1d1e65194) ) /* alphanumerics */

	ROM_REGION16_BE( 0x700000, "rle", 0 )
	ROM_LOAD16_BYTE( "136094-0041a.31n", 0x000000, 0x80000, CRC(474770e9) SHA1(507dac654d1c350ab530892e3ec19793629d3a07) )
	ROM_LOAD16_BYTE( "136094-0040a.31l", 0x000001, 0x80000, CRC(cb777468) SHA1(aa199bc02ab966b9f270057857aec50add8d684c) )
	ROM_LOAD16_BYTE( "136094-0043a.33n", 0x100000, 0x80000, CRC(353d2dc3) SHA1(82c2c862404ea4c94c9baee1d0ac32696fcf78bd) )
	ROM_LOAD16_BYTE( "136094-0042a.33l", 0x100001, 0x80000, CRC(17d49f77) SHA1(7ed85035128f3c9dc2bc3d9f387dba01a882e3f4) )
	ROM_LOAD16_BYTE( "136094-0045a.35n", 0x200000, 0x80000, CRC(13d89355) SHA1(becd43a51c643732025386d7781dcf784b20d031) )
	ROM_LOAD16_BYTE( "136094-0044a.35l", 0x200001, 0x80000, CRC(924c817e) SHA1(2c23fd8a85833875cfb2d60e1072bd59cea57e0c) )
	ROM_LOAD16_BYTE( "136094-0047a.37n", 0x300000, 0x80000, CRC(43ee7453) SHA1(3549f62e6b2f50d663fbb4068cb8907b5073ef5e) )
	ROM_LOAD16_BYTE( "136094-0046a.37l", 0x300001, 0x80000, CRC(980b9b92) SHA1(6a5cfb77b65c0a28dba705d4738c883be983cd4d) )
	ROM_LOAD16_BYTE( "136094-0049a.31t", 0x400000, 0x80000, CRC(25ac33af) SHA1(e76af04d2e2cbf5769c1f53e2eac4f85e57003ba) )
	ROM_LOAD16_BYTE( "136094-0048a.31r", 0x400001, 0x80000, CRC(d725a27d) SHA1(9d8d332a6c1d773888c2d9ef1d7ea1e865f6dd02) )
	ROM_LOAD16_BYTE( "136094-0051a.33t", 0x500000, 0x80000, CRC(a0fc90b2) SHA1(08ac2510287e8c3e8e52995d8f741b2a58e5b37b) )
	ROM_LOAD16_BYTE( "136094-0050a.33r", 0x500001, 0x80000, CRC(dcc206cf) SHA1(f0b7d5d289f4c0de99b536dd2874ffdabf648cb2) )
	ROM_LOAD16_BYTE( "136094-0053a.35t", 0x600000, 0x80000, CRC(74320763) SHA1(9cbf61c51dd96dc3e4a4227f3080766b9482a16a) )
	ROM_LOAD16_BYTE( "136094-0052a.35r", 0x600001, 0x80000, CRC(a7f9df2e) SHA1(c3e0c67081cf8f7b24350abf5a9adbb544ab44a7) )

	ROM_REGION( 0x80000, "jsa:oki1", 0 )
	ROM_LOAD( "136094-0082a.19e",  0x00000, 0x80000, CRC(fde543c4) SHA1(7d36d7f2f30d0ac40da77a36a47488d75474caaf) )

	ROM_REGION( 0x80000, "jsa:oki2", 0 )
	ROM_LOAD( "136094-0082a.19e",  0x00000, 0x80000, CRC(fde543c4) SHA1(7d36d7f2f30d0ac40da77a36a47488d75474caaf) )

	ROM_REGION( 0x0600, "proms", 0 )    /* microcode for growth renderer */
	ROM_LOAD( "136094-001a.22s",  0x0000, 0x0200, CRC(a70ade3f) SHA1(f4a558b17767eed2683c768d1b441e75edcff967) )
	ROM_LOAD( "136094-002a.21s",  0x0200, 0x0200, CRC(f4768b4d) SHA1(a506fa5386ab0ea2851ff1f8474d4bfc66deaa70) )
	ROM_LOAD( "136094-003a.20s",  0x0400, 0x0200, CRC(22a76ad4) SHA1(ce840c283bbd3a5f19dc8d91b19d1571eff51ff4) )
ROM_END
#endif

ROM_START( motofrenft )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 8*64k for 68000 code */
	ROM_LOAD32_BYTE( "136094-ft0.23e", 0x000000, 0x020000, CRC(99158754) SHA1(46e73a465aceb147e1ca1bd982448079880cc47e) )
	ROM_LOAD32_BYTE( "136094-ft1.23j", 0x000001, 0x020000, CRC(33c4e205) SHA1(8a223481cfe2aa45a815c6a18017a14502e929b3) )
	ROM_LOAD32_BYTE( "136094-ft2.37e", 0x000002, 0x020000, CRC(30eb94bb) SHA1(b7a2b41570d2110aaedea8a3b9d120af31671bbd) )
	ROM_LOAD32_BYTE( "136094-ft3.37j", 0x000003, 0x020000, CRC(a92e05e3) SHA1(354b6bbb058d10c4da55cb58bf05eae83350ba08) )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) /* 64k for 6502 code */
	ROM_LOAD( "136094-0080a.12c", 0x00000, 0x10000, CRC(0b1e565c) SHA1(03bdeafd8cf680f76bbd1f9aba6efac27f19a93c) )

	ROM_REGION( 0x180000, "tiles", 0 )
	ROM_LOAD( "136094-0036a.2d", 0x000000, 0x80000, CRC(1b63b493) SHA1(980141fec011fa2b5cb020eeecb4784d31679dba) ) /* playfield, planes 0-1 */
	ROM_LOAD( "136094-0037a.5d", 0x080000, 0x80000, CRC(6d290056) SHA1(fa32dbe5ac5e735d700d086353461eaa2c1dee55) ) /* playfield, planes 2-3 */
	ROM_LOAD( "136094-0038a.8d", 0x100000, 0x80000, CRC(38197c88) SHA1(dc5d4d878759503b8500e8e3a032f499bfeedcb1) ) /* playfield, planes 4-5 */

	ROM_REGION( 0x020000, "chars", 0 )
	ROM_LOAD( "136094-0025a.13n", 0x000000, 0x20000, CRC(6ab762ad) SHA1(c52dd207ff5adaffa458e020e7d452a1d1e65194) ) /* alphanumerics */

	ROM_REGION16_BE( 0x700000, "rle", 0 )
	ROM_LOAD16_BYTE( "136094-0041a.31n", 0x000000, 0x80000, CRC(474770e9) SHA1(507dac654d1c350ab530892e3ec19793629d3a07) )
	ROM_LOAD16_BYTE( "136094-0040a.31l", 0x000001, 0x80000, CRC(cb777468) SHA1(aa199bc02ab966b9f270057857aec50add8d684c) )
	ROM_LOAD16_BYTE( "136094-0043a.33n", 0x100000, 0x80000, CRC(353d2dc3) SHA1(82c2c862404ea4c94c9baee1d0ac32696fcf78bd) )
	ROM_LOAD16_BYTE( "136094-0042a.33l", 0x100001, 0x80000, CRC(17d49f77) SHA1(7ed85035128f3c9dc2bc3d9f387dba01a882e3f4) )
	ROM_LOAD16_BYTE( "136094-0045a.35n", 0x200000, 0x80000, CRC(13d89355) SHA1(becd43a51c643732025386d7781dcf784b20d031) )
	ROM_LOAD16_BYTE( "136094-0044a.35l", 0x200001, 0x80000, CRC(924c817e) SHA1(2c23fd8a85833875cfb2d60e1072bd59cea57e0c) )
	ROM_LOAD16_BYTE( "136094-0047a.37n", 0x300000, 0x80000, CRC(43ee7453) SHA1(3549f62e6b2f50d663fbb4068cb8907b5073ef5e) )
	ROM_LOAD16_BYTE( "136094-0046a.37l", 0x300001, 0x80000, CRC(980b9b92) SHA1(6a5cfb77b65c0a28dba705d4738c883be983cd4d) )
	ROM_LOAD16_BYTE( "136094-0049a.31t", 0x400000, 0x80000, CRC(25ac33af) SHA1(e76af04d2e2cbf5769c1f53e2eac4f85e57003ba) )
	ROM_LOAD16_BYTE( "136094-0048a.31r", 0x400001, 0x80000, CRC(d725a27d) SHA1(9d8d332a6c1d773888c2d9ef1d7ea1e865f6dd02) )
	ROM_LOAD16_BYTE( "136094-0051a.33t", 0x500000, 0x80000, CRC(a0fc90b2) SHA1(08ac2510287e8c3e8e52995d8f741b2a58e5b37b) )
	ROM_LOAD16_BYTE( "136094-0050a.33r", 0x500001, 0x80000, CRC(dcc206cf) SHA1(f0b7d5d289f4c0de99b536dd2874ffdabf648cb2) )
	ROM_LOAD16_BYTE( "136094-0053a.35t", 0x600000, 0x80000, CRC(74320763) SHA1(9cbf61c51dd96dc3e4a4227f3080766b9482a16a) )
	ROM_LOAD16_BYTE( "136094-0052a.35r", 0x600001, 0x80000, CRC(a7f9df2e) SHA1(c3e0c67081cf8f7b24350abf5a9adbb544ab44a7) )

	ROM_REGION( 0x80000, "jsa:oki1", 0 )
	ROM_LOAD( "136094-0082a.19e",  0x00000, 0x80000, CRC(fde543c4) SHA1(7d36d7f2f30d0ac40da77a36a47488d75474caaf) )

	ROM_REGION( 0x80000, "jsa:oki2", 0 )
	ROM_LOAD( "136094-0082a.19e",  0x00000, 0x80000, CRC(fde543c4) SHA1(7d36d7f2f30d0ac40da77a36a47488d75474caaf) )

	ROM_REGION( 0x0600, "proms", 0 )    /* microcode for growth renderer */
	ROM_LOAD( "136094-001a.22s",  0x0000, 0x0200, CRC(a70ade3f) SHA1(f4a558b17767eed2683c768d1b441e75edcff967) )
	ROM_LOAD( "136094-002a.21s",  0x0200, 0x0200, CRC(f4768b4d) SHA1(a506fa5386ab0ea2851ff1f8474d4bfc66deaa70) )
	ROM_LOAD( "136094-003a.20s",  0x0400, 0x0200, CRC(22a76ad4) SHA1(ce840c283bbd3a5f19dc8d91b19d1571eff51ff4) )
ROM_END


ROM_START( motofrenfta )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 8*64k for 68000 code */
	ROM_LOAD32_BYTE( "136094-0021d.23e", 0x000000, 0x020000, CRC(9f2c4280) SHA1(1952dbb76d1dca25b381ce86041fad6b3e212133) )
	ROM_LOAD32_BYTE( "136094-0022d.23j", 0x000001, 0x020000, CRC(efaee63a) SHA1(2c37655394a4f32b93777fdb38684a6cf8188da9) )
	ROM_LOAD32_BYTE( "136094-0023d.37e", 0x000002, 0x020000, CRC(e966c3b5) SHA1(fe76a3705b5f2feb1536a2fc9fd55ac23c692923) )
	ROM_LOAD32_BYTE( "136094-0024d.37j", 0x000003, 0x020000, CRC(29796ead) SHA1(535514eb25d79bc533a06f862afd62a6689228f9) )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) /* 64k for 6502 code */
	ROM_LOAD( "136094-0080a.12c", 0x00000, 0x10000, CRC(0b1e565c) SHA1(03bdeafd8cf680f76bbd1f9aba6efac27f19a93c) )

	ROM_REGION( 0x180000, "tiles", 0 )
	ROM_LOAD( "136094-0036a.2d", 0x000000, 0x80000, CRC(1b63b493) SHA1(980141fec011fa2b5cb020eeecb4784d31679dba) ) /* playfield, planes 0-1 */
	ROM_LOAD( "136094-0037a.5d", 0x080000, 0x80000, CRC(6d290056) SHA1(fa32dbe5ac5e735d700d086353461eaa2c1dee55) ) /* playfield, planes 2-3 */
	ROM_LOAD( "136094-0038a.8d", 0x100000, 0x80000, CRC(38197c88) SHA1(dc5d4d878759503b8500e8e3a032f499bfeedcb1) ) /* playfield, planes 4-5 */

	ROM_REGION( 0x020000, "chars", 0 )
	ROM_LOAD( "136094-0025a.13n", 0x000000, 0x20000, CRC(6ab762ad) SHA1(c52dd207ff5adaffa458e020e7d452a1d1e65194) ) /* alphanumerics */

	ROM_REGION16_BE( 0x700000, "rle", 0 )
	ROM_LOAD16_BYTE( "136094-0041a.31n", 0x000000, 0x80000, CRC(474770e9) SHA1(507dac654d1c350ab530892e3ec19793629d3a07) )
	ROM_LOAD16_BYTE( "136094-0040a.31l", 0x000001, 0x80000, CRC(cb777468) SHA1(aa199bc02ab966b9f270057857aec50add8d684c) )
	ROM_LOAD16_BYTE( "136094-0043a.33n", 0x100000, 0x80000, CRC(353d2dc3) SHA1(82c2c862404ea4c94c9baee1d0ac32696fcf78bd) )
	ROM_LOAD16_BYTE( "136094-0042a.33l", 0x100001, 0x80000, CRC(17d49f77) SHA1(7ed85035128f3c9dc2bc3d9f387dba01a882e3f4) )
	ROM_LOAD16_BYTE( "136094-0045a.35n", 0x200000, 0x80000, CRC(13d89355) SHA1(becd43a51c643732025386d7781dcf784b20d031) )
	ROM_LOAD16_BYTE( "136094-0044a.35l", 0x200001, 0x80000, CRC(924c817e) SHA1(2c23fd8a85833875cfb2d60e1072bd59cea57e0c) )
	ROM_LOAD16_BYTE( "136094-0047a.37n", 0x300000, 0x80000, CRC(43ee7453) SHA1(3549f62e6b2f50d663fbb4068cb8907b5073ef5e) )
	ROM_LOAD16_BYTE( "136094-0046a.37l", 0x300001, 0x80000, CRC(980b9b92) SHA1(6a5cfb77b65c0a28dba705d4738c883be983cd4d) )
	ROM_LOAD16_BYTE( "136094-0049a.31t", 0x400000, 0x80000, CRC(25ac33af) SHA1(e76af04d2e2cbf5769c1f53e2eac4f85e57003ba) )
	ROM_LOAD16_BYTE( "136094-0048a.31r", 0x400001, 0x80000, CRC(d725a27d) SHA1(9d8d332a6c1d773888c2d9ef1d7ea1e865f6dd02) )
	ROM_LOAD16_BYTE( "136094-0051a.33t", 0x500000, 0x80000, CRC(a0fc90b2) SHA1(08ac2510287e8c3e8e52995d8f741b2a58e5b37b) )
	ROM_LOAD16_BYTE( "136094-0050a.33r", 0x500001, 0x80000, CRC(dcc206cf) SHA1(f0b7d5d289f4c0de99b536dd2874ffdabf648cb2) )
	ROM_LOAD16_BYTE( "136094-0053a.35t", 0x600000, 0x80000, CRC(74320763) SHA1(9cbf61c51dd96dc3e4a4227f3080766b9482a16a) )
	ROM_LOAD16_BYTE( "136094-0052a.35r", 0x600001, 0x80000, CRC(a7f9df2e) SHA1(c3e0c67081cf8f7b24350abf5a9adbb544ab44a7) )

	ROM_REGION( 0x80000, "jsa:oki1", 0 )
	ROM_LOAD( "136094-0082a.19e",  0x00000, 0x80000, CRC(fde543c4) SHA1(7d36d7f2f30d0ac40da77a36a47488d75474caaf) )

	ROM_REGION( 0x80000, "jsa:oki2", 0 )
	ROM_LOAD( "136094-0082a.19e",  0x00000, 0x80000, CRC(fde543c4) SHA1(7d36d7f2f30d0ac40da77a36a47488d75474caaf) )

	ROM_REGION( 0x0600, "proms", 0 )    /* microcode for growth renderer */
	ROM_LOAD( "136094-001a.22s",  0x0000, 0x0200, CRC(a70ade3f) SHA1(f4a558b17767eed2683c768d1b441e75edcff967) )
	ROM_LOAD( "136094-002a.21s",  0x0200, 0x0200, CRC(f4768b4d) SHA1(a506fa5386ab0ea2851ff1f8474d4bfc66deaa70) )
	ROM_LOAD( "136094-003a.20s",  0x0400, 0x0200, CRC(22a76ad4) SHA1(ce840c283bbd3a5f19dc8d91b19d1571eff51ff4) )
ROM_END


ROM_START( motofrenmf )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 8*64k for 68000 code */
	ROM_LOAD32_BYTE( "136094-ftmd0.23e", 0x000000, 0x020000, CRC(9be0803e) SHA1(b5e3029ef43adfeafd5979d4ee49a3eb62efd629) )
	ROM_LOAD32_BYTE( "136094-ftmd1.23j", 0x000001, 0x020000, CRC(2a5e9b18) SHA1(af671680047678f86614e23439ac1f0420528343) )
	ROM_LOAD32_BYTE( "136094-ftmd2.37e", 0x000002, 0x020000, CRC(769223fc) SHA1(acfafae3d81a6a3a4ff82c6381590ac31ad80f23) )
	ROM_LOAD32_BYTE( "136094-ftmd3.37j", 0x000003, 0x020000, CRC(96382cc0) SHA1(ba2b6b105c552077767d1185886761fce3ec2885) )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) /* 64k for 6502 code */
	ROM_LOAD( "136094-0080a.12c", 0x00000, 0x10000, CRC(0b1e565c) SHA1(03bdeafd8cf680f76bbd1f9aba6efac27f19a93c) )

	ROM_REGION( 0x180000, "tiles", 0 )
	ROM_LOAD( "136094-0036a.2d", 0x000000, 0x80000, CRC(1b63b493) SHA1(980141fec011fa2b5cb020eeecb4784d31679dba) ) /* playfield, planes 0-1 */
	ROM_LOAD( "136094-0037a.5d", 0x080000, 0x80000, CRC(6d290056) SHA1(fa32dbe5ac5e735d700d086353461eaa2c1dee55) ) /* playfield, planes 2-3 */
	ROM_LOAD( "136094-0038a.8d", 0x100000, 0x80000, CRC(38197c88) SHA1(dc5d4d878759503b8500e8e3a032f499bfeedcb1) ) /* playfield, planes 4-5 */

	ROM_REGION( 0x020000, "chars", 0 )
	ROM_LOAD( "136094-0025a.13n", 0x000000, 0x20000, CRC(6ab762ad) SHA1(c52dd207ff5adaffa458e020e7d452a1d1e65194) ) /* alphanumerics */

	ROM_REGION16_BE( 0x700000, "rle", 0 )
	ROM_LOAD16_BYTE( "136094-0041a.31n", 0x000000, 0x80000, CRC(474770e9) SHA1(507dac654d1c350ab530892e3ec19793629d3a07) )
	ROM_LOAD16_BYTE( "136094-0040a.31l", 0x000001, 0x80000, CRC(cb777468) SHA1(aa199bc02ab966b9f270057857aec50add8d684c) )
	ROM_LOAD16_BYTE( "136094-0043a.33n", 0x100000, 0x80000, CRC(353d2dc3) SHA1(82c2c862404ea4c94c9baee1d0ac32696fcf78bd) )
	ROM_LOAD16_BYTE( "136094-0042a.33l", 0x100001, 0x80000, CRC(17d49f77) SHA1(7ed85035128f3c9dc2bc3d9f387dba01a882e3f4) )
	ROM_LOAD16_BYTE( "136094-0045a.35n", 0x200000, 0x80000, CRC(13d89355) SHA1(becd43a51c643732025386d7781dcf784b20d031) )
	ROM_LOAD16_BYTE( "136094-0044a.35l", 0x200001, 0x80000, CRC(924c817e) SHA1(2c23fd8a85833875cfb2d60e1072bd59cea57e0c) )
	ROM_LOAD16_BYTE( "136094-0047a.37n", 0x300000, 0x80000, CRC(43ee7453) SHA1(3549f62e6b2f50d663fbb4068cb8907b5073ef5e) )
	ROM_LOAD16_BYTE( "136094-0046a.37l", 0x300001, 0x80000, CRC(980b9b92) SHA1(6a5cfb77b65c0a28dba705d4738c883be983cd4d) )
	ROM_LOAD16_BYTE( "136094-0049a.31t", 0x400000, 0x80000, CRC(25ac33af) SHA1(e76af04d2e2cbf5769c1f53e2eac4f85e57003ba) )
	ROM_LOAD16_BYTE( "136094-0048a.31r", 0x400001, 0x80000, CRC(d725a27d) SHA1(9d8d332a6c1d773888c2d9ef1d7ea1e865f6dd02) )
	ROM_LOAD16_BYTE( "136094-0051a.33t", 0x500000, 0x80000, CRC(a0fc90b2) SHA1(08ac2510287e8c3e8e52995d8f741b2a58e5b37b) )
	ROM_LOAD16_BYTE( "136094-0050a.33r", 0x500001, 0x80000, CRC(dcc206cf) SHA1(f0b7d5d289f4c0de99b536dd2874ffdabf648cb2) )
	ROM_LOAD16_BYTE( "136094-0053a.35t", 0x600000, 0x80000, CRC(74320763) SHA1(9cbf61c51dd96dc3e4a4227f3080766b9482a16a) )
	ROM_LOAD16_BYTE( "136094-0052a.35r", 0x600001, 0x80000, CRC(a7f9df2e) SHA1(c3e0c67081cf8f7b24350abf5a9adbb544ab44a7) )

	ROM_REGION( 0x80000, "jsa:oki1", 0 )
	ROM_LOAD( "136094-0082a.19e",  0x00000, 0x80000, CRC(fde543c4) SHA1(7d36d7f2f30d0ac40da77a36a47488d75474caaf) )

	ROM_REGION( 0x80000, "jsa:oki2", 0 )
	ROM_LOAD( "136094-0082a.19e",  0x00000, 0x80000, CRC(fde543c4) SHA1(7d36d7f2f30d0ac40da77a36a47488d75474caaf) )

	ROM_REGION( 0x0600, "proms", 0 )    /* microcode for growth renderer */
	ROM_LOAD( "136094-001a.22s",  0x0000, 0x0200, CRC(a70ade3f) SHA1(f4a558b17767eed2683c768d1b441e75edcff967) )
	ROM_LOAD( "136094-002a.21s",  0x0200, 0x0200, CRC(f4768b4d) SHA1(a506fa5386ab0ea2851ff1f8474d4bfc66deaa70) )
	ROM_LOAD( "136094-003a.20s",  0x0400, 0x0200, CRC(22a76ad4) SHA1(ce840c283bbd3a5f19dc8d91b19d1571eff51ff4) )
ROM_END


ROM_START( rrreveng )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 8*64k for 68000 code */
	ROM_LOAD32_BYTE( "revenge.23e", 0x00000, 0x20000, CRC(3ade13a6) SHA1(672dd0800d6a1cf6cbb2adcebe452a0df71b3236) ) /* Test menu shows 06SEP1994 14:25:13 */
	ROM_LOAD32_BYTE( "revenge.23j", 0x00001, 0x20000, CRC(aff623d5) SHA1(3ad419deb2f40d62f5a6803035c5d08fe82833f4) )
	ROM_LOAD32_BYTE( "revenge.37e", 0x00002, 0x20000, CRC(b5e2a3e2) SHA1(b6ad6d03120ad6699af31d09474b82979ead65bb) )
	ROM_LOAD32_BYTE( "revenge.37j", 0x00003, 0x20000, CRC(6c7f114b) SHA1(2b9a627ec0a211da8080ea33a5486367b043952a) )

	ROM_REGION( 0x180000, "tiles", 0 )
	ROM_LOAD( "rralpl.2d", 0x000000, 0x80000, CRC(00488dad) SHA1(604f08a219db0438dcbf21337ebd497f353bd812) ) /* playfield, planes 0-1 */
	ROM_LOAD( "rralpm.5d", 0x080000, 0x80000, CRC(ade27447) SHA1(641fdca97a4b08251e111425d8467e4640433df7) ) /* playfield, planes 2-3 */
	ROM_LOAD( "rralph.8d", 0x100000, 0x80000, CRC(ef04f04e) SHA1(e518133096978c4a0152253231625c385a84530f) ) /* playfield, planes 4-5 */

	ROM_REGION( 0x020000, "chars", 0 )
	ROM_LOAD( "alpha.13n", 0x000000, 0x20000, CRC(f2efbd66) SHA1(d5339f0b3de7a102d659f7459b5f4800cab31829) ) /* alphanumerics */

	ROM_REGION16_BE( 0x500000, "rle", 0 )
	ROM_LOAD16_BYTE( "mo0h.31n",    0x000000, 0x80000, CRC(fc2755d5) SHA1(e24319161307efbb3828b21a6250869051f1ccc1) )
	ROM_LOAD16_BYTE( "mo0l.31l",    0x000001, 0x80000, CRC(f9f6bfe3) SHA1(c7e1479bb86646691d5ca7ee9127553cfd86571e) )
	ROM_LOAD16_BYTE( "rrmo1h.33n",  0x100000, 0x80000, CRC(c7a48389) SHA1(a263aa4829cb243440ebe0496dd4f0158d97e2cc) )
	ROM_LOAD16_BYTE( "rrmo1l.33l",  0x100001, 0x80000, CRC(085a67c1) SHA1(0449abb5fec9014bd0ad14ce4802f726a90bc48c) )
	ROM_LOAD16_BYTE( "rrmo2h.35n",  0x200000, 0x80000, CRC(aea35aff) SHA1(2e02d2e877d356f9fb07e8b55ce252a148192d59) )
	ROM_LOAD16_BYTE( "rrmo2l.35l",  0x200001, 0x80000, CRC(b256d6d6) SHA1(0f0a05e3e58a00662f99988a050f2f5a11592d29) )
	ROM_LOAD16_BYTE( "mo3h.37n",    0x300000, 0x80000, CRC(563baaeb) SHA1(e356aa2048c6addefa62d10a17fe5736afa54b16) )
	ROM_LOAD16_BYTE( "mo3l.37l",    0x300001, 0x80000, CRC(e0cf3396) SHA1(544863e5c18cce002ced8f96d5dfaedffdcfff1e) )
	ROM_LOAD16_BYTE( "revenge.31t", 0x400000, 0x80000, CRC(086fb896) SHA1(5ca3aea3a52e73a1054c88759d957709c2ad22a2) )
	ROM_LOAD16_BYTE( "revenge.31r", 0x400001, 0x80000, CRC(518fdd7c) SHA1(ccee646efb178aa3720e75524646e20d18b27694) )

	ROM_REGION( 0x0600, "proms", 0 )    /* microcode for growth renderer */
	ROM_LOAD( "136094-0001a.22s",  0x0000, 0x0200, CRC(a70ade3f) SHA1(f4a558b17767eed2683c768d1b441e75edcff967) ) /* 74S472AN BPROM */
	ROM_LOAD( "136094-0002a.21s",  0x0200, 0x0200, CRC(f4768b4d) SHA1(a506fa5386ab0ea2851ff1f8474d4bfc66deaa70) ) /* 74S472AN BPROM */
	ROM_LOAD( "136094-0003a.20s",  0x0400, 0x0200, CRC(22a76ad4) SHA1(ce840c283bbd3a5f19dc8d91b19d1571eff51ff4) ) /* 74S472AN BPROM */

	ROM_REGION( 0x0600, "pals", 0 ) /* none of these have been verified as good */
	ROM_LOAD( "136094-0019b.3n",  0x0000, 0x0157, CRC(598d5009) SHA1(9804f05fbf1b9324f8c3937e0953da02870d988b) ) /* GAL20V8A */
	ROM_LOAD( "136094-0010a.5n",  0x0000, 0x0117, CRC(87ff6393) SHA1(df1f0a5450485598c0ef7fa4981cc0e40a6a5073) ) /* GAL16V8A */
	ROM_LOAD( "136094-0011b.5r",  0x0000, 0x0117, CRC(832671eb) SHA1(85232128a4b03c4e3dffb4f2e6381a89f4f9aac5) ) /* GAL16V8A */
	ROM_LOAD( "136094-0009a.7n",  0x0000, 0x0117, NO_DUMP ) /* GAL16V8A */
	ROM_LOAD( "136094-0012a.7r",  0x0000, 0x0117, NO_DUMP ) /* GAL16V8A */
	ROM_LOAD( "136094-0007a.12l", 0x0000, 0x0117, NO_DUMP ) /* GAL16V8A */
	ROM_LOAD( "136094-0006a.13r", 0x0000, 0x0117, CRC(d5c84926) SHA1(22d2821ed77ad070163e3d188b1412f8d8d52977) ) /* GAL16V8A */
	ROM_LOAD( "136094-0008a.17l", 0x0000, 0x0117, CRC(b85ab18d) SHA1(eabcc2e54c2b6bc393603a31d22418edf60593ad) ) /* GAL16V8A, hand written "ROAD 2" over label */
	ROM_LOAD( "136094-0014a.22r", 0x0000, 0x0117, CRC(9dc3831d) SHA1(553c289801eb1e15118bc045ddca226343e6a623) ) /* GAL16V8A */
	ROM_LOAD( "136094-0016a.23r", 0x0000, 0x0117, NO_DUMP ) /* GAL16V8A */
	ROM_LOAD( "136094-0015a.23s", 0x0000, 0x0117, CRC(9404e122) SHA1(fb1db0fdb10ddeb7247dd254b3e725b9ef85097b) ) /* GAL16V8A */
	ROM_LOAD( "136094-0013a.24c", 0x0000, 0x0117, NO_DUMP ) /* GAL16V8A */
	ROM_LOAD( "136094-0018a.24j", 0x0000, 0x0117, CRC(9def4158) SHA1(11c168e2c16046e1213786c065906455fdb5a63c) ) /* GAL16V8A */
	ROM_LOAD( "136094-0017a.25c", 0x0000, 0x0117, CRC(76d8fa5b) SHA1(5fcb7b75f37f918331d99422ea3f0ea202665d5e) ) /* GAL16V8A */

	/* all roms above are from this PCB however the sound board was missing - assumed to be the same */

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) /* 64k for 6502 code */
	ROM_LOAD( "rr65snd.bin", 0x00000, 0x10000, CRC(d78429da) SHA1(a4d36d74986f08c793f15f2e67cb97a8c91c5e90) )

	ROM_REGION( 0x80000, "jsa:oki1", 0 )
	ROM_LOAD( "rralpc0.bin",  0x00000, 0x80000, CRC(1f7b6ecf) SHA1(1787a2e89618e1338d70a54684dbc7d44c5f5559) )

	ROM_REGION( 0x80000, "jsa:oki2", 0 )
	ROM_LOAD( "rralpc1.bin",  0x00000, 0x80000, CRC(7ccd26d7) SHA1(1a74bdc66482896f5b9795d27383aa993e5fbaa4) )
ROM_END


ROM_START( rrrevenga ) /* Same program roms as the set below, but shares more roms with the most current version (parent) */
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 8*64k for 68000 code */
	ROM_LOAD32_BYTE( "rrprghh.23e", 0x00000, 0x20000, CRC(d2903e9d) SHA1(8782cd6ee39e2159b9ebc68ecdc3ecefcdeb8623) ) /* Test menu shows 27JAN1994 17:02:20 */
	ROM_LOAD32_BYTE( "rrprghl.23j", 0x00001, 0x20000, CRC(1afd500c) SHA1(6d24087a839e5e7d9c764026a9f3089e52785cdb) )
	ROM_LOAD32_BYTE( "rrprglh.37e", 0x00002, 0x20000, CRC(2b03a6fc) SHA1(7c95a0307b854bd37fd327ff1af1b69aa60fb2fd) )
	ROM_LOAD32_BYTE( "rrprgll.37j", 0x00003, 0x20000, CRC(acf078da) SHA1(3506e105d3b208864ce12ab20e6250cb3a0005d6) )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) /* 64k for 6502 code */
	ROM_LOAD( "rr65snd.bin", 0x00000, 0x10000, CRC(d78429da) SHA1(a4d36d74986f08c793f15f2e67cb97a8c91c5e90) )

	ROM_REGION( 0x180000, "tiles", 0 )
	ROM_LOAD( "rralpl.2d", 0x000000, 0x80000, CRC(00488dad) SHA1(604f08a219db0438dcbf21337ebd497f353bd812) ) /* playfield, planes 0-1 */
	ROM_LOAD( "rralpm.5d", 0x080000, 0x80000, CRC(ade27447) SHA1(641fdca97a4b08251e111425d8467e4640433df7) ) /* playfield, planes 2-3 */
	ROM_LOAD( "rralph.8d", 0x100000, 0x80000, CRC(ef04f04e) SHA1(e518133096978c4a0152253231625c385a84530f) ) /* playfield, planes 4-5 */

	ROM_REGION( 0x020000, "chars", 0 )
	ROM_LOAD( "alpha.13n", 0x000000, 0x20000, CRC(f2efbd66) SHA1(d5339f0b3de7a102d659f7459b5f4800cab31829) ) /* alphanumerics */

	ROM_REGION16_BE( 0x500000, "rle", 0 )
	ROM_LOAD16_BYTE( "mo0h.31n",   0x000000, 0x80000, CRC(fc2755d5) SHA1(e24319161307efbb3828b21a6250869051f1ccc1) )
	ROM_LOAD16_BYTE( "mo0l.31l",   0x000001, 0x80000, CRC(f9f6bfe3) SHA1(c7e1479bb86646691d5ca7ee9127553cfd86571e) )
	ROM_LOAD16_BYTE( "rrmo1h.33n", 0x100000, 0x80000, CRC(c7a48389) SHA1(a263aa4829cb243440ebe0496dd4f0158d97e2cc) )
	ROM_LOAD16_BYTE( "rrmo1l.33l", 0x100001, 0x80000, CRC(085a67c1) SHA1(0449abb5fec9014bd0ad14ce4802f726a90bc48c) )
	ROM_LOAD16_BYTE( "rrmo2h.35n", 0x200000, 0x80000, CRC(aea35aff) SHA1(2e02d2e877d356f9fb07e8b55ce252a148192d59) )
	ROM_LOAD16_BYTE( "rrmo2l.35l", 0x200001, 0x80000, CRC(b256d6d6) SHA1(0f0a05e3e58a00662f99988a050f2f5a11592d29) )
	ROM_LOAD16_BYTE( "mo3h.37n",   0x300000, 0x80000, CRC(563baaeb) SHA1(e356aa2048c6addefa62d10a17fe5736afa54b16) )
	ROM_LOAD16_BYTE( "mo3l.37l",   0x300001, 0x80000, CRC(e0cf3396) SHA1(544863e5c18cce002ced8f96d5dfaedffdcfff1e) )
	ROM_LOAD16_BYTE( "mo4h.31t",   0x400000, 0x80000, CRC(af6a027e) SHA1(08038bddb6aa7e97f013f9d3e508f5501821e460) )
	ROM_LOAD16_BYTE( "mo4l.31r",   0x400001, 0x80000, CRC(9ebc5369) SHA1(ffd8418b328d99aa44fb1aed1db1aa6ac715c644) )

	ROM_REGION( 0x80000, "jsa:oki1", 0 )
	ROM_LOAD( "rralpc0.bin",  0x00000, 0x80000, CRC(1f7b6ecf) SHA1(1787a2e89618e1338d70a54684dbc7d44c5f5559) )

	ROM_REGION( 0x80000, "jsa:oki2", 0 )
	ROM_LOAD( "rralpc1.bin",  0x00000, 0x80000, CRC(7ccd26d7) SHA1(1a74bdc66482896f5b9795d27383aa993e5fbaa4) )

	ROM_REGION( 0x0600, "proms", 0 )    /* microcode for growth renderer */
	ROM_LOAD( "136094-0001a.22s",  0x0000, 0x0200, CRC(a70ade3f) SHA1(f4a558b17767eed2683c768d1b441e75edcff967) ) /* 74S472AN BPROM */
	ROM_LOAD( "136094-0002a.21s",  0x0200, 0x0200, CRC(f4768b4d) SHA1(a506fa5386ab0ea2851ff1f8474d4bfc66deaa70) ) /* 74S472AN BPROM */
	ROM_LOAD( "136094-0003a.20s",  0x0400, 0x0200, CRC(22a76ad4) SHA1(ce840c283bbd3a5f19dc8d91b19d1571eff51ff4) ) /* 74S472AN BPROM */

	ROM_REGION( 0x0600, "pals", 0 ) /* none of these have been verified as good */
	ROM_LOAD( "136094-0019b.3n",  0x0000, 0x0157, CRC(598d5009) SHA1(9804f05fbf1b9324f8c3937e0953da02870d988b) ) /* GAL20V8A */
	ROM_LOAD( "136094-0010a.5n",  0x0000, 0x0117, CRC(87ff6393) SHA1(df1f0a5450485598c0ef7fa4981cc0e40a6a5073) ) /* GAL16V8A */
	ROM_LOAD( "136094-0011b.5r",  0x0000, 0x0117, CRC(832671eb) SHA1(85232128a4b03c4e3dffb4f2e6381a89f4f9aac5) ) /* GAL16V8A */
	ROM_LOAD( "136094-0009a.7n",  0x0000, 0x0117, NO_DUMP ) /* GAL16V8A */
	ROM_LOAD( "136094-0012a.7r",  0x0000, 0x0117, NO_DUMP ) /* GAL16V8A */
	ROM_LOAD( "136094-0007a.12l", 0x0000, 0x0117, NO_DUMP ) /* GAL16V8A */
	ROM_LOAD( "136094-0006a.13r", 0x0000, 0x0117, CRC(d5c84926) SHA1(22d2821ed77ad070163e3d188b1412f8d8d52977) ) /* GAL16V8A */
	ROM_LOAD( "136094-0008a.17l", 0x0000, 0x0117, CRC(b85ab18d) SHA1(eabcc2e54c2b6bc393603a31d22418edf60593ad) ) /* GAL16V8A, hand written "ROAD 2" over label */
	ROM_LOAD( "136094-0014a.22r", 0x0000, 0x0117, CRC(9dc3831d) SHA1(553c289801eb1e15118bc045ddca226343e6a623) ) /* GAL16V8A */
	ROM_LOAD( "136094-0016a.23r", 0x0000, 0x0117, NO_DUMP ) /* GAL16V8A */
	ROM_LOAD( "136094-0015a.23s", 0x0000, 0x0117, CRC(9404e122) SHA1(fb1db0fdb10ddeb7247dd254b3e725b9ef85097b) ) /* GAL16V8A */
	ROM_LOAD( "136094-0013a.24c", 0x0000, 0x0117, NO_DUMP ) /* GAL16V8A */
	ROM_LOAD( "136094-0018a.24j", 0x0000, 0x0117, CRC(9def4158) SHA1(11c168e2c16046e1213786c065906455fdb5a63c) ) /* GAL16V8A */
	ROM_LOAD( "136094-0017a.25c", 0x0000, 0x0117, CRC(76d8fa5b) SHA1(5fcb7b75f37f918331d99422ea3f0ea202665d5e) ) /* GAL16V8A */
ROM_END


ROM_START( rrrevengb )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 8*64k for 68000 code */
	ROM_LOAD32_BYTE( "rrprghh.23e", 0x00000, 0x20000, CRC(d2903e9d) SHA1(8782cd6ee39e2159b9ebc68ecdc3ecefcdeb8623) )
	ROM_LOAD32_BYTE( "rrprghl.23j", 0x00001, 0x20000, CRC(1afd500c) SHA1(6d24087a839e5e7d9c764026a9f3089e52785cdb) )
	ROM_LOAD32_BYTE( "rrprglh.37e", 0x00002, 0x20000, CRC(2b03a6fc) SHA1(7c95a0307b854bd37fd327ff1af1b69aa60fb2fd) )
	ROM_LOAD32_BYTE( "rrprgll.37j", 0x00003, 0x20000, CRC(acf078da) SHA1(3506e105d3b208864ce12ab20e6250cb3a0005d6) )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) /* 64k for 6502 code */
	ROM_LOAD( "rr65snd.bin", 0x00000, 0x10000, CRC(d78429da) SHA1(a4d36d74986f08c793f15f2e67cb97a8c91c5e90) )

	ROM_REGION( 0x180000, "tiles", 0 )
	ROM_LOAD( "rralpl.2d", 0x000000, 0x80000, CRC(00488dad) SHA1(604f08a219db0438dcbf21337ebd497f353bd812) ) /* playfield, planes 0-1 */
	ROM_LOAD( "rralpm.5d", 0x080000, 0x80000, CRC(ade27447) SHA1(641fdca97a4b08251e111425d8467e4640433df7) ) /* playfield, planes 2-3 */
	ROM_LOAD( "rralph.8d", 0x100000, 0x80000, CRC(ef04f04e) SHA1(e518133096978c4a0152253231625c385a84530f) ) /* playfield, planes 4-5 */

	ROM_REGION( 0x020000, "chars", 0 )
	ROM_LOAD( "rralalph.13n", 0x000000, 0x20000, CRC(7ca93790) SHA1(5e2f069be4b15d63f418c8693e8550eb0ae22381) ) /* alphanumerics */

	ROM_REGION16_BE( 0x500000, "rle", 0 )
	ROM_LOAD16_BYTE( "rrmo0h.31n", 0x000000, 0x80000, CRC(9b7e0315) SHA1(856804e89f586a4e777da5f47dc29c4e34175f44) )
	ROM_LOAD16_BYTE( "rrmo0l.31l", 0x000001, 0x80000, CRC(10478697) SHA1(9682e82cfbdc20f63d5c49303265c598a334c15b) )
	ROM_LOAD16_BYTE( "rrmo1h.33n", 0x100000, 0x80000, CRC(c7a48389) SHA1(a263aa4829cb243440ebe0496dd4f0158d97e2cc) )
	ROM_LOAD16_BYTE( "rrmo1l.33l", 0x100001, 0x80000, CRC(085a67c1) SHA1(0449abb5fec9014bd0ad14ce4802f726a90bc48c) )
	ROM_LOAD16_BYTE( "rrmo2h.35n", 0x200000, 0x80000, CRC(aea35aff) SHA1(2e02d2e877d356f9fb07e8b55ce252a148192d59) )
	ROM_LOAD16_BYTE( "rrmo2l.35l", 0x200001, 0x80000, CRC(b256d6d6) SHA1(0f0a05e3e58a00662f99988a050f2f5a11592d29) )
	ROM_LOAD16_BYTE( "rrmo3h.37n", 0x300000, 0x80000, CRC(e02549d7) SHA1(8b373d4969c77a14b15588f8a7e2977277fc9752) )
	ROM_LOAD16_BYTE( "rrmo3l.37l", 0x300001, 0x80000, CRC(8c81b537) SHA1(16cf8804687590aedf9020f9619bd14240f3b628) )
	ROM_LOAD16_BYTE( "rrmo4h.31t", 0x400000, 0x80000, CRC(12bf3e11) SHA1(37b1a7fe0b50202030f5c1938b95a449bbd51add) )
	ROM_LOAD16_BYTE( "rrmo4l.31r", 0x400001, 0x80000, CRC(a80175f6) SHA1(db621902fdfa99ec532713f4314c6cbb8353a773) )

	ROM_REGION( 0x80000, "jsa:oki1", 0 )
	ROM_LOAD( "rralpc0.bin",  0x00000, 0x80000, CRC(1f7b6ecf) SHA1(1787a2e89618e1338d70a54684dbc7d44c5f5559) )

	ROM_REGION( 0x80000, "jsa:oki2", 0 )
	ROM_LOAD( "rralpc1.bin",  0x00000, 0x80000, CRC(7ccd26d7) SHA1(1a74bdc66482896f5b9795d27383aa993e5fbaa4) )

	ROM_REGION( 0x0600, "proms", 0 )    /* microcode for growth renderer */
	ROM_LOAD( "136094-0001a.22s",  0x0000, 0x0200, CRC(a70ade3f) SHA1(f4a558b17767eed2683c768d1b441e75edcff967) ) /* 74S472AN BPROM */
	ROM_LOAD( "136094-0002a.21s",  0x0200, 0x0200, CRC(f4768b4d) SHA1(a506fa5386ab0ea2851ff1f8474d4bfc66deaa70) ) /* 74S472AN BPROM */
	ROM_LOAD( "136094-0003a.20s",  0x0400, 0x0200, CRC(22a76ad4) SHA1(ce840c283bbd3a5f19dc8d91b19d1571eff51ff4) ) /* 74S472AN BPROM */

	ROM_REGION( 0x0600, "pals", 0 ) /* none of these have been verified as good */
	ROM_LOAD( "136094-0019b.3n",  0x0000, 0x0157, CRC(598d5009) SHA1(9804f05fbf1b9324f8c3937e0953da02870d988b) ) /* GAL20V8A */
	ROM_LOAD( "136094-0010a.5n",  0x0000, 0x0117, CRC(87ff6393) SHA1(df1f0a5450485598c0ef7fa4981cc0e40a6a5073) ) /* GAL16V8A */
	ROM_LOAD( "136094-0011b.5r",  0x0000, 0x0117, CRC(832671eb) SHA1(85232128a4b03c4e3dffb4f2e6381a89f4f9aac5) ) /* GAL16V8A */
	ROM_LOAD( "136094-0009a.7n",  0x0000, 0x0117, NO_DUMP ) /* GAL16V8A */
	ROM_LOAD( "136094-0012a.7r",  0x0000, 0x0117, NO_DUMP ) /* GAL16V8A */
	ROM_LOAD( "136094-0007a.12l", 0x0000, 0x0117, NO_DUMP ) /* GAL16V8A */
	ROM_LOAD( "136094-0006a.13r", 0x0000, 0x0117, CRC(d5c84926) SHA1(22d2821ed77ad070163e3d188b1412f8d8d52977) ) /* GAL16V8A */
	ROM_LOAD( "136094-0008a.17l", 0x0000, 0x0117, CRC(b85ab18d) SHA1(eabcc2e54c2b6bc393603a31d22418edf60593ad) ) /* GAL16V8A, hand written "ROAD 2" over label */
	ROM_LOAD( "136094-0014a.22r", 0x0000, 0x0117, CRC(9dc3831d) SHA1(553c289801eb1e15118bc045ddca226343e6a623) ) /* GAL16V8A */
	ROM_LOAD( "136094-0016a.23r", 0x0000, 0x0117, NO_DUMP ) /* GAL16V8A */
	ROM_LOAD( "136094-0015a.23s", 0x0000, 0x0117, CRC(9404e122) SHA1(fb1db0fdb10ddeb7247dd254b3e725b9ef85097b) ) /* GAL16V8A */
	ROM_LOAD( "136094-0013a.24c", 0x0000, 0x0117, NO_DUMP ) /* GAL16V8A */
	ROM_LOAD( "136094-0018a.24j", 0x0000, 0x0117, CRC(9def4158) SHA1(11c168e2c16046e1213786c065906455fdb5a63c) ) /* GAL16V8A */
	ROM_LOAD( "136094-0017a.25c", 0x0000, 0x0117, CRC(76d8fa5b) SHA1(5fcb7b75f37f918331d99422ea3f0ea202665d5e) ) /* GAL16V8A */
ROM_END



/*************************************
 *
 *  Driver initialization
 *
 *************************************/

void atarigx2_state::init_spclords()
{
	m_playfield_base = 0x000;

	m_maincpu->space(AS_PROGRAM).install_read_handler(0xc80f00, 0xc80fff, read32s_delegate(downcast<atari_136095_0072_device &>(*m_xga), FUNC(atari_136095_0072_device::polylsb_read)));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xc80f00, 0xc80fff, write32s_delegate(downcast<atari_136095_0072_device &>(*m_xga), FUNC(atari_136095_0072_device::polylsb_write)));
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0xca0000, 0xca0fff, read32s_delegate(*m_xga, FUNC(atari_xga_device::read)), write32s_delegate(*m_xga, FUNC(atari_xga_device::write)));
}


void atarigx2_state::init_motofren()
{
	m_playfield_base = 0x400;
/*
L/W=!68.A23*!E.A22*!E.A21                                       = 000x xxxx = 000000-1fffff
   +68.A23*E.A22*E.A21*68.A20*68.A19*68.A18*68.A17              = 1111 111x = fe0000-ffffff

68.XIO=68.A23*E.A22*E.A21*!68.A20*!68.A18*!68.A17*!68.A16       = 1110 x000 = e00000-e0ffff, e80000-e8ffff
    +68.A23*E.A22*E.A21*!68.A20*!68.A19                         = 1110 0xxx = e00000-e7ffff

68.ROM=!68.A23*68.AS*!E.A22*!E.A21                              = 000x xxxx = 000000-1fffff

68.EXT=68.A23*68.AS*E.A22*!E.A21*!68.A20                        = 1100 xxxx = c00000-cfffff

68.WRAM=68.A23*68.AS*E.A22*E.A21*68.A20*68.A19*68.A18*68.A17    = 1111 111x = fe0000-ffffff

XMEM=68.A23*E.A22*!E.A21*68.A20                                 = 1101 xxxx = d00000-dfffff

68.W1=68.A23*E.A22*!E.A21*68.A20*!68.A18                        = 1101 x0xx = d00000-d3ffff, d80000-dbffff

68.W0=68.A23*E.A22*!E.A21*68.A20*!68.A18                        = 1101 x0xx = d00000-d3ffff, d80000-dbffff
    +68.A23*E.A22*!E.A21*68.A20*!68.A19                         = 1101 0xxx = d00000-d7ffff
    +68.A23*E.A22*!E.A21*!68.A20*68.A19                         = 1100 1xxx = c80000-cfffff
    +!68.A23*!E.A22*!E.A21                                      = 000x xxxx = 000000-1fffff
*/
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0xca0000, 0xca0fff, read32s_delegate(*m_xga, FUNC(atari_xga_device::read)), write32s_delegate(*m_xga, FUNC(atari_xga_device::write)));
}

void atarigx2_state::init_rrreveng()
{
	// ROM-derived candidate: 17 tracks have plaintext height differences
	// and independently stored, smoothed curvature for validation.
	// All observed key mappings match Space Lords; the upper taps differ.
	// Keep the not-working/protection flags pending full runtime validation.
	init_spclords();
	downcast<atari_136095_0072_device &>(*m_xga).set_polynomial_high(0xf000);
}


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1992, spclords,  0,         atarigx2_0x400, spclords, atarigx2_state, init_spclords, ROT0, "Atari Games", "Space Lords (rev C)", MACHINE_NODEVICE_LAN )
GAME( 1992, spclordsb, spclords,  atarigx2_0x400, spclords, atarigx2_state, init_spclords, ROT0, "Atari Games", "Space Lords (rev B)", MACHINE_NODEVICE_LAN )
GAME( 1992, spclordsg, spclords,  atarigx2_0x400, spclords, atarigx2_state, init_spclords, ROT0, "Atari Games", "Space Lords (rev A, German)", MACHINE_NODEVICE_LAN )
GAME( 1992, spclordsa, spclords,  atarigx2_0x400, spclords, atarigx2_state, init_spclords, ROT0, "Atari Games", "Space Lords (rev A)", MACHINE_NODEVICE_LAN )

GAME( 1992, motofren,    0,        atarigx2_0x200, motofren, atarigx2_state, init_motofren, ROT0, "Atari Games", "Moto Frenzy", MACHINE_NODEVICE_LAN )
GAME( 1992, motofrenmd,  motofren, atarigx2_0x200, motofren, atarigx2_state, init_motofren, ROT0, "Atari Games", "Moto Frenzy (Mini Deluxe)", MACHINE_NODEVICE_LAN )
GAME( 1992, motofrenft,  motofren, atarigx2_0x200, motofren, atarigx2_state, init_motofren, ROT0, "Atari Games", "Moto Frenzy (Field Test Version, Jul 17, 1992)", MACHINE_NODEVICE_LAN )
GAME( 1992, motofrenfta, motofren, atarigx2_0x200, motofren, atarigx2_state, init_motofren, ROT0, "Atari Games", "Moto Frenzy (Field Test Version, Jul 22, 1992)", MACHINE_NODEVICE_LAN )
GAME( 1992, motofrenmf,  motofren, atarigx2_0x200, motofren, atarigx2_state, init_motofren, ROT0, "Atari Games", "Moto Frenzy (Mini Deluxe Field Test Version)", MACHINE_NODEVICE_LAN )

GAME( 1993, rrreveng,   0,        atarigx2_0x400, rrreveng, atarigx2_state, init_rrreveng, ROT0, "Atari Games", "Road Riot's Revenge (prototype, Sep 06, 1994)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING | MACHINE_NODEVICE_LAN )
GAME( 1993, rrrevenga,  rrreveng, atarigx2_0x400, rrreveng, atarigx2_state, init_rrreveng, ROT0, "Atari Games", "Road Riot's Revenge (prototype, Jan 27, 1994, set 1)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING | MACHINE_NODEVICE_LAN )
GAME( 1993, rrrevengb,  rrreveng, atarigx2_0x400, rrreveng, atarigx2_state, init_rrreveng, ROT0, "Atari Games", "Road Riot's Revenge (prototype, Jan 27, 1994, set 2)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING | MACHINE_NODEVICE_LAN )
