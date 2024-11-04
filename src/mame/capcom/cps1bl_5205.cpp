// license:BSD-3-Clause
// copyright-holders:David Haywood

/*
    CPS1 single board bootlegs (thought to be produced by "Playmark")

    sound hardware: Z80, YM2151, 2x oki MSM5205 (instead of oki M6295)

    Games known to use this h/w:
    Captain Commando                       911014 ETC
    Knights of the Round                   911127 ETC
    Street Fighter II: The World Warrior   910204 ETC
    Street Fighter II': Champion Edition   920313 ETC    * this might be hacked WW (uses WW portraits on character select screen)
    Street Fighter II': Magic Delta Turbo  920313 ETC
    The King of Dragons                    ?  (No dump)

    Generally the sound quality is quite poor compared to official Capcom hardware (consequence of M6295->2xM5205 conversion).
    Most noticeable is missing percussion backing of music tracks and no fade in/out effect.
    Often the 2x M5205 are clocked with a 400KHz xtal (should really be 384KHz) so pitch of samples is slightly out as well.
    The sf2 sets seem to have quite a few missing samples?

Status of each game:
--------------------
sf2mdt, sf2mdtb: OK.
sf2mdta, sf2ceb: Scroll 2X has strange 0x200 writes that cause missing fighters' portraits at the vs. screen
                 and glitched backgrounds during fights. Masking them out seems a hack.
captcommb2:      OK.
knightsb3:       OK.

*/

#include "emu.h"
#include "fcrash.h"

#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "sound/msm5205.h"
#include "sound/ymopm.h"
#include "speaker.h"


namespace {

#define CPS1_ROWSCROLL_OFFS  (0x20/2)    /* base of row scroll offsets in other RAM */
#define CODE_SIZE            0x400000


class cps1bl_5205_state : public fcrash_state
{
public:
	cps1bl_5205_state(const machine_config &mconfig, device_type type, const char *tag)
		: fcrash_state(mconfig, type, tag)
		, m_msm_mux(*this, "msm_mux%u", 1)
	{ }

	void captcommb2(machine_config &config);
	void knightsb(machine_config &config);
	void sf2b(machine_config &config);
	void sf2mdt(machine_config &config);

	void init_captcommb2();
	void init_knightsb();
	void init_sf2b();
	void init_sf2mdt();
	void init_sf2mdta();
	void init_sf2mdtb();

private:
	DECLARE_MACHINE_START(captcommb2);
	DECLARE_MACHINE_RESET(captcommb2);
	DECLARE_MACHINE_START(sf2mdt);

	void captcommb2_layer_w(offs_t offset, uint16_t data);
	void captcommb2_soundlatch_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t captcommb2_soundlatch_r();
	void captcommb2_snd_bankswitch_w(uint8_t data);
	void captcommb2_mux_select_w(int state);
	void knightsb_layer_w(offs_t offset, uint16_t data);
	void sf2b_layer_w(offs_t offset, uint16_t data);
	void sf2mdt_layer_w(offs_t offset, uint16_t data);
	void sf2mdt_soundlatch_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void sf2mdta_layer_w(offs_t offset, uint16_t data);

	void captcommb2_map(address_map &map) ATTR_COLD;
	void sf2b_map(address_map &map) ATTR_COLD;
	void sf2mdt_map(address_map &map) ATTR_COLD;
	void captcommb2_z80map(address_map &map) ATTR_COLD;

	bool m_captcommb2_mux_toggle = false;

	optional_device_array<ls157_device, 2> m_msm_mux;
};

class captcommb2_state : public cps1bl_5205_state
{
public:
	captcommb2_state(const machine_config &mconfig, device_type type, const char *tag)
		: cps1bl_5205_state(mconfig, type, tag)
	{ }

private:
	void bootleg_render_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect) override;
};


void cps1bl_5205_state::captcommb2_layer_w(offs_t offset, uint16_t data)
{
	switch (offset)
	{
	case 0x00:
		m_cps_a_regs[0x0e / 2] = data;  // scroll1 y
		break;
	case 0x01:
		m_cps_a_regs[0x0c / 2] = data;  // scroll1 x
		break;
	case 0x02:
		m_cps_a_regs[0x12 / 2] = data;  // scroll2 y
		m_cps_a_regs[CPS1_ROWSCROLL_OFFS] = data;  // probably don't need this
		break;
	case 0x03:
		m_cps_a_regs[0x10 / 2] = data;  // scroll2 x
		break;
	case 0x04:
		m_cps_a_regs[0x16 / 2] = data;  // scroll3 y
		break;
	case 0x05:
		m_cps_a_regs[0x14 / 2] = data;  // scroll3 x
		break;
	case 0x06:
		m_cps_b_regs[m_layer_enable_reg / 2] = data;
		m_cps_a_regs[0x02 / 2] = 0x9000 + ((data & 0x1f) << 5);  // scroll1 base
		break;
	case 0x10:
		m_cps_b_regs[m_layer_mask_reg[1] / 2] = data;
		break;
	case 0x11:
		m_cps_b_regs[m_layer_mask_reg[2] / 2] = (data & 0x8000) ? data & 0x7fff : data;
		m_cps_b_regs[m_layer_mask_reg[3] / 2] = (data & 0x8000) ? 0x7fff : 0x07ff;
		break;
	}
}

void cps1bl_5205_state::captcommb2_soundlatch_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_soundlatch->write(data & 0xff);
		m_audiocpu->set_input_line(0, ASSERT_LINE);
	}
}

uint8_t cps1bl_5205_state::captcommb2_soundlatch_r()
{
	uint8_t latch = m_soundlatch->read();
	m_audiocpu->set_input_line(0, CLEAR_LINE);
	return latch;
}

void cps1bl_5205_state::captcommb2_snd_bankswitch_w(uint8_t data)
{
	m_msm_1->reset_w(BIT(data, 5));
	m_msm_2->reset_w(BIT(data, 4));
	membank("bank1")->set_entry(data & 0x0f);
}

void cps1bl_5205_state::captcommb2_mux_select_w(int state)
{
	// toggle both mux select pins (and fire /nmi)
	// vck halved by flipflop IC186  ~2kHz
	if (!state)
		return;

	m_captcommb2_mux_toggle = !m_captcommb2_mux_toggle;
	m_msm_mux[0]->select_w(m_captcommb2_mux_toggle);
	m_msm_mux[1]->select_w(m_captcommb2_mux_toggle);
	m_audiocpu->set_input_line(INPUT_LINE_NMI, m_captcommb2_mux_toggle);
}

void cps1bl_5205_state::knightsb_layer_w(offs_t offset, uint16_t data)
{
	switch (offset)
	{
	case 0x00:
		m_cps_a_regs[0x0e / 2] = data;
		break;
	case 0x01:
		m_cps_a_regs[0x0c / 2] = data;
		break;
	case 0x02:
		m_cps_a_regs[0x12 / 2] = data;
		m_cps_a_regs[CPS1_ROWSCROLL_OFFS] = data; /* row scroll start */
		break;
	case 0x03:
		m_cps_a_regs[0x10 / 2] = data;
		break;
	case 0x04:
		m_cps_a_regs[0x16 / 2] = data;
		break;
	case 0x05:
		m_cps_a_regs[0x14 / 2] = data;
		break;
	case 0x06:
		{
			switch (data)
			{
			case 0x0000:
			case 0x001f:
			case 0x00ff:
			case 0x07ff:
				data = 0x12f2;
				break;
			case 0x2000:
				data = 0x06c0;
				break;
			case 0x5800:
			case 0x5f00:
				data = 0x12c0;
				break;
			case 0x80ff:
			case 0x87ff:
				data = 0x1380;
				break;
			case 0xa000:
				data = 0x24c0;
				break;
			case 0xd800:
				data = 0x1380;
				break;
			default:
				printf ("Unknown control word = %X\n",data);
				data = 0x12c0;
			}
		m_cps_b_regs[m_layer_enable_reg / 2] = data;
		break;
		}
	case 0x10:
		m_cps_b_regs[m_layer_mask_reg[1] / 2] = data;
		break;
	case 0x11:
		m_cps_b_regs[m_layer_mask_reg[2] / 2] = data;
		break;
	case 0x12:
		m_cps_b_regs[m_layer_mask_reg[3] / 2] = data;
	}
}

void cps1bl_5205_state::sf2b_layer_w(offs_t offset, uint16_t data)
{
	switch (offset)
	{
	case 0x06:
		m_cps_a_regs[0x0c / 2] = data + 0xffbe; /* scroll 1x */
		break;
	case 0x07:
		m_cps_a_regs[0x0e / 2] = data; /* scroll 1y */
		break;
	case 0x08:
		m_cps_a_regs[0x14 / 2] = data + 0xffce; /* scroll 3x */
		break;
	case 0x09:
		m_cps_a_regs[0x12 / 2] = data; /* scroll 2y */
		m_cps_a_regs[CPS1_ROWSCROLL_OFFS] = data; /* row scroll start */
		break;
	case 0x0a:
		m_cps_a_regs[0x10 / 2] = data + 0xffce; /* scroll 2x */
		break;
	case 0x0b:
		m_cps_a_regs[0x16 / 2] = data; /* scroll 3y */
		break;
	case 0x26:
		m_cps_b_regs[m_layer_enable_reg / 2] = data;
		break;
	default:
		printf("%X:%X ",offset,data);
	}
}

void cps1bl_5205_state::sf2mdt_layer_w(offs_t offset, uint16_t data)
{
	/* layer enable and scroll registers are written here - passing them to m_cps_b_regs and m_cps_a_regs for now for drawing routines
	the scroll layers aren't buttery smooth, due to the lack of using the row scroll address tables in the rendering code, this is also
	supported by the fact that the game doesn't write the table address anywhere */

	switch (offset)
	{
	case 0x06:
		m_cps_a_regs[0x14 / 2] = data + 0xffce; /* scroll 3x */
		break;
	case 0x07:
		m_cps_a_regs[0x16 / 2] = data; /* scroll 3y */
		break;
	case 0x08:
		m_cps_a_regs[0x10 / 2] = data + 0xffce; /* scroll 2x */
		break;
	case 0x09:
		m_cps_a_regs[0x0c / 2] = data + 0xffca; /* scroll 1x */
		break;
	case 0x0a:
		m_cps_a_regs[0x12 / 2] = data; /* scroll 2y */
		m_cps_a_regs[CPS1_ROWSCROLL_OFFS] = data; /* row scroll start */
		break;
	case 0x0b:
		m_cps_a_regs[0x0e / 2] = data; /* scroll 1y */
		break;
	case 0x26:
		m_cps_b_regs[m_layer_enable_reg / 2] = data;
	}
}

