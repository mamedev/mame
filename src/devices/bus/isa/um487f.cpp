// license:BSD-3-Clause
// copyright-holders:Tomás García-Merás Capote
/***************************************************************************

    Hercules + CGA card based on the UMC UM487F, as in the application
    circuit of its datasheet: 64 KiB of DRAM, a UM2310 character generator
    and a UM82C11 printer controller.

    SW1 selects the display mode and SW2 enables the printer port. SW3 feeds
    SW1 from the reset signal, which allows software to change the display
    mode, or from +5V, which locks it.

***************************************************************************/

#include "emu.h"
#include "um487f.h"

#include "cga.h"

#include "machine/pc_lpt.h"
#include "video/um487f.h"

#include "screen.h"


namespace {

ROM_START( isa_um487f )
	ROM_REGION( 0x2000, "chargen", 0 )
	// TODO: the UM2310 character generator isn't dumped. This IBM ROM has the same layout, but the
	// UM2310 datasheet shows a different CGA font for A11 = 0 and some different MGA characters.
	ROM_LOAD( "5788005.u33", 0x0000, 0x2000, BAD_DUMP CRC(0bf56d70) SHA1(c2a8b10808bf51a3c123ba3eb1e9dd608231916f) )
ROM_END

INPUT_PORTS_START( isa_um487f )
	PORT_START("JUMPERS")
	PORT_CONFNAME( 0x01, 0x00, "Display Mode (SW1)" )
	PORT_CONFSETTING(    0x00, "CGA" )
	PORT_CONFSETTING(    0x01, "MGA" )
	PORT_CONFNAME( 0x02, 0x02, "Printer Port (SW2)" )
	PORT_CONFSETTING(    0x00, DEF_STR( No ) )
	PORT_CONFSETTING(    0x02, DEF_STR( Yes ) )
	PORT_CONFNAME( 0x04, 0x04, "Software Display Mode Change (SW3)" )
	PORT_CONFSETTING(    0x00, DEF_STR( No ) )
	PORT_CONFSETTING(    0x04, DEF_STR( Yes ) )
INPUT_PORTS_END


class isa8_um487f_device : public device_t, public device_isa8_card_interface
{
public:
	isa8_um487f_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// the stand-in character generator is stock CGA's, so look for it under that device's name
	static auto parent_rom_device_type() { return &ISA8_CGA; }

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_reset_after_children() override ATTR_COLD;

	// device_isa8_card_interface implementation
	virtual void remap(int space_id, offs_t start, offs_t end) override;

private:
	void io_map(address_map &map) ATTR_COLD;

	required_device<um487f_device> m_video;
	required_device<pc_lpt_device> m_lpt;
	required_ioport m_jumpers;
};


isa8_um487f_device::isa8_um487f_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ISA8_UM487F, tag, owner, clock)
	, device_isa8_card_interface(mconfig, *this)
	, m_video(*this, "um487f")
	, m_lpt(*this, "lpt")
	, m_jumpers(*this, "JUMPERS")
{
}

void isa8_um487f_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen"));
	screen.set_raw(14.318181_MHz_XTAL, 912, 0, 640, 262, 0, 200); // the UM487F reconfigures it from its CRTC registers
	screen.set_screen_update(m_video, FUNC(um487f_device::screen_update));

	UM487F(config, m_video, 14.318181_MHz_XTAL);
	m_video->set_screen("screen");
	m_video->set_mga_clock(16.257_MHz_XTAL);
	m_video->set_chargen("chargen");

	PC_LPT(config, m_lpt);
	m_lpt->irq_handler().set([this] (int state) { m_isa->irq7_w(state); });
}

const tiny_rom_entry *isa8_um487f_device::device_rom_region() const
{
	return ROM_NAME( isa_um487f );
}

ioport_constructor isa8_um487f_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( isa_um487f );
}

void isa8_um487f_device::device_start()
{
	set_isa_device();
}

void isa8_um487f_device::device_reset()
{
	uint8_t const jumpers = m_jumpers->read();
	m_video->sws_w(BIT(~jumpers, 0));
	m_video->swr_w(BIT(jumpers, 0));

	remap(AS_PROGRAM, 0, 0xfffff);
	remap(AS_IO, 0, 0xffff);
}

void isa8_um487f_device::device_reset_after_children()
{
	if (BIT(m_jumpers->read(), 2))
	{
		m_video->sws_w(0);
		m_video->swr_w(0);
	}
}

void isa8_um487f_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_PROGRAM)
	{
		m_isa->install_memory(0xb0000, 0xbffff, read8sm_delegate(*m_video, FUNC(um487f_device::mem_r)), write8sm_delegate(*m_video, FUNC(um487f_device::mem_w)));
	}
	else if (space_id == AS_IO)
	{
		m_isa->install_device(0x03b0, 0x03df, *this, &isa8_um487f_device::io_map);

		if (BIT(m_jumpers->read(), 1))
			m_isa->install_device(0x03bc, 0x03be, read8sm_delegate(*m_lpt, FUNC(pc_lpt_device::read)), write8sm_delegate(*m_lpt, FUNC(pc_lpt_device::write)));
	}
}

void isa8_um487f_device::io_map(address_map &map)
{
	map(0x00, 0x0f).rw(m_video, FUNC(um487f_device::io_r), FUNC(um487f_device::io_w));
	map(0x20, 0x2f).lrw8(
			NAME([this] (offs_t offset) -> uint8_t { return m_video->io_r(0x20 + offset); }),
			NAME([this] (offs_t offset, uint8_t data) { m_video->io_w(0x20 + offset, data); }));
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(ISA8_UM487F, device_isa8_card_interface, isa8_um487f_device, "isa_um487f", "UMC UM487F HCGA card")
