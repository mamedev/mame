// license:BSD-3-Clause
// copyright-holders:R. Belmont, Tim Douglas
/***************************************************************************

    asc.h

    Apple Sound Chip (ASC) 344S0063
    Enhanced Apple Sound Chip (EASC) 343S1063
    Audio portion of "V8" ASIC (343S0116)
    Audio portion of "Sonora" ASIC (343S1065)
    Audio portion of "IOSB" ASIC (343S1078), copy-pasted in
      "PrimeTime" (343S0135) and "PrimeTime II" (343S0138) ASICs.
    Audio portion of "PrimeTime III" ASIC (343S1189)

***************************************************************************/

#ifndef MAME_SOUND_ASC_H
#define MAME_SOUND_ASC_H

#pragma once

class asc_base_device : public device_t, public device_sound_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::SOUND; }

	auto irqf_callback() { return write_irq.bind(); }

	virtual u8 read(offs_t offset);
	virtual void write(offs_t offset, u8 data);

protected:
	enum
	{
		R_VERSION = 0,
		R_MODE,
		R_CONTROL,
		R_FIFOMODE,
		R_FIFOSTAT,
		R_WTCONTROL,
		R_VOLUME,
		R_CLOCK,
		R_BATMANCONTROL,
		R_REG9,
		R_PLAYRECA,
		R_REGB,
		R_REGC,
		R_REGD,
		R_REGE,
		R_TEST,

		// EASC extended registers
		R_WRPTRA_H = (0xf00 - 0x800),
		R_WRPTRA_L,
		R_RDPTRA_H,
		R_RDPTRA_L,
		R_SRCA_H,
		R_SRCA_L,
		R_VOLA_L,
		R_VOLA_R,
		R_FIFOA_CTRL,
		R_FIFOA_IRQCTRL,

		R_CDXA_A = (0xf10 - 0x800),

		R_WRPTRB_H = (0xf20 - 0x800),
		R_WRPTRB_L,
		R_RDPTRB_H,
		R_RDPTRB_L,
		R_SRCB_H,
		R_SRCB_L,
		R_VOLB_L,
		R_VOLB_R,
		R_FIFOB_CTRL,
		R_FIFOB_IRQCTRL,

		R_CDXA_B = (0xf30 - 0x800),
	};

	asc_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_sound_interface override
	virtual void sound_stream_update(sound_stream &stream) override;

	TIMER_CALLBACK_MEMBER(delayed_stream_update);

	virtual u8 get_version();
	virtual void set_irq_line(int status);

	devcb_write_line write_irq;

	sound_stream *m_stream;

	// inline data
	u8   m_fifo[2][0x400];

	u8  m_regs[0x800];
	s16 m_fifo_clrptr[2];
	s16 m_last_left, m_last_right;

	u32  m_phase[4], m_incr[4];

	u16 m_fifo_rdptr[2];
	u16 m_fifo_wrptr[2];
	u16 m_fifo_cap[2];

	u32 m_sample_rate;

	emu_timer *m_timer;
};

class asc_device : public asc_base_device
{
public:
	asc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read(offs_t offset) override;
	virtual void write(offs_t offset, u8 data) override;
};

class asc_v8_device : public asc_base_device
{
public:
	asc_v8_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read(offs_t offset) override;
	virtual void write(offs_t offset, u8 data) override;

protected:
	asc_v8_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device_sound_interface override
	virtual void sound_stream_update(sound_stream &stream) override;
	virtual u8 get_version() override;

	virtual void device_reset() override ATTR_COLD;
};

class asc_sonora_device : public asc_base_device
{
public:
	asc_sonora_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read(offs_t offset) override;
	virtual void write(offs_t offset, u8 data) override;

protected:
	// device_sound_interface override
	virtual void sound_stream_update(sound_stream &stream) override;
	virtual u8 get_version() override;

	virtual void device_reset() override ATTR_COLD;

private:
	u8 m_fifo_irqen[2];
};

