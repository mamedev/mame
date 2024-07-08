// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Toshiba T6963C Dot Matrix LCD Controller
    Sharp LM24014H Dot Matrix LCD Unit

		TODO:
		- cursor
		- screen peek
		- screen copy
		- auto read mode

**********************************************************************/

#include "emu.h"
#include "t6963c.h"

//#define VERBOSE 1
#include "logmacro.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definitions
DEFINE_DEVICE_TYPE(T6963C, t6963c_device, "t6963c", "T6963C LCD Controller")
DEFINE_DEVICE_TYPE(LM24014H, lm24014h_device, "lm24014h", "LM24014H LCD Unit (240x64)")


//**************************************************************************
//  T6963C LCD CONTROLLER
//**************************************************************************

ALLOW_SAVE_TYPE(t6963c_device::auto_mode);

//-------------------------------------------------
//  t6963c_device - constructor
//-------------------------------------------------

t6963c_device::t6963c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, T6963C, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_display_config("display", ENDIANNESS_LITTLE, 8, 16, 0)
	, m_cgrom(*this, "cgrom")
	, m_display_ram(nullptr)
	, m_data(0)
	, m_adp(0)
	, m_auto_mode(auto_mode::NONE)
	, m_graphic_home(0)
	, m_text_home(0)
	, m_graphic_area(0)
	, m_text_area(0)
	, m_cgram_offset(0)
	, m_mode(0)
	, m_display_mode(0)
	, m_font_size(6)
	, m_number_cols(40)
	, m_number_lines(8)
	, m_read_data(0)
{
}


//-------------------------------------------------
//  memory_space_config - return the configuration
//  for the device's address spaces
//-------------------------------------------------

