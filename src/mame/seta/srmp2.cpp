// license:BSD-3-Clause
// copyright-holders:Yochizo, Takahiro Nogi
/***************************************************************************

Super Real Mahjong P2
-------------------------------------
driver by Yochizo and Takahiro Nogi

  Yochizo took charge of video and I/O part.
  Takahiro Nogi took charge of sound, I/O and NVRAM part.

  ... and this is based on "seta.c" driver written by Luca Elia.

  Thanks for your reference, Takahiro Nogi and Luca Elia.


Supported games :
==================
 Super Real Mahjong Part1     (C) 1987 Seta    P0-023
 Super Real Mahjong Part2     (C) 1987 Seta    P0-023
 Super Real Mahjong Part3     (C) 1988 Seta    P0-037A
 Real Mahjong Gold Yumehai    (C) 1988 Seta    P0-064A
 Mahjong Yuugi (set 1)        (C) 1990 Visco   P0-049A
 Mahjong Yuugi (set 2)        (C) 1990 Visco   P0-049A
 Mahjong Pon Chin Kan (set 1) (C) 1991 Visco   P0-049A
 Mahjong Pon Chin Kan (set 2) (C) 1991 Visco   P0-049A


System specs :
===============
   CPU       : 68000 (8MHz)
   Sound     : AY8910 + MSM5205
   Chips     : X1-001, X1-002A, X1-003, X1-004x2, X0-005 x2
           X1-001, X1-002A (64 pins) : Sprites
           X1-003          (?? pins) : Video output
           X1-004          (52 pins) : Inputs
           X0-005          (40 pins) : Interface


Known issues :
===============
 - IOX might be either a shared component between PCBs or every game have its own configuration.
   For now I've opted for the latter solution, until an HW test will be done ...
 - X0-005 is a probable MCU, most likely a 8741/8742 UPI type like the "M-Chip" in tnzs.cpp.
   One X0-005 handles the inputs; the other drives the MSM5205 with the aid of a 8243 expander.
   Are both of these functions really performed by the same (undumped) internal program?
 - AY-3-8910 sound may be wrong.
 - CPU clock of srmp3 does not match the real machine.
 - MSM5205 clock frequency in srmp3 is wrong.
 - irq acknowledge in either srmp2 and mjyuugi is a raw guess, there are multiple
   writes at the end of irq services and it isn't easy to determine what's the
   exact irq ack without HW tests ...

Note:
======
 - In mjyuugi and mjyuugia, DSW3 (Debug switch) is available if you
   turn on the cheat switch.


****************************************************************************/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "sound/msm5205.h"
#include "video/x1_001.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

namespace {

class srmp2_state : public driver_device
{
public:
	srmp2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_spritegen(*this, "spritegen"),
		m_msm(*this, "msm"),
		m_adpcm_rom(*this, "adpcm"),
		m_mainbank(*this, "mainbank"),
		m_keys(*this, "KEY%u", 0U),
		m_service(*this, "SERVICE")
	{ }

	void mjyuugi(machine_config &config);
	void srmp2(machine_config &config);
	void rmgoldyh(machine_config &config);
	void srmp3(machine_config &config);

private:
	struct iox_t
	{
		int reset = 0, ff_event,ff_1 = 0, protcheck[4]{}, protlatch[4]{};
		uint8_t data = 0;
		uint8_t mux = 0;
		uint8_t ff = 0;
	};

	required_device<cpu_device> m_maincpu;
	required_device<x1_001_device> m_spritegen;
	required_device<msm5205_device> m_msm;
	required_region_ptr<uint8_t> m_adpcm_rom;
	optional_memory_bank m_mainbank;
	required_ioport_array<8> m_keys;
	required_ioport m_service;

	uint8_t m_color_bank = 0;
	uint8_t m_gfx_bank = 0;
	uint8_t m_adpcm_bank = 0;
	int16_t m_adpcm_data = 0;
	uint32_t m_adpcm_sptr = 0;
	uint32_t m_adpcm_eptr = 0;
	iox_t m_iox;

	// common
	uint8_t vox_status_r();
	uint8_t iox_mux_r();
	uint8_t iox_status_r();
	void iox_command_w(uint8_t data);
	void iox_data_w(uint8_t data);
	void adpcm_int(int state);

	// mjuugi
	void mjyuugi_flags_w(uint16_t data);
	void mjyuugi_adpcm_bank_w(uint16_t data);
	uint8_t mjyuugi_irq2_ack_r(address_space &space);
	uint8_t mjyuugi_irq4_ack_r(address_space &space);

	// rmgoldyh
	void rmgoldyh_rombank_w(uint8_t data);

	// srmp2
	void srmp2_irq2_ack_w(uint8_t data);
	void srmp2_irq4_ack_w(uint8_t data);
	void srmp2_flags_w(uint16_t data);
	void adpcm_code_w(uint8_t data);

	// srmp3
	void srmp3_rombank_w(uint8_t data);
	void srmp3_flags_w(uint8_t data);
	void srmp3_irq_ack_w(uint8_t data);

	virtual void machine_start() override ATTR_COLD;
	DECLARE_MACHINE_START(srmp2);
	void srmp2_palette(palette_device &palette) const;
	DECLARE_MACHINE_START(srmp3);
	void srmp3_palette(palette_device &palette) const;
	DECLARE_MACHINE_START(rmgoldyh);
	DECLARE_MACHINE_START(mjyuugi);

	uint32_t screen_update_srmp2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_srmp3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	X1_001_SPRITE_GFXBANK_CB_MEMBER(srmp3_gfxbank_callback);

	uint8_t iox_key_matrix_calc(uint8_t p_side);

	void mjyuugi_map(address_map &map) ATTR_COLD;
	void rmgoldyh_io_map(address_map &map) ATTR_COLD;
	void rmgoldyh_map(address_map &map) ATTR_COLD;
	void srmp2_map(address_map &map) ATTR_COLD;
	void srmp3_io_map(address_map &map) ATTR_COLD;
	void srmp3_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

  Video hardware

***************************************************************************/

void srmp2_state::srmp2_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();
	for (int i = 0; i < palette.entries(); i++)
	{
		int const col = (color_prom[i] << 8) + color_prom[i + palette.entries()];
		palette.set_pen_color(i ^ 0x0f, pal5bit(col >> 10), pal5bit(col >> 5), pal5bit(col >> 0));
	}
}


void srmp2_state::srmp3_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();
	for (int i = 0; i < palette.entries(); i++)
	{
		int const col = (color_prom[i] << 8) + color_prom[i + palette.entries()];
		palette.set_pen_color(i,pal5bit(col >> 10), pal5bit(col >> 5), pal5bit(col >> 0));
	}
}

X1_001_SPRITE_GFXBANK_CB_MEMBER(srmp2_state::srmp3_gfxbank_callback)
{
	return (code & 0x3fff) + ((code & 0x2000) ? (m_gfx_bank<<13) : 0);
}


uint32_t srmp2_state::screen_update_srmp2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0x1ff, cliprect);

	m_spritegen->set_colorbase(m_color_bank<<5);

	m_spritegen->draw_sprites(screen,bitmap,cliprect,0x1000);
	return 0;
}

uint32_t srmp2_state::screen_update_srmp3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0x1f0, cliprect);

	m_spritegen->draw_sprites(screen,bitmap,cliprect,0x1000);
	return 0;
}


/***************************************************************************

  Configure IOX

***************************************************************************/

void srmp2_state::machine_start()
{
	m_adpcm_sptr = 0;
	m_adpcm_eptr = 0;

	save_item(NAME(m_adpcm_bank));
	save_item(NAME(m_adpcm_data));
	save_item(NAME(m_adpcm_sptr));
	save_item(NAME(m_adpcm_eptr));
	save_item(NAME(m_iox.mux));
	save_item(NAME(m_iox.ff));
	save_item(NAME(m_iox.data));
}

MACHINE_START_MEMBER(srmp2_state,srmp2)
{
	machine_start();

	m_iox.reset = 0x1f;
	m_iox.ff_event = -1;
	m_iox.ff_1 = 0x00;
	/* note: protection in srmp1/mjyuugi/ponchin is never checked, assume to be the same */
	m_iox.protcheck[0] = 0x60; m_iox.protlatch[0] = 0x2a;
	m_iox.protcheck[1] = -1;   m_iox.protlatch[1] = -1;
	m_iox.protcheck[2] = -1;   m_iox.protlatch[2] = -1;
	m_iox.protcheck[3] = -1;   m_iox.protlatch[3] = -1;

	save_item(NAME(m_color_bank));
}

MACHINE_START_MEMBER(srmp2_state,srmp3)
{
	machine_start();

	m_iox.reset = 0xc8;
	m_iox.ff_event = 0xef;
	m_iox.ff_1 = -1;
	m_iox.protcheck[0] = 0x49; m_iox.protlatch[0] = 0xc9;
	m_iox.protcheck[1] = 0x4c; m_iox.protlatch[1] = 0x00;
	m_iox.protcheck[2] = 0x1c; m_iox.protlatch[2] = 0x04;
	m_iox.protcheck[3] = 0x45; m_iox.protlatch[3] = 0x00;

	m_mainbank->configure_entries(0, 16, memregion("maincpu")->base(), 0x2000);

	save_item(NAME(m_gfx_bank));
}

