// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Driver for Midway V-Unit games

    driver by Aaron Giles

    Games supported:
        * Cruis'n USA (1994)        [7 sets]
        * Cruis'n World (1996)      [7 sets]
        * War Gods (1996)           [3 sets]
        * Off Road Challenge (1997) [6 sets]

    Known bugs:
        * textures for automatic/manual selection get overwritten in Cruis'n World
        * rendering needs to be looked at a little more closely to fix some holes
        * in Cruis'n World attract mode, right side of sky looks like it has wrapped
        * Off Road Challenge has polygon sorting issues, among other problems
        * Issues for the Wargods sets:
           All sets report as Game Type: 452 (12/11/1995) [which is wrong for newer sets]

**************************************************************************/

#include "emu.h"
#include "midvunit.h"

#include "cpu/tms32031/tms32031.h"
#include "cpu/adsp2100/adsp2100.h"
#include "machine/nvram.h"

#include "speaker.h"

#include "crusnusa.lh"


#define LOG_INPUT    (1U << 1)
#define LOG_CTRL     (1U << 2)
#define LOG_REGS     (1U << 3)
#define LOG_COMM     (1U << 4)
#define LOG_SOUND    (1U << 5)

#define LOG_ALL      (LOG_INPUT | LOG_CTRL | LOG_REGS | LOG_COMM | LOG_SOUND)

#define VERBOSE (0)
#include "logmacro.h"

#define LOGINPUT(...) LOGMASKED(LOG_INPUT, __VA_ARGS__)
#define LOGCTRL(...)  LOGMASKED(LOG_CTRL,  __VA_ARGS__)
#define LOGREGS(...)  LOGMASKED(LOG_REGS,  __VA_ARGS__)
#define LOGCOMM(...)  LOGMASKED(LOG_COMM,  __VA_ARGS__)
#define LOGSOUND(...) LOGMASKED(LOG_SOUND, __VA_ARGS__)



/*************************************
 *
 *  Machine init
 *
 *************************************/

void midvunit_base_state::machine_start()
{
	save_item(NAME(m_cmos_protected));
	save_item(NAME(m_control_data));
	save_item(NAME(m_timer_rate));
}


void midvunit_state::machine_start()
{
	midvunit_base_state::machine_start();

	save_item(NAME(m_adc_shift));
	save_item(NAME(m_last_port0));
	save_item(NAME(m_shifter_state));
	save_item(NAME(m_galil_input_index));
	save_item(NAME(m_galil_input_length));
	save_item(NAME(m_galil_output_index));
	save_item(NAME(m_wheel_board_output));
	save_item(NAME(m_wheel_board_last));
	save_item(NAME(m_wheel_board_u8_latch));
	save_item(NAME(m_comm_flags));
	save_item(NAME(m_comm_data));

	m_optional_drivers.resolve();
	m_wheel_motor.resolve();
}


void crusnwld_state::machine_start()
{
	midvunit_state::machine_start();

	save_item(NAME(m_bit_index));
}


void midvplus_state::machine_start()
{
	midvunit_base_state::machine_start();

	save_item(NAME(m_lastval));
}


void midvunit_base_state::machine_reset()
{
	m_dcs->reset_w(0);
	m_dcs->reset_w(1);

	memcpy(m_ram_base, memregion("maindata")->base(), 0x20000*4);
	m_maincpu->reset();
}


void midvplus_state::machine_reset()
{
	midvunit_base_state::machine_reset();

	m_ata->reset();
}



/*************************************
 *
 *  Input ports
 *
 *************************************/

uint32_t midvunit_state::port0_r()
{
	uint16_t val = m_in0->read();
	uint16_t diff = val ^ m_last_port0;

	if (!machine().side_effects_disabled())
	{
		// make sure the shift controls are mutually exclusive
		if ((diff & 0x0400) && !(val & 0x0400))
			m_shifter_state = (m_shifter_state == 1) ? 0 : 1;
		if ((diff & 0x0800) && !(val & 0x0800))
			m_shifter_state = (m_shifter_state == 2) ? 0 : 2;
		if ((diff & 0x1000) && !(val & 0x1000))
			m_shifter_state = (m_shifter_state == 4) ? 0 : 4;
		if ((diff & 0x2000) && !(val & 0x2000))
			m_shifter_state = (m_shifter_state == 8) ? 0 : 8;
		m_last_port0 = val;
	}

	val = (val | 0x3c00) ^ (m_shifter_state << 10);

	return (val << 16) | val;
}


/*************************************
 *
 *  ADC input ports
 *
 *************************************/

uint32_t midvunit_state::adc_r()
{
	if (!(m_control_data & 0x40))
		return m_adc->read() << m_adc_shift;
	else
	{
		if (!machine().side_effects_disabled())
			LOGINPUT("adc_r without enabling reads!\n");
	}

	return 0xffffffff;
}

void midvunit_state::adc_w(uint32_t data)
{
	if (!(m_control_data & 0x20))
		m_adc->write(data >> m_adc_shift);
	else
		LOGINPUT("adc_w without enabling writes!\n");
}



/*************************************
 *
 *  CMOS access
 *
 *************************************/

void midvunit_base_state::cmos_protect_w(uint32_t data)
{
	m_cmos_protected = ((data & 0xc00) != 0xc00);
}


void midvunit_state::cmos_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (!m_cmos_protected)
		COMBINE_DATA(&m_nvram[offset]);
}


uint32_t midvunit_state::cmos_r(offs_t offset)
{
	return m_nvram[offset];
}



/*************************************
 *
 *  System controls
 *
 *************************************/

void midvunit_base_state::control_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint16_t const olddata = m_control_data;
	COMBINE_DATA(&m_control_data);

	// bit 7 is the LED

	// bit 3 is the watchdog
	m_watchdog->reset_line_w(BIT(m_control_data, 3));

	// bit 1 is the DCS sound reset
	m_dcs->reset_w(BIT(m_control_data, 1));

	// log anything unusual
	if ((olddata ^ m_control_data) & ~0x00e8)
		LOGCTRL("control_w: old=%04X new=%04X diff=%04X\n", olddata, m_control_data, olddata ^ m_control_data);
}


void crusnwld_state::crusnwld_control_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint16_t const olddata = m_control_data;
	COMBINE_DATA(&m_control_data);

	// bit 11 is the DCS sound reset
	m_dcs->reset_w(BIT(m_control_data, 11));

	// bit 9 is the watchdog
	m_watchdog->reset_line_w(BIT(m_control_data, 9));

	// bit 8 is the LED

	// log anything unusual
	if ((olddata ^ m_control_data) & ~0xe800)
		LOGCTRL("crusnwld_control_w: old=%04X new=%04X diff=%04X\n", olddata, m_control_data, olddata ^ m_control_data);
}


void midvunit_base_state::sound_w(uint32_t data)
{
	LOGSOUND("Sound W = %02X\n", data);
	m_dcs->data_w(data & 0xff);
}



/*************************************
 *
 *  TMS32031 I/O accesses
 *
 *************************************/

uint32_t midvunit_base_state::tms32031_control_r(offs_t offset)
{
	// watch for accesses to the timers
	if (offset == 0x24 || offset == 0x34)
	{
		// timer is clocked at 100ns
		int const which = (offset >> 4) & 1;
		int32_t const result = (m_timer[which]->elapsed() * m_timer_rate).as_double();
		//LOGREGS("%06X:tms32031_control_r(%02X) = %08X\n", m_maincpu->pc(), offset, result);
		return result;
	}

	if (!machine().side_effects_disabled())
	{
		// log anything else except the memory control register
		if (offset != 0x64)
			LOGREGS("%06X:tms32031_control_r(%02X)\n", m_maincpu->pc(), offset);
	}

	return m_tms32031_control[offset];
}


void midvunit_base_state::tms32031_control_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_tms32031_control[offset]);


	if (offset == 0x64)
		; // ignore changes to the memory control register
	else if (offset == 0x20 || offset == 0x30)
	{
		// watch for accesses to the timers
		int const which = (offset >> 4) & 1;
		//LOGREGS("%06X:tms32031_control_w(%02X) = %08X\n", m_maincpu->pc(), offset, data);
		if (data & 0x40)
			m_timer[which]->reset();

		// bit 0x200 selects internal clocking, which is 1/2 the main CPU clock rate
		if (data & 0x200)
			m_timer_rate = (double)(m_maincpu->unscaled_clock() * 0.5);
		else
			m_timer_rate = 10000000.;
	}
	else
		LOGREGS("%06X:tms32031_control_w(%02X) = %08X\n", m_maincpu->pc(), offset, data);
}



/*************************************
 *
 *  Serial number access
 *
 *************************************/

uint32_t crusnwld_state::crusnwld_serial_status_r()
{
	uint16_t const in1 = (m_in1->read() & 0x7fff) | (m_midway_serial_pic2->status_r() << 15);
	return in1 | in1 << 16;
}


uint32_t crusnwld_state::crusnwld_serial_data_r()
{
	return m_midway_serial_pic2->read() << 16;
}


void crusnwld_state::crusnwld_serial_data_w(uint32_t data)
{
	m_midway_serial_pic2->write(data >> 16);
}


/*************************************
 *
 *  Some kind of protection-like
 *  device
 *
 *************************************/

// values from offset 3, 6, and 10 must add up to 0x904752a2
static const uint32_t bit_data[0x10] =
{
	0x3017c636,0x3017c636,0x3017c636,0x3017c636,
	0x3017c636,0x3017c636,0x3017c636,0x3017c636,
	0x3017c636,0x3017c636,0x3017c636,0x3017c636,
	0x3017c636,0x3017c636,0x3017c636,0x3017c636
};


uint32_t crusnwld_state::bit_data_r(offs_t offset)
{
	int const bit = BIT(bit_data[m_bit_index >> 5], ~m_bit_index & 0x1f);
	if (!machine().side_effects_disabled())
		m_bit_index = (m_bit_index + 1) & 0x1ff;
	return bit ? m_nvram[offset] : ~m_nvram[offset];
}


void crusnwld_state::bit_reset_w(uint32_t data)
{
	m_bit_index = 0;
}


uint32_t midvunit_state::wheel_board_r()
{
	//LOGINPUT("wheel_board_r: %08X\n", m_wheel_board_output);
	return m_wheel_board_output << 8;
}

void midvunit_state::set_input(const char *s)
{
	m_galil_input = s;
	m_galil_input_index = 0;
	m_galil_input_length = strlen(s);
}

void midvunit_state::wheel_board_w(uint32_t data)
{
	//LOGINPUT("wheel_board_w: %08X\n", data);

	// U8 PAL22V10 "DECODE0" TODO: Needs dump "A-19674"
	if (BIT(data, 11) && !BIT(m_wheel_board_last, 11))
	{
		LOGINPUT("Wheel board (U8 PAL22V10; DECODE0) = %03X\n", BIT(data, 11) | ((data & 0xF) << 1) | ((data & 0x700) << 1));
		m_wheel_board_u8_latch = 0;
		m_wheel_board_u8_latch |= BIT(data, 0) << 6; // WA0; A for U9
		m_wheel_board_u8_latch |= BIT(data, 1) << 5; // WA1; B for U9
		m_wheel_board_u8_latch |= BIT(data, 2) << 4; // WA2; C for U9
		m_wheel_board_u8_latch |= BIT(data, 3) << 3; // WA3; G2B for U9
	}

	if (!BIT(data, 9))
	{
		LOGINPUT("Wheel board (U13 74HC245; DCS) = %02X\n", data & 0xFF);
	}
	else if (!BIT(data, 10)) // G2A for U9
	{
		uint8_t const arg = data & 0xFF;
		uint8_t const wa = BIT(m_wheel_board_u8_latch, 6) | (BIT(m_wheel_board_u8_latch, 5) << 1) | (BIT(m_wheel_board_u8_latch, 4) << 2);
		if (BIT(m_wheel_board_u8_latch, 3))
		{
			// U19 PAL22V10 "GALIL" TODO: Needs dump "A-19675", needs Galil emulation
			LOGINPUT("Wheel board (U19 PAL22V10; GALIL) = %03X\n", (m_wheel_board_u8_latch & 0x78) | ((data & 0x3F) << 6));
			switch (wa)
			{
				case 0:
					m_wheel_board_output = m_galil_input[m_galil_input_index++];
					break;
				case 1:
					if (arg != 0xD)
					{
						m_galil_output[m_galil_output_index] = (char)arg;
						if (m_galil_output_index < 450)
							m_galil_output_index++;
					}
					else
					{
						// G, W, S, and Q are commented out because they are error commands.
						if (strstr(m_galil_output,"MG \"V\" IBO {$2.0}"))
							set_input("V$00");
						else if (strstr(m_galil_output,"MG \"X\", _TSX {$2.0}"))
							set_input("X$00");
						else if (strstr(m_galil_output,"MG \"Y\", _TSY {$2.0}"))
							set_input("Y$00");
						else if (strstr(m_galil_output,"MG \"Z\", _TSZ {$2.0}"))
							set_input("Z$00");
						/*else if (strstr(m_galil_output,"MG \"G\""))
						    set_input("G");
						else if (strstr(m_galil_output,"MG \"W\""))
						    set_input("W");
						else if (strstr(m_galil_output,"MG \"S\""))
						    set_input("S");
						else if (strstr(m_galil_output,"MG \"Q\""))
						    set_input("Q");*/
						else
							set_input(":");
						LOGINPUT("Galil Command: %s\n", m_galil_output);
						memset(m_galil_output, 0, m_galil_output_index);
						m_galil_output_index = 0;
					}
					break;
				case 2:
					m_wheel_board_output = (m_galil_input_index < m_galil_input_length) ? 0x80 : 0x0;
					break;
				case 3: // Galil init?
					break;
				case 4: // Galil init?
					set_input(":");
					m_galil_output_index = 0;
					memset(m_galil_output, 0, 450);
					break;
			}
		}
		else
		{
			// U9 74LS138
			switch (wa)
			{
				case 0: // SNDCTLZ
					LOGINPUT("Wheel board (U14 74HC574; DCS Control) = %02X\n", arg);
					break;
				case 1: // GALCTLZ
					LOGINPUT("Wheel board (U19 PAL22V10; Galil Control) = %02X\n", arg);
					break;
				case 2: // ATODWRZ
					LOGINPUT("Wheel board (ATODWRZ) = %02X\n", arg);
					break;
				case 3: // ATODRDZ
					LOGINPUT("Wheel board (ATODRDZ) = %02X\n", arg);
					break;
				case 4: // WHLCTLZ
					m_wheel_motor = arg;
					//LOGINPUT("Wheel board (U4 74HC574; Motor) = %02X\n", arg);
					break;
				case 5: // DRVCTLZ
					for (uint8_t bit = 0; bit < 8; bit++)
						m_optional_drivers[bit] = BIT(data, bit);
					//LOGINPUT("Wheel board (U10 74HC574; Lamps) = %02X\n", arg);
					break;
				case 6: // PRTCTLZ
					LOGINPUT("Wheel board (PRTCTLZ) = %02X\n", arg);
					break;
				case 7: // PRTSTATZ
					LOGINPUT("Wheel board (PRTSTATZ) = %02X\n", arg);
					break;
			}
		}
	}

	m_wheel_board_last = data;
}


ioport_value crusnusa_state::motion_r()
{
	uint8_t const status = m_motion->read();
	for (uint8_t bit = 0; bit < 8; bit++)
	{
		if (BIT(status, bit))
			return bit + 1;
	}
	return 0;
}


uint32_t midvunit_state::intcs_r()
{
	return 4;
}

uint16_t midvunit_state::comm_bus_out()
{
	uint16_t mask = 0;
	if (m_comm_flags & 0x20) // COMCOE
		mask |= (m_comm_data >> 4) & 0xf00;
	if (m_comm_flags & 0x40) // COMDOE
		mask |= 0xff;
	return m_comm_data & mask;
}

// To link multiple machines together will require comm_bus_out()
// to be called on each machine and bitwise ORed here.
// This must be done in real time with proper synchronization.
uint16_t midvunit_state::comm_bus_in()
{
	return comm_bus_out();
}

uint32_t midvunit_state::comcs_r(offs_t offset)
{
	if (offset != 0)
	{
		if (!machine().side_effects_disabled())
			LOGCOMM("comcs_r(%d)\n", offset);
		return 0;
	}
	else
	{
		uint16_t data = comm_bus_in();
		if (m_comm_flags & 0x20) // COMCOE
			data &= ~((m_comm_data >> 4) & 0xf00);
		if (m_comm_flags & 0x40) // COMDOE
			data &= ~0xff;
		return data << 16;
	}
}

void midvunit_state::comcs_w(offs_t offset, uint32_t data)
{
	switch (offset)
	{
		default: LOGCOMM("comcs_w(%d) = %08X\n", offset, data); break;
		case 0: m_comm_data = data >> 16; break;
		case 1: m_comm_flags = (data >> 24) & 0xe0; break;
	}
}


/*************************************
 *
 *  War Gods I/O ASICs
 *
 *************************************/

uint32_t midvplus_state::midvplus_misc_r(offs_t offset)
{
	uint32_t result = m_midvplus_misc[offset];

	switch (offset)
	{
		case 0:
			result = 0xb580;
			break;

		case 2:
			result = 0xf3ff;
			break;

		case 3:
			// seems to want loopback
			break;
	}

	if (!machine().side_effects_disabled())
	{
		if (offset != 0 && offset != 3)
			LOGCTRL("%06X:midvplus_misc_r(%d) = %08X\n", m_maincpu->pc(), offset, result);
	}
	return result;
}


void midvplus_state::midvplus_misc_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t const olddata = m_midvplus_misc[offset];
	bool logit = true;

	COMBINE_DATA(&m_midvplus_misc[offset]);

	switch (offset)
	{
		case 0:
			// bit 4 resets watchdog
			m_watchdog->reset_line_w(BIT(data, 4));

			logit = bool((olddata ^ m_midvplus_misc[offset]) & ~0x0010);
			break;

		case 3:
			logit = false;
			break;
	}

	if (logit)
		LOGCTRL("%06X:midvplus_misc_w(%d) = %08X\n", m_maincpu->pc(), offset, data);
}



/*************************************
 *
 *  War Gods RAM grossness
 *
 *************************************/

void midvplus_state::midvplus_xf1_w(uint8_t data)
{
//  osd_printf_debug("xf1_w = %d\n", data);

	if (m_lastval && !data)
		memcpy(m_ram_base, m_fastram_base, 0x20000*4);

	m_lastval = data;
}



/*************************************
 *
 *  Memory maps
 *
 *************************************/

void midvunit_state::midvunit_map(address_map &map)
{
	map(0x000000, 0x01ffff).ram().share(m_ram_base);
	map(0x400000, 0x41ffff).ram();
	map(0x600000, 0x600000).w(FUNC(midvunit_state::dma_queue_w));
	map(0x808000, 0x80807f).rw(FUNC(midvunit_state::tms32031_control_r), FUNC(midvunit_state::tms32031_control_w)).share(m_tms32031_control);
	map(0x900000, 0x97ffff).rw(FUNC(midvunit_state::videoram_r), FUNC(midvunit_state::videoram_w));
	map(0x980000, 0x980000).r(FUNC(midvunit_state::dma_queue_entries_r));
	map(0x980020, 0x980020).r(FUNC(midvunit_state::scanline_r));
	map(0x980020, 0x98002b).w(FUNC(midvunit_state::video_control_w));
	map(0x980040, 0x980040).rw(FUNC(midvunit_state::page_control_r), FUNC(midvunit_state::page_control_w));
	map(0x980080, 0x980080).noprw();
	map(0x980082, 0x980083).r(FUNC(midvunit_state::dma_trigger_r));
	map(0x990000, 0x990000).r(FUNC(midvunit_state::intcs_r));
	map(0x991030, 0x991030).lr16(NAME([this] () { return uint16_t(m_in1->read()); }));
//  map(0x991050, 0x991050).readonly(); // seems to be another port
	map(0x991060, 0x991060).r(FUNC(midvunit_state::port0_r));
	map(0x992000, 0x992000).lr16(NAME([this] () { return uint16_t(m_dsw->read()); }));
	map(0x993000, 0x993000).rw(FUNC(midvunit_state::adc_r), FUNC(midvunit_state::adc_w));
	map(0x994000, 0x994000).w(FUNC(midvunit_state::control_w));
	map(0x995000, 0x995000).rw(FUNC(midvunit_state::wheel_board_r), FUNC(midvunit_state::wheel_board_w));
	map(0x995020, 0x995020).w(FUNC(midvunit_state::cmos_protect_w));
	map(0x997000, 0x997008).rw(FUNC(midvunit_state::comcs_r), FUNC(midvunit_state::comcs_w));
	map(0x9a0000, 0x9a0000).w(FUNC(midvunit_state::sound_w));
	map(0x9c0000, 0x9c1fff).rw(FUNC(midvunit_state::cmos_r), FUNC(midvunit_state::cmos_w)).share(m_nvram);
	map(0x9e0000, 0x9e7fff).ram().w(FUNC(midvunit_state::paletteram_w)).share(m_paletteram);
	map(0xa00000, 0xbfffff).rw(FUNC(midvunit_state::textureram_r), FUNC(midvunit_state::textureram_w)).share(m_textureram);
	map(0xc00000, 0xffffff).rom().region("maindata", 0);
}


void crusnwld_state::offroadc_map(address_map &map)
{
	midvunit_map(map);

	map(0x991030, 0x991030).r(FUNC(crusnwld_state::crusnwld_serial_status_r));
	map(0x994000, 0x994000).w(FUNC(crusnwld_state::crusnwld_control_w));
	map(0x996000, 0x996000).rw(FUNC(crusnwld_state::crusnwld_serial_data_r), FUNC(crusnwld_state::crusnwld_serial_data_w));
}