device_memory_interface::space_config_vector t6963c_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_display_config),
	};
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void t6963c_device::device_start()
{
	m_display_ram = &space(0);

	save_item(NAME(m_data));
	save_item(NAME(m_adp));
	save_item(NAME(m_auto_mode));
	save_item(NAME(m_graphic_home));
	save_item(NAME(m_text_home));
	save_item(NAME(m_graphic_area));
	save_item(NAME(m_text_area));
	save_item(NAME(m_cgram_offset));
	save_item(NAME(m_mode));
	save_item(NAME(m_display_mode));
	save_item(NAME(m_font_size));
	save_item(NAME(m_number_cols));
	save_item(NAME(m_number_lines));
	save_item(NAME(m_read_data));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void t6963c_device::device_reset()
{
	m_auto_mode = auto_mode::NONE;
	m_data = 0;
	m_adp = 0;
	m_graphic_home = 0;
	m_text_home = 0;
	m_graphic_area = 0;
	m_text_area = 0;
	m_cgram_offset = 0;
	m_mode = 0;
	m_display_mode = 0;
}


//-------------------------------------------------
//  read - read from the status or data register
//-------------------------------------------------

u8 t6963c_device::read(offs_t offset)
{
	return BIT(offset, 0) ? 0x2b : m_read_data;
}


//-------------------------------------------------
//  write - write to the command or data register
//-------------------------------------------------

void t6963c_device::write(offs_t offset, u8 data)
{
	if (BIT(offset, 0))
	{
		// Command register
		do_command(data);
	}
	else
	{
		// Data register
		if (m_auto_mode == auto_mode::WRITE)
		{
			LOG("%s: Auto write %02X to %04X\n", machine().describe_context(), data, m_adp);
			m_display_ram->write_byte(m_adp++, data);
		}
		else
			m_data = (u16(data) << 8) | (m_data >> 8);
	}
}


//-------------------------------------------------
//  do_command - decode and execute one command
//-------------------------------------------------

void t6963c_device::do_command(u8 cmd)
{
	if ((cmd & 0xf0) == 0x20)
	{
		switch (cmd & 0x0f)
		{
		case 0x01:
			logerror("%s: Setting cursor pointer X:Y = %02X:%02X\n", machine().describe_context(), m_data & 0xff, m_data >> 8);
			break;

		case 0x02:
			LOG("%s: Setting offset register for CG RAM at %04X to %04XH\n",
				machine().describe_context(),
				(m_data & 0x1f) << 11,
				(m_data & 0x1f) << 11 | 0x7ff);

			m_cgram_offset = (m_data & 0x1f) << 11;
			break;

		case 0x04:
			LOG("%s: Setting address pointer = %04X\n", machine().describe_context(), m_data);
			m_adp = m_data;
			break;

		default:
			logerror("%s: Setting unknown pointer = %04X\n", machine().describe_context(), m_data);
			break;
		}
	}
	else if ((cmd & 0xfd) == 0x40)
	{
		LOG("%s: Setting %s home address = %04X\n", machine().describe_context(),
			BIT(cmd, 1) ? "graphic" : "text",
			m_data);

		if (BIT(cmd, 1))
			m_graphic_home = m_data;
		else
			m_text_home = m_data;
	}
	else if ((cmd & 0xfd) == 0x41)
	{
		LOG("%s: Setting %s area = %d columns\n", machine().describe_context(),
			BIT(cmd, 1) ? "graphic" : "text",
			m_data);


		if (BIT(cmd, 1))
			m_graphic_area = m_data;
		else
			m_text_area = m_data;
	}
	else if ((cmd & 0xf0) == 0x80)
	{
		LOG("%s: %s mode, %s\n", machine().describe_context(),
			(cmd & 0x07) == 0x00 ? "OR"
				: (cmd & 0x07) == 0x01 ? "EXOR"
				: (cmd & 0x07) == 0x03 ? "AND"
				: (cmd & 0x07) == 0x04 ? "Text attribute"
				: "Unknown",
			BIT(cmd, 3) ? "external CG RAM" : "internal CG ROM");

		m_mode = cmd & 0x0f;
	}
	else if ((cmd & 0xf0) == 0x90)
	{
		if (cmd == 0x90)
			LOG("%s: Display off\n", machine().describe_context());
		else
			LOG("%s: Graphic %s, text %s, cursor %s, blink %s\n",
				machine().describe_context(),
				BIT(cmd, 3) ? "on" : "off",
				BIT(cmd, 2) ? "on" : "off",
				BIT(cmd, 1) ? "on" : "off",
				BIT(cmd, 0) ? "on" : "off");
		m_display_mode = cmd & 0x0f;
	}
	else if ((cmd & 0xf8) == 0xa0)
	{
		logerror("%s: Select %d-line cursor\n", machine().describe_context(), (cmd & 0x07) + 1);
	}
	else if ((cmd & 0xfe) == 0xb0)
	{
		LOG("%s: Set data auto %s\n", machine().describe_context(), BIT(cmd, 0) ? "read" : "write");
		m_auto_mode = BIT(cmd, 0) ? auto_mode::READ : auto_mode::WRITE;
	}
	else if (cmd == 0xb2)
	{
		LOG("%s: Auto reset\n", machine().describe_context());
		m_auto_mode = auto_mode::NONE;
	}
	else if ((cmd & 0xf0) == 0xc0)
	{
		if (BIT(cmd, 0))
		{
			LOG("%s: Read data from %04X and %s ADP\n", machine().describe_context(),
					m_adp,
					(cmd & 0x0e) == 0x00 ? "increment"
					: (cmd & 0x0e) == 0x02 ? "decrement"
					: (cmd & 0x0e) == 0x04 ? "nonvariable"
					: "invalid");
			m_read_data = m_display_ram->read_byte(m_adp);
		}
		else
		{
			LOG("%s: Write %02X to %04X and %s ADP\n", machine().describe_context(),
					m_data >> 8,
					m_adp,
					(cmd & 0x0e) == 0x00 ? "increment"
					: (cmd & 0x0e) == 0x02 ? "decrement"
					: (cmd & 0x0e) == 0x04 ? "nonvariable"
					: "invalid");

			m_display_ram->write_byte(m_adp, m_data >> 8);
		}

		if ((cmd & 0x0e) == 0x00)
			++m_adp;
		else if ((cmd & 0x0e) == 0x02)
			--m_adp;
	}
	else if (cmd == 0xe0)
	{
		logerror("%s: Screen peek\n", machine().describe_context());
	}
	else if (cmd == 0xe8)
	{
		logerror("%s: Screen copy\n", machine().describe_context());
	}
	else if ((cmd & 0xf0) == 0xf0)
	{
		LOG("%s: %s bit %d\n", machine().describe_context(),
			BIT(cmd, 3) ? "Set" : "Reset",
			cmd & 0x07);

		u8 data = m_display_ram->read_byte(m_adp);
		if (BIT(cmd, 3))
			data |= 1 << (cmd & 0x07);
		else
			data &= ~(1 << (cmd & 0x07));

		m_display_ram->write_byte(m_adp, data);
	}
	else
	{
		logerror("%s: Unknown command %02X\n", machine().describe_context(), cmd);
	}
}

void t6963c_device::set_fs(u8 data)
{
	m_font_size = 8 - (data & 3);
}

void t6963c_device::set_md(u8 data)
{
	// MD0, MD1
	m_number_lines = 8 - (data & 3) * 2;

	if (BIT(data, 4)) // MDS
		m_number_lines += 8;

	switch((data >> 2) & 3) // MD2, MD3
	{
	case 0: m_number_cols = 32; break;
	case 1: m_number_cols = 40; break;
	case 2: m_number_cols = 64; break;
	case 3: m_number_cols = 80; break;
	}
}


uint32_t t6963c_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs;
	bitmap.fill(0, cliprect);

	// Text layer
	if (BIT(m_display_mode, 2))
	{
		offs = m_text_home;
		for(int y=0; y<m_number_lines; y++)
			for(int x=0; x<m_text_area; x++)
			{
				u8 c = m_display_ram->read_byte(offs++);

				for(int cy=0; cy<8; cy++)
				{
					u8 data;
					if (!BIT(m_mode, 3))
					{
						if (c < 0x80)
							data = m_cgrom[c * 8 + cy];
						else
							data = m_display_ram->read_byte(m_cgram_offset + (c & 0x7f) * 8 + cy);
					}
					else
						data = m_display_ram->read_byte(m_cgram_offset + c * 8 + cy);

					for(int cx=0; cx<m_font_size; cx++)
						bitmap.pix(y * 8 + cy, x * m_font_size + cx) = BIT(data, m_font_size - 1 - cx);
				}
			}
	}
	// Graphic layer
	if (BIT(m_display_mode, 3))
	{
		offs = m_graphic_home;
		for(int y=0; y<m_number_lines*8; y++)
			for(int x=0; x<m_graphic_area; x++)
			{
				u8 data = m_display_ram->read_byte(offs++);
				for(int i=0; i<m_font_size; i++)
				{
					int pix = BIT(data, m_font_size - 1 - i);
					switch(m_mode & 7)
					{
					case 0: // OR
						bitmap.pix(y, x * m_font_size + i) |= pix;
						break;
					case 1: // EXOR
						bitmap.pix(y, x * m_font_size + i) ^= pix;
						break;
					case 3: // AND
						bitmap.pix(y, x * m_font_size + i) &= pix;
						break;
					case 4: // Text attribute
						logerror("%s: Unimplemented Text attribute\n", machine().describe_context());
						break;
					}
				}
			}
	}
	return 0;
}


