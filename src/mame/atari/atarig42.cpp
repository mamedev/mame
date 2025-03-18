// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari G42 hardware

    driver by Aaron Giles

    Games supported:
        * Road Riot 4WD (1991)
        * Danger Express (1992)
        * Guardians of the 'Hood (1992)

    Note: Road Riot 4WD sports a communication link with another PCB allowing two players to
    to compete with each other. The way both PCBs communicate depends on the program version.
        * The kit version uses an optional com-link board consisting on an ADSP-2105 (10MHz XTAL)
          routing game link data through the ADSP's serial port.
        * The dedicated twin cab version uses dual port RAM to bridge the two PCBs together (not
          networked).

    TODO:
        * ASIC65 for Road Riot tests bad with a "TIMEOUT ERROR" for the internal RAM test. Comm port and checksum test ok
        * Hook com-link board ADSP-2105 (and serial port) for roadriot and roadriota
        * Hook dual port RAM and dual PCB comms for roadriotb

***************************************************************************/

#include "emu.h"
#include "atarig42.h"
#include "machine/eeprompar.h"
#include "machine/watchdog.h"
#include "emupal.h"
#include "speaker.h"


/*************************************
 *
 *  Initialization & interrupts
 *
 *************************************/

void atarig42_state::video_int_ack_w(uint16_t data)
{
	m_maincpu->set_input_line(M68K_IRQ_4, CLEAR_LINE);
}


void atarig42_state::machine_start()
{
	atarigen_state::machine_start();

	save_item(NAME(m_sloop_bank));
	save_item(NAME(m_sloop_next_bank));
	save_item(NAME(m_sloop_offset));
	save_item(NAME(m_sloop_state));

	m_sloop_bank = 0;
}



/*************************************
 *
 *  I/O read dispatch.
 *
 *************************************/

void atarig42_state::a2d_select_w(offs_t offset, uint8_t data)
{
	if (m_adc.found())
		m_adc->address_offset_start_w(offset, 0);
}


uint8_t atarig42_state::a2d_data_r(offs_t offset)
{
	if (!m_adc.found())
		return 0xff;

	uint8_t const result = m_adc->data_r();
	if (!machine().side_effects_disabled())
		m_adc->address_offset_start_w(offset, 0);
	return result;
}


void atarig42_state::io_latch_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	/* upper byte */
	if (ACCESSING_BITS_8_15)
	{
		/* bit 14 controls the ASIC65 reset line */
		m_asic65->reset_line(BIT(~data, 14));

		/* bits 13-11 are the MO control bits */
		m_rle->control_write((data >> 11) & 7);
	}

	/* lower byte */
	if (ACCESSING_BITS_0_7)
	{
		/* bit 4 resets the sound CPU */
		m_jsa->soundcpu().set_input_line(INPUT_LINE_RESET, BIT(data, 4) ? CLEAR_LINE : ASSERT_LINE);
		if (BIT(~data, 4))
			m_jsa->reset();

		/* bit 5 is /XRESET, probably related to the ASIC */

		/* bits 3 and 0 are coin counters */
	}
}


void atarig42_state::mo_command_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(m_mo_command);
	m_rle->command_write((data == 0) ? atari_rle_objects_device::COMMAND_CHECKSUM : atari_rle_objects_device::COMMAND_DRAW);
}



/*************************************
 *
 *  SLOOP banking -- Road Riot
 *
 *************************************/

void atarig42_0x200_state::roadriot_sloop_tweak(int offset)
{
/*
    sequence 1:

        touch $68000
        touch $68eee and $124/$678/$abc/$1024(bank) in the same instruction
        touch $69158/$6a690/$6e708/$71166

    sequence 2:

        touch $5edb4 to add 2 to the bank
        touch $5db0a to add 1 to the bank
        touch $5f042
        touch $69158/$6a690/$6e708/$71166
        touch $68000
        touch $5d532/$5d534
*/

	switch (offset)
	{
		/* standard 68000 -> 68eee -> (bank) addressing */
		case 0x68000/2:
			m_sloop_state = 1;
			break;
		case 0x68eee/2:
			if (m_sloop_state == 1)
				m_sloop_state = 2;
			break;
		case 0x00124/2:
			if (m_sloop_state == 2)
			{
				m_sloop_next_bank = 0;
				m_sloop_state = 3;
			}
			break;
		case 0x00678/2:
			if (m_sloop_state == 2)
			{
				m_sloop_next_bank = 1;
				m_sloop_state = 3;
			}
			break;
		case 0x00abc/2:
			if (m_sloop_state == 2)
			{
				m_sloop_next_bank = 2;
				m_sloop_state = 3;
			}
			break;
		case 0x01024/2:
			if (m_sloop_state == 2)
			{
				m_sloop_next_bank = 3;
				m_sloop_state = 3;
			}
			break;

		/* lock in the change? */
		case 0x69158/2:
			/* written if $ff8007 == 0 */
		case 0x6a690/2:
			/* written if $ff8007 == 1 */
		case 0x6e708/2:
			/* written if $ff8007 == 2 */
		case 0x71166/2:
			/* written if $ff8007 == 3 */
			if (m_sloop_state == 3)
				m_sloop_bank = m_sloop_next_bank;
			m_sloop_state = 0;
			break;

		/* bank offsets */
		case 0x5edb4/2:
			if (m_sloop_state == 0)
			{
				m_sloop_state = 10;
				m_sloop_offset = 0;
			}
			m_sloop_offset += 2;
			break;
		case 0x5db0a/2:
			if (m_sloop_state == 0)
			{
				m_sloop_state = 10;
				m_sloop_offset = 0;
			}
			m_sloop_offset += 1;
			break;

		/* apply the offset */
		case 0x5f042/2:
			if (m_sloop_state == 10)
			{
				m_sloop_bank = (m_sloop_bank + m_sloop_offset) & 3;
				m_sloop_offset = 0;
				m_sloop_state = 0;
			}
			break;

		/* unknown */
		case 0x5d532/2:
			break;
		case 0x5d534/2:
			break;

		default:
			break;
	}
}


uint16_t atarig42_0x200_state::roadriot_sloop_data_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		roadriot_sloop_tweak(offset);
	if (offset < 0x78000/2)
		return m_sloop_base[offset];
	else
		return m_sloop_base[0x78000/2 + m_sloop_bank * 0x1000 + (offset & 0xfff)];
}


void atarig42_0x200_state::roadriot_sloop_data_w(offs_t offset, uint16_t data)
{
	roadriot_sloop_tweak(offset);
}



/*************************************
 *
 *  SLOOP banking -- Guardians
 *
 *************************************/

