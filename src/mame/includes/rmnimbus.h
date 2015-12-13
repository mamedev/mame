// license:BSD-3-Clause
// copyright-holders:Phill Harvey-Smith, Carl
/*
    rmnimbus.c
    Machine driver for the Research Machines Nimbus.

    Phill Harvey-Smith
    2009-11-29.
*/
#include "emu.h"
#include "cpu/i86/i186.h"
#include "machine/z80dart.h"
#include "machine/wd_fdc.h"
#include "bus/scsi/scsi.h"
#include "machine/6522via.h"
#include "machine/ram.h"
#include "machine/eepromser.h"
#include "sound/ay8910.h"
#include "sound/msm5205.h"
#include "bus/centronics/ctronics.h"

#define MAINCPU_TAG "maincpu"
#define IOCPU_TAG   "iocpu"
#define Z80SIO_TAG  "z80sio"
#define FDC_TAG     "wd2793"
#define SCSIBUS_TAG "scsibus"
#define ER59256_TAG "er59256"
#define AY8910_TAG  "ay8910"
#define MONO_TAG    "mono"
#define MSM5205_TAG "msm5205"
#define VIA_TAG     "via6522"
#define CENTRONICS_TAG "centronics"

/* Mouse / Joystick */

#define JOYSTICK0_TAG           "joystick0"
#define MOUSE_BUTTON_TAG        "mousebtn"
#define MOUSEX_TAG              "mousex"
#define MOUSEY_TAG              "mousey"

/* Memory controller */
#define RAM_BANK00_TAG  "bank0"
#define RAM_BANK01_TAG  "bank1"
#define RAM_BANK02_TAG  "bank2"
#define RAM_BANK03_TAG  "bank3"
#define RAM_BANK04_TAG  "bank4"
#define RAM_BANK05_TAG  "bank5"
#define RAM_BANK06_TAG  "bank6"
#define RAM_BANK07_TAG  "bank7"

class rmnimbus_state : public driver_device
{
public:
	rmnimbus_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_msm(*this, MSM5205_TAG),
		m_scsibus(*this, SCSIBUS_TAG),
		m_ram(*this, RAM_TAG),
		m_eeprom(*this, ER59256_TAG),
		m_via(*this, VIA_TAG),
		m_centronics(*this, CENTRONICS_TAG),
		m_palette(*this, "palette"),
		m_scsi_data_out(*this, "scsi_data_out"),
		m_scsi_data_in(*this, "scsi_data_in"),
		m_scsi_ctrl_out(*this, "scsi_ctrl_out"),
		m_fdc(*this, FDC_TAG),
		m_z80sio(*this, Z80SIO_TAG),
		m_screen(*this, "screen"),
		m_io_config(*this, "config"),
		m_io_joystick0(*this, JOYSTICK0_TAG),
		m_io_mouse_button(*this, MOUSE_BUTTON_TAG),
		m_io_mousex(*this, MOUSEX_TAG),
		m_io_mousey(*this, MOUSEY_TAG)
	{
	}

	required_device<i80186_cpu_device> m_maincpu;
	required_device<msm5205_device> m_msm;
	required_device<SCSI_PORT_DEVICE> m_scsibus;
	required_device<ram_device> m_ram;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<via6522_device> m_via;
	required_device<centronics_device> m_centronics;
	required_device<palette_device> m_palette;
	required_device<output_latch_device> m_scsi_data_out;
	required_device<input_buffer_device> m_scsi_data_in;
	required_device<output_latch_device> m_scsi_ctrl_out;
	required_device<wd2793_t> m_fdc;
	required_device<z80sio2_device> m_z80sio;
	required_device<screen_device> m_screen;
	required_ioport m_io_config;
	required_ioport m_io_joystick0;
	required_ioport m_io_mouse_button;
	required_ioport m_io_mousex;
	required_ioport m_io_mousey;

	bitmap_ind16 m_video_mem;

	UINT32 m_debug_machine;
	UINT8 m_mcu_reg080;
	UINT8 m_iou_reg092;
	UINT8 m_last_playmode;
	UINT8 m_ay8910_a;
	UINT16 m_x, m_y, m_yline;
	UINT8 m_colours, m_mode, m_op;
	UINT32 m_debug_video;
	UINT8 m_vector;
	UINT8 m_eeprom_bits;
	UINT8 m_eeprom_state;

