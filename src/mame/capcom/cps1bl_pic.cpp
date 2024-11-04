// license:BSD-3-Clause
// copyright-holders:David Haywood

/*
    CPS1 single board bootlegs

    sound hardware: PIC16c57, oki M6295 (no z80)

    Games known to use this h/w:
    Cadillacs and Dinosaurs         930201 ETC
    The Punisher                    930422 ETC
    Saturday Night Slam Masters     930713 ETC
    Warriors of Fate                921002 ETC

    (Note, these are all CPS1.5/Q sound games)

    Generally the sound quality is very poor compared to official Capcom hardware.
    Both music and sound effects are produced by just a single M6295.
    Background music consists of short pre-recorded clips which loop continuously.
    Currently all games have no sound emulation due to the PICs being secured/protected.
    Unless any un-protected PIcs ever turn up (unlikely) then "decapping" of working chips is probably the
    only way valid dumps will ever be made.

Status of each game:
--------------------
dinopic:           No sound.
dinopic2:          No sound.
dinopic3:          No sound. Some minor gfx priority issues, confirmed present on real board.
jurassic99:        No sound.
punipic, punipic2: No sound. Problems in Central Park. Patches used.
punipic3:          Same as punipic, and doors are missing.
slampic:           No sound. Some minor gfx issues (sprites on character select screen).
slampic2:          No sound. All gfx issues confirmed present on real board.
wofpic:            No sound. Some minor gfx issues (sprite priorities mainly).  https://youtu.be/Ozlb2gNRSfs

all dinopic sets have some priority issues with sprites overlapping foreground objects on certain levels

brightness circuity present on pcb?
    slampic2    yes
    dinopic3    no
    jurassic99  no
    others      tbc...   assume no for now
*/

#include "emu.h"
#include "fcrash.h"

#include "cpu/m68000/m68000.h"
#include "cpu/pic16c5x/pic16c5x.h"
#include "sound/okim6295.h"
#include "machine/eepromser.h"
#include "speaker.h"


namespace {

#define CPS1_ROWSCROLL_OFFS  (0x20/2)    /* base of row scroll offsets in other RAM */
#define CODE_SIZE            0x400000


class cps1bl_pic_state : public cps1bl_no_brgt
{
public:
	cps1bl_pic_state(const machine_config &mconfig, device_type type, const char *tag)
		: cps1bl_no_brgt(mconfig, type, tag)
	{ }

	void punipic(machine_config &config);
	void slampic(machine_config &config);

	void init_dinopic();
	void init_punipic();
	void init_punipic3();
	void init_slampic();

protected:
	void dinopic_layer_w(offs_t offset, uint16_t data);

private:
	DECLARE_MACHINE_START(punipic);
	DECLARE_MACHINE_START(slampic);

	void punipic_layer_w(offs_t offset, uint16_t data);
	void slampic_layer_w(offs_t offset, uint16_t data);
	void slampic_layer2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void punipic_map(address_map &map) ATTR_COLD;
	void slampic_map(address_map &map) ATTR_COLD;
};

class slampic2_state : public fcrash_state
{
public:
	slampic2_state(const machine_config &mconfig, device_type type, const char *tag)
		: fcrash_state(mconfig, type, tag)
	{ }

	void slampic2(machine_config &config);
	void init_slampic2();

private:
	DECLARE_MACHINE_START(slampic2);
	uint16_t slampic2_cps_a_r(offs_t offset);
	void slampic2_sound_w(uint16_t data);
	void slampic2_sound2_w(uint16_t data);
	void slampic2_map(address_map &map) ATTR_COLD;
	void bootleg_render_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect) override;
};

class dinopic_state : public cps1bl_pic_state
{
public:
	dinopic_state(const machine_config &mconfig, device_type type, const char *tag)
		: cps1bl_pic_state(mconfig, type, tag)
	{ }

	void dinopic(machine_config &config);

private:
	void dinopic_layer2_w(uint16_t data);
	DECLARE_MACHINE_START(dinopic);
	void dinopic_map(address_map &map) ATTR_COLD;
	void bootleg_render_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect) override;
};

class wofpic_state : public dinopic_state
{
public:
	wofpic_state(const machine_config &mconfig, device_type type, const char *tag)
		: dinopic_state(mconfig, type, tag)
	{ }

	void wofpic(machine_config &config);
	void init_wofpic();

private:
	void wofpic_layer_w(offs_t offset, uint16_t data);
	void wofpic_layer2_w(uint16_t data);
	void wofpic_spr_base_w(uint16_t data);
	DECLARE_MACHINE_START(wofpic);
	void wofpic_map(address_map &map) ATTR_COLD;
};


void cps1bl_pic_state::dinopic_layer_w(offs_t offset, uint16_t data)
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
	default:
		logerror("%s: Unknown layer cmd %X %X\n",machine().describe_context(),offset<<1,data);
	}
}

void dinopic_state::dinopic_layer2_w(uint16_t data)
{
	m_cps_a_regs[0x06 / 2] = data;
}

void cps1bl_pic_state::punipic_layer_w(offs_t offset, uint16_t data)
{
	m_cps_a_regs[0x08/2] = 0;

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
		m_cps_a_regs[0x10 / 2] = data + 0xffc0;
		break;
	case 0x04:
		m_cps_a_regs[0x16 / 2] = data;
		break;
	case 0x05:
		m_cps_a_regs[0x14 / 2] = data;
		break;
	case 0x06:
			switch (data)
			{
			case 0x14:
			case 0x54:
				m_cps_a_regs[0x04 / 2] = 0x9100;
				break;
			case 0x24:
			case 0x64:
				m_cps_a_regs[0x04 / 2] = 0x90c0;
				break;
			case 0x3c:
			case 0x7c:
				m_cps_a_regs[0x04 / 2] = 0x9180;
				break;
			}

			break;
	case 0x07:
		// unknown
		break;
	default:
		logerror("%s: Unknown layer cmd %X %X\n",machine().describe_context(),offset<<1,data);
	}
}

void cps1bl_pic_state::slampic_layer_w(offs_t offset, uint16_t data)
{
	switch (offset)
	{
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
		dinopic_layer_w(offset, data);
		break;
	case 0x06: // scroll 2 base
		m_cps_a_regs[0x04/2] = data << 4;
		break;
	}
}

void cps1bl_pic_state::slampic_layer2_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_cps_a_regs[offset]);

	if (offset == 0x22 / 2)
	{
		// doesn't seem to write anywhere outside mainram?
		m_cps_b_regs[m_layer_enable_reg / 2] = m_mainram[0x8d72 / 2];
		m_cps_b_regs[m_layer_mask_reg[1] / 2] = m_mainram[0x8d74 / 2];
		m_cps_b_regs[m_layer_mask_reg[2] / 2] = m_mainram[0x8d76 / 2];
		m_cps_b_regs[m_layer_mask_reg[3] / 2] = m_mainram[0x8d78 / 2];
	}
}

uint16_t slampic2_state::slampic2_cps_a_r(offs_t offset)
{
	// checks bit 0 of 800132
	// no sound codes are sent unless this returns true, ready signal from the sound PIC?
	if (offset == 0x32 / 2)
		return 0xffff;
	else
		logerror("Read from cps-a register %02x\n", offset * 2);
	return 0;
}

void slampic2_state::slampic2_sound_w(uint16_t data)
{
	//logerror("Sound command: %04x\n", data);
}

void slampic2_state::slampic2_sound2_w(uint16_t data)
{
	//logerror("Sound2 command: %04x\n", data);
}

void wofpic_state::wofpic_layer_w(offs_t offset, uint16_t data)
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
			// see bootleggers routines starting at $101000
			// writes values 0-f to 98000c
			// how does this relate to layer control reg value?

			// original game values:
			// m_cps_b_regs[m_layer_enable_reg / 2] = m_mainram[0x6398 / 2];
			// m_cps_b_regs[m_layer_mask_reg[1] / 2] = m_mainram[0x639a / 2];
			// m_cps_b_regs[m_layer_mask_reg[2] / 2] = m_mainram[0x639c / 2];
			// m_cps_b_regs[m_layer_mask_reg[3] / 2] = m_mainram[0x639e / 2];

			m_cps_b_regs[0x3e / 2] = data;

			switch (data)
			{
			case 0:    // 12ce
				m_cps_b_regs[m_layer_enable_reg / 2] = 0x12ce;  // attract lvl 1
				m_cps_b_regs[m_layer_mask_reg[1] / 2] = 0x1f;
				m_cps_b_regs[m_layer_mask_reg[2] / 2] = 0x1ff;
				m_cps_b_regs[m_layer_mask_reg[3] / 2] = 0x7fff;
				break;
			case 1:    // 12c2, 12c6, 270a, 138e, 18ce
				m_cps_b_regs[m_layer_enable_reg / 2] = 0x138e;  // attract lvl 4
				m_cps_b_regs[m_layer_mask_reg[1] / 2] = 0x3f;
				m_cps_b_regs[m_layer_mask_reg[2] / 2] = 0x1ff;
				m_cps_b_regs[m_layer_mask_reg[3] / 2] = 0x7fff;
				break;
			case 2:
				m_cps_b_regs[m_layer_enable_reg / 2] = 0x12ce; // ?
				m_cps_b_regs[m_layer_mask_reg[2] / 2] = 0x780;
				m_cps_b_regs[m_layer_mask_reg[3] / 2] = 0;
				break;
			case 3:    // 1c8e, 1c82, 1c86, 270a
				m_cps_b_regs[m_layer_enable_reg / 2] = 0x1c8e;  // attract lvl 2
				m_cps_b_regs[m_layer_mask_reg[1] / 2] = 0x7ff;
				m_cps_b_regs[m_layer_mask_reg[2] / 2] = 0x780;
				m_cps_b_regs[m_layer_mask_reg[3] / 2] = 0;
				break;
			case 4:
				m_cps_b_regs[m_layer_enable_reg / 2] = 0x12ce; // ?
				m_cps_b_regs[m_layer_mask_reg[2] / 2] = 0;
				m_cps_b_regs[m_layer_mask_reg[3] / 2] = 0x7fff;
				break;
			case 5:
				break;
			case 6:
				m_cps_b_regs[m_layer_enable_reg / 2] = 0x12ce; // ?
				m_cps_b_regs[m_layer_mask_reg[2] / 2] = 0x781;
				m_cps_b_regs[m_layer_mask_reg[3] / 2] = 0x1f;
				break;
			case 7:
				break;
			case 8:
				m_cps_b_regs[m_layer_enable_reg / 2] = 0x12ce; // ?
				m_cps_b_regs[m_layer_mask_reg[2] / 2] = 0;
				m_cps_b_regs[m_layer_mask_reg[3] / 2] = 0x1f;
				break;
			case 9:
				break;
			case 10:
				m_cps_b_regs[m_layer_enable_reg / 2] = 0x12ce; // ?
				m_cps_b_regs[m_layer_mask_reg[2] / 2] = 0x40ff;
				m_cps_b_regs[m_layer_mask_reg[3] / 2] = 0x7fff;
				break;
			case 11:
				break;
			case 14:    // 12ce, 1b0e
				m_cps_b_regs[m_layer_enable_reg / 2] = 0x12ce;
				break;
			case 15:    // 270a, 1e0e, 138e, 270e
				m_cps_b_regs[m_layer_enable_reg / 2] = 0x138e;  // attract lvl 3
				m_cps_b_regs[m_layer_mask_reg[1] / 2] = 0x7fff;
				m_cps_b_regs[m_layer_mask_reg[2] / 2] = 0x7fff;
				m_cps_b_regs[m_layer_mask_reg[3] / 2] = 0x7fff;
				break;
			}
		}
		break;
	default:
		logerror("%s: Unknown layer cmd %X %X\n",machine().describe_context(),offset<<1,data);
	}
}

