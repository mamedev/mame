// license:BSD-3-Clause
// copyright-holders:Nathan Woods
#ifndef MAME_ACORN_ATOM_H
#define MAME_ACORN_ATOM_H

#pragma once


#include "cpu/m6502/m6502.h"
#include "imagedev/cassette.h"
#include "imagedev/floppy.h"
#include "imagedev/snapquik.h"
#include "machine/6522via.h"
#include "machine/i8255.h"
#include "machine/i8271.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "sound/spkrdev.h"
#include "video/mc6847.h"

#include "bus/centronics/ctronics.h"
#include "bus/generic/carts.h"
#include "bus/generic/slot.h"

#include "formats/atom_dsk.h"
#include "formats/atom_tap.h"
#include "formats/uef_cas.h"


#define SY6502_TAG      "ic22"
#define INS8255_TAG     "ic25"
#define MC6847_TAG      "ic31"
#define R6522_TAG       "ic1"
#define I8271_TAG       "ic13"
#define MC6854_TAG      "econet_ic1"
#define CENTRONICS_TAG  "centronics"
#define BASERAM_TAG     "baseram"


class atom_state : public driver_device
{
public:
	atom_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, SY6502_TAG)
		, m_vdg(*this, MC6847_TAG)
		, m_cassette(*this, "cassette")
		, m_fdc(*this, I8271_TAG)
		, m_floppies(*this, I8271_TAG ":%u", 0U)
		, m_centronics(*this, CENTRONICS_TAG)
		, m_speaker(*this, "speaker")
		, m_cart(*this, "cartslot")
		, m_vram(*this, "videoram")
		, m_io_keyboard(*this, "Y%u", 0U)
		, m_ppi(*this, INS8255_TAG)
		, m_via(*this, R6522_TAG)
	{ }

	void atombb(machine_config &config);
	void atom(machine_config &config);
	void atom_common(machine_config &config);
	DECLARE_INPUT_CHANGED_MEMBER( trigger_reset );

protected:
	required_device<cpu_device> m_maincpu;
	required_device<mc6847_base_device> m_vdg;
	required_device<cassette_image_device> m_cassette;
	optional_device<i8271_device> m_fdc;
	optional_device_array<floppy_connector, 2> m_floppies;
	required_device<centronics_device> m_centronics;
	required_device<speaker_sound_device> m_speaker;
	optional_device<generic_slot_device> m_cart;
	required_shared_ptr<uint8_t> m_vram;
	required_ioport_array<12> m_io_keyboard;
	required_device<i8255_device> m_ppi;
	required_device<via6522_device> m_via;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	void ppi_pa_w(uint8_t data);
	uint8_t ppi_pb_r();
	uint8_t ppi_pc_r();
	void ppi_pc_w(uint8_t data);
	uint8_t vdg_videoram_r(offs_t offset);
	void atom_8271_interrupt_callback(int state);
	void motor_w(int state);

	/* keyboard state */
	u8 m_keylatch = 0U;

	/* cassette state */
	bool m_hz2400 = 0;
	bool m_pc0 = 0;
	bool m_pc1 = 0;

	/* devices */
	bool m_previous_i8271_int_state = false;
	static void floppy_formats(format_registration &fr);
	void cassette_output_tick(int state);

	std::pair<std::error_condition, std::string> load_cart(device_image_interface &image, generic_slot_device &slot);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load) { return load_cart(image, *m_cart); }
	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);
	void atom_mem(address_map &map);
	void atombb_mem(address_map &map);
	void prophet_mem(address_map &map);
};

class atomeb_state : public atom_state
{
public:
	atomeb_state(const machine_config &mconfig, device_type type, const char *tag)
		: atom_state(mconfig, type, tag)
		, m_ext(*this, "rom_a%x", 0)
		, m_e0(*this, "rom_e0")
		, m_e1(*this, "rom_e1")
	{
	}

	void atomeb(machine_config &config);

private:
	void machine_start() override;
	void machine_reset() override;

	uint8_t eprom_r();
	void eprom_w(uint8_t data);
	uint8_t ext_r(offs_t offset);
	uint8_t dos_r(offs_t offset);

	/* eprom state */
	u8 m_eprom = 0;

	required_device_array<generic_slot_device, 16> m_ext;
	required_device<generic_slot_device> m_e0;
	required_device<generic_slot_device> m_e1;

	template<int I> DECLARE_DEVICE_IMAGE_LOAD_MEMBER(ext_load) { return load_cart(image, *m_ext[I]); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(e0_load) { return load_cart(image, *m_e0); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(e1_load) { return load_cart(image, *m_e1); }
	void atomeb_mem(address_map &map);
};

#endif // MAME_ACORN_ATOM_H
