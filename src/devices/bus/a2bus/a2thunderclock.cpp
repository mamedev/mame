// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2thunderclock.c

    Implemention of the Thunderware Thunderclock Plus.


    PCB Layout: (B1/B2 are batteries)
     _______________________________________________________________
    |                                                               |
    |    | | | |           uPD1990 CD4050 74LS174   74LS132         |
    |    | | | |              _      _     _     _     _            |
    |    | | | |             | |    | |   | |   | |   | |           |
    |    | | | |             |_|    |_|   |_|   |_|   |_|           |
    |    B1  B2                               74LS08                |
    |           74LS74  74LS109 74LS126        ____________  74LS27 |
    |               _      _     _            |            |    _   |
    |              | |    | |   | |           |   2716     |   | |  |
    |              |_|    |_|   |_|           |____________|   |_|  |
    |______________________________________                        _|
                                           |                      |
                                           |______________________|

*********************************************************************/

#include "emu.h"
#include "a2thunderclock.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(A2BUS_THUNDERCLOCK, a2bus_thunderclock_device, "a2thunpl", "ThunderWare ThunderClock Plus")

#define THUNDERCLOCK_ROM_REGION  "thunclk_rom"
#define THUNDERCLOCK_UPD1990_TAG "thunclk_upd"

ROM_START( thunderclock )
	ROM_REGION(0x800, THUNDERCLOCK_ROM_REGION, 0)
	ROM_LOAD( "thunderclock plus rom.bin", 0x0000, 0x0800, CRC(1b99c4e3) SHA1(60f434f5325899d7ea257a6e56e6f53eae65146a) )
ROM_END

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void a2bus_thunderclock_device::device_add_mconfig(machine_config &config)
{
	UPD1990A(config, m_upd1990ac, 1021800);
	m_upd1990ac->data_callback().set(FUNC(a2bus_thunderclock_device::upd_dataout_w));
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *a2bus_thunderclock_device::device_rom_region() const
{
	return ROM_NAME( thunderclock );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_thunderclock_device::a2bus_thunderclock_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_upd1990ac(*this, THUNDERCLOCK_UPD1990_TAG), m_rom(nullptr), m_dataout(0)
{
}

a2bus_thunderclock_device::a2bus_thunderclock_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_thunderclock_device(mconfig, A2BUS_THUNDERCLOCK, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_thunderclock_device::device_start()
{
	m_rom = device().machine().root_device().memregion(this->subtag(THUNDERCLOCK_ROM_REGION).c_str())->base();

	save_item(NAME(m_dataout));
}

void a2bus_thunderclock_device::device_reset()
{
	m_dataout = 0;
}


/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

uint8_t a2bus_thunderclock_device::read_c0nx(uint8_t offset)
{
	return (m_dataout << 7);
}


/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_thunderclock_device::write_c0nx(uint8_t offset, uint8_t data)
{
	// uPD1990AC hookup:
	// bit 0 = DATA IN?
	// bit 1 = CLK
	// bit 2 = STB
	// bit 3 = C0
	// bit 4 = C1
	// bit 5 = C2
	// bit 7 = data out
	if (offset == 0)
	{
		m_upd1990ac->cs_w(1);
		m_upd1990ac->oe_w(1);
		m_upd1990ac->data_in_w(data&0x01);
		m_upd1990ac->c0_w((data&0x08) >> 3);
		m_upd1990ac->c1_w((data&0x10) >> 4);
		m_upd1990ac->c2_w((data&0x20) >> 5);
		m_upd1990ac->stb_w((data&0x04) >> 2);
		m_upd1990ac->clk_w((data&0x02) >> 1);
	}
}

/*-------------------------------------------------
    read_cnxx - called for reads from this card's cnxx space
-------------------------------------------------*/

uint8_t a2bus_thunderclock_device::read_cnxx(uint8_t offset)
{
	// ROM is primarily a c800 image, but the first page is also the CnXX ROM
	return m_rom[offset];
}

/*-------------------------------------------------
    read_c800 - called for reads from this card's c800 space
-------------------------------------------------*/

uint8_t a2bus_thunderclock_device::read_c800(uint16_t offset)
{
	return m_rom[offset];
}

WRITE_LINE_MEMBER( a2bus_thunderclock_device::upd_dataout_w )
{
	if (state)
	{
		m_dataout = 1;
	}
	else
	{
		m_dataout = 0;
	}
}
