/*
 * trident.c
 *
 * Implementation of Trident VGA GUI accelerators
 *
 *
 */

#include "emu.h"
#include "trident.h"
#include "debugger.h"

const device_type TRIDENT_VGA = &device_creator<trident_vga_device>;

#define CRTC_PORT_ADDR ((vga.miscellaneous_output&1)?0x3d0:0x3b0)

#define READPIXEL8(x,y) (vga.memory[(y*offset() + x) % vga.svga_intf.vram_size])
#define WRITEPIXEL8(x,y,c) if(x<tri.accel_dest_y_clip && y<tri.accel_dest_y_clip) \
	(vga.memory[(y*offset() + x) % vga.svga_intf.vram_size] = c)

#define LOG (1)

trident_vga_device::trident_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: svga_device(mconfig, TRIDENT_VGA, "Trident TGUI9680", tag, owner, clock, "trident_vga", __FILE__)
{
}

void trident_vga_device::device_start()
{
	memset(&vga, 0, sizeof(vga));
	memset(&svga, 0, sizeof(svga));

	int i;
	for (i = 0; i < 0x100; i++)
		m_palette->set_pen_color(i, 0, 0, 0);

	// Avoid an infinite loop when displaying.  0 is not possible anyway.
	vga.crtc.maximum_scan_line = 1;


	// copy over interfaces
	vga.read_dipswitch = read8_delegate(); //read_dipswitch;
	vga.svga_intf.vram_size = 0x200000;

	vga.memory.resize_and_clear(vga.svga_intf.vram_size);
	save_item(NAME(vga.memory));
	save_pointer(vga.crtc.data,"CRTC Registers",0x100);
	save_pointer(vga.sequencer.data,"Sequencer Registers",0x100);
	save_pointer(vga.attribute.data,"Attribute Registers", 0x15);
	save_pointer(tri.accel_pattern,"Pattern Data", 0x80);

	m_vblank_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(vga_device::vblank_timer_cb),this));
	vga.svga_intf.seq_regcount = 0x0f;
	vga.svga_intf.crtc_regcount = 0x60;
	memset(&tri, 0, sizeof(tri));
}

void trident_vga_device::device_reset()
{
	svga_device::device_reset();
	svga.id = 0xd3;  // identifies at TGUI9660XGi
	tri.revision = 0x01;  // revision identifies as TGUI9680
	tri.new_mode = false;  // start up in old mode
	tri.dac_active = false;
	tri.linear_active = false;
	tri.mmio_active = false;
	tri.sr0f = 0x6f;
	tri.sr0c = 0x78;
	tri.port_3c3 = true;
	tri.accel_busy = false;
}

UINT16 trident_vga_device::offset()
{
	UINT16 off = svga_device::offset();

	if (svga.rgb8_en || svga.rgb15_en || svga.rgb16_en || svga.rgb32_en)
		return vga.crtc.offset << 3;  // don't know if this is right, but Eggs Playing Chicken switches off doubleword mode, but expects the same offset length
	else
		return off;
}

void trident_vga_device::trident_define_video_mode()
{
	int divisor = 1;
	int xtal;

	switch(tri.clock)
	{
	case 0:
	default: xtal = XTAL_25_1748MHz; break;
	case 1:  xtal = XTAL_28_63636MHz; break;
	case 2:  xtal = 44900000; break;
	case 3:  xtal = 36000000; break;
	case 4:  xtal = 57272000; break;
	case 5:  xtal = 65000000; break;
	case 6:  xtal = 50350000; break;
	case 7:  xtal = 40000000; break;
	case 8:  xtal = 88000000; break;
	case 9:  xtal = 98000000; break;
	case 10: xtal = 118800000; break;
	case 11: xtal = 108000000; break;
	case 12: xtal = 72000000; break;
	case 13: xtal = 77000000; break;
	case 14: xtal = 80000000; break;
	case 15: xtal = 75000000; break;
	}

	switch((tri.sr0d_new & 0x06) >> 1)
	{
	case 0:
	default:  break;  // no division
	case 1:   xtal = xtal / 2; break;
	case 2:   xtal = xtal / 4; break;
	case 3:   xtal = xtal / 1.5; break;
	}

	// TODO: determine when 8 bit modes are selected
	svga.rgb8_en = svga.rgb15_en = svga.rgb16_en = svga.rgb32_en = 0;
	switch((tri.pixel_depth & 0x0c) >> 2)
	{
	case 0:
	default: if(!(tri.pixel_depth & 0x10)) svga.rgb8_en = 1; break;
	case 1:  if((tri.dac & 0xf0) == 0x30) svga.rgb16_en = 1; else svga.rgb15_en = 1; break;
	case 2:  svga.rgb32_en = 1; break;
	}

	recompute_params_clock(divisor, xtal);
}

