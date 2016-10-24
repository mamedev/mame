// license:BSD-3-Clause
// copyright-holders:Mathis Rosenhauer
/*************************************************************************

    Centuri Aztarac hardware

*************************************************************************/

#include "machine/gen_latch.h"
#include "video/vector.h"

class aztarac_state : public driver_device
{
public:
	aztarac_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_vector(*this, "vector"),
		m_screen(*this, "screen"),
		m_soundlatch(*this, "soundlatch"),
		m_nvram(*this, "nvram") ,
		m_vectorram(*this, "vectorram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<vector_device> m_vector;
	required_device<screen_device> m_screen;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint16_t> m_nvram;
	required_shared_ptr<uint16_t> m_vectorram;

	int m_sound_status;
	int m_xcenter;
	int m_ycenter;

	uint16_t nvram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t joystick_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void ubr_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t sound_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void sound_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t snd_command_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t snd_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void snd_status_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	virtual void machine_start() override;
	virtual void video_start() override;

	void snd_timed_irq(device_t &device);
	int irq_callback(device_t &device, int irqline);

	inline void read_vectorram(uint16_t *vectorram, int addr, int *x, int *y, int *c);
};
