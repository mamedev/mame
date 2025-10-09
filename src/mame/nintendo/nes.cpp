// license:BSD-3-Clause
// copyright-holders:Brad Oliver,Fabio Priuli
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
#include "speaker.h"

namespace {

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
		m_disk(*this, "disk"),
		m_prg_bank(*this, "prg%u", 0U)
	{ }

	uint8_t fc_in0_r();
	uint8_t fc_in1_r();
	void fc_in0_w(uint8_t data);
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	uint32_t screen_update_nes(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_vblank_nes(int state);

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
	int m_last_frame_flip = 0;

	// misc
	ioport_port       *m_io_disksel = nullptr;

	std::unique_ptr<uint8_t[]>    m_ciram; // PPU nametable RAM - external to PPU!

	required_shared_ptr<uint8_t> m_mainram;

	required_device<ppu2c0x_device> m_ppu;
	required_device<screen_device> m_screen;
	optional_device<nes_control_port_device> m_exp;
	optional_device<nes_control_port_device> m_special;
	optional_device<nes_cart_slot_device> m_cartslot;
	optional_device<nes_disksys_device> m_disk;
	memory_bank_array_creator<4> m_prg_bank;
};

/***************************************************************************
    FUNCTIONS
***************************************************************************/

//-------------------------------------------------
//  machine_reset
//-------------------------------------------------

void nes_state::machine_reset()
{
	// Reset the mapper variables. Will also mark the char-gen ram as dirty
	if (m_cartslot)
		m_cartslot->pcb_reset();

	m_maincpu->reset();
}

//-------------------------------------------------
//  machine_start
//-------------------------------------------------

void nes_state::machine_start()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

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

	m_io_disksel = ioport("FLIPDISK");

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
		m_ppu->set_latch(*m_cartslot->m_cart, FUNC(device_nes_cart_interface::ppu_latch));

		// install additional handlers (read_h, read_ex, write_ex)
		static const int r_h_pcbs[] =
		{
			AVE_MAXI15,
			BANDAI_DATACH,
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
			UNL_AC08,
			UNL_SMB2J
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

		m_cartslot->pcb_start(m_ciram.get());
		m_cartslot->m_cart->pcb_reg_postload(machine());
	}

	// register saves
	save_item(NAME(m_last_frame_flip));
	save_pointer(NAME(m_ciram), 0x800);
}


//-------------------------------------------------
//  INPUTS
//-------------------------------------------------

uint8_t nes_base_state::nes_in0_r()
{
	uint8_t ret = 0x40;
	ret |= m_ctrl1->read_bit0();
	ret |= m_ctrl1->read_bit34();
	return ret;
}

uint8_t nes_base_state::nes_in1_r()
{
	uint8_t ret = 0x40;
	ret |= m_ctrl2->read_bit0();
	ret |= m_ctrl2->read_bit34();
	return ret;
}

void nes_base_state::nes_in0_w(uint8_t data)
{
	m_ctrl1->write(data);
	m_ctrl2->write(data);
}


uint8_t nes_state::fc_in0_r()
{
	uint8_t ret = 0x40;
	// bit 0 from controller port
	ret |= m_ctrl1->read_bit0();

	// bit 2 from P2 controller microphone
	ret |= m_ctrl2->read_bit2();

	// and bit 1 comes from expansion port
	ret |= m_exp->read_exp(0);
	return ret;
}

uint8_t nes_state::fc_in1_r()
{
	uint8_t ret = 0x40;
	// bit 0 from controller port
	ret |= m_ctrl2->read_bit0();

	// bits 1-4 from expansion port (in theory bit 0 also can be read on AV Famicom when controller is unplugged)
	ret |= m_exp->read_exp(1);
	return ret;
}

void nes_state::fc_in0_w(uint8_t data)
{
	m_ctrl1->write(data);
	m_ctrl2->write(data);
	m_exp->write(data);
}


void nes_state::init_famicom()
{
	// setup alt input handlers for additional FC input devices
	address_space &space = m_maincpu->space(AS_PROGRAM);
	space.install_read_handler(0x4016, 0x4016, read8smo_delegate(*this, FUNC(nes_state::fc_in0_r)));
	space.install_write_handler(0x4016, 0x4016, write8smo_delegate(*this, FUNC(nes_state::fc_in0_w)));
	space.install_read_handler(0x4017, 0x4017, read8smo_delegate(*this, FUNC(nes_state::fc_in1_r)));
}


