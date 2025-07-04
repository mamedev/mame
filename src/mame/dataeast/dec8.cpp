// license:BSD-3-Clause
// copyright-holders:Bryan McPhail,Stephane Humbert
/***************************************************************************

Various Data East 8 bit games:

Last Mission (rev 6)        (c) 1986 Data East USA (2*6809 + I8751)
Last Mission (rev 5)        (c) 1986 Data East USA (2*6809 + I8751)
Last Mission (Japan)        (c) 1986 Data East Corporation (2*6809 + I8751)
Shackled                    (c) 1986 Data East USA (2*6809 + I8751)
Breywood                    (c) 1986 Data East Corporation (2*6809 + I8751)
Gondomania                  (c) 1987 Data East USA (6809 + I8751)
Makyou Senshi               (c) 1987 Data East Corporation (6809 + I8751)
Garyo Retsuden              (c) 1987 Data East Corporation (6809 + I8751)
The Real Ghostbusters (2p)  (c) 1987 Data East USA (6809 + I8751)
The Real Ghostbusters (3p)  (c) 1987 Data East USA (6809 + I8751)
Meikyuu Hunter G            (c) 1987 Data East Corporation (6809 + I8751)
Captain Silver (World)      (c) 1987 Data East Corporation (2*6809 + I8751)
Captain Silver (Japan)      (c) 1987 Data East Corporation (2*6809 + I8751)
Psycho-Nics Oscar (World)   (c) 1987 Data East Corporation (2*6809)
Psycho-Nics Oscar (US)      (c) 1988 Data East USA (2*6809)
Psycho-Nics Oscar (Japan)   (c) 1987 Data East Corporation (2*6809)
Super Real Darwin (World)   (c) 1987 Data East Corporation (6809 + I8751)
Super Real Darwin (Japan)   (c) 1987 Data East Corporation (6809 + I8751)
Cobra Command (World)       (c) 1988 Data East Corporation (6809)
Cobra Command (Japan)       (c) 1988 Data East Corporation (6809)

All games use a 6502 for sound (some are encrypted), all games except Cobracom
and Oscar use an Intel 8751 for protection & coinage.

Meikyuu Hunter G was formerly known as Mazehunter. It's a Japan-only modified
version of Ghostbusters, due to licensing restrictions.

Emulation by Bryan McPhail, mish@tendril.co.uk
Thanks to Jose Miguel Morales Farreras for Super Real Darwin information!

TODO:
- shackled continue after game over does not work, see MT0418. It's not that
  big of an issue user-wise, since credits add more health. For breywood, it
  appears to work ok after the 1st level.
- srdarwin 'double' sprites appearing from the top of the screen are clipped
- strangely coloured butterfly on Garyo Retsuden water levels!
- gondo 2nd coin doesn't work, probably due to hacked MCU ROM
- ghostb coinage dipswitch
- how does meikyuhbl circumvent the MCU? It won't boot in MAME if MCU is removed

***************************************************************************/

#include "emu.h"
#include "dec8.h"

#include "cpu/m6502/r65c02.h"
#include "cpu/m6809/hd6309.h"
#include "cpu/m6809/m6809.h"
#include "sound/msm5205.h"
#include "sound/ymopn.h"
#include "sound/ymopl.h"

#include "deco222.h"
#include "speaker.h"


/******************************************************************************/

void dec8_state_base::buffer_spriteram16_w(u8 data)
{
	u8* spriteram = m_spriteram->live();

	// copy to a 16-bit region for the sprite chip
	for (int i = 0; i < 0x800/2 ; i++)
		m_buffered_spriteram16[i] = spriteram[(i * 2) + 1] | (spriteram[(i * 2) + 0] << 8);
}

// Only used by gondo, garyoret, ghostb, meikyuh
void ghostb_state::buffer_spriteram_w(int state)
{
	// rising edge
	if (!m_buffer_strobe && state)
		buffer_spriteram16_w(0);

	m_buffer_strobe = bool(state);
}

u8 dec8_mcu_state_base::i8751_hi_r()
{
	return m_i8751_return >> 8; // MSB
}

u8 dec8_mcu_state_base::i8751_lo_r()
{
	return m_i8751_return & 0xff; // LSB
}


/******************************************************************************/

template<unsigned Which>
u8 gondo_state::player_io_r(offs_t offset)
{
	int val = 1 << m_analog_io[Which]->read();

	switch (offset)
	{
		case 0: // Rotary low byte
			return ~(val & 0xff);
		case 1: // Joystick = bottom 4 bits, rotary = top 4
			return ((~val >> 4) & 0xf0) | (m_in_io[Which]->read() & 0xf);
	}
	return 0xff;
}


/***************************************************
*
* Hook-up for games that have a proper MCU dump.
*
***************************************************/

TIMER_CALLBACK_MEMBER(ghostb_state::mcu_irq_clear)
{
	// Gondomania schematics show a clocked LS194 shift register (3A) is used to
	// automatically clear the IRQ request. The MCU does not clear it itself.
	m_mcu->set_input_line(MCS51_INT1_LINE, CLEAR_LINE);
}

TIMER_CALLBACK_MEMBER(ghostb_state::audiocpu_nmi_clear)
{
	// Gondomania schematics show a LS194 for the sound IRQ, sharing the 6502 clock
	// S1=H, S0=L, LSI=H, and QA is the only output connected (to NMI)
	m_audiocpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

void dec8_mcu_state_base::i8751_lo_w(u8 data)
{
	m_i8751_value = (m_i8751_value & 0xff00) | data;
}

void dec8_mcu_state_base::i8751_hi_w(u8 data)
{
	m_i8751_value = (m_i8751_value & 0xff) | (data << 8);

	// SECIRQ is triggered on activating this latch
	if (m_i8751_p2 & 2)
		m_mcu->set_input_line(MCS51_INT1_LINE, ASSERT_LINE);
}

void ghostb_state::i8751_hi_w(u8 data)
{
	m_i8751_value = (m_i8751_value & 0xff) | (data << 8);

	// MCU interrupt is still level-triggered, but no ack?
	m_mcu->set_input_line(MCS51_INT1_LINE, ASSERT_LINE);
	m_i8751_timer->adjust(attotime::from_ticks(64, 12_MHz_XTAL / 8)); // 64 clocks not confirmed
}

void ghostb_state::gondo_i8751_hi_w(u8 data)
{
	m_i8751_value = (m_i8751_value & 0xff) | (data << 8);

	// MCU interrupt is edge-triggered
	m_mcu->set_input_line(MCS51_INT1_LINE, ASSERT_LINE);
	m_i8751_timer->adjust(attotime::from_ticks(4, 12_MHz_XTAL / 8));
}


/******************************************************************************/

void oscar_state::bank_w(u8 data)
{
	m_mainbank->set_entry(data & m_bank_mask);
}

void ghostb_state::gondo_bank_w(u8 data)
{
	/* Bit 0: SECCLR - acknowledge interrupt from I8751
	   Bit 1: NMI enable/disable
	   Bit 2: Not connected according to schematics
	   Bit 3: Screen flip
	   Bits 4-7: Bank switch
	*/
	m_mainbank->set_entry(data >> 4 & m_bank_mask);

	m_secclr = BIT(data, 0);
	if (!m_secclr)
		m_maincpu->set_input_line(M6809_IRQ_LINE, CLEAR_LINE);

	m_nmigate->in_w<0>(BIT(data, 1));
	flip_screen_set(BIT(data, 3));
}

void ghostb_state::ghostb_bank_w(u8 data)
{
	gondo_bank_w(data);

	// Bit 2: Sprite DMA (see gondo_scroll_w for gondo/garyoret)
	buffer_spriteram_w(BIT(data, 2));
}

void csilver_state::control_w(u8 data)
{
	/*
	    Bit 0x0f - ROM bank switch.
	    Bit 0x10 - Always set(?)
	    Bit 0x20 - Unused.
	    Bit 0x40 - Unused.
	    Bit 0x80 - Hold subcpu reset line high if clear, else low? (Not needed anyway)
	*/
	m_mainbank->set_entry(data & m_bank_mask);
}

void ghostb_state::sound_w(u8 data)
{
	m_soundlatch->write(data);
	m_audiocpu->set_input_line(m6502_device::NMI_LINE, ASSERT_LINE);
	m_6502_timer->adjust(m_audiocpu->cycles_to_attotime(4));
}

void csilver_state::adpcm_int(int state)
{
	m_toggle ^= 1;
	if (m_toggle)
		m_audiocpu->set_input_line(m6502_device::IRQ_LINE, ASSERT_LINE);

	m_msm->data_w(m_msm5205next >> 4);
	m_msm5205next <<= 4;
}

u8 csilver_state::adpcm_reset_r()
{
	m_msm->reset_w(0);
	return 0;
}

void csilver_state::adpcm_data_w(u8 data)
{
	m_msm5205next = data;
	m_audiocpu->set_input_line(m6502_device::IRQ_LINE, CLEAR_LINE);
}

void csilver_state::sound_bank_w(u8 data)
{
	m_soundbank->set_entry((data & 0x08) >> 3);
}


/******************************************************************************/

void dec8_state_base::main_irq_on_w(u8 data)
{
	m_maincpu->set_input_line(M6809_IRQ_LINE, ASSERT_LINE);
}

void dec8_state_base::main_irq_off_w(u8 data)
{
	m_maincpu->set_input_line(M6809_IRQ_LINE, CLEAR_LINE);
}

void dec8_state_base::sub_irq_on_w(u8 data)
{
	m_subcpu->set_input_line(M6809_IRQ_LINE, ASSERT_LINE);
}

void dec8_state_base::sub_irq_off_w(u8 data)
{
	m_subcpu->set_input_line(M6809_IRQ_LINE, CLEAR_LINE);
}

void dec8_state_base::both_firq_off_w(u8 data)
{
	m_maincpu->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);
	m_subcpu->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);
}

void dec8_state_base::flip_screen_w(u8 data) { flip_screen_set(data); }


/******************************************************************************/

void lastmisn_state::lastmisn_map(address_map &map)
{
	map(0x0000, 0x0fff).ram().share("share1");
	map(0x1000, 0x13ff).ram().w(m_palette, FUNC(deco_rmc3_device::write8)).share("palette");
	map(0x1400, 0x17ff).ram().w(m_palette, FUNC(deco_rmc3_device::write8_ext)).share("palette_ext");
	map(0x1800, 0x1800).portr("IN0").w(FUNC(lastmisn_state::sub_irq_off_w));
	map(0x1801, 0x1801).portr("IN1").w(FUNC(lastmisn_state::main_irq_off_w));
	map(0x1802, 0x1802).portr("IN2").w(FUNC(lastmisn_state::both_firq_off_w));
	map(0x1803, 0x1803).portr("DSW0").w(FUNC(lastmisn_state::main_irq_on_w));
	map(0x1804, 0x1804).portr("DSW1").w(FUNC(lastmisn_state::sub_irq_on_w));
	map(0x1805, 0x1805).w(FUNC(lastmisn_state::buffer_spriteram16_w));
	map(0x1806, 0x1806).r(FUNC(lastmisn_state::i8751_hi_r));
	map(0x1807, 0x1807).rw(FUNC(lastmisn_state::i8751_lo_r), FUNC(lastmisn_state::flip_screen_w));
	map(0x1809, 0x1809).w(FUNC(lastmisn_state::lastmisn_scrollx_w));
	map(0x180b, 0x180b).w(FUNC(lastmisn_state::lastmisn_scrolly_w));
	map(0x180c, 0x180c).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x180d, 0x180d).w(FUNC(lastmisn_state::lastmisn_control_w));
	map(0x180e, 0x180e).w(FUNC(lastmisn_state::i8751_hi_w));
	map(0x180f, 0x180f).w(FUNC(lastmisn_state::i8751_lo_w));
	map(0x2000, 0x27ff).ram().w(FUNC(lastmisn_state::videoram_w)).share(m_videoram);
	map(0x2800, 0x2fff).ram().share("spriteram");
	map(0x3000, 0x37ff).ram().share("share2");
	map(0x3800, 0x3fff).rw(FUNC(lastmisn_state::bg_ram_r), FUNC(lastmisn_state::bg_ram_w)).share(m_bg_ram);
	map(0x4000, 0x7fff).bankr(m_mainbank);
	map(0x8000, 0xffff).rom();
}

void lastmisn_state::lastmisn_sub_map(address_map &map)
{
	map(0x0000, 0x0fff).ram().share("share1");
	map(0x1000, 0x13ff).ram().w(m_palette, FUNC(deco_rmc3_device::write8)).share("palette");
	map(0x1400, 0x17ff).ram().w(m_palette, FUNC(deco_rmc3_device::write8_ext)).share("palette_ext");
	map(0x1800, 0x1800).portr("IN0").w(FUNC(lastmisn_state::sub_irq_off_w));
	map(0x1801, 0x1801).portr("IN1").w(FUNC(lastmisn_state::main_irq_off_w));
	map(0x1802, 0x1802).portr("IN2").w(FUNC(lastmisn_state::both_firq_off_w));
	map(0x1803, 0x1803).portr("DSW0").w(FUNC(lastmisn_state::main_irq_on_w));
	map(0x1804, 0x1804).portr("DSW1").w(FUNC(lastmisn_state::sub_irq_on_w));
	map(0x1805, 0x1805).w(FUNC(lastmisn_state::buffer_spriteram16_w));
	map(0x1806, 0x1806).r(FUNC(lastmisn_state::i8751_hi_r));
	map(0x1807, 0x1807).rw(FUNC(lastmisn_state::i8751_lo_r), FUNC(lastmisn_state::flip_screen_w));
	map(0x180c, 0x180c).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x180e, 0x180e).w(FUNC(lastmisn_state::i8751_hi_w));
	map(0x180f, 0x180f).w(FUNC(lastmisn_state::i8751_lo_w));
	map(0x2000, 0x27ff).ram().w(FUNC(lastmisn_state::videoram_w));
	map(0x2800, 0x2fff).writeonly().share("spriteram");
	map(0x3000, 0x37ff).ram().share("share2");
	map(0x3800, 0x3fff).rw(FUNC(lastmisn_state::bg_ram_r), FUNC(lastmisn_state::bg_ram_w));
	map(0x4000, 0xffff).rom();
}

void lastmisn_state::shackled_map(address_map &map)
{
	map(0x0000, 0x0fff).ram().share("share1");
	map(0x1000, 0x13ff).ram().w(m_palette, FUNC(deco_rmc3_device::write8)).share("palette");
	map(0x1400, 0x17ff).ram().w(m_palette, FUNC(deco_rmc3_device::write8_ext)).share("palette_ext");
	map(0x1800, 0x1800).portr("IN0").w(FUNC(lastmisn_state::sub_irq_off_w));
	map(0x1801, 0x1801).portr("IN1").w(FUNC(lastmisn_state::main_irq_off_w));
	map(0x1802, 0x1802).portr("IN2").w(FUNC(lastmisn_state::both_firq_off_w));
	map(0x1803, 0x1803).portr("DSW0").w(FUNC(lastmisn_state::main_irq_on_w));
	map(0x1804, 0x1804).portr("DSW1").w(FUNC(lastmisn_state::sub_irq_on_w));
	map(0x1805, 0x1805).w(FUNC(lastmisn_state::buffer_spriteram16_w));
	map(0x1807, 0x1807).w(FUNC(lastmisn_state::flip_screen_w));
	map(0x1809, 0x1809).w(FUNC(lastmisn_state::lastmisn_scrollx_w));
	map(0x180b, 0x180b).w(FUNC(lastmisn_state::lastmisn_scrolly_w));
	map(0x180c, 0x180c).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x180d, 0x180d).w(FUNC(lastmisn_state::shackled_control_w));
	map(0x2000, 0x27ff).ram().w(FUNC(lastmisn_state::videoram_w));
	map(0x2800, 0x2fff).ram().share("spriteram");
	map(0x3000, 0x37ff).ram().share("share2");
	map(0x3800, 0x3fff).rw(FUNC(lastmisn_state::bg_ram_r), FUNC(lastmisn_state::bg_ram_w)).share("bg_ram");
	map(0x4000, 0x7fff).bankr(m_mainbank);
	map(0x8000, 0xffff).rom();
}

void lastmisn_state::shackled_sub_map(address_map &map)
{
	map(0x0000, 0x0fff).ram().share("share1");
	map(0x1000, 0x13ff).ram().w(m_palette, FUNC(deco_rmc3_device::write8)).share("palette");
	map(0x1400, 0x17ff).ram().w(m_palette, FUNC(deco_rmc3_device::write8_ext)).share("palette_ext");
	map(0x1800, 0x1800).portr("IN0").w(FUNC(lastmisn_state::sub_irq_off_w));
	map(0x1801, 0x1801).portr("IN1").w(FUNC(lastmisn_state::main_irq_off_w));
	map(0x1802, 0x1802).portr("IN2").w(FUNC(lastmisn_state::both_firq_off_w));
	map(0x1803, 0x1803).portr("DSW0").w(FUNC(lastmisn_state::main_irq_on_w));
	map(0x1804, 0x1804).portr("DSW1").w(FUNC(lastmisn_state::sub_irq_on_w));
	map(0x1805, 0x1805).w(FUNC(lastmisn_state::buffer_spriteram16_w));
	map(0x1806, 0x1806).r(FUNC(lastmisn_state::i8751_hi_r));
	map(0x1807, 0x1807).rw(FUNC(lastmisn_state::i8751_lo_r), FUNC(lastmisn_state::flip_screen_w));
	map(0x1809, 0x1809).w(FUNC(lastmisn_state::lastmisn_scrollx_w));
	map(0x180b, 0x180b).w(FUNC(lastmisn_state::lastmisn_scrolly_w));
	map(0x180c, 0x180c).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x180d, 0x180d).w(FUNC(lastmisn_state::shackled_control_w));
	map(0x180e, 0x180e).w(FUNC(lastmisn_state::i8751_hi_w));
	map(0x180f, 0x180f).w(FUNC(lastmisn_state::i8751_lo_w));
	map(0x2000, 0x27ff).ram().w(FUNC(lastmisn_state::videoram_w)).share(m_videoram);
	map(0x2800, 0x2fff).ram().share("spriteram");
	map(0x3000, 0x37ff).ram().share("share2");
	map(0x3800, 0x3fff).rw(FUNC(lastmisn_state::bg_ram_r), FUNC(lastmisn_state::bg_ram_w));
	map(0x4000, 0xffff).rom();
}

void ghostb_state::ghostb_map(address_map &map)
{
	map(0x0000, 0x0fff).ram();
	map(0x1000, 0x17ff).ram();
	map(0x1800, 0x1fff).ram().w(FUNC(ghostb_state::videoram_w)).share(m_videoram);
	map(0x2000, 0x27ff).rw(m_tilegen[0], FUNC(deco_bac06_device::pf_data_8bit_r), FUNC(deco_bac06_device::pf_data_8bit_w));
	map(0x2800, 0x2bff).ram(); // colscroll? mirror?
	map(0x2c00, 0x2fff).rw(m_tilegen[0], FUNC(deco_bac06_device::pf_rowscroll_8bit_r), FUNC(deco_bac06_device::pf_rowscroll_8bit_w));
	map(0x3000, 0x37ff).ram().share("spriteram");
	map(0x3800, 0x3800).portr("IN0");
	map(0x3800, 0x3800).w(FUNC(ghostb_state::sound_w));
	map(0x3801, 0x3801).portr("IN1");
	map(0x3802, 0x3802).portr("IN2");
	map(0x3803, 0x3803).portr("DSW0");
	map(0x3820, 0x3820).portr("DSW1");
	map(0x3820, 0x3827).w(m_tilegen[0], FUNC(deco_bac06_device::pf_control0_8bit_w));
	map(0x3830, 0x383f).rw(m_tilegen[0], FUNC(deco_bac06_device::pf_control1_8bit_r), FUNC(deco_bac06_device::pf_control1_8bit_w));
	map(0x3840, 0x3840).r(FUNC(ghostb_state::i8751_hi_r));
	map(0x3840, 0x3840).w(FUNC(ghostb_state::ghostb_bank_w));
	map(0x3860, 0x3860).r(FUNC(ghostb_state::i8751_lo_r));
	map(0x3860, 0x3860).w(FUNC(ghostb_state::i8751_hi_w));
	map(0x3861, 0x3861).w(FUNC(ghostb_state::i8751_lo_w));
	map(0x4000, 0x7fff).bankr(m_mainbank);
	map(0x8000, 0xffff).rom();
}

void gondo_state::gondo_map(address_map &map)
{
	map(0x0000, 0x17ff).ram();
	map(0x1800, 0x1fff).ram().w(FUNC(gondo_state::videoram_w)).share(m_videoram);
	map(0x2000, 0x27ff).rw(FUNC(gondo_state::bg_ram_r), FUNC(gondo_state::bg_ram_w)).share("bg_ram");
	map(0x2800, 0x2bff).ram().w(m_palette, FUNC(deco_rmc3_device::write8)).share("palette");
	map(0x2c00, 0x2fff).ram().w(m_palette, FUNC(deco_rmc3_device::write8_ext)).share("palette_ext");
	map(0x3000, 0x37ff).ram().share("spriteram");
	map(0x3800, 0x3800).portr("DSW0");
	map(0x3801, 0x3801).portr("DSW1");
	map(0x380a, 0x380b).r(FUNC(gondo_state::player_io_r<0>));
	map(0x380c, 0x380d).r(FUNC(gondo_state::player_io_r<1>));
	map(0x380e, 0x380e).portr("IN3");
	map(0x380f, 0x380f).portr("IN2");
	map(0x3810, 0x3810).w(FUNC(gondo_state::sound_w));
	map(0x3818, 0x382f).w(FUNC(gondo_state::gondo_scroll_w));
	map(0x3830, 0x3830).w(FUNC(gondo_state::gondo_bank_w));
	map(0x3838, 0x3838).r(FUNC(gondo_state::i8751_hi_r));
	map(0x3839, 0x3839).r(FUNC(gondo_state::i8751_lo_r));
	map(0x383a, 0x383a).w(FUNC(gondo_state::gondo_i8751_hi_w));
	map(0x383b, 0x383b).w(FUNC(gondo_state::i8751_lo_w));
	map(0x4000, 0x7fff).bankr(m_mainbank);
	map(0x8000, 0xffff).rom();
}

void ghostb_state::garyoret_map(address_map &map)
{
	map(0x0000, 0x17ff).ram();
	map(0x1800, 0x1fff).ram().w(FUNC(ghostb_state::videoram_w)).share(m_videoram);
	map(0x2000, 0x27ff).rw(FUNC(ghostb_state::bg_ram_r), FUNC(ghostb_state::bg_ram_w)).share("bg_ram");
	map(0x2800, 0x2bff).ram().w(m_palette, FUNC(deco_rmc3_device::write8)).share("palette");
	map(0x2c00, 0x2fff).ram().w(m_palette, FUNC(deco_rmc3_device::write8_ext)).share("palette_ext");
	map(0x3000, 0x37ff).ram().share("spriteram");
	map(0x3800, 0x3800).portr("DSW0");
	map(0x3801, 0x3801).portr("DSW1");
	map(0x3808, 0x3808).nopr(); // ?
	map(0x380a, 0x380a).portr("IN1");
	map(0x380b, 0x380b).portr("IN0");
	map(0x3810, 0x3810).w(FUNC(ghostb_state::sound_w));
	map(0x3818, 0x382f).w(FUNC(ghostb_state::gondo_scroll_w));
	map(0x3830, 0x3830).w(FUNC(ghostb_state::gondo_bank_w));
	map(0x3838, 0x3838).w(FUNC(ghostb_state::gondo_i8751_hi_w));
	map(0x3839, 0x3839).w(FUNC(ghostb_state::i8751_lo_w));
	map(0x383a, 0x383a).r(FUNC(ghostb_state::i8751_hi_r));
	map(0x383b, 0x383b).r(FUNC(ghostb_state::i8751_lo_r));
	map(0x4000, 0x7fff).bankr(m_mainbank);
	map(0x8000, 0xffff).rom();
}

void csilver_state::main_map(address_map &map)
{
	map(0x0000, 0x0fff).ram().share("share1");
	map(0x1000, 0x13ff).ram().w(m_palette, FUNC(deco_rmc3_device::write8)).share("palette");
	map(0x1400, 0x17ff).ram().w(m_palette, FUNC(deco_rmc3_device::write8_ext)).share("palette_ext");
	map(0x1800, 0x1800).portr("IN1").w(FUNC(csilver_state::sub_irq_off_w));
	map(0x1801, 0x1801).portr("IN0").w(FUNC(csilver_state::main_irq_off_w));
	map(0x1802, 0x1802).w(FUNC(csilver_state::both_firq_off_w));
	map(0x1803, 0x1803).portr("IN2").w(FUNC(csilver_state::main_irq_on_w));
	map(0x1804, 0x1804).portr("DSW1").w(FUNC(csilver_state::sub_irq_on_w));
	map(0x1805, 0x1805).portr("DSW0").w(FUNC(csilver_state::buffer_spriteram16_w));
	map(0x1807, 0x1807).w(FUNC(csilver_state::flip_screen_w));
	map(0x1808, 0x180b).w(FUNC(csilver_state::scroll_w));
	map(0x180c, 0x180c).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x180d, 0x180d).w(FUNC(csilver_state::control_w));
	map(0x180e, 0x180e).w(FUNC(csilver_state::i8751_hi_w));
	map(0x180f, 0x180f).w(FUNC(csilver_state::i8751_lo_w));
	map(0x1c00, 0x1c00).r(FUNC(csilver_state::i8751_hi_r));
	map(0x1e00, 0x1e00).r(FUNC(csilver_state::i8751_lo_r));
	map(0x2000, 0x27ff).ram().w(FUNC(csilver_state::videoram_w));
	map(0x2800, 0x2fff).ram().share("spriteram");
	map(0x3000, 0x37ff).ram().share("share2");
	map(0x3800, 0x3fff).rw(FUNC(csilver_state::bg_ram_r), FUNC(csilver_state::bg_ram_w)).share("bg_ram");
	map(0x4000, 0x7fff).bankr(m_mainbank);
	map(0x8000, 0xffff).rom();
}

void csilver_state::sub_map(address_map &map)
{
	map(0x0000, 0x0fff).ram().share("share1");
	map(0x1000, 0x13ff).ram().w(m_palette, FUNC(deco_rmc3_device::write8)).share("palette");
	map(0x1400, 0x17ff).ram().w(m_palette, FUNC(deco_rmc3_device::write8_ext)).share("palette_ext");
	map(0x1800, 0x1800).w(FUNC(csilver_state::sub_irq_off_w));
	map(0x1801, 0x1801).w(FUNC(csilver_state::main_irq_off_w));
	map(0x1802, 0x1802).w(FUNC(csilver_state::both_firq_off_w));
	map(0x1803, 0x1803).portr("IN2").w(FUNC(csilver_state::main_irq_on_w));
	map(0x1804, 0x1804).portr("DSW1").w(FUNC(csilver_state::sub_irq_on_w));
	map(0x1805, 0x1805).portr("DSW0").w(FUNC(csilver_state::buffer_spriteram16_w));
	map(0x180c, 0x180c).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x2000, 0x27ff).ram().w(FUNC(csilver_state::videoram_w)).share(m_videoram);
	map(0x2800, 0x2fff).ram().share("spriteram");
	map(0x3000, 0x37ff).ram().share("share2");
	map(0x3800, 0x3fff).rw(FUNC(csilver_state::bg_ram_r), FUNC(csilver_state::bg_ram_w));
	map(0x4000, 0xffff).rom();
}

void oscar_state::oscar_map(address_map &map)
{
	map(0x0000, 0x0eff).ram().share("share1");
	map(0x0f00, 0x0fff).ram();
	map(0x1000, 0x1fff).ram().share("share2");
	map(0x2000, 0x27ff).ram().w(FUNC(oscar_state::videoram_w)).share(m_videoram);
	map(0x2800, 0x2fff).rw(m_tilegen[0], FUNC(deco_bac06_device::pf_data_8bit_r), FUNC(deco_bac06_device::pf_data_8bit_w));
	map(0x3000, 0x37ff).ram().share("spriteram");
	map(0x3800, 0x3bff).ram().w(m_palette, FUNC(deco_rmc3_device::write8)).share("palette");
	map(0x3c00, 0x3c00).portr("IN0");
	map(0x3c01, 0x3c01).portr("IN1");
	map(0x3c02, 0x3c02).portr("IN2");
	map(0x3c03, 0x3c03).portr("DSW0");
	map(0x3c04, 0x3c04).portr("DSW1");
	map(0x3c00, 0x3c07).w(m_tilegen[0], FUNC(deco_bac06_device::pf_control0_8bit_w));
	map(0x3c10, 0x3c1f).w(m_tilegen[0], FUNC(deco_bac06_device::pf_control1_8bit_w));
	map(0x3c80, 0x3c80).w(FUNC(oscar_state::buffer_spriteram16_w)); // DMA
	map(0x3d00, 0x3d00).w(FUNC(oscar_state::bank_w)); // BNKS
	map(0x3d80, 0x3d80).w(m_soundlatch, FUNC(generic_latch_8_device::write)); // SOUN
	map(0x3e00, 0x3e00).w(FUNC(oscar_state::coin_clear_w)); // COINCL
	map(0x3e80, 0x3e80).w(FUNC(oscar_state::sub_irq_on_w)); // IRQ 2
	map(0x3e81, 0x3e81).w(FUNC(oscar_state::main_irq_off_w)); // IRC 1
	map(0x3e82, 0x3e82).w(FUNC(oscar_state::main_irq_on_w)); // IRQ 1
	map(0x3e83, 0x3e83).w(FUNC(oscar_state::sub_irq_off_w)); // IRC 2
	map(0x4000, 0x7fff).bankr(m_mainbank);
	map(0x8000, 0xffff).rom();
}

