// license:GPL-2.0+
// copyright-holders:Raphael Nabet, Brett Wyer
/*****************************************************************************
 *
 * includes/concept.h
 *
 * Corvus Concept driver
 *
 * Raphael Nabet, 2003
 *
 ****************************************************************************/

#ifndef MAME_CONCEPT_CONCEPT_H
#define MAME_CONCEPT_CONCEPT_H

#include "cpu/m68000/m68000.h"
#include "machine/6522via.h"
#include "machine/mos6551.h"
#include "machine/mm58174.h"
#include "sound/spkrdev.h"
#include "bus/a2bus/a2bus.h"

#define ACIA_0_TAG   "acia0"
#define ACIA_1_TAG   "acia1"
#define VIA_0_TAG    "via6522_0"
#define KBD_ACIA_TAG "kbacia"

class concept_state : public driver_device
{
public:
	concept_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_acia0(*this, ACIA_0_TAG),
		m_acia1(*this, ACIA_1_TAG),
		m_via0(*this, VIA_0_TAG),
		m_kbdacia(*this, KBD_ACIA_TAG),
		m_speaker(*this, "spkr"),
		m_mm58174(*this, "mm58174"),
		m_a2bus(*this, "a2bus"),
		m_videoram(*this,"videoram")
	{ }

	void corvus_concept(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<mos6551_device> m_acia0;
	required_device<mos6551_device> m_acia1;
	required_device<via6522_device> m_via0;
	required_device<mos6551_device> m_kbdacia;
	required_device<speaker_sound_device> m_speaker;
	required_device<mm58174_device> m_mm58174;
	required_device<a2bus_device> m_a2bus;
	required_shared_ptr<uint16_t> m_videoram;

	bool m_clock_enable = false;
	uint8_t m_clock_address = 0U;
	uint8_t io_r(offs_t offset);
	void io_w(offs_t offset, uint8_t data);
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	uint8_t via_in_a();
	void via_out_a(uint8_t data);
	uint8_t via_in_b();
	void via_out_b(uint8_t data);
	void via_out_cb2(int state);

	void concept_memmap(address_map &map) ATTR_COLD;
};

#endif // MAME_CONCEPT_CONCEPT_H
