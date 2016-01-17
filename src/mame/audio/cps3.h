// license:BSD-3-Clause
// copyright-holders:David Haywood, Andreas Naive, Tomasz Slanina, ElSemi
/***************************************************************************

    Capcom CPS-3 Sound Hardware

****************************************************************************/

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

struct cps3_voice
{
	cps3_voice() :
		pos(0),
		frac(0)
	{
		memset(regs, 0, sizeof(UINT32)*8);
	}

	UINT32 regs[8];
	UINT32 pos;
	UINT32 frac;
};

// ======================> cps3_sound_device

class cps3_sound_device : public device_t,
							public device_sound_interface
{
public:
	cps3_sound_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	~cps3_sound_device() { }

	void set_base(INT8* base) { m_base = base; }

	DECLARE_WRITE32_MEMBER( cps3_sound_w );
	DECLARE_READ32_MEMBER( cps3_sound_r );

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	sound_stream *m_stream;
	cps3_voice m_voice[16];
	UINT16     m_key;
	INT8*      m_base;
};

extern const device_type CPS3;
