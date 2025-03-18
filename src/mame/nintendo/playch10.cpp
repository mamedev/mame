// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi,Brad Oliver
/***************************************************************************

PlayChoice-10 - (c) 1986 Nintendo of America

    Written by Ernesto Corvi.

    Portions of this code are heavily based on
    Brad Oliver's MESS implementation of the NES.

    Thanks to people that contributed to this driver, namely:
    - Brad Oliver.
    - Aaron Giles.

****************************************************************************

BIOS:
    Memory Map
    ----------
    0000 - 3fff = Program ROM (8T)
    8000 - 87ff = RAM (8V)
    8800 - 8fff = RAM (8W)
    9000 - 97ff = SRAM (8R - Videoram)
    Cxxx = /INST ROM SEL
    Exxx = /IDSEL

    Input Ports -----------
    Read:
    - Port 0
    bit0 = CHSelect(?)
    bit1 = Enter button
    bit2 = Reset button
    bit3 = INTDETECT
    bit4 = N/C
    bit5 = Coin 2
    bit6 = Service button
    bit7 = Coin 1
    - Port 1 = Dipswitch 1
    - Port 2 = Dipswitch 2
    - Port 3 = /DETECTCLR

    Write: (always bit 0)
    - Port 0 = SDCS (ShareD CS)
    - Port 1 = /CNTRLMASK
    - Port 2 = /DISPMASK
    - Port 3 = /SOUNDMASK
    - Port 4 = /GAMERES
    - Port 5 = /GAMESTOP
    - Port 6 = N/C
    - Port 7 = N/
    - Port 8 = NMI Enable
    - Port 9 = DOG DI
    - Port A = /PPURES
    - Port B = CSEL0 \
    - Port C = CSEL1  \ (Cartridge select: 0 to 9)
    - Port D = CSEL2  /
    - Port E = CSEL3 /
    - Port F = 8UP KEY

****************************************************************************

Working games:
--------------
    - 1942                              (NF) - Standard board
    - Balloon Fight                     (BF) - Standard board
    - Baseball                          (BA) - Standard board
    - Baseball Stars                    (B9) - F board
    - Captain Sky Hawk                  (YW) - i board
    - Castlevania                       (CV) - B board
    - Contra                            (CT) - B board
    - Double Dragon                     (WD) - F board
    - Double Dribble                    (DW) - B board
    - Dr. Mario                         (VU) - F board
    - Duck Hunt                         (DH) - Standard board
    - Excitebike                        (EB) - Standard board
    - Fester's Quest                    (EQ) - F board
    - Gauntlet                          (GL) - G board
    - Golf                              (GF) - Standard board
    - Gradius                           (GR) - A board
    - Hogan's Alley                     (HA) - Standard board
    - Kung Fu                           (SX) - Standard board
    - Mario Bros.                       (MA) - Standard board
    - Mario Open Golf                   (UG) - K board
    - Mega Man 3                        (XU) - G board
    - Metroid                           (MT) - D board
    - Mike Tyson's Punchout             (PT) - E board
    - Ninja Gaiden                      (NG) - F board
    - Ninja Gaiden 2                    (NW) - G board
    - Ninja Gaiden 3                    (3N) - G board
    - Nintendo World Cup                (XZ) - G board
    - Pinbot                            (IO) - H board
    - Power Blade                       (7T) - G board
    - Pro Wrestling                     (PW) - B board
    - Rad Racer                         (RC) - D board
    - Rad Racer II                      (QR) - G board
    - RC Pro Am                         (PM) - F board
    - Rescue Rangers                    (RU) - F board
    - Rockin' Kats                      (7A) - G board
    - Rush N' Attack                    (RA) - B board
    - Rygar                             (RY) - B board
    - Solar Jetman                      (LJ) - i board
    - Super C                           (UE) - G board
    - Super Mario Bros                  (SM) - Standard board
    - Super Mario Bros 2                (MW) - G board
    - Super Mario Bros 3                (UM) - G board
    - Tecmo Bowl                        (TW) - F board
    - Teenage Mutant Ninja Turtles      (U2) - F board
    - Teenage Mutant Ninja Turtles 2    (2N) - G board
    - Tennis                            (TE) - Standard board
    - Track & Field                     (TR) - A board
    - Trojan                            (TJ) - B board
    - The Goonies                       (GN) - C board
    - Volley Ball                       (VB) - Standard board
    - Wild Gunman                       (WG) - Standard board
    - Yo Noid                           (YC) - F board

Non working games due to missing roms:
--------------------------------------
    - ShatterHand                       (??) - ? board (likely doesn't exist)

****************************************************************************

Dipswitches information:
------------------------
Steph 2000.09.07

The 6 first DSWA (A-F) are used for coinage (units of time given for coin A/coin B)
When bit 6 of DSWB (O) is ON, units of time given for coin B are divided by 2

The 6 first DSWB (I-N) are used to set timer speed :
    [0x80d5] = ( ( (IN A,02) | 0xc0 ) + 0x3c ) & 0xff

When bit 7 of DSWB (P) is ON, you're in 'Freeplay' mode with 9999 units of time ...
However, this is effective ONLY if 7 other DSWB (I-O) are OFF !

I add the 32 combinations for coinage.

As I don't know what is the default value for timer speed, and I don't want to write
the 64 combinations, I only put some values ... Feel free to add the other ones ...

 DSW A    DSW B
HGFEDCBA PONMLKJI    coin A  coin B

xx000000 x0xxxxxx      300       0
xx000001 x0xxxxxx      300     100
xx000010 x0xxxxxx      300     200
xx000011 x0xxxxxx      300     300
xx000100 x0xxxxxx      300     400
xx000101 x0xxxxxx      300     500
xx000110 x0xxxxxx      300     600
xx000111 x0xxxxxx      300     700
xx001000 x0xxxxxx      300     800
xx001001 x0xxxxxx      300     900
xx001010 x0xxxxxx      150       0
xx001011 x0xxxxxx      150     200
xx001100 x0xxxxxx      150     400
xx001101 x0xxxxxx      150     600
xx001110 x0xxxxxx      150     800
xx001111 x0xxxxxx      150     500
xx010000 x0xxxxxx      300    1000
xx010001 x0xxxxxx      300    1100
xx010010 x0xxxxxx      300    1200
xx010011 x0xxxxxx      300    1300
xx010100 x0xxxxxx      300    1400
xx010101 x0xxxxxx      300    1500
xx010110 x0xxxxxx      300    1600
xx010111 x0xxxxxx      300    1700
xx011000 x0xxxxxx      300    1800
xx011001 x0xxxxxx      300    1900
xx011010 x0xxxxxx      150    1000
xx011011 x0xxxxxx      150    1200
xx011100 x0xxxxxx      150    1400
xx011101 x0xxxxxx      150    1600
xx011110 x0xxxxxx      150    1800
xx011111 x0xxxxxx      150    1500
xx100000 x0xxxxxx      300    2000
xx100001 x0xxxxxx      300    2100
xx100010 x0xxxxxx      300    2200
xx100011 x0xxxxxx      300    2300
xx100100 x0xxxxxx      300    2400
xx100101 x0xxxxxx      300    2500
xx100110 x0xxxxxx      300    2600
xx100111 x0xxxxxx      300    2700
xx101000 x0xxxxxx      300    2800
xx101001 x0xxxxxx      300    2900
xx101010 x0xxxxxx      150    2000
xx101011 x0xxxxxx      150    2200
xx101100 x0xxxxxx      150    2400
xx101101 x0xxxxxx      150    2600
xx101110 x0xxxxxx      150    2800
xx101111 x0xxxxxx      150    2500
xx110000 x0xxxxxx      300    3000
xx110001 x0xxxxxx      300    3100
xx110010 x0xxxxxx      300    3200
xx110011 x0xxxxxx      300    3300
xx110100 x0xxxxxx      300    3400
xx110101 x0xxxxxx      300    3500
xx110110 x0xxxxxx      300    3600
xx110111 x0xxxxxx      300    3700
xx111000 x0xxxxxx      300    3800
xx111001 x0xxxxxx      300    3900
xx111010 x0xxxxxx      150    3000
xx111011 x0xxxxxx      150    3200
xx111100 x0xxxxxx      150    3400
xx111101 x0xxxxxx      150    3600
xx111110 x0xxxxxx      150    3800
xx111111 x0xxxxxx      150    3500

xx000000 x1xxxxxx      300       0
xx000001 x1xxxxxx      300      50
xx000010 x1xxxxxx      300     100
xx000011 x1xxxxxx      300     150
xx000100 x1xxxxxx      300     200
xx000101 x1xxxxxx      300     250
xx000110 x1xxxxxx      300     300
xx000111 x1xxxxxx      300     350
xx001000 x1xxxxxx      300     400
xx001001 x1xxxxxx      300     450
xx001010 x1xxxxxx      150       0
xx001011 x1xxxxxx      150     100
xx001100 x1xxxxxx      150     200
xx001101 x1xxxxxx      150     300
xx001110 x1xxxxxx      150     400
xx001111 x1xxxxxx      150     250
xx010000 x1xxxxxx      300     500
xx010001 x1xxxxxx      300     550
xx010010 x1xxxxxx      300     600
xx010011 x1xxxxxx      300     650
xx010100 x1xxxxxx      300     700
xx010101 x1xxxxxx      300     750
xx010110 x1xxxxxx      300     800
xx010111 x1xxxxxx      300     850
xx011000 x1xxxxxx      300     900
xx011001 x1xxxxxx      300     950
xx011010 x1xxxxxx      150     500
xx011011 x1xxxxxx      150     600
xx011100 x1xxxxxx      150     700
xx011101 x1xxxxxx      150     800
xx011110 x1xxxxxx      150     750
xx100000 x1xxxxxx      300    1000
xx100001 x1xxxxxx      300    1050
xx100010 x1xxxxxx      300    1100
xx100011 x1xxxxxx      300    1150
xx100100 x1xxxxxx      300    1200
xx100101 x1xxxxxx      300    1250
xx100110 x1xxxxxx      300    1300
xx100111 x1xxxxxx      300    1350
xx101000 x1xxxxxx      300    1400
xx101001 x1xxxxxx      300    1450
xx101010 x1xxxxxx      150    1000
xx101011 x1xxxxxx      150    1100
xx101100 x1xxxxxx      150    1200
xx101101 x1xxxxxx      150    1300
xx101110 x1xxxxxx      150    1400
xx101111 x1xxxxxx      150    1250
xx110000 x1xxxxxx      300    1500
xx110001 x1xxxxxx      300    1550
xx110010 x1xxxxxx      300    1600
xx110011 x1xxxxxx      300    1650
xx110100 x1xxxxxx      300    1700
xx110101 x1xxxxxx      300    1750
xx110110 x1xxxxxx      300    1800
xx110111 x1xxxxxx      300    1850
xx111000 x1xxxxxx      300    1900
xx111001 x1xxxxxx      300    1950
xx111010 x1xxxxxx      150    1500
xx111011 x1xxxxxx      150    1600
xx111100 x1xxxxxx      150    1700
xx111101 x1xxxxxx      150    1800
xx111110 x1xxxxxx      150    1750

I know that the way I code the DSW isn't correct, but I don't know how to link
O to A-F AND, at the same time, O to P ... Any help is appreciated ...

****************************************************************************

Notes & Todo:
-------------

- Look at Ninja Gaiden 3. It has some slight timing issues on the
  second level. Probably related to the mapper's irq timing.
- Fix some remaining bad gfx in Rad Racer II.
- Implement Dipswitches properly once Mame can support it.
- Better control layout?. This thing has odd buttons.
- Find dumps of the rest of the RP5H01's and add the remaining games.
- Any PPU optimizations that retain accuracy are certainly welcome.

***************************************************************************/

#include "emu.h"

#include "bus/nes_ctrl/zapper_sensor.h"
#include "cpu/m6502/rp2a03.h"
#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "machine/rp5h01.h"
#include "machine/nvram.h"
#include "video/ppu2c0x.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include "playch10.lh"


namespace {

class playch10_state : public driver_device
{
public:
	playch10_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cartcpu(*this, "cart")
		, m_ppu(*this, "ppu")
		, m_rp5h01(*this, "rp5h01")
		, m_ram_8w(*this, "ram_8w")
		, m_videoram(*this, "videoram")
		, m_gfxdecode(*this, "gfxdecode")
		, m_sensor(*this, "sensor")
		, m_in(*this, "P%u", 1U)
		, m_gunx(*this, "GUNX")
		, m_guny(*this, "GUNY")
		, m_trigger(*this, "TRIGGER")
		, m_nt_page(*this, "nt_page%u", 0U)
		, m_prg_banks(*this, "prg%u", 0U)
		, m_prg_view(*this, "prg_view")
		, m_vrom_region(*this, "gfx2")
		, m_timedigits(*this, "digit_%u", 0U)
	{
	}

	void playch10(machine_config &config);
	void playch10_a(machine_config &config);
	void playch10_b(machine_config &config);
	void playch10_c(machine_config &config);
	void playch10_d(machine_config &config);
	void playch10_d2(machine_config &config);
	void playch10_e(machine_config &config);
	void playch10_f(machine_config &config);
	void playch10_f2(machine_config &config);
	void playch10_g(machine_config &config);
	void playch10_h(machine_config &config);
	void playch10_i(machine_config &config);
	void playch10_k(machine_config &config);

	void init_playch10();
	void init_pc_gun();
	void init_pcaboard();
	void init_pcbboard();
	void init_pccboard();
	void init_pcdboard();
	void init_pceboard();
	void init_pcfboard();
	void init_pcgboard();
	void init_pcgboard_type2();
	void init_pchboard();
	void init_pciboard();
	void init_pckboard();
	void init_pc_hrz();

