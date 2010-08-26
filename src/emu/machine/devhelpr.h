/**********************************************************************

    Generic modern device trampolines and other helpers, to minimize
    the amount of redundant copy/pasting between modernized devices.

**********************************************************************/

#ifndef __DEVHELPR_H__
#define __DEVHELPR_H__

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

#define GENERIC_DEVICE_CONFIG_SETUP(devname, devtag) \
	devname##_device_config::devname##_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock) \
	    : device_config(mconfig, static_alloc_device_config, devtag, tag, owner, clock) \
	{ } \
	\
	device_config *devname##_device_config::static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock) \
	{ return global_alloc(devname##_device_config(mconfig, tag, owner, clock)); } \
	\
	device_t *devname##_device_config::alloc_device(running_machine &machine) const \
	{ return auto_alloc(&machine, devname##_device(machine, *this)); }


#endif // __DEVHELPR_H__