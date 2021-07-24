// license:BSD-3-Clause
// copyright-holders:Golden Child
#include "emu.h"
#include "etibuffer.h"
#include "etibuffer.lh"
#include <cstdlib>

#define ETI_CENTRONICS_TAG "prn"

#define STROBE_DELAY 1  // in usec
#define ACK_DELAY 1     // in usec

ROM_START( etiprintbuffer_device )
	ROM_REGION( 0x800, "maincpu", 0 )
	ROM_LOAD( "etibuffer.rom",  0x0, 0x800, CRC(bd31d7b6) SHA1(cd76a9a53c6b9994c5721f8c393bc782143c6d3f))
ROM_END

static INPUT_PORTS_START ( etiprintbuffer_device )
	PORT_START("TEST")

	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Test Buffer") PORT_CODE(KEYCODE_2_PAD)
		PORT_CHANGED_MEMBER(DEVICE_SELF, etiprintbuffer_device, test_sw, 0)

	PORT_START("CLEAR")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Clear Buffer") PORT_CODE(KEYCODE_5_PAD)
		PORT_CHANGED_MEMBER(DEVICE_SELF, etiprintbuffer_device, clear_sw, 0)

	PORT_START("CONFIG")
	PORT_CONFNAME(0x03, 0x03, "Ram Size")
//  ram size of 0k will pass only 0xFF on to printer, use for ram test failure
//  PORT_CONFSETTING(0x00, "0K (buffer ram test fail)")
	PORT_CONFSETTING(0x01, "16K")
	PORT_CONFSETTING(0x02, "32K")
	PORT_CONFSETTING(0x03, "48K")
INPUT_PORTS_END

DEFINE_DEVICE_TYPE(ETIPRINTBUFFER, etiprintbuffer_device, "etiprintbuffer", "Electronics Today International Print Buffer 48K")

//--------------------------------------------------
//  device constructor
//--------------------------------------------------

etiprintbuffer_device::etiprintbuffer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	etiprintbuffer_device(mconfig, ETIPRINTBUFFER, tag, owner, clock)
{
}

etiprintbuffer_device::etiprintbuffer_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_centronics_peripheral_interface(mconfig, *this)
	, m_maincpu(*this, "maincpu")
	, m_screen(*this, "screen")
	, m_ctx(*this, ETI_CENTRONICS_TAG)
	, m_ctx_data_out(*this, "ctx_data_out")
	, m_printerready_led(*this, "printerready_led")
	, m_bufferready_led(*this, "bufferready_led")
{
}

//--------------------------------------------------
//  mem map
//--------------------------------------------------

void etiprintbuffer_device::etiprintbuffer_device_memmap(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7ff).rom().region("maincpu", 0);
	map(0x1000, 0x1fff).r(FUNC(etiprintbuffer_device::eti_read_1000));
	map(0x2000, 0x2fff).w(FUNC(etiprintbuffer_device::eti_write_2000));
	map(0x3000, 0x3fff).w(FUNC(etiprintbuffer_device::eti_write_3000));
}

//--------------------------------------------------
//  io map
//--------------------------------------------------

void etiprintbuffer_device::etiprintbuffer_device_iomap(address_map &map)
{
	map.unmap_value_high();
	map(0x0, 0xffff).r(FUNC(etiprintbuffer_device::eti_status_r));
}

//--------------------------------------------------
//  memory and io functions
//--------------------------------------------------

uint8_t etiprintbuffer_device::eti_status_r(offs_t offset)
{
	return  ((m_strobereceived ? 1 : 0 ) << 6) | // d6 is m_strobe status
			((m_busy           ? 0 : 1 ) << 7);  // d7 is not busy output
}

uint8_t etiprintbuffer_device::eti_read_1000(offs_t offset)
{
	output_ack(0);

	m_strobereceived = false;  // clear the flip flop
	m_bufferready_led = !m_strobereceived;
	output_busy(m_strobereceived);

	m_ack_timer->adjust(attotime::from_usec(ACK_DELAY), TIMER_ACK);
	return m_datalatch;
}

void etiprintbuffer_device::eti_write_2000(offs_t offset, uint8_t data)
{
	m_ctx_data_out->write(data);
}

