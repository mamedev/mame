/*
    SCN2674 - Advanced Video Display Controller (AVDC)  (Video Chip)

    This is a somewhat terrible implementation and should probably just be rewritten from scratch
    it is currently used by mpu4vid.c and still quite heavily tied to the behavior of that.

    I don't know if the timing bugs with those games comes from this, or emulation of the other
    MPU4 devices tho because even some of the non-video games seem laggy and prone to failure.

    -- currently expects (from the host driver)
     decoded gfx in regions 0,1,2,3 in various formats (normal, double height etc.)
      eventually the chip should just handle this without the need to decode anything

     a callback function for when the irqs are changed / updated

     a call from the video start from your video start function

     a call to the scanline function each scanline

     a call to the video draw (with ram pointer) from a screen update function

     video to be on the primary screen

     this could all be simplified / changed, the chip can actually be hooked up in various ways
     including working on a per scanline basis with almost no ram
*/

#include "emu.h"
#include "scn2674.h"



const device_type SCN2674_VIDEO = &device_creator<scn2674_device>;


static void default_scn2674_callback(running_machine &machine)
{
//  logerror("no scn2674_callback\n");
}


scn2674_device::scn2674_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SCN2674_VIDEO, "scn2674_device", tag, owner, clock)
{
	m_interrupt_callback = default_scn2674_callback;
}




void scn2674_device::device_start()
{
}

void scn2674_device::device_reset()
{
	m_scn2674_IR_pointer= 0;
	m_scn2674_screen1_l= 0;
	m_scn2674_screen1_h= 0;
	m_scn2674_cursor_l= 0;
	m_scn2674_cursor_h= 0;
	m_scn2674_screen2_l= 0;
	m_scn2674_screen2_h= 0;
	m_scn2674_irq_register= 0;
	m_scn2674_status_register= 0;
	m_scn2674_irq_mask= 0;
	m_scn2674_gfx_enabled= 0;
	m_scn2674_display_enabled= 0;
	m_scn2674_display_enabled_field= 0;
	m_scn2674_display_enabled_scanline= 0;
	m_scn2674_cursor_enabled= 0;
	m_IR0_scn2674_double_ht_wd= 0;
	m_IR0_scn2674_scanline_per_char_row= 0;
	m_IR0_scn2674_sync_select= 0;
	m_IR0_scn2674_buffer_mode_select= 0;
	m_IR1_scn2674_interlace_enable= 0;
	m_IR1_scn2674_equalizing_constant= 0;
	m_IR2_scn2674_row_table= 0;
	m_IR2_scn2674_horz_sync_width= 0;
	m_IR2_scn2674_horz_back_porch= 0;
	m_IR3_scn2674_vert_front_porch= 0;
	m_IR3_scn2674_vert_back_porch= 0;
	m_IR4_scn2674_rows_per_screen= 0;
	m_IR4_scn2674_character_blink_rate_divisor= 0;
	m_IR5_scn2674_character_per_row= 0;
	m_IR6_scn2674_cursor_first_scanline= 0;
	m_IR6_scn2674_cursor_last_scanline= 0;
	m_IR7_scn2674_cursor_underline_position= 0;
	m_IR7_scn2674_cursor_rate_divisor= 0;
	m_IR7_scn2674_cursor_blink= 0;
	m_IR7_scn2674_vsync_width= 0;
	m_IR8_scn2674_display_buffer_first_address_LSB= 0;
	m_IR9_scn2674_display_buffer_first_address_MSB= 0;
	m_IR9_scn2674_display_buffer_last_address= 0;
	m_IR10_scn2674_display_pointer_address_lower= 0;
	m_IR11_scn2674_display_pointer_address_upper= 0;
	m_IR11_scn2674_reset_scanline_counter_on_scrollup= 0;
	m_IR11_scn2674_reset_scanline_counter_on_scrolldown= 0;
	m_IR12_scn2674_scroll_start= 0;
	m_IR12_scn2674_split_register_1= 0;
	m_IR13_scn2674_scroll_end= 0;
	m_IR13_scn2674_split_register_2= 0;
	m_IR14_scn2674_scroll_lines= 0;
	m_IR14_scn2674_double_1= 0;
	m_IR14_scn2674_double_2= 0;
	m_scn2674_horz_front_porch= 0;
	m_scn2674_spl1= 0;
	m_scn2674_spl2= 0;
	m_scn2674_dbl1= 0;
	m_rowcounter= 0;
	m_linecounter= 0;
	m_scn2674_irq_state= 0;
}


