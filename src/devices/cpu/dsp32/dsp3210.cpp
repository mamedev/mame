// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    dsp3210.cpp
    AT&T DSP3210
    Emulation by R. Belmont, with inspiration from Aaron Giles' DSP32C and
    pappadf's dsp3210-sdk.

    Behavior the AT&T manual doesn't define was established from the Quadra
    840AV's DSP code.  Failing that, we followed the dsp3210-sdk reference
    emulator from the dsp3210-sdk.

    Model in brief:
      * flat 4 CKI per instruction, no bus arbitration or wait states;
      * pc/npc delayed branches: the word at pc is fetched before the
        previous instruction's side effects (irsh under the old map);
      * one 64 KB on-chip window (boot ROM, MMIO, RAM1, RAM0);
      * big-endian bus only (pcw[8] = 0 is logged and ignored);
      * exact 40-bit integer accumulators;
      * DA memory writes land four instructions later (deferred ring).

    TODOs: SIO and DMAC if a machine where they're used turns up

***************************************************************************/

#include "emu.h"
#include "dsp3210.h"

#include "dsp3210dau.hxx"
#include "dsp3210dis.h"

#include "multibyte.h"

#include <algorithm>
#include <iterator>

#define LOG_RESERVED    (1U << 1)

#define VERBOSE (0)
#include "logmacro.h"


namespace {

//**************************************************************************
//  CONSTANTS
//**************************************************************************

// 5-bit register codes with a special meaning
constexpr int RC_R0     = 0;
constexpr int RC_PC     = 15;
constexpr int RC_MINUS  = 22;       // -n pseudo-operand
constexpr int RC_PLUS   = 23;       // +n pseudo-operand
constexpr int RC_R20    = 24;
constexpr int RC_SP     = 25;       // r21
constexpr int RC_EVTP   = 26;       // r22
constexpr int RC_PCSH   = 30;

// registers that accept a write (r0, code 16, +-n and the reserved codes discard)
constexpr uint32_t WRITEABLE_REGS = 0x473efffe;

// CA ALU functions (F field, Table 10-2)
enum : int
{
	F_ADD = 0, F_SHL, F_RSUB, F_CRADD, F_SUB, F_RES5, F_ANDC, F_CMP,
	F_XOR, F_ROR, F_OR, F_ROL, F_SHR, F_ASR, F_AND, F_BTST
};

// DAU flags in m_dauflags, in ps[7:4] order
constexpr uint8_t DAU_N = 0x01;
constexpr uint8_t DAU_Z = 0x02;
constexpr uint8_t DAU_U = 0x04;
constexpr uint8_t DAU_V = 0x08;

// on-chip window layout (offsets within the 64 KB window): boot ROM
// 0x0000-0x03ff, MMIO 0x0400-0x07ff, RAM1 0xe000, RAM0 0xf000
constexpr offs_t ONCHIP_ROM_END = 0x0400;

// MMIO words with live state behind them; sub-word registers sit in the
// low lanes of their word (tcon is byte 0x0413, bioc 0x041b, bio 0x041e)
constexpr offs_t MMIO_TCON  = 0x0410;
constexpr offs_t MMIO_TIMER = 0x0414;
constexpr offs_t MMIO_BIOC  = 0x0418;
constexpr offs_t MMIO_BIO   = 0x041c;

// I/O register codes (formats 7b/7d)
constexpr int IOR_PS   = 0;
constexpr int IOR_EMR  = 8;
constexpr int IOR_SPC  = 10;
constexpr int IOR_PCW  = 12;
constexpr int IOR_DAUC = 14;
constexpr int IOR_CTR  = 15;

// The window is one big-endian byte array.
constexpr bool mmio_live(offs_t word)
{
	return (word >= MMIO_TCON) && (word <= MMIO_BIO);
}

// Get the byte lanes an access of `width` bytes at `offset` covers, as a mask
// over the big-endian word (word and long accesses arrive aligned)
constexpr uint32_t lane_mask(offs_t offset, int width)
{
	const uint32_t bits = (width == 4) ? 0xffffffff : (width == 2) ? 0xffff : 0xff;
	return bits << (8 * (4 - (offset & 3) - width));
}

// bit-reverse the low `nbits` bits (the carry-reverse add)
constexpr uint32_t bitrev(uint32_t x, int nbits)
{
	uint32_t r = 0;
	for (int i = 0; i < nbits; i++)
	{
		r |= BIT(x, i) << (nbits - 1 - i);
	}
	return r;
}

} // anonymous namespace

DEFINE_DEVICE_TYPE(DSP3210, dsp3210_device, "dsp3210", "AT&T DSP3210")

ROM_START(dsp3210)
	ROM_REGION(0x400, "bootrom", ROMREGION_ERASE00)
	ROM_LOAD("dsp3210_boot.bin", 0x000, 0x400, NO_DUMP)
ROM_END

