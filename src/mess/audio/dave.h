/**********************************************************************

    "Dave" Sound Chip

**********************************************************************/

#ifndef __DAVE_H__
#define __DAVE_H__

#include "devcb.h"


/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

class dave_sound_device : public device_t,
                                  public device_sound_interface
{
public:
	dave_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~dave_sound_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
private:
	// internal state
	void *m_token;
};

extern const device_type DAVE;


#define DAVE_INT_SELECTABLE		0
#define DAVE_INT_1KHZ_50HZ_TG	1
#define DAVE_INT_1HZ			2
#define DAVE_INT_INT1			3
#define DAVE_INT_INT2			4

#define DAVE_FIFTY_HZ_COUNTER_RELOAD	20
#define DAVE_ONE_HZ_COUNTER_RELOAD		1000

/* id's of external ints */
enum
{
	DAVE_INT1_ID,
	DAVE_INT2_ID
};


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct dave_interface
{
	devcb_read8 reg_r;
	devcb_write8 reg_w;
	devcb_write_line int_callback;
};


/***************************************************************************
    PROTOTYPES
***************************************************************************/
void dave_set_reg(device_t *device, offs_t offset, UINT8 data);

DECLARE_READ8_DEVICE_HANDLER ( dave_reg_r );
DECLARE_WRITE8_DEVICE_HANDLER ( dave_reg_w );

#endif /* __DAVE_H__ */
