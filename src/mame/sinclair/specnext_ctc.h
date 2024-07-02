// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
#ifndef MAME_SINCLAIR_SPECNEXT_CTC_H
#define MAME_SINCLAIR_SPECNEXT_CTC_H

#pragma once

#include "machine/z80ctc.h"

class specnext_ctc_device : public z80ctc_device
{

public:
	specnext_ctc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual int z80daisy_irq_ack() override;

};

DECLARE_DEVICE_TYPE(SPECNEXT_CTC, specnext_ctc_device)
#endif // MAME_SINCLAIR_SPECNEXT_CTC_H
