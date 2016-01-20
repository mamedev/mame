// license:BSD-3-Clause
// copyright-holders:Mike Appolo
/*************************************************************************

    Atari Major Havoc hardware

*************************************************************************/

#define MHAVOC_CLOCK        10000000
#define MHAVOC_CLOCK_5M     (MHAVOC_CLOCK/2)
#define MHAVOC_CLOCK_2_5M   (MHAVOC_CLOCK/4)
#define MHAVOC_CLOCK_1_25M  (MHAVOC_CLOCK/8)
#define MHAVOC_CLOCK_625K   (MHAVOC_CLOCK/16)

#define MHAVOC_CLOCK_156K   (MHAVOC_CLOCK_625K/4)
#define MHAVOC_CLOCK_5K     (MHAVOC_CLOCK_625K/16/8)
#define MHAVOC_CLOCK_2_4K   (MHAVOC_CLOCK_625K/16/16)


class mhavoc_state : public driver_device
{
public:
	mhavoc_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_zram0(*this, "zram0"),
		m_zram1(*this, "zram1"),
		m_alpha(*this, "alpha"),
		m_gamma(*this, "gamma"){ }

	required_shared_ptr<UINT8> m_zram0;
	required_shared_ptr<UINT8> m_zram1;
	required_device<cpu_device> m_alpha;
	optional_device<cpu_device> m_gamma;
	UINT8 m_alpha_data;
	UINT8 m_alpha_rcvd;
	UINT8 m_alpha_xmtd;
	UINT8 m_gamma_data;
	UINT8 m_gamma_rcvd;
	UINT8 m_gamma_xmtd;
	UINT8 m_player_1;
	UINT8 m_alpha_irq_clock;
	UINT8 m_alpha_irq_clock_enable;
	UINT8 m_gamma_irq_clock;
	UINT8 m_has_gamma_cpu;
	UINT8 m_speech_write_buffer;
	DECLARE_READ8_MEMBER(dual_pokey_r);
	DECLARE_WRITE8_MEMBER(dual_pokey_w);
	DECLARE_WRITE8_MEMBER(mhavoc_alpha_irq_ack_w);
	DECLARE_WRITE8_MEMBER(mhavoc_gamma_irq_ack_w);
	DECLARE_WRITE8_MEMBER(mhavoc_gamma_w);
	DECLARE_READ8_MEMBER(mhavoc_alpha_r);
	DECLARE_WRITE8_MEMBER(mhavoc_alpha_w);
	DECLARE_READ8_MEMBER(mhavoc_gamma_r);
	DECLARE_WRITE8_MEMBER(mhavoc_ram_banksel_w);
	DECLARE_WRITE8_MEMBER(mhavoc_rom_banksel_w);
	DECLARE_WRITE8_MEMBER(mhavoc_out_0_w);
	DECLARE_WRITE8_MEMBER(alphaone_out_0_w);
	DECLARE_WRITE8_MEMBER(mhavoc_out_1_w);
	DECLARE_WRITE8_MEMBER(mhavocrv_speech_data_w);
	DECLARE_WRITE8_MEMBER(mhavocrv_speech_strobe_w);
	DECLARE_READ8_MEMBER(quad_pokeyn_r);
	DECLARE_WRITE8_MEMBER(quad_pokeyn_w);
	DECLARE_CUSTOM_INPUT_MEMBER(tms5220_r);
	DECLARE_CUSTOM_INPUT_MEMBER(mhavoc_bit67_r);
	DECLARE_CUSTOM_INPUT_MEMBER(gamma_rcvd_r);
	DECLARE_CUSTOM_INPUT_MEMBER(gamma_xmtd_r);
	DECLARE_CUSTOM_INPUT_MEMBER(alpha_rcvd_r);
	DECLARE_CUSTOM_INPUT_MEMBER(alpha_xmtd_r);
	DECLARE_CUSTOM_INPUT_MEMBER(clock_r);
	DECLARE_DRIVER_INIT(mhavocrv);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	TIMER_CALLBACK_MEMBER(delayed_gamma_w);
	TIMER_DEVICE_CALLBACK_MEMBER(mhavoc_cpu_irq_clock);
};
