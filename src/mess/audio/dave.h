/**********************************************************************

    "Dave" Sound Chip

**********************************************************************/

#ifndef __DAVE_H__
#define __DAVE_H__

#include "devcb.h"


#define DAVE_INT_SELECTABLE     0
#define DAVE_INT_1KHZ_50HZ_TG   1
#define DAVE_INT_1HZ            2
#define DAVE_INT_INT1           3
#define DAVE_INT_INT2           4

#define DAVE_FIFTY_HZ_COUNTER_RELOAD    20
#define DAVE_ONE_HZ_COUNTER_RELOAD      1000

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
	devcb_read8 m_reg_r_cb;
	devcb_write8 m_reg_w_cb;
	devcb_write_line m_int_cb;
};


/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

class dave_sound_device : public device_t,
							public device_sound_interface,
							public dave_interface
{
public:
	dave_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~dave_sound_device() {}

	DECLARE_WRITE8_MEMBER(reg_w);
	DECLARE_READ8_MEMBER(reg_r);
	void set_reg(offs_t offset, UINT8 data);

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

private:
	// internal state
	devcb_resolved_read8 m_reg_r;
	devcb_resolved_write8 m_reg_w;
	devcb_resolved_write_line m_int_callback;

	TIMER_CALLBACK_MEMBER(dave_1khz_callback);
	DECLARE_WRITE8_MEMBER(sound_w);
	void refresh_ints();
	void refresh_selectable_int();
	void set_external_int_state(int int_id, int state);
	
	UINT8 m_regs[32];
	
	/* int latches (used by 1Hz, int1 and int2) */
	UINT32 m_int_latch;
	/* int enables */
	UINT32 m_int_enable;
	/* int inputs */
	UINT32 m_int_input;
	
	UINT32 m_int_irq;
	
	/* INTERRUPTS */
	
	/* internal timer */
	/* bit 2: 1kHz timer irq */
	/* bit 1: 50kHz timer irq */
	int m_timer_irq;
	/* 1khz timer - divided into 1kHz, 50Hz and 1Hz timer */
	emu_timer   *m_int_timer;
	/* state of 1kHz timer */
	UINT32 m_one_khz_state;
	/* state of 50Hz timer */
	UINT32 m_fifty_hz_state;
	
	/* counter used to trigger 50Hz from 1kHz timer */
	UINT32 m_fifty_hz_count;
	/* counter used to trigger 1Hz from 1kHz timer */
	UINT32 m_one_hz_count;
	
	
	/* SOUND SYNTHESIS */
	int m_period[4];
	int m_count[4];
	int m_level[4];
	
	/* these are used to force channels on/off */
	/* if one of the or values is 0x0ff, this means
	 the volume will be forced on,else it is dependant on
	 the state of the wave */
	int m_level_or[8];
	/* if one of the values is 0x00, this means the
	 volume is forced off, else it is dependant on the wave */
	int m_level_and[8];
	
	/* these are the current channel volumes in MAME form */
	int m_mame_volumes[8];
	
	/* update step */
	int m_update_step;
	
	sound_stream *m_sound_stream_var;
	
	/* temp here */
	int m_nick_virq;
};

extern const device_type DAVE;

#endif /* __DAVE_H__ */
