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


#include "k053252.h"


const device_type K053252 = device_creator<k053252_device>;

DEVICE_ADDRESS_MAP_START(map, 8, k053252_device)
	AM_RANGE(0x00, 0x00) AM_WRITE(hch_w)
	AM_RANGE(0x01, 0x01) AM_WRITE(hcl_w)
	AM_RANGE(0x02, 0x02) AM_WRITE(hfph_w)
	AM_RANGE(0x03, 0x03) AM_WRITE(hfpl_w)
	AM_RANGE(0x04, 0x04) AM_WRITE(hbph_w)
	AM_RANGE(0x05, 0x05) AM_WRITE(hbpl_w)
	AM_RANGE(0x06, 0x06) AM_WRITE(irq1_en_w)
	AM_RANGE(0x07, 0x07) AM_WRITE(irq2_en_w)
	AM_RANGE(0x08, 0x08) AM_WRITE(vch_w)
	AM_RANGE(0x09, 0x09) AM_WRITE(vcl_w)
	AM_RANGE(0x0a, 0x0a) AM_WRITE(vfp_w)
	AM_RANGE(0x0b, 0x0b) AM_WRITE(vbp_w)
	AM_RANGE(0x0c, 0x0c) AM_WRITE(sw_w)
	AM_RANGE(0x0d, 0x0d) AM_WRITE(tm_w)
	AM_RANGE(0x0e, 0x0e) AM_READWRITE(vcth_r, irq1_ack_w)
	AM_RANGE(0x0f, 0x0f) AM_READWRITE(vctl_r, irq2_ack_w)
ADDRESS_MAP_END



k053252_device::k053252_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, K053252, "K053252 Video Timing/Interrupt", tag, owner, clock, "k053252", __FILE__),
		device_video_interface(mconfig, *this, false),
		m_int1_cb(*this),
		m_int2_cb(*this),
		m_vblank_cb(*this),
		m_vsync_cb(*this)
{
}

READ8_MEMBER(k053252_device::vcth_r)
{
	return (m_vct & 0xfe) | ((m_vct >> 8) & 1);
}

READ8_MEMBER(k053252_device::vctl_r)
{
	return m_vct;
}

WRITE8_MEMBER(k053252_device::hch_w)
{
	m_hc = (m_hc & 0x00ff) | (data << 8);
	update_screen();
}

WRITE8_MEMBER(k053252_device::hcl_w)
{
	m_hc = (m_hc & 0xff00) | data;
	update_screen();
}

WRITE8_MEMBER(k053252_device::hfph_w)
{
	m_hfp = (m_hfp & 0x00ff) | (data << 8);
	update_screen();
}

WRITE8_MEMBER(k053252_device::hfpl_w)
{
	m_hfp = (m_hfp & 0xff00) | data;
	update_screen();
}

WRITE8_MEMBER(k053252_device::hbph_w)
{
	m_hbp = (m_hbp & 0x00ff) | (data << 8);
	update_screen();
}

WRITE8_MEMBER(k053252_device::hbpl_w)
{
	m_hbp = (m_hbp & 0xff00) | data;
	update_screen();
}

WRITE8_MEMBER(k053252_device::irq1_en_w)
{
	if(m_int1_on && !data) {
		m_int1_on = false;
		m_int1_cb(CLEAR_LINE);
	}
	m_int1_en = data;
	logerror("irq 1 enable %02x\n", data);
}

WRITE8_MEMBER(k053252_device::irq2_en_w)
{
	if(m_int2_on && !data) {
		m_int2_on = false;
		m_int2_cb(CLEAR_LINE);
	}
	m_int2_en = data;
	logerror("irq 2 enable %02x\n", data);
}

WRITE8_MEMBER(k053252_device::vch_w)
{
	m_vc = (m_vc & 0x00ff) | (data << 8);
	update_screen();
}

WRITE8_MEMBER(k053252_device::vcl_w)
{
	m_vc = (m_vc & 0xff00) | data;
	update_screen();
}

WRITE8_MEMBER(k053252_device::vfp_w)
{
	m_vfp = data;
	update_screen();
}

WRITE8_MEMBER(k053252_device::vbp_w)
{
	m_vbp = data;
	update_screen();
}

WRITE8_MEMBER(k053252_device::sw_w)
{
	m_sw = data;
	update_screen();
}

WRITE8_MEMBER(k053252_device::tm_w)
{
	m_tm = data;
}

WRITE8_MEMBER(k053252_device::irq1_ack_w)
{
	if(m_int1_on) {
		m_int1_on = false;
		m_int1_cb(CLEAR_LINE);
	}
}

