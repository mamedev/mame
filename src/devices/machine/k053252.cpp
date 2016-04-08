// license:LGPL-2.1+
// copyright-holders:Angelo Salese
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

(*) hcount total 512 (0x200), hdisp 384 (0x180), vcount total 289 (0x121), vdisp 256 (0x100)

     Definitions from GX, look similar, all values big-endian, write-only:

    0-1: bits 9-0: HC        - Total horizontal count (-1)  Hres ~ (HC+1) - HFP - HBP - 8*(HSW+1)
    2-3: bits 8-0: HFP       - HBlank front porch
    4-5: bits 8-0: HBP       - HBlank back porch
    6  : bits 7-0: INT1EN
    7  : bits 7-0: INT2EN
    8-9: bits 8-0: VC        - Total vertical count (-1)    Vres ~ (VC+1) - VFP - (VBP+1) - (VSW+1)
    a  : bits 7-0: VFP       - VBlank front porch
    b  : bits 7-0: VBP       - VBlank back porch (-1) (?)
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
- according to p.14-15 both HBP and VBP have +1 added, but to get correct visible areas you have to add it only to VBP
- understand how to interpret the back / front porch values, and remove the offset x/y hack
- dual screen support (for Konami GX types 3/4)
- viostorm and dbz reads the VCT port, but their usage is a side effect to send an irq ack thru the same port:
  i.e. first one uses move.b $26001d.l, $26001d.l, second one clr.b
- le2 sets int-time but never ever enables hblank irq?

***************************************************************************************************************************/


#include "emu.h"
#include "k053252.h"


const device_type K053252 = &device_creator<k053252_device>;

