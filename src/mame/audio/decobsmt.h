// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_AUDIO_DECOBSMT_H
#define MAME_AUDIO_DECOBSMT_H

#pragma once

#include "cpu/m6809/m6809.h"
#include "sound/bsmt2000.h"

#define DECOBSMT_TAG "decobsmt"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class decobsmt_device : public device_t
{
public:
	// construction/destruction
	decobsmt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void bsmt_reset_w(u8 data);
	u8 bsmt_status_r();
	void bsmt0_w(u8 data);
	void bsmt1_w(offs_t offset, u8 data);
	u8 bsmt_comms_r();
	void bsmt_comms_w(u8 data);

	DECLARE_WRITE_LINE_MEMBER(bsmt_reset_line);

	void bsmt_map(address_map &map);
	void decobsmt_map(address_map &map);
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	required_device<cpu_device> m_ourcpu;
	required_device<bsmt2000_device> m_bsmt;

	uint8_t m_bsmt_latch;
	uint8_t m_bsmt_reset;
	uint8_t m_bsmt_comms;

	INTERRUPT_GEN_MEMBER(decobsmt_firq_interrupt);

	void bsmt_ready_callback();
};


// device type definition
DECLARE_DEVICE_TYPE(DECOBSMT, decobsmt_device)

#endif  // MAME_AUDIO_DECOBSMT_H
