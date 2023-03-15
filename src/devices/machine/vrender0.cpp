// license:BSD-3-Clause
// copyright-holders:Angelo Salese, ElSemi
/***************************************************************************

    MagicEyes VRender0 SoC peripherals

    Device by Angelo Salese
    Based off original crystal.cpp by ElSemi

    TODO:
    - Improve encapsulation, still needs a few trampolines from host driver;
    - Proper PIO emulation;
    - Output CRTC border color;
    - Add VCLK select;

***************************************************************************/

#include "emu.h"
#include "vrender0.h"


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

vrender0soc_device::vrender0soc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VRENDER0_SOC, tag, owner, clock),
	m_host_cpu(*this, finder_base::DUMMY_TAG),
	m_screen(*this, "screen"),
	m_palette(*this, "palette"),
	m_vr0vid(*this, "vr0vid"),
	m_vr0snd(*this, "vr0snd"),
	m_lspeaker(*this, "lspeaker"),
	m_rspeaker(*this, "rspeaker"),
	m_uart(*this, "uart%u", 0),
	m_crtcregs(*this, "crtcregs"),
	write_tx(*this)
{
}

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
	map(0x03400, 0x037ff).rw(FUNC(vrender0soc_device::crtc_r), FUNC(vrender0soc_device::crtc_w)).share("crtcregs");
//  map(0x04000, 0x043ff)                            // RAMDAC & PLL
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
		VRENDER0_UART(config, uart, 3579500);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	// evolution soccer defaults
	m_screen->set_raw((XTAL(14'318'181)*2)/4, 455, 0, 320, 262, 0, 240);
	m_screen->set_screen_update(FUNC(vrender0soc_device::screen_update));
	m_screen->screen_vblank().set(FUNC(vrender0soc_device::screen_vblank));
	m_screen->set_palette(m_palette);

	VIDEO_VRENDER0(config, m_vr0vid, 14318180);
#ifdef IDLE_LOOP_SPEEDUP
	m_vr0vid->idleskip_cb().set(FUNC(vrender0soc_device::idle_skip_speedup_w));
#endif

	PALETTE(config, m_palette, palette_device::RGB_565);

	SPEAKER(config, m_lspeaker).front_left();
	SPEAKER(config, m_rspeaker).front_right();

	SOUND_VRENDER0(config, m_vr0snd, DERIVED_CLOCK(1,1)); // Correct?
	m_vr0snd->set_addrmap(vr0sound_device::AS_TEXTURE, &vrender0soc_device::texture_map);
	m_vr0snd->set_addrmap(vr0sound_device::AS_FRAME, &vrender0soc_device::frame_map);
	m_vr0snd->irq_callback().set(FUNC(vrender0soc_device::soundirq_cb));
	m_vr0snd->add_route(0, m_lspeaker, 1.0);
	m_vr0snd->add_route(1, m_rspeaker, 1.0);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vrender0soc_device::device_start()
{
	m_textureram = make_unique_clear<uint16_t []>(0x00800000/2);
	m_frameram = make_unique_clear<uint16_t []>(0x00800000/2);

	m_vr0vid->set_areas(m_textureram.get(), m_frameram.get());
	m_host_space = &m_host_cpu->space(AS_PROGRAM);

	if (this->clock() == 0)
		fatalerror("%s: bus clock not setup properly",this->tag());

	m_Timer[0] = timer_alloc(FUNC(vrender0soc_device::Timercb<0>), this);
	m_Timer[1] = timer_alloc(FUNC(vrender0soc_device::Timercb<1>), this);
	m_Timer[2] = timer_alloc(FUNC(vrender0soc_device::Timercb<2>), this);
	m_Timer[3] = timer_alloc(FUNC(vrender0soc_device::Timercb<3>), this);

	write_tx.resolve_all_safe();

	for (int i = 0; i < 2; i++)
	{
		m_uart[i]->set_channel_num(i);
		m_uart[i]->set_parent(this);
	}

	save_item(NAME(m_inten));
	save_item(NAME(m_intst));
	save_item(NAME(m_IntHigh));

	save_pointer(NAME(m_timer_control), 4);
	save_pointer(NAME(m_timer_count), 4);
	save_item(NAME(m_dma[0].src));
	save_item(NAME(m_dma[0].dst));
	save_item(NAME(m_dma[0].size));
	save_item(NAME(m_dma[0].ctrl));

	save_item(NAME(m_dma[1].ctrl));
	save_item(NAME(m_dma[1].src));
	save_item(NAME(m_dma[1].dst));
	save_item(NAME(m_dma[1].size));

#ifdef IDLE_LOOP_SPEEDUP
	save_item(NAME(m_FlipCntRead));
#endif
}

