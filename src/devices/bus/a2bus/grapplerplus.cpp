// license:BSD-3-Clause
// copyright-holders:R. Belmont, Golden Child
/*********************************************************************

    grapplerplus.c

    Orange Micro Grappler Plus Apple II Parallel Interface Card

*********************************************************************/

#include "emu.h"
#include "grapplerplus.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(A2BUS_GRAPPLERPLUS, a2bus_grapplerplus_device, "grapplerplus", "Grappler Interface Card")

#define GRAPPLERPLUS_ROM_REGION  "grapplerplus_rom"
#define GRAPPLERPLUS_CENTRONICS_TAG "centronics"

ROM_START( grapplerplus )
	ROM_REGION(0x001000, GRAPPLERPLUS_ROM_REGION, 0)
	ROM_LOAD( "grapplerplus.bin", 0x000000, 0x001000, CRC(6f88b70c) SHA1(433ae61a0553ee9c1628ea5b6376dac848c04cad) )
ROM_END

static INPUT_PORTS_START( grapplerplus )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "MSB Control (SW1)" )
	PORT_DIPSETTING( 0x00, "7 bit only" )
	PORT_DIPSETTING( 0x01, "8 bit" )

	PORT_DIPNAME( 0x0e, 0x0e, "Printer Type ID (SW2-4)" )
	PORT_DIPSETTING( 0x02, "Anadex Printers" )
	PORT_DIPSETTING( 0x04, "Apple Dot Matrix" )
	PORT_DIPSETTING( 0x06, "NEC 8023/C. Itoh 8510/DMP 85" )
	PORT_DIPSETTING( 0x08, "Okidata 84 w/o Step II Graphics" )
	PORT_DIPSETTING( 0x0a, "Star Gemini" )
	PORT_DIPSETTING( 0x0c, "Okidata 82A, 83A, 84, 92, 93" )
	PORT_DIPSETTING( 0x0e, "Epson Series" )
INPUT_PORTS_END

ioport_constructor a2bus_grapplerplus_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( grapplerplus );
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void a2bus_grapplerplus_device::device_add_mconfig(machine_config &config)
{
	CENTRONICS(config, m_ctx, centronics_devices, "printer");
	m_ctx->set_data_input_buffer(m_ctx_data_in);
	m_ctx->busy_handler().set(FUNC(a2bus_grapplerplus_device::busy_w));
	m_ctx->perror_handler().set(FUNC(a2bus_grapplerplus_device::perror_w));
	m_ctx->select_handler().set(FUNC(a2bus_grapplerplus_device::select_w));
	m_ctx->ack_handler().set(FUNC(a2bus_grapplerplus_device::ack_w));

	INPUT_BUFFER(config, m_ctx_data_in);
	OUTPUT_LATCH(config, m_ctx_data_out);
	m_ctx->set_output_latch(*m_ctx_data_out);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *a2bus_grapplerplus_device::device_rom_region() const
{
	return ROM_NAME( grapplerplus );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_grapplerplus_device::a2bus_grapplerplus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_grapplerplus_device(mconfig, A2BUS_GRAPPLERPLUS, tag, owner, clock)
{
}

a2bus_grapplerplus_device::a2bus_grapplerplus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_dsw1(*this, "DSW1"),
	m_ctx(*this, GRAPPLERPLUS_CENTRONICS_TAG),
	m_ctx_data_in(*this, "ctx_data_in"),
	m_ctx_data_out(*this, "ctx_data_out"),
	m_rom(*this, GRAPPLERPLUS_ROM_REGION),
	m_started(false), m_ack(0), m_busy(0), m_perror(0), m_select(0), m_irqbit(0), m_irqenable(false), m_timer(nullptr)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_grapplerplus_device::device_start()
{
	m_timer = timer_alloc(0, nullptr);
	m_timer->adjust(attotime::never);

	save_item(NAME(m_started));
	save_item(NAME(m_ack));
	save_item(NAME(m_busy));
	save_item(NAME(m_perror));
	save_item(NAME(m_select));
	save_item(NAME(m_irqbit));
	save_item(NAME(m_irqenable));
	save_item(NAME(m_rombank));
}

void a2bus_grapplerplus_device::device_reset()
{
	m_started = true;
	m_ack = 0;
	m_irqenable = false;
	m_irqbit = false;
	lower_slot_irq();
	m_timer->adjust(attotime::never);

	// set initial state of the strobe line
	clear_strobe();
}

void a2bus_grapplerplus_device::device_timer(emu_timer &timer, device_timer_id tid, int param, void *ptr)
{
	clear_strobe();

	m_timer->adjust(attotime::never);
}

/*-------------------------------------------------
    read_cnxx - called for reads from this card's cnxx space
-------------------------------------------------*/

uint8_t a2bus_grapplerplus_device::read_cnxx(uint8_t offset)
{
	m_rombank = 0; // reads to cnxx will reset rom bank to bank 1
	return m_rom[(offset&0xff)];
}

/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

uint8_t a2bus_grapplerplus_device::read_c0nx(uint8_t offset)
{
	uint8_t printertype = ~(m_dsw1->read()) & 0x0e;

	return  ((m_irqbit ? 1 : 0) << 7) |
			(BIT(printertype,1) << 6) |
			(BIT(printertype,2) << 5) |
			(BIT(printertype,3) << 4) |
			((m_busy ? 1 : 0)   << 3) |
			((m_perror ? 1 : 0) << 2) |
			((m_select ? 1 : 0) << 1) |
			((m_ack ? 1 : 0)    << 0);
}

uint8_t a2bus_grapplerplus_device::read_c800(uint16_t offset)
{
	return m_rom[offset + (m_rombank==0 ? 0x0 : 0x800)];
}

/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_grapplerplus_device::write_c0nx(uint8_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0: // Output data and send a strobe
			m_ctx_data_out->write(data);
			start_strobe();
			break;

		case 1: // Set rom bank to bank 2
			m_rombank=1;
			break;

		case 2: // Reset interrupt request and IRQ data bit
			m_irqenable = false;
			m_irqbit = false;
			break;

		case 4: // Output data, strobe, enable interrupt on ACK
			m_ctx_data_out->write(data);
			start_strobe();
			m_irqenable = true;
			m_irqbit = false;
			break;
	}
}

WRITE_LINE_MEMBER( a2bus_grapplerplus_device::ack_w )
{
	if (m_started)
	{
		m_ack = state;
		if (state && m_irqenable)
		{
			raise_slot_irq();
			m_irqbit = true;
		}
	}
}

WRITE_LINE_MEMBER( a2bus_grapplerplus_device::perror_w )
{
	m_perror = (state == ASSERT_LINE) ? true : false;
}

WRITE_LINE_MEMBER( a2bus_grapplerplus_device::busy_w )
{
	m_busy = (state == ASSERT_LINE) ? true : false;
}

WRITE_LINE_MEMBER( a2bus_grapplerplus_device::select_w )
{
	m_select = (state == ASSERT_LINE) ? true : false;
}

void a2bus_grapplerplus_device::start_strobe()
{
	int usec = 1;  // strobe length in microseconds
	m_ctx->write_strobe(CLEAR_LINE);
	m_timer->adjust(attotime::from_usec(usec), 0, attotime::never);
}

void a2bus_grapplerplus_device::clear_strobe()
{
	m_ctx->write_strobe(ASSERT_LINE);
}

