/*
 * cpc_rs232.h
 *
 *  Created on: 22/04/2014
 */

#ifndef CPC_RS232_H_
#define CPC_RS232_H_

#include "emu.h"
#include "machine/z80dart.h"
#include "machine/pit8253.h"
#include "bus/rs232/rs232.h"
#include "cpcexp.h"

class cpc_rs232_device : public device_t,
					 	 public device_cpc_expansion_card_interface
{
public:
	// construction/destruction
	cpc_rs232_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;

	DECLARE_WRITE_LINE_MEMBER(pit_out0_w);
	DECLARE_WRITE_LINE_MEMBER(pit_out1_w);
	DECLARE_WRITE_LINE_MEMBER(pit_out2_w);

	DECLARE_READ8_MEMBER(dart_r);
	DECLARE_WRITE8_MEMBER(dart_w);
	DECLARE_READ8_MEMBER(pit_r);
	DECLARE_WRITE8_MEMBER(pit_w);

	required_device<pit8253_device> m_pit;
	required_device<z80dart_device> m_dart;
	required_device<rs232_port_device> m_rs232;
protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

private:
	cpc_expansion_slot_device *m_slot;
};

// device type definition
extern const device_type CPC_RS232;

#endif /* CPC_RS232_H_ */
