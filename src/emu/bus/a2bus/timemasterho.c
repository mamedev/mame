// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    timemasterho.c

    Implemention of the Applied Engineering TimeMaster H.O.


    PCB Layout:
     _____________________________________________________
    |  ___       _            _____________________       |
    | | D |     | |MSM5832   |                     |u3    |
    | | I |   u1| |          |    HD46821P         | ___  |
    | | P |     |_|          |_____________________|| B | |
    | |_S_|                           _____         | A | |
    |                              u2|_____| 74LS245| T | |
    |  |J1    74LS00  74LS08         ____________   | T | |
    |  |         _     _          u4|            |  | E | |
    |  |      u5| | u6| |           |   2716     |  | R | |
    |  |        |_|   |_|           |____________|  |_Y_| |
    |____________________________                        _|
                                 |                      |
                                 |______________________|


    DIPS: 1:SET  2:MODE 3:NMI 4:IRQ
    1 & 4 are on by default.

    J1: 8 pins for X10 home control functions (top to bottom)
       1: ADJ  2: 5V  3: MODE  4: GND
       5: A    6: 5V  7: B     8: GND

    X10 functions not supported.

*********************************************************************/

#include "timemasterho.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type A2BUS_TIMEMASTERHO = &device_creator<a2bus_timemasterho_device>;

#define TIMEMASTER_ROM_REGION   "timemst_rom"
#define TIMEMASTER_PIA_TAG      "timemst_pia"
#define TIMEMASTER_M5832_TAG    "timemst_msm"

MACHINE_CONFIG_FRAGMENT( timemaster )
	MCFG_DEVICE_ADD(TIMEMASTER_PIA_TAG, PIA6821, 1021800)
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(a2bus_timemasterho_device, pia_out_a))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(a2bus_timemasterho_device, pia_out_b))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(a2bus_timemasterho_device, pia_irqa_w))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(a2bus_timemasterho_device, pia_irqb_w))

	MCFG_DEVICE_ADD(TIMEMASTER_M5832_TAG, MSM5832, 32768)
MACHINE_CONFIG_END

ROM_START( timemaster )
	ROM_REGION(0x1000, TIMEMASTER_ROM_REGION, 0)
	ROM_LOAD( "ae timemaster ii h.o. rom rev. 5.bin", 0x000000, 0x001000, CRC(ff5bd644) SHA1(ae0173da61581a06188c1bee89e95a0aa536c411) )
ROM_END

static INPUT_PORTS_START( tmho )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Set")
	PORT_DIPSETTING(    0x00, "Apple can't set clock")
	PORT_DIPSETTING(    0x01, "Apple can set clock")

	PORT_DIPNAME( 0x02, 0x00, "Mode")
	PORT_DIPSETTING(    0x00, "TimeMaster")
	PORT_DIPSETTING(    0x02, "Mountain AppleClock")

	PORT_DIPNAME( 0x04, 0x00, "NMI")
	PORT_DIPSETTING(    0x00,  DEF_STR(Off))
	PORT_DIPSETTING(    0x04,  DEF_STR(On))

	PORT_DIPNAME( 0x08, 0x08, "IRQ")
	PORT_DIPSETTING(    0x00,  DEF_STR(Off))
	PORT_DIPSETTING(    0x08,  DEF_STR(On))
INPUT_PORTS_END

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor a2bus_timemasterho_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( tmho );
}

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor a2bus_timemasterho_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( timemaster );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *a2bus_timemasterho_device::device_rom_region() const
{
	return ROM_NAME( timemaster );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_timemasterho_device::a2bus_timemasterho_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	device_a2bus_card_interface(mconfig, *this),
	m_pia(*this, TIMEMASTER_PIA_TAG),
	m_msm5832(*this, TIMEMASTER_M5832_TAG),
	m_dsw1(*this, "DSW1")
{
	m_started = false;
}

a2bus_timemasterho_device::a2bus_timemasterho_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, A2BUS_TIMEMASTERHO, "Applied Engineering TimeMaster H.O.", tag, owner, clock, "a2tmstho", __FILE__),
	device_a2bus_card_interface(mconfig, *this),
	m_pia(*this, TIMEMASTER_PIA_TAG),
	m_msm5832(*this, TIMEMASTER_M5832_TAG),
	m_dsw1(*this, "DSW1")
{
	m_started = false;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_timemasterho_device::device_start()
{
	// set_a2bus_device makes m_slot valid
	set_a2bus_device();

	m_rom = device().machine().root_device().memregion(this->subtag(TIMEMASTER_ROM_REGION).c_str())->base();
}

void a2bus_timemasterho_device::device_reset()
{
	m_msm5832->cs_w(ASSERT_LINE);   // CS is tied to Vcc
	m_started = true;
}


/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

UINT8 a2bus_timemasterho_device::read_c0nx(address_space &space, UINT8 offset)
{
	if (offset <= 3)
	{
		return m_pia->read(space, offset);
	}

	return 0xff;
}


/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_timemasterho_device::write_c0nx(address_space &space, UINT8 offset, UINT8 data)
{
	if (offset <= 3)
	{
		m_pia->write(space, offset, data);
	}
}

/*-------------------------------------------------
    read_cnxx - called for reads from this card's cnxx space
-------------------------------------------------*/

UINT8 a2bus_timemasterho_device::read_cnxx(address_space &space, UINT8 offset)
{
	if (m_started)
	{
		if (!(m_dsw1->read() & 2))  // TimeMaster native
		{
			return m_rom[offset+0xc00];
		}
	}

	// Mountain Computer compatible
	return m_rom[offset+0x800];
}

/*-------------------------------------------------
    read_c800 - called for reads from this card's c800 space
-------------------------------------------------*/

UINT8 a2bus_timemasterho_device::read_c800(address_space &space, UINT16 offset)
{
	return m_rom[offset+0xc00];
}

WRITE8_MEMBER(a2bus_timemasterho_device::pia_out_a)
{
	// port A appears to be input only
}

WRITE8_MEMBER(a2bus_timemasterho_device::pia_out_b)
{
	m_msm5832->address_w(data & 0xf);
	m_msm5832->hold_w((data>>4) & 1 ? ASSERT_LINE : CLEAR_LINE);
	m_msm5832->read_w((data>>5) & 1 ? ASSERT_LINE : CLEAR_LINE);

	if (m_started)
	{
		if (m_dsw1->read() & 1)
		{
			m_msm5832->write_w((data >> 6) & 1 ? ASSERT_LINE : CLEAR_LINE);
		}
	}

	// if it's a read, poke it into the PIA
	if ((data>>5) & 1)
	{
		m_pia->porta_w(m_msm5832->data_r(space, 0));
	}
}

void a2bus_timemasterho_device::update_irqs()
{
	UINT8 dip = 0;

	if (m_started)
	{
		dip = m_dsw1->read();
	}

	if ((m_irqa | m_irqb) == ASSERT_LINE)
	{
		if (dip & 4)
		{
			raise_slot_nmi();
		}
		if (dip & 8)
		{
			raise_slot_irq();
		}
	}
	else
	{
		lower_slot_irq();
		lower_slot_nmi();
	}
}

WRITE_LINE_MEMBER(a2bus_timemasterho_device::pia_irqa_w)
{
	m_irqa = state;
	update_irqs();
}

WRITE_LINE_MEMBER(a2bus_timemasterho_device::pia_irqb_w)
{
	m_irqb = state;
	update_irqs();
}