dsp3210_device::dsp3210_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cpu_device(mconfig, DSP3210, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_BIG, 32, 32)
	, m_onchip_config("onchip", ENDIANNESS_BIG, 8, 16, 0, address_map_constructor(FUNC(dsp3210_device::onchip_map), this))
	, m_bootrom(*this, "bootrom")
	, m_onchip(*this, "onchip")
	, m_bio_out_cb(*this)
	, m_bio_dir_cb(*this)
	, m_bio_in_cb(*this, 0)
	, m_iack_cb(*this)
	, m_straps(0)
	, m_r{ }
	, m_pc(0)
	, m_npc(0)
	, m_ppc(0)
	, m_prefetch(0)
	, m_prefetch_valid(false)
	, m_nzcflags(0)
	, m_vflags(0)
	, m_acc{ }
	, m_dauflags(0)
	, m_apipe_m{ }
	, m_apipe_e{ }
	, m_fpipe{ }
	, m_slot(0)
	, m_acc_lane{ }
	, m_emr(0)
	, m_pcw(0)
	, m_dauc(0)
	, m_ctr(0)
	, m_ps_pins(3)
	, m_onchip_base(0x50030000)
	, m_timer(0xffffffff)
	, m_timer_reload(0xffffffff)
	, m_tcon(0)
	, m_timer_loaded(false)
	, m_timer_out(false)
	, m_bio(0)
	, m_bioc(0)
	, m_pending(0)
	, m_level(0)
	, m_error_nonmaskable(false)
	, m_waiting(false)
	, m_asleep_slots(0)
	, m_parked(false)
	, m_park_cycles(0)
	, m_wake_timer(nullptr)
	, m_halted(false)
	, m_int_defer(false)
	, m_irsh(0x80000000)
	, m_irsh_addr(0)
	, m_irsh_pending(false)
	, m_servicing(0)
	, m_berr(false)
	, m_fetch_berr(false)
	, m_iack{ }
	, m_sh_nzcflags(0)
	, m_sh_vflags(0)
	, m_sh_dauflags(0)
	, m_sh_dauc(0)
	, m_sh_ctr(0)
	, m_sh_acc{ }
	, m_sh_apipe_m{ }
	, m_sh_apipe_e{ }
	, m_sh_fpipe{ }
	, m_sh_slot(0)
	, m_sh_do_active(false)
	, m_sh_do_lock(false)
	, m_sh_do_start(0)
	, m_sh_do_end(0)
	, m_sh_do_count(0)
	, m_do_active(false)
	, m_do_lock(false)
	, m_do_start(0)
	, m_do_end(0)
	, m_do_count(0)
	, m_mbuf{ }
	, m_icount(0)
	, m_ps_temp(0)
{
}

const tiny_rom_entry *dsp3210_device::device_rom_region() const
{
	return ROM_NAME(dsp3210);
}

void dsp3210_device::onchip_map(address_map &map)
{
	// the live MMIO words over the RAM: a later entry takes precedence
	map(0x0000, 0xffff).ram().share("onchip");
	map(MMIO_TCON, MMIO_BIO + 3).rw(FUNC(dsp3210_device::onchip_debug_r), FUNC(dsp3210_device::onchip_debug_w));
}

