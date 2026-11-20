// license:BSD-3-Clause
// copyright-holders:Brad Oliver,Fabio Priuli,Brian H. Gilbert
/***************************************************************************

  nes.cpp

  Driver file to handle emulation of the Nintendo Entertainment System (Famicom).

  MESS driver by Brad Oliver (bradman@pobox.com), NES sound code by Matt Conte.
  Based in part on the old xNes code, by Nicolas Hamel, Chuck Mason, Brad Oliver,
  Richard Bannister and Jeff Mitchell.

***************************************************************************/

#include "emu.h"

#include "bus/nes/disksys.h"
#include "bus/nes/nes_slot.h"
#include "bus/nes/nes_carts.h"
#include "bus/nes_ctrl/ctrl.h"
#include "cpu/m6502/rp2a03.h"
#include "video/ppu2c0x.h"
#include "screen.h"
#include "softlist_dev.h"
#include "sound/flt_rc.h"
#include "sound/ymopn.h"
#include "speaker.h"

namespace {

// ---- timing switches: each can be changed on its own, and only this file needs rebuilding ----
// CPU/PPU alignment in master clocks (NTSC 0-3, PAL 0-4); the hardware picks one at power-up and keeps it
constexpr u8 NES_CPU_PPU_ALIGNMENT = 0;
// Dendy-style clones (UMC UM6561: CPU = master / 15, PPU = master / 5 on one chip): a frame is exactly 35,464 CPU cycles,
// so the phase between the PPU's frame and the CPU cycles, which one chip resets together, fixes for good where in its CPU
// cycle the VBL flag is set. With the flag set 13 master clocks into the cycle, a $2002 read at the M2 rise of that cycle
// comes more than a PPU clock before the set, so it neither sees and clears the flag nor keeps it from being set, and a
// $2002 polling loop never takes the NMI away. With the flag set before that read (as with phase 0), Yingtao Xiao Wanzi
// (a Dendy game, NES 2.0 timing 3) loses an NMI whenever its poll lands in that cycle and its dialogue text breaks. No
// measurement of a real UM6561's phase is known; Mesen's Dendy mode makes the flag visible inside this range.
constexpr u8 NES_DENDY_CPU_PPU_ALIGNMENT = 7;
// make every CPU access at the M2 phase (rise) instead of at the start of the CPU cycle
constexpr bool NES_M2_ACCESS_AT_RISE = true;
// reads are made at the M2 rise like writes: the CPU latches the data at the fall, but the device being read responds when
// M2 rises, so making reads at the fall would delay every read by a CPU cycle
constexpr bool NES_M2_READ_SAMPLE_AT_FALL = false;

static void nes_m2_setup(rp2a03_core_device &cpu)
{
	cpu.set_m2_phase(NES_M2_ACCESS_AT_RISE);
	cpu.set_m2_read_sample_at_fall(NES_M2_READ_SAMPLE_AT_FALL);
}

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class nes_base_state : public driver_device
{
public:
	nes_base_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ctrl1(*this, "ctrl1"),
		m_ctrl2(*this, "ctrl2")
	{ }

	required_device<cpu_device> m_maincpu;
	optional_device<nes_control_port_device> m_ctrl1;
	optional_device<nes_control_port_device> m_ctrl2;

	uint8_t nes_in0_r();
	uint8_t nes_in1_r();
	void nes_in0_w(uint8_t data);

protected:
	// OUT0-2 ($4016 writes) reach the controller ports only when the APU passes from a get into a put cycle
	emu_timer *m_out_timer = nullptr;
	u8 m_out_pending = 0;
	TIMER_CALLBACK_MEMBER(out_latch_tick) { out_apply(m_out_pending); }
	virtual void out_apply(u8 data);
	void out_init();

	// Joypad output enables (NESdev "DMA", Register conflicts): /OE stays asserted across adjacent CPU cycles that read
	// the same joypad register, so the port sees one clock for the whole run of reads (NES-001 and AV Famicom). The RF
	// Famicom, Twin Famicom and Famicom Titler pass /OE for half of each cycle only: every read cycle clocks the port.
	bool joy_read_clocks(int port);
	bool m_joy_clock_every_read = false;
	bool m_cart_powered = false;    // the board has had its power-on setup
	u64 m_joy_cycle[2] = { ~u64(0) - 1, ~u64(0) - 1 };   // CPU cycle of the last read of $4016, $4017
	u8 m_joy_data[2] = { 0, 0 };                         // what the port drove on that read
};

class nes_state : public nes_base_state
{
public:
	nes_state(const machine_config &mconfig, device_type type, const char *tag) :
		nes_base_state(mconfig, type, tag),
		m_mainram(*this, "mainram"),
		m_ppu(*this, "ppu"),
		m_screen(*this, "screen"),
		m_exp(*this, "exp"),
		m_special(*this, "special"),
		m_cartslot(*this, "nes_slot"),
		m_out_hp1(*this, "out_hp1"),
		m_out_hp2(*this, "out_hp2"),
		m_out_lp(*this, "out_lp"),
		m_disk(*this, "disk"),
		m_epsm(*this, "epsm"),
		m_prg_bank(*this, "prg%u", 0U)
	{ }

