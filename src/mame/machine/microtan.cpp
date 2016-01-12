// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller
/******************************************************************************
 *  Microtan 65
 *
 *  machine driver
 *
 *  Juergen Buchmueller <pullmoll@t-online.de>, Jul 2000
 *
 *  Thanks go to Geoff Macdonald <mail@geoff.org.uk>
 *  for his site http:://www.geo255.redhotant.com
 *  and to Fabrice Frances <frances@ensica.fr>
 *  for his site http://www.ifrance.com/oric/microtan.html
 *
 *****************************************************************************/

/* Core includes */
#include "emu.h"
#include "includes/microtan.h"

/* Components */
#include "cpu/m6502/m6502.h"
#include "machine/6522via.h"
#include "machine/mos6551.h"
#include "sound/ay8910.h"

/* Devices */
#include "imagedev/cassette.h"
//#include "imagedev/snapquik.h"


#ifndef VERBOSE
#define VERBOSE 0
#endif

#define LOG(x)  do { if (VERBOSE) logerror x; } while (0)




static const char keyboard[8][9][8] = {
	{ /* normal */
		{ 27,'1','2','3','4','5','6','7'},
		{'8','9','0','-','=','`',127,  9},
		{'q','w','e','r','t','y','u','i'},
		{'o','p','[',']', 13,127,  0,  0},
		{'a','s','d','f','g','h','j','k'},
		{'l',';', 39, 92,  0,'z','x','c'},
		{'v','b','n','m',',','.','/',  0},
		{ 10,' ','-',',', 13,'.','0','1'},
		{'2','3','4','5','6','7','8','9'},
	},
	{ /* Shift */
		{ 27,'!','@','#','$','%','^','&'},
		{'*','(',')','_','+','~',127,  9},
		{'Q','W','E','R','T','Y','U','I'},
		{'O','P','{','}', 13,127,  0,  0},
		{'A','S','D','F','G','H','J','K'},
		{'L',':','"','|',  0,'Z','X','C'},
		{'V','B','N','M','<','>','?',  0},
		{ 10,' ','-',',', 13,'.','0','1'},
		{'2','3','4','5','6','7','8','9'},
	},
	{ /* Control */
		{ 27,'1','2','3','4','5','6','7'},
		{'8','9','0','-','=','`',127,  9},
		{ 17, 23,  5, 18, 20, 25, 21,  9},
		{ 15, 16, 27, 29, 13,127,  0,  0},
		{  1, 19,  4,  6,  7,  8, 10, 11},
		{ 12,';', 39, 28,  0, 26, 24,  3},
		{ 22,  2, 14, 13,',','.','/',  0},
		{ 10,' ','-',',', 13,'.','0','1'},
		{'2','3','4','5','6','7','8','9'},
	},
	{ /* Shift+Control */
		{ 27,'!',  0,'#','$','%', 30,'&'},
		{'*','(',')', 31,'+','~',127,  9},
		{ 17, 23,  5, 18, 20, 25, 21,  9},
		{ 15, 16, 27, 29, 13,127,  0,  0},
		{  1, 19,  4,  6,  7,  8, 10, 11},
		{ 12,':','"', 28,  0, 26, 24,  3},
		{ 22,  2, 14, 13,',','.','/',  0},
		{ 10,' ','-',',', 13,'.','0','1'},
		{'2','3','4','5','6','7','8','9'},
	},
	{ /* CapsLock */
		{ 27,'1','2','3','4','5','6','7'},
		{'8','9','0','-','=','`',127,  9},
		{'Q','W','E','R','T','Y','U','I'},
		{'O','P','[',']', 13,127,  0,  0},
		{'A','S','D','F','G','H','J','K'},
		{'L',';', 39, 92,  0,'Z','X','C'},
		{'V','B','N','M',',','.','/',  0},
		{ 10,' ','-',',', 13,'.','0','1'},
		{'2','3','4','5','6','7','8','9'},
	},
	{ /* Shift+CapsLock */
		{ 27,'!','@','#','$','%','^','&'},
		{'*','(',')','_','+','~',127,  9},
		{'Q','W','E','R','T','Y','U','I'},
		{'O','P','{','}', 13,127,  0,  0},
		{'A','S','D','F','G','H','J','K'},
		{'L',':','"','|',  0,'Z','X','C'},
		{'V','B','N','M','<','>','?',  0},
		{ 10,' ','-',',', 13,'.','0','1'},
		{'2','3','4','5','6','7','8','9'},
	},
	{ /* Control+CapsLock */
		{ 27,'1','2','3','4','5','6','7'},
		{'8','9','0','-','=','`',127,  9},
		{ 17, 23,  5, 18, 20, 25, 21,  9},
		{ 15, 16, 27, 29, 13,127,  0,  0},
		{  1, 19,  4,  6,  7,  8, 10, 11},
		{ 12,';', 39, 28,  0, 26, 24,  3},
		{ 22,  2, 14, 13,',','.','/',  0},
		{ 10,' ','-',',', 13,'.','0','1'},
		{'2','3','4','5','6','7','8','9'},
	},
	{ /* Shift+Control+CapsLock */
		{ 27,'!',  0,'#','$','%', 30,'&'},
		{'*','(',')', 31,'+','~',127,  9},
		{ 17, 23,  5, 18, 20, 25, 21,  9},
		{ 15, 16, 27, 29, 13,127,  0,  0},
		{  1, 19,  4,  6,  7,  8, 10, 11},
		{ 12,':','"', 28,  0, 26, 24,  3},
		{ 22,  2, 14, 13,',','.','/',  0},
		{ 10,' ','-',',', 13,'.','0','1'},
		{'2','3','4','5','6','7','8','9'},
	},
};