void dsp3210_device::device_start()
{
	space(AS_PROGRAM).cache(m_cache);
	space(AS_PROGRAM).specific(m_program);

	// the on-chip window: the boot ROM at 0, everything else zero
	std::copy_n(m_bootrom.target(), std::min<size_t>(m_bootrom.length(), ONCHIP_ROM_END), m_onchip.target());
	m_wake_timer = timer_alloc(FUNC(dsp3210_device::wake_tick), this);

	// debugger state
	state_add(STATE_GENPC,     "GENPC",     m_pc).callimport().noshow();
	state_add(STATE_GENPCBASE, "CURPC",     m_ppc).noshow();
	state_add(STATE_GENFLAGS,  "GENFLAGS",  m_ps_temp).callimport().callexport().formatstr("%8s").noshow();
	state_add(DSP3210_PC,      "PC",        m_pc).callimport();
	state_add(DSP3210_R0,      "R0",        m_r[RC_R0]).readonly();
	for (int i = 1; i <= 14; i++)
	{
		state_add(DSP3210_R0 + i, string_format("R%d", i).c_str(), m_r[i]);
	}
	for (int i = 15; i <= 19; i++)
	{
		state_add(DSP3210_R0 + i, string_format("R%d", i).c_str(), m_r[i + 2]);
	}
	state_add(DSP3210_R20,     "R20",       m_r[RC_R20]);
	state_add(DSP3210_R21,     "R21",       m_r[RC_SP]);
	state_add(DSP3210_R22,     "R22",       m_r[RC_EVTP]);
	state_add(DSP3210_PCSH,    "PCSH",      m_r[RC_PCSH]);
	state_add(DSP3210_EVTP,    "EVTP",      m_r[RC_EVTP]).noshow();
	for (int i = 0; i < 4; i++)
	{
		state_add(DSP3210_A0 + i,  string_format("A%d", i).c_str(),  m_acc[i].m).formatstr("%12s");
		state_add(DSP3210_A0M + i, string_format("A%dM", i).c_str(), m_acc_lane[i]).callexport();
		state_add(DSP3210_A0E + i, string_format("A%dE", i).c_str(), m_acc[i].e).mask(0xff);
	}
	state_add(DSP3210_PS,      "PS",        m_ps_temp).callimport().callexport().mask(0x3fff);
	state_add(DSP3210_EMR,     "EMR",       m_emr);
	state_add(DSP3210_PCW,     "PCW",       m_pcw).callimport();
	state_add(DSP3210_DAUC,    "DAUC",      m_dauc);
	state_add(DSP3210_CTR,     "CTR",       m_ctr).mask(0x3f);
	state_add(DSP3210_TIMER,   "TIMER",     m_timer);
	state_add(DSP3210_TCON,    "TCON",      m_tcon);
	state_add(DSP3210_BIO,     "BIO",       m_bio);
	state_add(DSP3210_BIOC,    "BIOC",      m_bioc);
	state_add(DSP3210_HALT,    "HALT",      m_halted);
	state_add(DSP3210_WAIT,    "WAIT",      m_waiting);

	set_icountptr(m_icount);

	// save state (the on-chip window is a memory share, saved with the space)
	save_item(NAME(m_r));
	save_item(NAME(m_pc));
	save_item(NAME(m_npc));
	save_item(NAME(m_ppc));
	save_item(NAME(m_prefetch));
	save_item(NAME(m_prefetch_valid));
	save_item(NAME(m_nzcflags));
	save_item(NAME(m_vflags));
	save_item(STRUCT_MEMBER(m_acc, m));
	save_item(STRUCT_MEMBER(m_acc, e));
	save_item(NAME(m_dauflags));
	save_item(NAME(m_apipe_m));
	save_item(NAME(m_apipe_e));
	save_item(NAME(m_fpipe));
	save_item(NAME(m_slot));
	save_item(NAME(m_emr));
	save_item(NAME(m_pcw));
	save_item(NAME(m_dauc));
	save_item(NAME(m_ctr));
	save_item(NAME(m_ps_pins));
	save_item(NAME(m_onchip_base));
	save_item(NAME(m_timer));
	save_item(NAME(m_timer_reload));
	save_item(NAME(m_tcon));
	save_item(NAME(m_timer_loaded));
	save_item(NAME(m_timer_out));
	save_item(NAME(m_bio));
	save_item(NAME(m_bioc));
	save_item(NAME(m_pending));
	save_item(NAME(m_level));
	save_item(NAME(m_error_nonmaskable));
	save_item(NAME(m_waiting));
	save_item(NAME(m_asleep_slots));
	save_item(NAME(m_parked));
	save_item(NAME(m_park_cycles));
	save_item(NAME(m_halted));
	save_item(NAME(m_int_defer));
	save_item(NAME(m_irsh));
	save_item(NAME(m_irsh_addr));
	save_item(NAME(m_irsh_pending));
	save_item(NAME(m_servicing));
	save_item(NAME(m_berr));
	save_item(NAME(m_fetch_berr));
	save_item(NAME(m_iack));
	save_item(NAME(m_sh_nzcflags));
	save_item(NAME(m_sh_vflags));
	save_item(NAME(m_sh_dauflags));
	save_item(NAME(m_sh_dauc));
	save_item(NAME(m_sh_ctr));
	save_item(STRUCT_MEMBER(m_sh_acc, m));
	save_item(STRUCT_MEMBER(m_sh_acc, e));
	save_item(NAME(m_sh_apipe_m));
	save_item(NAME(m_sh_apipe_e));
	save_item(NAME(m_sh_fpipe));
	save_item(NAME(m_sh_slot));
	save_item(NAME(m_sh_do_active));
	save_item(NAME(m_sh_do_lock));
	save_item(NAME(m_sh_do_start));
	save_item(NAME(m_sh_do_end));
	save_item(NAME(m_sh_do_count));
	save_item(NAME(m_do_active));
	save_item(NAME(m_do_lock));
	save_item(NAME(m_do_start));
	save_item(NAME(m_do_end));
	save_item(NAME(m_do_count));
	save_item(STRUCT_MEMBER(m_mbuf, addr));
	save_item(STRUCT_MEMBER(m_mbuf, data));
	save_item(STRUCT_MEMBER(m_mbuf, width));
}

void dsp3210_device::device_reset()
{
	// execution restarts at 0: the boot ROM in computer mode, external
	// memory in processor mode.
	m_pc = 0;
	m_npc = 4;
	m_prefetch_valid = false;

	m_r[RC_EVTP] = 0;
	m_nzcflags = 1;
	m_vflags = 0;

	m_emr = 0;
	m_pcw = 0x038f | (m_straps << 10);      // pcw[13:10] latch BIO[4:7]
	update_onchip_base();
	m_dauc = 0;

	m_tcon = 0;
	m_timer_loaded = false;

	// all BIO pins become inputs and the output register clears
	if (m_bio)
	{
		m_bio = 0;
		m_bio_out_cb(m_bio);
	}
	if (m_bioc)
	{
		m_bioc = 0;
		m_bio_dir_cb(m_bioc);
	}

	m_pending = 0;
	m_level = 0;
	m_error_nonmaskable = false;
	m_waiting = false;
	m_asleep_slots = 0;
	m_halted = false;

	// a reset wakes a parked sleep (the generic reset line only lifts its own suspension)
	m_parked = false;
	m_wake_timer->adjust(attotime::never);

	if (suspended(SUSPEND_REASON_TRIGGER))
	{
		resume(SUSPEND_REASON_TRIGGER);
	}

	m_int_defer = false;
	m_irsh = 0x80000000;                    // nop
	m_irsh_addr = 0;
	m_irsh_pending = false;
	m_servicing = 0;
	m_berr = false;
	m_fetch_berr = false;
	m_do_active = false;
	m_do_lock = false;

	// the DAU pipeline drains: nothing in flight and clear flags
	m_dauflags = 0;
	m_apipe_m = { };
	m_apipe_e = { };
	std::fill(std::begin(m_fpipe), std::end(m_fpipe), 0);
	std::fill(std::begin(m_mbuf), std::end(m_mbuf), mbuf_entry{ 0, 0, 0 });

	update_iack();
}


