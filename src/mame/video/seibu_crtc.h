/***************************************************************************

Template for skeleton device

***************************************************************************/

#pragma once

#ifndef __SEIBU_CRTCDEV_H__
#define __SEIBU_CRTCDEV_H__

struct seibu_crtc_interface
{
	const char         *m_screen_tag;
//	TODO: layer enable, scrolling
};

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_SEIBU_CRTC_ADD(_tag,_config,_freq) \
	MCFG_DEVICE_ADD(_tag, SEIBU_CRTC, _freq) \
	MCFG_DEVICE_CONFIG(_config)

#define SEIBU_CRTC_INTERFACE(name) \
	const seibu_crtc_interface (name) =


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> seibu_crtc_device

class seibu_crtc_device : public device_t,
                          public seibu_crtc_interface
{
public:
	// construction/destruction
	seibu_crtc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// I/O operations
	DECLARE_WRITE16_MEMBER( write );
	DECLARE_READ16_MEMBER( read );

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const;
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

	screen_device *m_screen;

private:
	const address_space_config      m_space_config;
	inline UINT16 read_word(offs_t address);
	inline void write_word(offs_t address, UINT16 data);
};


// device type definition
extern const device_type SEIBU_CRTC;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif
