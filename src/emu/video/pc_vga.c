/***************************************************************************

    Video Graphics Adapter (VGA) section

    Nathan Woods    npwoods@mess.org
    Peter Trauner   PeT mess@utanet.at

    This code takes care of installing the various VGA memory and port
    handlers

    The VGA standard is compatible with MDA, CGA, Hercules, EGA
    (mda, cga, hercules not real register compatible)
    several vga cards drive also mda, cga, ega monitors
    some vga cards have register compatible mda, cga, hercules modes

    ega/vga
    64k (early ega 16k) words of 32 bit memory


    ROM declarations:

    (oti 037 chip)
    ROM_LOAD("oakvga.bin", 0xc0000, 0x8000, 0x318c5f43)
    (tseng labs famous et4000 isa vga card (oem))
    ROM_LOAD("et4000b.bin", 0xc0000, 0x8000, 0xa903540d)
    (tseng labs famous et4000 isa vga card)
    ROM_LOAD("et4000.bin", 0xc0000, 0x8000, 0xf01e4be0)

***************************************************************************/

#include "emu.h"
#include "pc_vga.h"

/***************************************************************************

    Local variables

***************************************************************************/

static pc_video_update_proc (*pc_choosevideomode)(running_machine &machine, int *width, int *height);

static int pc_current_height;
static int pc_current_width;
/***************************************************************************

    Static declarations

***************************************************************************/

#define LOG_ACCESSES	0
#define LOG_REGISTERS	0

static PALETTE_INIT( vga );
static VIDEO_START( vga );
static VIDEO_RESET( vga );

static pc_video_update_proc pc_vga_choosevideomode(running_machine &machine, int *width, int *height);

/***************************************************************************

    MachineDriver stuff

***************************************************************************/

void pc_video_start(running_machine &machine, pc_video_update_proc (*choosevideomode)(running_machine &machine, int *width, int *height))
{
	pc_choosevideomode = choosevideomode;
	pc_current_height = -1;
	pc_current_width = -1;
}



SCREEN_UPDATE( pc_video )
{
	UINT32 rc = 0;
	int w = 0, h = 0;
	pc_video_update_proc video_update = pc_choosevideomode(screen->machine(), &w, &h);

	if (video_update)
	{
		if ((pc_current_width != w) || (pc_current_height != h))
		{
			int width = screen->width();
			int height = screen->height();

			pc_current_width = w;
			pc_current_height = h;

			if (pc_current_width > width)
				pc_current_width = width;
			if (pc_current_height > height)
				pc_current_height = height;

			if ((pc_current_width > 100) && (pc_current_height > 100))
				screen->set_visible_area(0, pc_current_width-1, 0, pc_current_height-1);

			bitmap_fill(bitmap, cliprect, 0);
		}

		video_update(bitmap);
	}
	return rc;
}
/***************************************************************************/

static PALETTE_INIT( vga )
{
	int i;
	for (i = 0; i < 0x100; i++)
		palette_set_color_rgb(machine, i, 0, 0, 0);
}

static UINT8 color_bitplane_to_packed[4/*plane*/][8/*pixel*/][256];

static struct
{
	struct pc_vga_interface vga_intf;
	struct pc_svga_interface svga_intf;

	UINT8 *memory;
	UINT8 *fontdirty;
	UINT16 pens[16]; /* the current 16 pens */

	UINT8 miscellaneous_output;
	UINT8 feature_control;
	UINT16 line_compare;  // for split-screen use.

	struct
	{
		UINT8 index;
		UINT8 *data;
	} sequencer;
	struct
	{
		UINT8 index;
		UINT8 *data;
	} crtc;
	struct
	{
		UINT8 index;
		UINT8 *data;
		UINT8 latch[4];
	} gc;
	struct { UINT8 index, data[0x15]; int state; } attribute;


	struct {
		UINT8 read_index, write_index, mask;
		int read;
		int state;
		struct { UINT8 red, green, blue; } color[0x100];
		int dirty;
	} dac;

	struct {
		int time;
		int visible;
	} cursor;

	struct {
		int (*get_clock)(void);

		int (*get_lines)(void);
		int (*get_sync_lines)(void);

		int (*get_columns)(void);
		int (*get_sync_columns)(void);

		attotime start_time;
		int retrace;
	} monitor;

	/* oak vga */
	struct { UINT8 reg; } oak;

	int log;
} vga;


#define REG(x) vga.crtc.data[x]

#define CRTC_CHAR_HEIGHT ((REG(9)&0x1f)+1)
#define CRTC_CURSOR_MODE (REG(0xa)&0x60)
#define CRTC_CURSOR_OFF 0x20
#define CRTC_SKEW	(REG(8)&15)
#define CRTC_CURSOR_POS ((REG(0xe)<<8)|REG(0xf))
#define CRTC_CURSOR_TOP	(REG(0xa)&0x1f)
#define CRTC_CURSOR_BOTTOM REG(0xb)

#define DOUBLESCAN ((vga.crtc.data[9]&0x80)||((vga.crtc.data[9]&0x1f)!=0))
#define CRTC_PORT_ADDR ((vga.miscellaneous_output&1)?0x3d0:0x3b0)

#define CRTC_ON (vga.crtc.data[0x17]&0x80)

#define LINES_HELPER ( (vga.crtc.data[0x12] \
				|((vga.crtc.data[7]&2)<<7) \
				|((vga.crtc.data[7]&0x40)<<3))+1 )
//#define TEXT_LINES (LINES_HELPER)
#define LINES (DOUBLESCAN?LINES_HELPER>>1:LINES_HELPER)
#define TEXT_LINES (LINES_HELPER >> ((vga.crtc.data[9]&0x80) ? 1 : 0))

