// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  SigmaSoft Interactive Graphics Controller (IGC) for the Heath TLB

  Not Currently Implemented
    - Interlace video support (with higher resolution)
    - Joystick support
    - Centronics support
    - Lightpen support
    - Alternate Font ROM support (missing ROM image)

****************************************************************************/

#include "emu.h"
#include "igc.h"

#include <algorithm>

#define LOG_REG (1U << 1)    // Shows register setup

//#define VERBOSE (LOG_REG)

#include "logmacro.h"

#define LOGREG(...)        LOGMASKED(LOG_REG, __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif


namespace bus::heathzenith::h19 {

INPUT_PORTS_START( igc_common )
	PORT_START("joystick_p1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )

	PORT_START("joystick_p2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(2)

INPUT_PORTS_END

namespace {

static INPUT_PORTS_START( igc )
	PORT_INCLUDE( tlb )

	PORT_INCLUDE( igc_common )
INPUT_PORTS_END

} // anonymous namespace


/**
 * SigmaSoft Interactive Graphics Controller (IGC)
 *
 *
 */
heath_igc_tlb_device::heath_igc_tlb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	heath_igc_tlb_device(mconfig, HEATH_IGC, tag, owner, clock)
{
}

heath_igc_tlb_device::heath_igc_tlb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	heath_tlb_device(mconfig, type, tag, owner, clock),
	m_joystick1(*this, "joystick_p1"),
	m_joystick2(*this, "joystick_p2")
{
}

ioport_constructor heath_igc_tlb_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(igc);
}

void heath_igc_tlb_device::device_add_mconfig(machine_config &config)
{
	heath_tlb_device::device_add_mconfig(config);

	m_crtc->set_update_row_callback(FUNC(heath_igc_tlb_device::crtc_update_row));
}

void heath_igc_tlb_device::device_start()
{
	heath_tlb_device::device_start();

	m_p_graphic_ram = make_unique_clear<u8[]>(0x3ffff);
	save_pointer(NAME(m_p_graphic_ram), 0x3ffff);

	save_item(NAME(m_data_reg));
	save_item(NAME(m_pixel_video_enabled));
	save_item(NAME(m_character_video_disabled));
	save_item(NAME(m_video_invert_enabled));
	save_item(NAME(m_alternate_character_set_enabled));
	save_item(NAME(m_video_fill_enabled));
	save_item(NAME(m_read_address_increment_disabled));
	save_item(NAME(m_memory_bank_select));
	save_item(NAME(m_io_address));
	save_item(NAME(m_window_address));
}

void heath_igc_tlb_device::device_reset()
{
	heath_tlb_device::device_reset();

	sigma_ctrl_w(0);
	m_data_reg       = 0x00;
	m_io_address     = 0x0000;
	m_window_address = 0x0000;
}

void heath_igc_tlb_device::sigma_w(u8 offset, u8 data)
{
	switch (offset)
	{
	case 0:
		sigma_video_mem_w(data);
		break;
	case 1:
		sigma_io_lo_addr_w(data);
		break;
	case 2:
		sigma_io_hi_addr_w(data);
		break;
	case 3:
		sigma_window_lo_addr_w(data);
		break;
	case 4:
		sigma_window_hi_addr_w(data);
		break;
	case 5:
		sigma_ctrl_w(data);
		break;
	}
}

u8 heath_igc_tlb_device::sigma_r(u8 offset)
{
	u8 val = 0;

	switch (offset)
	{
	case 0:
		val = sigma_video_mem_r();
		break;
	case 1:
		// TODO - Low pen address
		break;
	case 2:
		// TODO - High pen address
		break;
	case 3:
		// Left input device
		val = m_joystick1->read();
		break;
	case 4:
		// Right input device
		val = m_joystick2->read();
		break;
	case 5:
		val = sigma_ctrl_r();
		break;
	}

	return val;
}

void heath_igc_tlb_device::sigma_ctrl_w(u8 data)
{
	LOGREG("%s: data: %02x\n", FUNCNAME, data);

	m_pixel_video_enabled             = bool(BIT(data, 0));
	m_character_video_disabled        = bool(BIT(data, 1));
	m_video_invert_enabled            = bool(BIT(data, 2));
	m_alternate_character_set_enabled = bool(BIT(data, 3));
	m_video_fill_enabled              = bool(BIT(data, 4));
	m_read_address_increment_disabled = bool(BIT(data, 5));

	m_memory_bank_select              = bitswap<2>(data, 6, 7);

	if (m_video_fill_enabled)
	{
		std::fill_n(&m_p_graphic_ram[(m_memory_bank_select << 16) & 0x3ffff], 0x10000, m_data_reg);
	}
}

u8 heath_igc_tlb_device::sigma_ctrl_r()
{
	u8 ret_val = 0x00;

	ret_val |= m_pixel_video_enabled             ? 0x01 : 0x00;
	ret_val |= m_character_video_disabled        ? 0x02 : 0x00;
	ret_val |= m_video_invert_enabled            ? 0x04 : 0x00;
	ret_val |= m_alternate_character_set_enabled ? 0x08 : 0x00;
	ret_val |= m_video_fill_enabled              ? 0x10 : 0x00;
	ret_val |= m_read_address_increment_disabled ? 0x20 : 0x00;

	ret_val |= bitswap<2>(m_memory_bank_select, 0, 1) << 6;

	LOGREG("%s: ret_val: %02x\n", FUNCNAME, ret_val);

	return ret_val;
}

void heath_igc_tlb_device::sigma_video_mem_w(u8 data)
{
	LOGREG("%s: data: %02x\n", FUNCNAME, data);

	m_data_reg                                                     = data;

	m_p_graphic_ram[(m_memory_bank_select << 16) + m_io_address++] = data;
}

u8 heath_igc_tlb_device::sigma_video_mem_r()
{
	// control whether m_io_address is incremented during a read
	u32 addr = (m_read_address_increment_disabled || machine().side_effects_disabled()) ?
		((m_memory_bank_select << 16) + m_io_address) :
		((m_memory_bank_select << 16) + m_io_address++);

	return m_p_graphic_ram[addr];
}

void heath_igc_tlb_device::sigma_io_lo_addr_w(u8 data)
{
	LOGREG("%s: data: %02x\n", FUNCNAME, data);

	m_io_address = (m_io_address & 0xff00) | data;
}

void heath_igc_tlb_device::sigma_io_hi_addr_w(u8 data)
{
	LOGREG("%s: data: %02x\n", FUNCNAME, data);

	m_io_address = (data << 8) | (m_io_address & 0x00ff);
}

void heath_igc_tlb_device::sigma_window_lo_addr_w(u8 data)
{
	LOGREG("%s: data: %02x\n", FUNCNAME, data);

	m_window_address = (m_window_address & 0xff00) | data;
}

void heath_igc_tlb_device::sigma_window_hi_addr_w(u8 data)
{
	LOGREG("%s: data: %02x\n", FUNCNAME, data);

	m_window_address = (data << 8) | (m_window_address & 0x00ff);
}

MC6845_UPDATE_ROW(heath_igc_tlb_device::crtc_update_row)
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	u32               *p       = &bitmap.pix(y);

	if (de)
	{
		u8 video_invert = m_video_invert_enabled ? 0xff : 0;

		for (int x = 0; x < x_count; x++)
		{
			u8 output = 0x00;

			if (!m_character_video_disabled)
			{
				u8 inv = (x == cursor_x) ? 0xff : 0;
				u8 chr = m_p_videoram[(ma + x) & 0x7ff];

				if (chr & 0x80)
				{
					inv ^= 0xff;
					chr &= 0x7f;
				}

				// TODO handle alt font
				output |= m_p_chargen[(chr << 4) | ra] ^ inv;
			}

			if (m_pixel_video_enabled)
			{
				output |= m_p_graphic_ram[(((y * 80) + x) + m_window_address) & 0xffff];
			}

			output ^= video_invert;

			for (int b = 0; 8 > b; ++b)
			{
				*p++ = palette[BIT(output, b)];
			}
		}
	}
	else
	{
		std::fill_n(p, x_count * 8, palette[0]);
	}
}

} // namespace bus::heathzenith::h19

DEFINE_DEVICE_TYPE_PRIVATE(HEATH_IGC, device_heath_tlb_card_interface, bus::heathzenith::h19::heath_igc_tlb_device, "heath_igc_tlb_device", "Heath Terminal Logic Board plus SigmaSoft Interactive Graphics Controller")
