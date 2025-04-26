// license:BSD-3-Clause
// copyright-holders:Luca Elia, Olivier Galibert
/*************************************************************************************************************

                                            -= Astro Corp. CGA Hardware =-
                                                 (known as "Permus")

                                       driver by   Luca Elia (l.elia@tin.it)
                                   decryption by   Olivier Galibert

CPU:    68000
GFX:    ASTRO V0x (seen with x = 1,2,5,6 or 7)
SOUND:  OKI M6295 (AD-65)
OTHER:  EEPROM, Battery

 512 sprites, each made of N x M tiles. Tiles are 16x16x8 (16x32x8 in Stone Age)

---------------------------------------------------------------------------------------------------------------------------------------------------------------------
Year + Game              PCB ID                    CPU                Video          Chips                                      Notes
---------------------------------------------------------------------------------------------------------------------------------------------------------------------
00  Show Hand            CHE-B50-4002A             MC68HC000FN12      ASTRO V01      pLSI1016-60LJ, ASTRO 0001B MCU? (28 pins)
00  Wangpai Duijue       CHE-B50-4002A             MC68HC000FN12      ASTRO V01      pLSI1016,      MDT2020AP   MCU  (28 pins)
01  Magic Bomb (NB4.5)   None                      ASTRO V03          ASTRO V02      pLSI1016                                   Encrypted
02  Skill Drop GA        None                      JX-1689F1028N      ASTRO V02      pLSI1016-60LJ
02? Keno 21              ?                         ASTRO V102?        ASTRO V05      ASTRO F02?                                 not dumped
03  Magic Bomb (AB5.3)   CS350P003                 ASTRO V102PX-014?  ASTRO V01      ASTRO F02 2003-03-31                       Encrypted
03  Speed Drop           None                      JX-1689HP          ASTRO V05      pLSI1016-60LJ
03  Speed Master (V1.0)  M02                       AST-V102PX         scratched      scratched                                  Encrypted
04  Zoo                  M1.1                      ASTRO V102PX-005?  ASTRO V06      ASTRO F02 2005-02-18                       Encrypted
04  Magic Bomb (NB6.1)   J (CS350P001 + CS350P033) ASTRO V102PX-014?  ASTRO V07      ?,                                         Encrypted, select CGA / VGA via jumper
04  Go & Stop            K2 (CS350P011)            no markings        ASTRO V05      ASTRO F01 2007-06-03                       Encrypted
04  Magic Bomb (AA.72C)  M1.1                      ASTRO V102PX-014?  ASTRO V05      ASTRO F02 2005-02-18                       Encrypted
04  Speed Master (V1.0)  M02                       AST-V102PX         scratched      scratched                                  Encrypted
04  Speed Master (V1.0)  CS350P003                 scratched          scratched      scratched                                  Encrypted
05  Monkey Land (AA.13B) M1                        ASTRO V102PX-005?  ASTRO V06      ASTRO F02 2004-05-18                       Encrypted
05  Monkey Land (AA.21A) M1.1                      ASTRO V102PX-005?  ASTRO V06      ASTRO F02 2004-12-04                       Encrypted
05  Monkey Land (AA.21C) M1.2                      ASTRO V102PX-005?  ASTRO V07      ASTRO F01 2006-12-17                       Encrypted
05  Dino Dino            T-3802A                   ASTRO V102PX-010?  ASTRO V05      ASTRO F02 2003-03-12                       Encrypted
05  Stone Age            L1                        ASTRO V102PX-012?  ASTRO V05(x2)  ASTRO F02 2004-09-04                       Encrypted
05? Hacher (hack)        M1.2                      ?                  ?              ASTRO F02 2005-02-18                       Encrypted
05  Wicked Witch         O (CS350P032)             ASTRO V102PX-016?  ASTRO V06      ASTRO F02 2005-09-17                       Encrypted
06  Captain Shark        M1.2                      ASTRO V102PX-006?  ASTRO V06      ASTRO F02 2005-05-29                       Encrypted
06  Win Win Bingo        M1.2                      ASTRO V102PX-006?  ASTRO V06      ASTRO F02 2005-09-17                       Encrypted
07? Western Venture      O (CS350P032)             ASTRO V102?        ASTRO V07      ASTRO F01 2007-06-03                       Encrypted
07  Happy Farm           _P_ROHS                   ASTRO V102PX-008?  ASTRO V07      ASTRO ROHS BA21C00009 M835KK01             Encrypted
----------------------------------------------------------------------------------------------------------------------------------------------------------------------

- astoneag, magibomb, winbingo, etc.: to initialize EEPROM (and self test in some games), keep keyout (W) pressed during boot.
  Needs ticket (or reserve switch) and hopper lines to be 0 though (disconnected?).
- magibomb: sets with a "game count" option, when set to "yes", display the number of games played by the current player.
  Pressing payout (I) disallows registering further coin/key-ins, decreases this number, and activates the hopper.
  But when the games count is 0 only the first step is performed, preventing further coin-in until reset or key-out. Probably an original game bug.
- showhand: in the settings screen, to reset settings to factory defaults in battery-backed RAM, press keyout + memory reset (W + F1).
  When exiting the settings screen, settings are saved in the EEPROM.
- showhand: to mark the settings in battery-backed RAM as valid and saving them to EEPROM, keep keyout + memory reset (W + F1) pressed during boot.
- showhand: after pressing payout (I), press start while "COLLECTING POINTS" is displayed to activate the hopper.
  Otherwise, while "GAME" is displayed, the points just vanish.
- showhand: hopper jam error if "payout sensor type" is set to "normal low" in the settings.
  It seems the game code is buggy: it uses 4000 as hopper status mask (from showhanc) instead of 8000.
- winbingo: it starts with a Tetris game. To switch to the gambling game, press stop all + stop 2 (Z + C).

TODO:

- MCU on showhand/showhanc PCB, what does it do? Games run fine and there appears to be no interaction...
- Protection in magibomb sets with simpler encryption (writes to $a0101x, $a11100, etc.). Some sets appear to work fine regardless, others freeze.
- Fix mirror ROM checksum / ROM overlay without code patches (in games with heavier encryption).
- Find source of level 2 interrupt (sprite DMA end?).
- magibomba, westvent: need a redump of one of the program ROMs.
- magibombg, hacher: need a redump of the sprite ROMs.
- astoneag, dinodino, magibombd, magibombg: exiting from test menu goes haywire (requires a soft-reset with F3).
- magibombg,m: need RE of the CPU code and correct EEPROM.
- gostopac: stops with 'S4' message during boot. Needs RE of the CPU code and emulation of its peculiarities.
- monkeyl and clones: need RE of the CPU code, inputs and layout. After reset it initializes.
- speedmst,a,b: need RE of the CPU code, correct EEPROM. Won't boot right now.
- cptshark: needs verifying of inputs and layout
- wwitch: needs correct GFX ROMs loading / decode, RE of the CPU code, inputs, outputs. Currently starts but then freezes.
- hapfarm: doesn't work, possibly wrong interrupts. Code base differs significantly, needs studying. Probably the link between
  this generation of hardware and the one in astrocorp/hummer.cpp.

*************************************************************************************************************/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "machine/eepromser.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "machine/timer.h"
#include "sound/okim6295.h"
#include "video/ramdac.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "astoneag.lh"
#include "dinodino.lh"
#include "magibomb.lh"
#include "magibombb.lh"
#include "showhanc.lh"
#include "showhand.lh"
#include "skilldrp.lh"
#include "winbingo.lh"
#include "zoo.lh"

/***************************************************************************

    CPU Code (Protection)

***************************************************************************/

class astro_cpucode_device : public device_t
{
public:
	astro_cpucode_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// read handlers
	int do_read();              // DO

	// write handlers
	void cs_write(int state);   // CS signal (active high)
	void clk_write(int state);  // CLK signal (active high)

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_region_ptr<u16> m_region_key;

	u8 m_cs_state = CLEAR_LINE;
	attotime m_last_cs_rising_edge_time = attotime::zero;
	u8 m_clk_state = CLEAR_LINE;
	u16 m_shift_register = 0;
};

DEFINE_DEVICE_TYPE(ASTRO_CPUCODE, astro_cpucode_device, "astro_cpucode", "Astro CPU Code")

astro_cpucode_device::astro_cpucode_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ASTRO_CPUCODE, tag, owner, clock)
	, m_region_key(*this, DEVICE_SELF)
{ }

void astro_cpucode_device::device_start()
{
	save_item(NAME(m_cs_state));
	save_item(NAME(m_last_cs_rising_edge_time));
	save_item(NAME(m_clk_state));
	save_item(NAME(m_shift_register));
}

void astro_cpucode_device::device_reset()
{
	m_shift_register = 0;
}

int astro_cpucode_device::do_read()
{
	return BIT(m_shift_register, 15);
}

void astro_cpucode_device::cs_write(int state)
{
	if (state != m_cs_state)
	{
		m_cs_state = state;
		if (state == ASSERT_LINE)
		{
			m_last_cs_rising_edge_time = machine().time();
			constexpr int shift = 11; // 0 1 10AAAAAA R
			const u16 cpucode = *m_region_key;
			m_shift_register = BIT(cpucode, shift, 16 - shift) | (BIT(cpucode, 0, shift) << (16 - shift));
		}
	}
}

void astro_cpucode_device::clk_write(int state)
{
	if (m_cs_state == ASSERT_LINE && state != m_clk_state)
	{
		m_clk_state = state;
		if (state == ASSERT_LINE && machine().time() > m_last_cs_rising_edge_time)
			m_shift_register = BIT(m_shift_register, 15) | (BIT(m_shift_register, 0, 15) << 1);
	}
}

namespace {

class astrocorp_state : public driver_device
{
public:
	astrocorp_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_hopper(*this, "hopper"),
		m_ticket(*this, "ticket"),
		m_spriteram(*this, "spriteram"),
		m_eeprom_out(*this, "EEPROM_OUT"),
		m_can_flip_sprites(true),
		m_lamps(*this, "lamp%u", 0U)
	{ }

	void luckycoin(machine_config &config);
	void showhanc(machine_config &config);
	void showhand(machine_config &config);
	void skilldrp(machine_config &config);
	void speeddrp(machine_config &config);

	void init_showhanc();
	void init_showhand();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<hopper_device> m_hopper;
	required_device<hopper_device> m_ticket;

	// memory pointers
	required_shared_ptr<u16> m_spriteram;

	// i/o ports
	required_ioport m_eeprom_out;

	// video-related
	u8 m_screen_enable = 0;
	bool m_can_flip_sprites;

	TIMER_DEVICE_CALLBACK_MEMBER(irq_2_4_scanline_cb);

	void draw_sprites_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	virtual void eeprom_w(u8 data);
	void showhanc_outputs_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void showhand_outputs_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void skilldrp_outputs_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void magibomb_outputs_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void screen_enable_w(u8 data);
	u16 unk_r();
	void oki_bank_w(u8 data);

private:
	// video-related
	bitmap_ind16 m_bitmap;
	u16 m_sprite_dma = 0;

	output_finder<8> m_lamps;

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void luckycoin_map(address_map &map) ATTR_COLD;
	void showhanc_map(address_map &map) ATTR_COLD;
	void showhand_map(address_map &map) ATTR_COLD;
	void skilldrp_map(address_map &map) ATTR_COLD;
	void speeddrp_map(address_map &map) ATTR_COLD;
};

// Simpler encryption. No IRQs.
class magibomb_state : public astrocorp_state
{
public:
	magibomb_state(const machine_config &mconfig, device_type type, const char *tag) :
		astrocorp_state(mconfig, type, tag)
	{ }

	void magibomb(machine_config &config);
	void magibombb(machine_config &config);
	void magibombf(machine_config &config);

	void init_magibomb(); // simple encryption

private:
	u16 video_flags_r();

	void magibomb_base_map(address_map &map, u32 base_offs) ATTR_COLD;
	void magibomb_map(address_map &map) ATTR_COLD;
	void magibombb_map(address_map &map) ATTR_COLD;
	void magibombf_map(address_map &map) ATTR_COLD;
};

// Heavier encryption. CPU Code protection.
class zoo_state : public astrocorp_state
{
public:
	zoo_state(const machine_config &mconfig, device_type type, const char *tag) :
		astrocorp_state(mconfig, type, tag),
		m_cpucode(*this, "astro_cpucode"),
		m_cpucode_out(*this, "CPUCODE_OUT")
	{ }

	void hacher(machine_config &config);
	void dinodino(machine_config &config);
	void gostop(machine_config &config);
	void hapfarm(machine_config &config);
	void magibombd(machine_config &config);
	void magibombg(machine_config &config);
	void magibombm(machine_config &config);
	void monkeyl(machine_config &config);
	void monkeyld(machine_config &config);
	void speedmst(machine_config &config);
	void winbingo(machine_config &config);
	void wwitch(machine_config &config);
	void zoo(machine_config &config);

	void init_cptshark();
	void init_dinodino();
	void init_gostop();
	void init_hacher();
	void init_hapfarm();
	void init_magibombd();
	void init_magibombg();
	void init_magibombm();
	void init_monkeyl();
	void init_monkeyla();
	void init_monkeyld();
	void init_speedmst();
	void init_speedmsta();
	void init_speedmstb();
	void init_winbingo();
	void init_winbingoa();
	void init_wwitch();
	void init_zoo();

	virtual void eeprom_w(u8 data) override;

protected:
	virtual void machine_start() override ATTR_COLD;

	struct decryption_info {
		struct {
			// Address bits used for bitswap/xor selection
			u8 bits[3];
			struct {
				// 8-8 Bitswap
				u8 bits[8];
				// Xor
				u8 xor_mask;
			} entries[8];
		} rom[2];
		// Global address bitswap (src -> dest, some sets use bits 12-8 only, while others 12-2)
		u8 bits[11];
	};
	void decrypt_rom(const decryption_info &table);

private:
	required_device<astro_cpucode_device> m_cpucode;

	required_ioport m_cpucode_out;

	TIMER_DEVICE_CALLBACK_MEMBER(irq_1_2_scanline_cb);

	void dinodino_map(address_map &map) ATTR_COLD;
	void gostop_map(address_map &map) ATTR_COLD;
	void hacher_map(address_map &map) ATTR_COLD;
	void hapfarm_map(address_map &map) ATTR_COLD;
	void magibombd_map(address_map &map) ATTR_COLD;
	void magibombg_map(address_map &map) ATTR_COLD;
	void magibombm_map(address_map &map) ATTR_COLD;
	void monkeyl_map(address_map &map) ATTR_COLD;
	void speedmst_map(address_map &map) ATTR_COLD;
	void winbingo_map(address_map &map) ATTR_COLD;
	void wwitch_map(address_map &map) ATTR_COLD;
	void zoo_map(address_map &map) ATTR_COLD;

	static const decryption_info gostop_table;
	static const decryption_info v102_px005_table;
	static const decryption_info v102_px006_table;
	static const decryption_info v102_px008_table;
	static const decryption_info v102_px010_table;
	static const decryption_info v102_px014_table;
	static const decryption_info v102_px016_table;
};

// Adds RAMDAC and 16x32 sprites
class astoneag_state : public zoo_state
{
public:
	astoneag_state(const machine_config &mconfig, device_type type, const char *tag) :
		zoo_state(mconfig, type, tag),
		m_ramdac(*this, "ramdac")
	{ }

	void astoneag(machine_config &config);

	void init_astoneag();

private:
	required_device<ramdac_device> m_ramdac;

	void astoneag_map(address_map &map) ATTR_COLD;
	void ramdac_map(address_map &map) ATTR_COLD;

	static const decryption_info v102_px012_table;
	void interleave_sprites_16x32();
};

/***************************************************************************
                                Video
***************************************************************************/

void astrocorp_state::video_start()
{
	m_screen->register_screen_bitmap(m_bitmap);

	save_item(NAME(m_bitmap));
	save_item(NAME(m_screen_enable));
	save_item(NAME(m_sprite_dma));

	m_sprite_dma = 0;
}

/***************************************************************************
                              Sprites Format

    Offset:    Bits:                  Value:

    0          f--- ---- ---- ----    End flag (if 0 then stop processing)
               -e-- ---- ---- ----    ? set to 0
               --dc ba9- ---- ----    ignored?
               ---- ---8 7654 3210    X

    1                                 Code

    2          fedc ba-- ---- ----    ignored?
               ---- --9- ---- ----    Flip Y (ignored in showhand/showhanc)
               ---- ---8 ---- ----    Flip X ""
               ---- ---- 7654 3210    Y

    3          fedc ba98 ---- ----    X Size
               ---- ---- 7654 3210    Y Size

***************************************************************************/

void astrocorp_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const u16 *source = m_spriteram;
	const u16 * const finish = m_spriteram + m_spriteram.length();

	gfx_element * const gfx = m_gfxdecode->gfx(0);

	const int yfactor = gfx->height() / 16;

	for ( ; source < finish; source += 8 / 2 )
	{
		int sx      = source[ 0x0/2 ];
		int code    = source[ 0x2/2 ];
		int sy      = source[ 0x4/2 ];
		int size    = source[ 0x6/2 ];

		// end flag, particularly needed by magibomb
		if (!(sx & 0x8000))
			return;

		const int dimx = (size >> 8) & 0xff;
		const int dimy = (size >> 0) & 0xff;

		// Showhand pollutes the flip bits with negative y coordinates (when the new opponent face enters the screen).
		// So at least this game has no flip bits in sy.
		const int flipx = m_can_flip_sprites ? sy & 0x100 : 0;
		const int flipy = m_can_flip_sprites ? sy & 0x200 : 0;

		sx &= 0x01ff;
		sy &= 0x00ff;

		for (int y = 0 ; y < dimy ; y++)
		{
			for (int x = 0 ; x < dimx ; x++)
			{
				for (int ywrap = 0 ; ywrap <= 0x100 ; ywrap += 0x100)
				{
					for (int xwrap = 0 ; xwrap <= 0x200 ; xwrap += 0x200)
					{
						int resx = flipx ? (sx + 1) - ((x + 1) * 16 + xwrap) : sx + x * 16 - xwrap;
						int resy = flipy ? (sy + 1) - ((y + 1) * 16 + ywrap) : sy + y * 16 - ywrap;
						gfx->transpen(bitmap,cliprect,
								code, 0,
								flipx, flipy,
								resx, resy * yfactor, 0xff);
					}
				}
				code++;
			}
		}
	}
}

u32 astrocorp_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (BIT(m_screen_enable, 0))
		copybitmap(bitmap, m_bitmap, 0,0,0,0, cliprect);
	else
		bitmap.fill(m_palette->black_pen(), cliprect);

	return 0;
}

/***************************************************************************
                                Memory Maps
***************************************************************************/

void astrocorp_state::draw_sprites_w(offs_t offset, u16 data, u16 mem_mask)
{
	const u16 old = m_sprite_dma;
	const u16 now = COMBINE_DATA(&m_sprite_dma);

	if (!old && now)
		draw_sprites(m_bitmap, m_screen->visible_area());
}

void astrocorp_state::eeprom_w(u8 data)
{
	m_eeprom_out->write(data, 0xff);
}

void astrocorp_state::oki_bank_w(u8 data)
{
	m_oki->set_rom_bank(BIT(data, 0));
//  logerror("%s: OKI bank %02X\n", machine().describe_context(), data);
	if (data & (~1))
		logerror("%s: warning, unknown bits written in oki_bank_w = %02X\n", machine().describe_context(), data);
}

void astrocorp_state::showhand_outputs_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_hopper->motor_w(                        BIT(data,  0)); // (0001) hopper motor
		//                                        BIT(data,  1);  // (0002)
		machine().bookkeeping().coin_counter_w(0, BIT(data,  2)); // (0004) coin-in / key-in counter
		m_lamps[0] =                              BIT(data,  3);  // (0008) win lamp
		machine().bookkeeping().coin_counter_w(1, BIT(data,  4)); // (0010) coin-out counter
		m_lamps[1] =                              BIT(data,  5);  // (0020) error lamp (coin/hopper jam)
		//                                        BIT(data,  6);  // (0040)
		//                                        BIT(data,  7);  // (0080)
	}
	if (ACCESSING_BITS_8_15)
	{
		m_lamps[2] =                              BIT(data,  8);  // (0100) bet / double up lamp
		m_lamps[3] =                              BIT(data,  9);  // (0200) payout lamp
		//                                        BIT(data, 10);  // (0400)
		m_lamps[4] =                              BIT(data, 11);  // (0800) start / take lamp
		m_ticket->motor_w(                        BIT(data, 12)); // (1000) ticket motor
		m_lamps[5] =                              BIT(data, 13);  // (2000) select or yes / big lamp ?
		m_lamps[6] =                              BIT(data, 14);  // (4000) look / small lamp
		m_lamps[7] =                              BIT(data, 13);  // (8000) select or yes / big lamp ?
	}

	if (data & 0x04c2)
		logerror("%s: warning, unknown bits written in outputs_w = %04x\n", machine().describe_context(), data);

//  popmessage("OUT %04X",data);
}

void astrocorp_state::showhanc_outputs_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		//                                        BIT(data,  0);  // (0001)
		machine().bookkeeping().coin_counter_w(0, BIT(data,  1)); // (0002) coin-in / key-in counter
		//                                        BIT(data,  2);  // (0004)
		m_hopper->motor_w(                        BIT(data,  3)); // (0008) hopper motor
		machine().bookkeeping().coin_counter_w(1, BIT(data,  4)); // (0010) coin out
		m_lamps[0] =                              BIT(data,  5);  // (0020) error lamp (coin/hopper jam)
		//                                        BIT(data,  6);  // (0040)
		//                                        BIT(data,  7);  // (0080)
	}
	if (ACCESSING_BITS_8_15)
	{
		//                                        BIT(data,  8);  // (0100)
		m_lamps[1] =                              BIT(data,  9);  // (0200) payout lamp
		m_lamps[2] =                              BIT(data, 10);  // (0400) bet / double up lamp
		m_lamps[3] =                              BIT(data, 11);  // (0800) start / take lamp
		m_lamps[4] =                              BIT(data, 12);  // (1000) win lamp
		m_lamps[5] =                              BIT(data, 13);  // (2000) look / small lamp
		m_lamps[6] =                              BIT(data, 14);  // (4000) select or yes / big lamp ?
		m_lamps[7] =                              BIT(data, 15);  // (8000) select or yes / big lamp ?
	}

	if (data & 0x01c5)
		logerror("%s: warning, unknown bits written in outputs_w = %04x\n", machine().describe_context(), data);

//  popmessage("OUT %04X",data);
}

