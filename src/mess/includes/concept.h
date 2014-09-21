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

#include "machine/6522via.h"
#include "machine/mos6551.h"
#include "machine/mm58274c.h"   /* mm58274 seems to be compatible with mm58174 */
#include "machine/concept_exp.h"
#include "sound/speaker.h"

#define ACIA_0_TAG  "acia0"
#define ACIA_1_TAG  "acia1"
#define KBD_ACIA_TAG "kbacia"
#define SPEAKER_TAG	"spkr"

/* keyboard interface */
enum
{
	KeyQueueSize = 32,
	MaxKeyMessageLen = 1
};


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
		m_videoram(*this,"videoram") { }

	required_device<cpu_device> m_maincpu;
	required_device<mos6551_device> m_acia0;
	required_device<mos6551_device> m_acia1;
	required_device<mos6551_device> m_kbdacia;
	required_device<speaker_sound_device> m_speaker;
	required_device<mm58274c_device> m_mm58274;
	required_shared_ptr<UINT16> m_videoram;

	concept_exp_port_device *m_exp[4];

	UINT8 m_pending_interrupts;
	bool m_clock_enable;
	UINT8 m_clock_address;
	DECLARE_READ16_MEMBER(concept_io_r);
	DECLARE_WRITE16_MEMBER(concept_io_w);
	virtual void machine_start();
	virtual void video_start();
	UINT32 screen_update_concept(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER(via_in_a);
	DECLARE_WRITE8_MEMBER(via_out_a);
	DECLARE_READ8_MEMBER(via_in_b);
	DECLARE_WRITE8_MEMBER(via_out_b);
	DECLARE_WRITE_LINE_MEMBER(via_out_cb2);
	DECLARE_WRITE_LINE_MEMBER(via_irq_func);
	void concept_set_interrupt(int level, int state);
};

#endif /* CONCEPT_H_ */
