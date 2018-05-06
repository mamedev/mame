// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***********************************************************************

    Fujitsu MB88303 NMOS Television Display Controller (TVDC) emulation

    TOOD:
    - Character-size registers.
    - Proper hsync/vsync emulation.
    - Blinking.

***********************************************************************/

#include "emu.h"
#include "mb88303.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(MB88303, mb88303_device, "mb88303", "Fujitsu MB88303")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

/*static*/ const uint8_t mb88303_device::s_character_data[0x40][7] = {
	{ 0x0e, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x11 }, // 0x00
	{ 0x1e, 0x11, 0x11, 0x1e, 0x11, 0x11, 0x1e },
	{ 0x0e, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0e },
	{ 0x1e, 0x09, 0x09, 0x09, 0x09, 0x09, 0x1e },
	{ 0x1f, 0x10, 0x10, 0x1e, 0x10, 0x10, 0x1f },
	{ 0x1f, 0x10, 0x10, 0x1e, 0x10, 0x10, 0x10 },
	{ 0x0f, 0x10, 0x10, 0x13, 0x11, 0x11, 0x0f },
	{ 0x11, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x11 },
	{ 0x0e, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0e },
	{ 0x01, 0x01, 0x01, 0x01, 0x11, 0x11, 0x0e },
	{ 0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11 },
	{ 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1f },
	{ 0x11, 0x1b, 0x15, 0x15, 0x11, 0x11, 0x11 },
	{ 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x1f },
	{ 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80 },
	{ 0x11, 0x11, 0x19, 0x15, 0x13, 0x11, 0x11 }, // 0x10
	{ 0x0e, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e },
	{ 0x1e, 0x11, 0x11, 0x1e, 0x10, 0x10, 0x10 },
	{ 0x0e, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0d },
	{ 0x1e, 0x11, 0x11, 0x1e, 0x14, 0x12, 0x11 },
	{ 0x0e, 0x11, 0x10, 0x0e, 0x01, 0x11, 0x0e },
	{ 0x1f, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04 },
	{ 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e },
	{ 0x11, 0x11, 0x11, 0x11, 0x11, 0x0a, 0x04 },
	{ 0x11, 0x11, 0x11, 0x15, 0x15, 0x1b, 0x11 },
	{ 0x11, 0x11, 0x0a, 0x04, 0x0a, 0x11, 0x11 },
	{ 0x11, 0x11, 0x0a, 0x04, 0x04, 0x04, 0x04 },
	{ 0x1f, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1f },
	{ 0x00, 0x00, 0x04, 0x00, 0x04, 0x00, 0x00 },
	{ 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f },
	{ 0x02, 0x04, 0x04, 0x04, 0x04, 0x04, 0x02 },
	{ 0x0e, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0e }, // 0x20
	{ 0x04, 0x0c, 0x04, 0x04, 0x04, 0x04, 0x04 },
	{ 0x0e, 0x11, 0x01, 0x0e, 0x10, 0x10, 0x1f },
	{ 0x0e, 0x11, 0x01, 0x06, 0x01, 0x11, 0x0e },
	{ 0x06, 0x0a, 0x12, 0x12, 0x1f, 0x02, 0x02 },
	{ 0x1f, 0x10, 0x1e, 0x01, 0x01, 0x11, 0x0e },
	{ 0x0e, 0x11, 0x10, 0x1e, 0x11, 0x11, 0x0e },
	{ 0x1f, 0x11, 0x01, 0x02, 0x04, 0x04, 0x04 },
	{ 0x0e, 0x11, 0x11, 0x0e, 0x11, 0x11, 0x0e },
	{ 0x0e, 0x11, 0x11, 0x0f, 0x01, 0x11, 0x0e },
	{ 0x0e, 0x11, 0x01, 0x06, 0x04, 0x00, 0x04 },
	{ 0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x04 },
	{ 0x06, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x0c },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x08, 0x04, 0x04, 0x04, 0x04, 0x04, 0x08 },
	{ 0x00, 0x04, 0x0a, 0x15, 0x04, 0x04, 0x00 }, // 0x30
	{ 0x00, 0x04, 0x04, 0x15, 0x0a, 0x04, 0x00 },
	{ 0x00, 0x04, 0x08, 0x17, 0x08, 0x04, 0x00 },
	{ 0x00, 0x04, 0x02, 0x1d, 0x02, 0x04, 0x00 },
	{ 0x00, 0x04, 0x04, 0x1f, 0x04, 0x04, 0x00 },
	{ 0x00, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x00 },
	{ 0x00, 0x04, 0x15, 0x0e, 0x15, 0x04, 0x00 },
	{ 0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x00 },
	{ 0x00, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x00 },
	{ 0x0c, 0x12, 0x14, 0x08, 0x15, 0x12, 0x0d },
	{ 0x10, 0x1f, 0x02, 0x0f, 0x0a, 0x1f, 0x02 },
	{ 0x1f, 0x11, 0x1f, 0x11, 0x1f, 0x11, 0x11 },
	{ 0x1f, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x1f },
	{ 0x00, 0x00, 0x00, 0x00, 0x0c, 0x04, 0x08 },
	{ 0x00, 0x00, 0x00, 0x08, 0x15, 0x02, 0x00 },
	{ 0x0e, 0x11, 0x04, 0x1f, 0x1b, 0x1f, 0x11 }
};


