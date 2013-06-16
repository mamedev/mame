/***************************************************************************

    idectrl.h

    Generic (PC-style) IDE controller implementation.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __IDECTRL_H__
#define __IDECTRL_H__

#include "idehd.h"
#include "harddisk.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> ide_slot_device

class ide_slot_device : public device_t,
						public device_slot_interface
{
public:
	// construction/destruction
	ide_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	ide_device_interface *dev() { return m_dev; }
protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_config_complete();
private:
	ide_device_interface *m_dev;
};

// device type definition
extern const device_type IDE_SLOT;

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

#define MCFG_IDE_CONTROLLER_IRQ_HANDLER(_devcb) \
	devcb = &ide_controller_device::set_irq_handler(*device, DEVCB2_##_devcb);

#define MCFG_IDE_CONTROLLER_DMARQ_HANDLER(_devcb) \
	devcb = &ide_controller_device::set_dmarq_handler(*device, DEVCB2_##_devcb);

SLOT_INTERFACE_EXTERN(ide_devices);
SLOT_INTERFACE_EXTERN(ide_devices);

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_IDE_CONTROLLER_ADD(_tag, _slotintf, _master, _slave, _fixed) \
	MCFG_DEVICE_ADD(_tag, IDE_CONTROLLER, 0) \
	MCFG_IDE_SLOT_ADD(_tag ":0", _slotintf, _master, _fixed) \
	MCFG_IDE_SLOT_ADD(_tag ":1", _slotintf, _slave, _fixed) \
	MCFG_DEVICE_MODIFY(_tag)

#define MCFG_IDE_SLOT_ADD(_tag, _slot_intf, _def_slot, _fixed) \
	MCFG_DEVICE_ADD(_tag, IDE_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, _fixed)

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

#define IDE_CONFIG_REGISTERS                0x10

/* ----- device interface ----- */

class ide_controller_device : public device_t
{
public:
	ide_controller_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	ide_controller_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	template<class _Object> static devcb2_base &set_irq_handler(device_t &device, _Object object) { return downcast<ide_controller_device &>(device).m_irq_handler.set_callback(object); }
	template<class _Object> static devcb2_base &set_dmarq_handler(device_t &device, _Object object) { return downcast<ide_controller_device &>(device).m_dmarq_handler.set_callback(object); }

	UINT8 *identify_device_buffer(int drive);
	void ide_set_master_password(int drive, const UINT8 *password);
	void ide_set_user_password(int drive, const UINT8 *password);

	UINT16 read_dma();
	DECLARE_READ16_MEMBER(read_cs0);
	DECLARE_READ16_MEMBER(read_cs1);

	void write_dma(UINT16 data);
	DECLARE_WRITE16_MEMBER(write_cs0);
	DECLARE_WRITE16_MEMBER(write_cs1);
	DECLARE_WRITE_LINE_MEMBER(write_dmack);

	DECLARE_READ8_MEMBER(read_via_config);
	DECLARE_WRITE8_MEMBER(write_via_config);
	DECLARE_READ16_MEMBER(read_cs0_pc);
	DECLARE_READ16_MEMBER(read_cs1_pc);
	DECLARE_WRITE16_MEMBER(write_cs0_pc);
	DECLARE_WRITE16_MEMBER(write_cs1_pc);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	virtual void set_irq(int state);
	virtual void set_dmarq(int state);

private:
	DECLARE_WRITE_LINE_MEMBER(irq0_write_line);
	DECLARE_WRITE_LINE_MEMBER(dmarq0_write_line);

	DECLARE_WRITE_LINE_MEMBER(irq1_write_line);
	DECLARE_WRITE_LINE_MEMBER(dmarq1_write_line);

	UINT8           m_config_unknown;
	UINT8           m_config_register[IDE_CONFIG_REGISTERS];
	UINT8           m_config_register_num;

	ide_slot_device *m_slot[2];
	int m_irq[2];
	int m_dmarq[2];

	devcb2_write_line m_irq_handler;
	devcb2_write_line m_dmarq_handler;
};

extern const device_type IDE_CONTROLLER;


#define MCFG_BUS_MASTER_IDE_CONTROLLER_ADD(_tag, _slotintf, _master, _slave, _fixed) \
	MCFG_DEVICE_ADD(_tag, BUS_MASTER_IDE_CONTROLLER, 0) \
	MCFG_IDE_SLOT_ADD(_tag ":0", _slotintf, _master, _fixed) \
	MCFG_IDE_SLOT_ADD(_tag ":1", _slotintf, _slave, _fixed) \
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
