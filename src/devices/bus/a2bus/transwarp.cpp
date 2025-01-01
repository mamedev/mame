// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    transwarp.cpp

    Implementation of the Applied Engineering TransWarp accelerator

    TODO:
    - needs built-in language card, it's advertised to work w/o one.
    - Doesn't work with Swyft but advertised to; how does h/w get
      around the Fxxx ROM not checksumming right?

    To control this from software:
    - There's no way I can tell to detect it besides maybe measuring
      how many cycles between vblanks or something.
    - Write to $C074: 0 = fast speed, 1 = 1 MHz,
      3 = disables the TransWarp's CPU and restarts the Apple's 65(C)02.

*********************************************************************/

#include "emu.h"
#include "transwarp.h"
#include "cpu/m6502/m6502.h"
#include "cpu/m6502/w65c02.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_transwarp_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_transwarp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	a2bus_transwarp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// overrides of device_t functions
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual bool take_c800() override;

private:
	bool m_bEnabled;
	bool m_bReadA2ROM;
	bool m_bIn1MHzMode;
	emu_timer *m_timer;

	required_device<cpu_device> m_ourcpu;
	required_region_ptr<uint8_t> m_rom;
	required_ioport m_dsw1, m_dsw2;

	TIMER_CALLBACK_MEMBER(clock_adjust_tick);

	uint8_t dma_r(offs_t offset);
	void dma_w(offs_t offset, uint8_t data);

	void w65c02_mem(address_map &map) ATTR_COLD;

	void hit_slot(int slot);
	void hit_slot_joy();
};

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_TRANSWARP, device_a2bus_card_interface, a2bus_transwarp_device, "a2twarp", "Applied Engineering TransWarp")

void a2bus_transwarp_device::w65c02_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(a2bus_transwarp_device::dma_r), FUNC(a2bus_transwarp_device::dma_w));
}

ROM_START( warprom )
	ROM_REGION(0x1000, "twrom", 0)
	ROM_LOAD( "ae transwarp rom v1.4.bin", 0x000000, 0x001000, CRC(afe37f55) SHA1(7b75534e7895e04859a0b1337801c6eeb0cef52a) )
ROM_END