void crusnwld_state::crusnwld_map(address_map &map)
{
	offroadc_map(map);

	map(0x9d0000, 0x9d1fff).r(FUNC(crusnwld_state::bit_data_r));
	map(0x9d0000, 0x9d0000).w(FUNC(crusnwld_state::bit_reset_w));
}


void midvplus_state::midvplus_map(address_map &map)
{
	map(0x000000, 0x01ffff).ram().share(m_ram_base);
	map(0x400000, 0x41ffff).ram().share(m_fastram_base);
	map(0x600000, 0x600000).w(FUNC(midvplus_state::dma_queue_w));
	map(0x808000, 0x80807f).rw(FUNC(midvplus_state::tms32031_control_r), FUNC(midvplus_state::tms32031_control_w)).share(m_tms32031_control);
	map(0x900000, 0x97ffff).rw(FUNC(midvplus_state::videoram_r), FUNC(midvplus_state::videoram_w));
	map(0x980000, 0x980000).r(FUNC(midvplus_state::dma_queue_entries_r));
	map(0x980020, 0x980020).r(FUNC(midvplus_state::scanline_r));
	map(0x980020, 0x98002b).w(FUNC(midvplus_state::video_control_w));
	map(0x980040, 0x980040).rw(FUNC(midvplus_state::page_control_r), FUNC(midvplus_state::page_control_w));
	map(0x980080, 0x980080).noprw();
	map(0x980082, 0x980083).r(FUNC(midvplus_state::dma_trigger_r));
	map(0x990000, 0x99000f).rw(m_midway_ioasic, FUNC(midway_ioasic_device::read), FUNC(midway_ioasic_device::write));
	map(0x994000, 0x994000).w(FUNC(midvplus_state::control_w));
	map(0x995020, 0x995020).w(FUNC(midvplus_state::cmos_protect_w));
	map(0x9a0000, 0x9a0007).rw(m_ata, FUNC(ata_interface_device::cs0_r), FUNC(ata_interface_device::cs0_w)).umask32(0x0000ffff);
	map(0x9c0000, 0x9c7fff).ram().w(FUNC(midvplus_state::paletteram_w)).share(m_paletteram);
	map(0x9d0000, 0x9d000f).rw(FUNC(midvplus_state::midvplus_misc_r), FUNC(midvplus_state::midvplus_misc_w)).share(m_midvplus_misc);
	map(0xa00000, 0xbfffff).rw(FUNC(midvplus_state::textureram_r), FUNC(midvplus_state::textureram_w)).share(m_textureram);
	map(0xc00000, 0xcfffff).ram();
}



