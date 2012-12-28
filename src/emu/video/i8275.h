/***************************************************************************

    INTEL 8275 Programmable CRT Controller implementation

    25-05-2008 Initial implementation [Miodrag Milanovic]

    Copyright MESS team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#ifndef __I8275_VIDEO__
#define __I8275_VIDEO__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_I8275_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, I8275, 0) \
	MCFG_DEVICE_CONFIG(_intrf)


#define I8275_INTERFACE(name) \
	const i8275_interface (name) =



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class i8275_device;

// ======================> i8275_display_pixels_func

typedef void (*i8275_display_pixels_func)(i8275_device *device, bitmap_rgb32 &bitmap, int x, int y, UINT8 linecount, UINT8 charcode, UINT8 lineattr, UINT8 lten, UINT8 rvv, UINT8 vsp, UINT8 gpa, UINT8 hlgt);
#define I8275_DISPLAY_PIXELS(name)	void name(i8275_device *device, bitmap_rgb32 &bitmap, int x, int y, UINT8 linecount, UINT8 charcode, UINT8 lineattr, UINT8 lten, UINT8 rvv, UINT8 vsp, UINT8 gpa, UINT8 hlgt)


// ======================> i8275_interface

struct i8275_interface
{
	const char *m_screen_tag;		/* screen we are acting on */
	int m_width;					/* char width in pixels */
	int m_char_delay;				/* delay of display char */

	devcb_write_line m_out_drq_cb;
	devcb_write_line m_out_irq_cb;

	devcb_write_line m_out_hrtc_cb;
	devcb_write_line m_out_vrtc_cb;

	i8275_display_pixels_func m_display_pixels_func;
};


class i8275_device : public device_t,
					 public i8275_interface
{
public:
	i8275_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_WRITE8_MEMBER( dack_w );

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

	UINT8 get_parameter_light_pen(offs_t offset);
	void recompute_parameters();
	void set_parameter_reset(offs_t offset, UINT8 data);
	void set_parameter_cursor(offs_t offset, UINT8 data);
	void draw_char_line();

	devcb_resolved_write_line	m_out_drq_func;
	devcb_resolved_write_line	m_out_irq_func;
	devcb_resolved_write_line	m_out_hrtc_func;
	devcb_resolved_write_line	m_out_vrtc_func;

	screen_device *m_screen;
	bitmap_rgb32 m_bitmap;

	UINT8 m_status_reg;				/* value of status reggister */
	UINT8 m_num_of_params;			/* expected number of parameters */
	UINT8 m_current_command;			/* command currently executing */
	UINT8 m_param_type;				/* parameter type */

	UINT8 m_cursor_col;				/* current cursor column */
	UINT8 m_cursor_row;				/* current cursor row */

	UINT8 m_light_pen_col;			/* current light pen column */
	UINT8 m_light_pen_row;			/* current light pen row */

	/* reset command parameter values*/
	/* parameter 0 */
	UINT8 m_rows_type;
	UINT8 m_chars_per_row;
	/* parameter 1 */
	UINT8 m_vert_retrace_rows;
	UINT8 m_rows_per_frame;
	/* parameter 2 */
	UINT8 m_undeline_line_num;
	UINT8 m_lines_per_row;
	/* parameter 3 */
	UINT8 m_line_counter_mode;
	UINT8 m_field_attribute_mode;
	UINT8 m_cursor_format;
	UINT8 m_hor_retrace_count;

	/* values for start display command */
	UINT8 m_burst_space_code;
	UINT8 m_burst_count_code;

	/* buffers */
	UINT8 m_row_buffer_1[80];
	UINT8 m_row_buffer_2[80];
	UINT8 m_row_pos;
	UINT8 m_buffer_used;

	UINT8 m_fifo_buffer_1[16];
	UINT8 m_fifo_buffer_2[16];
	UINT8 m_fifo_write;

	int m_ypos;
	int m_current_row;

	UINT8 m_cursor_blink_cnt;
	UINT8 m_char_blink_cnt;

	UINT8 m_next_in_fifo;

	UINT8 m_lineattr;
	UINT8 m_rvv;
	UINT8 m_gpa;
	UINT8 m_hlgt;
	UINT8 m_underline;
	UINT8 m_blink;

	UINT8 m_last_data;
};

// device type definition
extern const device_type I8275;


#endif
