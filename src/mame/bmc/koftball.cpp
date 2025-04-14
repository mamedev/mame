// license:BSD-3-Clause
// copyright-holders: Tomasz Slanina

/*
King Of Football (c)1995 BMC

preliminary driver by Tomasz Slanina

TODO:
- lots of unknown writes / reads;
- one of the customs could contain a VIA6522-like core. bmc/bmcbowl.cpp uses the VIA6522 and the
  accesses are similar;
- probably jxzh also supports the mahjong keyboard. Check if one of the dips enable it and where it
  is read;
- better understanding of the koftball protection.

--

MC68000P10
M28 (OKI 6295, next to ROM C9)
BMC ADB40817(80 Pin PQFP - Google hits, but no datasheet or description)
RAMDAC TRC1710-80PCA (Monolithic 256-word by 18bit Look-up Table & Triple Video DAC with 6-bit DACs)
File 89C67 (Clone of YM2413. Next to 3.57954MHz OSC)
OSC: 21.47727MHz & 3.57954MHz
2 8-way dipswitches
part # scratched 64 pin PLCC (soccer ball sticker over this chip ;-)

ft5_v16_c5.u14 \
ft5_v16_c6.u15 | 68000 program code

ft5_v6_c9.u21 - Sound samples

ft5_v6_c1.u59 \
ft5_v6_c2.u60 | Graphics
ft5_v6_c3.u61 |
ft5_v6_c4.u58 /

*/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "machine/nvram.h"
#include "machine/timer.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"
#include "video/ramdac.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


// configurable logging
#define LOG_GFX   (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_GFX)

#include "logmacro.h"

#define LOGGFX(...)   LOGMASKED(LOG_GFX,   __VA_ARGS__)