/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( midvunit )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT ) // Slam Switch
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Enter") PORT_CODE(KEYCODE_F2) // Test switch
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("4th Gear")    // 4th
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("3rd Gear")    // 3rd
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("2nd Gear")    // 2nd
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("1st Gear")    // 1st
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Radio")   // radio
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("View 1")  // view 1
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("View 2")  // view 2
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("View 3") // view 3
	PORT_BIT( 0xff80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("WHEEL")     // wheel
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(25) PORT_KEYDELTA(20)

	PORT_START("ACCEL")     // gas pedal
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(20)

	PORT_START("BRAKE")     // brake pedal
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_SENSITIVITY(25) PORT_KEYDELTA(20)
INPUT_PORTS_END


static INPUT_PORTS_START( crusnusa )
	PORT_INCLUDE( midvunit )

	PORT_START("MOTION")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_NAME("Motion Status - Mat Not Plugged In")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_NAME("Motion Status - Mat Stepped On")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_NAME("Motion Status - Opto Path Broken")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_NAME("Motion Status - Opto Detector Not Receiving")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_NAME("Motion Status - Opto LED Not Emitting")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_NAME("Motion Status - Fail Safe Switch Engaged")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_NAME("Motion Status - Fail Safe Switch Not Connected Correctly")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_NAME("Motion Status - Board Not Plugged In")

	PORT_MODIFY("IN1")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_NAME("Motion Stop")
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_NAME("Motion Status - Device 1")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_NAME("Motion Status - Device 2")
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_NAME("Motion Status - Device 3")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_NAME("Motion Status - Device 4")
	PORT_BIT( 0xf000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(crusnusa_state, motion_r)

	PORT_START("DSW")
	// DSW2 at U97
	PORT_DIPNAME( 0x0001, 0x0000, "Link Status" )       PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0000, "Master" )
	PORT_DIPSETTING(      0x0001, "Slave" )
	PORT_DIPNAME( 0x0002, 0x0002, "Link???" )       PORT_DIPLOCATION("SW2:7") // Listed as Not Used in the manual
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Linking" )       PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:5") // Listed as Not Used in the manual
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Freeze" )        PORT_DIPLOCATION("SW2:4") // Listed as Not Used in the manual
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Cabinet ) )  PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0020, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0000, "Sitdown" )
	PORT_DIPNAME( 0x0040, 0x0040, "Enable Motion" )     PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW2:1" ) // Listed as Not Used in the manual

	// DSW3 at U19
	PORT_DIPNAME( 0x0100, 0x0100, "Coin Counters" )     PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(      0x0100, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPNAME( 0xfe00, 0xf800, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW3:7,6,5,4,3,2,1")
	PORT_DIPSETTING(      0xfe00, "USA-1" )
	PORT_DIPSETTING(      0xfa00, "USA-3" )
	PORT_DIPSETTING(      0xfc00, "USA-7" )
	PORT_DIPSETTING(      0xf800, "USA-8" )
	PORT_DIPSETTING(      0xf600, "Norway-1" )
	PORT_DIPSETTING(      0xee00, "Australia-1" )
	PORT_DIPSETTING(      0xea00, "Australia-2" )
	PORT_DIPSETTING(      0xec00, "Australia-3" )
	PORT_DIPSETTING(      0xe800, "Australia-4" )
	PORT_DIPSETTING(      0xde00, "Swiss-1" )
	PORT_DIPSETTING(      0xda00, "Swiss-2" )
	PORT_DIPSETTING(      0xdc00, "Swiss-3" )
	PORT_DIPSETTING(      0xce00, "Belgium-1" )
	PORT_DIPSETTING(      0xca00, "Belgium-2" )
	PORT_DIPSETTING(      0xcc00, "Belgium-3" )
	PORT_DIPSETTING(      0xbe00, "French-1" )
	PORT_DIPSETTING(      0xba00, "French-2" )
	PORT_DIPSETTING(      0xbc00, "French-3" )
	PORT_DIPSETTING(      0xb800, "French-4" )
	PORT_DIPSETTING(      0xb600, "Hungary-1" )
	PORT_DIPSETTING(      0xae00, "Taiwan-1" )
	PORT_DIPSETTING(      0xaa00, "Taiwan-2" )
	PORT_DIPSETTING(      0xac00, "Taiwan-3" )
	PORT_DIPSETTING(      0x9e00, "UK-1" )
	PORT_DIPSETTING(      0x9a00, "UK-2" )
	PORT_DIPSETTING(      0x9c00, "UK-3" )
	PORT_DIPSETTING(      0x8e00, "Finland-1" )
	PORT_DIPSETTING(      0x7e00, "German-1" )
	PORT_DIPSETTING(      0x7a00, "German-2" )
	PORT_DIPSETTING(      0x7c00, "German-3" )
	PORT_DIPSETTING(      0x7800, "German-4" )
	PORT_DIPSETTING(      0x7600, "Denmark-1" )
	PORT_DIPSETTING(      0x6e00, "Japan-1" )
	PORT_DIPSETTING(      0x6a00, "Japan-2" )
	PORT_DIPSETTING(      0x6c00, "Japan-3" )
	PORT_DIPSETTING(      0x5e00, "Italy-1" )
	PORT_DIPSETTING(      0x5a00, "Italy-2" )
	PORT_DIPSETTING(      0x5c00, "Italy-3" )
	PORT_DIPSETTING(      0x4e00, "Sweden-1" )
	PORT_DIPSETTING(      0x3e00, "Canada-1" )
	PORT_DIPSETTING(      0x3a00, "Canada-2" )
	PORT_DIPSETTING(      0x3c00, "Canada-3" )
	PORT_DIPSETTING(      0x3600, "General-1" )
	PORT_DIPSETTING(      0x3200, "General-3" )
	PORT_DIPSETTING(      0x3400, "General-5" )
	PORT_DIPSETTING(      0x3000, "General-7" )
	PORT_DIPSETTING(      0x2e00, "Austria-1" )
	PORT_DIPSETTING(      0x2a00, "Austria-2" )
	PORT_DIPSETTING(      0x2c00, "Austria-3" )
	PORT_DIPSETTING(      0x2800, "Austria-4" )
	PORT_DIPSETTING(      0x1e00, "Spain-1" )
	PORT_DIPSETTING(      0x1a00, "Spain-2" )
	PORT_DIPSETTING(      0x1c00, "Spain-3" )
	PORT_DIPSETTING(      0x1800, "Spain-4" )
	PORT_DIPSETTING(      0x0e00, "Netherland-1" )
INPUT_PORTS_END


static INPUT_PORTS_START( crusnwld )
	PORT_INCLUDE( midvunit )

	PORT_START("DSW")
	// DSW2 at U97
	PORT_DIPNAME( 0x0003, 0x0000, "Link Number" )       PORT_DIPLOCATION("SW2:8,7")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0001, "2" )
	PORT_DIPSETTING(      0x0002, "3" )
	PORT_DIPSETTING(      0x0003, "4" )
	PORT_DIPNAME( 0x0004, 0x0004, "Linking" )       PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0018, 0x0008, "Games Linked" )      PORT_DIPLOCATION("SW2:5,4")
	PORT_DIPSETTING(      0x0008, "2" )
	PORT_DIPSETTING(      0x0010, "3" )
	PORT_DIPSETTING(      0x0018, "4" )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Cabinet ) )  PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0020, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0000, "Sitdown" )
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW2:2" )        // Manual shows Not Used
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW2:1" )

	// DSW3 at U19
	PORT_DIPNAME( 0x0100, 0x0100, "Coin Counters" )     PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(      0x0100, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPNAME( 0xfe00, 0xf800, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW3:7,6,5,4,3,2,1")
	PORT_DIPSETTING(      0xfe00, "USA-1" )
	PORT_DIPSETTING(      0xfa00, "USA-3" )
	PORT_DIPSETTING(      0xfc00, "USA-7" )
	PORT_DIPSETTING(      0xf800, "USA-8" )
	PORT_DIPSETTING(      0xf600, "Norway-1" )
	PORT_DIPSETTING(      0xee00, "Australia-1" )
	PORT_DIPSETTING(      0xea00, "Australia-2" )
	PORT_DIPSETTING(      0xec00, "Australia-3" )
	PORT_DIPSETTING(      0xe800, "Australia-4" )
	PORT_DIPSETTING(      0xde00, "Swiss-1" )
	PORT_DIPSETTING(      0xda00, "Swiss-2" )
	PORT_DIPSETTING(      0xdc00, "Swiss-3" )
	PORT_DIPSETTING(      0xce00, "Belgium-1" )
	PORT_DIPSETTING(      0xca00, "Belgium-2" )
	PORT_DIPSETTING(      0xcc00, "Belgium-3" )
	PORT_DIPSETTING(      0xbe00, "French-1" )
	PORT_DIPSETTING(      0xba00, "French-2" )
	PORT_DIPSETTING(      0xbc00, "French-3" )
	PORT_DIPSETTING(      0xb800, "French-4" )
	PORT_DIPSETTING(      0xb600, "Hungary-1" )
	PORT_DIPSETTING(      0xae00, "Taiwan-1" )
	PORT_DIPSETTING(      0xaa00, "Taiwan-2" )
	PORT_DIPSETTING(      0xac00, "Taiwan-3" )
	PORT_DIPSETTING(      0x9e00, "UK-1" )
	PORT_DIPSETTING(      0x9a00, "UK-2" )
	PORT_DIPSETTING(      0x9c00, "UK-3" )
	PORT_DIPSETTING(      0x8e00, "Finland-1" )
	PORT_DIPSETTING(      0x7e00, "German-1" )
	PORT_DIPSETTING(      0x7a00, "German-2" )
	PORT_DIPSETTING(      0x7c00, "German-3" )
	PORT_DIPSETTING(      0x7800, "German-4" )
	PORT_DIPSETTING(      0x7600, "Denmark-1" )
	PORT_DIPSETTING(      0x6e00, "Japan-1" )
	PORT_DIPSETTING(      0x6a00, "Japan-2" )
	PORT_DIPSETTING(      0x6c00, "Japan-3" )
	PORT_DIPSETTING(      0x5e00, "Italy-1" )
	PORT_DIPSETTING(      0x5a00, "Italy-2" )
	PORT_DIPSETTING(      0x5c00, "Italy-3" )
	PORT_DIPSETTING(      0x4e00, "Sweden-1" )
	PORT_DIPSETTING(      0x3e00, "Canada-1" )
	PORT_DIPSETTING(      0x3a00, "Canada-2" )
	PORT_DIPSETTING(      0x3c00, "Canada-3" )
	PORT_DIPSETTING(      0x3600, "General-1" )
	PORT_DIPSETTING(      0x3200, "General-3" )
	PORT_DIPSETTING(      0x3400, "General-5" )
	PORT_DIPSETTING(      0x3000, "General-7" )
	PORT_DIPSETTING(      0x2e00, "Austria-1" )
	PORT_DIPSETTING(      0x2a00, "Austria-2" )
	PORT_DIPSETTING(      0x2c00, "Austria-3" )
	PORT_DIPSETTING(      0x2800, "Austria-4" )
	PORT_DIPSETTING(      0x1e00, "Spain-1" )
	PORT_DIPSETTING(      0x1a00, "Spain-2" )
	PORT_DIPSETTING(      0x1c00, "Spain-3" )
	PORT_DIPSETTING(      0x1800, "Spain-4" )
	PORT_DIPSETTING(      0x0e00, "Netherland-1" )
INPUT_PORTS_END


static INPUT_PORTS_START( offroadc )
	PORT_INCLUDE( midvunit )

	PORT_START("DSW")
	// DSW2 at U97
	PORT_DIPUNUSED_DIPLOC( 0x0001, 0x0001, "SW2:8" )        // Manual shows Not Used & "No Effect" for both On & Off
	PORT_DIPNAME( 0x0002, 0x0000, "Gear Shifter Switch" )       PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0002, "Normally Closed" )
	PORT_DIPSETTING(      0x0000, "Normally Open" )
	PORT_DIPNAME( 0x0004, 0x0004, "Added Attractions" )     PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0004, "Girls Present" )
	PORT_DIPSETTING(      0x0000, "Girls Missing" )
	PORT_DIPNAME( 0x0008, 0x0008, "Graphic Effects" )       PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0008, "Roadkill Present" )
	PORT_DIPSETTING(      0x0000, "Roadkill Missing" )
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "SW2:4" )        // Manual shows Not Used & "No Effect" for both On & Off
	PORT_DIPNAME( 0x0020, 0x0020, "Link" )              PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0020, "Disabled" )
	PORT_DIPSETTING(      0x0000, "Enabled" )
	PORT_DIPNAME( 0x00c0, 0x00c0, "Link Machine" )          PORT_DIPLOCATION("SW2:2,1")
	PORT_DIPSETTING(      0x00c0, "1" )
	PORT_DIPSETTING(      0x0080, "2" )
	PORT_DIPSETTING(      0x0040, "3" )
	PORT_DIPSETTING(      0x0000, "4" )

	// DSW3 at U19
	PORT_DIPUNUSED_DIPLOC( 0x0100, 0x0100, "SW3:8" )    // Manual states "Switches 6, 7 and 8 are not active. We recommend
	PORT_DIPUNUSED_DIPLOC( 0x0200, 0x0200, "SW3:7" )    // they be set to the facorty default (OFF) positions."
	PORT_DIPUNUSED_DIPLOC( 0x0400, 0x0400, "SW3:6" )
	PORT_DIPNAME( 0xf800, 0xf800, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW3:5,4,3,2,1")
	PORT_DIPSETTING(      0xf800, "USA 1" )
	PORT_DIPSETTING(      0xf000, "German 1" )
	PORT_DIPSETTING(      0xe800, "French 1" )
	PORT_DIPSETTING(      0xe000, "Canada 1" )
	PORT_DIPSETTING(      0xd800, "Swiss 1" )
	PORT_DIPSETTING(      0xd000, "Italy 1" )
	PORT_DIPSETTING(      0xc800, "UK 1" )
	PORT_DIPSETTING(      0xc000, "Spain 1" )
	PORT_DIPSETTING(      0xb800, "Australia 1" )
	PORT_DIPSETTING(      0xb000, "Japan 1" )
	PORT_DIPSETTING(      0xa800, "Taiwan 1" )
	PORT_DIPSETTING(      0xa000, "Austria 1" )
	PORT_DIPSETTING(      0x9800, "Belgium 1" )
	PORT_DIPSETTING(      0x9000, "Sweden 1" )
	PORT_DIPSETTING(      0x8800, "Finland 1" )
	PORT_DIPSETTING(      0x8000, "Netherlands 1" )
	PORT_DIPSETTING(      0x7800, "Norway 1" )
	PORT_DIPSETTING(      0x7000, "Denmark 1" )
	PORT_DIPSETTING(      0x6800, "Hungary 1" )
INPUT_PORTS_END


static INPUT_PORTS_START( wargods )
	PORT_START("DIPS")
	PORT_DIPNAME( 0x0001, 0x0001, "CRT Type / Resolution" )     PORT_DIPLOCATION("SW1:1") // This only works for the Dual Res version
	PORT_DIPSETTING(      0x0001, "Medium Res (24Khz)" )
	PORT_DIPSETTING(      0x0000, "Standard Res (15Khz)" )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:2") // Manual shows Not Used (must be Off)
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:3") // Manual shows Not Used (must be Off)
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Blood" )             PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Graphics" )          PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, "Family" )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:6") // Manual shows Not Used
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:7") // Manual shows Not Used
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:8") // Manual shows Not Used
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "Coinage Source" )        PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, "Dipswitch" )
	PORT_DIPSETTING(      0x0000, "CMOS" )
	PORT_DIPNAME( 0x3e00, 0x3e00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW2:2,3,4,5,6")
	PORT_DIPSETTING(      0x3e00, "USA-1" )
	PORT_DIPSETTING(      0x3c00, "USA-2" )
	PORT_DIPSETTING(      0x3a00, "USA-3" )
	PORT_DIPSETTING(      0x3800, "USA-4" )
	PORT_DIPSETTING(      0x3400, "USA-9" )
	PORT_DIPSETTING(      0x3200, "USA-10" )
	PORT_DIPSETTING(      0x3600, "USA-ECA" )
	PORT_DIPSETTING(      0x2e00, "German-1" )
	PORT_DIPSETTING(      0x2c00, "German-2" )
	PORT_DIPSETTING(      0x2a00, "German-3" )
	PORT_DIPSETTING(      0x2800, "German-4" )
	PORT_DIPSETTING(      0x2400, "German-5" )
	PORT_DIPSETTING(      0x2600, "German-ECA" )
	PORT_DIPSETTING(      0x1e00, "French-1" )
	PORT_DIPSETTING(      0x1c00, "French-2" )
	PORT_DIPSETTING(      0x1a00, "French-3" )
	PORT_DIPSETTING(      0x1800, "French-4" )
	PORT_DIPSETTING(      0x1400, "French-11" )
	PORT_DIPSETTING(      0x1200, "French-12" )
	PORT_DIPSETTING(      0x1600, "French-ECA" )
	PORT_DIPSETTING(      0x3000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:7") // Manual shows Not Used
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Test Switch" )           PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT )     // Slam Switch
	PORT_SERVICE_NO_TOGGLE( 0x0010, IP_ACTIVE_LOW )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0x6000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BILL1 )    // Bill

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0xff80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( wargodsa ) // For Medium Res only versions
	PORT_INCLUDE(wargods)

	PORT_MODIFY("DIPS")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:1") // Manual shows Not Used (must be Off)
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void midvunit_base_state::midvcommon(machine_config &config)
{
	constexpr XTAL CPU_CLOCK = 50_MHz_XTAL;

	// basic machine hardware
	TMS32031(config, m_maincpu, CPU_CLOCK);

	TIMER(config, m_timer[0]).configure_generic(nullptr);
	TIMER(config, m_timer[1]).configure_generic(nullptr);

	WATCHDOG_TIMER(config, m_watchdog);

	// video hardware
	PALETTE(config, m_palette).set_entries(32768);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(MIDVUNIT_VIDEO_CLOCK/2, 666, 0, 512, 432, 0, 400);
	m_screen->set_screen_update(FUNC(midvunit_base_state::screen_update));
	m_screen->set_palette(m_palette);
}


void midvunit_state::midvunit(machine_config &config)
{
	midvcommon(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &midvunit_state::midvunit_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	ADC0844(config, m_adc);
	m_adc->intr_callback().set_inputline("maincpu", 3);
	m_adc->ch1_callback().set_ioport("WHEEL");
	m_adc->ch2_callback().set_ioport("ACCEL");
	m_adc->ch3_callback().set_ioport("BRAKE");

	// sound hardware
	SPEAKER(config, "mono").front_center();

	DCS_AUDIO_2K(config, m_dcs, 0);
	m_dcs->set_maincpu_tag(m_maincpu);
	m_dcs->add_route(0, "mono", 1.0);
}


void crusnwld_state::crusnwld(machine_config &config)
{
	midvunit(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &crusnwld_state::crusnwld_map);

	// valid values are 450 or 460
	MIDWAY_SERIAL_PIC2(config, m_midway_serial_pic2, 0);
	m_midway_serial_pic2->set_upper(450);
	m_midway_serial_pic2->set_yearoffs(94);
}

void crusnwld_state::offroadc(machine_config &config)
{
	midvunit(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &crusnwld_state::offroadc_map);

	// valid values are 230 or 234
	MIDWAY_SERIAL_PIC2(config, m_midway_serial_pic2, 0);
	m_midway_serial_pic2->set_upper(230);
	m_midway_serial_pic2->set_yearoffs(94);
}

void midvplus_state::midvplus(machine_config &config)
{
	midvcommon(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &midvplus_state::midvplus_map);
	m_maincpu->xf1().set(FUNC(midvplus_state::midvplus_xf1_w));

	ATA_INTERFACE(config, m_ata).options(ata_devices, "hdd", nullptr, true);

	MIDWAY_IOASIC(config, m_midway_ioasic, 0);
	m_midway_ioasic->in_port_cb<0>().set_ioport("DIPS");
	m_midway_ioasic->in_port_cb<1>().set_ioport("SYSTEM");
	m_midway_ioasic->in_port_cb<2>().set_ioport("IN1");
	m_midway_ioasic->in_port_cb<3>().set_ioport("IN2");
	m_midway_ioasic->set_dcs_tag(m_dcs);
	m_midway_ioasic->set_shuffle(0);
	m_midway_ioasic->set_upper(452); // no alternates
	m_midway_ioasic->set_yearoffs(94);

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	DCS2_AUDIO_2115(config, m_dcs, 0);
	m_dcs->set_maincpu_tag(m_maincpu);
	m_dcs->set_dram_in_mb(2);
	m_dcs->set_polling_offset(0x3839);
	m_dcs->add_route(0, "rspeaker", 1.0);
	m_dcs->add_route(1, "lspeaker", 1.0);
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

/*
Cruis'n USA & Offroad Challenge (Midway V-Unit)
Midway, 1994

PCB Layout
----------

5770-14365-02 (C) 1994 NINTENDO (for Cruisin USA)
5770-14365-05 WILLIAMS ELECTRONICS GAMES INC. (for Offroad Challenge)
|-------------------------------------------------------------------------------|
|  SOUND.U5  SOUND.U9   BATTERY  MAX691  GAME.U26  GAME.U27  GAME.U28  GAME.U29 |
|  SOUND.U4  SOUND.U8                    GAME.U22  GAME.U23  GAME.U24  GAME.U25 |
|  SOUND.U3  SOUND.U7   6264  RESET_SW   GAME.U18  GAME.U19  GAME.U20  GAME.U21 |
|  SOUND.U2  SOUND.U6         PIC16C57   GAME.U14  GAME.U15  GAME.U16  GAME.U17 |
|                                        GAME.U10  GAME.U11  GAME.U12  GAME.U13 |
|            6116            PAL2              IDT7204      IDT7204             |
|                                          LH521007 LH521007   LH521007 LH521007|
|            6116   6116     PAL3                                               |
|             PAL1           PAL4      |----------|    4C4001 4C4001  |-------| |
|AD1851                      33.3333MHz|LSI       |    4C4001 4C4001  |TMS    | |
|                                      |L1A7968   |    4C4001 4C4001  |320C31 | |
|                                 40MHz|5410-1346500   4C4001 4C4001  |DSP    | |
|                                      |MIDWAY    |                   |-------| |
|      10MHz   ADSP-2105   CY7C199     |----------|                   50MHz     |
|TDA2030                   CY7C199                                        DSW(8)|
|       TL084                             D482234  D482234                      |
|     TL084                               D482234  D482234                DSW(8)|
|TDA2030                        PAL5            SN75160   SN75160   PAL6        |
|          P5                             P7      SN75176  SN75176    P11       |
|P3      P4  |--|        JAMMA         |--|P8 P9  SN75176  SN75176 P10   ADC0844|
|------------|  |----------------------|  |-------------------------------------|
Notes:
      TMS320C31 - Texas Instruments TMS320C31 32-bit Floating-Point DSP, clock input 50.000MHz
      ADSP-2105 - Analog Devices ADSP-2105 16-bit Fixed-Point DSP Microprocessor with On-Chip Memory, clock input 10.000MHz
      IDT7204   - IDT7204 4k x9 Async. FIFO
      LH521007  - Sharp LH521007AK-17 128k x8 SRAM (SOJ32)
      D482234   - NEC D482234LE-70 (possibly 256k x8 ?) DRAM (SOJ40)
      4C4001    - Micron Technology 4C4001JDJ-6 1M x4 DRAM (SOJ24/20)
      CY7C199   - Cypress CY7C199-20PC 32k x8 SRAM
      6264      - 8k x8 SRAM (battery-backed)
      6116      - 2k x8 SRAM
      AD1851    - Analog Devices AD1851 16 bit PCM Audio DAC
      MAX691    - Maxim MAX691 Master Reset IC (DIP16)
      TDA2030   - ST TDA2030 Audio AMP
      TL084     - Texas Instruments TL084 JFET-Input Operational Amplifier
      ADC0844   - National Semiconductor ADC0844 8-Bit Microprocessor Compatible A/D Converter with Multiplexer Option
      PIC16C57  - Microchip PIC16C57
                    - not populated for Cruis'n USA
                    - labelled 'Offroad 25" U904' for Offroad Challenge
      PAL1      - GAL20V8 labelled 'A-19668'
      PAL2      - PALC22V10
                    - labelled 'A-19669' for Cruis'n USA rev 2.1
                    - labelled 'A-19993' for Cruis'n USA rev 4.1 (might work with other revs too, but not 2.x)
                    - labelled 'A-21883 U38' for Offroad Challenge
      PAL3      - TIBPAL20L8
                    - labelled 'A-19670' for Cruis'n USA
                    - labelled 'A-21170 U43' for Offroad Challenge
      PAL4      - TIBPAL22V10
                    - labelled 'A-19671' for Cruis'n USA
                    - labelled 'A-21171 U54' for Offroad Challenge
      PAL5      - TIBPAL22V10
                    - labelled 'A-19672' for Cruis'n USA
                    - labelled 'A-21884 U114' for Offroad Challenge
      PAL6      - TIBPAL22V10
                    - labelled 'A-19673' for Cruis'n USA
                    - no label for Offroad Challenge
      P3 - P11  - various connectors for controls
      VSync     - 57.7090Hz  \
      HSync     - 24.807kHz  / measured via EL4583
      ROMs      - All ROMs 27C040 or 27C801
                  SOUND.Uxx - Sound ROMs
                  GAME.Uxx  - PROGRAM ROMs (including GFX)
*/

/*
Labels for v4.4 were in the following format:

------------------------
|    3/15/95           |
)  CRUIS'N USA  U10    |
|  (C)1995 Midway 87DF |
------------------------

Each label had it's corresponding U position and checksum value.
Also had V4.4 printed sideways at front of the label.

*/
ROM_START( crusnusa ) // Version 4.5, Tue Apr 11 1995 - 16:06:48
	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  // sound data
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u2.u2", 0x000000, 0x80000, CRC(b9338332) SHA1(e5c420e63c4eba0010a68c7e0a57ef210e2c83d2) ) // also known to be labeled as P2, the P1 revision hasn't been dumped or confirmed to be different
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u3.u3", 0x200000, 0x80000, CRC(cd8325d6) SHA1(d65d7263e056ca1d637adb44cafef523e0831a34) )
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u4.u4", 0x400000, 0x80000, CRC(fab457f3) SHA1(2b4b647838b7a8100afc25ca1ffdc74ed67ae00a) )
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u5.u5", 0x600000, 0x80000, CRC(becc92f4) SHA1(6dffa73ff5270155c44f295e443d5e77c03c0338) )
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u6.u6", 0x800000, 0x80000, CRC(a9f915d3) SHA1(6a16a2d7a807a775673e7121b54f37c583581203) )
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u7.u7", 0xa00000, 0x80000, CRC(424f0bbc) SHA1(f38a431fc0fb7102c51f2d5b6f716dd4669a9822) )
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u8.u8", 0xc00000, 0x80000, CRC(03c28199) SHA1(393b009acd3eceb346b8fff45ae2bdf4f53d041f) )
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u9.u9", 0xe00000, 0x80000, CRC(24ba6371) SHA1(f60a9ff73b3645e2c8bad67e2f6debc61b5e0653) )

	ROM_REGION32_LE( 0x1000000, "maindata", 0 ) // boot screen reports as HEAD 2 HEAD
	ROM_LOAD32_BYTE( "v4.5_4-11-95_cruisn_usa_u10_86b3.u10", 0x000000, 0x80000, CRC(b623eab9) SHA1(a523b0ee039904137e274bdd87f1962ea9ab3d38) ) // V4.5   4/11/95   CRUIS'N USA U10   (C)1995 Midway 86B3
	ROM_LOAD32_BYTE( "v4.5_4-11-95_cruisn_usa_u11_6d73.u11", 0x000001, 0x80000, CRC(2e47ee94) SHA1(6454093c4e77074c8d0ef1050c8b49043c44fe23) ) // V4.5   4/11/95   CRUIS'N USA U11   (C)1995 Midway 6D73
	ROM_LOAD32_BYTE( "v4.5_4-11-95_cruisn_usa_u12_4b32.u12", 0x000002, 0x80000, CRC(e9197942) SHA1(cfb6a47be35365f9d56721b4bb0bb6ab9163da8f) ) // V4.5   4/11/95   CRUIS'N USA U12   (C)1995 Midway 4B32
	ROM_LOAD32_BYTE( "v4.5_4-11-95_cruisn_usa_u13_430e.u13", 0x000003, 0x80000, CRC(4a7c434c) SHA1(bf3b4f22aa9445e1b0ebbbd54eebbf14ccaba8d2) ) // V4.5   4/11/95   CRUIS'N USA U13   (C)1995 Midway 430E
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u14.u14", 0x200000, 0x80000, CRC(6a4ae622) SHA1(f488e7616371125d5aef2047b8e0fc954ca4b9b4) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u15.u15", 0x200001, 0x80000, CRC(1a0ad3b7) SHA1(a5300f3c789a4d9d257fda3a280e882f17f4a99f) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u16.u16", 0x200002, 0x80000, CRC(799d4dd6) SHA1(f1208967544477005924f2a553037e0ffbc668ab) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u17.u17", 0x200003, 0x80000, CRC(3d68b660) SHA1(3f14e32c205a504ef39abf1e390bd8031d9d7b5b) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u18.u18", 0x400000, 0x80000, CRC(9e8193fb) SHA1(ec88c2b51bb607d3181e467f8b255c13efebc73c) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u19.u19", 0x400001, 0x80000, CRC(0bf60cde) SHA1(6c63b3eacaefeb405c8fdf641437786262bcb10d) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u20.u20", 0x400002, 0x80000, CRC(c07f68f0) SHA1(444ccf8e49fd9c0f707ab32347984ca5628207f9) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u21.u21", 0x400003, 0x80000, CRC(b0264aed) SHA1(d6a6eca4e4ecedfbc5590dbd06870761155ae8c5) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u22.u22", 0x600000, 0x80000, CRC(ad137193) SHA1(642a7c37940cb3b2b190661da7b1d4848c7c513d) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u23.u23", 0x600001, 0x80000, CRC(842449b0) SHA1(b23ebe28ff3c6a268ff9ae1242a4392d2305396b) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u24.u24", 0x600002, 0x80000, CRC(0b2275be) SHA1(3dc79095064cc158d37218c9a038b5b7a777fc66) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u25.u25", 0x600003, 0x80000, CRC(2b9fe68f) SHA1(2750613e61c1eaac629ef5b9e89fd88e99a262cc) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u26.u26", 0x800000, 0x80000, CRC(ae56b871) SHA1(1e218426084123c6c2389d96ce92691010012aa4) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u27.u27", 0x800001, 0x80000, CRC(2d977a8e) SHA1(8f4d511bfd6c3bee18daa7253be1a27d079aec8f) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u28.u28", 0x800002, 0x80000, CRC(cffa5fb1) SHA1(fb73bc8f65b604c374f88d0ecf06c50ef52f0547) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u29.u29", 0x800003, 0x80000, CRC(cbe52c60) SHA1(3f309ce8ef1784c830f4160cfe76dc3a0b438cac) )

	ROM_REGION( 0x0b33, "pals", 0 ) // all protected
	ROM_LOAD("a-19993.u38",  0x0000, 0x02e5, BAD_DUMP CRC(7e8b7b0d) SHA1(f9af19da171f949a11c5548da7b4277aecb6f2a8) ) // TIBPAL22V10-15BCNT
	ROM_LOAD("a-19670.u43",  0x0000, 0x0144, BAD_DUMP CRC(acafcc97) SHA1(b6f916838d08590a536fe925ec62d66e6ea3dcbc) ) // TIBPAL20L8-10CNT
	ROM_LOAD("a-19668.u52",  0x0000, 0x0157, BAD_DUMP CRC(7915134e) SHA1(aeb22e46abdc14a9e9b34cfe3b77da3e29b789fe) ) // GAL20V8B
	ROM_LOAD("a-19671.u54",  0x0000, 0x02dd, BAD_DUMP CRC(b9cce038) SHA1(8d1df026bdac66ea5493e9e51c23f8eb182b024e) ) // TIBPAL22V10-15BCNT
	ROM_LOAD("a-19673.u111", 0x0000, 0x02dd, BAD_DUMP CRC(8552977d) SHA1(a1a53d797697682b3f18893a90b6bef39ebb069e) ) // TIBPAL22V10-15BCNT
	ROM_LOAD("a-19672.u114", 0x0000, 0x0001, NO_DUMP ) // TIBPAL22V10-15BCNT
ROM_END

ROM_START( crusnusa44 ) // Version 4.4, Wed Mar 15 1995 - 10:52:28
	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  // sound data
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u2.u2", 0x000000, 0x80000, CRC(b9338332) SHA1(e5c420e63c4eba0010a68c7e0a57ef210e2c83d2) ) // also known to be labeled as P2, the P1 revision hasn't been dumped or confirmed to be different
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u3.u3", 0x200000, 0x80000, CRC(cd8325d6) SHA1(d65d7263e056ca1d637adb44cafef523e0831a34) )
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u4.u4", 0x400000, 0x80000, CRC(fab457f3) SHA1(2b4b647838b7a8100afc25ca1ffdc74ed67ae00a) )
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u5.u5", 0x600000, 0x80000, CRC(becc92f4) SHA1(6dffa73ff5270155c44f295e443d5e77c03c0338) )
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u6.u6", 0x800000, 0x80000, CRC(a9f915d3) SHA1(6a16a2d7a807a775673e7121b54f37c583581203) )
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u7.u7", 0xa00000, 0x80000, CRC(424f0bbc) SHA1(f38a431fc0fb7102c51f2d5b6f716dd4669a9822) )
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u8.u8", 0xc00000, 0x80000, CRC(03c28199) SHA1(393b009acd3eceb346b8fff45ae2bdf4f53d041f) )
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u9.u9", 0xe00000, 0x80000, CRC(24ba6371) SHA1(f60a9ff73b3645e2c8bad67e2f6debc61b5e0653) )

	ROM_REGION32_LE( 0x1000000, "maindata", 0 ) // boot screen reports as HEAD 2 HEAD
	ROM_LOAD32_BYTE( "v4.4_3-15-95_cruisn_usa_u10_87df.u10", 0x000000, 0x80000, CRC(e77e2a0c) SHA1(18fa0f1dccba743b9f0f2bb986c35dfb1cf5852a) ) // V4.4   3/15/95   CRUIS'N USA U10   (C)1995 Midway 87DF
	ROM_LOAD32_BYTE( "v4.4_3-15-95_cruisn_usa_u11_6e35.u11", 0x000001, 0x80000, CRC(6eb3e187) SHA1(f6d26998a302dc7de5a18d811fab90a042f01284) ) // V4.4   3/15/95   CRUIS'N USA U11   (C)1995 Midway 6E35
	ROM_LOAD32_BYTE( "v4.4_3-15-95_cruisn_usa_u12_4c08.u12", 0x000002, 0x80000, CRC(5942b6d6) SHA1(86b0422a0e1348d1c4136a23d1e17653bc3a1b0e) ) // V4.4   3/15/95   CRUIS'N USA U12   (C)1995 Midway 4C08
	ROM_LOAD32_BYTE( "v4.4_3-15-95_cruisn_usa_u13_43ea.u13", 0x000003, 0x80000, CRC(9da2124a) SHA1(36d61bbd0c991b415a8c85b58de77d94d2c61bcc) ) // V4.4   3/15/95   CRUIS'N USA U13   (C)1995 Midway 43EA
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u14.u14", 0x200000, 0x80000, CRC(6a4ae622) SHA1(f488e7616371125d5aef2047b8e0fc954ca4b9b4) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u15.u15", 0x200001, 0x80000, CRC(1a0ad3b7) SHA1(a5300f3c789a4d9d257fda3a280e882f17f4a99f) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u16.u16", 0x200002, 0x80000, CRC(799d4dd6) SHA1(f1208967544477005924f2a553037e0ffbc668ab) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u17.u17", 0x200003, 0x80000, CRC(3d68b660) SHA1(3f14e32c205a504ef39abf1e390bd8031d9d7b5b) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u18.u18", 0x400000, 0x80000, CRC(9e8193fb) SHA1(ec88c2b51bb607d3181e467f8b255c13efebc73c) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u19.u19", 0x400001, 0x80000, CRC(0bf60cde) SHA1(6c63b3eacaefeb405c8fdf641437786262bcb10d) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u20.u20", 0x400002, 0x80000, CRC(c07f68f0) SHA1(444ccf8e49fd9c0f707ab32347984ca5628207f9) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u21.u21", 0x400003, 0x80000, CRC(b0264aed) SHA1(d6a6eca4e4ecedfbc5590dbd06870761155ae8c5) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u22.u22", 0x600000, 0x80000, CRC(ad137193) SHA1(642a7c37940cb3b2b190661da7b1d4848c7c513d) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u23.u23", 0x600001, 0x80000, CRC(842449b0) SHA1(b23ebe28ff3c6a268ff9ae1242a4392d2305396b) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u24.u24", 0x600002, 0x80000, CRC(0b2275be) SHA1(3dc79095064cc158d37218c9a038b5b7a777fc66) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u25.u25", 0x600003, 0x80000, CRC(2b9fe68f) SHA1(2750613e61c1eaac629ef5b9e89fd88e99a262cc) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u26.u26", 0x800000, 0x80000, CRC(ae56b871) SHA1(1e218426084123c6c2389d96ce92691010012aa4) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u27.u27", 0x800001, 0x80000, CRC(2d977a8e) SHA1(8f4d511bfd6c3bee18daa7253be1a27d079aec8f) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u28.u28", 0x800002, 0x80000, CRC(cffa5fb1) SHA1(fb73bc8f65b604c374f88d0ecf06c50ef52f0547) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u29.u29", 0x800003, 0x80000, CRC(cbe52c60) SHA1(3f309ce8ef1784c830f4160cfe76dc3a0b438cac) )

	ROM_REGION( 0x0b33, "pals", 0 ) // all protected
	ROM_LOAD("a-19993.u38",  0x0000, 0x02e5, BAD_DUMP CRC(7e8b7b0d) SHA1(f9af19da171f949a11c5548da7b4277aecb6f2a8) ) // TIBPAL22V10-15BCNT
	ROM_LOAD("a-19670.u43",  0x0000, 0x0144, BAD_DUMP CRC(acafcc97) SHA1(b6f916838d08590a536fe925ec62d66e6ea3dcbc) ) // TIBPAL20L8-10CNT
	ROM_LOAD("a-19668.u52",  0x0000, 0x0157, BAD_DUMP CRC(7915134e) SHA1(aeb22e46abdc14a9e9b34cfe3b77da3e29b789fe) ) // GAL20V8B
	ROM_LOAD("a-19671.u54",  0x0000, 0x02dd, BAD_DUMP CRC(b9cce038) SHA1(8d1df026bdac66ea5493e9e51c23f8eb182b024e) ) // TIBPAL22V10-15BCNT
	ROM_LOAD("a-19673.u111", 0x0000, 0x02dd, BAD_DUMP CRC(8552977d) SHA1(a1a53d797697682b3f18893a90b6bef39ebb069e) ) // TIBPAL22V10-15BCNT
	ROM_LOAD("a-19672.u114", 0x0000, 0x0001, NO_DUMP ) // TIBPAL22V10-15BCNT
ROM_END

ROM_START( crusnusa41 ) // Version 4.1, Mon Feb 13 1995 - 16:53:40
	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  // sound data
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u2.u2", 0x000000, 0x80000, CRC(b9338332) SHA1(e5c420e63c4eba0010a68c7e0a57ef210e2c83d2) ) // also known to be labeled as P2, the P1 revision hasn't been dumped or confirmed to be different
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u3.u3", 0x200000, 0x80000, CRC(cd8325d6) SHA1(d65d7263e056ca1d637adb44cafef523e0831a34) )
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u4.u4", 0x400000, 0x80000, CRC(fab457f3) SHA1(2b4b647838b7a8100afc25ca1ffdc74ed67ae00a) )
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u5.u5", 0x600000, 0x80000, CRC(becc92f4) SHA1(6dffa73ff5270155c44f295e443d5e77c03c0338) )
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u6.u6", 0x800000, 0x80000, CRC(a9f915d3) SHA1(6a16a2d7a807a775673e7121b54f37c583581203) )
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u7.u7", 0xa00000, 0x80000, CRC(424f0bbc) SHA1(f38a431fc0fb7102c51f2d5b6f716dd4669a9822) )
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u8.u8", 0xc00000, 0x80000, CRC(03c28199) SHA1(393b009acd3eceb346b8fff45ae2bdf4f53d041f) )
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u9.u9", 0xe00000, 0x80000, CRC(24ba6371) SHA1(f60a9ff73b3645e2c8bad67e2f6debc61b5e0653) )

	ROM_REGION32_LE( 0x1000000, "maindata", 0 ) //  boot screen reports as HEAD 2 HEAD - Also known to be labeled as: CRUIS'N LINK   U10 REV. 4.1  (C) 1994 MIDWAY CORP
	ROM_LOAD32_BYTE( "l4.1_dual_game_linking_cruisin_u.s.a._u10_game_rom.u10", 0x000000, 0x80000, CRC(eb9372d1) SHA1(ab1e489b23b4540c4e0d1d9a6c9a2c9317f5c099) ) // L4.1   DUAL GAME LINKING   CRUISIN U.S.A.   U 10 GAME ROM
	ROM_LOAD32_BYTE( "l4.1_dual_game_linking_cruisin_u.s.a._u11_game_rom.u11", 0x000001, 0x80000, CRC(76f3cd40) SHA1(52276841944ada54d56ecd2da95998aabd699465) ) // L4.1   DUAL GAME LINKING   CRUISIN U.S.A.   U 11 GAME ROM
	ROM_LOAD32_BYTE( "l4.1_dual_game_linking_cruisin_u.s.a._u12_game_rom.u12", 0x000002, 0x80000, CRC(9021a376) SHA1(6a838d49bec4201e8ead7491e3b6d4a3a52dcb12) ) // L4.1   DUAL GAME LINKING   CRUISIN U.S.A.   U 12 GAME ROM
	ROM_LOAD32_BYTE( "l4.1_dual_game_linking_cruisin_u.s.a._u13_game_rom.u13", 0x000003, 0x80000, CRC(1687c932) SHA1(45947c0c22bd4e6640f792d0c7fd06a1f4483131) ) // L4.1   DUAL GAME LINKING   CRUISIN U.S.A.   U 13 GAME ROM
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u14.u14", 0x200000, 0x80000, CRC(6a4ae622) SHA1(f488e7616371125d5aef2047b8e0fc954ca4b9b4) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u15.u15", 0x200001, 0x80000, CRC(1a0ad3b7) SHA1(a5300f3c789a4d9d257fda3a280e882f17f4a99f) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u16.u16", 0x200002, 0x80000, CRC(799d4dd6) SHA1(f1208967544477005924f2a553037e0ffbc668ab) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u17.u17", 0x200003, 0x80000, CRC(3d68b660) SHA1(3f14e32c205a504ef39abf1e390bd8031d9d7b5b) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u18.u18", 0x400000, 0x80000, CRC(9e8193fb) SHA1(ec88c2b51bb607d3181e467f8b255c13efebc73c) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u19.u19", 0x400001, 0x80000, CRC(0bf60cde) SHA1(6c63b3eacaefeb405c8fdf641437786262bcb10d) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u20.u20", 0x400002, 0x80000, CRC(c07f68f0) SHA1(444ccf8e49fd9c0f707ab32347984ca5628207f9) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u21.u21", 0x400003, 0x80000, CRC(b0264aed) SHA1(d6a6eca4e4ecedfbc5590dbd06870761155ae8c5) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u22.u22", 0x600000, 0x80000, CRC(ad137193) SHA1(642a7c37940cb3b2b190661da7b1d4848c7c513d) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u23.u23", 0x600001, 0x80000, CRC(842449b0) SHA1(b23ebe28ff3c6a268ff9ae1242a4392d2305396b) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u24.u24", 0x600002, 0x80000, CRC(0b2275be) SHA1(3dc79095064cc158d37218c9a038b5b7a777fc66) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u25.u25", 0x600003, 0x80000, CRC(2b9fe68f) SHA1(2750613e61c1eaac629ef5b9e89fd88e99a262cc) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u26.u26", 0x800000, 0x80000, CRC(ae56b871) SHA1(1e218426084123c6c2389d96ce92691010012aa4) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u27.u27", 0x800001, 0x80000, CRC(2d977a8e) SHA1(8f4d511bfd6c3bee18daa7253be1a27d079aec8f) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u28.u28", 0x800002, 0x80000, CRC(cffa5fb1) SHA1(fb73bc8f65b604c374f88d0ecf06c50ef52f0547) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u29.u29", 0x800003, 0x80000, CRC(cbe52c60) SHA1(3f309ce8ef1784c830f4160cfe76dc3a0b438cac) )

	ROM_REGION( 0x0b33, "pals", 0 ) // all protected
	ROM_LOAD("a-19993.u38",  0x0000, 0x02e5, BAD_DUMP CRC(7e8b7b0d) SHA1(f9af19da171f949a11c5548da7b4277aecb6f2a8) ) // TIBPAL22V10-15BCNT
	ROM_LOAD("a-19670.u43",  0x0000, 0x0144, BAD_DUMP CRC(acafcc97) SHA1(b6f916838d08590a536fe925ec62d66e6ea3dcbc) ) // TIBPAL20L8-10CNT
	ROM_LOAD("a-19668.u52",  0x0000, 0x0157, BAD_DUMP CRC(7915134e) SHA1(aeb22e46abdc14a9e9b34cfe3b77da3e29b789fe) ) // GAL20V8B
	ROM_LOAD("a-19671.u54",  0x0000, 0x02dd, BAD_DUMP CRC(b9cce038) SHA1(8d1df026bdac66ea5493e9e51c23f8eb182b024e) ) // TIBPAL22V10-15BCNT
	ROM_LOAD("a-19673.u111", 0x0000, 0x02dd, BAD_DUMP CRC(8552977d) SHA1(a1a53d797697682b3f18893a90b6bef39ebb069e) ) // TIBPAL22V10-15BCNT
	ROM_LOAD("a-19672.u114", 0x0000, 0x0001, NO_DUMP ) // TIBPAL22V10-15BCNT