class asc_iosb_device : public asc_base_device
{
public:
	asc_iosb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read(offs_t offset) override;
	virtual void write(offs_t offset, u8 data) override;
	u16 read_w(offs_t offset);
	void write_w(offs_t offset, u16 data);

protected:
	// device_sound_interface override
	virtual void sound_stream_update(sound_stream &stream) override;
	virtual u8 get_version() override;

private:
	u8 m_fifo_irqen[2];
};

class asc_msc_device : public asc_v8_device
{
public:
	asc_msc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual u8 get_version() override;
};

class asc_easc_device : public asc_base_device
{
public:
	asc_easc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read(offs_t offset) override;
	virtual void write(offs_t offset, u8 data) override;

protected:
	// device_sound_interface override
	virtual void sound_stream_update(sound_stream &stream) override;
	virtual u8 get_version() override;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	u8 pop_fifo(int ch);
	s16 decode_cdxa(int ch, u8 xa_mode);

	u8 m_fifo_irqen[2];

	u32 m_src_step[2], m_src_accum[2];

	// CD-XA ADPCM decoder state, indexed by channel (0=A, 1=B)
	s16 m_xa_s0[2];    // previous decoded sample
	s16 m_xa_s1[2];    // sample before that
	u8 m_xa_param[2]; // current block header (filter[5:4], range[3:0])
	s32 m_xa_pos[2];   // position within block (0 = read new param)
	u8 m_xa_byte[2];  // packed byte buffer (4:1 and 8:1 modes)
	s32 m_xa_subpos[2]; // sub-position within packed byte
};

class asc_primetime3_device : public asc_base_device
{
public:
	asc_primetime3_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read(offs_t offset) override;
	virtual void write(offs_t offset, u8 data) override;

	u16 read16(offs_t offset);
	void write16(offs_t offset, u16 data, u16 mem_mask);

protected:
	virtual void sound_stream_update(sound_stream &stream) override;
	virtual u8 get_version() override;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;

private:
	enum { FIFO_A, FIFO_B, FIFO_REC };
	static constexpr u32 FIFO_SAMPLES = 1024;
	static constexpr u32 FIFO_BYTES = FIFO_SAMPLES * 2;
	static constexpr u32 FIFO_HALF_BYTES = FIFO_BYTES / 2;
	static constexpr u32 FIFO_PTR_MASK = FIFO_BYTES - 1;

	enum
	{
		R_COUNTA = 0xf0e - 0x800,
		R_COUNTB = 0xf2e - 0x800,
		R_WRPTRPLAYA_H = 0xf2a - 0x800,
		R_WRPTRPLAYA_L,
		R_RDPTRPLAYA_H,
		R_RDPTRPLAYA_L
	};

	void reset_fifos();
	u16 fifo_cap(int f) const;
	void push_byte(int f, u8 data);
	void push_sample(int f, s16 sample);
	s16 peek_sample(int f) const;
	s16 pop_sample(int f);
	void set_irq_pending(u8 pending);
	void update_status16();
	void update_fifo_timer();
	u8 counter_value() const;
	u16 pop_record();
	int pointer_fifo(offs_t offset) const;
	u16 &pointer(offs_t offset);
	bool recording() const { return BIT(m_regs[R_PLAYRECA], 0); }

	u8 m_fifo8[3][FIFO_BYTES];
	u16 m_rdptr16[3], m_wrptr16[3];
	bool m_full[3];
	u8 m_laststat;
	u8 m_irq_pending;
	u8 m_countbase;
	u16 m_rec_last;
};

// device type definition
DECLARE_DEVICE_TYPE(ASC, asc_device)
DECLARE_DEVICE_TYPE(ASC_V8, asc_v8_device)
DECLARE_DEVICE_TYPE(ASC_SONORA, asc_sonora_device)
DECLARE_DEVICE_TYPE(ASC_IOSB, asc_iosb_device)
DECLARE_DEVICE_TYPE(ASC_PRIMETIME3, asc_primetime3_device)
DECLARE_DEVICE_TYPE(ASC_MSC, asc_msc_device)
DECLARE_DEVICE_TYPE(ASC_EASC, asc_easc_device)

#endif // MAME_SOUND_ASC_H
