// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
/***************************************************************************

    L.E.Tech 'StarNovax' LE1080A-PA(V1) Security CPU device

    Z80-compatible CPU with additional internal features:

      - internal battery-backed RAM
      - system register block
      - internal interrupt controller
      - internal hardware RNG / random input source


    The following observations describe a recurring unknown ROM region
    seen in current LE-series program ROMs.

    This is not a confirmed LE1080A specification.
    However, the data appears consistently across multiple titles and
    may be relevant to CPU-specific behaviour, initialization,
    or security-related mechanisms.

    Further investigation is required, and future improvements to this
    core may depend on understanding this region.
    Contributions and findings from other developers are highly encouraged.

    (oohanabi ROM example)
    -------0--1--2--3--4--5--6--7--8--9--A--B--C--D--E--F   0123456789ABCDEF
    1F00  68 63 65 54 2E 45 2E 4C 00 00 FD 0F 00 00 00 00   hceT.E.L........
    1F10  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00   ................
    1F20  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00   ................
    1F30  03 98 0C 15 C5 65 DF 73 79 C7 29 E0 F8 8E EB B6   ....ﾅeﾟsyﾇ)璧狩ｶ
    1F40  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00   ................
    1F50  BB 5C 80 B1 0F F5 70 D8 9A 7C 10 D6 AB CB 1B FE   ｻ\.ｱ.ﾘ嘶.ﾖｫﾋ..
    1F60  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00   ................
    1F70  46 A0 2F 2B 0C 56 44 91 0E B1 78 D7 11 F2 9F 70   F./+.VD..ｱxﾗ.p

    Observed characteristics:

    - A fixed 128-byte region exists at 0x1F00 in all examined ROMs.

    - The first 8 bytes contain the string "hceT.E.L"
      (interpreted as "L.E.Tech" when reversed).

    - The 2-byte value at 0x1F0A-0x1F0B differs between machine families,
      but remains identical between closely related variants
      (e.g. gettermouse vs gettermouse2).

    - Data blocks at:
          0x1F30-0x1F3F
          0x1F50-0x1F5F
          0x1F70-0x1F7F
      vary significantly between ROMs and do not match simple patterns.

    Possible interpretations (unconfirmed):

    - Integrity / checksum data
    - Encrypted configuration or initialization data
    - Interrupt vector or internal control setup
    - Parameters related to internal RNG / random input generation


    NOTE:
    - Similar data structures have also been observed in V2-series
      LE CPUs (e.g. LE2080), suggesting this may be a shared design feature.

    - At present, this region is not explicitly accessed by the current
      emulation core, and its exact role remains unknown.

***************************************************************************/

#include "emu.h"
#include "le1080a.h"

#include <chrono>

#include "z80.inc"

#define LOG_GENERAL (1U << 0)
#define LOG_INT     (1U << 1) // shared with z80.lst generated code
#define LOG_UNDOC   (1U << 2) // shared with z80.lst generated code
#define LOG_SYSREG  (1U << 3)
#define LOG_IRQ     (1U << 4)

#define VERBOSE     (0)

#include "logmacro.h"


DEFINE_DEVICE_TYPE(LE1080A, le1080a_device, "le1080a", "L.E.Tech LE1080A-PA (V1)")

le1080a_device::le1080a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: z80_device(mconfig, LE1080A, tag, owner, clock)
	, device_nvram_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device-level initialization
//-------------------------------------------------

