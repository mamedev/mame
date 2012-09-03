/***************************************************************************

    INTEL 8275 Programmable CRT Controller implementation

    25-05-2008 Initial implementation [Miodrag Milanovic]

    Copyright MESS team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "emu.h"
#include "i8275.h"

#define I8275_COMMAND_RESET				0
#define I8275_COMMAND_START_DISPLAY		1
#define I8275_COMMAND_STOP_DISPLAY		2
#define I8275_COMMAND_READ_LIGHT_PEN	3
#define I8275_COMMAND_LOAD_CURSOR		4
#define I8275_COMMAND_ENABLE_INTERRUPT	5
#define I8275_COMMAND_DISABLE_INTERRUPT	6
#define I8275_COMMAND_PRESET_COUNTERS	7

#define I8275_PARAM_RESET				4
#define I8275_PARAM_READ_LIGHT_PEN		2
#define I8275_PARAM_LOAD_CURSOR			2

#define I8275_PARAM_NONE				0
#define I8275_PARAM_READ				1
#define I8275_PARAM_WRITE				2

#define I8275_STATUS_FIFO_OVERRUN		0x01
#define I8275_STATUS_DMA_UNDERRUN		0x02
#define I8275_STATUS_VIDEO_ENABLE		0x04
#define I8275_STATUS_IMPROPER_COMMAND	0x08
#define I8275_STATUS_LIGHT_PEN			0x10
#define I8275_STATUS_INTERRUPT_REQUEST	0x20
#define I8275_STATUS_INTERRUPT_ENABLE	0x40

#define I8275_ROW_TYPE_NORMAL			0
#define I8275_ROW_TYPE_SPACED			1

#define I8275_LINE_COUNTER_MODE_0		0
#define I8275_LINE_COUNTER_MODE_1		1

#define I8275_FIELD_ATTRIBUTE_TRANSPARENT		0
#define I8275_FIELD_ATTRIBUTE_NONTRANSPARENT	1

#define I8275_CURSOR_BLINK_REVERSED		0
#define I8275_CURSOR_BLINK_UNDERLINE	1
#define I8275_CURSOR_NONBLINK_REVERSED	2
#define I8275_CURSOR_NONBLINK_UNDERLINE	3


#define	VERBOSE			1

#define	LOG(x)		do { if (VERBOSE) logerror x; } while (0)


typedef struct _i8275_t i8275_t;

struct _i8275_t
{
	devcb_resolved_write_line	out_drq_func;
	devcb_resolved_write_line	out_irq_func;

	screen_device *screen;

	const i8275_interface *intf;

	UINT8 status_reg;				/* value of status reggister */
	UINT8 num_of_params;			/* expected number of parameters */
	UINT8 current_command;			/* command currently executing */
	UINT8 param_type;				/* parameter type */

	UINT8 cursor_col;				/* current cursor column */
	UINT8 cursor_row;				/* current cursor row */

	UINT8 light_pen_col;			/* current light pen column */
	UINT8 light_pen_row;			/* current light pen row */

	/* reset command parameter values*/
	/* parameter 0 */
	UINT8 rows_type;
	UINT8 chars_per_row;
	/* parameter 1 */
	UINT8 vert_retrace_rows;
	UINT8 rows_per_frame;
	/* parameter 2 */
	UINT8 undeline_line_num;
	UINT8 lines_per_row;
	/* parameter 3 */
	UINT8 line_counter_mode;
	UINT8 field_attribute_mode;
	UINT8 cursor_format;
	UINT8 hor_retrace_count;

	/* values for start display command */
	UINT8 burst_space_code;
	UINT8 burst_count_code;

	/* buffers */
	UINT8 row_buffer_1[80];
	UINT8 row_buffer_2[80];
	UINT8 row_pos;
	UINT8 buffer_used;

	UINT8 fifo_buffer_1[16];
	UINT8 fifo_buffer_2[16];
	UINT8 fifo_write;

	int ypos;
	int current_row;

	UINT8 cursor_blink_cnt;
	UINT8 char_blink_cnt;

	UINT8 next_in_fifo;

	UINT8 lineattr;
	UINT8 rvv;
	UINT8 gpa;
	UINT8 hlgt;
	UINT8 underline;
	UINT8 blink;

	UINT8 last_data;
};

