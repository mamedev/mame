/*************************************************************************

    video/crt.h

    CRT video emulation for TX-0 and PDP-1

*************************************************************************/

#ifndef CRT_H_
#define CRT_H_


/*----------- defined in video/crt.c -----------*/

struct crt_interface
{
	int num_levels;
	int offset_x, offset_y;
	int width, height;
};

void crt_plot(device_t *device, int x, int y);
void crt_eof(device_t *device);
void crt_update(device_t *device, bitmap_ind16 &bitmap);

class crt_device : public device_t
{
public:
	crt_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~crt_device() { global_free(m_token); }

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

extern const device_type CRT;


#define MCFG_CRT_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, CRT, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#endif /* CRT_H_ */
