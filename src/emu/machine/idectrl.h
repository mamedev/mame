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

#define MCFG_IDE_CONTROLLER_BUS_MASTER(bmcpu, bmspace) \
	ide_controller_device::set_bus_master(*device, bmcpu, bmspace);

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

	// static configuration helpers
	template<class _Object> static devcb2_base &set_irq_handler(device_t &device, _Object object) { return downcast<ide_controller_device &>(device).m_irq_handler.set_callback(object); }
	static void set_bus_master(device_t &device, const char *bmcpu, UINT32 bmspace) {ide_controller_device &ide = downcast<ide_controller_device &>(device); ide.bmcpu = bmcpu; ide.bmspace = bmspace; }

	UINT8 *ide_get_features(int drive);
	void ide_set_gnet_readlock(const UINT8 onoff);
	void ide_set_master_password(const UINT8 *password);
	void ide_set_user_password(const UINT8 *password);

	int ide_bus_r(int select, int offset);
	void ide_bus_w(int select, int offset, int data);

	UINT32 ide_controller_r(int reg, int size);
	void ide_controller_w(int reg, int size, UINT32 data);

	DECLARE_READ32_MEMBER( ide_controller32_r );
	DECLARE_WRITE32_MEMBER( ide_controller32_w );
	DECLARE_READ16_MEMBER( ide_controller16_pcmcia_r );
	DECLARE_WRITE16_MEMBER( ide_controller16_pcmcia_w );
	DECLARE_READ32_MEMBER( ide_bus_master32_r );
	DECLARE_WRITE32_MEMBER( ide_bus_master32_w );

	DECLARE_READ16_MEMBER( ide_controller16_r );
	DECLARE_WRITE16_MEMBER( ide_controller16_w );

	UINT32 ide_controller_read(int bank, offs_t offset, int size);
	void ide_controller_write(int bank, offs_t offset, int size, UINT32 data);
	UINT32 ide_bus_master_read(offs_t offset, int size);
	void ide_bus_master_write(offs_t offset, int size, UINT32 data);
	void signal_interrupt();
	void clear_interrupt();
	void read_sector_done();
	void write_sector_done();

	UINT8           status;

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

private:
	void signal_delayed_interrupt(attotime time, int buffer_ready);
	void next_sector();
	void security_error();
	void continue_read();
	void write_buffer_to_dma();
	void read_first_sector();
	void read_next_sector();
	void read_buffer_from_dma();
	void handle_command(UINT8 _command);
	void continue_write();

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

	UINT8           dma_active;
	address_space *dma_space;
	UINT8           dma_address_xor;
	UINT8           dma_last_buffer;
	offs_t          dma_address;
	offs_t          dma_descriptor;
	UINT32          dma_bytes_left;

	UINT8           bus_master_command;
	UINT8           bus_master_status;
	UINT32          bus_master_descriptor;

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
	const char *bmcpu;
	UINT32 bmspace;
};

extern const device_type IDE_CONTROLLER;

#endif  /* __IDECTRL_H__ */
