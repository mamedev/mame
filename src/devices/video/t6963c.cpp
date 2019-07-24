// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Toshiba T6963C Dot Matrix LCD Controller
    Sharp LM24014H Dot Matrix LCD Unit

**********************************************************************/

#include "emu.h"
#include "t6963c.h"

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
	, m_cgrom(*this, "cgrom", 0x400)
	, m_display_ram(nullptr)
	, m_data(0)
	, m_adp(0)
	, m_auto_mode(auto_mode::NONE)
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
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void t6963c_device::device_reset()
{
	m_auto_mode = auto_mode::NONE;
}


//-------------------------------------------------
//  read - read from the status or data register
//-------------------------------------------------

u8 t6963c_device::read(offs_t offset)
{
	return BIT(offset, 0) ? 0x0b : 0x00;
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
			logerror("%s: Auto write %02X to %04X\n", machine().describe_context(), data, m_adp);
			m_display_ram->write_byte(m_adp++, data);
		}
		else
			m_data = (u16(data) << 8) | (data >> 8);
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
			logerror("%s: Setting offset register for CG RAM at %04X to %04XH\n",
				machine().describe_context(),
				(m_data & 0x1f) << 11,
				(m_data & 0x1f) << 11 | 0x7ff);
			break;

		case 0x04:
			logerror("%s: Setting address pointer = %04X\n", machine().describe_context(), m_data);
			m_adp = m_data;
			break;

		default:
			logerror("%s: Setting unknown pointer = %04X\n", machine().describe_context(), m_data);
			break;
		}
	}
	else if ((cmd & 0xfd) == 0x40)
	{
		logerror("%s: Setting %s home address = %04X\n", machine().describe_context(),
			BIT(cmd, 1) ? "graphic" : "text",
			m_data);
	}
	else if ((cmd & 0xfd) == 0x41)
	{
		logerror("%s: Setting %s area = %d columns\n", machine().describe_context(),
			BIT(cmd, 1) ? "graphic" : "text",
			m_data);
	}
	else if ((cmd & 0xf0) == 0x80)
	{
		logerror("%s: %s mode, %s\n", machine().describe_context(),
			(cmd & 0x07) == 0x00 ? "OR"
				: (cmd & 0x07) == 0x01 ? "EXOR"
				: (cmd & 0x07) == 0x03 ? "AND"
				: (cmd & 0x07) == 0x04 ? "Text attribute"
				: "Unknown",
			BIT(cmd, 3) ? "external CG RAM" : "internal CG ROM");
	}
	else if ((cmd & 0xf0) == 0x90)
	{
		if (cmd == 0x90)
			logerror("%s: Display off\n", machine().describe_context());
		else
			logerror("%s: Text %s, graphic %s, cursor %s, blink %s\n",
				machine().describe_context(),
				BIT(cmd, 3) ? "on" : "off",
				BIT(cmd, 2) ? "on" : "off",
				BIT(cmd, 1) ? "on" : "off",
				BIT(cmd, 0) ? "on" : "off");
	}
	else if ((cmd & 0xf8) == 0xa0)
	{
		logerror("%s: Select %d-line cursor\n", machine().describe_context(), (cmd & 0x07) + 1);
	}
	else if ((cmd & 0xfe) == 0xb0)
	{
		logerror("%s: Set data auto %s\n", machine().describe_context(), BIT(cmd, 0) ? "read" : "write");
		m_auto_mode = BIT(cmd, 0) ? auto_mode::READ : auto_mode::WRITE;
	}
	else if (cmd == 0xb2)
	{
		logerror("%s: Auto reset\n", machine().describe_context());
		m_auto_mode = auto_mode::NONE;
	}
	else if ((cmd & 0xf0) == 0xc0)
	{
		if (BIT(cmd, 0))
		{
			logerror("%s: Read data from %04X and %s ADP\n", machine().describe_context(),
					BIT(cmd, 0) ? "read" : "write",
					m_adp,
					(cmd & 0x0e) == 0x00 ? "increment"
					: (cmd & 0x0e) == 0x02 ? "decrement"
					: (cmd & 0x0e) == 0x04 ? "nonvariable"
					: "invalid");
		}
		else
		{
			logerror("%s: Write %02X to %04X and %s ADP\n", machine().describe_context(),
					m_data >> 8,
					m_adp,
					(cmd & 0x0e) == 0x00 ? "increment"
					: (cmd & 0x0e) == 0x02 ? "decrement"
					: (cmd & 0x0e) == 0x04 ? "nonvariable"
					: "invalid");

			m_display_ram->write_byte(m_adp, m_data >> 8);
			if ((cmd & 0x0e) == 0x00)
				++m_adp;
			else if ((cmd & 0x0e) == 0x02)
				--m_adp;
		}
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
		logerror("%s: %s bit %d\n", machine().describe_context(),
			BIT(cmd, 3) ? "Set" : "Reset",
			cmd & 0x07);
	}
	else
	{
		logerror("%s: Unknown command %02X\n", machine().describe_context(), cmd);
	}
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
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void lm24014h_device::device_start()
{
}


//-------------------------------------------------
//  ram_map - address map for display memory
//-------------------------------------------------

void lm24014h_device::ram_map(address_map &map)
{
	map.global_mask(0x1fff);
	map(0x0000, 0x1fff).ram(); // TC5564AFL-15
}


//-------------------------------------------------
//  device_add_mconfig - add device-specific
//  machine configuration
//-------------------------------------------------

void lm24014h_device::device_add_mconfig(machine_config &config)
{
	T6963C(config, m_lcdc, 0); // XTAL is unknown
	m_lcdc->set_addrmap(0, &lm24014h_device::ram_map);
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
