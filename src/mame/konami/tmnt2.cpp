// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

This driver contains several Konami 68000 based games. For the most part they
run on incompatible boards, but since 90% of the work is done by the custom
ICs emulated in video/k0*.cpp, we can just as well keep them all
together.

driver by Nicola Salmoria

***************************************************************************

Notes:
- Golfing Greats has a peculiar way to know where the ball is laying: the
  hardware latches the color of roz pixel at the center (more or less) of the
  screen, and uses that to determine if it's water, fairway etc.
- glfgreatj uses a special controller.
  1 "shot controller (with stance selection button on the top of it)" and 3
  buttons for shot direction (right/left) and club selection.
  Twist the "shot controller" to adjust shot power, then release it.
  The controller returns to its default position by internal spring.

TODO:

- glfgreat: imperfect protection emulation:
  1. putting to MAX power on green causes the game to return an incorrect
     value a.k.a. it detects a bunker/rough/water hazard;
  2. top/back spins doesn't have any effect in-game;
- glfgreat: serious sound cut off -> "it's in the" ... "water"
- prmrsocr: when the field rotates before the penalty kicks, parts of the
  053936 tilemap that shouldn't be seen are visible. Maybe the tilemap ROM is
  banked, or there are controls to clip the visible region (registers 0x06 and
  0x07 of the 053936) or both.
- is NVBLK really vblank or something else? Investigate.
- some slowdowns in lgtnfght when there are many sprites on screen - vblank issue?

Updates:

- blswhstl: sprites are left on screen during attract mode(fixed)
  Sprite buffer should be cleared at vblank start. On the GX OBJDMA
  automatically occurs 32.0-42.7us after clearing but on older boards
  using the k053245, DMA must be triggered manually. The game uses a
  trick to disable sprites by simply not triggering OBJDMA.
- a garbage sprite is STILL sticking on screen in ssriders.(fixed)
- sprite colors / zoomed placement in tmnt2(improved MCU sim)
- I don't think I'm handling the palette dim control in tmnt2/ssriders
  correctly. TMNT2 stays dimmed most of the time.(fixed)
- sprite lag, quite evident in lgtnfght and mia but also in the others.
  Also see the left corner of the wall in punkshot DownTown level(should be better)
- ssriders: Billy no longer goes berserk at stage 4's boss.

* uncertain bugs:
- Detana!! Twin Bee's remaining sprite lag does not appear to be
  emulation related. While these common one-pixel lags are very obvious
  on VGA-class displays they're virtually invisible on TV and older
  15kHz arcade monitors.

***************************************************************************/

#include "emu.h"

#include "k053244_k053245.h"
#include "k052109.h"
#include "k051960.h"
#include "k053251.h"
#include "k054000.h"
#include "konamipt.h"
#include "konami_helper.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/adc0804.h"
#include "machine/eepromser.h"
#include "machine/k054321.h"
#include "machine/rescap.h"
#include "machine/watchdog.h"
#include "sound/k053260.h"
#include "sound/k054539.h"
#include "sound/okim6295.h"
#include "sound/ymopm.h"
#include "video/k053936.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

namespace {

class tmnt2_state : public driver_device
{
public:
	tmnt2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_tmnt2_rom(*this, "maincpu"),
		m_sunset_104000(*this, "sunset_104000"),
		m_tmnt2_1c0800(*this, "tmnt2_1c0800"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k053260(*this, "k053260"),
		m_k054539(*this, "k054539"),
		m_k052109(*this, "k052109"),
		m_k051960(*this, "k051960"),
		m_k053245(*this, "k053245"),
		m_k053251(*this, "k053251"),
		m_k053936(*this, "k053936"),
		m_k054000(*this, "k054000"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_coins(*this, "COINS"),
		m_eeprom(*this, "EEPROM"),
		m_eepromout(*this, "EEPROMOUT"),
		m_p2_eeprom(*this, "P2_EEPROM")
	{ }

	void blswhstl(machine_config &config);
	void ssriders(machine_config &config);
	void tmnt2(machine_config &config);
	void lgtnfght(machine_config &config);
	void thndrx2(machine_config &config);
	void punkshot(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	// memory pointers
	optional_shared_ptr<uint16_t> m_spriteram;
	required_region_ptr<uint16_t> m_tmnt2_rom;
	optional_shared_ptr<uint16_t> m_sunset_104000;
	optional_shared_ptr<uint16_t> m_tmnt2_1c0800;

	// video-related
	int        m_layer_colorbase[3]{};
	int        m_sprite_colorbase = 0;
	int        m_layerpri[3]{};
	int        m_sorted_layer[3]{};   // this might not be necessary, but tmnt2 uses it in a strange way...
	tilemap_t  *m_roz_tilemap = nullptr;
	int        m_glfgreat_pixel = 0;
	int        m_blswhstl_rombank = 0;
	int        m_lastdim = 0;
	int        m_lasten = 0;
	int        m_dim_c = 0;
	int        m_dim_v = 0; // lgtnfght, ssriders, tmnt2 only

	// misc
	emu_timer  *m_nmi_blocked = nullptr;
	int        m_toggle = 0;
	int        m_last = 0;

	// devices
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<k053260_device> m_k053260;
	optional_device<k054539_device> m_k054539;
	required_device<k052109_device> m_k052109;
	optional_device<k051960_device> m_k051960;
	optional_device<k05324x_device> m_k053245;
	required_device<k053251_device> m_k053251;
	optional_device<k053936_device> m_k053936;
	optional_device<k054000_device> m_k054000;
	optional_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	optional_ioport m_coins;
	optional_ioport m_eeprom;
	optional_ioport m_eepromout;
	optional_ioport m_p2_eeprom;

	uint16_t k052109_word_r(offs_t offset, uint16_t mem_mask = ~0);
	void k052109_word_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t k052109_word_noA12_r(offs_t offset, uint16_t mem_mask = ~0);
	void k052109_word_noA12_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t k053245_scattered_word_r(offs_t offset);
	void k053245_scattered_word_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t k053244_word_noA1_r(offs_t offset);
	void k053244_word_noA1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void sound_arm_nmi_w(uint8_t data);
	void z80_nmi_w(int state);
	uint16_t punkshot_kludge_r();
	uint16_t ssriders_protection_r(address_space &space);
	void ssriders_protection_w(address_space &space, offs_t offset, uint16_t data);
	uint16_t blswhstl_coin_r();
	uint16_t ssriders_eeprom_r();
	uint16_t sunsetbl_eeprom_r();
	void blswhstl_eeprom_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t thndrx2_eeprom_r();
	void thndrx2_eeprom_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void ssriders_soundkludge_w(uint16_t data);
	void tmnt2_1c0800_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void punkshot_0a0020_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void lgtnfght_0a0018_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void blswhstl_700300_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void ssriders_eeprom_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void ssriders_1c0300_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	DECLARE_VIDEO_START(lgtnfght);
	DECLARE_VIDEO_START(blswhstl);
	uint32_t screen_update_punkshot(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_lgtnfght(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_glfgreat(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_tmnt2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_thndrx2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank_blswhstl(int state);
	inline uint32_t tmnt2_get_word(uint32_t addr);
	void tmnt2_put_word(uint32_t addr, uint16_t data);
	K051960_CB_MEMBER(punkshot_sprite_callback);
	K051960_CB_MEMBER(thndrx2_sprite_callback);
	K05324X_CB_MEMBER(lgtnfght_sprite_callback);
	K05324X_CB_MEMBER(blswhstl_sprite_callback);
	K052109_CB_MEMBER(tmnt_tile_callback);
	K052109_CB_MEMBER(blswhstl_tile_callback);

	void blswhstl_main_map(address_map &map) ATTR_COLD;
	void lgtnfght_audio_map(address_map &map) ATTR_COLD;
	void lgtnfght_main_map(address_map &map) ATTR_COLD;
	void punkshot_audio_map(address_map &map) ATTR_COLD;
	void punkshot_main_map(address_map &map) ATTR_COLD;
	void ssriders_audio_map(address_map &map) ATTR_COLD;
	void ssriders_main_map(address_map &map) ATTR_COLD;
	void thndrx2_audio_map(address_map &map) ATTR_COLD;
	void thndrx2_main_map(address_map &map) ATTR_COLD;
	void tmnt2_main_map(address_map &map) ATTR_COLD;
};

class glfgreat_state : public tmnt2_state
{
public:
	glfgreat_state(const machine_config &mconfig, device_type type, const char *tag) :
		tmnt2_state(mconfig, type, tag),
		m_analog_controller(*this, "CONTROL%c", 'A')
	{ }

	void glfgreat(machine_config &config);

private:
	uint8_t controller_r();
	uint16_t glfgreat_rom_r(offs_t offset);
	void glfgreat_122000_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t glfgreat_ball_r();
	void glfgreat_sound_w(offs_t offset, uint8_t data);

	TILE_GET_INFO_MEMBER(glfgreat_get_roz_tile_info);
	DECLARE_VIDEO_START(glfgreat);

	void glfgreat_audio_map(address_map &map) ATTR_COLD;
	void glfgreat_main_map(address_map &map) ATTR_COLD;

	required_ioport_array<4> m_analog_controller;

	uint8_t    m_controller_select = 0;
	int        m_roz_rom_bank = 0;
	int        m_roz_char_bank = 0;
	int        m_roz_rom_mode = 0;
};

class prmrsocr_state : public tmnt2_state
{
public:
	prmrsocr_state(const machine_config &mconfig, device_type type, const char *tag) :
		tmnt2_state(mconfig, type, tag),
		m_audiobank(*this, "bank1")
	{ }

	void prmrsocr(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void prmrsocr_sound_irq_w(uint16_t data);
	void prmrsocr_audio_bankswitch_w(uint8_t data);
	void prmrsocr_eeprom_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void prmrsocr_122000_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t prmrsocr_rom_r(offs_t offset);

	TILE_GET_INFO_MEMBER(prmrsocr_get_roz_tile_info);
	DECLARE_VIDEO_START(prmrsocr);
	K05324X_CB_MEMBER(prmrsocr_sprite_callback);

	void prmrsocr_audio_map(address_map &map) ATTR_COLD;
	void prmrsocr_main_map(address_map &map) ATTR_COLD;

	required_memory_bank m_audiobank;

	int        m_sprite_bank = 0;
	int        m_roz_char_bank = 0;
};

class sunsetbl_state : public tmnt2_state
{
public:
	sunsetbl_state(const machine_config &mconfig, device_type type, const char *tag) :
		tmnt2_state(mconfig, type, tag)
	{ }

	void sunsetbl(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void irq_enable_w(u8 data);
	void sunsetbl_vblank_w(int state);
	K052109_CB_MEMBER(ssbl_tile_callback);

	void sunsetbl_main_map(address_map &map) ATTR_COLD;

	bool m_irq_enable = false;
};


uint16_t tmnt2_state::k052109_word_r(offs_t offset, uint16_t mem_mask)
{
	if (ACCESSING_BITS_8_15)
		return m_k052109->read(offset) << 8;
	else
		return m_k052109->read(offset + 0x2000);
}

void tmnt2_state::k052109_word_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// A13 = 68000 UDS. Due to this and related bus buffering, word writes only affect the MSB.
	// The "ROUND 1" text in punkshtj goes lost otherwise.
	if (ACCESSING_BITS_8_15)
		m_k052109->write(offset, (data >> 8) & 0xff);
	else
		m_k052109->write(offset + 0x2000, data & 0xff);
}

uint16_t tmnt2_state::k052109_word_noA12_r(offs_t offset, uint16_t mem_mask)
{
	/* some games have the A12 line not connected, so the chip spans */
	/* twice the memory range, with mirroring */
	offset = ((offset & 0x3000) >> 1) | (offset & 0x07ff);
	return k052109_word_r(offset, mem_mask);
}

void tmnt2_state::k052109_word_noA12_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	/* some games have the A12 line not connected, so the chip spans */
	/* twice the memory range, with mirroring */
	offset = ((offset & 0x3000) >> 1) | (offset & 0x07ff);
	k052109_word_w(offset, data, mem_mask);
}


/* the interface with the 053245 is weird. The chip can address only 0x800 bytes */
/* of RAM, but they put 0x4000 there. The CPU can access them all. Address lines */
/* A1, A5 and A6 don't go to the 053245. */
uint16_t tmnt2_state::k053245_scattered_word_r(offs_t offset)
{
	if (offset & 0x0031)
		return m_spriteram[offset];
	else
	{
		offset = ((offset & 0x000e) >> 1) | ((offset & 0x1fc0) >> 3);
		return m_k053245->k053245_word_r(offset);
	}
}

void tmnt2_state::k053245_scattered_word_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_spriteram[offset]);

	if (!(offset & 0x0031))
	{
		offset = ((offset & 0x000e) >> 1) | ((offset & 0x1fc0) >> 3);
		m_k053245->k053245_word_w(offset, data, mem_mask);
	}
}

uint16_t tmnt2_state::k053244_word_noA1_r(offs_t offset)
{
	offset &= ~1;   /* handle mirror address */

	return m_k053245->k053244_r(offset + 1) | (m_k053245->k053244_r(offset) << 8);
}

void tmnt2_state::k053244_word_noA1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	offset &= ~1;   /* handle mirror address */

	if (ACCESSING_BITS_8_15)
		m_k053245->k053244_w(offset, (data >> 8) & 0xff);
	if (ACCESSING_BITS_0_7)
		m_k053245->k053244_w(offset + 1, data & 0xff);
}



void glfgreat_state::glfgreat_sound_w(offs_t offset, uint8_t data)
{
	m_k053260->main_write(offset, data);

	if (offset)
		m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0xff); // Z80
}


void prmrsocr_state::prmrsocr_sound_irq_w(uint16_t data)
{
	m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0xff); // Z80
}

void prmrsocr_state::prmrsocr_audio_bankswitch_w(uint8_t data)
{
	m_audiobank->set_entry(data & 7);
}


void tmnt2_state::sound_arm_nmi_w(uint8_t data)
{
	// see notes in simpsons driver
	m_audiocpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	m_nmi_blocked->adjust(m_audiocpu->cycles_to_attotime(4));
}

void tmnt2_state::z80_nmi_w(int state)
{
	if (state && !m_nmi_blocked->enabled())
		m_audiocpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}


uint16_t tmnt2_state::punkshot_kludge_r()
{
	/* I don't know what's going on here; at one point, the code reads location */
	/* 0xffffff, and returning 0 causes the game to mess up - locking up in a */
	/* loop where the ball is continuously bouncing from the basket. Returning */
	/* a random number seems to prevent that. */
	return machine().rand();
}


/* protection simulation derived from a bootleg */
uint16_t tmnt2_state::ssriders_protection_r(address_space &space)
{
	int data = space.read_word(0x105a0a);
	int cmd = space.read_word(0x1058fc);

	switch (cmd)
	{
		case 0x100b:
			/* read twice in a row, first result discarded? */
			/* data is always == 0x75c */
			return 0x0064;

		case 0x6003:
			/* start of level */
			return data & 0x000f;

		case 0x6004:
			return data & 0x001f;

		case 0x6000:
			return data & 0x0001;

		case 0x0000:
			return data & 0x00ff;

		case 0x6007:
			return data & 0x00ff;

		case 0x8abc:
			/* collision table */
			data = -space.read_word(0x105818);
			data = ((data / 8 - 4) & 0x1f) * 0x40;
			// 0x1040c8 is the x scroll buffer, avoids stutter on slopes + scrolling (and it's actually more logical as HW pov)
			data += ((space.read_word(0x105cb0) + space.read_word(0x1040c8) - 6) / 8 + 12) & 0x3f;
			return data;

		default:
			popmessage("%06x: unknown protection read",m_maincpu->pc());
			logerror("%06x: read 1c0800 (D7=%02x 1058fc=%02x 105a0a=%02x)\n",m_maincpu->pc(),(uint32_t)m_maincpu->state_int(M68K_D7),cmd,data);
			return 0xffff;
	}
}

void tmnt2_state::ssriders_protection_w(address_space &space, offs_t offset, uint16_t data)
{
	if (offset == 1)
	{
		int logical_pri, hardware_pri;

		/* create sprite priority attributes */
		hardware_pri = 1;
		for (logical_pri = 1; logical_pri < 0x100; logical_pri <<= 1)
		{
			int i;

			for (i = 0; i < 128; i++)
			{
				if ((space.read_word(0x180006 + 128 * i) >> 8) == logical_pri)
				{
					m_k053245->k053245_word_w(8 * i, hardware_pri, 0x00ff);
					hardware_pri++;
				}
			}
		}
	}
}



/***************************************************************************

  EEPROM

***************************************************************************/

uint16_t tmnt2_state::blswhstl_coin_r()
{
	int res;

	/* bit 3 is service button */
	/* bit 6 is ??? VBLANK? OBJMPX? */
	res = m_coins->read();

	m_toggle ^= 0x40;
	return res ^ m_toggle;
}

uint16_t tmnt2_state::ssriders_eeprom_r()
{
	int res;

	/* bit 0 is EEPROM data */
	/* bit 1 is EEPROM ready */
	/* bit 2 is VBLANK (???) */
	/* bit 7 is service button */
	res = m_eeprom->read();

	m_toggle ^= 0x04;
	return res ^ m_toggle;
}

uint16_t tmnt2_state::sunsetbl_eeprom_r()
{
	int res;

	/* bit 0 is EEPROM data */
	/* bit 1 is EEPROM ready */
	/* bit 2 is VBLANK (???) */
	/* bit 3 is service button */
	res = m_eeprom->read();

	m_toggle ^= 0x04;
	return res ^ m_toggle;
}

void tmnt2_state::blswhstl_eeprom_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		/* bit 0 is data */
		/* bit 1 is cs (active low) */
		/* bit 2 is clock (active high) */
		m_eepromout->write(data, 0xff);
	}
}

uint16_t tmnt2_state::thndrx2_eeprom_r()
{
	int res;

	/* bit 0 is EEPROM data */
	/* bit 1 is EEPROM ready */
	/* bit 3 is VBLANK (???) */
	/* bit 7 is service button */
	res = m_p2_eeprom->read();
	m_toggle ^= 0x0800;
	return (res ^ m_toggle);
}

void tmnt2_state::thndrx2_eeprom_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		/* bit 0 is data */
		/* bit 1 is cs (active low) */
		/* bit 2 is clock (active high) */
		m_eepromout->write(data, 0xff);

		/* bit 5 triggers IRQ on sound cpu */
		if (m_last == 0 && (data & 0x20) != 0)
			m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0xff); // Z80
		m_last = data & 0x20;

		/* bit 6 = enable char ROM reading through the video RAM */
		m_k052109->set_rmrd_line((data & 0x40) ? ASSERT_LINE : CLEAR_LINE);
	}
}

void prmrsocr_state::prmrsocr_eeprom_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		prmrsocr_122000_w(offset, data, mem_mask);
	}

	if (ACCESSING_BITS_8_15)
	{
		/* bit 8 is data */
		/* bit 9 is cs (active low) */
		/* bit 10 is clock (active high) */
		m_eepromout->write(data, 0xffff);
	}
}


TILE_GET_INFO_MEMBER(glfgreat_state::glfgreat_get_roz_tile_info)
{
	uint8_t *rom = memregion("user1")->base();
	int code;

	tile_index += 0x40000 * m_roz_rom_bank;

	code = rom[tile_index + 0x80000] + 256 * rom[tile_index] + 256 * 256 * ((rom[tile_index / 4 + 0x100000] >> (2 * (tile_index & 3))) & 3);

	tileinfo.set(0, code & 0x3fff, code >> 14, 0);
}

TILE_GET_INFO_MEMBER(prmrsocr_state::prmrsocr_get_roz_tile_info)
{
	uint8_t *rom = memregion("user1")->base();
	int code = rom[tile_index + 0x20000] + 256 * rom[tile_index];

	tileinfo.set(0, code & 0x1fff, code >> 13, 0);
}




/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

K052109_CB_MEMBER(tmnt2_state::tmnt_tile_callback)
{
	*code |= ((*color & 0x03) << 8) | ((*color & 0x10) << 6) | ((*color & 0x0c) << 9) | (bank << 13);
	*color = m_layer_colorbase[layer] + ((*color & 0xe0) >> 5);
}

K052109_CB_MEMBER(sunsetbl_state::ssbl_tile_callback)
{
	if (layer == 0)
	{
		*code |= ((*color & 0x03) << 8) | ((*color & 0x10) << 6) | ((*color & 0x0c) << 9) | (bank << 13);
	}
	else
	{
		*code |= ((*color & 0x03) << 8) | ((*color & 0x10) << 6) | ((*color & 0x0c) << 9) | (bank << 13);
//      osd_printf_debug("L%d: bank %d code %x color %x\n", layer, bank, *code, *color);
	}

	*color = m_layer_colorbase[layer] + ((*color & 0xe0) >> 5);
}

K052109_CB_MEMBER(tmnt2_state::blswhstl_tile_callback)
{
	/* (color & 0x02) is flip y handled internally by the 052109 */
	*code |= ((*color & 0x01) << 8) | ((*color & 0x10) << 5) | ((*color & 0x0c) << 8) | (bank << 12) | m_blswhstl_rombank << 14;
	*color = m_layer_colorbase[layer] + ((*color & 0xe0) >> 5);
}



/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

K051960_CB_MEMBER(tmnt2_state::punkshot_sprite_callback)
{
	int pri = 0x20 | ((*color & 0x60) >> 2);
	if (pri <= m_layerpri[2])
		*priority = 0;
	else if (pri > m_layerpri[2] && pri <= m_layerpri[1])
		*priority = 0xf0;
	else if (pri > m_layerpri[1] && pri <= m_layerpri[0])
		*priority = 0xf0 | 0xcc;
	else
		*priority = 0xf0 | 0xcc | 0xaa;

	*code |= (*color & 0x10) << 9;
	*color = m_sprite_colorbase + (*color & 0x0f);
}

K051960_CB_MEMBER(tmnt2_state::thndrx2_sprite_callback)
{
	int pri = 0x20 | ((*color & 0x60) >> 2);
	if (pri <= m_layerpri[2])
		*priority = 0;
	else if (pri > m_layerpri[2] && pri <= m_layerpri[1])
		*priority = 0xf0;
	else if (pri > m_layerpri[1] && pri <= m_layerpri[0])
		*priority = 0xf0 | 0xcc;
	else
		*priority = 0xf0 | 0xcc | 0xaa;

	*color = m_sprite_colorbase + (*color & 0x0f);
}


/***************************************************************************

  Callbacks for the K053245

***************************************************************************/

K05324X_CB_MEMBER(tmnt2_state::lgtnfght_sprite_callback)
{
	int pri = 0x20 | ((*color & 0x60) >> 2);
	if (pri <= m_layerpri[2])
		*priority = 0;
	else if (pri > m_layerpri[2] && pri <= m_layerpri[1])
		*priority = 0xf0;
	else if (pri > m_layerpri[1] && pri <= m_layerpri[0])
		*priority = 0xf0 | 0xcc;
	else
		*priority = 0xf0 | 0xcc | 0xaa;

	*color = m_sprite_colorbase + (*color & 0x1f);
}

K05324X_CB_MEMBER(tmnt2_state::blswhstl_sprite_callback)
{
#if 0
if (machine().input().code_pressed(KEYCODE_Q) && (*color & 0x20)) *color = machine().rand();
if (machine().input().code_pressed(KEYCODE_W) && (*color & 0x40)) *color = machine().rand();
if (machine().input().code_pressed(KEYCODE_E) && (*color & 0x80)) *color = machine().rand();
#endif
	int pri = 0x20 | ((*color & 0x60) >> 2);
	if (pri <= m_layerpri[2])
		*priority = 0;
	else if (pri > m_layerpri[2] && pri <= m_layerpri[1])
		*priority = 0xf0;
	else if (pri > m_layerpri[1] && pri <= m_layerpri[0])
		*priority = 0xf0 | 0xcc;
	else
		*priority = 0xf0 | 0xcc | 0xaa;

	*color = m_sprite_colorbase + (*color & 0x1f);
}

K05324X_CB_MEMBER(prmrsocr_state::prmrsocr_sprite_callback)
{
	int pri = 0x20 | ((*color & 0x60) >> 2);
	if (pri <= m_layerpri[2])
		*priority = 0;
	else if (pri > m_layerpri[2] && pri <= m_layerpri[1])
		*priority = 0xf0;
	else if (pri > m_layerpri[1] && pri <= m_layerpri[0])
		*priority = 0xf0 | 0xcc;
	else
		*priority = 0xf0 | 0xcc | 0xaa;

	*code |= m_sprite_bank << 14;
	*color = m_sprite_colorbase + (*color & 0x1f);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START_MEMBER(tmnt2_state,lgtnfght)/* also tmnt2, ssriders */
{
	m_k053245->set_z_rejection(0);

	m_dim_c = m_dim_v = m_lastdim = m_lasten = 0;

	for (int i = 0; i < 3; i++)
	{
		m_layer_colorbase[i] = 0;
		m_sorted_layer[i] = 0;
	}

	save_item(NAME(m_dim_c));
	save_item(NAME(m_dim_v));
	save_item(NAME(m_lastdim));
	save_item(NAME(m_lasten));
}

VIDEO_START_MEMBER(glfgreat_state,glfgreat)
{
	m_roz_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(glfgreat_state::glfgreat_get_roz_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 512, 512);
	m_roz_tilemap->set_transparent_pen(0);

	m_controller_select = 0;
	m_roz_rom_bank = 0;
	m_roz_char_bank = 0;
	m_roz_rom_mode = 0;
	save_item(NAME(m_controller_select));
	save_item(NAME(m_roz_rom_bank));
	save_item(NAME(m_roz_char_bank));
	save_item(NAME(m_roz_rom_mode));
}

VIDEO_START_MEMBER(prmrsocr_state,prmrsocr)
{
	m_roz_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(prmrsocr_state::prmrsocr_get_roz_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 512, 256);
	m_roz_tilemap->set_transparent_pen(0);

	m_sprite_bank = 0;
	m_roz_char_bank = 0;
	save_item(NAME(m_sprite_bank));
	save_item(NAME(m_roz_char_bank));
}

VIDEO_START_MEMBER(tmnt2_state,blswhstl)
{
	m_blswhstl_rombank = -1;
	save_item(NAME(m_blswhstl_rombank));
}


/***************************************************************************

  Memory handlers

***************************************************************************/

void tmnt2_state::punkshot_0a0020_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		/* bit 0 = coin counter */
		machine().bookkeeping().coin_counter_w(0, data & 0x01);

		/* bit 2 = trigger irq on sound CPU */
		if (m_last == 0x04 && (data & 0x04) == 0)
			m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0xff); // Z80

		m_last = data & 0x04;

		/* bit 3 = enable char ROM reading through the video RAM */
		m_k052109->set_rmrd_line((data & 0x08) ? ASSERT_LINE : CLEAR_LINE);
	}
}