namespace {


#define NVRAM_HACK 1

class koftball_state : public driver_device
{
public:
	koftball_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_main_ram(*this, "main_ram"),
		m_videoram(*this, "videoram%u", 0U),
		m_pixram(*this, "pixram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void jxzh(machine_config &config) ATTR_COLD;
	void kaimenhu(machine_config &config) ATTR_COLD;
	void koftball(machine_config &config) ATTR_COLD;

	void init_koftball() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	optional_shared_ptr<u16> m_main_ram;
	required_shared_ptr_array<u16, 4> m_videoram;
	required_shared_ptr<u16> m_pixram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	tilemap_t *m_tilemap[4]{};
	u16 m_prot_data = 0;
	u8 m_irq_enable = 0;
	u8 m_gfx_ctrl = 0;
	u8 m_priority = 0;
	u8 m_backpen = 0;
	std::unique_ptr<bitmap_ind16> m_pixbitmap;
	u8 m_pixpal = 0;

	void irq_ack_w(u8 data);
	u16 random_number_r();
	u16 prot_r();
	u16 kaimenhu_prot_r();
	void prot_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void pixpal_w(offs_t offset, u8 data, u8 mem_mask = ~0);
	template <u8 Which> void videoram_w(offs_t offset, u16 data, u16 mem_mask = ~0);


	template <u8 Which> TILE_GET_INFO_MEMBER(get_tile_info);
	void draw_pixlayer(bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	TIMER_DEVICE_CALLBACK_MEMBER(interrupt);
	void jxzh_mem(address_map &map) ATTR_COLD;
	void kaimenhu_mem(address_map &map) ATTR_COLD;
	void koftball_mem(address_map &map) ATTR_COLD;
	void ramdac_map(address_map &map) ATTR_COLD;
};


void koftball_state::machine_start()
{
	m_prot_data = 0;

	save_item(NAME(m_prot_data));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_gfx_ctrl));
	save_item(NAME(m_priority));
	save_item(NAME(m_backpen));
	save_item(NAME(m_pixpal));
}

template <u8 Which>
TILE_GET_INFO_MEMBER(koftball_state::get_tile_info)
{
	int const data = m_videoram[Which][tile_index];
	tileinfo.set(0, data, 0, 0);
}

void koftball_state::pixpal_w(offs_t offset, u8 data, u8 mem_mask)
{
	COMBINE_DATA(&m_pixpal);
}

void koftball_state::video_start()
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(koftball_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(koftball_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(koftball_state::get_tile_info<2>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_tilemap[3] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(koftball_state::get_tile_info<3>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_tilemap[0]->set_transparent_pen(0);
	m_tilemap[2]->set_transparent_pen(0);

	m_tilemap[1]->set_transparent_pen(0);
	m_tilemap[3]->set_transparent_pen(0);

	m_pixbitmap = std::make_unique<bitmap_ind16>(512, 256);
}

// linear 512x256x4bpp
void koftball_state::draw_pixlayer(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const u8 pix_bank = (m_pixpal & 0xf) << 4;

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		const u16 pitch = y * 0x80;
		for (int x = cliprect.min_x; x <= cliprect.max_x >> 2; x++)
		{
			const u16 tile_data = m_pixram[(pitch + x) & 0xffff];
			for (int xi = 0; xi < 4; xi++)
			{
				const u8 nibble = (tile_data >> ((3 - xi) * 4)) & 0xf;
				if (nibble)
					bitmap.pix(y, (x << 2) + xi) = pix_bank | nibble;
			}
		}
	}
}

u32 koftball_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/*
	TODO:
	there are a lot of unused bits written to both 0x320000 and 0x2a000f during screen changes.
	Bit 1 and 5 of 0x320000 are the only ones that give correct layer results (see table below).
	What is the meaning of the others? More games running on this hw would help.

	The following table describes the attract mode sequence for jxzh. koftball is relatively simpler as it almost
	only uses the first 2 layers (only noted use of the 3rd layer is for the bookkeeping screen).

	                            layer0      layer 1     layer 2     layer 3     0x2a000f    0x320000
	title screen                over        under       off         off         0x13        0x98
	girl with scrolling tiles   prev screen prev screen over        under       0x1b        0xba
	                                                                            0x00        0x00
	demon                       over        under       prev screen prev screen 0x1f        0x98
	                                                                            0x00        0x00
	slot with road signs        prev screen prev screen over        under       0x17        0xba
	Chinese lanterns            prev screen prev screen over        under       0x17        0xba
	slot with numbers           prev screen prev screen over        under       0x17        0xba
	                                                                            0x00        0x00
	pirates                     over        under       prev screen prev screen 0x17        0x98
	                                                                            0x00        0x00
	tile race                   over        under       prev screen prev screen 0x17        0x98
	                                                                            0x00        0x00
	girl select after coin up   prev screen prev screen over        under       0x13        0x3a
	*/

	m_pixbitmap->fill(0, cliprect);
	draw_pixlayer(*m_pixbitmap, cliprect);

	bitmap.fill(m_backpen, cliprect);

	if (BIT(m_priority, 3))
		copyscrollbitmap_trans(bitmap, *m_pixbitmap, 0, 0, 0, 0, cliprect, 0);

	// TODO: or bit 1?
	if (BIT(m_gfx_ctrl, 5))
	{
		m_tilemap[3]->draw(screen, bitmap, cliprect, 0, 0);
		m_tilemap[2]->draw(screen, bitmap, cliprect, 0, 0);
	}
	else
	{
		m_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);
		m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
	}

	if (!BIT(m_priority, 3))
		copyscrollbitmap_trans(bitmap, *m_pixbitmap, 0, 0, 0, 0, cliprect, 0);

	return 0;
}

u16 koftball_state::random_number_r()
{
	return machine().rand();
}


u16 koftball_state::prot_r()
{
	switch (m_prot_data)
	{
		case 0x0000: return 0x0d00;
		case 0xff00: return 0x8d00;

		case 0x8000: return 0x0f0f;
	}

	logerror("unk prot r %x %x\n", m_prot_data, m_maincpu->pcbase());
	return machine().rand();
}

u16 koftball_state::kaimenhu_prot_r()
{
	switch (m_prot_data)
	{
		case 0x0000: return 0x1d00;
		case 0xff00: return 0x9d00;

		//case 0x8000: return 0x0f0f;
	}

	logerror("unk prot r %x %x\n", m_prot_data, m_maincpu->pcbase());
	return machine().rand();
}

void koftball_state::prot_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_prot_data);
}

