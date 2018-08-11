// license:BSD-3-Clause
// copyright-holders:superctr, Valley Bell
/*********************************************************

    Capcom System QSound™

*********************************************************/
#ifndef MAME_SOUND_QSOUNDBASE_H
#define MAME_SOUND_QSOUNDBASE_H

#pragma once

class qsound_base_device : public device_t, public device_sound_interface, public device_rom_interface
{
public:
	qsound_base_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock);

protected:
	tiny_rom_entry const *device_rom_region() const override;
};

#endif // MAME_SOUND_QSOUNDBASE_H