#define GRAPHIC_MODE (vga.gc.data[6]&1) /* else textmodus */

#define EGA_COLUMNS (vga.crtc.data[1]+1)
#define EGA_START_ADDRESS ((vga.crtc.data[0xd]|(vga.crtc.data[0xc]<<8))<<2)
#define EGA_LINE_LENGTH (vga.crtc.data[0x13]<<3)

#define VGA_COLUMNS (EGA_COLUMNS>>1)
#define VGA_START_ADDRESS (EGA_START_ADDRESS)
#define VGA_LINE_LENGTH (EGA_LINE_LENGTH<<2)

#define CHAR_WIDTH ((vga.sequencer.data[1]&1)?8:9)

#define TEXT_COLUMNS (vga.crtc.data[1]+1)
#define TEXT_START_ADDRESS (EGA_START_ADDRESS)
#define TEXT_LINE_LENGTH (EGA_LINE_LENGTH>>2)

#define TEXT_COPY_9COLUMN(ch) ((ch>=192)&&(ch<=223)&&(vga.attribute.data[0x10]&4))

#define FONT1 (  ((vga.sequencer.data[3]&0x3)    |((vga.sequencer.data[3]&0x10)<<2))*0x2000 )
#define FONT2 ( (((vga.sequencer.data[3]&0xc)>>2)|((vga.sequencer.data[3]&0x20)<<3))*0x2000 )


INLINE UINT8 rotate_right(UINT8 val, UINT8 rot)
{
	return (val >> rot) | (val << (8 - rot));
}



static int vga_get_clock(void)
{
	int clck=0;
	switch(vga.miscellaneous_output&0xc) {
	case 0: clck=25000000;break;
	case 4: clck=28000000;break;
	/* case 8: external */
	/* case 0xc: reserved */
	}
	if (vga.sequencer.data[1]&8) clck/=2;
	return clck;
}

static int vga_get_crtc_columns(void) /* in clocks! */
{
	int columns=vga.crtc.data[0]+5;

	if (!GRAPHIC_MODE)
		columns *= CHAR_WIDTH;
	else if (vga.gc.data[5]&0x40)
		columns *= 4;
	else
		columns *= 8;

	return columns;
}

static int vga_get_crtc_lines(void)
{
	int lines=(vga.crtc.data[6]
			   |((vga.crtc.data[7]&1)<<8)
			   |((vga.crtc.data[7]&0x20)<<(8-4)))+2;

	return lines;
}

static int vga_get_crtc_sync_lines(void)
{
	return 10;
}

static int vga_get_crtc_sync_columns(void)
{
	return 40;
}

INLINE WRITE8_HANDLER(vga_dirty_w)
{
	vga.memory[offset] = data;
}

INLINE WRITE8_HANDLER(vga_dirty_font_w)
{
	if (vga.memory[offset] != data)
	{
		vga.memory[offset] = data;
		if ((offset&3)==2)
			vga.fontdirty[offset>>7]=1;
	}
}

static READ8_HANDLER(vga_text_r)
{
	int data;
	data=vga.memory[((offset&~1)<<1)|(offset&1)];

	return data;
}

static WRITE8_HANDLER(vga_text_w)
{
	vga_dirty_w(space, ((offset&~1)<<1)|(offset&1),data);
}

INLINE UINT8 ega_bitplane_to_packed(UINT8 *latch, int number)
{
	return color_bitplane_to_packed[0][number][latch[0]]
		|color_bitplane_to_packed[1][number][latch[1]]
		|color_bitplane_to_packed[2][number][latch[2]]
		|color_bitplane_to_packed[3][number][latch[3]];
}

static READ8_HANDLER(vga_ega_r)
{
	int data;
	vga.gc.latch[0]=vga.memory[(offset<<2)];
	vga.gc.latch[1]=vga.memory[(offset<<2)+1];
	vga.gc.latch[2]=vga.memory[(offset<<2)+2];
	vga.gc.latch[3]=vga.memory[(offset<<2)+3];
	if (vga.gc.data[5]&8) {
		data=0;
		if (!(ega_bitplane_to_packed(vga.gc.latch, 0)^(vga.gc.data[2]&0xf&~vga.gc.data[7]))) data|=1;
		if (!(ega_bitplane_to_packed(vga.gc.latch, 1)^(vga.gc.data[2]&0xf&~vga.gc.data[7]))) data|=2;
		if (!(ega_bitplane_to_packed(vga.gc.latch, 2)^(vga.gc.data[2]&0xf&~vga.gc.data[7]))) data|=4;
		if (!(ega_bitplane_to_packed(vga.gc.latch, 3)^(vga.gc.data[2]&0xf&~vga.gc.data[7]))) data|=8;
		if (!(ega_bitplane_to_packed(vga.gc.latch, 4)^(vga.gc.data[2]&0xf&~vga.gc.data[7]))) data|=0x10;
		if (!(ega_bitplane_to_packed(vga.gc.latch, 5)^(vga.gc.data[2]&0xf&~vga.gc.data[7]))) data|=0x20;
		if (!(ega_bitplane_to_packed(vga.gc.latch, 6)^(vga.gc.data[2]&0xf&~vga.gc.data[7]))) data|=0x40;
		if (!(ega_bitplane_to_packed(vga.gc.latch, 7)^(vga.gc.data[2]&0xf&~vga.gc.data[7]))) data|=0x80;
	} else {
		data=vga.gc.latch[vga.gc.data[4]&3];
	}

	return data;
}

