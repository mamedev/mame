/************************************************************************
    saa505x

    MESS Driver By:

    Gordon Jefferyes
    mess_bbc@gjeffery.dircon.co.uk

 ************************************************************************/


typedef struct _saa505x_interface saa505x_interface;
struct _saa505x_interface
{
	void (*out_Pixel_func)(device_t *device, int offset, int data);
};

void teletext_DEW(device_t *device);
void teletext_LOSE_w(device_t *device, int offset, int data);
void teletext_data_w(device_t *device, int offset, int data);
void teletext_F1(device_t *device);

DECLARE_LEGACY_DEVICE(SAA505X, saa505x);

#define MCFG_SAA505X_VIDEO_ADD(_tag, _intf) \
	MCFG_DEVICE_ADD(_tag, SAA505X, 0) \
	MCFG_DEVICE_CONFIG(_intf)
