// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

BMC games using a 68k + VDB40817/SYA70521

driver by Luca Elia

Similar to bmcbowl, koftball, popobear

CPU:    68000
Video:  BMC VDB40817 + BMC SYA70521
Sound:  M6295 + UM3567
Other:  BMC B816140 (CPLD)

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "video/ramdac.h"
#include "sound/ym2413.h"
#include "sound/okim6295.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "machine/timer.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

class bmcpokr_state : public driver_device
{
public:
	bmcpokr_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_hopper(*this,"hopper"),
		m_videoram(*this, "videoram_%u", 1U),
		m_scrollram(*this, "scrollram_%u", 1U),
		m_pixram(*this, "pixram"),
		m_priority(*this, "priority"),
		m_layerctrl(*this, "layerctrl"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	DECLARE_READ_LINE_MEMBER(hopper_r);

	void bmcpokr(machine_config &config);
	void mjmaglmp(machine_config &config);

protected:
	virtual void device_post_load() override;

private:
	virtual void machine_start() override;

	// Devices
	required_device<m68000_device> m_maincpu;
	required_device<ticket_dispenser_device> m_hopper;
	required_shared_ptr_array<uint16_t, 2> m_videoram;
	required_shared_ptr_array<uint16_t, 3> m_scrollram;
	required_shared_ptr<uint16_t> m_pixram;
	required_shared_ptr<uint16_t> m_priority;
	required_shared_ptr<uint16_t> m_layerctrl;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	// Protection
	uint16_t m_prot_val;
	DECLARE_READ16_MEMBER(prot_r);
	DECLARE_WRITE16_MEMBER(prot_w);
	DECLARE_READ16_MEMBER(unk_r);

	// I/O
	uint8_t m_mux;
	DECLARE_WRITE8_MEMBER(mux_w);
	DECLARE_READ16_MEMBER(dsw_r);
	DECLARE_READ16_MEMBER(mjmaglmp_dsw_r);
	DECLARE_READ16_MEMBER(mjmaglmp_key_r);

	// Interrrupts
	uint8_t m_irq_enable;
	DECLARE_WRITE8_MEMBER(irq_enable_w);
	DECLARE_WRITE8_MEMBER(irq_ack_w);
	TIMER_DEVICE_CALLBACK_MEMBER(interrupt);

	// Video
	tilemap_t *m_tilemap[2];
	template<unsigned N> TILE_GET_INFO_MEMBER(get_tile_info);
	template<unsigned N> DECLARE_WRITE16_MEMBER(videoram_w);

	std::unique_ptr<bitmap_ind16> m_pixbitmap;
	void pixbitmap_redraw();
	uint8_t m_pixpal;
	DECLARE_WRITE16_MEMBER(pixram_w);
	DECLARE_WRITE8_MEMBER(pixpal_w);

	virtual void video_start() override;
	void draw_layer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void bmcpokr_mem(address_map &map);
	void mjmaglmp_map(address_map &map);
	void ramdac_map(address_map &map);
};

/***************************************************************************
                                Video Hardware
***************************************************************************/

// Tilemaps
template<unsigned N>
WRITE16_MEMBER(bmcpokr_state::videoram_w)
{
	COMBINE_DATA(&m_videoram[N][offset]);
	m_tilemap[N]->mark_tile_dirty(offset);
}

template<unsigned N>
TILE_GET_INFO_MEMBER(bmcpokr_state::get_tile_info)
{
	uint16_t data = m_videoram[N][tile_index];
	SET_TILE_INFO_MEMBER(0, data, 0, (data & 0x8000) ? TILE_FLIPX : 0);
}

void bmcpokr_state::video_start()
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(bmcpokr_state::get_tile_info<0>),this),TILEMAP_SCAN_ROWS,8,8,128,128);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(bmcpokr_state::get_tile_info<1>),this),TILEMAP_SCAN_ROWS,8,8,128,128);

	m_tilemap[0]->set_transparent_pen(0);
	m_tilemap[1]->set_transparent_pen(0);

	m_tilemap[0]->set_scroll_rows(1);
	m_tilemap[1]->set_scroll_rows(1);

	m_tilemap[0]->set_scroll_cols(1);
	m_tilemap[1]->set_scroll_cols(1);

	m_pixbitmap  = std::make_unique<bitmap_ind16>(0x400, 0x200);
}

// 1024 x 512 bitmap. 4 bits per pixel (every byte encodes 2 pixels) + palette register

WRITE16_MEMBER(bmcpokr_state::pixram_w)
{
	COMBINE_DATA(&m_pixram[offset]);

	int x = (offset & 0xff) << 2;
	int y = (offset >> 8);

	uint16_t pixpal = (m_pixpal & 0xf) << 4;

	uint16_t pen;
	if (ACCESSING_BITS_8_15)
	{
		pen = (data >> 12) & 0xf; m_pixbitmap->pix16(y, x + 0) = pen ? pixpal + pen : 0;
		pen = (data >>  8) & 0xf; m_pixbitmap->pix16(y, x + 1) = pen ? pixpal + pen : 0;
	}
	if (ACCESSING_BITS_0_7)
	{
		pen = (data >>  4) & 0xf; m_pixbitmap->pix16(y, x + 2) = pen ? pixpal + pen : 0;
		pen = (data >>  0) & 0xf; m_pixbitmap->pix16(y, x + 3) = pen ? pixpal + pen : 0;
	}
}