void atarig42_0x400_state::guardians_sloop_tweak(int offset)
{
	uint32_t *last_accesses = m_last_accesses;

	if (offset >= 0x7f7c0/2)
	{
		last_accesses[0] = last_accesses[1];
		last_accesses[1] = last_accesses[2];
		last_accesses[2] = last_accesses[3];
		last_accesses[3] = last_accesses[4];
		last_accesses[4] = last_accesses[5];
		last_accesses[5] = last_accesses[6];
		last_accesses[6] = last_accesses[7];
		last_accesses[7] = offset;

		if (last_accesses[0] == 0x7f7c0/2 && last_accesses[1] == 0x7f7ce/2 && last_accesses[2] == 0x7f7c2/2 && last_accesses[3] == 0x7f7cc/2 &&
			last_accesses[4] == 0x7f7c4/2 && last_accesses[5] == 0x7f7ca/2 && last_accesses[6] == 0x7f7c6/2 && last_accesses[7] == 0x7f7c8/2)
			m_sloop_bank = 0;

		if (last_accesses[0] == 0x7f7d0/2 && last_accesses[1] == 0x7f7de/2 && last_accesses[2] == 0x7f7d2/2 && last_accesses[3] == 0x7f7dc/2 &&
			last_accesses[4] == 0x7f7d4/2 && last_accesses[5] == 0x7f7da/2 && last_accesses[6] == 0x7f7d6/2 && last_accesses[7] == 0x7f7d8/2)
			m_sloop_bank = 1;

		if (last_accesses[0] == 0x7f7e0/2 && last_accesses[1] == 0x7f7ee/2 && last_accesses[2] == 0x7f7e2/2 && last_accesses[3] == 0x7f7ec/2 &&
			last_accesses[4] == 0x7f7e4/2 && last_accesses[5] == 0x7f7ea/2 && last_accesses[6] == 0x7f7e6/2 && last_accesses[7] == 0x7f7e8/2)
			m_sloop_bank = 2;

		if (last_accesses[0] == 0x7f7f0/2 && last_accesses[1] == 0x7f7fe/2 && last_accesses[2] == 0x7f7f2/2 && last_accesses[3] == 0x7f7fc/2 &&
			last_accesses[4] == 0x7f7f4/2 && last_accesses[5] == 0x7f7fa/2 && last_accesses[6] == 0x7f7f6/2 && last_accesses[7] == 0x7f7f8/2)
			m_sloop_bank = 3;
	}
}


uint16_t atarig42_0x400_state::guardians_sloop_data_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		guardians_sloop_tweak(offset);
	if (offset < 0x78000/2)
		return m_sloop_base[offset];
	else
		return m_sloop_base[0x78000/2 + m_sloop_bank * 0x1000 + (offset & 0xfff)];
}


void atarig42_0x400_state::guardians_sloop_data_w(offs_t offset, uint16_t data)
{
	guardians_sloop_tweak(offset);
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void atarig42_state::main_map(address_map &map)
{
	map(0x000000, 0x080001).rom();
	map(0xe00000, 0xe00001).portr("IN0");
	map(0xe00002, 0xe00003).portr("IN1");
	map(0xe00010, 0xe00011).portr("IN2");
	map(0xe00012, 0xe00013).portr("jsa:JSAIII");
	map(0xe00020, 0xe0002f).rw(FUNC(atarig42_state::a2d_data_r), FUNC(atarig42_state::a2d_select_w)).umask16(0xff00);
	map(0xe00031, 0xe00031).r(m_jsa, FUNC(atari_jsa_iii_device::main_response_r));
	map(0xe00041, 0xe00041).w(m_jsa, FUNC(atari_jsa_iii_device::main_command_w));
	map(0xe00050, 0xe00051).w(FUNC(atarig42_state::io_latch_w));
	map(0xe00060, 0xe00061).w("eeprom", FUNC(eeprom_parallel_28xx_device::unlock_write16));
	map(0xe03000, 0xe03001).w(FUNC(atarig42_state::video_int_ack_w));
	map(0xe03800, 0xe03801).w("watchdog", FUNC(watchdog_timer_device::reset16_w));
	map(0xe80000, 0xe80fff).ram();
	map(0xf40000, 0xf40001).r(m_asic65, FUNC(asic65_device::io_r));
	map(0xf60000, 0xf60001).r(m_asic65, FUNC(asic65_device::read));
	map(0xf80000, 0xf80003).w(m_asic65, FUNC(asic65_device::data_w));
	map(0xfa0000, 0xfa0fff).rw("eeprom", FUNC(eeprom_parallel_28xx_device::read), FUNC(eeprom_parallel_28xx_device::write)).umask16(0x00ff);
	map(0xfc0000, 0xfc0fff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0xff0000, 0xff0fff).ram().share("rle");
	map(0xff1000, 0xff1fff).ram();
	map(0xff2000, 0xff5fff).ram().w(m_playfield_tilemap, FUNC(tilemap_device::write16)).share("playfield");
	map(0xff6000, 0xff6fff).ram().w(m_alpha_tilemap, FUNC(tilemap_device::write16)).share("alpha");
	map(0xff7000, 0xff7001).ram().w(FUNC(atarig42_state::mo_command_w)).share(m_mo_command);
	map(0xff7002, 0xffffff).ram();
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( roadriot )
	PORT_START("IN0")       /* e00000 */
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0xf800, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")      /* e00002 */
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")       /* e00010 */
	PORT_BIT( 0x0007, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("adc", FUNC(adc0808_device::eoc_r))
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_SOUND_TO_MAIN_READY("jsa")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_MAIN_TO_SOUND_READY("jsa")
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW )
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("A2D0")      /* analog 0 */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10)

	PORT_START("A2D1")      /* analog 1 */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(16)
INPUT_PORTS_END


INPUT_PORTS_START( dangerex )
	PORT_START("IN0")       /* e00000 */
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_CUSTOM ) // Toggle 0 - D4
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_CUSTOM ) // Toggle 1 - D5
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_CUSTOM ) // Step SW - D6
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_CUSTOM ) // Freeze - D7
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)

	PORT_START("IN1")      /* e00002 */
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_CUSTOM ) // Test Yellow - D4
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_CUSTOM ) // Test Blue - D5
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_CUSTOM ) // Test Blue - D6
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_CUSTOM ) // Test Blue - D7
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)

	PORT_START("IN2")       /* e00010 */
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_SOUND_TO_MAIN_READY("jsa")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_MAIN_TO_SOUND_READY("jsa")
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW )
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("A2D0")      /* analog 0 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("A2D1")      /* analog 1 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( guardian )
	PORT_START("IN0")       /* e00000 */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(3)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(3)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(3)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)

	PORT_START("IN1")      /* e00002 */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x01c0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)

	PORT_START("IN2")       /* e00010 */
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_SOUND_TO_MAIN_READY("jsa")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_MAIN_TO_SOUND_READY("jsa")
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW )
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
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

static GFXDECODE_START( gfx_atarig42 )
	GFXDECODE_ENTRY( "tiles", 0, pflayout, 0x000, 64 )
	GFXDECODE_ENTRY( "chars", 0, gfx_8x8x4_packed_msb, 0x000, 16 )
	GFXDECODE_ENTRY( "tiles", 0, pftoplayout, 0x000, 64 )
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

void atarig42_state::atarig42(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 14.318181_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &atarig42_state::main_map);

	TIMER(config, "scantimer").configure_scanline(FUNC(atarig42_state::scanline_update), m_screen, 0, 8);

	EEPROM_2816(config, "eeprom").lock_after_write(true);

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	GFXDECODE(config, m_gfxdecode, "palette", gfx_atarig42);
	PALETTE(config, "palette").set_format(palette_device::IRGB_1555, 2048);

	TILEMAP(config, m_playfield_tilemap, m_gfxdecode, 2, 8,8);
	m_playfield_tilemap->set_layout(FUNC(atarig42_state::atarig42_playfield_scan), 128,64);
	m_playfield_tilemap->set_info_callback(FUNC(atarig42_state::get_playfield_tile_info));
	TILEMAP(config, m_alpha_tilemap, m_gfxdecode, 2, 8,8, TILEMAP_SCAN_ROWS, 64,32, 0).set_info_callback(FUNC(atarig42_state::get_alpha_tile_info));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	/* note: these parameters are from published specs, not derived */
	/* the board uses an SOS chip to generate video signals */
	m_screen->set_raw(14.318181_MHz_XTAL/2, 456, 0, 336, 262, 0, 240);
	m_screen->set_screen_update(FUNC(atarig42_state::screen_update));
	m_screen->set_palette("palette");
	m_screen->screen_vblank().set_inputline(m_maincpu, M68K_IRQ_4, ASSERT_LINE);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ATARI_JSA_III(config, m_jsa, 0);
	m_jsa->main_int_cb().set_inputline(m_maincpu, M68K_IRQ_5);
	m_jsa->test_read_cb().set_ioport("IN2").bit(6);
	m_jsa->add_route(ALL_OUTPUTS, "mono", 0.8);
}