INLINE UINT8 vga_latch_helper(UINT8 cpu, UINT8 latch, UINT8 mask)
{
	switch (vga.gc.data[3] & 0x18)
	{
		case 0x00:
			return rotate_right((cpu&mask)|(latch&~mask), vga.gc.data[3] & 0x07);
		case 0x08:
			return rotate_right(((cpu&latch)&mask)|(latch&~mask), vga.gc.data[3] & 0x07);
		case 0x10:
			return rotate_right(((cpu|latch)&mask)|(latch&~mask), vga.gc.data[3] & 0x07);
		case 0x18:
			return rotate_right(((cpu^latch)&mask)|(latch&~mask), vga.gc.data[3] & 0x07);
	}
	return 0; /* must not be reached, suppress compiler warning */
}

INLINE UINT8 vga_latch_write(int offs, UINT8 data)
{
	switch (vga.gc.data[5]&3) {
	case 0:
		if (vga.gc.data[1]&(1<<offs)) {
			return vga_latch_helper( (vga.gc.data[0]&(1<<offs))?vga.gc.data[8]:0,
									  vga.gc.latch[offs],vga.gc.data[8] );
		} else {
			return vga_latch_helper(data, vga.gc.latch[offs], vga.gc.data[8]);
		}
		break;
	case 1:
		return vga.gc.latch[offs];
	case 2:
		if (data&(1<<offs)) {
			return vga_latch_helper(0xff, vga.gc.latch[offs], vga.gc.data[8]);
		} else {
			return vga_latch_helper(0, vga.gc.latch[offs], vga.gc.data[8]);
		}
		break;
	case 3:
		if (vga.gc.data[0]&(1<<offs)) {
			return vga_latch_helper(0xff, vga.gc.latch[offs], data&vga.gc.data[8]);
		} else {
			return vga_latch_helper(0, vga.gc.latch[offs], data&vga.gc.data[8]);
		}
		break;
	}
	return 0; /* must not be reached, suppress compiler warning */
}

static WRITE8_HANDLER(vga_ega_w)
{
	if (vga.sequencer.data[2]&1)
		vga_dirty_w(space, offset<<2, vga_latch_write(0,data));
	if (vga.sequencer.data[2]&2)
		vga_dirty_w(space, (offset<<2)+1, vga_latch_write(1,data));
	if (vga.sequencer.data[2]&4)
		vga_dirty_font_w(space, (offset<<2)+2, vga_latch_write(2,data));
	if (vga.sequencer.data[2]&8)
		vga_dirty_w(space, (offset<<2)+3, vga_latch_write(3,data));
	if ((offset==0xffff)&&(data==0)) vga.log=1;
}

static  READ8_HANDLER(vga_vga_r)
{
	int data;
	data=vga.memory[((offset&~3)<<2)|(offset&3)];

	return data;
}

static WRITE8_HANDLER(vga_vga_w)
{
	vga_dirty_font_w(space, ((offset&~3)<<2)|(offset&3),data);
}

static void vga_cpu_interface(running_machine &machine)
{
	address_space *space = machine.firstcpu->memory().space(vga.vga_intf.mem_addressspace);
	static int sequencer, gc;
	read8_space_func read_handler; const char *read_handler_name;
	write8_space_func write_handler; const char *write_handler_name;
	UINT8 sel;
	int buswidth;
	UINT64 mask = 0;

	if ((gc==vga.gc.data[6])&&(sequencer==vga.sequencer.data[4])) return;

	gc=vga.gc.data[6];
	sequencer=vga.sequencer.data[4];

	if (vga.sequencer.data[4]&8)
	{
		read_handler = vga_vga_r; read_handler_name = "vga_vga_r";
		write_handler = vga_vga_w; write_handler_name = "vga_vga_w";
	}
	else if (vga.sequencer.data[4] & 4)
	{
		read_handler = vga_ega_r; read_handler_name = "vga_ega_r";
		write_handler = vga_ega_w; write_handler_name = "vga_ega_w";
	}
	else
	{
		read_handler = vga_text_r; read_handler_name = "vga_text_r";
		write_handler = vga_text_w; write_handler_name = "vga_text_w";
	}

	/* remap the VGA memory */

	if (vga.vga_intf.map_vga_memory)
	{
		sel = vga.gc.data[6] & 0x0c;
		switch(sel)
		{
			case 0x00:
				if (vga.vga_intf.vga_memory_bank != NULL)
				{
					vga.vga_intf.map_vga_memory(machine, vga.vga_intf.mem_offset + 0x00000, vga.vga_intf.mem_offset + 0x1FFFF, read_handler, read_handler_name, write_handler, write_handler_name);
					memory_set_bankptr(machine, vga.vga_intf.vga_memory_bank, vga.memory);
				}
				break;
			case 0x04:
				vga.vga_intf.map_vga_memory(machine, vga.vga_intf.mem_offset + 0x00000, vga.vga_intf.mem_offset + 0x0FFFF, read_handler, read_handler_name, write_handler, write_handler_name);
				break;
			case 0x08:
				vga.vga_intf.map_vga_memory(machine, vga.vga_intf.mem_offset + 0x10000, vga.vga_intf.mem_offset + 0x17FFF, read_handler, read_handler_name, write_handler, write_handler_name);
				break;
			case 0x0C:
				vga.vga_intf.map_vga_memory(machine, vga.vga_intf.mem_offset + 0x18000, vga.vga_intf.mem_offset + 0x1FFFF, read_handler, read_handler_name, write_handler, write_handler_name);
				break;
		}
	}
	else
	{
		buswidth = downcast<legacy_cpu_device *>(machine.firstcpu)->space_config(AS_PROGRAM)->m_databus_width;
		switch(buswidth)
		{
			case 8:
				mask = 0;
				break;

			case 16:
				mask = 0xffff;
				break;

			case 32:
				mask = 0xffffffff;
				break;

			case 64:
				mask = -1;
				break;

			default:
				fatalerror("VGA: Bus width %d not supported", buswidth);
				break;
		}

		sel = vga.gc.data[6] & 0x0c;
		if (sel)
		{
			if (sel == 0x04) space->install_legacy_read_handler(vga.vga_intf.mem_offset + 0x00000, vga.vga_intf.mem_offset + 0x0ffff, read_handler, read_handler_name, mask); else space->nop_read(vga.vga_intf.mem_offset + 0x00000, vga.vga_intf.mem_offset + 0x0ffff);
			if (sel == 0x08) space->install_legacy_read_handler(vga.vga_intf.mem_offset + 0x10000, vga.vga_intf.mem_offset + 0x17fff, read_handler, read_handler_name, mask); else space->nop_read(vga.vga_intf.mem_offset + 0x10000, vga.vga_intf.mem_offset + 0x17fff);
			if (sel == 0x0C) space->install_legacy_read_handler(vga.vga_intf.mem_offset + 0x18000, vga.vga_intf.mem_offset + 0x1ffff, read_handler, read_handler_name, mask); else space->nop_read(vga.vga_intf.mem_offset + 0x18000, vga.vga_intf.mem_offset + 0x1ffff);
			if (sel == 0x04) space->install_legacy_write_handler(vga.vga_intf.mem_offset + 0x00000, vga.vga_intf.mem_offset + 0x0ffff, write_handler, write_handler_name, mask); else space->nop_write(vga.vga_intf.mem_offset + 0x00000, vga.vga_intf.mem_offset + 0x0ffff);
			if (sel == 0x08) space->install_legacy_write_handler(vga.vga_intf.mem_offset + 0x10000, vga.vga_intf.mem_offset + 0x17fff, write_handler, write_handler_name, mask); else space->nop_write(vga.vga_intf.mem_offset + 0x10000, vga.vga_intf.mem_offset + 0x17fff);
			if (sel == 0x0C) space->install_legacy_write_handler(vga.vga_intf.mem_offset + 0x18000, vga.vga_intf.mem_offset + 0x1ffff, write_handler, write_handler_name, mask); else space->nop_write(vga.vga_intf.mem_offset + 0x18000, vga.vga_intf.mem_offset + 0x1ffff);
		}
		else
		{
			memory_set_bankptr(machine,"bank1", vga.memory);
			space->install_read_bank(vga.vga_intf.mem_offset + 0x00000, vga.vga_intf.mem_offset + 0x1ffff, "bank1" );
			space->install_write_bank(vga.vga_intf.mem_offset + 0x00000, vga.vga_intf.mem_offset + 0x1ffff, "bank1" );
		}

	}
}

