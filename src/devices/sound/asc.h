// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    asc.h

    Apple Sound Chip (ASC) 344S0063
    Enhanced Apple Sound Chip (EASC) 343S1063
    Audio portion of "V8" ASIC (343S0116)
    Audio portion of "Sonora" ASIC (343S1065)
    Audio portion of "IOSB" ASIC (343S1078), copy-pasted in
      "PrimeTime" (343S0135) and "PrimeTime II" (343S0138) ASICs.

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

// device type definition
DECLARE_DEVICE_TYPE(ASC, asc_device)
DECLARE_DEVICE_TYPE(ASC_V8, asc_v8_device)
DECLARE_DEVICE_TYPE(ASC_SONORA, asc_sonora_device)
DECLARE_DEVICE_TYPE(ASC_IOSB, asc_iosb_device)
DECLARE_DEVICE_TYPE(ASC_MSC, asc_msc_device)
DECLARE_DEVICE_TYPE(ASC_EASC, asc_easc_device)

#endif // MAME_SOUND_ASC_H
