// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/*****************************************************************************

    includes/micronic.h

*****************************************************************************/

#ifndef __MICRONIC__
#define __MICRONIC__

#include "cpu/z80/z80.h"
#include "video/hd61830.h"
#include "machine/mc146818.h"
#include "machine/ram.h"
#include "machine/nvram.h"
#include "sound/beep.h"
#include "imagedev/cassette.h"

#define SCREEN_TAG      "screen"
#define Z80_TAG         "z80"
#define MC146818_TAG    "mc146818"
#define HD61830_TAG     "hd61830"

class micronic_state : public driver_device
{
public:
	micronic_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, Z80_TAG),
		m_lcdc(*this, HD61830_TAG),
		m_beep(*this, "beeper"),
		m_rtc(*this, MC146818_TAG),
		m_nvram1(*this, "nvram1"),
		m_nvram2(*this, "nvram2"),
		m_ram(*this, RAM_TAG),
		m_ram_base(*this, "ram_base"),
		m_status_flag(1),
		m_bank1(*this, "bank1"),
		m_bit0(*this, "BIT0"),
		m_bit1(*this, "BIT1"),
		m_bit2(*this, "BIT2"),
		m_bit3(*this, "BIT3"),
		m_bit4(*this, "BIT4"),
		m_bit5(*this, "BIT5"),
		m_backbattery(*this, "BACKBATTERY"),
		m_mainbattery(*this, "MAINBATTERY"),
		m_cassette(*this, "cassette")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<hd61830_device> m_lcdc;
	required_device<beep_device> m_beep;
	required_device<mc146818_device> m_rtc;
	required_device<nvram_device> m_nvram1;
	required_device<nvram_device> m_nvram2;
	required_device<ram_device> m_ram;

	virtual void machine_start() override;
	virtual void machine_reset() override;
	void nvram_init(nvram_device &nvram, void *data, size_t size);

	DECLARE_READ8_MEMBER( keypad_r );
	DECLARE_READ8_MEMBER( status_flag_r );
	DECLARE_WRITE8_MEMBER( status_flag_w );
	DECLARE_WRITE8_MEMBER( kp_matrix_w );
	DECLARE_WRITE8_MEMBER( beep_w );
	DECLARE_READ8_MEMBER( irq_flag_r );
	DECLARE_WRITE8_MEMBER( port_2c_w );
	DECLARE_WRITE8_MEMBER( bank_select_w );
	DECLARE_WRITE8_MEMBER( lcd_contrast_w );
	DECLARE_WRITE8_MEMBER( rtc_address_w );
	DECLARE_READ8_MEMBER( rtc_data_r );
	DECLARE_WRITE8_MEMBER( rtc_data_w );
	DECLARE_WRITE_LINE_MEMBER( mc146818_irq );

	required_shared_ptr<UINT8> m_ram_base;
	UINT8 m_banks_num;
	UINT8 m_kp_matrix;
	UINT8 m_lcd_contrast;
	int m_lcd_backlight;
	UINT8 m_status_flag;
	DECLARE_PALETTE_INIT(micronic);

protected:
	required_memory_bank m_bank1;
	required_ioport m_bit0;
	required_ioport m_bit1;
	required_ioport m_bit2;
	required_ioport m_bit3;
	required_ioport m_bit4;
	required_ioport m_bit5;
	required_ioport m_backbattery;
	required_ioport m_mainbattery;
	optional_device<cassette_image_device> m_cassette;
};

#endif
