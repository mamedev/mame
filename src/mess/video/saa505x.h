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

class saa505x_device : public device_t
{
public:
	saa505x_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~saa505x_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
private:
	// internal state
	void *m_token;
};

extern const device_type SAA505X;


#define MCFG_SAA505X_VIDEO_ADD(_tag, _intf) \
	MCFG_DEVICE_ADD(_tag, SAA505X, 0) \
	MCFG_DEVICE_CONFIG(_intf)
