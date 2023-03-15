// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Opus 100PM (Personal Mainframe) UNIX processor subsystem
 *
 * Sources:
 *  - http://bitsavers.org/pdf/opusSystems/32k/800-00237-000_Opus_100pm_User_Manual_1987.pdf
 *
 * TODO
 *  - 32032-based 110PM
 */

 /*
 * WIP
 * ---
 * Using IBM 5160 as host avoids boot problem with 5170 and issues with
 * shadowing in ct486.
 *
 *  - opus ibm5160 -window -nomax -hard1 opus_c.chd -hard2 opus_d.chd -isa5 opus108pm
 *
 * System configuration and software installation from boot and kernel floppies
 * appears successful. Valid Systems hard disk images appear to expect a card
 * with different registers?
 *
 * Booting Opus5 from swap hangs with WAIT instruction at 0xf0087c, presume
 * requires an interrupt from PC which never arrives. Maybe clock device isn't
 * working properly?
 */

#include "emu.h"
#include "opus100pm.h"

#define LOG_GENERAL (1U << 0)
#define LOG_STAT    (1U << 1)
#define LOG_REG     (1U << 2)

//#define VERBOSE (LOG_GENERAL|LOG_REG)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(ISA8_OPUS108PM, isa8_opus108pm_device, "opus108pm", "Opus 108PM")

// CPU bit definition here matches software, but differs from documentation
enum card_stat_mask : u8
{
	CARD_STAT_CPU  = 0x01, // CPU type (0=32016, 1=32032)
	CARD_STAT_EIRQ = 0x04, // IRQ enabled
	CARD_STAT_OPT  = 0x08, // option switch open
	CARD_STAT_DMA  = 0x10, // DMA abort
	CARD_STAT_PAR  = 0x20, // parity error
	CARD_STAT_IRQ  = 0x40, // interrupt to host active
	CARD_STAT_INT  = 0x80, // interrupt to card active
};

// the three low bits in the host status register probably correspond to
// unemulated CTTL, /TSO and /CWAIT signals from the TCU
enum host_stat_mask : u8
{
	HOST_STAT_CTTL = 0x01, // CTTL (diagnostic only)
	HOST_STAT_TSO  = 0x02, // TSO* (diagnostic only)
	HOST_STAT_CWT  = 0x04, // CWT* (diagnostic only)
	HOST_STAT_RUN  = 0x08, // RUN* (diagnostic only)
	HOST_STAT_DMA  = 0x10, // DMA abort
	HOST_STAT_PAR  = 0x20, // parity error
	HOST_STAT_IRQ  = 0x40, // interrupt to host active
	HOST_STAT_INT  = 0x80, // interrupt to card active
};

static INPUT_PORTS_START(opus108pm)
	PORT_START("BASE")
	PORT_DIPNAME(0xf0000, 0xa0000, "Base Address")
	PORT_DIPSETTING(      0x00000, "00000")
	PORT_DIPSETTING(      0x10000, "10000")
	PORT_DIPSETTING(      0x20000, "20000")
	PORT_DIPSETTING(      0x30000, "30000")
	PORT_DIPSETTING(      0x40000, "40000")
	PORT_DIPSETTING(      0x50000, "50000")
	PORT_DIPSETTING(      0x60000, "60000")
	PORT_DIPSETTING(      0x70000, "70000")
	PORT_DIPSETTING(      0x80000, "80000")
	PORT_DIPSETTING(      0x90000, "90000")
	PORT_DIPSETTING(      0xa0000, "A0000")
	PORT_DIPSETTING(      0xb0000, "B0000")
	PORT_DIPSETTING(      0xc0000, "C0000")
	PORT_DIPSETTING(      0xd0000, "D0000")
	PORT_DIPSETTING(      0xe0000, "E0000")
	PORT_DIPSETTING(      0xf0000, "F0000")

	PORT_START("IRQ")
	PORT_DIPNAME(0x0f, 0x07, "IRQ")
	PORT_DIPSETTING(   0x02, "IRQ 2")
	PORT_DIPSETTING(   0x03, "IRQ 3")
	PORT_DIPSETTING(   0x07, "IRQ 7")
INPUT_PORTS_END

isa8_opus108pm_device::isa8_opus108pm_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ISA8_OPUS108PM, tag, owner, clock)
	, device_isa8_card_interface(mconfig, *this)
	, m_cpu(*this, "cpu")
	, m_fpu(*this, "fpu")
	, m_mmu(*this, "mmu")
	, m_ram(*this, "ram")
	, m_base(*this, "BASE")
	, m_irq(*this, "IRQ")
	, m_installed(false)
{
}

