/*
    video/rmnimbus.c

    Research machines Nimbus.

    2009-11-14, P.Harvey-Smith.

    This is my best guess implementation of the operation of the Nimbus
    video system.

    On the real machine, the Video chip has a block of 64K of memory which is
    completely seperate from the main 80186 memory.

    The main CPU write to the video chip via a series of registers in the
    0x0000 to 0x002F reigon, the video chip then manages all video memory
    from there.

    As I cannot find a datasheet for the vide chip marked
    MB61H201 Fujitsu RML 12835 GCV, I have had to determine most of its
    operation by disassembling the Nimbus bios and by writing experemental
    code on the real machine.
*/

#include "emu.h"
#include "debug/debugcpu.h"
#include "debug/debugcon.h"
#include "includes/rmnimbus.h"


#define WIDTH_MASK      0x07
#define XOR_MASK        0x08
#define MASK_4080       0x10

// Offsets of nimbus video registers within register array

#define reg000          0x00
#define reg002          0x01
#define reg004          0x02
#define reg006          0x03
#define reg008          0x04
#define reg00A          0x05
#define reg00C          0x06
#define reg00E          0x07
#define reg010          0x08
#define reg012          0x09
#define reg014          0x0A
#define reg016          0x0B
#define reg018          0x0C
#define reg01A          0x0D
#define reg01C          0x0E
#define reg01E          0x0F
#define reg020          0x10
#define reg022          0x11
#define reg024          0x12
#define reg026          0x13
#define reg028          0x14
#define reg02A          0x15
#define reg02C          0x16
#define reg02E          0x17

#define FG_COLOUR       (state->m_vidregs[reg024]&0x0F)
#define BG_COLOUR       ((state->m_vidregs[reg024]&0xF0)>>4)

#define IS_80COL        (state->m_vidregs[reg026]&MASK_4080)
#define IS_XOR          (state->m_vidregs[reg022]&XOR_MASK)




#define DEBUG_TEXT  0x01
#define DEBUG_DB    0x02
#define DEBUG_PIXEL 0x04

#define DEBUG_SET(flags)    ((state->m_debug_video & (flags))==(flags))

static UINT8 get_pixel(rmnimbus_state *state, UINT16 x, UINT16 y);
static UINT16 read_pixel_line(rmnimbus_state *state, UINT16 x, UINT16 y, UINT8 width);
static UINT16 read_pixel_data(rmnimbus_state *state, UINT16 x, UINT16 y);
static UINT16 read_reg_00A(rmnimbus_state *state);

static void set_pixel(rmnimbus_state *state, UINT16 x, UINT16 y, UINT8 colour);
static void write_pixel_line(rmnimbus_state *state, UINT16 x, UINT16 y, UINT16    data, UINT8 width);
static void write_pixel_data(rmnimbus_state *state, UINT16 x, UINT16 y, UINT16    data);
static void move_pixel_line(rmnimbus_state *state, UINT16 x, UINT16 y, UINT16    data, UINT8 width);
static void write_reg_004(rmnimbus_state *state);
static void write_reg_006(rmnimbus_state *state);
static void write_reg_010(rmnimbus_state *state);
static void write_reg_012(rmnimbus_state *state);
static void write_reg_014(rmnimbus_state *state);
static void write_reg_016(rmnimbus_state *state);
static void write_reg_01A(rmnimbus_state *state);
static void write_reg_01C(rmnimbus_state *state);
static void write_reg_01E(rmnimbus_state *state);
static void write_reg_026(rmnimbus_state *state);
static void change_palette(running_machine &machine, UINT8 bank, UINT16 colours, UINT8 regno);

static void video_debug(running_machine &machine, int ref, int params, const char *param[]);
static void video_regdump(running_machine &machine, int ref, int params, const char *param[]);

/*
    I'm not sure which of thes return values on a real machine, so for the time being I'm going
    to return the values for all of them, it doesn't seem to hurt !
*/