void cps1bl_5205_state::sf2mdt_soundlatch_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_8_15)
	{
		m_soundlatch->write(data >> 8);
		m_audiocpu->set_input_line(0, ASSERT_LINE);
	}
}

void cps1bl_5205_state::sf2mdta_layer_w(offs_t offset, uint16_t data)
{
	/* layer enable and scroll registers are written here - passing them to m_cps_b_regs and m_cps_a_regs for now for drawing routines
	the scroll layers aren't buttery smooth, due to the lack of using the row scroll address tables in the rendering code, this is also
	supported by the fact that the game doesn't write the table address anywhere */

	switch (offset)
	{
	case 0x06:
		m_cps_a_regs[0x0c / 2] = data + 0xffbe; /* scroll 1x */
		break;
	case 0x07:
		m_cps_a_regs[0x0e / 2] = data; /* scroll 1y */
		break;
	case 0x08:
		m_cps_a_regs[0x14 / 2] = data + 0xffce; /* scroll 3x */
		break;
	case 0x09:
		m_cps_a_regs[0x12 / 2] = data; /* scroll 2y */
		m_cps_a_regs[CPS1_ROWSCROLL_OFFS] = data; /* row scroll start */
		break;
	case 0x0a:
		m_cps_a_regs[0x10 / 2] = 0xffce; /* scroll 2x */
		break;
	case 0x0b:
		m_cps_a_regs[0x16 / 2] = data; /* scroll 3y */
		break;
	case 0x26:
		m_cps_b_regs[m_layer_enable_reg / 2] = data;
	}
}


void cps1bl_5205_state::captcommb2(machine_config &config)
{
	// xtals: 30MHz, 24MHz, 400KHz
	M68000(config, m_maincpu, 24000000 / 2);   // 12MHz measured on pcb
	m_maincpu->set_addrmap(AS_PROGRAM, &cps1bl_5205_state::captcommb2_map);
	m_maincpu->set_vblank_int("screen", FUNC(cps1bl_5205_state::cps1_interrupt));
	m_maincpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &cps1bl_5205_state::cpu_space_map);

	Z80(config, m_audiocpu, 30000000 / 8);  // 3.75MHz measured on pcb
	m_audiocpu->set_addrmap(AS_PROGRAM, &cps1bl_5205_state::captcommb2_z80map);

	MCFG_MACHINE_START_OVERRIDE(cps1bl_5205_state, captcommb2)
	MCFG_MACHINE_RESET_OVERRIDE(cps1bl_5205_state, captcommb2)

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(CPS_PIXEL_CLOCK, CPS_HTOTAL, CPS_HBEND, CPS_HBSTART, CPS_VTOTAL, CPS_VBEND, CPS_VBSTART);
	m_screen->set_screen_update(FUNC(cps1bl_5205_state::screen_update_fcrash));
	m_screen->screen_vblank().set(FUNC(cps1bl_5205_state::screen_vblank_cps1));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_cps1);
	PALETTE(config, m_palette, palette_device::BLACK).set_entries(0xc00);

	SPEAKER(config, "mono").front_center();
	GENERIC_LATCH_8(config, m_soundlatch);

	ym2151_device &ym2151(YM2151(config, "2151", 30000000 / 8));  // 3.75MHz measured on pcb
	// IRQ pin not used
	ym2151.add_route(0, "mono", 0.35);
	ym2151.add_route(1, "mono", 0.35);

	LS157(config, m_msm_mux[0], 0);
	m_msm_mux[0]->out_callback().set("msm1", FUNC(msm5205_device::data_w));

	LS157(config, m_msm_mux[1], 0);
	m_msm_mux[1]->out_callback().set("msm2", FUNC(msm5205_device::data_w));

	MSM5205(config, m_msm_1, 400000);  // 400kHz measured on pcb
	m_msm_1->vck_callback().set(FUNC(cps1bl_5205_state::captcommb2_mux_select_w));
	m_msm_1->vck_callback().append(m_msm_2, FUNC(msm5205_device::vclk_w));
	m_msm_1->set_prescaler_selector(msm5205_device::S96_4B);
	m_msm_1->add_route(ALL_OUTPUTS, "mono", 0.25);

	MSM5205(config, m_msm_2, 400000);
	m_msm_2->set_prescaler_selector(msm5205_device::SEX_4B);
	m_msm_2->add_route(ALL_OUTPUTS, "mono", 0.25);
}

void cps1bl_5205_state::knightsb(machine_config &config)
{
	captcommb2(config);
	m_msm_1->reset_routes().add_route(ALL_OUTPUTS, "mono", 0.5);
	m_msm_2->reset_routes().add_route(ALL_OUTPUTS, "mono", 0.5);
}

void cps1bl_5205_state::sf2b(machine_config &config)
{
	sf2mdt(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &cps1bl_5205_state::sf2b_map);
}

void cps1bl_5205_state::sf2mdt(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 12000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &cps1bl_5205_state::sf2mdt_map);
	m_maincpu->set_vblank_int("screen", FUNC(cps1bl_5205_state::irq4_line_hold)); /* triggers the sprite ram and scroll writes */

	Z80(config, m_audiocpu, 3750000);
	m_audiocpu->set_addrmap(AS_PROGRAM, &cps1bl_5205_state::captcommb2_z80map);

	MCFG_MACHINE_START_OVERRIDE(cps1bl_5205_state, sf2mdt)
	MCFG_MACHINE_RESET_OVERRIDE(cps1bl_5205_state, captcommb2)

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(CPS_PIXEL_CLOCK, CPS_HTOTAL, CPS_HBEND, CPS_HBSTART, CPS_VTOTAL, CPS_VBEND, CPS_VBSTART);
	m_screen->set_screen_update(FUNC(cps1bl_5205_state::screen_update_fcrash));
	m_screen->screen_vblank().set(FUNC(cps1bl_5205_state::screen_vblank_cps1));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_cps1);
	PALETTE(config, m_palette, palette_device::BLACK).set_entries(4096);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	YM2151(config, "2151", 3750000).add_route(0, "mono", 0.35).add_route(1, "mono", 0.35);

	LS157(config, m_msm_mux[0], 0);
	m_msm_mux[0]->out_callback().set("msm1", FUNC(msm5205_device::data_w));

	LS157(config, m_msm_mux[1], 0);
	m_msm_mux[1]->out_callback().set("msm2", FUNC(msm5205_device::data_w));

	MSM5205(config, m_msm_1, 400000);  // 400kHz ?
	m_msm_1->vck_callback().set(FUNC(cps1bl_5205_state::captcommb2_mux_select_w));
	m_msm_1->vck_callback().append(m_msm_2, FUNC(msm5205_device::vclk_w));
	m_msm_1->set_prescaler_selector(msm5205_device::S96_4B);
	m_msm_1->add_route(ALL_OUTPUTS, "mono", 0.25);

	MSM5205(config, m_msm_2, 400000);
	m_msm_2->set_prescaler_selector(msm5205_device::SEX_4B);
	m_msm_2->add_route(ALL_OUTPUTS, "mono", 0.25);
}


void cps1bl_5205_state::captcommb2_map(address_map &map)
{
	map(0x000000, 0x3fffff).rom();
	map(0x800000, 0x800001).portr("IN1");
	map(0x800002, 0x800003).portr("IN2"); // player 3 + 4 inputs
	map(0x800004, 0x800005).nopw();       // writes 00 here
	map(0x800006, 0x800007).w(FUNC(cps1bl_5205_state::captcommb2_soundlatch_w));
	map(0x800018, 0x80001f).r(FUNC(cps1bl_5205_state::cps1_dsw_r));
	map(0x800030, 0x800031).nopw();       // coinctrl
	map(0x800100, 0x80013f).ram().share("cps_a_regs");
	map(0x800140, 0x80017f).ram().share("cps_b_regs");
	map(0x800180, 0x800181).nopw();       // original sound latch, not used
	map(0x880000, 0x880001).nopw();       // ?
	map(0x900000, 0x92ffff).ram().w(FUNC(cps1bl_5205_state::cps1_gfxram_w)).share("gfxram");
	map(0x980000, 0x980023).w(FUNC(cps1bl_5205_state::captcommb2_layer_w));
	//  0x990000, 0x993fff  spriteram
	//  0x990000, 0x990001  sprite buffer flip
	//  0x991000, 0x9917ff  sprite buffer #1
	//  0x993000, 0x9937ff  sprite buffer #2
	map(0xff0000, 0xffffff).ram().share("mainram");
}

void cps1bl_5205_state::sf2b_map(address_map &map)
{
	map(0x000000, 0x3fffff).rom();
	map(0x708100, 0x7081ff).w(FUNC(cps1bl_5205_state::sf2b_layer_w));
	map(0x70c000, 0x70c001).portr("IN1");
	map(0x70c008, 0x70c009).portr("IN2");
	map(0x70c018, 0x70c01f).r(FUNC(cps1bl_5205_state::cps1_hack_dsw_r));
	map(0x70c106, 0x70c107).w(FUNC(cps1bl_5205_state::sf2mdt_soundlatch_w));
	map(0x70d000, 0x70d001).nopw(); // writes FFFF
	//map(0x800030, 0x800031).w(FUNC(cps1bl_5205_state::cps1_coinctrl_w));
	map(0x800100, 0x80013f).ram().share("cps_a_regs");  /* CPS-A custom */
	map(0x800140, 0x80017f).rw(FUNC(cps1bl_5205_state::cps1_cps_b_r), FUNC(cps1bl_5205_state::cps1_cps_b_w)).share("cps_b_regs");  /* CPS-B custom */
	map(0x900000, 0x92ffff).ram().w(FUNC(cps1bl_5205_state::cps1_gfxram_w)).share("gfxram");
	map(0xff0000, 0xffffff).ram().share("mainram");
}

void cps1bl_5205_state::sf2mdt_map(address_map &map)
{
	map(0x000000, 0x3fffff).rom();
	map(0x708100, 0x7081ff).w(FUNC(cps1bl_5205_state::sf2mdta_layer_w));
	map(0x70c000, 0x70c001).portr("IN1");
	map(0x70c008, 0x70c009).portr("IN2");
	map(0x70c018, 0x70c01f).r(FUNC(cps1bl_5205_state::cps1_hack_dsw_r));
	map(0x70c106, 0x70c107).w(FUNC(cps1bl_5205_state::sf2mdt_soundlatch_w));
	map(0x70d000, 0x70d001).nopw(); // writes FFFF
	//map(0x800030, 0x800031).w(FUNC(cps1bl_5205_state::cps1_coinctrl_w));
	map(0x800100, 0x80013f).ram().share("cps_a_regs");  /* CPS-A custom */
	map(0x800140, 0x80017f).ram().share("cps_b_regs");  /* CPS-B custom */
	map(0x900000, 0x92ffff).ram().w(FUNC(cps1bl_5205_state::cps1_gfxram_w)).share("gfxram");
	map(0xff0000, 0xffffff).ram().share("mainram");
}

/*
 *  z80 mapper IC7 GAL16V8 + IC19 74LS138
 *  138 pin 15  D000-D7FF R/W   ram
 *  138 pin 14  D800-DBFF R/W   ym
 *  138 pin 13  -               ?  pin not used
 *  138 pin 12  DC00-DFFF R/W   read sound latch, clear /int
 *  138 pin 11  E800-EBFF W     slave 5205
 *  138 pin 10  E000-E3FF W     latch bank and 5202 reset lines
 *  138 pin 9   E400-E7FF W     master 5205
 *  138 pin 7   EC00-EFFF W     ?  pin not used
 *
 *  gal pin 15  0000-BFFF R     rom
 */
void cps1bl_5205_state::captcommb2_z80map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr("bank1");
	map(0xd000, 0xd7ff).ram();
	map(0xd800, 0xd801).rw("2151", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xdc00, 0xdc00).r(FUNC(cps1bl_5205_state::captcommb2_soundlatch_r));   // clear /int here
	map(0xe000, 0xe000).w(FUNC(cps1bl_5205_state::captcommb2_snd_bankswitch_w));
	map(0xe400, 0xe400).w("msm_mux1", FUNC(ls157_device::ba_w));      // latch data for mux 1
	map(0xe800, 0xe800).w("msm_mux2", FUNC(ls157_device::ba_w));      // latch data for mux 2
}


MACHINE_START_MEMBER(cps1bl_5205_state, captcommb2)
{
	membank("bank1")->configure_entries(0, 16, memregion("audiocpu")->base() + 0x10000, 0x4000);

	m_layer_enable_reg = 0x28;
	m_layer_mask_reg[0] = 0x26;
	m_layer_mask_reg[1] = 0x24;
	m_layer_mask_reg[2] = 0x22;
	m_layer_mask_reg[3] = 0x20;
	m_layer_scroll1x_offset = 0x3e;
	m_layer_scroll2x_offset = 0x3c;
	m_layer_scroll3x_offset = 0x40;
	//m_sprite_base = 0x1000;
	m_sprite_list_end_marker = 0x8000;
	m_sprite_x_offset = 0;

	save_item(NAME(m_captcommb2_mux_toggle));
}

MACHINE_RESET_MEMBER(cps1bl_5205_state, captcommb2)
{
	m_captcommb2_mux_toggle = 0;
}

MACHINE_START_MEMBER(cps1bl_5205_state, sf2mdt)
{
	membank("bank1")->configure_entries(0, 8, memregion("audiocpu")->base() + 0x10000, 0x4000);

	m_layer_enable_reg = 0x26;
	m_layer_mask_reg[0] = 0x28;
	m_layer_mask_reg[1] = 0x2a;
	m_layer_mask_reg[2] = 0x2c;
	m_layer_mask_reg[3] = 0x2e;
	m_layer_scroll1x_offset = 0;
	m_layer_scroll2x_offset = 0;
	m_layer_scroll3x_offset = 0;
	m_sprite_base = 0x1000;
	m_sprite_list_end_marker = 0x8000;
	m_sprite_x_offset = 2;

	save_item(NAME(m_captcommb2_mux_toggle));
}


void cps1bl_5205_state::init_captcommb2()
{
	// gfx data bits 2 and 4 swapped
	uint8_t *gfx = memregion("gfx")->base();
	for (int i = 0; i < 0x400000; i++)
	{
		uint8_t x = gfx[i];
		gfx[i] = bitswap(x, 7, 6 ,5, 2, 3, 4, 1, 0);
	}

	init_mtwinsb();

	// patch - fix invisible test screen at start
	uint8_t *rom = memregion("maincpu")->base();
	rom[0x65c] = 0x68;
	rom[0x7b0] = 0x68;
}

void cps1bl_5205_state::init_knightsb()
{
	m_maincpu->space(AS_PROGRAM).unmap_write(0x980000, 0x980023);
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x980000, 0x980025, write16sm_delegate(*this, FUNC(cps1bl_5205_state::knightsb_layer_w)));

	init_mtwinsb();
}

void cps1bl_5205_state::init_sf2b()
{
	/* extra sprite ram */
	/* both of these need to be mapped - see the "Magic Delta Turbo" text on the title screen */
	m_bootleg_sprite_ram = std::make_unique<uint16_t[]>(0x2000);
	m_maincpu->space(AS_PROGRAM).install_ram(0x700000, 0x703fff, m_bootleg_sprite_ram.get());
	m_maincpu->space(AS_PROGRAM).install_ram(0x704000, 0x707fff, m_bootleg_sprite_ram.get());

	init_cps1();
}

void cps1bl_5205_state::init_sf2mdt()
{
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x708100, 0x7081ff, write16sm_delegate(*this, FUNC(cps1bl_5205_state::sf2mdt_layer_w)));

	/* extra work ram */
	m_bootleg_work_ram = std::make_unique<uint16_t[]>(0x8000);
	m_maincpu->space(AS_PROGRAM).install_ram(0xfc0000, 0xfcffff, m_bootleg_work_ram.get());

	init_sf2mdtb();
}

void cps1bl_5205_state::init_sf2mdta()
{
	/* extra work ram */
	m_bootleg_work_ram = std::make_unique<uint16_t[]>(0x8000);
	m_maincpu->space(AS_PROGRAM).install_ram(0xfc0000, 0xfcffff, m_bootleg_work_ram.get());

	init_sf2b();
}

void cps1bl_5205_state::init_sf2mdtb()
{
	uint32_t gfx_size = memregion( "gfx" )->bytes();
	uint8_t *rom = memregion( "gfx" )->base();
	for (int i = 0; i < gfx_size; i += 8)
	{
		uint8_t tmp = rom[i + 1];
		rom[i + 1] = rom[i + 4];
		rom[i + 4] = tmp;
		tmp = rom[i + 3];
		rom[i + 3] = rom[i + 6];
		rom[i + 6] = tmp;
	}

	init_sf2b();
}


#define CPS1_COINAGE_1 \
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) ) \
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) ) \
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) ) \
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) ) \
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) ) \
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) ) \
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) ) \
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_6C ) ) \
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) ) \
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) ) \
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) ) \
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) ) \
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) ) \
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) ) \
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) ) \
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_6C ) )

#define CPS1_DIFFICULTY_1(diploc) \
	PORT_DIPNAME( 0x07, 0x04, DEF_STR( Difficulty ) ) PORT_DIPLOCATION(diploc ":1,2,3") \
	PORT_DIPSETTING(    0x07, "1 (Easiest)" ) \
	PORT_DIPSETTING(    0x06, "2" ) \
	PORT_DIPSETTING(    0x05, "3" ) \
	PORT_DIPSETTING(    0x04, "4 (Normal)" ) \
	PORT_DIPSETTING(    0x03, "5" ) \
	PORT_DIPSETTING(    0x02, "6" ) \
	PORT_DIPSETTING(    0x01, "7" ) \
	PORT_DIPSETTING(    0x00, "8 (Hardest)" )

static INPUT_PORTS_START( captcommb2 )
	PORT_INCLUDE(captcomm)

	PORT_MODIFY("IN3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("IN2")  // Player 4
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START4 )
INPUT_PORTS_END

static INPUT_PORTS_START( sf2mdt )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Jab Punch") PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Strong Punch") PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 Fierce Punch") PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P2 Jab Punch") PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P2 Strong Punch") PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P2 Fierce Punch") PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")      /* Extra buttons */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P1 Short Kick") PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("P1 Forward Kick") PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("P1 Roundhouse Kick") PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P2 Short Kick") PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("P2 Forward Kick") PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("P2 Roundhouse Kick") PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSWA")
	CPS1_COINAGE_1
	PORT_DIPNAME( 0x40, 0x40, "2 Coins to Start, 1 to Continue" )   PORT_DIPLOCATION("SW(A):7")
	PORT_DIPSETTING( 0x40, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW(A):8" )

	PORT_START("DSWB")
	CPS1_DIFFICULTY_1( "SW(B)" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW(B):4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW(B):5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW(B):6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW(B):7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW(B):8" )

	PORT_START("DSWC")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW(C):1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW(C):2" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )                PORT_DIPLOCATION("SW(C):3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Freeze" )                            PORT_DIPLOCATION("SW(C):4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )              PORT_DIPLOCATION("SW(C):5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )              PORT_DIPLOCATION("SW(C):6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Allow_Continue ) )           PORT_DIPLOCATION("SW(C):7")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Game Mode")                          PORT_DIPLOCATION("SW(C):8")
	PORT_DIPSETTING(    0x80, "Game" )
	PORT_DIPSETTING(    0x00, DEF_STR( Test ) )
INPUT_PORTS_END

static INPUT_PORTS_START( sf2mdtb )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Jab Punch") PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Strong Punch") PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 Fierce Punch") PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P2 Jab Punch") PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P2 Strong Punch") PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P2 Fierce Punch") PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")      /* Extra buttons */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P1 Short Kick") PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("P1 Forward Kick") PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("P1 Roundhouse Kick") PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P2 Short Kick") PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("P2 Forward Kick") PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("P2 Roundhouse Kick") PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSWA")
	CPS1_COINAGE_1
	PORT_DIPNAME( 0x40, 0x40, "2 Coins to Start, 1 to Continue" )   PORT_DIPLOCATION("SW(A):7")
	PORT_DIPSETTING( 0x40, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	// debug mode? depending on other DSW setting get different "game" mode, autoplay, bonus round, etc...
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW(A):8" )

	PORT_START("DSWB")
	CPS1_DIFFICULTY_1("SW(B)")
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW(B):4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW(B):5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW(B):6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW(B):7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW(B):8" )

	PORT_START("DSWC")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW(C):1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW(C):2" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )                PORT_DIPLOCATION("SW(C):3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Freeze" )                            PORT_DIPLOCATION("SW(C):4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )              PORT_DIPLOCATION("SW(C):5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )              PORT_DIPLOCATION("SW(C):6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Allow_Continue ) )           PORT_DIPLOCATION("SW(C):7")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Game Mode")                          PORT_DIPLOCATION("SW(C):8")
	PORT_DIPSETTING(    0x80, "Game" )
	PORT_DIPSETTING(    0x00, DEF_STR( Test ) )