static READ8_HANDLER(vga_crtc_r)
{
	UINT8 data = 0xff;

	switch (offset) {
	case 4:
		data = vga.crtc.index;
		break;
	case 5:
		if (vga.crtc.index < vga.svga_intf.crtc_regcount)
			data = vga.crtc.data[vga.crtc.index];
		break;
	case 0xa:
		vga.attribute.state = 0;
		data = 0;/*4; */
#if 0 /* slow */
		{
			int clock=vga.monitor.get_clock();
			int lines=vga.monitor.get_lines();
			int columns=vga.monitor.get_columns();
			int diff = (((space->machine().time() - vga.monitor.start_time) * clock).seconds)
				%(lines*columns);
			if (diff<columns*vga.monitor.get_sync_lines()) data|=8;
			diff=diff/lines;
			if (diff%columns<vga.monitor.get_sync_columns()) data|=1;
		}
#elif 1
		if (vga.monitor.retrace)
		{
			data |= 1;
			if ((space->machine().time() - vga.monitor.start_time) > attotime::from_usec(300))
			{
				data |= 8;
				vga.monitor.retrace=0;
			}
		}
		else
		{
			if ((space->machine().time() - vga.monitor.start_time)  > attotime::from_msec(15))
				vga.monitor.retrace=1;
			vga.monitor.start_time=space->machine().time();
		}
#else
		// not working with ps2m30
		if (vga.monitor.retrace) data|=9;
		vga.monitor.retrace=0;
#endif
		/* ega diagnostic readback enough for oak bios */
		switch (vga.attribute.data[0x12]&0x30) {
		case 0:
			if (vga.attribute.data[0x11]&1) data|=0x10;
			if (vga.attribute.data[0x11]&4) data|=0x20;
			break;
		case 0x10:
			data|=(vga.attribute.data[0x11]&0x30);
			break;
		case 0x20:
			if (vga.attribute.data[0x11]&2) data|=0x10;
			if (vga.attribute.data[0x11]&8) data|=0x20;
			break;
		case 0x30:
			data|=(vga.attribute.data[0x11]&0xc0)>>2;
			break;
		}
		break;
	case 0xf:
		/* oak test */
		//data=0;
		/* pega bios on/off */
		data=0x80;
		break;
	}
	return data;
}

static WRITE8_HANDLER(vga_crtc_w)
{
	switch (offset)
	{
		case 0xa:
			vga.feature_control = data;
			break;

		case 4:
			vga.crtc.index = data;
			break;

		case 5:
			if (LOG_REGISTERS)
			{
				logerror("vga_crtc_w(): CRTC[0x%02X%s] = 0x%02X\n",
					vga.crtc.index,
					(vga.crtc.index < vga.svga_intf.crtc_regcount) ? "" : "?",
					data);
			}
			if(vga.crtc.index == 0x18 || vga.crtc.index == 0x07 || vga.crtc.index == 0x19 ) // Line compare
				vga.line_compare = (((vga.crtc.data[0x09] & 0x40) << 3) | ((vga.crtc.data[0x07] & 0x10) << 4) | vga.crtc.data[0x18])/2;
			if (vga.crtc.index < vga.svga_intf.crtc_regcount)
				vga.crtc.data[vga.crtc.index] = data;
			break;
	}
}



