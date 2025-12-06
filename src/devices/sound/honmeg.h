// license:GPL-2.0+
// copyright-holders:Jarek Burczynski, Hiromitsu Shioya, Hannes Janetzek
#ifndef MAME_SOUND_HONMEG_H
#define MAME_SOUND_HONMEG_H

#pragma once

class honmeg_device : public device_t,
                      public device_memory_interface,
                      public device_sound_interface
{
public:
	honmeg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void write(offs_t offset, uint8_t data);
	uint8_t read(offs_t offset);
    
	void set_clock(int clock);
	void wave_memory(address_map &map);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_stop() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;

    // device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

private:
	struct VOICE {
        uint8_t next_voice;
        uint8_t pitch_1;
        uint8_t pitch_2;
        int count_period() {
            return (pitch_1 | (pitch_2 << 8)) & 0xfff;
        }
		uint8_t phase;     // 8 bit binary counter (phase output)

        uint16_t next_event; // 12 bit, next event time

        struct SLOT {
            uint8_t sample;    // 4 bit  0x20X
            uint8_t output;    // 3 bit  0x2X0 ??? check
            uint8_t octave;    // 3 bit  0x10X

            // Got values:
            // - 0100 init 0x8
            // - 1100 note off 0xC
            // - 0110 note on  0x6 when next slot is used
            // - 1110 note on  0xE when next slot is not used 
            // BIT(3) END?
            // BIT(1) ACTIVE?
            
            // BIT(0,1,2) NEXT SLOT?
            uint8_t unused;    // 4 bit  0x1X0 ???
            
            uint8_t amplitude; // 8 bit  0x3XX

            // what is this used for???
            uint8_t pointer;    // 8 bit  0xXX

        };
        SLOT slot[8];
	};
	const address_space_config m_space_config;

    static constexpr int counter_advance = 4;
    
	// internal state
	sound_stream *m_stream;

	VOICE m_voi[32];

    uint16_t m_counter;
    uint8_t m_sample_wr_latch;

    int m_chip_clock;       // chip clock in Hz
    int m_rate;             // sample rate in Hz
    int m_ticks_per_sample; // number of counter steps per stream sample - FIXME
    
    float integrators[8] = {0, 0, 0, 0, 0, 0, 0, 0};

	void init_tables();
	void init(int clock, int rate);
};

DECLARE_DEVICE_TYPE(HONMEG, honmeg_device)

#endif // MAME_SOUND_HONMEG_H
