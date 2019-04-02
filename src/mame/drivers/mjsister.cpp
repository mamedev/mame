// license:BSD-3-Clause
// copyright-holders:Uki
/*****************************************************************************

    Mahjong Sisters (c) 1986 Toa Plan

    Driver by Uki

*****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


#define MCLK 12000000


class mjsister_state : public driver_device
{
public:
	mjsister_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mainlatch(*this, "mainlatch%u", 1),
		m_palette(*this, "palette"),
		m_dac(*this, "dac"),
		m_rombank(*this, "bank1")
	{ }

	void mjsister(machine_config &config);

private:
	enum
	{
		TIMER_DAC
	};

	/* video-related */
	std::unique_ptr<bitmap_ind16> m_tmpbitmap0;
	std::unique_ptr<bitmap_ind16> m_tmpbitmap1;
	bool m_flip_screen;
	bool m_video_enable;
	int  m_screen_redraw;
	int  m_vrambank;
	int  m_colorbank;

	/* misc */
	int  m_input_sel1;
	int  m_input_sel2;
	bool m_irq_enable;

	uint32_t m_dac_adr;
	uint32_t m_dac_bank;
	uint32_t m_dac_adr_s;
	uint32_t m_dac_adr_e;
	uint32_t m_dac_busy;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device_array<ls259_device, 2> m_mainlatch;
	required_device<palette_device> m_palette;
	required_device<dac_byte_interface> m_dac;

	/* memory */
	required_memory_bank m_rombank;
	uint8_t m_videoram0[0x8000];
	uint8_t m_videoram1[0x8000];
	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_WRITE8_MEMBER(dac_adr_s_w);
	DECLARE_WRITE8_MEMBER(dac_adr_e_w);
	DECLARE_WRITE_LINE_MEMBER(rombank_w);
	DECLARE_WRITE_LINE_MEMBER(flip_screen_w);
	DECLARE_WRITE_LINE_MEMBER(colorbank_w);
	DECLARE_WRITE_LINE_MEMBER(video_enable_w);
	DECLARE_WRITE_LINE_MEMBER(irq_enable_w);
	DECLARE_WRITE_LINE_MEMBER(vrambank_w);
	DECLARE_WRITE_LINE_MEMBER(dac_bank_w);
	DECLARE_WRITE_LINE_MEMBER(coin_counter_w);
	DECLARE_WRITE8_MEMBER(input_sel1_w);
	DECLARE_WRITE8_MEMBER(input_sel2_w);
	DECLARE_READ8_MEMBER(keys_r);
	TIMER_CALLBACK_MEMBER(dac_callback);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	INTERRUPT_GEN_MEMBER(interrupt);
	virtual void video_start() override;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void redraw();
	void plot0( int offset, uint8_t data );
	void plot1( int offset, uint8_t data );

	void mjsister_io_map(address_map &map);
	void mjsister_map(address_map &map);

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	emu_timer *m_dac_timer;
};


/*************************************
 *
 *  Video emulation
 *
 *************************************/

void mjsister_state::video_start()
{
	m_tmpbitmap0 = std::make_unique<bitmap_ind16>(256, 256);
	m_tmpbitmap1 = std::make_unique<bitmap_ind16>(256, 256);

	save_item(NAME(m_videoram0));
	save_item(NAME(m_videoram1));
}

void mjsister_state::plot0( int offset, uint8_t data )
{
	int x, y, c1, c2;

	x = offset & 0x7f;
	y = offset / 0x80;

	c1 = (data & 0x0f)        + m_colorbank * 0x20;
	c2 = ((data & 0xf0) >> 4) + m_colorbank * 0x20;

	m_tmpbitmap0->pix16(y, x * 2 + 0) = c1;
	m_tmpbitmap0->pix16(y, x * 2 + 1) = c2;
}

void mjsister_state::plot1( int offset, uint8_t data )
{
	int x, y, c1, c2;

	x = offset & 0x7f;
	y = offset / 0x80;

	c1 = data & 0x0f;
	c2 = (data & 0xf0) >> 4;

	if (c1)
		c1 += m_colorbank * 0x20 + 0x10;
	if (c2)
		c2 += m_colorbank * 0x20 + 0x10;

	m_tmpbitmap1->pix16(y, x * 2 + 0) = c1;
	m_tmpbitmap1->pix16(y, x * 2 + 1) = c2;
}

