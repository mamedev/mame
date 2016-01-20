// license:BSD-3-Clause
// copyright-holders:R. Belmont, ElSemi
#include "namcos2.h"
#include "video/c116.h"

#define NAMCOFL_HTOTAL      (288)   /* wrong */
#define NAMCOFL_HBSTART (288)
#define NAMCOFL_VTOTAL      (262)   /* needs to be checked */
#define NAMCOFL_VBSTART (224)

#define NAMCOFL_TILEMASKREGION      "tilemask"
#define NAMCOFL_TILEGFXREGION       "tile"
#define NAMCOFL_SPRITEGFXREGION "sprite"
#define NAMCOFL_ROTMASKREGION       "rotmask"
#define NAMCOFL_ROTGFXREGION        "rot"

#define NAMCOFL_TILEGFX     0
#define NAMCOFL_SPRITEGFX       1
#define NAMCOFL_ROTGFX          2

class namcofl_state : public namcos2_shared_state
{
public:
	namcofl_state(const machine_config &mconfig, device_type type, std::string tag)
		: namcos2_shared_state(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_mcu(*this,"mcu"),
		m_c116(*this,"c116"),
		m_shareram(*this, "shareram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_mcu;
	required_device<namco_c116_device> m_c116;
	emu_timer *m_raster_interrupt_timer;
	std::unique_ptr<UINT32[]> m_workram;
	required_shared_ptr<UINT16> m_shareram;
	UINT8 m_mcu_port6;
	UINT32 m_sprbank;

	DECLARE_READ32_MEMBER(fl_unk1_r);
	DECLARE_READ32_MEMBER(fl_network_r);
	DECLARE_READ32_MEMBER(namcofl_sysreg_r);
	DECLARE_WRITE32_MEMBER(namcofl_sysreg_w);
	DECLARE_WRITE8_MEMBER(namcofl_c116_w);
	DECLARE_READ32_MEMBER(namcofl_share_r);
	DECLARE_WRITE32_MEMBER(namcofl_share_w);
	DECLARE_WRITE16_MEMBER(mcu_shared_w);
	DECLARE_READ8_MEMBER(port6_r);
	DECLARE_WRITE8_MEMBER(port6_w);
	DECLARE_READ8_MEMBER(port7_r);
	DECLARE_READ8_MEMBER(dac7_r);
	DECLARE_READ8_MEMBER(dac6_r);
	DECLARE_READ8_MEMBER(dac5_r);
	DECLARE_READ8_MEMBER(dac4_r);
	DECLARE_READ8_MEMBER(dac3_r);
	DECLARE_READ8_MEMBER(dac2_r);
	DECLARE_READ8_MEMBER(dac1_r);
	DECLARE_READ8_MEMBER(dac0_r);
	DECLARE_WRITE32_MEMBER(namcofl_spritebank_w);
	DECLARE_DRIVER_INIT(speedrcr);
	DECLARE_DRIVER_INIT(finalapr);
	DECLARE_MACHINE_START(namcofl);
	DECLARE_MACHINE_RESET(namcofl);
	DECLARE_VIDEO_START(namcofl);
	UINT32 screen_update_namcofl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(network_interrupt_callback);
	TIMER_CALLBACK_MEMBER(vblank_interrupt_callback);
	TIMER_CALLBACK_MEMBER(raster_interrupt_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(mcu_irq0_cb);
	TIMER_DEVICE_CALLBACK_MEMBER(mcu_irq2_cb);
	TIMER_DEVICE_CALLBACK_MEMBER(mcu_adc_cb);
	void common_init();
};
