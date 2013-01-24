/*****************************************************************************
 *
 * includes/svi318.h
 *
 * Spectravideo SVI-318 and SVI-328
 *
 ****************************************************************************/

#ifndef SVI318_H_
#define SVI318_H_

#include "machine/i8255.h"
#include "machine/ins8250.h"
#include "machine/wd17xx.h"
#include "imagedev/cassette.h"
#include "sound/dac.h"
#include "machine/ram.h"
#include "machine/ctronics.h"


struct SVI_318
{
	/* general */
	UINT8   svi318;     /* Are we dealing with an SVI-318 or a SVI-328 model. 0 = 328, 1 = 318 */
	/* memory */
	UINT8   *empty_bank;
	UINT8   bank_switch;
	UINT8   bankLow;
	UINT8   bankHigh1;
	UINT8   *bankLow_ptr;
	UINT8   bankLow_read_only;
	UINT8   *bankHigh1_ptr;
	UINT8   bankHigh1_read_only;
	UINT8   *bankHigh2_ptr;
	UINT8   bankHigh2_read_only;
	/* keyboard */
	UINT8   keyboard_row;
	/* SVI-806 80 column card */
	UINT8   svi806_present;
	UINT8   svi806_ram_enabled;
	memory_region   *svi806_ram;
	UINT8   *svi806_gfx;
};

struct SVI318_FDC_STRUCT
{
	UINT8 driveselect;
	int drq;
	int irq;
	UINT8 heads[2];
};


class svi318_state : public driver_device
{
public:
	svi318_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cassette(*this, CASSETTE_TAG)
		, m_dac(*this, "dac")
		, m_ppi(*this, "ppi8255")
		, m_ram(*this, RAM_TAG)
		, m_centronics(*this, "centronics")
		, m_ins8250_0(*this, "ins8250_0")
		, m_ins8250_1(*this, "ins8250_1")
		, m_line0(*this, "LINE0")
		, m_line1(*this, "LINE1")
		, m_line2(*this, "LINE2")
		, m_line3(*this, "LINE3")
		, m_line4(*this, "LINE4")
		, m_line5(*this, "LINE5")
		, m_line6(*this, "LINE6")
		, m_line7(*this, "LINE7")
		, m_line8(*this, "LINE8")
		, m_line9(*this, "LINE9")
		, m_line10(*this, "LINE10")
		, m_joysticks(*this, "JOYSTICKS")
		, m_buttons(*this, "BUTTONS")
	{ }

	SVI_318 m_svi;
	UINT8 *m_pcart;
	UINT32 m_pcart_rom_size;
	SVI318_FDC_STRUCT m_fdc;
	DECLARE_WRITE8_MEMBER(svi318_ppi_w);
	DECLARE_READ8_MEMBER(svi318_psg_port_a_r);
	DECLARE_WRITE8_MEMBER(svi318_psg_port_b_w);
	DECLARE_WRITE8_MEMBER(svi318_fdc_drive_motor_w);
	DECLARE_WRITE8_MEMBER(svi318_fdc_density_side_w);
	DECLARE_READ8_MEMBER(svi318_fdc_irqdrq_r);
	DECLARE_WRITE8_MEMBER(svi806_ram_enable_w);
	DECLARE_WRITE8_MEMBER(svi318_writemem1);
	DECLARE_WRITE8_MEMBER(svi318_writemem2);
	DECLARE_WRITE8_MEMBER(svi318_writemem3);
	DECLARE_WRITE8_MEMBER(svi318_writemem4);
	DECLARE_READ8_MEMBER(svi318_io_ext_r);
	DECLARE_WRITE8_MEMBER(svi318_io_ext_w);
	DECLARE_DRIVER_INIT(svi318);
	DECLARE_MACHINE_START(svi318_pal);
	DECLARE_MACHINE_RESET(svi318);
	DECLARE_MACHINE_RESET(svi328_806);
	DECLARE_VIDEO_START(svi328_806);
	DECLARE_MACHINE_START(svi318_ntsc);
	DECLARE_WRITE_LINE_MEMBER(vdp_interrupt);
	DECLARE_WRITE_LINE_MEMBER(svi318_ins8250_interrupt);
	DECLARE_READ8_MEMBER(svi318_ppi_port_a_r);
	DECLARE_READ8_MEMBER(svi318_ppi_port_b_r);
	DECLARE_WRITE8_MEMBER(svi318_ppi_port_c_w);
	DECLARE_WRITE_LINE_MEMBER(svi_fdc_intrq_w);
	DECLARE_WRITE_LINE_MEMBER(svi_fdc_drq_w);

protected:
	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cassette;
	required_device<dac_device> m_dac;
	required_device<i8255_device> m_ppi;
	required_device<ram_device> m_ram;
	required_device<centronics_device> m_centronics;
	required_device<ins8250_device> m_ins8250_0;
	required_device<ins8250_device> m_ins8250_1;
	required_ioport m_line0;
	required_ioport m_line1;
	required_ioport m_line2;
	required_ioport m_line3;
	required_ioport m_line4;
	required_ioport m_line5;
	required_ioport m_line6;
	required_ioport m_line7;
	required_ioport m_line8;
	required_ioport m_line9;
	required_ioport m_line10;
	required_ioport m_joysticks;
	required_ioport m_buttons;

	void svi318_set_banks();
};


/*----------- defined in machine/svi318.c -----------*/

extern const i8255_interface svi318_ppi8255_interface;
extern const ins8250_interface svi318_ins8250_interface[2];
extern const wd17xx_interface svi_wd17xx_interface;

DEVICE_START( svi318_cart );
DEVICE_IMAGE_LOAD( svi318_cart );
DEVICE_IMAGE_UNLOAD( svi318_cart );


int svi318_cassette_present(running_machine &machine, int id);

MC6845_UPDATE_ROW( svi806_crtc6845_update_row );


#endif /* SVI318_H_ */