INLINE i8275_t *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == I8275);

	return (i8275_t *)downcast<i8275_device *>(device)->token();
}


/* Register Access */
static UINT8 i8275_get_parameter_light_pen(device_t *device, offs_t offset)
{
	i8275_t *i8275 = get_safe_token(device);
	UINT8 val = 0;

	switch(offset) {
		case 0 :
				val = i8275->light_pen_col;
				break;
		case 1 :
				val = i8275->light_pen_row;
				break;
	}
	return val;
}

READ8_DEVICE_HANDLER( i8275_r )
{
	UINT8 val;
	i8275_t *i8275 = get_safe_token(device);

	if (offset & 0x01)
	{
		/* Status register */
		val = i8275->status_reg;
		/* status reset after read */
		i8275->status_reg &= ~I8275_STATUS_FIFO_OVERRUN;
		i8275->status_reg &= ~I8275_STATUS_DMA_UNDERRUN;
		i8275->status_reg &= ~I8275_STATUS_IMPROPER_COMMAND;
		i8275->status_reg &= ~I8275_STATUS_LIGHT_PEN;
		i8275->status_reg &= ~I8275_STATUS_INTERRUPT_REQUEST;
	}
	else
	{
		/* Parameter register */
		val = 0x00;
		if (i8275->param_type==I8275_PARAM_READ) {
			if (i8275->num_of_params > 0) {
				val = i8275_get_parameter_light_pen(device, 2 - i8275->num_of_params);
				i8275->num_of_params--;
			} else {
				i8275->status_reg |= I8275_STATUS_IMPROPER_COMMAND;
			}
		} else {
			LOG(("i8275 : ERROR reading parameter\n"));
			i8275->status_reg |= I8275_STATUS_IMPROPER_COMMAND;
		}
	}
	return val;
}

static void i8275_recompute_parameters(device_t *device)
{
	i8275_t *i8275 = get_safe_token(device);
	int horiz_pix_total = 0;
	int vert_pix_total = 0;
	rectangle visarea;

	horiz_pix_total = (i8275->chars_per_row + 1) * i8275->intf->width;
	vert_pix_total  = (i8275->lines_per_row + 1) * (i8275->rows_per_frame + 1);
	if (i8275->rows_type==1) {
		vert_pix_total *= 2; // Use of spaced rows
	}

	visarea.set(0, horiz_pix_total - 1, 0, vert_pix_total - 1);

	i8275->screen->configure(horiz_pix_total, vert_pix_total, visarea,
				i8275->screen->frame_period().attoseconds);
}

static void i8275_set_parameter_reset(device_t *device, offs_t offset, UINT8 data)
{
	i8275_t *i8275 = get_safe_token(device);
	switch(offset) {
		case 0 :
				i8275->rows_type = (data >> 7) & 1;
				i8275->chars_per_row = data & 0x7f;
				break;
		case 1 :
				i8275->vert_retrace_rows = (data >> 6) & 3;
				i8275->rows_per_frame = data & 0x3f;
				break;
		case 2 :
				i8275->undeline_line_num = (data >> 4) & 0x0f;
				i8275->lines_per_row = data & 0x0f;
				break;
		case 3 :
				i8275->line_counter_mode = (data >> 7) & 1;
				i8275->field_attribute_mode = (data >> 6) & 1;
				i8275->cursor_format  = (data >> 4) & 3;
				i8275->hor_retrace_count = data & 0x0f;
				break;
	}
}

static void i8275_set_parameter_cursor(device_t *device, offs_t offset, UINT8 data)
{
	i8275_t *i8275 = get_safe_token(device);
	switch(offset) {
		case 0 :
				i8275->cursor_col = data;
				break;
		case 1 :
				i8275->cursor_row = data;
				break;
	}
}