void scn2674_device::set_irq_update_callback(device_t &device, s2574_interrupt_callback_func callback)
{
	scn2674_device &dev = downcast<scn2674_device &>(device);
	dev.m_interrupt_callback = callback;
}

// 15 Initialization Registers (8-bit each)
void scn2674_device::scn2674_write_init_regs(UINT8 data)
{
	LOG2674(("scn2674_write_init_regs %02x %02x\n",m_scn2674_IR_pointer,data));

//  m_scn2674_IR[m_scn2674_IR_pointer]=data;


	switch ( m_scn2674_IR_pointer) /* display some debug info, set mame specific variables */
	{
		case 0:
			m_IR0_scn2674_double_ht_wd = (data & 0x80)>>7;
			m_IR0_scn2674_scanline_per_char_row = ((data & 0x78)>>3) + 1;
			m_IR0_scn2674_sync_select = (data&0x04)>>2;
			m_IR0_scn2674_buffer_mode_select = (data&0x03);

			LOG2674(("IR0 - Double Ht Wd %02x\n",m_IR0_scn2674_double_ht_wd));//affects IR14 as well
			LOG2674(("IR0 - Scanlines per Character Row %02x\n",m_IR0_scn2674_scanline_per_char_row));//value+1 = scanlines

			if (m_IR0_scn2674_scanline_per_char_row != 8)
			{
				popmessage("Row size change, contact MAMEDEV");
			}
			LOG2674(("IR0 - Sync Select %02x\n",m_IR0_scn2674_sync_select));//1 = csync
			LOG2674(("IR0 - Buffer Mode Select %02x\n",m_IR0_scn2674_buffer_mode_select)); //0 independent 1 transparent 2 shared 3 row
			break;

		case 1:
			m_IR1_scn2674_interlace_enable = (data&0x80)>>7;
			m_IR1_scn2674_equalizing_constant = (data&0x7f)+1;

			LOG2674(("IR1 - Interlace Enable %02x\n",m_IR1_scn2674_interlace_enable));
			LOG2674(("IR1 - Equalizing Constant %02i CCLKs\n",m_IR1_scn2674_equalizing_constant));
			break;

		case 2:
			m_IR2_scn2674_row_table = (data&0x80)>>7;
			m_IR2_scn2674_horz_sync_width = (((data&0x78)>>3)*2) + 2;
			m_IR2_scn2674_horz_back_porch = ((data&0x07)*4) - 1;

			LOG2674(("IR2 - Row Table %02x\n",m_IR2_scn2674_row_table));
			LOG2674(("IR2 - Horizontal Sync Width %02i CCLKs\n",m_IR2_scn2674_horz_sync_width));
			LOG2674(("IR2 - Horizontal Back Porch %02i CCLKs\n",m_IR2_scn2674_horz_back_porch));
			break;

		case 3:
			m_IR3_scn2674_vert_front_porch =  (((data&0xe0)>>5) * 4)+4 ;
			m_IR3_scn2674_vert_back_porch = ((data&0x1f) * 2) + 4;

			LOG2674(("IR3 - Vertical Front Porch %02i Lines\n",m_IR3_scn2674_vert_front_porch));
			LOG2674(("IR3 - Vertical Back Porch %02i Lines\n",m_IR3_scn2674_vert_back_porch));
			break;

		case 4:
			m_IR4_scn2674_rows_per_screen = (data&0x7f) + 1;
			m_IR4_scn2674_character_blink_rate_divisor = ((data & 0x80)>>7 ? 128:64);

			LOG2674(("IR4 - Rows Per Screen %02i\n",m_IR4_scn2674_rows_per_screen));
			LOG2674(("IR4 - Character Blink Rate = 1/%02i\n",m_IR4_scn2674_character_blink_rate_divisor));
			break;

		case 5:
		   /* IR5 - Active Characters Per Row
             cccc cccc
             c = Characters Per Row */
			m_IR5_scn2674_character_per_row = data + 1;
			LOG2674(("IR5 - Active Characters Per Row %02i\n",m_IR5_scn2674_character_per_row));
			break;

		case 6:
			m_IR6_scn2674_cursor_last_scanline = (data & 0x0f);
			m_IR6_scn2674_cursor_first_scanline = (data & 0xf0)>>4;
			LOG2674(("IR6 - First Line of Cursor %02x\n",m_IR6_scn2674_cursor_first_scanline));
			LOG2674(("IR6 - Last Line of Cursor %02x\n",m_IR6_scn2674_cursor_last_scanline));
			break;

		case 7:
			m_IR7_scn2674_cursor_underline_position = (data & 0x0f);
			m_IR7_scn2674_cursor_rate_divisor = ((data & 0x10)>>4 ? 64:32);
			m_IR7_scn2674_cursor_blink = (data & 0x20)>>5;

			m_IR7_scn2674_vsync_width = vsync_table[(data & 0xC0)>>6];

			LOG2674(("IR7 - Underline Position %02x\n",m_IR7_scn2674_cursor_underline_position));
			LOG2674(("IR7 - Cursor rate 1/%02i\n",m_IR7_scn2674_cursor_rate_divisor));
			LOG2674(("IR7 - Cursor blink %02x\n",m_IR7_scn2674_cursor_blink));
			LOG2674(("IR7 - Vsync Width  %02i Lines\n",m_IR7_scn2674_vsync_width));
			break;

		case 8:
			m_IR8_scn2674_display_buffer_first_address_LSB = data;
			LOG2674(("IR8 - Display Buffer First Address LSB %02x\n",m_IR8_scn2674_display_buffer_first_address_LSB));
			break;

		case 9:
			m_IR9_scn2674_display_buffer_first_address_MSB = data & 0x0f;
			m_IR9_scn2674_display_buffer_last_address = (data & 0xf0)>>4;
			LOG2674(("IR9 - Display Buffer First Address MSB %02x\n",m_IR9_scn2674_display_buffer_first_address_MSB));
			LOG2674(("IR9 - Display Buffer Last Address %02x\n",m_IR9_scn2674_display_buffer_last_address));
			break;

		case 10:
			m_IR10_scn2674_display_pointer_address_lower = data;
			LOG2674(("IR10 - Display Pointer Address Lower %02x\n",m_IR10_scn2674_display_pointer_address_lower));
			break;

		case 11:
			m_IR11_scn2674_display_pointer_address_upper= data&0x3f;
			m_IR11_scn2674_reset_scanline_counter_on_scrollup= (data&0x40 >> 6);
			m_IR11_scn2674_reset_scanline_counter_on_scrolldown= (data&0x80 >> 7);

			LOG2674(("IR11 - Display Pointer Address Lower %02x\n",m_IR11_scn2674_display_pointer_address_upper));
			LOG2674(("IR11 - Reset Scanline Counter on Scroll Up %02x\n",m_IR11_scn2674_reset_scanline_counter_on_scrollup));
			LOG2674(("IR11 - Reset Scanline Counter on Scroll Down %02x\n",m_IR11_scn2674_reset_scanline_counter_on_scrolldown));
			break;

		case 12:
			m_IR12_scn2674_scroll_start = (data & 0x80)>>7;
			m_IR12_scn2674_split_register_1 = (data & 0x7f);
			LOG2674(("IR12 - Scroll Start %02x\n",m_IR12_scn2674_scroll_start));
			LOG2674(("IR12 - Split Register 1 %02x\n",m_IR12_scn2674_split_register_1));
			break;

		case 13:
			m_IR13_scn2674_scroll_end = (data & 0x80)>>7;
			m_IR13_scn2674_split_register_2 = (data & 0x7f);
			LOG2674(("IR13 - Scroll End %02x\n",m_IR13_scn2674_scroll_end));
			LOG2674(("IR13 - Split Register 2 %02x\n",m_IR13_scn2674_split_register_2));
			break;

		case 14:
			m_IR14_scn2674_scroll_lines = (data & 0x0f);
			if (!m_IR0_scn2674_double_ht_wd)
			{
				m_IR14_scn2674_double_2 = (data & 0x30)>>4;
				LOG2674(("IR14 - Double 2 %02x\n",m_IR14_scn2674_double_2));
			}
			//0 normal, 1, double width, 2, double width and double tops 3, double width and double bottoms
			//1 affects SSR1, 2 affects SSR2
			//If Double Height enabled in IR0, Screen start 1 upper (bits 7 and 6)replace Double 1, and Double 2 is unused
			m_IR14_scn2674_double_1 = (data & 0xc0)>>6;
			LOG2674(("IR14 - Double 1 %02x\n",m_IR14_scn2674_double_1));

			LOG2674(("IR14 - Scroll Lines %02i\n",m_IR14_scn2674_scroll_lines));
			break;

		case 15: /* not valid! */
			break;

	}

	m_scn2674_horz_front_porch = 2*(m_IR1_scn2674_equalizing_constant) + 3*(m_IR2_scn2674_horz_sync_width)-(m_IR5_scn2674_character_per_row) - m_IR2_scn2674_horz_back_porch;
	LOG2674(("Horizontal Front Porch %02x CCLKs\n",m_scn2674_horz_front_porch));

	m_scn2674_IR_pointer++;
	if (m_scn2674_IR_pointer>14)m_scn2674_IR_pointer=14;
}

