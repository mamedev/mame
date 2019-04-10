// license:BSD-3-Clause
// copyright-holders:Mathis Rosenhauer
/*************************************************************************

    Centuri Aztarac hardware

*************************************************************************/

#include "cpu/m68000/m68000.h"
#include "machine/gen_latch.h"
#include "machine/x2212.h"
#include "video/vector.h"
#include "screen.h"

class aztarac_state : public driver_device
{
public:
	aztarac_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_nvram(*this, "nvram"),
		m_vector(*this, "vector"),
		m_screen(*this, "screen"),
		m_soundlatch(*this, "soundlatch"),
		m_vectorram(*this, "vectorram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<x2212_device> m_nvram;
	required_device<vector_device> m_vector;
	required_device<screen_device> m_screen;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint16_t> m_vectorram;

	int m_sound_status;
	int m_xcenter;
	int m_ycenter;

	void nvram_store_w(uint16_t data);
	DECLARE_READ16_MEMBER(joystick_r);
	void ubr_w(uint8_t data);
	DECLARE_READ16_MEMBER(sound_r);
	DECLARE_WRITE16_MEMBER(sound_w);
	DECLARE_READ8_MEMBER(snd_command_r);
	DECLARE_READ8_MEMBER(snd_status_r);
	DECLARE_WRITE8_MEMBER(snd_status_w);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	DECLARE_WRITE_LINE_MEMBER(video_interrupt);
	INTERRUPT_GEN_MEMBER(snd_timed_irq);

	inline void read_vectorram(uint16_t *vectorram, int addr, int *x, int *y, int *c);
	void aztarac(machine_config &config);
	void main_map(address_map &map);
	void sound_map(address_map &map);
};