void bmcpokr_state::pixbitmap_redraw()
{
	uint16_t pixpal = (m_pixpal & 0xf) << 4;
	int offset = 0;
	for (int y = 0; y < 512; y++)
	{
		for (int x = 0; x < 1024; x += 4)
		{
			uint16_t data = m_pixram[offset++];
			uint16_t pen;
			pen = (data >> 12) & 0xf; m_pixbitmap->pix16(y, x + 0) = pen ? pixpal + pen : 0;
			pen = (data >>  8) & 0xf; m_pixbitmap->pix16(y, x + 1) = pen ? pixpal + pen : 0;
			pen = (data >>  4) & 0xf; m_pixbitmap->pix16(y, x + 2) = pen ? pixpal + pen : 0;
			pen = (data >>  0) & 0xf; m_pixbitmap->pix16(y, x + 3) = pen ? pixpal + pen : 0;
		}
	}
}

WRITE8_MEMBER(bmcpokr_state::pixpal_w)
{
	uint8_t old = m_pixpal;
	if (old != COMBINE_DATA(&m_pixpal))
		pixbitmap_redraw();
}

void bmcpokr_state::device_post_load()
{
	pixbitmap_redraw();
}

// Screen update

void bmcpokr_state::draw_layer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer)
{
	tilemap_t *tmap;
	uint16_t *scroll;
	uint16_t ctrl;

	switch (layer)
	{
		case 1:     tmap = m_tilemap[0]; scroll = m_scrollram[0]; ctrl = (m_layerctrl[0] >> 8) & 0xff; break;
		case 2:     tmap = m_tilemap[1]; scroll = m_scrollram[1]; ctrl = (m_layerctrl[0] >> 0) & 0xff; break;
		default:    tmap = nullptr;      scroll = m_scrollram[2]; ctrl = (m_layerctrl[1] >> 8) & 0xff; break;
	}

	if (ctrl == 0x00)
		return;

	bool linescroll = (ctrl == 0x1f);

	rectangle clip = cliprect;
	for (int y = 0; y < 0x100; y++)
	{
		if (linescroll)
		{
			if ( (y < cliprect.top()) || (y > cliprect.bottom()) )
				continue;

			clip.sety(y, y);
		}

		int sx = (scroll[y] & 0xff) * 4;
		int sy = ((scroll[y] >> 8) & 0xff) - y;

		if (tmap)
		{
			tmap->set_scrollx(0, sx);
			tmap->set_scrolly(0, sy);
			tmap->draw(screen, bitmap, clip, 0, 0);
		}
		else
		{
			sx = -sx;
			sy = -sy;
			copyscrollbitmap_trans(bitmap, *m_pixbitmap, 1, &sx, 1, &sy, cliprect, 0);
		}

		if (!linescroll)
			return;
	}
}

uint32_t bmcpokr_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int layers_ctrl = -1;

#ifdef MAME_DEBUG
	if (screen.machine().input().code_pressed(KEYCODE_Z))
	{
		int msk = 0;
		if (screen.machine().input().code_pressed(KEYCODE_Q))   msk |= 1;
		if (screen.machine().input().code_pressed(KEYCODE_W))   msk |= 2;
		if (screen.machine().input().code_pressed(KEYCODE_A))   msk |= 4;
		if (msk != 0) layers_ctrl &= msk;
	}
#endif

	bitmap.fill(m_palette->black_pen(), cliprect);

	if (layers_ctrl & 2)    draw_layer(screen, bitmap, cliprect, 2);
/*
    title:      17, 13/17
    dogs:       1b, 13, 17
    service:    17
    game:       17
*/
	if (*m_priority & 0x0008)
	{
		if (layers_ctrl & 4)    draw_layer(screen, bitmap, cliprect, 3);
		if (layers_ctrl & 1)    draw_layer(screen, bitmap, cliprect, 1);
	}
	else
	{
		if (layers_ctrl & 1)    draw_layer(screen, bitmap, cliprect, 1);
		if (layers_ctrl & 4)    draw_layer(screen, bitmap, cliprect, 3);
	}
	return 0;
}

/***************************************************************************
                                Protection
***************************************************************************/

READ16_MEMBER(bmcpokr_state::unk_r)
{
	return machine().rand();
}

// Hack!
READ16_MEMBER(bmcpokr_state::prot_r)
{
	switch (m_prot_val >> 8)
	{
		case 0x00:  return 0x1d << 8;
		case 0x94:  return 0x81 << 8;
	}
	return 0x00 << 8;
}
WRITE16_MEMBER(bmcpokr_state::prot_w)
{
	COMBINE_DATA(&m_prot_val);
//  logerror("%s: prot val = %04x\n", machine().describe_context(), m_prot_val);
}

/***************************************************************************
                                Memory Maps
***************************************************************************/

WRITE8_MEMBER(bmcpokr_state::mux_w)
{
	COMBINE_DATA(&m_mux);
	m_hopper->motor_w(BIT(data, 0)); // hopper motor
	machine().bookkeeping().coin_counter_w(1, BIT(data, 1));                // coin-in / key-in
	machine().bookkeeping().coin_counter_w(2, BIT(data, 2));                // pay-out
	//                           data & 0x60                  // DSW mux
	//                           data & 0x80                  // ? always on

//  popmessage("mux %04x", m_mux);
}
READ16_MEMBER(bmcpokr_state::dsw_r)
{
	switch ((m_mux >> 5) & 3)
	{
		case 0: return ioport("DSW4")->read() << 8;
		case 1: return ioport("DSW3")->read() << 8;
		case 2: return ioport("DSW2")->read() << 8;
		case 3: return ioport("DSW1")->read() << 8;
	}
	return 0xff << 8;
}