void isa8_opus108pm_device::device_add_mconfig(machine_config &config)
{
	NS32016(config, m_cpu, 20_MHz_XTAL / 2);
	m_cpu->set_addrmap(0, &isa8_opus108pm_device::map_cpu);

	NS32081(config, m_fpu, 20_MHz_XTAL / 2);
	m_cpu->set_fpu(m_fpu);

	NS32082(config, m_mmu, 20_MHz_XTAL / 2);
	m_cpu->set_mmu(m_mmu);

	RAM(config, m_ram);
	m_ram->set_default_size("1MiB");
	m_ram->set_extra_options("2MiB");
}

ioport_constructor isa8_opus108pm_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(opus108pm);
}

void isa8_opus108pm_device::device_start()
{
	set_isa_device();

	if (!m_ram->started())
		throw device_missing_dependencies();

	save_item(NAME(m_card_stat));
	save_item(NAME(m_host_stat));

	m_cpu->space(0).install_ram(0, m_ram->mask(), m_ram->pointer());
}

void isa8_opus108pm_device::device_reset()
{
	if (!m_installed)
	{
		u32 const base = m_base->read();

		m_isa->install_memory(base, base | 0xffff, *this, &isa8_opus108pm_device::map_isa);

		m_installed = true;
	}

	m_card_stat = CARD_STAT_OPT;
	m_host_stat = HOST_STAT_RUN;
}

void isa8_opus108pm_device::device_reset_after_children()
{
	m_cpu->suspend(SUSPEND_REASON_HALT, true);
	m_cpu->suspend(SUSPEND_REASON_RESET, false);
}

void isa8_opus108pm_device::map_cpu(address_map &map)
{
	// TODO: should unmapped memory accesses cause parity errors and NMI?

	// silence double-word access to byte-sized registers
	map(0x800000, 0x800003).noprw().mirror(0x030000);

	map(0x800000, 0x800000).r(FUNC(isa8_opus108pm_device::card_wait_r));
	map(0x810000, 0x810000).r(FUNC(isa8_opus108pm_device::card_stat_r));
	map(0x810000, 0x810000).w(FUNC(isa8_opus108pm_device::card_rste_w));
	map(0x820000, 0x820000).w(FUNC(isa8_opus108pm_device::card_ack_w));
	map(0x830000, 0x830000).w(FUNC(isa8_opus108pm_device::card_irq_w));
}

void isa8_opus108pm_device::map_isa(address_map &map)
{
	map(0x0000, 0xefff).lrw8(
		[this](offs_t offset)
		{
			if (!(m_host_stat & HOST_STAT_RUN))
				offset |= 0xff0000;

			// FIXME: invalid translation probably triggers "DMA abort" status
			// and generates an NMI; unsure what else is reported to the host
			if (m_mmu->translate(m_cpu->space(0), 0xa, offset, false, false) != ns32000_mmu_interface::COMPLETE)
				fatalerror("%s: host address translation failed (%s)\n", tag(), machine().describe_context());

			return m_ram->read(offset);
		}, "ram_r",
		[this](offs_t offset, u8 data)
		{
			if (!(m_host_stat & HOST_STAT_RUN))
				offset |= 0xff0000;

			// FIXME: invalid translation probably triggers "DMA abort" status
			// and generates an NMI; unsure what else is reported to the host
			if (m_mmu->translate(m_cpu->space(0), 0xa, offset, false, true) != ns32000_mmu_interface::COMPLETE)
				fatalerror("%s: host address translation failed (%s)\n", tag(), machine().describe_context());

			m_ram->write(offset, data);
		}, "ram_w");

	// software expects these registers at 0xfffN, not 0xf00N per documentation,
	// assume they are mirrored throughout the whole 4k range
	map(0xf000, 0xf000).mirror(0x0ff0).r(FUNC(isa8_opus108pm_device::host_stat_r));
	map(0xf001, 0xf001).mirror(0x0ff0).w(FUNC(isa8_opus108pm_device::host_eirq_w));
	map(0xf002, 0xf002).mirror(0x0ff0).w(FUNC(isa8_opus108pm_device::host_ack_w));
	map(0xf003, 0xf003).mirror(0x0ff0).w(FUNC(isa8_opus108pm_device::host_int_w));
	map(0xf004, 0xf004).mirror(0x0ff0).w(FUNC(isa8_opus108pm_device::host_nmi_w));
	map(0xf005, 0xf005).mirror(0x0ff0).w(FUNC(isa8_opus108pm_device::host_go_w));
	map(0xf006, 0xf006).mirror(0x0ff0).w(FUNC(isa8_opus108pm_device::host_run_w));
	map(0xf007, 0xf007).mirror(0x0ff0).w(FUNC(isa8_opus108pm_device::host_rst_w));
}

