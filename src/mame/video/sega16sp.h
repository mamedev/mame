// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Sega 16-bit sprite hardware

***************************************************************************/
#ifndef MAME_VIDEO_SEGA16SP_H
#define MAME_VIDEO_SEGA16SP_H

#pragma once

#include "video/sprite.h"
#include "segaic16.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_SEGA_HANGON_SPRITES_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, SEGA_HANGON_SPRITES, 0)
#define MCFG_SEGA_SHARRIER_SPRITES_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, SEGA_SHARRIER_SPRITES, 0)
#define MCFG_SEGA_OUTRUN_SPRITES_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, SEGA_OUTRUN_SPRITES, 0)
#define MCFG_SEGA_SYS16A_SPRITES_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, SEGA_SYS16A_SPRITES, 0)
#define MCFG_SEGA_SYS16B_SPRITES_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, SEGA_SYS16B_SPRITES, 0)
#define MCFG_SEGA_XBOARD_SPRITES_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, SEGA_XBOARD_SPRITES, 0)
#define MCFG_SEGA_YBOARD_SPRITES_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, SEGA_YBOARD_SPRITES, 0)

#define MCFG_BOOTLEG_SYS16A_SPRITES_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, BOOTLEG_SYS16A_SPRITES, 0)
#define MCFG_BOOTLEG_SYS16A_SPRITES_REMAP(_0,_1,_2,_3,_4,_5,_6,_7) \
	downcast<bootleg_sys16a_sprite_device &>(*device).set_remap(_0,_1,_2,_3,_4,_5,_6,_7);

#define MCFG_BOOTLEG_SYS16A_SPRITES_XORIGIN(_xorigin) \
	downcast<bootleg_sys16a_sprite_device &>(*device).set_local_originx(_xorigin);

#define MCFG_BOOTLEG_SYS16A_SPRITES_YORIGIN(_yorigin) \
	downcast<bootleg_sys16a_sprite_device &>(*device).set_local_originy(_yorigin);


#define MCFG_BOOTLEG_SYS16B_SPRITES_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, SEGA_SYS16B_SPRITES, 0)
#define MCFG_BOOTLEG_SYS16B_SPRITES_XORIGIN(_xorigin) \
	downcast<sega_16bit_sprite_device &>(*device).set_local_originx(_xorigin);
#define MCFG_BOOTLEG_SYS16B_SPRITES_YORIGIN(_yorigin) \
	downcast<sega_16bit_sprite_device &>(*device).set_local_originy(_yorigin);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> sega_16bit_sprite_device

class sega_16bit_sprite_device : public sprite16_device_ind16
{
protected:
	// construction/destruction
	sega_16bit_sprite_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner);

public:
	// live configuration
	void set_bank(int banknum, int offset) { m_bank[banknum] = offset; }
	void set_flip(bool flip) { m_flip = flip; }
	void set_local_origin(int x, int y)
	{
		m_xoffs_flipped = m_xoffs = x;
		m_yoffs_flipped = m_yoffs = y;
		set_origin(x, y);
	}

	void set_local_origin(int x, int y, int xf, int yf)
	{
		m_xoffs = x;
		m_yoffs = y;
		m_xoffs_flipped = xf;
		m_yoffs_flipped = yf;
		set_origin(x, y);
	}

	void set_local_originx_(int x)  { m_xoffs_flipped = m_xoffs = x; set_origin(x, m_yoffs); }
	void set_local_originy_(int y) { m_yoffs_flipped = m_yoffs = y; set_origin(m_xoffs, y); }

	void set_local_originx(int x)
	{
		set_local_originx_(x);
	};

	void set_local_originy(int y)
	{
		set_local_originy_(y);
	};

	// write trigger memory handler
	DECLARE_WRITE16_MEMBER( draw_write );

protected:
	// device-level overrides
	virtual void device_start() override;

	// internal state
	bool                        m_flip;                 // screen flip?
	uint8_t                       m_bank[16];             // banking redirection
	int m_xoffs, m_yoffs;
	int m_xoffs_flipped, m_yoffs_flipped;

};


// ======================> sega_hangon_sprite_device

class sega_hangon_sprite_device : public sega_16bit_sprite_device
{
public:
	// construction/destruction
	sega_hangon_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// subclass overrides
	virtual void draw(bitmap_ind16 &bitmap, const rectangle &cliprect) override;