MACHINE_START_MEMBER(srmp2_state,rmgoldyh)
{
	machine_start();

	m_iox.reset = 0xc8;
	m_iox.ff_event = 0xff;
	m_iox.ff_1 = -1;
	m_iox.protcheck[0] = 0x43; m_iox.protlatch[0] = 0x9a;
	m_iox.protcheck[1] = 0x45; m_iox.protlatch[1] = 0x00;
	m_iox.protcheck[2] = -1;   m_iox.protlatch[2] = -1;
	m_iox.protcheck[3] = -1;   m_iox.protlatch[3] = -1;

	m_mainbank->configure_entries(0, 32, memregion("maincpu")->base(), 0x2000);

	save_item(NAME(m_gfx_bank));
}

MACHINE_START_MEMBER(srmp2_state,mjyuugi)
{
	machine_start();

	m_iox.reset = 0x1f;
	m_iox.ff_event = -1;
	m_iox.ff_1 = 0x00;
	/* note: protection in srmp1/mjyuugi/ponchin is never checked, assume to be the same */
	m_iox.protcheck[0] = 0x60; m_iox.protlatch[0] = 0x2a;
	m_iox.protcheck[1] = -1;   m_iox.protlatch[1] = -1;
	m_iox.protcheck[2] = -1;   m_iox.protlatch[2] = -1;
	m_iox.protcheck[3] = -1;   m_iox.protlatch[3] = -1;

	save_item(NAME(m_gfx_bank));
}

/***************************************************************************

  Memory Handler(s)

***************************************************************************/

void srmp2_state::srmp2_flags_w(uint16_t data)
{
/*
    ---- ---x : Coin Counter
    ---x ---- : Coin Lock Out
    --x- ---- : ADPCM Bank
    x--- ---- : Palette Bank
*/


	machine().bookkeeping().coin_counter_w(0, BIT(data, 0) );
	machine().bookkeeping().coin_lockout_w(0, BIT(~data, 4) );
	m_adpcm_bank = ( (data & 0x20) >> 5 );
	m_color_bank = ( (data & 0x80) >> 7 );
}


void srmp2_state::mjyuugi_flags_w(uint16_t data)
{
/*
    ---- ---x : Coin Counter
    ---x ---- : Coin Lock Out
*/

	machine().bookkeeping().coin_counter_w(0, BIT(data, 0) );
	machine().bookkeeping().coin_lockout_w(0, BIT(~data, 4) );
}


void srmp2_state::mjyuugi_adpcm_bank_w(uint16_t data)
{
/*
    ---- xxxx : ADPCM Bank
    --xx ---- : GFX Bank
*/

	m_adpcm_bank = (data & 0x0f);
	m_gfx_bank = ((data >> 4) & 0x03);
}


void srmp2_state::adpcm_code_w(uint8_t data)
{
/*
    - Received data may be playing ADPCM number.
    - 0x000000 - 0x0000ff and 0x010000 - 0x0100ff are offset table.
    - When the hardware receives the ADPCM number, it refers the offset
      table and plays the ADPCM for itself.
*/

	m_adpcm_sptr = (m_adpcm_rom[((m_adpcm_bank * 0x10000) + (data << 2) + 0)] << 8);
	m_adpcm_eptr = (m_adpcm_rom[((m_adpcm_bank * 0x10000) + (data << 2) + 1)] << 8);
	m_adpcm_eptr  = (m_adpcm_eptr - 1) & 0x0ffff;

	m_adpcm_sptr += (m_adpcm_bank * 0x10000);
	m_adpcm_eptr += (m_adpcm_bank * 0x10000);

	//printf("%02x %08x %08x %08x\n",data,m_adpcm_sptr,m_adpcm_eptr,((m_adpcm_bank * 0x10000) + (data << 2) + 0));

	m_msm->reset_w(0);
	m_adpcm_data = -1;
}


void srmp2_state::adpcm_int(int state)
{
	if (m_adpcm_sptr)
	{
		if (m_adpcm_data == -1)
		{
			m_adpcm_data = m_adpcm_rom[m_adpcm_sptr];

			if (m_adpcm_sptr >= m_adpcm_eptr)
			{
				m_msm->reset_w(1);
				m_adpcm_data = 0;
				m_adpcm_sptr = 0;
			}
			else
			{
				m_msm->data_w((m_adpcm_data >> 4) & 0x0f);
			}
		}
		else
		{
			m_msm->data_w((m_adpcm_data >> 0) & 0x0f);
			m_adpcm_sptr++;
			m_adpcm_data = -1;
		}
	}
	else
	{
		m_msm->reset_w(1);
	}
}

uint8_t srmp2_state::vox_status_r()
{
	return 1;
}


uint8_t srmp2_state::iox_key_matrix_calc(uint8_t p_side)
{
	for (int i = 0; i < 4; i++)
	{
		for (int t = 0; t < 8; t ++)
		{
			if (!BIT(m_keys[i | (p_side << 2)]->read(), t))
				return (i << 3) | t | (p_side << 5);
		}
	}

	return 0;
}

uint8_t srmp2_state::iox_mux_r()
{
	/* first off check any pending protection value */
	for (int i = 0; i < 4; i++)
	{
		if (m_iox.protcheck[i] == -1)
			continue; //skip

		if (m_iox.data == m_iox.protcheck[i])
		{
			if (!machine().side_effects_disabled())
				m_iox.data = 0; //clear write latch
			return m_iox.protlatch[i];
		}
	}

	if (m_iox.ff == 0)
	{
		if (m_iox.mux != 1 && m_iox.mux != 2 && m_iox.mux != 4)
			return 0xff; //unknown command

		/* both side checks */
		if (m_iox.mux == 1)
		{
			uint8_t const p1_side = iox_key_matrix_calc(0);
			uint8_t const p2_side = iox_key_matrix_calc(1);

			if (p1_side != 0)
				return p1_side;
			else
				return p2_side;
		}

		/* check individual input side */
		return iox_key_matrix_calc((m_iox.mux == 2) ? 0 : 1);
	}

	return m_service->read() & 0xff;
}

uint8_t srmp2_state::iox_status_r()
{
	return 1;
}

void srmp2_state::iox_command_w(uint8_t data)
{
	/*
	bitwise command port apparently
	0x01: selects both sides
	0x02: selects 1P side
	0x04: selects 2P side
	*/

	m_iox.mux = data;
	m_iox.ff = 0; // this also set flip flop back to 0
}

void srmp2_state::iox_data_w(uint8_t data)
{
	m_iox.data = data;

	if(data == m_iox.reset && m_iox.reset != -1) //resets device
		m_iox.ff = 0;

	if(data == m_iox.ff_event && m_iox.ff_event != -1) // flip flop event
		m_iox.ff ^= 1;

	if(data == m_iox.ff_1 && m_iox.ff_1 != -1) // set flip flop to 1
		m_iox.ff = 1;
}

void srmp2_state::srmp3_rombank_w(uint8_t data)
{
/*
    ---- xxxx : MAIN ROM bank
    ---x ---- : unknown
    xxx- ---- : ADPCM ROM bank
*/
	m_adpcm_bank = ((data & 0xe0) >> 5);

	m_mainbank->set_entry(data & 0x0f);
}

/**************************************************************************

  Memory Map(s)

**************************************************************************/

void srmp2_state::srmp2_irq2_ack_w(uint8_t data)
{
	m_maincpu->set_input_line(2, CLEAR_LINE);
}

void srmp2_state::srmp2_irq4_ack_w(uint8_t data)
{
	m_maincpu->set_input_line(4, CLEAR_LINE);
}


void srmp2_state::srmp2_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x0c0000, 0x0c3fff).ram().share("nvram");
	map(0x140000, 0x143fff).ram().rw(m_spritegen, FUNC(x1_001_device::spritecode_r16), FUNC(x1_001_device::spritecode_w16));     /* Sprites Code + X + Attr */
	map(0x180000, 0x1805ff).ram().rw(m_spritegen, FUNC(x1_001_device::spriteylow_r16), FUNC(x1_001_device::spriteylow_w16));     /* Sprites Y */
	map(0x180600, 0x180607).ram().rw(m_spritegen, FUNC(x1_001_device::spritectrl_r16), FUNC(x1_001_device::spritectrl_w16));
	map(0x1c0000, 0x1c0001).nopw();                        /* ??? */
	map(0x800000, 0x800001).w(FUNC(srmp2_state::srmp2_flags_w));            /* ADPCM bank, Color bank, etc. */
	map(0x900000, 0x900001).portr("SYSTEM");             /* Coinage */
	map(0x900000, 0x900001).nopw();                        /* ??? */
	map(0xa00001, 0xa00001).rw(FUNC(srmp2_state::iox_mux_r), FUNC(srmp2_state::iox_command_w)); /* key matrix | I/O */
	map(0xa00003, 0xa00003).rw(FUNC(srmp2_state::iox_status_r), FUNC(srmp2_state::iox_data_w));
	map(0xb00001, 0xb00001).w(FUNC(srmp2_state::adpcm_code_w));   /* ADPCM number */
	map(0xb00003, 0xb00003).r(FUNC(srmp2_state::vox_status_r));      /* ADPCM voice status */
	map(0xc00001, 0xc00001).w(FUNC(srmp2_state::srmp2_irq2_ack_w));         /* irq ack lv 2 */
	map(0xd00001, 0xd00001).w(FUNC(srmp2_state::srmp2_irq4_ack_w));         /* irq ack lv 4 */
	map(0xe00000, 0xe00001).nopw();                        /* watchdog */
	map(0xf00001, 0xf00001).r("aysnd", FUNC(ay8910_device::data_r));
	map(0xf00000, 0xf00003).w("aysnd", FUNC(ay8910_device::address_data_w)).umask16(0x00ff);
}