//**************************************************************************
//  T6963C-BASED LCD UNITS
//**************************************************************************

//-------------------------------------------------
//  lm24014h_device - constructor
//-------------------------------------------------

lm24014h_device::lm24014h_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, LM24014H, tag, owner, clock)
	, m_lcdc(*this, "lcdc")
	, m_fs(1)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void lm24014h_device::device_start()
{
	save_item(NAME(m_fs));

	m_lcdc->set_md(4); // 8 lines x 40 columns
	m_lcdc->set_fs(m_fs << 1); // font size 6x8 or 8x8
}


//-------------------------------------------------
//  ram_map - address map for display memory
//-------------------------------------------------

void lm24014h_device::ram_map(address_map &map)
{
	map.global_mask(0x1fff);
	map(0x0000, 0x1fff).ram(); // TC5564AFL-15
}

void lm24014h_device::lcd_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(138, 146, 148));
	palette.set_pen_color(1, rgb_t(69, 62, 66));
}


//-------------------------------------------------
//  device_add_mconfig - add device-specific
//  machine configuration
//-------------------------------------------------

void lm24014h_device::device_add_mconfig(machine_config &config)
{
	T6963C(config, m_lcdc, 0); // XTAL is unknown
	m_lcdc->set_addrmap(0, &lm24014h_device::ram_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(240, 64);
	screen.set_visarea_full();
	screen.set_screen_update("lcdc", FUNC(t6963c_device::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(lm24014h_device::lcd_palette), 2);
}


//-------------------------------------------------
//  lm24014h - ROM definition
//-------------------------------------------------

ROM_START(lm24014h)
	ROM_REGION(0x400, "lcdc:cgrom", 0)
	ROM_LOAD("lm24014w_0101.bin", 0x000, 0x400, CRC(ba5f0719) SHA1(4680d3ae99102369b4145604ee7c25fa83760784))
ROM_END


//-------------------------------------------------
//  device_rom_region - return a pointer to the
//  rom region description for this device
//-------------------------------------------------

const tiny_rom_entry *lm24014h_device::device_rom_region() const
{
	return ROM_NAME(lm24014h);
}