INPUT_PORTS_END


void captcommb2_state::bootleg_render_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int pos;
	int last_sprite_offset = 0;
	uint16_t tileno, colour, xpos, ypos;
	bool flipx, flipy;
	uint16_t *sprite_ram = m_bootleg_sprite_ram.get();
	int base = (sprite_ram[0] ? 0x3000 : 0x1000) / 2;  // writes sprite buffer flip here instead of obj_base register

	// end of sprite table marker is 0x8000
	// 1st sprite always 0x100e/0x300e
	// sprites are: [ypos][tile#][color][xpos]
	// no block sprites
	for (pos = base + 7; pos < base + 0x400; pos += 4)
		if (sprite_ram[pos] == m_sprite_list_end_marker)
		{
			last_sprite_offset = pos - 3;
			break;
		}

	for (pos = last_sprite_offset - base; pos >= 0; pos -= 4)
	{
		tileno = sprite_ram[base + pos] & 0x7fff;      // see below
		xpos   = sprite_ram[base + pos + 2] & 0x1ff;
		ypos   = sprite_ram[base + pos - 1] & 0x1ff;
		flipx  = BIT(sprite_ram[base + pos + 1], 5);
		flipy  = BIT(sprite_ram[base + pos + 1], 6);
		colour = sprite_ram[base + pos + 1] & 0x1f;
		ypos   = 256 - ypos - 16;
		xpos   = xpos + m_sprite_x_offset + 49;

		if (flip_screen())
			m_gfxdecode->gfx(2)->prio_transpen(bitmap, cliprect, tileno, colour, !flipx, !flipy, 512-16-xpos, 256-16-ypos, screen.priority(), 2, 15);
		else
			m_gfxdecode->gfx(2)->prio_transpen(bitmap, cliprect, tileno, colour, flipx, flipy, xpos, ypos, screen.priority(), 2, 15);
	}

	/* tileno note:
	    sets the unused msb for certain tiles eg. middle parts of rocket launcher weapon,
	    this means the tile is out of range and therefore transparent,
	    most likely just a bug and the real h/w ignores the unused bit so the effect is not seen.
	*/
}


// ************************************************************************* CAPTCOMMB2

/*
    Captain Commando:

    h/w issues compared to original game (captcomm)
    -----------------------------------------------
    these are present on the real board so are not emulation issues:

    * End sequence row scroll effect doesn't work.
    * Capcom copyright text missing on title screen, deliberately shifted down out of visible area by bootleggers.
    * Capcom logo missing from end sequence, as above.
    * Sprite flickering effects eg. when character has invincibility, look a little different to original.
    * Certain static sprites wobble vertically just a pixel or two  eg. manhole covers, breakable oil drums etc.

    these are present on the real board but are unintentionally "fixed" in emulation:

    * All '0' characters are missing in test menu eg. sound test, input test etc.
    * Wrong tile displayed when character select count-down timer reaches zero (superscript '1' with white bar underneath)
*/
ROM_START( captcommb2 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 ) // = captcommr1 + additional code mapped on top
	ROM_LOAD16_BYTE( "5.bin", 0x000000, 0x80000, CRC(c3a6ed28) SHA1(f79fed35f7b0dc383837a2ead846acc686dd3487) )
	ROM_LOAD16_BYTE( "3.bin", 0x000001, 0x80000, CRC(28729335) SHA1(6dd23c2d41e4e182434fe80c03d5c90785e6c0ce) )
	ROM_LOAD16_BYTE( "4.bin", 0x100000, 0x20000, CRC(1b526d73) SHA1(3dd8dec61db4f4f5546937602a8fb01c639d72f8) )
	ROM_CONTINUE(             0x000000, 0x04000)
	ROM_CONTINUE(             0x018000, 0x04000)
	ROM_IGNORE(                         0x18000)
	ROM_LOAD16_BYTE( "2.bin", 0x100001, 0x20000, CRC(73c99709) SHA1(e122e3771b698c44fb998589af0542b1f2a3876a) )
	ROM_CONTINUE(             0x000001, 0x04000)
	ROM_CONTINUE(             0x018001, 0x04000)
	ROM_IGNORE(                         0x18000)

	ROM_REGION( 0x400000, "gfx", 0 ) // some data bits are swapped, see init()
	ROM_LOAD64_BYTE( "bnh-01.bin", 0x000000, 0x40000, CRC(ffbc3bdd) SHA1(fcee1befd8279d41a81689394a562e2344191e2a) )
	ROM_CONTINUE(                  0x000004, 0x40000)
	ROM_LOAD64_BYTE( "bnh-02.bin", 0x000001, 0x40000, CRC(40e58d52) SHA1(d980d075f4feeaf95ad599e1b95a1b550f6a85d9) )
	ROM_CONTINUE(                  0x000005, 0x40000)
	ROM_LOAD64_BYTE( "bnh-03.bin", 0x000002, 0x40000, CRC(58f92cad) SHA1(041cd7d7d325147eefab245cd0610203200be1ce) )
	ROM_CONTINUE(                  0x000006, 0x40000)
	ROM_LOAD64_BYTE( "bnh-04.bin", 0x000003, 0x40000, CRC(284eea8a) SHA1(b95cf797b3576d7d62f58d4a70d4b6e64ece7601) )
	ROM_CONTINUE(                  0x000007, 0x40000)
	ROM_LOAD64_BYTE( "bnh-05.bin", 0x200000, 0x40000, CRC(d02719b7) SHA1(c67bc53c22030c7a75f2fdde1480f619e2be314c) )
	ROM_CONTINUE(                  0x200004, 0x40000)
	ROM_LOAD64_BYTE( "bnh-06.bin", 0x200001, 0x40000, CRC(d9d43b55) SHA1(db462900958e06610cfdc47bb774f37ea1c0a1b7) )
	ROM_CONTINUE(                  0x200005, 0x40000)
	ROM_LOAD64_BYTE( "bnh-07.bin", 0x200002, 0x40000, CRC(03b7900d) SHA1(ade31f4b37e8ca50214c5b32a2e5899043f49c8a) )
	ROM_CONTINUE(                  0x200006, 0x40000)
	ROM_LOAD64_BYTE( "bnh-08.bin", 0x200003, 0x40000, CRC(327b8da8) SHA1(4bcc6fd637d382ce35b9387568c53d89a55e8ed2) )
	ROM_CONTINUE(                  0x200007, 0x40000)

	ROM_REGION( 0x50000, "audiocpu", 0 )
	ROM_LOAD( "1.bin", 0x00000, 0x40000, CRC(aed2f4bd) SHA1(3bd567dc350bf6ac3a349548790ad49eb5bd8307) )
	ROM_RELOAD(        0x10000, 0x40000 )

	/* pld devices */
	ROM_REGION( 0xc00, "plds", 0 )
	ROM_LOAD( "1_gal20v8.ic169", 0x000, 0x157, CRC(e5cf9f53) SHA1(9c5f88d2dcfaab1c1ba80019e9ec60f0c1fb8c49) )  // all unsecured
	ROM_LOAD( "2_gal16v8.ic7",   0x200, 0x117, CRC(0ebc7cd7) SHA1(95f0cba1f588920634b5f09b85197474201d9aed) )
	ROM_LOAD( "3_gal16v8.ic72",  0x400, 0x117, CRC(ebf1f643) SHA1(d2d1d60ced5665469589122001d9b7b36b7444d2) )
	ROM_LOAD( "4_gal16v8.ic80",  0x600, 0x117, CRC(2c43c330) SHA1(7df648b63bc0c845a579a29722004f28d3e04a10) )
	ROM_LOAD( "5_gal20v8.ic121", 0x800, 0x157, CRC(76fa8969) SHA1(8fce620ece2d70511868c9cdd8c421762a467c11) )
	ROM_LOAD( "6_gal20v8.ic120", 0xa00, 0x157, CRC(6a55a974) SHA1(f4df5a45409eca84c1ac36f7f8dbb218b79b57ac) )
	// ic116  tpc1020afn-084c  no dump
ROM_END


// ************************************************************************* KNIGHTSB, KNIGHTSB3

/*
    CPU
    1x MC68000P12 ic65 main
    1x Z0840006PSC ic1 sound
    1x YM2151 ic29 sound
    1x YM3012 ic30 sound
    2x LM324 ic15,ic31 sound
    2x M5205 ic184,ic185 sound
    1x TDA2003 ic14 sound
    1x oscillator 24.000000MHz (close to main)
    1x oscillator 29.821000MHz (close to sound)

    ROMs
    5x M27C2001 1,2,3,4,5 dumped
    4x mask ROM KA,KB,KC,KD not dumped

    RAMs
    4x KM62256ALP ic112,ic113,ic168,ic170
    1x SYC6116L ic24
    1x MCM2018AN ic7,ic8,ic51,ic56,ic70,ic71,ic77,ic78

    PLDs
    1x TPC1020AFN ic116 read protected
    3x GAL20V8A ic120,ic121,ic169 read protected
    3x GAL16V8A ic7,ic72,ic80 read protected

    Note
    1x JAMMA edge connector
    2x 10 legs connector
    1x trimmer (volume)
    3x 8x2 switches DIP

    FIXME - graphics ROMs are wrong, copied from the other version
    ROMs missing are KA.IC91 KB.IC92 KC.IC93 KD.IC94
*/
ROM_START( knightsb )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "3.ic173",    0x00001, 0x40000, CRC(c9c6e720) SHA1(e8a1cd73458b548e88fc49d8f659e0dc33a8e756) )
	ROM_LOAD16_BYTE( "5.ic172",    0x00000, 0x40000, CRC(7fd91118) SHA1(d2832b21309a467938891946d7af35d8095787a4) )
	ROM_LOAD16_BYTE( "2.ic175",    0x80001, 0x40000, CRC(1eb91343) SHA1(e02cfbbd7689346f14f2e3455ed17e7f0b51bad0) )
	ROM_LOAD16_BYTE( "4.ic176",    0x80000, 0x40000, CRC(af352703) SHA1(7855ac65752203f45af4ef41af8c291540a1c8a8) )

	ROM_REGION( 0x400000, "gfx", 0 ) /* bootleg had 4x 1meg mask ROMs, these need dumping so that the format is known */
	ROM_LOAD64_WORD( "kr_gfx1.rom",  0x000000, 0x80000, BAD_DUMP CRC(9e36c1a4) SHA1(772daae74e119371dfb76fde9775bda78a8ba125) )
	ROM_LOAD64_WORD( "kr_gfx3.rom",  0x000002, 0x80000, BAD_DUMP CRC(c5832cae) SHA1(a188cf401cd3a2909b377d3059f14d22ec3b0643) )
	ROM_LOAD64_WORD( "kr_gfx2.rom",  0x000004, 0x80000, BAD_DUMP CRC(f095be2d) SHA1(0427d1574062f277a9d04440019d5638b05de561) )
	ROM_LOAD64_WORD( "kr_gfx4.rom",  0x000006, 0x80000, BAD_DUMP CRC(179dfd96) SHA1(b1844e69da7ab13474da569978d5b47deb8eb2be) )
	ROM_LOAD64_WORD( "kr_gfx5.rom",  0x200000, 0x80000, BAD_DUMP CRC(1f4298d2) SHA1(4b162a7f649b0bcd676f8ca0c5eee9a1250d6452) )
	ROM_LOAD64_WORD( "kr_gfx7.rom",  0x200002, 0x80000, BAD_DUMP CRC(37fa8751) SHA1(b88b39d1f08621f15a5620095aef998346fa9891) )
	ROM_LOAD64_WORD( "kr_gfx6.rom",  0x200004, 0x80000, BAD_DUMP CRC(0200bc3d) SHA1(c900b1be2b4e49b951e5c1e3fd1e19d21b82986e) )
	ROM_LOAD64_WORD( "kr_gfx8.rom",  0x200006, 0x80000, BAD_DUMP CRC(0bb2b4e7) SHA1(983b800925d58e4aeb4e5105f93ed5faf66d009c) )

	ROM_REGION( 0x50000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "1.ic26",     0x00000, 0x40000, CRC(bd6f9cc1) SHA1(9f33cccef224d2204736a9eae761196866bd6e41) )
	ROM_RELOAD(            0x10000, 0x40000 )