void tmnt2_state::lgtnfght_0a0018_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		/* bit 0,1 = coin counter */
		machine().bookkeeping().coin_counter_w(0, data & 0x01);
		machine().bookkeeping().coin_counter_w(1, data & 0x02);

		/* bit 2 = trigger irq on sound CPU */
		if (m_last == 0x00 && (data & 0x04) == 0x04)
			m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0xff); // Z80

		m_last = data & 0x04;

		/* bit 3 = enable char ROM reading through the video RAM */
		m_k052109->set_rmrd_line((data & 0x08) ? ASSERT_LINE : CLEAR_LINE);
	}
}

void tmnt2_state::blswhstl_700300_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		/* bit 0,1 = coin counter */
		machine().bookkeeping().coin_counter_w(0,data & 0x01);
		machine().bookkeeping().coin_counter_w(1,data & 0x02);

		/* bit 3 = enable char ROM reading through the video RAM */
		m_k052109->set_rmrd_line((data & 0x08) ? ASSERT_LINE : CLEAR_LINE);

		/* bit 7 = select char ROM bank */
		if (m_blswhstl_rombank != ((data & 0x80) >> 7))
		{
			m_blswhstl_rombank = (data & 0x80) >> 7;
			machine().tilemap().mark_all_dirty();
		}

		/* other bits unknown */
	}
}


uint16_t glfgreat_state::glfgreat_rom_r(offs_t offset)
{
	if (m_roz_rom_mode)
		return memregion("zoom")->base()[m_roz_char_bank * 0x80000 + offset];
	else if (offset < 0x40000)
	{
		uint8_t *usr = memregion("user1")->base();
		return usr[offset + 0x80000 + m_roz_rom_bank * 0x40000] + 256 * usr[offset + m_roz_rom_bank * 0x40000];
	}
	else
		return memregion("user1")->base()[((offset & 0x3ffff) >> 2) + 0x100000 + m_roz_rom_bank * 0x10000];
}

void glfgreat_state::glfgreat_122000_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		/* bit 0,1 = coin counter */
		machine().bookkeeping().coin_counter_w(0, data & 0x01);
		machine().bookkeeping().coin_counter_w(1, data & 0x02);

		m_controller_select = (data & 0x0c) >> 2;

		/* bit 4 = enable char ROM reading through the video RAM */
		m_k052109->set_rmrd_line((data & 0x10) ? ASSERT_LINE : CLEAR_LINE);

		/* bit 5 = 53596 tile rom bank selection */
		if (m_roz_rom_bank != (data & 0x20) >> 5)
		{
			m_roz_rom_bank = (data & 0x20) >> 5;
			m_roz_tilemap->mark_all_dirty();
		}

		/* bit 6,7 = 53596 char bank selection for ROM test */
		m_roz_char_bank = (data & 0xc0) >> 6;
	}
	if (ACCESSING_BITS_8_15)
	{
		/* bit 8 = 53596 char/rom selection for ROM test */
		m_roz_rom_mode = data & 0x100;
	}
}


void tmnt2_state::ssriders_eeprom_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		/* bit 0 is data */
		/* bit 1 is cs (active low) */
		/* bit 2 is clock (active high) */
		ioport("EEPROMOUT")->write(data, 0xff);

		/* bits 3-4 control palette dimming */
		/* 4 = DIMPOL = when set, negate SHAD */
		/* 3 = DIMMOD = when set, or BRIT with [negated] SHAD */
		m_dim_c = data & 0x18;

		/* bit 5 selects sprite ROM for testing in TMNT2 (bits 5-7, actually, according to the schematics) */
		m_k053245->bankselect(((data & 0x20) >> 5) << 2);
	}
}

void tmnt2_state::ssriders_1c0300_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		/* bit 0,1 = coin counter */
		machine().bookkeeping().coin_counter_w(0, data & 0x01);
		machine().bookkeeping().coin_counter_w(1, data & 0x02);

		/* bit 3 = enable char ROM reading through the video RAM */
		m_k052109->set_rmrd_line((data & 0x08) ? ASSERT_LINE : CLEAR_LINE);

		/* bits 4-6 control palette dimming (DIM0-DIM2) */
		m_dim_v = (data & 0x70) >> 4;
	}
}

void prmrsocr_state::prmrsocr_122000_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		/* bit 0,1 = coin counter */
		machine().bookkeeping().coin_counter_w(0, data & 0x01);
		machine().bookkeeping().coin_counter_w(1, data & 0x02);

		/* bit 4 = enable char ROM reading through the video RAM */
		m_k052109->set_rmrd_line((data & 0x10) ? ASSERT_LINE : CLEAR_LINE);

		/* bit 6 = sprite ROM bank */
		m_sprite_bank = (data & 0x40) >> 6;
		m_k053245->bankselect(m_sprite_bank << 2);

		/* bit 7 = 53596 region selector for ROM test */
		m_roz_char_bank = (data & 0x80) >> 7;

		/* other bits unknown (unused?) */
	}
}

uint16_t prmrsocr_state::prmrsocr_rom_r(offs_t offset)
{
	if(m_roz_char_bank)
		return memregion("zoom")->base()[offset];
	else
	{
		uint8_t *usr = memregion("user1")->base();
		return 256 * usr[offset] + usr[offset + 0x020000];
	}
}



/***************************************************************************

  Display refresh

***************************************************************************/

uint32_t tmnt2_state::screen_update_punkshot(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// update color info and refresh tilemaps
	static const int K053251_CI[3] = { k053251_device::CI2, k053251_device::CI4, k053251_device::CI3 };
	m_sprite_colorbase = m_k053251->get_palette_index(k053251_device::CI1);

	for (int i = 0; i < 3; i++)
	{
		int prev_colorbase = m_layer_colorbase[i];
		m_layer_colorbase[i] = m_k053251->get_palette_index(K053251_CI[i]);

		if (m_layer_colorbase[i] != prev_colorbase)
			m_k052109->mark_tilemap_dirty(i);
	}

	m_k052109->tilemap_update();

	// sort layers and draw
	for (int i = 0; i < 3; i++)
	{
		m_sorted_layer[i] = i;
		m_layerpri[i] = m_k053251->get_priority(K053251_CI[i]);
	}

	konami_sortlayers3(m_sorted_layer, m_layerpri);

	screen.priority().fill(0, cliprect);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, m_sorted_layer[0], TILEMAP_DRAW_OPAQUE, 1);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, m_sorted_layer[1], 0, 2);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, m_sorted_layer[2], 0, 4);

	m_k051960->k051960_sprites_draw(bitmap, cliprect, screen.priority(), -1, -1);
	return 0;
}


uint32_t tmnt2_state::screen_update_lgtnfght(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// update color info and refresh tilemaps
	static const int K053251_CI[3] = { k053251_device::CI2, k053251_device::CI4, k053251_device::CI3 };
	int bg_colorbase = m_k053251->get_palette_index(k053251_device::CI0);
	m_sprite_colorbase = m_k053251->get_palette_index(k053251_device::CI1);

	for (int i = 0; i < 3; i++)
	{
		int prev_colorbase = m_layer_colorbase[i];
		m_layer_colorbase[i] = m_k053251->get_palette_index(K053251_CI[i]);

		if (m_layer_colorbase[i] != prev_colorbase)
			m_k052109->mark_tilemap_dirty(i);
	}

	m_k052109->tilemap_update();

	// sort layers and draw
	for (int i = 0; i < 3; i++)
	{
		m_sorted_layer[i] = i;
		m_layerpri[i] = m_k053251->get_priority(K053251_CI[i]);
	}

	konami_sortlayers3(m_sorted_layer, m_layerpri);

	screen.priority().fill(0, cliprect);
	bitmap.fill(16 * bg_colorbase, cliprect);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, m_sorted_layer[0], 0, 1);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, m_sorted_layer[1], 0, 2);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, m_sorted_layer[2], 0, 4);

	m_k053245->sprites_draw(bitmap, cliprect, screen.priority());
	return 0;
}


uint16_t glfgreat_state::glfgreat_ball_r()
{
#ifdef MAME_DEBUG
	popmessage("%04x", m_glfgreat_pixel);
#endif
	// if out of the ROZ layer palette range, it's in the water - return 0
	if (m_glfgreat_pixel < 0x400 || m_glfgreat_pixel >= 0x500)
		return 0;
	else
		return m_glfgreat_pixel & 0xff;
}

uint32_t tmnt2_state::screen_update_glfgreat(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// update color info and refresh tilemaps
	static const int K053251_CI[3] = { k053251_device::CI2, k053251_device::CI3, k053251_device::CI4 };
	int bg_colorbase = m_k053251->get_palette_index(k053251_device::CI0);
	m_sprite_colorbase  = m_k053251->get_palette_index(k053251_device::CI1);

	for (int i = 0; i < 3; i++)
	{
		int prev_colorbase = m_layer_colorbase[i];
		m_layer_colorbase[i] = m_k053251->get_palette_index(K053251_CI[i]);
		if (i == 1) m_layer_colorbase[i] += 8; // weird

		if (m_layer_colorbase[i] != prev_colorbase)
			m_k052109->mark_tilemap_dirty(i);
	}

	m_k052109->tilemap_update();

	// sort layers and draw
	for (int i = 0; i < 3; i++)
	{
		m_sorted_layer[i] = i;
		m_layerpri[i] = m_k053251->get_priority(K053251_CI[i]);
	}

	konami_sortlayers3(m_sorted_layer, m_layerpri);

	// not sure about the 053936 priority, but it seems to work
	screen.priority().fill(0, cliprect);
	bitmap.fill(16 * bg_colorbase, cliprect);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, m_sorted_layer[0], 0, 1);

	if (m_layerpri[0] >= 0x30 && m_layerpri[1] < 0x30)
	{
		m_k053936->zoom_draw(screen, bitmap, cliprect, m_roz_tilemap, 0, 1, 1);
		m_glfgreat_pixel = bitmap.pix(0x80, 0x105);
	}

	m_k052109->tilemap_draw(screen, bitmap, cliprect, m_sorted_layer[1], 0, 2);

	if (m_layerpri[1] >= 0x30 && m_layerpri[2] < 0x30)
	{
		m_k053936->zoom_draw(screen, bitmap, cliprect, m_roz_tilemap, 0, 1, 1);
		m_glfgreat_pixel = bitmap.pix(0x80, 0x105);
	}

	m_k052109->tilemap_draw(screen, bitmap, cliprect, m_sorted_layer[2], 0, 4);

	if (m_layerpri[2] >= 0x30)
	{
		m_k053936->zoom_draw(screen, bitmap, cliprect, m_roz_tilemap, 0, 1, 1);
		m_glfgreat_pixel = bitmap.pix(0x80, 0x105);
	}

	m_k053245->sprites_draw(bitmap, cliprect, screen.priority());
	return 0;
}

uint32_t tmnt2_state::screen_update_tmnt2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	double brt;
	int i, newdim, newen, cb, ce;

	newdim = m_dim_v | ((~m_dim_c & 0x10) >> 1);
	newen  = (m_k053251->get_priority(5) && m_k053251->get_priority(5) != 0x3e);

	if (newdim != m_lastdim || newen != m_lasten)
	{
		brt = 1.0;
		if (newen)
			brt -= (1.0 - PALETTE_DEFAULT_SHADOW_FACTOR) * newdim / 8;
		m_lastdim = newdim;
		m_lasten = newen;

		/*
		    Only affect the background and sprites, not text layer.
		    Instead of dimming each layer we dim the entire palette
		    except text colors because palette bases may change
		    anytime and there's no guarantee a dimmed color will be
		    reset properly.
		*/

		// find the text layer's palette range
		cb = m_layer_colorbase[m_sorted_layer[2]] << 4;
		ce = cb + 128;

		// dim all colors before it
		for (i = 0; i < cb; i++)
			m_palette->set_pen_contrast(i, brt);

		// reset all colors in range
		for (i = cb; i < ce; i++)
			m_palette->set_pen_contrast(i, 1.0);

		// dim all colors after it
		for (i = ce; i < 2048; i++)
			m_palette->set_pen_contrast(i, brt);

		// toggle shadow/highlight
		if (~m_dim_c & 0x10)
			m_palette->set_shadow_mode(1);
		else
			m_palette->set_shadow_mode(0);
	}

	screen_update_lgtnfght(screen, bitmap, cliprect);
	return 0;
}


uint32_t tmnt2_state::screen_update_thndrx2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// update color info and refresh tilemaps
	static const int K053251_CI[3] = { k053251_device::CI2, k053251_device::CI4, k053251_device::CI3 };
	int bg_colorbase = m_k053251->get_palette_index(k053251_device::CI0);
	m_sprite_colorbase = m_k053251->get_palette_index(k053251_device::CI1);

	for (int i = 0; i < 3; i++)
	{
		int prev_colorbase = m_layer_colorbase[i];
		m_layer_colorbase[i] = m_k053251->get_palette_index(K053251_CI[i]);

		if (m_layer_colorbase[i] != prev_colorbase)
			m_k052109->mark_tilemap_dirty(i);
	}

	m_k052109->tilemap_update();

	// sort layers and draw
	for (int i = 0; i < 3; i++)
	{
		m_sorted_layer[i] = i;
		m_layerpri[i] = m_k053251->get_priority(K053251_CI[i]);
	}

	konami_sortlayers3(m_sorted_layer, m_layerpri);

	screen.priority().fill(0, cliprect);
	bitmap.fill(16 * bg_colorbase, cliprect);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, m_sorted_layer[0], 0, 1);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, m_sorted_layer[1], 0, 2);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, m_sorted_layer[2], 0, 4);

	m_k051960->k051960_sprites_draw(bitmap, cliprect, screen.priority(), -1, -1);
	return 0;
}



/***************************************************************************

  Housekeeping

***************************************************************************/

void tmnt2_state::screen_vblank_blswhstl(int state)
{
	// on rising edge
	if (state)
	{
		m_k053245->clear_buffer();
	}
}

void tmnt2_state::punkshot_main_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x080000, 0x083fff).ram(); /* main RAM */
	map(0x090000, 0x090fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x0a0000, 0x0a0001).portr("DSW1_DSW2");
	map(0x0a0002, 0x0a0003).portr("COINS_DSW3");
	map(0x0a0004, 0x0a0005).portr("P3_P4");
	map(0x0a0006, 0x0a0007).portr("P1_P2");
	map(0x0a0020, 0x0a0021).w(FUNC(tmnt2_state::punkshot_0a0020_w));
	map(0x0a0040, 0x0a0043).rw(m_k053260, FUNC(k053260_device::main_read), FUNC(k053260_device::main_write)).umask16(0x00ff);
	map(0x0a0060, 0x0a007f).w(m_k053251, FUNC(k053251_device::write)).umask16(0x00ff);
	map(0x0a0080, 0x0a0081).w("watchdog", FUNC(watchdog_timer_device::reset16_w));
	map(0x100000, 0x107fff).rw(FUNC(tmnt2_state::k052109_word_noA12_r), FUNC(tmnt2_state::k052109_word_noA12_w));
	map(0x110000, 0x110007).rw(m_k051960, FUNC(k051960_device::k051937_r), FUNC(k051960_device::k051937_w));
	map(0x110400, 0x1107ff).rw(m_k051960, FUNC(k051960_device::k051960_r), FUNC(k051960_device::k051960_w));
	map(0xfffffc, 0xffffff).r(FUNC(tmnt2_state::punkshot_kludge_r));
}

void tmnt2_state::lgtnfght_main_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x080000, 0x080fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x090000, 0x093fff).ram(); /*main RAM */
	map(0x0a0000, 0x0a0001).portr("COINS");
	map(0x0a0002, 0x0a0003).portr("P1");
	map(0x0a0004, 0x0a0005).portr("P2");
	map(0x0a0006, 0x0a0007).portr("DSW1");
	map(0x0a0008, 0x0a0009).portr("DSW2");
	map(0x0a0010, 0x0a0011).portr("DSW3");
	map(0x0a0018, 0x0a0019).w(FUNC(tmnt2_state::lgtnfght_0a0018_w));
	map(0x0a0020, 0x0a0023).rw(m_k053260, FUNC(k053260_device::main_read), FUNC(k053260_device::main_write)).umask16(0x00ff);
	map(0x0a0028, 0x0a0029).w("watchdog", FUNC(watchdog_timer_device::reset16_w));
	map(0x0b0000, 0x0b3fff).rw(FUNC(tmnt2_state::k053245_scattered_word_r), FUNC(tmnt2_state::k053245_scattered_word_w)).share("spriteram");
	map(0x0c0000, 0x0c001f).rw(FUNC(tmnt2_state::k053244_word_noA1_r), FUNC(tmnt2_state::k053244_word_noA1_w));
	map(0x0e0000, 0x0e001f).w(m_k053251, FUNC(k053251_device::write)).umask16(0x00ff);
	map(0x100000, 0x107fff).rw(FUNC(tmnt2_state::k052109_word_noA12_r), FUNC(tmnt2_state::k052109_word_noA12_w));
}

void tmnt2_state::ssriders_soundkludge_w(uint16_t dat)
{
	/* I think this is more than just a trigger */
	m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0xff); // Z80
}

void tmnt2_state::blswhstl_main_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x180000, 0x183fff).rw(FUNC(tmnt2_state::k052109_word_r), FUNC(tmnt2_state::k052109_word_w));
	map(0x204000, 0x207fff).ram(); /* main RAM */
	map(0x300000, 0x303fff).rw(FUNC(tmnt2_state::k053245_scattered_word_r), FUNC(tmnt2_state::k053245_scattered_word_w)).share("spriteram");
	map(0x400000, 0x400fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x500000, 0x50003f).m(m_k054000, FUNC(k054000_device::map)).umask16(0x00ff);
	map(0x680000, 0x68001f).rw(FUNC(tmnt2_state::k053244_word_noA1_r), FUNC(tmnt2_state::k053244_word_noA1_w));
	map(0x700000, 0x700001).portr("P1");
	map(0x700002, 0x700003).portr("P2");
	map(0x700004, 0x700005).r(FUNC(tmnt2_state::blswhstl_coin_r));
	map(0x700006, 0x700007).portr("EEPROM");
	map(0x700200, 0x700201).w(FUNC(tmnt2_state::blswhstl_eeprom_w));
	map(0x700300, 0x700301).w(FUNC(tmnt2_state::blswhstl_700300_w));
	map(0x700400, 0x700401).rw("watchdog", FUNC(watchdog_timer_device::reset16_r), FUNC(watchdog_timer_device::reset16_w));
	map(0x780600, 0x780603).rw(m_k053260, FUNC(k053260_device::main_read), FUNC(k053260_device::main_write)).umask16(0x00ff);
	map(0x780604, 0x780605).w(FUNC(tmnt2_state::ssriders_soundkludge_w));
	map(0x780700, 0x78071f).w(m_k053251, FUNC(k053251_device::write)).umask16(0x00ff);
}

uint8_t glfgreat_state::controller_r()
{
	return m_analog_controller[m_controller_select]->read();
}

void glfgreat_state::glfgreat_main_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x100000, 0x103fff).ram(); /* main RAM */
	map(0x104000, 0x107fff).rw(FUNC(glfgreat_state::k053245_scattered_word_r), FUNC(glfgreat_state::k053245_scattered_word_w)).share("spriteram");
	map(0x108000, 0x108fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x10c000, 0x10cfff).rw(m_k053936, FUNC(k053936_device::linectrl_r), FUNC(k053936_device::linectrl_w));  /* 053936? */
	map(0x110000, 0x11001f).w(FUNC(glfgreat_state::k053244_word_noA1_w));              /* duplicate! */
	map(0x114000, 0x11401f).rw(m_k053245, FUNC(k05324x_device::k053244_r), FUNC(k05324x_device::k053244_w)).umask16(0x00ff);    /* duplicate! */
	map(0x118000, 0x11801f).w(m_k053936, FUNC(k053936_device::ctrl_w));
	map(0x11c000, 0x11c01f).w(m_k053251, FUNC(k053251_device::write)).umask16(0xff00);
	map(0x120000, 0x120001).portr("P1_P2");
	map(0x120002, 0x120003).portr("P3_P4");
	map(0x120004, 0x120005).portr("COINS_DSW3");
	map(0x120006, 0x120007).portr("DSW1_DSW2");
	map(0x121000, 0x121001).r(FUNC(glfgreat_state::glfgreat_ball_r));   /* returns the color of the center pixel of the roz layer */
	map(0x122000, 0x122001).w(FUNC(glfgreat_state::glfgreat_122000_w));
	map(0x123000, 0x123000).rw("adc", FUNC(adc0804_device::read), FUNC(adc0804_device::write));
	map(0x124000, 0x124001).w("watchdog", FUNC(watchdog_timer_device::reset16_w));
	map(0x125000, 0x125003).r(m_k053260, FUNC(k053260_device::main_read)).umask16(0xff00).w(FUNC(glfgreat_state::glfgreat_sound_w)).umask16(0xff00);
	map(0x200000, 0x207fff).rw(FUNC(glfgreat_state::k052109_word_noA12_r), FUNC(glfgreat_state::k052109_word_noA12_w));
	map(0x300000, 0x3fffff).r(FUNC(glfgreat_state::glfgreat_rom_r));
}

void prmrsocr_state::prmrsocr_main_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x103fff).ram(); /* main RAM */
	map(0x104000, 0x107fff).rw(FUNC(prmrsocr_state::k053245_scattered_word_r), FUNC(prmrsocr_state::k053245_scattered_word_w)).share("spriteram");
	map(0x108000, 0x108fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x10c000, 0x10cfff).rw(m_k053936, FUNC(k053936_device::linectrl_r), FUNC(k053936_device::linectrl_w));
	map(0x110000, 0x11001f).w(FUNC(prmrsocr_state::k053244_word_noA1_w));              /* duplicate! */
	map(0x114000, 0x11401f).rw(m_k053245, FUNC(k05324x_device::k053244_r), FUNC(k05324x_device::k053244_w)).umask16(0x00ff);    /* duplicate! */
	map(0x118000, 0x11801f).w(m_k053936, FUNC(k053936_device::ctrl_w));
	map(0x11c000, 0x11c01f).w(m_k053251, FUNC(k053251_device::write)).umask16(0xff00);
	map(0x120000, 0x120001).portr("P1_COINS");
	map(0x120002, 0x120003).portr("P2_EEPROM");
	map(0x121000, 0x12101f).m("k054321", FUNC(k054321_device::main_map)).umask16(0x00ff);
	map(0x122000, 0x122001).w(FUNC(prmrsocr_state::prmrsocr_eeprom_w));    /* EEPROM + video control */
	map(0x123000, 0x123001).w(FUNC(prmrsocr_state::prmrsocr_sound_irq_w));
	map(0x200000, 0x207fff).rw(FUNC(prmrsocr_state::k052109_word_noA12_r), FUNC(prmrsocr_state::k052109_word_noA12_w));
	map(0x280000, 0x280001).w("watchdog", FUNC(watchdog_timer_device::reset16_w));
	map(0x300000, 0x33ffff).r(FUNC(prmrsocr_state::prmrsocr_rom_r));
}


#if 1
inline uint32_t tmnt2_state::tmnt2_get_word( uint32_t addr )
{
	if (addr <= 0x07ffff / 2)
		return(m_tmnt2_rom[addr]);
	else if (addr >= 0x104000 / 2 && addr <= 0x107fff / 2)
		return(m_sunset_104000[addr - 0x104000 / 2]);
	else if (addr >= 0x180000 / 2 && addr <= 0x183fff / 2)
		return(m_spriteram[addr - 0x180000 / 2]);
	return 0;
}

void tmnt2_state::tmnt2_put_word( uint32_t addr, uint16_t data )
{
	uint32_t offs;
	if (addr >= 0x180000 / 2 && addr <= 0x183fff / 2)
	{
		m_spriteram[addr - 0x180000 / 2] = data;
		offs = addr - 0x180000 / 2;
		if (!(offs & 0x0031))
		{
			offs = ((offs & 0x000e) >> 1) | ((offs & 0x1fc0) >> 3);
			m_k053245->k053245_word_w(offs, data, 0xffff);
		}
	}
	else if (addr >= 0x104000 / 2 && addr <= 0x107fff / 2)
		m_sunset_104000[addr - 0x104000 / 2] = data;
}

void tmnt2_state::tmnt2_1c0800_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint32_t src_addr, dst_addr, mod_addr, attr1, code, attr2, cbase, cmod, color;
	int xoffs, yoffs, xmod, ymod, zmod, xzoom, yzoom, i;
	uint16_t *mcu;
	uint16_t src[4], mod[24];
	uint8_t keepaspect, xlock, ylock, zlock;

	COMBINE_DATA(&m_tmnt2_1c0800[offset]);

	if (offset != 0x18/2 || !ACCESSING_BITS_8_15)
		return;

	mcu = m_tmnt2_1c0800;
	if ((mcu[8] & 0xff00) != 0x8200)
		return;

	src_addr = (mcu[0] | (mcu[1] & 0xff) << 16) >> 1;
	dst_addr = (mcu[2] | (mcu[3] & 0xff) << 16) >> 1;
	mod_addr = (mcu[4] | (mcu[5] & 0xff) << 16) >> 1;
	zlock    = (mcu[8] & 0xff) == 0x0001;

	for (i = 0; i < 4; i++)
		src[i] = tmnt2_get_word(src_addr + i);
	for (i = 0; i < 24; i++) mod[i] =
		tmnt2_get_word(mod_addr + i);

	code = src[0];          // code

	i = src[1];
	attr1 = i >> 2 & 0x3f00;    // flip y, flip x and sprite size
	attr2 = i & 0x380;      // mirror y, mirror x, shadow
	cbase = i & 0x01f;      // base color
	cmod  = mod[0x2a / 2] >> 8;
	color = (cbase != 0x0f && cmod <= 0x1f && !zlock) ? cmod : cbase;

	xoffs = (int16_t)src[2];  // local x
	yoffs = (int16_t)src[3];  // local y

	i = mod[0];
	attr2 |= i & 0x0060;    // priority
	keepaspect = (i & 0x0014) == 0x0014;
	if (i & 0x8000) { attr1 |= 0x8000; }    // active
	if (keepaspect) { attr1 |= 0x4000; }    // keep aspect
