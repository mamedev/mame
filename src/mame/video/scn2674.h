
#define S674VERBOSE 0
#define LOG2674(x) do { if (S674VERBOSE) logerror x; } while (0)

typedef void (*s2574_interrupt_callback_func)(running_machine &machine);

static const UINT8 vsync_table[4] = {3,1,5,7}; //Video related

class scn2674_device : public device_t
{
public:
	scn2674_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	static void set_irq_update_callback(device_t &device, s2574_interrupt_callback_func callback);


//  int m_gfx_index;

	DECLARE_READ16_MEMBER( mpu4_vid_scn2674_r );
	DECLARE_WRITE16_MEMBER( mpu4_vid_scn2674_w );

	UINT8 get_irq_state( void )
	{
		return m_scn2674_irq_state;
	}



	void scn2574_draw(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, UINT16* vid_mainram);
	void scn2574_draw(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect, UINT16* vid_mainram);
	void init_stuff();
	void scn2674_do_scanline(running_machine &machine, int scanline);


protected:
	virtual void device_start();
	virtual void device_reset();

	s2574_interrupt_callback_func m_interrupt_callback;

	UINT8 m_scn2674_IR_pointer;
	UINT8 m_scn2674_screen1_l;
	UINT8 m_scn2674_screen1_h;
	UINT8 m_scn2674_cursor_l;
	UINT8 m_scn2674_cursor_h;
	UINT8 m_scn2674_screen2_l;
	UINT8 m_scn2674_screen2_h;
	UINT8 m_scn2674_irq_register;
	UINT8 m_scn2674_status_register;
	UINT8 m_scn2674_irq_mask;
	UINT8 m_scn2674_gfx_enabled;
	UINT8 m_scn2674_display_enabled;
	UINT8 m_scn2674_display_enabled_field;
	UINT8 m_scn2674_display_enabled_scanline;
	UINT8 m_scn2674_cursor_enabled;
	UINT8 m_IR0_scn2674_double_ht_wd;
	UINT8 m_IR0_scn2674_scanline_per_char_row;
	UINT8 m_IR0_scn2674_sync_select;
	UINT8 m_IR0_scn2674_buffer_mode_select;
	UINT8 m_IR1_scn2674_interlace_enable;
	UINT8 m_IR1_scn2674_equalizing_constant;
	UINT8 m_IR2_scn2674_row_table;
	UINT8 m_IR2_scn2674_horz_sync_width;
	UINT8 m_IR2_scn2674_horz_back_porch;
	UINT8 m_IR3_scn2674_vert_front_porch;
	UINT8 m_IR3_scn2674_vert_back_porch;
	UINT8 m_IR4_scn2674_rows_per_screen;
	UINT8 m_IR4_scn2674_character_blink_rate_divisor;
	UINT8 m_IR5_scn2674_character_per_row;
	UINT8 m_IR6_scn2674_cursor_first_scanline;
	UINT8 m_IR6_scn2674_cursor_last_scanline;
	UINT8 m_IR7_scn2674_cursor_underline_position;
	UINT8 m_IR7_scn2674_cursor_rate_divisor;
	UINT8 m_IR7_scn2674_cursor_blink;
	UINT8 m_IR7_scn2674_vsync_width;
	UINT8 m_IR8_scn2674_display_buffer_first_address_LSB;
	UINT8 m_IR9_scn2674_display_buffer_first_address_MSB;
	UINT8 m_IR9_scn2674_display_buffer_last_address;
	UINT8 m_IR10_scn2674_display_pointer_address_lower;
	UINT8 m_IR11_scn2674_display_pointer_address_upper;
	UINT8 m_IR11_scn2674_reset_scanline_counter_on_scrollup;
	UINT8 m_IR11_scn2674_reset_scanline_counter_on_scrolldown;
	UINT8 m_IR12_scn2674_scroll_start;
	UINT8 m_IR12_scn2674_split_register_1;
	UINT8 m_IR13_scn2674_scroll_end;
	UINT8 m_IR13_scn2674_split_register_2;
	UINT8 m_IR14_scn2674_scroll_lines;
	UINT8 m_IR14_scn2674_double_1;
	UINT8 m_IR14_scn2674_double_2;
	UINT8 m_scn2674_horz_front_porch;
	UINT8 m_scn2674_spl1;
	UINT8 m_scn2674_spl2;
	UINT8 m_scn2674_dbl1;
	int m_rowcounter;
	int m_linecounter;

	UINT8 m_scn2674_irq_state;

	void scn2674_write_init_regs(UINT8 data);
	void scn2674_write_command(running_machine &machine, UINT8 data);
	void scn2674_line(running_machine &machine);

	template<class _BitmapClass>
	void scn2574_draw_common( running_machine &machine, _BitmapClass &bitmap, const rectangle &cliprect, UINT16* vid_mainram );

private:

};


extern const device_type SCN2674_VIDEO;