UINT8 trident_vga_device::trident_seq_reg_read(UINT8 index)
{
	UINT8 res;

	res = 0xff;

	if(index <= 0x04)
		res = vga.sequencer.data[index];
	else
	{
		switch(index)
		{
			case 0x09:
				res = tri.revision;
				break;
			case 0x0b:
				res = svga.id;
				tri.new_mode = true;
				//debugger_break(machine());
				break;
			case 0x0c:  // Power Up Mode register 1
				res = tri.sr0c & 0xef;
				if(tri.port_3c3)
					res |= 0x10;
				break;
			case 0x0d:  // Mode Control 2
				//res = svga.rgb15_en;
				if(tri.new_mode)
					res = tri.sr0d_new;
				else
					res = tri.sr0d_old;
				break;
			case 0x0e:  // Mode Control 1
				if(tri.new_mode)
					res = tri.sr0e_new;
				else
					res = tri.sr0e_old;
				break;
			case 0x0f:  // Power Up Mode 2
				res = tri.sr0f;
				break;
		}
	}
	if(LOG) logerror("Trident SR%02X: read %02x\n",index,res);
	return res;
}

void trident_vga_device::trident_seq_reg_write(UINT8 index, UINT8 data)
{
	if(index <= 0x04)
	{
		vga.sequencer.data[vga.sequencer.index] = data;
		seq_reg_write(vga.sequencer.index,data);
		recompute_params();
	}
	else
	{
		if(LOG) logerror("Trident SR%02X: %s mode write %02x\n",index,tri.new_mode ? "new" : "old",data);
		switch(index)
		{
			case 0x0b:
				tri.new_mode = false;
				break;
			case 0x0c:  // Power Up Mode register 1
				if(data & 0x10)
					tri.port_3c3 = true;  // 'post port at 0x3c3'
				else
					tri.port_3c3 = false; // 'post port at 0x46e8'
				tri.sr0c = data;
				break;
			case 0x0d:  // Mode Control 2
				//svga.rgb15_en = data & 0x30; // TODO: doesn't match documentation
				if(tri.new_mode)
				{
					tri.sr0d_new = data;
					tri.clock = ((vga.miscellaneous_output & 0x0c) >> 2) | ((data & 0x01) << 2) | ((data & 0x40) >> 3);
					trident_define_video_mode();
				}
				else
					tri.sr0d_old = data;
				break;
			case 0x0e:  // Mode Control 1
				if(tri.new_mode)
				{
					tri.sr0e_new = data ^ 0x02;
					svga.bank_w = (data & 0x3f) ^ 0x02;  // bit 1 is inverted, used for card detection, it is not XORed on reading
					if(!(tri.gc0f & 0x01))
						svga.bank_r = (data & 0x3f) ^ 0x02;
					// TODO: handle planar modes, where bits 0 and 2 only are used
				}
				else
				{
					tri.sr0e_old = data;
					svga.bank_w = data & 0x0e;
					if(!(tri.gc0f & 0x01))
						svga.bank_r = data & 0x0e;
				}
				break;
			case 0x0f:  // Power Up Mode 2
				tri.sr0f = data;
				break;
		}
	}
}

UINT8 trident_vga_device::trident_crtc_reg_read(UINT8 index)
{
	UINT8 res;

	if(index <= 0x18)
		res = crtc_reg_read(index);
	else
	{
		switch(index)
		{
		case 0x1e:
			res = tri.cr1e;
			break;
		case 0x1f:
			res = tri.cr1f;
			break;
		case 0x21:
			res = tri.cr21;
			break;
		case 0x27:
			res = (vga.crtc.start_addr & 0x60000) >> 17;
			break;
		case 0x29:
			res = tri.cr29;
			break;
		case 0x38:
			res = tri.pixel_depth;
			break;
		case 0x39:
			res = tri.cr39;
			break;
		case 0x50:
			res = tri.cr50;
			break;
		default:
			res = vga.crtc.data[index];
			break;
		}
	}
	if(LOG) logerror("Trident CR%02X: read %02x\n",index,res);
	return res;
}
void trident_vga_device::trident_crtc_reg_write(UINT8 index, UINT8 data)
{
	if(index <= 0x18)
	{
		crtc_reg_write(index,data);
		trident_define_video_mode();
	}
	else
	{
		if(LOG) logerror("Trident CR%02X: write %02x\n",index,data);
		switch(index)
		{
		case 0x1e:  // Module Testing Register
			tri.cr1e = data;
			vga.crtc.start_addr = (vga.crtc.start_addr & 0xfffeffff) | ((data & 0x20)<<11);
			break;
		case 0x1f:
			tri.cr1f = data;  // "Software Programming Register"  written to by software (BIOS?)
			break;
		case 0x21:  // Linear aperture
			tri.cr21 = data;
			tri.linear_address = ((data & 0xc0)<<18) | ((data & 0x0f)<<20);
			tri.linear_active = data & 0x20;
			if(tri.linear_active)
				popmessage("Trident: Linear Aperture active - %08x, %s",tri.linear_address,(tri.cr21 & 0x10) ? "2MB" : "1MB" );
			break;
		case 0x27:
			vga.crtc.start_addr = (vga.crtc.start_addr & 0xfff9ffff) | ((data & 0x03)<<17);
			break;
		case 0x29:
			tri.cr29 = data;
			vga.crtc.offset = (vga.crtc.offset & 0xfeff) | ((data & 0x10)<<4);
			break;
		case 0x38:
			tri.pixel_depth = data;
			trident_define_video_mode();
			break;
		case 0x39:
			tri.cr39 = data;
			tri.mmio_active = data & 0x01;
			if(tri.mmio_active)
				popmessage("Trident: MMIO activated");
			break;
		case 0x50:
			tri.cr50 = data;
			break;
		default:
			//logerror("Trident: 3D4 index %02x write %02x\n",index,data);
			break;
		}
	}
}

