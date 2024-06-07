// license:BSD-3-Clause
// copyright-holders:Aaron Giles,Nicola Salmoria
/***************************************************************************

    AmeriDarts      (c) 1989 Ameri Corporation
    Cool Pool       (c) 1992 Catalina
    9 Ball Shootout (c) 1993 E-Scape/Bundra

    driver by Nicola Salmoria and Aaron Giles


    The main CPU is a TMS34010; it is encrypted in 9 Ball Shootout.

    The second CPU in AmeriDarts is a TMS32015; it controls sound and
    the trackball inputs.

    The second CPU in Cool Pool and 9 Ball Shootout is a TMS320C26; the code
    is the same in the two games.

    The audio DAC is a Micro Power Systems MP1210HN.

    Cool Pool:
    - The checksum test routine is wrong, e.g. when it says to be testing
      4U/8U it is actually reading 4U/8U/3U/7U, when testing 3U/7U it
      actually reads 2U/6U/1U/5U. The placement cannot therefore be exactly
      determined by the check passing.

***************************************************************************/

#include "emu.h"
#include "coolpool.h"

#include "cpu/tms32010/tms32010.h"
#include "cpu/tms32025/tms32025.h"
#include "machine/nvram.h"
#include "sound/dac.h"
#include "screen.h"
#include "speaker.h"

#include <algorithm>

#define LOG_PORT    (1U << 1)
#define LOG_INPUT   (1U << 2)

#define LOG_ALL     (LOG_PORT | LOG_INPUT)

#define VERBOSE (0)
#include "logmacro.h"

#define LOGPORT(...)  LOGMASKED(LOG_PORT,  __VA_ARGS__)
#define LOGINPUT(...) LOGMASKED(LOG_INPUT, __VA_ARGS__)

/*************************************
 *
 *  Local variables
 *
 *************************************/



static const uint16_t nvram_unlock_seq[] =
{
	0x3fb, 0x3fb, 0x3f8, 0x3fc, 0x3fa, 0x3fe, 0x3f9, 0x3fd, 0x3fb, 0x3ff
};


/*************************************
 *
 *  Video updates
 *
 *************************************/

TMS340X0_SCANLINE_RGB32_CB_MEMBER(amerdart_state::scanline)
{
	uint16_t const *const vram = &m_vram_base[(params->rowaddr << 8) & 0xff00];
	uint32_t *const dest = &bitmap.pix(scanline);
	int coladdr = params->coladdr;

	// update the palette
	if (scanline < 256)
		for (int x = 0; x < 16; x++)
		{
			uint16_t const pal = m_vram_base[x];
			m_palette->set_pen_color(x, rgb_t(pal4bit(pal >> 4), pal4bit(pal >> 8), pal4bit(pal >> 12)));
		}

	pen_t const *const pens = m_palette->pens();
	for (int x = params->heblnk; x < params->hsblnk; x += 4)
	{
		uint16_t const pixels = vram[coladdr++ & 0xff];
		dest[x + 0] = pens[(pixels >> 0) & 15];
		dest[x + 1] = pens[(pixels >> 4) & 15];
		dest[x + 2] = pens[(pixels >> 8) & 15];
		dest[x + 3] = pens[(pixels >> 12) & 15];
	}
}


TMS340X0_SCANLINE_RGB32_CB_MEMBER(_9ballsht_state::scanline)
{
	uint16_t const *const vram = &m_vram_base[(params->rowaddr << 8) & 0x1ff00];
	uint32_t *const dest = &bitmap.pix(scanline);
	pen_t const *const pens = m_tlc34076->pens();
	int coladdr = params->coladdr;

	for (int x = params->heblnk; x < params->hsblnk; x += 2)
	{
		uint16_t const pixels = vram[coladdr++ & 0xff];
		dest[x + 0] = pens[pixels & 0xff];
		dest[x + 1] = pens[pixels >> 8];
	}
}



/*************************************
 *
 *  Shift register access
 *
 *************************************/

TMS340X0_TO_SHIFTREG_CB_MEMBER(coolpool_base_state::to_shiftreg)
{
	memcpy(shiftreg, &m_vram_base[(address & ~0xfff) >> 4], 0x200);
}


TMS340X0_FROM_SHIFTREG_CB_MEMBER(coolpool_base_state::from_shiftreg)
{
	memcpy(&m_vram_base[(address & ~0xfff) >> 4], shiftreg, 0x200);
}



/*************************************
 *
 *  Game initialzation
 *
 *************************************/

void coolpool_base_state::machine_start()
{
	// assumes it can make an address mask with m_dsp_rom.length() - 1
	assert(!(m_dsp_rom.length() & (m_dsp_rom.length() - 1)));

	save_item(NAME(m_newx));
	save_item(NAME(m_newy));
	save_item(NAME(m_oldx));
	save_item(NAME(m_oldy));
	save_item(NAME(m_dx));
	save_item(NAME(m_dy));
	save_item(NAME(m_result));
	save_item(NAME(m_lastresult));

	save_item(NAME(m_nvram_write_seq));
	save_item(NAME(m_nvram_write_enable));
	save_item(NAME(m_same_cmd_count));

	save_item(NAME(m_iop_romaddr));
}

void amerdart_state::machine_start()
{
	coolpool_base_state::machine_start();

	m_lastresult = 0xffff;
	save_item(NAME(m_old_cmd));
}


void coolpool_base_state::machine_reset()
{
	m_nvram_write_enable = 0;
}


/*************************************
 *
 *  NVRAM writes with thrash protect
 *
 *************************************/

TIMER_DEVICE_CALLBACK_MEMBER(coolpool_base_state::nvram_write_timeout)
{
	m_nvram_write_enable = 0;
}


void coolpool_base_state::nvram_thrash_w(offs_t offset, uint16_t data)
{
	// keep track of the last few writes
	memmove(&m_nvram_write_seq[0], &m_nvram_write_seq[1], (NVRAM_UNLOCK_SEQ_LEN - 1) * sizeof(m_nvram_write_seq[0]));
	m_nvram_write_seq[NVRAM_UNLOCK_SEQ_LEN - 1] = offset & 0x3ff;

	// if they match the unlock sequence, enable writes and set a timeout
	if (!memcmp(nvram_unlock_seq, m_nvram_write_seq, sizeof(nvram_unlock_seq)))
	{
		m_nvram_write_enable = 1;
		m_nvram_timer->adjust(attotime::from_msec(1000));
	}
}


void coolpool_base_state::nvram_data_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// only the low 8 bits matter
	if (ACCESSING_BITS_0_7)
	{
		if (m_nvram_write_enable)
		{
			m_nvram[offset] = data & 0xff;
		}
	}
}


void coolpool_base_state::nvram_thrash_data_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	nvram_data_w(offset, data, mem_mask);
	nvram_thrash_w(offset, data);
}



/*************************************
 *
 *  AmeriDarts IOP handling
 *
 *************************************/

TIMER_DEVICE_CALLBACK_MEMBER(amerdart_state::amerdart_audio_int_gen)
{
	m_dsp->set_input_line(0, ASSERT_LINE);
	m_dsp->set_input_line(0, CLEAR_LINE);
}


