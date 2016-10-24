// license:BSD-3-Clause
// copyright-holders:R. Belmont
#pragma once

#ifndef __DECOBSMT_H__
#define __DECOBSMT_H__

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "sound/bsmt2000.h"

#define DECOBSMT_TAG "decobsmt"

#define MCFG_DECOBSMT_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, DECOBSMT, 0)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class decobsmt_device : public device_t
{
public:
		// construction/destruction
		decobsmt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

		void bsmt_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
		uint8_t bsmt_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
		void bsmt0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
		void bsmt1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
		uint8_t bsmt_comms_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
		void bsmt_comms_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

		void bsmt_reset_line(int state);

		void decobsmt_firq_interrupt(device_t &device);

		required_device<cpu_device> m_ourcpu;
		required_device<bsmt2000_device> m_bsmt;

protected:
		// device-level overrides
		virtual void device_start() override;
		virtual void device_reset() override;
		virtual machine_config_constructor device_mconfig_additions() const override;

private:
		uint8_t m_bsmt_latch;
		uint8_t m_bsmt_reset;
		uint8_t m_bsmt_comms;
};


// device type definition
extern const device_type DECOBSMT;

#endif  /* __DECOBSMT_H__ */