ROM_END


ROM_START( crusnusa40 ) // Version 4.0, Wed Feb 08 1995 - 10:45:14
	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  // sound data
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u2.u2", 0x000000, 0x80000, CRC(b9338332) SHA1(e5c420e63c4eba0010a68c7e0a57ef210e2c83d2) ) // also known to be labeled as P2, the P1 revision hasn't been dumped or confirmed to be different
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u3.u3", 0x200000, 0x80000, CRC(cd8325d6) SHA1(d65d7263e056ca1d637adb44cafef523e0831a34) )
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u4.u4", 0x400000, 0x80000, CRC(fab457f3) SHA1(2b4b647838b7a8100afc25ca1ffdc74ed67ae00a) )
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u5.u5", 0x600000, 0x80000, CRC(becc92f4) SHA1(6dffa73ff5270155c44f295e443d5e77c03c0338) )
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u6.u6", 0x800000, 0x80000, CRC(a9f915d3) SHA1(6a16a2d7a807a775673e7121b54f37c583581203) )
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u7.u7", 0xa00000, 0x80000, CRC(424f0bbc) SHA1(f38a431fc0fb7102c51f2d5b6f716dd4669a9822) )
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u8.u8", 0xc00000, 0x80000, CRC(03c28199) SHA1(393b009acd3eceb346b8fff45ae2bdf4f53d041f) )
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u9.u9", 0xe00000, 0x80000, CRC(24ba6371) SHA1(f60a9ff73b3645e2c8bad67e2f6debc61b5e0653) )

	ROM_REGION32_LE( 0x1000000, "maindata", 0 ) // boot screen reports as HEAD 2 HEAD
	ROM_LOAD32_BYTE( "l4_cruisin_u.s.a._game_rom_u10.u10", 0x000000, 0x80000, CRC(7526d8bf) SHA1(ef00ea3b6e1923d3e4d10bf3601b080a009fb711) )
	ROM_LOAD32_BYTE( "l4_cruisin_u.s.a._game_rom_u11.u11", 0x000001, 0x80000, CRC(bfc691b9) SHA1(41d1503c4290e396a49043fea7778851cdf11310) )
	ROM_LOAD32_BYTE( "l4_cruisin_u.s.a._game_rom_u12.u12", 0x000002, 0x80000, CRC(059c2234) SHA1(145ec1ab3a46c3316f39bd731730dcb57b55b4ec) )
	ROM_LOAD32_BYTE( "l4_cruisin_u.s.a._game_rom_u13.u13", 0x000003, 0x80000, CRC(39e0ff7d) SHA1(3b0f95bf2a6999b8ec8722e0bc0f3a60264469aa) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u14.u14", 0x200000, 0x80000, CRC(6a4ae622) SHA1(f488e7616371125d5aef2047b8e0fc954ca4b9b4) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u15.u15", 0x200001, 0x80000, CRC(1a0ad3b7) SHA1(a5300f3c789a4d9d257fda3a280e882f17f4a99f) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u16.u16", 0x200002, 0x80000, CRC(799d4dd6) SHA1(f1208967544477005924f2a553037e0ffbc668ab) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u17.u17", 0x200003, 0x80000, CRC(3d68b660) SHA1(3f14e32c205a504ef39abf1e390bd8031d9d7b5b) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u18.u18", 0x400000, 0x80000, CRC(9e8193fb) SHA1(ec88c2b51bb607d3181e467f8b255c13efebc73c) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u19.u19", 0x400001, 0x80000, CRC(0bf60cde) SHA1(6c63b3eacaefeb405c8fdf641437786262bcb10d) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u20.u20", 0x400002, 0x80000, CRC(c07f68f0) SHA1(444ccf8e49fd9c0f707ab32347984ca5628207f9) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u21.u21", 0x400003, 0x80000, CRC(b0264aed) SHA1(d6a6eca4e4ecedfbc5590dbd06870761155ae8c5) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u22.u22", 0x600000, 0x80000, CRC(ad137193) SHA1(642a7c37940cb3b2b190661da7b1d4848c7c513d) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u23.u23", 0x600001, 0x80000, CRC(842449b0) SHA1(b23ebe28ff3c6a268ff9ae1242a4392d2305396b) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u24.u24", 0x600002, 0x80000, CRC(0b2275be) SHA1(3dc79095064cc158d37218c9a038b5b7a777fc66) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u25.u25", 0x600003, 0x80000, CRC(2b9fe68f) SHA1(2750613e61c1eaac629ef5b9e89fd88e99a262cc) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u26.u26", 0x800000, 0x80000, CRC(ae56b871) SHA1(1e218426084123c6c2389d96ce92691010012aa4) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u27.u27", 0x800001, 0x80000, CRC(2d977a8e) SHA1(8f4d511bfd6c3bee18daa7253be1a27d079aec8f) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u28.u28", 0x800002, 0x80000, CRC(cffa5fb1) SHA1(fb73bc8f65b604c374f88d0ecf06c50ef52f0547) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u29.u29", 0x800003, 0x80000, CRC(cbe52c60) SHA1(3f309ce8ef1784c830f4160cfe76dc3a0b438cac) )

	ROM_REGION( 0x0b33, "pals", 0 ) // all protected
	ROM_LOAD("a-19993.u38",  0x0000, 0x02e5, BAD_DUMP CRC(7e8b7b0d) SHA1(f9af19da171f949a11c5548da7b4277aecb6f2a8) ) // TIBPAL22V10-15BCNT
	ROM_LOAD("a-19670.u43",  0x0000, 0x0144, BAD_DUMP CRC(acafcc97) SHA1(b6f916838d08590a536fe925ec62d66e6ea3dcbc) ) // TIBPAL20L8-10CNT
	ROM_LOAD("a-19668.u52",  0x0000, 0x0157, BAD_DUMP CRC(7915134e) SHA1(aeb22e46abdc14a9e9b34cfe3b77da3e29b789fe) ) // GAL20V8B
	ROM_LOAD("a-19671.u54",  0x0000, 0x02dd, BAD_DUMP CRC(b9cce038) SHA1(8d1df026bdac66ea5493e9e51c23f8eb182b024e) ) // TIBPAL22V10-15BCNT
	ROM_LOAD("a-19673.u111", 0x0000, 0x02dd, BAD_DUMP CRC(8552977d) SHA1(a1a53d797697682b3f18893a90b6bef39ebb069e) ) // TIBPAL22V10-15BCNT
	ROM_LOAD("a-19672.u114", 0x0000, 0x0001, NO_DUMP ) // TIBPAL22V10-15BCNT
ROM_END


ROM_START( crusnusa21 ) // Version 2.1, Wed Nov 09 1994 - 16:28:10
	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  // sound data
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u2.u2", 0x000000, 0x80000, CRC(b9338332) SHA1(e5c420e63c4eba0010a68c7e0a57ef210e2c83d2) ) // also known to be labeled as P2, the P1 revision hasn't been dumped or confirmed to be different
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u3.u3", 0x200000, 0x80000, CRC(cd8325d6) SHA1(d65d7263e056ca1d637adb44cafef523e0831a34) )
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u4.u4", 0x400000, 0x80000, CRC(fab457f3) SHA1(2b4b647838b7a8100afc25ca1ffdc74ed67ae00a) )
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u5.u5", 0x600000, 0x80000, CRC(becc92f4) SHA1(6dffa73ff5270155c44f295e443d5e77c03c0338) )
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u6.u6", 0x800000, 0x80000, CRC(a9f915d3) SHA1(6a16a2d7a807a775673e7121b54f37c583581203) )
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u7.u7", 0xa00000, 0x80000, CRC(424f0bbc) SHA1(f38a431fc0fb7102c51f2d5b6f716dd4669a9822) )
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u8.u8", 0xc00000, 0x80000, CRC(03c28199) SHA1(393b009acd3eceb346b8fff45ae2bdf4f53d041f) )
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u9.u9", 0xe00000, 0x80000, CRC(24ba6371) SHA1(f60a9ff73b3645e2c8bad67e2f6debc61b5e0653) )

	ROM_REGION32_LE( 0x1000000, "maindata", 0 )
	ROM_LOAD32_BYTE( "l2.1_cruisin_u.s.a._game_rom_u10.u10", 0x000000, 0x80000, CRC(bb759945) SHA1(dbf5270503cb58adb0abd34a8aece5933063ec66) )
	ROM_LOAD32_BYTE( "l2.1_cruisin_u.s.a._game_rom_u11.u11", 0x000001, 0x80000, CRC(4d2da096) SHA1(6ccb9fee095580089f8d43a2e86e0f8a4407dda5) )
	ROM_LOAD32_BYTE( "l2.1_cruisin_u.s.a._game_rom_u12.u12", 0x000002, 0x80000, CRC(4b66fe5e) SHA1(885d31c06b11209a1154789bc84e75d0ac9e1e8a) )
	ROM_LOAD32_BYTE( "l2.1_cruisin_u.s.a._game_rom_u13.u13", 0x000003, 0x80000, CRC(a165359f) SHA1(eefbeaa67282b3826503f4edff84282ff5f45d35) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u14.u14", 0x200000, 0x80000, CRC(6a4ae622) SHA1(f488e7616371125d5aef2047b8e0fc954ca4b9b4) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u15.u15", 0x200001, 0x80000, CRC(1a0ad3b7) SHA1(a5300f3c789a4d9d257fda3a280e882f17f4a99f) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u16.u16", 0x200002, 0x80000, CRC(799d4dd6) SHA1(f1208967544477005924f2a553037e0ffbc668ab) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u17.u17", 0x200003, 0x80000, CRC(3d68b660) SHA1(3f14e32c205a504ef39abf1e390bd8031d9d7b5b) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u18.u18", 0x400000, 0x80000, CRC(9e8193fb) SHA1(ec88c2b51bb607d3181e467f8b255c13efebc73c) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u19.u19", 0x400001, 0x80000, CRC(0bf60cde) SHA1(6c63b3eacaefeb405c8fdf641437786262bcb10d) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u20.u20", 0x400002, 0x80000, CRC(c07f68f0) SHA1(444ccf8e49fd9c0f707ab32347984ca5628207f9) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u21.u21", 0x400003, 0x80000, CRC(b0264aed) SHA1(d6a6eca4e4ecedfbc5590dbd06870761155ae8c5) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u22.u22", 0x600000, 0x80000, CRC(ad137193) SHA1(642a7c37940cb3b2b190661da7b1d4848c7c513d) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u23.u23", 0x600001, 0x80000, CRC(842449b0) SHA1(b23ebe28ff3c6a268ff9ae1242a4392d2305396b) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u24.u24", 0x600002, 0x80000, CRC(0b2275be) SHA1(3dc79095064cc158d37218c9a038b5b7a777fc66) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u25.u25", 0x600003, 0x80000, CRC(2b9fe68f) SHA1(2750613e61c1eaac629ef5b9e89fd88e99a262cc) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u26.u26", 0x800000, 0x80000, CRC(ae56b871) SHA1(1e218426084123c6c2389d96ce92691010012aa4) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u27.u27", 0x800001, 0x80000, CRC(2d977a8e) SHA1(8f4d511bfd6c3bee18daa7253be1a27d079aec8f) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u28.u28", 0x800002, 0x80000, CRC(cffa5fb1) SHA1(fb73bc8f65b604c374f88d0ecf06c50ef52f0547) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u29.u29", 0x800003, 0x80000, CRC(cbe52c60) SHA1(3f309ce8ef1784c830f4160cfe76dc3a0b438cac) )

	ROM_REGION( 0x0b33, "pals", 0 ) // all protected
	ROM_LOAD("a-19669.u38",  0x0000, 0x02dd, NO_DUMP ) // TIBPAL22V10-15BCNT  NOTE: Head to Head games use a different U38 PAL
	ROM_LOAD("a-19670.u43",  0x0000, 0x0144, BAD_DUMP CRC(acafcc97) SHA1(b6f916838d08590a536fe925ec62d66e6ea3dcbc) ) // TIBPAL20L8-10CNT
	ROM_LOAD("a-19668.u52",  0x0000, 0x0157, BAD_DUMP CRC(7915134e) SHA1(aeb22e46abdc14a9e9b34cfe3b77da3e29b789fe) ) // GAL20V8B
	ROM_LOAD("a-19671.u54",  0x0000, 0x02dd, BAD_DUMP CRC(b9cce038) SHA1(8d1df026bdac66ea5493e9e51c23f8eb182b024e) ) // TIBPAL22V10-15BCNT
	ROM_LOAD("a-19673.u111", 0x0000, 0x02dd, BAD_DUMP CRC(8552977d) SHA1(a1a53d797697682b3f18893a90b6bef39ebb069e) ) // TIBPAL22V10-15BCNT
	ROM_LOAD("a-19672.u114", 0x0000, 0x0001, NO_DUMP ) // TIBPAL22V10-15BCNT
ROM_END


