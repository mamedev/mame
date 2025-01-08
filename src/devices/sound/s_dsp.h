// license:LGPL-2.1+
// copyright-holders:R. Belmont, Brad Martin
/*****************************************************************************
 *
 * Nintendo/Sony S-DSP emulation
 *
 ****************************************************************************/

#ifndef MAME_SOUND_S_DSP_H
#define MAME_SOUND_S_DSP_H

/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/

class s_dsp_device : public device_t, public device_sound_interface, public device_memory_interface
{
public:
	s_dsp_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void set_volume(int volume);

	u8 dsp_io_r(offs_t offset);
	void dsp_io_w(offs_t offset, u8 data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_clock_changed() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	// device_memory_interface configuration
	virtual space_config_vector memory_space_config() const override;

	address_space_config m_data_config;

private:
	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::cache m_cache;
	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::specific m_data;
	inline u8 read_byte(offs_t a) { return m_cache.read_byte(a); }
	inline u16 read_word(offs_t a) { return read_byte(a) | (read_byte(a + 1) << 8); }
	inline void write_byte(offs_t a, u8 d) { m_data.write_byte(a, d); }
	inline void write_word(offs_t a, u16 d) { write_byte(a, d & 0xff); write_byte(a + 1, (d >> 8) & 0xff); }

	enum class env_state_t32 : u8
	{
		ATTACK,
		DECAY,
		SUSTAIN,
		RELEASE
	};

	struct voice_state_type                      /* Voice state type             */
	{
		u16             mem_ptr;        /* Sample data memory pointer   */
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
		u32             samp_id;        /* Sample ID#                   */
		int             sampptr;        /* Where in sampbuf we are      */
		s32             smp1;           /* Last sample (for BRR filter) */
		s32             smp2;           /* Second-to-last sample decoded*/
		s16             sampbuf[4];     /* Buffer for Gaussian interp   */
	};

	inline u16 vptr(u8 sd, u8 v) { return read_word((sd << 8) + (m_dsp_regs[v + 4] << 2)); }           /* Ptr to start of sample data  */
	inline u16 lptr(u8 sd, u8 v) { return read_word((sd << 8) + (m_dsp_regs[v + 4] << 2) + 2); }       /* Loop pointer in sample data  */
	inline u16 pitch(u8 v) { return (m_dsp_regs[v + 2] | (m_dsp_regs[v + 3] << 8)) & 0x3fff; }

	/* Make reading the ADSR code easier */
	inline u8 SL(u8 v) { return m_dsp_regs[(v << 4) + 6] >> 5; }         /* Returns SUSTAIN level        */
	inline u8 SR(u8 v) { return m_dsp_regs[(v << 4) + 6] & 0x1f; }       /* Returns SUSTAIN rate         */

	void dsp_reset();
	void dsp_update(s16 *sound_ptr);
	int advance_envelope(int v);
	void state_register();

	// internal state
	sound_stream          *m_channel;
	u8                    m_dsp_addr;
	u8                    m_dsp_regs[256];      /* DSP registers */

	int                   m_keyed_on;
	int                   m_keys;               /* 8-bits for 8 voices */
	voice_state_type      m_voice_state[8];

	/* Noise stuff */
	int                   m_noise_cnt;
	int                   m_noise_lev;

	/* These are for the FIR echo filter */
#ifndef NO_ECHO
	s16                   m_fir_lbuf[8];
	s16                   m_fir_rbuf[8];
	int                   m_fir_ptr;
	int                   m_echo_ptr;
#endif

};

DECLARE_DEVICE_TYPE(S_DSP, s_dsp_device)


#endif // MAME_SOUND_S_DSP_H
