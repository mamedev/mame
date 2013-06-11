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

SLOT_INTERFACE_EXTERN(ide_devices);
SLOT_INTERFACE_EXTERN(ide_devices);

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_IDE_CONTROLLER_ADD(_tag, _slotintf, _master, _slave, _fixed) \
	MCFG_IDE_SLOT_ADD("drive_0", _slotintf, _master, _fixed) \
	MCFG_IDE_SLOT_ADD("drive_1", _slotintf, _slave, _fixed) \
	MCFG_DEVICE_ADD(_tag, IDE_CONTROLLER, 0)

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

	UINT8 *ide_get_features(int drive);
	void ide_set_gnet_readlock(const UINT8 onoff);
	void ide_set_master_password(const UINT8 *password);
	void ide_set_user_password(const UINT8 *password);

	DECLARE_READ8_MEMBER(read_via_config);
	DECLARE_WRITE8_MEMBER(write_via_config);
	UINT16 read_dma();
	DECLARE_READ16_MEMBER(read_cs0);
	DECLARE_READ16_MEMBER(read_cs1);
	void write_dma(UINT16 data);
	DECLARE_WRITE16_MEMBER(write_cs0);
	DECLARE_WRITE16_MEMBER(write_cs1);

	DECLARE_READ16_MEMBER(read_cs0_pc);
	DECLARE_READ16_MEMBER(read_cs1_pc);
	DECLARE_WRITE16_MEMBER(write_cs0_pc);
	DECLARE_WRITE16_MEMBER(write_cs1_pc);

	virtual void set_irq(int state);
	virtual void set_dmarq(int state);
	void read_sector_done();
	void write_sector_done();

	UINT8           status;

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	void read_next_sector();
	void continue_write();

private:
	void signal_delayed_interrupt(attotime time, int buffer_ready);
	void next_sector();
	void security_error();
	void continue_read();
	void read_first_sector();
	void handle_command(UINT8 _command);
	void read_buffer_empty();
	void write_buffer_full();

	UINT8           dma_active;
	UINT8           adapter_control;
	UINT8           error;
	UINT8           command;
	UINT8           interrupt_pending;
	UINT8           precomp_offset;

	UINT8           buffer[IDE_DISK_SECTOR_SIZE];
	UINT16          buffer_offset;
	UINT16          sector_count;

	UINT16          block_count;
	UINT16          sectors_until_int;
	UINT8           verify_only;

	UINT8           config_unknown;
	UINT8           config_register[IDE_CONFIG_REGISTERS];
	UINT8           config_register_num;

	emu_timer *     last_status_timer;
	emu_timer *     reset_timer;

	UINT8           master_password_enable;
	UINT8           user_password_enable;
	const UINT8 *   master_password;
	const UINT8 *   user_password;

	UINT8           gnetreadlock;

	UINT8           cur_drive;
	ide_slot_device *slot[2];

	devcb2_write_line m_irq_handler;
};

extern const device_type IDE_CONTROLLER;


#define MCFG_BUS_MASTER_IDE_CONTROLLER_ADD(_tag, _slotintf, _master, _slave, _fixed) \
	MCFG_IDE_SLOT_ADD("drive_0", _slotintf, _master, _fixed) \
	MCFG_IDE_SLOT_ADD("drive_1", _slotintf, _slave, _fixed) \
	MCFG_DEVICE_ADD(_tag, BUS_MASTER_IDE_CONTROLLER, 0)

#define MCFG_BUS_MASTER_IDE_CONTROLLER_SPACE(bmcpu, bmspace) \
	bus_master_ide_controller_device::set_bus_master_space(*device, bmcpu, bmspace);

class bus_master_ide_controller_device : public ide_controller_device
{
public:
	bus_master_ide_controller_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	static void set_bus_master_space(device_t &device, const char *bmcpu, UINT32 bmspace) {bus_master_ide_controller_device &ide = downcast<bus_master_ide_controller_device &>(device); ide.bmcpu = bmcpu; ide.bmspace = bmspace; }

	DECLARE_READ32_MEMBER( ide_bus_master32_r );
	DECLARE_WRITE32_MEMBER( ide_bus_master32_w );

	virtual void set_irq(int state);
	virtual void set_dmarq(int state);

protected:
	virtual void device_start();

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
