// license:LGPL-2.1+
// copyright-holders:Angelo Salese
/***************************************************************************

Mitsubishi M50458 OSD chip

***************************************************************************/

#ifndef MAME_VIDEO_M50458_H
#define MAME_VIDEO_M50458_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> m50458_device

class m50458_device :   public device_t,
						public device_memory_interface,
						public device_video_interface
{
public:
	// construction/destruction
	template <typename T>
	m50458_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&screen_tag)
		: m50458_device(mconfig, tag, owner, clock)
	{
		set_screen(std::forward<T>(screen_tag));
	}

	m50458_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// I/O operations
	void write_bit(int state);
	void set_cs_line(int state);
	void set_clock_line(int state);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	enum m50458_state_t
	{
		OSD_SET_ADDRESS = 0,
		OSD_SET_DATA
	};

	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual space_config_vector memory_space_config() const override;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	inline uint16_t read_word(offs_t address);
	inline void write_word(offs_t address, uint16_t data);

	void vreg_120_w(uint16_t data);
	void vreg_121_w(uint16_t data);
	void vreg_122_w(uint16_t data);
	void vreg_123_w(uint16_t data);
	void vreg_124_w(uint16_t data);
	void vreg_125_w(uint16_t data);
	void vreg_126_w(uint16_t data);
	void vreg_127_w(uint16_t data);

	void m50458_vram(address_map &map) ATTR_COLD;

	const address_space_config      m_space_config;

	int m_latch;
	int m_reset_line;
	int m_clock_line;
	uint16_t m_current_cmd;
	int m_cmd_stream_pos;
	uint16_t m_osd_addr;
	std::unique_ptr<uint8_t[]> m_shadow_gfx;

	uint8_t m_phase;
	uint8_t m_scrf,m_scrr;
	uint8_t m_space;
	uint8_t m_hsz1,m_hsz2,m_hsz3;
	uint8_t m_vsz1,m_vsz2,m_vsz3;
	uint8_t m_blink;

	m50458_state_t m_osd_state;
};


// device type definition
DECLARE_DEVICE_TYPE(M50458, m50458_device)

#endif // MAME_VIDEO_M50458_H