READ8_HANDLER( vga_port_03b0_r )
{
	UINT8 data = 0xff;
	if (CRTC_PORT_ADDR==0x3b0)
		data=vga_crtc_r(space, offset);
	return data;
}

READ8_HANDLER( vga_port_03c0_r )
{
	UINT8 data = 0xff;

	switch (offset)
	{
		case 1:
			if (vga.attribute.state==0)
			{
				data = vga.attribute.index;
			}
			else
			{
				if ((vga.attribute.index&0x1f)<sizeof(vga.attribute.data))
					data=vga.attribute.data[vga.attribute.index&0x1f];
			}
			break;

		case 2:
			data = 0;
			switch ((vga.miscellaneous_output>>2)&3)
			{
				case 3:
					if (vga.vga_intf.read_dipswitch && vga.vga_intf.read_dipswitch(space, 0) & 0x01)
						data |= 0x10;
					break;
				case 2:
					if (vga.vga_intf.read_dipswitch && vga.vga_intf.read_dipswitch(space, 0) & 0x02)
						data |= 0x10;
					break;
				case 1:
					if (vga.vga_intf.read_dipswitch && vga.vga_intf.read_dipswitch(space, 0) & 0x04)
						data |= 0x10;
					break;
				case 0:
					if (vga.vga_intf.read_dipswitch && vga.vga_intf.read_dipswitch(space, 0) & 0x08)
						data |= 0x10;
					break;
			}
			break;

		case 3:
			data = vga.oak.reg;
			break;

		case 4:
			data = vga.sequencer.index;
			break;

		case 5:
			if (vga.sequencer.index < vga.svga_intf.seq_regcount)
				data = vga.sequencer.data[vga.sequencer.index];
			break;

		case 6:
			data = vga.dac.mask;
			break;

		case 7:
			if (vga.dac.read)
				data = 0;
			else
				data = 3;
			break;

		case 8:
			data = vga.dac.write_index;
			break;

		case 9:
			if (vga.dac.read)
			{
				switch (vga.dac.state++)
				{
					case 0:
						data = vga.dac.color[vga.dac.read_index].red;
						break;
					case 1:
						data = vga.dac.color[vga.dac.read_index].green;
						break;
					case 2:
						data = vga.dac.color[vga.dac.read_index].blue;
						break;
				}

				if (vga.dac.state==3)
				{
					vga.dac.state = 0;
					vga.dac.read_index++;
				}
			}
			break;

		case 0xa:
			data = vga.feature_control;
			break;

		case 0xc:
			data = vga.miscellaneous_output;
			break;

		case 0xe:
			data = vga.gc.index;
			break;

		case 0xf:
			if (vga.gc.index < vga.svga_intf.gc_regcount)
				data = vga.gc.data[vga.gc.index];
			break;
	}
	return data;
}

READ8_HANDLER(vga_port_03d0_r)
{
	UINT8 data = 0xff;
	if (CRTC_PORT_ADDR == 0x3d0)
		data = vga_crtc_r(space, offset);
	return data;
}

WRITE8_HANDLER( vga_port_03b0_w )
{
	if (LOG_ACCESSES)
		logerror("vga_port_03b0_w(): port=0x%04x data=0x%02x\n", offset + 0x3b0, data);

	if (CRTC_PORT_ADDR == 0x3b0)
		vga_crtc_w(space, offset, data);
}

WRITE8_HANDLER(vga_port_03c0_w)
{
	if (LOG_ACCESSES)
		logerror("vga_port_03c0_w(): port=0x%04x data=0x%02x\n", offset + 0x3c0, data);

	switch (offset) {
	case 0:
		if (vga.attribute.state==0)
		{
			vga.attribute.index=data;
		}
		else
		{
			if ((vga.attribute.index&0x1f)<sizeof(vga.attribute.data))
				vga.attribute.data[vga.attribute.index&0x1f]=data;
		}
		vga.attribute.state=!vga.attribute.state;
		break;
	case 2:
		vga.miscellaneous_output=data;
		break;
	case 3:
		vga.oak.reg = data;
		break;
	case 4:
		vga.sequencer.index = data;
		break;
	case 5:
		if (LOG_REGISTERS)
		{
			logerror("vga_port_03c0_w(): SEQ[0x%02X%s] = 0x%02X\n",
				vga.sequencer.index,
				(vga.sequencer.index < vga.svga_intf.seq_regcount) ? "" : "?",
				data);
		}
		if (vga.sequencer.index < vga.svga_intf.seq_regcount)
		{
			vga.sequencer.data[vga.sequencer.index] = data;
			vga_cpu_interface(space->machine());

			if (vga.sequencer.index == 0)
				vga.monitor.start_time = space->machine().time();
		}
		break;
	case 6:
		vga.dac.mask=data;
		break;
	case 7:
		vga.dac.read_index=data;
		vga.dac.state=0;
		vga.dac.read=1;
		break;
	case 8:
		vga.dac.write_index=data;
		vga.dac.state=0;
		vga.dac.read=0;
		break;
	case 9:
		if (!vga.dac.read)
		{
			switch (vga.dac.state++) {
			case 0:
				vga.dac.color[vga.dac.write_index].red=data;
				break;
			case 1:
				vga.dac.color[vga.dac.write_index].green=data;
				break;
			case 2:
				vga.dac.color[vga.dac.write_index].blue=data;
				break;
			}
			vga.dac.dirty=1;
			if (vga.dac.state==3) {
				vga.dac.state=0; vga.dac.write_index++;
#if 0
				if (vga.dac.write_index==64) {
					int i;
					mame_printf_debug("start palette\n");
					for (i=0;i<64;i++) {
						mame_printf_debug(" 0x%.2x, 0x%.2x, 0x%.2x,\n",
							   vga.dac.color[i].red*4,
							   vga.dac.color[i].green*4,
							   vga.dac.color[i].blue*4);
					}
				}
#endif
			}
		}
		break;
	case 0xe:
		vga.gc.index=data;
		break;
	case 0xf:
		if (LOG_REGISTERS)
		{
			logerror("vga_port_03c0_w(): GC[0x%02X%s] = 0x%02X\n",
				vga.gc.index,
				(vga.gc.index < vga.svga_intf.gc_regcount) ? "" : "?",
				data);
		}
		if (vga.gc.index < vga.svga_intf.gc_regcount)
		{
			vga.gc.data[vga.gc.index] = data;
			vga_cpu_interface(space->machine());
		}
		break;
	}
}