template <u8 Which>
void koftball_state::videoram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_videoram[Which][offset]);
	m_tilemap[Which]->mark_tile_dirty(offset);
}

void koftball_state::irq_ack_w(u8 data)
{
	for (int i = 1; i < 8; i++)
		if (BIT(data, i))
			m_maincpu->set_input_line(i, CLEAR_LINE);
}

// FIXME: merge video maps
void koftball_state::koftball_mem(address_map &map)
{
	map(0x000000, 0x01ffff).rom();
	map(0x220000, 0x22ffff).ram().share(m_main_ram);

	map(0x260000, 0x260fff).ram().w(FUNC(koftball_state::videoram_w<0>)).share(m_videoram[0]);
	map(0x261000, 0x261fff).ram().w(FUNC(koftball_state::videoram_w<1>)).share(m_videoram[1]);
	map(0x262000, 0x262fff).ram().w(FUNC(koftball_state::videoram_w<2>)).share(m_videoram[2]);
	map(0x263000, 0x263fff).ram().w(FUNC(koftball_state::videoram_w<3>)).share(m_videoram[3]);
	map(0x268000, 0x26ffff).ram();

	map(0x280000, 0x29ffff).ram().share(m_pixram);
	map(0x2a0007, 0x2a0007).w(FUNC(koftball_state::irq_ack_w));
	map(0x2a0009, 0x2a0009).lw8(NAME([this] (u8 data) { m_irq_enable = data; }));
	map(0x2a000f, 0x2a000f).lw8(NAME([this] (u8 data) { m_priority = data; LOGGFX("GFX ctrl $2a000f (priority) %02x\n", data); }));
	map(0x2a0017, 0x2a0017).w(FUNC(koftball_state::pixpal_w));
	map(0x2a0019, 0x2a0019).lw8(NAME([this] (u8 data) { m_backpen = data; LOGGFX("GFX ctrl $2a0019 (backpen) %02x\n", data); }));
	map(0x2a001a, 0x2a001b).nopw();
	map(0x2a0000, 0x2a001f).r(FUNC(koftball_state::random_number_r));
	map(0x2b0000, 0x2b0001).portr("DSW");
	map(0x2d8000, 0x2d8001).r(FUNC(koftball_state::random_number_r));
	map(0x2da000, 0x2da003).w("ymsnd", FUNC(ym2413_device::write)).umask16(0xff00);

	map(0x2db001, 0x2db001).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x2db002, 0x2db003).nopr(); // reads here during some scene changes
	map(0x2db003, 0x2db003).w("ramdac", FUNC(ramdac_device::pal_w));
	map(0x2db005, 0x2db005).w("ramdac", FUNC(ramdac_device::mask_w));

	map(0x2dc000, 0x2dc000).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x2f0000, 0x2f0003).portr("INPUTS");
	map(0x300000, 0x300001).nopw();
	map(0x320000, 0x320000).lw8(NAME([this] (u8 data) { m_gfx_ctrl = data; LOGGFX("GFX ctrl $320000 (layer enable) %02x\n", data); }));
	map(0x340000, 0x340001).r(FUNC(koftball_state::prot_r));
	map(0x360000, 0x360001).w(FUNC(koftball_state::prot_w));
}