WRITE8_MEMBER(k053252_device::irq2_ack_w)
{
	if(m_int2_on) {
		m_int2_on = false;
		m_int2_cb(CLEAR_LINE);
	}
}



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k053252_device::device_start()
{
	m_int1_cb.resolve_safe();
	m_int2_cb.resolve_safe();
	m_vblank_cb.resolve_safe();
	m_vsync_cb.resolve_safe();

	m_timer_frame = timer_alloc(TIMER_FRAME);
	m_timer_htimer = timer_alloc(TIMER_HTIMER);
	m_timer_source_vblank = timer_alloc(TIMER_SOURCE_VBLANK);

	save_item(NAME(m_vct));

	save_item(NAME(m_hc));
	save_item(NAME(m_hfp));
	save_item(NAME(m_hbp));
	save_item(NAME(m_vc));
	save_item(NAME(m_vfp));
	save_item(NAME(m_vbp));
	save_item(NAME(m_sw));
	save_item(NAME(m_tm));

	save_item(NAME(m_int1_en));
	save_item(NAME(m_int1_on));
	save_item(NAME(m_int2_en));
	save_item(NAME(m_int2_on));

	save_item(NAME(m_line_duration));
	save_item(NAME(m_frame));
	save_item(NAME(m_vblank_in_to_vsync_in));
	save_item(NAME(m_vsync_in_to_vsync_out));
	save_item(NAME(m_vsync_out_to_vblank_out));
	save_item(NAME(m_vsync_in_to_hblank_in));
	save_item(NAME(m_hblank_in_to_hsync_in));
	save_item(NAME(m_hsync_in_to_hblank_in));
	save_item(NAME(m_timer_frame_state));
	save_item(NAME(m_timer_htimer_state));

	m_hc = m_vc = 0;
	m_timer_frame_state = FT_WAIT_VBLANK_IN;
	m_timer_htimer_state = HT_WAIT_HBLANK_IN;

	if(m_screen)
		m_screen->register_vblank_callback(vblank_state_delegate(&k053252_device::screen_vblank, this));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k053252_device::device_reset()
{
	// Default to the standard 288x224 layout
	m_hc  = 0x17f;
	m_hfp = 0x010;
	m_hbp = 0x030;
	m_vc  = 0x107;
	m_vfp =  0x11;
	m_vbp =  0x0e;
	m_sw  =  0x73;
	m_tm  =  0xff;

	m_vct = 0x000;

	m_int1_on = false;
	m_int1_en = true;
	m_int2_on = false;
	m_int2_en = true;

	m_timer_frame_state = FT_WAIT_VBLANK_IN;
	m_timer_htimer_state = HT_WAIT_HBLANK_IN;

	update_screen();
}

void k053252_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id) {
	case TIMER_FRAME:
		frame_transition();
		break;
	case TIMER_HTIMER:
		htimer_transition();
		break;
	case TIMER_SOURCE_VBLANK:
		if(m_timer_frame_state == FT_WAIT_VBLANK_IN)
			frame_transition();
		break;
	}
}

void k053252_device::device_clock_changed()
{
	update_screen();
}

void k053252_device::screen_vblank(screen_device &src, bool state)
{
	if(state && m_timer_frame_state == FT_WAIT_VBLANK_IN)
		frame_transition();
}

