// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    CMD SuperCPU v2 + SuperRAM emulation

    The 65816 runs from its own SRAM and holds the 6510 off the bus
    with DMA. The C64 bus is only used for I/O, cartridge ROM, and for
    mirroring writes into C64 RAM so that the VIC can see them.

    Bus timing follows the VICE xscpu64 model: a C64 cycle counter plus
    a 20 MHz phase accumulator. The 65816's own clock is slaved to it,
    and every C64 bus access waits until the VIC and CIAs have run up
    to the cycle the access belongs to.

    C64 IRQ/NMI edges are queued with the C64 cycle they occurred on
    and applied by the 65816 access whose virtual cycle reaches them,
    deferring that access until the scheduler has set the input line.
    WAI makes no accesses, so while the 65816 drives RDY low a timer
    delivers them on C64 time and advances the virtual clock to match.

    TODO:

    - In turbo mode C64 IRQs/NMIs reach the 65816 one C64 cycle later
      than on xscpu64: the VIC runs a whole cycle after the 65816, and
      waiting for it would also delay the 65816's VIC writes by a cycle.

**********************************************************************/

#include "emu.h"
#include "supercpu.h"

#include "supercpu.lh"

#include <algorithm>


//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define W65816_TAG  "w65816"

namespace {

constexpr uint32_t ACCU_CYCLE = 20'000'000;
constexpr uint32_t ACCU_LATE = 18'000'000;
// Units are fractions of a host cycle with ACCU_CYCLE as the denominator.
constexpr uint32_t ACCU_IO = 9'500'000;
constexpr uint32_t ACCU_IO_CIA = 14'800'000;
constexpr uint32_t ACCU_IO_LONG = 15'600'000;
constexpr uint32_t ACCU_SLOW = 15'700'000;
// host IRQ/NMI transitions are seen by an opcode fetch LINE_DELAY host cycles
// later at 1 MHz; in turbo mode LINE_DELAY_FAST later, the first point where
// the VIC has always run, and LINE_PHASE into the cycle when leaving WAI
constexpr uint64_t LINE_DELAY = 2;
constexpr uint32_t LINE_PHASE = 2'000'000;
constexpr int64_t LINE_DELAY_FAST = ACCU_CYCLE + LINE_PHASE;

// mirrored page ranges, indexed by optimization register bits 0, 2, 6 and 7
constexpr uint16_t MIRRORS[16] =
{
	0x80bf, 0x80bf, 0x003f, 0x023f,
	0x407f, 0x407f, 0xc0ff, 0xc0ff,
	0x0407, 0x0407, 0x0000, 0x0000,
	0x00ff, 0x02ff, 0x00ff, 0x02ff
};

// C128 MMU registers
enum : uint8_t
{
	MMU_CR = 0,
	MMU_PCRA,
	MMU_MCR = 5,
	MMU_RCR,
	MMU_P0L,
	MMU_P0H,
	MMU_P1L,
	MMU_P1H
};

constexpr offs_t ADAPTER_PROCESSOR_PORT = 0xd1ff;
constexpr offs_t VIC_CLOCK_CONTROL = 0xd030;
constexpr offs_t MMU_MODE_CONTROL = 0xd505;
constexpr offs_t MMU_LOAD_CONFIG = 0xff00;

constexpr offs_t RCR_BOTTOM[4] = { 0x0400, 0x1000, 0x2000, 0x4000 };
constexpr offs_t RCR_TOP[4] = { 0xfc00, 0xf000, 0xe000, 0xc000 };

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C64_SUPERCPU, c64_supercpu_device, "c64_supercpu", "C64 SuperCPU v2 + SuperRAM")
DEFINE_DEVICE_TYPE(C128_SUPERCPU, c128_supercpu_device, "c128_supercpu", "C128 SuperCPU v2 + SuperRAM")


ROM_START( c64_supercpu )
	ROM_REGION( 0x20000, W65816_TAG, 0 )
	ROM_DEFAULT_BIOS("v204")
	ROM_SYSTEM_BIOS( 0, "v14", "SuperCPU DOS 1.4" )
	ROMX_LOAD( "scpu-dos-1.4.bin", 0x00000, 0x20000, CRC(78008489) SHA1(3422a7735f0f959d990ab39512d8815a5d8eab7a), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v204", "SuperCPU DOS 2.04" )
	ROMX_LOAD( "supercpu_dos_204.bin", 0x00000, 0x20000, CRC(f4151454) SHA1(6aa529a7b1b6de53e8979e407a77b4d5657727f5), ROM_BIOS(1) )
ROM_END


ROM_START( c128_supercpu )
	ROM_REGION( 0x20000, W65816_TAG, 0 )
	ROM_LOAD( "supercpu_dos_204.bin", 0x00000, 0x20000, CRC(f4151454) SHA1(6aa529a7b1b6de53e8979e407a77b4d5657727f5) )

	ROM_REGION( 0x100, "cpld", 0 )
	ROM_LOAD( "cpu64v2 pld v2.02 cpu-6404.bin", 0x000, 0x001, NO_DUMP )
	ROM_LOAD( "64-12a.bin", 0x000, 0x001, NO_DUMP )

	ROM_REGION( 0x100, "gal", 0 )
	ROM_LOAD( "cpu128-u2-a.u2", 0x000, 0x001, NO_DUMP )
	ROM_LOAD( "cpu128-u3-a.u3", 0x000, 0x001, NO_DUMP )
ROM_END


const tiny_rom_entry *c64_supercpu_device::device_rom_region() const
{
	return m_is_c128_model ? ROM_NAME( c128_supercpu ) : ROM_NAME( c64_supercpu );
}


void c64_supercpu_device::c64_supercpu_map(address_map &map)
{
	map(0x000000, 0xffffff).rw(FUNC(c64_supercpu_device::cpu_r), FUNC(c64_supercpu_device::cpu_w));
}

void c64_supercpu_device::c64_supercpu_vectors(address_map &map)
{
	map(0x00, 0x1f).r(FUNC(c64_supercpu_device::vector_r));
}


void c64_supercpu_device::device_add_mconfig(machine_config &config)
{
	W65816(config, m_maincpu, 20_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &c64_supercpu_device::c64_supercpu_map);
	m_maincpu->set_addrmap(w65816_device::AS_VECTORS, &c64_supercpu_device::c64_supercpu_vectors);
	m_maincpu->rdy_cb().set(FUNC(c64_supercpu_device::rdy_w));

	C64_EXPANSION_SLOT(config, m_exp, DERIVED_CLOCK(1, 1), c64_expansion_cards, nullptr);
	m_exp->set_passthrough();
	m_exp->cd_input_callback().set(FUNC(c64_supercpu_device::exp_dma_cd_r));
	m_exp->cd_output_callback().set(FUNC(c64_supercpu_device::exp_dma_cd_w));
	m_exp->dma_callback().set(FUNC(c64_supercpu_device::exp_dma_w));

	config.set_default_layout(layout_supercpu);
}


INPUT_CHANGED_MEMBER( c64_supercpu_device::unit_changed )
{
	m_turbo_led = m_fast && newval;
}

INPUT_CHANGED_MEMBER( c64_supercpu_device::speed_changed )
{
	m_sw_1mhz = !BIT(m_front->read(), 2);
	update_speed();
}

static INPUT_PORTS_START( c64_supercpu )
	PORT_START("FRONT")
	PORT_DIPNAME( 0x01, 0x01, "Unit" ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(c64_supercpu_device::unit_changed), 0)
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "JiffyDOS" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Speed" ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(c64_supercpu_device::speed_changed), 0)
	PORT_DIPSETTING(    0x00, "Normal" )
	PORT_DIPSETTING(    0x04, "Turbo" )

	PORT_START("RESET")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F11) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF_OWNER, FUNC(c64_expansion_slot_device::reset_w))

	PORT_START("CONFIG")
	PORT_CONFNAME( 0x07, 0x04, "SuperRAM (requires reset)" )
	PORT_CONFSETTING(    0x00, DEF_STR( None ) )
	PORT_CONFSETTING(    0x01, "1 MB" )
	PORT_CONFSETTING(    0x02, "4 MB" )
	PORT_CONFSETTING(    0x03, "8 MB" )
	PORT_CONFSETTING(    0x04, "16 MB" )
