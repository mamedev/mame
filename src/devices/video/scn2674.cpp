// license:BSD-3-Clause
// copyright-holders:Carl
/*
    SCN2674 - Advanced Video Display Controller (AVDC)  (Video Chip)
*/

#include "scn2674.h"

#define S674VERBOSE 0
#define LOG2674(x) do { if (S674VERBOSE) logerror x; } while (0)

const device_type SCN2674_VIDEO = &device_creator<scn2674_device>;


// default address map
static ADDRESS_MAP_START( scn2674_vram, AS_0, 8, scn2674_device )
	AM_RANGE(0x0000, 0xffff) AM_NOP
ADDRESS_MAP_END

scn2674_device::scn2674_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SCN2674_VIDEO, "Signetics SCN2674 AVDC", tag, owner, clock, "scn2674_device", __FILE__),
		device_video_interface(mconfig, *this),
		device_memory_interface(mconfig, *this),
		m_irq_cb(*this), m_IR_pointer(0), m_screen1_l(0), m_screen1_h(0), m_cursor_l(0), m_cursor_h(0), m_screen2_l(0), m_screen2_h(0), m_irq_register(0), m_status_register(0), m_irq_mask(0), 
	m_gfx_enabled(0), m_display_enabled(0), m_display_enabled_field(0), m_display_enabled_scanline(0), m_cursor_enabled(0), m_hpixels_per_column(0), m_text_hpixels_per_column(0), 
	m_gfx_hpixels_per_column(0), m_IR0_double_ht_wd(0), m_IR0_scanline_per_char_row(0), m_IR0_sync_select(0), m_IR0_buffer_mode_select(0), m_IR1_interlace_enable(0), m_IR1_equalizing_constant(0), 
	m_IR2_row_table(0), m_IR2_horz_sync_width(0), m_IR2_horz_back_porch(0), m_IR3_vert_front_porch(0), m_IR3_vert_back_porch(0), m_IR4_rows_per_screen(0), m_IR4_character_blink_rate_divisor(0), 
	m_IR5_character_per_row(0), m_IR6_cursor_first_scanline(0), m_IR6_cursor_last_scanline(0), m_IR7_cursor_underline_position(0), m_IR7_cursor_rate_divisor(0), m_IR7_cursor_blink(0), 
	m_IR7_vsync_width(0), m_IR8_display_buffer_first_address_LSB(0), m_IR9_display_buffer_first_address_MSB(0), m_IR9_display_buffer_last_address(0), m_IR10_display_pointer_address_lower(0), 
	m_IR11_display_pointer_address_upper(0), m_IR11_reset_scanline_counter_on_scrollup(0), m_IR11_reset_scanline_counter_on_scrolldown(0), m_IR12_scroll_start(0), m_IR12_split_register_1(0), 
	m_IR13_scroll_end(0), m_IR13_split_register_2(0), m_IR14_scroll_lines(0), m_IR14_double_1(0), m_IR14_double_2(0), m_spl1(0), m_spl2(0), m_dbl1(0), m_buffer(0), m_linecounter(0), m_address(0), 
	m_start1change(0), m_irq_state(0), m_scanline_timer(nullptr),
		m_space_config("videoram", ENDIANNESS_LITTLE, 8, 16, 0, NULL, *ADDRESS_MAP_NAME(scn2674_vram))
{
}

void scn2674_device::device_start()
{
	// resolve callbacks
	m_display_cb.bind_relative_to(*owner());
	m_irq_cb.resolve_safe();
	m_scanline_timer = timer_alloc(TIMER_SCANLINE);
	m_screen->register_screen_bitmap(m_bitmap);

	save_item(NAME(m_address));
	save_item(NAME(m_linecounter));
	save_item(NAME(m_screen2_l));
	save_item(NAME(m_screen2_h));
	save_item(NAME(m_cursor_l));
	save_item(NAME(m_cursor_h));
}