device_memory_interface::space_config_vector dsp3210_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA, &m_onchip_config)
	};
}


bool dsp3210_device::memory_translate(int spacenum, int intention, offs_t &address, address_space *&target_space)
{
	target_space = &space(spacenum);
	if ((spacenum == AS_PROGRAM) && onchip(address))
	{
		target_space = &space(AS_DATA);
		address &= 0xffff;
	}
	return true;
}


void dsp3210_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			break;

		case STATE_GENPC:
		case DSP3210_PC:
			// a PC written from outside restarts the pipeline
			m_npc = m_pc + 4;
			m_prefetch_valid = false;
			m_irsh_pending = false;
			m_do_active = false;
			m_waiting = false;
			m_halted = false;
			break;

		case DSP3210_PS:
			// only nzvc are writable
			m_nzcflags = (uint64_t(BIT(m_ps_temp, 3)) << 32) | (BIT(m_ps_temp, 0) ? 0x80000000 : (BIT(m_ps_temp, 1) ? 0 : 1));
			m_vflags = BIT(m_ps_temp, 2) ? 0x80000000 : 0;
			break;

		case DSP3210_PCW:
			update_onchip_base();
			break;

		default:
			fatalerror("dsp3210_device::state_import called for unexpected value\n");
	}
}

void dsp3210_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
		case DSP3210_PS:
			m_ps_temp = ps_r();
			break;

		case DSP3210_A0M:
		case DSP3210_A1M:
		case DSP3210_A2M:
		case DSP3210_A3M:
		{
			const int i = entry.index() - DSP3210_A0M;
			m_acc_lane[i] = dsp3210dau::lane(m_acc[i]);
			break;
		}

		default:
			fatalerror("dsp3210_device::state_export called for unexpected value\n");
	}
}

void dsp3210_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c%c%c%c%c%c%c",
					(m_dauflags & DAU_N) ? 'N' : '.',
					(m_dauflags & DAU_Z) ? 'Z' : '.',
					(m_dauflags & DAU_U) ? 'U' : '.',
					(m_dauflags & DAU_V) ? 'V' : '.',
					n_flag() ? 'n' : '.',
					z_flag() ? 'z' : '.',
					c_flag() ? 'c' : '.',
					v_flag() ? 'v' : '.');
			break;

		case DSP3210_A0:
		case DSP3210_A1:
		case DSP3210_A2:
		case DSP3210_A3:
			str = string_format("%12g", dsp3210dau::to_double(m_acc[entry.index() - DSP3210_A0]));
			break;
	}
}


std::unique_ptr<util::disasm_interface> dsp3210_device::create_disassembler()
{
	return std::make_unique<dsp3210_disassembler>();
}


//**************************************************************************
//  MEMORY ACCESSORS (and associated utilities)
//**************************************************************************

// a bus error on the access in progress; a debugger read through the
// faulting region must not inject one
void dsp3210_device::bus_error()
{
	if (!machine().side_effects_disabled())
	{
		m_berr = true;
	}
}

// A misaligned word or long access raises the Address Error (vector 4) if
// emr[4] is set.
inline offs_t dsp3210_device::check_align(offs_t addr, int size)
{
	if (addr & (size - 1))
	{
		if (take_error(4))
		{
			throw abort_exception();
		}
		addr &= ~offs_t(size - 1);
	}
	return addr;
}

// a bus error signalled during an external data access aborts the instruction
inline void dsp3210_device::check_berr()
{
	if (m_berr)
	{
		m_berr = false;
		if (take_error(1))
		{
			throw abort_exception();
		}
	}
}

void dsp3210_device::update_onchip_base()
{
	m_onchip_base = BIT(m_pcw, 10) ? 0x00000000 : 0x50030000;
}

// instruction fetch: a bus error here is taken at the next instruction
// boundary (the fetched word is never executed), never by the data access
// of the instruction executing now.
inline uint32_t dsp3210_device::fetch(offs_t addr)
{
	addr &= ~3;
	if (onchip(addr))
	{
		return onchip_read(addr & 0xffff, 4);
	}
	const uint32_t data = m_cache.read_dword(addr);
	if (m_berr)
	{
		m_berr = false;
		m_fetch_berr = true;
	}
	return data;
}

inline uint8_t dsp3210_device::read_byte(offs_t addr)
{
	if (onchip(addr))
	{
		return onchip_read(addr & 0xffff, 1);
	}
	const uint8_t data = m_program.read_byte(addr);
	check_berr();
	return data;
}

inline uint16_t dsp3210_device::read_word(offs_t addr)
{
	addr = check_align(addr, 2);
	if (onchip(addr))
	{
		return onchip_read(addr & 0xffff, 2);
	}
	const uint16_t data = m_program.read_word(addr);
	check_berr();
	return data;
}

inline uint32_t dsp3210_device::read_dword(offs_t addr)
{
	addr = check_align(addr, 4);
	if (onchip(addr))
	{
		return onchip_read(addr & 0xffff, 4);
	}
	const uint32_t data = m_program.read_dword(addr);
	check_berr();
	return data;
}