void koftball_state::jxzh_mem(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x200000, 0x200fff).ram().share("nvram");

	map(0x260000, 0x260fff).ram().w(FUNC(koftball_state::videoram_w<0>)).share(m_videoram[0]);
	map(0x261000, 0x261fff).ram().w(FUNC(koftball_state::videoram_w<1>)).share(m_videoram[1]);
	map(0x262000, 0x262fff).ram().w(FUNC(koftball_state::videoram_w<2>)).share(m_videoram[2]);
	map(0x263000, 0x263fff).ram().w(FUNC(koftball_state::videoram_w<3>)).share(m_videoram[3]);
	map(0x264b00, 0x264dff).ram(); // TODO: writes here at least at girl selection after coin up. Some kind of effect?
	map(0x268000, 0x26ffff).ram();

	map(0x280000, 0x29ffff).ram().share(m_pixram);
	map(0x2a0007, 0x2a0007).w(FUNC(koftball_state::irq_ack_w));
	map(0x2a0009, 0x2a0009).lw8(NAME([this] (u8 data) { m_irq_enable = data; }));
	map(0x2a000f, 0x2a000f).lw8(NAME([this] (u8 data) { m_priority = data; LOGGFX("GFX ctrl $2a000f (priority) %02x\n", data); }));
	map(0x2a0017, 0x2a0017).w(FUNC(koftball_state::pixpal_w));
	map(0x2a0019, 0x2a0019).lw8(NAME([this] (u8 data) { m_backpen = data; LOGGFX("GFX ctrl $2a0019 (backpen) %02x\n", data); }));
	map(0x2a001a, 0x2a001d).nopw();
	map(0x2a0000, 0x2a001f).r(FUNC(koftball_state::random_number_r));
	map(0x2b0000, 0x2b0001).portr("DSW");
	map(0x2da000, 0x2da003).w("ymsnd", FUNC(ym2413_device::write)).umask16(0xff00);

	map(0x2db001, 0x2db001).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x2db002, 0x2db003).nopr(); // reads here during some scene changes
	map(0x2db003, 0x2db003).w("ramdac", FUNC(ramdac_device::pal_w));
	map(0x2db005, 0x2db005).w("ramdac", FUNC(ramdac_device::mask_w));

	map(0x2dc000, 0x2dc000).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x2f0000, 0x2f0001).portr("INPUTS");
	map(0x300000, 0x300001).nopw();
	map(0x320000, 0x320000).lw8(NAME([this] (u8 data) { m_gfx_ctrl = data; LOGGFX("GFX ctrl $320000 (layer enable) %02x\n", data); }));
	map(0x340000, 0x340001).r(FUNC(koftball_state::prot_r));
	map(0x360000, 0x360001).w(FUNC(koftball_state::prot_w));
	map(0x380000, 0x380001).w(FUNC(koftball_state::prot_w));
	map(0x3a0000, 0x3a0001).w(FUNC(koftball_state::prot_w));
}

void koftball_state::kaimenhu_mem(address_map &map)
{
	jxzh_mem(map);

	map(0x340000, 0x340001).r(FUNC(koftball_state::kaimenhu_prot_r));
}

void koftball_state::ramdac_map(address_map &map)
{
	map(0x000, 0x3ff).rw("ramdac", FUNC(ramdac_device::ramdac_pal_r), FUNC(ramdac_device::ramdac_rgb666_w));
}