	int int_detect_r();

private:
	void up8w_w(int state);
	u8 ram_8w_r(offs_t offset);
	void ram_8w_w(offs_t offset, u8 data);
	void time_w(offs_t offset, u8 data);
	void sdcs_w(int state);
	void cntrl_mask_w(int state);
	void disp_mask_w(int state);
	void sound_mask_w(int state);
	void nmi_enable_w(int state);
	void dog_di_w(int state);
	void ppu_reset_w(int state);
	u8 pc10_detectclr_r();
	void cart_sel_w(u8 data);
	u8 pc10_prot_r();
	void pc10_prot_w(u8 data);
	void pc10_in0_w(u8 data);
	u8 pc10_in0_r();
	u8 pc10_in1_r();
	void pc10_chr_w(offs_t offset, u8 data);
	u8 pc10_chr_r(offs_t offset);
	void mmc1_rom_switch_w(offs_t offset, u8 data);
	void aboard_vrom_switch_w(u8 data);
	void bboard_rom_switch_w(u8 data);
	void cboard_vrom_switch_w(u8 data);
	void eboard_rom_switch_w(offs_t offset, u8 data);
	void gboard_rom_switch_w(offs_t offset, u8 data);
	void hboard_rom_switch_w(offs_t offset, u8 data);
	void iboard_rom_switch_w(u8 data);
	void playch10_videoram_w(offs_t offset, u8 data);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	void bios_io_map(address_map &map) ATTR_COLD;
	void bios_map(address_map &map) ATTR_COLD;
	void ppu_map(address_map &map) ATTR_COLD;
	void cart_map(address_map &map) ATTR_COLD;
	void cart_a_map(address_map &map) ATTR_COLD;
	void cart_b_map(address_map &map) ATTR_COLD;
	void cart_c_map(address_map &map) ATTR_COLD;
	void cart_d_map(address_map &map) ATTR_COLD;
	void cart_d2_map(address_map &map) ATTR_COLD;
	void cart_e_map(address_map &map) ATTR_COLD;
	void cart_f_map(address_map &map) ATTR_COLD;
	void cart_f2_map(address_map &map) ATTR_COLD;
	void cart_g_map(address_map &map) ATTR_COLD;
	void cart_h_map(address_map &map) ATTR_COLD;
	void cart_i_map(address_map &map) ATTR_COLD;
	void cart_k_map(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	struct chr_bank
	{
		int writable = 0;   // 1 for RAM, 0 for ROM
		u8* chr = nullptr;  // direct access to the memory
	};

	void playch10_palette(palette_device &palette) const;
	void vblank_irq(int state);

	void pc10_set_videorom_bank(int first, int count, int bank, int size);
	void pc10_set_videoram_bank(int first, int count, int bank, int size);
	void gboard_scanline_cb(int scanline, bool vblank, bool blanked);
	void int_detect_w(int state);
	void mapper9_latch(offs_t offset);
	void pc10_set_mirroring(int mirroring);

	u32 screen_update_playch10_top(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	u32 screen_update_playch10_bottom(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	u32 screen_update_playch10_single(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<cpu_device> m_maincpu;
	required_device<rp2a03_device> m_cartcpu;
	required_device<ppu2c0x_device> m_ppu;
	optional_device<rp5h01_device> m_rp5h01;

	required_shared_ptr<u8> m_ram_8w;
	required_shared_ptr<u8> m_videoram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<nes_zapper_sensor_device> m_sensor;

	required_ioport_array<2> m_in;
	optional_ioport m_gunx;
	optional_ioport m_guny;
	optional_ioport m_trigger;

	required_memory_bank_array<4> m_nt_page;
	std::unique_ptr<u8[]> m_nt_ram;
	std::unique_ptr<u8[]> m_cart_nt_ram;

	void init_prg_banking();
	void prg32(int bank);
	void prg16(int slot, int bank);
	void prg8(int slot, int bank);
	memory_bank_array_creator<4> m_prg_banks;
	memory_view m_prg_view;
	int m_prg_chunks = 0;

	optional_memory_region m_vrom_region;

	output_finder<4> m_timedigits;

	int m_up_8w = 0;
	int m_pc10_nmi_enable = 0;
	int m_pc10_dog_di = 0;
	int m_pc10_sdcs = 0;
	int m_pc10_dispmask = 0;
	int m_pc10_int_detect = 0;
	int m_pc10_game_mode = 0;
	int m_pc10_dispmask_old = 0;
	int m_pc10_gun_controller = 0;
	int m_cart_sel = 0;
	int m_cntrl_mask = 0;
	int m_input_latch[2]{};
	int m_mirroring = 0;
	int m_MMC2_bank[4]{};
	int m_MMC2_bank_latch[2]{};
	u8* m_vrom = nullptr;
	std::unique_ptr<u8[]> m_vram;
	chr_bank m_chr_page[8];
	u8 m_mmc1_shiftreg = 0;
	u8 m_mmc1_shiftcount = 0;
	u8 m_mmc1_prg16k = 0;
	u8 m_mmc1_chr4k = 0;
	u8 m_mmc1_switchlow = 0;
	int m_gboard_banks[2]{};
	int m_gboard_command = 0;
	int m_IRQ_count = 0;
	u8 m_IRQ_count_latch = 0;
	int m_IRQ_enable = 0;
	int m_pc10_bios = 0;
	tilemap_t *m_bg_tilemap = nullptr;
};


/***************************************************************************

  PlayChoice-10 Video Hardware

***************************************************************************/

void playch10_state::playch10_videoram_w(offs_t offset, u8 data)
{
	if (m_pc10_sdcs)
	{
		m_videoram[offset] = data;
		m_bg_tilemap->mark_tile_dirty(offset / 2);
	}
}

void playch10_state::playch10_palette(palette_device &palette) const
{
	static constexpr u8 coeff[4] = { 0x0e, 0x1f, 0x43, 0x8f };
	const u8 *color_prom = memregion("proms")->base();

	for (int i = 0; i < 256; i++)
	{
		for (int j = 0, r = 0, g = 0, b = 0; j < 4; j++)
		{
			r += coeff[j] * BIT(color_prom[0], j);
			g += coeff[j] * BIT(color_prom[256], j);
			b += coeff[j] * BIT(color_prom[2 * 256], j);
			palette.set_pen_color(i, rgb_t(r, g, b));
		}

		color_prom++;
	}
}

void playch10_state::int_detect_w(int state)
{
	if (state)
		m_pc10_int_detect = 1;
}

TILE_GET_INFO_MEMBER(playch10_state::get_bg_tile_info)
{
	u8 *videoram = m_videoram;
	int offs = tile_index * 2;
	int code = videoram[offs] + ((videoram[offs + 1] & 0x07) << 8);
	int color = (videoram[offs + 1] >> 3) & 0x1f;

	tileinfo.set(0, code, color, 0);
}

void playch10_state::video_start()
{
	const u8 *bios = memregion("maincpu")->base();
	m_pc10_bios = (bios[3] == 0x2a) ? 1 : 2;

	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(playch10_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS,
			8, 8, 32, 32);
}

/***************************************************************************

  Display refresh

***************************************************************************/

u32 playch10_state::screen_update_playch10_single(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	rectangle top_monitor = screen.visible_area();

	top_monitor.max_y = (top_monitor.max_y - top_monitor.min_y) / 2;

	if (m_pc10_dispmask_old != m_pc10_dispmask)
	{
		m_pc10_dispmask_old = m_pc10_dispmask;

		if (m_pc10_dispmask)
			m_pc10_game_mode ^= 1;
	}

	if (m_pc10_game_mode)
		// render the ppu
		m_ppu->render(bitmap, 0, 0, 0, 0, cliprect);
	else
	{
		// When the bios is accessing vram, the video circuitry can't access it
		if (!m_pc10_sdcs)
			m_bg_tilemap->draw(screen, bitmap, top_monitor, 0, 0);
	}

	return 0;
}

u32 playch10_state::screen_update_playch10_top(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// Single Monitor version
	if (m_pc10_bios != 1)
		return screen_update_playch10_single(screen, bitmap, cliprect);

	// When the bios is accessing vram, the video circuitry can't access it
	if (!m_pc10_sdcs)
		m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	else
		bitmap.fill(0, cliprect);

	return 0;
}

u32 playch10_state::screen_update_playch10_bottom(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// Single Monitor version
	if (m_pc10_bios != 1)
		return screen_update_playch10_single(screen, bitmap, cliprect);

	if (!m_pc10_dispmask)
		// render the ppu
		m_ppu->render(bitmap, 0, 0, 0, 0, cliprect);
	else
		bitmap.fill(0, cliprect);

	return 0;
}


//******************************************************************************


void playch10_state::up8w_w(int state)
{
	m_up_8w = state;
}

u8 playch10_state::ram_8w_r(offs_t offset)
{
	if (!m_up_8w)
		offset &= 0x3ff;
	return m_ram_8w[offset];
}

void playch10_state::ram_8w_w(offs_t offset, u8 data)
{
	if (!m_up_8w)
		offset &= 0x3ff;
	m_ram_8w[offset] = data;
}

// Only used in single monitor bios

void playch10_state::time_w(offs_t offset, u8 data)
{
	constexpr static u8 DIGIT_MAP[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	m_timedigits[offset] = DIGIT_MAP[data & 0x0f];
}


/*************************************
 *
 *  Init machine
 *
 *************************************/

void playch10_state::machine_reset()
{
	m_pc10_int_detect = 0;

	m_pc10_game_mode = 0;
	m_pc10_dispmask_old = 0;

	m_input_latch[0] = m_input_latch[1] = 0;

	// variables used only in MMC2 game (mapper 9)
	m_MMC2_bank[0] = m_MMC2_bank[1] = m_MMC2_bank[2] = m_MMC2_bank[3] = 0;
	m_MMC2_bank_latch[0] = m_MMC2_bank_latch[1] = 0xfe;

	// reset the security chip
	m_rp5h01->enable_w(1);
	m_rp5h01->enable_w(0);
	m_rp5h01->reset_w(0);
	m_rp5h01->reset_w(1);

	pc10_set_mirroring(m_mirroring);
}

void playch10_state::machine_start()
{
	m_timedigits.resolve();

	m_vrom = m_vrom_region ? m_vrom_region->base() : nullptr;

	// sanity check: make sure PRG/CHR sizes are powers of 2 and big enough
	[[maybe_unused]] int len = memregion("prg")->bytes();
	assert(!(len & (len - 1)) && len >= 0x8000);
	if (m_vrom)
	{
		len = m_vrom_region->bytes();
		assert(!(len & (len - 1)) && len >= 0x2000);
	}

	// allocate 2K of nametable ram here
	// this is on the main board and does not belong to the cart board
	m_nt_ram = std::make_unique<u8[]>(0x800);

	for (int i = 0; i < 4; i++)
		if (m_cart_nt_ram && i >= 2) // extra cart RAM for 4 screen mirroring
			m_nt_page[i]->configure_entries(0, 2, m_cart_nt_ram.get(), 0x400);
		else
			m_nt_page[i]->configure_entries(0, 2, m_nt_ram.get(), 0x400);

	if (m_vram)
		pc10_set_videoram_bank(0, 8, 0, 8);
	else
		pc10_set_videorom_bank(0, 8, 0, 8);
}

/*************************************
 *
 *  BIOS ports handling
 *
 *************************************/

int playch10_state::int_detect_r()
{
	return ~m_pc10_int_detect & 1;
}

void playch10_state::sdcs_w(int state)
{
	/*
	    Hooked to CLR on LS194A - Sheet 2, bottom left.
	    Drives character and color code to 0.
	    It's used to keep the screen black during redraws.
	    Also hooked to the video sram. Prevent writes.
	*/
	m_pc10_sdcs = !state;
}

void playch10_state::cntrl_mask_w(int state)
{
	m_cntrl_mask = !state;
}

void playch10_state::disp_mask_w(int state)
{
	m_pc10_dispmask = !state;
}

void playch10_state::sound_mask_w(int state)
{
	machine().sound().system_mute(!state);
}

void playch10_state::nmi_enable_w(int state)
{
	m_pc10_nmi_enable = state;
}

void playch10_state::dog_di_w(int state)
{
	m_pc10_dog_di = state;
}

void playch10_state::ppu_reset_w(int state)
{
	if (state)
		m_ppu->reset();
}

u8 playch10_state::pc10_detectclr_r()
{
	m_pc10_int_detect = 0;

	return 0;
}

void playch10_state::cart_sel_w(u8 data)
{
	m_cart_sel = data;
}


/*************************************
 *
 *  RP5H01 handling
 *
 *************************************/

u8 playch10_state::pc10_prot_r()
{
	int data = 0xe7;

	// we only support a single cart connected at slot 0
	if (m_cart_sel == 0)
	{
		data |= (~m_rp5h01->counter_r() << 4) & 0x10;    // D4
		data |= (m_rp5h01->data_r() << 3) & 0x08;        // D3
	}
	return data;
}

void playch10_state::pc10_prot_w(u8 data)
{
	// we only support a single cart connected at slot 0
	if (m_cart_sel == 0)
	{
		m_rp5h01->test_w(data & 0x10);       // D4
		m_rp5h01->clock_w(data & 0x08);      // D3
		m_rp5h01->reset_w(~data & 0x01);     // D0
	}
}

/*************************************
 *
 *  Input Ports
 *
 *************************************/

void playch10_state::pc10_in0_w(u8 data)
{
	// Toggling bit 0 high then low resets both controllers
	if (data & 1)
		return;

	// load up the latches
	for (int i = 0; i < 2; i++)
		m_input_latch[i] = m_in[i]->read();

	// apply any masking from the BIOS
	if (m_cntrl_mask)
	{
		// mask out select and start
		m_input_latch[0] &= ~0x0c;
	}
}

u8 playch10_state::pc10_in0_r()
{
	int ret = m_input_latch[0] & 1;

	// shift
	m_input_latch[0] >>= 1;

	// some games expect bit 6 to be set because the last entry on the data bus shows up
	// in the unused upper 3 bits, so typically a read from $4016 leaves 0x40 there.
	ret |= 0x40;

	return ret;
}

u8 playch10_state::pc10_in1_r()
{
	int ret = m_input_latch[1] & 1;

	// shift
	m_input_latch[1] >>= 1;

	// do the gun thing
	if (m_pc10_gun_controller)
	{
		if (!m_sensor->detect_light(m_gunx->read(), m_guny->read()))
			ret |= 0x08;

		// now, add the trigger if not masked
		if (!m_cntrl_mask)
			ret |= m_trigger->read() << 4;
	}

	// some games expect bit 6 to be set because the last entry on the data bus shows up
	// in the unused upper 3 bits, so typically a read from $4016 leaves 0x40 there.
	ret |= 0x40;

	return ret;
}
/*************************************
 *
 *  PPU External bus handlers
 *
 *************************************/

void playch10_state::pc10_chr_w(offs_t offset, u8 data)
{
	int bank = BIT(offset, 10, 3);
	if (m_chr_page[bank].writable)
		m_chr_page[bank].chr[offset & 0x3ff] = data;
}

u8 playch10_state::pc10_chr_r(offs_t offset)
{
	int bank = BIT(offset, 10, 3);
	return m_chr_page[bank].chr[offset & 0x3ff];
}

void playch10_state::pc10_set_mirroring(int mirroring)
{
	int bit = mirroring == PPU_MIRROR_HORZ || mirroring == PPU_MIRROR_HIGH;

	switch (mirroring)
	{
		case PPU_MIRROR_LOW:
		case PPU_MIRROR_HIGH:
			for (int i = 0; i < 4; i++)
				m_nt_page[i]->set_entry(bit);
			break;
		case PPU_MIRROR_HORZ:
		case PPU_MIRROR_VERT:
		case PPU_MIRROR_4SCREEN:
		default:
			for (int i = 0; i < 4; i++)
				m_nt_page[i]->set_entry(BIT(i, bit));
			break;
	}
}


void playch10_state::pc10_set_videorom_bank(int first, int count, int bank, int size)
{
	// first = first bank to map
	// count = number of 1K banks to map
	// bank = index of the bank
	// size = size of indexed banks (in KB)
	// note that this follows the original PPU banking and might be overly complex

	// yeah, this is probably a horrible assumption to make
	// but the driver is 100% consistant

	if (!m_vrom)    // do nothing if there is no CHR ROM to bank
		return;

	int len = m_vrom_region->bytes();
	len /= 0x400;   // convert to KB
	len /= size;    // convert to bank resolution
	len--;          // convert to mask
	bank &= len;    // should be the right mask

	for (int i = 0; i < count; i++)
	{
		m_chr_page[i + first].writable = 0;
		m_chr_page[i + first].chr = m_vrom + (i * 0x400) + (bank * size * 0x400);
	}
}

void playch10_state::pc10_set_videoram_bank(int first, int count, int bank, int size)
{
	// first = first bank to map
	// count = number of 1K banks to map
	// bank = index of the bank
	// size = size of indexed banks (in KB)
	// note that this follows the original PPU banking and might be overly complex

	// assumes 8K of vram
	// need 8K to fill address space
	// only pinbot (8k) banks at all

	for (int i = 0; i < count; i++)
	{
		m_chr_page[i + first].writable = 1;
		m_chr_page[i + first].chr = m_vram.get() + (((i * 0x400) + (bank * size * 0x400)) & 0x1fff);
	}
}

/*************************************
 *
 *  Common init for all games
 *
 *************************************/

void playch10_state::init_playch10()
{
	m_vram = nullptr;

	// set the controller to default
	m_pc10_gun_controller = 0;

	// default mirroring
	m_mirroring = PPU_MIRROR_VERT;
}

/**********************************************************************************
 *
 *  Game and Board-specific initialization
 *
 **********************************************************************************/

// Gun games

void playch10_state::init_pc_gun()
{
	// common init
	init_playch10();

	// set the control type
	m_pc10_gun_controller = 1;
}

// Horizontal mirroring

void playch10_state::init_pc_hrz()
{
	// common init
	init_playch10();

	// setup mirroring
	m_mirroring = PPU_MIRROR_HORZ;
}

// Init for games that bank PRG memory

void playch10_state::init_prg_banking()
{
	// common init
	init_playch10();

	u8 *base = memregion("prg")->base();
	m_prg_chunks = memregion("prg")->bytes() / 0x2000;

	for (int i = 0; i < 4; i++)
	{
		m_prg_banks[i]->configure_entries(0, m_prg_chunks, base, 0x2000);
		m_prg_banks[i]->set_entry(m_prg_chunks - 4 + i);
	}

	m_prg_view.select(0);
}

// safe banking helpers (only work when PRG size is a power of 2)

void playch10_state::prg32(int bank)
{
	bank = (bank << 2) & (m_prg_chunks - 1);

	for (int i = 0; i < 4; i++)
		m_prg_banks[i]->set_entry(bank + i);
}

void playch10_state::prg16(int slot, int bank)
{
	bank = (bank << 1) & (m_prg_chunks - 1);
	slot = (slot & 1) << 1;

	for (int i = 0; i < 2; i++)
		m_prg_banks[slot + i]->set_entry(bank + i);
}

void playch10_state::prg8(int slot, int bank)
{
	m_prg_banks[slot & 0x03]->set_entry(bank & (m_prg_chunks - 1));
}

// MMC1 mapper, used by D, F, and K boards

void playch10_state::mmc1_rom_switch_w(offs_t offset, u8 data)
{
	// reset mapper
	if (data & 0x80)
	{
		m_mmc1_shiftcount = 0;
		m_mmc1_prg16k = 1;
		m_mmc1_switchlow = 1;

		return;
	}

	// update shift register
	m_mmc1_shiftreg = (m_mmc1_shiftreg >> 1) | (data & 1) << 4;
	m_mmc1_shiftcount = (m_mmc1_shiftcount + 1) % 5;

	// are we done shifting?
	if (!m_mmc1_shiftcount)
	{
		// apply data to registers
		switch (BIT(offset, 13, 2))
		{
			case 0: // mirroring and options
			{
				static constexpr u8 mirr[4] = {
					PPU_MIRROR_LOW, PPU_MIRROR_HIGH, PPU_MIRROR_VERT, PPU_MIRROR_HORZ
				};

				pc10_set_mirroring(mirr[m_mmc1_shiftreg & 0x03]);

				m_mmc1_chr4k = m_mmc1_shiftreg & 0x10;
				m_mmc1_prg16k = m_mmc1_shiftreg & 0x08;
				m_mmc1_switchlow = m_mmc1_shiftreg & 0x04;
				break;
			}

			case 1: // video banking - bank 0 - 4k or 8k
				if (m_vram)
				{
					if (m_mmc1_chr4k)
						pc10_set_videoram_bank(0, 4, m_mmc1_shiftreg, 4);
					else
						pc10_set_videoram_bank(0, 8, m_mmc1_shiftreg & ~1, 4);
				}
				else
					if (m_mmc1_chr4k)
						pc10_set_videorom_bank(0, 4, m_mmc1_shiftreg, 4);
					else
						pc10_set_videorom_bank(0, 8, m_mmc1_shiftreg & ~1, 4);
				break;

			case 2: // video banking - bank 1 - 4k only
				if (m_mmc1_chr4k)
				{
					if (m_vram)
						pc10_set_videoram_bank(4, 4, m_mmc1_shiftreg, 4);
					else
						pc10_set_videorom_bank(4, 4, m_mmc1_shiftreg, 4);
				}
				break;

			case 3: // program banking
				if (m_mmc1_prg16k)
					prg16(!m_mmc1_switchlow, m_mmc1_shiftreg);
				else
					prg32(m_mmc1_shiftreg >> 1);
				break;
		}
	}
}

//**********************************************************************************
// A Board games (Track & Field, Gradius)

void playch10_state::aboard_vrom_switch_w(u8 data)
{
	pc10_set_videorom_bank(0, 8, data & 0x03, 8);
}

void playch10_state::init_pcaboard()
{
	// common init
	init_playch10();
}

//**********************************************************************************
// B Board games (Contra, Rush N' Attach, Pro Wrestling)

void playch10_state::bboard_rom_switch_w(u8 data)
{
	prg16(0, data);
}

void playch10_state::init_pcbboard()
{
	// point program banks to last 32K
	init_prg_banking();

	// allocate vram
	m_vram = std::make_unique<u8[]>(0x2000);
}

//**********************************************************************************
// C Board games (The Goonies)

void playch10_state::cboard_vrom_switch_w(u8 data)
{
	pc10_set_videorom_bank(0, 8, BIT(data, 1), 8);
}

void playch10_state::init_pccboard()
{
	// common init
	init_playch10();
}

//**********************************************************************************
// D Board games (Rad Racer, Metroid)

void playch10_state::init_pcdboard()
{
	// point program banks to last 32K
	init_prg_banking();

	// allocate vram
	m_vram = std::make_unique<u8[]>(0x2000);
}

//**********************************************************************************
// E Board (MMC2) games (Mike Tyson's Punchout)

// callback for the ppu_latch
void playch10_state::mapper9_latch(offs_t offset)
{
	if((offset & 0x1ff0) == 0x0fd0 && m_MMC2_bank_latch[0] != 0xfd)
	{
		m_MMC2_bank_latch[0] = 0xfd;
		pc10_set_videorom_bank(0, 4, m_MMC2_bank[0], 4);
	}
	else if((offset & 0x1ff0) == 0x0fe0 && m_MMC2_bank_latch[0] != 0xfe)
	{
		m_MMC2_bank_latch[0] = 0xfe;
		pc10_set_videorom_bank(0, 4, m_MMC2_bank[1], 4);
	}
	else if((offset & 0x1ff0) == 0x1fd0 && m_MMC2_bank_latch[1] != 0xfd)
	{
		m_MMC2_bank_latch[1] = 0xfd;
		pc10_set_videorom_bank(4, 4, m_MMC2_bank[2], 4);
	}
	else if((offset & 0x1ff0) == 0x1fe0 && m_MMC2_bank_latch[1] != 0xfe)
	{
		m_MMC2_bank_latch[1] = 0xfe;
		pc10_set_videorom_bank(4, 4, m_MMC2_bank[3], 4);
	}
}

void playch10_state::eboard_rom_switch_w(offs_t offset, u8 data)
{
	// a variation of mapper 9 on a nes
	switch (offset & 0x7000)
	{
		case 0x2000: // code bank switching
			prg8(0, data);
			break;

		case 0x3000: // gfx bank 0 - 4k
			m_MMC2_bank[0] = data;
			if (m_MMC2_bank_latch[0] == 0xfd)
				pc10_set_videorom_bank(0, 4, data, 4);
			break;

		case 0x4000: // gfx bank 0 - 4k
			m_MMC2_bank[1] = data;
			if (m_MMC2_bank_latch[0] == 0xfe)
				pc10_set_videorom_bank(0, 4, data, 4);
			break;

		case 0x5000: // gfx bank 1 - 4k
			m_MMC2_bank[2] = data;
			if (m_MMC2_bank_latch[1] == 0xfd)
				pc10_set_videorom_bank(4, 4, data, 4);
			break;

		case 0x6000: // gfx bank 1 - 4k
			m_MMC2_bank[3] = data;
			if (m_MMC2_bank_latch[1] == 0xfe)
				pc10_set_videorom_bank(4, 4, data, 4);
			break;

		case 0x7000: // mirroring
			pc10_set_mirroring(data & 1 ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;
	}
}

void playch10_state::init_pceboard()
{
	// point program banks to last 32K
	init_prg_banking();

	// ppu_latch callback
	m_ppu->set_latch(*this, FUNC(playch10_state::mapper9_latch));
}

//**********************************************************************************
// F Board games (Ninja Gaiden, Double Dragon)

void playch10_state::init_pcfboard()
{
	// point program banks to last 32K
	init_prg_banking();
}

//**********************************************************************************
// G Board (MMC3) games (Super Mario Bros. 3, etc)

void playch10_state::gboard_scanline_cb(int scanline, bool vblank, bool blanked)
{
	if (scanline < ppu2c0x_device::BOTTOM_VISIBLE_SCANLINE)
	{
		int priorCount = m_IRQ_count;
		if (m_IRQ_count)
			m_IRQ_count--;
		else
			m_IRQ_count = m_IRQ_count_latch;

		if (m_IRQ_enable && !blanked && !m_IRQ_count && priorCount) // according to blargg the latter should be present as well, but it breaks Rampart and Joe & Mac US: they probably use the alt irq!
		{
			m_cartcpu->set_input_line(0, ASSERT_LINE);
		}
	}
}

void playch10_state::gboard_rom_switch_w(offs_t offset, u8 data)
{
	switch (offset & 0x6001)
	{
		case 0x0000:
		{
			m_gboard_command = data;

			// reset the flippable banks
			int flip = BIT(m_gboard_command, 6) << 1;
			prg8(0 ^ flip, m_gboard_banks[0]);
			prg8(2 ^ flip, m_prg_chunks - 2);

			break;
		}

		case 0x0001:
		{
			u8 cmd = m_gboard_command & 0x07;
			int page = (m_gboard_command & 0x80) >> 5;

			switch (cmd)
			{
				case 0: // char banking
				case 1: // char banking
					data &= 0xfe;
					page ^= cmd << 1;
					pc10_set_videorom_bank(page, 2, data, 1);
					break;

				case 2: // char banking
				case 3: // char banking
				case 4: // char banking
				case 5: // char banking
					page ^= cmd + 2;
					pc10_set_videorom_bank(page, 1, data, 1);
					break;

				case 6: // program banking
				{
					m_gboard_banks[0] = data;

					int flip = BIT(m_gboard_command, 6) << 1;
					prg8(0 ^ flip, m_gboard_banks[0]);
					prg8(2 ^ flip, m_prg_chunks - 2);
					break;
				}

				case 7: // program banking - mid bank
					m_gboard_banks[1] = data;
					prg8(1, m_gboard_banks[1]);
					break;
			}
			break;
		}

		case 0x2000: // mirroring
			if (m_mirroring != PPU_MIRROR_4SCREEN)
				pc10_set_mirroring((data & 1) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			break;

		case 0x2001: // enable ram at $6000
			// ignored - we always enable it
			 break;

		case 0x4000: // scanline counter
			m_IRQ_count_latch = data;
			break;

		case 0x4001: // scanline latch
			m_IRQ_count = 0;
			break;

		case 0x6000: // disable irqs
			m_IRQ_enable = 0;
			m_cartcpu->set_input_line(0, CLEAR_LINE);
			break;

		case 0x6001: // enable irqs
			m_IRQ_enable = 1;
			break;
	}
}

void playch10_state::init_pcgboard()
{
	// point program banks to last 32K
	init_prg_banking();

	m_gboard_banks[0] = 0x1e;
	m_gboard_banks[1] = 0x1f;
	m_gboard_command = 0;
	m_IRQ_enable = 0;
	m_IRQ_count = m_IRQ_count_latch = 0;

	m_ppu->set_scanline_callback(*this, FUNC(playch10_state::gboard_scanline_cb));
}

void playch10_state::init_pcgboard_type2()
{
	// common init
	init_pcgboard();

	// enable 4 screen mirroring
	// 2K on the cart board, in addition to the 2K on the main board
	m_cart_nt_ram = std::make_unique<u8[]>(0x800);
	m_mirroring = PPU_MIRROR_4SCREEN;
}

//**********************************************************************************
// H Board games (PinBot)

void playch10_state::hboard_rom_switch_w(offs_t offset, u8 data)
{
	switch (offset & 0x6001)
	{
		case 0x0001:
		{
			u8 cmd = m_gboard_command & 0x07;
			int page = (m_gboard_command & 0x80) >> 5;

			switch (cmd)
			{
				case 0: // char banking
				case 1: // char banking
					data &= 0xfe;
					page ^= (cmd << 1);
					if (data & 0x40)
						pc10_set_videoram_bank(page, 2, data, 1);
					else
						pc10_set_videorom_bank(page, 2, data, 1);
					return;

				case 2: // char banking
				case 3: // char banking
				case 4: // char banking
				case 5: // char banking
					page ^= cmd + 2;
					if (data & 0x40)
						pc10_set_videoram_bank(page, 1, data, 1);
					else
						pc10_set_videorom_bank(page, 1, data, 1);
					return;
			}
		}
	}

	gboard_rom_switch_w(offset,data);
}

void playch10_state::init_pchboard()
{
	// common init
	init_pcgboard();

	// allocate vram
	m_vram = std::make_unique<u8[]>(0x2000);
}

//**********************************************************************************
// i Board games (Captain Sky Hawk, Solar Jetman)

void playch10_state::iboard_rom_switch_w(u8 data)
{
	prg32(data);

	pc10_set_mirroring(BIT(data, 4) ? PPU_MIRROR_HIGH : PPU_MIRROR_LOW);
}

void playch10_state::init_pciboard()
{
	// point program banks to last 32K
	init_prg_banking();

	m_mirroring = PPU_MIRROR_LOW;

	// allocate vram
	m_vram = std::make_unique<u8[]>(0x2000);
}

//**********************************************************************************
// K Board games (Mario Open Golf)

void playch10_state::init_pckboard()
{
	// later board but similar to D boards, i.e. MMC1 + 8K VRAM
	init_pcdboard();
}


//******************************************************************************

// BIOS
void playch10_state::bios_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x8000, 0x87ff).ram(); // 8V
	map(0x8800, 0x8fff).rw(FUNC(playch10_state::ram_8w_r), FUNC(playch10_state::ram_8w_w)).share("ram_8w");    // 8W
	map(0x9000, 0x97ff).ram().w(FUNC(playch10_state::playch10_videoram_w)).share("videoram");
	map(0xc000, 0xdfff).rom();
	map(0xe000, 0xffff).rw(FUNC(playch10_state::pc10_prot_r), FUNC(playch10_state::pc10_prot_w));
}

void playch10_state::bios_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("BIOS");
	map(0x01, 0x01).portr("SW1");
	map(0x02, 0x02).portr("SW2");
	map(0x03, 0x03).r(FUNC(playch10_state::pc10_detectclr_r));
	map(0x00, 0x07).w("outlatch1", FUNC(ls259_device::write_d0));
	map(0x08, 0x0f).w("outlatch2", FUNC(ls259_device::write_d0));
	map(0x10, 0x13).w(FUNC(playch10_state::time_w));
}

void playch10_state::ppu_map(address_map &map)
{
	map(0x0000, 0x1fff).rw(FUNC(playch10_state::pc10_chr_r), FUNC(playch10_state::pc10_chr_w));
	map(0x2000, 0x23ff).mirror(0x1000).bankrw(m_nt_page[0]);
	map(0x2400, 0x27ff).mirror(0x1000).bankrw(m_nt_page[1]);
	map(0x2800, 0x2bff).mirror(0x1000).bankrw(m_nt_page[2]);
	map(0x2c00, 0x2fff).mirror(0x1000).bankrw(m_nt_page[3]);
	map(0x3f00, 0x3fff).rw(m_ppu, FUNC(ppu2c0x_device::palette_read), FUNC(ppu2c0x_device::palette_write));
}

void playch10_state::cart_map(address_map &map)
{
	map(0x0000, 0x07ff).mirror(0x1800).ram();
	map(0x2000, 0x3fff).rw(m_ppu, FUNC(ppu2c0x_device::read), FUNC(ppu2c0x_device::write));
	map(0x4014, 0x4014).w(m_ppu, FUNC(ppu2c0x_device::spriteram_dma));
	map(0x4016, 0x4016).rw(FUNC(playch10_state::pc10_in0_r), FUNC(playch10_state::pc10_in0_w));
	map(0x4017, 0x4017).r(FUNC(playch10_state::pc10_in1_r));  // IN1 - input port 2 / PSG second control register
	// Games that don't bank PRG
	map(0x8000, 0xffff).rom().region("prg", 0);
	// Games that bank PRG
	map(0x8000, 0xffff).view(m_prg_view);
	m_prg_view[0](0x8000, 0x9fff).bankr(m_prg_banks[0]);
	m_prg_view[0](0xa000, 0xbfff).bankr(m_prg_banks[1]);
	m_prg_view[0](0xc000, 0xdfff).bankr(m_prg_banks[2]);
	m_prg_view[0](0xe000, 0xffff).bankr(m_prg_banks[3]);
}

void playch10_state::cart_a_map(address_map &map)
{
	cart_map(map);

	// switches vrom with writes to the $803e-$8041 area
	map(0x8000, 0x8fff).w(FUNC(playch10_state::aboard_vrom_switch_w));
}

void playch10_state::cart_b_map(address_map &map)
{
	cart_map(map);

	// Roms are banked at $8000 to $bfff
	map(0x8000, 0xffff).w(FUNC(playch10_state::bboard_rom_switch_w));
}

void playch10_state::cart_c_map(address_map &map)
{
	cart_map(map);

	// switches vrom with writes to $6000
	map(0x6000, 0x6000).w(FUNC(playch10_state::cboard_vrom_switch_w));
}

void playch10_state::cart_d_map(address_map &map)
{
	cart_map(map);

	// MMC1 mapper at $8000-$ffff
	map(0x8000, 0xffff).w(FUNC(playch10_state::mmc1_rom_switch_w));
}

void playch10_state::cart_d2_map(address_map &map)
{
	cart_d_map(map);

	// extra ram at $6000-$7fff
	map(0x6000, 0x7fff).ram();
}

void playch10_state::cart_e_map(address_map &map)
{
	cart_map(map);

	// nvram at $6000-$7fff
	map(0x6000, 0x7fff).ram().share("nvram");

	// MMC2 mapper at $8000-$ffff
	map(0x8000, 0xffff).w(FUNC(playch10_state::eboard_rom_switch_w));
}

void playch10_state::cart_f_map(address_map &map)
{
	cart_map(map);

	// MMC1 mapper at $8000-$ffff
	map(0x8000, 0xffff).w(FUNC(playch10_state::mmc1_rom_switch_w));
}

void playch10_state::cart_f2_map(address_map &map)
{
	cart_f_map(map);

	// extra ram at $6000-$7fff
	map(0x6000, 0x7fff).ram();
}

void playch10_state::cart_g_map(address_map &map)
{
	cart_map(map);

	// extra ram at $6000-$7fff
	map(0x6000, 0x7fff).ram();

	// MMC3 mapper at $8000-$ffff
	map(0x8000, 0xffff).w(FUNC(playch10_state::gboard_rom_switch_w));
}

void playch10_state::cart_h_map(address_map &map)
{
	cart_map(map);

	// MMC3 mapper at $8000-$ffff
	map(0x8000, 0xffff).w(FUNC(playch10_state::hboard_rom_switch_w));
}

void playch10_state::cart_i_map(address_map &map)
{
	cart_map(map);

	// Roms are banked at $8000 to $ffff
	map(0x8000, 0xffff).w(FUNC(playch10_state::iboard_rom_switch_w));
}

void playch10_state::cart_k_map(address_map &map)
{
	cart_map(map);

	// extra ram at $6000-$7fff
	map(0x6000, 0x7fff).ram();

	// MMC1 mapper at $8000-$ffff
	map(0x8000, 0xffff).w(FUNC(playch10_state::mmc1_rom_switch_w));
}

//******************************************************************************

static INPUT_PORTS_START( playch10 )
	PORT_START("BIOS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Channel Select") PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Enter") PORT_CODE(KEYCODE_MINUS)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Reset") PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(playch10_state::int_detect_r))   // INT Detect
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

/*

    The correct way to handle DIPs according to the manual.
    Doesn't work due to limitations of the conditional DIPs
    implementation in MAME.


    PORT_START("SW1")
    PORT_DIPNAME( 0x3f, 0x09, "Prime Time Bonus" )
    // STANDARD TIME (no bonus)
    PORT_DIPSETTING(    0x00, "0%" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0xc0 )
    // PRIME TIME (bonus) for 2 COINS
    PORT_DIPSETTING(    0x07, "8%" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    PORT_DIPSETTING(    0x08, "17%" )   PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    PORT_DIPSETTING(    0x09, "25%" )   PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    PORT_DIPSETTING(    0x10, "33%" )   PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    PORT_DIPSETTING(    0x11, "42%" )   PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    PORT_DIPSETTING(    0x12, "50%" )   PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    PORT_DIPSETTING(    0x13, "58%" )   PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    PORT_DIPSETTING(    0x14, "67%" )   PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    PORT_DIPSETTING(    0x15, "75%" )   PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    PORT_DIPSETTING(    0x16, "83%" )   PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    PORT_DIPSETTING(    0x17, "92%" )   PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    PORT_DIPSETTING(    0x18, "100%" )  PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    // PRIME TIME (bonus) for 4 COINS
    PORT_DIPSETTING(    0x04, "8%" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x00 )
    PORT_DIPSETTING(    0x05, "17%" )   PORT_CONDITION("SW2", 0xc0, EQUALS, 0x00 )
    PORT_DIPSETTING(    0x06, "25%" )   PORT_CONDITION("SW2", 0xc0, EQUALS, 0x00 )
    PORT_DIPSETTING(    0x07, "33%" )   PORT_CONDITION("SW2", 0xc0, EQUALS, 0x00 )
    PORT_DIPSETTING(    0x08, "42%" )   PORT_CONDITION("SW2", 0xc0, EQUALS, 0x00 )
    PORT_DIPSETTING(    0x09, "50%" )   PORT_CONDITION("SW2", 0xc0, EQUALS, 0x00 )
    PORT_DIPSETTING(    0x10, "58%" )   PORT_CONDITION("SW2", 0xc0, EQUALS, 0x00 )
    PORT_DIPSETTING(    0x11, "67%" )   PORT_CONDITION("SW2", 0xc0, EQUALS, 0x00 )
    PORT_DIPSETTING(    0x12, "75%" )   PORT_CONDITION("SW2", 0xc0, EQUALS, 0x00 )
    PORT_DIPSETTING(    0x13, "83%" )   PORT_CONDITION("SW2", 0xc0, EQUALS, 0x00 )
    PORT_DIPSETTING(    0x14, "92%" )   PORT_CONDITION("SW2", 0xc0, EQUALS, 0x00 )
    PORT_DIPSETTING(    0x15, "100%" )  PORT_CONDITION("SW2", 0xc0, EQUALS, 0x00 )

    PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )
    PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x40, DEF_STR( On ) )
    PORT_SERVICE( 0x80, IP_ACTIVE_HIGH )

    PORT_START("SW2")
    PORT_DIPNAME( 0x3f, 0x28, "Play Time/Coin" )
    // STANDARD TIME (no bonus)
    PORT_DIPSETTING(    0x3f, DEF_STR( Free_Play ) )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    PORT_DIPSETTING(    0x23, "2:00 (120)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0xc0 )
    PORT_DIPSETTING(    0x21, "2:10 (130)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0xc0 )
    PORT_DIPSETTING(    0x1f, "2:20 (140)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0xc0 )
    PORT_DIPSETTING(    0x1d, "2:30 (150)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0xc0 )
    PORT_DIPSETTING(    0x1b, "2:40 (160)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0xc0 )
    PORT_DIPSETTING(    0x19, "2:50 (170)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0xc0 )
    PORT_DIPSETTING(    0x17, "3:00 (180)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0xc0 )
    PORT_DIPSETTING(    0x15, "3:10 (190)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0xc0 )
    PORT_DIPSETTING(    0x13, "3:20 (200)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0xc0 )
    PORT_DIPSETTING(    0x11, "3:30 (210)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0xc0 )
    PORT_DIPSETTING(    0x0f, "3:40 (220)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0xc0 )
    PORT_DIPSETTING(    0x0d, "3:50 (230)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0xc0 )
    PORT_DIPSETTING(    0x0b, "4:00 (240)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0xc0 )
    PORT_DIPSETTING(    0x09, "4:10 (250)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0xc0 )
    PORT_DIPSETTING(    0x07, "4:20 (260)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0xc0 )
    PORT_DIPSETTING(    0x05, "4:30 (270)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0xc0 )
    PORT_DIPSETTING(    0x03, "4:40 (280)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0xc0 )
    PORT_DIPSETTING(    0x01, "4:50 (290)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0xc0 )
    // PRIME TIME (bonus) for 2 COINS
    PORT_DIPSETTING(    0x1c, "2:00 (120)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    PORT_DIPSETTING(    0x1e, "2:10 (130)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    PORT_DIPSETTING(    0x20, "2:20 (140)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    PORT_DIPSETTING(    0x22, "2:30 (150)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    PORT_DIPSETTING(    0x24, "2:40 (160)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    PORT_DIPSETTING(    0x26, "2:50 (170)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    PORT_DIPSETTING(    0x28, "3:00 (180)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    PORT_DIPSETTING(    0x2a, "3:10 (190)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    PORT_DIPSETTING(    0x2c, "3:20 (200)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    PORT_DIPSETTING(    0x2e, "3:30 (210)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    PORT_DIPSETTING(    0x30, "3:40 (220)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    PORT_DIPSETTING(    0x32, "3:50 (230)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    PORT_DIPSETTING(    0x34, "4:00 (240)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x80 )
    // PRIME TIME (bonus) for 4 COINS
    PORT_DIPSETTING(    0x1c, "2:00 (120)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x00 )
    PORT_DIPSETTING(    0x1e, "2:10 (130)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x00 )
    PORT_DIPSETTING(    0x20, "2:20 (140)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x00 )
    PORT_DIPSETTING(    0x22, "2:30 (150)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x00 )
    PORT_DIPSETTING(    0x24, "2:40 (160)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x00 )
    PORT_DIPSETTING(    0x26, "2:50 (170)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x00 )
    PORT_DIPSETTING(    0x28, "3:00 (180)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x00 )
    PORT_DIPSETTING(    0x2a, "3:10 (190)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x00 )
    PORT_DIPSETTING(    0x2c, "3:20 (200)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x00 )
    PORT_DIPSETTING(    0x2e, "3:30 (210)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x00 )
    PORT_DIPSETTING(    0x30, "3:40 (220)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x00 )
    PORT_DIPSETTING(    0x32, "3:50 (230)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x00 )
    PORT_DIPSETTING(    0x34, "4:00 (240)" )    PORT_CONDITION("SW2", 0xc0, EQUALS, 0x00 )

    PORT_DIPNAME( 0xc0, 0x80, "Bonus" )
    PORT_DIPSETTING(    0xc0, "Standard Time" )
    PORT_DIPSETTING(    0x80, "Prime Time for 2 Coins" )
    PORT_DIPSETTING(    0x00, "Prime Time for 4 Coins" )
*/
	PORT_START("SW1")
	PORT_DIPNAME( 0x3f, 0x00, "Units of time (coin A/coin B)" )
	PORT_DIPSETTING(    0x00, "300/0" )
	PORT_DIPSETTING(    0x01, "300/100" )
	PORT_DIPSETTING(    0x02, "300/200" )
	PORT_DIPSETTING(    0x03, "300/300" )
	PORT_DIPSETTING(    0x04, "300/400" )
	PORT_DIPSETTING(    0x05, "300/500" )
	PORT_DIPSETTING(    0x06, "300/600" )
	PORT_DIPSETTING(    0x07, "300/700" )
	PORT_DIPSETTING(    0x08, "300/800" )
	PORT_DIPSETTING(    0x09, "300/900" )
	PORT_DIPSETTING(    0x0a, "150/0" )
	PORT_DIPSETTING(    0x0b, "150/200" )
	PORT_DIPSETTING(    0x0c, "150/400" )
	PORT_DIPSETTING(    0x0f, "150/500" )
	PORT_DIPSETTING(    0x0d, "150/600" )
	PORT_DIPSETTING(    0x0e, "150/800" )
	PORT_DIPSETTING(    0x10, "300/1000" )
	PORT_DIPSETTING(    0x11, "300/1100" )
	PORT_DIPSETTING(    0x12, "300/1200" )
	PORT_DIPSETTING(    0x13, "300/1300" )
	PORT_DIPSETTING(    0x14, "300/1400" )
	PORT_DIPSETTING(    0x15, "300/1500" )
	PORT_DIPSETTING(    0x16, "300/1600" )
	PORT_DIPSETTING(    0x17, "300/1700" )
	PORT_DIPSETTING(    0x18, "300/1800" )
	PORT_DIPSETTING(    0x19, "300/1900" )
	PORT_DIPSETTING(    0x1a, "150/1000" )
	PORT_DIPSETTING(    0x1b, "150/1200" )
	PORT_DIPSETTING(    0x1c, "150/1400" )
	PORT_DIPSETTING(    0x1f, "150/1500" )
	PORT_DIPSETTING(    0x1d, "150/1600" )
	PORT_DIPSETTING(    0x1e, "150/1800" )
	PORT_DIPSETTING(    0x20, "300/2000" )
	PORT_DIPSETTING(    0x21, "300/2100" )
	PORT_DIPSETTING(    0x22, "300/2200" )
	PORT_DIPSETTING(    0x23, "300/2300" )
	PORT_DIPSETTING(    0x24, "300/2400" )
	PORT_DIPSETTING(    0x25, "300/2500" )
	PORT_DIPSETTING(    0x26, "300/2600" )
	PORT_DIPSETTING(    0x27, "300/2700" )
	PORT_DIPSETTING(    0x28, "300/2800" )
	PORT_DIPSETTING(    0x29, "300/2900" )
	PORT_DIPSETTING(    0x2a, "150/2000" )
	PORT_DIPSETTING(    0x2b, "150/2200" )
	PORT_DIPSETTING(    0x2c, "150/2400" )
	PORT_DIPSETTING(    0x2f, "150/2500" )
	PORT_DIPSETTING(    0x2d, "150/2600" )
	PORT_DIPSETTING(    0x2e, "150/2800" )
	PORT_DIPSETTING(    0x30, "300/3000" )
	PORT_DIPSETTING(    0x31, "300/3100" )
	PORT_DIPSETTING(    0x32, "300/3200" )
	PORT_DIPSETTING(    0x33, "300/3300" )
	PORT_DIPSETTING(    0x34, "300/3400" )
	PORT_DIPSETTING(    0x35, "300/3500" )
	PORT_DIPSETTING(    0x36, "300/3600" )
	PORT_DIPSETTING(    0x37, "300/3700" )
	PORT_DIPSETTING(    0x38, "300/3800" )
	PORT_DIPSETTING(    0x39, "300/3900" )
	PORT_DIPSETTING(    0x3a, "150/3000" )
	PORT_DIPSETTING(    0x3b, "150/3200" )
	PORT_DIPSETTING(    0x3c, "150/3400" )
	PORT_DIPSETTING(    0x3f, "150/3500" )
	PORT_DIPSETTING(    0x3d, "150/3600" )
	PORT_DIPSETTING(    0x3e, "150/3800" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_HIGH )

	PORT_START("SW2")
	PORT_DIPNAME( 0x40, 0x00, "Coin Mode" )
	PORT_DIPSETTING(    0x00, "Mode 1" )
	PORT_DIPSETTING(    0x40, "Mode 2" )
	PORT_DIPNAME( 0xbf, 0x3f, "Timer speed" )
	PORT_DIPSETTING(    0x05, "60 units per second" )
	PORT_DIPSETTING(    0x06, "30 units per second" )
	PORT_DIPSETTING(    0x07, "20 units per second" )
	PORT_DIPSETTING(    0x08, "15 units per second" )
	PORT_DIPSETTING(    0x0a, "10 units per second" )
	PORT_DIPSETTING(    0x0e, "6 units per second" )
	PORT_DIPSETTING(    0x10, "5 units per second" )
	PORT_DIPSETTING(    0x13, "4 units per second" )
	PORT_DIPSETTING(    0x18, "3 units per second" )
	PORT_DIPSETTING(    0x22, "2 units per second" )
	PORT_DIPSETTING(    0x3f, "1 unit per second" )
	PORT_DIPSETTING(    0x00, "1 unit every 4 seconds" )
	PORT_DIPSETTING(    0x80, DEF_STR( Free_Play ) )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("%p A") PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("%p B") PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START2 ) PORT_NAME("Game Select")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Start")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("%p A") PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("%p B") PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )    // wired to 1p select button
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )    // wired to 1p start button
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
INPUT_PORTS_END

// Input Ports for gun games
static INPUT_PORTS_START( playc10g )
	PORT_INCLUDE(playch10)