ROM_START( crusnusa20 ) // Version 2.0, Tue Oct 25 1994 - 11:49:09
	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  // sound data
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u2.u2", 0x000000, 0x80000, CRC(b9338332) SHA1(e5c420e63c4eba0010a68c7e0a57ef210e2c83d2) ) // also known to be labeled as P2, the P1 revision hasn't been dumped or confirmed to be different
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u3.u3", 0x200000, 0x80000, CRC(cd8325d6) SHA1(d65d7263e056ca1d637adb44cafef523e0831a34) )
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u4.u4", 0x400000, 0x80000, CRC(fab457f3) SHA1(2b4b647838b7a8100afc25ca1ffdc74ed67ae00a) )
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u5.u5", 0x600000, 0x80000, CRC(becc92f4) SHA1(6dffa73ff5270155c44f295e443d5e77c03c0338) )
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u6.u6", 0x800000, 0x80000, CRC(a9f915d3) SHA1(6a16a2d7a807a775673e7121b54f37c583581203) )
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u7.u7", 0xa00000, 0x80000, CRC(424f0bbc) SHA1(f38a431fc0fb7102c51f2d5b6f716dd4669a9822) )
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u8.u8", 0xc00000, 0x80000, CRC(03c28199) SHA1(393b009acd3eceb346b8fff45ae2bdf4f53d041f) )
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u9.u9", 0xe00000, 0x80000, CRC(24ba6371) SHA1(f60a9ff73b3645e2c8bad67e2f6debc61b5e0653) )

	ROM_REGION32_LE( 0x1000000, "maindata", 0 )
	ROM_LOAD32_BYTE( "l2.0_cruisin_u.s.a._game_rom_u10.u10", 0x000000, 0x80000, CRC(5cdeaee9) SHA1(af8efa5969bad1975de67c82b2eec48e27eb9798) )
	ROM_LOAD32_BYTE( "l2.0_cruisin_u.s.a._game_rom_u11.u11", 0x000001, 0x80000, CRC(87aaaeef) SHA1(79e5a35b7eaa086c693347e266276e2c31647fec) )
	ROM_LOAD32_BYTE( "l2.0_cruisin_u.s.a._game_rom_u12.u12", 0x000002, 0x80000, CRC(587b01ad) SHA1(776ffff9764356d513ef25377c1805041869a02a) )
	ROM_LOAD32_BYTE( "l2.0_cruisin_u.s.a._game_rom_u13.u13", 0x000003, 0x80000, CRC(23f1a96c) SHA1(d7ab96f3cbd42e0caa6b67c6de23c44f8c340621) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u14.u14", 0x200000, 0x80000, CRC(6a4ae622) SHA1(f488e7616371125d5aef2047b8e0fc954ca4b9b4) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u15.u15", 0x200001, 0x80000, CRC(1a0ad3b7) SHA1(a5300f3c789a4d9d257fda3a280e882f17f4a99f) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u16.u16", 0x200002, 0x80000, CRC(799d4dd6) SHA1(f1208967544477005924f2a553037e0ffbc668ab) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u17.u17", 0x200003, 0x80000, CRC(3d68b660) SHA1(3f14e32c205a504ef39abf1e390bd8031d9d7b5b) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u18.u18", 0x400000, 0x80000, CRC(9e8193fb) SHA1(ec88c2b51bb607d3181e467f8b255c13efebc73c) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u19.u19", 0x400001, 0x80000, CRC(0bf60cde) SHA1(6c63b3eacaefeb405c8fdf641437786262bcb10d) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u20.u20", 0x400002, 0x80000, CRC(c07f68f0) SHA1(444ccf8e49fd9c0f707ab32347984ca5628207f9) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u21.u21", 0x400003, 0x80000, CRC(b0264aed) SHA1(d6a6eca4e4ecedfbc5590dbd06870761155ae8c5) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u22.u22", 0x600000, 0x80000, CRC(ad137193) SHA1(642a7c37940cb3b2b190661da7b1d4848c7c513d) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u23.u23", 0x600001, 0x80000, CRC(842449b0) SHA1(b23ebe28ff3c6a268ff9ae1242a4392d2305396b) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u24.u24", 0x600002, 0x80000, CRC(0b2275be) SHA1(3dc79095064cc158d37218c9a038b5b7a777fc66) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u25.u25", 0x600003, 0x80000, CRC(2b9fe68f) SHA1(2750613e61c1eaac629ef5b9e89fd88e99a262cc) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u26.u26", 0x800000, 0x80000, CRC(ae56b871) SHA1(1e218426084123c6c2389d96ce92691010012aa4) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u27.u27", 0x800001, 0x80000, CRC(2d977a8e) SHA1(8f4d511bfd6c3bee18daa7253be1a27d079aec8f) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u28.u28", 0x800002, 0x80000, CRC(cffa5fb1) SHA1(fb73bc8f65b604c374f88d0ecf06c50ef52f0547) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u29.u29", 0x800003, 0x80000, CRC(cbe52c60) SHA1(3f309ce8ef1784c830f4160cfe76dc3a0b438cac) )

	ROM_REGION( 0x0b33, "pals", 0 ) // all protected
	ROM_LOAD("a-19669.u38",  0x0000, 0x02dd, NO_DUMP ) // TIBPAL22V10-15BCNT  NOTE: Head to Head games use a different U38 PAL
	ROM_LOAD("a-19670.u43",  0x0000, 0x0144, BAD_DUMP CRC(acafcc97) SHA1(b6f916838d08590a536fe925ec62d66e6ea3dcbc) ) // TIBPAL20L8-10CNT
	ROM_LOAD("a-19668.u52",  0x0000, 0x0157, BAD_DUMP CRC(7915134e) SHA1(aeb22e46abdc14a9e9b34cfe3b77da3e29b789fe) ) // GAL20V8B
	ROM_LOAD("a-19671.u54",  0x0000, 0x02dd, BAD_DUMP CRC(b9cce038) SHA1(8d1df026bdac66ea5493e9e51c23f8eb182b024e) ) // TIBPAL22V10-15BCNT
	ROM_LOAD("a-19673.u111", 0x0000, 0x02dd, BAD_DUMP CRC(8552977d) SHA1(a1a53d797697682b3f18893a90b6bef39ebb069e) ) // TIBPAL22V10-15BCNT
	ROM_LOAD("a-19672.u114", 0x0000, 0x0001, NO_DUMP ) // TIBPAL22V10-15BCNT
ROM_END


ROM_START( crusnusa11 ) // Version 1.1, Wed Aug 31 1994 - 18:44:40
	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  // sound data
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u2.u2", 0x000000, 0x80000, CRC(b9338332) SHA1(e5c420e63c4eba0010a68c7e0a57ef210e2c83d2) )
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u3.u3", 0x200000, 0x80000, CRC(cd8325d6) SHA1(d65d7263e056ca1d637adb44cafef523e0831a34) )
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u4.u4", 0x400000, 0x80000, CRC(fab457f3) SHA1(2b4b647838b7a8100afc25ca1ffdc74ed67ae00a) )
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u5.u5", 0x600000, 0x80000, CRC(becc92f4) SHA1(6dffa73ff5270155c44f295e443d5e77c03c0338) )
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u6.u6", 0x800000, 0x80000, CRC(a9f915d3) SHA1(6a16a2d7a807a775673e7121b54f37c583581203) )
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u7.u7", 0xa00000, 0x80000, CRC(424f0bbc) SHA1(f38a431fc0fb7102c51f2d5b6f716dd4669a9822) )
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u8.u8", 0xc00000, 0x80000, CRC(03c28199) SHA1(393b009acd3eceb346b8fff45ae2bdf4f53d041f) )
	ROM_LOAD16_BYTE( "l1_cruisin_u.s.a._sound_rom_u9.u9", 0xe00000, 0x80000, CRC(24ba6371) SHA1(f60a9ff73b3645e2c8bad67e2f6debc61b5e0653) )

	ROM_REGION32_LE( 0x1000000, "maindata", 0 )
	ROM_LOAD32_BYTE( "l1.1_cruisin_u.s.a._game_rom_u10.u10", 0x000000, 0x80000, CRC(44f36b34) SHA1(6180579631319b3748539c79a016e12e5543185a) )
	ROM_LOAD32_BYTE( "l1.1_cruisin_u.s.a._game_rom_u11.u11", 0x000001, 0x80000, CRC(f0328c25) SHA1(b129f3fb4eb95c5e27119ceced1484077b2fb1f9) )
	ROM_LOAD32_BYTE( "l1.1_cruisin_u.s.a._game_rom_u12.u12", 0x000002, 0x80000, CRC(0e529551) SHA1(30bc072cd175a6ce85f7688f7bb42bcb2ab1d672) )
	ROM_LOAD32_BYTE( "l1.1_cruisin_u.s.a._game_rom_u13.u13", 0x000003, 0x80000, CRC(bd5fa269) SHA1(46b1f7012020e5a960cb743c92cf251de18ff94a) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u14.u14", 0x200000, 0x80000, CRC(6a4ae622) SHA1(f488e7616371125d5aef2047b8e0fc954ca4b9b4) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u15.u15", 0x200001, 0x80000, CRC(1a0ad3b7) SHA1(a5300f3c789a4d9d257fda3a280e882f17f4a99f) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u16.u16", 0x200002, 0x80000, CRC(799d4dd6) SHA1(f1208967544477005924f2a553037e0ffbc668ab) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u17.u17", 0x200003, 0x80000, CRC(3d68b660) SHA1(3f14e32c205a504ef39abf1e390bd8031d9d7b5b) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u18.u18", 0x400000, 0x80000, CRC(9e8193fb) SHA1(ec88c2b51bb607d3181e467f8b255c13efebc73c) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u19.u19", 0x400001, 0x80000, CRC(0bf60cde) SHA1(6c63b3eacaefeb405c8fdf641437786262bcb10d) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u20.u20", 0x400002, 0x80000, CRC(c07f68f0) SHA1(444ccf8e49fd9c0f707ab32347984ca5628207f9) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u21.u21", 0x400003, 0x80000, CRC(b0264aed) SHA1(d6a6eca4e4ecedfbc5590dbd06870761155ae8c5) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u22.u22", 0x600000, 0x80000, CRC(ad137193) SHA1(642a7c37940cb3b2b190661da7b1d4848c7c513d) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u23.u23", 0x600001, 0x80000, CRC(842449b0) SHA1(b23ebe28ff3c6a268ff9ae1242a4392d2305396b) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u24.u24", 0x600002, 0x80000, CRC(0b2275be) SHA1(3dc79095064cc158d37218c9a038b5b7a777fc66) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u25.u25", 0x600003, 0x80000, CRC(2b9fe68f) SHA1(2750613e61c1eaac629ef5b9e89fd88e99a262cc) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u26.u26", 0x800000, 0x80000, CRC(ae56b871) SHA1(1e218426084123c6c2389d96ce92691010012aa4) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u27.u27", 0x800001, 0x80000, CRC(2d977a8e) SHA1(8f4d511bfd6c3bee18daa7253be1a27d079aec8f) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u28.u28", 0x800002, 0x80000, CRC(cffa5fb1) SHA1(fb73bc8f65b604c374f88d0ecf06c50ef52f0547) )
	ROM_LOAD32_BYTE( "l1_cruisin_u.s.a._game_rom_u29.u29", 0x800003, 0x80000, CRC(cbe52c60) SHA1(3f309ce8ef1784c830f4160cfe76dc3a0b438cac) )

	ROM_REGION( 0x0b33, "pals", 0 ) // all protected
	ROM_LOAD("a-19669.u38",  0x0000, 0x02dd, NO_DUMP ) // TIBPAL22V10-15BCNT  NOTE: Head to Head games use a different U38 PAL
	ROM_LOAD("a-19670.u43",  0x0000, 0x0144, BAD_DUMP CRC(acafcc97) SHA1(b6f916838d08590a536fe925ec62d66e6ea3dcbc) ) // TIBPAL20L8-10CNT
	ROM_LOAD("a-19668.u52",  0x0000, 0x0157, BAD_DUMP CRC(7915134e) SHA1(aeb22e46abdc14a9e9b34cfe3b77da3e29b789fe) ) // GAL20V8B
	ROM_LOAD("a-19671.u54",  0x0000, 0x02dd, BAD_DUMP CRC(b9cce038) SHA1(8d1df026bdac66ea5493e9e51c23f8eb182b024e) ) // TIBPAL22V10-15BCNT
	ROM_LOAD("a-19673.u111", 0x0000, 0x02dd, BAD_DUMP CRC(8552977d) SHA1(a1a53d797697682b3f18893a90b6bef39ebb069e) ) // TIBPAL22V10-15BCNT
	ROM_LOAD("a-19672.u114", 0x0000, 0x0001, NO_DUMP ) // TIBPAL22V10-15BCNT
ROM_END


/*
Some Cruis'n World PCBs have mask ROMs for the data ROMs

Mask ROMs are in the following format:
--------------------------------   --------------------------------
|    MIDWAY GAMES INC          |   |    MIDWAY GAMES INC          |
|    CRUISN WORLD              |   |    CRUISN WORLD              |
)    5341-15282-01             |   )    5341-15287-01             |
|    U3 SOUND                  |   |    U14 VIDEO IMAGE           |
|   (C)1996 MIDWAY GAMES       |   |   (C)1996 MIDWAY GAMES       |
--------------------------------   --------------------------------

*/
ROM_START( crusnwld ) // Version 2.5, Wed Nov 04 1998 - 15:50:52
	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  // sound data
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u2_sound.u2", 0x000000, 0x80000, CRC(7a233c89) SHA1(ecfad4bc48a69cd3399e3b3266c81574082e0169) )
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u3_sound.u3", 0x200000, 0x80000, CRC(be9a5ff0) SHA1(98d69dbfa6aa8462cdd46772e991ee418b79c653) )
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u4_sound.u4", 0x400000, 0x80000, CRC(69f02d84) SHA1(0fb4ff750de78505f241ae6cd18fccf3ddf4223f) )
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u5_sound.u5", 0x600000, 0x80000, CRC(9d0b9071) SHA1(05edf9073399a942a9d0b969274a7ebf4ca677da) )
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u6_sound.u6", 0x800000, 0x80000, CRC(df28f492) SHA1(c61f3870f59458b7bb5efbf93d697e3fa44a7830) )
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u7_sound.u7", 0xa00000, 0x80000, CRC(0128913e) SHA1(c11bc115877310c17f9b57f72b29d19b0ad71afa) )
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u8_sound.u8", 0xc00000, 0x80000, CRC(5127c08e) SHA1(4f0eae73817270fa156829100b66f0ff88fa422c) )
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u9_sound.u9", 0xe00000, 0x80000, CRC(84cdc781) SHA1(62287aa72903698d1890908adde53c39f8bd200c) )

	ROM_REGION32_LE( 0x1000000, "maindata", 0 )
	ROM_LOAD32_BYTE( "2.5_cruisn_world_automatic_u10.u10", 0x0000000, 0x100000, CRC(fd776872) SHA1(90df230b58b1c60d1ca7545ef177e5df30b4ea9d) ) // Labeled 2.5 Cruis'n World Automatic U10
	ROM_LOAD32_BYTE( "2.5_cruisn_world_automatic_u11.u11", 0x0000001, 0x100000, CRC(0c99a405) SHA1(14251187f00c198fbaa39817f8c95d1dbec80ec0) ) // Labeled 2.5 Cruis'n World Automatic U11
	ROM_LOAD32_BYTE( "2.5_cruisn_world_automatic_u12.u12", 0x0000002, 0x100000, CRC(3ba9fad8) SHA1(ac8d0dd4df3c1f1c28d93d615d7e24aed4a4a9b5) ) // Labeled 2.5 Cruis'n World Automatic U12
	ROM_LOAD32_BYTE( "2.5_cruisn_world_automatic_u13.u13", 0x0000003, 0x100000, CRC(21a79c9a) SHA1(e33f768c2309613e4936416ee5250d3b1690d11d) ) // Labeled 2.5 Cruis'n World Automatic U13
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u14_image.u14", 0x0400000, 0x100000, CRC(ee815091) SHA1(fb8a99bae07f42966f76a3bb073d7d8280d8efcb) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u15_image.u15", 0x0400001, 0x100000, CRC(e2da7bf1) SHA1(9d9a80055ee62476f47c95e30ec9a989d5d0e25b) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u16_image.u16", 0x0400002, 0x100000, CRC(05a7ad2f) SHA1(4bdfde671379ecefa3f8ceb6fc06e8df5d70fc22) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u17_image.u17", 0x0400003, 0x100000, CRC(d6278c0c) SHA1(3e152d755d69903718a84d4154e442a31026f3d8) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u18_image.u18", 0x0800000, 0x100000, CRC(e2dc2733) SHA1(c277643548c03d831a3b091f1a311accac9d106b) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u19_image.u19", 0x0800001, 0x100000, CRC(5223a070) SHA1(90ce48b2308fa9e7cb636c4732b20b8e177aa9b1) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u20_image.u20", 0x0800002, 0x100000, CRC(db535625) SHA1(599ccd6bcfb155eb68ac131de4af524510ab35b7) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u21_image.u21", 0x0800003, 0x100000, CRC(92a080e8) SHA1(e5e0faf820b5870a81f121b6ad4c37a9081724e4) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u22_image.u22", 0x0c00000, 0x100000, CRC(77c56318) SHA1(52344038942c83f3ce82f3169a345ceb86e43dcb) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u23_image.u23", 0x0c00001, 0x100000, CRC(6b920fc7) SHA1(993da81181f24075e1aead7c4b374f36dd86a9c3) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u24_image.u24", 0x0c00002, 0x100000, CRC(83485401) SHA1(58407818a82a7a3657530dcda7e373e678b58ab2) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u25_image.u25", 0x0c00003, 0x100000, CRC(0dad97a9) SHA1(cdb0c02da35243b118e37ff1519aa6ee1a79d06d) )
ROM_END


ROM_START( crusnwld24 ) // Version 2.4, Thu Feb 19 1998 - 13:43:26
	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  // sound data
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u2_sound.u2", 0x000000, 0x80000, CRC(7a233c89) SHA1(ecfad4bc48a69cd3399e3b3266c81574082e0169) )
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u3_sound.u3", 0x200000, 0x80000, CRC(be9a5ff0) SHA1(98d69dbfa6aa8462cdd46772e991ee418b79c653) )
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u4_sound.u4", 0x400000, 0x80000, CRC(69f02d84) SHA1(0fb4ff750de78505f241ae6cd18fccf3ddf4223f) )
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u5_sound.u5", 0x600000, 0x80000, CRC(9d0b9071) SHA1(05edf9073399a942a9d0b969274a7ebf4ca677da) )
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u6_sound.u6", 0x800000, 0x80000, CRC(df28f492) SHA1(c61f3870f59458b7bb5efbf93d697e3fa44a7830) )
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u7_sound.u7", 0xa00000, 0x80000, CRC(0128913e) SHA1(c11bc115877310c17f9b57f72b29d19b0ad71afa) )
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u8_sound.u8", 0xc00000, 0x80000, CRC(5127c08e) SHA1(4f0eae73817270fa156829100b66f0ff88fa422c) )
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u9_sound.u9", 0xe00000, 0x80000, CRC(84cdc781) SHA1(62287aa72903698d1890908adde53c39f8bd200c) )

	ROM_REGION32_LE( 0x1000000, "maindata", 0 )
	ROM_LOAD32_BYTE( "2.4_cruisn_world_u10_game.u10",  0x0000000, 0x100000, CRC(551ec903) SHA1(f3d983ca5d9a90b2898fb2c3adf8859ab7b43917) )
	ROM_LOAD32_BYTE( "2.4_cruisn_world_u11_game.u11",  0x0000001, 0x100000, CRC(4c57faf2) SHA1(d5717e6222bb59c5aba782bce04aa52c1d148c49) )
	ROM_LOAD32_BYTE( "2.4_cruisn_world_u12_game.u12",  0x0000002, 0x100000, CRC(3a4d9a30) SHA1(ac944555340502e9324df8360c3efc538315e474) )
	ROM_LOAD32_BYTE( "2.4_cruisn_world_u13_game.u13",  0x0000003, 0x100000, CRC(ca6a0c94) SHA1(217659d2ce1b970265df258e330148fef327c6f1) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u14_image.u14", 0x0400000, 0x100000, CRC(ee815091) SHA1(fb8a99bae07f42966f76a3bb073d7d8280d8efcb) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u15_image.u15", 0x0400001, 0x100000, CRC(e2da7bf1) SHA1(9d9a80055ee62476f47c95e30ec9a989d5d0e25b) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u16_image.u16", 0x0400002, 0x100000, CRC(05a7ad2f) SHA1(4bdfde671379ecefa3f8ceb6fc06e8df5d70fc22) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u17_image.u17", 0x0400003, 0x100000, CRC(d6278c0c) SHA1(3e152d755d69903718a84d4154e442a31026f3d8) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u18_image.u18", 0x0800000, 0x100000, CRC(e2dc2733) SHA1(c277643548c03d831a3b091f1a311accac9d106b) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u19_image.u19", 0x0800001, 0x100000, CRC(5223a070) SHA1(90ce48b2308fa9e7cb636c4732b20b8e177aa9b1) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u20_image.u20", 0x0800002, 0x100000, CRC(db535625) SHA1(599ccd6bcfb155eb68ac131de4af524510ab35b7) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u21_image.u21", 0x0800003, 0x100000, CRC(92a080e8) SHA1(e5e0faf820b5870a81f121b6ad4c37a9081724e4) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u22_image.u22", 0x0c00000, 0x100000, CRC(77c56318) SHA1(52344038942c83f3ce82f3169a345ceb86e43dcb) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u23_image.u23", 0x0c00001, 0x100000, CRC(6b920fc7) SHA1(993da81181f24075e1aead7c4b374f36dd86a9c3) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u24_image.u24", 0x0c00002, 0x100000, CRC(83485401) SHA1(58407818a82a7a3657530dcda7e373e678b58ab2) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u25_image.u25", 0x0c00003, 0x100000, CRC(0dad97a9) SHA1(cdb0c02da35243b118e37ff1519aa6ee1a79d06d) )
ROM_END


ROM_START( crusnwld23 ) // Version 2.3, Fri Jan 09 1998 - 10:25:49
	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  // sound data
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u2_sound.u2", 0x000000, 0x80000, CRC(7a233c89) SHA1(ecfad4bc48a69cd3399e3b3266c81574082e0169) )
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u3_sound.u3", 0x200000, 0x80000, CRC(be9a5ff0) SHA1(98d69dbfa6aa8462cdd46772e991ee418b79c653) )
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u4_sound.u4", 0x400000, 0x80000, CRC(69f02d84) SHA1(0fb4ff750de78505f241ae6cd18fccf3ddf4223f) )
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u5_sound.u5", 0x600000, 0x80000, CRC(9d0b9071) SHA1(05edf9073399a942a9d0b969274a7ebf4ca677da) )
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u6_sound.u6", 0x800000, 0x80000, CRC(df28f492) SHA1(c61f3870f59458b7bb5efbf93d697e3fa44a7830) )
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u7_sound.u7", 0xa00000, 0x80000, CRC(0128913e) SHA1(c11bc115877310c17f9b57f72b29d19b0ad71afa) )
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u8_sound.u8", 0xc00000, 0x80000, CRC(5127c08e) SHA1(4f0eae73817270fa156829100b66f0ff88fa422c) )
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u9_sound.u9", 0xe00000, 0x80000, CRC(84cdc781) SHA1(62287aa72903698d1890908adde53c39f8bd200c) )

	ROM_REGION32_LE( 0x1000000, "maindata", 0 )
	ROM_LOAD32_BYTE( "2.3_cruisn_world_u10_game.u10",  0x0000000, 0x100000, CRC(956e0642) SHA1(c023d41159bac9b468d6fc411005f66b15b9dff6) )
	ROM_LOAD32_BYTE( "2.3_cruisn_world_u11_game.u11",  0x0000001, 0x100000, CRC(b4ed2929) SHA1(22afc3c7bcc57b7b24b4156376df0b7fb8f0c9fb) )
	ROM_LOAD32_BYTE( "2.3_cruisn_world_u12_game.u12",  0x0000002, 0x100000, CRC(cd12528e) SHA1(685e2280448be2cd90a875cca9ef2ab3d2f8d3e1) )
	ROM_LOAD32_BYTE( "2.3_cruisn_world_u13_game.u13",  0x0000003, 0x100000, CRC(b096d211) SHA1(a2663b58e2f21bcfcba5317ff0ae91dd21a399f5) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u14_image.u14", 0x0400000, 0x100000, CRC(ee815091) SHA1(fb8a99bae07f42966f76a3bb073d7d8280d8efcb) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u15_image.u15", 0x0400001, 0x100000, CRC(e2da7bf1) SHA1(9d9a80055ee62476f47c95e30ec9a989d5d0e25b) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u16_image.u16", 0x0400002, 0x100000, CRC(05a7ad2f) SHA1(4bdfde671379ecefa3f8ceb6fc06e8df5d70fc22) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u17_image.u17", 0x0400003, 0x100000, CRC(d6278c0c) SHA1(3e152d755d69903718a84d4154e442a31026f3d8) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u18_image.u18", 0x0800000, 0x100000, CRC(e2dc2733) SHA1(c277643548c03d831a3b091f1a311accac9d106b) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u19_image.u19", 0x0800001, 0x100000, CRC(5223a070) SHA1(90ce48b2308fa9e7cb636c4732b20b8e177aa9b1) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u20_image.u20", 0x0800002, 0x100000, CRC(db535625) SHA1(599ccd6bcfb155eb68ac131de4af524510ab35b7) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u21_image.u21", 0x0800003, 0x100000, CRC(92a080e8) SHA1(e5e0faf820b5870a81f121b6ad4c37a9081724e4) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u22_image.u22", 0x0c00000, 0x100000, CRC(77c56318) SHA1(52344038942c83f3ce82f3169a345ceb86e43dcb) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u23_image.u23", 0x0c00001, 0x100000, CRC(6b920fc7) SHA1(993da81181f24075e1aead7c4b374f36dd86a9c3) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u24_image.u24", 0x0c00002, 0x100000, CRC(83485401) SHA1(58407818a82a7a3657530dcda7e373e678b58ab2) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u25_image.u25", 0x0c00003, 0x100000, CRC(0dad97a9) SHA1(cdb0c02da35243b118e37ff1519aa6ee1a79d06d) )
