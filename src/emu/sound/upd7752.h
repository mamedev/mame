/***************************************************************************

Template for skeleton device

***************************************************************************/

#pragma once

#ifndef __UPD7752DEV_H__
#define __UPD7752DEV_H__



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_UPD7752_ADD(_tag,_freq) \
	MCFG_DEVICE_ADD(_tag, UPD7752, _freq)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> upd7752_device

class upd7752_device : public device_t,
     				   public device_sound_interface,
     				   public device_memory_interface
{
public:
	// construction/destruction
	upd7752_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// I/O operations
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( read );
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const;

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_stop();
	virtual void device_reset();

	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

private:
	sound_stream  *m_stream;
	const address_space_config      m_space_config;
};


// device type definition
extern const device_type UPD7752;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif
