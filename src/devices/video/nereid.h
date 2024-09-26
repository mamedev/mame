// license:BSD-3-Clause
// copyright-holders:Sven Schnelle
#ifndef MAME_VIDEO_NEREID_H
#define MAME_VIDEO_NEREID_H

#pragma once

class nereid_device : public device_t, public device_palette_interface
{
public:
	nereid_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint16_t ctrl_r(offs_t offset, uint16_t mem_mask = ~0);
	void ctrl_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	rgb_t map_color(uint8_t input, uint8_t ovl = 0);

protected:
	nereid_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual u32 palette_entries() const noexcept override { return 0x200; }

private:
	static constexpr int NEREID_BUSY=1;

	static constexpr int NEREID_BLANK_ALL=0x2e;
	static constexpr int NEREID_WRITE_STROBE_OVERLAY=0x3c;
	static constexpr int NEREID_UNKNOWN_A0=0x50;
	static constexpr int NEREID_OVERLAY_CTL=0x51;
	static constexpr int NEREID_INDEX0=0x58;
	static constexpr int NEREID_RED_DATA=0x59;
	static constexpr int NEREID_GREEN_DATA=0x5a;
	static constexpr int NEREID_BLUE_DATA=0x5b;
	static constexpr int NEREID_INDEX=0x5c;
	static constexpr int NEREID_PLANE_MASK=0x5d;
	static constexpr int NEREID_OVERLAY_INDEX=0x5e;
	static constexpr int NEREID_REV=0x5f;
	static constexpr int NEREID_WRITE_STROBE=0x78;
	static constexpr int NEREID_READ_STROBE=0x7c;

	/* registers */
	uint8_t m_red;
	uint8_t m_green;
	uint8_t m_blue;
	uint8_t m_index;
	uint8_t m_plane_mask;
	uint8_t m_overlay_ctl;
	uint8_t m_unknown_a0;
	uint8_t m_overlay_index;
};

DECLARE_DEVICE_TYPE(NEREID, nereid_device)
#endif // MAME_VIDEO_NEREID_H