void wofpic_state::wofpic_layer2_w(uint16_t data)
{
	m_cps_a_regs[0x06 / 2] = data;
}

void wofpic_state::wofpic_spr_base_w(uint16_t data)
{
	m_sprite_base = data ? 0x3000 : 0x1000;
}


void dinopic_state::dinopic(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 12000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &dinopic_state::dinopic_map);
	m_maincpu->set_vblank_int("screen", FUNC(dinopic_state::cps1_interrupt));
	m_maincpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &dinopic_state::cpu_space_map);

	//PIC16C57(config, m_audiocpu, 3750000).set_disable(); /* no valid dumps .. */

	MCFG_MACHINE_START_OVERRIDE(dinopic_state, dinopic)

	EEPROM_93C46_8BIT(config, "eeprom");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(8*8, (64-8)*8-1, 2*8, 30*8-1 );
	m_screen->set_screen_update(FUNC(dinopic_state::screen_update_fcrash));
	m_screen->screen_vblank().set(FUNC(dinopic_state::screen_vblank_cps1));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_cps1);
	PALETTE(config, m_palette, palette_device::BLACK).set_entries(0xc00);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	OKIM6295(config, m_oki, 1000000, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.30);
}

void cps1bl_pic_state::punipic(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 12000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &cps1bl_pic_state::punipic_map);
	m_maincpu->set_vblank_int("screen", FUNC(cps1bl_pic_state::cps1_interrupt));
	m_maincpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &cps1bl_pic_state::cpu_space_map);

	//PIC16C57(config, m_audiocpu, 12000000).set_disable(); /* no valid dumps .. */

	MCFG_MACHINE_START_OVERRIDE(cps1bl_pic_state, punipic)

	EEPROM_93C46_8BIT(config, "eeprom");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(8*8, (64-8)*8-1, 2*8, 30*8-1 );
	m_screen->set_screen_update(FUNC(cps1bl_pic_state::screen_update_fcrash));
	m_screen->screen_vblank().set(FUNC(cps1bl_pic_state::screen_vblank_cps1));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_cps1);
	PALETTE(config, m_palette, palette_device::BLACK).set_entries(0xc00);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	OKIM6295(config, m_oki, 1000000, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.30);
}

void cps1bl_pic_state::slampic(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 12000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &cps1bl_pic_state::slampic_map);
	m_maincpu->set_vblank_int("screen", FUNC(cps1bl_pic_state::cps1_interrupt));
	m_maincpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &cps1bl_pic_state::cpu_space_map);

	//PIC16C57(config, m_audiocpu, 12000000).set_disable(); /* no valid dumps .. */

	MCFG_MACHINE_START_OVERRIDE(cps1bl_pic_state, slampic)

	EEPROM_93C46_8BIT(config, "eeprom");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(8*8, (64-8)*8-1, 2*8, 30*8-1 );
	m_screen->set_screen_update(FUNC(cps1bl_pic_state::screen_update_fcrash));
	m_screen->screen_vblank().set(FUNC(cps1bl_pic_state::screen_vblank_cps1));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_cps1);
	PALETTE(config, m_palette, palette_device::BLACK).set_entries(0xc00);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki, 1000000, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.30);
}

void slampic2_state::slampic2(machine_config &config)
{
	M68000(config, m_maincpu, 10000000);  // measured
	m_maincpu->set_addrmap(AS_PROGRAM, &slampic2_state::slampic2_map);
	m_maincpu->set_vblank_int("screen", FUNC(slampic2_state::cps1_interrupt));
	m_maincpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &slampic2_state::cpu_space_map);

	PIC16C57(config, m_audiocpu, 4000000);  // measured
	//m_audiocpu->set_disable();

	MCFG_MACHINE_START_OVERRIDE(slampic2_state, slampic2)

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(CPS_PIXEL_CLOCK, CPS_HTOTAL, CPS_HBEND, CPS_HBSTART, CPS_VTOTAL, CPS_VBEND, CPS_VBSTART);
	m_screen->set_screen_update(FUNC(slampic2_state::screen_update_fcrash));
	//m_screen->screen_vblank().set(FUNC(slampic2_state::screen_vblank_cps1));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_cps1);
	PALETTE(config, m_palette, palette_device::BLACK).set_entries(0xc00);

	SPEAKER(config, "mono").front_center();
	//GENERIC_LATCH_8(config, m_soundlatch);
	//GENERIC_LATCH_8(config, m_soundlatch2);
	OKIM6295(config, m_oki, 1000000, okim6295_device::PIN7_LOW);  // measured & pin 7 verified
	//m_oki->set_addrmap(0, &slampic2_state::slampic2_oki_map);
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.80);
}

void wofpic_state::wofpic(machine_config &config)
{
	dinopic(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &wofpic_state::wofpic_map);
	MCFG_MACHINE_START_OVERRIDE(wofpic_state, wofpic)
}


void dinopic_state::dinopic_map(address_map &map)
{
	map(0x000000, 0x3fffff).rom();
	map(0x800000, 0x800007).portr("IN1");            /* Player input ports */
	map(0x800006, 0x800007).w(FUNC(dinopic_state::cps1_soundlatch_w));    /* Sound command */
	map(0x800018, 0x80001f).r(FUNC(dinopic_state::cps1_dsw_r));            /* System input ports / Dip Switches */
	map(0x800030, 0x800037).w(FUNC(dinopic_state::cps1_coinctrl_w));
	map(0x800100, 0x80013f).w(FUNC(dinopic_state::cps1_cps_a_w)).share("cps_a_regs");  /* CPS-A custom */
	map(0x800140, 0x80017f).rw(FUNC(dinopic_state::cps1_cps_b_r), FUNC(dinopic_state::cps1_cps_b_w)).share("cps_b_regs");
	map(0x800222, 0x800223).w(FUNC(dinopic_state::dinopic_layer2_w));
	map(0x880000, 0x880001).nopw(); // always 0
	map(0x900000, 0x92ffff).ram().w(FUNC(dinopic_state::cps1_gfxram_w)).share("gfxram");
	map(0x980000, 0x98000b).w(FUNC(dinopic_state::dinopic_layer_w));
	map(0xf18000, 0xf19fff).ram();
	map(0xf1c000, 0xf1c001).portr("IN2");            /* Player 3 controls (later games) */
	map(0xf1c004, 0xf1c005).w(FUNC(dinopic_state::cpsq_coinctrl2_w));     /* Coin control2 (later games) */
	map(0xf1c006, 0xf1c007).portr("EEPROMIN").portw("EEPROMOUT");
	map(0xff0000, 0xffffff).ram().share("mainram");
}

void cps1bl_pic_state::punipic_map(address_map &map)
{
	map(0x000000, 0x3fffff).rom();
	map(0x800000, 0x800007).portr("IN1");            /* Player input ports */
	map(0x800006, 0x800007).w(FUNC(cps1bl_pic_state::cps1_soundlatch_w));    /* Sound command */
	map(0x800018, 0x80001f).r(FUNC(cps1bl_pic_state::cps1_dsw_r));            /* System input ports / Dip Switches */
	map(0x800030, 0x800037).w(FUNC(cps1bl_pic_state::cps1_coinctrl_w));
	map(0x800100, 0x80013f).w(FUNC(cps1bl_pic_state::cps1_cps_a_w)).share("cps_a_regs");  /* CPS-A custom */
	map(0x800140, 0x80017f).rw(FUNC(cps1bl_pic_state::cps1_cps_b_r), FUNC(cps1bl_pic_state::cps1_cps_b_w)).share("cps_b_regs");
	map(0x880000, 0x880001).nopw(); // same as 98000C
	map(0x900000, 0x92ffff).ram().w(FUNC(cps1bl_pic_state::cps1_gfxram_w)).share("gfxram");
	map(0x980000, 0x98000f).w(FUNC(cps1bl_pic_state::punipic_layer_w));
	map(0x990000, 0x990001).nopw(); // unknown
	map(0x991000, 0x991017).nopw(); // unknown
	map(0xf18000, 0xf19fff).ram();
	map(0xf1c006, 0xf1c007).portr("EEPROMIN").portw("EEPROMOUT");
	map(0xff0000, 0xffffff).ram().share("mainram");
}

void cps1bl_pic_state::slampic_map(address_map &map)
{
	map(0x000000, 0x3fffff).rom();
	map(0x800006, 0x800007).nopw(); //.w(FUNC(cps1bl_pic_state::cps1_soundlatch2_w));
	map(0x800000, 0x800007).portr("IN1");            /* Player input ports */
	map(0x800018, 0x80001f).r(FUNC(cps1bl_pic_state::cps1_dsw_r));            /* System input ports / Dip Switches */
	map(0x800030, 0x800037).w(FUNC(cps1bl_pic_state::cps1_coinctrl_w));
	map(0x800100, 0x80013f).ram().w(FUNC(cps1bl_pic_state::slampic_layer2_w)).share("cps_a_regs");  /* CPS-A custom */
	map(0x800140, 0x80017f).ram().share("cps_b_regs");
	map(0x880000, 0x880001).nopw(); //.w(FUNC(cps1bl_pic_state::cps1_soundlatch_w));    /* Sound command */
	map(0x900000, 0x92ffff).ram().w(FUNC(cps1bl_pic_state::cps1_gfxram_w)).share("gfxram");
	map(0x980000, 0x98000d).w(FUNC(cps1bl_pic_state::slampic_layer_w));
	map(0xf00000, 0xf0ffff).r(FUNC(cps1bl_pic_state::qsound_rom_r));          /* Slammasters protection */
	map(0xf18000, 0xf19fff).ram();
	map(0xf1c000, 0xf1c001).portr("IN2");            /* Player 3 controls (later games) */
	map(0xf1c004, 0xf1c005).w(FUNC(cps1bl_pic_state::cpsq_coinctrl2_w));     /* Coin control2 (later games) */
	map(0xf1c006, 0xf1c007).portr("EEPROMIN").portw("EEPROMOUT");
	map(0xf1f000, 0xf1ffff).noprw(); // writes 0 to range, then reads F1F6EC, occasionally writes 0d5f->f1f6f0
	map(0xff0000, 0xffffff).ram().share("mainram");
}

void slampic2_state::slampic2_map(address_map &map)
{
	map(0x000000, 0x3fffff).rom();
	map(0x800000, 0x800001).portr("IN1");
	map(0x800002, 0x800003).portr("IN2"); // player 3 + 4 inputs
	map(0x800018, 0x80001f).r(FUNC(slampic2_state::cps1_dsw_r));
	map(0x800030, 0x800031).nopw();  // coin ctrl
	map(0x800100, 0x80013f).ram().r(FUNC(slampic2_state::slampic2_cps_a_r)).share("cps_a_regs");
	map(0x800140, 0x80017f).ram().share("cps_b_regs");
	map(0x800180, 0x800181).w(FUNC(slampic2_state::slampic2_sound_w));   // sound
	map(0x800188, 0x800189).w(FUNC(slampic2_state::slampic2_sound2_w));  // sound
	map(0x8ffff8, 0x8fffff).nopw();  // ?
	map(0x900000, 0x92ffff).ram().mirror(0x6c0000).w(FUNC(slampic2_state::cps1_gfxram_w)).share("gfxram");
	//  0x930000, 0x933fff  spriteram mirror?
	//  0xf00000, 0xf3ffff  workram
	//  0xfc0000, 0xfeffff  gfxram
	//  0xff0000, 0xff3fff  spriteram
	map(0xff4000, 0xffffff).ram().share("mainram");

	/*
	                  slammast        slampic2
	sprite table 1    900000-9007ff   ff2000-ff27ff
	                                  ff2800-ff2fff  ?
	sprite table 2    904000-9047ff   ff3000-ff37ff
	                                  ff3800-ff3fff  ?

	gfxram            900000-91bfff   900000-91bfff
	                  91c000-92ffff   fdc000-feffff

	test menu reads 3p + 4p controls at original ports f1c000-f1c003
	start-up check tests f00000-f40000 region
	start-up check tests 930000-934000 region but ignores any failure found, mirrored with sprite table region?
	*/
}