READ16_MEMBER(rmnimbus_state::nimbus_video_io_r)
{
	rmnimbus_state *state = machine().driver_data<rmnimbus_state>();
    int     pc=cpu_get_pc(&space.device());
    UINT16  result;

    switch (offset)
    {
        case    reg000  : result=m_vidregs[reg000]; break;
        case    reg002  : result=m_vidregs[reg002]; break;
        case    reg004  : result=m_vidregs[reg004]; break;
        case    reg006  : result=m_vidregs[reg006]; break;
        case    reg008  : result=m_vidregs[reg008]; break;
        case    reg00A  : result=read_reg_00A(state); break;
        case    reg00C  : result=m_vidregs[reg00C]; break;
        case    reg00E  : result=m_vidregs[reg00E]; break;

        case    reg010  : result=m_vidregs[reg010]; break;
        case    reg012  : result=m_vidregs[reg012]; break;
        case    reg014  : result=m_vidregs[reg014]; break;
        case    reg016  : result=m_vidregs[reg016]; break;
        case    reg018  : result=m_vidregs[reg018]; break;
        case    reg01A  : result=m_vidregs[reg01A]; break;
        case    reg01C  : result=m_vidregs[reg01C]; break;
        case    reg01E  : result=m_vidregs[reg01E]; break;

        case    reg020  : result=m_vidregs[reg020]; break;
        case    reg022  : result=m_vidregs[reg022]; break;
        case    reg024  : result=m_vidregs[reg024]; break;
        case    reg026  : result=m_vidregs[reg026]; break;
        case    reg028  : result=m_hs_count; break; //result=m_vidregs[reg028]; break;
        case    reg02A  : result=m_vidregs[reg02A]; break;
        case    reg02C  : result=m_vidregs[reg02C]; break;
        case    reg02E  : result=m_vidregs[reg02E]; break;
        default         : result=0; break;
    }

    if(DEBUG_SET(DEBUG_TEXT))
        logerror("Nimbus video IOR at %05X from %04X mask=%04X, data=%04X\n",pc,(offset*2),mem_mask,result);

    return result;
}

static UINT8 get_pixel(rmnimbus_state *state, UINT16 x, UINT16 y)
{
    UINT8   result = 0;

    if((x<SCREEN_WIDTH_PIXELS) && (y<SCREEN_HEIGHT_LINES))
    {
        if(IS_80COL)
            result=state->m_video_mem[x][y];
        else
            result=state->m_video_mem[x*2][y];
    }

    return result;
}

static UINT16 read_pixel_line(rmnimbus_state *state, UINT16 x, UINT16 y, UINT8 width)
{
    UINT16  result = 0;
    UINT16  mask;
    UINT16  pixel_x;
    UINT16  colour;
    UINT8   shifts;

    if(DEBUG_SET(DEBUG_TEXT | DEBUG_PIXEL))
        logerror("read_pixel_line(x=%04X, y=%04X, width=%02X, bpp=%02X, pixel_mask=%02X)\n",x,y,width,state->m_bpp,state->m_pixel_mask);

    shifts=width-state->m_bpp;

    for(mask=state->m_pixel_mask, pixel_x=(x*(width/state->m_bpp)); mask>0; mask=(mask>>state->m_bpp), pixel_x++)
    {
        colour=get_pixel(state,pixel_x,y);

        if(state->m_bpp==1)
            colour=((colour==FG_COLOUR) ? 1 : 0) << shifts;
        else
            colour=colour << shifts;

        result=(result & ~mask)  | colour;

        shifts-=state->m_bpp;
    }

    return result;
}

static UINT16 read_pixel_data(rmnimbus_state *state, UINT16 x, UINT16 y)
{
    UINT16  result=0;

    if(DEBUG_SET(DEBUG_TEXT | DEBUG_PIXEL))
        logerror("read_pixel_data(x=%04X, y=%04X), reg022=%04X\n",x,y,state->m_vidregs[reg022]);

    if(IS_80COL)
    {
        switch (state->m_vidregs[reg022] & WIDTH_MASK)
        {
            case 0x00   : break;

            case 0x01   : break;

            case 0x02   : break;

            case 0x03   : break;

            case 0x04   : break;

            case 0x05   : break;

            case 0x06   : state->m_bpp=2; state->m_pixel_mask=0xC000;
                          result=read_pixel_line(state,x,y,16);
                          break;

            case 0x07   : break;
        }
    }
    else /* 40 Col */
    {
        switch (state->m_vidregs[reg022] & WIDTH_MASK)
        {
            case 0x00   : break;

            case 0x01   : break;

            case 0x02   : break;

            case 0x03   : break;

            case 0x04   : break;

            case 0x05   : break;

            case 0x06   : state->m_bpp=4; state->m_pixel_mask=0xF000;
                          result=read_pixel_line(state,x,y,16);
                          break;

            case 0x07   : break;
        }
    }

    return result;
}