//  if (i & 0x????) { attr1 ^= 0x2000; yoffs = -yoffs; }    // flip y (not used?)
	if (i & 0x4000) { attr1 ^= 0x1000; xoffs = -xoffs; }    // flip x

	xmod = (int16_t)mod[6];   // global x
	ymod = (int16_t)mod[7];   // global y
	zmod = (int16_t)mod[8];   // global z
	xzoom = mod[0x1c / 2];
	yzoom = (keepaspect) ? xzoom : mod[0x1e / 2];

	ylock = xlock = (i & 0x0020 && (!xzoom || xzoom == 0x100));

	/*
	    Scale factor is non-linear. The zoom vales are looked-up from
	    two to three nested tables and passed through a series of math
	    operations. The MCU is suspected to have its own tables for
	    translating zoom values to final scale factors or it knows where
	    to fetch them in ROM. There is no access to its internal code so
	    the scale curve is only approximated.

	    The most accurate method is to trace how MCU zoom is transformed
	    from ROM data, reverse the maths, plug the result into the sprite
	    zoom code and derive the scale factor from there; but zooming
	    would still suffer from precision loss in k053245->sprites_draw()
	    and drawgfx() producing gaps in logical sprite groups.

	    A few sample points on the real curve:

	     Zoom | Scale factor
	    ------+--------------
	     0    | 0.0
	     0x2c | 0x40/0x8d
	     0x2f | 0x40/0x80
	     0x4f | 1.0
	     0x60 | 0x40/0x2f
	     0x7b | 0x40/0x14
	*/
	if (!xlock)
	{
		i = xzoom - 0x4f00;
		if (i > 0)
		{
			i >>= 8;
			xoffs += (int)(pow(i, /*1.898461*/1.891292) * xoffs / 599.250121);
		}
		else if (i < 0)
		{
			i = (i >> 3) + (i >> 4) + (i >> 5) + (i >> 6) + xzoom;
			xoffs = (i > 0) ? (xoffs * i / 0x4f00) : 0;
		}
	}
	if (!ylock)
	{
		i = yzoom - 0x4f00;
		if (i > 0)
		{
			i >>= 8;
			yoffs += (int)(pow(i, /*1.898461*/1.891292) * yoffs / 599.250121);
		}
		else if (i < 0)
		{
			i = (i >> 3) + (i >> 4) + (i >> 5) + (i >> 6) + yzoom;
			yoffs = (i > 0) ? (yoffs * i / 0x4f00) : 0;
		}

	}
	if (!zlock) yoffs += zmod;
	xoffs += xmod;
	yoffs += ymod;

	tmnt2_put_word(dst_addr +  0, attr1);
	tmnt2_put_word(dst_addr +  2, code);
	tmnt2_put_word(dst_addr +  4, (uint32_t)yoffs);
	tmnt2_put_word(dst_addr +  6, (uint32_t)xoffs);
	tmnt2_put_word(dst_addr + 12, attr2 | color);
}
#else // for reference; do not remove
void tmnt2_state::tmnt2_1c0800_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(m_tmnt2_1c0800 + offset);
	if (offset == 0x0008 && (m_tmnt2_1c0800[0x8] & 0xff00) == 0x8200)
	{
		uint32_t CellSrc;
		uint32_t CellVar;
		uint16_t *src;
		int dst;
		int x,y;

		CellVar = m_tmnt2_1c0800[0x04] | (m_tmnt2_1c0800[0x05] << 16 );
		dst = m_tmnt2_1c0800[0x02] | (m_tmnt2_1c0800[0x03] << 16 );
		CellSrc = m_tmnt2_1c0800[0x00] | (m_tmnt2_1c0800[0x01] << 16 );
//        if (CellDest >= 0x180000 && CellDest < 0x183fe0) {
		CellVar -= 0x104000;
		src = (uint16_t *)(memregion("maincpu")->base() + CellSrc);

		CellVar >>= 1;

		write_word(dst + 0x00, 0x8000 | ((src[1] & 0xfc00) >> 2));  /* size, flip xy */
		write_word(dst + 0x04, src[0]); /* code */
		write_word(dst + 0x18, (src[1] & 0x3ff) ^       /* color, mirror, priority */
				(sunset_104000[CellVar + 0x00] & 0x0060));

		/* base color modifier */
		/* TODO: this is wrong, e.g. it breaks the explosions when you kill an */
		/* enemy, or surfs in the sewer level (must be blue for all enemies). */
		/* It fixes the enemies, though, they are not all purple when you throw them around. */
		/* Also, the bosses don't blink when they are about to die - don't know */
		/* if this is correct or not. */
//      if (m_sunset_104000[CellVar + 0x15] & 0x001f)
//          dst + 0x18->write_word((read_word(dst + 0x18) & 0xffe0) |
//                  (m_sunset_104000[CellVar + 0x15] & 0x001f));

		x = src[2];
		if (m_sunset_104000[CellVar + 0x00] & 0x4000)
		{
			/* flip x */
			write_word(dst + 0x00, read_word(dst + 0x00) ^ 0x1000);
			x = -x;
		}
		x += m_sunset_104000[CellVar + 0x06];
		write_word(dst + 0x0c, x);
		y = src[3];
		y += m_sunset_104000[CellVar + 0x07];
		/* don't do second offset for shadows */
		if ((m_tmnt2_1c0800[0x08] & 0x00ff) != 0x01)
			y += m_sunset_104000[CellVar + 0x08];
		write_word(dst + 0x08, y);
#if 0
logerror("copy command %04x sprite %08x data %08x: %04x%04x %04x%04x  modifiers %08x:%04x%04x %04x%04x %04x%04x %04x%04x %04x%04x %04x%04x %04x%04x %04x%04x %04x%04x %04x%04x %04x%04x %04x%04x\n",
	m_tmnt2_1c0800[0x05],
	CellDest,CellSrc,
	src[0], src[1], src[2], src[3],
	CellVar*2,
	m_sunset_104000[CellVar + 0x00],
	m_sunset_104000[CellVar + 0x01],
	m_sunset_104000[CellVar + 0x02],
	m_sunset_104000[CellVar + 0x03],
	m_sunset_104000[CellVar + 0x04],
	m_sunset_104000[CellVar + 0x05],
	m_sunset_104000[CellVar + 0x06],
	m_sunset_104000[CellVar + 0x07],
	m_sunset_104000[CellVar + 0x08],
	m_sunset_104000[CellVar + 0x09],
	m_sunset_104000[CellVar + 0x0a],
	m_sunset_104000[CellVar + 0x0b],
	m_sunset_104000[CellVar + 0x0c],
	m_sunset_104000[CellVar + 0x0d],
	m_sunset_104000[CellVar + 0x0e],
	m_sunset_104000[CellVar + 0x0f],
	m_sunset_104000[CellVar + 0x10],
	m_sunset_104000[CellVar + 0x11],
	m_sunset_104000[CellVar + 0x12],
	m_sunset_104000[CellVar + 0x13],
	m_sunset_104000[CellVar + 0x14],
	m_sunset_104000[CellVar + 0x15],
	m_sunset_104000[CellVar + 0x16],
	m_sunset_104000[CellVar + 0x17]
	);
#endif
//        }
	}
}
#endif

void tmnt2_state::tmnt2_main_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x104000, 0x107fff).ram().share("sunset_104000");   /* main RAM */
	map(0x140000, 0x140fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x180000, 0x183fff).ram().w(FUNC(tmnt2_state::k053245_scattered_word_w)).share("spriteram");   // k053245_scattered_word_r
	map(0x1c0000, 0x1c0001).portr("P1");
	map(0x1c0002, 0x1c0003).portr("P2");
	map(0x1c0004, 0x1c0005).portr("P3");
	map(0x1c0006, 0x1c0007).portr("P4");
	map(0x1c0100, 0x1c0101).portr("COINS");
	map(0x1c0102, 0x1c0103).r(FUNC(tmnt2_state::ssriders_eeprom_r));
	map(0x1c0200, 0x1c0201).w(FUNC(tmnt2_state::ssriders_eeprom_w));    /* EEPROM and gfx control */
	map(0x1c0300, 0x1c0301).w(FUNC(tmnt2_state::ssriders_1c0300_w));
	map(0x1c0400, 0x1c0401).rw("watchdog", FUNC(watchdog_timer_device::reset16_r), FUNC(watchdog_timer_device::reset16_w));
	map(0x1c0500, 0x1c057f).ram(); /* TMNT2 only (1J) unknown, mostly MCU blit offsets */
//  map(0x1c0800, 0x1c0801).r(FUNC(tmnt2_state::ssriders_protection_r)); /* protection device */
	map(0x1c0800, 0x1c081f).w(FUNC(tmnt2_state::tmnt2_1c0800_w)).share("tmnt2_1c0800");  /* protection device */
	map(0x5a0000, 0x5a001f).rw(FUNC(tmnt2_state::k053244_word_noA1_r), FUNC(tmnt2_state::k053244_word_noA1_w));
	map(0x5c0600, 0x5c0603).rw(m_k053260, FUNC(k053260_device::main_read), FUNC(k053260_device::main_write)).umask16(0x00ff);
	map(0x5c0604, 0x5c0605).w(FUNC(tmnt2_state::ssriders_soundkludge_w));
	map(0x5c0700, 0x5c071f).w(m_k053251, FUNC(k053251_device::write)).umask16(0x00ff);
	map(0x600000, 0x603fff).rw(FUNC(tmnt2_state::k052109_word_r), FUNC(tmnt2_state::k052109_word_w));
}

void tmnt2_state::ssriders_main_map(address_map &map)
{
	map(0x000000, 0x0bffff).rom();
	map(0x104000, 0x107fff).ram(); /* main RAM */
	map(0x140000, 0x140fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x180000, 0x183fff).rw(FUNC(tmnt2_state::k053245_scattered_word_r), FUNC(tmnt2_state::k053245_scattered_word_w)).share("spriteram");
	map(0x1c0000, 0x1c0001).portr("P1");
	map(0x1c0002, 0x1c0003).portr("P2");
	map(0x1c0004, 0x1c0005).portr("P3");
	map(0x1c0006, 0x1c0007).portr("P4");
	map(0x1c0100, 0x1c0101).portr("COINS");
	map(0x1c0102, 0x1c0103).r(FUNC(tmnt2_state::ssriders_eeprom_r));
	map(0x1c0200, 0x1c0201).w(FUNC(tmnt2_state::ssriders_eeprom_w));    /* EEPROM and gfx control */
	map(0x1c0300, 0x1c0301).w(FUNC(tmnt2_state::ssriders_1c0300_w));
	map(0x1c0400, 0x1c0401).rw("watchdog", FUNC(watchdog_timer_device::reset16_r), FUNC(watchdog_timer_device::reset16_w));
	map(0x1c0500, 0x1c057f).ram(); /* TMNT2 only (1J) unknown */
	map(0x1c0800, 0x1c0801).r(FUNC(tmnt2_state::ssriders_protection_r));
	map(0x1c0800, 0x1c0803).w(FUNC(tmnt2_state::ssriders_protection_w));
	map(0x5a0000, 0x5a001f).rw(FUNC(tmnt2_state::k053244_word_noA1_r), FUNC(tmnt2_state::k053244_word_noA1_w));
	map(0x5c0600, 0x5c0603).rw(m_k053260, FUNC(k053260_device::main_read), FUNC(k053260_device::main_write)).umask16(0x00ff);
	map(0x5c0604, 0x5c0605).w(FUNC(tmnt2_state::ssriders_soundkludge_w));
	map(0x5c0700, 0x5c071f).w(m_k053251, FUNC(k053251_device::write)).umask16(0x00ff);
	map(0x600000, 0x603fff).rw(FUNC(tmnt2_state::k052109_word_r), FUNC(tmnt2_state::k052109_word_w));
}

void sunsetbl_state::sunsetbl_main_map(address_map &map)
{
	map(0x000000, 0x0bffff).rom();
	map(0x104000, 0x107fff).ram(); /* main RAM */
	map(0x14c000, 0x14cfff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x14e700, 0x14e71f).w(m_k053251, FUNC(k053251_device::write)).umask16(0x00ff);
	map(0x180000, 0x183fff).rw(FUNC(sunsetbl_state::k053245_scattered_word_r), FUNC(sunsetbl_state::k053245_scattered_word_w)).share("spriteram");
	map(0x184000, 0x18ffff).ram();
	map(0x1c0300, 0x1c0301).w(FUNC(sunsetbl_state::ssriders_1c0300_w));
	map(0x1c0400, 0x1c0401).nopw();
	map(0x5a0000, 0x5a001f).rw(FUNC(sunsetbl_state::k053244_word_noA1_r), FUNC(sunsetbl_state::k053244_word_noA1_w));
	map(0x600000, 0x603fff).rw(FUNC(sunsetbl_state::k052109_word_r), FUNC(sunsetbl_state::k052109_word_w));
	map(0x604020, 0x60402f).nopw();    /* written every frame */
	map(0x604201, 0x604201).w(FUNC(sunsetbl_state::irq_enable_w));
	map(0x6119e2, 0x6119e3).nopw();    /* written a lot in some test menus (PC=18204) */
	map(0xc00000, 0xc00001).portr("P1");
	map(0xc00002, 0xc00003).portr("P2");
	map(0xc00004, 0xc00005).portr("P3");
	map(0xc00006, 0xc00007).portr("P4");
	map(0xc00200, 0xc00201).w(FUNC(sunsetbl_state::ssriders_eeprom_w));    /* EEPROM and gfx control */
	map(0xc00404, 0xc00405).portr("COINS");
	map(0xc00406, 0xc00407).r(FUNC(sunsetbl_state::sunsetbl_eeprom_r));
	map(0xc00601, 0xc00601).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x75d288, 0x75d289).nopr(); // read repeatedly in some test menus (PC=181f2)
}


void tmnt2_state::thndrx2_main_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x100000, 0x103fff).ram(); /* main RAM */
	map(0x200000, 0x200fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x300000, 0x30001f).w(m_k053251, FUNC(k053251_device::write)).umask16(0x00ff);
	map(0x400000, 0x400003).rw(m_k053260, FUNC(k053260_device::main_read), FUNC(k053260_device::main_write)).umask16(0x00ff);
	map(0x500000, 0x50003f).m(m_k054000, FUNC(k054000_device::map)).umask16(0x00ff);
	map(0x500100, 0x500101).w(FUNC(tmnt2_state::thndrx2_eeprom_w));
	map(0x500200, 0x500201).portr("P1_COINS");
	map(0x500202, 0x500203).r(FUNC(tmnt2_state::thndrx2_eeprom_r));
	map(0x500300, 0x500301).nopw();    /* watchdog reset? irq enable? */
	map(0x600000, 0x607fff).rw(FUNC(tmnt2_state::k052109_word_noA12_r), FUNC(tmnt2_state::k052109_word_noA12_w));
	map(0x700000, 0x700007).rw(m_k051960, FUNC(k051960_device::k051937_r), FUNC(k051960_device::k051937_w));
	map(0x700400, 0x7007ff).rw(m_k051960, FUNC(k051960_device::k051960_r), FUNC(k051960_device::k051960_w));
}

void tmnt2_state::punkshot_audio_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xf000, 0xf7ff).ram();
	map(0xf800, 0xf801).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xfa00, 0xfa00).w(FUNC(tmnt2_state::sound_arm_nmi_w));
	map(0xfc00, 0xfc2f).rw(m_k053260, FUNC(k053260_device::read), FUNC(k053260_device::write));
}


void tmnt2_state::lgtnfght_audio_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0xa000, 0xa001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xc000, 0xc02f).rw(m_k053260, FUNC(k053260_device::read), FUNC(k053260_device::write));
}


void glfgreat_state::glfgreat_audio_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xf000, 0xf7ff).ram();
	map(0xf800, 0xf82f).rw(m_k053260, FUNC(k053260_device::read), FUNC(k053260_device::write));
	map(0xfa00, 0xfa00).w(FUNC(glfgreat_state::sound_arm_nmi_w));
}


void tmnt2_state::ssriders_audio_map(address_map &map)
{
	map(0x0000, 0xefff).rom();
	map(0xf000, 0xf7ff).ram();
	map(0xf800, 0xf801).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xfa00, 0xfa2f).rw(m_k053260, FUNC(k053260_device::read), FUNC(k053260_device::write));
	map(0xfc00, 0xfc00).w(FUNC(tmnt2_state::sound_arm_nmi_w));
}


void tmnt2_state::thndrx2_audio_map(address_map &map)
{
	map(0x0000, 0xefff).rom();
	map(0xf000, 0xf7ff).ram();
	map(0xf800, 0xf801).mirror(0x0010).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xfa00, 0xfa00).w(FUNC(tmnt2_state::sound_arm_nmi_w));
	map(0xfc00, 0xfc2f).rw(m_k053260, FUNC(k053260_device::read), FUNC(k053260_device::write));
}


void prmrsocr_state::prmrsocr_audio_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr("bank1");
	map(0xc000, 0xdfff).ram();
	map(0xe000, 0xe12f).lrw8(
			NAME([this](offs_t offset) { return m_k054539->read(((offset & 0x100) << 1) | (offset & 0xff)); }),
			NAME([this](offs_t offset, u8 data) { m_k054539->write(((offset & 0x100) << 1) | (offset & 0xff), data); }));
	map(0xf000, 0xf003).m("k054321", FUNC(k054321_device::sound_map));
	map(0xf800, 0xf800).w(FUNC(prmrsocr_state::prmrsocr_audio_bankswitch_w));
}


static INPUT_PORTS_START( punkshtj ) // Japan 2 Players
	PORT_START("DSW1_DSW2")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "No Coin B", SW1)
	PORT_DIPUNUSED_DIPLOC( 0x0100, IP_ACTIVE_LOW, "SW2:1" ) // manual says "not used", but doesn't "should be kept OFF"
	PORT_DIPUNUSED_DIPLOC( 0x0200, IP_ACTIVE_LOW, "SW2:2" ) // manual says "not used", but doesn't "should be kept OFF"
	PORT_DIPNAME( 0x0c00, 0x0800, "Period Length" ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0c00, "1 Minutes" )
	PORT_DIPSETTING(      0x0800, "2 Minutes" )
	PORT_DIPSETTING(      0x0400, "3 Minutes" )
	PORT_DIPSETTING(      0x0000, "4 Minutes" )
	PORT_DIPUNUSED_DIPLOC( 0x1000, IP_ACTIVE_LOW, "SW2:5" ) // manual says "not used", but doesn't "should be kept OFF"
	PORT_DIPNAME( 0x6000, 0x4000, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(      0x6000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Difficult ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("COINS_DSW3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x2000, IP_ACTIVE_LOW, "SW3:2" ) // manual says "not used and should be kept OFF"
	PORT_SERVICE_DIPLOC( 0x4000, IP_ACTIVE_LOW, "SW3:3" )
	PORT_DIPNAME( 0x8000, 0x8000, "Freeze" ) PORT_DIPLOCATION("SW3:4") // manual says "not used and should be kept OFF"
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("P1_P2")
	KONAMI16_LSB( 1, IPT_UNKNOWN, IPT_UNKNOWN )
	KONAMI16_MSB( 2, IPT_UNKNOWN, IPT_UNKNOWN )

	PORT_START("P3_P4")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( punkshtj4 ) // FICTITIOUS Japan 4 Players
	PORT_INCLUDE( punkshtj )

	PORT_MODIFY("COINS_DSW3")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_SERVICE4 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("P3_P4")
	KONAMI16_LSB( 3, IPT_UNKNOWN, IPT_UNKNOWN )
	KONAMI16_MSB( 4, IPT_UNKNOWN, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( punksht_us_coinage )
	PORT_MODIFY("DSW1_DSW2")
	PORT_DIPNAME( 0x000f, 0x000f, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x000f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x000e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0x000d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x000b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x000a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0009, DEF_STR( 1C_7C ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Continue" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, "1 Coin" )
	PORT_DIPUNUSED_DIPLOC( 0x0020, IP_ACTIVE_LOW, "SW1:6" )
	PORT_DIPUNUSED_DIPLOC( 0x0040, IP_ACTIVE_LOW, "SW1:7" )
	PORT_DIPUNUSED_DIPLOC( 0x0080, IP_ACTIVE_LOW, "SW1:8" )
	// US manual says
	// Set No. 5, 6, 7, 8 OFF in Dip Switch No. 1
	// Put Dip Switch No. 5 to ON to give
	// "1 coin = CONTINUE"
INPUT_PORTS_END

static INPUT_PORTS_START( punkshot ) // US 4 Players set1
	PORT_INCLUDE( punkshtj4 )
	PORT_INCLUDE( punksht_us_coinage )

	PORT_MODIFY("DSW1_DSW2")
	PORT_DIPNAME( 0x0300, 0x0300, "Energy" ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0300, "30" )
	PORT_DIPSETTING(      0x0200, "40" )
	PORT_DIPSETTING(      0x0100, "50" )
	PORT_DIPSETTING(      0x0000, "60" )
	PORT_DIPNAME( 0x0c00, 0x0800, "Period Length" ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0c00, "2 Minutes" )
	PORT_DIPSETTING(      0x0800, "3 Minutes" )
	PORT_DIPSETTING(      0x0400, "4 Minutes" )
	PORT_DIPSETTING(      0x0000, "5 Minutes" )
	PORT_DIPNAME( 0x6000, 0x6000, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(      0x6000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Difficult ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Difficult ) )
INPUT_PORTS_END

static INPUT_PORTS_START( punksht2 ) // US 2 Players set2
	PORT_INCLUDE( punkshtj )
	PORT_INCLUDE( punksht_us_coinage )

	PORT_MODIFY("DSW1_DSW2")
	PORT_DIPNAME( 0x0300, 0x0300, "Energy" ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0300, "40" )
	PORT_DIPSETTING(      0x0200, "50" )
	PORT_DIPSETTING(      0x0100, "60" )
	PORT_DIPSETTING(      0x0000, "70" )
	PORT_DIPNAME( 0x0c00, 0x0c00, "Period Length" ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0c00, "3 Minutes" )
	PORT_DIPSETTING(      0x0800, "4 Minutes" )
	PORT_DIPSETTING(      0x0400, "5 Minutes" )
	PORT_DIPSETTING(      0x0000, "6 Minutes" )
	PORT_DIPNAME( 0x6000, 0x6000, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(      0x6000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Difficult ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( lgtnfght )
	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )   /* vblank? checked during boot */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1")
	KONAMI16_LSB( 1, IPT_BUTTON3, IPT_UNKNOWN )

	PORT_START("P2")
	KONAMI16_LSB( 2, IPT_BUTTON3, IPT_UNKNOWN )

	PORT_START("DSW2")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "No Coin B", SW1)
	/* "No Coin B" = coins produce sound, but no effect on coin counter */

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW2:3" ) // manual says "not used"
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "100K, 400K" )
	PORT_DIPSETTING(    0x10, "150K, 500K" )
	PORT_DIPSETTING(    0x08, "200K Only" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Sound" ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Mono ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Stereo ) )
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW3:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW3:4" ) // manual says "not used"
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( trigon )
	PORT_INCLUDE( lgtnfght )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x18, 0x10, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "100K, 400K" ) // US factory default
	PORT_DIPSETTING(    0x10, "150K, 500K" ) // JP factory default
	PORT_DIPSETTING(    0x08, "200K Only" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
INPUT_PORTS_END

static INPUT_PORTS_START( blswhstl )
	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* VBLANK? OBJMPX? */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1")
	KONAMI16_LSB( 1, IPT_UNKNOWN, IPT_UNKNOWN )

	PORT_START("P2")
	KONAMI16_LSB( 2, IPT_UNKNOWN, IPT_UNKNOWN )

	PORT_START("EEPROM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::do_read))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::ready_read))
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::di_write))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::cs_write))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::clk_write))
INPUT_PORTS_END

static INPUT_PORTS_START( glfgreat )
	PORT_START("DSW1_DSW2")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "No Coin B", SW1)
	/* "No Coin B" = coins produce sound, but no effect on coin counter */
	PORT_DIPNAME( 0x0300, 0x0100, "Players/Controllers" ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0300, "4/1" ) // Upright
	PORT_DIPSETTING(      0x0200, "4/2" ) // Upright (P1&P3=1stCtrl P2&P4=2ndCtrl)
	PORT_DIPSETTING(      0x0100, "4/4" ) // Upright
	PORT_DIPSETTING(      0x0000, "3/3" ) // Upright
	PORT_DIPNAME( 0x0400, 0x0000, "Sound" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Mono ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Stereo ) )
	PORT_DIPNAME( 0x1800, 0x1800, "Initial/Maximum Credit" ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(      0x1800, "2/3" )
	PORT_DIPSETTING(      0x1000, "2/4" )
	PORT_DIPSETTING(      0x0800, "2/5" )
	PORT_DIPSETTING(      0x0000, "3/5" )
	PORT_DIPNAME( 0x6000, 0x4000, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(      0x6000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Difficult ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("COINS_DSW3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE1 ) /* service coin */
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_SERVICE2) PORT_NAME(DEF_STR(Test))
	PORT_DIPNAME( 0x0800, 0x0000, "Freeze" )    /* ?? VBLANK ?? */
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x2000, IP_ACTIVE_LOW, "SW3:2" ) // manual says "not used"
	PORT_SERVICE_DIPLOC( 0x4000, IP_ACTIVE_LOW, "SW3:3" )
	PORT_DIPUNUSED_DIPLOC( 0x8000, IP_ACTIVE_LOW, "SW3:4" ) // manual says "not used"

	PORT_START("P1_P2")
	KONAMI16_LSB_40( 1, IPT_BUTTON3 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("adc", FUNC(adc0804_device::intr_r)) // shown in service mode DIP SW1:9, SW2:9 and SW3:5
	KONAMI16_MSB( 2, IPT_BUTTON3, IPT_UNUSED ) PORT_PLAYER(2)

	PORT_START("P3_P4")
	KONAMI16_LSB( 3, IPT_BUTTON3, IPT_UNUSED ) PORT_PLAYER(3)
	KONAMI16_MSB( 4, IPT_BUTTON3, IPT_UNUSED ) PORT_PLAYER(4)

	// actually unused by World/US sets but still tested in service mode
	PORT_START("CONTROLA")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(35) PORT_KEYDELTA(35) PORT_PLAYER(1)

	PORT_START("CONTROLB")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(35) PORT_KEYDELTA(35) PORT_PLAYER(2)

	PORT_START("CONTROLC")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(35) PORT_KEYDELTA(35) PORT_PLAYER(3)

	PORT_START("CONTROLD")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(35) PORT_KEYDELTA(35) PORT_PLAYER(4)
INPUT_PORTS_END

static INPUT_PORTS_START( glfgreatu )
	PORT_INCLUDE( glfgreat )

	PORT_MODIFY("DSW1_DSW2")
	PORT_DIPNAME( 0x1800, 0x1000, "Initial/Maximum Credit" ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(      0x1800, "2/2" )
	PORT_DIPSETTING(      0x1000, "2/3" )
	PORT_DIPSETTING(      0x0800, "2/4" )
	PORT_DIPSETTING(      0x0000, "2/5" )
INPUT_PORTS_END

static INPUT_PORTS_START( glfgreatj )
	PORT_INCLUDE( glfgreatu )

	PORT_MODIFY("DSW1_DSW2")
	PORT_DIPNAME( 0x0300, 0x0100, "Players/Controllers" ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0300, "2/1" ) // Upright
	PORT_DIPSETTING(      0x0200, "2/2" ) // Upright
	PORT_DIPSETTING(      0x0100, "4/2" ) // Cocktail (P1&P3 <-> P2&P4)
	PORT_DIPSETTING(      0x0000, "4/4" ) // Cocktail (P1&P2 <-> P3&P4)

	// I/O test in service mode actually returns same mapping as World/US revs
	// for accuracy we actually map these like the Jp flyer claims
	// (where stance button is on top of the ball shaped controller)
	PORT_MODIFY("P1_P2")
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Stance Select Button")
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Left Direction Button")
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 Right Direction Button")
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("P1 Club Select Button")
	PORT_BIT(  0x0100, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(  0x0200, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(  0x0400, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Stance Select Button")
	PORT_BIT(  0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(  0x1000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Left Direction Button")
	PORT_BIT(  0x2000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 Right Direction Button")
	PORT_BIT(  0x4000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("P2 Club Select Button")

	PORT_MODIFY("P3_P4")
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3) PORT_NAME("P3 Stance Select Button")
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_UNUSED  )
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3) PORT_NAME("P3 Left Direction Button")
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3) PORT_NAME("P3 Right Direction Button")
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(3) PORT_NAME("P3 Club Select Button")
	PORT_BIT(  0x0100, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(  0x0200, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(  0x0400, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4) PORT_NAME("P4 Stance Select Button")
	PORT_BIT(  0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(  0x1000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4) PORT_NAME("P4 Left Direction Button")
	PORT_BIT(  0x2000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4) PORT_NAME("P4 Right Direction Button")
	PORT_BIT(  0x4000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(4) PORT_NAME("P4 Club Select Button")
INPUT_PORTS_END

static INPUT_PORTS_START( ssriders )
	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1")
	KONAMI16_LSB( 1, IPT_UNKNOWN, IPT_START1 )

	PORT_START("P2")
	KONAMI16_LSB( 2, IPT_UNKNOWN, IPT_START2 )

	PORT_START("P3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("EEPROM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::do_read))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::ready_read))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* ?? TMNT2: OBJMPX */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))  /* ?? TMNT2: NVBLK */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* ?? TMNT2: IPL0 */
	PORT_BIT( 0x60, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* unused? */
	PORT_SERVICE_NO_TOGGLE( 0x80, IP_ACTIVE_LOW )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::di_write))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::cs_write))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::clk_write))
INPUT_PORTS_END

static INPUT_PORTS_START( ssridr4p )
	PORT_INCLUDE( ssriders )

	PORT_MODIFY("COINS")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE4 )

	PORT_MODIFY("P1")
	KONAMI16_LSB( 1, IPT_UNKNOWN, IPT_UNKNOWN )

	PORT_MODIFY("P2")
	KONAMI16_LSB( 2, IPT_UNKNOWN, IPT_UNKNOWN )

	PORT_MODIFY("P3")
	KONAMI16_LSB( 3, IPT_UNKNOWN, IPT_UNKNOWN )

	PORT_MODIFY("P4")
	KONAMI16_LSB( 4, IPT_UNKNOWN, IPT_UNKNOWN )

