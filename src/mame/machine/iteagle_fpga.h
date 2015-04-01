//*************************************
// iteagle fpga device
//*************************************
#ifndef ITEAGLE_FPGA_H
#define ITEAGLE_FPGA_H

#include "machine/pci.h"
#include "machine/idectrl.h"
#include "machine/eepromser.h"

#define MCFG_ITEAGLE_FPGA_ADD(_tag) \
	MCFG_PCI_DEVICE_ADD(_tag, ITEAGLE_FPGA, 0x55CC33AA, 0xAA, 0xAAAAAA, 0x00)

#define MCFG_ITEAGLE_EEPROM_ADD(_tag) \
	MCFG_PCI_DEVICE_ADD(_tag, ITEAGLE_EEPROM, 0xAABBCCDD, 0x00, 0x088000, 0x00)

#define MCFG_ITEAGLE_IDE_ADD(_tag) \
	MCFG_PCI_DEVICE_ADD(_tag, ITEAGLE_IDE, 0x11223344, 0x00, 0x010100, 0x00)

class iteagle_fpga_device : public pci_device {
public:
	iteagle_fpga_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual void device_start();
	virtual void device_reset();

private:

	UINT32 m_ctrl_regs[0x30];
	UINT32 m_fpga_regs[0x20];
	UINT32 m_rtc_regs[0x800];
	UINT32 m_prev_reg;

	DECLARE_ADDRESS_MAP(rtc_map, 32);
	DECLARE_ADDRESS_MAP(fpga_map, 32);
	DECLARE_ADDRESS_MAP(ctrl_map, 32);

	DECLARE_READ32_MEMBER( ctrl_r );
	DECLARE_WRITE32_MEMBER( ctrl_w );
	DECLARE_READ32_MEMBER( fpga_r );
	DECLARE_WRITE32_MEMBER( fpga_w );
	DECLARE_READ32_MEMBER( rtc_r );
	DECLARE_WRITE32_MEMBER( rtc_w );
};

class iteagle_eeprom_device : public pci_device {
public:
	iteagle_eeprom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;

	required_device<eeprom_serial_93cxx_device> m_eeprom;

protected:
	virtual void device_start();
	virtual void device_reset();

private:
	DECLARE_ADDRESS_MAP(eeprom_map, 32);
	DECLARE_READ32_MEMBER( eeprom_r );
	DECLARE_WRITE32_MEMBER( eeprom_w );
};

class iteagle_ide_device : public pci_device {
public:
	iteagle_ide_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;

	required_device<bus_master_ide_controller_device> m_ide;

protected:
	virtual void device_start();
	virtual void device_reset();

private:
	DECLARE_ADDRESS_MAP(ide_map, 32);
	DECLARE_ADDRESS_MAP(ide_ctrl_map, 32);

	DECLARE_READ32_MEMBER( ide_r );
	DECLARE_WRITE32_MEMBER( ide_w );
	DECLARE_READ32_MEMBER( ide_ctrl_r );
	DECLARE_WRITE32_MEMBER( ide_ctrl_w );
};

extern const device_type ITEAGLE_FPGA;
extern const device_type ITEAGLE_EEPROM;
extern const device_type ITEAGLE_IDE;

#endif