INPUT_PORTS_END


ioport_constructor c64_supercpu_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( c64_supercpu );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

c64_supercpu_device::c64_supercpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	c64_supercpu_device(mconfig, C64_SUPERCPU, tag, owner, clock, false)
{
}

c64_supercpu_device::c64_supercpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, bool c128) :
	device_t(mconfig, type, tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this),
	m_maincpu(*this, W65816_TAG),
	m_exp(*this, "exp"),
	m_rom(*this, W65816_TAG),
	m_front(*this, "FRONT"),
	m_config(*this, "CONFIG"),
	m_turbo_led(*this, "turbo_led"),
	m_is_c128_model(c128),
	m_sram(*this, "sram", c128 ? 0x40000 : 0x20000, ENDIANNESS_LITTLE),
	m_simm(*this, "simm", 0x100'0000, ENDIANNESS_LITTLE),
	m_dma_timer(nullptr),
	m_sw_1mhz(false),
	m_soft_1mhz(false),
	m_sys_1mhz(false),
	m_hwenable(false),
	m_dosext(false),
	m_ramlink(false),
	m_bootmap(true),
	m_optim(0xc7),
	m_simm_reg(4),
	m_pport(7),
	m_boot(false),
	m_armed(false),
	m_ultimax(false),
	m_host_dma(false),
	m_exp_dma(false),
	m_dma(false),
	m_host_is_c128(false),
	m_2mhz(false),
	m_native_c128_mode(false),
	m_p0h_latch(0),
	m_p1h_latch(0),
	m_simm_mask(0),
	m_simm_page(13),
	m_conf_page(13),
	m_conf_size(0x100'0000),
	m_simm_cell(0),
	m_fast(true),
	m_clk(1),
	m_accu(0),
	m_buffer_finish(0),
	m_buffer_half(0),
	m_buffer_steal(false),
	m_dirty(false),
	m_ba_low(false),
	m_ba_low_start(~uint64_t(0)),
	m_phi2_time(attotime::zero),
	m_in_access(false),
	m_inc_pending(false),
	m_step(ACCESS_START),
	m_wake_timer(nullptr),
	m_line_count(0),
	m_line_state(0),
	m_rdy(true)
{
}

c128_supercpu_device::c128_supercpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	c64_supercpu_device(mconfig, C128_SUPERCPU, tag, owner, clock, true)
{
}


void c64_supercpu_device::device_start()
{
	m_dma_timer = timer_alloc(FUNC(c64_supercpu_device::assert_dma), this);
	m_cycle_timer = timer_alloc(timer_expired_delegate());
	m_wake_timer = timer_alloc(FUNC(c64_supercpu_device::wake), this);

	std::fill(std::begin(m_line_clk), std::end(m_line_clk), 0);
	std::fill(std::begin(m_line_event), std::end(m_line_event), 0);

	save_item(NAME(m_sw_1mhz));
	save_item(NAME(m_soft_1mhz));
	save_item(NAME(m_sys_1mhz));
	save_item(NAME(m_hwenable));
	save_item(NAME(m_dosext));
	save_item(NAME(m_ramlink));
	save_item(NAME(m_bootmap));
	save_item(NAME(m_optim));
	save_item(NAME(m_simm_reg));
	save_item(NAME(m_pport));
	save_item(NAME(m_boot));
	save_item(NAME(m_armed));
	save_item(NAME(m_host_dma));
	save_item(NAME(m_exp_dma));
	save_item(NAME(m_host_is_c128));
	save_item(NAME(m_mmu));
	save_item(NAME(m_p0h_latch));
	save_item(NAME(m_p1h_latch));
	save_item(NAME(m_2mhz));
	save_item(NAME(m_native_c128_mode));
	save_item(NAME(m_simm_mask));
	save_item(NAME(m_simm_page));
	save_item(NAME(m_conf_page));
	save_item(NAME(m_conf_size));
	save_item(NAME(m_simm_cell));
	save_item(NAME(m_fast));
	save_item(NAME(m_clk));
	save_item(NAME(m_accu));
	save_item(NAME(m_buffer_finish));
	save_item(NAME(m_buffer_half));
	save_item(NAME(m_buffer_steal));
	save_item(NAME(m_dirty));
	save_item(NAME(m_ba_low));
	save_item(NAME(m_ba_low_start));
	save_item(NAME(m_phi2_time));
	save_item(NAME(m_in_access));
	save_item(NAME(m_inc_pending));
	save_item(NAME(m_step));
	save_item(NAME(m_line_clk));
	save_item(NAME(m_line_event));
	save_item(NAME(m_line_count));
	save_item(NAME(m_line_state));
	save_item(NAME(m_rdy));
}


void c64_supercpu_device::device_reset()
{
	m_optim = 0xc7;
	m_soft_1mhz = false;
	m_sys_1mhz = false;
	m_hwenable = false;
	m_dosext = false;
	m_ramlink = false;
	m_bootmap = true;
	m_simm_reg = 4;
	m_boot = false;
	m_armed = false;
	m_ultimax = false;
	m_host_is_c128 = false;
	m_2mhz = false;
	m_native_c128_mode = false;
	std::fill(std::begin(m_mmu), std::end(m_mmu), 0);
	m_mmu[MMU_P1L] = 0x01;
	m_p0h_latch = 0;
	m_p1h_latch = 0;
	m_sw_1mhz = !BIT(m_front->read(), 2);
	m_fast = !m_sw_1mhz;
	m_turbo_led = m_fast && BIT(m_front->read(), 0);
	update_simm(true);

	m_clk = attotime_to_clocks(machine().time()) + 2;
	m_accu = 0;
	m_buffer_finish = m_clk;
	m_buffer_half = 0;
	m_buffer_steal = false;
	m_dirty = true;
	m_in_access = false;
	m_inc_pending = false;
	m_step = ACCESS_START;
	m_pport = 7;
	m_simm_cell = 0;
	m_ba_low = false;
	m_ba_low_start = ~uint64_t(0);
	m_phi2_time = machine().time();

	if (BIT(m_front->read(), 0))
	{
		m_dma_timer->adjust(attotime::never);
		host_dma_w(false);

		if (m_is_c128_model)
			m_dma_timer->adjust(attotime::zero);
		else
			m_boot = true;

		// end every timeslice just before a C64 cycle boundary, so that the host chips have
		// executed exactly the cycles before the one the 65816 is accessing
		attotime const period = clocks_to_attotime(1);
		attotime const next = clocks_to_attotime(attotime_to_clocks(machine().time()) + 1) - attotime::from_nsec(20);
		m_cycle_timer->adjust(next - machine().time(), 0, period);
	}
	else
	{
		m_cycle_timer->adjust(attotime::never);
		m_dma_timer->adjust(attotime::never);
		host_dma_w(false);
	}
}


// device_reset_after_children - hold the 65816
//  until it has the bus

void c64_supercpu_device::device_reset_after_children()
{
	m_maincpu->suspend(SUSPEND_REASON_DISABLE, true);
}


TIMER_CALLBACK_MEMBER( c64_supercpu_device::assert_dma )
{
	host_dma_w(true);

	if (param == 2)
	{
		// the C128 VIC has a clock register at $D030, which reads back all ones on a C64
		m_host_is_c128 = !BIT(io_bus_r(VIC_CLOCK_CONTROL), 0);
		start_maincpu();
		return;
	}

	if (!param && !probe_c128_mmu())
	{
		host_dma_w(false);
		m_boot = true;
		return;
	}

	// the C128 boots in Z80 mode, where $D000-$DFFF is RAM to the bus master
	if (!BIT(m_slot->dma_cd_r(MMU_MODE_CONTROL), 0))
	{
		m_dma_timer->adjust(clocks_to_attotime(64), 1);
		return;
	}

	if (probe_adapter())
	{
		m_host_is_c128 = true;
		m_native_c128_mode = true;
		start_maincpu();
	}
	else
	{
		// without the MMU SuperAdapter the cartridge's EXROM makes the KERNAL switch to C64 mode
		host_dma_w(false);
		m_boot = true;
	}
}


// host_dma_w - the cartridge and the pass-through
//  port share the open collector /DMA line

void c64_supercpu_device::host_dma_w(bool state)
{
	m_host_dma = state;
	m_slot->dma_w(m_host_dma || m_exp_dma);
}


void c64_supercpu_device::exp_dma_w(int state)
{
	if (m_exp_dma == bool(state))
		return;

	m_exp_dma = state;
	host_dma_w(m_host_dma);

	if (m_exp_dma)
	{
		m_maincpu->suspend(SUSPEND_REASON_HALT, true);
	}
	else
	{
		if (!m_fast && !m_2mhz && m_inc_pending)
		{
			maincpu_inc();
			m_inc_pending = false;
		}

		m_clk = std::max(m_clk, attotime_to_clocks(machine().time()) + 2);
		m_dirty = true;
		m_maincpu->resume(SUSPEND_REASON_HALT);
	}
}


// exp_dma_cd_r/w - pass-through bus master cycles
//  see the 65816 memory map while it owns the bus

uint8_t c64_supercpu_device::exp_dma_cd_r(offs_t offset)
{
	if (!bus_master())
		return m_slot->dma_cd_r(offset);

	m_dma = true;
	uint8_t const data = bank0_r(offset);
	m_dma = false;

	return data;
}

void c64_supercpu_device::exp_dma_cd_w(offs_t offset, uint8_t data)
{
	if (!bus_master())
	{
		m_slot->dma_cd_w(offset, data);
		return;
	}

	m_dma = true;
	bank0_w(offset, data);
	m_dma = false;
}


// probe_c128_mmu - the C128 MMU registers at $FF00 read
//  back their reset values

bool c64_supercpu_device::probe_c128_mmu()
{
	static constexpr uint8_t MMU_RESET[] = { 0x80, 0x00, 0x00, 0x00, 0x00 };

	for (unsigned i = 0; i < std::size(MMU_RESET); i++)
		if (m_slot->dma_cd_r(MMU_LOAD_CONFIG + i) != MMU_RESET[i])
			return false;

	return true;
}


// start_maincpu - start the 65816 on a bus
//  the host CPU no longer drives

void c64_supercpu_device::start_maincpu()
{
	m_clk = attotime_to_clocks(machine().time()) + 2;
	m_buffer_finish = m_clk;
	m_maincpu->resume(SUSPEND_REASON_DISABLE);
}


uint8_t c64_supercpu_device::c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (m_boot && !roml)
	{
		// the 6510 runs the cartridge entry, which sets up its processor port, and is held at the BRK
		if (m_armed && (offset == 0x8019) && !machine().side_effects_disabled())
		{
			m_boot = false;
			m_armed = false;
			m_dma_timer->adjust(attotime::zero, 2);
		}

		return m_rom[0x8000 | (offset & 0x1fff)];
	}

	return m_exp->cd_r(offset, data, sphi2, ba, roml, romh, io1, io2);
}


void c64_supercpu_device::c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (m_boot && (offset == 0xd07e))
		m_armed = true;

	m_exp->cd_w(offset, data, sphi2, ba, roml, romh, io1, io2);
}


