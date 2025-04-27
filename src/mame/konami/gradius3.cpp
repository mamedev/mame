// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Gradius 3 (GX945) (c) 1989 Konami

    driver by Nicola Salmoria

    This board uses the well known 052109 051962 custom gfx chips, however unlike
    all other games they fetch gfx data from RAM. The gfx ROMs are memory mapped
    on cpu B and the needed parts are copied to RAM at run time.

    There's also something wrong in the way tile banks are implemented in
    k052109.cpp. They don't seem to be used by this game.

***************************************************************************/

#include "emu.h"

#include "k052109.h"
#include "k051960.h"
#include "konamipt.h"
#include "konami_helper.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "sound/k007232.h"
#include "sound/ymopm.h"

#include "emupal.h"
#include "speaker.h"


namespace {

class gradius3_state : public driver_device
{
public:
	gradius3_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_gfxram(*this, "k052109"),
		m_gfxrom(*this, "k051960"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_subcpu(*this, "sub"),
		m_k007232(*this, "k007232"),
		m_k052109(*this, "k052109"),
		m_k051960(*this, "k051960")
	{ }

	void gradius3(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	/* memory pointers */
	required_shared_ptr<uint16_t> m_gfxram;
	required_region_ptr<uint8_t> m_gfxrom;

	/* misc */
	int         m_priority = 0;
	int         m_irqAen = 0;
	int         m_irqBmask = 0;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_subcpu;
	required_device<k007232_device> m_k007232;
	required_device<k052109_device> m_k052109;
	required_device<k051960_device> m_k051960;

	uint16_t k052109_halfword_r(offs_t offset);
	void k052109_halfword_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void cpuA_ctrl_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void cpuB_irqenable_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void cpuB_irqtrigger_w(uint16_t data);
	void sound_irq_w(uint16_t data);
	uint16_t gradius3_gfxrom_r(offs_t offset);
	void gradius3_gfxram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void sound_bank_w(uint8_t data);
	uint32_t screen_update_gradius3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(cpuA_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(gradius3_sub_scanline);
	void gradius3_postload();
	void volume_callback(uint8_t data);
	K052109_CB_MEMBER(tile_callback);
	K051960_CB_MEMBER(sprite_callback);
	void gradius3_map(address_map &map) ATTR_COLD;
	void gradius3_map2(address_map &map) ATTR_COLD;
	void gradius3_s_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

K052109_CB_MEMBER(gradius3_state::tile_callback)
{
	static const int layer_colorbase[] = { 0 / 16, 512 / 16, 768 / 16 };

	/* (color & 0x02) is flip y handled internally by the 052109 */
	*code |= ((*color & 0x01) << 8) | ((*color & 0x1c) << 7);
	*color = layer_colorbase[layer] + ((*color & 0xe0) >> 5);
}


/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

K051960_CB_MEMBER(gradius3_state::sprite_callback)
{
	enum { sprite_colorbase = 256 / 16 };

	#define L0 GFX_PMASK_1
	#define L1 GFX_PMASK_2
	#define L2 GFX_PMASK_4
	static const int primask[2][4] =
	{
		{ L0|L2, L0, L0|L2, L0|L1|L2 },
		{ L1|L2, L2, 0,     L0|L1|L2 }
	};
	#undef L0
	#undef L1
	#undef L2

	int pri = ((*color & 0x60) >> 5);

	if (m_priority == 0)
		*priority = primask[0][pri];
	else
		*priority = primask[1][pri];

	*code |= (*color & 0x01) << 13;
	*color = sprite_colorbase + ((*color & 0x1e) >> 1);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void gradius3_state::gradius3_postload()
{
	m_k052109->gfx(0)->mark_all_dirty();
}

void gradius3_state::video_start()
{
	machine().save().register_postload(save_prepost_delegate(FUNC(gradius3_state::gradius3_postload), this));
}


/***************************************************************************

  Display refresh

***************************************************************************/

uint32_t gradius3_state::screen_update_gradius3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* TODO: this kludge enforces the char banks. For some reason, they don't work otherwise. */
	m_k052109->write(0x1d80, 0x10);
	m_k052109->write(0x1f00, 0x32);

	m_k052109->tilemap_update();

	screen.priority().fill(0, cliprect);
	if (m_priority == 0)
	{
		m_k052109->tilemap_draw(screen, bitmap, cliprect, 1, TILEMAP_DRAW_OPAQUE, 2);
		m_k052109->tilemap_draw(screen, bitmap, cliprect, 2, 0, 4);
		m_k052109->tilemap_draw(screen, bitmap, cliprect, 0, 0, 1);
	}
	else
	{
		m_k052109->tilemap_draw(screen, bitmap, cliprect, 0, TILEMAP_DRAW_OPAQUE, 1);
		m_k052109->tilemap_draw(screen, bitmap, cliprect, 1, 0, 2);
		m_k052109->tilemap_draw(screen, bitmap, cliprect, 2, 0, 4);
	}

	m_k051960->k051960_sprites_draw(bitmap, cliprect, screen.priority(), -1, -1);
	return 0;
}


/***************************************************************************

  Memory handlers

***************************************************************************/

uint16_t gradius3_state::gradius3_gfxrom_r(offs_t offset)
{
	return (m_gfxrom[2 * offset + 1] << 8) | m_gfxrom[2 * offset];
}

void gradius3_state::gradius3_gfxram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	int oldword = m_gfxram[offset];

	COMBINE_DATA(&m_gfxram[offset]);

	if (oldword != m_gfxram[offset])
		m_k052109->gfx(0)->mark_dirty(offset / 16);
}


uint16_t gradius3_state::k052109_halfword_r(offs_t offset)
{
	return m_k052109->read(offset);
}

void gradius3_state::k052109_halfword_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_k052109->write(offset, data & 0xff);

	/* is this a bug in the game or something else? */
	if (!ACCESSING_BITS_0_7)
	{
		m_k052109->write(offset, (data >> 8) & 0xff);
		// logerror("%s half %04x = %04x\n",machine().describe_context(),offset,data);
	}
}

void gradius3_state::cpuA_ctrl_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_8_15)
	{
		data >>= 8;

		/* bits 0-1 are coin counters */
		machine().bookkeeping().coin_counter_w(0, data & 0x01);
		machine().bookkeeping().coin_counter_w(1, data & 0x02);

		/* bit 2 selects layer priority */
		m_priority = data & 0x04;

		/* bit 3 enables cpu B */
		m_subcpu->set_input_line(INPUT_LINE_RESET, (data & 0x08) ? CLEAR_LINE : ASSERT_LINE);

		/* bit 5 enables irq */
		m_irqAen = data & 0x20;

		/* other bits unknown */
		//logerror("%s: write %04x to c0000\n",machine().describe_context(),data);
	}
}

void gradius3_state::cpuB_irqenable_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_8_15)
		m_irqBmask = (data >> 8) & 0x07;
}

