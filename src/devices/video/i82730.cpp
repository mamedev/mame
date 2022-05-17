// license:BSD-3-Clause
// copyright-holders:Dirk Best
/***************************************************************************

    Intel 82730

    Text Coprocessor

***************************************************************************/

#include "emu.h"
#include "i82730.h"

#include "screen.h"

#define LOG_GENERAL (1U << 0)
#define LOG_COMMANDS (1U << 1)
#define LOG_DATASTREAM (1U << 2)
//#define VERBOSE (LOG_GENERAL | LOG_COMMANDS | LOG_DATASTREAM)
#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(I82730, i82730_device, "i82730", "Intel 82730")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  i82730_device - constructor
//-------------------------------------------------

i82730_device::i82730_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, I82730, tag, owner, clock),
	device_video_interface(mconfig, *this),
	m_sint_handler(*this),
	m_update_row_cb(*this),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_program(nullptr),
	m_row_timer(nullptr),
	m_initialized(false),
	m_mode_set(false),
	m_ca(0),
	m_ca_latch(false),
	m_sysbus(0x00),
	m_ibp(0x0000),
	m_cbp(0x0000),
	m_list_switch(false),
	m_auto_line_feed(false),
	m_max_dma_count(0),
	m_lptr(0),
	m_status(0x0000),
	m_intmask(0xffff),
	m_sptr(0),
	m_row(nullptr),
	m_dma_count(0),
	m_row_count(0),
	m_row_index(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void i82730_device::device_start()
{
	// register bitmap
	screen().register_screen_bitmap(m_bitmap);

	// resolve callbacks
	m_sint_handler.resolve_safe();

	// bind delegates
	m_update_row_cb.resolve();

	// allocate row timer
	m_row_timer = timer_alloc(FUNC(i82730_device::row_update), this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void i82730_device::device_reset()
{
	m_program = &m_cpu->space(AS_PROGRAM);

	m_initialized = false;
	m_mode_set = false;

	m_ca = 0;
	m_ca_latch = false;
	m_status = 0x0000;
}


//**************************************************************************
//  MEMORY ACCESS
//**************************************************************************

uint8_t i82730_device::read_byte(offs_t address)
{
	return m_program->read_byte(address);
}

uint16_t i82730_device::read_word(offs_t address)
{
	uint16_t data;

	if (sysbus_16bit() && WORD_ALIGNED(address))
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

void i82730_device::write_byte(offs_t address, uint8_t data)
{
	m_program->write_byte(address, data);
}

void i82730_device::write_word(offs_t address, uint16_t data)
{
	if (sysbus_16bit() && WORD_ALIGNED(address))
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
	uint16_t code = m_status & ~m_intmask & ~(VDIP | DIP);
	write_word(m_cbp + 20, code);

	if (code)
		m_sint_handler(1);
}

void i82730_device::mode_set()
{
	uint32_t mptr = (read_word(m_cbp + 32) << 16) | read_word(m_cbp + 30);
	uint16_t tmp;

	tmp = read_word(mptr);
	m_mb.burst_length = (tmp >> 8) & 0x7f;
	m_mb.burst_space = tmp & 0x7f;

	tmp = read_word(mptr + 2);
	m_mb.line_length = (tmp >> 8) & 0xff;
	m_mb.hsyncstp = tmp & 0xff;

	tmp = read_word(mptr + 4);
	m_mb.hfldstrt = (tmp >> 8) & 0xff;
	m_mb.hfldstp = tmp & 0xff;

	tmp = read_word(mptr + 6);
	m_mb.hbrdstrt = (tmp >> 8) & 0xff;
	m_mb.hbrdstp = tmp & 0xff;

	tmp = read_word(mptr + 8);
	m_mb.scroll_margin = tmp & 0x1f;

	tmp = read_word(mptr + 10);
	m_mb.rvv_row = bool(BIT(tmp, 11));
	m_mb.blk_row = bool(BIT(tmp, 10));
	m_mb.dbl_hgt = bool(BIT(tmp, 9));
	m_mb.wdef = bool(BIT(tmp, 8));
	m_mb.lpr = tmp & 0x1f;

	tmp = read_word(mptr + 12);
	m_mb.nrmstrt = (tmp >> 8) & 0x1f;
	m_mb.nrmstp = tmp & 0x1f;

	tmp = read_word(mptr + 14);
	m_mb.supstrt = (tmp >> 8) & 0x1f;
	m_mb.supstp = tmp & 0x1f;

	tmp = read_word(mptr + 16);
	m_mb.substrt = (tmp >> 8) & 0x1f;
	m_mb.substp = tmp & 0x1f;

	tmp = read_word(mptr + 18);
	m_mb.cur1strt = (tmp >> 8) & 0x1f;
	m_mb.cur1stp = tmp & 0x1f;

	tmp = read_word(mptr + 20);
	m_mb.cur2strt = (tmp >> 8) & 0x1f;
	m_mb.cur2stp = tmp & 0x1f;

	tmp = read_word(mptr + 22);
	m_mb.u2_line_sel = (tmp >> 8) & 0x1f;
	m_mb.u1_line_sel = tmp & 0x1f;

	tmp = read_word(mptr + 24);
	m_mb.field_attribute_mask = tmp & 0x7fff;

	tmp = read_word(mptr + 26);
	m_mb.frame_length = tmp & 0x7ff;

	tmp = read_word(mptr + 28);
	m_mb.vsyncstp = tmp & 0x7ff;

	tmp = read_word(mptr + 30);
	m_mb.vfldstrt = tmp & 0x7ff;

	tmp = read_word(mptr + 32);
	m_mb.vfldstp = tmp & 0x7ff;

	tmp = read_word(mptr + 38);
	m_mb.duty_cyc_cursor = (tmp >> 12) & 0x0f;
	m_mb.cursor_blink = (tmp >> 8) & 0x0f;
	m_mb.frame_int_count = tmp & 0x0f;

	tmp = read_word(mptr + 40);
	m_mb.duty_cyc_char = (tmp >> 12) & 0x0f;
	m_mb.char_blink = (tmp >> 8) & 0x0f;
	m_mb.ile = bool(BIT(tmp, 7));
	m_mb.rfe = bool(BIT(tmp, 6));
	m_mb.bpol = bool(BIT(tmp, 5));
	m_mb.bue = bool(BIT(tmp, 4));
	m_mb.cr2_cd = bool(BIT(tmp, 3));
	m_mb.cr1_cd = bool(BIT(tmp, 2));
	m_mb.cr2_be = bool(BIT(tmp, 1));
	m_mb.cr1_be = bool(BIT(tmp, 0));

	tmp = read_word(mptr + 40);
	m_mb.reverse_video = (tmp >> 12) & 0x0f;
	m_mb.blinking_char = (tmp >> 8) & 0x0f;
	m_mb.cr2_rvv = bool(BIT(tmp, 3));
	m_mb.cr1_rvv = bool(BIT(tmp, 2));
	m_mb.cr2_oe = bool(BIT(tmp, 1));
	m_mb.cr1_oe = bool(BIT(tmp, 0));

	tmp = read_word(mptr + 42);
	m_mb.abs_line_count = (tmp >> 12) & 0x0f;
	m_mb.invisible_char = (tmp >> 8) & 0x0f;
	m_mb.underline2 = (tmp >> 4) & 0x0f;
	m_mb.underline1 = tmp & 0x0f;

	// setup screen mode
	rectangle visarea(m_mb.hbrdstrt * 16, m_mb.hbrdstp * 16 - 1, m_mb.vsyncstp, m_mb.vfldstp + m_mb.scroll_margin + 1 + m_mb.lpr - 1);
	attoseconds_t period = HZ_TO_ATTOSECONDS(clock() * 16) * m_mb.line_length * 16 * m_mb.frame_length;
	screen().configure(m_mb.line_length * 16, m_mb.frame_length, visarea, period);

	// start display is now valid
	m_mode_set = true;

	// output some debug info
	if (VERBOSE)
	{
		logerror("%s('%s'): ---- modeset ----\n", shortname(), basetag());
		logerror("%s('%s'): dma burst length %02x, space %02x\n", shortname(), basetag(), m_mb.burst_length, m_mb.burst_space);
		logerror("%s('%s'): margin %02x, lpr %02x\n", shortname(), basetag(), m_mb.scroll_margin, m_mb.lpr);
		logerror("%s('%s'): hsyncstp: %02x, line_length: %02x, hfldstrt: %02x, hbrdstart: %02x, hfldstop: %02x, hbrdstop: %02x\n",
			shortname(), basetag(), m_mb.hsyncstp, m_mb.line_length, m_mb.hfldstrt, m_mb.hbrdstrt, m_mb.hfldstp, m_mb.hbrdstp);
		logerror("%s('%s'): frame_length %04x, vsyncstp: %04x, vfldstrt: %04x, vfldstp: %04x\n",
			shortname(), basetag(), m_mb.frame_length, m_mb.vsyncstp, m_mb.vfldstrt, m_mb.vfldstp);
	}
}

void i82730_device::execute_command()
{
	uint8_t command = read_byte(m_cbp + 1);
	uint16_t tmp;

	tmp = read_word(m_cbp + 2);
	m_list_switch = bool(BIT(tmp, 7));
	m_auto_line_feed = bool(BIT(tmp, 6));

	tmp = read_word(m_cbp + 4);
	m_max_dma_count = tmp & 0xff;

	LOGMASKED(LOG_COMMANDS, "list switch %d, autolf %d, dma count %02x\n", m_list_switch, m_auto_line_feed, m_max_dma_count);

	switch (command)
	{
	// NOP
	case 0x00:
		break;

	// START DISPLAY
	case 0x01:
		LOGMASKED(LOG_COMMANDS, "Executing command START DISPLAY\n");
		if (m_mode_set)
		{
			m_status = (m_status & ~VDIP) | DIP;
			m_row_timer->adjust(screen().time_until_pos(0));
		}
		break;

	// START VIRTUAL DISPLAY
	case 0x02:
		LOGMASKED(LOG_COMMANDS, "Executing command START VIRTUAL DISPLAY - not implemented\n");
		break;

	// STOP DISPLAY
	case 0x03:
		LOGMASKED(LOG_COMMANDS, "Executing command STOP DISPLAY\n");
		m_status &= ~(VDIP | DIP);
		m_row_timer->reset();
		break;

	// MODE SET
	case 0x04:
		LOGMASKED(LOG_COMMANDS, "Executing command MODE SET\n");
		mode_set();
		break;

	// LOAD CBP
	case 0x05:
		LOGMASKED(LOG_COMMANDS, "Executing command LOAD CBP\n");
		m_cbp = (read_word(m_cbp + 16) << 16) | read_word(m_cbp + 14);
		LOGMASKED(LOG_COMMANDS, "--> New value = %08x\n", m_cbp);
		execute_command();
		break;

	// LOAD INTMASK
	case 0x06:
		LOGMASKED(LOG_COMMANDS, "Executing command LOAD INTMASK\n");
		m_intmask = read_word(m_cbp + 22);
		LOGMASKED(LOG_COMMANDS, "--> New value = %02x\n", m_intmask);
		break;

	// LPEN ENABLE
	case 0x07:
		LOGMASKED(LOG_COMMANDS, "Executing command LPEN ENABLE - not implemented\n");
		break;

	// READ STATUS
	case 0x08:
		LOGMASKED(LOG_COMMANDS, "Executing command READ STATUS\n");
		write_word(m_cbp + 18, m_status);
		m_status &= (VDIP | DIP);
		break;

	// LD CUR POS
	case 0x09:
		LOGMASKED(LOG_COMMANDS, "Executing command LD CUR POS\n");
		tmp = read_word(m_cbp + 26);
		m_cursor[0].y = (tmp >> 8) & 0xff;
		m_cursor[0].x = tmp & 0xff;
		tmp = read_word(m_cbp + 28);
		m_cursor[1].y = (tmp >> 8) & 0xff;
		m_cursor[1].x = tmp & 0xff;
		LOGMASKED(LOG_COMMANDS, "--> Cursor %d, %d and %d, %d\n", m_cursor[0].x, m_cursor[0].y, m_cursor[1].x, m_cursor[1].y);
		break;

	// SELF TEST
	case 0x0a:
		LOGMASKED(LOG_COMMANDS, "Executing command SELF TEST - not implemented\n");
		break;

	// TEST ROW BUFFER
	case 0x0b:
		LOGMASKED(LOG_COMMANDS, "Executing command TEST ROW BUFFER - not implemented\n");
		break;

	default:
		LOGMASKED(LOG_COMMANDS, "Executing command %02x - unknown\n", command);
		m_status |= RCC;
		update_interrupts();
		break;
	}

	// clear busy
	write_word(m_cbp, read_word(m_cbp) & 0xff00);
}

bool i82730_device::dscmd_endrow()
{
	LOGMASKED(LOG_DATASTREAM, "Executing datastream command ENDROW\n");

	return true;
}

bool i82730_device::dscmd_eof()
{
	LOGMASKED(LOG_DATASTREAM, "Executing datastream command EOF - not implemented\n");

	return false;
}

bool i82730_device::dscmd_eol()
{
	LOGMASKED(LOG_DATASTREAM, "Executing datastream command EOL\n");

	m_sptr = (read_word(m_lptr + 2) << 16) | read_word(m_lptr);
	m_lptr += 4;

	return true;
}

bool i82730_device::dscmd_fulrowdescrpt(uint8_t param)
{
	LOGMASKED(LOG_DATASTREAM, "Executing datastream command FULROWDESCRPT %d\n", param);

	uint16_t tmp;

	if (param >= 1)
	{
		tmp = read_word(m_sptr);
		m_sptr += 2;
		m_mb.rvv_row = bool(BIT(tmp, 11));
		m_mb.blk_row = bool(BIT(tmp, 10));
		m_mb.dbl_hgt = bool(BIT(tmp, 9));
		m_mb.wdef = bool(BIT(tmp, 8));
		m_mb.lpr = tmp & 0x1f;

		LOGMASKED(LOG_DATASTREAM, "--> RVV_ROW %d, BLK_ROW %d, DBL_HGT %d, WDEF %d, LPR %d\n", m_mb.rvv_row, m_mb.blk_row, m_mb.dbl_hgt, m_mb.wdef, m_mb.lpr);
	}

	if (param >= 2)
	{
		tmp = read_word(m_sptr);
		m_sptr += 2;
		m_mb.nrmstrt = (tmp >> 8) & 0x1f;
		m_mb.nrmstp = tmp & 0x1f;

		LOGMASKED(LOG_DATASTREAM, "--> NRMSTRT %d, NRMSTP %d\n", m_mb.nrmstrt, m_mb.nrmstp);
	}

	if (param >= 3)
	{
		tmp = read_word(m_sptr);
		m_sptr += 2;
		m_mb.supstrt = (tmp >> 8) & 0x1f;
		m_mb.supstp = tmp & 0x1f;

		LOGMASKED(LOG_DATASTREAM, "--> SUPSTRT %d, SUPSTP %d\n", m_mb.supstrt, m_mb.supstp);
	}

	if (param >= 4)
	{
		tmp = read_word(m_sptr);
		m_sptr += 2;
		m_mb.substrt = (tmp >> 8) & 0x1f;
		m_mb.substp = tmp & 0x1f;

		LOGMASKED(LOG_DATASTREAM, "--> SUBSTRT %d, SUBSTP %d\n", m_mb.substrt, m_mb.substp);
	}

	if (param >= 5)
	{
		tmp = read_word(m_sptr);
		m_sptr += 2;
		m_mb.cur1strt = (tmp >> 8) & 0x1f;
		m_mb.cur1stp = tmp & 0x1f;

		LOGMASKED(LOG_DATASTREAM, "--> CUR1STRT %d, CUR1STP %d\n", m_mb.cur1strt, m_mb.cur1stp);
	}

	if (param >= 6)
	{
		tmp = read_word(m_sptr);
		m_sptr += 2;
		m_mb.cur2strt = (tmp >> 8) & 0x1f;
		m_mb.cur2stp = tmp & 0x1f;

		LOGMASKED(LOG_DATASTREAM, "--> CUR2STRT %d, CUR2STP %d\n", m_mb.cur2strt, m_mb.cur2stp);
	}

	if (param >= 7)
	{
		tmp = read_word(m_sptr);
		m_sptr += 2;
		m_mb.u2_line_sel = (tmp >> 8) & 0x1f;
		m_mb.u1_line_sel = tmp & 0x1f;

		LOGMASKED(LOG_DATASTREAM, "--> U2 LINE SEL %d, U1 LINE SEL %d\n", m_mb.u2_line_sel, m_mb.u1_line_sel);
	}

	return false;
}

bool i82730_device::dscmd_sl_scroll_strt(uint8_t param)
{
	LOGMASKED(LOG_DATASTREAM, "Executing datastream command SL SCROLL START %d - not implemented\n", param);

	return false;
}

bool i82730_device::dscmd_sl_scroll_end(uint8_t param)
{
	LOGMASKED(LOG_DATASTREAM, "Executing datastream command SL SCROLL END %d - not implemented\n", param);

	return false;
}

bool i82730_device::dscmd_tab_to(uint8_t param)
{
	LOGMASKED(LOG_DATASTREAM, "Executing datastream command TAB TO %d - not implemented\n", param);

	return false;
}

bool i82730_device::dscmd_max_dma_count(uint8_t param)
{
	LOGMASKED(LOG_DATASTREAM, "Executing datastream command LD MAX DMA COUNT %02x\n", param);

	m_max_dma_count = param;

	return false;
}

bool i82730_device::dscmd_endstrg()
{
	LOGMASKED(LOG_DATASTREAM, "Executing datastream command ENDSTRG\n");

	m_sptr = (read_word(m_lptr + 2) << 16) | read_word(m_lptr);
	m_lptr += 4;

	LOGMASKED(LOG_DATASTREAM, "--> SPTR %08x\n", m_sptr);

	return false;
}

bool i82730_device::dscmd_skip(uint8_t param)
{
	LOGMASKED(LOG_DATASTREAM, "Executing datastream command SKIP %d - not implemented\n", param);

	return false;
}

bool i82730_device::dscmd_repeat(uint8_t param)
{
	LOGMASKED(LOG_DATASTREAM, "Executing datastream command REPEAT %d\n", param);

	uint16_t data = read_word(m_sptr);
	m_sptr += 2;

	LOGMASKED(LOG_DATASTREAM, "--> Repeating %02x\n", data);

	while (param--)
	{
		if (--m_dma_count && m_row_count < 200)
			m_row[m_row_count++] = data;
		else
			return true;
	}

	return false;
}

bool i82730_device::dscmd_sub_sup(uint8_t param)
{
	LOGMASKED(LOG_DATASTREAM, "Executing datastream command SUP SUB - not implemented\n", param);

	return false;
}

bool i82730_device::dscmd_rpt_sub_sup(uint8_t param)
{
	LOGMASKED(LOG_DATASTREAM, "Executing datastream command RPT SUP SUB - not implemented\n", param);

	return false;
}

bool i82730_device::dscmd_set_gen_pur_attrib(uint8_t param)
{
	LOGMASKED(LOG_DATASTREAM, "Executing datastream command SET GEN PUR ATTRIB - not implemented\n", param);

	return false;
}

bool i82730_device::dscmd_set_field_attrib()
{
	LOGMASKED(LOG_DATASTREAM, "Executing datastream command SET FIELD ATTRIB\n");

	m_mb.field_attribute_mask = read_word(m_sptr) & 0x7fff;
	m_sptr += 2;

	LOGMASKED(LOG_DATASTREAM, "--> New value = %04x\n", m_mb.field_attribute_mask);

	return false;
}

bool i82730_device::dscmd_init_next_process()
{
	LOGMASKED(LOG_DATASTREAM, "Executing datastream command INIT NEXT PROCESS - not implemented\n");

	return false;
}

bool i82730_device::execute_datastream_command(uint8_t command, uint8_t param)
{
	// RESERVED
	if (command >= 0x90 && command <= 0xbf)
	{
		LOGMASKED(LOG_DATASTREAM, "Executing reserved datastream command %02x\n", command);

		m_status |= RDC;
		update_interrupts();

		return false;
	}

	// NOP
	if (command >= 0xc0)
		return false;

	// DATASTREAM COMMANDS
	switch (command)
	{
		case 0x80: return dscmd_endrow();
		case 0x81: return dscmd_eof();
		case 0x82: return dscmd_eol();
		case 0x83: return dscmd_fulrowdescrpt(param);
		case 0x84: return dscmd_sl_scroll_strt(param);
		case 0x85: return dscmd_sl_scroll_end(param);
		case 0x86: return dscmd_tab_to(param);
		case 0x87: return dscmd_max_dma_count(param);
		case 0x88: return dscmd_endstrg();
		case 0x89: return dscmd_skip(param);
		case 0x8a: return dscmd_repeat(param);
		case 0x8b: return dscmd_sub_sup(param);
		case 0x8c: return dscmd_rpt_sub_sup(param);
		case 0x8d: return dscmd_set_gen_pur_attrib(param);
		case 0x8e: return dscmd_set_field_attrib();
		case 0x8f: return dscmd_init_next_process();
	}

	// should never get here
	return false;
}

void i82730_device::load_row()
{
	bool finished = false;

	m_dma_count = m_max_dma_count;
	m_row_count = 0;

	while (!finished)
	{
		uint16_t data = read_word(m_sptr);
		m_sptr += 2;

		if (BIT(data, 15))
		{
			finished = execute_datastream_command(data >> 8, data & 0xff);
		}
		else
		{
			// fetch data
			if (--m_dma_count > 0)
			{
				if (m_row_count < 200)
				{
					m_row[m_row_count++] = data;
				}
				else
				{
					// buffer overrun
					m_status |= DBOR;
					update_interrupts();
					finished = true;
				}
			}
			else
			{
				if (!m_auto_line_feed)
				{
					m_sptr = (read_word(m_lptr + 2) << 16) | read_word(m_lptr);
					m_lptr += 4;
				}

				finished = true;
			}
		}
	}
}

TIMER_CALLBACK_MEMBER( i82730_device::row_update )
{
	int y = screen().vpos();

	if (y == 0)
	{
		// clear interrupt status flags
		m_status &= (VDIP | DIP);

		// clear field attribute mask
		m_mb.field_attribute_mask = 0;

		// get listbase
		if (m_list_switch == 0)
			m_lptr = (read_word(m_cbp + 8) << 16) | read_word(m_cbp + 6);
		else
			m_lptr = (read_word(m_cbp + 12) << 16) | read_word(m_cbp + 10);

		m_sptr = (read_word(m_lptr + 2) << 16) | read_word(m_lptr);
		m_lptr += 4;

		// fetch initial row
		m_row_index = 0;
		m_row = &m_row_buffer[m_row_index][0];
		load_row();
	}
	else if (y >= m_mb.vsyncstp && y < m_mb.vfldstrt)
	{
		// blank (top border)
	}
	else if (y >= m_mb.vfldstrt && y < m_mb.vfldstp)
	{
		uint8_t lc = (y - m_mb.vfldstrt) % (m_mb.lpr + 1);

		// call driver
		m_update_row_cb(m_bitmap, m_row, lc, y - m_mb.vsyncstp, m_row_count);

		// swap buffers at end of row
		if (lc == m_mb.lpr)
		{
			m_row_index ^= 1;
			m_row = &m_row_buffer[m_row_index][0];

			// load status row data at end of regular display
			if (y == (m_mb.vfldstp - 1))
				m_sptr = (read_word(m_cbp + 36) << 16) | read_word(m_cbp + 34);

			load_row();
		}
	}
	else if (y >= m_mb.vfldstp && y < m_mb.vfldstp + m_mb.scroll_margin + 1)
	{
		// margin
	}
	else if (y >= m_mb.vfldstp + m_mb.scroll_margin + 1 && y < m_mb.vfldstp + m_mb.scroll_margin + 1 + m_mb.lpr + 1)
	{
		uint8_t lc = (y - (m_mb.vfldstp + m_mb.scroll_margin + 1)) % (m_mb.lpr + 1);

		// call driver
		m_update_row_cb(m_bitmap, m_row, lc, y - m_mb.vsyncstp, m_row_count);
	}
	else if (y == m_mb.vfldstp + m_mb.scroll_margin + 1 + m_mb.lpr + 1)
	{
		// check ca latch
		if (m_ca_latch)
			attention();

		// frame interrupt?
		if ((screen().frame_number() % m_mb.frame_int_count) == 0)
			m_status |= EONF;

		// check interrupts
		update_interrupts();
	}
	else
	{
		// vblank
	}

	// schedule next line (if enabled)
	if (m_status & DIP)
		m_row_timer->adjust(screen().time_until_pos((y + 1) % screen().height()));
}

void i82730_device::attention()
{
	execute_command();
	m_ca_latch = false;
}

WRITE_LINE_MEMBER( i82730_device::ca_w )
{
	// falling edge
	if (m_ca == 1 && state == 0)
		m_ca_latch = true;

	m_ca = state;

	// check ca every cycle if the display isn't active
	if (m_ca_latch && ((m_status & DIP) == 0))
	{
		if (!m_initialized)
		{
			// get system bus width
			m_sysbus = m_program->read_byte(0xfffffff6);

			// get intermediate block pointer
			m_ibp = (read_word(0xfffffffe) << 16) | read_word(0xfffffffc);

			m_cbp = (read_word(m_ibp + 4) << 16) | read_word(m_ibp + 2);

			// get system configuration byte
			uint8_t scb = read_byte(m_ibp + 6);

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

		attention();
	}
}

WRITE_LINE_MEMBER( i82730_device::irst_w )
{
	m_sint_handler(0);
}

uint32_t i82730_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, m_mb.hfldstrt * 16, 0, cliprect);
	return 0;
}