void scn2674_device::scn2674_write_command(running_machine &machine, UINT8 data)
{
	UINT8 operand;
	int i;

	LOG2674(("scn2674_write_command %02x\n",data));

	if (data==0x00)
	{
		/* master reset, configures registers */
		LOG2674(("master reset\n"));
		m_scn2674_IR_pointer=0;
		m_scn2674_irq_register = 0x00;
		m_scn2674_status_register = 0x20;//RDFLG activated
		m_linecounter =0;
		m_rowcounter =0;
		m_scn2674_irq_mask = 0x00;
		m_scn2674_gfx_enabled = 0;
		m_scn2674_display_enabled = 0;
		m_scn2674_cursor_enabled = 0;
		m_IR2_scn2674_row_table = 0;
	}

	if ((data&0xf0)==0x10)
	{
		/* set IR pointer */
		operand = data & 0x0f;
		LOG2674(("set IR pointer %02x\n",operand));

		m_scn2674_IR_pointer=operand;

	}

	/* ANY COMBINATION OF THESE ARE POSSIBLE */

	if ((data&0xe3)==0x22)
	{
		/* Disable GFX */
		LOG2674(("disable GFX %02x\n",data));
		m_scn2674_gfx_enabled = 0;
	}

	if ((data&0xe3)==0x23)
	{
		/* Enable GFX */
		LOG2674(("enable GFX %02x\n",data));
		m_scn2674_gfx_enabled = 1;
	}

	if ((data&0xe9)==0x28)
	{
		/* Display off */
		operand = data & 0x04;

		m_scn2674_display_enabled = 0;

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
			m_scn2674_display_enabled_field = 1;
			LOG2674(("display ON - next field %02x\n",data));
		}
		else
		{
			m_scn2674_display_enabled_scanline = 1;
			LOG2674(("display ON - next scanline %02x\n",data));
		}
	}

	if ((data&0xf1)==0x30)
	{
		/* Cursor Off */
		LOG2674(("cursor off %02x\n",data));
		m_scn2674_cursor_enabled = 0;
	}

	if ((data&0xf1)==0x31)
	{
		/* Cursor On */
		LOG2674(("cursor on %02x\n",data));
		m_scn2674_cursor_enabled = 1;
	}

	/* END */

	if ((data&0xe0)==0x40)
	{
		/* Reset Interrupt / Status bit */
		operand = data & 0x1f;
		LOG2674(("reset interrupt / status bit %02x\n",operand));

		m_scn2674_irq_register &= ~(data & 0x1f);
		m_scn2674_status_register &= ~(data & 0x1f);

		LOG2674(("IRQ Status after reset\n"));
		LOG2674(("Split 2   IRQ: %d Active\n",(m_scn2674_irq_register>>0)&1));
		LOG2674(("Ready     IRQ: %d Active\n",(m_scn2674_irq_register>>1)&1));
		LOG2674(("Split 1   IRQ: %d Active\n",(m_scn2674_irq_register>>2)&1));
		LOG2674(("Line Zero IRQ: %d Active\n",(m_scn2674_irq_register>>3)&1));
		LOG2674(("V-Blank   IRQ: %d Active\n",(m_scn2674_irq_register>>4)&1));

		m_scn2674_irq_state = 0;

		for (i = 0; i < 5; i++)
		{
			if ((m_scn2674_irq_register>>i&1)&(m_scn2674_irq_mask>>i&1))
			{
				m_scn2674_irq_state = 1;
			}
		}
		m_interrupt_callback(machine);

	}
	if ((data&0xe0)==0x80)
	{
		/* Disable Interrupt mask*/
		operand = data & 0x1f;
		m_scn2674_irq_mask &= ~(operand);
		LOG2674(("IRQ Mask after disable %x\n",operand));
		LOG2674(("Split 2   IRQ: %d Unmasked\n",(m_scn2674_irq_mask>>0)&1));
		LOG2674(("Ready     IRQ: %d Unmasked\n",(m_scn2674_irq_mask>>1)&1));
		LOG2674(("Split 1   IRQ: %d Unmasked\n",(m_scn2674_irq_mask>>2)&1));
		LOG2674(("Line Zero IRQ: %d Unmasked\n",(m_scn2674_irq_mask>>3)&1));
		LOG2674(("V-Blank   IRQ: %d Unmasked\n",(m_scn2674_irq_mask>>4)&1));

	}

	if ((data&0xe0)==0x60)
	{
		/* Enable Interrupt mask*/
		operand = data & 0x1f;
		m_scn2674_irq_mask |= (data & 0x1f);

		LOG2674(("IRQ Mask after enable %x\n",operand));
		LOG2674(("Split 2   IRQ: %d Unmasked\n",(m_scn2674_irq_mask>>0)&1));
		LOG2674(("Ready     IRQ: %d Unmasked\n",(m_scn2674_irq_mask>>1)&1));
		LOG2674(("Split 1   IRQ: %d Unmasked\n",(m_scn2674_irq_mask>>2)&1));
		LOG2674(("Line Zero IRQ: %d Unmasked\n",(m_scn2674_irq_mask>>3)&1));
		LOG2674(("V-Blank   IRQ: %d Unmasked\n",(m_scn2674_irq_mask>>4)&1));

	}

	/* Delayed Commands */
	/* These set 0x20 in status register when done */

	if (data == 0xa4)
	{
		/* read at pointer address */
		LOG2674(("DELAYED read at pointer address %02x\n",data));
	}

	if (data == 0xa2)
	{
		/* write at pointer address */
		LOG2674(("DELAYED write at pointer address %02x\n",data));
	}

	if (data == 0xa9)
	{
		/* increase cursor address */
		LOG2674(("DELAYED increase cursor address %02x\n",data));
	}

	if (data == 0xac)
	{
		/* read at cursor address */
		LOG2674(("DELAYED read at cursor address %02x\n",data));
	}

	if (data == 0xaa)
	{
		/* write at cursor address */
		LOG2674(("DELAYED write at cursor address %02x\n",data));
	}

	if (data == 0xad)
	{
		/* read at cursor address + increment */
		LOG2674(("DELAYED read at cursor address+increment %02x\n",data));
	}

	if (data == 0xab)
	{
		/* write at cursor address + increment */
		LOG2674(("DELAYED write at cursor address+increment %02x\n",data));
	}

	if (data == 0xbb)
	{
		/* write from cursor address to pointer address */
		LOG2674(("DELAYED write from cursor address to pointer address %02x\n",data));
	}

	if (data == 0xbd)
	{
		/* read from cursor address to pointer address */
		LOG2674(("DELAYED read from cursor address to pointer address %02x\n",data));
	}
}