ROM_END


ROM_START( crusnwld20 ) // Version 2.0, Tue Mar 18 1997 - 12:32:57
	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  // sound data
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u2_sound.u2", 0x000000, 0x80000, CRC(7a233c89) SHA1(ecfad4bc48a69cd3399e3b3266c81574082e0169) )
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u3_sound.u3", 0x200000, 0x80000, CRC(be9a5ff0) SHA1(98d69dbfa6aa8462cdd46772e991ee418b79c653) )
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u4_sound.u4", 0x400000, 0x80000, CRC(69f02d84) SHA1(0fb4ff750de78505f241ae6cd18fccf3ddf4223f) )
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u5_sound.u5", 0x600000, 0x80000, CRC(9d0b9071) SHA1(05edf9073399a942a9d0b969274a7ebf4ca677da) )
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u6_sound.u6", 0x800000, 0x80000, CRC(df28f492) SHA1(c61f3870f59458b7bb5efbf93d697e3fa44a7830) )
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u7_sound.u7", 0xa00000, 0x80000, CRC(0128913e) SHA1(c11bc115877310c17f9b57f72b29d19b0ad71afa) )
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u8_sound.u8", 0xc00000, 0x80000, CRC(5127c08e) SHA1(4f0eae73817270fa156829100b66f0ff88fa422c) )
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u9_sound.u9", 0xe00000, 0x80000, CRC(84cdc781) SHA1(62287aa72903698d1890908adde53c39f8bd200c) )

	ROM_REGION32_LE( 0x1000000, "maindata", 0 )
	ROM_LOAD32_BYTE( "2.0_cruisn_world_u10_game.u10",  0x0000000, 0x100000, CRC(2a04da6d) SHA1(0aab4f3dc4853de11234245ac14baa14cb3867f3) )
	ROM_LOAD32_BYTE( "2.0_cruisn_world_u11_game.u11",  0x0000001, 0x100000, CRC(26a8ad51) SHA1(522ef3499ba83fa808d7cdae71759e056df353bf) )
	ROM_LOAD32_BYTE( "2.0_cruisn_world_u12_game.u12",  0x0000002, 0x100000, CRC(236caec0) SHA1(f53df733943a52f94878bb1b7d6c877722b3fd82) )
	ROM_LOAD32_BYTE( "2.0_cruisn_world_u13_game.u13",  0x0000003, 0x100000, CRC(7e056e53) SHA1(62b593e093d06a8c0cca56e34f567f795bfc41fc) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u14_image.u14", 0x0400000, 0x100000, CRC(ee815091) SHA1(fb8a99bae07f42966f76a3bb073d7d8280d8efcb) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u15_image.u15", 0x0400001, 0x100000, CRC(e2da7bf1) SHA1(9d9a80055ee62476f47c95e30ec9a989d5d0e25b) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u16_image.u16", 0x0400002, 0x100000, CRC(05a7ad2f) SHA1(4bdfde671379ecefa3f8ceb6fc06e8df5d70fc22) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u17_image.u17", 0x0400003, 0x100000, CRC(d6278c0c) SHA1(3e152d755d69903718a84d4154e442a31026f3d8) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u18_image.u18", 0x0800000, 0x100000, CRC(e2dc2733) SHA1(c277643548c03d831a3b091f1a311accac9d106b) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u19_image.u19", 0x0800001, 0x100000, CRC(5223a070) SHA1(90ce48b2308fa9e7cb636c4732b20b8e177aa9b1) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u20_image.u20", 0x0800002, 0x100000, CRC(db535625) SHA1(599ccd6bcfb155eb68ac131de4af524510ab35b7) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u21_image.u21", 0x0800003, 0x100000, CRC(92a080e8) SHA1(e5e0faf820b5870a81f121b6ad4c37a9081724e4) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u22_image.u22", 0x0c00000, 0x100000, CRC(77c56318) SHA1(52344038942c83f3ce82f3169a345ceb86e43dcb) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u23_image.u23", 0x0c00001, 0x100000, CRC(6b920fc7) SHA1(993da81181f24075e1aead7c4b374f36dd86a9c3) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u24_image.u24", 0x0c00002, 0x100000, CRC(83485401) SHA1(58407818a82a7a3657530dcda7e373e678b58ab2) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u25_image.u25", 0x0c00003, 0x100000, CRC(0dad97a9) SHA1(cdb0c02da35243b118e37ff1519aa6ee1a79d06d) )
ROM_END


ROM_START( crusnwld19 ) // Version 1.9, Sat Mar 08 1997 - 14:48:17
	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  // sound data
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u2_sound.u2", 0x000000, 0x80000, CRC(7a233c89) SHA1(ecfad4bc48a69cd3399e3b3266c81574082e0169) )
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u3_sound.u3", 0x200000, 0x80000, CRC(be9a5ff0) SHA1(98d69dbfa6aa8462cdd46772e991ee418b79c653) )
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u4_sound.u4", 0x400000, 0x80000, CRC(69f02d84) SHA1(0fb4ff750de78505f241ae6cd18fccf3ddf4223f) )
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u5_sound.u5", 0x600000, 0x80000, CRC(9d0b9071) SHA1(05edf9073399a942a9d0b969274a7ebf4ca677da) )
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u6_sound.u6", 0x800000, 0x80000, CRC(df28f492) SHA1(c61f3870f59458b7bb5efbf93d697e3fa44a7830) )
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u7_sound.u7", 0xa00000, 0x80000, CRC(0128913e) SHA1(c11bc115877310c17f9b57f72b29d19b0ad71afa) )
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u8_sound.u8", 0xc00000, 0x80000, CRC(5127c08e) SHA1(4f0eae73817270fa156829100b66f0ff88fa422c) )
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u9_sound.u9", 0xe00000, 0x80000, CRC(84cdc781) SHA1(62287aa72903698d1890908adde53c39f8bd200c) )

	ROM_REGION32_LE( 0x1000000, "maindata", 0 )
	ROM_LOAD32_BYTE( "1.9_cruisn_world_u10_game.u10",  0x0000000, 0x100000, CRC(c5cf5316) SHA1(f9900526314c1ea2903fd9a01fcdb839609b1858) )
	ROM_LOAD32_BYTE( "1.9_cruisn_world_u11_game.u11",  0x0000001, 0x100000, CRC(0b183a06) SHA1(06585662abebedc986b22c10fd4f489ed1d94e67) )
	ROM_LOAD32_BYTE( "1.9_cruisn_world_u12_game.u12",  0x0000002, 0x100000, CRC(e32d1a8d) SHA1(ba5bbcee4fe67194e5e0bd99898116350450e83f) )
	ROM_LOAD32_BYTE( "1.9_cruisn_world_u13_game.u13",  0x0000003, 0x100000, CRC(91fcae27) SHA1(a5c2942b01ed6c8a8f4187b4d08a5f1868cf3e2e) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u14_image.u14", 0x0400000, 0x100000, CRC(ee815091) SHA1(fb8a99bae07f42966f76a3bb073d7d8280d8efcb) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u15_image.u15", 0x0400001, 0x100000, CRC(e2da7bf1) SHA1(9d9a80055ee62476f47c95e30ec9a989d5d0e25b) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u16_image.u16", 0x0400002, 0x100000, CRC(05a7ad2f) SHA1(4bdfde671379ecefa3f8ceb6fc06e8df5d70fc22) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u17_image.u17", 0x0400003, 0x100000, CRC(d6278c0c) SHA1(3e152d755d69903718a84d4154e442a31026f3d8) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u18_image.u18", 0x0800000, 0x100000, CRC(e2dc2733) SHA1(c277643548c03d831a3b091f1a311accac9d106b) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u19_image.u19", 0x0800001, 0x100000, CRC(5223a070) SHA1(90ce48b2308fa9e7cb636c4732b20b8e177aa9b1) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u20_image.u20", 0x0800002, 0x100000, CRC(db535625) SHA1(599ccd6bcfb155eb68ac131de4af524510ab35b7) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u21_image.u21", 0x0800003, 0x100000, CRC(92a080e8) SHA1(e5e0faf820b5870a81f121b6ad4c37a9081724e4) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u22_image.u22", 0x0c00000, 0x100000, CRC(77c56318) SHA1(52344038942c83f3ce82f3169a345ceb86e43dcb) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u23_image.u23", 0x0c00001, 0x100000, CRC(6b920fc7) SHA1(993da81181f24075e1aead7c4b374f36dd86a9c3) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u24_image.u24", 0x0c00002, 0x100000, CRC(83485401) SHA1(58407818a82a7a3657530dcda7e373e678b58ab2) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u25_image.u25", 0x0c00003, 0x100000, CRC(0dad97a9) SHA1(cdb0c02da35243b118e37ff1519aa6ee1a79d06d) )
ROM_END


ROM_START( crusnwld17 ) // Version 1.7, Fri Jan 24 1997 - 16:23:59
	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  // sound data
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u2_sound.u2", 0x000000, 0x80000, CRC(7a233c89) SHA1(ecfad4bc48a69cd3399e3b3266c81574082e0169) )
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u3_sound.u3", 0x200000, 0x80000, CRC(be9a5ff0) SHA1(98d69dbfa6aa8462cdd46772e991ee418b79c653) )
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u4_sound.u4", 0x400000, 0x80000, CRC(69f02d84) SHA1(0fb4ff750de78505f241ae6cd18fccf3ddf4223f) )
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u5_sound.u5", 0x600000, 0x80000, CRC(9d0b9071) SHA1(05edf9073399a942a9d0b969274a7ebf4ca677da) )
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u6_sound.u6", 0x800000, 0x80000, CRC(df28f492) SHA1(c61f3870f59458b7bb5efbf93d697e3fa44a7830) )
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u7_sound.u7", 0xa00000, 0x80000, CRC(0128913e) SHA1(c11bc115877310c17f9b57f72b29d19b0ad71afa) )
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u8_sound.u8", 0xc00000, 0x80000, CRC(5127c08e) SHA1(4f0eae73817270fa156829100b66f0ff88fa422c) )
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u9_sound.u9", 0xe00000, 0x80000, CRC(84cdc781) SHA1(62287aa72903698d1890908adde53c39f8bd200c) )

	ROM_REGION32_LE( 0x1000000, "maindata", 0 )
	ROM_LOAD32_BYTE( "1.7_cruisn_world_u10_game.u10",  0x0000000, 0x100000, CRC(afca0f15) SHA1(52ed51e31ba7f8ac1a71a7bdb64733b6e95b0669) )
	ROM_LOAD32_BYTE( "1.7_cruisn_world_u11_game.u11",  0x0000001, 0x100000, CRC(6610af52) SHA1(c6ab7f369bd0b05e0ce28c7829b870f5b6ddf12f) )
	ROM_LOAD32_BYTE( "1.7_cruisn_world_u12_game.u12",  0x0000002, 0x100000, CRC(ef0107b1) SHA1(350017ab56c220516dda53c8323eaf82d7dee8dd) )
	ROM_LOAD32_BYTE( "1.7_cruisn_world_u13_game.u13",  0x0000003, 0x100000, CRC(c1d68aa0) SHA1(07d5ec75d921935474a9de738b1b7e9cb0748483) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u14_image.u14", 0x0400000, 0x100000, CRC(ee815091) SHA1(fb8a99bae07f42966f76a3bb073d7d8280d8efcb) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u15_image.u15", 0x0400001, 0x100000, CRC(e2da7bf1) SHA1(9d9a80055ee62476f47c95e30ec9a989d5d0e25b) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u16_image.u16", 0x0400002, 0x100000, CRC(05a7ad2f) SHA1(4bdfde671379ecefa3f8ceb6fc06e8df5d70fc22) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u17_image.u17", 0x0400003, 0x100000, CRC(d6278c0c) SHA1(3e152d755d69903718a84d4154e442a31026f3d8) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u18_image.u18", 0x0800000, 0x100000, CRC(e2dc2733) SHA1(c277643548c03d831a3b091f1a311accac9d106b) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u19_image.u19", 0x0800001, 0x100000, CRC(5223a070) SHA1(90ce48b2308fa9e7cb636c4732b20b8e177aa9b1) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u20_image.u20", 0x0800002, 0x100000, CRC(db535625) SHA1(599ccd6bcfb155eb68ac131de4af524510ab35b7) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u21_image.u21", 0x0800003, 0x100000, CRC(92a080e8) SHA1(e5e0faf820b5870a81f121b6ad4c37a9081724e4) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u22_image.u22", 0x0c00000, 0x100000, CRC(77c56318) SHA1(52344038942c83f3ce82f3169a345ceb86e43dcb) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u23_image.u23", 0x0c00001, 0x100000, CRC(6b920fc7) SHA1(993da81181f24075e1aead7c4b374f36dd86a9c3) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u24_image.u24", 0x0c00002, 0x100000, CRC(83485401) SHA1(58407818a82a7a3657530dcda7e373e678b58ab2) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u25_image.u25", 0x0c00003, 0x100000, CRC(0dad97a9) SHA1(cdb0c02da35243b118e37ff1519aa6ee1a79d06d) )
ROM_END


