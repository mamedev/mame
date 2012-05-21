/**********************************************************************

    Generic modern device trampolines and other helpers, to minimize
    the amount of redundant copy/pasting between modernized devices.

**********************************************************************/

#ifndef __DEVHELPR_H__
#define __DEVHELPR_H__

#define READ32_DEVICE_HANDLER_TRAMPOLINE(devname, funcname) \
	READ32_DEVICE_HANDLER( funcname ) \
	{ return downcast<devname##_device*>(device)->funcname(offset); } \
	UINT32 devname##_device::funcname(UINT32 offset)

#define WRITE32_DEVICE_HANDLER_TRAMPOLINE(devname, funcname) \
	WRITE32_DEVICE_HANDLER( funcname ) \
	{ downcast<devname##_device*>(device)->funcname(offset, data, mem_mask); } \
	void devname##_device::funcname(UINT32 offset, UINT32 data, UINT32 mem_mask)

#define READ8_DEVICE_HANDLER_TRAMPOLINE(devname, funcname) \
	READ8_DEVICE_HANDLER( funcname ) \
	{ return downcast<devname##_device*>(device)->funcname(offset); } \
	UINT8 devname##_device::funcname(UINT32 offset)

#define WRITE8_DEVICE_HANDLER_TRAMPOLINE(devname, funcname) \
	WRITE8_DEVICE_HANDLER( funcname ) \
	{ downcast<devname##_device*>(device)->funcname(offset, data); } \
	void devname##_device::funcname(UINT32 offset, UINT8 data)

#define READ_LINE_DEVICE_HANDLER_TRAMPOLINE(devname, funcname) \
	READ_LINE_DEVICE_HANDLER( funcname ) \
	{ return downcast<devname##_device *>(device)->funcname(); } \
	void devname##_device::funcname()

#define WRITE_LINE_DEVICE_HANDLER_TRAMPOLINE(devname, funcname) \
	WRITE_LINE_DEVICE_HANDLER( funcname ) \
	{ return downcast<devname##_device *>(device)->funcname(state); } \
	void devname##_device::funcname(UINT8 state)

#endif // __DEVHELPR_H__