void astrocorp_state::skilldrp_outputs_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		machine().bookkeeping().coin_counter_w(0, BIT(data,  0));  // (0001) key in   count |
		//                                        BIT(data,  1));  // (0002) coin in  count |- skilldrp manual shows 1 in- and 1 out- counter
		machine().bookkeeping().coin_counter_w(1, BIT(data,  2));  // (0004) key out  count |
		m_hopper->motor_w(                        BIT(data,  3));  // (0008) hopper   motor
		//                                        BIT(data,  4));  // (0010) coin out count |
		m_lamps[0] =                              BIT(data,  5);   // (0020) error    lamp (coin/hopper jam: "call attendant")
		//                                        BIT(data,  6);   // (0040)
		m_ticket->motor_w(                        BIT(data,  7));  // (0080) ticket   motor
	}
	if (ACCESSING_BITS_8_15)
	{
		m_lamps[1] =                              BIT(data,  8);   // (0100) select / double up lamp
		//                                        BIT(data,  9);   // (0200)
		m_lamps[3] =                              BIT(data, 10);   // (0400) take lamp
		m_lamps[4] =                              BIT(data, 11);   // (0800) play (bet) lamp
		m_lamps[5] =                              BIT(data, 12);   // (1000) start lamp
		//                                        BIT(data, 13);   // (2000)
		m_lamps[7] =                              BIT(data, 14);   // (4000) win lamp
		//                                        BIT(data, 15);   // (8000) ticket dispenser enable?
	}
	// account                                                        (5d20 4d20 4520 4420 4020 4000)

	if (data & 0x2240)
		logerror("%s: warning, unknown bits written in outputs_w = %04x\n", machine().describe_context(), data);

//  popmessage("OUT %04X",data);
}

void astrocorp_state::magibomb_outputs_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		machine().bookkeeping().coin_counter_w(0, BIT(data,  0));  // (0001) key in   count |
		machine().bookkeeping().coin_counter_w(1, BIT(data,  1));  // (0002) coin in  count |- magibomb manual shows 2 in- and 2 out- counters
		machine().bookkeeping().coin_counter_w(2, BIT(data,  2));  // (0004) key out  count |
		m_hopper->motor_w(                        BIT(data,  3));  // (0008) hopper   motor
		machine().bookkeeping().coin_counter_w(3, BIT(data,  4));  // (0010) coin out count |
		m_lamps[0] =                              BIT(data,  5);   // (0020) error    lamp (coin/hopper jam: "call attendant")
		//                                        BIT(data,  6);   // (0040) ? used in-game
		m_ticket->motor_w(                        BIT(data,  7));  // (0080) ticket   motor
	}
	if (ACCESSING_BITS_8_15)
	{
		m_lamps[1] =                              BIT(data,  8);   // (0100) stop 1 / double up lamp
		m_lamps[2] =                              BIT(data,  9);   // (0200) stop all / big lamp
		m_lamps[3] =                              BIT(data, 10);   // (0400) stop 3 / take lamp
		m_lamps[4] =                              BIT(data, 11);   // (0800) bet lamp
		m_lamps[5] =                              BIT(data, 12);   // (1000) start lamp
		m_lamps[6] =                              BIT(data, 13);   // (2000) stop 2 / small lamp
		m_lamps[7] =                              BIT(data, 14);   // (4000) win lamp
		//                                        BIT(data, 15);   // (8000) ticket dispenser enable?
	}

	if (data & 0x0040)
		logerror("%s: warning, unknown bits written in outputs_w = %04x\n", machine().describe_context(), data);

//  popmessage("OUT %04X",data);
}

void astrocorp_state::screen_enable_w(u8 data)
{
	m_screen_enable = data;
//  popmessage("ENAB %02X",data);
	if (m_screen_enable & (~1))
		logerror("%s: warning, unknown bits written in screen_enable_w = %02X\n", machine().describe_context(), m_screen_enable);
}

u16 astrocorp_state::unk_r()
{
	return 0xffff;  // bit 3?
}

void astrocorp_state::showhand_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom();
	map(0x050000, 0x050fff).ram().share("spriteram");
	map(0x052000, 0x052001).nopr().w(FUNC(astrocorp_state::draw_sprites_w));
	map(0x054000, 0x054001).portr("INPUTS");
	map(0x058001, 0x058001).w(FUNC(astrocorp_state::eeprom_w));
	map(0x05a000, 0x05a001).w(FUNC(astrocorp_state::showhand_outputs_w));
	map(0x05e000, 0x05e001).portr("EEPROM_IN");
	map(0x060000, 0x0601ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x070000, 0x073fff).ram().share("nvram"); // battery
	map(0x080000, 0x080000).w(FUNC(astrocorp_state::oki_bank_w));
	map(0x0a0000, 0x0a0001).nopr().w(FUNC(astrocorp_state::screen_enable_w)).umask16(0x00ff);
	map(0x0d0000, 0x0d0001).r(FUNC(astrocorp_state::unk_r));
	map(0x0d0000, 0x0d0000).w(m_oki, FUNC(okim6295_device::write));
}

void astrocorp_state::showhanc_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom();
	map(0x060000, 0x0601ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x070000, 0x070000).w(FUNC(astrocorp_state::oki_bank_w));
	map(0x080000, 0x080fff).ram().share("spriteram");
	map(0x082000, 0x082001).nopr().w(FUNC(astrocorp_state::draw_sprites_w));
	map(0x084000, 0x084001).portr("INPUTS");
	map(0x088001, 0x088001).w(FUNC(astrocorp_state::eeprom_w));
	map(0x08a000, 0x08a001).w(FUNC(astrocorp_state::showhanc_outputs_w));
	map(0x08e000, 0x08e001).portr("EEPROM_IN");
	map(0x090000, 0x093fff).ram().share("nvram"); // battery
	map(0x0a0000, 0x0a0001).nopr().w(FUNC(astrocorp_state::screen_enable_w)).umask16(0x00ff);
	map(0x0e0000, 0x0e0001).r(FUNC(astrocorp_state::unk_r));
	map(0x0e0000, 0x0e0000).w(m_oki, FUNC(okim6295_device::write));
}

void astrocorp_state::skilldrp_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x200000, 0x200fff).ram().share("spriteram");
	map(0x202000, 0x202001).nopr().w(FUNC(astrocorp_state::draw_sprites_w));
	map(0x204000, 0x204001).portr("INPUTS");
	map(0x208001, 0x208001).w(FUNC(astrocorp_state::eeprom_w));
	map(0x20a000, 0x20a001).nopr().w(FUNC(astrocorp_state::skilldrp_outputs_w));
	map(0x20e000, 0x20e001).portr("EEPROM_IN");
	map(0x380000, 0x3801ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x400000, 0x400001).nopr().w(FUNC(astrocorp_state::screen_enable_w)).umask16(0x00ff);
	map(0x500000, 0x507fff).ram().share("nvram"); // battery
	map(0x580001, 0x580001).w(FUNC(astrocorp_state::oki_bank_w));
	map(0x600001, 0x600001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}

void astrocorp_state::luckycoin_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x280000, 0x2801ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x300000, 0x300fff).ram().share("spriteram");
	map(0x302000, 0x302001).nopr().w(FUNC(astrocorp_state::draw_sprites_w));
	map(0x304000, 0x304001).portr("INPUTS");
	map(0x308001, 0x308001).w(FUNC(astrocorp_state::eeprom_w));
	map(0x30a000, 0x30a001).nopr().w(FUNC(astrocorp_state::skilldrp_outputs_w));
	map(0x30e000, 0x30e001).portr("EEPROM_IN");
	map(0x400000, 0x407fff).ram().share("nvram"); // battery
	map(0x500000, 0x500001).nopr().w(FUNC(astrocorp_state::screen_enable_w)).umask16(0x00ff);
	map(0x580001, 0x580001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x600001, 0x600001).w(FUNC(astrocorp_state::oki_bank_w));
}

void astrocorp_state::speeddrp_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom();
	map(0x280000, 0x283fff).ram().share("nvram"); // battery
	map(0x380000, 0x380fff).ram().share("spriteram");
	map(0x382000, 0x382001).nopr().w(FUNC(astrocorp_state::draw_sprites_w));
	map(0x384000, 0x384001).portr("INPUTS");
	map(0x388001, 0x388001).w(FUNC(astrocorp_state::eeprom_w));
	map(0x38a000, 0x38a001).w(FUNC(astrocorp_state::skilldrp_outputs_w));
	map(0x38e000, 0x38e001).portr("EEPROM_IN");
	map(0x480000, 0x4801ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x500000, 0x500001).nopr().w(FUNC(astrocorp_state::screen_enable_w)).umask16(0x00ff);
	map(0x580001, 0x580001).w(FUNC(astrocorp_state::oki_bank_w));
	map(0x600001, 0x600001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}

// PC refers to magibomb:
// - 0x22a4 reads from $a0101a, resets if 0
// - 0x8ba if $a10001 & 0xf != 0 it writes "SEGA" to $a14000 (!?)
// - all irq service routines write an id number to register D1 and then reset the program flow,
//   is it expecting to NOT use irqs at all and actually using it for remote control?
// - tight loops at 0x1fb2, bypass by jumping to 0x1fb6, otherwise works on an initialized nvram/eeprom,
//   keep keyout pressed during boot to initialize (needs ticket and hopper lines to be 0 though: disconnected?).

u16 magibomb_state::video_flags_r()
{
	// bit 0: vblank? bit 3: sprite busy flag?
	return (m_screen->vblank() ^ 1) | ((m_screen->vblank() ^ 1) << 3);
}

void magibomb_state::magibomb_base_map(address_map &map, u32 base_offs)
{
	map(0x000000, 0x01ffff).rom();
//  map(0x040000, 0x07ffff) in client (later HW maps these ones at 0x50000-0x6ffff instead of 0x40000-0x5ffff)
	map(0x040000+base_offs, 0x043fff+base_offs).ram().share("nvram");
	map(0x050000+base_offs, 0x050fff+base_offs).ram().share("spriteram");
	map(0x052000+base_offs, 0x052001+base_offs).nopr().w(FUNC(magibomb_state::draw_sprites_w));
	map(0x054000+base_offs, 0x054001+base_offs).portr("INPUTS");
	map(0x058001+base_offs, 0x058001+base_offs).w(FUNC(magibomb_state::eeprom_w));
	map(0x05a000+base_offs, 0x05a001+base_offs).w(FUNC(magibomb_state::magibomb_outputs_w));
	map(0x05e000+base_offs, 0x05e001+base_offs).portr("EEPROM_IN");

	map(0x080001, 0x080001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x090000, 0x090000).w(FUNC(magibomb_state::oki_bank_w));
	map(0x0a0000, 0x0a0001).r(FUNC(magibomb_state::video_flags_r));
	map(0x0a0000, 0x0a0000).w(FUNC(magibomb_state::screen_enable_w));
	map(0x0b0000, 0x0b01ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");

	map(0xa00000, 0xa005ff).ram(); // unknown
//  map(0xa01000, 0xa0101f).rw
	map(0xa0101a, 0xa0101b).r(FUNC(magibomb_state::unk_r));
//      0xa10001.b r (bits 0-3)
//      0xa11100.w w (0x0000 -> 0x0100)
//      0xa11100.b r (bit 0)
//      0xa11200.w w (0x0000 -> 0x0100)
//      0xa14000.l w ("SEGA" !?)
//      0xc00004.w w (0x8000 -> 0x8100)
//      0xc00005.b r (bit 1)
}

void magibomb_state::magibomb_map(address_map &map)
{
	magibomb_base_map(map, 0x00000);
}

void magibomb_state::magibombb_map(address_map &map)
{
	magibomb_base_map(map, 0x10000);
}

void magibomb_state::magibombf_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom();
	map(0x060000, 0x063fff).ram().share("nvram");
	map(0x070000, 0x070001).r(FUNC(magibomb_state::video_flags_r));
	map(0x080000, 0x080fff).ram().share("spriteram");
	map(0x082000, 0x082001).nopr().w(FUNC(magibomb_state::draw_sprites_w));
	map(0x084000, 0x084001).portr("INPUTS");
	map(0x088001, 0x088001).w(FUNC(magibomb_state::eeprom_w));
	map(0x08a000, 0x08a001).w(FUNC(magibomb_state::magibomb_outputs_w));
	map(0x08e000, 0x08e001).portr("EEPROM_IN");
	map(0x090000, 0x0901ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x0a0000, 0x0a0001).nopr().w(FUNC(magibomb_state::screen_enable_w)).umask16(0xff00);
	map(0x0b0000, 0x0b0000).w(FUNC(magibomb_state::oki_bank_w));
	map(0x0c0001, 0x0c0001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));

	map(0xa00000, 0xa005ff).ram(); // unknown
	map(0xa0101a, 0xa0101b).r(FUNC(magibomb_state::unk_r));
}

void zoo_state::eeprom_w(u8 data)
{
	m_eeprom_out->write(data, 0xff);
	m_cpucode_out->write(data, 0xff);
}

void zoo_state::dinodino_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom().mirror(0x800000); // POST checks for ROM checksum at mirror
	map(0x900000, 0x903fff).ram().share("nvram"); // battery
	map(0xb00000, 0xb00fff).ram().share("spriteram");
	map(0xb02000, 0xb02001).nopr().w(FUNC(zoo_state::draw_sprites_w));
	map(0xb04000, 0xb04001).portr("INPUTS");
	map(0xb08001, 0xb08001).w(FUNC(zoo_state::eeprom_w));
	map(0xb0a000, 0xb0a001).w(FUNC(zoo_state::magibomb_outputs_w));
	map(0xb0e000, 0xb0e001).portr("EEPROM_IN");
	map(0xb80000, 0xb80001).portr("CPUCODE_IN");
	map(0xc80000, 0xc801ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xd00001, 0xd00001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xd80000, 0xd80000).w(FUNC(zoo_state::oki_bank_w));
//  map(0x??0001, 0x??0001).w(FUNC(zoo_state::screen_enable_w)); // unknown location
}

void zoo_state::magibombd_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom().mirror(0x800000); // POST checks for ROM checksum at mirror
	map(0xa80000, 0xa80fff).ram().share("spriteram");
	map(0xa82000, 0xa82001).nopr().w(FUNC(zoo_state::draw_sprites_w));
	map(0xa84000, 0xa84001).portr("INPUTS");
	map(0xa88001, 0xa88001).w(FUNC(zoo_state::eeprom_w));
	map(0xa8a000, 0xa8a001).w(FUNC(zoo_state::magibomb_outputs_w));
	map(0xa8e000, 0xa8e001).portr("EEPROM_IN");
	map(0xb80000, 0xb80001).portr("CPUCODE_IN");
	map(0xc80000, 0xc83fff).ram().share("nvram"); // battery
	map(0xd00000, 0xd001ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xd80000, 0xd80000).w(FUNC(zoo_state::oki_bank_w));
	map(0xe00001, 0xe00001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
//  map(0x??0001, 0x??0001).w(FUNC(zoo_state::screen_enable_w)); // unknown location
}

void zoo_state::magibombg_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom().mirror(0x800000); // POST checks for ROM checksum at mirror
	map(0xa00000, 0xa00fff).ram().share("spriteram");
	map(0xa02000, 0xa02001).nopr().w(FUNC(zoo_state::draw_sprites_w));
	map(0xa04000, 0xa04001).portr("INPUTS");
	map(0xa08001, 0xa08001).w(FUNC(zoo_state::eeprom_w));
	map(0xa0a000, 0xa0a001).w(FUNC(zoo_state::magibomb_outputs_w));
	map(0xa0e000, 0xa0e001).portr("EEPROM_IN");
	map(0xb80000, 0xb80001).portr("CPUCODE_IN");
	map(0xc00000, 0xc03fff).ram().share("nvram"); // battery
	map(0xd00000, 0xd001ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xd80000, 0xd80000).w(FUNC(zoo_state::oki_bank_w));
	map(0xe00001, 0xe00001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
//  map(0x??0001, 0x??0001).w(FUNC(zoo_state::screen_enable_w)); // unknown location
}

void zoo_state::magibombm_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom().mirror(0x800000); // POST checks for ROM checksum at mirror
	map(0x840000, 0x843fff).ram().share("nvram"); // battery
	map(0x880000, 0x880fff).ram().share("spriteram");
	map(0x882000, 0x882001).nopr().w(FUNC(zoo_state::draw_sprites_w));
	map(0x884000, 0x884001).portr("INPUTS");
	map(0x886000, 0x8861ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x888001, 0x888001).w(FUNC(zoo_state::eeprom_w));
	map(0x88a000, 0x88a001).w(FUNC(zoo_state::magibomb_outputs_w));
	map(0x890000, 0x890001).portr("EEPROM_IN");
	map(0x8a0000, 0x8a0001).portr("CPUCODE_IN");
	map(0x8b0000, 0x8b0000).w(FUNC(zoo_state::oki_bank_w));
	map(0x8c0001, 0x8c0001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
//  map(0x??0001, 0x??0001).w(FUNC(zoo_state::screen_enable_w)); // unknown location
}

void zoo_state::winbingo_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom().mirror(0x800000); // POST checks for ROM checksum at mirror
	map(0x900001, 0x900001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x980001, 0x980001).w(FUNC(zoo_state::oki_bank_w));
	map(0xa00000, 0xa03fff).ram().share("nvram"); // battery
	map(0xa80000, 0xa80fff).ram().share("spriteram");
	map(0xa82000, 0xa82001).nopr().w(FUNC(zoo_state::draw_sprites_w));
	map(0xa84000, 0xa84001).portr("INPUTS");
	map(0xa88001, 0xa88001).w(FUNC(zoo_state::eeprom_w));
	map(0xa8a000, 0xa8a001).w(FUNC(zoo_state::magibomb_outputs_w));
	map(0xa8e000, 0xa8e001).portr("EEPROM_IN");
	map(0xb00000, 0xb00001).nopr().w(FUNC(zoo_state::screen_enable_w)).umask16(0x00ff);
	map(0xd00000, 0xd001ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xd80000, 0xd80001).portr("CPUCODE_IN");
}

void zoo_state::hacher_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom().mirror(0x800000); // POST checks for ROM checksum at mirror
	map(0xb80000, 0xb80fff).ram().share("spriteram");
	map(0xb82000, 0xb82001).nopr().w(FUNC(zoo_state::draw_sprites_w));
	map(0xb84000, 0xb84001).portr("INPUTS");
	map(0xb88001, 0xb88001).w(FUNC(zoo_state::eeprom_w));
	map(0xb8a000, 0xb8a001).w(FUNC(zoo_state::magibomb_outputs_w));
	map(0xb8e000, 0xb8e001).portr("EEPROM_IN");
	map(0xa00001, 0xa00001).w(FUNC(zoo_state::oki_bank_w));
	map(0xc00001, 0xc00001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
//  map(0xc00001, 0xc00001).w(FUNC(zoo_state::screen_enable_w)); // also used as screen enable! But 0/1 are no-ops for the OKI, so maybe an innocuous bug of this hacked game?
	map(0xc80000, 0xc83fff).ram().share("nvram"); // battery
	map(0xd00000, 0xd001ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xd80000, 0xd80001).portr("CPUCODE_IN");
}

void zoo_state::zoo_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom().mirror(0x800000); // POST checks for ROM checksum at mirror
	map(0xa80000, 0xa83fff).ram().share("nvram"); // battery
	map(0xb80000, 0xb80fff).ram().share("spriteram");
	map(0xb82000, 0xb82001).nopr().w(FUNC(zoo_state::draw_sprites_w));
	map(0xb84000, 0xb84001).portr("INPUTS");
	map(0xb88001, 0xb88001).w(FUNC(zoo_state::eeprom_w));
	map(0xb8a000, 0xb8a001).w(FUNC(zoo_state::magibomb_outputs_w));
	map(0xb8e000, 0xb8e001).portr("EEPROM_IN");
	map(0xc00000, 0xc00001).nopr().w(FUNC(zoo_state::screen_enable_w)).umask16(0x00ff);
	map(0xc80000, 0xc801ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xd00001, 0xd00001).w(FUNC(zoo_state::oki_bank_w));
	map(0xd80001, 0xd80001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xe00000, 0xe00001).portr("CPUCODE_IN");
}

void zoo_state::gostop_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom().mirror(0x800000); // POST checks for ROM checksum at mirror
	map(0xb00001, 0xb00001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xb80000, 0xb80001).portr("CPUCODE_IN");
	map(0xc00000, 0xc00fff).ram().share("spriteram");
	map(0xc02000, 0xc02001).nopr().w(FUNC(zoo_state::draw_sprites_w));
	map(0xc04000, 0xc04001).portr("INPUTS");
	map(0xc08001, 0xc08001).w(FUNC(zoo_state::eeprom_w));
	map(0xc0a000, 0xc0a001).w(FUNC(zoo_state::magibomb_outputs_w));
	map(0xc0e000, 0xc0e001).portr("EEPROM_IN");
	map(0xc80000, 0xc801ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xd00000, 0xd03fff).ram().share("nvram"); // battery
//  map(0xb00001, 0xb00001).w(FUNC(zoo_state::oki_bank_w)); // TODO once it runs
//  map(0xc00000, 0xc00001).nopr().w(FUNC(zoo_state::screen_enable_w)).umask16(0x00ff); // unknown location
}

void zoo_state::monkeyl_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom().mirror(0x800000); // POST checks for ROM checksum at mirror
	map(0xa00000, 0xa00fff).ram().share("spriteram");
	map(0xa02000, 0xa02001).nopr().w(FUNC(zoo_state::draw_sprites_w));
	map(0xa04000, 0xa04001).portr("INPUTS");
	map(0xa08001, 0xa08001).w(FUNC(zoo_state::eeprom_w));
	map(0xa0a000, 0xa0a001).w(FUNC(zoo_state::magibomb_outputs_w));
	map(0xa0e000, 0xa0e001).portr("EEPROM_IN");
	map(0xa80000, 0xa83fff).ram().share("nvram"); // battery
	map(0xb80000, 0xb80001).portr("CPUCODE_IN");
	map(0xd00000, 0xd001ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xd80001, 0xd80001).w(FUNC(zoo_state::oki_bank_w));
	map(0xe80001, 0xe80001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}

void zoo_state::speedmst_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom().mirror(0x800000); // POST checks for ROM checksum at mirror
	map(0x850000, 0x850001).nopr().w(FUNC(zoo_state::screen_enable_w)).umask16(0x00ff);
	map(0x860000, 0x863fff).ram().share("nvram"); // battery
	map(0x880000, 0x880fff).ram().share("spriteram");
	map(0x882000, 0x882001).nopr().w(FUNC(zoo_state::draw_sprites_w));
	map(0x884000, 0x884001).portr("INPUTS");
	map(0x886000, 0x8861ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x888001, 0x888001).w(FUNC(zoo_state::eeprom_w));
	map(0x88a000, 0x88a001).w(FUNC(zoo_state::magibomb_outputs_w));
	map(0x890000, 0x890001).portr("EEPROM_IN");
	map(0x8a0000, 0x8a0001).portr("CPUCODE_IN");
	//map(0x8c0001, 0x8c0001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	//map(0x??0001, 0x??0001).w(FUNC(zoo_state::oki_bank_w))
}