static INPUT_PORTS_START( warp )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Slot 1" )
	PORT_DIPSETTING(    0x00, "Language Card" )
	PORT_DIPSETTING(    0x01, "Non-LC" )
	PORT_DIPNAME( 0x02, 0x02, "Slot 2" )
	PORT_DIPSETTING(    0x00, "Language Card" )
	PORT_DIPSETTING(    0x02, "Non-LC" )
	PORT_DIPNAME( 0x04, 0x04, "Slot 3" )
	PORT_DIPSETTING(    0x00, "Language Card" )
	PORT_DIPSETTING(    0x04, "Non-LC" )
	PORT_DIPNAME( 0x08, 0x08, "Slot 4" )
	PORT_DIPSETTING(    0x00, "Language Card" )
	PORT_DIPSETTING(    0x08, "Non-LC" )
	PORT_DIPNAME( 0x10, 0x10, "Slot 5" )
	PORT_DIPSETTING(    0x00, "Language Card" )
	PORT_DIPSETTING(    0x10, "Non-LC" )
	PORT_DIPNAME( 0x20, 0x20, "Slot 6" )
	PORT_DIPSETTING(    0x00, "Language Card" )
	PORT_DIPSETTING(    0x20, "Non-LC" )
	PORT_DIPNAME( 0x40, 0x40, "Slot 7" )
	PORT_DIPSETTING(    0x00, "Language Card" )
	PORT_DIPSETTING(    0x40, "Non-LC" )
	PORT_DIPNAME( 0x80, 0x00, "Speed" )
	PORT_DIPSETTING(    0x00, "Full acceleration" )
	PORT_DIPSETTING(    0x80, "Half acceleration" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Slot 1" )
	PORT_DIPSETTING(    0x00, "Stock speed" )
	PORT_DIPSETTING(    0x01, "Accelerated" )
	PORT_DIPNAME( 0x02, 0x02, "Slot 2" )
	PORT_DIPSETTING(    0x00, "Stock speed" )
	PORT_DIPSETTING(    0x02, "Accelerated" )
	PORT_DIPNAME( 0x04, 0x04, "Slot 3" )
	PORT_DIPSETTING(    0x00, "Stock speed" )
	PORT_DIPSETTING(    0x04, "Accelerated" )
	PORT_DIPNAME( 0x08, 0x08, "Slot 4" )
	PORT_DIPSETTING(    0x00, "Stock speed" )
	PORT_DIPSETTING(    0x08, "Accelerated" )
	PORT_DIPNAME( 0x10, 0x10, "Slot 5" )
	PORT_DIPSETTING(    0x00, "Stock speed" )
	PORT_DIPSETTING(    0x10, "Accelerated" )
	PORT_DIPNAME( 0x20, 0x00, "Slot 6" )
	PORT_DIPSETTING(    0x00, "Stock speed" )
	PORT_DIPSETTING(    0x20, "Accelerated" )
	PORT_DIPNAME( 0x40, 0x40, "Slot 7" )
	PORT_DIPSETTING(    0x00, "Stock speed" )
	PORT_DIPSETTING(    0x40, "Accelerated" )
	PORT_DIPNAME( 0x80, 0x00, "Transwarp Enable" )
	PORT_DIPSETTING(    0x00, "Acceleration enabled" )
	PORT_DIPSETTING(    0x80, "Acceleration disabled" )
INPUT_PORTS_END

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/*-------------------------------------------------
    rom_region - device-specific ROM region
-------------------------------------------------*/

const tiny_rom_entry *a2bus_transwarp_device::device_rom_region() const
{
	return ROM_NAME(warprom);
}

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor a2bus_transwarp_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( warp );
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void a2bus_transwarp_device::device_add_mconfig(machine_config &config)
{
	W65C02(config, m_ourcpu, DERIVED_CLOCK(1, 2));
	m_ourcpu->set_addrmap(AS_PROGRAM, &a2bus_transwarp_device::w65c02_mem);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_transwarp_device::a2bus_transwarp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_bEnabled(false),
	m_ourcpu(*this, "tw65c02"),
	m_rom(*this, "twrom"),
	m_dsw1(*this, "DSW1"),
	m_dsw2(*this, "DSW2")
{
}

a2bus_transwarp_device::a2bus_transwarp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_transwarp_device(mconfig, A2BUS_TRANSWARP, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_transwarp_device::device_start()
{
	m_timer = timer_alloc(FUNC(a2bus_transwarp_device::clock_adjust_tick), this);

	save_item(NAME(m_bEnabled));
	save_item(NAME(m_bReadA2ROM));
	save_item(NAME(m_bIn1MHzMode));
}

void a2bus_transwarp_device::device_reset()
{
	m_bEnabled = true;
	m_bReadA2ROM = false;
	raise_slot_dma();
	if (!(m_dsw2->read() & 0x80))
	{
		if (m_dsw1->read() & 0x80)
		{
			m_ourcpu->set_unscaled_clock(clock() / 4);
		}
		else
		{
			m_ourcpu->set_unscaled_clock(clock() / 2);
		}
	}
	else
	{
		m_ourcpu->set_unscaled_clock(1021800);
	}
}

TIMER_CALLBACK_MEMBER(a2bus_transwarp_device::clock_adjust_tick)
{
	if (m_bIn1MHzMode)
	{
		m_ourcpu->set_unscaled_clock(1021800);
	}
	else
	{
		if (!(m_dsw2->read() & 0x80))
		{
			if (m_dsw1->read() & 0x80)
			{
				m_ourcpu->set_unscaled_clock(clock() / 4);
			}
			else
			{
				m_ourcpu->set_unscaled_clock(clock() / 2);
			}
		}
	}
	m_timer->adjust(attotime::never);
}

uint8_t a2bus_transwarp_device::dma_r(offs_t offset)
{
	if (offset == 0xc070)
	{
		hit_slot_joy();
	}
	else if ((offset >= 0xc090) && (offset <= 0xc0ff))
	{
		hit_slot(((offset >> 4) & 0xf) - 8);
	}
	else if ((offset >= 0xc100) && (offset <= 0xc7ff))
	{
		hit_slot((offset >> 8) & 0x7);
	}
	else if ((offset >= 0xf000) && (!m_bReadA2ROM))
	{
		return m_rom[offset & 0xfff];
	}

	return slot_dma_read(offset);
}


//-------------------------------------------------
//  dma_w -
//-------------------------------------------------

void a2bus_transwarp_device::dma_w(offs_t offset, uint8_t data)
{
	//if ((offset >= 0xc070) && (offset <= 0xc07f)) printf("%02x to %04x\n", data, offset);

	if (offset == 0xc070)
	{
		hit_slot_joy();
	}
	else if (offset == 0xc072)
	{
		m_bReadA2ROM = true;
	}
	else if (offset == 0xc074)
	{
		if (data == 0)
		{
			if (m_dsw1->read() & 0x80)
			{
				m_ourcpu->set_unscaled_clock(clock() / 4);
			}
			else
			{
				m_ourcpu->set_unscaled_clock(clock() / 2);
			}
			m_bIn1MHzMode = false;
		}
		else if (data == 1)
		{
			m_ourcpu->set_unscaled_clock(1021800);
			m_bIn1MHzMode = true;
		}
		else if (data == 3)
		{
			// disable our CPU
			m_ourcpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
			// re-enable the Apple's
			lower_slot_dma();
		}
		return;
	}
	else if ((offset >= 0xc090) && (offset <= 0xc0ff))
	{
		hit_slot(((offset >> 4) & 0xf) - 8);
	}
	else if ((offset >= 0xc100) && (offset <= 0xc7ff))
	{
		hit_slot((offset >> 8) & 0x7);
	}

	slot_dma_write(offset, data);
}

bool a2bus_transwarp_device::take_c800()
{
	return false;
}

void a2bus_transwarp_device::hit_slot(int slot)
{
	// only do slot slowdown if acceleration is enabled
	if (!(m_dsw2->read() & 0x80))
	{
		// accleration's on, check the specific slot
		if (!(m_dsw2->read() & (1<<(slot-1))))
		{
			m_ourcpu->set_unscaled_clock(1021800);
			// slow down for 20 uSec, should be more than enough
			m_timer->adjust(attotime::from_usec(20));
		}
	}
}

void a2bus_transwarp_device::hit_slot_joy()
{
	// only do slot slowdown if acceleration is enabled
	if (!(m_dsw2->read() & 0x80))
	{
		// accleration's on
		m_ourcpu->set_unscaled_clock(1021800);
		// PREAD main loop counts up to 11*256 uSec, add 1 to cover the setup
		m_timer->adjust(attotime::from_usec(11*257));
	}
}
