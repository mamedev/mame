/******************************************************************************
    BBC Model B

    MESS Driver By:

    Gordon Jefferyes
    mess_bbc@romvault.com

    This is the first go around at converting the BBC code over to using
    mames built in mc6845, there are a number of features now incorrect
    or missing in this build:

    Cursors are missing.
    Mode 7 is shifted to the right by a couple of character.
    BBC split modes no longer work (Like is used in Elite.)

******************************************************************************/

#include "emu.h"
#include "includes/bbc.h"
#include "video/saa5050.h"
#include "video/mc6845.h"

/************************************************************************
 * C0 and C1 along with MA12 output from the 6845 drive 4 NAND gates in ICs 27,36 and 40
 * the outputs from these NAND gates (B1 to B4) along with MA8 to MA11 from the 6845 (A1 to B4) are added together
 * in IC39 74LS283 4 bit adder to form (S1 to S4) the logic is used to loop the screen memory for hardware scrolling.
 * when MA13 from the 6845 is low the latches IC8 and IC9 are enabled
 * they control the memory addressing for the Hi-Res modes.
 * when MA13 from the 6845 is high the latches IC10 and IC11 are enabled
 * they control the memory addressing for the Teletext mode.
 * IC 8 or IC10 drives the row select in the memory (the lower 7 bits in the memory address) and
 * IC 9 or IC11 drives the column select in the memory (the next 7 bits in the memory address) this
 * gives control of the bottom 14 bits of the memory, in a 32K model B 15 bits are needed to access
 * all the RAM, so S4 for the adder drives the CAS0 and CAS1 to access the top bit, in a 16K model A
 * the output of S4 is linked out to a 0v supply by link S25 to just access the 16K memory area.
 ************************************************************************/

unsigned int calculate_video_address(bbc_state *state,int ma,int ra)
{
	// ma = output from IC2 6845 MA address

	int c0=state->m_b4_video0; // output from IC32 74LS259 bits 4 and 5
	int c1=state->m_b5_video1;

	/* the 4 bit input port b on IC39 are produced by 4 NAND gates.
	these NAND gates take their
	inputs from c0 and c1 (from IC32) and ma12 (from the 6845) */

	/* get bit m12 from the 6845 */
	int ma12=(ma>>12)&1;

	// 4 bit input B on IC39 74LS283 (4 bit adder)
	/* 3 input NAND part of IC 36 */
	int b1=(~(c1 & c0 & ma12)) & 1;
	/* 2 input NAND part of IC40 (b3 is calculated before b2 and b4 because b3 feed back into b2 and b4) */
	int b3=(~(c0 & ma12)) & 1;
	/* 3 input NAND part of IC 36 */
	int b2=(~(c1 & b3 & ma12)) & 1;
	/* 2 input NAND part of IC 27 */
	int b4=(~(b3 & ma12)) & 1;

	/* inputs port b to IC39 are taken from the NAND gates b1 to b4 */
	int b=(b1<<0)|(b2<<1)|(b3<<2)|(b4<<3);

	/* inputs port a to IC39 are MA8 to MA11 from the 6845 */
	int a=(ma>>8)&0xf;

	/* IC39 performs the 4 bit add with the carry input set high */
	int s=(a+b+1)&0xf;

	/* if MA13 (TTXVDU) is low then IC8 and IC9 are used to calculate
	   the memory location required for the hi res video.
	   if MA13 is hight then IC10 and IC11 are used to calculate the memory location for the teletext chip*/
	unsigned int m;
	if ((ma>>13)&1)
	{
		// IC 10 and IC 11
		m=((ma&0x3ff)|0x3c00)|((s&0x8)<<11);
	} else {
		// IC 8 and IC 9
		m=((ma&0xff)<<3)|(s<<11)|(ra&0x7);
	}
	if (state->m_memorySize==16)
		return  m & 0x3fff;

	return m;
}

/************************************************************************
 * VideoULA
 ************************************************************************/

static const int pixels_per_byte_set[8]={ 2,4,8,16,1,2,4,8 };

static const int width_of_cursor_set[8]={ 0,0,1,2,1,0,2,4 };

/* this is a quick lookup array that puts bits 0,2,4,6 into bits 0,1,2,3
   this is used by the pallette lookup in the video ULA */
static void set_pixel_lookup(bbc_state *state)
{
	int i;
	for (i=0; i<256; i++)
	{
		state->m_pixel_bits[i] = (((i>>7)&1)<<3) | (((i>>5)&1)<<2) | (((i>>3)&1)<<1) | (((i>>1)&1)<<0);
	}
}


