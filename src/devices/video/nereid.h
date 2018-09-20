// license:BSD-3-Clause
// copyright-holders:Sven Schnelle
#ifndef MAME_VIDEO_NEREID_H
#define MAME_VIDEO_NEREID_H

#pragma once

class nereid_device : public device_t
{
public:
	nereid_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ16_MEMBER(ctrl_r);
	DECLARE_WRITE16_MEMBER(ctrl_w);

	rgb_t map_color(uint8_t input);

protected:
	nereid_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	static constexpr int NEREID_BUSY=1;
	static constexpr int NEREID_RED_DATA=0x59;
	static constexpr int NEREID_GREEN_DATA=0x5a;
	static constexpr int NEREID_BLUE_DATA=0x5b;
	static constexpr int NEREID_INDEX=0x5c;
	static constexpr int NEREID_PLANE_MASK=0x5d;
	static constexpr int NEREID_WRITE_STROBE=0x78;
	static constexpr int NEREID_READ_STROBE=0x7c;

	rgb_t m_palette[256];

	/* registers */
	uint8_t m_red;
	uint8_t m_green;
	uint8_t m_blue;
	uint8_t m_index;
	uint8_t m_plane_mask;
};

DECLARE_DEVICE_TYPE(NEREID, nereid_device)
#endif // MAME_VIDEO_NEREID_H
