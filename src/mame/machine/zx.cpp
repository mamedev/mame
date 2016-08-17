// license:GPL-2.0+
// copyright Olivier Galibert, Buchmueller, Krzysztof Strzecha, Robbbert
/***************************************************************************
    zx.c

    machine driver
    Juergen Buchmueller <pullmoll@t-online.de>, Dec 1999

****************************************************************************/

#include "includes/zx.h"

DRIVER_INIT_MEMBER(zx_state,zx)
{
	m_program = &m_maincpu->space(AS_PROGRAM);
	m_tape_input = timer_alloc(TIMER_TAPE_INPUT);

	if(m_ram->size() == 32768)
		m_program->unmap_readwrite(0x8000, 0xbfff);
	else if(m_ram->size() == 16384)
		m_program->unmap_readwrite(0x8000, 0xffff);
	else if(m_ram->size() < 16384)
		m_program->unmap_readwrite(0x4000 + m_ram->size(), 0xffff);
}

void zx_state::machine_reset()
{
	m_prev_refresh = 0xff;

	m_vsync_active = false;
	m_base_vsync_clock = 0;
	m_ypos = 0;

	m_nmi_on = false;
	m_nmi_generator_active = false;

	m_cassette_cur_level = 0;

	m_tape_input->adjust(attotime::from_hz(44100), 0, attotime::from_hz(44100));
}

void zx_state::zx_tape_input()
{
	m_cassette_cur_level = m_cassette->input();
}

void zx_state::drop_sync()
{
	if (m_vsync_active) {
		UINT64 time = m_maincpu->total_cycles();
		m_vsync_active = false;
		m_cassette->output(-1.0);

		int xs = 2*((m_vsync_start_time - m_base_vsync_clock) % 207);
		int ys = (m_vsync_start_time - m_base_vsync_clock) / 207;
		int xe = 2*((time - m_base_vsync_clock) % 207);
		int ye = (time - m_base_vsync_clock) / 207;
		if(xs >= 384) {
			xs = 0;
			ys++;
		}
		if(xe >= 384) {
			xe = 0;
			ye++;
		}
		if(ys < 311) {
			if(ye > 310) {
				ye = 311;
				xe = 0;
			}
			if(ys == ye) {
				UINT16 *dest = &m_bitmap_render->pix16(ys, xs);
				for(int x = xs; x < xe; x++)
					*dest++ = 1;
			} else {
				UINT16 *dest = &m_bitmap_render->pix16(ys, xs);
				for(int x = xs; x < 384; x++)
					*dest++ = 1;
				for(int y = ys+1; y < ye; y++) {
					dest = &m_bitmap_render->pix16(y, 0);
					for(int x = 0; x<384; x++)
						*dest++ = 1;
				}
				dest = &m_bitmap_render->pix16(ye, 0);
				for(int x = 0; x < xe; x++)
					*dest++ = 1;
			}
		}

		// Short is hsync
		if(time - m_vsync_start_time > 1000) {
			// Ignore too short frame times, they're cassette output
			if(time - m_base_vsync_clock > 52000) {
				logerror("frame time %d\n", int(time - m_base_vsync_clock));

				rectangle rect(0, 383, 0, 310);
				copybitmap(*m_bitmap_buffer, *m_bitmap_render, 0, 0, 0, 0, rect);
				m_bitmap_render->fill(0);
				m_base_vsync_clock = time;
				m_ypos = 0;
			}
			if(m_nmi_on)
				m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
			m_nmi_on = m_hsync_active = false;
			recalc_hsync();
		} else
			m_ypos = ((time-m_base_vsync_clock)%207) < 192 ? 0 : -1;
	}
}

READ8_MEMBER( zx_state::zx80_io_r )
{
	/* port FE = read keyboard, NTSC/PAL diode, and cass bit; turn off HSYNC-generator/cass-out
	    The upper 8 bits are used to select a keyboard scan line */

	UINT8 data = 0xff;

	if (!(offset & 0x01))
	{
		if ((offset & 0x0100) == 0)
			data &= m_io_row0->read();
		if ((offset & 0x0200) == 0)
			data &= m_io_row1->read();
		if ((offset & 0x0400) == 0)
			data &= m_io_row2->read();
		if ((offset & 0x0800) == 0)
			data &= m_io_row3->read();
		if ((offset & 0x1000) == 0)
			data &= m_io_row4->read();
		if ((offset & 0x2000) == 0)
			data &= m_io_row5->read();
		if ((offset & 0x4000) == 0)
			data &= m_io_row6->read();
		if ((offset & 0x8000) == 0)
			data &= m_io_row7->read();

		if (!m_io_config->read())
			data &= ~0x40;

		m_cassette->output(+1.0);

		if (!m_vsync_active && !m_nmi_generator_active) {
			m_vsync_active = true;
			m_vsync_start_time = m_maincpu->total_cycles();
		}

		if(m_cassette_cur_level <= 0)
			data &= 0x7f;
	}

	return data;
}

READ8_MEMBER( zx_state::zx81_io_r )
{
/* port FB = read printer status, not emulated
    FE = read keyboard, NTSC/PAL diode, and cass bit; turn off HSYNC-generator/cass-out
    The upper 8 bits are used to select a keyboard scan line */

	UINT8 data = 0xff;

	if (!(offset & 0x01))
	{
		if ((offset & 0x0100) == 0)
			data &= m_io_row0->read();
		if ((offset & 0x0200) == 0)
			data &= m_io_row1->read();
		if ((offset & 0x0400) == 0)
			data &= m_io_row2->read();
		if ((offset & 0x0800) == 0)
			data &= m_io_row3->read();
		if ((offset & 0x1000) == 0)
			data &= m_io_row4->read();
		if ((offset & 0x2000) == 0)
			data &= m_io_row5->read();
		if ((offset & 0x4000) == 0)
			data &= m_io_row6->read();
		if ((offset & 0x8000) == 0)
			data &= m_io_row7->read();

		if (!m_io_config->read())
			data &= ~0x40;

		m_cassette->output(+1.0);

		if (!m_vsync_active && !m_nmi_generator_active) {
			m_vsync_active = true;
			m_vsync_start_time = m_maincpu->total_cycles();
		}

		if(m_cassette_cur_level <= 0)
			data &= 0x7f;
	}

	return data;
}

READ8_MEMBER( zx_state::pc8300_io_r )
{
/* port F5 = sound
    F6 = unknown
    FB = read printer status, not emulated
    FE = read keyboard and cass bit; turn off HSYNC-generator/cass-out
    The upper 8 bits are used to select a keyboard scan line.
    No TV diode */

	UINT8 data = 0xff;
	UINT8 offs = offset & 0xff;

	if (offs == 0xf5)
	{
		m_speaker_state ^= 1;
		m_speaker->level_w(m_speaker_state);
	}
	else
	if (offs == 0xfe)
	{
		if ((offset & 0x0100) == 0)
			data &= m_io_row0->read();
		if ((offset & 0x0200) == 0)
			data &= m_io_row1->read();
		if ((offset & 0x0400) == 0)
			data &= m_io_row2->read();
		if ((offset & 0x0800) == 0)
			data &= m_io_row3->read();
		if ((offset & 0x1000) == 0)
			data &= m_io_row4->read();
		if ((offset & 0x2000) == 0)
			data &= m_io_row5->read();
		if ((offset & 0x4000) == 0)
			data &= m_io_row6->read();
		if ((offset & 0x8000) == 0)
			data &= m_io_row7->read();

		m_cassette->output(+1.0);
		if(m_cassette_cur_level <= 0)
			data &= 0x7f;
	}

	return data;
}

READ8_MEMBER( zx_state::pow3000_io_r )
{
/* port 7E = read NTSC/PAL diode
    F5 = sound
    F6 = unknown
    FB = read printer status, not emulated
    FE = read keyboard and cass bit; turn off HSYNC-generator/cass-out
    The upper 8 bits are used to select a keyboard scan line */

	UINT8 data = 0xff;
	UINT8 offs = offset & 0xff;

	if (offs == 0x7e)
	{
		data = (m_io_config->read());
	}
	else
	if (offs == 0xf5)
	{
		m_speaker_state ^= 1;
		m_speaker->level_w(m_speaker_state);
	}
	else
	if (offs == 0xfe)
	{
		if ((offset & 0x0100) == 0)
			data &= m_io_row0->read();
		if ((offset & 0x0200) == 0)
			data &= m_io_row1->read();
		if ((offset & 0x0400) == 0)
			data &= m_io_row2->read();
		if ((offset & 0x0800) == 0)
			data &= m_io_row3->read();
		if ((offset & 0x1000) == 0)
			data &= m_io_row4->read();
		if ((offset & 0x2000) == 0)
			data &= m_io_row5->read();
		if ((offset & 0x4000) == 0)
			data &= m_io_row6->read();
		if ((offset & 0x8000) == 0)
			data &= m_io_row7->read();

		m_cassette->output(+1.0);
		if(m_cassette_cur_level <= 0)
			data &= 0x7f;
	}

	return data;
}

WRITE8_MEMBER( zx_state::zx80_io_w )
{
/* port FF = write HSYNC and cass data */

	UINT8 offs = offset & 0xff;

	if (offs == 0xff)
		m_cassette->output(-1.0);
}

WRITE8_MEMBER( zx_state::zx81_io_w )
{
/* port F5 = unknown, pc8300/pow3000/lambda only
    F6 = unknown, pc8300/pow3000/lambda only
    FB = write data to printer, not emulated
    FD = turn off NMI generator
    FE = turn on NMI generator
    FF = write HSYNC and cass data */

	if (!(offset & 0x01) && !m_nmi_generator_active) {
		m_nmi_generator_active = true;
		m_nmi_on = m_hsync_active;
		m_maincpu->set_input_line(INPUT_LINE_NMI, m_nmi_on ? ASSERT_LINE : CLEAR_LINE);
		recalc_hsync();
	}

	if (!(offset & 0x02) && m_nmi_generator_active) {
		m_nmi_generator_active = false;
		if(m_nmi_on) {
			m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
			m_nmi_on = false;
		}
	}

	drop_sync();
}
