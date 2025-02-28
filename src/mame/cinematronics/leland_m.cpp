// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Paul Leaman
/***************************************************************************

    Cinematronics / Leland Cinemat System driver

    Leland machine hardware

***************************************************************************/

#include "emu.h"
#include "leland.h"
#include "leland_a.h"

#include "cpu/z80/z80.h"


/*************************************
 *
 *  Debugging
 *
 *************************************/

#define LOG_WARN            (1U << 1)
#define LOG_KEYCARDS        (1U << 2)
#define LOG_KEYCARDS_FULL   (1U << 3)
#define LOG_BANKSWITCHING_M (1U << 4)
#define LOG_BANKSWITCHING_S (1U << 5)
#define LOG_SOUNDPORT       (1U << 6)
#define LOG_EEPROM          (1U << 7)
#define LOG_BATTERY_RAM     (1U << 8)
#define LOG_XROM            (1U << 9)

#define VERBOSE             LOG_WARN
#include "logmacro.h"



/* Internal routines */

/*************************************
 *
 *  Generic dial encoding
 *
 *************************************/

int leland_state::dial_compute_value(int new_val, int indx)
{
	int delta = new_val - (int)m_dial_last_input[indx];
	u8 result = m_dial_last_result[indx] & 0x80;

	m_dial_last_input[indx] = new_val;

	if (delta > 0x80)
		delta -= 0x100;
	else if (delta < -0x80)
		delta += 0x100;

	if (delta < 0)
	{
		result = 0x80;
		delta = -delta;
	}
	else if (delta > 0)
		result = 0x00;

	if (delta > 0x1f)
		delta = 0x1f;
	result |= (m_dial_last_result[indx] + delta) & 0x1f;

	m_dial_last_result[indx] = result;
	return result;
}



/*************************************
 *
 *  Cerberus inputs
 *
 *************************************/

u8 leland_state::cerberus_dial_1_r()
{
	int original = m_io_in[0]->read();
	int modified = dial_compute_value(m_io_an[0]->read(), 0);
	return (original & 0xc0) | ((modified & 0x80) >> 2) | (modified & 0x1f);
}


u8 leland_state::cerberus_dial_2_r()
{
	int original = m_io_in[2]->read();
	int modified = dial_compute_value(m_io_an[1]->read(), 1);
	return (original & 0xc0) | ((modified & 0x80) >> 2) | (modified & 0x1f);
}



/*************************************
 *
 *  Alley Master inputs
 *
 *************************************/

void leland_state::alleymas_joystick_kludge(u8 data)
{
	/* catch the case where they clear this memory location at PC $1827 and change */
	/* the value written to be a 1 */
	if (m_master->pcbase() == 0x1827)
		*m_alleymas_kludge_mem = 1;
	else
		*m_alleymas_kludge_mem = data;

	/* while we're here, make sure the first 3 characters in battery RAM are a */
	/* valid name; otherwise, it will crash if you start a game and don't enter */
	/* your name */
	if (m_battery_ram[0] == 0)
	{
		m_battery_ram[0] = 'C';
		m_battery_ram[1] = 'I';
		m_battery_ram[2] = 'N';
	}
}



/*************************************
 *
 *  Danger Zone inputs
 *
 *************************************/

void leland_state::update_dangerz_xy()
{
	u8 newy = m_io_an[0]->read();
	u8 newx = m_io_an[1]->read();
	int deltay = newy - m_dial_last_input[0];
	int deltax = newx - m_dial_last_input[1];

	if (deltay <= -128) deltay += 256;
	else if (deltay >= 128) deltay -= 256;
	if (deltax <= -128) deltax += 256;
	else if (deltax >= 128) deltax -= 256;

	m_dangerz_y += deltay;
	m_dangerz_x += deltax;
	if (m_dangerz_y < 0) m_dangerz_y = 0;
	else if (m_dangerz_y >= 1024) m_dangerz_y = 1023;
	if (m_dangerz_x < 0) m_dangerz_x = 0;
	else if (m_dangerz_x >= 1024) m_dangerz_x = 1023;

	m_dial_last_input[0] = newy;
	m_dial_last_input[1] = newx;
}


u8 leland_state::dangerz_input_y_r()
{
	update_dangerz_xy();
	return m_dangerz_y & 0xff;
}


u8 leland_state::dangerz_input_x_r()
{
	update_dangerz_xy();
	return m_dangerz_x & 0xff;
}


u8 leland_state::dangerz_input_upper_r()
{
	update_dangerz_xy();
	return ((m_dangerz_y >> 2) & 0xc0) | ((m_dangerz_x >> 8) & 0x03);
}



/*************************************
 *
 *  Red Line Racer inputs
 *
 *************************************/

static const u8 redline_pedal_value[8] = { 0xf0, 0xe0, 0xc0, 0xd0, 0x90, 0xb0, 0x30, 0x70 };

u8 redline_state::redline_pedal_1_r()
{
	ioport_value const pedal = m_io_in[0]->read();
	return redline_pedal_value[pedal >> 5] | 0x0f;
}


u8 redline_state::redline_pedal_2_r()
{
	ioport_value const pedal = m_io_in[2]->read();
	return redline_pedal_value[pedal >> 5] | 0x0f;
}


u8 redline_state::redline_wheel_1_r()
{
	return dial_compute_value(m_io_an[0]->read(), 0);
}


u8 redline_state::redline_wheel_2_r()
{
	return dial_compute_value(m_io_an[1]->read(), 1);
}



/*************************************
 *
 *  Super Offroad inputs
 *
 *************************************/