UINT8 trident_vga_device::trident_gc_reg_read(UINT8 index)
{
	UINT8 res;

	if(index <= 0x0d)
		res = gc_reg_read(index);
	else
	{
		switch(index)
		{
		case 0x0e:
			res = tri.gc0e;
			break;
		case 0x0f:
			res = tri.gc0f;
			break;
		case 0x2f:
			res = tri.gc2f;
			break;
		default:
			res = 0xff;
			break;
		}
	}
	if(LOG) logerror("Trident GC%02X: read %02x\n",index,res);
	return res;
}

void trident_vga_device::trident_gc_reg_write(UINT8 index, UINT8 data)
{
	if(index <= 0x0d)
		gc_reg_write(index,data);
	else
	{
		if(LOG) logerror("Trident GC%02X: write %02x\n",index,data);
		switch(index)
		{
		case 0x0e:  // New Source Address Register (bit 1 is inverted here, also)
			tri.gc0e = data ^ 0x02;
			if(!(tri.gc0f & 0x04))  // if bank regs at 0x3d8/9 are not enabled
			{
				if(tri.gc0f & 0x01)  // if bank regs are separated
					svga.bank_r = (data & 0x1f) ^ 0x02;
			}
			break;
		case 0x0f:
			tri.gc0f = data;
			break;
		case 0x2f:  // XFree86 refers to this register as "MiscIntContReg", setting bit 2, but gives no indication as to what it does
			tri.gc2f = data;
			break;
		default:
			//logerror("Trident: Unimplemented GC register %02x write %02x\n",index,data);
			break;
		}
	}
}

READ8_MEMBER(trident_vga_device::port_03c0_r)
{
	UINT8 res;

	switch(offset)
	{
		case 0x05:
			res = trident_seq_reg_read(vga.sequencer.index);
			break;
		case 0x06:
			tri.dac_count++;
			if(tri.dac_count > 3)
				tri.dac_active = true;
			if(tri.dac_active)
				res = tri.dac;
			else
				res = vga_device::port_03c0_r(space,offset,mem_mask);
			break;
		case 0x07:
		case 0x08:
		case 0x09:
			tri.dac_active = false;
			tri.dac_count = 0;
			res = vga_device::port_03c0_r(space,offset,mem_mask);
			break;
		case 0x0f:
			res = trident_gc_reg_read(vga.gc.index);
			break;
		default:
			res = vga_device::port_03c0_r(space,offset,mem_mask);
			break;
	}

	return res;
}

WRITE8_MEMBER(trident_vga_device::port_03c0_w)
{
	switch(offset)
	{
		case 0x05:
			trident_seq_reg_write(vga.sequencer.index,data);
			break;
		case 0x06:
			if(tri.dac_active)
			{
				tri.dac = data;  // DAC command register
				tri.dac_active = false;
				tri.dac_count = 0;
				trident_define_video_mode();
			}
			else
				vga_device::port_03c0_w(space,offset,data,mem_mask);
			break;
		case 0x07:
		case 0x08:
		case 0x09:
			tri.dac_active = false;
			tri.dac_count = 0;
			vga_device::port_03c0_w(space,offset,data,mem_mask);
			break;
		case 0x0f:
			trident_gc_reg_write(vga.gc.index,data);
			break;
		default:
			vga_device::port_03c0_w(space,offset,data,mem_mask);
			break;
	}
}


READ8_MEMBER(trident_vga_device::port_03d0_r)
{
	UINT8 res = 0xff;

	if (CRTC_PORT_ADDR == 0x3d0)
	{
		switch(offset)
		{
			case 5:
				res = trident_crtc_reg_read(vga.crtc.index);
				break;
			case 8:
				if(tri.gc0f & 0x04)  // if enabled
					res = svga.bank_w & 0x3f;
				else
					res = 0xff;
				break;
			case 9:
				if(tri.gc0f & 0x04)  // if enabled
					if(tri.gc0f & 0x01)  // and if bank regs are separated
						res = svga.bank_r & 0x3f;
					else
						res = 0xff;
				else
					res = 0xff;
				break;
			default:
				res = vga_device::port_03d0_r(space,offset,mem_mask);
				break;
		}
	}

	return res;
}

WRITE8_MEMBER(trident_vga_device::port_03d0_w)
{
	if (CRTC_PORT_ADDR == 0x3d0)
	{
		switch(offset)
		{
			case 5:
				vga.crtc.data[vga.crtc.index] = data;
				trident_crtc_reg_write(vga.crtc.index,data);
				break;
			case 8:
				if(tri.gc0f & 0x04)  // if enabled
				{
					svga.bank_w = data & 0x3f;
					if(!(tri.gc0f & 0x01))  // if bank regs are not separated
						svga.bank_r = data & 0x3f; // then this is also the read bank register
				}
				break;
			case 9:
				if(tri.gc0f & 0x04)  // if enabled
				{
					if(tri.gc0f & 0x01)  // and if bank regs are separated
						svga.bank_r = data & 0x3f;
				}
				break;
			default:
				vga_device::port_03d0_w(space,offset,data,mem_mask);
				break;
		}
	}
}

READ8_MEMBER(trident_vga_device::port_83c6_r)
{
	UINT8 res = 0xff;
	switch(offset)
	{
	case 2:
		res = port_03c0_r(space,5,mem_mask);
		if(LOG) logerror("Trident: 83c6 read %02x\n",res);
		break;
	case 4:
		res = vga.sequencer.index;
		if(LOG) logerror("Trident: 83c8 seq read %02x\n",res);
		break;
	}
	return res;
}

WRITE8_MEMBER(trident_vga_device::port_83c6_w)
{
	switch(offset)
	{
	case 2:
		if(LOG) logerror("Trident: 83c6 seq write %02x\n",data);
		port_03c0_w(space,5,data,mem_mask);
		break;
	case 4:
		if(LOG) logerror("Trident: 83c8 seq index write %02x\n",data);
		vga.sequencer.index = data;
		break;
	}
}

READ8_MEMBER(trident_vga_device::mem_r )
{
	if (svga.rgb8_en || svga.rgb15_en || svga.rgb16_en || svga.rgb32_en)
	{
		int data;

		if(tri.new_mode)  // 64k from 0xA0000-0xAFFFF
		{
			offset &= 0xffff;
			data=vga.memory[(offset + (svga.bank_r*0x10000)) % vga.svga_intf.vram_size];
		}
		else   // 128k from 0xA0000-0xBFFFF
		{
			data=vga.memory[(offset + (svga.bank_r*0x10000)) % vga.svga_intf.vram_size];
		}
		return data;
	}

	return vga_device::mem_r(space,offset,mem_mask);
}

WRITE8_MEMBER(trident_vga_device::mem_w)
{
	if (svga.rgb8_en || svga.rgb15_en || svga.rgb16_en || svga.rgb32_en)
	{
		if(tri.new_mode)  // 64k from 0xA0000-0xAFFFF
		{
			offset &= 0xffff;
			vga.memory[(offset + (svga.bank_w*0x10000)) % vga.svga_intf.vram_size] = data;
		}
		else   // 128k from 0xA0000-0xBFFFF
		{
			vga.memory[(offset + (svga.bank_w*0x10000)) % vga.svga_intf.vram_size] = data;
		}
		return;
	}

	vga_device::mem_w(space,offset,data,mem_mask);
}

// 2D Acceleration functions (very WIP)

// From XFree86 source:
/*
Graphics Engine for 9440/9660/9680

#define GER_STATUS	0x2120
#define		GE_BUSY	0x80
#define GER_OPERMODE	0x2122		 Byte for 9440, Word for 96xx
#define		DST_ENABLE	0x200	// Destination Transparency
#define GER_COMMAND	0x2124
#define		GE_NOP		0x00	// No Operation
#define		GE_BLT		0x01	// BitBLT ROP3 only
#define		GE_BLT_ROP4	0x02	// BitBLT ROP4 (96xx only)
#define		GE_SCANLINE	0x03	// Scan Line
#define		GE_BRESLINE	0x04	// Bresenham Line
#define		GE_SHVECTOR	0x05	// Short Vector
#define		GE_FASTLINE	0x06	// Fast Line (96xx only)
#define		GE_TRAPEZ	0x07	// Trapezoidal fill (96xx only)
#define		GE_ELLIPSE	0x08	// Ellipse (96xx only) (RES)
#define		GE_ELLIP_FILL	0x09	// Ellipse Fill (96xx only) (RES)
#define	GER_FMIX	0x2127
#define GER_DRAWFLAG	0x2128		// long
#define		FASTMODE	1<<28
#define		STENCIL		0x8000
#define		SOLIDFILL	0x4000
#define		TRANS_ENABLE	0x1000
#define 	TRANS_REVERSE	0x2000
#define		YMAJ		0x0400
#define		XNEG		0x0200
#define		YNEG		0x0100
#define		SRCMONO		0x0040
#define		PATMONO		0x0020
#define		SCR2SCR		0x0004
#define		PAT2SCR		0x0002
#define GER_FCOLOUR	0x212C		// Word for 9440, long for 96xx
#define GER_BCOLOUR	0x2130		// Word for 9440, long for 96xx
#define GER_PATLOC	0x2134		// Word
#define GER_DEST_XY	0x2138
#define GER_DEST_X	0x2138		// Word
#define GER_DEST_Y	0x213A		// Word
#define GER_SRC_XY	0x213C
#define GER_SRC_X	0x213C		// Word
#define GER_SRC_Y	0x213E		// Word
#define GER_DIM_XY	0x2140
#define GER_DIM_X	0x2140		// Word
#define GER_DIM_Y	0x2142		// Word
#define GER_STYLE	0x2144		// Long
#define GER_CKEY	0x2168		// Long
#define GER_FPATCOL	0x2178
#define GER_BPATCOL	0x217C
#define GER_PATTERN	0x2180		// from 0x2180 to 0x21FF

 Additional - Graphics Engine for 96xx
#define GER_SRCCLIP_XY	0x2148
#define GER_SRCCLIP_X	0x2148		// Word
#define GER_SRCCLIP_Y	0x214A		// Word
#define GER_DSTCLIP_XY	0x214C
#define GER_DSTCLIP_X	0x214C		// Word
#define GER_DSTCLIP_Y	0x214E		// Word
*/

