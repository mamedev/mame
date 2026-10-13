// license:BSD-3-Clause
// copyright-holders:Angelo Salese, ElSemi
/**************************************************************************************************

MagicEyes VRender0 SoC peripherals

Device by Angelo Salese
Based off original crystal.cpp by ElSemi

TODO:
- Improve encapsulation, still needs a few trampolines from host driver;
- Proper PIO emulation;
- Output CRTC border color;
- Add VCLK select;
- Implement dynamic clock via PLL

**************************************************************************************************/

#include "emu.h"
#include "vrender0.h"

#define LOG_CRTC     (1U << 1)
#define LOG_DMA      (1U << 2)

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(VRENDER0_SOC, vrender0soc_device, "vrender0", "MagicEyes VRender0 SoC")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vrender0soc_device - constructor
//-------------------------------------------------

vrender0soc_device::vrender0soc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, VRENDER0_SOC, tag, owner, clock),
	device_mixer_interface(mconfig, *this),
	m_screen(*this, "screen"),
	m_palette(*this, "palette"),
	m_vr0vid(*this, "vr0vid"),
	m_vr0snd(*this, "vr0snd"),
	m_uart(*this, "uart%u", 0),
	m_crtcregs(*this, "crtcregs"),
	m_light_pen_cb(*this, 0),
	m_host_space(*this, finder_base::DUMMY_TAG, -1, 32),
	m_textureram(*this, "textureram", 0x800000, ENDIANNESS_LITTLE),
	m_frameram(*this, "frameram", 0x800000, ENDIANNESS_LITTLE),
	m_int_cb(*this),
	m_write_tx(*this)
{
}