int c64_supercpu_device::c64_game_r(offs_t offset, int sphi2, int ba, int rw)
{
	if (m_ultimax)
		return 0;

	return m_exp->game_r(offset, sphi2, ba, rw, m_slot->loram(), m_slot->hiram());
}


int c64_supercpu_device::c64_exrom_r(offs_t offset, int sphi2, int ba, int rw)
{
	if (m_ultimax)
		return 1;

	if (m_boot)
		return 0;

	return m_exp->exrom_r(offset, sphi2, ba, rw, m_slot->loram(), m_slot->hiram());
}


void c64_supercpu_device::c64_ba_w(int state)
{
	m_exp->ba_w(state);

	m_ba_low = !state;

	if (m_ba_low)
		m_ba_low_start = attotime_to_clocks(machine().time() + clocks_to_attotime(1) / 2) + 1 + 3;
	else
		m_ba_low_start = ~uint64_t(0);
}


void c64_supercpu_device::c64_phi2_w(int state)
{
	m_exp->phi2_w(state);

	if (state)
		m_phi2_time = machine().time() + clocks_to_attotime(1);
}


//**************************************************************************
//  CONFIGURATION
//**************************************************************************

void c64_supercpu_device::update_simm(bool reset)
{
	struct layout { uint32_t size; int32_t page; };
	static constexpr layout INSTALLED[] = {
		{ 0, 13 }, { 0x10'0000, 11 }, { 0x40'0000, 12 }, { 0x80'0000, 12 },
		{ 0x100'0000, 13 }, { 0x100'0000, 13 }, { 0x100'0000, 13 }, { 0x100'0000, 13 }
	};
	static constexpr layout CONFIGURED[] = {
		{ 0x10'0000, 11 }, { 0x40'0000, 12 }, { 0x80'0000, 12 }, { 0x100'0000, 12 },
		{ 0x100'0000, 13 }, { 0x100'0000, 13 }, { 0x100'0000, 13 }, { 0x100'0000, 13 }
	};

	if (reset)
	{
		layout const &installed = INSTALLED[m_config->read() & 7];
		m_simm_mask = installed.size ? installed.size - 1 : 0;
		m_simm_page = installed.page;
	}
	layout const &configured = CONFIGURED[m_simm_reg & 7];
	m_conf_page = configured.page;
	m_conf_size = configured.size;
}


void c64_supercpu_device::set_fast(bool fast)
{
	if (m_fast != fast)
	{
		if (!fast)
		{
			maincpu_inc();
			m_accu = ACCU_SLOW;
		}

		m_fast = fast;
		m_turbo_led = fast && BIT(m_front->read(), 0);
	}
}


void c64_supercpu_device::set_2mhz(bool state)
{
	if (m_2mhz != state)
	{
		if (!state && !m_fast)
		{
			m_accu = ACCU_SLOW;
			m_dirty = true;
		}

		m_2mhz = state;
	}
}


uint8_t c64_supercpu_device::config(offs_t offset)
{
	int const loram = BIT(m_pport, 0);
	int const hiram = BIT(m_pport, 1);
	int const exrom = !m_exp->exrom_r(offset, 1, 1, 1, loram, hiram);
	int const game = !m_exp->game_r(offset, 1, 1, 1, loram, hiram);

	return (m_pport & 7) | (exrom << 3) | (game << 4) | (m_hwenable << 5) | (m_dosext << 6) | (m_bootmap << 7);
}


c64_supercpu_device::memory_region c64_supercpu_device::decode(offs_t offset, uint8_t cfg)
{
	int const p = cfg & 7;
	bool const loram = BIT(cfg, 0);
	bool const hiram = BIT(cfg, 1);
	bool const charen = BIT(cfg, 2);
	bool const exrom = BIT(cfg, 3);
	bool const game = BIT(cfg, 4);
	bool const hw = BIT(cfg, 5);
	bool const dos = BIT(cfg, 6);
	bool const boot = BIT(cfg, 7);
	bool const internal = hw && dos && boot;
	bool const ultimax = game && !exrom;

	switch (offset >> 12)
	{
	case 0x0:
		return internal ? C64_RAM : SRAM_BANK0;

	case 0x1: case 0x2: case 0x3: case 0x4: case 0x5:
		if (internal)
			return ultimax ? ULTIMAX : C64_RAM;
		return dos ? SRAM_BANK1 : SRAM_BANK0;

	case 0x6: case 0x7:
		if (internal)
			return ultimax ? ULTIMAX : C64_RAM;
		return SRAM_BANK0;

	case 0x8: case 0x9:
		if (internal)
			return ultimax ? ULTIMAX : exrom ? CART_ROML : C64_RAM;
		if (boot)
			return EPROM;
		if (dos)
			return SRAM_BANK1;
		return (exrom && loram && hiram) ? CART_ROML : SRAM_BANK0;

	case 0xa: case 0xb:
		if (internal)
			return ultimax ? ULTIMAX : (exrom && game) ? CART_ROMH : EPROM;
		if (boot)
			return EPROM;
		if (exrom && game)
			return hiram ? CART_ROMH : SRAM_BANK0;
		return (loram && hiram) ? SRAM_BANK1 : SRAM_BANK0;

	case 0xc:
		if (internal)
			return ultimax ? ULTIMAX : C64_RAM;
		return boot ? EPROM : SRAM_BANK0;

	case 0xd:
	{
		bool const colour = (offset & 0xfc00) == 0xd800;

		if (internal)
			return (p < 5) ? CHAR_ROM : colour ? COLOUR_RAM_INTERNAL : IO;
		if (!loram && !hiram)
			return boot ? EPROM : SRAM_BANK0;
		if (charen)
			return colour ? COLOUR_RAM : IO;
		if (exrom && game && !hiram)
			return boot ? EPROM : SRAM_BANK0;
		return CHAR_ROM;
	}

	default:
		if (internal)
			return ultimax ? ULTIMAX : EPROM;
		if (boot)
			return EPROM;
		if (hiram)
			return hw ? KERNAL_SHADOW_HW : KERNAL_SHADOW;
		return SRAM_BANK0;
	}
}


unsigned c64_supercpu_device::mirror_mode() const
{
	return BIT(m_optim, 0) | (BIT(m_optim, 2) << 1) | (BIT(m_optim, 6) << 2) | (BIT(m_optim, 7) << 3);
}

bool c64_supercpu_device::mirrored(offs_t offset)
{
	int const mirror = mirror_mode();
	uint16_t const range = MIRRORS[mirror];
	int const page = offset >> 8;

	return range && (page >= (range >> 8)) && (page <= (range & 0xff));
}


// probe_adapter - look for the MMU SuperAdapter,
//  which latches the processor port at $D1FF

bool c64_supercpu_device::probe_adapter()
{
	for (uint8_t data : { 0x05, 0x02, 0x07 })
	{
		m_slot->dma_cd_w(ADAPTER_PROCESSOR_PORT, data);

		if ((m_slot->dma_cd_r(ADAPTER_PROCESSOR_PORT) ^ data) & 7)
			return false;
	}

	return true;
}


// mmu_w - track the C128 MMU registers, which
//  only the 65816 writes while it has the bus

void c64_supercpu_device::mmu_w(offs_t offset, uint8_t data)
{
	if (offset >= MMU_LOAD_CONFIG)
	{
		if (offset == MMU_LOAD_CONFIG)
			m_mmu[MMU_CR] = data & 0x7f;
		else
			m_mmu[MMU_CR] = m_mmu[MMU_PCRA + (offset & 7) - 1];
		return;
	}

	switch (offset & 0x0f)
	{
	case MMU_CR: case MMU_PCRA: case MMU_PCRA + 1: case MMU_PCRA + 2: case MMU_PCRA + 3:
		m_mmu[offset & 0x0f] = data & 0x7f;
		break;

	case MMU_MCR:
		m_mmu[MMU_MCR] = data;
		break;

	case MMU_RCR:
		m_mmu[MMU_RCR] = data & 0x4f;
		break;

	case MMU_P0L: case MMU_P1L:
		m_mmu[offset & 0x0f] = data;
		m_mmu[(offset & 0x0f) + 1] = ((offset & 0x0f) == MMU_P0L) ? m_p0h_latch : m_p1h_latch;
		break;

	case MMU_P0H:
		m_p0h_latch = data & 0x01;
		break;

	case MMU_P1H:
		m_p1h_latch = data & 0x01;
		break;
	}
}


// decode_c128 - bank 0 address decoding in C128
//  mode, following the MMU configuration register

c64_supercpu_device::memory_region c64_supercpu_device::decode_c128(offs_t offset)
{
	uint8_t const cr = m_mmu[MMU_CR];

	if ((offset >= MMU_LOAD_CONFIG) && (offset < 0xff05) && !m_dma)
		return MMU_REGISTERS;

	if ((offset == 0x0001) && !m_dma)
		return PROCESSOR_PORT;

	if (((offset & 0xf000) == 0xd000) && !BIT(cr, 0))
		return IO;

	if (m_bootmap && (offset >= 0x8000))
		return EPROM;

	mmu_selection const memory = mmu_memory(offset);
	if (memory == mmu_selection::SYSTEM_ROM)
		return ((offset & 0xf000) == 0xd000) ? C128_BUS : C128_ROM_SHADOW;

	if (memory == mmu_selection::RAM)
		return C128_RAM;

	return C128_BUS;
}


c64_supercpu_device::mmu_selection c64_supercpu_device::mmu_memory(offs_t offset) const
{
	uint8_t const cr = m_mmu[MMU_CR];
	if (offset < 0x4000)
		return mmu_selection::RAM;
	if (offset < 0x8000)
		return BIT(cr, 1) ? mmu_selection::RAM : mmu_selection::SYSTEM_ROM;
	return mmu_selection((cr >> ((offset < 0xc000) ? 2 : 4)) & 3);
}


// bank1 - SRAM address of bank $01, where the
//  C128 system ROM shadow replaces the bank
//  wherever the MMU selects system ROM

offs_t c64_supercpu_device::bank1(offs_t offset)
{
	return (m_native_c128_mode && (mmu_memory(offset) == mmu_selection::SYSTEM_ROM)) ? rom_shadow(offset, !m_hwenable) : (0x10000 | offset);
}


// rom_shadow - SRAM address of the C128 system
//  ROM shadow; the KERNAL has a second copy at
//  $2000 in bank 1, which banks $00 and $01 swap
//  between with the hardware register enable

offs_t c64_supercpu_device::rom_shadow(offs_t offset, bool swap)
{
	return (swap && (offset >= 0xe000)) ? (0x10000 | (offset - 0xc000)) : (0x30000 | offset);
}


// ram_address - SRAM address of C128 RAM, with
//  the MMU bank, page 0/1 relocation and common RAM

offs_t c64_supercpu_device::ram_address(offs_t offset)
{
	uint8_t const rcr = m_mmu[MMU_RCR];
	int const page = offset >> 8;
	int bank = BIT(m_mmu[MMU_CR], 6);
	int ta = page;

	if (page == 0)
	{
		ta = m_mmu[MMU_P0L];
		bank = BIT(m_mmu[MMU_P0H], 0);
	}
	else if (page == 1)
	{
		ta = m_mmu[MMU_P1L];
		bank = BIT(m_mmu[MMU_P1H], 0);
	}
	else if (mmu_memory(offset) == mmu_selection::RAM)
	{
		if (page == m_mmu[MMU_P0L])
			ta = 0;
		else if (page == m_mmu[MMU_P1L])
			ta = 1;
	}

	if ((BIT(rcr, 2) && (offset < RCR_BOTTOM[rcr & 3])) || (BIT(rcr, 3) && (offset >= RCR_TOP[rcr & 3])))
		bank = 0;

	return (bank ? 0x20000 : 0) | (ta << 8) | (offset & 0xff);
}


// mirrored_c128 - is a bank 0 write copied to
//  C128 RAM

bool c64_supercpu_device::mirrored_c128(offs_t offset, offs_t address)
{
	int const mirror = mirror_mode();

	switch (mirror)
	{
	case 8: case 9:
		if (BIT(address, 17))
			return false;
		break;

	case 12: case 13:
		break;

	default:
		if (BIT(address, 17) != BIT(m_optim, 1))
			return false;
		break;
	}

	return mirrored(offset);
}


uint32_t c64_supercpu_device::simm_address(offs_t offset)
{
	if (m_simm_page != m_conf_page)
		offset = ((offset >> m_conf_page) << m_simm_page) | (offset & util::make_bitmask<uint32_t>(m_simm_page));

	return offset;
}


uint8_t c64_supercpu_device::hardware_r(offs_t offset)
{
	uint8_t data = 0;

	switch (offset)
	{
	case 0xd0b0:
		data = m_native_c128_mode ? 0x00 : 0x40;
		break;

	case 0xd0b2:
		data = (m_hwenable << 7) | (m_sys_1mhz << 6);
		break;

	case 0xd0b3:
	case 0xd0b4:
		data = m_optim & 0xc0;
		break;

	case 0xd0b5:
		data = (BIT(m_front->read(), 1) << 7) | (m_sw_1mhz << 6);
		break;

	case 0xd0b6:
		data = emulation_mode() << 7;
		break;

	case 0xd0b8:
	case 0xd0b9:
		data = (m_soft_1mhz << 7) | (slow_mode() << 6);
		break;

	case 0xd0bc:
	case 0xd0bd:
	case 0xd0be:
	case 0xd0bf:
		data = (m_dosext << 7) | (m_ramlink << 6);
		break;
	}

	return data | (m_optim & 7);
}


void c64_supercpu_device::hardware_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0xd072:
		m_sys_1mhz = true;
		update_speed();
		break;

	case 0xd073:
		m_sys_1mhz = false;
		update_speed();
		break;

	case 0xd074:
	case 0xd075:
	case 0xd076:
	case 0xd077:
		if (m_hwenable)
			m_optim = (offset & 3) << 6;
		break;

	case 0xd078:
		if (m_hwenable)
		{
			m_simm_reg = data;
			update_simm();
		}
		break;

	case 0xd07a:
		m_soft_1mhz = true;
		update_speed();
		break;

	case 0xd079:
	case 0xd07b:
		m_soft_1mhz = false;
		update_speed();
		break;

	case 0xd07e:
		m_hwenable = true;
		update_speed();
		break;

	case 0xd07d:
	case 0xd07f:
		m_hwenable = false;
		update_speed();
		break;

	case 0xd0b2:
		if (m_hwenable)
		{
			m_sys_1mhz = BIT(data, 6);
			if (!BIT(data, 7))
				m_hwenable = false;
			update_speed();
		}
		break;

	case 0xd0b3:
		if (m_hwenable)
			m_optim = (m_optim & 0x38) | (data & 0xc7);
		break;

	case 0xd0b4:
		if (m_hwenable)
			m_optim = (m_optim & 0x3f) | (data & 0xc0);
		break;

	case 0xd0b6:
		if (m_hwenable)
			m_bootmap = false;
		break;

	case 0xd0b7:
		if (m_hwenable)
			m_bootmap = true;
		break;

	case 0xd0b8:
		if (m_hwenable)
		{
			m_soft_1mhz = BIT(data, 7);
			update_speed();
		}
		break;

	case 0xd0bc:
		if (m_hwenable)
			m_dosext = BIT(data, 7);
		break;

	case 0xd0be:
		if (m_hwenable)
			m_dosext = true;
		break;

	case 0xd0bd:
	case 0xd0bf:
		m_dosext = false;
		break;
	}
}