READ8_MEMBER(trident_vga_device::accel_r)
{
	UINT8 res = 0xff;

	if(offset >= 0x60)
		return tri.accel_pattern[(offset-0x60) % 0x80];

	switch(offset)
	{
	case 0x00:  // Status
		if(tri.accel_busy)
			res = 0x80;
		else
			res = 0x00;
		break;
	case 0x02:  // Operation Mode
		res = tri.accel_opermode & 0x00ff;
		break;
	case 0x03:
		res = (tri.accel_opermode & 0xff00) >> 8;
		break;
	case 0x04:  // Command register
		res = tri.accel_command;
		break;
	case 0x07:  // Foreground Mix?
		res = tri.accel_fmix;
		break;
	default:
		logerror("Trident: unimplemented acceleration register offset %02x read\n",offset);
	}
	return res;
}

WRITE8_MEMBER(trident_vga_device::accel_w)
{
	if(offset >= 0x60)
	{
		tri.accel_pattern[(offset-0x60) % 0x80] = data;
		return;
	}

	switch(offset)
	{
	case 0x02:  // Operation Mode
		tri.accel_opermode = (tri.accel_opermode & 0xff00) | data;
		if(LOG) logerror("Trident: Operation Mode set to %04x\n",tri.accel_opermode);
		break;
	case 0x03:
		tri.accel_opermode = (tri.accel_opermode & 0x00ff) | (data << 8);
		if(LOG) logerror("Trident: Operation Mode set to %04x\n",tri.accel_opermode);
		break;
	case 0x04:  // Command register
		tri.accel_command = data;
		accel_command();
		break;
	case 0x07:  // Foreground Mix?
		tri.accel_fmix = data;
		if(LOG) logerror("Trident: FMIX set to %02x\n",data);
		break;
	case 0x08:  // Draw flags
		tri.accel_drawflags = (tri.accel_drawflags & 0xffffff00) | data;
		if(LOG) logerror("Trident: Draw flags set to %08x\n",tri.accel_drawflags);
		break;
	case 0x09:
		tri.accel_drawflags = (tri.accel_drawflags & 0xffff00ff) | (data << 8);
		if(LOG) logerror("Trident: Draw flags set to %08x\n",tri.accel_drawflags);
		break;
	case 0x0a:
		tri.accel_drawflags = (tri.accel_drawflags & 0xff00ffff) | (data << 16);
		if(LOG) logerror("Trident: Draw flags set to %08x\n",tri.accel_drawflags);
		break;
	case 0x0b:
		tri.accel_drawflags = (tri.accel_drawflags & 0x00ffffff) | (data << 24);
		if(LOG) logerror("Trident: Draw flags set to %08x\n",tri.accel_drawflags);
		break;
	case 0x0c:  // Foreground Colour
		tri.accel_fgcolour = (tri.accel_fgcolour & 0xffffff00) | data;
		if(LOG) logerror("Trident: Foreground Colour set to %08x\n",tri.accel_fgcolour);
		break;
	case 0x0d:
		tri.accel_fgcolour = (tri.accel_fgcolour & 0xffff00ff) | (data << 8);
		if(LOG) logerror("Trident: Foreground Colour set to %08x\n",tri.accel_fgcolour);
		break;
	case 0x0e:
		tri.accel_fgcolour = (tri.accel_fgcolour & 0xff00ffff) | (data << 16);
		if(LOG) logerror("Trident: Foreground Colour set to %08x\n",tri.accel_fgcolour);
		break;
	case 0x0f:
		tri.accel_fgcolour = (tri.accel_fgcolour & 0x00ffffff) | (data << 24);
		if(LOG) logerror("Trident: Foreground Colour set to %08x\n",tri.accel_fgcolour);
		break;
	case 0x10:  // Background Colour
		tri.accel_bgcolour = (tri.accel_bgcolour & 0xffffff00) | data;
		if(LOG) logerror("Trident: Background Colour set to %08x\n",tri.accel_bgcolour);
		break;
	case 0x11:
		tri.accel_bgcolour = (tri.accel_bgcolour & 0xffff00ff) | (data << 8);
		if(LOG) logerror("Trident: Background Colour set to %08x\n",tri.accel_bgcolour);
		break;
	case 0x12:
		tri.accel_bgcolour = (tri.accel_bgcolour & 0xff00ffff) | (data << 16);
		if(LOG) logerror("Trident: Background Colour set to %08x\n",tri.accel_bgcolour);
		break;
	case 0x13:
		tri.accel_bgcolour = (tri.accel_bgcolour & 0x00ffffff) | (data << 24);
		if(LOG) logerror("Trident: Background Colour set to %08x\n",tri.accel_bgcolour);
		break;
	case 0x14:  // Pattern Location
		tri.accel_pattern_loc = (tri.accel_pattern_loc & 0xff00) | data;
		if(LOG) logerror("Trident: Pattern Location set to %04x\n",tri.accel_pattern_loc);
		debugger_break(machine());
		break;
	case 0x15:
		tri.accel_pattern_loc = (tri.accel_pattern_loc & 0x00ff) | (data << 8);
		if(LOG) logerror("Trident: Pattern Location set to %04x\n",tri.accel_pattern_loc);
		debugger_break(machine());
		break;
	case 0x18:  // Destination X
		tri.accel_dest_x = (tri.accel_dest_x & 0xff00) | data;
		if(LOG) logerror("Trident: Destination X set to %04x\n",tri.accel_dest_x);
		break;
	case 0x19:
		tri.accel_dest_x = (tri.accel_dest_x & 0x00ff) | (data << 8);
		if(LOG) logerror("Trident: Destination X set to %04x\n",tri.accel_dest_x);
		break;
	case 0x1a:  // Destination Y
		tri.accel_dest_y = (tri.accel_dest_y & 0xff00) | data;
		if(LOG) logerror("Trident: Destination Y set to %04x\n",tri.accel_dest_y);
		break;
	case 0x1b:
		tri.accel_dest_y = (tri.accel_dest_y & 0x00ff) | (data << 8);
		if(LOG) logerror("Trident: Destination Y set to %04x\n",tri.accel_dest_y);
		break;
	case 0x1c:  // Source X
		tri.accel_source_x = (tri.accel_source_x & 0xff00) | data;
		if(LOG) logerror("Trident: Source X set to %04x\n",tri.accel_source_x);
		break;
	case 0x1d:
		tri.accel_source_x = (tri.accel_source_x & 0x00ff) | (data << 8);
		if(LOG) logerror("Trident: Source X set to %04x\n",tri.accel_source_x);
		break;
	case 0x1e:  // Source Y
		tri.accel_source_y = (tri.accel_source_y & 0xff00) | data;
		if(LOG) logerror("Trident: Source Y set to %04x\n",tri.accel_source_y);
		break;
	case 0x1f:
		tri.accel_source_y = (tri.accel_source_y & 0x00ff) | (data << 8);
		if(LOG) logerror("Trident: Source Y set to %04x\n",tri.accel_source_y);
		break;
	case 0x20:  // Dimension(?) X
		tri.accel_dim_x = (tri.accel_dim_x & 0xff00) | data;
		if(LOG) logerror("Trident: Dimension X set to %04x\n",tri.accel_dim_x);
		break;
	case 0x21:
		tri.accel_dim_x = (tri.accel_dim_x & 0x00ff) | (data << 8);
		if(LOG) logerror("Trident: Dimension X set to %04x\n",tri.accel_dim_x);
		break;
	case 0x22:  // Dimension(?) Y
		tri.accel_dim_y = (tri.accel_dim_y & 0xff00) | data;
		if(LOG) logerror("Trident: Dimension y set to %04x\n",tri.accel_dim_y);
		break;
	case 0x23:
		tri.accel_dim_y = (tri.accel_dim_y & 0x00ff) | (data << 8);
		if(LOG) logerror("Trident: Dimension y set to %04x\n",tri.accel_dim_y);
		break;
	case 0x24:  // Style
		tri.accel_style = (tri.accel_style & 0xffffff00) | data;
		if(LOG) logerror("Trident: Style set to %08x\n",tri.accel_style);
		break;
	case 0x25:
		tri.accel_style = (tri.accel_style & 0xffff00ff) | (data << 8);
		if(LOG) logerror("Trident: Style set to %08x\n",tri.accel_style);
		break;
	case 0x26:
		tri.accel_style = (tri.accel_style & 0xff00ffff) | (data << 16);
		if(LOG) logerror("Trident: Style set to %08x\n",tri.accel_style);
		break;
	case 0x27:
		tri.accel_style = (tri.accel_style & 0x00ffffff) | (data << 24);
		if(LOG) logerror("Trident: Style set to %08x\n",tri.accel_style);
		break;
	case 0x28:  // Source Clip X
		tri.accel_source_x_clip = (tri.accel_source_x_clip & 0xff00) | data;
		if(LOG) logerror("Trident: Source X Clip set to %04x\n",tri.accel_source_x_clip);
		break;
	case 0x29:
		tri.accel_source_x_clip = (tri.accel_source_x_clip & 0x00ff) | (data << 8);
		if(LOG) logerror("Trident: Source X Clip set to %04x\n",tri.accel_source_x_clip);
		break;
	case 0x2a:  // Source Clip Y
		tri.accel_source_y_clip = (tri.accel_source_y_clip & 0xff00) | data;
		if(LOG) logerror("Trident: Source Y Clip set to %04x\n",tri.accel_source_y_clip);
		break;
	case 0x2b:
		tri.accel_source_y_clip = (tri.accel_source_y_clip & 0x00ff) | (data << 8);
		if(LOG) logerror("Trident: Source Y Clip set to %04x\n",tri.accel_source_y_clip);
		break;
	case 0x2c:  // Destination Clip X
		tri.accel_dest_x_clip = (tri.accel_dest_x_clip & 0xff00) | data;
		if(LOG) logerror("Trident: Destination X Clip set to %04x\n",tri.accel_dest_x_clip);
		break;
	case 0x2d:
		tri.accel_dest_x_clip = (tri.accel_dest_x_clip & 0x00ff) | (data << 8);
		if(LOG) logerror("Trident: Destination X Clip set to %04x\n",tri.accel_dest_x_clip);
		break;
	case 0x2e:  // Destination Clip Y
		tri.accel_dest_y_clip = (tri.accel_dest_y_clip & 0xff00) | data;
		if(LOG) logerror("Trident: Destination Y Clip set to %04x\n",tri.accel_dest_y_clip);
		break;
	case 0x2f:
		tri.accel_dest_y_clip = (tri.accel_dest_y_clip & 0x00ff) | (data << 8);
		if(LOG) logerror("Trident: Destination Y Clip set to %04x\n",tri.accel_dest_y_clip);
		break;
	case 0x48:  // CKEY (Chromakey?)
		tri.accel_ckey = (tri.accel_ckey & 0xffffff00) | data;
		if(LOG) logerror("Trident: CKey set to %08x\n",tri.accel_ckey);
		break;
	case 0x49:
		tri.accel_ckey = (tri.accel_ckey & 0xffff00ff) | (data << 8);
		if(LOG) logerror("Trident: CKey set to %08x\n",tri.accel_ckey);
		break;
	case 0x4a:
		tri.accel_ckey = (tri.accel_ckey & 0xff00ffff) | (data << 16);
		if(LOG) logerror("Trident: CKey set to %08x\n",tri.accel_ckey);
		break;
	case 0x4b:
		tri.accel_ckey = (tri.accel_ckey & 0x00ffffff) | (data << 24);
		if(LOG) logerror("Trident: CKey set to %08x\n",tri.accel_ckey);
		break;
	case 0x58:  // Foreground Pattern Colour
		tri.accel_fg_pattern_colour = (tri.accel_fg_pattern_colour & 0xffffff00) | data;
		if(LOG) logerror("Trident: FG Pattern Colour set to %08x\n",tri.accel_fg_pattern_colour);
		break;
	case 0x59:
		tri.accel_fg_pattern_colour = (tri.accel_fg_pattern_colour & 0xffff00ff) | (data << 8);
		if(LOG) logerror("Trident: FG Pattern Colour set to %08x\n",tri.accel_fg_pattern_colour);
		break;
	case 0x5a:
		tri.accel_fg_pattern_colour = (tri.accel_fg_pattern_colour & 0xff00ffff) | (data << 16);
		if(LOG) logerror("Trident: FG Pattern Colour set to %08x\n",tri.accel_fg_pattern_colour);
		break;
	case 0x5b:
		tri.accel_fg_pattern_colour = (tri.accel_fg_pattern_colour & 0x00ffffff) | (data << 24);
		if(LOG) logerror("Trident: FG Pattern Colour set to %08x\n",tri.accel_fg_pattern_colour);
		break;
	case 0x5c:  // Background Pattern Colour
		tri.accel_bg_pattern_colour = (tri.accel_bg_pattern_colour & 0xffffff00) | data;
		if(LOG) logerror("Trident: BG Pattern Colour set to %08x\n",tri.accel_bg_pattern_colour);
		break;
	case 0x5d:
		tri.accel_bg_pattern_colour = (tri.accel_bg_pattern_colour & 0xffff00ff) | (data << 8);
		if(LOG) logerror("Trident: BG Pattern Colour set to %08x\n",tri.accel_bg_pattern_colour);
		break;
	case 0x5e:
		tri.accel_bg_pattern_colour = (tri.accel_bg_pattern_colour & 0xff00ffff) | (data << 16);
		if(LOG) logerror("Trident: BG Pattern Colour set to %08x\n",tri.accel_bg_pattern_colour);
		break;
	case 0x5f:
		tri.accel_bg_pattern_colour = (tri.accel_bg_pattern_colour & 0x00ffffff) | (data << 24);
		if(LOG) logerror("Trident: BG Pattern Colour set to %08x\n",tri.accel_bg_pattern_colour);
		break;
	default:
		if(LOG) logerror("Trident: unimplemented acceleration register offset %02x write %02x\n",offset,data);
	}
}

