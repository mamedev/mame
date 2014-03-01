/***************************************************************************

    INTEL 8275 Programmable CRT Controller implementation

    25-05-2008 Initial implementation [Miodrag Milanovic]

    Copyright MESS team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "emu.h"
#include "i8275.h"

#define I8275_COMMAND_RESET             0
#define I8275_COMMAND_START_DISPLAY     1
#define I8275_COMMAND_STOP_DISPLAY      2
#define I8275_COMMAND_READ_LIGHT_PEN    3
#define I8275_COMMAND_LOAD_CURSOR       4
#define I8275_COMMAND_ENABLE_INTERRUPT  5
#define I8275_COMMAND_DISABLE_INTERRUPT 6
#define I8275_COMMAND_PRESET_COUNTERS   7

#define I8275_PARAM_RESET               4
#define I8275_PARAM_READ_LIGHT_PEN      2
#define I8275_PARAM_LOAD_CURSOR         2

#define I8275_PARAM_NONE                0
#define I8275_PARAM_READ                1
#define I8275_PARAM_WRITE               2

#define I8275_STATUS_FIFO_OVERRUN       0x01
#define I8275_STATUS_DMA_UNDERRUN       0x02
#define I8275_STATUS_VIDEO_ENABLE       0x04
#define I8275_STATUS_IMPROPER_COMMAND   0x08
#define I8275_STATUS_LIGHT_PEN          0x10
#define I8275_STATUS_INTERRUPT_REQUEST  0x20
#define I8275_STATUS_INTERRUPT_ENABLE   0x40

#define I8275_ROW_TYPE_NORMAL           0
#define I8275_ROW_TYPE_SPACED           1

#define I8275_LINE_COUNTER_MODE_0       0
#define I8275_LINE_COUNTER_MODE_1       1

#define I8275_FIELD_ATTRIBUTE_TRANSPARENT       0
#define I8275_FIELD_ATTRIBUTE_NONTRANSPARENT    1

#define I8275_CURSOR_BLINK_REVERSED     0
#define I8275_CURSOR_BLINK_UNDERLINE    1
#define I8275_CURSOR_NONBLINK_REVERSED  2
#define I8275_CURSOR_NONBLINK_UNDERLINE 3


#define VERBOSE         1

#define LOG(x)      do { if (VERBOSE) logerror x; } while (0)


struct i8275_t
{
};

/* Register Access */
UINT8 i8275_device::get_parameter_light_pen(offs_t offset)
{
	UINT8 val = 0;

	switch(offset) {
		case 0 :
				val = m_light_pen_col;
				break;
		case 1 :
				val = m_light_pen_row;
				break;
	}
	return val;
}

READ8_MEMBER( i8275_device::read )
{
	UINT8 val;

	if (offset & 0x01)
	{
		/* Status register */
		val = m_status_reg;
		/* status reset after read */
		m_status_reg &= ~I8275_STATUS_FIFO_OVERRUN;
		m_status_reg &= ~I8275_STATUS_DMA_UNDERRUN;
		m_status_reg &= ~I8275_STATUS_IMPROPER_COMMAND;
		m_status_reg &= ~I8275_STATUS_LIGHT_PEN;
		m_status_reg &= ~I8275_STATUS_INTERRUPT_REQUEST;
	}
	else
	{
		/* Parameter register */
		val = 0x00;
		if (m_param_type==I8275_PARAM_READ) {
			if (m_num_of_params > 0) {
				val = get_parameter_light_pen(2 - m_num_of_params);
				m_num_of_params--;
			} else {
				m_status_reg |= I8275_STATUS_IMPROPER_COMMAND;
			}
		} else {
			LOG(("i8275 : ERROR reading parameter\n"));
			m_status_reg |= I8275_STATUS_IMPROPER_COMMAND;
		}
	}
	return val;
}

void i8275_device::recompute_parameters()
{
	int horiz_pix_total = 0;
	int vert_pix_total = 0;
	rectangle visarea;

	horiz_pix_total = (m_chars_per_row + 1) * m_width;
	vert_pix_total  = (m_lines_per_row + 1) * (m_rows_per_frame + 1);
	if (m_rows_type==1) {
		vert_pix_total *= 2; // Use of spaced rows
	}

	visarea.set(0, horiz_pix_total - 1, 0, vert_pix_total - 1);

	m_screen->configure(horiz_pix_total, vert_pix_total, visarea,
				m_screen->frame_period().attoseconds);
}

