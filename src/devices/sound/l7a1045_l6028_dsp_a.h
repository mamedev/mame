// license:LGPL-2.1+
// copyright-holders:David Haywood, Angelo Salese, ElSemi
//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

struct l7a1045_voice
{
	l7a1045_voice() : end(0), mode(false),
	                  pos(0),
		frac(0), l_volume(0), r_volume(0)
	{
		//memset(regs, 0, sizeof(UINT32)*8);
		start = 0;
	}

	UINT32 start;
	UINT32 end;
	bool mode;
	UINT32 pos;
	UINT32 frac;
	UINT16 l_volume;
	UINT16 r_volume;
};

// ======================> l7a1045_sound_device

class l7a1045_sound_device : public device_t,
							public device_sound_interface
{
public:
	l7a1045_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~l7a1045_sound_device() { }

//  void set_base(INT8* base) { m_base = base; }

	DECLARE_WRITE16_MEMBER( l7a1045_sound_w );
	DECLARE_READ16_MEMBER( l7a1045_sound_r );

protected:
	// device-level overrides
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

private:
	sound_stream *m_stream;
	l7a1045_voice m_voice[32];
	UINT32    m_key;
	UINT8 *m_rom;
	INT32 m_rom_size;

	UINT8 m_audiochannel;
	UINT8 m_audioregister;

	struct l7a1045_48bit_data {
		UINT16 dat[3];
	};

	l7a1045_48bit_data m_audiodat[0x10][0x20];

	DECLARE_WRITE16_MEMBER(sound_select_w);
	DECLARE_WRITE16_MEMBER(sound_data_w);
	DECLARE_READ16_MEMBER(sound_data_r);
	DECLARE_WRITE16_MEMBER(sound_status_w);
};

extern const device_type L7A1045;