void trident_vga_device::accel_command()
{
	switch(tri.accel_command)
	{
	case 0x00:
		if(LOG) logerror("Trident: Command: NOP\n");
		break;
	case 0x01:
		if(LOG) logerror("Trident: Command: BitBLT ROP3 (Source %i,%i Dest %i,%i Size %i,%i)\n",tri.accel_source_x,tri.accel_source_y,tri.accel_dest_x,tri.accel_dest_y,tri.accel_dim_x,tri.accel_dim_y);
		logerror("BitBLT: Drawflags = %08x\n",tri.accel_drawflags);
		accel_bitblt();
		break;
	case 0x02:
		if(LOG) logerror("Trident: Command: BitBLT ROP4\n");
		break;
	case 0x03:
		if(LOG) logerror("Trident: Command: Scanline\n");
		break;
	case 0x04:
		if(LOG) logerror("Trident: Command: Bresenham Line (Source %i,%i Dest %i,%i Size %i,%i)\n",tri.accel_source_x,tri.accel_source_y,tri.accel_dest_x,tri.accel_dest_y,tri.accel_dim_x,tri.accel_dim_y);
		accel_line();
		break;
	case 0x05:
		if(LOG) logerror("Trident: Command: Short Vector\n");
		break;
	case 0x06:
		if(LOG) logerror("Trident: Command: Fast Line\n");
		break;
	case 0x07:
		if(LOG) logerror("Trident: Command: Trapezoid Fill\n");
		break;
	case 0x08:
		if(LOG) logerror("Trident: Command: Ellipse\n");
		break;
	case 0x09:
		if(LOG) logerror("Trident: Command: Ellipse Fill\n");
		break;
	default:
		logerror("Trident: Unknown acceleration command %02x\n",tri.accel_command);
	}
}