void i8275_device::set_parameter_reset(offs_t offset, UINT8 data)
{
	switch(offset) {
		case 0 :
				m_rows_type = (data >> 7) & 1;
				m_chars_per_row = data & 0x7f;
				break;
		case 1 :
				m_vert_retrace_rows = (data >> 6) & 3;
				m_rows_per_frame = data & 0x3f;
				break;
		case 2 :
				m_undeline_line_num = (data >> 4) & 0x0f;
				m_lines_per_row = data & 0x0f;
				break;
		case 3 :
				m_line_counter_mode = (data >> 7) & 1;
				m_field_attribute_mode = (data >> 6) & 1;
				m_cursor_format  = (data >> 4) & 3;
				m_hor_retrace_count = data & 0x0f;
				break;
	}
}

void i8275_device::set_parameter_cursor(offs_t offset, UINT8 data)
{
	switch(offset) {
		case 0 :
				m_cursor_col = data;
				break;
		case 1 :
				m_cursor_row = data;
				break;
	}
}


WRITE8_MEMBER( i8275_device::write )
{
	if (offset & 0x01)
	{
		/* Command register */
		if (m_num_of_params != 0) {
			m_status_reg |= I8275_STATUS_IMPROPER_COMMAND;
			return;
		}
		m_current_command = (data >> 5) & 7;
		m_num_of_params = I8275_PARAM_NONE;
		m_param_type = I8275_PARAM_NONE;
		switch(m_current_command) {
			case I8275_COMMAND_RESET        :
											m_num_of_params = I8275_PARAM_RESET;
											m_param_type = I8275_PARAM_WRITE;
											/* set status register */
											m_status_reg &= ~I8275_STATUS_INTERRUPT_ENABLE;
											m_status_reg &= ~I8275_STATUS_VIDEO_ENABLE;
											break;
			case I8275_COMMAND_START_DISPLAY:
											m_burst_space_code = (data >> 2) & 7;
											m_burst_count_code = data & 3;
											/* set status register */
											m_status_reg |= I8275_STATUS_VIDEO_ENABLE;
											m_status_reg |= I8275_STATUS_INTERRUPT_ENABLE;
											recompute_parameters();
											break;
			case I8275_COMMAND_STOP_DISPLAY :
											/* set status register */
											m_status_reg &= ~I8275_STATUS_VIDEO_ENABLE;
											break;
			case I8275_COMMAND_READ_LIGHT_PEN:
											m_num_of_params = I8275_PARAM_READ_LIGHT_PEN;
											m_param_type = I8275_PARAM_READ;
											break;
			case I8275_COMMAND_LOAD_CURSOR  :
											m_num_of_params = I8275_PARAM_LOAD_CURSOR;
											m_param_type = I8275_PARAM_WRITE;
											break;
			case I8275_COMMAND_ENABLE_INTERRUPT :
											/* set status register */
											m_status_reg |= I8275_STATUS_INTERRUPT_ENABLE;
											break;
			case I8275_COMMAND_DISABLE_INTERRUPT:
											/* set status register */
											m_status_reg &= ~I8275_STATUS_INTERRUPT_ENABLE;
											break;
			case I8275_COMMAND_PRESET_COUNTERS  :
											break;
		}
	}
	else
	{
		/* Parameter register */
		if (m_param_type==I8275_PARAM_WRITE) {
			if (m_num_of_params > 0) {
				if (m_current_command == I8275_COMMAND_RESET) {
					set_parameter_reset(4 - m_num_of_params ,data);
				} else {
					set_parameter_cursor(2 - m_num_of_params, data);
				}
				m_num_of_params--;
			} else {
				m_status_reg |= I8275_STATUS_IMPROPER_COMMAND;
			}
		} else {
			LOG(("i8275 : ERROR writing parameter\n"));
			m_status_reg |= I8275_STATUS_IMPROPER_COMMAND;
		}
	}
}


