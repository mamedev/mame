// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari, Nicola Salmoria, Luca Elia
/****************************************************************************

Royal Mahjong (c) 1981 Nichibutsu
and many other Dyna/Dynax games running in similar bare-bones hardware

driver by Zsolt Vasvari, Nicola Salmoria, Luca Elia

CPU:    Z80 or TLCS-90
Video:  Framebuffer
Sound:  AY-3-8910 (or YM2149)
OSC:    18.432MHz and 8MHz

-----------------------------------------------------------------------------------------------------------------------
Year + Game               Board(s)               CPU      Company            Notes
-----------------------------------------------------------------------------------------------------------------------
81  Royal Mahjong                                Z80      Nichibutsu
81? Open Mahjong                                 Z80      Sapporo Mechanic
82  Royal Mahjong         ? + FRM-03             Z80      Falcon             bootleg
83  Janyou Part II                               Z80      Cosmo Denshi
84? Jan Oh                FRM-00?                Z80      Toaplan            Incomplete program ROMs
84? Challenge Girl        FRM-03 + SK-1B         Z80      Paradise Denshi    The dumped set is a bootleg by Falcon
86  Ippatsu Gyakuten                             Z80      Public/Paradais
86  Don Den Mahjong       D039198L-0             Z80      Dyna Electronics
86  Jong Shin             D8702158L1-0           Z80      Dyna Electronics
86  Watashiha Suzumechan  D8803288L1-0           Z80      Dyna Electronics
86  Mahjong Shiyou (BET)  S-0086-001-00          Z80      Visco              Extra Z80 + MSM5205
86  Mahjong Yarou         FRM-00 (modified)      Z80      Visco
86  Mahjong Senka         FRM-00?                Z80      Visco
87  Mahjong Diplomat      D0706088L1-0           Z80      Dynax
87  Mahjong Studio 101    D1708228L1             Z80      Dynax
87  Tonton                D0908288L1-0           Z80      Dynax
88  Almond Pinky          D1401128L-0 + RM-1D    Z80      Dynax
89  Mahjong Shinkirou     D210301BL2 + FRM-00?   TLCS-90  Dynax
89  Mahjong Derringer     D2203018L              Z80      Dynax              Larger palette
90  Mahjong If..?         D2909278L              TLCS-90  Dynax              Larger palette
91  Mahjong Vegas         D5011308L1 + FRM-00    TLCS-90  Dynax              Larger palette, RTC
92  Mahjong Cafe Time     D6310128L1-1           TLCS-90  Dynax              Larger palette, RTC
93  Mahjong Cafe Doll     D76052208L-2           TLCS-90  Dynax              Larger palette, RTC
93  Ichi Ban Jian         MJ911                  Z80      Excel              Larger palette, additional YM2413
95  Mahjong Tensinhai     D10010318L1            TLCS-90  Dynax              Larger palette, RTC
96  Mj Raijinhai (DX)     D10010318L1  D10502168 TLCS-90  Dynax              Larger palette, RTC
96  Janputer '96          NS503X0727             Z80      Dynax              Larger palette, RTC
97  Pong Boo! 2           NEW PONG-BOO           Z80(?)   OCT                OKI M6295, no PROMs
97  Janputer Special      CS166P008 + NS5110207  Z80      Dynax              Larger palette, RTC
99  Mahjong Cafe Break    NS528-9812             TLCS-90  Nakanihon / Dynax  Larger palette, RTC
99  Mahjong Cafe Paradise ? + TSS001-0001        TLCS-90  Techno-Top         Larger palette, RTC
-----------------------------------------------------------------------------------------------------------------------

TODO:

- DIP switches and inputs in dondenmj, suzume, mjderngr...

- there's something fishy with the bank switching in tontonb/mjdiplob

- majs101b: service mode doesn't work

- mjtensin: random crashes, interrupts related

Stephh's notes (based on the games Z80 code and some tests) :

1) 'royalmah'

  - COIN1 doesn't work correctly, the screen goes black instead of showing the
    credits, and you can start a game but the "phantom" credit is not subtracted;
    with NVRAM support, this means the game would always boot to a black screen.
  - The doesn't seem to be any possibility to play a 2 players game
    (but the inputs are mapped so you can test them in the "test mode").
    P1 IN4 doesn't seem to be needed outside the "test mode" either.

2) 'tontonb'

  - The doesn't seem to be any possibility to play a 2 players game
    (but the inputs are mapped so you can test them in the "test mode")
    P1 IN4 doesn't seem to be needed outside the "test mode" either.

  - I've DELIBERATELY mapped DSW3 before DSW2 to try to spot the common
    things with the other Dynax mahjong games ! Please don't change this !

  - When "Special Combinations" DIP Switch is ON, there is a marker in
    front of a random combination. It's value is *2 then.

3) 'mjdiplob'

  - The doesn't seem to be any possibility to play a 2 players game
    (but the inputs are mapped so you can test them in the "test mode")
    P1 IN4 doesn't seem to be needed outside the "test mode" either.

  - When "Special Combinations" DIP Switch is ON, there is a marker in
    front of a random combination. It's value remains *1 though.
    Could it be a leftover from another game ('tontonb' for exemple) ?

- cafepara, janptr96, janptrsp: in service mode press in sequence N,Ron,Ron,N to
  access some hidden options. (thanks bnathan)

- cafebrk and cafepara share the same internal TMP91640 code, while
  cafedoll, mjvegas and ougonhai (dynax.cpp) share the same internal TMP90840
  code. Curiously, cafetime has the same internal TMP90840 code as cafedoll
  and mjvegas, but it's configured to run in external ROM mode.

****************************************************************************/

#include "emu.h"

#include "cpu/tlcs90/tlcs90.h"
#include "cpu/z80/tmpz84c015.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/msm6242.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "sound/msm5205.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class royalmah_state : public driver_device
{
public:
	royalmah_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette"),
		m_ay(*this, "aysnd"),
		m_videoram(*this, "videoram"),
		m_audiocpu(*this, "audiocpu"),
		m_rtc(*this, "rtc"),
		m_soundlatch(*this, "soundlatch"),
		m_rambank(*this, "rambank"),
		m_mainview(*this, "mainview"),
		m_key_row(*this, "KEY%u", 0U),
		m_dsw(*this, "DSW%u", 1U),
		m_dsw_top(*this, "DSWTOP"),
		m_gfx_rom(*this, "gfx1")
	{ }

	void royalmah(machine_config &config) ATTR_COLD;
	void janoh(machine_config &config) ATTR_COLD;
	void janoha(machine_config &config) ATTR_COLD;
	void jansou(machine_config &config) ATTR_COLD;
	void ippatsu(machine_config &config) ATTR_COLD;
	void janyoup2(machine_config &config) ATTR_COLD;
	void seljan(machine_config &config) ATTR_COLD;

	void init_jansou() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

	uint8_t player_1_port_r();
	uint8_t player_2_port_r();
	void input_port_select_w(uint8_t data);

	void royalmah_palbank_w(uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void royalmah_map(address_map &map) ATTR_COLD;
	void royalmah_iomap(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	optional_device<ay8910_device> m_ay;
	required_shared_ptr<uint8_t> m_videoram;
	optional_device<cpu_device> m_audiocpu;
	optional_device<msm6242_device> m_rtc;
	optional_device<generic_latch_8_device> m_soundlatch;
	optional_memory_bank m_rambank;
	memory_view m_mainview;
	required_ioport_array<10> m_key_row;
	optional_ioport_array<4> m_dsw;
	optional_ioport m_dsw_top;
	optional_region_ptr<uint8_t> m_gfx_rom;

	// used by most games
	uint8_t m_dsw_select = 0;
	uint8_t m_palette_base = 0;
	uint8_t m_flip_screen = 0;

private:
	void jansou_dsw_sel_w(uint8_t data);
	uint8_t jansou_dsw_r();
	void jansou_colortable_w(offs_t offset, uint8_t data);
	void jansou_6400_w(uint8_t data);
	void jansou_6401_w(uint8_t data);
	void jansou_6402_w(uint8_t data);
	uint8_t jansou_6403_r();
	uint8_t jansou_6404_r();
	uint8_t jansou_6405_r();
	void jansou_sound_w(uint8_t data);

	void royalmah_palette(palette_device &palette) const ATTR_COLD;

	void ippatsu_map(address_map &map) ATTR_COLD;
	void seljan_map(address_map &map) ATTR_COLD;

	void ippatsu_iomap(address_map &map) ATTR_COLD;
	void janyoup2_iomap(address_map &map) ATTR_COLD;
	void seljan_iomap(address_map &map) ATTR_COLD;

	void janoh_map(address_map &map) ATTR_COLD;
	void janoh_sub_map(address_map &map) ATTR_COLD;
	void janoh_sub_iomap(address_map &map) ATTR_COLD;

	void jansou_map(address_map &map) ATTR_COLD;
	void jansou_sub_iomap(address_map &map) ATTR_COLD;
	void jansou_sub_map(address_map &map) ATTR_COLD;

	// used by most games
	uint8_t m_input_port_select = 0;

	// game-specific
	uint8_t m_gfx_adr_l = 0;
	uint8_t m_gfx_adr_m = 0;
	uint8_t m_gfx_adr_h = 0;
	uint32_t m_gfx_adr = 0;
	uint8_t m_gfxdata0 = 0;
	uint8_t m_gfxdata1 = 0;
	uint8_t m_jansou_colortable[16] = { };
};


class royalmah_prgbank_state : public royalmah_state
{
public:
	royalmah_prgbank_state(const machine_config &mconfig, device_type type, const char *tag) :
		royalmah_state(mconfig, type, tag),
		m_mainbank(*this, "mainbank"),
		m_mainopbank(*this, "mainopbank")
	{
	}

	void dondenmj(machine_config &config) ATTR_COLD;
	void tahjong(machine_config &config) ATTR_COLD;
	void makaijan(machine_config &config) ATTR_COLD;
	void daisyari(machine_config &config) ATTR_COLD;
	void mjclub(machine_config &config) ATTR_COLD;
	void chalgirl(machine_config &config) ATTR_COLD;
	void rkjanoh2(machine_config &config) ATTR_COLD;
	void mjyarou(machine_config &config) ATTR_COLD;
	void mjsenka(machine_config &config) ATTR_COLD;
	void mjsiyoub(machine_config &config) ATTR_COLD;
	void suzume(machine_config &config) ATTR_COLD;
	void jongshin(machine_config &config) ATTR_COLD;
	void tontonb(machine_config &config) ATTR_COLD;
	void mjdiplob(machine_config &config) ATTR_COLD;
	void majs101b(machine_config &config) ATTR_COLD;
	void mjapinky(machine_config &config) ATTR_COLD;
	void mjderngr(machine_config &config) ATTR_COLD;
	void janptr96(machine_config &config) ATTR_COLD;
	void mjifb(machine_config &config) ATTR_COLD;
	void mjdejavu(machine_config &config) ATTR_COLD;
	void mjtensin(machine_config &config) ATTR_COLD;
	void majrjh(machine_config &config) ATTR_COLD;
	void cafedoll(machine_config &config) ATTR_COLD;
	void cafepara(machine_config &config) ATTR_COLD;
	void cafetime(machine_config &config) ATTR_COLD;
	void mjvegas(machine_config &config) ATTR_COLD;
	void mjvegasa(machine_config &config) ATTR_COLD;
	void ichiban(machine_config &config) ATTR_COLD;
	void pongboo2(machine_config &config) ATTR_COLD;

	void init_tahjong() ATTR_COLD;
	void init_dynax() ATTR_COLD;
	void init_suzume() ATTR_COLD;
	void init_daisyari() ATTR_COLD;
	void init_mjtensin() ATTR_COLD;
	void init_cafedoll() ATTR_COLD;
	void init_cafepara() ATTR_COLD;
	void init_cafetime() ATTR_COLD;
	void init_mjvegas() ATTR_COLD;
	void init_mjvegasa() ATTR_COLD;
	void init_jongshin() ATTR_COLD;
	void init_mjifb() ATTR_COLD;
	void init_tontonb() ATTR_COLD;
	void init_mjsenka() ATTR_COLD;
	void init_mjsiyoub() ATTR_COLD;
	void init_janptr96() ATTR_COLD;
	void init_chalgirl() ATTR_COLD;
	void init_ichiban() ATTR_COLD;
	void init_pongboo2() ATTR_COLD;

private:
	void tahjong_bank_w(uint8_t data);

	void mjderngr_coin_w(uint8_t data);
	void mjderngr_palbank_w(uint8_t data);

	uint8_t majs101b_dsw_r();

	void chalgirl_bank_w(uint8_t data);

	void mjyarou_bank_w(offs_t offset, uint8_t data);

	uint8_t suzume_dsw_r();
	void suzume_bank_w(uint8_t data);

	void jongshin_bank_w(uint8_t data);

	void mjapinky_bank_w(uint8_t data);
	void mjapinky_palbank_w(uint8_t data);
	uint8_t mjapinky_dsw_r();

	void tontonb_bank_w(uint8_t data);

	void dynax_bank_w(uint8_t data);

	uint8_t daisyari_dsw_r();
	void daisyari_bank_w(uint8_t data);

	uint8_t mjclub_dsw_r();
	void mjclub_bank_w(uint8_t data);

	void janptr96_dswsel_w(uint8_t data);
	uint8_t janptr96_dsw_r();
	void janptr96_rombank_w(uint8_t data);
	void janptr96_rambank_w(uint8_t data);
	uint8_t janptr96_unknown_r();
	void janptr96_coin_counter_w(uint8_t data);

	void mjifb_coin_counter_w(uint8_t data);
	uint8_t mjifb_p8_r();
	void mjifb_p3_w(uint8_t data);
	void mjifb_p4_w(uint8_t data);
	void mjifb_p8_w(uint8_t data);

	uint8_t mjtensin_p3_r();
	void mjtensin_p4_w(uint8_t data);
	void mjtensin_6ff3_w(uint8_t data);

	void cafetime_p4_w(uint8_t data);
	void cafetime_p3_w(uint8_t data);
	void cafetime_dsw_w(uint8_t data);
	uint8_t cafetime_dsw_r();
	uint8_t cafetime_7fe4_r();
	void cafetime_7fe3_w(uint8_t data);

	void cafedoll_p6_w(uint8_t data);
	void cafedoll_p7_w(uint8_t data);

	void mjvegasa_p4_w(uint8_t data);
	void mjvegasa_p3_w(uint8_t data);
	void mjvegasa_rombank_w(uint8_t data);
	uint8_t mjvegasa_rom_io_r(offs_t offset);
	void mjvegasa_rom_io_w(offs_t offset, uint8_t data);
	void mjvegasa_coin_counter_w(uint8_t data);
	void mjvegasa_12400_w(uint8_t data);
	uint8_t mjvegasa_12500_r();

	uint8_t mjvegas_p5_r();
	void mjvegas_p6_w(uint8_t data);
	void mjvegas_p7_w(uint8_t data);

	void mjderngr_palette(palette_device &palette) const ATTR_COLD;

	INTERRUPT_GEN_MEMBER(suzume_irq);

	void royalmah_banked_map(address_map &map) ATTR_COLD;
	void mjapinky_map(address_map &map) ATTR_COLD;
	void tahjong_map(address_map &map) ATTR_COLD;
	void chalgirl_map(address_map &map) ATTR_COLD;
	void mjsiyoub_map(address_map &map) ATTR_COLD;
	void ichiban_map(address_map &map) ATTR_COLD;
	void ichiban_opcodes_map(address_map &map) ATTR_COLD;
	void pongboo2_map(address_map &map) ATTR_COLD;
	void pongboo2_opcodes_map(address_map &map) ATTR_COLD;
	void mjsenka_opcodes_map(address_map &map) ATTR_COLD;
	void janptr96_map(address_map &map) ATTR_COLD;
	void janptr96_iomap(address_map &map) ATTR_COLD;
	void mjifb_map(address_map &map) ATTR_COLD;
	void mjdejavu_map(address_map &map) ATTR_COLD;
	void mjtensin_map(address_map &map) ATTR_COLD;
	void majrjh_map(address_map &map) ATTR_COLD;
	void mjvegasa_map(address_map &map) ATTR_COLD;
	void cafepara_map(address_map &map) ATTR_COLD;
	void cafetime_map(address_map &map) ATTR_COLD;

	void chalgirl_iomap(address_map &map) ATTR_COLD;
	void tahjong_iomap(address_map &map) ATTR_COLD;
	void suzume_iomap(address_map &map) ATTR_COLD;
	void jongshin_iomap(address_map &map) ATTR_COLD;
	void rkjanoh2_iomap(address_map &map) ATTR_COLD;
	void mjyarou_iomap(address_map &map) ATTR_COLD;
	void mjsiyoub_iomap(address_map &map) ATTR_COLD;
	void ichiban_iomap(address_map &map) ATTR_COLD;
	void pongboo2_iomap(address_map &map) ATTR_COLD;
	void dondenmj_iomap(address_map &map) ATTR_COLD;
	void makaijan_iomap(address_map &map) ATTR_COLD;
	void daisyari_iomap(address_map &map) ATTR_COLD;
	void mjclub_iomap(address_map &map) ATTR_COLD;
	void mjdiplob_iomap(address_map &map) ATTR_COLD;
	void tontonb_iomap(address_map &map) ATTR_COLD;
	void majs101b_iomap(address_map &map) ATTR_COLD;
	void mjderngr_iomap(address_map &map) ATTR_COLD;
	void mjapinky_iomap(address_map &map) ATTR_COLD;

	void mjsiyoub_audio_prg_map(address_map &map) ATTR_COLD;

	required_memory_bank m_mainbank;
	optional_memory_bank m_mainopbank;

	// used by most games
	uint8_t m_rombank = 0;

	// game-specific
	std::unique_ptr<uint8_t[]> m_janptr96_nvram;

	uint8_t m_suzume_bank = 0;
	uint8_t m_mjyarou_bank = 0;
	uint8_t m_mjvegas_p5_val = 0;
};


void royalmah_state::machine_start()
{
	save_item(NAME(m_input_port_select));
	save_item(NAME(m_dsw_select));
	save_item(NAME(m_palette_base));
	save_item(NAME(m_flip_screen));
}

void royalmah_state::royalmah_palette(palette_device &palette) const
{
	uint8_t const *const prom = memregion("proms")->base();

	offs_t const len(memregion("proms")->bytes());
	for (offs_t i = 0; i < len; i++)
	{
		uint8_t bit0, bit1, bit2;

		uint8_t const data = prom[i];

		// red component
		bit0 = BIT(data, 0);
		bit1 = BIT(data, 1);
		bit2 = BIT(data, 2);
		uint8_t const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// green component
		bit0 = BIT(data, 3);
		bit1 = BIT(data, 4);
		bit2 = BIT(data, 5);
		uint8_t const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// blue component
		bit0 = 0;
		bit1 = BIT(data, 6);
		bit2 = BIT(data, 7);
		uint8_t const b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, r, g, b);
	}
}

void royalmah_prgbank_state::mjderngr_palette(palette_device &palette) const
{
	uint8_t const *const prom = memregion("proms")->base();

	offs_t const len(memregion("proms")->bytes());
	for (offs_t i = 0; i < (len / 2); i++)
	{
		uint16_t const data = (prom[i] << 8) | prom[i + 0x200];

		// the bits are in reverse order
		uint8_t const r = bitswap<5>((data >>  0) & 0x1f, 0, 1, 2, 3, 4);
		uint8_t const g = bitswap<5>((data >>  5) & 0x1f, 0, 1, 2, 3, 4);
		uint8_t const b = bitswap<5>((data >> 10) & 0x1f, 0, 1, 2, 3, 4);

		palette.set_pen_color(i, pal5bit(r), pal5bit(g), pal5bit(b));
	}
}

void royalmah_state::royalmah_palbank_w(uint8_t data)
{
	// bit 1 = coin counter
	machine().bookkeeping().coin_counter_w(0, data & 2);

	// bit 2 = flip screen
	m_flip_screen = (data & 4) >> 2;

	// bit 3 = palette bank
	m_palette_base = (data >> 3) & 0x01;
}


void royalmah_prgbank_state::mjderngr_coin_w(uint8_t data)
{
	// bit 1 = coin counter
	machine().bookkeeping().coin_counter_w(0, data & 2);

	// bit 2 = flip screen
	m_flip_screen = (data & 4) >> 2;
}


void royalmah_prgbank_state::mjderngr_palbank_w(uint8_t data)
{
	m_palette_base = data;
}


uint32_t royalmah_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (offs_t offs = 0; offs < 0x4000; offs++)
	{
		uint8_t data1 = m_videoram[offs + 0x0000];
		uint8_t data2 = m_videoram[offs + 0x4000];

		uint8_t y = (m_flip_screen) ? 255 - (offs >> 6) : (offs >> 6);
		uint8_t x = (m_flip_screen) ? 255 - (offs << 2) : (offs << 2);

		for (int i = 0; i < 4; i++)
		{
			uint8_t pen = ((data2 >> 1) & 0x08) | ((data2 << 2) & 0x04) | ((data1 >> 3) & 0x02) | ((data1 >> 0) & 0x01);

			bitmap.pix(y, x) = m_palette->pen((m_palette_base << 4) | pen);

			x = (m_flip_screen) ? x - 1 : x + 1;
			data1 = data1 >> 1;
			data2 = data2 >> 1;
		}
	}

	return 0;
}

void royalmah_state::input_port_select_w(uint8_t data)
{
	m_input_port_select = data;
}

uint8_t royalmah_state::player_1_port_r()
{
	int ret = (m_key_row[0]->read() & 0xc0) | 0x3f;

	if ((m_input_port_select & 0x01) == 0)  ret &= m_key_row[0]->read();
	if ((m_input_port_select & 0x02) == 0)  ret &= m_key_row[1]->read();
	if ((m_input_port_select & 0x04) == 0)  ret &= m_key_row[2]->read();
	if ((m_input_port_select & 0x08) == 0)  ret &= m_key_row[3]->read();
	if ((m_input_port_select & 0x10) == 0)  ret &= m_key_row[4]->read();

	return ret;
}

uint8_t royalmah_state::player_2_port_r()
{
	int ret = (m_key_row[5]->read() & 0xc0) | 0x3f;

	if ((m_input_port_select & 0x01) == 0)  ret &= m_key_row[5]->read();
	if ((m_input_port_select & 0x02) == 0)  ret &= m_key_row[6]->read();
	if ((m_input_port_select & 0x04) == 0)  ret &= m_key_row[7]->read();
	if ((m_input_port_select & 0x08) == 0)  ret &= m_key_row[8]->read();
	if ((m_input_port_select & 0x10) == 0)  ret &= m_key_row[9]->read();

	return ret;
}



uint8_t royalmah_prgbank_state::majs101b_dsw_r()
{
	switch (m_dsw_select)
	{
		case 0x00: return m_dsw[2]->read();
		case 0x20: return m_dsw[3]->read();
		case 0x40: return m_dsw[1]->read();
	}
	return 0;
}



uint8_t royalmah_prgbank_state::suzume_dsw_r()
{
	if (m_suzume_bank & 0x40)
	{
		return m_suzume_bank;
	}
	else
	{
		switch (m_suzume_bank)
		{
			case 0x08: return m_dsw[3]->read();
			case 0x10: return m_dsw[2]->read();
			case 0x18: return m_dsw[1]->read();
		}
		return 0;
	}
}

void royalmah_prgbank_state::tahjong_bank_w(uint8_t data)
{
	m_mainbank->set_entry(data & 0x01);
}

void royalmah_prgbank_state::chalgirl_bank_w(uint8_t data) // TODO: verify behaviour by finishing the game
{
	logerror("chalgirl_bank_w: %02x\n", data);

	// bit 7: only set at start up, then always 0?
	// bit 6: always set?
	// bit 5, 4: always 0?
	// bit 3: always set?
	// bit 2, 1, 0: ROM bank?

	m_mainbank->set_entry(data & 0x07);
}

void royalmah_prgbank_state::mjyarou_bank_w(offs_t offset, uint8_t data) // TODO: taken from jongkyo.cpp, verify
{
	uint8_t mask = 1 << (offset >> 1);

	m_mjyarou_bank &= ~mask;

	if (offset & 1)
		m_mjyarou_bank |= mask;

	m_mainbank->set_entry(m_mjyarou_bank);

	if (m_mainopbank)
		m_mainopbank->set_entry(m_mjyarou_bank);
}

void royalmah_prgbank_state::suzume_bank_w(uint8_t data)
{
	m_suzume_bank = data;

	logerror("%04x: bank %02x\n", m_maincpu->pc(), data);

	// bits 6, 4 and 3 used for something input related?

	m_mainbank->set_entry(data & 0x07);
}

void royalmah_prgbank_state::jongshin_bank_w(uint8_t data)
{
	logerror("%04x: bank %02x\n", m_maincpu->pc(), data);

	m_mainbank->set_entry((data & 0x07) >> 1);
}

void royalmah_prgbank_state::mjapinky_bank_w(uint8_t data)
{
	m_mainbank->set_entry(data & 0x0f);
}

void royalmah_prgbank_state::mjapinky_palbank_w(uint8_t data)
{
	m_flip_screen = (data & 4) >> 2;
	m_palette_base = (data >> 3) & 0x01;
	machine().bookkeeping().coin_counter_w(0, data & 2);  // in
	machine().bookkeeping().coin_counter_w(1, data & 1);  // out
}

uint8_t royalmah_prgbank_state::mjapinky_dsw_r()
{
	if (m_mainbank->entry() == 0x0e)
		return m_dsw[2]->read();
	else
	{
		uint8_t *ptr = (uint8_t*)(m_mainbank->base());
		return ptr[0];
	}
}

void royalmah_prgbank_state::tontonb_bank_w(uint8_t data)
{
	logerror("%04x: bank %02x\n", m_maincpu->pc(), data);

	if (data == 0) return;  // tontonb fix?

	m_mainbank->set_entry(data & 0x0f);
}


// bits 5 and 6 seem to affect which DIP switch to read in 'majs101b'
void royalmah_prgbank_state::dynax_bank_w(uint8_t data)
{
//logerror("%04x: bank %02x\n", m_maincpu->pc(), data);

	m_dsw_select = data & 0x60;

	m_mainbank->set_entry(data & 0x1f);
}

uint8_t royalmah_prgbank_state::daisyari_dsw_r()
{
	switch (m_dsw_select)
	{
		case 0x00: return m_dsw[3]->read();
		case 0x04: return m_dsw[0]->read();
		case 0x08: return m_dsw[1]->read();
		case 0x0c: return m_dsw[2]->read();
	}

	return 0;
}

void royalmah_prgbank_state::daisyari_bank_w(uint8_t data)
{
	m_dsw_select = (data & 0xc);

	//  logerror ("daisyari_bank_w: %08x %02x\n", address, data);

	m_mainbank->set_entry(((data & 0x30) >> 3) | (data & 0x01));

	// bit 1 used too but unknown purpose.
}

uint8_t royalmah_prgbank_state::mjclub_dsw_r()
{
	switch (m_dsw_select)
	{
//      case 0x00: return m_dsw[3]->read();
		case 0x40: return m_dsw[1]->read();
		case 0x80: return m_dsw[2]->read();
		case 0xc0: return m_dsw[3]->read();
	}

	return 0;
}

void royalmah_prgbank_state::mjclub_bank_w(uint8_t data)
{
	m_dsw_select = data & 0xc0;

	m_mainbank->set_entry(data & 0x0f);

	// bit 5 used too but unknown purpose.
}


void royalmah_state::royalmah_map(address_map &map)
{
	map(0x0000, 0x6fff).rom().nopw();
	map(0x7000, 0x7fff).ram().share("nvram");
	map(0x8000, 0xffff).writeonly().share(m_videoram);
}

void royalmah_prgbank_state::royalmah_banked_map(address_map &map)
{
	royalmah_map(map);

	map(0x8000, 0xffff).bankr(m_mainbank);
}

void royalmah_prgbank_state::mjapinky_map(address_map &map)
{
	map(0x0000, 0x6fff).rom().nopw();
	map(0x7000, 0x77ff).ram().share("nvram");
	map(0x7800, 0x7fff).ram();
	map(0x8000, 0xffff).bankr(m_mainbank);
	map(0x8000, 0xffff).writeonly().share(m_videoram);
	map(0x8000, 0x8000).r(FUNC(royalmah_prgbank_state::mjapinky_dsw_r));
}

void royalmah_prgbank_state::tahjong_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().nopw();
	map(0x4000, 0x6fff).bankr(m_mainbank);
	map(0x7000, 0x7fff).ram().share("nvram");
	map(0x8000, 0xffff).writeonly().share(m_videoram);
}

void royalmah_prgbank_state::chalgirl_map(address_map &map) // TODO: guesswork, needs verifying
{
	map(0x0000, 0x6bff).rom().nopw();
	map(0x6c00, 0x6fff).bankr(m_mainbank);
	map(0x7000, 0x7fff).ram().share("nvram");
	map(0x8000, 0xffff).rom();
	map(0x8000, 0xffff).writeonly().share(m_videoram);
}

void royalmah_prgbank_state::mjsiyoub_map(address_map &map)
{
	map(0x0000, 0x6bff).rom().nopw();
	map(0x6c00, 0x6fff).bankr(m_mainbank);
	map(0x7000, 0x77ff).ram().share("nvram");
	map(0x7800, 0x7fff).rom().nopw();
	map(0x8000, 0xffff).nopr().writeonly().share(m_videoram);
}

void royalmah_prgbank_state::mjsenka_opcodes_map(address_map &map)
{
	map(0x0000, 0x6bff).rom().region("decrypted", 0);
	map(0x6c00, 0x6fff).bankr(m_mainopbank);
	map(0x8000, 0xffff).rom().region("maincpu", 0x8000);
}

void royalmah_state::ippatsu_map(address_map &map)
{
	royalmah_map(map);

	map(0x8000, 0xbfff).mirror(0x4000).rom();
}

void royalmah_state::seljan_map(address_map &map)
{
	map(0x0000, 0x8fff).rom().nopw();
	map(0xe000, 0xefff).ram().share("nvram");
}

void royalmah_prgbank_state::ichiban_map(address_map &map)
{
	map(0x0000, 0x6fff).rom().region("maincpu", 0x10000);
	map(0x7000, 0x7fff).ram().share("nvram");
	map(0x8000, 0xffff).bankr(m_mainbank);
	map(0x8000, 0xffff).writeonly().share(m_videoram);
}

void royalmah_prgbank_state::ichiban_opcodes_map(address_map &map)
{
	map(0x0000, 0x6fff).rom().region("maincpu", 0);
	map(0x8000, 0xffff).rom().region("maincpu", 0x8000);
}

void royalmah_prgbank_state::pongboo2_map(address_map &map)
{
	map(0x0000, 0x6fff).rom().region("maincpu", 0);
	map(0x7000, 0x7fff).ram().share("nvram");
	map(0x8000, 0xffff).bankr(m_mainbank); // TODO: proper range for banked ROM
	map(0x8000, 0xffff).writeonly().share(m_videoram);
}

void royalmah_prgbank_state::pongboo2_opcodes_map(address_map &map)
{
	map(0x0000, 0x6fff).rom().region("maincpu", 0x10000);
	map(0x7000, 0x7fff).ram().share("nvram");
	map(0x8000, 0xffff).rom().region("maincpu", 0x18000);
}

void royalmah_state::royalmah_iomap(address_map &map)
{
	map.global_mask(0xff);
	map(0x01, 0x01).r(m_ay, FUNC(ay8910_device::data_r));
	map(0x02, 0x03).w(m_ay, FUNC(ay8910_device::data_address_w));
	map(0x10, 0x10).portr("DSW1").w(FUNC(royalmah_state::royalmah_palbank_w));
	map(0x11, 0x11).portr("SYSTEM").w(FUNC(royalmah_state::input_port_select_w));
}

void royalmah_state::ippatsu_iomap(address_map &map)
{
	map.global_mask(0xff);
	map(0x01, 0x01).r(m_ay, FUNC(ay8910_device::data_r));
	map(0x02, 0x03).w(m_ay, FUNC(ay8910_device::data_address_w));
	map(0x10, 0x10).portr("DSW1").w(FUNC(royalmah_state::royalmah_palbank_w));
	map(0x11, 0x11).portr("SYSTEM").w(FUNC(royalmah_state::input_port_select_w));
	map(0x12, 0x12).portr("DSW2");
	map(0x13, 0x13).portr("DSW3");
}

void royalmah_state::janyoup2_iomap(address_map &map)
{
	ippatsu_iomap(map);

	map(0x20, 0x20).w("crtc", FUNC(mc6845_device::address_w));
	map(0x21, 0x21).w("crtc", FUNC(mc6845_device::register_w));
}

void royalmah_prgbank_state::chalgirl_iomap(address_map &map)
{
	royalmah_iomap(map);

	map(0x10, 0x10).w(FUNC(royalmah_prgbank_state::mjderngr_coin_w));
	map(0x40, 0x40).w(FUNC(royalmah_prgbank_state::chalgirl_bank_w));
}

void royalmah_state::seljan_iomap(address_map &map)
{
	map(0x0001, 0x0001).mirror(0x7f00).r(m_ay, FUNC(ay8910_device::data_r));
	map(0x0002, 0x0003).mirror(0x7f00).w(m_ay, FUNC(ay8910_device::data_address_w));
	map(0x0010, 0x0010).mirror(0x7f00).portr("DSW1").w(FUNC(royalmah_state::royalmah_palbank_w));
	map(0x0011, 0x0011).mirror(0x7f00).portr("SYSTEM").w(FUNC(royalmah_state::input_port_select_w));
	map(0x0012, 0x0012).mirror(0x7f00).portr("DSW2").w("crtc", FUNC(mc6845_device::address_w));
	map(0x0013, 0x0013).mirror(0x7f00).portr("DSW3").w("crtc", FUNC(mc6845_device::register_w));

	// TODO: following actually starts at 0x8000, needs custom 6845 routine because it uses start address register.
	map(0x7e00, 0xffff).ram().share(m_videoram);
}

void royalmah_prgbank_state::tahjong_iomap(address_map &map)
{
	map.global_mask(0xff);
	map(0x01, 0x01).r(m_ay, FUNC(ay8910_device::data_r));
	map(0x02, 0x03).w(m_ay, FUNC(ay8910_device::data_address_w));
	map(0x10, 0x10).portr("DSW1").w(FUNC(royalmah_prgbank_state::royalmah_palbank_w));
	map(0x11, 0x11).portr("SYSTEM").w(FUNC(royalmah_prgbank_state::input_port_select_w));
	map(0x12, 0x12).w(FUNC(royalmah_prgbank_state::tahjong_bank_w));
	map(0x13, 0x13).portr("DSW2");
}

void royalmah_prgbank_state::suzume_iomap(address_map &map)
{
	map.global_mask(0xff);
	map(0x01, 0x01).r(m_ay, FUNC(ay8910_device::data_r));
	map(0x02, 0x03).w(m_ay, FUNC(ay8910_device::data_address_w));
	map(0x10, 0x10).portr("DSW1").w(FUNC(royalmah_prgbank_state::royalmah_palbank_w));
	map(0x11, 0x11).portr("SYSTEM").w(FUNC(royalmah_prgbank_state::input_port_select_w));
	map(0x80, 0x80).r(FUNC(royalmah_prgbank_state::suzume_dsw_r));
	map(0x81, 0x81).w(FUNC(royalmah_prgbank_state::suzume_bank_w));
}

void royalmah_prgbank_state::jongshin_iomap(address_map &map)
{
	map.global_mask(0xff);
	map(0x01, 0x01).r(m_ay, FUNC(ay8910_device::data_r));
	map(0x02, 0x03).w(m_ay, FUNC(ay8910_device::data_address_w));
	map(0x10, 0x10).portr("DSW1").w(FUNC(royalmah_prgbank_state::royalmah_palbank_w));
	map(0x11, 0x11).portr("SYSTEM").w(FUNC(royalmah_prgbank_state::input_port_select_w));
	// map(0x80, 0x80).w(FUNC(royalmah_prgbank_state::???)); // set to 1 at start-up, then never changed?
	map(0x81, 0x81).portr("DSW2");
	map(0x82, 0x82).portr("DSW3");
	map(0xc0, 0xc0).w(FUNC(royalmah_prgbank_state::jongshin_bank_w));
}

void royalmah_prgbank_state::rkjanoh2_iomap(address_map &map)
{
	map.global_mask(0xff);
	map(0x01, 0x01).r(m_ay, FUNC(ay8910_device::data_r));
	map(0x02, 0x03).w(m_ay, FUNC(ay8910_device::data_address_w));
	map(0x10, 0x10).portr("DSW1").w(FUNC(royalmah_prgbank_state::mjderngr_coin_w));
	map(0x11, 0x11).portr("SYSTEM").w(FUNC(royalmah_prgbank_state::input_port_select_w));
	map(0x50, 0x55).w(FUNC(royalmah_prgbank_state::mjyarou_bank_w));
}

void royalmah_prgbank_state::mjyarou_iomap(address_map &map)
{
	rkjanoh2_iomap(map);

	map(0x12, 0x12).portr("DSW2");
}

void royalmah_prgbank_state::mjsiyoub_iomap(address_map &map)
{
	royalmah_iomap(map);

	map(0x13, 0x13).portr("DSW2");
	map(0x40, 0x49).w(FUNC(royalmah_prgbank_state::mjyarou_bank_w));
}

void royalmah_prgbank_state::ichiban_iomap(address_map &map)
{
	map.global_mask(0xff);
	map(0x01, 0x01).r("aysnd", FUNC(ym2149_device::data_r));
	map(0x02, 0x03).w("aysnd", FUNC(ym2149_device::data_address_w));
	map(0x10, 0x10).portr("DSW-A").w(FUNC(royalmah_prgbank_state::mjderngr_coin_w));
	map(0x11, 0x11).portr("SYSTEM").w(FUNC(royalmah_prgbank_state::input_port_select_w));
	map(0x12, 0x12).portr("DSW-B").lw8(NAME([this] (uint8_t data) { m_mainbank->set_entry(data); })); // TODO: only seems to write 0x00, 0x01, 0x02 and 0x04. How does the banking really work?
	map(0x13, 0x13).portr("DSW-C");
	map(0x14, 0x14).portr("DSW-D").lw8(NAME([this] (uint8_t data) { m_palette_base = data & 0x1f; })); // TODO: also uses bits 5 and 6
	map(0x16, 0x17).w("ymsnd", FUNC(ym2413_device::write));
}

void royalmah_prgbank_state::pongboo2_iomap(address_map &map) // TODO: banking, inputs
{
	map.global_mask(0xff);
	// map(0x50, 0x50) // reads P1 inputs from here, but active high / different from other games?
	// map(0x51, 0x51) // reads P2 inputs from here, but active high / different from other games?
	// map(0x80, 0x80).w // ??
	map(0x81, 0x81).w(FUNC(royalmah_prgbank_state::input_port_select_w)); // 0x01, 0x02, 0x04, 0x08, 0x10
	// map(0x82, 0x82).w // ??
	// map(0x84, 0x84).w // ??
	// map(0x91, 0x91).r // ??
	map(0x9a, 0x9a).w("oki", FUNC(okim6295_device::write));
	// map(0xa0, 0xbf).w // something similar to jansou_colortable_w ?
}

void royalmah_prgbank_state::dondenmj_iomap(address_map &map)
{
	map.global_mask(0xff);
	map(0x01, 0x01).r(m_ay, FUNC(ay8910_device::data_r));
	map(0x02, 0x03).w(m_ay, FUNC(ay8910_device::data_address_w));
	map(0x10, 0x10).portr("DSW1").w(FUNC(royalmah_prgbank_state::royalmah_palbank_w));
	map(0x11, 0x11).portr("SYSTEM").w(FUNC(royalmah_prgbank_state::input_port_select_w));
	map(0x85, 0x85).portr("DSW2");
	map(0x86, 0x86).portr("DSW3");
	map(0x87, 0x87).w(FUNC(royalmah_prgbank_state::dynax_bank_w));
}

void royalmah_prgbank_state::makaijan_iomap(address_map &map)
{
	map.global_mask(0xff);
	map(0x01, 0x01).r(m_ay, FUNC(ay8910_device::data_r));
	map(0x02, 0x03).w(m_ay, FUNC(ay8910_device::data_address_w));
	map(0x10, 0x10).portr("DSW1").w(FUNC(royalmah_prgbank_state::royalmah_palbank_w));
	map(0x11, 0x11).portr("SYSTEM").w(FUNC(royalmah_prgbank_state::input_port_select_w));
	map(0x84, 0x84).portr("DSW2");
	map(0x85, 0x85).portr("DSW3");
	map(0x86, 0x86).w(FUNC(royalmah_prgbank_state::dynax_bank_w));
}

void royalmah_prgbank_state::daisyari_iomap(address_map &map)
{
	map.global_mask(0xff);
	map(0x01, 0x01).r(m_ay, FUNC(ay8910_device::data_r));
	map(0x02, 0x03).w(m_ay, FUNC(ay8910_device::data_address_w));
	map(0x10, 0x10).w(FUNC(royalmah_prgbank_state::royalmah_palbank_w));
	map(0x11, 0x11).portr("SYSTEM").w(FUNC(royalmah_prgbank_state::input_port_select_w));
	map(0xc0, 0xc0).rw(FUNC(royalmah_prgbank_state::daisyari_dsw_r), FUNC(royalmah_prgbank_state::daisyari_bank_w));
}

void royalmah_prgbank_state::mjclub_iomap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).rw(FUNC(royalmah_prgbank_state::mjclub_dsw_r), FUNC(royalmah_prgbank_state::mjclub_bank_w));
	map(0x01, 0x01).r(m_ay, FUNC(ay8910_device::data_r));
	map(0x02, 0x03).w(m_ay, FUNC(ay8910_device::data_address_w));
	map(0x10, 0x10).portr("DSW1").w(FUNC(royalmah_prgbank_state::royalmah_palbank_w));
	map(0x11, 0x11).portr("SYSTEM").w(FUNC(royalmah_prgbank_state::input_port_select_w));
}

void royalmah_prgbank_state::mjdiplob_iomap(address_map &map)
{
	map.global_mask(0xff);
	map(0x01, 0x01).r(m_ay, FUNC(ay8910_device::data_r));
	map(0x02, 0x03).w(m_ay, FUNC(ay8910_device::data_address_w));
	map(0x10, 0x10).portr("DSW1").w(FUNC(royalmah_prgbank_state::royalmah_palbank_w));
	map(0x11, 0x11).portr("SYSTEM").w(FUNC(royalmah_prgbank_state::input_port_select_w));
	map(0x61, 0x61).w(FUNC(royalmah_prgbank_state::tontonb_bank_w));
	map(0x62, 0x62).portr("DSW2"); // DSW2
	map(0x63, 0x63).portr("DSW3"); // DSW3
}

void royalmah_prgbank_state::tontonb_iomap(address_map &map)
{
	map.global_mask(0xff);
	map(0x01, 0x01).r(m_ay, FUNC(ay8910_device::data_r));
	map(0x02, 0x03).w(m_ay, FUNC(ay8910_device::data_address_w));
	map(0x10, 0x10).portr("DSW1").w(FUNC(royalmah_prgbank_state::royalmah_palbank_w));
	map(0x11, 0x11).portr("SYSTEM").w(FUNC(royalmah_prgbank_state::input_port_select_w));
	map(0x44, 0x44).w(FUNC(royalmah_prgbank_state::tontonb_bank_w));
	map(0x46, 0x46).portr("DSW2"); // DSW2
	map(0x47, 0x47).portr("DSW3"); // DSW3
}

void royalmah_prgbank_state::majs101b_iomap(address_map &map)
{
	map.global_mask(0xff);
	map(0x01, 0x01).r(m_ay, FUNC(ay8910_device::data_r));
	map(0x02, 0x03).w(m_ay, FUNC(ay8910_device::data_address_w));
	map(0x10, 0x10).portr("DSW1").w(FUNC(royalmah_prgbank_state::royalmah_palbank_w));
	map(0x11, 0x11).portr("SYSTEM").w(FUNC(royalmah_prgbank_state::input_port_select_w));
	map(0x00, 0x00).rw(FUNC(royalmah_prgbank_state::majs101b_dsw_r), FUNC(royalmah_prgbank_state::dynax_bank_w));
}

void royalmah_prgbank_state::mjderngr_iomap(address_map &map)
{
	map.global_mask(0xff);
	map(0x01, 0x01).r(m_ay, FUNC(ay8910_device::data_r));
	map(0x02, 0x03).w(m_ay, FUNC(ay8910_device::data_address_w));
//  map(0x10, 0x10).portr("DSW1");
	map(0x10, 0x10).w(FUNC(royalmah_prgbank_state::mjderngr_coin_w));   // palette bank is set separately
	map(0x11, 0x11).portr("SYSTEM").w(FUNC(royalmah_prgbank_state::input_port_select_w));
	map(0x20, 0x20).w(FUNC(royalmah_prgbank_state::dynax_bank_w));
	map(0x40, 0x40).portr("DSW2");
	map(0x4c, 0x4c).portr("DSW1");
	map(0x60, 0x60).w(FUNC(royalmah_prgbank_state::mjderngr_palbank_w));
}

void royalmah_prgbank_state::mjapinky_iomap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(royalmah_prgbank_state::mjapinky_bank_w));
	map(0x01, 0x01).r(m_ay, FUNC(ay8910_device::data_r));
	map(0x02, 0x03).w(m_ay, FUNC(ay8910_device::data_address_w));
	map(0x04, 0x04).portr("DSW2");
	map(0x10, 0x10).portr("DSW1").w(FUNC(royalmah_prgbank_state::mjapinky_palbank_w));
	map(0x11, 0x11).portr("SYSTEM").w(FUNC(royalmah_prgbank_state::input_port_select_w));
}

void royalmah_state::janoh_map(address_map &map)
{
	map(0x0000, 0x6fff).rom().nopw();
	map(0x7000, 0x7fff).ram().share("nvram");
	map(0x8000, 0xffff).writeonly().share(m_videoram);
}


/* this CPU makes little sense - what is it for? why so many addresses accessed?
  -- it puts a value in shared ram to allow the main CPU to boot, then.. ?
*/
void royalmah_state::janoh_sub_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4100, 0x413f).ram();
	map(0x6000, 0x607f).ram();
	map(0x7000, 0x7000).nopr();
	map(0x7200, 0x7200).nopw();
	map(0xf000, 0xffff).ram().share("nvram");
}

void royalmah_state::janoh_sub_iomap(address_map &map)
{
}

void royalmah_prgbank_state::mjsiyoub_audio_prg_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
}


/****************************************************************************
                                Jansou
****************************************************************************/

void royalmah_state::jansou_dsw_sel_w(uint8_t data)
{
	m_dsw_select = data;
}

uint8_t royalmah_state::jansou_dsw_r()
{
	switch (m_dsw_select & 7)
	{
		case 1: return m_dsw[0]->read();
		case 2: return m_dsw[1]->read();
		case 4: return m_dsw[2]->read();
	}

	return 0xff;
}

void royalmah_state::jansou_colortable_w(offs_t offset, uint8_t data)
{
	m_jansou_colortable[offset] = data;
}

void royalmah_state::jansou_6400_w(uint8_t data)
{
	m_gfx_adr_l = data;
	m_gfx_adr = m_gfx_adr_h * 0x10000 + m_gfx_adr_m * 0x100 + m_gfx_adr_l;
}

void royalmah_state::jansou_6401_w(uint8_t data)
{
	m_gfx_adr_m = data;
	m_gfx_adr = m_gfx_adr_h * 0x10000 + m_gfx_adr_m * 0x100 + m_gfx_adr_l;
}

void royalmah_state::jansou_6402_w(uint8_t data)
{
	m_gfx_adr_h = data & 1;
	m_gfx_adr = m_gfx_adr_h * 0x10000 + m_gfx_adr_m * 0x100 + m_gfx_adr_l;
}

uint8_t royalmah_state::jansou_6403_r()
{
	int d0 = m_gfx_rom[m_gfx_adr];
	int d1 = m_gfx_rom[m_gfx_adr + 1];
	int c0 = m_jansou_colortable[d1 & 0x0f] & 0x0f;
	int c1 = m_jansou_colortable[(d1 & 0xf0) >> 4] >> 4;
	int c2 = m_jansou_colortable[d0 & 0x0f] & 0x0f;
	int c3 = m_jansou_colortable[(d0 & 0xf0) >> 4] >> 4;

	m_gfx_adr += 2;

	m_gfxdata0 = (c3 & 1) << 0 | ((c2 & 1) << 1) | ((c1 & 1) << 2) | ((c0 & 1) << 3)
				| ((c3 & 2) << 3) | ((c2 & 2) << 4) | ((c1 & 2) << 5) | ((c0 & 2) << 6);
	m_gfxdata1 = (c3 & 4) >> 2 | ((c2 & 4) >> 1) | (c1 & 4) | ((c0 & 4) << 1)
				| ((c3 & 8) << 1) | ((c2 & 8) << 2) | ((c1 & 8) << 3) | ((c0 & 8) << 4);

	return 0xff;
}

uint8_t royalmah_state::jansou_6404_r()
{
	return m_gfxdata0;
}

uint8_t royalmah_state::jansou_6405_r()
{
	return m_gfxdata1;
}

void royalmah_state::jansou_sound_w(uint8_t data)
{
	m_soundlatch->write(data);
	m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void royalmah_state::jansou_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().nopw();

	map(0x6000, 0x600f).w(FUNC(royalmah_state::jansou_colortable_w));
	map(0x6400, 0x6400).w(FUNC(royalmah_state::jansou_6400_w));
	map(0x6401, 0x6401).w(FUNC(royalmah_state::jansou_6401_w));
	map(0x6402, 0x6402).w(FUNC(royalmah_state::jansou_6402_w));
	map(0x6403, 0x6403).r(FUNC(royalmah_state::jansou_6403_r));
	map(0x6404, 0x6404).r(FUNC(royalmah_state::jansou_6404_r));
	map(0x6405, 0x6405).r(FUNC(royalmah_state::jansou_6405_r));
	map(0x6406, 0x6406).w(FUNC(royalmah_state::jansou_dsw_sel_w));
	map(0x6407, 0x6407).r(FUNC(royalmah_state::jansou_dsw_r));
	map(0x6800, 0x6800).w(FUNC(royalmah_state::jansou_sound_w));

	map(0x7000, 0x77ff).ram().share("nvram");
	map(0x8000, 0xffff).writeonly().share(m_videoram);
}

void royalmah_state::jansou_sub_map(address_map &map)
{
	map(0x0000, 0xffff).rom().nopw(); // tries to write to the stack at irq generation
}


void royalmah_state::jansou_sub_iomap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r(m_soundlatch, FUNC(generic_latch_8_device::read)).w("dac", FUNC(dac_byte_interface::data_w));
}


/****************************************************************************
                                Janputer '96
****************************************************************************/

void royalmah_prgbank_state::janptr96_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x6000, 0x6fff).bankrw("bank3").share("nvram");    // nvram
	map(0x7000, 0x7fff).bankrw(m_rambank);  // banked nvram
	map(0x8000, 0xffff).bankr(m_mainbank);
	map(0x8000, 0xffff).writeonly().share(m_videoram);
}

void royalmah_prgbank_state::janptr96_dswsel_w(uint8_t data)
{
	// 0x20 = 0 -> hopper on
	// 0x40 ?
	m_dsw_select = data;
}

uint8_t royalmah_prgbank_state::janptr96_dsw_r()
{
	uint8_t result = 0xff;
	if (~m_dsw_select & 0x01) result &= m_dsw[3]->read();
	if (~m_dsw_select & 0x02) result &= m_dsw[2]->read();
	if (~m_dsw_select & 0x04) result &= m_dsw[1]->read();
	if (~m_dsw_select & 0x08) result &= m_dsw[0]->read();
	if (~m_dsw_select & 0x10) result &= m_dsw_top->read();
	return result;
}

void royalmah_prgbank_state::janptr96_rombank_w(uint8_t data)
{
	m_mainbank->set_entry(data & 0x3f);
}

void royalmah_prgbank_state::janptr96_rambank_w(uint8_t data)
{
	m_rambank->set_entry(data & 0x07);
}

uint8_t royalmah_prgbank_state::janptr96_unknown_r()
{
	// 0x08 = 0 makes the game crash (e.g. in the m-ram test: nested interrupts?)
	return 0xff;
}

void royalmah_prgbank_state::janptr96_coin_counter_w(uint8_t data)
{
	m_flip_screen = (data & 4) >> 2;
	machine().bookkeeping().coin_counter_w(0, data & 2);  // in
	machine().bookkeeping().coin_counter_w(1, data & 1);  // out
}

void royalmah_prgbank_state::janptr96_iomap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(royalmah_prgbank_state::janptr96_rombank_w));    // BANK ROM Select
	map(0x20, 0x20).rw(FUNC(royalmah_prgbank_state::janptr96_unknown_r), FUNC(royalmah_prgbank_state::janptr96_rambank_w));
	map(0x50, 0x50).w(FUNC(royalmah_prgbank_state::mjderngr_palbank_w));
	map(0x60, 0x6f).rw(m_rtc, FUNC(msm6242_device::read), FUNC(msm6242_device::write));
	map(0x81, 0x81).r(m_ay, FUNC(ay8910_device::data_r));
	map(0x82, 0x83).w(m_ay, FUNC(ay8910_device::data_address_w));
	map(0x93, 0x93).w(FUNC(royalmah_prgbank_state::input_port_select_w));
	map(0xd8, 0xd8).w(FUNC(royalmah_prgbank_state::janptr96_coin_counter_w));
	map(0xd9, 0xd9).portr("SYSTEM").nopw(); // second input select?
}


/****************************************************************************
                                Mahjong If
****************************************************************************/

void royalmah_prgbank_state::mjifb_coin_counter_w(uint8_t data)
{
	m_flip_screen = ((data & 4) >> 2) ^ 1;
	machine().bookkeeping().coin_counter_w(0, data & 2);  // in
	machine().bookkeeping().coin_counter_w(1, data & 1);  // out
}

void royalmah_prgbank_state::mjifb_map(address_map &map)
{
	map(0x0000, 0x6fff).rom();
	map(0x7000, 0x7fff).ram().share("nvram");
	map(0x8000, 0xffff).writeonly().share(m_videoram);
	map(0x8000, 0xbfff).view(m_mainview);
	m_mainview[0](0x8000, 0x8000).portr("DSW4");
	m_mainview[0](0x8200, 0x8200).portr("DSW3");
	m_mainview[0](0x8e00, 0x8e00).lw8(NAME([this] (uint8_t data) { m_palette_base = data & 0x1f; }));
	m_mainview[0](0x9001, 0x9001).r(m_ay, FUNC(ay8910_device::data_r));
	m_mainview[0](0x9002, 0x9002).w(m_ay, FUNC(ay8910_device::data_w));
	m_mainview[0](0x9003, 0x9003).w(m_ay, FUNC(ay8910_device::address_w));
	m_mainview[0](0x9010, 0x9010).w(FUNC(royalmah_prgbank_state::mjifb_coin_counter_w));
	m_mainview[0](0x9011, 0x9011).portr("SYSTEM").w(FUNC(royalmah_prgbank_state::input_port_select_w));
	m_mainview[1](0x8000, 0xbfff).bankr(m_mainbank).lw8(NAME([this] (offs_t offset, uint8_t data) { m_videoram[offset] = data; }));
	map(0xc000, 0xffff).rom();
}

uint8_t royalmah_prgbank_state::mjifb_p8_r()
{
	return 0xff;
}

void royalmah_prgbank_state::mjifb_p3_w(uint8_t data)
{
	m_rombank = (m_rombank & 0x0f) | ((data & 0x0c) << 2);
	m_mainbank->set_entry(m_rombank);
}
void royalmah_prgbank_state::mjifb_p4_w(uint8_t data)
{
	m_rombank = (m_rombank & 0xf0) | (data & 0x0f);
	m_mainbank->set_entry(m_rombank);
}
void royalmah_prgbank_state::mjifb_p8_w(uint8_t data)
{
	m_mainview.select(BIT(data, 3));
}


/****************************************************************************
                           Mahjong Shinkirou Deja Vu
****************************************************************************/

void royalmah_prgbank_state::mjdejavu_map(address_map &map)
{
	map(0x0000, 0x6fff).rom();
	map(0x7000, 0x7fff).ram().share("nvram");
	map(0x8000, 0xffff).writeonly().share(m_videoram);
	map(0x8000, 0xbfff).view(m_mainview);
	m_mainview[0](0x8000, 0x8000).portr("DSW2");
	m_mainview[0](0x8001, 0x8001).portr("DSW1");
	m_mainview[0](0x8802, 0x8802).lw8(NAME([this] (uint8_t data) { m_palette_base = data & 0x1f; }));
	m_mainview[0](0x9001, 0x9001).r(m_ay, FUNC(ay8910_device::data_r));
	m_mainview[0](0x9002, 0x9002).w(m_ay, FUNC(ay8910_device::data_w));
	m_mainview[0](0x9003, 0x9003).w(m_ay, FUNC(ay8910_device::address_w));
	m_mainview[0](0x9010, 0x9010).w(FUNC(royalmah_prgbank_state::janptr96_coin_counter_w));
	m_mainview[0](0x9011, 0x9011).portr("SYSTEM").w(FUNC(royalmah_prgbank_state::input_port_select_w));
	m_mainview[1](0x8000, 0xbfff).bankr(m_mainbank).lw8(NAME([this] (offs_t offset, uint8_t data) { m_videoram[offset] = data; }));
	map(0xc000, 0xffff).rom();
}


/****************************************************************************
                                Mahjong Tensinhai
****************************************************************************/

uint8_t royalmah_prgbank_state::mjtensin_p3_r()
{
	return 0xff;
}

void royalmah_prgbank_state::mjtensin_p4_w(uint8_t data)
{
	m_rombank = (m_rombank & 0xf0) | (data & 0x0f);
	m_mainbank->set_entry(m_rombank);
}

void royalmah_prgbank_state::mjtensin_6ff3_w(uint8_t data)
{
	m_rombank = (data << 4) | (m_rombank & 0x0f);
	m_mainbank->set_entry(m_rombank);
}

void royalmah_prgbank_state::mjtensin_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x6000, 0x6fbf).ram();
	map(0x6fc1, 0x6fc1).r(m_ay, FUNC(ay8910_device::data_r));
	map(0x6fc2, 0x6fc3).w(m_ay, FUNC(ay8910_device::data_address_w));
	map(0x6fd0, 0x6fd0).w(FUNC(royalmah_prgbank_state::janptr96_coin_counter_w));
	map(0x6fd1, 0x6fd1).portr("SYSTEM").w(FUNC(royalmah_prgbank_state::input_port_select_w));
	map(0x6fe0, 0x6fef).rw(m_rtc, FUNC(msm6242_device::read), FUNC(msm6242_device::write));
	map(0x6ff0, 0x6ff0).rw(FUNC(royalmah_prgbank_state::janptr96_dsw_r), FUNC(royalmah_prgbank_state::janptr96_dswsel_w));
	map(0x6ff1, 0x6ff1).w(FUNC(royalmah_prgbank_state::mjderngr_palbank_w));
	map(0x6ff3, 0x6ff3).w(FUNC(royalmah_prgbank_state::mjtensin_6ff3_w));
	map(0x7000, 0x7fff).ram().share("nvram");
	map(0x8000, 0xffff).bankr(m_mainbank);
	map(0x8000, 0xffff).writeonly().share(m_videoram);
}

void royalmah_prgbank_state::majrjh_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x6000, 0x7eff).ram().share("nvram");
	map(0x7fc1, 0x7fc1).r(m_ay, FUNC(ay8910_device::data_r));
	map(0x7fc2, 0x7fc3).w(m_ay, FUNC(ay8910_device::data_address_w));
	map(0x7fd0, 0x7fd0).w(FUNC(royalmah_prgbank_state::janptr96_coin_counter_w));
	map(0x7fd1, 0x7fd1).portr("SYSTEM").w(FUNC(royalmah_prgbank_state::input_port_select_w));
	map(0x7fe0, 0x7fe0).w(FUNC(royalmah_prgbank_state::mjtensin_6ff3_w));
	map(0x7fe2, 0x7fe2).w(FUNC(royalmah_prgbank_state::mjderngr_palbank_w));
	map(0x7fe3, 0x7fe3).rw(FUNC(royalmah_prgbank_state::janptr96_dsw_r), FUNC(royalmah_prgbank_state::janptr96_dswsel_w));
	map(0x7ff0, 0x7fff).rw(m_rtc, FUNC(msm6242_device::read), FUNC(msm6242_device::write));
	map(0x8000, 0xffff).bankr(m_mainbank);
	map(0x8000, 0xffff).writeonly().share(m_videoram);
}

/****************************************************************************
                                Mahjong Cafe Time
****************************************************************************/

void royalmah_prgbank_state::cafetime_p4_w(uint8_t data)
{
	m_rombank = (m_rombank & 0xf0) | (data & 0x0f);
	m_mainbank->set_entry(m_rombank);
}

void royalmah_prgbank_state::cafetime_p3_w(uint8_t data)
{
	m_rombank = (m_rombank & 0x0f) | ((data & 0x0c) << 2);
	m_mainbank->set_entry(m_rombank);
}

void royalmah_prgbank_state::cafedoll_p6_w(uint8_t data)
{
	m_mjvegas_p5_val &= 0x0f;

	if (data & 0x01)
		m_mjvegas_p5_val |= (1 << 4);
}

void royalmah_prgbank_state::cafedoll_p7_w(uint8_t data)
{
	m_mjvegas_p5_val &= 0xf0;

	if (data & 0x0f)
		m_mjvegas_p5_val |= (1 << 3);
}

void royalmah_prgbank_state::cafetime_dsw_w(uint8_t data)
{
	m_dsw_select = data;
}
uint8_t royalmah_prgbank_state::cafetime_dsw_r()
{
	switch (m_dsw_select)
	{
		case 0x00: return m_dsw[0]->read();
		case 0x01: return m_dsw[1]->read();
		case 0x02: return m_dsw[2]->read();
		case 0x03: return m_dsw[3]->read();
		case 0x04: return m_dsw_top->read();
	}
	logerror("%04X: unmapped dsw read %02X\n", m_maincpu->pc(), m_dsw_select);
	return 0xff;
}

uint8_t royalmah_prgbank_state::cafetime_7fe4_r()
{
	return 0xff;
}
void royalmah_prgbank_state::cafetime_7fe3_w(uint8_t data)
{
//  logerror("7fe3_w: %02x", data);
}

void royalmah_prgbank_state::cafetime_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x6000, 0x7eff).ram().share("nvram");
	map(0x7fc1, 0x7fc1).r(m_ay, FUNC(ay8910_device::data_r));
	map(0x7fc2, 0x7fc3).w(m_ay, FUNC(ay8910_device::data_address_w));
	map(0x7fd0, 0x7fd0).w(FUNC(royalmah_prgbank_state::janptr96_coin_counter_w));
	map(0x7fd1, 0x7fd1).portr("SYSTEM").nopw();
	map(0x7fd3, 0x7fd3).w(FUNC(royalmah_prgbank_state::input_port_select_w));
	map(0x7fe0, 0x7fe0).r(FUNC(royalmah_prgbank_state::cafetime_dsw_r));
	map(0x7fe1, 0x7fe1).w(FUNC(royalmah_prgbank_state::cafetime_dsw_w));
	map(0x7fe2, 0x7fe2).w(FUNC(royalmah_prgbank_state::mjderngr_palbank_w));
	map(0x7fe3, 0x7fe3).w(FUNC(royalmah_prgbank_state::cafetime_7fe3_w));
	map(0x7fe4, 0x7fe4).r(FUNC(royalmah_prgbank_state::cafetime_7fe4_r));
	map(0x7ff0, 0x7fff).rw(m_rtc, FUNC(msm6242_device::read), FUNC(msm6242_device::write));
	map(0x8000, 0xffff).bankr(m_mainbank);
	map(0x8000, 0xffff).writeonly().share(m_videoram);
}


/****************************************************************************
                                Mahjong Cafe Paradise
****************************************************************************/

void royalmah_prgbank_state::cafepara_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x6000, 0x6fff).bankrw("bank3").share("nvram");    // nvram
	map(0x7000, 0x7fff).bankrw(m_rambank);  // banked nvram
	map(0x7fe1, 0x7fe1).r(m_ay, FUNC(ay8910_device::data_r));
	map(0x7fe2, 0x7fe3).w(m_ay, FUNC(ay8910_device::data_address_w));
	map(0x7ff0, 0x7ff0).w(FUNC(royalmah_prgbank_state::janptr96_coin_counter_w));
	map(0x7ff1, 0x7ff1).portr("SYSTEM").nopw();
	map(0x7ff3, 0x7ff3).w(FUNC(royalmah_prgbank_state::input_port_select_w));
	map(0x7ff4, 0x7ff4).lw8(NAME([this] (uint8_t data) { m_mainbank->set_entry(data); if (data >= 0x10) logerror("mainbank_w: %02x\n", data); }));
	map(0x7ff5, 0x7ff5).w(FUNC(royalmah_prgbank_state::janptr96_rambank_w));
	map(0x7ff6, 0x7ff6).w(FUNC(royalmah_prgbank_state::mjderngr_palbank_w));
	map(0x7ff7, 0x7ff7).w(FUNC(royalmah_prgbank_state::cafetime_7fe3_w));
	map(0x8000, 0xffff).bankr(m_mainbank);
	map(0x8000, 0xffff).writeonly().share(m_videoram);
	map(0xfff0, 0xffff).rw(m_rtc, FUNC(msm6242_device::read), FUNC(msm6242_device::write)); // TODO: this should probably be behind a view
}


/****************************************************************************
                               Mahjong Vegas
****************************************************************************/
uint8_t royalmah_prgbank_state::mjvegas_p5_r()
{
	return m_mjvegas_p5_val;
}

void royalmah_prgbank_state::mjvegas_p6_w(uint8_t data)
{
	m_mjvegas_p5_val &= 0x0f;

	if (data & 0x07)
		m_mjvegas_p5_val |= (1 << 4);
}