u8 redline_state::offroad_wheel_1_r()
{
	return dial_compute_value(m_io_an[3]->read(), 0);
}


u8 redline_state::offroad_wheel_2_r()
{
	return dial_compute_value(m_io_an[4]->read(), 1);
}


u8 redline_state::offroad_wheel_3_r()
{
	return dial_compute_value(m_io_an[5]->read(), 2);
}



/*************************************
 *
 *  Ataxx inputs
 *
 *************************************/

u8 ataxx_state::ataxx_trackball_r(offs_t offset)
{
	return dial_compute_value(m_io_an[offset]->read(), offset);
}



/*************************************
 *
 *  Indy Heat inputs
 *
 *************************************/

u8 ataxx_state::indyheat_analog_r(offs_t offset)
{
	switch (offset)
	{
	case 0:
		return 0;

	case 1:
		return m_analog_result;

	case 2:
		return 0;

	case 3:
		LOGMASKED(LOG_WARN, "Unexpected analog read(%02X)\n", 8 + offset);
		break;
	}
	return 0xff;
}


void ataxx_state::indyheat_analog_w(offs_t offset, u8 data)
{
	switch (offset)
	{
		case 3:
			m_analog_result = m_io_an[3 + data]->read();
			break;

		case 0:
		case 1:
		case 2:
			LOGMASKED(LOG_WARN, "Unexpected analog write(%02X) = %02X\n", 8 + offset, data);
			break;
	}
}



/*************************************
 *
 *  Machine initialization
 *
 *************************************/

void leland_state::machine_start()
{
	/* start scanline interrupts going */
	m_master_int_timer = timer_alloc(FUNC(leland_state::leland_interrupt_callback), this);

	save_item(NAME(m_dac_control));
	save_item(NAME(m_wcol_enable));
	save_item(NAME(m_analog_result));
	save_item(NAME(m_dial_last_input));
	save_item(NAME(m_dial_last_result));
	save_item(NAME(m_keycard_shift));
	save_item(NAME(m_keycard_bit));
	save_item(NAME(m_keycard_state));
	save_item(NAME(m_keycard_clock));
	save_item(NAME(m_keycard_command));
	save_item(NAME(m_top_board_bank));
	save_item(NAME(m_sound_port_bank));
	save_item(NAME(m_alternate_bank));
	save_item(NAME(m_battery_ram_enable));
}


void leland_state::machine_reset()
{
	m_master_int_timer->adjust(m_screen->time_until_pos(8), 8);

	/* reset globals */
	sound_port_w(0);
	m_wcol_enable = 0;

	m_dangerz_x = 512;
	m_dangerz_y = 512;
	m_analog_result = 0xff;
	memset(m_dial_last_input, 0, sizeof(m_dial_last_input));
	memset(m_dial_last_result, 0, sizeof(m_dial_last_result));

	m_keycard_shift = 0;
	m_keycard_bit = 0;
	m_keycard_state = 0;
	m_keycard_clock = 0;
	memset(m_keycard_command, 0, sizeof(m_keycard_command));

	m_top_board_bank = 0;
	m_sound_port_bank = 0;
	m_alternate_bank = 0;

	/* initialize the master banks */
	(this->*m_update_master_bank)();

	/* initialize the slave banks */
	if (m_slave_base.length() > 0x10000)
		m_slave_bankslot->set_base(&m_slave_base[0x10000]);
}


void ataxx_state::machine_start()
{
	// TODO: further untangle driver so the base class doesn't have stuff that isn't common and this can call the base implementation
	/* set the odd data banks */
	m_extra_tram = std::make_unique<u8[]>(ATAXX_EXTRA_TRAM_SIZE);

	/* start scanline interrupts going */
	m_master_int_timer = timer_alloc(FUNC(leland_state::ataxx_interrupt_callback), this);

	save_item(NAME(m_wcol_enable));
	save_item(NAME(m_dial_last_input));
	save_item(NAME(m_dial_last_result));
	save_item(NAME(m_master_bank));
	save_item(NAME(m_xrom1_addr));
	save_item(NAME(m_xrom2_addr));
	save_item(NAME(m_battery_ram_enable));
	save_pointer(NAME(m_extra_tram), ATAXX_EXTRA_TRAM_SIZE);
}


void ataxx_state::machine_reset()
{
	// TODO: further untangle driver so the base class doesn't have stuff that isn't common and this can call the base implementation
	memset(m_extra_tram.get(), 0, ATAXX_EXTRA_TRAM_SIZE);
	m_master_int_timer->adjust(m_screen->time_until_pos(8), 8);

	/* initialize the XROM */
	m_xrom1_addr = 0;
	m_xrom2_addr = 0;

	/* reset globals */
	m_wcol_enable = 0;

	m_analog_result = 0xff;
	memset(m_dial_last_input, 0, sizeof(m_dial_last_input));
	memset(m_dial_last_result, 0, sizeof(m_dial_last_result));

	m_master_bank = 0;

	/* initialize the master banks */
	ataxx_bankswitch();

	/* initialize the slave banks */
	if (m_slave_base.length() > 0x10000)
		m_slave_bankslot->set_base(&m_slave_base[0x10000]);
}



/*************************************
 *
 *  Master CPU interrupt handling
 *
 *************************************/

TIMER_CALLBACK_MEMBER(leland_state::leland_interrupt_callback)
{
	u8 scanline = param;

	/* interrupts generated on the VA10 line, which is every */
	/* 16 scanlines starting with scanline #8 */
	m_master->set_input_line(0, HOLD_LINE);

	/* set a timer for the next one */
	scanline += 16;
	m_master_int_timer->adjust(m_screen->time_until_pos(scanline), scanline);
}


TIMER_CALLBACK_MEMBER(leland_state::ataxx_interrupt_callback)
{
	u8 scanline = param;

	/* interrupts generated according to the interrupt control register */
	m_master->set_input_line(0, HOLD_LINE);

	/* set a timer for the next one */
	m_master_int_timer->adjust(m_screen->time_until_pos(scanline), scanline);
}


INTERRUPT_GEN_MEMBER(leland_state::leland_master_interrupt)
{
	/* check for coins here */
	if ((m_io_in[1]->read() & 0x0e) != 0x0e)
		device.execute().set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}



/*************************************
 *
 *  Master CPU bankswitch handlers
 *
 *************************************/

void leland_state::leland_master_alt_bankswitch_w(u8 data)
{
	/* update any bankswitching */
	if ((m_alternate_bank ^ data) & 0x0f)
		LOGMASKED(LOG_BANKSWITCHING_M, "%04X:alternate_bank = %02X\n", m_master->pc(), data & 0x0f);
	m_alternate_bank = data & 15;
	(this->*m_update_master_bank)();
}


void redline_state::redline_master_alt_bankswitch_w(u8 data)
{
	leland_master_alt_bankswitch_w(data);

	/* sound control is in the rest */
	m_sound->leland_80186_control_w(data);
}


/* bankswitching for Cerberus */
void leland_state::cerberus_bankswitch()
{
	/* no bankswitching */
}


/* bankswitching for Mayhem 2002, Power Play, World Series Baseball, and Alley Master */
void leland_state::mayhem_bankswitch()
{
	u8 *address;

	m_battery_ram_enable = ((m_sound_port_bank & 0x24) == 0);

	address = (!(m_sound_port_bank & 0x04)) ? &m_master_base[0x10000] : &m_master_base[0x1c000];
	m_master_bankslot[0]->set_base(address);

	address = m_battery_ram_enable ? m_battery_ram : &address[0x8000];
	m_master_bankslot[1]->set_base(address);
}


/* bankswitching for Danger Zone */
void leland_state::dangerz_bankswitch()
{
	u8 *address;

	m_battery_ram_enable = ((m_top_board_bank & 0x80) != 0);

	address = (!(m_alternate_bank & 1)) ? &m_master_base[0x02000] : &m_master_base[0x12000];
	m_master_bankslot[0]->set_base(address);

	address = m_battery_ram_enable ? m_battery_ram : &address[0x8000];
	m_master_bankslot[1]->set_base(address);
}


/* bankswitching for Baseball the Season II, Super Baseball, and Strike Zone */
void leland_state::basebal2_bankswitch()
{
	u8 *address;

	m_battery_ram_enable = (m_top_board_bank & 0x80);

	if (!m_battery_ram_enable)
		address = (!(m_sound_port_bank & 0x04)) ? &m_master_base[0x10000] : &m_master_base[0x1c000];
	else
		address = (!(m_top_board_bank & 0x40)) ? &m_master_base[0x28000] : &m_master_base[0x30000];
	m_master_bankslot[0]->set_base(address);

	address = m_battery_ram_enable ? m_battery_ram : &address[0x8000];
	m_master_bankslot[1]->set_base(address);
}


/* bankswitching for Red Line Racer */
void leland_state::redline_bankswitch()
{
	static const u32 bank_list[] = { 0x10000, 0x18000, 0x02000, 0x02000 };
	u8 *address;

	m_battery_ram_enable = ((m_alternate_bank & 3) == 1);

	address = &m_master_base[bank_list[m_alternate_bank & 3]];
	m_master_bankslot[0]->set_base(address);

	address = m_battery_ram_enable ? m_battery_ram : &m_master_base[0xa000];
	m_master_bankslot[1]->set_base(address);
}


/* bankswitching for Viper, Quarterback, Team Quarterback, and All American Football */
void leland_state::viper_bankswitch()
{
	static const u32 bank_list[] = { 0x02000, 0x10000, 0x18000, 0x02000 };
	u8 *address;

	m_battery_ram_enable = ((m_alternate_bank & 0x04) != 0);

	address = &m_master_base[bank_list[m_alternate_bank & 3]];
	if (bank_list[m_alternate_bank & 3] >= m_master_base.length())
	{
		LOGMASKED(LOG_WARN, "%s:Master bank %02X out of range!\n", machine().describe_context(), m_alternate_bank & 3);
		address = &m_master_base[bank_list[0]];
	}
	m_master_bankslot[0]->set_base(address);

	address = m_battery_ram_enable ? m_battery_ram : &m_master_base[0xa000];
	m_master_bankslot[1]->set_base(address);
}


/* bankswitching for Super Offroad, Super Offroad Track Pack, and Pig Out */
void leland_state::offroad_bankswitch()
{
	static const u32 bank_list[] = { 0x02000, 0x02000, 0x10000, 0x18000, 0x20000, 0x28000, 0x30000, 0x38000 };
	u8 *address;

	m_battery_ram_enable = ((m_alternate_bank & 7) == 1);

	address = &m_master_base[bank_list[m_alternate_bank & 7]];
	if (bank_list[m_alternate_bank & 7] >= m_master_base.length())
	{
		LOGMASKED(LOG_WARN, "%s:Master bank %02X out of range!\n", machine().describe_context(), m_alternate_bank & 7);
		address = &m_master_base[bank_list[0]];
	}
	m_master_bankslot[0]->set_base(address);

	address = m_battery_ram_enable ? m_battery_ram : &m_master_base[0xa000];
	m_master_bankslot[1]->set_base(address);
}