UINT8 microtan_state::read_dsw()
{
	UINT8 result;
	switch(machine().phase())
	{
		case MACHINE_PHASE_RESET:
		case MACHINE_PHASE_RUNNING:
			result = ioport("DSW")->read();
			break;

		default:
			result = 0x00;
			break;
	}
	return result;
}

void microtan_state::microtan_set_irq_line()
{
	/* The 6502 IRQ line is active low and probably driven
	   by open collector outputs (guess). Since MAME/MESS use
	   a non-0 value for ASSERT_LINE we OR the signals here */
	m_maincpu->set_input_line(0, m_via_0_irq_line | m_via_1_irq_line | m_kbd_irq_line);
}

/**************************************************************
 * VIA callback functions for VIA #0
 **************************************************************/
READ8_MEMBER(microtan_state::via_0_in_a)
{
	int data = ioport("JOY")->read();
	LOG(("microtan_via_0_in_a %02X\n", data));
	return data;
}

WRITE8_MEMBER(microtan_state::via_0_out_a)
{
	LOG(("microtan_via_0_out_a %02X\n", data));
}

WRITE8_MEMBER(microtan_state::via_0_out_b)
{
	LOG(("microtan_via_0_out_b %02X\n", data));
	/* bit #7 is the cassette output signal */
	m_cassette->output(data & 0x80 ? +1.0 : -1.0);
}

WRITE_LINE_MEMBER(microtan_state::via_0_out_ca2)
{
	LOG(("microtan_via_0_out_ca2 %d\n", state));
}

WRITE_LINE_MEMBER(microtan_state::via_0_out_cb2)
{
	LOG(("microtan_via_0_out_cb2 %d\n", state));
}

WRITE_LINE_MEMBER(microtan_state::via_0_irq)
{
	LOG(("microtan_via_0_irq %d\n", state));
	m_via_0_irq_line = state;
	microtan_set_irq_line();
}

/**************************************************************
 * VIA callback functions for VIA #1
 **************************************************************/