	PORT_START("TRIGGER")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Gun Trigger")

	PORT_START("GUNX")  // IN2 - FAKE - Gun X pos
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(30) PORT_MINMAX(0, 255)

	PORT_START("GUNY")  // IN3 - FAKE - Gun Y pos
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(30) PORT_MINMAX(0, 239)
INPUT_PORTS_END


static const gfx_layout bios_charlayout =
{
	8,8,    // 8*8 characters
	1024,   // 1024 characters
	3,      // 3 bits per pixel
	{ 0, 0x2000*8, 0x4000*8 },     // the bitplanes are separated
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8     // every char takes 8 consecutive bytes
};

static GFXDECODE_START( gfx_playch10 )
	GFXDECODE_ENTRY( "gfx1", 0, bios_charlayout,   0,  32 )
GFXDECODE_END

void playch10_state::vblank_irq(int state)
{
	if (state)
	{
		// LS161A, Sheet 1 - bottom left of Z80
		if (!m_pc10_dog_di && !m_pc10_nmi_enable)
			m_maincpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
		else if (m_pc10_nmi_enable)
			m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	}
}

void playch10_state::playch10(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 8000000/2); // 4 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &playch10_state::bios_map);
	m_maincpu->set_addrmap(AS_IO, &playch10_state::bios_io_map);