WRITE8_HANDLER(vga_port_03d0_w)
{
	if (LOG_ACCESSES)
		logerror("vga_port_03d0_w(): port=0x%04x data=0x%02x\n", offset + 0x3d0, data);

	if (CRTC_PORT_ADDR == 0x3d0)
		vga_crtc_w(space, offset, data);
}

void pc_vga_reset(running_machine &machine)
{
	/* clear out the VGA structure */
	memset(vga.pens, 0, sizeof(vga.pens));
	vga.miscellaneous_output = 0;
	vga.feature_control = 0;
	vga.sequencer.index = 0;
	memset(vga.sequencer.data, 0, vga.svga_intf.seq_regcount * sizeof(*vga.sequencer.data));
	vga.crtc.index = 0;
	memset(vga.crtc.data, 0, vga.svga_intf.crtc_regcount * sizeof(*vga.crtc.data));
	vga.gc.index = 0;
	memset(vga.gc.data, 0, vga.svga_intf.gc_regcount * sizeof(*vga.gc.data));
	memset(vga.gc.latch, 0, sizeof(vga.gc.latch));
	memset(&vga.attribute, 0, sizeof(vga.attribute));
	memset(&vga.dac, 0, sizeof(vga.dac));
	memset(&vga.cursor, 0, sizeof(vga.cursor));
	memset(&vga.monitor, 0, sizeof(vga.monitor));
	memset(&vga.oak, 0, sizeof(vga.oak));
	vga.log = 0;

	vga.gc.data[6] = 0xc; /* prevent xtbios excepting vga ram as system ram */
/* amstrad pc1640 bios relies on the position of
   the video memory area,
   so I introduced the reset to switch to b8000 area */
	vga.sequencer.data[4] = 0;
	vga_cpu_interface(machine);

	vga.line_compare = 0x3ff;
	// set CRTC register to match the line compare value
	vga.crtc.data[0x18] = vga.line_compare & 0xff;
	if(vga.line_compare & 0x100)
		vga.crtc.data[0x07] |= 0x10;
	if(vga.line_compare & 0x200)
		vga.crtc.data[0x09] |= 0x40;
}

void pc_vga_init(running_machine &machine, const struct pc_vga_interface *vga_intf, const struct pc_svga_interface *svga_intf)
{
	int i, j, k, mask1, buswidth;
	address_space *io_space; // , mem_space;

	memset(&vga, 0, sizeof(vga));

	for (k=0;k<4;k++)
	{
		for (mask1=0x80, j=0; j<8; j++, mask1>>=1)
		{
			for  (i=0; i<256; i++)
				color_bitplane_to_packed[k][j][i]=(i&mask1)?(1<<k):0;
		}
	}
	UINT64 mask = 0;
	/* copy over interfaces */
	vga.vga_intf = *vga_intf;
	if (svga_intf)
	{
		vga.svga_intf = *svga_intf;

		if (vga.svga_intf.seq_regcount < 0x05)
			fatalerror("Invalid SVGA sequencer register count");
		if (vga.svga_intf.gc_regcount < 0x09)
			fatalerror("Invalid SVGA GC register count");
		if (vga.svga_intf.crtc_regcount < 0x19)
			fatalerror("Invalid SVGA CRTC register count");
	}
	else
	{
		vga.svga_intf.vram_size = 0x40000;
		vga.svga_intf.seq_regcount = 0x05;
		vga.svga_intf.gc_regcount = 0x09;
		vga.svga_intf.crtc_regcount = 0x19;
	}

	vga.memory			= auto_alloc_array(machine, UINT8, vga.svga_intf.vram_size);
	vga.fontdirty		= auto_alloc_array(machine, UINT8, 0x800);
	vga.sequencer.data	= auto_alloc_array(machine, UINT8, vga.svga_intf.seq_regcount);
	vga.crtc.data		= auto_alloc_array(machine, UINT8, vga.svga_intf.crtc_regcount);
	vga.gc.data			= auto_alloc_array(machine, UINT8, vga.svga_intf.gc_regcount);
	memset(vga.memory, '\0', vga.svga_intf.vram_size);
	memset(vga.fontdirty, '\0', 0x800);
	memset(vga.sequencer.data, '\0', vga.svga_intf.seq_regcount);
	memset(vga.crtc.data, '\0', vga.svga_intf.crtc_regcount);
	memset(vga.gc.data, '\0', vga.svga_intf.gc_regcount);

	buswidth = downcast<legacy_cpu_device *>(machine.firstcpu)->space_config(AS_PROGRAM)->m_databus_width;
//  mem_space = machine.firstcpu->memory().space(vga.vga_intf.mem_addressspace);
	io_space = machine.firstcpu->memory().space(vga.vga_intf.port_addressspace);
	switch(buswidth)
	{
		case 8:
			mask = 0;
			break;

		case 16:
			mask = 0xffff;
			break;

		case 32:
			mask = 0xffffffff;
			break;

		case 64:
			mask = -1;
			break;

		default:
			fatalerror("VGA: Bus width %d not supported", buswidth);
			break;
	}
	io_space->install_legacy_read_handler(vga.vga_intf.port_offset + 0x3b0, vga.vga_intf.port_offset + 0x3bf, FUNC(vga_port_03b0_r), mask);
	io_space->install_legacy_read_handler(vga.vga_intf.port_offset + 0x3c0, vga.vga_intf.port_offset + 0x3cf, FUNC(vga_port_03c0_r), mask);
	io_space->install_legacy_read_handler(vga.vga_intf.port_offset + 0x3d0, vga.vga_intf.port_offset + 0x3df, FUNC(vga_port_03d0_r), mask);

	io_space->install_legacy_write_handler(vga.vga_intf.port_offset + 0x3b0, vga.vga_intf.port_offset + 0x3bf, FUNC(vga_port_03b0_w), mask);
	io_space->install_legacy_write_handler(vga.vga_intf.port_offset + 0x3c0, vga.vga_intf.port_offset + 0x3cf, FUNC(vga_port_03c0_w), mask);
	io_space->install_legacy_write_handler(vga.vga_intf.port_offset + 0x3d0, vga.vga_intf.port_offset + 0x3df, FUNC(vga_port_03d0_w), mask);

	pc_vga_reset(machine);
}