uint8_t srmp2_state::mjyuugi_irq2_ack_r(address_space &space)
{
	if (!machine().side_effects_disabled())
		m_maincpu->set_input_line(2, CLEAR_LINE);
	return space.unmap(); // value returned doesn't matter
}

uint8_t srmp2_state::mjyuugi_irq4_ack_r(address_space &space)
{
	if (!machine().side_effects_disabled())
		m_maincpu->set_input_line(4, CLEAR_LINE);
	return space.unmap(); // value returned doesn't matter
}

void srmp2_state::mjyuugi_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x100001).portr("SYSTEM");             /* Coinage */
	map(0x100000, 0x100001).w(FUNC(srmp2_state::mjyuugi_flags_w));          /* Coin Counter */
	map(0x100010, 0x100011).nopr();                         /* ??? */
	map(0x100010, 0x100011).w(FUNC(srmp2_state::mjyuugi_adpcm_bank_w));     /* ADPCM bank, GFX bank */
	map(0x200001, 0x200001).r(FUNC(srmp2_state::mjyuugi_irq2_ack_r)); /* irq ack lv 2? */
	map(0x300001, 0x300001).r(FUNC(srmp2_state::mjyuugi_irq4_ack_r)); /* irq ack lv 4? */
	map(0x500000, 0x500001).portr("DSW3-1");             /* DSW 3-1 */
	map(0x500010, 0x500011).portr("DSW3-2");             /* DSW 3-2 */
	map(0x700000, 0x7003ff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0x800000, 0x800001).nopr();             /* ??? */
	map(0x900001, 0x900001).rw(FUNC(srmp2_state::iox_mux_r), FUNC(srmp2_state::iox_command_w)); /* key matrix | I/O */
	map(0x900003, 0x900003).rw(FUNC(srmp2_state::iox_status_r), FUNC(srmp2_state::iox_data_w));
	map(0xa00001, 0xa00001).w(FUNC(srmp2_state::adpcm_code_w));           /* ADPCM number */
	map(0xb00003, 0xb00003).r(FUNC(srmp2_state::vox_status_r));      /* ADPCM voice status */
	map(0xb00001, 0xb00001).r("aysnd", FUNC(ay8910_device::data_r));
	map(0xb00000, 0xb00003).w("aysnd", FUNC(ay8910_device::address_data_w)).umask16(0x00ff);
	map(0xc00000, 0xc00001).nopw();                    /* ??? */
	map(0xd00000, 0xd005ff).ram().rw(m_spritegen, FUNC(x1_001_device::spriteylow_r16), FUNC(x1_001_device::spriteylow_w16)); /* Sprites Y */
	map(0xd00600, 0xd00607).ram().rw(m_spritegen, FUNC(x1_001_device::spritectrl_r16), FUNC(x1_001_device::spritectrl_w16));

	map(0xd02000, 0xd023ff).ram();                         /* ??? only writes $00fa */
	map(0xe00000, 0xe03fff).ram().rw(m_spritegen, FUNC(x1_001_device::spritecode_r16), FUNC(x1_001_device::spritecode_w16)); /* Sprites Code + X + Attr */
	map(0xffc000, 0xffffff).ram().share("nvram");
}

void srmp2_state::srmp3_flags_w(uint8_t data)
{
/*
    ---- ---x : Coin Counter
    ---x ---- : Coin Lock Out
    xx-- ---- : GFX Bank
*/

	machine().bookkeeping().coin_counter_w(0, BIT(data, 0) );
	machine().bookkeeping().coin_lockout_w(0, BIT(~data, 4) );
	m_gfx_bank = (data >> 6) & 0x03;
}

void srmp2_state::srmp3_irq_ack_w(uint8_t data)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
}

void srmp2_state::srmp3_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x9fff).bankr("mainbank");                            /* rom bank */
	map(0xa000, 0xa7ff).ram().share("nvram");   /* work ram */
	map(0xa800, 0xa800).nopw();                            /* flag ? */
	map(0xb000, 0xb2ff).ram().rw(m_spritegen, FUNC(x1_001_device::spriteylow_r8), FUNC(x1_001_device::spriteylow_w8));
	map(0xb300, 0xb303).ram().rw(m_spritegen, FUNC(x1_001_device::spritectrl_r8), FUNC(x1_001_device::spritectrl_w8));
	map(0xb800, 0xb800).nopw();                            /* flag ? */
	map(0xc000, 0xdfff).ram().rw(m_spritegen, FUNC(x1_001_device::spritecodelow_r8), FUNC(x1_001_device::spritecodelow_w8)); /* Sprites Code + X + Attr */
	map(0xe000, 0xffff).ram().rw(m_spritegen, FUNC(x1_001_device::spritecodehigh_r8), FUNC(x1_001_device::spritecodehigh_w8));
}

void srmp2_state::srmp3_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x20, 0x20).w(FUNC(srmp2_state::srmp3_irq_ack_w));                              /* interrupt acknowledge */
	map(0x40, 0x40).portr("SYSTEM").w(FUNC(srmp2_state::srmp3_flags_w));         /* coin, service | GFX bank, counter, lockout */
	map(0x60, 0x60).w(FUNC(srmp2_state::srmp3_rombank_w));                              /* ROM bank select */
	map(0xa0, 0xa0).w(FUNC(srmp2_state::adpcm_code_w));                   /* ADPCM number */
	map(0xa1, 0xa1).r(FUNC(srmp2_state::vox_status_r));                                  /* ADPCM voice status */
	map(0xc0, 0xc0).rw(FUNC(srmp2_state::iox_mux_r), FUNC(srmp2_state::iox_command_w));                 /* key matrix | I/O */
	map(0xc1, 0xc1).rw(FUNC(srmp2_state::iox_status_r), FUNC(srmp2_state::iox_data_w));
	map(0xe0, 0xe1).w("aysnd", FUNC(ay8910_device::address_data_w));
	map(0xe2, 0xe2).r("aysnd", FUNC(ay8910_device::data_r));
}

void srmp2_state::rmgoldyh_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x9fff).bankr("mainbank");                            /* rom bank */
	map(0xa000, 0xafff).ram().share("nvram");   /* work ram */
	map(0xb000, 0xb2ff).ram().rw(m_spritegen, FUNC(x1_001_device::spriteylow_r8), FUNC(x1_001_device::spriteylow_w8));
	map(0xb300, 0xb303).ram().rw(m_spritegen, FUNC(x1_001_device::spritectrl_r8), FUNC(x1_001_device::spritectrl_w8));
	map(0xb800, 0xb800).nopw();                            /* flag ? */
	map(0xc000, 0xdfff).ram().rw(m_spritegen, FUNC(x1_001_device::spritecodelow_r8), FUNC(x1_001_device::spritecodelow_w8)); /* Sprites Code + X + Attr */
	map(0xe000, 0xffff).ram().rw(m_spritegen, FUNC(x1_001_device::spritecodehigh_r8), FUNC(x1_001_device::spritecodehigh_w8));
}

void srmp2_state::rmgoldyh_rombank_w(uint8_t data)
{
/*
    ---x xxxx : MAIN ROM bank
    xxx- ---- : ADPCM ROM bank
*/
	m_adpcm_bank = ((data & 0xe0) >> 5);

	m_mainbank->set_entry(data & 0x1f);
}

void srmp2_state::rmgoldyh_io_map(address_map &map)
{
	map.global_mask(0xff);
	srmp3_io_map(map);
	map(0x00, 0x00).nopw(); /* watchdog */
	map(0x60, 0x60).w(FUNC(srmp2_state::rmgoldyh_rombank_w));                       /* ROM bank select */
	map(0x80, 0x80).portr("DSW4");
	map(0x81, 0x81).portr("DSW3");
}

/***************************************************************************

  Input Port(s)

***************************************************************************/