static UINT16 read_reg_00A(rmnimbus_state *state)
{
    return read_pixel_data(state, ++state->m_vidregs[reg002],state->m_vidregs[reg00C]);
}


/*
    Write to the video registers, the default action is to write to the array of registers.
    If a register also needs some special action call the action function for that register.

    Incase anyone wonders about the DEBUG_DB statement, this allows me to log which registers
    are being written to and then play them back at the real machine, this has helped greatly
    in figuring out what the video registers do.

*/

WRITE16_MEMBER(rmnimbus_state::nimbus_video_io_w)
{
    rmnimbus_state *state = machine().driver_data<rmnimbus_state>();
	int pc=cpu_get_pc(&space.device());

    if(offset<reg028)
    {
        if(DEBUG_SET(DEBUG_TEXT))
            logerror("Nimbus video IOW at %05X write of %04X to %04X mask=%04X\n",pc,data,(offset*2),mem_mask);

        if(DEBUG_SET(DEBUG_DB))
            logerror("dw %05X,%05X\n",(offset*2),data);
    }

    switch (offset)
    {
        case    reg000  : m_vidregs[reg000]=data; break;
        case    reg002  : m_vidregs[reg002]=data; break;
        case    reg004  : m_vidregs[reg004]=data; write_reg_004(state); break;
        case    reg006  : m_vidregs[reg006]=data; write_reg_006(state); break;
        case    reg008  : m_vidregs[reg008]=data; break;
        case    reg00A  : m_vidregs[reg00A]=data; break;
        case    reg00C  : m_vidregs[reg00C]=data; break;
        case    reg00E  : m_vidregs[reg00E]=data; break;

        case    reg010  : m_vidregs[reg010]=data; write_reg_010(state); break;
        case    reg012  : m_vidregs[reg012]=data; write_reg_012(state); break;
        case    reg014  : m_vidregs[reg014]=data; write_reg_014(state); break;
        case    reg016  : m_vidregs[reg016]=data; write_reg_016(state); break;
        case    reg018  : m_vidregs[reg018]=data; break;
        case    reg01A  : m_vidregs[reg01A]=data; write_reg_01A(state); break;
        case    reg01C  : m_vidregs[reg01C]=data; write_reg_01C(state);break;
        case    reg01E  : m_vidregs[reg01E]=data; write_reg_01E(state);break;

        case    reg020  : m_vidregs[reg020]=data; break;
        case    reg022  : m_vidregs[reg022]=data; break;
        case    reg024  : m_vidregs[reg024]=data; break;
        case    reg026  : m_vidregs[reg026]=data; write_reg_026(state); break;
        case    reg028  : change_palette(machine(),0,data,reg028); break;
        case    reg02A  : change_palette(machine(),1,data,reg02A); break;
        case    reg02C  : change_palette(machine(),2,data,reg02C); break;
        case    reg02E  : change_palette(machine(),3,data,reg02E); break;

        default         : break;
    }
}

static void set_pixel(rmnimbus_state *state, UINT16 x, UINT16 y, UINT8 colour)
{
    if(DEBUG_SET(DEBUG_TEXT | DEBUG_PIXEL))
        logerror("set_pixel(x=%04X, y=%04X, colour=%04X), IS_XOR=%02X\n",x,y,colour,IS_XOR);

    if(IS_80COL)
        colour&=0x03;

    if((x<SCREEN_WIDTH_PIXELS) && (y<SCREEN_HEIGHT_LINES))
    {
        if(IS_XOR)
            state->m_video_mem[x][y]^=colour;
        else
            state->m_video_mem[x][y]=colour;
    }
}

static void set_pixel40(rmnimbus_state *state, UINT16 x, UINT16 y, UINT8 colour)
{
    set_pixel(state, (x*2),y,colour);
    set_pixel(state, (x*2)+1,y,colour);
}