void k053252_device::update_screen()
{
	// Beware, it's called before reset through device_clock_changed()
	if(!m_hc || !m_vc)
		return;

	int hc = (m_hc & 0x3ff) + 1;
	int hfp = (m_hfp & 0x1ff);
	int hbp = (m_hbp & 0x1ff);
	int hsw = ((m_sw & 0xf) + 1)*8;

	int vc = (m_vc & 0x1ff) + 1;
	int vfp = (m_vfp & 0xff);
	int vbp = (m_vbp & 0xff) + 1;
	int vsw = ((m_sw >> 4) & 0xf) + 1;

	int width  = hc - hfp - hsw - hbp;
	int height = vc - vfp - vsw - vbp;

	// We put the origin of the screen bitmap at the start of the back porch (end of sync)
	rectangle visarea;
	visarea.min_x = hbp;
	visarea.min_y = vbp;
	visarea.max_x = hc - hsw - hfp - 1;
	visarea.max_y = vc - vsw - vfp - 1;

	// Screen vblank happens at coordinates (0, vc - vsw - vfp) and ends at (0, vbp)
	// Vsync happens at (0, vc - vsw) and ends at (0, 0)
	// Hsync happens at (hc - hsw, line) and ends at (0, line+1)
	// Hblank happens at (hc - hsw - hfp, line) and ends at (hbp, line+1)
	m_line_duration = attotime::from_ticks(hc, clock());
	m_frame = m_line_duration * vc;
	m_vblank_in_to_vsync_in = m_line_duration * vfp;
	m_vsync_in_to_vsync_out = m_line_duration * vsw;
	m_vsync_out_to_vblank_out = m_line_duration * vbp;
	m_vsync_in_to_hblank_in = attotime::from_ticks(hc - hsw - hfp, clock());
	m_hblank_in_to_hsync_in = attotime::from_ticks(hfp, clock());
	m_hsync_in_to_hblank_in = attotime::from_ticks(hc - hfp, clock());

	switch(m_timer_frame_state) {
	case FT_WAIT_VBLANK_IN:  break;
	case FT_WAIT_VSYNC_IN:   m_vblank_cb(CLEAR_LINE); break;
	case FT_WAIT_VSYNC_OUT:  m_vblank_cb(CLEAR_LINE); m_vsync_cb(ASSERT_LINE); break;
	case FT_WAIT_VBLANK_OUT: m_vblank_cb(CLEAR_LINE); break;
	}

	m_timer_frame->adjust(attotime::never);
	m_timer_frame_state = FT_WAIT_VBLANK_IN;
	m_timer_htimer->adjust(attotime::never);
	m_timer_htimer_state = HT_WAIT_HBLANK_IN;

	if(0) {
		visarea.min_x = 0;
		visarea.min_y = 0;
		visarea.max_x = hc-1;
		visarea.max_y = vc-1;
	}

	logerror("YTY '252 w %03x vis %03x hs %x\n", 
			 width, hbp, hsw);
	logerror("XTX '252 h %03x vis %03x vs %x\n", 
			 height, vbp, vsw);
	logerror("screen: %dMHz %d (%d - %d - %d - %d) %d (%d - %d - %d - %d) vbl %gHz, visarea (%d, %d)-(%d, %d)\n",
			 clock()/1000000,
			 hc, hbp, width,  hfp, hsw,
			 vc, vbp, height, vfp, vsw,
			 1/m_frame.as_double(),
			 visarea.min_x, visarea.min_y,
			 visarea.max_x, visarea.max_y);

	logerror("TIMINGS '252 %2d %3d %2d %3d %2d %2d %3d %2d %3d %2d %2d\n",
			 clock()/1000000,
			 hc, hbp, width,  hfp, hsw,
			 vc, vbp, height, vfp, vsw);

	// If there's a screen, use it.  Otherwise, setup a timer for a
	// vblank similar to what the screen would have done.
	if(m_screen)
		m_screen->configure(hc, vc, visarea, m_frame.as_attoseconds());
	else
		// Run the timer in half a frame then every frame
		m_timer_source_vblank->adjust(m_frame/2, 0, m_frame);
}

void k053252_device::frame_transition()
{
	switch(m_timer_frame_state) {
	case FT_WAIT_VBLANK_IN:
		m_timer_frame->adjust(m_vblank_in_to_vsync_in);
		m_timer_frame_state = FT_WAIT_VSYNC_IN;
		if(m_int1_en && !m_int1_on) {
			m_int1_on = true;
			m_int1_cb(ASSERT_LINE);
		}
		m_vblank_cb(ASSERT_LINE);
		break;
	case FT_WAIT_VSYNC_IN:
		m_timer_frame->adjust(m_vsync_in_to_vsync_out);
		m_timer_frame_state = FT_WAIT_VSYNC_OUT;
		m_timer_htimer->adjust(m_vsync_in_to_hblank_in);
		m_timer_htimer_state = HT_WAIT_HBLANK_IN;
		m_vct = (m_vc & 0x3ff) ^ 0x3ff;
		m_vsync_cb(ASSERT_LINE);
		break;
	case FT_WAIT_VSYNC_OUT:
		m_timer_frame->adjust(m_vsync_out_to_vblank_out);
		m_timer_frame_state = FT_WAIT_VBLANK_OUT;
		m_vsync_cb(CLEAR_LINE);
		break;
	case FT_WAIT_VBLANK_OUT:
		m_timer_frame->adjust(attotime::never);
		m_timer_frame_state = FT_WAIT_VBLANK_IN;
		m_vblank_cb(CLEAR_LINE);
		// dragoona locks up if there's an int1 pending when it's turned on
		// (the irq handler waits for the objdma ending but the objdma is enabled just after turning the int1 enable on)
		if(m_int1_on) {
			m_int1_on = false;
			m_int1_cb(CLEAR_LINE);
		}
		break;
	}
}

void k053252_device::htimer_transition()
{
	switch(m_timer_htimer_state) {
	case HT_WAIT_HBLANK_IN:
		if(m_int2_en && (m_vct ^ 0x3ff) - (m_vc & 0x1ff) == m_tm) {
			if(!m_int2_on) {
				m_int2_on = true;
				m_int2_cb(ASSERT_LINE);
			}
			m_timer_htimer->adjust(m_hblank_in_to_hsync_in);
			m_timer_htimer_state = HT_WAIT_HSYNC_IN;
		} else
			m_timer_htimer->adjust(m_line_duration);
		m_vct++;
		break;
	case HT_WAIT_HSYNC_IN:
		m_timer_htimer->adjust(m_hsync_in_to_hblank_in);
		m_timer_htimer_state = HT_WAIT_HBLANK_IN;
		if(m_int2_on) {
			m_int2_on = false;
			m_int2_cb(CLEAR_LINE);
		}
		break;
	}
}