WRITE8_MEMBER(bbc_state::bbc_videoULA_w)
{
	// Make sure vpos is never <0 2008-10-11 PHS.
	int vpos=machine().primary_screen->vpos();
	if(vpos==0)
		machine().primary_screen->update_partial(vpos);
	else
		machine().primary_screen->update_partial(vpos -1 );

	logerror("setting videoULA %.4x to:%.4x   at :%d \n",data,offset,machine().primary_screen->vpos() );


	switch (offset&0x01)
	{
	// Set the control register in the Video ULA
	case 0:
		{
		m_videoULA_Reg=data;
		m_videoULA_master_cursor_size=    (m_videoULA_Reg>>7)&0x01;
		m_videoULA_width_of_cursor=       (m_videoULA_Reg>>5)&0x03;
		m_videoULA_6845_clock_rate=       (m_videoULA_Reg>>4)&0x01;
		m_videoULA_characters_per_line=   (m_videoULA_Reg>>2)&0x03;
		m_videoULA_teletext_normal_select=(m_videoULA_Reg>>1)&0x01;
		m_videoULA_flash_colour_select=    m_videoULA_Reg    &0x01;

		m_videoULA_pallet_lookup=m_videoULA_flash_colour_select?m_videoULA_pallet0:m_videoULA_pallet1;

		m_emulation_cursor_size=width_of_cursor_set[m_videoULA_width_of_cursor|(m_videoULA_master_cursor_size<<2)];

		// this is the number of BBC pixels held in each byte
		if (m_videoULA_teletext_normal_select)
		{
			m_pixels_per_byte=6;
		} else {
			m_pixels_per_byte=pixels_per_byte_set[m_videoULA_characters_per_line|(m_videoULA_6845_clock_rate<<2)];
		}
		mc6845_device *mc6845 = machine().device<mc6845_device>("mc6845");
		mc6845->set_hpixels_per_column(m_pixels_per_byte);
		if (m_videoULA_6845_clock_rate)
			mc6845->set_clock(2000000);
		else
			mc6845->set_clock(1000000);
		}
		break;
	// Set a pallet register in the Video ULA
	case 1:
		int tpal=(data>>4)&0x0f;
		int tcol=data&0x0f;
		m_videoULA_pallet0[tpal]=tcol;
		m_videoULA_pallet1[tpal]=tcol<8?tcol:tcol^7;
		break;
	}
}

// VideoULA Internal Cursor controls

/*
static void set_cursor(bbc_state *state)
{
    state->m_cursor_state=state->m_VideoULA_CR?0:7;
}

static void BBC_Clock_CR(bbc_state *state)
{
    if (state->m_VideoULA_CR)
    {
        state->m_VideoULA_CR_counter-=1;
        if (state->m_VideoULA_CR_counter<=0) {
            state->m_VideoULA_CR=0;
            set_cursor(state);
        }
    }
}
*/
/************************************************************************
 * BBC circuits controlled by 6845 Outputs
 ************************************************************************/

int returned_pixel_count;
int returned_pixels[6];

