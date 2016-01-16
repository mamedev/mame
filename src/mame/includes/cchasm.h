// license:BSD-3-Clause
// copyright-holders:Mathis Rosenhauer
/*************************************************************************

    Cinematronics Cosmic Chasm hardware

*************************************************************************/

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

	cchasm_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ctc(*this, "ctc"),
		m_audiocpu(*this, "audiocpu"),
		m_dac1(*this, "dac1"),
		m_dac2(*this, "dac2"),
		m_vector(*this, "vector"),
		m_screen(*this, "screen"),
		m_ram(*this, "ram") { }

	required_device<cpu_device> m_maincpu;
	required_device<z80ctc_device> m_ctc;
	required_device<cpu_device> m_audiocpu;
	required_device<dac_device> m_dac1;
	required_device<dac_device> m_dac2;
	required_device<vector_device> m_vector;
	required_device<screen_device> m_screen;

	required_shared_ptr<UINT16> m_ram;

	int m_sound_flags;
	int m_coin_flag;
	int m_channel_active[2];
	int m_output[2];
	int m_xcenter;
	int m_ycenter;
	emu_timer *m_refresh_end_timer;

	DECLARE_WRITE16_MEMBER(led_w);
	DECLARE_WRITE16_MEMBER(refresh_control_w);
	DECLARE_WRITE8_MEMBER(reset_coin_flag_w);
	DECLARE_READ8_MEMBER(coin_sound_r);
	DECLARE_READ8_MEMBER(soundlatch2_r);
	DECLARE_WRITE8_MEMBER(soundlatch4_w);
	DECLARE_WRITE16_MEMBER(io_w);
	DECLARE_READ16_MEMBER(io_r);
	DECLARE_WRITE_LINE_MEMBER(ctc_timer_1_w);
	DECLARE_WRITE_LINE_MEMBER(ctc_timer_2_w);

	INPUT_CHANGED_MEMBER(set_coin_flag);

	virtual void video_start() override;
	virtual void sound_start() override;

	void refresh();

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