static INPUT_PORTS_START( seta_mjctrl )
	PORT_START("KEY0")  /* KEY MATRIX INPUT (3) */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_BIG ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY1")  /* KEY MATRIX INPUT (4) */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY2")  /* KEY MATRIX INPUT (5) */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(1)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY3")  /* KEY MATRIX INPUT (6) */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY4")  /* KEY MATRIX INPUT (3) */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_BIG ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY5")  /* KEY MATRIX INPUT (4) */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY6")  /* KEY MATRIX INPUT (5) */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(2)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY7")  /* KEY MATRIX INPUT (6) */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( srmp2 )
	PORT_START("SYSTEM")            /* Coinage (0) */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
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
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0xe0, "1 (Easy)" )
	PORT_DIPSETTING(    0xc0, "2" )
	PORT_DIPSETTING(    0xa0, "3" )
	PORT_DIPSETTING(    0x80, "4" )
	PORT_DIPSETTING(    0x60, "5" )
	PORT_DIPSETTING(    0x40, "6" )
	PORT_DIPSETTING(    0x20, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hard)" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x02, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_INCLUDE( seta_mjctrl )

	PORT_START("SERVICE")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( srmp3 )
	PORT_START("SYSTEM")            /* Coinage (0) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Debug Mode (Cheat)")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Open Reach of CPU" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0xe0, "1 (Easy)" )
	PORT_DIPSETTING(    0xc0, "2" )
	PORT_DIPSETTING(    0xa0, "3" )
	PORT_DIPSETTING(    0x80, "4" )
	PORT_DIPSETTING(    0x60, "5" )
	PORT_DIPSETTING(    0x40, "6" )
	PORT_DIPSETTING(    0x20, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hard)" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x02, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_INCLUDE( seta_mjctrl )

	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MEMORY_RESET )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Analyzer")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( rmgoldyh )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SERVICE")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("Payout")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MEMORY_RESET )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Test Mode")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Analyzer")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_INCLUDE( seta_mjctrl )

	// DIP switch sheets available at MameTesters (MT05599)
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, "Distribution list" ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, "32:24:16:12:8:4:2:1" )
	PORT_DIPSETTING(    0x02, "200:100:50:10:5:3:2:1" )
	PORT_DIPSETTING(    0x01, "100:50:25:10:5:3:2:1" )
	PORT_DIPSETTING(    0x00, "50:30:15:8:5:3:2:1" )
	PORT_DIPNAME( 0x0c, 0x04, "Max Bet" ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x04, "10" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0xf0, 0x40, "Payout percentage" ) PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(    0x00, "96%" )
	PORT_DIPSETTING(    0x10, "93%" )
	PORT_DIPSETTING(    0x20, "90%" )
	PORT_DIPSETTING(    0x30, "87%" )
	PORT_DIPSETTING(    0x40, "84%" )
	PORT_DIPSETTING(    0x50, "81%" )
	PORT_DIPSETTING(    0x60, "78%" )
	PORT_DIPSETTING(    0x70, "75%" )
	PORT_DIPSETTING(    0x80, "71%" )
	PORT_DIPSETTING(    0x90, "68%" )
	PORT_DIPSETTING(    0xa0, "65%" )
	PORT_DIPSETTING(    0xb0, "62%" )
	PORT_DIPSETTING(    0xc0, "59%" )
	PORT_DIPSETTING(    0xd0, "56%" )
	PORT_DIPSETTING(    0xe0, "53%" )
	PORT_DIPSETTING(    0xf0, "50%" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x0c, 0x0c, "Min Rate To Play" ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPNAME( 0x80, 0x80, "Credits Per Note" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, "5" )
	PORT_DIPSETTING(    0x00, "10" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "Coin Out Logic" ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, "Positive" )
	PORT_DIPSETTING(    0x00, "Negative" )
	PORT_DIPNAME( 0x02, 0x02, "Magic Switch" ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, "A SW" )
	PORT_DIPSETTING(    0x00, "F/F SW" )
	PORT_DIPNAME( 0x04, 0x04, "Service Count" ) PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPNAME( 0x10, 0x10, "Yakuman Bonus Times" ) PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x10, "Twice" )
	PORT_DIPSETTING(    0x00, "Once" )
	PORT_DIPNAME( 0xe0, 0xa0, "Yakuman Bonus Frequency" ) PORT_DIPLOCATION("SW3:6,7,8")
	PORT_DIPSETTING(    0xe0, DEF_STR( None ) )
	PORT_DIPSETTING(    0x60, "Only Once" )
	PORT_DIPSETTING(    0xa0, "Every 300 Coins" )
	PORT_DIPSETTING(    0x00, "Every 500 Coins" )
	PORT_DIPSETTING(    0xc0, "Every 700 Coins" )
	PORT_DIPSETTING(    0x40, "Every 1000 Coins" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x07, 0x01, "Payout Method" ) PORT_DIPLOCATION("SW4:1,2,3")
	PORT_DIPSETTING(    0x06, "Direct Pay" )
	PORT_DIPSETTING(    0x02, "Pool Out" )
	PORT_DIPSETTING(    0x01, "Pay Off" )
	PORT_DIPNAME( 0x08, 0x08, "Game Title" ) PORT_DIPLOCATION("SW4:4") // Marked as unused on dip sheet
	PORT_DIPSETTING(    0x08, "Real Mahjong Gold Yumehai" )
	PORT_DIPSETTING(    0x00, "Super Real Mahjong GOLD part.2" )
	PORT_DIPNAME( 0x10, 0x00, "Double Bet" ) PORT_DIPLOCATION("SW4:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Auto Tsumo" ) PORT_DIPLOCATION("SW4:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Renchan Rate" ) PORT_DIPLOCATION("SW4:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Last Chance Charge" ) PORT_DIPLOCATION("SW4:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( mjyuugi )
	PORT_START("SYSTEM")            /* Coinage (0) */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x07, "1 (Easy)" )
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x05, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x06, "5" )
	PORT_DIPSETTING(    0x04, "6" )
	PORT_DIPSETTING(    0x08, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hard)" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, "Gal Score" )
	PORT_DIPSETTING(    0x10, "+0" )
	PORT_DIPSETTING(    0x00, "+1000" )
	PORT_DIPNAME( 0x20, 0x20, "Player Score" )
	PORT_DIPSETTING(    0x20, "+0" )
	PORT_DIPSETTING(    0x00, "+1000" )
	PORT_DIPNAME( 0x40, 0x40, "Item price initialize ?" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) )
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_INCLUDE( seta_mjctrl )

	PORT_START("SERVICE")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_DIPNAME( 0x0004, 0x0004, "Debug Mode (Cheat)")
	PORT_DIPSETTING(   0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW3-1")            /* [Debug switch] */
	PORT_DIPNAME( 0x0001, 0x0001, "Debug  0 (Cheat)")
	PORT_DIPSETTING(   0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "Debug  1 (Cheat)")
	PORT_DIPSETTING(   0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Debug  2 (Cheat)")
	PORT_DIPSETTING(   0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Debug  3 (Cheat)")
	PORT_DIPSETTING(   0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0000, DEF_STR( On ) )
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW3-2")            /* [Debug switch] */
	PORT_DIPNAME( 0x0001, 0x0001, "Debug  4 (Cheat)")
	PORT_DIPSETTING(   0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "Debug  5 (Cheat)")
	PORT_DIPSETTING(   0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Debug  6 (Cheat)")
	PORT_DIPSETTING(   0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Debug  7 (Cheat)")
	PORT_DIPSETTING(   0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0000, DEF_STR( On ) )
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( ponchin )
	PORT_START("SYSTEM")            /* Coinage (0) */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x06, "1 (Easy)" )
	PORT_DIPSETTING(    0x05, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x07, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hard)" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x30, 0x30, "Player Score" )
	PORT_DIPSETTING(    0x30, "1000" )
	PORT_DIPSETTING(    0x20, "2000" )
//  PORT_DIPSETTING(    0x10, "1000" )
	PORT_DIPSETTING(    0x00, "3000" )
	PORT_DIPNAME( 0x40, 0x40, "Debug Mode (Cheat)")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Auto Tsumo" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_INCLUDE( seta_mjctrl )

	PORT_START("SERVICE")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW3-1")            /* [Debug switch] */
	PORT_DIPNAME( 0x0001, 0x0001, "Debug  0 (Cheat)")
	PORT_DIPSETTING(   0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "Debug  1 (Cheat)")
	PORT_DIPSETTING(   0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Debug  2 (Cheat)")
	PORT_DIPSETTING(   0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Debug  3 (Cheat)")
	PORT_DIPSETTING(   0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0000, DEF_STR( On ) )
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW3-2")            /* [Debug switch] */
	PORT_DIPNAME( 0x0001, 0x0001, "Debug  4 (Cheat)")
	PORT_DIPSETTING(   0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "Debug  5 (Cheat)")
	PORT_DIPSETTING(   0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Debug  6 (Cheat)")
	PORT_DIPSETTING(   0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Debug  7 (Cheat)")
	PORT_DIPSETTING(   0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0000, DEF_STR( On ) )
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************

  Machine Driver(s)

***************************************************************************/

static const gfx_layout charlayout =
{
	16, 16,
	RGN_FRAC(1, 2),
	4,
	{ RGN_FRAC(1, 2) + 8, RGN_FRAC(1, 2) + 0, 8, 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7, 128, 129, 130, 131, 132, 133, 134, 135 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
		16*16, 17*16, 18*16, 19*16, 20*16, 21*16, 22*16, 23*16 },
	16*16*2
};

static GFXDECODE_START( gfx_srmp2 )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 64 )
GFXDECODE_END

static GFXDECODE_START( gfx_srmp3 )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 32 )
GFXDECODE_END


void srmp2_state::srmp2(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 8_MHz_XTAL);  /* 8.00 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &srmp2_state::srmp2_map);
	m_maincpu->set_vblank_int("screen", FUNC(srmp2_state::irq4_line_assert));
	m_maincpu->set_periodic_int(FUNC(srmp2_state::irq2_line_assert), attotime::from_hz(15*60)); /* Interrupt times is not understood */

	MCFG_MACHINE_START_OVERRIDE(srmp2_state,srmp2)
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	X1_001(config, m_spritegen, 20_MHz_XTAL, "palette", gfx_srmp2);
	m_spritegen->set_transpen(15);
	m_spritegen->set_fg_xoffsets( 0x10, 0x10 );
	m_spritegen->set_fg_yoffsets( 0x05, 0x07 );
	m_spritegen->set_bg_xoffsets( 0x00, 0x00 ); // bg not used?
	m_spritegen->set_bg_yoffsets( 0x00, 0x00 ); // bg not used?

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(464, 256-16);
	screen.set_visarea(16, 464-1, 8, 256-1-24);
	screen.set_screen_update(FUNC(srmp2_state::screen_update_srmp2));
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(srmp2_state::srmp2_palette)).set_format(palette_device::xRGB_555, 1024); // sprites only

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ay8910_device &aysnd(AY8910(config, "aysnd", 20_MHz_XTAL / 16));
	aysnd.port_a_read_callback().set_ioport("DSW2");
	aysnd.port_b_read_callback().set_ioport("DSW1");
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.40);

	MSM5205(config, m_msm, 384000);
	m_msm->vck_legacy_callback().set(FUNC(srmp2_state::adpcm_int)); /* IRQ handler */
	m_msm->set_prescaler_selector(msm5205_device::S48_4B);  /* 8 KHz, 4 Bits  */
	m_msm->add_route(ALL_OUTPUTS, "mono", 0.45);
}

void srmp2_state::srmp3(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 3500000);    /* 3.50 MHz? or 4.00 MHz? */
	m_maincpu->set_addrmap(AS_PROGRAM, &srmp2_state::srmp3_map);
	m_maincpu->set_addrmap(AS_IO, &srmp2_state::srmp3_io_map);
	m_maincpu->set_vblank_int("screen", FUNC(srmp2_state::irq0_line_assert));

	MCFG_MACHINE_START_OVERRIDE(srmp2_state,srmp3)
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	X1_001(config, m_spritegen, 16000000, "palette", gfx_srmp3);
	m_spritegen->set_gfxbank_callback(FUNC(srmp2_state::srmp3_gfxbank_callback));
	m_spritegen->set_fg_xoffsets( 0x10, 0x10 );
	m_spritegen->set_fg_yoffsets( 0x06, 0x06 );
	m_spritegen->set_bg_xoffsets( -0x01, 0x10 );
	m_spritegen->set_bg_yoffsets( -0x06, 0x06 );

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(400, 256-16);
	screen.set_visarea(16, 400-1, 8, 256-1-24);
	screen.set_screen_update(FUNC(srmp2_state::screen_update_srmp3));
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(srmp2_state::srmp3_palette)).set_format(palette_device::xRGB_555, 512); // sprites only

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ay8910_device &aysnd(AY8910(config, "aysnd", 16000000/16));
	aysnd.port_a_read_callback().set_ioport("DSW2");
	aysnd.port_b_read_callback().set_ioport("DSW1");
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.20);

	MSM5205(config, m_msm, 384000);
	m_msm->vck_legacy_callback().set(FUNC(srmp2_state::adpcm_int)); /* IRQ handler */
	m_msm->set_prescaler_selector(msm5205_device::S48_4B);  /* 8 KHz, 4 Bits */
	m_msm->add_route(ALL_OUTPUTS, "mono", 0.45);
}