	uint8_t fc_in0_r();
	uint8_t fc_in1_r();
	virtual void out_apply(u8 data) override;
	void apply_revisions();
	void epsm_w(offs_t offset, u8 data);
	void epsm_access(u8 data);
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	uint32_t screen_update_nes(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void init_famicom();

	// these are needed until we modernize the FDS controller
	DECLARE_MACHINE_START(fds);
	DECLARE_MACHINE_START(famitwin);
	DECLARE_MACHINE_RESET(fds);
	DECLARE_MACHINE_RESET(famitwin);
	DECLARE_MACHINE_RESET(famitvc1);
	void setup_disk(nes_disksys_device *slot);

	void suborkbd(machine_config &config);
	void famipalc(machine_config &config);
	void famicom(machine_config &config);
	void famicomo(machine_config &config);
	void famitvc1(machine_config &config);
	void famitwin(machine_config &config);
	void fctitler(machine_config &config);
	void nespal(machine_config &config);
	void nespalc(machine_config &config);
	void nes(machine_config &config);
	void fds(machine_config &config);
	void nes_map(address_map &map) ATTR_COLD;

private:
	// video-related

	// misc

	std::unique_ptr<uint8_t[]>    m_ciram; // PPU nametable RAM - external to PPU!

	required_shared_ptr<uint8_t> m_mainram;

	required_device<ppu2c0x_device> m_ppu;
	required_device<screen_device> m_screen;
	optional_device<nes_control_port_device> m_exp;
	optional_device<nes_control_port_device> m_special;
	optional_device<nes_cart_slot_device> m_cartslot;
	required_device<filter_rc_device> m_out_hp1, m_out_hp2, m_out_lp;   // console audio output stage
	optional_device<nes_disksys_device> m_disk;
	// the Expansion Port Sound Module (NESdev "Expansion Port Sound Module"): a YMF288 on the expansion port of the front-loading
	// NES, which the Famicom does not have
	optional_device<ymf288_device> m_epsm;
	bool m_epsm_enabled = false;
	u8 m_epsm_out1 = 0;     // OUT1 as the module saw it last
	u8 m_epsm_data = 0;     // the YMF288 data byte, assembled from two nibbles of $4016 writes
	u8 m_epsm_addr = 0;     // YMF288 A1 and A0, latched with the high nibble
	// BMW8544 (NES 2.0 mapper 292) watches the data bus on every CPU write
	memory_passthrough_handler m_cart_write_tap;
	memory_passthrough_handler m_ppu_sync_tap;
	memory_bank_array_creator<4> m_prg_bank;
};

/***************************************************************************
    FUNCTIONS
***************************************************************************/

//-------------------------------------------------
//  machine_reset
//-------------------------------------------------

// the chip revisions chosen in the machine configuration
void nes_state::apply_revisions()
{
	// the 2C02 revision (only the PPU_2C02 type uses it)
	if (ioport_port *const rev = ioport("PPU_REV"))
	{
		m_ppu->set_revision(rev->read() & 0x03);
		m_ppu->set_composite((rev->read() >> 2) & 3);
	}

	// the RP2A03G's date of manufacture: chips made from 1990 on (and the RP2A03H) have the DMC's unexpected reload
	// DMA (NESdev "DMA", Bugs). Only the machines with an RP2A03G whose behavior NESdev documents have the setting.
	if (ioport_port *const rev = ioport("CPU_REV"))
		downcast<rp2a03_device &>(*m_maincpu).set_dmc_unexpected_dma(BIT(rev->read(), 0));

	// the module on the expansion port of the NES-001, if one is plugged in
	m_epsm_enabled = false;
	if (m_epsm)
		if (ioport_port *const epsm = ioport("EPSM"))
			m_epsm_enabled = BIT(epsm->read(), 0);
}

void nes_state::machine_reset()
{
	apply_revisions();

	// Set up the board at power-on. The reset button only reaches a board that can notice a reset (see
	// nes_cart_slot_device::console_reset_reaches_board): the others keep their registers.
	if (m_cartslot && (!m_cart_powered || m_cartslot->console_reset_reaches_board()))
		m_cartslot->pcb_reset();
	m_cart_powered = true;

	m_maincpu->reset();
}

//-------------------------------------------------
//  machine_start
//-------------------------------------------------

void nes_state::machine_start()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	out_init();
	save_item(NAME(m_epsm_out1));
	save_item(NAME(m_epsm_data));
	save_item(NAME(m_epsm_addr));

	// Fill main RAM with an arbitrary pattern (alternating 0x00/0xff) for software that depends on its contents at boot up (tsk tsk!)
	// The fill value is a compromise since certain games malfunction with zero-filled memory, others with one-filled memory
	// Examples: Minna no Taabou won't boot with all 0x00, Sachen's Dancing Block won't boot with all 0xff, Terminator 2 skips its copyright screen with all 0x00
	for (int i = 0; i < 0x800; i += 2)
	{
		m_mainram[i] = 0x00;
		m_mainram[i + 1] = 0xff;
	}

	// CIRAM (Character Internal RAM)
	// NES has 2KB of internal RAM which can be used to fill the 4x1KB banks of PPU RAM at $2000-$2fff
	// Line A10 is exposed to the carts, so that games can change CIRAM mapping in PPU (we emulate this with the set_nt_mirroring
	// function). CIRAM can also be disabled by the game (if e.g. VROM or cart RAM has to be used in PPU...
	m_ciram = std::make_unique<uint8_t[]>(0x800);
	// other pointers got set in the loading routine, because they 'belong' to the cart itself


