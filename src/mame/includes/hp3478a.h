// license:BSD-3-Clause
// copyright-holders:fenugrec
/***************************************************************************
 HP 3478A Digital Multimeter

 XXX Not sure why I need this file, can this all go in hp3478a.cpp ?
****************************************************************************/

#pragma once

#ifndef MAME_INCLUDES_HP3478A_H
#define MAME_INCLUDES_HP3478A_H

#include "cpu/mcs48/mcs48.h"

/* port pin/bit defs */
#define P20	(1 << 0)
#define P21	(1 << 1)
#define P22	(1 << 2)
#define P23	(1 << 3)
#define P24	(1 << 4)
#define P25	(1 << 5)
#define P26	(1 << 6)
#define P27	(1 << 7)

#define VFD_TAG     "vfd"

class hp3478a_state : public driver_device
{
public:
	hp3478a_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_bank0(*this, "bank0")
	{
	}

	void hp3478a(machine_config &config);
	void i8039_io(address_map &map);
	void i8039_map(address_map &map);

protected:
	virtual void machine_start() override;
	//virtual void machine_reset() override;	//not needed?

	DECLARE_WRITE8_MEMBER(p1write);
	DECLARE_READ8_MEMBER(p1read);
	DECLARE_WRITE8_MEMBER(p2write);
	DECLARE_READ8_MEMBER(busread);
	DECLARE_WRITE8_MEMBER(buswrite);

	required_device<i8039_device> m_maincpu;
	required_memory_bank m_bank0;

	/////////////// stuff for internal LCD emulation
	// could be split to a separate driver ?
	std::unique_ptr<output_finder<16> > m_outputs;

	void lcd_interface(uint8_t p2new);
	void lcd_map_chars(void);
	uint8_t lcd_bitcount;
	uint8_t lcd_want;
	uint64_t lcd_bitbuf;
	enum class lcd_state : uint8_t {
		IDLE,
		SYNC_SKIP,
		SELECTED_ISA,
		SELECTED_IWA
	} m_lcdstate;
	enum class lcd_iwatype : uint8_t {
		ANNUNS,
		REG_A,
		REG_B,
		REG_C,
		DISCARD
	} m_lcdiwa;
	uint8_t lcd_chrbuf[12];	//raw digits (not ASCII)
	uint8_t lcd_text[13];	//mapped to ASCII
	uint32_t lcd_segdata[12];

	uint8_t p2_oldstate;	//used to detect edges on Port2 IO pins. Should be saveable ?

};


#endif // MAME_INCLUDES_HP3478A_H