void scn2674_device::device_reset()
{
	m_screen1_l = 0;
	m_screen1_h = 0;
	m_cursor_l = 0;
	m_cursor_h = 0;
	m_screen2_l = 0;
	m_screen2_h = 0;
	m_irq_register = 0;
	m_status_register = 0;
	m_irq_mask = 0;
	m_gfx_enabled = 0;
	m_display_enabled = 0;
	m_display_enabled_field = 0;
	m_display_enabled_scanline = 0;
	m_cursor_enabled = 0;
	m_IR0_double_ht_wd = 0;
	m_IR0_scanline_per_char_row = 1;
	m_IR0_sync_select = 0;
	m_IR0_buffer_mode_select = 0;
	m_IR1_interlace_enable = 0;
	m_IR1_equalizing_constant = 0;
	m_IR2_row_table = 0;
	m_IR2_horz_sync_width = 0;
	m_IR2_horz_back_porch = 0;
	m_IR3_vert_front_porch = 0;
	m_IR3_vert_back_porch = 0;
	m_IR4_rows_per_screen = 0;
	m_IR4_character_blink_rate_divisor = 0;
	m_IR5_character_per_row = 0;
	m_IR6_cursor_first_scanline = 0;
	m_IR6_cursor_last_scanline = 0;
	m_IR7_cursor_underline_position = 0;
	m_IR7_cursor_rate_divisor = 0;
	m_IR7_cursor_blink = 0;
	m_IR7_vsync_width = 0;
	m_IR8_display_buffer_first_address_LSB = 0;
	m_IR9_display_buffer_first_address_MSB = 0;
	m_IR9_display_buffer_last_address = 0;
	m_IR10_display_pointer_address_lower = 0;
	m_IR11_display_pointer_address_upper = 0;
	m_IR11_reset_scanline_counter_on_scrollup = 0;
	m_IR11_reset_scanline_counter_on_scrolldown = 0;
	m_IR12_scroll_start = 0;
	m_IR12_split_register_1 = 0;
	m_IR13_scroll_end = 0;
	m_IR13_split_register_2 = 0;
	m_IR14_scroll_lines = 0;
	m_IR14_double_1 = 0;
	m_IR14_double_2 = 0;
	m_spl1 = 0;
	m_spl2 = 0;
	m_dbl1 = 0;
	m_buffer = 0;
	m_linecounter = 0;
	m_irq_state = 0;
	m_IR_pointer = 0;
	m_address = 0;
	m_start1change = 0;
	m_hpixels_per_column = m_text_hpixels_per_column;
}

