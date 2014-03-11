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

class svision_sound_device; // defined below

class svision_state : public driver_device
{
public:
	svision_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_reg(*this, "reg"),
		m_videoram(*this, "videoram"),
		m_joy(*this, "JOY"),
		m_joy2(*this, "JOY2"),
		m_palette(*this, "palette")  { }

	svision_sound_device *m_sound;
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
	DECLARE_PALETTE_INIT(svision);
	DECLARE_PALETTE_INIT(svisionp);
	DECLARE_PALETTE_INIT(svisionn);
	DECLARE_MACHINE_RESET(tvlink);
	UINT32 screen_update_svision(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_tvlink(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(svision_frame_int);
	TIMER_CALLBACK_MEMBER(svision_pet_timer);
	TIMER_CALLBACK_MEMBER(svision_timer);
	TIMER_DEVICE_CALLBACK_MEMBER(svision_pet_timer_dev);
	void svision_irq();
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(svision_cart);

protected:
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<UINT8> m_reg;
	required_shared_ptr<UINT8> m_videoram;
	required_ioport m_joy;
	optional_ioport m_joy2;
	required_device<palette_device> m_palette;

	memory_region *m_user1;
	memory_bank *m_bank1;
	memory_bank *m_bank2;
};


/*----------- defined in drivers/svision.c -----------*/

void svision_irq( running_machine &machine );


/*----------- defined in audio/svision.c -----------*/


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

enum SVISION_NOISE_Type
{
	SVISION_NOISE_Type7Bit,
	SVISION_NOISE_Type14Bit
};

struct SVISION_NOISE
{
	SVISION_NOISE() :
		on(0),
		right(0),
		left(0),
		play(0),
		type(SVISION_NOISE_Type7Bit),
		state(0),
		volume(0),
		count(0),
		step(0.0),
		pos(0.0),
		value(0)
	{
		memset(reg, 0, sizeof(UINT8)*3);
	}

	UINT8 reg[3];
	int on, right, left, play;
	SVISION_NOISE_Type type;
	int state;
	int volume;
	int count;
	double step, pos;
	int value; // currently simple random function
};

struct SVISION_DMA
{
	SVISION_DMA() :
		on(0),
		right(0),
		left(0),
		ca14to16(0),
		start(0),
		size(0),
		pos(0.0),
		step(0.0),
		finished(0)
	{
		memset(reg, 0, sizeof(UINT8)*5);
	}

	UINT8 reg[5];
	int on, right, left;
	int ca14to16;
	int start,size;
	double pos, step;
	int finished;
};

struct SVISION_CHANNEL
{
	SVISION_CHANNEL() :
		on(0),
		waveform(0),
		volume(0),
		pos(0),
		size(0),
		count(0)
	{
		memset(reg, 0, sizeof(UINT8)*4);
	}

	UINT8 reg[4];
	int on;
	int waveform, volume;
	int pos;
	int size;
	int count;
};


// ======================> svision_sound_device

class svision_sound_device : public device_t,
								public device_sound_interface
{
public:
	svision_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~svision_sound_device() { }

protected:
	// device-level overrides
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

public:
	DECLARE_WRITE8_MEMBER( svision_sounddma_w );
	DECLARE_WRITE8_MEMBER( svision_noise_w );

public:
	int *dma_finished();
	void sound_decrement();
	void soundport_w(int which, int offset, int data);

private:
	sound_stream *m_mixer_channel;
	SVISION_DMA m_dma;
	SVISION_NOISE m_noise;
	SVISION_CHANNEL m_channel[2];
};

extern const device_type SVISION;


#endif /* SVISION_H_ */
