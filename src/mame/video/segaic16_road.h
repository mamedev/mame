// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/* Sega Road Generators */
#ifndef MAME_VIDEO_SEGAIC16_ROAD_H
#define MAME_VIDEO_SEGAIC16_ROAD_H

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
		uint8_t           index;                          /* index of this structure */
		uint8_t           type;                           /* type of road system (see segaic16.h for details) */
		uint8_t           control;                        /* control register value */
		uint16_t          colorbase1;                     /* color base for road ROM data */
		uint16_t          colorbase2;                     /* color base for road background data */
		uint16_t          colorbase3;                     /* color base for sky data */
		int32_t           xoffs;                          /* X scroll offset */
		void            (*draw)(struct road_info *info, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority);
		uint16_t *        roadram;                        /* pointer to roadram pointer */
		std::unique_ptr<uint16_t[]>        buffer;                         /* buffered roadram pointer */
		std::unique_ptr<uint8_t[]>          gfx;                            /* expanded road graphics */
	};



	segaic16_road_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint16_t *segaic16_roadram_0;
	void segaic16_road_hangon_decode(running_machine &machine, struct road_info *info);
	void segaic16_road_outrun_decode(running_machine &machine, struct road_info *info);

	struct road_info segaic16_road[MAX_ROADS];
	void segaic16_road_init(running_machine &machine, int which, int type, int colorbase1, int colorbase2, int colorbase3, int xoffs);

	void segaic16_road_draw(int which, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority);

	DECLARE_READ16_MEMBER( segaic16_road_control_0_r );
	DECLARE_WRITE16_MEMBER( segaic16_road_control_0_w );


protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state
};

DECLARE_DEVICE_TYPE(SEGAIC16_ROAD, segaic16_road_device)

#endif // MAME_VIDEO_SEGAIC16_ROAD_H