void amerdart_state::misc_w(uint16_t data)
{
	LOGPORT("%08x:IOP_system_w %04x\n",m_maincpu->pc(),data);

	machine().bookkeeping().coin_counter_w(0, BIT(~data, 0));
	machine().bookkeeping().coin_counter_w(1, BIT(~data, 1));

	// bits 10-15 are counted down over time

	m_dsp->set_input_line(INPUT_LINE_RESET, BIT(data, 10) ? ASSERT_LINE : CLEAR_LINE);
}

int amerdart_state::dsp_bio_line_r()
{
	// Skip idle checking
	if (!machine().side_effects_disabled())
	{
		if (m_old_cmd == m_main2dsp->pending_r())
			m_same_cmd_count += 1;
		else
			m_same_cmd_count = 0;

		if (m_same_cmd_count >= 5)
		{
			m_same_cmd_count = 5;
			m_dsp->spin();
		}
		m_old_cmd = m_main2dsp->pending_r();
	}
	return m_main2dsp->pending_r() ? CLEAR_LINE : ASSERT_LINE;
}

/*************************************
 *
 *  Ameri Darts trackball inputs
 *
 *************************************/

static int amerdart_trackball_inc(int data)
{
	switch (data & 0x03)    // Bits of opposite track direction must both change with identical levels
	{
		case 0x00:  data ^= 0x03;   break;
		case 0x01:  data ^= 0x01;   break;
		case 0x02:  data ^= 0x01;   break;
		case 0x03:  data ^= 0x03;   break;
	}
	return data;
}
static int amerdart_trackball_dec(int data)
{
	switch (data & 0x03)    // Bits of opposite track direction must both change with opposing levels
	{
		case 0x00:  data ^= 0x01;   break;
		case 0x01:  data ^= 0x03;   break;
		case 0x02:  data ^= 0x03;   break;
		case 0x03:  data ^= 0x01;   break;
	}
	return data;
}

int amerdart_state::amerdart_trackball_direction(int num, int data)
{
	uint16_t result_x = (data & 0x0c) >> 2;
	uint16_t result_y = (data & 0x03) >> 0;

	if ((m_dx[num] == 0) && (m_dy[num] < 0))        // Up
	{
		if (!machine().side_effects_disabled())
			m_oldy[num]--;
		result_x = amerdart_trackball_inc(result_x);
		result_y = amerdart_trackball_inc(result_y);
	}
	if ((m_dx[num] == 0) && (m_dy[num] > 0))        // Down
	{
		if (!machine().side_effects_disabled())
			m_oldy[num]++;
		result_x = amerdart_trackball_dec(result_x);
		result_y = amerdart_trackball_dec(result_y);
	}
	if ((m_dx[num] < 0) && (m_dy[num] == 0))        // Left
	{
		if (!machine().side_effects_disabled())
			m_oldx[num]--;
		result_x = amerdart_trackball_inc(result_x);
		result_y = amerdart_trackball_dec(result_y);
	}
	if ((m_dx[num] > 0) && (m_dy[num] == 0))        // Right
	{
		if (!machine().side_effects_disabled())
			m_oldx[num]++;
		result_x = amerdart_trackball_dec(result_x);
		result_y = amerdart_trackball_inc(result_y);
	}
	if ((m_dx[num] < 0) && (m_dy[num] < 0))         // Left & Up
	{
		if (!machine().side_effects_disabled())
		{
			m_oldx[num]--;
			m_oldy[num]--;
		}
		result_x = amerdart_trackball_inc(result_x);
	}
	if ((m_dx[num] < 0) && (m_dy[num] > 0))         // Left & Down
	{
		if (!machine().side_effects_disabled())
		{
			m_oldx[num]--;
			m_oldy[num]++;
		}
		result_y = amerdart_trackball_dec(result_y);
	}
	if ((m_dx[num] > 0) && (m_dy[num] < 0))         // Right & Up
	{
		if (!machine().side_effects_disabled())
		{
			m_oldx[num]++;
			m_oldy[num]--;
		}
		result_y = amerdart_trackball_inc(result_y);
	}
	if ((m_dx[num] > 0) && (m_dy[num] > 0))         // Right & Down
	{
		if (!machine().side_effects_disabled())
		{
			m_oldx[num]++;
			m_oldy[num]++;
		}
		result_x = amerdart_trackball_dec(result_x);
	}

	data = ((result_x << 2) & 0x0c) | ((result_y << 0) & 0x03);

	return data;
}


uint16_t amerdart_state::amerdart_trackball_r(offs_t offset)
{
/*
    Trackballs seem to be handled as though they're rotated 45 degrees anti-clockwise.

    Sensor data as shown on Input test screen and associated bits read by TMS32015 DSP port:
    xxyy xxyy ???? ????
    |||| |||| |||| ||||
    |||| |||| ++++-++++-- Unused
    |||| ||||
    |||| |||+------------ Trackball 1 Up    sensor
    |||| ||+------------- Trackball 1 Down  sensor
    |||| |+-------------- Trackball 1 Left  sensor
    |||| +--------------- Trackball 1 Right sensor
    ||||
    |||+----------------- Trackball 2 Up    sensor
    ||+------------------ Trackball 2 Down  sensor
    |+------------------- Trackball 2 Left  sensor
    +-------------------- Trackball 2 Right sensor

    Opposite direction bits toggling the   same   level indicate increment (+) input movement  (00 -> 11 -> 00, etc)
    Opposite direction bits toggling the opposite level indicate decrement (-) input movement  (01 -> 10 -> 01, etc)

    Input state requirements to indicate trackball direction.
    Direction      StateX  StateY
    =============================
    UP              X +     Y +
    Down            X -     Y -
    Left            X +     Y -
    Right           X -     Y +
    Up   & Left     X +     Y 0
    Up   & Right    X 0     Y +
    Down & Left     X 0     Y -
    Down & Right    X -     Y 0

*/

	if (!machine().side_effects_disabled())
	{
		m_result = (m_lastresult | 0x00ff);

		m_newx[1] = m_in_xaxis[0]->read();   // Trackball 1  Left - Right
		m_newy[1] = m_in_yaxis[0]->read();   // Trackball 1   Up  - Down
		m_newx[2] = m_in_xaxis[1]->read();   // Trackball 2  Left - Right
		m_newy[2] = m_in_yaxis[1]->read();   // Trackball 2   Up  - Down

		m_dx[1] = (int8_t)(m_newx[1] - m_oldx[1]);
		m_dy[1] = (int8_t)(m_newy[1] - m_oldy[1]);
		m_dx[2] = (int8_t)(m_newx[2] - m_oldx[2]);
		m_dy[2] = (int8_t)(m_newy[2] - m_oldy[2]);

		// Determine Trackball 1 direction state
		m_result = (m_result & 0xf0ff) | (amerdart_trackball_direction(1, ((m_result >>  8) & 0xf)) <<  8);

		// Determine Trackball 2 direction state
		m_result = (m_result & 0x0fff) | (amerdart_trackball_direction(2, ((m_result >> 12) & 0xf)) << 12);


//  LOGINPUT("%08X:read port 6 (X=%02X Y=%02X oldX=%02X oldY=%02X oldRes=%04X Res=%04X)\n", m_dsp->pc(), m_newx, m_newy, m_oldx, m_oldy, m_lastresult, m_result);

		m_lastresult = m_result;
	}

	return m_result;
}


/*************************************
 *
 *  Cool Pool IOP control
 *
 *************************************/

