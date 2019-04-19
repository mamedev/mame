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
#include "machine/mos6551.h"

//#include "imagedev/snapquik.h"

//#define VERBOSE 1
#include "logmacro.h"


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

uint8_t microtan_state::read_dsw()
{
	switch(machine().phase())
	{
	case machine_phase::RESET:
	case machine_phase::RUNNING:
		return ioport("DSW")->read();

	default:
		return 0x00;
	}
}

/**************************************************************
 * VIA callback functions for VIA #0
 **************************************************************/
READ8_MEMBER(microtan_state::via_0_in_a)
{
	int data = ioport("JOY")->read();
	LOG("via_0_in_a %02X\n", data);
	return data;
}

WRITE8_MEMBER(microtan_state::via_0_out_a)
{
	LOG("via_0_out_a %02X\n", data);
}

WRITE8_MEMBER(microtan_state::via_0_out_b)
{
	LOG("via_0_out_b %02X\n", data);
	/* bit #7 is the cassette output signal */
	m_cassette->output(data & 0x80 ? +1.0 : -1.0);
}

WRITE_LINE_MEMBER(microtan_state::via_0_out_ca2)
{
	LOG("via_0_out_ca2 %d\n", state);
}

WRITE_LINE_MEMBER(microtan_state::via_0_out_cb2)
{
	LOG("via_0_out_cb2 %d\n", state);
}

/**************************************************************
 * VIA callback functions for VIA #1
 **************************************************************/

WRITE8_MEMBER(microtan_state::via_1_out_a)
{
	LOG("via_1_out_a %02X\n", data);
}

WRITE8_MEMBER(microtan_state::via_1_out_b)
{
	LOG("via_1_out_b %02X\n", data);
}

WRITE_LINE_MEMBER(microtan_state::via_1_out_ca2)
{
	LOG("via_1_out_ca2 %d\n", state);
}

WRITE_LINE_MEMBER(microtan_state::via_1_out_cb2)
{
	LOG("via_1_out_cb2 %d\n", state);
}

void microtan_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_READ_CASSETTE:
		read_cassette(ptr, param);
		break;
	case TIMER_PULSE_NMI:
		pulse_nmi(ptr, param);
		break;
	default:
		assert_always(false, "Unknown id in microtan_state::device_timer");
	}
}


TIMER_CALLBACK_MEMBER(microtan_state::read_cassette)
{
	double level = m_cassette->input();

	LOG("read_cassette: %g\n", level);
	if (level < -0.07)
		m_via6522[0]->write_cb2(0);
	else if (level > +0.07)
		m_via6522[0]->write_cb2(1);
}

READ8_MEMBER(microtan_state::sound_r)
{
	int data = 0xff;
	LOG("sound_r: -> %02x\n", data);
	return data;
}

WRITE8_MEMBER(microtan_state::sound_w)
{
	LOG("sound_w: <- %02x\n", data);
}


READ8_MEMBER(microtan_state::bffx_r)
{
	int data = 0xff;
	switch( offset & 3 )
	{
	case  0: /* BFF0: read enables chunky graphics */
		m_chunky_graphics = 1;
		LOG("bff0_r: -> %02x (chunky graphics on)\n", data);
		break;
	case  1: /* BFF1: read undefined (?) */
		LOG("bff1_r: -> %02x\n", data);
		break;
	case  2: /* BFF2: read undefined (?) */
		LOG("bff2_r: -> %02x\n", data);
		break;
	default: /* BFF3: read keyboard ASCII value */
		data = m_keyboard_ascii;
		LOG("bff3_r: -> %02x (keyboard ASCII)\n", data);
	}
	return data;
}


