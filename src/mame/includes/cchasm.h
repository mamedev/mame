// license:BSD-3-Clause
// copyright-holders:Mathis Rosenhauer

/*************************************************************************

    Cinematronics Cosmic Chasm hardware

*************************************************************************/

#include "machine/gen_latch.h"
#include "machine/z80ctc.h"
#include "sound/dac.h"
#include "video/vector.h"

class cchasm_state : public driver_device
{
public:
	enum
	{
		TIMER_REFRESH_END
	};

	cchasm_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ctc(*this, "ctc"),
		m_audiocpu(*this, "audiocpu"),
		m_dac1(*this, "dac1"),
		m_dac2(*this, "dac2"),
		m_vector(*this, "vector"),
		m_screen(*this, "screen"),
		m_soundlatch(*this, "soundlatch"),
		m_soundlatch2(*this, "soundlatch2"),
		m_soundlatch3(*this, "soundlatch3"),
		m_soundlatch4(*this, "soundlatch4"),
		m_ram(*this, "ram") { }

	required_device<cpu_device> m_maincpu;
	required_device<z80ctc_device> m_ctc;
	required_device<cpu_device> m_audiocpu;
	required_device<dac_bit_interface> m_dac1;
	required_device<dac_bit_interface> m_dac2;
	required_device<vector_device> m_vector;
	required_device<screen_device> m_screen;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<generic_latch_8_device> m_soundlatch2;
	required_device<generic_latch_8_device> m_soundlatch3;
	required_device<generic_latch_8_device> m_soundlatch4;

	required_shared_ptr<uint16_t> m_ram;

	int m_sound_flags;
	int m_coin_flag;
	int m_output[2];
	int m_xcenter;
	int m_ycenter;
	emu_timer *m_refresh_end_timer;

	void led_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void refresh_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void reset_coin_flag_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t coin_sound_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t soundlatch2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void soundlatch4_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void io_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t io_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void ctc_timer_1_w(int state);
	void ctc_timer_2_w(int state);

	void set_coin_flag(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);

	virtual void video_start() override;
	virtual void sound_start() override;

	void refresh();

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