void royalmah_prgbank_state::mjvegas_p7_w(uint8_t data)
{
	m_mjvegas_p5_val &= 0xf0;

	if (data & 0x07)
		m_mjvegas_p5_val |= (1 << 3);
}
void royalmah_prgbank_state::mjvegasa_p4_w(uint8_t data)
{
	m_rombank = (m_rombank & 0xf8) | ((data & 0x0e) >> 1);
	m_mainbank->set_entry(m_rombank);
}
void royalmah_prgbank_state::mjvegasa_p3_w(uint8_t data)
{
	m_rombank = (m_rombank & 0xf7) | ((data & 0x04) << 1);
	m_mainbank->set_entry(m_rombank);
}
void royalmah_prgbank_state::mjvegasa_rombank_w(uint8_t data)
{
	m_rombank = (m_rombank & 0x0f) | ((data & 0x0f) << 4);
	m_mainbank->set_entry(m_rombank);
}

uint8_t royalmah_prgbank_state::mjvegasa_rom_io_r(offs_t offset)
{
	if ((m_rombank & 0x70) != 0x70)
	{
		uint8_t *ptr = (uint8_t*)(m_mainbank->base());
		return ptr[offset];
	}

	return m_rtc->read(offset & 0xf);

	//logerror("mjvegasa_rom_io_r: %04X: unmapped IO read at %04X\n", m_maincpu->pc(), offset);
	//return 0xff;
}

void royalmah_prgbank_state::mjvegasa_rom_io_w(offs_t offset, uint8_t data)
{
	if ((m_rombank & 0x70) != 0x70)
	{
		m_videoram[offset] = data;
		return;
	}

	offset += 0x8000;

	if((offset & 0xfff0) == 0x8000)
	{
		m_rtc->write(offset & 0xf, data);
		return;
	}

	logerror("mjvegasa_rom_io_w: %04X: unmapped IO write at %04X = %02X\n", m_maincpu->pc(), offset, data);
}

void royalmah_prgbank_state::mjvegasa_coin_counter_w(uint8_t data)
{
	m_flip_screen = (data & 4) >> 2;
	machine().bookkeeping().coin_counter_w(0, data & 2);  // in
	machine().bookkeeping().coin_counter_w(1, data & 1);  // out
}

// hopper?
void royalmah_prgbank_state::mjvegasa_12400_w(uint8_t data)
{
	// bits 0 & 1
//  logerror("12400_w: %02x", data);
}
uint8_t royalmah_prgbank_state::mjvegasa_12500_r()
{
	// bits 0 & 2
	return 0xff;
}

void royalmah_prgbank_state::mjvegasa_map(address_map &map)
{
	map(0x00000, 0x05fff).rom();
	map(0x06000, 0x07fff).ram().share("nvram");
	map(0x08000, 0x0ffff).bankr(m_mainbank).w(FUNC(royalmah_prgbank_state::mjvegasa_rom_io_w)).share(m_videoram);
	map(0x08000, 0x0800f).r(FUNC(royalmah_prgbank_state::mjvegasa_rom_io_r));

	map(0x10001, 0x10001).r(m_ay, FUNC(ay8910_device::data_r));
	map(0x10002, 0x10003).w(m_ay, FUNC(ay8910_device::data_address_w));
	map(0x10010, 0x10010).w(FUNC(royalmah_prgbank_state::mjvegasa_coin_counter_w));
	map(0x10011, 0x10011).portr("SYSTEM").w(FUNC(royalmah_prgbank_state::input_port_select_w));
	map(0x10013, 0x10013).w(FUNC(royalmah_prgbank_state::input_port_select_w));

	map(0x12000, 0x12000).w(FUNC(royalmah_prgbank_state::mjvegasa_rombank_w));
	map(0x12100, 0x12100).r(FUNC(royalmah_prgbank_state::cafetime_dsw_r));
	map(0x12200, 0x12200).w(FUNC(royalmah_prgbank_state::cafetime_dsw_w));
	map(0x12300, 0x12300).w(FUNC(royalmah_prgbank_state::mjderngr_palbank_w));
	map(0x12400, 0x12400).w(FUNC(royalmah_prgbank_state::mjvegasa_12400_w));
	map(0x12500, 0x12500).r(FUNC(royalmah_prgbank_state::mjvegasa_12500_r));
}



static INPUT_PORTS_START( mjctrl1 )
	PORT_START("KEY0")  // P1 IN0
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Credit Clear") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Credit Clear") PORT_CODE(KEYCODE_8)

	PORT_START("KEY1")  // P1 IN1
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY2")  // P1 IN2
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY3")  // P1 IN3
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY4")  // P1 IN4
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY5")  // P2 IN0
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY6")  // P2 IN1
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) PORT_PLAYER(2)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY7")  // P2 IN2
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY8")  // P2 IN3
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(2)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY9")  // P2 IN4
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL ) PORT_PLAYER(2)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")    // IN10
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 ) // "Note" ("Paper Money") = 10 Credits
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_MEMORY_RESET )  // Memory Reset
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE1 )  // Analizer (Statistics)
	PORT_SERVICE( 0x08, IP_ACTIVE_HIGH )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( mjctrl2 )
	PORT_INCLUDE( mjctrl1 )

	PORT_MODIFY("KEY0")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Payout") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( royalmah )
	PORT_INCLUDE( mjctrl1 )

	PORT_MODIFY("KEY5")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )  // "COIN2"
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3 )  // "COIN1", but not working

	PORT_START("DSW1")  // DSW  (inport $10)
	PORT_DIPNAME( 0x0f, 0x0f, "Pay Out Rate" )
	PORT_DIPSETTING(    0x0f, "96%" )
	PORT_DIPSETTING(    0x0e, "93%" )
	PORT_DIPSETTING(    0x0d, "90%" )
	PORT_DIPSETTING(    0x0c, "87%" )
	PORT_DIPSETTING(    0x0b, "84%" )
	PORT_DIPSETTING(    0x0a, "81%" )
	PORT_DIPSETTING(    0x09, "78%" )
	PORT_DIPSETTING(    0x08, "75%" )
	PORT_DIPSETTING(    0x07, "71%" )
	PORT_DIPSETTING(    0x06, "68%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x04, "62%" )
	PORT_DIPSETTING(    0x03, "59%" )
	PORT_DIPSETTING(    0x02, "56%" )
	PORT_DIPSETTING(    0x01, "53%" )
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPNAME( 0x30, 0x30, "Maximum Bet" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x20, "10" )
	PORT_DIPSETTING(    0x30, "20" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( tahjong )
	PORT_INCLUDE( mjctrl1 )

	PORT_MODIFY("KEY5")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )  // "COIN2"
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3 )  // "COIN1", but not working

	PORT_START("DSW1")  // port $10
	PORT_DIPNAME( 0x07, 0x07, "SWB:1,2,3" ) PORT_DIPLOCATION("SWB:1,2,3")   // only with bets on
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x06, "5 (6)" )
	PORT_DIPSETTING(    0x07, "5 (7)" )
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SWB:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SWB:5")
	PORT_DIPNAME( 0x60, 0x60, "Winnings" ) PORT_DIPLOCATION("SWB:6,7")
	PORT_DIPSETTING(    0x40, "30 30 10 10 5 5 1 1" )
	PORT_DIPSETTING(    0x20, "32 24 16 12 8 4 2 1" )
	PORT_DIPSETTING(    0x60, "100 50 30 10 5 4 3 2" )
	PORT_DIPSETTING(    0x00, "200 50 30 10 5 4 3 2" )
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SWB:8")

	PORT_START("DSW2")  // port $13
	PORT_DIPNAME( 0x07, 0x07, "Pay Out Rate" ) PORT_DIPLOCATION("SWA:1,2,3")
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPSETTING(    0x04, "62%" )
	PORT_DIPSETTING(    0x02, "68%" )
	PORT_DIPSETTING(    0x06, "71%" )
	PORT_DIPSETTING(    0x01, "75%" )
	PORT_DIPSETTING(    0x05, "78%" )
	PORT_DIPSETTING(    0x03, "81%" )
	PORT_DIPSETTING(    0x07, "90%" )
	PORT_DIPNAME( 0x18, 0x18, "Maximum Bet" ) PORT_DIPLOCATION("SWA:4,5")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPSETTING(    0x18, "20" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SWA:6")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x40, 0x40, "Allow Bets" ) PORT_DIPLOCATION("SWA:7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Note Rate" )     PORT_DIPLOCATION("SWA:8")
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x80, "10" )
INPUT_PORTS_END

static INPUT_PORTS_START( janyoup2 )
	PORT_INCLUDE( royalmah )

	PORT_START("DSW2")  // DSW  (inport $12)
	PORT_DIPNAME( 0x01, 0x01, "Unknown 1-0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 1-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 1-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 1-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 1-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 1-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 1-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 1-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")  // DSW  (inport $13)
	PORT_DIPNAME( 0x01, 0x01, "Unknown 2-0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 2-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 2-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 2-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 2-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 2-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 2-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 2-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( seljan )
	PORT_INCLUDE( royalmah )

	PORT_START("DSW2")  // DSW  (inport $12)
	PORT_DIPNAME( 0x01, 0x01, "Unknown 1-0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 1-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 1-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 1-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 1-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Unknown 1-5" ) // these two activate credit analyzer if they are off
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Unknown 1-6" ) // ^
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 1-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")  // DSW  (inport $13)
	PORT_DIPNAME( 0x01, 0x01, "Unknown 2-0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 2-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 2-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 2-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 2-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 2-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 2-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 2-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( suzume )
	PORT_INCLUDE( mjctrl2 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, "Pay Out Rate" )
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPSETTING(    0x01, "53%" )
	PORT_DIPSETTING(    0x02, "56%" )
	PORT_DIPSETTING(    0x03, "59%" )
	PORT_DIPSETTING(    0x04, "62%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x06, "68%" )
	PORT_DIPSETTING(    0x07, "71%" )
	PORT_DIPSETTING(    0x08, "75%" )
	PORT_DIPSETTING(    0x09, "78%" )
	PORT_DIPSETTING(    0x0a, "81%" )
	PORT_DIPSETTING(    0x0b, "84%" )
	PORT_DIPSETTING(    0x0c, "87%" )
	PORT_DIPSETTING(    0x0d, "90%" )
	PORT_DIPSETTING(    0x0e, "93%" )
	PORT_DIPSETTING(    0x0f, "96%" )
	PORT_DIPNAME( 0x30, 0x30, "Maximum Bet" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x20, "10" )
	PORT_DIPSETTING(    0x30, "20" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, "Winnings" )
	PORT_DIPSETTING(    0x03, "50 30 15 8 5 3 2 1" )
	PORT_DIPSETTING(    0x00, "50 30 20 15 8 6 3 2" )
	PORT_DIPSETTING(    0x02, "100 50 25 10 5 3 2 1" )
	PORT_DIPSETTING(    0x01, "200 100 50 10 5 3 2 1" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x03, "8" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Background" )
	PORT_DIPSETTING(    0x08, "Black" )
	PORT_DIPSETTING(    0x00, "Green" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Girls" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( jongshin )
	PORT_INCLUDE( mjctrl2 )

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x00, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x00, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x00, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x00, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x00, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x00, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x00, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x00, "SW1:8")

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x00, "SW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x00, "SW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x00, "SW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x00, "SW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x00, "SW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x00, "SW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x00, "SW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x00, "SW2:8") // setting this causes the game to continually reset on title screen

	PORT_START("DSW3")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x00, "SW3:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x00, "SW3:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x00, "SW3:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x00, "SW3:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x00, "SW3:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x00, "SW3:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x00, "SW3:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x00, "SW3:8")
INPUT_PORTS_END

static INPUT_PORTS_START( mjyarou )
	PORT_INCLUDE( royalmah )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( tontonb )
	PORT_INCLUDE( mjctrl2 )

	PORT_START("DSW1")  // DSW1 (inport $10 -> 0x73b0)
	PORT_DIPNAME( 0x0f, 0x0f, "Pay Out Rate" )
	PORT_DIPSETTING(    0x0f, "96%" )
	PORT_DIPSETTING(    0x0e, "93%" )
	PORT_DIPSETTING(    0x0d, "90%" )
	PORT_DIPSETTING(    0x0c, "87%" )
	PORT_DIPSETTING(    0x0b, "84%" )
	PORT_DIPSETTING(    0x0a, "81%" )
	PORT_DIPSETTING(    0x09, "78%" )
	PORT_DIPSETTING(    0x08, "75%" )
	PORT_DIPSETTING(    0x07, "71%" )
	PORT_DIPSETTING(    0x06, "68%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x04, "62%" )
	PORT_DIPSETTING(    0x03, "59%" )
	PORT_DIPSETTING(    0x02, "56%" )
	PORT_DIPSETTING(    0x01, "53%" )
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPNAME( 0x30, 0x30, "Maximum Bet" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x20, "10" )
	PORT_DIPSETTING(    0x30, "20" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )      // affects videoram - flip screen ?
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Debug Mode ?" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")  // DSW3 (inport $47 -> 0x73b1)
	PORT_DIPNAME( 0x03, 0x03, "Winnings" )          // check code at 0x0e6d
	PORT_DIPSETTING(    0x00, "32 24 16 12 8 4 2 1" )   // table at 0x4e7d
	PORT_DIPSETTING(    0x03, "50 30 15 8 5 3 2 1" )    // table at 0x4e4d
	PORT_DIPSETTING(    0x02, "100 50 25 10 5 3 2 1" )  // table at 0x4e5d
	PORT_DIPSETTING(    0x01, "200 100 50 10 5 3 2 1" ) // table at 0x4e6d
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )      // check code at 0x5184
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )      // stores something at 0x76ff
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )      // check code at 0x1482, 0x18c2, 0x1a1d, 0x1a83, 0x2d2f and 0x2d85
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, "Maximum Payout ?" )      // check code at 0x1ab7
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPSETTING(    0x20, "200" )
	PORT_DIPSETTING(    0x40, "300" )
	PORT_DIPSETTING(    0x60, "500" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )      // check code at 0x18c2, 0x1a1d, 0x2d2f and 0x2d85
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")  // DSW2 (inport $46 -> 0x73b2)
	PORT_DIPNAME( 0x01, 0x00, "Special Combinations" )  // see notes
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )      // check code at 0x07c5
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )      // check code at 0x5375
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )      // check code at 0x5241
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )      // untested ?
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )      // check code at 0x13aa
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Full Tests" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

// TODO: check dip-switches
static INPUT_PORTS_START( makaijan )
	PORT_INCLUDE( mjctrl2 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, "Pay Out Rate" )
	PORT_DIPSETTING(    0x0f, "96%" )
	PORT_DIPSETTING(    0x0e, "93%" )
	PORT_DIPSETTING(    0x0d, "90%" )
	PORT_DIPSETTING(    0x0c, "87%" )
	PORT_DIPSETTING(    0x0b, "84%" )
	PORT_DIPSETTING(    0x0a, "81%" )
	PORT_DIPSETTING(    0x09, "78%" )
	PORT_DIPSETTING(    0x08, "75%" )
	PORT_DIPSETTING(    0x07, "71%" )
	PORT_DIPSETTING(    0x06, "68%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x04, "62%" )
	PORT_DIPSETTING(    0x03, "59%" )
	PORT_DIPSETTING(    0x02, "56%" )
	PORT_DIPSETTING(    0x01, "53%" )
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPNAME( 0x30, 0x30, "Maximum Bet" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x20, "10" )
	PORT_DIPSETTING(    0x30, "20" )
	PORT_DIPNAME( 0x40, 0x00, "Background Color" )
	PORT_DIPSETTING(    0x00, "Green" )
	PORT_DIPSETTING(    0x40, "Black" )
	PORT_DIPNAME( 0x80, 0x80, "Debug Mode ?" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x00, "Special Combinations" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Full Tests" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "DSW2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

// TODO: check dip-switches
static INPUT_PORTS_START( daisyari )
	PORT_INCLUDE( mjctrl2 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, "Pay Out Rate" )
	PORT_DIPSETTING(    0x0f, "96%" )
	PORT_DIPSETTING(    0x0e, "93%" )
	PORT_DIPSETTING(    0x0d, "90%" )
	PORT_DIPSETTING(    0x0c, "87%" )
	PORT_DIPSETTING(    0x0b, "84%" )
	PORT_DIPSETTING(    0x0a, "81%" )
	PORT_DIPSETTING(    0x09, "78%" )
	PORT_DIPSETTING(    0x08, "75%" )
	PORT_DIPSETTING(    0x07, "71%" )
	PORT_DIPSETTING(    0x06, "68%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x04, "62%" )
	PORT_DIPSETTING(    0x03, "59%" )
	PORT_DIPSETTING(    0x02, "56%" )
	PORT_DIPSETTING(    0x01, "53%" )
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPNAME( 0x30, 0x30, "Maximum Bet" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x20, "10" )
	PORT_DIPSETTING(    0x30, "20" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Full Tests" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "DSW2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x00, "DSW4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( mjclub )
	PORT_INCLUDE( mjctrl2 )

	// On the main board
	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x08, "Pay Out Rate" )  PORT_DIPLOCATION("SW4:1,2,3,4")
	PORT_DIPSETTING(    0x0f, "96%" )
	PORT_DIPSETTING(    0x0e, "93%" )
	PORT_DIPSETTING(    0x0d, "90%" )
	PORT_DIPSETTING(    0x0c, "87%" )
	PORT_DIPSETTING(    0x0b, "84%" )
	PORT_DIPSETTING(    0x0a, "81%" )
	PORT_DIPSETTING(    0x09, "78%" )
	PORT_DIPSETTING(    0x08, "75%" )
	PORT_DIPSETTING(    0x07, "71%" )
	PORT_DIPSETTING(    0x06, "68%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x04, "62%" )
	PORT_DIPSETTING(    0x03, "59%" )
	PORT_DIPSETTING(    0x02, "56%" )
	PORT_DIPSETTING(    0x01, "53%" )
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPNAME( 0x30, 0x20, "Maximum Bet" )   PORT_DIPLOCATION("SW4:5,6")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x20, "10" )
	PORT_DIPSETTING(    0x30, "20" )
	PORT_DIPNAME( 0x40, 0x40, "Note Rate" )     PORT_DIPLOCATION("SW4:7")
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPNAME( 0x80, 0x80, "Data Display" )  PORT_DIPLOCATION("SW4:8")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	// On the subboard
	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x00, "Game Type" )                     PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x00, "A" )
	PORT_DIPSETTING(    0x02, "B" )
	PORT_DIPSETTING(    0x01, "C" )
	PORT_DIPSETTING(    0x03, "D" )
	PORT_DIPUNUSED_DIPLOC( 0x0c, 0x08, "SW2:3,4" ) PORT_CONDITION("DSW3", 0x03, EQUALS, 0x00)
	PORT_DIPUNUSED_DIPLOC( 0x0c, 0x08, "SW2:3,4" ) PORT_CONDITION("DSW3", 0x03, EQUALS, 0x02)
	PORT_DIPNAME( 0x0c, 0x08, "Bonus Rate (3renchan bonus)" )   PORT_DIPLOCATION("SW2:3,4") PORT_CONDITION("DSW3", 0x03, EQUALS, 0x01)
	PORT_DIPSETTING(    0x00, "A (1 2 2 3 pts.)" )
	PORT_DIPSETTING(    0x04, "B (1 2 2 5 pts.)" )
	PORT_DIPSETTING(    0x08, "C (1 2 3 6 pts.)" )
	PORT_DIPSETTING(    0x0c, "D (1 2 6 10 pts.)" )
	PORT_DIPNAME( 0x0c, 0x08, "Bonus Rate (5renchan bonus)" )   PORT_DIPLOCATION("SW2:3,4") PORT_CONDITION("DSW3", 0x03, EQUALS, 0x03)
	PORT_DIPSETTING(    0x00, "A (5 pts.)" )
	PORT_DIPSETTING(    0x04, "B (10 pts.)" )
	PORT_DIPSETTING(    0x08, "C (15 pts.)" )
	PORT_DIPSETTING(    0x0c, "D (20 pts.)" )
	PORT_DIPNAME( 0x30, 0x00, "CPU Houjuu Pattern" )            PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x00, "100% Free" )
	PORT_DIPSETTING(    0x10, "75% Free" )
	PORT_DIPSETTING(    0x20, "50% Free" )
	PORT_DIPSETTING(    0x30, "25% Free" )
	PORT_DIPNAME( 0x40, 0x00, "Payout Rate Autochange" )        PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Test Mode" )                     PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	// On the subboard
	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "Double Odds Bonus" )             PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, "Extra Bet" )                     PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x00, "Color Hai Bonus" )               PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x00, "Sangenhai Bonus" )               PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPUNUSED_DIPLOC( 0x70, 0x00, "SW3:5,6,7" )
	PORT_DIPNAME( 0x80, 0x00, "Coin Needed for Last Chance" )   PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	// On the subboard
	PORT_START("DSW4")
	PORT_DIPNAME( 0x03, 0x00, "Odds Rate" )             PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, "A (50 30 15 8 5 4 3 2)" )
	PORT_DIPSETTING(    0x02, "B (100 40 20 10 5 4 3 2)" )
	PORT_DIPSETTING(    0x01, "C (150 70 30 10 5 4 3 2)" )
	PORT_DIPSETTING(    0x00, "D (32 24 16 12 8 4 2 1)" )
	PORT_DIPNAME( 0x3c, 0x3c, "Bonus Awarded at:" )     PORT_DIPLOCATION("SW1:3,4,5,6")
	PORT_DIPSETTING(    0x00, "1st Time Only" )
	PORT_DIPSETTING(    0x20, "200 Coins" )
	PORT_DIPSETTING(    0x10, "300 Coins" )
	PORT_DIPSETTING(    0x30, "400 Coins" )
	PORT_DIPSETTING(    0x08, "500 Coins" )
	PORT_DIPSETTING(    0x28, "600 Coins" )
	PORT_DIPSETTING(    0x18, "700 Coins" )
	PORT_DIPSETTING(    0x38, "1000 Coins" )
	PORT_DIPSETTING(    0x3c, "Never" )
	PORT_DIPNAME( 0x40, 0x40, "Bonus Occurrence" )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, "Once" )
	PORT_DIPSETTING(    0x40, "Twice" )
	PORT_DIPNAME( 0x80, 0x80, "Background Color" )      PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, "White" )     // Black according to manual
	PORT_DIPSETTING(    0x80, "Green" )
INPUT_PORTS_END

static INPUT_PORTS_START( mjdiplob )
	PORT_INCLUDE( mjctrl2 )

	PORT_START("DSW1")  // DSW1 (inport $10 -> 0x76fa)
	PORT_DIPNAME( 0x0f, 0x0f, "Pay Out Rate" ) PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x0f, "96%" )
	PORT_DIPSETTING(    0x0e, "93%" )
	PORT_DIPSETTING(    0x0d, "90%" )
	PORT_DIPSETTING(    0x0c, "87%" )
	PORT_DIPSETTING(    0x0b, "84%" )
	PORT_DIPSETTING(    0x0a, "81%" )
	PORT_DIPSETTING(    0x09, "78%" )
	PORT_DIPSETTING(    0x08, "75%" )
	PORT_DIPSETTING(    0x07, "71%" )
	PORT_DIPSETTING(    0x06, "68%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x04, "62%" )
	PORT_DIPSETTING(    0x03, "59%" )
	PORT_DIPSETTING(    0x02, "56%" )
	PORT_DIPSETTING(    0x01, "53%" )
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPNAME( 0x30, 0x30, "Maximum Bet" ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x20, "10" )
	PORT_DIPSETTING(    0x30, "20" )
	PORT_DIPNAME( 0x40, 0x00, "Table Color" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, "Green" )
	PORT_DIPSETTING(    0x40, "Black" )
	PORT_DIPNAME( 0x80, 0x80, "Debug Mode ?" ) PORT_DIPLOCATION("SW1:8")     // check code at 0x0b94 and 0x0de2
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")  // DSW2 (inport $62 -> 0x76fb)
	PORT_DIPNAME( 0x03, 0x03, "Winnings" ) PORT_DIPLOCATION("SW2:1,2")         // check code at 0x09cd
	PORT_DIPSETTING(    0x00, "32 24 16 12 8 4 2 1" )   // table at 0x4b82
	PORT_DIPSETTING(    0x03, "50 30 15 8 5 3 2 1" )    // table at 0x4b52
	PORT_DIPSETTING(    0x02, "100 50 25 10 5 3 2 1" )  // table at 0x4b62
	PORT_DIPSETTING(    0x01, "200 100 50 10 5 3 2 1" ) // table at 0x4b72
	PORT_DIPNAME( 0x04, 0x00, "Yakuman Bonus" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Yakuman Bonus Period Control" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x08, "First time only" )
	PORT_DIPNAME( 0x30, 0x30, "Yakuman Bonus Timing Control" ) PORT_DIPLOCATION("SW2:5,6")      // check code at 0x166c, Yakuman Bonus every x coins in
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPSETTING(    0x10, "200" )
	PORT_DIPSETTING(    0x20, "300" )
	PORT_DIPSETTING(    0x30, "500" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:7")      // check code at 0x2c64, unused according to manual
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:8")     // check code at 0x2c64, unused according to manual
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW3")  // DSW3 (inport $63 -> 0x76fc), switches 5-7 could do with verifying by someone who understands Japanese and mahjong (see MT05553 for manual)
	PORT_DIPNAME( 0x01, 0x00, "Baibai Bonus" ) PORT_DIPLOCATION("SW3:1")  // see notes about 'Special Combinations'
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Don Den" ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "W-Bet" ) PORT_DIPLOCATION("SW3:3")      // check code at 0x531f and 0x5375
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Last Chance" ) PORT_DIPLOCATION("SW3:4")     // check code at 0x5240
	PORT_DIPSETTING(    0x08, "Free" )
	PORT_DIPSETTING(    0x00, "Charge" )
	PORT_DIPNAME( 0x10, 0x00, "Renchan Rate" ) PORT_DIPLOCATION("SW3:5")      // check code at 0x2411
	PORT_DIPSETTING(    0x00, "Good" )
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPNAME( 0x20, 0x00, "Renchan" ) PORT_DIPLOCATION("SW3:6")      // check code at 0x2411 and 0x4beb
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Auto Tsumo?" ) PORT_DIPLOCATION("SW3:7")      // check code at 0x24ff, 0x25f2, 0x3fcf and 0x45d7
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Full Tests" ) PORT_DIPLOCATION("SW3:8")           // seems to hang after the last animation, unused according to manual
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( mjderngr )
	PORT_INCLUDE( mjctrl2 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "ROM & Animation Test" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( majs101b )
	PORT_INCLUDE( mjctrl2 )

	PORT_START("DSW1")  // DSW1 (inport $10 -> 0x76fd)
	PORT_DIPNAME( 0x0f, 0x0f, "Pay Out Rate" )
	PORT_DIPSETTING(    0x0f, "96%" )
	PORT_DIPSETTING(    0x0e, "93%" )
	PORT_DIPSETTING(    0x0d, "90%" )
	PORT_DIPSETTING(    0x0c, "87%" )
	PORT_DIPSETTING(    0x0b, "84%" )
	PORT_DIPSETTING(    0x0a, "81%" )
	PORT_DIPSETTING(    0x09, "78%" )
	PORT_DIPSETTING(    0x08, "75%" )
	PORT_DIPSETTING(    0x07, "71%" )
	PORT_DIPSETTING(    0x06, "68%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x04, "62%" )
	PORT_DIPSETTING(    0x03, "59%" )
	PORT_DIPSETTING(    0x02, "56%" )
	PORT_DIPSETTING(    0x01, "53%" )
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPNAME( 0x30, 0x30, "Maximum Bet" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x20, "10" )
	PORT_DIPSETTING(    0x30, "20" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Debug Mode ?" )      // check code at 0x1635
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")  // DSW2 (inport $00 (after out 0,$40) -> 0x76fa)
	PORT_DIPNAME( 0x03, 0x03, "Winnings" )          // check code at 0x14e4
	PORT_DIPSETTING(    0x00, "32 24 16 12 8 4 2 1" )   // table at 0x1539
	PORT_DIPSETTING(    0x03, "50 30 15 8 5 3 2 1" )    // table at 0x1509
	PORT_DIPSETTING(    0x02, "100 50 25 10 5 3 2 1" )  // table at 0x1519
	PORT_DIPSETTING(    0x01, "200 100 50 10 5 3 2 1" ) // table at 0x1529
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )      // check code at 0x1220, 0x128d, 0x13b1, 0x13cb and 0x2692
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x38, 0x00, "Maximum Payout ?" )      // check code at 0x12c1
	PORT_DIPSETTING(    0x20, "200" )
	PORT_DIPSETTING(    0x10, "300" )
	PORT_DIPSETTING(    0x30, "400" )
	PORT_DIPSETTING(    0x08, "500" )
	PORT_DIPSETTING(    0x28, "600" )
	PORT_DIPSETTING(    0x18, "700" )
	PORT_DIPSETTING(    0x00, "1000" )
//  PORT_DIPSETTING(    0x38, "1000" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )      // check code at 0x1333
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Background" )
	PORT_DIPSETTING(    0x00, "Black" )
	PORT_DIPSETTING(    0x80, "Gray" )

	PORT_START("DSW3")  // DSW3 (inport $00 (after out 0,$00) -> 0x76fc)
	PORT_DIPNAME( 0x01, 0x00, "Special Combinations" )  // see notes
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )      // check code at 0x1cf9
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )      // check code at 0x21a9, 0x21dc and 0x2244
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )      // check code at 0x2b7f
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )      // check code at 0x50ba
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )      // check code at 0x1f65
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )      // check code at 0x6412
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      // check code at 0x2cb2 and 0x2d02
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW4")  // DSW4 (inport $00 (after out 0,$20) -> 0x76fb)
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Unknown ) )      // stored at 0x702f - check code at 0x1713,
	PORT_DIPSETTING(    0x00, "0" )             // 0x33d1, 0x3408, 0x3415, 0x347c, 0x3492, 0x350d,
	PORT_DIPSETTING(    0x01, "1" )             // 0x4af9, 0x4b1f and 0x61f6
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPNAME( 0x0c, 0x00, "Difficulty ?" )      // check code at 0x4b5c and 0x6d72
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )             // 0x05 - 0x03, 0x02, 0x02, 0x01
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )           // 0x0a - 0x05, 0x02, 0x02, 0x01
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )             // 0x0f - 0x06, 0x03, 0x02, 0x01
	PORT_DIPSETTING(    0x0c, DEF_STR( Hardest ) )          // 0x14 - 0x0a, 0x06, 0x02, 0x01
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Unknown ) )      // check code at 0x228e
	PORT_DIPSETTING(    0x00, "0x00" )
	PORT_DIPSETTING(    0x10, "0x10" )
	PORT_DIPSETTING(    0x20, "0x20" )
	PORT_DIPSETTING(    0x30, "0x30" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )      // check code at 0x11e4
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Full Tests" )            // check code at 0x006d
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( mjapinky )
	PORT_INCLUDE( mjctrl2 )

	PORT_START("DSW1")  // IN11
	PORT_DIPNAME( 0x01, 0x01, "Unknown 1-0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 1-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 1-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 1-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, "Maximum Bet" )
	PORT_DIPSETTING(    0x30, "1" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x40, 0x00, "Background" )
	PORT_DIPSETTING(    0x40, "Black" )
	PORT_DIPSETTING(    0x00, "Green" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 1-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")  // IN12
	PORT_DIPNAME( 0x03, 0x03, "Unknown 2-0&1" )
	PORT_DIPSETTING(    0x03, "0" )
	PORT_DIPSETTING(    0x02, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 2-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 2-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, "Unknown 2-4&5" )
	PORT_DIPSETTING(    0x30, "0" )
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0xc0, 0xc0, "Unknown 2-6&7" )
	PORT_DIPSETTING(    0xc0, "0" )
	PORT_DIPSETTING(    0x80, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0x00, "3" )

	PORT_START("DSW3")  // IN13
	PORT_DIPNAME( 0x01, 0x01, "Unknown 3-0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 3-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 3-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 3-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 3-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 3-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 3-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Full Test" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( janptr96 )
	PORT_INCLUDE( mjctrl1 )

	PORT_START("DSW4")  // IN11
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "In Game Music" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Girls (Demo)" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Don Den Key" )
	PORT_DIPSETTING(    0x80, "Start" )
	PORT_DIPSETTING(    0x00, "Flip/Flop" )

	PORT_START("DSW3")  // IN12
	PORT_DIPNAME( 0x07, 0x07, "YAKUMAN Bonus" )
	PORT_DIPSETTING(    0x07, "Cut" )
	PORT_DIPSETTING(    0x06, "1 T" )
	PORT_DIPSETTING(    0x05, "300" )
	PORT_DIPSETTING(    0x04, "500" )
	PORT_DIPSETTING(    0x03, "700" )
	PORT_DIPSETTING(    0x02, "1000" )
	PORT_DIPSETTING(    0x01, "1000?" )
	PORT_DIPSETTING(    0x00, "1000?" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x30, "0" )
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")  // IN13
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x0c, 0x0c, "Min Credits To Start" )
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x30, 0x30, "Payout" )
	PORT_DIPSETTING(    0x30, "300" )
	PORT_DIPSETTING(    0x20, "500" )
	PORT_DIPSETTING(    0x10, "700" )
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPNAME( 0x40, 0x40, "W-BET" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Last Chance" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")  // IN14
	PORT_DIPNAME( 0x0f, 0x0f, "Pay Out Rate" )
	PORT_DIPSETTING(    0x0f, "96%" )
	PORT_DIPSETTING(    0x0e, "93%" )
	PORT_DIPSETTING(    0x0d, "90%" )
	PORT_DIPSETTING(    0x0c, "87%" )
	PORT_DIPSETTING(    0x0b, "84%" )
	PORT_DIPSETTING(    0x0a, "81%" )
	PORT_DIPSETTING(    0x09, "78%" )
	PORT_DIPSETTING(    0x08, "75%" )
	PORT_DIPSETTING(    0x07, "71%" )
	PORT_DIPSETTING(    0x06, "68%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x04, "62%" )
	PORT_DIPSETTING(    0x03, "59%" )
	PORT_DIPSETTING(    0x02, "56%" )
	PORT_DIPSETTING(    0x01, "53%" )
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPNAME( 0x30, 0x30, "Odds Rate" )
	PORT_DIPSETTING(    0x30, "1 2 4 8 12 16 24 32" )
	PORT_DIPSETTING(    0x00, "1 2 3 5 8 15 30 50" )
	PORT_DIPSETTING(    0x20, "2 3 6 8 12 15 30 50" )
	PORT_DIPSETTING(    0x10, "1 2 3 5 10 25 50 100" )
	PORT_DIPNAME( 0xc0, 0xc0, "Maximum Bet" )
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0x80, "5" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x00, "20" )

	PORT_START("DSWTOP")    // IN15
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Debug Mode" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Credits Per Note" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( mjifb )
	PORT_INCLUDE( mjctrl2 )

	// Default dip-switch settings taken from manual for "single type" cabinet
	// When they differ for "corner type" cabinet, it's noted in a comment near the dip-switch

	PORT_START("PORT3_5")   // IN10 - DSW1 (P3 & P5)
	PORT_DIPNAME( 0x0f, 0x07, "Pay Out Rate" ) PORT_DIPLOCATION("DSW1:1,2,3,4")
	PORT_DIPSETTING(    0x0f, "96%" )
	PORT_DIPSETTING(    0x0e, "93%" )
	PORT_DIPSETTING(    0x0d, "90%" )
	PORT_DIPSETTING(    0x0c, "87%" )
	PORT_DIPSETTING(    0x0b, "84%" )
	PORT_DIPSETTING(    0x0a, "81%" )
	PORT_DIPSETTING(    0x09, "78%" )
	PORT_DIPSETTING(    0x08, "75%" )
	PORT_DIPSETTING(    0x07, "71%" )
	PORT_DIPSETTING(    0x06, "68%" ) // default for "corner type" cabinet
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x04, "62%" )
	PORT_DIPSETTING(    0x03, "59%" )
	PORT_DIPSETTING(    0x02, "56%" )
	PORT_DIPSETTING(    0x01, "53%" )
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPNAME( 0x30, 0x10, "Maximum Bet" ) PORT_DIPLOCATION("DSW1:5,6")
	PORT_DIPSETTING(    0x30, "1" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0xc0, 0xc0, "Minimum Rate to Play" ) PORT_DIPLOCATION("DSW1:7,8")
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPSETTING(    0x00, "4" )

	PORT_START("PORT6_7")   // IN11 - DSW2 (P6 & P7)
	PORT_DIPNAME( 0x03, 0x00, "Winnings" ) PORT_DIPLOCATION("DSW2:1,2")
	PORT_DIPSETTING(    0x03, "32 24 16 12 8 4 2 1" )
	PORT_DIPSETTING(    0x00, "50 30 15 8 5 3 2 1" )
	PORT_DIPSETTING(    0x01, "100 50 25 10 5 3 2 1" )
	PORT_DIPSETTING(    0x02, "200 100 50 10 5 3 2 1" )
	PORT_DIPNAME( 0x04, 0x00, "Credits Per Note" ) PORT_DIPLOCATION("DSW2:3")
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x38, 0x20, "YAKUMAN Bonus" ) PORT_DIPLOCATION("DSW2:4,5,6")
	PORT_DIPSETTING(    0x38, "None" ) // default for "corner type" cabinet
	PORT_DIPSETTING(    0x30, "Once at start" )
	PORT_DIPSETTING(    0x28, "Every 300 coins" )
	PORT_DIPSETTING(    0x20, "Every 500 coins" )
	PORT_DIPSETTING(    0x18, "Every 700 coins" )
	PORT_DIPSETTING(    0x10, "Every 1000 coins" )
