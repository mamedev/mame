// license:BSD-3-Clause
// copyright-holders:R. Belmont,Ryan Holtz,Fabio Priuli
#ifndef __GBA_ROM_H
#define __GBA_ROM_H

#include "gba_slot.h"
#include "machine/intelfsh.h"

// GBA RTC device

enum {
	S3511_RTC_IDLE = 0,
	S3511_RTC_DATAOUT,
	S3511_RTC_DATAIN,
	S3511_RTC_COMMAND
};

class gba_s3511_device
{
public:
	gba_s3511_device(running_machine &machine);
	running_machine &machine() const { return m_machine; }
	
	void update_time(int len);
	UINT8 convert_to_bcd(int val);
	
	int read_line();
	void write(UINT16 data, int gpio_dirs);
	
protected:
	int m_phase;
	UINT8 m_last_val, m_bits, m_command;
	int m_data_len;
	UINT8 m_data[7];
	
	running_machine& m_machine;
};



// GBA EEPROM device
// TODO: is it possible to merge this with the standard EEPROM devices in the core?

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

class gba_eeprom_device
{
public:
	gba_eeprom_device(running_machine &machine, UINT8 *eeprom, UINT32 size, int addr_bits);
	running_machine &machine() const { return m_machine; }
	
	UINT32 read();
	void write(UINT32 data);
	
protected:
	UINT8 *m_data;
	UINT32 m_data_size;
	int m_state;
	int m_command;
	int m_count;
	int m_addr;
	int m_bits;
	int m_addr_bits;
	UINT8 m_eep_data;
	
	running_machine& m_machine;
};



// ======================> gba_rom_device

class gba_rom_device : public device_t,
						public device_gba_cart_interface
{
public:
	// construction/destruction
	gba_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	gba_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// reading and writing
	virtual DECLARE_READ32_MEMBER(read_rom) override { return m_rom[offset]; }

	virtual DECLARE_READ32_MEMBER(read_gpio) override;
	virtual DECLARE_WRITE32_MEMBER(write_gpio) override;
	
	virtual UINT16 gpio_dev_read(int gpio_dirs) { return 0; }
	virtual void gpio_dev_write(UINT16 data, int gpio_dirs) {}

private:
	UINT16 m_gpio_regs[4];
	UINT8 m_gpio_write_only, m_gpio_dirs;
};


// ======================> gba_rom_sram_device

class gba_rom_sram_device : public gba_rom_device
{
public:
	// construction/destruction
	gba_rom_sram_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	gba_rom_sram_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// reading and writing
	virtual DECLARE_READ32_MEMBER(read_ram) override;
	virtual DECLARE_WRITE32_MEMBER(write_ram) override;
};


// ======================> gba_rom_drilldoz_device

class gba_rom_drilldoz_device : public gba_rom_sram_device
{
public:
	// construction/destruction
	gba_rom_drilldoz_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	
	// device-level overrides
	virtual void gpio_dev_write(UINT16 data, int gpio_dirs) override;
};


// ======================> gba_rom_wariotws_device

class gba_rom_wariotws_device : public gba_rom_sram_device
{
public:
	// construction/destruction
	gba_rom_wariotws_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual ioport_constructor device_input_ports() const override;

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual UINT16 gpio_dev_read(int gpio_dirs) override;
	virtual void gpio_dev_write(UINT16 data, int gpio_dirs) override;

private:
	UINT8 m_last_val;
	int m_counter;
	required_ioport m_gyro_z;
};


// ======================> gba_rom_flash_device

class gba_rom_flash_device : public gba_rom_device
{
public:
	// construction/destruction
	gba_rom_flash_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	gba_rom_flash_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_reset() override;

	// reading and writing
	virtual DECLARE_READ32_MEMBER(read_ram) override;
	virtual DECLARE_WRITE32_MEMBER(write_ram) override;

protected:
	//UINT32 m_flash_size;
	UINT32 m_flash_mask;
	required_device<intelfsh8_device> m_flash;
};


// ======================> gba_rom_flash_rtc_device

class gba_rom_flash_rtc_device : public gba_rom_flash_device
{
public:
	// construction/destruction
	gba_rom_flash_rtc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	
	// device-level overrides
	virtual void device_start() override;
	virtual UINT16 gpio_dev_read(int gpio_dirs) override;
	virtual void gpio_dev_write(UINT16 data, int gpio_dirs) override;
	
private:
	std::unique_ptr<gba_s3511_device> m_rtc;
};