//**************************************************************************
//  BUS TIMING
//**************************************************************************

// clk_time - machine time of a C64 cycle and
//  20 MHz phase

attotime c64_supercpu_device::clk_time(uint64_t clk, uint32_t accu)
{
	attotime const period = clocks_to_attotime(1);

	return clocks_to_attotime(clk - 1) + attotime(0, attoseconds_t(double(period.attoseconds()) * accu / ACCU_CYCLE));
}


void c64_supercpu_device::maincpu_inc()
{
	m_clk++;
	m_dirty = true;
}


void c64_supercpu_device::advance_phase(uint32_t delta, bool wrap_at_boundary)
{
	m_accu += delta;
	if ((m_accu > ACCU_CYCLE) || (wrap_at_boundary && (m_accu == ACCU_CYCLE)))
	{
		m_accu -= ACCU_CYCLE;
		m_clk++;
	}
}

void c64_supercpu_device::clock_inc()
{
	if (m_fast)
		advance_phase(clock());
	else if (m_2mhz)
	{
		// Preserve the C128 half-cycle endpoint; turbo uses VICE's strict wrap.
		advance_phase(ACCU_CYCLE / 2, true);
		m_dirty = true;
	}
	else
		maincpu_inc();
}


// host_line_w - queue a host IRQ/NMI transition
//  for the host cycle it happened on