//  PORT_DIPSETTING(    0x08, "Every 1000 coins?" )
//  PORT_DIPSETTING(    0x00, "Every 1000 coins?" )
	PORT_DIPNAME( 0x40, 0x40, "YAKUMAN Bonus Frequency" ) PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x00, "Once" )
	PORT_DIPSETTING(    0x40, "Twice" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hard ) )

	PORT_START("DSW3")  // IN13 - DSW3 ($8200)
	PORT_DIPNAME( 0x01, 0x00, "Automatic Rate Change" ) PORT_DIPLOCATION("DSW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) ) // default for "corner type" cabinet
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Service Count" ) PORT_DIPLOCATION("DSW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) ) // default for "corner type" cabinet
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Renchan Rate" ) PORT_DIPLOCATION("DSW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "W-Bet" ) PORT_DIPLOCATION("DSW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Bye-Bye Bonus" ) PORT_DIPLOCATION("DSW3:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Last Chance" ) PORT_DIPLOCATION("DSW3:6")
	PORT_DIPSETTING(    0x20, "Free" )
	PORT_DIPSETTING(    0x00, "Charge" )
	PORT_DIPNAME( 0x40, 0x00, "Auto Tsumo" ) PORT_DIPLOCATION("DSW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Background Color" ) PORT_DIPLOCATION("DSW3:8")
	PORT_DIPSETTING(    0x80, "Black" )
	PORT_DIPSETTING(    0x00, "Blue" ) // gray according to manual?!?!

	PORT_START("DSW4")  // IN14 - DSW4 ($8000)
	PORT_DIPNAME( 0x01, 0x00, "Double-tile exchange" ) PORT_DIPLOCATION("DSW4:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Double hand-tile game" ) PORT_DIPLOCATION("DSW4:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Blind-tile game" ) PORT_DIPLOCATION("DSW4:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Don Den Button" ) PORT_DIPLOCATION("DSW4:4")
	PORT_DIPSETTING(    0x08, "F/F Button" )
	PORT_DIPSETTING(    0x00, "A Button" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("DSW4:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) // default for "corner type" cabinet
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("DSW4:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Animation" )  PORT_DIPLOCATION("DSW4:7") // marked as "unused" in manual
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Full Tests" ) PORT_DIPLOCATION("DSW4:8") // marked as "unused" in manual
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( mjtensin )
	PORT_INCLUDE( mjctrl1 )

	PORT_START("DSW4")  // IN11
	PORT_DIPNAME( 0x0f, 0x0f, "Pay Out Rate" )
	PORT_DIPSETTING(    0x0f, "96%" )
	PORT_DIPSETTING(    0x0e, "93%" )
	PORT_DIPSETTING(    0x0d, "90%" )
	PORT_DIPSETTING(    0x0c, "87%" )
	PORT_DIPSETTING(    0x0b, "84%" )
	PORT_DIPSETTING(    0x0a, "81%" )
	PORT_DIPSETTING(    0x09, "78%" )
	PORT_DIPSETTING(    0x08, "75%" )
	PORT_DIPSETTING(    0x07, "71%" )
	PORT_DIPSETTING(    0x06, "68%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x04, "62%" )
	PORT_DIPSETTING(    0x03, "59%" )
	PORT_DIPSETTING(    0x02, "56%" )
	PORT_DIPSETTING(    0x01, "53%" )
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPNAME( 0x30, 0x30, "Odds Rate" )
	PORT_DIPSETTING(    0x30, "1 2 4 8 12 16 24 32" )
	PORT_DIPSETTING(    0x00, "1 2 3 5 8 15 30 50" )
	PORT_DIPSETTING(    0x20, "2 3 6 8 12 15 30 50" )
	PORT_DIPSETTING(    0x10, "1 2 3 5 10 25 50 100" )
	PORT_DIPNAME( 0xc0, 0xc0, "Maximum Bet" )
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0x80, "5" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x00, "20" )

	PORT_START("DSW3")  // IN12
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x0c, 0x0c, "Min Credits To Start" )
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x30, 0x30, "Payout" )
	PORT_DIPSETTING(    0x30, "300" )
	PORT_DIPSETTING(    0x20, "500" )
	PORT_DIPSETTING(    0x10, "700" )
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPNAME( 0x40, 0x40, "W-BET" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Last Chance" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")  // IN13
	PORT_DIPNAME( 0x03, 0x03, "YAKUMAN Bonus" )
	PORT_DIPSETTING(    0x03, "Cut" )
	PORT_DIPSETTING(    0x02, "500" )
	PORT_DIPSETTING(    0x01, "700" )
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x18, "0" )
	PORT_DIPSETTING(    0x10, "1" )
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x60, "0" )
	PORT_DIPSETTING(    0x40, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")  // IN14
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "In Game Music" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Girls (Demo)" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Show Clock" )
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("DSWTOP")    // IN15
	PORT_DIPNAME( 0x01, 0x01, "Credits Per Note" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( cafetime ) // dips definitions and defaults taken from MT05580, this uses 10 switch dip banks
	PORT_INCLUDE( mjctrl2 )

	PORT_START("DSW1")  // IN11
	PORT_DIPNAME( 0x0f, 0x07, "Pay Out Rate" ) PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x0f, "96%" )
	PORT_DIPSETTING(    0x0e, "93%" )
	PORT_DIPSETTING(    0x0d, "90%" )
	PORT_DIPSETTING(    0x0c, "87%" )
	PORT_DIPSETTING(    0x0b, "84%" )
	PORT_DIPSETTING(    0x0a, "81%" )
	PORT_DIPSETTING(    0x09, "78%" )
	PORT_DIPSETTING(    0x08, "75%" )
	PORT_DIPSETTING(    0x07, "71%" )
	PORT_DIPSETTING(    0x06, "68%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x04, "62%" )
	PORT_DIPSETTING(    0x03, "59%" )
	PORT_DIPSETTING(    0x02, "56%" )
	PORT_DIPSETTING(    0x01, "53%" )
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPNAME( 0x30, 0x00, "Odds Rate" ) PORT_DIPLOCATION("SW1:5,6") // Yakuman - Triple - Double - Haneman - Mangan - 3 Han - 2 Han - 1 Han
	PORT_DIPSETTING(    0x30, "32-24-16-12-8-4-2-1" )
	PORT_DIPSETTING(    0x00, "50-30-15-8-5-3-2-1" )
	PORT_DIPSETTING(    0x10, "100-50-25-10-5-3-2-1" )
	PORT_DIPSETTING(    0x20, "200-100-50-10-5-3-2-1" )
	PORT_DIPNAME( 0xc0, 0x40, "Maximum Bet" ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0x80, "5" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x00, "20" )

	PORT_START("DSW2")  // IN12, defaults for 'single type'
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, "1C_10C" )
	PORT_DIPNAME( 0x0c, 0x0c, "Minimum Rate" ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x70, 0x40, "Yakuman Bonus" ) PORT_DIPLOCATION("SW2:5,6,7") // default for 'corner type' would be 0x20
	PORT_DIPSETTING(    0x70, DEF_STR( No ) )
	PORT_DIPSETTING(    0x60, "Once on Start" )
	PORT_DIPSETTING(    0x50, "Every 300 Coins" )
	PORT_DIPSETTING(    0x40, "Every 500 Coins" )
	PORT_DIPSETTING(    0x30, "Every 700 Coins" )
	PORT_DIPSETTING(    0x20, "Every 1000 Coins" )
	PORT_DIPSETTING(    0x10, DEF_STR( Unknown ) ) // not listed on dip sheet
	PORT_DIPSETTING(    0x00, DEF_STR( Unknown ) ) // not listed on dip sheet
	PORT_DIPNAME( 0x80, 0x00, "Yakuman Bonus Cycle" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, "Once" )
	PORT_DIPSETTING(    0x80, "Twice" )

	PORT_START("DSW3")  // IN13
	PORT_DIPNAME( 0x01, 0x01, "Payout Type" ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, "Credits" )
	PORT_DIPSETTING(    0x00, "Hopper" )
	PORT_DIPNAME( 0x02, 0x02, "Hopper Type" ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Reversed" )
	PORT_DIPNAME( 0x04, 0x00, "W-Bet" ) PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Renchan Rate" ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Last Chance" ) PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Good Time Timer" ) PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x00, "3 Minutes" )
	PORT_DIPSETTING(    0x20, "5 Minutes" )
	PORT_DIPNAME( 0x40, 0x00, "Quiz Bonus" ) PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Quiz Bonus Points" ) PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x80, "3" )

	PORT_START("DSW4")  // IN14, defaults for 'single type'
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW4:1") // Should be off for 'single type' and on for 'corner type', left on for easier regression testing
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "BGM during Gameplay" ) PORT_DIPLOCATION("SW4:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Don Den Button" )  PORT_DIPLOCATION("SW4:3") // Off for 'corner type'
	PORT_DIPSETTING(    0x04, "Start Button" )
	PORT_DIPSETTING(    0x00, "F/F Button" )
	PORT_DIPNAME( 0x08, 0x00, "Auto Reach" ) PORT_DIPLOCATION("SW4:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Full Test" )  PORT_DIPLOCATION("SW4:5") // 4 koma comic test
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Background" ) PORT_DIPLOCATION("SW4:6")
	PORT_DIPSETTING(    0x20, "Black" )
	PORT_DIPSETTING(    0x00, "Green" )
	PORT_DIPNAME( 0x40, 0x40, "Book (Stage) Select" ) PORT_DIPLOCATION("SW4:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Yakuman Match" ) PORT_DIPLOCATION("SW4:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWTOP")    // IN15, actually dips 9-10 of the first 4 banks
	PORT_DIPNAME( 0x01, 0x00, "Credits Per Note" ) PORT_DIPLOCATION("SW1:9")
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:10")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:9")
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:10") // listed as Unused OFF
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, "Quiz Questions Table" ) PORT_DIPLOCATION("SW3:9,10")
	PORT_DIPSETTING(    0x30, "A" )
	PORT_DIPSETTING(    0x20, "B" )
	PORT_DIPSETTING(    0x10, "C" )
	PORT_DIPSETTING(    0x00, "D" )
	PORT_DIPNAME( 0x40, 0x40, "Yakuman Match Frequency" ) PORT_DIPLOCATION("SW4:9")
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, "Often" )
	PORT_DIPNAME( 0x80, 0x80, "4 Koma Comic Type" ) PORT_DIPLOCATION("SW4:10")
	PORT_DIPSETTING(    0x00, "A" )
	PORT_DIPSETTING(    0x80, "B" )
INPUT_PORTS_END

static INPUT_PORTS_START( ippatsu )
	PORT_INCLUDE( mjctrl1 )

	PORT_MODIFY("KEY5")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )  // "COIN2"
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3 )  // "COIN1", but not working


	PORT_START("DSW1") // DSW  (inport $10)
	PORT_DIPNAME( 0x0f, 0x0f, "Pay Out Rate" )
	PORT_DIPSETTING(    0x0f, "96%" )
	PORT_DIPSETTING(    0x0e, "93%" )
	PORT_DIPSETTING(    0x0d, "90%" )
	PORT_DIPSETTING(    0x0c, "87%" )
	PORT_DIPSETTING(    0x0b, "84%" )
	PORT_DIPSETTING(    0x0a, "81%" )
	PORT_DIPSETTING(    0x09, "78%" )
	PORT_DIPSETTING(    0x08, "75%" )
	PORT_DIPSETTING(    0x07, "71%" )
	PORT_DIPSETTING(    0x06, "68%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x04, "62%" )
	PORT_DIPSETTING(    0x03, "59%" )
	PORT_DIPSETTING(    0x02, "56%" )
	PORT_DIPSETTING(    0x01, "53%" )
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, "Maximum Bet" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x40, "7" )
	PORT_DIPSETTING(    0x60, "10" )
	PORT_DIPNAME( 0x80, 0x80, "First Chance" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )

	PORT_START("DSW2")  // DSW  (inport $12)
	PORT_DIPNAME( 0x03, 0x03, "Cut" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, "2 Bai" )
	PORT_DIPSETTING(    0x01, "3 Bai" )
	PORT_DIPSETTING(    0x03, "Yakuman" )
	PORT_DIPNAME( 0x0c, 0x0c, "Yakuman Bonus" )
	PORT_DIPSETTING(    0x00, "32" )
	PORT_DIPSETTING(    0x04, "100" )
	PORT_DIPSETTING(    0x08, "200" )
	PORT_DIPSETTING(    0x0c, "300" )
	PORT_DIPNAME( 0x30, 0x30, "Unknown 1-4&5*" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x30, "6" )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 1-6*" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 1-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")  // DSW  (inport $13)
	PORT_DIPNAME( 0x01, 0x01, "Unknown 2-0*" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "Second Bonus" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x04, "Allow Bets" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x18, 0x18, "Unknown 2-3&4*" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x18, "2" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x08, "8" )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 2-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 2-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 2-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( mjdejavu )
	PORT_INCLUDE( mjctrl2 )

	PORT_START("PORT3_5")   // IN11 - DSW3 (P3 & P5)
	PORT_DIPNAME( 0x03, 0x03, "Unknown 3-0&1*" )
	PORT_DIPSETTING(    0x00, "1 1" )
	PORT_DIPSETTING(    0x02, "3 4" )
	PORT_DIPSETTING(    0x01, "1 2" )
	PORT_DIPSETTING(    0x03, "1 4" )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 3-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 3-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 3-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 3-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 3-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 3-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("PORT6_7")   // IN12 - DSW4 (P6 & P7)
	PORT_DIPNAME( 0x01, 0x01, "Unknown 4-0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 4-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 4-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 4-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 4-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 4-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 4-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Full Tests" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")  // IN13 - DSW1 ($8001)
	PORT_DIPNAME( 0x0f, 0x07, "Pay Out Rate" )
	PORT_DIPSETTING(    0x0f, "96%" )
	PORT_DIPSETTING(    0x0e, "93%" )
	PORT_DIPSETTING(    0x0d, "90%" )
	PORT_DIPSETTING(    0x0c, "87%" )
	PORT_DIPSETTING(    0x0b, "84%" )
	PORT_DIPSETTING(    0x0a, "81%" )
	PORT_DIPSETTING(    0x09, "78%" )
	PORT_DIPSETTING(    0x08, "75%" )
	PORT_DIPSETTING(    0x07, "71%" )
	PORT_DIPSETTING(    0x06, "68%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x04, "62%" )
	PORT_DIPSETTING(    0x03, "59%" )
	PORT_DIPSETTING(    0x02, "56%" )
	PORT_DIPSETTING(    0x01, "53%" )
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPNAME( 0x30, 0x30, "Maximum Bet" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x20, "10" )
	PORT_DIPSETTING(    0x30, "20" )
	PORT_DIPNAME( 0x40, 0x40, "Credits Per Note" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPNAME( 0x80, 0x80, "Background" )
	PORT_DIPSETTING(    0x80, "Gray" )
	PORT_DIPSETTING(    0x00, "Black" )

	PORT_START("DSW2")  // IN14 - DSW2 ($8000)
	PORT_DIPNAME( 0x03, 0x03, "Winnings" )
	PORT_DIPSETTING(    0x00, "32 24 16 12 8 4 2 1" )
	PORT_DIPSETTING(    0x03, "50 30 15 8 5 3 2 1" )
	PORT_DIPSETTING(    0x02, "100 50 25 10 5 3 2 1" )
	PORT_DIPSETTING(    0x01, "200 100 50 10 5 3 2 1" )
	PORT_DIPNAME( 0x3c, 0x3c, "YAKUMAN Bonus" )
	PORT_DIPSETTING(    0x3c, "Cut" )
	PORT_DIPSETTING(    0x20, "300" )
	PORT_DIPSETTING(    0x10, "500" )
	PORT_DIPSETTING(    0x08, "700" )
	PORT_DIPSETTING(    0x00, "1000" )
//  PORT_DIPSETTING(    0x04, "1000?" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, "1/4" )
	PORT_DIPSETTING(    0x80, "2/4" )
INPUT_PORTS_END

static INPUT_PORTS_START( jansou )
	PORT_INCLUDE( mjctrl2 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x00, "Pay Out Rate" )  PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x0f, "50%" )
	PORT_DIPSETTING(    0x0e, "53%" )
	PORT_DIPSETTING(    0x0d, "56%" )
	PORT_DIPSETTING(    0x0c, "59%" )
	PORT_DIPSETTING(    0x0b, "62%" )
	PORT_DIPSETTING(    0x0a, "65%" )
	PORT_DIPSETTING(    0x09, "68%" )
	PORT_DIPSETTING(    0x08, "71%" )
	PORT_DIPSETTING(    0x07, "74%" )
	PORT_DIPSETTING(    0x06, "77%" )
	PORT_DIPSETTING(    0x05, "80%" )
	PORT_DIPSETTING(    0x04, "83%" )
	PORT_DIPSETTING(    0x03, "86%" )
	PORT_DIPSETTING(    0x02, "89%" )
	PORT_DIPSETTING(    0x01, "92%" )
	PORT_DIPSETTING(    0x00, "95%" )
	PORT_DIPNAME( 0x30, 0x00, "Maximum Bet" )   PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x30, "1" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 1-7" )   PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 1-8" )   PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x00, "First Chance" )  PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( None ) )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x0c, 0x00, "Last Chance" )   PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, DEF_STR( None ) )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 2-5" )   PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 2-6" )   PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 2-7" )   PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Middle Chance" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "Unknown 3-1" )   PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 3-2" )   PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 3-3" )   PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 3-4" )   PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 3-5" )   PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Girl" )          PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 3-7" )   PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 3-8" )   PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( mjvegasa ) // dips definitions and defaults from manual (machine translated)
	PORT_INCLUDE( mjctrl2 )

	PORT_START("DSW1")  // 6810
	PORT_DIPNAME( 0x0f, 0x07, "Pay Out Rate" ) PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x0f, "96%" )
	PORT_DIPSETTING(    0x0e, "93%" )
	PORT_DIPSETTING(    0x0d, "90%" )
	PORT_DIPSETTING(    0x0c, "87%" )
	PORT_DIPSETTING(    0x0b, "84%" )
	PORT_DIPSETTING(    0x0a, "81%" )
	PORT_DIPSETTING(    0x09, "78%" )
	PORT_DIPSETTING(    0x08, "75%" )
	PORT_DIPSETTING(    0x07, "71%" )
	PORT_DIPSETTING(    0x06, "68%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x04, "62%" )
	PORT_DIPSETTING(    0x03, "59%" )
	PORT_DIPSETTING(    0x02, "56%" )
	PORT_DIPSETTING(    0x01, "53%" )
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPNAME( 0x30, 0x00, "Odds Rate" ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x30, "1 2 4 8 12 16 24 32" )
	PORT_DIPSETTING(    0x00, "1 2 3 5 8 15 30 50" )
	PORT_DIPSETTING(    0x10, "1 2 3 5 10 25 50 100" )
	PORT_DIPSETTING(    0x20, "1 2 3 5 10 50 100 200" )
	PORT_DIPNAME( 0xc0, 0x40, "Max Bet" ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0x80, "5" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x00, "20" )

	PORT_START("DSW2")  // 6811
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x0c, 0x0c, "Minimum Rate" ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x70, 0x70, "YAKUMAN Bonus" )  PORT_DIPLOCATION("SW2:5,6,7") // this is the default for 'corner' machines, for 'single' it's listed as 0x30
	PORT_DIPSETTING(    0x70, DEF_STR( No ) )
	PORT_DIPSETTING(    0x60, "Once on Start" )
	PORT_DIPSETTING(    0x50, "300" )
	PORT_DIPSETTING(    0x40, "500" )
	PORT_DIPSETTING(    0x30, "700" )
	PORT_DIPSETTING(    0x20, "1000" )
	PORT_DIPSETTING(    0x10, "1000" ) // dip combination not listed in the manual
	PORT_DIPSETTING(    0x00, "1000" ) // dip combination not listed in the manual
	PORT_DIPNAME( 0x80, 0x80, "Yakuman Bonus Cycle" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, "Once" )
	PORT_DIPSETTING(    0x80, "Twice" )

	PORT_START("DSW3")  // 6812
	PORT_DIPNAME( 0x01, 0x01, "Payout Type" ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, "Credits" )
	PORT_DIPSETTING(    0x00, "Hopper" )
	PORT_DIPNAME( 0x02, 0x02, "Hopper Type" ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Reversed" )
	PORT_DIPNAME( 0x04, 0x00, "Service Count" ) PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "W-Bet" ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Renchan Rate" ) PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Auto Reach" ) PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Auto Tsumo" ) PORT_DIPLOCATION("SW3:7") // machine translated as 'automatic mode'
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Yakuman Match" ) PORT_DIPLOCATION("SW3:8") // machine translated as 'service point'
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW4")  // 6813
	PORT_DIPNAME( 0x01, 0x00, "Last Chance" ) PORT_DIPLOCATION("SW4:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Show Clock" ) PORT_DIPLOCATION("SW4:2")
	PORT_DIPSETTING(    0x02, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x00, "Girls" ) PORT_DIPLOCATION("SW4:3") // Renshu Gal Display according to machine translation
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Background" ) PORT_DIPLOCATION("SW4:4")
	PORT_DIPSETTING(    0x08, "Black" )
	PORT_DIPSETTING(    0x00, "Green" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW4:5") // default off according to manual, but left on for testing convenience
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "In Game Music" ) PORT_DIPLOCATION("SW4:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Yakuman Match Frequency" ) PORT_DIPLOCATION("SW4:7") // machine translated as 'time service frequency'
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW4:8") // 'OFF' in manual
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWTOP")    // 6814
	PORT_DIPNAME( 0x01, 0x01, "Credits Per Note" ) PORT_DIPLOCATION("SW1:9")
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:10")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:9")
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x08, 0x00, "Wave of Dividends" ) PORT_DIPLOCATION("SW2:10")
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( High ) )
	PORT_DIPNAME( 0x10, 0x00, "Don Den Key" ) PORT_DIPLOCATION("SW3:9")
	PORT_DIPSETTING(    0x00, "Flip-Flop" )
	PORT_DIPSETTING(    0x10, "Start" )
	PORT_DIPNAME( 0x20, 0x00, "Don Den Times" ) PORT_DIPLOCATION("SW3:10")
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x20, "8" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW4:9") // 'OFF' in manual
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Debug Mode" ) PORT_DIPLOCATION("SW4:10") // e.g. press start in bet screen ('OFF' in manual)
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( ichiban )
	PORT_INCLUDE( mjctrl2 )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 ) // "Note" ("Paper Money") = 10 Credits
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MEMORY_RESET )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )  // Analizer (Statistics). This plus service mode give access to dip page
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW-A")
	PORT_DIPNAME( 0x07, 0x07, "Pay Out" ) PORT_DIPLOCATION("DSW-A:1,2,3")
	PORT_DIPSETTING(    0x00, "60%" )
	PORT_DIPSETTING(    0x01, "65%" )
	PORT_DIPSETTING(    0x02, "70%" )
	PORT_DIPSETTING(    0x03, "75%" )
	PORT_DIPSETTING(    0x04, "80%" )
	PORT_DIPSETTING(    0x05, "85%" )
	PORT_DIPSETTING(    0x06, "90%" )
	PORT_DIPSETTING(    0x07, "95%" )
	PORT_DIPNAME( 0x18, 0x18, "Wup Level" ) PORT_DIPLOCATION("DSW-A:4,5")
	PORT_DIPSETTING(    0x18, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x60, 0x60, "Last Chance" ) PORT_DIPLOCATION("DSW-A:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPSETTING(    0x20, "8" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x80, 0x80, "Pay Sound" ) PORT_DIPLOCATION("DSW-A:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW-B")
	PORT_DIPNAME( 0x03, 0x03, "Odds Type" ) PORT_DIPLOCATION("DSW-B:1,2")
	PORT_DIPSETTING(    0x03, "A" )
	PORT_DIPSETTING(    0x02, "B" )
	PORT_DIPSETTING(    0x01, "C" )
	PORT_DIPSETTING(    0x00, "D" )
	PORT_DIPUNUSED_DIPLOC(0x04, 0x04, "DSW-B:3") // 3 and 4 have no apparent effect in 'Analizer 2' page
	PORT_DIPUNUSED_DIPLOC(0x08, 0x08, "DSW-B:4")
	PORT_DIPNAME( 0x30, 0x30, "Reset" ) PORT_DIPLOCATION("DSW-B:5,6")
	PORT_DIPSETTING(    0x30, "100" )
	PORT_DIPSETTING(    0x20, "150" )
	PORT_DIPSETTING(    0x10, "300" )
	PORT_DIPSETTING(    0x00, "500" )
	PORT_DIPNAME( 0x40, 0x40, "Bakaze" ) PORT_DIPLOCATION("DSW-B:7")
	PORT_DIPSETTING(    0x40, "Move" )
	PORT_DIPSETTING(    0x00, "No Move" )
	PORT_DIPNAME( 0x80, 0x80, "Pai" ) PORT_DIPLOCATION("DSW-B:8")
	PORT_DIPSETTING(    0x80, "Tate" )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )

	PORT_START("DSW-C")
	PORT_DIPNAME( 0x03, 0x03, "China" ) PORT_DIPLOCATION("DSW-C:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, "Hane" )
	PORT_DIPSETTING(    0x01, "Bai" )
	PORT_DIPSETTING(    0x00, "3Bai" )
	PORT_DIPNAME( 0x04, 0x04, "Yaku" ) PORT_DIPLOCATION("DSW-C:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Cut" )
	PORT_DIPNAME( 0x08, 0x08, "3Bai" ) PORT_DIPLOCATION("DSW-C:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Cut" )
	PORT_DIPUNUSED_DIPLOC(0x10, 0x10, "DSW-C:5") // 5 and 6 have no apparent effect in 'Analizer 2' page
	PORT_DIPUNUSED_DIPLOC(0x20, 0x20, "DSW-C:6")
	PORT_DIPNAME( 0x40, 0x40, "Utidome" ) PORT_DIPLOCATION("DSW-C:7")
	PORT_DIPSETTING(    0x40, "Nasi" )
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPNAME( 0x80, 0x80, "Bet Max" ) PORT_DIPLOCATION("DSW-C:8")
	PORT_DIPSETTING(    0x80, "10" )
	PORT_DIPSETTING(    0x00, "20" )

	PORT_START("DSW-D")
	PORT_DIPNAME( 0x03, 0x03, "Bet Skip" ) PORT_DIPLOCATION("DSW-D:1,2")
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("DSW-D:3,4")
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x70, 0x70, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("DSW-D:5,6,7") // 'Note'
	PORT_DIPSETTING(    0x70, "1" )
	PORT_DIPSETTING(    0x60, "2" )
	PORT_DIPSETTING(    0x50, "5" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x30, "20" )
	PORT_DIPSETTING(    0x20, "25" )
	PORT_DIPSETTING(    0x10, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPUNUSED_DIPLOC(0x80, 0x80, "DSW-D:8") // 8 has no apparent effect in 'Analizer 2' page
INPUT_PORTS_END


void royalmah_state::royalmah(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 18432000 / 6);        // 3.072 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &royalmah_state::royalmah_map);
	m_maincpu->set_addrmap(AS_IO, &royalmah_state::royalmah_iomap);
	m_maincpu->set_vblank_int("screen", FUNC(royalmah_state::irq0_line_hold));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// video hardware
	PALETTE(config, m_palette, FUNC(royalmah_state::royalmah_palette), 16*4);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_size(256, 256);
	screen.set_visarea(0, 255, 8, 247);
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_screen_update(FUNC(royalmah_state::screen_update));

	// sound hardware
	SPEAKER(config, "speaker").front_center();

	AY8910(config, m_ay, 18432000 / 12);
	m_ay->port_a_read_callback().set(FUNC(royalmah_state::player_1_port_r));
	m_ay->port_b_read_callback().set(FUNC(royalmah_state::player_2_port_r));
	m_ay->add_route(ALL_OUTPUTS, "speaker", 0.33);
}


void royalmah_state::janoh(machine_config &config)
{
	royalmah(config);

	m_maincpu->set_clock(8000000 / 2);   // 4 MHz ?
	m_maincpu->set_addrmap(AS_PROGRAM, &royalmah_state::janoh_map);
}

void royalmah_state::janoha(machine_config &config)
{
	janoh(config);

	z80_device &sub(Z80(config, "sub", 4000000));        // 4 MHz ?
	sub.set_addrmap(AS_PROGRAM, &royalmah_state::janoh_sub_map);
	sub.set_addrmap(AS_IO, &royalmah_state::janoh_sub_iomap);
	sub.set_vblank_int("screen", FUNC(royalmah_state::irq0_line_hold));
}

void royalmah_state::jansou(machine_config &config)
{
	royalmah(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &royalmah_state::jansou_map);
	m_maincpu->set_addrmap(AS_IO, &royalmah_state::royalmah_iomap);

	Z80(config, m_audiocpu, 4000000); // 4.000 MHz
	m_audiocpu->set_addrmap(AS_PROGRAM, &royalmah_state::jansou_sub_map);
	m_audiocpu->set_addrmap(AS_IO, &royalmah_state::jansou_sub_iomap);
	m_audiocpu->set_periodic_int(FUNC(royalmah_state::irq0_line_hold), attotime::from_hz(4000000 / 512));

	GENERIC_LATCH_8(config, m_soundlatch);

	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.25); // unknown DAC
}