ROM_END

/*
    Knights of the Round
    pcb marking: ORD 92032
    Very similar to knightsb set:
     maincpu roms are just 1 byte different, vector 1 (stack pointer init) is ff80d6 instead of ff81d6
     knightsb gfx roms are 4x 1MB (but not dumped), these are 8x 512KB (suspect data is same)
    Some sound samples are very quiet on real pcb
    Confirmed clocks (measured) are same as captcommb2:
     xtals: 30MHz, 24MHz, 400KHz
     68k = 12MHz (P10 model, overclocked)
     z80/ym = 3.75MHz
     5202 = 400KHz
*/
ROM_START( knightsb3 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )
	ROM_LOAD16_BYTE( "5.bin", 0x00000, 0x80000, CRC(b818272c) SHA1(680b1539bbeebf26706c9367decce2a8de0144e4) )  // 27c040
	ROM_LOAD16_BYTE( "3.bin", 0x00001, 0x80000, CRC(b0b9a4c2) SHA1(7d49b260224756303f9c6cdb67e8c531b0f5689f) )  // 27c040

	ROM_REGION( 0x400000, "gfx", 0 ) // = knights but arranged differently
	ROM_LOAD64_BYTE( "svr-01.bin", 0x000000, 0x40000, CRC(b08dc61f) SHA1(9527636ba0ccc7f02db6ba7013e932582ff85a93) )
	ROM_CONTINUE(                  0x000004, 0x40000)
	ROM_LOAD64_BYTE( "svr-02.bin", 0x000001, 0x40000, CRC(cca262aa) SHA1(587b25a724a89095299bd1f655d833d26a420c30) )
	ROM_CONTINUE(                  0x000005, 0x40000)
	ROM_LOAD64_BYTE( "svr-03.bin", 0x000002, 0x40000, CRC(1fe7056c) SHA1(eb9e5955c6cf2cfef565672cd0efcfd6921fefc3) )
	ROM_CONTINUE(                  0x000006, 0x40000)
	ROM_LOAD64_BYTE( "svr-04.bin", 0x000003, 0x40000, CRC(b29ce7cf) SHA1(d8f99c57561c60bec260c6b5daef81ba7856b547) )
	ROM_CONTINUE(                  0x000007, 0x40000)
	ROM_LOAD64_BYTE( "svr-05.bin", 0x200000, 0x40000, CRC(1c774671) SHA1(d553b87e8a0f13f404cff64089847325a18d1afb) )
	ROM_CONTINUE(                  0x200004, 0x40000)
	ROM_LOAD64_BYTE( "svr-06.bin", 0x200001, 0x40000, CRC(05463aa3) SHA1(27cc2724e22bf74e972283d6c35d31cea2c1a943) )
	ROM_CONTINUE(                  0x200005, 0x40000)
	ROM_LOAD64_BYTE( "svr-07.bin", 0x200002, 0x40000, CRC(87944aaa) SHA1(57d4637d5cf10b9cef95e12c64362c04a604cf64) )
	ROM_CONTINUE(                  0x200006, 0x40000)
	ROM_LOAD64_BYTE( "svr-08.bin", 0x200003, 0x40000, CRC(aa9d82fb) SHA1(41ff75bc0cc3766c19d79080893b52d9c759a443) )
	ROM_CONTINUE(                  0x200007, 0x40000)

	// TODO: dump
	ROM_REGION( 0x50000, "audiocpu", 0 )
	ROM_LOAD( "1.ic26", 0x00000, 0x40000, CRC(bd6f9cc1) SHA1(9f33cccef224d2204736a9eae761196866bd6e41) )  // knightsb
	ROM_RELOAD( 0x10000, 0x40000 )

	/* pld devices:
	     ________________________
	    |                   4    |      (no component reference markings on pcb)
	    |                        |
	  ==       2    3            |
	  ==                    7    |
	  ==                         |
	  ==                         |
	  ==                         |
	    |   1            5    6  |
	    |________________________|

	#1   palce20v8h     secured, bruteforce ok
	#2   palce16v8h     secured, bruteforce ok
	#3   palce16v8h     secured, registered
	#4   palce16v8h     secured, registered
	#5   palce20v8h     secured, registered
	#6   palce20v8h     secured, registered
	#7   a1020a pl84c   unattempted
	*/
	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "1_palce20v8.bin", 0x000, 0x157, CRC(a5078c38) SHA1(59558a514ec60cd7148ede78a5641f5e6c0479c8) )
	ROM_LOAD( "2_palce16v8.bin", 0x200, 0x117, CRC(bad3316b) SHA1(b25141540fbaab028ba563f4fe1796b6039a4d59) )
ROM_END


// ************************************************************************* SF2B, SF2B2

ROM_START( sf2b )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "pf1-2-sg076.bin", 0x000000, 0x100000, CRC(1d15bc7a) SHA1(834627545f191f39de6beb008c89623f2b88c13b) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROM_LOAD32_WORD( "pf4-sg072.bin", 0x000000, 0x100000, CRC(16289710) SHA1(4f3236712b979a1eb2fa97740e32d7913cee0d0d) )
	ROM_LOAD32_WORD( "pf7-sg103.bin", 0x000002, 0x100000, CRC(fb78022e) SHA1(b8974387056dd52db96b01cc4648edc814398c7e) )
	ROM_LOAD32_WORD( "pf5-sg095.bin", 0x200000, 0x100000, CRC(0a6be48b) SHA1(b7e72c94d4e3eb4a6bba6608d9b9a093c8901ad9) )
	ROM_LOAD32_WORD( "pf8-sg101.bin", 0x200002, 0x100000, CRC(6258c7cf) SHA1(4cd7519245c0aa816934a43e6743160f715d7dc2) )
	ROM_LOAD32_WORD( "pf6-sg068.bin", 0x400000, 0x100000, CRC(9b5b09d7) SHA1(698a6aab41e495bd0c37a19aee16a84f04d15797) )
	ROM_LOAD32_WORD( "pf9-sh001.bin", 0x400002, 0x100000, CRC(9f25090e) SHA1(12ff0431ef6550db446985c8914ac7d78eec6b6d) )

	ROM_REGION( 0x30000, "audiocpu", 0 ) /* Sound program + samples  */
	ROM_LOAD( "3snd.ic28", 0x00000, 0x20000, CRC(d5bee9cc) SHA1(e638cb5ce7a22c18b60296a7defe8b03418da56c) )
	ROM_RELOAD(            0x10000, 0x20000 )
ROM_END

// this PCB has stickers in Spanish. It's extremely similar to sf2b, but audiocpu ROM is identical to sf2mdt and 11.bin is slightly different.
ROM_START( sf2b2 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "2.bin", 0x000000, 0x80000, CRC(42809e5a) SHA1(ee91ecfce29bc50cf3f492ff646109c60bf65551) )
	ROM_LOAD16_BYTE( "1.bin", 0x000001, 0x80000, CRC(e58db26c) SHA1(da1a4e063fa770257fd3df5fdb3785c1856511a5) )

	ROM_REGION( 0x600000, "gfx", 0 ) /* rearranged in init */
	ROM_LOAD64_WORD( "5.bin",  0x000000, 0x80000, CRC(47fab9ed) SHA1(1709becbe189b21f2c1920acef96f9412eb954e2) )
	ROM_LOAD64_WORD( "8.bin",  0x000002, 0x80000, CRC(b8c39d56) SHA1(ee2939f42e95c926bdd88adf326eee02cba3f37a) )
	ROM_LOAD64_WORD( "11.bin", 0x000004, 0x80000, CRC(6e8c98d8) SHA1(fbd7d788349fd418c48aedd906c40960e41c20f1) )
	ROM_LOAD64_WORD( "14.bin", 0x000006, 0x80000, CRC(672d4f85) SHA1(511a8878d14d3fd39c9a22efb983550098ea8760) )
	ROM_LOAD64_WORD( "4.bin",  0x200000, 0x80000, CRC(69d7b06b) SHA1(b428a0b5dfdee20d4d198673fe3b0147cad2d5bd) )
	ROM_LOAD64_WORD( "7.bin",  0x200002, 0x80000, CRC(ded88f5f) SHA1(71c63fed5a15f6ce1df878dca7aa5d53868e68ee) )
	ROM_LOAD64_WORD( "10.bin", 0x200004, 0x80000, CRC(8c2fca3c) SHA1(a84399e91dbf5790c3fe003385f6d9f4bc9d3366) )
	ROM_LOAD64_WORD( "13.bin", 0x200006, 0x80000, CRC(26f09d38) SHA1(3babc4f502ea9e07f79306b1abc9c94f484f9cc1) )
	ROM_LOAD64_WORD( "6.bin",  0x400000, 0x80000, CRC(b6215991) SHA1(5e20632e1a2d6eebe3b5d314cf2549bb74d7118e) )
	ROM_LOAD64_WORD( "9.bin",  0x400002, 0x80000, CRC(b6a71ed7) SHA1(1850b4b4aa4b5cafc594b174322afefbdf215221) )
	ROM_LOAD64_WORD( "12.bin", 0x400004, 0x80000, CRC(971903fa) SHA1(849ee7200815ef73f75456e656f061f1e852af59) )
	ROM_LOAD64_WORD( "15.bin", 0x400006, 0x80000, CRC(00983914) SHA1(4ead6bbce6ca8c4cc884d55c1f821242d0e67fae) )

	ROM_REGION( 0x30000, "audiocpu", 0 ) /* Sound program + samples  */
	ROM_LOAD( "3.bin", 0x00000, 0x20000, CRC(17d5ba8a) SHA1(6ff3b8860d7e1fdee3561846f645eb4d3a8965ec) )
	ROM_RELOAD(        0x10000, 0x20000 )
