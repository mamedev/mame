// license:GPL-2.0+
// copyright-holders:Kevin Thacker, Dirk Best, Phill Harvey-Smith
/***************************************************************************

    Tatung Einstein

***************************************************************************/

#ifndef EINSTEIN_H_
#define EINSTEIN_H_

#include "emu.h"
#include "video/mc6845.h"
#include "cpu/z80/z80daisy.h"
#include "machine/wd_fdc.h"
#include "machine/z80ctc.h"
#include "video/tms9928a.h"
#include "machine/ram.h"
#include "machine/i8251.h"
#include "bus/centronics/ctronics.h"

/***************************************************************************
    CONSTANTS
***************************************************************************/

/* xtals */
#define XTAL_X001  XTAL_10_738635MHz
#define XTAL_X002  XTAL_8MHz

/* integrated circuits */
#define IC_I001  "i001"  /* Z8400A */
#define IC_I030  "i030"  /* AY-3-8910 */
#define IC_I038  "i038"  /* TMM9129 */
#define IC_I042  "i042"  /* WD1770-PH */
#define IC_I050  "i050"  /* ADC0844CCN */
#define IC_I058  "i058"  /* Z8430A */
#define IC_I060  "i060"  /* uPD8251A */
#define IC_I063  "i063"  /* Z8420A */

/* interrupt sources */
#define EINSTEIN_KEY_INT   (1<<0)
#define EINSTEIN_ADC_INT   (1<<1)
#define EINSTEIN_FIRE_INT  (1<<2)

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class einstein_state : public driver_device
{
public:
	einstein_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_fdc(*this, IC_I042),
		m_color_screen(*this, "screen"),
		m_ctc(*this, IC_I058),
		m_tms9929a(*this, "tms9929a"),
		m_region_gfx1(*this, "gfx1"),
		m_mc6845(*this, "crtc"),
		m_crtc_screen(*this, "80column"),
		m_uart(*this, IC_I060),
		m_ram(*this, RAM_TAG),
		m_centronics(*this, "centronics"),
		m_region_bios(*this, "bios"),
		m_bank1(*this, "bank1"),
		m_bank2(*this, "bank2"),
		m_bank3(*this, "bank3"),
		m_line0(*this, "LINE0"),
		m_line1(*this, "LINE1"),
		m_line2(*this, "LINE2"),
		m_line3(*this, "LINE3"),
		m_line4(*this, "LINE4"),
		m_line5(*this, "LINE5"),
		m_line6(*this, "LINE6"),
		m_line7(*this, "LINE7"),
		m_extra(*this, "EXTRA"),
		m_buttons(*this, "BUTTONS"),
		m_config(*this, "config"),
		m_80column_dips(*this, "80column_dips"),
		m_palette(*this, "palette")  { }

	required_device<wd1770_t> m_fdc;
	required_device<screen_device> m_color_screen;
	required_device<z80ctc_device> m_ctc;
	required_device<tms9929a_device> m_tms9929a;
	optional_memory_region m_region_gfx1;

	int m_rom_enabled;
	int m_interrupt;
	int m_interrupt_mask;
	int m_ctc_trigger;

	/* keyboard */
	uint8_t m_keyboard_line;
	uint8_t m_keyboard_data;

	/* 80 column device */
	optional_device<mc6845_device> m_mc6845;
	optional_device<screen_device> m_crtc_screen;
	std::unique_ptr<uint8_t[]> m_crtc_ram;
	uint8_t   m_de;

	int m_centronics_busy;
	int m_centronics_perror;
	int m_centronics_fault;

	DECLARE_FLOPPY_FORMATS( floppy_formats );
	void einstein_80col_ram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t einstein_80col_ram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t einstein_80col_state_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void einstein_keyboard_line_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t einstein_keyboard_data_read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void einstein_rom_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t einstein_kybintmsk_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void einstein_kybintmsk_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void einstein_adcintmsk_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void einstein_fire_int_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void write_centronics_busy(int state);
	void write_centronics_perror(int state);
	void write_centronics_fault(int state);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	void machine_start_einstein2();
	void machine_reset_einstein2();
	uint32_t screen_update_einstein2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void einstein_keyboard_timer_callback(timer_device &timer, void *ptr, int32_t param);
	void einstein_ctc_trigger_callback(timer_device &timer, void *ptr, int32_t param);
	void einstein_6845_de_changed(int state);
	void einstein_drsel_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void einstein_serial_transmit_clock(int state);
	void einstein_serial_receive_clock(int state);
	MC6845_UPDATE_ROW(crtc_update_row);

protected:
	required_device<i8251_device> m_uart;
	required_device<ram_device> m_ram;
	required_device<centronics_device> m_centronics;
	required_memory_region m_region_bios;
	required_memory_bank m_bank1;
	required_memory_bank m_bank2;
	required_memory_bank m_bank3;
	required_ioport m_line0;
	required_ioport m_line1;
	required_ioport m_line2;
	required_ioport m_line3;
	required_ioport m_line4;
	required_ioport m_line5;
	required_ioport m_line6;
	required_ioport m_line7;
	required_ioport m_extra;
	required_ioport m_buttons;
	required_ioport m_config;
	optional_ioport m_80column_dips;
public:
	optional_device<palette_device> m_palette;

	void einstein_scan_keyboard();
	void einstein_page_rom();
};


// ======================> einstein_keyboard_daisy_device

class einstein_keyboard_daisy_device :  public device_t,
						public device_z80daisy_interface
{
public:
	// construction/destruction
	einstein_keyboard_daisy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

private:
	virtual void device_start() override;
	// z80daisy_interface overrides
	virtual int z80daisy_irq_state() override;
	virtual int z80daisy_irq_ack() override;
	virtual void z80daisy_irq_reti() override;
};

extern const device_type EINSTEIN_KEYBOARD_DAISY;


// ======================> einstein_adc_daisy_device

class einstein_adc_daisy_device :   public device_t,
						public device_z80daisy_interface
{
public:
	// construction/destruction
	einstein_adc_daisy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

private:
	virtual void device_start() override;
	// z80daisy_interface overrides
	virtual int z80daisy_irq_state() override;
	virtual int z80daisy_irq_ack() override;
	virtual void z80daisy_irq_reti() override;

};

extern const device_type EINSTEIN_ADC_DAISY;

// ======================> einstein_fire_daisy_device

class einstein_fire_daisy_device :  public device_t,
						public device_z80daisy_interface
{
public:
	// construction/destruction
	einstein_fire_daisy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

private:
	virtual void device_start() override;
	// z80daisy_interface overrides
	virtual int z80daisy_irq_state() override;
	virtual int z80daisy_irq_ack() override;
	virtual void z80daisy_irq_reti() override;
};

extern const device_type EINSTEIN_FIRE_DAISY;

#endif /* EINSTEIN_H_ */
