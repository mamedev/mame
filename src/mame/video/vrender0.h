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
 FUNCTION PROTOTYPES
 ***************************************************************************/

DEVICE_GET_INFO( vr0video );


/***************************************************************************
 DEVICE CONFIGURATION MACROS
 ***************************************************************************/

#define VIDEO_VRENDER0 DEVICE_GET_INFO_NAME( vr0video )

#define MDRV_VIDEO_VRENDER0_ADD(_tag, _interface) \
MDRV_DEVICE_ADD(_tag, VIDEO_VRENDER0, 0) \
MDRV_DEVICE_CONFIG(_interface)


/***************************************************************************
 DEVICE I/O FUNCTIONS
 ***************************************************************************/

extern int vrender0_ProcessPacket(running_device *device, UINT32 PacketPtr, UINT16 *Dest, UINT8 *TEXTURE);

#endif /* __VR0VIDEO_H__ */