void i8275_device::draw_char_line()
{
	int xpos = 0;
	int line = 0;
	UINT8 lc = 0;
	UINT8 vsp = 0;
	UINT8 lten = 0;
	UINT8 fifo_read = 0;

	for(line=0;line<=m_lines_per_row;line++) {
		// If line counter is 1 then select right values
		lc = (m_line_counter_mode==1) ? (line - 1) % m_lines_per_row : line;
		fifo_read = 0;
		for(xpos=0;xpos<=m_chars_per_row;xpos++) {
			UINT8 chr = (m_buffer_used==0) ? m_row_buffer_2[xpos] : m_row_buffer_1[xpos];
			if (m_undeline_line_num & 0x08) {
				vsp = (line==0 || line==m_lines_per_row) ? 1 : 0;
			}

			if ((chr & 0x80)==0x80) {
				if ((chr & 0xc0)==0xc0) {
					// character attribute code
					m_lineattr = 0;
				} else {
					// field attribute code
					m_hlgt = chr & 1;
					m_blink = (chr >> 1) & 1;
					m_gpa = (chr >> 2) & 3;
					m_rvv = (chr >> 4) & 1;
					m_underline = (chr >> 5) & 1;
				}

				if (m_field_attribute_mode==0) {
					chr = (m_buffer_used==0) ? m_fifo_buffer_2[fifo_read] : m_fifo_buffer_1[fifo_read];
					fifo_read = (fifo_read + 1 ) % 16;
				} else {
					vsp = 1;
				}
			}
			if (vsp==0 && m_blink) {
				vsp = (m_char_blink_cnt < 32)  ? 1: 0;
			}
			if ((m_current_row == m_cursor_row) && (xpos ==  m_cursor_col - m_char_delay)) {
				int vis = 1;
				if ((m_cursor_format & 2)==0) {
					vis = (m_cursor_blink_cnt<16) ? 1 : 0;
				}
				if ((m_cursor_format & 1)==1) {
					lten = (line == m_undeline_line_num) ? vis : 0; //underline
				} else {
					lten = vis; // block cursor
				}
			} else {
				lten = 0;
			}

			m_display_pixels_func(this, m_bitmap,
				xpos * m_width, // x position on screen of starting point
				m_ypos, // y position on screen
				lc, // current line of char
				(chr & 0x7f),  // char code to be displayed
				m_lineattr,  // line attribute code
				lten | m_underline,  // light enable signal
				m_rvv,  // reverse video signal
				vsp, // video suppression
				m_gpa,  // general purpose attribute code
				m_hlgt  // highlight
			);
			vsp = 0;
		}
		m_ypos++;
	}
	m_current_row++;
}

WRITE8_MEMBER( i8275_device::dack_w )
{
	if (m_next_in_fifo == 1)
	{
		m_next_in_fifo = 0;

		if(m_buffer_used == 0)
		{
			m_fifo_buffer_1[m_fifo_write] = data & 0x7f;
		}
		else
		{
			m_fifo_buffer_2[m_fifo_write] = data & 0x7f;
		}
		m_fifo_write = (m_fifo_write + 1) % 16;

		if (m_last_data == 0xf1) {
			m_row_pos = m_chars_per_row + 1;
		}
	}
	else
	{
		if(m_buffer_used == 0)
		{
			m_row_buffer_1[m_row_pos++] = data;
		}
		else
		{
			m_row_buffer_2[m_row_pos++] = data;
		}
		if (m_field_attribute_mode==0)
		{
			if ((data & 0x80)==0x80)
			{
				m_last_data = data;
				m_next_in_fifo = 1;
			}
		}
	}

	if ((m_row_pos - 1)==m_chars_per_row )
	{
		m_buffer_used = (m_buffer_used==0) ? 1 : 0;
		m_row_pos = 0;
		m_fifo_write = 0;
		draw_char_line();
	}
	if (m_current_row == (m_rows_per_frame + 1))
	{
		m_ypos = 0;
		m_current_row = 0;

		m_out_drq_func(0);
	}
}

/* Screen Update */
UINT32 i8275_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_ypos = 0;
	m_lineattr = 0;
	m_rvv = 0;
	m_gpa = 0;
	m_hlgt = 0;
	m_underline = 0;
	m_blink = 0;
	m_row_pos = 0;
	m_fifo_write = 0;

	if ((m_status_reg & I8275_STATUS_VIDEO_ENABLE)==0) {
		bitmap.fill(rgb_t(0x00,0x00,0x00), cliprect);
	} else {
		// if value < 16 it is visible otherwise not
		m_cursor_blink_cnt++;
		if(m_cursor_blink_cnt==32) m_cursor_blink_cnt = 0;
		// if value < 32 it is visible otherwise not
		m_char_blink_cnt++;
		if(m_char_blink_cnt==64) m_char_blink_cnt = 0;

		m_out_drq_func(1);
	}
	if (m_status_reg & I8275_STATUS_INTERRUPT_ENABLE) {
		m_status_reg |= I8275_STATUS_INTERRUPT_REQUEST;
	}

	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);

	return 0;
}

/* Device Interface */

const device_type I8275 = &device_creator<i8275_device>;

i8275_device::i8275_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, I8275, "Intel 8275", tag, owner, clock, "i8275", __FILE__),
		device_video_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void i8275_device::device_config_complete()
{
	// inherit a copy of the static data
	const i8275_interface *intf = reinterpret_cast<const i8275_interface *>(static_config());
	if (intf != NULL)
		*static_cast<i8275_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		memset(&m_out_drq_cb, 0, sizeof(m_out_drq_cb));
		memset(&m_out_irq_cb, 0, sizeof(m_out_irq_cb));
		memset(&m_out_hrtc_cb, 0, sizeof(m_out_hrtc_cb));
		memset(&m_out_vrtc_cb, 0, sizeof(m_out_vrtc_cb));
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void i8275_device::device_start()
{
	/* get the screen device */
	m_screen->register_screen_bitmap(m_bitmap);

	/* resolve callbacks */
	m_out_drq_func.resolve(m_out_drq_cb, *this);
	m_out_irq_func.resolve(m_out_irq_cb, *this);
	m_out_hrtc_func.resolve(m_out_hrtc_cb, *this);
	m_out_vrtc_func.resolve(m_out_vrtc_cb, *this);

	/* register for state saving */
	save_item(NAME(m_status_reg));
	save_item(NAME(m_num_of_params));
	save_item(NAME(m_current_command));
	save_item(NAME(m_param_type));

	save_item(NAME(m_cursor_col));
	save_item(NAME(m_cursor_row));

	save_item(NAME(m_light_pen_col));
	save_item(NAME(m_light_pen_row));

	save_item(NAME(m_rows_type));
	save_item(NAME(m_chars_per_row));
	save_item(NAME(m_vert_retrace_rows));
	save_item(NAME(m_rows_per_frame));
	save_item(NAME(m_undeline_line_num));
	save_item(NAME(m_lines_per_row));
	save_item(NAME(m_line_counter_mode));
	save_item(NAME(m_field_attribute_mode));
	save_item(NAME(m_cursor_format));
	save_item(NAME(m_hor_retrace_count));

	save_item(NAME(m_burst_space_code));
	save_item(NAME(m_burst_count_code));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void i8275_device::device_reset()
{
	m_status_reg = 0;
	m_num_of_params = 0;
	m_current_command = 0;
	m_param_type = 0;

	m_cursor_col = 0;
	m_cursor_row = 0;

	m_light_pen_col = 0;
	m_light_pen_row = 0;

	m_rows_type = 0;
	m_chars_per_row = 0;
	m_vert_retrace_rows = 0;
	m_rows_per_frame = 0;
	m_undeline_line_num = 0;
	m_lines_per_row = 0;
	m_line_counter_mode = 0;
	m_field_attribute_mode = 0;
	m_cursor_format = 0;
	m_hor_retrace_count = 0;

	m_burst_space_code = 0;
	m_burst_count_code = 0;

	m_row_pos = 0;
	m_buffer_used = 0;

	m_fifo_write = 0;

	m_ypos = 0;
	m_current_row = 0;

	m_cursor_blink_cnt = 0;
	m_char_blink_cnt = 0;
	m_next_in_fifo = 0;

	m_lineattr = 0;
	m_rvv = 0;
	m_gpa = 0;
	m_hlgt = 0;
	m_underline = 0;
	m_blink = 0;
}
