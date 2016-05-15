// license:BSD-3-Clause
// copyright-holders:Darius Goad
/*
 * nv4.c
 *
 * Implementation of the nVidia NV4 or RIVA TNT of video card
 *
 * Current status:
 * No acceleration will work until this is ported to PCI.
 */

#include "nv4.h"

#define CRTC_PORT_ADDR ((vga.miscellaneous_output&1)?0x3d0:0x3b0)
#define LOG_REG        1

const device_type NV4 = &device_creator<nv4_vga_device>;

nv4_vga_device::nv4_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: svga_device(mconfig, NV4, "nVidia NV4", tag, owner, clock, "nv4_vga", __FILE__)
{
}

nv4_vga_device::nv4_vga_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: svga_device(mconfig, type, name, tag, owner, clock, shortname, source)
{
}

void nv4_vga_device::device_start()
{
	zero();

	int i;
	for (i = 0; i < 0x100; i++)
		m_palette->set_pen_color(i, 0, 0, 0);

	// Avoid an infinite loop when displaying.  0 is not possible anyway.
	vga.crtc.maximum_scan_line = 1;

	// copy over interfaces
	vga.read_dipswitch = read8_delegate(); //read_dipswitch;
	vga.svga_intf.seq_regcount = 0x6;
	vga.svga_intf.crtc_regcount = 0x40;
	vga.svga_intf.vram_size = 0x1000000;
	vga.memory.resize(vga.svga_intf.vram_size);
	memset(&vga.memory[0], 0, vga.svga_intf.vram_size);
	save_item(vga.memory,"Video RAM");
	save_pointer(vga.crtc.data,"CRTC Registers",0x100);
	save_pointer(vga.sequencer.data,"Sequencer Registers",0x100);
	save_pointer(vga.attribute.data,"Attribute Registers", 0x15);

	m_vblank_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(vga_device::vblank_timer_cb),this));
}

void nv4_vga_device::device_reset()
{
	vga_device::device_reset();
}

UINT32 nv4_vga_device::nv4_pramdac_read(UINT32 addr)
{
	UINT8 res = 0;

	switch(addr)
	{
		case 0x680508:
			res = nv4.pramdac.vpll;
			break;
	}

	return res;
}

void nv4_vga_device::nv4_pramdac_write(UINT32 addr, UINT32 data)
{
	switch(addr)
	{
		case 0x680508:
			nv4.pramdac.vpll = data & 0x3ffff;
			nv4_define_video_mode();
			break;
	}
}

UINT32 nv4_vga_device::nv4_mmio_read(UINT32 addr)
{
	UINT8 res = 0;

	switch(addr)
	{
		case 0x680000 ... 0x680fff:
			res = nv4_pramdac_read(addr);
			break;
	}

	return res;
}

void nv4_vga_device::nv4_mmio_write(UINT32 addr, UINT32 data)
{
	switch(addr)
	{
		case 0x680000 ... 0x680fff:
			nv4_pramdac_write(addr, data);
			break;
	}
}

UINT8 nv4_vga_device::nv4_rma_read(int index)
{
	UINT8 res = 0;

	switch(index)
	{
		case 0x04:
			res = (nv4.rma.addr >> 0) & 0xff;
			break;
		case 0x05:
			res = (nv4.rma.addr >> 8) & 0xff;
			break;
		case 0x06:
			res = (nv4.rma.addr >> 16) & 0xff;
			break;
		case 0x07:
			res = (nv4.rma.addr >> 24) & 0xff;
			break;
		case 0x08:
			nv4.rma.data = nv4_mmio_read(nv4.rma.addr);
			res = (nv4.rma.data >> 0) & 0xff;
			break;
		case 0x09:
			res = (nv4.rma.data >> 8) & 0xff;
			break;
		case 0x0a:
			res = (nv4.rma.data >> 16) & 0xff;
			break;
		case 0x0b:
			res = (nv4.rma.data >> 24) & 0xff;
			break;
	}

	return res;
}

void nv4_vga_device::nv4_rma_write(int index, UINT8 data)
{
	switch(index)
	{
		case 0x04:
			nv4.rma.addr &= ~0x000000ff;
			nv4.rma.addr |= data << 0;
			break;
		case 0x05:
			nv4.rma.addr &= ~0x0000ff00;
			nv4.rma.addr |= data << 8;
			break;
		case 0x06:
			nv4.rma.addr &= ~0x00ff0000;
			nv4.rma.addr |= data << 16;
			break;
		case 0x07:
			nv4.rma.addr &= ~0xff000000;
			nv4.rma.addr |= data << 24;
			break;
		case 0x08:
			nv4.rma.data &= ~0x000000ff;
			nv4.rma.data |= data << 0;
			break;
		case 0x09:
			nv4.rma.data &= ~0x0000ff00;
			nv4.rma.data |= data << 8;
			break;
		case 0x0a:
			nv4.rma.data &= ~0x00ff0000;
			nv4.rma.data |= data << 16;
			break;
		case 0x0b:
			nv4.rma.data &= ~0xff000000;
			nv4.rma.data |= data << 24;
			nv4_mmio_write(nv4.rma.addr, nv4.rma.data);
			break;
	}
}