void zoo_state::wwitch_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom().mirror(0x800000); // POST checks for ROM checksum at mirror
	map(0xa00001, 0xa00001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xa80000, 0xa80fff).ram().share("spriteram");
	map(0xa82000, 0xa82001).nopr().w(FUNC(zoo_state::draw_sprites_w));
	map(0xa84000, 0xa84001).portr("INPUTS");
	map(0xa88001, 0xa88001).w(FUNC(zoo_state::eeprom_w));
	map(0xa8a000, 0xa8a001).w(FUNC(zoo_state::magibomb_outputs_w));
	map(0xa8e000, 0xa8e001).portr("EEPROM_IN");
	map(0xb00000, 0xb00000).w(FUNC(zoo_state::oki_bank_w));
	map(0xb80000, 0xb83fff).ram().share("nvram"); // battery
	map(0xc00000, 0xc00001).portr("CPUCODE_IN");
	map(0xd00000, 0xd001ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
//  map(0x??0001, 0x??0001).w(FUNC(zoo_state::screen_enable_w)); // unknown location
}

void zoo_state::hapfarm_map(address_map &map) // TODO: verify everything
{
	map(0x000000, 0x03ffff).rom().mirror(0x800000); // POST checks for ROM checksum at mirror
	map(0x480000, 0x483fff).ram().share("nvram"); // battery
	map(0xb00000, 0xb00001).portr("CPUCODE_IN");
	map(0xb80000, 0xb80fff).ram().share("spriteram");
	map(0xb82000, 0xb82001).nopr().w(FUNC(zoo_state::draw_sprites_w));
	map(0xb84000, 0xb84001).portr("INPUTS");
	map(0xb88001, 0xb88001).w(FUNC(zoo_state::eeprom_w));
	map(0xb8a000, 0xb8a001).w(FUNC(zoo_state::magibomb_outputs_w));
	map(0xb8e000, 0xb8e001).portr("EEPROM_IN");
	map(0xd00000, 0xd001ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xd80000, 0xd80000).w(FUNC(zoo_state::oki_bank_w));
	map(0xe00001, 0xe00001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
//  map(0x??0001, 0x??0001).w(FUNC(zoo_state::screen_enable_w)); // unknown location
}

void astoneag_state::astoneag_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom().mirror(0x800000); // POST checks for ROM checksum at mirror
	map(0xb00000, 0xb03fff).ram().share("nvram"); // battery
	map(0xb80000, 0xb80001).portr("CPUCODE_IN");
	map(0xc00000, 0xc00fff).ram().share("spriteram");
	map(0xc02000, 0xc02001).nopr().w(FUNC(astoneag_state::draw_sprites_w));
	map(0xc04000, 0xc04001).portr("INPUTS");
	map(0xc08001, 0xc08001).w(FUNC(astoneag_state::eeprom_w));
	map(0xc0a000, 0xc0a001).w(FUNC(astoneag_state::magibomb_outputs_w));
	map(0xc0e000, 0xc0e001).portr("EEPROM_IN");
	map(0xd00000, 0xd00000).w(m_ramdac, FUNC(ramdac_device::index_w));
	map(0xd00002, 0xd00002).w(m_ramdac, FUNC(ramdac_device::pal_w));
	map(0xd00004, 0xd00004).w(m_ramdac, FUNC(ramdac_device::mask_w));
	map(0xd80000, 0xd80000).w(FUNC(astoneag_state::oki_bank_w));
	map(0xe00001, 0xe00001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xf80000, 0xf80001).nopw(); // ? ffff->0000 before EEPROM read
//  map(0x??0001, 0x??0001).w(FUNC(astoneag_state::screen_enable_w)); // unknown location
}

/***************************************************************************
                                Input Ports
***************************************************************************/

static INPUT_PORTS_START( showhand )
	PORT_START("INPUTS")    // 54000
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_COIN1         ) PORT_IMPULSE(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_GAMBLE_KEYOUT ) // press with memory_reset if eeprom error (green screen) or to reset settings
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_UNKNOWN       )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_GAMBLE_BET    ) PORT_NAME("Bet / Double Up")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_POKER_HOLD3   ) PORT_NAME("Look / Small") // HOLD5 in full test mode
	PORT_SERVICE_NO_TOGGLE( 0x0020,   IP_ACTIVE_LOW     ) // settings
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_UNKNOWN       )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_CUSTOM        ) PORT_READ_LINE_DEVICE_MEMBER("ticket", FUNC(hopper_device::line_r))
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_POKER_HOLD2   ) PORT_NAME("Yes / Big") // HOLD4 in full test mode
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_GAMBLE_PAYOUT ) // HOLD1 in full test mode
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_POKER_HOLD1   ) PORT_NAME("Select") // HOLD2 in full test mode
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_START         ) PORT_NAME("Start / Take")
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_MEMORY_RESET  ) // press with keyout if eeprom error (green screen) or to reset settings
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_UNKNOWN       )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_GAMBLE_KEYIN  )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_CUSTOM        ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(hopper_device::line_r))

	PORT_START( "EEPROM_IN" )
	PORT_BIT( 0xfff7, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))

	PORT_START( "EEPROM_OUT" )
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::di_write))
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::clk_write))
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::cs_write))
INPUT_PORTS_END

static INPUT_PORTS_START( showhanc )
	PORT_INCLUDE( showhand )

	PORT_MODIFY("INPUTS")    // 84000
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_COIN1         ) PORT_IMPULSE(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_GAMBLE_KEYOUT ) // press with memory_reset if eeprom error (green screen) or to reset settings
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_UNKNOWN       )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_GAMBLE_BET    ) PORT_NAME("Bet / Double Up")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_POKER_HOLD1   ) PORT_NAME("Select")
	PORT_SERVICE_NO_TOGGLE( 0x0020,   IP_ACTIVE_LOW     ) // settings
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_GAMBLE_PAYOUT ) // note: no payout with default settings
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_START         ) PORT_NAME("Start / Take") // HOLD1 in full test mode
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_UNKNOWN       )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_POKER_HOLD3   ) PORT_NAME("Look / Small") // HOLD5 in full test mode
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNKNOWN       ) // must be 0 for payout/lamps to work, might be a hopper empty sense? HOLD2 in full test mode
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_POKER_HOLD2   ) PORT_NAME("Yes / Big") // HOLD4 in full test mode
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_MEMORY_RESET  ) // press with keyout if eeprom error (green screen) or to reset settings. "DOOR OPEN" error in-game.
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNKNOWN       ) // must be 0 for payout/lamps to work, might be a hopper empty sense?
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_CUSTOM        ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(hopper_device::line_r))
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_GAMBLE_KEYIN  )
INPUT_PORTS_END

static INPUT_PORTS_START( skilldrp )
	PORT_INCLUDE( showhand )

	// Skill Drop Georgia manual, buttons layout:
	// ( START )   ( PLAY )   ( SELECT / DOUBLE )   ( TAKE )
	PORT_MODIFY("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_COIN1         ) PORT_IMPULSE(1) // 5 causes coin jam in magibomb
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_GAMBLE_TAKE   )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_GAMBLE_D_UP   ) PORT_NAME("Select / Double Up")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_UNKNOWN       )
	PORT_SERVICE_NO_TOGGLE( 0x0020,   IP_ACTIVE_LOW     )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_START         )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_OTHER         ) PORT_NAME("Ticket Out") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_UNKNOWN       )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_UNKNOWN       )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_GAMBLE_BET    ) PORT_NAME("Play")
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_CUSTOM        ) PORT_READ_LINE_DEVICE_MEMBER("ticket", FUNC(hopper_device::line_r))
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_GAMBLE_BOOK   )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_CUSTOM        ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(hopper_device::line_r))
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_GAMBLE_KEYIN  )
INPUT_PORTS_END

static INPUT_PORTS_START( magibomb )
	PORT_INCLUDE( skilldrp )

	// Magic Bomb manuals (American Alpha and AA7 versions), 36-pin layout:
	// [ ALL STOP ]   [ STOP1 / DOUBLE UP ]   [ STOP2 / ODDS ]   [ STOP3 / TAKE ]   [ START ]
	PORT_MODIFY("INPUTS")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_SLOT_STOP3    ) PORT_NAME("Stop Reel 3 / Take Score")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_SLOT_STOP1    ) PORT_NAME("Stop Reel 1 / Double Up")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_OTHER         ) PORT_NAME("Reserve Switch") // shown in test mode
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_SLOT_STOP_ALL ) PORT_NAME("Stop All Reels / Big")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_SLOT_STOP2    ) PORT_NAME("Stop Reel 2 / Small / Odds")
INPUT_PORTS_END

static INPUT_PORTS_START( magibombd )
	PORT_INCLUDE( magibomb )

	PORT_MODIFY( "EEPROM_IN" )
	PORT_BIT( 0xfffb, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))

	PORT_START( "CPUCODE_IN" )
	PORT_BIT( 0xfffb, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_READ_LINE_DEVICE_MEMBER("astro_cpucode", FUNC(astro_cpucode_device::do_read))

	PORT_MODIFY( "EEPROM_OUT" )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::cs_write))

	PORT_START( "CPUCODE_OUT" )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("astro_cpucode", FUNC(astro_cpucode_device::clk_write))
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("astro_cpucode", FUNC(astro_cpucode_device::cs_write))
INPUT_PORTS_END

static INPUT_PORTS_START( magibombg )
	PORT_INCLUDE( magibomb )

	PORT_START( "CPUCODE_IN" )
	PORT_BIT( 0xfff7, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_READ_LINE_DEVICE_MEMBER("astro_cpucode", FUNC(astro_cpucode_device::do_read))

	PORT_START( "CPUCODE_OUT" )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("astro_cpucode", FUNC(astro_cpucode_device::clk_write))
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("astro_cpucode", FUNC(astro_cpucode_device::cs_write))
INPUT_PORTS_END

static INPUT_PORTS_START( dinodino )
	PORT_INCLUDE( magibomb )

	PORT_MODIFY("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_COIN1         ) PORT_IMPULSE(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_SLOT_STOP3    ) PORT_NAME("Take Score")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_SLOT_STOP1    ) PORT_NAME("Double Up")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_OTHER         ) PORT_NAME("Reserve Switch") // shown in test mode
	PORT_SERVICE_NO_TOGGLE( 0x0020,   IP_ACTIVE_LOW     )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_START         )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_OTHER         ) PORT_NAME("Ticket Out") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_SLOT_STOP_ALL ) PORT_NAME("Auto")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_SLOT_STOP2    ) PORT_NAME("Odds")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_GAMBLE_BET    ) PORT_NAME("Play")
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_CUSTOM        ) PORT_READ_LINE_DEVICE_MEMBER("ticket", FUNC(hopper_device::line_r))
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_GAMBLE_BOOK   )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_CUSTOM        ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(hopper_device::line_r))
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_GAMBLE_KEYIN  )

	PORT_START( "CPUCODE_IN" )
	PORT_BIT( 0xfff7, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_READ_LINE_DEVICE_MEMBER("astro_cpucode", FUNC(astro_cpucode_device::do_read))

	PORT_START( "CPUCODE_OUT" )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("astro_cpucode", FUNC(astro_cpucode_device::clk_write))
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("astro_cpucode", FUNC(astro_cpucode_device::cs_write))
INPUT_PORTS_END

static INPUT_PORTS_START( winbingo )
	PORT_INCLUDE( magibombd )

	PORT_MODIFY("INPUTS")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_SLOT_STOP3    ) PORT_NAME("Take Score / Select")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_SLOT_STOP1    ) PORT_NAME("Line / Double Up")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_OTHER         ) PORT_NAME("Reserve Switch") // shown in test mode
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_SLOT_STOP_ALL ) PORT_NAME("Auto")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_SLOT_STOP2    ) PORT_NAME("Odds")
INPUT_PORTS_END

static INPUT_PORTS_START( astoneag )
	PORT_INCLUDE( dinodino )

	PORT_MODIFY("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_COIN1         ) PORT_IMPULSE(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_SLOT_STOP3    ) PORT_NAME("Take")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_SLOT_STOP1    ) PORT_NAME("Auto Play")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_GAMBLE_DOOR   )
	PORT_SERVICE_NO_TOGGLE( 0x0020,   IP_ACTIVE_LOW     )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_START         ) PORT_NAME("Start / Stop")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_BILL1         )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_SLOT_STOP_ALL ) PORT_NAME("Odds")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_SLOT_STOP2    ) PORT_NAME("Select Line / Double Up")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_GAMBLE_BET    )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_OTHER         ) PORT_NAME("Reserve Switch") // shown in test mode
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_GAMBLE_BOOK   )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_CUSTOM        ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(hopper_device::line_r))
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_GAMBLE_KEYIN  )
INPUT_PORTS_END

/* Zoo manual, 36-pin layout:
[   SKIP    ]   [     4     ]
[   NEXT    ]   [           ]
[           ]   [ DOUBLEX1  ]

[     1     ]   [     2     ]
[    ODDS   ]   [           ]
[ DOUBLE1/2 ]   [    TAKE   ]

[     3     ]   [     5     ]
[    PLAY   ]   [           ]
[  DOUBLEX2 ]   [   START   ] */

/***************************************************************************
                                Graphics Layout
***************************************************************************/

static GFXDECODE_START( gfx_astrocorp )
	GFXDECODE_ENTRY("sprites", 0, gfx_16x16x8_raw, 0, 1)
GFXDECODE_END

GFXLAYOUT_RAW(gfx_16x32x8_raw, 16, 32, 16 * 8, 16 * 32 * 8);

static GFXDECODE_START( gfx_astoneag )
	GFXDECODE_ENTRY("sprites", 0, gfx_16x32x8_raw, 0, 1)
GFXDECODE_END

/***************************************************************************
                                Machine Drivers
***************************************************************************/

void astrocorp_state::machine_start()
{
	m_lamps.resolve();
	m_screen_enable = 0;
}

void astrocorp_state::showhand(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 20_MHz_XTAL / 2); // MC68HC000FN12 (10 MHz ?)
	m_maincpu->set_addrmap(AS_PROGRAM, &astrocorp_state::showhand_map);
	m_maincpu->set_vblank_int("screen", FUNC(astrocorp_state::irq4_line_hold));

	NVRAM(config, "nvram");
	EEPROM_93C46_16BIT(config, "eeprom");

	HOPPER(config, m_ticket, attotime::from_msec(200));
	HOPPER(config, m_hopper, attotime::from_msec(200));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(26.601712_MHz_XTAL / 4, 433, 0, 320, 261, 0, 240); // ~15.354kHz Hsync, ~58.846Hz Vsync
	m_screen->set_screen_update(FUNC(astrocorp_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_astrocorp);
	PALETTE(config, m_palette).set_format(palette_device::BGR_565, 0x100);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki, 20_MHz_XTAL / 20, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0);
}

void astrocorp_state::showhanc(machine_config &config)
{
	showhand(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &astrocorp_state::showhanc_map);
}

TIMER_DEVICE_CALLBACK_MEMBER(astrocorp_state::irq_2_4_scanline_cb)
{
	int scanline = param;

	if (scanline == m_screen->visible_area().max_y + 1) // vblank-in?
		m_maincpu->set_input_line(4, HOLD_LINE); // sprites, sound, i/o

	if (scanline == 0) // vblank-out?
		m_maincpu->set_input_line(2, HOLD_LINE); // palette
}

void astrocorp_state::skilldrp(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 24_MHz_XTAL / 2); // JX-1689F1028N GRX586.V5 (12 MHz ?)
	m_maincpu->set_addrmap(AS_PROGRAM, &astrocorp_state::skilldrp_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(astrocorp_state::irq_2_4_scanline_cb), "screen", 0, 1);

	NVRAM(config, "nvram");
	EEPROM_93C46_16BIT(config, "eeprom");

	HOPPER(config, m_ticket, attotime::from_msec(200));
	HOPPER(config, m_hopper, attotime::from_msec(200));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	// TODO: verify H/VSync & pixel clock for this type of HW
	// (most likely different to compensate for the higher HRes)
	m_screen->set_raw(24_MHz_XTAL / 2, 781, 0, 512, 261, 0, 240); // double horizontal resolution
	m_screen->set_screen_update(FUNC(astrocorp_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_astrocorp);
	PALETTE(config, m_palette).set_format(palette_device::BGR_565, 0x100);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki, 24_MHz_XTAL / 24, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0);
}

void astrocorp_state::luckycoin(machine_config &config)
{
	skilldrp(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &astrocorp_state::luckycoin_map);
}

void astrocorp_state::speeddrp(machine_config &config)
{
	skilldrp(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &astrocorp_state::speeddrp_map);
}

void magibomb_state::magibomb(machine_config &config)
{
	skilldrp(config);
	m_maincpu->set_clock(80_MHz_XTAL / 5); // ASTRO V03 (16 MHz ?)
	m_maincpu->set_addrmap(AS_PROGRAM, &magibomb_state::magibomb_map);
	config.device_remove("scantimer"); // irqs soft-reset the game

	m_screen->set_raw(80_MHz_XTAL / 3 / 4, 433, 0, 320, 261, 0, 240); // standard horizontal res
}

void magibomb_state::magibombb(machine_config &config)
{
	magibomb(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &magibomb_state::magibombb_map);
}

void magibomb_state::magibombf(machine_config &config)
{
	magibomb(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &magibomb_state::magibombf_map);
}

TIMER_DEVICE_CALLBACK_MEMBER(zoo_state::irq_1_2_scanline_cb)
{
	int scanline = param;

	// NOTE: in astoneag title screen, if the IRQ levels are swapped, a timing issue causes the SP to overwrite the palette data
	//       before it's written to the RAMDAC, causing corrupt colors

	if (scanline == m_screen->visible_area().max_y + 1) // vblank-in?
		m_maincpu->set_input_line(1, HOLD_LINE); // palette

	if (scanline == 0) // vblank-out?
		m_maincpu->set_input_line(2, HOLD_LINE); // sprites, sound, i/o
}

void zoo_state::machine_start()
{
	astrocorp_state::machine_start();
	// doesn't seem to have a suitable handler for this, so keep it always enabled
	// (may be a side effect for having RAMDAC installed)
	// TODO: on initial boot this causes MAME default palette shown
	// do we need to all_black it out? Confirm after decryption is complete
	m_screen_enable = 1;
}

void zoo_state::zoo(machine_config &config)
{
	skilldrp(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &zoo_state::zoo_map);
	TIMER(config.replace(), "scantimer").configure_scanline(FUNC(zoo_state::irq_1_2_scanline_cb), "screen", 0, 1);

	m_screen->set_raw(26.824_MHz_XTAL / 4, 437, 0, 320, 261, 0, 240); // ??? ~15.345kHz Hsync, ??? ~58.795Hz Vsync

	ASTRO_CPUCODE(config, m_cpucode, 0);
}

void zoo_state::magibombd(machine_config &config)
{
	zoo(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &zoo_state::magibombd_map);
}

void zoo_state::magibombg(machine_config &config)
{
	zoo(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &zoo_state::magibombg_map);
	TIMER(config.replace(), "scantimer").configure_scanline(FUNC(zoo_state::irq_2_4_scanline_cb), "screen", 0, 1);
}

void zoo_state::magibombm(machine_config &config)
{
	magibombg(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &zoo_state::magibombm_map);
}

void zoo_state::winbingo(machine_config &config)
{
	zoo(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &zoo_state::winbingo_map);
	m_screen->set_raw(24_MHz_XTAL / 2, 781, 0, 512, 261, 0, 240); // double horizontal resolution
}

void zoo_state::hacher(machine_config &config)
{
	winbingo(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &zoo_state::hacher_map);
}

void zoo_state::dinodino(machine_config &config)
{
	winbingo(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &zoo_state::dinodino_map);

	TIMER(config.replace(), "scantimer").configure_scanline(FUNC(zoo_state::irq_2_4_scanline_cb), "screen", 0, 1);
}

void zoo_state::gostop(machine_config &config)
{
	zoo(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &zoo_state::gostop_map);

	TIMER(config.replace(), "scantimer").configure_scanline(FUNC(zoo_state::irq_2_4_scanline_cb), "screen", 0, 1);
}

void zoo_state::monkeyl(machine_config &config)
{
	winbingo(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &zoo_state::monkeyl_map);
}

void zoo_state::monkeyld(machine_config &config)
{
	monkeyl(config);
	TIMER(config.replace(), "scantimer").configure_scanline(FUNC(zoo_state::irq_2_4_scanline_cb), "screen", 0, 1);
}

void zoo_state::speedmst(machine_config &config)
{
	gostop(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &zoo_state::speedmst_map);
}

void zoo_state::wwitch(machine_config &config)
{
	winbingo(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &zoo_state::wwitch_map);
}

void zoo_state::hapfarm(machine_config &config)
{
	winbingo(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &zoo_state::hapfarm_map);
}

void astoneag_state::ramdac_map(address_map &map)
{
	map(0x000, 0x2ff).rw(m_ramdac, FUNC(ramdac_device::ramdac_pal_r), FUNC(ramdac_device::ramdac_rgb666_w));
}

void astoneag_state::astoneag(machine_config &config)
{
	winbingo(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &astoneag_state::astoneag_map);

	// 120MHz XTAL + VGA connector
	// TODO: verify H/VSync & pixel clock for this type of HW
	// (most likely different to compensate for the higher HRes and higher VRes)
	m_screen->set_raw(120_MHz_XTAL / 10 * 2, 781, 0, 512, 261*2, 0, 240*2); // double horizontal & vertical resolution (~30.730kHz Hsync ?? , ~58.869Hz Vsync ??)

	// Adds RAMDAC
	PALETTE(config.replace(), m_palette).set_entries(256);
	RAMDAC(config, m_ramdac, 0, m_palette);
	m_ramdac->set_addrmap(0, &astoneag_state::ramdac_map);

	// Tiles are double size vertically
	GFXDECODE(config.replace(), m_gfxdecode, m_palette, gfx_astoneag);
}

/***************************************************************************
                                ROMs Loading
***************************************************************************/

