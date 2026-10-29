// license:BSD-3-Clause
// copyright-holders:Olivier Galibert,Brian H. Gilbert
/***************************************************************************

    rp2a03.cpp

    6502, NES variant

***************************************************************************/

#include "emu.h"
#include "rp2a03.h"
#include "rp2a03d.h"

DEFINE_DEVICE_TYPE(RP2A03_CORE, rp2a03_core_device, "rp2a03_core", "Ricoh RP2A03 core") // needed for some VT systems with XOP instead of standard APU
DEFINE_DEVICE_TYPE(RP2A03,      rp2a03_device,      "rp2a03",      "Ricoh RP2A03")      // earliest version, found in punchout, spnchout, dkong3, VS. systems, and some early Famicoms
DEFINE_DEVICE_TYPE(RP2A03G,     rp2a03g_device,     "rp2a03g",     "Ricoh RP2A03G")     // later revision, found in front-loader NES


void rp2a03_device::rp2a03_map(address_map &map)
{
	map(0x4000, 0x4013).w(m_apu, FUNC(nesapu_device::write));
	map(0x4015, 0x4015).lw8(NAME([this](u8 data) { m_apu->write(0x15, data); }));
	map(0x4017, 0x4017).lw8(NAME([this](u8 data) { m_apu->write(0x17, data); }));
	// bit 5 of $4015 is not driven: it is whatever the CPU's internal data bus last held, which DMA cycles never
	// change (AccuracyCoin "Internal Data Bus" test 2)
	map(0x4015, 0x4015).lr8(NAME([this]() { return u8((m_apu->status_r() & ~0x20) | (m_ibus & 0x20)); }));
	map(0x4000, 0x4014).lr8(NAME([this](offs_t) { return m_bus; }));
	map(0x4018, 0x401f).lr8(NAME([this](offs_t) { return m_bus; }));
	// 0x4014 w -> NES sprite DMA (is this internal?)
	// 0x4016 w -> d0-d2: RP2A03 OUT0,OUT1,OUT2
	// 0x4016 r -> d0-d4: RP2A03 IN0
	// 0x4017 r -> d0-d4: RP2A03 IN1
}



rp2a03_core_device::rp2a03_core_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: m6502_device(mconfig, type, tag, owner, clock)
	, m_m2_cb(*this)
{
	set_address_width(16, true);
}

rp2a03_core_device::rp2a03_core_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: rp2a03_core_device(mconfig, RP2A03_CORE, tag, owner, clock)
{
}



rp2a03_device::rp2a03_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: rp2a03_core_device(mconfig, type, tag, owner, clock)
	, device_mixer_interface(mconfig, *this)
	, m_apu(*this, "nesapu")
{
	m_program_config.m_internal_map = address_map_constructor(FUNC(rp2a03_device::rp2a03_map), this);
}

rp2a03_device::rp2a03_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: rp2a03_device(mconfig, RP2A03, tag, owner, clock)
{
	// the original letterless 2A03 holds M2 high longer (NESdev "CPU pinout": 17/24)
	set_m2_waveform(7, 24);
	// its writes are made where the 2A03G makes them: the written value is not on the bus before phi2 on either chip
	set_m2_write_point(9);
}

rp2a03g_device::rp2a03g_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: rp2a03_device(mconfig, RP2A03G, tag, owner, clock)
{
}



std::unique_ptr<util::disasm_interface> rp2a03_core_device::create_disassembler()
{
	return std::make_unique<rp2a03_disassembler>();
}

void rp2a03_device::apu_irq(int state)
{
	// the APU is inside the chip: its IRQ output is a level, set right away, rather than an input line change MAME queues
	// until the APU returns
	m_apu_irq_state = (state != 0);
	// a change at or before the last phi2 fall (an APU timer on a cycle boundary) was seen by that sample
	if (machine().time() <= m_last_phi2)
		m_irq_phi2_level = m_irq_state || m_apu_irq_state;
}

