/*****************************************************************************
 *
 * includes/oric.h
 *
 ****************************************************************************/

#ifndef ORIC_H_
#define ORIC_H_

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "sound/ay8910.h"
#include "sound/wave.h"
#include "machine/6522via.h"
#include "machine/6551acia.h"
#include "machine/ctronics.h"
#include "machine/wd17xx.h"
//#include <stdio.h>
#include "machine/applefdc.h"
#include "imagedev/flopdrv.h"
#include "imagedev/cassette.h"
#include "formats/oric_dsk.h"
#include "formats/ap2_dsk.h"
#include "formats/oric_tap.h"

enum
{
	TELESTRAT_MEM_BLOCK_UNDEFINED,
	TELESTRAT_MEM_BLOCK_RAM,
	TELESTRAT_MEM_BLOCK_ROM
};

struct telestrat_mem_block
{
	int     MemType;
	unsigned char *ptr;
};


/* current state of the display */
/* some attributes persist until they are turned off.
This structure holds this persistant information */
struct oric_vh_state
{
	/* foreground and background colour used for rendering */
	/* if flash attribute is set, these two will both be equal to background colour */
	UINT8 active_foreground_colour;
	UINT8 active_background_colour;
	/* current foreground and background colour */
	UINT8 foreground_colour;
	UINT8 background_colour;
	UINT8 mode;
	/* text attributes */
	UINT8 text_attributes;

	offs_t read_addr;

	/* current addr to fetch data */
	UINT8 *char_data;
	/* base of char data */
	UINT8 *char_base;

	/* if (1<<3), display graphics, if 0, hide graphics */
	/* current count */
	UINT8 flash_count;
};


class oric_state : public driver_device
{
public:
	oric_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_ram(*this, "ram")
		, m_maincpu(*this, "maincpu")
		, m_ay8912(*this, "ay8912")
		, m_centronics(*this, "centronics")
		, m_cassette(*this, CASSETTE_TAG)
		, m_via6522_0(*this, "via6522_0")
		, m_region_maincpu(*this, "maincpu")
		, m_bank1(*this, "bank1")
		, m_bank2(*this, "bank2")
		, m_bank3(*this, "bank3")
		, m_bank4(NULL)
		, m_bank5(*this, "bank5")
		, m_bank6(*this, "bank6")
		, m_bank7(*this, "bank7")
		, m_io_row0(*this, "ROW0")
		, m_io_row1(*this, "ROW1")
		, m_io_row2(*this, "ROW2")
		, m_io_row3(*this, "ROW3")
		, m_io_row4(*this, "ROW4")
		, m_io_row5(*this, "ROW5")
		, m_io_row6(*this, "ROW6")
		, m_io_row7(*this, "ROW7")
		, m_io_floppy(*this, "FLOPPY")
	{ }

