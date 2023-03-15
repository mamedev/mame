// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
// thanks-to:John Whitworth
/***************************************************************************

    SuperSprite FM+ by DragonPlus Electronics (formerly Dragon MSX 2+)

***************************************************************************/

#include "emu.h"
#include "dragon_msx2.h"
#include "render.h"
#include "screen.h"
#include "speaker.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(DRAGON_MSX2, dragon_msx2_device, "dragon_msx2", "6x09 SuperSprite FM+")


//-------------------------------------------------
//  INPUT_PORTS( dragon_msx2 )
//-------------------------------------------------

INPUT_PORTS_START( dragon_msx2 )
	PORT_START("CONFIG")
	PORT_CONFNAME(0x01, 0x00, "J1 PWR")
	PORT_CONFSETTING(0x00, "Cart Port")
	PORT_CONFSETTING(0x01, "External")
	PORT_CONFNAME(0x02, 0x00, "J2 CART")
	PORT_CONFSETTING(0x00, "N/C")
	PORT_CONFSETTING(0x02, "HSYNC")
	PORT_CONFNAME(0x04, 0x00, "J3 Port Select")
	PORT_CONFSETTING(0x00, "FF7* (Native)")
	PORT_CONFSETTING(0x04, "FF5* (MPI)")
	PORT_CONFNAME(0x08, 0x08, "J4 Default Video")
	PORT_CONFSETTING(0x00, "V9958")
	PORT_CONFSETTING(0x08, "MC6847")
	PORT_CONFNAME(0x10, 0x10, "J5 Video Lock")
	PORT_CONFSETTING(0x00, "Locked")
	PORT_CONFSETTING(0x10, "Unlocked")
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor dragon_msx2_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( dragon_msx2 );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  dragon_msx2_device - constructor
//-------------------------------------------------

dragon_msx2_device::dragon_msx2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, DRAGON_MSX2, tag, owner, clock)
	, device_cococart_interface(mconfig, *this )
	, m_ym2413(*this, "ym2413")
	, m_v9958(*this, "v9958")
	, m_ym2149(*this, "ym2149")
	, m_config(*this, "CONFIG")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dragon_msx2_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void dragon_msx2_device::device_reset()
{
	// pin 26 is tied to GND
	m_ym2149->set_pin26_low_w();

	// Native mode
	if (!BIT(m_config->read(), 2))
	{
		install_readwrite_handler(0xff76, 0xff77, read8sm_delegate(*m_ym2413, FUNC(ym2413_device::read)), write8sm_delegate(*m_ym2413, FUNC(ym2413_device::write)));
		install_readwrite_handler(0xff78, 0xff7b, read8sm_delegate(*m_v9958, FUNC(v9958_device::read)), write8sm_delegate(*m_v9958, FUNC(v9958_device::write)));
		install_write_handler(0xff7c, 0xff7c, write8smo_delegate(*m_ym2149, FUNC(ym2149_device::address_w)));
		install_readwrite_handler(0xff7d, 0xff7d, read8smo_delegate(*m_ym2149, FUNC(ym2149_device::data_r)), write8smo_delegate(*m_ym2149, FUNC(ym2149_device::data_w)));
		install_write_handler(0xff7e, 0xff7e, write8smo_delegate(*this, FUNC(dragon_msx2_device::video_select_w)));
	}

	// Default Video
	machine().render().first_target()->set_view(!BIT(m_config->read(), 3));
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void dragon_msx2_device::device_add_mconfig(machine_config &config)
{
	V9958(config, m_v9958, 21.477272_MHz_XTAL);
	m_v9958->set_screen_pal("screen");
	m_v9958->set_vram_size(0x20000);
	m_v9958->int_cb().set([this](int state) { set_line_value(line::NMI, state); });
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	SPEAKER(config, "speaker").front_center();
	YM2413(config, m_ym2413, 21.477272_MHz_XTAL / 6).add_route(ALL_OUTPUTS, "speaker", 1.0);
	YM2149(config, m_ym2149, 21.477272_MHz_XTAL / 6).add_route(ALL_OUTPUTS, "speaker", 1.0);
}


//-------------------------------------------------
//  scs_read
//-------------------------------------------------

u8 dragon_msx2_device::scs_read(offs_t offset)
{
	u8 data = 0x00;

	if (BIT(m_config->read(), 2)) // MPI mode
	{
		switch (offset)
		{
		case 0x16: case 0x17:
			data = m_ym2413->read(offset & 1);
			break;

		case 0x18: case 0x19: case 0x1a: case 0x1b:
			data = m_v9958->read(offset & 3);
			break;

		case 0x1d:
			data = m_ym2149->data_r();
			break;
		}
	}

	return data;
}

//-------------------------------------------------
//  scs_write
//-------------------------------------------------

void dragon_msx2_device::scs_write(offs_t offset, u8 data)
{
	if (BIT(m_config->read(), 2)) // MPI mode
	{
		switch (offset)
		{
		case 0x16: case 0x17:
			m_ym2413->write(offset & 1, data);
			break;

		case 0x18: case 0x19: case 0x1a: case 0x1b:
			m_v9958->write(offset & 3, data);
			break;

		case 0x1c:
			m_ym2149->address_w(data);
			break;

		case 0x1d:
			m_ym2149->data_w(data);
			break;

		case 0x1e:
			video_select_w(data);
			break;
		}
	}
}

void dragon_msx2_device::video_select_w(u8 data)
{
	if (BIT(m_config->read(), 4)) // Unlocked
		machine().render().first_target()->set_view(!BIT(data, 0));
}