void le1080a_device::device_start()
{
	z80_device::device_start();

	m_irq0_timer = timer_alloc(FUNC(le1080a_device::irq0_tick), this);
	m_irq1_timer = timer_alloc(FUNC(le1080a_device::irq1_tick), this);

	m_irq0_off_timer = timer_alloc(FUNC(le1080a_device::irq0_clear), this);
	m_irq1_off_timer = timer_alloc(FUNC(le1080a_device::irq1_clear), this);
	m_irq2_off_timer = timer_alloc(FUNC(le1080a_device::irq2_clear), this);

	install_internal_ram_and_sysregs();

	// Save only emulation-relevant state here.
	save_item(NAME(m_internal_ram));
	save_item(NAME(m_irq0_vector));
	save_item(NAME(m_irq1_vector));
	save_item(NAME(m_irq2_vector));
	save_item(NAME(m_irq0_rate_hz));
	save_item(NAME(m_irq1_rate_hz));
	save_item(NAME(m_irq0_line_state));
	save_item(NAME(m_irq1_line_state));
	save_item(NAME(m_irq2_line_state));
	save_item(NAME(m_internal_rng_state));
	save_item(NAME(m_internal_rng_counter));
	save_item(NAME(m_sysreg_shadow));

	m_sysreg_shadow.fill(0x00);
}


//-------------------------------------------------
//  device reset and timer reconfiguration
//-------------------------------------------------

void le1080a_device::device_reset()
{
	z80_device::device_reset();

	// Seed the LE1080A internal RNG source with host wall-clock time
	// to prevent deterministic boot behavior where games hit bonus events
	// at identical game counts on every start.
	u32 const time_seed = u32(std::chrono::high_resolution_clock::now().time_since_epoch().count());

	for (int i = 0; i < INTERNAL_RNG_INPUT_COUNT; i++)
		m_internal_rng_state[i] = u8((0x13 + (i * 0x44)) ^ (time_seed >> (i * 8)));

	m_internal_rng_counter = u8(time_seed);

	m_irq0_line_state = CLEAR_LINE;
	m_irq1_line_state = CLEAR_LINE;
	m_irq2_line_state = CLEAR_LINE;

	if (m_irq0_rate_hz)
		m_irq0_timer->adjust(attotime::from_hz(m_irq0_rate_hz), 0, attotime::from_hz(m_irq0_rate_hz));
	else
		m_irq0_timer->adjust(attotime::never);

	if (m_irq1_rate_hz)
		m_irq1_timer->adjust(attotime::from_hz(m_irq1_rate_hz), 0, attotime::from_hz(m_irq1_rate_hz));
	else
		m_irq1_timer->adjust(attotime::never);
}


//-------------------------------------------------
//  execution loop
//-------------------------------------------------

void le1080a_device::execute_run()
{
	#include "cpu/z80/le1080a.hxx"
}


//-------------------------------------------------
//  internal RAM and system register installation
//-------------------------------------------------

void le1080a_device::install_internal_ram_and_sysregs()
{
	address_space &space = this->space(AS_PROGRAM);

	space.install_ram(INTERNAL_RAM_BASE, INTERNAL_RAM_BASE + INTERNAL_RAM_SIZE - 1, &m_internal_ram[0]);

	// Known LE1080A internal register area.
	space.install_readwrite_handler(0xff00, 0xff3f,
		read8sm_delegate(*this,  FUNC(le1080a_device::sysreg_r)),
		write8sm_delegate(*this, FUNC(le1080a_device::sysreg_w)));

	// Remaining FFxxh area is currently unmapped/unknown for LE1080A.
	space.install_readwrite_handler(0xff40, 0xffff,
		read8sm_delegate(*this, FUNC(le1080a_device::unmapped_internal_r)),
		write8sm_delegate(*this, FUNC(le1080a_device::unmapped_internal_w)));
}

void le1080a_device::install_sysreg_handler(u8 start, u8 end, read8sm_delegate read, write8sm_delegate write)
{
	for (u16 i = start; i <= end; i++)
	{
		m_sysreg[i].r = read;
		m_sysreg[i].w = write;
	}
}

u8 le1080a_device::unmapped_internal_r(offs_t offset)
{
	u16 const addr = 0xff40 | (offset & 0x00bf);

	LOGMASKED(LOG_SYSREG, "%s: unmapped LE1080A internal register read %04X PC=%04X\n",
		machine().describe_context(), addr, pc());

	return 0xff;
}

