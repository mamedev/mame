// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller
/******************************************************************************
 *  Microtan 65
 *
 *  Thanks go to Geoff Macdonald <mail@geoff.org.uk>
 *  for his site http://www.geoff.org.uk/microtan/index.htm
 *  and to Fabrice Frances <frances@ensica.fr>
 *  for his site http://oric.free.fr/microtan.html
 *
 *****************************************************************************/

#include "emu.h"
#include "microtan.h"

//#define VERBOSE 1
#include "logmacro.h"


uint8_t microtan_state::bffx_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (machine().side_effects_disabled()) return data;

	switch (offset & 3)
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
	case 3: /* BFF3: read keyboard/keypad */
		data = m_keyboard->read() | (m_keyboard_int_flag << 7);
		LOG("bff3_r: -> %02x (keyboard ASCII)\n", data);
		break;
	}
	return data;
}


/* This callback is called one clock cycle after BFF2 is written (delayed nmi) */
TIMER_CALLBACK_MEMBER(microtan_state::pulse_nmi)
{
	m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void microtan_state::bffx_w(offs_t offset, uint8_t data)
{
	switch (offset & 3)
	{
	case 0: /* BFF0: write reset keyboard interrupt flag */
		/* This removes bit 7 from the ASCII value of the last key pressed. */
		LOG("bff0_w: %d <- %02x (keyboard IRQ clear )\n", offset, data);
		kbd_int(0);
		break;
	case 1: /* BFF1: write delayed NMI */
		LOG("bff1_w: %d <- %02x (delayed NMI)\n", offset, data);
		m_pulse_nmi_timer->adjust(m_maincpu->cycles_to_attotime(8));
		break;
	case 2: /* BFF2: write keypad */
		LOG("bff2_w: %d <- %02x (keypad column)\n", offset, data);
		m_keyboard->write(data);
		break;
	case 3: /* BFF3: write disable chunky graphics */
		LOG("bff3_w: %d <- %02x (chunky graphics off)\n", offset, data);
		m_chunky_graphics = 0;
		break;
	}
}


uint8_t mt6809_state::bffx_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (machine().side_effects_disabled()) return data;

	switch (offset & 3)
	{
	case 3: /* BFF3: read keyboard */
		data = m_keyboard->read();
		LOG("bff3_r: -> %02x (keyboard ASCII)\n", data);
		break;
	}
	return data;
}

void mt6809_state::bffx_w(offs_t offset, uint8_t data)
{
	switch (offset & 3)
	{
	case 0: /* BFF0: write reset keyboard interrupt flag */
		LOG("bff0_w: %d <- %02x (keyboard NMI clear )\n", offset, data);
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
		break;
	}
}


void microtan_state::kbd_int(int state)
{
	m_keyboard_int_flag = state;

	m_irq_line->in_w<IRQ_KBD>(state);
}


void microtan_state::pgm_chargen_w(offs_t offset, uint8_t data)
{
	switch (offset & 0x200)
	{
	case 0x000:
		/* update char &80-&1F */
		m_gfx1->base()[offset | 0x800] = data;
		m_gfxdecode->gfx(0)->mark_dirty(0x80 | (offset >> 4));
		break;
	case 0x200:
		/* update char &E0-&FF */
		m_gfx1->base()[offset | 0xc00] = data;
		m_gfxdecode->gfx(0)->mark_dirty(0xc0 | (offset >> 4));
		break;
	}
}