/***************************************************************************

Show Hand (Italy)
(C) 2000? Astro Corp.

PCB CHE-B50-4002A 88 94V-0 0002

CPU     1x MC68HC000FN12 (main)
        1x pLSI1016-60LJ (main)
        1x ASTRO V01 0005 (custom)
        1x AD-65 (equivalent to OKI6295)(sound)
        1x ASTRO 0001B (custom)
        1x oscillator 20.000MHz
        1x oscillator 26.601712MHz (6x PAL subcarrier)

ROMs    2x 27C512 (1,2)
        2x M27C801 (3,4)
        1x M27C4001 (5)
        1x Atmel 93C46 (not dumped)

Note    1x 28x2 JAMMA edge connector
        1x 18x2 edge connector
        1x 10x2 edge connector
        1x pushbutton
        1x trimmer (volume)
        1x 2x2 dip switches (JAMMA 10/18/28)

Hardware info by f205v

***************************************************************************/

ROM_START( showhand )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1-8.even.u16", 0x00000, 0x10000, CRC(cf34bf0d) SHA1(72ad7ca63ef89451b2572d64cccfa764b9d9b353) )
	ROM_LOAD16_BYTE( "2-8.odd.u17",  0x00001, 0x10000, CRC(dd031c36) SHA1(198d0e685dd2d824a04c787f8a17c173efa272d9) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD16_BYTE( "4.even.u26", 0x000000, 0x100000, CRC(8a706e42) SHA1(989688ee3a5e4fc11fb502e43c9d6012488982ee) )
	ROM_LOAD16_BYTE( "3.odd.u26",  0x000001, 0x100000, CRC(a624b750) SHA1(fc5b09f8a10cba5fb2474e1edd62a0400177a5ad) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "5", 0x00000, 0x80000, CRC(e6987122) SHA1(fb3e7c2399057c64b5c496a393f6f22a1e54c844) )

	ROM_REGION16_LE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46.u9", 0x00, 0x80, CRC(1583a53e) SHA1(e4ba380e8cce64fba94dd3f9506ae43f37d11440) ) // factory default
ROM_END

/***************************************************************************

Show Hand (China) - Wángpái Duìjué (王牌对决)
Astro Corp, 199?

PCB Layout
----------
CHE-B50-4002A
|----------------------------------------|
|     LATTICE    JAMMA   SW   VOL UPC1242|
|     PLSI1016           U26             |
|                6264                 U43|
|                        U25    M6295    |
|     68000                              |
|                U17    |-----|          |
|                       |ASTRO|          |
|                U16    |V01  |          |
|                       |-----|MDT2020AP/3V
|                6264           20MHz    |
|DSW(2)                      26.601712MHz|
|93C46           6116  6116    KM681000  |
|BATTERY         6116  6116    KM681000  |
|----------------------------------------|
Notes:
      68000 clock - 10.000MHz [20/2]
      M6295 clock - 1.000MHz [20/20], pin 7 HIGH
      VSync - 58.846Hz
      HSync - 15.354kHz

Hardware info by Guru

***************************************************************************/

ROM_START( showhanc )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1even.u16", 0x00000, 0x10000, CRC(d1295bdb) SHA1(bb035ee89b21368fb11c3b9cd23164b68feb84bd) )
	ROM_LOAD16_BYTE( "2odd.u17",  0x00001, 0x10000, CRC(bbca78e7) SHA1(a163569acad8d6b8821602ce24013fc46887aba9) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD16_BYTE( "4even.u26", 0x00000, 0x100000, CRC(285375e0) SHA1(63b47105f0751c65e528139074f5b342450495ba) )
	ROM_LOAD16_BYTE( "3odd.u25",  0x00001, 0x100000, CRC(b93e3a91) SHA1(5192375c32518532e08bddfe000efdee587e1ecc) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "5.u43", 0x00000, 0x80000, CRC(d6b70f02) SHA1(5a94680594c1f06196fe3bcf7faf56e2ed576f01) )

	ROM_REGION16_LE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46.u9", 0x00, 0x80, CRC(f0c30fdc) SHA1(48f6f964e096e8b9f675f84400a5c99b92964c3d) ) // factory default
ROM_END

/***************************************************************************

Skill Drop Georgia

"Sep 13 2002 09:17:54" in code rom and AAI276832 on sticker.

No specific PCB model or number....

Astro V02 0022 160pin PQFP ("ASTRO02" silkscreened under chip)
JX-1689F1028N GRC586.V5 (68K core, has direct connection to program roms)
Lattice IspLSI 1016 60LJ socketed FPGA
OKI 6295 clone chip (AD-65 or U6295)

EEPROM  Atmel 93C46
Battery 3.6V
OSC     24.000MHz

PC1 is a push button for test mode
VR1 is for sound volume

      +---------+   +----------------------------+ +----+
  +---+Connector+---+   28 Pin Edge  Connector   +-+    |
  |                                                  VR1|
  |                                                     |
+-+             +------+                                |
|      ULN2003A |IspLSI| UT6264CPC     ROM#4*           |
|  ULN2003A     | 1016 |             +---------+        |
|8              +------++----------+ |ROM#6 U26|        |
|                       |ROM#2  U20| +---------+  +----+|
|L        +-------+     +----------+   ROM#3*     |6295||
|i        | JX-   |                               +----+|
|n        | 1689F | +--------------+                    |
|e        | 1028N | |  ROM#7 U100  |                    |
|r        +-------+ +--------------+                    |
|                                    +----------+   +---+
|C                      +----------+ |          |   |   |
|o                      |ROM#1  U21| |  Astro   |   | R |
|n                      +----------+ |  V02     |   | O |
|n                                   |  0022    |   | M |
|e                                   +----------+   | # |
|c                       UT6264CPC                  | 5 |
|t                                                  |   |
|o                                                  +---+
|r     93C46              6116      6116      RAM1      |
|                                                  24MHz|
+-+ BAT1 PC1              6116      6116      RAM1      |
  +-----------------------------------------------------+

ROM#1 & ROM#2 are 32pin sockets
ROM#7 is a 40pin socket
ROM#3 & ROM#4 at U27 & U25 are optional unpopulated 32pin sockets and overlap with ROM#6
ROM#6 is 29F1610MC flash rom

RAM1 are SEC KM681000BLG-7L RAM chips

***************************************************************************/

ROM_START( skilldrp )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "7-skill_drop_g1.01s.u100", 0x00000, 0x40000, CRC(8867df18) SHA1(19ad0104647b6f5c8b6c06749c24defdcacfd54d) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "mx29f1610amc.u26", 0x000000, 0x200000, CRC(4fdac800) SHA1(bcafceb6c34866c474714347e23f9e819b5fcfa6) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "5-skill_drop.rom5", 0x00000, 0x80000, CRC(a479e06d) SHA1(ee690d39188b8a43652c4aa5bf8267c1f6632d2f) ) // No chip location just "ROM#5" silkscreened under socket

	ROM_REGION16_LE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46.u6", 0x00, 0x80, CRC(01c4bc62) SHA1(49710d2dac73791b4019b1dc15e0b5159c6fbaef) ) // factory default
ROM_END

ROM_START( skilldrpa )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "7-skill_drop_g1.0s.u100", 0x00000, 0x40000, CRC(f968b783) SHA1(1d693b1d460e659ca94aae8625ea26e120053f84) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "mx29f1610amc.u26", 0x000000, 0x200000, CRC(4fdac800) SHA1(bcafceb6c34866c474714347e23f9e819b5fcfa6) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "5-skill_drop.rom5", 0x00000, 0x80000, CRC(a479e06d) SHA1(ee690d39188b8a43652c4aa5bf8267c1f6632d2f) ) // No chip location just "ROM#5" silkscreened under socket

	ROM_REGION16_LE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46.u6", 0x00, 0x80, CRC(01c4bc62) SHA1(49710d2dac73791b4019b1dc15e0b5159c6fbaef) ) // factory default
ROM_END

ROM_START( luckycoin )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "u100", 0x00000, 0x40000, CRC(77bbeebc) SHA1(45f5a18694e2a93d9c299dc1f405df32c9773ce6) ) // label was peeled off

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "mx29f1610amc.u26", 0x000000, 0x200000, CRC(9e6184b7) SHA1(23014b32d129e39c3956c36516d3920fd6e2e11b) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "5-lucky_coin.rom5", 0x00000, 0x80000, CRC(a479e06d) SHA1(ee690d39188b8a43652c4aa5bf8267c1f6632d2f) ) // No chip location just "ROM#5" silkscreened under socket

	ROM_REGION16_LE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46.u6", 0x00, 0x80, CRC(01c4bc62) SHA1(49710d2dac73791b4019b1dc15e0b5159c6fbaef) ) // factory default
ROM_END

/***************************************************************************

Speed Drop

"16:01:26 Sep 3 2003" in code

No specific PCB model or number, same as used for Skill Drop but with newer video chip

Astro V05 0206 160pin PQFP ("ASTRO02" silkscreened under chip)
JX-1689HP TA5265188 (68K core, has direct connection to program roms)
Lattice IspLSI 1016 60LJ socketed FPGA
OKI 6295 clone chip (AD-65 or U6295)

EEPROM  Atmel 93C46
Battery 3.6V
OSC     24.000MHz

PC1 is a push button for test mode
VR1 is for sound volume

      +---------+   +----------------------------+ +----+
  +---+Connector+---+   28 Pin Edge  Connector   +-+    |
  |                                                  VR1|
  |                                                     |
+-+             +------+                                |
|      ULN2003A |IspLSI| UT6264CPC     ROM#4*           |
|  ULN2003A     | 1016 |             +---------+        |
|8              +------++----------+ |ROM#6 U26|        |
|                       |ROM#2  U20| +---------+  +----+|
|L        +-------+     +----------+   ROM#3*     |6295||
|i        | JX-   |                               +----+|
|n        | 1689HP| +--------------+                    |
|e        |       | |  ROM#7 U100  |                    |
|r        +-------+ +--------------+                    |
|                                    +----------+   +---+
|C                      +----------+ |          |   |   |
|o                      |ROM#1  U21| |  Astro   |   | R |
|n                      +----------+ |  V05     |   | O |
|n                                   |  0206    |   | M |
|e                                   +----------+   | # |
|c                       UT6264CPC                  | 5 |
|t                                                  |   |
|o                                                  +---+
|r     93C46              6116      6116      RAM1      |
|                                                  24MHz|
+-+ BAT1 PC1              6116      6116      RAM1      |
  +-----------------------------------------------------+

ROM#1 & ROM#2 are 32pin sockets
ROM#7 is a 40pin socket
ROM#3 & ROM#4 at U27 & U25 are optional unpopulated 32pin sockets and overlap with ROM#6
ROM#6 is 29F1610MC flash rom

RAM1 are SEC KM681000BLG-7L RAM chips

1 SPEED DROP 1.06 is a WinBond W27C512
2 SPEED DROP 1.06 is a WinBond W27C512
5 SPEED DROP is ST 27C4001

***************************************************************************/

ROM_START( speeddrp )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1_speed_drop_1.06.u21", 0x00000, 0x10000, CRC(ff4d0859) SHA1(abdb90d3498f64a9ac779f5fd66d313c1df3425b) )
	ROM_LOAD16_BYTE( "2_speed_drop_1.06.u20", 0x00001, 0x10000, CRC(a00cc120) SHA1(eb1e9a084aca18e71901ed599f4621c301bab43e) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "mx29f1610amc.u26", 0x000000, 0x200000, CRC(baa0f728) SHA1(12f0e7689eb6555f86ac9a7272e8e119faa968e0) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "5_speed_drop.rom5", 0x00000, 0x80000, CRC(684bb8b5) SHA1(65276ce03da7be7275646f5a0d9d163eecb78190) ) // No chip location just "ROM#5" silkscreened under socket

	ROM_REGION16_LE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46.u6", 0x00, 0x80, CRC(6890534e) SHA1(a62893015e53c02551d57d0e1cce436b6df8d289) ) // factory default
ROM_END

/***************************************************************************

Magic Bomb

***************************************************************************/

ROM_START( magibomb )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rom1", 0x00000, 0x10000, CRC(f74596fe) SHA1(8311ca73c975bda6846e1ba958fcf62655a111d0) )
	ROM_LOAD16_BYTE( "rom2", 0x00001, 0x10000, CRC(ca0b693d) SHA1(27fc7db9d3f19d6dcb8756e4fc9c3839116977fe) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "gfx", 0x000000, 0x200000, BAD_DUMP CRC(042f7992) SHA1(2e175994d0b14200a92bdb46e82847b1a1c88265) ) // dumped for the Ver. A3.1 set, should be the same for most sets, marking as bad as precaution

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "rom5", 0x00000, 0x80000, CRC(c9edbf1b) SHA1(8e3a96a38aea23950d6add66a5a3d079013bc217) )

	ROM_REGION16_LE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46.u6", 0x00, 0x80, CRC(28644b9e) SHA1(3d63892bf5207145039ae7994552e2ab67cd8293) ) // factory default
ROM_END

ROM_START( magibomba )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rom1", 0x00000, 0x10000, BAD_DUMP CRC(f01ab462) SHA1(5c9052f66da166f926910562a6a8aea5397549db) ) // BADADDR xxxxxx-xxxxxxxxx
	ROM_LOAD16_BYTE( "rom2", 0x00001, 0x10000, CRC(ac3224ef) SHA1(810117dc89369eee0f4f5b6744cfbf0cb70ccce6) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "gfx", 0x000000, 0x200000, BAD_DUMP CRC(042f7992) SHA1(2e175994d0b14200a92bdb46e82847b1a1c88265) ) // dumped for the Ver. A3.1 set, should be the same for most sets, marking as bad as precaution

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "rom5", 0x00000, 0x80000, CRC(f7d14414) SHA1(af932df09aa970ec05cc12e590e152e7288c1f5c) )

	ROM_REGION16_LE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46.u6", 0x00, 0x80, BAD_DUMP CRC(28644b9e) SHA1(3d63892bf5207145039ae7994552e2ab67cd8293) ) // factory default from parent (this set doesn't work)
ROM_END

ROM_START( magibombb )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rom1", 0x00000, 0x10000, CRC(ac69612f) SHA1(0f1f3a05f90d4c0e65bc2b68c6510d2aa95f68c4) )
	ROM_LOAD16_BYTE( "rom2", 0x00001, 0x10000, CRC(55ea8303) SHA1(974bc026f0b284ffad50d3e441613952db768e04) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "gfx", 0x000000, 0x200000, BAD_DUMP CRC(042f7992) SHA1(2e175994d0b14200a92bdb46e82847b1a1c88265) ) // dumped for the Ver. A3.1 set, should be the same for most sets, marking as bad as precaution

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "rom5", 0x00000, 0x80000, CRC(c9edbf1b) SHA1(8e3a96a38aea23950d6add66a5a3d079013bc217) )

	ROM_REGION16_LE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46.u6", 0x00, 0x80, CRC(53bb180a) SHA1(8a2b7ae3abf31a1972864cf96e1ac74ed69fb1ee) ) // factory default
ROM_END

ROM_START( magibombc )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rom1", 0x00000, 0x10000, CRC(e388b79d) SHA1(ca3a6ffee39f3ac629f29fdd8ee98ddcf0787f9e) )
	ROM_LOAD16_BYTE( "rom2", 0x00001, 0x10000, CRC(3d2abaa5) SHA1(8a39abda8e52a0d8a8d0c54a45b72a7d9f624e84) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "gfx", 0x000000, 0x200000, BAD_DUMP CRC(042f7992) SHA1(2e175994d0b14200a92bdb46e82847b1a1c88265) ) // dumped for the Ver. A3.1 set, should be the same for most sets, marking as bad as precaution

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "rom5", 0x00000, 0x80000, CRC(c9edbf1b) SHA1(8e3a96a38aea23950d6add66a5a3d079013bc217) )

	ROM_REGION16_LE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46.u6", 0x00, 0x80, CRC(e7d335e8) SHA1(cda263e6012985f17765a449b0614615c35efcfe) ) // factory default
ROM_END

ROM_START( magibombd )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rom1", 0x00000, 0x10000, CRC(91883755) SHA1(bc6dd5cac352f169beac790825e86a2ecb45af05) )
	ROM_LOAD16_BYTE( "rom2", 0x00001, 0x10000, CRC(b55bc270) SHA1(7486018b87708cf5b653c27b1ed824e441c169b7) )
	ROM_FILL(                0x20000, 0x20000, 0xff )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "mx29f1610ml.u26", 0x000000, 0x200000, CRC(c120f256) SHA1(f7468fb73b2f7453c950256880c0b8db24849a4e) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "rom5", 0x00000, 0x80000, CRC(c9edbf1b) SHA1(8e3a96a38aea23950d6add66a5a3d079013bc217) )

	ROM_REGION16_LE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46.u6", 0x00, 0x80, CRC(7278d581) SHA1(c0851466b669e42c657ca5517c00c44d76e559f4) ) // factory default

	ROM_REGION16_LE( 0x02, "astro_cpucode", 0 )
	ROM_LOAD( "magibombd_cpucode.key", 0x00, 0x02, CRC(ee980d67) SHA1(f3bdb8a14701ec01828f7c92f18e9bba4c56a4e0) )
ROM_END

ROM_START( magibombe ) // no Bin Laden GFX, supports 8 and 16 min bet
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rom1", 0x00000, 0x10000, CRC(d564fac4) SHA1(ac236a5760be6c4f518a8eb2af708f433e0fb899) )
	ROM_LOAD16_BYTE( "rom2", 0x00001, 0x10000, CRC(ee212839) SHA1(e6175b27f3c510e98bc85e04be2fdde6e0289dfb) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "gfx", 0x000000, 0x200000, CRC(042f7992) SHA1(2e175994d0b14200a92bdb46e82847b1a1c88265) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "rom5", 0x00000, 0x80000, CRC(c9edbf1b) SHA1(8e3a96a38aea23950d6add66a5a3d079013bc217) )

	ROM_REGION16_LE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46.u6", 0x00, 0x80, CRC(2e38cfad) SHA1(0f2490ca2ba738723b4c014a4fec4f631167f786) ) // factory default
ROM_END

ROM_START( magibombf )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rom1.u21", 0x00000, 0x10000, CRC(bc9a9c68) SHA1(cee6d20322ba500f82321fa56ca71c8ec152b953) )
	ROM_LOAD16_BYTE( "rom2.u20", 0x00001, 0x10000, CRC(b52bfa4d) SHA1(e413536867148967f4ddbf1cf81c4cf45da41d1e) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "29f1610mc.u26", 0x000000, 0x200000, BAD_DUMP CRC(042f7992) SHA1(2e175994d0b14200a92bdb46e82847b1a1c88265) ) // dumped for the Ver. A3.1 set, should be the same for most sets, marking as bad as precaution

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "rom5.u33", 0x00000, 0x80000, CRC(c9edbf1b) SHA1(8e3a96a38aea23950d6add66a5a3d079013bc217) )

	ROM_REGION16_LE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46.u6", 0x00, 0x80, CRC(a01e5ce7) SHA1(77213c0b426ca1806bb8a4c55e9e7f4f2db66962) ) // factory default
ROM_END

ROM_START( magibombg )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1 magic bomb nb.6.1.u20", 0x00000, 0x10000, CRC(8aaa14c3) SHA1(44ade91596b8cd907c7556f3c5f977e2c7b4029d) )
	ROM_LOAD16_BYTE( "2 magic bomb nb.6.1.u19", 0x00001, 0x10000, CRC(d371c3ab) SHA1(3f63883ad2735a772e9dbc958346b692420667dc) )
	ROM_FILL(                                   0x20000, 0x20000, 0xff )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "mx29f1610mc.u26", 0x000000, 0x200000, BAD_DUMP CRC(042f7992) SHA1(2e175994d0b14200a92bdb46e82847b1a1c88265) ) // wasn't dumped, different from all the other sets

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "5 magic bomb", 0x00000, 0x80000, CRC(c9edbf1b) SHA1(8e3a96a38aea23950d6add66a5a3d079013bc217) )

	ROM_REGION16_LE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46.u10", 0x00, 0x80, BAD_DUMP CRC(a01e5ce7) SHA1(77213c0b426ca1806bb8a4c55e9e7f4f2db66962)  ) // TODO: this is correct for magibombf, create good one

	ROM_REGION16_LE( 0x02, "astro_cpucode", ROMREGION_ERASE00 )
	ROM_LOAD( "magibombg_cpucode.key", 0x00, 0x02, NO_DUMP ) // TODO: RE correct one
ROM_END

ROM_START( magibombh )// min bet 1 8 16 32, minimum percentage % 91/50%
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rom1.u21", 0x00000, 0x10000, CRC(9bc790f5) SHA1(f75c6378a0067013556bf6e63cfa28475dfbe8f4) )
	ROM_LOAD16_BYTE( "rom2.u20", 0x00001, 0x10000, CRC(b8ff0c2d) SHA1(422ea7578dbe1093ea886621ebefad0cc8e74eb4) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "29f1610mc.u26", 0x000000, 0x200000, BAD_DUMP CRC(042f7992) SHA1(2e175994d0b14200a92bdb46e82847b1a1c88265) ) // dumped for the Ver. A3.1 set, should be the same for most sets, marking as bad as precaution

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "rom5.u33", 0x00000, 0x80000, CRC(c9edbf1b) SHA1(8e3a96a38aea23950d6add66a5a3d079013bc217) )

	ROM_REGION16_LE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46.u6", 0x00, 0x80, CRC(845be081) SHA1(af42f41148cc28c2b6717cff44bb90caaa088184) ) // factory default
ROM_END

ROM_START( magibombi )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rom1.u21", 0x00000, 0x10000, CRC(9e7201df) SHA1(b4439639d3c0827172ffe1a801df9b9f3330711e) )
	ROM_LOAD16_BYTE( "rom2.u20", 0x00001, 0x10000, CRC(0880e66d) SHA1(cef642ced8718a9cbe12b3e4c087debfa9591a9b) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "29f1610mc.u26", 0x000000, 0x200000, BAD_DUMP CRC(042f7992) SHA1(2e175994d0b14200a92bdb46e82847b1a1c88265) ) // dumped for the Ver. A3.1 set, should be the same for most sets, marking as bad as precaution

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "rom5.u33", 0x00000, 0x80000, CRC(c9edbf1b) SHA1(8e3a96a38aea23950d6add66a5a3d079013bc217) )

	ROM_REGION16_LE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46.u6", 0x00, 0x80, CRC(4ea5fdb1) SHA1(0c1aefc517587ee7456d158d3614201da34beb14) ) // factory default
ROM_END

