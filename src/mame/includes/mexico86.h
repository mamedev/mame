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
	required_shared_ptr<UINT8> m_objectram;
	required_shared_ptr<UINT8> m_protection_ram;
	required_shared_ptr<UINT8> m_videoram;

	/* video-related */
	int      m_charbank;

	/* mcu */
	/* mexico86 68705 protection */
	UINT8    m_port_a_in;
	UINT8    m_port_a_out;
	UINT8    m_ddr_a;
	UINT8    m_port_b_in;
	UINT8    m_port_b_out;
	UINT8    m_ddr_b;
	int      m_address;
	int      m_latch;
	/* kikikai mcu simulation */
	int      m_mcu_running;
	int      m_mcu_initialised;
	int      m_coin_last;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<cpu_device> m_subcpu;
	optional_device<cpu_device> m_mcu;
	required_device<ym2203_device> m_ymsnd;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	/* queue */
	UINT8 m_queue[64];
	int m_qfront;
	int m_qstate;
	DECLARE_WRITE8_MEMBER(mexico86_sub_output_w);
	DECLARE_WRITE8_MEMBER(mexico86_f008_w);
	DECLARE_READ8_MEMBER(mexico86_68705_port_a_r);
	DECLARE_WRITE8_MEMBER(mexico86_68705_port_a_w);
	DECLARE_WRITE8_MEMBER(mexico86_68705_ddr_a_w);
	DECLARE_READ8_MEMBER(mexico86_68705_port_b_r);
	DECLARE_WRITE8_MEMBER(mexico86_68705_port_b_w);
	DECLARE_WRITE8_MEMBER(mexico86_68705_ddr_b_w);
	DECLARE_WRITE8_MEMBER(mexico86_bankswitch_w);
	DECLARE_READ8_MEMBER(kiki_ym2203_r);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	UINT32 screen_update_mexico86(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_kikikai(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(kikikai_interrupt);
	INTERRUPT_GEN_MEMBER(mexico86_m68705_interrupt);
	void mcu_simulate(  );
	void kiki_clogic(int address, int latch);
};
