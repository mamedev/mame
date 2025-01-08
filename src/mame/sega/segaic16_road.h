// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/* Sega Road Generators */
#ifndef MAME_SEGA_SEGAIC16_ROAD_H
#define MAME_SEGA_SEGAIC16_ROAD_H

#pragma once



class segaic16_road_device : public device_t
{
public:
	/* road systems */
	static constexpr unsigned MAX_ROADS          = 1;

	static constexpr unsigned ROAD_HANGON        = 0;
	static constexpr unsigned ROAD_SHARRIER      = 1;
	static constexpr unsigned ROAD_OUTRUN        = 2;
	static constexpr unsigned ROAD_XBOARD        = 3;

	static constexpr unsigned ROAD_BACKGROUND    = 0;
	static constexpr unsigned ROAD_FOREGROUND    = 1;


	struct road_info
	{
		using draw_func = void (*)(struct road_info *info, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority);

		u8                      index = 0;          // index of this structure
		u8                      type = 0;           // type of road system (see segaic16.h for details)
		u8                      control = 0;        // control register value
		u16                     colorbase1 = 0;     // color base for road ROM data
		u16                     colorbase2 = 0;     // color base for road background data
		u16                     colorbase3 = 0;     // color base for sky data
		s32                     xoffs = 0;          // X scroll offset
		draw_func               draw = nullptr;
		u16 *                   roadram = nullptr;  // pointer to roadram pointer
		std::unique_ptr<u16[]>  buffer;             // buffered roadram pointer
		std::unique_ptr<u8[]>   gfx;                // expanded road graphics
	};


	segaic16_road_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void segaic16_road_hangon_decode(struct road_info *info);
	void segaic16_road_outrun_decode(struct road_info *info);

	struct road_info segaic16_road[MAX_ROADS];
	void segaic16_road_init(int which, int type, int colorbase1, int colorbase2, int colorbase3, int xoffs);

	void segaic16_road_draw(int which, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority);

	u16 segaic16_road_control_0_r();
	void segaic16_road_control_0_w(offs_t offset, u16 data, u16 mem_mask = ~0);


protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// internal state
	required_region_ptr<u8> m_gfx_region;
	required_shared_ptr<u16> m_roadram;
};

DECLARE_DEVICE_TYPE(SEGAIC16_ROAD, segaic16_road_device)

#endif // MAME_SEGA_SEGAIC16_ROAD_H