// 15 Initialization Registers (8-bit each)
void scn2674_device::write_init_regs(UINT8 data)
{
	LOG2674(("scn2674_write_init_regs %02x %02x\n",m_IR_pointer,data));

	switch ( m_IR_pointer) /* display some debug info, set mame specific variables */
	{
		case 0:
			m_IR0_double_ht_wd = (data & 0x80)>>7;
			m_IR0_scanline_per_char_row = ((data & 0x78)>>3) + 1;
			m_IR0_sync_select = (data&0x04)>>2;
			m_IR0_buffer_mode_select = (data&0x03);

			LOG2674(("IR0 - Double Ht Wd %02x\n",m_IR0_double_ht_wd));//affects IR14 as well
			LOG2674(("IR0 - Scanlines per Character Row %02x\n",m_IR0_scanline_per_char_row));//value+1 = scanlines
			LOG2674(("IR0 - Sync Select %02x\n",m_IR0_sync_select));//1 = csync
			LOG2674(("IR0 - Buffer Mode Select %02x\n",m_IR0_buffer_mode_select)); //0 independent 1 transparent 2 shared 3 row
			break;

		case 1:
			m_IR1_interlace_enable = (data&0x80)>>7;
			m_IR1_equalizing_constant = (data&0x7f)+1;

			LOG2674(("IR1 - Interlace Enable %02x\n",m_IR1_interlace_enable));
			LOG2674(("IR1 - Equalizing Constant %02i CCLKs\n",m_IR1_equalizing_constant));
			break;

		case 2:
			m_IR2_row_table = (data&0x80)>>7;
			m_IR2_horz_sync_width = (((data&0x78)>>3)*2) + 2;
			m_IR2_horz_back_porch = ((data&0x07)*4) - 1;

			LOG2674(("IR2 - Row Table %02x\n",m_IR2_row_table));
			LOG2674(("IR2 - Horizontal Sync Width %02i CCLKs\n",m_IR2_horz_sync_width));
			LOG2674(("IR2 - Horizontal Back Porch %02i CCLKs\n",m_IR2_horz_back_porch));
			break;

		case 3:
			m_IR3_vert_front_porch =  (((data&0xe0)>>5) * 4)+4 ;
			m_IR3_vert_back_porch = ((data&0x1f) * 2) + 4;

			LOG2674(("IR3 - Vertical Front Porch %02i Lines\n",m_IR3_vert_front_porch));
			LOG2674(("IR3 - Vertical Back Porch %02i Lines\n",m_IR3_vert_back_porch));
			break;

		case 4:
			m_IR4_rows_per_screen = (data&0x7f) + 1;
			m_IR4_character_blink_rate_divisor = ((data & 0x80)>>7 ? 128:64);

			LOG2674(("IR4 - Rows Per Screen %02i\n",m_IR4_rows_per_screen));
			LOG2674(("IR4 - Character Blink Rate = 1/%02i\n",m_IR4_character_blink_rate_divisor));
			break;

		case 5:
			/* IR5 - Active Characters Per Row
			 cccc cccc
			 c = Characters Per Row */
			m_IR5_character_per_row = data + 1;
			LOG2674(("IR5 - Active Characters Per Row %02i\n",m_IR5_character_per_row));
			break;

		case 6:
			m_IR6_cursor_last_scanline = (data & 0x0f);
			m_IR6_cursor_first_scanline = (data & 0xf0)>>4;
			LOG2674(("IR6 - First Line of Cursor %02x\n",m_IR6_cursor_first_scanline));
			LOG2674(("IR6 - Last Line of Cursor %02x\n",m_IR6_cursor_last_scanline));
			break;

		case 7:
		{
			const UINT8 vsync_table[4] = {3,1,5,7};
			m_IR7_cursor_underline_position = (data & 0x0f);
			m_IR7_cursor_rate_divisor = ((data & 0x10)>>4 ? 64:32);
			m_IR7_cursor_blink = (data & 0x20)>>5;

			m_IR7_vsync_width = vsync_table[(data & 0xC0)>>6];

			LOG2674(("IR7 - Underline Position %02x\n",m_IR7_cursor_underline_position));
			LOG2674(("IR7 - Cursor rate 1/%02i\n",m_IR7_cursor_rate_divisor));
			LOG2674(("IR7 - Cursor blink %02x\n",m_IR7_cursor_blink));
			LOG2674(("IR7 - Vsync Width  %02i Lines\n",m_IR7_vsync_width));
			break;
		}

		case 8:
			m_IR8_display_buffer_first_address_LSB = data;
			LOG2674(("IR8 - Display Buffer First Address LSB %02x\n",m_IR8_display_buffer_first_address_LSB));
			break;

		case 9:
			m_IR9_display_buffer_first_address_MSB = data & 0x0f;
			m_IR9_display_buffer_last_address = (data & 0xf0)>>4;
			LOG2674(("IR9 - Display Buffer First Address MSB %02x\n",m_IR9_display_buffer_first_address_MSB));
			LOG2674(("IR9 - Display Buffer Last Address %02x\n",m_IR9_display_buffer_last_address));
			break;

		case 10:
			m_IR10_display_pointer_address_lower = data;
			LOG2674(("IR10 - Display Pointer Address Lower %02x\n",m_IR10_display_pointer_address_lower));
			break;

		case 11:
			m_IR11_display_pointer_address_upper= data&0x3f;
			m_IR11_reset_scanline_counter_on_scrollup= (data&0x40 >> 6);
			m_IR11_reset_scanline_counter_on_scrolldown= (data&0x80 >> 7);

			LOG2674(("IR11 - Display Pointer Address Lower %02x\n",m_IR11_display_pointer_address_upper));
			LOG2674(("IR11 - Reset Scanline Counter on Scroll Up %02x\n",m_IR11_reset_scanline_counter_on_scrollup));
			LOG2674(("IR11 - Reset Scanline Counter on Scroll Down %02x\n",m_IR11_reset_scanline_counter_on_scrolldown));
			break;

		case 12:
			m_IR12_scroll_start = (data & 0x80)>>7;
			m_IR12_split_register_1 = (data & 0x7f);
			LOG2674(("IR12 - Scroll Start %02x\n",m_IR12_scroll_start));
			LOG2674(("IR12 - Split Register 1 %02x\n",m_IR12_split_register_1));
			break;

		case 13:
			m_IR13_scroll_end = (data & 0x80)>>7;
			m_IR13_split_register_2 = (data & 0x7f);
			LOG2674(("IR13 - Scroll End %02x\n",m_IR13_scroll_end));
			LOG2674(("IR13 - Split Register 2 %02x\n",m_IR13_split_register_2));
			break;

		case 14:
			m_IR14_scroll_lines = (data & 0x0f);
			if (!m_IR0_double_ht_wd)
			{
				m_IR14_double_2 = (data & 0x30)>>4;
				LOG2674(("IR14 - Double 2 %02x\n",m_IR14_double_2));
			}
			//0 normal, 1, double width, 2, double width and double tops 3, double width and double bottoms
			//1 affects SSR1, 2 affects SSR2
			//If Double Height enabled in IR0, Screen start 1 upper (bits 7 and 6)replace Double 1, and Double 2 is unused
			m_IR14_double_1 = (data & 0xc0)>>6;
			LOG2674(("IR14 - Double 1 %02x\n",m_IR14_double_1));

			LOG2674(("IR14 - Scroll Lines %02i\n",m_IR14_scroll_lines));
			break;

		case 15: /* not valid! */
			break;

	}
	recompute_parameters();

	m_IR_pointer++;
	if (m_IR_pointer>14)m_IR_pointer=14;
}