WRITE8_DEVICE_HANDLER( i8275_w )
{
	i8275_t *i8275 = get_safe_token(device);

	if (offset & 0x01)
	{
		/* Command register */
		if (i8275->num_of_params != 0) {
			i8275->status_reg |= I8275_STATUS_IMPROPER_COMMAND;
			return;
		}
		i8275->current_command = (data >> 5) & 7;
		i8275->num_of_params = I8275_PARAM_NONE;
		i8275->param_type = I8275_PARAM_NONE;
		switch(i8275->current_command) {
			case I8275_COMMAND_RESET		:
											i8275->num_of_params = I8275_PARAM_RESET;
											i8275->param_type = I8275_PARAM_WRITE;
											/* set status register */
											i8275->status_reg &= ~I8275_STATUS_INTERRUPT_ENABLE;
											i8275->status_reg &= ~I8275_STATUS_VIDEO_ENABLE;
											break;
			case I8275_COMMAND_START_DISPLAY:
											i8275->burst_space_code = (data >> 2) & 7;
											i8275->burst_count_code = data & 3;
											/* set status register */
											i8275->status_reg |= I8275_STATUS_VIDEO_ENABLE;
											i8275->status_reg |= I8275_STATUS_INTERRUPT_ENABLE;
											i8275_recompute_parameters(device);
											break;
			case I8275_COMMAND_STOP_DISPLAY :
											/* set status register */
											i8275->status_reg &= ~I8275_STATUS_VIDEO_ENABLE;
											break;
			case I8275_COMMAND_READ_LIGHT_PEN:
											i8275->num_of_params = I8275_PARAM_READ_LIGHT_PEN;
											i8275->param_type = I8275_PARAM_READ;
											break;
			case I8275_COMMAND_LOAD_CURSOR	:
											i8275->num_of_params = I8275_PARAM_LOAD_CURSOR;
											i8275->param_type = I8275_PARAM_WRITE;
											break;
			case I8275_COMMAND_ENABLE_INTERRUPT	:
											/* set status register */
											i8275->status_reg |= I8275_STATUS_INTERRUPT_ENABLE;
											break;
			case I8275_COMMAND_DISABLE_INTERRUPT:
											/* set status register */
											i8275->status_reg &= ~I8275_STATUS_INTERRUPT_ENABLE;
											break;
			case I8275_COMMAND_PRESET_COUNTERS	:
											break;
		}
	}
	else
	{
		/* Parameter register */
		if (i8275->param_type==I8275_PARAM_WRITE) {
			if (i8275->num_of_params > 0) {
				if (i8275->current_command == I8275_COMMAND_RESET) {
					i8275_set_parameter_reset(device, 4 - i8275->num_of_params ,data);
				} else {
					i8275_set_parameter_cursor(device, 2 - i8275->num_of_params, data);
				}
				i8275->num_of_params--;
			} else {
				i8275->status_reg |= I8275_STATUS_IMPROPER_COMMAND;
			}
		} else {
			LOG(("i8275 : ERROR writing parameter\n"));
			i8275->status_reg |= I8275_STATUS_IMPROPER_COMMAND;
		}
	}
}


static void i8275_draw_char_line(device_t *device)
{
	i8275_t *i8275 = get_safe_token(device);
	int xpos = 0;
	int line = 0;
	UINT8 lc = 0;
	UINT8 vsp = 0;
	UINT8 lten = 0;
	UINT8 fifo_read = 0;

	for(line=0;line<=i8275->lines_per_row;line++) {
		// If line counter is 1 then select right values
		lc = (i8275->line_counter_mode==1) ? (line - 1) % i8275->lines_per_row : line;
		fifo_read = 0;
		for(xpos=0;xpos<=i8275->chars_per_row;xpos++) {
			UINT8 chr =	(i8275->buffer_used==0) ? i8275->row_buffer_2[xpos] : i8275->row_buffer_1[xpos];
			if (i8275->undeline_line_num & 0x08) {
				vsp = (line==0 || line==i8275->lines_per_row) ? 1 : 0;
			}

			if ((chr & 0x80)==0x80) {
				if ((chr & 0xc0)==0xc0) {
					// character attribute code
					i8275->lineattr = 0;
				} else {
					// field attribute code
					i8275->hlgt = chr & 1;
					i8275->blink = (chr >> 1) & 1;
					i8275->gpa = (chr >> 2) & 3;
					i8275->rvv = (chr >> 4) & 1;
					i8275->underline = (chr >> 5) & 1;
				}

				if (i8275->field_attribute_mode==0) {
					chr = (i8275->buffer_used==0) ? i8275->fifo_buffer_2[fifo_read] : i8275->fifo_buffer_1[fifo_read];
					fifo_read = (fifo_read + 1 ) % 16;
				} else {
					vsp = 1;
				}
			}
			if (vsp==0 && i8275->blink) {
				vsp = (i8275->char_blink_cnt < 32)  ? 1: 0;
			}
			if ((i8275->current_row == i8275->cursor_row) && (xpos ==  i8275->cursor_col - i8275->intf->char_delay)) {
				int vis = 1;
				if ((i8275->cursor_format & 2)==0) {
					vis = (i8275->cursor_blink_cnt<16) ? 1 : 0;
				}
				if ((i8275->cursor_format & 1)==1) {
					lten = (line == i8275->undeline_line_num) ? vis : 0; //underline
				} else {
					lten = vis; // block cursor
				}
			} else {
				lten = 0;
			}

			i8275->intf->display_pixels(device,
				xpos * i8275->intf->width, // x position on screen of starting point
				i8275->ypos, // y position on screen
				lc, // current line of char
				(chr & 0x7f),  // char code to be displayed
				i8275->lineattr,  // line attribute code
				lten | i8275->underline,  // light enable signal
				i8275->rvv,  // reverse video signal
				vsp, // video suppression
				i8275->gpa,  // general purpose attribute code
				i8275->hlgt  // highlight
			);
			vsp = 0;
		}
		i8275->ypos++;
	}
	i8275->current_row++;
}

