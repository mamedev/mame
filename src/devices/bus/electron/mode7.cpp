// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    JAFA Mode 7 Display Unit

    TODO:
      The SAA5050 output is currently being sent to a separate screen, it
      should be output to the same screen as the Electron.
      The Electron video output is 640x312 whereas the SAA5050 output is
      480x500 (doubled vertically to account for interlace). Only one
      output is ever active at any time, which is selected by the MA13
      line from the HD6845.

**********************************************************************/


#include "emu.h"
#include "mode7.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ELECTRON_MODE7, electron_mode7_device, "electron_mode7", "JAFA Mode 7 Display Unit")


//-------------------------------------------------
//  ROM( mode7 )
//-------------------------------------------------

ROM_START( mode7 )
	ROM_REGION(0x2000, "exp_rom", 0)
	ROM_SYSTEM_BIOS(0, "191", "Mode 7 OS v1.91")
	ROMX_LOAD("mode7_1.91.rom", 0x0000, 0x2000, CRC(1863bfe6) SHA1(a18c12c8e4bcd5284aac5b0f0a33dc040176084a), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "15", "Mode 7 OS v1.5")
	ROMX_LOAD("mode7_1.5.rom", 0x0000, 0x2000, CRC(eeea69ee) SHA1(3689a6ca8693fa9a6e53a02cf34d613303cb34d7), ROM_BIOS(1))
ROM_END

//-------------------------------------------------
//  INPUT_PORTS( mode7 )
//-------------------------------------------------

static INPUT_PORTS_START( mode7 )
	PORT_START("BUTTON")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Mode 7 Break") PORT_CODE(KEYCODE_HOME) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(electron_mode7_device::break_button), 0)
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor electron_mode7_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( mode7 );
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void electron_mode7_device::device_add_mconfig(machine_config &config)
{
	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(16_MHz_XTAL, 768, 0, 480, 622, 0, 500);
	m_screen->set_screen_update("hd6845", FUNC(hd6845s_device::screen_update));

	HD6845S(config, m_hd6845, DERIVED_CLOCK(1, 8)); // FIXME: double clock until interlace is implemented
	m_hd6845->set_screen("screen");
	m_hd6845->set_show_border_area(false);
	m_hd6845->set_char_width(12);
	m_hd6845->set_update_row_callback(FUNC(electron_mode7_device::crtc_update_row));
	m_hd6845->out_vsync_callback().set(FUNC(electron_mode7_device::vsync_changed));

	SAA5050(config, m_trom, 6_MHz_XTAL);

	/* pass-through */
	ELECTRON_EXPANSION_SLOT(config, m_exp, DERIVED_CLOCK(1, 1), electron_expansion_devices, nullptr);
	m_exp->irq_handler().set(DEVICE_SELF_OWNER, FUNC(electron_expansion_slot_device::irq_w));
	m_exp->nmi_handler().set(DEVICE_SELF_OWNER, FUNC(electron_expansion_slot_device::nmi_w));
}

const tiny_rom_entry *electron_mode7_device::device_rom_region() const
{
	return ROM_NAME( mode7 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  electron_mode7_device - constructor
//-------------------------------------------------

electron_mode7_device::electron_mode7_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ELECTRON_MODE7, tag, owner, clock)
	, device_electron_expansion_interface(mconfig, *this)
	, m_screen(*this, "screen")
	, m_hd6845(*this, "hd6845")
	, m_trom(*this, "saa5050")
	, m_exp(*this, "exp")
	, m_exp_rom(*this, "exp_rom")
	, m_romsel(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void electron_mode7_device::device_start()
{
	m_videoram = make_unique_clear<uint8_t[]>(0x800);

	/* register for save states */
	save_pointer(NAME(m_videoram), 0x800);
	save_item(NAME(m_romsel));
}

//-------------------------------------------------
//  expbus_r - expansion data read
//-------------------------------------------------

uint8_t electron_mode7_device::expbus_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (offset >= 0x7c00 && offset < 0x8000)
	{
		data = m_videoram[offset & 0x3ff];
	}
	else if (offset >= 0x8000 && offset < 0xc000)
	{
		if (m_romsel == 15)
		{
			data = m_exp_rom->base()[offset & 0x1fff];
		}
	}
	else
	{
		switch (offset)
		{
		case 0xfc1e:
			data = m_hd6845->status_r();
			break;
		case 0xfc1f:
			data = m_hd6845->register_r();
			break;
		}
	}

	data &= m_exp->expbus_r(offset);

	return data;
}

//-------------------------------------------------
//  expbus_w - expansion data write
//-------------------------------------------------

void electron_mode7_device::expbus_w(offs_t offset, uint8_t data)
{
	if (offset >= 0x7c00 && offset < 0x8000)
	{
		m_videoram[offset & 0x3ff] = data & 0x7f;
	}
	else
	{
		switch (offset)
		{
		case 0xfc1c:
			m_hd6845->address_w(data);
			break;
		case 0xfc1d:
			m_hd6845->register_w(data);
			break;

		case 0xfe05:
			if ((data & 0xf0) != 0xf0)
				m_romsel = data & 0x0f;
			break;
		}
	}

	m_exp->expbus_w(offset, data);
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

MC6845_UPDATE_ROW(electron_mode7_device::crtc_update_row)
{
	uint32_t* p = &bitmap.pix(y);

	m_trom->lose_w(1);
	m_trom->lose_w(0);

	for (int column = 0; column < x_count; column++)
	{
		m_trom->write(m_videoram[(ma + column) & 0x3ff]);

		m_trom->f1_w(1);
		m_trom->f1_w(0);

		for (int bit = 0; bit < 12; bit++)
		{
			m_trom->tr6_w(1);
			m_trom->tr6_w(0);

			int col = m_trom->get_rgb() ^ ((column == cursor_x) ? 7 : 0);

			int r = BIT(col, 0) * 0xff;
			int g = BIT(col, 1) * 0xff;
			int b = BIT(col, 2) * 0xff;

			*p++ = rgb_t(r, g, b);
		}
	}
}

void electron_mode7_device::vsync_changed(int state)
{
	m_trom->dew_w(state);
}


INPUT_CHANGED_MEMBER(electron_mode7_device::break_button)
{
	m_slot->nmi_w(!newval);
}