// base +$0180'0000
void vrender0soc_device::regs_map(address_map &map)
{
//  map(0x00000, 0x003ff)                            // System/General
	map(0x00000, 0x00003).r(FUNC(vrender0soc_device::sysid_r));
	map(0x00004, 0x00007).r(FUNC(vrender0soc_device::cfgr_r));
	map(0x00010, 0x00017).noprw(); // watchdog
//  map(0x00400, 0x007ff)                            // Local Memory Controller
//  map(0x00800, 0x00bff)                            // DMA
	map(0x00800, 0x00803).rw(FUNC(vrender0soc_device::dmac_r<0>), FUNC(vrender0soc_device::dmac_w<0>));
	map(0x00804, 0x00807).rw(FUNC(vrender0soc_device::dmasa_r<0>), FUNC(vrender0soc_device::dmasa_w<0>));
	map(0x00808, 0x0080b).rw(FUNC(vrender0soc_device::dmada_r<0>), FUNC(vrender0soc_device::dmada_w<0>));
	map(0x0080c, 0x0080f).rw(FUNC(vrender0soc_device::dmatc_r<0>), FUNC(vrender0soc_device::dmatc_w<0>));
	map(0x00810, 0x00813).rw(FUNC(vrender0soc_device::dmac_r<1>), FUNC(vrender0soc_device::dmac_w<1>));
	map(0x00814, 0x00817).rw(FUNC(vrender0soc_device::dmasa_r<1>), FUNC(vrender0soc_device::dmasa_w<1>));
	map(0x00818, 0x0081b).rw(FUNC(vrender0soc_device::dmada_r<1>), FUNC(vrender0soc_device::dmada_w<1>));
	map(0x0081c, 0x0081f).rw(FUNC(vrender0soc_device::dmatc_r<1>), FUNC(vrender0soc_device::dmatc_w<1>));

//  map(0x00c00, 0x00fff)                            // Interrupt Controller
	map(0x00c04, 0x00c07).rw(FUNC(vrender0soc_device::intvec_r), FUNC(vrender0soc_device::intvec_w));
	map(0x00c08, 0x00c0b).rw(FUNC(vrender0soc_device::inten_r), FUNC(vrender0soc_device::inten_w));
	map(0x00c0c, 0x00c0f).rw(FUNC(vrender0soc_device::intst_r), FUNC(vrender0soc_device::intst_w));
//  map(0x01000, 0x013ff)                            // UART
	map(0x01000, 0x0101f).m(m_uart[0], FUNC(vr0uart_device::regs_map));
	map(0x01020, 0x0103f).m(m_uart[1], FUNC(vr0uart_device::regs_map));
//  map(0x01400, 0x017ff)                            // Timer & Counter
	map(0x01400, 0x01403).rw(FUNC(vrender0soc_device::tmcon_r<0>), FUNC(vrender0soc_device::tmcon_w<0>));
	map(0x01404, 0x01407).rw(FUNC(vrender0soc_device::tmcnt_r<0>), FUNC(vrender0soc_device::tmcnt_w<0>)).umask32(0x0000ffff);
	map(0x01408, 0x0140b).rw(FUNC(vrender0soc_device::tmcon_r<1>), FUNC(vrender0soc_device::tmcon_w<1>));
	map(0x0140c, 0x0140f).rw(FUNC(vrender0soc_device::tmcnt_r<1>), FUNC(vrender0soc_device::tmcnt_w<1>)).umask32(0x0000ffff);
	map(0x01410, 0x01413).rw(FUNC(vrender0soc_device::tmcon_r<2>), FUNC(vrender0soc_device::tmcon_w<2>));
	map(0x01414, 0x01417).rw(FUNC(vrender0soc_device::tmcnt_r<2>), FUNC(vrender0soc_device::tmcnt_w<2>)).umask32(0x0000ffff);
	map(0x01418, 0x0141b).rw(FUNC(vrender0soc_device::tmcon_r<3>), FUNC(vrender0soc_device::tmcon_w<3>));
	map(0x0141c, 0x0141f).rw(FUNC(vrender0soc_device::tmcnt_r<3>), FUNC(vrender0soc_device::tmcnt_w<3>)).umask32(0x0000ffff);

//  map(0x01800, 0x01bff)                            // Pulse Width Modulation
//  map(0x02000, 0x023ff)                            // PIO (Port)
//  map(0x02004, 0x02007).rw(FUNC(vrender0soc_device::PIO_r), FUNC(vrender0soc_device::PIO_w)); // PIOLDAT
//  map(0x02008, 0x0200b)                                                             // PIOEDAT
//  map(0x02400, 0x027ff)                            // Peripheral Chip Select
//  map(0x02800, 0x02bff)                            // SIO
//  map(0x03400, 0x037ff)                            // CRT Controller
	map(0x03400, 0x03437).rw(FUNC(vrender0soc_device::crtc_r), FUNC(vrender0soc_device::crtc_w)).share(m_crtcregs);
	// LIGHT0X / LIGHT0Y / LIGHT1X / LIGHT1Y
	map(0x03438, 0x03447).lr32(NAME([this] (offs_t offset, u32 mem_mask) { return m_light_pen_cb[offset](0, mem_mask) & mem_mask; }));
	// LIGHTC: Light Pen Input Control
	map(0x03448, 0x0344b).lrw32(
		NAME([this] () { return m_lightc; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (ACCESSING_BITS_0_7)
				m_lightc = data & 3;
		})
	);
//  map(0x04000, 0x043ff)                            // RAMDAC & PLL
//  map(0x04000, 0x04003)                            // PLL control register
//  map(0x04004, 0x04007)                            // PLL Program register
}

void vrender0soc_device::audiovideo_map(address_map &map)
{
	map(0x00000000, 0x0000ffff).m(m_vr0vid, FUNC(vr0video_device::regs_map));
	map(0x00800000, 0x00ffffff).m(FUNC(vrender0soc_device::texture_map));
	map(0x01000000, 0x017fffff).m(FUNC(vrender0soc_device::frame_map));
	map(0x01800000, 0x01800fff).m(m_vr0snd, FUNC(vr0sound_device::sound_map));
}

void vrender0soc_device::texture_map(address_map &map)
{
	map(0x000000, 0x7fffff).rw(FUNC(vrender0soc_device::textureram_r), FUNC(vrender0soc_device::textureram_w));
}