	if (m_cartslot && m_cartslot->m_cart)
	{
		// Set up memory handlers
		space.install_read_handler(0x4100, 0x5fff, read8sm_delegate(*m_cartslot, FUNC(nes_cart_slot_device::read_l)));
		space.install_write_handler(0x4100, 0x5fff, write8sm_delegate(*m_cartslot, FUNC(nes_cart_slot_device::write_l)));
		space.install_read_handler(0x6000, 0x7fff, read8sm_delegate(*m_cartslot, FUNC(nes_cart_slot_device::read_m)));
		space.install_write_handler(0x6000, 0x7fff, write8sm_delegate(*m_cartslot, FUNC(nes_cart_slot_device::write_m)));
		for(int i = 0; i < 4; i++)
			space.install_read_bank(0x8000 + 0x2000*i, 0x9fff + 0x2000*i, m_prg_bank[i]);
		space.install_write_handler(0x8000, 0xffff, write8sm_delegate(*m_cartslot, FUNC(nes_cart_slot_device::write_h)));

		m_ppu->space(AS_PROGRAM).install_readwrite_handler(0, 0x1fff, read8sm_delegate(*m_cartslot->m_cart, FUNC(device_nes_cart_interface::chr_r)), write8sm_delegate(*m_cartslot->m_cart, FUNC(device_nes_cart_interface::chr_w)));
		m_ppu->space(AS_PROGRAM).install_readwrite_handler(0x2000, 0x3eff, read8sm_delegate(*m_cartslot->m_cart, FUNC(device_nes_cart_interface::nt_r)), write8sm_delegate(*m_cartslot->m_cart, FUNC(device_nes_cart_interface::nt_w)));
		m_ppu->set_scanline_callback(*m_cartslot->m_cart, FUNC(device_nes_cart_interface::scanline_irq));
		m_ppu->set_hblank_callback(*m_cartslot->m_cart, FUNC(device_nes_cart_interface::hblank_irq));
		if (m_cartslot->m_cart->wants_a12())
		{
			m_ppu->set_a12_callback(*m_cartslot->m_cart, FUNC(device_nes_cart_interface::ppu_a12_rise));
			m_cartslot->m_cart->set_a12_driven(true);
			m_ppu->set_a12_on_fall(m_cartslot->m_cart->mmc3_irq_variant() == 3);
			if (m_cartslot->m_cart->wants_a12_filter())
				m_ppu->set_a12_filter_callback(*m_cartslot->m_cart, FUNC(device_nes_cart_interface::a12_rise_accepted));
		}
		if (m_cartslot->m_cart->wants_a12_level())
			m_ppu->set_a12_level_callback(*m_cartslot->m_cart, FUNC(device_nes_cart_interface::ppu_a12_level));
		if (m_cartslot->m_cart->wants_ppu_addr_bus())
			m_ppu->set_addr_bus_callback(*m_cartslot->m_cart, FUNC(device_nes_cart_interface::ppu_addr_bus));
		if (m_cartslot->m_cart->wants_ppu_strobes())
		{
			m_ppu->set_ale_callback(*m_cartslot->m_cart, FUNC(device_nes_cart_interface::ppu_ale));
			m_ppu->set_rd_callback(*m_cartslot->m_cart, FUNC(device_nes_cart_interface::ppu_rd));
			m_ppu->set_wr_callback(*m_cartslot->m_cart, FUNC(device_nes_cart_interface::ppu_wr));
		}
		if (m_cartslot->m_cart->wants_m2())
			downcast<rp2a03_core_device &>(*m_maincpu).set_m2_callback(*m_cartslot->m_cart, FUNC(device_nes_cart_interface::m2_w));
		m_ppu->set_latch(*m_cartslot->m_cart, FUNC(device_nes_cart_interface::ppu_latch));
		if (m_cartslot->m_cart->wants_read_latch())
			m_ppu->set_read_latch(*m_cartslot->m_cart, FUNC(device_nes_cart_interface::ppu_read_latch));

		// install additional handlers (read_h, read_ex, write_ex)
		static const int r_h_pcbs[] =
		{
			BMC_BS300,
			NORAEBANG_BOARD,
			BROKE_RAINBOW,
			BMC_HIK8IN1,
			BMC_FAMILY_4646,
			BMC_TECHLINE9IN1,
			BMC_ING022,
			BMC_PJOY84,
			AVE_MAXI15,
			BANDAI_DATACH,
			KONAMI_VRC5,      // $DC00/$DD00 read the Kanji translation
			BMC_REALTEC432,   // the solder pad replaces PRG-ROM on reads
			WAIXING_SGZLZ,    // submapper 3: the solder pad replaces PRG-ROM on reads
			BMC_1200IN1,      // submapper 1: the solder pad replaces PRG A3-A0
			BANDAI_KARAOKE,
			BATMAP_SRRX,
			BMC_70IN1,
			BMC_800IN1,
			BMC_8157,
			BMC_970630C,
			BMC_DS927,
			BMC_KC885,
			BMC_TELETUBBIES,
			BMC_VT5201,
			BTL_PALTHENA,
			CAMERICA_ALADDIN,
			GG_NROM,
			KAISER_KS7010,
			KAISER_KS7022,
			KAISER_KS7030,
			KAISER_KS7031,
			KAISER_KS7037,
			KAISER_KS7057,
			SACHEN_3013,
			SACHEN_3014,
			STD_DISKSYS,
			STD_EXROM,
			STD_NROM368,
			SUNSOFT_DCS,
			UNL_2708,
			UNL_2A03PURITANS,
			UNL_43272,
			UNL_EH8813A,
			UNL_LH10,
			UNL_LH32,
			UNL_RT01
		};

		static const int w_ex_pcbs[] =
		{
			BMC_N32_4IN1,
			BTL_SMB2JB,
			BTL_YUNG08,
			BMC_BITCORP4602,    // $4022
			UNL_AC08,
			UNL_SMB2J,
			BMC_TECHLINE9IN1
		};

		static const int rw_ex_pcbs[] =
		{
			BTL_09034A,
			KAISER_KS7017,
			STD_DISKSYS,
			UNL_603_5052
		};

		int pcb_id = m_cartslot->get_pcb_id();

		if (std::find(std::begin(r_h_pcbs), std::end(r_h_pcbs), pcb_id) != std::end(r_h_pcbs))
		{
			logerror("read_h installed!\n");
			space.install_read_handler(0x8000, 0xffff, read8sm_delegate(*m_cartslot, FUNC(nes_cart_slot_device::read_h)));
		}

		if (std::find(std::begin(w_ex_pcbs), std::end(w_ex_pcbs), pcb_id) != std::end(w_ex_pcbs))
		{
			logerror("write_ex installed!\n");
			space.install_write_handler(0x4020, 0x40ff, write8sm_delegate(*m_cartslot, FUNC(nes_cart_slot_device::write_ex)));
		}

		if (std::find(std::begin(rw_ex_pcbs), std::end(rw_ex_pcbs), pcb_id) != std::end(rw_ex_pcbs))
		{
			logerror("read_ex & write_ex installed!\n");
			space.install_read_handler(0x4020, 0x40ff, read8sm_delegate(*m_cartslot, FUNC(nes_cart_slot_device::read_ex)));
			space.install_write_handler(0x4020, 0x40ff, write8sm_delegate(*m_cartslot, FUNC(nes_cart_slot_device::write_ex)));
		}

		// BMW8544 (NES 2.0 mapper 292) latches the data of every CPU write, including writes
		// to internal RAM and zero page, which never reach the cartridge handlers
		if (pcb_id == UNL_BMW8544)
		{
			m_cart_write_tap = space.install_write_tap(
					0x0000, 0xffff, "nes_cart_bus_write",
					[this] (offs_t offset, u8 &data, u8 mem_mask)
					{
						if (!machine().side_effects_disabled())
							m_cartslot->m_cart->cpu_write_latch(data);
					});
		}

		// The PPU is run lazily: bring it up to the present before every cartridge write, or a bank or mirroring switch
		// made mid-line would also apply to the earlier dots of the line still to be drawn (Fire Hawk's title screen
		// switches mapper 71 single-screen mirroring at dot ~235 of the split line, drawing the whole line from the
		// other nametable)
		m_ppu_sync_tap = space.install_write_tap(
				0x4020, 0xffff, "nes_ppu_sync",
				[this] (offs_t offset, u8 &data, u8 mem_mask)
				{
					if (machine().side_effects_disabled())
						return;
					// a register that only loads when M2 falls (see nes_cart_slot_device::register_loads_at_m2_fall)
					// still shows its old value to the PPU for the rest of this CPU cycle
					if (m_cartslot && m_cartslot->register_loads_at_m2_fall())
						m_ppu->catch_up_to(machine().time() - m_maincpu->bus_offset() + m_maincpu->cycles_to_attotime(1));
					else
						m_ppu->catch_up();
				});

		m_cartslot->pcb_start(m_ciram.get());
		m_cartslot->m_cart->pcb_reg_postload(machine());
	}

	// register saves
	save_pointer(NAME(m_ciram), 0x800);
}


//-------------------------------------------------
//  INPUTS
//-------------------------------------------------

bool nes_base_state::joy_read_clocks(int port)
{
	if (machine().side_effects_disabled())
		return false;
	const u64 cycle = m_maincpu->total_cycles();
	const bool adjacent = (cycle == m_joy_cycle[port]) || (cycle == m_joy_cycle[port] + 1);
	m_joy_cycle[port] = cycle;
	return m_joy_clock_every_read || !adjacent;
}

uint8_t nes_base_state::nes_in0_r()
{
	if (joy_read_clocks(0))
		m_joy_data[0] = m_ctrl1->read_bit0() | m_ctrl1->read_bit34();
	return (downcast<rp2a03_device &>(*m_maincpu).open_bus() & 0xe0) | m_joy_data[0]; // bits 5-7 are open bus
}

uint8_t nes_base_state::nes_in1_r()
{
	if (joy_read_clocks(1))
		m_joy_data[1] = m_ctrl2->read_bit0() | m_ctrl2->read_bit34();
	return (downcast<rp2a03_device &>(*m_maincpu).open_bus() & 0xe0) | m_joy_data[1]; // bits 5-7 are open bus
}

void nes_base_state::out_apply(u8 data)
{
	m_ctrl1->write(data);
	m_ctrl2->write(data);
}

void nes_base_state::out_init()
{
	m_out_timer = timer_alloc(FUNC(nes_base_state::out_latch_tick), this);
	save_item(NAME(m_out_pending));
	save_item(NAME(m_cart_powered));
	save_item(NAME(m_joy_cycle));
	save_item(NAME(m_joy_data));
}

void nes_base_state::nes_in0_w(uint8_t data)
{
	// The 2A03 latches its OUT pins from the $4016 register only at the end of a "get" APU cycle (as the APU moves into a
	// put cycle). A write made on such a cycle is latched as it ends; one made on a put cycle waits for the next cycle,
	// unless a later write replaces it first - so a one-cycle pulse from DEC $4016 strobes the controllers only when it
	// covers that latch point (AccuracyCoin "Controller Strobing" tests 3/4; TriCNES)
	m_out_pending = data;
	if (!m_out_timer)
	{
		out_apply(data);
		return;
	}
	const u64 cycle = m_maincpu->total_cycles();
	const bool get = (cycle & 1) == 0;   // rp2a03_core_device::dma_get_cycle()'s convention
	if (get)
	{
		m_out_timer->adjust(attotime::never);
		out_apply(data);
	}
	else
		m_out_timer->adjust(m_maincpu->cycles_to_attotime(2) - m_maincpu->bus_offset());
}


uint8_t nes_state::fc_in0_r()
{
	if (joy_read_clocks(0))
	{
		// bit 0 from controller port, bit 2 from P2 controller microphone, bit 1 from expansion port
		m_joy_data[0] = (m_ctrl1->read_bit0() | m_ctrl2->read_bit2() | m_exp->read_exp(0)) & 0x07;
	}
	// The Famicom only drives D0-D2 of $4016: its two controller lines of port 1 that the NES puts on D3 and D4 do not
	// exist, so bits 3-7 are open bus (NESdev "Standard controller": "the top three (or five) bits are not driven")
	return (downcast<rp2a03_device &>(*m_maincpu).open_bus() & 0xf8) | m_joy_data[0];
}

uint8_t nes_state::fc_in1_r()
{
	if (joy_read_clocks(1))
	{
		// bit 0 from controller port, bits 1-4 from expansion port (in theory bit 0 also can be read on AV Famicom when
		// controller is unplugged)
		m_joy_data[1] = m_ctrl2->read_bit0() | m_exp->read_exp(1);
	}
	return (downcast<rp2a03_device &>(*m_maincpu).open_bus() & 0xe0) | m_joy_data[1]; // bits 5-7 are open bus
}

// the OUT pins are latched inside the 2A03 (see nes_in0_w), whatever console it sits in; on the Famicom they also
// reach the expansion port
void nes_state::out_apply(u8 data)
{
	nes_base_state::out_apply(data);
	if (m_exp)
		m_exp->write(data);
	if (m_epsm_enabled)
		epsm_access(data);
}

// The module's two ways in (NESdev "Expansion Port Sound Module"). Fully decoded, as Mesen2 and the games written for it
// use it: $401C = register write with A1 = 0, $401D = data write with A1 = 0, $401E and $401F the same with A1 = 1; these are
// the YMF288's four ports in order. The module has no read path, so reads of these addresses stay open bus.
void nes_state::epsm_w(offs_t offset, u8 data)
{
	if (m_epsm_enabled)
		m_epsm->write(offset, data);
}

// Universal access, from $4016 writes: when OUT1 (bit 1) rises, bits 7-4 are latched as the high nibble of the data and
// bits 3 and 2 as YMF288 A0 and A1. NESdev gives only that latch; the rest is from Mesen2: when OUT1 falls, bits 7-4 of that
// write are the low nibble and the byte is written to the chip.
void nes_state::epsm_access(u8 data)
{
	const bool high = BIT(data, 1);
	const bool was_high = BIT(m_epsm_out1, 0);
	if (high && !was_high)
	{
		m_epsm_data = (data & 0xf0) | (m_epsm_data & 0x0f);
		m_epsm_addr = (BIT(data, 2) << 1) | BIT(data, 3);
	}
	else if (!high && was_high)
	{
		m_epsm_data = (m_epsm_data & 0xf0) | (data >> 4);
		m_epsm->write(m_epsm_addr, m_epsm_data);
	}
	m_epsm_out1 = high ? 1 : 0;
}


void nes_state::init_famicom()
{
	// setup alt input handlers for additional FC input devices
	address_space &space = m_maincpu->space(AS_PROGRAM);
	space.install_read_handler(0x4016, 0x4016, read8smo_delegate(*this, FUNC(nes_state::fc_in0_r)));
	space.install_read_handler(0x4017, 0x4017, read8smo_delegate(*this, FUNC(nes_state::fc_in1_r)));
}


void nes_state::video_start()
{
}


/***************************************************************************

  Display refresh

***************************************************************************/

uint32_t nes_state::screen_update_nes(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// render the ppu
	m_ppu->render(bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}

void nes_state::nes_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().mirror(0x1800).share("mainram");                              // RAM
	map(0x2000, 0x3fff).rw(m_ppu, FUNC(ppu2c0x_device::read), FUNC(ppu2c0x_device::write)); // PPU registers
	map(0x4014, 0x4014).lw8(NAME([this] (u8 data) { downcast<rp2a03_device &>(*m_maincpu).oam_dma_w(data); })); // sprite DMA (2A03 DMA unit)
	map(0x4016, 0x4016).rw(FUNC(nes_state::nes_in0_r), FUNC(nes_state::nes_in0_w));         // IN0 - input port 1
	map(0x4017, 0x4017).r(FUNC(nes_state::nes_in1_r));                                      // IN1 - input port 2
	map(0x401c, 0x401f).w(FUNC(nes_state::epsm_w));                                         // Expansion Port Sound Module, when plugged in
	map(0x4020, 0x40ff).lr8(NAME([this] (offs_t) { return downcast<rp2a03_device &>(*m_maincpu).open_bus(); })); // unmapped unless the cart has hardware here
	// 0x4100-0x5fff -> LOW HANDLER defined on a pcb base
	// 0x6000-0x7fff -> MID HANDLER defined on a pcb base
	// 0x8000-0xffff -> HIGH HANDLER defined on a pcb base
}

// input devices go through slot options; the PPU revision is a machine configuration setting (NESdev "PPU variants")
static INPUT_PORTS_START( ppu_revision )
	PORT_START("PPU_REV")
	PORT_CONFNAME( 0x03, 0x02, "PPU revision" )
	PORT_CONFSETTING(    0x00, "RP2C02 to RP2C02D (OAM and palette not readable)" )
	PORT_CONFSETTING(    0x01, "RP2C02E (OAM and palette not readable, OAMADDR corruption)" )
	PORT_CONFSETTING(    0x02, "RP2C02G" )
	PORT_CONFSETTING(    0x03, "RP2C02H" )
	PORT_CONFNAME( 0x0c, 0x00, "Video output" )
	PORT_CONFSETTING(    0x00, "RGB palette" )
	PORT_CONFSETTING(    0x04, "Composite (decoded signal)" )
	PORT_CONFSETTING(    0x08, "Composite, delay-line chroma (PAL)" )
INPUT_PORTS_END

// NESdev "DMA", Bugs: the unexpected reload DMA exists on the RP2A03H and on the RP2A03G made from 1990 on
static INPUT_PORTS_START( cpu_revision )
	PORT_START("CPU_REV")
	PORT_CONFNAME( 0x01, 0x00, "CPU revision" )
	PORT_CONFSETTING(    0x00, "RP2A03G (made before 1990)" )
	PORT_CONFSETTING(    0x01, "RP2A03G (made from 1990 on) / RP2A03H" )
INPUT_PORTS_END

// the Expansion Port Sound Module plugs into the expansion port on the bottom of the front-loading NES-001 (NESdev
// "Expansion Port Sound Module"); the Famicom has no such port
static INPUT_PORTS_START( epsm )
	PORT_START("EPSM")
	PORT_CONFNAME( 0x01, 0x00, "Expansion port sound module" )
	PORT_CONFSETTING(    0x00, "None" )
	PORT_CONFSETTING(    0x01, "EPSM (YMF288)" )
INPUT_PORTS_END

static INPUT_PORTS_START( nes )
	PORT_INCLUDE( ppu_revision )
	PORT_INCLUDE( cpu_revision )
	PORT_INCLUDE( epsm )
INPUT_PORTS_END

static INPUT_PORTS_START( famicom )
	PORT_INCLUDE( ppu_revision )
	PORT_INCLUDE( cpu_revision )
INPUT_PORTS_END

// machines without the CPU revision setting: the 2A07 (NESdev: not known whether it has the DMC DMA bugs), the clone
// CPUs, and the earliest RP2A03, which predates the bug
static INPUT_PORTS_START( nes_fixedcpu )
	PORT_INCLUDE( ppu_revision )
INPUT_PORTS_END

// the PAL NES: a 2A07 without the CPU revision setting, and the expansion port
static INPUT_PORTS_START( nespal )
	PORT_INCLUDE( nes_fixedcpu )
	PORT_INCLUDE( epsm )
INPUT_PORTS_END


void nes_state::nes(machine_config &config)
{
	// basic machine hardware
	rp2a03_device &maincpu(RP2A03G(config, m_maincpu, NTSC_APU_CLOCK));
	maincpu.set_addrmap(AS_PROGRAM, &nes_state::nes_map);
	nes_m2_setup(maincpu);

	// NESdev "NTSC video": a dot is 4 master clocks, a line 341 dots and a frame 262 lines (60.0988 Hz); the picture is the 256
	// dots of the first 240 lines. The PPU keeps its own vblank and takes its dot times from the line period.
	SCREEN(config, m_screen);
	m_screen->set_raw(RP2A03_NTSC_XTAL / 4, 341, 0, 256, 262, 0, 240);
	m_screen->set_screen_update(FUNC(nes_state::screen_update_nes));

	PPU_2C02(config, m_ppu);
	m_ppu->set_cpu_tag(m_maincpu);
	m_ppu->set_cpu_ppu_alignment(NES_CPU_PPU_ALIGNMENT);
	m_ppu->set_power_on_warmup(true);   // NES-001: CPU and PPU leave reset together (NESdev "PPU power up state")
	m_ppu->int_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	// sound hardware
	// console audio output stage (NESdev wiki "APU Mixer"): first-order high-pass filters at 90 Hz and 440 Hz and a
	// first-order low-pass at 14 kHz, after the APU and the cartridge audio are mixed; R = 1 / (2 pi f C) with C = 1 uF
	SPEAKER(config, "mono").front_center();
	FILTER_RC(config, m_out_hp1).set_rc(filter_rc_device::HIGHPASS, 1768.39, 0, 0, CAP_U(1)).add_route(ALL_OUTPUTS, m_out_hp2, 1.0);   // 90 Hz
	FILTER_RC(config, m_out_hp2).set_rc(filter_rc_device::HIGHPASS, 361.71, 0, 0, CAP_U(1)).add_route(ALL_OUTPUTS, m_out_lp, 1.0);     // 440 Hz
	FILTER_RC(config, m_out_lp).set_rc(filter_rc_device::LOWPASS, 11.368, 0, 0, CAP_U(1)).add_route(ALL_OUTPUTS, "mono", 1.0);       // 14 kHz
	maincpu.add_route(ALL_OUTPUTS, m_out_hp1, 0.90);

	// The module mixes its YMF288 (clocked at 8 MHz as in Mesen2) with the console's audio on its own output, after the
	// console's output stage, so it does not go through the filters above. Its level against the console's is not documented:
	// FM and SSG are mixed at half level each, a choice that has not been measured.
	YMF288(config, m_epsm, XTAL(8'000'000));
	m_epsm->add_route(ALL_OUTPUTS, "mono", 0.5);

	NES_CONTROL_PORT(config, m_ctrl1, nes_control_port1_devices, "joypad").set_screen_tag(m_screen);
	NES_CONTROL_PORT(config, m_ctrl2, nes_control_port2_devices, "joypad").set_screen_tag(m_screen);
	NES_CONTROL_PORT(config, m_special, nes_control_special_devices, nullptr).set_screen_tag(m_screen);

	NES_CART_SLOT(config, m_cartslot, NTSC_APU_CLOCK, nes_cart, nullptr).set_must_be_loaded(true);
	m_cartslot->set_cpu(m_maincpu);
	m_cartslot->irq_callback().set_inputline(m_maincpu, m6502_device::IRQ_LINE);
	m_cartslot->add_route(ALL_OUTPUTS, m_out_hp1, 1.0);   // cartridge (expansion) audio joins the console's before the filters
	SOFTWARE_LIST(config, "cart_list").set_original("nes").set_filter("!EXP");
	SOFTWARE_LIST(config, "ade_list").set_original("nes_ade");         // Camerica/Codemasters Aladdin Deck Enhancer mini-carts
	SOFTWARE_LIST(config, "ntb_list").set_original("nes_ntbrom");      // Sunsoft Nantettate! Baseball mini-carts
	SOFTWARE_LIST(config, "kstudio_list").set_original("nes_kstudio"); // Bandai Karaoke Studio expansion carts
	SOFTWARE_LIST(config, "datach_list").set_original("nes_datach");   // Bandai Datach Joint ROM System mini-carts
	SOFTWARE_LIST(config, "famibox_list").set_compatible("famibox");   // FamicomBox/FamicomStation carts
}

void nes_state::nespal(machine_config &config)
{
	nes(config);

	// basic machine hardware
	m_maincpu->set_clock(PAL_APU_CLOCK);
	downcast<rp2a03_core_device &>(*m_maincpu).set_m2_waveform(13, 32);   // 2A07: M2 is high for 19/32 of a cycle
	downcast<rp2a03_core_device &>(*m_maincpu).set_dma_2a07(true);        // 2A07: no extra register reads during DMA

	PPU_2C07(config.replace(), m_ppu);
	m_ppu->set_cpu_tag(m_maincpu);
	m_ppu->set_cpu_ppu_alignment(NES_CPU_PPU_ALIGNMENT);
	m_ppu->set_power_on_warmup(true);
	m_ppu->int_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	m_cartslot->set_clock(PAL_APU_CLOCK);

	// video hardware
	// NESdev "PAL video": a dot is 5 master clocks of the PAL crystal, 341 dots to a line, 312 lines to a frame (50.0070 Hz);
	// the Dendy clones use the same
	m_screen->set_raw(RP2A03_PAL_XTAL / 5, 341, 0, 256, 312, 0, 240);
}

void nes_state::famicom(machine_config &config)
{
	nes(config);
	config.device_remove("epsm");      // the Famicom has no expansion port module slot
	m_ppu->set_console_reset(false);   // the Famicom reset button only resets the CPU
	m_ppu->set_power_on_warmup(false); // and at power-on its PPU leaves reset about a frame before the CPU
	m_joy_clock_every_read = true;     // RF Famicom (also the Twin Famicom and Famicom Titler): see joy_read_clocks()

	// the Famicom's audio output has only a first-order high-pass filter at 37 Hz (NESdev wiki "APU Mixer"), followed
	// by its RF modulator; C = 0 disables a filter stage
	m_out_hp1->set_rc(filter_rc_device::HIGHPASS, 4301.5, 0, 0, CAP_U(1));   // 37 Hz
	m_out_hp2->set_rc(filter_rc_device::HIGHPASS, 361.71, 0, 0, 0);
	m_out_lp->set_rc(filter_rc_device::LOWPASS, 11.368, 0, 0, 0);

	NES_CONTROL_PORT(config.replace(), m_ctrl1, fc_control_port1_devices, "joypad").set_screen_tag(m_screen);
	NES_CONTROL_PORT(config.replace(), m_ctrl2, fc_control_port2_devices, "joypad").set_screen_tag(m_screen);
	NES_CONTROL_PORT(config, m_exp, fc_expansion_devices, nullptr).set_screen_tag(m_screen);

	subdevice<software_list_device>("cart_list")->set_filter(nullptr);
	SOFTWARE_LIST(config, "flop_list").set_original("famicom_flop");
	SOFTWARE_LIST(config, "cass_list").set_original("famicom_cass");
}

void nes_state::famicomo(machine_config &config)
{
	famicom(config);

	// basic machine hardware
	rp2a03_device &maincpu(RP2A03(config.replace(), m_maincpu, NTSC_APU_CLOCK));
	maincpu.set_addrmap(AS_PROGRAM, &nes_state::nes_map);
	nes_m2_setup(maincpu);

	// sound hardware
	maincpu.add_route(ALL_OUTPUTS, m_out_hp1, 0.90);
}

void nes_state::nespalc(machine_config &config)
{
	nespal(config);
	config.device_remove("epsm");      // the clones have no expansion port module slot

	m_maincpu->set_clock(PALC_APU_CLOCK);
	// no M2 duty cycle is documented for the PAL famiclone CPUs: use the 2A03G figure
	downcast<rp2a03_core_device &>(*m_maincpu).set_m2_waveform(9, 24);
	// nor is the 2A07's DMA fix documented for them (NESdev "DMA": UA6527P clones have the 2A03's DMC DMA bugs)
	downcast<rp2a03_core_device &>(*m_maincpu).set_dma_2a07(false);
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_state::nes_map);

