// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/*************************************************************************

    Coors Light Bowling/Bowl-O-Rama hardware

*************************************************************************/

#include "machine/gen_latch.h"
#include "machine/nvram.h"
#include "machine/watchdog.h"
#include "video/tms34061.h"

class capbowl_state : public driver_device
{
public:
	enum
	{
		TIMER_UPDATE
	};

	capbowl_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_watchdog(*this, "watchdog"),
		m_audiocpu(*this, "audiocpu"),
		m_tms34061(*this, "tms34061"),
		m_screen(*this, "screen"),
		m_soundlatch(*this, "soundlatch"),
		m_rowaddress(*this, "rowaddress") { }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<cpu_device> m_audiocpu;
	required_device<tms34061_device> m_tms34061;
	required_device<screen_device> m_screen;
	required_device<generic_latch_8_device> m_soundlatch;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_rowaddress;

	/* video-related */
	offs_t m_blitter_addr;

	/* input-related */
	uint8_t m_last_trackball_val[2];

	emu_timer *m_update_timer;

	// common
	uint8_t track_0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t track_1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void track_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sndcmd_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tms34061_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t tms34061_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	// capbowl specific
	void capbowl_rom_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// bowlrama specific
	void bowlrama_blitter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t bowlrama_blitter_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	void init_capbowl();
	virtual void machine_start() override;
	virtual void machine_reset() override;

	void interrupt(device_t &device);
	void update(void *ptr, int32_t param);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	inline rgb_t pen_for_pixel( uint8_t *src, uint8_t pix );

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