void atarig42_0x200_state::atarig42_0x200(machine_config &config)
{
	atarig42(config);
	ATARI_RLE_OBJECTS(config, m_rle, 0, modesc_0x200);

	ADC0809(config, m_adc, 14.318181_MHz_XTAL / 16);
	m_adc->in_callback<0>().set_ioport("A2D0");
	m_adc->in_callback<1>().set_ioport("A2D1");

	/* ASIC65 */
	ASIC65(config, m_asic65, 0, ASIC65_ROMBASED);
}

void atarig42_0x400_state::atarig42_0x400(machine_config &config)
{
	atarig42(config);
	ATARI_RLE_OBJECTS(config, m_rle, 0, modesc_0x400);

	/* ASIC65 */
	ASIC65(config, m_asic65, 0, ASIC65_GUARDIANS);
}



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( roadriot ) // Test mode shows DSP COMM and DSP LINK tests; This is a conversion kit version.
	ROM_REGION( 0x80004, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "136089-3214.8d", 0x00000, 0x20000, CRC(6b4dc220) SHA1(43517d8adbc8771ac4e4dcde221def12e1859b61) )
	ROM_LOAD16_BYTE( "136089-3213.8c", 0x00001, 0x20000, CRC(2f182b74) SHA1(b6b30bc068b5eeb52e0000236b7f58b1e88bb154) )
	ROM_LOAD16_BYTE( "136089-2016.9d", 0x40000, 0x20000, CRC(6191653c) SHA1(97d1a84a585149e8f2c49cab7af22dc755dff350) )
	ROM_LOAD16_BYTE( "136089-2015.9c", 0x40001, 0x20000, CRC(0d34419a) SHA1(f16e9fb4cd537d727611cb7dd5537c030671fe1e) )

	ROM_REGION( 0x2000, "asic65:asic65cpu", 0 )   /* ASIC65 TMS32015 code */
	ROM_LOAD( "136089-1012.3f", 0x00000, 0x0a80, CRC(7c5498e7) SHA1(9d8b235baf7b75bef8ef9b168647c5b2b80b2cb3) )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) /* 6502 code */
	ROM_LOAD( "136089-1047.12c", 0x00000, 0x10000, CRC(849dd26c) SHA1(05a0b2a5f7ee4437448b5f076d3066d96dec2320) )

	ROM_REGION( 0xc0000, "tiles", 0 )
	ROM_LOAD( "136089-1041.22d",    0x000000, 0x20000, CRC(b7451f92) SHA1(9fd17913630e457e406e596f2d86afff98787750) ) /* playfield, planes 0-1 */
	ROM_LOAD( "136089-1038.22c",    0x020000, 0x20000, CRC(90f3c6ee) SHA1(7607509e2d3b2080a918cfaf2879dbed6b79d029) )
	ROM_LOAD( "136089-1037.2021d",  0x040000, 0x20000, CRC(d40de62b) SHA1(fa6dfd20bdad7874ae33a1027a9bb0ea200f86ca) ) /* playfield, planes 2-3 */
	ROM_LOAD( "136089-1039.2021c",  0x060000, 0x20000, CRC(a7e836b1) SHA1(d41f1e4166ca757176c6976be2a953db5db05e48) )
	ROM_LOAD( "136089-1040.20d",    0x080000, 0x20000, CRC(a81ae93f) SHA1(b694ba5fab35f8fa505a02039ae62f7af3c7ae1d) ) /* playfield, planes 4-5 */
	ROM_LOAD( "136089-1042.20c",    0x0a0000, 0x20000, CRC(b8a6d15a) SHA1(43d2be9d40a84b2c01d80bbcac737eda04d55999) )

	ROM_REGION( 0x020000, "chars", 0 )
	ROM_LOAD( "136089-1046.22j",    0x000000, 0x20000, CRC(0005bab0) SHA1(257e1b23eea117fe6701a67134b96d9d9fe10caf) ) /* alphanumerics */

	ROM_REGION16_BE( 0x200000, "rle", 0 )
	ROM_LOAD16_BYTE( "136089-1018.2s", 0x000000, 0x20000, CRC(19590a94) SHA1(e375b7e01a8b1f366bb4e7750e33f0b6d9ae2042) )
	ROM_LOAD16_BYTE( "136089-1017.2p", 0x000001, 0x20000, CRC(c2bf3f69) SHA1(f822359070b1907973ee7ee35469f4a59f720830) )
	ROM_LOAD16_BYTE( "136089-1020.3s", 0x040000, 0x20000, CRC(bab110e4) SHA1(0c4e3521474249517e7832df1bc63aca6d6a6c91) )
	ROM_LOAD16_BYTE( "136089-1019.3p", 0x040001, 0x20000, CRC(791ad2c5) SHA1(4ef218fbf38a9c6b58c86f203843988df1c935f6) )
	ROM_LOAD16_BYTE( "136089-1022.4s", 0x080000, 0x20000, CRC(79cba484) SHA1(ce361a432f1fe627086bab3c1131118fd15056f1) )
	ROM_LOAD16_BYTE( "136089-1021.4p", 0x080001, 0x20000, CRC(86a2e257) SHA1(98d95d2e67fecc332f6c66358a1f8d58b168c74b) )
	ROM_LOAD16_BYTE( "136089-1024.5s", 0x0c0000, 0x20000, CRC(67d28478) SHA1(cfc9da6d20c65d11c2a59a38660a8da4d1cc219d) )
	ROM_LOAD16_BYTE( "136089-1023.5p", 0x0c0001, 0x20000, CRC(74638838) SHA1(bea0fb21ccb946e023c88791ce5a8dd92b44baec) )
	ROM_LOAD16_BYTE( "136089-1026.6s", 0x100000, 0x20000, CRC(24933c37) SHA1(516393aae51fc9634a5c6d5134be058d6067e114) )
	ROM_LOAD16_BYTE( "136089-1025.6p", 0x100001, 0x20000, CRC(054a163b) SHA1(1b0b129c093398bc5c14b3fdd87dfe149f555fac) )
	ROM_LOAD16_BYTE( "136089-1028.7s", 0x140000, 0x20000, CRC(31ff090a) SHA1(7b43ed37901c3f94cae90c84b3c8c689d7b64dd6) )
	ROM_LOAD16_BYTE( "136089-1027.7p", 0x140001, 0x20000, CRC(bbe5b69b) SHA1(9eaa551fba763824d36fc41bfe0e6d735a9e68c5) )
	ROM_LOAD16_BYTE( "136089-1030.8s", 0x180000, 0x20000, CRC(6c89d2c5) SHA1(0bf2990ce1cd5ec5b84f7e3171725540e6238408) )
	ROM_LOAD16_BYTE( "136089-1029.8p", 0x180001, 0x20000, CRC(40d9bde5) SHA1(aca6e07ea96e4618412d32fe4d4cd293ae82d940) )
	ROM_LOAD16_BYTE( "136089-1032.9s", 0x1c0000, 0x20000, CRC(eca3c595) SHA1(5d067b7c02675b1e6dd3c4046697a16f873f80af) )
	ROM_LOAD16_BYTE( "136089-1031.9p", 0x1c0001, 0x20000, CRC(88acdb53) SHA1(5bf2424ee75a25248a8ce38c8605b6780da4e323) )

	ROM_REGION( 0x80000, "jsa:oki1", 0 )
	ROM_LOAD( "136089-1048.19e",  0x00000, 0x20000, CRC(2db638a7) SHA1(45da8088f7439beacc3056952a4d631d9633efa7) )
	ROM_LOAD( "136089-1049.17e",  0x20000, 0x20000, CRC(e1dd7f9e) SHA1(6b9a240aa84d210d3052daab6ea26f9cd0e62dc1) )
	ROM_LOAD( "136089-1050.15e",  0x40000, 0x20000, CRC(64d410bb) SHA1(877bccca7ff37a9dd8294bc1453487a2f516ca7d) )
	ROM_LOAD( "136089-1051.12e",  0x60000, 0x20000, CRC(bffd01c8) SHA1(f6de000f61ea0c1ddb31ee5301506e5e966638c2) )

	ROM_REGION( 0x800, "eeprom", 0 )
	ROM_LOAD( "roadriot-eeprom.5c", 0x0000, 0x800, CRC(8d9b957d) SHA1(9d895c5977a3f405130594a10d530a82a6aa265f) )

	ROM_REGION( 0x0600, "proms", 0 )    /* microcode for growth renderer */
	ROM_LOAD( "136089-1001.20p",  0x0000, 0x0200, CRC(5836cb5a) SHA1(2c797f6a1227d6e1fd7a12f99f0254072c8c266e) )
	ROM_LOAD( "136089-1002.22p",  0x0200, 0x0200, CRC(44288753) SHA1(811582015264f85a32643196cdb331a41430318f) )
	ROM_LOAD( "136089-1003.21p",  0x0400, 0x0200, CRC(1f571706) SHA1(26d5ea59163b3482ab1f8a26178d0849c5fd9692) )

	ROM_REGION( 0x10000, "commcpu", 0 ) // ADSP-2105 code for communications with another PCB
	ROM_LOAD( "136087-1025.4e", 0x0000, 0x10000, CRC(4c645933) SHA1(7a1cf049e368059a79b03598de73c30d8dae5e90) )
