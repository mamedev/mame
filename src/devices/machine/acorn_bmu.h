// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/*********************************************************************

    Acorn Battery Management Unit

*********************************************************************/

#ifndef MAME_MACHINE_ACORN_BMU_H
#define MAME_MACHINE_ACORN_BMU_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> acorn_bmu_device

class acorn_bmu_device : public device_t
{
public:
	acorn_bmu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void scl_w(int state);
	void sda_w(int state);
	int sda_r();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	static constexpr uint8_t BMU_SLAVE_ADDRESS = 0xa2;

	// internal state
	int m_slave_address;
	int m_scl;
	int m_sdaw;
	int m_sdar;
	int m_state;
	int m_bits;
	int m_shift;
	int m_devsel;
	int m_register;

	enum { STATE_IDLE, STATE_DEVSEL, STATE_REGISTER, STATE_DATAIN, STATE_DATAOUT, STATE_READSELACK };

	// registers
	enum
	{
		BMU_VERSION           = 0x50,
		BMU_TEMPERATURE       = 0x52,
		BMU_CURRENT           = 0x54,
		BMU_VOLTAGE           = 0x56,
		BMU_STATUS            = 0x5c,
		BMU_CHARGE_RATE       = 0x5e,
		BMU_CAPACITY_NOMINAL  = 0x80,
		BMU_CAPACITY_MEASURED = 0x82,
		BMU_CAPACITY_USED     = 0x88,
		BMU_CAPACITY_USABLE   = 0x8a,
		BMU_CHARGE_ESTIMATE   = 0x8e,
		BMU_COMMAND           = 0x90,
		BMU_AUTOSTART         = 0x9e
	};

	// flags
	enum
	{
		STATUS_THRESHOLD_3        = 1 << 0, // Battery fully charged
		STATUS_LID_OPEN           = 1 << 1, // Lid open
		STATUS_THRESHOLD_2        = 1 << 2, // Battery flat [shutdown now]
		STATUS_THRESHOLD_1        = 1 << 3, // Battery low [warn user]
		STATUS_CHARGING_FAULT     = 1 << 4, // Fault in charging system
		STATUS_CHARGE_STATE_KNOWN = 1 << 5, // Charge state known
		STATUS_BATTERY_PRESENT    = 1 << 6, // Battery present
		STATUS_CHARGER_PRESENT    = 1 << 7  // Charger present
	};

	uint8_t m_estimate;
};

// device type definition
DECLARE_DEVICE_TYPE(ACORN_BMU, acorn_bmu_device)

#endif // MAME_MACHINE_ACORN_BMU_H