INPUT_PORTS_END

/* Same as 'ssridr4p', but additional Start button for each player.  Seemingly only needed in sets with *U* region/version codes (EG: ADD, UDA).
   COIN3, COIN4, SERVICE3 and SERVICE4 only have an effect in the "test mode". */
static INPUT_PORTS_START( ssrid4ps )

	PORT_INCLUDE( ssridr4p )

	PORT_MODIFY("P1")
	KONAMI16_LSB( 1, IPT_UNKNOWN, IPT_START1 )

	PORT_MODIFY("P2")
	KONAMI16_LSB( 2, IPT_UNKNOWN, IPT_START2 )

	PORT_MODIFY("P3")
	KONAMI16_LSB( 3, IPT_UNKNOWN, IPT_START3 )

	PORT_MODIFY("P4")
	KONAMI16_LSB( 4, IPT_UNKNOWN, IPT_START4 )

INPUT_PORTS_END

/* Version for the bootleg, which has the service switch a little different */
static INPUT_PORTS_START( sunsetbl )

	PORT_INCLUDE( ssrid4ps )

	PORT_MODIFY("EEPROM")
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* unused? */

INPUT_PORTS_END

static INPUT_PORTS_START( qgakumon )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) // Joystick control : Left
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) // Joystick control : Right
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) // Joystick control : Up
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) // Joystick control : Down
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) // Joystick control : Button
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) // Joystick control : Left
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) // Joystick control : Right
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) // Joystick control : Up
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) // Joystick control : Down
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2) // Joystick control : Button
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("P3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("EEPROM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::do_read))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::ready_read))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* ?? TMNT2: OBJMPX */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))  /* ?? TMNT2: NVBLK (needs to be ACTIVE_HIGH to avoid problems) */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* ?? TMNT2: IPL0 */
	PORT_BIT( 0x60, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* unused? */
	PORT_SERVICE_NO_TOGGLE( 0x80, IP_ACTIVE_LOW )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::di_write))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::cs_write))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::clk_write))
INPUT_PORTS_END

static INPUT_PORTS_START( thndrx2 )
	PORT_START("P1_COINS")
	KONAMI16_LSB( 1, IPT_UNKNOWN, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0800, IP_ACTIVE_LOW )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2_EEPROM")
	KONAMI16_LSB( 2, IPT_UNKNOWN, IPT_START2 )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::do_read))
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::ready_read))
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )  /* VBLK?? */
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::di_write))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::cs_write))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::clk_write))
INPUT_PORTS_END

static INPUT_PORTS_START( prmrsocr )
	PORT_START("P1_COINS")
	KONAMI16_LSB( 1, IPT_UNKNOWN, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0200, IP_ACTIVE_LOW )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x1000, 0x0000, "Sound" ) PORT_DIPLOCATION("SW:1")
	PORT_DIPSETTING(      0x1000, DEF_STR( Mono ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Stereo ) )
	PORT_DIPUNUSED_DIPLOC( 0x2000, IP_ACTIVE_LOW, "SW:2" ) // manual says "not used"
	PORT_DIPUNUSED_DIPLOC( 0x4000, IP_ACTIVE_LOW, "SW:3" ) // manual says "not used"
	PORT_DIPUNUSED_DIPLOC( 0x8000, IP_ACTIVE_LOW, "SW:4" ) // manual says "not used"

	PORT_START("P2_EEPROM")
	KONAMI16_LSB( 2, IPT_UNKNOWN, IPT_START2 )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::do_read))
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::ready_read))
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::di_write))
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::cs_write))
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_er5911_device::clk_write))
INPUT_PORTS_END


void tmnt2_state::machine_start()
{
	save_item(NAME(m_toggle));
	save_item(NAME(m_last));
	save_item(NAME(m_sprite_colorbase));
	save_item(NAME(m_layer_colorbase));
	save_item(NAME(m_layerpri));
	save_item(NAME(m_sorted_layer));

	m_nmi_blocked = timer_alloc(timer_expired_delegate());
}

void sunsetbl_state::machine_start()
{
	tmnt2_state::machine_start();

	save_item(NAME(m_irq_enable));
}

void tmnt2_state::machine_reset()
{
	m_toggle = 0;
	m_last = 0;

	if (m_audiocpu && m_k053260)
	{
		// Z80 _NMI goes low at same time as reset
		m_audiocpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
		m_audiocpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
	}
}

void tmnt2_state::punkshot(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(24'000'000)/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &tmnt2_state::punkshot_main_map);

	Z80(config, m_audiocpu, XTAL(3'579'545));
	m_audiocpu->set_addrmap(AS_PROGRAM, &tmnt2_state::punkshot_audio_map);
	/* NMIs are generated by the 053260 */

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_video_attributes(VIDEO_UPDATE_AFTER_VBLANK);
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(64*8, 32*8);
	screen.set_visarea(14*8, (64-14)*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(tmnt2_state::screen_update_punkshot));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 2048);
	m_palette->enable_shadows();
	m_palette->enable_hilights();

	K052109(config, m_k052109, 0);
	m_k052109->set_palette(m_palette);
	m_k052109->set_screen("screen");
	m_k052109->set_tile_callback(FUNC(tmnt2_state::tmnt_tile_callback));
	m_k052109->irq_handler().set_inputline(m_maincpu, M68K_IRQ_4);

	K051960(config, m_k051960, 0);
	m_k051960->set_palette(m_palette);
	m_k051960->set_screen("screen");
	m_k051960->set_sprite_callback(FUNC(tmnt2_state::punkshot_sprite_callback));

	K053251(config, m_k053251, 0);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	YM2151(config, "ymsnd", XTAL(3'579'545)).add_route(0, "mono", 1.0).add_route(1, "mono", 1.0);

	K053260(config, m_k053260, XTAL(3'579'545));
	m_k053260->add_route(ALL_OUTPUTS, "mono", 0.70);
	m_k053260->sh1_cb().set(FUNC(tmnt2_state::z80_nmi_w));
}

void tmnt2_state::lgtnfght(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(24'000'000)/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &tmnt2_state::lgtnfght_main_map);

	Z80(config, m_audiocpu, XTAL(3'579'545));
	m_audiocpu->set_addrmap(AS_PROGRAM, &tmnt2_state::lgtnfght_audio_map);

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_video_attributes(VIDEO_UPDATE_AFTER_VBLANK);
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(64*8, 32*8);
	screen.set_visarea(12*8, (64-12)*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(tmnt2_state::screen_update_lgtnfght));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 2048);
	m_palette->enable_shadows();
	m_palette->enable_hilights();

	MCFG_VIDEO_START_OVERRIDE(tmnt2_state,lgtnfght)

	K052109(config, m_k052109, 0);
	m_k052109->set_palette(m_palette);
	m_k052109->set_screen("screen");
	m_k052109->set_tile_callback(FUNC(tmnt2_state::tmnt_tile_callback));
	m_k052109->irq_handler().set_inputline(m_maincpu, M68K_IRQ_5);

	K053245(config, m_k053245, 0);
	m_k053245->set_palette(m_palette);
	m_k053245->set_sprite_callback(FUNC(tmnt2_state::lgtnfght_sprite_callback));

	K053251(config, m_k053251, 0);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	YM2151(config, "ymsnd", XTAL(3'579'545)).add_route(0, "speaker", 1.0, 0).add_route(1, "speaker", 1.0, 1);

	K053260(config, m_k053260, XTAL(3'579'545));
	m_k053260->add_route(0, "speaker", 0.70, 0);
	m_k053260->add_route(1, "speaker", 0.70, 1);
	m_k053260->sh1_cb().set(FUNC(tmnt2_state::z80_nmi_w));
}

void tmnt2_state::blswhstl(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(32'000'000)/2);       /* Confirmed */
	m_maincpu->set_addrmap(AS_PROGRAM, &tmnt2_state::blswhstl_main_map);

	Z80(config, m_audiocpu, XTAL(3'579'545));
	m_audiocpu->set_addrmap(AS_PROGRAM, &tmnt2_state::ssriders_audio_map);
	/* NMIs are generated by the 053260 */

	EEPROM_ER5911_8BIT(config, "eeprom");

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_video_attributes(VIDEO_UPDATE_AFTER_VBLANK);
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(64*8, 32*8);
	screen.set_visarea(12*8, (64-12)*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(tmnt2_state::screen_update_lgtnfght));
	screen.screen_vblank().set(FUNC(tmnt2_state::screen_vblank_blswhstl));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 2048);
	m_palette->enable_shadows();
	m_palette->enable_hilights();

	MCFG_VIDEO_START_OVERRIDE(tmnt2_state, blswhstl)

	K052109(config, m_k052109, 0);
	m_k052109->set_palette(m_palette);
	m_k052109->set_screen("screen");
	m_k052109->set_tile_callback(FUNC(tmnt2_state::blswhstl_tile_callback));
	m_k052109->irq_handler().set_inputline(m_maincpu, M68K_IRQ_4);

	K053245(config, m_k053245, 0);
	m_k053245->set_palette(m_palette);
	m_k053245->set_sprite_callback(FUNC(tmnt2_state::blswhstl_sprite_callback));

	K053251(config, m_k053251, 0);
	K054000(config, m_k054000, 0);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	YM2151(config, "ymsnd", XTAL(3'579'545)).add_route(0, "speaker", 0.70, 0).add_route(1, "speaker", 0.70, 1);

	K053260(config, m_k053260, XTAL(3'579'545));
	m_k053260->add_route(0, "speaker", 0.50, 1);   /* fixed inverted stereo channels */
	m_k053260->add_route(1, "speaker", 0.50, 0);
	m_k053260->sh1_cb().set(FUNC(tmnt2_state::z80_nmi_w));
}



static GFXDECODE_START( gfx_glfgreat )
	GFXDECODE_ENTRY( "zoom", 0, gfx_16x16x4_packed_lsb, 0x400, 16 )
GFXDECODE_END

void glfgreat_state::glfgreat(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(32'000'000)/2);       /* Confirmed */
	m_maincpu->set_addrmap(AS_PROGRAM, &glfgreat_state::glfgreat_main_map);

	Z80(config, m_audiocpu, XTAL(3'579'545));
	m_audiocpu->set_addrmap(AS_PROGRAM, &glfgreat_state::glfgreat_audio_map);
	/* NMIs are generated by the 053260 */

	WATCHDOG_TIMER(config, "watchdog");

	adc0804_device &adc(ADC0804(config, "adc", RES_K(10), CAP_P(150)));
	adc.vin_callback().set(FUNC(glfgreat_state::controller_r));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_video_attributes(VIDEO_UPDATE_AFTER_VBLANK);
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(64*8, 32*8);
	screen.set_visarea(14*8, (64-14)*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(glfgreat_state::screen_update_glfgreat));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_glfgreat);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 2048);
	m_palette->enable_shadows();
	m_palette->enable_hilights();

	MCFG_VIDEO_START_OVERRIDE(glfgreat_state,glfgreat)

	K052109(config, m_k052109, 0);
	m_k052109->set_palette(m_palette);
	m_k052109->set_screen("screen");
	m_k052109->set_tile_callback(FUNC(glfgreat_state::tmnt_tile_callback));
	m_k052109->irq_handler().set_inputline(m_maincpu, M68K_IRQ_5);

	K053245(config, m_k053245, 0);
	m_k053245->set_palette(m_palette);
	m_k053245->set_sprite_callback(FUNC(glfgreat_state::lgtnfght_sprite_callback));

	K053936(config, m_k053936, 0);
	m_k053936->set_wrap(1);
	m_k053936->set_offsets(85, 0);

	K053251(config, m_k053251, 0);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	K053260(config, m_k053260, XTAL(3'579'545));
	m_k053260->add_route(0, "speaker", 1.0, 1);
	m_k053260->add_route(1, "speaker", 1.0, 0);
	m_k053260->sh1_cb().set(FUNC(glfgreat_state::z80_nmi_w));
}

void prmrsocr_state::machine_start()
{
	tmnt2_state::machine_start();
	uint8_t *ROM = memregion("audiocpu")->base();
	m_audiobank->configure_entries(0, 8, &ROM[0x10000], 0x4000);
}

void prmrsocr_state::prmrsocr(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(32'000'000)/2);       /* Confirmed */
	m_maincpu->set_addrmap(AS_PROGRAM, &prmrsocr_state::prmrsocr_main_map);

	Z80(config, m_audiocpu, 8000000);  /* ? */
	m_audiocpu->set_addrmap(AS_PROGRAM, &prmrsocr_state::prmrsocr_audio_map);
	/* NMIs are generated by the 054539 */

	EEPROM_ER5911_8BIT(config, "eeprom");

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_video_attributes(VIDEO_UPDATE_AFTER_VBLANK);
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(64*8, 32*8);
	screen.set_visarea(14*8, (64-14)*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(prmrsocr_state::screen_update_glfgreat));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_glfgreat);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 2048);
	m_palette->enable_shadows();
	m_palette->enable_hilights();

	MCFG_VIDEO_START_OVERRIDE(prmrsocr_state,prmrsocr)

	K052109(config, m_k052109, 0);
	m_k052109->set_palette(m_palette);
	m_k052109->set_screen("screen");
	m_k052109->set_tile_callback(FUNC(prmrsocr_state::tmnt_tile_callback));
	m_k052109->irq_handler().set_inputline(m_maincpu, M68K_IRQ_5);

	K053245(config, m_k053245, 0);
	m_k053245->set_palette(m_palette);
	m_k053245->set_sprite_callback(FUNC(prmrsocr_state::prmrsocr_sprite_callback));

	K053936(config, m_k053936, 0);
	m_k053936->set_offsets(85, 1);

	K053251(config, m_k053251, 0);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	K054321(config, "k054321", "speaker");

	K054539(config, m_k054539, XTAL(18'432'000));
	m_k054539->timer_handler().set_inputline("audiocpu", INPUT_LINE_NMI);
	m_k054539->add_route(0, "speaker", 1.0, 0);
	m_k054539->add_route(1, "speaker", 1.0, 1);
}

void tmnt2_state::tmnt2(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(32'000'000)/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &tmnt2_state::tmnt2_main_map);

	Z80(config, m_audiocpu, 8000000);
	/* 8 MHz; clock is correct, but there's 1 cycle wait for ROM/RAM access. */
	/* Access speed of ROM/RAM used on the machine is 150ns, without the wait, they cannot run on 8MHz. */
	/* We are not emulating the wait state, so the ROM test ends at 02 instead of 00. */
	m_audiocpu->set_addrmap(AS_PROGRAM, &tmnt2_state::ssriders_audio_map);
	/* NMIs are generated by the 053260 */

	EEPROM_ER5911_8BIT(config, "eeprom");

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_video_attributes(VIDEO_UPDATE_AFTER_VBLANK);
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(64*8, 32*8);
	screen.set_visarea(13*8, (64-13)*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(tmnt2_state::screen_update_tmnt2));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 2048);
	m_palette->enable_shadows();
	m_palette->enable_hilights();

	MCFG_VIDEO_START_OVERRIDE(tmnt2_state,lgtnfght)

	K052109(config, m_k052109, 0);
	m_k052109->set_palette(m_palette);
	m_k052109->set_screen("screen");
	m_k052109->set_tile_callback(FUNC(tmnt2_state::tmnt_tile_callback));
	m_k052109->irq_handler().set_inputline(m_maincpu, M68K_IRQ_4);

	K053245(config, m_k053245, 0);
	m_k053245->set_palette(m_palette);
	m_k053245->set_sprite_callback(FUNC(tmnt2_state::lgtnfght_sprite_callback));

	K053251(config, m_k053251, 0);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	YM2151(config, "ymsnd", XTAL(3'579'545)).add_route(0, "speaker", 1.0, 0).add_route(1, "speaker", 1.0, 1);

	K053260(config, m_k053260, XTAL(3'579'545));
	m_k053260->add_route(0, "speaker", 0.75, 0);
	m_k053260->add_route(1, "speaker", 0.75, 1);
	m_k053260->sh1_cb().set(FUNC(tmnt2_state::z80_nmi_w));
}

void tmnt2_state::ssriders(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(32'000'000)/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &tmnt2_state::ssriders_main_map);

	Z80(config, m_audiocpu, 8000000);
	m_audiocpu->set_addrmap(AS_PROGRAM, &tmnt2_state::ssriders_audio_map);
	/* NMIs are generated by the 053260 */

	EEPROM_ER5911_8BIT(config, "eeprom");

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_video_attributes(VIDEO_UPDATE_AFTER_VBLANK);
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(64*8, 32*8);
	screen.set_visarea(14*8, (64-14)*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(tmnt2_state::screen_update_tmnt2));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 2048);
	m_palette->enable_shadows();
	m_palette->enable_hilights();

	MCFG_VIDEO_START_OVERRIDE(tmnt2_state,lgtnfght)

	K052109(config, m_k052109, 0);
	m_k052109->set_palette(m_palette);
	m_k052109->set_screen("screen");
	m_k052109->set_tile_callback(FUNC(tmnt2_state::tmnt_tile_callback));
	m_k052109->irq_handler().set_inputline(m_maincpu, M68K_IRQ_4);

	K053245(config, m_k053245, 0);
	m_k053245->set_palette(m_palette);
	m_k053245->set_sprite_callback(FUNC(tmnt2_state::lgtnfght_sprite_callback));

	K053251(config, m_k053251, 0);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	YM2151(config, "ymsnd", XTAL(3'579'545)).add_route(0, "speaker", 1.0, 0).add_route(1, "speaker", 1.0, 1);

	K053260(config, m_k053260, XTAL(3'579'545));
	m_k053260->add_route(0, "speaker", 0.70, 0);
	m_k053260->add_route(1, "speaker", 0.70, 1);
	m_k053260->sh1_cb().set(FUNC(tmnt2_state::z80_nmi_w));
}

void sunsetbl_state::irq_enable_w(u8 data)
{
	m_irq_enable = BIT(data, 2);
	if (!m_irq_enable)
		m_maincpu->set_input_line(M68K_IRQ_4, CLEAR_LINE);
}

void sunsetbl_state::sunsetbl_vblank_w(int state)
{
	if (state && m_irq_enable)
		m_maincpu->set_input_line(M68K_IRQ_4, ASSERT_LINE);
}

void sunsetbl_state::sunsetbl(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 16000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &sunsetbl_state::sunsetbl_main_map);

	EEPROM_ER5911_8BIT(config, "eeprom");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_video_attributes(VIDEO_UPDATE_AFTER_VBLANK);
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(64*8, 32*8);
	screen.set_visarea(14*8, (64-14)*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(sunsetbl_state::screen_update_tmnt2));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(sunsetbl_state::sunsetbl_vblank_w));

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 2048);
	m_palette->enable_shadows();
	m_palette->enable_hilights();

	MCFG_VIDEO_START_OVERRIDE(sunsetbl_state,lgtnfght)

	K052109(config, m_k052109, 0);
	m_k052109->set_palette(m_palette);
	m_k052109->set_screen(nullptr);
	m_k052109->set_tile_callback(FUNC(sunsetbl_state::ssbl_tile_callback));

	K053245(config, m_k053245, 0);
	m_k053245->set_palette(m_palette);
	m_k053245->set_sprite_callback(FUNC(sunsetbl_state::lgtnfght_sprite_callback));

	K053251(config, m_k053251, 0);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	okim6295_device &oki(OKIM6295(config, "oki", 1056000, okim6295_device::PIN7_HIGH)); // clock frequency & pin 7 not verified
	oki.add_route(ALL_OUTPUTS, "speaker", 1.0, 0);
	oki.add_route(ALL_OUTPUTS, "speaker", 1.0, 1);
}

void tmnt2_state::thndrx2(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 12000000);   /* 12 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &tmnt2_state::thndrx2_main_map);

	Z80(config, m_audiocpu, XTAL(3'579'545));
	m_audiocpu->set_addrmap(AS_PROGRAM, &tmnt2_state::thndrx2_audio_map);
	/* NMIs are generated by the 053260 */

	EEPROM_ER5911_8BIT(config, "eeprom");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(64*8, 32*8);
	screen.set_visarea(14*8, (64-14)*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(tmnt2_state::screen_update_thndrx2));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 2048);
	m_palette->enable_shadows();
	m_palette->enable_hilights();

	K052109(config, m_k052109, 0);
	m_k052109->set_palette(m_palette);
	m_k052109->set_screen("screen");
	m_k052109->set_tile_callback(FUNC(tmnt2_state::tmnt_tile_callback));
	m_k052109->irq_handler().set_inputline(m_maincpu, M68K_IRQ_4);

	K051960(config, m_k051960, 0);
	m_k051960->set_palette(m_palette);
	m_k051960->set_screen("screen");
	m_k051960->set_sprite_callback(FUNC(tmnt2_state::thndrx2_sprite_callback));

	K053251(config, m_k053251, 0);
	K054000(config, m_k054000, 0);

	/* sound hardware */
	// NB: game defaults in mono
	SPEAKER(config, "speaker", 2).front();

	YM2151(config, "ymsnd", XTAL(3'579'545)).add_route(0, "speaker", 0.25, 0).add_route(1, "speaker", 0.25, 1);

	K053260(config, m_k053260, XTAL(3'579'545));
	m_k053260->add_route(0, "speaker", 0.50, 0);
	m_k053260->add_route(1, "speaker", 0.50, 1);
	m_k053260->sh1_cb().set(FUNC(tmnt2_state::z80_nmi_w));
}



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( punkshot )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 2*128k for 68000 code */
	ROM_LOAD16_BYTE( "907-j02.i7",   0x00000, 0x20000, CRC(dbb3a23b) SHA1(78f999f4e5b12641195a7f9f7fedf696e32ff0c0) )
	ROM_LOAD16_BYTE( "907-j03.i10",  0x00001, 0x20000, CRC(2151d1ab) SHA1(e71768142b903825f8104ffc90906b0d471599e0) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "907f01.e8",    0x0000, 0x8000, CRC(f040c484) SHA1(f76a739cacc0aba98a5bf85a48c81cef0d9bbfb4) )

	ROM_REGION( 0x80000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "907d06.e23",   0x000000, 0x40000, CRC(f5cc38f4) SHA1(e6dc9994582a08740dc2fcb30a38771053627d5f) )
	ROM_LOAD32_WORD( "907d05.e22",   0x000002, 0x40000, CRC(e25774c1) SHA1(74fda3b418b4b0064b5e660a93122b07f6d41416) )

	ROM_REGION( 0x200000, "k051960", 0 )    /* sprites */
	ROM_LOAD32_WORD( "907d07.k2",    0x000000, 0x100000, CRC(b0fe4543) SHA1(3be1caef29084063dd8754c1eecc34a2ec842415) )
	ROM_LOAD32_WORD( "907d08.k7",    0x000002, 0x100000, CRC(d5ac8d9d) SHA1(cb330be1c5c016465ef7048b3b29c65a741ee45b) )

	ROM_REGION( 0x80000, "k053260", 0 ) /* samples for 053260 */
	ROM_LOAD( "907d04.d3",    0x0000, 0x80000, CRC(090feb5e) SHA1(2394907b62ff0724c277642caf6375239249e2d7) )
ROM_END

ROM_START( punkshot2 )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 2*128k for 68000 code */
	ROM_LOAD16_BYTE( "907m02.i7",    0x00000, 0x20000, CRC(59e14575) SHA1(249fc98a2d5fa3e4779438c37d22c0256be8d3fa) )
	ROM_LOAD16_BYTE( "907m03.i10",   0x00001, 0x20000, CRC(adb14b1e) SHA1(c5db1c3b70ab3e53cd6a600b82bdccda4db05f90) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "907f01.e8",    0x0000, 0x8000, CRC(f040c484) SHA1(f76a739cacc0aba98a5bf85a48c81cef0d9bbfb4) )

	ROM_REGION( 0x80000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "907d06.e23",   0x000000, 0x40000, CRC(f5cc38f4) SHA1(e6dc9994582a08740dc2fcb30a38771053627d5f) )
	ROM_LOAD32_WORD( "907d05.e22",   0x000002, 0x40000, CRC(e25774c1) SHA1(74fda3b418b4b0064b5e660a93122b07f6d41416) )

	ROM_REGION( 0x200000, "k051960", 0 )    /* sprites */
	ROM_LOAD32_WORD( "907d07.k2",    0x000000, 0x100000, CRC(b0fe4543) SHA1(3be1caef29084063dd8754c1eecc34a2ec842415) )
	ROM_LOAD32_WORD( "907d08.k7",    0x000002, 0x100000, CRC(d5ac8d9d) SHA1(cb330be1c5c016465ef7048b3b29c65a741ee45b) )

	ROM_REGION( 0x80000, "k053260", 0 ) /* samples for the 053260 */
	ROM_LOAD( "907d04.d3",    0x0000, 0x80000, CRC(090feb5e) SHA1(2394907b62ff0724c277642caf6375239249e2d7) )