void microtan_state::machine_start()
{
	uint8_t *dst = memregion("gfx2")->base();

	for (int i = 0; i < 256; i++)
	{
		switch (i & 3)
		{
		case 0: dst[0] = dst[1] = dst[2] = dst[3] = 0x00; break;
		case 1: dst[0] = dst[1] = dst[2] = dst[3] = 0xf0; break;
		case 2: dst[0] = dst[1] = dst[2] = dst[3] = 0x0f; break;
		case 3: dst[0] = dst[1] = dst[2] = dst[3] = 0xff; break;
		}
		dst += 4;
		switch (BIT(i, 2, 2))
		{
		case 0: dst[0] = dst[1] = dst[2] = dst[3] = 0x00; break;
		case 1: dst[0] = dst[1] = dst[2] = dst[3] = 0xf0; break;
		case 2: dst[0] = dst[1] = dst[2] = dst[3] = 0x0f; break;
		case 3: dst[0] = dst[1] = dst[2] = dst[3] = 0xff; break;
		}
		dst += 4;
		switch (BIT(i, 4, 2))
		{
		case 0: dst[0] = dst[1] = dst[2] = dst[3] = 0x00; break;
		case 1: dst[0] = dst[1] = dst[2] = dst[3] = 0xf0; break;
		case 2: dst[0] = dst[1] = dst[2] = dst[3] = 0x0f; break;
		case 3: dst[0] = dst[1] = dst[2] = dst[3] = 0xff; break;
		}
		dst += 4;
		switch (BIT(i, 6, 2))
		{
		case 0: dst[0] = dst[1] = dst[2] = dst[3] = 0x00; break;
		case 1: dst[0] = dst[1] = dst[2] = dst[3] = 0xf0; break;
		case 2: dst[0] = dst[1] = dst[2] = dst[3] = 0x0f; break;
		case 3: dst[0] = dst[1] = dst[2] = dst[3] = 0xff; break;
		}
		dst += 4;
	}

	m_pulse_nmi_timer = timer_alloc(FUNC(microtan_state::pulse_nmi), this);
}