void rp2a03_core_device::device_start()
{
	m_mintf = std::make_unique<mi_2a03>(*this);
	m6502_device::device_start();
	m2_update_rise();

	// the 2A03 samples IRQ when phi2 falls, whatever drives it (NESdev "CPU interrupts")
	m_irq_at_phi2 = true;

	// NESdev CPU power up state: A, X and Y are 0 and P is $34 (the reset sequence then leaves S at $FD);
	// a later reset leaves A, X and Y alone
	m_A = m_X = m_Y = 0;
	m_P = 0x34;
	save_item(NAME(m_bus));
	save_item(NAME(m_ibus));
	save_item(NAME(m_m2_state));
	save_item(NAME(m_nmi_sample));
	save_item(NAME(m_irq_phi2_level));
	save_item(NAME(m_last_phi2));
	save_item(NAME(m_irq_assert_at));
	save_item(NAME(m_irq_reported_at));
	save_item(NAME(m_oam_active));
	save_item(NAME(m_oam_first));
	save_item(NAME(m_oam_halt));
	save_item(NAME(m_oam_aligned));
	save_item(NAME(m_oam_page));
	save_item(NAME(m_oam_index));
	save_item(NAME(m_oam_byte));
	save_item(NAME(m_dmc_req));
	save_item(NAME(m_dmc_halt));
	save_item(NAME(m_dmc_block));
	save_item(NAME(m_last_dummy_adr));
}

void rp2a03_core_device::device_reset()
{
	m_nmi_sample = false;
	m_irq_phi2_level = false;
	m_last_phi2 = attotime::zero;
	m_irq_assert_at = attotime::never;
	m6502_device::device_reset();
	m_m2_state = false;
	m_m2_depth = 0;
	set_bus_offset(attotime::zero);
	m_oam_active = m_oam_first = m_oam_halt = m_oam_aligned = false;
	m_dmc_req = m_dmc_halt = false;
	m_dmc_block = 0;
	m_rdy_state = true;
}

void rp2a03_core_device::oam_dma_request(u8 page)
{
	// TriCNES: the sprite DMA starts on the next CPU read cycle; if that is a get cycle it is a halt cycle first
	m_oam_page = page;
	m_oam_index = 0;
	m_oam_active = true;
	m_oam_first = true;
	m_oam_halt = false;
	m_oam_aligned = false;
	m_rdy_state = false;
}

// the DMA unit alternates "get" (read) and "put" (write) cycles, also when idle. NESdev does not say which
// absolute cycles are gets (it is random per power-on on hardware); counting CPU cycles from power-on, the even
// ones are (cpu_interrupts_v2 4-irq_and_dma, sprdma_and_dmc_dma and sprdma_and_dmc_dma_512 all pass this way)
bool rp2a03_core_device::dma_get_cycle(u64 cycle) const
{
	return (cycle & 1) == 0;
}

// the APU asks for a DMC DMA: a timer (reload) fetch at a byte boundary, or the load after a $4015 enable; a fetch
// already pending absorbs it, and a timer fetch cannot start right after a DMC get (TriCNES CannotRunDMCDMARightNow)
void rp2a03_core_device::dmc_request(bool load)
{
	if (m_dmc_req || (!load && (m_dmc_block == 2)))
		return;
	m_dmc_req = true;
	m_dmc_halt = true;
}