READ_LINE_MEMBER(bmcpokr_state::hopper_r)
{
	// motor off should clear the sense bit (I guess ticket.c should actually do this).
	// Otherwise a hopper bit stuck low will prevent several keys from being registered.
	return (m_mux & 0x01) ? m_hopper->line_r() : 1;
}

WRITE8_MEMBER(bmcpokr_state::irq_enable_w)
{
	COMBINE_DATA(&m_irq_enable);
}
WRITE8_MEMBER(bmcpokr_state::irq_ack_w)
{
	for (int i = 1; i < 8; i++)
	{
		if (BIT(data, i))
		{
			m_maincpu->set_input_line(i, CLEAR_LINE);
		}
	}
}

void bmcpokr_state::bmcpokr_mem(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x210000, 0x21ffff).ram().share("nvram");

	map(0x280000, 0x287fff).ram().w(FUNC(bmcpokr_state::videoram_w<0>)).share("videoram_1");
	map(0x288000, 0x28ffff).ram().w(FUNC(bmcpokr_state::videoram_w<1>)).share("videoram_2");
	map(0x290000, 0x297fff).ram();

	map(0x2a0000, 0x2dffff).ram().w(FUNC(bmcpokr_state::pixram_w)).share("pixram");

	map(0x2ff800, 0x2ff9ff).ram().share("scrollram_1");
	map(0x2ffa00, 0x2ffbff).ram().share("scrollram_2");
	map(0x2ffc00, 0x2ffdff).ram().share("scrollram_3");
	map(0x2ffe00, 0x2fffff).ram();

	map(0x320000, 0x320003).ram().share("layerctrl");

	map(0x330000, 0x330001).rw(FUNC(bmcpokr_state::prot_r), FUNC(bmcpokr_state::prot_w));

	map(0x340000, 0x340001).ram(); // 340001.b, rw
	map(0x340002, 0x340003).ram(); // 340003.b, w(9d)
	map(0x340007, 0x340007).w(FUNC(bmcpokr_state::irq_ack_w));
	map(0x340009, 0x340009).w(FUNC(bmcpokr_state::irq_enable_w));
	map(0x34000e, 0x34000f).ram().share("priority");    // 34000f.b, w (priority?)
	map(0x340017, 0x340017).w(FUNC(bmcpokr_state::pixpal_w));
	map(0x340018, 0x340019).ram(); // 340019.b, w
	map(0x34001a, 0x34001b).r(FUNC(bmcpokr_state::unk_r)).nopw();
	map(0x34001c, 0x34001d).ram(); // 34001d.b, w(0)

	map(0x350001, 0x350001).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x350003, 0x350003).w("ramdac", FUNC(ramdac_device::pal_w));
	map(0x350005, 0x350005).w("ramdac", FUNC(ramdac_device::mask_w));

	map(0x360000, 0x360001).r(FUNC(bmcpokr_state::dsw_r));

	map(0x370000, 0x370001).portr("INPUTS");

	map(0x380001, 0x380001).w(FUNC(bmcpokr_state::mux_w));

	map(0x390000, 0x390003).w("ymsnd", FUNC(ym2413_device::write)).umask16(0x00ff);
	map(0x398001, 0x398001).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));

	map(0x3b0000, 0x3b0001).portr("INPUTS2");
}


READ16_MEMBER(bmcpokr_state::mjmaglmp_dsw_r)
{
	switch ((m_mux >> 4) & 7)
	{
		case 7: return ioport("DSW1")->read() << 8;
		case 6: return ioport("DSW2")->read() << 8;
		case 5: return ioport("DSW3")->read() << 8;
		case 3: return ioport("DSW4")->read() << 8;
	}
	return 0xff << 8;
}

READ16_MEMBER(bmcpokr_state::mjmaglmp_key_r)
{
	uint16_t key = 0x3f;
	switch ((m_mux >> 4) & 7)
	{
		case 0: key = ioport("KEY1")->read(); break;
		case 1: key = ioport("KEY2")->read(); break;
		case 2: key = ioport("KEY3")->read(); break;
		case 3: key = ioport("KEY4")->read(); break;
		case 4: key = ioport("KEY5")->read(); break;
	}
	return ioport("INPUTS")->read() | (key & 0x3f);
}

void bmcpokr_state::mjmaglmp_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x210000, 0x21ffff).ram().share("nvram");

	map(0x280000, 0x287fff).ram().w(FUNC(bmcpokr_state::videoram_w<0>)).share("videoram_1");
	map(0x288000, 0x28ffff).ram().w(FUNC(bmcpokr_state::videoram_w<1>)).share("videoram_2");
	map(0x290000, 0x297fff).ram();

	map(0x2a0000, 0x2dffff).ram().w(FUNC(bmcpokr_state::pixram_w)).share("pixram");

	map(0x2ff800, 0x2ff9ff).ram().share("scrollram_1");
	map(0x2ffa00, 0x2ffbff).ram().share("scrollram_2");
	map(0x2ffc00, 0x2ffdff).ram().share("scrollram_3");
	map(0x2ffe00, 0x2fffff).ram();

	map(0x320000, 0x320003).ram().share("layerctrl");

	map(0x388001, 0x388001).w(FUNC(bmcpokr_state::mux_w));

	map(0x390000, 0x390001).r(FUNC(bmcpokr_state::mjmaglmp_dsw_r));

	map(0x398000, 0x398001).r(FUNC(bmcpokr_state::mjmaglmp_key_r));

	map(0x3c8800, 0x3c8803).w("ymsnd", FUNC(ym2413_device::write)).umask16(0x00ff);
	map(0x3c9001, 0x3c9001).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));

	map(0x3c9801, 0x3c9801).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x3c9803, 0x3c9803).w("ramdac", FUNC(ramdac_device::pal_w));
	map(0x3c9805, 0x3c9805).w("ramdac", FUNC(ramdac_device::mask_w));

	map(0x3ca000, 0x3ca001).ram(); // 3ca001.b, rw
	map(0x3ca002, 0x3ca003).ram(); // 3ca003.b, w(9d)
	map(0x3ca007, 0x3ca007).w(FUNC(bmcpokr_state::irq_ack_w));
	map(0x3ca009, 0x3ca009).w(FUNC(bmcpokr_state::irq_enable_w));
	map(0x3ca00e, 0x3ca00f).ram().share("priority");    // 3ca00f.b, w (priority?)
	map(0x3ca017, 0x3ca017).w(FUNC(bmcpokr_state::pixpal_w));
	map(0x3ca018, 0x3ca019).ram(); // 3ca019.b, w
	map(0x3ca01a, 0x3ca01b).r(FUNC(bmcpokr_state::unk_r)).nopw();
	map(0x3ca01c, 0x3ca01d).ram(); // 3ca01d.b, w(0)
}

