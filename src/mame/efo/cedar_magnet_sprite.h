// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_EFO_CEDAR_MAGNET_SPRITE_H
#define MAME_EFO_CEDAR_MAGNET_SPRITE_H

#pragma once

#include "cedar_magnet_board.h"

#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "machine/z80pio.h"
#include "machine/bankdev.h"

DECLARE_DEVICE_TYPE(CEDAR_MAGNET_SPRITE, cedar_magnet_sprite_device)

class cedar_magnet_sprite_device : public device_t, public cedar_magnet_board_interface
{
public:
	// construction/destruction
	cedar_magnet_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void sprite_port80_w(u8 data);
	void sprite_port84_w(u8 data);
	void sprite_port88_w(u8 data);
	void sprite_port8c_w(u8 data);
	void sprite_port9c_w(u8 data);

	u8 exzisus_hack_r(offs_t offset);

	INTERRUPT_GEN_MEMBER(irq);

	u32 draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int palbase);

	void cedar_magnet_sprite_io(address_map &map) ATTR_COLD;
	void cedar_magnet_sprite_map(address_map &map) ATTR_COLD;
	void cedar_magnet_sprite_sub_ram_map(address_map &map) ATTR_COLD;
protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void do_blit();

	std::unique_ptr<u8[]> m_framebuffer;
	u8 m_pio2_pb_data = 0;

	required_device<address_map_bank_device> m_sprite_ram_bankdev;

	u8 m_upperaddr = 0;
	u8 m_loweraddr = 0;

	u8 m_spritesize = 0;
	u8 m_pio0_pb_data = 0;
	u8 m_spritecodelow = 0;
	u8 m_spritecodehigh = 0;

	int m_high_write = 0;
	u8 m_uppersprite = 0;

	u8 pio0_pa_r();
	void pio0_pa_w(u8 data);
//  u8 pio0_pb_r();
	void pio0_pb_w(u8 data);

//  u8 pio1_pa_r();
	void pio1_pa_w(u8 data);
//  u8 pio1_pb_r();
	void pio1_pb_w(u8 data);

//  u8 pio2_pa_r();
	void pio2_pa_w(u8 data);
//  u8 pio2_pb_r();
	void pio2_pb_w(u8 data);

	required_device_array<z80pio_device, 3> m_pio;
};

#endif // MAME_EFO_CEDAR_MAGNET_SPRITE_H
