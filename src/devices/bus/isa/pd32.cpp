// license:BSD-3-Clause
// copyright-holders:Dave Rand
/*********************************************************************

    PD32 public-domain NS32016 coprocessor board

    Host interface (PAL lhu646/lhudec32, ports at 170h):
      170h  R/W  the data latch
      172h  W    software reset -- LATCHES until an INT32 write
            R    bit 0 = latch full (PD32-to-host byte waiting)
      174h  W    INT32: interrupt the 32016, release latched reset
      176h  W    clear service request
            R    bit 0 = SREQ (PD32 wants service)
    SREQ also drives a host interrupt via jumper J2.

    32016 memory map (PAL lhudec32):
      000000  EPROM while SWAP (power-on/reset), RAM after
      800000  RAM alias while SWAP only
      e00000+ top region, decoded by A8/A9:
        A9=0 A8=0  PARIO: the data latch (used at fc0000)
        A9=0 A8=1  INT86: a READ asserts SREQ (fc0100)
        A9=1 A8=0  NS32202 ICU, registers on 4-byte stride (fffe00)
    SWAP clears when the boot ROM (relocated to RAM) writes the ICU
    PDAT register with G1 low.

    The latch is one 16-bit word deep with hardware wait-states on
    both sides: the 32016 stalls on an empty read or full write via
    /RDY (CWAIT), and the host stalls via IOCHRDY (the i86 core's
    port access retry), exactly as the PALs arranged it.  This
    lockstep is load-bearing: both the boot ROM and the kernel
    dismiss the INT32 doorbell once per message, so neither side
    may ever run a transfer ahead of the other.  SREQ asserts
    combinationally, and the host software reset resets the whole
    32016 complex (CPU, MMU, FPU, ICU, latch and flags).

*********************************************************************/

#include "emu.h"
#include "pd32.h"

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(ISA8_PD32, isa8_pd32_device, "pd32", "PD32 coprocessor (Rand/Scolaro)")

static constexpr int TRIG_LATCH = 78232;
static constexpr int TRIG_HOST = 78233;
static constexpr int TRIG_HOSTW = 78234;

isa8_pd32_device::isa8_pd32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ISA8_PD32, tag, owner, clock)
	, device_isa8_card_interface(mconfig, *this)
	, device_memory_interface(mconfig, *this)
	, m_cpu(*this, "cpu")
	, m_fpu(*this, "fpu")
	, m_mmu(*this, "mmu")
	, m_icu(*this, "icu")
	, m_eprom(*this, "eprom")
	, m_icu_config("icu", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(isa8_pd32_device::icu_map), this))
	, m_lowmem(*this, "lowmem")
	, m_himem(*this, "himem")
	, m_ram(*this, "ram", 0x200000, ENDIANNESS_LITTLE)
	, m_pdir(0xff)
	, m_pdat(0xff)
	, m_swap(true)
	, m_swap_staged(false)
	, m_swap_entry(0)
	, m_rsti(false)
	, m_sreq(false)
	, m_pair_even(0)
	, m_pair_valid(false)
	, m_wpair_valid(false)
{
}

ROM_START(pd32)
	ROM_REGION16_LE(0x800, "eprom", 0)
	ROM_LOAD16_BYTE("rom.000", 0x000, 0x400, CRC(496e8088) SHA1(522056eb821b6c875f8ce0963bc3cd24e7303759))
	ROM_LOAD16_BYTE("rom.001", 0x001, 0x400, CRC(b2b45c15) SHA1(6a8b0e6cd27fd3a7d7ca870790c6793edf9de083))
ROM_END

const tiny_rom_entry *isa8_pd32_device::device_rom_region() const
{
	return ROM_NAME(pd32);
}

device_memory_interface::space_config_vector isa8_pd32_device::memory_space_config() const
{
	return space_config_vector{ std::make_pair(0, &m_icu_config) };
}

void isa8_pd32_device::icu_map(address_map &map)
{
	map(0x00, 0x1f).m(m_icu, FUNC(ns32202_device::map<0>));
}

// only the HVCT byte at FFFE00 is fetched in these cycles
void isa8_pd32_device::cpu_iam_map(address_map &map)
{
	map(0xfffe00, 0xfffe1f).m(m_icu, FUNC(ns32202_device::map<0>));
}

void isa8_pd32_device::cpu_eim_map(address_map &map)
{
	map(0xfffe00, 0xfffe1f).m(m_icu, FUNC(ns32202_device::map<1>));
}

