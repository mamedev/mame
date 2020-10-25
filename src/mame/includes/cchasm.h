// license:BSD-3-Clause
// copyright-holders:Mathis Rosenhauer

/*************************************************************************

    Cinematronics Cosmic Chasm hardware

*************************************************************************/

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/z80ctc.h"
#include "sound/dac.h"
#include "video/vector.h"
#include "screen.h"

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
	required_device<z80_device> m_audiocpu;
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

	void led_w(offs_t offset, uint16_t data);
	void refresh_control_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void reset_coin_flag_w(uint8_t data);
	uint8_t coin_sound_r();
	uint8_t soundlatch2_r();
	void soundlatch4_w(uint8_t data);
	void io_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t io_r(offs_t offset);
	DECLARE_WRITE_LINE_MEMBER(ctc_timer_1_w);
	DECLARE_WRITE_LINE_MEMBER(ctc_timer_2_w);

	INPUT_CHANGED_MEMBER(set_coin_flag);

	virtual void video_start() override;
	virtual void sound_start() override;

	void refresh();

	void cchasm(machine_config &config);
	void memmap(address_map &map);
	void sound_memmap(address_map &map);
	void sound_portmap(address_map &map);
protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