INTERRUPT_GEN_MEMBER(gradius3_state::cpuA_interrupt)
{
	if (m_irqAen)
		device.execute().set_input_line(2, HOLD_LINE);
}


TIMER_DEVICE_CALLBACK_MEMBER(gradius3_state::gradius3_sub_scanline)
{
	int scanline = param;

	if(scanline == 240 && m_irqBmask & 1) // vblank-out irq
		m_subcpu->set_input_line(1, HOLD_LINE);

	if(scanline ==  16 && m_irqBmask & 2) // sprite end DMA irq
		m_subcpu->set_input_line(2, HOLD_LINE);
}

void gradius3_state::cpuB_irqtrigger_w(uint16_t data)
{
	if (m_irqBmask & 4)
	{
		logerror("%04x trigger cpu B irq 4 %02x\n",m_maincpu->pc(),data);
		m_subcpu->set_input_line(4, HOLD_LINE);
	}
	else
		logerror("%04x MISSED cpu B irq 4 %02x\n",m_maincpu->pc(),data);
}

void gradius3_state::sound_irq_w(uint16_t data)
{
	m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0xff); // Z80
}

void gradius3_state::sound_bank_w(uint8_t data)
{
	int bank_A, bank_B;

	/* banks # for the 007232 (chip 1) */
	bank_A = ((data >> 0) & 0x03);
	bank_B = ((data >> 2) & 0x03);
	m_k007232->set_bank(bank_A, bank_B);
}


/***************************************************************************

  Address maps

***************************************************************************/

void gradius3_state::gradius3_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x040000, 0x043fff).ram();
	map(0x080000, 0x080fff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0x0c0000, 0x0c0001).w(FUNC(gradius3_state::cpuA_ctrl_w));  /* halt cpu B, irq enable, priority, coin counters, other? */
	map(0x0c8000, 0x0c8001).portr("SYSTEM");
	map(0x0c8002, 0x0c8003).portr("P1");
	map(0x0c8004, 0x0c8005).portr("P2");
	map(0x0c8006, 0x0c8007).portr("DSW3");
	map(0x0d0000, 0x0d0001).portr("DSW1");
	map(0x0d0002, 0x0d0003).portr("DSW2");
	map(0x0d8000, 0x0d8001).w(FUNC(gradius3_state::cpuB_irqtrigger_w));
	map(0x0e0000, 0x0e0001).w("watchdog", FUNC(watchdog_timer_device::reset16_w));
	map(0x0e8000, 0x0e8000).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x0f0000, 0x0f0001).w(FUNC(gradius3_state::sound_irq_w));
	map(0x100000, 0x103fff).ram().share("share1");
	map(0x14c000, 0x153fff).rw(FUNC(gradius3_state::k052109_halfword_r), FUNC(gradius3_state::k052109_halfword_w));
	map(0x180000, 0x19ffff).ram().w(FUNC(gradius3_state::gradius3_gfxram_w)).share("k052109");
}


void gradius3_state::gradius3_map2(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x103fff).ram();
	map(0x140000, 0x140001).w(FUNC(gradius3_state::cpuB_irqenable_w));
	map(0x200000, 0x203fff).ram().share("share1");
	map(0x24c000, 0x253fff).rw(FUNC(gradius3_state::k052109_halfword_r), FUNC(gradius3_state::k052109_halfword_w));
	map(0x280000, 0x29ffff).ram().w(FUNC(gradius3_state::gradius3_gfxram_w)).share("k052109");
	map(0x2c0000, 0x2c000f).rw(m_k051960, FUNC(k051960_device::k051937_r), FUNC(k051960_device::k051937_w)).umask16(0x00ff);
	map(0x2c0800, 0x2c0fff).rw(m_k051960, FUNC(k051960_device::k051960_r), FUNC(k051960_device::k051960_w)).umask16(0x00ff);
	map(0x400000, 0x5fffff).r(FUNC(gradius3_state::gradius3_gfxrom_r));     /* gfx ROMs are mapped here, and copied to RAM */
}


