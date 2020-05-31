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
	vr0uart_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

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

	u32 m_ucon = 0; // control
	u32 m_ubdr = 0; // baud rate
	u32 m_ustat = 0; // status
	util::fifo<u8, 16> m_urxb_fifo; // receive FIFO

	void update_serial_config();
	inline u32 calculate_baud_rate();

	virtual void tra_callback() override;
	virtual void tra_complete() override;
	virtual void rcv_complete() override;

	inline void tx_send_byte(u8 val);
	int m_channel_num;
	vrender0soc_device *m_parent;
};


// device type definition
DECLARE_DEVICE_TYPE(VRENDER0_UART, vr0uart_device)


class vrender0soc_device : public device_t
{
public:
	// construction/destruction
	vrender0soc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void regs_map(address_map &map);
	void audiovideo_map(address_map &map);
	void texture_map(address_map &map);
	void frame_map(address_map &map);
	template<class T> void set_host_cpu_tag(T &&tag) { m_host_cpu.set_tag(std::forward<T>(tag)); }
	void set_external_vclk(const u32 vclk) { m_ext_vclk = vclk; }
	void set_external_vclk(const XTAL vclk) { m_ext_vclk = vclk.value(); }
	bool crt_is_blanked() { return ((m_crtcregs[0] & 0x0200) == 0x0200); }
	bool crt_active_vblank_irq();
	void IntReq( int num );
	u8 irq_callback();
	bool irq_pending() { return m_intst; }
	void write_line_tx(int port, u8 value);
	template <int Port> auto tx_callback() { return write_tx[Port].bind(); }
	template <int Port> DECLARE_WRITE_LINE_MEMBER(rx_w) { m_uart[Port]->rx_w((u8)state); }

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
	required_shared_ptr <u32> m_crtcregs;
	u16 *m_textureram = nullptr;
	u16 *m_frameram = nullptr;

	address_space *m_host_space = nullptr;
	u32 m_ext_vclk = 0;

	devcb_write_line::array<2> write_tx;

	// INTC
	u32 m_inten = 0;
	u32 inten_r();
	void inten_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	u32 intvec_r();
	void intvec_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	u8 m_IntHigh = 0;
	u32 m_intst = 0;
	u32 intst_r();
	void intst_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	DECLARE_WRITE_LINE_MEMBER(soundirq_cb);

	// Timer
	template<int Which> void tmcon_w(offs_t offset, u32 data, u32 mem_mask);
	template<int Which> u32 tmcon_r();
	template<int Which> void tmcnt_w(offs_t offset, u16 data, u16 mem_mask);
	template<int Which> u16 tmcnt_r();
	TIMER_CALLBACK_MEMBER(Timercb);

	u32 m_timer_control[4] = { 0, 0, 0, 0 };
	u16 m_timer_count[4] = { 0, 0, 0, 0 };
	emu_timer  *m_Timer[4] = { nullptr, nullptr, nullptr, nullptr };
	void TimerStart(int which);

	// DMAC
	// register map
	enum DMAC : u32
	{
		DMA_ENABLE         = (1 << 10), // DMA enable
		DMA_POLARITY       = (1 << 9), // DMA polarity, not implemented
		DMA_INTERNAL_WRITE = (1 << 8), // Enable writing DMASA, DMADA register and internal counter simultaneously
		DMA_REPEAT         = (1 << 7), // DMA repeat mode
		DMA_RELOAD_ADDR    = (1 << 6), // Reload address in DMA repeat mode
		DMA_SRCHOLD        = (1 << 5), // Hold source address
		DMA_SRCDEC         = (1 << 4), // Decrease source address
		DMA_DSTHOLD        = (1 << 3), // Hold destination address
		DMA_DSTDEC         = (1 << 2), // Decrease destination address
		DMA_32BIT          = (1 << 1), // 32 bit transfer
		DMA_16BIT          = (1 << 0) // 16 bit transfer
	};
	template<int Which> u32 dmac_r();
	template<int Which> void dmac_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template<int Which> u32 dmatc_r();
	template<int Which> void dmatc_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template<int Which> u32 dmasa_r();
	template<int Which> void dmasa_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template<int Which> u32 dmada_r();
	template<int Which> void dmada_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	struct {
		u32 src = 0;
		u32 dst = 0;
		u32 size = 0;
		u32 ctrl = DMA_INTERNAL_WRITE;
		u32 int_src = 0;
		u32 int_dst = 0;
		u32 int_cnt = 0;
		emu_timer *timer = nullptr;
	}m_dma[2];
	TIMER_CALLBACK_MEMBER(dma_timer_cb);

	// CRTC
	u32 crtc_r(offs_t offset);
	void crtc_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void crtc_update();
	inline bool crt_is_interlaced();

	// Misc
	u32 sysid_r();
	u32 cfgr_r();

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank);

	u16 textureram_r(offs_t offset);
	void textureram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 frameram_r(offs_t offset);
	void frameram_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	// Hacks
#ifdef IDLE_LOOP_SPEEDUP
	u8     m_FlipCntRead = 0;
	DECLARE_WRITE_LINE_MEMBER(idle_skip_resume_w);
	DECLARE_WRITE_LINE_MEMBER(idle_skip_speedup_w);
#endif
};


// device type definition
DECLARE_DEVICE_TYPE(VRENDER0_SOC, vrender0soc_device)


#endif // MAME_MACHINE_VRENDER0_H
