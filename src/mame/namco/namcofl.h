// license:BSD-3-Clause
// copyright-holders:R. Belmont, ElSemi

#include "cpu/i960/i960.h"
#include "namcomcu.h"
#include "machine/timer.h"
#include "namco_c123tmap.h"
#include "namco_c116.h"
#include "namco_c169roz.h"
#include "namco_c355spr.h"

#include "emupal.h"
#include "screen.h"

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
		m_workram(*this, "workram"),
		m_shareram(*this, "shareram"),
		m_mainbank(*this, "mainbank"),
		m_in0(*this, "IN0"),
		m_in1(*this, "IN1"),
		m_in2(*this, "IN2"),
		m_misc(*this, "MISC"),
		m_accel(*this, "ACCEL"),
		m_brake(*this, "BRAKE"),
		m_wheel(*this, "WHEEL")
	{ }

	void namcofl(machine_config &config);

	void driver_init() override;

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<i960_cpu_device> m_maincpu;
	required_device<namco_c116_device> m_c116;
	required_device<screen_device> m_screen;
	required_device<namco_c123tmap_device> m_c123tmap;
	required_device<namco_c169roz_device> m_c169roz;
	required_device<namco_c355spr_device> m_c355spr;
	required_device<m37710_cpu_device> m_mcu;
	required_shared_ptr<uint32_t> m_workram;
	required_shared_ptr<uint32_t> m_shareram;
	memory_view m_mainbank;
	required_ioport m_in0;
	required_ioport m_in1;
	required_ioport m_in2;
	required_ioport m_misc;
	optional_ioport m_accel;
	optional_ioport m_brake;
	optional_ioport m_wheel;

	emu_timer *m_raster_interrupt_timer = nullptr;
	emu_timer *m_vblank_interrupt_timer = nullptr;
	emu_timer *m_network_interrupt_timer = nullptr;
	uint8_t m_mcu_port6 = 0;
	uint32_t m_sprbank = 0;

	uint32_t unk1_r();
	uint8_t network_r(offs_t offset);
	uint32_t sysreg_r();
	void sysreg_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void c116_w(offs_t offset, uint8_t data);
	uint16_t mcu_shared_r(offs_t offset);
	void mcu_shared_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint8_t port6_r();
	void port6_w(uint8_t data);
	uint8_t port7_r();
	uint8_t dac7_r();
	uint8_t dac6_r();
	uint8_t dac5_r();
	uint8_t dac4_r();
	uint8_t dac3_r();
	uint8_t dac2_r();
	uint8_t dac1_r();
	uint8_t dac0_r();
	void spritebank_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(network_interrupt_callback);
	TIMER_CALLBACK_MEMBER(vblank_interrupt_callback);
	TIMER_CALLBACK_MEMBER(raster_interrupt_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(mcu_irq0_cb);
	TIMER_DEVICE_CALLBACK_MEMBER(mcu_irq2_cb);
	int FLobjcode2tile(int code);
	void TilemapCB(uint16_t code, int *tile, int *mask);
	void RozCB(uint16_t code, int *tile, int *mask, int which);
	void namcoc75_am(address_map &map);
	void namcofl_mem(address_map &map);
};