void isa8_opus108pm_device::update_isa_irq(int state)
{
	LOG("update_isa_irq %d\n", state);

	switch (m_irq->read())
	{
	case 2: m_isa->irq2_w(state); break;
	case 3: m_isa->irq3_w(state); break;
	case 7: m_isa->irq7_w(state); break;

	default:
		fatalerror("%s: invalid isa irq %d\n", tag(), m_irq->read());
	}
}

u8 isa8_opus108pm_device::card_stat_r()
{
	LOGMASKED(LOG_STAT, "card_stat_r 0x%02x (%s)\n", m_card_stat, machine().describe_context());

	return m_card_stat;
}

void isa8_opus108pm_device::card_rste_w(u8 data)
{
	LOG("card_rste_w (%s)\n", machine().describe_context());

	m_card_stat &= ~(CARD_STAT_PAR | CARD_STAT_DMA);
	m_host_stat &= ~(HOST_STAT_PAR | HOST_STAT_DMA);
}

void isa8_opus108pm_device::card_ack_w(u8 data)
{
	if (m_card_stat & CARD_STAT_INT)
	{
		LOG("card_ack_w (%s)\n", machine().describe_context());
		m_card_stat &= ~CARD_STAT_INT;
		m_host_stat &= ~HOST_STAT_INT;
		m_cpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
	}
}

void isa8_opus108pm_device::card_irq_w(u8 data)
{
	if (!(m_card_stat & CARD_STAT_IRQ))
	{
		LOG("card_irq_w (%s)\n", machine().describe_context());
		m_card_stat |= CARD_STAT_IRQ;
		m_host_stat |= HOST_STAT_IRQ;

		if (m_card_stat & CARD_STAT_EIRQ)
			update_isa_irq(1);
	}
}

u8 isa8_opus108pm_device::host_stat_r()
{
	// HACK: diagnostic test requires changes in this host status bit, this
	// arbitrary toggle satisfies the test but is not accurate
	m_host_stat ^= HOST_STAT_CWT;

	LOGMASKED(LOG_STAT, "host_stat_r 0x%02x (%s)\n", m_host_stat, machine().describe_context());
	return m_host_stat;
}

void isa8_opus108pm_device::host_eirq_w(u8 data)
{
	if (!(m_card_stat & CARD_STAT_EIRQ))
	{
		LOG("host_eirq_w (%s)\n", machine().describe_context());
		m_card_stat |= CARD_STAT_EIRQ;

		if (m_card_stat & CARD_STAT_IRQ)
			update_isa_irq(1);
	}
}

void isa8_opus108pm_device::host_ack_w(u8 data)
{
	if (m_host_stat & HOST_STAT_IRQ)
	{
		LOG("host_ack_w (%s)\n", machine().describe_context());
		m_card_stat &= ~CARD_STAT_IRQ;
		m_host_stat &= ~HOST_STAT_IRQ;

		update_isa_irq(0);
	}
}

void isa8_opus108pm_device::host_int_w(u8 data)
{
	if (!(m_host_stat & HOST_STAT_INT))
	{
		LOG("host_int_w (%s)\n", machine().describe_context());
		m_card_stat |= CARD_STAT_INT;
		m_host_stat |= HOST_STAT_INT;
		m_cpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
	}
}

void isa8_opus108pm_device::host_nmi_w(u8 data)
{
	LOG("host_nmi_w (%s)\n", machine().describe_context());
	m_cpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	m_cpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

void isa8_opus108pm_device::host_go_w(u8 data)
{
	LOG("host_go_w (%s)\n", machine().describe_context());

	m_cpu->resume(SUSPEND_REASON_RESET);
}

void isa8_opus108pm_device::host_run_w(u8 data)
{
	LOG("host_run_w (%s)\n", machine().describe_context());

	m_host_stat &= ~HOST_STAT_RUN;
	m_cpu->resume(SUSPEND_REASON_HALT);
}

void isa8_opus108pm_device::host_rst_w(u8 data)
{
	LOG("host_rst_w (%s)\n", machine().describe_context());

	m_card_stat &= (CARD_STAT_OPT | CARD_STAT_CPU);
	m_host_stat = HOST_STAT_RUN;

	m_cpu->reset();
	m_fpu->reset();
	m_mmu->reset();

	m_cpu->suspend(SUSPEND_REASON_HALT, true);
	m_cpu->suspend(SUSPEND_REASON_RESET, false);
}