void vrender0soc_device::write_line_tx(int port, uint8_t value)
{
	//printf("callback %d %02x\n",port,value);
	write_tx[port & 1](value);
}



//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void vrender0soc_device::device_reset()
{
	// TODO: improve CRT defaults
	m_crtcregs[1] = 0x0000002a;

	//m_FlipCount = 0;
	m_IntHigh = 0;

	m_dma[0].ctrl = 0;
	m_dma[1].ctrl = 0;

	for (int i = 0; i < 4; i++)
	{
		m_timer_control[i] = 0xff << 8;
		m_Timer[i]->adjust(attotime::never);
	}

#ifdef IDLE_LOOP_SPEEDUP
	m_FlipCntRead = 0;
#endif
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

/*
 *
 * Texture/FrameRAM 16-bit trampolines
 *
 */

uint16_t vrender0soc_device::textureram_r(offs_t offset)
{
	return m_textureram[offset];
}

void vrender0soc_device::textureram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_textureram[offset]);
}

uint16_t vrender0soc_device::frameram_r(offs_t offset)
{
	return m_frameram[offset];
}

void vrender0soc_device::frameram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_frameram[offset]);
}

/*
 *
 * INT Controller
 *
 */

uint32_t vrender0soc_device::intvec_r()
{
	return (m_IntHigh & 7) << 8;
}

void vrender0soc_device::intvec_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_intst &= ~(1 << (data & 0x1f));
		if (!m_intst)
			m_host_cpu->set_input_line(SE3208_INT, CLEAR_LINE);
	}
	if (ACCESSING_BITS_8_15)
		m_IntHigh = (data >> 8) & 7;
}

uint32_t vrender0soc_device::inten_r()
{
	return m_inten;
}

void vrender0soc_device::inten_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_inten);
	// P'S Attack has a timer 0 irq service with no call to intvec_w but just this
	m_intst &= m_inten;
	if (!m_intst)
		m_host_cpu->set_input_line(SE3208_INT, CLEAR_LINE);
}

uint32_t vrender0soc_device::intst_r()
{
	return m_intst;
}

void vrender0soc_device::intst_w(uint32_t data)
{
	// TODO: contradicts with documentation, games writes to this?
	// ...
}

void vrender0soc_device::IntReq( int num )
{
	if (m_inten & (1 << num))
	{
		m_intst |= (1 << num);
		m_host_cpu->set_input_line(SE3208_INT, ASSERT_LINE);
	}

#ifdef IDLE_LOOP_SPEEDUP
	idle_skip_resume_w(ASSERT_LINE);
#endif
}


uint8_t vrender0soc_device::irq_callback()
{
	for (int i = 0; i < 32; ++i)
	{
		if (BIT(m_intst, i))
		{
			return (m_IntHigh << 5) | i;
		}
	}
	return 0;       //This should never happen
}


WRITE_LINE_MEMBER(vrender0soc_device::soundirq_cb)
{
	if (state)
	{
		IntReq(2);
	}
}

/*
 *
 * Timer
 *
 */


void vrender0soc_device::TimerStart(int which)
{
	int PD = (m_timer_control[which] >> 8) & 0xff;
	int TCV = m_timer_count[which] & 0xffff;
	// TODO: documentation claims this is bus clock, may be slower than the CPU itself
	attotime period = attotime::from_hz(this->clock()) * ((PD + 1) * (TCV + 1));
	m_Timer[which]->adjust(period);

//  printf("timer %d start, PD = %x TCV = %x period = %s\n", which, PD, TCV, period.as_string());
}