	// memory regions
	required_region_ptr<uint16_t> m_sprite_region_ptr;
};


// ======================> sega_sharrier_sprite_device

class sega_sharrier_sprite_device : public sega_16bit_sprite_device
{
public:
	// construction/destruction
	sega_sharrier_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// subclass overrides
	virtual void draw(bitmap_ind16 &bitmap, const rectangle &cliprect) override;

	// memory regions
	required_region_ptr<uint32_t> m_sprite_region_ptr;
};


// ======================> sega_outrun_sprite_device

class sega_outrun_sprite_device : public sega_16bit_sprite_device
{
public:
	// construction/destruction
	sega_outrun_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
protected:
	sega_outrun_sprite_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, bool xboard_variant);

protected:
	// subclass overrides
	virtual void draw(bitmap_ind16 &bitmap, const rectangle &cliprect) override;

	// configuration
	bool            m_is_xboard;
	required_region_ptr<uint32_t> m_sprite_region_ptr;
};

class sega_xboard_sprite_device : public sega_outrun_sprite_device
{
public:
	// construction/destruction
	sega_xboard_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// ======================> sega_sys16a_sprite_device

class sega_sys16a_sprite_device : public sega_16bit_sprite_device
{
public:
	// construction/destruction
	sega_sys16a_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// subclass overrides
	virtual void draw(bitmap_ind16 &bitmap, const rectangle &cliprect) override;

	// memory regions
	required_region_ptr<uint16_t> m_sprite_region_ptr;
};


// ======================> bootleg_sys16a_sprite_device

class bootleg_sys16a_sprite_device : public sega_16bit_sprite_device
{
public:
	// construction/destruction
	bootleg_sys16a_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	void set_remap(uint8_t offs0, uint8_t offs1, uint8_t offs2, uint8_t offs3, uint8_t offs4, uint8_t offs5, uint8_t offs6, uint8_t offs7);

protected:
	// subclass overrides
	virtual void draw(bitmap_ind16 &bitmap, const rectangle &cliprect) override;

	// memory regions
	required_region_ptr<uint16_t> m_sprite_region_ptr;

	// internal state
	uint8_t       m_addrmap[8];
};


// ======================> sega_sys16b_sprite_device

class sega_sys16b_sprite_device : public sega_16bit_sprite_device
{
public:
	// construction/destruction
	sega_sys16b_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// subclass overrides
	virtual void draw(bitmap_ind16 &bitmap, const rectangle &cliprect) override;

	// memory regions
	required_region_ptr<uint16_t> m_sprite_region_ptr;
};


// ======================> sega_yboard_sprite_device

class sega_yboard_sprite_device : public sega_16bit_sprite_device
{
public:
	typedef segaic16_video_device::rotate_info rotate_info;

	// construction/destruction
	sega_yboard_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void set_rotate_ptr(rotate_info* segaic16_rotate) { m_segaic16_rotate = segaic16_rotate; }

protected:
	// subclass overrides
	virtual void draw(bitmap_ind16 &bitmap, const rectangle &cliprect) override;

	required_region_ptr<uint64_t>     m_sprite_region_ptr;
	rotate_info*                    m_segaic16_rotate;
};


// device type definition
DECLARE_DEVICE_TYPE(SEGA_HANGON_SPRITES,    sega_hangon_sprite_device)
DECLARE_DEVICE_TYPE(SEGA_SHARRIER_SPRITES,  sega_sharrier_sprite_device)
DECLARE_DEVICE_TYPE(SEGA_OUTRUN_SPRITES,    sega_outrun_sprite_device)
DECLARE_DEVICE_TYPE(SEGA_SYS16A_SPRITES,    sega_sys16a_sprite_device)
DECLARE_DEVICE_TYPE(BOOTLEG_SYS16A_SPRITES, bootleg_sys16a_sprite_device)
DECLARE_DEVICE_TYPE(SEGA_SYS16B_SPRITES,    sega_sys16b_sprite_device)
DECLARE_DEVICE_TYPE(SEGA_XBOARD_SPRITES,    sega_xboard_sprite_device)
DECLARE_DEVICE_TYPE(SEGA_YBOARD_SPRITES,    sega_yboard_sprite_device)


#endif // MAME_VIDEO_SEGA16SP_H
