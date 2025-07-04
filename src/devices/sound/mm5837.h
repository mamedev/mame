// license:BSD-3-Clause
// copyright-holders:Dirk Best
/***************************************************************************

    National Semiconductor MM5837

    Digital Noise Source

               ___ ___
       VDD  1 |*  u   | 5  N/C
       VGG  2 |       | 6  N/C
    OUTPUT  3 |       | 7  N/C
       VSS  4 |_______| 8  N/C


    Couriersud , 16.02.2020

    The following measurements were taken on Saturday, 15.02.2020 using
    a MM5837. These deviate significantly from the measurements cited
    below from 'mims circuit scrapbook'. I assume mims has noted down the
    frequencies measured on the output pin and has not taken the effort to
    convert them to the internal frequency.

    Some simple math shows that the internal frequency used to drive the
    lfsr (2^17-1 length) is 4 times the frequency measured at pin 3.
    The lfsr produces 32768 l-h transitions during one lfsr cycle. This can
    either be derived using some debug code or stochastic analysis.

    The frequencies measured are also in the order of the 100kHz the
    S2688 datasheet (MM5837 replacement) states.

    The "sweet spot" of 32768Hz was measured at 12.91 supply voltage.

       V     Fpin3     Fint           T
     7.2         1        4     32768.0
     7.4        26      104      1260.3
     7.5       110      440       297.9
     7.6       205      820       159.8
     7.7       370    1,480        88.6
     8.0       950    3,800        34.5
     8.5     2,900   11,600        11.3
     9.0     5,570   22,280         5.9
     9.5     8,300   33,200         3.9
    10.0    11,300   45,200         2.9
    10.5    14,800   59,200         2.2
    11.0    18,100   72,400         1.8
    11.5    22,050   88,200         1.5
    12.0    25,800  103,200         1.3
    12.5    29,750  119,000         1.1
    12.91   32,768  131,072         1.0
    13.0    33,670  134,680         1.0
    13.5    37,800  151,200         0.9
    14.0    41,900  167,600         0.8
    14.5    46,050  184,200         0.7
    15.0    49,700  198,800         0.7
    15.5    52,450  209,800         0.6
    16.0    53,800  215,200         0.6
    16.5    53,900  215,600         0.6
    17.0    53,000  212,000         0.6
    17.5    51,500  206,000         0.6
    18.0    49,000  196,000         0.7
    18.5    45,900  183,600         0.7
    19.0    42,500  170,000         0.8
    19.5    39,200  156,800         0.8
    20.0    36,000  144,000         0.9
    20.5    32,900  131,600         1.0
    21.0    29,600  118,400         1.1
    21.5    26,700  106,800         1.2
    22.0    24,200   96,800         1.4
    22.5    21,450   85,800         1.5
    23.0    18,800   75,200         1.7
    23.5    16,650   66,600         2.0
    24.0    13,900   55,600         2.4
    24.5    11,950   47,800         2.7

    Output levels (VGG tied to VDD), RL 22k to VDD
    These were measured using my 30 year old Hameg.

    Vsupply  Vmin   Vmax
         20   6.0    18.0
         15   4.0    13.5
         12   3.0    10.5
         10   2.5     9.0

***************************************************************************/

#ifndef MAME_SOUND_MM5837_H
#define MAME_SOUND_MM5837_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mm5837_source

class mm5837_source
{
public:
	// construction/destruction
	mm5837_source() :
		m_shift(0x1ffff)
	{
	}

	// reset to base state
	void reset()
	{
		m_shift = 0x1ffff;
	}

	// clock one time and return the shift value
	u8 clock()
	{
		int tap_14 = BIT(m_shift, 13);
		int tap_17 = BIT(m_shift, 16);
		int zero = (m_shift == 0) ? 1 : 0;

		m_shift <<= 1;
		m_shift |= tap_14 ^ tap_17 ^ zero;

		return BIT(m_shift, 16);
	}

	// taken from the book 'mims circuit scrapbook, vol. 1'
	static u32 frequency(double vdd)
	{
		// Vdd should be negative -6.2..-15V
		if (vdd > -6.2 || vdd < -15)
			throw emu_fatalerror("mm5837 frequency should be -6.2V .. -15V");

		// curve fitting done in excel from this table:
		// { 0, 0, 0, 0, 0, 0, 1, 2267, 8731, 16382, 23531, 32564, 38347, 40010, 37800, 33173 }
		double result = 191.98*vdd*vdd*vdd + 5448.4*vdd*vdd + 43388*vdd + 105347;

		// datasheet claims frequency range 24-56kHz at Vdd=-14V, but also lists
		// maximum cycle times as ranging from 1.1-2.4s; since the 17-bit shift
		// register cycles after 131072 clocks, we would expect the actual cycle
		// times to be 2.34-5.46s at the 24-56kHz frequency range, so I'm presuming
		// that it ticks 2x per "clock", effectively running at 2x the frequency
		result *= 2;

		// make sure the result isn't TOO crazy
		result = std::max(result, 100.0);
		return u32(result);
	}

	// leave this public for easy saving
	u32 m_shift;
};


// ======================> mm5837_device

class mm5837_device : public device_t
{
public:
	// construction/destruction
	mm5837_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// configuration
	void set_vdd(double voltage) { m_vdd = voltage; }
	auto output_callback() { return m_output_cb.bind(); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(update_clock_output);

private:
	// internal state
	devcb_write_line m_output_cb;     // output callback
	emu_timer *m_timer;               // output timer
	double m_vdd;                     // configured voltage
	mm5837_source m_source;           // noise source
};


// ======================> mm5837_stream_device

class mm5837_stream_device : public device_t, public device_sound_interface
{
public:
	// construction/destruction
	mm5837_stream_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	void set_vdd(double voltage) { m_vdd = voltage; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	sound_stream *m_stream;           // sound stream
	double m_vdd;                     // configured voltage
	mm5837_source m_source;           // noise source
};


// device type definitions
DECLARE_DEVICE_TYPE(MM5837, mm5837_device)
DECLARE_DEVICE_TYPE(MM5837_STREAM, mm5837_stream_device)


#endif // MAME_SOUND_MM5837_H
