#ifndef CUSTOM_H
#define CUSTOM_H

typedef struct _custom_sound_interface custom_sound_interface;
struct _custom_sound_interface
{
	void *(*start)(int clock, const custom_sound_interface *config);
	void (*stop)(void *token);
	void (*reset)(void *token);
	void *extra_data;
};

void *custom_get_token(int index);


#endif