WRITE8_MEMBER(microtan_state::via_1_out_a)
{
	LOG(("microtan_via_1_out_a %02X\n", data));
}

WRITE8_MEMBER(microtan_state::via_1_out_b)
{
	LOG(("microtan_via_1_out_b %02X\n", data));
}

WRITE_LINE_MEMBER(microtan_state::via_1_out_ca2)
{
	LOG(("microtan_via_1_out_ca2 %d\n", state));
}

WRITE_LINE_MEMBER(microtan_state::via_1_out_cb2)
{
	LOG(("microtan_via_1_out_cb2 %d\n", state));
}

WRITE_LINE_MEMBER(microtan_state::via_1_irq)
{
	LOG(("microtan_via_1_irq %d\n", state));
	m_via_1_irq_line = state;
	microtan_set_irq_line();
}

void microtan_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_READ_CASSETTE:
		microtan_read_cassette(ptr, param);
		break;
	case TIMER_PULSE_NMI:
		microtan_pulse_nmi(ptr, param);
		break;
	default:
		assert_always(FALSE, "Unknown id in microtan_state::device_timer");
	}
}


TIMER_CALLBACK_MEMBER(microtan_state::microtan_read_cassette)
{
	double level = m_cassette->input();

	LOG(("microtan_read_cassette: %g\n", level));
	if (level < -0.07)
		m_via6522_0->write_cb2(0);
	else if (level > +0.07)
		m_via6522_0->write_cb2(1);
}

READ8_MEMBER(microtan_state::microtan_sound_r)
{
	int data = 0xff;
	LOG(("microtan_sound_r: -> %02x\n", data));
	return data;
}

WRITE8_MEMBER(microtan_state::microtan_sound_w)
{
	LOG(("microtan_sound_w: <- %02x\n", data));
}


READ8_MEMBER(microtan_state::microtan_bffx_r)
{
	int data = 0xff;
	switch( offset & 3 )
	{
	case  0: /* BFF0: read enables chunky graphics */
		m_chunky_graphics = 1;
		LOG(("microtan_bff0_r: -> %02x (chunky graphics on)\n", data));
		break;
	case  1: /* BFF1: read undefined (?) */
		LOG(("microtan_bff1_r: -> %02x\n", data));
		break;
	case  2: /* BFF2: read undefined (?) */
		LOG(("microtan_bff2_r: -> %02x\n", data));
		break;
	default: /* BFF3: read keyboard ASCII value */
		data = m_keyboard_ascii;
		LOG(("microtan_bff3_r: -> %02x (keyboard ASCII)\n", data));
	}
	return data;
}


/* This callback is called one clock cycle after BFF2 is written (delayed nmi) */
TIMER_CALLBACK_MEMBER(microtan_state::microtan_pulse_nmi)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

WRITE8_MEMBER(microtan_state::microtan_bffx_w)
{
	switch( offset & 3 )
	{
	case 0: /* BFF0: write reset keyboard interrupt flag */
		/* This removes bit 7 from the ASCII value of the last key pressed. */
		LOG(("microtan_bff0_w: %d <- %02x (keyboard IRQ clear )\n", offset, data));
		m_keyboard_ascii &= ~0x80;
		m_kbd_irq_line = CLEAR_LINE;
		microtan_set_irq_line();
		break;
	case 1: /* BFF1: write delayed NMI */
		LOG(("microtan_bff1_w: %d <- %02x (delayed NMI)\n", offset, data));
		timer_set(m_maincpu->cycles_to_attotime(8), TIMER_PULSE_NMI);
		break;
	case 2: /* BFF2: write keypad column write (what is this meant for?) */
		LOG(("microtan_bff2_w: %d <- %02x (keypad column)\n", offset, data));
		m_keypad_column = data;
		break;
	default: /* BFF3: write disable chunky graphics */
		LOG(("microtan_bff3_w: %d <- %02x (chunky graphics off)\n", offset, data));
		m_chunky_graphics = 0;
	}
}

void microtan_state::store_key(int key)
{
	LOG(("microtan: store key '%c'\n", key));
	m_keyboard_ascii = key | 0x80;
	m_kbd_irq_line = ASSERT_LINE;
	microtan_set_irq_line();
}

INTERRUPT_GEN_MEMBER(microtan_state::microtan_interrupt)
{
	int mod, row, col, chg, newvar;
	static const char *const keynames[] = { "ROW0", "ROW1", "ROW2", "ROW3", "ROW4", "ROW5", "ROW6", "ROW7", "ROW8" };

	if( m_repeat )
	{
		if( !--m_repeat )
			m_repeater = 4;
	}
	else if( m_repeater )
	{
		m_repeat = m_repeater;
	}


	row = 9;
	newvar = ioport("ROW8")->read();
	chg = m_keyrows[--row] ^ newvar;

	while ( !chg && row > 0)
	{
		newvar = ioport(keynames[row - 1])->read();
		chg = m_keyrows[--row] ^ newvar;
	}
	if (!chg)
		--row;

	if (row >= 0)
	{
		m_repeater = 0x00;
		m_mask = 0x00;
		m_key = 0x00;
		m_lastrow = row;
		/* CapsLock LED */
		if( row == 3 && chg == 0x80 )
			output().set_led_value(1, (m_keyrows[3] & 0x80) ? 0 : 1);

		if (newvar & chg)  /* key(s) pressed ? */
		{
			mod = 0;

			/* Shift modifier */
			if ( (m_keyrows[5] & 0x10) || (m_keyrows[6] & 0x80) )
				mod |= 1;

			/* Control modifier */
			if (m_keyrows[3] & 0x40)
				mod |= 2;

			/* CapsLock modifier */
			if (m_keyrows[3] & 0x80)
				mod |= 4;

			/* find newvar key */
			m_mask = 0x01;
			for (col = 0; col < 8; col ++)
			{
				if (chg & m_mask)
				{
					newvar &= m_mask;
					m_key = keyboard[mod][row][col];
					break;
				}
				m_mask <<= 1;
			}
			if( m_key )   /* normal key */
			{
				m_repeater = 30;
				store_key(m_key);
			}
			else
			if( (row == 0) && (chg == 0x04) ) /* Ctrl-@ (NUL) */
				store_key(0);
			m_keyrows[row] |= newvar;
		}
		else
		{
			m_keyrows[row] = newvar;
		}
		m_repeat = m_repeater;
	}
	else
	if ( m_key && (m_keyrows[m_lastrow] & m_mask) && m_repeat == 0 )
	{
		store_key(m_key);
	}
}

DRIVER_INIT_MEMBER(microtan_state,microtan)
{
	UINT8 *dst = memregion("gfx2")->base();
	int i;
	address_space &space = m_maincpu->space(AS_PROGRAM);

	for (i = 0; i < 256; i++)
	{
		switch (i & 3)
		{
		case 0: dst[ 0] = dst[ 1] = dst[ 2] = dst[ 3] = 0x00; break;
		case 1: dst[ 0] = dst[ 1] = dst[ 2] = dst[ 3] = 0xf0; break;
		case 2: dst[ 0] = dst[ 1] = dst[ 2] = dst[ 3] = 0x0f; break;
		case 3: dst[ 0] = dst[ 1] = dst[ 2] = dst[ 3] = 0xff; break;
		}
		dst += 4;
		switch ((i >> 2) & 3)
		{
		case 0: dst[ 0] = dst[ 1] = dst[ 2] = dst[ 3] = 0x00; break;
		case 1: dst[ 0] = dst[ 1] = dst[ 2] = dst[ 3] = 0xf0; break;
		case 2: dst[ 0] = dst[ 1] = dst[ 2] = dst[ 3] = 0x0f; break;
		case 3: dst[ 0] = dst[ 1] = dst[ 2] = dst[ 3] = 0xff; break;
		}
		dst += 4;
		switch ((i >> 4) & 3)
		{
		case 0: dst[ 0] = dst[ 1] = dst[ 2] = dst[ 3] = 0x00; break;
		case 1: dst[ 0] = dst[ 1] = dst[ 2] = dst[ 3] = 0xf0; break;
		case 2: dst[ 0] = dst[ 1] = dst[ 2] = dst[ 3] = 0x0f; break;
		case 3: dst[ 0] = dst[ 1] = dst[ 2] = dst[ 3] = 0xff; break;
		}
		dst += 4;
		switch ((i >> 6) & 3)
		{
		case 0: dst[ 0] = dst[ 1] = dst[ 2] = dst[ 3] = 0x00; break;
		case 1: dst[ 0] = dst[ 1] = dst[ 2] = dst[ 3] = 0xf0; break;
		case 2: dst[ 0] = dst[ 1] = dst[ 2] = dst[ 3] = 0x0f; break;
		case 3: dst[ 0] = dst[ 1] = dst[ 2] = dst[ 3] = 0xff; break;
		}
		dst += 4;
	}

	switch (read_dsw() & 3)
	{
		case 0:  // 1K only :)
			space.nop_readwrite(0x0400, 0xbbff);
			break;
		case 1:  // +7K TANEX
			space.install_ram(0x0400, 0x1fff,nullptr);
			space.nop_readwrite(0x2000, 0xbbff);
			break;
		default: // +7K TANEX + 40K TANRAM
			space.install_ram(0x0400, 0xbbff, nullptr);
			break;
	}

	m_timer = timer_alloc(TIMER_READ_CASSETTE);

	m_via6522_0->write_ca1(1);
	m_via6522_0->write_ca2(1);

	m_via6522_0->write_pb0(1);
	m_via6522_0->write_pb1(1);
	m_via6522_0->write_pb2(1);
	m_via6522_0->write_pb3(1);
	m_via6522_0->write_pb4(1);
	m_via6522_0->write_pb5(1);
	m_via6522_0->write_pb6(1);
	m_via6522_0->write_pb7(1);
	m_via6522_0->write_cb1(1);
	m_via6522_0->write_cb2(1);

	m_via6522_1->write_pa0(1);
	m_via6522_1->write_pa1(1);
	m_via6522_1->write_pa2(1);
	m_via6522_1->write_pa3(1);
	m_via6522_1->write_pa4(1);
	m_via6522_1->write_pa5(1);
	m_via6522_1->write_pa6(1);
	m_via6522_1->write_pa7(1);
	m_via6522_1->write_ca1(1);
	m_via6522_1->write_ca2(1);

	m_via6522_1->write_pb0(1);
	m_via6522_1->write_pb1(1);
	m_via6522_1->write_pb2(1);
	m_via6522_1->write_pb3(1);
	m_via6522_1->write_pb4(1);
	m_via6522_1->write_pb5(1);
	m_via6522_1->write_pb6(1);
	m_via6522_1->write_pb7(1);
	m_via6522_1->write_cb1(1);
	m_via6522_1->write_cb2(1);
}

void microtan_state::machine_reset()
{
	int i;
	static const char *const keynames[] = { "ROW0", "ROW1", "ROW2", "ROW3", "ROW4", "ROW5", "ROW6", "ROW7", "ROW8" };

	for (i = 1; i < 10;  i++)
	{
		m_keyrows[i] = ioport(keynames[i-1])->read();
	}
	output().set_led_value(1, (m_keyrows[3] & 0x80) ? 0 : 1);
}

int microtan_state::microtan_verify_snapshot(UINT8 *data, int size)
{
	if (size == 8263)
	{
		logerror("microtan_snapshot_id: magic size %d found\n", size);
		return IMAGE_VERIFY_PASS;
	}
	else
	{
		if (4 + data[2] + 256 * data[3] + 1 + 16 + 16 + 16 + 1 + 1 + 16 + 16 + 64 + 7 == size)
		{
			logerror("microtan_snapshot_id: header RAM size + structures matches filesize %d\n", size);
			return IMAGE_VERIFY_PASS;
		}
	}

	return IMAGE_VERIFY_FAIL;
}

int microtan_state::parse_intel_hex(UINT8 *snapshot_buff, char *src)
{
	char line[128];
	int /*row = 0,*/ column = 0, last_addr = 0, last_size = 0;

	while (*src)
	{
		if (*src == '\r' || *src == '\n')
		{
			if (column)
			{
				unsigned int size, addr, null, b[32], cs, n;

				line[column] = '\0';
				/*row++;*/
				n = sscanf(line, ":%02x%04x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
					&size, &addr, &null,
					&b[ 0], &b[ 1], &b[ 2], &b[ 3], &b[ 4], &b[ 5], &b[ 6], &b[ 7],
					&b[ 8], &b[ 9], &b[10], &b[11], &b[12], &b[13], &b[14], &b[15],
					&b[16], &b[17], &b[18], &b[19], &b[20], &b[21], &b[22], &b[23],
					&b[24], &b[25], &b[26], &b[27], &b[28], &b[29], &b[30], &b[31],
					&cs);
				if (n == 0)
				{
					logerror("parse_intel_hex: malformed line [%s]\n", line);
				}
				else if (n == 1)
				{
					logerror("parse_intel_hex: only size found [%s]\n", line);
				}
				else if (n == 2)
				{
					logerror("parse_intel_hex: only size and addr found [%s]\n", line);
				}
				else if (n == 3)
				{
					logerror("parse_intel_hex: only size, addr and null found [%s]\n", line);
				}
				else
				if (null != 0)
				{
					logerror("parse_intel_hex: warning null byte is != 0 [%s]\n", line);
				}
				else
				{
					int i, sum;

					n -= 3;

					sum = size + (addr & 0xff) + ((addr >> 8) & 0xff);
					if (n != 32 + 1)
						cs = b[n-1];

					last_addr = addr;
					last_size = n-1;
					logerror("parse_intel_hex: %04X", addr);
					for (i = 0; i < n-1; i++)
					{
						sum += b[i];
						snapshot_buff[addr++] = b[i];
					}
					logerror("-%04X checksum %02X+%02X = %02X\n", addr-1, cs, sum & 0xff, (cs + sum) & 0xff);
				}
			}
			column = 0;
		}
		else
		{
			line[column++] = *src;
		}
		src++;
	}
	/* register preset? */
	if (last_size == 7)
	{
		logerror("parse_intel_hex: registers (?) at %04X\n", last_addr);
		memcpy(&snapshot_buff[8192+64], &snapshot_buff[last_addr], last_size);
	}
	return IMAGE_INIT_PASS;
}

