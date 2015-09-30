// license:???
// copyright-holders:Sean Young,Tomas Karlsson
/*****************************************************************************
 *
 * includes/svi318.h
 *
 * Spectravideo SVI-318 and SVI-328
 *
 ****************************************************************************/

#ifndef SVI318_H_
#define SVI318_H_

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/ins8250.h"
#include "machine/wd_fdc.h"
#include "machine/ram.h"
#include "machine/buffer.h"
#include "imagedev/cassette.h"
#include "sound/dac.h"
#include "sound/ay8910.h"
#include "sound/wave.h"
#include "video/mc6845.h"
#include "video/tms9928a.h"
#include "bus/centronics/ctronics.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"


class svi318_state : public driver_device
{
public:
	svi318_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_cassette(*this, "cassette"),
		m_dac(*this, "dac"),
		m_ppi(*this, "ppi8255"),
		m_ram(*this, RAM_TAG),
		m_centronics(*this, "centronics"),
		m_cent_data_out(*this, "cent_data_out"),
		m_ins8250_0(*this, "ins8250_0"),
		m_ins8250_1(*this, "ins8250_1"),
		m_cart(*this, "cartslot"),
		m_fd1793(*this, "wd179x"),
		m_floppy0(*this, "wd179x:0"),
		m_floppy1(*this, "wd179x:1"),
		m_crtc(*this, "crtc"),
		m_line(*this, "LINE"),
		m_joysticks(*this, "JOYSTICKS"),
		m_buttons(*this, "BUTTONS"),
		m_palette(*this, "palette"),
		m_floppy(NULL),
		m_bank1(*this, "bank1"),
		m_bank2(*this, "bank2"),
		m_bank3(*this, "bank3"),
		m_bank4(*this, "bank4")
	{ }

	DECLARE_FLOPPY_FORMATS(floppy_formats);

	// FDC
	int m_drq;
	int m_irq;

	DECLARE_WRITE8_MEMBER(ppi_w);
	DECLARE_READ8_MEMBER(psg_port_a_r);
	DECLARE_WRITE8_MEMBER(psg_port_b_w);
	DECLARE_WRITE8_MEMBER(fdc_drive_motor_w);
	DECLARE_WRITE8_MEMBER(fdc_density_side_w);
	DECLARE_READ8_MEMBER(fdc_irqdrq_r);
	DECLARE_WRITE8_MEMBER(svi806_ram_enable_w);
	DECLARE_WRITE8_MEMBER(writemem1);
	DECLARE_WRITE8_MEMBER(writemem2);
	DECLARE_WRITE8_MEMBER(writemem3);
	DECLARE_WRITE8_MEMBER(writemem4);
	DECLARE_READ8_MEMBER(io_ext_r);
	DECLARE_WRITE8_MEMBER(io_ext_w);
	DECLARE_DRIVER_INIT(svi318);
	DECLARE_DRIVER_INIT(svi328_806);
	DECLARE_WRITE_LINE_MEMBER(vdp_interrupt);
	DECLARE_WRITE_LINE_MEMBER(ins8250_interrupt);
	DECLARE_READ8_MEMBER(ppi_port_a_r);
	DECLARE_READ8_MEMBER(ppi_port_b_r);
	DECLARE_WRITE8_MEMBER(ppi_port_c_w);
	DECLARE_WRITE_LINE_MEMBER(fdc_intrq_w);
	DECLARE_WRITE_LINE_MEMBER(fdc_drq_w);
	bool cart_verify(UINT8 *ROM);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(svi318_cart);
	DECLARE_WRITE_LINE_MEMBER(write_centronics_busy);

	virtual void machine_start();
	virtual void machine_reset();

	MC6845_UPDATE_ROW(crtc_update_row);
	memory_region *m_cart_rom;
	memory_region *m_bios_rom;

protected:
	required_device<z80_device> m_maincpu;
	required_device<cassette_image_device> m_cassette;
	required_device<dac_device> m_dac;
	required_device<i8255_device> m_ppi;
	required_device<ram_device> m_ram;
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_cent_data_out;
	required_device<ins8250_device> m_ins8250_0;
	required_device<ins8250_device> m_ins8250_1;
	required_device<generic_slot_device> m_cart;
	required_device<fd1793_t> m_fd1793;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	optional_device<mc6845_device> m_crtc;
	required_ioport_array<11> m_line;
	required_ioport m_joysticks;
	required_ioport m_buttons;
	optional_device<palette_device> m_palette;

private:

	void set_banks();
	void postload();

	// memory banking
	UINT8   m_bank_switch;
	UINT8   m_bank_low;
	UINT8   m_bank_high;

	UINT8   m_bank_low_read_only;
	UINT8   m_bank_high1_read_only;
	UINT8   m_bank_high2_read_only;

	UINT8   *m_empty_bank;
	UINT8   *m_bank_low_ptr;
	UINT8   *m_bank_high1_ptr;
	UINT8   *m_bank_high2_ptr;

	// keyboard
	UINT8   m_keyboard_row;

	floppy_image_device *m_floppy;

	// centronics
	int m_centronics_busy;

	// SVI-806 80 column card
	UINT8   m_svi806_present;
	UINT8   m_svi806_ram_enabled;
	dynamic_buffer m_svi806_ram;
	UINT8   *m_svi806_gfx;

	required_memory_bank m_bank1;
	required_memory_bank m_bank2;
	required_memory_bank m_bank3;
	optional_memory_bank m_bank4;
};

#endif /* SVI318_H_ */