/* bankswitching for Ataxx, WSF, Indy Heat, and Brute Force */
void ataxx_state::ataxx_bankswitch()
{
	static const u32 bank_list[] =
	{
		0x02000, 0x18000, 0x20000, 0x28000, 0x30000, 0x38000, 0x40000, 0x48000,
		0x50000, 0x58000, 0x60000, 0x68000, 0x70000, 0x78000, 0x80000, 0x88000
	};
	u8 *address;

	m_battery_ram_enable = ((m_master_bank & 0x30) == 0x10);

	address = &m_master_base[bank_list[m_master_bank & 15]];
	if (bank_list[m_master_bank & 15] >= m_master_base.length())
	{
		LOGMASKED(LOG_WARN, "%s:Master bank %02X out of range!\n", machine().describe_context(), m_master_bank & 15);
		address = &m_master_base[bank_list[0]];
	}
	m_master_bankslot[0]->set_base(address);

	if (m_battery_ram_enable)
		address = m_battery_ram;
	else if ((m_master_bank & 0x30) == 0x20)
		address = &m_ataxx_qram[(m_master_bank & 0xc0) << 8];
	else
		address = &m_master_base[0xa000];
	m_master_bankslot[1]->set_base(address);

	m_wcol_enable = ((m_master_bank & 0x30) == 0x30);
}



/*************************************
 *
 *  EEPROM handling (64 x 16bits)
 *
 *************************************/

void leland_state::leland_init_eeprom(u8 default_val, const u16 *data, u8 serial_offset, u8 serial_type)
{
	u8 xorval = (serial_type == SERIAL_TYPE_ADD_XOR || serial_type == SERIAL_TYPE_ENCRYPT_XOR) ? 0xff : 0x00;
	u8 eeprom_data[64*2];
	u32 serial;

	/*
	    NOTE: This code is just illustrative, and explains how to generate the
	    serial numbers for the classic Leland games. We currently load default
	    EEPROM data from the ROMs rather than generating it.

	    Here are the input parameters for various games:

	        game        default_val     serial_offset   serial_type
	        cerberus    0x00            0               SERIAL_TYPE_NONE
	        mayhem      0x00            0x28            SERIAL_TYPE_ADD
	        powrplay    0xff            0x2d            SERIAL_TYPE_ADD_XOR
	        wseries     0xff            0x12            SERIAL_TYPE_ENCRYPT_XOR
	        alleymas    0xff            0x0c            SERIAL_TYPE_ENCRYPT_XOR
	        upyoural    0xff            0x0c            SERIAL_TYPE_ENCRYPT_XOR
	        dangerz     0xff            0x10            SERIAL_TYPE_ENCRYPT_XOR
	        basebal2    0xff            0x12            SERIAL_TYPE_ENCRYPT_XOR
	        dblplay     0xff            0x11            SERIAL_TYPE_ENCRYPT_XOR
	        strkzone    0xff            0x0f            SERIAL_TYPE_ENCRYPT_XOR
	        redlin2p    0xff            0x18            SERIAL_TYPE_ENCRYPT_XOR
	        quarterb    0xff            0x24            SERIAL_TYPE_ENCRYPT_XOR
	        viper       0xff            0x0c            SERIAL_TYPE_ENCRYPT_XOR
	        teamqb      0xff            0x1a            SERIAL_TYPE_ENCRYPT_XOR
	        aafb        0xff            0x1a            SERIAL_TYPE_ENCRYPT_XOR
	        offroad     0xff            0x00            SERIAL_TYPE_ENCRYPT_XOR
	        pigout      0xff            0x00            SERIAL_TYPE_ENCRYPT
	*/

	/* initialize everything to the default value */
	memset(eeprom_data, default_val, sizeof(eeprom_data));

	/* fill in the preset data */
	while (*data != 0xffff)
	{
		int offset = *data++;
		int value = *data++;
		eeprom_data[offset * 2 + 0] = value >> 8;
		eeprom_data[offset * 2 + 1] = value & 0xff;
	}

	/* pick a serial number -- examples of real serial numbers:

	    Team QB:      21101957
	    AAFB:         26101119 and 26101039
	*/
	serial = 0x12345678;

	/* switch off the serial number type */
	switch (serial_type)
	{
		case SERIAL_TYPE_ADD:
		case SERIAL_TYPE_ADD_XOR:
		{
			int i;
			for (i = 0; i < 10; i++)
			{
				int digit;

				if (i >= 8)
					digit = 0;
				else
					digit = ((serial << (i * 4)) >> 28) & 15;
				digit = ('0' + digit) * 2;

				eeprom_data[serial_offset * 2 +  0 + (i ^ 1)] = (digit / 3) ^ xorval;
				eeprom_data[serial_offset * 2 + 10 + (i ^ 1)] = (digit / 3) ^ xorval;
				eeprom_data[serial_offset * 2 + 20 + (i ^ 1)] = (digit - (2 * (digit / 3))) ^ xorval;
			}
			break;
		}

		case SERIAL_TYPE_ENCRYPT:
		case SERIAL_TYPE_ENCRYPT_XOR:
		{
			int d, e, h, l;

			/* break the serial number out into pieces */
			l = (serial >> 24) & 0xff;
			h = (serial >> 16) & 0xff;
			e = (serial >> 8) & 0xff;
			d = serial & 0xff;

			/* decrypt the data */
			h = ((h ^ 0x2a ^ l) ^ 0xff) + 5;
			d = ((d + 0x2a) ^ e) ^ 0xff;
			l ^= e;
			e ^= 0x2a;

			/* store the bytes */
			eeprom_data[serial_offset * 2 + 0] = h ^ xorval;
			eeprom_data[serial_offset * 2 + 1] = l ^ xorval;
			eeprom_data[serial_offset * 2 + 2] = d ^ xorval;
			eeprom_data[serial_offset * 2 + 3] = e ^ xorval;
			break;
		}
	}
}