void gradius3_state::gradius3_s_map(address_map &map)
{
	map(0x0000, 0xefff).rom();
	map(0xf000, 0xf000).w(FUNC(gradius3_state::sound_bank_w));             /* 007232 bankswitch */
	map(0xf010, 0xf010).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0xf020, 0xf02d).rw(m_k007232, FUNC(k007232_device::read), FUNC(k007232_device::write));
	map(0xf030, 0xf031).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xf800, 0xffff).ram();
}


/***************************************************************************

  Input ports

***************************************************************************/

static INPUT_PORTS_START( gradius3 )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1")
	KONAMI8_MONO_B123_UNK                       // button1 = power-up, button2 = shoot, button3 = missile

	PORT_START("P2")
	KONAMI8_COCKTAIL_B123_UNK

	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "Invalid", SW1)
	/* "Invalid" = both coin slots disabled */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "20k and every 70k" )
	PORT_DIPSETTING(    0x10, "100k and every 100k" )
	PORT_DIPSETTING(    0x08, "50k only" )
	PORT_DIPSETTING(    0x00, "100k only" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Upright Controls" )          PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Dual ) )
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW3:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW3:4" )           /* Manual says it's unused */
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


/***************************************************************************

  Machine configuration

***************************************************************************/

void gradius3_state::volume_callback(uint8_t data)
{
	m_k007232->set_volume(0, (data >> 4) * 0x11, 0);
	m_k007232->set_volume(1, 0, (data & 0x0f) * 0x11);
}

void gradius3_state::machine_start()
{
	save_item(NAME(m_irqAen));
	save_item(NAME(m_irqBmask));
	save_item(NAME(m_priority));
}

void gradius3_state::machine_reset()
{
	/* start with cpu B halted */
	m_subcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_irqAen = 0;
	m_irqBmask = 0;
	m_priority = 0;

}

void gradius3_state::gradius3(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(10'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &gradius3_state::gradius3_map);
	m_maincpu->set_vblank_int("screen", FUNC(gradius3_state::cpuA_interrupt));

	M68000(config, m_subcpu, XTAL(10'000'000));
	m_subcpu->set_addrmap(AS_PROGRAM, &gradius3_state::gradius3_map2);
	TIMER(config, "scantimer").configure_scanline(FUNC(gradius3_state::gradius3_sub_scanline), "screen", 0, 1);
	/* 4 is triggered by cpu A, the others are unknown but required for the game to run. */

	Z80(config, m_audiocpu, 3579545);
	m_audiocpu->set_addrmap(AS_PROGRAM, &gradius3_state::gradius3_s_map);

	config.set_maximum_quantum(attotime::from_hz(6000));

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(12*8, (64-12)*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(gradius3_state::screen_update_gradius3));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::xRGB_555, 2048).enable_shadows();

	K052109(config, m_k052109, 0);
	m_k052109->set_palette("palette");
	m_k052109->set_screen(nullptr);
	m_k052109->set_tile_callback(FUNC(gradius3_state::tile_callback));
	m_k052109->set_char_ram(true);

	K051960(config, m_k051960, 0);
	m_k051960->set_palette("palette");
	m_k051960->set_screen("screen");
	m_k051960->set_sprite_callback(FUNC(gradius3_state::sprite_callback));
	m_k051960->set_plane_order(K051960_PLANEORDER_GRADIUS3);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	GENERIC_LATCH_8(config, "soundlatch");

	YM2151(config, "ymsnd", 3579545).add_route(0, "speaker", 1.0, 0).add_route(0, "speaker", 1.0, 1);

	K007232(config, m_k007232, 3579545);
	m_k007232->port_write().set(FUNC(gradius3_state::volume_callback));
	m_k007232->add_route(0, "speaker", 0.20, 0);
	m_k007232->add_route(0, "speaker", 0.20, 1);
	m_k007232->add_route(1, "speaker", 0.20, 0);
	m_k007232->add_route(1, "speaker", 0.20, 1);
}


/***************************************************************************

  ROM definitions

***************************************************************************/

ROM_START( gradius3 )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "945_r13.f15", 0x00000, 0x20000, CRC(cffd103f) SHA1(6bd15e8c2e6e5223d7de9b0b375f36f3e81f60ba) )
	ROM_LOAD16_BYTE( "945_r12.e15", 0x00001, 0x20000, CRC(0b968ef6) SHA1(ba28d16d94b13aac791b11d3d91df26f78e2e477) )

	ROM_REGION( 0x100000, "sub", 0 )
	ROM_LOAD16_BYTE( "945_m09.r17",  0x000000, 0x20000, CRC(b4a6df25) SHA1(85533cf140d28f6f81c0b49b8061bda0924a613a) )
	ROM_LOAD16_BYTE( "945_m08.n17",  0x000001, 0x20000, CRC(74e981d2) SHA1(e7b47a2da01ff73293d2100c48fdf00b33125af5) )
	ROM_LOAD16_BYTE( "945_l06b.r11", 0x040000, 0x20000, CRC(83772304) SHA1(a90c75a3de670b6ec5e0fc201876d463b4a76766) )
	ROM_LOAD16_BYTE( "945_l06a.n11", 0x040001, 0x20000, CRC(e1fd75b6) SHA1(6160d80a2f1bf550e85d6253cf521a96f5a644cc) )
	ROM_LOAD16_BYTE( "945_l07c.r15", 0x080000, 0x20000, CRC(c1e399b6) SHA1(e95bd478dd3beea0175bf9ee4cededb111c4ace1) )
	ROM_LOAD16_BYTE( "945_l07a.n15", 0x080001, 0x20000, CRC(96222d04) SHA1(b55700f683a556b0e73dbac9c7b4ce485420d21c) )
	ROM_LOAD16_BYTE( "945_l07d.r13", 0x0c0000, 0x20000, CRC(4c16d4bd) SHA1(01dcf169b78a1e495214b10181401d1920b0c924) )
	ROM_LOAD16_BYTE( "945_l07b.n13", 0x0c0001, 0x20000, CRC(5e209d01) SHA1(0efa1bbfdc7e2ba1e0bb96245e2bfe961258b446) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "945_r05.d9", 0x00000, 0x10000, CRC(c8c45365) SHA1(b9a7b736b52bca42c7b8c8ed64c8df73e0116158) ) /* Same as 945 M05, but different label */

	ROM_REGION( 0x200000, "k051960", 0 )   /* graphics (addressable by the main CPU) */
	ROM_LOAD32_WORD( "945_a02.l3",  0x000000, 0x80000, CRC(4dfffd74) SHA1(588210bac27448240ef08961f70b714b69cb3ffd) )
	ROM_LOAD32_WORD( "945_a01.h3",  0x000002, 0x80000, CRC(339d6dd2) SHA1(6a52b826aba92c75fc6a5926184948735dc20812) )
	ROM_LOAD32_BYTE( "945_l04a.k6", 0x100000, 0x20000, CRC(884e21ee) SHA1(ce86dd3a06775e5b1aa09db010dcb674e67828e7) )
	ROM_LOAD32_BYTE( "945_l04c.m6", 0x100001, 0x20000, CRC(45bcd921) SHA1(e51a8a71362a6fb55124aa1dce74519c0a3c6e3f) )
	ROM_LOAD32_BYTE( "945_l03a.e6", 0x100002, 0x20000, CRC(a67ef087) SHA1(fd63474f3bbde5dfc53ed4c1db25d6411a8b54d2) )
	ROM_LOAD32_BYTE( "945_l03c.h6", 0x100003, 0x20000, CRC(a56be17a) SHA1(1d387736144c30fcb5de54235331ab1ff70c356e) )
	ROM_LOAD32_BYTE( "945_l04b.k8", 0x180000, 0x20000, CRC(843bc67d) SHA1(cdf8421083f24ab27867ed5d08d8949da192b2b9) )
	ROM_LOAD32_BYTE( "945_l04d.m8", 0x180001, 0x20000, CRC(0a98d08e) SHA1(1e0ca51a2d45c01fa3f11950ddd387f41ddae691) )
	ROM_LOAD32_BYTE( "945_l03b.e8", 0x180002, 0x20000, CRC(933e68b9) SHA1(f3a39446ca77d17fdbd938bd5f718ae9d5570879) )
	ROM_LOAD32_BYTE( "945_l03d.h8", 0x180003, 0x20000, CRC(f375e87b) SHA1(6427b966795c907c8e516244872fe52217da62c4) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "945l14.j28", 0x0000, 0x0100, CRC(c778c189) SHA1(847eaf379ba075c25911c6f83dd63ff390534f60) )  /* priority encoder (not used) */

	ROM_REGION( 0x80000, "k007232", 0 ) /* 007232 samples */
	ROM_LOAD( "945_a10.b15",  0x00000, 0x40000, CRC(1d083e10) SHA1(b116f133a7647ef7a6c373aff00e9622d9954b61) )
	ROM_LOAD( "945_l11a.c18", 0x40000, 0x20000, CRC(6043f4eb) SHA1(1c2e9ace1cfdde504b7b6158e3c3f54dc5ae33d4) )
	ROM_LOAD( "945_l11b.c20", 0x60000, 0x20000, CRC(89ea3baf) SHA1(8edcbaa7969185cfac48c02559826d1b8b081f3f) )