void _9ballsht_state::misc_w(uint16_t data)
{
	LOGPORT("%08x:IOP_system_w %04x\n",m_maincpu->pc(),data);

	machine().bookkeeping().coin_counter_w(0, BIT(~data, 0));
	machine().bookkeeping().coin_counter_w(1, BIT(~data, 1));

	m_dsp->set_input_line(INPUT_LINE_RESET, BIT(data, 10) ? ASSERT_LINE : CLEAR_LINE);
}


/*************************************
 *
 *  Cool Pool IOP communications
 *  (from IOP side)
 *
 *************************************/

uint16_t _9ballsht_state::dsp_bio_line_r()
{
	return m_main2dsp->pending_r() ? CLEAR_LINE : ASSERT_LINE;
}


uint16_t _9ballsht_state::dsp_hold_line_r()
{
	return CLEAR_LINE;  // ???
}


/*************************************
 *
 *  IOP ROM and DAC access
 *
 *************************************/

uint16_t coolpool_base_state::dsp_rom_r()
{
	return m_dsp_rom[m_iop_romaddr & (m_dsp_rom.length() - 1)];
}


void coolpool_base_state::dsp_romaddr_w(offs_t offset, uint16_t data)
{
	switch (offset)
	{
		case 0:
			m_iop_romaddr = (m_iop_romaddr & 0xffff00) | (data >> 8);
			break;

		case 1:
			m_iop_romaddr = (m_iop_romaddr & 0x0000ff) | (data << 8);
			break;
	}
}



/*************************************
 *
 *  Cool Pool trackball inputs
 *
 *************************************/

uint16_t coolpool_state::coolpool_input_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		m_result = (m_in1->read() & 0x00ff) | (m_lastresult & 0xff00);
		m_newx[1] = m_xaxis->read();
		m_newy[1] = m_yaxis->read();
		m_dx[1] = (int8_t)(m_newx[1] - m_oldx[1]);
		m_dy[1] = (int8_t)(m_newy[1] - m_oldy[1]);

		if (m_dx[1] < 0)
		{
			m_oldx[1]--;
			switch (m_result & 0x300)
			{
				case 0x000: m_result ^= 0x200;  break;
				case 0x100: m_result ^= 0x100;  break;
				case 0x200: m_result ^= 0x100;  break;
				case 0x300: m_result ^= 0x200;  break;
			}
		}
		if (m_dx[1] > 0)
		{
			m_oldx[1]++;
			switch (m_result & 0x300)
			{
				case 0x000: m_result ^= 0x100;  break;
				case 0x100: m_result ^= 0x200;  break;
				case 0x200: m_result ^= 0x200;  break;
				case 0x300: m_result ^= 0x100;  break;
			}
		}

		if (m_dy[1] < 0)
		{
			m_oldy[1]--;
			switch (m_result & 0xc00)
			{
				case 0x000: m_result ^= 0x800;  break;
				case 0x400: m_result ^= 0x400;  break;
				case 0x800: m_result ^= 0x400;  break;
				case 0xc00: m_result ^= 0x800;  break;
			}
		}
		if (m_dy[1] > 0)
		{
			m_oldy[1]++;
			switch (m_result & 0xc00)
			{
				case 0x000: m_result ^= 0x400;  break;
				case 0x400: m_result ^= 0x800;  break;
				case 0x800: m_result ^= 0x800;  break;
				case 0xc00: m_result ^= 0x400;  break;
			}
		}

//  LOGINPUT("%08X:read port 7 (X=%02X Y=%02X oldX=%02X oldY=%02X res=%04X)\n", m_dsp->pc(),
//      m_newx[1], m_newy[1], m_oldx[1], m_oldy[1], m_result);
		m_lastresult = m_result;
	}
	return m_result;
}



/*************************************
 *
 *  Main Memory maps
 *
 *************************************/

void amerdart_state::main_map(address_map &map)
{
	map(0x00000000, 0x000fffff).ram().share(m_vram_base);
	map(0x04000000, 0x0400000f).w(FUNC(amerdart_state::misc_w));
	map(0x05000000, 0x0500000f).r(m_dsp2main, FUNC(generic_latch_16_device::read)).w(m_main2dsp, FUNC(generic_latch_16_device::write));
	map(0x06000000, 0x06007fff).ram().w(FUNC(amerdart_state::nvram_thrash_data_w)).share("nvram");
	map(0xffb00000, 0xffffffff).rom().region("maincpu", 0);
}


void coolpool_state::coolpool_map(address_map &map)
{
	map(0x00000000, 0x001fffff).ram().share(m_vram_base);
	map(0x01000000, 0x010000ff).rw(m_tlc34076, FUNC(tlc34076_device::read), FUNC(tlc34076_device::write)).umask16(0x00ff);    // IMSG176P-40
	map(0x02000000, 0x020000ff).r(m_dsp2main, FUNC(generic_latch_16_device::read)).w(m_main2dsp, FUNC(generic_latch_16_device::write));
	map(0x03000000, 0x0300000f).w(FUNC(coolpool_state::misc_w));
	map(0x03000000, 0x03ffffff).rom().region("maingfx", 0);
	map(0x06000000, 0x06007fff).ram().w(FUNC(coolpool_state::nvram_thrash_data_w)).share("nvram");
	map(0xffe00000, 0xffffffff).rom().region("maincpu", 0);
}


void _9ballsht_state::nballsht_map(address_map &map)
{
	map(0x00000000, 0x001fffff).ram().share(m_vram_base);
	map(0x02000000, 0x020000ff).r(m_dsp2main, FUNC(generic_latch_16_device::read)).w(m_main2dsp, FUNC(generic_latch_16_device::write));
	map(0x03000000, 0x0300000f).w(FUNC(_9ballsht_state::misc_w));
	map(0x04000000, 0x040000ff).rw(m_tlc34076, FUNC(tlc34076_device::read), FUNC(tlc34076_device::write)).umask16(0x00ff);    // IMSG176P-40
	map(0x06000000, 0x0601ffff).mirror(0x00020000).ram().w(FUNC(_9ballsht_state::nvram_thrash_data_w)).share("nvram");
	map(0xff000000, 0xff7fffff).rom().region("maingfx", 0);
	map(0xffc00000, 0xffffffff).rom().region("maincpu", 0);
}



/*************************************
 *
 *  DSP Memory maps
 *
 *************************************/

void amerdart_state::dsp_pgm_map(address_map &map)
{
	map(0x000, 0x0fff).rom();
}
	// 000 - 0FF  TMS32015 Internal Data RAM (256 words) in Data Address Space


void amerdart_state::dsp_io_map(address_map &map)
{
	map(0x00, 0x01).w(FUNC(amerdart_state::dsp_romaddr_w));
	map(0x02, 0x02).w(m_dsp2main, FUNC(generic_latch_16_device::write));
	map(0x03, 0x03).w("dac", FUNC(dac_word_interface::data_w));
	map(0x04, 0x04).r(FUNC(amerdart_state::dsp_rom_r));
	map(0x05, 0x05).portr("IN0");
	map(0x06, 0x06).r(FUNC(amerdart_state::amerdart_trackball_r));
	map(0x07, 0x07).r(m_main2dsp, FUNC(generic_latch_16_device::read));
}


void _9ballsht_state::dsp_pgm_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
}