void trident_vga_device::accel_bitblt()
{
	int x,y;
	int sx,sy;
	int xdir,ydir;
	int xstart,xend,ystart,yend;

	if(tri.accel_drawflags & 0x0200)
	{
		xdir = -1;
		xstart = tri.accel_dest_x;
		xend = tri.accel_dest_x-tri.accel_dim_x-1;
	}
	else
	{
		xdir = 1;
		xstart = tri.accel_dest_x;
		xend = tri.accel_dest_x+tri.accel_dim_x+1;
	}
	if(tri.accel_drawflags & 0x0100)
	{
		ydir = -1;
		ystart = tri.accel_dest_y;
		yend = tri.accel_dest_y-tri.accel_dim_y-1;
	}
	else
	{
		ydir = 1;
		ystart = tri.accel_dest_y;
		yend = tri.accel_dest_y+tri.accel_dim_y+1;
	}
	sy = tri.accel_source_y;

//	printf("BitBLT: flags=%08x source %i, %i dest %i, %i size %i, %i dir %i,%i\n",
//			tri.accel_drawflags,tri.accel_source_x,tri.accel_source_y,tri.accel_dest_x,tri.accel_dest_y,
//			tri.accel_dim_x,tri.accel_dim_y,xdir,ydir);
	for(y=ystart;y!=yend;y+=ydir,sy+=ydir)
	{
		sx = tri.accel_source_x;
		for(x=xstart;x!=xend;x+=xdir,sx+=xdir)
		{
			if(tri.accel_drawflags & 0x4000)  // Solid fill
			{
				WRITEPIXEL8(x,y,tri.accel_fgcolour);
			}
			else
			{
				WRITEPIXEL8(x,y,READPIXEL8(sx,sy));
			}
		}
	}
}