WRITE8_MEMBER(mjsister_state::videoram_w)
{
	if (m_vrambank)
	{
		m_videoram1[offset] = data;
		plot1(offset, data);
	}
	else
	{
		m_videoram0[offset] = data;
		plot0(offset, data);
	}
}

uint32_t mjsister_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int flip = m_flip_screen;
	int i, j;

	if (m_screen_redraw)
	{
		int offs;

		for (offs = 0; offs < 0x8000; offs++)
		{
			plot0(offs, m_videoram0[offs]);
			plot1(offs, m_videoram1[offs]);
		}
		m_screen_redraw = 0;
	}

	if (m_video_enable)
	{
		for (i = 0; i < 256; i++)
			for (j = 0; j < 4; j++)
				bitmap.pix16(i, 256 + j) = m_colorbank * 0x20;

		copybitmap(bitmap, *m_tmpbitmap0, flip, flip, 0, 0, cliprect);
		copybitmap_trans(bitmap, *m_tmpbitmap1, flip, flip, 2, 0, cliprect, 0);
	}
	else
		bitmap.fill(m_palette->black_pen(), cliprect);
	return 0;
}

/*************************************
 *
 *  Memory handlers
 *
 *************************************/

void mjsister_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
	case TIMER_DAC:
		dac_callback(ptr, param);
		break;
	default:
		assert_always(false, "Unknown id in mjsister_state::device_timer");
	}
}

TIMER_CALLBACK_MEMBER(mjsister_state::dac_callback)
{
	uint8_t *DACROM = memregion("samples")->base();

	m_dac->write(DACROM[(m_dac_bank * 0x10000 + m_dac_adr++) & 0x1ffff]);

	if (((m_dac_adr & 0xff00 ) >> 8) !=  m_dac_adr_e)
		m_dac_timer->adjust(attotime::from_hz(MCLK) * 1024);
	else
		m_dac_busy = 0;
}

WRITE8_MEMBER(mjsister_state::dac_adr_s_w)
{
	m_dac_adr_s = data;
}

WRITE8_MEMBER(mjsister_state::dac_adr_e_w)
{
	m_dac_adr_e = data;
	m_dac_adr = m_dac_adr_s << 8;

	if (m_dac_busy == 0)
		synchronize(TIMER_DAC);

	m_dac_busy = 1;
}

WRITE_LINE_MEMBER(mjsister_state::rombank_w)
{
	m_rombank->set_entry((m_mainlatch[0]->q0_r() << 1) | m_mainlatch[1]->q6_r());
}

WRITE_LINE_MEMBER(mjsister_state::flip_screen_w)
{
	m_flip_screen = state;
}

WRITE_LINE_MEMBER(mjsister_state::colorbank_w)
{
	m_colorbank = (m_mainlatch[0]->output_state() >> 2) & 7;
	redraw();
}

WRITE_LINE_MEMBER(mjsister_state::video_enable_w)
{
	m_video_enable = state;
}

WRITE_LINE_MEMBER(mjsister_state::irq_enable_w)
{
	m_irq_enable = state;
	if (!m_irq_enable)
		m_maincpu->set_input_line(0, CLEAR_LINE);
}

WRITE_LINE_MEMBER(mjsister_state::vrambank_w)
{
	m_vrambank = state;
}

WRITE_LINE_MEMBER(mjsister_state::dac_bank_w)
{
	m_dac_bank = state;
}

WRITE_LINE_MEMBER(mjsister_state::coin_counter_w)
{
	machine().bookkeeping().coin_counter_w(0, state);
}

WRITE8_MEMBER(mjsister_state::input_sel1_w)
{
	m_input_sel1 = data;
}

WRITE8_MEMBER(mjsister_state::input_sel2_w)
{
	m_input_sel2 = data;
}

READ8_MEMBER(mjsister_state::keys_r)
{
	int p, i, ret = 0;
	static const char *const keynames[] = { "KEY0", "KEY1", "KEY2", "KEY3", "KEY4", "KEY5" };

	p = m_input_sel1 & 0x3f;
	//  p |= ((m_input_sel2 & 8) << 4) | ((m_input_sel2 & 0x20) << 1);

	for (i = 0; i < 6; i++)
	{
		if (BIT(p, i))
			ret |= ioport(keynames[i])->read();
	}

	return ret;
}

/*************************************
 *
 *  Address maps
 *
 *************************************/

void mjsister_state::mjsister_map(address_map &map)
{
	map(0x0000, 0x77ff).rom();
	map(0x7800, 0x7fff).ram();
	map(0x8000, 0xffff).bankr("bank1").w(FUNC(mjsister_state::videoram_w));
}

