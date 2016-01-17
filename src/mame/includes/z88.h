// license:GPL-2.0+
// copyright-holders:Kevin Thacker,Sandro Ronco
/*****************************************************************************
 *
 * includes/z88.h
 *
 ****************************************************************************/

#ifndef Z88_H_
#define Z88_H_

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/upd65031.h"
#include "machine/ram.h"
#include "bus/z88/z88.h"
#include "bus/z88/flash.h"
#include "bus/z88/ram.h"
#include "bus/z88/rom.h"
#include "sound/speaker.h"
#include "rendlay.h"

#define Z88_NUM_COLOURS 3

#define Z88_SCREEN_WIDTH        640
#define Z88_SCREEN_HEIGHT       64

#define Z88_SCR_HW_REV  (1<<4)
#define Z88_SCR_HW_HRS  (1<<5)
#define Z88_SCR_HW_UND  (1<<1)
#define Z88_SCR_HW_FLS  (1<<3)
#define Z88_SCR_HW_GRY  (1<<2)
#define Z88_SCR_HW_CURS (Z88_SCR_HW_HRS|Z88_SCR_HW_FLS|Z88_SCR_HW_REV)
#define Z88_SCR_HW_NULL (Z88_SCR_HW_HRS|Z88_SCR_HW_GRY|Z88_SCR_HW_REV)

enum
{
	Z88_BANK_ROM = 1,
	Z88_BANK_RAM,
	Z88_BANK_CART,
	Z88_BANK_UNMAP
};


class z88_state : public driver_device
{
public:
	z88_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_ram(*this, RAM_TAG),
			m_palette(*this, "palette")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<palette_device> m_palette;

	virtual void machine_start() override;
	virtual void machine_reset() override;
	DECLARE_READ8_MEMBER(kb_r);
	UPD65031_MEMORY_UPDATE(bankswitch_update);
	UPD65031_SCREEN_UPDATE(lcd_update);

	// cartridges read/write
	DECLARE_READ8_MEMBER(bank0_cart_r);
	DECLARE_READ8_MEMBER(bank1_cart_r);
	DECLARE_READ8_MEMBER(bank2_cart_r);
	DECLARE_READ8_MEMBER(bank3_cart_r);
	DECLARE_WRITE8_MEMBER(bank0_cart_w);
	DECLARE_WRITE8_MEMBER(bank1_cart_w);
	DECLARE_WRITE8_MEMBER(bank2_cart_w);
	DECLARE_WRITE8_MEMBER(bank3_cart_w);

	// defined in video/z88.c
	inline void plot_pixel(bitmap_ind16 &bitmap, int x, int y, UINT16 color);
	inline UINT8* convert_address(UINT32 offset);
	void vh_render_8x8(bitmap_ind16 &bitmap, int x, int y, UINT16 pen0, UINT16 pen1, UINT8 *gfx);
	void vh_render_6x8(bitmap_ind16 &bitmap, int x, int y, UINT16 pen0, UINT16 pen1, UINT8 *gfx);
	void vh_render_line(bitmap_ind16 &bitmap, int x, int y, UINT16 pen);

	struct
	{
		UINT8 slot;
		UINT8 page;
	} m_bank[4];

	int                   m_bank_type[4];
	UINT8 *               m_bios;
	UINT8 *               m_ram_base;
	z88cart_slot_device * m_carts[4];
	DECLARE_PALETTE_INIT(z88);
};

#endif /* Z88_H_ */