READ16_MEMBER( scn2674_device::mpu4_vid_scn2674_r )
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
			LOG2674(("Read Irq Register %02x %06x\n",m_scn2674_irq_register,space.device().safe_pc()));
			return m_scn2674_irq_register;

		case 1:
			LOG2674(("Read Status Register %02X %06x\n",m_scn2674_status_register,space.device().safe_pc()));
			return m_scn2674_status_register;

		case 2: LOG2674(("Read Screen1_l Register %06x\n",space.device().safe_pc()));return m_scn2674_screen1_l;
		case 3: LOG2674(("Read Screen1_h Register %06x\n",space.device().safe_pc()));return m_scn2674_screen1_h;
		case 4: LOG2674(("Read Cursor_l Register %06x\n",space.device().safe_pc()));return m_scn2674_cursor_l;
		case 5: LOG2674(("Read Cursor_h Register %06x\n",space.device().safe_pc()));return m_scn2674_cursor_h;
		case 6:	LOG2674(("Read Screen2_l Register %06x\n",space.device().safe_pc()));return m_scn2674_screen2_l;
		case 7: LOG2674(("Read Screen2_h Register %06x\n",space.device().safe_pc()));return m_scn2674_screen2_h;
	}

	return 0xffff;
}


WRITE16_MEMBER( scn2674_device::mpu4_vid_scn2674_w )
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

	data &=0x00ff; /* it's an 8-bit chip on a 16-bit board, feel the cheapness. */

	switch (offset)
	{
		case 0:
			scn2674_write_init_regs(data);
			break;

		case 1:
			scn2674_write_command(space.machine(), data);
			break;

		case 2: m_scn2674_screen1_l = data; break;
		case 3:
			m_scn2674_screen1_h = (data&0x3f);//uppermost two bytes not part of register
			m_scn2674_dbl1=(data & 0xc0)>>6;
			if (m_IR0_scn2674_double_ht_wd)
			{
				m_IR14_scn2674_double_1 = m_scn2674_dbl1;
				LOG2674(("IR14 - Double 1 overridden %02x\n",m_IR14_scn2674_double_1));
			}
			break;

		case 4: m_scn2674_cursor_l  = data; break;
		case 5: m_scn2674_cursor_h  = data; break;
		case 6:	m_scn2674_screen2_l = data; break;
		case 7:
			m_scn2674_screen2_h = (data&0x3f);
			m_scn2674_spl1 = (data & 0x40);
			m_scn2674_spl2 = (data & 0x80);
			break;

		break;
	}
}