ROM_START( crusnwld13 ) // Version 1.3, Mon Nov 25 1996 - 23:22:45
	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  // sound data
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u2_sound.u2", 0x000000, 0x80000, CRC(7a233c89) SHA1(ecfad4bc48a69cd3399e3b3266c81574082e0169) )
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u3_sound.u3", 0x200000, 0x80000, CRC(be9a5ff0) SHA1(98d69dbfa6aa8462cdd46772e991ee418b79c653) )
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u4_sound.u4", 0x400000, 0x80000, CRC(69f02d84) SHA1(0fb4ff750de78505f241ae6cd18fccf3ddf4223f) )
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u5_sound.u5", 0x600000, 0x80000, CRC(9d0b9071) SHA1(05edf9073399a942a9d0b969274a7ebf4ca677da) )
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u6_sound.u6", 0x800000, 0x80000, CRC(df28f492) SHA1(c61f3870f59458b7bb5efbf93d697e3fa44a7830) )
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u7_sound.u7", 0xa00000, 0x80000, CRC(0128913e) SHA1(c11bc115877310c17f9b57f72b29d19b0ad71afa) )
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u8_sound.u8", 0xc00000, 0x80000, CRC(5127c08e) SHA1(4f0eae73817270fa156829100b66f0ff88fa422c) )
	ROM_LOAD16_BYTE( "1.0_cruisn_world_u9_sound.u9", 0xe00000, 0x80000, CRC(84cdc781) SHA1(62287aa72903698d1890908adde53c39f8bd200c) )

	ROM_REGION32_LE( 0x1000000, "maindata", 0 )
	ROM_LOAD32_BYTE( "1.3_cruisn_world_u10_game.u10",  0x0000000, 0x100000, CRC(d361d17d) SHA1(7f42baec5492c4040e030e6233e500eb54bd9cba) )
	ROM_LOAD32_BYTE( "1.3_cruisn_world_u11_game.u11",  0x0000001, 0x100000, CRC(b0c0a462) SHA1(22ae081c3eb9f298aea73e99a0124becd540f0df) )
	ROM_LOAD32_BYTE( "1.3_cruisn_world_u12_game.u12",  0x0000002, 0x100000, CRC(5e7c566b) SHA1(81e6f21309bd3ba8589bc591a9ba5729f301539e) )
	ROM_LOAD32_BYTE( "1.3_cruisn_world_u13_game.u13",  0x0000003, 0x100000, CRC(46886e9c) SHA1(a2400f42fef9838fd8a347e8a249ba977d9fbcfe) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u14_image.u14", 0x0400000, 0x100000, CRC(ee815091) SHA1(fb8a99bae07f42966f76a3bb073d7d8280d8efcb) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u15_image.u15", 0x0400001, 0x100000, CRC(e2da7bf1) SHA1(9d9a80055ee62476f47c95e30ec9a989d5d0e25b) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u16_image.u16", 0x0400002, 0x100000, CRC(05a7ad2f) SHA1(4bdfde671379ecefa3f8ceb6fc06e8df5d70fc22) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u17_image.u17", 0x0400003, 0x100000, CRC(d6278c0c) SHA1(3e152d755d69903718a84d4154e442a31026f3d8) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u18_image.u18", 0x0800000, 0x100000, CRC(e2dc2733) SHA1(c277643548c03d831a3b091f1a311accac9d106b) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u19_image.u19", 0x0800001, 0x100000, CRC(5223a070) SHA1(90ce48b2308fa9e7cb636c4732b20b8e177aa9b1) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u20_image.u20", 0x0800002, 0x100000, CRC(db535625) SHA1(599ccd6bcfb155eb68ac131de4af524510ab35b7) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u21_image.u21", 0x0800003, 0x100000, CRC(92a080e8) SHA1(e5e0faf820b5870a81f121b6ad4c37a9081724e4) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u22_image.u22", 0x0c00000, 0x100000, CRC(77c56318) SHA1(52344038942c83f3ce82f3169a345ceb86e43dcb) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u23_image.u23", 0x0c00001, 0x100000, CRC(6b920fc7) SHA1(993da81181f24075e1aead7c4b374f36dd86a9c3) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u24_image.u24", 0x0c00002, 0x100000, CRC(83485401) SHA1(58407818a82a7a3657530dcda7e373e678b58ab2) )
	ROM_LOAD32_BYTE( "1.1_cruisn_world_u25_image.u25", 0x0c00003, 0x100000, CRC(0dad97a9) SHA1(cdb0c02da35243b118e37ff1519aa6ee1a79d06d) )
ROM_END


/*
Some Off Road Challenge PCBs were all EPROMs, most seemed to have mask ROMs for the data ROMs

Mask ROMs have been seen in two formats:
--------------------------------   --------------------------------
|    MIDWAY GAMES INC          |   |    MIDWAY GAMES INC          |
|    OFFROAD CHALLENGE         |   |    OFFROAD CHALLENGE         |
)    5341-15511-01             |   )    5341-15510-01             |
|    U3 SOUND                  |   |    U14 VIDEO IMAGE           |
|   (C)1997 MIDWAY GAMES       |   |   (C)1997 MIDWAY GAMES       |
--------------------------------   --------------------------------
                                or
--------------------------------   --------------------------------
|                              |   |                              |
|    OFF ROAD                  |   |    OFF ROAD                  |
)    U3 SOUND                  |   )    U14 IMAGE                 |
|    1997 MIDWAY               |   |    1997 MIDWAY               |
|                              |   |                              |
--------------------------------   --------------------------------

*/
ROM_START( offroadc ) // Version 1.63, Tue 03-03-98
	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  // sound data
	ROM_LOAD16_BYTE( "1.0_off_road_u2_sound.u2", 0x000000, 0x80000, CRC(69976e9d) SHA1(63c886ac2563c43a10840f49f929f8613cd94de2) ) // generally a M27C4001 EPROM with label
	ROM_LOAD16_BYTE( "offroad_challenge_5341-15511-01_u3_sound.u3", 0x200000, 0x80000, CRC(2db9b548) SHA1(4f454a3e6a8851b0ef5d325dd28102d57ea11a11) ) // these are mask ROMs
	ROM_LOAD16_BYTE( "offroad_challenge_5341-15511-02_u4_sound.u4", 0x400000, 0x80000, CRC(42bdf9d0) SHA1(04add0f0ee7fa61de1913cc0b988345d3d430cde) )
	ROM_LOAD16_BYTE( "offroad_challenge_5341-15511-03_u5_sound.u5", 0x600000, 0x80000, CRC(569cc84b) SHA1(08b917cc41fae6b6a3e9d9461a783d3d2865e72a) )
	ROM_LOAD16_BYTE( "offroad_challenge_5341-15511-04_u6_sound.u6", 0x800000, 0x80000, CRC(0896f679) SHA1(dde39ef17834256909ef2c9fcd5b5fb9939d5178) )
	ROM_LOAD16_BYTE( "offroad_challenge_5341-15511-05_u7_sound.u7", 0xa00000, 0x80000, CRC(fe242d6a) SHA1(8fbac22ed23044841f309ce58c5b1affcdd5d114) )
	ROM_LOAD16_BYTE( "offroad_challenge_5341-15511-06_u8_sound.u8", 0xc00000, 0x80000, CRC(5da13f12) SHA1(2bb5e929e8bc6c70cb4475024a6b0bb07ac25244) )
	ROM_LOAD16_BYTE( "offroad_challenge_5341-15511-07_u9_sound.u9", 0xe00000, 0x80000, CRC(7ad27f69) SHA1(b33665d0593a95b58d529720aae49e90449bf714) )

	ROM_REGION32_LE( 0x1000000, "maindata", 0 )
	ROM_LOAD32_BYTE( "1.63_off_road_u10_game.u10", 0x0000000, 0x100000, CRC(faaf81b8) SHA1(d0bd40b2cf5d07db9f668826cc7f0ed84c4e84bf) ) // Version 1.63 program ROMs
	ROM_LOAD32_BYTE( "1.63_off_road_u11_game.u11", 0x0000001, 0x100000, CRC(f68e9655) SHA1(e29926ea24cfbd228a2136d04a63a92eba0098d7) )
	ROM_LOAD32_BYTE( "1.63_off_road_u12_game.u12", 0x0000002, 0x100000, CRC(6a5295b3) SHA1(ac72fe205ffb306598400e8b1d9c98ae67b0bab9) )
	ROM_LOAD32_BYTE( "1.63_off_road_u13_game.u13", 0x0000003, 0x100000, CRC(cb9233b5) SHA1(2d23b6a2312a75dbaa44de3224512c844aaac7b5) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-01_u14_video_image.u14", 0x0400000, 0x100000, CRC(1e41d14b) SHA1(3f7c5fae1f8b82ddd811720837fa298785a8dd27) ) // mask ROMs
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-02_u15_video_image.u15", 0x0400001, 0x100000, CRC(654d623d) SHA1(a944b8f8d71b099d7b5bbd7df6effb90afc3aec8) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-03_u16_video_image.u16", 0x0400002, 0x100000, CRC(259774d8) SHA1(90cdf659324b84b3c2c59497cc5611e8f12629a6) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-04_u17_video_image.u17", 0x0400003, 0x100000, CRC(50c61434) SHA1(52bc603101b4f88b7d892af683b7c8358cabbf4a) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-05_u18_video_image.u18", 0x0800000, 0x100000, CRC(015be91c) SHA1(1624537068c6bc5fa6235bf0b0343347c337e8d8) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-06_u19_video_image.u19", 0x0800001, 0x100000, CRC(cfc6b70e) SHA1(8c5ad84c50ca142726db0595153cf04caaabec9c) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-07_u20_video_image.u20", 0x0800002, 0x100000, CRC(f48d6e33) SHA1(8b9c205e24f217ac110cdd82388c056ebbbb09b0) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-08_u21_video_image.u21", 0x0800003, 0x100000, CRC(17794b56) SHA1(8bfd8f5b43056bfe7f62524bb8c3a8564a3a9413) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-09_u22_video_image.u22", 0x0c00000, 0x100000, CRC(f2a6e622) SHA1(a7d7004e95b058124cc02e8073dab8fbed8813c5) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-10_u23_video_image.u23", 0x0c00001, 0x100000, CRC(1cba6e20) SHA1(a7c9c58bfc4d26decb08979d83cccedb27528eb6) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-11_u24_video_image.u24", 0x0c00002, 0x100000, CRC(fd3ce11f) SHA1(78c65267712488784bc6dc14eef98a90494a9553) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-12_u25_video_image.u25", 0x0c00003, 0x100000, CRC(78f8e5db) SHA1(7ec2a5add27d66c43ba5cb7182554321007f5798) )
ROM_END


ROM_START( offroadc5 ) // Version 1.50, Tue 10-21-97
	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  // sound data
	ROM_LOAD16_BYTE( "1.0_off_road_u2_sound.u2", 0x000000, 0x80000, CRC(69976e9d) SHA1(63c886ac2563c43a10840f49f929f8613cd94de2) ) // generally a M27C4001 EPROM with label
	ROM_LOAD16_BYTE( "offroad_challenge_5341-15511-01_u3_sound.u3", 0x200000, 0x80000, CRC(2db9b548) SHA1(4f454a3e6a8851b0ef5d325dd28102d57ea11a11) ) // these are mask ROMs
	ROM_LOAD16_BYTE( "offroad_challenge_5341-15511-02_u4_sound.u4", 0x400000, 0x80000, CRC(42bdf9d0) SHA1(04add0f0ee7fa61de1913cc0b988345d3d430cde) )
	ROM_LOAD16_BYTE( "offroad_challenge_5341-15511-03_u5_sound.u5", 0x600000, 0x80000, CRC(569cc84b) SHA1(08b917cc41fae6b6a3e9d9461a783d3d2865e72a) )
	ROM_LOAD16_BYTE( "offroad_challenge_5341-15511-04_u6_sound.u6", 0x800000, 0x80000, CRC(0896f679) SHA1(dde39ef17834256909ef2c9fcd5b5fb9939d5178) )
	ROM_LOAD16_BYTE( "offroad_challenge_5341-15511-05_u7_sound.u7", 0xa00000, 0x80000, CRC(fe242d6a) SHA1(8fbac22ed23044841f309ce58c5b1affcdd5d114) )
	ROM_LOAD16_BYTE( "offroad_challenge_5341-15511-06_u8_sound.u8", 0xc00000, 0x80000, CRC(5da13f12) SHA1(2bb5e929e8bc6c70cb4475024a6b0bb07ac25244) )
	ROM_LOAD16_BYTE( "offroad_challenge_5341-15511-07_u9_sound.u9", 0xe00000, 0x80000, CRC(7ad27f69) SHA1(b33665d0593a95b58d529720aae49e90449bf714) )

	ROM_REGION32_LE( 0x1000000, "maindata", 0 )
	ROM_LOAD32_BYTE( "1.5_off_road_u10_game.u10",  0x0000000, 0x100000, CRC(f464be4f) SHA1(da6c04ae49d033f92cdd62f997841365c4a08616) ) // Version 1.50 program ROMs
	ROM_LOAD32_BYTE( "1.5_off_road_u11_game.u11",  0x0000001, 0x100000, CRC(eaddc9ac) SHA1(a6b810bf7460e3257bf6acdc3b79c532fb71ad68) )
	ROM_LOAD32_BYTE( "1.5_off_road_u12_game.u12",  0x0000002, 0x100000, CRC(a2da68da) SHA1(b8dcc042b9926055bff9020599c1c218f08b1727) )
	ROM_LOAD32_BYTE( "1.5_off_road_u13_game.u13",  0x0000003, 0x100000, CRC(b4755ee2) SHA1(1c4cde7ca60a6e8bff12aed348e7148e20a8caba) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-01_u14_video_image.u14", 0x0400000, 0x100000, CRC(1e41d14b) SHA1(3f7c5fae1f8b82ddd811720837fa298785a8dd27) ) // mask ROMs
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-02_u15_video_image.u15", 0x0400001, 0x100000, CRC(654d623d) SHA1(a944b8f8d71b099d7b5bbd7df6effb90afc3aec8) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-03_u16_video_image.u16", 0x0400002, 0x100000, CRC(259774d8) SHA1(90cdf659324b84b3c2c59497cc5611e8f12629a6) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-04_u17_video_image.u17", 0x0400003, 0x100000, CRC(50c61434) SHA1(52bc603101b4f88b7d892af683b7c8358cabbf4a) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-05_u18_video_image.u18", 0x0800000, 0x100000, CRC(015be91c) SHA1(1624537068c6bc5fa6235bf0b0343347c337e8d8) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-06_u19_video_image.u19", 0x0800001, 0x100000, CRC(cfc6b70e) SHA1(8c5ad84c50ca142726db0595153cf04caaabec9c) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-07_u20_video_image.u20", 0x0800002, 0x100000, CRC(f48d6e33) SHA1(8b9c205e24f217ac110cdd82388c056ebbbb09b0) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-08_u21_video_image.u21", 0x0800003, 0x100000, CRC(17794b56) SHA1(8bfd8f5b43056bfe7f62524bb8c3a8564a3a9413) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-09_u22_video_image.u22", 0x0c00000, 0x100000, CRC(f2a6e622) SHA1(a7d7004e95b058124cc02e8073dab8fbed8813c5) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-10_u23_video_image.u23", 0x0c00001, 0x100000, CRC(1cba6e20) SHA1(a7c9c58bfc4d26decb08979d83cccedb27528eb6) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-11_u24_video_image.u24", 0x0c00002, 0x100000, CRC(fd3ce11f) SHA1(78c65267712488784bc6dc14eef98a90494a9553) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-12_u25_video_image.u25", 0x0c00003, 0x100000, CRC(78f8e5db) SHA1(7ec2a5add27d66c43ba5cb7182554321007f5798) )
ROM_END


ROM_START( offroadc4 ) // Version 1.40, Mon 10-06-97
	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  // sound data
	ROM_LOAD16_BYTE( "1.0_off_road_u2_sound.u2", 0x000000, 0x80000, CRC(69976e9d) SHA1(63c886ac2563c43a10840f49f929f8613cd94de2) ) // generally a M27C4001 EPROM with label
	ROM_LOAD16_BYTE( "offroad_challenge_5341-15511-01_u3_sound.u3", 0x200000, 0x80000, CRC(2db9b548) SHA1(4f454a3e6a8851b0ef5d325dd28102d57ea11a11) ) // these are mask ROMs
	ROM_LOAD16_BYTE( "offroad_challenge_5341-15511-02_u4_sound.u4", 0x400000, 0x80000, CRC(42bdf9d0) SHA1(04add0f0ee7fa61de1913cc0b988345d3d430cde) )
	ROM_LOAD16_BYTE( "offroad_challenge_5341-15511-03_u5_sound.u5", 0x600000, 0x80000, CRC(569cc84b) SHA1(08b917cc41fae6b6a3e9d9461a783d3d2865e72a) )
	ROM_LOAD16_BYTE( "offroad_challenge_5341-15511-04_u6_sound.u6", 0x800000, 0x80000, CRC(0896f679) SHA1(dde39ef17834256909ef2c9fcd5b5fb9939d5178) )
	ROM_LOAD16_BYTE( "offroad_challenge_5341-15511-05_u7_sound.u7", 0xa00000, 0x80000, CRC(fe242d6a) SHA1(8fbac22ed23044841f309ce58c5b1affcdd5d114) )
	ROM_LOAD16_BYTE( "offroad_challenge_5341-15511-06_u8_sound.u8", 0xc00000, 0x80000, CRC(5da13f12) SHA1(2bb5e929e8bc6c70cb4475024a6b0bb07ac25244) )
	ROM_LOAD16_BYTE( "offroad_challenge_5341-15511-07_u9_sound.u9", 0xe00000, 0x80000, CRC(7ad27f69) SHA1(b33665d0593a95b58d529720aae49e90449bf714) )

	ROM_REGION32_LE( 0x1000000, "maindata", 0 )
	ROM_LOAD32_BYTE( "1.4_off_road_u10_game.u10",  0x0000000, 0x100000, CRC(d263b078) SHA1(d376e120e05cf8526b002300db345fd0b9775702) ) // Version 1.40 program ROMs
	ROM_LOAD32_BYTE( "1.4_off_road_u11_game.u11",  0x0000001, 0x100000, CRC(1b443a72) SHA1(0e16d923f0e97f21e92c8d5b431fcaa0815b2c87) )
	ROM_LOAD32_BYTE( "1.4_off_road_u12_game.u12",  0x0000002, 0x100000, CRC(4e82a34b) SHA1(c22a3f638b7e226add511147982339b1f59821e9) )
	ROM_LOAD32_BYTE( "1.4_off_road_u13_game.u13",  0x0000003, 0x100000, CRC(558b859c) SHA1(b7946a4b44976b08a691622000e1457021267d1a) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-01_u14_video_image.u14", 0x0400000, 0x100000, CRC(1e41d14b) SHA1(3f7c5fae1f8b82ddd811720837fa298785a8dd27) ) // mask ROMs
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-02_u15_video_image.u15", 0x0400001, 0x100000, CRC(654d623d) SHA1(a944b8f8d71b099d7b5bbd7df6effb90afc3aec8) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-03_u16_video_image.u16", 0x0400002, 0x100000, CRC(259774d8) SHA1(90cdf659324b84b3c2c59497cc5611e8f12629a6) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-04_u17_video_image.u17", 0x0400003, 0x100000, CRC(50c61434) SHA1(52bc603101b4f88b7d892af683b7c8358cabbf4a) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-05_u18_video_image.u18", 0x0800000, 0x100000, CRC(015be91c) SHA1(1624537068c6bc5fa6235bf0b0343347c337e8d8) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-06_u19_video_image.u19", 0x0800001, 0x100000, CRC(cfc6b70e) SHA1(8c5ad84c50ca142726db0595153cf04caaabec9c) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-07_u20_video_image.u20", 0x0800002, 0x100000, CRC(f48d6e33) SHA1(8b9c205e24f217ac110cdd82388c056ebbbb09b0) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-08_u21_video_image.u21", 0x0800003, 0x100000, CRC(17794b56) SHA1(8bfd8f5b43056bfe7f62524bb8c3a8564a3a9413) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-09_u22_video_image.u22", 0x0c00000, 0x100000, CRC(f2a6e622) SHA1(a7d7004e95b058124cc02e8073dab8fbed8813c5) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-10_u23_video_image.u23", 0x0c00001, 0x100000, CRC(1cba6e20) SHA1(a7c9c58bfc4d26decb08979d83cccedb27528eb6) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-11_u24_video_image.u24", 0x0c00002, 0x100000, CRC(fd3ce11f) SHA1(78c65267712488784bc6dc14eef98a90494a9553) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-12_u25_video_image.u25", 0x0c00003, 0x100000, CRC(78f8e5db) SHA1(7ec2a5add27d66c43ba5cb7182554321007f5798) )
ROM_END


ROM_START( offroadc3 ) // Version 1.30, Mon 09-15-97
	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  // sound data
	ROM_LOAD16_BYTE( "1.0_off_road_u2_sound.u2", 0x000000, 0x80000, CRC(69976e9d) SHA1(63c886ac2563c43a10840f49f929f8613cd94de2) ) // generally a M27C4001 EPROM with label
	ROM_LOAD16_BYTE( "offroad_challenge_5341-15511-01_u3_sound.u3", 0x200000, 0x80000, CRC(2db9b548) SHA1(4f454a3e6a8851b0ef5d325dd28102d57ea11a11) ) // these are mask ROMs
	ROM_LOAD16_BYTE( "offroad_challenge_5341-15511-02_u4_sound.u4", 0x400000, 0x80000, CRC(42bdf9d0) SHA1(04add0f0ee7fa61de1913cc0b988345d3d430cde) )
	ROM_LOAD16_BYTE( "offroad_challenge_5341-15511-03_u5_sound.u5", 0x600000, 0x80000, CRC(569cc84b) SHA1(08b917cc41fae6b6a3e9d9461a783d3d2865e72a) )
	ROM_LOAD16_BYTE( "offroad_challenge_5341-15511-04_u6_sound.u6", 0x800000, 0x80000, CRC(0896f679) SHA1(dde39ef17834256909ef2c9fcd5b5fb9939d5178) )
	ROM_LOAD16_BYTE( "offroad_challenge_5341-15511-05_u7_sound.u7", 0xa00000, 0x80000, CRC(fe242d6a) SHA1(8fbac22ed23044841f309ce58c5b1affcdd5d114) )
	ROM_LOAD16_BYTE( "offroad_challenge_5341-15511-06_u8_sound.u8", 0xc00000, 0x80000, CRC(5da13f12) SHA1(2bb5e929e8bc6c70cb4475024a6b0bb07ac25244) )
	ROM_LOAD16_BYTE( "offroad_challenge_5341-15511-07_u9_sound.u9", 0xe00000, 0x80000, CRC(7ad27f69) SHA1(b33665d0593a95b58d529720aae49e90449bf714) )

	ROM_REGION32_LE( 0x1000000, "maindata", 0 )
	ROM_LOAD32_BYTE( "1.3_off_road_u10.u10",  0x0000000, 0x100000, CRC(71c62ce2) SHA1(e6bdbf3df4795f4cf29a08641cc59d90aed73b57) ) // Version 1.30 program ROMs
	ROM_LOAD32_BYTE( "1.3_off_road_u11.u11",  0x0000001, 0x100000, CRC(9e362dbb) SHA1(2480710f1081679ff87239a8e28a9a3f235bd3dc) )
	ROM_LOAD32_BYTE( "1.3_off_road_u12.u12",  0x0000002, 0x100000, CRC(9e0a5b06) SHA1(63bbe427713fc966c65dab575dd42cdce6b00874) )
	ROM_LOAD32_BYTE( "1.3_off_road_u13.u13",  0x0000003, 0x100000, CRC(d602db7e) SHA1(48bc762a83baeb382476619f54631ccbe12d1b2c) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-01_u14_video_image.u14", 0x0400000, 0x100000, CRC(1e41d14b) SHA1(3f7c5fae1f8b82ddd811720837fa298785a8dd27) ) // mask ROMs
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-02_u15_video_image.u15", 0x0400001, 0x100000, CRC(654d623d) SHA1(a944b8f8d71b099d7b5bbd7df6effb90afc3aec8) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-03_u16_video_image.u16", 0x0400002, 0x100000, CRC(259774d8) SHA1(90cdf659324b84b3c2c59497cc5611e8f12629a6) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-04_u17_video_image.u17", 0x0400003, 0x100000, CRC(50c61434) SHA1(52bc603101b4f88b7d892af683b7c8358cabbf4a) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-05_u18_video_image.u18", 0x0800000, 0x100000, CRC(015be91c) SHA1(1624537068c6bc5fa6235bf0b0343347c337e8d8) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-06_u19_video_image.u19", 0x0800001, 0x100000, CRC(cfc6b70e) SHA1(8c5ad84c50ca142726db0595153cf04caaabec9c) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-07_u20_video_image.u20", 0x0800002, 0x100000, CRC(f48d6e33) SHA1(8b9c205e24f217ac110cdd82388c056ebbbb09b0) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-08_u21_video_image.u21", 0x0800003, 0x100000, CRC(17794b56) SHA1(8bfd8f5b43056bfe7f62524bb8c3a8564a3a9413) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-09_u22_video_image.u22", 0x0c00000, 0x100000, CRC(f2a6e622) SHA1(a7d7004e95b058124cc02e8073dab8fbed8813c5) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-10_u23_video_image.u23", 0x0c00001, 0x100000, CRC(1cba6e20) SHA1(a7c9c58bfc4d26decb08979d83cccedb27528eb6) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-11_u24_video_image.u24", 0x0c00002, 0x100000, CRC(fd3ce11f) SHA1(78c65267712488784bc6dc14eef98a90494a9553) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-12_u25_video_image.u25", 0x0c00003, 0x100000, CRC(78f8e5db) SHA1(7ec2a5add27d66c43ba5cb7182554321007f5798) )
ROM_END


ROM_START( offroadc1 ) // Version 1.10, Mon 08-18-97
	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  // sound data
	ROM_LOAD16_BYTE( "1.0_off_road_u2_sound.u2", 0x000000, 0x80000, CRC(69976e9d) SHA1(63c886ac2563c43a10840f49f929f8613cd94de2) ) // generally a M27C4001 EPROM with label
	ROM_LOAD16_BYTE( "offroad_challenge_5341-15511-01_u3_sound.u3", 0x200000, 0x80000, CRC(2db9b548) SHA1(4f454a3e6a8851b0ef5d325dd28102d57ea11a11) ) // these are mask ROMs
	ROM_LOAD16_BYTE( "offroad_challenge_5341-15511-02_u4_sound.u4", 0x400000, 0x80000, CRC(42bdf9d0) SHA1(04add0f0ee7fa61de1913cc0b988345d3d430cde) )
	ROM_LOAD16_BYTE( "offroad_challenge_5341-15511-03_u5_sound.u5", 0x600000, 0x80000, CRC(569cc84b) SHA1(08b917cc41fae6b6a3e9d9461a783d3d2865e72a) )
	ROM_LOAD16_BYTE( "offroad_challenge_5341-15511-04_u6_sound.u6", 0x800000, 0x80000, CRC(0896f679) SHA1(dde39ef17834256909ef2c9fcd5b5fb9939d5178) )
	ROM_LOAD16_BYTE( "offroad_challenge_5341-15511-05_u7_sound.u7", 0xa00000, 0x80000, CRC(fe242d6a) SHA1(8fbac22ed23044841f309ce58c5b1affcdd5d114) )
	ROM_LOAD16_BYTE( "offroad_challenge_5341-15511-06_u8_sound.u8", 0xc00000, 0x80000, CRC(5da13f12) SHA1(2bb5e929e8bc6c70cb4475024a6b0bb07ac25244) )
	ROM_LOAD16_BYTE( "offroad_challenge_5341-15511-07_u9_sound.u9", 0xe00000, 0x80000, CRC(7ad27f69) SHA1(b33665d0593a95b58d529720aae49e90449bf714) )

	ROM_REGION32_LE( 0x1000000, "maindata", 0 )
	ROM_LOAD32_BYTE( "1.1_off_road_u10.u10",  0x0000000, 0x100000, CRC(4729660c) SHA1(0baff6a27015f4eb3fe0a981ecbac33d140e872a) ) // Version 1.10 program ROMs
	ROM_LOAD32_BYTE( "1.1_off_road_u11.u11",  0x0000001, 0x100000, CRC(6272d013) SHA1(860121184282627ed692e56a0dafee8b64562811) )
	ROM_LOAD32_BYTE( "1.1_off_road_u12.u12",  0x0000002, 0x100000, CRC(9c8326be) SHA1(55f16d14379f57d08ed84d82f9db1a582bc223a1) )
	ROM_LOAD32_BYTE( "1.1_off_road_u13.u13",  0x0000003, 0x100000, CRC(53bbc181) SHA1(1ab29a27a216eb09d69a9f3d681865de1a904717) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-01_u14_video_image.u14", 0x0400000, 0x100000, CRC(1e41d14b) SHA1(3f7c5fae1f8b82ddd811720837fa298785a8dd27) ) // mask ROMs
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-02_u15_video_image.u15", 0x0400001, 0x100000, CRC(654d623d) SHA1(a944b8f8d71b099d7b5bbd7df6effb90afc3aec8) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-03_u16_video_image.u16", 0x0400002, 0x100000, CRC(259774d8) SHA1(90cdf659324b84b3c2c59497cc5611e8f12629a6) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-04_u17_video_image.u17", 0x0400003, 0x100000, CRC(50c61434) SHA1(52bc603101b4f88b7d892af683b7c8358cabbf4a) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-05_u18_video_image.u18", 0x0800000, 0x100000, CRC(015be91c) SHA1(1624537068c6bc5fa6235bf0b0343347c337e8d8) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-06_u19_video_image.u19", 0x0800001, 0x100000, CRC(cfc6b70e) SHA1(8c5ad84c50ca142726db0595153cf04caaabec9c) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-07_u20_video_image.u20", 0x0800002, 0x100000, CRC(f48d6e33) SHA1(8b9c205e24f217ac110cdd82388c056ebbbb09b0) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-08_u21_video_image.u21", 0x0800003, 0x100000, CRC(17794b56) SHA1(8bfd8f5b43056bfe7f62524bb8c3a8564a3a9413) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-09_u22_video_image.u22", 0x0c00000, 0x100000, CRC(f2a6e622) SHA1(a7d7004e95b058124cc02e8073dab8fbed8813c5) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-10_u23_video_image.u23", 0x0c00001, 0x100000, CRC(1cba6e20) SHA1(a7c9c58bfc4d26decb08979d83cccedb27528eb6) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-11_u24_video_image.u24", 0x0c00002, 0x100000, CRC(fd3ce11f) SHA1(78c65267712488784bc6dc14eef98a90494a9553) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-12_u25_video_image.u25", 0x0c00003, 0x100000, CRC(78f8e5db) SHA1(7ec2a5add27d66c43ba5cb7182554321007f5798) )
ROM_END


ROM_START( offroadc0 ) // Version 1.00, Mon 07-28-97
	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  // sound data
	ROM_LOAD16_BYTE( "1.0_off_road_u2_sound.u2", 0x000000, 0x80000, CRC(69976e9d) SHA1(63c886ac2563c43a10840f49f929f8613cd94de2) ) // generally a M27C4001 EPROM with label
	ROM_LOAD16_BYTE( "offroad_challenge_5341-15511-01_u3_sound.u3", 0x200000, 0x80000, CRC(2db9b548) SHA1(4f454a3e6a8851b0ef5d325dd28102d57ea11a11) ) // these are mask ROMs
	ROM_LOAD16_BYTE( "offroad_challenge_5341-15511-02_u4_sound.u4", 0x400000, 0x80000, CRC(42bdf9d0) SHA1(04add0f0ee7fa61de1913cc0b988345d3d430cde) )
	ROM_LOAD16_BYTE( "offroad_challenge_5341-15511-03_u5_sound.u5", 0x600000, 0x80000, CRC(569cc84b) SHA1(08b917cc41fae6b6a3e9d9461a783d3d2865e72a) )
	ROM_LOAD16_BYTE( "offroad_challenge_5341-15511-04_u6_sound.u6", 0x800000, 0x80000, CRC(0896f679) SHA1(dde39ef17834256909ef2c9fcd5b5fb9939d5178) )
	ROM_LOAD16_BYTE( "offroad_challenge_5341-15511-05_u7_sound.u7", 0xa00000, 0x80000, CRC(fe242d6a) SHA1(8fbac22ed23044841f309ce58c5b1affcdd5d114) )
	ROM_LOAD16_BYTE( "offroad_challenge_5341-15511-06_u8_sound.u8", 0xc00000, 0x80000, CRC(5da13f12) SHA1(2bb5e929e8bc6c70cb4475024a6b0bb07ac25244) )
	ROM_LOAD16_BYTE( "offroad_challenge_5341-15511-07_u9_sound.u9", 0xe00000, 0x80000, CRC(7ad27f69) SHA1(b33665d0593a95b58d529720aae49e90449bf714) )

	ROM_REGION32_LE( 0x1000000, "maindata", 0 )
	ROM_LOAD32_BYTE( "off_road_u10_game.u10",  0x0000000, 0x100000, CRC(f3535a2c) SHA1(b9578ac9bf3092d983608bc26535127108565062) ) // Version 1.00 program ROMs  - mask ROMS simply labeled:  OFF ROAD   U1x GAME   1997 Midway
	ROM_LOAD32_BYTE( "off_road_u11_game.u11",  0x0000001, 0x100000, CRC(16c904ee) SHA1(ad0a7d9db239cfeb43aba6c4a0a830ba010f397f) )
	ROM_LOAD32_BYTE( "off_road_u12_game.u12",  0x0000002, 0x100000, CRC(d267d7bf) SHA1(eaafba935392444871c17a06e7f513545846aac5) )
	ROM_LOAD32_BYTE( "off_road_u13_game.u13",  0x0000003, 0x100000, CRC(e6274bb3) SHA1(3203d0bf6ce01efd8e28114b95ddf31e70df5e6e) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-01_u14_video_image.u14", 0x0400000, 0x100000, CRC(1e41d14b) SHA1(3f7c5fae1f8b82ddd811720837fa298785a8dd27) ) // mask ROMs
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-02_u15_video_image.u15", 0x0400001, 0x100000, CRC(654d623d) SHA1(a944b8f8d71b099d7b5bbd7df6effb90afc3aec8) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-03_u16_video_image.u16", 0x0400002, 0x100000, CRC(259774d8) SHA1(90cdf659324b84b3c2c59497cc5611e8f12629a6) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-04_u17_video_image.u17", 0x0400003, 0x100000, CRC(50c61434) SHA1(52bc603101b4f88b7d892af683b7c8358cabbf4a) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-05_u18_video_image.u18", 0x0800000, 0x100000, CRC(015be91c) SHA1(1624537068c6bc5fa6235bf0b0343347c337e8d8) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-06_u19_video_image.u19", 0x0800001, 0x100000, CRC(cfc6b70e) SHA1(8c5ad84c50ca142726db0595153cf04caaabec9c) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-07_u20_video_image.u20", 0x0800002, 0x100000, CRC(f48d6e33) SHA1(8b9c205e24f217ac110cdd82388c056ebbbb09b0) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-08_u21_video_image.u21", 0x0800003, 0x100000, CRC(17794b56) SHA1(8bfd8f5b43056bfe7f62524bb8c3a8564a3a9413) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-09_u22_video_image.u22", 0x0c00000, 0x100000, CRC(f2a6e622) SHA1(a7d7004e95b058124cc02e8073dab8fbed8813c5) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-10_u23_video_image.u23", 0x0c00001, 0x100000, CRC(1cba6e20) SHA1(a7c9c58bfc4d26decb08979d83cccedb27528eb6) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-11_u24_video_image.u24", 0x0c00002, 0x100000, CRC(fd3ce11f) SHA1(78c65267712488784bc6dc14eef98a90494a9553) )
	ROM_LOAD32_BYTE( "offroad_challenge_5341-15510-12_u25_video_image.u25", 0x0c00003, 0x100000, CRC(78f8e5db) SHA1(7ec2a5add27d66c43ba5cb7182554321007f5798) )
ROM_END


/*
War Gods
Midway, 1996

This game runs on hardware that appears to be similar to Cruisin' USA but using
a 420M 2.5" IDE hard drive. Only about 100M of the hard drive is used.
There are only 2 ROMs located at U12 and U41.

PCB LAYOUT
|-------------------------------------------------------------------------------------------------------------------|
|                                                                         SEAGATE ST9420AG                          |
|                                                                                                                   |
|                                                                                                                   |
|                                                                                                                   |
|                                                 |-------------|                                                   |
|NEC431008LE-15 x4          LH540204U-20 x2       |MIDWAY       |                U12              16.000MHz         |
|                                                 |5410-14591-00|                |-------------|                    |
| |---------|                      |-------------||(C)1995      |                |MIDWAY       |                    |
| |TMS320C31|         16.6667MHz   |LSI LIA7968  ||             |                |5410-14590-00|   ADSP-2115        |
| |PQL60    |         40.0000MHz   |5410-1346500 ||             |                |(C)1995      |                    |
| |         |                      |MIDWAY MFG CO||-------------|                |-------------|   NEC4218160 x1    |
| |         |                      |             |                                   M628032-15E x3                 |
| |---------|                      |             |                        ALTERA                                    |
|  60.00MHz                        |-------------|  NEC4218160-60 x2      EPM7032LC44-15T                           |
|                                                                                                                   |
|                                                                                                                   |
|                                                                            U41                                    |
|                                                                                                                   |
|                         TMS55165DGH-70 x2          NEC424260-70 x4                                                |
|                                                                                                                   |
|                                                                                  4MHz                  BATT+3V    |
|TL084 x2                                                                          PIC16C57(U69)  MAX232            |
|                         M628032-20E x2                                    |-------------|                         |-|
|AD1866                                                                     |MIDWAY       |     SW2   ULN2064B      | |9 PIN
|                                                                           |5410-14589-00|                         | |SERIAL
|                                                                           |(C) 1995     |     SW1                 |-|LINK
|                                                                           |             |                         |
|                                                                           |             |                         |
|                                                                           |-------------|                         |
|                                                                                                                   |
|                             |--|           J   A   M   M   A           |--|                                       |
|-----------------------------|  |---------------------------------------|  |---------------------------------------|
*/

ROM_START( wargods ) // Boot EPROM Version 1.0, Game Type: 452 (11/07/1996)
	ROM_REGION16_LE( 0x10000, "dcs", 0 )    // sound data
	ROM_LOAD16_BYTE( "u2.rom",   0x000000, 0x8000, CRC(bec7d3ae) SHA1(db80aa4a645804a4574b07b9f34dec6b6b64190d) )

	ROM_REGION32_LE( 0x1000000, "maindata", 0 )
	ROM_LOAD( "u41.rom", 0x000000, 0x20000, CRC(398c54cc) SHA1(6c4b5d6ec5c844dcbf181f9d86a9196a088ed2db) )

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "wargods_11-07-1996", 0, SHA1(7585bc65b1038589cb59d3e7c56e08ca9d7015b8) ) // HDD had a label of 10-09-1996, but the game reports
																						  // a version of 11-07-1996, so it was probably upgraded
																						  // in the field.
	ROM_REGION( 0x2000, "serial_security_pic", 0 ) // security PIC (provides game ID code and serial number)
	ROM_LOAD( "452_wargods.u69",  0x0000, 0x2000, CRC(b908f560) SHA1(68b081f0583aa35c2daeedd43e030ebdcea1a54c) )
ROM_END

ROM_START( wargodsa ) // Boot EPROM Version 1.0, Game Type: 452 (08/15/1996)
	ROM_REGION16_LE( 0x10000, "dcs", 0 )    // sound data
	ROM_LOAD16_BYTE( "u2.rom",   0x000000, 0x8000, CRC(bec7d3ae) SHA1(db80aa4a645804a4574b07b9f34dec6b6b64190d) )

	ROM_REGION32_LE( 0x1000000, "maindata", 0 )
	ROM_LOAD( "u41.rom", 0x000000, 0x20000, CRC(398c54cc) SHA1(6c4b5d6ec5c844dcbf181f9d86a9196a088ed2db) )

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "wargods_08-15-1996", 0, SHA1(5dee00be40c315fbb1d6e3994dae8e498ab87fb2) )

	ROM_REGION( 0x2000, "serial_security_pic", 0 ) // security PIC (provides game ID code and serial number)
	ROM_LOAD( "452_wargods.u69",  0x0000, 0x2000, CRC(b908f560) SHA1(68b081f0583aa35c2daeedd43e030ebdcea1a54c) )
ROM_END

ROM_START( wargodsb ) // Boot EPROM Version 1.0, Game Type: 452 (12/11/1995)
	ROM_REGION16_LE( 0x10000, "dcs", 0 )    // sound data
	ROM_LOAD16_BYTE( "u2.rom",   0x000000, 0x8000, CRC(bec7d3ae) SHA1(db80aa4a645804a4574b07b9f34dec6b6b64190d) )

	ROM_REGION32_LE( 0x1000000, "maindata", 0 )
	ROM_LOAD( "u41.rom", 0x000000, 0x20000, CRC(398c54cc) SHA1(6c4b5d6ec5c844dcbf181f9d86a9196a088ed2db) )

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "wargods_12-11-1995", 0, SHA1(141063f95867fdcc4b15c844e510696604a70c6a) )

	ROM_REGION( 0x2000, "serial_security_pic", 0 ) // security PIC (provides game ID code and serial number)
	ROM_LOAD( "452_wargods.u69",  0x0000, 0x2000, CRC(b908f560) SHA1(68b081f0583aa35c2daeedd43e030ebdcea1a54c) )
ROM_END



/*************************************
 *
 *  Driver init
 *
 *************************************/

uint32_t midvunit_base_state::generic_speedup_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		m_maincpu->eat_cycles(100);
	return m_generic_speedup[offset];
}


void crusnusa_state::init_crusnusa_common(offs_t speedup)
{
	m_adc_shift = 24;

	// speedups
	m_maincpu->space(AS_PROGRAM).install_read_handler(speedup, speedup + 1, read32sm_delegate(*this, FUNC(crusnusa_state::generic_speedup_r)));
	m_generic_speedup = m_ram_base + speedup;
}

void crusnusa_state::init_crusnusa()  { init_crusnusa_common(0xc93e); }
void crusnusa_state::init_crusnu40()  { init_crusnusa_common(0xc957); }
void crusnusa_state::init_crusnu21()  { init_crusnusa_common(0xc051); }


void crusnwld_state::init_crusnwld_common(offs_t speedup)
{
	m_adc_shift = 16;

	// speedups
	if (speedup)
	{
		m_maincpu->space(AS_PROGRAM).install_read_handler(speedup, speedup + 1, read32sm_delegate(*this, FUNC(crusnwld_state::generic_speedup_r)));
		m_generic_speedup = m_ram_base + speedup;
	}
}

void crusnwld_state::init_crusnwld()  { init_crusnwld_common(0xd4c0); }
#if 0
void midvunit_state::init_crusnw20()  { init_crusnwld_common(0xd49c); }
void midvunit_state::init_crusnw13()  { init_crusnwld_common(0); }
#endif

void crusnwld_state::init_offroadc()
{
	m_adc_shift = 16;

	// speedups
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x195aa, 0x195aa, read32sm_delegate(*this, FUNC(crusnwld_state::generic_speedup_r)));
	m_generic_speedup = m_ram_base + 0x195aa;
}


void midvplus_state::init_wargods()
{
	uint8_t default_nvram[256];

	// we need proper NVRAM
	memset(default_nvram, 0xff, sizeof(default_nvram));
	default_nvram[0x0e] = default_nvram[0x2e] = 0x67;
	default_nvram[0x0f] = default_nvram[0x2f] = 0x32;
	default_nvram[0x10] = default_nvram[0x30] = 0x0a;
	default_nvram[0x11] = default_nvram[0x31] = 0x00;
	default_nvram[0x12] = default_nvram[0x32] = 0xaf;
	default_nvram[0x17] = default_nvram[0x37] = 0xd8;
	default_nvram[0x18] = default_nvram[0x38] = 0xe7;
	m_midway_ioasic->set_default_nvram(default_nvram);

	// speedups
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x2f4c, 0x2f4c, read32sm_delegate(*this, FUNC(midvplus_state::generic_speedup_r)));
	m_generic_speedup = m_ram_base + 0x2f4c;
}



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAMEL( 1994, crusnusa,   0,        midvunit, crusnusa, crusnusa_state, init_crusnusa, ROT0, "Midway", "Cruis'n USA (v4.5)", MACHINE_SUPPORTS_SAVE, layout_crusnusa )
GAMEL( 1994, crusnusa44, crusnusa, midvunit, crusnusa, crusnusa_state, init_crusnu40, ROT0, "Midway", "Cruis'n USA (v4.4)", MACHINE_SUPPORTS_SAVE, layout_crusnusa )
GAMEL( 1994, crusnusa41, crusnusa, midvunit, crusnusa, crusnusa_state, init_crusnu40, ROT0, "Midway", "Cruis'n USA (v4.1)", MACHINE_SUPPORTS_SAVE, layout_crusnusa )
GAMEL( 1994, crusnusa40, crusnusa, midvunit, crusnusa, crusnusa_state, init_crusnu40, ROT0, "Midway", "Cruis'n USA (v4.0)", MACHINE_SUPPORTS_SAVE, layout_crusnusa )
GAMEL( 1994, crusnusa21, crusnusa, midvunit, crusnusa, crusnusa_state, init_crusnu21, ROT0, "Midway", "Cruis'n USA (v2.1)", MACHINE_SUPPORTS_SAVE, layout_crusnusa )
GAMEL( 1994, crusnusa20, crusnusa, midvunit, crusnusa, crusnusa_state, init_crusnu21, ROT0, "Midway", "Cruis'n USA (v2.0)", MACHINE_SUPPORTS_SAVE, layout_crusnusa )
GAMEL( 1994, crusnusa11, crusnusa, midvunit, crusnusa, crusnusa_state, init_crusnu21, ROT0, "Midway", "Cruis'n USA (v1.1)", MACHINE_SUPPORTS_SAVE, layout_crusnusa )