//-------------------------------------------------
//  mb88303_device - constructor
//-------------------------------------------------

mb88303_device::mb88303_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MB88303, tag, owner, clock)
	, m_write_vow(*this)
	, m_write_vobn(*this)
	, m_write_do(*this)
	, m_horiz_display_pos(0)
	, m_vert_display_pos(0)
	, m_display_ctrl(0)
	, m_general_out(7)
	, m_address(0)
	, m_addr_inc_mode(false)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mb88303_device::device_start()
{
	// resolve callbacks
	m_write_vow.resolve_safe();
	m_write_vobn.resolve_safe();
	m_write_do.resolve_safe();

	// register for state saving
	save_item(NAME(m_display_mem));
	save_item(NAME(m_horiz_display_pos));
	save_item(NAME(m_vert_display_pos));
	save_item(NAME(m_display_ctrl));
	save_item(NAME(m_general_out));
	save_item(NAME(m_address));
	save_item(NAME(m_addr_inc_mode));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mb88303_device::device_reset()
{
	m_horiz_display_pos = 0;
	m_vert_display_pos = 0;
	m_display_ctrl = 0;
	m_general_out = 0x07;
	m_address = 0;
	m_addr_inc_mode = false;

	memset(m_display_mem, 0, sizeof(uint8_t) * 180);

	m_da = 0;
	m_adm = 0;
	m_reset_n = 1;
	m_ldi = 0;
	m_hsync_n = 1;
	m_vsync_n = 1;

	m_write_vow(CLEAR_LINE);
	m_write_vobn(ASSERT_LINE);
	m_write_do(m_general_out);
}


WRITE8_MEMBER( mb88303_device::da_w )
{
	//logerror("%s: mb88303_device::da_w: %02x\n", machine().describe_context(), data);
	m_da = data;
}

WRITE_LINE_MEMBER( mb88303_device::adm_w )
{
	//logerror("%s: mb88303_device::adm_w: %02x\n", machine().describe_context(), state);
	m_adm = state;
}

WRITE_LINE_MEMBER( mb88303_device::reset_n_w )
{
	logerror("%s: mb88303_device::reset_n_w: %02x\n", machine().describe_context(), state);
	uint8_t old = m_reset_n;
	m_reset_n = state;

	if (old != m_reset_n && m_reset_n)
	{
		m_horiz_display_pos = 0;
		m_vert_display_pos = 0;
		m_display_ctrl = 0;
		m_general_out = 0x07;
		m_addr_inc_mode = false;

		m_write_vow(CLEAR_LINE);
		m_write_vobn(ASSERT_LINE);
		m_write_do(m_general_out);
	}
}

WRITE_LINE_MEMBER( mb88303_device::ldi_w )
{
	//logerror("%s: mb88303_device::ldi_w: %02x\n", machine().describe_context(), state);
	uint8_t old = m_ldi;
	m_ldi = state;

	if (m_ldi != old)
	{
		if (!m_ldi)
		{
			// Direct Address Mode: At the trailing edge of LDI, a 7-bit
			// data on DA6-DA0 is written into an internal control register
			// or an internal display memory location that is designated by
			// the address latched at the leading edge.

			// Address Increment Mode: At the trailing edge of LDI, a 7-bit
			// data on DA6-DA0 is written into an internal control register
			// or a display memory location that is indicated by the
			// address register.
			process_data();
		}
		else if (!m_adm)
		{
			// Direct Address Mode: At the leading edge of LDI, an 8-bit
			// address on DA7-DA0 is automatically latched into the
			// internal address register.
			//logerror("%s: mb88303_device::ldi_w: new address = %02x\n", machine().describe_context(), m_da);
			m_address = m_da;
		}
		else
		{
			// Address Increment Mode: At the leading edge of LDI, the
			// address register is automatically incremented.
			m_address++;
			//logerror("%s: mb88303_device::ldi_w: new address = %02x\n", machine().describe_context(), m_address);
		}
	}
}

void mb88303_device::process_data()
{
	switch (m_address)
	{
		case 180:
			hdpr_w(m_da);
			break;
		case 181:
			vdpr_w(m_da);
			break;
		case 182:
			dcr_w(m_da);
			break;
		case 183:
			gor_w(m_da);
			break;
		default:
			assert(m_address < 180);
			m_display_mem[m_address] = m_da;
			break;
	}
}

WRITE_LINE_MEMBER( mb88303_device::hsync_n_w )
{
	m_hsync_n = state;
}

WRITE_LINE_MEMBER( mb88303_device::vsync_n_w )
{
	m_vsync_n = state;
}

void mb88303_device::hdpr_w(uint8_t data)
{
	logerror("%s: mb88303_device::hdpr_w: %02x\n", machine().describe_context(), data);
	m_horiz_display_pos = data & 0x3f;
}

void mb88303_device::vdpr_w(uint8_t data)
{
	logerror("%s: mb88303_device::vdpr_w: %02x\n", machine().describe_context(), data);
	m_vert_display_pos = data & 0x3f;
}

void mb88303_device::dcr_w(uint8_t data)
{
	logerror("%s: mb88303_device::dcr_w: %02x\n", machine().describe_context(), data);
	m_display_ctrl = data & 0x7f;
}

void mb88303_device::gor_w(uint8_t data)
{
	//logerror("%s: mb88303_device::gor_w: %02x\n", machine().describe_context(), data);
	uint8_t old = m_general_out;
	m_general_out = data & 0x07;
	if (m_general_out != old)
	{
		m_write_do(m_general_out);
	}
}


bool mb88303_device::blank_display()
{
	return !BIT(m_display_ctrl, BLK_BIT);
}

bool mb88303_device::show_background()
{
	return BIT(m_display_ctrl, BLKB_BIT);
}

bool mb88303_device::enable_blinking()
{
	return BIT(m_display_ctrl, BLINK_BIT);
}

void mb88303_device::update_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (blank_display())
	{
		bitmap.fill(0, cliprect);
		return;
	}

	for (unsigned x = 0; x < VISIBLE_COLUMNS; x++)
	{
		for (unsigned y = 0; y < VISIBLE_LINES; y++)
		{
			uint16_t bitmap_y = y * 9;
			uint16_t bitmap_x = x * 6;
			const uint8_t *gfx = s_character_data[m_display_mem[y * VISIBLE_COLUMNS + x] & 0x3f];
			for (unsigned row = 0; row < 7; row++)
			{
				for (unsigned bit = 0; bit < 6; bit++)
				{
					if (gfx[row] & 0x80)
						continue;
					const uint32_t color = BIT(gfx[row], bit) ? 0xffffffff : 0;
					if (!show_background() && !color)
						continue;
					bitmap.pix32(bitmap_y + row + m_vert_display_pos, bitmap_x + (6 - bit) + m_horiz_display_pos) = color;
				}
			}
		}
	}
}