void oscar_state::oscar_sub_map(address_map &map)
{
	map(0x0000, 0x0eff).ram().share("share1");
	map(0x0f00, 0x0fff).ram();
	map(0x1000, 0x1fff).ram().share("share2");
	map(0x3e80, 0x3e80).w(FUNC(oscar_state::sub_irq_on_w)); // IRQ 2
	map(0x3e81, 0x3e81).w(FUNC(oscar_state::main_irq_off_w)); // IRC 1
	map(0x3e82, 0x3e82).w(FUNC(oscar_state::main_irq_on_w)); // IRQ 1
	map(0x3e83, 0x3e83).w(FUNC(oscar_state::sub_irq_off_w)); // IRC 2
	map(0x4000, 0xffff).rom();
}

void srdarwin_state::main_map(address_map &map)
{
	map(0x0000, 0x05ff).ram();
	map(0x0600, 0x07ff).ram().share("spriteram");
	map(0x0800, 0x0fff).ram().w(FUNC(srdarwin_state::srdarwin_videoram_w)).share(m_videoram);
	map(0x1000, 0x13ff).ram();
	map(0x1400, 0x17ff).rw(FUNC(srdarwin_state::bg_ram_r), FUNC(srdarwin_state::bg_ram_w)).share("bg_ram");
	map(0x1800, 0x1800).w(FUNC(srdarwin_state::i8751_hi_w));
	map(0x1801, 0x1801).w(FUNC(srdarwin_state::i8751_lo_w));
	map(0x1803, 0x1803).nopw();
	map(0x1804, 0x1804).w(m_spriteram, FUNC(buffered_spriteram8_device::write));
	map(0x1805, 0x1806).w(FUNC(srdarwin_state::control_w));
	map(0x2000, 0x2000).r(FUNC(srdarwin_state::i8751_hi_r)).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x2001, 0x2001).rw(FUNC(srdarwin_state::i8751_lo_r), FUNC(srdarwin_state::flip_screen_w));
	map(0x2800, 0x288f).w(m_palette, FUNC(deco_rmc3_device::write8)).share("palette");
	map(0x3000, 0x308f).w(m_palette, FUNC(deco_rmc3_device::write8_ext)).share("palette_ext");
	map(0x3800, 0x3800).portr("DSW0");
	map(0x3801, 0x3801).portr("IN0");
	map(0x3802, 0x3802).portr("IN1");
	map(0x3803, 0x3803).portr("DSW1");
	map(0x4000, 0x7fff).bankr(m_mainbank);
	map(0x8000, 0xffff).rom();
}

void srdarwin_state::srdarwinb_map(address_map &map)
{
	srdarwin_state::main_map(map);

	map(0x1800, 0x1801).nopw();
	map(0x2000, 0x2000).unmapr();
	map(0x2001, 0x2001).portr("COIN");
}

void oscar_state::cobra_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x0800, 0x0fff).rw(m_tilegen[0], FUNC(deco_bac06_device::pf_data_8bit_r), FUNC(deco_bac06_device::pf_data_8bit_w));
	map(0x1000, 0x17ff).rw(m_tilegen[1], FUNC(deco_bac06_device::pf_data_8bit_r), FUNC(deco_bac06_device::pf_data_8bit_w));
	map(0x1800, 0x1fff).ram();
	map(0x2000, 0x27ff).ram().w(FUNC(oscar_state::videoram_w)).share(m_videoram);
	map(0x2800, 0x2fff).ram().share("spriteram");
	map(0x3000, 0x31ff).ram().w(m_palette, FUNC(deco_rmc3_device::write8)).share("palette");
	map(0x3200, 0x37ff).nopw(); // Unused
	map(0x3800, 0x3800).portr("IN0");
	map(0x3801, 0x3801).portr("IN1");
	map(0x3802, 0x3802).portr("DSW0");
	map(0x3803, 0x3803).portr("DSW1");
	map(0x3800, 0x3807).w(m_tilegen[0], FUNC(deco_bac06_device::pf_control0_8bit_w));
	map(0x3810, 0x381f).w(m_tilegen[0], FUNC(deco_bac06_device::pf_control1_8bit_w));
	map(0x3a00, 0x3a00).portr("IN2");
	map(0x3a00, 0x3a07).w(m_tilegen[1], FUNC(deco_bac06_device::pf_control0_8bit_w));
	map(0x3a10, 0x3a1f).w(m_tilegen[1], FUNC(deco_bac06_device::pf_control1_8bit_w));
	map(0x3c00, 0x3c00).w(FUNC(oscar_state::bank_w));
	map(0x3c02, 0x3c02).w(FUNC(oscar_state::buffer_spriteram16_w));
	map(0x3e00, 0x3e00).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x4000, 0x7fff).bankr(m_mainbank);
	map(0x8000, 0xffff).rom();
}


/******************************************************************************/

// Used for Cobra Command, Maze Hunter, Super Real Darwin etc
void dec8_state_base::dec8_s_map(address_map &map)
{
	map(0x0000, 0x05ff).ram();
	map(0x2000, 0x2001).w("ym1", FUNC(ym2203_device::write));
	map(0x4000, 0x4001).w("ym2", FUNC(ym3812_device::write));
	map(0x6000, 0x6000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x8000, 0xffff).rom();
}

// Used by Gondomania, Psycho-Nics Oscar & Garyo Retsuden
void dec8_state_base::oscar_s_map(address_map &map)
{
	map(0x0000, 0x05ff).ram();
	map(0x2000, 0x2001).w("ym1", FUNC(ym2203_device::write));
	map(0x4000, 0x4001).w("ym2", FUNC(ym3526_device::write));
	map(0x6000, 0x6000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x8000, 0xffff).rom();
}

// Used by the bootleg which has a standard M6502 with predecrypted opcodes
void oscar_state::oscarbl_s_opcodes_map(address_map &map)
{
	map(0x8000, 0xffff).rom().region("audiocpu", 0x10000);
}

// Used by Last Mission, Shackled & Breywood
void lastmisn_state::ym3526_s_map(address_map &map)
{
	map(0x0000, 0x05ff).ram();
	map(0x0800, 0x0801).w("ym1", FUNC(ym2203_device::write));
	map(0x1000, 0x1001).w("ym2", FUNC(ym3526_device::write));
	map(0x3000, 0x3000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x8000, 0xffff).rom();
}

// Captain Silver - same sound system as Pocket Gal
void csilver_state::sound_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x0800, 0x0801).w("ym1", FUNC(ym2203_device::write));
	map(0x1000, 0x1001).w("ym2", FUNC(ym3526_device::write));
	map(0x1800, 0x1800).w(FUNC(csilver_state::adpcm_data_w)); // ADPCM data for the MSM5205 chip
	map(0x2000, 0x2000).w(FUNC(csilver_state::sound_bank_w));
	map(0x3000, 0x3000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x3400, 0x3400).r(FUNC(csilver_state::adpcm_reset_r)); // ? not sure
	map(0x4000, 0x7fff).bankr("soundbank");
	map(0x8000, 0xffff).rom();
}


/******************************************************************************/

/*
    Gondomania schematics show the following:

    Port P0 - attached to 2 * LS374 at location 4C & 1C
    Port P1 - attached to 2 * LS374 at location 3C & 2C
    Port P2.2 -> SECIRQ (IRQ to main CPU)
    Port P2.3 -> 'COUNT' (Enable coin counter - also wired directly to coinage) [not emulated]
    Port P2.4-7 -> Enable latches 4C, 1C, 3C, 2C
    Port P3.4-7 -> Directly attached to coinage connector (3 coins & service)

*/

u8 dec8_mcu_state_base::i8751_port0_r()
{
	return m_i8751_port0;
}

void dec8_mcu_state_base::i8751_port0_w(u8 data)
{
	m_i8751_port0 = data;
}

u8 dec8_mcu_state_base::i8751_port1_r()
{
	return m_i8751_port1;
}

void dec8_mcu_state_base::i8751_port1_w(u8 data)
{
	m_i8751_port1 = data;
}

void ghostb_state::mcu_to_main_w(u8 data)
{
	const u8 fall = ~data & m_i8751_p2;
	const u8 rise = data & ~m_i8751_p2;
	m_i8751_p2 = data;

	// P24-P27: controls latches for main CPU communication
	if (BIT(fall, 4))
		m_i8751_port0 = m_i8751_value >> 8;
	if (BIT(fall, 5))
		m_i8751_port1 = m_i8751_value & 0xff;
	if (BIT(fall, 6))
		m_i8751_return = (m_i8751_return & 0xff) | (m_i8751_port0 << 8);
	if (BIT(fall, 7))
		m_i8751_return = (m_i8751_return & 0xff00) | m_i8751_port1;

	// P22: IRQ to main CPU
	if (BIT(rise, 2) && m_secclr)
		m_maincpu->set_input_line(M6809_IRQ_LINE, ASSERT_LINE);

	// P20,P21: N/C
}

void lastmisn_state::mcu_to_main_w(u8 data)
{
	const u8 fall = ~data & m_i8751_p2;
	const u8 rise = data & ~m_i8751_p2;
	m_i8751_p2 = data;

	// P24-P27: controls latches for main CPU communication
	if (BIT(fall, 4))
	{
		m_i8751_port0 = m_i8751_value >> 8;

		// lastmisn mcu sets p0 to 0x00
		m_mcu->set_port_forced_input(0, m_i8751_port0);
	}
	if (BIT(fall, 5))
		m_i8751_port1 = m_i8751_value & 0xff;
	if (BIT(fall, 6))
		m_i8751_return = (m_i8751_return & 0xff) | (m_i8751_port0 << 8);
	if (BIT(fall, 7))
		m_i8751_return = (m_i8751_return & 0xff00) | m_i8751_port1;

	// P22: FIRQ to both CPUs
	if (BIT(rise, 2))
	{
		m_maincpu->set_input_line(M6809_FIRQ_LINE, ASSERT_LINE);
		m_subcpu->set_input_line(M6809_FIRQ_LINE, ASSERT_LINE);
	}

	// P20,P21: clear MCU INT0/1
	if (BIT(~data, 0))
		m_mcu->set_input_line(MCS51_INT0_LINE, CLEAR_LINE);
	if (BIT(~data, 1))
		m_mcu->set_input_line(MCS51_INT1_LINE, CLEAR_LINE);
}

// Super Real Darwin is similar but only appears to have a single port
void srdarwin_state::mcu_to_main_w(u8 data)
{
	const u8 fall = ~data & m_i8751_p2;
	//const u8 rise = data & ~m_i8751_p2;
	m_i8751_p2 = data;

	// P24-P27: controls latches for main CPU communication
	if (BIT(fall, 4))
		m_i8751_port0 = m_i8751_value >> 8;
	if (BIT(fall, 5))
		m_i8751_port0 = m_i8751_value & 0xff;
	if (BIT(fall, 6))
		m_i8751_return = (m_i8751_return & 0xff) | (m_i8751_port0 << 8);
	if (BIT(fall, 7))
		m_i8751_return = (m_i8751_return & 0xff00) | m_i8751_port0;

	// P22: maincpu has no IRQ/FIRQ handler

	// P21: clear MCU INT1
	if (BIT(~data, 1))
		m_mcu->set_input_line(MCS51_INT1_LINE, CLEAR_LINE);
}


void csilver_state::mcu_to_main_w(u8 data)
{
	const u8 fall = ~data & m_i8751_p2;
	const u8 rise = data & ~m_i8751_p2;
	m_i8751_p2 = data;

	// P24-P27: controls latches for main CPU communication
	if (BIT(fall, 4))
		m_i8751_port0 = m_i8751_value >> 8;
	if (BIT(fall, 5))
		m_i8751_port1 = m_i8751_value & 0xff;
	if (BIT(fall, 6))
		m_i8751_return = (m_i8751_return & 0xff) | (m_i8751_port0 << 8);
	if (BIT(fall, 7))
		m_i8751_return = (m_i8751_return & 0xff00) | m_i8751_port1;

	// P22: FIRQ to both CPUs
	if (BIT(rise, 2))
	{
		m_maincpu->set_input_line(M6809_FIRQ_LINE, ASSERT_LINE);
		m_subcpu->set_input_line(M6809_FIRQ_LINE, ASSERT_LINE);
	}

	// P21: clear MCU INT1
	if (BIT(~data, 1))
		m_mcu->set_input_line(MCS51_INT1_LINE, CLEAR_LINE);
}


/******************************************************************************/

// verified from M6809 code
static INPUT_PORTS_START( lastmisn )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) // shoot
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) // bomb
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) // select
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL // shoot
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL // bomb
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL // select
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED ) // coins read through MCU
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED ) // coins read through MCU
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))

	PORT_START("COIN")
	PORT_BIT( 0x1f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_WRITE_LINE_DEVICE_MEMBER("coin", FUNC(input_merger_device::in_w<1>))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_WRITE_LINE_DEVICE_MEMBER("coin", FUNC(input_merger_device::in_w<0>))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_WRITE_LINE_DEVICE_MEMBER("coin", FUNC(input_merger_device::in_w<2>))

	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_B ) )               PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_A ) )               PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )          PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) )              PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x40, "Invulnerability (Cheat)")        PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Infinite Lives (Cheat)")         PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )                PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )           PORT_DIPLOCATION("SW2:2,3") // tables at 0x82c1 (4 words) and 0xde38 (3 words) in 'lastmisn', 0x82c1 and 0xde17 in 'lastmisno'
	PORT_DIPSETTING(    0x06, "30k 70k 70k+" )
	PORT_DIPSETTING(    0x04, "40k 90k 90k+" )
	PORT_DIPSETTING(    0x02, "40k and 80k" )
	PORT_DIPSETTING(    0x00, "50k only" )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) )           PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW2:6")
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW2:7")
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Allow_Continue ) )       PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
INPUT_PORTS_END

// verified from M6809 code
static INPUT_PORTS_START( lastmisnj )
	PORT_INCLUDE(lastmisn)

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )           PORT_DIPLOCATION("SW2:2,3") // tables at 0x82b7 (4 words) and 0xdd29 (3 words)
	PORT_DIPSETTING(    0x06, "30k 50k 50k+" )
	PORT_DIPSETTING(    0x04, "30k 70k 70k+" )
	PORT_DIPSETTING(    0x02, "50k 100k 100k+" )
	PORT_DIPSETTING(    0x00, "50k only" )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) )           PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )                 // "difficult"
	PORT_DIPSETTING(    0x08, DEF_STR( Very_Hard ) )            // "very difficult"
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )              // "top difficult"
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW2:8")
INPUT_PORTS_END


// verified from M6809 code
static INPUT_PORTS_START( shackled )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED ) // coins read through MCU
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED ) // coins read through MCU
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) // tested and discarded by vestigial code at start
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))

	PORT_START("COIN")
	PORT_BIT( 0x1f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_WRITE_LINE_DEVICE_MEMBER("coin", FUNC(input_merger_device::in_w<0>))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_WRITE_LINE_DEVICE_MEMBER("coin", FUNC(input_merger_device::in_w<1>))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_WRITE_LINE_DEVICE_MEMBER("coin", FUNC(input_merger_device::in_w<2>))

	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )          PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED( 0x02, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW1:2")
	PORT_DIPUNUSED( 0x04, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW1:3")
	PORT_DIPUNUSED( 0x08, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW1:4")
	PORT_DIPNAME( 0x10, 0x10, "Leave Off" )                     PORT_DIPLOCATION("SW1:5") // game doesn't boot when this is On - code at 0x401a - related to MCU - "dias" in Dip Switches page
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW1:6")
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW1:7")
	PORT_DIPNAME( 0x80, 0x80, "Freeze" )                        PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	// tables in main CPU : 0x859b (Help), 0x85e9 (6-Help), 0x8fbe (Coin), 0x91b6 (Heart)
	PORT_DIPNAME( 0x07, 0x07, "Coin/Heart/Help/6-Help" )        PORT_DIPLOCATION("SW2:1,2,3") // name from Dip Switches page
	PORT_DIPSETTING( 0x00, "2/100/50/200" )
	PORT_DIPSETTING( 0x01, "4/100/60/300" )
	PORT_DIPSETTING( 0x02, "6/200/70/300" )
	PORT_DIPSETTING( 0x03, "8/200/80/400" )
	PORT_DIPSETTING( 0x07, "10/200/100/500" )
	PORT_DIPSETTING( 0x06, "12/300/100/600" )
	PORT_DIPSETTING( 0x05, "18/400/200/700" )
	PORT_DIPSETTING( 0x04, "20/500/200/800" )
	PORT_DIPUNUSED( 0x08, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW2:4")
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) )           PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Very_Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW2:7")
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )          PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

// verified from M6809 code
static INPUT_PORTS_START( breywood )
	PORT_INCLUDE(shackled)

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, "Power" )                         PORT_DIPLOCATION("SW2:1,2,3,4") // table at 0x41be in sub CPU
	PORT_DIPSETTING( 0x07, "200" )
	PORT_DIPSETTING( 0x0b, "300" )
	PORT_DIPSETTING( 0x03, "400" )
	PORT_DIPSETTING( 0x0d, "500" )
	PORT_DIPSETTING( 0x05, "600" )
	PORT_DIPSETTING( 0x09, "700" )
	PORT_DIPSETTING( 0x01, "800" )
	PORT_DIPSETTING( 0x0e, "900" )
	PORT_DIPSETTING( 0x0f, "1000" )
	PORT_DIPSETTING( 0x06, "2000" )
	PORT_DIPSETTING( 0x0a, "3000" )
	PORT_DIPSETTING( 0x02, "4000" )
	PORT_DIPSETTING( 0x0c, "5000" )
	PORT_DIPSETTING( 0x04, "6000" )
	PORT_DIPSETTING( 0x08, "7000" )
	PORT_DIPSETTING( 0x00, "8000" )
INPUT_PORTS_END


// verified from HD6309 code
static INPUT_PORTS_START( gondo )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	// Top 4 bits are rotary controller

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	// Top 4 bits are rotary controller

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("COIN") // hooked up on the i8751
	// Low 4 bits not connected on schematics
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) // produces sound but gives 0 credits - coinage not initialised in the MCU

	PORT_START("AN0") // player 1 12-way rotary control
	PORT_BIT( 0x0f, 0x00, IPT_POSITIONAL ) PORT_POSITIONS(12) PORT_WRAPS PORT_SENSITIVITY(15) PORT_KEYDELTA(1) PORT_CODE_DEC(KEYCODE_Z) PORT_CODE_INC(KEYCODE_X) PORT_REVERSE PORT_FULL_TURN_COUNT(12)

	PORT_START("AN1") // player 2 12-way rotary control
	PORT_BIT( 0x0f, 0x00, IPT_POSITIONAL ) PORT_POSITIONS(12) PORT_WRAPS PORT_SENSITIVITY(15) PORT_KEYDELTA(1) PORT_CODE_DEC(KEYCODE_N) PORT_CODE_INC(KEYCODE_M) PORT_PLAYER(2) PORT_REVERSE PORT_FULL_TURN_COUNT(12)

	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )               PORT_DIPLOCATION("SW1:1,2") // table at 0x01b8 in MCU (4 bytes : coins in 4 MSbits and credits in 4 LSbits)
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )               PORT_DIPLOCATION("SW1:3,4") // table at 0x01bc in MCU (4 bytes : coins in 4 MSbits and credits in 4 LSbits)
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW1:5")
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )          PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )          PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Swap buttons" )                  PORT_DIPLOCATION("SW1:8") // code at 0x8a2b in 'gondo', 0x88c5 in 'makyosen' - undocumented in the manual
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )                PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x00, "99 (Cheat)") // gives 99 lives
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )           PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Allow_Continue ) )       PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW2:6")
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW2:7")
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW2:8")
INPUT_PORTS_END


// verified from HD6309 code
static INPUT_PORTS_START( garyoret )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) // shoot
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) // bomb
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) // shoot
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) // bomb
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))

	PORT_START("COIN") // hooked up on the i8751
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) // produces sound but gives 0 credits - coinage not initialised in the MCU

	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )               PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )               PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW1:5")
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )          PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )          PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW1:8") // not tested - no cocktail when simultaneous players anyway

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )                PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPUNUSED( 0x02, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW2:2")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )           PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW2:5")
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW2:6")
	PORT_DIPNAME( 0x40, 0x40, "Leave Off" )                     PORT_DIPLOCATION("SW2:7") // game doesn't boot when this is On - code at 0x807f and test at 0x819e
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW2:8")
INPUT_PORTS_END


// verified from HD6309 code
static INPUT_PORTS_START( ghostb )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) // "FIRE"
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) // beam / upgradable shot when out of energy
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) // "FIRE"
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) // beam / upgradable shot when out of energy
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("COIN")
	// Low 4 bits not connected on schematics
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) // produce sound but gives 0 credits - "ANDA" instruction at 0x8a5a

	PORT_START("DSW0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 ) // Tested on real hardware
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	// 1-2 should be coinage.
//  PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )               PORT_DIPLOCATION("SW1:1,2")
//  PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
//  PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
//  PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
//  PORT_DIPLOCATION("SW1:3") // Manual says 'Must Be Off'. Note: Turning on 3+4+5+8 does nothing on real hardware.
	PORT_DIPUNUSED( 0x04, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW1:4") // Manual says 'Must Be Off'. See note
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW1:5") // Manual says 'Must Be Off'. See note
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )          PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )          PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW1:8") // not tested - no cocktail when simultaneous players anyway. Manual says 'Must Be Off'. See note

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )                PORT_DIPLOCATION("SW2:1,2") // lives are added when STARTn is pressed
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x00, "Invulnerability (Cheat)")        // gives 1 life and energy does not decrease
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )           PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x30, 0x30, "Max Scene Time" )                PORT_DIPLOCATION("SW2:5,6") // 1:00 is added when STARTn is pressed until max scene time is reached
	PORT_DIPSETTING(    0x00, "4:00" )
	PORT_DIPSETTING(    0x10, "4:30" )
	PORT_DIPSETTING(    0x30, "5:00" )
	PORT_DIPSETTING(    0x20, "6:00" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Allow_Continue ) )       PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Energy Bonus" )                  PORT_DIPLOCATION("SW2:8") // energy is set to value each new life
	PORT_DIPSETTING(    0x80, DEF_STR( None ) )                                           // 0x0100
	PORT_DIPSETTING(    0x00, "+25%" )                                                    // 0x0140
INPUT_PORTS_END

// verified from HD6309 code
static INPUT_PORTS_START( ghostb2a )
	PORT_INCLUDE(ghostb)

	// BUTTON1 : upgradable shot - BUTTON2 : beam (provided you have energy)

	PORT_MODIFY("COIN")
	// Low 4 bits not connected on schematics
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) // produce sound but gives 0 lives - "ANDA" instruction at 0x8a20

	PORT_MODIFY("DSW0")
	// NO start buttons - to start a game, press any button from any player
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	// 1 & 2 should be coinage
//  PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )               PORT_DIPLOCATION("SW1:1,2")
//  PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
//  PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
//  PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )

	PORT_MODIFY("DSW1")
	// lives are added when COINn is pressed
	// 1:00 is added when COINn is pressed until max scene time is reached
	PORT_DIPNAME( 0x80, 0x80, "Energy Bonus" )                  PORT_DIPLOCATION("SW2:8") // energy is added when COINn is pressed
	PORT_DIPSETTING(    0x80, DEF_STR( None ) )                                           // 0x0040
	PORT_DIPSETTING(    0x00, "+50%" )                                                    // 0x0060
INPUT_PORTS_END

// verified from HD6309 code
static INPUT_PORTS_START( ghostb3 )
	PORT_INCLUDE(ghostb2a)

	PORT_MODIFY("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3) // upgradable shot
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3) // beam (provided you have energy)

	PORT_MODIFY("COIN")
	// Low 4 bits not connected on schematics
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3 )
INPUT_PORTS_END

// verified from HD6309 code
static INPUT_PORTS_START( meikyuh )
	PORT_INCLUDE(ghostb)

	PORT_MODIFY("COIN")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	// BUTTON1 : upgradable shot - BUTTON2 : circular fire (provided you have energy) - BUTTON1 + BUTTON2 : beam (provided you have energy)

	PORT_MODIFY("DSW1")
	// lives are added when STARTn is pressed - 1 extra life is awarded on 2nd credit and after for the same player who gets then 2, 4 or 6 additional lives
	// max time scene is always 6:00 at start - 0:30 is subed every 8 levels - 1:00 is added when STARTn is pressed until max scene time is reached
	PORT_DIPNAME( 0x10, 0x10, "Energy Bonus" )                  PORT_DIPLOCATION("SW2:5") // energy is added when STARTn is pressed
	PORT_DIPSETTING(    0x10, DEF_STR( None ) )                                           // 0x0020
	PORT_DIPSETTING(    0x00, "+50%" )                                                    // 0x0030
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW2:6")
	PORT_DIPNAME( 0x80, 0x80, "Freeze" )                        PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


// verified from M6809 code
static INPUT_PORTS_START( csilver )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) // sword
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) // jump
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL // sword
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL // jump
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED ) // coins read through MCU
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED ) // ^
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))

	PORT_START("COIN") // hooked up on the i8751
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_B ) )               PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_A ) )               PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW1:5")
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )          PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )          PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )              PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )                PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x00, "255 (Cheat)")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )           PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Allow_Continue ) )       PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW2:6")
	PORT_DIPNAME( 0x40, 0x40, "No Key for Door (Cheat)")        PORT_DIPLOCATION("SW2:7") // code at 0x9816 in sub CPU
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW2:8")
INPUT_PORTS_END

// verified from M6809 code
static INPUT_PORTS_START( csilverj )
	PORT_INCLUDE(csilver)

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_B ) )               PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_A ) )               PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
INPUT_PORTS_END


// verified from HD6309 code
static INPUT_PORTS_START( oscar )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) // shoot
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) // jump
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) // select
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL // shoot
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL // jump
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL // select
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_WRITE_LINE_DEVICE_MEMBER("coin", FUNC(input_merger_device::in_w<1>))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_WRITE_LINE_DEVICE_MEMBER("coin", FUNC(input_merger_device::in_w<0>))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_WRITE_LINE_DEVICE_MEMBER("coin", FUNC(input_merger_device::in_w<2>))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))

	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_B ) )               PORT_DIPLOCATION("SW1:1,2") // table at 0xf8e3 (4 * 2 bytes : coins then credits)
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_A ) )               PORT_DIPLOCATION("SW1:3,4") // table at 0xf8eb (4 * 2 bytes : coins then credits)
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x10, 0x10, "Freeze Mode" )                   PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )          PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )          PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )              PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )                PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x00, "Infinite (Cheat)")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )           PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )           PORT_DIPLOCATION("SW2:5,6") // tables at 0x82d8 (4 words) and 0xf3fe (3 words)
	PORT_DIPSETTING(    0x30, "40k 100k 60k+" )
	PORT_DIPSETTING(    0x20, "60k 160k 100k+" )
	PORT_DIPSETTING(    0x10, "90k 240k 150k+" )
	PORT_DIPSETTING(    0x00, "50k only" )
	PORT_DIPNAME( 0x40, 0x40, "Invulnerability (Cheat)")        PORT_DIPLOCATION("SW2:7") // not when falling into void or water - also gives infinite time
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Allow_Continue ) )       PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )
INPUT_PORTS_END

// verified from HD6309 code
static INPUT_PORTS_START( oscarj )
	PORT_INCLUDE(oscar)

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_B ) )               PORT_DIPLOCATION("SW1:1,2") // table at 0xf8d6 (4 * 2 bytes : coins then credits) in 'oscarj1', 0xf8e6 in 'oscarj2', 0xf8f2 in 'oscaru'
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_A ) )               PORT_DIPLOCATION("SW1:3,4") // table at 0xf8de (4 * 2 bytes : coins then credits) in 'oscarj1', 0xf8ee in 'oscarj2', 0xf8fa in 'oscaru'
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )

	// bonus lives : tables at 0x82d8 (4 words) and 0xf3f1 (3 words) in 'oscarj1', 0x82de and 0xf401 in 'orscarj2', 0x82d8 and 0xf412 in 'orscaru' - same as in 'oscar'
INPUT_PORTS_END


// verified from M6809 code
static INPUT_PORTS_START( srdarwin )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("COIN") // hooked up on the i8751
	PORT_BIT( 0x1f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_B ) )               PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_A ) )               PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW1:5")
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )          PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )          PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )              PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )                PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x00, "28 (Cheat)")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )           PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Bonus_Life ) )           PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, "Every 50k" )                     // table at 0xab06 - last bonus life at 850k
	PORT_DIPSETTING(    0x00, "Every 100k" )                    // table at 0xab17 - last bonus life at 900k
	PORT_DIPNAME( 0x20, 0x20, "After Stage 10" )                PORT_DIPLOCATION("SW2:6") // code at 0xab94
	PORT_DIPSETTING(    0x20, "Back to Stage 1" )
	PORT_DIPSETTING(    0x00, "Game Over" )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW2:7")
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Allow_Continue ) )       PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )
INPUT_PORTS_END

// verified from M6809 code
static INPUT_PORTS_START( srdarwinj )
	PORT_INCLUDE(srdarwin)

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_B ) )               PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_A ) )               PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
INPUT_PORTS_END