	RP2A03G(config, m_cartcpu, NTSC_APU_CLOCK); // really RP2A03E
	m_cartcpu->set_addrmap(AS_PROGRAM, &playch10_state::cart_map);

	ls259_device &outlatch1(LS259(config, "outlatch1")); // 7D
	outlatch1.q_out_cb<0>().set(FUNC(playch10_state::sdcs_w));
	outlatch1.q_out_cb<1>().set(FUNC(playch10_state::cntrl_mask_w));
	outlatch1.q_out_cb<2>().set(FUNC(playch10_state::disp_mask_w));
	outlatch1.q_out_cb<3>().set(FUNC(playch10_state::sound_mask_w));
	outlatch1.q_out_cb<4>().set_inputline(m_cartcpu, INPUT_LINE_RESET).invert(); // GAMERES
	outlatch1.q_out_cb<5>().set_inputline(m_cartcpu, INPUT_LINE_HALT).invert(); // GAMESTOP

	ls259_device &outlatch2(LS259(config, "outlatch2")); // 7E
	outlatch2.q_out_cb<0>().set(FUNC(playch10_state::nmi_enable_w));
	outlatch2.q_out_cb<1>().set(FUNC(playch10_state::dog_di_w));
	outlatch2.q_out_cb<2>().set(FUNC(playch10_state::ppu_reset_w));
	outlatch2.q_out_cb<7>().set(FUNC(playch10_state::up8w_w));
	outlatch2.parallel_out_cb().set(FUNC(playch10_state::cart_sel_w)).rshift(3).mask(0x0f);

	// video hardware
	GFXDECODE(config, m_gfxdecode, "palette", gfx_playch10);
	PALETTE(config, "palette", FUNC(playch10_state::playch10_palette), 256);
	config.set_default_layout(layout_playch10);

	screen_device &bottom(SCREEN(config, "bottom", SCREEN_TYPE_RASTER));
	bottom.set_refresh_hz(60);
	bottom.set_size(32*8, 262);
	bottom.set_visarea(0*8, 32*8-1, 0*8, 30*8-1);
	bottom.set_screen_update(FUNC(playch10_state::screen_update_playch10_bottom));

	screen_device &top(SCREEN(config, "top", SCREEN_TYPE_RASTER));
	top.set_refresh_hz(60);
	top.set_size(32*8, 262);
	top.set_visarea(0*8, 32*8-1, 0*8, 30*8-1);
	top.set_screen_update(FUNC(playch10_state::screen_update_playch10_top));
	top.screen_vblank().set(FUNC(playch10_state::vblank_irq));

	PPU_2C03B(config, m_ppu, 0);
	m_ppu->set_addrmap(0, &playch10_state::ppu_map);
	m_ppu->set_screen("bottom");
	m_ppu->set_cpu_tag("cart");
	m_ppu->int_callback().set_inputline(m_cartcpu, INPUT_LINE_NMI);
	m_ppu->int_callback().append(FUNC(playch10_state::int_detect_w));

	NES_ZAPPER_SENSOR(config, m_sensor, 0).set_screen_tag("bottom");

	SPEAKER(config, "mono").front_center();
	m_cartcpu->add_route(ALL_OUTPUTS, "mono", 0.50);

	RP5H01(config, m_rp5h01, 0);
}

void playch10_state::playch10_a(machine_config &config)
{
	playch10(config);
	m_cartcpu->set_addrmap(AS_PROGRAM, &playch10_state::cart_a_map);
}

void playch10_state::playch10_b(machine_config &config)
{
	playch10(config);
	m_cartcpu->set_addrmap(AS_PROGRAM, &playch10_state::cart_b_map);
}

void playch10_state::playch10_c(machine_config &config)
{
	playch10(config);
	m_cartcpu->set_addrmap(AS_PROGRAM, &playch10_state::cart_c_map);
}

void playch10_state::playch10_d(machine_config &config)
{
	playch10(config);
	m_cartcpu->set_addrmap(AS_PROGRAM, &playch10_state::cart_d_map);
}

void playch10_state::playch10_d2(machine_config &config)
{
	playch10(config);
	m_cartcpu->set_addrmap(AS_PROGRAM, &playch10_state::cart_d2_map);
}

void playch10_state::playch10_e(machine_config &config)
{
	playch10(config);
	m_cartcpu->set_addrmap(AS_PROGRAM, &playch10_state::cart_e_map);
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
}

void playch10_state::playch10_f(machine_config &config)
{
	playch10(config);
	m_cartcpu->set_addrmap(AS_PROGRAM, &playch10_state::cart_f_map);
}

void playch10_state::playch10_f2(machine_config &config)
{
	playch10(config);
	m_cartcpu->set_addrmap(AS_PROGRAM, &playch10_state::cart_f2_map);
}

void playch10_state::playch10_g(machine_config &config)
{
	playch10(config);
	m_cartcpu->set_addrmap(AS_PROGRAM, &playch10_state::cart_g_map);
}

void playch10_state::playch10_h(machine_config &config)
{
	playch10(config);
	m_cartcpu->set_addrmap(AS_PROGRAM, &playch10_state::cart_h_map);
}

void playch10_state::playch10_i(machine_config &config)
{
	playch10(config);
	m_cartcpu->set_addrmap(AS_PROGRAM, &playch10_state::cart_i_map);
}