ROM_END

ROM_START( roadriota ) // Test mode shows DSP COMM and DSP LINK tests; This is a conversion kit version.
	ROM_REGION( 0x80004, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "136089-3114.8d", 0x00000, 0x20000, CRC(a2bd949c) SHA1(f96064d491b4d488cadebd3a63a6d3edf9236046) )
	ROM_LOAD16_BYTE( "136089-3113.8c", 0x00001, 0x20000, CRC(68c45cb1) SHA1(e38c7ad3f3d301e59a1d9f53e8f2c28e91d691fe) )
	ROM_LOAD16_BYTE( "136089-2016.9d", 0x40000, 0x20000, CRC(6191653c) SHA1(97d1a84a585149e8f2c49cab7af22dc755dff350) )
	ROM_LOAD16_BYTE( "136089-2015.9c", 0x40001, 0x20000, CRC(0d34419a) SHA1(f16e9fb4cd537d727611cb7dd5537c030671fe1e) )

	ROM_REGION( 0x2000, "asic65:asic65cpu", 0 )   /* ASIC65 TMS32015 code */
	ROM_LOAD( "136089-1012.3f", 0x00000, 0x0a80, CRC(7c5498e7) SHA1(9d8b235baf7b75bef8ef9b168647c5b2b80b2cb3) )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) /* 6502 code */
	ROM_LOAD( "136089-1047.12c", 0x00000, 0x10000, CRC(849dd26c) SHA1(05a0b2a5f7ee4437448b5f076d3066d96dec2320) )

	ROM_REGION( 0xc0000, "tiles", 0 )
	ROM_LOAD( "136089-1041.22d",    0x000000, 0x20000, CRC(b7451f92) SHA1(9fd17913630e457e406e596f2d86afff98787750) ) /* playfield, planes 0-1 */
	ROM_LOAD( "136089-1038.22c",    0x020000, 0x20000, CRC(90f3c6ee) SHA1(7607509e2d3b2080a918cfaf2879dbed6b79d029) )
	ROM_LOAD( "136089-1037.2021d",  0x040000, 0x20000, CRC(d40de62b) SHA1(fa6dfd20bdad7874ae33a1027a9bb0ea200f86ca) ) /* playfield, planes 2-3 */
	ROM_LOAD( "136089-1039.2021c",  0x060000, 0x20000, CRC(a7e836b1) SHA1(d41f1e4166ca757176c6976be2a953db5db05e48) )
	ROM_LOAD( "136089-1040.20d",    0x080000, 0x20000, CRC(a81ae93f) SHA1(b694ba5fab35f8fa505a02039ae62f7af3c7ae1d) ) /* playfield, planes 4-5 */
	ROM_LOAD( "136089-1042.20c",    0x0a0000, 0x20000, CRC(b8a6d15a) SHA1(43d2be9d40a84b2c01d80bbcac737eda04d55999) )

	ROM_REGION( 0x020000, "chars", 0 )
	ROM_LOAD( "136089-1046.22j",    0x000000, 0x20000, CRC(0005bab0) SHA1(257e1b23eea117fe6701a67134b96d9d9fe10caf) ) /* alphanumerics */

	ROM_REGION16_BE( 0x200000, "rle", 0 )
	ROM_LOAD16_BYTE( "136089-1018.2s", 0x000000, 0x20000, CRC(19590a94) SHA1(e375b7e01a8b1f366bb4e7750e33f0b6d9ae2042) )
	ROM_LOAD16_BYTE( "136089-1017.2p", 0x000001, 0x20000, CRC(c2bf3f69) SHA1(f822359070b1907973ee7ee35469f4a59f720830) )
	ROM_LOAD16_BYTE( "136089-1020.3s", 0x040000, 0x20000, CRC(bab110e4) SHA1(0c4e3521474249517e7832df1bc63aca6d6a6c91) )
	ROM_LOAD16_BYTE( "136089-1019.3p", 0x040001, 0x20000, CRC(791ad2c5) SHA1(4ef218fbf38a9c6b58c86f203843988df1c935f6) )
	ROM_LOAD16_BYTE( "136089-1022.4s", 0x080000, 0x20000, CRC(79cba484) SHA1(ce361a432f1fe627086bab3c1131118fd15056f1) )
	ROM_LOAD16_BYTE( "136089-1021.4p", 0x080001, 0x20000, CRC(86a2e257) SHA1(98d95d2e67fecc332f6c66358a1f8d58b168c74b) )
	ROM_LOAD16_BYTE( "136089-1024.5s", 0x0c0000, 0x20000, CRC(67d28478) SHA1(cfc9da6d20c65d11c2a59a38660a8da4d1cc219d) )
	ROM_LOAD16_BYTE( "136089-1023.5p", 0x0c0001, 0x20000, CRC(74638838) SHA1(bea0fb21ccb946e023c88791ce5a8dd92b44baec) )
	ROM_LOAD16_BYTE( "136089-1026.6s", 0x100000, 0x20000, CRC(24933c37) SHA1(516393aae51fc9634a5c6d5134be058d6067e114) )
	ROM_LOAD16_BYTE( "136089-1025.6p", 0x100001, 0x20000, CRC(054a163b) SHA1(1b0b129c093398bc5c14b3fdd87dfe149f555fac) )
	ROM_LOAD16_BYTE( "136089-1028.7s", 0x140000, 0x20000, CRC(31ff090a) SHA1(7b43ed37901c3f94cae90c84b3c8c689d7b64dd6) )
	ROM_LOAD16_BYTE( "136089-1027.7p", 0x140001, 0x20000, CRC(bbe5b69b) SHA1(9eaa551fba763824d36fc41bfe0e6d735a9e68c5) )
	ROM_LOAD16_BYTE( "136089-1030.8s", 0x180000, 0x20000, CRC(6c89d2c5) SHA1(0bf2990ce1cd5ec5b84f7e3171725540e6238408) )
	ROM_LOAD16_BYTE( "136089-1029.8p", 0x180001, 0x20000, CRC(40d9bde5) SHA1(aca6e07ea96e4618412d32fe4d4cd293ae82d940) )
	ROM_LOAD16_BYTE( "136089-1032.9s", 0x1c0000, 0x20000, CRC(eca3c595) SHA1(5d067b7c02675b1e6dd3c4046697a16f873f80af) )
	ROM_LOAD16_BYTE( "136089-1031.9p", 0x1c0001, 0x20000, CRC(88acdb53) SHA1(5bf2424ee75a25248a8ce38c8605b6780da4e323) )

	ROM_REGION( 0x80000, "jsa:oki1", 0 )
	ROM_LOAD( "136089-1048.19e",  0x00000, 0x20000, CRC(2db638a7) SHA1(45da8088f7439beacc3056952a4d631d9633efa7) )
	ROM_LOAD( "136089-1049.17e",  0x20000, 0x20000, CRC(e1dd7f9e) SHA1(6b9a240aa84d210d3052daab6ea26f9cd0e62dc1) )
	ROM_LOAD( "136089-1050.15e",  0x40000, 0x20000, CRC(64d410bb) SHA1(877bccca7ff37a9dd8294bc1453487a2f516ca7d) )
	ROM_LOAD( "136089-1051.12e",  0x60000, 0x20000, CRC(bffd01c8) SHA1(f6de000f61ea0c1ddb31ee5301506e5e966638c2) )

	ROM_REGION( 0x800, "eeprom", 0 )
	ROM_LOAD( "roadriot-eeprom.5c", 0x0000, 0x800, CRC(8d9b957d) SHA1(9d895c5977a3f405130594a10d530a82a6aa265f) )

	ROM_REGION( 0x0600, "proms", 0 )    /* microcode for growth renderer */
	ROM_LOAD( "136089-1001.20p",  0x0000, 0x0200, CRC(5836cb5a) SHA1(2c797f6a1227d6e1fd7a12f99f0254072c8c266e) )
	ROM_LOAD( "136089-1002.22p",  0x0200, 0x0200, CRC(44288753) SHA1(811582015264f85a32643196cdb331a41430318f) )
	ROM_LOAD( "136089-1003.21p",  0x0400, 0x0200, CRC(1f571706) SHA1(26d5ea59163b3482ab1f8a26178d0849c5fd9692) )

	ROM_REGION( 0x10000, "commcpu", 0 ) // ADSP-2105 code for communications with another PCB
	ROM_LOAD( "136087-1025.4e", 0x0000, 0x10000, CRC(4c645933) SHA1(7a1cf049e368059a79b03598de73c30d8dae5e90) )
