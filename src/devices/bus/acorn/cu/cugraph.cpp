// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Control Universal Colour Graphics card (CU-GRAPH)

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/CU_Graph2.html

**********************************************************************/

#include "emu.h"
#include "cugraph.h"

#include "video/ef9365.h"
#include "screen.h"


namespace {

class cu_graph_device : public device_t, public device_acorn_bus_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::PALETTE | feature::GRAPHICS; }

protected:
	cu_graph_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, type, tag, owner, clock)
		, device_acorn_bus_interface(mconfig, *this)
		, m_gdp(*this, "ef9366")
	{
	}

	//device_t overrides
	virtual void device_start() override ATTR_COLD;

	required_device<ef9365_device> m_gdp;

	void colour_w(offs_t offset, uint8_t data);
	uint8_t pixel_r(offs_t offset);
	void irq_w(int state)
	{
		m_bus->irq_w(state);
	}
};

class cu_graphc_device : public cu_graph_device
{
public:
	cu_graphc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: cu_graph_device(mconfig, CU_GRAPHC, tag, owner, clock)
	{
	}

protected:
	// device_t overrides
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class cu_graphm_device : public cu_graph_device
{
public:
	cu_graphm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: cu_graph_device(mconfig, CU_GRAPHM, tag, owner, clock)
	{
	}

protected:
	// device_t overrides
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void cu_graphm_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_size(512, 312);
	screen.set_visarea(0, 512 - 1, 0, 256 - 1);
	screen.set_refresh_hz(50);
	screen.set_screen_update("ef9366", FUNC(ef9365_device::screen_update));
	PALETTE(config, "palette", palette_device::MONOCHROME_INVERTED);

	EF9365(config, m_gdp, 14_MHz_XTAL / 8); // EF9366
	m_gdp->set_screen("screen");
	m_gdp->set_palette_tag("palette");
	m_gdp->set_nb_bitplanes(1);
	m_gdp->set_display_mode(ef9365_device::DISPLAY_MODE_512x256);
	m_gdp->irq_handler().set(FUNC(cu_graphm_device::irq_w));
}

void cu_graphc_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_size(512, 312);
	screen.set_visarea(0, 512 - 1, 0, 256 - 1);
	screen.set_refresh_hz(50);
	screen.set_screen_update("ef9366", FUNC(ef9365_device::screen_update));
	PALETTE(config, "palette").set_entries(8);

	EF9365(config, m_gdp, 14_MHz_XTAL / 8); // EF9366
	m_gdp->set_screen("screen");
	m_gdp->set_palette_tag("palette");
	m_gdp->set_nb_bitplanes(3);
	m_gdp->set_display_mode(ef9365_device::DISPLAY_MODE_512x256);
	m_gdp->irq_handler().set(FUNC(cu_graphc_device::irq_w));
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cu_graph_device::device_start()
{
	address_space &space = m_bus->memspace();

	space.install_readwrite_handler(0xdf00, 0xdf0f, emu::rw_delegate(*m_gdp, FUNC(ef9365_device::data_r)), emu::rw_delegate(*m_gdp, FUNC(ef9365_device::data_w)));
	space.install_readwrite_handler(0xdf10, 0xdf1f, emu::rw_delegate(*this, FUNC(cu_graph_device::pixel_r)), emu::rw_delegate(*this, FUNC(cu_graph_device::colour_w)));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cu_graphm_device::device_reset()
{
	/* Generate hardware palette */
	for (int i = 0; i<2; i++)
	{
		m_gdp->set_color_entry(i, (i ^ 1) * 255, (i ^ 1) * 255, (i ^ 1) * 255);
	}
}

void cu_graphc_device::device_reset()
{
	/* Generate hardware palette */
	for (int i = 0; i<8; i++)
	{
		m_gdp->set_color_entry(i, ((i & 1) ^ 1) * 255, (((i & 2) >> 1) ^ 1) * 255, (((i & 4) >> 2) ^ 1) * 255);
	}
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void cu_graph_device::colour_w(offs_t offset, uint8_t data)
{
	if (BIT(data, 7))
	{
		// TODO: flash ??
	}
	else
	{
		m_gdp->set_color_filler(data >> 4);
	}
}

uint8_t cu_graph_device::pixel_r(offs_t offset)
{
	uint8_t data = 0x00;

	switch (offset & 0x03)
	{
	case 0: // Red
	case 1: // Green
	case 2: // Blue
		data = m_gdp->get_last_readback_word(offset & 3, 0);
		break;
	case 3: // PPORT
		logerror("cu_graph_device: unhandled read from PPORT\n");
		break;
	}

	return data;
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(CU_GRAPHM, device_acorn_bus_interface, cu_graphm_device, "cu_graphm", "Control Universal High Resolution Graphics card (monochrome)")
DEFINE_DEVICE_TYPE_PRIVATE(CU_GRAPHC, device_acorn_bus_interface, cu_graphc_device, "cu_graphc", "Control Universal High Resolution Graphics card (colour)")
