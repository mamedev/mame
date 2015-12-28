// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Intel 82730

    Text Coprocessor

***************************************************************************/

#include "i82730.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define VERBOSE 1
#define VERBOSE_COMMANDS    1
#define VERBOSE_DATASTREAM  0


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type I82730 = &device_creator<i82730_device>;

const char *i82730_device::m_command_names[] =
{
	/* 00 */ "NOP",
	/* 01 */ "START DISPLAY",
	/* 02 */ "START VIRTUAL DISPLAY",
	/* 03 */ "STOP DISPLAY",
	/* 04 */ "MODE SET",
	/* 05 */ "LOAD CBP",
	/* 06 */ "LOAD INTMASK",
	/* 07 */ "LPEN ENABLE",
	/* 08 */ "READ STATUS",
	/* 09 */ "LD CUR POS",
	/* 0a */ "SELF TEST",
	/* 0b */ "TEST ROW BUFFER"
};


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  i82730_device - constructor
//-------------------------------------------------

i82730_device::i82730_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, I82730, "I82730", tag, owner, clock, "i82730", __FILE__),
	device_video_interface(mconfig, *this),
	m_sint_handler(*this),
	m_cpu_tag(nullptr), m_program(nullptr),
	m_row_timer(nullptr),
	m_initialized(false), m_mode_set(false),
	m_ca(0),
	m_sysbus(0x00), m_ibp(0x0000), m_cbp(0x0000), m_intmask(0xffff), m_status(0x0000),
	m_list_switch(0), m_auto_line_feed(0), m_max_dma_count(0),
	m_lptr(0), m_sptr(0),
	m_dma_burst_space(0), m_dma_burst_length(0),
	m_hfldstrt(0), m_margin(0), m_lpr(0), m_field_attribute_mask(0), m_vsyncstp(0), m_vfldstrt(0), m_vfldstp(0),
	m_frame_int_count(0),
	m_row_index(0)
{
}

//-------------------------------------------------
//  set_cpu_tag - set cpu we are attached to
//-------------------------------------------------

void i82730_device::set_cpu_tag(device_t &device, device_t *owner, const char *tag)
{
	i82730_device &dev = dynamic_cast<i82730_device &>(device);
	dev.m_cpu_tag = tag;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void i82730_device::device_start()
{
	// register bitmap
	m_screen->register_screen_bitmap(m_bitmap);

	// resolve callbacks
	m_sint_handler.resolve_safe();

	// bind delegates
	m_update_row_cb.bind_relative_to(*owner());

	// allocate row timer
	m_row_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(i82730_device::row_update), this));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void i82730_device::device_reset()
{
	cpu_device *cpu = m_owner->subdevice<cpu_device>(m_cpu_tag);
	m_program = &cpu->space(AS_PROGRAM);

	m_initialized = false;
	m_mode_set = false;

	m_ca = 0;
	m_status = 0x0000;
}


//**************************************************************************
//  MEMORY ACCESS
//**************************************************************************

UINT8 i82730_device::read_byte(offs_t address)
{
	return m_program->read_byte(address);
}

UINT16 i82730_device::read_word(offs_t address)
{
	UINT16 data;

	if (sysbus_16bit() && !(address & 1))
	{
		data = m_program->read_word(address);
	}
	else
	{
		data  = m_program->read_byte(address);
		data |= m_program->read_byte(address + 1) << 8;
	}

	return data;
}

void i82730_device::write_byte(offs_t address, UINT8 data)
{
	m_program->write_byte(address, data);
}

