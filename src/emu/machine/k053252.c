/***************************************************************************************************************************

    Konami 053252 chip emulation, codenamed "CCU"

    device emulation by Angelo Salese, based off notes by Olivier Galibert

============================================================================================================================

left res = current in game, right res = computed

hexion:    02 FF 00 4D 00 73 00 00 01 1F 05 0E B7 7C 00 00 512x256 ~ 512x256 <- writes to e and f regs, in an irq ack fashion
overdriv:  01 7F 00 22 00 0D 00 03 01 07 10 0F 73 00 00 00 304x256 ~ 305x224
esckids:   01 7F 00 12 00 0D 00 01 01 07 08 07 73 00 00 00 304x224 ~ 321x240
rollerg:   01 7F 00 23 00 1D 02 00 01 07 10 0F 73 00 02 00 288x224 ~ 288x224 <- writes to 6 and e regs, in an irq ack fashion
gaiapols:  01 FB 00 19 00 37 00 00 01 06 10 0E 75 00 D1 00 376x224 ~ 380x224
mmaulers:  01 7F 00 19 00 27 00 00 01 07 10 0F 73 00 00 00 288x224 ~ 288x224
mystwarr:  01 7F 00 12 00 2E 00 00 01 07 11 0E 73 00 00 00 288x224 ~ 288x224
metamrph:  01 7F 00 11 00 27 01 00 01 07 10 0F 74 00 00 00 288x224 ~ 288x224
viostorm:  01 FF 00 16 00 39 00 00 01 07 11 0E 75 00 00 00 384x224 ~ 385x224
mtlchamp:  01 FF 00 21 00 37 00 00 01 07 11 0E 74 00 00 00 384x224 ~ 384x224
dbz:       01 FF 00 21 00 37 00 00 01 20 0C 0E 54 00 00 00 384x256 ~ 384x256
dbz2:      01 FF 00 21 00 37 00 00 01 20 0C 0E 54 00 00 00 384x256 ~ 384x256
xexex:     01 FF 00 21 00 37 01 00 00 20 0C 0E 54 00 00 00 384x256 ~ 384x256 (*)
(all konamigx, cowboys of moo mesa, run & gun, dj main)

(*) hblank duration 512 (0x200), hdisp 384 (0x180), vblank duration 288 (0x120), vdisp 256 (0x100)

     Definitions from GX, look similar, all values big-endian, write-only:

    0-1: bits 9-0: HC        - Total hblank duration (-1)     Hres ~ (HC+1) - HFP - HBP - 8*(HSW+1)
    2-3: bits 8-0: HFP       - HBlank front porch
    4-5: bits 8-0: HBP       - HBlank back porch
    6  : bits 7-0: INT1EN
    7  : bits 7-0: INT2EN
    8-9: bits 8-0: VC        - Total vblank duration
    a  : bits 7-0: VFP       - VBlank front porch             Vres ~ VC - VFP - VBP - (VSW+1)
    b  : bits 7-0: VBP       - VBlank back porch
    c  : bits 7-4: VSW       - V-Sync Width
    c  : bits 3-0: HSW       - H-Sync Width
    d  : bits 7-0: INT-TIME
    e  : bits 7-0: INT1ACK
    f  : bits 7-0: INT2ACK

     Read-only:
    e-f: bits 8-0: VCT

TODO:
- xexex sets up 0x20 as the VC? default value?
- xexex layers are offsetted if you try to use the CCU
- understand how to interpret the back / front porch values, and remove the offset x/y hack

***************************************************************************************************************************/


#include "emu.h"
#include "k053252.h"

struct k053252_state
{
	UINT8   regs[16];
	UINT16  hc,hfp,hbp;
	UINT16  vc,vfp,vbp;
	UINT8   vsw,hsw;

	screen_device *screen;
	devcb_resolved_write_line int1_en;
	devcb_resolved_write_line int2_en;
	devcb_resolved_write_line int1_ack;
	devcb_resolved_write_line int2_ack;
	//devcb_resolved_write8     int_time;
	int offsx,offsy;
};


/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE k053252_state *k053252_get_safe_token( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == K053252);

	return (k053252_state *)downcast<k053252_device *>(device)->token();
}

INLINE const k053252_interface *k053252_get_interface( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == K053252);

	return (const k053252_interface *) device->static_config();
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

READ8_DEVICE_HANDLER( k053252_r )
{
	k053252_state *k053252 = k053252_get_safe_token(device);

	//TODO: debugger_access()
	popmessage("Warning: k053252 read %02x, contact MAMEdev",offset);

	return k053252->regs[offset];
}

