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

#ifndef CONCEPT_H_
#define CONCEPT_H_

#include "cpu/m68000/m68000.h"
#include "machine/6522via.h"
#include "machine/mos6551.h"
#include "machine/mm58274c.h"   /* mm58274 seems to be compatible with mm58174 */
#include "sound/speaker.h"
#include "bus/a2bus/a2bus.h"

#define ACIA_0_TAG  "acia0"
#define ACIA_1_TAG  "acia1"
#define KBD_ACIA_TAG "kbacia"
#define SPEAKER_TAG "spkr"
#define A2BUS_TAG "a2bus"

class concept_state : public driver_device
{
public:
	concept_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_acia0(*this, ACIA_0_TAG),
		m_acia1(*this, ACIA_1_TAG),
		m_kbdacia(*this, KBD_ACIA_TAG),
		m_speaker(*this, SPEAKER_TAG),
		m_mm58274(*this,"mm58274c"),
		m_a2bus(*this, A2BUS_TAG),
		m_videoram(*this,"videoram") { }

	required_device<cpu_device> m_maincpu;
	required_device<mos6551_device> m_acia0;
	required_device<mos6551_device> m_acia1;
	required_device<mos6551_device> m_kbdacia;
	required_device<speaker_sound_device> m_speaker;
	required_device<mm58274c_device> m_mm58274;
	required_device<a2bus_device> m_a2bus;
	required_shared_ptr<uint16_t> m_videoram;

	uint8_t m_pending_interrupts;
	bool m_clock_enable;
	uint8_t m_clock_address;
	uint16_t concept_io_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void concept_io_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_concept(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	uint8_t via_in_a(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void via_out_a(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t via_in_b(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void via_out_b(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void via_out_cb2(int state);
	void via_irq_func(int state);
	void concept_set_interrupt(int level, int state);
};

#endif /* CONCEPT_H_ */