inline void dsp3210_device::write_byte(offs_t addr, uint8_t data)
{
	if (onchip(addr))
	{
		onchip_write(addr & 0xffff, 1, data);
	}
	else
	{
		m_program.write_byte(addr, data);
		check_berr();
	}
}

inline void dsp3210_device::write_word(offs_t addr, uint16_t data)
{
	addr = check_align(addr, 2);
	if (onchip(addr))
	{
		onchip_write(addr & 0xffff, 2, data);
	}
	else
	{
		m_program.write_word(addr, data);
		check_berr();
	}
}

inline void dsp3210_device::write_dword(offs_t addr, uint32_t data)
{
	addr = check_align(addr, 4);
	if (onchip(addr))
	{
		onchip_write(addr & 0xffff, 4, data);
	}
	else
	{
		m_program.write_dword(addr, data);
		check_berr();
	}
}


//**************************************************************************
//  ON-CHIP WINDOW
//**************************************************************************

uint32_t dsp3210_device::onchip_read(offs_t offset, int width)
{
	const offs_t word = offset & ~3;
	if (mmio_live(word))
	{
		const int shift = 8 * (4 - (offset & 3) - width);
		return (mmio_read(word) & lane_mask(offset, width)) >> shift;
	}

	const uint8_t *const p = &m_onchip[offset];
	switch (width)
	{
		case 1:  return p[0];
		case 2:  return get_u16be(p);
		default: return get_u32be(p);
	}
}

void dsp3210_device::onchip_write(offs_t offset, int width, uint32_t data)
{
	if (offset < ONCHIP_ROM_END)
	{
		return;                             // boot ROM
	}

	const offs_t word = offset & ~3;
	if (mmio_live(word))
	{
		const int shift = 8 * (4 - (offset & 3) - width);
		mmio_write(word, data << shift, lane_mask(offset, width));
		return;
	}

	uint8_t *const p = &m_onchip[offset];
	switch (width)
	{
		case 1:  p[0] = data; break;
		case 2:  put_u16be(p, data); break;
		default: put_u32be(p, data); break;
	}
}

// the "onchip" space's view of the live MMIO words
uint8_t dsp3210_device::onchip_debug_r(offs_t offset)
{
	return onchip_read(MMIO_TCON + offset, 1);
}

void dsp3210_device::onchip_debug_w(offs_t offset, uint8_t data)
{
	onchip_write(MMIO_TCON + offset, 1, data);
}

// the timer output as it appears on BIO1 under tcon[4]: the toggle
// flip-flop, or the idle level of the one-CKI pulse (active low unless
// tcon[3]) - a read never lands on the pulse itself
bool dsp3210_device::timer_output() const
{
	return BIT(m_tcon, 2) ? m_timer_out : !BIT(m_tcon, 3);
}

// the 32-bit image of a live MMIO word: the register in its lanes, zero
// in the lanes nothing sits in
uint32_t dsp3210_device::mmio_read(offs_t word)
{
	switch (word)
	{
		case MMIO_TCON:
			return m_tcon;

		case MMIO_TIMER:
			return m_timer;

		case MMIO_BIOC:
			return m_bioc;

		case MMIO_BIO:
		{
			// per pin, bit 2n = the pin level and bit 2n+1 = the output
			// register bit; an output pin shows what it drives, which
			// for BIO1 under tcon[4] is the timer output
			uint8_t pins = (m_bio_in_cb() & ~m_bioc) | (m_bio & m_bioc);
			if (BIT(m_bioc, 1) && BIT(m_tcon, 4))
			{
				pins = (pins & ~0x02) | (timer_output() << 1);
			}
			uint32_t v = 0;
			for (int i = 0; i < 8; i++)
			{
				v |= (BIT(pins, i) << (2 * i)) | (BIT(m_bio, i) << (2 * i + 1));
			}
			return v;
		}

		default:
			return 0;
	}
}

// a write to a live MMIO word; `mask` is the lanes the access covers, and
// a lane outside it is untouched (for bio, an unwritten field is 00 = keep)
void dsp3210_device::mmio_write(offs_t word, uint32_t data, uint32_t mask)
{
	switch (word)
	{
		case MMIO_TCON:
			if (mask & 0xff)
			{
				m_tcon = data & 0xff;
			}
			break;

		case MMIO_TIMER:
			// loads the counter and the initial count register; the load
			// takes the place of this slot's decrement
			m_timer = m_timer_reload = (m_timer & ~mask) | (data & mask);
			m_timer_loaded = true;
			break;

		case MMIO_BIOC:
			if (mask & 0xff)
			{
				m_bioc = data & 0xff;
				m_bio_dir_cb(m_bioc);
			}
			break;

		case MMIO_BIO:
		{
			// eight 2-bit fields, BFn at bits 2n+1:2n - 00 keep, 01 clear,
			// 10 set, 11 toggle
			const uint32_t fields = data & mask & 0xffff;
			uint8_t bio = m_bio;
			for (int i = 0; i < 8; i++)
			{
				switch (BIT(fields, 2 * i, 2))
				{
					case 1: bio &= ~(1 << i); break;
					case 2: bio |= 1 << i; break;
					case 3: bio ^= 1 << i; break;
				}
			}
			// the output register is observed regardless of bioc: the AV
			// kernel rings its doorbell by toggling BIO0 without ever
			// making it an output
			if (bio != m_bio)
			{
				m_bio = bio;
				m_bio_out_cb(m_bio);
			}
			break;
		}

		default:
			break;
	}
}