// the board wires the ICU's A0-A4 to the CPU's A2-A6
uint8_t isa8_pd32_device::icu_r(offs_t offset)
{
	unsigned const reg = offset >> 2;

	// PDAT reads: output pins echo the output latch (which resets
	// high -- G1 must stay up through read-modify-writes until
	// explicitly dropped); input pins read the hardware.  G4 is
	// wired to the latch FULL flag: the kernel's console output
	// polls it waiting for the host to drain each byte pair.
	if (reg == 19)
	{
		uint8_t hw = 0xff;
		if (m_to_host.empty())
			hw &= ~0x10;
		return (m_pdat & ~m_pdir) | (hw & m_pdir);
	}

	return m_icu_space.read_byte(reg);
}

void isa8_pd32_device::icu_w(offs_t offset, uint8_t data)
{
	unsigned const reg = offset >> 2;

	// the boot ROM swaps RAM in by configuring G1 as an output (PDIR)
	// and then dropping it in the port data (PDAT); the PDAT update
	// alone must not trigger (the self-test indicator writes get there
	// first, while G1 is still an input)
	if (reg == 21)
		m_pdir = data;
	if (reg == 19)
	{
		m_pdat = data;
		// G1 low with the EPROM still mapped: the handoff has begun,
		// but the remaining handoff code (the tail of the execute
		// handler and the relocated stub) still runs from the EPROM
		// contents via the real chip's prefetch queue; stage the
		// swap and commit it at the stub's IPS write below
		if (!BIT(m_pdir, 1) && !BIT(data, 1) && m_swap)
			m_swap_staged = true;
	}
	if (reg == 20 && m_swap_staged)
	{
		// the stub's final act before jumping to the entry vector;
		// commit the swap and complete the prefetched jump through
		// the vector at 2Ch.  The jump must land after the current
		// (IPS-write) instruction retires - the core adds the
		// instruction length to PC on completion, so poking PC from
		// inside this handler would land entry+length and skip the
		// kernel's first instruction (the SETCFG).  Defer it to a
		// zero-delay callback between instructions.
		m_swap_staged = false;
		set_swap(false);
		m_swap_entry = m_ram[0x2c] | (m_ram[0x2d] << 8) | (m_ram[0x2e] << 16) | (m_ram[0x2f] << 24);
		LOG("EPROM swapped out; kernel entry %06x\n", m_swap_entry);
		m_cpu->abort_timeslice();
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(isa8_pd32_device::swap_jump), this));
	}

	m_icu_space.write_byte(reg, data);
}

void isa8_pd32_device::cpu_map(address_map &map)
{
	// boot: EPROM at zero (2KB used, mirrored through the low decode)
	map(0x000000, 0x0007ff).view(m_lowmem);
	m_lowmem[0](0x000000, 0x0007ff).rom().region("eprom", 0);
	m_lowmem[1](0x000000, 0x0007ff).lrw8(
			NAME([this](offs_t offset) { return m_ram[offset]; }),
			NAME([this](offs_t offset, uint8_t data) { m_ram[offset] = data; }));
	map(0x000800, 0x1fffff).lrw8(
			NAME([this](offs_t offset) { return m_swap ? 0xff : m_ram[offset + 0x800]; }),
			NAME([this](offs_t offset, uint8_t data) { if (!m_swap) m_ram[offset + 0x800] = data; }));

	// RAM alias at 800000 during boot only
	map(0x800000, 0x9fffff).view(m_himem);
	m_himem[0](0x800000, 0x9fffff).lrw8(
			NAME([this](offs_t offset) { return m_ram[offset]; }),
			NAME([this](offs_t offset, uint8_t data) { m_ram[offset] = data; }));

	// unpopulated decode space probed by the sizing march
	map(0x200000, 0x7fffff).noprw();
	map(0xa00000, 0xdfffff).noprw();

	// the top region: PARIO / INT86 / ICU decoded by A8-A9
	map(0xe00000, 0xffffff).rw(FUNC(isa8_pd32_device::top_r), FUNC(isa8_pd32_device::top_w));
	map(0xfffe00, 0xfffe7f).rw(FUNC(isa8_pd32_device::icu_r), FUNC(isa8_pd32_device::icu_w));
}

void isa8_pd32_device::device_add_mconfig(machine_config &config)
{
	NS32016(config, m_cpu, 10'000'000);
	m_cpu->set_addrmap(0, &isa8_pd32_device::cpu_map);
	// interrupt-acknowledge and end-of-interrupt (RETI) bus cycles
	// read the ICU's HVCT with distinct ST codes; the ICU clears the
	// in-service bit only on the end-of-interrupt flavour
	m_cpu->set_addrmap(4, &isa8_pd32_device::cpu_iam_map);
	m_cpu->set_addrmap(6, &isa8_pd32_device::cpu_eim_map);

	NS32081(config, m_fpu, 10'000'000);
	m_cpu->set_fpu(m_fpu);

	NS32082(config, m_mmu, 10'000'000);
	m_cpu->set_mmu(m_mmu);

	NS32202(config, m_icu, 10'000'000);
	m_icu->out_int().set(FUNC(isa8_pd32_device::icu_int_w));
}