/*************************************
 *
 *  EEPROM handling (128 x 16bits)
 *
 *************************************/

void leland_state::ataxx_init_eeprom(const u16 *data)
{
	u8 eeprom_data[128*2];
	u8 serial_offset = 0;
	u8 default_val = 0;
	u32 serial;

	/*
	    NOTE: This code is just illustrative, and explains how to generate the
	    serial numbers for the classic Leland games. We currently load default
	    EEPROM data from the ROMs rather than generating it.
	*/

	/* initialize everything to the default value */
	memset(eeprom_data, default_val, sizeof(eeprom_data));

	/* fill in the preset data */
	while (*data != 0xffff)
	{
		int offset = *data++;
		int value = *data++;
		eeprom_data[offset * 2 + 0] = value >> 8;
		eeprom_data[offset * 2 + 1] = value & 0xff;
	}

	/* pick a serial number -- examples of real serial numbers:

	    WSF:         30101190
	    Indy Heat:   31201339
	*/
	serial = 0x12345678;

	/* encrypt the serial number */
	{
		int d, e, h, l;

		/* break the serial number out into pieces */
		l = (serial >> 24) & 0xff;
		h = (serial >> 16) & 0xff;
		e = (serial >> 8) & 0xff;
		d = serial & 0xff;

		/* decrypt the data */
		h = ((h ^ 0x2a ^ l) ^ 0xff) + 5;
		d = ((d + 0x2a) ^ e) ^ 0xff;
		l ^= e;
		e ^= 0x2a;

		/* store the bytes */
		eeprom_data[serial_offset * 2 + 0] = h;
		eeprom_data[serial_offset * 2 + 1] = l;
		eeprom_data[serial_offset * 2 + 2] = d;
		eeprom_data[serial_offset * 2 + 3] = e;
	}

	/* compute the checksum */
	{
		int i, sum = 0;
		for (i = 0; i < 0x7f * 2; i++)
			sum += eeprom_data[i];
		sum ^= 0xffff;
		eeprom_data[0x7f * 2 + 0] = (sum >> 8) & 0xff;
		eeprom_data[0x7f * 2 + 1] = sum & 0xff;
	}
}



/*************************************
 *
 *  Ataxx EEPROM interfacing
 *
 *************************************/

u8 ataxx_state::eeprom_r()
{
	int port = m_io_in[2]->read();
	LOGMASKED(LOG_EEPROM, "%s:EE read\n", machine().describe_context());
	return port;
}


void ataxx_state::eeprom_w(u8 data)
{
	LOGMASKED(LOG_EEPROM, "%s:EE write %d%d%d\n", machine().describe_context(),
			(data >> 6) & 1, (data >> 5) & 1, (data >> 4) & 1);
	m_eeprom->di_write (BIT(data, 4));
	m_eeprom->clk_write(BIT(data, 5));
	m_eeprom->cs_write (BIT(data, 6));
}



/*************************************
 *
 *  Battery backed RAM
 *
 *************************************/

void leland_state::leland_battery_ram_w(offs_t offset, u8 data)
{
	if (m_battery_ram_enable)
	{
		LOGMASKED(LOG_BATTERY_RAM, "%04X:BatteryW@%04X=%02X\n", m_master->pc(), offset, data);
		m_battery_ram[offset] = data;
	}
	else
		LOGMASKED(LOG_WARN, "%04X:BatteryW@%04X (invalid!)\n", m_master->pc(), offset);
}


void ataxx_state::ataxx_battery_ram_w(offs_t offset, u8 data)
{
	if (m_battery_ram_enable)
	{
		LOGMASKED(LOG_BATTERY_RAM, "%04X:BatteryW@%04X=%02X\n", m_master->pc(), offset, data);
		m_battery_ram[offset] = data;
	}
	else if ((m_master_bank & 0x30) == 0x20)
	{
		m_screen->update_partial(m_screen->vpos() - 1);
		m_ataxx_qram[((m_master_bank & 0xc0) << 8) | offset] = data;
		m_tilemap->mark_tile_dirty(((m_master_bank & 0x80) << 8) | offset);
	}
	else
		LOGMASKED(LOG_WARN, "%04X:BatteryW@%04X (invalid!)\n", m_master->pc(), offset);
}



/*************************************
 *
 *  Master CPU keycard I/O
 *
 ************************************/

/*--------------------------------------------------------------------

  A note about keycards:

  These were apparently static programmable ROMs that could be
  inserted into certain games. The games would then save/load
  statistics on them.

  The data is accessed via a serial protocol, which is
  implemented below. There are two known commands that are
  written; each command is 3 bytes and accesses 128 bytes of
  data from the keycard:

        62 00 80
        9D 00 80

  the last byte appears to specify the length of data to transfer.

  The format of the data on the card is pretty heavily encrypted.
  The first 7 bytes read serves as a header:

        D5 43 49 4E 2A xx yy

  where xx is a game-specific key, and yy is the complement of the
  key. For example, World Series Baseball uses 04/FB. Alley Master
  uses 05/FA.

  The last 112 bytes of data is encrypted via the following method:

        for (i = 16, b = 0x70, r = 0x08; i < 128; i++, b--, r += 0x10)
        {
            u8 a = original_data[i] ^ 0xff;
            a = (a >> 3) | (a << 5);
            a = (((a ^ r) + 1 + b) ^ b) - b;
            encrypted_data[i] = a;
        }

  The data that is encrypted is stored alternating with a checksum
  byte. The checksum for a value A is ((A ^ 0xa5) + 0x27) ^ 0x34.

--------------------------------------------------------------------*/