UINT8 nv4_vga_device::nv4_crtc_reg_read(UINT8 index)
{
	UINT8 res;

	if(index <= 0x18)
		res = crtc_reg_read(index);
	else
	{
		switch(index)
		{
			//Reading and writing of the I2C bit-banger is necessary to keep Linux and UNIVBE from hanging
			case 0x3e:
				res = (nv4.i2c.sda << 3) | (nv4.i2c.scl << 2);
				break;
			default:
				res = vga.crtc.data[index];
				//debugger_break(machine);
				//printf("%02x\n",index);
				break;
		}
	}

	return res;
}

void nv4_vga_device::nv4_define_video_mode()
{
	int divisor = 1;
	int xtal = (vga.miscellaneous_output & 0xc) ? XTAL_28_63636MHz : XTAL_25_1748MHz;

	if((vga.miscellaneous_output & 0xc) == 0x0c)
	{
		UINT8 m,n,p;
		m = nv4.pramdac.vpll & 0xff;
		n = (nv4.pramdac.vpll >> 8) & 0xff;
		p = (nv4.pramdac.vpll >> 16) & 3;

		xtal = 13500;
		if(m == 0) xtal = 1;
		else xtal = (xtal * n) / (1 << p) / m;
	}

	svga.rgb8_en = 0;
	svga.rgb15_en = 0;
	svga.rgb16_en = 0;
	svga.rgb32_en = 0;

	switch(vga.crtc.data[0x28] & 3)
	{
		case 1:
			svga.rgb8_en = 1;
			break;
		case 2:
			svga.rgb16_en = 1;
			break;
		case 3:
			svga.rgb32_en = 1;
			break;
	}

	recompute_params_clock(divisor, xtal);
}

void nv4_vga_device::nv4_crtc_reg_write(UINT8 index, UINT8 data)
{
	if(index <= 0x18)
	{
		crtc_reg_write(index,data);
		nv4_define_video_mode();
	}
	else
	{
		switch(index)
		{
			case 0x19:
				vga.crtc.start_addr_latch &= 0xffff;
				vga.crtc.start_addr_latch |= ((data & 0x1f) << 16);
				vga.crtc.offset &= 0xff;
				vga.crtc.offset |= ((data & 0xe0) << 3);
				nv4_define_video_mode();
				break;
			case 0x1d:
				svga.bank_w = data;
				break;
			case 0x1e:
				svga.bank_r = data;
				break;
			case 0x25:
				vga.crtc.vert_total         &= 0x3ff;
				vga.crtc.vert_disp_end      &= 0x3ff;
				vga.crtc.vert_blank_start   &= 0x3ff;
				vga.crtc.vert_retrace_start &= 0x3ff;
				vga.crtc.horz_total         &=  0xff;
				if(data & 0x01) vga.crtc.vert_total         |= 0x400;
				if(data & 0x02) vga.crtc.vert_disp_end      |= 0x400;
				if(data & 0x04) vga.crtc.vert_blank_start   |= 0x400;
				if(data & 0x08) vga.crtc.vert_retrace_start |= 0x400;
				if(data & 0x10) vga.crtc.horz_total         |= 0x100;
				nv4_define_video_mode();
				break;
			case 0x28:
				vga.crtc.data[index] = data;
				nv4_define_video_mode();
				break;
			case 0x2d:
				vga.crtc.horz_total &= 0xff;
				if(data & 0x01) vga.crtc.horz_disp_end |= 0x100;
				nv4_define_video_mode();
				break;
			case 0x38:
				nv4.rma.mode = data & 7;
				break;
			//Reading and writing of the I2C bit-banger is necessary to keep Linux and UNIVBE from hanging
			case 0x3f:
				nv4.i2c.scl = data & 0x20;
				nv4.i2c.sda = data & 0x10;
				break;
			default:
				if(LOG_REG) logerror("NV4: CR%02X write %02x\n",index,data);
				break;
		}
	}
}


READ8_MEMBER(nv4_vga_device::port_03b0_r)
{
	UINT8 res = 0xff;

	if (CRTC_PORT_ADDR == 0x3b0)
	{
		switch(offset)
		{
			case 5:
				res = nv4_crtc_reg_read(vga.crtc.index);
				break;
			default:
				res = vga_device::port_03b0_r(space,offset,mem_mask);
				break;
		}
	}

	return res;
}