void scn2674_device::write_command(UINT8 data)
{
	UINT8 operand;
	int i;


	if (data==0x00)
	{
		/* master reset, configures registers */
		LOG2674(("master reset\n"));
		m_IR_pointer=0;
		m_irq_register = 0x00;
		m_status_register = 0x20;//RDFLG activated
		m_linecounter =0;
		m_irq_mask = 0x00;
		m_gfx_enabled = 0;
		m_display_enabled = 0;
		m_cursor_enabled = 0;
		m_IR2_row_table = 0;
	}

	if ((data&0xf0)==0x10)
	{
		/* set IR pointer */
		operand = data & 0x0f;
		LOG2674(("set IR pointer %02x\n",operand));

		m_IR_pointer=operand;

	}

	/* ANY COMBINATION OF THESE ARE POSSIBLE */

	if ((data&0xe3)==0x22)
	{
		/* Disable GFX */
		LOG2674(("disable GFX %02x\n",data));
		m_gfx_enabled = 0;
		recompute_parameters();
	}

	if ((data&0xe3)==0x23)
	{
		/* Enable GFX */
		LOG2674(("enable GFX %02x\n",data));
		m_gfx_enabled = 1;
		recompute_parameters();
	}

	if ((data&0xe9)==0x28)
	{
		/* Display off */
		operand = data & 0x04;

		m_display_enabled = 0;

		if (operand)
			LOG2674(("display OFF - float DADD bus %02x\n",data));
		else
			LOG2674(("display OFF - no float DADD bus %02x\n",data));
	}

	if ((data&0xe9)==0x29)
	{
		/* Display on */
		operand = data & 0x04;

		if (operand)
		{
			m_display_enabled_field = 1;
			LOG2674(("display ON - next field %02x\n",data));
		}
		else
		{
			m_display_enabled_scanline = 1;
			LOG2674(("display ON - next scanline %02x\n",data));
		}
		recompute_parameters(); // start the scanline timer
	}

	if ((data&0xf1)==0x30)
	{
		/* Cursor Off */
		LOG2674(("cursor off %02x\n",data));
		m_cursor_enabled = 0;
	}

	if ((data&0xf1)==0x31)
	{
		/* Cursor On */
		LOG2674(("cursor on %02x\n",data));
		m_cursor_enabled = 1;
	}

	/* END */

	if ((data&0xe0)==0x40)
	{
		/* Reset Interrupt / Status bit */
		operand = data & 0x1f;
		LOG2674(("reset interrupt / status bit %02x\n",operand));

		m_irq_register &= ~(data & 0x1f);
		m_status_register &= ~(data & 0x1f);

		LOG2674(("IRQ Status after reset\n"));
		LOG2674(("Split 2   IRQ: %d Active\n",(m_irq_register>>0)&1));
		LOG2674(("Ready     IRQ: %d Active\n",(m_irq_register>>1)&1));
		LOG2674(("Split 1   IRQ: %d Active\n",(m_irq_register>>2)&1));
		LOG2674(("Line Zero IRQ: %d Active\n",(m_irq_register>>3)&1));
		LOG2674(("V-Blank   IRQ: %d Active\n",(m_irq_register>>4)&1));

		m_irq_state = 0;

		for (i = 0; i < 5; i++)
		{
			if ((m_irq_register>>i&1)&(m_irq_mask>>i&1))
			{
				m_irq_state = 1;
			}
		}
		m_irq_cb(m_irq_register ? 1 : 0);

	}
	if ((data&0xe0)==0x80)
	{
		/* Disable Interrupt mask*/
		operand = data & 0x1f;
		m_irq_mask &= ~(operand);
		LOG2674(("IRQ Mask after disable %x\n",operand));
		LOG2674(("Split 2   IRQ: %d Unmasked\n",(m_irq_mask>>0)&1));
		LOG2674(("Ready     IRQ: %d Unmasked\n",(m_irq_mask>>1)&1));
		LOG2674(("Split 1   IRQ: %d Unmasked\n",(m_irq_mask>>2)&1));
		LOG2674(("Line Zero IRQ: %d Unmasked\n",(m_irq_mask>>3)&1));
		LOG2674(("V-Blank   IRQ: %d Unmasked\n",(m_irq_mask>>4)&1));

	}

	if ((data&0xe0)==0x60)
	{
		/* Enable Interrupt mask*/
		operand = data & 0x1f;
		m_irq_mask |= (data & 0x1f);

		LOG2674(("IRQ Mask after enable %x\n",operand));
		LOG2674(("Split 2   IRQ: %d Unmasked\n",(m_irq_mask>>0)&1));
		LOG2674(("Ready     IRQ: %d Unmasked\n",(m_irq_mask>>1)&1));
		LOG2674(("Split 1   IRQ: %d Unmasked\n",(m_irq_mask>>2)&1));
		LOG2674(("Line Zero IRQ: %d Unmasked\n",(m_irq_mask>>3)&1));
		LOG2674(("V-Blank   IRQ: %d Unmasked\n",(m_irq_mask>>4)&1));

	}

	/* Delayed Commands */
	/* These set 0x20 in status register when done */
	// These use the pointer address according to the datasheet but the pcx expects the screen start 2 address instead
	switch(data)
	{
		case 0xa4:
			/* read at pointer address */
			m_buffer = space().read_byte(m_screen2_l | (m_screen2_h << 8));
			LOG2674(("DELAYED read at pointer address %02x\n",data));
			break;

		case 0xa2:
			/* write at pointer address */
			space().write_byte(m_screen2_l | (m_screen2_h << 8), m_buffer);
			LOG2674(("DELAYED write at pointer address %02x\n",data));
			break;

		case 0xa9:
			/* increment cursor address */
			if(!(++m_cursor_l))
				m_cursor_h++;
			LOG2674(("DELAYED increase cursor address %02x\n",data));
			break;

		case 0xac:
			/* read at cursor address */
			m_buffer = space().read_byte(m_cursor_l | (m_cursor_h << 8));
			LOG2674(("DELAYED read at cursor address %02x\n",data));
			break;

		case 0xaa:
			/* write at cursor address */
			space().write_byte(m_cursor_l | (m_cursor_h << 8), m_buffer);
			LOG2674(("DELAYED write at cursor address %02x\n",data));
			break;

		case 0xad:
			/* read at cursor address + increment */
			m_buffer = space().read_byte(m_cursor_l | (m_cursor_h << 8));
			if(!(++m_cursor_l))
				m_cursor_h++;
			LOG2674(("DELAYED read at cursor address+increment %02x\n",data));
			break;

		case 0xab:
			/* write at cursor address + increment */
			space().write_byte(m_cursor_l | (m_cursor_h << 8), m_buffer);
			if(!(++m_cursor_l))
				m_cursor_h++;
			LOG2674(("DELAYED write at cursor address+increment %02x\n",data));
			break;

		case 0xbb:
			/* write from cursor address to pointer address TODO: transfer only during blank*/
			for(i = m_cursor_l | (m_cursor_h << 8); i != (m_screen2_l | (m_screen2_h << 8)); i = ((i + 1) & 0xffff))
				space().write_byte(i, m_buffer);
			space().write_byte(i, m_buffer); // get the last
			m_cursor_l = m_screen2_l;
			m_cursor_h = m_screen2_h;
			LOG2674(("DELAYED write from cursor address to pointer address %02x\n",data));
			break;

		case 0xbd:
			/* read from cursor address to pointer address */
			LOG2674(("DELAYED read from cursor address to pointer address %02x\n",data));
			break;
	}
}