int microtan_state::parse_zillion_hex(UINT8 *snapshot_buff, char *src)
{
	char line[128];
	int parsing = 0, /*row = 0,*/ column = 0;

	while (*src)
	{
		if (parsing)
		{
			if (*src == '}')
				parsing = 0;
			else
			{
				if (*src == '\r' || *src == '\n')
				{
					if (column)
					{
						unsigned int addr, b[8], n;

						line[column] = '\0';
						/*row++;*/
						n = sscanf(line, "%x %x %x %x %x %x %x %x %x", &addr, &b[0], &b[1], &b[2], &b[3], &b[4], &b[5], &b[6], &b[7]);
						if (n == 0)
						{
							logerror("parse_zillion_hex: malformed line [%s]\n", line);
						}
						else if (n == 1)
						{
							logerror("parse_zillion_hex: only addr found [%s]\n", line);
						}
						else
						{
							int i;

							logerror("parse_zillion_hex: %04X", addr);
							for (i = 0; i < n-1; i++)
								snapshot_buff[addr++] = b[i];
							logerror("-%04X\n", addr-1);
						}
					}
					column = 0;
				}
				else
				{
					line[column++] = *src;
				}
			}
		}
		else
		{
			if (*src == '\r' || *src == '\n')
			{
				if (column)
				{
					int addr, n;

					/*row++;*/
					line[column] = '\0';
					n = sscanf(line, "G%x", (unsigned int *) &addr);
					if (n == 1 && !snapshot_buff[8192+64+0] && !snapshot_buff[8192+64+1])
					{
						logerror("microtan_hexfile_init: go addr %04X\n", addr);
						snapshot_buff[8192+64+0] = addr & 0xff;
						snapshot_buff[8192+64+1] = (addr >> 8) & 0xff;
					}
				}
				column = 0;
			}
			else
			{
				line[column++] = *src;
			}
			if (*src == '{')
			{
				parsing = 1;
				column = 0;
			}
		}
		src++;
	}
	return IMAGE_INIT_PASS;
}

void microtan_state::microtan_set_cpu_regs(const UINT8 *snapshot_buff, int base)
{
	logerror("microtan_snapshot_copy: PC:%02X%02X P:%02X A:%02X X:%02X Y:%02X SP:1%02X",
		snapshot_buff[base+1], snapshot_buff[base+0], snapshot_buff[base+2], snapshot_buff[base+3],
		snapshot_buff[base+4], snapshot_buff[base+5], snapshot_buff[base+6]);
	m_maincpu->set_state_int(M6502_PC, snapshot_buff[base+0] + 256 * snapshot_buff[base+1]);
	m_maincpu->set_state_int(M6502_P, snapshot_buff[base+2]);
	m_maincpu->set_state_int(M6502_A, snapshot_buff[base+3]);
	m_maincpu->set_state_int(M6502_X, snapshot_buff[base+4]);
	m_maincpu->set_state_int(M6502_Y, snapshot_buff[base+5]);
	m_maincpu->set_state_int(M6502_S, snapshot_buff[base+6]);
}

