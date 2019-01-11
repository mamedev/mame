// license:BSD-3-Clause
// copyright-holders:Aaron Giles,Frank Palazzolo
/***********************************************************

    Astrocade custom 'IO' chip

************************************************************
                            _____   _____
                   Vss   1 |*    \_/     | 40  MXD7
                   SI0   2 |             | 39  MXD6
                   SI1   3 |             | 38  MXD5
                   SI2   4 |             | 37  MXD4
                   SI3   5 |             | 36  MXD3
                   SI4   6 |             | 35  MXD2
                   SI5   7 |             | 34  MXD1
                   SI6   8 |             | 33  MXD0
                   SI7   9 |             | 32  SO0
                  POT0  10 |    CUSTOM   | 31  SO1
                  POT1  11 |     I/O     | 30  SO2
                  POT2  12 |             | 29  SO3
                  POT3  13 |             | 28  SO4
                DISCHG  14 |             | 27  SO5
                 MONOS  15 |             | 26  SO6
                     ϕ  16 |             | 25  SO7
                /RESET  17 |             | 24  AUDIO
                  TEST  18 |             | 23  /RD
                 /IORQ  19 |             | 22  Vgg
                    /ϕ  20 |_____________| 21  Vdd

***********************************************************/

#ifndef MAME_SOUND_ASTROCDE_H
#define MAME_SOUND_ASTROCDE_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> astrocade_io_device

class astrocade_io_device : public device_t, public device_sound_interface
{
public:
	astrocade_io_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration access
	auto si_cb() { return m_si_callback.bind(); }
	template <std::size_t Bit> auto so_cb() { return m_so_callback[Bit].bind(); }
	template <std::size_t Pot> void set_pot_tag(const char *tag) { m_pots[Pot].set_tag(tag); }

protected:
	// device-level overrides
	virtual void device_resolve_objects() override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

public:
	DECLARE_WRITE8_MEMBER(write);
	DECLARE_READ8_MEMBER(read);

private:
	void state_save_register();

	sound_stream *m_stream;       /* sound stream */

	uint8_t       m_reg[8];         /* 8 control registers */

	uint8_t       m_master_count;   /* current master oscillator count */
	uint16_t      m_vibrato_clock;  /* current vibrato clock */

	uint8_t       m_noise_clock;    /* current noise generator clock */
	uint16_t      m_noise_state;    /* current noise LFSR state */

	uint8_t       m_a_count;        /* current tone generator A count */
	uint8_t       m_a_state;        /* current tone generator A state */

	uint8_t       m_b_count;        /* current tone generator B count */
	uint8_t       m_b_state;        /* current tone generator B state */

	uint8_t       m_c_count;        /* current tone generator C count */
	uint8_t       m_c_state;        /* current tone generator C state */

	uint8_t       m_bitswap[256];   /* bitswap table */

	devcb_read8   m_si_callback;
	devcb_write8  m_so_callback[8];
	optional_ioport_array<4> m_pots;
};

DECLARE_DEVICE_TYPE(ASTROCADE_IO, astrocade_io_device)

#endif // MAME_SOUND_ASTROCDE_H
