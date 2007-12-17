#ifndef CUSTOM_H
#define CUSTOM_H

struct CustomSound_interface
{
	void *(*start)(int clock, const struct CustomSound_interface *config);
	void (*stop)(void *token);
	void (*reset)(void *token);
	void *extra_data;
};

void *custom_get_token(int index);


#endif