void srmp2_state::rmgoldyh(machine_config &config)
{
	srmp3(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &srmp2_state::rmgoldyh_map);
	m_maincpu->set_addrmap(AS_IO, &srmp2_state::rmgoldyh_io_map);

	MCFG_MACHINE_START_OVERRIDE(srmp2_state,rmgoldyh)
}

void srmp2_state::mjyuugi(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 16_MHz_XTAL / 2);  /* 8.00 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &srmp2_state::mjyuugi_map);
	m_maincpu->set_vblank_int("screen", FUNC(srmp2_state::irq4_line_assert));
	m_maincpu->set_periodic_int(FUNC(srmp2_state::irq2_line_assert), attotime::from_hz(15*60)); /* Interrupt times is not understood */

	MCFG_MACHINE_START_OVERRIDE(srmp2_state,mjyuugi)

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	X1_001(config, m_spritegen, 16_MHz_XTAL, "palette", gfx_srmp3);
	m_spritegen->set_gfxbank_callback(FUNC(srmp2_state::srmp3_gfxbank_callback));
	m_spritegen->set_fg_xoffsets( 0x10, 0x10 );
	m_spritegen->set_fg_yoffsets( 0x06, 0x06 );
	m_spritegen->set_bg_yoffsets( 0x09, 0x07 );
	m_spritegen->set_spritelimit( 0x1ff-6 );

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(400, 256-16);
	screen.set_visarea(16, 400-1, 0, 256-1-16);
	screen.set_screen_update(FUNC(srmp2_state::screen_update_srmp3));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::xRGB_555, 512); // sprites only

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ay8910_device &aysnd(AY8910(config, "aysnd", 16_MHz_XTAL / 16));
	aysnd.port_a_read_callback().set_ioport("DSW2");
	aysnd.port_b_read_callback().set_ioport("DSW1");
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.20);

	MSM5205(config, m_msm, 384000);
	m_msm->vck_legacy_callback().set(FUNC(srmp2_state::adpcm_int)); /* IRQ handler */
	m_msm->set_prescaler_selector(msm5205_device::S48_4B);  /* 8 KHz, 4 Bits */
	m_msm->add_route(ALL_OUTPUTS, "mono", 0.45);
}



/***************************************************************************

  Game driver(s)

***************************************************************************/

/***************************************************************************

Super Real Mahjong PI
(c)1987 Seta
P0-023
M6100242A
860161517

CPU: 68000
Sound: AY-3-8910, M5205
OSC: 8.000MHz(X1) 20.000MHz(X2)
Custom chips: X1-003, X2-003(x3), X1-004(x2), X0-005
plus 2 chips covered with heatsink
Others: Battery, D8243HC

UB0-1.19     [a5960661] 831000
UB0-2.17     [b88dbeba]  |
UB0-3.18     [6302cfa8] /

UB0-4.60     [cb6f7cce]
UB0-5.61     [7b48c540]
UB0-6.62     [6c891ac5]
UB0-7.63     [60a45755]

UB0-8.64     [b58024b9] MB83256
UB0-9.65     [e28c2566]  |
UB0-10.66    [d20a935c]  |
UB0-11.67    [30957ca8] /

UB1.12       [d2ed93c8] MB7124E
UB2.13       [7e7d25f7] /

UB-3 (PAL16L8A - not dumped)

***************************************************************************/