void royalmah_prgbank_state::dondenmj(machine_config &config)
{
	royalmah(config);

	m_maincpu->set_clock(8000000 / 2);   // 4 MHz ?
	m_maincpu->set_addrmap(AS_PROGRAM, &royalmah_prgbank_state::royalmah_banked_map);
	m_maincpu->set_addrmap(AS_IO, &royalmah_prgbank_state::dondenmj_iomap);
}

void royalmah_prgbank_state::tahjong(machine_config &config)
{
	royalmah(config);

	m_maincpu->set_clock(8000000 / 2);   // 4 MHz ?
	m_maincpu->set_addrmap(AS_PROGRAM, &royalmah_prgbank_state::tahjong_map);
	m_maincpu->set_addrmap(AS_IO, &royalmah_prgbank_state::tahjong_iomap);
}

void royalmah_prgbank_state::makaijan(machine_config &config)
{
	royalmah(config);

	m_maincpu->set_clock(8000000 / 2);   // 4 MHz ?
	m_maincpu->set_addrmap(AS_PROGRAM, &royalmah_prgbank_state::royalmah_banked_map);
	m_maincpu->set_addrmap(AS_IO, &royalmah_prgbank_state::makaijan_iomap);
}

void royalmah_prgbank_state::daisyari(machine_config &config)
{
	royalmah(config);

	m_maincpu->set_clock(8000000 / 2);   // 4 MHz ?
	m_maincpu->set_addrmap(AS_PROGRAM, &royalmah_prgbank_state::royalmah_banked_map);
	m_maincpu->set_addrmap(AS_IO, &royalmah_prgbank_state::daisyari_iomap);
}

void royalmah_prgbank_state::mjclub(machine_config &config)
{
	royalmah(config);

	m_maincpu->set_clock(8000000 / 2);   // 4 MHz ?
	m_maincpu->set_addrmap(AS_PROGRAM, &royalmah_prgbank_state::royalmah_banked_map);
	m_maincpu->set_addrmap(AS_IO, &royalmah_prgbank_state::mjclub_iomap);
}

void royalmah_prgbank_state::chalgirl(machine_config &config)
{
	royalmah(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &royalmah_prgbank_state::chalgirl_map);
	m_maincpu->set_addrmap(AS_IO, &royalmah_prgbank_state::chalgirl_iomap);
}

void royalmah_prgbank_state::rkjanoh2(machine_config &config)
{
	chalgirl(config);

	m_maincpu->set_addrmap(AS_IO, &royalmah_prgbank_state::rkjanoh2_iomap);
}

void royalmah_prgbank_state::mjyarou(machine_config &config)
{
	chalgirl(config);

	m_maincpu->set_addrmap(AS_IO, &royalmah_prgbank_state::mjyarou_iomap);
}

void royalmah_prgbank_state::mjsenka(machine_config &config)
{
	mjyarou(config);

	m_maincpu->set_addrmap(AS_OPCODES, &royalmah_prgbank_state::mjsenka_opcodes_map);
}

void royalmah_prgbank_state::mjsiyoub(machine_config &config)
{
	royalmah(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &royalmah_prgbank_state::mjsiyoub_map);
	m_maincpu->set_addrmap(AS_IO, &royalmah_prgbank_state::mjsiyoub_iomap);

	// TODO: implement extra sound chips. Z80 ROM is scrambled.
	Z80(config, m_audiocpu, 18.432_MHz_XTAL / 6); // divider not verified
	m_audiocpu->set_addrmap(AS_PROGRAM, &royalmah_prgbank_state::mjsiyoub_audio_prg_map);
	m_audiocpu->set_disable(); // until it's decrypted, to avoid clogging the error log

	MSM5205(config, "msm", 400_kHz_XTAL);
}

void royalmah_state::ippatsu(machine_config &config)
{
	royalmah(config);

	m_maincpu->set_clock(8000000 / 2);   // 4 MHz ?
	m_maincpu->set_addrmap(AS_PROGRAM, &royalmah_state::ippatsu_map);
	m_maincpu->set_addrmap(AS_IO, &royalmah_state::ippatsu_iomap);
}

void royalmah_state::janyoup2(machine_config &config)
{
	royalmah(config);

	m_maincpu->set_clock(XTAL(18'432'000) / 4); // unknown divider
	m_maincpu->set_addrmap(AS_IO, &royalmah_state::janyoup2_iomap);

	hd6845s_device &crtc(HD6845S(config, "crtc", XTAL(18'432'000) / 12)); // unknown divider
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(4);
}

void royalmah_state::seljan(machine_config &config)
{
	janyoup2(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &royalmah_state::seljan_map);
	m_maincpu->set_addrmap(AS_IO, &royalmah_state::seljan_iomap);
}

INTERRUPT_GEN_MEMBER(royalmah_prgbank_state::suzume_irq)
{
	if (m_suzume_bank & 0x40)
		device.execute().pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void royalmah_prgbank_state::suzume(machine_config &config)
{
	dondenmj(config);

	m_maincpu->set_addrmap(AS_IO, &royalmah_prgbank_state::suzume_iomap);
	m_maincpu->set_vblank_int("screen", FUNC(royalmah_prgbank_state::suzume_irq));
}

void royalmah_prgbank_state::jongshin(machine_config &config)
{
	dondenmj(config);

	m_maincpu->set_addrmap(AS_IO, &royalmah_prgbank_state::jongshin_iomap);
}

void royalmah_prgbank_state::tontonb(machine_config &config)
{
	dondenmj(config);

	m_maincpu->set_addrmap(AS_IO, &royalmah_prgbank_state::tontonb_iomap);
}

void royalmah_prgbank_state::mjdiplob(machine_config &config)
{
	dondenmj(config);

	m_maincpu->set_addrmap(AS_IO, &royalmah_prgbank_state::mjdiplob_iomap);
}

void royalmah_prgbank_state::majs101b(machine_config &config)
{
	dondenmj(config);

	m_maincpu->set_addrmap(AS_IO, &royalmah_prgbank_state::majs101b_iomap);
}

void royalmah_prgbank_state::mjapinky(machine_config &config)
{
	dondenmj(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &royalmah_prgbank_state::mjapinky_map);
	m_maincpu->set_addrmap(AS_IO, &royalmah_prgbank_state::mjapinky_iomap);
}

void royalmah_prgbank_state::mjderngr(machine_config &config)
{
	dondenmj(config);

	m_maincpu->set_addrmap(AS_IO, &royalmah_prgbank_state::mjderngr_iomap);

	// video hardware
	m_palette->set_entries(16 * 32);
	m_palette->set_init(FUNC(royalmah_prgbank_state::mjderngr_palette));
}

void royalmah_prgbank_state::janptr96(machine_config &config)
{
	mjderngr(config);

	tmpz84c015_device &maincpu(TMPZ84C015(config.replace(), "maincpu", XTAL(16'000'000) / 2));    // 8 MHz?
	maincpu.set_addrmap(AS_PROGRAM, &royalmah_prgbank_state::janptr96_map);
	maincpu.set_addrmap(AS_IO, &royalmah_prgbank_state::janptr96_iomap);
	maincpu.in_pa_callback().set(FUNC(royalmah_prgbank_state::janptr96_dsw_r));
	maincpu.out_pb_callback().set(FUNC(royalmah_prgbank_state::janptr96_dswsel_w));
	// internal CTC channels 0 & 1 have falling edge triggers

	screen_device &screen(*subdevice<screen_device>("screen"));
	screen.set_visarea(0, 255, 8, 255-8);
	screen.screen_vblank().set(m_maincpu, FUNC(tmpz84c015_device::trg0)).invert();

	// devices
	MSM6242(config, m_rtc, 32.768_kHz_XTAL).out_int_handler().set(m_maincpu, FUNC(tmpz84c015_device::trg1)).invert();
}


void royalmah_prgbank_state::mjifb(machine_config &config)
{
	mjderngr(config);

	tmp90841_device &tmp(TMP90841(config.replace(), m_maincpu, 8000000));   // ?
	tmp.set_addrmap(AS_PROGRAM, &royalmah_prgbank_state::mjifb_map);
	tmp.port_read<3>().set_ioport("PORT3_5").rshift(6);
	tmp.port_write<3>().set(FUNC(royalmah_prgbank_state::mjifb_p3_w));
	tmp.port_write<4>().set(FUNC(royalmah_prgbank_state::mjifb_p4_w));
	tmp.port_read<5>().set_ioport("PORT3_5");
	tmp.port_read<6>().set_ioport("PORT6_7");
	tmp.port_read<7>().set_ioport("PORT6_7").rshift(4);
	tmp.port_read<8>().set(FUNC(royalmah_prgbank_state::mjifb_p8_r));
	tmp.port_write<8>().set(FUNC(royalmah_prgbank_state::mjifb_p8_w));

	screen_device &screen(*subdevice<screen_device>("screen"));
	screen.set_visarea(0, 255, 8, 255-8);
	screen.screen_vblank().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
}


void royalmah_prgbank_state::mjdejavu(machine_config &config)
{
	mjderngr(config);
	tmp90841_device &tmp(TMP90841(config.replace(), m_maincpu, 8000000));   // ?
	tmp.set_addrmap(AS_PROGRAM, &royalmah_prgbank_state::mjdejavu_map);
	tmp.port_read<3>().set_ioport("PORT3_5").rshift(6);
	tmp.port_write<3>().set(FUNC(royalmah_prgbank_state::mjifb_p3_w));
	tmp.port_write<4>().set(FUNC(royalmah_prgbank_state::mjifb_p4_w));
	tmp.port_read<5>().set_ioport("PORT3_5");
	tmp.port_read<6>().set_ioport("PORT6_7");
	tmp.port_read<7>().set_ioport("PORT6_7").rshift(4);
	tmp.port_read<8>().set(FUNC(royalmah_prgbank_state::mjifb_p8_r));
	tmp.port_write<8>().set(FUNC(royalmah_prgbank_state::mjifb_p8_w));

	screen_device &screen(*subdevice<screen_device>("screen"));
	screen.set_visarea(0, 255, 8, 255-8);
	screen.screen_vblank().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
}


void royalmah_prgbank_state::mjtensin(machine_config &config)
{
	mjderngr(config);
	tmp90841_device &tmp(TMP90841(config.replace(), m_maincpu, 12000000));  // ?
	tmp.set_addrmap(AS_PROGRAM, &royalmah_prgbank_state::mjtensin_map);
	tmp.port_read<3>().set(FUNC(royalmah_prgbank_state::mjtensin_p3_r));
	tmp.port_write<4>().set(FUNC(royalmah_prgbank_state::mjtensin_p4_w));

	screen_device &screen(*subdevice<screen_device>("screen"));
	screen.set_visarea(0, 255, 8, 255-8);
	screen.screen_vblank().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	// devices
	MSM6242(config, m_rtc, 32.768_kHz_XTAL).out_int_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ1);
}

void royalmah_prgbank_state::majrjh(machine_config &config)
{
	mjtensin(config);
	tmp91640_device &tmp(TMP91640(config.replace(), m_maincpu, 12_MHz_XTAL));
	tmp.set_addrmap(AS_PROGRAM, &royalmah_prgbank_state::majrjh_map);
	tmp.port_read<3>().set(FUNC(royalmah_prgbank_state::mjtensin_p3_r));
	tmp.port_write<4>().set(FUNC(royalmah_prgbank_state::mjtensin_p4_w));
}

void royalmah_prgbank_state::cafetime(machine_config &config)
{
	mjderngr(config);
	tmp90841_device &tmp(TMP90841(config.replace(), m_maincpu, 12000000));  // ?
	tmp.set_addrmap(AS_PROGRAM, &royalmah_prgbank_state::cafetime_map);
	tmp.port_write<3>().set(FUNC(royalmah_prgbank_state::cafetime_p3_w));
	tmp.port_write<4>().set(FUNC(royalmah_prgbank_state::cafetime_p4_w));

	screen_device &screen(*subdevice<screen_device>("screen"));
	screen.set_visarea(0, 255, 8, 255-8);
	screen.screen_vblank().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	// devices
	MSM6242(config, m_rtc, 32.768_kHz_XTAL).out_int_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ1);
}

void royalmah_prgbank_state::cafedoll(machine_config &config)
{
	cafetime(config);
	tmp90840_device &tmp(TMP90840(config.replace(), m_maincpu, XTAL(8'000'000))); // XTAL is verified, should it be divided?
	tmp.set_addrmap(AS_PROGRAM, &royalmah_prgbank_state::cafetime_map);
	tmp.port_write<3>().set(FUNC(royalmah_prgbank_state::cafetime_p3_w));
	tmp.port_write<4>().set(FUNC(royalmah_prgbank_state::cafetime_p4_w));
	tmp.port_read<5>().set(FUNC(royalmah_prgbank_state::mjvegas_p5_r));
	tmp.port_write<6>().set(FUNC(royalmah_prgbank_state::cafedoll_p6_w));
	tmp.port_write<7>().set(FUNC(royalmah_prgbank_state::cafedoll_p7_w));
}

void royalmah_prgbank_state::cafepara(machine_config &config)
{
	cafetime(config);
	tmp91640_device &tmp(TMP91640(config.replace(), m_maincpu, XTAL(8'000'000))); // XTAL is verified, should it be divided?
	tmp.set_addrmap(AS_PROGRAM, &royalmah_prgbank_state::cafepara_map);
	tmp.port_read<3>().set([this] () { logerror("%s: p3 in\n", machine().describe_context()); return uint8_t(0); }); // read sometimes
	tmp.port_read<4>().set([this] () { logerror("%s: p4 in\n", machine().describe_context()); return uint8_t(0); }); // not seen yet
	tmp.port_read<5>().set([this] () { logerror("%s: p5 in\n", machine().describe_context()); return uint8_t(0); }); // dips 5-8 for each of the 4 dip banks + dips 9-10 for first and second bank
	tmp.port_read<6>().set([this] () { logerror("%s: p6 in\n", machine().describe_context()); return uint8_t(0); }); // dips 1-4 for each of the 4 dip banks + dips 9-10 for third and fourth bank
	tmp.port_read<7>().set([this] () { logerror("%s: p7 in\n", machine().describe_context()); return uint8_t(0); }); // not seen yet
	tmp.port_read<8>().set([this] () { logerror("%s: p8 in\n", machine().describe_context()); return uint8_t(0); });
	tmp.port_write<3>().set([this] (uint8_t data) { logerror("%s: p3 out %02X\n", machine().describe_context(), data); }); // 0x6c at startup, remnant of older games?
	tmp.port_write<4>().set([this] (uint8_t data) { logerror("%s: p4 out %02X\n", machine().describe_context(), data); }); // 0x00 at startup
	tmp.port_write<5>().set([this] (uint8_t data) { logerror("%s: p5 out %02X\n", machine().describe_context(), data); }); // not seen yet
	tmp.port_write<6>().set([this] (uint8_t data) { logerror("%s: p6 out %02X\n", machine().describe_context(), data); }); // not seen yet
	tmp.port_write<7>().set([this] (uint8_t data) { logerror("%s: p7 out %02X\n", machine().describe_context(), data); }); // seen 0x07, 0x0b, 0x0d, 0x0f. DSW select
	tmp.port_write<8>().set([this] (uint8_t data) { logerror("%s: p8 out %02X\n", machine().describe_context(), data); }); // 0x00 or 0x08, most probably view but could also have to do with DSW select
}

void royalmah_prgbank_state::mjvegasa(machine_config &config)
{
	mjderngr(config);
	tmp90841_device &tmp(TMP90841(config.replace(), m_maincpu, XTAL(8'000'000))); // ?
	tmp.set_addrmap(AS_PROGRAM, &royalmah_prgbank_state::mjvegasa_map);
	tmp.port_read<3>().set(FUNC(royalmah_prgbank_state::mjtensin_p3_r));
	tmp.port_write<3>().set(FUNC(royalmah_prgbank_state::mjvegasa_p3_w));
	tmp.port_write<4>().set(FUNC(royalmah_prgbank_state::mjvegasa_p4_w));

	screen_device &screen(*subdevice<screen_device>("screen"));
	screen.set_visarea(0, 255, 8, 255-8);
	screen.screen_vblank().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	// devices
	MSM6242(config, m_rtc, 32.768_kHz_XTAL).out_int_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ1);
}

void royalmah_prgbank_state::mjvegas(machine_config &config)
{
	mjvegasa(config);

	tmp90840_device &tmp(TMP90840(config.replace(), m_maincpu, XTAL(8'000'000))); // XTAL is verified, should it be divided?
	tmp.set_addrmap(AS_PROGRAM, &royalmah_prgbank_state::mjvegasa_map);
	tmp.port_read<3>().set(FUNC(royalmah_prgbank_state::mjtensin_p3_r));
	tmp.port_write<3>().set(FUNC(royalmah_prgbank_state::mjvegasa_p3_w));
	tmp.port_write<4>().set(FUNC(royalmah_prgbank_state::mjvegasa_p4_w));
	tmp.port_read<5>().set(FUNC(royalmah_prgbank_state::mjvegas_p5_r));
	tmp.port_write<6>().set(FUNC(royalmah_prgbank_state::mjvegas_p6_w));
	tmp.port_write<7>().set(FUNC(royalmah_prgbank_state::mjvegas_p7_w));
}

void royalmah_prgbank_state::ichiban(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 18.432_MHz_XTAL / 3);
	m_maincpu->set_addrmap(AS_PROGRAM, &royalmah_prgbank_state::ichiban_map);
	m_maincpu->set_addrmap(AS_IO, &royalmah_prgbank_state::ichiban_iomap);
	m_maincpu->set_addrmap(AS_OPCODES, &royalmah_prgbank_state::ichiban_opcodes_map);
	m_maincpu->set_vblank_int("screen", FUNC(royalmah_prgbank_state::irq0_line_hold));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_size(256, 256);
	screen.set_visarea(0, 255, 8, 247);
	screen.set_refresh_hz(60.5686);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	screen.set_screen_update(FUNC(royalmah_prgbank_state::screen_update));

	PALETTE(config, m_palette, palette_device::RGB_444_PROMS, "proms", 512);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ym2149_device &ay(YM2149(config, "aysnd", 18.432_MHz_XTAL / 12));
	ay.port_a_read_callback().set(FUNC(royalmah_prgbank_state::player_1_port_r));
	ay.port_b_read_callback().set(FUNC(royalmah_prgbank_state::player_2_port_r));
	ay.add_route(ALL_OUTPUTS, "mono", 0.30);

	YM2413(config, "ymsnd", 18.432_MHz_XTAL / 6).add_route(ALL_OUTPUTS, "mono", 0.5);
}

void royalmah_prgbank_state::pongboo2(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 48_MHz_XTAL / 12); // not a Z80, probably some derived core
	m_maincpu->set_addrmap(AS_PROGRAM, &royalmah_prgbank_state::pongboo2_map);
	m_maincpu->set_addrmap(AS_IO, &royalmah_prgbank_state::pongboo2_iomap);
	m_maincpu->set_addrmap(AS_OPCODES, &royalmah_prgbank_state::pongboo2_opcodes_map);
	m_maincpu->set_vblank_int("screen", FUNC(royalmah_prgbank_state::irq0_line_hold));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_size(256, 256);
	screen.set_visarea(0, 255, 8, 247);
	screen.set_refresh_hz(60.5686);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_screen_update(FUNC(royalmah_prgbank_state::screen_update));

	PALETTE(config, m_palette).set_format(palette_device::xRGB_444, 512); // wrong

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, "oki", 1'000'000, okim6295_device::PIN7_LOW).add_route(ALL_OUTPUTS, "mono", 0.5); // frequency and pin 7 not verified
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

/***************************************************************************

Royal Mahjong
(c)1981 Nichibutsu

CPU: Z80
Sound: AY-3-8910
OSC: 18.432MHz

***************************************************************************/

ROM_START( royalmj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.p1", 0x0000, 0x1000, CRC(549544bb) SHA1(dfb221572c7bfd267a22c0a944830d5f127f9942) )
	ROM_LOAD( "2.p2", 0x1000, 0x1000, CRC(afc8a61e) SHA1(4134f6404f955838fc48fd0f87b83ebc75c1a021) )
	ROM_LOAD( "3.p3", 0x2000, 0x1000, CRC(5d33e54d) SHA1(bf5e0ad5177c086f1cea5c90d7273a841db941bc) )
	ROM_LOAD( "4.p4", 0x3000, 0x1000, CRC(91339560) SHA1(0fb4141e236ab57b3e915dadb982b28ca11d269f) )
	ROM_LOAD( "5.p5", 0x4000, 0x1000, CRC(cc9123a3) SHA1(75276045247a0c9ac5810be01f3b58ad63101f9b) )
	ROM_LOAD( "6.p6", 0x5000, 0x1000, CRC(92150a0f) SHA1(5c97ba5014abdba4afc78e02e7d90e6ca4d777ac) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "18s030n.6k", 0x0000, 0x0020, CRC(d3007282) SHA1(e4d863ab193e49208ed0f59dcddb1da0492314f6) ) // sldh w/tahjong
ROM_END

ROM_START( royalmah )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rom1",       0x0000, 0x1000, CRC(69b37a62) SHA1(7792528754b0df4e11f4ebe33380b713ac7351a3) )
	ROM_LOAD( "rom2",       0x1000, 0x1000, CRC(0c8351b6) SHA1(9e6b48fd39dd98478d1e3557df839b09652c4349) )
	ROM_LOAD( "rom3",       0x2000, 0x1000, CRC(b7736596) SHA1(4b8bc175d945e695b767b9fb2227ffc1cd4b0547) )
	ROM_LOAD( "rom4",       0x3000, 0x1000, CRC(e3c7c15c) SHA1(a335374cc0f5b1d8e689cc304d006dd97f3e35e7) )
	ROM_LOAD( "rom5",       0x4000, 0x1000, CRC(16c09c73) SHA1(ea712f9ca3200ca27434e4200187b488e24f4c65) )
	ROM_LOAD( "rom6",       0x5000, 0x1000, CRC(92687327) SHA1(4fafba5881dca2a147616d94dd055eba6aa3c653) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "f-rom.bpr",  0x0000, 0x0020, CRC(d3007282) SHA1(e4d863ab193e49208ed0f59dcddb1da0492314f6) )
ROM_END

ROM_START( openmj )
	ROM_REGION( 0x7000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "10", 0x0000, 0x2000, CRC(4042920e) SHA1(19753bcb27ebf391ab824a45c6e41d956826a263) )
	ROM_LOAD( "20", 0x2000, 0x2000, CRC(8fa0f735) SHA1(645154d51c0679b953b9ffc2f1d3b8f2752a0796) )
	ROM_LOAD( "30", 0x4000, 0x2000, CRC(00045cd7) SHA1(0c32995753c1da14dacc8bc6c12dbcbdcae4e1b0) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "82s123.prm", 0x00, 0x20, CRC(d3007282) SHA1(e4d863ab193e49208ed0f59dcddb1da0492314f6) )
ROM_END

ROM_START( tahjong )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "p1.bin", 0x00000, 0x1000, CRC(49c922b3) SHA1(6672a4f739d428b722ab22e7ca93064211557abc) )
	ROM_LOAD( "p2.bin", 0x01000, 0x1000, CRC(c33e3cc3) SHA1(707c437f180ddd2916b5806f208e0a478207d528) )
	ROM_LOAD( "p3.bin", 0x02000, 0x1000, CRC(9e741a74) SHA1(6c8e8eb04331d48b72e2be270c13dbf8deb76005) )
	ROM_LOAD( "p4.bin", 0x03000, 0x1000, CRC(dc6ae62b) SHA1(d0c51047f734c885b7f19972c1bf0408199fde51) )
	ROM_LOAD( "p5.bin", 0x04000, 0x1000, CRC(cc9123a3) SHA1(75276045247a0c9ac5810be01f3b58ad63101f9b) ) // same as royalmj (unused)
	ROM_LOAD( "p6.bin", 0x05000, 0x1000, CRC(92150a0f) SHA1(5c97ba5014abdba4afc78e02e7d90e6ca4d777ac) ) // ""

	ROM_LOAD( "s1.bin", 0x10000, 0x4000, CRC(beff21af) SHA1(4dc40a1ac4e36401b1ad8ae3954af6e5001e0a67) )
	ROM_LOAD( "s2.bin", 0x14000, 0x4000, CRC(fed42e7c) SHA1(31136dff07bd1883dc2d107823ba83a34abf003d) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "18s030n.6k", 0x0000, 0x0020, CRC(c074c0f0) SHA1(b62519d1496ea366b0ea8ed657bd758ce93875ec) ) // sldh w/royalmj
ROM_END

ROM_START( janputer )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "1.bin",   0x0000, 0x2000, CRC(f36f4222) SHA1(ce18c273a59f86cb17ea6ba8a3daefc3d750df1e) )
	ROM_LOAD( "2.bin",   0x2000, 0x2000, CRC(7d57cb48) SHA1(38b97c5d02e3ab6187e5e0f86c06b8a3747b51a8) )
	ROM_LOAD( "3.bin",   0x4000, 0x2000, CRC(fb481d9a) SHA1(122e2b0d11fe1fe8cf219da9c8f96fe5a1016bb6) )
	ROM_LOAD( "7.bin",   0x6000, 0x1000, CRC(bb00fb9e) SHA1(4d2965a0339328d1700b39c166a5a92a96b05e67) )

	ROM_REGION( 0x20, "proms", 0 )
	// taken from Royal Mahjong, might or might not be the same.
	ROM_LOAD( "82s123.prm", 0x00, 0x20, BAD_DUMP CRC(d3007282) SHA1(e4d863ab193e49208ed0f59dcddb1da0492314f6) )
ROM_END

/***************************************************************************
Janyou Part II
(c)1984 Cosmo Denshi

CPU: Z80
Sound: AY-3-8910
Video: HD46505SP(HD6845SP)
OSC: 18.432MHz

ROMs:
1.C110       [36ebb3d0]
2.C109       [324426d4]
3.C108       [e98b6d34]
4.C107       [377b8ce9]

N82S123N.C98 [d3007282]


Others:
Battery
empty socket for MC68705
***************************************************************************/

ROM_START( janyoup2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.c110",       0x0000, 0x2000, CRC(36ebb3d0) SHA1(39c0cdd1dc5878539768074dad3c39aac4ace8bf) )
	ROM_LOAD( "2.c109",       0x2000, 0x2000, CRC(324426d4) SHA1(409244c8458d9bafa325746c37de9e7b955b3787) )
	ROM_LOAD( "3.c108",       0x4000, 0x2000, CRC(e98b6d34) SHA1(e27ab9a03aff750df78c5db52a112247bdd31328) )
	ROM_LOAD( "4.c107",       0x6000, 0x1000, CRC(377b8ce9) SHA1(a5efc517ae975e54af5325b8b3f4867e9f449d4c) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "n82s123n.c98", 0x0000, 0x0020, CRC(d3007282) SHA1(e4d863ab193e49208ed0f59dcddb1da0492314f6) )
ROM_END

ROM_START( seljan ) // Z80A + HD46505SP + AY891X (a sticker covers the chip type)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2",       0x0000, 0x2000, CRC(8d26d334) SHA1(4ac0f0961f666954caf59336f8389312db9dc263) ) // ok
	ROM_LOAD( "1",       0x2000, 0x2000, CRC(d670d7c3) SHA1(2106ecf6ad675b8fd167f1f21d615afdfb5bca6d) )
	ROM_LOAD( "3",       0x4000, 0x2000, CRC(6c0dfd50) SHA1(1f91ff0ef2f24414888ae8e5ebac72a5bb48780b) )
	ROM_LOAD( "5",       0x6000, 0x2000, CRC(22eb98ee) SHA1(aae0ba5098852e6fef3dd7cc97dfad97ca444bb7) )
	ROM_LOAD( "4",       0x8000, 0x1000, CRC(d41e2a10) SHA1(9c24f89ba877ab599ea89961b5e705fa770867be) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123_1",  0x0000, 0x0020, CRC(f1df0310) SHA1(cc7dd39a0aa10b57039143e587eee02cf5dd2e5c) )
ROM_END

/****************************************************************************

Ippatsu Gyakuten
(c)1986 Public Software / Paradais

modified Royal Mahjong hardware

CPU: Z80
Sound: AY-3-8910

ROMs:
11(27256)
12(27128)
82S123AN

dumped by sayu

****************************************************************************/

ROM_START( ippatsu )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "11", 0x0000, 0x8000, CRC(5f563be7) SHA1(2ce486777bd61a2de789683cd0c8abeefe31775b) )
	ROM_LOAD( "12", 0x8000, 0x4000, CRC(a09a43b0) SHA1(da12e669ccd036da817a69bd549e8668e6a45730) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123an", 0x00, 0x20, CRC(3bde1bbd) SHA1(729498483943f960e38c4ada992b099b698b497a) )
ROM_END

// has NEW DOUBLE BET MOJHONG (sic) in ROM but title screen shows 赤麻雀 (Red Mahjong)
ROM_START( akamj )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "b3.bin",  0x00000, 0x8000, CRC(fb2e5a3e) SHA1(6a06b36dd3baa4c0aba339aaf2e00d82d08cbb22) )
	ROM_LOAD( "bc2.bin", 0x08000, 0x8000, CRC(38d2aa91) SHA1(b0469803265c6359dc5680e669324a41b84548d7) )
	ROM_LOAD( "bc1.bin", 0x10000, 0x8000, CRC(38d2aa91) SHA1(b0469803265c6359dc5680e669324a41b84548d7) ) // yes, same ROM 2 times

	ROM_REGION( 0x20, "proms", 0 )
	// taken from Royal Mahjong, might or might not be the same.
	ROM_LOAD( "82s123.prm", 0x00, 0x20, BAD_DUMP CRC(d3007282) SHA1(e4d863ab193e49208ed0f59dcddb1da0492314f6) )
ROM_END


ROM_START( suzume )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "p1.bin",     0x00000, 0x1000, CRC(e9706967) SHA1(2e3d78178623de6552c9036da90e02f240d94055) )
	ROM_LOAD( "p2.bin",     0x01000, 0x1000, CRC(dd48cd62) SHA1(1ce7b515fabae5054f0ac284a9ed5760f59d18fa) )
	ROM_LOAD( "p3.bin",     0x02000, 0x1000, CRC(10a05c23) SHA1(f13ba660bc5eff9057b1ab46f564f586c76e945d) )
	ROM_LOAD( "p4.bin",     0x03000, 0x1000, CRC(267eaf52) SHA1(56e2f5d7080463dc0f11a2751590ac2b79eb02c5) )
	ROM_LOAD( "p5.bin",     0x04000, 0x1000, CRC(2fde346b) SHA1(7f45aa4427b4cb6bf6cc5919d397b25d53e133f3) )
	ROM_LOAD( "p6.bin",     0x05000, 0x1000, CRC(57f42ac7) SHA1(209b2f62a64ddf544578f144d9ec83478603c8b2) )
	// bank switched ROMs follow
	ROM_LOAD( "1.1a",       0x10000, 0x08000, CRC(f670dd47) SHA1(d0236021ae4dd5a10603dde038eb777feeff016f) )    // 0
	ROM_LOAD( "2.1c",       0x18000, 0x08000, CRC(140b11aa) SHA1(6f6a96135434324dcb486596920cb785fe2bf1a2) )    // 1
	ROM_LOAD( "3.1d",       0x20000, 0x08000, CRC(3d437b61) SHA1(175308086e1d7ab566c82dcaeef9f50690edf92a) )    // 2
	ROM_LOAD( "4.1e",       0x28000, 0x08000, CRC(9da8952e) SHA1(956d16b82ff8fe733a7b3135d082e18ea5167dfe) )    // 3
	ROM_LOAD( "5.1h",       0x30000, 0x08000, CRC(04a6f41a) SHA1(37117faf6bc823770413faa7618387ca6f16fa34) )    // 4

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "ic6k.bin",   0x0000, 0x0020, CRC(97e1defe) SHA1(b5002218b2292f7623dd9a205ce183dedeec03f1) )
ROM_END

