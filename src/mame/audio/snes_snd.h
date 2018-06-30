// license:LGPL-2.1+
// copyright-holders:R. Belmont, Brad Martin
/*****************************************************************************
 *
 * audio/snes_spc.h
 *
 ****************************************************************************/

#ifndef MAME_AUDIO_SNES_SND_H
#define MAME_AUDIO_SNES_SND_H


/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/

class snes_sound_device : public device_t, public device_sound_interface
{
public:
	snes_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 32'000);

	void set_volume(int volume);

	DECLARE_READ8_MEMBER( spc_io_r );
	DECLARE_READ8_MEMBER( spc_ram_r );
	DECLARE_READ8_MEMBER( spc_port_out );
	DECLARE_WRITE8_MEMBER( spc_io_w );
	DECLARE_WRITE8_MEMBER( spc_ram_w );
	DECLARE_WRITE8_MEMBER( spc_port_in );

//  uint8_t *spc_get_ram() { return m_ram; }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	enum class env_state_t32 : u8
	{
		ATTACK,
		DECAY,
		SUSTAIN,
		RELEASE
	};


	static constexpr unsigned SNES_SPCRAM_SIZE = 0x10000;


	struct voice_state_type                      /* Voice state type             */
	{
		uint16_t        mem_ptr;        /* Sample data memory pointer   */
		int             end;            /* End or loop after block      */
		int             envcnt;         /* Counts to envelope update    */
		env_state_t32   envstate;       /* Current envelope state       */
		int             envx;           /* Last env height (0-0x7FFF)   */
		int             filter;         /* Last header's filter         */
		int             half;           /* Active nybble of BRR         */
		int             header_cnt;     /* Bytes before new header (0-8)*/
		int             mixfrac;        /* Fractional part of smpl pstn */
		int             on_cnt;         /* Is it time to turn on yet?   */
		int             pitch;          /* Sample pitch (4096->32000Hz) */
		int             range;          /* Last header's range          */
		uint32_t        samp_id;        /* Sample ID#                   */
		int             sampptr;        /* Where in sampbuf we are      */
		int32_t         smp1;           /* Last sample (for BRR filter) */
		int32_t         smp2;           /* Second-to-last sample decoded*/
		short           sampbuf[4];   /* Buffer for Gaussian interp   */
	};

	struct src_dir_type                      /* Source directory entry       */
	{
		uint16_t  vptr;           /* Ptr to start of sample data  */
		uint16_t  lptr;           /* Loop pointer in sample data  */
	};


	DECLARE_READ8_MEMBER(dsp_io_r);
	DECLARE_WRITE8_MEMBER(dsp_io_w);
//  TIMER_CALLBACK_MEMBER(spc_timer);
	void dsp_reset();
	void dsp_update(short *sound_ptr);
	int advance_envelope(int v);
	void state_register();

	// internal state
	std::unique_ptr<uint8_t[]>                   m_ram;
	sound_stream            *m_channel;
	uint8_t                 m_dsp_regs[256];      /* DSP registers */
	uint8_t                 m_ipl_region[64];     /* SPC top 64 bytes */

	int                     m_keyed_on;
	int                     m_keys;               /* 8-bits for 8 voices */
	voice_state_type        m_voice_state[8];

	/* Noise stuff */
	int                     m_noise_cnt;
	int                     m_noise_lev;

	/* These are for the FIR echo filter */
#ifndef NO_ECHO
	short                   m_fir_lbuf[8];
	short                   m_fir_rbuf[8];
	int                     m_fir_ptr;
	int                     m_echo_ptr;
#endif

	enum
	{
		TIMER_TICK_ID = 1
	};
	/* timers */
	emu_timer                 *m_tick_timer;
	bool                      m_timer_enabled[3];
	uint16_t                  m_counter[3];
	uint8_t                   m_subcounter[3];
	inline void update_timer_tick(uint8_t which);

	/* IO ports */
	uint8_t                   m_port_in[4];         /* SPC input ports */
	uint8_t                   m_port_out[4];        /* SPC output ports */

	uint16_t                  m_TnDIV[3]; /**< Timer N Divider */
};

DECLARE_DEVICE_TYPE(SNES_SOUND, snes_sound_device)


#endif // MAME_AUDIO_SNES_SND_H