int leland_state::keycard_r()
{
	int result = 0;

	LOGMASKED(LOG_KEYCARDS_FULL, "  (%s:keycard_r)\n", machine().describe_context());

	/* if we have a valid keycard read state, we're reading from the keycard */
	if (m_keycard_state & 0x80)
	{
		/* clock in new data */
		if (m_keycard_bit == 1)
		{
			m_keycard_shift = 0xff;  /* no data, but this is where we would clock it in */
			LOGMASKED(LOG_KEYCARDS, "  (clocked in %02X)\n", m_keycard_shift);
		}

		/* clock in the bit */
		result = (~m_keycard_shift & 1) << ((m_keycard_state >> 4) & 3);
		LOGMASKED(LOG_KEYCARDS, "  (read %02X)\n", result);
	}
	return result;
}

void leland_state::keycard_w(int data)
{
	int new_state = data & 0xb0;
	int new_clock = data & 0x40;

	LOGMASKED(LOG_KEYCARDS_FULL, "  (%s:keycard_w=%02X)\n", machine().describe_context(), data);

	/* check for going active */
	if (!m_keycard_state && new_state)
	{
		m_keycard_command[0] = m_keycard_command[1] = m_keycard_command[2] = 0;
		LOGMASKED(LOG_KEYCARDS, "keycard going active (state=%02X)\n", new_state);
	}

	/* check for going inactive */
	else if (m_keycard_state && !new_state)
	{
		m_keycard_command[0] = m_keycard_command[1] = m_keycard_command[2] = 0;
		LOGMASKED(LOG_KEYCARDS, "keycard going inactive\n");
	}

	/* check for clocks */
	else if (m_keycard_state == new_state)
	{
		/* work off of falling edge */
		if (!new_clock && m_keycard_clock)
		{
			m_keycard_shift >>= 1;
			m_keycard_bit = (m_keycard_bit + 1) & 7;
		}

		/* look for a bit write */
		else if (!new_clock && !m_keycard_clock && !(data & 0x80))
		{
			LOGMASKED(LOG_KEYCARDS, "  (write %02X)\n", data);

			m_keycard_shift &= ~0x80;
			if (data & (1 << ((new_state >> 4) & 3)))
				m_keycard_shift |= 0x80;

			/* clock out the data on the last bit */
			if (m_keycard_bit == 7)
			{
				LOGMASKED(LOG_KEYCARDS, "  (clocked out %02X)\n", m_keycard_shift);
				m_keycard_command[0] = m_keycard_command[1];
				m_keycard_command[1] = m_keycard_command[2];
				m_keycard_command[2] = m_keycard_shift;
				if (m_keycard_command[0] == 0x62 && m_keycard_command[1] == 0x00 && m_keycard_command[2] == 0x80)
				{
					LOGMASKED(LOG_KEYCARDS, "  (got command $62)\n");
				}
			}
		}
	}

	/* error case */
	else
	{
		/* only an error if the selected bit changes; read/write transitions are okay */
		if ((new_state & 0x30) != (m_keycard_state & 0x30))
			LOGMASKED(LOG_KEYCARDS, "ERROR: Caught keycard state transition %02X -> %02X\n", m_keycard_state, new_state);
	}

	m_keycard_state = new_state;
	m_keycard_clock = new_clock;
}



/*************************************
 *
 *  Master CPU analog and keycard I/O
 *
 *************************************/

u8 leland_state::master_analog_key_r(offs_t offset)
{
	int result = 0;

	switch (offset)
	{
		case 0x00:  /* FD = analog data read */
			result = m_analog_result;
			break;

		case 0x01:  /* FE = analog status read */
			/* bit 7 indicates the analog input is busy for some games */
			result = 0x00;
			break;

		case 0x02:  /* FF = keycard serial data read */
			result = keycard_r();

			/* bit 7 indicates the analog input is busy for some games */
			result &= ~0x80;
			break;
	}
	return result;
}



void leland_state::master_analog_key_w(offs_t offset, u8 data)
{
	switch (offset)
	{
		case 0x00:  /* FD = analog port trigger */
			break;

		case 0x01:  /* FE = analog port select/bankswitch */
			m_analog_result = m_io_an[data & 15]->read();

			/* update top board banking for some games */
			if ((m_top_board_bank ^ data) & 0xc0)
				LOGMASKED(LOG_BANKSWITCHING_M, "%04X:top_board_bank = %02X\n", m_master->pc(), data & 0xc0);
			m_top_board_bank = data & 0xc0;
			(this->*m_update_master_bank)();
			break;

		case 0x02:  /* FF = keycard data write */
			keycard_w(data);
			break;
	}
}



/*************************************
 *
 *  Master CPU internal I/O
 *
 *************************************/