void isa8_pd32_device::device_start()
{
	set_isa_device();

	space(0).specific(m_icu_space);

	save_item(NAME(m_swap));
	save_item(NAME(m_swap_staged));
	save_item(NAME(m_swap_entry));
	save_item(NAME(m_pdir));
	save_item(NAME(m_pdat));
	save_item(NAME(m_rsti));
	save_item(NAME(m_sreq));
	save_item(NAME(m_pair_even));
	save_item(NAME(m_pair_valid));
	save_item(NAME(m_wpair_valid));

}

void isa8_pd32_device::device_reset()
{
	remap(AS_IO, 0, 0xffff);

	m_to_pd32.clear();
	m_to_host.clear();
	m_pdir = 0xff;
	m_pdat = 0xff;
	set_swap(true);
	m_rsti = false;
	set_sreq(false);
	update_reset();
}

void isa8_pd32_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_IO)
		m_isa->install_device(0x170, 0x177,
				read8sm_delegate(*this, FUNC(isa8_pd32_device::host_r)),
				write8sm_delegate(*this, FUNC(isa8_pd32_device::host_w)));
}

void isa8_pd32_device::set_swap(bool swap)
{
	m_swap = swap;
	m_lowmem.select(swap ? 0 : 1);
	// the PAL stops decoding the 800000 alias once SWAP clears, but the
	// swap stub executes from the alias and survives on the real chip's
	// prefetch queue; keep the alias alive instead (nothing else uses it)
	m_himem.select(0);
}

void isa8_pd32_device::update_reset()
{
	m_cpu->set_input_line(INPUT_LINE_RESET, m_rsti ? ASSERT_LINE : CLEAR_LINE);
	if (m_rsti)
	{
		// the host's software reset resets the whole 32016 complex:
		// without this, a second UNIX load runs with the previous
		// kernel's MMU translation still enabled over reloaded memory
		m_mmu->reset();
		m_fpu->reset();
		m_icu->reset();

		// and returns the EPROM to low memory and flushes the latch
		set_swap(true);
		m_to_pd32.clear();
		m_to_host.clear();
		m_pair_valid = false;
		m_wpair_valid = false;
		set_sreq(false);
	}
}

TIMER_CALLBACK_MEMBER(isa8_pd32_device::swap_jump)
{
	// complete the handoff stub's prefetched "jump r0" (see icu_w)
	m_cpu->set_state_int(STATE_GENPC, m_swap_entry);
}

void isa8_pd32_device::set_sreq(bool state)
{
	m_sreq = state;
	// SREQ drives a host interrupt via jumper J2; polled mode needs no line
	m_isa->irq5_w(state ? 1 : 0);
}

void isa8_pd32_device::icu_int_w(int state)
{
	// the ICU's /INT output is active low
	m_cpu->set_input_line(INPUT_LINE_IRQ0, state ? CLEAR_LINE : ASSERT_LINE);
}

//-------------------------------------------------
//  host side
//-------------------------------------------------

uint8_t isa8_pd32_device::host_r(offs_t offset)
{
	// A0 is not decoded: odd ports alias their even neighbours (the
	// host's 16-bit string I/O arrives as byte pairs at 170h/171h)
	switch (offset & 6)
	{
	case 0: // data latch
		if (m_to_host.empty())
		{
			// the board wait-states the host (IWAIT/IOCHRDY) until the
			// PD32 writes; arm an access retry on the reading CPU and
			// suspend it until data arrives.  If this is the odd half
			// of a 16-bit read, give back the even byte consumed a
			// moment ago so the restarted instruction sees both.
			if (!machine().side_effects_disabled())
			{
				if (BIT(offset, 0) && m_pair_valid)
					m_to_host.push_front(m_pair_even);
				m_pair_valid = false;
				cpu_device *const cpu = dynamic_cast<cpu_device *>(&machine().scheduler().currently_executing()->device());
				if (cpu)
				{
					cpu->retry_access();
					cpu->suspend_until_trigger(TRIG_HOST, true);
					cpu->eat_cycles(cpu->cycles_remaining());
				}
			}
			return 0xff;
		}
		else
		{
			uint8_t const data = m_to_host.front();
			if (!machine().side_effects_disabled())
			{
				m_to_host.pop_front();
				m_pair_even = data;
				m_pair_valid = !BIT(offset, 0);
			}
			return data;
		}

	case 2: // bit 0 = latch full (ID0 = FULL0 + FULL1, active low: 0 =
		// data present).  The 646 latch is shared by both directions,
		// so the host's send loops poll this to know when the PD32
		// has consumed the previous word.
		return (m_to_host.empty() && m_to_pd32.empty()) ? 0xff : 0xfe;

	case 6: // bit 0 = service request (active low: 0 = PD32 wants service)
		return m_sreq ? 0xfe : 0xff;
	}

	return 0xff;
}

