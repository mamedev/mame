// license:BSD-3-Clause
// copyright-holders:Golden Child
/***************************************************************************

    Epson RX-80 Dot Matrix printer (skeleton)

 Main CPU is a UPD7810 running at 11 MHz.
     8K of mask ROM (marked EPSON M64200CA)
     uses 256 bytes of ram inside upd7810, no external ram chips
     has a limited line buffer of 137 bytes maximum from ff00 to ff88,
     used for character buffer as well as during graphic print operation.
     137 bytes is approximate maximum line length during condensed print.

*****************************************************************************/

#include "emu.h"
#include "epson_rx80.h"

#include "cpu/upd7810/upd7810.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class epson_rx80_device :  public device_t, public device_centronics_peripheral_interface
{
public:
	// construction/destruction
	epson_rx80_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::PRINTER; }

	/* Centronics stuff */
	virtual void input_init(int state) override;
	virtual void input_strobe(int state) override;
	virtual void input_data0(int state) override { if (state) m_centronics_data |= 0x01; else m_centronics_data &= ~0x01; }
	virtual void input_data1(int state) override { if (state) m_centronics_data |= 0x02; else m_centronics_data &= ~0x02; }
	virtual void input_data2(int state) override { if (state) m_centronics_data |= 0x04; else m_centronics_data &= ~0x04; }
	virtual void input_data3(int state) override { if (state) m_centronics_data |= 0x08; else m_centronics_data &= ~0x08; }
	virtual void input_data4(int state) override { if (state) m_centronics_data |= 0x10; else m_centronics_data &= ~0x10; }
	virtual void input_data5(int state) override { if (state) m_centronics_data |= 0x20; else m_centronics_data &= ~0x20; }
	virtual void input_data6(int state) override { if (state) m_centronics_data |= 0x40; else m_centronics_data &= ~0x40; }
	virtual void input_data7(int state) override { if (state) m_centronics_data |= 0x80; else m_centronics_data &= ~0x80; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual bool supports_pin35_5v() override { return true; }

private:
	uint8_t centronics_data_r(offs_t offset) { return m_centronics_data; };

	void epson_rx80_mem(address_map &map) ATTR_COLD;

	required_device<upd7810_device> m_maincpu;

	uint8_t m_centronics_data;
};

//-------------------------------------------------
//  ROM( epson_rx80 )
//-------------------------------------------------

ROM_START( epson_rx80 )
	ROM_REGION(0x2000, "maincpu", 0)  // 8K rom for upd7810
	ROM_LOAD("rx80_2764.bin", 0x0000, 0x2000, CRC(5206104a) SHA1(3e304f5331181aedb321d3db23a9387e3cfacf0c))
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *epson_rx80_device::device_rom_region() const
{
	return ROM_NAME( epson_rx80 );
}


//-------------------------------------------------
//  ADDRESS_MAP( epson_rx80_mem )
//-------------------------------------------------

void epson_rx80_device::epson_rx80_mem(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("maincpu", 0);
	map(0xd800, 0xd800).r(FUNC(epson_rx80_device::centronics_data_r));
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void epson_rx80_device::device_add_mconfig(machine_config &config)
{
	upd7810_device &upd(UPD7810(config, m_maincpu, 11000000)); // 11 Mhz
	upd.set_addrmap(AS_PROGRAM, &epson_rx80_device::epson_rx80_mem);

}


//-------------------------------------------------
//  INPUT_PORTS( epson_rx80 )
//-------------------------------------------------

INPUT_PORTS_START( epson_rx80 )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor epson_rx80_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( epson_rx80 );
}


//-------------------------------------------------
//  epson_rx80_device - constructor
//-------------------------------------------------

epson_rx80_device::epson_rx80_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, EPSON_RX80, tag, owner, clock),
	device_centronics_peripheral_interface(mconfig, *this),
	m_maincpu(*this, "maincpu")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void epson_rx80_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void epson_rx80_device::device_reset()
{
}

/***************************************************************************
    Centronics
***************************************************************************/

void epson_rx80_device::input_strobe(int state)
{
}

void epson_rx80_device::input_init(int state)
{
}

} // anonymous namespace

// GLOBAL
DEFINE_DEVICE_TYPE_PRIVATE(EPSON_RX80, device_centronics_peripheral_interface, epson_rx80_device, "epson_rx80", "Epson RX-80")
