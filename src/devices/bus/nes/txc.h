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
	nes_txc_22211_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);
	nes_txc_22211_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_READ8_MEMBER(read_l) override;
	virtual DECLARE_WRITE8_MEMBER(write_l) override;
	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual void pcb_reset() override;

protected:
	UINT8 m_reg[4];
};


// ======================> nes_txc_dumarac_device

class nes_txc_dumarc_device : public nes_txc_22211_device
{
public:
	// construction/destruction
	nes_txc_dumarc_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual DECLARE_WRITE8_MEMBER(write_h) override;
};


// ======================> nes_txc_mjblock_device

class nes_txc_mjblock_device : public nes_txc_22211_device
{
public:
	// construction/destruction
	nes_txc_mjblock_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual DECLARE_READ8_MEMBER(read_l) override;
};


// ======================> nes_txc_strikew_device

class nes_txc_strikew_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_txc_strikew_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual void pcb_reset() override;
};


// ======================> nes_txc_commandos_device

class nes_txc_commandos_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_txc_commandos_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_READ8_MEMBER(read_l) override;
	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual void pcb_reset() override;
};




// device type definition
extern const device_type NES_TXC_22211;
extern const device_type NES_TXC_DUMARACING;
extern const device_type NES_TXC_MJBLOCK;
extern const device_type NES_TXC_STRIKEW;
extern const device_type NES_TXC_COMMANDOS;

#endif
