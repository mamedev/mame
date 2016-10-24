// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Model Racing Dribbling hardware

*************************************************************************/

#include "machine/i8255.h"
#include "machine/watchdog.h"

class dribling_state : public driver_device
{
public:
	dribling_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_watchdog(*this, "watchdog"),
		m_ppi8255_0(*this, "ppi8255_0"),
		m_ppi8255_1(*this, "ppi8255_1"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"){ }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<watchdog_timer_device> m_watchdog;
	optional_device<i8255_device>  m_ppi8255_0;
	optional_device<i8255_device>  m_ppi8255_1;
	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;

	/* misc */
	uint8_t    m_abca;
	uint8_t    m_dr;
	uint8_t    m_ds;
	uint8_t    m_sh;
	uint8_t    m_input_mux;
	uint8_t    m_di;

	uint8_t ioread(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void iowrite(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dribling_colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t dsr_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t input_mux0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void misc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sound_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void shr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	void palette_init_dribling(palette_device &palette);
	uint32_t screen_update_dribling(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void dribling_irq_gen(device_t &device);
};
