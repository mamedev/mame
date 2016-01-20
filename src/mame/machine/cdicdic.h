// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************


    CD-i Mono-I CDIC MCU simulation
    -------------------

    written by Ryan Holtz


*******************************************************************************

STATUS:

- Just enough for the Mono-I CD-i board to work somewhat properly.

TODO:

- Decapping and proper emulation.

*******************************************************************************/

#pragma once

#ifndef __CDICDIC_H__
#define __CDICDIC_H__

#include "emu.h"
#include "cdrom.h"
#include "sound/cdda.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_CDICDIC_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, MACHINE_CDICDIC, 0)
#define MCFG_CDICDIC_REPLACE(_tag) \
	MCFG_DEVICE_REPLACE(_tag, MACHINE_CDICDIC, 0)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cdicdic_device

class cdicdic_device : public device_t
{
public:
	// construction/destruction
	cdicdic_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// non-static internal members
	void sample_trigger();
	void process_delayed_command();

	DECLARE_READ16_MEMBER( regs_r );
	DECLARE_WRITE16_MEMBER( regs_w );
	DECLARE_READ16_MEMBER( ram_r );
	DECLARE_WRITE16_MEMBER( ram_w );


protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// internal callbacks
	TIMER_CALLBACK_MEMBER( audio_sample_trigger );
	TIMER_CALLBACK_MEMBER( trigger_readback_int );

private:
	// internal state
	UINT16 m_command;           // CDIC Command Register            (0x303c00)
	UINT32 m_time;              // CDIC Time Register               (0x303c02)
	UINT16 m_file;              // CDIC File Register               (0x303c06)
	UINT32 m_channel;           // CDIC Channel Register            (0x303c08)
	UINT16 m_audio_channel;     // CDIC Audio Channel Register      (0x303c0c)

	UINT16 m_audio_buffer;      // CDIC Audio Buffer Register       (0x303ff4)
	UINT16 m_x_buffer;          // CDIC X-Buffer Register           (0x303ff6)
	UINT16 m_dma_control;       // CDIC DMA Control Register        (0x303ff8)
	UINT16 m_z_buffer;          // CDIC Z-Buffer Register           (0x303ffa)
	UINT16 m_interrupt_vector;  // CDIC Interrupt Vector Register   (0x303ffc)
	UINT16 m_data_buffer;       // CDIC Data Buffer Register        (0x303ffe)

	emu_timer *m_interrupt_timer;
	cdrom_file *m_cd;

	emu_timer *m_audio_sample_timer;
	INT32 m_audio_sample_freq;
	INT32 m_audio_sample_size;

	UINT16 m_decode_addr;
	UINT8 m_decode_delay;
	attotime m_decode_period;

	int m_xa_last[4];
	std::unique_ptr<UINT16[]> m_ram;

	// static internal members
	static void decode_xa_mono(INT32 *cdic_xa_last, const UINT8 *xa, INT16 *dp);
	static void decode_xa_mono8(INT32 *cdic_xa_last, const UINT8 *xa, INT16 *dp);
	static void decode_xa_stereo(INT32 *cdic_xa_last, const UINT8 *xa, INT16 *dp);
	static void decode_xa_stereo8(INT32 *cdic_xa_last, const UINT8 *xa, INT16 *dp);

	static const INT32 s_cdic_adpcm_filter_coef[5][2];

	// non-static internal members
	UINT32 increment_cdda_frame_bcd(UINT32 bcd);
	UINT32 increment_cdda_sector_bcd(UINT32 bcd);
	void decode_audio_sector(const UINT8 *xa, INT32 triggered);
};

// device type definition
extern const device_type MACHINE_CDICDIC;

#endif // __CDICDIC_H__