void playch10_state::playch10_k(machine_config &config)
{
	playch10(config);
	m_cartcpu->set_addrmap(AS_PROGRAM, &playch10_state::cart_k_map);
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

#define ROM_LOAD_BIOS(bios,name,offset,length,hash) \
	ROMX_LOAD(name, offset, length, hash, ROM_BIOS(bios))

#define BIOS_CPU                                            \
	ROM_SYSTEM_BIOS( 0, "dual",    "Dual Monitor Version" ) \
	ROM_SYSTEM_BIOS( 1, "single",  "Single Monitor Version" ) \
	ROM_SYSTEM_BIOS( 2, "alt",     "Alternate BIOS" ) /* this bios doesn't work properly, selecting service mode causes it to hang, is it good? maybe different hw? */ \
	ROM_SYSTEM_BIOS( 3, "singleb", "Single Monitor Version (Newer?)" ) /* Newer single screen? Four bytes different, reported bugfix in freeplay */ \
	ROM_SYSTEM_BIOS( 4, "dualb",   "Dual Monitor Version (alternate)" ) /* this BIOS doesn't work properly, was found on a PCH1-03-CPU PCB */ \
	ROM_REGION( 0x10000, "maincpu", 0 )                     \
	ROM_LOAD_BIOS( 0, "pch1-c__8t_e-2.8t", 0x00000, 0x4000, CRC(d52fa07a) SHA1(55cabf52ae10c050c2229081a80b9fe5454ab8c5) ) \
	ROM_LOAD_BIOS( 1, "pck1-c.8t",         0x00000, 0x4000, CRC(503ee8b1) SHA1(3bd20bc71cac742d1b8c1430a6426d0a19db7ad0) ) \
	ROM_LOAD_BIOS( 2, "pch1-c_8te.8t",     0x00000, 0x4000, CRC(123ffa37) SHA1(3bef754a5a85a8498bb6222ddf5cb9021f264db5) ) \
	ROM_LOAD_BIOS( 3, "pck1-c_fix.8t",     0x00000, 0x4000, CRC(0be8ceb4) SHA1(45b127a537370226e6b30be2b5a92ad05673ca7f) ) \
	ROM_LOAD_BIOS( 4, "pch1-c__8t_e-1.8t", 0x00000, 0x4000, CRC(88a8f87e) SHA1(d1d7ffe68d2e7aa2faf8ad41a7ec8421265ede42) )


#define BIOS_GFX                                            \
	ROM_REGION( 0x6000, "gfx1", 0 ) \
	ROM_LOAD_BIOS( 0, "pch1-c__8k.8k",      0x00000, 0x2000, CRC(9acffb30) SHA1(b814f10ef23f2ca445fabafcbf7f25e2d454ba8c) )   \
	ROM_LOAD_BIOS( 0, "pch1-c__8m_e-1.8m",  0x02000, 0x2000, CRC(c1232eee) SHA1(beaf9fa2d091a3c7f70c51e966d885b1f9f0935f) )   \
	ROM_LOAD_BIOS( 0, "pch1-c__8p_e-1.8p",  0x04000, 0x2000, CRC(30c15e23) SHA1(69166afdb2fe827c7f1919cdf4197caccbd961fa) )   \
	\
	ROM_LOAD_BIOS( 1, "pch1-c__8k.8k",      0x00000, 0x2000, CRC(9acffb30) SHA1(b814f10ef23f2ca445fabafcbf7f25e2d454ba8c) )   \
	ROM_LOAD_BIOS( 1, "pch1-c__8m_e-1.8m",  0x02000, 0x2000, CRC(c1232eee) SHA1(beaf9fa2d091a3c7f70c51e966d885b1f9f0935f) )   \
	ROM_LOAD_BIOS( 1, "pch1-c__8p_e-1.8p",  0x04000, 0x2000, CRC(30c15e23) SHA1(69166afdb2fe827c7f1919cdf4197caccbd961fa) )   \
	\
	ROM_LOAD_BIOS( 2, "pch1-c__8k.8k",      0x00000, 0x2000, CRC(9acffb30) SHA1(b814f10ef23f2ca445fabafcbf7f25e2d454ba8c) ) \
	ROM_LOAD_BIOS( 2, "pch1-c_8m.8m",       0x02000, 0x2000, CRC(83ebc7a3) SHA1(a7c607138f4f9b96ab5d3a82c47895f77672e296) )   \
	ROM_LOAD_BIOS( 2, "pch1-c_8p-8p",       0x04000, 0x2000, CRC(90e1b80c) SHA1(c4f4b135b2a11743518aaa0554c365b4a8cf299a) )   \
	\
	ROM_LOAD_BIOS( 3, "pch1-c__8k.8k",      0x00000, 0x2000, CRC(9acffb30) SHA1(b814f10ef23f2ca445fabafcbf7f25e2d454ba8c) )   \
	ROM_LOAD_BIOS( 3, "pch1-c__8m_e-1.8m",  0x02000, 0x2000, CRC(c1232eee) SHA1(beaf9fa2d091a3c7f70c51e966d885b1f9f0935f) )   \
	ROM_LOAD_BIOS( 3, "pch1-c__8p_e-1.8p",  0x04000, 0x2000, CRC(30c15e23) SHA1(69166afdb2fe827c7f1919cdf4197caccbd961fa) )   \
	\
	ROM_REGION( 0x0300, "proms", ROMREGION_INVERT )         \
	ROM_LOAD( "pch1-c-6f.82s129an.6f",    0x0000, 0x0100, CRC(e5414ca3) SHA1(d2878411cda84ffe0afb2e538a67457f51bebffb) )    \
	ROM_LOAD( "pch1-c-6e.82s129an.6e",    0x0100, 0x0100, CRC(a2625c6e) SHA1(a448b47c9289902e26a3d3c4c7d5a7968c385e81) )    \
	ROM_LOAD( "pch1-c-6d.82s129an.6d",    0x0200, 0x0100, CRC(1213ebd4) SHA1(0ad386fc3eab5e53c0288ad1de33639a9e461b7c) )    \
	\
	ROM_REGION( 0xc0, "ppu:palette", 0 )                    \
	ROM_LOAD( "rp2c0x.pal", 0x00, 0xc0, CRC(48de65dc) SHA1(d10acafc8da9ff479c270ec01180cca61efe62f5) )




/******************************************************************************/

/* Standard Games */
ROM_START( pc_smb )     /* Super Mario Bros. */
	BIOS_CPU
	ROM_LOAD( "u3sm",    0x0c000, 0x2000, CRC(4b5f717d) SHA1(c39c90f9503c4692af4a8fdb3e18ef7cf04e897f) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x8000, "prg", 0 )
	ROM_LOAD( "u1sm",    0x0000, 0x8000, CRC(5cf548d3) SHA1(fefa1097449a3a11ebf8c6199e905996c5dc8fbd) )

	ROM_REGION( 0x02000, "gfx2", 0 )    /* cart gfx */
	ROM_LOAD( "u2sm",    0x00000, 0x2000, CRC(867b51ad) SHA1(394badaf0b0bdd0ea279a1bca89a9d9ddc00b1b5) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(bd82d775) SHA1(e15c369d638156eeb0cd141aeeec877c62810b64) )
ROM_END

ROM_START( pc_ebike )   /* Excitebike */
	BIOS_CPU
	ROM_LOAD( "u3eb",    0x0c000, 0x2000, CRC(8ff0e787) SHA1(35a6d7186dee4fd4ba015ec0db5181768411aa3c) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x8000, "prg", 0 )
	ROM_LOAD( "u1eb",    0x0000, 0x4000, CRC(3a94fa0b) SHA1(6239e91ccefdc017d233cbae388c6568a17ed04b) )
	ROM_RELOAD(          0x4000, 0x4000 )

	ROM_REGION( 0x02000, "gfx2", 0 )    /* cart gfx */
	ROM_LOAD( "u2eb",    0x00000, 0x2000, CRC(e5f72401) SHA1(a8bf028e1a62677e48e88cf421bb2a8051eb800c) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(a0263750) SHA1(2ab6e43c2bc4c07fc7887defc4fc81502167ef60) )
ROM_END

ROM_START( pc_1942 )    /* 1942 */
	BIOS_CPU
	ROM_LOAD( "u3",      0x0c000, 0x2000, CRC(415b8807) SHA1(9d6161bbc6dec5873cc6d8a570141d4af42fa232) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x8000, "prg", 0 )
	ROM_LOAD( "u1",      0x0000, 0x8000, CRC(c4e8c04a) SHA1(d608f769333b13da9c67f07599e405944893a950) )

	ROM_REGION( 0x02000, "gfx2", 0 )    /* cart gfx */
	ROM_LOAD( "u2",      0x00000, 0x2000, CRC(03379b76) SHA1(d2a6ca1cdd8935525f59f1d38806b2296cb12a12) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(29893c7f) SHA1(58478b7de2177c8dc1d6885bd34eeeeb5e46d7a3) )
ROM_END

ROM_START( pc_bfght )   /* Balloon Fight */
	BIOS_CPU
	ROM_LOAD( "bf-u3",   0x0c000, 0x2000, CRC(a9949544) SHA1(0bb9fab67769a4eaa1b903a3217dbb5ca6feddb8) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x8000, "prg", 0 )
	ROM_LOAD( "bf-u1",   0x0000, 0x4000, CRC(575ed2fe) SHA1(63527ea590aa79a6b09896c35021de785fd40851) )
	ROM_RELOAD(          0x4000, 0x4000 )

	ROM_REGION( 0x02000, "gfx2", 0 )    /* cart gfx */
	ROM_LOAD( "bf-u2",   0x00000, 0x2000, CRC(c642a1df) SHA1(e73cd3d4c0bad8e6f7a1aa6a580f3817a83756a9) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(be3c42fb) SHA1(da40c57bda36d1dbacdf246e0d2579b6be616480) )
ROM_END

ROM_START( pc_bball )   /* Baseball */
	BIOS_CPU
	ROM_LOAD( "ba-u3",   0x0c000, 0x2000, CRC(06861a0d) SHA1(b7263280a39f544ca4ab1b4d3e8c5fe17ea95e57) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x8000, "prg", 0 )
	ROM_LOAD( "ba-u1",   0x0000, 0x4000, CRC(39d1fa03) SHA1(28d84cfefa81bbfd3d26e0f70f1b9f53383e54ad) )
	ROM_RELOAD(          0x4000, 0x4000 )

	ROM_REGION( 0x02000, "gfx2", 0 )    /* cart gfx */
	ROM_LOAD( "ba-u2",   0x00000, 0x2000, CRC(cde71b82) SHA1(296ccef8a1fd9209f414ce0c788ab0dc95058242) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(7940cfc4) SHA1(9e36ceb5aac023472f48f2f06cf171bffa49a664) )
ROM_END

ROM_START( pc_golf )    /* Golf */
	BIOS_CPU
	ROM_LOAD( "gf-u3",   0x0c000, 0x2000, CRC(882dea87) SHA1(e3bbca36efa66231b933713dec032bbb926b36e5) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x8000, "prg", 0 )
	ROM_LOAD( "gf-u1",   0x0000, 0x4000, CRC(f9622bfa) SHA1(b4e341a91f614bb19c67cc0205b2443591567aea) )
	ROM_RELOAD(          0x4000, 0x4000 )

	ROM_REGION( 0x02000, "gfx2", 0 )    /* cart gfx */
	ROM_LOAD( "gf-u2",   0x00000, 0x2000, CRC(ff6fc790) SHA1(40177839b61f375f2ad03b203328683264845b5b) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(2cd98ef6) SHA1(bd5142c6a29df674ab835c8beafff7e93712d88f) )
ROM_END

ROM_START( pc_kngfu )   /* Kung Fu */
	BIOS_CPU
	ROM_LOAD( "sx-u3",   0x0c000, 0x2000, CRC(ead71b7e) SHA1(e255c08f92d6188dad6b27446b0117cd7cee4364) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x8000, "prg", 0 )
	ROM_LOAD( "sx-u1",   0x0000, 0x8000, CRC(0516375e) SHA1(55dc3550c6133f8624eb6cf3d2f145e4313c2ff6) )

	ROM_REGION( 0x02000, "gfx2", 0 )    /* cart gfx */
	ROM_LOAD( "sx-u2",   0x00000, 0x2000, CRC(430b49a4) SHA1(7e618dbff521c3d5ee0f3d8bb01d2e770395a6bc) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(a1687f01) SHA1(ff4c3e925ece14acfa6f51c87af310ebbe3af638) )
ROM_END

ROM_START( pc_tenis )   /* Tennis */
	BIOS_CPU
	ROM_LOAD( "te-u3",   0x0c000, 0x2000, CRC(6928e920) SHA1(0bdc64a6f37d8cf5e8efacc5004a6ae43a28cd60) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x8000, "prg", 0 )
	ROM_LOAD( "te-u1",   0x0000, 0x4000, CRC(8b2e3e81) SHA1(e54274c0b0d651458c5459d41872b1f99904d0fb) )
	ROM_RELOAD(          0x4000, 0x4000 )

	ROM_REGION( 0x02000, "gfx2", 0 )    /* cart gfx */
	ROM_LOAD( "te-u2",   0x00000, 0x2000, CRC(3a34c45b) SHA1(2cc26a01c38ead50503dccb3ee929ba7a2b6772c) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(bcc9a48e) SHA1(a293898f17b627cdf8e7a1074ef30ad8c2392977) )
ROM_END

ROM_START( pc_vball )   /* Volley Ball */
	BIOS_CPU
	ROM_LOAD( "vb-u3",   0x0c000, 0x2000, CRC(9104354e) SHA1(84374b1df747800f7e70b5fb6a16fd3607b724c9) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x8000, "prg", 0 )
	ROM_LOAD( "vb-u1",   0x0000, 0x8000, CRC(35226b99) SHA1(548787ba5ca00290da4efc9af40054dc1889014c) )

	ROM_REGION( 0x02000, "gfx2", 0 )    /* cart gfx */
	ROM_LOAD( "vb-u2",   0x00000, 0x2000, CRC(2415dce2) SHA1(fd89b4a542989a89c2d0467257dca57518bfa96b) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(f701863f) SHA1(78614e6b8a78384d9aeab439eb8d53a3691dd0a1) )
ROM_END

ROM_START( pc_mario )   /* Mario Bros. */
	BIOS_CPU
	ROM_LOAD( "ma-u3",   0x0c000, 0x2000, CRC(a426c5c0) SHA1(0cf31de3eb18f17830dd9aa3a33fe4a6947f6ceb) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x8000, "prg", 0 )
	ROM_LOAD( "ma-u1",   0x0000, 0x4000, CRC(75f6a9f3) SHA1(b6f88f7a2f9a49cc9182a244571730198f1edc4b) )
	ROM_RELOAD(          0x4000, 0x4000 )

	ROM_REGION( 0x02000, "gfx2", 0 )    /* cart gfx */
	ROM_LOAD( "ma-u2",   0x00000, 0x2000, CRC(10f77435) SHA1(a646c3443832ada84d31a3a8a4b34aebc17cecd5) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(79006635) SHA1(10dcb24fb7717b993110512115ab04310dc637d0) )
ROM_END

/* Gun Games */
ROM_START( pc_duckh )   /* Duck Hunt */
	BIOS_CPU
	ROM_LOAD( "u3",      0x0c000, 0x2000, CRC(2f9ec5c6) SHA1(1e1b835339b030605841a032f066ccb5ca1fef20) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x8000, "prg", 0 )
	ROM_LOAD( "u1",      0x0000, 0x4000, CRC(90ca616d) SHA1(b742576317cd6a04caac25252d5593844c9a0bb6) )
	ROM_RELOAD(          0x4000, 0x4000 )

	ROM_REGION( 0x04000, "gfx2", 0 )    /* cart gfx */
	ROM_LOAD( "u2",      0x00000, 0x2000, CRC(4e049e03) SHA1(ffad32a3bab2fb3826bc554b1b9838e837513576) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(8cd6aad6) SHA1(4543cdb55c3521e1b5d61f82d4800c414658fd6d) )
ROM_END

ROM_START( pc_hgaly )   /* Hogan's Alley */
	BIOS_CPU
	ROM_LOAD( "ha-u3",   0x0c000, 0x2000, CRC(a2525180) SHA1(9c981c1679c59c7b7c069f7d1cb86cb8aa280f22) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x8000, "prg", 0 )
	ROM_LOAD( "ha-u1",   0x0000, 0x4000, CRC(8963ae6e) SHA1(bca489ed0fb58e1e99f36c427bc0d7d805b6c61a) )
	ROM_RELOAD(          0x4000, 0x4000 )

	ROM_REGION( 0x04000, "gfx2", 0 )    /* cart gfx */
	ROM_LOAD( "ha-u2",   0x00000, 0x2000, CRC(5df42fc4) SHA1(4fcf23151d9f11c1ef1b1007dd8058f5d5fe9ab8) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(5ac61521) SHA1(75d2ad958336061e70049272ce4c88bff182f96d) )
ROM_END

ROM_START( pc_wgnmn )   /* Wild Gunman */
	BIOS_CPU
	ROM_LOAD( "wg-u3",   0x0c000, 0x2000, CRC(da08afe5) SHA1(0f505ccee372a37971bad7bbbb7341336ee70f97) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x8000, "prg", 0 )
	ROM_LOAD( "wg-u1",   0x0000, 0x4000, CRC(389960db) SHA1(6b38f2c86ef27f653a2bdb9c682ac0bc981c7db6) )
	ROM_RELOAD(          0x4000, 0x4000 )

	ROM_REGION( 0x04000, "gfx2", 0 )    /* cart gfx */
	ROM_LOAD( "wg-u2",   0x00000, 0x2000, CRC(a5e04856) SHA1(9194d89a34f687742216889cbb3e717a9ae81c92) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(def015a3) SHA1(b542828a74744d87331821635777d7715e22a15b) )
ROM_END

/* A-Board Games */
ROM_START( pc_tkfld )   /* Track & Field */
	BIOS_CPU
	ROM_LOAD( "u4tr",    0x0c000, 0x2000, CRC(70184fd7) SHA1(bc6f6f942948ddf5a7130d9688f12ef5511a7a30) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x8000, "prg", 0 )
	ROM_LOAD( "u2tr",    0x0000, 0x8000, CRC(d7961e01) SHA1(064cb6e3e5525682a1805b01ba64f2fd75462496) )

	ROM_REGION( 0x08000, "gfx2", 0 )    /* cart gfx */
	ROM_LOAD( "u3tr",    0x00000, 0x8000, CRC(03bfbc4b) SHA1(ffc4e0e1d858fb4472423ae1c1fdc1e8197c30f0) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(1e2e7f1e) SHA1(4b65f5b217586653a1d0da96539cc9bc50d989e2) )
ROM_END

ROM_START( pc_grdus )   /* Gradius */
	BIOS_CPU
	ROM_LOAD( "gr-u4",   0x0c000, 0x2000, CRC(27d76160) SHA1(605d58c57969c831778b95356fcf103a1d5f98a3) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x8000, "prg", 0 )
	ROM_LOAD( "gr-u2",   0x0000, 0x8000, CRC(aa96889c) SHA1(e4380a7c0778541af8216e3ac1e14ff23fb074a9) )

	ROM_REGION( 0x08000, "gfx2", 0 )    /* cart gfx */
	ROM_LOAD( "gr-u3",   0x00000, 0x8000, CRC(de963bec) SHA1(ecb76b5897658ebac31a07516bb2a5820279474f) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(b8d5bf8a) SHA1(1c208fa5409b6e21aa576e1b9e086e830dc26a1a) )
ROM_END

ROM_START( pc_grdue )   /* Gradius (Early version) */
	BIOS_CPU
	ROM_LOAD( "gr-u4",   0x0c000, 0x2000, CRC(27d76160) SHA1(605d58c57969c831778b95356fcf103a1d5f98a3) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x8000, "prg", 0 )
	ROM_LOAD( "gr-u1e",  0x0000, 0x8000, CRC(9204a65d) SHA1(500693f8f65b1e2f09b722c5fa28b32088e22a29) )

	ROM_REGION( 0x08000, "gfx2", 0 )    /* cart gfx */
	ROM_LOAD( "gr-u3",   0x00000, 0x8000, CRC(de963bec) SHA1(ecb76b5897658ebac31a07516bb2a5820279474f) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(b8d5bf8a) SHA1(1c208fa5409b6e21aa576e1b9e086e830dc26a1a) )
ROM_END


/* B-Board Games */
ROM_START( pc_rnatk )   /* Rush N' Attack */
	BIOS_CPU
	ROM_LOAD( "ra-u4",   0x0c000, 0x2000, CRC(ebab7f8c) SHA1(ae46e46d878cdbc28cd42b40dae1fd1a6c1b31ed) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x20000, "prg", 0 )
	ROM_LOAD( "ra-u1",   0x00000, 0x10000, CRC(5660b3a6) SHA1(4e7ad9be59990e4a560d87a1bac9b708074e9db1) ) /* banked */
	ROM_LOAD( "ra-u2",   0x10000, 0x10000, CRC(2a1bca39) SHA1(ca1eebf85bea85ce7bcdf38933ae495856e17ae1) ) /* banked */

	/* No cart gfx - uses vram */

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(1f6596b2) SHA1(e50780241ae3a16811bd92eb366f642a4b5eadf3) )
ROM_END