//**************************************************************************
//  I/O REGISTERS
//**************************************************************************

uint16_t dsp3210_device::ps_r() const
{
	uint16_t ps = 0;
	if (n_flag())
	{
		ps |= 0x0001;
	}
	if (z_flag())
	{
		ps |= 0x0002;
	}
	if (v_flag())
	{
		ps |= 0x0004;
	}
	if (c_flag())
	{
		ps |= 0x0008;
	}
	ps |= (m_dauflags & 0x0f) << 4;         // N Z U V
	ps |= 0x0200;                           // OBE: with no SIO the output buffer is always empty
	ps |= (m_ps_pins & 3) << 12;            // IR0/IR1 live levels, 1 = negated
	return ps;
}

uint32_t dsp3210_device::ior_read(int reg)
{
	switch (reg)
	{
		case IOR_PS:   return ps_r();
		case IOR_EMR:  return m_emr;
		case IOR_SPC:  return 0;            // write-only
		case IOR_PCW:  return m_pcw;
		case IOR_DAUC: return m_dauc;
		case IOR_CTR:  return m_ctr;
		default:
			logerror("read of reserved ior%d at %08X\n", reg, m_ppc);
			return 0;
	}
}

void dsp3210_device::ior_write(int reg, int width, uint32_t data)
{
	// a byte or short move to an I/O register affects only that lane
	const uint32_t mask = (width == 1) ? 0xff : (width == 2) ? 0xffff : 0xffffffff;

	switch (reg)
	{
		case IOR_PS:
			// only nzvc are writable
			m_nzcflags = (uint64_t(BIT(data, 3)) << 32) | (BIT(data, 0) ? 0x80000000 : (BIT(data, 1) ? 0 : 1));
			m_vflags = BIT(data, 2) ? 0x80000000 : 0;
			break;

		case IOR_EMR:
			// bit 0 (SL) puts the processing level on the IACK pins
			m_emr = (m_emr & ~mask) | (data & mask & 0xffff);
			update_iack();
			break;

		case IOR_SPC:
			do_spc(width);
			break;

		case IOR_PCW:
			if (!BIT(m_pcw, 13))            // BRC locks pcw until reset or an error exception
			{
				m_pcw = (m_pcw & ~mask) | (data & mask & 0xffff);
				update_onchip_base();
				if (!BIT(m_pcw, 8))
				{
					logerror("pcw = %04X selects little-endian mode, not supported\n", m_pcw);
				}
			}
			break;

		case IOR_DAUC:
			m_dauc = data & 0x7f;
			break;

		case IOR_CTR:
			m_ctr = data & 0x3f;
			break;

		default:
			logerror("write of %08X to reserved ior%d at %08X\n", data, reg, m_ppc);
			break;
	}
}

// the spc pseudo-register: a store selects the operation by its size
void dsp3210_device::do_spc(int width)
{
	switch (width)
	{
		case 4:
			// waiti: ignored if an interrupt is already pending; its
			// latent instruction runs when one is recognised
			if (!pending_vector())
			{
				m_waiting = true;
				m_asleep_slots = 0;
			}
			break;

		case 2:
			// bkpt: hardware behaviour undocumented; stop until reset,
			// letting the DA stores in the pipeline complete
			m_halted = true;
			flush_mbuf();
			logerror("bkpt at %08X\n", m_ppc);
			break;

		case 1:
			// sftrst: drop from error level back to base level
			m_level = 0;
			m_error_nonmaskable = false;
			update_iack();
			break;
	}
}


//**************************************************************************
//  INTERRUPTS AND PINS
//**************************************************************************

// the highest-priority unmasked pending interrupt (8 = EXT0 highest,
// 15 = EXT1 lowest), or 0
int dsp3210_device::pending_vector() const
{
	const uint16_t p = m_pending & m_emr & 0xff00;
	if (p == 0)
	{
		return 0;
	}
	for (int vn = 8; vn <= 15; vn++)
	{
		if (BIT(p, vn))
		{
			return vn;
		}
	}
	return 0;
}

void dsp3210_device::update_ir_pin(int pin, int state)
{
	const uint8_t mask = 1 << pin;
	const bool asserted = (state != CLEAR_LINE);
	const bool was_asserted = !(m_ps_pins & mask);

	// the request is latched on the asserting edge; ps mirrors the level
	if (asserted && !was_asserted)
	{
		m_pending |= 1 << (pin ? 15 : 8);
	}
	if (asserted)
	{
		m_ps_pins &= ~mask;
	}
	else
	{
		m_ps_pins |= mask;
	}
}

void dsp3210_device::execute_set_input(int inputnum, int state)
{
	switch (inputnum)
	{
		case DSP3210_IR0:
		case DSP3210_IR1:
			update_ir_pin(inputnum, state);
			break;

		case DSP3210_BERR:
			// delivered between instructions: the error is taken at the
			// next boundary
			if (state != CLEAR_LINE)
			{
				m_berr = true;
			}
			break;
	}
}

