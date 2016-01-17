// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef __MSX_SLOT_MUSIC_H
#define __MSX_SLOT_MUSIC_H


#include "bus/msx_slot/slot.h"
#include "bus/msx_slot/rom.h"
#include "sound/2413intf.h"


extern const device_type MSX_SLOT_MUSIC;


#define MCFG_MSX_SLOT_MUSIC_ADD(_tag, _startpage, _numpages, _region, _offset, _ym2413_tag) \
	MCFG_MSX_INTERNAL_SLOT_ADD(_tag, MSX_SLOT_MUSIC, _startpage, _numpages) \
	msx_slot_rom_device::set_rom_start(*device, _region, _offset); \
	msx_slot_music_device::set_ym2413_tag(*device, _ym2413_tag);
class msx_slot_music_device : public msx_slot_rom_device
{
public:
	msx_slot_music_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	static void set_ym2413_tag(device_t &device, std::string tag) { dynamic_cast<msx_slot_music_device &>(device).m_ym2413_tag = tag; }

	virtual void device_start() override;

	virtual DECLARE_READ8_MEMBER(read) override;

	DECLARE_WRITE8_MEMBER(write_ym2413);

private:
	ym2413_device *m_ym2413;
	std::string m_ym2413_tag;
};


#endif
