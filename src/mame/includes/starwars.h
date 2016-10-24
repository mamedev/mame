// license:BSD-3-Clause
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

	uint8_t m_sound_data;
	uint8_t m_main_data;
	required_device<riot6532_device> m_riot;
	uint8_t *m_slapstic_source;
	uint8_t *m_slapstic_base;
	uint8_t m_slapstic_current_bank;
	uint8_t m_is_esb;
	required_shared_ptr<uint8_t> m_mathram;
	uint8_t m_control_num;
	int m_MPA;
	int m_BIC;
	uint16_t m_dvd_shift;
	uint16_t m_quotient_shift;
	uint16_t m_divisor;
	uint16_t m_dividend;
	std::unique_ptr<uint8_t[]> m_PROM_STR;
	std::unique_ptr<uint8_t[]> m_PROM_MAS;
	std::unique_ptr<uint8_t[]> m_PROM_AM;
	int m_math_run;
	emu_timer *m_math_timer;
	int16_t m_A;
	int16_t m_B;
	int16_t m_C;
	int32_t m_ACC;
	void irq_ack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t esb_slapstic_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void esb_slapstic_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void starwars_nstore_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void starwars_out_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t starwars_adc_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void starwars_adc_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t starwars_prng_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t starwars_div_reh_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t starwars_div_rel_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void starwars_math_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	ioport_value matrix_flag_r(ioport_field &field, void *param);
	uint8_t starwars_sin_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void starwars_sout_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t starwars_main_read_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t starwars_main_ready_flag_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void starwars_main_wr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void starwars_soundrst_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void quad_pokeyn_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_esb();
	void init_starwars();
	virtual void machine_reset() override;
	void math_run_clear(void *ptr, int32_t param);
	void main_callback(void *ptr, int32_t param);
	void sound_callback(void *ptr, int32_t param);
	uint8_t r6532_porta_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void r6532_porta_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void snd_interrupt(int state);
	void starwars_mproc_init();
	void starwars_mproc_reset();
	void run_mproc();
	void esb_slapstic_tweak(address_space &space, offs_t offset);
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<atari_slapstic_device> m_slapstic_device;
};