ROM_START( jongshin )
	ROM_REGION( 0x28000, "maincpu", 0 ) // all 2732
	ROM_LOAD( "js1.p1",     0x00000, 0x1000, CRC(0c33eb1c) SHA1(4258f2df8e9d4d3fd3fd77c555bd36ced601c45f) )
	ROM_LOAD( "js2.p2",     0x01000, 0x1000, CRC(a74bfa19) SHA1(378ec5dcddbe1c1e66b9ec0576b898442e3ba89c) )
	ROM_LOAD( "js3.p3",     0x02000, 0x1000, CRC(7519804f) SHA1(4181e58964ae149e675c2aeb49edee6f5d06f6ed) )
	ROM_LOAD( "js4.p4",     0x03000, 0x1000, CRC(2d6b49bc) SHA1(4e75df252cc3af003a99cc9425980a9d2a457558) )
	ROM_LOAD( "js5.p5",     0x04000, 0x1000, CRC(4631153f) SHA1(d31e15de3d54118905946836e72b4794dae89004) )
	ROM_LOAD( "js6.p6",     0x05000, 0x1000, CRC(ed32bd57) SHA1(6d5d1ae959e07207146197c7c370810306dca462) )
	// bank switched ROMs follow
	ROM_LOAD( "dyna 1.8c",       0x10000, 0x08000, CRC(d2cea54a) SHA1(16143974731d3b81ad377ebe58c9253c127e5588) ) // 27256
	ROM_LOAD( "dyna 2.7c",       0x18000, 0x08000, CRC(9d7c62ff) SHA1(92de7bb84f6f64b887b5500a54ff6f0b84e0b07d) ) // 27256
	ROM_LOAD( "dyna 3.6c",       0x20000, 0x04000, CRC(b716f2e1) SHA1(f29617185771a43f057dff062a2493bcf281c85a) ) // 27128

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123.6k",   0x0000, 0x0020, CRC(faa20ce5) SHA1(408e90c13d5bd5fefdd9767a7643808a3cd9c111) )
ROM_END

ROM_START( dondenmj )
	ROM_REGION( 0x90000, "maincpu", 0 )
	ROM_LOAD( "dn5.1h",     0x00000, 0x08000, CRC(3080252e) SHA1(e039087afc36a0c594da093ea599b81a1d757139) )
	// bank switched ROMs follow
	ROM_LOAD( "dn1.1e",     0x18000, 0x08000, CRC(1cd9c48a) SHA1(12bc519889dacea59ae49672ad5313fff3a99f12) )    // 1
	ROM_LOAD( "dn2.1d",     0x20000, 0x04000, CRC(7a72929d) SHA1(7955f41883fa53876172bac417955ed0b5eb43f4) )    // 2
	ROM_LOAD( "dn3.2h",     0x30000, 0x08000, CRC(b09d2897) SHA1(0cde3e16ca333be01a5ab3a232f2ea602faec7a2) )    // 4
	ROM_LOAD( "dn4.2e",     0x50000, 0x08000, CRC(67d7dcd6) SHA1(6b708a29de1f4738eb2d4e667327d9433ff7216c) )    // 8

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "ic6k.bin",   0x0000, 0x0020, CRC(97e1defe) SHA1(b5002218b2292f7623dd9a205ce183dedeec03f1) )
ROM_END

ROM_START( mjdiplob )
	ROM_REGION( 0x90000, "maincpu", 0 )
	ROM_LOAD( "071.4l",     0x00000, 0x10000, CRC(81a6d6b0) SHA1(c6169e6d5f35304a0c3efcc2175c3213650f179c) )
	// bank switched ROMs follow
	ROM_RELOAD(             0x10000, 0x10000 )              // 0,1
	ROM_LOAD( "072.4k",     0x20000, 0x10000, CRC(a992bb85) SHA1(e60231e04831dac122d1d49a68641ee47b57faaf) )    // 2,3
	ROM_LOAD( "073.4j",     0x30000, 0x10000, CRC(562ed64f) SHA1(42b4a7e5a8de4dde83c12d7b9facf561bc872978) )    // 4,5
	ROM_LOAD( "074.4h",     0x40000, 0x10000, CRC(1eba0140) SHA1(0d0b95be338d7450ad3b24cc47e24e94f86dcefe) )    // 6,7

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "ic6k.bin",   0x0000, 0x0020, CRC(c1e427df) SHA1(9a9980d93dff4b87a940398b18277acaf946eeab) )
ROM_END

ROM_START( tontonb )
	ROM_REGION( 0x90000, "maincpu", 0 )
	ROM_LOAD( "091.5e",     0x00000, 0x10000, CRC(d8d67b59) SHA1(7e7a85df738f80fc031cda8a104ac9c7b3e24785) )
	// bank switched ROMs follow
	ROM_RELOAD(             0x10000, 0x10000 )              // 0,1
	/**/                                                    // 2,3 unused
	ROM_LOAD( "093.5b",     0x30000, 0x10000, CRC(24b6be55) SHA1(11390d6ed55d7d0b7b84c6d36d4ac5330a06abba) )    // 4,5
	/**/                                                    // 6,7 unused
	ROM_LOAD( "092.5c",     0x50000, 0x10000, CRC(7ff2738b) SHA1(89a49f89705f499439dc024fc70c87141a84780b) )    // 8,9

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "ic6k.bin",   0x0000, 0x0020, CRC(97e1defe) SHA1(b5002218b2292f7623dd9a205ce183dedeec03f1) )
ROM_END

/***************************************************************************

Makaijan
(c)1987 Dynax

***************************************************************************/

ROM_START( makaijan )
	ROM_REGION( 0x90000, "maincpu", 0 )
	ROM_LOAD( "ic1h.bin",     0x00000, 0x10000, CRC(7448c220) SHA1(ebb6564b83ce4f40a6e50a1be734e2086d97f592) )
	// bank switched ROMs follow
	ROM_COPY( "maincpu",    0x08000, 0x10000, 0x8000 )
	ROM_COPY( "maincpu",    0x08000, 0x18000, 0x8000 )
	ROM_LOAD( "052.1e",     0x50000, 0x10000, CRC(a881ca93) SHA1(499e17d2f57caa49c391d57dd737399fe4672f78) )
	ROM_LOAD( "053.1d",     0x30000, 0x10000, CRC(5f1d3e88) SHA1(152fde9f8e506f7f4ca1b2ecf8a828ece0501f78) )
	ROM_LOAD( "054.2h",     0x70000, 0x10000, CRC(ebc387c7) SHA1(7dfc892a5cccde7494ed06bbab88b4ea320dffbc) )
	ROM_LOAD( "055.2e",     0x20000, 0x10000, CRC(e26852ae) SHA1(8f8edefe851fd3641a5b4b227fb4dd976cdfa3e9) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "ic6k.bin",   0x0000, 0x0020, CRC(97e1defe) SHA1(b5002218b2292f7623dd9a205ce183dedeec03f1) )
ROM_END

/***************************************************************************

Daisyarin
(c)1989 Best System

***************************************************************************/

ROM_START( daisyari )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "1a.bin",     0x00000, 0x10000, CRC(7d14f90c) SHA1(742684d0785a93a45de0467e004db00531d016e2) )
	// bank switched ROMs follow
	ROM_COPY( "maincpu",    0x00000, 0x10000, 0x10000 )
	ROM_LOAD( "1c.bin",     0x20000, 0x10000, CRC(edfe52b9) SHA1(704c107fc8b89f561d2031d10468c124ab3d007a) ) // 2
	ROM_LOAD( "1d.bin",     0x30000, 0x10000, CRC(38f54a98) SHA1(d06eb851c75bfb2d8dd99bf5072c7f359f1f17e2) ) // 3
	ROM_LOAD( "1f.bin",     0x40000, 0x10000, CRC(b635f295) SHA1(dba3a59133c33c915dba678c510f00fb476f24da) ) // 4

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "6k.bin",   0x0000, 0x0020, CRC(c1e427df) SHA1(9a9980d93dff4b87a940398b18277acaf946eeab) )
ROM_END

/***************************************************************************

Mahjong Club
(c)XEX

Royal Mahjong subboard

1
3
4
5
63s081n

6116 RAM
surface scratched 40pin DIP (Z80?)
4.000MHz

***************************************************************************/

ROM_START( mjclub )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "5",     0x00000, 0x10000, CRC(cd148465) SHA1(42d1848656e461cfbf3fc0ba88ef8f4e67425f8c) )
	// bank switched ROMs follow
	ROM_COPY( "maincpu", 0x00000, 0x10000, 0x10000 )
	ROM_LOAD( "1",       0x50000, 0x10000, CRC(d0131f4b) SHA1(aac40b47b48f0ebfb07aaf17cb2a080fdcaa4697) )
	ROM_LOAD( "3",       0x60000, 0x10000, CRC(25628c38) SHA1(5166934c488c2f91bd6026c7896ad3536727d950) )
	ROM_LOAD( "4",       0x70000, 0x10000, CRC(a6ada333) SHA1(5fd44bf298a6f327118b98641af1aa0910519ded) )
	ROM_COPY( "maincpu", 0x50000, 0x40000, 0x10000 ) // guess
	ROM_COPY( "maincpu", 0x60000, 0x30000, 0x10000 )
	ROM_COPY( "maincpu", 0x70000, 0x20000, 0x10000 )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "63s081n", 0x0000, 0x0020, CRC(4add90c5) SHA1(de14abcba6eee53e73801ff12c45a75e875e6ca3) )
ROM_END

ROM_START( majs101b )
	ROM_REGION( 0x90000, "maincpu", 0 )
	ROM_LOAD( "171.3e",     0x00000, 0x10000, CRC(fa3c553b) SHA1(fda212559c4d55610a12ad2927afe21f9069c7b6) )
	// bank switched ROMs follow
	/**/                                                    // 0,1 unused
	ROM_RELOAD(             0x20000, 0x10000 )              // 2,3
	ROM_LOAD( "172.3f",     0x30000, 0x20000, CRC(7da39a63) SHA1(34d07978a326c83e5b51ce19619d52a75a501795) )    // 4,5,6,7
	ROM_LOAD( "173.3h",     0x50000, 0x20000, CRC(7a9e71ae) SHA1(ce1bde6e05f81b7dbb14015514397ed72f8dd92a) )    // 8,9,a,b
	ROM_LOAD( "174.3j",     0x70000, 0x10000, CRC(972c2cc9) SHA1(ba78d29d1723783dbd0e8c754d2422caad5ab367) )    // c,d

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "ic6k.bin",   0x0000, 0x0020, CRC(c1e427df) SHA1(9a9980d93dff4b87a940398b18277acaf946eeab) )
ROM_END

ROM_START( mjderngr )
	ROM_REGION( 0xb0000, "maincpu", 0 )
	ROM_LOAD( "2201.1a",    0x00000, 0x08000, CRC(54ec531d) SHA1(c5d9c575f6bdc499bae35123d7ad5bd4869b6ed9) )
	// bank switched ROMs follow
	ROM_CONTINUE(           0x10000, 0x08000 )              // 0
	ROM_LOAD( "2202.1b",    0x30000, 0x10000, CRC(edcf97f2) SHA1(8143f41d511fa01bd86faf829eb2c139292d705f) )    // 4,5
	ROM_LOAD( "2203.1d",    0x50000, 0x10000, CRC(a33368c0) SHA1(e216b65d7ed59d7cbf2b5d078799915d707b5291) )    // 8,9
	ROM_LOAD( "2204.1e",    0x70000, 0x20000, CRC(ed5fde4b) SHA1(d55487ae1007d43b71f06ae5c407c75db7054515) )    // c,d,e,f
	ROM_LOAD( "2205.1f",    0x90000, 0x20000, CRC(cfb8075d) SHA1(31f613a1a9b5f4295b552aeeddb760605ce2ac70) )    // 0x10,0x11,0x12,0x13

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "ic3g.bin",   0x000, 0x200, CRC(d43f4c7c) SHA1(117d2e4e8d5bea3e5dc903a4b87bd71786ae009c) )
	ROM_LOAD( "ic4g.bin",   0x200, 0x200, CRC(30cf7831) SHA1(b4593d51c6ceb301279a01a98665e4be8a3c403d) )
ROM_END

/***************************************************************************

Mahjong If (BET type)
(c)1990 Dynax

CPU:    Unknown 64P(Toshiba TLCS-90 series?)
Sound:  AY-3-8910
OSC:    8.000MHz
        18.432MHz


2911.1B   prg.
2902.1C
2903.1D
2904.1E
2905.1F
2906.1G

D29-1.4C  color
D29-2.4D

***************************************************************************/

ROM_START( mjifb )
	ROM_REGION( 0xd0000, "maincpu", 0 )
	ROM_LOAD( "2911.1b",    0x00000, 0x10000, CRC(138a31a1) SHA1(7e77c63a968206b8e61aaa423e19a766e4142554) )
	// bank switched ROMs follow
	ROM_RELOAD(             0x10000, 0x08000 )  // bank 0 = 8000-bfff
	ROM_CONTINUE(           0x10000, 0x08000 )
	ROM_LOAD( "2903.1d",    0x30000, 0x20000, CRC(90c44965) SHA1(6904bfa7475f9de921bc2abcfc337b3daf7e0fad) )
	ROM_LOAD( "2906.1g",    0x50000, 0x20000, CRC(ad469345) SHA1(914ea4c77a540467da779ea78c52e66b05c30475) )
	ROM_LOAD( "2904.1e",    0x70000, 0x20000, CRC(2791abfa) SHA1(a8fd1a7e1cf4441b447a4605ad2f1c13775f92da) )
	ROM_LOAD( "2905.1f",    0x90000, 0x20000, CRC(b7a73cf7) SHA1(d93111e6d5f84e331f8198d8c595e3500abed133) )
	ROM_LOAD( "2902.1c",    0xb0000, 0x10000, CRC(0ce02a98) SHA1(69f6bca9af8548038401839047a304a4aa97cfe6) )
	ROM_RELOAD(             0xc0000, 0x10000 )

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "d29-2.4d",   0x000, 0x200, CRC(78252f6a) SHA1(1869147bc6b7573c2543bdf6b17d6c3c1debdddb) )
	ROM_LOAD( "d29-1.4c",   0x200, 0x200, CRC(4aaec8cf) SHA1(fbe1c3729d078a422ffe68dfde495fcb9f329cdd) )
ROM_END

ROM_START( mjifb2 )
	ROM_REGION( 0xd0000, "maincpu", 0 )
	ROM_LOAD( "2921.bin",    0x00000, 0x10000, CRC(9f2bfa4e) SHA1(7d6ca22bf0a91d65fde34ae321054638df705eef) )
	// bank switched ROMs follow
	ROM_RELOAD(             0x10000, 0x08000 )  // bank 0 = 8000-bfff
	ROM_CONTINUE(           0x10000, 0x08000 )
	ROM_LOAD( "2903.1d",    0x30000, 0x20000, CRC(90c44965) SHA1(6904bfa7475f9de921bc2abcfc337b3daf7e0fad) )
	ROM_LOAD( "2906.1g",    0x50000, 0x20000, CRC(ad469345) SHA1(914ea4c77a540467da779ea78c52e66b05c30475) )
	ROM_LOAD( "2904.1e",    0x70000, 0x20000, CRC(2791abfa) SHA1(a8fd1a7e1cf4441b447a4605ad2f1c13775f92da) )
	ROM_LOAD( "2905.1f",    0x90000, 0x20000, CRC(b7a73cf7) SHA1(d93111e6d5f84e331f8198d8c595e3500abed133) )
	ROM_LOAD( "2902.1c",    0xb0000, 0x10000, CRC(0ce02a98) SHA1(69f6bca9af8548038401839047a304a4aa97cfe6) )
	ROM_RELOAD(             0xc0000, 0x10000 )

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "d29-2.4d",   0x000, 0x200, CRC(78252f6a) SHA1(1869147bc6b7573c2543bdf6b17d6c3c1debdddb) )
	ROM_LOAD( "d29-1.4c",   0x200, 0x200, CRC(4aaec8cf) SHA1(fbe1c3729d078a422ffe68dfde495fcb9f329cdd) )
ROM_END

ROM_START( mjifb3 )
	ROM_REGION( 0xd0000, "maincpu", 0 )
	ROM_LOAD( "2931.bin",    0x00000, 0x10000, CRC(2a3133de) SHA1(9fdc8c145d3da17ec5f86810716f1b1a2abd8023) )
	// bank switched ROMs follow
	ROM_RELOAD(             0x10000, 0x08000 )  // bank 0 = 8000-bfff
	ROM_CONTINUE(           0x10000, 0x08000 )
	ROM_LOAD( "2903.1d",    0x30000, 0x20000, CRC(90c44965) SHA1(6904bfa7475f9de921bc2abcfc337b3daf7e0fad) )
	ROM_LOAD( "2906.1g",    0x50000, 0x20000, CRC(ad469345) SHA1(914ea4c77a540467da779ea78c52e66b05c30475) )
	ROM_LOAD( "2904.1e",    0x70000, 0x20000, CRC(2791abfa) SHA1(a8fd1a7e1cf4441b447a4605ad2f1c13775f92da) )
	ROM_LOAD( "2905.1f",    0x90000, 0x20000, CRC(b7a73cf7) SHA1(d93111e6d5f84e331f8198d8c595e3500abed133) )
	ROM_LOAD( "2902.1c",    0xb0000, 0x10000, CRC(0ce02a98) SHA1(69f6bca9af8548038401839047a304a4aa97cfe6) )
	ROM_RELOAD(             0xc0000, 0x10000 )

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "d29-2.4d",   0x000, 0x200, CRC(78252f6a) SHA1(1869147bc6b7573c2543bdf6b17d6c3c1debdddb) )
	ROM_LOAD( "d29-1.4c",   0x200, 0x200, CRC(4aaec8cf) SHA1(fbe1c3729d078a422ffe68dfde495fcb9f329cdd) )
ROM_END

/***************************************************************************

Janputer '96
(c)1996 Dynax

Colour PROMs are TBP28S42's

***************************************************************************/

ROM_START( janptr96 )
	ROM_REGION( 0x210000, "maincpu", 0 )
	ROM_LOAD( "503x-1.1h", 0x000000, 0x40000, CRC(39914ecd) SHA1(e5796a95a7e3e7b61da63d50fa089be2946ba611) )
	// bank switched ROMs follow
	ROM_RELOAD(            0x010000, 0x40000 )
	ROM_RELOAD(            0x050000, 0x40000 )
	ROM_LOAD( "503x-2.1g", 0x090000, 0x80000, CRC(d4b1ed79) SHA1(e1e266339d1d05c0405bfd32b67f215807696c82) )
	ROM_LOAD( "503x-3.1f", 0x110000, 0x80000, CRC(9ba4deb0) SHA1(e9d44a6ed849ff90c0b1f9321cdd62e18c3fd35c) )
	ROM_LOAD( "503x-4.1e", 0x190000, 0x80000, CRC(e266ca0b) SHA1(d84608e7b474061a680510a266842e667bf2eab5) )

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "ns503b.3h", 0x000, 0x200, CRC(3b2a6b12) SHA1(ebd2929e6acbde989964bfef602b81f2f2fe04eb) )
	ROM_LOAD( "ns503a.3j", 0x200, 0x200, CRC(fe49b2f0) SHA1(a36ca005380cc92dfe473254c26be2cef2ced9b4) )
ROM_END

/***************************************************************************

Janputer Special
(c)1997 Dynax

Colour PROMs are TBP28642's

***************************************************************************/

ROM_START( janptrsp )
	ROM_REGION( 0x210000, "maincpu", 0 )
	ROM_LOAD( "ns51101.1h", 0x000000, 0x80000, CRC(44492ca1) SHA1(49e3dc9872a26e446599deb47161b8f52e4968c4) )
	// bank switched ROMs follow
	ROM_RELOAD(             0x010000, 0x80000 )
	ROM_LOAD( "ns51102.1g", 0x090000, 0x80000, CRC(01e6aa19) SHA1(a761fe69fb69c0bf101033e71813742c9fc2d747) )
	ROM_LOAD( "ns51103.1f", 0x110000, 0x80000, CRC(0fc94805) SHA1(035002e8354673a063faacd3cb91d0512cab677a) )
	ROM_LOAD( "ns51104.1e", 0x190000, 0x80000, CRC(00442508) SHA1(268cc0c76bb9c21213c941952dbc891778ad397e) )

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "ns511b.3h", 0x000, 0x200, CRC(1286434f) SHA1(6818549d0e8b231d7071e67923f47b96fc6e1bb6) )
	ROM_LOAD( "ns511a.3j", 0x200, 0x200, CRC(26b6714c) SHA1(0d110c1e3f7050a3c4ecbed630a0a8b522f1b0b0) )
ROM_END

/***************************************************************************

Mahjong Tensinhai
Dynax, 1995

PCB Layout
----------

Top board

D10010318L1
|----------------------------------------|
|DSW2(1)  DSW4(10)                  DIP16|
|                 |---|                  |
|DSW1(10) DSW3(10)| * |                  |
|                 |---|     PROM2        |
|                                        |
|                           PROM1        |
|                                        |
|                                        |
|                                        |
|                                        |
|                        1001.5E         |
| |-------------|                        |
| |     &       |        1002.4E    |---||
| |-------------|                   | D ||
|12MHz                   1003.3E    | I ||
|                                   | P ||
|BATTERY        32.768kHz           |40 ||
|         CLOCK          6264       |---||
|----------------------------------------|
Notes:
      Most of the chips have their surface scratched off.
      *     - Unknown PLCC44 IC. Possibly Mach110 or similar CPLD
      &     - Unknown SDIP64 IC. Possibly a Toshiba TMP91P640? Clock input 12.000MHz
      CLOCK - Some kind of clock IC, like Oki M6242 or similar
      PROM1 - 82S147 PROM labelled 'D100-1'
      PROM2 - 82S147 PROM labelled 'D100-2'
      DIP16 - Socket for cable that joins to lower board
      DIP40 - Socket for connector that joins to lower board


Bottom board

|--------------------------------------------------------|
|    BATTERY 6116                                        |
|  VOL                                                   |
|                                                        |
|                                              DIP40     |
|                                                        |
|           DSW(8)                              18.432MHz|
|                                                        |
|                                                        |
|M      DIP16                                            |
|A              4164    4164                             |
|H                                                       |
|J              4164    4164                             |
|O                                                       |
|N              4164    4164                             |
|G                                                       |
|2              4164    4164                             |
|8  AY3-8910                                             |
|               4164    4164                             |
|                                                        |
|               4164    4164                             |
|                                                        |
|               4164    4164                             |
|                                                        |
|               4164    4164                             |
|--------------------------------------------------------|
Notes:
      DIP16 - Socket for cable that joins to upper board
      DIP40 - Socket for connector that joins to upper board
      AY3-8910 clock - 1.536 [18.432/12]
      HSync - 15.5kHz
      VSync - 60Hz

***************************************************************************/

ROM_START( mjtensin )
	ROM_REGION( 0x290000, "maincpu", 0 )
	ROM_LOAD( "1001.5e", 0x000000, 0x80000, CRC(960e1fe9) SHA1(11f5164b2c75c0e684e910ee8e09de978bdaff2f) )
	// bank switched ROMs follow
	ROM_RELOAD(          0x010000, 0x80000 )
	ROM_RELOAD(          0x090000, 0x80000 )

	ROM_LOAD( "1002.4e", 0x110000, 0x80000, CRC(240eb7af) SHA1(2309e1c251fe55f6e6b97b5db94fa2fe914b88f4) )

	ROM_LOAD( "1003.3e", 0x210000, 0x80000, CRC(876081bf) SHA1(fe962cfa9318a9444123bcaf3406e22fb08e8c4e) )

	ROM_REGION( 0x2000, "internal_rom", 0 ) // the MCU is configured for external ROM usage, but does have the internal ROM. Let's load it for completeness' sake.
	ROM_LOAD( "mjtensin-mcu.3c", 0x0000, 0x2000, CRC(13804e4f) SHA1(34b5072528ad42c78ecae344da09182b850b4db1) )

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "d100-2.7e",  0x000, 0x200, CRC(6edeed23) SHA1(f4420c473ebbe3df92b0f5b1f0e4d5495fcb9fda) )
	ROM_LOAD( "d100-1.6e",  0x200, 0x200, CRC(88befd59) SHA1(cbcb437f9f6b5e542dc69f5c9e85ccbae47080af) )
ROM_END

/***************************************************************************

Mahjong Raijinhai DX
Dynax, 1996

PCB Layout
----------

Top board

D10010318L1
sticker - D10502168
|----------------------------------------|
|DSW2(1)  DSW4(10)                  DIP16|
|                 |---|                  |
|DSW1(10) DSW3(10)| * |                  |
|                 |---|     PROM2        |
|                                        |
|                           PROM1        |
|                                        |
|                                        |
|                                        |
|                                        |
|                        1051.5E         |
| |-------------|                        |
| |     &       |        1052.4E    |---||
| |-------------|                   | D ||
|12MHz                   1053.3E    | I ||
|                                   | P ||
|BATTERY        32.768kHz           |40 ||
|         CLOCK          6264       |---||
|----------------------------------------|
Notes:
      Most of the chips have their surface scratched off.
      *     - Unknown PLCC44 IC. Possibly Mach110 or similar CPLD
      &     - Unknown SDIP64 IC. Probably a Toshiba TMP91P640. Clock input 12.000MHz
              Was read as a TMP91P640 and found to be protected.
      CLOCK - Some kind of clock IC, like Oki M6242 or similar
      PROM1 - TBP28S42 (equivalent to 82S147) PROM labelled 'D105-1'
      PROM2 - TBP28S42 (equivalent to 82S147) PROM labelled 'D105-2'
      DIP16 - Socket for cable that joins to lower board
      DIP40 - Socket for connector that joins to lower board


Bottom board

|--------------------------------------------------------|
|    BATTERY 6116                                        |
|  VOL                                                   |
|                                                        |
|                                              DIP40     |
|                                                        |
|           DSW(8)                              18.432MHz|
|                                                        |
|                                                        |
|M      DIP16                                            |
|A              4116    4116                             |
|H                                                       |
|J              4116    4116                             |
|O                                                       |
|N              4116    4116                             |
|G                                                       |
|2              4116    4116                             |
|8  AY3-8910                                             |
|               4116    4116                             |
|                                                        |
|               4116    4116                             |
|                                                        |
|               4116    4116                             |
|                                                        |
|               4116    4116                             |
|--------------------------------------------------------|
Notes:
      DIP16 - Socket for cable that joins to upper board
      DIP40 - Socket for connector that joins to upper board
      AY3-8910 clock - 1.536 [18.432/12]
      HSync - 15.5kHz
      VSync - 60Hz

***************************************************************************/

ROM_START( majrjhdx ) // ROM test gives all ok
	ROM_REGION( 0x290000, "maincpu", 0 )
	ROM_LOAD( "1051d.5e",         0x00000, 0x40000, CRC(54c31732) SHA1(049e76c42fd248f975c7cce7e74b1f79e2a96bea) )
	ROM_LOAD( "tmp91p640n-10.3c", 0x00000, 0x04000, CRC(129a11c7) SHA1(450a6a7da29c9206937a16701b34075cda338147) ) // dump from majrjh, works fine
	ROM_COPY( "maincpu", 0x00000, 0x10000, 0x40000 )
	ROM_COPY( "maincpu", 0x00000, 0x50000, 0x40000 )
	ROM_COPY( "maincpu", 0x10000, 0x90000, 0x80000 )
	ROM_LOAD( "1053d.3e",  0x110000, 0x80000, CRC(10bf7f0f) SHA1(c042240296ac7202da14e809bff36c9b0f97a3df) )
	ROM_LOAD( "1052d.4e",  0x210000, 0x80000, CRC(7200599c) SHA1(32e7caad9a9ea756b699f601fab90a419a437f57) )

	ROM_REGION( 0x400, "proms", 0 ) // Color PROMs
	ROM_LOAD( "d105-2.7e", 0x000, 0x200, CRC(587bca5a) SHA1(327f7bfa035f652bbbfba3f74715515236322c09) )
	ROM_LOAD( "d105-1.6e", 0x200, 0x200, CRC(6d0ce028) SHA1(35f70000a850782356734323fa93b150a77f807c) )
ROM_END

ROM_START( majrjh ) // ROM test gives all ok
	ROM_REGION( 0x290000, "maincpu", 0 )
	ROM_LOAD( "1051d.5e",         0x000000, 0x80000, CRC(5fdc5f9e) SHA1(4a599f83ee1c8ae41a44e98694b6d5515a29b813) ) // 1ST AND 2ND HALF IDENTICAL, tested as 2MB anyway
	ROM_LOAD( "tmp91p640n-10.3c", 0x000000, 0x04000, CRC(129a11c7) SHA1(450a6a7da29c9206937a16701b34075cda338147) ) // MCU has pins 9 to 10 & 12 to 15 stripped out
	ROM_COPY( "maincpu", 0x00000, 0x010000, 0x80000 )
	ROM_COPY( "maincpu", 0x00000, 0x090000, 0x80000 )
	ROM_LOAD( "1053d.3e",         0x110000, 0x80000, CRC(e5abd309) SHA1(7d80ab9f7bcc66d7332c60a0d02c123582c31a34) )
	ROM_LOAD( "1052d.4e",         0x210000, 0x80000, CRC(7200599c) SHA1(32e7caad9a9ea756b699f601fab90a419a437f57) )

	ROM_REGION( 0x400, "proms", 0 ) // Color PROMs
	ROM_LOAD( "d105-2.7e", 0x000, 0x200, CRC(587bca5a) SHA1(327f7bfa035f652bbbfba3f74715515236322c09) )
	ROM_LOAD( "d105-1.6e", 0x200, 0x200, CRC(6d0ce028) SHA1(35f70000a850782356734323fa93b150a77f807c) )
ROM_END

/***************************************************************************

Almond Pinky
Dynax, 1988

This game runs on Royal Mahjong hardware.
It appears Royal Mahjong was originally manufactured by Nichibutsu
This PCB says "(C) 1983 Nichibutsu" on it.

Top PCB
-------
D1401128L-0
|------------------------------------------|
|         |---------|                      |
|DIP40    |         |                      |
| Z80A    |   &     |                      |
|         |---------|                      |
|     8MHz                           DSW1  |
|                                          |
|                                          |
|                                          |
|                                          |
|                                    DSW2  |
|                                          |
|                                          |
|                                          |
|DYNAX DYNAX DYNAX DYNAX DYNAX DYNAX       |
| 146   145   144   143   142   141  6116 *|
|------------------------------------------|
Notes:
      Every chip has its surface scratched
      *     - 4 pin power connector joined to main PCB
      DSWx  - have 8 switches each
      DIP40 - Socket joins to main PCB
      &     - Large Dynax ceramic SIP module (DAC or similar)
      Z80   - clock 4MHz [8/2]
      All ROMs type 27512

Note! On Royal Mahjong-based PCBs where there is a 3 pin RGB connector (3 wires) tied to the main
board and joining to the top board, the color PROM is located on the top daughterboard. Usually that
chip is an 82S123 (32 bytes), 82S129 (256 bytes) or 82S147 (512 bytes). In any case, you can be sure
there is a PROM(s) on the PCB somewhere if the RGB connector cable is present.


Main PCB
--------
RM-1D (C) Nichibutsu 1983
|------------------------------------------------|
|  6116 DIP24  P6    P5    P4    P3     P2    P1 |
|MB3712                                          |
|     BATTERY                                    |
|              DSW(8)                    DIP40   |
|                                                |
|1                                      18.432MHz|
|8    TBP18S030.6K                               |
|W                                               |
|A                                               |
|Y                                               |
|   AY3-8910                                     |
|            4116    4116                        |
|1           4116    4116                        |
|0           4116    4116                        |
|W           4116    4116                        |
|A           4116    4116                        |
|Y           4116    4116                        |
|            4116    4116                        |
|            4116    4116                        |
|------------------------------------------------|
Notes:
      DIP40 - Socket joins to top PCB
      DIP24 - Unpopulated DIP24 position (no socket)
      TBP18S030.6K - Color PROM (32 bytes)
      P1-P6 - Program ROM sockets (DIP24)

***************************************************************************/

ROM_START( mjapinky )
	ROM_REGION( 0x90000, "maincpu", 0 )
	ROM_LOAD( "141.4d",     0x00000, 0x10000, CRC(0c4fb83a) SHA1(5d467e8fae715ca4acf88f8e9437c7cdf9f876bd) )
	// bank switched ROMs follow
	ROM_RELOAD(             0x10000, 0x10000 )
	ROM_LOAD( "142.4e",     0x20000, 0x10000, CRC(129806f0) SHA1(d12d2c5bb0c653f2e4974c47004ada128ac30bea) )
	ROM_LOAD( "143.4f",     0x30000, 0x10000, CRC(3d0bc452) SHA1(ad61eaa892121f90f31a6baf83158a11e6051430) )
	ROM_LOAD( "144.4h",     0x40000, 0x10000, CRC(24509a18) SHA1(ab9daed2cbc72d02c2168a4c93f70ebfe3916ea2) )
	ROM_LOAD( "145.4j",     0x50000, 0x10000, CRC(fea3375a) SHA1(cbb89b72cfba9c0448d152dfdbedb20b9896516e) )
	ROM_LOAD( "146.4k",     0x60000, 0x10000, CRC(be27a9b9) SHA1(f12402182f598391e445245b345f49084a69620a) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "18s030n.clr",   0x0000, 0x0020, CRC(5736d0aa) SHA1(298b51340d2697347842cfaa5921f31c7b7f9748) )
ROM_END

/***************************************************************************

Mahjong Cafe Time
(c)1992 Dynax
Modified Royal Mahjong PCB
D6310128L1-1 (Sub PCB)

CPU: Z80
Sound: AY-3-8910

ROMs:
6301.2E      [1fc10e7c]
6302.3E      [02bbdf78]
6303.5E      [0e71eea8]
6304.6E      [53c581d6]

D63-1.7F     [e7410136] MB7124H
D63-2.8F     [af735b42] /

***************************************************************************/