READ8_MEMBER( scn2674_device::read )
{
	/*
	Offset:  Purpose
	 0       Interrupt Register
	 1       Status Register
	 2       Screen Start 1 Lower Register
	 3       Screen Start 1 Upper Register
	 4       Cursor Address Lower Register
	 5       Cursor Address Upper Register
	 6       Screen Start 2 Lower Register
	 7       Screen Start 2 Upper Register
	*/

	switch (offset)
	{
		/*  Status / Irq Register

		    --RV ZSRs

		 6+7 -- = ALWAYS 0
		  5  R  = RDFLG (Status Register Only)
		  4  V  = Vblank
		  3  Z  = Line Zero
		  2  S  = Split 1
		  1  R  = Ready
		  0  s  = Split 2
		*/

		case 0:
			LOG2674(("Read Irq Register %02x %06x\n",m_irq_register,space.device().safe_pc()));
			return m_irq_register;

		case 1:
			LOG2674(("Read Status Register %02X %06x\n",m_status_register,space.device().safe_pc()));
			return m_status_register;

		case 2: LOG2674(("Read Screen1_l Register %06x\n",space.device().safe_pc()));return m_screen1_l;
		case 3: LOG2674(("Read Screen1_h Register %06x\n",space.device().safe_pc()));return m_screen1_h & 0x3f;
		case 4: LOG2674(("Read Cursor_l Register %06x\n",space.device().safe_pc()));return m_cursor_l;
		case 5: LOG2674(("Read Cursor_h Register %06x\n",space.device().safe_pc()));return m_cursor_h;
		case 6: LOG2674(("Read Screen2_l Register %06x\n",space.device().safe_pc()));return m_screen2_l;
		case 7: LOG2674(("Read Screen2_h Register %06x\n",space.device().safe_pc()));return m_screen2_h;
	}

	return 0xff;
}