void c64_supercpu_device::host_line_w(int line, int state)
{
	if (m_line_count == LINE_QUEUE)
	{
		push_line(m_line_event[0]);
		std::copy(m_line_clk + 1, m_line_clk + LINE_QUEUE, m_line_clk);
		std::copy(m_line_event + 1, m_line_event + LINE_QUEUE, m_line_event);
		m_line_count--;
	}

	m_line_clk[m_line_count] = attotime_to_clocks(machine().time() + clocks_to_attotime(1) / 2);
	m_line_event[m_line_count++] = (line << 1) | (state ? 1 : 0);

	if (!m_rdy && (m_line_count == 1))
		arm_wake();
}


bool c64_supercpu_device::push_line(uint8_t event)
{
	int const line = BIT(event, 1);

	if (BIT(m_line_state, line) == BIT(event, 0))
		return false;

	m_line_state ^= 1 << line;
	m_maincpu->set_input_line(line ? w65816_device::NMI_LINE : w65816_device::IRQ_LINE, BIT(event, 0));
	return true;
}


bool c64_supercpu_device::line_visible(uint64_t clk) const
{
	if (m_fast)
		return (int64_t(m_clk - clk) * ACCU_CYCLE + m_accu) >= LINE_DELAY_FAST;
	else
		return int64_t(m_clk - clk) >= int64_t(LINE_DELAY);
}


