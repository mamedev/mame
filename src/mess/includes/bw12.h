#ifndef __BW12__
#define __BW12__

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/ram.h"
#include "formats/bw12_dsk.h"
#include "formats/hxcmfm_dsk.h"
#include "formats/imd_dsk.h"
#include "formats/mfi_dsk.h"
#include "machine/6821pia.h"
#include "machine/ctronics.h"
#include "machine/kb3600.h"
#include "machine/pit8253.h"
#include "machine/ram.h"
#include "machine/rescap.h"
#include "machine/upd765.h"
#include "machine/z80dart.h"
#include "video/mc6845.h"
#include "sound/dac.h"

#define SCREEN_TAG			"screen"
#define Z80_TAG				"ic35"
#define MC6845_TAG			"ic14"
#define UPD765_TAG			"ic45"
#define Z80SIO_TAG			"ic15"
#define PIT8253_TAG			"ic34"
#define PIA6821_TAG			"ic16"
#define MC1408_TAG			"ic4"
#define AY3600PRO002_TAG	"ic74"
#define CENTRONICS_TAG		"centronics"
#define FLOPPY_TIMER_TAG	"motor_off"

#define BW12_VIDEORAM_MASK	0x7ff
#define BW12_CHARROM_MASK	0xfff

class bw12_state : public driver_device
{
public:
	bw12_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, Z80_TAG),
		  m_pia(*this, PIA6821_TAG),
		  m_sio(*this, Z80SIO_TAG),
		  m_fdc(*this, UPD765_TAG),
		  m_kbc(*this, AY3600PRO002_TAG),
		  m_crtc(*this, MC6845_TAG),
		  m_centronics(*this, CENTRONICS_TAG),
		  m_ram(*this, RAM_TAG),
		  m_floppy0(*this, UPD765_TAG ":0:525dd"),
		  m_floppy1(*this, UPD765_TAG ":1:525dd"),
		  m_floppy_timer(*this, FLOPPY_TIMER_TAG),
		  m_video_ram(*this, "video_ram")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<pia6821_device> m_pia;
	required_device<z80dart_device> m_sio;
	required_device<upd765a_device> m_fdc;
	required_device<ay3600_device> m_kbc;
	required_device<mc6845_device> m_crtc;
	required_device<centronics_device> m_centronics;
	required_device<ram_device> m_ram;
	required_device<floppy_image_device> m_floppy0;
	required_device<floppy_image_device> m_floppy1;
	required_device<timer_device> m_floppy_timer;

	virtual void machine_start();
	virtual void machine_reset();

	virtual void video_start();

	void bankswitch();
	void floppy_motor_off();
	void set_floppy_motor_off_timer();
	void ls259_w(int address, int data);

	DECLARE_READ8_MEMBER( ls259_r );
	DECLARE_WRITE8_MEMBER( ls259_w );
	void fdc_intrq_w(bool state);
	DECLARE_READ8_MEMBER( pia_pa_r );
	DECLARE_READ_LINE_MEMBER( pia_cb1_r );
	DECLARE_WRITE_LINE_MEMBER( pia_cb2_w );
	DECLARE_WRITE_LINE_MEMBER( pit_out2_w );
	DECLARE_READ_LINE_MEMBER( ay3600_shift_r );
	DECLARE_READ_LINE_MEMBER( ay3600_control_r );
	DECLARE_WRITE_LINE_MEMBER( ay3600_data_ready_w );

	/* memory state */
	int m_bank;

	/* video state */
	required_shared_ptr<UINT8> m_video_ram;
	UINT8 *m_char_rom;

	/* PIT state */
	int m_pit_out2;

	/* keyboard state */
	int m_key_data[9];
	int m_key_sin;
	int m_key_stb;
	int m_key_shift;

	/* floppy state */
	int m_fdc_int;
	int m_motor_on;
	int m_motor0;
	int m_motor1;
	TIMER_DEVICE_CALLBACK_MEMBER(floppy_motor_off_tick);
	DECLARE_WRITE_LINE_MEMBER(pit_out0_w);
	DECLARE_FLOPPY_FORMATS( bw12_floppy_formats );
	DECLARE_FLOPPY_FORMATS( bw14_floppy_formats );
};

#endif