ROM_END

ROM_START( punkshot2e )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 2*128k for 68000 code */
	ROM_LOAD16_BYTE( "907_2.i7",    0x00000, 0x20000, CRC(aa55516c) SHA1(297202c3dc817b016b646341712e7e1805bc98a5) )
	ROM_LOAD16_BYTE( "907_3.i10",   0x00001, 0x20000, CRC(0d3aa3d5) SHA1(39a9f4aac9463a88f1f57dc9aeb5718793aaa2c1) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "907f01.e8",    0x0000, 0x8000, CRC(f040c484) SHA1(f76a739cacc0aba98a5bf85a48c81cef0d9bbfb4) )

	ROM_REGION( 0x80000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "907d06.e23",   0x000000, 0x40000, CRC(f5cc38f4) SHA1(e6dc9994582a08740dc2fcb30a38771053627d5f) )
	ROM_LOAD32_WORD( "907d05.e22",   0x000002, 0x40000, CRC(e25774c1) SHA1(74fda3b418b4b0064b5e660a93122b07f6d41416) )

	ROM_REGION( 0x200000, "k051960", 0 )    /* sprites */
	ROM_LOAD32_WORD( "907d07.k2",    0x000000, 0x100000, CRC(b0fe4543) SHA1(3be1caef29084063dd8754c1eecc34a2ec842415) )
	ROM_LOAD32_WORD( "907d08.k7",    0x000002, 0x100000, CRC(d5ac8d9d) SHA1(cb330be1c5c016465ef7048b3b29c65a741ee45b) )

	ROM_REGION( 0x80000, "k053260", 0 ) /* samples for the 053260 */
	ROM_LOAD( "907d04.d3",    0x0000, 0x80000, CRC(090feb5e) SHA1(2394907b62ff0724c277642caf6375239249e2d7) )
ROM_END

ROM_START( punkshotj )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 2*128k for 68000 code */
	ROM_LOAD16_BYTE( "907z02.i7",    0x00000, 0x20000, CRC(7a3a5c89) SHA1(240967b911df8939b048bbcdfcac668455fc82e9) )
	ROM_LOAD16_BYTE( "907z03.i10",   0x00001, 0x20000, CRC(22a3d9d6) SHA1(76f016435956088aa680297ee9ba0abda446a7bb) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "907f01.e8",    0x0000, 0x8000, CRC(f040c484) SHA1(f76a739cacc0aba98a5bf85a48c81cef0d9bbfb4) )

	ROM_REGION( 0x80000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "907d06.e23",   0x000000, 0x40000, CRC(f5cc38f4) SHA1(e6dc9994582a08740dc2fcb30a38771053627d5f) )
	ROM_LOAD32_WORD( "907d05.e22",   0x000002, 0x40000, CRC(e25774c1) SHA1(74fda3b418b4b0064b5e660a93122b07f6d41416) )

	ROM_REGION( 0x200000, "k051960", 0 )    /* sprites */
	ROM_LOAD32_WORD( "907d07.k2",    0x000000, 0x100000, CRC(b0fe4543) SHA1(3be1caef29084063dd8754c1eecc34a2ec842415) )
	ROM_LOAD32_WORD( "907d08.k7",    0x000002, 0x100000, CRC(d5ac8d9d) SHA1(cb330be1c5c016465ef7048b3b29c65a741ee45b) )

	ROM_REGION( 0x80000, "k053260", 0 ) /* samples for the 053260 */
	ROM_LOAD( "907d04.d3",    0x0000, 0x80000, CRC(090feb5e) SHA1(2394907b62ff0724c277642caf6375239249e2d7) )
ROM_END

