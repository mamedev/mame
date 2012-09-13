#pragma once

#ifndef __KYOCERA__
#define __KYOCERA__


#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "imagedev/cartslot.h"
#include "imagedev/cassette.h"
#include "machine/ctronics.h"
#include "machine/i8155.h"
#include "machine/i8251.h"
#include "machine/im6402.h"
#include "machine/ram.h"
#include "machine/rp5c01.h"
#include "machine/upd1990a.h"
#include "video/hd44102.h"
#include "video/hd61830.h"
#include "sound/speaker.h"
#include "rendlay.h"

#define SCREEN_TAG		"screen"
#define I8085_TAG		"m19"
#define I8155_TAG		"m25"
#define UPD1990A_TAG	"m18"
#define IM6402_TAG		"m22"
#define MC14412_TAG		"m31"
#define HD44102_0_TAG	"m1"
#define HD44102_1_TAG	"m2"
#define HD44102_2_TAG	"m3"
#define HD44102_3_TAG	"m4"
#define HD44102_4_TAG	"m5"
#define HD44102_5_TAG	"m6"
#define HD44102_6_TAG	"m7"
#define HD44102_7_TAG	"m8"
#define HD44102_8_TAG	"m9"
#define HD44102_9_TAG	"m10"
#define CENTRONICS_TAG	"centronics"

//#define I8085_TAG     "m19"
//#define I8155_TAG     "m12"
//#define MC14412_TAG   "m8"
#define RP5C01A_TAG		"m301"
#define TCM5089_TAG		"m11"
#define I8251_TAG		"m20"
#define HD61830_TAG		"m18"

class kc85_state : public driver_device
{
public:
	kc85_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, I8085_TAG),
		  m_rtc(*this, UPD1990A_TAG),
		  m_uart(*this, IM6402_TAG),
		  m_lcdc0(*this, HD44102_0_TAG),
		  m_lcdc1(*this, HD44102_1_TAG),
		  m_lcdc2(*this, HD44102_2_TAG),
		  m_lcdc3(*this, HD44102_3_TAG),
		  m_lcdc4(*this, HD44102_4_TAG),
		  m_lcdc5(*this, HD44102_5_TAG),
		  m_lcdc6(*this, HD44102_6_TAG),
		  m_lcdc7(*this, HD44102_7_TAG),
		  m_lcdc8(*this, HD44102_8_TAG),
		  m_lcdc9(*this, HD44102_9_TAG),
		  m_centronics(*this, CENTRONICS_TAG),
		  m_speaker(*this, SPEAKER_TAG),
		  m_cassette(*this, CASSETTE_TAG),
		  m_ram(*this, RAM_TAG)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<upd1990a_device> m_rtc;
	optional_device<im6402_device> m_uart;
	required_device<hd44102_device> m_lcdc0;
	required_device<hd44102_device> m_lcdc1;
	required_device<hd44102_device> m_lcdc2;
	required_device<hd44102_device> m_lcdc3;
	required_device<hd44102_device> m_lcdc4;
	required_device<hd44102_device> m_lcdc5;
	required_device<hd44102_device> m_lcdc6;
	required_device<hd44102_device> m_lcdc7;
	required_device<hd44102_device> m_lcdc8;
	required_device<hd44102_device> m_lcdc9;
	required_device<centronics_device> m_centronics;
	required_device<device_t> m_speaker;
	required_device<cassette_image_device> m_cassette;
	required_device<ram_device> m_ram;

	virtual void machine_start();

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER( uart_status_r );
	DECLARE_WRITE8_MEMBER( uart_ctrl_w );
	DECLARE_WRITE8_MEMBER( modem_w );
	DECLARE_WRITE8_MEMBER( ctrl_w );
	DECLARE_READ8_MEMBER( keyboard_r );
	DECLARE_READ8_MEMBER( lcd_r );
	DECLARE_WRITE8_MEMBER( lcd_w );
	DECLARE_WRITE8_MEMBER( i8155_pa_w );
	DECLARE_WRITE8_MEMBER( i8155_pb_w );
	DECLARE_READ8_MEMBER( i8155_pc_r );
	DECLARE_WRITE_LINE_MEMBER( i8155_to_w );

	/* memory state */
	UINT8 m_bank;			/* memory bank selection */

	/* keyboard state */
	UINT16 m_keylatch;		/* keyboard latch */

	/* sound state */
	int m_buzzer;				/* buzzer select */
	int m_bell;				/* bell output */
	
	DECLARE_PALETTE_INIT(kc85);
};

class trsm100_state : public kc85_state
{
public:
	trsm100_state(const machine_config &mconfig, device_type type, const char *tag)
		: kc85_state(mconfig, type, tag) { }

	virtual void machine_start();
};

class pc8201_state : public kc85_state
{
public:
	pc8201_state(const machine_config &mconfig, device_type type, const char *tag)
		: kc85_state(mconfig, type, tag) { }

	virtual void machine_start();

	DECLARE_READ8_MEMBER( bank_r );
	DECLARE_WRITE8_MEMBER( bank_w );
	DECLARE_WRITE8_MEMBER( scp_w );
	DECLARE_READ8_MEMBER( uart_status_r );
	DECLARE_WRITE8_MEMBER( romah_w );
	DECLARE_WRITE8_MEMBER( romal_w );
	DECLARE_WRITE8_MEMBER( romam_w );
	DECLARE_READ8_MEMBER( romrd_r );

	void bankswitch(UINT8 data);

	// ROM cassette
	int m_rom_sel;
	UINT32 m_rom_addr;

	/* peripheral state */
	int m_iosel;				/* serial interface select */
};

class tandy200_state : public driver_device
{
public:
	tandy200_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, I8085_TAG),
		  m_rtc(*this, RP5C01A_TAG),
		  m_lcdc(*this, HD61830_TAG),
		  m_centronics(*this, CENTRONICS_TAG),
		  m_speaker(*this, SPEAKER_TAG),
		  m_cassette(*this, CASSETTE_TAG),
		  m_ram(*this, RAM_TAG)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<rp5c01_device> m_rtc;
	required_device<hd61830_device> m_lcdc;
	required_device<centronics_device> m_centronics;
	required_device<device_t> m_speaker;
	required_device<cassette_image_device> m_cassette;
	required_device<ram_device> m_ram;

	virtual void machine_start();

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER( bank_r );
	DECLARE_WRITE8_MEMBER( bank_w );
	DECLARE_READ8_MEMBER( stbk_r );
	DECLARE_WRITE8_MEMBER( stbk_w );
	DECLARE_WRITE8_MEMBER( i8155_pa_w );
	DECLARE_WRITE8_MEMBER( i8155_pb_w );
	DECLARE_READ8_MEMBER( i8155_pc_r );
	DECLARE_WRITE_LINE_MEMBER( i8155_to_w );

	void bankswitch(UINT8 data);

	/* memory state */
	UINT8 m_bank;			/* memory bank selection */

	/* keyboard state */
	UINT16 m_keylatch;		/* keyboard latch */
	int m_tp;				/* timing pulse */

	/* sound state */
	int m_buzzer;			/* buzzer select */
	int m_bell;				/* bell output */
};

/* ---------- defined in video/kyocera.c ---------- */

MACHINE_CONFIG_EXTERN( kc85_video );
MACHINE_CONFIG_EXTERN( tandy200_video );

#endif