void le1080a_device::unmapped_internal_w(offs_t offset, u8 data)
{
	u16 const addr = 0xff40 | (offset & 0x00bf);

	LOGMASKED(LOG_SYSREG, "%s: unmapped LE1080A internal register write %04X=%02X PC=%04X\n",
		machine().describe_context(), addr, data, pc());
}


//-------------------------------------------------
//  system register accessors
//-------------------------------------------------

u8 le1080a_device::sysreg_r(offs_t offset)
{
	u8 const idx = offset & 0xff;
	if (m_sysreg[idx].r.has_value())
		return (*m_sysreg[idx].r)(idx);

	return m_sysreg_shadow[idx];
}

void le1080a_device::sysreg_w(offs_t offset, u8 data)
{
	u8 const idx = offset & 0xff;

	m_sysreg_shadow[idx] = data;

	LOGMASKED(LOG_SYSREG, "%s: LE1080A SYSREG W FF%02X=%02X PC=%04X\n",
		machine().describe_context(), idx, data, pc());

	if (m_sysreg[idx].w.has_value())
		(*m_sysreg[idx].w)(idx, data);
}


//-------------------------------------------------
//  current V1 system-register observations
//-------------------------------------------------
//
// FF14h:
//   - Cleared to 00h during boot initialization in multiple V1 programs.
//   - Yamasa V1 power-fail handling appears to write 01h after entering the
//     power-fail path and before/while the save sequence completes.
//   - Tentatively suspected to act as an interrupt disable, acknowledge or
//     in-service latch related to IRQ1 or IRQ2. Exact meaning is unconfirmed.
//
// FF18h / FF19h:
//   - Tentatively suspected to be part of an LE1080A-visible external I/O
//     window rather than simple internal control bits.
//   - Current Yamasa V1 observations include FF18h writes used for sound-code
//     output, and FF19h reads used for external switch/input states.
//   - Exact mapping and scope are still unconfirmed.
//
// FF26h / FF3Ah:
//   - Additional V1 addresses observed in Yamasa code paths so far.
//   - FF26h is pulsed 01 -> 00, suggesting a strobe/acknowledge style use.
//   - FF3Ah is written with different values at boot and during IRQ handling,
//     suggesting mode/mask/state control.
//
// RST 08h-30h dispatchers:
//   - Some V1 programs use compact helper trampolines that POP the return
//     address, consume inline parameter bytes, and continue via LD PC,HL
//     rather than a conventional RET.
//   - This behaves more like a compact macro/dispatch mechanism than a normal
//     subroutine and makes static control-flow analysis less direct.
//
// Interrupt vector / timer configuration:
//   - The current core provides driver-configurable interrupt vectors and
//     timer rates as a practical approximation.
//   - However, the original LE1080A method for configuring interrupt vector
//     addresses and interrupt periods remains unconfirmed.
//   - Current V1 program behavior suggests these settings are influenced by
//     internal control state rather than by plain Z80-visible conventions.
//   - More accurate emulation will likely require deeper analysis of the
//     LE1080A interrupt controller and related internal configuration paths.


//-------------------------------------------------
//  interrupt input handling and timer sources
//-------------------------------------------------

bool le1080a_device::irq0_enabled() const
{
	return BIT(m_sysreg_shadow[0x22], 7);
}

bool le1080a_device::irq1_enabled() const
{
	return BIT(m_sysreg_shadow[0x30], 7);
}

bool le1080a_device::irq2_enabled() const
{
	return (m_sysreg_shadow[0x2a] | m_sysreg_shadow[0x2b]) != 0;
}

void le1080a_device::update_irq_attention()
{
	if (m_irq0_line_state != CLEAR_LINE || m_irq1_line_state != CLEAR_LINE || m_irq2_line_state != CLEAR_LINE)
		set_service_attention<SA_LE1080A_IRQ_ON, 1>();
	else
		set_service_attention<SA_LE1080A_IRQ_ON, 0>();
}

