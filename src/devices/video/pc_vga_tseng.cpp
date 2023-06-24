// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * Tseng Labs VGA family
 *
 * TODO:
 * - ET3000 (VGA only?)
 * - ET4000AX
 * \- No logging whatsoever;
 * \- Unsupported True Color modes;
 * - ET4000/W32 (2d accelerator)
 * - ET4000/W32p (PCI version)
 *
 */

#include "emu.h"
#include "pc_vga_tseng.h"

// TODO: refactor this macro
#define GRAPHIC_MODE (vga.gc.alpha_dis) /* else text mode */

DEFINE_DEVICE_TYPE(TSENG_VGA,  tseng_vga_device,  "tseng_vga",  "Tseng Labs ET4000AX SVGA")

tseng_vga_device::tseng_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: svga_device(mconfig, TSENG_VGA, tag, owner, clock)
{
}

void tseng_vga_device::device_start()
{
	svga_device::device_start();
	memset(&et4k, 0, sizeof(et4k));

	save_item(NAME(et4k.reg_3d8));
	save_item(NAME(et4k.dac_ctrl));
	save_item(NAME(et4k.dac_state));
	save_item(NAME(et4k.horz_overflow));
	save_item(NAME(et4k.aux_ctrl));
	save_item(NAME(et4k.ext_reg_ena));
	save_item(NAME(et4k.misc1));
	save_item(NAME(et4k.misc2));
}

void tseng_vga_device::tseng_define_video_mode()
{
	int divisor;
	int xtal = 0;
	svga.rgb8_en = 0;
	svga.rgb15_en = 0;
	svga.rgb16_en = 0;
	svga.rgb24_en = 0;
	switch(((et4k.aux_ctrl << 1) & 4)|(vga.miscellaneous_output & 0xc)>>2)
	{
		case 0:
			xtal = XTAL(25'174'800).value();
			break;
		case 1:
			xtal = XTAL(28'636'363).value();
			break;
		case 2:
			xtal = 16257000*2; //2xEGA clock
			break;
		case 3:
			xtal = XTAL(40'000'000).value();
			break;
		case 4:
			xtal = XTAL(36'000'000).value();
			break;
		case 5:
			xtal = XTAL(45'000'000).value();
			break;
		case 6:
			xtal = 31000000;
			break;
		case 7:
			xtal = 38000000;
			break;
	}
	switch(et4k.dac_ctrl & 0xe0)
	{
		case 0xa0:
			svga.rgb15_en = 1;
			divisor = 2;
			break;
		case 0xe0:
			svga.rgb16_en = 1;
			divisor = 2;
			break;
		case 0x60:
			svga.rgb24_en = 1;
			divisor = 3;
			xtal *= 2.0f/3.0f;
			break;
		default:
			svga.rgb8_en = (!(vga.sequencer.data[1] & 8) && (vga.sequencer.data[4] & 8) && vga.gc.shift256 && vga.crtc.div2 && GRAPHIC_MODE);
			divisor = 1;
			break;
	}
	recompute_params_clock(divisor, xtal);
}

uint8_t tseng_vga_device::tseng_crtc_reg_read(uint8_t index)
{
	uint8_t res;

	if(index <= 0x18)
		res = crtc_reg_read(index);
	else
	{
		switch(index)
		{
			case 0x34:
				res = et4k.aux_ctrl;
				break;
			case 0x3f:
				res = et4k.horz_overflow;
				break;
			default:
				res = vga.crtc.data[index];
				//printf("%02x\n",index);
				break;
		}
	}

	return res;
}

void tseng_vga_device::tseng_crtc_reg_write(uint8_t index, uint8_t data)
{
	if(index <= 0x18)
		crtc_reg_write(index,data);
	else
	{
		switch(index)
		{
			case 0x34:
				et4k.aux_ctrl = data;
				break;
			case 0x3f:
				et4k.horz_overflow = data;
				vga.crtc.horz_total = (vga.crtc.horz_total & 0xff) | ((data & 1) << 8);
				break;
			default:
				//printf("%02x %02x\n",index,data);
				break;
		}
	}
}

uint8_t tseng_vga_device::tseng_seq_reg_read(uint8_t index)
{
	uint8_t res;

	res = 0xff;

	if(index <= 0x04)
		res = vga.sequencer.data[index];
	else
	{
		switch(index)
		{
			case 0x06:
			case 0x07:
				//printf("%02x\n",index);
				break;
		}
	}

	return res;
}

void tseng_vga_device::tseng_seq_reg_write(uint8_t index, uint8_t data)
{
	if(index <= 0x04)
	{
		vga.sequencer.data[vga.sequencer.index] = data;
		seq_reg_write(vga.sequencer.index,data);
	}
	else
	{
		switch(index)
		{
			case 0x06:
			case 0x07:
				//printf("%02x %02x\n",index,data);
				break;
		}
	}
}

uint8_t tseng_vga_device::port_03b0_r(offs_t offset)
{
	uint8_t res = 0xff;

	if (get_crtc_port() == 0x3b0)
	{
		switch(offset)
		{
			case 5:
				res = tseng_crtc_reg_read(vga.crtc.index);
				break;
			case 8:
				res = et4k.reg_3d8;
				break;
			default:
				res = vga_device::port_03b0_r(offset);
				break;
		}
	}

	return res;
}

