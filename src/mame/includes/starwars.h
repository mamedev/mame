// license:???
// copyright-holders:Steve Baines, Frank Palazzolo
/***************************************************************************

    Atari Star Wars hardware

***************************************************************************/

#include "machine/6532riot.h"
#include "includes/slapstic.h"


class starwars_state : public driver_device
{
public:
	starwars_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_riot(*this, "riot"),
		m_mathram(*this, "mathram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_slapstic_device(*this, "slapstic")
		{ }

	UINT8 m_sound_data;
	UINT8 m_main_data;
	required_device<riot6532_device> m_riot;
	UINT8 *m_slapstic_source;
	UINT8 *m_slapstic_base;
	UINT8 m_slapstic_current_bank;
	offs_t m_slapstic_last_pc;
	offs_t m_slapstic_last_address;
	UINT8 m_is_esb;
	required_shared_ptr<UINT8> m_mathram;
	UINT8 m_control_num;
	int m_MPA;
	int m_BIC;
	UINT16 m_dvd_shift;
	UINT16 m_quotient_shift;
	UINT16 m_divisor;
	UINT16 m_dividend;
	UINT8 *m_PROM_STR;
	UINT8 *m_PROM_MAS;
	UINT8 *m_PROM_AM;
	int m_math_run;
	emu_timer *m_math_timer;
	INT16 m_A;
	INT16 m_B;
	INT16 m_C;
	INT32 m_ACC;
	DECLARE_WRITE8_MEMBER(irq_ack_w);
	DECLARE_READ8_MEMBER(esb_slapstic_r);
	DECLARE_WRITE8_MEMBER(esb_slapstic_w);
	DECLARE_WRITE8_MEMBER(starwars_nstore_w);
	DECLARE_WRITE8_MEMBER(starwars_out_w);
	DECLARE_READ8_MEMBER(starwars_adc_r);
	DECLARE_WRITE8_MEMBER(starwars_adc_select_w);
	DECLARE_READ8_MEMBER(starwars_prng_r);
	DECLARE_READ8_MEMBER(starwars_div_reh_r);
	DECLARE_READ8_MEMBER(starwars_div_rel_r);
	DECLARE_WRITE8_MEMBER(starwars_math_w);
	DECLARE_CUSTOM_INPUT_MEMBER(matrix_flag_r);
	DECLARE_READ8_MEMBER(starwars_sin_r);
	DECLARE_WRITE8_MEMBER(starwars_sout_w);
	DECLARE_READ8_MEMBER(starwars_main_read_r);
	DECLARE_READ8_MEMBER(starwars_main_ready_flag_r);
	DECLARE_WRITE8_MEMBER(starwars_main_wr_w);
	DECLARE_WRITE8_MEMBER(starwars_soundrst_w);
	DECLARE_WRITE8_MEMBER(quad_pokeyn_w);
	DECLARE_DIRECT_UPDATE_MEMBER(esb_setdirect);
	DECLARE_DRIVER_INIT(esb);
	DECLARE_DRIVER_INIT(starwars);
	virtual void machine_reset() override;
	TIMER_CALLBACK_MEMBER(math_run_clear);
	TIMER_CALLBACK_MEMBER(main_callback);
	TIMER_CALLBACK_MEMBER(sound_callback);
	DECLARE_READ8_MEMBER(r6532_porta_r);
	DECLARE_WRITE8_MEMBER(r6532_porta_w);
	DECLARE_WRITE_LINE_MEMBER(snd_interrupt);
	void starwars_mproc_init();
	void starwars_mproc_reset();
	void run_mproc();
	void esb_slapstic_tweak(address_space &space, offs_t offset);
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<atari_slapstic_device> m_slapstic_device;
};