void _9ballsht_state::dsp_io_base_map(address_map &map)
{
	map(0x00, 0x01).w(FUNC(_9ballsht_state::dsp_romaddr_w));
	map(0x02, 0x02).r(m_main2dsp, FUNC(generic_latch_16_device::read)).w(m_dsp2main, FUNC(generic_latch_16_device::write));
	map(0x03, 0x03).w("dac", FUNC(dac_word_interface::data_w));
	map(0x04, 0x04).r(FUNC(_9ballsht_state::dsp_rom_r));
	map(0x05, 0x05).portr("IN0");
}


void coolpool_state::coolpool_dsp_io_map(address_map &map)
{
	dsp_io_base_map(map);
	map(0x07, 0x07).r(FUNC(coolpool_state::coolpool_input_r));
}


void _9ballsht_state::nballsht_dsp_io_map(address_map &map)
{
	dsp_io_base_map(map);
	map(0x07, 0x07).portr("IN1");
}


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( amerdart )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_SERVICE_NO_TOGGLE( 0x0010, IP_ACTIVE_LOW )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("XAXIS1")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("YAXIS1")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("XAXIS2")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(15) PORT_PLAYER(2)

	PORT_START("YAXIS2")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(15) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( coolpool )
	PORT_START("IN0")
	PORT_BIT( 0x00ff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0700, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 English")
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Lock")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Lock")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 English")
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_SERVICE_NO_TOGGLE( 0x0010, IP_ACTIVE_LOW )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_CUSTOM )

	PORT_START("XAXIS")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)

	PORT_START("YAXIS")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_REVERSE
INPUT_PORTS_END