// the IACK pins: with emr[0] (SL) clear they acknowledge the external
// interrupt being serviced, with SL set they show the processing level
// (00 base, 01 interrupt, 10 error); a double error asserts both.  An SL
// pulse inside an EXT1 handler drops IACK1 for one instruction, which a
// board can use to acknowledge IR1N.
void dsp3210_device::update_iack()
{
	uint8_t iack[2];
	if (m_level == 3)
	{
		iack[0] = iack[1] = 1;
	}
	else if (BIT(m_emr, 0))
	{
		iack[0] = BIT(m_level, 0);
		iack[1] = BIT(m_level, 1);
	}
	else
	{
		iack[0] = (m_servicing == 8) ? 1 : 0;
		iack[1] = (m_servicing == 15) ? 1 : 0;
	}
	for (int pin = 0; pin < 2; pin++)
	{
		if (iack[pin] != m_iack[pin])
		{
			m_iack[pin] = iack[pin];
			m_iack_cb[pin](iack[pin]);
		}
	}
}

// Error entry (7.5.2, Table 7-3).  Returns whether it was taken.
bool dsp3210_device::take_error(int vn)
{
	if ((vn >= 4) && !BIT(m_emr, vn))
	{
		return false;
	}
	if (m_level == 3)
	{
		return true;
	}
	if (m_level == 2)
	{
		if (vn >= 4)
		{
			return false;
		}
		if (m_error_nonmaskable)
		{
			m_level = 3;
			m_halted = true;
			update_iack();
			return true;
		}
	}

	m_do_active = false;
	m_waiting = false;
	m_error_nonmaskable = (vn < 4);
	m_level = 2;
	m_pcw &= ~(1 << 13);
	m_tcon &= ~1;
	for (mbuf_entry &e : m_mbuf)
	{
		e.width = 0;
	}

	m_r[RC_R20] = m_ppc + 8;
	m_pc = m_r[RC_EVTP] + vn * 8;
	m_npc = m_pc + 4;
	m_prefetch = fetch(m_pc);
	m_icount -= 11;                         // three CKI of error state and the dispatch
	update_iack();
	return true;
}

void dsp3210_device::take_interrupt(int vn)
{
	m_pending &= ~(1 << vn);

	m_sh_nzcflags = m_nzcflags;
	m_sh_vflags = m_vflags;
	m_sh_dauflags = m_dauflags;
	m_sh_dauc = m_dauc;
	m_sh_ctr = m_ctr;
	std::copy(std::begin(m_acc), std::end(m_acc), std::begin(m_sh_acc));
	m_sh_apipe_m = m_apipe_m;
	m_sh_apipe_e = m_apipe_e;
	std::copy(std::begin(m_fpipe), std::end(m_fpipe), std::begin(m_sh_fpipe));
	m_sh_slot = m_slot;
	m_sh_do_active = m_do_active;
	m_sh_do_lock = m_do_lock;
	m_sh_do_start = m_do_start;
	m_sh_do_end = m_do_end;
	m_sh_do_count = m_do_count;
	m_do_active = false;

	m_irsh = m_prefetch;
	m_irsh_addr = m_pc;
	m_r[RC_PCSH] = m_pc + 4;
	m_level = 1;
	m_servicing = vn;
	m_pc = m_r[RC_EVTP] + vn * 8;
	m_npc = m_pc + 4;
	m_prefetch = fetch(m_pc);
	m_icount -= 8;                          // two instruction cycles
	update_iack();
}

// The timer (9.3) is clocked once per instruction slot at CKI/4 and twice
// at CKI/2, including while asleep in waiti.
void dsp3210_device::tick_timer()
{
	if (m_timer_loaded)
	{
		m_timer_loaded = false;
		return;
	}
	uint32_t ticks = timer_ticks_per_slot();
	while (ticks--)
	{
		if (m_timer == 0)
		{
			if (!BIT(m_tcon, 1))
			{
				return;                     // one-shot: holds at zero
			}
			m_timer = m_timer_reload;       // the reload consumes a tick
			if (m_timer != 0)
			{
				continue;
			}
		}
		else if (--m_timer != 0)
		{
			continue;
		}
		timer_zero();
	}
}

// the count reached zero: the vector-9 request (taken if emr[9]) and the
// output toggle flip-flop
void dsp3210_device::timer_zero()
{
	m_pending |= 1 << 9;
	m_timer_out = !m_timer_out;
}

// timer ticks per instruction slot, 0 when the timer is not counting from CKI
uint32_t dsp3210_device::timer_ticks_per_slot() const
{
	if (!BIT(m_tcon, 0))
	{
		return 0;
	}
	switch (BIT(m_tcon, 5, 3))
	{
		case 0:  return 2;
		case 1:  return 1;
		default: return 0;
	}
}

// how many slots a settled sleep can skip at once
uint32_t dsp3210_device::sleep_slots() const
{
	uint32_t n = std::max(m_icount / 4, 1);
	const uint32_t tps = timer_ticks_per_slot();
	if (tps)
	{
		if (m_timer_loaded || ((m_timer == 0) && BIT(m_tcon, 1)))
		{
			n = 1;
		}
		else if (m_timer != 0)
		{
			n = std::min<uint32_t>(n, (m_timer - 1) / tps);
		}
	}
	return n;
}

// a settled sleep with no timer event due in the next two slots can be parked
bool dsp3210_device::can_park() const
{
	const uint32_t tps = timer_ticks_per_slot();
	if (!tps)
	{
		return true;
	}
	if (m_timer_loaded)
	{
		return false;
	}
	if (m_timer == 0)
	{
		return !BIT(m_tcon, 1);             // an ended one-shot holds at zero
	}
	return m_timer >= (2 * tps);
}