ROM_START( punkshot2a ) // possibly hack of an undumped original version (copyright year removed and substituted with 'SRS')
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 2*128k for 68000 code */
	ROM_LOAD16_BYTE( "i7",    0x00000, 0x20000, CRC(f17400ad) SHA1(14408d4eca2ea5a021c208cbb811e51efbf33953) )
	ROM_LOAD16_BYTE( "i10",   0x00001, 0x20000, CRC(552668ec) SHA1(4317d9c0b2aabd603a5bd4981c764c09407b4b7e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "907f01.e8",    0x0000, 0x8000, CRC(f040c484) SHA1(f76a739cacc0aba98a5bf85a48c81cef0d9bbfb4) )

	ROM_REGION( 0x80000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "907d06.e23",   0x000000, 0x40000, CRC(f5cc38f4) SHA1(e6dc9994582a08740dc2fcb30a38771053627d5f) )
	ROM_LOAD32_WORD( "907d05.e22",   0x000002, 0x40000, CRC(e25774c1) SHA1(74fda3b418b4b0064b5e660a93122b07f6d41416) )

	ROM_REGION( 0x200000, "k051960", 0 )    /* sprites */
	ROM_LOAD32_WORD( "907d07.k2",    0x000000, 0x100000, CRC(b0fe4543) SHA1(3be1caef29084063dd8754c1eecc34a2ec842415) )
	ROM_LOAD32_WORD( "907d08.k7",    0x000002, 0x100000, CRC(d5ac8d9d) SHA1(cb330be1c5c016465ef7048b3b29c65a741ee45b) )

	ROM_REGION( 0x80000, "k053260", 0 ) /* samples for the 053260 */
	ROM_LOAD( "907d04.d3",    0x0000, 0x80000, CRC(090feb5e) SHA1(2394907b62ff0724c277642caf6375239249e2d7) )
ROM_END

ROM_START( lgtnfght )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 2*128k for 68000 code */
	ROM_LOAD16_BYTE( "939k02.e11",   0x00000, 0x20000, CRC(2dfefa53) SHA1(135f3d06b04f950d1afc5fc0f8237c7af0e426b5) )
	ROM_LOAD16_BYTE( "939k03.e15",   0x00001, 0x20000, CRC(14f0c454) SHA1(bc1fd3a58b493b443b93077014fdf37cf563e879) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "939e01.d7",    0x0000, 0x8000, CRC(4a5fc848) SHA1(878825e07c2718b7c923ad7c77daddf18cb28beb) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "939a07.k14",   0x000000, 0x80000, CRC(7955dfcf) SHA1(012644c1bfbe2e5d1c7ba25f29ebfde7dbfd1c0d) )
	ROM_LOAD32_WORD( "939a08.k19",   0x000002, 0x80000, CRC(ed95b385) SHA1(5aa5291cf1a8935b0a65ae10aa20b9cf9a138b03) )

	ROM_REGION( 0x100000, "k053245", 0 )    /* sprites */
	ROM_LOAD32_WORD( "939a06.k8", 0x000000, 0x80000, CRC(e393c206) SHA1(9b35fc6dba1f15c3d9d69ff5a4e1673c539aa533) )
	ROM_LOAD32_WORD( "939a05.k2", 0x000002, 0x80000, CRC(3662d47a) SHA1(789c3f07ce812902050970f48be5115b8e95bea0) )

	ROM_REGION( 0x80000, "k053260", 0 ) /* samples for the 053260 */
	ROM_LOAD( "939a04.c5",    0x0000, 0x80000, CRC(c24e2b6e) SHA1(affc142883c2383afd08dcf156e48709ceca49fd) )
ROM_END

ROM_START( lgtnfghtu )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 2*128k for 68000 code */
	ROM_LOAD16_BYTE( "939m02.e11",   0x00000, 0x20000, CRC(61a12184) SHA1(f6d82aa0a444f885fd1e5d3d1464798b639a1710) )
	ROM_LOAD16_BYTE( "939m03.e15",   0x00001, 0x20000, CRC(6db6659d) SHA1(def943b906eab68a0b86f9a28fb0b9a1f3b65e4c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "939e01.d7",    0x0000, 0x8000, CRC(4a5fc848) SHA1(878825e07c2718b7c923ad7c77daddf18cb28beb) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "939a07.k14",   0x000000, 0x80000, CRC(7955dfcf) SHA1(012644c1bfbe2e5d1c7ba25f29ebfde7dbfd1c0d) )
	ROM_LOAD32_WORD( "939a08.k19",   0x000002, 0x80000, CRC(ed95b385) SHA1(5aa5291cf1a8935b0a65ae10aa20b9cf9a138b03) )

	ROM_REGION( 0x100000, "k053245", 0 )    /* sprites */
	ROM_LOAD32_WORD( "939a06.k8", 0x000000, 0x80000, CRC(e393c206) SHA1(9b35fc6dba1f15c3d9d69ff5a4e1673c539aa533) )
	ROM_LOAD32_WORD( "939a05.k2", 0x000002, 0x80000, CRC(3662d47a) SHA1(789c3f07ce812902050970f48be5115b8e95bea0) )

	ROM_REGION( 0x80000, "k053260", 0 ) /* samples for the 053260 */
	ROM_LOAD( "939a04.c5",    0x0000, 0x80000, CRC(c24e2b6e) SHA1(affc142883c2383afd08dcf156e48709ceca49fd) )
ROM_END

ROM_START( lgtnfghta )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 2*128k for 68000 code */
	ROM_LOAD16_BYTE( "939j_02.e11",   0x00000, 0x20000, CRC(bbbb4a74) SHA1(5ba5bb1a5b48a99aafc2b5b5bafe755566eda0e9) )
	ROM_LOAD16_BYTE( "939j_03.e15",   0x00001, 0x20000, CRC(8d4da7b7) SHA1(0c5b0421ce6908eec458dcded3609d150a710b97) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "939e01.d7",    0x0000, 0x8000, CRC(4a5fc848) SHA1(878825e07c2718b7c923ad7c77daddf18cb28beb) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "939a07.k14",   0x000000, 0x80000, CRC(7955dfcf) SHA1(012644c1bfbe2e5d1c7ba25f29ebfde7dbfd1c0d) )
	ROM_LOAD32_WORD( "939a08.k19",   0x000002, 0x80000, CRC(ed95b385) SHA1(5aa5291cf1a8935b0a65ae10aa20b9cf9a138b03) )

	ROM_REGION( 0x100000, "k053245", 0 )    /* sprites */
	ROM_LOAD32_WORD( "939a06.k8", 0x000000, 0x80000, CRC(e393c206) SHA1(9b35fc6dba1f15c3d9d69ff5a4e1673c539aa533) )
	ROM_LOAD32_WORD( "939a05.k2", 0x000002, 0x80000, CRC(3662d47a) SHA1(789c3f07ce812902050970f48be5115b8e95bea0) )

	ROM_REGION( 0x80000, "k053260", 0 ) /* samples for the 053260 */
	ROM_LOAD( "939a04.c5",    0x0000, 0x80000, CRC(c24e2b6e) SHA1(affc142883c2383afd08dcf156e48709ceca49fd) )
ROM_END

ROM_START( trigon )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 2*128k for 68000 code */
	ROM_LOAD16_BYTE( "939j02.e11",   0x00000, 0x20000, CRC(38381d1b) SHA1(d4ddf883f61e5d48143cf467ba3c9c5b37f7e790) )
	ROM_LOAD16_BYTE( "939j03.e15",   0x00001, 0x20000, CRC(b5beddcd) SHA1(dc5d79793d5453f284bf7fd198ba7c4ab1fc09c3) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "939e01.d7",    0x0000, 0x8000, CRC(4a5fc848) SHA1(878825e07c2718b7c923ad7c77daddf18cb28beb) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "939a07.k14",   0x000000, 0x80000, CRC(7955dfcf) SHA1(012644c1bfbe2e5d1c7ba25f29ebfde7dbfd1c0d) )
	ROM_LOAD32_WORD( "939a08.k19",   0x000002, 0x80000, CRC(ed95b385) SHA1(5aa5291cf1a8935b0a65ae10aa20b9cf9a138b03) )

	ROM_REGION( 0x100000, "k053245", 0 )    /* sprites */
	ROM_LOAD32_WORD( "939a06.k8", 0x000000, 0x80000, CRC(e393c206) SHA1(9b35fc6dba1f15c3d9d69ff5a4e1673c539aa533) )
	ROM_LOAD32_WORD( "939a05.k2", 0x000002, 0x80000, CRC(3662d47a) SHA1(789c3f07ce812902050970f48be5115b8e95bea0) )

	ROM_REGION( 0x80000, "k053260", 0 ) /* samples for the 053260 */
	ROM_LOAD( "939a04.c5",    0x0000, 0x80000, CRC(c24e2b6e) SHA1(affc142883c2383afd08dcf156e48709ceca49fd) )
ROM_END

ROM_START( blswhstl )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 4*128k for 68000 code */
	ROM_LOAD16_BYTE( "060_l02.e09", 0x000000, 0x20000, CRC(e8b7b234) SHA1(65ae9faf34ed8ab71013acdc84e9429e5f5fb7a2) )
	ROM_LOAD16_BYTE( "060_l03.g09", 0x000001, 0x20000, CRC(3c26d281) SHA1(d348305ecd4457e023bcdbc39842096d23c455fb) )
	ROM_LOAD16_BYTE( "060_l09.e11", 0x040000, 0x20000, CRC(14628736) SHA1(87f7a65cffb87085b3e21043bd46fbb7db9266dd) )
	ROM_LOAD16_BYTE( "060_l10.g11", 0x040001, 0x20000, CRC(f738ad4a) SHA1(5aea4afa4bf935d3e92856eff745f61ed4d98165) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "060j01.f3",  0x0000, 0x10000, CRC(f9d9a673) SHA1(8e5631c20dc37913cc7fa84f7ef786ff1ef85f09) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD_SWAP( "060e07.k16",  0x000000, 0x080000, CRC(c400edf3) SHA1(3f507df8804c1774e2e213f5eb8be0aa7e818d65) )
	ROM_LOAD32_WORD_SWAP( "060e08.k12",  0x000002, 0x080000, CRC(70dddba1) SHA1(2acb94f249cf89b3d53798a6ee1c960f84a04d2e) )

	ROM_REGION( 0x100000, "k053245", 0 )    /* sprites */
	ROM_LOAD32_WORD_SWAP( "060e06.k7", 0x000000, 0x080000, CRC(09381492) SHA1(5a3008dec99a8e0043405e9c4f5145794b8606e0) )
	ROM_LOAD32_WORD_SWAP( "060e05.k3", 0x000002, 0x080000, CRC(32454241) SHA1(7a246b255ff30118c4f8e07e6ba03a22fd5ddc8a) )

	ROM_REGION( 0x100000, "k053260", 0 )    /* samples for the 053260 */
	ROM_LOAD( "060e04.d1",  0x0000, 0x100000, CRC(c680395d) SHA1(acde593a5ec501e89c8aaca6c4fbacf707a727e1) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "blswhstl.nv", 0x0000, 0x080, CRC(87434e3f) SHA1(458d21cfc0ef3415c0b09d8d748263b9218bdb24) )
ROM_END

ROM_START( blswhstla )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 4*128k for 68000 code */
	ROM_LOAD16_BYTE( "060_m02.e09", 0x000000, 0x20000, CRC(bc9dd08f) SHA1(463634e1d8f3419b840beef0cedfc9c060166d0c) )
	ROM_LOAD16_BYTE( "060_m03.g09", 0x000001, 0x20000, CRC(7b6ee4a4) SHA1(d6c9d60058accd6f9ac6c2b9306057efc3fee461) )
	ROM_LOAD16_BYTE( "060_m09.e11", 0x040000, 0x20000, CRC(14628736) SHA1(87f7a65cffb87085b3e21043bd46fbb7db9266dd) )
	ROM_LOAD16_BYTE( "060_m10.g11", 0x040001, 0x20000, CRC(f738ad4a) SHA1(5aea4afa4bf935d3e92856eff745f61ed4d98165) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "060j01.f3",  0x0000, 0x10000, CRC(f9d9a673) SHA1(8e5631c20dc37913cc7fa84f7ef786ff1ef85f09) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD_SWAP( "060e07.k16",  0x000000, 0x080000, CRC(c400edf3) SHA1(3f507df8804c1774e2e213f5eb8be0aa7e818d65) )
	ROM_LOAD32_WORD_SWAP( "060e08.k12",  0x000002, 0x080000, CRC(70dddba1) SHA1(2acb94f249cf89b3d53798a6ee1c960f84a04d2e) )

	ROM_REGION( 0x100000, "k053245", 0 )    /* sprites */
	ROM_LOAD32_WORD_SWAP( "060e06.k7", 0x000000, 0x080000, CRC(09381492) SHA1(5a3008dec99a8e0043405e9c4f5145794b8606e0) )
	ROM_LOAD32_WORD_SWAP( "060e05.k3", 0x000002, 0x080000, CRC(32454241) SHA1(7a246b255ff30118c4f8e07e6ba03a22fd5ddc8a) )

	ROM_REGION( 0x100000, "k053260", 0 )    /* samples for the 053260 */
	ROM_LOAD( "060e04.d1",  0x0000, 0x100000, CRC(c680395d) SHA1(acde593a5ec501e89c8aaca6c4fbacf707a727e1) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "blswhstl.nv", 0x0000, 0x080, CRC(87434e3f) SHA1(458d21cfc0ef3415c0b09d8d748263b9218bdb24) )
ROM_END

ROM_START( detatwin )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 4*128k for 68000 code */
	ROM_LOAD16_BYTE( "060_j02.e09", 0x000000, 0x20000, CRC(11b761ac) SHA1(1a143b0a43da48bdcfe085a2a9d1a2de0329fafd) )
	ROM_LOAD16_BYTE( "060_j03.g09", 0x000001, 0x20000, CRC(8d0b588c) SHA1(a444493557cc19c7828b40a54dac9165c1f5b541) )
	ROM_LOAD16_BYTE( "060_j09.e11", 0x040000, 0x20000, CRC(f2a5f15f) SHA1(4b8786e5ce0b895e6358e16e2a0a926325d0afcc) )
	ROM_LOAD16_BYTE( "060_j10.g11", 0x040001, 0x20000, CRC(36eefdbc) SHA1(a3ec5078779b4ab33edf32e04db3e221e52b36c7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "060j01.f3",  0x0000, 0x10000, CRC(f9d9a673) SHA1(8e5631c20dc37913cc7fa84f7ef786ff1ef85f09) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD_SWAP( "060e07.k16",  0x000000, 0x080000, CRC(c400edf3) SHA1(3f507df8804c1774e2e213f5eb8be0aa7e818d65) )
	ROM_LOAD32_WORD_SWAP( "060e08.k12",  0x000002, 0x080000, CRC(70dddba1) SHA1(2acb94f249cf89b3d53798a6ee1c960f84a04d2e) )

	ROM_REGION( 0x100000, "k053245", 0 )    /* sprites */
	ROM_LOAD32_WORD_SWAP( "060e06.k7", 0x000000, 0x080000, CRC(09381492) SHA1(5a3008dec99a8e0043405e9c4f5145794b8606e0) )
	ROM_LOAD32_WORD_SWAP( "060e05.k3", 0x000002, 0x080000, CRC(32454241) SHA1(7a246b255ff30118c4f8e07e6ba03a22fd5ddc8a) )

	ROM_REGION( 0x100000, "k053260", 0 )    /* samples for the 053260 */
	ROM_LOAD( "060e04.d1",  0x0000, 0x100000, CRC(c680395d) SHA1(acde593a5ec501e89c8aaca6c4fbacf707a727e1) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "blswhstl.nv", 0x0000, 0x080, CRC(87434e3f) SHA1(458d21cfc0ef3415c0b09d8d748263b9218bdb24) )
ROM_END

ROM_START( glfgreat )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 2*128k for 68000 code */
	ROM_LOAD16_BYTE( "061l02.1h",   0x000000, 0x20000, CRC(ac7399f4) SHA1(27f95bd41cb550ea0395a93138066896b834551e) )
	ROM_LOAD16_BYTE( "061l03.4h",   0x000001, 0x20000, CRC(77b0ff5c) SHA1(e47701402a9a6f69cfbc72de0fee4cbdd79fbc6e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "061f01.4e",    0x0000, 0x8000, CRC(ab9a2a57) SHA1(c92738b4d3754c2378cd1e6ae786faa0c5a65808) )

	// the tile and sprite ROMs are actually 16-bit ROMs on a 32-bit bus, but the data lines are
	// swapped so that D0-D7 and D16-D23 come from one ROM and D8-D15 and D24-D31 from the other
	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD16_BYTE( "061d14.12l", 0x000000, 0x080000, CRC(b9440924) SHA1(d12763f1c999cfa4f2d6f685a73c8c20204f9cbb) )
	ROM_LOAD16_BYTE( "061d13.12k", 0x000001, 0x080000, CRC(9f999f0b) SHA1(f83e3e9e44d7d5ba4c72f72db1ab9f98a0e80fe2) )

	ROM_REGION( 0x200000, "k053245", 0 )    /* sprites */
	ROM_LOAD16_BYTE( "061d11.3k", 0x000000, 0x100000, CRC(c45b66a3) SHA1(bfb7f9a39d195857893d3f04c28d5c89442c3ac7) )
	ROM_LOAD16_BYTE( "061d12.8k", 0x000001, 0x100000, CRC(d305ecd1) SHA1(28cba6b5eb56b6f5c01e9da341a5c0e2ed3cb407) )

	ROM_REGION( 0x180000, "zoom", 0 )   /* 053936 tiles */
	ROM_LOAD( "061b08.14g",   0x000000, 0x080000, CRC(6ab739c3) SHA1(37ed6c9b224189c183895517d6a72738fe92ecc4) )
	ROM_LOAD( "061b09.15g",   0x080000, 0x080000, CRC(42c7a603) SHA1(e98e484ca817ed65c7fb80a87d732e70d120676f) )
	ROM_LOAD( "061b10.17g",   0x100000, 0x080000, CRC(10f89ce7) SHA1(cf6a16ed0174db640780da4d11076efeb48a6119) )

	ROM_REGION( 0x120000, "user1", 0 )  /* 053936 tilemaps */
	ROM_LOAD( "061b07.18d",   0x000000, 0x080000, CRC(517887e2) SHA1(ff7aa0df2cda3c745a195879c71727352696ef3a) )
	ROM_LOAD( "061b06.16d",   0x080000, 0x080000, CRC(41ada2ad) SHA1(7b200e44e040e3d79f2603a02c9991b4655407d4) )
	ROM_LOAD( "061b05.15d",   0x100000, 0x020000, CRC(2456fb11) SHA1(e1bdb9f5983751d28addad6977a44df3d9899a14) )

	ROM_REGION( 0x100000, "k053260", 0 )    /* samples for the 053260 */
	ROM_LOAD( "061e04.1d",    0x0000, 0x100000, CRC(7921d8df) SHA1(19ca4850ec489cca245e90a41bfc22493cd52263) )
ROM_END

ROM_START( glfgreatu )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 2*128k for 68000 code */
	ROM_LOAD16_BYTE( "061k02.1h",   0x000000, 0x20000, CRC(7d7a4d43) SHA1(e942b256ccb155f86253888884f79db819e501da) )
	ROM_LOAD16_BYTE( "061k03.4h",   0x000001, 0x20000, CRC(3647999a) SHA1(3edd0aaa017800b5cd436399f12dc2f23882a8ce) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "061f01.4e",    0x0000, 0x8000, CRC(ab9a2a57) SHA1(c92738b4d3754c2378cd1e6ae786faa0c5a65808) )

	// the tile and sprite ROMs are actually 16-bit ROMs on a 32-bit bus, but the data lines are
	// swapped so that D0-D7 and D16-D23 come from one ROM and D8-D15 and D24-D31 from the other
	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD16_BYTE( "061d14.12l", 0x000000, 0x080000, CRC(b9440924) SHA1(d12763f1c999cfa4f2d6f685a73c8c20204f9cbb) )
	ROM_LOAD16_BYTE( "061d13.12k", 0x000001, 0x080000, CRC(9f999f0b) SHA1(f83e3e9e44d7d5ba4c72f72db1ab9f98a0e80fe2) )

	ROM_REGION( 0x200000, "k053245", 0 )    /* sprites */
	ROM_LOAD16_BYTE( "061d11.3k", 0x000000, 0x100000, CRC(c45b66a3) SHA1(bfb7f9a39d195857893d3f04c28d5c89442c3ac7) )
	ROM_LOAD16_BYTE( "061d12.8k", 0x000001, 0x100000, CRC(d305ecd1) SHA1(28cba6b5eb56b6f5c01e9da341a5c0e2ed3cb407) )

	ROM_REGION( 0x180000, "zoom", 0 )   /* 053936 tiles */
	ROM_LOAD( "061b08.14g",   0x000000, 0x080000, CRC(6ab739c3) SHA1(37ed6c9b224189c183895517d6a72738fe92ecc4) )
	ROM_LOAD( "061b09.15g",   0x080000, 0x080000, CRC(42c7a603) SHA1(e98e484ca817ed65c7fb80a87d732e70d120676f) )
	ROM_LOAD( "061b10.17g",   0x100000, 0x080000, CRC(10f89ce7) SHA1(cf6a16ed0174db640780da4d11076efeb48a6119) )

	ROM_REGION( 0x120000, "user1", 0 )  /* 053936 tilemaps */
	ROM_LOAD( "061b07.18d",   0x000000, 0x080000, CRC(517887e2) SHA1(ff7aa0df2cda3c745a195879c71727352696ef3a) )
	ROM_LOAD( "061b06.16d",   0x080000, 0x080000, CRC(41ada2ad) SHA1(7b200e44e040e3d79f2603a02c9991b4655407d4) )
	ROM_LOAD( "061b05.15d",   0x100000, 0x020000, CRC(2456fb11) SHA1(e1bdb9f5983751d28addad6977a44df3d9899a14) )

	ROM_REGION( 0x100000, "k053260", 0 )    /* samples for the 053260 */
	ROM_LOAD( "061e04.1d",    0x0000, 0x100000, CRC(7921d8df) SHA1(19ca4850ec489cca245e90a41bfc22493cd52263) )
ROM_END

ROM_START( glfgreatj )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 2*128k for 68000 code */
	ROM_LOAD16_BYTE( "061j02.1h",   0x000000, 0x20000, CRC(7f0d95f4) SHA1(20b66cb07ca350dcc11d781511d04988bcff9019) )
	ROM_LOAD16_BYTE( "061j03.4h",   0x000001, 0x20000, CRC(06caa38b) SHA1(95a08133f6b025db5f50f528aad480af579ebe3d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "061f01.4e",    0x0000, 0x8000, CRC(ab9a2a57) SHA1(c92738b4d3754c2378cd1e6ae786faa0c5a65808) )

	// the tile and sprite ROMs are actually 16-bit ROMs on a 32-bit bus, but the data lines are
	// swapped so that D0-D7 and D16-D23 come from one ROM and D8-D15 and D24-D31 from the other
	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD16_BYTE( "061d14.12l", 0x000000, 0x080000, CRC(b9440924) SHA1(d12763f1c999cfa4f2d6f685a73c8c20204f9cbb) )
	ROM_LOAD16_BYTE( "061d13.12k", 0x000001, 0x080000, CRC(9f999f0b) SHA1(f83e3e9e44d7d5ba4c72f72db1ab9f98a0e80fe2) )

	ROM_REGION( 0x200000, "k053245", 0 )    /* sprites */
	ROM_LOAD16_BYTE( "061d11.3k", 0x000000, 0x100000, CRC(c45b66a3) SHA1(bfb7f9a39d195857893d3f04c28d5c89442c3ac7) )
	ROM_LOAD16_BYTE( "061d12.8k", 0x000001, 0x100000, CRC(d305ecd1) SHA1(28cba6b5eb56b6f5c01e9da341a5c0e2ed3cb407) )

	ROM_REGION( 0x180000, "zoom", 0 )   /* 053936 tiles */
	ROM_LOAD( "061b08.14g",   0x000000, 0x080000, CRC(6ab739c3) SHA1(37ed6c9b224189c183895517d6a72738fe92ecc4) )
	ROM_LOAD( "061b09.15g",   0x080000, 0x080000, CRC(42c7a603) SHA1(e98e484ca817ed65c7fb80a87d732e70d120676f) )
	ROM_LOAD( "061b10.17g",   0x100000, 0x080000, CRC(10f89ce7) SHA1(cf6a16ed0174db640780da4d11076efeb48a6119) )

	ROM_REGION( 0x120000, "user1", 0 )  /* 053936 tilemaps */
	ROM_LOAD( "061b07.18d",   0x000000, 0x080000, CRC(517887e2) SHA1(ff7aa0df2cda3c745a195879c71727352696ef3a) )
	ROM_LOAD( "061b06.16d",   0x080000, 0x080000, CRC(41ada2ad) SHA1(7b200e44e040e3d79f2603a02c9991b4655407d4) )
	ROM_LOAD( "061b05.15d",   0x100000, 0x020000, CRC(2456fb11) SHA1(e1bdb9f5983751d28addad6977a44df3d9899a14) )

	ROM_REGION( 0x100000, "k053260", 0 )    /* samples for the 053260 */
	ROM_LOAD( "061e04.1d",    0x0000, 0x100000, CRC(7921d8df) SHA1(19ca4850ec489cca245e90a41bfc22493cd52263) )
ROM_END

ROM_START( tmnt2 )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 4*128k for 68000 code */
	ROM_LOAD16_BYTE( "063uaa02.8e",  0x000000, 0x20000, CRC(58d5c93d) SHA1(6618678ec2da33d2ee6335cca7c9d49e9148b799) )
	ROM_LOAD16_BYTE( "063uaa03.8g",  0x000001, 0x20000, CRC(0541fec9) SHA1(985364616a95e7dd008b5be02c0f0bf5eef54b3d) )
	ROM_LOAD16_BYTE( "063uaa04.10e", 0x040000, 0x20000, CRC(1d441a7d) SHA1(97ce51eaf1c7560c19d8453f93ce01b0f71fe36d) )
	ROM_LOAD16_BYTE( "063uaa05.10g", 0x040001, 0x20000, CRC(9c428273) SHA1(92202b6061313e464c2d9760926852b833994d28) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "063b01.2f", 0x0000, 0x10000, CRC(364f548a) SHA1(e0636e27d4fc48b2ccb1417b63d2b68d9e272c06) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "063b12.16k", 0x000000, 0x080000, CRC(d3283d19) SHA1(49e4daa9cbe4d99bf71fcee6237cb434a0d55312) )
	ROM_LOAD32_WORD( "063b11.12k", 0x000002, 0x080000, CRC(6ebc0c15) SHA1(e6848405076937fbf8ec6d318293a0ff922725f4) )

	ROM_REGION( 0x400000, "k053245", 0 )    /* sprites */
	ROM_LOAD32_WORD( "063b09.7l", 0x000000, 0x100000, CRC(2d7a9d2a) SHA1(a26f9c1a07152bc8c7bcd797d4485bf848f5e2a0) )
	ROM_LOAD32_WORD( "063b07.3l", 0x000002, 0x100000, CRC(d9bee7bf) SHA1(7bbb65138fbd216b80412783e6f0072742101440) )
	ROM_LOAD32_WORD( "063b10.7k", 0x200000, 0x080000, CRC(f2dd296e) SHA1(a2aad10bfb0904dd73c2ee11049648c94de7f4d5) )
	ROM_LOAD32_WORD( "063b08.3k", 0x200002, 0x080000, CRC(3b1ae36f) SHA1(9e69cae8b517497ac77c4d148f56f2bb6a23de89) )
	/* second half empty */

	ROM_REGION( 0x200000, "k053260", 0 )    /* samples for the 053260 */
	ROM_LOAD( "063b06.1d",  0x0000, 0x200000, CRC(1e510aa5) SHA1(02b9bd6bb6b098026a620e4d671c40a31ad9e318) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "tmnt2_uaa.nv", 0x0000, 0x080, CRC(284357bd) SHA1(4f1c78b7cc86379678b1b84bcf59f8599a8c3686) )
ROM_END

ROM_START( tmnt22pu )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 4*128k for 68000 code */
	ROM_LOAD16_BYTE( "063uda02.8e",  0x000000, 0x20000, CRC(aadffe3a) SHA1(f20eaef64f81b91726675006aa45807b0841f046) )
	ROM_LOAD16_BYTE( "063uda03.8g",  0x000001, 0x20000, CRC(125687a8) SHA1(ab8eb954a56cbb18a26af3431aa8d60406ef23b5) )
	ROM_LOAD16_BYTE( "063uda04.10e", 0x040000, 0x20000, CRC(fb5c7ded) SHA1(322ec2a4a6a2ecea0865bc72b6c1d23e52da33da) )
	ROM_LOAD16_BYTE( "063uda05.10g", 0x040001, 0x20000, CRC(3c40fe66) SHA1(d2d1f24bf8ab44d24478f021f0b651095f623860) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "063b01.2f", 0x0000, 0x10000, CRC(364f548a) SHA1(e0636e27d4fc48b2ccb1417b63d2b68d9e272c06) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "063b12.16k", 0x000000, 0x080000, CRC(d3283d19) SHA1(49e4daa9cbe4d99bf71fcee6237cb434a0d55312) )
	ROM_LOAD32_WORD( "063b11.12k", 0x000002, 0x080000, CRC(6ebc0c15) SHA1(e6848405076937fbf8ec6d318293a0ff922725f4) )

	ROM_REGION( 0x400000, "k053245", 0 )    /* sprites */
	ROM_LOAD32_WORD( "063b09.7l", 0x000000, 0x100000, CRC(2d7a9d2a) SHA1(a26f9c1a07152bc8c7bcd797d4485bf848f5e2a0) )
	ROM_LOAD32_WORD( "063b07.3l", 0x000002, 0x100000, CRC(d9bee7bf) SHA1(7bbb65138fbd216b80412783e6f0072742101440) )
	ROM_LOAD32_WORD( "063b10.7k", 0x200000, 0x080000, CRC(f2dd296e) SHA1(a2aad10bfb0904dd73c2ee11049648c94de7f4d5) )
	ROM_LOAD32_WORD( "063b08.3k", 0x200002, 0x080000, CRC(3b1ae36f) SHA1(9e69cae8b517497ac77c4d148f56f2bb6a23de89) )
	/* second half empty */

	ROM_REGION( 0x200000, "k053260", 0 )    /* samples for the 053260 */
	ROM_LOAD( "063b06.1d",  0x0000, 0x200000, CRC(1e510aa5) SHA1(02b9bd6bb6b098026a620e4d671c40a31ad9e318) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting with invisible error message
	ROM_LOAD( "tmnt2_uda.nv", 0x0000, 0x080, CRC(44928d33) SHA1(44024927987f6bb8bdac3dbd1fdc81d7b55c0f5a) )
ROM_END

ROM_START( tmnt24pu )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 4*128k for 68000 code */
	ROM_LOAD16_BYTE( "063uea02.8e",  0x000000, 0x20000, CRC(5eebc67f) SHA1(dbfbe0bdd40cfb01824d59e9785759ffdfdcba55) )
	ROM_LOAD16_BYTE( "063uea03.8g",  0x000001, 0x20000, CRC(1b956869) SHA1(3cea77c8b6ca93899a044c28a45f5966937b7df7) )
	ROM_LOAD16_BYTE( "063uea04.10e", 0x040000, 0x20000, CRC(e13d93a6) SHA1(7297a4c19d1e338fa41a3983bb9179a6c3cd6ea4) )
	ROM_LOAD16_BYTE( "063uea05.10g", 0x040001, 0x20000, CRC(a3a1f5ea) SHA1(6d869a8ba457c40928ca0bcf3e5b7a436faa185c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "063b01.2f", 0x0000, 0x10000, CRC(364f548a) SHA1(e0636e27d4fc48b2ccb1417b63d2b68d9e272c06) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "063b12.16k", 0x000000, 0x080000, CRC(d3283d19) SHA1(49e4daa9cbe4d99bf71fcee6237cb434a0d55312) )
	ROM_LOAD32_WORD( "063b11.12k", 0x000002, 0x080000, CRC(6ebc0c15) SHA1(e6848405076937fbf8ec6d318293a0ff922725f4) )

	ROM_REGION( 0x400000, "k053245", 0 )    /* sprites */
	ROM_LOAD32_WORD( "063b09.7l", 0x000000, 0x100000, CRC(2d7a9d2a) SHA1(a26f9c1a07152bc8c7bcd797d4485bf848f5e2a0) )
	ROM_LOAD32_WORD( "063b07.3l", 0x000002, 0x100000, CRC(d9bee7bf) SHA1(7bbb65138fbd216b80412783e6f0072742101440) )
	ROM_LOAD32_WORD( "063b10.7k", 0x200000, 0x080000, CRC(f2dd296e) SHA1(a2aad10bfb0904dd73c2ee11049648c94de7f4d5) )
	ROM_LOAD32_WORD( "063b08.3k", 0x200002, 0x080000, CRC(3b1ae36f) SHA1(9e69cae8b517497ac77c4d148f56f2bb6a23de89) )
	/* second half empty */

	ROM_REGION( 0x200000, "k053260", 0 )    /* samples for the 053260 */
	ROM_LOAD( "063b06.1d",  0x0000, 0x200000, CRC(1e510aa5) SHA1(02b9bd6bb6b098026a620e4d671c40a31ad9e318) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting with invisible error message
	ROM_LOAD( "tmnt2_uea.nv", 0x0000, 0x080, CRC(4f086faa) SHA1(de5395737173a6b12ab0cab33f42c44151ceff89) )
ROM_END

ROM_START( tmht22pe )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 4*128k for 68000 code */
	ROM_LOAD16_BYTE( "063eba02.8e",  0x000000, 0x20000, CRC(99409094) SHA1(18059da85c59eb6ce193111bb8c7bd6601b1e698) )
	ROM_LOAD16_BYTE( "063eba03.8g",  0x000001, 0x20000, CRC(4d65650f) SHA1(95c03b215f1db8377b2f6b4686055fcf0117f878) )
	ROM_LOAD16_BYTE( "063eba04.10e", 0x040000, 0x20000, CRC(f6e3b9c7) SHA1(1ad5cb30ad3ef6e2cd954c3e2f1b6775cbf7a676) )
	ROM_LOAD16_BYTE( "063eba05.10g", 0x040001, 0x20000, CRC(1bad6696) SHA1(8da436bce4cafd9e09e5272f0c1c37395c26ac02) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "063b01.2f", 0x0000, 0x10000, CRC(364f548a) SHA1(e0636e27d4fc48b2ccb1417b63d2b68d9e272c06) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "063b12.16k", 0x000000, 0x080000, CRC(d3283d19) SHA1(49e4daa9cbe4d99bf71fcee6237cb434a0d55312) )
	ROM_LOAD32_WORD( "063b11.12k", 0x000002, 0x080000, CRC(6ebc0c15) SHA1(e6848405076937fbf8ec6d318293a0ff922725f4) )

	ROM_REGION( 0x400000, "k053245", 0 )    /* sprites */
	ROM_LOAD32_WORD( "063b09.7l", 0x000000, 0x100000, CRC(2d7a9d2a) SHA1(a26f9c1a07152bc8c7bcd797d4485bf848f5e2a0) )
	ROM_LOAD32_WORD( "063b07.3l", 0x000002, 0x100000, CRC(d9bee7bf) SHA1(7bbb65138fbd216b80412783e6f0072742101440) )
	ROM_LOAD32_WORD( "063b10.7k", 0x200000, 0x080000, CRC(f2dd296e) SHA1(a2aad10bfb0904dd73c2ee11049648c94de7f4d5) )
	ROM_LOAD32_WORD( "063b08.3k", 0x200002, 0x080000, CRC(3b1ae36f) SHA1(9e69cae8b517497ac77c4d148f56f2bb6a23de89) )
	/* second half empty */

	ROM_REGION( 0x200000, "k053260", 0 )    /* samples for the 053260 */
	ROM_LOAD( "063b06.1d",  0x0000, 0x200000, CRC(1e510aa5) SHA1(02b9bd6bb6b098026a620e4d671c40a31ad9e318) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting with invisible error message
	ROM_LOAD( "tmnt2_eba.nv", 0x0000, 0x080, CRC(c0a3ed50) SHA1(6deec720c7f1c607740076cb8b5b5becd175aed0) )
ROM_END

ROM_START( tmht24pe )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 4*128k for 68000 code */
	ROM_LOAD16_BYTE( "063eaa02.8e",  0x000000, 0x20000, CRC(665a68de) SHA1(3cefc2cb0f0a42f1044ef04862669fc7893005da) )
	ROM_LOAD16_BYTE( "063eaa03.8g",  0x000001, 0x20000, CRC(7b7fb3af) SHA1(1c96283af8fc81c30136dfe2efbd113cb7dd3d66) )
	ROM_LOAD16_BYTE( "063eaa04.10e", 0x040000, 0x20000, CRC(69f38e1d) SHA1(a63aa86a11f803fa3f07c5eb2fdbdb75bb850d55) )
	ROM_LOAD16_BYTE( "063eaa05.10g", 0x040001, 0x20000, CRC(818032af) SHA1(60d416a58696add58493c0f2297b3a4af5f46d6d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "063b01.2f", 0x0000, 0x10000, CRC(364f548a) SHA1(e0636e27d4fc48b2ccb1417b63d2b68d9e272c06) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "063b12.16k", 0x000000, 0x080000, CRC(d3283d19) SHA1(49e4daa9cbe4d99bf71fcee6237cb434a0d55312) )
	ROM_LOAD32_WORD( "063b11.12k", 0x000002, 0x080000, CRC(6ebc0c15) SHA1(e6848405076937fbf8ec6d318293a0ff922725f4) )

	ROM_REGION( 0x400000, "k053245", 0 )    /* sprites */
	ROM_LOAD32_WORD( "063b09.7l", 0x000000, 0x100000, CRC(2d7a9d2a) SHA1(a26f9c1a07152bc8c7bcd797d4485bf848f5e2a0) )
	ROM_LOAD32_WORD( "063b07.3l", 0x000002, 0x100000, CRC(d9bee7bf) SHA1(7bbb65138fbd216b80412783e6f0072742101440) )
	ROM_LOAD32_WORD( "063b10.7k", 0x200000, 0x080000, CRC(f2dd296e) SHA1(a2aad10bfb0904dd73c2ee11049648c94de7f4d5) )
	ROM_LOAD32_WORD( "063b08.3k", 0x200002, 0x080000, CRC(3b1ae36f) SHA1(9e69cae8b517497ac77c4d148f56f2bb6a23de89) )
	/* second half empty */

	ROM_REGION( 0x200000, "k053260", 0 )    /* samples for the 053260 */
	ROM_LOAD( "063b06.1d",  0x0000, 0x200000, CRC(1e510aa5) SHA1(02b9bd6bb6b098026a620e4d671c40a31ad9e318) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting with invisible error message
	ROM_LOAD( "tmnt2_eaa.nv", 0x0000, 0x080, CRC(124af18f) SHA1(bbd0629663135fc6c08b000b886ea76a96592a9e) )
ROM_END

ROM_START( tmnt2a )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 4*128k for 68000 code */
	ROM_LOAD16_BYTE( "063ada02.8e",  0x000000, 0x20000, CRC(4f11b587) SHA1(111051da23ce7035405b4d12c0f18dcc1d6c8ddc) )
	ROM_LOAD16_BYTE( "063ada03.8g",  0x000001, 0x20000, CRC(82a1b9ac) SHA1(161e8fd33e0e5c9349fec98b02225ed37578e488) )
	ROM_LOAD16_BYTE( "063ada04.10e", 0x040000, 0x20000, CRC(05ad187a) SHA1(27a36a02ef792d87ffa2364537c42b6c50d6e4f0) )
	ROM_LOAD16_BYTE( "063ada05.10g", 0x040001, 0x20000, CRC(d4826547) SHA1(ffee07be64469fa386a0979352b4fe20c352fee4) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "063b01.2f", 0x0000, 0x10000, CRC(364f548a) SHA1(e0636e27d4fc48b2ccb1417b63d2b68d9e272c06) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "063b12.16k", 0x000000, 0x080000, CRC(d3283d19) SHA1(49e4daa9cbe4d99bf71fcee6237cb434a0d55312) )
	ROM_LOAD32_WORD( "063b11.12k", 0x000002, 0x080000, CRC(6ebc0c15) SHA1(e6848405076937fbf8ec6d318293a0ff922725f4) )

	ROM_REGION( 0x400000, "k053245", 0 )    /* sprites */
	ROM_LOAD32_WORD( "063b09.7l", 0x000000, 0x100000, CRC(2d7a9d2a) SHA1(a26f9c1a07152bc8c7bcd797d4485bf848f5e2a0) )
	ROM_LOAD32_WORD( "063b07.3l", 0x000002, 0x100000, CRC(d9bee7bf) SHA1(7bbb65138fbd216b80412783e6f0072742101440) )
	ROM_LOAD32_WORD( "063b10.7k", 0x200000, 0x080000, CRC(f2dd296e) SHA1(a2aad10bfb0904dd73c2ee11049648c94de7f4d5) )
	ROM_LOAD32_WORD( "063b08.3k", 0x200002, 0x080000, CRC(3b1ae36f) SHA1(9e69cae8b517497ac77c4d148f56f2bb6a23de89) )
	/* second half empty */

	ROM_REGION( 0x200000, "k053260", 0 )    /* samples for the 053260 */
	ROM_LOAD( "063b06.1d",  0x0000, 0x200000, CRC(1e510aa5) SHA1(02b9bd6bb6b098026a620e4d671c40a31ad9e318) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "tmnt2_ada.nv", 0x0000, 0x080, CRC(063068a0) SHA1(c1da5319428fd8fb60305a2d7cc166596b2fe5a4) )
ROM_END

ROM_START( tmnt2o )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 4*128k for 68000 code */
	ROM_LOAD16_BYTE( "063oaa02.8e",  0x000000, 0x20000, CRC(07067dbf) SHA1(3ffd6f09b24dfd9a403097cbba19ae7b51110dc5) )
	ROM_LOAD16_BYTE( "063oaa03.8g",  0x000001, 0x20000, CRC(b0fadf6b) SHA1(8dde410d323dd6dd5b08b32b5363875782b45191) )
	ROM_LOAD16_BYTE( "063oaa04.10e", 0x040000, 0x20000, CRC(b5eb7c49) SHA1(72193d8a334f22cf7a617e42eaebce45b8eb83f2) )
	ROM_LOAD16_BYTE( "063oaa05.10g", 0x040001, 0x20000, CRC(bd4dcca4) SHA1(3b47c086fccefe172746c00af88e1f98914c6567) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "063b01.2f", 0x0000, 0x10000, CRC(364f548a) SHA1(e0636e27d4fc48b2ccb1417b63d2b68d9e272c06) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "063b12.16k", 0x000000, 0x080000, CRC(d3283d19) SHA1(49e4daa9cbe4d99bf71fcee6237cb434a0d55312) )
	ROM_LOAD32_WORD( "063b11.12k", 0x000002, 0x080000, CRC(6ebc0c15) SHA1(e6848405076937fbf8ec6d318293a0ff922725f4) )

	ROM_REGION( 0x400000, "k053245", 0 )    /* sprites */
	ROM_LOAD32_WORD( "063b09.7l", 0x000000, 0x100000, CRC(2d7a9d2a) SHA1(a26f9c1a07152bc8c7bcd797d4485bf848f5e2a0) )
	ROM_LOAD32_WORD( "063b07.3l", 0x000002, 0x100000, CRC(d9bee7bf) SHA1(7bbb65138fbd216b80412783e6f0072742101440) )
	ROM_LOAD32_WORD( "063b10.7k", 0x200000, 0x080000, CRC(f2dd296e) SHA1(a2aad10bfb0904dd73c2ee11049648c94de7f4d5) )
	ROM_LOAD32_WORD( "063b08.3k", 0x200002, 0x080000, CRC(3b1ae36f) SHA1(9e69cae8b517497ac77c4d148f56f2bb6a23de89) )
	/* second half empty */

	ROM_REGION( 0x200000, "k053260", 0 )    /* samples for the 053260 */
	ROM_LOAD( "063b06.1d",  0x0000, 0x200000, CRC(1e510aa5) SHA1(02b9bd6bb6b098026a620e4d671c40a31ad9e318) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "tmnt2_oaa.nv", 0x0000, 0x080, CRC(e3858340) SHA1(6bac93b5485d68dfd2e15615fa7e95d631e9bf67) )
ROM_END

ROM_START( qgakumon )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 4*256k for 68000 code */
	ROM_LOAD16_BYTE( "248jaa02.8e",  0x000000, 0x40000, CRC(fab79410) SHA1(8b1a8946ee65505608cf026c9fca87365ccef089) )
	ROM_LOAD16_BYTE( "248jaa03.8g",  0x000001, 0x40000, CRC(8d888ef3) SHA1(1ef2636620abff8e3fe0258c90c5c8c0bf33f2d5) )
	ROM_LOAD16_BYTE( "248jaa04.10e", 0x080000, 0x40000, CRC(56cb16cb) SHA1(a659229b43fba59c055e1da061fbfb19ecbb5c24) )
	ROM_LOAD16_BYTE( "248jaa05.10g", 0x080001, 0x40000, CRC(27614fcd) SHA1(c44d1dd3f16914f9616d6370098eaf6fa8a44542) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "248a01.2f",          0x0000, 0x10000, CRC(a8a41cc6) SHA1(ad0d73bbdaacb8d5d0c7971ec4357eec665ee7cf) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "248a12.16k",       0x000000, 0x080000, CRC(62870987) SHA1(f502c44781a077590038dcca9bf76c8a047169be) )
	ROM_LOAD32_WORD( "248a11.12k",       0x000002, 0x080000, CRC(fad2dbfd) SHA1(a6cc9a612467c43ae4194f71b43a442272f0fde1) )

	ROM_REGION( 0x400000, "k053245", 0 )    /* sprites */
	ROM_LOAD32_WORD( "248a09.7l", 0x000000, 0x100000, CRC(a176e205) SHA1(e0b2176a1525711c6e692f88a913f57b9bdd0046) )
	ROM_LOAD32_WORD( "248a07.3l", 0x000002, 0x100000, CRC(9595589f) SHA1(3e48f66448577a8fa39b6707e89c2267152b6f0b) )

	ROM_REGION( 0x200000, "k053260", 0 )    /* samples for the 053260 */
	ROM_LOAD( "248a06.1d",       0x0000, 0x200000, CRC(0fba1def) SHA1(f2ba23213effd06f14c7a179acea974c78c2198f) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting with error
	ROM_LOAD( "qgakumon.nv", 0x0000, 0x080, CRC(847bd238) SHA1(d6f7bf215055b7e9ea1027f4b4e34fea25b3273b) )
ROM_END

ROM_START( ssriders )
	ROM_REGION( 0xc0000, "maincpu", 0 ) /* 2*256k and 2*128k for 68000 code */
	ROM_LOAD16_BYTE( "064eac02.8e",  0x000000, 0x40000, CRC(5a5425f4) SHA1(213226558d772f3ae573ee851b881536ce2faa2a) )
	ROM_LOAD16_BYTE( "064eac03.8g",  0x000001, 0x40000, CRC(093c00fb) SHA1(208a3688504bad3bc23135ceb0f15226dd98558e) )
	ROM_LOAD16_BYTE( "064eab04.10e", 0x080000, 0x20000, CRC(ef2315bd) SHA1(2c8b11321cb5fdb78d760fabca666c0d8cc5b298) )
	ROM_LOAD16_BYTE( "064eab05.10g", 0x080001, 0x20000, CRC(51d6fbc4) SHA1(e80de7d155b7f263c48ef4ae2702059be3c18e76) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "064e01.2f",   0x0000, 0x10000, CRC(44b9bc52) SHA1(4654d6e14c6956c40a19cb41155accb63f0da338) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "064e12.16k",   0x000000, 0x080000, CRC(e2bdc619) SHA1(04449deb267b0beacfa33640b593eb16194aa0d9) )
	ROM_LOAD32_WORD( "064e11.12k",   0x000002, 0x080000, CRC(2d8ca8b0) SHA1(7c882f79c2402cf75979c681071007d76e4db9ae) )

	ROM_REGION( 0x200000, "k053245", 0 )    /* sprites */
	ROM_LOAD32_WORD( "064e09.7l", 0x000000, 0x100000, CRC(4160c372) SHA1(0b36181e5ccd785c7fb89b9f41e458066a42c3b0) )
	ROM_LOAD32_WORD( "064e07.3l", 0x000002, 0x100000, CRC(64dd673c) SHA1(bea4d17a71dd21c635866ee69b4892dc9d0ab455) )

	ROM_REGION( 0x100000, "k053260", 0 )    /* samples for the 053260 */
	ROM_LOAD( "064e06.1d",    0x0000, 0x100000, CRC(59810df9) SHA1(a0affc6330bdbfab1447dc0cf13c20ff708c2c71) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "ssriders_eac.nv", 0x0000, 0x080, CRC(f6d641a7) SHA1(6c680d49e1b07a47b29ff263a6009d226bd120cf) )
ROM_END

ROM_START( ssridersebd )
	ROM_REGION( 0xc0000, "maincpu", 0 ) /* 2*256k and 2*128k for 68000 code */
	ROM_LOAD16_BYTE( "064ebd02.8e",  0x000000, 0x40000, CRC(8deef9ac) SHA1(406ef2b022a59ed958674f432ed50f6ed37fd3c4) )
	ROM_LOAD16_BYTE( "064ebd03.8g",  0x000001, 0x40000, CRC(2370c107) SHA1(85d2bd8dde928f647a5d34ac98d2df2ed559f7a2) )
	ROM_LOAD16_BYTE( "064eab04.10e", 0x080000, 0x20000, CRC(ef2315bd) SHA1(2c8b11321cb5fdb78d760fabca666c0d8cc5b298) )
	ROM_LOAD16_BYTE( "064eab05.10g", 0x080001, 0x20000, CRC(51d6fbc4) SHA1(e80de7d155b7f263c48ef4ae2702059be3c18e76) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "064e01.2f",   0x0000, 0x10000, CRC(44b9bc52) SHA1(4654d6e14c6956c40a19cb41155accb63f0da338) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "064e12.16k",   0x000000, 0x080000, CRC(e2bdc619) SHA1(04449deb267b0beacfa33640b593eb16194aa0d9) )
	ROM_LOAD32_WORD( "064e11.12k",   0x000002, 0x080000, CRC(2d8ca8b0) SHA1(7c882f79c2402cf75979c681071007d76e4db9ae) )

	ROM_REGION( 0x200000, "k053245", 0 )    /* sprites */
	ROM_LOAD32_WORD( "064e09.7l", 0x000000, 0x100000, CRC(4160c372) SHA1(0b36181e5ccd785c7fb89b9f41e458066a42c3b0) )
	ROM_LOAD32_WORD( "064e07.3l", 0x000002, 0x100000, CRC(64dd673c) SHA1(bea4d17a71dd21c635866ee69b4892dc9d0ab455) )

	ROM_REGION( 0x100000, "k053260", 0 )    /* samples for the 053260 */
	ROM_LOAD( "064e06.1d",    0x0000, 0x100000, CRC(59810df9) SHA1(a0affc6330bdbfab1447dc0cf13c20ff708c2c71) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "ssriders_ebd.nv", 0x0000, 0x080, CRC(cbc903f6) SHA1(43cb8e7f9b1da05da93878bd236f63036e32e379) )
ROM_END

ROM_START( ssridersebc )
	ROM_REGION( 0xc0000, "maincpu", 0 ) /* 2*256k and 2*128k for 68000 code */
	ROM_LOAD16_BYTE( "064ebc02.8e",  0x000000, 0x40000, CRC(9bd7d164) SHA1(492abdaf62fe7cb72b7e53076a05c987503c738a) )
	ROM_LOAD16_BYTE( "064ebc03.8g",  0x000001, 0x40000, CRC(40fd4165) SHA1(c30d7560aae6e9f0bebe2d6d3e0e11b56634de0c) )
	ROM_LOAD16_BYTE( "064eab04.10e", 0x080000, 0x20000, CRC(ef2315bd) SHA1(2c8b11321cb5fdb78d760fabca666c0d8cc5b298) )
	ROM_LOAD16_BYTE( "064eab05.10g", 0x080001, 0x20000, CRC(51d6fbc4) SHA1(e80de7d155b7f263c48ef4ae2702059be3c18e76) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "064e01.2f",   0x0000, 0x10000, CRC(44b9bc52) SHA1(4654d6e14c6956c40a19cb41155accb63f0da338) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "064e12.16k",   0x000000, 0x080000, CRC(e2bdc619) SHA1(04449deb267b0beacfa33640b593eb16194aa0d9) )
	ROM_LOAD32_WORD( "064e11.12k",   0x000002, 0x080000, CRC(2d8ca8b0) SHA1(7c882f79c2402cf75979c681071007d76e4db9ae) )

	ROM_REGION( 0x200000, "k053245", 0 )    /* sprites */
	ROM_LOAD32_WORD( "064e09.7l", 0x000000, 0x100000, CRC(4160c372) SHA1(0b36181e5ccd785c7fb89b9f41e458066a42c3b0) )
	ROM_LOAD32_WORD( "064e07.3l", 0x000002, 0x100000, CRC(64dd673c) SHA1(bea4d17a71dd21c635866ee69b4892dc9d0ab455) )

	ROM_REGION( 0x100000, "k053260", 0 )    /* samples for the 053260 */
	ROM_LOAD( "064e06.1d",    0x0000, 0x100000, CRC(59810df9) SHA1(a0affc6330bdbfab1447dc0cf13c20ff708c2c71) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "ssriders_ebc.nv", 0x0000, 0x080, CRC(1918e90f) SHA1(edddbe80a5c9dd64411596b1aa4d7fce7b7090ae) )
ROM_END

ROM_START( ssriderseaa )
	ROM_REGION( 0xc0000, "maincpu", 0 ) /* 2*256k and 2*128k for 68000 code */
	ROM_LOAD16_BYTE( "064eaa02.8e",   0x000000, 0x40000, CRC(4844660f) SHA1(d2ef7a1b20f09cb63564e62dfe09bfed098a0faa) )
	ROM_LOAD16_BYTE( "064eaa03.8g",   0x000001, 0x40000, CRC(0b9bcc7c) SHA1(d291da7f1eaa79ab1dfa402b862ba69061c83bdb) )
	ROM_LOAD16_BYTE( "064eaa04.10e",  0x080000, 0x20000, CRC(5d917c1c) SHA1(3a8b410b27bf5e37f9263945abf85ac69f217350) )
	ROM_LOAD16_BYTE( "064eaa05.10g",  0x080001, 0x20000, CRC(f4647b74) SHA1(653ecbf1f3fc8d304e1c7683b2a1a20bed0aefe0) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "064eaa01.2f",   0x0000, 0x10000, CRC(bce45d82) SHA1(7f6d17fad0b556243c59d25a94925d259d98d81a) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "064e12.16k",   0x000000, 0x080000, CRC(e2bdc619) SHA1(04449deb267b0beacfa33640b593eb16194aa0d9) )
	ROM_LOAD32_WORD( "064e11.12k",   0x000002, 0x080000, CRC(2d8ca8b0) SHA1(7c882f79c2402cf75979c681071007d76e4db9ae) )

	ROM_REGION( 0x200000, "k053245", 0 )    /* sprites */
	ROM_LOAD32_WORD( "064e09.7l", 0x000000, 0x100000, CRC(4160c372) SHA1(0b36181e5ccd785c7fb89b9f41e458066a42c3b0) )
	ROM_LOAD32_WORD( "064e07.3l", 0x000002, 0x100000, CRC(64dd673c) SHA1(bea4d17a71dd21c635866ee69b4892dc9d0ab455) )

	ROM_REGION( 0x100000, "k053260", 0 )    /* samples for the 053260 */
	ROM_LOAD( "064e06.1d",    0x0000, 0x100000, CRC(59810df9) SHA1(a0affc6330bdbfab1447dc0cf13c20ff708c2c71) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "ssriders_eaa.nv", 0x0000, 0x080, CRC(74a45ef5) SHA1(da01f14684315bfb5c180f0c64a14350c34ae945) )
ROM_END

ROM_START( ssridersuda )
	ROM_REGION( 0xc0000, "maincpu", 0 ) /* 2*256k and 2*128k for 68000 code */
	ROM_LOAD16_BYTE( "064uda02.8e",  0x000000, 0x40000, CRC(5129a6b7) SHA1(8892d7043e7b0aee9eaffde9fa9bfd9bbfb7f15f) )
	ROM_LOAD16_BYTE( "064uda03.8g",  0x000001, 0x40000, CRC(9f887214) SHA1(f5e22230b7dca42242f3f244e45e67a4bbbdb65f) )
	ROM_LOAD16_BYTE( "064eab04.10e", 0x080000, 0x20000, CRC(ef2315bd) SHA1(2c8b11321cb5fdb78d760fabca666c0d8cc5b298) )
	ROM_LOAD16_BYTE( "064eab05.10g", 0x080001, 0x20000, CRC(51d6fbc4) SHA1(e80de7d155b7f263c48ef4ae2702059be3c18e76) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "064e01.2f",   0x0000, 0x10000, CRC(44b9bc52) SHA1(4654d6e14c6956c40a19cb41155accb63f0da338) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "064e12.16k",   0x000000, 0x080000, CRC(e2bdc619) SHA1(04449deb267b0beacfa33640b593eb16194aa0d9) )
	ROM_LOAD32_WORD( "064e11.12k",   0x000002, 0x080000, CRC(2d8ca8b0) SHA1(7c882f79c2402cf75979c681071007d76e4db9ae) )

	ROM_REGION( 0x200000, "k053245", 0 )    /* sprites */
	ROM_LOAD32_WORD( "064e09.7l", 0x000000, 0x100000, CRC(4160c372) SHA1(0b36181e5ccd785c7fb89b9f41e458066a42c3b0) )
	ROM_LOAD32_WORD( "064e07.3l", 0x000002, 0x100000, CRC(64dd673c) SHA1(bea4d17a71dd21c635866ee69b4892dc9d0ab455) )

	ROM_REGION( 0x100000, "k053260", 0 )    /* samples for the 053260 */
	ROM_LOAD( "064e06.1d",    0x0000, 0x100000, CRC(59810df9) SHA1(a0affc6330bdbfab1447dc0cf13c20ff708c2c71) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "ssriders_uda.nv", 0x0000, 0x080, CRC(148c6d64) SHA1(38016ba7e29f821a4c0de67a7ddc686d307e4659) )
ROM_END

ROM_START( ssridersuab )
	ROM_REGION( 0xc0000, "maincpu", 0 ) /* 2*256k and 2*128k for 68000 code */
	ROM_LOAD16_BYTE( "064uab02.8e",  0x000000, 0x40000, CRC(f1a3c548) SHA1(8977d919f1d0fc3ab6801dd89a81a79e86abca69) )
	ROM_LOAD16_BYTE( "064uab03.8g",  0x000001, 0x40000, CRC(66a61287) SHA1(a3defe361a7528ef8c88743355fb4983e3523564) )
	ROM_LOAD16_BYTE( "064eab04.10e", 0x080000, 0x20000, CRC(ef2315bd) SHA1(2c8b11321cb5fdb78d760fabca666c0d8cc5b298) )
	ROM_LOAD16_BYTE( "064eab05.10g", 0x080001, 0x20000, CRC(51d6fbc4) SHA1(e80de7d155b7f263c48ef4ae2702059be3c18e76) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "064e01.2f",   0x0000, 0x10000, CRC(44b9bc52) SHA1(4654d6e14c6956c40a19cb41155accb63f0da338) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "064e12.16k",   0x000000, 0x080000, CRC(e2bdc619) SHA1(04449deb267b0beacfa33640b593eb16194aa0d9) )
	ROM_LOAD32_WORD( "064e11.12k",   0x000002, 0x080000, CRC(2d8ca8b0) SHA1(7c882f79c2402cf75979c681071007d76e4db9ae) )

	ROM_REGION( 0x200000, "k053245", 0 )    /* sprites */
	ROM_LOAD32_WORD( "064e09.7l", 0x000000, 0x100000, CRC(4160c372) SHA1(0b36181e5ccd785c7fb89b9f41e458066a42c3b0) )
	ROM_LOAD32_WORD( "064e07.3l", 0x000002, 0x100000, CRC(64dd673c) SHA1(bea4d17a71dd21c635866ee69b4892dc9d0ab455) )

	ROM_REGION( 0x100000, "k053260", 0 )    /* samples for the 053260 */
	ROM_LOAD( "064e06.1d",    0x0000, 0x100000, CRC(59810df9) SHA1(a0affc6330bdbfab1447dc0cf13c20ff708c2c71) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "ssriders_uab.nv", 0x0000, 0x080, CRC(fe08b210) SHA1(151eecde7c3200b6df660502d06e872e5c1d14b0) )
ROM_END

ROM_START( ssridersuac )
	ROM_REGION( 0xc0000, "maincpu", 0 ) /* 2*256k and 2*128k for 68000 code */
	ROM_LOAD16_BYTE( "064uac02.8e",  0x000000, 0x40000, CRC(870473b6) SHA1(2e2fd5c6df3fa8da6655699043e08a8f918ef63c) )
	ROM_LOAD16_BYTE( "064uac03.8g",  0x000001, 0x40000, CRC(eadf289a) SHA1(824230714ae0c1d065e83719bb344e76a5ca1fba) )
	ROM_LOAD16_BYTE( "064eab04.10e", 0x080000, 0x20000, CRC(ef2315bd) SHA1(2c8b11321cb5fdb78d760fabca666c0d8cc5b298) )
	ROM_LOAD16_BYTE( "064eab05.10g", 0x080001, 0x20000, CRC(51d6fbc4) SHA1(e80de7d155b7f263c48ef4ae2702059be3c18e76) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "064e01.2f",   0x0000, 0x10000, CRC(44b9bc52) SHA1(4654d6e14c6956c40a19cb41155accb63f0da338) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "064e12.16k",   0x000000, 0x080000, CRC(e2bdc619) SHA1(04449deb267b0beacfa33640b593eb16194aa0d9) )
	ROM_LOAD32_WORD( "064e11.12k",   0x000002, 0x080000, CRC(2d8ca8b0) SHA1(7c882f79c2402cf75979c681071007d76e4db9ae) )

	ROM_REGION( 0x200000, "k053245", 0 )    /* sprites */
	ROM_LOAD32_WORD( "064e09.7l", 0x000000, 0x100000, CRC(4160c372) SHA1(0b36181e5ccd785c7fb89b9f41e458066a42c3b0) )
	ROM_LOAD32_WORD( "064e07.3l", 0x000002, 0x100000, CRC(64dd673c) SHA1(bea4d17a71dd21c635866ee69b4892dc9d0ab455) )

	ROM_REGION( 0x100000, "k053260", 0 )    /* samples for the 053260 */
	ROM_LOAD( "064e06.1d",    0x0000, 0x100000, CRC(59810df9) SHA1(a0affc6330bdbfab1447dc0cf13c20ff708c2c71) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "ssriders_uac.nv", 0x0000, 0x080, CRC(26c8f1a0) SHA1(56b933d61fb3a3da787db8e6714b53178f3c98db) )
ROM_END

ROM_START( ssridersubc )
	ROM_REGION( 0xc0000, "maincpu", 0 ) /* 2*256k and 2*128k for 68000 code */
	ROM_LOAD16_BYTE( "064ubc02.8e",  0x000000, 0x40000, CRC(aca7fda5) SHA1(318fdefbea70724e95f2537b1915bc3a7abbb644) )
	ROM_LOAD16_BYTE( "064ubc03.8g",  0x000001, 0x40000, CRC(bb1fdeff) SHA1(1b74954258e3e8fdc80dd3c27785c945e57d36f8) )
	ROM_LOAD16_BYTE( "064eab04.10e", 0x080000, 0x20000, CRC(ef2315bd) SHA1(2c8b11321cb5fdb78d760fabca666c0d8cc5b298) )
	ROM_LOAD16_BYTE( "064eab05.10g", 0x080001, 0x20000, CRC(51d6fbc4) SHA1(e80de7d155b7f263c48ef4ae2702059be3c18e76) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "064e01.2f",   0x0000, 0x10000, CRC(44b9bc52) SHA1(4654d6e14c6956c40a19cb41155accb63f0da338) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "064e12.16k",   0x000000, 0x080000, CRC(e2bdc619) SHA1(04449deb267b0beacfa33640b593eb16194aa0d9) )
	ROM_LOAD32_WORD( "064e11.12k",   0x000002, 0x080000, CRC(2d8ca8b0) SHA1(7c882f79c2402cf75979c681071007d76e4db9ae) )

	ROM_REGION( 0x200000, "k053245", 0 )    /* sprites */
	ROM_LOAD32_WORD( "064e09.7l", 0x000000, 0x100000, CRC(4160c372) SHA1(0b36181e5ccd785c7fb89b9f41e458066a42c3b0) )
	ROM_LOAD32_WORD( "064e07.3l", 0x000002, 0x100000, CRC(64dd673c) SHA1(bea4d17a71dd21c635866ee69b4892dc9d0ab455) )

	ROM_REGION( 0x100000, "k053260", 0 )    /* samples for the 053260 */
	ROM_LOAD( "064e06.1d",    0x0000, 0x100000, CRC(59810df9) SHA1(a0affc6330bdbfab1447dc0cf13c20ff708c2c71) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "ssriders_ubc.nv", 0x0000, 0x080, CRC(31c5504f) SHA1(fa6ed1860122552e94abb27a6ea75f4cb8054e76) )
ROM_END

ROM_START( ssridersabd )
	ROM_REGION( 0xc0000, "maincpu", 0 ) /* 2*256k and 2*128k for 68000 code */
	ROM_LOAD16_BYTE( "064abd02.8e",  0x000000, 0x40000, CRC(713406cb) SHA1(23769413bfce6cf7039437d0fa25a7b4b9c86387) )
	ROM_LOAD16_BYTE( "064abd03.8g",  0x000001, 0x40000, CRC(680feb3c) SHA1(379082cccdbc579a88afcf771f6deb64e4baf4d6) )
	ROM_LOAD16_BYTE( "064eab04.10e", 0x080000, 0x20000, CRC(ef2315bd) SHA1(2c8b11321cb5fdb78d760fabca666c0d8cc5b298) )
	ROM_LOAD16_BYTE( "064eab05.10g", 0x080001, 0x20000, CRC(51d6fbc4) SHA1(e80de7d155b7f263c48ef4ae2702059be3c18e76) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "064e01.2f",   0x0000, 0x10000, CRC(44b9bc52) SHA1(4654d6e14c6956c40a19cb41155accb63f0da338) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "064e12.16k",   0x000000, 0x080000, CRC(e2bdc619) SHA1(04449deb267b0beacfa33640b593eb16194aa0d9) )
	ROM_LOAD32_WORD( "064e11.12k",   0x000002, 0x080000, CRC(2d8ca8b0) SHA1(7c882f79c2402cf75979c681071007d76e4db9ae) )

	ROM_REGION( 0x200000, "k053245", 0 )    /* sprites */
	ROM_LOAD32_WORD( "064e09.7l", 0x000000, 0x100000, CRC(4160c372) SHA1(0b36181e5ccd785c7fb89b9f41e458066a42c3b0) )
	ROM_LOAD32_WORD( "064e07.3l", 0x000002, 0x100000, CRC(64dd673c) SHA1(bea4d17a71dd21c635866ee69b4892dc9d0ab455) )

	ROM_REGION( 0x100000, "k053260", 0 )    /* samples for the 053260 */
	ROM_LOAD( "064e06.1d",    0x0000, 0x100000, CRC(59810df9) SHA1(a0affc6330bdbfab1447dc0cf13c20ff708c2c71) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "ssriders_abd.nv", 0x0000, 0x080, CRC(bfdafedf) SHA1(bc786d4fb3495a6baf2ae13c19392c6740a2be6d) )
ROM_END

ROM_START( ssridersadd )
	ROM_REGION( 0xc0000, "maincpu", 0 ) /* 2*256k and 2*128k for 68000 code */
	ROM_LOAD16_BYTE( "064add02.8e",  0x000000, 0x40000, CRC(06b0138e) SHA1(6d8e00a62faae1b87fafed288a920edd5456b660) )
	ROM_LOAD16_BYTE( "064add03.8g",  0x000001, 0x40000, CRC(03eb8b91) SHA1(e12f5e5eb89e59277d027f9942fdc38f67cb1066) )
	ROM_LOAD16_BYTE( "064eab04.10e", 0x080000, 0x20000, CRC(ef2315bd) SHA1(2c8b11321cb5fdb78d760fabca666c0d8cc5b298) )
	ROM_LOAD16_BYTE( "064eab05.10g", 0x080001, 0x20000, CRC(51d6fbc4) SHA1(e80de7d155b7f263c48ef4ae2702059be3c18e76) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "064e01.2f",   0x0000, 0x10000, CRC(44b9bc52) SHA1(4654d6e14c6956c40a19cb41155accb63f0da338) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "064e12.16k",   0x000000, 0x080000, CRC(e2bdc619) SHA1(04449deb267b0beacfa33640b593eb16194aa0d9) )
	ROM_LOAD32_WORD( "064e11.12k",   0x000002, 0x080000, CRC(2d8ca8b0) SHA1(7c882f79c2402cf75979c681071007d76e4db9ae) )

	ROM_REGION( 0x200000, "k053245", 0 )    /* sprites */
	ROM_LOAD32_WORD( "064e09.7l", 0x000000, 0x100000, CRC(4160c372) SHA1(0b36181e5ccd785c7fb89b9f41e458066a42c3b0) )
	ROM_LOAD32_WORD( "064e07.3l", 0x000002, 0x100000, CRC(64dd673c) SHA1(bea4d17a71dd21c635866ee69b4892dc9d0ab455) )

	ROM_REGION( 0x100000, "k053260", 0 )    /* samples for the 053260 */
	ROM_LOAD( "064e06.1d",    0x0000, 0x100000, CRC(59810df9) SHA1(a0affc6330bdbfab1447dc0cf13c20ff708c2c71) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "ssriders_add.nv", 0x0000, 0x080, CRC(f06242d5) SHA1(181732b07c74c3f5cfa7838ec029ed42d5216b17) )
ROM_END

ROM_START( ssridersjbd )
	ROM_REGION( 0xc0000, "maincpu", 0 ) /* 2*256k and 2*128k for 68000 code */
	ROM_LOAD16_BYTE( "064jbd02.8e",  0x000000, 0x40000, CRC(7acdc1e3) SHA1(09679403abe695758d01fb0161168bc93888f915) )
	ROM_LOAD16_BYTE( "064jbd03.8g",  0x000001, 0x40000, CRC(6a424918) SHA1(3e7a66adc934b1ed4ecd75a36d5a1c133916ac66) )
	ROM_LOAD16_BYTE( "064eab04.10e", 0x080000, 0x20000, CRC(ef2315bd) SHA1(2c8b11321cb5fdb78d760fabca666c0d8cc5b298) )
	ROM_LOAD16_BYTE( "064eab05.10g", 0x080001, 0x20000, CRC(51d6fbc4) SHA1(e80de7d155b7f263c48ef4ae2702059be3c18e76) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "064e01.2f",   0x0000, 0x10000, CRC(44b9bc52) SHA1(4654d6e14c6956c40a19cb41155accb63f0da338) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "064e12.16k",   0x000000, 0x080000, CRC(e2bdc619) SHA1(04449deb267b0beacfa33640b593eb16194aa0d9) )
	ROM_LOAD32_WORD( "064e11.12k",   0x000002, 0x080000, CRC(2d8ca8b0) SHA1(7c882f79c2402cf75979c681071007d76e4db9ae) )

	ROM_REGION( 0x200000, "k053245", 0 )    /* sprites */
	ROM_LOAD32_WORD( "064e09.7l", 0x000000, 0x100000, CRC(4160c372) SHA1(0b36181e5ccd785c7fb89b9f41e458066a42c3b0) )
	ROM_LOAD32_WORD( "064e07.3l", 0x000002, 0x100000, CRC(64dd673c) SHA1(bea4d17a71dd21c635866ee69b4892dc9d0ab455) )

	ROM_REGION( 0x100000, "k053260", 0 )    /* samples for the 053260 */
	ROM_LOAD( "064e06.1d",    0x0000, 0x100000, CRC(59810df9) SHA1(a0affc6330bdbfab1447dc0cf13c20ff708c2c71) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting with invisible error
	ROM_LOAD( "ssriders_jbd.nv", 0x0000, 0x080, CRC(006200e3) SHA1(a6a0640c1a6d69a41db90f1fc935e97f2fa68553) )
ROM_END

ROM_START( ssridersjad )
	ROM_REGION( 0xc0000, "maincpu", 0 ) /* 2*256k and 2*128k for 68000 code */
	ROM_LOAD16_BYTE( "064jad02.8e",  0x000000, 0x40000, CRC(13709ee6) SHA1(89f047711c2d978382fa8259d5dba9223a9c96db) )
	ROM_LOAD16_BYTE( "064jad03.8g",  0x000001, 0x40000, CRC(2fa32229) SHA1(15027f8903389ddc57ada3f79ee5595520b06936) )
	ROM_LOAD16_BYTE( "064eab04.10e", 0x080000, 0x20000, CRC(ef2315bd) SHA1(2c8b11321cb5fdb78d760fabca666c0d8cc5b298) )
	ROM_LOAD16_BYTE( "064eab05.10g", 0x080001, 0x20000, CRC(51d6fbc4) SHA1(e80de7d155b7f263c48ef4ae2702059be3c18e76) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "064e01.2f",   0x0000, 0x10000, CRC(44b9bc52) SHA1(4654d6e14c6956c40a19cb41155accb63f0da338) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "064e12.16k",   0x000000, 0x080000, CRC(e2bdc619) SHA1(04449deb267b0beacfa33640b593eb16194aa0d9) )
	ROM_LOAD32_WORD( "064e11.12k",   0x000002, 0x080000, CRC(2d8ca8b0) SHA1(7c882f79c2402cf75979c681071007d76e4db9ae) )

	ROM_REGION( 0x200000, "k053245", 0 )    /* sprites */
	ROM_LOAD32_WORD( "064e09.7l", 0x000000, 0x100000, CRC(4160c372) SHA1(0b36181e5ccd785c7fb89b9f41e458066a42c3b0) )
	ROM_LOAD32_WORD( "064e07.3l", 0x000002, 0x100000, CRC(64dd673c) SHA1(bea4d17a71dd21c635866ee69b4892dc9d0ab455) )

	ROM_REGION( 0x100000, "k053260", 0 )    /* samples for the 053260 */
	ROM_LOAD( "064e06.1d",    0x0000, 0x100000, CRC(59810df9) SHA1(a0affc6330bdbfab1447dc0cf13c20ff708c2c71) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "ssriders_jad.nv", 0x0000, 0x080, CRC(8962058c) SHA1(2aa470d02c7047c637d216690c281a144b70b4c3) )
ROM_END

ROM_START( ssridersjac )
	ROM_REGION( 0xc0000, "maincpu", 0 ) /* 2*256k and 2*128k for 68000 code */
	ROM_LOAD16_BYTE( "064jac02.8e",  0x000000, 0x40000, CRC(64a35f6f) SHA1(161127360e68054898cbe4d90382cf1682c8870b) )
	ROM_LOAD16_BYTE( "064jac03.8g",  0x000001, 0x40000, CRC(b5957946) SHA1(432b4bfd93e15f71c910b48deb76d65f30b7ce03) )
	ROM_LOAD16_BYTE( "064eab04.10e", 0x080000, 0x20000, CRC(ef2315bd) SHA1(2c8b11321cb5fdb78d760fabca666c0d8cc5b298) )
	ROM_LOAD16_BYTE( "064eab05.10g", 0x080001, 0x20000, CRC(51d6fbc4) SHA1(e80de7d155b7f263c48ef4ae2702059be3c18e76) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "064e01.2f",   0x0000, 0x10000, CRC(44b9bc52) SHA1(4654d6e14c6956c40a19cb41155accb63f0da338) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "064e12.16k",   0x000000, 0x080000, CRC(e2bdc619) SHA1(04449deb267b0beacfa33640b593eb16194aa0d9) )
	ROM_LOAD32_WORD( "064e11.12k",   0x000002, 0x080000, CRC(2d8ca8b0) SHA1(7c882f79c2402cf75979c681071007d76e4db9ae) )

	ROM_REGION( 0x200000, "k053245", 0 )    /* sprites */
	ROM_LOAD32_WORD( "064e09.7l", 0x000000, 0x100000, CRC(4160c372) SHA1(0b36181e5ccd785c7fb89b9f41e458066a42c3b0) )
	ROM_LOAD32_WORD( "064e07.3l", 0x000002, 0x100000, CRC(64dd673c) SHA1(bea4d17a71dd21c635866ee69b4892dc9d0ab455) )

	ROM_REGION( 0x100000, "k053260", 0 )    /* samples for the 053260 */
	ROM_LOAD( "064e06.1d",    0x0000, 0x100000, CRC(59810df9) SHA1(a0affc6330bdbfab1447dc0cf13c20ff708c2c71) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "ssriders_jac.nv", 0x0000, 0x080, CRC(eeb0c55f) SHA1(659524bc31eb3568d251c98a554edbea898c1b45) )
ROM_END

ROM_START( ssridersb )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 2*32k for 68000 code */
	ROM_LOAD16_WORD_SWAP( "sunsetb.03",   0x000000, 0x080000, CRC(37ffe90b) SHA1(3f8542243f2a0c0718056672a906b70af5894a86) )
	ROM_LOAD16_WORD_SWAP( "sunsetb.04",   0x080000, 0x080000, CRC(8ff647b7) SHA1(75144ce928fc4e7d24d9dd50a93e11ea41903bc4) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	// should be sunsetb.09 and .10 from the bootleg, but .09 is a bad dump and .10 matches the parent's 064e11.12k, so we just use the parent's roms
	ROM_LOAD32_WORD( "064e12.16k",   0x000000, 0x080000, CRC(e2bdc619) SHA1(04449deb267b0beacfa33640b593eb16194aa0d9) )
	ROM_LOAD32_WORD( "064e11.12k",   0x000002, 0x080000, CRC(2d8ca8b0) SHA1(7c882f79c2402cf75979c681071007d76e4db9ae) )

	ROM_REGION( 0x200000, "k053245", 0 )    /* sprites */
	// loading and decoding are wrong (encrypted?)
	ROM_LOAD( "sunsetb.05",   0x000000, 0x080000, BAD_DUMP CRC(8a0ff31a) SHA1(fee21d787d1cddd04713e10b1622f3fa231ebc4e) )
	ROM_LOAD( "sunsetb.06",   0x080000, 0x080000, BAD_DUMP CRC(fdf2c887) SHA1(a165c7e6495d870324f59262ad4175a039e199a5) )
	ROM_LOAD( "sunsetb.07",   0x100000, 0x080000, BAD_DUMP CRC(a545b1ed) SHA1(249f1f1a992f05c0dc23bd52785a355a402a0d10) )
	ROM_LOAD( "sunsetb.08",   0x180000, 0x080000, BAD_DUMP CRC(f867cd38) SHA1(633703474010364dc47176965daa873d548da074) )

	ROM_REGION( 0x100000, "oki", 0 )    /* samples */
	ROM_LOAD( "sunsetb.01",   0x000000, 0x080000, CRC(1a8b5ca2) SHA1(4101686c7bf3243273a52fca046b252fc3c78721) )
	ROM_LOAD( "sunsetb.02",   0x080000, 0x080000, CRC(5d485523) SHA1(478119cb6273d870ca04a66e9b964ca0424f6fbd) )
ROM_END

ROM_START( ssriders2 )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 2*32k for 68000 code */
	ROM_LOAD16_WORD_SWAP( "3.bin",   0x000000, 0x080000, CRC(d8d802c5) SHA1(1b5362edd6756586b95b59779a74c804fe69786a) )
	ROM_LOAD16_WORD_SWAP( "4.bin",   0x080000, 0x080000, CRC(8ff647b7) SHA1(75144ce928fc4e7d24d9dd50a93e11ea41903bc4) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "9.bin",   0x000000, 0x080000, CRC(e2bdc619) SHA1(04449deb267b0beacfa33640b593eb16194aa0d9) )
	ROM_LOAD32_WORD( "10.bin",  0x000002, 0x080000, CRC(2d8ca8b0) SHA1(7c882f79c2402cf75979c681071007d76e4db9ae) )

	ROM_REGION( 0x200000, "k053245", 0 )    /* sprites */
	// loading and decoding are wrong (encrypted?)
	ROM_LOAD( "5.bin",   0x000000, 0x080000, CRC(4ee77259) SHA1(92cb3ae296b1c42b70ce636a989c03d898ca35cf) )
	ROM_LOAD( "6.bin",   0x080000, 0x080000, CRC(fdf2c887) SHA1(a165c7e6495d870324f59262ad4175a039e199a5) )
	ROM_LOAD( "7.bin",   0x100000, 0x080000, CRC(3f1f7222) SHA1(14547c308180e5009f3ea8edcea58d96aa039919) )
	ROM_LOAD( "8.bin",   0x180000, 0x080000, CRC(a91b9171) SHA1(e7002fe176196c297073ebf48e6fa5b1fe62caa1) )

	ROM_REGION( 0x100000, "oki", 0 )    /* samples */
	ROM_LOAD( "1.bin",   0x000000, 0x080000, CRC(1a8b5ca2) SHA1(4101686c7bf3243273a52fca046b252fc3c78721) )
	ROM_LOAD( "2.bin",   0x080000, 0x080000, CRC(5d485523) SHA1(478119cb6273d870ca04a66e9b964ca0424f6fbd) )
ROM_END

ROM_START( thndrx2 )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 2*32k for 68000 code */
	ROM_LOAD16_BYTE( "073-ea-l02.11c", 0x000000, 0x20000, CRC(eae02b51) SHA1(ac513919b183d5353792418e6190c484c5cf1bcd) )
	ROM_LOAD16_BYTE( "073-ea-l03.12c", 0x000001, 0x20000, CRC(738ed007) SHA1(4539fd37ca9d7b25ee3b79c428c8f6c3be484bdf) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "073-c01.4f",   0x0000, 0x10000, CRC(44ebe83c) SHA1(9274df6affa4f0456d273ff3aa1bda7d2a20416e) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "073-c06.16k",  0x000000, 0x080000, CRC(24e22b42) SHA1(7e5e14495bd4adbe5d1cbec75262c9c4c83f5793) )
	ROM_LOAD32_WORD( "073-c05.12k",  0x000002, 0x080000, CRC(952a935f) SHA1(87ed81616a243d679f7501db7acdd8b6617f85a3) )

	ROM_REGION( 0x100000, "k051960", 0 )    /* sprites */
	ROM_LOAD32_WORD( "073-c07.7k",   0x000000, 0x080000, CRC(14e93f38) SHA1(bf111b68be722c9c2f0f9c7700b3af6cd8fd28be) )
	ROM_LOAD32_WORD( "073-c08.3k",   0x000002, 0x080000, CRC(09fab3ab) SHA1(af54c7bfe8edc5b5ea2c4fba4d5c637cfcbbeff5) )

	ROM_REGION( 0x80000, "k053260", 0 ) /* samples for the 053260 */
	ROM_LOAD( "073-b04.2d",   0x0000, 0x80000, CRC(05287a0b) SHA1(10784b8be6a93a5ebf22a884f99c116e51ae8743) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "thndrx2.nv", 0x0000, 0x080, CRC(ca613e98) SHA1(bde0d1722acfef19cf8ec091bdc296f8b9fa8125) )
ROM_END

ROM_START( thndrx2a )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 2*32k for 68000 code */
	ROM_LOAD16_BYTE( "073-aa-m02.11c", 0x000000, 0x20000, CRC(5b5b4cc0) SHA1(9f67169fba4523e2893e5ecf17b1be8cdedba83e) )
	ROM_LOAD16_BYTE( "073-aa-m03.12c", 0x000001, 0x20000, CRC(320435a8) SHA1(5f656867049b614b0834ef6d8e36fe86118ea1cf) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "073-c01.4f",   0x0000, 0x10000, CRC(44ebe83c) SHA1(9274df6affa4f0456d273ff3aa1bda7d2a20416e) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "073-c06.16k",  0x000000, 0x080000, CRC(24e22b42) SHA1(7e5e14495bd4adbe5d1cbec75262c9c4c83f5793) )
	ROM_LOAD32_WORD( "073-c05.12k",  0x000002, 0x080000, CRC(952a935f) SHA1(87ed81616a243d679f7501db7acdd8b6617f85a3) )

	ROM_REGION( 0x100000, "k051960", 0 )    /* sprites */
	ROM_LOAD32_WORD( "073-c07.7k",   0x000000, 0x080000, CRC(14e93f38) SHA1(bf111b68be722c9c2f0f9c7700b3af6cd8fd28be) )
	ROM_LOAD32_WORD( "073-c08.3k",   0x000002, 0x080000, CRC(09fab3ab) SHA1(af54c7bfe8edc5b5ea2c4fba4d5c637cfcbbeff5) )

	ROM_REGION( 0x80000, "k053260", 0 ) /* samples for the 053260 */
	ROM_LOAD( "073-b04.2d",   0x0000, 0x80000, CRC(05287a0b) SHA1(10784b8be6a93a5ebf22a884f99c116e51ae8743) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "thndrx2a.nv", 0x0000, 0x080, CRC(f7893d00) SHA1(1165ad3485749015458df2840a34b5b0e1810aad) )
ROM_END

ROM_START( thndrx2j )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 2*32k for 68000 code */
	ROM_LOAD16_BYTE( "073-ja-k02.11c", 0x000000, 0x20000, CRC(0c8b2d3f) SHA1(44ca5d96d8f85ae2760df4e1c339916e0a76143f) )
	ROM_LOAD16_BYTE( "073-ja-k03.12c", 0x000001, 0x20000, CRC(3803b427) SHA1(95b755c70ac55af604c6b44bc41b761efce19f48) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "073-c01.4f",   0x0000, 0x10000, CRC(44ebe83c) SHA1(9274df6affa4f0456d273ff3aa1bda7d2a20416e) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "073-c06.16k",  0x000000, 0x080000, CRC(24e22b42) SHA1(7e5e14495bd4adbe5d1cbec75262c9c4c83f5793) )
	ROM_LOAD32_WORD( "073-c05.12k",  0x000002, 0x080000, CRC(952a935f) SHA1(87ed81616a243d679f7501db7acdd8b6617f85a3) )

	ROM_REGION( 0x100000, "k051960", 0 )    /* sprites */
	ROM_LOAD32_WORD( "073-c07.7k",   0x000000, 0x080000, CRC(14e93f38) SHA1(bf111b68be722c9c2f0f9c7700b3af6cd8fd28be) )
	ROM_LOAD32_WORD( "073-c08.3k",   0x000002, 0x080000, CRC(09fab3ab) SHA1(af54c7bfe8edc5b5ea2c4fba4d5c637cfcbbeff5) )

	ROM_REGION( 0x80000, "k053260", 0 ) /* samples for the 053260 */
	ROM_LOAD( "073-b04.2d",   0x0000, 0x80000, CRC(05287a0b) SHA1(10784b8be6a93a5ebf22a884f99c116e51ae8743) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "thndrx2j.nv", 0x0000, 0x080, CRC(83b4799b) SHA1(da4f443cbcb06aa5983423cb6fecc1f803235f61) )
ROM_END


ROM_START( prmrsocr )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 2*256k for 68000 code */
	ROM_LOAD16_BYTE( "101eab08.1h", 0x000000, 0x40000, CRC(47208de6) SHA1(fe4ef56688d4a50f67a604357e7beea785106cd1) ) // 3.bin
	ROM_LOAD16_BYTE( "101eab07.4h", 0x000001, 0x40000, CRC(5f408eca) SHA1(f2f6e126bfdcf884b477f49cb95f5e673357e9e0) ) // 3.bin

	ROM_REGION( 0x30000, "audiocpu", 0 )
	ROM_LOAD( "101c05.5e",   0x00000, 0x20000, CRC(02c3679f) SHA1(e6d878185e73baca24ac98891c647856be9353c4) ) // 1.bin
	ROM_RELOAD(              0x10000, 0x20000 )

	// the tile and sprite ROMs are actually 16-bit ROMs on a 32-bit bus, but the data lines are
	// swapped so that D0-D7 and D16-D23 come from one ROM and D8-D15 and D24-D31 from the other
	ROM_REGION( 0x080000, "k052109", 0 )    /* tiles */
	ROM_LOAD16_BYTE( "101a12.12l", 0x000000, 0x040000, CRC(33530d7f) SHA1(87859ad058fb79e357101675706373f83a3f23d4) )
	ROM_LOAD16_BYTE( "101a11.12k", 0x000001, 0x040000, CRC(7f773271) SHA1(0c6a62c6eb1897e88e893576f751e3d4fc788036) )

	ROM_REGION( 0x400000, "k053245", 0 )    /* sprites */
	ROM_LOAD16_BYTE( "101a09.3l", 0x000000, 0x200000, CRC(b6a1b424) SHA1(4cf7bb4b8176977dea10fb80fcd9d6e24cc6d1b9) )
	ROM_LOAD16_BYTE( "101a10.8l", 0x000001, 0x200000, CRC(bbd58adc) SHA1(ad9bd4df995de6e6290f27c58c7892c7191802e4) )

	ROM_REGION( 0x080000, "zoom", 0 )   /* 053936 tiles */
	ROM_LOAD( "101a03.18f",   0x000000, 0x080000, CRC(59a1a91c) SHA1(f596a40784a671e97116df6561682eb6c5c44e08) )

	ROM_REGION( 0x040000, "user1", 0 )  /* 053936 tilemaps */
	ROM_LOAD( "101a01.18d",   0x000000, 0x020000, CRC(716f910f) SHA1(fbe69cac266084ea1efb094a7f863dca39f12500) )
	ROM_LOAD( "101a02.16d",   0x020000, 0x020000, CRC(222869c7) SHA1(0a9bea294ff3281f316dd4beecc4c94d75d52b49) )

	ROM_REGION( 0x200000, "k054539", 0 )    /* samples for the 054539 */
	ROM_LOAD( "101a06.1d",    0x0000, 0x200000, CRC(4f48e043) SHA1(f50e8642d9d3a028c243777640e7cd13da1abf86) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "prmrsocr_eab.nv", 0x0000, 0x080, CRC(daf60801) SHA1(de21774c4276ec399745c26a9278f7c58dbe6ad0) )
ROM_END

ROM_START( prmrsocrj )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 2*256k for 68000 code */
	ROM_LOAD16_BYTE( "101jab08.1h", 0x000000, 0x40000, CRC(c22b528c) SHA1(6c96ba573f7bb5be1d52d9352f57d7a402bc96b4) )
	ROM_LOAD16_BYTE( "101jab07.4h", 0x000001, 0x40000, CRC(06e7acaf) SHA1(d7197bb1c3b28cbe82dd4e25302e00f7c1838208) )

	ROM_REGION( 0x30000, "audiocpu", 0 )
	ROM_LOAD( "101c05.5e",   0x00000, 0x20000, CRC(02c3679f) SHA1(e6d878185e73baca24ac98891c647856be9353c4) )
	ROM_RELOAD(              0x10000, 0x20000 )

	// the tile and sprite ROMs are actually 16-bit ROMs on a 32-bit bus, but the data lines are
	// swapped so that D0-D7 and D16-D23 come from one ROM and D8-D15 and D24-D31 from the other
	ROM_REGION( 0x080000, "k052109", 0 )    /* tiles */
	ROM_LOAD16_BYTE( "101a12.12l", 0x000000, 0x040000, CRC(33530d7f) SHA1(87859ad058fb79e357101675706373f83a3f23d4) )
	ROM_LOAD16_BYTE( "101a11.12k", 0x000001, 0x040000, CRC(7f773271) SHA1(0c6a62c6eb1897e88e893576f751e3d4fc788036) )

	ROM_REGION( 0x400000, "k053245", 0 )    /* sprites */
	ROM_LOAD16_BYTE( "101a09.3l", 0x000000, 0x200000, CRC(b6a1b424) SHA1(4cf7bb4b8176977dea10fb80fcd9d6e24cc6d1b9) )
	ROM_LOAD16_BYTE( "101a10.8l", 0x000001, 0x200000, CRC(bbd58adc) SHA1(ad9bd4df995de6e6290f27c58c7892c7191802e4) )

	ROM_REGION( 0x080000, "zoom", 0 )   /* 053936 tiles */
	ROM_LOAD( "101a03.18f",   0x000000, 0x080000, CRC(59a1a91c) SHA1(f596a40784a671e97116df6561682eb6c5c44e08) )

	ROM_REGION( 0x040000, "user1", 0 )  /* 053936 tilemaps */
	ROM_LOAD( "101a01.18d",   0x000000, 0x020000, CRC(716f910f) SHA1(fbe69cac266084ea1efb094a7f863dca39f12500) )
	ROM_LOAD( "101a02.16d",   0x020000, 0x020000, CRC(222869c7) SHA1(0a9bea294ff3281f316dd4beecc4c94d75d52b49) )

	ROM_REGION( 0x200000, "k054539", 0 )    /* samples for the 054539 */
	ROM_LOAD( "101a06.1d",    0x0000, 0x200000, CRC(4f48e043) SHA1(f50e8642d9d3a028c243777640e7cd13da1abf86) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "prmrsocr_jab.nv", 0x0000, 0x080, CRC(5a88f95a) SHA1(18fdb598eee3958c45694bdac77e02790c936968) )
ROM_END

} // anonymous namespace


//    YEAR  NAME         PARENT    MACHINE   INPUT      STATE        INIT         MONITOR COMPANY    FULLNAME,FLAGS
GAME( 1990, punkshot,    0,        punkshot, punkshot,  tmnt2_state, empty_init,  ROT0,   "Konami",  "Punk Shot (US 4 Players)",            MACHINE_SUPPORTS_SAVE )
GAME( 1990, punkshot2,   punkshot, punkshot, punksht2,  tmnt2_state, empty_init,  ROT0,   "Konami",  "Punk Shot (US 2 Players)",            MACHINE_SUPPORTS_SAVE )
GAME( 1990, punkshot2e,  punkshot, punkshot, punksht2,  tmnt2_state, empty_init,  ROT0,   "Konami",  "Punk Shot (World 2 Players)",         MACHINE_SUPPORTS_SAVE )
GAME( 1990, punkshotj,   punkshot, punkshot, punkshtj,  tmnt2_state, empty_init,  ROT0,   "Konami",  "Punk Shot (Japan 2 Players)",         MACHINE_SUPPORTS_SAVE )
GAME( 1990, punkshot2a,  punkshot, punkshot, punksht2,  tmnt2_state, empty_init,  ROT0,   "Konami",  "Punk Shot (Asia 2 Players, hacked?)", MACHINE_SUPPORTS_SAVE )

GAME( 1990, lgtnfght,    0,        lgtnfght, lgtnfght,  tmnt2_state, empty_init,  ROT90,  "Konami",  "Lightning Fighters (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, lgtnfghta,   lgtnfght, lgtnfght, lgtnfght,  tmnt2_state, empty_init,  ROT90,  "Konami",  "Lightning Fighters (Asia)",  MACHINE_SUPPORTS_SAVE )
GAME( 1990, lgtnfghtu,   lgtnfght, lgtnfght, lgtnfght,  tmnt2_state, empty_init,  ROT90,  "Konami",  "Lightning Fighters (US)",    MACHINE_SUPPORTS_SAVE )
GAME( 1990, trigon,      lgtnfght, lgtnfght, trigon,    tmnt2_state, empty_init,  ROT90,  "Konami",  "Trigon (Japan)",             MACHINE_SUPPORTS_SAVE )

GAME( 1991, blswhstl,    0,        blswhstl, blswhstl,  tmnt2_state, empty_init,  ROT90,  "Konami",  "Bells & Whistles (World, version L)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, blswhstla,   blswhstl, blswhstl, blswhstl,  tmnt2_state, empty_init,  ROT90,  "Konami",  "Bells & Whistles (Asia, version M)",  MACHINE_SUPPORTS_SAVE )
GAME( 1991, detatwin,    blswhstl, blswhstl, blswhstl,  tmnt2_state, empty_init,  ROT90,  "Konami",  "Detana!! Twin Bee (Japan, version J)", MACHINE_SUPPORTS_SAVE )

GAME( 1991, glfgreat,    0,        glfgreat, glfgreat,  glfgreat_state, empty_init, ROT0, "Konami", "Golfing Greats (World, version L)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 1991, glfgreatu,   glfgreat, glfgreat, glfgreatu, glfgreat_state, empty_init, ROT0, "Konami", "Golfing Greats (US, version K)",    MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 1991, glfgreatj,   glfgreat, glfgreat, glfgreatj, glfgreat_state, empty_init, ROT0, "Konami", "Golfing Greats (Japan, version J)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )

GAME( 1991, tmnt2,       0,        tmnt2,    ssridr4p,  tmnt2_state, empty_init,  ROT0,   "Konami",  "Teenage Mutant Ninja Turtles - Turtles in Time (4 Players ver UAA)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, tmnt2a,      tmnt2,    tmnt2,    ssrid4ps,  tmnt2_state, empty_init,  ROT0,   "Konami",  "Teenage Mutant Ninja Turtles - Turtles in Time (4 Players ver ADA)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, tmnt2o,      tmnt2,    tmnt2,    ssrid4ps,  tmnt2_state, empty_init,  ROT0,   "Konami",  "Teenage Mutant Ninja Turtles - Turtles in Time (4 Players ver OAA)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, tmht22pe,    tmnt2,    tmnt2,    ssriders,  tmnt2_state, empty_init,  ROT0,   "Konami",  "Teenage Mutant Hero Turtles - Turtles in Time (2 Players ver EBA)",  MACHINE_SUPPORTS_SAVE )
GAME( 1991, tmht24pe,    tmnt2,    tmnt2,    ssridr4p,  tmnt2_state, empty_init,  ROT0,   "Konami",  "Teenage Mutant Hero Turtles - Turtles in Time (4 Players ver EAA)",  MACHINE_SUPPORTS_SAVE )
GAME( 1991, tmnt22pu,    tmnt2,    tmnt2,    ssriders,  tmnt2_state, empty_init,  ROT0,   "Konami",  "Teenage Mutant Ninja Turtles - Turtles in Time (2 Players ver UDA)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, tmnt24pu,    tmnt2,    tmnt2,    ssrid4ps,  tmnt2_state, empty_init,  ROT0,   "Konami",  "Teenage Mutant Ninja Turtles - Turtles in Time (4 Players ver UEA)", MACHINE_SUPPORTS_SAVE )

GAME( 1993, qgakumon,    0,        tmnt2,    qgakumon,  tmnt2_state, empty_init,  ROT0,   "Konami",  "Quiz Gakumon no Susume (Japan ver. JA1 Type H)", MACHINE_SUPPORTS_SAVE )

GAME( 1991, ssriders,    0,        ssriders, ssridr4p,  tmnt2_state, empty_init,  ROT0,   "Konami",  "Sunset Riders (4 Players ver EAC)",           MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1991, ssriderseaa, ssriders, ssriders, ssridr4p,  tmnt2_state, empty_init,  ROT0,   "Konami",  "Sunset Riders (4 Players ver EAA)",           MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1991, ssridersebd, ssriders, ssriders, ssriders,  tmnt2_state, empty_init,  ROT0,   "Konami",  "Sunset Riders (2 Players ver EBD)",           MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1991, ssridersebc, ssriders, ssriders, ssriders,  tmnt2_state, empty_init,  ROT0,   "Konami",  "Sunset Riders (2 Players ver EBC)",           MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1991, ssridersuda, ssriders, ssriders, ssrid4ps,  tmnt2_state, empty_init,  ROT0,   "Konami",  "Sunset Riders (4 Players ver UDA)",           MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1991, ssridersuac, ssriders, ssriders, ssridr4p,  tmnt2_state, empty_init,  ROT0,   "Konami",  "Sunset Riders (4 Players ver UAC)",           MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1991, ssridersuab, ssriders, ssriders, ssridr4p,  tmnt2_state, empty_init,  ROT0,   "Konami",  "Sunset Riders (4 Players ver UAB)",           MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1991, ssridersubc, ssriders, ssriders, ssriders,  tmnt2_state, empty_init,  ROT0,   "Konami",  "Sunset Riders (2 Players ver UBC)",           MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1991, ssridersadd, ssriders, ssriders, ssrid4ps,  tmnt2_state, empty_init,  ROT0,   "Konami",  "Sunset Riders (4 Players ver ADD)",           MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1991, ssridersabd, ssriders, ssriders, ssriders,  tmnt2_state, empty_init,  ROT0,   "Konami",  "Sunset Riders (2 Players ver ABD)",           MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1991, ssridersjad, ssriders, ssriders, ssridr4p,  tmnt2_state, empty_init,  ROT0,   "Konami",  "Sunset Riders (4 Players ver JAD)",           MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1991, ssridersjac, ssriders, ssriders, ssridr4p,  tmnt2_state, empty_init,  ROT0,   "Konami",  "Sunset Riders (4 Players ver JAC)",           MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1991, ssridersjbd, ssriders, ssriders, ssriders,  tmnt2_state, empty_init,  ROT0,   "Konami",  "Sunset Riders (2 Players ver JBD)",           MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1991, ssridersb,   ssriders, sunsetbl, sunsetbl,  sunsetbl_state, empty_init, ROT0, "bootleg", "Sunset Riders (bootleg 4 Players ver ADD)",   MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1991, ssriders2,   ssriders, sunsetbl, sunsetbl,  sunsetbl_state, empty_init, ROT0, "bootleg", "Sunset Riders 2 (bootleg 4 Players ver ADD)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )

GAME( 1991, thndrx2,     0,        thndrx2,  thndrx2,   tmnt2_state, empty_init,  ROT0, "Konami",  "Thunder Cross II (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, thndrx2a,    thndrx2,  thndrx2,  thndrx2,   tmnt2_state, empty_init,  ROT0, "Konami",  "Thunder Cross II (Asia)",  MACHINE_SUPPORTS_SAVE )
GAME( 1991, thndrx2j,    thndrx2,  thndrx2,  thndrx2,   tmnt2_state, empty_init,  ROT0, "Konami",  "Thunder Cross II (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1993, prmrsocr,    0,        prmrsocr, prmrsocr,  prmrsocr_state, empty_init, ROT0, "Konami",  "Premier Soccer (ver EAB)", MACHINE_SUPPORTS_SAVE )
GAME( 1993, prmrsocrj,   prmrsocr, prmrsocr, prmrsocr,  prmrsocr_state, empty_init, ROT0, "Konami",  "Premier Soccer (ver JAB)", MACHINE_SUPPORTS_SAVE )