	// UMC 6538 and friends -- extends time for rendering dummy scanlines
	PPU_PALC(config.replace(), m_ppu);
	m_ppu->set_cpu_tag(m_maincpu);
	m_ppu->set_cpu_ppu_alignment(NES_DENDY_CPU_PPU_ALIGNMENT);
	m_ppu->int_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	m_cartslot->set_clock(PALC_APU_CLOCK);
}

void nes_state::famipalc(machine_config &config)
{
	nespalc(config);

	NES_CONTROL_PORT(config.replace(), m_ctrl1, fc_control_port1_devices, "joypad").set_screen_tag(m_screen);
	NES_CONTROL_PORT(config.replace(), m_ctrl2, fc_control_port2_devices, "joypad").set_screen_tag(m_screen);
	NES_CONTROL_PORT(config, m_exp, fc_expansion_devices, nullptr).set_screen_tag(m_screen);

	SOFTWARE_LIST(config, "cass_list").set_original("famicom_cass");
}

void nes_state::suborkbd(machine_config &config)
{
	famipalc(config);

	// TODO: emulate the parallel port bus!
	m_exp->set_default_option("subor_keyboard");
	m_exp->set_fixed(true);
}

void nes_state::setup_disk(nes_disksys_device *slot)
{
	if (slot)
	{
		address_space &space = m_maincpu->space(AS_PROGRAM);

		// Set up memory handlers
		space.install_read_handler(0x4020, 0x40ff, read8sm_delegate(*slot, FUNC(nes_disksys_device::read_ex)));
		space.install_write_handler(0x4020, 0x40ff, write8sm_delegate(*slot, FUNC(nes_disksys_device::write_ex)));
		space.install_read_handler(0x4100, 0x5fff, read8sm_delegate(*slot, FUNC(device_nes_cart_interface::read_l)));
		space.install_write_handler(0x4100, 0x5fff, write8sm_delegate(*slot, FUNC(device_nes_cart_interface::write_l)));
		space.install_read_handler(0x6000, 0x7fff, read8sm_delegate(*slot, FUNC(nes_disksys_device::read_m)));
		space.install_write_handler(0x6000, 0x7fff, write8sm_delegate(*slot, FUNC(nes_disksys_device::write_m)));
		space.install_read_handler(0x8000, 0xffff, read8sm_delegate(*slot, FUNC(nes_disksys_device::read_h)));
		space.install_write_handler(0x8000, 0xffff, write8sm_delegate(*slot, FUNC(nes_disksys_device::write_h)));

		slot->vram_alloc(0x2000);
		slot->prgram_alloc(0x8000);

		slot->pcb_start(machine(), m_ciram.get(), false);
		m_ppu->space(AS_PROGRAM).install_readwrite_handler(0, 0x1fff, read8sm_delegate(*slot, FUNC(device_nes_cart_interface::chr_r)), write8sm_delegate(*slot, FUNC(device_nes_cart_interface::chr_w)));
		m_ppu->space(AS_PROGRAM).install_readwrite_handler(0x2000, 0x3eff, read8sm_delegate(*slot, FUNC(device_nes_cart_interface::nt_r)), write8sm_delegate(*slot, FUNC(device_nes_cart_interface::nt_w)));
		m_ppu->set_scanline_callback(*slot, FUNC(device_nes_cart_interface::scanline_irq));
		m_ppu->set_hblank_callback(*slot, FUNC(nes_disksys_device::hblank_irq));
		// the RAM adapter's timer IRQ and DRAM refresh watchdog count CPU cycles on M2: with the adapter on the
		// motherboard (no cartridge slot) the CPU's M2 has to be connected to it here, as for a cartridge
		if (static_cast<device_nes_cart_interface &>(*slot).wants_m2())
			downcast<rp2a03_core_device &>(*m_maincpu).set_m2_callback(*slot, FUNC(device_nes_cart_interface::m2_w));
		m_ppu->set_latch(*slot, FUNC(device_nes_cart_interface::ppu_latch));
		if (slot->wants_read_latch())
			m_ppu->set_read_latch(*slot, FUNC(device_nes_cart_interface::ppu_read_latch));
	}
}

