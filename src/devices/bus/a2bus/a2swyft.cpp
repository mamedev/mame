// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2swyft.c

    Implementation of the IAI SwyftCard

*********************************************************************/

#include "emu.h"
#include "a2swyft.h"


namespace {

/***************************************************************************
    PARAMETERS
***************************************************************************/

#define SWYFT_ROM_REGION  "swyft_rom"

ROM_START( swyft )
	ROM_REGION(0x4000, SWYFT_ROM_REGION, 0)
	ROM_LOAD( "840-003a.bin", 0x000000, 0x004000, CRC(5d6673e9) SHA1(1554bd03c536789f0ff7d1ef6c992265e311935d) )
	ROM_REGION(0x1000, "pal16r4", 0)
	ROM_LOAD( "swyft_pal16r4.jed", 0x0000, 0x08EF, CRC(462a6938) SHA1(38be885539cf91423a246378c411ac8b2f150ec6) ) // swyft3.pal derived by D. Elvey, works as a replacement pal (original is protected?)
ROM_END

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_swyft_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_swyft_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

protected:
	a2bus_swyft_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_inh_rom(uint16_t offset) override;
	virtual uint16_t inh_start() override { return 0xd000; }
	virtual uint16_t inh_end() override { return 0xffff; }
	virtual int inh_type() override;

private:
	required_region_ptr<uint8_t> m_rom;
	int m_rombank = 0;
	int m_inh_state = 0;
};

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *a2bus_swyft_device::device_rom_region() const
{
	return ROM_NAME( swyft );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_swyft_device::a2bus_swyft_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		a2bus_swyft_device(mconfig, A2BUS_SWYFT, tag, owner, clock)
{
}

a2bus_swyft_device::a2bus_swyft_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
		device_t(mconfig, type, tag, owner, clock),
		device_a2bus_card_interface(mconfig, *this),
		m_rom(*this, SWYFT_ROM_REGION)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_swyft_device::device_start()
{
	save_item(NAME(m_rombank));
}

void a2bus_swyft_device::device_reset()
{
	m_rombank = 0;

	m_inh_state = INH_READ; // read-enable the ROM
	recalc_slot_inh();
}

uint8_t a2bus_swyft_device::read_c0nx(uint8_t offset)
{
	switch (offset)
	{
		case 0:
			m_rombank = 0;
			m_inh_state = INH_READ;
			recalc_slot_inh();
			break;

		case 1:
			m_rombank = 0;
			m_inh_state = INH_NONE;
			recalc_slot_inh();
			break;

		case 2:
			m_rombank = 0x1000;
			m_inh_state = INH_READ;
			recalc_slot_inh();
			break;
	}

	return 0xff;
}

void a2bus_swyft_device::write_c0nx(uint8_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0:
			m_rombank = 0;
			m_inh_state = INH_READ;
			recalc_slot_inh();
			break;

		case 1:
			m_rombank = 0;
			m_inh_state = INH_NONE;
			recalc_slot_inh();
			break;

		case 2:
			m_rombank = 0x1000;
			m_inh_state = INH_READ;
			recalc_slot_inh();
			break;
	}
}

uint8_t a2bus_swyft_device::read_inh_rom(uint16_t offset)
{
	offset -= 0xd000;

	if (offset < 0x1000)    // banked area d000-dfff
	{
		return m_rom[offset + m_rombank];
	}
	else                    // fixed area e000-ffff
	{
		return m_rom[offset - 0x1000 + 0x2000];
	}
}

int a2bus_swyft_device::inh_type()
{
	return m_inh_state;
}

} // anonymous namespace


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_SWYFT, device_a2bus_card_interface, a2bus_swyft_device, "a2swyft", "IAI SwyftCard")
