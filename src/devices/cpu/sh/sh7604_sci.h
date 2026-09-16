// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

  SH7604 SCI Controller

***************************************************************************/

#ifndef MAME_CPU_SH_SH7604_SCI_H
#define MAME_CPU_SH_SH7604_SCI_H

#pragma once



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sh7604_sci_device

class sh7604_sci_device : public device_t
{
public:
	// construction/destruction
	sh7604_sci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// I/O operations
	void sci_regs(address_map &map) ATTR_COLD;

	void write(address_space &space, offs_t offset, uint8_t data);
	uint8_t read(address_space &space, offs_t offset);

	uint8_t serial_mode_r();
	void serial_mode_w(uint8_t data);
	uint8_t bitrate_r();
	void bitrate_w(uint8_t data);
	uint8_t serial_control_r();
	void serial_control_w(uint8_t data);

	uint8_t transmit_data_r();
	void transmit_data_w(uint8_t data);
	uint8_t serial_status_r();
	void serial_ack_w(uint8_t data);
	uint8_t receive_data_r();

protected:
	enum
	{
		STATUS_MPBT = 1 << 0,
		STATUS_MPB =  1 << 1,
		STATUS_TEND = 1 << 2,
		STATUS_PER =  1 << 3,
		STATUS_FER =  1 << 4,
		STATUS_ORER = 1 << 5,
		STATUS_RDRF = 1 << 6,
		STATUS_TDRE = 1 << 7
	};

	// device-level overrides
//  virtual void device_validity_check(validity_checker &valid) const;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	const address_space_config      m_space_config;
	uint8_t m_smr;
	uint8_t m_scr;
	uint8_t m_ssr;
	uint8_t m_brr;
};


// device type definition
DECLARE_DEVICE_TYPE(SH7604_SCI, sh7604_sci_device)

#endif // MAME_CPU_SH_SH7604_SCI_H