/***************************************************************************
                                Input Ports
***************************************************************************/

static INPUT_PORTS_START( bmcpokr )
	PORT_START("INPUTS")
	// Poker controls:
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN  ) PORT_CONDITION("DSW4",0x80,EQUALS,0x80) // KEY-IN          [KEY-IN, credit +500]
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_POKER_HOLD5   ) PORT_CONDITION("DSW4",0x80,EQUALS,0x80) // HOLD 5
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_POKER_HOLD4   ) PORT_CONDITION("DSW4",0x80,EQUALS,0x80) // HOLD 4
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_POKER_HOLD2   ) PORT_CONDITION("DSW4",0x80,EQUALS,0x80) // HOLD 2
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_POKER_HOLD1   ) PORT_CONDITION("DSW4",0x80,EQUALS,0x80) // HOLD 1          [INSTRUCTIONS]
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_POKER_HOLD3   ) PORT_CONDITION("DSW4",0x80,EQUALS,0x80) // HOLD 3
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL   ) PORT_CONDITION("DSW4",0x80,EQUALS,0x80) // n.a.            [START, ESC in service mode]
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE   ) PORT_CONDITION("DSW4",0x80,EQUALS,0x80) // SCORE
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_POKER_BET     ) PORT_CONDITION("DSW4",0x80,EQUALS,0x80) // BET             [BET, credit -1]
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH,IPT_CUSTOM       ) PORT_READ_LINE_MEMBER(bmcpokr_state, hopper_r)  // HP [HOPPER, credit -100]
	PORT_SERVICE_NO_TOGGLE( 0x0400, IP_ACTIVE_LOW      ) PORT_CONDITION("DSW4",0x80,EQUALS,0x80) // ACCOUNT         [SERVICE MODE]
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_CONDITION("DSW4",0x80,EQUALS,0x80) // KEY-OUT         [KEY-OUT, no hopper]
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP   ) PORT_CONDITION("DSW4",0x80,EQUALS,0x80) // DOUBLE-UP
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_GAMBLE_LOW    ) PORT_CONDITION("DSW4",0x80,EQUALS,0x80) // SMALL
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH   ) PORT_CONDITION("DSW4",0x80,EQUALS,0x80) // BIG
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN1         ) PORT_CONDITION("DSW4",0x80,EQUALS,0x80) PORT_IMPULSE(5) // COIN-IN [COIN-IN, credit +100, coin-jam]

	// Joystick controls:
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN  )                PORT_CONDITION("DSW4",0x80,EQUALS,0x00) // B2              [KEY-IN, credit +500]
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON3       ) PORT_PLAYER(1) PORT_CONDITION("DSW4",0x80,EQUALS,0x00) // C1
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON2       ) PORT_PLAYER(1) PORT_CONDITION("DSW4",0x80,EQUALS,0x00) // B1
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(1) PORT_CONDITION("DSW4",0x80,EQUALS,0x00) // <Right>1 (4th)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_POKER_HOLD1   )                PORT_CONDITION("DSW4",0x80,EQUALS,0x00) // <Left>1 (3rd)   [INSTRUCTIONS]
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1       ) PORT_PLAYER(1) PORT_CONDITION("DSW4",0x80,EQUALS,0x00) // A1
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL   )                PORT_CONDITION("DSW4",0x80,EQUALS,0x00) // n.a.            [START, ESC in service mode]
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_CONDITION("DSW4",0x80,EQUALS,0x00) // <Left>2 (3rd)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_POKER_BET     )                PORT_CONDITION("DSW4",0x80,EQUALS,0x00) // <Down>1 (2nd)   [BET, credit -1]
//  PORT_BIT( 0x0200, IP_ACTIVE_HIGH,IPT_CUSTOM       ) PORT_READ_LINE_MEMBER(bmcpokr_state, hopper_r)  // HP [HOPPER, credit -100]
	PORT_SERVICE_NO_TOGGLE( 0x0400, IP_ACTIVE_LOW      )                PORT_CONDITION("DSW4",0x80,EQUALS,0x00) // A2              [SERVICE MODE]
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )                PORT_CONDITION("DSW4",0x80,EQUALS,0x00) // C2              [KEY-OUT, no hopper]
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP   )                PORT_CONDITION("DSW4",0x80,EQUALS,0x00) // S1              [START, ESC in service mode]
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(2) PORT_CONDITION("DSW4",0x80,EQUALS,0x00) // <Right>2 (4th)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP   ) PORT_PLAYER(1) PORT_CONDITION("DSW4",0x80,EQUALS,0x00) // <Up>1 (1st)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN1         )                PORT_CONDITION("DSW4",0x80,EQUALS,0x00) PORT_IMPULSE(5) // <coin-in> (1st) [COIN-IN, credit +100, coin-jam]

	PORT_START("INPUTS2")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) // <coin-out> (2nd)    [COIN-OUT, hopper (otherwise pay-error)]

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("DIP1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	PORT_DIPNAME( 0x02, 0x00, "Doube-Up Game" ) PORT_DIPLOCATION("DIP1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x02, DEF_STR( No ) )
	PORT_DIPNAME( 0x04, 0x00, "Slot Machine" ) PORT_DIPLOCATION("DIP1:3")
	PORT_DIPSETTING(    0x00, "Machinery" )
	PORT_DIPSETTING(    0x04, "??" )
	PORT_DIPNAME( 0x08, 0x00, "Poker Game" ) PORT_DIPLOCATION("DIP1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "DIP1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "DIP1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "DIP1:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "DIP1:8" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, "Credit Limit" ) PORT_DIPLOCATION("DIP2:1,2")
	PORT_DIPSETTING(    0x03, "5k" )
	PORT_DIPSETTING(    0x02, "10k" )
	PORT_DIPSETTING(    0x01, "50k" )
	PORT_DIPSETTING(    0x00, "100k" )
	PORT_DIPNAME( 0x0c, 0x0c, "Key-In Limit" ) PORT_DIPLOCATION("DIP2:3,4")
	PORT_DIPSETTING(    0x0c, "5k" )
	PORT_DIPSETTING(    0x08, "10k" )
	PORT_DIPSETTING(    0x04, "20k" )
	PORT_DIPSETTING(    0x00, "50k" )
	PORT_DIPNAME( 0x10, 0x10, "Open Cards Mode" ) PORT_DIPLOCATION("DIP2:5")
	PORT_DIPSETTING(    0x10, "Reels" )
	PORT_DIPSETTING(    0x00, "Turn Over" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "DIP2:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "DIP2:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "DIP2:8" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "Win Rate" ) PORT_DIPLOCATION("DIP3:1,2")
	PORT_DIPSETTING(    0x02, "96" )
	PORT_DIPSETTING(    0x01, "97" )
	PORT_DIPSETTING(    0x03, "98" )
	PORT_DIPSETTING(    0x00, "99" )
	PORT_DIPNAME( 0x0c, 0x0c, "Double-Up Rate" ) PORT_DIPLOCATION("DIP3:3,4")
	PORT_DIPSETTING(    0x08, "93" )
	PORT_DIPSETTING(    0x04, "94" )
	PORT_DIPSETTING(    0x00, "95" )
	PORT_DIPSETTING(    0x0c, "96" )
	PORT_DIPNAME( 0x10, 0x10, "Bonus Bet" ) PORT_DIPLOCATION("DIP3:5")
	PORT_DIPSETTING(    0x10, "30" )
	PORT_DIPSETTING(    0x00, "48" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "DIP3:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "DIP3:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "DIP3:8" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, "Max Bet" ) PORT_DIPLOCATION("DIP4:1")
	PORT_DIPSETTING(    0x01, "48" )
	PORT_DIPSETTING(    0x00, "96" )
	PORT_DIPNAME( 0x06, 0x06, "Min Bet" ) PORT_DIPLOCATION("DIP4:2,3")
	PORT_DIPSETTING(    0x06, "6" )
	PORT_DIPSETTING(    0x04, "12" )
	PORT_DIPSETTING(    0x02, "18" )
	PORT_DIPSETTING(    0x00, "30" )
	PORT_DIPNAME( 0x18, 0x18, "Credits Per Coin" ) PORT_DIPLOCATION("DIP4:4,5")
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPSETTING(    0x08, "20" )
	PORT_DIPSETTING(    0x18, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x60, 0x60, "Credits Per Key-In" ) PORT_DIPLOCATION("DIP4:6,7")
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x20, "50" )
	PORT_DIPSETTING(    0x60, "100" )
	PORT_DIPSETTING(    0x00, "500" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Controls ) ) PORT_DIPLOCATION("DIP4:8")
	PORT_DIPSETTING(    0x80, "Poker" )
	PORT_DIPSETTING(    0x00, DEF_STR( Joystick ) )