// After the CPU's access of every cycle: the APU's part of the cycle, then RDY for the next one. RDY only stops
// read cycles: a DMA requested while the CPU writes waits for its next read.
void rp2a03_core_device::execute_set_input(int inputnum, int state)
{
	// with the edge detector reading /NMI itself, the input line only records the level
	if ((inputnum == NMI_LINE) && m_nmi_level)
	{
		m_nmi_state = (state == ASSERT_LINE);
		return;
	}
	if (m_irq_at_phi2 && ((inputnum == IRQ_LINE) || (inputnum == APU_IRQ_LINE)))
	{
		m6502_device::execute_set_input(inputnum, state);

		// MAME queues an input line change until the device that made it returns, and delivers it at the time it was
		// made: a change made at or before the last phi2 fall was seen by that sample
		if (machine().time() <= m_last_phi2)
			m_irq_phi2_level = m_irq_state || m_apu_irq_state;

		// an IRQ the reporting device only found out about after it was asserted (an A12 edge the PPU finds late): if
		// it was asserted before the last phi2 sample, that sample saw it. The reported time applies to the IRQ line
		// change that follows the report within a cycle, and to nothing later.
		if (inputnum == IRQ_LINE)
		{
			if ((state == ASSERT_LINE) && (m_irq_assert_at != attotime::never)
					&& (machine().time() <= m_irq_reported_at + cycles_to_attotime(1)) && (m_irq_assert_at <= m_last_phi2))
				m_irq_phi2_level = true;
			m_irq_assert_at = attotime::never;
		}
		return;
	}
	m6502_device::execute_set_input(inputnum, state);
}

void rp2a03_core_device::cycle_end()
{
	if (m_nmi_level)
	{
		// phi2 falls when the cycle ends; don't accept an edge on the cycle RESET is released (as the input line does)
		const bool low = m_nmi_level(local_time() - bus_offset() + cycles_to_attotime(1));
		if (low && !m_nmi_sample && total_cycles())
			m_nmi_pending = true;
		m_nmi_sample = low;
	}
	const bool get = dma_get_cycle(total_cycles());
	if (m_dmc_req)
		dmc_implicit_clear();   // the implicit-abort window lasts until the cycle after the fetch is pending
	apu_cycle_end(get);
	if (m_irq_at_phi2)
	{
		m_irq_phi2_level = m_irq_state || m_apu_irq_state;   // phi2 falls as the cycle ends, after the APU's own IRQs
		m_last_phi2 = local_time() - bus_offset() + cycles_to_attotime(1);
	}
	if (get && (m_dmc_block > 0))
		m_dmc_block -= 2;
	m_rdy_state = !((m_dmc_req && dmc_dma_enabled()) || m_oam_active);
}

void rp2a03_core_device::dma_halt_cycle(uint16_t adr)
{
	// One CPU read cycle taken by the DMA unit (TriCNES's DMA cycle). Get cycles read for the DMA unit, put cycles
	// write OAM; a halted DMA, a DMC put and a sprite DMA's alignment put let the halted CPU repeat its read. Both
	// halt flags clear after a get cycle. With both DMAs running, the DMC wins get cycles and the sprite DMA put cycles.
	const bool get = dma_get_cycle(total_cycles());
	if (m_oam_active && m_oam_first)
	{
		m_oam_first = false;
		if (get)
			m_oam_halt = true;
	}
	enum { HALTED, DMC_GET, OAM_GET, OAM_PUT } act;
	const bool dmc = m_dmc_req, oam = m_oam_active;
	if (!get)
	{
		if (dmc && oam)
			act = (m_dmc_halt && m_oam_halt) ? HALTED : (m_oam_halt ? HALTED : OAM_PUT);   // a DMC put is a repeated read
		else if (dmc)
			act = HALTED;
		else
			act = m_oam_halt ? HALTED : OAM_PUT;
	}
	else
	{
		if (dmc && oam)
			act = (m_dmc_halt && m_oam_halt) ? HALTED : ((!m_oam_halt && m_dmc_halt) ? OAM_GET : DMC_GET);
		else if (dmc)
			act = m_dmc_halt ? HALTED : DMC_GET;
		else
			act = m_oam_halt ? HALTED : OAM_GET;
	}
	if ((act == OAM_PUT) && !m_oam_aligned)
		act = HALTED;   // alignment cycle: the CPU's read is repeated

	address_space &prog = space(AS_PROGRAM);
	const u16 dmc_src = (act == DMC_GET) ? dmc_dma_address() : 0;
	const u16 oam_src = (m_oam_page << 8) | m_oam_index;
	switch (act)
	{
	case DMC_GET: m2_begin(dmc_src, false); break;
	case OAM_GET: m2_begin(oam_src, false); break;
	case OAM_PUT: m2_begin(0x2004, true); break;
	default: m2_begin(adr, false); break;
	}

	if (act == DMC_GET)
	{
		u8 data = prog.read_byte(dmc_src);
		m_bus = data;
		// with the halted CPU's address in $4000-$401F the APU registers answer to the fetch address's low 5 bits too:
		// $xx15 is read (clearing the frame IRQ) without driving the bus, and $xx16/$xx17 drive the controller bits with
		// the sample byte left in the floating ones (AccuracyCoin "DMA Bus Conflicts")
		if ((adr & 0xffe0) == 0x4000)
		{
			const u8 reg = dmc_src & 0x1f;
			if (reg == 0x15)
				(void)prog.read_byte(0x4015);
			else if ((reg == 0x16) || (reg == 0x17))
			{
				// 2A07: a CPU halted on one joypad register does not get the other joypad clocked by a DMA address that
				// matches it in the low 5 bits (NESdev "DMA", Register conflicts). That is the only 2A07 difference
				// documented for the register decode, so a CPU halted anywhere else in $4000-$401F reads the joypads as on
				// the 2A03.
				if (!m_dma_2a07 || ((adr & 0xfffe) != 0x4016) || ((0x4000 | reg) == adr))
					m_bus = data = prog.read_byte(0x4000 | reg);
			}
		}
		m_dmc_req = false;
		m_oam_aligned = false;   // a sprite DMA has to realign after this
		m_dmc_block = 2;
		dmc_dma_done(data);
		m_last_dummy_adr = -1;
	}
	else if (act == OAM_GET)
	{
		// The APU/IO registers are only selected while the 6502's own address bus is in $4000-$401F. A DMA reading
		// $4000-$401F while the halted CPU sits elsewhere sees open bus, with no side effects (AccuracyCoin "APU Register
		// Activation" test 4). While the CPU's address is in that range, the registers answer to the low 5 bits of any
		// address the DMA reads (mirrors every $20 bytes): $xx15 drives the status bits except bit 5 over whatever else is
		// on the bus, and on open bus $xx16/$xx17 show the controllers (tests 6/7). $4020-$7FFF is taken as undecoded.
		const bool apu_active = (adr & 0xffe0) == 0x4000;
		const u8 reg = oam_src & 0x1f;
		if ((oam_src & 0xffe0) == 0x4000)
			m_oam_byte = apu_active ? prog.read_byte(oam_src) : m_bus;
		else if (apu_active && (reg >= 0x15) && (reg <= 0x17))
		{
			const bool decoded = (oam_src < 0x4000) || (oam_src >= 0x8000);
			const u8 other = decoded ? prog.read_byte(oam_src) : m_bus;
			if (reg == 0x15)
				m_oam_byte = u8((prog.read_byte(0x4015) & ~0x20) | (other & 0x20));
			else
			{
				// the controller port is read (and clocked) either way, but a decoded memory drives the bus over it; on the
				// 2A07 a CPU halted on one joypad register does not clock the other one (NESdev "DMA", Register conflicts),
				// the only documented 2A07 difference: halted anywhere else in $4000-$401F the joypads are read as on the 2A03
				const bool clocked = !m_dma_2a07 || ((adr & 0xfffe) != 0x4016) || ((0x4000 | reg) == adr);
				const u8 port = clocked ? prog.read_byte(0x4000 | reg) : m_bus;
				m_oam_byte = decoded ? other : port;
			}
		}
		else
			m_oam_byte = prog.read_byte(oam_src);
		m_bus = m_oam_byte;
		m_oam_aligned = true;
		m_last_dummy_adr = -1;
	}
	else if (act == OAM_PUT)
	{
		prog.write_byte(0x2004, m_oam_byte);
		m_oam_aligned = false;
		if (++m_oam_index == 256)
			m_oam_active = false;
		m_last_dummy_adr = -1;
	}
	else
	{
		// the halted CPU repeats its read. The joypad output enables stay asserted across adjacent reads of one joypad
		// register, so the port sees one clock for them; the console's $4016/$4017 handlers decide that, as the RF
		// Famicom gates the enables and clocks on every read (NESdev "DMA", Register conflicts).
		// The 2A07 does the same: its DMA only ever halts the CPU on an opcode fetch (see m6502make.py), and bus traces of
		// an RP2A07 (NESdev forum "PAL DMA behavior", lidnariq, Apr 2025) show that fetch read again on every halted
		// cycle ("FFBC FFBC FFBC --DMCDMA--"): "all of the behavior during the DMA appears to be the same". The extra
		// register reads of the 2A03 are avoided on the 2A07 because the halt never falls on a data read, not because the
		// halted cycles stop reading; the earlier model here (one read on the halting cycle, none after it, none at all
		// for $2000-$401F) had no hardware source.
		{
			const u8 data = prog.read_byte(adr);
			if (adr != 0x4015)
				m_bus = data;
		}
		m_last_dummy_adr = adr;
	}
	if (get)
	{
		m_dmc_halt = false;
		m_oam_halt = false;
	}
	m2_end();
	cycle_end();
}

void rp2a03_device::oam_dma_w(u8 data)
{
	oam_dma_request(data);
}

void rp2a03_device::apu_dmc_dma(offs_t offset, u16 data)
{
	if (offset == 2)
	{
		// a reload DMA of the later chips that follows a DMC get at once, which m_dmc_block otherwise rules out
		m_dmc_req = true;
		m_dmc_halt = true;
	}
	else
		dmc_request(offset == 1);
}

void rp2a03_device::apu_cycle_end(bool get)
{
	m_apu->dmc_cycle_end(get);
}

bool rp2a03_device::dmc_dma_enabled()
{
	return m_apu->dmc_dma_allowed();
}

void rp2a03_device::dmc_implicit_clear()
{
	m_apu->dmc_implicit_clear();
}

u16 rp2a03_device::dmc_dma_address()
{
	return m_apu->dmc_address();
}

void rp2a03_device::dmc_dma_done(u8 data)
{
	m_apu->dmc_dma_done(data);
}

uint8_t rp2a03_device::apu_read_mem(offs_t offset)
{
	m2_begin(offset, false);   // a read made outside a CPU or DMA cycle is a pulse of its own; inside one it nests
	const uint8_t data = m_mintf->m_program.read_interruptible(offset);
	m2_end();
	return data;
}

void rp2a03_device::device_add_mconfig(machine_config &config)
{
	APU_2A03(config, m_apu, DERIVED_CLOCK(1,1));
	m_apu->irq().set(FUNC(rp2a03_device::apu_irq));
	m_apu->mem_read().set(FUNC(rp2a03_device::apu_read_mem));
	m_apu->dmc_dma().set(FUNC(rp2a03_device::apu_dmc_dma));
	m_apu->add_route(ALL_OUTPUTS, *this, 1.0, 0);
}

void rp2a03g_device::device_add_mconfig(machine_config &config)
{
	NES_APU(config, m_apu, DERIVED_CLOCK(1,1));
	m_apu->irq().set(FUNC(rp2a03g_device::apu_irq));
	m_apu->mem_read().set(FUNC(rp2a03g_device::apu_read_mem));
	m_apu->dmc_dma().set(FUNC(rp2a03g_device::apu_dmc_dma));
	m_apu->add_route(ALL_OUTPUTS, *this, 1.0, 0);
}


#include "cpu/m6502/rp2a03.hxx"
