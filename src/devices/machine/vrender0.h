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

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vrender0soc_device

class vrender0soc_device : public device_t
{
public:
	// construction/destruction
	vrender0soc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void regs_map(address_map &map);
	template<class T> void set_host_cpu_tag(T &&tag) { m_host_cpu.set_tag(std::forward<T>(tag)); }
	template<class T> void set_host_screen_tag(T &&tag) { m_host_screen.set_tag(std::forward<T>(tag)); }
	bool crt_is_blanked() { return ((m_crtcregs[0] & 0x0200) == 0x0200); }
	bool crt_active_vblank_irq();
	void IntReq( int num );
	int irq_callback();
	auto idleskip_cb() { return m_idleskip_cb.bind(); }
	bool irq_pending() { return m_intst & m_inten; }

protected:
	// device-level overrides
	//virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_device <se3208_device> m_host_cpu;
	required_device <screen_device> m_host_screen;
	required_shared_ptr <uint32_t> m_crtcregs;
	devcb_write_line  m_idleskip_cb;

	address_space *m_host_space;
	// To move into SoC own device
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
};


// device type definition
DECLARE_DEVICE_TYPE(VRENDER0_SOC, vrender0soc_device)



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************


#endif // MAME_MACHINE_VRENDER0_H