/* This callback is called one clock cycle after BFF2 is written (delayed nmi) */
TIMER_CALLBACK_MEMBER(microtan_state::pulse_nmi)
{
	m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

WRITE8_MEMBER(microtan_state::bffx_w)
{
	switch( offset & 3 )
	{
	case 0: /* BFF0: write reset keyboard interrupt flag */
		/* This removes bit 7 from the ASCII value of the last key pressed. */
		LOG("bff0_w: %d <- %02x (keyboard IRQ clear )\n", offset, data);
		m_keyboard_ascii &= ~0x80;
		m_irq_line->in_w<IRQ_KBD>(0);
		break;
	case 1: /* BFF1: write delayed NMI */
		LOG("bff1_w: %d <- %02x (delayed NMI)\n", offset, data);
		m_pulse_nmi_timer->adjust(m_maincpu->cycles_to_attotime(8));
		break;
	case 2: /* BFF2: write keypad column write (what is this meant for?) */
		LOG("bff2_w: %d <- %02x (keypad column)\n", offset, data);
		m_keypad_column = data;
		break;
	default: /* BFF3: write disable chunky graphics */
		LOG("bff3_w: %d <- %02x (chunky graphics off)\n", offset, data);
		m_chunky_graphics = 0;
	}
}

void microtan_state::store_key(int key)
{
	LOG("microtan: store key '%c'\n", key);
	m_keyboard_ascii = key | 0x80;
	m_irq_line->in_w<IRQ_KBD>(1);
}

INTERRUPT_GEN_MEMBER(microtan_state::interrupt)
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
			m_led = BIT(~m_keyrows[3], 7);

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

void microtan_state::init_microtan()
{
	uint8_t *dst = memregion("gfx2")->base();
	address_space &space = m_maincpu->space(AS_PROGRAM);

	for (int i = 0; i < 256; i++)
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

	m_read_cassette_timer = timer_alloc(TIMER_READ_CASSETTE);
	m_pulse_nmi_timer = timer_alloc(TIMER_PULSE_NMI);

	m_via6522[0]->write_ca1(1);
	m_via6522[0]->write_ca2(1);

	m_via6522[0]->write_pb(0xff);
	m_via6522[0]->write_cb1(1);
	m_via6522[0]->write_cb2(1);

	m_via6522[1]->write_pa(0xff);
	m_via6522[1]->write_ca1(1);
	m_via6522[1]->write_ca2(1);

	m_via6522[1]->write_pb(0xff);
	m_via6522[1]->write_cb1(1);
	m_via6522[1]->write_cb2(1);
}

void microtan_state::machine_start()
{
	m_led.resolve();

	save_item(NAME(m_keypad_column));
	save_item(NAME(m_keyboard_ascii));
	save_item(NAME(m_keyrows));
	save_item(NAME(m_lastrow));
	save_item(NAME(m_mask));
	save_item(NAME(m_key));
	save_item(NAME(m_repeat));
	save_item(NAME(m_repeater));
}

void microtan_state::machine_reset()
{
	static const char *const keynames[] = { "ROW0", "ROW1", "ROW2", "ROW3", "ROW4", "ROW5", "ROW6", "ROW7", "ROW8" };

	for (int i = 1; i < 10;  i++)
	{
		m_keyrows[i] = ioport(keynames[i-1])->read();
	}
	m_led = BIT(~m_keyrows[3], 7);
}

image_verify_result microtan_state::verify_snapshot(uint8_t *data, int size)
{
	if (size == 8263)
	{
		logerror("snapshot_id: magic size %d found\n", size);
		return image_verify_result::PASS;
	}
	else
	{
		if (4 + data[2] + 256 * data[3] + 1 + 16 + 16 + 16 + 1 + 1 + 16 + 16 + 64 + 7 == size)
		{
			logerror("snapshot_id: header RAM size + structures matches filesize %d\n", size);
			return image_verify_result::PASS;
		}
	}

	return image_verify_result::FAIL;
}

image_init_result microtan_state::parse_intel_hex(uint8_t *snapshot_buff, char *src)
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
	return image_init_result::PASS;
}

image_init_result microtan_state::parse_zillion_hex(uint8_t *snapshot_buff, char *src)
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
	return image_init_result::PASS;
}

void microtan_state::set_cpu_regs(const uint8_t *snapshot_buff, int base)
{
	logerror("snapshot_copy: PC:%02X%02X P:%02X A:%02X X:%02X Y:%02X SP:1%02X",
		snapshot_buff[base+1], snapshot_buff[base+0], snapshot_buff[base+2], snapshot_buff[base+3],
		snapshot_buff[base+4], snapshot_buff[base+5], snapshot_buff[base+6]);
	m_maincpu->set_state_int(M6502_PC, snapshot_buff[base+0] + 256 * snapshot_buff[base+1]);
	m_maincpu->set_state_int(M6502_P, snapshot_buff[base+2]);
	m_maincpu->set_state_int(M6502_A, snapshot_buff[base+3]);
	m_maincpu->set_state_int(M6502_X, snapshot_buff[base+4]);
	m_maincpu->set_state_int(M6502_Y, snapshot_buff[base+5]);
	m_maincpu->set_state_int(M6502_S, snapshot_buff[base+6]);
}

