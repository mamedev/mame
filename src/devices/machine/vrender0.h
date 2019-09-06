// license:BSD-3-Clause
// copyright-holders:Angelo Salese, ElSemi
/***************************************************************************

    MagicEyes VRender0 SoC

***************************************************************************/

#ifndef MAME_MACHINE_VRENDER0_H
#define MAME_MACHINE_VRENDER0_H

#pragma once

#include "cpu/se3208/se3208.h"
#include "screen.h"
#include "video/vrender0.h"
#include "sound/vrender0.h"
#include "emupal.h"
#include "speaker.h"
#include "diserial.h"

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define IDLE_LOOP_SPEEDUP



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vr0uart_device

class vrender0soc_device;

class vr0uart_device : public device_t,
					   public device_serial_interface
{
public:
	// construction/destruction
	vr0uart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void regs_map(address_map &map);
	void set_channel_num(int ch) { m_channel_num = ch; }
	void set_parent(vrender0soc_device *parent) { m_parent = parent; }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	DECLARE_READ32_MEMBER( control_r );
	DECLARE_WRITE32_MEMBER( control_w );
	DECLARE_READ32_MEMBER( baud_rate_div_r );
	DECLARE_WRITE32_MEMBER( baud_rate_div_w );
	DECLARE_READ32_MEMBER( status_r ); 
	DECLARE_WRITE32_MEMBER( transmit_buffer_w );
	DECLARE_READ32_MEMBER( receive_buffer_r );
	TIMER_CALLBACK_MEMBER( break_timer_cb );
	
	uint32_t m_ucon; // control
	uint32_t m_ubdr; // baud rate
	uint32_t m_ustat; // status
	util::fifo<uint8_t, 16> m_urxb_fifo; // receive FIFO
	
	void update_serial_config();
	inline uint32_t calculate_baud_rate();
	
	virtual void tra_callback() override;
	virtual void tra_complete() override;
	virtual void rcv_complete() override;

	inline void tx_send_byte(uint8_t val);
	int m_channel_num;	
	vrender0soc_device *m_parent;
};


// device type definition
DECLARE_DEVICE_TYPE(VRENDER0_UART, vr0uart_device)


class vrender0soc_device : public device_t
{
public:
	// construction/destruction
	vrender0soc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void regs_map(address_map &map);
	void audiovideo_map(address_map &map);
	template<class T> void set_host_cpu_tag(T &&tag) { m_host_cpu.set_tag(std::forward<T>(tag)); }
	void set_external_vclk(const uint32_t vclk) { m_ext_vclk = vclk; } 
	void set_external_vclk(const XTAL vclk) { m_ext_vclk = vclk.value(); } 
	bool crt_is_blanked() { return ((m_crtcregs[0] & 0x0200) == 0x0200); }
	bool crt_active_vblank_irq();
	void IntReq( int num );
	int irq_callback();
	bool irq_pending() { return m_intst; }
	void write_line_tx(int port, uint8_t value);
	template <int Port> auto tx_callback() { return write_tx[Port].bind(); }
	template <int Port> DECLARE_WRITE_LINE_MEMBER(rx_w) { m_uart[Port]->rx_w((uint8_t)state); }
	
protected:
	// device-level overrides
	//virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_device <se3208_device> m_host_cpu;
	required_device <screen_device> m_screen;
	required_device <palette_device> m_palette;
	required_device <vr0video_device> m_vr0vid;
	required_device <vr0sound_device> m_vr0snd;
	required_device <speaker_device> m_lspeaker;
	required_device <speaker_device> m_rspeaker;
	required_device_array <vr0uart_device, 2> m_uart;
	required_shared_ptr <uint32_t> m_crtcregs;
	uint16_t *m_textureram;
	uint16_t *m_frameram;
	
	address_space *m_host_space;
	uint32_t m_ext_vclk;

	devcb_write_line write_tx[2];

	// INTC
	uint32_t m_inten;
	DECLARE_READ32_MEMBER(inten_r);
	DECLARE_WRITE32_MEMBER(inten_w);

	DECLARE_READ32_MEMBER(intvec_r);
	DECLARE_WRITE32_MEMBER(intvec_w);

	uint8_t m_IntHigh;
	uint32_t m_intst;
	DECLARE_READ32_MEMBER(intst_r);
	DECLARE_WRITE32_MEMBER(intst_w);

	// Timer
	template<int Which> DECLARE_WRITE32_MEMBER(tmcon_w);
	template<int Which> DECLARE_READ32_MEMBER(tmcon_r);
	template<int Which> DECLARE_WRITE16_MEMBER(tmcnt_w);
	template<int Which> DECLARE_READ16_MEMBER(tmcnt_r);
	TIMER_CALLBACK_MEMBER(Timercb);

	uint32_t m_timer_control[4];
	uint16_t m_timer_count[4];
	emu_timer  *m_Timer[4];
	void TimerStart(int which);

	// DMAC
	template<int Which> DECLARE_READ32_MEMBER(dmac_r);
	template<int Which> DECLARE_WRITE32_MEMBER(dmac_w);
	template<int Which> DECLARE_READ32_MEMBER(dmatc_r);
	template<int Which> DECLARE_WRITE32_MEMBER(dmatc_w);
	template<int Which> DECLARE_READ32_MEMBER(dmasa_r);
	template<int Which> DECLARE_WRITE32_MEMBER(dmasa_w);
	template<int Which> DECLARE_READ32_MEMBER(dmada_r);
	template<int Which> DECLARE_WRITE32_MEMBER(dmada_w);
	inline int dma_setup_hold(uint8_t setting, uint8_t bitmask);
	struct {
		uint32_t src;
		uint32_t dst;
		uint32_t size;
		uint32_t ctrl;
	}m_dma[2];

	// CRTC
	DECLARE_READ32_MEMBER(crtc_r);
	DECLARE_WRITE32_MEMBER(crtc_w);
	void crtc_update();
	inline bool crt_is_interlaced();

	// Misc
	DECLARE_READ32_MEMBER( sysid_r );
	DECLARE_READ32_MEMBER( cfgr_r );
	
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank);
	
	DECLARE_READ16_MEMBER( textureram_r );
	DECLARE_WRITE16_MEMBER( textureram_w );
	DECLARE_READ16_MEMBER( frameram_r );
	DECLARE_WRITE16_MEMBER( frameram_w );

	// Hacks
#ifdef IDLE_LOOP_SPEEDUP
	uint8_t     m_FlipCntRead;
	DECLARE_WRITE_LINE_MEMBER(idle_skip_resume_w);
	DECLARE_WRITE_LINE_MEMBER(idle_skip_speedup_w);
#endif
};


// device type definition
DECLARE_DEVICE_TYPE(VRENDER0_SOC, vrender0soc_device)



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************


#endif // MAME_MACHINE_VRENDER0_H
