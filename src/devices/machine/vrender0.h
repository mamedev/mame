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

	void regs_map(address_map &map) ATTR_COLD;
	void set_channel_num(int ch) { m_channel_num = ch; }
	void set_parent(vrender0soc_device *parent) { m_parent = parent; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint32_t control_r();
	void control_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t baud_rate_div_r();
	void baud_rate_div_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t status_r();
	void transmit_buffer_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t receive_buffer_r(offs_t offset, uint32_t mem_mask = ~0);
	TIMER_CALLBACK_MEMBER( break_timer_cb );

	uint32_t m_ucon = 0; // control
	uint32_t m_ubdr = 0; // baud rate
	uint32_t m_ustat = 0; // status
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

	void regs_map(address_map &map) ATTR_COLD;
	void audiovideo_map(address_map &map) ATTR_COLD;
	void texture_map(address_map &map) ATTR_COLD;
	void frame_map(address_map &map) ATTR_COLD;
	template<class T> void set_host_cpu_tag(T &&tag) { m_host_cpu.set_tag(std::forward<T>(tag)); }
	void set_external_vclk(const uint32_t vclk) { m_ext_vclk = vclk; }
	void set_external_vclk(const XTAL vclk) { m_ext_vclk = vclk.value(); }
	bool crt_is_blanked() { return ((m_crtcregs[0] & 0x0200) == 0x0200); }
	bool crt_active_vblank_irq();
	void IntReq( int num );
	uint8_t irq_callback();
	bool irq_pending() { return m_intst; }
	void write_line_tx(int port, uint8_t value);
	template <int Port> auto tx_callback() { return write_tx[Port].bind(); }
	template <int Port> void rx_w(int state) { m_uart[Port]->rx_w((uint8_t)state); }

protected:
	// device-level overrides
	//virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_device <se3208_device> m_host_cpu;
	required_device <screen_device> m_screen;
	required_device <palette_device> m_palette;
	required_device <vr0video_device> m_vr0vid;
	required_device <vr0sound_device> m_vr0snd;
	required_device <speaker_device> m_speaker;
	required_device_array <vr0uart_device, 2> m_uart;
	required_shared_ptr <uint32_t> m_crtcregs;
	std::unique_ptr<uint16_t []> m_textureram;
	std::unique_ptr<uint16_t []> m_frameram;

	address_space *m_host_space = nullptr;
	uint32_t m_ext_vclk = 0;

	devcb_write_line::array<2> write_tx;

	// INTC
	uint32_t m_inten = 0;
	uint32_t inten_r();
	void inten_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint32_t intvec_r();
	void intvec_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint8_t m_IntHigh = 0;
	uint32_t m_intst = 0;
	uint32_t intst_r();
	void intst_w(uint32_t data);

	void soundirq_cb(int state);

	// Timer
	template<int Which> void tmcon_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	template<int Which> uint32_t tmcon_r();
	template<int Which> void tmcnt_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	template<int Which> uint16_t tmcnt_r();
	template<int Which> TIMER_CALLBACK_MEMBER(Timercb);

	uint32_t m_timer_control[4] = { 0, 0, 0, 0 };
	uint16_t m_timer_count[4] = { 0, 0, 0, 0 };
	emu_timer  *m_Timer[4] = { nullptr, nullptr, nullptr, nullptr };
	void TimerStart(int which);

	// DMAC
	template<int Which> uint32_t dmac_r();
	template<int Which> void dmac_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	template<int Which> uint32_t dmatc_r();
	template<int Which> void dmatc_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	template<int Which> uint32_t dmasa_r();
	template<int Which> void dmasa_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	template<int Which> uint32_t dmada_r();
	template<int Which> void dmada_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	inline int dma_setup_hold(uint8_t setting, uint8_t bitmask);
	struct {
		uint32_t src = 0;
		uint32_t dst = 0;
		uint32_t size = 0;
		uint32_t ctrl = 0;
	}m_dma[2];

	// CRTC
	uint32_t crtc_r(offs_t offset);
	void crtc_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void crtc_update();
	inline bool crt_is_interlaced();

	// Misc
	uint32_t sysid_r();
	uint32_t cfgr_r();

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);

	uint16_t textureram_r(offs_t offset);
	void textureram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t frameram_r(offs_t offset);
	void frameram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
};


// device type definition
DECLARE_DEVICE_TYPE(VRENDER0_SOC, vrender0soc_device)


#endif // MAME_MACHINE_VRENDER0_H