void vrender0soc_device::frame_map(address_map &map)
{
	map(0x000000, 0x7fffff).rw(FUNC(vrender0soc_device::frameram_r), FUNC(vrender0soc_device::frameram_w));
}

//-------------------------------------------------
//  device_add_mconfig - device-specific machine
//  configuration addiitons
//-------------------------------------------------

void vrender0soc_device::device_add_mconfig(machine_config &config)
{
	for (required_device<vr0uart_device> &uart : m_uart)
		VRENDER0_UART(config, uart, 3'579'500); // DERIVED_CLOCK(1, 24));

	SCREEN(config, m_screen);
	// evolution soccer defaults
	m_screen->set_raw((XTAL(14'318'181)*2)/4, 455, 0, 320, 262, 0, 240);
	m_screen->set_screen_update(FUNC(vrender0soc_device::screen_update));
	m_screen->screen_vblank().set(FUNC(vrender0soc_device::screen_vblank));
	m_screen->set_palette(m_palette);

	// runs at double speed WRT the bus clock
	VIDEO_VRENDER0(config, m_vr0vid, DERIVED_CLOCK(1, 1));
	m_vr0vid->set_addrmap(vr0video_device::AS_TEXTURE, &vrender0soc_device::texture_map);
	m_vr0vid->set_addrmap(vr0video_device::AS_FRAME, &vrender0soc_device::frame_map);
	m_vr0vid->set_screen(m_screen);

	PALETTE(config, m_palette, palette_device::RGB_565);

	SOUND_VRENDER0(config, m_vr0snd, DERIVED_CLOCK(1, 2)); // Correct?
	m_vr0snd->set_addrmap(vr0sound_device::AS_TEXTURE, &vrender0soc_device::texture_map);
	m_vr0snd->set_addrmap(vr0sound_device::AS_FRAME, &vrender0soc_device::frame_map);
	m_vr0snd->irq_callback().set(FUNC(vrender0soc_device::soundirq_cb));
	m_vr0snd->add_route(0, *this, 1.0, 0);
	m_vr0snd->add_route(1, *this, 1.0, 1);
}

void vrender0soc_device::write_line_tx(int port, u8 value)
{
	//logerror("callback %d %02x\n", port, value);
	m_write_tx[port & 1](value);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vrender0soc_device::device_start()
{
	if (this->clock() == 0)
		fatalerror("%s: bus clock not setup properly", machine().describe_context());

	m_timer[0].timer = timer_alloc(FUNC(vrender0soc_device::timer_cb<0>), this);
	m_timer[1].timer = timer_alloc(FUNC(vrender0soc_device::timer_cb<1>), this);
	m_timer[2].timer = timer_alloc(FUNC(vrender0soc_device::timer_cb<2>), this);
	m_timer[3].timer = timer_alloc(FUNC(vrender0soc_device::timer_cb<3>), this);

	for (int i = 0; i < 2; i++)
	{
		m_uart[i]->set_channel_num(i);
		m_uart[i]->set_parent(this);
		m_uart[i]->set_external_clock(m_uart_uclk);
	}

	save_item(NAME(m_inten));
	save_item(NAME(m_int_high));
	save_item(NAME(m_intst));

	save_item(STRUCT_MEMBER(m_timer, control));
	save_item(STRUCT_MEMBER(m_timer, count));

	save_item(STRUCT_MEMBER(m_dma, src));
	save_item(STRUCT_MEMBER(m_dma, dst));
	save_item(STRUCT_MEMBER(m_dma, size));
	save_item(STRUCT_MEMBER(m_dma, ctrl));
	m_dma[0].timer = timer_alloc(FUNC(vrender0soc_device::dma_step_cb<0>), this);
	m_dma[1].timer = timer_alloc(FUNC(vrender0soc_device::dma_step_cb<1>), this);

	// TODO: register CRTC
	save_item(NAME(m_lightc));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void vrender0soc_device::device_reset()
{
	// TODO: improve CRT defaults
	m_crtcregs[1] = 0x0000002a;

	//m_FlipCount = 0;
	m_int_high = 0;

	for (auto &dma : m_dma)
	{
		dma.ctrl = 0;
		dma.timer->adjust(attotime::never);
	}
	for (auto &tmr : m_timer)
	{
		tmr.control = 0xff << 8;
		tmr.timer->adjust(attotime::never);
	}
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

/*
 *
 * Texture/FrameRAM 16-bit trampolines
 *
 */

u16 vrender0soc_device::textureram_r(offs_t offset)
{
	return m_textureram[offset];
}

void vrender0soc_device::textureram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_textureram[offset]);
}

u16 vrender0soc_device::frameram_r(offs_t offset)
{
	return m_frameram[offset];
}

void vrender0soc_device::frameram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_frameram[offset]);
}

