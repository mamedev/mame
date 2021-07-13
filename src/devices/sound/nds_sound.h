// license:BSD-3-Clause
// copyright-holders:cam900
/***************************************************************************

    Nintendo DS Sound Hardware Emulation

***************************************************************************/

#ifndef MAME_SOUND_NDS_SOUND_H
#define MAME_SOUND_NDS_SOUND_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nds_sound_device

class nds_sound_device : public device_t, public device_sound_interface, public device_memory_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::SOUND; }

	// construction/destruction
	nds_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// channel registers
	u32 channel_control_r(offs_t offset);
	void channel_w(offs_t offset, u32 data, u32 mem_mask);

	// capture registers
	u8 capture_control_r(offs_t offset);
	u32 capture_addr_r(offs_t offset);
	void capture_control_w(offs_t offset, u8 data);
	void capture_addrlen_w(offs_t offset, u32 data, u32 mem_mask);

	// global registers
	u32 control_r();
	u32 bias_r();
	void control_w(offs_t offset, u32 data, u32 mem_mask);
	void bias_w(offs_t offset, u32 data, u32 mem_mask);

	// register map
	void amap(address_map &map);
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_clock_changed() override;

	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	// device_memory_interface configuration
	virtual space_config_vector memory_space_config() const override;

	address_space_config m_data_config;

	memory_access<27, 2, 0, ENDIANNESS_LITTLE>::cache    m_cache;
	memory_access<27, 2, 0, ENDIANNESS_LITTLE>::specific m_space;

private:
	// structs
	static constexpr unsigned CLOCK_DIVIDE = 512; // for genetate sample rate, correct?
	enum
	{
		STATE_ADPCM_LOAD = 0,
		STATE_PRE_LOOP,
		STATE_POST_LOOP
	};

	struct channel_t 
	{
		// inline constants
		const u8 voldiv_shift[4] = {0, 1, 2, 4};

		// control bits
		int volume() const { return BIT(control, 0, 7); } // global volume
		u32 voldiv() const { return voldiv_shift[BIT(control, 8, 2)]; } // volume shift
		bool hold() const  { return BIT(control, 15); } // hold bit
		u32 pan() const    { return BIT(control, 16, 7); } // panning (0...127, 0 = left, 127 = right, 64 = half)
		u32 duty() const   { return BIT(control, 24, 3); } // PSG duty
		u32 repeat() const { return BIT(control, 27, 2); } // Repeat mode (Manual, Loop infinitely, One-shot)
		u32 format() const { return BIT(control, 29, 2); } // Sound Format (PCM8, PCM16, ADPCM, PSG/Noise when exists)
		bool busy() const  { return BIT(control, 31); } // Busy flag

		// calculated values
		int lvol() const   { return (pan() == 0x7f) ? 0 : 128 - pan(); } // calculated left volume
		int rvol() const   { return (pan() == 0x7f) ? 128 : pan(); } // calculated right volume

		// calculated address
		u32 addr() const { return (sourceaddr & ~3) + (cur_bitaddr >> 3) + (cur_state == STATE_POST_LOOP ? ((loopstart + cur_addr) << 2) : (cur_addr << 2)); }

		void keyon();
		void keyoff();
		void fetch();
		void advance();
		void update();

		void write(offs_t offset, u32 data, u32 mem_mask);

		// registers
		u32 control    = 0; // Control
		u32 sourceaddr = 0; // Source Address
		u16 freq       = 0; // Frequency
		u16 loopstart  = 0; // Loop Start
		u32 length     = 0; // Length

		// configuration
		nds_sound_device *host = nullptr; // host device
		bool psg               = false;   // PSG Enable
		bool noise             = false;   // Noise Enable

		// internal states
		bool playing         = false;   // playing flag
		int adpcm_out        = 0;       // current ADPCM sample value
		int adpcm_index      = 0;       // current ADPCM step
		int prev_adpcm_out   = 0;       // previous ADPCM sample value
		int prev_adpcm_index = 0;       // previous ADPCM step
		u32 cur_addr         = 0;       // current address
		int cur_state        = 0;       // current state
		int cur_bitaddr      = 0;       // bit address
		int delay            = 0;       // delay
		s16 sample           = 0;       // current sample
		u32 lfsr             = 0x7fff;  // noise LFSR
		s16 lfsr_out         = 0x7fff;  // LFSR output
		u32 counter          = 0x10000; // clock counter
		int output           = 0;       // current output
		int loutput          = 0;       // current left output
		int routput          = 0;       // current right output
	};

	struct capture_t
	{
		// inline constants
		// control bits
		bool addmode() const    { return BIT(control, 0); } // Add mode (add channel 1/3 output with channel 0/2)
		bool get_source() const { return BIT(control, 1); } // Select source (left or right mixer, channel 0/2)
		bool repeat() const     { return BIT(control, 2); } // repeat flag
		bool format() const     { return BIT(control, 3); } // store format (PCM16, PCM8)
		bool busy() const       { return BIT(control, 7); } // busy flag

		// FIFO offset mask
		u32 fifo_mask() const   { return format() ? 7 : 3; }

		// calculated address
		u32 waddr() const { return (dstaddr & ~3) + (cur_waddr << 2); }

		void capture_on();
		void capture_off();
		void update(int mix);
		bool fifo_write();
		void addrlen_w(offs_t offset, u32 data, u32 mem_mask);

		// registers
		u8 control  = 0;   // Control
		u32 dstaddr = 0;   // Destination Address
		u32 length  = 0;   // Buffer Length

		// configuration
		nds_sound_device *host = nullptr; // host device
		channel_t *input = nullptr;       // Input channel
		channel_t *output = nullptr;      // Output channel

		// internal states
		u32 counter          = 0x10000; // clock counter
		u32 cur_addr         = 0;       // current address
		u32 cur_waddr        = 0;       // current write address
		int cur_bitaddr      = 0;       // bit address
		bool enable          = 0;       // capture enable

		// FIFO
		PAIR fifo[8];                 // FIFO (8 word, for 16 sample delay)
		u32 fifo_head        = 0;     // FIFO head
		u32 fifo_tail        = 0;     // FIFO tail
		bool fifo_empty      = true;  // FIFO empty flag
		bool fifo_full       = false; // FIFO full flag
	};

	channel_t m_channel[16]; // 16 channels
	capture_t m_capture[2]; // 2 capture channels

	inline u8 mvol() const      { return BIT(m_control, 0, 7); } // master volume
	inline u8 lout_from() const { return BIT(m_control, 8, 2); } // left output source (mixer, channel 1, channel 3, channel 1+3)
	inline u8 rout_from() const { return BIT(m_control, 10, 2); } // right output source (mixer, channel 1, channel 3, channel 1+3)
	inline bool mix_ch1() const { return BIT(m_control, 12); } // mix/bypass channel 1
	inline bool mix_ch3() const { return BIT(m_control, 13); } // mix/bypass channel 3
	inline bool enable() const  { return BIT(m_control, 15); } // global enable

	u32 m_control = 0; // global control
	u32 m_bias = 0; // output bias

	// internal states
	sound_stream *        m_stream;
};


// device type definition
DECLARE_DEVICE_TYPE(NDS_SOUND, nds_sound_device)

#endif // MAME_SOUND_NDS_SOUND_H
