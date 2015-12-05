// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __NES_REXSOFT_H
#define __NES_REXSOFT_H

#include "mmc3.h"


// ======================> nes_rex_dbz5_device

class nes_rex_dbz5_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_rex_dbz5_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_READ8_MEMBER(read_l);
	virtual DECLARE_READ8_MEMBER(read_m) override { return read_l(space, offset, mem_mask); }
	virtual DECLARE_WRITE8_MEMBER(write_l);
	virtual void chr_cb( int start, int bank, int source ) override;

	virtual void pcb_reset() override;

private:
	UINT8 m_extra;
};


// ======================> nes_rex_sl1632_device

class nes_rex_sl1632_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_rex_sl1632_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual void pcb_reset() override;

protected:
	virtual void set_prg(int prg_base, int prg_mask) override;
	virtual void set_chr(UINT8 chr, int chr_base, int chr_mask) override;

	UINT8 m_mode, m_mirror;
	UINT8 m_extra_bank[12];
};





// device type definition
extern const device_type NES_REX_DBZ5;
extern const device_type NES_REX_SL1632;

#endif
