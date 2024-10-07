// license:BSD-3-Clause
// copyright-holders:R. Belmont,Ryan Holtz,Fabio Priuli
#ifndef MAME_BUS_GBA_ROM_H
#define MAME_BUS_GBA_ROM_H

#pragma once

#include "gba_slot.h"
#include "machine/intelfsh.h"

// GBA RTC device

class gba_s3511_device
{
public:
	gba_s3511_device(running_machine &machine);
	running_machine &machine() const { return m_machine; }

	void update_time(int len);
	uint8_t convert_to_bcd(int val);

	int read_line();
	void write(uint16_t data, int gpio_dirs);

protected:
	enum
	{
		S3511_RTC_IDLE = 0,
		S3511_RTC_DATAOUT,
		S3511_RTC_DATAIN,
		S3511_RTC_COMMAND
	};

	int m_phase;
	uint8_t m_last_val, m_bits, m_command;
	int m_data_len;
	uint8_t m_data[7];

	running_machine& m_machine;
};



// GBA EEPROM device
// TODO: is it possible to merge this with the standard EEPROM devices in the core?

class gba_eeprom_device
{
public:
	gba_eeprom_device(running_machine &machine, uint8_t *eeprom, uint32_t size, int addr_bits);
	running_machine &machine() const { return m_machine; }

	uint32_t read();
	void write(uint32_t data);

protected:
	enum
	{
		EEP_IDLE = 0,
		EEP_COMMAND,
		EEP_ADDR,
		EEP_AFTERADDR,
		EEP_READ,
		EEP_WRITE,
		EEP_AFTERWRITE,
		EEP_READFIRST
	};

	uint8_t *m_data;
	uint32_t m_data_size;
	int m_state;
	int m_command;
	int m_count;
	int m_addr;
	int m_bits;
	int m_addr_bits;
	uint8_t m_eep_data;

	running_machine& m_machine;
};



// ======================> gba_rom_device

class gba_rom_device : public device_t,
						public device_gba_cart_interface
{
public:
	// construction/destruction
	gba_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint32_t read_rom(offs_t offset) override { return m_rom[offset]; }

	virtual uint32_t read_gpio(offs_t offset, uint32_t mem_mask = ~0) override;
	virtual void write_gpio(offs_t offset, uint32_t data, uint32_t mem_mask = ~0) override;

	virtual uint16_t gpio_dev_read(int gpio_dirs) { return 0; }
	virtual void gpio_dev_write(uint16_t data, int gpio_dirs) { }

protected:
	gba_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint16_t m_gpio_regs[4];
	uint8_t m_gpio_write_only, m_gpio_dirs;
};


// ======================> gba_rom_sram_device

class gba_rom_sram_device : public gba_rom_device
{
public:
	// construction/destruction
	gba_rom_sram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint32_t read_ram(offs_t offset, uint32_t mem_mask = ~0) override;
	virtual void write_ram(offs_t offset, uint32_t data, uint32_t mem_mask = ~0) override;

protected:
	gba_rom_sram_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
};


// ======================> gba_rom_drilldoz_device

class gba_rom_drilldoz_device : public gba_rom_sram_device
{
public:
	// construction/destruction
	gba_rom_drilldoz_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void gpio_dev_write(uint16_t data, int gpio_dirs) override;

private:
	output_finder<> m_rumble;
};


// ======================> gba_rom_wariotws_device

class gba_rom_wariotws_device : public gba_rom_sram_device
{
public:
	// construction/destruction
	gba_rom_wariotws_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual uint16_t gpio_dev_read(int gpio_dirs) override;
	virtual void gpio_dev_write(uint16_t data, int gpio_dirs) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	output_finder<> m_rumble;

	uint8_t m_last_val;
	int m_counter;
	required_ioport m_gyro_z;
};


// ======================> gba_rom_flash_device

class gba_rom_flash_device : public gba_rom_device
{
public:
	// construction/destruction
	gba_rom_flash_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

// reading and writing
	virtual uint32_t read_ram(offs_t offset, uint32_t mem_mask = ~0) override;
	virtual void write_ram(offs_t offset, uint32_t data, uint32_t mem_mask = ~0) override;

protected:
	gba_rom_flash_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_reset() override ATTR_COLD;

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	//uint32_t m_flash_size;
	uint32_t m_flash_mask;
	required_device<intelfsh8_device> m_flash;
};


// ======================> gba_rom_flash_rtc_device

class gba_rom_flash_rtc_device : public gba_rom_flash_device
{
public:
	// construction/destruction
	gba_rom_flash_rtc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual uint16_t gpio_dev_read(int gpio_dirs) override;
	virtual void gpio_dev_write(uint16_t data, int gpio_dirs) override;

protected:
	virtual void device_start() override ATTR_COLD;

private:
	std::unique_ptr<gba_s3511_device> m_rtc;
};


// ======================> gba_rom_flash1m_device

