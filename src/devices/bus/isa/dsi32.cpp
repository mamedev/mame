// license:BSD-3-Clause
// copyright-holders:Dave Rand
/*********************************************************************

    Definicon Systems DSI-32 coprocessor board

    Host interface (default jumpering, "standard"):
      - 64K memory window at E0000h into the 32032 address space
      - port 160h: page latch, an 8-bit value driving A16-A23 of
        the window's 32032-side address
      - port 150h: bus flags
          bit 0 = set DIAG vector (PAL jams DIA, CPU spins)
          bit 1 = interrupt request to the 32032
          bit 2 = reset the 32032
          bit 3 = inhibit DRAM refresh
    The alternative jumpering moves these to D0000h/2B0h/2A0h.

    32032-side map:
      - DRAM from zero (2MB fitted here; the period kits shipped
        256K or 1MB, but the first 14MB are uniquely decoded)
      - SCN2681 DUART, registers on 4-byte stride, decoded both
        "low" at 200100h (the window's page 20h) and "high" at
        F7FE00h.  The shipped 32IO probes the high decode by
        reading register 0: after the loader's channel setup the
        MR pointer rests on MR2A = 07h, and a readback of 7 means
        the DUART answered.
      - the DUART's OP5 (set by 32IO via SOPR, cleared by the
        host's service loop via ROPR through the window) drives
        the PC's IRQ2.

    The 32032 begins held: the power-on latch in the DIAG vector
    PAL keeps the CPU jammed until the loader's first control
    write.  DIAG-jam and reset are both modelled as holding the
    CPU in reset; execution begins at address 0 (the loader's
    12-byte bootstrap) when the control port goes to 00h.

*********************************************************************/

#include "emu.h"
#include "dsi32.h"

#include "bus/rs232/rs232.h"

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(ISA8_DSI32, isa8_dsi32_device, "dsi32", "Definicon DSI-32 coprocessor")

isa8_dsi32_device::isa8_dsi32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ISA8_DSI32, tag, owner, clock)
	, device_isa8_card_interface(mconfig, *this)
	, m_cpu(*this, "cpu")
	, m_fpu(*this, "fpu")
	, m_mmu(*this, "mmu")
	, m_duart(*this, "duart")
	, m_jumpers(*this, "JUMPERS")
	, m_control(0)
	, m_page(0)
	, m_alt(false)
{
}

static INPUT_PORTS_START(dsi32)
	PORT_START("JUMPERS")
	PORT_CONFNAME(0x01, 0x00, "I/O and window addresses")
	PORT_CONFSETTING(0x00, "Standard (E0000h, ports 150h/160h)")
	PORT_CONFSETTING(0x01, "Alternate (D0000h, ports 2A0h/2B0h)")
INPUT_PORTS_END

ioport_constructor isa8_dsi32_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(dsi32);
}

void isa8_dsi32_device::cpu_map(address_map &map)
{
	map(0x000000, 0x1fffff).ram();
	map(0x200100, 0x20013f).rw(m_duart, FUNC(scn2681_device::read), FUNC(scn2681_device::write)).umask32(0x000000ff);
	map(0xf7fe00, 0xf7fe3f).rw(m_duart, FUNC(scn2681_device::read), FUNC(scn2681_device::write)).umask32(0x000000ff);
}

void isa8_dsi32_device::device_add_mconfig(machine_config &config)
{
	NS32032(config, m_cpu, 10'000'000); // advanced kit; starter kit was 6 MHz
	m_cpu->set_addrmap(0, &isa8_dsi32_device::cpu_map);

	NS32081(config, m_fpu, 10'000'000);
	m_cpu->set_fpu(m_fpu);

	NS32082(config, m_mmu, 10'000'000);
	m_cpu->set_mmu(m_mmu);

	SCN2681(config, m_duart, 3.6864_MHz_XTAL);
	m_duart->outport_cb().set(FUNC(isa8_dsi32_device::duart_op_w));

	rs232_port_device &porta(RS232_PORT(config, "serial1", default_rs232_devices, nullptr));
	m_duart->a_tx_cb().set(porta, FUNC(rs232_port_device::write_txd));
	porta.rxd_handler().set(m_duart, FUNC(scn2681_device::rx_a_w));

	rs232_port_device &portb(RS232_PORT(config, "serial2", default_rs232_devices, nullptr));
	m_duart->b_tx_cb().set(portb, FUNC(rs232_port_device::write_txd));
	portb.rxd_handler().set(m_duart, FUNC(scn2681_device::rx_b_w));
}