void trident_vga_device::accel_line()
{
	UINT8 col = tri.accel_fgcolour & 0xff;
//    TGUI_SRC_XY(dmin-dmaj,dmin);
//    TGUI_DEST_XY(x,y);
//    TGUI_DIM_XY(dmin+e,len);
	INT16 dx = abs(tri.accel_source_x);
	INT16 dy = abs(tri.accel_source_y);
	INT16 err = tri.accel_dim_x - tri.accel_source_y;
	int sx = (tri.accel_drawflags & 0x0200) ? -1 : 1;
	int sy = (tri.accel_drawflags & 0x0100) ? -1 : 1;
	int count = 0;
	INT16 temp;

//	if(LOG_8514) logerror("8514/A: Command (%04x) - Line (Bresenham) - %i,%i  Axial %i, Diagonal %i, Error %i, Major Axis %i, Minor Axis %i\n",ibm8514.current_cmd,
//		ibm8514.curr_x,ibm8514.curr_y,ibm8514.line_axial_step,ibm8514.line_diagonal_step,ibm8514.line_errorterm,ibm8514.rect_width,ibm8514.rect_height);

	if(tri.accel_drawflags & 0x400)
	{
		temp = dx; dx = dy; dy = temp;
	}
	for(;;)
	{
		WRITEPIXEL8(tri.accel_dest_x,tri.accel_dest_y,col);
		if (count > tri.accel_dim_y) break;
		count++;
		if((err*2) > -dy)
		{
			err -= dy;
			tri.accel_dest_x += sx;
		}
		if((err*2) < dx)
		{
			err += dx;
			tri.accel_dest_y += sy;
		}
	}
}