ROM_END

ROM_START( roadriotb ) // Test mode shows only COMM RAM test; This is a dedicated twin version.
	ROM_REGION( 0x80004, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "136089-2014.8d", 0x00000, 0x20000, CRC(bf8aaafc) SHA1(1594d91b56609d49921c866d8f5796619e79217b) ) /* Program ROMs in Blue labels,  */
	ROM_LOAD16_BYTE( "136089-2013.8c", 0x00001, 0x20000, CRC(5dd2dd70) SHA1(8f6a0e809ec1f6feea8a18197a789086a7b9dd6a) ) /* other ROMs in Yellow labels   */
	ROM_LOAD16_BYTE( "136089-2016.9d", 0x40000, 0x20000, CRC(6191653c) SHA1(97d1a84a585149e8f2c49cab7af22dc755dff350) ) /* PALs & BPROMs in White labels */
	ROM_LOAD16_BYTE( "136089-2015.9c", 0x40001, 0x20000, CRC(0d34419a) SHA1(f16e9fb4cd537d727611cb7dd5537c030671fe1e) )

	ROM_REGION( 0x2000, "asic65:asic65cpu", 0 )   /* ASIC65 TMS32015 code */
	ROM_LOAD( "136089-1012.3f", 0x00000, 0x0a80, CRC(7c5498e7) SHA1(9d8b235baf7b75bef8ef9b168647c5b2b80b2cb3) )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) /* 6502 code */
	ROM_LOAD( "136089-1047.12c", 0x00000, 0x10000, CRC(849dd26c) SHA1(05a0b2a5f7ee4437448b5f076d3066d96dec2320) )

	ROM_REGION( 0xc0000, "tiles", 0 )
	ROM_LOAD( "136089-1041.22d",    0x000000, 0x20000, CRC(b7451f92) SHA1(9fd17913630e457e406e596f2d86afff98787750) ) /* playfield, planes 0-1 */
	ROM_LOAD( "136089-1038.22c",    0x020000, 0x20000, CRC(90f3c6ee) SHA1(7607509e2d3b2080a918cfaf2879dbed6b79d029) )
	ROM_LOAD( "136089-1037.2021d",  0x040000, 0x20000, CRC(d40de62b) SHA1(fa6dfd20bdad7874ae33a1027a9bb0ea200f86ca) ) /* playfield, planes 2-3 */
	ROM_LOAD( "136089-1039.2021c",  0x060000, 0x20000, CRC(a7e836b1) SHA1(d41f1e4166ca757176c6976be2a953db5db05e48) )
	ROM_LOAD( "136089-1040.20d",    0x080000, 0x20000, CRC(a81ae93f) SHA1(b694ba5fab35f8fa505a02039ae62f7af3c7ae1d) ) /* playfield, planes 4-5 */
	ROM_LOAD( "136089-1042.20c",    0x0a0000, 0x20000, CRC(b8a6d15a) SHA1(43d2be9d40a84b2c01d80bbcac737eda04d55999) )

	ROM_REGION( 0x020000, "chars", 0 )
	ROM_LOAD( "136089-1046.22j",    0x000000, 0x20000, CRC(0005bab0) SHA1(257e1b23eea117fe6701a67134b96d9d9fe10caf) ) /* alphanumerics */

	ROM_REGION16_BE( 0x200000, "rle", 0 )
	ROM_LOAD16_BYTE( "136089-1018.2s", 0x000000, 0x20000, CRC(19590a94) SHA1(e375b7e01a8b1f366bb4e7750e33f0b6d9ae2042) )
	ROM_LOAD16_BYTE( "136089-1017.2p", 0x000001, 0x20000, CRC(c2bf3f69) SHA1(f822359070b1907973ee7ee35469f4a59f720830) )
	ROM_LOAD16_BYTE( "136089-1020.3s", 0x040000, 0x20000, CRC(bab110e4) SHA1(0c4e3521474249517e7832df1bc63aca6d6a6c91) )
	ROM_LOAD16_BYTE( "136089-1019.3p", 0x040001, 0x20000, CRC(791ad2c5) SHA1(4ef218fbf38a9c6b58c86f203843988df1c935f6) )
	ROM_LOAD16_BYTE( "136089-1022.4s", 0x080000, 0x20000, CRC(79cba484) SHA1(ce361a432f1fe627086bab3c1131118fd15056f1) )
	ROM_LOAD16_BYTE( "136089-1021.4p", 0x080001, 0x20000, CRC(86a2e257) SHA1(98d95d2e67fecc332f6c66358a1f8d58b168c74b) )
	ROM_LOAD16_BYTE( "136089-1024.5s", 0x0c0000, 0x20000, CRC(67d28478) SHA1(cfc9da6d20c65d11c2a59a38660a8da4d1cc219d) )
	ROM_LOAD16_BYTE( "136089-1023.5p", 0x0c0001, 0x20000, CRC(74638838) SHA1(bea0fb21ccb946e023c88791ce5a8dd92b44baec) )
	ROM_LOAD16_BYTE( "136089-1026.6s", 0x100000, 0x20000, CRC(24933c37) SHA1(516393aae51fc9634a5c6d5134be058d6067e114) )
	ROM_LOAD16_BYTE( "136089-1025.6p", 0x100001, 0x20000, CRC(054a163b) SHA1(1b0b129c093398bc5c14b3fdd87dfe149f555fac) )
	ROM_LOAD16_BYTE( "136089-1028.7s", 0x140000, 0x20000, CRC(31ff090a) SHA1(7b43ed37901c3f94cae90c84b3c8c689d7b64dd6) )
	ROM_LOAD16_BYTE( "136089-1027.7p", 0x140001, 0x20000, CRC(bbe5b69b) SHA1(9eaa551fba763824d36fc41bfe0e6d735a9e68c5) )
	ROM_LOAD16_BYTE( "136089-1030.8s", 0x180000, 0x20000, CRC(6c89d2c5) SHA1(0bf2990ce1cd5ec5b84f7e3171725540e6238408) )
	ROM_LOAD16_BYTE( "136089-1029.8p", 0x180001, 0x20000, CRC(40d9bde5) SHA1(aca6e07ea96e4618412d32fe4d4cd293ae82d940) )
	ROM_LOAD16_BYTE( "136089-1032.9s", 0x1c0000, 0x20000, CRC(eca3c595) SHA1(5d067b7c02675b1e6dd3c4046697a16f873f80af) )
	ROM_LOAD16_BYTE( "136089-1031.9p", 0x1c0001, 0x20000, CRC(88acdb53) SHA1(5bf2424ee75a25248a8ce38c8605b6780da4e323) )

	ROM_REGION( 0x80000, "jsa:oki1", 0 )
	ROM_LOAD( "136089-1048.19e",  0x00000, 0x20000, CRC(2db638a7) SHA1(45da8088f7439beacc3056952a4d631d9633efa7) )
	ROM_LOAD( "136089-1049.17e",  0x20000, 0x20000, CRC(e1dd7f9e) SHA1(6b9a240aa84d210d3052daab6ea26f9cd0e62dc1) )
	ROM_LOAD( "136089-1050.15e",  0x40000, 0x20000, CRC(64d410bb) SHA1(877bccca7ff37a9dd8294bc1453487a2f516ca7d) )
	ROM_LOAD( "136089-1051.12e",  0x60000, 0x20000, CRC(bffd01c8) SHA1(f6de000f61ea0c1ddb31ee5301506e5e966638c2) )

	ROM_REGION( 0x800, "eeprom", 0 )
	ROM_LOAD( "roadriot-eeprom.5c", 0x0000, 0x800, CRC(8d9b957d) SHA1(9d895c5977a3f405130594a10d530a82a6aa265f) )

	ROM_REGION( 0x0600, "proms", 0 )    /* microcode for growth renderer */
	ROM_LOAD( "136089-1001.20p",  0x0000, 0x0200, CRC(5836cb5a) SHA1(2c797f6a1227d6e1fd7a12f99f0254072c8c266e) )
	ROM_LOAD( "136089-1002.22p",  0x0200, 0x0200, CRC(44288753) SHA1(811582015264f85a32643196cdb331a41430318f) )
	ROM_LOAD( "136089-1003.21p",  0x0400, 0x0200, CRC(1f571706) SHA1(26d5ea59163b3482ab1f8a26178d0849c5fd9692) )