	DECLARE_READ8_MEMBER(nimbus_mcu_r);
	DECLARE_WRITE8_MEMBER(nimbus_mcu_w);
	DECLARE_READ8_MEMBER(scsi_r);
	DECLARE_WRITE8_MEMBER(scsi_w);
	DECLARE_WRITE8_MEMBER(fdc_ctl_w);
	DECLARE_READ8_MEMBER(nimbus_pc8031_r);
	DECLARE_WRITE8_MEMBER(nimbus_pc8031_w);
	DECLARE_READ8_MEMBER(nimbus_pc8031_iou_r);
	DECLARE_WRITE8_MEMBER(nimbus_pc8031_iou_w);
	DECLARE_READ8_MEMBER(nimbus_pc8031_port_r);
	DECLARE_WRITE8_MEMBER(nimbus_pc8031_port_w);
	DECLARE_READ8_MEMBER(nimbus_iou_r);
	DECLARE_WRITE8_MEMBER(nimbus_iou_w);
	DECLARE_WRITE8_MEMBER(nimbus_sound_ay8910_porta_w);
	DECLARE_WRITE8_MEMBER(nimbus_sound_ay8910_portb_w);
	DECLARE_READ8_MEMBER(nimbus_mouse_js_r);
	DECLARE_WRITE8_MEMBER(nimbus_mouse_js_w);
	DECLARE_READ16_MEMBER(nimbus_video_io_r);
	DECLARE_WRITE16_MEMBER(nimbus_video_io_w);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	virtual void video_reset() override;
	UINT32 screen_update_nimbus(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(sio_interrupt);
	DECLARE_WRITE_LINE_MEMBER(nimbus_fdc_intrq_w);
	DECLARE_WRITE_LINE_MEMBER(nimbus_fdc_drq_w);
	DECLARE_WRITE8_MEMBER(nimbus_via_write_portb);
	DECLARE_WRITE_LINE_MEMBER(write_scsi_bsy);
	DECLARE_WRITE_LINE_MEMBER(write_scsi_cd);
	DECLARE_WRITE_LINE_MEMBER(write_scsi_io);
	DECLARE_WRITE_LINE_MEMBER(write_scsi_msg);
	DECLARE_WRITE_LINE_MEMBER(write_scsi_req);
	DECLARE_WRITE_LINE_MEMBER(nimbus_msm5205_vck);
	DECLARE_WRITE_LINE_MEMBER(write_scsi_iena);

	UINT8 get_pixel(UINT16 x, UINT16 y);
	UINT16 read_pixel_line(UINT16 x, UINT16 y, UINT8 pixels, UINT8 bpp);
	UINT16 read_pixel_data(UINT16 x, UINT16 y);
	void set_pixel(UINT16 x, UINT16 y, UINT8 colour);
	void set_pixel40(UINT16 x, UINT16 y, UINT8 colour);
	void write_pixel_line(UINT16 x, UINT16 y, UINT16, UINT8 pixels, UINT8 bpp);
	void move_pixel_line(UINT16 x, UINT16 y, UINT8 width);
	void write_pixel_data(UINT16 x, UINT16 y, UINT16    data);
	void change_palette(UINT8 bank, UINT16 colours);
	void external_int(UINT8 vector, bool state);
	DECLARE_READ8_MEMBER(cascade_callback);
	void nimbus_bank_memory();
	void memory_reset();
	void fdc_reset();
	UINT8 fdc_driveno(UINT8 drivesel);
	void hdc_reset();
	void hdc_ctrl_write(UINT8 data);
	void hdc_post_rw();
	void hdc_drq(bool state);
	void pc8031_reset();
	//void ipc_dumpregs();
	void iou_reset();
	void rmni_sound_reset();
	void mouse_js_reset();
	void check_scsi_irq();

	int m_scsi_iena;
	int m_scsi_msg;
	int m_scsi_bsy;
	int m_scsi_io;
	int m_scsi_cd;
	int m_scsi_req;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	enum
	{
		TIMER_MOUSE
	};

	// Static data related to Floppy and SCSI hard disks
	struct
	{
		UINT8   reg400;
	} m_nimbus_drives;

	/* 8031 Peripheral controler */
	struct
	{
		UINT8   ipc_in;
		UINT8   ipc_out;
		UINT8   status_in;
		UINT8   status_out;
	} m_ipc_interface;

	/* Mouse/Joystick */
	struct
	{
		UINT8   m_mouse_px;
		UINT8   m_mouse_py;

		UINT8   m_mouse_x;
		UINT8   m_mouse_y;
		UINT8   m_mouse_pc;
		UINT8   m_mouse_pcx;
		UINT8   m_mouse_pcy;

		UINT8   m_intstate_x;
		UINT8   m_intstate_y;

		UINT8   m_reg0a4;

		emu_timer   *m_mouse_timer;
	} m_nimbus_mouse;
};