GAMEL( 1996, crusnwld,   0,        crusnwld, crusnwld, crusnwld_state, init_crusnwld, ROT0, "Midway", "Cruis'n World (v2.5)", MACHINE_SUPPORTS_SAVE, layout_crusnusa )
GAMEL( 1996, crusnwld24, crusnwld, crusnwld, crusnwld, crusnwld_state, init_crusnwld, ROT0, "Midway", "Cruis'n World (v2.4)", MACHINE_SUPPORTS_SAVE, layout_crusnusa )
GAMEL( 1996, crusnwld23, crusnwld, crusnwld, crusnwld, crusnwld_state, init_crusnwld, ROT0, "Midway", "Cruis'n World (v2.3)", MACHINE_SUPPORTS_SAVE, layout_crusnusa )
GAMEL( 1996, crusnwld20, crusnwld, crusnwld, crusnwld, crusnwld_state, init_crusnwld, ROT0, "Midway", "Cruis'n World (v2.0)", MACHINE_SUPPORTS_SAVE, layout_crusnusa )
GAMEL( 1996, crusnwld19, crusnwld, crusnwld, crusnwld, crusnwld_state, init_crusnwld, ROT0, "Midway", "Cruis'n World (v1.9)", MACHINE_SUPPORTS_SAVE, layout_crusnusa )
GAMEL( 1996, crusnwld17, crusnwld, crusnwld, crusnwld, crusnwld_state, init_crusnwld, ROT0, "Midway", "Cruis'n World (v1.7)", MACHINE_SUPPORTS_SAVE, layout_crusnusa )
GAMEL( 1996, crusnwld13, crusnwld, crusnwld, crusnwld, crusnwld_state, init_crusnwld, ROT0, "Midway", "Cruis'n World (v1.3)", MACHINE_SUPPORTS_SAVE, layout_crusnusa )

GAMEL( 1997, offroadc,  0,         offroadc, offroadc, crusnwld_state, init_offroadc, ROT0, "Midway", "Off Road Challenge (v1.63)", MACHINE_SUPPORTS_SAVE, layout_crusnusa )
GAMEL( 1997, offroadc5, offroadc,  offroadc, offroadc, crusnwld_state, init_offroadc, ROT0, "Midway", "Off Road Challenge (v1.50)", MACHINE_SUPPORTS_SAVE, layout_crusnusa )
GAMEL( 1997, offroadc4, offroadc,  offroadc, offroadc, crusnwld_state, init_offroadc, ROT0, "Midway", "Off Road Challenge (v1.40)", MACHINE_SUPPORTS_SAVE, layout_crusnusa )
GAMEL( 1997, offroadc3, offroadc,  offroadc, offroadc, crusnwld_state, init_offroadc, ROT0, "Midway", "Off Road Challenge (v1.30)", MACHINE_SUPPORTS_SAVE, layout_crusnusa )
GAMEL( 1997, offroadc1, offroadc,  offroadc, offroadc, crusnwld_state, init_offroadc, ROT0, "Midway", "Off Road Challenge (v1.10)", MACHINE_SUPPORTS_SAVE, layout_crusnusa )
GAMEL( 1997, offroadc0, offroadc,  offroadc, offroadc, crusnwld_state, init_offroadc, ROT0, "Midway", "Off Road Challenge (v1.00)", MACHINE_SUPPORTS_SAVE, layout_crusnusa )

GAME(  1995, wargods,   0,         midvplus, wargods,  midvplus_state, init_wargods,  ROT0, "Midway", "War Gods (HD 10/09/1996 - Dual Resolution)", MACHINE_SUPPORTS_SAVE )
GAME(  1995, wargodsa,  wargods,   midvplus, wargodsa, midvplus_state, init_wargods,  ROT0, "Midway", "War Gods (HD 08/15/1996)", MACHINE_SUPPORTS_SAVE )
GAME(  1995, wargodsb,  wargods,   midvplus, wargodsa, midvplus_state, init_wargods,  ROT0, "Midway", "War Gods (HD 12/11/1995)", MACHINE_SUPPORTS_SAVE )