ROM_END


ROM_START( dangerex )
	ROM_REGION( 0x80004, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "dx8d-0h.8d", 0x00000, 0x20000, CRC(4957b65d) SHA1(de9f187b6496cf96d29c4b1b29887abc2bdf9bf0) )
	ROM_LOAD16_BYTE( "dx8c-0l.8c", 0x00001, 0x20000, CRC(aedcb497) SHA1(7e201b7db5c0ff661f782566a6b17299d514c77a) )
	ROM_LOAD16_BYTE( "dx9d-1h.9d", 0x40000, 0x20000, CRC(2eb943e2) SHA1(87dbf11720e2938bf5755b13231fc668ab3e0e05) )
	ROM_LOAD16_BYTE( "dx9c-1l.9c", 0x40001, 0x20000, CRC(79de4c91) SHA1(31de5e927aff4efcf4217da3c704ece2d393faf9) )

	ROM_REGION( 0x2000, "asic65:asic65cpu", 0 )   /* ASIC65 TMS32015 code */
	ROM_LOAD( "136089-1012.3f", 0x00000, 0x0a80, NO_DUMP )

	ROM_REGION( 0x14000, "jsa:cpu", 0 ) /* 6502 code */
	ROM_LOAD( "dx12c-5.12c", 0x10000, 0x4000, CRC(d72621f7) SHA1(4bf5c98dd2434cc6ed1bddb6baf42f41cf138e1a) )
	ROM_CONTINUE(            0x04000, 0xc000 )

	ROM_REGION( 0xc0000, "tiles", 0 )
	ROM_LOAD( "dxc117-22d.22d",    0x000000, 0x20000, CRC(5532995a) SHA1(21e001c911adb91dbe43e895ae8582df65f2995d) ) /* playfield, planes 0-1 */
	ROM_LOAD( "dx82-22c.22c",      0x020000, 0x20000, CRC(9548599b) SHA1(d08bae8dabce0175f956631ddfbf091653af035e) )
	ROM_LOAD( "dxc116-20-21d.21d", 0x040000, 0x20000, CRC(ebbf0fd8) SHA1(4ceb026c4231b675215110c16c8f75551cdfa461) ) /* playfield, planes 2-3 */
	ROM_LOAD( "dxc81-22-21c.21c",  0x060000, 0x20000, CRC(24cb1d34) SHA1(4fc558c8dee3654abd164e5e4adddf9bfcbca3ae) )
	ROM_LOAD( "dxc115-20d.20d",    0x080000, 0x20000, CRC(2819ce54) SHA1(9a3c041d9046af41997dc1d9f41bf0e1be9489f9) ) /* playfield, planes 4-5 */
	ROM_LOAD( "dxc80-20c.20c",     0x0a0000, 0x20000, CRC(a8ffe459) SHA1(92a10694c38a4fbe3022662f4e8e4e214aab31c9) )

	ROM_REGION( 0x020000, "chars", 0 )
	ROM_LOAD( "dxc187-22j.22j",   0x000000, 0x20000, CRC(7231ecc2) SHA1(8b1b0aed3a0d907630e120395b0a97fd9a1ef8cc) ) /* alphanumerics */

	ROM_REGION16_BE( 0x800000, "rle", 0 )
	ROM_LOAD16_BYTE( "dx2s-0h.2s", 0x000000, 0x80000, CRC(89902ce2) SHA1(f910ad65f3780e28c9920c4185c1ea807ec478aa) )
	ROM_LOAD16_BYTE( "dx2p-0l.2p", 0x000001, 0x80000, CRC(dabe7e1c) SHA1(1f77bba57b7025333c27ee3d548d08ee960d63d6) )
	ROM_LOAD16_BYTE( "dx3s-1h.3s", 0x100000, 0x80000, CRC(ffeec3d1) SHA1(de40083ce3862f2b5d37f5f255f93b2b2487ed96) )
	ROM_LOAD16_BYTE( "dx3p-1l.3p", 0x100001, 0x80000, CRC(40b0a300) SHA1(3ec055bdc30e62c5e95541b15c53f1d439ccb5b4) )
	ROM_LOAD16_BYTE( "dx4s-2h.4s", 0x200000, 0x80000, CRC(1e4d0c50) SHA1(fbb5422f43e1c4f8787073c1cdaeb75121a67a35) )
	ROM_LOAD16_BYTE( "dx4p-2l.4p", 0x200001, 0x80000, CRC(00d586e1) SHA1(7e8419a5972a5e0fc372c72b5c5f8f3ff4294a2b) )
	ROM_LOAD16_BYTE( "dx5s-3h.5s", 0x300000, 0x80000, CRC(98e26315) SHA1(dbc567c3b9d00f827acb8ba2d5a79a7d2acc7ddd) )
	ROM_LOAD16_BYTE( "dx5p-3l.5p", 0x300001, 0x80000, CRC(e37b1413) SHA1(490911006f0e10319117dae3a87623d8244ea182) )
	ROM_LOAD16_BYTE( "dx6s-4h.6s", 0x400000, 0x80000, CRC(b2bc538c) SHA1(1bda4a6c40c9389573857879263c0f01b5f029d3) )
	ROM_LOAD16_BYTE( "dx6p-4l.6p", 0x400001, 0x80000, CRC(5e3aefb8) SHA1(492eb7f42bbeefbc377d3a491fe07b64c1c8a11c) )
	ROM_LOAD16_BYTE( "dx7s-5h.7s", 0x500000, 0x80000, CRC(396d706e) SHA1(7682287df2ad2283b5999c3df272baa9559b7bd5) )
	ROM_LOAD16_BYTE( "dx7p-5l.7p", 0x500001, 0x80000, CRC(102c827d) SHA1(6ae405032a07081841a5a3fb48422bd67d37372c) )
	ROM_LOAD16_BYTE( "dx8s-6h.8s", 0x600000, 0x80000, CRC(af3b1f90) SHA1(d514a7b5e9bb263bbb1164a5174fa6943c4e5fb4) )
	ROM_LOAD16_BYTE( "dx8p-6l.8p", 0x600001, 0x80000, CRC(a0e7311b) SHA1(9c4d443c727c3bd59f035bf27c4f7e74046d4c45) )
	ROM_LOAD16_BYTE( "dx9s-7h.7h", 0x700000, 0x80000, CRC(5bf0ce01) SHA1(22f971842371eb36b2dc6ae303ef3955dd9884c2) )
	ROM_LOAD16_BYTE( "dx9p-7l.7l", 0x700001, 0x80000, CRC(e6f1d9fa) SHA1(160b4c9a90bdc48c990e5d4a24b17a284c9b4da8) )

	ROM_REGION( 0x100000, "jsa:oki1", 0 )
	ROM_LOAD( "dx19e-0.19e",  0x00000, 0x20000, CRC(e7d9d5fb) SHA1(756b1d59168855a707181dd6c437a59313da5a7c) )
	ROM_LOAD( "dx17e-1.17e",  0x20000, 0x20000, CRC(ccb73a18) SHA1(3e853f7c7ab32b18fdb6529d37d28eb96c5365dc) )
	ROM_LOAD( "dx15e-2.15e",  0x40000, 0x20000, CRC(11596234) SHA1(77eab7cb4ad83a50c23127b4fb1bbfd4aa2c6f8d) )
	ROM_LOAD( "dx12e-3.12e",  0x60000, 0x20000, CRC(c0ffd43c) SHA1(dcd7e3cc5d46db0d0a7fe3806bddbca235492d35) )

	ROM_REGION( 0x800, "eeprom", 0 )
	ROM_LOAD( "dx-eeprom.5c", 0x0000, 0x800, CRC(d14e813d) SHA1(b206ce85f7a87986d401d34eafa188b5bffae08c) )

	ROM_REGION( 0x0600, "proms", 0 )    /* microcode for growth renderer */
	ROM_LOAD( "092-1001.20p",  0x0000, 0x0200, NO_DUMP )
	ROM_LOAD( "092-1002.22p",  0x0200, 0x0200, NO_DUMP )
	ROM_LOAD( "092-1003.21p",  0x0400, 0x0200, NO_DUMP )