ROM_START( cafetime )
	ROM_REGION( 0x210000, "maincpu", 0 )
	ROM_LOAD( "6301.2e", 0x000000, 0x40000, CRC(1fc10e7c) SHA1(0ed6bfd4cc6fc64bbf55bd3c6bde2d8ba9da2afb) )
	// bank switched ROMs follow
	ROM_RELOAD(          0x010000, 0x40000 )
	ROM_RELOAD(          0x050000, 0x40000 )
	ROM_LOAD( "6302.3e", 0x090000, 0x80000, CRC(02bbdf78) SHA1(e1e107541236ed92854fac4e12c9b300dbac9822) )
	ROM_LOAD( "6303.5e", 0x110000, 0x80000, CRC(0e71eea8) SHA1(f95c3b7acee6deabff4aca83b490e255648e2f19) )
	ROM_LOAD( "6304.6e", 0x190000, 0x80000, CRC(53c581d6) SHA1(d9cfda63a8f2e92873f69c673d3efe5c22cfa0de) )

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "d63-2.8f", 0x000, 0x200, CRC(af735b42) SHA1(deddde3e276d5b9de72e267f65399d80783c6244) )
	ROM_LOAD( "d63-1.7f", 0x200, 0x200, CRC(e7410136) SHA1(54d3aec0d11485d4f419e76f9c4071ab9b817937) )
ROM_END

/***************************************************************************

Mahjong Cafe Doll
Dynax, 1993

This game runs on Royal Mahjong hardware.

Top PCB
-------
D76052208L-2
|-----------------------------------|
|   7601  6264        RTC   BATTERY |
|DIP40                              |
|   7602                  8MHz      |
|                PAL           PAL  |
|   7603           |-----------|    |
|                  |    CPU    |    |
|   DIP32          |-----------|    |
|                                   |
|                                   |
|                                   |
|                                   |
|                                   |
| 82s147.7F                        &|
|*                      DSW3   DSW1 |
| 82S147.9F  %          DSW4   DSW2 |
|-----------------------------------|
Notes:
      Every chip has its surface scratched, except the PROMs
      *     - Connector joined to main PCB
      &     - Power input connector
      %     - RGB Video output
      DIP32 - Empty DIP32 socket
      DSWx  - have 10 switches each
      DIP40 - Socket joins to main PCB
      CPU   - unknown SDIP64 chip. Possibly TMP90P640 or similar TLCS-90 type CPU


Main PCB
--------
no pcb number
|------------------------------------------------|
|   6116 DIP24 DIP24 DIP24 DIP28 DIP28 DIP28     |
|        DIP24                                   |
|HA1368                                          |
|                                        DIP40   |
|                                                |
|             DSW(8)                    18.432MHz|
|M                                               |
|A                                               |
|H                                               |
|J                                               |
|O  AY3-8910                                     |
|N           4116    4116                        |
|G           4116    4116                        |
|2           4116    4116                        |
|8           4116    4116                        |
|            4116    4116                        |
|            4116    4116                        |
|            4116    4116                        |
|            4116    4116                        |
|------------------------------------------------|
Notes:
      DIP40 - Sockets joins to top PCB
      DIP24/28 - Unpopulated sockets

***************************************************************************/

ROM_START( cafedoll )
	ROM_REGION( 0x190000, "maincpu", 0 )
	ROM_LOAD( "7601",              0x000000, 0x80000, CRC(20c80ad9) SHA1(e45edd101c6e26c0fa3c3f15f4a4152a853e41bd) )
	ROM_LOAD( "76xx.tmp90840",     0x000000, 0x02000, CRC(091a85dc) SHA1(964ccbc13466464c2feee10f807078ec517bed5c) ) // internal ROM, MCU has pins 10 & 12 to 16 stripped out
	// bank switched ROMs follow
	ROM_COPY( "maincpu", 0x000000, 0x010000, 0x80000 )
	ROM_LOAD( "7602",              0x090000, 0x80000, CRC(f472960c) SHA1(cc2feb4374ba94035101114c73e1690cfeac9b91) )
	ROM_LOAD( "7603",              0x110000, 0x80000, CRC(c4293019) SHA1(afd717844e9e681ada14e80cd10dce0ed60d4259) )

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "d76-2_82s147.9f", 0x000, 0x200, CRC(9c1d0512) SHA1(3ca82d4271badc890701ecc76b97e80b16509b50) )
	ROM_LOAD( "d76-1_82s147.7f", 0x200, 0x200, CRC(9a75349c) SHA1(2071132267aafd8facf1d7841093d9a45c30a8d3) )
ROM_END

ROM_START( cafedollg ) // カフェドール グレート (Cafe Doll Great) sticker on PCB, G appended to the main program ROM, but still boots as standard Cafe Doll?
	ROM_REGION( 0x190000, "maincpu", 0 )
	ROM_LOAD( "7601g",             0x000000, 0x080000, CRC(e42779bf) SHA1(0a0d8f74da8c0d3b6349f3528b008642aa3efe9c) )
	ROM_LOAD( "76xx.tmp90840",     0x000000, 0x002000, BAD_DUMP CRC(091a85dc) SHA1(964ccbc13466464c2feee10f807078ec517bed5c) ) // internal ROM, MCU has pins 10 & 12 to 16 stripped out, not dumped for this set but verified on PCB that it works
	// bank switched ROMs follow
	ROM_COPY( "maincpu", 0x000000, 0x010000, 0x080000 )
	ROM_LOAD( "7602",              0x090000, 0x100000, CRC(23fd53c4) SHA1(af75b6e9bf5efe77574861bfc0595824abe99d18) ) // same data of 7602 + 7603 of the parent set

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "d76-2_82s147.9f", 0x000, 0x200, CRC(9c1d0512) SHA1(3ca82d4271badc890701ecc76b97e80b16509b50) )
	ROM_LOAD( "d76-1_82s147.7f", 0x200, 0x200, CRC(9a75349c) SHA1(2071132267aafd8facf1d7841093d9a45c30a8d3) )
ROM_END

/***************************************************************************

Mahjong Cafe Paradise
1999 Techno-Top

Royal Mahjong board. No ROMs on the base board.

Top board looks like typical Dynax with scratched SDIP64.
It is marked 'TSS001-0001 - Techno Top Limited' and has just 2 EPROMs and 2 PROMs.
Everything else is scratched but there's a 32.768kHz OSC, RTC and connected battery.
Also, 4 DIP sw each with 10 switches and an 8MHz OSC next to the SDIP64 chip,
and a PLCC68 chip (likely FPGA)

***************************************************************************/

ROM_START( cafepara )
	ROM_REGION( 0x290000, "maincpu", 0 )
	// VIDEO & AM MICRO COMPUTER SYSTEMS 1999 TECHNO-TOP,LIMITED NAGOYA JAPAN MAHJONG CAFE PARADISE TSS001 VER. 1.00
	ROM_LOAD( "00101.1h",           0x000000, 0x080000, CRC(f5917280) SHA1(e6180e36643075ab9fa5bc27baef2a464a23f581) ) // external ROM with first 0x4000 empty
	ROM_LOAD( "cafepara.tmp91640",  0x000000, 0x004000, CRC(0575607c) SHA1(e641ffd1bd44f2b4a0cdf72c49990933a0f0ff22) ) // internal ROM

	// bank switched ROMs follow (test mode checks 0x50 banks)
	ROM_COPY( "maincpu",  0x000000, 0x010000, 0x080000 )
	ROM_LOAD( "00102.1d",           0x090000, 0x200000, CRC(ed3b5447) SHA1(ac24e9c00c94c35d2b2ec35f0c4262ceeda5408f) )

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "ts001b.4h", 0x000, 0x200, CRC(b0019654) SHA1(78ba9b35744849c430f99137ea0da3d5564cc72a) )
	ROM_LOAD( "ts001a.4j", 0x200, 0x200, CRC(e89d4db0) SHA1(ff191a76fe1144e72a1cf3769f0156adf2d0507f) )
ROM_END

/***************************************************************************

Mahjong Vegas
Dynax, 199?

This game runs on Royal Mahjong hardware.

Top PCB
-------
D5011308L1
|-----------------------------------|
| DIP32 DIP32 5003  5002 DIP32 5001A|
|DIP40                          62XX|
|                         32.768kHz |
|                             RTC   |
|                   |-----------|   |
|                   |    CPU    |   |
|                   |-----------|   |
|DSW4   DSW2                        |
|    DSW3   DSW1           PAL      |
|                                   |
|                                   |
|                                   |
|    D50-2                          |
|*      D50-1            8MHz       |
|  %         &             BATTERY  |
|-----------------------------------|
Notes:
      Every chip has its surface scratched
      *     - Cable connector joined to main PCB (to original PROM socket on main board)
      %     - RGB Video output
      &     - +12V input to top PCB
      DIP32 - Empty DIP32 socket
      DSWx  - have 10 switches each
      DIP40 - Socket joins to main PCB
      CPU   - unknown SDIP64 chip. Possibly TMP90P640 or similar TLCS-90 type CPU
              Pins 9-14 have been broken off and removed!
      62XX  - 6264 or 62256 SRAM
      D50-* - 82S147 color PROMs


Main PCB
--------
FRM-00 (with Falcon Logo.... PCB is made by Falcon)
|------------------------------------------------|
|  6116 DIP24 ROM6  ROM5  ROM4  ROM3  ROM2  ROM1 |
|HA1368                                          |
|                                                |
|     VOL                                DIP40   |
|                                                |
|             DSW(8)                    18.432MHz|
|M                                               |
|A                                               |
|H                                               |
|J                                               |
|O  AY3-8910                                     |
|N           4116    4116                        |
|G           4116    4116                        |
|2           4116    4116                        |
|8           4116    4116                        |
|            4116    4116                        |
|            4116    4116                        |
|            4116    4116                        |
|            4116    4116                        |
|------------------------------------------------|
Notes:
      DIP40 - Socket joins to top PCB
      DIP24 - Unpopulated socket
      AY-3-8910 - clock 1.536MHz (18.432/12]
      ROM* - Unpopulated DIP24 sockets

***************************************************************************/

ROM_START( mjvegas )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF  )
	ROM_LOAD( "5001a.1b",          0x00000, 0x20000, CRC(91859a47) SHA1(3c452405bf28f5e7302eaccdf472e91b64629a67) ) // external ROM with first 0x2000 empty
	ROM_LOAD( "50xx.tmp90840",     0x00000, 0x02000, CRC(091a85dc) SHA1(964ccbc13466464c2feee10f807078ec517bed5c) ) // internal ROM, MCU has pins 9 to 14 stripped out

	// bank switched ROMs follow
	ROM_COPY( "maincpu", 0x000000, 0x070000, 0x020000 )   // 0c-0f
	ROM_LOAD( "5002.1d",           0x210000, 0x80000, CRC(016c0a32) SHA1(5c5fdd631eacb36a0ee7dba9e070c2d3d3d8fd5b) ) // 40-4f
	ROM_LOAD( "5003.1e",           0x2f0000, 0x20000, CRC(5323cc85) SHA1(58b75ba560f05a0568024f52ee89f54713219452) ) // 5c-5f

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "d50-2_82s147.4h", 0x000, 0x200, CRC(3c960ea2) SHA1(65e05e3f129e9e6fcb14b7d44a75a76919c54d52) )
	ROM_LOAD( "d50-1_82s147.4g", 0x200, 0x200, CRC(50c0d0ec) SHA1(222899456cd2e15391d8d0f771bbd5e5333d6ba3) )
ROM_END

ROM_START( mjvegasa )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )    // 100 banks
	ROM_LOAD( "5040.1b", 0x00000, 0x20000, CRC(c4f03128) SHA1(758567f74de333207dfe6c1cb72b2afffb0c8f4b) )
	// bank switched ROMs follow
	ROM_RELOAD(           0x070000, 0x20000 )   // 0c-0f
	ROM_LOAD( "5002.1d",  0x210000, 0x80000, CRC(016c0a32) SHA1(5c5fdd631eacb36a0ee7dba9e070c2d3d3d8fd5b) ) // 40-4f
	ROM_LOAD( "5003.1e",  0x2f0000, 0x20000, CRC(5323cc85) SHA1(58b75ba560f05a0568024f52ee89f54713219452) ) // 5c-5f

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "d50-2_82s147.4h", 0x000, 0x200, CRC(3c960ea2) SHA1(65e05e3f129e9e6fcb14b7d44a75a76919c54d52) )
	ROM_LOAD( "d50-1_82s147.4g", 0x200, 0x200, CRC(50c0d0ec) SHA1(222899456cd2e15391d8d0f771bbd5e5333d6ba3) )
ROM_END

/***************************************************************************

Mahjong Shinkirou Deja Vu (+ some ROMs from Jan Oh (Toaplan) !?)

This game runs on Royal Mahjong hardware.

Top PCB
-------
D210301BL2
|-----------------------------------|
| DIP32 DIP32 2104  2103  2102 2101 |
|DIP40                          6116|
|                                   |
|DSW3                               |
|  |-----------|                    |
|  |    CPU    |                    |
|  |-----------|                    |
|DSW4                               |
|             8MHz                  |
|                         DSW2  DSW1|
|                                   |
|                                   |
|                                   |
|*                                  |
|  %                                |
|-----------------------------------|
Notes:
      Every chip has its surface scratched
      *     - Connector joined to main PCB
      %     - RGB Video output
      DIP32 - Empty DIP32 socket
      DSWx  - have 8 switches each
      DIP40 - Socket joins to main PCB
      CPU   - unknown SDIP64 chip. Possibly TMP90P640 or similar TLCS-90 type CPU


Main PCB
--------
FRM-00
|------------------------------------------------|
|  6116 DIP24 ROM6  ROM5  ROM4  ROM3  ROM2  ROM1 |
|HA1368                                          |
|                                                |
|                                        DIP40   |
|                                                |
|             DSW(8)                    18.432MHz|
|M                                               |
|A                                               |
|H                                               |
|J                                               |
|O  AY3-8910                                     |
|N           4116    4116                        |
|G           4116    4116                        |
|2           4116    4116                        |
|8           4116    4116                        |
|            4116    4116                        |
|            4116    4116                        |
|            4116    4116                        |
|            4116    4116                        |
|------------------------------------------------|
Notes:
      DIP40 - Sockets joins to top PCB
      DIP24 - Unpopulated socket

***************************************************************************/

ROM_START( mjdejavu )
	ROM_REGION( 0xd0000, "maincpu", 0 )
	ROM_LOAD( "2101.1b", 0x00000, 0x10000, CRC(b0426ea7) SHA1(ac39cbf5d78acdaa4b01d948917965c3aa2761b8) )
	// bank switched ROMs follow
	ROM_RELOAD(          0x10000, 0x08000 )
	ROM_CONTINUE(        0x10000, 0x08000 ) // 0
	// unused
	ROM_LOAD( "2103.1d", 0x30000, 0x20000, CRC(ed5fde4b) SHA1(d55487ae1007d43b71f06ae5c407c75db7054515) )   // 8
	// unused
	ROM_LOAD( "2104.1e", 0x70000, 0x20000, CRC(cfb8075d) SHA1(31f613a1a9b5f4295b552aeeddb760605ce2ac70) )   // 18
	// unused
	ROM_LOAD( "2102.1c", 0xb0000, 0x20000, CRC(f461e422) SHA1(c3505feb32650fdd5c0d7f30faed69b65d94937a) )   // 28

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "82s147.4d", 0x000, 0x200, CRC(d43f4c7c) SHA1(117d2e4e8d5bea3e5dc903a4b87bd71786ae009c) )
	ROM_LOAD( "82s147.4c", 0x200, 0x200, CRC(30cf7831) SHA1(b4593d51c6ceb301279a01a98665e4be8a3c403d) )
ROM_END

ROM_START( mjdejav2 )
	ROM_REGION( 0xd0000, "maincpu", 0 )
	ROM_LOAD( "210a.1b", 0x00000, 0x10000, CRC(caa5c267) SHA1(c779f9217f56d9d3b1ee9fadca07f7917d203e8e) )
	// bank switched ROMs follow
	ROM_RELOAD(          0x10000, 0x08000 )
	ROM_CONTINUE(        0x10000, 0x08000 ) // 0
	// unused
	ROM_LOAD( "2103.1d", 0x30000, 0x20000, CRC(ed5fde4b) SHA1(d55487ae1007d43b71f06ae5c407c75db7054515) )   // 8
	// unused
	ROM_LOAD( "2104.1e", 0x70000, 0x20000, CRC(cfb8075d) SHA1(31f613a1a9b5f4295b552aeeddb760605ce2ac70) )   // 18
	// unused
	ROM_LOAD( "210b.1c", 0xb0000, 0x20000, CRC(d4383830) SHA1(491333277e5e2341d1c1cc20f8cc32aa6b020b6c) )   // 28

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "82s147.4d", 0x000, 0x200, CRC(d43f4c7c) SHA1(117d2e4e8d5bea3e5dc903a4b87bd71786ae009c) )
	ROM_LOAD( "82s147.4c", 0x200, 0x200, CRC(30cf7831) SHA1(b4593d51c6ceb301279a01a98665e4be8a3c403d) )
ROM_END

// Incomplete romset (missing rom7 at $6000): "Jan Oh" by Toaplan, on royalmah hardware (try pc=64f).
ROM_START( janoh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rom1.p1",  0x0000, 0x1000, CRC(8fc19963) SHA1(309e941c059a97b117090fd9dd69a00031aa6109) )    // "1984 JAN OH"
	ROM_LOAD( "rom2.p12", 0x1000, 0x1000, CRC(e1141ae1) SHA1(38f7a71b367a607bb20a5cbe62e7c87c96c6997c) )
	ROM_LOAD( "rom3.p2",  0x2000, 0x1000, CRC(66e6d2f4) SHA1(d7e00e5bfee60daf844c46d36b1f4860fba70759) )    // "JANOH TOAPLAN 84"
	ROM_LOAD( "rom4.p3",  0x3000, 0x1000, CRC(9186f02c) SHA1(b7dc2d6c19e67dd3f841cbb56df9589e3e6941f7) )
	ROM_LOAD( "rom5.p4",  0x4000, 0x1000, CRC(f3c478a8) SHA1(02a8504457cbcdd3e67e7f5ba60fb789f198a51d) )
	ROM_LOAD( "rom6.p5",  0x5000, 0x1000, CRC(92687327) SHA1(4fafba5881dca2a147616d94dd055eba6aa3c653) )
	ROM_LOAD( "rom7.p6",  0x6000, 0x1000, NO_DUMP )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "janho.color", 0x00, 0x20, NO_DUMP )
ROM_END

/***************************************************************************

Mahjong Cafe Break
Dynax/Nakanihon, 199?

This game runs on Royal Mahjong hardware with a Nakanihon top board

Top PCB Layout
--------------

NS528-9812
|-------------------------|
|X  NS528A2               |
|   NS528B2  PLCC84 528011|
|RGB                 DIP32|
|          8MHz      DIP32|
|DSW4(10)                 |
|DSW3(10)   SDIP64  6264  |
|DSW2(10)                 |
|DSW1(10)           52802 |
|                 A3      |
|BATTERY     A1   A2  Y A4|
|-------------------------|
Notes:
      RGB 3  - wire cable tied to mainboard
      X      - DIP16 socket with flat cable plugged in coming from main board PROM socket
      PLCC84 - unknown PLCC84 in a socket
      DIP32  - unpopulated DIP32 socket
      SDIP64 - unknown CPU, probably TLCS-90 (TMP91640)
      A1     - unknown DIP8 IC, possibly MB3771 reset/watchdog chip
      A2/A3  - unknown DIP14 ICs, probably logic
      A4     - unknown DIP18 IC, RTC IC
      Y      - 32.768kHz OSC for RTC

***************************************************************************/

ROM_START( cafebrk )
	ROM_REGION( 0x290000, "maincpu", 0 )
	ROM_LOAD( "528011.1f",         0x000000, 0x080000, CRC(440ae60b) SHA1(c24efd76ba73adcb614b1974e8f92592800ba53c) )
	ROM_LOAD( "528.tmp91640",      0x000000, 0x004000, CRC(0575607c) SHA1(e641ffd1bd44f2b4a0cdf72c49990933a0f0ff22) ) // internal ROM

	// bank switched ROMs follow
	ROM_COPY( "maincpu", 0x000000, 0x010000, 0x080000 )
	ROM_LOAD( "52802.1d",          0x090000, 0x200000, CRC(bf4760fc) SHA1(d54ab9e298800a31d95a5f8b98ab9ba5b2866acf) )

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "ns528b2.4h", 0x000, 0x200, CRC(5699e69a) SHA1(fe13b93dd2c4a16865b4edcb0fee1390fdade725) )
	ROM_LOAD( "ns528a2.4j", 0x200, 0x200, CRC(b5a3a569) SHA1(8e31c600ae24b672b614908ee920a333ed600941) )
ROM_END

/*

Janou
(c)1985 Toaplan (distributed by SNK)

RM-1C (modified Royal Mahjong hardware)

CPU: Z80x2 (on subboard)
Sound: AY-3-8910
OSC: 18.432MHz

ROMs:
JO1
JO2
JO3
JO4
JO5
JO6
JO7


Subboard GX002A
C8 (2764)
18S030.44
18S030.45

HM6116
MSM2128x4


dumped by sayu
--- Team Japump!!! ---

*/

ROM_START( janoha )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jo1",       0x0000, 0x1000, CRC(1a7dd28d) SHA1(347085c2b305861e4a4a602c3b3b0c57889f7f45) )
	ROM_LOAD( "jo2",       0x1000, 0x1000, CRC(e92ca79f) SHA1(9714ebee954dd98cf98b340e1dc424a4b2a78c36) )
	ROM_LOAD( "jo3",       0x2000, 0x1000, CRC(8e349cac) SHA1(27442fc97750ceb6e928682ee545a9ebff4511ac) )
	ROM_LOAD( "jo4",       0x3000, 0x1000, CRC(f2bcac9a) SHA1(46eea014edf9f260b35b5f9bd0fd0a0236da16ef) )
	ROM_LOAD( "jo5",       0x4000, 0x1000, CRC(16c09c73) SHA1(ea712f9ca3200ca27434e4200187b488e24f4c65) )
	ROM_LOAD( "jo6",       0x5000, 0x1000, CRC(92687327) SHA1(4fafba5881dca2a147616d94dd055eba6aa3c653) )
	ROM_LOAD( "jo7",       0x6000, 0x1000, CRC(f9a3fea6) SHA1(898c030b34f7432568e080e2814619d836d98a2f) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "c8",       0x0000, 0x2000, CRC(a37ed493) SHA1(a3246c635ee77f96afd96285ef7091f6fc0d7636) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "18s030.44",  0x0000, 0x0020, CRC(d4eabf78) SHA1(f14778b552ff483e36e7c30ee67e8e2075790ea2) )
	ROM_LOAD( "18s030.45",  0x0020, 0x0020, CRC(c6a24ae9) SHA1(ec7a4dee2fec2f7151ddc39e40a3eee6a1c4992d) ) // another color PROM?
ROM_END

/*

Mahjong Shiyou (BET type)
(c)1986 Visco

Board:  S-0086-001-00
CPU:    Z80-A x2
Sound:  AY-3-8910
        M5205
OSC:    18.432MHz
        400KHz


1.1K       Z80#2 prg.
2.1G

3.3G       Z80#1 prg.
4.3F

COLOR.BPR  color

*/

ROM_START( mjsiyoub )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "3.3g", 0x00000, 0x8000, CRC(47d0f16e) SHA1(a125be052668ba93756bf940af31a10e91a3d307) )
	ROM_LOAD( "4.3f", 0x08000, 0x8000, CRC(6cd6a200) SHA1(1c53e5caacdb9c660bd98f5331bf5354581f74c9) )

	// encrypted Z80
	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "1.1k", 0x00000, 0x8000, CRC(a1083321) SHA1(b36772e90be60270234df16cf92d87f8d950190d) )
	ROM_LOAD( "2.1g", 0x08000, 0x4000, CRC(cfe5de1d) SHA1(4acf9a752aa3c02b0889b0b49d3744359fa24460) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "color.bpr", 0x00, 0x20,  CRC(d21367e5) SHA1(b28321ac8f99abfebe2ef4da0c751cefe9f3f3b6) )
ROM_END

/*

Mahjong Senka
(c)1986 Visco

Modified Royal Mahjong Hardware

CPU: Z80
Sound: AY-3-8910
OSC: 18.432MHz
Others: Battery

ROMs:
1
2
3
4
1.2L (N82S129N)
2.2K (N82S123N)
3.1D (N82S129N)
4.8K (N82S123N) - color PROM


dumped by sayu

--- Team Japump!!! ---
http://japump.i.am/

*/

ROM_START( mjsenka )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "3",       0x0000, 0x4000, CRC(b2d8be1f) SHA1(da75e1072d271de2dbd897a551f6c32593f6421b) )
	ROM_LOAD( "4",       0x4000, 0x2000, CRC(e9e84999) SHA1(7b5f0edd92cf3a45e85055460e6cb00b154fd152) )
	ROM_LOAD( "1",       0x6000, 0x2000, CRC(83e943d1) SHA1(c4f9b5036627ccb369e7db03a743e496b149af85) )
	ROM_LOAD( "2",       0x8000, 0x2000, CRC(cdb02fc5) SHA1(5de6b15b79ea7c4246a294b17f166e53be6a4abc) )

	ROM_REGION( 0x10000, "decrypted", ROMREGION_ERASE00 )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "4.8k",  0x0000, 0x0020, CRC(41bd4d69) SHA1(4d2da761b338b62b2ea151c201063a24d6e4cc97) )

	ROM_REGION( 0x0220, "user1", 0 ) //?
	ROM_LOAD( "1.2l",  0x0000, 0x0100, CRC(24599429) SHA1(6c93bb2e7bc9902cace0c9d482fc1584c4c1a114) )
	ROM_LOAD( "3.1d",  0x0100, 0x0100, CRC(86aeafd1) SHA1(c4e5c56ce5baf2be3962675ae333e28bd8108a00) )
	ROM_LOAD( "2.2k",  0x0200, 0x0020, CRC(46014727) SHA1(eec451f292ee319fa6bfbbf223aaa12b231692c1) )
ROM_END

/*

Mahjong Yarou
(c)1986 Visco/Video System

FRM-00 (modified Royal Mahjong hardware)

CPU: Z80 (on subboard)
Sound: AY-3-8910
OSC: 18.432MHz

ROMs:
1(2732)
2(2732)
3(2732)
4(2732)
5(2732)
6(2732)
4.6K (18S030) - pin14 is connected to subboard's WS
                pin9 is not inserted to the socket

Subboard:
7(2764)
8(2764)
N82S129N.IC4
N82S123N.IC7
N82S129N.IC15

Connector between mainboard and subboard
sub - main
 CK - LS368 (1K) pin12
 HD - LS08  (2E) pin1
 VD - LS08  (2E) pin2
 WS - 18S030(6K) pin14
 () - LS138 (3K) pin13


Mainboard
----------------------------------------------------------
    1         2       3       4       5        6       7
A 74LS04    74LS86  74LS153  MB8116  MB8116  74LS157
B 74LS161   74LS86  74LS153  MB8116  MB8116  74LS95
C 74LS161   74LS86  74LS153  MB8116  MB8116  74LS157
D 74LS74    74LS86  74LS153  MB8116  MB8116  74LS95    8
E 74LS161   74LS08  74LS153  MB8116  MB8116  74LS157   9
F 74LS161   74LS74  74LS00   MB8116  MB8116  74LS95    1
H 74LS74    74LS00  74LS175  MB8116  MB8116  74LS157   0
J 74LS107   74LS32  74LS10   MB8116  MB8116  74LS95
K 74LS368   74LS241 74LS138  74LS08  74LS174 4.6K
L 18.432MHz 74LS241 74LS138  74LS04  74LS244 74LS174
M (socket to subbd) 74LS367  74LS08  DIPSW   74LS368
N                   (74LS245)74LS138 74LS04  TC40H000P

  1     2     3     4     5     6                      6 B
                                                       1 A
                                                       1 T
                                                       6 T
----------------------------------------------------------

Subboard
-----------------------------------------------------------
74LS42(IC21)   ?(IC22)        ?(IC23)        74LS85(IC24)
74LS125(IC16)  74LS08(IC17)   74LS393(IC9)   82S129N(IC15)
74LS161(IC6)   82S123N(IC7)   74LS161(IC8)   74LS157(IC14)
82S129N(IC4)   74LS259(IC5)   74LS32(IC12)   74LS74(IC13)
7(IC2)                        PAL20X10(IC19) 74LS00(IC20)
8(IC3)                        74LS245(IC18)  DIPSW
                                             74LS32(IC11)
Z80A                                         74LS04(IC10)
                                             5pin connector
-----------------------------------------------------------


dumped by sayu

--- Team Japump!!! ---
http://japump.i.am/

*/

ROM_START( mjyarou )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.p1",       0x0000, 0x1000, CRC(312c3b29) SHA1(ec2e14b392cf761f0a7079376994418fd463a06c) ) // on main board
	ROM_LOAD( "2.p2",       0x1000, 0x1000, CRC(98f14097) SHA1(cd1f72d6effa50f95386dfc5fa9b5056d83e554f) ) // "
	ROM_LOAD( "3.p3",       0x2000, 0x1000, CRC(295dbf40) SHA1(d6ac7bd88da849e418e750e2c91a594f65bdff39) ) // "
	ROM_LOAD( "4.p4",       0x3000, 0x1000, CRC(a6a078c8) SHA1(936be36c7c938c705e7054a42c1908bb5a5ee1bb) ) // "
	ROM_LOAD( "5.p5",       0x4000, 0x1000, CRC(3179657e) SHA1(703fc57ae71554345754267c31809cf7af7f1639) ) // "
	ROM_LOAD( "6.p6",       0x5000, 0x1000, CRC(6ccc05b4) SHA1(6eefba6023673edd86e82a0ad861a4d8f7f6652b) ) // "
	ROM_LOAD( "7.ic2",      0x6000, 0x2000, CRC(de6bfdd8) SHA1(dd9727d4d7de4add48cde229b8aa194f0492af7e) ) // on sub board
	ROM_LOAD( "8.ic3",      0x8000, 0x2000, CRC(1adef246) SHA1(b5f5598daf71694effffbfb486b03fcda5a593ee) ) // "

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "4.6k",         0x0000, 0x0020, CRC(41bd4d69) SHA1(4d2da761b338b62b2ea151c201063a24d6e4cc97) ) // on main board

	ROM_REGION( 0x0220, "user1", 0 ) //?
	ROM_LOAD( "82s129n.ic15",  0x0000, 0x0100, CRC(86aeafd1) SHA1(c4e5c56ce5baf2be3962675ae333e28bd8108a00) ) // on sub board
	ROM_LOAD( "82s129n.ic4",   0x0100, 0x0100, CRC(f09d3c4c) SHA1(a9e752d75e7f3ebd05add4ccf2f9f15d8f9a8d15) ) // "
	ROM_LOAD( "82s123n.ic7",   0x0200, 0x0020, CRC(46014727) SHA1(eec451f292ee319fa6bfbbf223aaa12b231692c1) ) // on sub board
ROM_END