static void k053252_res_change( device_t *device )
{
	k053252_state *k053252 = k053252_get_safe_token(device);

	if(k053252->screen != NULL)
	{
		if(k053252->hc && k053252->vc &&
		   k053252->hbp && k053252->hfp &&
		   k053252->vbp && k053252->vfp &&
		   k053252->hsw && k053252->vsw) //safety checks
		{
			rectangle visarea;
			//(HC+1) - HFP - HBP - 8*(HSW+1)
			//VC - VFP - VBP - (VSW+1)
			attoseconds_t refresh = HZ_TO_ATTOSECONDS(device->clock()) * (k053252->hc) * k053252->vc;

			//printf("H %d %d %d %d\n",k053252->hc,k053252->hfp,k053252->hbp,k053252->hsw);
			//printf("V %d %d %d %d\n",k053252->vc,k053252->vfp,k053252->vbp,k053252->vsw);

			visarea.min_x = k053252->offsx;
			visarea.min_y = k053252->offsy;
			visarea.max_x = k053252->offsx + k053252->hc - k053252->hfp - k053252->hbp - 8*(k053252->hsw) - 1;
			visarea.max_y = k053252->offsy + k053252->vc - k053252->vfp - k053252->vbp - (k053252->vsw) - 1;

			k053252->screen->configure(k053252->hc, k053252->vc, visarea, refresh);
		}
	}
}

WRITE8_DEVICE_HANDLER( k053252_w )
{
	k053252_state *k053252 = k053252_get_safe_token(device);

	k053252->regs[offset] = data;

	switch(offset)
	{
		case 0x00:
		case 0x01:
			k053252->hc  = (k053252->regs[1]&0xff);
			k053252->hc |= ((k053252->regs[0]&0x03)<<8);
			k053252->hc ++;
			logerror("%d (%04x) HC set\n",k053252->hc,k053252->hc);
			k053252_res_change(device);
			break;
		case 0x02:
		case 0x03:
			k053252->hfp  = (k053252->regs[3]&0xff);
			k053252->hfp |= ((k053252->regs[2]&0x01)<<8);
			logerror("%d (%04x) HFP set\n",k053252->hfp,k053252->hfp);
			k053252_res_change(device);
			break;
		case 0x04:
		case 0x05:
			k053252->hbp  = (k053252->regs[5]&0xff);
			k053252->hbp |= ((k053252->regs[4]&0x01)<<8);
			logerror("%d (%04x) HBP set\n",k053252->hbp,k053252->hbp);
			k053252_res_change(device);
			break;
		case 0x06: k053252->int1_en(data); break;
		case 0x07: k053252->int2_en(data); break;
		case 0x08:
		case 0x09:
			k053252->vc  = (k053252->regs[9]&0xff);
			k053252->vc |= ((k053252->regs[8]&0x01)<<8);
			logerror("%d (%04x) VC set\n",k053252->vc,k053252->vc);
			k053252_res_change(device);
			break;
		case 0x0a:
			k053252->vfp  = (k053252->regs[0x0a]&0xff);
			logerror("%d (%04x) VFP set\n",k053252->vfp,k053252->vfp);
			k053252_res_change(device);
			break;
		case 0x0b:
			k053252->vbp  = (k053252->regs[0x0b]&0xff);
			logerror("%d (%04x) VBP set\n",k053252->vbp,k053252->vbp);
			k053252_res_change(device);
			break;
		case 0x0c:
			k053252->vsw  = ((k053252->regs[0x0c]&0xf0) >> 4) + 1;
			k053252->hsw  = ((k053252->regs[0x0c]&0x0f) >> 0) + 1;
			logerror("%02x VSW / %02x HSW set\n",k053252->vsw,k053252->hsw);
			k053252_res_change(device);
			break;
		//case 0x0d: k053252->int_time(data); break;
		case 0x0e: k053252->int1_ack(1); break;
		case 0x0f: k053252->int2_ack(1); break;
	}
}



/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( k053252 )
{
	k053252_state *k053252 = k053252_get_safe_token(device);
	const k053252_interface *intf = k053252_get_interface(device);

	device->save_item(NAME(k053252->regs));
	k053252->screen = device->machine().device<screen_device>(intf->screen);
	k053252->int1_en.resolve(intf->int1_en, *device);
	k053252->int2_en.resolve(intf->int2_en, *device);
	k053252->int1_ack.resolve(intf->int1_ack, *device);
	k053252->int2_ack.resolve(intf->int2_ack, *device);
	//k053252->int_time.resolve(intf->int_time, *device);
	k053252->offsx = intf->offsx;
	k053252->offsy = intf->offsy;
}

static DEVICE_RESET( k053252 )
{
	k053252_state *k053252 = k053252_get_safe_token(device);
	int i;

	for (i = 0; i < 16; i++)
		k053252->regs[i] = 0;

	k053252->regs[0x08] = 1; // Xexex apparently does a wrong assignment for VC (sets up the INT enable register instead)
}


const device_type K053252 = &device_creator<k053252_device>;

k053252_device::k053252_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, K053252, "Konami 053252", tag, owner, clock)
{
	m_token = global_alloc_clear(k053252_state);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void k053252_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k053252_device::device_start()
{
	DEVICE_START_NAME( k053252 )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k053252_device::device_reset()
{
	DEVICE_RESET_NAME( k053252 )(this);
}



