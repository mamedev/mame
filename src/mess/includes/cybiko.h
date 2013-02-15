/*****************************************************************************
 *
 * includes/cybiko.h
 *
 * Cybiko Wireless Inter-tainment System
 *
 * (c) 2001-2007 Tim Schuerewegen
 *
 * Cybiko Classic (V1)
 * Cybiko Classic (V2)
 * Cybiko Xtreme
 *
 ****************************************************************************/

#ifndef CYBIKO_H_
#define CYBIKO_H_

/* Core includes */
#include "emu.h"
#include "emuopts.h"
#include "sound/speaker.h"

/* Components */
#include "cpu/h83002/h8.h"
#include "video/hd66421.h"
#include "machine/pcf8593.h"
#include "machine/at45dbxx.h"
#include "machine/intelfsh.h"
#include "machine/ram.h"
#include "imagedev/snapquik.h"

struct CYBIKO_RS232_PINS
{
	int sck; // serial clock
	int txd; // transmit data
	int rxd; // receive data
};

struct CYBIKO_RS232
{
	CYBIKO_RS232_PINS pin;
	UINT8 rx_bits, rx_byte, tx_byte, tx_bits;
};

class cybiko_state : public driver_device
{
public:
	cybiko_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_crtc(*this, "hd66421"),
	m_speaker(*this, SPEAKER_TAG),
	m_rtc(*this, "rtc"),
	m_ram(*this, RAM_TAG),
	m_flash1(*this, "flash1"),
	m_flash2(*this, "flash2"),
	m_flashxt(*this, "flashxt")
	{ }

	CYBIKO_RS232 m_rs232;
	DECLARE_READ16_MEMBER(cybiko_lcd_r);
	DECLARE_WRITE16_MEMBER(cybiko_lcd_w);
	DECLARE_READ16_MEMBER(cybikov1_key_r);
	DECLARE_READ16_MEMBER(cybikov2_key_r);
	DECLARE_READ16_MEMBER(cybikoxt_key_r);
	DECLARE_WRITE16_MEMBER(cybiko_usb_w);
	DECLARE_READ8_MEMBER(cybikov1_io_reg_r);
	DECLARE_READ8_MEMBER(cybikov2_io_reg_r);
	DECLARE_READ8_MEMBER(cybikoxt_io_reg_r);
	DECLARE_WRITE8_MEMBER(cybikov1_io_reg_w);
	DECLARE_WRITE8_MEMBER(cybikov2_io_reg_w);
	DECLARE_WRITE8_MEMBER(cybikoxt_io_reg_w);
	DECLARE_READ8_MEMBER(cybikov2_flash_r);
	DECLARE_READ16_MEMBER(cybikoxt_flash_r);
	int cybiko_key_r( offs_t offset, int mem_mask);
	void cybiko_rs232_write_byte(int data);
	void cybiko_rs232_pin_sck(int data);
	void cybiko_rs232_pin_txd(int data);
	int cybiko_rs232_pin_rxd();
	int cybiko_rs232_rx_queue();
	void cybiko_rs232_init();
	void cybiko_rs232_exit();
	void cybiko_rs232_reset();

	required_device<cpu_device> m_maincpu;
	required_device<hd66421_device> m_crtc;
	required_device<speaker_sound_device> m_speaker;
	required_device<pcf8593_device> m_rtc;
	required_device<ram_device> m_ram;
	optional_device<at45db041_device> m_flash1;
	optional_device<intelfsh8_device> m_flash2;
	optional_device<intelfsh16_device> m_flashxt;
	DECLARE_DRIVER_INIT(cybikoxt);
	DECLARE_DRIVER_INIT(cybiko);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void palette_init();
	void machine_stop_cybiko();
};

NVRAM_HANDLER( cybikoxt );
QUICKLOAD_LOAD( cybiko );
QUICKLOAD_LOAD( cybikoxt );

#endif /* CYBIKO_H_ */