// verified from M6809 code
static INPUT_PORTS_START( cobracom )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) // fire
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) // missile
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) // fire
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) // missile
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 ) // always adds 1 credit
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))

	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )               PORT_DIPLOCATION("SW1:1,2") // code at 0x88b7 in 'cobracom', 0x890e in 'cobracomj'
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )               PORT_DIPLOCATION("SW1:3,4") // code at 0x889b in 'cobracom', 0x88f2 in 'cobracomj'
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )              PORT_DIPLOCATION("SW1:5") // Manual says 'Off'
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )          PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )          PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )              PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )                PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "99 (Cheat)")                     // lose a life before getting 2nd bonus life !
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )           PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Allow_Continue ) )       PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Bonus_Life ) )           PORT_DIPLOCATION("SW2:6") // table at 0xa898 (2* 2 words) in 'cobracomj', 0xa8fe in 'cobracomj'
	PORT_DIPSETTING(    0x20, "50k and 150k" )
	PORT_DIPSETTING(    0x00, "100k and 200k" )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW2:7")
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )                       PORT_DIPLOCATION("SW2:8") // previously "Freeze" : code at 0x8849 in 'cobracomj', 0x88a0 in 'cobracomj'
INPUT_PORTS_END


/******************************************************************************/

static const gfx_layout charlayout_32k =
{
	8,8,
	1024,
	2,
	{ 0x4000*8,0x0000*8 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 // every sprite takes 8 consecutive bytes
};

static const gfx_layout chars_3bpp =
{
	8,8,
	1024,
	3,
	{ 0x6000*8,0x4000*8,0x2000*8 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 // every sprite takes 8 consecutive bytes
};

// SRDarwin characters - very unusual layout for Data East
static const gfx_layout charlayout_16k =
{
	8,8,    // 8*8 characters
	1024,
	2,  // 2 bits per pixel
	{ 0, 4 },   // the two bitplanes for 4 pixels are packed into one byte
	{ 0x2000*8+0, 0x2000*8+1, 0x2000*8+2, 0x2000*8+3, 0, 1, 2, 3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 // every char takes 8 consecutive bytes
};

static const gfx_layout oscar_charlayout =
{
	8,8,
	1024,
	3,
	{ 0x3000*8,0x2000*8,0x1000*8 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 // every sprite takes 8 consecutive bytes
};

// Darwin sprites - only 3bpp
static const gfx_layout sr_sprites =
{
	16,16,
	2048,
	3,
	{ 0x10000*8,0x20000*8,0x00000*8 },
	{ 16*8, 1+(16*8), 2+(16*8), 3+(16*8), 4+(16*8), 5+(16*8), 6+(16*8), 7+(16*8),
		0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 ,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8 },
//  { 0*8, 2*8, 1*8, 3*8, 4*8, 6*8, 5*8, 7*8 ,8*8,10*8,9*8,11*8,12*8,14*8,13*8,15*8 }, bootleg decode
	16*16
};

static const gfx_layout srdarwin_tiles =
{
	16,16,
	256,
	4,
	{ 0x8000*8, 0x8000*8+4, 0, 4 },
	{ 0, 1, 2, 3, 1024*8*8+0, 1024*8*8+1, 1024*8*8+2, 1024*8*8+3,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+1024*8*8+0, 16*8+1024*8*8+1, 16*8+1024*8*8+2, 16*8+1024*8*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8    // every tile takes 32 consecutive bytes
};

static const gfx_layout tiles =
{
	16,16,
	4096,
	4,
	{ 0x60000*8,0x40000*8,0x20000*8,0x00000*8 },
	{ 16*8, 1+(16*8), 2+(16*8), 3+(16*8), 4+(16*8), 5+(16*8), 6+(16*8), 7+(16*8),
		0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 ,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8},
	16*16
};

// X flipped on Ghostbusters tiles
static const gfx_layout tiles_r =
{
	16,16,
	2048,
	4,
	{ 0x20000*8,0x00000*8,0x30000*8,0x10000*8 },
	{ 7,6,5,4,3,2,1,0,
		7+(16*8), 6+(16*8), 5+(16*8), 4+(16*8), 3+(16*8), 2+(16*8), 1+(16*8), 0+(16*8) },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 ,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8},
	16*16
};

static GFXDECODE_START( gfx_cobracom )
	GFXDECODE_ENTRY( "char",    0, charlayout_32k, 0, 8 )
	GFXDECODE_ENTRY( "tiles2",  0, tiles,        128, 4 )
	GFXDECODE_ENTRY( "tiles1",  0, tiles,        192, 4 )
GFXDECODE_END

static GFXDECODE_START( gfx_cobracom_spr )
	GFXDECODE_ENTRY( "sprites", 0, tiles, 64, 4 )
GFXDECODE_END

static GFXDECODE_START( gfx_ghostb )
	GFXDECODE_ENTRY( "char",    0, chars_3bpp, 0,  4 )
	GFXDECODE_ENTRY( "tiles1",  0, tiles_r,  512, 16 )
GFXDECODE_END

static GFXDECODE_START( gfx_srdarwin )
	GFXDECODE_ENTRY( "char",    0x00000, charlayout_16k, 128, 4 ) // Only 1 used so far :/
	GFXDECODE_ENTRY( "sprites", 0x00000, sr_sprites,      64, 8 )
	GFXDECODE_ENTRY( "tiles1",  0x00000, srdarwin_tiles,   0, 8 )
	GFXDECODE_ENTRY( "tiles1",  0x10000, srdarwin_tiles,   0, 8 )
	GFXDECODE_ENTRY( "tiles1",  0x20000, srdarwin_tiles,   0, 8 )
	GFXDECODE_ENTRY( "tiles1",  0x30000, srdarwin_tiles,   0, 8 )
GFXDECODE_END

static GFXDECODE_START( gfx_gondo )
	GFXDECODE_ENTRY( "char",    0, chars_3bpp, 0, 16 ) // Chars
	GFXDECODE_ENTRY( "tiles1",  0, tiles,    768, 16 ) // Tiles
GFXDECODE_END

static GFXDECODE_START( gfx_gondo_spr )
	GFXDECODE_ENTRY( "sprites", 0, tiles, 256, 32 ) // Sprites
GFXDECODE_END

static GFXDECODE_START( gfx_oscar )
	GFXDECODE_ENTRY( "char",    0, oscar_charlayout, 256,  8 ) // Chars
	GFXDECODE_ENTRY( "tiles1",  0, tiles,            384,  8 ) // Tiles
GFXDECODE_END

static GFXDECODE_START( gfx_oscar_spr )
	GFXDECODE_ENTRY( "sprites", 0, tiles, 0, 16 ) // Sprites
GFXDECODE_END

static GFXDECODE_START( gfx_shackled )
	GFXDECODE_ENTRY( "char",    0, chars_3bpp, 0,  4 )
	GFXDECODE_ENTRY( "tiles1",  0, tiles,    768, 16 )
GFXDECODE_END

static GFXDECODE_START( gfx_shackled_spr )
	GFXDECODE_ENTRY( "sprites", 0, tiles, 256, 16 )
GFXDECODE_END


/******************************************************************************/

// Coins generate NMI's
void oscar_state::coin_irq(int state)
{
	if (state && !m_coin_state)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	m_coin_state = bool(state);
}

void oscar_state::coin_clear_w(u8 data)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

void lastmisn_state::shackled_coin_irq(int state)
{
	if (state && !m_coin_state)
		m_mcu->set_input_line(MCS51_INT0_LINE, ASSERT_LINE);
	m_coin_state = bool(state);
}


/******************************************************************************/

void dec8_state_base::machine_start()
{
	u8 *ROM = memregion("mainbank")->base();
	u8 max_bank = memregion("mainbank")->bytes() / 0x4000;
	m_mainbank->configure_entries(0, max_bank, &ROM[0], 0x4000);
	m_bank_mask = (max_bank - 1) & 0xf;

	save_item(NAME(m_coin_state));
	save_item(NAME(m_scroll));
}

void dec8_state_base::machine_reset()
{
	m_scroll[0] = m_scroll[1] = m_scroll[2] = m_scroll[3] = 0;
}


void dec8_mcu_state_base::machine_start()
{
	dec8_state_base::machine_start();

	m_i8751_p2 = 0xff;

	save_item(NAME(m_i8751_p2));
	save_item(NAME(m_i8751_port0));
	save_item(NAME(m_i8751_port1));
	save_item(NAME(m_i8751_return));
	save_item(NAME(m_i8751_value));
}

void dec8_mcu_state_base::machine_reset()
{
	dec8_state_base::machine_reset();

	m_i8751_port0 = m_i8751_port1 = 0;
	m_i8751_return = m_i8751_value = 0;
}


void ghostb_state::machine_start()
{
	lastmisn_state::machine_start();

	m_6502_timer = timer_alloc(FUNC(ghostb_state::audiocpu_nmi_clear), this);
	m_i8751_timer = timer_alloc(FUNC(ghostb_state::mcu_irq_clear), this);

	save_item(NAME(m_secclr));
	save_item(NAME(m_buffer_strobe));
}

void ghostb_state::machine_reset()
{
	lastmisn_state::machine_reset();

	// reset clears LS273 latch, which disables NMI
	ghostb_bank_w(0);
}


void csilver_state::machine_start()
{
	lastmisn_state::machine_start();

	u8 *RAM = memregion("audiocpu")->base();
	m_soundbank->configure_entries(0, 2, &RAM[0], 0x4000);

	save_item(NAME(m_msm5205next));
	save_item(NAME(m_toggle));
}

void csilver_state::machine_reset()
{
	lastmisn_state::machine_reset();

	m_msm5205next = 0;
	m_toggle = 0;
}


void dec8_state_base::set_screen_raw_params(machine_config &config)
{
	// DECO video CRTC, matches PCB measurements
	m_screen->set_raw(12_MHz_XTAL / 2, 384, 0, 256, 272, 8, 248);
}

void lastmisn_state::lastmisn(machine_config &config)
{
	// basic machine hardware
	MC6809E(config, m_maincpu, 12_MHz_XTAL / 8); // MC68B09EP in schematics
	m_maincpu->set_addrmap(AS_PROGRAM, &lastmisn_state::lastmisn_map);

	MC6809E(config, m_subcpu, 12_MHz_XTAL / 8); // MC68B09EP in schematics
	m_subcpu->set_addrmap(AS_PROGRAM, &lastmisn_state::lastmisn_sub_map);

	R65C02(config, m_audiocpu, 12_MHz_XTAL / 8);
	m_audiocpu->set_addrmap(AS_PROGRAM, &lastmisn_state::ym3526_s_map); // NMIs are caused by the main CPU

	I8751(config, m_mcu, 8_MHz_XTAL);
	m_mcu->port_in_cb<0>().set(FUNC(lastmisn_state::i8751_port0_r));
	m_mcu->port_out_cb<0>().set(FUNC(lastmisn_state::i8751_port0_w));
	m_mcu->port_in_cb<1>().set(FUNC(lastmisn_state::i8751_port1_r));
	m_mcu->port_out_cb<1>().set(FUNC(lastmisn_state::i8751_port1_w));
	m_mcu->port_out_cb<2>().set(FUNC(lastmisn_state::mcu_to_main_w));
	m_mcu->port_in_cb<3>().set_ioport("COIN");

	config.set_perfect_quantum(m_mcu);

	INPUT_MERGER_ANY_LOW(config, "coin").output_handler().set(FUNC(lastmisn_state::shackled_coin_irq));

	// video hardware
	BUFFERED_SPRITERAM8(config, m_spriteram);

	DECO_KARNOVSPRITES(config, m_spritegen_krn, 0, m_palette, gfx_shackled_spr);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	set_screen_raw_params(config);
	m_screen->set_screen_update(FUNC(lastmisn_state::screen_update_lastmisn));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_shackled);
	DECO_RMC3(config, m_palette, 0, 1024); // xxxxBBBBGGGGRRRR with custom weighting

	MCFG_VIDEO_START_OVERRIDE(lastmisn_state,lastmisn)

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	INPUT_MERGER_ANY_HIGH(config, m_soundirq);
	m_soundirq->output_handler().set_inputline(m_audiocpu, m6502_device::IRQ_LINE);

	ym2203_device &ym1(YM2203(config, "ym1", 12_MHz_XTAL / 8));
	ym1.irq_handler().set(m_soundirq, FUNC(input_merger_device::in_w<0>));
	ym1.add_route(0, "mono", 0.20);
	ym1.add_route(1, "mono", 0.20);
	ym1.add_route(2, "mono", 0.20);
	ym1.add_route(3, "mono", 0.40);

	ym3526_device &ym2(YM3526(config, "ym2", 12_MHz_XTAL / 4));
	ym2.irq_handler().set(m_soundirq, FUNC(input_merger_device::in_w<1>));
	ym2.add_route(ALL_OUTPUTS, "mono", 0.80);
}

void lastmisn_state::shackled(machine_config &config)
{
	// basic machine hardware
	MC6809E(config, m_maincpu, 12_MHz_XTAL / 8);
	m_maincpu->set_addrmap(AS_PROGRAM, &lastmisn_state::shackled_map);

	MC6809E(config, m_subcpu, 12_MHz_XTAL / 8);
	m_subcpu->set_addrmap(AS_PROGRAM, &lastmisn_state::shackled_sub_map);

	R65C02(config, m_audiocpu, 12_MHz_XTAL / 8);
	m_audiocpu->set_addrmap(AS_PROGRAM, &lastmisn_state::ym3526_s_map); // NMIs are caused by the main CPU

	I8751(config, m_mcu, 8_MHz_XTAL);
	m_mcu->port_in_cb<0>().set(FUNC(lastmisn_state::i8751_port0_r));
	m_mcu->port_out_cb<0>().set(FUNC(lastmisn_state::i8751_port0_w));
	m_mcu->port_in_cb<1>().set(FUNC(lastmisn_state::i8751_port1_r));
	m_mcu->port_out_cb<1>().set(FUNC(lastmisn_state::i8751_port1_w));
	m_mcu->port_out_cb<2>().set(FUNC(lastmisn_state::mcu_to_main_w));
	m_mcu->port_in_cb<3>().set_ioport("COIN");

	config.set_perfect_quantum(m_maincpu); // needs heavy sync, otherwise one of the two CPUs will miss an IRQ and cause the game to hang

	INPUT_MERGER_ANY_LOW(config, "coin").output_handler().set(FUNC(lastmisn_state::shackled_coin_irq));

	// video hardware
	BUFFERED_SPRITERAM8(config, m_spriteram);

	DECO_KARNOVSPRITES(config, m_spritegen_krn, 0, m_palette, gfx_shackled_spr);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	set_screen_raw_params(config);
	m_screen->set_screen_update(FUNC(lastmisn_state::screen_update_shackled));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_shackled);
	DECO_RMC3(config, m_palette, 0, 1024); // xxxxBBBBGGGGRRRR with custom weighting

	MCFG_VIDEO_START_OVERRIDE(lastmisn_state,shackled)

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	INPUT_MERGER_ANY_HIGH(config, m_soundirq);
	m_soundirq->output_handler().set_inputline(m_audiocpu, m6502_device::IRQ_LINE);

	ym2203_device &ym1(YM2203(config, "ym1", 12_MHz_XTAL / 8));
	ym1.irq_handler().set(m_soundirq, FUNC(input_merger_device::in_w<0>));
	ym1.add_route(0, "mono", 0.20);
	ym1.add_route(1, "mono", 0.20);
	ym1.add_route(2, "mono", 0.20);
	ym1.add_route(3, "mono", 0.40);

	ym3526_device &ym2(YM3526(config, "ym2", 12_MHz_XTAL / 4));
	ym2.irq_handler().set(m_soundirq, FUNC(input_merger_device::in_w<1>));
	ym2.add_route(ALL_OUTPUTS, "mono", 0.80);
}

void gondo_state::gondo(machine_config &config)
{
	// basic machine hardware
	HD6309E(config, m_maincpu, 3'000'000); // HD63C09EP
	m_maincpu->set_addrmap(AS_PROGRAM, &gondo_state::gondo_map);

	R65C02(config, m_audiocpu, 1'500'000);
	m_audiocpu->set_addrmap(AS_PROGRAM, &gondo_state::oscar_s_map); // NMIs are caused by the main CPU

	I8751(config, m_mcu, 8_MHz_XTAL);
	m_mcu->port_in_cb<0>().set(FUNC(gondo_state::i8751_port0_r));
	m_mcu->port_out_cb<0>().set(FUNC(gondo_state::i8751_port0_w));
	m_mcu->port_in_cb<1>().set(FUNC(gondo_state::i8751_port1_r));
	m_mcu->port_out_cb<1>().set(FUNC(gondo_state::i8751_port1_w));
	m_mcu->port_out_cb<2>().set(FUNC(gondo_state::mcu_to_main_w));
	m_mcu->port_in_cb<3>().set_ioport("COIN");

	config.set_perfect_quantum(m_mcu);

	// video hardware
	BUFFERED_SPRITERAM8(config, m_spriteram);

	DECO_KARNOVSPRITES(config, m_spritegen_krn, 0, m_palette, gfx_gondo_spr);
	m_spritegen_krn->set_colpri_callback(FUNC(gondo_state::gondo_colpri_cb));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	set_screen_raw_params(config);
	m_screen->set_screen_update(FUNC(gondo_state::screen_update_gondo));
	m_screen->screen_vblank().set(m_nmigate, FUNC(input_merger_device::in_w<1>));
	m_screen->screen_vblank().append_inputline(m_mcu, MCS51_INT0_LINE);
	m_screen->set_palette(m_palette);

	INPUT_MERGER_ALL_HIGH(config, m_nmigate).output_handler().set_inputline(m_maincpu, INPUT_LINE_NMI);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_gondo);
	DECO_RMC3(config, m_palette, 0, 1024); // xxxxBBBBGGGGRRRR with custom weighting

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	INPUT_MERGER_ANY_HIGH(config, m_soundirq);
	m_soundirq->output_handler().set_inputline(m_audiocpu, m6502_device::IRQ_LINE);

	ym2203_device &ym1(YM2203(config, "ym1", 1'500'000));
	ym1.irq_handler().set(m_soundirq, FUNC(input_merger_device::in_w<0>));
	ym1.add_route(0, "mono", 0.20);
	ym1.add_route(1, "mono", 0.20);
	ym1.add_route(2, "mono", 0.20);
	ym1.add_route(3, "mono", 0.40);

	ym3526_device &ym2(YM3526(config, "ym2", 3'000'000));
	ym2.irq_handler().set(m_soundirq, FUNC(input_merger_device::in_w<1>));
	ym2.add_route(ALL_OUTPUTS, "mono", 0.50);
}

void ghostb_state::garyoret(machine_config &config)
{
	// basic machine hardware
	HD6309E(config, m_maincpu, 3'000'000); // HD63C09EP
	m_maincpu->set_addrmap(AS_PROGRAM, &ghostb_state::garyoret_map);

	R65C02(config, m_audiocpu, 1'500'000);
	m_audiocpu->set_addrmap(AS_PROGRAM, &ghostb_state::oscar_s_map); // NMIs are caused by the main CPU

	I8751(config, m_mcu, 8_MHz_XTAL);
	m_mcu->port_in_cb<0>().set(FUNC(ghostb_state::i8751_port0_r));
	m_mcu->port_out_cb<0>().set(FUNC(ghostb_state::i8751_port0_w));
	m_mcu->port_in_cb<1>().set(FUNC(ghostb_state::i8751_port1_r));
	m_mcu->port_out_cb<1>().set(FUNC(ghostb_state::i8751_port1_w));
	m_mcu->port_out_cb<2>().set(FUNC(ghostb_state::mcu_to_main_w));
	m_mcu->port_in_cb<3>().set_ioport("COIN");

	config.set_perfect_quantum(m_mcu);

	// video hardware
	BUFFERED_SPRITERAM8(config, m_spriteram);

	DECO_KARNOVSPRITES(config, m_spritegen_krn, 0, m_palette, gfx_gondo_spr);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	set_screen_raw_params(config);
	m_screen->set_screen_update(FUNC(ghostb_state::screen_update_garyoret));
	m_screen->screen_vblank().set(m_nmigate, FUNC(input_merger_device::in_w<1>));
	m_screen->screen_vblank().append_inputline(m_mcu, MCS51_INT0_LINE);
	m_screen->set_palette(m_palette);

	INPUT_MERGER_ALL_HIGH(config, m_nmigate).output_handler().set_inputline(m_maincpu, INPUT_LINE_NMI);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_gondo);
	DECO_RMC3(config, m_palette, 0, 1024); // xxxxBBBBGGGGRRRR with custom weighting

	MCFG_VIDEO_START_OVERRIDE(ghostb_state,garyoret)

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	INPUT_MERGER_ANY_HIGH(config, m_soundirq);
	m_soundirq->output_handler().set_inputline(m_audiocpu, m6502_device::IRQ_LINE);

	ym2203_device &ym1(YM2203(config, "ym1", 1'500'000));
	ym1.irq_handler().set(m_soundirq, FUNC(input_merger_device::in_w<0>));
	ym1.add_route(0, "mono", 0.20);
	ym1.add_route(1, "mono", 0.20);
	ym1.add_route(2, "mono", 0.20);
	ym1.add_route(3, "mono", 0.40);

	ym3526_device &ym2(YM3526(config, "ym2", 3'000'000));
	ym2.irq_handler().set(m_soundirq, FUNC(input_merger_device::in_w<1>));
	ym2.add_route(ALL_OUTPUTS, "mono", 0.80);
}

void ghostb_state::ghostb(machine_config &config)
{
	// basic machine hardware
	HD6309E(config, m_maincpu, 12_MHz_XTAL / 4); // HD63C09EP, clock verified
	m_maincpu->set_addrmap(AS_PROGRAM, &ghostb_state::ghostb_map);

	DECO_222(config, m_audiocpu, 12_MHz_XTAL / 8); // also seen with stock M6502, clock verified
	m_audiocpu->set_addrmap(AS_PROGRAM, &ghostb_state::dec8_s_map); // NMIs are caused by the main CPU

	I8751(config, m_mcu, 8_MHz_XTAL); // 8.0MHz OSC next to MCU - clock verified
	m_mcu->port_in_cb<0>().set(FUNC(ghostb_state::i8751_port0_r));
	m_mcu->port_out_cb<0>().set(FUNC(ghostb_state::i8751_port0_w));
	m_mcu->port_in_cb<1>().set(FUNC(ghostb_state::i8751_port1_r));
	m_mcu->port_out_cb<1>().set(FUNC(ghostb_state::i8751_port1_w));
	m_mcu->port_out_cb<2>().set(FUNC(ghostb_state::mcu_to_main_w));
	m_mcu->port_in_cb<3>().set_ioport("COIN");

	config.set_perfect_quantum(m_mcu);

	// video hardware
	BUFFERED_SPRITERAM8(config, m_spriteram);

	DECO_BAC06(config, m_tilegen[0], 0);
	m_tilegen[0]->set_gfx_region_wide(1, 1, 0);
	m_tilegen[0]->set_gfxdecode_tag(m_gfxdecode);

	DECO_KARNOVSPRITES(config, m_spritegen_krn, 0, m_palette, gfx_shackled_spr);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	set_screen_raw_params(config);
	m_screen->set_screen_update(FUNC(ghostb_state::screen_update_ghostb));
	m_screen->screen_vblank().set(m_nmigate, FUNC(input_merger_device::in_w<1>));
	m_screen->screen_vblank().append_inputline(m_mcu, MCS51_INT0_LINE);
	m_screen->set_palette(m_palette);

	INPUT_MERGER_ALL_HIGH(config, m_nmigate).output_handler().set_inputline(m_maincpu, INPUT_LINE_NMI);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_ghostb);
	DECO_RMC3(config, m_palette, 0, 1024); // xxxxBBBBGGGGRRRR with custom weighting
	m_palette->set_prom_region("proms");
	m_palette->set_init(m_palette, FUNC(deco_rmc3_device::palette_init_proms));
	MCFG_VIDEO_START_OVERRIDE(ghostb_state,ghostb)

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	INPUT_MERGER_ANY_HIGH(config, m_soundirq);
	m_soundirq->output_handler().set_inputline(m_audiocpu, m6502_device::IRQ_LINE);

	ym2203_device &ym1(YM2203(config, "ym1", 12_MHz_XTAL / 8)); // clock verified
	ym1.irq_handler().set(m_soundirq, FUNC(input_merger_device::in_w<0>));
	ym1.add_route(0, "mono", 0.20);
	ym1.add_route(1, "mono", 0.20);
	ym1.add_route(2, "mono", 0.20);
	ym1.add_route(3, "mono", 0.40);

	ym3812_device &ym2(YM3812(config, "ym2", 12_MHz_XTAL / 4)); // clock verified
	ym2.irq_handler().set(m_soundirq, FUNC(input_merger_device::in_w<1>));
	ym2.add_route(ALL_OUTPUTS, "mono", 0.80);
}

void ghostb_state::meikyuh(machine_config &config)
{
	ghostb(config);

	R65C02(config.replace(), m_audiocpu, 1'500'000);
	m_audiocpu->set_addrmap(AS_PROGRAM, &ghostb_state::dec8_s_map);
}


void csilver_state::csilver(machine_config &config)
{
	// basic machine hardware
	MC6809E(config, m_maincpu, 12_MHz_XTAL / 8); // verified on pcb
	m_maincpu->set_addrmap(AS_PROGRAM, &csilver_state::main_map);

	MC6809E(config, m_subcpu, 12_MHz_XTAL / 8); // verified on pcb
	m_subcpu->set_addrmap(AS_PROGRAM, &csilver_state::sub_map);

	R65C02(config, m_audiocpu, 12_MHz_XTAL / 8); // verified on pcb
	m_audiocpu->set_addrmap(AS_PROGRAM, &csilver_state::sound_map); // NMIs are caused by the main CPU

	config.set_perfect_quantum(m_mcu);

	I8751(config, m_mcu, 8_MHz_XTAL);
	m_mcu->port_in_cb<0>().set(FUNC(csilver_state::i8751_port0_r));
	m_mcu->port_out_cb<0>().set(FUNC(csilver_state::i8751_port0_w));
	m_mcu->port_in_cb<1>().set(FUNC(csilver_state::i8751_port1_r));
	m_mcu->port_out_cb<1>().set(FUNC(csilver_state::i8751_port1_w));
	m_mcu->port_out_cb<2>().set(FUNC(csilver_state::mcu_to_main_w));
	m_mcu->port_in_cb<3>().set_ioport("COIN");

	config.set_perfect_quantum(m_mcu);

	// video hardware
	BUFFERED_SPRITERAM8(config, m_spriteram);

	DECO_KARNOVSPRITES(config, m_spritegen_krn, 0, m_palette, gfx_shackled_spr);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	set_screen_raw_params(config);
	m_screen->set_screen_update(FUNC(csilver_state::screen_update_lastmisn));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set_inputline(m_subcpu, INPUT_LINE_NMI);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_shackled);
	DECO_RMC3(config, m_palette, 0, 1024); // xxxxBBBBGGGGRRRR with custom weighting

	MCFG_VIDEO_START_OVERRIDE(csilver_state,lastmisn)

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	ym2203_device &ym1(YM2203(config, "ym1", 12_MHz_XTAL / 8)); // verified on pcb
	ym1.add_route(0, "mono", 0.10);
	ym1.add_route(1, "mono", 0.10);
	ym1.add_route(2, "mono", 0.10);
	ym1.add_route(3, "mono", 0.20);

	ym3526_device &ym2(YM3526(config, "ym2", 12_MHz_XTAL / 4)); // verified on pcb
	ym2.add_route(ALL_OUTPUTS, "mono", 0.40);

	MSM5205(config, m_msm, 384_kHz_XTAL); // verified on pcb
	m_msm->vck_legacy_callback().set(FUNC(csilver_state::adpcm_int)); // interrupt function
	m_msm->set_prescaler_selector(msm5205_device::S48_4B); // 8KHz
	m_msm->add_route(ALL_OUTPUTS, "mono", 0.80);
}

void oscar_state::oscar(machine_config &config)
{
	// basic machine hardware
	MC6809E(config, m_maincpu, 12_MHz_XTAL / 8); // PCB seen both HD6309EP or MC6809EP, clock verified on pcb
	m_maincpu->set_addrmap(AS_PROGRAM, &oscar_state::oscar_map);

	MC6809E(config, m_subcpu, 12_MHz_XTAL / 8); // PCB seen both HD6309EP or MC6809EP, clock verified on pcb
	m_subcpu->set_addrmap(AS_PROGRAM, &oscar_state::oscar_sub_map);

	DECO_222(config, m_audiocpu, 12_MHz_XTAL / 8); // IC labeled "C10707-1"
	m_audiocpu->set_addrmap(AS_PROGRAM, &oscar_state::oscar_s_map); // NMIs are caused by the main CPU

	config.set_maximum_quantum(attotime::from_hz(6000));

	INPUT_MERGER_ANY_LOW(config, "coin").output_handler().set(FUNC(oscar_state::coin_irq)); // 1S1588 x3 (D1-D3) + RCDM-I5

	// video hardware
	BUFFERED_SPRITERAM8(config, m_spriteram);

	DECO_BAC06(config, m_tilegen[0], 0);
	m_tilegen[0]->set_gfx_region_wide(1, 1, 0);
	m_tilegen[0]->set_gfxdecode_tag(m_gfxdecode);
	m_tilegen[0]->set_tile_callback(FUNC(oscar_state::oscar_tile_cb));

	DECO_MXC06(config, m_spritegen_mxc, 0, m_palette, gfx_oscar_spr);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	set_screen_raw_params(config);
	m_screen->set_screen_update(FUNC(oscar_state::screen_update_oscar));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_oscar);
	DECO_RMC3(config, m_palette, 0, 1024); // xxxxBBBBGGGGRRRR with custom weighting

	MCFG_VIDEO_START_OVERRIDE(oscar_state,oscar)

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	INPUT_MERGER_ANY_HIGH(config, m_soundirq);
	m_soundirq->output_handler().set_inputline(m_audiocpu, m6502_device::IRQ_LINE);

	ym2203_device &ym1(YM2203(config, "ym1", 12_MHz_XTAL / 8)); // verified on pcb
	ym1.irq_handler().set(m_soundirq, FUNC(input_merger_device::in_w<0>));
	ym1.add_route(0, "mono", 0.20);
	ym1.add_route(1, "mono", 0.20);
	ym1.add_route(2, "mono", 0.20);
	ym1.add_route(3, "mono", 0.40);

	ym3526_device &ym2(YM3526(config, "ym2", 12_MHz_XTAL / 4)); // verified on pcb
	ym2.irq_handler().set(m_soundirq, FUNC(input_merger_device::in_w<1>));
	ym2.add_route(ALL_OUTPUTS, "mono", 0.80);
}

