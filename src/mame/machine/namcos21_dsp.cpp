// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino, David Haywood

/*

Common code for the original Namco System 21 DSP board, with a single DSP
used by Winning Run, Driver's Eyes

TODO: handle protection properly and with callbacks
      some of the list processing should probably be in the 3d device, split it out

*/

#include "emu.h"
#include "namcos21_dsp.h"

DEFINE_DEVICE_TYPE(NAMCOS21_DSP, namcos21_dsp_device, "namcos21_dsp_device", "Namco System 21 DSP Setup (1x TMS320C25 type)")

namcos21_dsp_device::namcos21_dsp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NAMCOS21_DSP, tag, owner, clock),
	m_dsp(*this, "dsp"),
	m_winrun_dspbios(*this,"winrun_dspbios"),
	m_winrun_polydata(*this,"winrun_polydata"),
	m_ptrom16(*this,"point16"),
	m_renderer(*this, finder_base::DUMMY_TAG)
{
}

void namcos21_dsp_device::device_start()
{
	m_winrun_dspcomram = std::make_unique<uint16_t[]>(0x1000*2);
	m_suspend_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(namcos21_dsp_device::suspend_callback),this));
	m_pointram = std::make_unique<uint8_t[]>(PTRAM_SIZE);
	m_pointram_idx = 0;
}

TIMER_CALLBACK_MEMBER(namcos21_dsp_device::suspend_callback)
{
	m_dsp->suspend(SUSPEND_REASON_HALT, true);
}

void namcos21_dsp_device::device_reset()
{
	m_poly_frame_width = m_renderer->get_width();
	m_poly_frame_height = m_renderer->get_height();
	// can't suspend directly from here, needs to be on a timer?
	m_suspend_timer->adjust(attotime::zero);
}


READ16_MEMBER(namcos21_dsp_device::winrun_dspcomram_r)
{
	int bank = 1-(m_winrun_dspcomram_control[0x4/2]&1);
	uint16_t *mem = &m_winrun_dspcomram[0x1000*bank];
	return mem[offset];
}
WRITE16_MEMBER(namcos21_dsp_device::winrun_dspcomram_w)
{
	int bank = 1-(m_winrun_dspcomram_control[0x4/2]&1);
	uint16_t *mem = &m_winrun_dspcomram[0x1000*bank];
	COMBINE_DATA( &mem[offset] );
}

READ16_MEMBER(namcos21_dsp_device::winrun_cuskey_r)
{
	int pc = m_dsp->pc();
	switch( pc )
	{
	case 0x0064: /* winrun91 */
		return 0xFEBB;
	case 0x006c: /* winrun91 */
		return 0xFFFF;
	case 0x0073: /* winrun91 */
		return 0x0144;

	case 0x0075: /* winrun */
		return 0x24;

	default:
		break;
	}
	return 0;
}

WRITE16_MEMBER(namcos21_dsp_device::winrun_cuskey_w)
{
}

void namcos21_dsp_device::winrun_flush_poly()
{
	if( m_winrun_poly_index>0 )
	{
		const uint16_t *pSource = m_winrun_poly_buf;
		uint16_t color;
		int sx[4], sy[4], zcode[4];
		int j;
		color = *pSource++;
		if( color&0x8000 )
		{ /* direct-draw */
			for( j=0; j<4; j++ )
			{
				sx[j] = m_poly_frame_width/2  + (int16_t)*pSource++;
				sy[j] = m_poly_frame_height/2 + (int16_t)*pSource++;
				zcode[j] = *pSource++;
			}
			m_renderer->draw_quad(sx, sy, zcode, color&0x7fff);
		}
		else
		{
			int quad_idx = color*6;
			for(;;)
			{
				uint8_t code = m_pointram[quad_idx++];
				color = m_pointram[quad_idx++];
				for( j=0; j<4; j++ )
				{
					uint8_t vi = m_pointram[quad_idx++];
					sx[j] = m_poly_frame_width/2  + (int16_t)pSource[vi*3+0];
					sy[j] = m_poly_frame_height/2 + (int16_t)pSource[vi*3+1];
					zcode[j] = pSource[vi*3+2];
				}
				m_renderer->draw_quad(sx, sy, zcode, color&0x7fff);
				if( code&0x80 )
				{ /* end-of-quadlist marker */
					break;
				}
			}
		}
		m_winrun_poly_index = 0;
	}
} /* winrun_flushpoly */

READ16_MEMBER(namcos21_dsp_device::winrun_poly_reset_r)
{
	winrun_flush_poly();
	return 0;
}

WRITE16_MEMBER(namcos21_dsp_device::winrun_dsp_render_w)
{
	if( m_winrun_poly_index<WINRUN_MAX_POLY_PARAM )
	{
		m_winrun_poly_buf[m_winrun_poly_index++] = data;
	}
	else
	{
		logerror( "WINRUN_POLY_OVERFLOW\n" );
	}
}

WRITE16_MEMBER(namcos21_dsp_device::winrun_dsp_pointrom_addr_w)
{
	if( offset==0 )
	{ /* port 8 */
		m_winrun_pointrom_addr = data;
	}
	else
	{ /* port 9 */
		m_winrun_pointrom_addr |= (data<<16);
	}
}

READ16_MEMBER(namcos21_dsp_device::winrun_dsp_pointrom_data_r)
{
	return m_ptrom16[m_winrun_pointrom_addr++];
}

WRITE16_MEMBER(namcos21_dsp_device::winrun_dsp_complete_w)
{
	if( data )
	{
		winrun_flush_poly();
		m_dsp->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
		m_renderer->swap_and_clear_poly_framebuffer();
	}
}

READ16_MEMBER(namcos21_dsp_device::winrun_table_r)
{
	return m_winrun_polydata[offset];
}

WRITE16_MEMBER(namcos21_dsp_device::winrun_dspbios_w)
{
	COMBINE_DATA( &m_winrun_dspbios[offset] );
	if( offset==0xfff ) // is this the real trigger?
	{
		m_winrun_dsp_alive = 1;
		m_dsp->resume(SUSPEND_REASON_HALT);
	}
}

//380000 : read : dsp status? 1 = busy
//380000 : write(0x01) - done before dsp comram init
//380004 : dspcomram bank, as seen by 68k
//380008 : read : state?

READ16_MEMBER(namcos21_dsp_device::winrun_68k_dspcomram_r)
{
	int bank = m_winrun_dspcomram_control[0x4/2]&1;
	uint16_t *mem = &m_winrun_dspcomram[0x1000*bank];
	return mem[offset];
}

WRITE16_MEMBER(namcos21_dsp_device::winrun_68k_dspcomram_w)
{
	int bank = m_winrun_dspcomram_control[0x4/2]&1;
	uint16_t *mem = &m_winrun_dspcomram[0x1000*bank];
	COMBINE_DATA( &mem[offset] );
}

READ16_MEMBER(namcos21_dsp_device::winrun_dspcomram_control_r)
{
	return m_winrun_dspcomram_control[offset];
}

WRITE16_MEMBER(namcos21_dsp_device::winrun_dspcomram_control_w)
{
	COMBINE_DATA( &m_winrun_dspcomram_control[offset] );
}


void namcos21_dsp_device::winrun_dsp_program(address_map &map)
{
	// MCU is used in external program mode, program is uploaded to shared RAM by the 68k
	map(0x0000, 0x0fff).ram().share("winrun_dspbios");
}

void namcos21_dsp_device::winrun_dsp_data(address_map &map)
{
	map(0x2000, 0x200f).rw(FUNC(namcos21_dsp_device::winrun_cuskey_r), FUNC(namcos21_dsp_device::winrun_cuskey_w));
	map(0x4000, 0x4fff).rw(FUNC(namcos21_dsp_device::winrun_dspcomram_r), FUNC(namcos21_dsp_device::winrun_dspcomram_w));
	map(0x8000, 0xffff).r(FUNC(namcos21_dsp_device::winrun_table_r));
}

void namcos21_dsp_device::winrun_dsp_io(address_map &map)
{
	map(0x08, 0x09).rw(FUNC(namcos21_dsp_device::winrun_dsp_pointrom_data_r), FUNC(namcos21_dsp_device::winrun_dsp_pointrom_addr_w));
	map(0x0a, 0x0a).w(FUNC(namcos21_dsp_device::winrun_dsp_render_w));
	map(0x0b, 0x0b).nopw();
	map(0x0c, 0x0c).w(FUNC(namcos21_dsp_device::winrun_dsp_complete_w));
}


void namcos21_dsp_device::device_add_mconfig(machine_config &config)
{
	tms32025_device& dsp(TMS32025(config, m_dsp, 24000000*2)); /* 48 MHz? overclocked */
	dsp.set_addrmap(AS_PROGRAM, &namcos21_dsp_device::winrun_dsp_program);
	dsp.set_addrmap(AS_DATA, &namcos21_dsp_device::winrun_dsp_data);
	dsp.set_addrmap(AS_IO, &namcos21_dsp_device::winrun_dsp_io);
	dsp.bio_in_cb().set(FUNC(namcos21_dsp_device::winrun_poly_reset_r));
	dsp.hold_in_cb().set_constant(0);
	dsp.hold_ack_out_cb().set_nop();
	dsp.xf_out_cb().set_nop();
}

WRITE16_MEMBER(namcos21_dsp_device::pointram_control_w)
{
	COMBINE_DATA( &m_pointram_control );
	m_pointram_idx = 0; /* HACK */
}

READ16_MEMBER(namcos21_dsp_device::pointram_data_r)
{
	return m_pointram[m_pointram_idx];
}

WRITE16_MEMBER(namcos21_dsp_device::pointram_data_w)
{
	if( ACCESSING_BITS_0_7 )
	{
		m_pointram[m_pointram_idx++] = data;
		m_pointram_idx &= (PTRAM_SIZE-1);
	}
}