void scn2674_device::scn2674_line(running_machine &machine)
{
	if (m_linecounter==0)/* Ready - this triggers for the first scanline of the screen */
	{
		m_scn2674_status_register |= 0x02;
		if (m_scn2674_irq_mask&0x02)
		{
			LOG2674(("SCN2674 Ready\n"));
			m_scn2674_irq_state = 1;
			m_scn2674_irq_register |= 0x02;
			m_interrupt_callback(machine);
		}
	}

	// should be triggered at the start of each ROW (line zero for that row)
	if (( m_linecounter%8 == 0)&& (m_linecounter < 297) )
	{
		m_scn2674_status_register |= 0x08;
		if (m_scn2674_irq_mask&0x08)
		{
			LOG2674(("SCN2674 Line Zero\n"));
			m_scn2674_irq_state = 1;
			m_scn2674_irq_register |= 0x08;
			m_interrupt_callback(machine);
		}
			m_rowcounter = ((m_rowcounter+1)% 37);//Not currently used
	}

	// this is ROWS not scanlines!!
	if ((m_linecounter == m_IR12_scn2674_split_register_1*8)&&(m_linecounter != 0))
	/* Split Screen 1 */
	{
		if (m_scn2674_spl1)
		{
			popmessage("Split screen 1 address shift required, contact MAMEDEV");
		}
		m_scn2674_status_register |= 0x04;
		if (m_scn2674_irq_mask&0x04)
		{
			machine.primary_screen->update_partial(m_linecounter);
			m_scn2674_irq_register |= 0x04;
			LOG2674(("SCN2674 Split Screen 1\n"));
			m_scn2674_irq_state = 1;
			m_interrupt_callback(machine);
//          machine.primary_screen->update_partial(m_linecounter);
		}
	}

	// this is in ROWS not scanlines!!!
	if ((m_linecounter == m_IR13_scn2674_split_register_2*8)&&(m_linecounter != 0))
	/* Split Screen 2 */
	{
		if (m_scn2674_spl2)
		{
			popmessage("Split screen 2 address shift required, contact MAMEDEV");
		}
		m_scn2674_status_register |= 0x01;
		if (m_scn2674_irq_mask&0x01)
		{
			machine.primary_screen->update_partial(m_linecounter);
			LOG2674(("SCN2674 Split Screen 2 irq\n"));
			m_scn2674_irq_state = 1;
			m_scn2674_irq_register |= 0x01;
			m_interrupt_callback(machine);
			//machine.primary_screen->update_partial(m_linecounter);
		}
	}

	if (m_linecounter==296)//front porch
	{

		m_scn2674_status_register |= 0x10;
		if (m_scn2674_irq_mask&0x10)
		{
			LOG2674(("vblank irq\n"));
			m_scn2674_irq_state = 1;
			m_scn2674_irq_register |= 0x10;
			m_interrupt_callback(machine);
		}

	}

}