ROM_END


// ************************************************************************* SF2CEB

ROM_START( sf2ceb )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "3.ic171", 0x000000, 0x80000, CRC(a2355d90) SHA1(6c9e1294c55a5a9f244f6f1ce46224c51f910bb1) )
	ROM_LOAD16_BYTE( "5.ic171", 0x000001, 0x80000, CRC(c6f86e84) SHA1(546841fe7d423fff05a7772aa57fa3274515c32b) )
	ROM_LOAD16_BYTE( "2.ic171", 0x100000, 0x20000, CRC(74844192) SHA1(99cd546c78cce7f632007af454d8a55eddb6b19b) )
	ROM_LOAD16_BYTE( "4.ic171", 0x100001, 0x20000, CRC(bd98ff15) SHA1(ed902d949b0b5c5beaaea78a4b418ffa6db9e1df) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROM_LOAD32_WORD( "pf4-sg072.ic90", 0x000000, 0x100000, CRC(446575c7) SHA1(2bd769674fbe280d304b389daf74202cf9e4ac22) )
	ROM_LOAD32_WORD( "pf7-sg103.ic88", 0x000002, 0x100000, CRC(fb78022e) SHA1(b8974387056dd52db96b01cc4648edc814398c7e) )
	ROM_LOAD32_WORD( "pf5-sg063.ic91", 0x200000, 0x100000, CRC(0a6be48b) SHA1(b7e72c94d4e3eb4a6bba6608d9b9a093c8901ad9) )
	ROM_LOAD32_WORD( "pf8-sg101.ic93", 0x200002, 0x100000, CRC(6258c7cf) SHA1(4cd7519245c0aa816934a43e6743160f715d7dc2) )
	ROM_LOAD32_WORD( "pf6-sg070.ic86", 0x400000, 0x100000, CRC(9b5b09d7) SHA1(698a6aab41e495bd0c37a19aee16a84f04d15797) )
	ROM_LOAD32_WORD( "pf9-sh001.ic84", 0x400002, 0x100000, CRC(9f25090e) SHA1(12ff0431ef6550db446985c8914ac7d78eec6b6d) )

	ROM_REGION( 0x30000, "audiocpu", 0 ) /* Sound program + samples  */
	ROM_LOAD( "3.ic28", 0x00000, 0x20000, CRC(d5bee9cc) SHA1(e638cb5ce7a22c18b60296a7defe8b03418da56c) )
	ROM_RELOAD(         0x10000, 0x20000 )
ROM_END

ROM_START( sf2ceb2 ) // sf2ceeab3 in FBNeo, all ROMs but the first two program ROMs match sf2mdt. Dump has been confirmed on 2 different PCBs
	ROM_REGION( CODE_SIZE, "maincpu", 0 )
	ROM_LOAD16_BYTE( "3.ic172", 0x000000, 0x80000, CRC(11b5fe98) SHA1(6dda11e6c443a7c0ddf17a9840c93be00a424472) )
	ROM_LOAD16_BYTE( "1.ic171", 0x000001, 0x80000, CRC(6d948623) SHA1(0bcdda9ba2ef2051ad70277fbc383035a63540f3) )
	ROM_LOAD16_BYTE( "4.ic176", 0x100000, 0x20000, CRC(1073b7b6) SHA1(81ca1eab65ceac69520584bb23a684ccb9d92f89) )
	ROM_LOAD16_BYTE( "2.ic175", 0x100001, 0x20000, CRC(924c6ce2) SHA1(676a912652bd75da5087f0c7eae047b7681a993c) )

	ROM_REGION( 0x600000, "gfx", 0 ) // rearranged in init
	ROM_LOAD64_WORD( "7.ic90",  0x000000, 0x80000, CRC(896eaf48) SHA1(5a13ae8b554e05eed3d5749aaf5845d499bce45b) )
	ROM_LOAD64_WORD( "10.ic88", 0x000002, 0x80000, CRC(ef3f5be8) SHA1(d4e1de7d7caf6977e48544d6701618ae70c717f9) )
	ROM_LOAD64_WORD( "13.ic89", 0x000004, 0x80000, CRC(305dd72a) SHA1(c373b517c23f3b019abb06e21f6b9ab6e1e47909) )
	ROM_LOAD64_WORD( "16.ic87", 0x000006, 0x80000, CRC(e57f6db9) SHA1(b37f95737804002ec0e237472eaacf0bc1e868e8) )
	ROM_LOAD64_WORD( "6.ic91",  0x200000, 0x80000, CRC(054cd5c4) SHA1(07f275e118c141a84ca15a2e9edc81694af37cf2) )
	ROM_LOAD64_WORD( "9.ic93",  0x200002, 0x80000, CRC(818ca33d) SHA1(dfb707e17c83216f8a62e905f8c7cd6d406b417b) )
	ROM_LOAD64_WORD( "12.ic92", 0x200004, 0x80000, CRC(87e069e8) SHA1(cddd3be84f8379134590bfbbb080518f28120e49) )
	ROM_LOAD64_WORD( "15.ic94", 0x200006, 0x80000, CRC(5dfb44d1) SHA1(08e44b8efc84f9cfc829aabf704155ddc700de76) )
	ROM_LOAD64_WORD( "8.ic86",  0x400000, 0x80000, CRC(34bbb3fa) SHA1(7794e89258f12b17d38c3d302dc15c502a8c8eb6) )
	ROM_LOAD64_WORD( "11.ic84", 0x400002, 0x80000, CRC(cea6d1d6) SHA1(9c953db42f0d877e43c0c239f69a00df39a18295) )
	ROM_LOAD64_WORD( "14.ic85", 0x400004, 0x80000, CRC(7d9f1a67) SHA1(6deb7fff867c42b13a32bb11eda798cfdb4cbaa8) )
	ROM_LOAD64_WORD( "17.ic83", 0x400006, 0x80000, CRC(91a9a05d) SHA1(5266ceddd2df925e79b4200843dec2f7aa9297b3) )

	ROM_REGION( 0x30000, "audiocpu", 0 ) // Sound program + samples
	ROM_LOAD( "5.ic26", 0x00000, 0x20000, CRC(17d5ba8a) SHA1(6ff3b8860d7e1fdee3561846f645eb4d3a8965ec) )
	ROM_RELOAD(         0x10000, 0x20000 )
ROM_END

ROM_START( sf2ceb3 ) // sf2ceeab4 in FBNeo, all ROMs but the first match sf2ceb2. Changes do not seem a result of bit-rot
	ROM_REGION( CODE_SIZE, "maincpu", 0 ) // main CPU has a (sic) 'Street Figter III 00325' sticker
	ROM_LOAD16_BYTE( "3.ic172", 0x000000, 0x80000, CRC(30848e16) SHA1(b48809350f033010d33666a8cd5a610f9721f994) )
	ROM_LOAD16_BYTE( "1.ic171", 0x000001, 0x80000, CRC(6d948623) SHA1(0bcdda9ba2ef2051ad70277fbc383035a63540f3) )
	ROM_LOAD16_BYTE( "4.ic176", 0x100000, 0x20000, CRC(1073b7b6) SHA1(81ca1eab65ceac69520584bb23a684ccb9d92f89) )
	ROM_LOAD16_BYTE( "2.ic175", 0x100001, 0x20000, CRC(924c6ce2) SHA1(676a912652bd75da5087f0c7eae047b7681a993c) )

	ROM_REGION( 0x600000, "gfx", 0 ) // rearranged in init
	ROM_LOAD64_WORD( "7.ic90",  0x000000, 0x80000, CRC(896eaf48) SHA1(5a13ae8b554e05eed3d5749aaf5845d499bce45b) )
	ROM_LOAD64_WORD( "10.ic88", 0x000002, 0x80000, CRC(ef3f5be8) SHA1(d4e1de7d7caf6977e48544d6701618ae70c717f9) )
	ROM_LOAD64_WORD( "13.ic89", 0x000004, 0x80000, CRC(305dd72a) SHA1(c373b517c23f3b019abb06e21f6b9ab6e1e47909) )
	ROM_LOAD64_WORD( "16.ic87", 0x000006, 0x80000, CRC(e57f6db9) SHA1(b37f95737804002ec0e237472eaacf0bc1e868e8) )
	ROM_LOAD64_WORD( "6.ic91",  0x200000, 0x80000, CRC(054cd5c4) SHA1(07f275e118c141a84ca15a2e9edc81694af37cf2) )
	ROM_LOAD64_WORD( "9.ic93",  0x200002, 0x80000, CRC(818ca33d) SHA1(dfb707e17c83216f8a62e905f8c7cd6d406b417b) )
	ROM_LOAD64_WORD( "12.ic92", 0x200004, 0x80000, CRC(87e069e8) SHA1(cddd3be84f8379134590bfbbb080518f28120e49) )
	ROM_LOAD64_WORD( "15.ic94", 0x200006, 0x80000, CRC(5dfb44d1) SHA1(08e44b8efc84f9cfc829aabf704155ddc700de76) )
	ROM_LOAD64_WORD( "8.ic86",  0x400000, 0x80000, CRC(34bbb3fa) SHA1(7794e89258f12b17d38c3d302dc15c502a8c8eb6) )
	ROM_LOAD64_WORD( "11.ic84", 0x400002, 0x80000, CRC(cea6d1d6) SHA1(9c953db42f0d877e43c0c239f69a00df39a18295) )
	ROM_LOAD64_WORD( "14.ic85", 0x400004, 0x80000, CRC(7d9f1a67) SHA1(6deb7fff867c42b13a32bb11eda798cfdb4cbaa8) )
	ROM_LOAD64_WORD( "17.ic83", 0x400006, 0x80000, CRC(91a9a05d) SHA1(5266ceddd2df925e79b4200843dec2f7aa9297b3) )

	ROM_REGION( 0x30000, "audiocpu", 0 ) // Sound program + samples
	ROM_LOAD( "5.ic26", 0x00000, 0x20000, CRC(17d5ba8a) SHA1(6ff3b8860d7e1fdee3561846f645eb4d3a8965ec) )
	ROM_RELOAD(         0x10000, 0x20000 )
ROM_END

ROM_START( sf2ceb4 ) // sf2ceeab5 in FBNeo, all ROMs but ic171 match sf2ceb2. Dump has been confirmed on 3 different PCBs
	ROM_REGION( CODE_SIZE, "maincpu", 0 )
	ROM_LOAD16_BYTE( "3.ic172", 0x000000, 0x80000, CRC(11b5fe98) SHA1(6dda11e6c443a7c0ddf17a9840c93be00a424472) )
	ROM_LOAD16_BYTE( "5.ic171", 0x000001, 0x80000, CRC(43e85f2c) SHA1(56026e5d0ba4e0fb1bc92b981f69d0fc9d7af1d2) )
	ROM_LOAD16_BYTE( "2.ic176", 0x100000, 0x20000, CRC(1073b7b6) SHA1(81ca1eab65ceac69520584bb23a684ccb9d92f89) )
	ROM_LOAD16_BYTE( "4.ic175", 0x100001, 0x20000, CRC(924c6ce2) SHA1(676a912652bd75da5087f0c7eae047b7681a993c) )

	ROM_REGION( 0x600000, "gfx", 0 ) // rearranged in init
	ROM_LOAD64_WORD( "10.ic90", 0x000000, 0x80000, CRC(896eaf48) SHA1(5a13ae8b554e05eed3d5749aaf5845d499bce45b) )
	ROM_LOAD64_WORD( "7.ic88",  0x000002, 0x80000, CRC(ef3f5be8) SHA1(d4e1de7d7caf6977e48544d6701618ae70c717f9) )
	ROM_LOAD64_WORD( "16.ic89", 0x000004, 0x80000, CRC(305dd72a) SHA1(c373b517c23f3b019abb06e21f6b9ab6e1e47909) )
	ROM_LOAD64_WORD( "13.ic87", 0x000006, 0x80000, CRC(e57f6db9) SHA1(b37f95737804002ec0e237472eaacf0bc1e868e8) )
	ROM_LOAD64_WORD( "11.ic91", 0x200000, 0x80000, CRC(054cd5c4) SHA1(07f275e118c141a84ca15a2e9edc81694af37cf2) )
	ROM_LOAD64_WORD( "8.ic93",  0x200002, 0x80000, CRC(818ca33d) SHA1(dfb707e17c83216f8a62e905f8c7cd6d406b417b) )
	ROM_LOAD64_WORD( "17.ic92", 0x200004, 0x80000, CRC(87e069e8) SHA1(cddd3be84f8379134590bfbbb080518f28120e49) )
	ROM_LOAD64_WORD( "14.ic94", 0x200006, 0x80000, CRC(5dfb44d1) SHA1(08e44b8efc84f9cfc829aabf704155ddc700de76) )
	ROM_LOAD64_WORD( "9.ic86",  0x400000, 0x80000, CRC(34bbb3fa) SHA1(7794e89258f12b17d38c3d302dc15c502a8c8eb6) )
	ROM_LOAD64_WORD( "6.ic84",  0x400002, 0x80000, CRC(cea6d1d6) SHA1(9c953db42f0d877e43c0c239f69a00df39a18295) )
	ROM_LOAD64_WORD( "15.ic85", 0x400004, 0x80000, CRC(7d9f1a67) SHA1(6deb7fff867c42b13a32bb11eda798cfdb4cbaa8) )
	ROM_LOAD64_WORD( "12.ic83", 0x400006, 0x80000, CRC(91a9a05d) SHA1(5266ceddd2df925e79b4200843dec2f7aa9297b3) )

	ROM_REGION( 0x30000, "audiocpu", 0 ) // Sound program + samples
	ROM_LOAD( "1.ic26", 0x00000, 0x20000, CRC(17d5ba8a) SHA1(6ff3b8860d7e1fdee3561846f645eb4d3a8965ec) )
	ROM_RELOAD(         0x10000, 0x20000 )
ROM_END

// main PCB is marked "110-09-91 CH35/1 COMP" on component side
// sub PCB is marked "LS 938" on solder side
ROM_START( sf2ceb5 ) // sf2ceba in FBNeo, it`s a mix between sf2ceb and sf2mdt
	ROM_REGION( CODE_SIZE, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "3.ic171", 0x000000, 0x80000, CRC(a2355d90) SHA1(6c9e1294c55a5a9f244f6f1ce46224c51f910bb1) )
	ROM_LOAD16_BYTE( "5.ic171", 0x000001, 0x80000, CRC(c6f86e84) SHA1(546841fe7d423fff05a7772aa57fa3274515c32b) )
	ROM_LOAD16_BYTE( "2.ic171", 0x100000, 0x20000, CRC(74844192) SHA1(99cd546c78cce7f632007af454d8a55eddb6b19b) )
	ROM_LOAD16_BYTE( "4.ic171", 0x100001, 0x20000, CRC(bd98ff15) SHA1(ed902d949b0b5c5beaaea78a4b418ffa6db9e1df) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROM_LOAD32_WORD( "pf4-sh058.ic90", 0x000000, 0x100000, CRC(446575c7) SHA1(2bd769674fbe280d304b389daf74202cf9e4ac22) )
	ROM_LOAD32_WORD( "pf7-sh072.ic88", 0x000002, 0x100000, CRC(fb78022e) SHA1(b8974387056dd52db96b01cc4648edc814398c7e) )
	ROM_LOAD32_WORD( "pf5-sh036.ic91", 0x200000, 0x100000, CRC(0a6be48b) SHA1(b7e72c94d4e3eb4a6bba6608d9b9a093c8901ad9) )
	ROM_LOAD32_WORD( "pf8-sh074.ic93", 0x200002, 0x100000, CRC(6258c7cf) SHA1(4cd7519245c0aa816934a43e6743160f715d7dc2) )
	ROM_LOAD32_WORD( "pf6-sh071.ic86", 0x400000, 0x100000, CRC(9b5b09d7) SHA1(698a6aab41e495bd0c37a19aee16a84f04d15797) )
	ROM_LOAD32_WORD( "pf9-sh065.ic84", 0x400002, 0x100000, CRC(9f25090e) SHA1(12ff0431ef6550db446985c8914ac7d78eec6b6d) )

	ROM_REGION( 0x30000, "audiocpu", 0 ) // Sound program + samples
	ROM_LOAD( "5.ic26", 0x00000, 0x20000, CRC(17d5ba8a) SHA1(6ff3b8860d7e1fdee3561846f645eb4d3a8965ec) )
	ROM_RELOAD(         0x10000, 0x20000 )
ROM_END

// ************************************************************************* SF2MDT, SF2MDTA, SF2MDTB

/*
    CPU
    1x MC68000P12 (main)
    1x TPC1020AFN-084C (main)
    1x Z0840006PSC-Z80CPU (sound)
    1x YM2151 (sound)
    1x YM3012 (sound)
    2x M5205 (sound)
    2x LM324N (sound)
    1x TDA2003 (sound)
    1x oscillator 24.000000MHz
    1x oscillator 30.000MHz

    ROMs
    14x AM27C040 (1,3,6,7,8,9,10,11,12,13,14,15,16,17)
    3x TMS27C010A (2,4,5)
    3x PAL 16S20 (ic7,ic72, ic80) (read protected, not dumped)
    3x GAL20V8A (ic120, ic121, ic169) (read protected, not dumped)

    Note
    1x JAMMA edge connector
    1x trimmer (volume)
    3x 8x2 switches dip
*/
ROM_START( sf2mdt )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "3.ic172", 0x000000, 0x80000, CRC(5301b41f) SHA1(6855a57b21e8c5d74e5cb18f9ce6af650d7fb422) )
	ROM_LOAD16_BYTE( "1.ic171", 0x000001, 0x80000, CRC(c1c803f6) SHA1(9fe18ae2553a63d8e4dcc20bafd5a4634f8b93c4) )
	ROM_LOAD16_BYTE( "4.ic176", 0x100000, 0x20000, CRC(1073b7b6) SHA1(81ca1eab65ceac69520584bb23a684ccb9d92f89) )
	ROM_LOAD16_BYTE( "2.ic175", 0x100001, 0x20000, CRC(924c6ce2) SHA1(676a912652bd75da5087f0c7eae047b7681a993c) )

	ROM_REGION( 0x600000, "gfx", 0 ) /* rearranged in init */
	ROM_LOAD64_WORD( "7.ic90",  0x000000, 0x80000, CRC(896eaf48) SHA1(5a13ae8b554e05eed3d5749aaf5845d499bce45b) )
	ROM_LOAD64_WORD( "10.ic88", 0x000002, 0x80000, CRC(ef3f5be8) SHA1(d4e1de7d7caf6977e48544d6701618ae70c717f9) )
	ROM_LOAD64_WORD( "13.ic89", 0x000004, 0x80000, CRC(305dd72a) SHA1(c373b517c23f3b019abb06e21f6b9ab6e1e47909) )
	ROM_LOAD64_WORD( "16.ic87", 0x000006, 0x80000, CRC(e57f6db9) SHA1(b37f95737804002ec0e237472eaacf0bc1e868e8) )
	ROM_LOAD64_WORD( "6.ic91",  0x200000, 0x80000, CRC(054cd5c4) SHA1(07f275e118c141a84ca15a2e9edc81694af37cf2) )
	ROM_LOAD64_WORD( "9.ic93",  0x200002, 0x80000, CRC(818ca33d) SHA1(dfb707e17c83216f8a62e905f8c7cd6d406b417b) )
	ROM_LOAD64_WORD( "12.ic92", 0x200004, 0x80000, CRC(87e069e8) SHA1(cddd3be84f8379134590bfbbb080518f28120e49) )
	ROM_LOAD64_WORD( "15.ic94", 0x200006, 0x80000, CRC(5dfb44d1) SHA1(08e44b8efc84f9cfc829aabf704155ddc700de76) )
	ROM_LOAD64_WORD( "8.ic86",  0x400000, 0x80000, CRC(34bbb3fa) SHA1(7794e89258f12b17d38c3d302dc15c502a8c8eb6) )
	ROM_LOAD64_WORD( "11.ic84", 0x400002, 0x80000, CRC(cea6d1d6) SHA1(9c953db42f0d877e43c0c239f69a00df39a18295) )
	ROM_LOAD64_WORD( "14.ic85", 0x400004, 0x80000, CRC(7d9f1a67) SHA1(6deb7fff867c42b13a32bb11eda798cfdb4cbaa8) )
	ROM_LOAD64_WORD( "17.ic83", 0x400006, 0x80000, CRC(91a9a05d) SHA1(5266ceddd2df925e79b4200843dec2f7aa9297b3) )

	ROM_REGION( 0x30000, "audiocpu", 0 ) /* Sound program + samples  */
	ROM_LOAD( "5.ic26", 0x00000, 0x20000, CRC(17d5ba8a) SHA1(6ff3b8860d7e1fdee3561846f645eb4d3a8965ec) )
	ROM_RELOAD(         0x10000, 0x20000 )
ROM_END

ROM_START( sf2mdta )
	/* unconfirmed if working on real hardware, pf4 is a bad dump (bad pin) */
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "3.mdta", 0x000000, 0x80000, CRC(9f544ef4) SHA1(f784809e59a5fcabd6d15d3f1c36250a5528c9f8) )
	ROM_LOAD16_BYTE( "5.mdta", 0x000001, 0x80000, CRC(d76d6621) SHA1(aa9cea9ddace212a7b3c535b8f6e3fbc50da1f94) )
	ROM_LOAD16_BYTE( "2.mdta", 0x100000, 0x20000, CRC(74844192) SHA1(99cd546c78cce7f632007af454d8a55eddb6b19b) )
	ROM_LOAD16_BYTE( "4.mdta", 0x100001, 0x20000, CRC(bd98ff15) SHA1(ed902d949b0b5c5beaaea78a4b418ffa6db9e1df) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROM_LOAD32_WORD( "pf4 sh058.ic89", 0x000000, 0x100000, CRC(16289710) SHA1(4f3236712b979a1eb2fa97740e32d7913cee0d0d) )
	ROM_LOAD32_WORD( "pf7 sh072.ic92", 0x000002, 0x100000, CRC(fb78022e) SHA1(b8974387056dd52db96b01cc4648edc814398c7e) )
	ROM_LOAD32_WORD( "pf5 sh036.ic90", 0x200000, 0x100000, CRC(0a6be48b) SHA1(b7e72c94d4e3eb4a6bba6608d9b9a093c8901ad9) )
	ROM_LOAD32_WORD( "pf8 sh074.ic93", 0x200002, 0x100000, CRC(6258c7cf) SHA1(4cd7519245c0aa816934a43e6743160f715d7dc2) )
	ROM_LOAD32_WORD( "pf6 sh070.ic88", 0x400000, 0x100000, CRC(9b5b09d7) SHA1(698a6aab41e495bd0c37a19aee16a84f04d15797) )
	ROM_LOAD32_WORD( "pf9 sh001.ic91", 0x400002, 0x100000, CRC(9f25090e) SHA1(12ff0431ef6550db446985c8914ac7d78eec6b6d) )

	ROM_REGION( 0x30000, "audiocpu", 0 ) /* Sound program + samples  */
	ROM_LOAD( "1.ic28", 0x00000, 0x20000, CRC(d5bee9cc) SHA1(e638cb5ce7a22c18b60296a7defe8b03418da56c) )
	ROM_RELOAD(         0x10000, 0x20000 )
ROM_END

ROM_START( sf2mdtb )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "3.ic172", 0x000000, 0x80000, CRC(0bdb9da2) SHA1(5224ee81d94be70a84ffaa3a56b8093aa36d6b4f) ) // sldh
	ROM_LOAD16_BYTE( "1.ic171", 0x000001, 0x80000, CRC(d88abbce) SHA1(57667a92710bb1d37daed09262c3064d09cbf4af) ) // sldh
	ROM_LOAD16_BYTE( "4.ic176", 0x100000, 0x20000, CRC(74844192) SHA1(99cd546c78cce7f632007af454d8a55eddb6b19b) ) // sldh
	ROM_LOAD16_BYTE( "2.ic175", 0x100001, 0x20000, CRC(bd98ff15) SHA1(ed902d949b0b5c5beaaea78a4b418ffa6db9e1df) ) // sldh

	ROM_REGION( 0x600000, "gfx", 0 ) /* rearranged in init */
	ROM_LOAD64_WORD( "7.ic90",  0x000000, 0x80000, CRC(896eaf48) SHA1(5a13ae8b554e05eed3d5749aaf5845d499bce45b) )
	ROM_LOAD64_WORD( "10.ic88", 0x000002, 0x80000, CRC(ef3f5be8) SHA1(d4e1de7d7caf6977e48544d6701618ae70c717f9) )
	ROM_LOAD64_WORD( "13.ic89", 0x000004, 0x80000, CRC(305dd72a) SHA1(c373b517c23f3b019abb06e21f6b9ab6e1e47909) )
	ROM_LOAD64_WORD( "16.ic87", 0x000006, 0x80000, CRC(e57f6db9) SHA1(b37f95737804002ec0e237472eaacf0bc1e868e8) )
	ROM_LOAD64_WORD( "6.ic91",  0x200000, 0x80000, CRC(054cd5c4) SHA1(07f275e118c141a84ca15a2e9edc81694af37cf2) )
	ROM_LOAD64_WORD( "9.ic93",  0x200002, 0x80000, CRC(818ca33d) SHA1(dfb707e17c83216f8a62e905f8c7cd6d406b417b) )
	ROM_LOAD64_WORD( "12.ic92", 0x200004, 0x80000, CRC(87e069e8) SHA1(cddd3be84f8379134590bfbbb080518f28120e49) )
	ROM_LOAD64_WORD( "15.ic94", 0x200006, 0x80000, CRC(5dfb44d1) SHA1(08e44b8efc84f9cfc829aabf704155ddc700de76) )
	ROM_LOAD64_WORD( "8.ic86",  0x400000, 0x80000, CRC(34bbb3fa) SHA1(7794e89258f12b17d38c3d302dc15c502a8c8eb6) )
	ROM_LOAD64_WORD( "11.ic84", 0x400002, 0x80000, CRC(cea6d1d6) SHA1(9c953db42f0d877e43c0c239f69a00df39a18295) )
	ROM_LOAD64_WORD( "14.ic85", 0x400004, 0x80000, CRC(7d9f1a67) SHA1(6deb7fff867c42b13a32bb11eda798cfdb4cbaa8) )
	ROM_LOAD64_WORD( "17.ic83", 0x400006, 0x80000, CRC(91a9a05d) SHA1(5266ceddd2df925e79b4200843dec2f7aa9297b3) )

	ROM_REGION( 0x30000, "audiocpu", 0 ) /* Sound program + samples  */
	ROM_LOAD( "5.ic28", 0x00000, 0x20000, CRC(d5bee9cc) SHA1(e638cb5ce7a22c18b60296a7defe8b03418da56c) )
	ROM_RELOAD(         0x10000, 0x20000 )
