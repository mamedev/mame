// license:BSD-3-Clause
// copyright-holders:Dave Rand, Patrick Mackinlay
/*********************************************************************

    Opus Systems PM-100 coprocessor board

    Host interface: a 64KB ISA memory window (jumpered to segment
    8000/9000/A000/D000/E000; the monitor probes them in the order
    E000, D000, A000, 9000, 8000).  The window covers the first 64KB
    of board RAM, except that the top sixteen bytes decode the
    control/status register file:

      base+FFF0  R   status: 80 = host-to-32016 interrupt pending
                             40 = 32016-to-host interrupt pending
                             20 = parity error    10 = DMA abort
                             08 = 32016 not running (st_init)
                             04/02/01 = CWT/TSO/CTTL (bus state)
      base+FFF1      eirq: enable 32016-to-host interrupt
      base+FFF2      rirq: reset 32016-to-host interrupt
      base+FFF3      int:  interrupt the 32016 (NVI)
      base+FFF4      nmi:  NMI the 32016
      base+FFF5      go:   start the 32016
      base+FFF6      run:  start the 32016, disable host interrupt
      base+FFF7      rst:  reset and hold the 32016

    FFF1-FFF7 are address strobes: any access, read or write,
    triggers the action (the monitor's probe detects the board by
    alternately *reading* run and rst and watching st_init follow).

    32016 side (physical):
      000000-1FFFFF  DRAM; the communication page, the host command
                     ring and all I/O buffers live in the first 64KB
                     (the kernel maps it at virtual FF0000, "DMA
                     space").  The kernel image is linked at 0 and
                     its first word doubles as the comm page header.
      FFF000  R_WAIT    wait-state generator (scope sync at +4)
      FFF200  R_STAT    status; bit 4 DMA error, bit 5 parity error
      FFF400  R_C_ACK   any write clears the host-to-32016 interrupt
      FFF600  R_C_IRQ   write 1: interrupt the host; write 0: clear

    There is no ICU and no on-board timer: the PM-100 kernel runs
    non-vectored (SETCFG [M,F]), and a single NVI multiplexes clock
    ticks, I/O completions and request handshakes through flags in
    the comm page, with the host's clock driver supplying time.

    The Opus software was preserved by Al Kossow; nearly all of the
    files this emulation was derived from came directly from the
    bitsavers.org archives.

    This driver supersedes the earlier opus108pm skeleton by Patrick
    Mackinlay, whose WIP notes identified the two blockers resolved
    here: the host register layout and the swap-boot WAIT that hangs
    waiting on a PC interrupt (the comm-page interrupt handshake).

*********************************************************************/

#include "emu.h"
#include "opus_pm.h"

#include "endianness.h"

#define VERBOSE 0
#include "logmacro.h"


DEFINE_DEVICE_TYPE(ISA8_OPUS_PM100, isa8_opus_pm100_device, "opus_pm100", "Opus Systems PM-100 coprocessor (NS32016)")
DEFINE_DEVICE_TYPE(ISA8_OPUS_PM110, isa8_opus_pm110_device, "opus_pm110", "Opus Systems PM-110 coprocessor (NS32032)")

// How long a GO/RUN strobe holds the board in reset before the 32016 fetches its
// first instruction.  Models the real RUN/GO reset pulse: it clears stale board
// state and keeps the slave halted long enough for the host's power-up tests to
// read a clean status before the slave can run.
static constexpr unsigned CPU_RESET_HOLD_US = 500'000;  // 500 ms


isa8_opus_pm100_device_base::isa8_opus_pm100_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_isa8_card_interface(mconfig, *this)
	, m_cpu(*this, "cpu")
	, m_fpu(*this, "fpu")
	, m_mmu(*this, "mmu")
	, m_seg(*this, "SEG")
	, m_irq(*this, "IRQ")
	, m_start_timer(nullptr)
	, m_running(false)
	, m_window_top(false)
	, m_irq_enb(false)
	, m_irq_latch(false)
	, m_int_slave(false)
	, m_installed_irq(-1)
{
}

