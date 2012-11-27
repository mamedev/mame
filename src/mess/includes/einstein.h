/*****************************************************************************
 *
 * includes/einstein.h
 *
 ****************************************************************************/

#ifndef EINSTEIN_H_
#define EINSTEIN_H_

#include "video/mc6845.h"
#include "cpu/z80/z80daisy.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"
#include "machine/z80ctc.h"

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
		  m_fdc(*this, IC_I042)
		  { }

	required_device<wd1770_t> m_fdc;
	device_t *m_color_screen;
	z80ctc_device *m_ctc;

	int m_rom_enabled;
	int m_interrupt;
	int m_interrupt_mask;
	int m_ctc_trigger;

	/* keyboard */
	UINT8 m_keyboard_line;
	UINT8 m_keyboard_data;

	/* 80 column device */
	mc6845_device *m_mc6845;
	screen_device *m_crtc_screen;
	UINT8 *m_crtc_ram;
	UINT8	m_de;

	DECLARE_FLOPPY_FORMATS( floppy_formats );
	DECLARE_WRITE8_MEMBER(einstein_80col_ram_w);
	DECLARE_READ8_MEMBER(einstein_80col_ram_r);
	DECLARE_READ8_MEMBER(einstein_80col_state_r);
	DECLARE_WRITE8_MEMBER(einstein_keyboard_line_write);
	DECLARE_READ8_MEMBER(einstein_keyboard_data_read);
	DECLARE_WRITE8_MEMBER(einstein_rom_w);
	DECLARE_READ8_MEMBER(einstein_kybintmsk_r);
	DECLARE_WRITE8_MEMBER(einstein_kybintmsk_w);
	DECLARE_WRITE8_MEMBER(einstein_adcintmsk_w);
	DECLARE_WRITE8_MEMBER(einstein_fire_int_w);
	virtual void machine_start();
	virtual void machine_reset();
	DECLARE_MACHINE_START(einstein2);
	DECLARE_MACHINE_RESET(einstein2);
	UINT32 screen_update_einstein2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(einstein_keyboard_timer_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(einstein_ctc_trigger_callback);
	DECLARE_WRITE_LINE_MEMBER(einstein_6845_de_changed);
	DECLARE_WRITE8_MEMBER(einstein_drsel_w);
	DECLARE_WRITE_LINE_MEMBER(einstein_serial_transmit_clock);
	DECLARE_WRITE_LINE_MEMBER(einstein_serial_receive_clock);
};


// ======================> einstein_keyboard_daisy_device

class einstein_keyboard_daisy_device :	public device_t,
						public device_z80daisy_interface
{
public:
	// construction/destruction
	einstein_keyboard_daisy_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

private:
	virtual void device_start();
	// z80daisy_interface overrides
	virtual int z80daisy_irq_state();
	virtual int z80daisy_irq_ack();
	virtual void z80daisy_irq_reti();
};

extern const device_type EINSTEIN_KEYBOARD_DAISY;


// ======================> einstein_adc_daisy_device

class einstein_adc_daisy_device :	public device_t,
						public device_z80daisy_interface
{
public:
	// construction/destruction
	einstein_adc_daisy_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

private:
	virtual void device_start();
	// z80daisy_interface overrides
	virtual int z80daisy_irq_state();
	virtual int z80daisy_irq_ack();
	virtual void z80daisy_irq_reti();

};

extern const device_type EINSTEIN_ADC_DAISY;

// ======================> einstein_fire_daisy_device

class einstein_fire_daisy_device :	public device_t,
						public device_z80daisy_interface
{
public:
	// construction/destruction
	einstein_fire_daisy_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

private:
	virtual void device_start();
	// z80daisy_interface overrides
	virtual int z80daisy_irq_state();
	virtual int z80daisy_irq_ack();
	virtual void z80daisy_irq_reti();
};

extern const device_type EINSTEIN_FIRE_DAISY;

#endif /* EINSTEIN_H_ */