static MC6845_UPDATE_ROW( vid_update_row )
{
	bbc_state *state = device->machine().driver_data<bbc_state>();
	const rgb_t *palette = palette_entry_list_raw(bitmap.palette());

	logerror("MC6845_UPDATE_ROW: ma=%d, ra=%d, y=%d, x_count=%d\n",ma,ra,y,x_count);

	if (state->m_videoULA_teletext_normal_select)
	{
		state->m_trom->lose_w(1);
		state->m_trom->lose_w(0);

		for(int x_pos=0; x_pos<x_count; x_pos++)
		{
			//Teletext Latch bits 0 to 5 go to bits 0 to 5 on the Teletext chip
			//Teletext Latch bit 6 is only passed onto bits 6 on the Teletext chip if DE is true
			//Teletext Latch bit 7 goes to LOSE on the Teletext chip

			returned_pixel_count=0;

			state->m_trom->write((state->m_Teletext_Latch&0x3f)|(state->m_Teletext_Latch&0x40));

			state->m_trom->f1_w(1);
			state->m_trom->f1_w(0);

			if (((ma>>13)&1)==0)
			{
				state->m_Teletext_Latch=0;
			} else {
				state->m_Teletext_Latch=(state->m_BBC_Video_RAM[calculate_video_address(state,ma+x_pos,ra)]);
			}
			for(int pixelno=0;pixelno<6;pixelno++)
			{
				state->m_trom->tr6_w(1);
				state->m_trom->tr6_w(0);

				int col=state->m_trom->get_rgb();

				int r = BIT(col, 0) * 0xff;
				int g = BIT(col, 1) * 0xff;
				int b = BIT(col, 2) * 0xff;

				rgb_t rgb = MAKE_RGB(r, g, b);

				bitmap.pix32(y, (x_pos*state->m_pixels_per_byte)+pixelno) = rgb;
			}
		}

		if (ra == 18)
		{
			state->m_trom->lose_w(1);
			state->m_trom->lose_w(0);
		}
	}
	else
	{
		// this is IC38 and IC41 takes 6845 DisplayEnabled and 6845 RA3
		int DE= !(ra>=8);

		if (DE)
		{
			for(int x_pos=0; x_pos<x_count; x_pos++)
			{
				int vmem=calculate_video_address(state,ma+x_pos,ra);
				unsigned char i=state->m_BBC_Video_RAM[vmem];

				for(int pixelno=0;pixelno<state->m_pixels_per_byte;pixelno++)
				{
					int col=state->m_videoULA_pallet_lookup[state->m_pixel_bits[i]];
					bitmap.pix32(y, (x_pos*state->m_pixels_per_byte)+pixelno)=palette[col];
					i=(i<<1)|1;
				}
			}
		}
		else
		{
			for(int x_pos=0; x_pos<x_count; x_pos++)
			{
				for(int pixelno=0;pixelno<state->m_pixels_per_byte;pixelno++)
				{
					bitmap.pix32(y, (x_pos*state->m_pixels_per_byte)+pixelno)=palette[7];
				}
			}
		}
	}
}

WRITE_LINE_MEMBER(bbc_state::bbc_vsync)
{
	m_trom->dew_w(state);
}


const mc6845_interface bbc_mc6845_intf =
{
	"screen",                       /* screen number */
	8,                              /* numbers of pixels per video memory address */
	NULL,                           /* begin_update */
	vid_update_row,                 /* update_row */
	NULL,                           /* end_update */
	DEVCB_NULL,                     /* on_de_changed */
	DEVCB_NULL,                     /* on_cur_changed */
	DEVCB_NULL,                     /* on_hsync_changed */
	DEVCB_DRIVER_LINE_MEMBER(bbc_state,bbc_vsync),          /* on_vsync_changed */
	NULL
};




/************************************************************************
 * memory interface to BBC's 6845
 ************************************************************************/

WRITE8_MEMBER(bbc_state::bbc_6845_w)
{
	mc6845_device *mc6845 = machine().device<mc6845_device>("mc6845");
	switch(offset & 1)
	{
		case 0 :
			mc6845->address_w(space,0,data);
			break;
		case 1 :
			mc6845->register_w(space,0,data);
			break;
	}
	return;
}

READ8_MEMBER(bbc_state::bbc_6845_r)
{
	mc6845_device *mc6845 = machine().device<mc6845_device>("mc6845");

	switch (offset&1)
	{
		case 0: return mc6845->status_r(space,0);
		case 1: return mc6845->register_r(space,0);
	}
	return 0;
}





/**** BBC B+ Shadow Ram change ****/

void bbcbp_setvideoshadow(running_machine &machine, int vdusel)
{
	bbc_state *state = machine.driver_data<bbc_state>();
	if (vdusel)
	{
		state->m_BBC_Video_RAM= state->memregion("maincpu")->base()+0x8000;
	} else {
		state->m_BBC_Video_RAM= machine.root_device().memregion("maincpu")->base();
	}
}

/************************************************************************
 * bbc_vh_start
 * Initialize the BBC video emulation
 ************************************************************************/

static void common_init(running_machine &machine, int memorySize)
{
	bbc_state *state = machine.driver_data<bbc_state>();
	state->m_emulation_cursor_size = 1;

	state->m_VideoULA_CR = 7;
	state->m_VideoULA_CR_counter = 0;

	set_pixel_lookup(state);

	state->m_BBC_Video_RAM = state->memregion("maincpu")->base();
	state->m_memorySize=memorySize;

}

VIDEO_START_MEMBER(bbc_state,bbca)
{
	common_init(machine(),16);
}

VIDEO_START_MEMBER(bbc_state,bbcb)
{
	common_init(machine(),32);
}