ROM_START( srmp1 )
	ROM_REGION( 0x40000, "maincpu", 0 )                 /* 68000 Code */
	ROM_LOAD16_BYTE( "ub0-2.17", 0x000000, 0x20000, CRC(71a00a3d) SHA1(8deb07a4621e0f0f1d6dd503cd7f4f826a63c255) )
	ROM_LOAD16_BYTE( "ub0-3.18", 0x000001, 0x20000, CRC(2950841b) SHA1(1859636602375b4cadbd23457a0d16bc85063ff5) )

	ROM_REGION( 0x800, "mcu1", 0 )
	ROM_LOAD( "x0-005_117.bin", 0x000, 0x800, NO_DUMP )

	ROM_REGION( 0x800, "mcu2", 0 )
	ROM_COPY( "mcu1", 0x000, 0x000, 0x800 )

	ROM_REGION( 0x200000, "gfx1", 0 )   /* Sprites */
	ROM_LOAD16_BYTE( "ub0-8.64",  0x000000, 0x008000, CRC(b58024b9) SHA1(d38750fa90c1b14288884a3c2713b90f0b941bfb) )
	ROM_LOAD16_BYTE( "ub0-9.65",  0x000001, 0x008000, CRC(e28c2566) SHA1(91ce9d7138e05f9bcd31dfeaaa558d4431ae9515) )
	ROM_LOAD       ( "ubo-5.61",  0x040000, 0x040000, CRC(7b48c540) SHA1(06229caec6846581f95409204ea22f0f76684b08) )
	ROM_LOAD       ( "ubo-4.60",  0x080000, 0x040000, CRC(cb6f7cce) SHA1(27d7c2f4f998023081fac1bbacfd4b0b56edaee2) )
	ROM_LOAD16_BYTE( "ub0-10.66", 0x100000, 0x008000, CRC(d20a935c) SHA1(1d01b0ccfb6c2ea226e16e555796acd10b10e835) )
	ROM_LOAD16_BYTE( "ub0-11.67", 0x100001, 0x008000, CRC(30957ca8) SHA1(02aaa3f2f266e8f4db2d35c177546365d7836004) )
	ROM_LOAD       ( "ubo-7.63",  0x140000, 0x040000, CRC(60a45755) SHA1(22bbf024bbe2186b621389a23697e55d512b501a) )
	ROM_LOAD       ( "ubo-6.62",  0x180000, 0x040000, CRC(6c891ac5) SHA1(eab595bce16e4cdc465a5e2e029c3949a0f28629) )

	ROM_REGION( 0x020000, "adpcm", 0 )              /* Samples */
	ROM_LOAD( "ub0-1.19", 0x000000, 0x20000, CRC(5f21b48c) SHA1(1838632609c176dbaab1d88f9368c03259a5e954) )

	ROM_REGION( 0x800, "proms", 0 )                 /* Color PROMs */
	ROM_LOAD( "ub1.12", 0x000, 0x200, CRC(d2ed93c8) SHA1(334d4fe6fa477758b336b138ffc306e2d6334371) )
	ROM_LOAD( "ub2.13", 0x400, 0x200, CRC(7e7d25f7) SHA1(e5bf5071567f95c3bb70347f0b86a9703f9f2e6c) )
ROM_END

ROM_START( srmp2 ) // PCB: P0-023
	ROM_REGION( 0x040000, "maincpu", 0 )                    /* 68000 Code */
	ROM_LOAD16_BYTE( "uco-2.17", 0x000000, 0x020000, CRC(0d6c131f) SHA1(be85f2578b0ae2a072565605b7dbeb970e5e3851) )
	ROM_LOAD16_BYTE( "uco-3.18", 0x000001, 0x020000, CRC(e9fdf5f8) SHA1(aa1f8cc3f1d0ed942403c0473605775bc1537cbf) )

	ROM_REGION( 0x800, "mcu1", 0 )
	ROM_LOAD( "x0-005_117.bin", 0x000, 0x800, NO_DUMP )

	ROM_REGION( 0x800, "mcu2", 0 )
	ROM_COPY( "mcu1", 0x000, 0x000, 0x800 )

	ROM_REGION( 0x200000, "gfx1", 0 )   /* Sprites */
	ROM_LOAD       ( "ubo-4.60",  0x000000, 0x040000, CRC(cb6f7cce) SHA1(27d7c2f4f998023081fac1bbacfd4b0b56edaee2) )
	ROM_LOAD       ( "ubo-5.61",  0x040000, 0x040000, CRC(7b48c540) SHA1(06229caec6846581f95409204ea22f0f76684b08) )
	ROM_LOAD16_BYTE( "uco-8.64",  0x080000, 0x040000, CRC(1ca1c7c9) SHA1(05bcca1f88d976d836944a7f5cc74a857fdf6cb9) )
	ROM_LOAD16_BYTE( "uco-9.65",  0x080001, 0x040000, CRC(ef75471b) SHA1(b9843559d36e071bc7d8d81eef44424c4566a10e) )
	ROM_LOAD       ( "ubo-6.62",  0x100000, 0x040000, CRC(6c891ac5) SHA1(eab595bce16e4cdc465a5e2e029c3949a0f28629) )
	ROM_LOAD       ( "ubo-7.63",  0x140000, 0x040000, CRC(60a45755) SHA1(22bbf024bbe2186b621389a23697e55d512b501a) )
	ROM_LOAD16_BYTE( "uco-10.66", 0x180000, 0x040000, CRC(cb6bd857) SHA1(1bd673e10416bc3ca14859cc15cd05caa7d7a625) )
	ROM_LOAD16_BYTE( "uco-11.67", 0x180001, 0x040000, CRC(199f79c0) SHA1(46f437e90ee25c242bf418c0fa1af77d6e4cafc6) )

	ROM_REGION( 0x020000, "adpcm", 0 )              /* Samples */
	ROM_LOAD( "uco-1.19", 0x000000, 0x020000, CRC(f284af8e) SHA1(f0b5ef8ae98101bf8c8885e469a5a36dd5e29129) )

	ROM_REGION( 0x000800, "proms", 0 )                  /* Color PROMs */
	ROM_LOAD( "uc-1o.12", 0x000000, 0x000400, CRC(fa59b5cb) SHA1(171c4c36bd1c8e6548b34a9f6e2ff755ecf09b47) )
	ROM_LOAD( "uc-2o.13", 0x000400, 0x000400, CRC(50a33b96) SHA1(cfb6d3cb6b73d1bf484014fb340c28bc1774137d) )
ROM_END

ROM_START( srmp3 ) // PCB: P0-037A
	ROM_REGION( 0x020000, "maincpu", 0 )                    /* Z80 Code */
	ROM_LOAD( "za0-10.bin", 0x000000, 0x020000, CRC(939d126f) SHA1(7a5c7f7fbee8de11a08194d3c8f10a20f8dc2f0a) )

	ROM_REGION( 0x800, "mcu1", 0 )
	ROM_LOAD( "x0-005_117.bin", 0x000, 0x800, NO_DUMP )

	ROM_REGION( 0x800, "mcu2", 0 )
	ROM_COPY( "mcu1", 0x000, 0x000, 0x800 )

	ROM_REGION( 0x400000, "gfx1", 0 )   /* Sprites */
	ROM_LOAD16_BYTE( "za0-02.bin", 0x000000, 0x080000, CRC(85691946) SHA1(8b91210b1b6671ba2c9ec6722e5dc40bdf44e4b5) )
	ROM_LOAD16_BYTE( "za0-04.bin", 0x000001, 0x080000, CRC(c06e7a96) SHA1(a2dfb81004ea72bfa21724374eb8533af606a5df) )
	ROM_LOAD16_BYTE( "za0-01.bin", 0x100000, 0x080000, CRC(95e0d87c) SHA1(34e6c0a95e63cf092092e27c7ba2f649ebf56507) )
	ROM_LOAD16_BYTE( "za0-03.bin", 0x100001, 0x080000, CRC(7c98570e) SHA1(26e28e67bca9954d62d72260370ea872c6058a10) )
	ROM_LOAD16_BYTE( "za0-06.bin", 0x200000, 0x080000, CRC(8b874b0a) SHA1(27fe1ccc2938e1703e484e2925a2f073064cf019) )
	ROM_LOAD16_BYTE( "za0-08.bin", 0x200001, 0x080000, CRC(3de89d88) SHA1(1e6dabe6aeee6a2613feab26b871c235bf491bfa) )
	ROM_LOAD16_BYTE( "za0-05.bin", 0x300000, 0x080000, CRC(80d3b4e6) SHA1(d31d3f904ee8463c1efbb1d106eeb3dc0dc42ab8) )
	ROM_LOAD16_BYTE( "za0-07.bin", 0x300001, 0x080000, CRC(39d15129) SHA1(62b71a82cfc39e6dab3175e03eca5ff92e854f13) )

	ROM_REGION( 0x080000, "adpcm", 0 )              /* Samples */
	ROM_LOAD( "za0-11.bin", 0x000000, 0x080000, CRC(2248c23f) SHA1(35591b51bb23dfd7fa81a05026e9ec0789bb0dde) )

	ROM_REGION( 0x000400, "proms", 0 )                  /* Color PROMs */
	ROM_LOAD( "za0-12.prm", 0x000000, 0x000200, CRC(1ac5387c) SHA1(022f204dbe2374478279b586451673a08ee489c8) )
	ROM_LOAD( "za0-13.prm", 0x000200, 0x000200, CRC(4ea3d2fe) SHA1(c7d18b9c1331e08faadf33e52033c658bf2b16fc) )
ROM_END

/***************************************************************************

Real Mahjong Gold Yumehai
(c)1988? Alba
The game is BET version of Super Real Mahjong P3,
but the PCB is totally different.

P0-064A

CPU: Z80
Sound: AY-3-8910, M5205
OSC: 16.000MHz (X1), 10.000MHz (X3)

Custom chips:
X1-001
X1-002
X2-003 x3
X1-004 x2
X0-005
X1-007

ROMs:
ZF0_001_001.U2 [ce5b0ba0] \
ZF0_002_002.U3 [e2226425] -- Main programs

ZF0_1.U70    [78ba5d05] \
ZF0_2.U71    [b0f548e6] |-- tiles?
ZF0_3.U72    [771c27a1] /

ZA0-1.U52 \
ZA0-2.U51 |
ZA0-3.U50 |
ZA0-4.U49 |
ZA0-5.U48 |- Sprites (same as srmp3)
ZA0-6.U47 |
ZA0-7.U46 |
ZA0-8.U45 /

ZA0-11.U16 - Samples (same as srmp3)

ZF0-12.U58   [1ac5387c] \
ZF0-13.U59   [4ea3d2fe] -- Color PROM

ZF0-5.U35 - PAL16L8A-2CN(not dumped)

Others:
CR2032 battery
***************************************************************************/

ROM_START( rmgoldyh )
	ROM_REGION( 0x040000, "maincpu", 0 )                    /* Z80 Code */
	ROM_LOAD( "zf0_001_001.u2", 0x000000, 0x020000, CRC(ce5b0ba0) SHA1(c499e7dc0e3ffe783204e930356c91ea228baf62) )
	ROM_LOAD( "zf0_002_002.u3", 0x020000, 0x020000, CRC(e2226425) SHA1(36925c68492a3ea4af19d611a455eae688aaab62) )

	ROM_REGION( 0x800, "mcu1", 0 )
	ROM_LOAD( "x0-005_117.bin", 0x000, 0x800, NO_DUMP )

	ROM_REGION( 0x800, "mcu2", 0 )
	ROM_COPY( "mcu1", 0x000, 0x000, 0x800 )

	ROM_REGION( 0x800000, "gfx1", ROMREGION_ERASE00 )   /* Sprites */
	ROM_LOAD16_BYTE( "za0-02.u51", 0x000000, 0x080000, CRC(85691946) SHA1(8b91210b1b6671ba2c9ec6722e5dc40bdf44e4b5) )
	ROM_LOAD16_BYTE( "za0-04.u49", 0x000001, 0x080000, CRC(c06e7a96) SHA1(a2dfb81004ea72bfa21724374eb8533af606a5df) )
	ROM_LOAD16_BYTE( "za0-01.u52", 0x100000, 0x080000, CRC(95e0d87c) SHA1(34e6c0a95e63cf092092e27c7ba2f649ebf56507) )
	ROM_LOAD16_BYTE( "za0-03.u50", 0x100001, 0x080000, CRC(7c98570e) SHA1(26e28e67bca9954d62d72260370ea872c6058a10) )
	/* socket 4 is empty */
	ROM_LOAD16_BYTE( "zf0_3.u72",  0x200001, 0x008000, CRC(771c27a1) SHA1(5c95edcd5e155cbb4448888bba62c98cf8d4b577) )
	ROM_LOAD16_BYTE( "za0-06.u47", 0x400000, 0x080000, CRC(8b874b0a) SHA1(27fe1ccc2938e1703e484e2925a2f073064cf019) )
	ROM_LOAD16_BYTE( "za0-08.u45", 0x400001, 0x080000, CRC(3de89d88) SHA1(1e6dabe6aeee6a2613feab26b871c235bf491bfa) )
	ROM_LOAD16_BYTE( "za0-05.u48", 0x500000, 0x080000, CRC(80d3b4e6) SHA1(d31d3f904ee8463c1efbb1d106eeb3dc0dc42ab8) )
	ROM_LOAD16_BYTE( "za0-07.u46", 0x500001, 0x080000, CRC(39d15129) SHA1(62b71a82cfc39e6dab3175e03eca5ff92e854f13) )
	ROM_LOAD16_BYTE( "zf0_2.u71",  0x600000, 0x008000, CRC(b0f548e6) SHA1(84e3acb10ae3669bf65bd8c93273acacb5136737) )
	ROM_LOAD16_BYTE( "zf0_1.u70",  0x600001, 0x008000, CRC(78ba5d05) SHA1(21cd5ecbd55a5beaece82c974752dac4281b467a) )

	ROM_REGION( 0x080000, "adpcm", 0 )              /* Samples */
	ROM_LOAD( "za0-11.u16", 0x000000, 0x080000, CRC(2248c23f) SHA1(35591b51bb23dfd7fa81a05026e9ec0789bb0dde) )

	ROM_REGION( 0x000400, "proms", 0 )                  /* Color PROMs */
	ROM_LOAD( "zf0-12.u58", 0x000000, 0x000200, CRC(1ac5387c) SHA1(022f204dbe2374478279b586451673a08ee489c8) )
	ROM_LOAD( "zf0-13.u59", 0x000200, 0x000200, CRC(4ea3d2fe) SHA1(c7d18b9c1331e08faadf33e52033c658bf2b16fc) )
ROM_END

ROM_START( mjyuugi ) // PCB: P0-049A
	ROM_REGION( 0x080000, "maincpu", 0 )                    /* 68000 Code */
	ROM_LOAD16_BYTE( "um001.001", 0x000000, 0x020000, CRC(28d5340f) SHA1(683d89987b8b794695fdb6104d8e6ff5204afafb) )
	ROM_LOAD16_BYTE( "um001.003", 0x000001, 0x020000, CRC(275197de) SHA1(2f8efa112f23f172eaef9bb732b2a253307dd896) )
	ROM_LOAD16_BYTE( "um001.002", 0x040000, 0x020000, CRC(d5dd4710) SHA1(b70c280f828af507c73ebec3209043eb7ce0ce95) )
	ROM_LOAD16_BYTE( "um001.004", 0x040001, 0x020000, CRC(c5ddb567) SHA1(1a35228439108f3d866547d94d4bafca54a710ec) )

	ROM_REGION( 0x800, "mcu1", 0 )
	ROM_LOAD( "x0-005_117.bin", 0x000, 0x800, NO_DUMP )

	ROM_REGION( 0x800, "mcu2", 0 )
	ROM_COPY( "mcu1", 0x000, 0x000, 0x800 )

	ROM_REGION( 0x400000, "gfx1", 0 )   /* Sprites */
	ROM_LOAD16_BYTE( "maj-001.10",  0x000000, 0x080000, CRC(3c08942a) SHA1(59165052d7c760ac82157844d54c8dced4125259) )
	ROM_LOAD16_BYTE( "maj-001.08",  0x000001, 0x080000, CRC(e2444311) SHA1(88673d57d54ef0674c8c23a95da5d03cb9c894aa) )
	ROM_LOAD16_BYTE( "maj-001.09",  0x100000, 0x080000, CRC(a1974860) SHA1(f944026cf1aadb9c24ac689cc67d374eea17cb85) )
	ROM_LOAD16_BYTE( "maj-001.07",  0x100001, 0x080000, CRC(b1f1d118) SHA1(37d64ba662b431cf0fdee12983c95b9989eb00af) )
	ROM_LOAD16_BYTE( "maj-001.06",  0x200000, 0x080000, CRC(4c60acdd) SHA1(0ab69cc3ea4bebd9e7b139c89b5ac42a621493e2) )
	ROM_LOAD16_BYTE( "maj-001.04",  0x200001, 0x080000, CRC(0a4b2de1) SHA1(f9cddeffcdceb06053216502eb03d52abf527eb2) )
	ROM_LOAD16_BYTE( "maj-001.05",  0x300000, 0x080000, CRC(6be7047a) SHA1(22ce8c6fead9e16550047dea341983f59c3a6c28) )
	ROM_LOAD16_BYTE( "maj-001.03",  0x300001, 0x080000, CRC(c4fb6ea0) SHA1(b5cd3cf71831fecd096cd7bae6fb813504d1e0d5) )

	ROM_REGION( 0x100000, "adpcm", 0 )              /* Samples */
	ROM_LOAD( "maj-001.01", 0x000000, 0x080000, CRC(029a0b60) SHA1(d02788b8673ae73aca81f1570ff335982ac9ab40) )
	ROM_LOAD( "maj-001.02", 0x080000, 0x080000, CRC(eb28e641) SHA1(67e1d89c9b40e4a83a3783d4343d7a8121668091) )
ROM_END

