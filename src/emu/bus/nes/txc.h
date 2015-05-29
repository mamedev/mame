// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __NES_TXC_H
#define __NES_TXC_H

#include "nxrom.h"


// ======================> nes_txc_22211_device

class nes_txc_22211_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_txc_22211_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	nes_txc_22211_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual DECLARE_READ8_MEMBER(read_l);
	virtual DECLARE_WRITE8_MEMBER(write_l);
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual void pcb_reset();

protected:
	UINT8 m_reg[4];
};


// ======================> nes_txc_dumarac_device

class nes_txc_dumarc_device : public nes_txc_22211_device
{
public:
	// construction/destruction
	nes_txc_dumarc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual DECLARE_WRITE8_MEMBER(write_h);
};


// ======================> nes_txc_mjblock_device

class nes_txc_mjblock_device : public nes_txc_22211_device
{
public:
	// construction/destruction
	nes_txc_mjblock_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual DECLARE_READ8_MEMBER(read_l);
};


// ======================> nes_txc_strikew_device

class nes_txc_strikew_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_txc_strikew_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual void pcb_reset();
};


// ======================> nes_txc_commandos_device

class nes_txc_commandos_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_txc_commandos_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual DECLARE_READ8_MEMBER(read_l);
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual void pcb_reset();
};




// device type definition
extern const device_type NES_TXC_22211;
extern const device_type NES_TXC_DUMARACING;
extern const device_type NES_TXC_MJBLOCK;
extern const device_type NES_TXC_STRIKEW;
extern const device_type NES_TXC_COMMANDOS;

#endif
