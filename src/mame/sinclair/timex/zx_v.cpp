// license:GPL-2.0+
// copyright-holders: Olivier Galibert, Juergen Buchmueller, Krzysztof Strzecha, Robbbert
/***************************************************************************
    zx.c

    video hardware
    Juergen Buchmueller <pullmoll@t-online.de>, Dec 1999

    The ZX has a very unorthodox video RAM system.  To start a scanline,
    the CPU must jump to video RAM at 0xC000, which is a mirror of the
    RAM at 0x4000.  The video chip (ULA?) pulls a switcharoo and changes
    the video bytes as they are fetched by the CPU.

    The video chip draws the scanline until a HALT instruction (0x76) is
    reached, which indicates no further video RAM for this scanline.  Any
    other video byte is used to generate a tile and at the same time,
    appears to the CPU as a NOP (0x00) instruction.

****************************************************************************/

#include "emu.h"
#include "zx.h"


TIMER_CALLBACK_MEMBER(zx_state::zx_ula_hsync)
{
	m_hsync_active = !m_hsync_active;
	if(m_hsync_active)
		m_ypos++;
	if(m_nmi_generator_active) {
		m_nmi_on = m_hsync_active;
		m_maincpu->set_input_line(INPUT_LINE_NMI, m_nmi_on ? ASSERT_LINE : CLEAR_LINE);
	}
	recalc_hsync();
}

void zx_state::refresh_w(offs_t offset, uint8_t data)
{
	if((offset ^ m_prev_refresh) & 0x40)
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, offset & 0x40 ? CLEAR_LINE : ASSERT_LINE);
	m_prev_refresh = offset;
	if(m_ula_char_buffer != 0xffff) {
		uint64_t time = m_maincpu->total_cycles();
		int x = 2*((time-m_base_vsync_clock) % 207);
		int y = (time-m_base_vsync_clock) / 207;
		uint8_t pixels;
		if(m_region_gfx1)
			pixels = m_region_gfx1->base()[((m_ula_char_buffer & 0x3f) << 3) | (m_ypos & 7)];
		else
			pixels = m_program->read_byte((offset & 0xfe00) | ((m_ula_char_buffer & 0x3f) << 3) | (m_ypos & 7));
		if(m_ula_char_buffer & 0x80)
			pixels = ~pixels;
		if(x < 384-8 && y < 311) {
			uint16_t *dest = &m_bitmap_render->pix(y, x);
			for(int i=0; i<8; i++)
				*dest++ |= pixels & (0x80 >> i) ? 1 : 0;
		}
		m_ula_char_buffer = 0xffff;
	}
}

void zx_state::recalc_hsync()
{
	uint64_t time = machine().time().as_ticks(m_maincpu->clock());
	uint32_t step = (time - m_base_vsync_clock) % 207;
	uint32_t delta;
	if (m_hsync_active)
		delta = 207 - step;
	else {
		if(step < 192)
			delta = 192 - step;
		else
			delta = 399 - step;
	}

	m_ula_hsync->adjust(m_maincpu->cycles_to_attotime(delta));
}

uint8_t zx_state::ula_low_r(offs_t offset)
{
	uint8_t cdata = m_program->read_byte(offset);
	if(machine().side_effects_disabled())
		return cdata;

	if(m_maincpu->state_int(Z80_HALT))
		return cdata;

	if(m_nmi_on) {
		uint64_t time = m_maincpu->total_cycles();
		int pos = (time-m_base_vsync_clock) % 207;
		if(pos >= 192)
			m_maincpu->adjust_icount(pos - 207);
	}
	return cdata;
}

uint8_t zx_state::ula_high_r(offs_t offset)
{
	uint8_t cdata = m_program->read_byte(offset);

	if(machine().side_effects_disabled())
		return cdata;

	if(m_maincpu->state_int(Z80_HALT))
		return cdata;

	if(m_nmi_on) {
		uint64_t time = m_maincpu->total_cycles();
		int pos = (time-m_base_vsync_clock) % 207;
		if(pos >= 192)
			m_maincpu->adjust_icount(pos - 207);
	}

	if(cdata & 0x40)
		return cdata;

	m_ula_char_buffer = cdata;
	return 0x00; // nop
}

void zx_state::video_start()
{
	m_ula_hsync = timer_alloc(FUNC(zx_state::zx_ula_hsync), this);
	m_ula_char_buffer = 0xffff;

	m_bitmap_render = std::make_unique<bitmap_ind16>(384, 311);
	m_bitmap_buffer = std::make_unique<bitmap_ind16>(384, 311);
}

uint32_t zx_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, *m_bitmap_buffer, 0, 0, 0, 0, cliprect);
	return 0;
}
