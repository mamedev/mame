// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/* Sega Road Generators */


#pragma once
#ifndef __SEGAIC16_ROAD_H__
#define __SEGAIC16_ROAD_H__


/* road systems */
#define SEGAIC16_MAX_ROADS          1

#define SEGAIC16_ROAD_HANGON        0
#define SEGAIC16_ROAD_SHARRIER      1
#define SEGAIC16_ROAD_OUTRUN        2
#define SEGAIC16_ROAD_XBOARD        3

#define SEGAIC16_ROAD_BACKGROUND    0
#define SEGAIC16_ROAD_FOREGROUND    1



struct road_info
{
	UINT8           index;                          /* index of this structure */
	UINT8           type;                           /* type of road system (see segaic16.h for details) */
	UINT8           control;                        /* control register value */
	UINT16          colorbase1;                     /* color base for road ROM data */
	UINT16          colorbase2;                     /* color base for road background data */
	UINT16          colorbase3;                     /* color base for sky data */
	INT32           xoffs;                          /* X scroll offset */
	void            (*draw)(struct road_info *info, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority);
	UINT16 *        roadram;                        /* pointer to roadram pointer */
	std::unique_ptr<UINT16[]>        buffer;                         /* buffered roadram pointer */
	std::unique_ptr<UINT8[]>          gfx;                            /* expanded road graphics */
};



class segaic16_road_device : public device_t
{
public:
	segaic16_road_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	~segaic16_road_device() {}

	UINT16 *segaic16_roadram_0;
	void segaic16_road_hangon_decode(running_machine &machine, struct road_info *info);
	void segaic16_road_outrun_decode(running_machine &machine, struct road_info *info);

	struct road_info segaic16_road[SEGAIC16_MAX_ROADS];
	void segaic16_road_init(running_machine &machine, int which, int type, int colorbase1, int colorbase2, int colorbase3, int xoffs);

	void segaic16_road_draw(int which, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority);

	DECLARE_READ16_MEMBER( segaic16_road_control_0_r );
	DECLARE_WRITE16_MEMBER( segaic16_road_control_0_w );


protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state
};

extern const device_type SEGAIC16_ROAD;

#define MCFG_SEGAIC16_ROAD_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, SEGAIC16_ROAD, 0)

#endif