ROM_END

} // anonymous namespace


// ************************************************************************* DRIVER MACROS

GAME( 1991,  captcommb2,  captcomm, captcommb2,  captcommb2,  captcommb2_state,   init_captcommb2,  ROT0,  "bootleg",  "Captain Commando (bootleg with 2xMSM5205)",  MACHINE_SUPPORTS_SAVE )  // 911014 ETC

GAME( 1991,  knightsb,    knights,  knightsb,    knights,     captcommb2_state,   init_knightsb,    ROT0,  "bootleg",  "Knights of the Round (bootleg with 2xMSM5205, set 1)",  MACHINE_SUPPORTS_SAVE )  // 911127 ETC
GAME( 1991,  knightsb3,   knights,  knightsb,    knights,     captcommb2_state,   init_knightsb,    ROT0,  "bootleg",  "Knights of the Round (bootleg with 2xMSM5205, set 2)",  MACHINE_SUPPORTS_SAVE )  // 911127 ETC

GAME( 1992,  sf2b,        sf2,      sf2b,        sf2mdt,      cps1bl_5205_state,  init_sf2b,        ROT0,  "bootleg (Playmark)",  "Street Fighter II: The World Warrior (bootleg, set 1)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )  // 910204 ETC
GAME( 1992,  sf2b2,       sf2,      sf2b,        sf2mdt,      cps1bl_5205_state,  init_sf2mdtb,     ROT0,  "bootleg", "Street Fighter II: The World Warrior (bootleg, set 2)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )              // 910204 ETC

GAME( 1992,  sf2ceb,      sf2ce,    sf2mdt,      sf2mdt,      cps1bl_5205_state,  init_sf2mdta,     ROT0,  "bootleg (Playmark)",  "Street Fighter II': Champion Edition (Playmark bootleg, set 1)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )  // 920313 ETC
GAME( 1992,  sf2ceb2,     sf2ce,    sf2mdt,      sf2mdt,      cps1bl_5205_state,  init_sf2mdtb,     ROT0,  "bootleg",  "Street Fighter II': Champion Edition (bootleg, set 1)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )  // 920313 ETC
GAME( 1992,  sf2ceb3,     sf2ce,    sf2mdt,      sf2mdt,      cps1bl_5205_state,  init_sf2mdtb,     ROT0,  "bootleg",  "Street Fighter II': Champion Edition (bootleg, set 2)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )  // 920313 ETC
GAME( 1992,  sf2ceb4,     sf2ce,    sf2mdt,      sf2mdt,      cps1bl_5205_state,  init_sf2mdtb,     ROT0,  "bootleg (Playmark)",  "Street Fighter II': Champion Edition (Playmark bootleg, set 2)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )  // 920313 ETC
GAME( 1992,  sf2ceb5,     sf2ce,    sf2mdt,      sf2mdt,      cps1bl_5205_state,  init_sf2mdta,     ROT0,  "bootleg (Playmark)",  "Street Fighter II': Champion Edition (Playmark bootleg, set 3)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )  // 920313 ETC

GAME( 1992,  sf2mdt,      sf2ce,    sf2mdt,      sf2mdt,      cps1bl_5205_state,  init_sf2mdt,      ROT0,  "bootleg",  "Street Fighter II': Magic Delta Turbo (bootleg, set 1)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )  // 920313 ETC
GAME( 1992,  sf2mdta,     sf2ce,    sf2mdt,      sf2mdt,      cps1bl_5205_state,  init_sf2mdta,     ROT0,  "bootleg",  "Street Fighter II': Magic Delta Turbo (bootleg, set 2)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )  // 920313 ETC
GAME( 1992,  sf2mdtb,     sf2ce,    sf2mdt,      sf2mdtb,     cps1bl_5205_state,  init_sf2mdtb,     ROT0,  "bootleg",  "Street Fighter II': Magic Delta Turbo (bootleg, set 3)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )  // 920313 ETC