void isa8_pd32_device::host_w(offs_t offset, uint8_t data)
{
	switch (offset & 6)
	{
	case 0: // data latch
		// the 74LS646 holds a single 16-bit word: the PAL wait-states
		// host writes (FIX -> IOCHRDY) while both byte lanes are full.
		// This lockstep is load-bearing: the boot ROM and the kernel
		// clear the INT32 doorbell once per message, so the host must
		// never run more than one transfer ahead.
		if (m_to_pd32.size() >= 2)
		{
			if (!machine().side_effects_disabled())
			{
				// if this is the odd half of a word whose even byte
				// was just accepted, give it back so the restarted
				// instruction rewrites both
				if (BIT(offset, 0) && m_wpair_valid)
				{
					m_to_pd32.pop_back();
					m_wpair_valid = false;
				}
				cpu_device *const cpu = dynamic_cast<cpu_device *>(&machine().scheduler().currently_executing()->device());
				if (cpu)
				{
					cpu->retry_access();
					cpu->suspend_until_trigger(TRIG_HOSTW, true);
					cpu->eat_cycles(cpu->cycles_remaining());
				}
			}
			break;
		}
		m_to_pd32.push_back(data);
		m_wpair_valid = !BIT(offset, 0);
		machine().scheduler().trigger(TRIG_LATCH);
		break;

	case 2: // software reset, latching
		LOG("host reset\n");
		m_rsti = true;
		update_reset();
		break;

	case 4: // INT32: release reset, interrupt the 32016
		machine().scheduler().trigger(TRIG_LATCH);
		if (m_rsti)
		{
			LOG("host releases reset\n");
			m_rsti = false;
			update_reset();
		}
		// INT32 is combinational in the decode PAL: it strobes active
		// (low) only for the duration of the host's OUT instruction,
		// wired to IR13 (caught edge-triggered or by polling IPND)
		m_icu->ir_w<13>(0);
		m_icu->ir_w<13>(1);
		break;

	case 6: // clear service request
		set_sreq(false);
		break;
	}
}

//-------------------------------------------------
//  32016 side
//-------------------------------------------------

uint8_t isa8_pd32_device::top_r(offs_t offset)
{
	offs_t const addr = 0xe00000 + offset;

	if (!BIT(addr, 9))
	{
		if (BIT(addr, 8)) // INT86: a read asserts SREQ
		{
			// SREQ latches in the decode PAL combinationally: it must
			// be visible to the host immediately, or the host's
			// check-SREQ-before-send collision avoidance races the
			// announce and the protocol deadlocks under full-duplex
			// load.  A host that reacts before the message body is
			// queued simply wait-states on the empty latch, exactly
			// as the real hardware did.
			if (!machine().side_effects_disabled())
				set_sreq(true);
			return 0xff;
		}
		else // PARIO
		{
			if (m_to_pd32.empty())
			{
				// hardware wait-states the CPU until the host writes:
				// arm /RDY so the instruction restarts, and suspend
				// until the host side pushes a byte
				if (!machine().side_effects_disabled())
				{
					m_cpu->rdy_w(1);
					m_cpu->suspend_until_trigger(TRIG_LATCH, true);
					m_cpu->eat_cycles(m_cpu->cycles_remaining());
				}
				return 0xff;
			}
			else
			{
				uint8_t const data = m_to_pd32.front();
				if (!machine().side_effects_disabled())
				{
					m_to_pd32.pop_front();
					// wake a host wait-stated on the full latch
					machine().scheduler().trigger(TRIG_HOSTW);
				}
				return data;
			}
		}
	}

	return 0xff;
}

void isa8_pd32_device::top_w(offs_t offset, uint8_t data)
{
	offs_t const addr = 0xe00000 + offset;

	if (!BIT(addr, 9) && !BIT(addr, 8)) // PARIO
	{
		m_to_host.push_back(data);
		machine().scheduler().trigger(TRIG_HOST);
	}
}
