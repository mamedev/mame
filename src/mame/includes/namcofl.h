// license:BSD-3-Clause
// copyright-holders:R. Belmont, ElSemi

#include "machine/namcomcu.h"
#include "machine/timer.h"
#include "screen.h"
#include "video/namco_c123tmap.h"
#include "video/namco_c116.h"
#include "video/namco_c169roz.h"
#include "video/namco_c355spr.h"
#include "emupal.h"

#define NAMCOFL_HTOTAL      (288)   /* wrong */
#define NAMCOFL_HBSTART (288)
#define NAMCOFL_VTOTAL      (262)   /* needs to be checked */
#define NAMCOFL_VBSTART (224)

#define NAMCOFL_SPRITEGFX       0

class namcofl_state : public driver_device
{
public:
	namcofl_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_c116(*this, "c116"),
		m_screen(*this, "screen"),
		m_c123tmap(*this, "c123tmap"),
		m_c169roz(*this, "c169roz"),
		m_c355spr(*this, "c355spr"),
		m_mcu(*this, "mcu"),
		m_in0(*this, "IN0"),
		m_in1(*this, "IN1"),
		m_in2(*this, "IN2"),
		m_misc(*this, "MISC"),
		m_accel(*this, "ACCEL"),
		m_brake(*this, "BRAKE"),
		m_wheel(*this, "WHEEL"),
		m_shareram(*this, "shareram", 32) { }

	void namcofl(machine_config &config);

	void init_speedrcr();
	void init_finalapr();

private:
	required_device<cpu_device> m_maincpu;
	required_device<namco_c116_device> m_c116;
	required_device<screen_device> m_screen;
	required_device<namco_c123tmap_device> m_c123tmap;
	required_device<namco_c169roz_device> m_c169roz;
	required_device<namco_c355spr_device> m_c355spr;
	required_device<m37710_cpu_device> m_mcu;
	required_ioport m_in0;
	required_ioport m_in1;
	required_ioport m_in2;
	required_ioport m_misc;
	optional_ioport m_accel;
	optional_ioport m_brake;
	optional_ioport m_wheel;
	emu_timer *m_raster_interrupt_timer;
	emu_timer *m_vblank_interrupt_timer;
	emu_timer *m_network_interrupt_timer;
	std::unique_ptr<uint32_t[]> m_workram;
	required_shared_ptr<uint16_t> m_shareram;
	uint8_t m_mcu_port6;
	uint32_t m_sprbank;

	DECLARE_READ32_MEMBER(fl_unk1_r);
	DECLARE_READ32_MEMBER(fl_network_r);
	DECLARE_READ32_MEMBER(namcofl_sysreg_r);
	DECLARE_WRITE32_MEMBER(namcofl_sysreg_w);
	DECLARE_WRITE8_MEMBER(namcofl_c116_w);
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
	DECLARE_MACHINE_START(namcofl);
	DECLARE_MACHINE_RESET(namcofl);
	DECLARE_VIDEO_START(namcofl);
	uint32_t screen_update_namcofl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(network_interrupt_callback);
	TIMER_CALLBACK_MEMBER(vblank_interrupt_callback);
	TIMER_CALLBACK_MEMBER(raster_interrupt_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(mcu_irq0_cb);
	TIMER_DEVICE_CALLBACK_MEMBER(mcu_irq2_cb);
	TIMER_DEVICE_CALLBACK_MEMBER(mcu_adc_cb);
	void common_init();
	int FLobjcode2tile(int code);
	void TilemapCB(uint16_t code, int *tile, int *mask);
	void RozCB(uint16_t code, int *tile, int *mask, int which);
	void namcoc75_am(address_map &map);
	void namcofl_mem(address_map &map);
};