// pop_lines - drive the 65816 with every transition
//  that its current cycle has reached

bool c64_supercpu_device::pop_lines(bool waiting)
{
	unsigned n = 0;
	bool pushed = false;

	while ((n < m_line_count) && (waiting ? (m_line_clk[n] <= m_clk) : line_visible(m_line_clk[n])))
		pushed |= push_line(m_line_event[n++]);

	if (n)
	{
		std::copy(m_line_clk + n, m_line_clk + m_line_count, m_line_clk);
		std::copy(m_line_event + n, m_line_event + m_line_count, m_line_event);
		m_line_count -= n;

		if (!m_rdy)
			arm_wake();
	}

	return pushed;
}


// deliver_lines - stall the access until the scheduler
//  has applied the new line levels to the 65816

bool c64_supercpu_device::deliver_lines()
{
	if (!pop_lines(false))
		return true;

	m_maincpu->defer_access();
	return false;
}


void c64_supercpu_device::arm_wake()
{
	if (!m_line_count)
	{
		m_wake_timer->adjust(attotime::never);
		return;
	}

	attotime const due = clocks_to_attotime(m_line_clk[0] + (m_fast ? 0 : LINE_DELAY) - 1);
	attotime const now = machine().time();
	m_wake_timer->adjust((due > now) ? (due - now) : attotime::zero);
}


void c64_supercpu_device::rdy_w(int state)
{
	m_rdy = state;

	if (m_rdy)
		m_wake_timer->adjust(attotime::never);
	else
		arm_wake();
}


// wake - a 65816 holding RDY low in WAI makes no bus cycles, so
//  transitions reach it on host time, and its clock resumes there

TIMER_CALLBACK_MEMBER(c64_supercpu_device::wake)
{
	if (!m_line_count || m_rdy)
		return;

	if (m_maincpu->suspended(SUSPEND_ANY_REASON))
	{
		m_wake_timer->adjust(clocks_to_attotime(1));
		return;
	}

	uint64_t const clk = m_line_clk[0] + (m_fast ? 0 : LINE_DELAY);
	uint32_t const accu = m_fast ? LINE_PHASE : ACCU_SLOW;

	if ((m_clk < clk) || ((m_clk == clk) && (m_accu < accu)))
	{
		m_clk = clk;
		m_accu = accu;
		m_dirty = true;
	}

	pop_lines(true);
}


bool c64_supercpu_device::begin_access()
{
	if (!m_in_access)
	{
		if (m_inc_pending)
		{
			clock_inc();
			m_inc_pending = false;
		}

		m_in_access = true;
		m_step = ACCESS_START;
	}

	if (m_dirty && !catch_up())
		return false;

	return !m_line_count || deliver_lines();
}


// catch_up - stall the 65816 until its clock has
//  reached the modelled bus time

bool c64_supercpu_device::catch_up()
{
	attotime const target = clk_time(m_clk + 1, m_accu) - m_maincpu->cycles_to_attotime(2);
	attotime const now = m_maincpu->local_time();

	if (target > now)
	{
		int const cycles = (target - now).as_attoseconds() / m_maincpu->cycles_to_attotime(1).as_attoseconds();

		if (cycles > 0)
		{
			m_maincpu->adjust_icount(-cycles);

			if (m_maincpu->cycles_remaining() <= 0)
			{
				m_maincpu->defer_access();
				return false;
			}
		}
	}

	if (m_fast)
		m_dirty = false;

	return true;
}


// sync - wait until the C64 chips have executed
//  the current cycle

bool c64_supercpu_device::sync()
{
	attotime const target = clocks_to_attotime(m_clk) - clocks_to_attotime(1) / 2;

	if (m_phi2_time < target)
	{
		m_maincpu->defer_access();
		return false;
	}

	return true;
}


bool c64_supercpu_device::steal()
{
	for (;;)
	{
		if (!catch_up() || !sync())
			return false;

		if (!m_ba_low)
			return true;

		maincpu_inc();
	}
}


// wait_buffer - wait for the write buffer to
//  finish its C64 bus cycle

bool c64_supercpu_device::wait_buffer()
{
	if (!m_buffer_steal && ((m_buffer_finish > m_clk) || ((m_buffer_finish == m_clk) && (m_buffer_half > m_accu))))
	{
		m_accu = m_buffer_half;

		while (m_clk < m_buffer_finish)
		{
			if (!catch_up() || !sync())
				return false;

			if (m_clk >= m_ba_low_start)
			{
				m_buffer_steal = true;
				break;
			}

			maincpu_inc();
		}
	}

	if (m_buffer_steal)
	{
		if (!steal())
			return false;

		m_buffer_steal = false;
		maincpu_inc();
		return wait_buffer();
	}

	return true;
}


bool c64_supercpu_device::stretch_read_io()
{
	if (m_dma)
		return true;

	enum { WAIT_BUS, WAIT_BUFFER, SET_PHASE, SYNCHRONIZE };

	if (m_step == WAIT_BUS)
	{
		if (!steal())
			return false;

		m_step = WAIT_BUFFER;
	}

	if (m_step == WAIT_BUFFER)
	{
		if (m_fast && !wait_buffer())
			return false;

		m_step = SET_PHASE;
	}

	if (m_step == SET_PHASE)
	{
		if (m_fast)
		{
			if (m_accu >= ACCU_LATE)
				maincpu_inc();

			maincpu_inc();
			m_accu = ACCU_IO;
		}

		m_step = SYNCHRONIZE;
	}

	return catch_up() && sync();
}


bool c64_supercpu_device::stretch_write_io_start()
{
	if (m_dma)
		return true;

	enum { WAIT_BUFFER, SET_PHASE, WAIT_BUS, SYNCHRONIZE };

	if (m_step == WAIT_BUFFER)
	{
		if (m_fast && !wait_buffer())
			return false;

		m_step = SET_PHASE;
	}

	if (m_step == SET_PHASE)
	{
		if (m_fast && (m_accu >= ACCU_LATE))
			maincpu_inc();

		m_step = WAIT_BUS;
	}

	if (m_step == WAIT_BUS)
	{
		if (!catch_up() || !sync())
			return false;

		if ((m_clk >= m_ba_low_start) && !steal())
			return false;

		m_step = SYNCHRONIZE;
	}

	return catch_up() && sync();
}


bool c64_supercpu_device::stretch_write_io_start_cia()
{
	if (m_dma)
		return true;

	enum { WAIT_BUFFER, SET_PHASE, WAIT_BUS, SYNCHRONIZE };

	if (m_step == WAIT_BUFFER)
	{
		if (m_fast && !wait_buffer())
			return false;

		m_step = SET_PHASE;
	}

	if (m_step == SET_PHASE)
	{
		if (m_fast)
		{
			if (m_accu >= ACCU_LATE)
				maincpu_inc();

			maincpu_inc();
		}

		m_step = WAIT_BUS;
	}

	if (m_step == WAIT_BUS)
	{
		if (m_fast && !steal())
			return false;

		m_step = SYNCHRONIZE;
	}

	return catch_up() && sync();
}


void c64_supercpu_device::stretch_write_io(uint32_t accu)
{
	if (m_fast && !m_dma)
	{
		m_accu = accu;
		maincpu_inc();
	}
}