void mt6809_state::machine_start()
{
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
					logerror("parse_intel_hex: malformed line [%s]\n", line);
				else
				if (n == 1)
					logerror("parse_intel_hex: only size found [%s]\n", line);
				else
				if (n == 2)
					logerror("parse_intel_hex: only size and addr found [%s]\n", line);
				else
				if (n == 3)
					logerror("parse_intel_hex: only size, addr and null found [%s]\n", line);
				else
				if (null != 0)
					logerror("parse_intel_hex: warning null byte is != 0 [%s]\n", line);
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
			line[column++] = *src;

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
							logerror("parse_zillion_hex: malformed line [%s]\n", line);
						else
						if (n == 1)
							logerror("parse_zillion_hex: only addr found [%s]\n", line);
						else
						{
							logerror("parse_zillion_hex: %04X", addr);
							for (int i = 0; i < n-1; i++)
								snapshot_buff[addr++] = b[i];
							logerror("-%04X\n", addr-1);
						}
					}
					column = 0;
				}
				else
					line[column++] = *src;
			}
		}
		else
		{
			if (*src == '\r' || *src == '\n')
			{
				if (column)
				{
					int addr;

					/*row++;*/
					line[column] = '\0';
					int n = sscanf(line, "G%x", (unsigned int *) &addr);
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
				line[column++] = *src;

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
	logerror("snapshot_copy: PC:%02X%02X P:%02X A:%02X X:%02X Y:%02X SP:1%02X\n",
		snapshot_buff[base+1], snapshot_buff[base+0], snapshot_buff[base+2], snapshot_buff[base+3],
		snapshot_buff[base+4], snapshot_buff[base+5], snapshot_buff[base+6]);
	m_maincpu->set_state_int(M6502_PC, snapshot_buff[base+0] + (snapshot_buff[base+1] << 8));
	m_maincpu->set_state_int(M6502_P, snapshot_buff[base+2]);
	m_maincpu->set_state_int(M6502_A, snapshot_buff[base+3]);
	m_maincpu->set_state_int(M6502_X, snapshot_buff[base+4]);
	m_maincpu->set_state_int(M6502_Y, snapshot_buff[base+5]);
	m_maincpu->set_state_int(M6502_S, snapshot_buff[base+6] + 0x100);
}

void microtan_state::snapshot_copy(uint8_t *snapshot_buff, int snapshot_size)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	/* check for .DMP file format */
	if (snapshot_size == 8263)
	{
		/********** DMP format
		 * Lower 8k of RAM (0000 to 1fff)
		 * 64 bytes of chunky graphics bits (first byte bit is for character at 0200, bit 1=0201, etc)
		 * 7 bytes of CPU registers (PCL, PCH, PSW, A, IX, IY, SP)
		 */

		int base = 0;
		/* 8K of RAM from 0000 to 1fff */
		for (int i = 0; i < 0x2000; i++)
			space.write_byte(i, snapshot_buff[base + i]);

		base += 8192;
		/* 64 bytes of chunky graphics info */
		for (int i = 0; i < 32*16; i++)
			m_chunky_buffer[i] = (snapshot_buff[base+i/8] >> (i&7)) & 1;

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
		 * 16 bytes: 2nd 6522 (0xbfe0 to 0xbfef)
		 * 16 bytes: Microtan IO (0xbff0 to 0xbfff)
		 *  1 byte : Space Invasion sound (0xbc04)
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

		int base = 4;
		for (int i = 0; i < snapshot_buff[2] + 256 * snapshot_buff[3] + 1; i++)
			space.write_byte(i, snapshot_buff[base + i]);
		base += ramend + 1;

		/* first set of VIA6522 registers */
		for (int i = 0; i < 16; i++ )
			space.write_byte(0xbfc0 + i, snapshot_buff[base++]);

		/* second set of VIA6522 registers */
		for (int i = 0; i < 16; i++ )
			space.write_byte(0xbfe0 + i, snapshot_buff[base++]);

		/* microtan IO bff0-bfff */
		for (int i = 0; i < 16; i++ )
		{
			if (i < 4)
				bffx_w(i, snapshot_buff[base++]);
		}

		space.write_byte(0xbc04, snapshot_buff[base++]);
		m_chunky_graphics = snapshot_buff[base++];

		/* first set of AY8910 registers */
		for (int i = 0; i < 16; i++ )
		{
			space.write_byte(0xbc00, i);
			space.write_byte(0xbc01, snapshot_buff[base++]);
		}

		/* second set of AY8910 registers */
		for (int i = 0; i < 16; i++ )
		{
			space.write_byte(0xbc02, i);
			space.write_byte(0xbc03, snapshot_buff[base++]);
		}

		for (int i = 0; i < 32*16; i++)
			m_chunky_buffer[i] = (snapshot_buff[base+i/8] >> (i&7)) & 1;

		base += 64;

		set_cpu_regs(snapshot_buff, base);
	}
}

SNAPSHOT_LOAD_MEMBER(microtan_state::snapshot_cb)
{
	uint64_t snapshot_len = image.length();
	if (snapshot_len < 4 || snapshot_len >= 66000)
	{
		//image.seterror(image_error::INVALIDIMAGE);
		return image_init_result::FAIL;
	}

	auto snapshot_buff = std::make_unique<uint8_t []>(snapshot_len);
	if (image.fread(snapshot_buff.get(), snapshot_len) != snapshot_len)
	{
		//image.seterror(image_error::UNSPECIFIED);
		return image_init_result::FAIL;
	}

	if (verify_snapshot(snapshot_buff.get(), snapshot_len) != image_verify_result::PASS)
	{
		//image.seterror(image_error::INVALIDIMAGE);
		return image_init_result::FAIL;
	}

	snapshot_copy(snapshot_buff.get(), snapshot_len);
	return image_init_result::PASS;
}

QUICKLOAD_LOAD_MEMBER(microtan_state::quickload_cb)
{
	int snapshot_size = 8263;   /* magic size */
	std::vector<uint8_t> snapshot_buff(snapshot_size, 0);
	std::vector<char> buff(image.length() + 1);
	image_init_result rc;

	image.fread(&buff[0], image.length());

	buff[image.length()] = '\0';

	if (buff[0] == ':')
		rc = parse_intel_hex(&snapshot_buff[0], &buff[0]);
	else
		rc = parse_zillion_hex(&snapshot_buff[0], &buff[0]);
	if (rc == image_init_result::PASS)
		snapshot_copy(&snapshot_buff[0], snapshot_size);
	return rc;
}
