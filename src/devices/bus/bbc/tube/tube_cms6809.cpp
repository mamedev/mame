// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    CMS 6809 2nd Processor

    Start with *FLEX:
           f0 - OSCLI command
           f1 - display help
       CTRL+F - boot FLEX from ROM
    U <drive> - boot from floppy drive

**********************************************************************/

#include "emu.h"
#include "tube_cms6809.h"

#include "bus/acorn/bus.h"
#include "cpu/m6809/m6809.h"
#include "machine/6522via.h"
#include "machine/input_merger.h"

#include "softlist_dev.h"


namespace {

// ======================> bbc_tube_cms6809_device

class bbc_tube_cms6809_device : public device_t, public device_bbc_tube_interface
{
public:
	bbc_tube_cms6809_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, BBC_TUBE_CMS6809, tag, owner, clock)
		, device_bbc_tube_interface(mconfig, *this)
		, m_maincpu(*this, "maincpu")
		, m_via(*this, "via%u", 0)
		, m_irqs(*this, "irqs")
		, m_bus(*this, "bus")
	{
	}

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual uint8_t host_r(offs_t offset) override;
	virtual void host_w(offs_t offset, uint8_t data) override;

private:
	required_device<cpu_device> m_maincpu;
	required_device_array<via6522_device, 2> m_via;
	required_device<input_merger_device> m_irqs;
	required_device<acorn_bus_device> m_bus;

	void tube_cms6809_mem(address_map &map) ATTR_COLD;
};


//-------------------------------------------------
//  ADDRESS_MAP( tube_cms6809_mem )
//-------------------------------------------------

void bbc_tube_cms6809_device::tube_cms6809_mem(address_map &map)
{
	map(0x0000, 0xffff).ram();
	map(0xef00, 0xef0f).m(m_via[1], FUNC(via6522_device::map));
	map(0xf000, 0xffff).rom().region("boot", 0);
	map(0xfc00, 0xfdff).lrw8(NAME([this](offs_t offset) { return m_bus->read(offset | 0xfc00); }), NAME([this](offs_t offset, uint8_t data) { m_bus->write(offset | 0xfc00, data); }));
}


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START(tube_cms6809)
	ROM_REGION(0x4000, "exp_rom", 0)
	ROM_LOAD("cmsflex_v3.01.rom", 0x0000, 0x4000, CRC(87c7b09f) SHA1(7f2f8666298276713f6035f1dd12d1237cb8a81b))

	ROM_REGION(0x1000, "boot", 0)
	ROM_LOAD("cms6809_v6.rom", 0x0000, 0x1000, CRC(93e3b8f4) SHA1(f431f27941e13a92cdd0a9e2eda891c73d59d835))
ROM_END

const tiny_rom_entry *bbc_tube_cms6809_device::device_rom_region() const
{
	return ROM_NAME( tube_cms6809 );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_tube_cms6809_device::device_add_mconfig(machine_config &config)
{
	MC6809(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &bbc_tube_cms6809_device::tube_cms6809_mem);

	INPUT_MERGER_ANY_HIGH(config, m_irqs).output_handler().set_inputline(m_maincpu, M6809_IRQ_LINE);

	MOS6522(config, m_via[0], 4_MHz_XTAL / 4);
	m_via[0]->writepb_handler().set(m_via[1], FUNC(via6522_device::write_pa));
	m_via[0]->ca2_handler().set(m_via[1], FUNC(via6522_device::write_cb1));
	m_via[0]->cb2_handler().set(m_via[1], FUNC(via6522_device::write_ca1));
	m_via[0]->irq_handler().set(DEVICE_SELF_OWNER, FUNC(bbc_tube_slot_device::irq_w));

	MOS6522(config, m_via[1], 4_MHz_XTAL / 4);
	m_via[1]->writepb_handler().set(m_via[0], FUNC(via6522_device::write_pa));
	m_via[1]->ca2_handler().set(m_via[0], FUNC(via6522_device::write_cb1));
	m_via[1]->cb2_handler().set(m_via[0], FUNC(via6522_device::write_ca1));
	m_via[1]->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<0>));

	/* 7 Slot Backplane */
	ACORN_BUS(config, m_bus, 0);
	m_bus->out_irq_callback().set(m_irqs, FUNC(input_merger_device::in_w<1>));
	m_bus->out_nmi_callback().set_inputline(m_maincpu, M6809_FIRQ_LINE);
	ACORN_BUS_SLOT(config, "bus1", m_bus, cms_bus_devices, nullptr);
	ACORN_BUS_SLOT(config, "bus2", m_bus, cms_bus_devices, nullptr);
	ACORN_BUS_SLOT(config, "bus3", m_bus, cms_bus_devices, nullptr);
	ACORN_BUS_SLOT(config, "bus4", m_bus, cms_bus_devices, nullptr);
	ACORN_BUS_SLOT(config, "bus5", m_bus, cms_bus_devices, nullptr);
	ACORN_BUS_SLOT(config, "bus6", m_bus, cms_bus_devices, nullptr);

	//SOFTWARE_LIST(config, "flop_ls_6809").set_original("bbc_flop_6809").set_filter("CMS");
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_tube_cms6809_device::device_start()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t bbc_tube_cms6809_device::host_r(offs_t offset)
{
	if (offset & 0x10)
		return m_via[0]->read(offset & 0xf);
	else
		return 0xfe;
}

void bbc_tube_cms6809_device::host_w(offs_t offset, uint8_t data)
{
	if (offset & 0x10)
		m_via[0]->write(offset & 0xf, data);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(BBC_TUBE_CMS6809, device_bbc_tube_interface, bbc_tube_cms6809_device, "bbc_tube_cms6809", "CMS 6809 2nd Processor")