WRITE8_MEMBER( scn2674_device::write )
{
	/*
	Offset:  Purpose
	 0       Initialization Registers
	 1       Command Register
	 2       Screen Start 1 Lower Register
	 3       Screen Start 1 Upper Register
	 4       Cursor Address Lower Register
	 5       Cursor Address Upper Register
	 6       Screen Start 2 Lower Register
	 7       Screen Start 2 Upper Register
	*/

	switch (offset)
	{
		case 0:
			write_init_regs(data);
			break;

		case 1:
			write_command(data);
			break;

		case 2:
			m_screen1_l = data;
			if(!m_screen->vblank())
				m_start1change = (m_linecounter / m_IR0_scanline_per_char_row) + 1;
			break;
		case 3:
			m_screen1_h = data;
			m_dbl1=(data & 0xc0)>>6;
			if (m_IR0_double_ht_wd)
			{
				m_IR14_double_1 = m_dbl1;
				m_screen1_h &= 0x3f;
				LOG2674(("IR14 - Double 1 overridden %02x\n",m_IR14_double_1));
			}
			if(!m_screen->vblank())
				m_start1change = (m_linecounter / m_IR0_scanline_per_char_row) + 1;
			break;

		case 4: m_cursor_l  = data; break;
		case 5: m_cursor_h  = (data & 0x3f); break;
		case 6: m_screen2_l = data; break;
		case 7:
			m_screen2_h = (data&0x3f);
			m_spl1 = (data & 0x40);
			m_spl2 = (data & 0x80);
			break;
	}
}

