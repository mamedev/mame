// license:BSD-3-Clause
// copyright-holders:Ted Green
//*************************************
// iteagle fpga device
//*************************************
#ifndef ITEAGLE_FPGA_H
#define ITEAGLE_FPGA_H

#include "machine/pci.h"
#include "machine/idectrl.h"
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
#define MCFG_ITEAGLE_IDE_ADD(_tag) \
	MCFG_PCI_DEVICE_ADD(_tag, ITEAGLE_IDE, 0x1080C693, 0x00, 0x060100, 0x00)

#define MCFG_ITEAGLE_IDE_IRQ_ADD(_cpu_tag, _irq_num) \
	downcast<iteagle_ide_device *>(device)->set_irq_info(_cpu_tag, _irq_num);

class iteagle_fpga_device : public pci_device,
				public device_nvram_interface
{
public:
	iteagle_fpga_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	void set_init_info(int version, int seq_init) {m_version=version; m_seq_init=seq_init;}
	void set_irq_info(const char *tag, const int irq_num, const int serial_num) {
		m_cpu_tag = tag; m_irq_num = irq_num; m_serial_irq_num = serial_num;}

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual void nvram_read(emu_file &file) override;
	virtual void nvram_write(emu_file &file) override;

private:
	emu_timer *     m_timer;
	const char *m_cpu_tag;
	cpu_device *m_cpu;
	int m_irq_num;
	int m_serial_irq_num;

	UINT32 m_fpga_regs[0x20/4];
	UINT32 m_rtc_regs[0x800/4];
	UINT32 m_ram[0x20000/4];
	UINT32 m_prev_reg;

	std::string m_serial_str;
	std::string m_serial_rx3;
	UINT8 m_serial_idx;
	bool  m_serial_data;
	UINT8 m_serial_com0[0x10];
	UINT8 m_serial_com1[0x10];
	UINT8 m_serial_com2[0x10];
	UINT8 m_serial_com3[0x10];

	UINT32 m_version;
	UINT32 m_seq_init;
	UINT32 m_seq;
	UINT32 m_seq_rem1, m_seq_rem2;
	void update_sequence(UINT32 data);
	void update_sequence_eg1(UINT32 data);

	DECLARE_ADDRESS_MAP(rtc_map, 32);
	DECLARE_ADDRESS_MAP(fpga_map, 32);
	DECLARE_ADDRESS_MAP(ram_map, 32);

	DECLARE_READ32_MEMBER( fpga_r );
	DECLARE_WRITE32_MEMBER( fpga_w );
	DECLARE_READ32_MEMBER( rtc_r );
	DECLARE_WRITE32_MEMBER( rtc_w );

	DECLARE_READ32_MEMBER( ram_r );
	DECLARE_WRITE32_MEMBER( ram_w );
};

class iteagle_eeprom_device : public pci_device {
public:
	iteagle_eeprom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void map_extra(UINT64 memory_window_start, UINT64 memory_window_end, UINT64 memory_offset, address_space *memory_space,
							UINT64 io_window_start, UINT64 io_window_end, UINT64 io_offset, address_space *io_space) override;

	required_device<eeprom_serial_93cxx_device> m_eeprom;

	void set_info(int sw_version, int hw_version) {m_sw_version=sw_version; m_hw_version=hw_version;}
protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	address_space *m_memory_space;
	UINT16 m_sw_version;
	UINT8 m_hw_version;

	DECLARE_ADDRESS_MAP(eeprom_map, 32);
	DECLARE_READ32_MEMBER( eeprom_r );
	DECLARE_WRITE32_MEMBER( eeprom_w );
};

class iteagle_ide_device : public pci_device {
public:
	iteagle_ide_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	void set_irq_info(const char *tag, const int irq_num);

	required_device<bus_master_ide_controller_device> m_ide;
	required_device<bus_master_ide_controller_device> m_ide2;
	DECLARE_WRITE_LINE_MEMBER(ide_interrupt);
	DECLARE_WRITE_LINE_MEMBER(ide2_interrupt);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	const char *m_cpu_tag;
	cpu_device *m_cpu;
	int m_irq_num;
	int m_irq_status;

	UINT32 m_ctrl_regs[0xd0/4];
	UINT8 m_rtc_regs[0x100];

	DECLARE_ADDRESS_MAP(ctrl_map, 32);
	DECLARE_ADDRESS_MAP(ide_map, 32);
	DECLARE_ADDRESS_MAP(ide_ctrl_map, 32);
	DECLARE_ADDRESS_MAP(ide2_map, 32);
	DECLARE_ADDRESS_MAP(ide2_ctrl_map, 32);

	DECLARE_READ32_MEMBER( ctrl_r );
	DECLARE_WRITE32_MEMBER( ctrl_w );

	DECLARE_READ32_MEMBER( ide_r );
	DECLARE_WRITE32_MEMBER( ide_w );
	DECLARE_READ32_MEMBER( ide_ctrl_r );
	DECLARE_WRITE32_MEMBER( ide_ctrl_w );

	DECLARE_READ32_MEMBER( ide2_r );
	DECLARE_WRITE32_MEMBER( ide2_w );
	DECLARE_READ32_MEMBER( ide2_ctrl_r );
	DECLARE_WRITE32_MEMBER( ide2_ctrl_w );

};

extern const device_type ITEAGLE_FPGA;
extern const device_type ITEAGLE_EEPROM;
extern const device_type ITEAGLE_IDE;

#endif
