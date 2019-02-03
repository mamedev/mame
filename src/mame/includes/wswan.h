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

#define WSWAN_TYPE_MONO 0
#define WSWAN_TYPE_COLOR 1

#define INTERNAL_EEPROM_SIZE    1024

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
		m_cursx(*this, "CURSX"),
		m_cursy(*this, "CURSY"),
		m_buttons(*this, "BUTTONS")
	{ }

	void wswan(machine_config &config);

protected:
	/* Interrupt flags */
	static const uint8_t WSWAN_IFLAG_STX    = 0x01;
	static const uint8_t WSWAN_IFLAG_KEY    = 0x02;
	static const uint8_t WSWAN_IFLAG_RTC    = 0x04;
	static const uint8_t WSWAN_IFLAG_SRX    = 0x08;
	static const uint8_t WSWAN_IFLAG_LCMP   = 0x10;
	static const uint8_t WSWAN_IFLAG_VBLTMR = 0x20;
	static const uint8_t WSWAN_IFLAG_VBL    = 0x40;
	static const uint8_t WSWAN_IFLAG_HBLTMR = 0x80;

	/* Interrupts */
	static const uint8_t WSWAN_INT_STX    = 0;
	static const uint8_t WSWAN_INT_KEY    = 1;
	static const uint8_t WSWAN_INT_RTC    = 2;
	static const uint8_t WSWAN_INT_SRX    = 3;
	static const uint8_t WSWAN_INT_LCMP   = 4;
	static const uint8_t WSWAN_INT_VBLTMR = 5;
	static const uint8_t WSWAN_INT_VBL    = 6;
	static const uint8_t WSWAN_INT_HBLTMR = 7;

	struct SoundDMA
	{
		uint32_t  source;     /* Source address */
		uint16_t  size;       /* Size */
		uint8_t   enable;     /* Enabled */
	};

	required_device<cpu_device> m_maincpu;
	required_device<wswan_video_device> m_vdp;
	required_device<wswan_sound_device> m_sound;
	required_device<ws_cart_slot_device> m_cart;

	uint8_t m_ws_portram[256];
	uint8_t m_internal_eeprom[INTERNAL_EEPROM_SIZE];
	uint8_t m_system_type;
	SoundDMA m_sound_dma;
	std::unique_ptr<uint8_t[]> m_ws_bios_bank;
	uint8_t m_bios_disabled;
	uint8_t m_rotate;

	required_ioport m_cursx;
	required_ioport m_cursy;
	required_ioport m_buttons;

	DECLARE_READ8_MEMBER(bios_r);
	DECLARE_READ8_MEMBER(port_r);
	DECLARE_WRITE8_MEMBER(port_w);

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
};

#endif // MAME_INCLUDES_WSWAN_H