	optional_shared_ptr<UINT8> m_ram;
	bool m_is_telestrat;
	UINT8 m_irqs;
	UINT8 *m_ram_0x0c000;
	UINT8 m_keyboard_line;
	UINT8 m_key_sense_bit;
	UINT8 m_keyboard_mask;
	UINT8 m_via_port_a_data;
	UINT8 m_psg_control;
	UINT8 m_previous_portb_data;
	UINT8 m_port_3fa_w;
	UINT8 m_port_3fb_w;
	UINT8 m_wd179x_int_state;
	UINT8 m_port_314_r;
	UINT8 m_port_318_r;
	UINT8 m_port_314_w;
	UINT8 m_telestrat_bank_selection;
	UINT8 m_telestrat_via2_port_a_data;
	UINT8 m_telestrat_via2_port_b_data;
	telestrat_mem_block m_telestrat_blocks[8];
	oric_vh_state m_vh_state;
	DECLARE_WRITE8_MEMBER(oric_psg_porta_write);
	DECLARE_WRITE8_MEMBER(apple2_v2_interface_w);
	DECLARE_READ8_MEMBER(oric_jasmin_r);
	DECLARE_WRITE8_MEMBER(oric_jasmin_w);
	DECLARE_READ8_MEMBER(oric_microdisc_r);
	DECLARE_WRITE8_MEMBER(oric_microdisc_w);
	DECLARE_READ8_MEMBER(oric_IO_r);
	DECLARE_WRITE8_MEMBER(oric_IO_w);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	virtual void palette_init();
	DECLARE_MACHINE_START(telestrat);
	UINT32 screen_update_oric(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(oric_refresh_tape);
	TIMER_CALLBACK_MEMBER(oric_vh_timer_callback);
	DECLARE_READ8_MEMBER(oric_via_in_a_func);
	DECLARE_READ8_MEMBER(oric_via_in_b_func);
	DECLARE_WRITE8_MEMBER(oric_via_out_a_func);
	DECLARE_WRITE8_MEMBER(oric_via_out_b_func);
	DECLARE_READ8_MEMBER(oric_via_in_ca2_func);
	DECLARE_READ8_MEMBER(oric_via_in_cb2_func);
	DECLARE_WRITE8_MEMBER(oric_via_out_ca2_func);
	DECLARE_WRITE8_MEMBER(oric_via_out_cb2_func);
	DECLARE_WRITE_LINE_MEMBER(oric_jasmin_wd179x_drq_w);
	DECLARE_WRITE_LINE_MEMBER(oric_microdisc_wd179x_intrq_w);
	DECLARE_WRITE_LINE_MEMBER(oric_microdisc_wd179x_drq_w);
	DECLARE_WRITE_LINE_MEMBER(oric_wd179x_intrq_w);
	DECLARE_WRITE_LINE_MEMBER(oric_wd179x_drq_w);
	DECLARE_WRITE_LINE_MEMBER(oric_via_irq_func);
	DECLARE_READ8_MEMBER(telestrat_via2_in_a_func);
	DECLARE_WRITE8_MEMBER(telestrat_via2_out_a_func);
	DECLARE_READ8_MEMBER(telestrat_via2_in_b_func);
	DECLARE_WRITE8_MEMBER(telestrat_via2_out_b_func);
	DECLARE_WRITE_LINE_MEMBER(telestrat_via2_irq_func);

protected:
	required_device<cpu_device> m_maincpu;
	required_device<device_t> m_ay8912;
	required_device<centronics_device> m_centronics;
	required_device<cassette_image_device> m_cassette;
	required_device<via6522_device> m_via6522_0;
	required_memory_region m_region_maincpu;
	required_memory_bank m_bank1;
	required_memory_bank m_bank2;
	optional_memory_bank m_bank3;
	memory_bank *m_bank4;
	optional_memory_bank m_bank5;
	optional_memory_bank m_bank6;
	optional_memory_bank m_bank7;
	required_ioport m_io_row0;
	required_ioport m_io_row1;
	required_ioport m_io_row2;
	required_ioport m_io_row3;
	required_ioport m_io_row4;
	required_ioport m_io_row5;
	required_ioport m_io_row6;
	required_ioport m_io_row7;
	required_ioport m_io_floppy;

	void oric_microdisc_refresh_wd179x_ints();
	void oric_refresh_ints();
	void oric_keyboard_sense_refresh();
	void oric_psg_connection_refresh(address_space &space);
	void oric_common_init_machine();
	void oric_install_apple2_interface();
	void oric_install_apple2_v2_interface();
	void oric_install_microdisc_interface();
	void oric_install_jasmin_interface();
	void oric_microdisc_set_mem_0x0c000();
	void telestrat_refresh_mem();
	void oric_enable_memory(int low, int high, int rd, int wr);
	void oric_jasmin_set_mem_0x0c000();

	void oric_vh_update_attribute(UINT8 c);
	void oric_vh_update_flash();
	void oric_refresh_charset();
};

/*----------- defined in machine/oric.c -----------*/
extern const via6522_interface oric_6522_interface;
extern const via6522_interface telestrat_via2_interface;
extern const wd17xx_interface oric_wd17xx_interface;

#endif /* ORIC_H_ */