void le1080a_device::execute_set_input(int inputnum, int state)
{
	bool update_attention = false;

	switch (inputnum)
	{
		case LE1080A_INPUT_LINE_IRQ0:
			m_irq0_line_state = state;
			update_attention = true;
			break;

		case LE1080A_INPUT_LINE_IRQ1:
			m_irq1_line_state = state;
			update_attention = true;
			break;

		case LE1080A_INPUT_LINE_IRQ2:
			LOGMASKED(LOG_IRQ, "%s: LE1080A manual IRQ2 state=%d FF2Ah=%02X PC=%04X\n",
				machine().describe_context(), state, m_sysreg_shadow[0x2a], pc());
			m_irq2_line_state = state;
			update_attention = true;
			break;

		default:
			z80_device::execute_set_input(inputnum, state);
			return;
	}

	if (update_attention)
		update_irq_attention();
}

TIMER_CALLBACK_MEMBER(le1080a_device::irq0_tick)
{
	if (!irq0_enabled())
		return;

	LOGMASKED(LOG_IRQ, "%s: LE1080A IRQ0 ASSERT vector=%04X PC=%04X\n",
		machine().describe_context(), m_irq0_vector, pc());

	if (m_irq0_line_state == CLEAR_LINE)
		pulse_irq_line(LE1080A_INPUT_LINE_IRQ0);
}

TIMER_CALLBACK_MEMBER(le1080a_device::irq1_tick)
{
	if (!irq1_enabled())
		return;

	LOGMASKED(LOG_IRQ, "%s: LE1080A IRQ1 ASSERT vector=%04X PC=%04X\n",
		machine().describe_context(), m_irq1_vector, pc());

	if (m_irq1_line_state == CLEAR_LINE)
		pulse_irq_line(LE1080A_INPUT_LINE_IRQ1);
}

void le1080a_device::pulse_irq_line(int line)
{
	set_input_line(line, ASSERT_LINE);

	emu_timer *t = nullptr;

	switch (line)
	{
		case LE1080A_INPUT_LINE_IRQ0: t = m_irq0_off_timer;
			break;

		case LE1080A_INPUT_LINE_IRQ1: t = m_irq1_off_timer;
			break;

		case LE1080A_INPUT_LINE_IRQ2: t = m_irq2_off_timer;
			break;
	}

	if (t)
		t->adjust(attotime::from_usec(IRQ_PULSE_WIDTH_USEC));
}

void le1080a_device::trigger_power_fail()
{
	LOGMASKED(LOG_IRQ, "%s: LE1080A POWERFAIL trigger PC=%04X\n",
		machine().describe_context(), pc());

	set_input_line(LE1080A_INPUT_LINE_IRQ2, ASSERT_LINE);
}

TIMER_CALLBACK_MEMBER(le1080a_device::irq0_clear)
{
	set_input_line(LE1080A_INPUT_LINE_IRQ0, CLEAR_LINE);
}

TIMER_CALLBACK_MEMBER(le1080a_device::irq1_clear)
{
	set_input_line(LE1080A_INPUT_LINE_IRQ1, CLEAR_LINE);
}

TIMER_CALLBACK_MEMBER(le1080a_device::irq2_clear)
{
	set_input_line(LE1080A_INPUT_LINE_IRQ2, CLEAR_LINE);
}