void scn2674_device::recompute_parameters()
{
	m_hpixels_per_column = m_gfx_enabled ? m_gfx_hpixels_per_column : m_text_hpixels_per_column;
	int horiz_pix_total = ((m_IR1_equalizing_constant + (m_IR2_horz_sync_width << 1)) << 1) * m_hpixels_per_column;
	int vert_pix_total = m_IR4_rows_per_screen * m_IR0_scanline_per_char_row + m_IR3_vert_front_porch + m_IR3_vert_back_porch + m_IR7_vsync_width;
	attoseconds_t refresh = m_screen->frame_period().attoseconds();
	int max_visible_x = (m_IR5_character_per_row * m_hpixels_per_column) - 1;
	int max_visible_y = (m_IR4_rows_per_screen * m_IR0_scanline_per_char_row) - 1;

	if(!horiz_pix_total || !vert_pix_total)
	{
		m_scanline_timer->adjust(attotime::never);
		return;
	}

	LOG2674(("width %u height %u max_x %u max_y %u refresh %f\n", horiz_pix_total, vert_pix_total, max_visible_x, max_visible_y, 1 / ATTOSECONDS_TO_DOUBLE(refresh)));

	rectangle visarea;
	visarea.set(0, max_visible_x, 0, max_visible_y);
	m_screen->configure(horiz_pix_total, vert_pix_total, visarea, refresh);

	m_scanline_timer->adjust(m_screen->time_until_pos(0, 0), 0, m_screen->scan_period());
}