INPUT_PORTS_END

static INPUT_PORTS_START( mjmaglmp )
	PORT_START("INPUTS")
	// Joystick controls:
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1         ) PORT_CONDITION("DSW2",0x01,EQUALS,0x00) // START
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_CONDITION("DSW2",0x01,EQUALS,0x00) // UP
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_CONDITION("DSW2",0x01,EQUALS,0x00) // DOWN
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_CONDITION("DSW2",0x01,EQUALS,0x00) // LEFT
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_CONDITION("DSW2",0x01,EQUALS,0x00) // RIGHT
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_CONDITION("DSW2",0x01,EQUALS,0x00) // 1P E1 (select)

	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN2          ) // NOTE
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT  ) // KEY DOWN
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Pay Out") PORT_CODE(KEYCODE_O) // PAY
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH,IPT_CUSTOM        ) PORT_READ_LINE_MEMBER(bmcpokr_state, hopper_r)  // HOPPER
	PORT_SERVICE_NO_TOGGLE( 0x0400, IP_ACTIVE_LOW       ) // ACCOUNT
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_SERVICE1       ) PORT_NAME("Reset") // RESET
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN        ) // (unused)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2        ) PORT_CONDITION("DSW2",0x01,EQUALS,0x00) // 1P E2 (bet)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3        ) PORT_CONDITION("DSW2",0x01,EQUALS,0x00) // 1P E3 (select)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN1          ) // COIN

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A        ) PORT_CONDITION("DSW2",0x01,EQUALS,0x01)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E        ) PORT_CONDITION("DSW2",0x01,EQUALS,0x01)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I        ) PORT_CONDITION("DSW2",0x01,EQUALS,0x01)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M        ) PORT_CONDITION("DSW2",0x01,EQUALS,0x01)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN      ) PORT_CONDITION("DSW2",0x01,EQUALS,0x01)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1           ) PORT_CONDITION("DSW2",0x01,EQUALS,0x01)

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B        ) PORT_CONDITION("DSW2",0x01,EQUALS,0x01)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F        ) PORT_CONDITION("DSW2",0x01,EQUALS,0x01)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J        ) PORT_CONDITION("DSW2",0x01,EQUALS,0x01)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N        ) PORT_CONDITION("DSW2",0x01,EQUALS,0x01)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH    ) PORT_CONDITION("DSW2",0x01,EQUALS,0x01)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET      ) PORT_CONDITION("DSW2",0x01,EQUALS,0x01)

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C        ) PORT_CONDITION("DSW2",0x01,EQUALS,0x01)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G        ) PORT_CONDITION("DSW2",0x01,EQUALS,0x01)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K        ) PORT_CONDITION("DSW2",0x01,EQUALS,0x01)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI      ) PORT_CONDITION("DSW2",0x01,EQUALS,0x01)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON      ) PORT_CONDITION("DSW2",0x01,EQUALS,0x01)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN          ) PORT_CONDITION("DSW2",0x01,EQUALS,0x01)

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D        ) PORT_CONDITION("DSW2",0x01,EQUALS,0x01)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H        ) PORT_CONDITION("DSW2",0x01,EQUALS,0x01)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L        ) PORT_CONDITION("DSW2",0x01,EQUALS,0x01)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON      ) PORT_CONDITION("DSW2",0x01,EQUALS,0x01)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN          ) PORT_CONDITION("DSW2",0x01,EQUALS,0x01)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN          ) PORT_CONDITION("DSW2",0x01,EQUALS,0x01)

	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN          ) PORT_CONDITION("DSW2",0x01,EQUALS,0x01)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE    ) PORT_CONDITION("DSW2",0x01,EQUALS,0x01)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP) PORT_CONDITION("DSW2",0x01,EQUALS,0x01)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN          ) PORT_CONDITION("DSW2",0x01,EQUALS,0x01)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG      ) PORT_CONDITION("DSW2",0x01,EQUALS,0x01)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL    ) PORT_CONDITION("DSW2",0x01,EQUALS,0x01)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("DIP1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	PORT_DIPNAME( 0x02, 0x00, "Doube-Up Game" )             PORT_DIPLOCATION("DIP1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x02, DEF_STR( No ) )
	PORT_DIPNAME( 0x04, 0x04, "Coin Sw. Function" )         PORT_DIPLOCATION("DIP1:3")
	PORT_DIPSETTING(    0x00, "Coin" )
	PORT_DIPSETTING(    0x04, "Note" )
	PORT_DIPNAME( 0x08, 0x08, "Pay Sw. Function" )          PORT_DIPLOCATION("DIP1:4")
	PORT_DIPSETTING(    0x00, "Pay-Out" )
	PORT_DIPSETTING(    0x08, "Key-Down" )
	PORT_DIPNAME( 0x10, 0x10, "Game Hint" )                 PORT_DIPLOCATION("DIP1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPNAME( 0x20, 0x20, "Direct Double" )             PORT_DIPLOCATION("DIP1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPNAME( 0x40, 0x40, "Coin Acceptor" )             PORT_DIPLOCATION("DIP1:7")
	PORT_DIPSETTING(    0x00, "Mechanical" )
	PORT_DIPSETTING(    0x40, "Electronic" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "DIP1:8" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Controls ) )         PORT_DIPLOCATION("DIP2:1")
	PORT_DIPSETTING(    0x01, "Keyboard" )
	PORT_DIPSETTING(    0x00, DEF_STR( Joystick ) )
	PORT_DIPNAME( 0x02, 0x02, "Key-In Limit" )              PORT_DIPLOCATION("DIP2:2")
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPSETTING(    0x02, "5000" )
	PORT_DIPNAME( 0x04, 0x04, "Double Lose Pool" )          PORT_DIPLOCATION("DIP2:3")
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPSETTING(    0x04, "100" )
	PORT_DIPNAME( 0x18, 0x18, "Double Over / Round Bonus" ) PORT_DIPLOCATION("DIP2:4,5")
	PORT_DIPSETTING(    0x10, "100 / 10" )
	PORT_DIPSETTING(    0x18, "200 / 10" )
	PORT_DIPSETTING(    0x08, "300 / 15" )
	PORT_DIPSETTING(    0x00, "500 / 25" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "DIP2:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "DIP2:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "DIP2:8" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "Pay-Out Rate" )              PORT_DIPLOCATION("DIP3:1,2")
	PORT_DIPSETTING(    0x02, "75" )
	PORT_DIPSETTING(    0x01, "82" )
	PORT_DIPSETTING(    0x03, "85" )
	PORT_DIPSETTING(    0x00, "88" )
	PORT_DIPNAME( 0x0c, 0x0c, "Double-Up Rate" )            PORT_DIPLOCATION("DIP3:3,4")
	PORT_DIPSETTING(    0x08, "95" )
	PORT_DIPSETTING(    0x04, "96" )
	PORT_DIPSETTING(    0x00, "97" )
	PORT_DIPSETTING(    0x0c, "98" )
	PORT_DIPNAME( 0x30, 0x30, "Game Enhance Type" )         PORT_DIPLOCATION("DIP3:5,6")
	PORT_DIPSETTING(    0x10, "Small" )
	PORT_DIPSETTING(    0x00, "Big" )
	PORT_DIPSETTING(    0x30, "Normal" )
	PORT_DIPSETTING(    0x20, "Bonus" )
	PORT_DIPNAME( 0xc0, 0xc0, "Credit Limit" )              PORT_DIPLOCATION("DIP3:7,8")
	PORT_DIPSETTING(    0x00, "300" )
	PORT_DIPSETTING(    0x80, "500" )
	PORT_DIPSETTING(    0x40, "1000" )
	PORT_DIPSETTING(    0xc0, "2000" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, "Max Bet" )                   PORT_DIPLOCATION("DIP4:1")
	PORT_DIPSETTING(    0x01, "10" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x06, 0x06, "Min Bet" )                   PORT_DIPLOCATION("DIP4:2,3")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x06, "3" )
	PORT_DIPSETTING(    0x04, "6" )
	PORT_DIPSETTING(    0x02, "9" )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Coinage ) )          PORT_DIPLOCATION("DIP4:4,5")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x60, 0x60, "Credits Per Key-In" )        PORT_DIPLOCATION("DIP4:6,7")
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPSETTING(    0x60, "10" )
	PORT_DIPSETTING(    0x20, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "DIP4:8" )
