/*****************************************************************************
 *
 * includes/llc.h
 *
 ****************************************************************************/

#ifndef LLC_H_
#define LLC_H_

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "machine/z80pio.h"
#include "machine/z80ctc.h"
#include "machine/ram.h"
#include "machine/k7659kb.h"
#include "sound/speaker.h"

class llc_state : public driver_device
{
public:
	llc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_speaker(*this, SPEAKER_TAG),
	m_p_videoram(*this, "videoram"){ }

	DECLARE_WRITE8_MEMBER(llc2_rom_disable_w);
	DECLARE_WRITE8_MEMBER(llc2_basic_enable_w);
	DECLARE_WRITE8_MEMBER(kbd_put);
	DECLARE_READ8_MEMBER(llc1_port1_a_r);
	DECLARE_READ8_MEMBER(llc1_port2_a_r);
	DECLARE_READ8_MEMBER(llc1_port2_b_r);
	DECLARE_WRITE8_MEMBER(llc1_port1_a_w);
	DECLARE_WRITE8_MEMBER(llc1_port1_b_w);
	DECLARE_READ8_MEMBER(llc2_port1_b_r);
	DECLARE_READ8_MEMBER(llc2_port2_a_r);
	DECLARE_WRITE8_MEMBER(llc2_port1_b_w);
	const UINT8 *m_p_chargen;
	optional_device<device_t> m_speaker;
	optional_shared_ptr<UINT8> m_p_videoram;
	bool m_rv;
	UINT8 m_term_status;
	UINT8 m_llc1_key;
private:
	UINT8 m_porta;
	UINT8 m_term_data;
public:
	DECLARE_DRIVER_INIT(llc2);
	DECLARE_DRIVER_INIT(llc1);
	virtual void video_start();
	DECLARE_MACHINE_START(llc1);
	DECLARE_MACHINE_RESET(llc1);
	DECLARE_MACHINE_RESET(llc2);
};


/*----------- defined in machine/llc.c -----------*/
extern MACHINE_START( llc1 );
extern MACHINE_RESET( llc1 );

extern const z80pio_interface llc1_z80pio1_intf;
extern const z80pio_interface llc1_z80pio2_intf;
extern const z80pio_interface llc2_z80pio1_intf;
extern const z80pio_interface llc2_z80pio2_intf;

extern const z80ctc_interface llc1_ctc_intf;
extern const z80ctc_interface llc2_ctc_intf;

extern MACHINE_RESET( llc2 );

/*----------- defined in video/llc.c -----------*/

extern VIDEO_START( llc );
extern SCREEN_UPDATE_IND16( llc1 );
extern SCREEN_UPDATE_IND16( llc2 );

#endif
