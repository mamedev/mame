// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Mertec Compact Companion

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Mertec_CompactComp.html

    TODO:
    - Fix userport, by somehow passing lines into joyport
    - Not sure whether any 1MHz bus devices should work on the 2MHz bus

**********************************************************************/

#include "emu.h"
#include "mertec.h"

#include "bus/bbc/1mhzbus/1mhzbus.h"
#include "bus/bbc/analogue/analogue.h"
#include "bus/bbc/userport/userport.h"
#include "machine/6821pia.h"
#include "machine/upd7002.h"


namespace {

class bbc_mertec_device : public device_t, public device_bbc_exp_interface
{
public:
	bbc_mertec_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, BBC_MERTEC, tag, owner, clock)
		, device_bbc_exp_interface(mconfig, *this)
		, m_pia(*this, "pia")
		, m_upd7002(*this, "upd7002")
		, m_analog(*this, "analogue")
		, m_userport(*this, "userport")
		, m_2mhzbus(*this, "2mhzbus")
		, m_rom(*this, "rom")
	{
	}

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD { }

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual uint8_t fred_r(offs_t offset) override;
	virtual void fred_w(offs_t offset, uint8_t data) override;
	virtual uint8_t jim_r(offs_t offset) override;
	virtual void jim_w(offs_t offset, uint8_t data) override;
	virtual uint8_t rom_r(offs_t offset) override;
	virtual void rom_w(offs_t offset, uint8_t data) override;

	virtual uint8_t pb_r() override;
	virtual void pb_w(uint8_t data) override;

private:
	required_device<pia6821_device> m_pia;
	required_device<upd7002_device> m_upd7002;
	required_device<bbc_analogue_slot_device> m_analog;
	required_device<bbc_userport_slot_device> m_userport;
	required_device<bbc_1mhzbus_slot_device> m_2mhzbus;
	required_region_ptr<uint8_t> m_rom;

	uint8_t adc_ctrl_r();
	void adc_ctrl_w(uint8_t data);

	uint8_t m_adc_ctrl = 0;
	uint8_t m_adc_data = 0;
};


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START(mertec)
	ROM_REGION(0x8000, "rom", 0)
	ROM_LOAD("mertec-companion-v0.99.rom", 0x0000, 0x8000, CRC(af8ff8d7) SHA1(0c4017ffbb480168e54c6b153da257ec5ea29d4e))
ROM_END

const tiny_rom_entry *bbc_mertec_device::device_rom_region() const
{
	return ROM_NAME(mertec);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_mertec_device::device_add_mconfig(machine_config &config)
{
	PIA6821(config, m_pia, DERIVED_CLOCK(1, 8));
	m_pia->readpa_handler().set(FUNC(bbc_mertec_device::adc_ctrl_r));
	m_pia->writepa_handler().set(FUNC(bbc_mertec_device::adc_ctrl_w));
	m_pia->readpb_handler().set([this]() { return m_adc_data; });
	m_pia->writepb_handler().set([this](uint8_t data) { m_adc_data = data; });
	m_pia->irqb_handler().set(DEVICE_SELF_OWNER, FUNC(bbc_exp_slot_device::irq_w));

	UPD7002(config, m_upd7002, DERIVED_CLOCK(1, 8));
	m_upd7002->get_analogue_callback().set(m_analog, FUNC(bbc_analogue_slot_device::ch_r));
	m_upd7002->eoc_callback().set(m_pia, FUNC(pia6821_device::cb1_w));

	// analogue port
	BBC_ANALOGUE_SLOT(config, m_analog, bbc_analogue_devices, nullptr);
	//m_analog->lpstb_handler().set(DEVICE_SELF_OWNER, FUNC(bbc_exp_slot_device::lpstb_w));

	// user port
	BBC_USERPORT_SLOT(config, m_userport, bbc_userport_devices, nullptr);
	m_userport->cb1_handler().set(DEVICE_SELF_OWNER, FUNC(bbc_exp_slot_device::cb1_w));
	m_userport->cb2_handler().set(DEVICE_SELF_OWNER, FUNC(bbc_exp_slot_device::cb2_w));

	// 2mhz bus port
	BBC_1MHZBUS_SLOT(config, m_2mhzbus, DERIVED_CLOCK(1, 4), bbc_1mhzbus_devices, nullptr);
	m_2mhzbus->irq_handler().set(DEVICE_SELF_OWNER, FUNC(bbc_exp_slot_device::irq_w));
	m_2mhzbus->nmi_handler().set(DEVICE_SELF_OWNER, FUNC(bbc_exp_slot_device::nmi_w));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t bbc_mertec_device::adc_ctrl_r()
{
	return (m_adc_ctrl & 0x1f) | (m_analog->pb_r() << 1);
}

void bbc_mertec_device::adc_ctrl_w(uint8_t data)
{
	m_adc_ctrl = data & 0x1f;

	if (!BIT(data, 4)) // CS
	{
		switch (data & 0x0c)
		{
		case 0x04: m_upd7002->write(data & 3, m_adc_data); break;
		case 0x08: m_adc_data = m_upd7002->read(data & 3); break;
		}
	}

	m_analog->pb_w(data >> 1);
}

uint8_t bbc_mertec_device::fred_r(offs_t offset)
{
	return m_2mhzbus->fred_r(offset);
}

void bbc_mertec_device::fred_w(offs_t offset, uint8_t data)
{
	m_2mhzbus->fred_w(offset, data);
}

uint8_t bbc_mertec_device::jim_r(offs_t offset)
{
	return m_2mhzbus->jim_r(offset);
}

void bbc_mertec_device::jim_w(offs_t offset, uint8_t data)
{
	m_2mhzbus->jim_w(offset, data);
}

uint8_t bbc_mertec_device::rom_r(offs_t offset)
{
	if ((offset & 0x6000) == 0x6000)
		return m_pia->read(offset);
	else
		return m_rom[offset];
}

void bbc_mertec_device::rom_w(offs_t offset, uint8_t data)
{
	if ((offset & 0x6000) == 0x6000)
		m_pia->write(offset, data);
}

uint8_t bbc_mertec_device::pb_r()
{
	return m_userport->pb_r();
}

void bbc_mertec_device::pb_w(uint8_t data)
{
	m_userport->pb_w(data);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(BBC_MERTEC, device_bbc_exp_interface, bbc_mertec_device, "bbc_mertec", "Mertec Compact Companion");
