// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn AKA25 Ethernet

    http://chrisacorns.computinghistory.org.uk/32bit_UpgradesA2G/Acorn_AKA25_Ethernet.html

**********************************************************************/

#include "emu.h"
#include "ether1.h"
#include "machine/i82586.h"

#include "multibyte.h"


namespace {

class arc_ether1_aka25_device : public device_t, public device_archimedes_podule_interface
{
public:
	// construction/destruction
	arc_ether1_aka25_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_archimedes_podule_interface overrides
	virtual void ioc_map(address_map &map) override ATTR_COLD;

private:
	required_memory_region m_podule_rom;
	required_shared_ptr<u16> m_ram;
	required_device<i82586_device> m_lance;

	void lan_map(address_map &map) ATTR_COLD;

	void checksum();
	void control_w(u8 data);

	u8 m_rom_page;
	u8 m_ram_page;
};


void arc_ether1_aka25_device::ioc_map(address_map &map)
{
	map(0x0000, 0x003f).mirror(0x1fc0).lr8(NAME([this](offs_t offset) { return m_podule_rom->base()[offset | ((m_rom_page << 4) & 0x1f)]; })).umask32(0x000000ff);
	map(0x0000, 0x0000).lw8(NAME([this](u8 data) { m_ram_page = data & 0x0f; }));
	map(0x0004, 0x0004).w(FUNC(arc_ether1_aka25_device::control_w));
	map(0x2000, 0x3fff).lrw16(NAME([this](offs_t offset) { return m_ram[offset | (m_ram_page << 11)]; }), NAME([this](offs_t offset, u16 data) { m_ram[offset | (m_ram_page << 11)] = data; })).umask32(0x0000ffff);
}


void arc_ether1_aka25_device::lan_map(address_map &map)
{
	map(0x000000, 0x00ffff).mirror(0xff0000).ram().share("ram");
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void arc_ether1_aka25_device::device_add_mconfig(machine_config &config)
{
	I82586(config, m_lance, 20_MHz_XTAL / 2);
	m_lance->out_irq_cb().set([this](int state) { m_rom_page = state; set_pirq(state); });
	m_lance->set_addrmap(0, &arc_ether1_aka25_device::lan_map);
}


//-------------------------------------------------
//  ROM( ether1_aka25 )
//-------------------------------------------------

ROM_START( ether1_aka25 )
	ROM_REGION(0x20, "podule_rom", 0)
	ROM_LOAD("27ls19.bin", 0x00, 0x20, CRC(e034ae68) SHA1(9a9404028d166d9cbb466513f514be5d3ea77956))
ROM_END

const tiny_rom_entry *arc_ether1_aka25_device::device_rom_region() const
{
	return ROM_NAME( ether1_aka25 );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  arc_ether1_aka25_device - constructor
//-------------------------------------------------

arc_ether1_aka25_device::arc_ether1_aka25_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ARC_ETHER1_AKA25, tag, owner, clock)
	, device_archimedes_podule_interface(mconfig, *this)
	, m_podule_rom(*this, "podule_rom")
	, m_ram(*this, "ram")
	, m_lance(*this, "lance")
	, m_rom_page(0)
	, m_ram_page(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void arc_ether1_aka25_device::device_start()
{
	checksum();

	save_item(NAME(m_rom_page));
	save_item(NAME(m_ram_page));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void arc_ether1_aka25_device::device_reset()
{
	m_rom_page = 0x00;
	m_ram_page = 0x00;
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void arc_ether1_aka25_device::checksum()
{
	// PROM CRC calculation (from A500/R200 Service Manual)
	u32 chk = 0xffffffff;

	for (int i = 0; i < 28; i++)
	{
		// CRC on bytes 0..28
		u8 byte = m_podule_rom->base()[i];
		for (int j = 0; j < 8; j++)
		{
			if (((byte & 1) ^ (chk >> 31)) != 0)
				chk = (chk << 1) ^ (0x04c11db7);
			else
				chk = (chk << 1);

			byte = byte >> 1;
		}
	}

	// get CRC from PROM
	u32 checksum = get_u32le(&m_podule_rom->base()[28]);

	// test to see if the same
	logerror("checksum: %08x %s\n", chk, checksum == chk ? "Pass" : "Fail");
}

void arc_ether1_aka25_device::control_w(u8 data)
{
	m_lance->reset_w(BIT(data, 0)); // b0 Reset
	//m_sia->lpbk(BIT(data, 1));    // b1 Loop-Back
	m_lance->ca(BIT(data, 2));      // b2 Channel Attention
	if (BIT(data, 3)) set_pirq(0);  // b3 Clear Interrupt
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(ARC_ETHER1_AKA25, device_archimedes_podule_interface, arc_ether1_aka25_device, "arc_ether1_aka25", "Acorn AKA25 Ethernet")
