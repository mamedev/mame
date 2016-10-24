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
	namcofl_state(const machine_config &mconfig, device_type type, const char *tag)
		: namcos2_shared_state(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_mcu(*this,"mcu"),
		m_c116(*this,"c116"),
		m_in0(*this, "IN0"),
		m_in1(*this, "IN1"),
		m_in2(*this, "IN2"),
		m_misc(*this, "MISC"),
		m_accel(*this, "ACCEL"),
		m_brake(*this, "BRAKE"),
		m_wheel(*this, "WHEEL"),
		m_shareram(*this, "shareram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_mcu;
	required_device<namco_c116_device> m_c116;
	required_ioport m_in0;
	required_ioport m_in1;
	required_ioport m_in2;
	required_ioport m_misc;
	optional_ioport m_accel;
	optional_ioport m_brake;
	optional_ioport m_wheel;
	emu_timer *m_raster_interrupt_timer;
	std::unique_ptr<uint32_t[]> m_workram;
	required_shared_ptr<uint16_t> m_shareram;
	uint8_t m_mcu_port6;
	uint32_t m_sprbank;

	uint32_t fl_unk1_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t fl_network_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t namcofl_sysreg_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void namcofl_sysreg_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void namcofl_c116_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint32_t namcofl_share_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void namcofl_share_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void mcu_shared_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t port6_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void port6_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t port7_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t dac7_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t dac6_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t dac5_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t dac4_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t dac3_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t dac2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t dac1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t dac0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void namcofl_spritebank_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void init_speedrcr();
	void init_finalapr();
	void machine_start_namcofl();
	void machine_reset_namcofl();
	void video_start_namcofl();
	uint32_t screen_update_namcofl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void network_interrupt_callback(void *ptr, int32_t param);
	void vblank_interrupt_callback(void *ptr, int32_t param);
	void raster_interrupt_callback(void *ptr, int32_t param);
	void mcu_irq0_cb(timer_device &timer, void *ptr, int32_t param);
	void mcu_irq2_cb(timer_device &timer, void *ptr, int32_t param);
	void mcu_adc_cb(timer_device &timer, void *ptr, int32_t param);
	void common_init();
};
