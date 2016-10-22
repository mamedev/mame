// license:BSD-3-Clause
// copyright-holders:David Haywood

#pragma once

#ifndef CEDAR_MAGNET_SPRITE_DEF
#define CEDAR_MAGNET_SPRITE_DEF

#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "machine/z80pio.h"
#include "machine/bankdev.h"
#include "cedar_magnet_board.h"

extern const device_type CEDAR_MAGNET_SPRITE;

#define MCFG_CEDAR_MAGNET_SPRITE_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, CEDAR_MAGNET_SPRITE, 0)


class cedar_magnet_sprite_device :  public cedar_magnet_board_device
{
public:
	// construction/destruction
	cedar_magnet_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	
	UINT8 m_framebuffer[0x10000];
	UINT8 pio2_pb_data;

	required_device<address_map_bank_device> m_sprite_ram_bankdev;

	DECLARE_READ8_MEMBER(pio0_pa_r);
	DECLARE_WRITE8_MEMBER(pio0_pa_w);
//	DECLARE_READ8_MEMBER(pio0_pb_r);
	DECLARE_WRITE8_MEMBER(pio0_pb_w);

//	DECLARE_READ8_MEMBER(pio1_pa_r);
	DECLARE_WRITE8_MEMBER(pio1_pa_w);
//	DECLARE_READ8_MEMBER(pio1_pb_r);
	DECLARE_WRITE8_MEMBER(pio1_pb_w);

//	DECLARE_READ8_MEMBER(pio2_pa_r);
	DECLARE_WRITE8_MEMBER(pio2_pa_w);
//	DECLARE_READ8_MEMBER(pio2_pb_r);
	DECLARE_WRITE8_MEMBER(pio2_pb_w);

	DECLARE_WRITE8_MEMBER(sprite_port80_w);
	DECLARE_WRITE8_MEMBER(sprite_port84_w);
	DECLARE_WRITE8_MEMBER(sprite_port88_w);
	DECLARE_WRITE8_MEMBER(sprite_port8c_w);
	DECLARE_WRITE8_MEMBER(sprite_port9c_w);

	DECLARE_READ8_MEMBER(exzisus_hack_r);

	UINT8 m_upperaddr;
	UINT8 m_loweraddr;
	
	void do_blit();

	UINT8 m_spritesize;
	UINT8 pio0_pb_data;
	UINT8 m_spritecodelow;
	UINT8 m_spritecodehigh;
	
	int m_high_write;
	UINT8 m_uppersprite;

	INTERRUPT_GEN_MEMBER(irq);

	required_device<z80pio_device> m_pio0;
	required_device<z80pio_device> m_pio1;
	required_device<z80pio_device> m_pio2;
	
	UINT32 draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int palbase);
protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
};

#endif
