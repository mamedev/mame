// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina, David Haywood
/* ST0016 - CPU (z80) + Sound + Video */

#pragma once

#ifndef __ST0016_CPU__
#define __ST0016_CPU__

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/st0016.h"

extern UINT8 macs_cart_slot;

#define ISMACS  (st0016_game&0x80)
#define ISMACS1 (((st0016_game&0x180)==0x180))
#define ISMACS2 (((st0016_game&0x180)==0x080))


#define ST0016_MAX_SPR_BANK   0x10
#define ST0016_MAX_CHAR_BANK  0x10000
#define ST0016_MAX_PAL_BANK   4

#define ST0016_SPR_BANK_SIZE  0x1000
#define ST0016_CHAR_BANK_SIZE 0x20
#define ST0016_PAL_BANK_SIZE  0x200

#define UNUSED_PEN 1024

#define ST0016_SPR_BANK_MASK  (ST0016_MAX_SPR_BANK-1)
#define ST0016_CHAR_BANK_MASK (ST0016_MAX_CHAR_BANK-1)
#define ST0016_PAL_BANK_MASK  (ST0016_MAX_PAL_BANK-1)



class st0016_cpu_device : public z80_device
{
public:
	st0016_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32);

	DECLARE_WRITE8_MEMBER(st0016_sprite_bank_w);
	DECLARE_WRITE8_MEMBER(st0016_palette_bank_w);
	DECLARE_WRITE8_MEMBER(st0016_character_bank_w);
	DECLARE_READ8_MEMBER(st0016_sprite_ram_r);
	DECLARE_WRITE8_MEMBER(st0016_sprite_ram_w);
	DECLARE_READ8_MEMBER(st0016_sprite2_ram_r);
	DECLARE_WRITE8_MEMBER(st0016_sprite2_ram_w);
	DECLARE_READ8_MEMBER(st0016_palette_ram_r);
	DECLARE_WRITE8_MEMBER(st0016_palette_ram_w);
	DECLARE_READ8_MEMBER(st0016_character_ram_r);
	DECLARE_WRITE8_MEMBER(st0016_character_ram_w);
	DECLARE_READ8_MEMBER(st0016_vregs_r);
	DECLARE_READ8_MEMBER(st0016_dma_r);
	DECLARE_WRITE8_MEMBER(st0016_vregs_w);
	DECLARE_READ8_MEMBER(soundram_read);


	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void st0016_save_init();
	void draw_bgmap(bitmap_ind16 &bitmap,const rectangle &cliprect, int priority);

	void startup();
	UINT32 update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void st0016_draw_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	UINT8 *st0016_spriteram,*st0016_paletteram;

	UINT32 st0016_game;


	INT32 st0016_spr_bank,st0016_spr2_bank,st0016_pal_bank,st0016_char_bank;
	int spr_dx,spr_dy;

	UINT8 st0016_vregs[0xc0];
	int st0016_ramgfx;
	UINT8 *m_charram;

protected:
	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

	const address_space_config m_io_space_config;
	const address_space_config m_space_config;


	const address_space_config *memory_space_config(address_spacenum spacenum) const override
	{
		switch (spacenum)
		{
			case AS_IO: return &m_io_space_config;
			case AS_PROGRAM: return &m_space_config;
			default: return z80_device::memory_space_config(spacenum);
		}
	};
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;

public: // speglsht needs to access this for mixing
	required_device<palette_device> m_palette;

private:

};


// device type definition
extern const device_type ST0016_CPU;


#endif /// __ST0016_CPU__