ROM_START( pc_cntra )   /* Contra */
	BIOS_CPU
	ROM_LOAD( "u4ct",    0x0c000, 0x2000, CRC(431486cf) SHA1(8b8a2bcddb1dfa027c249b62659dcc7bb8ec2778) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x20000, "prg", 0 )
	ROM_LOAD( "u1ct",    0x00000, 0x10000, CRC(9fcc91d4) SHA1(ad1742a0da87cf7f26f81a99f185f0c28b9e7e6e) ) /* banked */
	ROM_LOAD( "u2ct",    0x10000, 0x10000, CRC(612ad51d) SHA1(4428e136b55778299bb269520b459c7112c0d6b2) ) /* banked */

	/* No cart gfx - uses vram */

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(8ab3977a) SHA1(61d3a7981fbe8a76ab7eee032059d42b50892e97) )
ROM_END

ROM_START( pc_pwrst )   /* Pro Wrestling */
	BIOS_CPU
	ROM_LOAD( "pw-u4",   0x0c000, 0x2000, CRC(0f03d71b) SHA1(82b94c2e4568d6de4d8cff49f3e416005a2e22ec) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x20000, "prg", 0 )
	ROM_LOAD( "pw-u1",   0x00000, 0x08000, CRC(6242c2ce) SHA1(ea7d1cf9dece021c9a40772af7c6dcaf58b10585) ) /* banked */
	ROM_RELOAD(          0x08000, 0x08000 )
	ROM_LOAD( "pw-u2",   0x10000, 0x10000, CRC(ef6aa17c) SHA1(52171699eaee0b811952c5706584cff4e7cfb39a) ) /* banked */

	/* No cart gfx - uses vram */

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(4c6b7983) SHA1(377bd6267ae1d3ab13389a8adf894e116b3c9daa) )
ROM_END

ROM_START( pc_cvnia )   /* Castlevania */
	BIOS_CPU
	ROM_LOAD( "u4cv",    0x0c000, 0x2000, CRC(a2d4245d) SHA1(3703171d526e6de99e475afe0d942d69b89950a9) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x20000, "prg", 0 )
	ROM_LOAD( "u1cv",    0x00000, 0x10000, CRC(add4fc52) SHA1(bbb4638a8e7660911896393d61580610a6535c62) ) /* banked */
	ROM_LOAD( "u2cv",    0x10000, 0x10000, CRC(7885e567) SHA1(de1e5a5b4bbd0116c91564edc3d552239074e8ae) ) /* banked */

	/* No cart gfx - uses vram */

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(7da2f045) SHA1(e6048a1f94103c2896eeb33dd7f6bc639831dd7d) )
ROM_END

ROM_START( pc_dbldr )   /* Double Dribble */
	BIOS_CPU
	ROM_LOAD( "dw-u4",    0x0c000, 0x2000, CRC(5006eef8) SHA1(6051d4750d95cdc0a71ecec40b5be4477921ca54) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x20000, "prg", 0 )
	ROM_LOAD( "dw-u1",    0x00000, 0x10000, CRC(78e08e61) SHA1(a278e012ac89b8ae56d4a186c99f5ea2591f87b5) ) /* banked */
	ROM_LOAD( "dw-u2",    0x10000, 0x10000, CRC(ab554cde) SHA1(86f5788f856dd9336eaaadf8d5295435b0421486) ) /* banked */

	/* No cart gfx - uses vram */

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(9b5f4bd2) SHA1(998d2766763eb66f4052f9f16fbfb93d5b41a582) )
ROM_END

ROM_START( pc_rygar )   /* Rygar */
	BIOS_CPU
	ROM_LOAD( "ry-u4",    0x0c000, 0x2000, CRC(7149071b) SHA1(fbc7157eb16eedfc8808ab6224406037e41c44ef) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x20000, "prg", 0 )
	ROM_LOAD( "ry-u1",    0x00000, 0x10000, CRC(aa2e54bc) SHA1(b44cd385d4019a535a4924a093ee9b097b850db4) ) /* banked */
	ROM_LOAD( "ry-u2",    0x10000, 0x10000, CRC(80cb158b) SHA1(012f378e0b5a5bbd32ad837cdfa096df6843d274) ) /* banked */

	/* No cart gfx - uses vram */

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(b69309ab) SHA1(a11ae46ed4c6ae5c22bab36593a53535a257fd4f) )
ROM_END

ROM_START( pc_trjan )   /* Trojan */
	BIOS_CPU
	ROM_LOAD( "tj-u4",    0x0c000, 0x2000, CRC(10835e1d) SHA1(ae0f3ec8d52707088af79d00bca0871af105da36) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x20000, "prg", 0 )
	ROM_LOAD( "tj-u1",    0x00000, 0x10000, CRC(335c0e62) SHA1(62164235dc8e2a4419cb38f4cacf7ba2f3eb536b) ) /* banked */
	ROM_LOAD( "tj-u2",    0x10000, 0x10000, CRC(c0ddc79e) SHA1(5c23bb54eda6a55357e97d7322db453170e27598) ) /* banked */

	/* No cart gfx - uses vram */

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(14df772f) SHA1(bb1c58d53ee8c059b3a06d43ee4faf849d4f005f) )
ROM_END

/* C-Board Games */
ROM_START( pc_goons )   /* The Goonies */
	BIOS_CPU
	ROM_LOAD( "gn-u3",   0x0c000, 0x2000, CRC(33adedd2) SHA1(c85151819e2550e60cbe8f7d247a8da88cb805a4) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x8000, "prg", 0 )
	ROM_LOAD( "gn-u1",   0x0000, 0x8000, CRC(efeb0c34) SHA1(8e0374858dce0a10ffcfc5109f8287ebdea388e8) )

	ROM_REGION( 0x04000, "gfx2", 0 )    /* cart gfx */
	ROM_LOAD( "gn-u2",   0x00000, 0x4000, CRC(0f9c7f49) SHA1(f2fcf55d22a38a01df45393c90c73ff14b3b647c) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(cdd62d08) SHA1(e2752127ac0b1217f0216854b68a5e5957a565b3) )
ROM_END

/* D-Board Games */
ROM_START( pc_radrc )   /* Rad Racer */
	BIOS_CPU
	ROM_LOAD( "rc-u5",   0x0c000, 0x2000, CRC(ae60fd08) SHA1(fa7c201499cd702d8eef545bb05b0df833d2b406) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x20000, "prg", 0 )
	ROM_LOAD( "rc-u1",   0x00000, 0x10000, CRC(dce369a7) SHA1(d7f293956d605af7cb6b81dbb80eaa4ad482ac0e) )
	ROM_LOAD( "rc-u2",   0x10000, 0x10000, CRC(389a79b5) SHA1(58de166d757e58c515272efc9d0bc03d1eb1086d) ) /* banked */

	/* No cart gfx - uses vram */

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(17c880f9) SHA1(41be451fcc46a746d5d31dba09f524c0af0cd214) )
ROM_END

ROM_START( pc_mtoid )   /* Metroid */
	BIOS_CPU
	ROM_LOAD( "mt-u5",   0x0c000, 0x2000, CRC(3dc25049) SHA1(bf0f72db9e6904f065801e490014405a734eb04e) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x20000, "prg", 0 )
	ROM_LOAD( "mt-u1",   0x00000, 0x10000, CRC(4006ff10) SHA1(9563a6b4ff91c78ab9cbf97ea47a3f62524844d2) )
	ROM_LOAD( "mt-u2",   0x10000, 0x10000, CRC(ace6bbd8) SHA1(ac9c22bcc33aeee18b4f42a5a628bc5e147b4c29) ) /* banked */

	/* No cart gfx - uses vram */

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(efab54c9) SHA1(1d0122b7c90a364d54bf6eaa37ce439d706a4357) )
ROM_END

/* E-Board Games */
ROM_START( pc_miket )   /* Mike Tyson's Punchout */
	BIOS_CPU
	ROM_LOAD( "u5pt",    0x0c000, 0x2000, CRC(b434e567) SHA1(8e23c580b5556aacbeeb36fe36e778137c780903) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x20000, "prg", 0 )
	ROM_LOAD( "u1pt",    0x00000, 0x20000, CRC(dfd9a2ee) SHA1(484a6793949b8cbbc65e3bcc9188bc63bb17b575) ) /* banked */

	ROM_REGION( 0x20000, "gfx2", 0 )    /* cart gfx */
	ROM_LOAD( "u3pt",    0x00000, 0x20000, CRC(570b48ea) SHA1(33de517b16b61625909d2eb5307c08b337b542c4) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(60f7ea1d) SHA1(fcc04cbd8ed233bb1358fc55800f9bb6c75b195b) )
ROM_END

/* F-Board Games */
ROM_START( pc_ngaid )   /* Ninja Gaiden */
	BIOS_CPU
	ROM_LOAD( "u2ng",    0x0c000, 0x2000, CRC(7505de96) SHA1(a9cbe6d4d2d33aeecb3e041315fbb266c886ebf1) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x20000, "prg", 0 )
	ROM_LOAD( "u4ng",    0x00000, 0x20000, CRC(5f1e7b19) SHA1(ead83487d9be2f1d16c1d0b438a361a06508cd85) )   /* banked */

	ROM_REGION( 0x020000, "gfx2", 0 )   /* cart gfx */
	ROM_LOAD( "u1ng",   0x00000, 0x20000, CRC(eccd2dcb) SHA1(2a319086f7c22b8fe7ca8ab72436a7c8d07b915e) )    /* banked */

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(ec5641d6) SHA1(05f546aec5a9db167688a9abbac922f5ced7f7c5) )
ROM_END

ROM_START( pc_ddrgn )   /* Double Dragon */
	BIOS_CPU
	ROM_LOAD( "wd-u2",   0x0c000, 0x2000, CRC(dfca1578) SHA1(6bc00bb2913edeaecd885fee449b8a9955c509bf) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x20000, "prg", 0 )
	ROM_LOAD( "wd-u4",  0x00000, 0x20000, CRC(05c97f64) SHA1(36913e92943c6bb40521ab13c843691a8db4cbc9) )    /* banked */

	ROM_REGION( 0x020000, "gfx2", 0 )   /* cart gfx */
	ROM_LOAD( "wd-u1",  0x00000, 0x20000, CRC(5ebe0fd0) SHA1(4a948c9784433e051f1015a6b6e985a98b81b80d) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(f9739d62) SHA1(c9574ed8f24ffb7ab5a6bb1b79805fb6dc6e991a) )
ROM_END

ROM_START( pc_drmro )   /* Dr Mario */
	BIOS_CPU
	ROM_LOAD( "vu-u2",   0x0c000, 0x2000, CRC(4b7869ac) SHA1(37afb84d963233ad92cc424fcf992aa76ea0599f) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x8000, "prg", 0 )
	ROM_LOAD( "vu-u4",  0x0000, 0x8000, CRC(cb02a930) SHA1(6622564abc5ce28f523b0da95054d1ea825f7bd5) )    /* banked */

	ROM_REGION( 0x020000, "gfx2", 0 )   /* cart gfx */
	ROM_LOAD( "vu-u1",  0x00000, 0x08000, CRC(064d4ab3) SHA1(bcdc34435bf631422ea2701f00744a3606c6dce8) )
	ROM_RELOAD(         0x08000, 0x08000 )
	ROM_RELOAD(         0x10000, 0x08000 )
	ROM_RELOAD(         0x18000, 0x08000 )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(1b26e58c) SHA1(bd2d81d3cc54966ef154b3487d43ecbc316d6d22) )
ROM_END

ROM_START( pc_virus )   /* Virus (from location test board) */
	BIOS_CPU
	ROM_LOAD( "u2",   0x0c000, 0x2000, CRC(d2764d91) SHA1(393b54148e9250f14d83318aed6686cc04b923e6) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x8000, "prg", 0 )
	ROM_LOAD( "virus 3-12-90.u4",  0x0000, 0x8000, CRC(a5239a77) SHA1(f1e79906bcbee4e0c62036d6ba95385b95daa53f) )    /* banked */

	ROM_REGION( 0x020000, "gfx2", 0 )   /* cart gfx */
	ROM_LOAD( "virus 3-12-90.u1",  0x00000, 0x08000, CRC(d233c2ae) SHA1(0de301894edfc50b26b6e4cf3697a15065035c5e) )
	ROM_RELOAD(         0x08000, 0x08000 )
	ROM_RELOAD(         0x10000, 0x08000 )
	ROM_RELOAD(         0x18000, 0x08000 )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.u6", 0x00, 0x10, CRC(b69309ab) SHA1(a11ae46ed4c6ae5c22bab36593a53535a257fd4f) )
ROM_END