isa8_opus_pm100_device::isa8_opus_pm100_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: isa8_opus_pm100_device_base(mconfig, ISA8_OPUS_PM100, tag, owner, clock)
	, m_ram(*this, "ram", 0x40'0000, ENDIANNESS_LITTLE)
{
}
isa8_opus_pm110_device::isa8_opus_pm110_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: isa8_opus_pm100_device_base(mconfig, ISA8_OPUS_PM110, tag, owner, clock)
	, m_ram(*this, "ram", 0x40'0000, ENDIANNESS_LITTLE)
{
}

static INPUT_PORTS_START(opus_pm100)
	PORT_START("SEG")
	PORT_CONFNAME(0xf000, 0xd000, "Memory window segment")
	PORT_CONFSETTING(0x8000, "8000h")
	PORT_CONFSETTING(0x9000, "9000h")
	PORT_CONFSETTING(0xa000, "A000h")
	PORT_CONFSETTING(0xd000, "D000h")
	PORT_CONFSETTING(0xe000, "E000h")

	PORT_START("IRQ")
	PORT_CONFNAME(0x0f, 0x05, "Host interrupt")
	PORT_CONFSETTING(0x02, "IRQ2")
	PORT_CONFSETTING(0x03, "IRQ3")
	PORT_CONFSETTING(0x05, "IRQ5")
	PORT_CONFSETTING(0x07, "IRQ7")
INPUT_PORTS_END

ioport_constructor isa8_opus_pm100_device_base::device_input_ports() const
{
	return INPUT_PORTS_NAME(opus_pm100);
}

void isa8_opus_pm100_device_base::cpu_map(address_map &map)
{
	// DRAM (the full 4MB, plus its two top-of-space aliases) is installed
	// directly in device_start via install_ram(); see the note there.
	// the four I/O registers decode in two images: at A23 high with
	// A17-A16 selecting the register (800000/810000/820000/830000 =
	// WAIT/STAT/C.ACK/C.IRQ -- the monitor's page tables use these),
	// and as four 512-byte pages at FFF000/FFF200/FFF400/FFF600
	// (the kernel's R_WAIT/R_STAT/R_C_ACK/R_C_IRQ)
	map(0x800000, 0x83ffff).rw(FUNC(isa8_opus_pm100_device_base::slave_reg_r), FUNC(isa8_opus_pm100_device_base::slave_reg_w));
	map(0xfff000, 0xfff7ff).lrw8(
			NAME([this](offs_t offset) { return slave_reg_r((offset & 0x600) << 7); }),
			NAME([this](offs_t offset, u8 data) { slave_reg_w((offset & 0x600) << 7, data); }));
}

void isa8_opus_pm100_device::device_add_mconfig(machine_config &config)
{
	ns32016_device &cpu = NS32016(config, m_cpu, 20_MHz_XTAL / 2);
	cpu.set_addrmap(AS_PROGRAM, &isa8_opus_pm100_device::cpu_map);
	cpu.set_fpu(m_fpu);
	cpu.set_mmu(m_mmu);

	NS32081(config, m_fpu, 20_MHz_XTAL / 2);
	NS32082(config, m_mmu, 20_MHz_XTAL / 2);
}

// Opus 110PM: identical board, NS32032 in place of the NS32016
void isa8_opus_pm110_device::device_add_mconfig(machine_config &config)
{
	ns32032_device &cpu = NS32032(config, m_cpu, 20_MHz_XTAL / 2);
	cpu.set_addrmap(AS_PROGRAM, &isa8_opus_pm110_device::cpu_map);
	cpu.set_fpu(m_fpu);
	cpu.set_mmu(m_mmu);

	NS32081(config, m_fpu, 20_MHz_XTAL / 2);
	NS32082(config, m_mmu, 20_MHz_XTAL / 2);
}

void isa8_opus_pm100_device_base::device_start()
{
	set_isa_device();

	m_start_timer = timer_alloc(FUNC(isa8_opus_pm100_device_base::start_cpu), this);

	save_item(NAME(m_running));
	save_item(NAME(m_window_top));
	save_item(NAME(m_irq_enb));
	save_item(NAME(m_irq_latch));
	save_item(NAME(m_int_slave));
}

void isa8_opus_pm100_device::device_start()
{
	isa8_opus_pm100_device_base::device_start();

	// Install DRAM directly rather than through per-byte map trampolines (the
	// 32016 has a 16-bit data bus, so a trampoline costs two calls per word).
	// The board must be modelled full-size: the slave's power-up routine sizes
	// RAM by tagging the top word of every 512KB bank across 0-4MB and reading
	// it back, so a short array leaves banks unmapped and sizing runs away.
	// The same array also answers the top of the 16MB space -- FF0000-FFEFFF is
	// the "DMA space" window where the monitor relocates and keeps the comm
	// page, and FFFE00-FFFFFF mirrors the interrupt stack.
	m_cpu->space(AS_PROGRAM).install_ram(0x00'0000, 0x3f'ffff, &m_ram[0x00'0000 >> 1]);
	m_cpu->space(AS_PROGRAM).install_ram(0xff'0000, 0xff'efff, &m_ram[0x3f'0000 >> 1]);
	m_cpu->space(AS_PROGRAM).install_ram(0xff'fe00, 0xff'ffff, &m_ram[0x3f'fe00 >> 1]);
}

void isa8_opus_pm110_device::device_start()
{
	isa8_opus_pm100_device_base::device_start();

	// Install DRAM directly rather than through per-byte map trampolines (the
	// 32032 has a 32-bit data bus, so a trampoline costs four calls per dword).
	// The board must be modelled full-size: the slave's power-up routine sizes
	// RAM by tagging the top word of every 512KB bank across 0-4MB and reading
	// it back, so a short array leaves banks unmapped and sizing runs away.
	// The same array also answers the top of the 16MB space -- FF0000-FFEFFF is
	// the "DMA space" window where the monitor relocates and keeps the comm
	// page, and FFFE00-FFFFFF mirrors the interrupt stack.
	m_cpu->space(AS_PROGRAM).install_ram(0x00'0000, 0x3f'ffff, &m_ram[0x00'0000 >> 2]);
	m_cpu->space(AS_PROGRAM).install_ram(0xff'0000, 0xff'efff, &m_ram[0x3f'0000 >> 2]);
	m_cpu->space(AS_PROGRAM).install_ram(0xff'fe00, 0xff'ffff, &m_ram[0x3f'fe00 >> 2]);
}

void isa8_opus_pm100_device_base::device_reset()
{
	remap(AS_PROGRAM, 0, 0xfffff);

	reset_card();

	m_installed_irq = -1;
}

// Return the slave subsystem to its power-on state.  Driven both by the host's
// rst CSR strobe and by a machine reset, so the two can never drift apart -- a
// reloaded monitor must see exactly the board a cold boot would.  The CPU is
// held in reset (released by go/run, at which point its device_reset runs); the
// MMU returns to physical mode and the FPU plus every host-interface latch --
// including the 32016-to-host interrupt ENABLE, which the run strobe also clears
// but rst previously left set -- go back to their reset values.
void isa8_opus_pm100_device_base::reset_card()
{
	set_running(false);
	m_mmu->reset();   // MSR=0 -> physical mode, regardless of prior run state
	m_fpu->reset();
	m_window_top = false;
	m_irq_enb = false;
	m_int_slave = false;
	m_cpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
	set_host_irq(false);
}

void isa8_opus_pm100_device_base::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_PROGRAM)
	{
		offs_t const base = offs_t(m_seg->read()) << 4;
		// the shared comm-page window; the host CSRs sit in the last 16 bytes
		m_isa->install_memory(base, base + 0xffef,
				emu::rw_delegate(*this, FUNC(isa8_opus_pm100_device_base::window_r)),
				emu::rw_delegate(*this, FUNC(isa8_opus_pm100_device_base::window_w)));
		// address strobes at base+FFF1..FFF7 (the offset is the register number);
		// any access fires, so install for both read and write
		m_isa->install_memory(base + 0xfff0, base + 0xfff7,
				emu::rw_delegate(*this, FUNC(isa8_opus_pm100_device_base::csr_strobe_r)),
				emu::rw_delegate(*this, FUNC(isa8_opus_pm100_device_base::csr_strobe_w)));
		// the status register at base+FFF0 overrides the strobe image there
		m_isa->install_memory(base + 0xfff0, base + 0xfff0,
				emu::rw_delegate(*this, FUNC(isa8_opus_pm100_device_base::csr_r)),
				emu::rw_delegate(*this, FUNC(isa8_opus_pm100_device_base::csr_w)));
	}
}

void isa8_opus_pm100_device_base::set_running(bool running)
{
	bool const stopping = m_running && !running;
	m_running = running;
	m_cpu->set_input_line(INPUT_LINE_RESET, running ? CLEAR_LINE : ASSERT_LINE);
	if (stopping)
	{
		m_mmu->reset();
		m_fpu->reset();
	}
}

// Reset-hold elapsed: release the 32016 to run from its clean power-on state
// (the CPU reset line clears here, so its device_reset runs -- PC=0, CFG cleared).
TIMER_CALLBACK_MEMBER(isa8_opus_pm100_device_base::start_cpu)
{
	set_running(true);
}

void isa8_opus_pm100_device_base::update_host_irq()
{
	int const state = (m_irq_latch && m_irq_enb) ? ASSERT_LINE : CLEAR_LINE;
	int const line = m_irq->read();

	if (m_installed_irq >= 0 && m_installed_irq != line && state == ASSERT_LINE)
		irq_line_w(m_installed_irq, CLEAR_LINE);

	irq_line_w(line, state);
	m_installed_irq = line;
}

void isa8_opus_pm100_device_base::irq_line_w(int line, int state)
{
	switch (line)
	{
	case 2: m_isa->irq2_w(state); break;
	case 3: m_isa->irq3_w(state); break;
	case 5: m_isa->irq5_w(state); break;
	case 7: m_isa->irq7_w(state); break;
	}
}

void isa8_opus_pm100_device_base::set_host_irq(bool state)
{
	m_irq_latch = state;
	update_host_irq();
}

// Map a host-window offset to the physical DRAM cell it reaches.  Per the User
// Manual (8.1, 8.3): the 64KB window is NOT a fixed page.  In the INITIALIZE state
// the address top 8 bits are forced to 0 -- the PC sees the first 60K of physical
// low memory (the boot download).  After the RUN command they are forced to 1, so
// each window access is the virtual address 0xFF0000|offset and is translated by the
// 32082 MMU -- exactly the path the slave's own DMASPACE (0xFF0000) accesses take.
// This makes the host and the MMU-mapped slave (opconfig) reach the comm page through
// the same map, instead of our old fixed stand-in.
offs_t isa8_opus_pm100_device_base::window_phys(offs_t offset, bool write)
{
	if (!m_window_top || !m_mmu)
		return offset & 0x3fffff;                          // INITIALIZE: physical low
	u32 addr = 0xff0000 | offset;                          // RUN: top 8 bits forced to 1
	if (m_mmu->translate(m_cpu->space(AS_PROGRAM), ns32000::ST_ODT, addr, false, write, false, true /*suppress faults*/)
			== ns32000_mmu_interface::COMPLETE)
		return addr & 0x3fffff;
	return (0xff0000 | offset) & 0x3fffff;                 // untranslated fallback
}

// the window covers the shared page (the host CSRs at the top are installed separately)
uint8_t isa8_opus_pm100_device::window_r(offs_t offset)
{
	return util::little_endian_cast<uint8_t const>(&m_ram[0])[window_phys(offset, false)];
}

uint8_t isa8_opus_pm110_device::window_r(offs_t offset)
{
	return util::little_endian_cast<uint8_t const>(&m_ram[0])[window_phys(offset, false)];
}

void isa8_opus_pm100_device::window_w(offs_t offset, uint8_t data)
{
	util::little_endian_cast<uint8_t>(&m_ram[0])[window_phys(offset, true)] = data;
}

void isa8_opus_pm110_device::window_w(offs_t offset, uint8_t data)
{
	util::little_endian_cast<uint8_t>(&m_ram[0])[window_phys(offset, true)] = data;
}

// CSR registers 1-7 are address strobes: reads trigger too
void isa8_opus_pm100_device_base::csr_strobe(offs_t reg)
{
	switch (reg)
	{
	case 1: // eirq: enable interrupt to host
		m_irq_enb = true;
		update_host_irq();
		break;

	case 2: // rirq: reset interrupt from 32016
		set_host_irq(false);
		break;

	case 3: // int: interrupt the 32016
		m_int_slave = true;
		m_cpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
		break;

	case 4: // nmi
		m_cpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
		break;

	case 5: // go
	case 6: // run (also disables the host interrupt)
		// Start the 32016.  Every start first resets the whole board and holds it
		// in reset for a few milliseconds before the first instruction executes --
		// the real RUN/GO sequence pulses reset, and the host's power-up tests
		// rely on a clean board (without this, stale MMU/comm state from a prior
		// program makes the Level-2 test overflow "as if reset was not done", and
		// a resident image can run to its C.IRQ before the host reads status).
		// reset_card() clears every latch including the host-interrupt enable, so
		// it covers RUN's "disable host interrupt" too.  start_cpu releases it.
		reset_card();
		// Report the board as RUNNING immediately (st_init clears) so the host's
		// run/stop probe sees the strobe take effect -- the CPU itself stays held
		// by the reset line until start_cpu fires; only its first fetch is delayed.
		m_running = true;
		m_start_timer->adjust(attotime::from_usec(CPU_RESET_HOLD_US));
		break;

	case 7: // rst: reset and hold the whole card.  Per the manual this stops the
		// CPU and forces the MMU into physical mode; it must restore the full
		// power-on baseline (translation off, interrupt enable/latches clear)
		// so that re-running the host monitor -- e.g. quitting opconfig to DOS
		// and reloading after doing real work -- starts from a clean board.
		reset_card();
		break;
	}
}

// host status register (base+FFF0)
uint8_t isa8_opus_pm100_device_base::csr_r(offs_t offset)
{
	uint8_t stat = 0;
	if (m_int_slave)
		stat |= 0x80;
	if (m_irq_latch)
		stat |= 0x40;
	if (!m_running)
		stat |= 0x08;
	// live bus-state bits, as the host samples them: TSO (low only
	// during a TCU cycle) always reads high, CWT (low only during
	// refresh) and CTTL (the slave clock) toggle.  The monitor's
	// power-up test accumulates AND/OR over a thousand reads and
	// requires TSO constant and at least one of the others moving.
	uint64_t const phase = machine().time().as_ticks(m_cpu->clock());
	stat |= 0x02;                               // TSO
	stat |= (phase & 1) ? 0x01 : 0x00;          // CTTL
	stat |= ((phase % 31) != 0) ? 0x04 : 0x00;  // CWT low on refresh only
	return stat;
}

void isa8_opus_pm100_device_base::csr_w(offs_t offset, uint8_t data)
{
	// the status register is read-only
}

// the address strobes (base+FFF1..FFF7): a read fires the strobe too, so guard
// it against the debugger's side-effect-free probes; a write always fires
uint8_t isa8_opus_pm100_device_base::csr_strobe_r(offs_t reg)
{
	if (!machine().side_effects_disabled())
		csr_strobe(reg);
	return 0xff;
}

void isa8_opus_pm100_device_base::csr_strobe_w(offs_t reg, uint8_t data)
{
	csr_strobe(reg);
}

uint8_t isa8_opus_pm100_device_base::slave_reg_r(offs_t offset)
{
	// R_STAT: no DMA or parity errors are ever modelled
	return 0;
}

void isa8_opus_pm100_device_base::slave_reg_w(offs_t offset, uint8_t data)
{
	// A 32016 held in reset (the board stopped, between the host's rst strobe
	// and the next go/run) drives nothing on the bus.  Our CPU reset is applied
	// at the next sync, so it can slip an instruction or two of stale DRAM after
	// rst; ignore any control writes those produce, exactly as the real board's
	// RST would gate them off, so a stopped slave can never raise C.IRQ.
	if (!m_running)
		return;

	switch (offset & 0x30000)
	{
	case 0x00000: // WAIT (and the scope-sync flip-flop)
		break;

	case 0x10000: // STAT: reset DMA/parity error flags
		break;

	case 0x20000: // C.ACK: any write clears the host-to-32016 interrupt
		m_int_slave = false;
		m_cpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
		break;

	case 0x30000: // C.IRQ: any write raises the 32016-to-host interrupt
		// (the shell's IRQPC() writes 0); only the host's rirq strobe
		// clears the latch.  This is also where the monitor first
		// announces it has relocated, after which the host window tracks
		// the slave through the MMU (see window_phys / m_window_top).
		m_window_top = true;
		set_host_irq(true);
		break;
	}
}