template<int Which>
TIMER_CALLBACK_MEMBER(vrender0soc_device::Timercb)
{
	static const int num[] = { 0, 1, 9, 10 };

	if (m_timer_control[Which] & 2)
		TimerStart(Which);
	else
		m_timer_control[Which] &= ~1;

	IntReq(num[Which]);
}

template<int Which>
uint32_t vrender0soc_device::tmcon_r()
{
	return m_timer_control[Which];
}

template<int Which>
void vrender0soc_device::tmcon_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t old = m_timer_control[Which];
	data = COMBINE_DATA(&m_timer_control[Which]);

	if ((data ^ old) & 1)
	{
		if (data & 1)
		{
			TimerStart(Which);
		}
		else
		{
			// Timer stop
			m_Timer[Which]->adjust(attotime::never);
//          printf("timer %d stop\n", Which);
		}
	}
}

template<int Which>
uint16_t vrender0soc_device::tmcnt_r()
{
	return m_timer_count[Which] & 0xffff;
}

template<int Which>
void vrender0soc_device::tmcnt_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_timer_count[Which]);
}

/*
 *
 * DMA Controller
 *
 */

// helper
// bit 5 and bit 3 of the DMA control don't increment source/destination addresses if enabled.
// At the time of writing P's Attack is the only SW that uses this feature,
// in a work RAM to area $4500000 transfer, probably to extend something ...
inline int vrender0soc_device::dma_setup_hold(uint8_t setting, uint8_t bitmask)
{
	return setting & bitmask ? 0 : (setting & 2) ? 4 : (1 << (setting & 1));
}

template<int Which> uint32_t vrender0soc_device::dmasa_r() { return m_dma[Which].src; }
template<int Which> void vrender0soc_device::dmasa_w(offs_t offset, uint32_t data, uint32_t mem_mask) { COMBINE_DATA(&m_dma[Which].src); }
template<int Which> uint32_t vrender0soc_device::dmada_r() { return m_dma[Which].dst; }
template<int Which> void vrender0soc_device::dmada_w(offs_t offset, uint32_t data, uint32_t mem_mask) { COMBINE_DATA(&m_dma[Which].dst); }
template<int Which> uint32_t vrender0soc_device::dmatc_r() { return m_dma[Which].size; }
template<int Which> void vrender0soc_device::dmatc_w(offs_t offset, uint32_t data, uint32_t mem_mask) { COMBINE_DATA(&m_dma[Which].size); }
template<int Which> uint32_t vrender0soc_device::dmac_r() { return m_dma[Which].ctrl; }
template<int Which>
void vrender0soc_device::dmac_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (((data ^ m_dma[Which].ctrl) & (1 << 10)) && (data & (1 << 10)))   //DMAOn
	{
		uint32_t const CTR = data;
		uint32_t const SRC = m_dma[Which].src;
		uint32_t const DST = m_dma[Which].dst;
		uint32_t const CNT = m_dma[Which].size;
		const int src_inc = dma_setup_hold(CTR, 0x20);
		const int dst_inc = dma_setup_hold(CTR, 0x08);

		if ((CTR & 0xd4) != 0)
			popmessage("DMA%d with unhandled mode %02x, contact MAMEdev",Which,CTR);

		if (CTR & 0x2)  //32 bits
		{
			for (int i = 0; i < CNT; ++i)
			{
				uint32_t v = m_host_space->read_dword(SRC + i * src_inc);
				m_host_space->write_dword(DST + i * dst_inc, v);
			}
		}
		else if (CTR & 0x1) //16 bits
		{
			for (int i = 0; i < CNT; ++i)
			{
				uint16_t v = m_host_space->read_word(SRC + i * src_inc);
				m_host_space->write_word(DST + i * dst_inc, v);
			}
		}
		else    //8 bits
		{
			for (int i = 0; i < CNT; ++i)
			{
				uint8_t v = m_host_space->read_byte(SRC + i * src_inc);
				m_host_space->write_byte(DST + i * dst_inc, v);
			}
		}
		data &= ~(1 << 10);
		// TODO: insta-DMA
		m_dma[Which].size = 0;
		IntReq(7 + Which);
	}
	COMBINE_DATA(&m_dma[Which].ctrl);
}