class gba_rom_flash1m_device : public gba_rom_device
{
public:
	// construction/destruction
	gba_rom_flash1m_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint32_t read_ram(offs_t offset, uint32_t mem_mask = ~0) override;
	virtual void write_ram(offs_t offset, uint32_t data, uint32_t mem_mask = ~0) override;

protected:
	gba_rom_flash1m_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_reset() override ATTR_COLD;

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	//uint32_t m_flash_size;
	uint32_t m_flash_mask;
	required_device<intelfsh8_device> m_flash;
};


// ======================> gba_rom_flash1m_rtc_device

class gba_rom_flash1m_rtc_device : public gba_rom_flash1m_device
{
public:
	// construction/destruction
	gba_rom_flash1m_rtc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual uint16_t gpio_dev_read(int gpio_dirs) override;
	virtual void gpio_dev_write(uint16_t data, int gpio_dirs) override;

protected:
	virtual void device_start() override ATTR_COLD;

private:
	std::unique_ptr<gba_s3511_device> m_rtc;
};


// ======================> gba_rom_eeprom_device

class gba_rom_eeprom_device : public gba_rom_device
{
public:
	// construction/destruction
	gba_rom_eeprom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint32_t read_ram(offs_t offset, uint32_t mem_mask = ~0) override;
	virtual void write_ram(offs_t offset, uint32_t data, uint32_t mem_mask = ~0) override;

protected:
	gba_rom_eeprom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	std::unique_ptr<gba_eeprom_device> m_eeprom;
};


// ======================> gba_rom_yoshiug_device

class gba_rom_yoshiug_device : public gba_rom_eeprom_device
{
public:
	// construction/destruction
	gba_rom_yoshiug_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// reading and writing
	virtual uint32_t read_tilt(offs_t offset, uint32_t mem_mask = ~0) override;
	virtual void write_tilt(offs_t offset, uint32_t data) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	int m_tilt_ready;
	uint16_t m_xpos, m_ypos;
	required_ioport m_tilt_x;
	required_ioport m_tilt_y;
};


// ======================> gba_rom_eeprom64_device

class gba_rom_eeprom64_device : public gba_rom_device
{
public:
	// construction/destruction
	gba_rom_eeprom64_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint32_t read_ram(offs_t offset, uint32_t mem_mask = ~0) override;
	virtual void write_ram(offs_t offset, uint32_t data, uint32_t mem_mask = ~0) override;

protected:
	gba_rom_eeprom64_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	std::unique_ptr<gba_eeprom_device> m_eeprom;
};


// ======================> gba_rom_boktai_device

class gba_rom_boktai_device : public gba_rom_eeprom64_device
{
public:
	// construction/destruction
	gba_rom_boktai_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual uint16_t gpio_dev_read(int gpio_dirs) override;
	virtual void gpio_dev_write(uint16_t data, int gpio_dirs) override;

private:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	std::unique_ptr<gba_s3511_device> m_rtc;
	required_ioport m_sensor;
	uint8_t m_last_val;
	int m_counter;
};


// ======================> gba_rom_3dmatrix_device

class gba_rom_3dmatrix_device : public gba_rom_device
{
public:
	// construction/destruction
	gba_rom_3dmatrix_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual void write_mapper(offs_t offset, uint32_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint32_t m_src, m_dst, m_nblock;
};


// device type definition
DECLARE_DEVICE_TYPE(GBA_ROM_STD,         gba_rom_device)
DECLARE_DEVICE_TYPE(GBA_ROM_SRAM,        gba_rom_sram_device)
DECLARE_DEVICE_TYPE(GBA_ROM_DRILLDOZ,    gba_rom_drilldoz_device)
DECLARE_DEVICE_TYPE(GBA_ROM_WARIOTWS,    gba_rom_wariotws_device)
DECLARE_DEVICE_TYPE(GBA_ROM_EEPROM,      gba_rom_eeprom_device)
DECLARE_DEVICE_TYPE(GBA_ROM_YOSHIUG,     gba_rom_yoshiug_device)
DECLARE_DEVICE_TYPE(GBA_ROM_EEPROM64,    gba_rom_eeprom64_device)
DECLARE_DEVICE_TYPE(GBA_ROM_BOKTAI,      gba_rom_boktai_device)
DECLARE_DEVICE_TYPE(GBA_ROM_FLASH,       gba_rom_flash_device)
DECLARE_DEVICE_TYPE(GBA_ROM_FLASH_RTC,   gba_rom_flash_rtc_device)
DECLARE_DEVICE_TYPE(GBA_ROM_FLASH1M,     gba_rom_flash1m_device)
DECLARE_DEVICE_TYPE(GBA_ROM_FLASH1M_RTC, gba_rom_flash1m_rtc_device)
DECLARE_DEVICE_TYPE(GBA_ROM_3DMATRIX,    gba_rom_3dmatrix_device)


#endif // MAME_BUS_GBA_ROM_H
