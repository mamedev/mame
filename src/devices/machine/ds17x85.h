// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Dallas DS17x85/DS17x87 Real Time Clocks with extended NVSRAM

***********************************************************************
                            _____   _____
                  /PWR   1 |*    \_/     | 24  Vcc
   (DS17x87: N.C.)  X1   2 |             | 23  SQW
   (DS17x87: N.C.)  X2   3 |             | 22  Vbaux
                   AD0   4 |             | 21  /RCLR
                   AD1   5 |   DS168x    | 20  Vbat (DS17x87: N.C.)
                   AD2   6 |   DS1728x   | 19  /IRQ
                   AD3   7 |   DS1748x   | 18  /KS
                   AD4   8 |   DS1788x   | 17  /RD
                   AD5   9 |             | 16  GND  (DS17x87: N.C.)
                   AD6  10 |             | 15  /WR
                   AD7  11 |             | 14  ALE
                   GND  12 |_____________| 13  /CS

**********************************************************************/

#ifndef MAME_MACHINE_DS17X85_H
#define MAME_MACHINE_DS17X85_H

#pragma once

#include "mc146818.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ds17x85_device

class ds17x85_device : public mc146818_device
{
protected:
	enum
	{
		REG_EXT_MODEL = 0x40 + 0x40,
		REG_EXT_SERIAL1 = 0x40 + 0x41,
		REG_EXT_SERIAL2 = 0x40 + 0x42,
		REG_EXT_SERIAL3 = 0x40 + 0x43,
		REG_EXT_SERIAL4 = 0x40 + 0x44,
		REG_EXT_SERIAL5 = 0x40 + 0x45,
		REG_EXT_SERIAL6 = 0x40 + 0x46,
		REG_EXT_SERIAL_CRC = 0x40 + 0x47,
		REG_EXT_CENTURY = 0x40 + 0x48,
		REG_EXT_DATE_ALARM = 0x40 + 0x49,
		REG_EXT_4A = 0x40 + 0x4a,
		REG_EXT_4B = 0x40 + 0x4b,
		REG_EXT_SMI_STACK2 = 0x40 + 0x4e,
		REG_EXT_SMI_STACK3 = 0x40 + 0x4f,
		REG_EXT_RAM_ADDRL = 0x40 + 0x50,
		REG_EXT_RAM_ADDRH = 0x40 + 0x51,
		REG_EXT_RAM_DATA = 0x40 + 0x53,
		REG_EXT_WRITE_COUNTER = 0x40 + 0x5e,
		REG_EXT_RAM_BASE = 0x40 + 0x80
	};

	enum
	{
		REG_EXT_4A_VRT2 = 0x80,
		REG_EXT_4A_INCR = 0x40,
		REG_EXT_4A_BME = 0x20,
		REG_EXT_4A_PAB = 0x08,
		REG_EXT_4A_RF = 0x04,
		REG_EXT_4A_WF = 0x02,
		REG_EXT_4A_KF = 0x01
	};

	enum
	{
		REG_EXT_4B_ABE = 0x80,
		REG_EXT_4B_E32k = 0x40,
		REG_EXT_4B_CS = 0x20,
		REG_EXT_4B_RCE = 0x10,
		REG_EXT_4B_PRS = 0x08,
		REG_EXT_4B_RIE = 0x04,
		REG_EXT_4B_WIE = 0x02,
		REG_EXT_4B_KSE = 0x01
	};

	ds17x85_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 model, u32 extram_size);

	// device-specific overrides
	virtual void device_start() override ATTR_COLD;

	// device_nvram_interface overrides
	virtual void nvram_default() override;

	// mc146818_device overrides
	virtual int data_size() const override { return REG_EXT_RAM_BASE + m_extram_size; }
	virtual int data_logical_size() const override { return 128; }
	virtual bool century_count_enabled() const override { return true; }
	virtual int get_timer_bypass() const override;
	virtual void internal_set_address(uint8_t data) override;
	virtual uint8_t internal_read(offs_t offset) override;
	virtual void internal_write(offs_t offset, uint8_t data) override;

private:
	const u8 m_model;
	const u32 m_extram_size;

	u8 m_smi_stack1;
};

// ======================> ds1685_device

class ds1685_device : public ds17x85_device
{
public:
	// device type constructor
	ds1685_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

// ======================> ds1687_device

class ds1687_device : public ds17x85_device
{
public:
	// device type constructor
	ds1687_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

// ======================> ds17285_device

class ds17285_device : public ds17x85_device
{
public:
	// device type constructor
	ds17285_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

// ======================> ds17287_device

class ds17287_device : public ds17x85_device
{
public:
	// device type constructor
	ds17287_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

// ======================> ds17485_device

class ds17485_device : public ds17x85_device
{
public:
	// device type constructor
	ds17485_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

// ======================> ds17487_device

class ds17487_device : public ds17x85_device
{
public:
	// device type constructor
	ds17487_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

// ======================> ds17885_device

class ds17885_device : public ds17x85_device
{
public:
	// device type constructor
	ds17885_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

// ======================> ds17887_device

class ds17887_device : public ds17x85_device
{
public:
	// device type constructor
	ds17887_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

// device type declarations
DECLARE_DEVICE_TYPE(DS1685, ds1685_device)
DECLARE_DEVICE_TYPE(DS1687, ds1687_device)
DECLARE_DEVICE_TYPE(DS17285, ds17285_device)
DECLARE_DEVICE_TYPE(DS17287, ds17287_device)
DECLARE_DEVICE_TYPE(DS17485, ds17485_device)
DECLARE_DEVICE_TYPE(DS17487, ds17487_device)
DECLARE_DEVICE_TYPE(DS17885, ds17885_device)
DECLARE_DEVICE_TYPE(DS17887, ds17887_device)

#endif // MAME_MACHINE_DS17X85_H
