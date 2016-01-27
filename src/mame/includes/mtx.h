// license:BSD-3-Clause
// copyright-holders:Lee Ward, Dirk Best, Curt Coder
/*************************************************************************

    Memotech MTX 500, MTX 512 and RS 128

*************************************************************************/

#ifndef __MTX__
#define __MTX__

#include "imagedev/snapquik.h"
#include "imagedev/cassette.h"
#include "bus/centronics/ctronics.h"
#include "machine/z80dart.h"
#include "machine/z80ctc.h"
#include "sound/sn76496.h"
#include "machine/ram.h"

#define Z80_TAG         "z80"
#define Z80CTC_TAG      "z80ctc"
#define Z80DART_TAG     "z80dart"
#define FD1793_TAG      "fd1793" // SDX
#define FD1791_TAG      "fd1791" // FDX
#define SN76489A_TAG    "sn76489a"
#define MC6845_TAG      "mc6845"
#define SCREEN_TAG      "screen"
#define CENTRONICS_TAG  "centronics"

class mtx_state : public driver_device
{
public:
	mtx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, Z80_TAG),
		m_sn(*this, SN76489A_TAG),
		m_z80ctc(*this, Z80CTC_TAG),
		m_z80dart(*this, Z80DART_TAG),
		m_cassette(*this, "cassette"),
		m_centronics(*this, CENTRONICS_TAG),
		m_ram(*this, RAM_TAG)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<sn76489a_device> m_sn;
	required_device<z80ctc_device> m_z80ctc;
	optional_device<z80dart_device> m_z80dart;
	required_device<cassette_image_device> m_cassette;
	required_device<centronics_device> m_centronics;
	required_device<ram_device> m_ram;

	/* keyboard state */
	UINT8 m_key_sense;

	/* video state */
	UINT8 *m_video_ram;
	UINT8 *m_attr_ram;

	/* sound state */
	UINT8 m_sound_latch;

	/* timers */
	device_t *m_cassette_timer;

	int m_centronics_busy;
	int m_centronics_fault;
	int m_centronics_perror;
	int m_centronics_select;

	DECLARE_WRITE8_MEMBER(mtx_bankswitch_w);
	DECLARE_WRITE8_MEMBER(mtx_sound_latch_w);
	DECLARE_WRITE8_MEMBER(mtx_sense_w);
	DECLARE_READ8_MEMBER(mtx_key_lo_r);
	DECLARE_READ8_MEMBER(mtx_key_hi_r);
	DECLARE_WRITE8_MEMBER(hrx_address_w);
	DECLARE_READ8_MEMBER(hrx_data_r);
	DECLARE_WRITE8_MEMBER(hrx_data_w);
	DECLARE_READ8_MEMBER(hrx_attr_r);
	DECLARE_WRITE8_MEMBER(hrx_attr_w);
	DECLARE_MACHINE_START(mtx512);
	DECLARE_MACHINE_RESET(mtx512);
	TIMER_DEVICE_CALLBACK_MEMBER(ctc_tick);
	TIMER_DEVICE_CALLBACK_MEMBER(cassette_tick);
	DECLARE_WRITE_LINE_MEMBER(ctc_trg1_w);
	DECLARE_WRITE_LINE_MEMBER(ctc_trg2_w);
	DECLARE_WRITE_LINE_MEMBER(mtx_tms9929a_interrupt);
	DECLARE_READ8_MEMBER(mtx_strobe_r);
	DECLARE_READ8_MEMBER(mtx_sound_strobe_r);
	DECLARE_WRITE8_MEMBER(mtx_cst_w);
	DECLARE_READ8_MEMBER(mtx_prt_r);
	DECLARE_WRITE_LINE_MEMBER(write_centronics_busy);
	DECLARE_WRITE_LINE_MEMBER(write_centronics_fault);
	DECLARE_WRITE_LINE_MEMBER(write_centronics_perror);
	DECLARE_WRITE_LINE_MEMBER(write_centronics_select);
	void bankswitch(UINT8 data);
	DECLARE_SNAPSHOT_LOAD_MEMBER( mtx );
};

#endif /* __MTX_H__ */