u8 leland_state::leland_master_input_r(offs_t offset)
{
	int result = 0xff;

	switch (offset)
	{
		case 0x00:  /* /GIN0 */
			result = m_io_in[0]->read();
			break;

		case 0x01:  /* /GIN1 */
			result = m_io_in[1]->read();
			if (m_slave->state_int(Z80_HALT))
				result ^= 0x01;
			break;

		case 0x02:  /* /GIN2 */
		case 0x12:
			m_master->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
			break;

		case 0x03:  /* /IGID */
		case 0x13:
			if (m_ay8910)
				result &= m_ay8910->data_r();

			if (m_ay8912)
				result &= m_ay8912->data_r();
			break;

		case 0x10:  /* /GIN0 */
			result = m_io_in[2]->read();
			break;

		case 0x11:  /* /GIN1 */
			result = m_io_in[3]->read();
			LOGMASKED(LOG_EEPROM, "%s:EE read\n", machine().describe_context());
			break;

		default:
			LOGMASKED(LOG_WARN, "Master I/O read offset %02X\n", offset);
			break;
	}
	return result;
}


void leland_state::leland_master_output_w(offs_t offset, u8 data)
{
	switch (offset)
	{
		case 0x09:  /* /MCONT */
			m_slave->set_input_line(INPUT_LINE_RESET, BIT(data, 0) ? CLEAR_LINE : ASSERT_LINE);
			m_wcol_enable = BIT(data, 1);
			m_slave->set_input_line(INPUT_LINE_NMI, BIT(data, 2) ? CLEAR_LINE : ASSERT_LINE);
			m_slave->set_input_line(0, BIT(data, 3) ? CLEAR_LINE : ASSERT_LINE);

			LOGMASKED(LOG_EEPROM, "%s:EE write %d%d%d\n", machine().describe_context(),
					(data >> 6) & 1, (data >> 5) & 1, (data >> 4) & 1);
			m_eeprom->di_write (BIT(data, 4));
			m_eeprom->clk_write(BIT(data, 5));
			m_eeprom->cs_write (BIT(data, 6));
			break;

		case 0x0a:  /* /OGIA */
		case 0x0b:  /* /OGID */
			if (m_ay8910)
				m_ay8910->address_data_w(offset - 0x0a, data);

			if (m_ay8912)
				m_ay8912->address_data_w(offset - 0x0a, data);
			break;

		case 0x0c:  /* /BKXL */
		case 0x0d:  /* /BKXH */
		case 0x0e:  /* /BKYL */
		case 0x0f:  /* /BKYH */
			scroll_w(offset - 0x0c, data);
			break;

		default:
			LOGMASKED(LOG_WARN, "Master I/O write offset %02X=%02X\n", offset, data);
			break;
	}
}


u8 ataxx_state::ataxx_master_input_r(offs_t offset)
{
	int result = 0xff;

	switch (offset)
	{
		case 0x06:  /* /GIN0 */
			result = m_io_in[0]->read();
			break;

		case 0x07:  /* /SLVBLK */
			result = m_io_in[1]->read();
			if (m_slave->state_int(Z80_HALT))
				result ^= 0x01;
			break;

		default:
			LOGMASKED(LOG_WARN, "Master I/O read offset %02X\n", offset);
			break;
	}
	return result;
}


void ataxx_state::ataxx_master_output_w(offs_t offset, u8 data)
{
	switch (offset)
	{
		case 0x00:  /* /BKXL */
		case 0x01:  /* /BKXH */
		case 0x02:  /* /BKYL */
		case 0x03:  /* /BKYH */
			scroll_w(offset, data);
			break;

		case 0x04:  /* /MBNK */
			if ((m_master_bank ^ data) & 0xff)
				LOGMASKED(LOG_BANKSWITCHING_M, "%04X:master_bank = %02X\n", m_master->pc(), data & 0xff);
			m_master_bank = data;
			ataxx_bankswitch();
			break;

		case 0x05:  /* /SLV0 */
			m_slave->set_input_line(0, (data & 0x01) ? CLEAR_LINE : ASSERT_LINE);
			m_slave->set_input_line(INPUT_LINE_NMI, (data & 0x04) ? CLEAR_LINE : ASSERT_LINE);
			m_slave->set_input_line(INPUT_LINE_RESET, (data & 0x10) ? CLEAR_LINE : ASSERT_LINE);
			break;

		case 0x08:  /*  */
		{
			u8 scanline = data + 1;
			m_master_int_timer->adjust(m_screen->time_until_pos(scanline), scanline);
			break;
		}

		default:
			LOGMASKED(LOG_WARN, "Master I/O write offset %02X=%02X\n", offset, data);
			break;
	}
}



/*************************************
 *
 *  Master CPU palette gates
 *
 *************************************/

void leland_state::gated_paletteram_w(offs_t offset, u8 data)
{
	if (m_wcol_enable)
		m_palette->write8(offset, data);
}


u8 leland_state::gated_paletteram_r(offs_t offset)
{
	if (m_wcol_enable)
		return m_palette->basemem().read8(offset);
	return 0xff;
}


void ataxx_state::paletteram_and_misc_w(offs_t offset, u8 data)
{
	if (m_wcol_enable)
		m_palette->write8(offset, data);
	else if (offset == 0x7f8 || offset == 0x7f9)
		master_video_addr_w(offset - 0x7f8, data);
	else if (offset == 0x7fc)
	{
		m_xrom1_addr = (m_xrom1_addr & 0xff00) | (data & 0x00ff);
		LOGMASKED(LOG_XROM, "%04X:XROM1 address low write = %02X (addr=%04X)\n", m_master->pc(), data, m_xrom1_addr);
	}
	else if (offset == 0x7fd)
	{
		m_xrom1_addr = (m_xrom1_addr & 0x00ff) | ((data << 8) & 0xff00);
		LOGMASKED(LOG_XROM, "%04X:XROM1 address high write = %02X (addr=%04X)\n", m_master->pc(), data, m_xrom1_addr);
	}
	else if (offset == 0x7fe)
	{
		m_xrom2_addr = (m_xrom2_addr & 0xff00) | (data & 0x00ff);
		LOGMASKED(LOG_XROM, "%04X:XROM2 address low write = %02X (addr=%04X)\n", m_master->pc(), data, m_xrom2_addr);
	}
	else if (offset == 0x7ff)
	{
		m_xrom2_addr = (m_xrom2_addr & 0x00ff) | ((data << 8) & 0xff00);
		LOGMASKED(LOG_XROM, "%04X:XROM2 address high write = %02X (addr=%04X)\n", m_master->pc(), data, m_xrom2_addr);
	}
	else
		m_extra_tram[offset] = data;
}