MACHINE_START_MEMBER( nes_state, fds )
{
	m_ciram = std::make_unique<uint8_t[]>(0x800);
	setup_disk(m_disk);
	out_init();   // the 2A03's OUT latch (this override replaces nes_state::machine_start(), which sets it up)

	// register saves
	save_pointer(NAME(m_ciram), 0x800);
}

MACHINE_RESET_MEMBER( nes_state, fds )
{
	apply_revisions();

	// Reset the mapper variables
	m_disk->pcb_reset();

	// the rest is the same as for nes/famicom/dendy
	m_maincpu->reset();
}

void nes_state::fds(machine_config &config)
{
	famicom(config);

	MCFG_MACHINE_START_OVERRIDE(nes_state, fds)
	MCFG_MACHINE_RESET_OVERRIDE(nes_state, fds)

	config.device_remove("nes_slot");
	NES_DISKSYS(config, m_disk, NTSC_APU_CLOCK);
	m_disk->set_cpu(m_maincpu);
	m_disk->irq_callback().set_inputline(m_maincpu, m6502_device::IRQ_LINE);
	// the RAM adapter is on the motherboard here, not in a cartridge slot: its audio goes straight into the console's
	subdevice<device_sound_interface>("disk:rp2c33snd")->reset_routes().add_route(0, m_out_hp1, 0.227);

	config.device_remove("cart_list");
	config.device_remove("cass_list");
	config.device_remove("ade_list");
	config.device_remove("ntb_list");
	config.device_remove("kstudio_list");
	config.device_remove("datach_list");
	config.device_remove("famibox_list");
}

