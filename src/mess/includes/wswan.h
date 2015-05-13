// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*****************************************************************************
 *
 * includes/wswan.h
 *
 ****************************************************************************/

#ifndef WSWAN_H_
#define WSWAN_H_

#define WSWAN_TYPE_MONO 0
#define WSWAN_TYPE_COLOR 1

#define INTERNAL_EEPROM_SIZE    1024

#include "emu.h"
#include "cpu/v30mz/v30mz.h"
#include "machine/nvram.h"
#include "audio/wswan_snd.h"
#include "video/wswan_video.h"
#include "bus/wswan/slot.h"
#include "bus/wswan/rom.h"


struct SoundDMA
{
	UINT32  source;     /* Source address */
	UINT16  size;       /* Size */
	UINT8   enable;     /* Enabled */
};


class wswan_state : public driver_device
{
public:
	wswan_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_vdp(*this, "vdp"),
		m_sound(*this, "custom"),
		m_cart(*this, "cartslot"),
		m_cursx(*this, "CURSX"),
		m_cursy(*this, "CURSY"),
		m_buttons(*this, "BUTTONS")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<wswan_video_device> m_vdp;
	required_device<wswan_sound_device> m_sound;
	required_device<ws_cart_slot_device> m_cart;
	DECLARE_READ8_MEMBER(bios_r);
	DECLARE_READ8_MEMBER(port_r);
	DECLARE_WRITE8_MEMBER(port_w);

	UINT8 m_ws_portram[256];
	UINT8 m_internal_eeprom[INTERNAL_EEPROM_SIZE];
	UINT8 m_system_type;
	SoundDMA m_sound_dma;
	UINT8 *m_ws_bios_bank;
	UINT8 m_bios_disabled;
	UINT8 m_rotate;

	void set_irq_line(int irq);
	void dma_sound_cb();
	void common_start();
	virtual void machine_start();
	virtual void machine_reset();
	DECLARE_PALETTE_INIT(wswan);
	DECLARE_MACHINE_START(wscolor);
	DECLARE_PALETTE_INIT(wscolor);

protected:
	/* Interrupt flags */
	static const UINT8 WSWAN_IFLAG_STX    = 0x01;
	static const UINT8 WSWAN_IFLAG_KEY    = 0x02;
	static const UINT8 WSWAN_IFLAG_RTC    = 0x04;
	static const UINT8 WSWAN_IFLAG_SRX    = 0x08;
	static const UINT8 WSWAN_IFLAG_LCMP   = 0x10;
	static const UINT8 WSWAN_IFLAG_VBLTMR = 0x20;
	static const UINT8 WSWAN_IFLAG_VBL    = 0x40;
	static const UINT8 WSWAN_IFLAG_HBLTMR = 0x80;

	/* Interrupts */
	static const UINT8 WSWAN_INT_STX    = 0;
	static const UINT8 WSWAN_INT_KEY    = 1;
	static const UINT8 WSWAN_INT_RTC    = 2;
	static const UINT8 WSWAN_INT_SRX    = 3;
	static const UINT8 WSWAN_INT_LCMP   = 4;
	static const UINT8 WSWAN_INT_VBLTMR = 5;
	static const UINT8 WSWAN_INT_VBL    = 6;
	static const UINT8 WSWAN_INT_HBLTMR = 7;

	required_ioport m_cursx;
	required_ioport m_cursy;
	required_ioport m_buttons;

	void register_save();
	void handle_irqs();
	void clear_irq_line(int irq);
};


#endif /* WSWAN_H_ */
