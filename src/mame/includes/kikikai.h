// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi

#include "cpu/m6805/m68705.h"
#include "cpu/m6800/m6801.h"

#include "sound/2203intf.h"
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

private:
	/* memory pointers */
	required_shared_ptr<u8> m_mainram;

	/* video-related */
	int      m_charbank;

	/* devices */
	optional_device<cpu_device>         m_subcpu;   // kicknrun / mexico86 only
	optional_device<cpu_device>         m_mcu;
	required_device<ym2203_device>      m_ymsnd;
	required_device<gfxdecode_device>   m_gfxdecode;
	required_device<palette_device>     m_palette;

	/* queue */
	//u8 m_queue[64];
	//int m_qfront;
	//int m_qstate;
	DECLARE_WRITE8_MEMBER(kicknrun_sub_output_w);
	virtual  DECLARE_WRITE8_MEMBER(main_f008_w);

	DECLARE_WRITE8_MEMBER(main_bankswitch_w);
	DECLARE_READ8_MEMBER(kiki_ym2203_r);

	virtual INTERRUPT_GEN_MEMBER(kikikai_interrupt);

	void main_map(address_map &map);
	void sound_map(address_map &map);
	void kicknrun_sub_cpu_map(address_map &map);
	void mcu_map(address_map& map);

	/* Kiki KaiKai / Kick 'n Run MCU */
	uint8_t    m_ddr1;
	uint8_t    m_ddr2;
	uint8_t    m_ddr3;
	uint8_t    m_ddr4;
	uint8_t    m_port1_in;
	uint8_t    m_port2_in;
	uint8_t    m_port3_in;
	uint8_t    m_port4_in;
	uint8_t    m_port1_out;
	uint8_t    m_port2_out;
	uint8_t    m_port3_out;
	uint8_t    m_port4_out;

	DECLARE_READ8_MEMBER(kikikai_mcu_ddr1_r);
	DECLARE_WRITE8_MEMBER(kikikai_mcu_ddr1_w);
	DECLARE_READ8_MEMBER(kikikai_mcu_ddr2_r);
	DECLARE_WRITE8_MEMBER(kikikai_mcu_ddr2_w);
	DECLARE_READ8_MEMBER(kikikai_mcu_ddr3_r);
	DECLARE_WRITE8_MEMBER(kikikai_mcu_ddr3_w);
	DECLARE_READ8_MEMBER(kikikai_mcu_ddr4_r);
	DECLARE_WRITE8_MEMBER(kikikai_mcu_ddr4_w);
	DECLARE_READ8_MEMBER(kikikai_mcu_port1_r);
	DECLARE_WRITE8_MEMBER(kikikai_mcu_port1_w);
	DECLARE_READ8_MEMBER(kikikai_mcu_port2_r);
	DECLARE_WRITE8_MEMBER(kikikai_mcu_port2_w);
	DECLARE_READ8_MEMBER(kikikai_mcu_port3_r);
	DECLARE_WRITE8_MEMBER(kikikai_mcu_port3_w);
	DECLARE_READ8_MEMBER(kikikai_mcu_port4_r);
	DECLARE_WRITE8_MEMBER(kikikai_mcu_port4_w);
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
	virtual DECLARE_WRITE8_MEMBER(main_f008_w) override;

	INTERRUPT_GEN_MEMBER(mexico86_m68705_interrupt);
	DECLARE_WRITE8_MEMBER(mexico86_68705_port_a_w);
	DECLARE_WRITE8_MEMBER(mexico86_68705_port_b_w);

	optional_device<m68705p_device>     m_68705mcu;

	/* mcu */
	/* mexico86 68705 protection */
	u8       m_port_a_out;
	u8       m_port_b_out;
	int      m_address;
	u8       m_latch;

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
	virtual DECLARE_WRITE8_MEMBER(main_f008_w) override;

	virtual INTERRUPT_GEN_MEMBER(kikikai_interrupt) override;

	void mcu_simulate(  );

	/* kikikai mcu simulation */
	int      m_kikikai_simulated_mcu_running;
	int      m_kikikai_simulated_mcu_initialised;
	bool     m_coin_last[2];
	u8       m_coin_fract;
};