ROM_END


ROM_START( guardian )
	ROM_REGION( 0x80004, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "136092-2021.8e",  0x00000, 0x20000, CRC(efea1e02) SHA1(f0f1ef300f36953aff73b68ffe6d9950ac575f7d) )
	ROM_LOAD16_BYTE( "136092-2020.8cd", 0x00001, 0x20000, CRC(a8f655ba) SHA1(2defe4d138613e248718a617d103794e051746f7) )
	ROM_LOAD16_BYTE( "136092-2023.9e",  0x40000, 0x20000, CRC(cfa29316) SHA1(4e0e76304e29ee59bc2ce9a704e3f651dc9d473c) )
	ROM_LOAD16_BYTE( "136092-2022.9cd", 0x40001, 0x20000, CRC(ed2abc91) SHA1(81531040d5663f6ab82e924210056e3737e17a8d) )

	ROM_REGION( 0x2000, "asic65:asic65cpu", 0 )   /* ASIC65 TMS32015 code */
	ROM_LOAD( "136089-1012.3f", 0x00000, 0x0a80, NO_DUMP )

	ROM_REGION( 0x10000, "jsa:cpu", 0 ) /* 6502 code */
	ROM_LOAD( "136092-0080-snd.12c", 0x00000, 0x10000, CRC(0388f805) SHA1(49c11313bc4192dbe294cf68b652cb19047889fd) )

	ROM_REGION( 0x180000, "tiles", 0 )
	ROM_LOAD( "136092-0037a.23e",  0x000000, 0x80000, CRC(ca10b63e) SHA1(243a2a440e1bc9135d3dbe6553d39c54b9bdcd13) ) /* playfield, planes 0-1 */
	ROM_LOAD( "136092-0038a.22e",  0x080000, 0x80000, CRC(cb1431a1) SHA1(d7b8f49a1e794ca2083e4bf0fa3870ce08caa53a) ) /* playfield, planes 2-3 */
	ROM_LOAD( "136092-0039a.20e",  0x100000, 0x80000, CRC(2eee7188) SHA1(d3adbd7b20bc898fee35b6ba781e7775f82acd19) ) /* playfield, planes 4-5 */

	ROM_REGION( 0x020000, "chars", 0 )
	ROM_LOAD( "136092-0030.23k",   0x000000, 0x20000, CRC(0fd7baa1) SHA1(7802d732e5173291628ed498ad0fab71aeef4688) ) /* alphanumerics */

	ROM_REGION16_BE( 0x600000, "rle", 0 )
	ROM_LOAD16_BYTE( "136092-0041.2s",  0x000000, 0x80000, CRC(a2a5ae08) SHA1(d99f925bbc9a72432e13328ee8422fde615db90f) )
	ROM_LOAD16_BYTE( "136092-0040.2p",  0x000001, 0x80000, CRC(ef95132e) SHA1(288de1d15956a612b7d19ceb2cf853490bf42b05) )
	ROM_LOAD16_BYTE( "136092-0043.3s",  0x100000, 0x80000, CRC(6438b8e4) SHA1(ee1446209fbcab8b17c88c53b65e754a85f279d1) )
	ROM_LOAD16_BYTE( "136092-0042.3p",  0x100001, 0x80000, CRC(46bf7c0d) SHA1(12414de2698178b158ec4ca0fbb176943c944cec) )
	ROM_LOAD16_BYTE( "136092-0045.4s",  0x200000, 0x80000, CRC(4f4f2bee) SHA1(8276cdcd252d2d8fa41ab28e76a6bd72613c14ec) )
	ROM_LOAD16_BYTE( "136092-0044.4p",  0x200001, 0x80000, CRC(20a4250b) SHA1(6a2e2596a9eef2792f7cdab648dd455b8e420a74) )
	ROM_LOAD16_BYTE( "136092-0063a.6s", 0x300000, 0x80000, CRC(93933bcf) SHA1(a67d4839ffdb0eafbc2d68a60fb3bf1507c793cf) )
	ROM_LOAD16_BYTE( "136092-0062a.6p", 0x300001, 0x80000, CRC(613e6f1d) SHA1(fd2ea18d245d0895e0bac6c5caa6d35fdd6a199f) )
	ROM_LOAD16_BYTE( "136092-0065a.7s", 0x400000, 0x80000, CRC(6bcd1205) SHA1(c883c55f88d274ba8aa48c962406b253e1f8001e) )
	ROM_LOAD16_BYTE( "136092-0064a.7p", 0x400001, 0x80000, CRC(7b4dce05) SHA1(36545917388e704f73a9b4d85189ec978d655b63) )
	ROM_LOAD16_BYTE( "136092-0067a.9s", 0x500000, 0x80000, CRC(15845fba) SHA1(f7b670a8d48a5e9c261150914a06ef9a938a84e7) )
	ROM_LOAD16_BYTE( "136092-0066a.9p", 0x500001, 0x80000, CRC(7130c575) SHA1(b3ea109981a1e5c631705b23dfad4a3a3daf7734) )

	ROM_REGION( 0x80000, "jsa:oki1", 0 )
	ROM_LOAD( "136092-0010-snd.19e",  0x00000, 0x80000, CRC(bca27f40) SHA1(91a41eac116eb7d9a790abc590eb06328726d1c2) )

	ROM_REGION( 0x800, "eeprom", 0 )
	ROM_LOAD( "guardian-eeprom.5c", 0x0000, 0x800, CRC(85835fab) SHA1(747e2851c8baa0e7f1c0784b0d6900514230ab07) )

	ROM_REGION( 0x0600, "proms", 0 )    /* microcode for growth renderer */
	ROM_LOAD( "136092-1001.20p",  0x0000, 0x0200, CRC(b3251eeb) SHA1(5e83baa70aaa28f07f32657bf974fd87719972d3) )
	ROM_LOAD( "136092-1002.22p",  0x0200, 0x0200, CRC(0c5314da) SHA1(a9c7ee3ab015c7f3ada4200acd2854eb9a5c74b0) )
	ROM_LOAD( "136092-1003.21p",  0x0400, 0x0200, CRC(344b406a) SHA1(f4422f8c0d7004d0277a4fc77718d555f80fcf69) )

	ROM_REGION( 0x1500, "plds", 0 )
	ROM_LOAD( "gal16v8a.3l",  0x0000, 0x0117, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "gal16v8a.7c",  0x0200, 0x0117, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "gal16v8a.13s", 0x0400, 0x0117, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "gal16v8a.16j", 0x0600, 0x0117, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "gal16v8a.17c", 0x0800, 0x0117, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "gal16v8a.18k", 0x0a00, 0x0117, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "gal16v8a.20c", 0x0c00, 0x0117, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "gal16v8a.22l", 0x0e00, 0x0117, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "gal6001b.15h", 0x1000, 0x0410, NO_DUMP ) /* PAL is read protected */
