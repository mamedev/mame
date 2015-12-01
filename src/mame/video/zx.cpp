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

#include "includes/zx.h"


void zx_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_TAPE_INPUT:
		zx_tape_input();
		break;
	case TIMER_ULA_HSYNC:
		zx_ula_hsync();
		break;
	default:
		assert_always(FALSE, "Unknown id in zx_state::device_timer");
	}
}


void zx_state::zx_ula_hsync()
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

WRITE16_MEMBER(zx_state::refresh_w)
{
	if((data ^ m_prev_refresh) & 0x40)
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, data & 0x40 ? CLEAR_LINE : ASSERT_LINE);
	m_prev_refresh = data;
	if(m_ula_char_buffer != 0xffff) {
		UINT64 time = m_maincpu->total_cycles();
		int x = 2*((time-m_base_vsync_clock) % 207);
		int y = (time-m_base_vsync_clock) / 207;
		UINT8 pixels;
		if(m_region_gfx1)
			pixels = m_region_gfx1->base()[((m_ula_char_buffer & 0x3f) << 3) | (m_ypos & 7)];
		else
			pixels = m_program->read_byte((data & 0xfe00) | ((m_ula_char_buffer & 0x3f) << 3) | (m_ypos & 7));
		if(m_ula_char_buffer & 0x80)
			pixels = ~pixels;
		if(x < 384-8 && y < 311) {
			UINT16 *dest = &m_bitmap_render->pix16(y, x);
			for(int i=0; i<8; i++)
				*dest++ |= pixels & (0x80 >> i) ? 1 : 0;
		}
		m_ula_char_buffer = 0xffff;
	}
}

void zx_state::recalc_hsync()
{
	UINT64 time = machine().time().as_ticks(m_maincpu->clock());
	UINT32 step = (time - m_base_vsync_clock) % 207;
	UINT32 delta;
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

READ8_MEMBER(zx_state::ula_low_r)
{
	UINT8 cdata = m_program->read_byte(offset);
	if(space.debugger_access())
		return cdata;

	if(m_maincpu->state_int(Z80_HALT))
		return cdata;

	if(m_nmi_on) {
		UINT64 time = m_maincpu->total_cycles();
		int pos = (time-m_base_vsync_clock) % 207;
		if(pos >= 192)
			m_maincpu->adjust_icount(pos - 207);
	}
	return cdata;
}

READ8_MEMBER(zx_state::ula_high_r)
{
	UINT8 cdata = m_program->read_byte(offset);

	if(space.debugger_access())
		return cdata;

	if(m_maincpu->state_int(Z80_HALT))
		return cdata;

	if(m_nmi_on) {
		UINT64 time = m_maincpu->total_cycles();
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
	m_ula_hsync = timer_alloc(TIMER_ULA_HSYNC);
	m_ula_char_buffer = 0xffff;

	m_bitmap_render = auto_bitmap_ind16_alloc(machine(), 384, 311);
	m_bitmap_buffer = auto_bitmap_ind16_alloc(machine(), 384, 311);
}

UINT32 zx_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, *m_bitmap_buffer, 0, 0, 0, 0, cliprect);
	return 0;
}
