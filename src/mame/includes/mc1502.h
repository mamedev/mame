// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/*****************************************************************************
 *
 * includes/mc1502.h
 *
 ****************************************************************************/

#ifndef MC1502_H_
#define MC1502_H_

#include "imagedev/cassette.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "bus/isa/isa.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/ram.h"
#include "bus/isa/xsu_cards.h"
#include "sound/speaker.h"
#include "bus/centronics/ctronics.h"

class mc1502_state : public driver_device
{
public:
	mc1502_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_upd8251(*this, "upd8251"),
		m_pic8259(*this, "pic8259"),
		m_pit8253(*this, "pit8253"),
		m_ppi8255n1(*this, "ppi8255n1"),
		m_ppi8255n2(*this, "ppi8255n2"),
		m_isabus(*this, "isa"),
		m_speaker(*this, "speaker"),
		m_cassette(*this, "cassette"),
		m_centronics(*this, "centronics"),
		m_ram(*this, RAM_TAG) { }

	required_device<cpu_device>  m_maincpu;
	required_device<i8251_device> m_upd8251;
	required_device<pic8259_device>  m_pic8259;
	required_device<pit8253_device>  m_pit8253;
	required_device<i8255_device>  m_ppi8255n1;
	required_device<i8255_device>  m_ppi8255n2;
	required_device<isa8_device>  m_isabus;
	required_device<speaker_sound_device>  m_speaker;
	required_device<cassette_image_device>  m_cassette;
	required_device<centronics_device> m_centronics;
	required_device<ram_device> m_ram;

	DECLARE_DRIVER_INIT(mc1502);
	DECLARE_MACHINE_START(mc1502);
	DECLARE_MACHINE_RESET(mc1502);

	TIMER_CALLBACK_MEMBER(keyb_signal_callback);

	struct {
		UINT8       pulsing;
		UINT16      mask;       /* input lines */
		emu_timer   *keyb_signal_timer;
	} m_kbd;

	UINT8 m_ppi_portb;
	UINT8 m_ppi_portc;
	UINT8 m_spkrdata;

	DECLARE_WRITE_LINE_MEMBER(mc1502_pit8253_out1_changed);
	DECLARE_WRITE_LINE_MEMBER(mc1502_pit8253_out2_changed);
	DECLARE_WRITE_LINE_MEMBER(mc1502_speaker_set_spkrdata);
	DECLARE_WRITE_LINE_MEMBER(mc1502_i8251_syndet);

	DECLARE_WRITE8_MEMBER(mc1502_ppi_portb_w);
	DECLARE_WRITE8_MEMBER(mc1502_ppi_portc_w);
	DECLARE_READ8_MEMBER(mc1502_ppi_portc_r);
	DECLARE_READ8_MEMBER(mc1502_kppi_porta_r);
	DECLARE_WRITE8_MEMBER(mc1502_kppi_portb_w);
	DECLARE_WRITE8_MEMBER(mc1502_kppi_portc_w);

private:
	int m_pit_out2;
};

#endif /* MC1502_H_ */