MACHINE_RESET_MEMBER( nes_state, famitvc1 )
{
	apply_revisions();

	// TODO: supposedly the C1 used the cartridge connector audio pins to detect
	// the presence of a cart and picks the builtin ROM accordingly. If so, the C1
	// should not support Famicom expansion audio chips.

	// Reset the mapper variables. Will also mark the char-gen ram as dirty
	if (m_cartslot->exists())
	{
		m_cartslot->pcb_reset();
	}
	else
	{
		m_maincpu->space(AS_PROGRAM).install_rom(0x8000, 0x9fff, 0x6000, memregion("canvas_prg")->base());
		m_ppu->space(AS_PROGRAM).install_rom(0x0000, 0x1fff, memregion("canvas_chr")->base());
	}

	m_maincpu->reset();
}

void nes_state::famitvc1(machine_config &config)
{
	famicomo(config); // has an RP2A03 like the original Famicom
	m_joy_clock_every_read = false;   // not one of the consoles NESdev lists with the gated joypad enables

	MCFG_MACHINE_RESET_OVERRIDE( nes_state, famitvc1 )

	PPU_2C03B(config.replace(), m_ppu);
	m_ppu->set_cpu_tag(m_maincpu);
	m_ppu->int_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	m_cartslot->set_must_be_loaded(false);
}

MACHINE_START_MEMBER( nes_state, famitwin )
{
	// start the base nes stuff
	machine_start();

	// if there is no cart inserted, setup the disk expansion instead
	if (!m_cartslot->exists())
	{
		setup_disk(m_disk);

		// replace the famicom disk ROM with the twin famicom one (until we modernize the floppy drive)
		m_maincpu->space(AS_PROGRAM).install_rom(0xe000, 0xffff, memregion("maincpu")->base() + 0xe000);
	}
}

MACHINE_RESET_MEMBER( nes_state, famitwin )
{
	apply_revisions();

	// Reset the mapper variables. Will also mark the char-gen ram as dirty
	m_cartslot->pcb_reset();
	// if there is no cart inserted, initialize the disk expansion instead
	if (!m_cartslot->exists())
		m_disk->pcb_reset();

	// the rest is the same as for nes/famicom/dendy
	m_maincpu->reset();
}

void nes_state::famitwin(machine_config &config)
{
	famicom(config);

	MCFG_MACHINE_START_OVERRIDE( nes_state, famitwin )
	MCFG_MACHINE_RESET_OVERRIDE( nes_state, famitwin )

	m_cartslot->set_must_be_loaded(false);

	NES_DISKSYS(config, m_disk, NTSC_APU_CLOCK);
	m_disk->set_cpu(m_maincpu);
	m_disk->irq_callback().set_inputline(m_maincpu, m6502_device::IRQ_LINE);
	// the built-in disk drive is on the motherboard, not in a cartridge slot: its audio goes straight into the console's
	subdevice<device_sound_interface>("disk:rp2c33snd")->reset_routes().add_route(0, m_out_hp1, 0.227);
}

void nes_state::fctitler(machine_config &config)
{
	famicom(config);

	// PPU is really RC2C05-99, but it can't be like the other 2C05s since they swap PPUCTRL and PPUMASK registers
	PPU_2C03B(config.replace(), m_ppu);
	m_ppu->set_cpu_tag(m_maincpu);
	m_ppu->int_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
}


ROM_START( nes )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )  // Main RAM
ROM_END

ROM_START( nespal )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )  // Main RAM
ROM_END

ROM_START( famicom )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )  // Main RAM
ROM_END

ROM_START( famicomo )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )  // Main RAM
ROM_END

