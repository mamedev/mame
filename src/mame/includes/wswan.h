// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*****************************************************************************
 *
 * includes/wswan.h
 *
 ****************************************************************************/
#ifndef MAME_INCLUDES_WSWAN_H
#define MAME_INCLUDES_WSWAN_H

#pragma once

#include "cpu/v30mz/v30mz.h"
#include "machine/nvram.h"
#include "audio/wswan.h"
#include "video/wswan.h"
#include "bus/wswan/slot.h"
#include "bus/wswan/rom.h"
#include "emupal.h"


class wswan_state : public driver_device
{
public:
	wswan_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_vdp(*this, "vdp"),
		m_sound(*this, "custom"),
		m_cart(*this, "cartslot"),
		m_region_maincpu(*this, "maincpu"),
		m_cursx(*this, "CURSX"),
		m_cursy(*this, "CURSY"),
		m_buttons(*this, "BUTTONS")
	{ }

	void wswan(machine_config &config);

protected:
	// Interrupt flags
	static const u8 WSWAN_IFLAG_STX    = 0x01;
	static const u8 WSWAN_IFLAG_KEY    = 0x02;
	static const u8 WSWAN_IFLAG_RTC    = 0x04;
	static const u8 WSWAN_IFLAG_SRX    = 0x08;
	static const u8 WSWAN_IFLAG_LCMP   = 0x10;
	static const u8 WSWAN_IFLAG_VBLTMR = 0x20;
	static const u8 WSWAN_IFLAG_VBL    = 0x40;
	static const u8 WSWAN_IFLAG_HBLTMR = 0x80;

	// Interrupts
	static const u8 WSWAN_INT_STX    = 0;
	static const u8 WSWAN_INT_KEY    = 1;
	static const u8 WSWAN_INT_RTC    = 2;
	static const u8 WSWAN_INT_SRX    = 3;
	static const u8 WSWAN_INT_LCMP   = 4;
	static const u8 WSWAN_INT_VBLTMR = 5;
	static const u8 WSWAN_INT_VBL    = 6;
	static const u8 WSWAN_INT_HBLTMR = 7;

	static const u32 INTERNAL_EEPROM_SIZE = 1024;	// 16kbit on WSC
	static const u32 INTERNAL_EEPROM_SIZE_WS = 64;	// 1kbit on WS

	enum enum_system { TYPE_WSWAN=0, TYPE_WSC };

	struct sound_dma_t
	{
		u32  source;     // Source address
		u16  size;       // Size
		u8   enable;     // Enabled
	};

	required_device<cpu_device> m_maincpu;
	required_device<wswan_video_device> m_vdp;
	required_device<wswan_sound_device> m_sound;
	required_device<ws_cart_slot_device> m_cart;

	u8 m_ws_portram[256];
	u8 m_internal_eeprom[INTERNAL_EEPROM_SIZE * 2];
	u8 m_system_type;
	sound_dma_t m_sound_dma;
	u8 m_bios_disabled;
	u8 m_rotate;

	required_memory_region m_region_maincpu;
	required_ioport m_cursx;
	required_ioport m_cursy;
	required_ioport m_buttons;

	u8 bios_r(offs_t offset);
	u8 port_r(offs_t offset);
	void port_w(offs_t offset, u8 data);

	void set_irq_line(int irq);
	void dma_sound_cb();
	void common_start();
	virtual void machine_start() override;
	virtual void machine_reset() override;
	void wswan_palette(palette_device &palette) const;

	void wswan_io(address_map &map);
	void wswan_mem(address_map &map);
	void wswan_snd(address_map &map);

	void register_save();
	void handle_irqs();
	void clear_irq_line(int irq);
	virtual u16 get_internal_eeprom_address();
};

class wscolor_state : public wswan_state
{
public:
	using wswan_state::wswan_state;
	void wscolor(machine_config &config);

protected:
	virtual void machine_start() override;
	void wscolor_mem(address_map &map);
	void wscolor_palette(palette_device &palette) const;
	virtual u16 get_internal_eeprom_address() override;
};

#endif // MAME_INCLUDES_WSWAN_H
