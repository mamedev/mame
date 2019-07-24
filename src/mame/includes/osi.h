// license:BSD-3-Clause
// copyright-holders:Curt Coder, Robbbert, Wilbert Pol
#ifndef MAME_INCLUDES_OSI_H
#define MAME_INCLUDES_OSI_H

#pragma once


#include "cpu/m6502/m6502.h"
#include "formats/basicdsk.h"
#include "imagedev/cassette.h"
#include "imagedev/floppy.h"
#include "machine/6850acia.h"
#include "machine/6821pia.h"
#include "machine/timer.h"
#include "machine/ram.h"
#include "sound/discrete.h"
#include "sound/beep.h"
#include "emupal.h"

#define SCREEN_TAG      "screen"
#define M6502_TAG       "m6502"
#define DISCRETE_TAG    "discrete"

#define X1          3932160
#define UK101_X1    XTAL(8'000'000)

#define OSI600_VIDEORAM_SIZE    0x400
#define OSI630_COLORRAM_SIZE    0x400

class sb2m600_state : public driver_device
{
public:
	sb2m600_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_cassbit(0)
		, m_cassold(0)
		, m_maincpu(*this, M6502_TAG)
		, m_acia(*this, "acia")
		, m_cass(*this, "cassette")
		, m_discrete(*this, DISCRETE_TAG)
		, m_ram(*this, RAM_TAG)
		, m_video_ram(*this, "video_ram")
		, m_color_ram(*this, "color_ram")
		, m_p_chargen(*this, "chargen")
		, m_io_keyboard(*this, "ROW%u", 0)
		, m_io_sound(*this, "Sound")
		, m_io_reset(*this, "Reset")
	{ }

	void osi600(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_READ8_MEMBER( keyboard_r );
	DECLARE_WRITE8_MEMBER( keyboard_w );
	DECLARE_WRITE8_MEMBER( ctrl_w );
	TIMER_DEVICE_CALLBACK_MEMBER(kansas_w);
	TIMER_DEVICE_CALLBACK_MEMBER(kansas_r);

	void floppy_index_callback(floppy_image_device *floppy, int state);

	void osi630_palette(palette_device &palette) const;

	void osi600_video(machine_config &config);
	void osi630_video(machine_config &config);
	void osi600_mem(address_map &map);

	uint8_t m_cass_data[4];
	bool m_cassbit;
	bool m_cassold;
	required_device<cpu_device> m_maincpu;
	required_device<acia6850_device> m_acia;
	required_device<cassette_image_device> m_cass;
	optional_device<discrete_sound_device> m_discrete;
	required_device<ram_device> m_ram;
	required_shared_ptr<uint8_t> m_video_ram;
	optional_shared_ptr<uint8_t> m_color_ram;
	required_region_ptr<u8> m_p_chargen;
	required_ioport_array<8> m_io_keyboard;
	required_ioport m_io_sound;
	required_ioport m_io_reset;

	/* floppy state */
	int m_fdc_index;

	/* keyboard state */
	uint8_t m_keylatch;

	/* video state */
	int m_32;
	int m_coloren;
};

class c1p_state : public sb2m600_state
{
public:
	c1p_state(const machine_config &mconfig, device_type type, const char *tag)
		: sb2m600_state(mconfig, type, tag)
		, m_beeper(*this, "beeper")
	{ }

	void init_c1p();
	void c1p(machine_config &config);

protected:
	enum
	{
		TIMER_SETUP_BEEP
	};

	virtual void machine_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	DECLARE_WRITE8_MEMBER( osi630_ctrl_w );
	DECLARE_WRITE8_MEMBER( osi630_sound_w );
	void c1p_mem(address_map &map);

	required_device<beep_device> m_beeper;
};

class c1pmf_state : public c1p_state
{
public:
	c1pmf_state(const machine_config &mconfig, device_type type, const char *tag)
		: c1p_state(mconfig, type, tag)
		, m_floppy0(*this, "floppy0")
		, m_floppy1(*this, "floppy1")
	{ }

	void c1pmf(machine_config &config);

protected:
	virtual void machine_start() override;

	DECLARE_READ8_MEMBER( osi470_pia_pa_r );
	DECLARE_WRITE8_MEMBER( osi470_pia_pa_w );
	DECLARE_WRITE8_MEMBER( osi470_pia_pb_w );
	DECLARE_WRITE_LINE_MEMBER( osi470_pia_cb2_w );

	void c1pmf_mem(address_map &map);

private:
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
};

class uk101_state : public sb2m600_state
{
public:
	uk101_state(const machine_config &mconfig, device_type type, const char *tag)
		: sb2m600_state(mconfig, type, tag)
	{ }

	void uk101(machine_config &config);

protected:
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE8_MEMBER( keyboard_w );
	void uk101_video(machine_config &config);
	void uk101_mem(address_map &map);
};

#endif // MAME_INCLUDES_OSI_H