ROM_START( pc_bload )   /* Bases Loaded (from location test board) */
	BIOS_CPU
	ROM_LOAD( "new game 1.u2",   0x0c000, 0x2000, CRC(43879cc5) SHA1(dfde35e255825fffc22b5495c1e3bc1cfad7e9c0) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x40000, "prg", 0 )
	ROM_LOAD( "u3",  0x00000, 0x20000, CRC(14a77a61) SHA1(6283f0dc8e9a2bbcd7ed452aa30cf646a6526837) )    /* banked */
	ROM_LOAD( "bases loaded 9a70 prg-h.u4",  0x20000, 0x20000, CRC(f158f941) SHA1(e58bdcfb62d25348f5c81b2cf8001fc2c9e04eb2) )    /* banked */

	ROM_REGION( 0x020000, "gfx2", 0 )   /* cart gfx */
	ROM_LOAD( "u1",  0x00000, 0x20000, CRC(02ff6ae9) SHA1(ba15b91f917c9e722d1d8b24b5783bd5eac6a4e7) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.u6", 0x00, 0x10, CRC(b69309ab) SHA1(a11ae46ed4c6ae5c22bab36593a53535a257fd4f) )
ROM_END

ROM_START( pc_ftqst )   /* Fester's Quest */
	BIOS_CPU
	ROM_LOAD( "eq-u2",   0x0c000, 0x2000, CRC(85326040) SHA1(866bd15e77d911147b191c13d062cef7ae4dcf62) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x20000, "prg", 0 )
	ROM_LOAD( "eq-u4",  0x00000, 0x20000, CRC(953a3eaf) SHA1(a22c0a64d63036b6b8d147994a3055e1040a5282) )    /* banked */

	ROM_REGION( 0x020000, "gfx2", 0 )   /* cart gfx */
	ROM_LOAD( "eq-u1",  0x00000, 0x20000, CRC(0ca17ab5) SHA1(a8765d6245f64b2d94c454662a24f8d8e277aa5a) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(1c601cd7) SHA1(bc13067475aac4a7b8bf5f0df96bdd5ba33f1cd7) )
ROM_END

ROM_START( pc_rcpam )   /* RC Pro Am */
	BIOS_CPU
	ROM_LOAD( "pm-u2",   0x0c000, 0x2000, CRC(358c2de7) SHA1(0f37d7e8303a7b87ad0584c6e0a79f3029c529f8) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x8000, "prg", 0 )
	ROM_LOAD( "pm-u4",  0x0000, 0x8000, CRC(82cfde25) SHA1(4eb9abe896e597f8ecabb4f044d8c4b545a51b11) )    /* banked */

	ROM_REGION( 0x020000, "gfx2", 0 )   /* cart gfx */
	ROM_LOAD( "pm-u1",  0x00000, 0x08000, CRC(83c90d47) SHA1(26917e1e016d2be0fa48d766d332779aae12b053) )
	ROM_RELOAD(         0x08000, 0x08000 )
	ROM_RELOAD(         0x10000, 0x08000 )
	ROM_RELOAD(         0x18000, 0x08000 )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(d71d8085) SHA1(67e30ff0c31c6600890408c4dc4d0d2f19856363) )
ROM_END

ROM_START( pc_rrngr )   /* Rescue Rangers */
	BIOS_CPU
	ROM_LOAD( "ru-u2",   0x0c000, 0x2000, CRC(2a4bfc4b) SHA1(87f58659d43a236af22682df4bd01593b69c9975) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x20000, "prg", 0 )
	ROM_LOAD( "ru-u4",  0x00000, 0x20000, CRC(02931525) SHA1(28ddca5d299e7894e3c3aa0a193684ca3e384ee9) )    /* banked */

	ROM_REGION( 0x020000, "gfx2", 0 )   /* cart gfx */
	ROM_LOAD( "ru-u1",  0x00000, 0x20000, CRC(218d4224) SHA1(37a729021173bec08a8497ad03fd58379b0fce39) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(1c2e1865) SHA1(ab2aa76d74c9e76c7ee3f9a211b1aefe5708a23f) )
ROM_END

ROM_START( pc_ynoid )   /* Yo! Noid */
	BIOS_CPU
	ROM_LOAD( "yc-u2",   0x0c000, 0x2000, CRC(0449805c) SHA1(3f96687eae047d1f8095fbb55c0659c9b0e10166) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x20000, "prg", 0 )
	ROM_LOAD( "yc-u4",  0x00000, 0x20000, CRC(4affeee7) SHA1(54da2aa7ca56d9b593c8bcabf0bb1d701439013d) )    /* banked */

	ROM_REGION( 0x020000, "gfx2", 0 )   /* cart gfx */
	ROM_LOAD( "yc-u1",  0x00000, 0x20000, CRC(868f7343) SHA1(acb6f6eb9e8beb0636c59a999c8f5920ef7786a3) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(8c376465) SHA1(39b06fd2ecd5f06b90b2fe06406c9155f5601bd8) )
ROM_END

ROM_START( pc_tmnt )    /* Teenage Mutant Ninja Turtles */
	BIOS_CPU
	ROM_LOAD( "u2u2",   0x0c000, 0x2000, CRC(bdce58c0) SHA1(abaf89c0ac55cce816a7c6542a868ab47e02d550) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x20000, "prg", 0 )
	ROM_LOAD( "u4u2",   0x00000, 0x20000, CRC(0ccd28d5) SHA1(05606cafba838eeb36198b5e5e9d11c3729971b3) )    /* banked */

	ROM_REGION( 0x020000, "gfx2", 0 )   /* cart gfx */
	ROM_LOAD( "u1u2",   0x00000, 0x20000, CRC(91f01f53) SHA1(171ed0792f3ca3f195145000d96b91aa57898773) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(f5a38e98) SHA1(26ef27294fc14d84920132023fbcf406d89ce2ee) )
ROM_END

ROM_START( pc_bstar )   /* Baseball Stars */
	BIOS_CPU
	ROM_LOAD( "b9-u2",   0x0c000, 0x2000, CRC(69f3fd7c) SHA1(1cfaa40f18b1455bb41ec0e57d6a227ed3e582eb) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x20000, "prg", 0 )
	ROM_LOAD( "b9-u4",   0x00000, 0x20000, CRC(d007231a) SHA1(60690eaeacb79dbcab7dfe1c1e40da1aac235793) )   /* banked */

	ROM_REGION( 0x020000, "gfx2", 0 )   /* cart gfx */
	ROM_LOAD( "b9-u1",   0x00000, 0x20000, CRC(ce149864) SHA1(00c88525756a360f42b27f0e2afaa0a19c2645a6) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(3e871350) SHA1(b338f9ef4e18d14843c6a1e8ecb974bca1df73d4) )
ROM_END

ROM_START( pc_tbowl )   /* Tecmo Bowl */
	BIOS_CPU
	ROM_LOAD( "tw-u2",   0x0c000, 0x2000, CRC(162aa313) SHA1(d0849ce87969c077fc14790ce5658e9857035413) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x20000, "prg", 0 )
	ROM_LOAD( "tw-u4",   0x00000, 0x20000, CRC(4f0c69be) SHA1(c0b09dc81070b935b3c621b07deb62dfa521a396) )   /* banked */

	ROM_REGION( 0x020000, "gfx2", 0 )   /* cart gfx */
	ROM_LOAD( "tw-u1",   0x00000, 0x20000, CRC(44b078ef) SHA1(ae0c24f4ddd822b19c60e31257279b33b5f3fcad) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(18b2d1d3) SHA1(f4d023531b3d69cad4c9c511878e5aa6afb0ac59) )
ROM_END

/* G-Board Games */
ROM_START( pc_smb3 )    /* Super Mario Bros 3 */
	BIOS_CPU
	ROM_LOAD( "u3um",    0x0c000, 0x2000, CRC(45e92f7f) SHA1(9071d5f18639ac58d6d4d72674856f9ecab911f0) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x40000, "prg", 0 )
	ROM_LOAD( "u4um",    0x00000, 0x20000, CRC(590b4d7c) SHA1(ac45940b71215a3a48983e22e1c7e71a71642b91) )   /* banked */
	ROM_LOAD( "u5um",    0x20000, 0x20000, CRC(bce25425) SHA1(69468643a3a8b9220d675e2cdc4245ada81a492c) )   /* banked */

	ROM_REGION( 0x020000, "gfx2", 0 )   /* cart gfx */
	ROM_LOAD( "u1um",    0x00000, 0x20000, CRC(c2928c49) SHA1(2697d1f21b72a6d8e7d2a2d2c51c9c5550f68b56) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(e48f4945) SHA1(66fe537cfe540317d6194847321ce4a9bdf0bba4) )
ROM_END

ROM_START( pc_gntlt )   // Gauntlet
	BIOS_CPU
	ROM_LOAD( "u3gl",    0x0c000, 0x2000, CRC(57575b92) SHA1(7ac633f253496f353d388bef30e6ec74a3d18814) ) // extra bios code for this game
	BIOS_GFX

	ROM_REGION( 0x20000, "prg", 0 )
	ROM_LOAD( "nes-gl-0 prg",0x00000, 0x20000, CRC(834d1924) SHA1(e1d8553a0deaf3bb17c9ea798ec52f2723db3aea) )   // banked

	ROM_REGION( 0x010000, "gfx2", 0 )   // cart gfx
	ROM_LOAD( "nes-gl-0 chr", 0x00000, 0x10000, CRC(26d819a2) SHA1(1e5eec5beb976f79373e589186382c337ed6e84a) )

	ROM_REGION( 0x10, "rp5h01", 0 ) // rp5h01 data
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(ba7f2e13) SHA1(8b9ee3b18bcb4b258a46d1c900b18a9cb2594046) )
ROM_END

ROM_START( pc_pwbld )   /* Power Blade */
	BIOS_CPU
	ROM_LOAD( "7t-u3",    0x0c000, 0x2000, CRC(edcc21c6) SHA1(5d73c6a747cfe951dc7c6ddfbb29859e9548aded) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x20000, "prg", 0 )
	ROM_LOAD( "7t-u5",   0x00000, 0x20000, CRC(faa957b1) SHA1(612c4823ed588652a78017096a6d76dd8064807a) )   /* banked */

	ROM_REGION( 0x020000, "gfx2", 0 )   /* cart gfx */
	ROM_LOAD( "7t-u1",    0x00000, 0x20000, CRC(344be4a6) SHA1(2894292544f4315df44cda1bdc96047453da03e8) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(31a05a48) SHA1(8b340600feae03bb5cdab852a9879ecffcc8a2b9) )
ROM_END

ROM_START( pc_ngai3 )   /* Ninja Gaiden 3 */
	BIOS_CPU
	ROM_LOAD( "u33n",    0x0c000, 0x2000, CRC(c7ba0f59) SHA1(a4822035a10a2b5de3517b461dd357b2fa5da917) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x20000, "prg", 0 )
	ROM_LOAD( "u53n",    0x00000, 0x20000, CRC(f0c77dcb) SHA1(bda1184e27f3c3e92e58519508dd281b06c70d9b) )   /* banked */

	ROM_REGION( 0x020000, "gfx2", 0 )   /* cart gfx */
	ROM_LOAD( "u13n",    0x00000, 0x20000, CRC(584bcf5d) SHA1(f4582e2a382c8424f839e848e95e88a7f46307dc) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(13755943) SHA1(b7d809b0f60ef489777ccb35868f5c1e777356e0) )
ROM_END

// this is identcal to the USA NES release with the generic 'New Game 2' menu rom.
// TT-CHR.U1           = nes-ni-0 chr          nes:ttoonadvu Tiny Toon Adventures (USA)
// TT-GM2.U3           = u2                    pc_virus   Virus (Dr. Mario prototype, PlayChoice-10)
// TT-PRG.U5           = nes-ni-0 prg          nes:ttoonadvu Tiny Toon Adventures (USA)
ROM_START( pc_ttoon )   /* Tiny Toon Adventures */
	BIOS_CPU
	ROM_LOAD( "tt-gm2.u3",   0x0c000, 0x2000, CRC(d2764d91) SHA1(393b54148e9250f14d83318aed6686cc04b923e6) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x20000, "prg", 0 )
	ROM_LOAD( "tt-prg.u5",    0x00000, 0x20000, CRC(9cb55b96) SHA1(437c326a4575895b9d7e567cab4f70b2f44ed8dd) )   /* banked */

	ROM_REGION( 0x020000, "gfx2", 0 )   /* cart gfx */
	ROM_LOAD( "tt-chr.u1",    0x00000, 0x20000, CRC(a024ae14) SHA1(2e797a173161a61c14ce299e3c5a31c6029f2b50) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.u6", 0x00, 0x10, CRC(b69309ab) SHA1(a11ae46ed4c6ae5c22bab36593a53535a257fd4f) )
ROM_END


ROM_START( pc_radr2 )   /* Rad Racer II */
	BIOS_CPU
	ROM_LOAD( "qr-u3",    0x0c000, 0x2000, CRC(0c8fea63) SHA1(7ac04b151df732bd16708655352b7f13926f004f) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x10000, "prg", 0 )
	ROM_LOAD( "qr-u5",    0x00000, 0x10000, CRC(ab90e397) SHA1(0956f7d9a216549dbd80b1dbf2653b36a320d0ab) )  /* banked */

	ROM_REGION( 0x010000, "gfx2", 0 )   /* cart gfx */
	ROM_LOAD( "qr-u1",    0x00000, 0x10000, CRC(07df55d8) SHA1(dd0fa0a79d30eb04917d7309a62adfb037ef9ca5) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(40c4f294) SHA1(3fcac63fe6f147b662d59d25f905f797a1f5d0db) )
ROM_END

ROM_START( pc_rkats )   /* Rockin' Kats */
	BIOS_CPU
	ROM_LOAD( "7a-u3",    0x0c000, 0x2000, CRC(352b1e3c) SHA1(bb72b586ec4b482aef462b017de5662d83631df1) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x20000, "prg", 0 )
	ROM_LOAD( "7a-u5",    0x00000, 0x20000, CRC(319ccfcc) SHA1(06e1c34af917b84a990db895c7b44df1b3393c96) )  /* banked */

	ROM_REGION( 0x020000, "gfx2", 0 )   /* cart gfx */
	ROM_LOAD( "7a-u1",    0x00000, 0x20000, CRC(487aa440) SHA1(ee7ebbcf89c81ba59beda1bd27289dae21bb8071) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(56ab5bf9) SHA1(9546f6e20fdb13146c5db5353a1cb2a95931d909) )
ROM_END

ROM_START( pc_suprc )   /* Super C */
	BIOS_CPU
	ROM_LOAD( "ue-u3",    0x0c000, 0x2000, CRC(a30ca248) SHA1(19feb1b4f749768773e0d24777d7e60b2b6260e2) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x20000, "prg", 0 )
	ROM_LOAD( "ue-u5",    0x00000, 0x20000, CRC(c7fbecc3) SHA1(2653456c91031dfa73a50cab3835068a7bface8d) )  /* banked */

	ROM_REGION( 0x020000, "gfx2", 0 )   /* cart gfx */
	ROM_LOAD( "ue-u1",    0x00000, 0x20000, CRC(153295c1) SHA1(4ff1caaedca52fb9bb0ca6c8fac24edda77308d7) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(d477095e) SHA1(a179dffe529889f8e17e9f04958fea28611df0d3) )
ROM_END

ROM_START( pc_tmnt2 )   /* Teenage Mutant Ninja Turtles II */
	BIOS_CPU
	ROM_LOAD( "2n-u3",    0x0c000, 0x2000, CRC(65298370) SHA1(fd120f43e465a2622f2e2679ace2fb0fe7e709b1) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x40000, "prg", 0 )
	ROM_LOAD( "2n-u5",    0x00000, 0x40000, CRC(717e1c46) SHA1(b49cc88e026dac7f5ba96f5c16bcb897addbe259) )  /* banked */

	ROM_REGION( 0x040000, "gfx2", 0 )   /* cart gfx */
	ROM_LOAD( "2n-u1",    0x00000, 0x40000, CRC(0dbc575f) SHA1(8094278cf3267757953ab761dbccf38589142376) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(237e8519) SHA1(81b368d0784e4172c5cf9f4f4b92e29e05d34ae7) )
ROM_END

ROM_START( pc_wcup )    /* Nintendo World Cup */
	BIOS_CPU
	ROM_LOAD( "xz-u3",    0x0c000, 0x2000, CRC(c26cb22f) SHA1(18fea97b498812915bbd53a20b4f0a2130de6faf) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x20000, "prg", 0 )
	ROM_LOAD( "xz-u5",    0x00000, 0x20000, CRC(314ee295) SHA1(0a5963feb5a6b47f0e7bea5bdd3d5835300af7b6) )  /* banked */

	ROM_REGION( 0x020000, "gfx2", 0 )   /* cart gfx */
	ROM_LOAD( "xz-u1",    0x00000, 0x20000, CRC(92477d53) SHA1(33225bd5ee72f92761fdce931c93dd54e6885bd4) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(e17e1d76) SHA1(3e4e1ddcc8524bf451cb568b1357ec1f0a8be44c) )
ROM_END

ROM_START( pc_mman3 )   /* Mega Man 3 */
	BIOS_CPU
	ROM_LOAD( "xu-u3",   0x0c000, 0x2000, CRC(c3984e09) SHA1(70d7e5d9cf9b1f358e1be84a0e8c5997b1aae2d9) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x40000, "prg", 0 )
	ROM_LOAD( "xu-u4",   0x00000, 0x20000, CRC(98a3263c) SHA1(02c8d8301fb220c3f4fd82bdc8cd2388b975fd05) )   /* banked */
	ROM_LOAD( "xu-u5",   0x20000, 0x20000, CRC(d365647a) SHA1(4f39de6249c5f8b7cfa34bc955fd7ea6251569b5) )   /* banked */

	ROM_REGION( 0x020000, "gfx2", 0 )   /* cart gfx */
	ROM_LOAD( "xu-u1",    0x00000, 0x20000, CRC(4028916e) SHA1(f986f72ba5284129620d31c0779ac6d50638e6f1) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(0fe6e900) SHA1(544d8af1aa9186bf76d0a35e78b20e94d3afbcb5) )
ROM_END

ROM_START( pc_smb2 )    /* Super Mario Bros 2 */
	BIOS_CPU
	ROM_LOAD( "mw-u3",   0x0c000, 0x2000, CRC(beaeb43a) SHA1(c7dd186d6167e39924a000eb80bd33beedb2b8c8) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x20000, "prg", 0 )
	ROM_LOAD( "mw-u5",   0x00000, 0x20000, CRC(07854b3f) SHA1(9bea58ba97730c84232a4acbb23c3ea7bce14ec5) )   /* banked */

	ROM_REGION( 0x020000, "gfx2", 0 )   /* cart gfx */
	ROM_LOAD( "mw-u1",    0x00000, 0x20000, CRC(f2ba1170) SHA1(d9976b677ad222b76fbdaf31713374e2f283d44e) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(372f4e84) SHA1(cdf221d49f3b454997d696f213d60b5dce0ce9fb) )
ROM_END

ROM_START( pc_ngai2 )   /* Ninja Gaiden 2 */
	BIOS_CPU
	ROM_LOAD( "nw-u3",   0x0c000, 0x2000, CRC(bc178cde) SHA1(2613f501f92d358f0085aa7002c752cb9a8521ca) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x20000, "prg", 0 )
	ROM_LOAD( "nw-u5",   0x00000, 0x20000, CRC(c43da8e2) SHA1(702a4cf2f57fff7183f2d3c18b8997a38cadc6cd) )   /* banked */

	ROM_REGION( 0x020000, "gfx2", 0 )   /* cart gfx */
	ROM_LOAD( "nw-u1",    0x00000, 0x20000, CRC(8e0c8bb0) SHA1(6afe24b8e57f5a2174000a706b66209d7e310ed6) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(682dffd1) SHA1(87ea54b3d725a552b397ccb2af0ccf8bd6452a7c) )
ROM_END

/* H-Board Games */
ROM_START( pc_pinbt )   /* PinBot */
	BIOS_CPU
	ROM_LOAD( "io-u3",   0x0c000, 0x2000, CRC(15ba8a2e) SHA1(e64180b2f12189e3ac1e155f3544f28af8003f97) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x20000, "prg", 0 )
	ROM_LOAD( "io-u5",   0x00000, 0x20000, CRC(9f75b83b) SHA1(703e41d4c1a4716b324dece6df2ce12a847f082c) )   /* banked */

	ROM_REGION( 0x010000, "gfx2", 0 )   /* cart gfx */
	ROM_LOAD( "io-u1",    0x00000, 0x10000, CRC(9089fc24) SHA1(0bc92a0853c5ebc47c3adbc4e919ea41a55297d0) )

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(ac75f323) SHA1(4bffff024132d6f71d6aa55e24af400d2915aca4) )
ROM_END