static void write_pixel_line(rmnimbus_state *state, UINT16 x, UINT16 y, UINT16    data, UINT8 width)
{
    UINT16  mask;
    UINT16  pixel_x;
    UINT16  colour;
    UINT8   shifts;

    if(DEBUG_SET(DEBUG_TEXT | DEBUG_PIXEL))
        logerror("write_pixel_line(x=%04X, y=%04X, data=%04X, width=%02X, bpp=%02X, pixel_mask=%02X)\n",x,y,data,width,state->m_bpp,state->m_pixel_mask);

    shifts=width-state->m_bpp;

    for(mask=state->m_pixel_mask, pixel_x=(x*(width/state->m_bpp)); mask>0; mask=(mask>>state->m_bpp), pixel_x++)
    {
        if(state->m_bpp==1)
            colour=(data & mask) ? FG_COLOUR : BG_COLOUR;
        else
            colour=(data & mask) >> shifts;

        //logerror("write_pixel_line: data=%04X, mask=%04X, shifts=%02X, bpp=%02X colour=%02X\n",data,mask,shifts,state->m_bpp,colour);

        if(IS_80COL)
            set_pixel(state,pixel_x,y,colour);
        else
            set_pixel40(state,pixel_x,y,colour);

        shifts-=state->m_bpp;
    }
}

static void move_pixel_line(rmnimbus_state *state, UINT16 x, UINT16 y, UINT16    data, UINT8 width)
{
    UINT16  pixelno;
    UINT16  pixelx;

    if(DEBUG_SET(DEBUG_TEXT | DEBUG_PIXEL))
       logerror("move_pixel_line(x=%04X, y=%04X, data=%04X, width=%02X)\n",x,y,data,width);

    for(pixelno=0;pixelno<width;pixelno++)
    {
        pixelx=(x*width)+pixelno;
        if(DEBUG_SET(DEBUG_TEXT | DEBUG_PIXEL))
            logerror("pixelx=%04X\n",pixelx);
        state->m_video_mem[pixelx][state->m_vidregs[reg020]]=state->m_video_mem[pixelx][y];
    }
}



/*
    The values in the bottom 3 bits of reg022 seem to determine the number of bits per pixel
    for following operations.

    The values that I have decoded so far are :

    000 1bpp, foreground and background colours taken from reg024
    001 2bpp, using the first 4 colours of the pallette
    010
    011
    100 4bpp, must be a 16 bit word, of which the upper byte is a mask anded with the lower byte
              containing the pixel data for two pixels.
    101 Move pixel data at x,reg020 to x,y, used for scrolling.
    110 if 40 col
            4bpp, 16 bit word containing the pixel data for 4 pixels.
        else
            2bpp, 16 bit word containing the pixel data for 8 pixels.
    111

    Bit 3 of reg022 is as follows :

    0   pixels are written from supplied colour data
    1   pixels are xor'ed onto the screen
*/

static void write_pixel_data(rmnimbus_state *state, UINT16 x, UINT16 y, UINT16    data)
{
    if(DEBUG_SET(DEBUG_TEXT | DEBUG_PIXEL))
        logerror("write_pixel_data(x=%04X, y=%04X, data=%04X), reg022=%04X\n",x,y,data,state->m_vidregs[reg022]);

    if(IS_80COL)
    {
        switch (state->m_vidregs[reg022] & WIDTH_MASK)
        {
            case 0x00   : state->m_bpp=1; state->m_pixel_mask=0x8000;
                          write_pixel_line(state,x,y,data,16);
                          break;

            case 0x01   : state->m_bpp=1; state->m_pixel_mask=0x80;
                          write_pixel_line(state,x,y,data,8);
                          break;

            case 0x02   : state->m_bpp=1; state->m_pixel_mask=0x0080;
                          write_pixel_line(state,x,y,data,8);
                          break;

            case 0x03   : state->m_bpp=1;
                          set_pixel(state,x,y,FG_COLOUR);
                          break;

            case 0x04   : state->m_bpp=2; state->m_pixel_mask=0xC0;
                          write_pixel_line(state,x,y,((data & 0xFF) & ((data & 0xFF00)>>8)),8);
                          break;

            case 0x05   : move_pixel_line(state,x,y,data,16);
                          break;

            case 0x06   : state->m_bpp=2; state->m_pixel_mask=0xC000;
                          write_pixel_line(state,x,y,data,16);
                          break;

            case 0x07   : state->m_bpp=1;
                          set_pixel(state,x,y,FG_COLOUR);
                          break;
        }
    }
    else /* 40 Col */
    {
        switch (state->m_vidregs[reg022] & WIDTH_MASK)
        {
            case 0x00   : state->m_bpp=1; state->m_pixel_mask=0x0080;
                          write_pixel_line(state,x,y,data,8);
                          break;

            case 0x01   : state->m_bpp=2; state->m_pixel_mask=0xC0;
                          write_pixel_line(state,x,y,data,8);
                          break;

            case 0x02   : state->m_bpp=1; state->m_pixel_mask=0x0080;
                          set_pixel40(state,x,y,FG_COLOUR);
                          break;

            case 0x03   : state->m_bpp=1;
                          set_pixel(state,x,y,FG_COLOUR);
                          break;

            case 0x04   : state->m_bpp=4; state->m_pixel_mask=0xF0;
                          write_pixel_line(state,x,y,((data & 0xFF) & ((data & 0xFF00)>>8)),8);
                          break;

            case 0x05   : move_pixel_line(state,x,y,data,16);
                          break;

            case 0x06   : state->m_bpp=4; state->m_pixel_mask=0xF000;
                          write_pixel_line(state,x,y,data,16);
                          break;

            case 0x07   : state->m_bpp=1;
                          set_pixel(state,x,y,FG_COLOUR);
                          break;
        }
    }
}

