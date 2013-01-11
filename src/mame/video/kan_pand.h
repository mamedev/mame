/*************************************************************************

    kan_pand.h

    Implementation of Kaneko Pandora sprite chip

**************************************************************************/

#ifndef __KAN_PAND_H__
#define __KAN_PAND_H__

#include "devlegcy.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct kaneko_pandora_interface
{
	const char *screen;
	UINT8      gfx_region;
	int        x;
	int        y;
};

class kaneko_pandora_device : public device_t
{
public:
	kaneko_pandora_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~kaneko_pandora_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
private:
	// internal state
	void *m_token;
};

extern const device_type KANEKO_PANDORA;


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_KANEKO_PANDORA_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, KANEKO_PANDORA, 0) \
	MCFG_DEVICE_CONFIG(_interface)


/***************************************************************************
    DEVICE I/O FUNCTIONS
***************************************************************************/

void pandora_update(device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect);
void pandora_eof(device_t *device);
void pandora_set_clear_bitmap(device_t *device, int clear);
void pandora_set_bg_pen( device_t *device, int pen );

DECLARE_WRITE8_DEVICE_HANDLER ( pandora_spriteram_w );
DECLARE_READ8_DEVICE_HANDLER( pandora_spriteram_r );

DECLARE_WRITE16_DEVICE_HANDLER( pandora_spriteram_LSB_w );
DECLARE_READ16_DEVICE_HANDLER( pandora_spriteram_LSB_r );

#endif /* __KAN_PAND_H__ */