WRITE8_DEVICE_HANDLER( i8275_dack_w )
{
	i8275_t *i8275 = get_safe_token(device);

	if (i8275->next_in_fifo == 1)
	{
		i8275->next_in_fifo = 0;

		if(i8275->buffer_used == 0)
		{
			i8275->fifo_buffer_1[i8275->fifo_write] = data & 0x7f;
		}
		else
		{
			i8275->fifo_buffer_2[i8275->fifo_write] = data & 0x7f;
		}
		i8275->fifo_write = (i8275->fifo_write + 1) % 16;

		if (i8275->last_data == 0xf1) {
			i8275->row_pos = i8275->chars_per_row + 1;
		}
	}
	else
	{
		if(i8275->buffer_used == 0)
		{
			i8275->row_buffer_1[i8275->row_pos++] = data;
		}
		else
		{
			i8275->row_buffer_2[i8275->row_pos++] = data;
		}
		if (i8275->field_attribute_mode==0)
		{
			if ((data & 0x80)==0x80)
			{
				i8275->last_data = data;
				i8275->next_in_fifo = 1;
			}
		}
	}

	if ((i8275->row_pos - 1)==i8275->chars_per_row )
	{
		i8275->buffer_used = (i8275->buffer_used==0) ? 1 : 0;
		i8275->row_pos = 0;
		i8275->fifo_write = 0;
		i8275_draw_char_line(device);
	}
	if (i8275->current_row == (i8275->rows_per_frame + 1))
	{
		i8275->ypos = 0;
		i8275->current_row = 0;

		i8275->out_drq_func(0);
	}
}

/* Screen Update */
void i8275_update(device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	i8275_t *i8275 = get_safe_token(device);
	i8275->ypos = 0;
	i8275->lineattr = 0;
	i8275->rvv = 0;
	i8275->gpa = 0;
	i8275->hlgt = 0;
	i8275->underline = 0;
	i8275->blink = 0;
	i8275->row_pos = 0;
	i8275->fifo_write = 0;

	if ((i8275->status_reg & I8275_STATUS_VIDEO_ENABLE)==0) {
		bitmap.fill(get_black_pen(device->machine()), cliprect);
	} else {
		// if value < 16 it is visible otherwise not
		i8275->cursor_blink_cnt++;
		if(i8275->cursor_blink_cnt==32) i8275->cursor_blink_cnt = 0;
		// if value < 32 it is visible otherwise not
		i8275->char_blink_cnt++;
		if(i8275->char_blink_cnt==64) i8275->char_blink_cnt = 0;

		i8275->out_drq_func(1);
	}
	if (i8275->status_reg & I8275_STATUS_INTERRUPT_ENABLE) {
		i8275->status_reg |= I8275_STATUS_INTERRUPT_REQUEST;
	}
}

/* Device Interface */

static DEVICE_START( i8275 )
{
	i8275_t *i8275 = get_safe_token(device);

	/* validate arguments */
	assert(device != NULL);
	assert(device->tag() != NULL);
	assert(device->static_config() != NULL);

	i8275->intf = (const i8275_interface*)device->static_config();

	assert(i8275->intf->display_pixels != NULL);

	/* get the screen device */
	i8275->screen = device->machine().device<screen_device>(i8275->intf->screen_tag);


	assert(i8275->screen != NULL);

	/* resolve callbacks */
	i8275->out_drq_func.resolve(i8275->intf->out_drq_func, *device);
	i8275->out_irq_func.resolve(i8275->intf->out_irq_func, *device);

	/* register for state saving */
	device->save_item(NAME(i8275->status_reg));
	device->save_item(NAME(i8275->num_of_params));
	device->save_item(NAME(i8275->current_command));
	device->save_item(NAME(i8275->param_type));

	device->save_item(NAME(i8275->cursor_col));
	device->save_item(NAME(i8275->cursor_row));

	device->save_item(NAME(i8275->light_pen_col));
	device->save_item(NAME(i8275->light_pen_row));

	device->save_item(NAME(i8275->rows_type));
	device->save_item(NAME(i8275->chars_per_row));
	device->save_item(NAME(i8275->vert_retrace_rows));
	device->save_item(NAME(i8275->rows_per_frame));
	device->save_item(NAME(i8275->undeline_line_num));
	device->save_item(NAME(i8275->lines_per_row));
	device->save_item(NAME(i8275->line_counter_mode));
	device->save_item(NAME(i8275->field_attribute_mode));
	device->save_item(NAME(i8275->cursor_format));
	device->save_item(NAME(i8275->hor_retrace_count));

	device->save_item(NAME(i8275->burst_space_code));
	device->save_item(NAME(i8275->burst_count_code));
}

static DEVICE_RESET( i8275 )
{
	i8275_t *i8275 = get_safe_token(device);

	i8275->status_reg = 0;
	i8275->num_of_params = 0;
	i8275->current_command = 0;
	i8275->param_type = 0;

	i8275->cursor_col = 0;
	i8275->cursor_row = 0;

	i8275->light_pen_col = 0;
	i8275->light_pen_row = 0;

	i8275->rows_type = 0;
	i8275->chars_per_row = 0;
	i8275->vert_retrace_rows = 0;
	i8275->rows_per_frame = 0;
	i8275->undeline_line_num = 0;
	i8275->lines_per_row = 0;
	i8275->line_counter_mode = 0;
	i8275->field_attribute_mode = 0;
	i8275->cursor_format = 0;
	i8275->hor_retrace_count = 0;

	i8275->burst_space_code = 0;
	i8275->burst_count_code = 0;

	i8275->row_pos = 0;
	i8275->buffer_used = 0;

	i8275->fifo_write = 0;

	i8275->ypos = 0;
	i8275->current_row = 0;

	i8275->cursor_blink_cnt = 0;
	i8275->char_blink_cnt = 0;
	i8275->next_in_fifo = 0;

	i8275->lineattr = 0;
	i8275->rvv = 0;
	i8275->gpa = 0;
	i8275->hlgt = 0;
	i8275->underline = 0;
	i8275->blink = 0;
}

DEVICE_GET_INFO( i8275 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(i8275_t);					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(i8275);		break;
		case DEVINFO_FCT_STOP:							/* Nothing */								break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME(i8275);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Intel 8275");				break;
		case DEVINFO_STR_FAMILY:						strcpy(info->s, "Intel 8275");				break;
		case DEVINFO_STR_VERSION:						strcpy(info->s, "1.0");						break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);					break;
		case DEVINFO_STR_CREDITS:						strcpy(info->s, "Copyright MESS Team");		break;
	}
}

const device_type I8275 = &device_creator<i8275_device>;

i8275_device::i8275_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, I8275, "Intel 8275", tag, owner, clock)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(i8275_t));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void i8275_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void i8275_device::device_start()
{
	DEVICE_START_NAME( i8275 )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void i8275_device::device_reset()
{
	DEVICE_RESET_NAME( i8275 )(this);
}