static void write_reg_004(rmnimbus_state *state)
{
    state->m_vidregs[reg002]=0;
    state->m_vidregs[reg00C]++;
}

static void write_reg_006(rmnimbus_state *state)
{
    state->m_vidregs[reg00C]++;
    state->m_vidregs[reg002]=state->m_vidregs[reg006];
}

static void write_reg_010(rmnimbus_state *state)
{
    write_pixel_data(state, state->m_vidregs[reg002],state->m_vidregs[reg00C],state->m_vidregs[reg010]);
}

static void write_reg_012(rmnimbus_state *state)
{
    // I dunno if this is actually what is happening as the regs seem to be write only....
    // doing this however does seem to make some programs (worms from the welcom disk)
    // work correctly.
    state->m_vidregs[reg002]=state->m_vidregs[reg012];

    write_pixel_data(state, state->m_vidregs[reg012],state->m_vidregs[reg00C],FG_COLOUR);
}

static void write_reg_014(rmnimbus_state *state)
{
    write_pixel_data(state, state->m_vidregs[reg002],state->m_vidregs[reg00C]++,state->m_vidregs[reg014]);
}

static void write_reg_016(rmnimbus_state *state)
{
	state->m_vidregs[reg002]=state->m_vidregs[reg016];

    write_pixel_data(state, state->m_vidregs[reg002],state->m_vidregs[reg00C]++,FG_COLOUR);
}


static void write_reg_01A(rmnimbus_state *state)
{
    write_pixel_data(state, ++state->m_vidregs[reg002],state->m_vidregs[reg00C],state->m_vidregs[reg01A]);
}

static void write_reg_01C(rmnimbus_state *state)
{
    // I dunno if this is actually what is happening as the regs seem to be write only....
    // doing this however does seem to make some programs (welcome from the welcom disk,
    // and others using the standard RM box menus) work correctly.
    state->m_vidregs[reg00C]=state->m_vidregs[reg01C];

    write_pixel_data(state, state->m_vidregs[reg002],state->m_vidregs[reg01C],FG_COLOUR);
}

static void write_reg_01E(rmnimbus_state *state)
{
	state->m_vidregs[reg00C]=state->m_vidregs[reg01E];

    write_pixel_data(state, ++state->m_vidregs[reg002],state->m_vidregs[reg00C],FG_COLOUR);
}

/*
    bits 0..3 of reg026 contain the border colour.
    bit 5 contains the 40/80 column (320/640 pixel) flag.
*/

static void write_reg_026(rmnimbus_state *state)
{
    if(DEBUG_SET(DEBUG_TEXT))
        logerror("reg 026 write, border_colour=%02X\n",state->m_vidregs[reg026] & 0x0F);
}