ROM_START( magibombj )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1_mb_aa72c.u26", 0x00000, 0x10000, CRC(7faa57c3) SHA1(6176848354f90b819d4ffe2d91cdd9cca36ff58f) )
	ROM_LOAD16_BYTE( "2_mb_aa72c.u25", 0x00001, 0x10000, CRC(987c3e4e) SHA1(d2ffac771dc1d30f0598c79cad0691753e4945f7) )
	ROM_FILL(                          0x20000, 0x20000, 0xff )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "mx29f1610ml.u26", 0x000000, 0x200000, CRC(c120f256) SHA1(f7468fb73b2f7453c950256880c0b8db24849a4e) ) // the "U26" marking is present twice

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "5_mb_e1.0", 0x00000, 0x80000, CRC(c9edbf1b) SHA1(8e3a96a38aea23950d6add66a5a3d079013bc217) )

	ROM_REGION16_LE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46.u6", 0x00, 0x80, CRC(7278d581) SHA1(c0851466b669e42c657ca5517c00c44d76e559f4) ) // factory default

	ROM_REGION16_LE( 0x02, "astro_cpucode", 0 )
	ROM_LOAD( "magibombj_cpucode.key", 0x00, 0x02, CRC(ee980d67) SHA1(f3bdb8a14701ec01828f7c92f18e9bba4c56a4e0) )
ROM_END

ROM_START( magibombk ) // min bet 8 16 32 64, minimum percentage % 91/50%, no Bin Laden GFX
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1_magic_bomb_a3.0.u21", 0x00000, 0x10000, CRC(c9f3c9da) SHA1(ef23a954e4ec1de7a570561ed00715957aea93e3) )
	ROM_LOAD16_BYTE( "2_magic_bomb_a3.0.u20", 0x00001, 0x10000, CRC(18d90060) SHA1(90d5a1d496538b1d28bf676cd3ac3f77c62668f7) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "gfx", 0x000000, 0x200000, CRC(042f7992) SHA1(2e175994d0b14200a92bdb46e82847b1a1c88265) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "5_magic_bomb_e1.0", 0x00000, 0x80000, CRC(c9edbf1b) SHA1(8e3a96a38aea23950d6add66a5a3d079013bc217) )

	ROM_REGION16_LE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46.u6", 0x00, 0x80, CRC(2e38cfad) SHA1(0f2490ca2ba738723b4c014a4fec4f631167f786) ) // factory default
ROM_END

ROM_START( magibombl ) // MIN BET 1-XXX, MINIMUM % = 91/50%, OSAMA BIN LADEN GFX
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1_magic_bomb_ab4.3.u21", 0x00000, 0x10000, CRC(0ca10f98) SHA1(ed1e051998893c1dafca8d8d317fcfad2bf4dd58) )
	ROM_LOAD16_BYTE( "2_magic_bomb_ab4.3.u20", 0x00001, 0x10000, CRC(b9e9f385) SHA1(ad58215d5afa390a3d6ae6f73b5be2c9905e6eac) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "gfx", 0x000000, 0x200000, CRC(042f7992) SHA1(2e175994d0b14200a92bdb46e82847b1a1c88265) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "5_magic_bomb_e1.0", 0x00000, 0x80000, CRC(c9edbf1b) SHA1(8e3a96a38aea23950d6add66a5a3d079013bc217) )

	ROM_REGION16_LE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46.u6", 0x00, 0x80, CRC(53bb180a) SHA1(8a2b7ae3abf31a1972864cf96e1ac74ed69fb1ee) ) // factory default
ROM_END

ROM_START( magibombm )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "2_m._b._ab5.3.u22", 0x00000, 0x10000, CRC(e568681a) SHA1(ec55de2ebfb2b10e1873cc11ec4f47e17dd2fffa) )
	ROM_LOAD16_BYTE( "1_m._b._ab5.3.u23", 0x00001, 0x10000, CRC(dfb63c3d) SHA1(0f0bd5cccf5ab08003bfb751355304386682bbd0) )
	ROM_FILL(                             0x20000, 0x20000, 0xff )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "gfx", 0x000000, 0x200000, BAD_DUMP CRC(042f7992) SHA1(2e175994d0b14200a92bdb46e82847b1a1c88265) ) // not correct for this set

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "5_m._b._e1.0.u44", 0x00000, 0x80000, CRC(c9edbf1b) SHA1(8e3a96a38aea23950d6add66a5a3d079013bc217) )

	ROM_REGION16_LE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46", 0x00, 0x80, CRC(e717146d) SHA1(beca9d9cece337b05f77881073a4da7ddb9659da) ) // factory default

	ROM_REGION16_LE( 0x02, "astro_cpucode", 0 )
	ROM_LOAD( "magibombg_cpucode.key", 0x00, 0x02, NO_DUMP ) // TODO: RE correct one
ROM_END

ROM_START( magibombn ) // MIN BET 1-8-16-32 MINIMUM % = 91/50%
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1_magic_bomb_eb4.0.u21", 0x00000, 0x10000, CRC(8341c745) SHA1(9680d6907eef05e7ce0a45ffcbaf45adf9d47c6d) )
	ROM_LOAD16_BYTE( "2_magic_bomb_eb4.0.u20", 0x00001, 0x10000, CRC(0e47f8fa) SHA1(dac2a071b21768482206a5058aac88f943375287) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "gfx", 0x000000, 0x200000, CRC(042f7992) SHA1(2e175994d0b14200a92bdb46e82847b1a1c88265) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "5_magic_bomb_e1.0", 0x00000, 0x80000, CRC(c9edbf1b) SHA1(8e3a96a38aea23950d6add66a5a3d079013bc217) )

	ROM_REGION16_LE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46.u6", 0x00, 0x80, CRC(59306190) SHA1(c4e6f3070da90c447786e7bc769b37bdc0fa3b77) ) // factory default
ROM_END


/***************************************************************************

Western Venture
(c) ASTRO

ASTRO V07 0709
ASTRO F01 2007-06-03
ASTRO ??? (V102?)
AD-65?
93C46 EEPROM

XTAL 24 MHz (near F01)
XTAL 18.432 MHz (near empty 44 pin PLCC socket and RS232?)

Push Button

ROM types:

EV29001TSC-70R
MX29F1610MC-10

***************************************************************************/

ROM_START( westvent )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1_w.v.aa.02.d.bin", 0x00000, 0x20000, CRC(5e208192) SHA1(5a35a419fe95513b68423d4eb6c77fdd375667f3) )  // good?
	ROM_LOAD16_BYTE( "2_w.v.aa.02.d.bin", 0x00001, 0x20000, BAD_DUMP CRC(f56d3ead) SHA1(2cf8960eab221cfce1c7ac6a20d002c4b05d8cc6) ) // FIXED BITS (xxxxxx0xxxxxxxxx)

	ROM_REGION( 0x600000, "sprites", 0 )
	ROM_LOAD( "mx29f1610mc_bottom.bin", 0x000000, 0x200000, CRC(e2dd58d5) SHA1(9ab881cfb2ee6cbc48aa28ba28529adb00803e44) ) // no U location on the PCB
	ROM_LOAD( "mx29f1610mc_middle.u51", 0x200000, 0x200000, CRC(7348fd37) SHA1(b5ec0994afb5bceae5627c37f1b35c7abcfd2f0a) )
	ROM_LOAD( "mx29f1610mc_top.u30",    0x400000, 0x200000, CRC(75bbaae0) SHA1(ef35775dd481ff343df1ee071ccd52b024d084b7) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "5.bin", 0x00000, 0x80000, CRC(92dc09d1) SHA1(6b448b3372e78047d054c5e42fcfcff7f75ff9b9) )

	ROM_REGION16_LE( 0x80, "eeprom", 0 )
	ROM_LOAD( "at93c46.bin", 0x00, 0x80, CRC(fd961d46) SHA1(e238da65e8769575f17b4464fb00f5c4813bafab) )

	ROM_REGION16_LE( 0x02, "astro_cpucode", 0 )
	ROM_LOAD( "westvent_cpucode.key", 0x00, 0x02, NO_DUMP )
ROM_END

/***************************************************************************

Win Win Bingo

ASTRO M1.2 PCB:
 Astro V06 0430 160pin PQFP ("ASTRO02" silkscreened under chip)
   Boards are known to have either Astro V06 0430 or Astro V07 0610
 Astro V102PX-006 at U10 (68K core, has direct connection to program roms)
 Astro F02 2005-09-17 socketed FPGA type chip (used for encryption?)
 OKI 6295 clone chip (AD-65 or U6295)

RAM:
 2 x HMI HM62C64P-70
 4 x HMI HM6116P-70
 2 x BSI BS62LV1023SC-70

EEPROM 93C46
Battery 3.6V
OSC    24MHz

PC1 is a push button for test mode
VR1 is for sound volume

      +---------+   +----------------------------+ +----+
  +---+Connector+---+   28 Pin Edge  Connector   +-+    |
  |                                                  VR1|
  |                    24MHz        +---------+         |
+-+          +-------+              |ROM#4 U30|         |
|            | Astro |   HM62C64P   +---------+         |
|            |  F2   | +----------+ +---------+         |
|8           |       | |ROM#2  U25| |ROM#3 U26|         |
|   ULN2003A +-------+ +----------+ +---------+         |
|L                                                      |
|i  ULN2003A +------+                                   |
|n           |Astro |      ROM#7*                 +----+|
|e           |V102PX|                             |6295||
|r           +------+                             +----+|
|                                    +----------+       |
|C                     +----------+  |          |   +---+
|o                     |ROM#1  U26|  |  Astro   |   |   |
|n                     +----------+  |  V06     |   | R |
|n                                   |  0430    |   | O |
|e                                   +----------+   | M |
|c                       HM62C64P                   | # |
|t                                               J1 | 5 |
|o            93C46                                 |   |
|r                        6116      6116      RAM1  +---+
|                                                       |
+-+ BAT1 PC1              6116      6116      RAM1      |
  +-----------------------------------------------------+

ROM#7 at U16 is an unpopulated 40pin socket
ROM#1 & ROM#2 are SYNCMOS F29C51001T
ROM#3 is a 29F1610MC flash rom
ROM#4 is a 29F1610MC flash rom (optionally populated based on game)
ROM#5 is a MX 27C4000PC-12

RAM1 are BSI BS62LV1023SC-70 RAM chips

J1 is an 2 pin connector, unknown purpose

***************************************************************************/

ROM_START( winbingo )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "f29c51001t.u31", 0x00000, 0x20000, CRC(964cfdb5) SHA1(01109466e07f5e906be300bc69310171d34f2e6c) )
	ROM_LOAD16_BYTE( "f29c51001t.u25", 0x00001, 0x20000, CRC(4ebeec72) SHA1(c603265e6319cff94a0c75017a12c6d86787f906) )

	ROM_REGION( 0x1000000, "sprites", 0 )
	ROM_LOAD( "mx29f1610mc.u26", 0x000000, 0x200000, CRC(ad1f61e7) SHA1(845aa01d49c50bcadaed16d76c0dd9131a425b46) )
	ROM_LOAD( "mx29f1610mc.u30", 0x200000, 0x200000, CRC(31613d99) SHA1(1c720f8d981c3e9cb9d9b3b27eb95e7f72ccfc93) ) // girl animation at 1ff0 in u26 continues into u30, it needs this rom here
	ROM_RELOAD(                  0x400000, 0x200000 )
	ROM_RELOAD(                  0x600000, 0x200000 )
	ROM_RELOAD(                  0x800000, 0x200000 ) // u30 tiles addressed as 8xxx also
	ROM_RELOAD(                  0xa00000, 0x200000 )
	ROM_RELOAD(                  0xc00000, 0x200000 )
	ROM_RELOAD(                  0xe00000, 0x200000 )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "mx27c4000pc.u35", 0x00000, 0x80000, CRC(445d81c0) SHA1(cacb9c262740c31ea42f406e9f960a1edd1b3ead) )

	ROM_REGION16_LE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46.u13", 0x00, 0x80, CRC(a8c974b1) SHA1(4a42aeded6296881bc343179863e515230ed5fc5) ) // factory default

	ROM_REGION16_LE( 0x02, "astro_cpucode", 0 )
	ROM_LOAD( "winbingo_cpucode.key", 0x00, 0x02, CRC(66035e7e) SHA1(d8696eeeb66e4050635141ed987c9347db408cfb) )
ROM_END

ROM_START( winbingoa )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "f29c51001t.u31", 0x00000, 0x20000, CRC(c33676c3) SHA1(9f5b7d05d187cf59948a572f80c55cb8fa1f656f) ) // sldh
	ROM_LOAD16_BYTE( "f29c51001t.u25", 0x00001, 0x20000, CRC(43c7b2d8) SHA1(16ee79c34b7c485dfccecdf3e0ae9f18f8a20150) ) // sldh

	ROM_REGION( 0x1000000, "sprites", 0 )
	ROM_LOAD( "mx29f1610mc.u26", 0x000000, 0x200000, BAD_DUMP CRC(ad1f61e7) SHA1(845aa01d49c50bcadaed16d76c0dd9131a425b46) )
	ROM_LOAD( "mx29f1610mc.u30", 0x200000, 0x200000, BAD_DUMP CRC(31613d99) SHA1(1c720f8d981c3e9cb9d9b3b27eb95e7f72ccfc93) ) // girl animation at 1ff0 in u26 continues into u30, it needs this rom here
	ROM_RELOAD(                  0x400000, 0x200000 )
	ROM_RELOAD(                  0x600000, 0x200000 )
	ROM_RELOAD(                  0x800000, 0x200000 ) // u30 tiles addressed as 8xxx also
	ROM_RELOAD(                  0xa00000, 0x200000 )
	ROM_RELOAD(                  0xc00000, 0x200000 )
	ROM_RELOAD(                  0xe00000, 0x200000 )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "mx27c4000pc.u35", 0x00000, 0x80000, CRC(e48ed57d) SHA1(11995b90e70e010b292ba9db2da0af4ebf795c1a) ) // sldh

	ROM_REGION16_LE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46.u13", 0x00, 0x80, CRC(a8c974b1) SHA1(4a42aeded6296881bc343179863e515230ed5fc5) ) // factory default

	ROM_REGION16_LE( 0x02, "astro_cpucode", 0 )
	ROM_LOAD( "winbingoa_cpucode.key", 0x00, 0x02, CRC(6bd252c3) SHA1(8d1ad9546795e81573df1c79f7be28cf41fd04e0) )
ROM_END

/***************************************************************************

Hacher (graphics hack of Win Win Bingo Ver. EN.01.6)

ASTRO M1.2 PCB with Astro F02 2005-02-18

***************************************************************************/

ROM_START( hacher )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "w-w-en-01-6.rom1", 0x00000, 0x20000, CRC(994acd32) SHA1(ee137ca96f4e2d22f2bae32051bbf2bd487e8c5a) )
	ROM_LOAD16_BYTE( "w-w-en-01-6.rom2", 0x00001, 0x20000, CRC(b45c3f64) SHA1(c8f26fc3f9e2c46d8083d249f79ff8a3d47b67d0) )

	ROM_REGION( 0x1000000, "sprites", 0 )
	ROM_LOAD( "wb3.bin", 0x000000, 0x200000, CRC(d97e5056) SHA1(662fefc2dcac31023fa063fbf891b05a139e48d8) )
	ROM_LOAD( "wb4.bin", 0x800000, 0x200000, BAD_DUMP CRC(5cd7dcd9) SHA1(69e5fd0c8c5c14938c02f4f50e5b16fc0fbff7e4) ) // FIXED BITS (xxxxxxxxxxxxx1xx)
	ROM_RELOAD(                    0x400000, 0x200000 )
	ROM_RELOAD(                    0x600000, 0x200000 )
	ROM_RELOAD(                    0x800000, 0x200000 ) // wb4 tiles addressed as 8xxx also
	ROM_RELOAD(                    0xa00000, 0x200000 )
	ROM_RELOAD(                    0xc00000, 0x200000 )
	ROM_RELOAD(                    0xe00000, 0x200000 )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "win-win-bingo.ic5", 0x00000, 0x80000, CRC(445d81c0) SHA1(cacb9c262740c31ea42f406e9f960a1edd1b3ead) ) // = mx27c4000pc.u35 winbingo

	ROM_REGION16_LE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46.u13", 0x00, 0x80, CRC(6c776ff8) SHA1(5f48b0ac657df41a747718ef0f3b14715c40cb4a) ) // factory default

	ROM_REGION16_LE( 0x02, "astro_cpucode", 0 )
	ROM_LOAD( "hacher_cpucode.key", 0x00, 0x02, CRC(6fdfa275) SHA1(d22c5fcaf182ccb78ec486a9f1d47ad0d1c63466) )
ROM_END

/***************************************************************************

Zoo by Astro

ASTRO M1.1 PCB:
 Astro V06 0430 160pin PQFP ("ASTRO02" silkscreened under chip)
 Astro V102PX-005 T042652846 at U10 (68K core, has direct connection to program roms)
 Astro F02 2005-02-18 socketed FPGA type chip (used for encryption?)
 OKI 6295 clone chip (AD-65 or U6295)

RAM:
 2 x KTC KT76C88-70LL
 4 x HM6116L-70
 2 x BSI BS62LV1025SC-70

EEPROM 93C46
Battery 3.6V
OSC    26.824MHz

PC1 is a push button for test mode
VR1 is for sound volume


ZOO ZO.02.D at both U25 & U26 are Winbond W27C512 roms and are program code
5 ZOO is a MX 27C4000 rom and is the sample rom
29F1610mc at U26 (yes "U26" is present twice on the PCB) are the graphics

      +---------+   +----------------------------+ +----+
  +---+Connector+---+   28 Pin Edge  Connector   +-+    |
  |                                                  VR1|
  |                   26.824MHz     +---------+         |
+-+          +-------+              |ROM#4 U30|         |
|            | Astro |   KT76C88    +---------+         |
|            |  F2   | +----------+ +---------+         |
|8           |       | |ROM#2  U25| |ROM#3 U26|         |
|   ULN2003A +-------+ +----------+ +---------+         |
|L                                                      |
|i  ULN2003A +------+                                   |
|n           |Astro |      ROM#7*                 +----+|
|e           |V102PX|                             |6295||
|r           +------+                             +----+|
|                                    +----------+       |
|C                     +----------+  |          |   +---+
|o                     |ROM#1  U26|  |  Astro   |   |   |
|n                     +----------+  |  V06     |   | R |
|n                                   |  0430    |   | O |
|e                                   +----------+   | M |
|c                       KT76C88                    | # |
|t                                               J1 | 5 |
|o            93C46                                 |   |
|r                        6116      6116      RAM1  +---+
|                                                       |
+-+ BAT1 PC1              6116      6116      RAM1      |
  +-----------------------------------------------------+

ROM#7 at U16 is an unpopulated 40pin socket
ROM#3 is a 29F1610MC flash rom
ROM#4 is a 29F1610MC flash rom (optionally populated based on game)

RAM1 are BSI BS62LV1025SC-70 RAM chips

J1 is an 2 pin connector, unknown purpose

***************************************************************************/

ROM_START( zoo )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "zoo_zo.02.d.u26", 0x00000, 0x10000, CRC(1a3be45a) SHA1(877be4c9e8d5e7c4644e7bcb9a6729443ed772a4) )
	ROM_LOAD16_BYTE( "zoo_zo.02.d.u25", 0x00001, 0x10000, CRC(8566aa21) SHA1(319192e2074f3bdda6001d8e9a4b97e98826d7ce) )
	ROM_FILL(                           0x20000, 0x20000, 0xff )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "29f1610mc.u26", 0x000000, 0x200000, CRC(f5cfd915) SHA1(ec869b47d0762102509dcfc1349d94340037fad5) ) // the "U26" marking is present twice (should have been U31?)

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "5_zoo", 0x00000, 0x80000, CRC(b0c9f7aa) SHA1(99345ba0f8da3907f26c9bd29d70135f3ab7cd60) )

	ROM_REGION16_LE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46.u13", 0x00, 0x80, CRC(2af3fab5) SHA1(7097e735eb1beda8aff12f783d4039b4b9aab651) ) // factory default

	ROM_REGION16_LE( 0x02, "astro_cpucode", 0 )
	ROM_LOAD( "zoo_cpucode.key", 0x00, 0x02, CRC(cd3fdba4) SHA1(33d33bf711f96e18a2fc515689853f92cd71713d) )
ROM_END

/***************************************************************************

Stone Age
(c) ASTRO

PCB ID  CPU                Video        Chips
L1      ASTRO V102PX-012?  ASTRO V05x2  ASTRO F02 2004-09-04

      +---------+   +----------------------------+ +-------------+
  +---+Connector+---+   28 Pin Edge  Connector   +-+             |
  |                                                   VR1        |
  |   93c46                                                      |
+-+          +-------+                          +----+ +-------+ |
|            | Astro |   HM62C64P               |6295| |ROM #7 | |
|            |  F2   | +----------+             +----+ +-------+ |
|8           |       | |ROM#2  U19|                              |
|   ULN2003A +-------+ +----------+      +--+ +--+ +--+ +--+     |
|L                                       | R| | R| | R| | R|     |
|i  ULN2003A +------+                    | o| | o| | o| | o|     |
|n           |Astro |      ROM#8*        | m| | m| | m| | m|     |
|e           |V102PX|                    | #| | #| | #| | #|     |
|r           +------+                    | 3| | 4| | 5| | 6|     |
|                                        +--+ +--+ +--+ +--+     |
|                                                                |
|                                      +----------+  +----------+|
|C                     +----------+    |          |  |          ||
|o                     |ROM#1  U21|    |  Astro   |  |  Astro   ||
|n                     +----------+    |  V05     |  |  V05     ||
|n                                     |  0424    |  |  0424    ||
|e                                     +----------+  +----------+|
|c                       HM62C64P                                |
|t                                                               |
|o        ASTRO0312     ASTRO  120Mhz                            |
|r                                    6116                       |
|                  6116      6116     6116   RAM1       RAM1     |
|                                                                |
+-+ BAT1 PC1  VGA  6116      6116              RAM1     RAM1     |
  +--------------------------------------------------------------+

ROM#7 at U16 is an unpopulated 40pin socket
ROM#1 & ROM#2 are MX 26c10000VPC-10
ROM#3,4,5,6 are a 29F1610ML flash rom
ROM#7 is a MX 27C4000PC-12

RAM1 are SEC KM681000BLG-7 RAM chips
PC1 is reset
Video output only on VGA connector, video signals ARE VGA

***************************************************************************/