ROM_START( mjyuugia ) // PCB: P0-049A
	ROM_REGION( 0x080000, "maincpu", 0 )                    /* 68000 Code */
	ROM_LOAD16_BYTE( "um_001.001", 0x000000, 0x020000, CRC(76dc0594) SHA1(4bd81616769cdc59eaf6f7921e404e166500f67f) )
	ROM_LOAD16_BYTE( "um001.003",  0x000001, 0x020000, CRC(275197de) SHA1(2f8efa112f23f172eaef9bb732b2a253307dd896) )
	ROM_LOAD16_BYTE( "um001.002",  0x040000, 0x020000, CRC(d5dd4710) SHA1(b70c280f828af507c73ebec3209043eb7ce0ce95) )
	ROM_LOAD16_BYTE( "um001.004",  0x040001, 0x020000, CRC(c5ddb567) SHA1(1a35228439108f3d866547d94d4bafca54a710ec) )

	ROM_REGION( 0x800, "mcu1", 0 )
	ROM_LOAD( "x0-005_117.bin", 0x000, 0x800, NO_DUMP )

	ROM_REGION( 0x800, "mcu2", 0 )
	ROM_COPY( "mcu1", 0x000, 0x000, 0x800 )

	ROM_REGION( 0x400000, "gfx1", 0 )   /* Sprites */
	ROM_LOAD16_BYTE( "maj-001.10", 0x000000, 0x080000, CRC(3c08942a) SHA1(59165052d7c760ac82157844d54c8dced4125259) )
	ROM_LOAD16_BYTE( "maj-001.08", 0x000001, 0x080000, CRC(e2444311) SHA1(88673d57d54ef0674c8c23a95da5d03cb9c894aa) )
	ROM_LOAD16_BYTE( "maj-001.09", 0x100000, 0x080000, CRC(a1974860) SHA1(f944026cf1aadb9c24ac689cc67d374eea17cb85) )
	ROM_LOAD16_BYTE( "maj-001.07", 0x100001, 0x080000, CRC(b1f1d118) SHA1(37d64ba662b431cf0fdee12983c95b9989eb00af) )
	ROM_LOAD16_BYTE( "maj-001.06", 0x200000, 0x080000, CRC(4c60acdd) SHA1(0ab69cc3ea4bebd9e7b139c89b5ac42a621493e2) )
	ROM_LOAD16_BYTE( "maj-001.04", 0x200001, 0x080000, CRC(0a4b2de1) SHA1(f9cddeffcdceb06053216502eb03d52abf527eb2) )
	ROM_LOAD16_BYTE( "maj-001.05", 0x300000, 0x080000, CRC(6be7047a) SHA1(22ce8c6fead9e16550047dea341983f59c3a6c28) )
	ROM_LOAD16_BYTE( "maj-001.03", 0x300001, 0x080000, CRC(c4fb6ea0) SHA1(b5cd3cf71831fecd096cd7bae6fb813504d1e0d5) )

	ROM_REGION( 0x100000, "adpcm", 0 )              /* Samples */
	ROM_LOAD( "maj-001.01", 0x000000, 0x080000, CRC(029a0b60) SHA1(d02788b8673ae73aca81f1570ff335982ac9ab40) )
	ROM_LOAD( "maj-001.02", 0x080000, 0x080000, CRC(eb28e641) SHA1(67e1d89c9b40e4a83a3783d4343d7a8121668091) )
ROM_END

ROM_START( ponchin ) // PCB: P0-049A
	ROM_REGION( 0x080000, "maincpu", 0 )                    /* 68000 Code */
	ROM_LOAD16_BYTE( "um2_1_1.u22", 0x000000, 0x020000, CRC(cf88efbb) SHA1(7bd2304d365524fc5bcf3fb30752f5efec73a9f5) )
	ROM_LOAD16_BYTE( "um2_1_3.u42", 0x000001, 0x020000, CRC(e053458f) SHA1(db4a34589a08d0252d700144a6260a0f6c4e8e30) )
	ROM_LOAD16_BYTE( "um2_1_2.u29", 0x040000, 0x020000, CRC(5c2f9bcf) SHA1(e2880123373653c7e5d85fb957474e1c5774640d) )
	ROM_LOAD16_BYTE( "um2_1_4.u44", 0x040001, 0x020000, CRC(2ad4e0c7) SHA1(ca97b825af41f86ebbfc2cf88faafb240c4058d1) )

	// Suspected MCU has different markings than on other PCBs (like NEC instead of Mitsubishi)
	ROM_REGION( 0x800, "mcu1", 0 )
	ROM_LOAD( "x0-005_258.bin", 0x000, 0x800, NO_DUMP )

	ROM_REGION( 0x800, "mcu2", 0 )
	ROM_COPY( "mcu1", 0x000, 0x000, 0x800 )

	ROM_REGION( 0x400000, "gfx1", 0 )   /* Sprites */
	ROM_LOAD16_BYTE( "um2_1_8.u55", 0x000000, 0x080000, CRC(f74a8cb3) SHA1(d1bf712f7ef97a96fc251c7729b39e9f10aab45d) )
	ROM_LOAD16_BYTE( "um2_1_7.u43", 0x000001, 0x080000, CRC(1e87ca84) SHA1(5ddbfd92d6ed1947a3c35f3e93cbcca5059fa1f9) )
	ROM_LOAD16_BYTE( "um2_1_6.u28", 0x200000, 0x080000, CRC(b11e85a7) SHA1(02971b45791d06f88efbae8e0713d28105faf341) )
	ROM_LOAD16_BYTE( "um2_1_5.u20", 0x200001, 0x080000, CRC(a5469d11) SHA1(7e96af23c8434c32f87be1482309999d6a7b33bb) )

	ROM_REGION( 0x100000, "adpcm", 0 )              /* Samples */
	ROM_LOAD( "um2_1_9.u56",  0x000000, 0x080000, CRC(9165c79a) SHA1(854e30fc6121f7b3e5e1e5b6772757a92b63aef8) )
	ROM_LOAD( "um2_1_10.u63", 0x080000, 0x080000, CRC(53e643e9) SHA1(3b221217e8f846ae96a9a47149037cea19d97549) )
ROM_END

ROM_START( ponchina ) // PCB: P0-049A
	ROM_REGION( 0x080000, "maincpu", 0 )                    /* 68000 Code */
	ROM_LOAD16_BYTE( "u22.bin",     0x000000, 0x020000, CRC(9181de20) SHA1(03fdb289d862ff2d87249d35991bd60784e172d9) )
	ROM_LOAD16_BYTE( "um2_1_3.u42", 0x000001, 0x020000, CRC(e053458f) SHA1(db4a34589a08d0252d700144a6260a0f6c4e8e30) )
	ROM_LOAD16_BYTE( "um2_1_2.u29", 0x040000, 0x020000, CRC(5c2f9bcf) SHA1(e2880123373653c7e5d85fb957474e1c5774640d) )
	ROM_LOAD16_BYTE( "um2_1_4.u44", 0x040001, 0x020000, CRC(2ad4e0c7) SHA1(ca97b825af41f86ebbfc2cf88faafb240c4058d1) )

	ROM_REGION( 0x800, "mcu1", 0 )
	ROM_LOAD( "x0-005_258.bin", 0x000, 0x800, NO_DUMP )

	ROM_REGION( 0x800, "mcu2", 0 )
	ROM_COPY( "mcu1", 0x000, 0x000, 0x800 )

	ROM_REGION( 0x400000, "gfx1", 0 )   /* Sprites */
	ROM_LOAD16_BYTE( "um2_1_8.u55", 0x000000, 0x080000, CRC(f74a8cb3) SHA1(d1bf712f7ef97a96fc251c7729b39e9f10aab45d) )
	ROM_LOAD16_BYTE( "um2_1_7.u43", 0x000001, 0x080000, CRC(1e87ca84) SHA1(5ddbfd92d6ed1947a3c35f3e93cbcca5059fa1f9) )
	ROM_LOAD16_BYTE( "um2_1_6.u28", 0x200000, 0x080000, CRC(b11e85a7) SHA1(02971b45791d06f88efbae8e0713d28105faf341) )
	ROM_LOAD16_BYTE( "um2_1_5.u20", 0x200001, 0x080000, CRC(a5469d11) SHA1(7e96af23c8434c32f87be1482309999d6a7b33bb) )

	ROM_REGION( 0x100000, "adpcm", 0 )              /* Samples */
	ROM_LOAD( "um2_1_9.u56",  0x000000, 0x080000, CRC(9165c79a) SHA1(854e30fc6121f7b3e5e1e5b6772757a92b63aef8) )
	ROM_LOAD( "um2_1_10.u63", 0x080000, 0x080000, CRC(53e643e9) SHA1(3b221217e8f846ae96a9a47149037cea19d97549) )
ROM_END

} // anonymous namespace


GAME( 1987, srmp1,     0,        srmp2,    srmp2,    srmp2_state, empty_init, ROT0, "Seta",                "Super Real Mahjong Part 1 (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, srmp2,     0,        srmp2,    srmp2,    srmp2_state, empty_init, ROT0, "Seta",                "Super Real Mahjong Part 2 (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, srmp3,     0,        srmp3,    srmp3,    srmp2_state, empty_init, ROT0, "Seta",                "Super Real Mahjong Part 3 (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, rmgoldyh,  srmp3,    rmgoldyh, rmgoldyh, srmp2_state, empty_init, ROT0, "Seta (Alba license)", "Real Mahjong Gold Yumehai / Super Real Mahjong GOLD part.2 (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, mjyuugi,   0,        mjyuugi,  mjyuugi,  srmp2_state, empty_init, ROT0, "Visco",               "Mahjong Yuugi (Japan set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, mjyuugia,  mjyuugi,  mjyuugi,  mjyuugi,  srmp2_state, empty_init, ROT0, "Visco",               "Mahjong Yuugi (Japan set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, ponchin,   0,        mjyuugi,  ponchin,  srmp2_state, empty_init, ROT0, "Visco",               "Mahjong Pon Chin Kan (Japan set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, ponchina,  ponchin,  mjyuugi,  ponchin,  srmp2_state, empty_init, ROT0, "Visco",               "Mahjong Pon Chin Kan (Japan set 2)", MACHINE_SUPPORTS_SAVE )