/*
 *
 * INT Controller
 *
 */

u32 vrender0soc_device::intvec_r()
{
	return (m_int_high & 7) << 8;
}

void vrender0soc_device::intvec_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_intst &= ~(1 << (data & 0x1f));
		if (!m_intst)
			m_int_cb(CLEAR_LINE);
	}
	if (ACCESSING_BITS_8_15)
		m_int_high = (data >> 8) & 7;
}

u32 vrender0soc_device::inten_r()
{
	return m_inten;
}

void vrender0soc_device::inten_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_inten);
	// psattack has a timer 0 irq service with no call to intvec_w but just this
	m_intst &= m_inten;
	if (!m_intst)
		m_int_cb(CLEAR_LINE);
}

u32 vrender0soc_device::intst_r()
{
	return m_intst;
}

void vrender0soc_device::intst_w(u32 data)
{
	// TODO: contradicts with documentation, games writes to this?
	// ...
}

void vrender0soc_device::int_req(int num)
{
	// TODO: this doesn't look right, it should still plonk in pending state even if irq masked
	if (m_inten & (1 << num))
	{
		m_intst |= (1 << num);
		m_int_cb(ASSERT_LINE);
	}
}


u8 vrender0soc_device::irq_callback()
{
	// NOTE: the highest irq source would be b26
	for (int i = 0; i < 32; ++i)
	{
		if (BIT(m_intst, i))
		{
			return (m_int_high << 5) | i;
		}
	}
	// This should never happen
	return 0;
}


void vrender0soc_device::soundirq_cb(int state)
{
	if (state)
	{
		int_req(IRQ_WAVE_SYNTH);
	}
}

/*
 *
 * Timer
 *
 */


void vrender0soc_device::timer_start(int which)
{
	vr0_timer &tmr = m_timer[which];
	int const pd = (tmr.control >> 8) & 0xff;
	int const tcv = tmr.count & 0xffff;
	// TODO: documentation claims this is bus clock, half the internal PLL frequency.
	attotime const period = attotime::from_hz(this->clock()) * 2 * ((pd + 1) * (tcv + 1));
	tmr.timer->adjust(period);

//  logerror("timer %d start, pd = %x tcv = %x period = %s\n", which, pd, tcv, period.as_string());
}

template<int Which> TIMER_CALLBACK_MEMBER(vrender0soc_device::timer_cb)
{
	static const int num[] = { IRQ_TIMER0, IRQ_TIMER1, IRQ_TIMER2, IRQ_TIMER3 };
	vr0_timer &tmr = m_timer[Which];

	if (BIT(tmr.control, 1))
		timer_start(Which);
	else
		tmr.control &= ~1;

	int_req(num[Which]);
}

template<int Which> u32 vrender0soc_device::tmcon_r()
{
	return m_timer[Which].control;
}

template<int Which> void vrender0soc_device::tmcon_w(offs_t offset, u32 data, u32 mem_mask)
{
	vr0_timer &tmr = m_timer[Which];
	u32 const old = tmr.control;
	data = COMBINE_DATA(&tmr.control);

	if (BIT(data ^ old, 0))
	{
		if (BIT(data, 0))
		{
			timer_start(Which);
		}
		else
		{
			// Timer stop
			tmr.timer->adjust(attotime::never);
//          logerror("%s: timer %d stop\n", machine().describe_context(), Which);
		}
	}
}

