// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Prism VTX 5000 Modem

**********************************************************************/

#include "emu.h"
#include "vtx5000.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "bus/rs232/rs232.h"


namespace {

class spectrum_vtx5000_device : public device_t, public device_spectrum_expansion_interface
{
public:
	spectrum_vtx5000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, SPECTRUM_VTX5000, tag, owner, clock)
		, device_spectrum_expansion_interface(mconfig, *this)
		, m_exp(*this, "exp")
		, m_usart(*this, "i8251")
		, m_rom(*this, "rom")
	{
	}

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual uint8_t mreq_r(offs_t offset) override;
	virtual uint8_t iorq_r(offs_t offset) override;
	virtual void iorq_w(offs_t offset, uint8_t data) override;
	virtual bool romcs() override;

	// passthru
	virtual void pre_opcode_fetch(offs_t offset) override { m_exp->pre_opcode_fetch(offset); }
	virtual void post_opcode_fetch(offs_t offset) override { m_exp->post_opcode_fetch(offset); }
	virtual void pre_data_fetch(offs_t offset) override { m_exp->pre_data_fetch(offset); }
	virtual void post_data_fetch(offs_t offset) override { m_exp->post_data_fetch(offset); }
	virtual void mreq_w(offs_t offset, uint8_t data) override { if (m_exp->romcs()) m_exp->mreq_w(offset, data); }

private:
	required_device<spectrum_expansion_slot_device> m_exp;
	required_device<i8251_device> m_usart;
	required_memory_region m_rom;

	bool m_romcs;
};


ROM_START(vtx5000)
	ROM_REGION( 0x2000, "rom", 0 )
	ROM_SYSTEM_BIOS(0, "v31mnet", "V3.1 Micronet")
	ROMX_LOAD("vtx5000-3-1.rom", 0x0000, 0x2000, CRC(1431505c) SHA1(42ae2b47c44d8c0d6a6c6a2b39a4a25a6d3a4bda), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "v31hlink", "V3.1 Homelink")
	ROMX_LOAD("homelink.rom", 0x0000, 0x2000, CRC(527d7f32) SHA1(4490186845b198ca9191dd711bd38274cba6f04b), ROM_BIOS(1))
ROM_END


const tiny_rom_entry *spectrum_vtx5000_device::device_rom_region() const
{
	return ROM_NAME(vtx5000);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void spectrum_vtx5000_device::device_add_mconfig(machine_config &config)
{
	I8251(config, m_usart, 2.4576_MHz_XTAL);
	m_usart->txd_handler().set("modem", FUNC(rs232_port_device::write_txd));
	m_usart->dtr_handler().set("modem", FUNC(rs232_port_device::write_dtr));
	m_usart->rts_handler().set([this](int state) { m_romcs = state; });

	clock_device &rx_clock(CLOCK(config, "rx_clock", 2.4576_MHz_XTAL / 16 / 2));  // 1200 baud
	rx_clock.signal_handler().set(m_usart, FUNC(i8251_device::write_rxc));

	clock_device &tx_clock(CLOCK(config, "tx_clock", 2.4576_MHz_XTAL / 16 / 32)); // 75 baud
	tx_clock.signal_handler().set(m_usart, FUNC(i8251_device::write_txc));

	rs232_port_device &rs232(RS232_PORT(config, "modem", default_rs232_devices, nullptr)); // TCM3101 (internal modem)
	rs232.rxd_handler().set(m_usart, FUNC(i8251_device::write_rxd));
	rs232.dcd_handler().set(m_usart, FUNC(i8251_device::write_dsr)).invert();

	// passthru
	SPECTRUM_EXPANSION_SLOT(config, m_exp, spectrum_expansion_devices, nullptr);
	m_exp->irq_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::irq_w));
	m_exp->nmi_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::nmi_w));
	m_exp->fb_r_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::fb_r));
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void spectrum_vtx5000_device::device_start()
{
	// CTS grounded
	m_usart->write_cts(0);

	save_item(NAME(m_romcs));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void spectrum_vtx5000_device::device_reset()
{
	m_romcs = 1;
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

bool spectrum_vtx5000_device::romcs()
{
	return m_romcs || m_exp->romcs();
}


uint8_t spectrum_vtx5000_device::mreq_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (m_romcs)
		data = m_rom->base()[offset & 0x1fff];

	if (m_exp->romcs())
		data &= m_exp->mreq_r(offset);

	return data;
}


uint8_t spectrum_vtx5000_device::iorq_r(offs_t offset)
{
	uint8_t data = m_exp->iorq_r(offset);

	if ((offset & 0x7f) == 0x7f)
		data = m_usart->read(BIT(offset, 7));

	return data;
}

void spectrum_vtx5000_device::iorq_w(offs_t offset, uint8_t data)
{
	if ((offset & 0x7f) == 0x7f)
		m_usart->write(BIT(offset, 7), data);

	m_exp->iorq_w(offset, data);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(SPECTRUM_VTX5000, device_spectrum_expansion_interface, spectrum_vtx5000_device, "spectrum_vtx5000", "Prism VTX 5000 Modem")
