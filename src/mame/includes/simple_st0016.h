// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina

#include "machine/st0016.h"
#include "machine/timer.h"
#include "screen.h"

class st0016_state : public driver_device
{
public:
	st0016_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_subcpu(*this, "sub"),
		m_screen(*this, "screen"),
		m_mainbank(*this, "mainbank")
	{ }

	void st0016(machine_config &config);
	void renju(machine_config &config);
	void mayjinsn(machine_config &config);

	void init_nratechu();
	void init_mayjinsn();
	void init_mayjisn2();
	void init_renju();

private:
	int mux_port;
	// uint32_t m_st0016_rom_bank;

	required_device<st0016_cpu_device> m_maincpu;
	optional_device<cpu_device> m_subcpu;
	required_device<screen_device> m_screen;

	required_memory_bank m_mainbank;

	uint8_t mux_r();
	void mux_select_w(uint8_t data);
	uint32_t latch32_r(offs_t offset);
	void latch32_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint8_t latch8_r(offs_t offset);
	void latch8_w(offs_t offset, uint8_t data);

	void st0016_rom_bank_w(uint8_t data);

	virtual void machine_start() override;
	DECLARE_VIDEO_START(st0016);
	uint32_t screen_update_st0016(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(st0016_int);

	void renju_mem(address_map &map);
	void st0016_io(address_map &map);
	void st0016_m2_io(address_map &map);
	void st0016_mem(address_map &map);
	void v810_mem(address_map &map);
};