static INPUT_PORTS_START( koftball )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BILL1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) // also decrease in sound test
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) // also increase in sound test
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("unknown1") PORT_CODE(KEYCODE_A)

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("unknown2") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_SERVICE_NO_TOGGLE( 0x1000, IP_ACTIVE_LOW ) // test mode enter
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("unknown3") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("unknown4") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("unknown5") PORT_CODE(KEYCODE_G)

	PORT_START("DSW")
	PORT_DIPNAME(    0x0001, 0x0001, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(         0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(         0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x0002, 0x0002, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(         0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(         0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x0004, 0x0004, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:!6")
	PORT_DIPSETTING(         0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(         0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x0008, 0x0008, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:!5")
	PORT_DIPSETTING(         0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(         0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x0010, 0x0010, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:!4")
	PORT_DIPSETTING(         0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(         0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x0020, 0x0020, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:!3")
	PORT_DIPSETTING(         0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(         0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x0040, 0x0040, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:!2")
	PORT_DIPSETTING(         0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(         0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x0080, 0x0080, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:!1")
	PORT_DIPSETTING(         0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(         0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x0100, 0x0100, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(         0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(         0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x0200, 0x0200, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(         0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(         0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x0400, 0x0400, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:!6")
	PORT_DIPSETTING(         0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(         0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x0800, 0x0800, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:!5")
	PORT_DIPSETTING(         0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(         0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x1000, 0x1000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:!4")
	PORT_DIPSETTING(         0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(         0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x2000, 0x2000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:!3")
	PORT_DIPSETTING(         0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(         0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x4000, 0x4000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:!2")
	PORT_DIPSETTING(         0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(         0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x8000, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:!1")
	PORT_DIPSETTING(         0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(         0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( jxzh )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_MAHJONG_RON )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("hopper switch") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_SERVICE_NO_TOGGLE( 0x1000, IP_ACTIVE_LOW )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP) PORT_NAME("Change Tile / Double Up")

	PORT_START("DSW")
	PORT_DIPNAME(    0x0001, 0x0001, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(         0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(         0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x0002, 0x0002, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(         0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(         0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x0004, 0x0004, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:!6")
	PORT_DIPSETTING(         0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(         0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x0008, 0x0008, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:!5")
	PORT_DIPSETTING(         0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(         0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x0010, 0x0010, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:!4")
	PORT_DIPSETTING(         0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(         0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x0020, 0x0020, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:!3")
	PORT_DIPSETTING(         0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(         0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x0040, 0x0040, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:!2")
	PORT_DIPSETTING(         0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(         0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x0080, 0x0080, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:!1")
	PORT_DIPSETTING(         0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(         0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x0100, 0x0100, DEF_STR( Service_Mode ) ) PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(         0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(         0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x0200, 0x0200, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(         0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(         0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x0400, 0x0400, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:!6")
	PORT_DIPSETTING(         0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(         0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x0800, 0x0800, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:!5")
	PORT_DIPSETTING(         0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(         0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x1000, 0x1000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:!4")
	PORT_DIPSETTING(         0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(         0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x2000, 0x2000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:!3")
	PORT_DIPSETTING(         0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(         0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x4000, 0x4000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:!2")
	PORT_DIPSETTING(         0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(         0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x8000, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:!1")
	PORT_DIPSETTING(         0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(         0x0000, DEF_STR( On ) )
INPUT_PORTS_END


TIMER_DEVICE_CALLBACK_MEMBER(koftball_state::interrupt)
{
	int const scanline = param;

	if (scanline == 240)
		if (BIT(m_irq_enable, 2))
			m_maincpu->set_input_line(2, ASSERT_LINE);

	if (scanline == 128)
		if (BIT(m_irq_enable, 3))
			m_maincpu->set_input_line(3, ASSERT_LINE);

	if (scanline == 64)
		if (BIT(m_irq_enable, 6))
			m_maincpu->set_input_line(6, ASSERT_LINE);
}

static const gfx_layout tilelayout =
{
	8,8,
	RGN_FRAC(1,2),
	8,
	{0,1,2,3, RGN_FRAC(1,2)+0,RGN_FRAC(1,2)+1,RGN_FRAC(1,2)+2,RGN_FRAC(1,2)+3 },
	{ 0, 4, 8, 12, 16,20,  24, 28 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*32
};

static GFXDECODE_START( gfx_koftball )
	GFXDECODE_ENTRY( "tiles", 0, tilelayout, 0, 1 )
GFXDECODE_END


void koftball_state::koftball(machine_config &config)
{
	M68000(config, m_maincpu, 21.477272_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &koftball_state::koftball_mem);
	TIMER(config, "scantimer").configure_scanline(FUNC(koftball_state::interrupt), "screen", 0, 1);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_screen_update(FUNC(koftball_state::screen_update));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 64*8-1, 0*8, 29*8-1);
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_entries(256);
	ramdac_device &ramdac(RAMDAC(config, "ramdac", 0, m_palette));
	ramdac.set_addrmap(0, &koftball_state::ramdac_map);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_koftball);

	SPEAKER(config, "speaker", 2).front();

	ym2413_device &ymsnd(YM2413(config, "ymsnd", 3.579545_MHz_XTAL));
	ymsnd.add_route(ALL_OUTPUTS, "speaker", 0.50, 0);
	ymsnd.add_route(ALL_OUTPUTS, "speaker", 0.50, 1);

	okim6295_device &oki(OKIM6295(config, "oki", 21.477272_MHz_XTAL / 22, okim6295_device::PIN7_HIGH)); // clock frequency & pin 7 verified for jxzh
	oki.add_route(ALL_OUTPUTS, "speaker", 0.50, 0);
	oki.add_route(ALL_OUTPUTS, "speaker", 0.50, 1);
}

void koftball_state::jxzh(machine_config &config)
{
	koftball(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &koftball_state::jxzh_mem);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
}

void koftball_state::kaimenhu(machine_config &config)
{
	jxzh(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &koftball_state::kaimenhu_mem);
}


ROM_START( koftball )
	ROM_REGION( 0x20000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "ft5_v16_c6.u15", 0x00000, 0x10000, CRC(5e1784a5) SHA1(5690d315500fb533b12b598cb0a51bd1eadd0505) )
	ROM_LOAD16_BYTE( "ft5_v16_c5.u14", 0x00001, 0x10000, CRC(45c856e3) SHA1(0a25cfc2b09f1bf996f9149ee2a7d0a7e51794b7) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD16_BYTE( "ft5_v6_c3.u61", 0x00000, 0x20000, CRC(f3f747f3) SHA1(6e376d42099733e52779c089303391eeddf4fa87) )
	ROM_LOAD16_BYTE( "ft5_v6_c4.u58", 0x00001, 0x20000, CRC(8b774574) SHA1(a79c1cf90d7b5ef0aba17770700b2fe18846f7b7) )
	ROM_LOAD16_BYTE( "ft5_v6_c1.u59", 0x40000, 0x20000, CRC(b33a008f) SHA1(c4fd40883fa1c1cbc58f7b342fed753c52f0cf59) )
	ROM_LOAD16_BYTE( "ft5_v6_c2.u60", 0x40001, 0x20000, CRC(3dc22223) SHA1(dc74800c51de3b6a7fbf7214a1da1d2f3d2aea84) )

	ROM_REGION( 0x40000, "oki", 0 ) // Samples
	ROM_LOAD( "ft5_v6_c9.u21", 0x00000, 0x10000, CRC(f6216740) SHA1(3d1c795da2f8093e937107e3848cb96338536faf) )
ROM_END

/*******************************************************************
錦繡中華 (Jǐnxiù Zhōnghuá), BMC 1996
Hardware Info by Guru
---------------------

BMC-A-41210
|-------------------------------------------------|
|21.47727MHz  9  AR17961              uPC1242H   1|
|                        4   3   2   1           0|
|T518A      6264                           7805  W|
|  555                 |-----|              VOL  A|
|           6264       |40817|     HM86171       Y|
|                      |-----|                  |-|
|           62256                               |
|                      |-----|             LM324|-|
|           62256      |PLCC |   3567             |
|                      |68   |                   M|
| |-|             GAL1 |-----|    3.579545MHz    A|
| |6|   5   6116  GAL2                           H|
| |8|                                            J|
| |0|           SCAP                  DIP1       O|
| |0|                                            N|
| |0|   6   6116                                 G|
| |-|                                 DIP2        |
|-------------------------------------------------|
Notes:
      68000 - Clock 10.738635MHz [21.47727/2]
    AR17961 - Equivalent to OKI M6295 4-Channel ADPCM Voice Synthesis LSI. Clock input 0.976239545MHz [21.47727/22]. Pin 7 HIGH
       3567 - Equivalent to Yamaha YM2413 OPLL FM Synthesis Sound Chip. Clock input 3.579545MHz
      61256 - 32kB x8-bit SRAM
       6264 - 8kB x8-bit SRAM
       6116 - 2kB x8-bit SRAM (both backed-up by 5.5V Supercap)
     DIP1/2 - 8-position DIP Switch
     GAL1/2 - LATTICE GAL16V8B PLD
   uPC1242H - NEC uPC1242H Audio Power Amp
      LM324 - LM324 Quad Operational Amplifier
        555 - 555 Timer (provides master reset)
       7805 - LM7805 5V Linear Regulator
    HM86171 - HMC HM86171-80 Color Palette RAMDAC
      40817 - BMC QFP100 Custom Chip marked 'BMC VDB40817'
     PLCC68 - Unknown PLCC68 IC. Surface scratched but likely to be BMC B816140 (CPLD)
      T518A - Mitsumi PST518A Master Reset IC (TO92)
       SCAP - 5.5V 0.1F Supercap
        5/6 - 27C1001 EPROM (main program)
    1/2/3/4 - 23C4000 mask ROM (gfx)
          9 - 23C2000 mask ROM (OKI samples)

*******************************************************************/

ROM_START( jxzh )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "bmc_m6k.u15", 0x00000, 0x20000, CRC(410ee342) SHA1(2b83e0fc2c5f9a2d745755572eba751bfac107f5) )
	ROM_LOAD16_BYTE( "bmc_m5k.u14", 0x00001, 0x20000, CRC(43b67d0a) SHA1(f421c71165d79881c208d332416f1c82057f5680) )

	ROM_REGION( 0x200000, "tiles", 0 )
	ROM_LOAD16_BYTE( "bmc_mj9601-3.u61", 0x000000, 0x80000, CRC(b0c66e6f) SHA1(7539178d3bd4c012f0dd2f642e5a02303779109d) )
	ROM_LOAD16_BYTE( "bmc_mj9601-4.u58", 0x000001, 0x80000, CRC(04a307f1) SHA1(8a45de790305c3cc4285a91d19b95d696d31bd11) )
	ROM_LOAD16_BYTE( "bmc_mj9601-1.u59", 0x100000, 0x80000, CRC(184b8ba8) SHA1(0b84b9540ff72a57982a8f9e107a6d8d9314fdd1) )
	ROM_LOAD16_BYTE( "bmc_mj9601-2.u60", 0x100001, 0x80000, CRC(f82e0f34) SHA1(4051c7b24f865cf7fb77eb89dde79cb30bdba7a0) )

	ROM_REGION( 0x40000, "oki", 0 ) // Samples
	ROM_LOAD( "bmc_mj9601-9.u21", 0x00000, 0x40000, CRC(0ffcae13) SHA1(f8501c7c8a8bebf5da95aa3b275dd514f1014971) )
ROM_END

// 開門胡 (Kāi Mén Hú) - a program swap of jxzh (or vice versa)
// same PCB as jxzh, but with a File KB89C67 in place of the 3567 (both are YM2413 clones) and a Music TR9C1710-80PCA RAMDAC instead of the HM86171-80
ROM_START( kaimenhu )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "bmc_kaimenhu_v02_5.u15", 0x00000, 0x20000, CRC(a3a017d0) SHA1(fa2194ded96721e0e2d1301bff7adfef37824fec) ) // kaimenhu is actually written 開門胡
	ROM_LOAD16_BYTE( "bmc_kaimenhu_v02_5.u14", 0x00001, 0x20000, CRC(29ef1fc1) SHA1(ec054f8fb6100f95f6120d409bdd5d9d0b8b21ee) ) // kaimenhu is actually written 開門胡

	ROM_REGION( 0x200000, "tiles", 0 ) // exactly the same as jxzh
	ROM_LOAD16_BYTE( "bmc_mj9601-3.u61", 0x000000, 0x80000, CRC(b0c66e6f) SHA1(7539178d3bd4c012f0dd2f642e5a02303779109d) )
	ROM_LOAD16_BYTE( "bmc_mj9601-4.u58", 0x000001, 0x80000, CRC(04a307f1) SHA1(8a45de790305c3cc4285a91d19b95d696d31bd11) )
	ROM_LOAD16_BYTE( "bmc_mj9601-1.u59", 0x100000, 0x80000, CRC(184b8ba8) SHA1(0b84b9540ff72a57982a8f9e107a6d8d9314fdd1) )
	ROM_LOAD16_BYTE( "bmc_mj9601-2.u60", 0x100001, 0x80000, CRC(f82e0f34) SHA1(4051c7b24f865cf7fb77eb89dde79cb30bdba7a0) )

	ROM_REGION( 0x40000, "oki", 0 ) // Samples, exactly the same as jxzh
	ROM_LOAD( "bmc_mj9601-9.u21", 0x00000, 0x40000, CRC(0ffcae13) SHA1(f8501c7c8a8bebf5da95aa3b275dd514f1014971) )
ROM_END

#if NVRAM_HACK

static const u16 nvram[]=
{
	0x0000,0x5555,0x0000,0x5555,0x0000,0x5555,0x0000,0x5555,
	0x0000,0x5555,0x0000,0x0000,0x0000,0x0000,0x5555,0x5555,
	0x0000,0x0000,0x5555,0x5555,0x0000,0x0000,0x5555,0x5555,
	0x0000,0x0000,0x5555,0x5555,0x0000,0x0000,0x5555,0x5555,
	0x0000,0x0000,0x5555,0x5555,0x0000,0x0000,0x5555,0x5555,
	0x0000,0x0000,0x5555,0x5555,0x0000,0x0000,0x5555,0x5555,
	0x0000,0x0000,0x5555,0x5555,0x0000,0x0000,0x5555,0x5555,
	0x0000,0x0000,0x5555,0x5555,0x0000,0x0000,0x5555,0x5555,
	0x0000,0x0000,0x5555,0x5555,0x0000,0x0000,0x5555,0x5555,
	0x0000,0x0000,0x5555,0x5555,0x0000,0x0000,0x5555,0x5555,
	0x0000,0x0000,0x5555,0x5555,0x0000,0x0000,0x5555,0x5555,
	0x0000,0x0000,0x5555,0x5555,0x0000,0x0000,0x5555,0x5555,
	0x0000,0x0000,0x5555,0x5555,0x0000,0x0000,0x5555,0x5555,
	0x0000,0x0000,0x5555,0x5555,0x0000,0x0000,0x5555,0x5555,
	0x0000,0x0000,0x5555,0x5555,0x0000,0x0000,0x0467,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0xffff
};

#endif
void koftball_state::init_koftball()
{
#if NVRAM_HACK
	{
		int offset = 0;
		while (nvram[offset] != 0xffff)
		{
			m_main_ram[offset] = nvram[offset];
			++offset;
		}
	}
#endif
}

} // anonymous namespace


GAME( 1995, koftball, 0,    koftball, koftball, koftball_state, init_koftball, ROT0, "BMC", "King of Football",  MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1996, jxzh,     0,    jxzh,     jxzh,     koftball_state, empty_init,    ROT0, "BMC", "Jinxiu Zhonghua",   MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1996, kaimenhu, jxzh, kaimenhu, jxzh,     koftball_state, empty_init,    ROT0, "BMC", "Kai Men Hu",        MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
