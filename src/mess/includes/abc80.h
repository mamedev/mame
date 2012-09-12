/*****************************************************************************
 *
 * includes/abc80.h
 *
 ****************************************************************************/

#ifndef __ABC80__
#define __ABC80__


#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "imagedev/flopdrv.h"
#include "imagedev/printer.h"
#include "imagedev/cassette.h"
#include "machine/abcbus.h"
#include "machine/abc80kb.h"
#include "machine/abc830.h"
#include "machine/ram.h"
#include "machine/serial.h"
#include "machine/z80pio.h"
#include "sound/sn76477.h"

#define ABC80_HTOTAL	384
#define ABC80_HBEND		35
#define ABC80_HBSTART	384
#define ABC80_VTOTAL	312
#define ABC80_VBEND		15
#define ABC80_VBSTART	312

#define ABC80_K5_HSYNC			0x01
#define ABC80_K5_DH				0x02
#define ABC80_K5_LINE_END		0x04
#define ABC80_K5_ROW_START		0x08

#define ABC80_K2_VSYNC			0x01
#define ABC80_K2_DV				0x02
#define ABC80_K2_FRAME_END		0x04
#define ABC80_K2_FRAME_RESET	0x08

#define ABC80_J3_BLANK			0x01
#define ABC80_J3_TEXT			0x02
#define ABC80_J3_GRAPHICS		0x04
#define ABC80_J3_VERSAL			0x08

#define ABC80_E7_VIDEO_RAM		0x01
#define ABC80_E7_INT_RAM		0x02
#define ABC80_E7_31K_EXT_RAM	0x04
#define ABC80_E7_16K_INT_RAM	0x08

#define ABC80_CHAR_CURSOR		0x80

#define SCREEN_TAG			"screen"
#define Z80_TAG				"ab67"
#define Z80PIO_TAG			"cd67"
#define SN76477_TAG			"g8"
#define RS232_TAG			"ser"
#define TIMER_CASSETTE_TAG	"cass"

class abc80_state : public driver_device
{
public:
	abc80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, Z80_TAG),
		  m_pio(*this, Z80PIO_TAG),
		  m_psg(*this, SN76477_TAG),
		  m_cassette(*this, CASSETTE_TAG),
		  m_bus(*this, ABCBUS_TAG),
		  m_kb(*this, ABC80_KEYBOARD_TAG),
		  m_ram(*this, RAM_TAG),
		  m_rs232(*this, RS232_TAG),
		  m_tape_in(1),
		  m_tape_in_latch(1)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<z80pio_device> m_pio;
	required_device<device_t> m_psg;
	required_device<cassette_image_device> m_cassette;
	required_device<abcbus_slot_device> m_bus;
	required_device<abc80_keyboard_device> m_kb;
	required_device<ram_device> m_ram;
	required_device<rs232_port_device> m_rs232;

	enum
	{
		TIMER_ID_PIO,
		TIMER_ID_CASSETTE,
		TIMER_ID_BLINK,
		TIMER_ID_VSYNC_ON,
		TIMER_ID_VSYNC_OFF,
		TIMER_ID_FAKE_KEYBOARD_SCAN,
		TIMER_ID_FAKE_KEYBOARD_CLEAR
	};

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	virtual void machine_start();

	virtual void video_start();
	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void update_screen(bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void scan_keyboard();

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_WRITE_LINE_MEMBER( vco_voltage_w );

	DECLARE_READ8_MEMBER( pio_pa_r );
	DECLARE_READ8_MEMBER( pio_pb_r );
	DECLARE_WRITE8_MEMBER( pio_pb_w );

	DECLARE_WRITE_LINE_MEMBER( keydown_w );

	// keyboard state
	int m_key_data;
	int m_key_strobe;
	int m_pio_astb;

	// video state
	UINT8 *m_video_ram;
	UINT8 m_latch;
	int m_blink;

	// cassette state
	int m_tape_in;
	int m_tape_in_latch;

	// memory regions
	const UINT8 *m_mmu_rom;			// memory mapping ROM
	const UINT8 *m_char_rom;		// character generator ROM
	const UINT8 *m_hsync_prom;		// horizontal sync PROM
	const UINT8 *m_vsync_prom;		// horizontal sync PROM
	const UINT8 *m_line_prom;		// line address PROM
	const UINT8 *m_attr_prom;		// character attribute PROM

	// timers
	emu_timer *m_pio_timer;
	emu_timer *m_cassette_timer;
	emu_timer *m_blink_timer;
	emu_timer *m_vsync_on_timer;
	emu_timer *m_vsync_off_timer;
	emu_timer *m_kb_timer;
};

//----------- defined in video/abc80.c -----------

MACHINE_CONFIG_EXTERN( abc80_video );

#endif
