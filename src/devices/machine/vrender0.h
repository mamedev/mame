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
#include "diserial.h"

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

	void regs_map(address_map &map) ATTR_COLD;
	void set_channel_num(s32 ch) { m_channel_num = ch; }
	void set_parent(vrender0soc_device *parent) { m_parent = parent; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void tra_callback() override;
	virtual void tra_complete() override;
	virtual void rcv_complete() override;

private:
	u32 m_ucon = 0; // control
	u32 m_ubdr = 0; // baud rate
	u32 m_ustat = 0; // status
	util::fifo<u8, 16> m_urxb_fifo; // receive FIFO

	s32 m_channel_num = 0;
	vrender0soc_device *m_parent = nullptr;

	u32 control_r();
	void control_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 baud_rate_div_r();
	void baud_rate_div_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 status_r();
	void transmit_buffer_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 receive_buffer_r(offs_t offset, u32 mem_mask = ~0);
	TIMER_CALLBACK_MEMBER(break_timer_cb);

	void update_serial_config();
	inline u32 calculate_baud_rate();

	inline void tx_send_byte(u8 val);
};


// device type definition
DECLARE_DEVICE_TYPE(VRENDER0_UART, vr0uart_device)


class vrender0soc_device : public device_t, public device_mixer_interface
{
public:
	// construction/destruction
	vrender0soc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configurations
	template<class T> void set_host_space_tag(T &&tag, int spacenum) { m_host_space.set_tag(std::forward<T>(tag), spacenum); }
	void set_external_vclk(const u32 vclk) { m_ext_vclk = vclk; }
	void set_external_vclk(const XTAL vclk) { m_ext_vclk = vclk.value(); }
	auto int_callback() { return m_int_cb.bind(); }
	template <int Port> auto tx_callback() { return m_write_tx[Port].bind(); }
	template <int Port> void rx_w(int state) { m_uart[Port]->rx_w((u8)state); }

	// handlers
	bool crt_is_blanked() { return ((m_crtcregs[0] & 0x0200) == 0x0200); }
	bool crt_active_vblank_irq();
	void int_req(int num);
	u8 irq_callback();
	bool irq_pending() { return m_intst; }
	void write_line_tx(int port, u8 value);

	// address map
	void regs_map(address_map &map) ATTR_COLD;
	void audiovideo_map(address_map &map) ATTR_COLD;
	void texture_map(address_map &map) ATTR_COLD;
	void frame_map(address_map &map) ATTR_COLD;

protected:
	// device-level overrides
	//virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<vr0video_device> m_vr0vid;
	required_device<vr0sound_device> m_vr0snd;
	required_device_array<vr0uart_device, 2> m_uart;
	required_shared_ptr<u32> m_crtcregs;
	required_address_space m_host_space;
	memory_share_creator<u16> m_textureram;
	memory_share_creator<u16> m_frameram;

	u32 m_ext_vclk = 0;

	u32 m_inten = 0;
	u8 m_int_high = 0;
	u32 m_intst = 0;

	u32 m_timer_control[4]{0};
	u16 m_timer_count[4]{0};
	emu_timer  *m_timer[4]{nullptr};

	struct {
		u32 src = 0;
		u32 dst = 0;
		u32 size = 0;
		u32 ctrl = 0;
	} m_dma[2];

	devcb_write_line m_int_cb;
	devcb_write_line::array<2> m_write_tx;

	// INTC
	u32 inten_r();
	void inten_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	u32 intvec_r();
	void intvec_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	u32 intst_r();
	void intst_w(u32 data);

	void soundirq_cb(int state);

	// Timer
	template<int Which> void tmcon_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template<int Which> u32 tmcon_r();
	template<int Which> void tmcnt_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	template<int Which> u16 tmcnt_r();
	template<int Which> TIMER_CALLBACK_MEMBER(timer_cb);

	void timer_start(int which);

	// DMAC
	template<int Which> u32 dmac_r();
	template<int Which> void dmac_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template<int Which> u32 dmatc_r();
	template<int Which> void dmatc_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template<int Which> u32 dmasa_r();
	template<int Which> void dmasa_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template<int Which> u32 dmada_r();
	template<int Which> void dmada_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	inline int dma_setup_hold(u8 setting, u8 bitmask);

	// CRTC
	u32 crtc_r(offs_t offset);
	void crtc_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void crtc_update();
	inline bool crt_is_interlaced();

	// Misc
	u32 sysid_r();
	u32 cfgr_r();

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);

	u16 textureram_r(offs_t offset);
	void textureram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 frameram_r(offs_t offset);
	void frameram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
};


// device type definition
DECLARE_DEVICE_TYPE(VRENDER0_SOC, vrender0soc_device)


#endif // MAME_MACHINE_VRENDER0_H
