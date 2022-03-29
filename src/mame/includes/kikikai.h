// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi

#include "cpu/m6805/m68705.h"
#include "cpu/m6800/m6801.h"

#include "sound/ymopn.h"
#include "emupal.h"
#include "screen.h"

class kikikai_state : public driver_device
{
public:
	kikikai_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_screen(*this, "screen"),
		m_mcu_sharedram(*this, "mcu_sharedram"),
		m_mainram(*this, "mainram"),
		m_subcpu(*this, "sub"),
		m_mcu(*this, "mcu"),
		m_ymsnd(*this, "ymsnd"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{
	}

	void base(machine_config &config);
	void kicknrun(machine_config &config);

protected:
	required_device<cpu_device>         m_maincpu;
	required_device<cpu_device>         m_audiocpu;
	required_device<screen_device>      m_screen;
	required_shared_ptr<u8> m_mcu_sharedram;

	u32 screen_update_kicknrun(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_kikikai(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	IRQ_CALLBACK_MEMBER(mcram_vect_r);

private:
	/* memory pointers */
	required_shared_ptr<u8> m_mainram;

	/* video-related */
	int      m_charbank = 0;

	/* devices */
	optional_device<cpu_device>         m_subcpu;   // kicknrun / mexico86 only
	optional_device<m6801_cpu_device>   m_mcu;
	required_device<ym2203_device>      m_ymsnd;
	required_device<gfxdecode_device>   m_gfxdecode;
	required_device<palette_device>     m_palette;

	/* queue */
	//u8 m_queue[64]{};
	//int m_qfront = 0;
	//int m_qstate = 0;
	void kicknrun_sub_output_w(uint8_t data);
	virtual void main_f008_w(uint8_t data);

	void main_bankswitch_w(uint8_t data);
	uint8_t kiki_ym2203_r(offs_t offset);

	virtual INTERRUPT_GEN_MEMBER(kikikai_interrupt);

	void main_map(address_map &map);
	void sound_map(address_map &map);
	void kicknrun_sub_cpu_map(address_map &map);
	void mcu_map(address_map& map);

	/* Kiki KaiKai / Kick 'n Run MCU */
	uint8_t    m_port3_in = 0U;
	uint8_t    m_port1_out = 0U;
	uint8_t    m_port2_out = 0U;
	uint8_t    m_port3_out = 0U;
	uint8_t    m_port4_out = 0U;

	void kikikai_mcu_port1_w(uint8_t data);
	void kikikai_mcu_port2_w(uint8_t data);
	uint8_t kikikai_mcu_port3_r();
	void kikikai_mcu_port3_w(uint8_t data);
	void kikikai_mcu_port4_w(uint8_t data);
};

class mexico86_state : public kikikai_state
{
public:
	mexico86_state(const machine_config& mconfig, device_type type, const char* tag)
		: kikikai_state(mconfig, type, tag),
		m_68705mcu(*this, "68705mcu")
	{
	}

	void mexico86_68705(machine_config& config);
	void knightb(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	virtual void main_f008_w(uint8_t data) override;

	INTERRUPT_GEN_MEMBER(mexico86_m68705_interrupt);
	void mexico86_68705_port_a_w(u8 data);
	void mexico86_68705_port_b_w(offs_t offset, u8 data, u8 mem_mask = ~0);

	optional_device<m68705p_device>     m_68705mcu;

	/* mcu */
	/* mexico86 68705 protection */
	u8       m_port_a_out = 0U;
	u8       m_port_b_out = 0U;
	int      m_address = 0;
	u8       m_latch = 0U;

};

class kikikai_simulation_state : public kikikai_state
{
public:
	kikikai_simulation_state(const machine_config& mconfig, device_type type, const char* tag)
		: kikikai_state(mconfig, type, tag)
	{
	}

	void kikikai(machine_config& config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	virtual void main_f008_w(uint8_t data) override;

	virtual INTERRUPT_GEN_MEMBER(kikikai_interrupt) override;

	void mcu_simulate(  );

	/* kikikai mcu simulation */
	int      m_kikikai_simulated_mcu_running = 0;
	int      m_kikikai_simulated_mcu_initialised = 0;
	bool     m_coin_last[2]{};
	u8       m_coin_fract = 0U;
};