INPUT_PORTS_END

/***************************************************************************
                                Graphics Layout
***************************************************************************/

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,2),
	8,
	{ RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+1, RGN_FRAC(1,2)+2, RGN_FRAC(1,2)+3, 0, 1, 2, 3  },
	{ STEP8(0, 4) },
	{ STEP8(0, 4*8) },
	8*8*4
};

static GFXDECODE_START( gfx_bmcpokr )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 1 )
GFXDECODE_END

/***************************************************************************
                                Machine Drivers
***************************************************************************/

TIMER_DEVICE_CALLBACK_MEMBER(bmcpokr_state::interrupt)
{
	int scanline = param;

	if (scanline == 240)
		if (BIT(m_irq_enable, 2)) m_maincpu->set_input_line(2, ASSERT_LINE);

	if (scanline == 128)
		if (BIT(m_irq_enable, 3)) m_maincpu->set_input_line(3, ASSERT_LINE);

	if (scanline == 64)
		if (BIT(m_irq_enable, 6)) m_maincpu->set_input_line(6, ASSERT_LINE);
}

void bmcpokr_state::ramdac_map(address_map &map)
{
	map(0x000, 0x3ff).rw("ramdac", FUNC(ramdac_device::ramdac_pal_r), FUNC(ramdac_device::ramdac_rgb666_w));
}