void oscar_state::oscarbl(machine_config &config)
{
	oscar(config);

	R65C02(config.replace(), m_audiocpu, 12_MHz_XTAL / 8);
	m_audiocpu->set_addrmap(AS_PROGRAM, &oscar_state::oscar_s_map); // NMIs are caused by the main CPU
	m_audiocpu->set_addrmap(AS_OPCODES, &oscar_state::oscarbl_s_opcodes_map);
}

void srdarwin_state::srdarwin(machine_config &config)
{
	// basic machine hardware
	MC6809E(config, m_maincpu, 1'500'000); // MC68A09EP or HD63?09EP
	m_maincpu->set_addrmap(AS_PROGRAM, &srdarwin_state::main_map);

	DECO_222(config, m_audiocpu, 1'500'000);
	m_audiocpu->set_addrmap(AS_PROGRAM, &srdarwin_state::dec8_s_map); // NMIs are caused by the main CPU

	I8751(config, m_mcu, 8_MHz_XTAL); // unknown frequency
	m_mcu->port_in_cb<0>().set(FUNC(srdarwin_state::i8751_port0_r));
	m_mcu->port_out_cb<0>().set(FUNC(srdarwin_state::i8751_port0_w));
	m_mcu->port_out_cb<2>().set(FUNC(srdarwin_state::mcu_to_main_w));
	m_mcu->port_in_cb<3>().set_ioport("COIN");

	config.set_perfect_quantum(m_mcu);

	// video hardware
	BUFFERED_SPRITERAM8(config, m_spriteram);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	set_screen_raw_params(config);
	m_screen->set_screen_update(FUNC(srdarwin_state::screen_update));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set_inputline(m_maincpu, INPUT_LINE_NMI);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_srdarwin);
	DECO_RMC3(config, m_palette, 0, 144); // xxxxBBBBGGGGRRRR with custom weighting

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	INPUT_MERGER_ANY_HIGH(config, m_soundirq);
	m_soundirq->output_handler().set_inputline(m_audiocpu, m6502_device::IRQ_LINE);

	ym2203_device &ym1(YM2203(config, "ym1", 1'500'000));
	ym1.irq_handler().set(m_soundirq, FUNC(input_merger_device::in_w<0>));
	ym1.add_route(0, "mono", 0.20);
	ym1.add_route(1, "mono", 0.20);
	ym1.add_route(2, "mono", 0.20);
	ym1.add_route(3, "mono", 0.40);

	ym3812_device &ym2(YM3812(config, "ym2", 3'000'000));
	ym2.irq_handler().set(m_soundirq, FUNC(input_merger_device::in_w<1>));
	ym2.add_route(ALL_OUTPUTS, "mono", 0.80);
}

void srdarwin_state::srdarwinb(machine_config &config)
{
	srdarwin(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &srdarwin_state::srdarwinb_map);

	config.device_remove("mcu");
	config.set_perfect_quantum("");
}

void oscar_state::cobracom(machine_config &config)
{
	// basic machine hardware
	MC6809E(config, m_maincpu, 1'500'000); // MC68B09EP
	m_maincpu->set_addrmap(AS_PROGRAM, &oscar_state::cobra_map);

	R65C02(config, m_audiocpu, 1'500'000);
	m_audiocpu->set_addrmap(AS_PROGRAM, &oscar_state::dec8_s_map); // NMIs are caused by the main CPU

	// video hardware
	BUFFERED_SPRITERAM8(config, m_spriteram);

	DECO_BAC06(config, m_tilegen[0], 0);
	m_tilegen[0]->set_gfx_region_wide(1, 1, 0);
	m_tilegen[0]->set_gfxdecode_tag(m_gfxdecode);

	DECO_BAC06(config, m_tilegen[1], 0);
	m_tilegen[1]->set_gfx_region_wide(2, 2, 0);
	m_tilegen[1]->set_gfxdecode_tag(m_gfxdecode);

	DECO_MXC06(config, m_spritegen_mxc, 0, m_palette, gfx_cobracom_spr);
	m_spritegen_mxc->set_colpri_callback(FUNC(oscar_state::cobracom_colpri_cb));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	set_screen_raw_params(config);
	m_screen->set_screen_update(FUNC(oscar_state::screen_update_cobracom));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set_inputline(m_maincpu, INPUT_LINE_NMI);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_cobracom);
	DECO_RMC3(config, m_palette, 0, 256); // xxxxBBBBGGGGRRRR with custom weighting

	MCFG_VIDEO_START_OVERRIDE(oscar_state,cobracom)

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	INPUT_MERGER_ANY_HIGH(config, m_soundirq);
	m_soundirq->output_handler().set_inputline(m_audiocpu, m6502_device::IRQ_LINE);

	ym2203_device &ym1(YM2203(config, "ym1", 1'500'000));
	ym1.irq_handler().set(m_soundirq, FUNC(input_merger_device::in_w<0>));
	ym1.add_route(0, "mono", 0.20);
	ym1.add_route(1, "mono", 0.20);
	ym1.add_route(2, "mono", 0.20);
	ym1.add_route(3, "mono", 0.40);

	ym3812_device &ym2(YM3812(config, "ym2", 3'000'000));
	ym2.irq_handler().set(m_soundirq, FUNC(input_merger_device::in_w<1>));
	ym2.add_route(ALL_OUTPUTS, "mono", 0.80);
}


/******************************************************************************/

ROM_START( lastmisn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "last_mission_dl03-8.13h", 0x08000, 0x08000, CRC(a4f8d54b) SHA1(4525826fa5d12c22e0f3bc1c3a9673b86a34aad1) ) // Rev 8 roms

	ROM_REGION( 0x10000, "mainbank", 0 )
	ROM_LOAD( "last_mission_dl04-5.7h",  0x00000, 0x10000, CRC(7dea1552) SHA1(920684413e2ba4313111e79821c5714977b26b1a) )

	ROM_REGION( 0x10000, "sub", 0 ) // CPU 2, 1st 16k is empty
	ROM_LOAD( "last_mission_dl02-5.18h", 0x0000, 0x10000, CRC(ec9b5daf) SHA1(86d47bad123676abc82dd7c92943878c54c33075) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "last_mission_dl05-.5h",   0x8000, 0x8000, CRC(1a5df8c0) SHA1(83d36b1d5fb87f50c44f3110804d6bbdbbc0da99) )

	ROM_REGION( 0x1000, "mcu", 0 ) // i8751 microcontroller
	ROM_LOAD( "last_mission_dl00-e.18a", 0x0000, 0x1000, CRC(e97481c6) SHA1(5c6b0e3585712c03b1b657c814c502c396ffa333) BAD_DUMP ) // not verified to be the same data as the "A" MCU dump

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "last_mission_dl01-.2a",   0x00000, 0x2000, CRC(f3787a5d) SHA1(3701df42cb2aca951963703e72c6c7b272eed82b) )
	ROM_CONTINUE(                        0x06000, 0x2000 )
	ROM_CONTINUE(                        0x04000, 0x2000 )
	ROM_CONTINUE(                        0x02000, 0x2000 )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "last_mission_dl11-.13f",  0x00000, 0x08000, CRC(36579d3b) SHA1(8edf952dafcd5bc66e08074687f0bec809fd4c2f) )
	ROM_LOAD( "last_mission_dl12-.9f",   0x20000, 0x08000, CRC(2ba6737e) SHA1(c5e4c27726bf14e9cd60d62e2f17ea5be8093c37) )
	ROM_LOAD( "last_mission_dl13-.8f",   0x40000, 0x08000, CRC(39a7dc93) SHA1(3b7968fd06ac0379525c1d3e73f8bbe18ea36439) )
	ROM_LOAD( "last_mission_dl10-.16f",  0x60000, 0x08000, CRC(fe275ea8) SHA1(2f089f96583235f1f5226ef2a64b430d84efbeee) )

	ROM_REGION( 0x80000, "tiles1", 0 )
	ROM_LOAD( "last_mission_dl09-.12k",  0x00000, 0x10000, CRC(6a5a0c5d) SHA1(0106cf693c284be5faf96e56b651fab92a410915) )
	ROM_LOAD( "last_mission_dl08-.14k",  0x20000, 0x10000, CRC(3b38cfce) SHA1(d6829bed6916fb301c08031bd466ee4dcc05b275) )
	ROM_LOAD( "last_mission_dl07-.15k",  0x40000, 0x10000, CRC(1b60604d) SHA1(1ee15cfdac87f7eeb92050766293b894cfad1466) )
	ROM_LOAD( "last_mission_dl06-.17k",  0x60000, 0x10000, CRC(c43c26a7) SHA1(896e278935b100edc12cd970469f2e8293eb96cc) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "dl-14.9c",    0x0000, 0x0100, CRC(2e55aa12) SHA1(c0f2b9649467eb9d2c1e47589b5990f5c5e8cc93) ) // Priority (Not yet used)
ROM_END

ROM_START( lastmisnu6 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "last_mission_dl03-6.13h", 0x08000, 0x08000, CRC(47751a5e) SHA1(190970a6eb849781e8853f2bed7b34ac44e569ca) ) // Rev 6 roms

	ROM_REGION( 0x10000, "mainbank", 0 )
	ROM_LOAD( "last_mission_dl04-5.7h",  0x00000, 0x10000, CRC(7dea1552) SHA1(920684413e2ba4313111e79821c5714977b26b1a) )

	ROM_REGION( 0x10000, "sub", 0 ) // CPU 2, 1st 16k is empty
	ROM_LOAD( "last_mission_dl02-5.18h", 0x0000, 0x10000, CRC(ec9b5daf) SHA1(86d47bad123676abc82dd7c92943878c54c33075) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "last_mission_dl05-.5h",   0x8000, 0x8000, CRC(1a5df8c0) SHA1(83d36b1d5fb87f50c44f3110804d6bbdbbc0da99) )

	ROM_REGION( 0x1000, "mcu", 0 ) // i8751 microcontroller
	ROM_LOAD( "last_mission_dl00-a.18a", 0x0000, 0x1000, CRC(e97481c6) SHA1(5c6b0e3585712c03b1b657c814c502c396ffa333) ) // Hand written "A", some MCUs are known to be labeled DL00-7, it's not verified to be the same data

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "last_mission_dl01-.2a",   0x00000, 0x2000, CRC(f3787a5d) SHA1(3701df42cb2aca951963703e72c6c7b272eed82b) )
	ROM_CONTINUE(                        0x06000, 0x2000 )
	ROM_CONTINUE(                        0x04000, 0x2000 )
	ROM_CONTINUE(                        0x02000, 0x2000 )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "last_mission_dl11-.13f",  0x00000, 0x08000, CRC(36579d3b) SHA1(8edf952dafcd5bc66e08074687f0bec809fd4c2f) )
	ROM_LOAD( "last_mission_dl12-.9f",   0x20000, 0x08000, CRC(2ba6737e) SHA1(c5e4c27726bf14e9cd60d62e2f17ea5be8093c37) )
	ROM_LOAD( "last_mission_dl13-.8f",   0x40000, 0x08000, CRC(39a7dc93) SHA1(3b7968fd06ac0379525c1d3e73f8bbe18ea36439) )
	ROM_LOAD( "last_mission_dl10-.16f",  0x60000, 0x08000, CRC(fe275ea8) SHA1(2f089f96583235f1f5226ef2a64b430d84efbeee) )

	ROM_REGION( 0x80000, "tiles1", 0 )
	ROM_LOAD( "last_mission_dl09-.12k",  0x00000, 0x10000, CRC(6a5a0c5d) SHA1(0106cf693c284be5faf96e56b651fab92a410915) )
	ROM_LOAD( "last_mission_dl08-.14k",  0x20000, 0x10000, CRC(3b38cfce) SHA1(d6829bed6916fb301c08031bd466ee4dcc05b275) )
	ROM_LOAD( "last_mission_dl07-.15k",  0x40000, 0x10000, CRC(1b60604d) SHA1(1ee15cfdac87f7eeb92050766293b894cfad1466) )
	ROM_LOAD( "last_mission_dl06-.17k",  0x60000, 0x10000, CRC(c43c26a7) SHA1(896e278935b100edc12cd970469f2e8293eb96cc) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "dl-14.9c",    0x0000, 0x0100, CRC(2e55aa12) SHA1(c0f2b9649467eb9d2c1e47589b5990f5c5e8cc93) ) // Priority (Not yet used)
ROM_END

ROM_START( lastmisnu5 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "last_mission_dl03-5.13h", 0x08000, 0x08000, CRC(357f5f6b) SHA1(a114aac50db62a6bcb943681e517ad7c88ec47f4) ) // Rev 5 roms

	ROM_REGION( 0x10000, "mainbank", 0 )
	ROM_LOAD( "last_mission_dl04-5.7h",  0x00000, 0x10000, CRC(7dea1552) SHA1(920684413e2ba4313111e79821c5714977b26b1a) )

	ROM_REGION( 0x10000, "sub", 0 ) // CPU 2, 1st 16k is empty
	ROM_LOAD( "last_mission_dl02-5.18h", 0x0000, 0x10000, CRC(ec9b5daf) SHA1(86d47bad123676abc82dd7c92943878c54c33075) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "last_mission_dl05-.5h",   0x8000, 0x8000, CRC(1a5df8c0) SHA1(83d36b1d5fb87f50c44f3110804d6bbdbbc0da99) )

	ROM_REGION( 0x1000, "mcu", 0 ) // i8751 microcontroller
	ROM_LOAD( "last_mission_dl00-a.18a", 0x0000, 0x1000, CRC(e97481c6) SHA1(5c6b0e3585712c03b1b657c814c502c396ffa333) ) // Hand written "A", some MCUs are known to be labeled DL00-7, it's not verified to be the same data

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "last_mission_dl01-.2a",   0x00000, 0x2000, CRC(f3787a5d) SHA1(3701df42cb2aca951963703e72c6c7b272eed82b) )
	ROM_CONTINUE(                        0x06000, 0x2000 )
	ROM_CONTINUE(                        0x04000, 0x2000 )
	ROM_CONTINUE(                        0x02000, 0x2000 )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "last_mission_dl11-.13f",  0x00000, 0x08000, CRC(36579d3b) SHA1(8edf952dafcd5bc66e08074687f0bec809fd4c2f) )
	ROM_LOAD( "last_mission_dl12-.9f",   0x20000, 0x08000, CRC(2ba6737e) SHA1(c5e4c27726bf14e9cd60d62e2f17ea5be8093c37) )
	ROM_LOAD( "last_mission_dl13-.8f",   0x40000, 0x08000, CRC(39a7dc93) SHA1(3b7968fd06ac0379525c1d3e73f8bbe18ea36439) )
	ROM_LOAD( "last_mission_dl10-.16f",  0x60000, 0x08000, CRC(fe275ea8) SHA1(2f089f96583235f1f5226ef2a64b430d84efbeee) )

	ROM_REGION( 0x80000, "tiles1", 0 )
	ROM_LOAD( "last_mission_dl09-.12k",  0x00000, 0x10000, CRC(6a5a0c5d) SHA1(0106cf693c284be5faf96e56b651fab92a410915) )
	ROM_LOAD( "last_mission_dl08-.14k",  0x20000, 0x10000, CRC(3b38cfce) SHA1(d6829bed6916fb301c08031bd466ee4dcc05b275) )
	ROM_LOAD( "last_mission_dl07-.15k",  0x40000, 0x10000, CRC(1b60604d) SHA1(1ee15cfdac87f7eeb92050766293b894cfad1466) )
	ROM_LOAD( "last_mission_dl06-.17k",  0x60000, 0x10000, CRC(c43c26a7) SHA1(896e278935b100edc12cd970469f2e8293eb96cc) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "dl-14.9c",    0x00000, 0x0100, CRC(2e55aa12) SHA1(c0f2b9649467eb9d2c1e47589b5990f5c5e8cc93) ) // Priority (Not yet used)
ROM_END

ROM_START( lastmisnj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dl03-.13h",   0x08000, 0x08000, CRC(4be5e7e1) SHA1(9f943658663da31947cebdcbcb5f4e2be0714c06) )

	ROM_REGION( 0x10000, "mainbank", 0 )
	ROM_LOAD( "dl04-.7h",    0x00000, 0x10000, CRC(f026adf9) SHA1(4ccd0e714a6eb7cee388c93beee2d5510407c961) )

	ROM_REGION( 0x10000, "sub", 0 ) // CPU 2, 1st 16k is empty
	ROM_LOAD( "dl02-.18h",   0x0000, 0x10000, CRC(d0de2b5d) SHA1(e0bb34c2a2ef6fc6f05ab9a98bd23a39004c0c05) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "last_mission_dl05-.5h",  0x8000, 0x8000, CRC(1a5df8c0) SHA1(83d36b1d5fb87f50c44f3110804d6bbdbbc0da99) )

	ROM_REGION( 0x1000, "mcu", 0 ) // created from dump of the US version
	ROM_LOAD( "last_mission_japan.18a", 0x0000, 0x1000, BAD_DUMP CRC(0d58c3a1) SHA1(184e75324b7ab2de8e6441f0c954046db80b2640) ) // correct ROM label when real MCU is dumped

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "last_mission_dl01-.2a",    0x00000, 0x2000, CRC(f3787a5d) SHA1(3701df42cb2aca951963703e72c6c7b272eed82b) )
	ROM_CONTINUE(                         0x06000, 0x2000 )
	ROM_CONTINUE(                         0x04000, 0x2000 )
	ROM_CONTINUE(                         0x02000, 0x2000 )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "last_mission_dl11-.13f",   0x00000, 0x08000, CRC(36579d3b) SHA1(8edf952dafcd5bc66e08074687f0bec809fd4c2f) )
	ROM_LOAD( "last_mission_dl12-.9f",    0x20000, 0x08000, CRC(2ba6737e) SHA1(c5e4c27726bf14e9cd60d62e2f17ea5be8093c37) )
	ROM_LOAD( "last_mission_dl13-.8f",    0x40000, 0x08000, CRC(39a7dc93) SHA1(3b7968fd06ac0379525c1d3e73f8bbe18ea36439) )
	ROM_LOAD( "last_mission_dl10-.16f",   0x60000, 0x08000, CRC(fe275ea8) SHA1(2f089f96583235f1f5226ef2a64b430d84efbeee) )

	ROM_REGION( 0x80000, "tiles1", 0 )
	ROM_LOAD( "last_mission_dl09-.12k",   0x00000, 0x10000, CRC(6a5a0c5d) SHA1(0106cf693c284be5faf96e56b651fab92a410915) )
	ROM_LOAD( "last_mission_dl08-.14k",   0x20000, 0x10000, CRC(3b38cfce) SHA1(d6829bed6916fb301c08031bd466ee4dcc05b275) )
	ROM_LOAD( "last_mission_dl07-.15k",   0x40000, 0x10000, CRC(1b60604d) SHA1(1ee15cfdac87f7eeb92050766293b894cfad1466) )
	ROM_LOAD( "last_mission_dl06-.17k",   0x60000, 0x10000, CRC(c43c26a7) SHA1(896e278935b100edc12cd970469f2e8293eb96cc) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "dl-14.9c",    0x0000, 0x0100, CRC(2e55aa12) SHA1(c0f2b9649467eb9d2c1e47589b5990f5c5e8cc93) ) // Priority (Not yet used)
ROM_END

ROM_START( shackled )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dk-02.13h", 0x08000, 0x08000, CRC(87f8fa85) SHA1(1cb93a60eefdb453a3cc6ec9c5cc2e367fb8aeb0) )

	ROM_REGION( 0x40000, "mainbank", 0 )
	ROM_LOAD( "dk-06.7h",  0x00000, 0x10000, CRC(69ad62d1) SHA1(1aa23b12ab4f1908cddd25f091e1f9bd70a5113c) )
	ROM_LOAD( "dk-05.9h",  0x10000, 0x10000, CRC(598dd128) SHA1(10843c5352eef03c8675df6abaf23c9c9c795aa3) )
	ROM_LOAD( "dk-04.10h", 0x20000, 0x10000, CRC(36d305d4) SHA1(17586c316aff405cf20c1467d69c98fa2a3c2630) )
	ROM_LOAD( "dk-03.11h", 0x30000, 0x08000, CRC(6fd90fd1) SHA1(2f8db17e5545c82d243a7e23e7bda2c2a9101360) )
	ROM_RELOAD(            0x38000, 0x08000 )

	ROM_REGION( 0x10000, "sub", 0 ) // CPU 2, 1st 16k is empty
	ROM_LOAD( "dk-01.18h", 0x00000, 0x10000, CRC(71fe3bda) SHA1(959cce01362b2c670c2e15b03a78a1ff9cea4ee9) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "dk-07.5h", 0x08000, 0x08000, CRC(887e4bcc) SHA1(6427396080e9cd8647adff47c8ed04593a14268c) )

	ROM_REGION( 0x1000, "mcu", 0 ) // ID8751H (fake) MCU (based on 'breywood' with ID byte changed from 00 to 01)
	ROM_LOAD( "dk-e.18a", 0x0000, 0x1000, CRC(1af06149) SHA1(b9cb2a4986dbcfc78b0cbea2c1e2bdac1db479cd) BAD_DUMP ) // Hand written "E"

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "dk-00.2a", 0x00000, 0x08000, CRC(69b975aa) SHA1(38cb96768c79ff1aa1b4b190e08ec9155baf698a) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "dk-12.15k", 0x00000, 0x10000, CRC(615c2371) SHA1(30b25dc27d34646d886a465c77622eaa894d83c3) )
	ROM_LOAD( "dk-13.14k", 0x10000, 0x10000, CRC(479aa503) SHA1(1167f0d15439c95a1094f81855203e863ce0488d) )
	ROM_LOAD( "dk-14.13k", 0x20000, 0x10000, CRC(cdc24246) SHA1(1a4189bc2b1fa99740dd7921608159936ba3bd07) )
	ROM_LOAD( "dk-15.11k", 0x30000, 0x10000, CRC(88db811b) SHA1(7d3c4a80925f323efb589798b4a341d1a2ca95f9) )
	ROM_LOAD( "dk-16.10k", 0x40000, 0x10000, CRC(061a76bd) SHA1(5bcb513e48bed9b7c4207d94531be691a85e295d) )
	ROM_LOAD( "dk-17.9k",  0x50000, 0x10000, CRC(a6c5d8af) SHA1(58f3fece9a5ef8b39090a2f39610381b8e7cdbf7) )
	ROM_LOAD( "dk-18.8k",  0x60000, 0x10000, CRC(4d466757) SHA1(701d79bebbba4f266e19080d16ff2f93ffa94287) )
	ROM_LOAD( "dk-19.6k",  0x70000, 0x10000, CRC(1911e83e) SHA1(174e9db3f2211ecbbb93c6bda8f6185dbfdbc818) )

	ROM_REGION( 0x80000, "tiles1", 0 )
	ROM_LOAD( "dk-11.12k", 0x00000, 0x10000, CRC(5cf5719f) SHA1(8c7582ac19010421ec748391a193aa18e51b981f) )
	ROM_LOAD( "dk-10.14k", 0x20000, 0x10000, CRC(408e6d08) SHA1(28cb76792e5f84bd101a91cb82597a5939804f84) )
	ROM_LOAD( "dk-09.15k", 0x40000, 0x10000, CRC(c1557fac) SHA1(7d39ec793113a48baf45c2ea07abb07e2e48985a) )
	ROM_LOAD( "dk-08.17k", 0x60000, 0x10000, CRC(5e54e9f5) SHA1(1ab41a3bde1f2c2be670e89cf402be28001c17d1) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "dk-20.9c", 0x0000, 0x0100, CRC(ff3cd588) SHA1(7360a9f046d517885d456d89026d047fb1fd8d5a) ) // Priority (Not yet used) BPROM type MB7052
ROM_END

ROM_START( breywood )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dj02-2.13h", 0x08000, 0x08000, CRC(c19856b9) SHA1(766994703bb59879c311675353d7231ad27c7c16) )

	ROM_REGION( 0x40000, "mainbank", 0 )
	ROM_LOAD( "dj06-2.7h",  0x00000, 0x10000, CRC(2860ea02) SHA1(7ac090c3ae9d71baa6227ec9555f1c9f2d25ea0d) )
	ROM_LOAD( "dj05-2.9h",  0x10000, 0x10000, CRC(0fdd915e) SHA1(262df956dfc727c710ade28af7f33fddaafd7ee2) )
	ROM_LOAD( "dj04-2.10h", 0x20000, 0x10000, CRC(71036579) SHA1(c58ff3222b5bcd75d58c5f282554e92103e80916) )
	ROM_LOAD( "dj03-2.11h", 0x30000, 0x08000, CRC(308f4893) SHA1(539c138ff01c5718cc8a982482b989468d532699) )
	ROM_RELOAD(             0x38000, 0x08000 )

	ROM_REGION( 0x10000, "sub", 0 ) // CPU 2, 1st 16k is empty
	ROM_LOAD( "dj1-2y.18h", 0x0000, 0x10000, CRC(3d9fb623) SHA1(6e5eaad9bb0a432e2da5da5b18a2ed36617bdde2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "dj07-1.5h", 0x8000, 0x8000, CRC(4a471c38) SHA1(963ed7b6afeefdfc2cf0d65b0998f973330e6495) )

	ROM_REGION( 0x1000, "mcu", 0 ) // i8751 microcontroller
	ROM_LOAD( "dj.18a", 0x0000, 0x1000, CRC(4cb20332) SHA1(e0bbba7be22e7bcff82fb0ae441410e559ec4566) )

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "dj-00.2a",  0x00000, 0x08000, CRC(815a891a) SHA1(e557d6a35821a8589d9e3df0f42131b58b08c8ca) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "dj12.15k", 0x00000, 0x10000, CRC(2b7634f2) SHA1(56d963d4960d9b3e888c8107340763e176adfa9b) )
	ROM_LOAD( "dj13.14k", 0x10000, 0x10000, CRC(4530a952) SHA1(99251a21347815cba465669e18df31262bcdaba1) )
	ROM_LOAD( "dj14.13k", 0x20000, 0x10000, CRC(87c28833) SHA1(3f1a294065326389d304e540bc880844c6c7cb06) )
	ROM_LOAD( "dj15.11k", 0x30000, 0x10000, CRC(bfb43a4d) SHA1(56092935147a3b643a9b39eb7cfc067a764644c5) )
	ROM_LOAD( "dj16.10k", 0x40000, 0x10000, CRC(f9848cc4) SHA1(6d8e77b67ce4d418defba6f6979632f31d2307c6) )
	ROM_LOAD( "dj17.9k",  0x50000, 0x10000, CRC(baa3d218) SHA1(3c31df23cc871cffd9a4dafae106e4a98f5af848) )
	ROM_LOAD( "dj18.8k",  0x60000, 0x10000, CRC(12afe533) SHA1(6df3471c16a714d118717da549a7523aa388ddd3) )
	ROM_LOAD( "dj19.6k",  0x70000, 0x10000, CRC(03373755) SHA1(d2541dd957803168f246d96b7cd74eae7fd43188) )

	ROM_REGION( 0x80000, "tiles1", 0 )
	ROM_LOAD( "dj11.12k", 0x00000, 0x10000, CRC(067e2a43) SHA1(f1da7455aab21f94ed25a93b0ebfde69baa475d1) )
	ROM_LOAD( "dj10.14k", 0x20000, 0x10000, CRC(c19733aa) SHA1(3dfcfd33c5c4f792bb941ac933301c03ddd72b03) )
	ROM_LOAD( "dj09.15k", 0x40000, 0x10000, CRC(e37d5dbe) SHA1(ff79b4f6d8b0a3061e78d15480df0155650f347f) )
	ROM_LOAD( "dj08.17k", 0x60000, 0x10000, CRC(beee880f) SHA1(9a818a75cbec425a13f629bda6d50aa341aa1896) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "dk-20.9c", 0x0000, 0x0100, CRC(ff3cd588) SHA1(7360a9f046d517885d456d89026d047fb1fd8d5a) ) // Priority (Not yet used) BPROM type MB7052
ROM_END