VIDEO_START_MEMBER(bbc_state,bbcbp)
{
	common_init(machine(),32);
}

VIDEO_START_MEMBER(bbc_state,bbcm)
{
	common_init(machine(),32);
}


/*

Old removed BBC mc6845 video code.
This has now all been replaced by MAMEs mc6845 code.



static void BBC_draw_hi_res(running_machine &machine);
static void BBC_draw_teletext(running_machine &machine);




static void BBC_draw_teletext(running_machine &machine)
{
    bbc_state *state = machine.driver_data<bbc_state>();

    //Teletext Latch bits 0 to 5 go to bits 0 to 5 on the Teletext chip
    //Teletext Latch bit 6 is only passed onto bits 6 on the Teletext chip if DE is true
    //Teletext Latch bit 7 goes to LOSE on the Teletext chip

    teletext_LOSE_w(state->m_saa505x, 0, (state->m_Teletext_Latch>>7)&1);

    teletext_F1(state->m_saa505x);

    teletext_data_w(state->m_saa505x, 0, (state->m_Teletext_Latch&0x3f)|((state->m_Teletext_Latch&0x40)|(m6845_display_enabled_r(0)?0:0x40)));

    int meml=m6845_memory_address_r(0);

    if (((meml>>13)&1)==0)
    {
        state->m_Teletext_Latch=0;
    } else {
        state->m_Teletext_Latch=(state->m_BBC_Video_RAM[calculate_video_address(state,meml)]&0x7f)|(m6845_display_enabled_r(0)?0x80:0);
    }

}





// This is the actual output of the Video ULA this fuction does all the output to the screen in the BBC emulator

static void BBC_ula_drawpixel(bbc_state *state, int col, int number_of_pixels)
{
    int pixel_count;
    int pixel_temp;
    if ((state->m_BBC_display>=state->m_BBC_display_left) && ((state->m_BBC_display+number_of_pixels)<state->m_BBC_display_right))
    {

        pixel_temp=col^state->m_cursor_state;
        for(pixel_count=0;pixel_count<number_of_pixels;pixel_count++)
        {
            *(state->m_BBC_display++) = pixel_temp;
        }
    } else {
        state->m_BBC_display += number_of_pixels;
    }
}


// the Video ULA hi-res shift registers, pallette lookup and display enabled circuits





static const struct m6845_interface BBC6845 =
{
    0,// Memory Address register
    0,// Row Address register
    BBC_Set_HSync,// Horizontal status
    BBC_Set_VSync,// Vertical status
    0,// Display Enabled status
    0,// Cursor status
    BBC_Set_CRE, // Cursor status Emulation
};



static void BBC_draw_hi_res(running_machine &machine)
{
    bbc_state *state = machine.driver_data<bbc_state>();
    int meml;
    unsigned char i=0;
    int sc1;

    // this is IC38 and IC41 takes 6845 DisplayEnabled and 6845 RA3
    int DE=m6845_display_enabled_r(0) && (!(m6845_row_address_r(0)&8));

    if (DE)
    {
        // read the memory location for the next screen location.
        meml=calculate_video_address(state,m6845_memory_address_r(0));

        i=state->m_BBC_Video_RAM[meml];

        for(sc1=0;sc1<state->m_pixels_per_byte;sc1++)
        {
            BBC_ula_drawpixel(state, state->m_videoULA_pallet_lookup[state->m_pixel_bits[i]], state->m_emulation_pixels_per_real_pixel);
            i=(i<<1)|1;
        }

    } else {
        // if the display is not enable, just draw a blank area.
        BBC_ula_drawpixel(state, 0, state->m_emulation_pixels_per_byte);
    }
}


// RGB input to the Video ULA from the Teletext IC
// Just pass on the output at the correct pixel size.
void bbc_draw_RGB_in(device_t *device, int offset,int data)
{
    bbc_state *state = device->machine().driver_data<bbc_state>();
    BBC_ula_drawpixel(state, data, state->m_emulation_pixels_per_real_pixel);
}






// called when the 6845 changes the HSync
static void BBC_Set_HSync(running_machine &machine, int offset, int data)
{
    bbc_state *state = machine.driver_data<bbc_state>();
    // catch the falling edge
    if((!data)&&(state->m_BBC_HSync))
    {
        state->m_y_screen_pos+=1;

        if ((state->m_y_screen_pos>=0) && (state->m_y_screen_pos<300))
        {
            state->m_BBC_display_left = &state->m_BBC_bitmap->pix16(state->m_y_screen_pos);
            state->m_BBC_display_right = state->m_BBC_display_left + 800;

        } else {
            state->m_BBC_display_left = &state->m_BBC_bitmap->pix16(0);
            state->m_BBC_display_right = state->m_BBC_display_left;
        }

        state->m_BBC_display = state->m_BBC_display_left + state->m_x_screen_offset;

    }
    state->m_BBC_HSync=data;
}

// called when the 6845 changes the VSync
static void BBC_Set_VSync(running_machine &machine, int offset, int data)
{
    bbc_state *state = machine.driver_data<bbc_state>();
    // catch the falling edge
    if ((!data)&&(state->m_BBC_VSync))
    {
        state->m_y_screen_pos=state->m_y_screen_offset;

        if ((state->m_y_screen_pos>=0) && (state->m_y_screen_pos<300))
        {
            state->m_BBC_display_left = &state->m_BBC_bitmap->pix16(state->m_y_screen_pos);
            state->m_BBC_display_right = state->m_BBC_display_left + 800;

        } else {
            state->m_BBC_display_left = &state->m_BBC_bitmap->pix16(0);
            state->m_BBC_display_right = state->m_BBC_display_left;
        }

        state->m_BBC_display = state->m_BBC_display_left + state->m_x_screen_offset;

        teletext_DEW(state->m_saa505x);
    }
    state->m_BBC_VSync=data;

}

// called when the 6845 changes the Cursor Enabled
static void BBC_Set_CRE(running_machine &machine, int offset, int data)
{
    bbc_state *state = machine.driver_data<bbc_state>();
    if (data&2) {
        state->m_VideoULA_CR_counter=state->m_emulation_cursor_size;
        state->m_VideoULA_CR=1;
        // set the pallet on
        if (data&1) set_cursor(state);
    }
}





WRITE8_MEMBER(bbc_state::bbc_6845_w)
{
    mc6845_device *mc6845 = machine().device<mc6845_device>("mc6845");
    switch(offset & 1)
    {
        case 0 :
            mc6845->address_w(space,0,data);
            break;
        case 1 :
            mc6845->register_w(space,0,data);
            break;
    }
    return;

    switch (offset&1)
    {
        case 0:
            m6845_address_w(0,data);
            break;
        case 1:
            m6845_register_w(0,data);
            break;
    }

}

 READ8_HANDLER (bbc_6845_r)
{

    mc6845_device *mc6845 = space.machine().device<mc6845_device>("mc6845");

    switch (offset&1)
    {
        case 0: return mc6845->status_r(space,0); break;
        case 1: return mc6845->register_r(space,0); break;
    }
    return 0;

    int retval=0;

    switch (offset&1)
    {
        case 0:
            break;
        case 1:
            retval=m6845_register_r(0);
            break;
    }
    return retval;
}





UINT32 bbc_state::screen_update_bbc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{

    mc6845_device *mc6845 = machine().device<mc6845_device>("mc6845");
    mc6845->update( bitmap, cliprect);

    return 0;



    long c;

    //logerror ("Box %d by %d \n",cliprect.min_y,cliprect.max_y);

    c = 0; // this is used to time out the screen redraw, in the case that the 6845 is in some way out state.


    m_BBC_bitmap=bitmap;

    m_BBC_display_left=&m_BBC_bitmap->pix16(0);
    m_BBC_display_right=m_BBC_display_left;
    m_BBC_display=m_BBC_display_left;

    // loop until the end of the Vertical Sync pulse
    // or until a timeout (this catches the 6845 with silly register values that would not give a VSYNC signal)
    while((m_BBC_VSync)&&(c<60000))
    {
        // Clock the 6845
        m6845_clock(machine());
        c++;
    }


    // loop until the Vertical Sync pulse goes high
    // or until a timeout (this catches the 6845 with silly register values that would not give a VSYNC signal)
    while((!m_BBC_VSync)&&(c<60000))
    {
        if ((m_y_screen_pos>=cliprect.min_y) && (m_y_screen_pos<=cliprect.max_y)) (m_draw_function)(machine());

        // and check the cursor
        if (m_VideoULA_CR) BBC_Clock_CR(this);

        // Clock the 6845
        m6845_clock(machine());
        c++;
    }

    return 0;
}

void bbc_frameclock(running_machine &machine)
{
    m6845_frameclock();
}

*/
