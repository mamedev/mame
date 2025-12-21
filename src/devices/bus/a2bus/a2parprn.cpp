// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#include "emu.h"
#include "a2parprn.h"

#include "bus/centronics/ctronics.h"

//#define VERBOSE 1
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

namespace {

class a2bus_parprn_device : public device_t, public device_a2bus_card_interface
{
public:
	a2bus_parprn_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	// device_a2bus_card_interface implementation
	virtual u8 read_c0nx(u8 offset) override;
	virtual void write_c0nx(u8 offset, u8 data) override;
	virtual u8 read_cnxx(u8 offset) override;
	virtual bool take_c800() override { return false; }

protected:
	a2bus_parprn_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock);

	// device_t implementation
	virtual tiny_rom_entry const *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// printer status inputs
	void ack_w(int state);

	// timer handlers
	TIMER_CALLBACK_MEMBER(update_strobe);

	required_device<centronics_device>      m_printer_conn;
	required_device<output_latch_device>    m_printer_out;
	required_ioport                         m_input_config;

protected:
	required_region_ptr<u8>                 m_prom;

private:
	emu_timer *                             m_strobe_timer;

	u8  m_next_strobe;  // B3 pin 13
	u8  m_ack_latch;    // B2 pin 6
	u8  m_ack_in;       // pin 2
};


class a2bus_4dparprn_device : public a2bus_parprn_device
{
public:
	a2bus_4dparprn_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	// device_t implementation
	virtual tiny_rom_entry const *device_rom_region() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
};



// FIXME: get proper PROM dumps.
/*
    There are most likely multiple revisions.
    6309 and 74471 PROMs were used.

    Example silkscreen:

    SINGAPORE
    341-0005-00
    cAPPLE 78-02
    SN74S471N

    Example label:

    APPLE
    ©1978

    PROM image here is from entering a listing and zero-filling unused areas.

    ********************************
    *                              *
    *   PRINTER CARD I FIRMWARE    *
    *                              *
    *        WOZ   11/1/77         *
    *     APPLE COMPUTER INC.      *
    *     ALL RIGHTS RESERVED      *
    *                              *
    ********************************
*/
ROM_START(parprn)
	ROM_REGION(0x100, "prom", 0)
	ROM_LOAD( "prom.b4", 0x0000, 0x0100, BAD_DUMP CRC(00b742ca) SHA1(c67888354aa013f9cb882eeeed924e292734e717) )
ROM_END

ROM_START(4dparprn)
	ROM_REGION(0x100, "prom", 0)
	ROM_LOAD( "rom.bin", 0x0000, 0x0100, CRC(189262c9) SHA1(d6179664d6860df5ed26fce72f253e28f933b01a) )
	ROM_IGNORE(0x700) // same data is repeated 8 times
ROM_END


INPUT_PORTS_START(parprn)
	PORT_START("CFG")
	PORT_CONFNAME(0x01, 0x00, "Acknowledge latching edge")
	PORT_CONFSETTING(   0x00, "Falling (/Y-B)")
	PORT_CONFSETTING(   0x01, "Rising (Y-B)")
	PORT_CONFNAME(0x06, 0x02, "Printer ready")
	PORT_CONFSETTING(   0x00, "Always (S5-C-D)")
	PORT_CONFSETTING(   0x02, "Acknowledge latch (Z-C-D)")
	PORT_CONFSETTING(   0x04, "ACK (Y-C-D)")
	PORT_CONFSETTING(   0x06, "/ACK (/Y-C-D)")
	PORT_CONFNAME(0x08, 0x00, "Strobe polarity")
	PORT_CONFSETTING(   0x00, "Negative (S5-A-/X, GND-X)")
	PORT_CONFSETTING(   0x08, "Positive (S5-X, GND-A-/X)")
	PORT_CONFNAME(0x10, 0x10, "Character width")
	PORT_CONFSETTING(   0x00, "7-bit")
	PORT_CONFSETTING(   0x10, "8-bit")
INPUT_PORTS_END



a2bus_parprn_device::a2bus_parprn_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_printer_conn(*this, "prn"),
	m_printer_out(*this, "prn_out"),
	m_input_config(*this, "CFG"),
	m_prom(*this, "prom"),
	m_strobe_timer(nullptr),
	m_next_strobe(0U),
	m_ack_latch(0U),
	m_ack_in(1U)
{
}


a2bus_parprn_device::a2bus_parprn_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	a2bus_parprn_device(mconfig, A2BUS_PARPRN, tag, owner, clock)
{
}


a2bus_4dparprn_device::a2bus_4dparprn_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	a2bus_parprn_device(mconfig, A2BUS_4DPARPRN, tag, owner, clock)
{
}