void nes_state::video_start()
{
	m_last_frame_flip =  0;
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

void nes_state::screen_vblank_nes(int state)
{
	// on rising edge
	if (!state)
	{
		// if this is a disk system game, check for the flip-disk key
		if ((m_cartslot && m_cartslot->exists() && (m_cartslot->get_pcb_id() == STD_DISKSYS))   // first scenario = disksys in m_cartslot (= famicom)
				|| m_disk)  // second scenario = disk via fixed internal disk option (fds & famitwin)
		{
			if (m_io_disksel)
			{
				// latch this input so it doesn't go at warp speed
				if ((m_io_disksel->read() & 0x01) && (!m_last_frame_flip))
				{
					if (m_disk)
						m_disk->disk_flip_side();
					else
						m_cartslot->disk_flip_side();
					m_last_frame_flip = 1;
				}

				if (!(m_io_disksel->read() & 0x01))
					m_last_frame_flip = 0;
			}
		}
	}
}


void nes_state::nes_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().mirror(0x1800).share("mainram");                              // RAM
	map(0x2000, 0x3fff).rw(m_ppu, FUNC(ppu2c0x_device::read), FUNC(ppu2c0x_device::write)); // PPU registers
	map(0x4014, 0x4014).w(m_ppu, FUNC(ppu2c0x_device::spriteram_dma));                            // stupid address space hole
	map(0x4016, 0x4016).rw(FUNC(nes_state::nes_in0_r), FUNC(nes_state::nes_in0_w));         // IN0 - input port 1
	map(0x4017, 0x4017).r(FUNC(nes_state::nes_in1_r));                                      // IN1 - input port 2
	// 0x4100-0x5fff -> LOW HANDLER defined on a pcb base
	// 0x6000-0x7fff -> MID HANDLER defined on a pcb base
	// 0x8000-0xffff -> HIGH HANDLER defined on a pcb base
}

static INPUT_PORTS_START( nes )
	// input devices go through slot options
INPUT_PORTS_END

static INPUT_PORTS_START( famicom )
	// input devices go through slot options
	PORT_START("FLIPDISK") // fake key
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Change Disk Side") PORT_CODE(KEYCODE_SPACE)
INPUT_PORTS_END


void nes_state::nes(machine_config &config)
{
	// basic machine hardware
	rp2a03_device &maincpu(RP2A03G(config, m_maincpu, NTSC_APU_CLOCK));
	maincpu.set_addrmap(AS_PROGRAM, &nes_state::nes_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60.0988);
	// This isn't used so much to calulate the vblank duration (the PPU code tracks that manually) but to determine
	// the number of cycles in each scanline for the PPU scanline timer. Since the PPU has 20 vblank scanlines + 2
	// non-rendering scanlines, we compensate. This ends up being 2500 cycles for the non-rendering portion, 2273
	// cycles for the actual vblank period.
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC((113.66/(NTSC_APU_CLOCK.dvalue()/1000000)) *
							 (ppu2c0x_device::VBLANK_LAST_SCANLINE_NTSC-ppu2c0x_device::VBLANK_FIRST_SCANLINE+1+2)));
	m_screen->set_size(32*8, 262);
	m_screen->set_visarea(0*8, 32*8-1, 0*8, 30*8-1);
	m_screen->set_screen_update(FUNC(nes_state::screen_update_nes));
	m_screen->screen_vblank().set(FUNC(nes_state::screen_vblank_nes));

	PPU_2C02(config, m_ppu);
	m_ppu->set_cpu_tag(m_maincpu);
	m_ppu->int_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	maincpu.add_route(ALL_OUTPUTS, "mono", 0.90);

	NES_CONTROL_PORT(config, m_ctrl1, nes_control_port1_devices, "joypad").set_screen_tag(m_screen);
	NES_CONTROL_PORT(config, m_ctrl2, nes_control_port2_devices, "joypad").set_screen_tag(m_screen);
	NES_CONTROL_PORT(config, m_special, nes_control_special_devices, nullptr).set_screen_tag(m_screen);

	NES_CART_SLOT(config, m_cartslot, NTSC_APU_CLOCK, nes_cart, nullptr).set_must_be_loaded(true);
	SOFTWARE_LIST(config, "cart_list").set_original("nes").set_filter("!exp");
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

	PPU_2C07(config.replace(), m_ppu);
	m_ppu->set_cpu_tag(m_maincpu);
	m_ppu->int_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	m_cartslot->set_clock(PAL_APU_CLOCK);

	// video hardware
	m_screen->set_refresh_hz(50.0070);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC((106.53/(PAL_APU_CLOCK.dvalue()/1000000)) *
							 (ppu2c0x_device::VBLANK_LAST_SCANLINE_PAL-ppu2c0x_device::VBLANK_FIRST_SCANLINE+1+2)));
	m_screen->set_size(32*8, 312);
	m_screen->set_visarea(0*8, 32*8-1, 0*8, 30*8-1);
}

void nes_state::famicom(machine_config &config)
{
	nes(config);

	NES_CONTROL_PORT(config.replace(), m_ctrl1, fc_control_port1_devices, "joypad").set_screen_tag(m_screen);
	NES_CONTROL_PORT(config.replace(), m_ctrl2, fc_control_port2_devices, "joypad").set_screen_tag(m_screen);
	NES_CONTROL_PORT(config, m_exp, fc_expansion_devices, nullptr).set_screen_tag(m_screen);

	SOFTWARE_LIST(config.replace(), "cart_list").set_original("nes");
	SOFTWARE_LIST(config, "flop_list").set_original("famicom_flop");
	SOFTWARE_LIST(config, "cass_list").set_original("famicom_cass");
}

