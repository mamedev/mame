// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2pic.c

    Apple II Parallel Interface Card (670-0021)

*********************************************************************/

#include "a2pic.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type A2BUS_PIC = &device_creator<a2bus_pic_device>;

#define PIC_ROM_REGION  "pic_rom"
#define PIC_CENTRONICS_TAG "pic_ctx"

MACHINE_CONFIG_FRAGMENT( pic )
	MCFG_CENTRONICS_ADD(PIC_CENTRONICS_TAG, centronics_devices, "printer")
	MCFG_CENTRONICS_DATA_INPUT_BUFFER("ctx_data_in")
	MCFG_CENTRONICS_ACK_HANDLER(WRITELINE(a2bus_pic_device, ack_w))

	MCFG_DEVICE_ADD("ctx_data_in", INPUT_BUFFER, 0)
	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("ctx_data_out", PIC_CENTRONICS_TAG)
MACHINE_CONFIG_END

ROM_START( pic )
	ROM_REGION(0x000200, PIC_ROM_REGION, 0)
	ROM_LOAD( "341-0057.bin", 0x000000, 0x000200, CRC(0d2d84ee) SHA1(bfc5b863d37e59875a6159528eb0f2b6082063b5) )
ROM_END

static INPUT_PORTS_START( pic )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x00, "Strobe length (SW1-3)" )
	PORT_DIPSETTING(    0x00, "1 microsecond" )
	PORT_DIPSETTING(    0x01, "3 microseconds" )
	PORT_DIPSETTING(    0x02, "5 microseconds" )
	PORT_DIPSETTING(    0x03, "7 microseconds" )
	PORT_DIPSETTING(    0x04, "9 microseconds" )
	PORT_DIPSETTING(    0x05, "11 microseconds" )
	PORT_DIPSETTING(    0x06, "13 microseconds" )
	PORT_DIPSETTING(    0x07, "15 microseconds" )

	PORT_DIPNAME( 0x08, 0x08, "Strobe polarity (SW4)" )
	PORT_DIPSETTING(    0x00, "Positive" )
	PORT_DIPSETTING(    0x08, "Negative" )

	PORT_DIPNAME( 0x10, 0x10, "Acknowledge polarity (SW5)" )
	PORT_DIPSETTING(    0x00, "Positive" )
	PORT_DIPSETTING(    0x10, "Negative" )

	PORT_DIPNAME( 0x20, 0x20, "Firmware (SW6)" )
	PORT_DIPSETTING(    0x00, "Parallel Printer (341-0005)" )
	PORT_DIPSETTING(    0x20, "Centronics (341-0019)" )

	PORT_DIPNAME( 0x40, 0x00, "Use interrupts (SW7)" )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x40, "On" )
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor a2bus_pic_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( pic );
}

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor a2bus_pic_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( pic );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *a2bus_pic_device::device_rom_region() const
{
	return ROM_NAME( pic );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_pic_device::a2bus_pic_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, A2BUS_PIC, "Apple Parallel Interface Card", tag, owner, clock, "a2pic", __FILE__),
		device_a2bus_card_interface(mconfig, *this),
		m_dsw1(*this, "DSW1"),
		m_ctx(*this, PIC_CENTRONICS_TAG),
		m_ctx_data_in(*this, "ctx_data_in"),
		m_ctx_data_out(*this, "ctx_data_out"), m_rom(nullptr),
		m_started(false), m_ack(0), m_irqenable(false), m_autostrobe(false), m_timer(nullptr)
{
}

a2bus_pic_device::a2bus_pic_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
		device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_a2bus_card_interface(mconfig, *this),
		m_dsw1(*this, "DSW1"),
		m_ctx(*this, PIC_CENTRONICS_TAG),
		m_ctx_data_in(*this, "ctx_data_in"),
		m_ctx_data_out(*this, "ctx_data_out"), m_rom(nullptr),
		m_started(false), m_ack(0), m_irqenable(false), m_autostrobe(false), m_timer(nullptr)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_pic_device::device_start()
{
	// set_a2bus_device makes m_slot valid
	set_a2bus_device();

	m_rom = device().machine().root_device().memregion(this->subtag(PIC_ROM_REGION).c_str())->base();

	m_timer = timer_alloc(0, nullptr);
	m_timer->adjust(attotime::never);

	save_item(NAME(m_ack));
	save_item(NAME(m_irqenable));
	save_item(NAME(m_autostrobe));
}

void a2bus_pic_device::device_reset()
{
	m_started = true;
	m_ack = 0;
	m_irqenable = false;
	m_autostrobe = false;
	lower_slot_irq();
	m_timer->adjust(attotime::never);

	// set initial state of the strobe line depending on the dipswitch
	clear_strobe();
}

void a2bus_pic_device::device_timer(emu_timer &timer, device_timer_id tid, int param, void *ptr)
{
	clear_strobe();

	m_timer->adjust(attotime::never);
}

/*-------------------------------------------------
    read_cnxx - called for reads from this card's cnxx space
-------------------------------------------------*/

UINT8 a2bus_pic_device::read_cnxx(address_space &space, UINT8 offset)
{
	m_autostrobe = true;

	if (m_dsw1->read() & 0x20)
	{
		return m_rom[(offset&0xff) | 0x100];
	}

	return m_rom[(offset&0xff)];
}

/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

UINT8 a2bus_pic_device::read_c0nx(address_space &space, UINT8 offset)
{
	UINT8 rv = 0;

	switch (offset)
	{
		case 3:
			return m_ctx_data_in->read();

		case 4:
			rv = m_ack;

			// clear flip-flop
			if (m_dsw1->read() & 0x10)    // negative polarity
			{
				m_ack |= 0x80;
			}
			else
			{
				m_ack &= ~0x80;
			}

			return rv;

		case 6: // does reading this really work?
			m_irqenable = true;
			break;

		case 7:
			m_irqenable = false;
			m_autostrobe = false;
			lower_slot_irq();
			break;

	}

	return 0;
}

/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_pic_device::write_c0nx(address_space &space, UINT8 offset, UINT8 data)
{
	switch (offset)
	{
		case 0: // set data out and send a strobe
			m_ctx_data_out->write(data);

			if (m_autostrobe)
			{
				start_strobe();
			}
			break;

		case 2: // send a strobe
			start_strobe();
			break;

		case 6: // enable interrupt on ACK
			m_irqenable = true;
			break;

		case 7: // disable and acknowledge IRQ, reset ACK flip-flop, disable autostrobe
			m_irqenable = false;
			m_autostrobe = false;
			lower_slot_irq();
			break;
	}
}

WRITE_LINE_MEMBER( a2bus_pic_device::ack_w )
{
	if (m_started)
	{
		UINT8 dsw1 = m_dsw1->read();

		if (dsw1 & 0x10)    // negative polarity
		{
			m_ack = (state == ASSERT_LINE) ? 0x00 : 0x80;
		}
		else
		{
			m_ack = (state == ASSERT_LINE) ? 0x80 : 0x00;
		}

		m_ack |= 0x40;  // set ACK flip-flop

		if ((dsw1 & 0x40) && (m_irqenable))
		{
			raise_slot_irq();
		}
	}
}

void a2bus_pic_device::start_strobe()
{
	int usec = ((m_dsw1->read() & 7) * 2) + 1;  // strobe length in microseconds

	if (m_dsw1->read() & 0x8)   // negative polarity
	{
		m_ctx->write_strobe(CLEAR_LINE);
	}
	else
	{
		m_ctx->write_strobe(ASSERT_LINE);
	}

	m_timer->adjust(attotime::from_usec(usec), 0, attotime::never);
}

void a2bus_pic_device::clear_strobe()
{
	if (m_dsw1->read() & 0x8)   // negative polarity
	{
		m_ctx->write_strobe(ASSERT_LINE);
	}
	else
	{
		m_ctx->write_strobe(CLEAR_LINE);
	}
}
