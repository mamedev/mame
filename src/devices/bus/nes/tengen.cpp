// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Brian H. Gilbert
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Tengen PCBs

 iNES mappers: 64, 158


 Here we emulate the following PCBs

 * Tengen 800032 [mapper 64]
 * Tengen 800037 [mapper 158]

 Note, Tetris' Tengen 800008 [mapper 148] is implemented in sachen.cpp.

 ***********************************************************************************************************/


#include "emu.h"
#include "tengen.h"

#include "video/ppu2c0x.h"      // this has to be included so that IRQ functions can access ppu2c0x_device::BOTTOM_VISIBLE_SCANLINE

#include <algorithm>


#ifdef NES_PCB_DEBUG
#define VERBOSE (LOG_GENERAL)
#else
#define VERBOSE (0)
#endif
#include "logmacro.h"


 //-------------------------------------------------
 //  constructor
 //-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_TENGEN_800032, nes_tengen032_device, "nes_tengen032", "NES Cart Tengen 800032 PCB")
DEFINE_DEVICE_TYPE(NES_TENGEN_800037, nes_tengen037_device, "nes_tengen037", "NES Cart Tengen 800037 PCB")


nes_tengen032_device::nes_tengen032_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, u32 clock)
	: nes_nrom_device(mconfig, type, tag, owner, clock)
	, m_ctrl(0), m_mirror(0), m_prg_mask(0x1f), m_chr_mask(0xff), m_a12_level(0)
	, m_irq_latch(0), m_irq_count(0), m_irq_enable(false), m_irq_cpu_mode(false), m_irq_prescaler(0)
	, m_irq_reload(false), m_irq_swallow(false), m_irq_assert_next(false), m_irq_assert_rise(false)
	, m_m2_fall(attotime::never), m_m2_period(attotime::zero)
	, m_a12_rise(attotime::zero), m_a12_high_fall(attotime::zero)
	, m_a12_pending(false), m_a12_pending_fall(attotime::zero), m_a12_pending_lows(0)
	, m_c001_open(false), m_c001_time(attotime::zero), m_c001_level(0)
{
	std::fill(std::begin(m_reg), std::end(m_reg), 0);
}

nes_tengen032_device::nes_tengen032_device(const machine_config& mconfig, const char* tag, device_t* owner, u32 clock)
	: nes_tengen032_device(mconfig, NES_TENGEN_800032, tag, owner, clock)
{
}

nes_tengen037_device::nes_tengen037_device(const machine_config& mconfig, const char* tag, device_t* owner, u32 clock)
	: nes_tengen032_device(mconfig, NES_TENGEN_800037, tag, owner, clock)
{
	// the pin that drives CHR A17 on 800032 drives CIRAM A10 here, so the CHR ROM sees bank bits 0-6 only;
	// the board takes at most 128K of PRG ROM (Kevin Horton), so PRG A17 is not wired and the ROM sees bits 0-3
	m_prg_mask = 0x0f;
	m_chr_mask = 0x7f;
}