ROM_END

ROM_START( gradius3j )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "945_313.f15", 0x00000, 0x20000, CRC(706494e1) SHA1(f14fd1f01ee6a9cdcb441432608ba26dedd71b57) )
	ROM_LOAD16_BYTE( "945_312.e15", 0x00001, 0x20000, CRC(6dcd00ab) SHA1(d0cffcb00f89ccaba5fda8a64d1cb6e4af8c2f27) )

	ROM_REGION( 0x100000, "sub", 0 )
	ROM_LOAD16_BYTE( "945_m09.r17",  0x000000, 0x20000, CRC(b4a6df25) SHA1(85533cf140d28f6f81c0b49b8061bda0924a613a) )
	ROM_LOAD16_BYTE( "945_m08.n17",  0x000001, 0x20000, CRC(74e981d2) SHA1(e7b47a2da01ff73293d2100c48fdf00b33125af5) )
	ROM_LOAD16_BYTE( "945_l06b.r11", 0x040000, 0x20000, CRC(83772304) SHA1(a90c75a3de670b6ec5e0fc201876d463b4a76766) )
	ROM_LOAD16_BYTE( "945_l06a.n11", 0x040001, 0x20000, CRC(e1fd75b6) SHA1(6160d80a2f1bf550e85d6253cf521a96f5a644cc) )
	ROM_LOAD16_BYTE( "945_l07c.r15", 0x080000, 0x20000, CRC(c1e399b6) SHA1(e95bd478dd3beea0175bf9ee4cededb111c4ace1) )
	ROM_LOAD16_BYTE( "945_l07a.n15", 0x080001, 0x20000, CRC(96222d04) SHA1(b55700f683a556b0e73dbac9c7b4ce485420d21c) )
	ROM_LOAD16_BYTE( "945_l07d.r13", 0x0c0000, 0x20000, CRC(4c16d4bd) SHA1(01dcf169b78a1e495214b10181401d1920b0c924) )
	ROM_LOAD16_BYTE( "945_l07b.n13", 0x0c0001, 0x20000, CRC(5e209d01) SHA1(0efa1bbfdc7e2ba1e0bb96245e2bfe961258b446) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "945_m05.d9", 0x00000, 0x10000, CRC(c8c45365) SHA1(b9a7b736b52bca42c7b8c8ed64c8df73e0116158) )

	ROM_REGION( 0x200000, "k051960", 0 )   /* graphics (addressable by the main CPU) */
	ROM_LOAD32_WORD( "945_a02.l3",  0x000000, 0x80000, CRC(4dfffd74) SHA1(588210bac27448240ef08961f70b714b69cb3ffd) )
	ROM_LOAD32_WORD( "945_a01.h3",  0x000002, 0x80000, CRC(339d6dd2) SHA1(6a52b826aba92c75fc6a5926184948735dc20812) )
	ROM_LOAD32_BYTE( "945_l04a.k6", 0x100000, 0x20000, CRC(884e21ee) SHA1(ce86dd3a06775e5b1aa09db010dcb674e67828e7) )
	ROM_LOAD32_BYTE( "945_l04c.m6", 0x100001, 0x20000, CRC(45bcd921) SHA1(e51a8a71362a6fb55124aa1dce74519c0a3c6e3f) )
	ROM_LOAD32_BYTE( "945_l03a.e6", 0x100002, 0x20000, CRC(a67ef087) SHA1(fd63474f3bbde5dfc53ed4c1db25d6411a8b54d2) )
	ROM_LOAD32_BYTE( "945_l03c.h6", 0x100003, 0x20000, CRC(a56be17a) SHA1(1d387736144c30fcb5de54235331ab1ff70c356e) )
	ROM_LOAD32_BYTE( "945_l04b.k8", 0x180000, 0x20000, CRC(843bc67d) SHA1(cdf8421083f24ab27867ed5d08d8949da192b2b9) )
	ROM_LOAD32_BYTE( "945_l04d.m8", 0x180001, 0x20000, CRC(0a98d08e) SHA1(1e0ca51a2d45c01fa3f11950ddd387f41ddae691) )
	ROM_LOAD32_BYTE( "945_l03b.e8", 0x180002, 0x20000, CRC(933e68b9) SHA1(f3a39446ca77d17fdbd938bd5f718ae9d5570879) )
	ROM_LOAD32_BYTE( "945_l03d.h8", 0x180003, 0x20000, CRC(f375e87b) SHA1(6427b966795c907c8e516244872fe52217da62c4) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "945l14.j28", 0x0000, 0x0100, CRC(c778c189) SHA1(847eaf379ba075c25911c6f83dd63ff390534f60) )  /* priority encoder (not used) */

	ROM_REGION( 0x80000, "k007232", 0 ) /* 007232 samples */
	ROM_LOAD( "945_a10.b15",  0x00000, 0x40000, CRC(1d083e10) SHA1(b116f133a7647ef7a6c373aff00e9622d9954b61) )
	ROM_LOAD( "945_l11a.c18", 0x40000, 0x20000, CRC(6043f4eb) SHA1(1c2e9ace1cfdde504b7b6158e3c3f54dc5ae33d4) )
	ROM_LOAD( "945_l11b.c20", 0x60000, 0x20000, CRC(89ea3baf) SHA1(8edcbaa7969185cfac48c02559826d1b8b081f3f) )