// ======================> gba_rom_flash1m_device

class gba_rom_flash1m_device : public gba_rom_device
{
public:
	// construction/destruction
	gba_rom_flash1m_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	gba_rom_flash1m_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_reset() override;

	// reading and writing
	virtual DECLARE_READ32_MEMBER(read_ram) override;
	virtual DECLARE_WRITE32_MEMBER(write_ram) override;

protected:
	//UINT32 m_flash_size;
	UINT32 m_flash_mask;
	required_device<intelfsh8_device> m_flash;
};


// ======================> gba_rom_flash1m_rtc_device

class gba_rom_flash1m_rtc_device : public gba_rom_flash1m_device
{
public:
	// construction/destruction
	gba_rom_flash1m_rtc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	
	// device-level overrides
	virtual void device_start() override;
	virtual UINT16 gpio_dev_read(int gpio_dirs) override;
	virtual void gpio_dev_write(UINT16 data, int gpio_dirs) override;

private:
	std::unique_ptr<gba_s3511_device> m_rtc;
};


// ======================> gba_rom_eeprom_device

class gba_rom_eeprom_device : public gba_rom_device
{
public:
	// construction/destruction
	gba_rom_eeprom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	gba_rom_eeprom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;

	// reading and writing
	virtual DECLARE_READ32_MEMBER(read_ram) override;
	virtual DECLARE_WRITE32_MEMBER(write_ram) override;

private:
	std::unique_ptr<gba_eeprom_device> m_eeprom;
};


// ======================> gba_rom_yoshiug_device

class gba_rom_yoshiug_device : public gba_rom_eeprom_device
{
public:
	// construction/destruction
	gba_rom_yoshiug_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual ioport_constructor device_input_ports() const override;
	
	// reading and writing
	virtual DECLARE_READ32_MEMBER(read_tilt) override;
	virtual DECLARE_WRITE32_MEMBER(write_tilt) override;
	
private:
	int m_tilt_ready;
	UINT16 m_xpos, m_ypos;
	required_ioport m_tilt_x;
	required_ioport m_tilt_y;
};


// ======================> gba_rom_eeprom64_device

class gba_rom_eeprom64_device : public gba_rom_device
{
public:
	// construction/destruction
	gba_rom_eeprom64_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	gba_rom_eeprom64_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;

	// reading and writing
	virtual DECLARE_READ32_MEMBER(read_ram) override;
	virtual DECLARE_WRITE32_MEMBER(write_ram) override;

protected:
	std::unique_ptr<gba_eeprom_device> m_eeprom;
};


// ======================> gba_rom_boktai_device

class gba_rom_boktai_device : public gba_rom_eeprom64_device
{
public:
	// construction/destruction
	gba_rom_boktai_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual ioport_constructor device_input_ports() const override;

	virtual UINT16 gpio_dev_read(int gpio_dirs) override;
	virtual void gpio_dev_write(UINT16 data, int gpio_dirs) override;
	
private:
	std::unique_ptr<gba_s3511_device> m_rtc;
	required_ioport m_sensor;
	UINT8 m_last_val;
	int m_counter;
};


// ======================> gba_rom_3dmatrix_device

class gba_rom_3dmatrix_device : public gba_rom_device
{
public:
	// construction/destruction
	gba_rom_3dmatrix_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// reading and writing
	virtual DECLARE_WRITE32_MEMBER(write_mapper) override;

private:
	UINT32 m_src, m_dst, m_nblock;
};


// device type definition
extern const device_type GBA_ROM_STD;
extern const device_type GBA_ROM_SRAM;
extern const device_type GBA_ROM_DRILLDOZ;
extern const device_type GBA_ROM_WARIOTWS;
extern const device_type GBA_ROM_EEPROM;
extern const device_type GBA_ROM_YOSHIUG;
extern const device_type GBA_ROM_EEPROM64;
extern const device_type GBA_ROM_BOKTAI;
extern const device_type GBA_ROM_FLASH;
extern const device_type GBA_ROM_FLASH_RTC;
extern const device_type GBA_ROM_FLASH1M;
extern const device_type GBA_ROM_FLASH1M_RTC;
extern const device_type GBA_ROM_3DMATRIX;



#endif