k053252_device::k053252_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, K053252, "K053252 Timing/Interrupt", tag, owner, clock, "k053252", __FILE__),
		device_video_interface(mconfig, *this),
		m_int1_en_cb(*this),
		m_int2_en_cb(*this),
		m_int1_ack_cb(*this),
		m_int2_ack_cb(*this),
		//m_int_time_cb(*this),
		m_offsx(0),
		m_offsy(0),
		// ugly, needed to work with the rungun etc. video demux board
		m_slave_screen(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k053252_device::device_start()
{
	m_int1_en_cb.resolve_safe();
	m_int2_en_cb.resolve_safe();
	m_int1_ack_cb.resolve_safe();
	m_int2_ack_cb.resolve_safe();
	//m_int_time_cb.resolve_safe();

	save_item(NAME(m_regs));
	save_item(NAME(m_hc));
	save_item(NAME(m_hfp));
	save_item(NAME(m_hbp));
	save_item(NAME(m_vc));
	save_item(NAME(m_vfp));
	save_item(NAME(m_vbp));
	save_item(NAME(m_vsw));
	save_item(NAME(m_hsw));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k053252_device::device_reset()
{
	int i;

	for (i = 0; i < 16; i++)
		m_regs[i] = 0;

	m_regs[0x08] = 1; // Xexex apparently does a wrong assignment for VC (sets up the INT enable register instead)

	reset_internal_state();
}

void k053252_device::reset_internal_state()
{
	m_hc=0;
	m_hfp=0;
	m_hbp=0;
	m_vc=0;
	m_vfp=0;
	m_vbp=0;
	m_vsw=0;
	m_hsw=0;
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

READ8_MEMBER( k053252_device::read )
{
	//TODO: debugger_access()
	switch(offset)
	{
		/* VCT read-back */
		// TODO: correct?
		case 0x0e:
			return ((m_screen->vpos()-m_vc) >> 8) & 1;
		case 0x0f:
			return (m_screen->vpos()-m_vc) & 0xff;
		default:
			//popmessage("Warning: k053252 read %02x, contact MAMEdev",offset);
			break;
	}

	return m_regs[offset];
}

void k053252_device::res_change()
{
	if(m_hc && m_vc &&
		m_hbp && m_hfp &&
		m_vbp && m_vfp &&
		m_hsw && m_vsw) //safety checks
	{
		rectangle visarea;
		//(HC+1) - HFP - HBP - 8*(HSW+1)
		//VC - VFP - VBP - (VSW+1)
		attoseconds_t refresh = HZ_TO_ATTOSECONDS(clock()) * (m_hc) * m_vc;

		visarea.min_x = m_offsx;
		visarea.min_y = m_offsy;
		visarea.max_x = m_offsx + m_hc - m_hfp - m_hbp - 8*(m_hsw) - 1;
		visarea.max_y = m_offsy + m_vc - m_vfp - m_vbp - (m_vsw) - 1;

		m_screen->configure(m_hc, m_vc, visarea, refresh);

		if (m_slave_screen.found())
			m_slave_screen->configure(m_hc, m_vc, visarea, refresh);

#if 0
		attoseconds_t hsync = HZ_TO_ATTOSECONDS(clock()) * (m_hc);
		printf("H %d HFP %d HSW %d HBP %d\n",m_hc,m_hfp,m_hsw*8,m_hbp);
		printf("V %d VFP %d VSW %d VBP %d\n",m_vc,m_vfp,m_vsw,m_vbp);
		// L stands for Legacy ...
		printf("L %d %d\n",m_offsx,m_offsy);
		printf("Screen params: Clock: %u V-Sync %.2f H-Sync %.f\n",clock(),ATTOSECONDS_TO_HZ(refresh),ATTOSECONDS_TO_HZ(hsync));
		printf("visible screen area: %d x %d\n\n",(visarea.max_x - visarea.min_x) + 1,(visarea.max_y - visarea.min_y) + 1);
#endif
	}
}

WRITE8_MEMBER( k053252_device::write )
{
	m_regs[offset] = data;

	switch(offset)
	{
		case 0x00:
		case 0x01:
			m_hc  = (m_regs[1]&0xff);
			m_hc |= ((m_regs[0]&0x03)<<8);
			m_hc++;
			logerror("%d (%04x) HC set\n",m_hc,m_hc);
			res_change();
			break;
		case 0x02:
		case 0x03:
			m_hfp  = (m_regs[3]&0xff);
			m_hfp |= ((m_regs[2]&0x01)<<8);
			logerror("%d (%04x) HFP set\n",m_hfp,m_hfp);
			res_change();
			break;
		case 0x04:
		case 0x05:
			m_hbp  = (m_regs[5]&0xff);
			m_hbp |= ((m_regs[4]&0x01)<<8);
			logerror("%d (%04x) HBP set\n",m_hbp,m_hbp);
			res_change();
			break;
		case 0x06: m_int1_en_cb(data); break;
		case 0x07: m_int2_en_cb(data); break;
		case 0x08:
		case 0x09:
			m_vc  = (m_regs[9]&0xff);
			m_vc |= ((m_regs[8]&0x01)<<8);
			m_vc++;
			logerror("%d (%04x) VC set\n",m_vc,m_vc);
			res_change();
			break;
		case 0x0a:
			m_vfp  = (m_regs[0x0a]&0xff);
			logerror("%d (%04x) VFP set\n",m_vfp,m_vfp);
			res_change();
			break;
		case 0x0b:
			m_vbp  = (m_regs[0x0b]&0xff);
			m_vbp++;
			logerror("%d (%04x) VBP set\n",m_vbp,m_vbp);
			res_change();
			break;
		case 0x0c:
			m_vsw  = ((m_regs[0x0c]&0xf0) >> 4) + 1;
			m_hsw  = ((m_regs[0x0c]&0x0f) >> 0) + 1;
			logerror("%02x VSW / %02x HSW set\n",m_vsw,m_hsw);
			res_change();
			break;

		//case 0x0d: m_int_time(data); break;
		case 0x0e: m_int1_ack_cb(1); break;
		case 0x0f: m_int2_ack_cb(1); break;
	}
}


void k053252_device::static_set_slave_screen(device_t &device, const char *tag)
{
	k053252_device &dev = downcast<k053252_device &>(device);
	dev.m_slave_screen.set_tag(tag);
}