template<int Which> u16 vrender0soc_device::tmcnt_r()
{
	return m_timer[Which].count & 0xffff;
}

template<int Which> void vrender0soc_device::tmcnt_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_timer[Which].count);
}

/*
 *
 * DMA Controller
 *
 */

// helper
// bit 5 and bit 3 of the DMA control don't in/decrement source/destination addresses if enabled.
// At the time of writing P's Attack is the only SW that uses this feature,
// in a work RAM to area $4500000 transfer, probably to extend something ...
inline int dma_setup_hold(u8 setting, u8 holdbit, u8 dirbit)
{
	if (BIT(setting, holdbit))
		return 0;

	int const amount = BIT(setting, 1) ? 4 : (1 << BIT(setting, 0));
	return BIT(setting, dirbit) ? -amount : amount;
}

template<int Which> u32 vrender0soc_device::dmasa_r() { return m_dma[Which].src; }
template<int Which> void vrender0soc_device::dmasa_w(offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_dma[Which].src); }
template<int Which> u32 vrender0soc_device::dmada_r() { return m_dma[Which].dst; }
template<int Which> void vrender0soc_device::dmada_w(offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_dma[Which].dst); }
template<int Which> u32 vrender0soc_device::dmatc_r() { return m_dma[Which].size & 0xff'ffff; }
template<int Which> void vrender0soc_device::dmatc_w(offs_t offset, u32 data, u32 mem_mask) {
	COMBINE_DATA(&m_dma[Which].size);
	m_dma[Which].size &= 0xff'ffff;
}
template<int Which> u32 vrender0soc_device::dmac_r() { return m_dma[Which].ctrl; }

// -x-- ---- ---- DMAENx enable
// --x- ---- ---- DMAPOLx request active polarity (0: high, 1: low)
// ---x ---- ---- DMAWRENx Counter write enable (0: disable, 1: enable)
// ---- xx-- ---- DMAMODEx transfer mode
// ---- 0x-- ----          Single transfer
// ---- 10-- ----          Repeat with reload counter
// ---- 11-- ----          Repeat with reload counter and registers
// ---- --x- ---- DMASHOLDx Source address hold (0: in/decrease, 1: fix address)
// ---- ---x ---- SMASDIRx  Source address direction (0: increase, 1: decrease)
// ---- ---- x--- DMADHOLDx Destination address hold (0: in/decrease, 1: fix address)
// ---- ---- -x-- DMADIRx   Destination address direction (0: increase, 1: decrease)
// ---- ---- --xx DMATRWIDTHx Transfer width
// ---- ---- --00 8 bit
// ---- ---- --01 16 bit
// ---- ---- --1x 32 bit
template<int Which> void vrender0soc_device::dmac_w(offs_t offset, u32 data, u32 mem_mask)
{
	vr0_dma &dma = m_dma[Which];

	if (ACCESSING_BITS_0_15)
	{
		// DMAENx
		if (BIT(data ^ dma.ctrl, 10) && BIT(data, 10))
		{
			if (data & 0x80)
				popmessage("machine/vrender0.cpp: DMA%d with repeat mode %02x", Which, data & 0xc0);

			if (dma.size == 0)
				popmessage("machine/vrender0.cpp: DMA%d attempt with zero size length", Which);

			LOGMASKED(LOG_DMA, "DMA%d src %08x dst %08x size: %06x ctrl: %03x\n"
				, Which, dma.src, dma.dst, dma.size, data);

			m_dma[Which].timer->adjust(attotime::from_ticks(2, this->clock()));
		}

		COMBINE_DATA(&dma.ctrl);
	}
}

// TODO: sketchy details
// - is burst or cycle steal?
// - how many cycles it actually takes to complete a transfer?
// - how live updating works, and the implications of repeat latches;
// - if a dma size of 0 is really ignored or it rolls over at max (definitely need use case);
// - why psattack mixes in transfers with both HOLDs enabled (where src is ATA and dst is frame RAM),
//   is it just flushing PIO data or there's more of it?
template <unsigned Which> TIMER_CALLBACK_MEMBER(vrender0soc_device::dma_step_cb)
{
	vr0_dma &dma = m_dma[Which];

	if (!BIT(dma.ctrl, 10))
		return;

	// transfer ends at zero, not at rolling over (cfr. psattack)
	if (dma.size == 0)
	{
		dma.ctrl &= ~(1 << 10);
		int_req(IRQ_DMA0 + Which);
		return;
	}

	u32 const src = dma.src;
	u32 const dst = dma.dst;
	int const src_inc = dma_setup_hold(dma.ctrl, 5, 4);
	int const dst_inc = dma_setup_hold(dma.ctrl, 3, 2);

	switch(dma.ctrl & 3)
	{
		case 0:
			m_host_space->write_byte(dst, m_host_space->read_byte(src));
			break;
		case 1:
			m_host_space->write_word(dst, m_host_space->read_word(src));
			break;
		case 2:
		default:
			m_host_space->write_dword(dst, m_host_space->read_dword(src));
			break;
	}

	dma.src += src_inc;
	dma.dst += dst_inc;
	dma.size --;

	m_dma[Which].timer->adjust(attotime::from_ticks(4, this->clock()));
}

/*
 *
 * CRT Controller
 *
 */

// [0] CRTC Status / Mode CRTMOD
// x--- ---- ---- ---- Horizontal Sync Status (active low)
// -x-- ---- ---- ---- Vertical Display Enable Status (1 when in display area)
// --x- ---- ---- ---- Horizontal & Vertical Blank period (active low)
// ---x ---- ---- ---- External video signal status
// ---- --x- ---- ---- Screen Blank Enable
// ---- ---x ---- ---- CRTC write protect 0x04~0x27
// ---- ---- x--- ---- Horizontal Scan Line Number (0: 525 1: 625)
// ---- ---- -x-- ---- Color Burst Frequency
// ---- ---- -0-- ---- 3.58 MHz
// ---- ---- -1-- ---- 4.43 MHz
// ---- ---- ---- x--- Select Display Start Field in interlace mode
// ---- ---- ---- 0--- NTSC: odd PAL: even
// ---- ---- ---- 1--- NTSC: even PAL: odd
// ---- ---- ---- --xx Vertical Sync width generation
// ---- ---- ---- --00 Serration only
// ---- ---- ---- --01 Pre-equalization and serration
// ---- ---- ---- --10 Post-equalization and serration
// ---- ---- ---- --11 Pre/Post-equalization and serration
u32 vrender0soc_device::crtc_r(offs_t offset)
{
	u32 res = m_crtcregs[offset];
	switch (offset)
	{
		case 0: // CRTC Status / Mode
		{
			res &= 0x03ff;
			u32 const hdisp = (m_crtcregs[0x0c / 4] + 1);
			u32 vdisp = (m_crtcregs[0x1c / 4] + 1);

			if (crt_is_interlaced()) // Interlace
				vdisp <<= 1;

			const int vpos = m_screen->vpos();
			const int hpos = m_screen->hpos();

			// Vertical display enable status
			if (vpos <= vdisp)
				res |= 1 << 14;

			// horizontal & vertical blank period
			if (hpos <= hdisp && vpos <= vdisp)
				res |= 1 << 13;

			// horizontal sync status
			// TODO: sync not display, depends on the auto stuff in CRTC calcs, particularly front porch
			// (donghaer sets zero there)
			if (hpos <= hdisp)
				res |= 1 << 15;

			break;
		}
		default:
			break;
	}
	return res;
}

void vrender0soc_device::crtc_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (((m_crtcregs[0] & 0x0100) == 0x0100) && (offset > 0) && (offset < 0x28/4)) // Write protect
		return;

	u32 const old = m_crtcregs[offset];
	switch (offset * 4)
	{
		case 0: // CRTC Status / Mode Register (CRTMOD)
			mem_mask &= ~0xfffffc00; // Bit 31-10 Reserved
			// TODO: blank screen bit 9 should be routed to video device
			// (if anything ever bothered with it)
			break;
		case 0x04: // CRTC Timing Control Register (CRTTIM)
			mem_mask &= ~0xffffc000; // Bit 31-14 Reserved
			break;
		case 0x08: // Horizontal Sync Width / Back Porch Register (HSWBP)
			mem_mask &= ~0xffff0000; // Bit 31-16 Reserved
			break;
		case 0x0c: // Horizontal Display Total Register (HDISP)
			mem_mask &= ~0xfffffc00; // Bit 31-10 Reserved
			break;
		case 0x10: // Horizontal Sync Front Porch Register (HSFP)
			mem_mask &= ~0xfffffe00; // Bit 31-9 Reserved
			break;
		case 0x14: // Field Window Bound Register (FWINB)
			mem_mask &= ~0xffff80c0; // Bit 31-15, 7-6 Reserved
			break;
		case 0x18: // Vertical Sync Back Porch Register (VSBP)
			mem_mask &= ~0xffffff00; // Bit 31-8 Reserved
			break;
		case 0x1c: // Vertical Display Total Register (VDISP)
			mem_mask &= ~0xfffffe00; // Bit 31-9 Reserved
			break;
		case 0x20: // Horizontal Total Register (HTOT)
			mem_mask &= ~0xffffe000; // Bit 31-13 Reserved
			if (BIT(data, 10) == 0) // enable bit
				return;
			break;
		case 0x24: // Vertical Total Register (VTOT)
			mem_mask &= ~0xfffff000; // Bit 31-12 Reserved
			if (BIT(data, 11) == 0) // enable bit
				return;
			break;
		case 0x28: // Horizontal Line Back Porch Register (HLBP)
			mem_mask &= ~0xfffffc00; // Bit 31-10 Reserved
			break;
		case 0x2c: // CRT Display Start Address 0 Register (STAD0)
			mem_mask &= ~0xffff8000; // Bit 31-15 Reserved
			break;
		case 0x30: // CRT Display Start Address 1 Register (STAD1)
			mem_mask &= ~0xffff8000; // Bit 31-15 Reserved
			break;
		// TODO: register 0x34 TCOL (video overlay color)
		default:
			return;
	}
	COMBINE_DATA(&m_crtcregs[offset]);
	if (old ^ m_crtcregs[offset])
		crtc_update();

}

