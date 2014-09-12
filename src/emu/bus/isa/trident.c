/*
 * trident.c
 *
 * Implementation of Trident VGA GUI accelerators
 *
 *
 */

#include "emu.h"
#include "trident.h"

const device_type TRIDENT_VGA = &device_creator<trident_vga_device>;

#define CRTC_PORT_ADDR ((vga.miscellaneous_output&1)?0x3d0:0x3b0)

trident_vga_device::trident_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: svga_device(mconfig, TRIDENT_VGA, "Trident TGUI9680", tag, owner, clock, "trident_vga", __FILE__)
{
}

void trident_vga_device::device_start()
{
	svga_device::device_start();
	vga.svga_intf.seq_regcount = 0x0f;
	vga.svga_intf.crtc_regcount = 0x50;
	memset(&tri, 0, sizeof(tri));
}

void trident_vga_device::device_reset()
{
	svga_device::device_reset();
	svga.id = 0xd3;  // identifies at TGUI9660XGi
	tri.revision = 0x01;  // revision identifies as TGUI9680
	tri.new_mode = false;  // start up in old mode
	tri.dac_active = false;
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
				break;
			case 0x0c:  // Power Up Mode register 1
				res = tri.sr0c & 0xef;
				if(tri.port_3c3)
					res |= 0x10;
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
		logerror("Trident SR%02X: %s mode write %02x\n",index,tri.new_mode ? "new" : "old",data);
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
					svga.bank_w = (data & 0x0f) ^ 0x02;  // bit 1 is inverted, used for card detection, it is not XORed on reading
					if(!(tri.gc0f & 0x01))
						svga.bank_r = (data & 0x0f) ^ 0x02;
					// TODO: handle planar modes, where bits 0 and 2 only are used
				}
				else
				{
					tri.sr0e_old = data;
					svga.bank_w = data & 0x06;
					if(!(tri.gc0f & 0x01))
						svga.bank_r = data & 0x06;
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
		case 0x1f:
			res = tri.cr1f;
			break;
		case 0x38:
			res = tri.pixel_depth;
			break;
		default:
			res = vga.crtc.data[index];
			break;
		}
	}
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
		logerror("Trident CR%02X: write %02x\n",index,data);
		switch(index)
		{
		case 0x1f:
			tri.cr1f = data;  // "Software Programming Register"  written to by software (BIOS?)
			break;
		case 0x38:
			tri.pixel_depth = data;
			trident_define_video_mode();
			break;
		default:
			logerror("Trident: 3D4 index %02x write %02x\n",index,data);
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
		default:
			res = 0xff;
			break;
		}
	}
	return res;
}

void trident_vga_device::trident_gc_reg_write(UINT8 index, UINT8 data)
{
	if(index <= 0x0d)
		gc_reg_write(index,data);
	else
	{
		logerror("Trident GC%02X: write %02x\n",index,data);
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
		default:
			logerror("Trident: Unimplemented GC register %02x write %02x\n",index,data);
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
					res = svga.bank_w & 0x1f;
				else
					res = 0xff;
				break;
			case 9:
				if(tri.gc0f & 0x04)  // if enabled
					if(tri.gc0f & 0x01)  // and if bank regs are separated
						res = svga.bank_r & 0x1f;
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
					svga.bank_w = data & 0x1f;
					if(!(tri.gc0f & 0x01))  // if bank regs are not separated
						svga.bank_r = data & 0x1f; // then this is also the read bank register
				}
				break;
			case 9:
				if(tri.gc0f & 0x04)  // if enabled
				{
					if(tri.gc0f & 0x01)  // and if bank regs are separated
						svga.bank_r = data & 0x1f;
				}
				break;
			default:
				vga_device::port_03d0_w(space,offset,data,mem_mask);
				break;
		}
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