/*
 *
 * CRT Controller
 *
 */

uint32_t vrender0soc_device::crtc_r(offs_t offset)
{
	uint32_t res = m_crtcregs[offset];
	uint32_t hdisp = (m_crtcregs[0x0c / 4] + 1);
	uint32_t vdisp = (m_crtcregs[0x1c / 4] + 1);
	switch (offset)
	{
		case 0: // CRTC Status / Mode
			if (crt_is_interlaced()) // Interlace
				vdisp <<= 1;

			if (m_screen->vpos() <= vdisp) // Vertical display enable status
				res |=  0x4000;

			if (m_screen->hpos() > hdisp) // horizontal & vertical blank period
				res &= ~0x2000;
			else
				res |=  0x2000;

			break;
		default:
			break;
	}
	return res;
}

void vrender0soc_device::crtc_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (((m_crtcregs[0] & 0x0100) == 0x0100) && (offset > 0) && (offset < 0x28/4)) // Write protect
		return;

	uint32_t old = m_crtcregs[offset];
	switch (offset * 4)
	{
		case 0: // CRTC Status / Mode Register (CRTMOD)
			mem_mask &= ~0xfffffc00; // Bit 31-10 Reserved
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
		case 0x38: // Light Pen 0 X Register (LIGHT0X)
			mem_mask &= ~0xfffff800; // Bit 31-11 Reserved
			break;
		case 0x3c: // Light Pen 0 Y Register (LIGHT0Y)
			mem_mask &= ~0xfffffe00; // Bit 31-9 Reserved
			break;
		case 0x40: // Light Pen 1 X Register (LIGHT1X)
			mem_mask &= ~0xfffff800; // Bit 31-11 Reserved
			break;
		case 0x44: // Light Pen 1 Y Register (LIGHT1Y)
			mem_mask &= ~0xfffffe00; // Bit 31-9 Reserved
			break;
		case 0x48: // Light Pen Input Control Register (LIGHTC)
			mem_mask &= ~0xfffffffc; // Bit 31-2 Reserved
			break;
		default:
			return;
	}
	COMBINE_DATA(&m_crtcregs[offset]);
	if (old ^ m_crtcregs[offset])
		crtc_update();

}

inline bool vrender0soc_device::crt_is_interlaced()
{
	return (m_crtcregs[0x30 / 4] & 1) == 0;
}

bool vrender0soc_device::crt_active_vblank_irq()
{
	if (crt_is_interlaced() == false)
		return true;

	// bit 3 of CRTC reg -> select display start even/odd fields
	return (m_screen->frame_number() & 1) ^ ((m_crtcregs[0] & 8) >> 3);
}