void wofpic_state::wofpic_map(address_map &map)
{
	map(0x000000, 0x3fffff).rom();
	map(0x800000, 0x800007).portr("IN1");            /* Player input ports */
	map(0x800006, 0x800007).w(FUNC(wofpic_state::cps1_soundlatch_w));    /* Sound command */
	map(0x800008, 0x800009).w(FUNC(wofpic_state::wofpic_layer2_w));
	map(0x800018, 0x80001f).r(FUNC(wofpic_state::cps1_dsw_r));            /* System input ports / Dip Switches */
	map(0x800030, 0x800037).w(FUNC(wofpic_state::cps1_coinctrl_w));
	map(0x800100, 0x80013f).w(FUNC(wofpic_state::cps1_cps_a_w)).share("cps_a_regs");  /* CPS-A custom */
	map(0x800140, 0x80017f).rw(FUNC(wofpic_state::cps1_cps_b_r), FUNC(wofpic_state::cps1_cps_b_w)).share("cps_b_regs");  /* Only writes here at boot */
	map(0x880000, 0x880001).nopw(); // ?
	map(0x900000, 0x92ffff).ram().w(FUNC(wofpic_state::cps1_gfxram_w)).share("gfxram");
	map(0x980000, 0x98000d).w(FUNC(wofpic_state::wofpic_layer_w));
	map(0xf18000, 0xf19fff).nopw(); // few q-sound leftovers
	map(0xf1c000, 0xf1c001).portr("IN2");            /* Player 3 controls (later games) */
	map(0xf1c004, 0xf1c005).w(FUNC(wofpic_state::cpsq_coinctrl2_w));     /* Coin control2 (later games) */
	map(0xf1c006, 0xf1c007).portr("EEPROMIN").portw("EEPROMOUT");
	map(0xff0000, 0xffffff).ram().share("mainram");
}


MACHINE_START_MEMBER(dinopic_state, dinopic)
{
	m_layer_enable_reg = 0x0a;
	m_layer_mask_reg[0] = 0x0c;
	m_layer_mask_reg[1] = 0x0e;
	m_layer_mask_reg[2] = 0x00;
	m_layer_mask_reg[3] = 0x02;
	m_layer_scroll1x_offset = 0x40;
	m_layer_scroll2x_offset = 0x40;
	m_layer_scroll3x_offset = 0x40;
	m_sprite_base = 0x1000;
	m_sprite_list_end_marker = 0x8000;
	m_sprite_x_offset = 0;
}

MACHINE_START_MEMBER(cps1bl_pic_state, punipic)
{
	m_layer_enable_reg = 0x12;
	m_layer_mask_reg[0] = 0x14;
	m_layer_mask_reg[1] = 0x16;
	m_layer_mask_reg[2] = 0x08;
	m_layer_mask_reg[3] = 0x0a;
	m_layer_scroll1x_offset = 0x46; // text
	m_layer_scroll3x_offset = 0x46; // green patch in the park
	m_sprite_base = 0x1000;
	m_sprite_list_end_marker = 0x8000;
	m_sprite_x_offset = 0;
}

MACHINE_START_MEMBER(cps1bl_pic_state, slampic)
{
	m_layer_enable_reg = 0x16;
	m_layer_mask_reg[0] = 0x00;
	m_layer_mask_reg[1] = 0x02;
	m_layer_mask_reg[2] = 0x28;
	m_layer_mask_reg[3] = 0x2a;
	m_layer_scroll1x_offset = 0x40;
	m_layer_scroll2x_offset = 0x40;
	m_layer_scroll3x_offset = 0x40;
	m_sprite_base = 0x1000;
	m_sprite_list_end_marker = 0x8000;
	m_sprite_x_offset = 2;
}

MACHINE_START_MEMBER(slampic2_state, slampic2)
{
	m_layer_enable_reg = 0x16;
	m_layer_mask_reg[1] = 0x02;
	m_layer_mask_reg[2] = 0x28;
	m_layer_mask_reg[3] = 0x2a;
	m_layer_scroll1x_offset = 12;  // y offset 1px too low
	m_layer_scroll2x_offset = 14;  // y offset 1px too low
	m_layer_scroll3x_offset = 15;  // y offset 1px too low
	m_sprite_base = 0x1000;
	m_sprite_list_end_marker = 0xff00;
	m_sprite_x_offset = 0;
}

MACHINE_START_MEMBER(wofpic_state, wofpic)
{
	m_layer_enable_reg = 0x26;
	m_layer_mask_reg[0] = 0x28;
	m_layer_mask_reg[1] = 0x2a;
	m_layer_mask_reg[2] = 0x2c;
	m_layer_mask_reg[3] = 0x2e;
	m_layer_scroll1x_offset = 0x40;
	m_layer_scroll2x_offset = 0x40;
	m_layer_scroll3x_offset = 0x40;
	m_sprite_base = 0x1000;
	m_sprite_list_end_marker = 0x8000;
	m_sprite_x_offset = 2;
}


void cps1bl_pic_state::init_dinopic()
{
	m_bootleg_sprite_ram = std::make_unique<uint16_t[]>(0x2000);
	m_maincpu->space(AS_PROGRAM).install_ram(0x990000, 0x993fff, m_bootleg_sprite_ram.get());
	init_cps1();
}

void cps1bl_pic_state::init_punipic()
{
	uint16_t *mem16 = (uint16_t *)memregion("maincpu")->base();
	mem16[0x5A8/2] = 0x4E71; // set data pointers
	mem16[0x4DF0/2] = 0x33ED;
	mem16[0x4DF2/2] = 0xDB2E;
	mem16[0x4DF4/2] = 0x0080;
	mem16[0x4DF6/2] = 0x0152;
	mem16[0x4DF8/2] = 0x4E75;

	init_dinopic();
}

void cps1bl_pic_state::init_punipic3()
{
	uint16_t *mem16 = (uint16_t *)memregion("maincpu")->base();
	mem16[0x5A6/2] = 0x4E71; // set data pointers
	mem16[0x5A8/2] = 0x4E71;

	init_dinopic();
}

void slampic2_state::init_slampic2()
{
	m_bootleg_sprite_ram = std::make_unique<uint16_t[]>(0x2000);
	m_maincpu->space(AS_PROGRAM).install_ram(0x930000, 0x933fff, m_bootleg_sprite_ram.get());
	m_maincpu->space(AS_PROGRAM).install_ram(0xff0000, 0xff3fff, m_bootleg_sprite_ram.get());

	m_bootleg_work_ram = std::make_unique<uint16_t[]>(0x20000);
	m_maincpu->space(AS_PROGRAM).install_ram(0xf00000, 0xf3ffff, m_bootleg_work_ram.get());

	init_cps1();
}

void wofpic_state::init_wofpic()
{
	m_bootleg_sprite_ram = std::make_unique<uint16_t[]>(0x2000);
	m_maincpu->space(AS_PROGRAM).install_ram(0x990000, 0x993fff, m_bootleg_sprite_ram.get());
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x990000, 0x990001, write16smo_delegate(*this, FUNC(wofpic_state::wofpic_spr_base_w)));
	init_cps1();
}


