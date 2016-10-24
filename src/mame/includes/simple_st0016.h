// license:LGPL-2.1+
// copyright-holders:Tomasz Slanina

#include "machine/st0016.h"

class st0016_state : public driver_device
{
public:
	st0016_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_subcpu(*this, "sub"),
		m_screen(*this, "screen")
	{ }

	int mux_port;
	// uint32_t m_st0016_rom_bank;

	optional_device<st0016_cpu_device> m_maincpu;
	uint8_t mux_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mux_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint32_t latch32_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void latch32_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint8_t latch8_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void latch8_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void st0016_rom_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_nratechu();
	void init_mayjinsn();
	void init_mayjisn2();
	void init_renju();
	virtual void machine_start() override;
	void video_start_st0016();
	uint32_t screen_update_st0016(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void st0016_int(timer_device &timer, void *ptr, int32_t param);
	optional_device<cpu_device> m_subcpu;
	required_device<screen_device> m_screen;
};