void microtan_state::snapshot_copy(uint8_t *snapshot_buff, int snapshot_size)
{
	uint8_t *RAM = memregion("maincpu")->base();
	address_space &space = m_maincpu->space(AS_PROGRAM);

	/* check for .DMP file format */
	if (snapshot_size == 8263)
	{
		/********** DMP format
		 * Lower 8k of RAM (0000 to 1fff)
		 * 64 bytes of chunky graphics bits (first byte bit is for character at 0200, bit 1=0201, etc)
		 * 7 bytes of CPU registers (PCL, PCH, PSW, A, IX, IY, SP)
		 */
		logerror("snapshot_copy: magic size %d found, assuming *.DMP format\n", snapshot_size);

		int base = 0;
		/* 8K of RAM from 0000 to 1fff */
		memcpy(RAM, &snapshot_buff[base], 8192);
		base += 8192;
		/* 64 bytes of chunky graphics info */
		for (int i = 0; i < 32*16; i++)
		{
			m_chunky_buffer[i] = (snapshot_buff[base+i/8] >> (i&7)) & 1;
		}
		base += 64;
		set_cpu_regs(snapshot_buff, base);
	}
	else
	{
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
		int ramend = snapshot_buff[2] + 256 * snapshot_buff[3];
		if (2 + 2 + ramend + 1 + 16 + 16 + 16 + 1 + 1 + 16 + 16 + 64 + 7 != snapshot_size)
		{
			logerror("snapshot_copy: size %d doesn't match RAM size %d + structure size\n", snapshot_size, ramend+1);
			return;
		}

		logerror("snapshot_copy: size %d found, assuming *.M65 format\n", snapshot_size);
		int base = 4;
		memcpy(RAM, &snapshot_buff[base], snapshot_buff[2] + 256 * snapshot_buff[3] + 1);
		base += ramend + 1;

		/* first set of VIA6522 registers */
		for (int i = 0; i < 16; i++ )
			m_via6522[0]->write(i, snapshot_buff[base++]);

		/* second set of VIA6522 registers */
		for (int i = 0; i < 16; i++ )
			m_via6522[1]->write(i, snapshot_buff[base++]);

		/* microtan IO bff0-bfff */
		for (int i = 0; i < 16; i++ )
		{
			RAM[0xbff0+i] = snapshot_buff[base++];
			if (i < 4)
				bffx_w(space, i, RAM[0xbff0+i]);
		}

		sound_w(space, 0, snapshot_buff[base++]);
		m_chunky_graphics = snapshot_buff[base++];

		/* first set of AY8910 registers */
		for (int i = 0; i < 16; i++ )
		{
			m_ay8910[0]->address_w(i);
			m_ay8910[0]->data_w(snapshot_buff[base++]);
		}

		/* second set of AY8910 registers */
		for (int i = 0; i < 16; i++ )
		{
			m_ay8910[0]->address_w(i);
			m_ay8910[0]->data_w(snapshot_buff[base++]);
		}

		for (int i = 0; i < 32*16; i++)
		{
			m_chunky_buffer[i] = (snapshot_buff[base+i/8] >> (i&7)) & 1;
		}
		base += 64;

		set_cpu_regs(snapshot_buff, base);
	}
}

SNAPSHOT_LOAD_MEMBER( microtan_state, microtan )
{
	uint8_t *snapshot_buff = (uint8_t*)image.ptr();
	if (!snapshot_buff)
		return image_init_result::FAIL;

	if (verify_snapshot(snapshot_buff, snapshot_size) != image_verify_result::PASS)
		return image_init_result::FAIL;

	snapshot_copy(snapshot_buff, snapshot_size);
	return image_init_result::PASS;
}

QUICKLOAD_LOAD_MEMBER( microtan_state, microtan )
{
	int snapshot_size = 8263;   /* magic size */
	std::vector<uint8_t> snapshot_buff(snapshot_size, 0);
	std::vector<char> buff(quickload_size + 1);
	image_init_result rc;

	image.fread(&buff[0], quickload_size);

	buff[quickload_size] = '\0';

	if (buff[0] == ':')
		rc = parse_intel_hex(&snapshot_buff[0], &buff[0]);
	else
		rc = parse_zillion_hex(&snapshot_buff[0], &buff[0]);
	if (rc == image_init_result::PASS)
		snapshot_copy(&snapshot_buff[0], snapshot_size);
	return rc;
}