/* i-Board Games */
ROM_START( pc_cshwk )   /* Captain Sky Hawk */
	BIOS_CPU
	ROM_LOAD( "yw-u3",   0x0c000, 0x2000, CRC(9d988209) SHA1(b355911d31dfc611b9e90cca82fc10035483b89c) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x20000, "prg", 0 )
	ROM_LOAD( "yw-u1",   0x00000, 0x20000, CRC(a5e0208a) SHA1(e12086a3f1a3b5e9ec035cb778505e43f501416a) ) /* banked */

	/* No cart gfx - uses vram */

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(eb1c794f) SHA1(d32f841fd4306389d716229da9bffea909186689) )
ROM_END

ROM_START( pc_sjetm )   /* Solar Jetman */
	BIOS_CPU
	ROM_LOAD( "lj-u3",   0x0c000, 0x2000, CRC(273d8e75) SHA1(b13b97545b39f6b0459440fb6594ebe03366dfc9) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x40000, "prg", 0 )
	ROM_LOAD( "lj-u1",   0x00000, 0x40000, CRC(8111ba08) SHA1(caa4d1ab710bd766f8505ef24f5702dac6e988af) ) /* banked */

	/* No cart gfx - uses vram */

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(f3ae712a) SHA1(51f443c65e64f1a9eb565ce017b50ec9bd4a5520) )
ROM_END


/* K-Board Games */
ROM_START( pc_moglf )   /* Mario Open Golf */
	BIOS_CPU
	ROM_LOAD( "ug-u2",   0x0c000, 0x2000, CRC(e932fe2b) SHA1(563380482525fdadd05fced2af61d5198d1654a5) ) /* extra bios code for this game */
	BIOS_GFX

	ROM_REGION( 0x40000, "prg", 0 )
	ROM_LOAD( "ug-u4",   0x00000, 0x40000, CRC(091a6a4c) SHA1(2d5ac7c65ce63d409b6e0b2e2185d81bc7c57c69) ) /* banked */

	/* No cart gfx - uses vram */

	ROM_REGION( 0x10, "rp5h01", 0 ) /* rp5h01 data */
	ROM_LOAD( "security.prm", 0x00, 0x10, CRC(633766d5) SHA1(3a2564f3a2daf3a789e4c4056822f12243c89619) )
ROM_END

/***************************************************************************

  BIOS driver(s)

***************************************************************************/

ROM_START( playch10 )
	BIOS_CPU
	BIOS_GFX
	ROM_REGION( 0x10000, "prg", ROMREGION_ERASE00 )
ROM_END

} // anonymous namespace

/******************************************************************************/


/* A dummy driver, so that the bios can be debugged, and to serve as */
/* parent for the other drivers, so that we do not have to include */
/* them in every zip file */
GAME( 1986, playch10, 0, playch10, playch10, playch10_state, init_playch10, ROT0, "Nintendo of America", "PlayChoice-10 BIOS", MACHINE_IS_BIOS_ROOT )

//******************************************************************************


//    YEAR  NAME      PARENT    MACHINE     INPUT     STATE           INIT           MONITOR

// Standard Games
GAME( 1983, pc_tenis, playch10, playch10,   playch10, playch10_state, init_pc_hrz,   ROT0, "Nintendo",                                 "Tennis (PlayChoice-10)", 0 )
GAME( 1983, pc_mario, playch10, playch10,   playch10, playch10_state, init_pc_hrz,   ROT0, "Nintendo",                                 "Mario Bros. (PlayChoice-10)", 0 )
GAME( 1984, pc_bball, playch10, playch10,   playch10, playch10_state, init_pc_hrz,   ROT0, "Nintendo of America",                      "Baseball (PlayChoice-10)", 0 )
GAME( 1984, pc_bfght, playch10, playch10,   playch10, playch10_state, init_pc_hrz,   ROT0, "Nintendo",                                 "Balloon Fight (PlayChoice-10)", 0 )
GAME( 1984, pc_ebike, playch10, playch10,   playch10, playch10_state, init_playch10, ROT0, "Nintendo",                                 "Excitebike (PlayChoice-10)", MACHINE_IMPERFECT_GRAPHICS ) // scanline in middle of screen scrolls when it should not
GAME( 1984, pc_golf,  playch10, playch10,   playch10, playch10_state, init_pc_hrz,   ROT0, "Nintendo",                                 "Golf (PlayChoice-10)", 0 )
GAME( 1985, pc_kngfu, playch10, playch10,   playch10, playch10_state, init_playch10, ROT0, "Irem (Nintendo license)",                  "Kung Fu (PlayChoice-10)", 0 )
GAME( 1985, pc_smb,   playch10, playch10,   playch10, playch10_state, init_playch10, ROT0, "Nintendo",                                 "Super Mario Bros. (PlayChoice-10)", 0 )
GAME( 1986, pc_vball, playch10, playch10,   playch10, playch10_state, init_playch10, ROT0, "Nintendo",                                 "Volley Ball (PlayChoice-10)", 0 )
GAME( 1987, pc_1942,  playch10, playch10,   playch10, playch10_state, init_pc_hrz,   ROT0, "Capcom",                                   "1942 (PlayChoice-10)", 0 )

// Gun Games
GAME( 1984, pc_duckh, playch10, playch10,   playc10g, playch10_state, init_pc_gun,   ROT0, "Nintendo",                                 "Duck Hunt (PlayChoice-10)", 0 )
GAME( 1984, pc_hgaly, playch10, playch10,   playc10g, playch10_state, init_pc_gun,   ROT0, "Nintendo",                                 "Hogan's Alley (PlayChoice-10)", 0 )
GAME( 1984, pc_wgnmn, playch10, playch10,   playc10g, playch10_state, init_pc_gun,   ROT0, "Nintendo",                                 "Wild Gunman (PlayChoice-10)", 0 )

// A-Board Games
GAME( 1986, pc_grdus, playch10, playch10_a, playch10, playch10_state, init_pcaboard, ROT0, "Konami",                                   "Gradius (PlayChoice-10)" , 0) // date: 860917
GAME( 1986, pc_grdue, pc_grdus, playch10_a, playch10, playch10_state, init_pcaboard, ROT0, "Konami",                                   "Gradius (PlayChoice-10, older)" , 0) // date: 860219
GAME( 1987, pc_tkfld, playch10, playch10_a, playch10, playch10_state, init_pcaboard, ROT0, "Konami (Nintendo of America license)",     "Track & Field (PlayChoice-10)", 0 )

// B-Board Games
GAME( 1986, pc_pwrst, playch10, playch10_b, playch10, playch10_state, init_pcbboard, ROT0, "Nintendo",                                 "Pro Wrestling (PlayChoice-10)", 0 )
GAME( 1986, pc_trjan, playch10, playch10_b, playch10, playch10_state, init_pcbboard, ROT0, "Capcom USA (Nintendo of America license)", "Trojan (PlayChoice-10)", 0 )
GAME( 1987, pc_cvnia, playch10, playch10_b, playch10, playch10_state, init_pcbboard, ROT0, "Konami (Nintendo of America license)",     "Castlevania (PlayChoice-10)", 0 )
GAME( 1987, pc_dbldr, playch10, playch10_b, playch10, playch10_state, init_pcbboard, ROT0, "Konami (Nintendo of America license)",     "Double Dribble (PlayChoice-10)", 0 )
GAME( 1987, pc_rnatk, playch10, playch10_b, playch10, playch10_state, init_pcbboard, ROT0, "Konami (Nintendo of America license)",     "Rush'n Attack (PlayChoice-10)", 0 )
GAME( 1987, pc_rygar, playch10, playch10_b, playch10, playch10_state, init_pcbboard, ROT0, "Tecmo (Nintendo of America license)",      "Rygar (PlayChoice-10)", 0 )
GAME( 1988, pc_cntra, playch10, playch10_b, playch10, playch10_state, init_pcbboard, ROT0, "Konami (Nintendo of America license)",     "Contra (PlayChoice-10)", 0 )

// C-Board Games
GAME( 1986, pc_goons, playch10, playch10_c, playch10, playch10_state, init_pccboard, ROT0, "Konami",                                   "The Goonies (PlayChoice-10)", 0 )

// D-Board Games
GAME( 1986, pc_mtoid, playch10, playch10_d2,playch10, playch10_state, init_pcdboard, ROT0, "Nintendo",                                 "Metroid (PlayChoice-10)", 0 )
GAME( 1987, pc_radrc, playch10, playch10_d, playch10, playch10_state, init_pcdboard, ROT0, "Square",                                   "Rad Racer (PlayChoice-10)", MACHINE_IMPERFECT_GRAPHICS )

// E-Board Games
GAME( 1987, pc_miket, playch10, playch10_e, playch10, playch10_state, init_pceboard, ROT0, "Nintendo",                                 "Mike Tyson's Punch-Out!! (PlayChoice-10)", MACHINE_IMPERFECT_GRAPHICS )

// F-Board Games
GAME( 1987, pc_rcpam, playch10, playch10_f, playch10, playch10_state, init_pcfboard, ROT0, "Rare",                                     "R.C. Pro-Am (PlayChoice-10)", 0 )
GAME( 1988, pc_ddrgn, playch10, playch10_f, playch10, playch10_state, init_pcfboard, ROT0, "Technos Japan",                            "Double Dragon (PlayChoice-10)", 0 )
GAME( 1989, pc_ngaid, playch10, playch10_f, playch10, playch10_state, init_pcfboard, ROT0, "Tecmo (Nintendo of America license)",      "Ninja Gaiden (PlayChoice-10)", 0 )
GAME( 1989, pc_tmnt,  playch10, playch10_f, playch10, playch10_state, init_pcfboard, ROT0, "Konami (Nintendo of America license)",     "Teenage Mutant Ninja Turtles (PlayChoice-10)", 0 )
GAME( 1989, pc_ftqst, playch10, playch10_f, playch10, playch10_state, init_pcfboard, ROT0, "Sunsoft (Nintendo of America license)",    "Uncle Fester's Quest: The Addams Family (PlayChoice-10)", 0 )
GAME( 1989, pc_bstar, playch10, playch10_f2,playch10, playch10_state, init_pcfboard, ROT0, "SNK (Nintendo of America license)",        "Baseball Stars: Be a Champ! (PlayChoice-10)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1989, pc_tbowl, playch10, playch10_f, playch10, playch10_state, init_pcfboard, ROT0, "Tecmo (Nintendo of America license)",      "Tecmo Bowl (PlayChoice-10)", 0 )
GAME( 1990, pc_virus, pc_drmro, playch10_f, playch10, playch10_state, init_pcfboard, ROT0, "Nintendo",                                 "Virus (Dr. Mario prototype, PlayChoice-10)", 0 )
GAME( 1990, pc_rrngr, playch10, playch10_f, playch10, playch10_state, init_pcfboard, ROT0, "Capcom USA (Nintendo of America license)", "Chip'n Dale: Rescue Rangers (PlayChoice-10)", 0 )
GAME( 1990, pc_drmro, playch10, playch10_f, playch10, playch10_state, init_pcfboard, ROT0, "Nintendo",                                 "Dr. Mario (PlayChoice-10)", 0 )
GAME( 1990, pc_bload, playch10, playch10_f, playch10, playch10_state, init_pcfboard, ROT0, "Jaleco (Nintendo of America license)",     "Bases Loaded (Prototype, PlayChoice-10)", 0 )
GAME( 1990, pc_ynoid, playch10, playch10_f, playch10, playch10_state, init_pcfboard, ROT0, "Capcom USA (Nintendo of America license)", "Yo! Noid (PlayChoice-10)", 0 )

// G-Board Games
GAME( 1988, pc_smb2,  playch10, playch10_g, playch10, playch10_state, init_pcgboard, ROT0, "Nintendo",                                 "Super Mario Bros. 2 (PlayChoice-10)", 0 )
GAME( 1988, pc_smb3,  playch10, playch10_g, playch10, playch10_state, init_pcgboard, ROT0, "Nintendo",                                 "Super Mario Bros. 3 (PlayChoice-10)", 0 )
GAME( 1990, pc_mman3, playch10, playch10_g, playch10, playch10_state, init_pcgboard, ROT0, "Capcom USA (Nintendo of America license)", "Mega Man III (PlayChoice-10)", 0 )
GAME( 1990, pc_suprc, playch10, playch10_g, playch10, playch10_state, init_pcgboard, ROT0, "Konami (Nintendo of America license)",     "Super C (PlayChoice-10)", 0 )
GAME( 1990, pc_tmnt2, playch10, playch10_g, playch10, playch10_state, init_pcgboard, ROT0, "Konami (Nintendo of America license)",     "Teenage Mutant Ninja Turtles II: The Arcade Game (PlayChoice-10)", 0 )
GAME( 1990, pc_wcup,  playch10, playch10_g, playch10, playch10_state, init_pcgboard, ROT0, "Technos Japan (Nintendo license)",         "Nintendo World Cup (PlayChoice-10)", 0 )
GAME( 1990, pc_ngai2, playch10, playch10_g, playch10, playch10_state, init_pcgboard, ROT0, "Tecmo (Nintendo of America license)",      "Ninja Gaiden Episode II: The Dark Sword of Chaos (PlayChoice-10)", MACHINE_IMPERFECT_GRAPHICS ) // level 2 BG graphics are a total mess
GAME( 1991, pc_ngai3, playch10, playch10_g, playch10, playch10_state, init_pcgboard, ROT0, "Tecmo (Nintendo of America license)",      "Ninja Gaiden Episode III: The Ancient Ship of Doom (PlayChoice-10)", 0 )
GAME( 1991, pc_pwbld, playch10, playch10_g, playch10, playch10_state, init_pcgboard, ROT0, "Taito (Nintendo of America license)",      "Power Blade (PlayChoice-10)", 0 )
GAME( 1991, pc_rkats, playch10, playch10_g, playch10, playch10_state, init_pcgboard, ROT0, "Atlus (Nintendo of America license)",      "Rockin' Kats (PlayChoice-10)", 0 )
GAME( 1991, pc_ttoon, playch10, playch10_g, playch10, playch10_state, init_pcgboard, ROT0, "Konami (Nintendo of America license)",     "Tiny Toon Adventures (prototype) (PlayChoice-10)", 0 ) // Code is final USA NES version of the game, (which is MMC3C according to nes.xml, but this cart has MMC3B)

// variant with 4 screen mirror
GAME( 1990, pc_radr2, playch10, playch10_g, playch10, playch10_state, init_pcgboard_type2, ROT0, "Square (Nintendo of America license)", "Rad Racer II (PlayChoice-10)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1985, pc_gntlt, playch10, playch10_g, playch10, playch10_state, init_pcgboard_type2, ROT0, "Atari / Tengen (Nintendo of America license)", "Gauntlet (PlayChoice-10)", 0 )

// H-Board Games
GAME( 1988, pc_pinbt, playch10, playch10_h, playch10, playch10_state, init_pchboard, ROT0, "Rare (Nintendo of America license)",       "PinBot (PlayChoice-10)", MACHINE_IMPERFECT_GRAPHICS )

// i-Board Games
GAME( 1989, pc_cshwk, playch10, playch10_i, playch10, playch10_state, init_pciboard, ROT0, "Rare (Nintendo of America license)",       "Captain Sky Hawk (PlayChoice-10)", MACHINE_IMPERFECT_GRAPHICS ) // severe graphics glitches on second level
GAME( 1990, pc_sjetm, playch10, playch10_i, playch10, playch10_state, init_pciboard, ROT0, "Rare",                                     "Solar Jetman (PlayChoice-10)", MACHINE_IMPERFECT_GRAPHICS )

// K-Board Games
GAME( 1991, pc_moglf, playch10, playch10_k, playch10, playch10_state, init_pckboard, ROT0, "Nintendo",                                 "Mario's Open Golf (PlayChoice-10)", 0 )