void vrender0soc_device::crtc_update()
{
	uint32_t hdisp = m_crtcregs[0x0c / 4] + 1;
	uint32_t vdisp = m_crtcregs[0x1c / 4];
	if (hdisp == 0 || vdisp == 0)
		return;

	bool interlace_mode = crt_is_interlaced();

	if (interlace_mode)
		vdisp <<= 1;

	uint32_t htot = (m_crtcregs[0x20 / 4] & 0x3ff) + 1;
	uint32_t vtot = (m_crtcregs[0x24 / 4] & 0x7ff);

	// adjust htotal in case it's not setup by the game
	// (datasheet mentions that it can be done automatically shrug):
	// - the two Sealy games do that
	// - Cross Puzzle sets up an HTotal of 400 with 640x480 display
	// - donghaer writes a 0 to the htot when entering interlace mode
	// TODO: we may as well just ditch reading from HTOTAL and VTOTAL and use these instead
	if (htot <= 1 || htot <= hdisp)
	{
		uint32_t hbp = (m_crtcregs[0x08 / 4] & 0xff00) >> 8;
		uint32_t hsw = (m_crtcregs[0x08 / 4] & 0xff);
		uint32_t hsfp = m_crtcregs[0x10 / 4] & 0xff;
		if (hbp == 0 && hsw == 0 && hsfp == 0)
			return;

		htot = hdisp + (hbp+1) + (hsw+1) + (hsfp+1);
		m_crtcregs[0x20 / 4] = ((htot & 0x3ff) - 1);
	}

	// urachamu
	if (vtot == 0)
	{
		uint32_t vbp = (m_crtcregs[0x08 / 4] & 0xff);
		if (vbp == 0)
			return;

		vtot = vdisp + (vbp + 1);
		m_crtcregs[0x24 / 4] = ((vtot & 0x7ff) - 1);
	}

	// ext vclk set up by Sealy games in menghong.cpp
	uint32_t pixel_clock = (BIT(m_crtcregs[0x04 / 4], 3)) ? 14318180 : m_ext_vclk;
	if (pixel_clock == 0)
		fatalerror("%s: Accessing external vclk in CRTC parameters, please set it up via setter in config\n",this->tag());

	if (BIT(m_crtcregs[0x04 / 4], 7))
		pixel_clock *= 2;
	// TODO: divider setting = 0 is reserved, guess it just desyncs the signal?
	pixel_clock /= (m_crtcregs[0x04 / 4] & 7) + 1;

	//printf("DCLK divider %d\n",(m_crtcregs[0x04 / 4] & 7) + 1);
	//printf("VCLK select %d\n",(m_crtcregs[0x04 / 4] & 8));
	//printf("CBCLK divider %d\n",((m_crtcregs[0x04 / 4] & 0x70) >> 4) + 1);
	//printf("ivclk speed %d\n",(m_crtcregs[0x04 / 4] & 0x80));

	if (interlace_mode == false)
	{
		vtot >>= 1;
		vtot += 1;
	}
	//else
	//  pixel_clock >>= 1;


	vtot += 9;

	//printf("%dX%d %dX%d %d\n",htot, vtot, hdisp, vdisp, pixel_clock);

	rectangle const visarea(0, hdisp - 1, 0, vdisp - 1);
	m_screen->configure(htot, vtot, visarea, HZ_TO_ATTOSECONDS(pixel_clock) * vtot * htot);
}

// accessed by cross puzzle
uint32_t vrender0soc_device::sysid_r()
{
	// Device ID: VRender0+ -> 0x0a
	// Revision Number -> 0x00
	logerror("%s: read SYSID\n",this->tag());
	return 0x00000a00;
}

uint32_t vrender0soc_device::cfgr_r()
{
	// TODO: this truly needs real HW verification,
	//       only Cross Puzzle reads this so far so leaving a logerror
	// -x-- ---- Main Clock select (0 -> External Clock)
	// --xx x--- Reserved for Chip Test Mode
	// ---- -xx- Local ROM Data Bus Width (01 -> 16 bit)
	// ---- ---x Local Memory Bus Width (0 -> 16 bit)
	logerror("%s: read CFGR\n",this->tag());
	return 0x00000041;
}

/*
 *
 * Video configuration
 *
 */

uint32_t vrender0soc_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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

WRITE_LINE_MEMBER(vrender0soc_device::screen_vblank)
{
	// rising edge
	if (state)
	{
		if (crt_active_vblank_irq() == true)
			IntReq(24);      //VRender0 VBlank

		m_vr0vid->execute_flipping();
	}
}

/*
 *
 * Hacks
 *
 */

#ifdef IDLE_LOOP_SPEEDUP
WRITE_LINE_MEMBER(vrender0soc_device::idle_skip_resume_w)
{
	m_FlipCntRead = 0;
	m_host_cpu->resume(SUSPEND_REASON_SPIN);
}

WRITE_LINE_MEMBER(vrender0soc_device::idle_skip_speedup_w)
{
	m_FlipCntRead++;
	if (m_FlipCntRead >= 16 && irq_pending() == false && state == ASSERT_LINE)
		m_host_cpu->suspend(SUSPEND_REASON_SPIN, 1);
}
#endif