void mjsister_state::mjsister_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).nopw(); /* HD46505? */
	map(0x10, 0x10).w("aysnd", FUNC(ay8910_device::address_w));
	map(0x11, 0x11).r("aysnd", FUNC(ay8910_device::data_r));
	map(0x12, 0x12).w("aysnd", FUNC(ay8910_device::data_w));
	map(0x20, 0x20).r(FUNC(mjsister_state::keys_r));
	map(0x21, 0x21).portr("IN0");
	map(0x30, 0x30).w("mainlatch1", FUNC(ls259_device::write_nibble_d0));
	map(0x31, 0x31).w("mainlatch2", FUNC(ls259_device::write_nibble_d0));
	map(0x32, 0x32).w(FUNC(mjsister_state::input_sel1_w));
	map(0x33, 0x33).w(FUNC(mjsister_state::input_sel2_w));
	map(0x34, 0x34).w(FUNC(mjsister_state::dac_adr_s_w));
	map(0x35, 0x35).w(FUNC(mjsister_state::dac_adr_e_w));
}


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( mjsister )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )          PORT_DIPLOCATION("DSW1:8,7,6")
	PORT_DIPSETTING(    0x03, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "DSW1:5")
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("DSW1:4,3") // see code at $141C
	PORT_DIPSETTING(    0x30, "0" )
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Test ) )             PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")      /* not on PCB */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE3 ) PORT_OPTIONAL PORT_NAME("Memory Reset 1") // only tested in service mode?
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_GAMBLE_BOOK ) PORT_OPTIONAL PORT_NAME("Analyzer") // only tested in service mode?
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_SERVICE4 ) PORT_OPTIONAL PORT_NAME("Memory Reset 2") // only tested in service mode?
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT ) PORT_OPTIONAL // only tested in service mode?
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Hopper") PORT_CODE(KEYCODE_8) // only tested in service mode?

	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_MAHJONG_B )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_MAHJONG_C )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_MAHJONG_D )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_MAHJONG_E )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_MAHJONG_F )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_MAHJONG_G )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_MAHJONG_H )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_MAHJONG_SCORE )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_MAHJONG_I )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_MAHJONG_J )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_MAHJONG_L )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_MAHJONG_M )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_MAHJONG_N )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_MAHJONG_CHI )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_MAHJONG_PON )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_MAHJONG_KAN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_MAHJONG_REACH )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_MAHJONG_RON )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_MAHJONG_BIG )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_MAHJONG_BET )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_MAHJONG_SMALL )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

INPUT_PORTS_END

/*************************************
 *
 *  Machine driver
 *
 *************************************/

void mjsister_state::redraw()
{
	/* we can skip saving tmpbitmaps because we can redraw them from vram */
	m_screen_redraw = 1;
}

void mjsister_state::machine_start()
{
	uint8_t *ROM = memregion("maincpu")->base();

	m_rombank->configure_entries(0, 4, &ROM[0x10000], 0x8000);

	m_dac_timer = timer_alloc(TIMER_DAC);

	save_item(NAME(m_dac_busy));
	save_item(NAME(m_flip_screen));
	save_item(NAME(m_video_enable));
	save_item(NAME(m_vrambank));
	save_item(NAME(m_colorbank));
	save_item(NAME(m_input_sel1));
	save_item(NAME(m_input_sel2));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_dac_adr));
	save_item(NAME(m_dac_bank));
	save_item(NAME(m_dac_adr_s));
	save_item(NAME(m_dac_adr_e));
	machine().save().register_postload(save_prepost_delegate(FUNC(mjsister_state::redraw), this));
}

void mjsister_state::machine_reset()
{
	m_dac_busy = 0;
	m_flip_screen = 0;
	m_video_enable = 0;
	m_screen_redraw = 0;
	m_input_sel1 = 0;
	m_input_sel2 = 0;
	m_dac_adr = 0;
	m_dac_bank = 0;
	m_dac_adr_s = 0;
	m_dac_adr_e = 0;
}

INTERRUPT_GEN_MEMBER(mjsister_state::interrupt)
{
	if (m_irq_enable)
		m_maincpu->set_input_line(0, ASSERT_LINE);
}