void bmcpokr_state::machine_start()
{
	save_item(NAME(m_prot_val));
	save_item(NAME(m_mux));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_pixpal));
}

void bmcpokr_state::bmcpokr(machine_config &config)
{
	M68000(config, m_maincpu, XTAL(42'000'000) / 4); // 68000 @10.50MHz (42/4)
	m_maincpu->set_addrmap(AS_PROGRAM, &bmcpokr_state::bmcpokr_mem);

	TIMER(config, "scantimer", 0).configure_scanline(FUNC(bmcpokr_state::interrupt), "screen", 0, 1);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh(HZ_TO_ATTOSECONDS(58.935));    // HSync - 15.440kHz, VSync - 58.935Hz
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update(FUNC(bmcpokr_state::screen_update));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 60*8-1, 0*8, 30*8-1);
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_entries(256);

	ramdac_device &ramdac(RAMDAC(config, "ramdac", 0, m_palette));
	ramdac.set_addrmap(0, &bmcpokr_state::ramdac_map);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_bmcpokr);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	TICKET_DISPENSER(config, m_hopper, 0);
	m_hopper->set_period(attotime::from_msec(10));
	m_hopper->set_senses(TICKET_MOTOR_ACTIVE_HIGH, TICKET_STATUS_ACTIVE_LOW, false);    // hopper stuck low if too slow

	SPEAKER(config, "mono").front_center();

	YM2413(config, "ymsnd", XTAL(42'000'000) / 12).add_route(ALL_OUTPUTS, "mono", 1.00);    // UM3567 @3.50MHz (42/12)

	OKIM6295(config, "oki", XTAL(42'000'000) / 40, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.00);   // M6295 @1.05MHz (42/40), pin 7 not verified
}

void bmcpokr_state::mjmaglmp(machine_config &config)
{
	bmcpokr(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &bmcpokr_state::mjmaglmp_map);
}

/***************************************************************************
                                ROMs Loading
***************************************************************************/