static TIMER_CALLBACK(vga_timer)
{
	vga.monitor.retrace=1;
}

static VIDEO_START( vga )
{
	vga.monitor.get_clock=vga_get_clock;
	vga.monitor.get_lines=vga_get_crtc_lines;
	vga.monitor.get_columns=vga_get_crtc_columns;
	vga.monitor.get_sync_lines=vga_get_crtc_sync_lines;
	vga.monitor.get_sync_columns=vga_get_crtc_sync_columns;
	machine.scheduler().timer_pulse(attotime::from_hz(60), FUNC(vga_timer));
	pc_video_start(machine, pc_vga_choosevideomode);
}

static VIDEO_RESET( vga )
{
	pc_vga_reset(machine);
}

static void vga_vh_text(bitmap_t *bitmap)
{
	UINT8 ch, attr;
	UINT8 bits;
	UINT8 *font;
	UINT16 *bitmapline;
	int width=CHAR_WIDTH, height=CRTC_CHAR_HEIGHT;
	int pos, line, column, mask, w, h, addr;
	pen_t pen;

	if (CRTC_CURSOR_MODE!=CRTC_CURSOR_OFF)
	{
		if (++vga.cursor.time>=0x10)
		{
			vga.cursor.visible^=1;
			vga.cursor.time=0;
		}
	}

	for (addr = TEXT_START_ADDRESS, line = -CRTC_SKEW; line < TEXT_LINES;
		 line += height, addr += TEXT_LINE_LENGTH)
	{
		for (pos = addr, column=0; column<TEXT_COLUMNS; column++, pos++)
		{
			ch   = vga.memory[(pos<<2) + 0];
			attr = vga.memory[(pos<<2) + 1];
			font = vga.memory+2+(ch<<(5+2))+FONT1;

			for (h = MAX(-line, 0); (h < height) && (line+h < MIN(TEXT_LINES, bitmap->height)); h++)
			{
				bitmapline = BITMAP_ADDR16(bitmap, line+h, 0);
				bits = font[h<<2];

				assert(bitmapline);

				for (mask=0x80, w=0; (w<width)&&(w<8); w++, mask>>=1)
				{
					if (bits&mask)
						pen = vga.pens[attr & 0x0f];
					else
						pen = vga.pens[attr >> 4];
					bitmapline[column*width+w] = pen;
				}
				if (w<width)
				{
					/* 9 column */
					if (TEXT_COPY_9COLUMN(ch)&&(bits&1))
						pen = vga.pens[attr & 0x0f];
					else
						pen = vga.pens[attr >> 4];
					bitmapline[column*width+w] = pen;
				}
			}
			if ((CRTC_CURSOR_MODE!=CRTC_CURSOR_OFF)
				&&vga.cursor.visible&&(pos==CRTC_CURSOR_POS))
			{
				for (h=CRTC_CURSOR_TOP;
					 (h<=CRTC_CURSOR_BOTTOM)&&(h<height)&&(line+h<TEXT_LINES);
					 h++)
				{
					plot_box(bitmap, column*width, line+h, width, 1, vga.pens[attr&0xf]);
				}
			}
		}
	}
}

static void vga_vh_ega(bitmap_t *bitmap)
{
	int pos, line, column, c, addr, i;
	int height = CRTC_CHAR_HEIGHT;
	UINT16 *bitmapline;
	UINT16 *newbitmapline;
	pen_t pen;

	for (addr=EGA_START_ADDRESS, pos=0, line=0; line<LINES;
		 line += height, addr=(addr+EGA_LINE_LENGTH)&0x3ffff)
	{
		bitmapline = BITMAP_ADDR16(bitmap, line, 0);

		for (pos=addr, c=0, column=0; column<EGA_COLUMNS; column++, c+=8, pos=(pos+4)&0x3ffff)
		{
			int data[4];

			data[0]=vga.memory[pos];
			data[1]=vga.memory[pos+1]<<1;
			data[2]=vga.memory[pos+2]<<2;
			data[3]=vga.memory[pos+3]<<3;

			for (i = 7; i >= 0; i--)
			{
				pen = vga.pens[(data[0]&1) | (data[1]&2) | (data[2]&4) | (data[3]&8)];
				bitmapline[c+i] = pen;

				data[0]>>=1;
				data[1]>>=1;
				data[2]>>=1;
				data[3]>>=1;
			}
		}

		for (i = 1; i < height; i++)
		{
			if (line + i >= LINES)
				break;

			newbitmapline = BITMAP_ADDR16(bitmap, line+i, 0);
			memcpy(newbitmapline, bitmapline, EGA_COLUMNS * 8 * sizeof(UINT16));
		}
	}
}

