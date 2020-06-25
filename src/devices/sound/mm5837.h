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

class mm5837_device : public device_t
{
public:
	// construction/destruction
	mm5837_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// configuration
	void set_vdd_voltage(int voltage) { m_vdd = voltage; }
	auto output_callback() { return m_output_cb.bind(); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	// taken from the book 'mims circuit scrapbook, vol. 1'
	// this is the frequency the chip runs at when given a vdd voltage of -0 to -15
	static constexpr int m_frequency[16] = { 0, 0, 0, 0, 0, 0, 1, 2267, 8731, 16382, 23531, 32564, 38347, 40010, 37800, 33173 };

	// callbacks
	devcb_write_line m_output_cb;

	// voltage (as positive number)
	int m_vdd;

	// output timer
	emu_timer *m_timer;

	// state
	uint32_t m_shift;
};

// device type definition
DECLARE_DEVICE_TYPE(MM5837, mm5837_device)

#endif // MAME_SOUND_MM5837_H
