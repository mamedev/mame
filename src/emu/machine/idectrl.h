/***************************************************************************

    idectrl.h

    Generic (PC-style) IDE controller implementation.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __IDECTRL_H__
#define __IDECTRL_H__

#include "ataintf.h"

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_IDE_CONTROLLER_ADD(_tag, _slotintf, _master, _slave, _fixed) \
	MCFG_DEVICE_ADD(_tag, IDE_CONTROLLER, 0) \
	MCFG_ATA_SLOT_ADD(_tag ":0", _slotintf, _master, _fixed) \
	MCFG_ATA_SLOT_ADD(_tag ":1", _slotintf, _slave, _fixed) \
	MCFG_DEVICE_MODIFY(_tag)

class ide_controller_device : public ata_interface_device
{
public:
	ide_controller_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	ide_controller_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);

	virtual DECLARE_READ16_MEMBER(read_cs0);
	virtual DECLARE_READ16_MEMBER(read_cs1);
	virtual DECLARE_WRITE16_MEMBER(write_cs0);
	virtual DECLARE_WRITE16_MEMBER(write_cs1);
};

extern const device_type IDE_CONTROLLER;


#define MCFG_BUS_MASTER_IDE_CONTROLLER_ADD(_tag, _slotintf, _master, _slave, _fixed) \
	MCFG_DEVICE_ADD(_tag, BUS_MASTER_IDE_CONTROLLER, 0) \
	MCFG_ATA_SLOT_ADD(_tag ":0", _slotintf, _master, _fixed) \
	MCFG_ATA_SLOT_ADD(_tag ":1", _slotintf, _slave, _fixed) \
	MCFG_DEVICE_MODIFY(_tag)

#define MCFG_BUS_MASTER_IDE_CONTROLLER_SPACE(bmcpu, bmspace) \
	bus_master_ide_controller_device::set_bus_master_space(*device, bmcpu, bmspace);

class bus_master_ide_controller_device : public ide_controller_device
{
public:
	bus_master_ide_controller_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	static void set_bus_master_space(device_t &device, const char *bmcpu, UINT32 bmspace) {bus_master_ide_controller_device &ide = downcast<bus_master_ide_controller_device &>(device); ide.bmcpu = bmcpu; ide.bmspace = bmspace; }

	DECLARE_READ32_MEMBER( ide_bus_master32_r );
	DECLARE_WRITE32_MEMBER( ide_bus_master32_w );

protected:
	virtual void device_start();

	virtual void set_irq(int state);
	virtual void set_dmarq(int state);

private:
	void execute_dma();

	const char *bmcpu;
	UINT32 bmspace;
	address_space * dma_space;
	UINT8           dma_address_xor;

	offs_t          dma_address;
	UINT32          dma_bytes_left;
	offs_t          dma_descriptor;
	UINT8           dma_last_buffer;
	UINT8           bus_master_command;
	UINT8           bus_master_status;
	UINT32          bus_master_descriptor;
	int m_irq;
	int m_dmarq;
};

extern const device_type BUS_MASTER_IDE_CONTROLLER;

#endif  /* __IDECTRL_H__ */
