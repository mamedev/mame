// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    TI-99/8 Speech Synthesizer
    See speech8.c for documentation

    Michael Zapf
    October 2010
    February 2012: Rewritten as class

*****************************************************************************/

#ifndef __TISPEECH8__
#define __TISPEECH8__

#include "emu.h"
#include "ti99defs.h"
#include "sound/tms5220.h"

extern const device_type TI99_SPEECH8;

#define MCFG_SPEECH8_READY_CALLBACK(_write) \
	devcb = &ti998_spsyn_device::set_ready_wr_callback(*device, DEVCB_##_write);

class ti998_spsyn_device : public bus8z_device
{
public:
	ti998_spsyn_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_ready_wr_callback(device_t &device, _Object object) { return downcast<ti998_spsyn_device &>(device).m_ready.set_callback(object); }

	DECLARE_READ8Z_MEMBER(readz);
	DECLARE_WRITE8_MEMBER(write);

	DECLARE_READ8Z_MEMBER(crureadz) { };
	DECLARE_WRITE8_MEMBER(cruwrite) { };

	DECLARE_WRITE_LINE_MEMBER( speech8_ready );

	DECLARE_READ8_MEMBER( spchrom_read );
	DECLARE_WRITE8_MEMBER( spchrom_load_address );
	DECLARE_WRITE8_MEMBER( spchrom_read_and_branch );

protected:
	virtual void    device_start();
	virtual void    device_reset(void);
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;

private:
	tms5220_device  *m_vsp;
//  UINT8           *m_speechrom;           // pointer to speech ROM data
//  int             m_load_pointer;         // which 4-bit nibble will be affected by load address
//  int             m_rombits_count;        // current bit position in ROM
//  UINT32          m_sprom_address;        // 18 bit pointer in ROM
//  UINT32          m_sprom_length;         // length of data pointed by speechrom_data, from 0 to 2^18

	// Ready line to the CPU
	devcb_write_line m_ready;
};

#define MCFG_TISPEECH8_ADD(_tag, _conf)     \
	MCFG_DEVICE_ADD(_tag, TI99_SPEECH8, 0)  \
	MCFG_DEVICE_CONFIG(_conf)

#endif