static INPUT_PORTS_START( slampic )
	PORT_INCLUDE(slammast)

	PORT_MODIFY("IN2")  // players 3 + 4  (player 4 doesn't work in test menu but ok in game)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START4 )

	PORT_MODIFY("IN3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( slampic2 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	//PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW(A):1,2,3")
	PORT_DIPSETTING( 0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING( 0x06, DEF_STR( 1C_2C ) )  // A:cccxxx0x C:xx0xxxxx = coinage (freeplay + "2 coins start" must be off)
	PORT_DIPSETTING( 0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING( 0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING( 0x03, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING( 0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING( 0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING( 0x00, DEF_STR( 4C_1C ) )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW(A):4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW(A):5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW(A):6" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW(A):7")
	PORT_DIPSETTING( 0x40, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, "2 Coins Start" )  // A:000xxx1x C:xx0xxxxx = 2 coins start (other coinage + freeplay must be off)
	PORT_DIPNAME( 0x80, 0x80, "Chuter" ) PORT_DIPLOCATION("SW(A):8")
	PORT_DIPSETTING( 0x80, "Single Chuter" )
	PORT_DIPSETTING( 0x00, "Multi Chuters" )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x07, 0x04, "Game Difficulty" ) PORT_DIPLOCATION("SW(B):1,2,3")
	PORT_DIPSETTING( 0x07, "(0) Extra Easy" )
	PORT_DIPSETTING( 0x06, "(1) Very Easy" )
	PORT_DIPSETTING( 0x05, "(2) Easy" )
	PORT_DIPSETTING( 0x04, "(3) Normal" )
	PORT_DIPSETTING( 0x03, "(4) Hard" )
	PORT_DIPSETTING( 0x02, "(5) Very Hard" )
	PORT_DIPSETTING( 0x01, "(6) Extra Hard" )
	PORT_DIPSETTING( 0x00, "(7) Hardest" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW(B):4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW(B):5" )
	PORT_DIPNAME( 0x20, 0x20, "Join In") PORT_DIPLOCATION("SW(B):6")
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Cabinet" ) PORT_DIPLOCATION("SW(B):7,8")
	PORT_DIPSETTING( 0xc0, "2 Players Cabinet" )
	//PORT_DIPSETTING( 0x80, "Invalid" )              // only coin 1 works, credits both player 1 and 2
	PORT_DIPSETTING( 0x40, "4 Players Cabinet" )
	PORT_DIPSETTING( 0x00, "2x2 Players Cabinet" )  // only coins 1,3 work, 1 credits 1+2, 2 credits 3+4

	PORT_START("DSWC")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW(C):1" )
	PORT_DIPNAME( 0x02, 0x02, "Game Mode" ) PORT_DIPLOCATION("SW(C):2")
	PORT_DIPSETTING( 0x02, "For Business" )
	PORT_DIPSETTING( 0x00, "For Photographing" )  // doesn't seem to do anything?
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW(C):3")
	PORT_DIPSETTING( 0x04, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )  // A:000xxx0x C:xx1xxxxx = freeplay (other coinage + "2 coins start" must be off)
	PORT_DIPNAME( 0x08, 0x08, "Freeze" ) PORT_DIPLOCATION("SW(C):4")
	PORT_DIPSETTING( 0x08, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW(C):5")  // doesn't work
	PORT_DIPSETTING( 0x10, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW(C):6")
	PORT_DIPSETTING( 0x20, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW(C):7")
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x40, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW(C):8" )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START3 )
INPUT_PORTS_END


#define DRAWSPRITE(CODE, COLOR, FLIPX, FLIPY, SX, SY)                                                                                               \
{                                                                                                                                                   \
	if (flip_screen())                                                                                                                              \
		m_gfxdecode->gfx(2)->prio_transpen(bitmap, cliprect, CODE, COLOR, !(FLIPX), !(FLIPY), 512-16-(SX), 256-16-(SY), screen.priority(), 2, 15);  \
	else                                                                                                                                            \
		m_gfxdecode->gfx(2)->prio_transpen(bitmap, cliprect, CODE, COLOR, FLIPX, FLIPY, SX, SY, screen.priority(), 2, 15);                          \
}

void slampic2_state::bootleg_render_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int i, j = 0;
	int last_sprite_offset = 0;
	uint16_t tileno, colour, xpos, ypos;
	uint16_t obj_base = m_cps_a_regs[0];
	uint16_t *sprite_ram = m_bootleg_sprite_ram.get();

	switch (obj_base)
	{
	case 0x9000:
		sprite_ram += m_sprite_base;  // ff2000
		break;
	case 0x9040:
		sprite_ram += m_sprite_base + 0x800;  // ff3000
		break;
	default:
		logerror("Unknown sprite table location: %04x\n", obj_base);
		sprite_ram += m_sprite_base;  // ff2000
	}

	while (last_sprite_offset < m_obj_size / 2)
	{
		if (sprite_ram[last_sprite_offset + 3] == m_sprite_list_end_marker)
			break;
		last_sprite_offset += 4;
	}

	for (i = last_sprite_offset; i > 0; i -= 4)
	{
		xpos   = sprite_ram[j];
		ypos   = sprite_ram[j + 1];
		tileno = sprite_ram[j + 2];
		colour = sprite_ram[j + 3];

		if (colour & 0xff00 )  // block sprite
		{
			int nx = (colour & 0x0f00) >> 8;
			int ny = (colour & 0xf000) >> 12;
			int nxs, nys, sx, sy;
			nx++;
			ny++;

			if (colour & 0x40)  // y flip
			{
				if (colour & 0x20)  // x flip
				{
					for (nys = 0; nys < ny; nys++)
					{
						for (nxs = 0; nxs < nx; nxs++)
						{
							sx = (xpos + nxs * 16) & 0x1ff;
							sy = (ypos + nys * 16) & 0x1ff;
							DRAWSPRITE((tileno & ~0xf) + ((tileno + (nx - 1) - nxs) & 0xf) + 0x10 * (ny - 1 - nys), (colour & 0x1f), 1, 1, sx, sy);
						}
					}
				}
				else  // no x flip
				{
					for (nys = 0; nys < ny; nys++)
					{
						for (nxs = 0; nxs < nx; nxs++)
						{
							sx = (xpos + nxs * 16) & 0x1ff;
							sy = (ypos + nys * 16) & 0x1ff;
							DRAWSPRITE((tileno & ~0xf) + ((tileno + nxs) & 0xf) + 0x10 * (ny - 1 - nys), (colour & 0x1f), 0, 1, sx, sy);
						}
					}
				}
			}
			else  // no y flip
			{
				if (colour & 0x20)  // x flip
				{
					for (nys = 0; nys < ny; nys++)
					{
						for (nxs = 0; nxs<nx; nxs++)
						{
							sx = (xpos + nxs * 16) & 0x1ff;
							sy = (ypos + nys * 16) & 0x1ff;
							DRAWSPRITE((tileno & ~0xf) + ((tileno + (nx - 1) - nxs) & 0xf) + 0x10 * nys, (colour & 0x1f), 1, 0, sx, sy);
						}
					}
				}
				else  // no x flip
				{
					for (nys = 0; nys < ny; nys++)
					{
						for (nxs = 0; nxs < nx; nxs++)
						{
							sx = (xpos + nxs * 16) & 0x1ff;
							sy = (ypos + nys * 16) & 0x1ff;
							DRAWSPRITE((tileno & ~0xf) + ((tileno + nxs) & 0xf) + 0x10 * nys, (colour & 0x1f), 0, 0, sx, sy);
						}
					}
				}
			}
		}
		else
			DRAWSPRITE(tileno, (colour & 0x1f), (colour & 0x20), (colour & 0x40), (xpos & 0x1ff), (ypos & 0x1ff));

		j += 4;
	}
}

void dinopic_state::bootleg_render_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int pos;
	int base = m_sprite_base / 2;
	int last_sprite_offset = 0;
	uint16_t tileno, colour, xpos, ypos;
	bool flipx, flipy;
	uint16_t *sprite_ram = m_bootleg_sprite_ram.get();

	// end of sprite table marker is 0x8000
	// 1st sprite always 0x100e
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
		tileno = sprite_ram[base + pos] & 0x7fff;
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
}


// ************************************************************************* DINOPIC, DINOPIC2, DINOPIC3

/*

Cadillac Bootleg Hardware:

1x 68000p10
1x PIC16c57
1x AD-65
1x OSC 30mhz
1x OSC 24mhz
13x 27c4000 ROMS

*/
ROM_START( dinopic )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "3.bin", 0x000001, 0x80000, CRC(13dfeb08) SHA1(cd2f9dd64f4fabe93901247e36dff3763169716d) )
	ROM_LOAD16_BYTE( "5.bin", 0x000000, 0x80000, CRC(96dfcbf1) SHA1(a8bda6edae2c1b79db7ae8a8976fd2457f874373) )
	ROM_LOAD16_BYTE( "2.bin", 0x100001, 0x80000, CRC(0e4058ba) SHA1(346f9e34ea53dd1bf5cdafa1e38bf2edb09b9a7f) )
	ROM_LOAD16_BYTE( "7.bin", 0x100000, 0x80000, CRC(6133f349) SHA1(d13af99910623f62c090d25372a2253dbc2f8cbe) )

	ROM_REGION( 0x400000, "gfx", 0 ) // same data, different format, except for 8 which is a 99% match (bad ROM?)
	ROM_LOAD64_BYTE( "4.bin",  0x000000, 0x40000, CRC(f3c2c98d) SHA1(98ae51a67fa4159456a4a205eebdd8d1775888d1) )
	ROM_CONTINUE(              0x000004, 0x40000)
	ROM_LOAD64_BYTE( "8.bin",  0x000001, 0x40000, CRC(d574befc) SHA1(56482e7a9aa8439f30e3cf72311495ce677a083d) )
	ROM_CONTINUE(              0x000005, 0x40000)
	ROM_LOAD64_BYTE( "9.bin",  0x000002, 0x40000, CRC(55ef0adc) SHA1(3b5551ae76ae80882d37fc70a1031a57885d6840) )
	ROM_CONTINUE(              0x000006, 0x40000)
	ROM_LOAD64_BYTE( "6.bin",  0x000003, 0x40000, CRC(cc0805fc) SHA1(c512734c28b878a30a0de249929f69784d5d77a1) )
	ROM_CONTINUE(              0x000007, 0x40000)
	ROM_LOAD64_BYTE( "13.bin", 0x200000, 0x40000, CRC(1371f714) SHA1(d2c98096fab08e3d4fd2482e6ebfc970ead656ee) )
	ROM_CONTINUE(              0x200004, 0x40000)
	ROM_LOAD64_BYTE( "12.bin", 0x200001, 0x40000, CRC(b284c4a7) SHA1(166f571e0afa115f8e38ba427b40e30abcfd70ee) )
	ROM_CONTINUE(              0x200005, 0x40000)
	ROM_LOAD64_BYTE( "11.bin", 0x200002, 0x40000, CRC(b7ad3394) SHA1(58dec34d9d991ff2817c8a7847749716abae6c77) )
	ROM_CONTINUE(              0x200006, 0x40000)
	ROM_LOAD64_BYTE( "10.bin", 0x200003, 0x40000, CRC(88847705) SHA1(05dc90067921960e417b7436056a5e1f86abaa1a) )
	ROM_CONTINUE(              0x200007, 0x40000)

	ROM_REGION( 0x28000, "audiocpu", 0 ) /* PIC16c57 - protected, dump isn't valid */
	ROM_LOAD( "pic16c57-rp", 0x00000, 0x2d4c, BAD_DUMP CRC(5a6d393c) SHA1(1391a1590aff5f75bb6fae1c83eddb796b53135d) )

	ROM_REGION( 0x80000, "oki", 0 ) /* OKI6295 samples */
	ROM_LOAD( "1.bin", 0x000000, 0x80000,  CRC(7d921309) SHA1(d51e60e904d302c2516b734189e141aa171b2b82) )
ROM_END

/* this is basically the same set as above, from a different bootleg pcb, with a few extra pal dumps etc.
   the first dump will probably be removed eventually

  CPU
  1x TS68000CP10 (main)
  1x AD-65 (sound)(equivalent to M6295)
  1x PIC16C57-XT/P
  1x A1020B-PL84C
  1x oscillator 24.000MHz (close to main)
  1x oscillator 30.000MHz (close to sound)

  ROMs
  13x 27C4000
  3x GAL20V8A
  3x PALCE16V8H
  1x CAT93C46P

  Note
  1x JAMMA edge connector
  1x 10-pin connector (player 3 inputs)
  1x trimmer (volume)
*/
ROM_START( dinopic2 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "27c4000-m12374r-2.bin", 0x000001, 0x80000, CRC(13dfeb08) SHA1(cd2f9dd64f4fabe93901247e36dff3763169716d) )
	ROM_LOAD16_BYTE( "27c4000-m12481.bin",    0x000000, 0x80000, CRC(96dfcbf1) SHA1(a8bda6edae2c1b79db7ae8a8976fd2457f874373) )
	ROM_LOAD16_BYTE( "27c4000-m12374r-1.bin", 0x100001, 0x80000, CRC(0e4058ba) SHA1(346f9e34ea53dd1bf5cdafa1e38bf2edb09b9a7f) )
	ROM_LOAD16_BYTE( "27c4000-m12374r-3.bin", 0x100000, 0x80000, CRC(6133f349) SHA1(d13af99910623f62c090d25372a2253dbc2f8cbe) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD64_BYTE( "27c4000-m12481-4.bin", 0x000000, 0x40000, CRC(f3c2c98d) SHA1(98ae51a67fa4159456a4a205eebdd8d1775888d1) )
	ROM_CONTINUE(                            0x000004, 0x40000)
	ROM_LOAD64_BYTE( "27c4000-m12481-3.bin", 0x000001, 0x40000, CRC(a0e1f6e0) SHA1(119af72fb6e75933b6d39bc4a8030823ce9b7611) ) // this one is a perfect match, unlike dinopic set
	ROM_CONTINUE(                            0x000005, 0x40000)
	ROM_LOAD64_BYTE( "27c4000-m12481-2.bin", 0x000002, 0x40000, CRC(55ef0adc) SHA1(3b5551ae76ae80882d37fc70a1031a57885d6840) )
	ROM_CONTINUE(                            0x000006, 0x40000)
	ROM_LOAD64_BYTE( "27c4000-m12481-1.bin", 0x000003, 0x40000, CRC(cc0805fc) SHA1(c512734c28b878a30a0de249929f69784d5d77a1) )
	ROM_CONTINUE(                            0x000007, 0x40000)
	ROM_LOAD64_BYTE( "27c4000-m12481-8.bin", 0x200000, 0x40000, CRC(1371f714) SHA1(d2c98096fab08e3d4fd2482e6ebfc970ead656ee) )
	ROM_CONTINUE(                            0x200004, 0x40000)
	ROM_LOAD64_BYTE( "27c4000-m12481-7.bin", 0x200001, 0x40000, CRC(b284c4a7) SHA1(166f571e0afa115f8e38ba427b40e30abcfd70ee) )
	ROM_CONTINUE(                            0x200005, 0x40000)
	ROM_LOAD64_BYTE( "27c4000-m12481-6.bin", 0x200002, 0x40000, CRC(b7ad3394) SHA1(58dec34d9d991ff2817c8a7847749716abae6c77) )
	ROM_CONTINUE(                            0x200006, 0x40000)
	ROM_LOAD64_BYTE( "27c4000-m12481-5.bin", 0x200003, 0x40000, CRC(88847705) SHA1(05dc90067921960e417b7436056a5e1f86abaa1a) )
	ROM_CONTINUE(                            0x200007, 0x40000)

	ROM_REGION( 0x28000, "audiocpu", 0 ) /* PIC16c57 - protected, dump isn't valid */
	ROM_LOAD( "pic16c57-xt.hex", 0x00000, 0x26cc, BAD_DUMP CRC(a6a5eac4) SHA1(2039789084836769180f0bfd230c2553a37e2aaf) )

	ROM_REGION( 0x80000, "oki", 0 ) /* OKI6295 samples */
	ROM_LOAD( "27c4000-m12623.bin", 0x000000, 0x80000,  CRC(7d921309) SHA1(d51e60e904d302c2516b734189e141aa171b2b82) )

	/* pld devices:
	     ______________________________
	    |               16-2           |      (no component reference markings on pcb)
	    |                              |
	  ==         16-1                  |
	  ==                               |
	  ==                               |
	  ==                               |
	  ==                               |
	    |  20-1         20-2 20-3 16-3 |
	    |______________________________|
	*/
	ROM_REGION( 0xc00, "plds", 0 )
	ROM_LOAD( "gal20v8a-1.bin",   0x000, 0x157, CRC(cd99ca47) SHA1(ee1d990fd294aa46f56f31264134251569f6792e) )
	ROM_LOAD( "gal20v8a-2.bin",   0x200, 0x157, CRC(60d016b9) SHA1(add42c763c819f3fe6d7cf3adc7123a52c2a3be9) )
	ROM_LOAD( "gal20v8a-3.bin",   0x400, 0x157, CRC(049b7f4f) SHA1(6c6ea03d9a293db69a8bd10e042ee75e3c01313c) )
	ROM_LOAD( "palce16v8h-1.bin", 0x600, 0x117, CRC(48253c66) SHA1(8c94e655b768c45c3edf6ef39e62e3b7a4e57530) )
	ROM_LOAD( "palce16v8h-2.bin", 0x800, 0x117, CRC(9ae375ba) SHA1(6f227c2a5b1170a41e6419f12d1e1f98edc6f8e5) )
	ROM_LOAD( "palce16v8h-3.bin", 0xa00, 0x117, CRC(b0f10adf) SHA1(5136e9495ef6c37edb0ddf1fe70c0d48c4785c80) )
ROM_END

/*
    Cadillacs and Dinosaurs
    pcb marking: 3M05B
    maincpu roms are same data as dinopic but arranged as 2x 2MB 16-bit mask roms
    Confirmed clocks (measured):
     xtals: 30MHz, 24MHz
     68k = 12MHz (P10 model, overclocked)
     pic = 3.75MHz
     oki = 1MHz

    repair note:
    for any gfx issues, check the 9x Harris CD74HC597E shift registers,
    (4 were dead on the board used for this dump!)
*/
ROM_START( dinopic3 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 ) // = dinopic but arranged differently
	ROM_LOAD16_WORD_SWAP( "tk1-305_27c800.bin", 0x000000, 0x100000, CRC(aa468337) SHA1(496df3bd62cdea0b104f96a7988ad21c94a70c2b) )
	ROM_LOAD16_WORD_SWAP( "tk1-204_27c800.bin", 0x100000, 0x100000, CRC(0efd1ddb) SHA1(093cf7906eda36533c7021329c629ba5a995c5ee) )

	ROM_REGION( 0x400000, "gfx", 0 ) // = dino but arranged differently
	ROM_LOAD64_WORD("tb416-02_27c160.bin", 0x000000, 0x80000, CRC(bfd01d21) SHA1(945f2764b0ca7f9e1569a591363c70207e8efbd0) )
	ROM_CONTINUE( 0x200000, 0x80000 )
	ROM_CONTINUE( 0x000004, 0x80000 )
	ROM_CONTINUE( 0x200004, 0x80000 )
	ROM_LOAD64_WORD("tb415-01_27c160.bin", 0x000002, 0x80000, CRC(ef508ec5) SHA1(ebb521b51d7269b4a9b441bd44b6d5320a72aaaa) )
	ROM_CONTINUE( 0x200002, 0x80000 )
	ROM_CONTINUE( 0x000006, 0x80000 )
	ROM_CONTINUE( 0x200006, 0x80000 )

	// no markings, assume pic16c57, secured
	//ROM_REGION( 0x2000, "audiocpu", 0 )
	//ROM_LOAD( "pic_t1.bin", 0x0000, 0x1007, NO_DUMP )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "ti-i_27c040.bin", 0x000000, 0x80000, CRC(7d921309) SHA1(d51e60e904d302c2516b734189e141aa171b2b82) )  // = dinopic, dinopic2

	/* pld devices:
	     __________________________
	    |                  6       |      (no component reference markings on pcb)
	    |                      7   |
	  ==            5              |
	  ==                           |
	  ==                           |
	  ==                           |
	  ==                         4 |
	    |     1            2  3    |
	    |__________________________|

	#1   palce20v8   next to main cpu        secured                 = dinopic2 "gal20v8a-1.bin", tested ok
	#2   palce20v8   below gfx roms, left    secured                 = dinopic2 "gal20v8a-2.bin", tested ok
	#3   palce20v8   below gfx roms, middle  secured                 = dinopic2 "gal20v8a-3.bin", tested ok
	#4   palce16v8   below gfx roms, right   secured, bruteforce ok
	#5   palce16v8   just below 30M xtal     secured                 = dinopic2 "palce16v8h-1.bin", tested ok
	#6   palce16v8   just to right 30M xtal  secured                 = dinopic2 "palce16v8h-2.bin", tested ok
	#7   a1020b      actel plcc84            unattempted
	(The dinopic2 1-3,5-6 dumps were burnt, tested, and work ok on this board)
	*/
	ROM_REGION( 0xc00, "plds", 0 )
	ROM_LOAD( "1_palce20v8.bin", 0x000, 0x157, CRC(cd99ca47) SHA1(ee1d990fd294aa46f56f31264134251569f6792e) )  // dinopic2
	ROM_LOAD( "2_palce20v8.bin", 0x200, 0x157, CRC(60d016b9) SHA1(add42c763c819f3fe6d7cf3adc7123a52c2a3be9) )  // dinopic2
	ROM_LOAD( "3_palce20v8.bin", 0x400, 0x157, CRC(049b7f4f) SHA1(6c6ea03d9a293db69a8bd10e042ee75e3c01313c) )  // dinopic2
	ROM_LOAD( "4_palce16v8.bin", 0x600, 0x117, CRC(97a67c6d) SHA1(822411f878f1efe462a7a8e93960a1fc5140422e) )
	ROM_LOAD( "5_palce16v8.bin", 0x800, 0x117, CRC(48253c66) SHA1(8c94e655b768c45c3edf6ef39e62e3b7a4e57530) )  // dinopic2
	ROM_LOAD( "6_palce16v8.bin", 0xa00, 0x117, CRC(9ae375ba) SHA1(6f227c2a5b1170a41e6419f12d1e1f98edc6f8e5) )  // dinopic2
ROM_END

/*
    Jurassic 99 (Cadillacs and Dinosaurs bootleg)
    pcb marking: H11F6
    uses a pin compatible EMC EM78P447AP instead of usual PIC 16c57, secured unfortunately so no dump

    Confirmed clocks (measured):
     xtals: 30MHz, 24MHz
     68k  = 12MHz
     em78 = 3.75MHz
     oki  = 1MHz     pin 7 high

      __________________________________________
      |TDA2003(V)  U6295  ROM 30MHz   6116      |
      | 93C46   EM78P447AP            6116      |
    ==                                6116      |
    ==       6116                     6116      |
    ==       6116       P2                      |
    ==                       P3       A1020A    |
    ==         (T)                        P8    |
    ==       62256               P6     6116    |
    ==       62256                      6116    |
    ==       ROM1            P4  P8             |
    ==       ROM2            P5  P8   GFXROM1   |
    ==                    62256  P8             |
      # 68K  P1   24MHz   62256  P8   GFXROM2   |
      |_________________________________________|

    V = volume pot
    # = player 3 connector
    T = test mode button
    U6295 = oki M6295 clone
    68K = MC68HC000FN16 PLCC68
*/
ROM_START( jurassic99 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "210204_rom2.bin",  0x000000, 0x100000, CRC(3f713043) SHA1(90e81c651772e895a56146c986c64ff8c35826ac) )
	ROM_LOAD16_WORD_SWAP( "210105a_rom1.bin", 0x100000, 0x100000, CRC(e6294edf) SHA1(4f9515e2e060dad165f6cb513baee2568c82c1be) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD64_WORD("210101a_cda2.bin", 0x000000, 0x80000, CRC(3f167412) SHA1(2636065b37998d5c3008edc1c13d073305132f4f) )
	ROM_CONTINUE( 0x000004, 0x80000 )
	ROM_CONTINUE( 0x200000, 0x80000 )
	ROM_CONTINUE( 0x200004, 0x80000 )
	ROM_LOAD64_WORD("210102_cdb2.bin",  0x000002, 0x80000, CRC(8a6920d8) SHA1(099bfc37b524f60c82332c83c3f1af411b14e35a) )
	ROM_CONTINUE( 0x000006, 0x80000 )
	ROM_CONTINUE( 0x200002, 0x80000 )
	ROM_CONTINUE( 0x200006, 0x80000 )

	// EMC EM78P447AP, secured
	//ROM_REGION( 0x3000, "audiocpu", 0 )
	//ROM_LOAD( "u28.bin", 0x0000, 0x2020, NO_DUMP )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "21003_u27.bin", 0x000000, 0x80000, CRC(7d921309) SHA1(d51e60e904d302c2516b734189e141aa171b2b82) )  // == dinopic, dinopic2, dinopic3

	/* pld devices:
	U25    ATF20V8B-15PC  1?  secured, bruteforce ok
	U66    ATF16V8B-15PC  2   unsecured
	U100   ATF16V8B-15PC  3   unsecured
	U118   ATF20V8B-15PC  4?  secured, registered, dinopic3 #2 tested ok
	U146   ATF20V8B-15PC  5   secured, registered, dinopic3 #3 tested ok
	U160   ATF16V8B-15PC  6   unsecured
	U124   Actel A1020A   7?  84-pin plcc, unattempted
	U97G   ATF16V8B-15PC  8   secured, registered
	U96G   ATF16V8B-15PC  8   secured, registered
	U98G   ATF16V8B-15PC  8   secured, registered
	U99G   ATF16V8B-15PC  8   secured, registered
	U134G  ATF16V8B-15PC  8?  secured, registered

	3rd column numbers are what's hand-written on each chip
	? = hard to read or rubbed off
	the #8 pals appear to be just 74LS298 equivalents, can be replaced with real 74LS298 (additional 16-pin footprints underneath each chip) or hand-crafted jed
	*/
	ROM_REGION( 0x1600, "plds", 0 )
	ROM_LOAD( "1_atf20v8.u25",   0x0000, 0x157, CRC(cd99ca47) SHA1(ee1d990fd294aa46f56f31264134251569f6792e) )  // == dinopic2 #20-1, dinopic3 #1
	ROM_LOAD( "2_atf16v8.u66",   0x0200, 0x117, CRC(48253c66) SHA1(8c94e655b768c45c3edf6ef39e62e3b7a4e57530) )  // == dinopic2 #16-1, dinopic3 #5
	ROM_LOAD( "3_atf16v8.u100",  0x0400, 0x117, CRC(9ae375ba) SHA1(6f227c2a5b1170a41e6419f12d1e1f98edc6f8e5) )  // == dinopic2 #16-2, dinopic3 #6
	ROM_LOAD( "4_atf20v8.u118",  0x0600, 0x157, CRC(60d016b9) SHA1(add42c763c819f3fe6d7cf3adc7123a52c2a3be9) )  // == dinopic2 #20-2, dinopic3 #2
	ROM_LOAD( "5_atf20v8.u146",  0x0800, 0x157, CRC(049b7f4f) SHA1(6c6ea03d9a293db69a8bd10e042ee75e3c01313c) )  // == dinopic2 #20-3, dinopic3 #3
	ROM_LOAD( "6_atf16v8.u160",  0x0a00, 0x117, CRC(b0f10adf) SHA1(5136e9495ef6c37edb0ddf1fe70c0d48c4785c80) )  // == dinopic2 #16-3
	ROM_LOAD( "8_atf16v8.u96g",  0x0c00, 0x117, BAD_DUMP CRC(11f38ab7) SHA1(4c21b199410a57d6a6a0d9a528b590fefa844856) )  // hand-crafted, works ok on real board
	ROM_LOAD( "8_atf16v8.u97g",  0x0e00, 0x117, BAD_DUMP CRC(11f38ab7) SHA1(4c21b199410a57d6a6a0d9a528b590fefa844856) )
	ROM_LOAD( "8_atf16v8.u98g",  0x1000, 0x117, BAD_DUMP CRC(11f38ab7) SHA1(4c21b199410a57d6a6a0d9a528b590fefa844856) )
	ROM_LOAD( "8_atf16v8.u99g",  0x1200, 0x117, BAD_DUMP CRC(11f38ab7) SHA1(4c21b199410a57d6a6a0d9a528b590fefa844856) )
	ROM_LOAD( "8_atf16v8.u134g", 0x1400, 0x117, BAD_DUMP CRC(11f38ab7) SHA1(4c21b199410a57d6a6a0d9a528b590fefa844856) )
ROM_END


// ************************************************************************* PUNIPIC, PUNIPIC2, PUNIPIC3

/* bootleg with pic, like dinopic / dinopic2 */
ROM_START( punipic )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "cpu5.bin", 0x000000, 0x80000, CRC(c3151563) SHA1(61d3a20c25fea8a94ae6e473a87c21968867cba0) )
	ROM_LOAD16_BYTE( "cpu3.bin", 0x000001, 0x80000, CRC(8c2593ac) SHA1(4261bc72b96c3a5690df35c5d8b71524765693d9) )
	ROM_LOAD16_BYTE( "cpu4.bin", 0x100000, 0x80000, CRC(665a5485) SHA1(c07920d110ca9c35f6cbff94a6a889c17300f994) )
	ROM_LOAD16_BYTE( "cpu2.bin", 0x100001, 0x80000, CRC(d7b13f39) SHA1(eb7cd92b44fdef3b72672b0be6786c526421b627) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD64_BYTE( "gfx9.bin",  0x000000, 0x40000, CRC(9b9a887a) SHA1(8805b36fc18837bd7c64c751b435d72b763b2235) )
	ROM_CONTINUE(                 0x000004, 0x40000)
	ROM_LOAD64_BYTE( "gfx8.bin",  0x000001, 0x40000, CRC(2b94287a) SHA1(815d88e66f537e17550fc0483616f02f7126bfb1) )
	ROM_CONTINUE(                 0x000005, 0x40000)
	ROM_LOAD64_BYTE( "gfx7.bin",  0x000002, 0x40000, CRC(e9bd74f5) SHA1(8ed7098c69d1c70093c99956bf82e532bd6fc7ac) )
	ROM_CONTINUE(                 0x000006, 0x40000)
	ROM_LOAD64_BYTE( "gfx6.bin",  0x000003, 0x40000, CRC(a5e1c8a4) SHA1(3596265a45cf6bbf16c623f0fce7cdc65f9338ad) )
	ROM_CONTINUE(                 0x000007, 0x40000)
	ROM_LOAD64_BYTE( "gfx13.bin", 0x200000, 0x40000, CRC(6d75a193) SHA1(6c5a89517926d7ba4a925a3df800d4bdb8a6938d) )
	ROM_CONTINUE(                 0x200004, 0x40000)
	ROM_LOAD64_BYTE( "gfx12.bin", 0x200001, 0x40000, CRC(a3c205c1) SHA1(6317cc49434dbbb9a249ddd4b50bd791803b3ebe) )
	ROM_CONTINUE(                 0x200005, 0x40000)
	ROM_LOAD64_BYTE( "gfx11.bin", 0x200002, 0x40000, CRC(22f2ec92) SHA1(9186bfc5db71dc5b099c9a985e8fdd5710772d1c) )
	ROM_CONTINUE(                 0x200006, 0x40000)
	ROM_LOAD64_BYTE( "gfx10.bin", 0x200003, 0x40000, CRC(763974c9) SHA1(f9b93c7cf0cb8c212fc21c57c85459b7d2e4e2fd) )
	ROM_CONTINUE(                 0x200007, 0x40000)

	ROM_REGION( 0x28000, "audiocpu", 0 ) /* PIC16c57 - protected */
	ROM_LOAD( "pic16c57", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x200000, "oki", 0 ) /* OKI6295 */
	ROM_LOAD( "sound.bin", 0x000000, 0x80000, CRC(aeec9dc6) SHA1(56fd62e8db8aa96cdd242d8c705849a413567780) )
ROM_END

/* alt bootleg with PIC, same program ROMs as above, bigger graphics ROMs

Punisher
1993, Capcom

This is a bootleg version running on a single PCB.

PCB Layout
----------

|-----------------------------------------|
|    93C46  SOUND   30MHz  PAL            |
|    M6295  PIC16C57                      |
|           6116     PAL   6116           |
|           6116           6116  ACTEL    |
|                          6116  A1020B   |
|J                         6116           |
|A   TEST                  6116           |
|M                         6116           |
|M                                        |
|A                                        |
|    62256  62256        62256  PU13478   |
|     PRG1   PRG2                         |
|     PRG3   PRG4        62256  PU11256   |
|                                      PAL|
|       68000      24MHz        PAL   PAL |
|-----------------------------------------|

Notes:
      Measured clocks
      ---------------
      68000 clock: 12.000MHz (24 / 2)
      M6295 clock: 937.5kHz  (30 / 32), sample rate = 30000000 / 32 / 132
      16C57 clock: 3.75MHz   (30 / 8)   NOTE! 4096-byte internal ROM is protected and can't be read out.
      VSYNC      : 60Hz

      ROMs
      ----
      PRG*  - 4M  mask ROM (read as 27C040)
      SOUND - 4M  mask ROM (read as 27C040)
      PU*   - 16M mask ROM (read as 27C160)

*/
ROM_START( punipic2 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "prg4.bin", 0x000000, 0x80000, CRC(c3151563) SHA1(61d3a20c25fea8a94ae6e473a87c21968867cba0) )
	ROM_LOAD16_BYTE( "prg3.bin", 0x000001, 0x80000, CRC(8c2593ac) SHA1(4261bc72b96c3a5690df35c5d8b71524765693d9) )
	ROM_LOAD16_BYTE( "prg2.bin", 0x100000, 0x80000, CRC(665a5485) SHA1(c07920d110ca9c35f6cbff94a6a889c17300f994) )
	ROM_LOAD16_BYTE( "prg1.bin", 0x100001, 0x80000, CRC(d7b13f39) SHA1(eb7cd92b44fdef3b72672b0be6786c526421b627) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD64_WORD( "pu11256.bin", 0x000000, 0x80000, CRC(6581faea) SHA1(2b0e96998002a1df96c7869ec965257d2ecfb531) )
	ROM_CONTINUE(                   0x200000, 0x80000 )
	ROM_CONTINUE(                   0x000004, 0x80000 )
	ROM_CONTINUE(                   0x200004, 0x80000 )
	ROM_LOAD64_WORD( "pu13478.bin", 0x000002, 0x80000, CRC(61613de4) SHA1(8f8c46ce907be2b4c4715ad88bfd1456818bdd2c) )
	ROM_CONTINUE(                   0x200002, 0x80000 )
	ROM_CONTINUE(                   0x000006, 0x80000 )
	ROM_CONTINUE(                   0x200006, 0x80000 )

	ROM_REGION( 0x28000, "audiocpu", 0 ) /* PIC16c57 - protected */
	ROM_LOAD( "pic16c57", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x200000, "oki", 0 ) /* OKI6295 */
	ROM_LOAD( "sound.bin", 0x000000, 0x80000, CRC(aeec9dc6) SHA1(56fd62e8db8aa96cdd242d8c705849a413567780) )

	ROM_REGION( 0x200000, "user1", 0 ) /* other */
	ROM_LOAD( "93c46.bin", 0x00, 0x80, CRC(36ab4e7d) SHA1(60bea43051d86d9aefcbb7a390cf0c7d8b905a4b) )
ROM_END

/* the readme doesn't actually state this has a PIC, and there's no sound ROM
   so it might be different */
ROM_START( punipic3 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "psb5b.rom", 0x000000, 0x80000, CRC(58f42c05) SHA1(e243928f0bbecdf2a8d07cf4a6fdea4440e46c01) )
	ROM_LOAD16_BYTE( "psb3b.rom", 0x000001, 0x80000, CRC(90113db4) SHA1(4decc203ae3ee4abcb2e017f11cd20eae2abf3f3) )
	ROM_LOAD16_BYTE( "psb4a.rom", 0x100000, 0x80000, CRC(665a5485) SHA1(c07920d110ca9c35f6cbff94a6a889c17300f994) )
	ROM_LOAD16_BYTE( "psb2a.rom", 0x100001, 0x80000, CRC(d7b13f39) SHA1(eb7cd92b44fdef3b72672b0be6786c526421b627) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD64_WORD( "psb-a.rom", 0x000000, 0x80000, CRC(57f0f5e3) SHA1(130b6e92181994bbe874261e0895db65d4f3d5d1) )
	ROM_CONTINUE(                 0x000004, 0x80000 )
	ROM_CONTINUE(                 0x200000, 0x80000 )
	ROM_CONTINUE(                 0x200004, 0x80000 )
	ROM_LOAD64_WORD( "psb-b.rom", 0x000002, 0x80000, CRC(d9eb867e) SHA1(9b6eaa4a780da5c9cf09658fcab3a1a6f632c2f4) )
	ROM_CONTINUE(                 0x000006, 0x80000 )
	ROM_CONTINUE(                 0x200002, 0x80000 )
	ROM_CONTINUE(                 0x200006, 0x80000 )

	ROM_REGION( 0x28000, "audiocpu", ROMREGION_ERASE00 ) /* PIC16c57 (maybe, not listed in readme) */
	//ROM_LOAD( "pic16c57", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x200000, "oki", ROMREGION_ERASE00 ) /* OKI6295 */
	//ROM_LOAD( "sound.bin", 0x000000, 0x80000, CRC(aeec9dc6) SHA1(56fd62e8db8aa96cdd242d8c705849a413567780) )
ROM_END


// ************************************************************************* SLAMPIC, SLAMPIC2

ROM_START( slampic )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "5.bin", 0x000000, 0x80000,  CRC(7dba63cd) SHA1(222e781ffc40c5c23f5789c0682f549f00beeb8d) )
	ROM_LOAD16_BYTE( "3.bin", 0x000001, 0x80000,  CRC(d86671f3) SHA1(d95fae27b0f4d3688f1c2229c9d3780724a870a8) )
	ROM_LOAD16_BYTE( "4.bin", 0x100000, 0x80000,  CRC(d14d0e42) SHA1(b60c44193b247dc4856bd36d69cbbe9dcb2d21a7) )
	ROM_LOAD16_BYTE( "2.bin", 0x100001, 0x80000,  CRC(38063cd8) SHA1(e647433414ff4fdc0b2c4c7036b8995a95289efa) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROM_LOAD64_BYTE( "9.bin",  0x000000, 0x40000, CRC(dc140351) SHA1(0e69e1c8ded85ba26eb8236449d38ead0243ae78) )
	ROM_CONTINUE(              0x000004, 0x40000)
	ROM_LOAD64_BYTE( "8.bin",  0x000001, 0x40000, CRC(9ae88035) SHA1(3329e9582ca052940e115e759bb3d96f4a9c87fa) )
	ROM_CONTINUE(              0x000005, 0x40000)
	ROM_LOAD64_BYTE( "7.bin",  0x000002, 0x40000, CRC(5321f759) SHA1(7538a6587cf1077921b938070185e0a0ce5ca922) )
	ROM_CONTINUE(              0x000006, 0x40000)
	ROM_LOAD64_BYTE( "6.bin",  0x000003, 0x40000, CRC(c8eb5f76) SHA1(a361d2d2dfe71789736666b744ae5f1e4bf7e1b2) )
	ROM_CONTINUE(              0x000007, 0x40000)
	ROM_LOAD64_BYTE( "17.bin", 0x200000, 0x40000, CRC(21652214) SHA1(039335251f6553c4f36e2d33e8b43fb5726e833e) )
	ROM_CONTINUE(              0x200004, 0x40000)
	ROM_LOAD64_BYTE( "16.bin", 0x200001, 0x40000, CRC(d49d2eb0) SHA1(1af01575340730166975be93bae438e2b0492f98) )
	ROM_CONTINUE(              0x200005, 0x40000)
	ROM_LOAD64_BYTE( "15.bin", 0x200002, 0x40000, CRC(0d98bfd6) SHA1(c11fbf555880a933a4cbf6faa517f59f8443304f) )
	ROM_CONTINUE(              0x200006, 0x40000)
	ROM_LOAD64_BYTE( "14.bin", 0x200003, 0x40000, CRC(807284f1) SHA1(c747c3eaade31c2633fb0a0682dbea900bf2b092) )
	ROM_CONTINUE(              0x200007, 0x40000)
	ROM_LOAD64_BYTE( "13.bin", 0x400000, 0x40000, CRC(293579c5) SHA1(9adafe29664b20834365b339f7ae379cdb9ee138) )
	ROM_CONTINUE(              0x400004, 0x40000)
	ROM_LOAD64_BYTE( "12.bin", 0x400001, 0x40000, CRC(c3727ce7) SHA1(c4abc2c59152c59a45f85393e9525505bc2c9e6e) )
	ROM_CONTINUE(              0x400005, 0x40000)
	ROM_LOAD64_BYTE( "11.bin", 0x400002, 0x40000, CRC(2919883b) SHA1(44ad979daae673c77b3157d2b352797d4ad0ec24) )
	ROM_CONTINUE(              0x400006, 0x40000)
	ROM_LOAD64_BYTE( "10.bin", 0x400003, 0x40000, CRC(f538e620) SHA1(354cd0548b067dfc8782bbe13b0a9c2083dbd290) )
	ROM_CONTINUE(              0x400007, 0x40000)

	ROM_REGION( 0x2000, "audiocpu", 0 ) /* PIC16c57 - protected, dump isn't valid */
	ROM_LOAD( "pic16c57-xt-p.bin", 0x00000, 0x2000, BAD_DUMP CRC(aeae5ccc) SHA1(553afb68f7bf130cdf34e24512f72b4ecef1576f) )

	ROM_REGION( 0x80000, "oki", 0 ) /* OKI6295 samples */
	ROM_LOAD( "18.bin", 0x00000, 0x80000, CRC(73a0c11c) SHA1(a66e1a964313e21c4436200d36c598dcb277cd34) )

	ROM_REGION( 0x20000, "user1", 0 ) // not in the dump, but needed for protection
	ROM_LOAD( "mb_qa.5k", 0x00000, 0x20000, CRC(e21a03c4) SHA1(98c03fd2c9b6bf8a4fc25a4edca87fff7c3c3819) )

	/* pld devices:
	     ________________________
	    |              1         |      (no component reference markings on pcb)
	    |                        |
	  ==         2               |
	  ==                         |
	  ==                         |
	  ==                         |
	  ==                         |
	    |  3           4   5   6 |
	    |________________________|
	*/
	ROM_REGION( 0xc00, "plds", 0 )
	ROM_LOAD( "1_palce16v8.bin", 0x000, 0x117, CRC(bac89609) SHA1(4796a476843b448059ed28ef735d9c6a7886fdef) )  // all unsecured
	ROM_LOAD( "2_palce16v8.bin", 0x200, 0x117, CRC(680edfd5) SHA1(b1b6ad4e2c4e23c384de32326986a58bc74a12ca) )
	ROM_LOAD( "3_palce20v8.bin", 0x400, 0x157, CRC(f1fe9368) SHA1(821b5ad60cd1aa1f325fd07af3b9c4d116aa227e) )
	ROM_LOAD( "4_palce20v8.bin", 0x600, 0x157, CRC(20946530) SHA1(307ad5644aca89d1462510f12fd10187a50376b6) )
	ROM_LOAD( "5_palce20v8.bin", 0x800, 0x157, CRC(44df0cc6) SHA1(b6c4249d6d173792d2736654c93dc30c15c9a4fb) )
	ROM_LOAD( "6_palce16v8.bin", 0xa00, 0x117, CRC(12516583) SHA1(990225b1a8fecb2b95011f25d7d3cc35840103f3) )
ROM_END

/*
    Saturday Night Slam Masters: single board bootleg

    CPU
    1x  MC68000P10           main cpu

    GFX
    1x  Custom QFP 160-pin   "PLUS-B A37558.6 9325"   CPS-B-xx clone?

    RAM
    2x  NEC D431000ACZ-70L   main ram   1Mbit (128Kx8) SRAM 70ns
    2x  SRM20256LM12         gfx?       256Kbit (32Kx8) SRAM 120ns SOP28   (mounted on SOP->DIP adapter pcbs)
    6x  T6116S45L            gfx?       16Kbit (2Kx8) SRAM 45ns
    4x  T6116S35L            gfx?       16Kbit (2Kx8) SRAM 35ns

    ROMS
    4x   27C040-15 EPROM      main rom   4Mbit (512Kx8)
    16x  MX27C4000PC-15 OTP   gfx        4Mbit (512Kx8)
    1x   27C020-15 EPROM      sound      2Mbit (256Kx8)
    2x   MX27C4000PC-15 OTP   sound      4Mbit (512Kx8)
    1x   AM27512DC EPROM      ?          512Kbit (64kx8)  1983!

    PLD
    1x   TPC1020AFN-084C
    14x  PALCE16V8H-25PC/4
    4x   PALCE20V8H-25PC/4
    1x   PALCE22V10H-25PC/4

    SOUND
    1x  PIC16C57-XT/P     sound cpu
    1x  TD735             sample player  (Oki MSM6295 clone)
    1x  NEC uPC1242H      power amp
    1x  LM324N            op amp

    MISC
    1x  16MHz xtal
    1x  10MHz xtal
    1x  PST518A             reset generator
    3x  8 pos dipswitch
    2x  10-pin connectors   player 3 & 4 inputs
    No eeprom!

    INPUTS
    CN3: Player 3
    CN4: Player 4

    1   gnd
    2   nc
    3   right
    4   left
    5   down
    6   up
    7   btn 1
    8   btn 2
    9   coin
    10  start

    player 3 btn 3:  jamma 25  (non-std, player 1 btn 4/neogeo btn D)
    player 4 btn 4:  jamma ac  (non-std, player 2 btn 4/neogeo btn D)


    h/w issues compared to original game (slammast)
    -----------------------------------------------
    these are present on the real board so are not emulation issues:

    * On the title screen, the blue crystal-like effect behind the main "slammasters" logo is missing.
    * The bottom and side crowd animations have missing frames.
    * The foreground ropes of the wrestling ring are glitchy and don't always line up properly with the end sections,
        the original game draws all 3 ropes on scroll2 instead of with sprites when 4 players are on screen,
        this bootleg draws the top red rope on scroll2 even with 2 players on screen.
    * Player 3/4 inputs don't work in test menu (except both btn 3), seems test menu code hasn't been hacked to use the different ports.
    * No eeprom on the board, has dipswitches instead.
    * Crashes if "memory test" is attempted in test menu.
    * Flip screen dipswitch does nothing (but change is shown in test menu).
*/
ROM_START( slampic2 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )
	ROM_LOAD16_BYTE( "4.bin", 0x000000, 0x80000, CRC(105cfefd) SHA1(83a34bc83782ae04be1665a91b44625d24f99466) )
	ROM_LOAD16_BYTE( "2.bin", 0x000001, 0x80000, CRC(6026c95e) SHA1(8503587941ad14a757ad337dc36591fedcddaa41) )
	ROM_LOAD16_BYTE( "3.bin", 0x100000, 0x80000, CRC(0effa84a) SHA1(03342bd4cb1de8652bab874c11cb1ecb69a339c7) )
	ROM_LOAD16_BYTE( "1.bin", 0x100001, 0x80000, CRC(8fcb683a) SHA1(4648656bed010a0c27748df4a78c73c5cae07442) )

	ROM_REGION( 0x600000, "gfx", 0 )  // overall just 2 bytes diff vs official set (slammast)
	ROM_LOAD64_BYTE( "rom7.bin",  0x000000, 0x40000, CRC(b5669ad3) SHA1(ceb3d2a6d6c1443a40d37c8f2ba5f3cf03315908) )  // ~ slampic 9.bin  [1/2] 99.914551% [2/2] IDENTICAL
	ROM_CONTINUE(                 0x000004, 0x40000)
	ROM_LOAD64_BYTE( "rom8.bin",  0x000001, 0x40000, CRC(f07a6085) SHA1(68795a0f5151a45f053059bc2fe4a622d5e10d8a) )  // ~ slampic 8.bin  [1/2] 99.999237% [2/2] IDENTICAL  2 bytes diff
	ROM_CONTINUE(                 0x000005, 0x40000)
	ROM_LOAD64_BYTE( "rom5.bin",  0x000002, 0x40000, CRC(5321f759) SHA1(7538a6587cf1077921b938070185e0a0ce5ca922) )  // = slampic 7.bin
	ROM_CONTINUE(                 0x000006, 0x40000)
	ROM_LOAD64_BYTE( "rom6.bin",  0x000003, 0x40000, CRC(c8eb5f76) SHA1(a361d2d2dfe71789736666b744ae5f1e4bf7e1b2) )  // = slampic 6.bin
	ROM_CONTINUE(                 0x000007, 0x40000)
	ROM_LOAD64_BYTE( "rom11.bin", 0x200000, 0x40000, CRC(21652214) SHA1(039335251f6553c4f36e2d33e8b43fb5726e833e) )  // = slampic 17.bin
	ROM_CONTINUE(                 0x200004, 0x40000)
	ROM_LOAD64_BYTE( "rom12.bin", 0x200001, 0x40000, CRC(d49d2eb0) SHA1(1af01575340730166975be93bae438e2b0492f98) )  // = slampic 16.bin
	ROM_CONTINUE(                 0x200005, 0x40000)
	ROM_LOAD64_BYTE( "rom9.bin",  0x200002, 0x40000, CRC(0d98bfd6) SHA1(c11fbf555880a933a4cbf6faa517f59f8443304f) )  // = slampic 15.bin
	ROM_CONTINUE(                 0x200006, 0x40000)
	ROM_LOAD64_BYTE( "rom10.bin", 0x200003, 0x40000, CRC(807284f1) SHA1(c747c3eaade31c2633fb0a0682dbea900bf2b092) )  // = slampic 14.bin
	ROM_CONTINUE(                 0x200007, 0x40000)
	ROM_LOAD64_BYTE( "rom15.bin", 0x400000, 0x40000, CRC(293579c5) SHA1(9adafe29664b20834365b339f7ae379cdb9ee138) )  // = slampic 13.bin
	ROM_CONTINUE(                 0x400004, 0x40000)
	ROM_LOAD64_BYTE( "rom16.bin", 0x400001, 0x40000, CRC(c3727ce7) SHA1(c4abc2c59152c59a45f85393e9525505bc2c9e6e) )  // = slampic 12.bin
	ROM_CONTINUE(                 0x400005, 0x40000)
	ROM_LOAD64_BYTE( "rom13.bin", 0x400002, 0x40000, CRC(2919883b) SHA1(44ad979daae673c77b3157d2b352797d4ad0ec24) )  // = slampic 11.bin
	ROM_CONTINUE(                 0x400006, 0x40000)
	ROM_LOAD64_BYTE( "rom14.bin", 0x400003, 0x40000, CRC(f538e620) SHA1(354cd0548b067dfc8782bbe13b0a9c2083dbd290) )  // = slampic 10.bin
	ROM_CONTINUE(                 0x400007, 0x40000)

	// this region contains first 0x40000 bytes of 1st 0x200000 region (rom7/8/5/6.bin)
	//   then,              last 0x1c0000 bytes of 3rd 0x200000 region (rom15/16/13/14.bin)
	// game doesn't seem to need it ???
	// ROM_LOAD64_BYTE( "rom1.bin", 0x600000, 0x40000, CRC(8f2c41a4) SHA1(097edfbe9c14f299727fe53e4b83a674f7501561) )  // ~ 15.bin
	// ROM_CONTINUE(                0x600004, 0x40000)
	// ROM_LOAD64_BYTE( "rom2.bin", 0x600001, 0x40000, CRC(65f3dc43) SHA1(01d9ec3ef913ae235bd98ee6921c366f34547d36) )  // ~ 16.bin
	// ROM_CONTINUE(                0x600005, 0x40000)
	// ROM_LOAD64_BYTE( "rom3.bin", 0x600002, 0x40000, CRC(3cd830e3) SHA1(ac1f055c9516efd01bc66b18313cb315705bd2b0) )  // ~ 13.bin
	// ROM_CONTINUE(                0x600006, 0x40000)
	// ROM_LOAD64_BYTE( "rom4.bin", 0x600003, 0x40000, CRC(9683dd30) SHA1(8b258b386baff5e06a9b7f176c49507f7e531b95) )  // ~ 14.bin
	// ROM_CONTINUE(                0x600007, 0x40000)

	ROM_REGION( 0x2000, "audiocpu", 0 ) // NO DUMP  -  protected PIC
	ROM_LOAD( "pic_u33.bin", 0x0000, 0x1007, BAD_DUMP CRC(6dba4094) SHA1(ca3362de83205fc6563d16a59b8e6e4bb7ebf4a6) )

	ROM_REGION( 0x140000, "oki", 0 )
	ROM_LOAD( "v1.bin", 0x000000, 0x40000, CRC(8962b469) SHA1(91dc12610a0b780ee2b314cd346182d97279c175) )  // 27c020 w/ sticker "7"
	ROM_LOAD( "v2.bin", 0x040000, 0x80000, CRC(6687df38) SHA1(d1015ae089fab5c5b4d1ab51b20f3aa6b77ed348) )  // 27c4000
	ROM_LOAD( "v3.bin", 0x0c0000, 0x80000, CRC(5782baee) SHA1(c01f8cd08d0c7b78c010ce3f1567383b7435de9f) )  // 27c4000

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "24.bin", 0x00000, 0x10000, CRC(13ea1c44) SHA1(5b05fe4c3920e33d94fac5f59e09ff14b3e427fe) )  // = various sf2 bootlegs (sf2ebbl etc.) "unknown (bootleg priority?)"

	/* pld devices:
	#1    P7    palce16V8        todo...
	#2    P1    palce16V8        secured, bruteforce ok
	#3    P18   palce16V8        todo...
	#4    P17   palce16V8        todo...
	#5    P15   palce16V8        todo...
	#6    P4    palce16V8        todo...
	#7    P5    palce16V8        todo...
	#8    P6    palce16V8        todo...
	#9    P10   palce20V8        todo...
	#10   P8    palce20V8        secured
	#11   P14   palce20V8        todo...
	#12   P9    palce16V8        todo...
	#13   P11   palce16V8        todo...
	#14   P16   palce16V8        todo...
	#15   P2    palce16V8        todo...
	#16   P21   palce16V8        todo...
	#17   P3    palce16V8        todo...
	#18   P12   palce20V8        todo...
	#19   P13   palce22V10       todo...
	#20   U14   tpc1020afn-084c  unattempted
	*/
	ROM_REGION( 0x0200, "plds", 0 )  // sound
	ROM_LOAD( "2_gal16v8.p1", 0x0000, 0x0117, CRC(a944ff96) SHA1(2871a1c70b91fcd8628e63497afa1275f3a27f93) )
ROM_END

ROM_START( wofpic )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1.4m", 0x000001, 0x80000, CRC(d2ae67a8) SHA1(cb1a9f8e6999598b9a1a7c449f147a0c5773b154) )
	ROM_LOAD16_BYTE( "2.4m", 0x000000, 0x80000, CRC(61fd0a01) SHA1(a7b5bdddd7b31645e33314c1d3649e1506cecfea) )
	ROM_LOAD16_BYTE( "3.1m", 0x100001, 0x20000, CRC(739379be) SHA1(897f61527213902fda04bc28339f1f4278bf5ae9) ) // 1xxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "4.1m", 0x100000, 0x20000, CRC(fe5eee87) SHA1(be1230f64c1e59ae3ff3e58593070613966ac79d) ) // 11xxxxxxxxxxxxxxx = 0x00

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD64_BYTE( "m12073-2", 0x000000, 0x40000, CRC(c8dcaa95) SHA1(bcaeaefd40ffa1b32e80457cffcc1ceab461af1d) )
	ROM_CONTINUE(                0x000004, 0x40000)
	ROM_LOAD64_BYTE( "m12223-2", 0x000001, 0x40000, CRC(1ab0000c) SHA1(0d0004cc1725c38d140ecb8dc9666361b2d3e607) )
	ROM_CONTINUE(                0x000005, 0x40000)
	ROM_LOAD64_BYTE( "m12223-1", 0x000002, 0x40000, CRC(8425ff6b) SHA1(9a051089c2a492b8c63484582f95c578704b6820) )
	ROM_CONTINUE(                0x000006, 0x40000)
	ROM_LOAD64_BYTE( "m12073-1", 0x000003, 0x40000, CRC(24ce197b) SHA1(0ccdbd6f6a30e6d1479f8702c3e8561b16303550) )
	ROM_CONTINUE(                0x000007, 0x40000)
	ROM_LOAD64_BYTE( "m12073-6", 0x200000, 0x40000, CRC(9d20ef9b) SHA1(cbf3cb6bd7a73312e5061082554f2e17aae08621) )
	ROM_CONTINUE(                0x200004, 0x40000)
	ROM_LOAD64_BYTE( "m12073-5", 0x200001, 0x40000, CRC(90c93dd2) SHA1(d3d2b0bcbcbb21a41f986eb752ab114697eb9402) )
	ROM_CONTINUE(                0x200005, 0x40000)
	ROM_LOAD64_BYTE( "m12073-4", 0x200002, 0x40000, CRC(219fd7e2) SHA1(af765eb7b275ed541c08e243b22b5c9f54c1a8ec) )
	ROM_CONTINUE(                0x200006, 0x40000)
	ROM_LOAD64_BYTE( "m12073-3", 0x200003, 0x40000, CRC(efc17c9a) SHA1(26429a9039bb249e17945508c16645c82f7f412a) )
	ROM_CONTINUE(                0x200007, 0x40000)

	ROM_REGION( 0x2000, "audiocpu", 0 ) // NO DUMP  -  protected PIC
	ROM_LOAD( "pic.bin", 0x0000, 0x1007, NO_DUMP )

	ROM_REGION( 0x080000, "oki", 0 )
	ROM_LOAD( "ma12073.4mm", 0x00000, 0x80000, CRC(ac421276) SHA1(56786c23b0d96e1a2540e7269aa20fd390f98b5b) )
ROM_END

} // anonymous namespace


// ************************************************************************* DRIVER MACROS

GAME( 1993,  dinopic,    dino,      dinopic,   dino,      dinopic_state,     init_dinopic,   ROT0,  "bootleg",  "Cadillacs and Dinosaurs (bootleg with PIC16C57, set 1)",  MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )     // 930201 ETC
GAME( 1993,  dinopic2,   dino,      dinopic,   dino,      dinopic_state,     init_dinopic,   ROT0,  "bootleg",  "Cadillacs and Dinosaurs (bootleg with PIC16C57, set 2)",  MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )  // 930201 ETC
GAME( 1993,  dinopic3,   dino,      dinopic,   dino,      dinopic_state,     init_dinopic,   ROT0,  "bootleg",  "Cadillacs and Dinosaurs (bootleg with PIC16C57, set 3)",  MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )     // 930201 ETC
GAME( 1993,  jurassic99, dino,      dinopic,   dino,      dinopic_state,     init_dinopic,   ROT0,  "bootleg",  "Jurassic 99 (Cadillacs and Dinosaurs bootleg with EM78P447AP)",  MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )     // 930201 ?

GAME( 1993,  punipic,    punisher,  punipic,   punisher,  cps1bl_pic_state,  init_punipic,   ROT0,  "bootleg",  "The Punisher (bootleg with PIC16C57, set 1)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )  // 930422 ETC
GAME( 1993,  punipic2,   punisher,  punipic,   punisher,  cps1bl_pic_state,  init_punipic,   ROT0,  "bootleg",  "The Punisher (bootleg with PIC16C57, set 2)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )  // 930422 ETC
GAME( 1993,  punipic3,   punisher,  punipic,   punisher,  cps1bl_pic_state,  init_punipic3,  ROT0,  "bootleg",  "The Punisher (bootleg with PIC16C57, set 3)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )  // 930422 ETC

GAME( 1993,  slampic,    slammast,  slampic,   slampic,   cps1bl_pic_state,  init_dinopic,   ROT0,  "bootleg",  "Saturday Night Slam Masters (bootleg with PIC16C57, set 1)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )  // 930713 ETC
GAME( 1993,  slampic2,   slammast,  slampic2,  slampic2,  slampic2_state,    init_slampic2,  ROT0,  "bootleg",  "Saturday Night Slam Masters (bootleg with PIC16C57, set 2)",  MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )  // 930713 ETC

GAME( 1992,  wofpic,     wof,       wofpic,    wof,       wofpic_state,      init_wofpic,    ROT0,  "bootleg",  "Warriors of Fate (bootleg with PIC16C57)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )  // 921002 ETC