ROM_START( astoneag )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1-s-a-eng-03-a.rom1", 0x00000, 0x20000, CRC(5e600713) SHA1(48ac0a52f90b972b77064e9e59711082aa95c654) )
	ROM_LOAD16_BYTE( "2-s-a-eng-03-a.rom2", 0x00001, 0x20000, CRC(488e355e) SHA1(6550292cae7eda95a24e1982e869540464b1fcdd) )

	ROM_REGION( 0x2000000, "sprites", 0 ) // 16 x 32 tiles !!
	ROM_LOAD16_BYTE( "29f1610.rom4", 0x0000000, 0x200000, CRC(1affd8db) SHA1(2523f156933c61d36b6646944b5da874f8424864) )
	ROM_LOAD16_BYTE( "29f1610.rom5", 0x0000001, 0x200000, CRC(2b77d827) SHA1(b082254e1c8a7945e2a406b1b937a763b30cb496) )
	ROM_LOAD16_BYTE( "29f1610.rom3", 0x1000000, 0x200000, CRC(8d4e66f0) SHA1(744f83b35684aa6653b0d93b303f2914cd0250ba) )
	ROM_LOAD16_BYTE( "29f1610.rom6", 0x1000001, 0x200000, CRC(eb8ee0e7) SHA1(c6c973460ca96b54151f7523f6afc0184b8fbd40) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "5-s-a-eng-03-a.rom7", 0x00000, 0x80000, CRC(1b13b0c2) SHA1(d6d8c8070ba146b444958fa0b896cebc12b32f5c) )

	ROM_REGION16_LE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46.u99", 0x00, 0x80, CRC(be7cb115) SHA1(0f2eb141e84785b565b387cd52a1594bc876fc8d) ) // factory default

	ROM_REGION16_LE( 0x02, "astro_cpucode", 0 )
	ROM_LOAD( "astoneag_cpucode.key", 0x00, 0x02, CRC(fd3642ce) SHA1(8b4e630e114f36c9c15b1a6e6185156786ec9502) )
ROM_END

/***************************************************************************

Dino Dino
Astro Corp.

PCB Layout
----------

ASTRO T-3802A PCB with ASTRO F02 2003-03-12
|--------------------------------------------|
|        |------|                    TDA2003 |
|ULN2003 |ASTRO |   V62C51864            VOL |
|        |F02   |            ROM4            |
|        |------|   ROM2                     |
|ULN2003                     ROM3      M6295 |
|        |-------|                           |
|8       |ASTRO  |                           |
|L       |V102PX |                       ROM5|
|I       |-010   |                           |
|N       |-------|            |------|       |
|E                            |ASTRO |       |
|R                  ROM1      |V05   |       |
|                             |------|       |
|                   V62C51864                |
|                                       24MHz|
|        93C46      6116 6116    HM628128    |
|        SW1        6116 6116    HM628128    |
|--------------------------------------------|

***************************************************************************/

ROM_START( dinodino )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "dd_rom1.u20", 0x00000, 0x20000, CRC(056613dd) SHA1(e8481177b1dacda222fe4fae2b50841ddb0c87ba) )
	ROM_LOAD16_BYTE( "dd_rom2.u19", 0x00001, 0x20000, CRC(575519c5) SHA1(249fe33b6ea0bc154125a988315f571a30b9375c) )

	ROM_REGION( 0x1000000, "sprites", 0 )
	ROM_LOAD( "dd_rom3.u26", 0x000000, 0x200000, CRC(47c95b43) SHA1(43e9a13c38f2f7d13dd4dcb105c65e43b18ccdbf) )
	ROM_LOAD( "dd_rom4.u24", 0x200000, 0x200000, CRC(2cf4be21) SHA1(831d7d125c4161b42b017a69fc05e30a51172620) ) // rotating cherries uses 1fff as tile code for 1 frame, they need this rom here
	ROM_RELOAD(              0x400000, 0x200000 )
	ROM_RELOAD(              0x600000, 0x200000 )
	ROM_RELOAD(              0x800000, 0x200000 ) // rom4 tiles addressed as 8xxx also
	ROM_RELOAD(              0xa00000, 0x200000 )
	ROM_RELOAD(              0xc00000, 0x200000 )
	ROM_RELOAD(              0xe00000, 0x200000 )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "dd_rom5.u33", 0x00000, 0x80000, CRC(482e456a) SHA1(c7111522383c4e1fd98b0f759153be98dcbe06c1) )

	ROM_REGION16_LE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46.u10", 0x0000, 0x0080, CRC(29fe0e1d) SHA1(9694d7a7ad401d0fec879c183a28b3aa1ff02407) ) // factory default

	ROM_REGION16_LE( 0x02, "astro_cpucode", 0 )
	ROM_LOAD( "dinodino_cpucode.key", 0x00, 0x02, CRC(674b2687) SHA1(9c1338a41162fa2ffbd2ea0a579ad87a6c084a0e) )
ROM_END

/***************************************************************************

Go Stop
Astro Corp.

***************************************************************************/

ROM_START( gostopac )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "go stop en1.0 1.u20", 0x00000, 0x10000, CRC(288eb9e2) SHA1(cb1181a3ff260374d786fa3b967e3200cdc545c5) )
	ROM_LOAD16_BYTE( "go stop en1.0 2.u19", 0x00001, 0x10000, CRC(6cee46c2) SHA1(f63914264fbe9c3124f7e6e1fbd6fb1a72314de9) )
	ROM_FILL(                               0x20000, 0x20000, 0xff )

	ROM_REGION( 0x1000000, "sprites", 0 )
	ROM_LOAD( "u26", 0x000000, 0x200000, CRC(f8ca0d7b) SHA1(d023521b58f2550a069ae45392f8a7e4959a355b) )
	ROM_LOAD( "u24", 0x200000, 0x200000, CRC(51892a3f) SHA1(5839a12262a603d4672cc2ab9b4951f7092b5a53) )
	ROM_RELOAD(      0x400000, 0x200000 )
	ROM_RELOAD(      0x600000, 0x200000 )
	ROM_RELOAD(      0x800000, 0x200000 )
	ROM_RELOAD(      0xa00000, 0x200000 )
	ROM_RELOAD(      0xc00000, 0x200000 )
	ROM_RELOAD(      0xe00000, 0x200000 )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "go stop 3.u33", 0x00000, 0x80000, CRC(3d519d59) SHA1(58f0fb42724da75c56eae37ffe47c7e0caf87ee4) )

	ROM_REGION16_LE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46.u10", 0x0000, 0x0080, CRC(c5bfafa4) SHA1(b482fad6445487a5c14c4360c7b3b0b02f16944c) ) // TODO: factory default

	ROM_REGION16_LE( 0x02, "astro_cpucode", 0 )
	ROM_LOAD( "gostopac_cpucode.key", 0x00, 0x02, NO_DUMP )
ROM_END

/***************************************************************************

Monkey Land
Astro Corp.

***************************************************************************/

ROM_START( monkeyl )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1_m.l._aa.21.a.u26", 0x00000, 0x20000, CRC(2286237f) SHA1(cbf6589fe3eedd9482f8ba01c4510a5699085e69) ) // F29C51001T
	ROM_LOAD16_BYTE( "1_m.l._aa.21.a.u25", 0x00001, 0x20000, CRC(0612f893) SHA1(6300d9defc75b1b09fad31f719c841c728fe3ff6) ) // F29C51001T

	ROM_REGION( 0x1000000, "sprites", 0 )
	ROM_LOAD( "mx29f1610mc.u26", 0x000000, 0x200000, CRC(1ec8b75d) SHA1(53f501126c81bdc8ccd8c44ed9f82078156c48ff) ) // the "U26" marking is present twice (should have been U31?)
	ROM_LOAD( "mx29f1610mc.u30", 0x200000, 0x200000, CRC(9d26fa05) SHA1(571f5515411e504bf7661b8b5d358dc6c55e6fbe) )
	ROM_RELOAD(      0x400000, 0x200000 )
	ROM_RELOAD(      0x600000, 0x200000 )
	ROM_RELOAD(      0x800000, 0x200000 )
	ROM_RELOAD(      0xa00000, 0x200000 )
	ROM_RELOAD(      0xc00000, 0x200000 )
	ROM_RELOAD(      0xe00000, 0x200000 )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "5_m.l._e1.0.u33", 0x00000, 0x80000, CRC(62122100) SHA1(519df7825ab62f0648192e6b1760dd9cc5ec7f9f) )

	ROM_REGION16_LE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46.u10", 0x0000, 0x0080, CRC(eab965cc) SHA1(258358c89faaf643b526d1014946e14567cba88d) ) // TODO: factory default

	ROM_REGION16_LE( 0x02, "astro_cpucode", 0 )
	ROM_LOAD( "monkeyl_cpucode.key", 0x00, 0x02, NO_DUMP )
ROM_END

ROM_START( monkeyla ) // MIN BET 1-XXX, % = LEVELS 1-8
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1_m.l._aa.13.b.u26", 0x00000, 0x20000, CRC(0ff56936) SHA1(70f9cf3b6b71514b2401b7a516ae85ad86b5321f) ) // F29C51001T
	ROM_LOAD16_BYTE( "1_m.l._aa.13.b.u25", 0x00001, 0x20000, CRC(d261b6ed) SHA1(347feb51b67cd0e0faeff98fa023b3f4a8cb9c32) ) // F29C51001T

	ROM_REGION( 0x1000000, "sprites", 0 )
	ROM_LOAD( "mx29f1610mc.u26", 0x000000, 0x200000, CRC(1ec8b75d) SHA1(53f501126c81bdc8ccd8c44ed9f82078156c48ff) ) // the "U26" marking is present twice (should have been U31?)
	ROM_LOAD( "mx29f1610mc.u30", 0x200000, 0x200000, CRC(9d26fa05) SHA1(571f5515411e504bf7661b8b5d358dc6c55e6fbe) )
	ROM_RELOAD(      0x400000, 0x200000 )
	ROM_RELOAD(      0x600000, 0x200000 )
	ROM_RELOAD(      0x800000, 0x200000 )
	ROM_RELOAD(      0xa00000, 0x200000 )
	ROM_RELOAD(      0xc00000, 0x200000 )
	ROM_RELOAD(      0xe00000, 0x200000 )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "5_m.l._e1.0.u33", 0x00000, 0x80000, CRC(62122100) SHA1(519df7825ab62f0648192e6b1760dd9cc5ec7f9f) )

	ROM_REGION16_LE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46.u10", 0x0000, 0x0080, CRC(28e861d6) SHA1(4faa4d62954fd9a263d24caa6214353a109ec4f1) ) // TODO: factory default

	ROM_REGION16_LE( 0x02, "astro_cpucode", 0 )
	ROM_LOAD( "monkeyla_cpucode.key", 0x00, 0x02, NO_DUMP )
ROM_END

ROM_START( monkeylb )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "f29c51001t.u26", 0x00000, 0x20000, CRC(bae5b055) SHA1(742f6aa7b66abdcfe140fb777aecb5388e9f8bdf) )
	ROM_LOAD16_BYTE( "f29c51001t.u25", 0x00001, 0x20000, CRC(06362b30) SHA1(a73bc2bc217ecae6ed48ec30481f88767058837b) )

	ROM_REGION( 0x1000000, "sprites", 0 )
	ROM_LOAD( "mx29f1610mc.u26", 0x000000, 0x200000, CRC(1ec8b75d) SHA1(53f501126c81bdc8ccd8c44ed9f82078156c48ff) ) // the "U26" marking is present twice (should have been U31?)
	ROM_LOAD( "mx29f1610mc.u30", 0x200000, 0x200000, CRC(9d26fa05) SHA1(571f5515411e504bf7661b8b5d358dc6c55e6fbe) )
	ROM_RELOAD(      0x400000, 0x200000 )
	ROM_RELOAD(      0x600000, 0x200000 )
	ROM_RELOAD(      0x800000, 0x200000 )
	ROM_RELOAD(      0xa00000, 0x200000 )
	ROM_RELOAD(      0xc00000, 0x200000 )
	ROM_RELOAD(      0xe00000, 0x200000 )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "5_m.l._e1.0.u33", 0x00000, 0x80000, CRC(62122100) SHA1(519df7825ab62f0648192e6b1760dd9cc5ec7f9f) )

	ROM_REGION16_LE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46.u10", 0x0000, 0x0080, CRC(eab965cc) SHA1(258358c89faaf643b526d1014946e14567cba88d) ) // TODO: factory default

	ROM_REGION16_LE( 0x02, "astro_cpucode", 0 )
	ROM_LOAD( "monkeylb_cpucode.key", 0x00, 0x02, NO_DUMP )
ROM_END

ROM_START( monkeylc )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1_m.l._aa.21.c.u26", 0x00000, 0x20000, CRC(482303fd) SHA1(494df379dd4436546e2ce03194ac2f214a91bd92) ) // F29C51001T
	ROM_LOAD16_BYTE( "1_m.l._aa.21.c.u25", 0x00001, 0x20000, CRC(ccb3392e) SHA1(2ea301ae857f4360012f3adfc96b66319503a033) ) // F29C51001T

	ROM_REGION( 0x1000000, "sprites", 0 )
	ROM_LOAD( "mx29f1610mc.u26", 0x000000, 0x200000, CRC(1ec8b75d) SHA1(53f501126c81bdc8ccd8c44ed9f82078156c48ff) ) // the "U26" marking is present twice (should have been U31?)
	ROM_LOAD( "mx29f1610mc.u30", 0x200000, 0x200000, CRC(9d26fa05) SHA1(571f5515411e504bf7661b8b5d358dc6c55e6fbe) )
	ROM_RELOAD(      0x400000, 0x200000 )
	ROM_RELOAD(      0x600000, 0x200000 )
	ROM_RELOAD(      0x800000, 0x200000 )
	ROM_RELOAD(      0xa00000, 0x200000 )
	ROM_RELOAD(      0xc00000, 0x200000 )
	ROM_RELOAD(      0xe00000, 0x200000 )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "5_monkey_land.u33", 0x00000, 0x80000, CRC(62122100) SHA1(519df7825ab62f0648192e6b1760dd9cc5ec7f9f) )

	ROM_REGION16_LE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46.u10", 0x0000, 0x0080, CRC(eab965cc) SHA1(258358c89faaf643b526d1014946e14567cba88d) ) // TODO: factory default

	ROM_REGION16_LE( 0x02, "astro_cpucode", 0 )
	ROM_LOAD( "monkeylc_cpucode.key", 0x00, 0x02, NO_DUMP )
ROM_END

ROM_START( monkeyld )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "w27e010.u20", 0x00000, 0x20000, CRC(4a5c78bc) SHA1(e310605ee102eee706c9e672ca722fa5c6163f39) )
	ROM_LOAD16_BYTE( "w27e010.u19", 0x00001, 0x20000, CRC(ac37c3be) SHA1(489d4aafdd3228b096988090da9b712a9c08b74c) )

	ROM_REGION( 0x1000000, "sprites", 0 ) // not dumped for this set
	ROM_LOAD( "mx29f1610mc.u26", 0x000000, 0x200000, BAD_DUMP CRC(1ec8b75d) SHA1(53f501126c81bdc8ccd8c44ed9f82078156c48ff) )
	ROM_LOAD( "mx29f1610mc.u30", 0x200000, 0x200000, BAD_DUMP CRC(9d26fa05) SHA1(571f5515411e504bf7661b8b5d358dc6c55e6fbe) )
	ROM_RELOAD(      0x400000, 0x200000 )
	ROM_RELOAD(      0x600000, 0x200000 )
	ROM_RELOAD(      0x800000, 0x200000 )
	ROM_RELOAD(      0xa00000, 0x200000 )
	ROM_RELOAD(      0xc00000, 0x200000 )
	ROM_RELOAD(      0xe00000, 0x200000 )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "mx27c4000.rom5", 0x00000, 0x80000, CRC(10971c7f) SHA1(9de24e1585c1505dd07679dcb4eed94972fb09b7) )

	ROM_REGION16_LE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46.u10", 0x0000, 0x0080, CRC(eab965cc) SHA1(258358c89faaf643b526d1014946e14567cba88d) ) // TODO: factory default

	ROM_REGION16_LE( 0x02, "astro_cpucode", 0 )
	ROM_LOAD( "monkeyld_cpucode.key", 0x00, 0x02, NO_DUMP )
ROM_END


/***************************************************************************

Speed Master
D2 Enterprises

***************************************************************************/

ROM_START( speedmst )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1_s.m._d2.01.c.u23", 0x00000, 0x10000, CRC(f98cdc5f) SHA1(59088d649ec47c94469de3d409d75ab29ebdd4ec) ) // 27C512
	ROM_LOAD16_BYTE( "2_s.m._d2.01.c.u22", 0x00001, 0x10000, CRC(f5017c91) SHA1(fa5e4888ff819dbaa5f10f089c6de2f32b8888ac) ) // 27C512
	ROM_FILL(                              0x20000, 0x20000, 0xff )

	ROM_REGION( 0x1000000, "sprites", 0 )
	ROM_LOAD16_WORD_SWAP( "mx29f1610ml.u31", 0x000000, 0x200000, BAD_DUMP CRC(70b33a01) SHA1(d8330c5ff0a47127d00bad313963becf6bc92280) ) // chip wouldn't read, taken from other sets
	ROM_RELOAD(                              0x200000, 0x200000 )
	ROM_RELOAD(                              0x400000, 0x200000 )
	ROM_RELOAD(                              0x600000, 0x200000 )
	ROM_RELOAD(                              0x800000, 0x200000 )
	ROM_RELOAD(                              0xa00000, 0x200000 )
	ROM_RELOAD(                              0xc00000, 0x200000 )
	ROM_RELOAD(                              0xe00000, 0x200000 )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "5_speed_master.u44", 0x00000, 0x80000, CRC(5eacf25c) SHA1(affbf8176b6cbc406a281793c0a9c79e7175942d) ) // 27C4000

	ROM_REGION16_LE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46.u9", 0x0000, 0x0080, CRC(5e03d141) SHA1(d475bd0509fb977d38f012c4eb1e9cb88c3d130a) )

	ROM_REGION16_LE( 0x02, "astro_cpucode", 0 )
	ROM_LOAD( "speedmst_cpucode.key", 0x00, 0x02, NO_DUMP )
ROM_END

ROM_START( speedmsta )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "speed_master_1.u23", 0x00000, 0x10000, CRC(c99200af) SHA1(0573ebc4bd866850aa6c16a0b9db7d7229865dbf) ) // 27C512
	ROM_LOAD16_BYTE( "speed_master_2.u22", 0x00001, 0x10000, CRC(041ff0de) SHA1(63b129605d1c330836fdd5fa8a654f5d20170f0d) ) // 27C512
	ROM_FILL(                              0x20000, 0x20000, 0xff )

	ROM_REGION( 0x1000000, "sprites", 0 )
	ROM_LOAD16_WORD_SWAP( "mx29f1610ml.u31", 0x000000, 0x200000, CRC(70b33a01) SHA1(d8330c5ff0a47127d00bad313963becf6bc92280) )
	ROM_RELOAD(                              0x200000, 0x200000 )
	ROM_RELOAD(                              0x400000, 0x200000 )
	ROM_RELOAD(                              0x600000, 0x200000 )
	ROM_RELOAD(                              0x800000, 0x200000 )
	ROM_RELOAD(                              0xa00000, 0x200000 )
	ROM_RELOAD(                              0xc00000, 0x200000 )
	ROM_RELOAD(                              0xe00000, 0x200000 )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "5_speed_master.u44", 0x00000, 0x80000, CRC(d2c7025f) SHA1(3337216811c8b81ce72cfdd1ed9eecd4b0622dc7) ) // SLDH, 27C4001

	ROM_REGION16_LE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46.u9", 0x0000, 0x0080, BAD_DUMP CRC(5e03d141) SHA1(d475bd0509fb977d38f012c4eb1e9cb88c3d130a) ) // dumped for the D2.01.C set

	ROM_REGION16_LE( 0x02, "astro_cpucode", 0 )
	ROM_LOAD( "speedmsta_cpucode.key", 0x00, 0x02, NO_DUMP )
ROM_END

ROM_START( speedmstb )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1_speed_master_d1.6.u23", 0x00000, 0x10000, CRC(beb8bcb8) SHA1(a06f4cb151ae6e80a4d5335a774be06d2c1feb2b) ) // 27C512
	ROM_LOAD16_BYTE( "2_speed_master_d1.6.u22", 0x00001, 0x10000, CRC(2a0912fd) SHA1(e83bb6f131ea0d6dd5dc8e71e21a76b1f1ba1f09) ) // 27C512
	ROM_FILL(                              0x20000, 0x20000, 0xff )

	ROM_REGION( 0x1000000, "sprites", 0 )
	ROM_LOAD16_WORD_SWAP( "mx29f1610ml.u31", 0x000000, 0x200000, CRC(70b33a01) SHA1(d8330c5ff0a47127d00bad313963becf6bc92280) )
	ROM_RELOAD(                              0x200000, 0x200000 )
	ROM_RELOAD(                              0x400000, 0x200000 )
	ROM_RELOAD(                              0x600000, 0x200000 )
	ROM_RELOAD(                              0x800000, 0x200000 )
	ROM_RELOAD(                              0xa00000, 0x200000 )
	ROM_RELOAD(                              0xc00000, 0x200000 )
	ROM_RELOAD(                              0xe00000, 0x200000 )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "5_speed_master.u44", 0x00000, 0x80000, CRC(d2c7025f) SHA1(3337216811c8b81ce72cfdd1ed9eecd4b0622dc7) ) // 27C040

	ROM_REGION16_LE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46.u9", 0x0000, 0x0080, BAD_DUMP CRC(5e03d141) SHA1(d475bd0509fb977d38f012c4eb1e9cb88c3d130a) ) // dumped for the D2.01.C set

	ROM_REGION16_LE( 0x02, "astro_cpucode", 0 )
	ROM_LOAD( "speedmstb_cpucode.key", 0x00, 0x02, NO_DUMP )
ROM_END

/***************************************************************************

Captain Shark
Astro Corp. / American Alpha

***************************************************************************/

ROM_START( cptshark ) // clearly based on Win Win Bingo, still has strings for it
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1_c.s._cs.01.6.u31", 0x00000, 0x20000, CRC(87eb4151) SHA1(600154bde858a185d18db9a2a2cb58284622e905) ) // F29C51001T
	ROM_LOAD16_BYTE( "2_c.s._cs.01.6.u25", 0x00001, 0x20000, CRC(ed59d9b5) SHA1(30ac8a988e8b80463b1e67614a784c75582bcf16) ) // F29C51001T

	ROM_REGION( 0x1000000, "sprites", 0 )
	ROM_LOAD( "mx29f1610mc.u26", 0x000000, 0x200000, CRC(ed034ac1) SHA1(7f26d81e65cb4519018ee01f247b91cd711cba4f) )
	ROM_LOAD( "mx29f1610mc.u30", 0x200000, 0x200000, CRC(b21423a2) SHA1(106eea71a0f7ed58342b180f976093d8da477608) )
	ROM_RELOAD(                  0x400000, 0x200000 )
	ROM_RELOAD(                  0x600000, 0x200000 )
	ROM_RELOAD(                  0x800000, 0x200000 )
	ROM_RELOAD(                  0xa00000, 0x200000 )
	ROM_RELOAD(                  0xc00000, 0x200000 )
	ROM_RELOAD(                  0xe00000, 0x200000 )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "5_captain_shark.u35", 0x00000, 0x80000, CRC(e48ed57d) SHA1(11995b90e70e010b292ba9db2da0af4ebf795c1a) ) // 27C040, same as winbingoa

	ROM_REGION16_LE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46.u13", 0x00, 0x80, CRC(d2f41598) SHA1(3876553e5cfc5b8c8383e1dc828a0f393bb15e22) ) // TODO: factory default

	ROM_REGION16_LE( 0x02, "astro_cpucode", 0 )
	ROM_LOAD( "cptshark_cpucode.key", 0x00, 0x02, CRC(c38c8e25) SHA1(7e866fc75f4ddbbd6efbbd359f2b378c798e2cec) )
