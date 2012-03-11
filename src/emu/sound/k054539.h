/*********************************************************

    Konami 054539 PCM Sound Chip

*********************************************************/

#pragma once

#ifndef __K054539_H__
#define __K054539_H__

#define MCFG_K054539_ADD(_tag, _clock, _interface) \
	MCFG_DEVICE_ADD(_tag, K054539, _clock) \
	k054539_device::static_set_interface(*device, _interface); \

struct k054539_interface
{
	const char *rgnoverride;
	void (*apan)(device_t *, double, double);	/* Callback for analog output mixing levels (0..1 for each channel) */
	void (*irq)(device_t *);
};


//* control flags, may be set at DRIVER_INIT().
#define K054539_RESET_FLAGS     0
#define K054539_REVERSE_STEREO  1
#define K054539_DISABLE_REVERB  2
#define K054539_UPDATE_AT_KEYON 4

void k054539_init_flags(device_t *device, int flags);

void k054539_set_gain(device_t *device, int channel, double gain);

class k054539_device : public device_t,
					   public device_sound_interface,
					   public k054539_interface
{
public:
	enum {
		RESET_FLAGS     = 0,
		REVERSE_STEREO  = 1,
		DISABLE_REVERB  = 2,
		UPDATE_AT_KEYON = 4
	};

	// construction/destruction
	k054539_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	static void static_set_interface(device_t &device, const k054539_interface &interface);

	DECLARE_WRITE8_MEMBER(write);
	DECLARE_READ8_MEMBER(read);

	void init_flags(int flags);

	/*
      Note that the eight PCM channels of a K054539 do not have separate
      volume controls. Considering the global attenuation equation may not
      be entirely accurate, k054539_set_gain() provides means to control
      channel gain. It can be called anywhere but preferrably from
      DRIVER_INIT().

      Parameters:
          channel : 0 - 7
          gain    : 0.0=silent, 1.0=no gain, 2.0=twice as loud, etc.
    */
	void set_gain(int channel, double gain);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

private:
	struct channel {
		UINT32 pos;
		UINT32 pfrac;
		INT32 val;
		INT32 pval;
	};

	double voltab[256];
	double pantab[0xf];

	double gain[8];
	UINT8 posreg_latch[8][3];
	int flags;

	unsigned char regs[0x230];
	unsigned char *ram;
	int reverb_pos;

	INT32 cur_ptr;
	int cur_limit;
	unsigned char *cur_zone;
	unsigned char *rom;
	UINT32 rom_size;
	UINT32 rom_mask;

	channel channels[8];
	sound_stream *stream;

	bool regupdate();
	void keyon(int channel);
	void keyoff(int channel);
	void init_chip();
	void reset_zones();
};

extern const device_type K054539;

#endif /* __K054539_H__ */
