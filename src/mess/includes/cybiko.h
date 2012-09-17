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
#include "machine/sst39vfx.h"
#include "machine/ram.h"

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
	m_crtc(*this, "hd66421"),
	m_speaker(*this, SPEAKER_TAG),
	m_rtc(*this, "rtc"),
	m_flash1(*this, "flash1")
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
	int cybiko_key_r( offs_t offset, int mem_mask);
	void cybiko_rs232_write_byte(int data);
	void cybiko_rs232_pin_sck(int data);
	void cybiko_rs232_pin_txd(int data);
	int cybiko_rs232_pin_rxd();
	int cybiko_rs232_rx_queue();

	required_device<hd66421_device> m_crtc;
	required_device<device_t> m_speaker;
	required_device<device_t> m_rtc;
	optional_device<device_t> m_flash1;
	DECLARE_DRIVER_INIT(cybikoxt);
	DECLARE_DRIVER_INIT(cybikov1);
	DECLARE_DRIVER_INIT(cybikov2);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void palette_init();
	DECLARE_MACHINE_START(cybikov2);
	DECLARE_MACHINE_RESET(cybikov2);
	DECLARE_MACHINE_START(cybikoxt);
	DECLARE_MACHINE_RESET(cybikoxt);
};


/*----------- defined in machine/cybiko.c -----------*/

// driver init

// machine start




// machine reset





#endif /* CYBIKO_H_ */