void etiprintbuffer_device::eti_write_3000(offs_t offset, uint8_t data)
{
	m_ctx->write_strobe(0);
	m_strobeout_timer->adjust(attotime::from_usec(STROBE_DELAY), TIMER_STROBEOUT);
}

//--------------------------------------------------
//  device init
//--------------------------------------------------

void etiprintbuffer_device::device_start()
{
	m_printerready_led.resolve();
	m_bufferready_led.resolve();
	m_ack_timer = timer_alloc(TIMER_ACK);
	m_strobeout_timer = timer_alloc(TIMER_STROBEOUT);

	save_item(NAME(m_ram));
	save_item(NAME(m_data));
	save_item(NAME(m_datalatch));
	save_item(NAME(m_strobereceived));
	save_item(NAME(m_strobe));
	save_item(NAME(m_busy));
}

void etiprintbuffer_device::device_reset()
{
	// Setup 16k blocks in address space

	address_space &mem = m_maincpu->space(AS_PROGRAM);
	u16 ram16kblocks = ioport("CONFIG")->read() & 0x03;
	mem.unmap_readwrite(0x4000, 0xffff);
	if (ram16kblocks) mem.install_ram(0x4000, 0x4000 + (ram16kblocks * 0x4000) - 1, m_ram);

	// Initialize printer input port

	// On the etibuffer printer input port, only STB, D0-D7, ACK and BUSY are actually connected
	// The Apple 2 grappler cards expect select to be high, so unless we set it to 1, the etibuffer won't work.

	output_busy(0);
	output_ack(1);
	output_select(1);  // (line not actually connected to etibuffer) set this to 1 for grappler cards
	output_fault(1);   // (line not actually connected)
	output_perror(0);  // (line not actually connected)

	// Initialize printer output port

	m_ctx->write_strobe(1);
}

//--------------------------------------------------
//  device mconfig
//--------------------------------------------------

void etiprintbuffer_device::device_add_mconfig(machine_config &config)
{
	Z80(config, m_maincpu, 2_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &etiprintbuffer_device::etiprintbuffer_device_memmap);
	m_maincpu->set_addrmap(AS_IO,      &etiprintbuffer_device::etiprintbuffer_device_iomap);

	CENTRONICS(config, m_ctx, centronics_devices, "printer");
	m_ctx->busy_handler().set(FUNC(etiprintbuffer_device::busy_w));

	OUTPUT_LATCH(config, m_ctx_data_out);
	m_ctx->set_output_latch(*m_ctx_data_out);

	config.set_default_layout(layout_etibuffer);
}

//--------------------------------------------------
//  tiny rom entry
//--------------------------------------------------

const tiny_rom_entry *etiprintbuffer_device::device_rom_region() const
{
	return ROM_NAME( etiprintbuffer_device );
}

//--------------------------------------------------
//  ioport
//--------------------------------------------------

ioport_constructor etiprintbuffer_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( etiprintbuffer_device );
}

//--------------------------------------------------
//  INPUT_CHANGED_MEMBERS
//--------------------------------------------------

INPUT_CHANGED_MEMBER(etiprintbuffer_device::clear_sw)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? CLEAR_LINE : ASSERT_LINE);
}

INPUT_CHANGED_MEMBER(etiprintbuffer_device::test_sw)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, newval ? CLEAR_LINE : ASSERT_LINE);
}

//--------------------------------------------------
//  WRITE_LINE_MEMBERS
//--------------------------------------------------

WRITE_LINE_MEMBER( etiprintbuffer_device::busy_w )
{
	m_busy = state;
	m_printerready_led = !m_busy;
}

WRITE_LINE_MEMBER( etiprintbuffer_device::input_strobe )
{
	if (m_strobe && !state)
	{
		m_datalatch = m_data;

		m_strobereceived = true;
		output_busy(m_strobereceived);
		m_bufferready_led = !m_strobereceived;
	}
	m_strobe = state;
}

//--------------------------------------------------
//  TIMERS
//--------------------------------------------------

void etiprintbuffer_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
	case TIMER_ACK:
		output_ack(1);
		output_busy(m_strobereceived);
		break;
	case TIMER_STROBEOUT:
		m_ctx->write_strobe(1);
	}
}