void i82730_device::write_word(offs_t address, UINT16 data)
{
	if (sysbus_16bit() && !(address & 1))
	{
		m_program->write_word(address, data);
	}
	else
	{
		m_program->write_byte(address, data & 0xff);
		m_program->write_byte(address + 1, (data >> 8) & 0xff);
	}
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void i82730_device::update_interrupts()
{
	UINT16 code = m_status & ~m_intmask & ~(VDIP | DIP);
	write_word(m_cbp + 20, code);

	if (code)
		m_sint_handler(1);
}

void i82730_device::mode_set()
{
	UINT32 mptr = (read_word(m_cbp + 32) << 16) | read_word(m_cbp + 30);
	UINT16 tmp;

	tmp = read_word(mptr);
	m_dma_burst_space = tmp & 0x7f;
	m_dma_burst_length = (tmp >> 8) & 0x7f;

	tmp = read_word(mptr + 2);
	UINT8 hsyncstp = tmp & 0xff;
	UINT8 line_length = (tmp >> 8) & 0xff;

	tmp = read_word(mptr + 4);
	UINT8 hfldstp = tmp & 0xff;
	m_hfldstrt = (tmp >> 8) & 0xff;

	tmp = read_word(mptr + 6);
	UINT8 hbrdstp = tmp & 0xff;
	UINT8 hbrdstrt = (tmp >> 8) & 0xff;

	tmp = read_word(mptr + 8);
	m_margin = tmp & 0x1f;

	tmp = read_word(mptr + 10);
	m_lpr = tmp & 0x1f;

	tmp = read_word(mptr + 24);
	m_field_attribute_mask = tmp & 0x7fff;

	tmp = read_word(mptr + 26);
	UINT16 frame_length = tmp & 0x7ff;

	tmp = read_word(mptr + 28);
	m_vsyncstp = tmp & 0x7ff;

	tmp = read_word(mptr + 30);
	m_vfldstrt = tmp & 0x7ff;

	tmp = read_word(mptr + 32);
	m_vfldstp = tmp & 0x7ff;

	tmp = read_word(mptr + 38);
	m_frame_int_count = tmp & 0x0f;

	// setup screen mode
	rectangle visarea(hbrdstrt * 16, hbrdstp * 16 - 1, m_vsyncstp, m_vfldstp + m_margin + 1 + m_lpr - 1);
	attoseconds_t period = HZ_TO_ATTOSECONDS(clock() * 16) * line_length * 16 * frame_length;
	m_screen->configure(line_length * 16, frame_length, visarea, period);

	// start display is now valid
	m_mode_set = true;

	// adjust timer for the new mode
	m_row_timer->adjust(m_screen->time_until_pos(0));

	// output some debug info
	if (VERBOSE)
	{
		logerror("%s('%s'): ---- setting mode ----\n", shortname(), basetag());
		logerror("%s('%s'): dma burst length %02x, space %02x\n", shortname(), basetag(), m_dma_burst_length, m_dma_burst_space);
		logerror("%s('%s'): margin %02x, lpr %02x\n", shortname(), basetag(), m_margin, m_lpr);
		logerror("%s('%s'): hsyncstp: %02x, line_length: %02x, hfldstrt: %02x, hbrdstart: %02x, hfldstop: %02x, hbrdstop: %02x\n",
			shortname(), basetag(), hsyncstp, line_length, m_hfldstrt, hbrdstrt, hfldstp, hbrdstp);
		logerror("%s('%s'): frame_length %04x, vsyncstp: %04x, vfldstrt: %04x, vfldstp: %04x\n",
			shortname(), basetag(), frame_length, m_vsyncstp, m_vfldstrt, m_vfldstp);
	}
}

void i82730_device::execute_command()
{
	UINT8 command = read_byte(m_cbp + 1);
	UINT16 tmp;

	if (VERBOSE_COMMANDS && command < ARRAY_LENGTH(m_command_names))
		logerror("%s('%s'): executing command: %s [cbp = %08x]\n", shortname(), basetag(), m_command_names[command], m_cbp);

	tmp = read_word(m_cbp + 2);
	m_list_switch = BIT(tmp, 6);
	m_auto_line_feed = BIT(tmp, 7);

	tmp = read_word(m_cbp + 4);
	m_max_dma_count = tmp & 0xff;

	switch (command)
	{
	// NOP
	case 0x00:
		break;

	// START DISPLAY
	case 0x01:
		if (m_mode_set)
			m_status = (m_status & ~VDIP) | DIP;
		break;

	// START VIRTUAL DISPLAY
	case 0x02:
		if (m_mode_set)
			m_status = VDIP | (m_status & ~DIP);
		break;

	// STOP DISPLAY
	case 0x03:
		m_status &= ~(VDIP | DIP);
		break;

	// MODE SET
	case 0x04:
		mode_set();
		break;

	// LOAD CBP
	case 0x05:
		m_cbp = (read_word(m_cbp + 16) << 16) | read_word(m_cbp + 14);
		execute_command();
		break;

	// LOAD INTMASK
	case 0x06:
		m_intmask = read_word(m_cbp + 22);
		if (VERBOSE_COMMANDS)
			logerror("%s('%s'): intmask now %04x\n", shortname(), basetag(), m_intmask);
		break;

	// LPEN ENABLE
	case 0x07:
		fatalerror("%s('%s'): Unimplemented command %s\n", shortname(), basetag(), m_command_names[command]);
		break;

	// READ STATUS
	case 0x08:
		write_word(m_cbp + 18, m_status);
		m_status &= (VDIP | DIP);
		break;

	// LD CUR POS
	case 0x09:
		fatalerror("%s('%s'): Unimplemented command %s\n", shortname(), basetag(), m_command_names[command]);
		break;

	// SELF TEST
	case 0x0a:
		fatalerror("%s('%s'): Unimplemented command %s\n", shortname(), basetag(), m_command_names[command]);
		break;

	// TEST ROW BUFFER
	case 0x0b:
		fatalerror("%s('%s'): Unimplemented command %s\n", shortname(), basetag(), m_command_names[command]);
		break;

	default:
		if (VERBOSE_COMMANDS)
			logerror("%s('%s'): executing command: (reserved) [cbp = %08x]\n", shortname(), basetag(), m_cbp);
		m_status |= RCC;
		update_interrupts();
		break;
	}

	// clear busy
	write_word(m_cbp, read_word(m_cbp) & 0xff00);
}

void i82730_device::load_row()
{
	bool finished = false;

	m_row[m_row_index].count = 0;

	while (!finished)
	{
		UINT16 data = read_word(m_sptr);
		m_sptr += 2;

		if (BIT(data, 15))
		{
			switch (data >> 8)
			{
			case 0x8e:
				m_field_attribute_mask = read_word(m_sptr) & 0x7fff;
				m_sptr += 2;

				if (VERBOSE_DATASTREAM)
					logerror("%s('%s'): SET FIELD ATTRIB to %04x\n", shortname(), basetag(), m_field_attribute_mask);

				break;

			default:
				fatalerror("%s('%s'): Unimplemented datastream command %02x\n", shortname(), basetag(), data >> 8);
			}
		}
		else
		{
			// maximum row size is 200
			if (m_row[m_row_index].count < m_max_dma_count && m_row[m_row_index].count < 200)
			{
				m_row[m_row_index].data[m_row[m_row_index].count++] = data;
			}
			else
			{
#if 0
				// move to next string?
				if (m_auto_line_feed == 0)
				{
					m_sptr = (read_word(m_lptr + 2) << 16) | read_word(m_lptr);
					m_lptr += 4;
				}
#endif
				finished = true;
			}
		}
	}

	m_sptr -= 2;
}

TIMER_CALLBACK_MEMBER( i82730_device::row_update )
{
	int y = m_screen->vpos();

	if (y == 0)
	{
		// clear interrupt status flags
		m_status &= (VDIP | DIP);

		// clear field attribute mask
		m_field_attribute_mask = 0;

		// get listbase
		if (m_list_switch)
			m_lptr = (read_word(m_cbp + 8) << 16) | read_word(m_cbp + 6);
		else
			m_lptr = (read_word(m_cbp + 12) << 16) | read_word(m_cbp + 10);

		m_sptr = (read_word(m_lptr + 2) << 16) | read_word(m_lptr);
		m_lptr += 4;

		// fetch initial row
		m_row_index = 0;
		load_row();
	}
	else if (y >= m_vsyncstp && y < m_vfldstrt)
	{
		// blank (top border)
	}
	else if (y >= m_vfldstrt && y < m_vfldstp)
	{
		UINT8 lc = (y - m_vfldstrt) % (m_lpr + 1);

		// call driver
		m_update_row_cb(m_bitmap, m_row[m_row_index].data, lc, y - m_vsyncstp, m_row[m_row_index].count);

		// swap buffers at end of row
		if (lc == m_lpr)
		{
			m_row_index ^= 1;
			load_row();
		}
	}
	else if (y >= m_vfldstp && y < m_vfldstp + m_margin + 1)
	{
		// margin
	}
	else if (y >= m_vfldstp + m_margin + 1 && y < m_vfldstp + m_margin + 1 + m_lpr + 1)
	{
		UINT8 lc = (y - (m_vfldstp + m_margin + 1)) % (m_lpr + 1);

		m_sptr = (read_word(m_cbp + 36) << 16) | read_word(m_cbp + 34);
		load_row();

		// call driver
		m_update_row_cb(m_bitmap, m_row[m_row_index].data, lc, y - m_vsyncstp, m_row[m_row_index].count);
	}
	else if (y == m_vfldstp + m_margin + 1 + m_lpr + 1)
	{
		// todo: check ca

		// frame interrupt?
		if ((m_screen->frame_number() % m_frame_int_count) == 0)
			m_status |= EONF;

		// check interrupts
		update_interrupts();
	}
	else
	{
		// vblank
	}

	m_row_timer->adjust(m_screen->time_until_pos((y + 1) % m_screen->height()));
}

WRITE_LINE_MEMBER( i82730_device::ca_w )
{
	if (VERBOSE)
		logerror("%s('%s'): ca_w %d\n", shortname(), basetag(), state);

	// falling edge
	if (m_ca == 1 && state == 0)
	{
		if (!m_initialized)
		{
			// get system bus width
			m_sysbus = m_program->read_byte(0xfffffff6);

			// get intermediate block pointer
			m_ibp = (read_word(0xfffffffe) << 16) | read_word(0xfffffffc);

			// get system configuration byte
			UINT8 scb = read_byte(m_ibp + 6);

			// clear busy
			write_word(m_ibp, read_word(m_ibp) & 0xff00);

			// done
			m_initialized = true;

			// output some debug info
			if (VERBOSE)
			{
				logerror("%s('%s'): ---- initializing ----\n", shortname(), basetag());
				logerror("%s('%s'): %s system bus\n", shortname(), basetag(), sysbus_16bit() ? "16-bit" : "8-bit");
				logerror("%s('%s'): intermediate block pointer: %08x\n", shortname(), basetag(), m_ibp);
				logerror("%s('%s'): addrbus: %s, clno: %d, clpos: %d, mode: %s, dtw16: %s, srdy: %s\n", shortname(), basetag(),
					BIT(scb, 0) ? "32-bit" : "16-bit", (scb >> 1) & 0x03, (scb >> 3) & 0x03,
					BIT(scb, 5) ? "master" : "slave", BIT(scb, 6) ? "16-bit" : "8-bit", BIT(scb, 7) ? "synchronous" : "asynchronous");
			}
		}

		// fetch command block pointer
		m_cbp = (read_word(m_ibp + 4) << 16) | read_word(m_ibp + 2);

		// and execute command
		execute_command();
	}

	m_ca = state;
}

WRITE_LINE_MEMBER( i82730_device::irst_w )
{
	if (VERBOSE)
		logerror("%s('%s'): irst_w %d\n", shortname(), basetag(), state);

	m_sint_handler(0);
}

UINT32 i82730_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, m_hfldstrt * 16, 0, cliprect);
	return 0;
}