void nes_state::famicomo(machine_config &config)
{
	famicom(config);

	// basic machine hardware
	rp2a03_device &maincpu(RP2A03(config.replace(), m_maincpu, NTSC_APU_CLOCK));
	maincpu.set_addrmap(AS_PROGRAM, &nes_state::nes_map);

	// sound hardware
	maincpu.add_route(ALL_OUTPUTS, "mono", 0.90);
}

void nes_state::nespalc(machine_config &config)
{
	nespal(config);

	m_maincpu->set_clock(PALC_APU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_state::nes_map);

	// UMC 6538 and friends -- extends time for rendering dummy scanlines
	PPU_PALC(config.replace(), m_ppu);
	m_ppu->set_cpu_tag(m_maincpu);
	m_ppu->int_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	m_cartslot->set_clock(PALC_APU_CLOCK);

	// video hardware
	m_screen->set_refresh_hz(50.0070);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC((113.66/(PALC_APU_CLOCK.dvalue()/1000000)) *
							 (ppu2c0x_device::VBLANK_LAST_SCANLINE_PAL-ppu2c0x_device::VBLANK_FIRST_SCANLINE_PALC+1+2)));
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
		m_ppu->set_latch(*slot, FUNC(device_nes_cart_interface::ppu_latch));
	}
}

MACHINE_START_MEMBER( nes_state, fds )
{
	m_ciram = std::make_unique<uint8_t[]>(0x800);
	m_io_disksel = ioport("FLIPDISK");
	setup_disk(m_disk);

	// register saves
	save_item(NAME(m_last_frame_flip));
	save_pointer(NAME(m_ciram), 0x800);
}

MACHINE_RESET_MEMBER( nes_state, fds )
{
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
	NES_DISKSYS(config, "disk", NTSC_APU_CLOCK);

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

	NES_DISKSYS(config, "disk", NTSC_APU_CLOCK);
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
CONS( 1985, nes,      0,       0,      nes,      nes,     nes_state, empty_init,   "Nintendo",      "Nintendo Entertainment System / Famicom (NTSC)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
CONS( 1987, nespal,   nes,     0,      nespal,   nes,     nes_state, empty_init,   "Nintendo",      "Nintendo Entertainment System (PAL)",            MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )

// Famicom hardware
CONS( 1983, famicom,  0,       nes,    famicom,  famicom, nes_state, init_famicom, "Nintendo",      "Famicom",                         MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
CONS( 1983, famicomo, famicom, 0,      famicomo, famicom, nes_state, init_famicom, "Nintendo",      "Famicom (earlier, with RP2A03)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
CONS( 1983, famitvc1, famicom, 0,      famitvc1, famicom, nes_state, init_famicom, "Sharp",         "My Computer Terebi C1",           MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
CONS( 1986, fds,      famicom, 0,      fds,      famicom, nes_state, init_famicom, "Nintendo",      "Famicom (w/ Disk System add-on)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
CONS( 1986, famitwin, famicom, 0,      famitwin, famicom, nes_state, init_famicom, "Sharp",         "Twin Famicom",                    MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
CONS( 1989, fctitler, famicom, 0,      fctitler, famicom, nes_state, init_famicom, "Sharp",         "Famicom Titler",                  MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )

// Clone hardware
// Many knockoffs using derivatives of the UMC board design, later incorporated into single CMOS chips, were manufactured before and past the end of the Famicom's timeline.

// !! PAL clones documented here !!
// Famicom-based
CONS( 1992, iq501,    0,       nes,    famipalc, nes,     nes_state, init_famicom, "Micro Genius",  "IQ-501",               MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
CONS( 1992, iq502,    0,       nes,    famipalc, nes,     nes_state, init_famicom, "Micro Genius",  "IQ-502",               MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
CONS( 1992, dendy,    iq501,   0,      famipalc, nes,     nes_state, init_famicom, "Steepler",      "Dendy Classic 1",      MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
CONS( 1992, dendy2,   iq502,   0,      famipalc, nes,     nes_state, init_famicom, "Steepler",      "Dendy Classic 2",      MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
CONS( 198?, gchinatv, 0,       nes,    famipalc, nes,     nes_state, init_famicom, "Golden China",  "Golden China TV Game", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )

// Subor/Xiao Ba Wang hardware and derivatives
// These clones implement a keyboard and a parallel port for printing from a word processor. Later models have mice, PS/2 ports, serial ports and a floppy drive.
CONS( 1993, sb486,    0,       nes,    suborkbd, nes,     nes_state, init_famicom, "Subor",         "SB-486", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )

// !! NTSC clones documented here !!
// Famicom-based
// Bung hardware
// Mice, keyboard, etc, including a floppy drive that allows you to run games with a selection of 4 internal "mappers" available on the system.
CONS( 1996, drpcjr,   0,       nes,    famicom,  famicom, nes_state, init_famicom, "Bung",          "Doctor PC Jr", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