inline bool vrender0soc_device::crt_is_interlaced()
{
	return BIT(~m_crtcregs[0x30 / 4], 0);
}

bool vrender0soc_device::crt_active_vblank_irq()
{
	if (!crt_is_interlaced())
		return true;

	// bit 3 of CRTC reg -> select display start even/odd fields
	return (m_screen->frame_number() & 1) ^ ((m_crtcregs[0] & 8) >> 3);
}

void vrender0soc_device::crtc_update()
{
	u32 const hdisp = m_crtcregs[0x0c / 4] + 1;
	u32 vdisp = m_crtcregs[0x1c / 4];
	if (hdisp == 0 || vdisp == 0)
		return;

	bool const interlace_mode = crt_is_interlaced();

	if (interlace_mode)
		vdisp <<= 1;

	u32 htot = (m_crtcregs[0x20 / 4] & 0x3ff) + 1;
	u32 vtot = (m_crtcregs[0x24 / 4] & 0x7ff);

	// adjust htotal in case it's not setup by the game
	// (datasheet mentions that it can be done automatically shrug):
	// - the two Sealy games do that
	// - Cross Puzzle sets up an HTotal of 400 with 640x480 display
	// - donghaer writes a 0 to the htot when entering interlace mode
	// TODO: we may as well just ditch reading from HTOTAL and VTOTAL and use these instead
	if (htot <= 1 || htot <= hdisp)
	{
		u32 const hbp = (m_crtcregs[0x08 / 4] & 0xff00) >> 8;
		u32 const hsw = (m_crtcregs[0x08 / 4] & 0xff);
		u32 const hsfp = m_crtcregs[0x10 / 4] & 0xff;
		if (hbp == 0 && hsw == 0 && hsfp == 0)
			return;

		htot = hdisp + (hbp + 1) + (hsw + 1) + (hsfp + 1);
		m_crtcregs[0x20 / 4] = ((htot & 0x3ff) - 1);
	}

	// urachamu
	if (vtot == 0)
	{
		u32 const vbp = (m_crtcregs[0x08 / 4] & 0xff);
		if (vbp == 0)
			return;

		vtot = vdisp + (vbp + 1);
		m_crtcregs[0x24 / 4] = ((vtot & 0x7ff) - 1);
	}

	// ext vclk set up by Sealy games in menghong.cpp
	u32 pixel_clock = (BIT(m_crtcregs[0x04 / 4], 3)) ? 14318180/* TODO: Input clock? */ : m_ext_vclk;
	if (pixel_clock == 0)
		fatalerror("%s: Accessing external vclk in CRTC parameters, please set it up via setter in config\n", machine().describe_context());

	if (BIT(m_crtcregs[0x04 / 4], 7))
		pixel_clock *= 2;
	// TODO: divider setting = 0 is reserved, guess it just desyncs the signal?
	pixel_clock /= (m_crtcregs[0x04 / 4] & 7) + 1;

	LOGMASKED(LOG_CRTC, "DCLK divider %d\n",(m_crtcregs[0x04 / 4] & 7) + 1);
	LOGMASKED(LOG_CRTC, "VCLK select %d\n",(m_crtcregs[0x04 / 4] & 8));
	LOGMASKED(LOG_CRTC, "CBCLK divider %d\n",((m_crtcregs[0x04 / 4] & 0x70) >> 4) + 1);
	LOGMASKED(LOG_CRTC, "ivclk speed %d\n",(m_crtcregs[0x04 / 4] & 0x80));

	if (!interlace_mode)
	{
		vtot >>= 1;
		vtot += 1;
	}
	//else
	//  pixel_clock >>= 1;

	vtot += 9;

	LOGMASKED(LOG_CRTC, "total: %dx%d display: %dx%d clock: %d\n",htot, vtot, hdisp, vdisp, pixel_clock);

	rectangle const visarea(0, hdisp - 1, 0, vdisp - 1);
	m_screen->configure(htot, vtot, visarea, attotime::from_ticks(vtot * htot, pixel_clock));
}