ROM_START( gondo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dt00-e.f3", 0x08000, 0x08000, CRC(912a7eee) SHA1(15af89babf166dadaa77640e1973d7ebb4c078db) ) // Verified only DT00-E & DT03-E have the "-E" extention

	ROM_REGION( 0x40000, "mainbank", 0 )
	ROM_LOAD( "dt01.f5",   0x00000, 0x10000, CRC(c39bb877) SHA1(9beb59ba19f38417c5d4d36e8f3c41f2b017d2d6) )
	ROM_LOAD( "dt02.f6",   0x10000, 0x10000, CRC(925307a4) SHA1(1e8b8eb21df1a11b14c981b343b34c6cc3676517) ) // same label as the US version, but content identical to Japanese version
	ROM_LOAD( "dt03-e.f7", 0x20000, 0x10000, CRC(ee7475eb) SHA1(8c68198ea1c3e89c9c2c4ba0e5d3f47afb8eecd4) )
	ROM_FILL(              0x30000, 0x10000, 0xff)

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "dt05-e.h5", 0x8000, 0x8000, CRC(ec08aa29) SHA1(ce83974ae095d9518d1ebf9f7e712f0cbc2c1b42) )

	ROM_REGION( 0x1000, "mcu", 0 ) // i8751 microcontroller
	ROM_LOAD( "dt-e.b1", 0x0000, 0x1000, BAD_DUMP CRC(0d0532ec) SHA1(30894f69ff24c1be4b684e07729bbb3e0f353086) ) // hand-crafted from the US version

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "dt14-e.b18", 0x00000, 0x08000, CRC(00cbe9c8) SHA1(de7b640de8fd54ee79194945c96d5768d09f483b) ) // identical to Japanese version

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "dt19.f13",   0x00000, 0x10000, CRC(da2abe4b) SHA1(d53e4769671f3fd437edcff7e7ea05156bbcb45d) ) // All sprite data matches the Japanese set
	ROM_LOAD( "dt20-e.f15", 0x10000, 0x08000, CRC(0eef7f56) SHA1(05c23aa6a598478cd4822634cff96055c585e9d2) ) // Verified only DT17-E, DT18-E, DT20-E & DT22-E have the "-E" extention
	ROM_LOAD( "dt16.f9",    0x20000, 0x10000, CRC(e9955d8f) SHA1(aeef5e18f9d36c1bab3000e95205ce1b18cfbf0b) ) // DT15, DT16, DT19 & DT21 do NOT have the "-E" extention
	ROM_LOAD( "dt18-e.f12", 0x30000, 0x08000, CRC(2b2d1468) SHA1(a144ac1b367e1fec876156230e9ab1c99416962e) )
	ROM_LOAD( "dt15.f8",    0x40000, 0x10000, CRC(a54b2eb6) SHA1(25cb61f67135672154f1ad8e0c49ec04655e91de) )
	ROM_LOAD( "dt17-e.f11", 0x50000, 0x08000, CRC(75ae349a) SHA1(15755a28925d5ed37fab4bd988716fcc5d20c290) )
	ROM_LOAD( "dt21.f16",   0x60000, 0x10000, CRC(1c5f682d) SHA1(4b7022cce930a9e9a0087c91e8344269fe7ed889) )
	ROM_LOAD( "dt22-e.f18", 0x70000, 0x08000, CRC(c8ffb148) SHA1(ae1a8b3cd1f5e423dc1a3c7d05f9fe7c689432e3) )

	ROM_REGION( 0x80000, "tiles1", 0 )
	ROM_LOAD( "dt08.h10", 0x00000, 0x08000, CRC(aec483f5) SHA1(1d6de823ab0eeb9c89e9c227428ff278663627f3) ) // Tiles data is the same for all 3 regions
	ROM_CONTINUE(         0x10000, 0x08000 )
	ROM_LOAD( "dt09.h12", 0x08000, 0x08000, CRC(446f0ce0) SHA1(072b88d6de5aa0ed6b1d60c266bcf170dea927d5) )
	ROM_LOAD( "dt06.h7",  0x20000, 0x08000, CRC(3fe1527f) SHA1(b8df4bef2b1a879b65214025fc3b5998ef5c8886) )
	ROM_CONTINUE(         0x30000, 0x08000 )
	ROM_LOAD( "dt07.h9",  0x28000, 0x08000, CRC(61f9bce5) SHA1(ef8a5f5e4c66a143304bcab50ca87579f1507864) )
	ROM_LOAD( "dt12.h16", 0x40000, 0x08000, CRC(1a72ca8d) SHA1(f412758452cb3417e85c355ccb8794fde7edf1cc) )
	ROM_CONTINUE(         0x50000, 0x08000 )
	ROM_LOAD( "dt13.h18", 0x48000, 0x08000, CRC(ccb81aec) SHA1(56e524ed4373b7bd1074a0d22ff75ede379f1696) )
	ROM_LOAD( "dt10.h13", 0x60000, 0x08000, CRC(cfcfc9ed) SHA1(57f43d638cf864d68420f0203740be7bda9da5ca) )
	ROM_CONTINUE(         0x70000, 0x08000 )
	ROM_LOAD( "dt11.h15", 0x68000, 0x08000, CRC(53e9cf17) SHA1(8cbb45154a60f42f1b1e7299b12d2e92fc194df8) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "ds-23.b10", 0x0000, 0x0400, CRC(dcbfec4e) SHA1(a375caef4575746870e285d90ba991ea7daefad6) ) // BPROM type MB7122E for Priority (Not yet used)
ROM_END

ROM_START( gondou )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dt00.f3", 0x08000, 0x08000, CRC(a8cf9118) SHA1(865744c9866957d686a31608d356e279fe58934e) )

	ROM_REGION( 0x40000, "mainbank", 0 )
	ROM_LOAD( "dt01.f5", 0x00000, 0x10000, CRC(c39bb877) SHA1(9beb59ba19f38417c5d4d36e8f3c41f2b017d2d6) )
	ROM_LOAD( "dt02.f6", 0x10000, 0x10000, CRC(bb5e674b) SHA1(8057dc7464a8b6987536f248d607957923b223cf) )
	ROM_LOAD( "dt03.f7", 0x20000, 0x10000, CRC(99c32b13) SHA1(3d79f48e7d198cb2e519d592a89eda505044bce5) )
	ROM_FILL(            0x30000, 0x10000, 0xff)

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "dt05.h5", 0x8000, 0x8000, CRC(ec08aa29) SHA1(ce83974ae095d9518d1ebf9f7e712f0cbc2c1b42) ) // == dt05-e.h5

	ROM_REGION( 0x1000, "mcu", 0 ) // i8751 microcontroller
	ROM_LOAD( "dt-a.b1", 0x0000, 0x1000, CRC(03abceeb) SHA1(a16b779d7cea1c1437f85fa6b6e08894a46a5674) )

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "dt14.b18", 0x00000, 0x08000, CRC(4bef16e1) SHA1(b8157a7a1b8f36cea1fd353267a4e03d920cb4aa) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "dt19.f13", 0x00000, 0x10000, CRC(da2abe4b) SHA1(d53e4769671f3fd437edcff7e7ea05156bbcb45d) )
	ROM_LOAD( "dt20.f15", 0x10000, 0x08000, CRC(42d01002) SHA1(5a289ffdc83c05f21908a5d0b6247da5b51c1ddd) ) // Unique data to the US set
	ROM_LOAD( "dt16.f9",  0x20000, 0x10000, CRC(e9955d8f) SHA1(aeef5e18f9d36c1bab3000e95205ce1b18cfbf0b) )
	ROM_LOAD( "dt18.f12", 0x30000, 0x08000, CRC(c0c5df1c) SHA1(5b0f71f590434cdd0545ce098666798927727469) ) // Unique data to the US set
	ROM_LOAD( "dt15.f8",  0x40000, 0x10000, CRC(a54b2eb6) SHA1(25cb61f67135672154f1ad8e0c49ec04655e91de) )
	ROM_LOAD( "dt17.f11", 0x50000, 0x08000, CRC(3bbcff0d) SHA1(a8f7aa56ff49ed6b29240c3504d6c9945944953b) ) // Unique data to the US set
	ROM_LOAD( "dt21.f16", 0x60000, 0x10000, CRC(1c5f682d) SHA1(4b7022cce930a9e9a0087c91e8344269fe7ed889) )
	ROM_LOAD( "dt22.f18", 0x70000, 0x08000, CRC(c1876a5f) SHA1(66122ce765723765e20036bd4d461a210c8b94d3) ) // Unique data to the US set

	ROM_REGION( 0x80000, "tiles1", 0 )
	ROM_LOAD( "dt08.h10", 0x00000, 0x08000, CRC(aec483f5) SHA1(1d6de823ab0eeb9c89e9c227428ff278663627f3) ) // Tiles data is the same for all 3 regions
	ROM_CONTINUE(         0x10000, 0x08000 )
	ROM_LOAD( "dt09.h12", 0x08000, 0x08000, CRC(446f0ce0) SHA1(072b88d6de5aa0ed6b1d60c266bcf170dea927d5) )
	ROM_LOAD( "dt06.h7",  0x20000, 0x08000, CRC(3fe1527f) SHA1(b8df4bef2b1a879b65214025fc3b5998ef5c8886) )
	ROM_CONTINUE(         0x30000, 0x08000 )
	ROM_LOAD( "dt07.h9",  0x28000, 0x08000, CRC(61f9bce5) SHA1(ef8a5f5e4c66a143304bcab50ca87579f1507864) )
	ROM_LOAD( "dt12.h16", 0x40000, 0x08000, CRC(1a72ca8d) SHA1(f412758452cb3417e85c355ccb8794fde7edf1cc) )
	ROM_CONTINUE(         0x50000, 0x08000 )
	ROM_LOAD( "dt13.h18", 0x48000, 0x08000, CRC(ccb81aec) SHA1(56e524ed4373b7bd1074a0d22ff75ede379f1696) )
	ROM_LOAD( "dt10.h13", 0x60000, 0x08000, CRC(cfcfc9ed) SHA1(57f43d638cf864d68420f0203740be7bda9da5ca) )
	ROM_CONTINUE(         0x70000, 0x08000 )
	ROM_LOAD( "dt11.h15", 0x68000, 0x08000, CRC(53e9cf17) SHA1(8cbb45154a60f42f1b1e7299b12d2e92fc194df8) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "ds-23.b10", 0x0000, 0x0400, CRC(dcbfec4e) SHA1(a375caef4575746870e285d90ba991ea7daefad6) ) // BPROM type MB7122E for Priority (Not yet used)
ROM_END

ROM_START( makyosen )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ds00.f3", 0x08000, 0x08000, CRC(33bb16fe) SHA1(5d3873b66e0d08b35d56a8b508c774b27368a100) )

	ROM_REGION( 0x40000, "mainbank", 0 )
	ROM_LOAD( "ds01.f5", 0x00000, 0x10000, CRC(c39bb877) SHA1(9beb59ba19f38417c5d4d36e8f3c41f2b017d2d6) )
	ROM_LOAD( "ds02.f6", 0x10000, 0x10000, CRC(925307a4) SHA1(1e8b8eb21df1a11b14c981b343b34c6cc3676517) )
	ROM_LOAD( "ds03.f7", 0x20000, 0x10000, CRC(9c0fcbf6) SHA1(bfe42b5277fea111840a9f59b2cb8dfe44444029) )
	ROM_FILL(            0x30000, 0x10000, 0xff)

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ds05.h5",  0x8000, 0x8000, CRC(e6e28ca9) SHA1(3b1f8219331db1910bfb428f8964f8fc1063df6f) ) // == dt05-e.h5

	ROM_REGION( 0x1000, "mcu", 0 ) // i8751 microcontroller
	ROM_LOAD( "ds.b1",  0x0000, 0x1000, CRC(08f36e35) SHA1(e8913da71704a89fad41d5bfba45682119166681) )

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "ds14.b18", 0x00000, 0x08000, CRC(00cbe9c8) SHA1(de7b640de8fd54ee79194945c96d5768d09f483b) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "ds19.f13", 0x00000, 0x10000, CRC(da2abe4b) SHA1(d53e4769671f3fd437edcff7e7ea05156bbcb45d) ) // == dt19.f13
	ROM_LOAD( "ds20.f15", 0x10000, 0x08000, CRC(0eef7f56) SHA1(05c23aa6a598478cd4822634cff96055c585e9d2) ) // == dt20-e.f15
	ROM_LOAD( "ds16.f9",  0x20000, 0x10000, CRC(e9955d8f) SHA1(aeef5e18f9d36c1bab3000e95205ce1b18cfbf0b) ) // == dt16.f9
	ROM_LOAD( "ds18.f12", 0x30000, 0x08000, CRC(2b2d1468) SHA1(a144ac1b367e1fec876156230e9ab1c99416962e) ) // == dt18-e.f12
	ROM_LOAD( "ds15.f8",  0x40000, 0x10000, CRC(a54b2eb6) SHA1(25cb61f67135672154f1ad8e0c49ec04655e91de) ) // == dt15.f8
	ROM_LOAD( "ds17.f11", 0x50000, 0x08000, CRC(75ae349a) SHA1(15755a28925d5ed37fab4bd988716fcc5d20c290) ) // == dt17-e.f11
	ROM_LOAD( "ds21.f16", 0x60000, 0x10000, CRC(1c5f682d) SHA1(4b7022cce930a9e9a0087c91e8344269fe7ed889) ) // == dt21.f16
	ROM_LOAD( "ds22.f18", 0x70000, 0x08000, CRC(c8ffb148) SHA1(ae1a8b3cd1f5e423dc1a3c7d05f9fe7c689432e3) ) // == dt22-e.f18

	ROM_REGION( 0x80000, "tiles1", 0 )
	ROM_LOAD( "ds08.h10", 0x00000, 0x08000, CRC(aec483f5) SHA1(1d6de823ab0eeb9c89e9c227428ff278663627f3) ) // Tiles data is the same for all 3 regions
	ROM_CONTINUE(         0x10000, 0x08000 )
	ROM_LOAD( "ds09.h12", 0x08000, 0x08000, CRC(446f0ce0) SHA1(072b88d6de5aa0ed6b1d60c266bcf170dea927d5) )
	ROM_LOAD( "ds06.h7",  0x20000, 0x08000, CRC(3fe1527f) SHA1(b8df4bef2b1a879b65214025fc3b5998ef5c8886) )
	ROM_CONTINUE(         0x30000, 0x08000 )
	ROM_LOAD( "ds07.h9",  0x28000, 0x08000, CRC(61f9bce5) SHA1(ef8a5f5e4c66a143304bcab50ca87579f1507864) )
	ROM_LOAD( "ds12.h16", 0x40000, 0x08000, CRC(1a72ca8d) SHA1(f412758452cb3417e85c355ccb8794fde7edf1cc) )
	ROM_CONTINUE(         0x50000, 0x08000 )
	ROM_LOAD( "ds13.h18", 0x48000, 0x08000, CRC(ccb81aec) SHA1(56e524ed4373b7bd1074a0d22ff75ede379f1696) )
	ROM_LOAD( "ds10.h13", 0x60000, 0x08000, CRC(cfcfc9ed) SHA1(57f43d638cf864d68420f0203740be7bda9da5ca) )
	ROM_CONTINUE(         0x70000, 0x08000 )
	ROM_LOAD( "ds11.h15", 0x68000, 0x08000, CRC(53e9cf17) SHA1(8cbb45154a60f42f1b1e7299b12d2e92fc194df8) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "ds-23.b10", 0x0000, 0x0400, CRC(dcbfec4e) SHA1(a375caef4575746870e285d90ba991ea7daefad6) ) // BPROM type MB7122E for Priority (Not yet used)

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "pal16r4nc.u10", 0x0000, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16r4nc.g11", 0x0200, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal16r4nc.s1",  0x0400, 0x0104, NO_DUMP ) // PAL is read protected
ROM_END

ROM_START( garyoret )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dv00", 0x08000, 0x08000, CRC(cceaaf05) SHA1(b8f54638feab77d023e01ced947e1269f0d19fd8) )

	ROM_REGION( 0x40000, "mainbank", 0 )
	ROM_LOAD( "dv01", 0x00000, 0x10000, CRC(c33fc18a) SHA1(0d9594b0e6c39aea5b9f15f6aa364b31604f1066) )
	ROM_LOAD( "dv02", 0x10000, 0x10000, CRC(f9e26ce7) SHA1(8589594ebc7ae16942739382273a222dfa30b3b7) )
	ROM_LOAD( "dv03", 0x20000, 0x10000, CRC(55d8d699) SHA1(da1519cd54d27cc406420ce0845e43f7228cfd4e) )
	ROM_LOAD( "dv04", 0x30000, 0x10000, CRC(ed3d00ee) SHA1(6daa2ee509235ad03d3012a00382820279add620) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "dv05", 0x08000, 0x08000, CRC(c97c347f) SHA1(a1b22733dc15d524af97db3e608a82503a49b182) )

	ROM_REGION( 0x1000, "mcu", 0 ) // ID8751H (fake) MCU based on 'gondo' one
	ROM_LOAD( "dv__.mcu", 0x0000, 0x1000, BAD_DUMP CRC(37cacec6) SHA1(d81fe36939ccac784a83a69dfc6898b22a4515ec) )

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "dv14", 0x00000, 0x08000, CRC(fb2bc581) SHA1(d597976c5ae586166c49051cc3de8cf0c2e5a5e1) ) // Characters

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "dv22", 0x00000, 0x10000, CRC(cef0367e) SHA1(8beb3a6b91ec0a6ec052243c8f626a581d349f65) )
	ROM_LOAD( "dv21", 0x10000, 0x08000, CRC(90042fb7) SHA1(f19bbf158c92030e8fddb5087b5b69b71956baf8) )
	ROM_LOAD( "dv20", 0x20000, 0x10000, CRC(451a2d8c) SHA1(f4eea444b797d394edeb514ddc1c494fd7ccc2f2) )
	ROM_LOAD( "dv19", 0x30000, 0x08000, CRC(14e1475b) SHA1(f0aec5b7b4be0da06a73ed382e7e851654e47e47) )
	ROM_LOAD( "dv18", 0x40000, 0x10000, CRC(7043bead) SHA1(5d1be8b9cd56ae43d60406b05258d20de980096d) )
	ROM_LOAD( "dv17", 0x50000, 0x08000, CRC(28f449d7) SHA1(cf1bc690b67910c42ad09531ab1d88461d00b944) )
	ROM_LOAD( "dv16", 0x60000, 0x10000, CRC(37e4971e) SHA1(9442c315b7cdbcc046d9e6838cb793c1857174ed) )
	ROM_LOAD( "dv15", 0x70000, 0x08000, CRC(ca41b6ac) SHA1(d7a9ef6c89741c1e8e17a668a9d39ea089f5c73f) )

	ROM_REGION( 0x80000, "tiles1", 0 )
	ROM_LOAD( "dv08", 0x00000, 0x08000, CRC(89c13e15) SHA1(6507e60de5cd78a5b46090e4825a44c2a23631d7) )
	ROM_CONTINUE(     0x10000, 0x08000 )
	ROM_LOAD( "dv09", 0x08000, 0x08000, CRC(6a345a23) SHA1(b86f81b9fe25acd833ca3e2cff6cfa853c02280a) )
	ROM_CONTINUE(     0x18000, 0x08000 )

	ROM_LOAD( "dv06", 0x20000, 0x08000, CRC(1eb52a20) SHA1(46670ed973f794be9c2c7e6bf5d97db51211e9a9) )
	ROM_CONTINUE(     0x30000, 0x08000 )
	ROM_LOAD( "dv07", 0x28000, 0x08000, CRC(e7346ef8) SHA1(8083a7a182e8ed904daf2f692115d01b3d0830eb) )
	ROM_CONTINUE(     0x38000, 0x08000 )

	ROM_LOAD( "dv12", 0x40000, 0x08000, CRC(46ba5af4) SHA1(a1c13e7e3c85060202120b64e3cee32c1f733270) )
	ROM_CONTINUE(     0x50000, 0x08000 )
	ROM_LOAD( "dv13", 0x48000, 0x08000, CRC(a7af6dfd) SHA1(fa41bdafb64c79bd9769903fd37d4d5172b66a52) )
	ROM_CONTINUE(     0x58000, 0x08000 )

	ROM_LOAD( "dv10", 0x60000, 0x08000, CRC(68b6d75c) SHA1(ac719ef6b30ac9e63fab13cb359e6114493f274d) )
	ROM_CONTINUE(     0x70000, 0x08000 )
	ROM_LOAD( "dv11", 0x68000, 0x08000, CRC(b5948aee) SHA1(587afbfeda985bede9e4b5f57dad6763f4294962) )
	ROM_CONTINUE(     0x78000, 0x08000 )
ROM_END

ROM_START( ghostb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dz01-22.1d", 0x08000, 0x08000, CRC(fc65fdf2) SHA1(b6ffe2043d5dbff061a9806631646428eed95dd3) )

	ROM_REGION( 0x40000, "mainbank", 0 )
	ROM_LOAD( "dz02.3d",    0x00000, 0x10000, CRC(8e117541) SHA1(7dfa6eabb29f39a615f3e5123bddcc7197ab82d0) )
	ROM_LOAD( "dz03.4d",    0x10000, 0x10000, CRC(5606a8f4) SHA1(e46e887f13f648fe2162cb853b3c20fa60e3d215) )
	ROM_LOAD( "dz04-21.6d", 0x20000, 0x10000, CRC(7d46582f) SHA1(22e70675d76e2a93a732370fa42cc4b79381f4b0) )
	ROM_LOAD( "dz05-21.7d", 0x30000, 0x10000, CRC(23e1c758) SHA1(c6432682e1d4429d0cfa8de6a05ca0152611b5b1) )

	ROM_REGION( 2*0x10000, "audiocpu", 0 ) // 64K for sound CPU + 64k for decrypted opcodes
	ROM_LOAD( "dz06.5f", 0x8000, 0x8000, CRC(798f56df) SHA1(aee33cd0c102015114e17f6cb98945e7cc806f55) )

	ROM_REGION( 0x1000, "mcu", 0 ) // i8751 microcontroller
	ROM_LOAD( "dz-1.1b", 0x0000, 0x1000, CRC(9f5f3cb5) SHA1(5ef2b8a5411dde28277d9364db014763019ecf15) )

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "dz00.16b", 0x00000, 0x08000, CRC(992b4f31) SHA1(a9f255286193ccc261a9b6983aabf3c76ebe5ce5) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "dz15.14f", 0x00000, 0x10000, CRC(a01a5fd9) SHA1(c15e11fbc0ede9e4a232abe37e6d221d5789ce8e) )
	ROM_LOAD( "dz16.15f", 0x10000, 0x10000, CRC(5a9a344a) SHA1(f4e8c2bae023ce996e99383873eba23ab6f972a8) )
	ROM_LOAD( "dz12.9f",  0x20000, 0x10000, CRC(817fae99) SHA1(4179501aedbdf5bb0824bf1c13e033685e57a207) )
	ROM_LOAD( "dz14.12f", 0x30000, 0x10000, CRC(0abbf76d) SHA1(fefb0cb7b866452b890bcf8c47b1ed95df35095e) )
	ROM_LOAD( "dz11.8f",  0x40000, 0x10000, CRC(a5e19c24) SHA1(a4aae81a116577ee3cdd9e1a46cae413ae252b76) )
	ROM_LOAD( "dz13.1f",  0x50000, 0x10000, CRC(3e7c0405) SHA1(2cdcb9a902acecec0729a906b7edb44baf130d32) )
	ROM_LOAD( "dz17.17f", 0x60000, 0x10000, CRC(40361b8b) SHA1(6ee59129e236ead3d9b828fb9726311b7a4f2ff6) )
	ROM_LOAD( "dz18.18f", 0x70000, 0x10000, CRC(8d219489) SHA1(0490ad84085d1a60ece1b8ab45f0c551d2ac219d) )

	ROM_REGION( 0x40000, "tiles1", 0 )
	ROM_LOAD( "dz07.12f", 0x00000, 0x10000, CRC(e7455167) SHA1(a4582ced57862563ef626a25ced4072bc2c95750) )
	ROM_LOAD( "dz08.14f", 0x10000, 0x10000, CRC(32f9ddfe) SHA1(2b8c228b0ca938ab7495d53e1a39275a8b872828) )
	ROM_LOAD( "dz09.15f", 0x20000, 0x10000, CRC(bb6efc02) SHA1(ec501cd4a624d9c36a545dd100bc4f2f8b1e5cc0) )
	ROM_LOAD( "dz10.17f", 0x30000, 0x10000, CRC(6ef9963b) SHA1(f12a2e2b0451a118234b2995185bb14d4998d430) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "dz19a.10d", 0x0000, 0x0400, CRC(47e1f83b) SHA1(f073eea1f33ed7a4862e4efd143debf1e0ee64b4) )
	ROM_LOAD( "dz20a.11d", 0x0400, 0x0400, CRC(d8fe2d99) SHA1(56f8fcf2f871c7d52d4299a5b9988401ada4319d) )
ROM_END

ROM_START( ghostb2a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dz01.1d", 0x08000, 0x08000, CRC(7c5bb4b1) SHA1(75865102c9bfbf9bd341b8ea54f49904c727f474) )

	ROM_REGION( 0x40000, "mainbank", 0 )
	ROM_LOAD( "dz02.3d", 0x00000, 0x10000, CRC(8e117541) SHA1(7dfa6eabb29f39a615f3e5123bddcc7197ab82d0) )
	ROM_LOAD( "dz03.4d", 0x10000, 0x10000, CRC(5606a8f4) SHA1(e46e887f13f648fe2162cb853b3c20fa60e3d215) )
	ROM_LOAD( "dz04.6d", 0x20000, 0x10000, CRC(d09bad99) SHA1(bde8e4316cedf1d292f0aed8627b0dc6794c6e07) )
	ROM_LOAD( "dz05.7d", 0x30000, 0x10000, CRC(0315f691) SHA1(3bfad18b9f230e64c608a22af20c3b00dca3e6da) )

	ROM_REGION( 2*0x10000, "audiocpu", 0 ) // 64K for sound CPU + 64k for decrypted opcodes
	ROM_LOAD( "dz06.5f", 0x8000, 0x8000, CRC(798f56df) SHA1(aee33cd0c102015114e17f6cb98945e7cc806f55) )

	ROM_REGION( 0x1000, "mcu", 0 ) // i8751 microcontroller
	ROM_LOAD( "dz-1.1b", 0x0000, 0x1000, CRC(9f5f3cb5) SHA1(5ef2b8a5411dde28277d9364db014763019ecf15) )

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "dz00.16b", 0x00000, 0x08000, CRC(992b4f31) SHA1(a9f255286193ccc261a9b6983aabf3c76ebe5ce5) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "dz15.14f", 0x00000, 0x10000, CRC(a01a5fd9) SHA1(c15e11fbc0ede9e4a232abe37e6d221d5789ce8e) )
	ROM_LOAD( "dz16.15f", 0x10000, 0x10000, CRC(5a9a344a) SHA1(f4e8c2bae023ce996e99383873eba23ab6f972a8) )
	ROM_LOAD( "dz12.9f",  0x20000, 0x10000, CRC(817fae99) SHA1(4179501aedbdf5bb0824bf1c13e033685e57a207) )
	ROM_LOAD( "dz14.12f", 0x30000, 0x10000, CRC(0abbf76d) SHA1(fefb0cb7b866452b890bcf8c47b1ed95df35095e) )
	ROM_LOAD( "dz11.8f",  0x40000, 0x10000, CRC(a5e19c24) SHA1(a4aae81a116577ee3cdd9e1a46cae413ae252b76) )
	ROM_LOAD( "dz13.1f",  0x50000, 0x10000, CRC(3e7c0405) SHA1(2cdcb9a902acecec0729a906b7edb44baf130d32) )
	ROM_LOAD( "dz17.17f", 0x60000, 0x10000, CRC(40361b8b) SHA1(6ee59129e236ead3d9b828fb9726311b7a4f2ff6) )
	ROM_LOAD( "dz18.18f", 0x70000, 0x10000, CRC(8d219489) SHA1(0490ad84085d1a60ece1b8ab45f0c551d2ac219d) )

	ROM_REGION( 0x40000, "tiles1", 0 )
	ROM_LOAD( "dz07.12f", 0x00000, 0x10000, CRC(e7455167) SHA1(a4582ced57862563ef626a25ced4072bc2c95750) )
	ROM_LOAD( "dz08.14f", 0x10000, 0x10000, CRC(32f9ddfe) SHA1(2b8c228b0ca938ab7495d53e1a39275a8b872828) )
	ROM_LOAD( "dz09.15f", 0x20000, 0x10000, CRC(bb6efc02) SHA1(ec501cd4a624d9c36a545dd100bc4f2f8b1e5cc0) )
	ROM_LOAD( "dz10.17f", 0x30000, 0x10000, CRC(6ef9963b) SHA1(f12a2e2b0451a118234b2995185bb14d4998d430) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "dz19a.10d", 0x0000, 0x0400, CRC(47e1f83b) SHA1(f073eea1f33ed7a4862e4efd143debf1e0ee64b4) )
	ROM_LOAD( "dz20a.11d", 0x0400, 0x0400, CRC(d8fe2d99) SHA1(56f8fcf2f871c7d52d4299a5b9988401ada4319d) )
ROM_END

