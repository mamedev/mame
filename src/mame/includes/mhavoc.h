// license:BSD-3-Clause
// copyright-holders:Mike Appolo
/*************************************************************************

    Atari Major Havoc hardware

*************************************************************************/

#include "machine/timer.h"
#include "sound/pokey.h"
#include "sound/tms5220.h"

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
	mhavoc_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_zram0(*this, "zram0"),
		m_zram1(*this, "zram1"),
		m_alpha(*this, "alpha"),
		m_gamma(*this, "gamma"),
		m_pokey(*this, "pokey%u", 1U),
		m_tms(*this, "tms"),
		m_lamps(*this, "lamp%u", 0U),
		m_coin(*this, "COIN"),
		m_service(*this, "SERVICE")
	{ }

	void alphaone(machine_config &config);
	void mhavoc(machine_config &config);
	void mhavocrv(machine_config &config);

	void init_mhavocrv();

	DECLARE_CUSTOM_INPUT_MEMBER(coin_service_r);
	DECLARE_READ_LINE_MEMBER(gamma_rcvd_r);
	DECLARE_READ_LINE_MEMBER(gamma_xmtd_r);
	DECLARE_READ_LINE_MEMBER(alpha_rcvd_r);
	DECLARE_READ_LINE_MEMBER(alpha_xmtd_r);
	DECLARE_READ_LINE_MEMBER(clock_r);

private:
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

	TIMER_CALLBACK_MEMBER(delayed_gamma_w);
	TIMER_DEVICE_CALLBACK_MEMBER(mhavoc_cpu_irq_clock);
	void alpha_map(address_map &map);
	void alphaone_map(address_map &map);
	void gamma_map(address_map &map);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	required_shared_ptr<uint8_t> m_zram0;
	required_shared_ptr<uint8_t> m_zram1;
	required_device<cpu_device> m_alpha;
	optional_device<cpu_device> m_gamma;
	optional_device_array<pokey_device, 4> m_pokey;
	optional_device<tms5220_device> m_tms;
	output_finder<2> m_lamps;
	optional_ioport m_coin;
	optional_ioport m_service;

	uint8_t m_alpha_data;
	uint8_t m_alpha_rcvd;
	uint8_t m_alpha_xmtd;
	uint8_t m_gamma_data;
	uint8_t m_gamma_rcvd;
	uint8_t m_gamma_xmtd;
	uint8_t m_player_1;
	uint8_t m_alpha_irq_clock;
	uint8_t m_alpha_irq_clock_enable;
	uint8_t m_gamma_irq_clock;
	uint8_t m_has_gamma_cpu;
	uint8_t m_speech_write_buffer;
};
