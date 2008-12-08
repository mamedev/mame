#pragma once

#ifndef __CUSTOM_H__
#define __CUSTOM_H__

typedef struct _custom_sound_interface custom_sound_interface;
struct _custom_sound_interface
{
	void *(*start)(const device_config *device, int clock, const custom_sound_interface *config);
	void (*stop)(void *token);
	void (*reset)(void *token);
	void *extra_data;
};

void *custom_get_token(int index);

#define CUSTOM_START(name) void *name(const device_config *device, int clock, const custom_sound_interface *config)
#define CUSTOM_STOP(name) void name(void *token)
#define CUSTOM_RESET(name) void name(void *token)

#endif /* __CUSTOM_H__ */
