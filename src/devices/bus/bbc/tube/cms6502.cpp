// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    CMS 6502 2nd Processor

**********************************************************************/

#include "emu.h"
#include "cms6502.h"

#include "cpu/m6502/m6502.h"
#include "machine/6522via.h"
#include "machine/m3002.h"


namespace {

class bbc_cms6502_device : public device_t, public device_bbc_tube_interface
{
public:
	bbc_cms6502_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, BBC_CMS6502, tag, owner, clock)
		, device_bbc_tube_interface(mconfig, *this)
		, m_maincpu(*this, "maincpu")
		, m_via(*this, "via%u", 0U)
	{
	}

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD { }

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual uint8_t host_r(offs_t offset) override;
	virtual void host_w(offs_t offset, uint8_t data) override;

private:
	void cms6502_mem(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device_array<via6522_device, 2> m_via;
};


//-------------------------------------------------
//  ADDRESS_MAP( cms6502_mem )
//-------------------------------------------------

void bbc_cms6502_device::cms6502_mem(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x8000, 0xbfff).rom().region("rom", 0);
	map(0xc000, 0xffff).rom().region("mos", 0);
	map(0xfc00, 0xfc0f).m(m_via[1], FUNC(via6522_device::map));
	map(0xfc30, 0xfc30).mirror(0xf).rw("rtc", FUNC(m3002_device::read), FUNC(m3002_device::write));
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( cms6502 )
	ROM_REGION(0x2000, "exp_rom", 0)
	ROM_LOAD("cmslink.rom", 0x0000, 0x2000, CRC(f3055016) SHA1(619467dc58414089190cbde76a45129d9dfd188e))

	ROM_REGION(0x4000, "mos", 0)
	ROM_LOAD("cms6502_1.19.rom", 0x0000, 0x4000, CRC(e59d30fd) SHA1(835dbb0a8906aaa7ea890f37dc09e9f28e208125))

	ROM_REGION(0x4000, "rom", 0)
	ROM_LOAD("basic2.rom", 0x0000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281))
ROM_END

const tiny_rom_entry *bbc_cms6502_device::device_rom_region() const
{
	return ROM_NAME( cms6502 );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_cms6502_device::device_add_mconfig(machine_config &config)
{
	M6502(config, m_maincpu, 1_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &bbc_cms6502_device::cms6502_mem);

	MOS6522(config, m_via[0], 1_MHz_XTAL);
	m_via[0]->writepb_handler().set(m_via[1], FUNC(via6522_device::write_pa));
	m_via[0]->ca2_handler().set(m_via[1], FUNC(via6522_device::write_cb1));
	m_via[0]->cb2_handler().set(m_via[1], FUNC(via6522_device::write_ca1));
	m_via[0]->irq_handler().set(DEVICE_SELF_OWNER, FUNC(bbc_tube_slot_device::irq_w));

	MOS6522(config, m_via[1], 1_MHz_XTAL);
	m_via[1]->writepb_handler().set(m_via[0], FUNC(via6522_device::write_pa));
	m_via[1]->ca2_handler().set(m_via[0], FUNC(via6522_device::write_cb1));
	m_via[1]->cb2_handler().set(m_via[0], FUNC(via6522_device::write_ca1));
	m_via[1]->irq_handler().set_inputline(m_maincpu, M6502_IRQ_LINE);

	M3002(config, "rtc", 32.768_kHz_XTAL);
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t bbc_cms6502_device::host_r(offs_t offset)
{
	if (offset & 0x10)
		return m_via[0]->read(offset & 0xf);
	else
		return 0xfe;
}

void bbc_cms6502_device::host_w(offs_t offset, uint8_t data)
{
	if (offset & 0x10)
		m_via[0]->write(offset & 0xf, data);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(BBC_CMS6502, device_bbc_tube_interface, bbc_cms6502_device, "bbc_cms6502", "CMS 6502 2nd Processor")