// scanline timer


void scn2674_device::scn2674_do_scanline(running_machine &machine, int scanline)
{
	//This represents the scanline counter in the SCN2674. Note that we ignore the horizontal blanking

	if (((m_scn2674_display_enabled_scanline) || (m_scn2674_display_enabled_field && (m_IR1_scn2674_interlace_enable == 0)))&&(!m_scn2674_display_enabled))
	{
		m_scn2674_display_enabled = 1;
		m_scn2674_display_enabled_scanline = 0;
		m_scn2674_display_enabled_field = 0;
	}
	if (m_scn2674_display_enabled)
	{
		m_linecounter = scanline;
	}
	else
	{
		m_linecounter =297;//hold the counter in the vsync point, it's not clear whether this is done or not
	}
	scn2674_line(machine);
//  timer.machine().scheduler().synchronize();
}


////// screen update
template<class _BitmapClass>
void scn2674_device::scn2574_draw_common( running_machine &machine, _BitmapClass &bitmap, const rectangle &cliprect, UINT16* vid_mainram )
{
	int x, y/*, count = 0*/;
	/* this is in main ram.. i think it must transfer it out of here??? */
	/* count = 0x0018b6/2; - crmaze count = 0x004950/2; - turnover */
	/* we're in row table mode...thats why */
	for(y = 0; y < m_IR4_scn2674_rows_per_screen; y++)
	{
		int screen2_base = (m_scn2674_screen2_h << 8) | m_scn2674_screen2_l;
		UINT16 rowbase = (vid_mainram[1+screen2_base+(y*2)]<<8)|vid_mainram[screen2_base+(y*2)];
		int dbl_size=0;
		int gfxregion = 0;

		if (m_IR0_scn2674_double_ht_wd)
		{
			dbl_size = (rowbase & 0xc000)>>14;  /* ONLY if double size is enabled.. otherwise it can address more chars given more RAM */
		}

		if (dbl_size&2)
		{
			gfxregion = 1;
		}
		for(x = 0; x < m_IR5_scn2674_character_per_row; x++)
		{
			UINT16 tiledat;
			UINT16 attr;

			tiledat = vid_mainram[(rowbase+x)&0x7fff];
			attr = tiledat >>12;

			if (attr)
				drawgfx_opaque(bitmap,cliprect,machine.gfx[gfxregion],tiledat,0,0,0,(x*8),(y*8));

		}
		if (dbl_size&2)
		{
			y++;/* skip a row? */
		}

	}
}

void scn2674_device::scn2574_draw( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, UINT16* vid_mainram )
{ scn2574_draw_common(machine, bitmap, cliprect, vid_mainram); }

void scn2674_device::scn2574_draw( running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect, UINT16* vid_mainram )
{ scn2574_draw_common(machine, bitmap, cliprect, vid_mainram); }

void scn2674_device::init_stuff()
{
	// from video start
	m_scn2674_IR_pointer = 0;
}