static void vga_vh_vga(bitmap_t *bitmap)
{
	int pos, line, column, c, addr, curr_addr;
	UINT16 *bitmapline;

	curr_addr = 0;
	if(vga.sequencer.data[4] & 0x08)
	{
		for (addr = VGA_START_ADDRESS, line=0; line<LINES; line++, addr+=VGA_LINE_LENGTH, curr_addr+=VGA_LINE_LENGTH)
		{
			if(line < (vga.line_compare & 0xff))
				curr_addr = addr;
			if(line == (vga.line_compare & 0xff))
				curr_addr = 0;
			bitmapline = BITMAP_ADDR16(bitmap, line, 0);
			addr %= vga.svga_intf.vram_size;
			for (pos=curr_addr, c=0, column=0; column<VGA_COLUMNS; column++, c+=8, pos+=0x20)
			{
				if(pos + 0x20 > vga.svga_intf.vram_size)
					return;
				bitmapline[c+0] = vga.memory[pos+0];
				bitmapline[c+1] = vga.memory[pos+1];
				bitmapline[c+2] = vga.memory[pos+2];
				bitmapline[c+3] = vga.memory[pos+3];
				bitmapline[c+4] = vga.memory[pos+0x10];
				bitmapline[c+5] = vga.memory[pos+0x11];
				bitmapline[c+6] = vga.memory[pos+0x12];
				bitmapline[c+7] = vga.memory[pos+0x13];
			}
		}
	}
	else
	{
		for (addr = VGA_START_ADDRESS, line=0; line<LINES; line++, addr+=VGA_LINE_LENGTH/4, curr_addr+=VGA_LINE_LENGTH/4)
		{
			if(line < (vga.line_compare & 0xff))
				curr_addr = addr;
			if(line == (vga.line_compare & 0xff))
				curr_addr = 0;
			bitmapline = BITMAP_ADDR16(bitmap, line, 0);
			addr %= vga.svga_intf.vram_size;
			for (pos=curr_addr, c=0, column=0; column<VGA_COLUMNS; column++, c+=8, pos+=0x08)
			{
				if(pos + 0x08 > vga.svga_intf.vram_size)
					return;
				bitmapline[c+0] = vga.memory[pos+0];
				bitmapline[c+1] = vga.memory[pos+1];
				bitmapline[c+2] = vga.memory[pos+2];
				bitmapline[c+3] = vga.memory[pos+3];
				bitmapline[c+4] = vga.memory[pos+4];
				bitmapline[c+5] = vga.memory[pos+5];
				bitmapline[c+6] = vga.memory[pos+6];
				bitmapline[c+7] = vga.memory[pos+7];
			}
		}
	}
}

static pc_video_update_proc pc_vga_choosevideomode(running_machine &machine, int *width, int *height)
{
	pc_video_update_proc proc = NULL;
	int i;

	if (CRTC_ON)
	{
		if (vga.dac.dirty)
		{
			for (i=0; i<256;i++)
			{
				palette_set_color_rgb(machine, i,(vga.dac.color[i].red & 0x3f) << 2,
									 (vga.dac.color[i].green & 0x3f) << 2,
									 (vga.dac.color[i].blue & 0x3f) << 2);
			}
			vga.dac.dirty = 0;
		}

		if (vga.attribute.data[0x10] & 0x80)
		{
			for (i=0; i<16;i++)
			{
				vga.pens[i] = machine.pens[(vga.attribute.data[i]&0x0f)
										 |((vga.attribute.data[0x14]&0xf)<<4)];
			}
		}
		else
		{
			for (i=0; i<16;i++)
			{
				vga.pens[i]=machine.pens[(vga.attribute.data[i]&0x3f)
										 |((vga.attribute.data[0x14]&0xc)<<4)];
			}
		}

		if (vga.svga_intf.choosevideomode)
			proc = vga.svga_intf.choosevideomode(vga.sequencer.data, vga.crtc.data, vga.gc.data, width, height);

		if (!proc)
		{
			if (!GRAPHIC_MODE)
			{
				proc = vga_vh_text;
				*height = TEXT_LINES;
				*width = TEXT_COLUMNS * CHAR_WIDTH;
			}
			else if (vga.gc.data[5]&0x40)
			{
				proc = vga_vh_vga;
				*height = LINES;
				*width = VGA_COLUMNS * 8;
			}
			else
			{
				proc = vga_vh_ega;
				*height = LINES;
				*width = EGA_COLUMNS * 8;
			}
		}
	}
	return proc;
}



void *pc_vga_memory(void)
{
	return vga.memory;
}



size_t pc_vga_memory_size(void)
{
	return vga.svga_intf.vram_size;
}

MACHINE_CONFIG_FRAGMENT( pcvideo_vga )
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(720, 480)
	MCFG_SCREEN_VISIBLE_AREA(0,720-1, 0,480-1)
	MCFG_SCREEN_UPDATE(pc_video)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */

	MCFG_PALETTE_LENGTH(0x100)
	MCFG_PALETTE_INIT(vga)

	MCFG_VIDEO_START(vga)
	MCFG_VIDEO_RESET(vga)
MACHINE_CONFIG_END