WRITE8_MEMBER(nv4_vga_device::port_03b0_w)
{
	if (CRTC_PORT_ADDR == 0x3b0)
	{
		switch(offset)
		{
			case 5:
				vga.crtc.data[vga.crtc.index] = data;
				nv4_crtc_reg_write(vga.crtc.index,data);
				break;
			default:
				vga_device::port_03b0_w(space,offset,data,mem_mask);
				break;
		}
	}
}

READ8_MEMBER(nv4_vga_device::port_03d0_r)
{
	UINT8 res = 0xff;

	switch(offset)
	{
		case 0:
			if(nv4.rma.mode & 1) res = nv4_rma_read(((nv4.rma.mode & 6) << 1) + 0);
			break;
		case 1:
			if(nv4.rma.mode & 1) res = nv4_rma_read(((nv4.rma.mode & 6) << 1) + 1);
			break;
		case 2:
			if(nv4.rma.mode & 1) res = nv4_rma_read(((nv4.rma.mode & 6) << 1) + 2);
			break;
		case 3:
			if(nv4.rma.mode & 1) res = nv4_rma_read(((nv4.rma.mode & 6) << 1) + 3);
			break;
	}

	if (CRTC_PORT_ADDR == 0x3d0)
	{
		switch(offset)
		{
			case 5:
				res = nv4_crtc_reg_read(vga.crtc.index);
				break;
			default:
				res = vga_device::port_03d0_r(space,offset,mem_mask);
				break;
		}
	}

	return res;
}

WRITE8_MEMBER(nv4_vga_device::port_03d0_w)
{
	switch(offset)
	{
		case 0:
			if(nv4.rma.mode & 1) nv4_rma_write(((nv4.rma.mode & 6) << 1) + 0, data);
			break;
		case 1:
			if(nv4.rma.mode & 1) nv4_rma_write(((nv4.rma.mode & 6) << 1) + 1, data);
			break;
		case 2:
			if(nv4.rma.mode & 1) nv4_rma_write(((nv4.rma.mode & 6) << 1) + 2, data);
			break;
		case 3:
			if(nv4.rma.mode & 1) nv4_rma_write(((nv4.rma.mode & 6) << 1) + 3, data);
			break;
	}
	if (CRTC_PORT_ADDR == 0x3d0)
	{
		switch(offset)
		{
			case 5:
				vga.crtc.data[vga.crtc.index] = data;
				nv4_crtc_reg_write(vga.crtc.index,data);
				break;
			default:
				vga_device::port_03d0_w(space,offset,data,mem_mask);
				break;
		}
	}
}

READ8_MEMBER(nv4_vga_device::mem_r)
{
	if (svga.rgb8_en || svga.rgb15_en || svga.rgb16_en || svga.rgb32_en)
	{
		UINT8 data;
		data = 0xff;
		if(vga.sequencer.data[4] & 0x8)
		{
			if(offset + (svga.bank_r*0x8000) < vga.svga_intf.vram_size)
				data = vga.memory[offset + (svga.bank_r*0x8000)];
		}
		else
		{
			int i;

			for(i=0;i<4;i++)
			{
				if(vga.sequencer.map_mask & 1 << i)
				{
					if(offset*4+i+(svga.bank_r*0x8000) < vga.svga_intf.vram_size)
						data |= vga.memory[offset*4+i+(svga.bank_r*0x8000)];
				}
			}
		}
		return data;
	}
	if((offset + (svga.bank_r*0x8000)) < vga.svga_intf.vram_size)
		return vga_device::mem_r(space,offset,mem_mask);
	else
		return 0xff;
}

WRITE8_MEMBER(nv4_vga_device::mem_w)
{
	if (svga.rgb8_en || svga.rgb15_en || svga.rgb16_en || svga.rgb32_en)
	{
	//  printf("%08x %02x (%02x %02x) %02X\n",offset,data,vga.sequencer.map_mask,svga.bank_w,(vga.sequencer.data[4] & 0x08));
		if(vga.sequencer.data[4] & 0x8)
		{
			if((offset + (svga.bank_w*0x8000)) < vga.svga_intf.vram_size)
				vga.memory[(offset + (svga.bank_w*0x8000))] = data;
		}
		else
		{
			int i;
			for(i=0;i<4;i++)
			{
				if(vga.sequencer.map_mask & 1 << i)
				{
					if((offset*4+i+(svga.bank_w*0x8000)) < vga.svga_intf.vram_size)
						vga.memory[(offset*4+i+(svga.bank_w*0x8000))] = data;
				}
			}
		}
		return;
	}

	if((offset + (svga.bank_w*0x8000)) < vga.svga_intf.vram_size)
		vga_device::mem_w(space,offset,data,mem_mask);
}