ROM_START( famitvc1 )
	ROM_REGION( 0x2000, "canvas_prg", 0 )
	ROM_LOAD( "ix0402ce.ic109", 0x0000, 0x2000, CRC(96456b13) SHA1(a4dcb3c4f2be5077f0d197e870a26414287ce189) ) // dump needs verified

	ROM_REGION( 0x2000, "canvas_chr", 0 )
	ROM_LOAD( "ix0403ce.ic110", 0x0000, 0x2000, CRC(9cba4524) SHA1(2bd833f8049bf7a14ce337c3cde35f2140242a18) ) // dump needs verified

	ROM_REGION( 0xc0, "ppu:palette", 0 )
	ROM_LOAD( "rp2c0x.pal", 0x00, 0xc0, CRC(48de65dc) SHA1(d10acafc8da9ff479c270ec01180cca61efe62f5) )
ROM_END

#define rom_fds rom_famicom

ROM_START( famitwin )
	ROM_REGION( 0x10000, "maincpu", 0 )  // Main RAM
	ROM_LOAD( "rp2c33a-02.bin", 0xe000, 0x2000, CRC(4df24a6c) SHA1(e4e41472c454f928e53eb10e0509bf7d1146ecc1) ) // "Famicom" logo instead of Nintendo logo
ROM_END

ROM_START( fctitler )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )  // Main RAM

	// builtin has its own MMC1B1, 8K PRGRAM (battery backed), and 8K CHRRAM
	// TODO: add switch that selects which boots: builtin vs cart
	ROM_REGION( 0x80000, "builtin_prg", 0 )
	ROM_LOAD( "x1252ce.prg", 0x00000, 0x80000, CRC(696712f9) SHA1(1b9475f569ea9943122676ce65165dc82d11ef38) )

	ROM_REGION( 0xc0, "ppu:palette", 0 )
	ROM_LOAD( "rp2c0x.pal", 0x00, 0xc0, CRC(48de65dc) SHA1(d10acafc8da9ff479c270ec01180cca61efe62f5) )
ROM_END

// see http://www.disgruntleddesigner.com/chrisc/drpcjr/index.html
// and http://www.disgruntleddesigner.com/chrisc/drpcjr/DrPCJrMemMap.txt
ROM_START( drpcjr )
	ROM_REGION( 0x18000, "maincpu", 0 )  // Main RAM + program banks
	// 4 banks to be mapped in 0xe000-0xffff (or 8 banks to be mapped in 0xe000-0xefff & 0xf000-0xffff).
	// Banks selected by writing at 0x4180
	ROM_LOAD("drpcjr_bios.bin", 0x10000, 0x8000, CRC(c8fbef89) SHA1(2cb0a817b31400cdf27817d09bae7e69f41b062b) ) // bios vers. 1.0a
	// Not sure if we should support this: hacked version 1.5a by Chris Covell with bugfixes and GameGenie support
//  ROM_LOAD("drpcjr_v1_5_gg.bin", 0x10000, 0x8000, CRC(98f2033b) SHA1(93c114da787a19279d1a46667c2f69b49e25d4f1) )
ROM_END

ROM_START( iq501 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )  // Main RAM
ROM_END

ROM_START( iq502 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )  // Main RAM
ROM_END

ROM_START( dendy )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )  // Main RAM
ROM_END

ROM_START( dendy2 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )  // Main RAM
ROM_END

ROM_START( gchinatv )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )  // Main RAM
ROM_END

ROM_START( sb486 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )  // Main RAM
ROM_END

} // anonymous namespace

/***************************************************************************

  Game driver(s)

***************************************************************************/

//    YEAR  NAME      PARENT   COMPAT  MACHINE   INPUT    CLASS      INIT          COMPANY          FULLNAME

// Nintendo Entertainment System hardware
CONS( 1985, nes,      0,       0,      nes,      nes,     nes_state, empty_init,   "Nintendo",      "Nintendo Entertainment System / Famicom (NTSC)", MACHINE_SUPPORTS_SAVE )
CONS( 1987, nespal,   nes,     0,      nespal,   nespal, nes_state, empty_init,   "Nintendo",      "Nintendo Entertainment System (PAL)",            MACHINE_SUPPORTS_SAVE )

// Famicom hardware
CONS( 1983, famicom,  0,       nes,    famicom,  famicom, nes_state, init_famicom, "Nintendo",      "Famicom",                         MACHINE_SUPPORTS_SAVE )
CONS( 1983, famicomo, famicom, 0,      famicomo, nes_fixedcpu, nes_state, init_famicom, "Nintendo",      "Famicom (earlier, with RP2A03)",  MACHINE_SUPPORTS_SAVE )
CONS( 1983, famitvc1, famicom, 0,      famitvc1, nes_fixedcpu, nes_state, init_famicom, "Sharp",         "My Computer Terebi C1",           MACHINE_SUPPORTS_SAVE )
CONS( 1986, fds,      famicom, 0,      fds,      famicom, nes_state, init_famicom, "Nintendo",      "Famicom (w/ Disk System add-on)", MACHINE_SUPPORTS_SAVE )
CONS( 1986, famitwin, famicom, 0,      famitwin, famicom, nes_state, init_famicom, "Sharp",         "Twin Famicom",                    MACHINE_SUPPORTS_SAVE )
CONS( 1989, fctitler, famicom, 0,      fctitler, famicom, nes_state, init_famicom, "Sharp",         "Famicom Titler",                  MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )

// Clone hardware
// Many knockoffs using derivatives of the UMC board design, later incorporated into single CMOS chips, were manufactured before and past the end of the Famicom's timeline.

// !! PAL clones documented here !!
// Famicom-based
CONS( 1992, iq501,    0,       nes,    famipalc, nes_fixedcpu, nes_state, init_famicom, "Micro Genius",  "IQ-501",               MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
CONS( 1992, iq502,    0,       nes,    famipalc, nes_fixedcpu, nes_state, init_famicom, "Micro Genius",  "IQ-502",               MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
CONS( 1992, dendy,    iq501,   0,      famipalc, nes_fixedcpu, nes_state, init_famicom, "Steepler",      "Dendy Classic 1",      MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
CONS( 1992, dendy2,   iq502,   0,      famipalc, nes_fixedcpu, nes_state, init_famicom, "Steepler",      "Dendy Classic 2",      MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
CONS( 198?, gchinatv, 0,       nes,    famipalc, nes_fixedcpu, nes_state, init_famicom, "Golden China",  "Golden China TV Game", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )

// Subor/Xiao Ba Wang hardware and derivatives
// These clones implement a keyboard and a parallel port for printing from a word processor. Later models have mice, PS/2 ports, serial ports and a floppy drive.
CONS( 1993, sb486,    0,       nes,    suborkbd, nes_fixedcpu, nes_state, init_famicom, "Subor",         "SB-486", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )

// !! NTSC clones documented here !!
// Famicom-based
// Bung hardware
// Mice, keyboard, etc, including a floppy drive that allows you to run games with a selection of 4 internal "mappers" available on the system.
CONS( 1996, drpcjr,   0,       nes,    famicom,  nes_fixedcpu, nes_state, init_famicom, "Bung",          "Doctor PC Jr", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
