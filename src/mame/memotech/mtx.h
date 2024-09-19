// license:BSD-3-Clause
// copyright-holders:Lee Ward, Dirk Best, Curt Coder
/*************************************************************************

    Memotech MTX 500, MTX 512 and RS 128

*************************************************************************/

#ifndef MAME_MEMOTECH_MTX_H
#define MAME_MEMOTECH_MTX_H

#include "imagedev/snapquik.h"
#include "imagedev/cassette.h"
#include "bus/centronics/ctronics.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "bus/mtx/exp.h"
#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "machine/z80sio.h"
#include "machine/z80ctc.h"
#include "sound/sn76496.h"
#include "machine/ram.h"
#include "machine/timer.h"


class mtx_state : public driver_device
{
public:
	mtx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_cassold(0)
		, m_maincpu(*this, "z80")
		, m_sn(*this, "sn76489a")
		, m_z80ctc(*this, "z80ctc")
		, m_z80dart(*this, "z80dart")
		, m_cassette(*this, "cassette")
		, m_centronics(*this, "centronics")
		, m_keyboard(*this, "ROW%u", 0U)
		, m_joystick(*this, "JOY%u", 0U)
		, m_joysticks(*this, "JOYSTICKS")
		, m_reset(*this, "RESET")
		, m_country(*this, "SWA")
		, m_ram(*this, RAM_TAG)
		, m_exp_int(*this, "exp_int")
		, m_exp_ext(*this, "exp_ext")
		, m_extrom(*this, "extrom")
		, m_rammap_bank1(*this, "rammap_bank1")
		, m_rammap_bank2(*this, "rammap_bank2")
		, m_rammap_bank3(*this, "rammap_bank3")
		, m_rommap_bank1(*this, "rommap_bank1")
		, m_rommap_bank2(*this, "rommap_bank2")
		, m_rommap_bank3(*this, "rommap_bank3")
	{ }

	void rs128(machine_config &config);
	void mtx500(machine_config &config);
	void mtx512(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(trigger_reset);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	bool m_cassold;
	required_device<z80_device> m_maincpu;
	required_device<sn76489a_device> m_sn;
	required_device<z80ctc_device> m_z80ctc;
	optional_device<z80dart_device> m_z80dart;
	required_device<cassette_image_device> m_cassette;
	required_device<centronics_device> m_centronics;
	required_ioport_array<8> m_keyboard;
	required_ioport_array<8> m_joystick;
	required_ioport m_joysticks;
	required_ioport m_reset;
	required_ioport m_country;
	required_device<ram_device> m_ram;
	required_device<mtx_exp_slot_device> m_exp_int;
	required_device<mtx_exp_slot_device> m_exp_ext;
	required_device<generic_slot_device> m_extrom;
	memory_bank_creator m_rammap_bank1;
	memory_bank_creator m_rammap_bank2;
	memory_bank_creator m_rammap_bank3;
	memory_bank_creator m_rommap_bank1;
	memory_bank_creator m_rommap_bank2;
	memory_bank_creator m_rommap_bank3;

	/* keyboard state */
	uint8_t m_key_sense = 0;

	/* video state */
	uint8_t *m_video_ram = nullptr;
	uint8_t *m_attr_ram = nullptr;

	/* sound state */
	uint8_t m_sound_latch = 0;

	/* timers */
	device_t *m_cassette_timer = nullptr;

	int m_centronics_busy = 0;
	int m_centronics_fault = 0;
	int m_centronics_perror = 0;
	int m_centronics_select = 0;

	void mtx_subpage_w(uint8_t data);
	void mtx_bankswitch_w(uint8_t data);
	void mtx_sound_latch_w(uint8_t data);
	void mtx_sense_w(uint8_t data);
	uint8_t mtx_key_lo_r();
	uint8_t mtx_key_hi_r();
	void hrx_address_w(offs_t offset, uint8_t data);
	uint8_t hrx_data_r();
	void hrx_data_w(uint8_t data);
	uint8_t hrx_attr_r();
	void hrx_attr_w(uint8_t data);
	TIMER_DEVICE_CALLBACK_MEMBER(ctc_tick);
	TIMER_DEVICE_CALLBACK_MEMBER(cassette_tick);
	void ctc_trg1_w(int state);
	void ctc_trg2_w(int state);
	uint8_t mtx_strobe_r();
	uint8_t mtx_sound_strobe_r();
	void mtx_cst_w(uint8_t data);
	void mtx_cst_motor_w(uint8_t data);
	uint8_t mtx_prt_r();
	void write_centronics_busy(int state);
	void write_centronics_fault(int state);
	void write_centronics_perror(int state);
	void write_centronics_select(int state);
	void bankswitch(uint8_t data);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(extrom_load);
	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);
	DECLARE_SNAPSHOT_LOAD_MEMBER(snapshot_cb);

	void mtx_io(address_map &map) ATTR_COLD;
	void mtx_mem(address_map &map) ATTR_COLD;
	void rs128_io(address_map &map) ATTR_COLD;
};

#endif // MAME_MEMOTECH_MTX_H