ROM_END

/***************************************************************************

Wicked Witch
Astro Corp.

***************************************************************************/

ROM_START( wwitch )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1_wicked_w_aa.01.a.u26", 0x00000, 0x20000, CRC(6c654105) SHA1(b07e807864d7d0ccb80111369f8dc205fe45aea4) ) // F29C51001T
	ROM_LOAD16_BYTE( "2_wicked_w_aa.01.a.u25", 0x00001, 0x20000, CRC(66206bb4) SHA1(80513d7dc7cd664238f01a6c1b3e40e7696e2211) ) // F29C51001T

	ROM_REGION( 0x600000, "sprites", 0 )
	ROM_LOAD( "mx29f1610mc.bin", 0x000000, 0x200000, CRC(8dad2fc0) SHA1(88c4bda8e247839029a8c9a84d3bd598892b1775) ) // no U location on the PCB, silkscreened 'ROM # 7' on PCB under the chip
	ROM_LOAD( "mx29f1610mc.u30", 0x200000, 0x200000, CRC(d4e7b00d) SHA1(2689d19fcdd828d0d47265362f6625377a90c1e4) ) // silkscreened 'ROM # 4' on PCB under the chip
	ROM_LOAD( "mx29f1610mc.u51", 0x400000, 0x200000, CRC(05bc898d) SHA1(c88c14e4858943b2ea719abe0cc9ac0738d682dd) ) // silkscreened 'ROM # 3' on PCB under the chip

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "5_wicked_w_aa.01.a.bin", 0x00000, 0x80000, CRC(298014f3) SHA1(98da24e84ec69c48e8754f8406e11e68fb352e28) )

	ROM_REGION16_LE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46.u13", 0x00, 0x80, CRC(663f14cd) SHA1(8a675a4e270d86d3a350e7ddbeb3f9d958798bd1) )

	ROM_REGION16_LE( 0x02, "astro_cpucode", 0 )
	ROM_LOAD( "wwitch_cpucode.key", 0x00, 0x02, NO_DUMP )
ROM_END

/***************************************************************************

Happy Farm
Astro Corp.

***************************************************************************/

ROM_START( hapfarm ) // also has Hot Runner N3.00 string, so probably derived from that code base
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1_happy_farm_us.01.02.b.u26", 0x00000, 0x20000, CRC(c60bbc39) SHA1(fcf23c9c75bc221a310b87de58dea75f018065c2) ) // EV29001TSC-70R read as F29C51001T
	ROM_LOAD16_BYTE( "2_happy_farm_us.01.02.b.u25", 0x00001, 0x20000, CRC(186a3e0f) SHA1(8e2474beb9264dbadc154cdfdc7b911cc1a3a601) ) // EV29001TSC-70R read as F29C51001T

	ROM_REGION( 0x600000, "sprites", 0 )
	ROM_LOAD( "mx29f1610mc.bin", 0x000000, 0x200000, CRC(fea5629e) SHA1(cbee315f7264dc6f3dd2a973cf948854138827e3) ) // no U location on the PCB, silkscreened 'ROM # 7' on PCB under the chip
	ROM_LOAD( "mx29f1610mc.u30", 0x200000, 0x200000, CRC(97af8968) SHA1(344834f7fae193e9f505d451cb6fc7240e5e84b9) ) // silkscreened 'ROM # 4' on PCB under the chip
	ROM_LOAD( "mx29f1610mc.u51", 0x400000, 0x200000, CRC(42faf0e8) SHA1(de596978c9a57d7667b78755ea111972857b6313) ) // silkscreened 'ROM # 3' on PCB under the chip

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "5_happy_farm.bin", 0x00000, 0x40000, CRC(859a6086) SHA1(2821ae6f86df8a3bbff0f348cf6a36c1c3fec68a) ) // 0xxxxxxxxxxxxxxxxxx = 0xFF
	ROM_CONTINUE(                 0x00000, 0x40000 )

	ROM_REGION16_LE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46.u13", 0x00, 0x80, CRC(c11f2b48) SHA1(174681a01b30b036e948a90313add8e7de9aef55) )

	ROM_REGION16_LE( 0x02, "astro_cpucode", 0 )
	ROM_LOAD( "hapfarm_cpucode.key", 0x00, 0x02, NO_DUMP )
ROM_END

void astrocorp_state::init_showhand()
{
#if 0
	u16 * const rom = (u16*)memregion("maincpu")->base();

//  rom[0x0a1a/2] = 0x6000; // prevent hopper jam
//  rom[0x6e7a/2] = 0x8000; // fix hopper jam when payout sensor type set to "normal low" (buggy code). Requires matching polarity of the hopper.

	rom[0x1498/2] = 0x149a; // enable full test mode (has garbled graphics in i/o test, monitor screen unreachable?)

	rom[0x12f6/2] = 0x6000; // rom error
	rom[0x4916/2] = 0x6000; // rom error
#endif

	m_can_flip_sprites = false;
}

void astrocorp_state::init_showhanc()
{
#if 0
	u16 * const rom = (u16*)memregion("maincpu")->base();

	rom[0x14d8/2] = 0x14da; // enable full test mode (has garbled graphics in i/o test, monitor and image screens unreachable?)

	rom[0x139c/2] = 0x6000; // rom error
#endif

	m_can_flip_sprites = false;
}

// Simpler encryption (fixed bitswap, no xor)

void magibomb_state::init_magibomb()
{
	const u32 size = memregion("maincpu")->bytes();
	u8 * const rom = memregion("maincpu")->base();

	std::unique_ptr<u8[]> tmp = std::make_unique<u8[]>(size);

	// decrypt data
	std::copy_n(rom, size, &tmp[0]);
	for (u32 i = 1; i < size; i += 2)
		tmp[i] = bitswap<8>(rom[i], 6, 7, 5, 0, 4, 1, 3, 2);

	// descramble addresses
	for (u32 i = 0; i < size; i++)
	{
		u32 dest = (i & ~0xffff) | bitswap<16>(i, 14,15,13,8,12,9,11,10, 7,6,5,4,3,2,1,0);
		rom[dest] = tmp[i];
	}
}

// Heavier encryption

void zoo_state::decrypt_rom(const decryption_info &table)
{
	const u32 size = memregion("maincpu")->bytes();
	u16 * const rom = (u16 *)memregion("maincpu")->base();
	std::unique_ptr<u16[]> tmp = std::make_unique<u16[]>(size/2);

	// Pass 1: decrypt high and low byte independently.  They go
	// through a bitswap and an xor, choosing between 8 possibilities
	// through address bits.

	for (u32 i = 0; i != size; i += 2) {
		u16 orig = rom[i >> 1];
		u16 result = 0;
		for (u32 rb = 0; rb < 2; rb ++) {
			u8 val = orig >> (rb ? 0 : 8);
			u32 index =
				(BIT(i, table.rom[rb].bits[0]) << 2) |
				(BIT(i, table.rom[rb].bits[1]) << 1) |
				BIT(i, table.rom[rb].bits[2]);
			val = bitswap(val,
						  table.rom[rb].entries[index].bits[0],
						  table.rom[rb].entries[index].bits[1],
						  table.rom[rb].entries[index].bits[2],
						  table.rom[rb].entries[index].bits[3],
						  table.rom[rb].entries[index].bits[4],
						  table.rom[rb].entries[index].bits[5],
						  table.rom[rb].entries[index].bits[6],
						  table.rom[rb].entries[index].bits[7]);
			val = val ^ table.rom[rb].entries[index].xor_mask;

			result |= val << (rb ? 0 : 8);
		}
		tmp[i >> 1] = result;
	}

	// Pass 2: copy back the decrypted data following the address
	// scrambling
	for (u32 i = 0; i != size; i += 2) {
		u32 dest =
			(i & 0xffffe003) |
			(BIT(i, table.bits[0])  << 12) |
			(BIT(i, table.bits[1])  << 11) |
			(BIT(i, table.bits[2])  << 10) |
			(BIT(i, table.bits[3])  <<  9) |
			(BIT(i, table.bits[4])  <<  8) |
			(BIT(i, table.bits[5])  <<  7) |
			(BIT(i, table.bits[6])  <<  6) |
			(BIT(i, table.bits[7])  <<  5) |
			(BIT(i, table.bits[8])  <<  4) |
			(BIT(i, table.bits[9])  <<  3) |
			(BIT(i, table.bits[10]) <<  2);
		rom[dest >> 1] = tmp[i >> 1];
	}
}

const zoo_state::decryption_info zoo_state::v102_px014_table = {
	{
		{
			{ 8, 11, 9 },
			{
				{ { 7, 5, 4, 6,  0, 3, 2, 1 }, 0x00 },
				{ { 1, 4, 6, 0,  2, 5, 3, 7 }, 0xd0 },
				{ { 1, 7, 4, 3,  6, 5, 0, 2 }, 0x88 },
				{ { 6, 5, 2, 3,  7, 1, 0, 4 }, 0xd1 },
				{ { 6, 1, 7, 2,  4, 0, 3, 5 }, 0x64 },
				{ { 1, 7, 2, 6,  5, 4, 3, 0 }, 0x83 },
				{ { 6, 7, 4, 2,  5, 0, 1, 3 }, 0x81 },
				{ { 7, 5, 1, 0,  2, 4, 6, 3 }, 0xea },
			}
		},
		{
			{ 12, 11, 10 },
			{
				{ { 6, 5, 4, 3,  2, 1, 0, 7 }, 0x90 },
				{ { 2, 4, 0, 7,  5, 6, 3, 1 }, 0x32 },
				{ { 7, 1, 0, 6,  5, 2, 3, 4 }, 0xa9 },
				{ { 2, 0, 3, 5,  1, 4, 6, 7 }, 0xa2 },
				{ { 3, 0, 6, 5,  2, 1, 4, 7 }, 0x02 },
				{ { 0, 1, 6, 4,  5, 2, 7, 3 }, 0x30 },
				{ { 3, 5, 2, 7,  6, 1, 4, 0 }, 0x0a },
				{ { 0, 6, 4, 2,  7, 3, 1, 5 }, 0x81 },
			}
		}
	},
	{ 12, 9, 8, 10, 11, 7, 6, 5, 4, 3, 2 }
};

void zoo_state::init_magibombd()
{
	decrypt_rom(v102_px014_table);
#if 1
	// TODO: There's more stuff happening for addresses < 0x400...
	// override reset vector for now
	u16 * const rom = (u16 *)memregion("maincpu")->base();
	rom[0x00004/2] = 0x0000;
	rom[0x00006/2] = 0x0446;

	rom[0x00400/2] = 0x4e75; // overlay!?

	rom[0x00f80/2] = 0x4e75; // Mirror ROM word checksum (it expects 0)
#endif
}

void zoo_state::init_magibombg()
{
	decrypt_rom(v102_px014_table);
#if 1
	// TODO: There's more stuff happening for addresses < 0x400...
	// override reset vector for now
	u16 * const rom = (u16 *)memregion("maincpu")->base();
	rom[0x00004/2] = 0x0000;
	rom[0x00006/2] = 0x043e;

	rom[0x00400/2] = 0x4e75; // overlay!?

	rom[0x00f26/2] = 0x4e75; // Mirror ROM word checksum (it expects 0)
#endif
}

void zoo_state::init_magibombm()
{
	decrypt_rom(v102_px014_table);
#if 1
	// TODO: There's more stuff happening for addresses < 0x400...
	// override reset vector for now
	u16 * const rom = (u16 *)memregion("maincpu")->base();
	rom[0x00004/2] = 0x0000;
	rom[0x00006/2] = 0x043e;

	rom[0x00400/2] = 0x4e75; // overlay!?

	rom[0x00f80/2] = 0x4e75; // Mirror ROM word checksum (it expects 0)
#endif
}

const zoo_state::decryption_info zoo_state::v102_px006_table = {
	{
		{
			{ 8, 11, 9 },
			{
				{ { 7, 5, 4, 6,  0, 3, 2, 1 }, 0x00 },
				{ { 1, 4, 6, 0,  2, 5, 3, 7 }, 0xd0 },
				{ { 1, 7, 4, 3,  6, 5, 0, 2 }, 0x88 },
				{ { 6, 5, 2, 3,  7, 1, 0, 4 }, 0xd1 },
				{ { 6, 1, 7, 2,  4, 0, 3, 5 }, 0x64 },
				{ { 1, 7, 2, 6,  5, 4, 3, 0 }, 0x83 },
				{ { 6, 7, 4, 2,  5, 0, 1, 3 }, 0x81 },
				{ { 7, 5, 1, 0,  2, 4, 6, 3 }, 0xea },
			}
		},
		{
			{ 12, 11, 10 },
			{
				{ { 6, 5, 4, 3,  2, 1, 0, 7 }, 0x90 },
				{ { 2, 4, 0, 7,  5, 6, 3, 1 }, 0x32 },
				{ { 7, 1, 0, 6,  5, 2, 3, 4 }, 0xa9 },
				{ { 2, 0, 3, 5,  1, 4, 6, 7 }, 0xa2 },
				{ { 3, 0, 6, 5,  2, 1, 4, 7 }, 0x02 },
				{ { 0, 1, 6, 4,  5, 2, 7, 3 }, 0x30 },
				{ { 3, 5, 2, 7,  6, 1, 4, 0 }, 0x0a },
				{ { 0, 6, 4, 2,  7, 3, 1, 5 }, 0x81 },
			}
		}
	},
	{ 12, 9, 8, 10, 11, 7, 5, 3, 6, 2, 4 }
};

void zoo_state::init_winbingo()
{
	decrypt_rom(v102_px006_table);
#if 1
	// TODO: There's more stuff happening for addresses < 0x400...
	// override reset vector for now
	u16 * const rom = (u16 *)memregion("maincpu")->base();
	rom[0x00004/2] = 0x0000;
	rom[0x00006/2] = 0x0400;

	rom[0x0821c/2] = 0x4e71; // Mirror ROM word checksum (it expects 0)
#endif
}

void zoo_state::init_winbingoa()
{
	decrypt_rom(v102_px006_table);
#if 1
	// TODO: There's more stuff happening for addresses < 0x400...
	// override reset vector for now
	u16 * const rom = (u16 *)memregion("maincpu")->base();
	rom[0x00004/2] = 0x0000;
	rom[0x00006/2] = 0x0400;

	rom[0x081f2/2] = 0x4e71; // Mirror ROM word checksum (it expects 0)
#endif
}

void zoo_state::init_hacher()
{
	decrypt_rom(v102_px006_table);
#if 1
	// TODO: There's more stuff happening for addresses < 0x400...
	// override reset vector for now
	u16 * const rom = (u16 *)memregion("maincpu")->base();
	rom[0x00004/2] = 0x0000;
	rom[0x00006/2] = 0x0400;

	rom[0x07d6e/2] = 0x4e71; // Mirror ROM word checksum (it expects 0)
#endif
}

void zoo_state::init_cptshark()
{
	decrypt_rom(v102_px006_table);
#if 1
	// TODO: There's more stuff happening for addresses < 0x400...
	// override reset vector for now
	u16 * const rom = (u16 *)memregion("maincpu")->base();
	rom[0x00004/2] = 0x0000;
	rom[0x00006/2] = 0x0400;

	rom[0x08a2a/2] = 0x4e71; // Mirror ROM word checksum (it expects 0)
#endif
}

const zoo_state::decryption_info zoo_state::v102_px005_table = {
	{
		{
			{ 8, 9, 10 },
			{
				{ { 7, 5, 4, 6,  0, 3, 2, 1 }, 0x00 },
				{ { 1, 4, 6, 0,  2, 5, 3, 7 }, 0xd0 },
				{ { 1, 7, 4, 3,  6, 5, 0, 2 }, 0x88 },
				{ { 6, 5, 2, 3,  7, 1, 0, 4 }, 0xd1 },
				{ { 6, 1, 7, 2,  4, 0, 3, 5 }, 0x64 },
				{ { 1, 7, 2, 6,  5, 4, 3, 0 }, 0x83 },
				{ { 6, 7, 4, 2,  5, 0, 1, 3 }, 0x81 },
				{ { 7, 5, 1, 0,  2, 4, 6, 3 }, 0xea },
			}
		},
		{
			{ 12, 9, 11 },
			{
				{ { 6, 5, 4, 3,  2, 1, 0, 7 }, 0x90 },
				{ { 2, 4, 0, 7,  5, 6, 3, 1 }, 0x32 },
				{ { 7, 1, 0, 6,  5, 2, 3, 4 }, 0xa9 },
				{ { 2, 0, 3, 5,  1, 4, 6, 7 }, 0xa2 },
				{ { 3, 0, 6, 5,  2, 1, 4, 7 }, 0x02 },
				{ { 0, 1, 6, 4,  5, 2, 7, 3 }, 0x30 },
				{ { 3, 5, 2, 7,  6, 1, 4, 0 }, 0x0a },
				{ { 0, 6, 4, 2,  7, 3, 1, 5 }, 0x81 },
			}
		}
	},
	{ 12, 10, 8, 11, 9, 7, 5, 3, 6, 2, 4 }
};

void zoo_state::init_zoo()
{
	decrypt_rom(v102_px005_table);
#if 1
	// TODO: There's more stuff happening for addresses < 0x400...
	// override reset vector for now
	u16 * const rom = (u16 *)memregion("maincpu")->base();
	rom[0x00004/2] = 0x0000;
	rom[0x00006/2] = 0x0400;

	rom[0x00010/2] = 0x1e00; // ROM 0405 ERROR
	rom[0x00012/2] = 0x10d0; // ""

	rom[0x091ac/2] = 0x4e71; // Mirror ROM word checksum (it expects 0)
#endif
}

const zoo_state::decryption_info zoo_state::v102_px010_table = {
	{
		{
			{ 8, 11, 9 },
			{
				{ { 7, 5, 4, 6,  0, 3, 2, 1 }, 0x00 },
				{ { 1, 4, 6, 0,  2, 5, 3, 7 }, 0xd0 },
				{ { 1, 7, 4, 3,  6, 5, 0, 2 }, 0x88 },
				{ { 6, 5, 2, 3,  7, 1, 0, 4 }, 0xd1 },
				{ { 6, 1, 7, 2,  4, 0, 3, 5 }, 0x64 },
				{ { 1, 7, 2, 6,  5, 4, 3, 0 }, 0x83 },
				{ { 6, 7, 4, 2,  5, 0, 1, 3 }, 0x81 },
				{ { 7, 5, 1, 0,  2, 4, 6, 3 }, 0xea },
			}
		},
		{
			{ 12, 11, 10 },
			{
				{ { 6, 5, 4, 3,  2, 1, 0, 7 }, 0x90 },
				{ { 2, 4, 0, 7,  5, 6, 3, 1 }, 0x32 },
				{ { 7, 1, 0, 6,  5, 2, 3, 4 }, 0xa9 },
				{ { 2, 0, 3, 5,  1, 4, 6, 7 }, 0xa2 },
				{ { 3, 0, 6, 5,  2, 1, 4, 7 }, 0x02 },
				{ { 0, 1, 6, 4,  5, 2, 7, 3 }, 0x30 },
				{ { 3, 5, 2, 7,  6, 1, 4, 0 }, 0x0a },
				{ { 0, 6, 4, 2,  7, 3, 1, 5 }, 0x81 },
			}
		}
	},
	{ 12, 9, 8, 10, 11, 7, 2, 6, 3, 5, 4 }
};

void zoo_state::init_dinodino()
{
	decrypt_rom(v102_px010_table);
#if 1
	// TODO: There's more stuff happening for addresses < 0x400...
	// override reset vector for now
	u16 * const rom = (u16 *)memregion("maincpu")->base();
	rom[0x00004/2] = 0x0000;
	rom[0x00006/2] = 0x0446;

	rom[0x00400/2] = 0x4e75; // overlay!?

	rom[0x02236/2] = 0x4e75; // Mirror ROM word checksum (it expects 0)
#endif
}

const zoo_state::decryption_info zoo_state::gostop_table = {
	{
		{
			{ 8, 9, 10 },
			{
				{ { 7, 5, 4, 6,  0, 3, 2, 1 }, 0x00 },
				{ { 1, 4, 6, 0,  2, 5, 3, 7 }, 0xd0 },
				{ { 1, 7, 4, 3,  6, 5, 0, 2 }, 0x88 },
				{ { 6, 5, 2, 3,  7, 1, 0, 4 }, 0xd1 },
				{ { 6, 1, 7, 2,  4, 0, 3, 5 }, 0x64 },
				{ { 1, 7, 2, 6,  5, 4, 3, 0 }, 0x83 },
				{ { 6, 7, 4, 2,  5, 0, 1, 3 }, 0x81 },
				{ { 7, 5, 1, 0,  2, 4, 6, 3 }, 0xea },
			}
		},
		{
			{ 12, 9, 11 },
			{
				{ { 6, 5, 4, 3,  2, 1, 0, 7 }, 0x90 },
				{ { 2, 4, 0, 7,  5, 6, 3, 1 }, 0x32 },
				{ { 7, 1, 0, 6,  5, 2, 3, 4 }, 0xa9 },
				{ { 2, 0, 3, 5,  1, 4, 6, 7 }, 0xa2 },
				{ { 3, 0, 6, 5,  2, 1, 4, 7 }, 0x02 },
				{ { 0, 1, 6, 4,  5, 2, 7, 3 }, 0x30 },
				{ { 3, 5, 2, 7,  6, 1, 4, 0 }, 0x0a },
				{ { 0, 6, 4, 2,  7, 3, 1, 5 }, 0x81 },
			}
		}
	},
	{ 12, 10, 8, 11, 9, 7, 2, 6, 3, 5, 4 }
};

void zoo_state::init_gostop()
{
	decrypt_rom(gostop_table);
#if 1
	// TODO: There's more stuff happening for addresses < 0x400...
	// override reset vector for now
	u16 * const rom = (u16 *)memregion("maincpu")->base();
	rom[0x00004/2] = 0x0000;
	rom[0x00006/2] = 0x040e;
#endif
}