static INPUT_PORTS_START( 9ballsht )
	PORT_START("IN0")
	PORT_BIT( 0x00ff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0300, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_SERVICE_NO_TOGGLE( 0x0010, IP_ACTIVE_LOW )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
INPUT_PORTS_END


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void amerdart_state::amerdart(machine_config &config)
{
	// basic machine hardware
	TMS34010(config, m_maincpu, XTAL(40'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &amerdart_state::main_map);
	m_maincpu->set_halt_on_reset(false);
	m_maincpu->set_pixel_clock(XTAL(40'000'000)/12);
	m_maincpu->set_pixels_per_clock(2);
	m_maincpu->set_scanline_rgb32_callback(FUNC(amerdart_state::scanline));
	m_maincpu->set_shiftreg_in_callback(FUNC(amerdart_state::to_shiftreg));
	m_maincpu->set_shiftreg_out_callback(FUNC(amerdart_state::from_shiftreg));

	tms32015_device &dsp(TMS32015(config, m_dsp, XTAL(40'000'000)/2));
	dsp.set_addrmap(AS_PROGRAM, &amerdart_state::dsp_pgm_map);
	// Data Map is internal to the CPU
	dsp.set_addrmap(AS_IO, &amerdart_state::dsp_io_map);
	dsp.bio().set(FUNC(amerdart_state::dsp_bio_line_r));

	TIMER(config, "audioint").configure_scanline(FUNC(amerdart_state::amerdart_audio_int_gen), "screen", 0, 1);

	GENERIC_LATCH_16(config, m_main2dsp);
	GENERIC_LATCH_16(config, m_dsp2main);
	m_dsp2main->data_pending_callback().set_inputline(m_maincpu, 1);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	TIMER(config, m_nvram_timer).configure_generic(FUNC(amerdart_state::nvram_write_timeout));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(40'000'000)/6, 212*2, 0, 161*2, 262, 0, 241);
	screen.set_screen_update(m_maincpu, FUNC(tms34010_device::tms340x0_rgb32));

	PALETTE(config, m_palette).set_entries(16);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	MP1210(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 1.0);
}


void _9ballsht_state::_9ballsht(machine_config &config)
{
	// basic machine hardware
	TMS34010(config, m_maincpu, XTAL(40'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &_9ballsht_state::nballsht_map);
	m_maincpu->set_halt_on_reset(false);
	m_maincpu->set_pixel_clock(XTAL(40'000'000)/6);
	m_maincpu->set_pixels_per_clock(1);
	m_maincpu->set_scanline_rgb32_callback(FUNC(_9ballsht_state::scanline));
	m_maincpu->set_shiftreg_in_callback(FUNC(_9ballsht_state::to_shiftreg));
	m_maincpu->set_shiftreg_out_callback(FUNC(_9ballsht_state::from_shiftreg));

	tms32026_device& dsp(TMS32026(config, m_dsp, XTAL(40'000'000)));
	dsp.set_addrmap(AS_PROGRAM, &_9ballsht_state::dsp_pgm_map);
	dsp.set_addrmap(AS_IO, &_9ballsht_state::nballsht_dsp_io_map);
	dsp.bio_in_cb().set(FUNC(_9ballsht_state::dsp_bio_line_r));
	dsp.hold_in_cb().set(FUNC(_9ballsht_state::dsp_hold_line_r));
//  dsp.hold_ack_out_cb().set(FUNC(_9ballsht_state::dsp_HOLDA_signal_w));

	GENERIC_LATCH_16(config, m_main2dsp);
	m_main2dsp->data_pending_callback().set_inputline(m_dsp, 0); // ???  I have no idea who should generate this!
															// the DSP polls the status bit so it isn't strictly
															// necessary to also have an IRQ

	GENERIC_LATCH_16(config, m_dsp2main);
	m_dsp2main->data_pending_callback().set_inputline(m_maincpu, 1);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	TIMER(config, m_nvram_timer).configure_generic(FUNC(_9ballsht_state::nvram_write_timeout));

	// video hardware
	TLC34076(config, m_tlc34076, tlc34076_device::TLC34076_6_BIT);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(40'000'000)/6, 424, 0, 320, 262, 0, 240);
	screen.set_screen_update(m_maincpu, FUNC(tms34010_device::tms340x0_rgb32));

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	MP1210(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 1.0);
}


void coolpool_state::coolpool(machine_config &config)
{
	_9ballsht(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &coolpool_state::coolpool_map);
	m_dsp->set_addrmap(AS_IO, &coolpool_state::coolpool_dsp_io_map);
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( amerdart ) // You need to check the sum16 values listed on the labels to determine different sets
	ROM_REGION16_LE( 0x0a0000, "maincpu", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u31 4e74", 0x000001, 0x10000, CRC(9628c422) SHA1(46b71acc746760962e34e9d7876f9499ea7d5c7c) )
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u32 0ef7", 0x000000, 0x10000, CRC(2d651ed0) SHA1(e2da2c3d8f25c17e26fd435c75983b2db8691993) )
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u38 10b4", 0x020001, 0x10000, CRC(1eb8c887) SHA1(220f566043535c54ad1cf2216966c7f42099e50b) )
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u39 f45a", 0x020000, 0x10000, CRC(2ab1ea68) SHA1(4e29a274c5c62b6ca92119eb320200beb784ca55) )
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u45 c1f9", 0x040001, 0x10000, CRC(74394375) SHA1(ceb7ae4e3253351da362cd0ada87702164005d17) )
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u46 1f84", 0x040000, 0x10000, CRC(1188047e) SHA1(249f25582ab72eeee37798418460de312053660e) )
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u52 cdfd", 0x060001, 0x10000, CRC(5ac2f06d) SHA1(b3a5d0cd94bdffdbf5bd17dbb30c07bfad3fa5d0) )
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u53 d432", 0x060000, 0x10000, CRC(4bd25cf0) SHA1(d1092cc3b6172d6567acd21f79b22043380102b7) )
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u57 6016", 0x080001, 0x10000, CRC(f620f935) SHA1(bf891fce1f04f3ad5b8b72d43d041ceacb0b65bc) ) // Different then set 2 or 3
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u58 48af", 0x080000, 0x10000, CRC(f1b3d7c4) SHA1(7b897230d110be7a5eb05eda927d00561ebb9ce3) ) // Different then set 2 or 3

	ROM_REGION( 0x10000, "dsp", 0 ) // TMS32015 code
	ROM_LOAD16_WORD( "tms320e15.bin", 0x0000, 0x2000, CRC(375db4ea) SHA1(11689c89ce62f44f43cb8973b4ec6e6b0024ed14) ) // Passes internal checksum routine

	ROM_REGION( 0x100000, "dspdata", 0 )              // TMS32015 audio sample data
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u1 4461",  0x000000, 0x10000, CRC(3f459482) SHA1(d9d489efd0d9217fceb3bf1a3b37a78d6823b4d9) ) // Different then set 2 or 3
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u16 abd6", 0x010000, 0x10000, CRC(7437e8bf) SHA1(754be4822cd586590f09e706d7eb48e5ba8c8817) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u2 cae4",  0x020000, 0x10000, CRC(a587fffd) SHA1(f33f511d1bf1d6eb3c42535593a9718571174c4b) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u17 b791", 0x030000, 0x10000, CRC(e32bdd0f) SHA1(0662abbe84f0bad2631566b506ef016fcd79b9ee) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u3 046e",  0x040000, 0x10000, CRC(984d343a) SHA1(ee214830de4cb22d2d8e9d3ca335eff05af4abb6) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u18 100e", 0x050000, 0x10000, CRC(de3b4d7c) SHA1(68e7ffe2d84aef7c24d1787c4f9b6950c0107741) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u4 7887",  0x060000, 0x10000, CRC(c4765ff6) SHA1(7dca61d32300047ca1c089057e617553d60a0995) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u19 94cf", 0x070000, 0x10000, CRC(7109247c) SHA1(201809ec6599b30c26823bde6851b6eaa2589710) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u5 47cb",  0x080000, 0x10000, CRC(3b63b890) SHA1(a1223cb8884d5365af7d3f607657efff877f8845) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u20 6cd4", 0x090000, 0x10000, CRC(038b7d2d) SHA1(80bab18ca36d2bc101da7f3f6e1c82d8a802c14c) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u6 ef71",  0x0a0000, 0x10000, CRC(5cdb9aa9) SHA1(fae5d2c7f649bcba8068c8bc8266ee411258535e) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u21 fe8a", 0x0b0000, 0x10000, CRC(9b0b8978) SHA1(b31d0451ecd7085c191d20b2b41d0e8fe551996c) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u7 2671",  0x0c0000, 0x10000, CRC(147083a2) SHA1(c04c38145ab159bd519e6325477a3f7d0eebbda1) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u22 8fd7", 0x0d0000, 0x10000, CRC(4b92588a) SHA1(eea262c1a122015364a0046ff2bc7816f5f6821d) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u8 961c",  0x0e0000, 0x10000, CRC(975b368c) SHA1(1d637ce8c5d60833bb25aab2610e1a856720235e) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u23 abef", 0x0f0000, 0x10000, CRC(d7c2b13b) SHA1(3561e08011f649e4d0c47792745b2a014167e816) ) // Different then set 2 or 3
ROM_END

ROM_START( amerdart2 ) // You need to check the sum16 values listed on the labels to determine different sets
	ROM_REGION16_LE( 0x0a0000, "maincpu", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u31 4e74", 0x000001, 0x10000, CRC(9628c422) SHA1(46b71acc746760962e34e9d7876f9499ea7d5c7c) )
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u32 0ef7", 0x000000, 0x10000, CRC(2d651ed0) SHA1(e2da2c3d8f25c17e26fd435c75983b2db8691993) )
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u38 10b4", 0x020001, 0x10000, CRC(1eb8c887) SHA1(220f566043535c54ad1cf2216966c7f42099e50b) )
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u39 f45a", 0x020000, 0x10000, CRC(2ab1ea68) SHA1(4e29a274c5c62b6ca92119eb320200beb784ca55) )
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u45 c1f9", 0x040001, 0x10000, CRC(74394375) SHA1(ceb7ae4e3253351da362cd0ada87702164005d17) )
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u46 1f84", 0x040000, 0x10000, CRC(1188047e) SHA1(249f25582ab72eeee37798418460de312053660e) )
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u52 cdfd", 0x060001, 0x10000, CRC(5ac2f06d) SHA1(b3a5d0cd94bdffdbf5bd17dbb30c07bfad3fa5d0) )
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u53 d432", 0x060000, 0x10000, CRC(4bd25cf0) SHA1(d1092cc3b6172d6567acd21f79b22043380102b7) )
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u57 1a0c", 0x080001, 0x10000, CRC(8a70f849) SHA1(dfd4cf90de2ab8cbeff458f0fd20110c1ed009e9) ) // Different then set 1 or 3
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u58 0d81", 0x080000, 0x10000, CRC(8bb81975) SHA1(b7666572ab543991c7deaa0ebefb8b4526a7e386) ) // Different then set 1 or 3

	ROM_REGION( 0x10000, "dsp", 0 ) // TMS32015 code
	ROM_LOAD16_WORD( "tms320e15.bin", 0x0000, 0x2000, CRC(375db4ea) SHA1(11689c89ce62f44f43cb8973b4ec6e6b0024ed14) ) // Passes internal checksum routine

	ROM_REGION( 0x100000, "dspdata", 0 )              // TMS32015 audio sample data
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u1 222f",  0x000000, 0x10000, CRC(e2bb7f54) SHA1(39eeb61a852b93331f445cc1c993727e52959660) ) // Different then set 1
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u16 abd6", 0x010000, 0x10000, CRC(7437e8bf) SHA1(754be4822cd586590f09e706d7eb48e5ba8c8817) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u2 cae4",  0x020000, 0x10000, CRC(a587fffd) SHA1(f33f511d1bf1d6eb3c42535593a9718571174c4b) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u17 b791", 0x030000, 0x10000, CRC(e32bdd0f) SHA1(0662abbe84f0bad2631566b506ef016fcd79b9ee) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u3 046e",  0x040000, 0x10000, CRC(984d343a) SHA1(ee214830de4cb22d2d8e9d3ca335eff05af4abb6) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u18 100e", 0x050000, 0x10000, CRC(de3b4d7c) SHA1(68e7ffe2d84aef7c24d1787c4f9b6950c0107741) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u4 7887",  0x060000, 0x10000, CRC(c4765ff6) SHA1(7dca61d32300047ca1c089057e617553d60a0995) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u19 94cf", 0x070000, 0x10000, CRC(7109247c) SHA1(201809ec6599b30c26823bde6851b6eaa2589710) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u5 47cb",  0x080000, 0x10000, CRC(3b63b890) SHA1(a1223cb8884d5365af7d3f607657efff877f8845) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u20 6cd4", 0x090000, 0x10000, CRC(038b7d2d) SHA1(80bab18ca36d2bc101da7f3f6e1c82d8a802c14c) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u6 ef71",  0x0a0000, 0x10000, CRC(5cdb9aa9) SHA1(fae5d2c7f649bcba8068c8bc8266ee411258535e) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u21 fe8a", 0x0b0000, 0x10000, CRC(9b0b8978) SHA1(b31d0451ecd7085c191d20b2b41d0e8fe551996c) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u7 2671",  0x0c0000, 0x10000, CRC(147083a2) SHA1(c04c38145ab159bd519e6325477a3f7d0eebbda1) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u22 8fd7", 0x0d0000, 0x10000, CRC(4b92588a) SHA1(eea262c1a122015364a0046ff2bc7816f5f6821d) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u8 961c",  0x0e0000, 0x10000, CRC(975b368c) SHA1(1d637ce8c5d60833bb25aab2610e1a856720235e) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u23 b806", 0x0f0000, 0x10000, CRC(7c1e6f2e) SHA1(21ae530e4bd7c0c9f1a84f01f136c71952c8adc4) ) // Different then set 1
ROM_END

ROM_START( amerdart3 ) // You need to check the sum16 values listed on the labels to determine different sets
	ROM_REGION16_LE( 0x0a0000, "maincpu", 0 ) // 34010 code
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u31 4e74", 0x000001, 0x10000, CRC(9628c422) SHA1(46b71acc746760962e34e9d7876f9499ea7d5c7c) )
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u32 0ef7", 0x000000, 0x10000, CRC(2d651ed0) SHA1(e2da2c3d8f25c17e26fd435c75983b2db8691993) )
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u38 10b4", 0x020001, 0x10000, CRC(1eb8c887) SHA1(220f566043535c54ad1cf2216966c7f42099e50b) )
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u39 f45a", 0x020000, 0x10000, CRC(2ab1ea68) SHA1(4e29a274c5c62b6ca92119eb320200beb784ca55) )
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u45 c1f9", 0x040001, 0x10000, CRC(74394375) SHA1(ceb7ae4e3253351da362cd0ada87702164005d17) )
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u46 1f84", 0x040000, 0x10000, CRC(1188047e) SHA1(249f25582ab72eeee37798418460de312053660e) )
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u52 cdfd", 0x060001, 0x10000, CRC(5ac2f06d) SHA1(b3a5d0cd94bdffdbf5bd17dbb30c07bfad3fa5d0) )
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u53 d432", 0x060000, 0x10000, CRC(4bd25cf0) SHA1(d1092cc3b6172d6567acd21f79b22043380102b7) )
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u57 4cac", 0x080001, 0x10000, CRC(2d653c7b) SHA1(0feebe6440aabe844049013aa063ed0259b7bec4) ) // Different then set 2 or 3
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u58 729e", 0x080000, 0x10000, CRC(8cef479a) SHA1(80002e215416a11ff071523ee67218a1aabe155b) ) // Different then set 2 or 3

	ROM_REGION( 0x10000, "dsp", 0 ) // TMS32015 code
	ROM_LOAD16_WORD( "tms320e15.bin", 0x0000, 0x2000, CRC(375db4ea) SHA1(11689c89ce62f44f43cb8973b4ec6e6b0024ed14) ) // Passes internal checksum routine

	ROM_REGION( 0x100000, "dspdata", 0 )              // TMS32015 audio sample data
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u1 222f",  0x000000, 0x10000, CRC(e2bb7f54) SHA1(39eeb61a852b93331f445cc1c993727e52959660) ) // Same as set 2
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u16 abd6", 0x010000, 0x10000, CRC(7437e8bf) SHA1(754be4822cd586590f09e706d7eb48e5ba8c8817) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u2 cae4",  0x020000, 0x10000, CRC(a587fffd) SHA1(f33f511d1bf1d6eb3c42535593a9718571174c4b) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u17 b791", 0x030000, 0x10000, CRC(e32bdd0f) SHA1(0662abbe84f0bad2631566b506ef016fcd79b9ee) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u3 046e",  0x040000, 0x10000, CRC(984d343a) SHA1(ee214830de4cb22d2d8e9d3ca335eff05af4abb6) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u18 100e", 0x050000, 0x10000, CRC(de3b4d7c) SHA1(68e7ffe2d84aef7c24d1787c4f9b6950c0107741) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u4 7887",  0x060000, 0x10000, CRC(c4765ff6) SHA1(7dca61d32300047ca1c089057e617553d60a0995) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u19 94cf", 0x070000, 0x10000, CRC(7109247c) SHA1(201809ec6599b30c26823bde6851b6eaa2589710) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u5 47cb",  0x080000, 0x10000, CRC(3b63b890) SHA1(a1223cb8884d5365af7d3f607657efff877f8845) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u20 6cd4", 0x090000, 0x10000, CRC(038b7d2d) SHA1(80bab18ca36d2bc101da7f3f6e1c82d8a802c14c) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u6 ef71",  0x0a0000, 0x10000, CRC(5cdb9aa9) SHA1(fae5d2c7f649bcba8068c8bc8266ee411258535e) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u21 fe8a", 0x0b0000, 0x10000, CRC(9b0b8978) SHA1(b31d0451ecd7085c191d20b2b41d0e8fe551996c) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u7 2671",  0x0c0000, 0x10000, CRC(147083a2) SHA1(c04c38145ab159bd519e6325477a3f7d0eebbda1) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u22 8fd7", 0x0d0000, 0x10000, CRC(4b92588a) SHA1(eea262c1a122015364a0046ff2bc7816f5f6821d) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u8 961c",  0x0e0000, 0x10000, CRC(975b368c) SHA1(1d637ce8c5d60833bb25aab2610e1a856720235e) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u23 b806", 0x0f0000, 0x10000, CRC(7c1e6f2e) SHA1(21ae530e4bd7c0c9f1a84f01f136c71952c8adc4) ) // Same as set 2
ROM_END


ROM_START( coolpool )
	ROM_REGION16_LE( 0x40000, "maincpu", 0 )  // 34010 code
	ROM_LOAD16_BYTE( "u112b",        0x00000, 0x20000, CRC(aa227769) SHA1(488e357a7aad07369cade3110cde14ba8562c66c) )
	ROM_LOAD16_BYTE( "u113b",        0x00001, 0x20000, CRC(5b5f82f1) SHA1(82afb6a8d94cf09960b962d5208aab451b56feae) )

	ROM_REGION16_LE( 0x200000, "maingfx", 0 )  // gfx data read by main CPU
	ROM_LOAD16_BYTE( "u04",          0x000000, 0x20000, CRC(66a9940e) SHA1(7fa587280ecfad6b06194868de09cbdd57cf517f) )
	ROM_CONTINUE(                    0x100000, 0x20000 )
	ROM_LOAD16_BYTE( "u08",          0x000001, 0x20000, CRC(56789cf4) SHA1(5ad867d5029fdac9dccd01a6979171aa30d9a6eb) )
	ROM_CONTINUE(                    0x100001, 0x20000 )
	ROM_LOAD16_BYTE( "u03",          0x040000, 0x20000, CRC(02bc792a) SHA1(8085cff38868a307d6d29a7aadf3d6a99cbe85bb) )
	ROM_CONTINUE(                    0x140000, 0x20000 )
	ROM_LOAD16_BYTE( "u07",          0x040001, 0x20000, CRC(7b2fcb9f) SHA1(fa912663891bac6ba78519f030ba2c718e3514c3) )
	ROM_CONTINUE(                    0x140001, 0x20000 )
	ROM_LOAD16_BYTE( "u02",          0x080000, 0x20000, CRC(3b7d757d) SHA1(8737721764b181b050d776b2d2e1208419f8e5eb) )
	ROM_CONTINUE(                    0x180000, 0x20000 )
	ROM_LOAD16_BYTE( "u06",          0x080001, 0x20000, CRC(c09353a2) SHA1(f3588ec75b757232bdaa40d055e171a501122bfa) )
	ROM_CONTINUE(                    0x180001, 0x20000 )
	ROM_LOAD16_BYTE( "u01",          0x0c0000, 0x20000, CRC(948a5faf) SHA1(186ab3ab0ede168beaa4dae0cba753df10cdac46) )
	ROM_CONTINUE(                    0x1c0000, 0x20000 )
	ROM_LOAD16_BYTE( "u05",          0x0c0001, 0x20000, CRC(616965e2) SHA1(588ea3c5c7838c50b2157ff1074f629d9d85791c) )
	ROM_CONTINUE(                    0x1c0001, 0x20000 )

	ROM_REGION( 0x40000, "dsp", 0 ) // TMS320C26
	ROM_LOAD16_BYTE( "u34",          0x00000, 0x08000, CRC(dc1df70b) SHA1(e42fa7e34e50e0bd2aaeea5c55d750ed3286610d) )
	ROM_LOAD16_BYTE( "u35",          0x00001, 0x08000, CRC(ac999431) SHA1(7e4c2dcaedcb7e7c67072a179e4b8488d2bbdac7) )

	ROM_REGION( 0x200000, "dspdata", 0 )  // TMS32026 data
	ROM_LOAD( "u17c",         0x000000, 0x40000, CRC(ea3cc41d) SHA1(e703e789dfbcfaec878a990031ce839164c51253) )
	ROM_LOAD( "u16c",         0x040000, 0x40000, CRC(2e6680ea) SHA1(cb30dc789039aab491428d075fee9e0bc04fd2ce) )
	ROM_LOAD( "u15c",         0x080000, 0x40000, CRC(8e5f248e) SHA1(a954d3c20dc0b70f83c4c238db30a33285fcb353) )
	ROM_LOAD( "u14c",         0x0c0000, 0x40000, CRC(dcd6cf71) SHA1(b1f53bffdd19f5da1d8664765d504568d1f5867c) )
	ROM_LOAD( "u13c",         0x100000, 0x40000, CRC(5a7fe750) SHA1(bbbd45380545cb0f17d9f6811b2a7300fa3b682d) )
	ROM_LOAD( "u12c",         0x140000, 0x40000, CRC(4f246958) SHA1(ee4446159635b6c44d88d8f6aac52787a89403c1) )
	ROM_LOAD( "u11c",         0x180000, 0x40000, CRC(92cd2b03) SHA1(e80df65f8ec5ed2178f623bdd975e2b01a12a184) )
	ROM_LOAD( "u10c",         0x1c0000, 0x40000, CRC(a3dbcae3) SHA1(af997f3f56f406d5eb9fa415e1672b2d129815b8) )
ROM_END


ROM_START( 9ballsht )
	ROM_REGION16_LE( 0x80000, "maincpu", 0 )  // 34010 code
	ROM_LOAD16_BYTE( "u112",         0x00000, 0x40000, CRC(b3855e59) SHA1(c3175df24b85897783169bcaccd61630e512f7f6) )
	ROM_LOAD16_BYTE( "u113",         0x00001, 0x40000, CRC(30cbf462) SHA1(64b2e2d40c2a92c4f4823dc866e5464792954ac3) )

	ROM_REGION16_LE( 0x100000, "maingfx", 0 )  // gfx data read by main CPU
	ROM_LOAD16_BYTE( "e-scape =c=1994 c316.u110", 0x00000, 0x80000, CRC(890ed5c0) SHA1(eaf06ee5b6c5ed0103b535396b4517012818a416) )
	ROM_LOAD16_BYTE( "e-scape =c=1994 13f2.u111", 0x00001, 0x80000, CRC(1a9f1145) SHA1(ba52a6d1aca26484c320518f69c66ce3ceb4adcf) )

	ROM_REGION( 0x40000, "dsp", 0 ) // TMS320C26
	ROM_LOAD16_BYTE( "e-scape =c=1994 89bc.u34", 0x00000, 0x08000, CRC(dc1df70b) SHA1(e42fa7e34e50e0bd2aaeea5c55d750ed3286610d) )
	ROM_LOAD16_BYTE( "e-scape =c=1994 af4a.u35", 0x00001, 0x08000, CRC(ac999431) SHA1(7e4c2dcaedcb7e7c67072a179e4b8488d2bbdac7) )

	ROM_REGION( 0x100000, "dspdata", 0 )  // TMS32026 data
	ROM_LOAD( "u54",          0x00000, 0x80000, CRC(1be5819c) SHA1(308b5b1fe05634419d03956ae1b2e5a61206900f) )
	ROM_LOAD( "u53",          0x80000, 0x80000, CRC(d401805d) SHA1(f4bcb2bdc45c3bc5ca423e518cdea8b3a7e8d60e) )
ROM_END

/*
  all ROMs for this set were missing except for the main program,
  I assume the others are the same.
 */
ROM_START( 9ballsht2 )
	ROM_REGION16_LE( 0x80000, "maincpu", 0 )  // 34010 code
	ROM_LOAD16_BYTE( "e-scape.112",  0x00000, 0x40000, CRC(aee8114f) SHA1(a0d0e9e3a879393585b85ac6d04e31a7d4221179) )
	ROM_LOAD16_BYTE( "e-scape.113",  0x00001, 0x40000, CRC(ccd472a7) SHA1(d074080e987c233b26b3c72248411c575f7a2293) )

	ROM_REGION16_LE( 0x100000, "maingfx", 0 )  // gfx data read by main CPU
	ROM_LOAD16_BYTE( "e-scape =c=1994 c316.u110", 0x00000, 0x80000, CRC(890ed5c0) SHA1(eaf06ee5b6c5ed0103b535396b4517012818a416) )
	ROM_LOAD16_BYTE( "e-scape =c=1994 13f2.u111", 0x00001, 0x80000, CRC(1a9f1145) SHA1(ba52a6d1aca26484c320518f69c66ce3ceb4adcf) )

	ROM_REGION( 0x40000, "dsp", 0 ) // TMS320C26
	ROM_LOAD16_BYTE( "e-scape =c=1994 89bc.u34", 0x00000, 0x08000, CRC(dc1df70b) SHA1(e42fa7e34e50e0bd2aaeea5c55d750ed3286610d) )
	ROM_LOAD16_BYTE( "e-scape =c=1994 af4a.u35", 0x00001, 0x08000, CRC(ac999431) SHA1(7e4c2dcaedcb7e7c67072a179e4b8488d2bbdac7) )

	ROM_REGION( 0x100000, "dspdata", 0 )  // TMS32026 data
	ROM_LOAD( "u54",          0x00000, 0x80000, CRC(1be5819c) SHA1(308b5b1fe05634419d03956ae1b2e5a61206900f) )
	ROM_LOAD( "u53",          0x80000, 0x80000, CRC(d401805d) SHA1(f4bcb2bdc45c3bc5ca423e518cdea8b3a7e8d60e) )
ROM_END

ROM_START( 9ballsht3 )
	ROM_REGION16_LE( 0x80000, "maincpu", 0 )  // 34010 code
	ROM_LOAD16_BYTE( "8e_1826.112",  0x00000, 0x40000, CRC(486f7a8b) SHA1(635e3b1e7a21a86dd3d0ea994e9b923b06df587e) )
	ROM_LOAD16_BYTE( "8e_6166.113",  0x00001, 0x40000, CRC(c41db70a) SHA1(162112f9f5bb6345920a45c41da6a249796bd21f) )

	ROM_REGION16_LE( 0x100000, "maingfx", 0 )  // gfx data read by main CPU
	ROM_LOAD16_BYTE( "e-scape =c=1994 c316.u110", 0x00000, 0x80000, CRC(890ed5c0) SHA1(eaf06ee5b6c5ed0103b535396b4517012818a416) )
	ROM_LOAD16_BYTE( "e-scape =c=1994 13f2.u111", 0x00001, 0x80000, CRC(1a9f1145) SHA1(ba52a6d1aca26484c320518f69c66ce3ceb4adcf) )

	ROM_REGION( 0x40000, "dsp", 0 ) // TMS320C26
	ROM_LOAD16_BYTE( "e-scape =c=1994 89bc.u34", 0x00000, 0x08000, CRC(dc1df70b) SHA1(e42fa7e34e50e0bd2aaeea5c55d750ed3286610d) )
	ROM_LOAD16_BYTE( "e-scape =c=1994 af4a.u35", 0x00001, 0x08000, CRC(ac999431) SHA1(7e4c2dcaedcb7e7c67072a179e4b8488d2bbdac7) )

	ROM_REGION( 0x100000, "dspdata", 0 )  // TMS32026 data
	ROM_LOAD( "u54",          0x00000, 0x80000, CRC(1be5819c) SHA1(308b5b1fe05634419d03956ae1b2e5a61206900f) )
	ROM_LOAD( "u53",          0x80000, 0x80000, CRC(d401805d) SHA1(f4bcb2bdc45c3bc5ca423e518cdea8b3a7e8d60e) )
ROM_END


// all checksums correctly match sum16 printed on rom labels
ROM_START( 9ballshtc )
	ROM_REGION16_LE( 0x80000, "maincpu", 0 )  // 34010 code
	ROM_LOAD16_BYTE( "e-scape =c=1994 3990.u112",  0x00000, 0x40000, CRC(7ba2749a) SHA1(e2ddc2600234dbebbb423f201cc4061fd0b9911a) )
	ROM_LOAD16_BYTE( "e-scape =c=1994 b72f.u113",  0x00001, 0x40000, CRC(1e0f3c62) SHA1(3c24a38dcb553fd84b0b44a5a8d93a14435e22b0) )

	ROM_REGION16_LE( 0x100000, "maingfx", 0 )  // gfx data read by main CPU
	ROM_LOAD16_BYTE( "e-scape =c=1994 c316.u110", 0x00000, 0x80000, CRC(890ed5c0) SHA1(eaf06ee5b6c5ed0103b535396b4517012818a416) )
	ROM_LOAD16_BYTE( "e-scape =c=1994 13f2.u111", 0x00001, 0x80000, CRC(1a9f1145) SHA1(ba52a6d1aca26484c320518f69c66ce3ceb4adcf) )

	ROM_REGION( 0x40000, "dsp", 0 ) // TMS320C26
	ROM_LOAD16_BYTE( "e-scape =c=1994 89bc.u34", 0x00000, 0x08000, CRC(dc1df70b) SHA1(e42fa7e34e50e0bd2aaeea5c55d750ed3286610d) )
	ROM_LOAD16_BYTE( "e-scape =c=1994 af4a.u35", 0x00001, 0x08000, CRC(ac999431) SHA1(7e4c2dcaedcb7e7c67072a179e4b8488d2bbdac7) )

	ROM_REGION( 0x100000, "dspdata", 0 )  // TMS32026 data
	ROM_LOAD( "e-scape =c=1994 0000.u54", 0x00000, 0x80000, CRC(04b509a0) SHA1(093343741a3d8d0786fd443e68dd85b414c6cf9e) )
	ROM_LOAD( "e-scape =c=1994 2df8.u53", 0x80000, 0x80000, CRC(c8a7b576) SHA1(7eb71dd791fdcbfe71764a454f0a1d3130d8a57e) )
ROM_END




/*************************************
 *
 *  Driver init
 *
 *************************************/

void _9ballsht_state::init_9ballsht()
{
	// decrypt the main program ROMs
	uint16_t *rom = (uint16_t *)memregion("maincpu")->base();
	int len = memregion("maincpu")->bytes();
	for (int a = 0; a < len/2; a++)
	{
		uint16_t const hi = rom[a] >> 8;
		uint16_t const lo = rom[a] & 0xff;

		int nhi = bitswap<8>(hi,5,2,0,7,6,4,3,1) ^ 0x29;
		if (hi & 0x01) nhi ^= 0x03;
		if (hi & 0x10) nhi ^= 0xc1;
		if (hi & 0x20) nhi ^= 0x40;
		if (hi & 0x40) nhi ^= 0x12;

		int nlo = bitswap<8>(lo,5,3,4,6,7,1,2,0) ^ 0x80;
		if ((lo & 0x02) && (lo & 0x04)) nlo ^= 0x01;
		if (lo & 0x04) nlo ^= 0x0c;
		if (lo & 0x08) nlo ^= 0x10;

		rom[a] = (nhi << 8) | nlo;
	}

	// decrypt the sub data ROMs
	rom = (uint16_t *)memregion("dspdata")->base();
	len = memregion("dspdata")->bytes();
	for (int a = 1; a < len/2; a += 4)
	{
		// just swap bits 1 and 2 of the address
		std::swap<uint16_t>(rom[a], rom[a+1]);
	}
}



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1989, amerdart,  0,        amerdart,  amerdart, amerdart_state,  empty_init,    ROT0, "Ameri",                               "AmeriDarts (set 1)",           MACHINE_SUPPORTS_SAVE )
GAME( 1989, amerdart2, amerdart, amerdart,  amerdart, amerdart_state,  empty_init,    ROT0, "Ameri",                               "AmeriDarts (set 2)",           MACHINE_SUPPORTS_SAVE )
GAME( 1989, amerdart3, amerdart, amerdart,  amerdart, amerdart_state,  empty_init,    ROT0, "Ameri",                               "AmeriDarts (set 3)",           MACHINE_SUPPORTS_SAVE )
GAME( 1992, coolpool,  0,        coolpool,  coolpool, coolpool_state,  empty_init,    ROT0, "Catalina",                            "Cool Pool",                    0 )
GAME( 1993, 9ballsht,  0,        _9ballsht, 9ballsht, _9ballsht_state, init_9ballsht, ROT0, "E-Scape EnterMedia (Bundra license)", "9-Ball Shootout (set 1)",      0 )
GAME( 1993, 9ballsht2, 9ballsht, _9ballsht, 9ballsht, _9ballsht_state, init_9ballsht, ROT0, "E-Scape EnterMedia (Bundra license)", "9-Ball Shootout (set 2)",      0 )
GAME( 1993, 9ballsht3, 9ballsht, _9ballsht, 9ballsht, _9ballsht_state, init_9ballsht, ROT0, "E-Scape EnterMedia (Bundra license)", "9-Ball Shootout (set 3)",      0 )
GAME( 1993, 9ballshtc, 9ballsht, _9ballsht, 9ballsht, _9ballsht_state, init_9ballsht, ROT0, "E-Scape EnterMedia (Bundra license)", "9-Ball Shootout Championship", 0 )
