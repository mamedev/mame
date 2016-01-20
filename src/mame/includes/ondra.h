// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 * includes/ondra.h
 *
 ****************************************************************************/

#ifndef ONDRA_H_
#define ONDRA_H_

#include "imagedev/cassette.h"
#include "machine/ram.h"

class ondra_state : public driver_device
{
public:
	ondra_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_cassette(*this, "cassette"),
		m_ram(*this, RAM_TAG),
		m_region_maincpu(*this, "maincpu"),
		m_bank1(*this, "bank1"),
		m_bank2(*this, "bank2"),
		m_bank3(*this, "bank3"),
		m_line0(*this, "LINE0"),
		m_line1(*this, "LINE1"),
		m_line2(*this, "LINE2"),
		m_line3(*this, "LINE3"),
		m_line4(*this, "LINE4"),
		m_line5(*this, "LINE5"),
		m_line6(*this, "LINE6"),
		m_line7(*this, "LINE7"),
		m_line8(*this, "LINE8"),
		m_line9(*this, "LINE9"),
		m_nmi(*this, "NMI") { }

	UINT8 m_video_enable;
	UINT8 m_bank1_status;
	UINT8 m_bank2_status;
	DECLARE_READ8_MEMBER(ondra_keyboard_r);
	DECLARE_WRITE8_MEMBER(ondra_port_03_w);
	DECLARE_WRITE8_MEMBER(ondra_port_09_w);
	DECLARE_WRITE8_MEMBER(ondra_port_0a_w);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_ondra(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(ondra_interrupt);
	TIMER_CALLBACK_MEMBER(nmi_check_callback);

protected:
	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cassette;
	required_device<ram_device> m_ram;
	required_memory_region m_region_maincpu;
	required_memory_bank m_bank1;
	required_memory_bank m_bank2;
	required_memory_bank m_bank3;
	required_ioport m_line0;
	required_ioport m_line1;
	required_ioport m_line2;
	required_ioport m_line3;
	required_ioport m_line4;
	required_ioport m_line5;
	required_ioport m_line6;
	required_ioport m_line7;
	required_ioport m_line8;
	required_ioport m_line9;
	required_ioport m_nmi;

	void ondra_update_banks();
};

#endif