bool c64_supercpu_device::stretch_write()
{
	if (m_dma)
		return true;

	enum { WAIT_BUFFER, START_BUFFER, SYNCHRONIZE };

	if (m_step == WAIT_BUFFER)
	{
		if (m_fast && !wait_buffer())
			return false;

		m_step = START_BUFFER;
	}

	if (m_step == START_BUFFER)
	{
		if (m_fast)
		{
			m_buffer_finish = m_clk + 1;
			if (m_accu >= ACCU_LATE)
				m_buffer_finish++;
			m_buffer_half = ACCU_IO;
		}

		m_step = SYNCHRONIZE;
	}

	return catch_up() && sync();
}


// stretch_io_2mhz - move a 2 MHz I/O access into
//  the second half of the C128 bus cycle

void c64_supercpu_device::stretch_io_2mhz()
{
	if (!m_dma && !m_fast && m_2mhz && (m_accu < ACCU_SLOW))
	{
		m_accu = ACCU_SLOW;
		m_dirty = true;
	}
}


// stretch_cycles - add 65816 wait states, in
//  half cycles

void c64_supercpu_device::stretch_cycles(int halves)
{
	advance_phase(clock() * halves / 2);
	m_dirty = true;
}


void c64_supercpu_device::stretch_simm(offs_t offset, bool write)
{
	uint32_t const row_mask = ~util::make_bitmask<uint32_t>(m_conf_page);
	int halves = 0;

	if (write)
	{
		halves = ((m_simm_cell ^ offset) & row_mask) ? 4 : 2;
	}
	else
	{
		if (!((m_simm_cell ^ offset) & ~3U))
			return;
		else if ((m_simm_cell ^ offset) & row_mask)
			halves = 5;
		else if (!(((m_simm_cell + 4) ^ offset) & ~3U))
		{
			m_simm_cell = offset;
			return;
		}
		else
			halves = 2;
	}

	m_simm_cell = offset;
	stretch_cycles(halves);
}


//**************************************************************************
//  MEMORY ACCESS
//**************************************************************************

// io_bus_r - C64 bus read in Ultimax mode, which
//  maps I/O whatever the 6510 processor port holds

uint8_t c64_supercpu_device::io_bus_r(offs_t offset)
{
	m_ultimax = !m_native_c128_mode;
	uint8_t const data = bus_r(offset);
	m_ultimax = false;

	return data;
}


void c64_supercpu_device::io_bus_w(offs_t offset, uint8_t data)
{
	m_ultimax = !m_native_c128_mode;
	bus_w(offset, data);
	m_ultimax = false;
}


uint8_t c64_supercpu_device::io_r(offs_t offset)
{
	if (((offset & 0xfff0) == 0xd0b0) || ((offset & 0xfe00) == 0xd200))
	{
		if (!check_ba_read())
			return 0;

		return ((offset & 0xfff0) == 0xd0b0) ? hardware_r(offset) : m_sram[0x10000 | offset];
	}

	stretch_io_2mhz();

	if (!stretch_read_io())
		return 0;

	return io_bus_r(offset);
}


void c64_supercpu_device::io_w(offs_t offset, uint8_t data)
{
	bool const hardware = ((offset >= 0xd071) && (offset < 0xd080)) || ((offset & 0xfff0) == 0xd0b0);
	unsigned const page = (offset >> 8) & 0x0f;

	if (!hardware && ((offset & 0xfe00) != 0xd200))
		stretch_io_2mhz();

	switch (page)
	{
	case 0x2: case 0x3:
		if (!stretch_write())
			return;
		if (m_hwenable || (offset == 0xd27e))
			m_sram[0x10000 | offset] = data;
		return;

	case 0x6: case 0x8: case 0x9: case 0xa: case 0xb:
		if (!stretch_write())
			return;
		io_bus_w(offset, data);
		return;

	case 0xc: case 0xd:
		if (!stretch_write_io_start_cia())
			return;
		m_sram[0x10000 | offset] = data;
		io_bus_w(offset, data);
		stretch_write_io(ACCU_IO_CIA);
		return;
	}

	if (!stretch_write_io_start())
		return;

	if ((page == 0) || (page == 1) || (page == 4) || (page == 5))
		m_sram[0x10000 | offset] = data;

	if (hardware)
	{
		bool const fast = m_fast;
		hardware_w(offset, data);
		if (!fast && m_fast)
			return;
	}
	else
	{
		bool const mmu = m_native_c128_mode && (offset >= 0xd500) && (offset < 0xd50c);

		if (mmu && m_dma)
			return;
		if (mmu)
			mmu_w(offset, data);

		io_bus_w(offset, data);

		if (m_native_c128_mode && (offset == MMU_MODE_CONTROL) && BIT(data, 6))
			m_native_c128_mode = false;
		if (m_host_is_c128 && ((offset & 0xfe3f) == VIC_CLOCK_CONTROL))
			set_2mhz(BIT(data, 0));
		if (offset == 0xdf7e)
			m_ramlink = true;
		else if (offset == 0xdf7f)
			m_ramlink = false;
	}

	stretch_write_io(((offset == 0xdf01) || (offset == 0xdf21)) ? ACCU_IO_LONG : ACCU_IO);
}


uint8_t c64_supercpu_device::bank0_r(offs_t offset)
{
	memory_region const area = m_native_c128_mode ? decode_c128(offset) : decode(offset, config(offset));
	switch (area)
	{
	case SRAM_BANK0: case SRAM_BANK1: case KERNAL_SHADOW: case KERNAL_SHADOW_HW:
	case COLOUR_RAM: case C128_RAM: case C128_ROM_SHADOW:
		if (!check_ba_read())
			return 0;
		return m_sram[bank0_address(offset, area)];

	case EPROM:
		return rom_r(offset);

	case IO:
		return io_r(offset);

	default:
		if (!stretch_read_io())
			return 0;
		return bus_r(bank0_address(offset, area));
	}
}


void c64_supercpu_device::bank0_w(offs_t offset, uint8_t data)
{
	if (m_native_c128_mode)
	{
		bank0_c128_w(offset, data);
		return;
	}

	uint8_t const cfg = config(offset);
	memory_region const area = decode(offset, cfg);
	bool const internal = (cfg & 0xe0) == 0xe0;
	int const page = offset >> 8;

	switch (area)
	{
	case IO:
		io_w(offset, data);
		return;

	case COLOUR_RAM:
		if (!stretch_write())
			return;
		m_sram[0x10000 | offset] = data;
		io_bus_w(offset, data);
		return;

	case COLOUR_RAM_INTERNAL:
		if (!stretch_write())
			return;
		bus_w(offset, data);
		return;

	case ULTIMAX:
		if (!stretch_write_io_start())
			return;
		bus_w(offset, data);
		stretch_write_io(ACCU_IO);
		return;
	default:
		break;
	}

	if (internal)
	{
		if ((offset == MMU_LOAD_CONFIG) ? !stretch_write_io_start() : !stretch_write())
			return;

		if ((offset == 0x0001) && !m_dma)
			pport_w(data);

		bus_w(offset, data);

		if (offset == MMU_LOAD_CONFIG)
			stretch_write_io(ACCU_IO_LONG);
		return;
	}

	bool const mirror = mirrored(offset);

	if (offset == MMU_LOAD_CONFIG)
	{
		if (!stretch_write_io_start())
			return;

		m_sram[offset] = data;

		if (mirror || !m_dma)
			bus_w(offset, data);

		stretch_write_io(ACCU_IO_LONG);
		return;
	}

	if (mirror)
	{
		if (!stretch_write())
			return;
	}
	else if (page != 0x00)
	{
		if (!check_ba_write())
			return;
	}

	m_sram[offset] = data;

	if ((offset == 0x0001) && !m_dma)
		pport_w(data);

	if (mirror)
		bus_w(offset, data);
}