ROM_END



/*************************************
 *
 *  Driver initialization
 *
 *************************************/

void atarig42_0x200_state::init_roadriot()
{
	m_playfield_base = 0x400;

	address_space &main = m_maincpu->space(AS_PROGRAM);
	main.install_readwrite_handler(0x000000, 0x07ffff, read16sm_delegate(*this, FUNC(atarig42_0x200_state::roadriot_sloop_data_r)), write16sm_delegate(*this, FUNC(atarig42_0x200_state::roadriot_sloop_data_w)));
	m_sloop_base = (uint16_t *)memregion("maincpu")->base();

	/*
	Road Riot color MUX

	CRA10=!MGEP*!AN.VID7*AN.0               -- if (mopri < pfpri) && (!alpha)
	   +!AN.VID7*AN.0*MO.0                  or if (mopix == 0) && (!alpha)

	CRA9=MGEP*!AN.VID7*AN.0*!MO.0           -- if (mopri >= pfpri) && (mopix != 0) && (!alpha)
	   +!AN.VID7*AN.0*PF.VID9               or if (pfpix & 0x200) && (!alpha)

	CRA8=MGEP*!AN.VID7*AN.0*!MO.0*MVID8     -- if (mopri >= pfpri) && (mopix != 0) && (mopix & 0x100) && (!alpha)
	   +!MGEP*!AN.VID7*AN.0*PF.VID8         or if (mopri < pfpri) && (pfpix & 0x100) && (!alpha)
	   +!AN.VID7*AN.0*MO.0*PF.VID8          or if (pfpix & 0x100) && (!alpha)

	CRMUXB=!AN.VID7*AN.0                    -- if (!alpha)

	CRMUXA=!MGEP                            -- if (mopri < pfpri)
	   +MO.0                                or (mopix == 0)
	   +AN.VID7                             or (alpha)
	   +!AN.0
	*/
}


void atarig42_0x400_state::init_dangerex()
{
	m_playfield_base = 0x000;
}


void atarig42_0x400_state::init_guardian()
{
	m_playfield_base = 0x000;

	/* it looks like they jsr to $80000 as some kind of protection */
	/* put an RTS there so we don't die */
	*(uint16_t *)&memregion("maincpu")->base()[0x80000] = 0x4E75;

	address_space &main = m_maincpu->space(AS_PROGRAM);
	main.install_readwrite_handler(0x000000, 0x07ffff, read16sm_delegate(*this, FUNC(atarig42_0x400_state::guardians_sloop_data_r)), write16sm_delegate(*this, FUNC(atarig42_0x400_state::guardians_sloop_data_w)));
	m_sloop_base = (uint16_t *)memregion("maincpu")->base();

	/*
	Guardians color MUX

	CRA10=MGEP*!AN.VID7*AN.0*!MO.0          -- if (mopri >= pfpri) && (!alpha) && (mopix != 0)

	CRA9=MGEP*!AN.VID7*AN.0*!MO.0*MVID9     -- if (mopri >= pfpri) && (!alpha) && (mopix != 0) && (mopix & 0x200)
	   +!MGEP*!AN.VID7*AN.0*PF.VID9         or if (mopri < pfpri) && (!alpha) && (pfpix & 0x200)
	   +!AN.VID7*AN.0*MO.0*PF.VID9          or if (mopix == 0) && (!alpha) && (pfpix & 0x200)

	CRA8=MGEP*!AN.VID7*AN.0*!MO.0*MVID8     -- if (mopri >= pfpri) && (!alpha) && (mopix != 0) && (mopix & 0x100)
	   +!MGEP*!AN.VID7*AN.0*PF.VID8         or if (mopri < pfpri) && (!alpha) && (pfpix & 0x100)
	   +!AN.VID7*AN.0*MO.0*PF.VID8          or if (mopix == 0) && (!alpha) && (pfpix & 0x100)

	CRMUXB=!AN.VID7*AN.0                    -- if (!alpha)

	CRMUXA=!MGEP                            -- if (mopri < pfpri)
	   +MO.0                                or (mopix == 0)
	   +AN.VID7                             or (alpha)
	   +!AN.0
	*/
}



/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1991, roadriot,  0,        atarig42_0x200, roadriot, atarig42_0x200_state, init_roadriot, ROT0, "Atari Games", "Road Riot 4WD (04 Dec 1991, conversion kit)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NODEVICE_LAN )
GAME( 1991, roadriota, roadriot, atarig42_0x200, roadriot, atarig42_0x200_state, init_roadriot, ROT0, "Atari Games", "Road Riot 4WD (13 Nov 1991, conversion kit)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NODEVICE_LAN )
GAME( 1991, roadriotb, roadriot, atarig42_0x200, roadriot, atarig42_0x200_state, init_roadriot, ROT0, "Atari Games", "Road Riot 4WD (04 Jun 1991, dedicated twin)", MACHINE_UNEMULATED_PROTECTION | MACHINE_NODEVICE_LAN )
GAME( 1992, dangerex,  0,        atarig42_0x400, dangerex, atarig42_0x400_state, init_dangerex, ROT0, "Atari Games", "Danger Express (prototype)", 0 )
GAME( 1992, guardian,  0,        atarig42_0x400, guardian, atarig42_0x400_state, init_guardian, ROT0, "Atari Games", "Guardians of the 'Hood", 0 )