void zoo_state::init_monkeyl()
{
	decrypt_rom(v102_px005_table);
#if 1
	// TODO: There's more stuff happening for addresses < 0x400...
	// override reset vector for now
	u16 * const rom = (u16 *)memregion("maincpu")->base();
	rom[0x00004/2] = 0x0000;
	rom[0x00006/2] = 0x0470;

	rom[0x00400/2] = 0x4e75; // overlay!?

	rom[0x01352/2] = 0x4e75; // Mirror ROM word checksum (it expects 0)
#endif
}

void zoo_state::init_monkeyla()
{
	decrypt_rom(v102_px005_table);
#if 1
	// TODO: There's more stuff happening for addresses < 0x400...
	// override reset vector for now
	u16 * const rom = (u16 *)memregion("maincpu")->base();
	rom[0x00004/2] = 0x0000;
	rom[0x00006/2] = 0x0470;

	rom[0x00400/2] = 0x4e75; // overlay!?

	rom[0x01334/2] = 0x4e75; // Mirror ROM word checksum (it expects 0)
#endif
}

void zoo_state::init_monkeyld()
{
	decrypt_rom(v102_px005_table);
#if 1
	// TODO: There's more stuff happening for addresses < 0x400...
	// override reset vector for now
	u16 * const rom = (u16 *)memregion("maincpu")->base();
	rom[0x00004/2] = 0x0000;
	rom[0x00006/2] = 0x0468;

	rom[0x00400/2] = 0x4e75; // overlay!?

	rom[0x01334/2] = 0x4e75; // Mirror ROM word checksum (it expects 0)
#endif
}

void zoo_state::init_speedmst()
{
	decrypt_rom(v102_px005_table);
#if 1
	// TODO: There's more stuff happening for addresses < 0x400...
	// override reset vector for now
	u16 * const rom = (u16 *)memregion("maincpu")->base();
	rom[0x00004/2] = 0x0000;
	rom[0x00006/2] = 0x0400;

	rom[0x08736/2] = 0x4e71; // Mirror ROM word checksum (it expects 0)
#endif
}

void zoo_state::init_speedmsta()
{
	decrypt_rom(v102_px005_table);
#if 1
	// TODO: There's more stuff happening for addresses < 0x400...
	// override reset vector for now
	u16 * const rom = (u16 *)memregion("maincpu")->base();
	rom[0x00004/2] = 0x0000;
	rom[0x00006/2] = 0x0400;

	rom[0x086fc/2] = 0x4e71; // Mirror ROM word checksum (it expects 0)
#endif
}

void zoo_state::init_speedmstb()
{
	decrypt_rom(v102_px005_table);
#if 1
	// TODO: There's more stuff happening for addresses < 0x400...
	// override reset vector for now
	u16 * const rom = (u16 *)memregion("maincpu")->base();
	rom[0x00004/2] = 0x0000;
	rom[0x00006/2] = 0x0400;

	rom[0x0871a/2] = 0x4e71; // Mirror ROM word checksum (it expects 0)
#endif
}

const zoo_state::decryption_info zoo_state::v102_px016_table = {
	{
		{
			{ 11, 10, 9 },
			{
				{ { 7, 5, 4, 6,  0, 3, 2, 1 }, 0x00 },
				{ { 1, 4, 6, 0,  2, 5, 3, 7 }, 0xd0 },
				{ { 1, 7, 4, 3,  6, 5, 0, 2 }, 0x88 },
				{ { 6, 5, 2, 3,  7, 1, 0, 4 }, 0xd1 },
				{ { 6, 1, 7, 2,  4, 0, 3, 5 }, 0x64 },
				{ { 1, 7, 2, 6,  5, 4, 3, 0 }, 0x83 },
				{ { 6, 7, 4, 2,  5, 0, 1, 3 }, 0x81 },
				{ { 7, 5, 1, 0,  2, 4, 6, 3 }, 0xea },
			}
		},
		{
			{ 12, 10, 8 },
			{
				{ { 6, 5, 4, 3,  2, 1, 0, 7 }, 0x90 },
				{ { 2, 4, 0, 7,  5, 6, 3, 1 }, 0x32 },
				{ { 7, 1, 0, 6,  5, 2, 3, 4 }, 0xa9 },
				{ { 2, 0, 3, 5,  1, 4, 6, 7 }, 0xa2 },
				{ { 3, 0, 6, 5,  2, 1, 4, 7 }, 0x02 },
				{ { 0, 1, 6, 4,  5, 2, 7, 3 }, 0x30 },
				{ { 3, 5, 2, 7,  6, 1, 4, 0 }, 0x0a },
				{ { 0, 6, 4, 2,  7, 3, 1, 5 }, 0x81 },
			}
		}
	},
	{ 12, 9, 11, 8, 10, 7, 2, 4, 6, 5, 3 }
};

void zoo_state::init_wwitch()
{
	decrypt_rom(v102_px016_table);
#if 1
	// TODO: There's more stuff happening for addresses < 0x400...
	// override reset vector for now
	u16 * const rom = (u16 *)memregion("maincpu")->base();
	rom[0x00004/2] = 0x0000;
	rom[0x00006/2] = 0x0446;

	rom[0x00400/2] = 0x4e75; // overlay!?

	rom[0x01220/2] = 0x4e75; // Mirror ROM word checksum (it expects 0)
#endif
}

const zoo_state::decryption_info zoo_state::v102_px008_table = {
	{
		{
			{ 11, 10, 9 },
			{
				{ { 7, 5, 4, 6,  0, 3, 2, 1 }, 0x00 },
				{ { 1, 4, 6, 0,  2, 5, 3, 7 }, 0xd0 },
				{ { 1, 7, 4, 3,  6, 5, 0, 2 }, 0x88 },
				{ { 6, 5, 2, 3,  7, 1, 0, 4 }, 0xd1 },
				{ { 6, 1, 7, 2,  4, 0, 3, 5 }, 0x64 },
				{ { 1, 7, 2, 6,  5, 4, 3, 0 }, 0x83 },
				{ { 6, 7, 4, 2,  5, 0, 1, 3 }, 0x81 },
				{ { 7, 5, 1, 0,  2, 4, 6, 3 }, 0xea },
			}
		},
		{
			{ 12, 10, 8 },
			{
				{ { 6, 5, 4, 3,  2, 1, 0, 7 }, 0x90 },
				{ { 2, 4, 0, 7,  5, 6, 3, 1 }, 0x32 },
				{ { 7, 1, 0, 6,  5, 2, 3, 4 }, 0xa9 },
				{ { 2, 0, 3, 5,  1, 4, 6, 7 }, 0xa2 },
				{ { 3, 0, 6, 5,  2, 1, 4, 7 }, 0x02 },
				{ { 0, 1, 6, 4,  5, 2, 7, 3 }, 0x30 },
				{ { 3, 5, 2, 7,  6, 1, 4, 0 }, 0x0a },
				{ { 0, 6, 4, 2,  7, 3, 1, 5 }, 0x81 },
			}
		}
	},
	{ 12, 9, 11, 8, 10, 7, 2, 6, 3, 5, 4 }
};

void zoo_state::init_hapfarm()
{
	decrypt_rom(v102_px008_table);
#if 1
	// TODO: There's more stuff happening for addresses < 0x400...
	// override reset vector for now
	u16 * const rom = (u16 *)memregion("maincpu")->base();
	rom[0x00004/2] = 0x0000;
	rom[0x00006/2] = 0x0400;

	rom[0x32126/2] = 0x4e75; // Mirror ROM word checksum (it expects 0)
#endif
}

const astoneag_state::decryption_info astoneag_state::v102_px012_table = {
	{
		{
			{ 11, 10, 9 },
			{
				{ { 7, 5, 4, 6,  0, 3, 2, 1 }, 0x00 },
				{ { 1, 4, 6, 0,  2, 5, 3, 7 }, 0xd0 },
				{ { 1, 7, 4, 3,  6, 5, 0, 2 }, 0x88 },
				{ { 6, 5, 2, 3,  7, 1, 0, 4 }, 0xd1 },
				{ { 6, 1, 7, 2,  4, 0, 3, 5 }, 0x64 },
				{ { 1, 7, 2, 6,  5, 4, 3, 0 }, 0x83 },
				{ { 6, 7, 4, 2,  5, 0, 1, 3 }, 0x81 },
				{ { 7, 5, 1, 0,  2, 4, 6, 3 }, 0xea },
			}
		},
		{
			{ 12, 10, 8 },
			{
				{ { 6, 5, 4, 3,  2, 1, 0, 7 }, 0x90 },
				{ { 2, 4, 0, 7,  5, 6, 3, 1 }, 0x32 },
				{ { 7, 1, 0, 6,  5, 2, 3, 4 }, 0xa9 },
				{ { 2, 0, 3, 5,  1, 4, 6, 7 }, 0xa2 },
				{ { 3, 0, 6, 5,  2, 1, 4, 7 }, 0x02 },
				{ { 0, 1, 6, 4,  5, 2, 7, 3 }, 0x30 },
				{ { 3, 5, 2, 7,  6, 1, 4, 0 }, 0x0a },
				{ { 0, 6, 4, 2,  7, 3, 1, 5 }, 0x81 },
			}
		}
	},
	{ 12, 9, 11, 8, 10, 7, 6, 5, 4, 3, 2 }
};

void astoneag_state::init_astoneag()
{
	decrypt_rom(v102_px012_table);
#if 1
	// TODO: There's more stuff happening for addresses < 0x400...
	// override reset vector for now
	u16 * const rom = (u16 *)memregion("maincpu")->base();
	rom[0x00004/2] = 0x0000;
	rom[0x00006/2] = 0x0440;

	rom[0x00400/2] = 0x4e75; // overlay!?

	rom[0x02872/2] = 0x4e75; // Mirror ROM word checksum (it expects 0)
#endif

	interleave_sprites_16x32();
}

void astoneag_state::interleave_sprites_16x32()
{
	u8 * const rom = memregion("sprites")->base();
	const u32 size = memregion("sprites")->bytes();

	std::unique_ptr<u8[]> tmp = std::make_unique<u8[]>(size);
	std::copy_n(rom, size, &tmp[0]);

	for (u32 i = 0; i != size; i += 1)
		rom[i] = tmp[(i & ~0xff) | (bitswap<8>(i, 7,6,5, 3,2,1, 4, 0) ^ 2)];
}

} // anonymous namespace

//     YEAR   NAME       PARENT    MACHINE    INPUTS     STATE            INIT            ROT   COMPANY        FULLNAME                                         FLAGS                                                  LAYOUT
GAMEL( 2000,  showhand,  0,        showhand,  showhand,  astrocorp_state, init_showhand,  ROT0, "Astro Corp.", "Show Hand (Italy)",                             MACHINE_SUPPORTS_SAVE,                                 layout_showhand  )
GAMEL( 2000,  showhanc,  showhand, showhanc,  showhanc,  astrocorp_state, init_showhanc,  ROT0, "Astro Corp.", "Wangpai Duijue (China)",                        MACHINE_SUPPORTS_SAVE,                                 layout_showhanc  )
GAMEL( 2002,  skilldrp,  0,        skilldrp,  skilldrp,  astrocorp_state, empty_init,     ROT0, "Astro Corp.", "Skill Drop Georgia (Ver. G1.01S, Oct 1 2002)",  MACHINE_SUPPORTS_SAVE,                                 layout_skilldrp  ) // Oct  1 2002 09:42:32
GAMEL( 2002,  skilldrpa, skilldrp, skilldrp,  skilldrp,  astrocorp_state, empty_init,     ROT0, "Astro Corp.", "Skill Drop Georgia (Ver. G1.0S, Sep 13 2002)",  MACHINE_SUPPORTS_SAVE,                                 layout_skilldrp  ) // Sep 13 2002 09:17:54
GAMEL( 2002,  luckycoin, skilldrp, luckycoin, skilldrp,  astrocorp_state, empty_init,     ROT0, "Astro Corp.", "Lucky Coin (Jun 24 2002)",                      MACHINE_SUPPORTS_SAVE,                                 layout_skilldrp  ) // Jun 24 2002 13:02:31
GAMEL( 2003,  speeddrp,  0,        speeddrp,  skilldrp,  astrocorp_state, empty_init,     ROT0, "Astro Corp.", "Speed Drop (Ver. 1.06, Sep 3 2003)",            MACHINE_SUPPORTS_SAVE,                                 layout_skilldrp  ) // Sep  3 2003 16:01:26

// Simpler encryption
GAMEL( 2001?, magibomb,  0,        magibomb,  magibomb,  magibomb_state,  init_magibomb,  ROT0, "Astro Corp.", "Magic Bomb (Ver. L3.5S)",                       MACHINE_SUPPORTS_SAVE | MACHINE_UNEMULATED_PROTECTION, layout_magibomb  )
GAMEL( 2002,  magibomba, magibomb, magibomb,  magibomb,  magibomb_state,  init_magibomb,  ROT0, "Astro Corp.", "Magic Bomb (Ver. BR4.4, 04/19/02S)",            MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING,           layout_magibomb  ) // One bad program ROM
GAMEL( 2002,  magibombb, magibomb, magibombb, magibomb,  magibomb_state,  init_magibomb,  ROT0, "Astro Corp.", "Magic Bomb (Ver. AB4.5A, 07/10/02S)",           MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING,           layout_magibombb )
GAMEL( 2001,  magibombc, magibomb, magibombb, magibomb,  magibomb_state,  init_magibomb,  ROT0, "Astro Corp.", "Magic Bomb (Ver. AB4.2, 11/10/01)",             MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING,           layout_magibomb  )
GAMEL( 2001?, magibombe, magibomb, magibombb, magibomb,  magibomb_state,  init_magibomb,  ROT0, "Astro Corp.", "Magic Bomb (Ver. A3.1)",                        MACHINE_SUPPORTS_SAVE | MACHINE_UNEMULATED_PROTECTION, layout_magibomb  )
GAMEL( 2002,  magibombf, magibomb, magibombf, magibomb,  magibomb_state,  init_magibomb,  ROT0, "Astro Corp.", "Magic Bomb (Ver. NB4.5, 06/14/02S)",            MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING,           layout_magibomb  )
GAMEL( 2001?, magibombh, magibomb, magibombb, magibomb,  magibomb_state,  init_magibomb,  ROT0, "Astro Corp.", "Magic Bomb (Ver. A4.0A)",                       MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING,           layout_magibomb  )
GAMEL( 2001?, magibombi, magibomb, magibombb, magibomb,  magibomb_state,  init_magibomb,  ROT0, "Astro Corp.", "Magic Bomb (Ver. A3.6A)",                       MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING,           layout_magibomb  )
GAMEL( 2001?, magibombk, magibomb, magibombf, magibomb,  magibomb_state,  init_magibomb,  ROT0, "Astro Corp.", "Magic Bomb (Ver. A3.0)",                        MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING,           layout_magibomb  )
GAMEL( 2001,  magibombl, magibomb, magibombb, magibomb,  magibomb_state,  init_magibomb,  ROT0, "Astro Corp.", "Magic Bomb (Ver. AB4.3A, 10/30/01S)",           MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING,           layout_magibomb  )
GAMEL( 2001,  magibombn, magibomb, magibombf, magibomb,  magibomb_state,  init_magibomb,  ROT0, "Astro Corp.", "Magic Bomb (Ver. EB4.0, 05/04/01)",             MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING,           layout_magibomb  )

// Heavier encryption
GAMEL( 2004,  zoo,       0,        zoo,       magibombd, zoo_state,       init_zoo,       ROT0, "Astro Corp.", "Zoo (Ver. ZO.02.D, Aug 27 2004)",               MACHINE_SUPPORTS_SAVE | MACHINE_UNEMULATED_PROTECTION, layout_zoo       ) // 10:53:44 Aug 27 2004
GAME(  2004,  gostopac,  0,        gostop,    dinodino,  zoo_state,       init_gostop,    ROT0, "Astro Corp.", "Go & Stop",                                     MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAMEL( 2005,  dinodino,  0,        dinodino,  dinodino,  zoo_state,       init_dinodino,  ROT0, "Astro Corp.", "Dino Dino (Ver. A1.1, 01/13/2005)",             MACHINE_SUPPORTS_SAVE | MACHINE_UNEMULATED_PROTECTION, layout_dinodino  ) // 13/01.2005 10:59
GAMEL( 2005,  astoneag,  0,        astoneag,  astoneag,  astoneag_state,  init_astoneag,  ROT0, "Astro Corp.", "Stone Age (Astro, Ver. EN.03.A, 2005/02/21)",   MACHINE_SUPPORTS_SAVE | MACHINE_UNEMULATED_PROTECTION, layout_astoneag  )
GAME(  2005,  monkeyl,   0,        monkeyl,   magibombd, zoo_state,       init_monkeyl,   ROT0, "Astro Corp.", "Monkey Land (Ver. AA.21.A)",                    MACHINE_SUPPORTS_SAVE | MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING ) // 18/02/2005 15:47
GAME(  2004,  monkeyla,  monkeyl,  monkeyl,   magibombd, zoo_state,       init_monkeyla,  ROT0, "Astro Corp.", "Monkey Land (Ver. AA.13.B)",                    MACHINE_SUPPORTS_SAVE | MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING ) // 23/04/2004 14:57
GAME(  2005,  monkeylb,  monkeyl,  monkeyl,   magibombd, zoo_state,       init_monkeyl,   ROT0, "Astro Corp.", "Monkey Land (Ver. AA.21.B)",                    MACHINE_SUPPORTS_SAVE | MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING ) // 25/05/2005 11:18
GAME(  2007,  monkeylc,  monkeyl,  monkeyl,   magibombd, zoo_state,       init_monkeyl,   ROT0, "Astro Corp.", "Monkey Land (Ver. AA.21.C)",                    MACHINE_SUPPORTS_SAVE | MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING ) // 05/03/2007 10:40
GAME(  2003,  monkeyld,  monkeyl,  monkeyld,  magibombg, zoo_state,       init_monkeyld,  ROT0, "Astro Corp.", "Monkey Land (Ver. A1.2)",                       MACHINE_SUPPORTS_SAVE | MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING ) // 122303S
GAMEL( 2005,  magibombd, magibomb, magibombd, magibombd, zoo_state,       init_magibombd, ROT0, "Astro Corp.", "Magic Bomb (Ver. AA.72.D, 14/11/05)",           MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING,           layout_magibombb ) // 15/11/05 09:31
GAMEL( 2005,  magibombj, magibomb, magibombd, magibombd, zoo_state,       init_magibombd, ROT0, "Astro Corp.", "Magic Bomb (Ver. AA.72.C, 25/05/05)",           MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING,           layout_magibombb ) // 25/05/05 11:26
GAMEL( 2004,  magibombg, magibomb, magibombg, magibombg, zoo_state,       init_magibombg, ROT0, "Astro Corp.", "Magic Bomb (Ver. NB6.1, 26/04/04)",             MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING,           layout_magibomb  ) // 26/04/04. Undumped sprite ROM
GAMEL( 2003,  magibombm, magibomb, magibombm, magibombg, zoo_state,       init_magibombm, ROT0, "Astro Corp.", "Magic Bomb (Ver. AB5.3, 20/06/03)",             MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING,           layout_magibomb  ) // 062003S
GAMEL( 2004,  speedmst,  0,        speedmst,  magibombg, zoo_state,       init_speedmst,  ROT0, "D2 Enterprises", "Speed Master (Ver. V1.0, Apr 29 2004)",      MACHINE_SUPPORTS_SAVE | MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING, layout_magibomb  ) // Apr 29 2004 16:29:35
GAMEL( 2003,  speedmsta, speedmst, speedmst,  magibombg, zoo_state,       init_speedmsta, ROT0, "D2 Enterprises", "Speed Master (Ver. V1.0, May 23 2003)",      MACHINE_SUPPORTS_SAVE | MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING, layout_magibomb  ) // May 23 2003 16:38:02
GAMEL( 2003,  speedmstb, speedmst, speedmst,  magibombg, zoo_state,       init_speedmstb, ROT0, "D2 Enterprises", "Speed Master (Ver. V1.0, Apr 28 2004)",      MACHINE_SUPPORTS_SAVE | MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING, layout_magibomb  ) // Apr 28 2004 17:21:26
GAMEL( 2006,  cptshark,  0,        winbingo,  winbingo,  zoo_state,       init_cptshark,  ROT0, "Astro Corp. / American Alpha", "Captain Shark (Ver. CS.01.6, Apr 21 2006)", MACHINE_SUPPORTS_SAVE | MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING, layout_winbingo  ) // 13:50:11 Apr 21 2006
GAMEL( 2006,  winbingo,  0,        winbingo,  winbingo,  zoo_state,       init_winbingo,  ROT0, "Astro Corp.", "Win Win Bingo (Ver. GM.03.3, Feb 23 2006)",     MACHINE_SUPPORTS_SAVE | MACHINE_UNEMULATED_PROTECTION, layout_winbingo  ) // 15:47:48 Feb 23 2006
GAMEL( 2006,  winbingoa, winbingo, winbingo,  winbingo,  zoo_state,       init_winbingoa, ROT0, "Astro Corp.", "Win Win Bingo (Ver. GM.05.1, May 11 2006)",     MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING,           layout_winbingo  ) // 11:02:07 May 11 2006. Undumped sprite ROMs
GAMEL( 2005,  hacher,    winbingo, hacher,    winbingo,  zoo_state,       init_hacher,    ROT0, "bootleg (Gametron)", "Hacher (hack of Win Win Bingo EN.01.6)", MACHINE_SUPPORTS_SAVE | MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_GRAPHICS, layout_winbingo  ) // 14:25:46 Mar 10 2005. One bad sprite ROM
GAME ( 2007?, westvent,  0,        skilldrp,  skilldrp,  astrocorp_state, empty_init,     ROT0, "Astro Corp.", "Western Venture (Ver. AA.02.D)",                MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING                             ) // One bad program ROM
GAME ( 2005,  wwitch,    0,        wwitch,    magibombd, zoo_state,       init_wwitch,    ROT0, "Astro Corp.", "Wicked Witch (Ver. AA.01.A)",                   MACHINE_SUPPORTS_SAVE | MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING ) // 29/12/05 09:29
GAME ( 2008,  hapfarm,   0,        hapfarm,   magibombd, zoo_state,       init_hapfarm,   ROT0, "Astro Corp.", "Happy Farm (Ver. US.01.02.B)",                  MACHINE_SUPPORTS_SAVE | MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING ) // 2008/10/16