offs_t c64_supercpu_device::bank0_address(offs_t offset, memory_region area)
{
	switch (area)
	{
	case SRAM_BANK1: case KERNAL_SHADOW: case COLOUR_RAM: return 0x10000 | offset;
	case KERNAL_SHADOW_HW: return 0x8000 + offset;
	case C128_RAM: return ram_address(offset);
	case C128_ROM_SHADOW: return rom_shadow(offset, m_hwenable);
	case PROCESSOR_PORT: return ADAPTER_PROCESSOR_PORT;
	default: return offset;
	}
}


void c64_supercpu_device::bank0_c128_w(offs_t offset, uint8_t data)
{
	switch (decode_c128(offset))
	{
	case IO:
		io_w(offset, data);
		return;

	case MMU_REGISTERS:
		if (!stretch_write_io_start())
			return;
		mmu_w(offset, data);
		bus_w(offset, data);
		stretch_write_io(ACCU_IO_LONG);
		return;

	case PROCESSOR_PORT:
		if (!stretch_write_io_start())
			return;
		m_sram[ram_address(offset)] = data;
		pport_w(data);
		bus_w(ADAPTER_PROCESSOR_PORT, data);
		stretch_write_io(ACCU_IO);
		return;
	default:
		break;
	}

	offs_t const address = ram_address(offset);
	bool const mirror = mirrored_c128(offset, address) && !(m_dma && (offset >= MMU_LOAD_CONFIG) && (offset < 0xff05));
	int const page = offset >> 8;

	if (mirror)
	{
		if (!stretch_write())
			return;
	}
	else if (page != 0x00)
	{
		if (!check_ba_write())
			return;
	}

	m_sram[address] = data;

	if (mirror)
		bus_w(offset, data);
}


c64_supercpu_device::long_read c64_supercpu_device::decode_long(offs_t offset)
{
	switch (offset & 0xfe0000)
	{
	case 0x000000:
		return { SRAM_BANK1, (offset & 0xfffe) ? bank1(offset & 0xffff) : (offset & 1), 0x3ffff };
	case 0xf60000:
		if (m_simm_mask)
			return { SUPERRAM, simm_address(offset) & m_simm_mask, 0x1ffff };
		break;
	case 0xf80000: case 0xfa0000: case 0xfc0000: case 0xfe0000:
		return { EPROM, offset & 0x1ffff, 0x1ffff };
	default:
		if (m_simm_mask && (offset < m_conf_size))
			return { SUPERRAM, simm_address(offset), m_simm_mask };
		break;
	}
	return { OPEN_BUS, offset >> 16, 0xff };
}

uint8_t c64_supercpu_device::rom_r(offs_t offset)
{
	if (m_dma)
		return m_rom[offset];

	if (m_step == ACCESS_START)
	{
		if (m_fast)
			stretch_cycles(6);
		m_step = ACCESS_WAIT;
	}
	if (!(m_fast ? catch_up() : steal()))
		return 0;
	return m_rom[offset];
}

uint8_t c64_supercpu_device::simm_r(offs_t address, offs_t mask)
{
	if (m_step == ACCESS_START)
	{
		if (m_fast)
			stretch_simm(address, false);
		m_step = ACCESS_WAIT;
	}
	if (!(m_fast ? catch_up() : steal()))
		return 0;
	return m_simm[address & mask];
}

uint8_t c64_supercpu_device::long_r(offs_t offset)
{
	long_read const read = decode_long(offset);
	switch (read.area)
	{
	case SRAM_BANK1: return m_sram[read.address];
	case EPROM: return rom_r(read.address);
	case SUPERRAM: return simm_r(read.address, read.mask);
	default: return read.address;
	}
}


void c64_supercpu_device::long_w(offs_t offset, uint8_t data)
{
	switch (offset & 0xfe0000)
	{
	case 0x000000:
		m_sram[(offset & 0xfffe) ? bank1(offset & 0xffff) : (offset & 1)] = data;
		break;

	case 0xf60000:
		if (m_simm_mask)
		{
			uint32_t const addr = simm_address(offset) & m_simm_mask;

			if (m_hwenable)
				m_simm[addr & 0x1ffff] = data;
			if (m_fast)
				stretch_simm(addr, true);
		}
		break;

	case 0xf80000: case 0xfa0000: case 0xfc0000: case 0xfe0000:
		if (m_fast)
			stretch_cycles(6);
		break;

	default:
		if (m_simm_mask && (offset < m_conf_size))
		{
			uint32_t const addr = simm_address(offset);

			m_simm[addr & m_simm_mask] = data;
			if (m_fast)
				stretch_simm(addr, true);
		}
		break;
	}
}


uint8_t c64_supercpu_device::cpu_r(offs_t offset)
{
	if (machine().side_effects_disabled())
		return peek(offset);

	if (!begin_access())
		return 0;

	if (!m_maincpu->get_vda() && !m_maincpu->get_vpa())
	{
		end_access();
		return 0;
	}

	uint8_t const data = (offset & 0xff0000) ? long_r(offset) : bank0_r(offset);

	if (!m_maincpu->access_to_be_redone_noclear())
		end_access();

	return data;
}


void c64_supercpu_device::cpu_w(offs_t offset, uint8_t data)
{
	if (machine().side_effects_disabled())
		return;

	if (!begin_access())
		return;

	if (offset & 0xff0000)
		long_w(offset, data);
	else
		bank0_w(offset, data);

	if (!m_maincpu->access_to_be_redone_noclear())
		end_access();
}


uint8_t c64_supercpu_device::vector_r(offs_t offset)
{
	offs_t const addr = 0xffe0 | offset;
	memory_region const area = m_native_c128_mode ? decode_c128(addr) : decode(addr, config(addr));
	bool const reroute = ((area == KERNAL_SHADOW_HW) || (area == KERNAL_SHADOW) || (area == C128_ROM_SHADOW)) && (!emulation_mode() || m_hwenable || m_sys_1mhz || m_dosext || m_ramlink);

	return cpu_r(reroute ? (0xf80000 | addr) : addr);
}


uint8_t c64_supercpu_device::peek(offs_t offset)
{
	if (offset & 0xff0000)
	{
		long_read const read = decode_long(offset);
		switch (read.area)
		{
		case SRAM_BANK1: return m_sram[read.address];
		case EPROM: return m_rom[read.address];
		case SUPERRAM: return m_simm[read.address & read.mask];
		default: return read.address;
		}
	}

	memory_region const area = m_native_c128_mode ? decode_c128(offset) : decode(offset, config(offset));
	switch (area)
	{
	case SRAM_BANK0: case SRAM_BANK1: case KERNAL_SHADOW: case KERNAL_SHADOW_HW:
	case COLOUR_RAM: case C128_RAM: case C128_ROM_SHADOW:
		return m_sram[bank0_address(offset, area)];
	case EPROM: return m_rom[offset];
	case IO:
		if ((offset & 0xfff0) == 0xd0b0)
			return hardware_r(offset);
		if ((offset & 0xfe00) == 0xd200)
			return m_sram[0x10000 | offset];
		return io_bus_r(offset);
	default: return bus_r(bank0_address(offset, area));
	}
}
