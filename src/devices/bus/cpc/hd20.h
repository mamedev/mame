// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*

    Dobbertin HD20 hard disk

    Fixed disk interface for the Amstrad CPC

    Controller: Seagate ST11M XT HD Controller
    Disk: 3.5" 20MB Seagate, Kyocera, NEC or Miniscribe (Geometry: 615 cylinders/4 heads/17 sectors)

*/

#ifndef HD20_H_
#define HD20_H_

#include "emu.h"
#include "cpcexp.h"
#include "bus/isa/hdc.h"

class cpc_hd20_device  : public device_t,
							public device_cpc_expansion_card_interface
{
public:
	// construction/destruction
	cpc_hd20_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual const rom_entry *device_rom_region() const;

	DECLARE_READ8_MEMBER(hdc_r);
	DECLARE_WRITE8_MEMBER(hdc_w);
	DECLARE_WRITE_LINE_MEMBER(irq_w);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

private:
	cpc_expansion_slot_device *m_slot;

	required_device<xt_hdc_device> m_hdc;
};

// device type definition
extern const device_type CPC_HD20;

#endif /* HD20_H_ */