// accessed by cross puzzle
u32 vrender0soc_device::sysid_r()
{
	// Device ID: VRender0+ -> 0x0a
	// Revision Number -> 0x00
	if (!machine().side_effects_disabled())
		logerror("%s: read SYSID\n", machine().describe_context());
	return 0x00000a00;
}

// -x-- ---- Main Clock select (0 -> External Clock)
// --xx x--- Reserved for Chip Test Mode (related to JTAG)
// ---- -xx- Local ROM Data Bus Width (01 -> 16 bit)
// ---- ---x Local Memory Bus Width (0 -> 16 bit)
u32 vrender0soc_device::cfgr_r()
{
	// TODO: convert as devcb_read8, these are strapping pins
	// only Cross Puzzle reads this so far so leaving a logerror
	if (!machine().side_effects_disabled())
		logerror("%s: read CFGR\n", machine().describe_context());
	return 0x00000041;
}

/*
 *
 * Video configuration
 *
 */

u32 vrender0soc_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (crt_is_blanked()) // Blank Screen
	{
		bitmap.fill(0, cliprect);
		return 0;
	}

	// TODO: chip can do superimposing, cfr. TCOL register in CRTC
	m_vr0vid->screen_update(screen, bitmap, cliprect);
	return 0;
}

void vrender0soc_device::screen_vblank(int state)
{
	// rising edge
	if (state)
	{
		if (crt_active_vblank_irq() == true)
		{
			int_req(IRQ_VBLANK);
			m_vr0vid->execute_flipping();
		}
	}
}