void scn2674_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
		case TIMER_SCANLINE:
		{
			int dw = m_IR0_double_ht_wd ? m_IR14_double_1 : 0;  // double width
			if(((m_display_enabled_scanline) || (m_display_enabled_field && !m_IR1_interlace_enable)) && (!m_display_enabled))
			{
				m_display_enabled = 1;
				m_display_enabled_scanline = 0;
				m_display_enabled_field = 0;
			}

			m_linecounter++;

			if(m_linecounter >= m_screen->height())
			{
				m_linecounter = 0;
				m_address = (m_screen1_h << 8) | m_screen1_l;
			}

			if(m_linecounter == (m_IR4_rows_per_screen * m_IR0_scanline_per_char_row))
			{
				m_status_register |= 0x10;
				if(m_irq_mask & 0x10)
				{
					LOG2674(("vblank irq\n"));
					m_irq_state = 1;
					m_irq_register |= 0x10;
					m_irq_cb(1);
				}
			}

			if(m_linecounter >= (m_IR4_rows_per_screen * m_IR0_scanline_per_char_row))
				break;

			int charrow = m_linecounter % m_IR0_scanline_per_char_row;
			int tilerow = charrow;

			// should be triggered at the start of each ROW (line zero for that row)
			if(!charrow)
			{
				m_status_register |= 0x08;
				if (m_irq_mask & 0x08)
				{
					LOG2674(("SCN2674 Line Zero\n"));
					m_irq_state = 1;
					m_irq_register |= 0x08;
					m_irq_cb(1);
				}
			}

			if((m_linecounter == (m_IR12_split_register_1 * m_IR0_scanline_per_char_row)) && m_linecounter) /* Split Screen 1 */
			{
				m_status_register |= 0x04;
				if(m_irq_mask & 0x04)
				{
					LOG2674(("SCN2674 Split Screen 1 irq\n"));
					m_irq_state = 1;
					m_irq_register |= 0x04;
					m_irq_cb(1);
				}
				if(m_spl1)
					m_address = (m_screen2_h << 8) | m_screen2_l;
				if(!m_IR0_double_ht_wd)
					dw = m_IR14_double_1;
			}

			if((m_linecounter == (m_IR13_split_register_2 * m_IR0_scanline_per_char_row)) && m_linecounter) /* Split Screen 2 */
			{
				m_status_register |= 0x01;
				if(m_irq_mask & 0x01)
				{
					LOG2674(("SCN2674 Split Screen 2 irq\n"));
					m_irq_state = 1;
					m_irq_register |= 0x01;
					m_irq_cb(1);
				}
				if(m_spl2)
					m_address = (m_screen2_h << 8) | m_screen2_l;
				if(!m_IR0_double_ht_wd)
					dw = m_IR14_double_2;
			}

			if(!m_display_enabled)
				break;

			if(m_IR2_row_table)
			{
				if(m_IR0_double_ht_wd)
					dw = m_screen1_h >> 6;
				if(!charrow)
				{
					UINT16 addr = (m_screen2_h << 8) | m_screen2_l;
					UINT16 line = space().read_word(addr);
					m_screen1_h = (line >> 8);
					m_screen1_l = line & 0xff;
					if(m_IR0_double_ht_wd)
					{
						dw = line >> 14;
						line &= ~0xc000;
					}
					m_address = line;
					addr += 2;
					m_screen2_h = (addr >> 8) & 0x3f;
					m_screen2_l = addr & 0xff;
				}
			}
			else if(m_start1change && (m_start1change == (m_linecounter / m_IR0_scanline_per_char_row)))
			{
				m_address = (m_screen1_h << 8) | m_screen1_l;
				m_start1change = 0;
			}

			if(dw == 2)
				tilerow >>= 1;
			else if(dw == 3)
				tilerow = (charrow + m_IR0_scanline_per_char_row) >> 1;

			UINT16 address = m_address;

			for(int i = 0; i < m_IR5_character_per_row; i++)
			{
				bool cursor_on = ((address & 0x3fff) == ((m_cursor_h << 8) | m_cursor_l));

				if (!m_display_cb.isnull())
					m_display_cb(m_bitmap,
									i * m_hpixels_per_column,
									m_linecounter,
									tilerow,
									space().read_byte(address),
									address,
									(charrow >= m_IR6_cursor_first_scanline) && (charrow <= m_IR6_cursor_last_scanline) && cursor_on,
									dw != 0,
									m_gfx_enabled != 0,
									charrow == m_IR7_cursor_underline_position,
									m_IR7_cursor_blink && (m_screen->frame_number() & (m_IR7_cursor_rate_divisor ? 0x40 : 0x20)));
				address = (address + 1) & 0xffff;

				if(address > ((m_IR9_display_buffer_last_address << 10) | 0x3ff))
					address = (m_IR9_display_buffer_first_address_MSB << 8) | m_IR8_display_buffer_first_address_LSB;
			}

			if(m_gfx_enabled || (charrow == (m_IR0_scanline_per_char_row - 1)))
				m_address = address;
		}
	}
}

UINT32 scn2674_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (!m_display_enabled)
		m_bitmap.fill(rgb_t::black);
	else
		copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);

	return 0;
}