void microtan_state::microtan_snapshot_copy(UINT8 *snapshot_buff, int snapshot_size)
{
	UINT8 *RAM = memregion("maincpu")->base();
	address_space &space = m_maincpu->space(AS_PROGRAM);
	ay8910_device *ay8910 = machine().device<ay8910_device>("ay8910.1");

	/* check for .DMP file format */
	if (snapshot_size == 8263)
	{
		int i, base;
		/********** DMP format
		 * Lower 8k of RAM (0000 to 1fff)
		 * 64 bytes of chunky graphics bits (first byte bit is for character at 0200, bit 1=0201, etc)
		 * 7 bytes of CPU registers (PCL, PCH, PSW, A, IX, IY, SP)
		 */
		logerror("microtan_snapshot_copy: magic size %d found, assuming *.DMP format\n", snapshot_size);

		base = 0;
		/* 8K of RAM from 0000 to 1fff */
		memcpy(RAM, &snapshot_buff[base], 8192);
		base += 8192;
		/* 64 bytes of chunky graphics info */
		for (i = 0; i < 32*16; i++)
		{
			m_chunky_buffer[i] = (snapshot_buff[base+i/8] >> (i&7)) & 1;
		}
		base += 64;
		microtan_set_cpu_regs(snapshot_buff, base);
	}
	else
	{
		int i, ramend, base;
		/********** M65 format ************************************
		 *  2 bytes: File version
		 *  2 bytes: RAM size
		 *  n bytes: RAM (0000 to RAM Size)
		 * 16 bytes: 1st 6522 (0xbfc0 to 0xbfcf)
		 * 16 bytes: 2ns 6522 (0xbfe0 to 0xbfef)
		 * 16 bytes: Microtan IO (0xbff0 to 0xbfff)
		 *  1 byte : Invaders sound (0xbc04)
		 *  1 byte : Chunky graphics state (0=off, 1=on)
		 * 16 bytes: 1st AY8910 registers
		 * 16 bytes: 2nd AY8910 registers
		 * 64 bytes: Chunky graphics bits (first byte bit 0 is for character at 0200, bit 1=0201, etc)
		 *  7 bytes: CPU registers (PCL, PCH, PSW, A, IX, IY, SP)
		 */
		ramend = snapshot_buff[2] + 256 * snapshot_buff[3];
		if (2 + 2 + ramend + 1 + 16 + 16 + 16 + 1 + 1 + 16 + 16 + 64 + 7 != snapshot_size)
		{
			logerror("microtan_snapshot_copy: size %d doesn't match RAM size %d + structure size\n", snapshot_size, ramend+1);
			return;
		}

		logerror("microtan_snapshot_copy: size %d found, assuming *.M65 format\n", snapshot_size);
		base = 4;
		memcpy(RAM, &snapshot_buff[base], snapshot_buff[2] + 256 * snapshot_buff[3] + 1);
		base += ramend + 1;

		/* first set of VIA6522 registers */
		for (i = 0; i < 16; i++ )
			m_via6522_0->write(space, i, snapshot_buff[base++]);

		/* second set of VIA6522 registers */
		for (i = 0; i < 16; i++ )
			m_via6522_1->write(space, i, snapshot_buff[base++]);

		/* microtan IO bff0-bfff */
		for (i = 0; i < 16; i++ )
		{
			RAM[0xbff0+i] = snapshot_buff[base++];
			if (i < 4)
				microtan_bffx_w(space, i, RAM[0xbff0+i]);
		}

		microtan_sound_w(space, 0, snapshot_buff[base++]);
		m_chunky_graphics = snapshot_buff[base++];

		/* first set of AY8910 registers */
		for (i = 0; i < 16; i++ )
		{
			ay8910->address_w(generic_space(), 0, i);
			ay8910->data_w(generic_space(), 0, snapshot_buff[base++]);
		}

		/* second set of AY8910 registers */
		for (i = 0; i < 16; i++ )
		{
			ay8910->address_w(generic_space(), 0, i);
			ay8910->data_w(generic_space(), 0, snapshot_buff[base++]);
		}

		for (i = 0; i < 32*16; i++)
		{
			m_chunky_buffer[i] = (snapshot_buff[base+i/8] >> (i&7)) & 1;
		}
		base += 64;

		microtan_set_cpu_regs(snapshot_buff, base);
	}
}

SNAPSHOT_LOAD_MEMBER( microtan_state, microtan )
{
	UINT8 *snapshot_buff;

	snapshot_buff = (UINT8*)image.ptr();
	if (!snapshot_buff)
		return IMAGE_INIT_FAIL;

	if (microtan_verify_snapshot(snapshot_buff, snapshot_size)==IMAGE_VERIFY_FAIL)
		return IMAGE_INIT_FAIL;

	microtan_snapshot_copy(snapshot_buff, snapshot_size);
	return IMAGE_INIT_PASS;
}

QUICKLOAD_LOAD_MEMBER( microtan_state, microtan )
{
	int snapshot_size = 8263;   /* magic size */
	dynamic_buffer snapshot_buff(snapshot_size, 0);
	std::vector<char> buff(quickload_size + 1);
	int rc;

	image.fread(&buff[0], quickload_size);

	buff[quickload_size] = '\0';

	if (buff[0] == ':')
		rc = parse_intel_hex(&snapshot_buff[0], &buff[0]);
	else
		rc = parse_zillion_hex(&snapshot_buff[0], &buff[0]);
	if (rc == IMAGE_INIT_PASS)
		microtan_snapshot_copy(&snapshot_buff[0], snapshot_size);
	return rc;
}
