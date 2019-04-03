// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2pic.c

    Apple II Parallel Interface Card (670-0021)

*********************************************************************/

#include "emu.h"
#include "a2pic.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(A2BUS_PIC, a2bus_pic_device, "a2pic", "Apple Parallel Interface Card")

#define PIC_ROM_REGION  "pic_rom"
#define PIC_CENTRONICS_TAG "pic_ctx"

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
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void a2bus_pic_device::device_add_mconfig(machine_config &config)
{
	CENTRONICS(config, m_ctx, centronics_devices, "printer");
	m_ctx->set_data_input_buffer(m_ctx_data_in);
	m_ctx->ack_handler().set(FUNC(a2bus_pic_device::ack_w));

	INPUT_BUFFER(config, m_ctx_data_in);
	OUTPUT_LATCH(config, m_ctx_data_out);
	m_ctx->set_output_latch(*m_ctx_data_out);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *a2bus_pic_device::device_rom_region() const
{
	return ROM_NAME( pic );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_pic_device::a2bus_pic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_pic_device(mconfig, A2BUS_PIC, tag, owner, clock)
{
}

a2bus_pic_device::a2bus_pic_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
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

uint8_t a2bus_pic_device::read_cnxx(uint8_t offset)
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

uint8_t a2bus_pic_device::read_c0nx(uint8_t offset)
{
	uint8_t rv = 0;

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

void a2bus_pic_device::write_c0nx(uint8_t offset, uint8_t data)
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
		uint8_t dsw1 = m_dsw1->read();

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