void isa8_dsi32_device::device_start()
{
	set_isa_device();

	save_item(NAME(m_control));
	save_item(NAME(m_alt));
	save_item(NAME(m_page));
}

void isa8_dsi32_device::device_reset()
{
	// latch the address-select jumper: remap() may be called with the
	// machine running, and the decode must not change on the fly
	m_alt = BIT(m_jumpers->read(), 0);

	remap(AS_PROGRAM, 0, 0xfffff);
	remap(AS_IO, 0, 0xffff);

	// the DIAG vector PAL's power-on latch holds the CPU until the loader runs
	m_control = 0x01;
	m_page = 0;
	update_hold();
}

void isa8_dsi32_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_PROGRAM)
	{
		const offs_t wbase = m_alt ? 0xd0000 : 0xe0000;

		m_isa->install_memory(wbase, wbase + 0xffff,
				read8sm_delegate(*this, FUNC(isa8_dsi32_device::window_r)),
				write8sm_delegate(*this, FUNC(isa8_dsi32_device::window_w)));
	}
	else if (space_id == AS_IO)
	{
		const offs_t iobase = m_alt ? 0x2a0 : 0x150;
		const offs_t pgbase = m_alt ? 0x2b0 : 0x160;

		m_isa->install_device(iobase, iobase,
				read8smo_delegate(*this, NAME([]() { return 0xff; })),
				write8smo_delegate(*this, FUNC(isa8_dsi32_device::control_w)));
		m_isa->install_device(pgbase, pgbase,
				read8smo_delegate(*this, NAME([this]() { return m_page; })),
				write8smo_delegate(*this, FUNC(isa8_dsi32_device::page_w)));
	}
}

void isa8_dsi32_device::update_hold()
{
	// DIAG-jam or reset both keep the CPU from making progress
	const bool held = (m_control & 0x05) != 0;
	m_cpu->set_input_line(INPUT_LINE_RESET, held ? ASSERT_LINE : CLEAR_LINE);
}

void isa8_dsi32_device::control_w(uint8_t data)
{
	LOG("control_w %02x (DIAG=%d INT32K=%d RESET=%d RFSHI=%d)\n",
			data, BIT(data, 0), BIT(data, 1), BIT(data, 2), BIT(data, 3));

	const uint8_t changed = m_control ^ data;
	m_control = data;

	if (BIT(changed, 0) || BIT(changed, 2))
		update_hold();

	if (BIT(changed, 1))
		m_cpu->set_input_line(INPUT_LINE_IRQ0, BIT(data, 1) ? ASSERT_LINE : CLEAR_LINE);
}

void isa8_dsi32_device::page_w(uint8_t data)
{
	m_page = data;
}

uint8_t isa8_dsi32_device::window_r(offs_t offset)
{
	return m_cpu->space(AS_PROGRAM).read_byte((offs_t(m_page) << 16) | offset);
}

void isa8_dsi32_device::window_w(offs_t offset, uint8_t data)
{
	m_cpu->space(AS_PROGRAM).write_byte((offs_t(m_page) << 16) | offset, data);
}

void isa8_dsi32_device::duart_op_w(uint8_t data)
{
	// pin levels: OP5 low (OPR5 set by 32IO's SOPR) requests the PC's IRQ2
	m_isa->irq2_w(BIT(data, 5) ? 0 : 1);
}
