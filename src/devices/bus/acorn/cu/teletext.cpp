// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Control Universal Teletext Video Interface card

**********************************************************************/

#include "emu.h"
#include "teletext.h"

#include "bus/centronics/ctronics.h"
#include "machine/6522via.h"
#include "sound/beep.h"
#include "video/mc6845.h"
#include "video/saa5050.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class cu_teletext_device : public device_t, public device_acorn_bus_interface
{
public:
	cu_teletext_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, CU_TELETEXT, tag, owner, clock)
		, device_acorn_bus_interface(mconfig, *this)
		, m_videoram(*this, "videoram", 0x400, ENDIANNESS_LITTLE)
		, m_crtc(*this, "mc6845")
		, m_trom(*this, "saa5050")
		, m_via(*this, "via6522")
		, m_option(*this, "OPTION")
		, m_beeper(*this, "beeper")
	{
	}

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	memory_share_creator<uint8_t> m_videoram;
	required_device<mc6845_device> m_crtc;
	required_device<saa5050_device> m_trom;
	required_device<via6522_device> m_via;
	required_ioport m_option;
	required_device<beep_device> m_beeper;

	MC6845_UPDATE_ROW(crtc_update_row);
	void vsync_changed(int state);
	void de_changed(int state);
	void irq_w(int state)
	{
		m_bus->irq_w(state);
	}
};


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START(cu_teletext)
	PORT_START("OPTION")
	PORT_CONFNAME(0x01, 0x00, "Buzzer fitted")
	PORT_CONFSETTING(0x00, DEF_STR(No))
	PORT_CONFSETTING(0x01, DEF_STR(Yes))
INPUT_PORTS_END

ioport_constructor cu_teletext_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(cu_teletext);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void cu_teletext_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(12_MHz_XTAL, 768, 132, 612, 312, 20, 270);
	screen.set_screen_update("mc6845", FUNC(mc6845_device::screen_update));

	PALETTE(config, "palette").set_entries(8);

	HD6845S(config, m_crtc, 6_MHz_XTAL / 3);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(12);
	m_crtc->out_vsync_callback().set(FUNC(cu_teletext_device::vsync_changed));
	m_crtc->out_de_callback().set(FUNC(cu_teletext_device::de_changed));
	m_crtc->set_update_row_callback(FUNC(cu_teletext_device::crtc_update_row));

	SAA5050(config, m_trom, 6_MHz_XTAL);

	MOS6522(config, m_via, DERIVED_CLOCK(1, 1));
	m_via->writepa_handler().set("cent_data_out", FUNC(output_latch_device::write));
	m_via->writepb_handler().set([this](uint8_t data) { m_beeper->set_state(m_option->read() ? BIT(data, 7) : 0); });
	m_via->ca2_handler().set("centronics", FUNC(centronics_device::write_strobe));
	m_via->irq_handler().set(FUNC(cu_teletext_device::irq_w));

	centronics_device &centronics(CENTRONICS(config, "centronics", centronics_devices, "printer"));
	centronics.ack_handler().set(m_via, FUNC(via6522_device::write_ca1));
	centronics.busy_handler().set(m_via, FUNC(via6522_device::write_pb6));
	output_latch_device &latch(OUTPUT_LATCH(config, "cent_data_out"));
	centronics.set_output_latch(latch);

	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 300).add_route(ALL_OUTPUTS, "mono", 0.5); // TODO: unknown frequency
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cu_teletext_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cu_teletext_device::device_reset()
{
	address_space &space = m_bus->memspace();

	space.install_ram(0xd400, 0xd7ff, m_videoram);
	space.install_readwrite_handler(0xd800, 0xd800, emu::rw_delegate(*m_crtc, FUNC(mc6845_device::status_r)), emu::rw_delegate(*m_crtc, FUNC(mc6845_device::address_w)));
	space.install_readwrite_handler(0xd801, 0xd801, emu::rw_delegate(*m_crtc, FUNC(mc6845_device::register_r)), emu::rw_delegate(*m_crtc, FUNC(mc6845_device::register_w)));
	space.install_device(0xd900, 0xd90f, *m_via, &via6522_device::map);

	//m_beeper->set_state(0);
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

MC6845_UPDATE_ROW(cu_teletext_device::crtc_update_row)
{
	uint32_t *p = &bitmap.pix(y);

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

void cu_teletext_device::vsync_changed(int state)
{
	m_trom->dew_w(state);
	m_via->write_cb1(state);
}

void cu_teletext_device::de_changed(int state)
{
	m_via->write_cb2(state);
	m_via->write_pb0(state);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(CU_TELETEXT, device_acorn_bus_interface, cu_teletext_device, "cu_teletext", "Control Universal Teletext Video Interface card")