void tseng_vga_device::port_03b0_w(offs_t offset, uint8_t data)
{
	if (get_crtc_port() == 0x3b0)
	{
		switch(offset)
		{
			case 5:
				vga.crtc.data[vga.crtc.index] = data;
				tseng_crtc_reg_write(vga.crtc.index,data);
				break;
			case 8:
				et4k.reg_3d8 = data;
				if(data == 0xa0)
					et4k.ext_reg_ena = true;
				else if(data == 0x29)
					et4k.ext_reg_ena = false;
				break;
			default:
				vga_device::port_03b0_w(offset,data);
				break;
		}
	}
	tseng_define_video_mode();
}

void tseng_vga_device::tseng_attribute_reg_write(uint8_t index, uint8_t data)
{
	switch(index)
	{
		case 0x16:
			et4k.misc1 = data;
			#if 0
			svga.rgb8_en = 0;
			svga.rgb15_en = 0;
			svga.rgb16_en = 0;
			svga.rgb32_en = 0;
			/* TODO: et4k and w32 are different here */
			switch(et4k.misc1 & 0x30)
			{
				case 0:
					// normal power-up mode
					break;
				case 0x10:
					svga.rgb8_en = 1;
					break;
				case 0x20:
				case 0x30:
					popmessage("Tseng 15/16 bit HiColor mode, contact MAMEdev");
					break;
			}
			#endif
			break;
		case 0x17: et4k.misc2 = data; break;
		default:
			attribute_reg_write(index,data);
	}

}

uint8_t tseng_vga_device::port_03c0_r(offs_t offset)
{
	uint8_t res;

	switch(offset)
	{
		case 0x01:
			switch(vga.attribute.index)
			{
				case 0x16: res = et4k.misc1; break;
				case 0x17: res = et4k.misc2; break;
				default:
					res = vga_device::port_03c0_r(offset);
					break;
			}

			break;

		case 0x05:
			res = tseng_seq_reg_read(vga.sequencer.index);
			break;
		case 0x0d:
			res = svga.bank_w & 0xf;
			res |= (svga.bank_r & 0xf) << 4;
			break;
		case 0x06:
			if(et4k.dac_state == 4)
			{
				if(!et4k.dac_ctrl)
					et4k.dac_ctrl = 0x80;
				res = et4k.dac_ctrl;
				break;
			}
			et4k.dac_state++;
			res = vga_device::port_03c0_r(offset);
			break;
		case 0x08:
			et4k.dac_state = 0;
			[[fallthrough]];
		default:
			res = vga_device::port_03c0_r(offset);
			break;
	}

	return res;
}

void tseng_vga_device::port_03c0_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
		case 0:
			if (vga.attribute.state==0)
			{
				vga.attribute.index=data;
			}
			else
			{
				tseng_attribute_reg_write(vga.attribute.index,data);
			}
			vga.attribute.state=!vga.attribute.state;
			break;

		case 0x05:
			tseng_seq_reg_write(vga.sequencer.index,data);
			break;
		case 0x0d:
			svga.bank_w = data & 0xf;
			svga.bank_r = (data & 0xf0) >> 4;
			break;
		case 0x06:
			if(et4k.dac_state == 4)
			{
				et4k.dac_ctrl = data;
				break;
			}
			[[fallthrough]];
		default:
			vga_device::port_03c0_w(offset,data);
			break;
	}
	tseng_define_video_mode();
}

uint8_t tseng_vga_device::port_03d0_r(offs_t offset)
{
	uint8_t res = 0xff;

	if (get_crtc_port() == 0x3d0)
	{
		switch(offset)
		{
			case 5:
				res = tseng_crtc_reg_read(vga.crtc.index);
				break;
			case 8:
				res = et4k.reg_3d8;
				break;
			default:
				res = vga_device::port_03d0_r(offset);
				break;
		}
	}

	return res;
}

void tseng_vga_device::port_03d0_w(offs_t offset, uint8_t data)
{
	if (get_crtc_port() == 0x3d0)
	{
		switch(offset)
		{
			case 5:
				vga.crtc.data[vga.crtc.index] = data;
				tseng_crtc_reg_write(vga.crtc.index,data);
				//if((vga.crtc.index & 0xfe) != 0x0e)
				//  printf("%02x %02x %d\n",vga.crtc.index,data,screen().vpos());
				break;
			case 8:
				et4k.reg_3d8 = data;
				if(data == 0xa0)
					et4k.ext_reg_ena = true;
				else if(data == 0x29)
					et4k.ext_reg_ena = false;
				break;
			default:
				vga_device::port_03d0_w(offset,data);
				break;
		}
	}
	tseng_define_video_mode();
}

uint8_t tseng_vga_device::mem_r(offs_t offset)
{
	if(svga.rgb8_en || svga.rgb15_en || svga.rgb16_en || svga.rgb24_en)
	{
		offset &= 0xffff;
		return vga.memory[(offset+svga.bank_r*0x10000)];
	}

	return vga_device::mem_r(offset);
}

void tseng_vga_device::mem_w(offs_t offset, uint8_t data)
{
	if(svga.rgb8_en || svga.rgb15_en || svga.rgb16_en || svga.rgb24_en)
	{
		offset &= 0xffff;
		vga.memory[(offset+svga.bank_w*0x10000)] = data;
	}
	else
		vga_device::mem_w(offset,data);
}
