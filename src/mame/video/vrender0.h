#ifndef __VR0VIDEO_H__
#define __VR0VIDEO_H__

#include "devcb.h"

/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/

typedef struct _vr0video_interface vr0video_interface;
struct _vr0video_interface
{
	const char *cpu;
};

/***************************************************************************
 DEVICE CONFIGURATION MACROS
 ***************************************************************************/

class vr0video_device : public device_t
{
public:
	vr0video_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~vr0video_device() { global_free(m_token); }

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

extern const device_type VIDEO_VRENDER0;


#define MCFG_VIDEO_VRENDER0_ADD(_tag, _interface) \
MCFG_DEVICE_ADD(_tag, VIDEO_VRENDER0, 0) \
MCFG_DEVICE_CONFIG(_interface)


/***************************************************************************
 DEVICE I/O FUNCTIONS
 ***************************************************************************/

int vrender0_ProcessPacket(device_t *device, UINT32 PacketPtr, UINT16 *Dest, UINT8 *TEXTURE);

#endif /* __VR0VIDEO_H__ */