/*-------------------------------------------------

 Tengen 800032 Board (RAMBO-1 ASIC)

 Games: Klax, Skull & Crossbones, Shinobi, Rolling Thunder, Road Runner,
 Hard Drivin' (prototype), Xybots (prototype), Police Academy (prototype)

 iNES: mapper 64

 Pins (NESdev pinout, Kevin Horton): no console reset input, so a reset keeps every register; PRG A13-A17 and
 CHR A10-A17 are the bank outputs; registers decode with mask $E001.

 IRQ counter (Kevin Horton's chip document, the register text of the NESdev page, and Hard Drivin', whose handler
 at $FDB4 writes $C001 before $C000 = 0 and needs the IRQ on the next line):
  - $C001 clears the counter and makes the next clock load the latch; if the counter is then 0 the IRQ fires.
  - Other clocks: a counter of 0 is reloaded without an IRQ, anything else is decremented and fires on reaching 0.
	So latch N gives an IRQ every N+1 clocks, and latch 0 none until $C001 is written again (Kevin Horton).
  - If A12 had been low for 16 M2 falls when $C001 was written, the first clock after it does nothing, which is
	Kevin Horton's N+2 after $C001 (puNES makes the same A12 condition). Hard Drivin' writes $C001 a few cycles
	after the sprite fetches' A12 pulses, so its next line is clocked.
  - Scanline clock: M2 falls sample A12; the first high sample after 16 low ones clocks the counter (NESdev).
  - Cycle clock: every 4th M2 fall; $C001 resets the prescaler (NESdev).
  - A fired IRQ goes out about 20 ns after the next M2 fall (James's measurements, both modes), after the CPU's
	IRQ sample of that cycle, so the CPU first sees it in the following cycle. $E000 disables and acknowledges,
	$E001 enables without acknowledging (NESdev).

 NOT DOCUMENTED (the choice made here):
  - Power-on values of all registers: 0.
  - What $E000 does to an IRQ waiting for its M2 fall: it is still asserted.
  - In cycle mode, when $C001 is written with A12 low, the start of that low period is only known at the next A12
	rise; if the reload clock comes first, the low period is taken to start at the last known high sample.

 In MAME: Supported.

 Tested: Hard Drivin' (USA) (Proto), Klax (USA), Skull & Crossbones (USA), Shinobi (USA) and Rolling Thunder (USA)
 play. Road Runner (USA), Xybots (USA) (Proto) and Police Academy (USA) (Proto) were not rerun after the IRQ output
 change.

 -------------------------------------------------*/

void nes_tengen032_device::device_start()
{
	common_start();
	m_ppu_dev = machine().root_device().subdevice<ppu2c0x_device>("ppu");

	save_item(NAME(m_ctrl));
	save_item(NAME(m_reg));
	save_item(NAME(m_mirror));
	save_item(NAME(m_irq_latch));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_cpu_mode));
	save_item(NAME(m_irq_prescaler));
	save_item(NAME(m_irq_reload));
	save_item(NAME(m_irq_swallow));
	save_item(NAME(m_irq_assert_next));
	save_item(NAME(m_irq_assert_rise));
	save_item(NAME(m_m2_fall));
	save_item(NAME(m_m2_period));
	save_item(NAME(m_a12_rise));
	save_item(NAME(m_a12_high_fall));
	save_item(NAME(m_a12_pending));
	save_item(NAME(m_a12_pending_fall));
	save_item(NAME(m_a12_pending_lows));
	save_item(NAME(m_a12_level));
	save_item(NAME(m_c001_open));
	save_item(NAME(m_c001_time));
	save_item(NAME(m_c001_level));
}

void nes_tengen032_device::pcb_reset()
{
	// no reset pin: the registers keep their values, the outputs are put back from them
	update_prg();
	update_chr();
	update_mirroring();
}


/*-------------------------------------------------
 banking
 -------------------------------------------------*/

void nes_tengen032_device::update_prg()
{
	// $E000-$FFFF has every wired PRG address line high
	const bool swap = BIT(m_ctrl, 6);
	prg8_89((swap ? m_reg[15] : m_reg[6]) & m_prg_mask);
	prg8_ab(m_reg[7] & m_prg_mask);
	prg8_cd((swap ? m_reg[6] : m_reg[15]) & m_prg_mask);
	prg8_ef(m_prg_mask);
}

void nes_tengen032_device::update_chr()
{
	const int inv = BIT(m_ctrl, 7) ? 4 : 0;
	const bool k = BIT(m_ctrl, 5);

	chr1_x(0 ^ inv, (k ? m_reg[0] : (m_reg[0] & 0xfe)) & m_chr_mask, m_chr_source);
	chr1_x(1 ^ inv, (k ? m_reg[8] : (m_reg[0] | 0x01)) & m_chr_mask, m_chr_source);
	chr1_x(2 ^ inv, (k ? m_reg[1] : (m_reg[1] & 0xfe)) & m_chr_mask, m_chr_source);
	chr1_x(3 ^ inv, (k ? m_reg[9] : (m_reg[1] | 0x01)) & m_chr_mask, m_chr_source);
	chr1_x(4 ^ inv, m_reg[2] & m_chr_mask, m_chr_source);
	chr1_x(5 ^ inv, m_reg[3] & m_chr_mask, m_chr_source);
	chr1_x(6 ^ inv, m_reg[4] & m_chr_mask, m_chr_source);
	chr1_x(7 ^ inv, m_reg[5] & m_chr_mask, m_chr_source);
}