/***************************************************************************

Dongfang Shenlong ("Eastern Dragon")
BMC 1999

PCB Layout
----------

BMC-A81212
|---------------------------------------|
|          CH-A-401  CH-M-301  CH-M-701 |
|M11B416256A                   UM3567   |
|42MHz     CH-M-201  CH-M-101     M6295 |
|      VDB40817                        1|
|              HM86171-80        VOL   0|
|      SYA70521                        W|
|                        LM324   7805  A|
|DSW1                                  Y|
|      68000                   TDA2003  |
|DSW2       CH-M-505  CH-M-605          |
|            6264     6264             2|
|DSW3  CPLD   74HC132 74LS05           2|
|                 555                  W|
|DSW4                                  A|
|        BATT  SW       JAMMA          Y|
|---------------------------------------|
Notes:
      RAM - M11B416256, 6264(x2)
      VDB40817/SYA70521 - Unknown QFP100
      CPLD - unknown PLCC44 chip labelled 'BMC B816140'
      BATT - 5.5 volt 0.047F super cap
      68000 @10.50MHz (42/4)
      M6295 @1.05MHz (42/40)
      UM3567 @3.50MHz (42/12)
      HSync - 15.440kHz
      VSync - 58.935Hz

***************************************************************************/

ROM_START( bmcpokr )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "ch-m-605.u13", 0x000000, 0x20000, CRC(c5c3fcd1) SHA1(b77fef734c290d52ae877a24bb3ee42b24eb5cb8) )
	ROM_LOAD16_BYTE( "ch-m-505.u12", 0x000001, 0x20000, CRC(d6effaf1) SHA1(b446d3beb3393bc8b3bcd0d543945e6fb6a375b9) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "ch-m-101.u39", 0x000000, 0x80000, CRC(f4b82e0a) SHA1(f545c6ab1375518de06900f02a0eb5af1edeeb47) )
	ROM_LOAD16_BYTE( "ch-m-201.u40", 0x000001, 0x80000, CRC(520571cb) SHA1(5c006f10d6192939003f8197e8bb64908a826fc1) )
	ROM_LOAD16_BYTE( "ch-m-301.u45", 0x100000, 0x80000, CRC(daba09c3) SHA1(e5d2f92b63288c36faa367a3306d1999264843e8) )
	ROM_LOAD16_BYTE( "ch-a-401.u29", 0x100001, 0x80000, CRC(5ee5d39f) SHA1(f6881aa5c755831d885f7adf35a5a094f7302205) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "ch-m-701.u10", 0x00000, 0x40000,  CRC(e01be644) SHA1(b68682786d5b40cb5672cfd7f717adcfb8fac7d3) )
ROM_END

/***************************************************************************

Mahjong Magic Lamp (BMC, 2000)

PCB Layout
----------

BMC-A70809
  |--------------------------------------|
|-|               6116          555      |
|            0.1UF    JA-A-602  JA-A-502 |
|                                  68000 |
|   TD62003                   42MHz      |
|            PAL                         |
|            PAL                 51C4160 |
|                         SYA70521       |
|-|    U3567                             |
  |                                      |
  |             HM86171   VDB40817       |
|-| VOL                             DSW4 |
|  7805  6295                       DSW3 |
|  AMP          JA-A-301 JA-A-401   DSW2 |
|-|    JA-A-901 JA-A-201 JA-A-101   DSW1 |
  |--------------------------------------|
Notes:
      68000   - clock 10.5000MHz [42/4]
      M6295   - clock 1.0500MHz [42/40]. Pin 7 HIGH
      U3567   - = YM2413, clock 3.5000MHz [42/12]
      HM86171 - HMC RAMDAC. Clock input 10.5000MHz [42/4]
      VDB/SYA - custom QFP100 GFX chips badged with BMC logo
      51C4160 - SOJ40 video RAM, possibly 4M DRAM (256k x 16-bit)
      555     - 555 Timer
      DSW1-4  - 8-position DIP switches
      AMP     - NEC uPC1241H
      VSync   - 58.9342Hz
      HSync   - 15.4408kHz

***************************************************************************/

ROM_START( mjmaglmp )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "ja-a-602.u10", 0x000000, 0x20000, CRC(b69e235c) SHA1(04e5d0d667de29680e4a35d0d98b587447e54ce3) )
	ROM_LOAD16_BYTE( "ja-a-502.u11", 0x000001, 0x20000, CRC(bb609da3) SHA1(ffadc20912e0a9ebe0d1a1f7f94dfaccb48be5c1) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "ja-a-101.u41", 0x000000, 0x80000, CRC(7878b9a1) SHA1(7efacb063b47e518c4d3856e90d7532f478e54dd) )
	ROM_LOAD16_BYTE( "ja-a-201.u42", 0x000001, 0x80000, CRC(b74f3b2b) SHA1(09724909a14aebc135029d97fafcd215a84f05e3) )
	ROM_LOAD16_BYTE( "ja-a-301.u43", 0x100000, 0x80000, CRC(2bbaf65e) SHA1(d792054671671a2e479b89ad29bc7b3f935804f9) )
	ROM_LOAD16_BYTE( "ja-a-401.u44", 0x100001, 0x80000, CRC(9292acb1) SHA1(01ce7997305dd5fdc5dc2b801046303a4d8a89c0) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "ja-a-901.u6", 0x00000, 0x40000, CRC(25f36d00) SHA1(c182348340ca67ad69d1a67c58b47d6371a725c9) )
ROM_END

GAME( 1999, bmcpokr,  0, bmcpokr,  bmcpokr,  bmcpokr_state, empty_init, ROT0, "BMC", "Dongfang Shenlong",             MACHINE_SUPPORTS_SAVE )
GAME( 2000, mjmaglmp, 0, mjmaglmp, mjmaglmp, bmcpokr_state, empty_init, ROT0, "BMC", "Mahjong Magic Lamp (v. JAA02)", MACHINE_SUPPORTS_SAVE )