static void change_palette(running_machine &machine, UINT8 bank, UINT16 colours, UINT8 regno)
{
	rmnimbus_state *state = machine.driver_data<rmnimbus_state>();
    UINT8   colourno;
    UINT16  mask;
    UINT8   shifts;
    UINT8   paletteidx;
    UINT8   colourmax;
    UINT8   first;

    // for the register's data has changed update it, and then update the pallette, else do nothing.
    if(state->m_vidregs[regno]!=colours)
        state->m_vidregs[regno]=colours;
    else
        return;

    // Setup parameters for pallette change
    colourmax=IS_80COL ? 1 : 4;
    first=IS_80COL ? bank : bank*4;

    shifts=0;
    mask=0x000F;

    // loop over changing colours
    for(colourno=first; colourno<(first+colourmax); colourno++)
    {
        paletteidx=(colours & mask) >> shifts;
        palette_set_color_rgb(machine, colourno, nimbus_palette[paletteidx][RED], nimbus_palette[paletteidx][GREEN], nimbus_palette[paletteidx][BLUE]);

        if(DEBUG_SET(DEBUG_TEXT))
            logerror("set colourno[%02X](r,g,b)=(%02X,%02X,%02X), paletteidx=%02X\n",colourno, nimbus_palette[paletteidx][RED], nimbus_palette[paletteidx][GREEN], nimbus_palette[paletteidx][BLUE],paletteidx);
        mask=mask<<4;
        shifts+=4;
    }
}

static void video_debug(running_machine &machine, int ref, int params, const char *param[])
{
	rmnimbus_state *state = machine.driver_data<rmnimbus_state>();
    if(params>0)
    {
        sscanf(param[0],"%d",&state->m_debug_video);
    }
    else
    {
        debug_console_printf(machine,"Error usage : nimbus_vid_debug <debuglevel>\n");
        debug_console_printf(machine,"Current debuglevel=%02X\n",state->m_debug_video);
    }
}

static void video_regdump(running_machine &machine, int ref, int params, const char *param[])
{
	rmnimbus_state *state = machine.driver_data<rmnimbus_state>();
    int regno;

    for(regno=0;regno<0x08;regno++)
    {
        debug_console_printf(machine,"reg%03X=%04X reg%03X=%04X reg%03X=%04X\n",
                regno*2,state->m_vidregs[regno],
                (regno+0x08)*2,state->m_vidregs[regno+0x08],
                (regno+0x10)*2,state->m_vidregs[regno+0x10]);

        logerror("reg%03X=%04X reg%03X=%04X reg%03X=%04X\n",
                regno*2,state->m_vidregs[regno],
                (regno+0x08)*2,state->m_vidregs[regno+0x08],
                (regno+0x10)*2,state->m_vidregs[regno+0x10]);
    }
}

VIDEO_START( nimbus )
{
	rmnimbus_state *state = machine.driver_data<rmnimbus_state>();
    state->m_debug_video=0;

    logerror("VIDEO_START\n");

	if (machine.debug_flags & DEBUG_FLAG_ENABLED)
	{
        debug_console_register_command(machine, "nimbus_vid_debug", CMDFLAG_NONE, 0, 0, 1, video_debug);
        debug_console_register_command(machine, "nimbus_vid_regdump", CMDFLAG_NONE, 0, 0, 1, video_regdump);
    }
}

VIDEO_RESET( nimbus )
{
	rmnimbus_state *state = machine.driver_data<rmnimbus_state>();
    // When we reset clear the video registers and video memory.
    memset(&state->m_vidregs,0x00,sizeof(state->m_vidregs));
    memset(&state->m_video_mem,0,sizeof(state->m_video_mem));

    state->m_bpp=4;          // bits per pixel
    logerror("Video reset\n");
}

SCREEN_VBLANK( nimbus )
{

//    logerror("SCREEN_VBLANK( nimbus )\n");
}

SCREEN_UPDATE_IND16( nimbus )
{
	rmnimbus_state *state = screen.machine().driver_data<rmnimbus_state>();
    int     XCoord;
    int     YCoord = screen.vpos();

    for(XCoord=0;XCoord<SCREEN_WIDTH_PIXELS;XCoord++)
    {
        bitmap.pix16(YCoord, XCoord)=state->m_video_mem[XCoord][YCoord];
    }

    state->m_hs_count++;
    if((state->m_hs_count & 0x000F)>0x0A)
        state->m_hs_count&=0xFFF0;

    return 0;
}

