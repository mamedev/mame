// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Speculator

    A Spectrum emulator

***************************************************************************/

#ifndef MAME_BUS_EINSTEIN_PIPE_SPECULATOR_H
#define MAME_BUS_EINSTEIN_PIPE_SPECULATOR_H

#pragma once

#include "pipe.h"
#include "machine/74123.h"
#include "imagedev/cassette.h"
#include "sound/spkrdev.h"
#include "speaker.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> einstein_speculator_device

class einstein_speculator_device : public device_t, public device_tatung_pipe_interface
{
public:
	// construction/destruction
	einstein_speculator_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void int_w(int state) override;

	uint8_t ram_r(offs_t offset);
	void ram_w(offs_t offset, uint8_t data);
	uint8_t tape_r();
	void nmi_w(uint8_t data);

	void ic5a_q_w(int state);
	void ic5b_q_w(int state);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	offs_t address_translate(offs_t offset);

	required_device<ttl74123_device> m_ic5a;
	required_device<ttl74123_device> m_ic5b;
	required_device<cassette_image_device> m_cassette;
	required_device<speaker_sound_device> m_speaker;

	std::unique_ptr<uint8_t[]> m_ram;

	int m_nmisel;
};

// device type definition
DECLARE_DEVICE_TYPE(EINSTEIN_SPECULATOR, einstein_speculator_device)

#endif // MAME_BUS_EINSTEIN_PIPE_SPECULATOR_H