ROM_START( mjyarou2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "my_1",    0x0000, 0x1000, CRC(c303a013) SHA1(1360aa8657740036f017eba864eed2cbc3c9ad62) )
	ROM_LOAD( "my_2",    0x1000, 0x1000, CRC(0c7f87cc) SHA1(2e56656b2dd860471eab9dfcb1dfd4e0b23a8df9) )
	ROM_LOAD( "my_3",    0x2000, 0x1000, CRC(e94d5f9a) SHA1(c8365991aecf0b1d22c7ce38dd15f6d4a5f70cac) )
	ROM_LOAD( "my_4",    0x3000, 0x1000, CRC(a6a078c8) SHA1(936be36c7c938c705e7054a42c1908bb5a5ee1bb) )
	ROM_LOAD( "my_5",    0x4000, 0x1000, CRC(3179657e) SHA1(703fc57ae71554345754267c31809cf7af7f1639) )
	ROM_LOAD( "my_6",    0x5000, 0x1000, CRC(0ca1cedc) SHA1(bd5d54b185e6ff5633d59df4c06a2094fca9cbf2) )
	ROM_LOAD( "7",       0x6000, 0x2000, CRC(de6bfdd8) SHA1(dd9727d4d7de4add48cde229b8aa194f0492af7e) )
	ROM_LOAD( "8",       0x8000, 0x2000, CRC(1adef246) SHA1(b5f5598daf71694effffbfb486b03fcda5a593ee) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123",       0x0000, 0x0020, CRC(41bd4d69) SHA1(4d2da761b338b62b2ea151c201063a24d6e4cc97) )

	ROM_REGION( 0x0220, "user1", 0 ) //?
	ROM_LOAD( "82s129n.ic15",  0x0000, 0x0100, CRC(86aeafd1) SHA1(c4e5c56ce5baf2be3962675ae333e28bd8108a00) )  // not dumped for this board
	ROM_LOAD( "82s129n.ic4",   0x0100, 0x0100, CRC(f09d3c4c) SHA1(a9e752d75e7f3ebd05add4ccf2f9f15d8f9a8d15) )  // not dumped for this board
	ROM_LOAD( "82s123n.ic7",   0x0200, 0x0020, CRC(46014727) SHA1(eec451f292ee319fa6bfbbf223aaa12b231692c1) ) // not dumped for this board
ROM_END

/*

Jansou
(c)1985 Dyna Industry

upgrade kit for Royal Mahjong
G85-12-05RL

CPU: Z80A
OSC: 4.000MHz
Other: surface scratched 40pin DIP device

ROMs:
1
2
3
4
5

Color PROM:
N82S123AN


dumped by sayu
--- Team Japump!!! ---
http://japump.i.am/

*/
/*
Nothing can be done on this one due of missing main program ROM(s) (surface scratched 40pin DIP device).
A string of the current Z80 ROM at offset 0x90 says "THE Janso Voice Version 1.0 (c) Copy Right 1985 Dyna",
so it's just a voice player.
*/
ROM_START( jansou )
	//Missing main CPU program ROM
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "unk", 0x0000, 0x8000, NO_DUMP )

	// These probably hook up with the main CPU program, they are standard 4bpp bitmaps.
	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "3", 0x00000, 0x8000, CRC(75a0bef0) SHA1(c2f5b3ddc55b58d3ea784d8b3d0a0f577d313341) )
	ROM_LOAD( "4", 0x08000, 0x8000, CRC(7304899a) SHA1(636b7673563f75ff2ef95eef3b99f80ef0c45fee) )
	ROM_LOAD( "5", 0x10000, 0x8000, CRC(57a4d300) SHA1(35d211d50052cd76721dbd6ad02ec7cb56c475d1) )

	// this is just a Z80 Voice Player (and latches port I/O $00 with the main CPU)
	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "1", 0x00000, 0x8000, CRC(0321ac7e) SHA1(1a0372a25f979461db09cd153c15daaa556c3d1f) )
	ROM_LOAD( "2", 0x08000, 0x8000, CRC(fea7f3c6) SHA1(c196be0030b00cfb747b9dbfa387048d20c70b74) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "n82s123an", 0x0000, 0x0020, CRC(e9598146) SHA1(619e7eb76cc3e882b5b3e55cdd23fe00b0a1fe45) )
ROM_END

ROM_START( jansoua )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.bin",  0x0000, 0x1000, CRC(aa66a9fc) SHA1(e2a956f17d294e160e16297866cd9be117254ea4) )
	ROM_LOAD( "2.bin",  0x1000, 0x1000, CRC(3b6ef098) SHA1(eda181971153888e63aa14e10b0b199383f2d627) )
	ROM_LOAD( "3.bin",  0x2000, 0x1000, CRC(63070d44) SHA1(c9c08f774a94cfb4e291f3d7ef81b0f0f9f74460) )
	ROM_LOAD( "4.bin",  0x3000, 0x1000, CRC(2b14d3c1) SHA1(210d6f212bda7fb7225e5606b34f674cc5f85150) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "3s.bin", 0x00000, 0x8000, CRC(64df20f6) SHA1(6cbe4718d47b52c229863219dba3e1f964ba667a) )
	ROM_LOAD( "4s.bin", 0x08000, 0x8000, CRC(8ddc8258) SHA1(a97a5efd06965a70e34684986dd8538a35e43d31) )
	ROM_LOAD( "5s.bin", 0x10000, 0x8000, CRC(1745c996) SHA1(6905774b4bdd0bfcc34b847efb037f9d92884a6b) )

	// this is just a z80 Voice Player (and latches port I/O $00 with the main CPU)
	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "1",  0x00000, 0x8000, CRC(0321ac7e) SHA1(1a0372a25f979461db09cd153c15daaa556c3d1f) )
	ROM_LOAD( "2",  0x08000, 0x8000, CRC(fea7f3c6) SHA1(c196be0030b00cfb747b9dbfa387048d20c70b74) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "n82s123an", 0x0000, 0x0020, CRC(e9598146) SHA1(619e7eb76cc3e882b5b3e55cdd23fe00b0a1fe45) )
ROM_END

ROM_START( jangtaku )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "v13_rm_1.p1",  0x0000, 0x1000, CRC(fe761b2b) SHA1(7be1dab12295f1fa399e3d618644715f0300391b) )
	ROM_LOAD( "v13_rm_2.p2",  0x1000, 0x1000, CRC(888e97f5) SHA1(cef86f3f108bc40ea18d9b0676940d9340490aa9) )
	ROM_LOAD( "v13_rm_3.p3",  0x2000, 0x1000, CRC(1b428819) SHA1(a419017f9c077c0dd21000b3b9f949c47f20343e) )
	ROM_LOAD( "v13_rm_4.p4",  0x3000, 0x1000, CRC(72d8abdf) SHA1(286a4fe6899c53daeb1a656b94206f9a19ceaee0) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "3.d3", 0x00000, 0x8000, CRC(a865ac4c) SHA1(f8bff3b44a229ddb6165a986faa3de058001e285) )
	ROM_LOAD( "4.f3", 0x08000, 0x8000, CRC(7f4c7194) SHA1(f9bf6e12b3a89b3d25ad15a85432e22bf4e10a24) )
	ROM_LOAD( "5.g3", 0x10000, 0x8000, CRC(091df750) SHA1(6b7ad93bdfb6a7e11f923b084de3e713cf4d1dad) )

	// this is just a z80 Voice Player (and latches port I/O $00 with the main CPU)
	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "1.a3",  0x00000, 0x8000, CRC(745162d3) SHA1(145269c60c87e772e6cbca40213d286ec05c9134) )
	// THE Jantaku Voice   Ver 1.0     (C) Copy Right 1986             DYNA Computer Service CO.,LTD.  By Satoshi Kato

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123a.6k", 0x0000, 0x0020, CRC(e9598146) SHA1(619e7eb76cc3e882b5b3e55cdd23fe00b0a1fe45) )
ROM_END

ROM_START( rkjanoh2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pf_1",         0x000000, 0x001000, CRC(582e7eda) SHA1(96578b6142051d9452f23c8c1b674e2d8a4c3b62) )
	ROM_LOAD( "pf_2",         0x001000, 0x001000, CRC(49e7dc40) SHA1(d6232a82b6927c79dd47884e5e2a6589c5524424) )
	ROM_LOAD( "pf_3_1",       0x002000, 0x001000, CRC(a1fdc929) SHA1(27cab4da2365bcf311d7f00d75e8db150183b108) )
	ROM_LOAD( "pf_4l",        0x003000, 0x001000, CRC(c9ccdfa0) SHA1(ce6f2df7fb6739ddf0529bcae0596e4593ecc3e0) )
	//ROM_LOAD( "pf_4_fewest",  0x003000, 0x001000, CRC(9a1650a0) SHA1(2da5957879d9f207721fc2f0d63dccc32850cbe2) )
	//ROM_LOAD( "pf_4_middle",  0x003000, 0x001000, CRC(b1a721d8) SHA1(de24ec4bac7ec761c7b25a7ba62b850006444bbc) )
	ROM_LOAD( "pf_5",         0x004000, 0x001000, CRC(8a858464) SHA1(55c71ce1c30e908dfc8c21237256dfbb75c55363) )
	ROM_LOAD( "pf_6",         0x005000, 0x001000, CRC(5b649918) SHA1(191a221a515c261d90d7432443a7fbc8da71e7ac) )
	ROM_LOAD( "pf_7",         0x006000, 0x001000, CRC(c4fdd2ac) SHA1(76c5645534b87dde87acfb4140d0f3ba18c95cd2) )
	ROM_LOAD( "pf_8",         0x008000, 0x002000, CRC(c789e2b3) SHA1(33b5c8f22a1e337816a61fd2c91bc175a412d10e) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123",       0x000, 0x020, CRC(74a53e94) SHA1(ca9114bd9b2b07f5abe82616b41ae9fdb9537a4f) )
ROM_END

// video reference: https://www.youtube.com/watch?v=66KDyKQA1q0 (video of the original, this is a bootleg)
ROM_START( chalgirl ) // TODO: verify ROM load / banking, this is correct for starting the game but it needs finishing the game to check it.
	ROM_REGION( 0x10000, "maincpu", 0 ) // the first 6 ROMs are on the FRM-03 board, the following on the SK-1B sub board
	ROM_LOAD( "cg_11.o1",  0x0000, 0x1000, CRC(1c064d3e) SHA1(cfe7e536efb377f009d7c9bee5b5a814ad1404ad) ) // 2732
	ROM_LOAD( "cg_22.o2",  0x1000, 0x1000, CRC(3244fe61) SHA1(3163f14b8977fe4b05aa11d332347c5f3cc2fbfa) ) // 2732
	ROM_LOAD( "cg_33.o3",  0x2000, 0x1000, CRC(692ef940) SHA1(c5a54161bedf26695aedad66016b5a789dbe13e5) ) // 2732
	ROM_LOAD( "cg_4.o3",   0x3000, 0x1000, CRC(562aa45f) SHA1(fc052b8f3b2f105c9282468d385a9ad554ec00f5) ) // 2732
	ROM_LOAD( "cg_5.o4",   0x4000, 0x1000, CRC(c0849a41) SHA1(1d901d59248cf0d8beff03207c46e3c50d0011cf) ) // 2732
	ROM_LOAD( "cg_6.o4",   0x5000, 0x1000, CRC(92687327) SHA1(4fafba5881dca2a147616d94dd055eba6aa3c653) ) // 2732
	ROM_LOAD( "7.f1",      0x6000, 0x1000, CRC(465db6e1) SHA1(e08fe431ab8676372fc13c51a2684a0320e01031) ) // 2732
	ROM_LOAD( "10.b1",     0x8000, 0x2000, CRC(04e76413) SHA1(e85db89084a8ae448fde4e202e8516b2e14a8265) ) // 2764
	ROM_LOAD( "8.e1",      0xa000, 0x2000, CRC(42fafb6f) SHA1(32d06ee4fe28033ae04058d806917bf5b5690246) ) // 2764
	ROM_LOAD( "9.c1",      0xc000, 0x2000, CRC(8291e28a) SHA1(9a05cae191babdf3f78a7131985d8c6583f7c973) ) // 2764

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "f-rom.bpr",  0x0000, 0x0020, BAD_DUMP CRC(d3007282) SHA1(e4d863ab193e49208ed0f59dcddb1da0492314f6) ) // not dumped for this set, using the one from royalmah for now, girl colors don't seem correct
ROM_END


/***************************************************************************

Ichi Ban Jyan
Excel, 1993

PCB Layout
----------

MJ911
|----------------------------------|
|MB3712 DSW-D DSW-C DSW-B DSW-A  SW|
|   M6378                   BATT   |
|VOL               6264         3  |
|    YM2413 MJB                    |
|M                  1           2  |
|A  YM2149  MJG  |-------|         |
|H               |ALTERA |  Z80    |
|J          MJR  |EP1810 |         |
|O               |       |  ALTERA |
|N               |-------|  EP910  |
|G                                 |
|                                  |
|      41464  41464                |
|      41464  41464       18.432MHz|
|----------------------------------|
Notes:
Z80 clock - 6.144MHz [18.432/3]
YM2149 clock - 1.536MHz [18.432/12]
YM2413 clock - 3.072MHz [18.432/6]
M6378 - OKI MSM6378A Voice Synthesis IC with 256Kbit OTP ROM (DIP16) - not populated
VSync - 60.5686Hz
HSync - 15.510kHz

***************************************************************************/

ROM_START( ichiban ) // TODO: how does the banking work?
	ROM_REGION( 0x60000, "maincpu", 0 ) // opcodes in first half are mixed with pseudo-random garbage
	ROM_LOAD( "3.u15", 0x00000, 0x20000, CRC(76240568) SHA1(cf055d1eaae25661a49ec4722a2c7caca862e66a) )
	ROM_LOAD( "1.u28", 0x20000, 0x08000, CRC(2caa4d3f) SHA1(5e5af164880140b764c097a65388c22ba5ea572b) ) // bank 2 (title screen)
	ROM_IGNORE(0x18000)
	ROM_LOAD( "2.u14", 0x30000, 0x08000, CRC(b4834d8e) SHA1(836ddf7586dc5440faf88f5ec50a32265e9a0ec8) ) // bank 4 (mahjong tiles)
	ROM_IGNORE(0x18000)

	// 1.u28
	// 1st 0x8000 contain title screen
	// 2nd 0x8000 contain paytable
	// 3rd 0x8000 contain in-game background
	// 4th 0x8000 contain double up screen

	// 2.u14
	// 1st 0x8000 contain mahjong tiles (resulting in bad GFX for player's hand' tiles)
	// 2nd 0x8000 contain mahjong tiles
	// 3rd 0x8000 contain reels?
	// 4th 0x8000 empty


	ROM_REGION( 0x600, "proms", 0 )
	ROM_LOAD( "mjr.u36", 0x000, 0x200, CRC(31cd7a90) SHA1(1525ad19d748561a52626e4ab13df67d9bedf3b8) )
	ROM_LOAD( "mjg.u37", 0x200, 0x200, CRC(5b3562aa) SHA1(ada60d2a5a5a657d7b209d18a23b685305d9ff7b) )
	ROM_LOAD( "mjb.u38", 0x400, 0x200, CRC(0ef881cb) SHA1(44b61a443d683f5cb2d1b1a4f74d8a8f41021de5) )
ROM_END


/*
Pong Boo! 2 by OCT

PCB is etched
NEW PONG-BOO MADE IN JAPAN
OCT. Co,Ltd. All Rights Reserved

Main components
- 4x scratched chips. 1 is supposed to be the main CPU, possibly some kind of Z80 or Z180 based SoC?
- 1x 48.000 XTAL
- 1x OKI M6295 sound chip (there's also an unpopulated space marked for a YM2413)
- 1x CY7C185-15PC 8k x8 SRAM
- 1x CY7C199-15PC 32k x8 SRAM
- 1x HM6264ALSP-12 with a battery nearby
- 2x SN74LS174N
- 4x bank of 8 DIP-switches

This game shares the same programmer / programming team as Ichi Ban Jian by Excel.
It seems to use a similar split opcodes / data ROM arrangement.
*/

ROM_START( pongboo2 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "1a.1.u016", 0x00000, 0x80000, CRC(07a95a99) SHA1(10897f8f0d8799797a577dffc9ba36b8904bb6c4) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "2.u0513", 0x00000, 0x40000, CRC(97bafe5b) SHA1(bff4df17cdb2d3ee63cc88ff486a7f5964182a15) )
ROM_END

void royalmah_prgbank_state::init_tahjong()
{
	m_mainbank->configure_entries(0, 2, memregion("maincpu")->base() + 0x10000, 0x4000);
}

void royalmah_state::init_jansou()
{
	save_item(NAME(m_gfx_adr_l));
	save_item(NAME(m_gfx_adr_m));
	save_item(NAME(m_gfx_adr_h));
	save_item(NAME(m_gfx_adr));
	save_item(NAME(m_gfxdata0));
	save_item(NAME(m_gfxdata1));
	save_item(NAME(m_jansou_colortable));
}

void royalmah_prgbank_state::init_dynax()
{
	m_mainbank->configure_entries(0, 32, memregion("maincpu")->base() + 0x10000, 0x8000);
}

void royalmah_prgbank_state::init_suzume()
{
	m_mainbank->configure_entries(0, 8, memregion("maincpu")->base() + 0x10000, 0x8000);

	save_item(NAME(m_suzume_bank));
}

void royalmah_prgbank_state::init_daisyari()
{
	m_mainbank->configure_entries(0, 8, memregion("maincpu")->base() + 0x10000, 0x8000);
}

void royalmah_prgbank_state::init_mjtensin()
{
	m_mainbank->configure_entries(0, 80, memregion("maincpu")->base() + 0x10000, 0x8000);

	save_item(NAME(m_rombank));
}

void royalmah_prgbank_state::init_cafetime()
{
	m_mainbank->configure_entries(0, 64, memregion("maincpu")->base() + 0x10000, 0x8000);

	save_item(NAME(m_rombank));
}

void royalmah_prgbank_state::init_cafedoll()
{
	init_cafetime();

	save_item(NAME(m_mjvegas_p5_val));
}

void royalmah_prgbank_state::init_cafepara()
{
	m_mainbank->configure_entries(0, 80, memregion("maincpu")->base() + 0x10000, 0x8000);

	save_item(NAME(m_rombank));

	m_janptr96_nvram = std::make_unique<uint8_t[]>(0x1000 * 9);
	membank("bank3")->set_base(m_janptr96_nvram.get());
	subdevice<nvram_device>("nvram")->set_base(m_janptr96_nvram.get(), 0x1000 * 9);
	m_rambank->configure_entries(0, 8, m_janptr96_nvram.get() + 0x1000, 0x1000);
}

void royalmah_prgbank_state::init_mjvegasa()
{
	m_mainbank->configure_entries(0, 128, memregion("maincpu")->base() + 0x10000, 0x8000);

	save_item(NAME(m_rombank));
}

void royalmah_prgbank_state::init_mjvegas()
{
	init_mjvegasa();

	save_item(NAME(m_mjvegas_p5_val));
}

void royalmah_prgbank_state::init_jongshin()
{
	m_mainbank->configure_entries(0, 3, memregion("maincpu")->base() + 0x10000, 0x8000);
}

void royalmah_prgbank_state::init_mjifb()
{
	m_mainbank->configure_entries(0, 256, memregion("maincpu")->base() + 0x10000, 0x4000);

	save_item(NAME(m_rombank));
}

void royalmah_prgbank_state::init_tontonb()
{
	m_mainbank->configure_entries(0, 16, memregion("maincpu")->base() + 0x10000, 0x8000);
}

void royalmah_prgbank_state::init_janptr96()
{
	m_mainbank->configure_entries(0, 64, memregion("maincpu")->base() + 0x10000, 0x8000);

	m_janptr96_nvram = std::make_unique<uint8_t[]>(0x1000 * 9);
	membank("bank3")->set_base(m_janptr96_nvram.get());
	subdevice<nvram_device>("nvram")->set_base(m_janptr96_nvram.get(), 0x1000 * 9);
	m_rambank->configure_entries(0, 8, m_janptr96_nvram.get() + 0x1000, 0x1000);
}

void royalmah_prgbank_state::init_mjsenka()
{
	uint8_t *rom = memregion("maincpu")->base();
	uint8_t *decrypted = memregion("decrypted")->base();

	for (int i = 0x0000; i < 0xa000; i++)
		decrypted[i] = rom[i];

	for (int i = 0x6000; i < 0x8000; i++)
	{
		switch (i & 0x15)
		{
			case 0x00: decrypted[i] ^= 0x03; break;
			case 0x01: decrypted[i] ^= 0x07; break;
			case 0x04: decrypted[i] ^= 0x01; break;
			case 0x05: decrypted[i] ^= 0x05; break;
			case 0x10: decrypted[i] ^= 0x02; break;
			case 0x11: decrypted[i] ^= 0x06; break;
			case 0x14: decrypted[i] ^= 0x00; break;
			case 0x15: decrypted[i] ^= 0x04; break;
		}

		if (i & 0x02)
			decrypted[i] ^= 0x80;

		decrypted[i] = bitswap<8>(decrypted[i], 2, 6, 5, 4, 3, 0, 7, 1);
	}

	for (int i = 0x6000; i < 0x8000; i++)
	{
		switch (i & 0x0e)
		{
			case 0x00: rom[i] ^= 0x01; break;
			case 0x02: rom[i] ^= 0x03; break;
			case 0x04: rom[i] ^= 0x05; break;
			case 0x06: rom[i] ^= 0x07; break;
			case 0x08: rom[i] ^= 0x00; break;
			case 0x0a: rom[i] ^= 0x02; break;
			case 0x0c: rom[i] ^= 0x04; break;
			case 0x0e: rom[i] ^= 0x06; break;
		}

		if ((i & 0x01) == 0x00)
			rom[i] ^= 0x80;

		rom[i] = bitswap<8>(rom[i], 0, 6, 5, 4, 3, 1, 2, 7);
	}

	m_mainopbank->configure_entries(0, 8, &decrypted[0x8000], 0x400);

	init_chalgirl();
}

void royalmah_prgbank_state::init_mjsiyoub()
{
	m_mainbank->configure_entries(0, 32, memregion("maincpu")->base() + 0x8000, 0x400);

	save_item(NAME(m_mjyarou_bank));
}

void royalmah_prgbank_state::init_chalgirl()
{
	m_mainbank->configure_entries(0, 8, memregion("maincpu")->base() + 0x8000, 0x400);

	save_item(NAME(m_mjyarou_bank));
}

void royalmah_prgbank_state::init_ichiban()
{
	// TODO: work out banking
	m_mainbank->configure_entries(0, 8, memregion("maincpu")->base() + 0x10000, 0x8000);
}

void royalmah_prgbank_state::init_pongboo2()
{
	// TODO: work out banking
	m_mainbank->configure_entry(0, memregion("maincpu")->base() + 0x8000);
}

} // anonymous namespace


// the original Janputer (Sanritsu) is not yet dumped, basically Royal Mahjong but non-BET type
GAME( 1981,  royalmj,  0,        royalmah, royalmah, royalmah_state,         empty_init,    ROT0,   "Nichibutsu",                 "Royal Mahjong (Japan, v1.13)",          0 )
GAME( 1981?, openmj,   royalmj,  royalmah, royalmah, royalmah_state,         empty_init,    ROT0,   "Sapporo Mechanic",           "Open Mahjong (Japan)",                  0 )
GAME( 1982,  royalmah, royalmj,  royalmah, royalmah, royalmah_state,         empty_init,    ROT0,   "bootleg",                    "Royal Mahjong (Falcon bootleg, v1.01)", 0 )
GAME( 1984?, chalgirl, 0,        chalgirl, royalmah, royalmah_prgbank_state, init_chalgirl, ROT0,   "bootleg",                    "Challenge Girl (Falcon bootleg)", MACHINE_WRONG_COLORS | MACHINE_NOT_WORKING ) // verify ROM loading / banking, bad girl colors
GAME( 1983,  seljan,   0,        seljan,   seljan,   royalmah_state,         empty_init,    ROT0,   "Jem / Dyna Corp",            "Sel-Jan (Japan)",                       0 )
GAME( 1983,  janyoup2, royalmj,  janyoup2, janyoup2, royalmah_state,         empty_init,    ROT0,   "Cosmo Denshi",               "Janyou Part II (ver 7.03, July 1 1983)",0 )
GAME( 1985,  tahjong,  royalmj,  tahjong,  tahjong,  royalmah_prgbank_state, init_tahjong,  ROT0,   "Bally Pond / Nasco",         "Tahjong Yakitori (ver. 2-1)",           0 ) // 1985 Jun. 17
GAME( 1981,  janputer, 0,        royalmah, royalmah, royalmah_state,         empty_init,    ROT0,   "bootleg (Paradise Denshi Ltd. / Mes)", "New Double Bet Mahjong (bootleg of Royal Mahjong)", 0 ) // MT #05392
GAME( 1981,  akamj,    0,        ippatsu,  ippatsu,  royalmah_state,         empty_init,    ROT0,   "bootleg (Paradise Denshi Ltd.)", "Aka Mahjong (Double Bet)",          0 )
GAME( 1984,  rkjanoh2, 0,        rkjanoh2, royalmah, royalmah_prgbank_state, init_chalgirl, ROT0,   "SNK / Dyna Corp",            "Royal King Jang Oh 2 (v4.00 1984 Jun 10th)", MACHINE_WRONG_COLORS | MACHINE_NOT_WORKING ) // never seems to set the palette bank?
GAME( 1984,  janoh,    0,        janoh,    royalmah, royalmah_state,         empty_init,    ROT0,   "Toaplan",                    "Jan Oh (set 1)",                        MACHINE_NOT_WORKING )
GAME( 1984,  janoha,   janoh,    janoha,   royalmah, royalmah_state,         empty_init,    ROT0,   "Toaplan",                    "Jan Oh (set 2)",                        MACHINE_NOT_WORKING ) // this one is complete?
GAME( 1985,  jansou,   0,        jansou,   jansou,   royalmah_state,         init_jansou,   ROT0,   "Dyna Computer",              "Jansou (set 1)",                        MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1985,  jansoua,  jansou,   jansou,   jansou,   royalmah_state,         init_jansou,   ROT0,   "Dyna Computer",              "Jansou (V 1.1)",                        0 )
GAME( 1986,  jangtaku, 0,        jansou,   jansou,   royalmah_state,         init_jansou,   ROT0,   "Dyna Computer",              "Jang Taku (V 1.3)",                     0 )
GAME( 1986,  dondenmj, 0,        dondenmj, majs101b, royalmah_prgbank_state, init_dynax,    ROT0,   "Dyna Electronics",           "Don Den Mahjong (Japan)",               0 )
GAME( 1986,  ippatsu,  0,        ippatsu,  ippatsu,  royalmah_state,         empty_init,    ROT0,   "Public Software / Paradais", "Ippatsu Gyakuten (Japan)",              0 )
GAME( 1986,  suzume,   0,        suzume,   suzume,   royalmah_prgbank_state, init_suzume,   ROT0,   "Dyna Electronics",           "Watashiha Suzumechan (Japan)",          0 )
GAME( 1986,  jongshin, 0,        jongshin, jongshin, royalmah_prgbank_state, init_jongshin, ROT0,   "Dyna Electronics",           "Jong Shin (Japan)",                     0 )
GAME( 1986,  mjsiyoub, 0,        mjsiyoub, mjyarou,  royalmah_prgbank_state, init_mjsiyoub, ROT0,   "Visco",                      "Mahjong Shiyou (Japan)",                MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_COLORS | MACHINE_NOT_WORKING ) // MSM5205 isn't hooked up, colors need to be verified against original
GAME( 1986,  mjsenka,  0,        mjsenka,  mjyarou,  royalmah_prgbank_state, init_mjsenka,  ROT0,   "Visco",                      "Mahjong Senka (Japan)",                 MACHINE_WRONG_COLORS | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING ) // never seems to set the palette bank?
GAME( 1986,  mjyarou,  0,        mjyarou,  mjyarou,  royalmah_prgbank_state, init_chalgirl, ROT0,   "Visco / Video System",       "Mahjong Yarou (Japan, set 1)",          MACHINE_WRONG_COLORS | MACHINE_IMPERFECT_GRAPHICS ) // never seems to set the palette bank?
GAME( 1986,  mjyarou2, mjyarou,  mjyarou,  mjyarou,  royalmah_prgbank_state, init_chalgirl, ROT0,   "Visco / Video System",       "Mahjong Yarou (Japan, set 2)",          MACHINE_WRONG_COLORS | MACHINE_IMPERFECT_GRAPHICS ) // never seems to set the palette bank?
GAME( 1986?, mjclub,   0,        mjclub,   mjclub,   royalmah_prgbank_state, init_tontonb,  ROT0,   "Xex",                        "Mahjong Club (Japan)",                  0 )
GAME( 1987,  mjdiplob, 0,        mjdiplob, mjdiplob, royalmah_prgbank_state, init_tontonb,  ROT0,   "Dynax",                      "Mahjong Diplomat (Japan)",              0 )
GAME( 1987,  tontonb,  0,        tontonb,  tontonb,  royalmah_prgbank_state, init_tontonb,  ROT0,   "Dynax",                      "Tonton (Japan)",                        0 )
GAME( 1987,  makaijan, 0,        makaijan, makaijan, royalmah_prgbank_state, init_dynax,    ROT0,   "Dynax",                      "Makaijan (Japan)",                      0 )
GAME( 1988,  majs101b, 0,        majs101b, majs101b, royalmah_prgbank_state, init_dynax,    ROT0,   "Dynax",                      "Mahjong Studio 101 (Japan)",            0 )
GAME( 1988,  mjapinky, 0,        mjapinky, mjapinky, royalmah_prgbank_state, init_tontonb,  ROT0,   "Dynax",                      "Almond Pinky (Japan)",                  0 )
GAME( 1989,  mjdejavu, 0,        mjdejavu, mjdejavu, royalmah_prgbank_state, init_mjifb,    ROT0,   "Dynax",                      "Mahjong Shinkirou Deja Vu (Japan)",     MACHINE_NOT_WORKING ) // MT #00964
GAME( 1989,  mjdejav2, mjdejavu, mjdejavu, mjdejavu, royalmah_prgbank_state, init_mjifb,    ROT0,   "Dynax",                      "Mahjong Shinkirou Deja Vu 2 (Japan)",   MACHINE_NOT_WORKING )
GAME( 1989,  mjderngr, 0,        mjderngr, mjderngr, royalmah_prgbank_state, init_dynax,    ROT0,   "Dynax",                      "Mahjong Derringer (Japan)",             0 )
GAME( 1989,  daisyari, 0,        daisyari, daisyari, royalmah_prgbank_state, init_daisyari, ROT0,   "Best System",                "Daisyarin (Japan)",                     0 )
GAME( 1990,  mjifb,    0,        mjifb,    mjifb,    royalmah_prgbank_state, init_mjifb,    ROT0,   "Dynax",                      "Mahjong If...?",                        0 )
GAME( 1990,  mjifb2,   mjifb,    mjifb,    mjifb,    royalmah_prgbank_state, init_mjifb,    ROT0,   "Dynax",                      "Mahjong If...? (2921)",                 0 )
GAME( 1990,  mjifb3,   mjifb,    mjifb,    mjifb,    royalmah_prgbank_state, init_mjifb,    ROT0,   "Dynax",                      "Mahjong If...? (2931)",                 0 )
GAME( 1991,  mjvegasa, 0,        mjvegasa, mjvegasa, royalmah_prgbank_state, init_mjvegasa, ROT0,   "Dynax",                      "Mahjong Vegas (Japan, unprotected)",    0 )
GAME( 1991,  mjvegas,  mjvegasa, mjvegas,  mjvegasa, royalmah_prgbank_state, init_mjvegas,  ROT0,   "Dynax",                      "Mahjong Vegas (Japan)",                 0 )
GAME( 1992,  cafetime, 0,        cafetime, cafetime, royalmah_prgbank_state, init_cafetime, ROT0,   "Dynax",                      "Mahjong Cafe Time",                     0 )
GAME( 1993,  cafedoll, 0,        cafedoll, cafetime, royalmah_prgbank_state, init_cafedoll, ROT0,   "Dynax",                      "Mahjong Cafe Doll (Japan, Ver. 1.00)",  MACHINE_NOT_WORKING ) // fails protection check (at 0x178 it puts 0x55 in 0xFFBF instead of 0x56 like the code expects and chaos ensues)
GAME( 1993,  cafedollg,cafedoll, cafedoll, cafetime, royalmah_prgbank_state, init_cafedoll, ROT0,   "Dynax",                      "Mahjong Cafe Doll Great (Japan, Ver. 1.00)", MACHINE_NOT_WORKING ) // fails protection check (at 0x178 it puts 0x55 in 0xFFBF instead of 0x56 like the code expects and chaos ensues)
GAME( 1993,  ichiban,  0,        ichiban,  ichiban,  royalmah_prgbank_state, init_ichiban,  ROT0,   "Excel",                      "Ichi Ban Jyan",                         MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS ) // ROM banking is wrong, causing several GFX problems
GAME( 1995,  mjtensin, 0,        mjtensin, mjtensin, royalmah_prgbank_state, init_mjtensin, ROT0,   "Dynax",                      "Mahjong Tensinhai (Japan)",             MACHINE_NOT_WORKING )
GAME( 1996,  majrjhdx, 0,        majrjh,   mjtensin, royalmah_prgbank_state, init_mjtensin, ROT0,   "Dynax",                      "Mahjong Raijinhai DX (Ver. D105)",      0 )
GAME( 1996,  majrjh,   majrjhdx, majrjh,   mjtensin, royalmah_prgbank_state, init_mjtensin, ROT0,   "Dynax",                      "Mahjong Raijinhai (Ver. D105)",         0 )
GAME( 1996,  janptr96, 0,        janptr96, janptr96, royalmah_prgbank_state, init_janptr96, ROT0,   "Dynax",                      "Janputer '96 (Japan)",                  0 )
GAME( 1997,  janptrsp, 0,        janptr96, janptr96, royalmah_prgbank_state, init_janptr96, ROT0,   "Dynax",                      "Janputer Special (Japan)",              0 )
GAME( 1997,  pongboo2, 0,        pongboo2, ichiban,  royalmah_prgbank_state, init_pongboo2, ROT0,   "OCT",                        "Pong Boo! 2 (Ver. 1.31)",               MACHINE_NOT_WORKING | MACHINE_WRONG_COLORS ) // banking, palette, inputs
GAME( 1999,  cafebrk,  0,        cafepara, cafetime, royalmah_prgbank_state, init_cafepara, ROT0,   "Nakanihon / Dynax",          "Mahjong Cafe Break (Ver. 1.01J)",       MACHINE_NOT_WORKING ) // needs correct banking and / or 1d ROM descrambling
GAME( 1999,  cafepara, 0,        cafepara, cafetime, royalmah_prgbank_state, init_cafepara, ROT0,   "Techno-Top",                 "Mahjong Cafe Paradise (Ver. 1.00)",     MACHINE_NOT_WORKING ) // needs correct banking and / or 1d ROM descrambling
