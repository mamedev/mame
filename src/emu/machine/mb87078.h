/*****************************************************************************

  MB87078 6-bit,4-channel electronic volume controller emulator


*****************************************************************************/

#ifndef __MB87078_H__
#define __MB87078_H__

#include "devlegcy.h"



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef void (*mb87078_gain_changed_cb)(running_machine &machine, int channel, int percent /*, float decibels*/);

struct mb87078_interface
{
	mb87078_gain_changed_cb   gain_changed_cb;
};

class mb87078_device : public device_t
{
public:
	mb87078_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~mb87078_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
private:
	// internal state
	void *m_token;
};

extern const device_type MB87078;


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_MB87078_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, MB87078, 0) \
	MCFG_DEVICE_CONFIG(_interface)


/***************************************************************************
    DEVICE I/O FUNCTIONS
***************************************************************************/

void mb87078_data_w(device_t *device, int data, int dsel);
void mb87078_reset_comp_w(device_t *device, int level);


/* mb87078_gain_decibel_r will return 'channel' gain on the device.
   Returned value represents channel gain expressed in decibels,
   Range from 0 to -32.0 (or -256.0 for -infinity) */
float mb87078_gain_decibel_r(device_t *device, int channel);


/* mb87078_gain_percent_r will return 'channel' gain on the device.
   Returned value represents channel gain expressed in percents of maximum volume.
   Range from 100 to 0. (100 = 0dB; 50 = -6dB; 0 = -infinity)
   This function is designed for use with MAME mixer_xxx() functions. */
int   mb87078_gain_percent_r(device_t *device, int channel);


#endif	/* __MB87078_H__ */