ROM_START( ghostb3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dz01-3b.1d", 0x08000, 0x08000, CRC(c8cc862a) SHA1(e736107beb11a12cdf413655c6874df28d5a9c70) )

	ROM_REGION( 0x40000, "mainbank", 0 )
	ROM_LOAD( "dz02.3d",    0x00000, 0x10000, CRC(8e117541) SHA1(7dfa6eabb29f39a615f3e5123bddcc7197ab82d0) )
	ROM_LOAD( "dz03.4d",    0x10000, 0x10000, CRC(5606a8f4) SHA1(e46e887f13f648fe2162cb853b3c20fa60e3d215) )
	ROM_LOAD( "dz04-1.6d",  0x20000, 0x10000, CRC(3c3eb09f) SHA1(ae4975992698fa97c68a857a25b470a05539160a) )
	ROM_LOAD( "dz05-1.7d",  0x30000, 0x10000, CRC(b4971d33) SHA1(25e052c4b414c7bd7b6e3ae9c211873902afb5f7) )

	ROM_REGION( 2*0x10000, "audiocpu", 0 ) // 64K for sound CPU + 64k for decrypted opcodes
	ROM_LOAD( "dz06.5f", 0x8000, 0x8000, CRC(798f56df) SHA1(aee33cd0c102015114e17f6cb98945e7cc806f55) )

	ROM_REGION( 0x1000, "mcu", 0 ) // i8751 microcontroller
	ROM_LOAD( "dz-1.1b", 0x0000, 0x1000, CRC(9f5f3cb5) SHA1(5ef2b8a5411dde28277d9364db014763019ecf15) )

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "dz00.16b", 0x00000, 0x08000, CRC(992b4f31) SHA1(a9f255286193ccc261a9b6983aabf3c76ebe5ce5) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "dz15.14f", 0x00000, 0x10000, CRC(a01a5fd9) SHA1(c15e11fbc0ede9e4a232abe37e6d221d5789ce8e) )
	ROM_LOAD( "dz16.15f", 0x10000, 0x10000, CRC(5a9a344a) SHA1(f4e8c2bae023ce996e99383873eba23ab6f972a8) )
	ROM_LOAD( "dz12.9f",  0x20000, 0x10000, CRC(817fae99) SHA1(4179501aedbdf5bb0824bf1c13e033685e57a207) )
	ROM_LOAD( "dz14.12f", 0x30000, 0x10000, CRC(0abbf76d) SHA1(fefb0cb7b866452b890bcf8c47b1ed95df35095e) )
	ROM_LOAD( "dz11.8f",  0x40000, 0x10000, CRC(a5e19c24) SHA1(a4aae81a116577ee3cdd9e1a46cae413ae252b76) )
	ROM_LOAD( "dz13.1f",  0x50000, 0x10000, CRC(3e7c0405) SHA1(2cdcb9a902acecec0729a906b7edb44baf130d32) )
	ROM_LOAD( "dz17.17f", 0x60000, 0x10000, CRC(40361b8b) SHA1(6ee59129e236ead3d9b828fb9726311b7a4f2ff6) )
	ROM_LOAD( "dz18.18f", 0x70000, 0x10000, CRC(8d219489) SHA1(0490ad84085d1a60ece1b8ab45f0c551d2ac219d) )

	ROM_REGION( 0x40000, "tiles1", 0 )
	ROM_LOAD( "dz07.12f", 0x00000, 0x10000, CRC(e7455167) SHA1(a4582ced57862563ef626a25ced4072bc2c95750) )
	ROM_LOAD( "dz08.14f", 0x10000, 0x10000, CRC(32f9ddfe) SHA1(2b8c228b0ca938ab7495d53e1a39275a8b872828) )
	ROM_LOAD( "dz09.15f", 0x20000, 0x10000, CRC(bb6efc02) SHA1(ec501cd4a624d9c36a545dd100bc4f2f8b1e5cc0) )
	ROM_LOAD( "dz10.17f", 0x30000, 0x10000, CRC(6ef9963b) SHA1(f12a2e2b0451a118234b2995185bb14d4998d430) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "dz19a.10d", 0x0000, 0x0400, CRC(47e1f83b) SHA1(f073eea1f33ed7a4862e4efd143debf1e0ee64b4) )
	ROM_LOAD( "dz20a.11d", 0x0400, 0x0400, CRC(d8fe2d99) SHA1(56f8fcf2f871c7d52d4299a5b9988401ada4319d) )
ROM_END

// DZ-1 is the verified correct MCU code for the ghostb3a set below, both DZ01-2 & DZ04- ROMs have been verified correct from multiple sources
ROM_START( ghostb3a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dz01-2.1d", 0x08000, 0x08000, CRC(1b16890e) SHA1(eebd253d616b6286937b72cfb64612877f383932) )

	ROM_REGION( 0x40000, "mainbank", 0 )
	ROM_LOAD( "dz02-.3d",  0x00000, 0x10000, CRC(8e117541) SHA1(7dfa6eabb29f39a615f3e5123bddcc7197ab82d0) ) // == dz02.3d (ghostb3)
	ROM_LOAD( "dz03-.4d",  0x10000, 0x10000, CRC(5606a8f4) SHA1(e46e887f13f648fe2162cb853b3c20fa60e3d215) ) // == dz03.4d (ghostb3)
	ROM_LOAD( "dz04-.6d",  0x20000, 0x10000, CRC(490b4525) SHA1(3066b76f8fe99c8f9f1cdf943209883a199a4184) )
	ROM_LOAD( "dz05-.7d",  0x30000, 0x10000, CRC(b4971d33) SHA1(25e052c4b414c7bd7b6e3ae9c211873902afb5f7) ) // == dz05-1.7d (ghostb3)

	ROM_REGION( 2*0x10000, "audiocpu", 0 ) // 64K for sound CPU + 64k for decrypted opcodes
	ROM_LOAD( "dz06.5f", 0x8000, 0x8000, CRC(798f56df) SHA1(aee33cd0c102015114e17f6cb98945e7cc806f55) )

	ROM_REGION( 0x1000, "mcu", 0 ) // i8751 microcontroller
	ROM_LOAD( "dz-1.1b", 0x0000, 0x1000, CRC(9f5f3cb5) SHA1(5ef2b8a5411dde28277d9364db014763019ecf15) )

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "dz00.16b", 0x00000, 0x08000, CRC(992b4f31) SHA1(a9f255286193ccc261a9b6983aabf3c76ebe5ce5) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "dz15.14f", 0x00000, 0x10000, CRC(a01a5fd9) SHA1(c15e11fbc0ede9e4a232abe37e6d221d5789ce8e) )
	ROM_LOAD( "dz16.15f", 0x10000, 0x10000, CRC(5a9a344a) SHA1(f4e8c2bae023ce996e99383873eba23ab6f972a8) )
	ROM_LOAD( "dz12.9f",  0x20000, 0x10000, CRC(817fae99) SHA1(4179501aedbdf5bb0824bf1c13e033685e57a207) )
	ROM_LOAD( "dz14.12f", 0x30000, 0x10000, CRC(0abbf76d) SHA1(fefb0cb7b866452b890bcf8c47b1ed95df35095e) )
	ROM_LOAD( "dz11.8f",  0x40000, 0x10000, CRC(a5e19c24) SHA1(a4aae81a116577ee3cdd9e1a46cae413ae252b76) )
	ROM_LOAD( "dz13.1f",  0x50000, 0x10000, CRC(3e7c0405) SHA1(2cdcb9a902acecec0729a906b7edb44baf130d32) )
	ROM_LOAD( "dz17.17f", 0x60000, 0x10000, CRC(40361b8b) SHA1(6ee59129e236ead3d9b828fb9726311b7a4f2ff6) )
	ROM_LOAD( "dz18.18f", 0x70000, 0x10000, CRC(8d219489) SHA1(0490ad84085d1a60ece1b8ab45f0c551d2ac219d) )

	ROM_REGION( 0x40000, "tiles1", 0 )
	ROM_LOAD( "dz07.12f", 0x00000, 0x10000, CRC(e7455167) SHA1(a4582ced57862563ef626a25ced4072bc2c95750) )
	ROM_LOAD( "dz08.14f", 0x10000, 0x10000, CRC(32f9ddfe) SHA1(2b8c228b0ca938ab7495d53e1a39275a8b872828) )
	ROM_LOAD( "dz09.15f", 0x20000, 0x10000, CRC(bb6efc02) SHA1(ec501cd4a624d9c36a545dd100bc4f2f8b1e5cc0) )
	ROM_LOAD( "dz10.17f", 0x30000, 0x10000, CRC(6ef9963b) SHA1(f12a2e2b0451a118234b2995185bb14d4998d430) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "dz19a.10d", 0x0000, 0x0400, CRC(47e1f83b) SHA1(f073eea1f33ed7a4862e4efd143debf1e0ee64b4) )
	ROM_LOAD( "dz20a.11d", 0x0400, 0x0400, CRC(d8fe2d99) SHA1(56f8fcf2f871c7d52d4299a5b9988401ada4319d) )
ROM_END

/*

Meikyuu Hunter G (Data East, 1987)
Hardware info by Guru

PCB Layout
----------

DE-0273-1
|-------------------------------------------------------------|
|  2018           DW09                       DW00             |
|  2018                                                       |
|                 DW08                                      |-|
|   |---------|                                    6264     | |
|   |         |   DW07                                      | |
|   |L7A0072  |                                             | |
|   |DATA EAST|   DW06                                      | |
|   |BAC 06   |                                             | |
|J  |---------|                                             | |
|A                             DW19                         |-|
|M                                                            |
|M   DSW1      DSW2        DW18                               |
|A                                                          |-|
|                6116    |---|                              | |
|                        | H |                              | |
|                DW05    | D | DW04                         | |
|                        | 6 |                       2018   | |
|   65C02        YM3812  | 3 | DW03                         | |
|                        | C |                              | |
|   YM2203      YM3014   | 0 | DW02                         |-|
|                YM3014  | 9 |                                |
|        VOL  UPC324     |---| DW01-5           i8751H  8MHz  |
|-------------------------------------------------------------|
Notes:
      2018         - 2K x8 SRAM (DIP24)
      6116         - 2K x8 SRAM (DIP24)
      6264         - 8K x8 SRAM (DIP28)
      6502 CPU clock - 1.500MHz
      6309 CPU clock - 3.000MHz
      YM2203 clock   - 1.500MHz
      8751 clock     - 8.000MHz (contents secured)
      YM3812 clock   - 3.000MHz
      VSync       - 58Hz
      HSync       - 15.68kHz
      ROMs -
            DW00/DW01/DW05      - 27C256
            DW02/DW03/DW04      \
            DW06/DW07/DW08/DW09 / 27C512
            DW18 - Fujitsu MB7132, compatible with Philips 82S181
            DW19 - Fujitsu MB7122, compatible with Philips 82S137


DE-0259-1
|-------------------------------------------------------------|
|                                                             |
|                       2018                                  |
|   2018                                                    |-|
|                       2018                                | |
|                                         2018              | |
|   2018                                   2018             | |
|                                                           | |
|                       DW10                    |-------|   | |
|                                               |       |   | |
|                       DW11  2018              | DRL40 |   |-|
|                                               |       |     |
|                       DW12                    |-------|     |
|                                                           |-|
|                       DW13  2018                          | |
|     VSC30                                     |-------|   | |
|                       DW14                    |       |   | |
|                                               | DRL40 |   | |
|                       DW15  2018              |       |   | |
| HMC20                                         |-------|   | |
|                       DW16                                |-|
|                                                             |
|12MHz                 DW17  2018                             |
|-------------------------------------------------------------|
Notes:
      2018 - 2K x8 SRAM (DIP24)
      All ROMs 27512
      DECO Custom ICs -
                        DECO VSC30 M60348 6102 (DIP40)
                        DECO HMC20 M60232 722001 (DIP28)
                        DATA EAST DRL40 TC17G042AF 8053 8702A (x2, QFP144)
*/

ROM_START( meikyuh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dw01-5.1d", 0x08000, 0x08000, CRC(87610c39) SHA1(47b83e7decd18f117d00a9f55c42da93b337c04a) )

	ROM_REGION( 0x40000, "mainbank", 0 )
	ROM_LOAD( "dw02.3d",   0x00000, 0x10000, CRC(40c9b0b8) SHA1(81deb25e00eb4d4c5133ea42cda279c318ee771c) )
	ROM_LOAD( "dw03.4d",   0x10000, 0x10000, CRC(5606a8f4) SHA1(e46e887f13f648fe2162cb853b3c20fa60e3d215) )
	ROM_LOAD( "dw04.6d",   0x20000, 0x10000, CRC(235c0c36) SHA1(f0635f8348459cb8a56eb6184f1bc31c3a82de6a) )
	ROM_FILL(              0x30000, 0x10000, 0xff)

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "dw05.5f", 0x8000, 0x8000, CRC(c28c4d82) SHA1(ad88506bcbc9763e39d6e6bb25ef2bd6aa929f30) )

	ROM_REGION( 0x1000, "mcu", 0 ) // i8751 microcontroller
	ROM_LOAD( "dw.1b", 0x0000, 0x1000, CRC(28e9ced9) SHA1(a3d6dfa1e44fa93c0f30fa0a88b6dd3d6e5c4dda) )

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "dw00.16b", 0x00000, 0x8000, CRC(3d25f15c) SHA1(590518460d069bc235b5efebec81731d7a2375de) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "dw14.14f", 0x00000, 0x10000, CRC(9b0dbfa9) SHA1(c9db6e70b217a34fbc2bf17da3f5ec6f0130514a) )
	ROM_LOAD( "dw15.15f", 0x10000, 0x10000, CRC(95683fda) SHA1(aa91ad1cd685790e29e16d64bd75a5b4367cf87b) )
	ROM_LOAD( "dw11.9f",  0x20000, 0x10000, CRC(1b1fcca7) SHA1(17e510c1b3efa0f6da49461c286b89295db6b9a6) )
	ROM_LOAD( "dw13.12f", 0x30000, 0x10000, CRC(e7413056) SHA1(62048a9648cbb6b651e3409f77cee268822fd2e1) )
	ROM_LOAD( "dw10.8f",  0x40000, 0x10000, CRC(57667546) SHA1(e7756997ea04204e62404ce8069f8cdb33cb4565) )
	ROM_LOAD( "dw12.1f",  0x50000, 0x10000, CRC(4c548db8) SHA1(988411ab41884c926ca971e7b58f406f85be3b54) )
	ROM_LOAD( "dw16.17f", 0x60000, 0x10000, CRC(e5bcf927) SHA1(b96bd4c124c9745fae1c1f35bdbbdec9f97ab4a5) )
	ROM_LOAD( "dw17.18f", 0x70000, 0x10000, CRC(9e10f723) SHA1(159c5e3d821a10b64cd6d538d19063d0f5b057c0) )

	ROM_REGION( 0x40000, "tiles1", 0 )
	ROM_LOAD( "dw06.12f", 0x00000, 0x10000, CRC(b65e029d) SHA1(f8791d57f688f16e0f076361603510e7133f4e36) )
	ROM_LOAD( "dw07.14f", 0x10000, 0x10000, CRC(668d995d) SHA1(dc6221de6103168c8e19f2c6eb159b8989ca2208) )
	ROM_LOAD( "dw08.15f", 0x20000, 0x10000, CRC(bb2cf4a0) SHA1(78806adb6a9ad9fc0707ead567a3220eb2bdb32f) )
	ROM_LOAD( "dw09.17f", 0x30000, 0x10000, CRC(6a528d13) SHA1(f1ef592f1efea637abde26bb8e3d02d552582a43) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "dw18.9d",  0x0000, 0x0400, CRC(75f1945f) SHA1(6fa436ae21851ec30847d57c31bdd2fd695e08af)  )
	ROM_LOAD( "dw19.10d", 0x0400, 0x0400, CRC(cc16f3fa) SHA1(4562106ff752f5fc5ae00ff098141e5d74fe4700)  )
ROM_END

/*

Meikyuu Hunter G (Japan, bootleg)

the code is very different, this is a bootleg board. It lacks original labels
and the IC positions are different on the sprite ROMs

this version lacks the linescroll effects when starting the game / demoplay, but the demoplay seems
more complete, whereas in the original the players appear to get stuck before reaching the boss.
Probably bootlegged from a different revision.

CPU
---

CPUs PCB (AT0789A):
1x MC68B09EP (main)
1x 8751H (missing, the socket is empty!)
1x UM6502 (sound)
1x YM2203 (sound)
1x YM3526 (sound)
2x Y3414B (sound)
1x CA324E (sound)
1x oscillator 8.0000MHz

ROMs PCB (AT0789B):
1x oscillator 12.000MHz


ROMs
----

CPUs PCB (AT0789A):
3x P27256
2x TMM24512
5x M27512ZB
3x N82S137N

ROMs PCB (AT0789B):
8x M27512ZB
3x PAL16R4ANC (not dumped)
Note    CPUs PCB (AT0789A):
1x 28x2 JAMMA edge connector
1x trimmer (volume)
2x 8 switches dip
2x 50 pins flat cable connector to ROMs PCB

ROMs PCB (AT0789B):
2x 50 pins flat cable connector to CPUs PCB

------------------------------------
There is a small piggyback attached under CPUs PCB full of 74Sxx

ALL MEMORIES ARE MASK ROMS!

*/

ROM_START( meikyuhbl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "27256.1d", 0x08000, 0x08000, CRC(d5b5e8a2) SHA1(0155d1d0ddbd764b960148c3c9ef34223e101659) ) // dw-01-5.1d matched 6.552124%

	ROM_REGION( 0x40000, "mainbank", 0 )
	ROM_LOAD( "24512.3d", 0x00000, 0x10000, CRC(40c9b0b8) SHA1(81deb25e00eb4d4c5133ea42cda279c318ee771c) )
	ROM_LOAD( "24512.4d", 0x10000, 0x10000, CRC(5606a8f4) SHA1(e46e887f13f648fe2162cb853b3c20fa60e3d215) )
	ROM_LOAD( "27512.6d", 0x20000, 0x10000, CRC(8ca6055d) SHA1(37dc5d3b158dc5d7c9677fc4f82e10804181619f) ) // dw-04.6d matched 99.995422% (verified on 2 different PCBs, so almost certainly good)
	ROM_FILL(             0x30000, 0x10000, 0xff)

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "27256.5f", 0x8000, 0x8000, CRC(c28c4d82) SHA1(ad88506bcbc9763e39d6e6bb25ef2bd6aa929f30) )

	ROM_REGION( 0x1000, "mcu", 0 ) // i8751 microcontroller - should be unpopulated
	ROM_LOAD( "dw.1b", 0x0000, 0x1000, CRC(28e9ced9) SHA1(a3d6dfa1e44fa93c0f30fa0a88b6dd3d6e5c4dda) )

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "27256.16b", 0x00000, 0x8000, CRC(3d25f15c) SHA1(590518460d069bc235b5efebec81731d7a2375de) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "92.6m", 0x00000, 0x10000, CRC(9b0dbfa9) SHA1(c9db6e70b217a34fbc2bf17da3f5ec6f0130514a) )
	ROM_LOAD( "93.6o", 0x10000, 0x10000, CRC(95683fda) SHA1(aa91ad1cd685790e29e16d64bd75a5b4367cf87b) )
	ROM_LOAD( "89.6i", 0x20000, 0x10000, CRC(1b1fcca7) SHA1(17e510c1b3efa0f6da49461c286b89295db6b9a6) )
	ROM_LOAD( "91.6l", 0x30000, 0x10000, CRC(e7413056) SHA1(62048a9648cbb6b651e3409f77cee268822fd2e1) )
	ROM_LOAD( "88.6h", 0x40000, 0x10000, CRC(57667546) SHA1(e7756997ea04204e62404ce8069f8cdb33cb4565) )
	ROM_LOAD( "90.6k", 0x50000, 0x10000, CRC(4c548db8) SHA1(988411ab41884c926ca971e7b58f406f85be3b54) )
	ROM_LOAD( "94.6p", 0x60000, 0x10000, CRC(e5bcf927) SHA1(b96bd4c124c9745fae1c1f35bdbbdec9f97ab4a5) )
	ROM_LOAD( "95.6r", 0x70000, 0x10000, CRC(9e10f723) SHA1(159c5e3d821a10b64cd6d538d19063d0f5b057c0) )

	ROM_REGION( 0x40000, "tiles1", 0 )
	ROM_LOAD( "27512.12f", 0x00000, 0x10000, CRC(b65e029d) SHA1(f8791d57f688f16e0f076361603510e7133f4e36) )
	ROM_LOAD( "27512.14f", 0x10000, 0x10000, CRC(668d995d) SHA1(dc6221de6103168c8e19f2c6eb159b8989ca2208) )
	ROM_LOAD( "27512.15f", 0x20000, 0x10000, CRC(bb2cf4a0) SHA1(78806adb6a9ad9fc0707ead567a3220eb2bdb32f) )
	ROM_LOAD( "27512.17f", 0x30000, 0x10000, CRC(6a528d13) SHA1(f1ef592f1efea637abde26bb8e3d02d552582a43) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD_NIB_LOW(  "82s137.12d", 0x0000, 0x0400, CRC(bf922733) SHA1(c2566b2ad3d7520aa57a1e8027d4832631bd9a72) )
	ROM_LOAD_NIB_HIGH( "82s137.13d", 0x0000, 0x0400, CRC(4ccc328e) SHA1(7d527f5265b65ac070c41e89b39c38c1ba42b544) )
	ROM_LOAD(          "82s137.10d", 0x0400, 0x0400, CRC(cc16f3fa) SHA1(4562106ff752f5fc5ae00ff098141e5d74fe4700) )

	ROM_REGION( 0x600, "plds", ROMREGION_ERASEFF )
	ROM_LOAD( "pal16r4anc.16",  0x000, 0x104, NO_DUMP )
	ROM_LOAD( "pal16r4anc.158", 0x200, 0x104, NO_DUMP )
	ROM_LOAD( "pal16r4anc.165", 0x400, 0x104, NO_DUMP )
ROM_END

/*

Captain Silver

Main Components
---------------

Top board (DATA EAST DE-0250-3):
2x MC68B09EP (18e,19e)(main)
1x RP65C02A (3f)(sound)
1x YM3812 (1e)(sound)
1x YM2203 (1f)(sound)
2x Y30148 (1j,2j)(sound)
1x OKI M5205 (3j)(sound)
1x NEC PC3403C (1j)(sound)
1x C4558C (2j)(sound)
1x oscillator 8.000 (x1)
1x ID8751H (read protected)

Lower board (DATA EAST DE-0251-2):
1x DECO TC15G032AY-0013-8644a-DSPC10 (square component, with 135 pass-through pins)(14h)
1x DECO VSC30-M60348-6102 (DIL40)(9a)
1x DECO HMC20-M60232-6902 (DIL28)(14a)
1x oscillator 12.000 (x1)

ROMs
----

Top board (DATA EAST DE-0250-3):
2x MBM27256 (00,03)
10x MBM27C512 (01,02,04,05,06,07,08,09,10,11)
1x MB7122 (DIL18) (15)

Lower board (DATA EAST DE-0251-2):
3x MBM27C512

Notes
-----

Top board (DATA EAST DE-0250-3):
1x JAMMA edge connector
2x 25x2 legs connectors to lower board (cn1,cn2)
1x trimmer (volume)
2x 8 switches dip (7k,16k)

Lower board (DATA EAST DE-0251-2):
2x 25x2 legs connectors to top board (cn1,cn2)

*/

ROM_START( csilver )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dx03-12.18d", 0x08000, 0x08000, CRC(2d926e7c) SHA1(cf38e92904edb1870b0a4965f9049d67efe8cf6a) )

	ROM_REGION( 0x20000, "mainbank", 0 )
	ROM_LOAD( "dx01.12d", 0x00000, 0x10000, CRC(570fb50c) SHA1(3002f53182834a060fc282be1bc5767906e19ba2) )
	ROM_LOAD( "dx02.13d", 0x10000, 0x10000, CRC(58625890) SHA1(503a969085f6dcb16687217c48136ea22d07c89f) )

	ROM_REGION( 0x10000, "sub", 0 ) // CPU 2, 1st 16k is empty
	ROM_LOAD( "dx04-1.19d", 0x0000, 0x10000,  CRC(29432691) SHA1(a76ecd27d217c66a0e43f93e29efe83c657925c3) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "dx05.3f", 0x00000, 0x10000,  CRC(eb32cf25) SHA1(9390c88033259c65eb15320e31f5d696970987cc) )

	ROM_REGION( 0x1000, "mcu", 0 ) // i8751 microcontroller
	// 017F: B4 4C 0D : cjne  a,#$4C,$018F (ID code 0x4c = World version)
	ROM_LOAD( "dx-8.19a", 0x0000, 0x1000, CRC(c0266263) SHA1(27ac6fa4af7195f04249c04dec168ab82158704e) )

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "dx00.3d",  0x00000, 0x08000, CRC(f01ef985) SHA1(d5b823bd7c0efcf3137f8643c5d99a260bed5675) )

	ROM_REGION( 0x80000, "sprites", 0 ) // 3bpp
	ROM_LOAD( "dx14.15k",  0x00000, 0x10000, CRC(80f07915) SHA1(ea100f12ef3a68110af911fa9beeb73b388f069d) )
	// 0x10000-0x1ffff empty
	ROM_LOAD( "dx13.13k",  0x20000, 0x10000, CRC(d32c02e7) SHA1(d0518ec31e9e3f7b4e76fba5d7c05c33c61a9c72) )
	// 0x30000-0x3ffff empty
	ROM_LOAD( "dx12.10k",  0x40000, 0x10000, CRC(ac78b76b) SHA1(c2be347fd950894401123ada8b27bfcfce53e66b) )
	// 0x50000-0x5ffff empty
	// 0x60000-0x7ffff empty (no 4th plane)

	ROM_REGION( 0x80000, "tiles1", 0 ) // 3bpp
	ROM_LOAD( "dx06.5f",  0x00000, 0x10000, CRC(b6fb208c) SHA1(027d33f0b5feb6f0433134213cfcef96790eaace) )
	ROM_LOAD( "dx07.7f",  0x10000, 0x10000, CRC(ee3e1817) SHA1(013496976a9ffacf1587b3a6fc0f548becb1ab0e) )
	ROM_LOAD( "dx08.8f",  0x20000, 0x10000, CRC(705900fe) SHA1(53b9d09f9780a3bf3545bc27a2855ebee3884124) )
	ROM_LOAD( "dx09.10f", 0x30000, 0x10000, CRC(3192571d) SHA1(240c6c099f1e6edbf0be7d5a4ec396b056c9f70f) )
	ROM_LOAD( "dx10.12f", 0x40000, 0x10000, CRC(3ef77a32) SHA1(97b97c35a6ca994d2e7a6e7a63101eda9709bcb1) )
	ROM_LOAD( "dx11.13f", 0x50000, 0x10000, CRC(9cf3d5b8) SHA1(df4974f8412ab1cf65871b8e4e3dbee478bf4d21) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "dx-15.b10", 0x0000, 0x0400, CRC(dcbfec4e) SHA1(a375caef4575746870e285d90ba991ea7daefad6) ) // BPROM type MB7122E for priority (Not yet used), location on alternate board unknown
ROM_END

// There is known to exist an identical ROM set with different PCB locations designated for an alternate ROM board (noted on the right of the ROM definition)
ROM_START( csilverj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dx03-2.18d", 0x08000, 0x08000, CRC(02dd8cfc) SHA1(f29c0d9dd03e8c52672c0f3dbee44a93c5b4261d) ) // dx03-3.a4 (Different ROM label but identical to dx03-2.18d)

	ROM_REGION( 0x20000, "mainbank", 0 )
	ROM_LOAD( "dx01.12d",   0x00000, 0x10000, CRC(570fb50c) SHA1(3002f53182834a060fc282be1bc5767906e19ba2) ) // dx01.a2
	ROM_LOAD( "dx02.13d",   0x10000, 0x10000, CRC(58625890) SHA1(503a969085f6dcb16687217c48136ea22d07c89f) ) // dx01.a3

	ROM_REGION( 0x10000, "sub", 0 ) // CPU 2, 1st 16k is empty
	ROM_LOAD( "dx04-1.19d", 0x0000, 0x10000,  CRC(29432691) SHA1(a76ecd27d217c66a0e43f93e29efe83c657925c3) ) // dx04-1.a5

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "dx05.3f", 0x00000, 0x10000,  CRC(eb32cf25) SHA1(9390c88033259c65eb15320e31f5d696970987cc) ) // dx05.a6

	ROM_REGION( 0x1000, "mcu", 0 ) // i8751 microcontroller
	// hand modified version of csilver ROM
	ROM_LOAD( "id8751h_japan.mcu", 0x0000, 0x1000, BAD_DUMP CRC(6e801217) SHA1(2d8f7ae533dd8146acf8461d61ddd839544adf55) )

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "dx00.3d",  0x00000, 0x08000, CRC(f01ef985) SHA1(d5b823bd7c0efcf3137f8643c5d99a260bed5675) ) // dx00.a1

	ROM_REGION( 0x80000, "sprites", 0 ) // 3bpp
	ROM_LOAD( "dx14.15k",  0x00000, 0x10000, CRC(80f07915) SHA1(ea100f12ef3a68110af911fa9beeb73b388f069d) ) // dx14.b5
	// 0x10000-0x1ffff empty
	ROM_LOAD( "dx13.13k",  0x20000, 0x10000, CRC(d32c02e7) SHA1(d0518ec31e9e3f7b4e76fba5d7c05c33c61a9c72) ) // dx13.b4
	// 0x30000-0x3ffff empty
	ROM_LOAD( "dx12.10k",  0x40000, 0x10000, CRC(ac78b76b) SHA1(c2be347fd950894401123ada8b27bfcfce53e66b) ) // dx12.b3
	// 0x50000-0x5ffff empty
	// 0x60000-0x7ffff empty (no 4th plane)

	ROM_REGION( 0x80000, "tiles1", 0 ) // 3bpp
	ROM_LOAD( "dx06.5f",  0x00000, 0x10000, CRC(b6fb208c) SHA1(027d33f0b5feb6f0433134213cfcef96790eaace) ) // dx06.a7
	ROM_LOAD( "dx07.7f",  0x10000, 0x10000, CRC(ee3e1817) SHA1(013496976a9ffacf1587b3a6fc0f548becb1ab0e) ) // dx07.a8
	ROM_LOAD( "dx08.8f",  0x20000, 0x10000, CRC(705900fe) SHA1(53b9d09f9780a3bf3545bc27a2855ebee3884124) ) // dx08.a9
	ROM_LOAD( "dx09.10f", 0x30000, 0x10000, CRC(3192571d) SHA1(240c6c099f1e6edbf0be7d5a4ec396b056c9f70f) ) // dx09.a10
	ROM_LOAD( "dx10.12f", 0x40000, 0x10000, CRC(3ef77a32) SHA1(97b97c35a6ca994d2e7a6e7a63101eda9709bcb1) ) // dx10.b1
	ROM_LOAD( "dx11.13f", 0x50000, 0x10000, CRC(9cf3d5b8) SHA1(df4974f8412ab1cf65871b8e4e3dbee478bf4d21) ) // dx11.b2

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "dx-15.b10", 0x0000, 0x0400, CRC(dcbfec4e) SHA1(a375caef4575746870e285d90ba991ea7daefad6) ) // BPROM type MB7122E for priority (Not yet used), location on alternate board unknown
ROM_END

