// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
#include "sound/2203intf.h"

class mexico86_state : public driver_device
{
public:
	mexico86_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_objectram(*this, "objectram"),
		m_protection_ram(*this, "protection_ram"),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_subcpu(*this, "sub"),
		m_mcu(*this, "mcu"),
		m_ymsnd(*this, "ymsnd"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{
	}

	/* memory pointers */
	required_shared_ptr<uint8_t> m_objectram;
	required_shared_ptr<uint8_t> m_protection_ram;
	required_shared_ptr<uint8_t> m_videoram;

	/* video-related */
	int      m_charbank;

	/* mcu */
	/* mexico86 68705 protection */
	uint8_t    m_port_a_in;
	uint8_t    m_port_a_out;
	uint8_t    m_ddr_a;
	uint8_t    m_port_b_in;
	uint8_t    m_port_b_out;
	uint8_t    m_ddr_b;
	int      m_address;
	int      m_latch;
	/* kikikai mcu simulation */
	int      m_mcu_running;
	int      m_mcu_initialised;
	bool     m_coin_last[2];
	uint8_t    m_coin_fract;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<cpu_device> m_subcpu;
	optional_device<cpu_device> m_mcu;
	required_device<ym2203_device> m_ymsnd;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	/* queue */
	uint8_t m_queue[64];
	int m_qfront;
	int m_qstate;
	void mexico86_sub_output_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mexico86_f008_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mexico86_68705_port_a_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mexico86_68705_port_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mexico86_68705_ddr_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mexico86_68705_port_b_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mexico86_68705_port_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mexico86_68705_ddr_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mexico86_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t kiki_ym2203_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	uint32_t screen_update_mexico86(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_kikikai(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void kikikai_interrupt(device_t &device);
	void mexico86_m68705_interrupt(device_t &device);
	void mcu_simulate(  );
	bool mcu_coin_counter_w(bool condition);
};