void nes_tengen032_device::update_mirroring()
{
	set_nt_mirroring(BIT(m_mirror, 0) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
}


/*-------------------------------------------------
 M2 falling edge grid
 -------------------------------------------------*/

namespace {

	s64 floor_div(s64 a, s64 b)
	{
		s64 q = a / b;
		if ((a % b) && ((a < 0) != (b < 0)))
			q--;
		return q;
	}

	s64 ceil_div(s64 a, s64 b)
	{
		return -floor_div(-a, b);
	}

} // anonymous namespace

// attoseconds from the latest M2 fall, limited to one second either way
s64 nes_tengen032_device::rel_as(const attotime& t) const
{
	const s64 far = ATTOSECONDS_PER_SECOND;
	if (t >= m_m2_fall)
	{
		const attotime d = t - m_m2_fall;
		return d.seconds() ? far : d.attoseconds();
	}
	const attotime d = m_m2_fall - t;
	return d.seconds() ? -far : -d.attoseconds();
}

attotime nes_tengen032_device::fall_time(s64 k) const
{
	return (k >= 0) ? (m_m2_fall + m_m2_period * u32(k)) : (m_m2_fall - m_m2_period * u32(-k));
}

// number of M2 falls f with after < f <= upto (a fall at the instant A12 rises still sees it low)
s64 nes_tengen032_device::falls_in(const attotime& after, const attotime& upto) const
{
	if (m_m2_period.is_zero() || m_m2_fall.is_never())
		return 0;
	const s64 p = m_m2_period.as_attoseconds();
	const s64 lo = floor_div(rel_as(after), p) + 1;
	const s64 hi = floor_div(rel_as(upto), p);
	return (hi >= lo) ? (hi - lo + 1) : 0;
}


/*-------------------------------------------------
 IRQ
 -------------------------------------------------*/

void nes_tengen032_device::irq_clock()
{
	if (m_irq_reload)
	{
		if (m_c001_open)
		{
			// the clock came before the A12 rise that tells the state at the write: use A12 as of the write
			m_irq_swallow = !m_c001_level && (falls_in(m_a12_high_fall, m_c001_time) >= 16);
			m_c001_open = false;
		}
		if (m_irq_swallow)
		{
			m_irq_swallow = false;
			return;
		}
		m_irq_reload = false;
		m_irq_count = m_irq_latch;
		if (!m_irq_count && m_irq_enable)
			m_irq_assert_next = true;
	}
	else if (!m_irq_count)
		m_irq_count = m_irq_latch;
	else if (!--m_irq_count && m_irq_enable)
		m_irq_assert_next = true;
}

// an M2 fall that sees A12 high: clocks the counter if 16 low samples came before it
void nes_tengen032_device::a12_sample_high(const attotime& fall, u8 low_falls)
{
	m_a12_high_fall = fall;

	if (m_irq_cpu_mode || (low_falls < 16))
		return;

	irq_clock();

	// reported late: the fall after this one may already have passed
	if (m_irq_assert_next && ((fall + m_m2_period) <= (m_m2_fall + m_m2_period / 2)))
	{
		m_irq_assert_next = false;
		m_irq_assert_rise = true;
	}
}

bool nes_tengen032_device::a12_rise_accepted(const attotime& low_since, const attotime& when)
{
	if (m_m2_fall.is_never() || m_m2_period.is_zero())
	{
		m_a12_rise = when;
		return false;
	}

	const s64 p = m_m2_period.as_attoseconds();

	// the high period that ended at low_since: its last M2 fall saw A12 high (a fall at the instant A12 falls sees it low)
	if (low_since > m_a12_rise)
	{
		const s64 lo = floor_div(rel_as(m_a12_rise), p) + 1;
		const s64 hi = ceil_div(rel_as(low_since), p) - 1;
		if (hi >= lo)
		{
			const attotime f = fall_time(hi);
			if (f > m_a12_high_fall)
				m_a12_high_fall = f;
		}
	}

	// a rise still waiting for its first M2 fall: A12 fell again before that fall, so it sampled low
	if (m_a12_pending && (m_a12_pending_fall >= low_since))
		m_a12_pending = false;

	// the first rise after a $C001 write tells A12's state at the write: low since low_since, or high before it
	if (m_c001_open && (when > m_c001_time))
	{
		m_irq_swallow = (m_c001_time >= low_since) && (falls_in(m_a12_high_fall, m_c001_time) >= 16);
		m_c001_open = false;
	}

	const u8 lows = u8(std::min<s64>(falls_in(m_a12_high_fall, when), 16));
	const attotime first = fall_time(floor_div(rel_as(when), p) + 1);
	m_a12_rise = when;

	if (first > (m_m2_fall + m_m2_period / 2))
	{
		m_a12_pending = true;
		m_a12_pending_fall = first;
		m_a12_pending_lows = lows;
	}
	else
		a12_sample_high(first, lows);

	return false;
}

void nes_tengen032_device::m2_w(int state, offs_t address, bool write)
{
	if (state)
	{
		// /IRQ went low just after the last M2 fall (James: about 20 ns after it), too late for the CPU's sample in that
		// cycle: it is seen from this cycle on
		if (m_irq_assert_rise)
		{
			m_irq_assert_rise = false;
			set_irq_line(ASSERT_LINE);
		}
		return;
	}

	const attotime now = machine().time();
	if (!m_m2_fall.is_never() && (now > m_m2_fall))
		m_m2_period = now - m_m2_fall;
	m_m2_fall = now;

	// a clock at the previous fall fired: /IRQ goes low just after this one
	if (m_irq_assert_next)
	{
		m_irq_assert_next = false;
		m_irq_assert_rise = true;
	}

	if (m_a12_pending && (m_a12_pending_fall <= (now + m_m2_period / 2)))
	{
		m_a12_pending = false;
		a12_sample_high(now, m_a12_pending_lows);
	}

	if (m_irq_cpu_mode)
	{
		m_irq_prescaler = u8((m_irq_prescaler + 1) & 3);
		if (!m_irq_prescaler)
			irq_clock();
	}
}


/*-------------------------------------------------
 registers
 -------------------------------------------------*/

void nes_tengen032_device::write_h(offs_t offset, u8 data)
{
	// bring the PPU up to the write, so that fetches already made use the old banks and every A12 edge before the
	// write has been seen
	if (m_ppu_dev)
		m_ppu_dev->catch_up();

	switch (offset & 0x6001)
	{
	case 0x0000:
		m_ctrl = data;
		update_prg();
		update_chr();
		break;

	case 0x0001:
	{
		const u8 index = m_ctrl & 0x0f;
		switch (index)
		{
		case 0: case 1: case 2: case 3: case 4: case 5: case 8: case 9:
			m_reg[index] = data;
			update_chr();
			break;
		case 6: case 7: case 15:
			m_reg[index] = data;
			update_prg();
			break;
		default:    // RA-RE: no function
			break;
		}
		break;
	}

	case 0x2000:
		m_mirror = data & 0x01;
		update_mirroring();
		break;

	case 0x2001:    // no known function
		break;

	case 0x4000:
		m_irq_latch = data;
		break;

	case 0x4001:
		m_irq_count = 0;
		m_irq_reload = true;
		m_irq_swallow = false;
		m_irq_cpu_mode = BIT(data, 0);
		m_irq_prescaler = 0;
		m_c001_open = true;
		m_c001_time = machine().time();
		m_c001_level = m_a12_level;
		break;

	case 0x6000:
		m_irq_enable = false;
		set_irq_line(CLEAR_LINE);
		break;

	case 0x6001:
		m_irq_enable = true;
		break;
	}
}


/*-------------------------------------------------

 Tengen 800037 Board

 Games: Alien Syndrome

 iNES: mapper 158

 The RAMBO-1 above with one pin rewired (NESdev "INES Mapper 158" and the variant pinout): pin 22, CHR A17 on
 800032, drives CIRAM A10, and pin 29, CIRAM A10 on 800032, is not connected. So the nametable page is bit 7 of the
 CHR bank the RAMBO-1 selects for the nametable address, the $A000 mirroring bit has no effect, and the CHR ROM
 sees bank bits 0-6 only. The board takes at most 128K of PRG ROM (Kevin Horton's 800037 page), so PRG A17 is
 not wired either and the PRG ROM sees bank bits 0-3. This is how TLSROM (mapper 118) differs from MMC3.

 CIRAM A10 is bit 7 of the CHR bank the RAMBO-1 selects for PPU A10-A12 of the access, the same 1 KiB slot
 decode as for $0000-$1FFF. Rendering fetches nametables at $2000-$2FFF (PPU A12 low), so the slots are those of
 $0000-$0FFF: R0, R0, R1, R1 (K = 0), R0, R8, R1, R9 (K = 1), or R2-R5 when the CHR A12 inversion is set. A PPUDATA
 access to $3000-$3EFF, and the nametable read under a PPUDATA palette read at $3F00-$3FFF, have PPU A12 high and
 take the slots of $1000-$1FFF.

 In MAME: Supported.

 Tested: Alien Syndrome (USA) (Unl) plays Round 1; a test ROM checks the nametable pages for K clear and set, the
 CHR A12 inversion, $3000-$3EFF and the palette underlay (13 of 13).

 -------------------------------------------------*/

 // bit 7 of the bank in 1 KiB slot 0-7 of the PPU pattern space, after the CHR A12 inversion
int nes_tengen037_device::ciram_a10(int slot) const
{
	static constexpr u8 bank_2k[8] = { 0, 0, 1, 1, 2, 3, 4, 5 };
	static constexpr u8 bank_1k[8] = { 0, 8, 1, 9, 2, 3, 4, 5 };

	slot ^= BIT(m_ctrl, 7) ? 4 : 0;
	return BIT(m_reg[(BIT(m_ctrl, 5) ? bank_1k : bank_2k)[slot]], 7);
}

// offset is PPU address - $2000. $3000-$3EFF has PPU A12 in the offset; the PPU passes the nametable read under a
// palette read at $3F00-$3FFF as $2F00-$2FFF, while the bus (whose A12 level the PPU reports before the access) holds $3Fxx
int nes_tengen037_device::nt_page(offs_t offset) const
{
	const int a12 = (BIT(offset, 12) || m_a12_level) ? 4 : 0;
	return ciram_a10(a12 | BIT(offset, 10, 2));
}

void nes_tengen037_device::update_chr()
{
	nes_tengen032_device::update_chr();

	// the page pointers, for the PPU A12 low case; nt_r and nt_w below decide every access
	for (int i = 0; i < 4; i++)
		set_nt_page(i, CIRAM, ciram_a10(i), 1);
}

u8 nes_tengen037_device::nt_r(offs_t offset)
{
	return get_ciram_base()[(nt_page(offset) << 10) | (offset & 0x3ff)];
}

void nes_tengen037_device::nt_w(offs_t offset, u8 data)
{
	get_ciram_base()[(nt_page(offset) << 10) | (offset & 0x3ff)] = data;
}

void nes_tengen037_device::write_h(offs_t offset, u8 data)
{
	LOG("tengen037 write_h, offset: %04x, data: %02x\n", offset, data);

	// $A000 still latches in the RAMBO-1, but its output pin is not connected
	nes_tengen032_device::write_h(offset, data);
}
