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
	uint8_t m_key_sense;

	/* video state */
	uint8_t *m_video_ram;
	uint8_t *m_attr_ram;

	/* sound state */
	uint8_t m_sound_latch;

	/* timers */
	device_t *m_cassette_timer;

	int m_centronics_busy;
	int m_centronics_fault;
	int m_centronics_perror;
	int m_centronics_select;

	void mtx_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mtx_sound_latch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mtx_sense_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mtx_key_lo_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t mtx_key_hi_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void hrx_address_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t hrx_data_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void hrx_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t hrx_attr_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void hrx_attr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void machine_start_mtx512();
	void machine_reset_mtx512();
	void ctc_tick(timer_device &timer, void *ptr, int32_t param);
	void cassette_tick(timer_device &timer, void *ptr, int32_t param);
	void ctc_trg1_w(int state);
	void ctc_trg2_w(int state);
	void mtx_tms9929a_interrupt(int state);
	uint8_t mtx_strobe_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t mtx_sound_strobe_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mtx_cst_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mtx_prt_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void write_centronics_busy(int state);
	void write_centronics_fault(int state);
	void write_centronics_perror(int state);
	void write_centronics_select(int state);
	void bankswitch(uint8_t data);
	DECLARE_SNAPSHOT_LOAD_MEMBER( mtx );
};

#endif /* __MTX_H__ */
