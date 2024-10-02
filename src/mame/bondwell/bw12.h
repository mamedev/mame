// license:BSD-3-Clause
// copyright-holders:Curt Coder

#ifndef MAME_BONDWELL_BW12_H
#define MAME_BONDWELL_BW12_H

#include "cpu/z80/z80.h"
#include "machine/ram.h"
#include "formats/bw12_dsk.h"
#include "imagedev/floppy.h"
#include "machine/74259.h"
#include "machine/6821pia.h"
#include "bus/centronics/ctronics.h"
#include "machine/kb3600.h"
#include "machine/pit8253.h"
#include "machine/ram.h"
#include "machine/rescap.h"
#include "machine/timer.h"
#include "machine/upd765.h"
#include "machine/z80sio.h"
#include "video/mc6845.h"
#include "emupal.h"

#define SCREEN_TAG          "screen"
#define Z80_TAG             "ic35"
#define MC6845_TAG          "ic14"
#define UPD765_TAG          "ic45"
#define Z80SIO_TAG          "ic15"
#define PIT8253_TAG         "ic34"
#define PIA6821_TAG         "ic16"
#define AY3600PRO002_TAG    "ic74"
#define CENTRONICS_TAG      "centronics"
#define FLOPPY_TIMER_TAG    "motor_off"
#define RS232_A_TAG         "rs232a"
#define RS232_B_TAG         "rs232b"

#define BW12_VIDEORAM_MASK  0x7ff
#define BW12_CHARROM_MASK   0xfff

class bw12_state : public driver_device
{
public:
	bw12_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, Z80_TAG)
		, m_latch(*this, "latch")
		, m_pia(*this, PIA6821_TAG)
		, m_sio(*this, Z80SIO_TAG)
		, m_fdc(*this, UPD765_TAG)
		, m_kbc(*this, AY3600PRO002_TAG)
		, m_crtc(*this, MC6845_TAG)
		, m_pit(*this, PIT8253_TAG)
		, m_palette(*this, "palette")
		, m_centronics(*this, CENTRONICS_TAG)
		, m_ram(*this, RAM_TAG)
		, m_floppy(*this, UPD765_TAG ":%u:525dd", 1U)
		, m_floppy_timer(*this, FLOPPY_TIMER_TAG)
		, m_rom(*this, Z80_TAG)
		, m_char_rom(*this, "chargen")
		, m_video_ram(*this, "video_ram")
		, m_modifiers(*this, "MODIFIERS")
		, m_bank(*this, "bank")
	{ }

	void bw14(machine_config &config);
	void bw12(machine_config &config);

private:
	void bankswitch();

	void ls138_a0_w(int state);
	void ls138_a1_w(int state);
	void init_w(int state);
	void motor0_w(int state);
	void motor1_w(int state);

	uint8_t ls259_r(offs_t offset);
	uint8_t pia_pa_r();
	void pia_cb2_w(int state);
	void pit_out2_w(int state);
	int ay3600_shift_r();
	int ay3600_control_r();
	void ay3600_data_ready_w(int state);
	MC6845_UPDATE_ROW( crtc_update_row );

	void floppy_motor_on_off();
	TIMER_DEVICE_CALLBACK_MEMBER(floppy_motor_off_tick);
	static void bw12_floppy_formats(format_registration &fr);
	static void bw14_floppy_formats(format_registration &fr);

	void write_centronics_busy(int state);
	void write_centronics_fault(int state);
	void write_centronics_perror(int state);

	void common(machine_config &config);
	void bw12_io(address_map &map) ATTR_COLD;
	void bw12_mem(address_map &map) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<ls259_device> m_latch;
	required_device<pia6821_device> m_pia;
	required_device<z80sio_device> m_sio;
	required_device<upd765a_device> m_fdc;
	required_device<ay3600_device> m_kbc;
	required_device<mc6845_device> m_crtc;
	required_device<pit8253_device> m_pit;
	required_device<palette_device> m_palette;
	required_device<centronics_device> m_centronics;
	required_device<ram_device> m_ram;
	required_device_array<floppy_image_device, 2> m_floppy;
	required_device<timer_device> m_floppy_timer;
	required_memory_region m_rom;
	required_memory_region m_char_rom;
	required_shared_ptr<uint8_t> m_video_ram;
	required_ioport m_modifiers;
	required_memory_bank m_bank;

	/* memory state */
	int m_curbank = 0;

	/* PIT state */
	int m_pit_out2 = 0;

	/* keyboard state */
	int m_key_data[9]{};
	int m_key_sin = 0;
	int m_key_stb = 0;
	int m_key_shift = 0;

	/* floppy state */
	int m_motor_on = 0;
	int m_motor0 = 0;
	int m_motor1 = 0;

	int m_centronics_busy = 0;
	int m_centronics_fault = 0;
	int m_centronics_perror = 0;
};

#endif // MAME_BONDWELL_BW12_H