//-------------------------------------------------
//  pseudo-random / hardware RNG input source
//-------------------------------------------------
//
// LE1080A appears to provide one or more hardware-side random/entropy
// sources.  Using std::rand()/rand() directly would introduce host-dependent
// behaviour and poor determinism across platforms.
//
// machine().rand() provides a deterministic MAME-managed random source,
// but raw output alone tends to look too uniform for the behaviour games
// appear to expect.  To better approximate a hardware entropy source,
// the value is mixed with internal state, a per-read counter, execution PC,
// CPU cycles, and virtual attotime.
//
// This is an empirical heuristic implementation.  While it avoids trivial
// repeating patterns and runs deterministically, it does not yet fully reflect
// true hardware behaviour.
//
// Additional observations and known discrepancies:
//
// - Program-visible random inputs vary across titles:
//   Some games map them via MMIO while others access them via I/O ports,
//   with varying numbers of ports (typically 2 to 4 lanes).
//
// - Observed gameplay discrepancy:
//   In all tested V1 titles, regular bonus gameplay exhibits anomalies.
//   Most notably during JAC games (regular bonus rounds), misses/blanks occur
//   where real hardware virtually guarantees a hit on every spin (e.g. JAC
//   in-play alignment).  This strongly indicates that the software expects
//   the random input to conform to a specific range, bit-mask, or distribution
//   curve that the current full 8-bit generator violates.
//
// - Suspected hardware configuration mechanisms:
//   The exact internal RNG circuit and its mapping are unconfirmed.
//   It is suspected that internal LE1080A system registers (0xFF00-0xFF3F)
//   and/or external configuration pins may control:
//     * Output value range / bit width per port
//     * Number of active RNG channels
//     * Address mapping to specific MMIO / I/O spaces
//
// - Forcing constant values on suspected RNG ports causes games to lock up
//   or enter deterministic non-functional states, confirming the CPU
//   and software tightly integrate these inputs for outcome generation.
//
// Further hardware analysis or reverse engineering of the LE1080A internal
// logic and register configurations will be required to achieve full accuracy.

u8 le1080a_device::internal_rng_r(offs_t offset)
{
	int const index = offset % INTERNAL_RNG_INPUT_COUNT;
	m_internal_rng_counter++;

	u32 const sub_cycles = u32(machine().time().attoseconds() & 0xffffffff);

	u8 v = m_internal_rng_state[index];
	v ^= u8(machine().rand() & 0xff);
	v ^= u8(sub_cycles);
	v ^= u8(sub_cycles >> 8);
	v ^= u8(m_internal_rng_counter);
	v ^= u8(pc());
	v ^= u8(pc() >> 8);
	v ^= u8(total_cycles());
	v ^= u8(total_cycles() >> 8);

	m_internal_rng_state[index] = v;
	return v;
}


//-------------------------------------------------
//  NVRAM backing for internal RAM
//-------------------------------------------------

void le1080a_device::nvram_default()
{
	std::fill(m_internal_ram.begin(), m_internal_ram.end(), 0x00);
	std::fill(m_sysreg_shadow.begin(), m_sysreg_shadow.end(), 0x00);
}

bool le1080a_device::nvram_read(util::read_stream &file)
{
	std::size_t actual = 0;
	std::error_condition err = file.read_some(&m_internal_ram[0], m_internal_ram.size(), actual);
	if (err || (actual != m_internal_ram.size()))
	{
		LOGMASKED(LOG_GENERAL, "LE1080A NVRAM READ FAIL (internal RAM) err=%d actual=%u expected=%u\n",
			int(err.value()), unsigned(actual), unsigned(m_internal_ram.size()));
		return false;
	}

	std::fill(m_sysreg_shadow.begin(), m_sysreg_shadow.end(), 0x00);
	return true;
}

bool le1080a_device::nvram_write(util::write_stream &file)
{
	std::size_t actual = 0;
	std::error_condition err = file.write_some(&m_internal_ram[0], m_internal_ram.size(), actual);
	if (err || (actual != m_internal_ram.size()))
	{
		LOGMASKED(LOG_GENERAL, "LE1080A NVRAM WRITE FAIL (internal RAM) err=%d actual=%u expected=%u\n",
			int(err.value()), unsigned(actual), unsigned(m_internal_ram.size()));
		return false;
	}

	return true;
}
