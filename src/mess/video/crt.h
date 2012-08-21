/*************************************************************************

    video/crt.h

    CRT video emulation for TX-0 and PDP-1

*************************************************************************/

#ifndef CRT_H_
#define CRT_H_


/*----------- defined in video/crt.c -----------*/

typedef struct _crt_interface crt_interface;
struct _crt_interface
{
	int num_levels;
	int offset_x, offset_y;
	int width, height;
};

void crt_plot(device_t *device, int x, int y);
void crt_eof(device_t *device);
void crt_update(device_t *device, bitmap_ind16 &bitmap);

DECLARE_LEGACY_DEVICE(CRT, crt);

#define MCFG_CRT_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, CRT, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#endif /* CRT_H_ */