void mjsister_state::mjsister(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, MCLK/2); /* 6.000 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &mjsister_state::mjsister_map);
	m_maincpu->set_addrmap(AS_IO, &mjsister_state::mjsister_io_map);
	m_maincpu->set_periodic_int(FUNC(mjsister_state::interrupt), attotime::from_hz(2*60));

	LS259(config, m_mainlatch[0]);
	m_mainlatch[0]->q_out_cb<0>().set(FUNC(mjsister_state::rombank_w));
	m_mainlatch[0]->q_out_cb<1>().set(FUNC(mjsister_state::flip_screen_w));
	m_mainlatch[0]->q_out_cb<2>().set(FUNC(mjsister_state::colorbank_w));
	m_mainlatch[0]->q_out_cb<3>().set(FUNC(mjsister_state::colorbank_w));
	m_mainlatch[0]->q_out_cb<4>().set(FUNC(mjsister_state::colorbank_w));
	m_mainlatch[0]->q_out_cb<5>().set(FUNC(mjsister_state::video_enable_w));
	m_mainlatch[0]->q_out_cb<6>().set(FUNC(mjsister_state::irq_enable_w));
	m_mainlatch[0]->q_out_cb<7>().set(FUNC(mjsister_state::vrambank_w));

	LS259(config, m_mainlatch[1]);
	m_mainlatch[1]->q_out_cb<2>().set(FUNC(mjsister_state::coin_counter_w));
	m_mainlatch[1]->q_out_cb<5>().set(FUNC(mjsister_state::dac_bank_w));
	m_mainlatch[1]->q_out_cb<6>().set(FUNC(mjsister_state::rombank_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	screen.set_size(256+4, 256);
	screen.set_visarea(0, 255+4, 8, 247);
	screen.set_screen_update(FUNC(mjsister_state::screen_update));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette, palette_device::RGB_444_PROMS, "proms", 256);


	/* sound hardware */
	SPEAKER(config, "speaker").front_center();

	ay8910_device &aysnd(AY8910(config, "aysnd", MCLK/8));
	aysnd.port_a_read_callback().set_ioport("DSW1");
	aysnd.port_b_read_callback().set_ioport("DSW2");
	aysnd.add_route(ALL_OUTPUTS, "speaker", 0.15);

	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.5); // unknown DAC
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
	vref.add_route(0, "dac", -1.0, DAC_VREF_NEG_INPUT);
}

/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( mjsister )
	ROM_REGION( 0x30000, "maincpu", 0 )   /* CPU */
	ROM_LOAD( "ms00.bin",  0x00000, 0x08000, CRC(9468c33b) SHA1(63aecdcaa8493d58549dfd1d217743210cf953bc) )
	ROM_LOAD( "ms01t.bin", 0x10000, 0x10000, CRC(a7b6e530) SHA1(fda9bea214968a8814d2c43226b3b32316581050) ) /* banked */
	ROM_LOAD( "ms02t.bin", 0x20000, 0x10000, CRC(7752b5ba) SHA1(84dcf27a62eb290ba07c85af155897ec72f320a8) ) /* banked */

	ROM_REGION( 0x20000, "samples", 0 ) /* samples */
	ROM_LOAD( "ms03.bin", 0x00000,  0x10000, CRC(10a68e5e) SHA1(a0e2fa34c1c4f34642f65fbf17e9da9c2554a0c6) )
	ROM_LOAD( "ms04.bin", 0x10000,  0x10000, CRC(641b09c1) SHA1(15cde906175bcb5190d36cc91cbef003ef91e425) )

	ROM_REGION( 0x00400, "proms", 0 ) /* color PROMs */
	ROM_LOAD( "ms05.bpr", 0x0000,  0x0100, CRC(dd231a5f) SHA1(be008593ac8ba8f5a1dd5b188dc7dc4c03016805) ) // R
	ROM_LOAD( "ms06.bpr", 0x0100,  0x0100, CRC(df8e8852) SHA1(842a891440aef55a560d24c96f249618b9f4b97f) ) // G
	ROM_LOAD( "ms07.bpr", 0x0200,  0x0100, CRC(6cb3a735) SHA1(468ae3d40552dc2ec24f5f2988850093d73948a6) ) // B
	ROM_LOAD( "ms08.bpr", 0x0300,  0x0100, CRC(da2b3b38) SHA1(4de99c17b227653bc1b904f1309f447f5a0ab516) ) // ?
ROM_END


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1986, mjsister, 0, mjsister, mjsister, mjsister_state, empty_init, ROT0, "Toaplan", "Mahjong Sisters (Japan)", MACHINE_SUPPORTS_SAVE )
