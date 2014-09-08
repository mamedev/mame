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
			case 0x0b:
				res = svga.id;
				// TODO: new mode registers selected
				break;
			case 0x0d:
				res = svga.rgb15_en;
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
		switch(index)
		{
			case 0x0b:
				// TODO: old mode registers selected
				break;
			case 0x0d:
				svga.rgb15_en = data & 0x30; // TODO: doesn't match documentation
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
			case 8:
				res = svga.bank_w & 0x1f; // TODO: a lot more complex than this
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
			case 8:
				svga.bank_w = data & 0x1f; // TODO: a lot more complex than this
				break;
			default:
				vga_device::port_03d0_w(space,offset,data,mem_mask);
				break;
		}
	}
}

READ8_MEMBER(trident_vga_device::mem_r )
{
	if (svga.rgb15_en & 0x30)
	{
		int data;
		if(offset & 0x10000)  // TODO: old reg mode actually CAN read to the upper bank
			return 0;
		data=vga.memory[offset + (svga.bank_w*0x10000)];
		return data;
	}

	return vga_device::mem_r(space,offset,mem_mask);
}

WRITE8_MEMBER(trident_vga_device::mem_w)
{
	if (svga.rgb15_en & 0x30)
	{
		if(offset & 0x10000) // TODO: old reg mode actually CAN write to the upper bank
			return;
		vga.memory[offset + (svga.bank_w*0x10000)]= data;
		return;
	}

	vga_device::mem_w(space,offset,data,mem_mask);
}