u8 ataxx_state::paletteram_and_misc_r(offs_t offset)
{
	if (m_wcol_enable)
		return m_palette->basemem().read8(offset);
	else if (offset == 0x7fc || offset == 0x7fd)
	{
		int result = m_xrom_base[0x00000 | m_xrom1_addr | ((offset & 1) << 16)];
		LOGMASKED(LOG_XROM, "%04X:XROM1 read(%d) = %02X (addr=%04X)\n", m_master->pc(), offset - 0x7fc, result, m_xrom1_addr);
		return result;
	}
	else if (offset == 0x7fe || offset == 0x7ff)
	{
		int result = m_xrom_base[0x20000 | m_xrom2_addr | ((offset & 1) << 16)];
		LOGMASKED(LOG_XROM, "%04X:XROM2 read(%d) = %02X (addr=%04X)\n", m_master->pc(), offset - 0x7fc, result, m_xrom2_addr);
		return result;
	}
	else
		return m_extra_tram[offset];
}



/*************************************
 *
 *  AY8910-controlled graphics latch
 *
 *************************************/

u8 leland_state::sound_port_r()
{
	return m_gfx_control;
}


void leland_state::sound_port_w(u8 data)
{
	/* update the graphics banking */
	gfx_port_w(data);

	/* set the new value */
	m_gfx_control = data;
	m_dac_control = data & 3;

	/* some bankswitching occurs here */
	if ((m_sound_port_bank ^ data) & 0x24)
		LOGMASKED(LOG_BANKSWITCHING_M, "%s:sound_port_bank = %02X\n", machine().describe_context(), data & 0x24);
	m_sound_port_bank = data & 0x24;
	(this->*m_update_master_bank)();
}



/*************************************
 *
 *  Slave CPU bankswitching
 *
 *************************************/

void leland_state::slave_small_banksw_w(u8 data)
{
	int bankaddress = 0x10000 + 0xc000 * (data & 1);

	if (bankaddress >= m_slave_base.length())
	{
		LOGMASKED(LOG_WARN, "%04X:Slave bank %02X out of range!", m_slave->pc(), data & 1);
		bankaddress = 0x10000;
	}
	m_slave_bankslot->set_base(&m_slave_base[bankaddress]);

	LOGMASKED(LOG_BANKSWITCHING_S, "%04X:Slave bank = %02X (%05X)\n", m_slave->pc(), data & 1, bankaddress);
}


void leland_state::slave_large_banksw_w(u8 data)
{
	int bankaddress = 0x10000 + 0x8000 * (data & 15);

	if (bankaddress >= m_slave_base.length())
	{
		LOGMASKED(LOG_WARN, "%04X:Slave bank %02X out of range!", m_slave->pc(), data & 15);
		bankaddress = 0x10000;
	}
	m_slave_bankslot->set_base(&m_slave_base[bankaddress]);

	LOGMASKED(LOG_BANKSWITCHING_S, "%04X:Slave bank = %02X (%05X)\n", m_slave->pc(), data & 15, bankaddress);
}


void leland_state::ataxx_slave_banksw_w(u8 data)
{
	int bankaddress, bank = data & 15;

	if (bank == 0)
		bankaddress = 0x2000;
	else
	{
		bankaddress = 0x10000 * bank + 0x8000 * ((data >> 4) & 1);
		if (m_slave_base.length() > 0x100000)
			bankaddress += 0x100000 * ((data >> 5) & 1);
	}

	if (bankaddress >= m_slave_base.length())
	{
		LOGMASKED(LOG_WARN, "%04X:Slave bank %02X out of range!", m_slave->pc(), data & 0x3f);
		bankaddress = 0x2000;
	}
	m_slave_bankslot->set_base(&m_slave_base[bankaddress]);

	LOGMASKED(LOG_BANKSWITCHING_S, "%04X:Slave bank = %02X (%05X)\n", m_slave->pc(), data, bankaddress);
}



/*************************************
 *
 *  Slave CPU I/O
 *
 *************************************/

u8 leland_state::raster_r()
{
	return m_screen->vpos();
}



/*************************************
 *
 *  Driver initialization
 *
 *************************************/

/* also called by Ataxx */
void leland_state::rotate_memory(const char *cpuname)
{
	int startaddr = 0x10000;
	int banks = (memregion(cpuname)->bytes() - startaddr) / 0x8000;
	u8 *ram = memregion(cpuname)->base();
	u8 temp[0x2000];

	for (int i = 0; i < banks; i++)
	{
		memmove(temp, &ram[startaddr + 0x0000], 0x2000);
		memmove(&ram[startaddr + 0x0000], &ram[startaddr + 0x2000], 0x6000);
		memmove(&ram[startaddr + 0x6000], temp, 0x2000);
		startaddr += 0x8000;
	}
}
