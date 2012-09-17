/***************************************************************************

    idectrl.h

    Generic (PC-style) IDE controller implementation.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __IDECTRL_H__
#define __IDECTRL_H__

#include "devlegcy.h"

#include "harddisk.h"
#include "imagedev/harddriv.h"

#define IDE_DISK_SECTOR_SIZE			512

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/
// ======================> ide_device_interface

class ide_device_interface : public device_slot_card_interface
{
public:
	ide_device_interface(const machine_config &mconfig, device_t &device);
public:
	virtual int	 read_sector(UINT32 lba, void *buffer) = 0;
	virtual int	 write_sector(UINT32 lba, const void *buffer) = 0;

	UINT8 *get_features() { return m_features;}

	UINT16 get_cylinders() { return m_num_cylinders; }
	UINT16 get_sectors() { return m_num_sectors; }
	UINT16 get_heads() { return m_num_heads; }
	void set_geometry(UINT8 sectors, UINT8 heads) { m_num_sectors= sectors; m_num_heads=heads; }
	virtual bool is_ready() { return true; }
	virtual void read_key(UINT8 key[]) { }
protected:
	UINT8			m_features[IDE_DISK_SECTOR_SIZE];
	UINT16			m_num_cylinders;
	UINT8			m_num_sectors;
	UINT8			m_num_heads;
};

// ======================> ide_slot_device

class ide_slot_device :	public device_t,
						public device_slot_interface
{
public:
	// construction/destruction
	ide_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	int	 read_sector(UINT32 lba, void *buffer) { return (m_dev) ? m_dev->read_sector(lba,buffer) : 0; }
	int	 write_sector(UINT32 lba, const void *buffer) { return (m_dev) ? m_dev->write_sector(lba,buffer) : 0; }
	UINT8 *get_features() { return (m_dev) ? m_dev->get_features() : NULL;}

	UINT16 get_cylinders() { return (m_dev) ? m_dev->get_cylinders() : 0; }
	UINT16 get_sectors() { return (m_dev) ? m_dev->get_sectors() : 0; }
	UINT16 get_heads() { return (m_dev) ? m_dev->get_heads() : 0; }
	void set_geometry(UINT8 sectors, UINT8 heads) { if (m_dev) m_dev->set_geometry(sectors,heads); }
	bool is_ready() { return (m_dev) ? m_dev->is_ready() : false; }
	bool is_connected() { return (m_dev) ? true : false; }
	void read_key(UINT8 key[]) { if (m_dev) m_dev->read_key(key); }
protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_config_complete();
private:
	ide_device_interface *m_dev;
};

// device type definition
extern const device_type IDE_SLOT;

// ======================> ide_hdd_device

class ide_hdd_device : public device_t,
					   public ide_device_interface
{
public:
    // construction/destruction
    ide_hdd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	ide_hdd_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);

	virtual int	 read_sector(UINT32 lba, void *buffer) { return hard_disk_read(m_disk, lba, buffer); }
	virtual int	 write_sector(UINT32 lba, const void *buffer) { return hard_disk_write(m_disk, lba, buffer); }
	virtual void read_key(UINT8 key[]);
protected:
    // device-level overrides
    virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete() { m_shortname = "hdd"; }

	void ide_build_features();
	virtual bool is_ready() { return (m_disk != NULL); }
protected:
	chd_file       *m_handle;
	hard_disk_file *m_disk;
};
// device type definition
extern const device_type IDE_HARDDISK;

// ======================> ide_hdd_image_device

class ide_hdd_image_device : public ide_hdd_device
{
public:
    // construction/destruction
    ide_hdd_image_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
    // device-level overrides
    virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete() { m_shortname = "hdd_image"; }
	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;
};
// device type definition
extern const device_type IDE_HARDDISK_IMAGE;

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct ide_config
{
	void	(*interrupt)(device_t *device, int state);
	const char *bmcpu;		/* name of bus master CPU */
	UINT32 bmspace;			/* address space of bus master transfer */
};


SLOT_INTERFACE_EXTERN(ide_devices);
SLOT_INTERFACE_EXTERN(ide_image_devices);

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_IDE_CONTROLLER_ADD(_tag, _config, _slotintf, _master, _slave, _fixed) \
	MCFG_DEVICE_ADD(_tag, IDE_CONTROLLER, 0) \
	MCFG_DEVICE_CONFIG(_config) \
	MCFG_IDE_SLOT_ADD("drive_0", _slotintf, _master, NULL, _fixed) \
	MCFG_IDE_SLOT_ADD("drive_1", _slotintf, _slave, NULL, _fixed) \

#define MCFG_IDE_SLOT_ADD(_tag, _slot_intf, _def_slot, _def_inp, _fixed) \
	MCFG_DEVICE_ADD(_tag, IDE_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, _def_inp, _fixed) \

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

UINT8 *ide_get_features(device_t *device, int drive);

void ide_set_master_password(device_t *device, const UINT8 *password);
void ide_set_user_password(device_t *device, const UINT8 *password);

void ide_set_gnet_readlock(device_t *device, const UINT8 onoff);

int ide_bus_r(device_t *config, int select, int offset);
void ide_bus_w(device_t *config, int select, int offset, int data);

UINT32 ide_controller_r(device_t *config, int reg, int size);
void ide_controller_w(device_t *config, int reg, int size, UINT32 data);

DECLARE_READ32_DEVICE_HANDLER( ide_controller32_r );
DECLARE_WRITE32_DEVICE_HANDLER( ide_controller32_w );
DECLARE_READ32_DEVICE_HANDLER( ide_controller32_pcmcia_r );
DECLARE_WRITE32_DEVICE_HANDLER( ide_controller32_pcmcia_w );
DECLARE_READ32_DEVICE_HANDLER( ide_bus_master32_r );
DECLARE_WRITE32_DEVICE_HANDLER( ide_bus_master32_w );

DECLARE_READ16_DEVICE_HANDLER( ide_controller16_r );
DECLARE_WRITE16_DEVICE_HANDLER( ide_controller16_w );


/* ----- device interface ----- */

class ide_controller_device : public device_t
{
public:
	ide_controller_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~ide_controller_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
private:
	// internal state
	void *m_token;
};

extern const device_type IDE_CONTROLLER;



#endif	/* __IDECTRL_H__ */