ROM_START( csilverja ) // DE-0250-3 + DE-0251-2
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dx03-1.18d", 0x08000, 0x08000, CRC(d42905be) SHA1(5a406466aa9bb2b2591d02fc87289cb93f7358c6) )

	ROM_REGION( 0x20000, "mainbank", 0 )
	ROM_LOAD( "dx01.12d",   0x00000, 0x10000, CRC(570fb50c) SHA1(3002f53182834a060fc282be1bc5767906e19ba2) )
	ROM_LOAD( "dx02.13d",   0x10000, 0x10000, CRC(58625890) SHA1(503a969085f6dcb16687217c48136ea22d07c89f) )

	ROM_REGION( 0x10000, "sub", 0 ) // CPU 2, 1st 16k is empty
	ROM_LOAD( "dx04-1.19d", 0x0000, 0x10000,  CRC(29432691) SHA1(a76ecd27d217c66a0e43f93e29efe83c657925c3) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "dx05.3f", 0x00000, 0x10000,  CRC(eb32cf25) SHA1(9390c88033259c65eb15320e31f5d696970987cc) )

	ROM_REGION( 0x1000, "mcu", 0 ) // i8751 microcontroller
	// hand modified version of csilver ROM
	ROM_LOAD( "id8751h_japan.mcu", 0x0000, 0x1000, BAD_DUMP CRC(6e801217) SHA1(2d8f7ae533dd8146acf8461d61ddd839544adf55) )

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "dx00.3d",  0x00000, 0x08000, CRC(f01ef985) SHA1(d5b823bd7c0efcf3137f8643c5d99a260bed5675) )

	ROM_REGION( 0x80000, "sprites", 0 ) // 3bpp
	ROM_LOAD( "dx14.15k",  0x00000, 0x10000, CRC(80f07915) SHA1(ea100f12ef3a68110af911fa9beeb73b388f069d) )
	// 0x10000-0x1ffff empty
	ROM_LOAD( "dx13.13k",  0x20000, 0x10000, CRC(d32c02e7) SHA1(d0518ec31e9e3f7b4e76fba5d7c05c33c61a9c72) )
	// 0x30000-0x3ffff empty
	ROM_LOAD( "dx12.10k",  0x40000, 0x10000, CRC(ac78b76b) SHA1(c2be347fd950894401123ada8b27bfcfce53e66b) )
	// 0x50000-0x5ffff empty
	// 0x60000-0x7ffff empty (no 4th plane)

	ROM_REGION( 0x80000, "tiles1", 0 ) // 3bpp
	ROM_LOAD( "dx06.5f",  0x00000, 0x10000, CRC(b6fb208c) SHA1(027d33f0b5feb6f0433134213cfcef96790eaace) )
	ROM_LOAD( "dx07.7f",  0x10000, 0x10000, CRC(ee3e1817) SHA1(013496976a9ffacf1587b3a6fc0f548becb1ab0e) )
	ROM_LOAD( "dx08.8f",  0x20000, 0x10000, CRC(705900fe) SHA1(53b9d09f9780a3bf3545bc27a2855ebee3884124) )
	ROM_LOAD( "dx09.10f", 0x30000, 0x10000, CRC(3192571d) SHA1(240c6c099f1e6edbf0be7d5a4ec396b056c9f70f) )
	ROM_LOAD( "dx10.12f", 0x40000, 0x10000, CRC(3ef77a32) SHA1(97b97c35a6ca994d2e7a6e7a63101eda9709bcb1) )
	ROM_LOAD( "dx11.13f", 0x50000, 0x10000, CRC(9cf3d5b8) SHA1(df4974f8412ab1cf65871b8e4e3dbee478bf4d21) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "dx-15.b10", 0x0000, 0x400, CRC(dcbfec4e) SHA1(a375caef4575746870e285d90ba991ea7daefad6) ) // BPROM type MB7122E for priority (Not yet used), location on alternate board unknown
ROM_END

ROM_START( oscar )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "du10", 0x08000, 0x08000, CRC(120040d8) SHA1(22d5f84f3ca724cbf39dfc4790f2175ba4945aaf) ) // This label is probably incorrect. The correct label is needed

	ROM_REGION( 0x10000, "mainbank", 0 )
	ROM_LOAD( "ed09", 0x00000, 0x10000, CRC(e2d4bba9) SHA1(99f0310debe51f4bcd00b5fdaedc1caf2eeccdeb) ) // for the world set because DU is the code for the Japanese version

	ROM_REGION( 0x10000, "sub", 0 ) // CPU 2, 1st 16k is empty
	ROM_LOAD( "du11", 0x0000, 0x10000, CRC(ff45c440) SHA1(4769944bcfebcdcba7ed7d5133d4d9f98890d75c) )

	ROM_REGION( 2*0x10000, "audiocpu", 0 ) // 64K for sound CPU + 64k for decrypted opcodes
	ROM_LOAD( "ed12", 0x8000, 0x8000, CRC(432031c5) SHA1(af2deea48b98eb0f9e85a4fb1486021f999f9abd) )

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "ed08", 0x00000, 0x04000, CRC(308ac264) SHA1(fd1c4ec4e4f99c33e93cd15e178c4714251a9b7e) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "ed04", 0x00000, 0x10000, CRC(416a791b) SHA1(e6541b713225289a43962363029eb0e22a1ecb4a) )
	ROM_LOAD( "ed05", 0x20000, 0x10000, CRC(fcdba431) SHA1(0be2194519c36ddf136610f60890506eda1faf0b) )
	ROM_LOAD( "ed06", 0x40000, 0x10000, CRC(7d50bebc) SHA1(06375f3273c48c7c7d81f1c15cbc5d3f3e05b8ed) )
	ROM_LOAD( "ed07", 0x60000, 0x10000, CRC(8fdf0fa5) SHA1(2b4d1ca1436864e89b13b3fa151a4a3708592e0a) )

	ROM_REGION( 0x80000, "tiles1", 0 )
	ROM_LOAD( "ed01", 0x00000, 0x10000, CRC(d3a58e9e) SHA1(35eda2aa630fc2c11a1aff2b00bcfabe2f3d4249) )
	ROM_LOAD( "ed03", 0x20000, 0x10000, CRC(4fc4fb0f) SHA1(0906762a3adbffe765e072255262fedaa78bdb2a) )
	ROM_LOAD( "ed00", 0x40000, 0x10000, CRC(ac201f2d) SHA1(77f13eb6a1a44444ca9b25363031451b0d68c988) )
	ROM_LOAD( "ed02", 0x60000, 0x10000, CRC(7ddc5651) SHA1(f5ec5245cf3d9d4d9c1df6a8128c24565e331317) )
ROM_END

ROM_START( oscarbl ) // very similar to the original, main difference it's it uses a standard M6502 for sound with predecrypted opcodes.
	ROM_REGION( 0x10000, "maincpu", 0 ) // same as the original
	ROM_LOAD( "m27c256.3",  0x08000, 0x08000, CRC(120040d8) SHA1(22d5f84f3ca724cbf39dfc4790f2175ba4945aaf) )

	ROM_REGION( 0x10000, "mainbank", 0 )
	ROM_LOAD( "at27c512.2", 0x00000, 0x10000, CRC(e2d4bba9) SHA1(99f0310debe51f4bcd00b5fdaedc1caf2eeccdeb) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "at27c512.4", 0x00000, 0x10000, CRC(2ad9ef5d) SHA1(19db4446a6a5f75c7ddb2807b69d7c40d8b2d55a) )

	ROM_REGION( 2*0x10000, "audiocpu", 0 )
	ROM_LOAD( "at27c512.1", 0x08000, 0x10000, CRC(302ff92c) SHA1(222cc1e4673a5439da1cdd07cc65dc23f522da1c) ) // data in the first half, opcodes in the second

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "ed08", 0x00000, 0x04000, BAD_DUMP CRC(308ac264) SHA1(fd1c4ec4e4f99c33e93cd15e178c4714251a9b7e) ) // not included in this set, probably same

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "at27c512.6", 0x00000, 0x10000, CRC(967315b5) SHA1(b7a081241477ab8e62fe3df1b7025b50ceaba180) )
	ROM_LOAD( "at27c512.7", 0x20000, 0x10000, CRC(fcdba431) SHA1(0be2194519c36ddf136610f60890506eda1faf0b) )
	ROM_LOAD( "at27c512.8", 0x40000, 0x10000, CRC(7d50bebc) SHA1(06375f3273c48c7c7d81f1c15cbc5d3f3e05b8ed) )
	ROM_LOAD( "at27c512.9", 0x60000, 0x10000, CRC(8fdf0fa5) SHA1(2b4d1ca1436864e89b13b3fa151a4a3708592e0a) )

	ROM_REGION( 0x80000, "tiles1", 0 )
	ROM_LOAD( "at27c512.10", 0x00000, 0x10000, CRC(98cb4ffc) SHA1(75465a1e7c113db766e14d22b31bf999c520a8bf) )
	ROM_LOAD( "at27c512.11", 0x20000, 0x10000, CRC(bff7fddc) SHA1(129e99fa99920e647a53b9af64c9c288c1e8ad57) )
	ROM_LOAD( "at27c512.12", 0x40000, 0x10000, CRC(eff3b56c) SHA1(427d3aa053fa81ef004019eb8bb0aa97787f283e) )
	ROM_LOAD( "at27c512.13", 0x60000, 0x10000, CRC(7ddc5651) SHA1(f5ec5245cf3d9d4d9c1df6a8128c24565e331317) )
ROM_END

ROM_START( oscaru )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ed10", 0x08000, 0x08000, CRC(f9b0d4d4) SHA1(dc2aba978ba96f365027c7be5714728d5d7fb802) )

	ROM_REGION( 0x10000, "mainbank", 0 )
	ROM_LOAD( "ed09", 0x00000, 0x10000, CRC(e2d4bba9) SHA1(99f0310debe51f4bcd00b5fdaedc1caf2eeccdeb) )

	ROM_REGION( 0x10000, "sub", 0 ) // CPU 2, 1st 16k is empty
	ROM_LOAD( "ed11", 0x0000, 0x10000,  CRC(10e5d919) SHA1(13bc3497cb4aaa6dd272853819212ad63656f8a7) )

	ROM_REGION( 2*0x10000, "audiocpu", 0 ) // 64K for sound CPU + 64k for decrypted opcodes
	ROM_LOAD( "ed12", 0x8000, 0x8000,  CRC(432031c5) SHA1(af2deea48b98eb0f9e85a4fb1486021f999f9abd) )

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "ed08", 0x00000, 0x04000, CRC(308ac264) SHA1(fd1c4ec4e4f99c33e93cd15e178c4714251a9b7e) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "ed04", 0x00000, 0x10000, CRC(416a791b) SHA1(e6541b713225289a43962363029eb0e22a1ecb4a) )
	ROM_LOAD( "ed05", 0x20000, 0x10000, CRC(fcdba431) SHA1(0be2194519c36ddf136610f60890506eda1faf0b) )
	ROM_LOAD( "ed06", 0x40000, 0x10000, CRC(7d50bebc) SHA1(06375f3273c48c7c7d81f1c15cbc5d3f3e05b8ed) )
	ROM_LOAD( "ed07", 0x60000, 0x10000, CRC(8fdf0fa5) SHA1(2b4d1ca1436864e89b13b3fa151a4a3708592e0a) )

	ROM_REGION( 0x80000, "tiles1", 0 )
	ROM_LOAD( "ed01", 0x00000, 0x10000, CRC(d3a58e9e) SHA1(35eda2aa630fc2c11a1aff2b00bcfabe2f3d4249) )
	ROM_LOAD( "ed03", 0x20000, 0x10000, CRC(4fc4fb0f) SHA1(0906762a3adbffe765e072255262fedaa78bdb2a) )
	ROM_LOAD( "ed00", 0x40000, 0x10000, CRC(ac201f2d) SHA1(77f13eb6a1a44444ca9b25363031451b0d68c988) )
	ROM_LOAD( "ed02", 0x60000, 0x10000, CRC(7ddc5651) SHA1(f5ec5245cf3d9d4d9c1df6a8128c24565e331317) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "du-13.bin", 0x0000, 0x0200, CRC(bea1f87e) SHA1(f5215992e4b53c9cd4c7e0b20ff5cfdce3ab6d02) ) // Priority (Not yet used)
ROM_END

ROM_START( oscarj1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "du10-1", 0x08000, 0x08000, CRC(4ebc9f7a) SHA1(df8fdc4b4b203dc1bdd048f069fb6b723bdea0d2) )

	ROM_REGION( 0x10000, "mainbank", 0 )
	ROM_LOAD( "ed09",   0x00000, 0x10000, CRC(e2d4bba9) SHA1(99f0310debe51f4bcd00b5fdaedc1caf2eeccdeb) )

	ROM_REGION( 0x10000, "sub", 0 ) // CPU 2, 1st 16k is empty
	ROM_LOAD( "du11", 0x0000, 0x10000, CRC(ff45c440) SHA1(4769944bcfebcdcba7ed7d5133d4d9f98890d75c) )

	ROM_REGION( 2*0x10000, "audiocpu", 0 ) // 64K for sound CPU + 64k for decrypted opcodes
	ROM_LOAD( "ed12", 0x8000, 0x8000, CRC(432031c5) SHA1(af2deea48b98eb0f9e85a4fb1486021f999f9abd) )

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "ed08", 0x00000, 0x04000, CRC(308ac264) SHA1(fd1c4ec4e4f99c33e93cd15e178c4714251a9b7e) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "ed04", 0x00000, 0x10000, CRC(416a791b) SHA1(e6541b713225289a43962363029eb0e22a1ecb4a) )
	ROM_LOAD( "ed05", 0x20000, 0x10000, CRC(fcdba431) SHA1(0be2194519c36ddf136610f60890506eda1faf0b) )
	ROM_LOAD( "ed06", 0x40000, 0x10000, CRC(7d50bebc) SHA1(06375f3273c48c7c7d81f1c15cbc5d3f3e05b8ed) )
	ROM_LOAD( "ed07", 0x60000, 0x10000, CRC(8fdf0fa5) SHA1(2b4d1ca1436864e89b13b3fa151a4a3708592e0a) )

	ROM_REGION( 0x80000, "tiles1", 0 )
	ROM_LOAD( "ed01", 0x00000, 0x10000, CRC(d3a58e9e) SHA1(35eda2aa630fc2c11a1aff2b00bcfabe2f3d4249) )
	ROM_LOAD( "ed03", 0x20000, 0x10000, CRC(4fc4fb0f) SHA1(0906762a3adbffe765e072255262fedaa78bdb2a) )
	ROM_LOAD( "ed00", 0x40000, 0x10000, CRC(ac201f2d) SHA1(77f13eb6a1a44444ca9b25363031451b0d68c988) )
	ROM_LOAD( "ed02", 0x60000, 0x10000, CRC(7ddc5651) SHA1(f5ec5245cf3d9d4d9c1df6a8128c24565e331317) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "du-13.bin", 0x0000, 0x0200, CRC(bea1f87e) SHA1(f5215992e4b53c9cd4c7e0b20ff5cfdce3ab6d02) ) // Priority (Not yet used)
ROM_END

/***************************************************************************

Psycho-Nics Oscar (Data East, 1987)
Hardware info by Guru
---------------------

DE-0286-2
OSCAR 7891-1077 (sticker)
|--------------------------------------------------------------|
|       2018(2)     PR-1.F20           |--------|    DU07.A20  |
|       2018(2)                        |DATAEAST|              |
|                   PR-0.F18           |MXC 06  |              |
|RCDM-15                               |        |    DU06.A16  |
|      SW2                       2018  |--------|              |
|        DU11.H16   68B09(2)     2018                          |
|        5864                                        DU05.A14  |
|J                                     |-----------|           |
|A       DU10-2.H12              2018  | DATAEAST  |           |
|M                  68B09(1)           |  BAC 06   | DU04.A11  |
|M       DU09.H10                      |           |           |
|A                                     |           |           |
|                                      |-----------|   8416(1) |
|      SW1                                             8416(1) |
|                                        DU-13.C8    DU03.A6   |
|8416(3)   C10707-1              8416(2)                       |
|                      DU08.E5                       DU02.A5   |
|DU12.K5   YM3812                                              |
|3403 YM3014(1)                                      DU01.A3   |
|VOL  YM3014(2)                                                |
|MB3730    YM2203                            12MHz   DU00.A1   |
|--------------------------------------------------------------|
Notes:
    68B09(1) - Motorola MC68B09EP CPU. Clock Q & E 1.500MHz [12/8] (main program CPU)
    68B09(2) - Motorola MC68B09EP CPU. Clock Q & E 1.500MHz [12/8] (sub program CPU)
               Note: These are EP types so the clocks are measured on the Q & E quadrature clock inputs
    C10707-1 - Data East custom CPU marked 'C10707-1'. PCB marked 'DECO 222'. This is actually an encrypted 6502 CPU.
               Clocks are 1.500MHz on pins 3, 37 & 39
      YM2203 - Yamaha YM2203 FM Operator Type-N (OPN) sound chip. Clock 1.500MHz [12/8]
      YM3812 - Yamaha YM3812 FM Operator Type-L II (OPL II) Sound Chip. Clock 3.000MHz [12/4]
               Note this is fully compatible with YM3526 and either chip can be seen on PCBs in the wild
   YM3014(1) - Yamaha YM3014 Serial Input Floating D/A Converter. Clock 750kHz [12/4/4]. This is used with the YM3812
   YM3014(2) - Yamaha YM3014 Serial Input Floating D/A Converter. Clock 1.000MHz [12/4/3]. This is used with the YM2203
     8416(1) - Fujitsu MB8416 2kBx8-bit SRAM (background tile RAM)
     8416(2) - Fujitsu MB8416 2kBx8-bit SRAM (characters / text layer RAM)
     8416(3) - Fujitsu MB8416 2kBx8-bit SRAM (sound CPU RAM)
        5864 - Sony CXK5864 8kBx8-bit SRAM (main/sub CPU RAM)
        2018 - Toshiba TMM2018 2kBx8-bit SRAM (sprite RAM)
     2018(2) - Toshiba TMM2018 2kBx8-bit SRAM (color RAM)
       SW1/2 - 8-position DIP switch
        3403 - NEC uPC3403 Quad Operational Amplifier
      MB3730 - Fujitsu MB3730 14W BTL Audio Power Amplifier
     RCDM-15 - Custom SIP package for coin counters
      MXC 06 - Data East MXC 06 custom sprite generator
      BAC 06 - Data East BAC 06 custom tile generator
        PR-1 - MMI PAL16L8
        PR-0 - MMI PAL10L8
  DU10-2.H12 - 27C256 32kBx8-bit OTP EPROM (main CPU program)
    DU09.H10 - 27C512 64kBx8-bit OTP EPROM (main CPU program)
    DU11.H16 - 27C512 64kBx8-bit OTP EPROM (sub CPU program)
     DU12.K5 - 27C256 32kBx8-bit OTP EPROM (C10707-1 sound program)
     DU08.E5 - 27C128 16kBx8-bit OTP EPROM (characters / text layer)
     DU00.A1 - 27C512 64kBx8-bit OTP EPROM (tiles)
     DU01.A3 \
     DU02.A5 | 27C512 64kBx8-bit EPROM (tiles)
     DU03.A6 /
    DU04.A11 \ 27C512 64kBx8-bit EPROM (sprites)
    DU05.A14 /
    DU06.A16 \ 27C512 64kBx8-bit OTP EPROM (sprites)
    DU07.A20 /
    DU-13.C8 - Fujitsu MB7124 512x8-bit Bipolar PROM (priority)
               Compatible with 82S147. When removed sprites and characters/texts do not show on screen.
       HSync - 15.6178kHz. Measured on PROM at C8
       VSync - 57.4184Hz. Measured on logic near the 12MHz xtal

***************************************************************************/

ROM_START( oscarj2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "du10-2.h12", 0x08000, 0x08000, CRC(114e898d) SHA1(1072ccabe6d53c50cdfa1e27d5d848ecdd6559cc) )

	ROM_REGION( 0x10000, "mainbank", 0 )
	ROM_LOAD( "du09.h10",   0x00000, 0x10000, CRC(e2d4bba9) SHA1(99f0310debe51f4bcd00b5fdaedc1caf2eeccdeb) )

	ROM_REGION( 0x10000, "sub", 0 ) // CPU 2, 1st 16k is empty
	ROM_LOAD( "du11.h16", 0x0000, 0x10000, CRC(ff45c440) SHA1(4769944bcfebcdcba7ed7d5133d4d9f98890d75c) )

	ROM_REGION( 2*0x10000, "audiocpu", 0 ) // 64K for sound CPU + 64k for decrypted opcodes
	ROM_LOAD( "du12.k5", 0x8000, 0x8000, CRC(432031c5) SHA1(af2deea48b98eb0f9e85a4fb1486021f999f9abd) )

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "du08.e5", 0x00000, 0x04000, CRC(308ac264) SHA1(fd1c4ec4e4f99c33e93cd15e178c4714251a9b7e) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "du04.a11", 0x00000, 0x10000, CRC(416a791b) SHA1(e6541b713225289a43962363029eb0e22a1ecb4a) )
	ROM_LOAD( "du05.a14", 0x20000, 0x10000, CRC(fcdba431) SHA1(0be2194519c36ddf136610f60890506eda1faf0b) )
	ROM_LOAD( "du06.a16", 0x40000, 0x10000, CRC(7d50bebc) SHA1(06375f3273c48c7c7d81f1c15cbc5d3f3e05b8ed) )
	ROM_LOAD( "du07.a20", 0x60000, 0x10000, CRC(8fdf0fa5) SHA1(2b4d1ca1436864e89b13b3fa151a4a3708592e0a) )

	ROM_REGION( 0x80000, "tiles1", 0 )
	ROM_LOAD( "du01.a3", 0x00000, 0x10000, CRC(d3a58e9e) SHA1(35eda2aa630fc2c11a1aff2b00bcfabe2f3d4249) )
	ROM_LOAD( "du03.a6", 0x20000, 0x10000, CRC(4fc4fb0f) SHA1(0906762a3adbffe765e072255262fedaa78bdb2a) )
	ROM_LOAD( "du00.a1", 0x40000, 0x10000, CRC(ac201f2d) SHA1(77f13eb6a1a44444ca9b25363031451b0d68c988) )
	ROM_LOAD( "du02.a5", 0x60000, 0x10000, CRC(7ddc5651) SHA1(f5ec5245cf3d9d4d9c1df6a8128c24565e331317) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "du-13.c8", 0x0000, 0x0200, CRC(bea1f87e) SHA1(f5215992e4b53c9cd4c7e0b20ff5cfdce3ab6d02) ) // Priority (Not yet used)
ROM_END

ROM_START( srdarwin )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dy_01-e.b14", 0x00000, 0x10000, CRC(176e9299) SHA1(20cd44ab610e384ab4f0172054c9adc432b12e9c) )

	ROM_REGION( 0x20000, "mainbank", 0 )
	ROM_LOAD( "dy_00.b16",   0x00000, 0x10000, CRC(2bf6b461) SHA1(435d922c7b9df7f2b2f774346caed81d330be8a0) )
	ROM_LOAD( "dy_01-e.b14", 0x10000, 0x10000, CRC(176e9299) SHA1(20cd44ab610e384ab4f0172054c9adc432b12e9c) )

	ROM_REGION( 2*0x10000, "audiocpu", 0 ) // 64K for sound CPU + 64k for decrypted opcodes
	ROM_LOAD( "dy_04.d7", 0x8000, 0x8000, CRC(2ae3591c) SHA1(f21b06d84e2c3d3895be0812024641fd006e45cf) )

	ROM_REGION( 0x1000, "mcu", 0 ) // i8751 microcontroller
	// 0160: B4 6B 0D : cjne  a,#$6B,$0170 (ID code 0x6b = World version)
	ROM_LOAD( "dy-e.d11", 0x0000, 0x1000, CRC(11cd6ca4) SHA1(ec70f84228e37f9fc1bda85fa52a009f61c500b2) )

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "dy_05.b6", 0x00000, 0x4000, CRC(8780e8a3) SHA1(03ea91fdc5aba8e139201604fb3bf9b69f71f056) )

	ROM_REGION( 0x30000, "sprites", 0 )
	ROM_LOAD( "dy_07.h16", 0x00000, 0x8000, CRC(97eaba60) SHA1(e3252b67bad7babcf4ece39f46ae4aeb950eb92b) )
	ROM_LOAD( "dy_06.h14", 0x08000, 0x8000, CRC(c279541b) SHA1(eb3737413499d07b6c2af99a95b27b2590e670c5) )
	ROM_LOAD( "dy_09.k13", 0x10000, 0x8000, CRC(d30d1745) SHA1(647b6121ab6fa812368da45e1295cc41f73be89d) )
	ROM_LOAD( "dy_08.k11", 0x18000, 0x8000, CRC(71d645fd) SHA1(a74a9b9697fc39b4e675e732a9d7d82976cc95dd) )
	ROM_LOAD( "dy_11.k16", 0x20000, 0x8000, CRC(fd9ccc5b) SHA1(b38c44c01acdc455d4192e4c8be1d68d9eb0c7b6) )
	ROM_LOAD( "dy_10.k14", 0x28000, 0x8000, CRC(88770ab8) SHA1(0a4a807a8d3b0653864bd984872d5567836f8cf8) )

	ROM_REGION( 0x40000, "tiles1", 0 )
	ROM_LOAD( "dy_03.b4",  0x00000, 0x4000, CRC(44f2a4f9) SHA1(97368dd112451cd630f2fa5ba54679e84e7d4d97) )
	ROM_CONTINUE(          0x10000, 0x4000 )
	ROM_CONTINUE(          0x20000, 0x4000 )
	ROM_CONTINUE(          0x30000, 0x4000 )
	ROM_LOAD( "dy_02.b5",  0x08000, 0x4000, CRC(522d9a9e) SHA1(248274ed6df604357cad386fcf0521b26810aa0e) )
	ROM_CONTINUE(          0x18000, 0x4000 )
	ROM_CONTINUE(          0x28000, 0x4000 )
	ROM_CONTINUE(          0x38000, 0x4000 )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "dy_12.f4",  0x0000, 0x0100, CRC(ebfaaed9) SHA1(5723dbfa3eb3fc4df8c8975b320a5c49848309d8) ) // Priority (Not yet used)
ROM_END

ROM_START( srdarwinj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dy_01.b14", 0x00000, 0x10000, CRC(1eeee4ff) SHA1(89a70de8bd61c671582b11773ce69b2edcd9c2f8) )

	ROM_REGION( 0x20000, "mainbank", 0 )
	ROM_LOAD( "dy_00.b16", 0x00000, 0x10000, CRC(2bf6b461) SHA1(435d922c7b9df7f2b2f774346caed81d330be8a0) )
	ROM_LOAD( "dy_01.b14", 0x10000, 0x10000, CRC(1eeee4ff) SHA1(89a70de8bd61c671582b11773ce69b2edcd9c2f8) )

	ROM_REGION( 2*0x10000, "audiocpu", 0 ) // 64K for sound CPU + 64k for decrypted opcodes
	ROM_LOAD( "dy_04.d7", 0x8000, 0x8000, CRC(2ae3591c) SHA1(f21b06d84e2c3d3895be0812024641fd006e45cf) )

	ROM_REGION( 0x1000, "mcu", 0 ) // i8751 microcontroller
	ROM_LOAD( "dy.d11", 0x0000, 0x1000, BAD_DUMP CRC(4ac2ca9d) SHA1(6e07788df9fcf4248a9d3e87b8c5f54776bd269e) ) // hand-modified copy of world version to correct region + coinage

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "dy_05.b6", 0x00000, 0x4000, CRC(8780e8a3) SHA1(03ea91fdc5aba8e139201604fb3bf9b69f71f056) )

	ROM_REGION( 0x30000, "sprites", 0 )
	ROM_LOAD( "dy_07.h16", 0x00000, 0x8000, CRC(97eaba60) SHA1(e3252b67bad7babcf4ece39f46ae4aeb950eb92b) )
	ROM_LOAD( "dy_06.h14", 0x08000, 0x8000, CRC(c279541b) SHA1(eb3737413499d07b6c2af99a95b27b2590e670c5) )
	ROM_LOAD( "dy_09.k13", 0x10000, 0x8000, CRC(d30d1745) SHA1(647b6121ab6fa812368da45e1295cc41f73be89d) )
	ROM_LOAD( "dy_08.k11", 0x18000, 0x8000, CRC(71d645fd) SHA1(a74a9b9697fc39b4e675e732a9d7d82976cc95dd) )
	ROM_LOAD( "dy_11.k16", 0x20000, 0x8000, CRC(fd9ccc5b) SHA1(b38c44c01acdc455d4192e4c8be1d68d9eb0c7b6) )
	ROM_LOAD( "dy_10.k14", 0x28000, 0x8000, CRC(88770ab8) SHA1(0a4a807a8d3b0653864bd984872d5567836f8cf8) )

	ROM_REGION( 0x40000, "tiles1", 0 )
	ROM_LOAD( "dy_03.b4",  0x00000, 0x4000, CRC(44f2a4f9) SHA1(97368dd112451cd630f2fa5ba54679e84e7d4d97) )
	ROM_CONTINUE(          0x10000, 0x4000 )
	ROM_CONTINUE(          0x20000, 0x4000 )
	ROM_CONTINUE(          0x30000, 0x4000 )
	ROM_LOAD( "dy_02.b5",  0x08000, 0x4000, CRC(522d9a9e) SHA1(248274ed6df604357cad386fcf0521b26810aa0e) )
	ROM_CONTINUE(          0x18000, 0x4000 )
	ROM_CONTINUE(          0x28000, 0x4000 )
	ROM_CONTINUE(          0x38000, 0x4000 )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "dy_12.f4",  0x0000, 0x0100, CRC(ebfaaed9) SHA1(5723dbfa3eb3fc4df8c8975b320a5c49848309d8) ) // Priority (Not yet used)
ROM_END

ROM_START( srdarwinb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "3.bin",  0x00000, 0x10000, CRC(7942b43f) SHA1(15de0c02d45d06c145fba48ef05baae793a1cb46) )

	ROM_REGION( 0x20000, "mainbank", 0 )
	ROM_LOAD( "4.bin",  0x00000, 0x10000, CRC(2bf6b461) SHA1(435d922c7b9df7f2b2f774346caed81d330be8a0) )
	ROM_LOAD( "3.bin",  0x10000, 0x10000, CRC(7942b43f) SHA1(15de0c02d45d06c145fba48ef05baae793a1cb46) )

	ROM_REGION( 2*0x10000, "audiocpu", 0 ) // 64K for sound CPU + 64k for decrypted opcodes
	ROM_LOAD( "5.bin", 0x8000, 0x8000, CRC(2ae3591c) SHA1(f21b06d84e2c3d3895be0812024641fd006e45cf) )

	ROM_REGION( 0x8000, "char", 0 )
	ROM_LOAD( "12.bin",  0x0000, 0x4000, CRC(f5c835cd) SHA1(8b41862dc18ba2e2677c94ecef45a40467aa89ad) )
	ROM_CONTINUE(        0x0000, 0x4000 ) // all data in the 2nd half

	ROM_REGION( 0x30000, "sprites", 0 )
	ROM_LOAD( "6.bin",   0x00000, 0x8000, CRC(3c84a2c6) SHA1(558a7d9acb5af06a7728d010262e3a35c3cdbe25) )
	ROM_LOAD( "7.bin",   0x08000, 0x8000, CRC(990cfc7b) SHA1(84ab42010a483e8a4cd86357898f55e644d1b11e) )
	ROM_LOAD( "10.bin",  0x10000, 0x8000, CRC(cf7dcdc1) SHA1(a94a970ff564da0fef8cd7cdcbf9aee5f83b596c) )
	ROM_LOAD( "11.bin",  0x18000, 0x8000, CRC(3674e392) SHA1(d91387c2412bf950993751c2e4764f818489316f) )
	ROM_LOAD( "8.bin",   0x20000, 0x8000, CRC(cc39b73f) SHA1(8cdbef67526f29ccf77c04aec5e34253bcdf96c3) )
	ROM_LOAD( "9.bin",   0x28000, 0x8000, CRC(d15aaa08) SHA1(69649f0cb2f11107b37ae7914dd90f4c6269316f) )

	ROM_REGION( 0x40000, "tiles1", 0 )
	ROM_LOAD( "1.bin",  0x00000, 0x4000, CRC(44f2a4f9) SHA1(97368dd112451cd630f2fa5ba54679e84e7d4d97) )
	ROM_CONTINUE(       0x10000, 0x4000 )
	ROM_CONTINUE(       0x20000, 0x4000 )
	ROM_CONTINUE(       0x30000, 0x4000 )
	ROM_LOAD( "2.bin",  0x08000, 0x4000, CRC(522d9a9e) SHA1(248274ed6df604357cad386fcf0521b26810aa0e) )
	ROM_CONTINUE(       0x18000, 0x4000 )
	ROM_CONTINUE(       0x28000, 0x4000 )
	ROM_CONTINUE(       0x38000, 0x4000 )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s123.bin",  0x0000, 0x0100, NO_DUMP ) // Priority (Not yet used)
ROM_END


ROM_START( cobracom )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "el11-5.5j", 0x08000, 0x08000, CRC(af0a8b05) SHA1(096e4e7f2785a20bfaec14277413ce4e20e90214) )

	ROM_REGION( 0x20000, "mainbank", 0 )
	ROM_LOAD( "el12-4.7j", 0x00000, 0x10000, CRC(7a44ef38) SHA1(d7dc277dce08f9d073290e100be4a7ca2e2b82cb) )
	ROM_LOAD( "el13.9j",   0x10000, 0x10000, CRC(04505acb) SHA1(2220efb277884588859375dab9960f04f07273a7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "el10-4.1f", 0x8000, 0x8000, CRC(edfad118) SHA1(10de8805472346fead62460a3fdc09ae26a4e0d5) )

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "el14.14j", 0x00000, 0x08000, CRC(47246177) SHA1(51b025740dc03b04009ac97d8d110ab521894386) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "el00-4.2a", 0x00000, 0x10000, CRC(122da2a8) SHA1(ce72f16abf7e5449c7d044d4b827e8735c3be0ff) )
	ROM_LOAD( "el01-4.3a", 0x20000, 0x10000, CRC(27bf705b) SHA1(196c35aaf3816d3eef4c2af6d146a90a48365d33) )
	ROM_LOAD( "el02-4.5a", 0x40000, 0x10000, CRC(c86fede6) SHA1(97584fa19591651fcfb39d1b2b6306165e93554c) )
	ROM_LOAD( "el03-4.6a", 0x60000, 0x10000, CRC(1d8a855b) SHA1(429261c200dddc62a330be8aea150b2037133188) )

	ROM_REGION( 0x80000, "tiles1", 0 )
	ROM_LOAD( "el05.15a", 0x00000, 0x10000, CRC(1c4f6033) SHA1(4a7dece911166d1ff5f41df6ec5140596206d8d4) )
	ROM_LOAD( "el06.16a", 0x20000, 0x10000, CRC(d24ba794) SHA1(b34b7bbaab4ebdd81c87d363f087cc92e27e8d1c) )
	ROM_LOAD( "el04.13a", 0x40000, 0x10000, CRC(d80a49ce) SHA1(1a92413b5ab53f80e44a954433e69ec5fe2c0aa6) )
	ROM_LOAD( "el07.18a", 0x60000, 0x10000, CRC(6d771fc3) SHA1(f29979f3aa07bdb544fb0c1d773c5558b4533390) )

	ROM_REGION( 0x80000, "tiles2", 0 )
	ROM_LOAD( "el08.7d", 0x00000, 0x08000, CRC(cb0dcf4c) SHA1(e14853f83ee9ba5cbf2eb1e085fee4e65af3cc25) )
	ROM_CONTINUE(        0x40000, 0x08000 )
	ROM_LOAD( "el09.9d", 0x20000, 0x08000, CRC(1fae5be7) SHA1(be6e090b0b82648b385d9b6d11775f3ff40f0af3) )
	ROM_CONTINUE(        0x60000, 0x08000 )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "eh15.12f", 0x0000, 0x0200, CRC(279e540c) SHA1(9e5e707da9f7c403c63d77fa378f22da9906d4e5) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "pt-0.16h", 0x0000, 0x0117, CRC(add5074f) SHA1(352d984ba4d50c437ee9e5d8e8d00b0da6b3b8f3) )
ROM_END

ROM_START( cobracoma )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "el11-4.5j", 0x08000, 0x08000, CRC(6dca6734) SHA1(1d165845680df2f1febd2b7d2f3163d68523496e) )

	ROM_REGION( 0x20000, "mainbank", 0 )
	ROM_LOAD( "el12-4.7j", 0x00000, 0x10000, CRC(7a44ef38) SHA1(d7dc277dce08f9d073290e100be4a7ca2e2b82cb) )
	ROM_LOAD( "el13.9j",   0x10000, 0x10000, CRC(04505acb) SHA1(2220efb277884588859375dab9960f04f07273a7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "el10-4.1f", 0x8000, 0x8000, CRC(edfad118) SHA1(10de8805472346fead62460a3fdc09ae26a4e0d5) )

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "el14.14j", 0x00000, 0x08000, CRC(47246177) SHA1(51b025740dc03b04009ac97d8d110ab521894386) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "el00-4.2a", 0x00000, 0x10000, CRC(122da2a8) SHA1(ce72f16abf7e5449c7d044d4b827e8735c3be0ff) )
	ROM_LOAD( "el01-4.3a", 0x20000, 0x10000, CRC(27bf705b) SHA1(196c35aaf3816d3eef4c2af6d146a90a48365d33) )
	ROM_LOAD( "el02-4.5a", 0x40000, 0x10000, CRC(c86fede6) SHA1(97584fa19591651fcfb39d1b2b6306165e93554c) )
	ROM_LOAD( "el03-4.6a", 0x60000, 0x10000, CRC(1d8a855b) SHA1(429261c200dddc62a330be8aea150b2037133188) )

	ROM_REGION( 0x80000, "tiles1", 0 )
	ROM_LOAD( "el05.15a", 0x00000, 0x10000, CRC(1c4f6033) SHA1(4a7dece911166d1ff5f41df6ec5140596206d8d4) )
	ROM_LOAD( "el06.16a", 0x20000, 0x10000, CRC(d24ba794) SHA1(b34b7bbaab4ebdd81c87d363f087cc92e27e8d1c) )
	ROM_LOAD( "el04.13a", 0x40000, 0x10000, CRC(d80a49ce) SHA1(1a92413b5ab53f80e44a954433e69ec5fe2c0aa6) )
	ROM_LOAD( "el07.18a", 0x60000, 0x10000, CRC(6d771fc3) SHA1(f29979f3aa07bdb544fb0c1d773c5558b4533390) )

	ROM_REGION( 0x80000, "tiles2", 0 )
	ROM_LOAD( "el08.7d", 0x00000, 0x08000, CRC(cb0dcf4c) SHA1(e14853f83ee9ba5cbf2eb1e085fee4e65af3cc25) )
	ROM_CONTINUE(        0x40000, 0x08000 )
	ROM_LOAD( "el09.9d", 0x20000, 0x08000, CRC(1fae5be7) SHA1(be6e090b0b82648b385d9b6d11775f3ff40f0af3) )
	ROM_CONTINUE(        0x60000, 0x08000 )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "eh15.12f", 0x0000, 0x0200, CRC(279e540c) SHA1(9e5e707da9f7c403c63d77fa378f22da9906d4e5) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "pt-0.16h", 0x0000, 0x0117, CRC(add5074f) SHA1(352d984ba4d50c437ee9e5d8e8d00b0da6b3b8f3) )
ROM_END

ROM_START( cobracomb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "el11.5j", 0x08000, 0x08000, CRC(c6a102e3) SHA1(51d9781d13a8a98c9bbdf34fa79ebfd8a152567e) ) // "zero" revision - IE: Original version

	ROM_REGION( 0x20000, "mainbank", 0 )
	ROM_LOAD( "el12.7j", 0x00000, 0x10000, CRC(72b2dab4) SHA1(ff84a04cfe920a0d8105fbbd1f44ee86822cf0d3) ) // "zero" revision - IE: Original version
	ROM_LOAD( "el13.9j", 0x10000, 0x10000, CRC(04505acb) SHA1(2220efb277884588859375dab9960f04f07273a7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "el10.1f", 0x8000, 0x8000, CRC(62ca5e89) SHA1(b04acaccc58846e0d277868a873a440b7f8071b0) )

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "el14.14j", 0x00000, 0x08000, CRC(47246177) SHA1(51b025740dc03b04009ac97d8d110ab521894386) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "el00.2a", 0x00000, 0x10000, CRC(d96b6797) SHA1(01c4a9f2bebb13cba14636690cd5356db73f045e) )
	ROM_LOAD( "el01.3a", 0x20000, 0x10000, CRC(3fef9c02) SHA1(e4b731faf6a2f4e5fed8ba9bd07e0f203981ffec) )
	ROM_LOAD( "el02.5a", 0x40000, 0x10000, CRC(bfae6c34) SHA1(9503a120e11e9466cd9a2931fd44a631d72ca5f0) )
	ROM_LOAD( "el03.6a", 0x60000, 0x10000, CRC(d56790f8) SHA1(1cc7cb9f7102158de14a737e9317a54f01790ba8) )

	ROM_REGION( 0x80000, "tiles1", 0 )
	ROM_LOAD( "el05.15a", 0x00000, 0x10000, CRC(1c4f6033) SHA1(4a7dece911166d1ff5f41df6ec5140596206d8d4) )
	ROM_LOAD( "el06.16a", 0x20000, 0x10000, CRC(d24ba794) SHA1(b34b7bbaab4ebdd81c87d363f087cc92e27e8d1c) )
	ROM_LOAD( "el04.13a", 0x40000, 0x10000, CRC(d80a49ce) SHA1(1a92413b5ab53f80e44a954433e69ec5fe2c0aa6) )
	ROM_LOAD( "el07.18a", 0x60000, 0x10000, CRC(6d771fc3) SHA1(f29979f3aa07bdb544fb0c1d773c5558b4533390) )

	ROM_REGION( 0x80000, "tiles2", 0 )
	ROM_LOAD( "el08.7d", 0x00000, 0x08000, CRC(cb0dcf4c) SHA1(e14853f83ee9ba5cbf2eb1e085fee4e65af3cc25) )
	ROM_CONTINUE(        0x40000, 0x08000 )
	ROM_LOAD( "el09.9d", 0x20000, 0x08000, CRC(1fae5be7) SHA1(be6e090b0b82648b385d9b6d11775f3ff40f0af3) )
	ROM_CONTINUE(        0x60000, 0x08000 )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "eh15.12f", 0x0000, 0x0200, CRC(279e540c) SHA1(9e5e707da9f7c403c63d77fa378f22da9906d4e5) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "pt-0.16h", 0x0000, 0x0117, CRC(add5074f) SHA1(352d984ba4d50c437ee9e5d8e8d00b0da6b3b8f3) )
ROM_END

ROM_START( cobracomj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "eh11.5j", 0x08000, 0x08000, CRC(868637e1) SHA1(8b1e3e045e341bb94b1f6c7d89198b22e6c19de7) )

	ROM_REGION( 0x20000, "mainbank", 0 )
	ROM_LOAD( "eh12.7j", 0x00000, 0x10000, CRC(7c878a83) SHA1(9b2a3083c6dae69626fdab16d97517d30eaa1859) )
	ROM_LOAD( "el13.9j", 0x10000, 0x10000, CRC(04505acb) SHA1(2220efb277884588859375dab9960f04f07273a7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "eh10.1f", 0x8000, 0x8000, CRC(62ca5e89) SHA1(b04acaccc58846e0d277868a873a440b7f8071b0) )

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "eh14.14j", 0x00000, 0x08000, CRC(47246177) SHA1(51b025740dc03b04009ac97d8d110ab521894386) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "eh00.2a", 0x00000, 0x10000, CRC(d96b6797) SHA1(01c4a9f2bebb13cba14636690cd5356db73f045e) )
	ROM_LOAD( "eh01.3a", 0x20000, 0x10000, CRC(3fef9c02) SHA1(e4b731faf6a2f4e5fed8ba9bd07e0f203981ffec) )
	ROM_LOAD( "eh02.5a", 0x40000, 0x10000, CRC(bfae6c34) SHA1(9503a120e11e9466cd9a2931fd44a631d72ca5f0) )
	ROM_LOAD( "eh03.6a", 0x60000, 0x10000, CRC(d56790f8) SHA1(1cc7cb9f7102158de14a737e9317a54f01790ba8) )

	ROM_REGION( 0x80000, "tiles1", 0 )
	ROM_LOAD( "eh05.15a", 0x00000, 0x10000, CRC(1c4f6033) SHA1(4a7dece911166d1ff5f41df6ec5140596206d8d4) )
	ROM_LOAD( "eh06.16a", 0x20000, 0x10000, CRC(d24ba794) SHA1(b34b7bbaab4ebdd81c87d363f087cc92e27e8d1c) )
	ROM_LOAD( "eh04.13a", 0x40000, 0x10000, CRC(d80a49ce) SHA1(1a92413b5ab53f80e44a954433e69ec5fe2c0aa6) )
	ROM_LOAD( "eh07.18a", 0x60000, 0x10000, CRC(6d771fc3) SHA1(f29979f3aa07bdb544fb0c1d773c5558b4533390) )

	ROM_REGION( 0x80000, "tiles2", 0 )
	ROM_LOAD( "eh08.7d", 0x00000, 0x08000, CRC(cb0dcf4c) SHA1(e14853f83ee9ba5cbf2eb1e085fee4e65af3cc25) )
	ROM_CONTINUE(        0x40000, 0x08000 )
	ROM_LOAD( "eh09.9d", 0x20000, 0x08000, CRC(1fae5be7) SHA1(be6e090b0b82648b385d9b6d11775f3ff40f0af3) )
	ROM_CONTINUE(        0x60000, 0x08000 )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "eh15.12f", 0x0000, 0x0200, CRC(279e540c) SHA1(9e5e707da9f7c403c63d77fa378f22da9906d4e5) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "pt-0.16h", 0x0000, 0x0117, CRC(add5074f) SHA1(352d984ba4d50c437ee9e5d8e8d00b0da6b3b8f3) )
ROM_END

// bootleg of the Japanese release manufactured in Italy. Contents are the same but for PROMs and PAL
// main PCB is marked: "LC" on component side ("LC" is the Italian for "Lato Componenti" which translates to "Components Side")
// main PCB is marked: "LS" on solder side ("LS" is the Italian for "Lato Saldature" which translates to "Solders Side")
ROM_START( cobracomjb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cobra4.bin", 0x08000, 0x08000, CRC(868637e1) SHA1(8b1e3e045e341bb94b1f6c7d89198b22e6c19de7) )

	ROM_REGION( 0x20000, "mainbank", 0 )
	ROM_LOAD( "cobra3.bin", 0x00000, 0x10000, CRC(7c878a83) SHA1(9b2a3083c6dae69626fdab16d97517d30eaa1859) )
	ROM_LOAD( "cobra2.bin", 0x10000, 0x10000, CRC(04505acb) SHA1(2220efb277884588859375dab9960f04f07273a7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "cobra5.bin", 0x8000, 0x8000, CRC(62ca5e89) SHA1(b04acaccc58846e0d277868a873a440b7f8071b0) )

	ROM_REGION( 0x08000, "char", 0 )
	ROM_LOAD( "cobra1.bin", 0x00000, 0x08000, CRC(47246177) SHA1(51b025740dc03b04009ac97d8d110ab521894386) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "cob17.bin", 0x00000, 0x10000, CRC(d96b6797) SHA1(01c4a9f2bebb13cba14636690cd5356db73f045e) )
	ROM_LOAD( "cob16.bin", 0x20000, 0x10000, CRC(3fef9c02) SHA1(e4b731faf6a2f4e5fed8ba9bd07e0f203981ffec) )
	ROM_LOAD( "cob15.bin", 0x40000, 0x10000, CRC(bfae6c34) SHA1(9503a120e11e9466cd9a2931fd44a631d72ca5f0) )
	ROM_LOAD( "cob14.bin", 0x60000, 0x10000, CRC(d56790f8) SHA1(1cc7cb9f7102158de14a737e9317a54f01790ba8) )

	ROM_REGION( 0x80000, "tiles1", 0 )
	ROM_LOAD( "cob13.bin", 0x00000, 0x10000, CRC(1c4f6033) SHA1(4a7dece911166d1ff5f41df6ec5140596206d8d4) )
	ROM_LOAD( "cob12.bin", 0x20000, 0x10000, CRC(d24ba794) SHA1(b34b7bbaab4ebdd81c87d363f087cc92e27e8d1c) )
	ROM_LOAD( "cob11.bin", 0x40000, 0x10000, CRC(d80a49ce) SHA1(1a92413b5ab53f80e44a954433e69ec5fe2c0aa6) )
	ROM_LOAD( "cob10.bin", 0x60000, 0x10000, CRC(6d771fc3) SHA1(f29979f3aa07bdb544fb0c1d773c5558b4533390) )

	ROM_REGION( 0x80000, "tiles2", 0 )
	ROM_LOAD( "cobra6.bin", 0x00000, 0x08000, CRC(c991298f) SHA1(3f79773a8def6b79b77c2ca6b8e8dbde5bdcd127) )
	ROM_LOAD( "cobra8.bin", 0x20000, 0x08000, CRC(6bcc5982) SHA1(5bafa9a7a115d2b67f4fd582dc79f7e2848e5010) )
	ROM_LOAD( "cobra7.bin", 0x40000, 0x08000, CRC(f5e267e5) SHA1(4abda964b8ebceeaba7717d4df23a85862934490) )
	ROM_LOAD( "cobra9.bin", 0x60000, 0x08000, CRC(c90443b5) SHA1(2ede9a05e54a662c1b08672921a0390d8ce128a7) )

	ROM_REGION( 0x0500, "proms", 0 )
	ROM_LOAD( "63s481n.3",   0x0000, 0x0200, CRC(279e540c) SHA1(9e5e707da9f7c403c63d77fa378f22da9906d4e5) )
	ROM_LOAD( "am27s21pc.1", 0x0200, 0x0100, CRC(9f6aa3e5) SHA1(518247d4581eee3a078269fcf0c86d182cf622cd) )
	ROM_LOAD( "am27s21pc.2", 0x0300, 0x0100, CRC(af46d1ee) SHA1(281bcc61d9d67b007c1399e228ec6baf6ab5d4ff) )
	ROM_LOAD( "am27s21pc.3", 0x0400, 0x0100, CRC(1e4189e8) SHA1(ce62b19005f565b734d8db3ee49d7c070ed53ad6) )

	ROM_REGION( 0x0104, "plds", 0 )
	ROM_LOAD( "pal16l8a-2cn.bin", 0x0000, 0x0104, CRC(3ef8cf68) SHA1(9410a139fb10628bc612d198f5c9f04b2b34f52f) )
ROM_END


void ghostb_state::init_meikyuhbl()
{
	// this bootleg has the high nibble of the first 0x400 bytes with reversed bits.
	// Address it here instead of hacking the DECO RM-C3 device.
	u8 *proms = memregion("proms")->base();

	for (int i = 0; i < 0x400; i++)
		proms[i] = bitswap<8>(proms[i], 4, 5, 6, 7, 3, 2, 1, 0);

	m_palette->update();
}

void srdarwin_state::init_srdarwinb()
{
	// this bootleg has the sprite ROMs with bytes swapped.
	// just worked here to avoid a new gfxdecode and machine driver.
	u8 *rom = memregion("sprites")->base();

	for (int i = 0; i < 0x30000; i += 4)
	{
		u8 byte1 = rom[i + 1];

		rom[i + 1] = rom[i + 2];
		rom[i + 2] = byte1;
	}
}


/******************************************************************************/

GAME( 1986, lastmisn,   0,        lastmisn,  lastmisn,  lastmisn_state, empty_init,     ROT270, "Data East Corporation", "Last Mission (World revision 8)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, lastmisnu6, lastmisn, lastmisn,  lastmisn,  lastmisn_state, empty_init,     ROT270, "Data East USA",         "Last Mission (US revision 6)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, lastmisnu5, lastmisn, lastmisn,  lastmisn,  lastmisn_state, empty_init,     ROT270, "Data East USA",         "Last Mission (US revision 5)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, lastmisnj,  lastmisn, lastmisn,  lastmisnj, lastmisn_state, empty_init,     ROT270, "Data East Corporation", "Last Mission (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, shackled,   0,        shackled,  shackled,  lastmisn_state, empty_init,     ROT0,   "Data East USA",         "Shackled (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, breywood,   shackled, shackled,  breywood,  lastmisn_state, empty_init,     ROT0,   "Data East Corporation", "Breywood (Japan revision 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, gondo,      0,        gondo,     gondo,     gondo_state,    empty_init,     ROT270, "Data East Corporation", "Gondomania (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, gondou,     gondo,    gondo,     gondo,     gondo_state,    empty_init,     ROT270, "Data East USA",         "Gondomania (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, makyosen,   gondo,    gondo,     gondo,     gondo_state,    empty_init,     ROT270, "Data East Corporation", "Makyou Senshi (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, garyoret,   0,        garyoret,  garyoret,  ghostb_state,   empty_init,     ROT0,   "Data East Corporation", "Garyo Retsuden (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, ghostb,     0,        ghostb,    ghostb,    ghostb_state,   empty_init,     ROT0,   "Data East USA",         "The Real Ghostbusters (US 2 Players, revision 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, ghostb2a,   ghostb,   ghostb,    ghostb2a,  ghostb_state,   empty_init,     ROT0,   "Data East USA",         "The Real Ghostbusters (US 2 Players)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, ghostb3,    ghostb,   ghostb,    ghostb3,   ghostb_state,   empty_init,     ROT0,   "Data East USA",         "The Real Ghostbusters (US 3 Players, revision 3B?)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, ghostb3a,   ghostb,   ghostb,    ghostb3,   ghostb_state,   empty_init,     ROT0,   "Data East USA",         "The Real Ghostbusters (US 3 Players, revision 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, meikyuh,    0,        meikyuh,   meikyuh,   ghostb_state,   empty_init,     ROT0,   "Data East Corporation", "Meikyuu Hunter G (Japan)", MACHINE_SUPPORTS_SAVE ) // modified Ghostbusters
GAME( 1987, meikyuhbl,  meikyuh,  meikyuh,   meikyuh,   ghostb_state,   init_meikyuhbl, ROT0,   "bootleg",               "Meikyuu Hunter G (Japan, bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, csilver,    0,        csilver,   csilver,   csilver_state,  empty_init,     ROT0,   "Data East Corporation", "Captain Silver (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, csilverj,   csilver,  csilver,   csilverj,  csilver_state,  empty_init,     ROT0,   "Data East Corporation", "Captain Silver (Japan, revision 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, csilverja,  csilver,  csilver,   csilver,   csilver_state,  empty_init,     ROT0,   "Data East Corporation", "Captain Silver (Japan, revision 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, oscar,      0,        oscar,     oscar,     oscar_state,    empty_init,     ROT0,   "Data East Corporation", "Psycho-Nics Oscar (World revision 0)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, oscarbl,    oscar,    oscarbl,   oscar,     oscar_state,    empty_init,     ROT0,   "bootleg",               "Psycho-Nics Oscar (World revision 0, bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, oscaru,     oscar,    oscar,     oscarj,    oscar_state,    empty_init,     ROT0,   "Data East USA",         "Psycho-Nics Oscar (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, oscarj1,    oscar,    oscar,     oscarj,    oscar_state,    empty_init,     ROT0,   "Data East Corporation", "Psycho-Nics Oscar (Japan revision 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, oscarj2,    oscar,    oscar,     oscarj,    oscar_state,    empty_init,     ROT0,   "Data East Corporation", "Psycho-Nics Oscar (Japan revision 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, srdarwin,   0,        srdarwin,  srdarwin,  srdarwin_state, empty_init,     ROT270, "Data East Corporation", "SRD: Super Real Darwin (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, srdarwinj,  srdarwin, srdarwin,  srdarwinj, srdarwin_state, empty_init,     ROT270, "Data East Corporation", "SRD: Super Real Darwin (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, srdarwinb,  srdarwin, srdarwinb, srdarwinj, srdarwin_state, init_srdarwinb, ROT270, "bootleg",               "SRD: Super Real Darwin (Japan, bootleg)", MACHINE_SUPPORTS_SAVE )

// Unlike most Deco games of this period Cobra Command does not seem to have a Data East USA release.  Instead the Data East Corporation release
// was used in the US as evidenced by boards with the EL romset bearing AAMA seal stickers (American Amusement Machine Association)
GAME( 1988, cobracom,   0,        cobracom,  cobracom,  oscar_state,    empty_init,     ROT0,   "Data East Corporation", "Cobra-Command (World/US revision 5)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, cobracoma,  cobracom, cobracom,  cobracom,  oscar_state,    empty_init,     ROT0,   "Data East Corporation", "Cobra-Command (World/US revision 4)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, cobracomb,  cobracom, cobracom,  cobracom,  oscar_state,    empty_init,     ROT0,   "Data East Corporation", "Cobra-Command (World/US)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, cobracomj,  cobracom, cobracom,  cobracom,  oscar_state,    empty_init,     ROT0,   "Data East Corporation", "Cobra-Command (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, cobracomjb, cobracom, cobracom,  cobracom,  oscar_state,    empty_init,     ROT0,   "bootleg",               "Cobra-Command (Japan, bootleg)", MACHINE_SUPPORTS_SAVE )