ROM_END

ROM_START( gradius3ja )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "945_s13.f15", 0x00000, 0x20000, CRC(70c240a2) SHA1(82dc391572e1f61b0182cb031654d71adcdd5f6e) )
	ROM_LOAD16_BYTE( "945_s12.e15", 0x00001, 0x20000, CRC(bbc300d4) SHA1(e1ca98bc591575285d7bd2d4fefdf35fed10dcb6) )

	ROM_REGION( 0x100000, "sub", 0 )
	ROM_LOAD16_BYTE( "945_m09.r17",  0x000000, 0x20000, CRC(b4a6df25) SHA1(85533cf140d28f6f81c0b49b8061bda0924a613a) )
	ROM_LOAD16_BYTE( "945_m08.n17",  0x000001, 0x20000, CRC(74e981d2) SHA1(e7b47a2da01ff73293d2100c48fdf00b33125af5) )
	ROM_LOAD16_BYTE( "945_l06b.r11", 0x040000, 0x20000, CRC(83772304) SHA1(a90c75a3de670b6ec5e0fc201876d463b4a76766) )
	ROM_LOAD16_BYTE( "945_l06a.n11", 0x040001, 0x20000, CRC(e1fd75b6) SHA1(6160d80a2f1bf550e85d6253cf521a96f5a644cc) )
	ROM_LOAD16_BYTE( "945_l07c.r15", 0x080000, 0x20000, CRC(c1e399b6) SHA1(e95bd478dd3beea0175bf9ee4cededb111c4ace1) )
	ROM_LOAD16_BYTE( "945_l07a.n15", 0x080001, 0x20000, CRC(96222d04) SHA1(b55700f683a556b0e73dbac9c7b4ce485420d21c) )
	ROM_LOAD16_BYTE( "945_l07d.r13", 0x0c0000, 0x20000, CRC(4c16d4bd) SHA1(01dcf169b78a1e495214b10181401d1920b0c924) )
	ROM_LOAD16_BYTE( "945_l07b.n13", 0x0c0001, 0x20000, CRC(5e209d01) SHA1(0efa1bbfdc7e2ba1e0bb96245e2bfe961258b446) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "945_m05.d9", 0x00000, 0x10000, CRC(c8c45365) SHA1(b9a7b736b52bca42c7b8c8ed64c8df73e0116158) )

	ROM_REGION( 0x200000, "k051960", 0 )   /* graphics (addressable by the main CPU) */
	ROM_LOAD32_WORD( "945_a02.l3",  0x000000, 0x80000, CRC(4dfffd74) SHA1(588210bac27448240ef08961f70b714b69cb3ffd) )
	ROM_LOAD32_WORD( "945_a01.h3",  0x000002, 0x80000, CRC(339d6dd2) SHA1(6a52b826aba92c75fc6a5926184948735dc20812) )
	ROM_LOAD32_BYTE( "945_l04a.k6", 0x100000, 0x20000, CRC(884e21ee) SHA1(ce86dd3a06775e5b1aa09db010dcb674e67828e7) )
	ROM_LOAD32_BYTE( "945_l04c.m6", 0x100001, 0x20000, CRC(45bcd921) SHA1(e51a8a71362a6fb55124aa1dce74519c0a3c6e3f) )
	ROM_LOAD32_BYTE( "945_l03a.e6", 0x100002, 0x20000, CRC(a67ef087) SHA1(fd63474f3bbde5dfc53ed4c1db25d6411a8b54d2) )
	ROM_LOAD32_BYTE( "945_l03c.h6", 0x100003, 0x20000, CRC(a56be17a) SHA1(1d387736144c30fcb5de54235331ab1ff70c356e) )
	ROM_LOAD32_BYTE( "945_l04b.k8", 0x180000, 0x20000, CRC(843bc67d) SHA1(cdf8421083f24ab27867ed5d08d8949da192b2b9) )
	ROM_LOAD32_BYTE( "945_l04d.m8", 0x180001, 0x20000, CRC(0a98d08e) SHA1(1e0ca51a2d45c01fa3f11950ddd387f41ddae691) )
	ROM_LOAD32_BYTE( "945_l03b.e8", 0x180002, 0x20000, CRC(933e68b9) SHA1(f3a39446ca77d17fdbd938bd5f718ae9d5570879) )
	ROM_LOAD32_BYTE( "945_l03d.h8", 0x180003, 0x20000, CRC(f375e87b) SHA1(6427b966795c907c8e516244872fe52217da62c4) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "945l14.j28", 0x0000, 0x0100, CRC(c778c189) SHA1(847eaf379ba075c25911c6f83dd63ff390534f60) )  /* priority encoder (not used) */

	ROM_REGION( 0x80000, "k007232", 0 ) /* 007232 samples */
	ROM_LOAD( "945_a10.b15",  0x00000, 0x40000, CRC(1d083e10) SHA1(b116f133a7647ef7a6c373aff00e9622d9954b61) )
	ROM_LOAD( "945_l11a.c18", 0x40000, 0x20000, CRC(6043f4eb) SHA1(1c2e9ace1cfdde504b7b6158e3c3f54dc5ae33d4) )
	ROM_LOAD( "945_l11b.c20", 0x60000, 0x20000, CRC(89ea3baf) SHA1(8edcbaa7969185cfac48c02559826d1b8b081f3f) )
ROM_END

// Same as normal gradius3ja set in content but with some ROMs split and populated differently.
ROM_START( gradius3jas )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "945_s13.f15", 0x00000, 0x20000, CRC(70c240a2) SHA1(82dc391572e1f61b0182cb031654d71adcdd5f6e) )
	ROM_LOAD16_BYTE( "945_s12.e15", 0x00001, 0x20000, CRC(bbc300d4) SHA1(e1ca98bc591575285d7bd2d4fefdf35fed10dcb6) )

	ROM_REGION( 0x100000, "sub", 0 )
	ROM_LOAD16_BYTE( "945_m09.r17",  0x000000, 0x20000, CRC(b4a6df25) SHA1(85533cf140d28f6f81c0b49b8061bda0924a613a) )
	ROM_LOAD16_BYTE( "945_m08.n17",  0x000001, 0x20000, CRC(74e981d2) SHA1(e7b47a2da01ff73293d2100c48fdf00b33125af5) )
	ROM_LOAD16_BYTE( "945_l06b.r11", 0x040000, 0x20000, CRC(83772304) SHA1(a90c75a3de670b6ec5e0fc201876d463b4a76766) )
	ROM_LOAD16_BYTE( "945_l06a.n11", 0x040001, 0x20000, CRC(e1fd75b6) SHA1(6160d80a2f1bf550e85d6253cf521a96f5a644cc) )
	ROM_LOAD16_BYTE( "945_l07c.r15", 0x080000, 0x20000, CRC(c1e399b6) SHA1(e95bd478dd3beea0175bf9ee4cededb111c4ace1) )
	ROM_LOAD16_BYTE( "945_l07a.n15", 0x080001, 0x20000, CRC(96222d04) SHA1(b55700f683a556b0e73dbac9c7b4ce485420d21c) )
	ROM_LOAD16_BYTE( "945_l07d.r13", 0x0c0000, 0x20000, CRC(4c16d4bd) SHA1(01dcf169b78a1e495214b10181401d1920b0c924) )
	ROM_LOAD16_BYTE( "945_l07b.n13", 0x0c0001, 0x20000, CRC(5e209d01) SHA1(0efa1bbfdc7e2ba1e0bb96245e2bfe961258b446) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "945_m05.d9", 0x00000, 0x10000, CRC(c8c45365) SHA1(b9a7b736b52bca42c7b8c8ed64c8df73e0116158) )

	ROM_REGION( 0x200000, "k051960", 0 )   /* graphics (addressable by the main CPU) */
	ROM_LOAD32_BYTE( "945_a02a.k2", 0x000000, 0x20000, CRC(fbb81511) SHA1(e7988d52e323e46117f5080c469daf9b119f28a0) )
	ROM_LOAD32_BYTE( "945_a02c.m2", 0x000001, 0x20000, CRC(031b55e8) SHA1(64ed8dee60bf012df7c1ed496af1c75263c052a6) )
	ROM_LOAD32_BYTE( "945_a01a.e2", 0x000002, 0x20000, CRC(bace5abb) SHA1(b32df63294c0730f463335b1b760494389c60062) )
	ROM_LOAD32_BYTE( "945_a01c.h2", 0x000003, 0x20000, CRC(d91b29a6) SHA1(0c3027a08996f4c2b86dd88695241b21c8dffd64) )
	ROM_LOAD32_BYTE( "945_a02b.k4", 0x080000, 0x20000, CRC(c0fed4ab) SHA1(f01975b13759cae7c8dfd24f9b3f4ac960d32957) )
	ROM_LOAD32_BYTE( "945_a02d.m4", 0x080001, 0x20000, CRC(d462817c) SHA1(00137e38454e7c3548a1a9553c5ee644916b3959) )
	ROM_LOAD32_BYTE( "945_a01b.e4", 0x080002, 0x20000, CRC(b426090e) SHA1(06a671a648e3255146fe0c325d5451d4f75f08aa) )
	ROM_LOAD32_BYTE( "945_a01d.h4", 0x080003, 0x20000, CRC(3990c09a) SHA1(1f6a089c1d03fb95d4d96fecc0379bde26ee2b9d) )

	ROM_LOAD32_BYTE( "945_l04a.k6", 0x100000, 0x20000, CRC(884e21ee) SHA1(ce86dd3a06775e5b1aa09db010dcb674e67828e7) )
	ROM_LOAD32_BYTE( "945_l04c.m6", 0x100001, 0x20000, CRC(45bcd921) SHA1(e51a8a71362a6fb55124aa1dce74519c0a3c6e3f) )
	ROM_LOAD32_BYTE( "945_l03a.e6", 0x100002, 0x20000, CRC(a67ef087) SHA1(fd63474f3bbde5dfc53ed4c1db25d6411a8b54d2) )
	ROM_LOAD32_BYTE( "945_l03c.h6", 0x100003, 0x20000, CRC(a56be17a) SHA1(1d387736144c30fcb5de54235331ab1ff70c356e) )
	ROM_LOAD32_BYTE( "945_l04b.k8", 0x180000, 0x20000, CRC(843bc67d) SHA1(cdf8421083f24ab27867ed5d08d8949da192b2b9) )
	ROM_LOAD32_BYTE( "945_l04d.m8", 0x180001, 0x20000, CRC(0a98d08e) SHA1(1e0ca51a2d45c01fa3f11950ddd387f41ddae691) )
	ROM_LOAD32_BYTE( "945_l03b.e8", 0x180002, 0x20000, CRC(933e68b9) SHA1(f3a39446ca77d17fdbd938bd5f718ae9d5570879) )
	ROM_LOAD32_BYTE( "945_l03d.h8", 0x180003, 0x20000, CRC(f375e87b) SHA1(6427b966795c907c8e516244872fe52217da62c4) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "945l14.j28", 0x0000, 0x0100, CRC(c778c189) SHA1(847eaf379ba075c25911c6f83dd63ff390534f60) )  /* priority encoder (not used) */

	ROM_REGION( 0x80000, "k007232", 0 ) /* 007232 samples */
	ROM_LOAD16_BYTE( "945_a10a.c14", 0x00000, 0x20000, CRC(ec717414) SHA1(8c63d5fe01d0833529fca91bc80cdbd8a04174c0) )
	ROM_LOAD16_BYTE( "945_a10b.c16", 0x00001, 0x20000, CRC(709e30e4) SHA1(27fcea720cd2498f1870c9290d30dcb3dd81d5e5) )
	ROM_LOAD( "945_l11a.c18", 0x40000, 0x20000, CRC(6043f4eb) SHA1(1c2e9ace1cfdde504b7b6158e3c3f54dc5ae33d4) )
	ROM_LOAD( "945_l11b.c20", 0x60000, 0x20000, CRC(89ea3baf) SHA1(8edcbaa7969185cfac48c02559826d1b8b081f3f) )
ROM_END

ROM_START( gradius3a )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "945_13.f15", 0x00000, 0x20000, CRC(9974fe6b) SHA1(c18ad8d7c93bf58d886715d8e210177cf49f220b) )
	ROM_LOAD16_BYTE( "945_12.e15", 0x00001, 0x20000, CRC(e9771b91) SHA1(c9f4610b897c13742b44b546e2bed8ee21945f61) )

	ROM_REGION( 0x100000, "sub", 0 )
	ROM_LOAD16_BYTE( "945_m09.r17",  0x000000, 0x20000, CRC(b4a6df25) SHA1(85533cf140d28f6f81c0b49b8061bda0924a613a) )
	ROM_LOAD16_BYTE( "945_m08.n17",  0x000001, 0x20000, CRC(74e981d2) SHA1(e7b47a2da01ff73293d2100c48fdf00b33125af5) )
	ROM_LOAD16_BYTE( "945_l06b.r11", 0x040000, 0x20000, CRC(83772304) SHA1(a90c75a3de670b6ec5e0fc201876d463b4a76766) )
	ROM_LOAD16_BYTE( "945_l06a.n11", 0x040001, 0x20000, CRC(e1fd75b6) SHA1(6160d80a2f1bf550e85d6253cf521a96f5a644cc) )
	ROM_LOAD16_BYTE( "945_l07c.r15", 0x080000, 0x20000, CRC(c1e399b6) SHA1(e95bd478dd3beea0175bf9ee4cededb111c4ace1) )
	ROM_LOAD16_BYTE( "945_l07a.n15", 0x080001, 0x20000, CRC(96222d04) SHA1(b55700f683a556b0e73dbac9c7b4ce485420d21c) )
	ROM_LOAD16_BYTE( "945_l07d.r13", 0x0c0000, 0x20000, CRC(4c16d4bd) SHA1(01dcf169b78a1e495214b10181401d1920b0c924) )
	ROM_LOAD16_BYTE( "945_l07b.n13", 0x0c0001, 0x20000, CRC(5e209d01) SHA1(0efa1bbfdc7e2ba1e0bb96245e2bfe961258b446) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "945_m05.d9", 0x00000, 0x10000, CRC(c8c45365) SHA1(b9a7b736b52bca42c7b8c8ed64c8df73e0116158) )

	ROM_REGION( 0x200000, "k051960", 0 )   /* graphics (addressable by the main CPU) */
	ROM_LOAD32_WORD( "945_a02.l3",  0x000000, 0x80000, CRC(4dfffd74) SHA1(588210bac27448240ef08961f70b714b69cb3ffd) )
	ROM_LOAD32_WORD( "945_a01.h3",  0x000002, 0x80000, CRC(339d6dd2) SHA1(6a52b826aba92c75fc6a5926184948735dc20812) )
	ROM_LOAD32_BYTE( "945_l04a.k6", 0x100000, 0x20000, CRC(884e21ee) SHA1(ce86dd3a06775e5b1aa09db010dcb674e67828e7) )
	ROM_LOAD32_BYTE( "945_l04c.m6", 0x100001, 0x20000, CRC(45bcd921) SHA1(e51a8a71362a6fb55124aa1dce74519c0a3c6e3f) )
	ROM_LOAD32_BYTE( "945_l03a.e6", 0x100002, 0x20000, CRC(a67ef087) SHA1(fd63474f3bbde5dfc53ed4c1db25d6411a8b54d2) )
	ROM_LOAD32_BYTE( "945_l03c.h6", 0x100003, 0x20000, CRC(a56be17a) SHA1(1d387736144c30fcb5de54235331ab1ff70c356e) )
	ROM_LOAD32_BYTE( "945_l04b.k8", 0x180000, 0x20000, CRC(843bc67d) SHA1(cdf8421083f24ab27867ed5d08d8949da192b2b9) )
	ROM_LOAD32_BYTE( "945_l04d.m8", 0x180001, 0x20000, CRC(0a98d08e) SHA1(1e0ca51a2d45c01fa3f11950ddd387f41ddae691) )
	ROM_LOAD32_BYTE( "945_l03b.e8", 0x180002, 0x20000, CRC(933e68b9) SHA1(f3a39446ca77d17fdbd938bd5f718ae9d5570879) )
	ROM_LOAD32_BYTE( "945_l03d.h8", 0x180003, 0x20000, CRC(f375e87b) SHA1(6427b966795c907c8e516244872fe52217da62c4) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "945l14.j28", 0x0000, 0x0100, CRC(c778c189) SHA1(847eaf379ba075c25911c6f83dd63ff390534f60) )  /* priority encoder (not used) */

	ROM_REGION( 0x80000, "k007232", 0 ) /* 007232 samples */
	ROM_LOAD( "945_a10.b15",  0x00000, 0x40000, CRC(1d083e10) SHA1(b116f133a7647ef7a6c373aff00e9622d9954b61) )
	ROM_LOAD( "945_l11a.c18", 0x40000, 0x20000, CRC(6043f4eb) SHA1(1c2e9ace1cfdde504b7b6158e3c3f54dc5ae33d4) )
	ROM_LOAD( "945_l11b.c20", 0x60000, 0x20000, CRC(89ea3baf) SHA1(8edcbaa7969185cfac48c02559826d1b8b081f3f) )
ROM_END

} // anonymous namespace


GAME( 1989, gradius3,    0,        gradius3, gradius3, gradius3_state, empty_init, ROT0, "Konami", "Gradius III (World, version R)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, gradius3j,   gradius3, gradius3, gradius3, gradius3_state, empty_init, ROT0, "Konami", "Gradius III: Densetsu kara Shinwa e (Japan, version 3, newer)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, gradius3ja,  gradius3, gradius3, gradius3, gradius3_state, empty_init, ROT0, "Konami", "Gradius III: Densetsu kara Shinwa e (Japan, version S)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, gradius3jas, gradius3, gradius3, gradius3, gradius3_state, empty_init, ROT0, "Konami", "Gradius III: Densetsu kara Shinwa e (Japan, version S, split)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, gradius3a,   gradius3, gradius3, gradius3, gradius3_state, empty_init, ROT0, "Konami", "Gradius III (Asia)", MACHINE_SUPPORTS_SAVE )
