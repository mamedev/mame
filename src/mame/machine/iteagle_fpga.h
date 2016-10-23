// license:BSD-3-Clause
// copyright-holders:Ted Green
//*************************************
// iteagle fpga device
//*************************************
#ifndef ITEAGLE_FPGA_H
#define ITEAGLE_FPGA_H

#include "machine/pci.h"
#include "machine/nvram.h"
#include "machine/eepromser.h"

//MCFG_PCI_DEVICE_ADD(_tag, _type, _main_id, _revision, _pclass, _subsystem_id)

#define MCFG_ITEAGLE_FPGA_ADD(_tag, _cpu_tag, _irq_num, _serial_irq_num) \
	MCFG_PCI_DEVICE_ADD(_tag, ITEAGLE_FPGA, 0x55CC33AA, 0xAA, 0xAAAAAA, 0x00) \
	downcast<iteagle_fpga_device *>(device)->set_irq_info(_cpu_tag, _irq_num, _serial_irq_num);

#define MCFG_ITEAGLE_FPGA_INIT(_version, _seq_init) \
	downcast<iteagle_fpga_device *>(device)->set_init_info(_version, _seq_init);

#define MCFG_ITEAGLE_EEPROM_ADD(_tag) \
	MCFG_PCI_DEVICE_ADD(_tag, ITEAGLE_EEPROM, 0x80861229, 0x02, 0x020000, 0x00)

#define MCFG_ITEAGLE_EEPROM_INIT(_sw_version, _hw_version) \
	downcast<iteagle_eeprom_device *>(device)->set_info(_sw_version, _hw_version);

// Mimic Cypress CY82C693 Peripheral Controller
#define MCFG_ITEAGLE_PERIPH_ADD(_tag) \
	MCFG_PCI_DEVICE_ADD(_tag, ITEAGLE_PERIPH, 0x1080C693, 0x00, 0x060100, 0x00)


class iteagle_fpga_device : public pci_device
{
public:
	iteagle_fpga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual machine_config_constructor device_mconfig_additions() const override;

	required_device<nvram_device> m_rtc;

	void set_init_info(int version, int seq_init) {m_version=version; m_seq_init=seq_init;}
	void set_irq_info(const char *tag, const int irq_num, const int serial_num) {
		m_cpu_tag = tag; m_irq_num = irq_num; m_serial_irq_num = serial_num;}

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	emu_timer *     m_timer;
	const char *m_cpu_tag;
	cpu_device *m_cpu;
	int m_irq_num;
	int m_serial_irq_num;

	uint32_t m_fpga_regs[0x20 / 4];
	uint32_t m_rtc_regs[0x800 / 4];
	uint32_t m_ram[0x20000 / 4];
	uint32_t m_prev_reg;

	std::string m_serial_str;
	std::string m_serial_rx3;
	uint8_t m_serial_idx;
	bool  m_serial_data;
	uint8_t m_serial_com0[0x10];
	uint8_t m_serial_com1[0x10];
	uint8_t m_serial_com2[0x10];
	uint8_t m_serial_com3[0x10];

	uint32_t m_version;
	uint32_t m_seq_init;
	uint32_t m_seq;
	uint32_t m_seq_rem1, m_seq_rem2;
	void update_sequence(uint32_t data);
	void update_sequence_eg1(uint32_t data);

	DECLARE_ADDRESS_MAP(rtc_map, 32);
	DECLARE_ADDRESS_MAP(fpga_map, 32);
	DECLARE_ADDRESS_MAP(ram_map, 32);

	uint32_t fpga_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void fpga_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t rtc_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void rtc_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

	uint32_t ram_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void ram_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
};

class iteagle_eeprom_device : public pci_device {
public:
	iteagle_eeprom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	required_device<eeprom_serial_93cxx_device> m_eeprom;

	void set_info(int sw_version, int hw_version) {m_sw_version=sw_version; m_hw_version=hw_version;}
protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	address_space *m_memory_space;
	uint16_t m_sw_version;
	uint8_t m_hw_version;

	std::array<uint16_t, 0x40> m_iteagle_default_eeprom;

	DECLARE_ADDRESS_MAP(eeprom_map, 32);
	uint32_t eeprom_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void eeprom_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
};

class iteagle_periph_device : public pci_device {
public:
	iteagle_periph_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual machine_config_constructor device_mconfig_additions() const override;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	optional_device<nvram_device> m_rtc;

	uint32_t m_ctrl_regs[0xd0/4];
	uint8_t m_rtc_regs[0x100];

	DECLARE_ADDRESS_MAP(ctrl_map, 32);

	uint32_t ctrl_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void ctrl_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

};

extern const device_type ITEAGLE_FPGA;
extern const device_type ITEAGLE_EEPROM;
extern const device_type ITEAGLE_PERIPH;

#endif
