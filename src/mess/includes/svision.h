/*****************************************************************************
 *
 * includes/svision.h
 *
 ****************************************************************************/

#ifndef SVISION_H_
#define SVISION_H_

struct svision_t
{
	emu_timer *timer1;
	int timer_shot;
};

struct svision_pet_t
{
	int state;
	int on, clock, data;
	UINT8 input;
	emu_timer *timer;
};

struct tvlink_t
{
	UINT32 palette[4/*0x40?*/]; /* rgb8 */
	int palette_on;
};


class svision_state : public driver_device
{
public:
	svision_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_reg(*this, "reg"),
		m_videoram(*this, "videoram"){ }

	required_shared_ptr<UINT8> m_reg;
	required_shared_ptr<UINT8> m_videoram;
	device_t *m_sound;
	int *m_dma_finished;
	svision_t m_svision;
	svision_pet_t m_pet;
	tvlink_t m_tvlink;
	DECLARE_READ8_MEMBER(svision_r);
	DECLARE_WRITE8_MEMBER(svision_w);
	DECLARE_READ8_MEMBER(tvlink_r);
	DECLARE_WRITE8_MEMBER(tvlink_w);
	DECLARE_DRIVER_INIT(svisions);
	DECLARE_DRIVER_INIT(svision);
	virtual void machine_reset();
	virtual void palette_init();
	DECLARE_PALETTE_INIT(svisionp);
	DECLARE_PALETTE_INIT(svisionn);
	DECLARE_MACHINE_RESET(tvlink);
	UINT32 screen_update_svision(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_tvlink(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(svision_frame_int);
	TIMER_CALLBACK_MEMBER(svision_pet_timer);
	TIMER_CALLBACK_MEMBER(svision_timer);
	TIMER_DEVICE_CALLBACK_MEMBER(svision_pet_timer_dev);
};


/*----------- defined in drivers/svision.c -----------*/

void svision_irq( running_machine &machine );


/*----------- defined in audio/svision.c -----------*/

class svision_sound_device : public device_t,
                                  public device_sound_interface
{
public:
	svision_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~svision_sound_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
private:
	// internal state
	void *m_token;
};

extern const device_type SVISION;


int *svision_dma_finished(device_t *device);
void svision_sound_decrement(device_t *device);
void svision_soundport_w(device_t *device, int which, int offset, int data);
DECLARE_WRITE8_DEVICE_HANDLER( svision_sounddma_w );
DECLARE_WRITE8_DEVICE_HANDLER( svision_noise_w );


#endif /* SVISION_H_ */