//----------------------------------------------
//  device_a2bus_card_interface implementation
//----------------------------------------------

u8 a2bus_parprn_device::read_c0nx(u8 offset)
{
	if (!machine().side_effects_disabled())
	{
		LOG("Read C0n%01X (effective open-bus write)\n", offset);

		// R/W is ignored, so it has the same effect as a write
		// the bus is open, but 74LS logic has low-impedance inputs so it's likely to latch 0xff
		write_c0nx(offset, 0xffU);
	}

	return 0x00U;
}


void a2bus_parprn_device::write_c0nx(u8 offset, u8 data)
{
	LOG("Write C0n%01X=%02X\n", offset, data);
	if (m_ack_latch)
		LOG("Clearing acknowledge latch\n");
	else
		LOG("Previous data not acknowledged\n");

	ioport_value const cfg(m_input_config->read());

	m_printer_out->write(data & (BIT(cfg, 4) ? 0xffU : 0x7fU));
	m_printer_conn->write_strobe(BIT(~cfg, 3));
	m_next_strobe = BIT(cfg, 3);
	m_ack_latch = 0U;
	m_strobe_timer->adjust(attotime::from_ticks(1, clock()));
}


u8 a2bus_parprn_device::read_cnxx(u8 offset)
{
	ioport_value const cfg(m_input_config->read());

	if (BIT(cfg, 2))
	{
		if (!BIT(offset, 6) || (BIT(offset, 7) && (BIT(cfg, 1) != m_ack_in)))
			offset |= 0x40U;
		else
			offset &= 0xbfU;
	}
	else if (BIT(cfg, 1))
	{
		if (!BIT(offset, 6) || (BIT(offset, 7) && !m_ack_latch))
			offset |= 0x40U;
		else
			offset &= 0xbfU;
	}
	else
	{
		offset ^= 0x40U;
	}

	return m_prom[offset];
}



//----------------------------------------------
//  device_t implementation
//----------------------------------------------

tiny_rom_entry const *a2bus_parprn_device::device_rom_region() const
{
	return ROM_NAME(parprn);
}


tiny_rom_entry const *a2bus_4dparprn_device::device_rom_region() const
{
	return ROM_NAME(4dparprn);
}


void a2bus_parprn_device::device_add_mconfig(machine_config &config)
{
	CENTRONICS(config, m_printer_conn, centronics_devices, "printer");
	m_printer_conn->ack_handler().set(FUNC(a2bus_parprn_device::ack_w));

	OUTPUT_LATCH(config, m_printer_out);
	m_printer_conn->set_output_latch(*m_printer_out);
}


ioport_constructor a2bus_parprn_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(parprn);
}


void a2bus_parprn_device::device_start()
{
	m_strobe_timer = timer_alloc(FUNC(a2bus_parprn_device::update_strobe), this);

	save_item(NAME(m_next_strobe));
	save_item(NAME(m_ack_latch));
	save_item(NAME(m_ack_in));
}


void a2bus_4dparprn_device::device_start()
{
	for (unsigned i = 0U; 0x100 > i; ++i)
		m_prom[i] = (m_prom[i] << 4) | (m_prom[i] >> 4);

	a2bus_parprn_device::device_start();
}


void a2bus_parprn_device::device_reset()
{
	m_ack_latch = 1U;
}



//----------------------------------------------
//  printer status inputs
//----------------------------------------------

void a2bus_parprn_device::ack_w(int state)
{
	if (bool(state) != bool(m_ack_in))
	{
		m_ack_in = state ? 1U : 0U;
		LOG("ACK=%u\n", m_ack_in);
		if (started() && (m_ack_in == BIT(m_input_config->read(), 0)))
		{
			LOG("Active ACK edge\n");
			m_ack_latch = 1U;
		}
	}
}



//----------------------------------------------
//  timer handlers
//----------------------------------------------

TIMER_CALLBACK_MEMBER(a2bus_parprn_device::update_strobe)
{
	ioport_value const cfg(m_input_config->read());

	LOG("Output /STROBE=%u\n", m_next_strobe);
	m_printer_conn->write_strobe(m_next_strobe);
	if (m_next_strobe == BIT(cfg, 3))
	{
		LOG("Start strobe timer\n");
		m_next_strobe = BIT(~cfg, 3);
		m_strobe_timer->adjust(attotime::from_ticks(1, clock()));
	}
}

} // anonymous namespace



DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_PARPRN, device_a2bus_card_interface, a2bus_parprn_device, "a2parprn", "Apple II Parallel Printer Interface Card")
DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_4DPARPRN, device_a2bus_card_interface, a2bus_4dparprn_device, "4dparprn", "Fourth Dimension Parallel Printer Interface")