TIMER_CALLBACK_MEMBER(dsp3210_device::wake_tick)
{
	signal_interrupt_trigger();
}

// everything that happens once per slot of core time, executed or asleep
inline void dsp3210_device::end_slot()
{
	advance_pipes();
	tick_timer();
}


//**************************************************************************
//  INSTRUCTION HANDLERS AND DISPATCH TABLE
//**************************************************************************

#include "dsp3210ops.hxx"
#include "dsp3210tbl.hxx"


//**************************************************************************
//  CORE EXECUTION LOOP
//**************************************************************************

// a DA store queued four slots ago lands at the start of this one; a bus
// error on it enters the error exception like any data access
void dsp3210_device::drain_mbuf()
{
	mbuf_entry &e = m_mbuf[m_slot & 3];
	if (e.width)
	{
		const mbuf_entry store = e;
		e.width = 0;
		try
		{
			mem_write(store.addr, store.width, store.data);
		}
		catch (abort_exception const &)
		{
			// the error exception has been entered
		}
	}
}

void dsp3210_device::execute_run()
{
	if (m_halted)
	{
		debugger_wait_hook();
		m_icount = 0;
		return;
	}

	if (!m_prefetch_valid)
	{
		m_prefetch = fetch(m_pc);
		m_prefetch_valid = true;
	}

	if (m_parked)
	{
		// back from a parked sleep
		m_parked = false;
		m_wake_timer->adjust(attotime::never);
		uint64_t slots = (total_cycles() - m_park_cycles) / 4;
		const uint32_t tps = timer_ticks_per_slot();
		if (tps && (m_timer != 0))
		{
			slots = std::min<uint64_t>(slots, (m_timer - 1) / tps);
			m_timer -= slots * tps;
		}
		m_slot += slots;
	}

	do
	{
		drain_mbuf();

		// a bus error signalled by the line, or by the last fetch
		if (m_berr || m_fetch_berr)
		{
			m_berr = false;
			m_fetch_berr = false;
			take_error(1);
		}

		// waiti: asleep until an unmasked request or an error arrives.
		if (m_waiting)
		{
			if (pending_vector())
			{
				m_waiting = false;
				m_int_defer = true;         // the latent instruction runs before the interrupt is taken
			}
			else
			{
				debugger_wait_hook();

				// the first four slots drain the deferred ring and settle
				// the pipes one at a time.
				if (m_asleep_slots < 4)
				{
					m_asleep_slots++;
					end_slot();
					m_icount -= 4;
					continue;
				}
				if (can_park())
				{
					// nothing but the timer can happen: suspend until a
					// line is asserted or the timer's count runs out
					const uint32_t tps = timer_ticks_per_slot();
					if (tps && (m_timer != 0))
					{
						m_wake_timer->adjust(clocks_to_attotime(4 * (uint64_t(m_timer) + tps - 1) / tps));
					}
					m_park_cycles = total_cycles();
					m_parked = true;
					spin_until_interrupt();
					m_icount = 0;
					return;
				}
				const uint32_t n = sleep_slots();
				if (n <= 1)
				{
					end_slot();
					m_icount -= 4;
					continue;
				}
				const uint32_t tps = timer_ticks_per_slot();
				if (tps && (m_timer != 0))
				{
					m_timer -= n * tps;     // sleep_slots() stops short of zero
				}
				m_slot += n;                // the pipes already hold the resting state
				m_icount -= 4 * n;
				continue;
			}
		}

		// interrupt recognition: base level, not in the shadow of a
		// branch, not inside a dolock loop, no irsh replay pending and
		// not deferred by waiti's latent instruction
		if (!m_int_defer && !m_irsh_pending && (m_level == 0) && (m_npc == m_pc + 4) && !(m_do_active && m_do_lock))
		{
			const int vn = pending_vector();
			if (vn)
			{
				take_interrupt(vn);
			}
		}
		m_int_defer = false;

		if (m_irsh_pending)
		{
			// ireturn: the word prefetched before the interrupt executes
			// in place of a delay slot; pc/npc already aim at pcsh
			m_irsh_pending = false;
			m_ppc = m_irsh_addr;
			debugger_instruction_hook(m_ppc);
			m_prefetch = fetch(m_pc);
			try
			{
				(this->*s_ops[m_irsh >> 21])(m_irsh);
			}
			catch (abort_exception const &)
			{
				// an error exception aborted the instruction
			}
		}
		else
		{
			m_ppc = m_pc;
			debugger_instruction_hook(m_pc);

			// The word after this one is fetched before this one's side
			// effects.  A memory-map switch or a store to the next
			// address is not seen by the already fetched instruction.
			const uint32_t op = m_prefetch;
			m_pc = m_npc;
			m_npc = m_pc + 4;
			m_prefetch = fetch(m_pc);

			try
			{
				(this->*s_ops[op >> 21])(op);
			}
			catch (abort_exception const &)
			{
				// an error exception aborted the instruction
			}
		}

		if (m_do_active && (m_ppc == m_do_end))
		{
			if (--m_do_count == 0)
			{
				m_do_active = false;
			}
			else
			{
				m_pc = m_do_start;
				m_npc = m_pc + 4;
				m_prefetch = fetch(m_pc);
			}
		}

		end_slot();
		m_icount -= 4;
	} while ((m_icount > 0) && !m_halted);
}
