/***************************************************************************

    votrax.c

    Simple VOTRAX SC-01 simulator based on sample fragments.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#include "emu.h"
#include "votrax.h"


//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define TEMP_HACKS		(1)

#define LOG_TIMING		(0)
#define LOG_LOWPARAM	(0)
#define LOG_GLOTTAL		(0)
#define LOG_TRANSITION	(0)



//**************************************************************************
//  CONSTANTS
//**************************************************************************

// note that according to the patent timing circuit, p1/p2 and phi1/phi2
// run 4x faster than all references in the patent text
const UINT32 P_CLOCK_BIT = 5;		// 5 according to timing diagram
const UINT32 PHI_CLOCK_BIT = 3;		// 3 according to timing diagram



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type VOTRAX_SC01 = &device_creator<votrax_sc01_device>;

// ROM definition for the Votrax phoneme ROM
ROM_START( votrax_sc01 )
	ROM_REGION( 0x200, "phoneme", 0 )
	ROM_LOAD( "sc01.bin", 0x0000, 0x200, CRC(0353dd6c) SHA1(00e8e497b96a10bd9f4d7e559433c3c209b0d3a8) )
ROM_END

// textual phoneme names for debugging
const char *const votrax_sc01_device::s_phoneme_table[64] =
{
	"EH3",	"EH2",	"EH1",	"PA0",	"DT",	"A1",	"A2",	"ZH",
	"AH2",	"I3",	"I2",	"I1",	"M",	"N",	"B",	"V",
	"CH",	"SH",	"Z",	"AW1",	"NG",	"AH1",	"OO1",	"OO",
	"L",	"K",	"J",	"H",	"G",	"F",	"D",	"S",
	"A",	"AY",	"Y1",	"UH3",	"AH",	"P",	"O",	"I",
	"U",	"Y",	"T",	"R",	"E",	"W",	"AE",	"AE1",
	"AW2",	"UH2",	"UH1",	"UH",	"O2",	"O1",	"IU",	"U1",
	"THV",	"TH",	"ER",	"EH",	"E1",	"AW",	"PA1",	"STOP"
};

// this waveform is derived from measuring fig. 10 in the patent
// it is only an approximation
const double votrax_sc01_device::s_glottal_wave[16] =
{
	0,  
	16.0/22.0, 
	-22.0/22.0, 
	-17.0/22.0, 
	-15.0/22.0, 
	-10.0/22.0,  
	-7.0/22.0,  
	-4.0/22.0,
	0,   
	0,   
	0,   
	0,   
	0,   
	0,   
	0,   
	0
};



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  votrax_sc01_device - constructor
//-------------------------------------------------

votrax_sc01_device::votrax_sc01_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, VOTRAX_SC01, "Votrax SC-01", "votrax", tag, owner, clock),
	  device_sound_interface(mconfig, *this),
	  m_stream(NULL),
	  m_phoneme_timer(NULL)
{
}


//-------------------------------------------------
//  static_set_interface - configuration helper
//  to set the interface
//-------------------------------------------------

void votrax_sc01_device::static_set_interface(device_t &device, const votrax_sc01_interface &interface)
{
	votrax_sc01_device &votrax = downcast<votrax_sc01_device &>(device);
	static_cast<votrax_sc01_interface &>(votrax) = interface;
}



//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

//-------------------------------------------------
//  write - handle a write to the control register
//-------------------------------------------------

WRITE8_MEMBER( votrax_sc01_device::write )
{
	// flush out anything currently processing
	m_stream->update();

	// only 6 bits matter
	m_phoneme = data & 0x3f;
const UINT8 *rom = m_rom + (m_phoneme << 3);
mame_printf_debug("%s: STROBE %s (F1=%X F2=%X FC=%X F3=%X F2Q=%X VA=%X FA=%X CL=%X CLD=%X VD=%X PAC=%X PH=%02X)\n", 
		machine().time().as_string(3), s_phoneme_table[m_phoneme],
		rom[0] >> 4, rom[1] >> 4, rom[2] >> 4, rom[3] >> 4, rom[4] >> 4, rom[5] >> 4, rom[6] >> 4,
		rom[3] & 0xf, rom[4] & 0xf, rom[5] & 0xf, rom[6] & 0xf, rom[7]);
	
	// the STROBE signal resets the phoneme counter
	m_counter_84 = 0xf;
	
	// not in the schematics, but necessary to fully reset the request latch
	m_latch_92 = 0;

	// clear the request signal
	m_request_func(m_request_state = m_internal_request = CLEAR_LINE);
	m_phoneme_timer->adjust(attotime::zero);
}


//-------------------------------------------------
//  inflection_w - handle a write to the 
//	inflection bits
//-------------------------------------------------

WRITE8_MEMBER( votrax_sc01_device::inflection_w )
{
	// only 2 bits matter
	data &= 3;
	if (m_inflection == data)
		return;
	
	// append an inflection marker
	m_stream->update();
	m_inflection = data;
}



//**************************************************************************
//  CORE LOGIC
//**************************************************************************

//-------------------------------------------------
//  update_subphoneme_clock_period - re-compute the 
//  period of the sub-phoneme clock, as a multiple 
//  of the master clock
//-------------------------------------------------

void votrax_sc01_device::update_subphoneme_clock_period()
{
	assert(m_latch_80 < 128);
	
/*
	The sub-phoneme timing circuit is based off the switching capacitor
	technique described in the Votrax patent. Replacing the capacitor
	ladder with [Rx] representing the effective resistance, the circuit
	becomes essentially a pair of op-amps:

		 VM
		 | i1
		[R1]
		 |                       Vc
		 +----------------------+
		 |        +---|C1|---+  |
		[R2]      |          |  |  |\
		 |Vb i2   |    |\    |  +--++\
		 +--[Rx]--+----+-\   |     |  >
		 |             |  >--+-----+-/
		[R3]      +----++/   Vc    |/
		 |i3      |    |/
		 +--------+ Va
		 |
		[R4]
		 |
		 0

	We have two op-amps, the left used as a standard amplifier, the right
	one as a comparator.  The circuit triggers when the two inputs of the
	right op-amp are equal.

	The left part of the circuit (before C1) is simply a current injector.
	It's all made of resistors, there's no modulated input, so everything
	is going to be constant.  If you don't know about op-amps used as
	amplifiers, you just need to know that it forces its two inputs to
	have the same voltage while not sending or providing any current
	through there (only though its output in fact).

	In the schema, the injected current is i2.  Basic equations apply:
	  Va = R4.i3
	  Vb = Va + R3.i3
	  Vb = Va + Rx.i2
	  Vc = Vb + R2.i1
	  VM = Vc + R1.i1
	  i1 = i2 + i3

	And the tipping happens when the voltage on the right of C1 reaches
	Vc, so:
	  Vc = Va + i2.T/C1

	(i2 being a constant, the integration is kinda easy)

	Some maths later:
	  R3.i3 = Rx.i2 -> i3 = Rx/R3.i2
	  i1 = (1+Rx/R3).i2
	  Va + (Rx + R2 + R2.Rx/R3).i2 = Va + T/C1.i2
	  T = C1*(Rx*(1+R2/R3) + R2)

	Which isn't, interestingly though not surprisingly, dependant on Vm,
	R1 or R4.  And you have to round it to the next multiple of
	0.2ms+0.1ms due to the clocking on p2 and its offset to p1 (charging
	only happens on p1 active), and add one p1/p2 cycle (0.2ms) for the
	discharge.

	So now you have your base clock, which you have to multiply by 16 to
	get the phoneme length.

	r2 = 9e3
	r3 = 1e3
	c1 = 1000e-12
	rx = 1/(5KHz * cx)
*/

	// determine total capacitance
	double cx = 0;
	if ((m_latch_80 & 0x01) != 0) cx += 5e-12;
	if ((m_latch_80 & 0x02) != 0) cx += 11e-12;
	if ((m_latch_80 & 0x04) != 0) cx += 21e-12;
	if ((m_latch_80 & 0x08) != 0) cx += 43e-12;
	if ((m_latch_80 & 0x10) != 0) cx += 86e-12;
	if ((m_latch_80 & 0x20) != 0) cx += 173e-12;
	if ((m_latch_80 & 0x40) != 0) cx += 345e-12;
	
	// apply the equation above to determine charging time
	// note that the 5kHz listed above for P1 is for a nominal master
	// clock frequency of 1.28MHz, meaning it is master clock / 128
	// which should be the P1 clock but appears to be a bit different
	double p1_frequency = double(m_master_clock_freq) / double(1 << (P_CLOCK_BIT + 2));
	double rx = 1.0 / (p1_frequency * cx);
	double period = 1000e-12 * (rx * (1.0 + 9e3 / 1e3) + 9e3);
	
	// convert to master clock cycles and round up
	m_subphoneme_period = UINT32(ceil(period * double(m_master_clock_freq)));
}


//-------------------------------------------------
//  sound_stream_update - handle update requests
//  for our sound stream
//-------------------------------------------------

void votrax_sc01_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// determine how many master half-clocks per sample
	int clocks_per_sample = m_master_clock_freq / stream.sample_rate();

	// iterate over clocks (samples)
	stream_sample_t *dest = outputs[0];
	while (samples--)
	{
		// run the digital logic at the master clock rate
		double glottal_out = 0;
		double noise_out = 0;
		for (int curclock = 0; curclock < clocks_per_sample; curclock++)
		{
if (LOG_TIMING | LOG_LOWPARAM | LOG_GLOTTAL | LOG_TRANSITION)
{
	if (m_counter_34 % 32 == 0 && m_master_clock == 0)
	{
	if (LOG_TIMING)
		mame_printf_debug("MCLK C034 L070 L072 BET1  P1   P2  PHI1 PHI2 PH1' PH2' SUBC C088 C084 L092 IIRQ ");
	if (LOG_LOWPARAM)
		mame_printf_debug("F132 F114 F112 F142 L080 ");
	if (LOG_GLOTTAL)
		mame_printf_debug("C220 C222 C224 C234 C236 FGAT GLSY ");
	if (LOG_TRANSITION)
		mame_printf_debug("0625 C046 L046 A0-2 L168 L170  FC   VA   FA   F1   F2   F3   F2Q ");
	mame_printf_debug("\n");
	}
	if (LOG_TIMING)
		mame_printf_debug("%4X %4X %4X %4X %4X %4X %4X %4X %4X %4X %4X %4X %4X %4X %4X %4X ", m_master_clock, m_counter_34, m_latch_70, m_latch_72, m_beta1, m_p1, m_p2, m_phi1, m_phi2, m_phi1_20, m_phi2_20, m_subphoneme_count, m_clock_88, m_counter_84, m_latch_92, m_internal_request);
	if (LOG_LOWPARAM)
		mame_printf_debug("%4X %4X %4X %4X %4X ", m_srff_132, m_srff_114, m_srff_112, m_srff_142, m_latch_80);
	if (LOG_GLOTTAL)
		mame_printf_debug("%4X %4X %4X %4X %4X %4X %4X ", m_counter_220, m_counter_222, m_counter_224, m_counter_234, m_counter_236, m_fgate, m_glottal_sync);
	if (LOG_TRANSITION)
		mame_printf_debug("%4X %4X %4X %4X %4X %4X %4X %4X %4X %4X %4X %4X %4X ", m_0625_clock, m_counter_46, m_latch_46, m_latch_72 & 7, m_latch_168, m_latch_170, m_fc, m_va, m_fa, m_f1, m_f2, m_f3, m_f2q);
	mame_printf_debug("\n");
}

			//==============================================
			//
			// Timing circuit (patent figure 2a)
			//
			//==============================================

			// update master clock
			m_master_clock ^= 1;
			
			// on the falling edge of the master clock, advance the 10-bit counter at 34
			UINT8 old_latch_72 = m_latch_72;
			if (m_master_clock == 0)
				m_counter_34 = (m_counter_34 + 1) & 0x3ff;
			else
			{
				m_latch_70 = m_counter_34 & 0xf;
				m_latch_72 = ((m_counter_34 >> 4) & 7) | ((m_counter_34 >> 6) & 8);
			}

			// derive beta 1 clock:
			//	set if m_latch_70.0 == 1
			//	reset if m_latch_70.0 == 0 
//			UINT8 old_beta1 = m_beta1;
			m_beta1 = BIT(m_latch_70, 0);

			// derive p2 clock:
			//	set if (m_counter_34.P_CLOCK_BIT & clock) == 1
			//	reset if (m_counter_34.P_CLOCK_BIT == 0)
			UINT8 old_p2 = m_p2;
			if (BIT(m_counter_34, P_CLOCK_BIT) & m_master_clock)
				m_p2 = 1;
			else if (!BIT(m_counter_34, P_CLOCK_BIT))
				m_p2 = 0;
			
			// derive p1 clock:
			//	set if (!m_counter_34.P_CLOCK_BIT & clock) == 1
			//	reset if (m_counter_34.P_CLOCK_BIT == 1)
//			UINT8 old_p1 = m_p1;
			if (BIT(~m_counter_34, P_CLOCK_BIT) & m_master_clock)
				m_p1 = 1;
			else if (BIT(m_counter_34, P_CLOCK_BIT))
				m_p1 = 0;
			
			// derive phi2 clock:
			//	set if (m_counter_34.PHI_CLOCK_BIT & clock) == 1
			//	reset if (m_counter_34.PHI_CLOCK_BIT == 0)
			UINT8 old_phi2 = m_phi2;
			if (BIT(m_counter_34, PHI_CLOCK_BIT) & m_master_clock)
				m_phi2 = 1;
			else if (!BIT(m_counter_34, PHI_CLOCK_BIT))
				m_phi2 = 0;
			
			// derive phi1 clock:
			//	set if (!m_counter_34.PHI_CLOCK_BIT & clock) == 1
			//	reset if (m_counter_34.PHI_CLOCK_BIT == 1)
			UINT8 old_phi1 = m_phi1;
			if (BIT(~m_counter_34, PHI_CLOCK_BIT) & m_master_clock)
				m_phi1 = 1;
			else if (BIT(m_counter_34, PHI_CLOCK_BIT))
				m_phi1 = 0;

			// derive alternate phi2 clock:
			//	set if (m_counter_34.PHI_CLOCK_BIT & clock) == 1
			//	reset if (m_counter_34.PHI_CLOCK_BIT == 0)
			UINT8 old_phi2_20 = m_phi2_20;
			if (BIT(m_counter_34, PHI_CLOCK_BIT + 2) & m_master_clock)
				m_phi2_20 = 1;
			else if (!BIT(m_counter_34, PHI_CLOCK_BIT + 2))
				m_phi2_20 = 0;
			
			// derive alternate phi1 clock:
			//	set if (!m_counter_34.PHI_CLOCK_BIT & clock) == 1
			//	reset if (m_counter_34.PHI_CLOCK_BIT == 1)
//			UINT8 old_phi1_20 = m_phi1_20;
			if (BIT(~m_counter_34, PHI_CLOCK_BIT + 2) & m_master_clock)
				m_phi1_20 = 1;
			else if (BIT(m_counter_34, PHI_CLOCK_BIT + 2))
				m_phi1_20 = 0;

			// determine rising edges of each clock of interest
//			UINT8 beta1_rising = (old_beta1 ^ m_beta1) & m_beta1;
			UINT8 p2_rising = (old_p2 ^ m_p2) & m_p2;
//			UINT8 p1_rising = (old_p1 ^ m_p1) & m_p1;
			UINT8 phi2_rising = (old_phi2 ^ m_phi2) & m_phi2;
			UINT8 phi1_rising = (old_phi1 ^ m_phi1) & m_phi1;
			UINT8 phi2_20_rising = (old_phi2_20 ^ m_phi2_20) & m_phi2_20;
//			UINT8 phi1_20_rising = (old_phi1_20 ^ m_phi1_20) & m_phi1_20;
			UINT8 a0_rising = BIT((old_latch_72 ^ m_latch_72) & m_latch_72, 0);
			UINT8 a2_rising = BIT((old_latch_72 ^ m_latch_72) & m_latch_72, 2);
			UINT8 _125k_rising = BIT((old_latch_72 ^ m_latch_72) & m_latch_72, 3);

			// track subphoneme counter state
			if (!(m_latch_42 | m_phi1))
				m_subphoneme_count = 0;
			else
				m_subphoneme_count++;
			if (p2_rising)
				m_latch_42 = (m_subphoneme_count < m_subphoneme_period);

			// update the state of the subphoneme clock line
			UINT8 old_clock_88 = m_clock_88;
			m_clock_88 = !m_latch_42;	//!(m_latch_42 | m_phi1); -- figure 7 seems to be wrong here
			UINT8 clock_88_rising = (old_clock_88 ^ m_clock_88) & m_clock_88;
			
			// the A/R line holds the counter in reset except during phoneme processing,
			// when it is clocked on the rising edge of the subphoneme timer clock
			if (m_internal_request != CLEAR_LINE)
				m_counter_84 = 0xf;
			else if (clock_88_rising)
			{
				m_counter_84 = (m_counter_84 - 1) & 0x0f;
mame_printf_debug("counter=%d\n", m_counter_84);
			}
			
			// clock the zero count latch
			if (p2_rising)
				m_latch_92 = ((m_counter_84 == 0) | (m_latch_92 << 1)) & 3;
			
			// once both bits are set, the request line goes high
			if (m_latch_92 == 3)
			{
				// if the request line was previously low, reset the VD/CLD flip-flops
				if (m_internal_request == CLEAR_LINE)
					m_srff_112 = m_srff_114 = 0;
				m_internal_request = ASSERT_LINE;
			}

			//==============================================
			//
			// Low parameter clocking (patent figure 2b)
			//
			//==============================================

			// fetch ROM data; note that the address lines come directly from
			// counter_34 and not from the latches, which are 1 cycle delayed
			UINT8 romdata = m_rom[(m_phoneme << 3) | ((m_counter_34 >> 4) & 7)];

			// update the ROM data; ROM format is (upper nibble/lower nibble)
			//	+00 = F1 parameter / 0
			//	+01 = F2 parameter / 0
			//	+02 = FC parameter / 0
			//	+03 = F3 parameter / CL
			//	+04 = F2Q Parameter / CLD
			//	+05 = VA Parameter / VD
			//	+06 = FA Parameter / PAC
			//	+07 = Phoneme timing (full 7 bits)
			
			// latch a new value from ROM on phi2
			UINT8 a = m_latch_72 & 7;
			UINT8 romdata_swapped;
			if (phi2_rising)
			{
				switch (a)
				{
					// update CL
					case 3:
						m_srff_132 = m_srff_114 & BIT(~romdata, 3);
						break;
				
					// update CLD
					case 4:
						romdata_swapped = (BIT(romdata, 0) << 3) | (BIT(romdata, 1) << 2) | (BIT(romdata, 2) << 1) | (BIT(romdata, 3) << 0);
						if (m_counter_84 != 0 && romdata_swapped == (m_counter_84 ^ 0xf))
							m_srff_114 = 1;
						break;
				
					// update VD
					case 5:
						romdata_swapped = (BIT(romdata, 0) << 3) | (BIT(romdata, 1) << 2) | (BIT(romdata, 2) << 1) | (BIT(romdata, 3) << 0);
						if (m_counter_84 != 0 && romdata_swapped == (m_counter_84 ^ 0xf))
							m_srff_112 = 1;
						break;
				
					// update FF == PAC & (VA | FA)
					case 6:
						m_srff_142 = BIT(romdata, 3);
						break;
					
					// update PH
					case 7:
						if (m_latch_80 != (romdata & 0x7f))
						{
							m_latch_80 = romdata & 0x7f;
mame_printf_debug("[PH=%02X]\n", m_latch_80);
							UINT32 old_period = m_subphoneme_period;
							update_subphoneme_clock_period();
							m_subphoneme_count = (m_subphoneme_count * m_subphoneme_period) / old_period;
							m_phoneme_timer->adjust(attotime::zero);
						}
						break;
				}
			}
			
			//==============================================
			//
			// Glottal circuit (patent figure 6)
			//
			//==============================================

			// determine the TC output from the counters (note that TC requires ET)
			UINT8 counter_222_tc = (m_counter_222 == 0xf);
			UINT8 counter_220_tc = (m_counter_220 == 0xf && counter_222_tc);
			UINT8 counter_224_tc = (m_counter_224 == 0xf && counter_222_tc);

			// clock glottal counter 224 on rising edge of a0
			if (a0_rising)
			{
				// counter 224 is only enabled if TC of counter 222 is 1
				if (counter_222_tc)
				{
					// if counter 220's TC is 1, do a load instead of a count
					if (counter_220_tc)
						m_counter_224 = (m_inflection << 1) | ((~m_f1 & 0x8) >> 3);
					else
						m_counter_224 = (m_counter_224 + 1) & 0xf;
				}
			}
			
			// clock remaining glottal counters (220, 222, 236) on rising edge of phi2
			if (phi2_20_rising)
			{
				// counter 220 is only enabled if TC of counter 222 is 1
				if (counter_222_tc)
				{
					// if counter 220's TC is 1, do a load instead of a count
					if (counter_220_tc)
						m_counter_220 = (m_inflection << 1) | ((~m_f1 & 0x8) >> 3);
					else
						m_counter_220 = (m_counter_220 + 1) & 0xf;
				}
				
				// counter 222 is always enabled
				if (1)
				{
					// if counter 220's TC is 1, do a load instead of a count
					if (counter_220_tc)
						m_counter_222 = (~m_f1 & 0x7) << 1;
					else
						m_counter_222 = (m_counter_222 + 1) & 0xf;
				}
				
				// counter 236 is always enabled
				if (1)
				{
					m_counter_236 = (m_counter_236 + 1) & 0xf;
				
					// rising edge of Q1 from counter 236 clocks counter 234
					if ((m_counter_236 & 0x3) == 0x2)
					{
						// counter 234 is only enabled if it has not reached terminal
						if (m_counter_234 != 0xf)
							m_counter_234 = (m_counter_234 + 1) & 0xf;
					}
				}
			}
			
			// update FGATE state
			if (counter_220_tc)
				m_fgate = 0;
			if (counter_224_tc)
				m_fgate = 1;
			
			// apply asynchronous clear to counters 234/236
			if (counter_220_tc && m_phi1_20)
				m_counter_236 = m_counter_234 = 0;
				
			// derive glottal circuit output signals
#if !TEMP_HACKS
			UINT8 old_glottal_sync = m_glottal_sync;
#endif
			m_glottal_sync = (m_counter_234 == 0);
			glottal_out = s_glottal_wave[m_counter_234];
			
			//==============================================
			//
			// Transition circuit (patent figure 3a/3b)
			//
			//==============================================
			
			// divide 1.25k clock by 2 (lower-left of 46)
			UINT8 old_0625_clock = m_0625_clock;
			if (_125k_rising)
				m_0625_clock = !m_0625_clock;
			UINT8 _0625_rising = (old_0625_clock ^ m_0625_clock) & m_0625_clock;
			
			// update counter above
			if (_0625_rising)
			{
				if (m_counter_46 == 0xf)
					m_counter_46 = 0xd;
				else
					m_counter_46 = (m_counter_46 + 1) & 0xf;
			}
			
			// and then the latch to the right
			if (a2_rising)
				m_latch_46 = (BIT(m_counter_46, 1) << 0) |
							 (BIT(m_latch_46, 0) << 1) |
							 (m_0625_clock << 2) |
							 (BIT(m_latch_46, 2) << 3);

#if TEMP_HACKS
			m_latch_46 = 0xf;
#endif

			// determine the read/write signal
			UINT8 ram_write = 0;
			switch (a)
			{
				// write if not FF and low 2 bits of latch
				// FF is the S/R flip-flop at 142 ANDed with !(/FA & /VA)
				case 0:	case 1:	case 2: case 3: case 4:
					if (!(m_srff_142 & !((m_fa == 0) & (m_va == 0))) && (m_latch_46 & 0x3) == 0x3)
						ram_write = 1;
					break;
				
				case 5:
					if ((m_latch_46 & 0xc) == 0xc && m_srff_112)
						ram_write = 1;
					break;
				
				case 6:
					if ((m_latch_46 & 0xc) == 0xc && m_srff_114)
						ram_write = 1;
					break;
			}
			
			// gate on the phi2 clock (OR gate @ 172)
			ram_write &= m_phi2;
			
			// write the transitioned values to RAM if requested
			// (note we consolidate the serial addition and clocking steps here)
			if (ram_write)
			{
				UINT8 old = (m_latch_168 << 4) | m_latch_170;
				m_ram[a] = old - (old >> 3) + ((romdata & 0xf0) >> 3);
			}
			
			// latch some parameter values on rising edge of phi2
			if (phi2_rising)
			{
				switch (a)
				{
					case 2:
						m_fc = m_latch_168;
						break;
					
					case 5:
						m_va = m_latch_168;
						break;
					
					case 6:
						m_fa = m_latch_168;
						break;
				}
			}
			
			// latch remaining parameter values on rising edge of (phi2 & glottal sync)
#if TEMP_HACKS
			if (phi2_rising)
#else
			UINT8 old_phi2_glottal = (old_phi2 & old_glottal_sync);
			UINT8 new_phi2_glottal = m_phi2 & m_glottal_sync;
			if ((old_phi2_glottal ^ new_phi2_glottal) & new_phi2_glottal)
#endif
				switch (a)
				{
					case 0:
						m_f1 = m_latch_168;
						break;
					
					case 1:
						m_f2 = (m_latch_168 << 1) | (m_latch_170 >> 3);
						break;
					
					case 3:
						m_f3 = m_latch_168;
						break;
					
					case 4:
						m_f2q = m_latch_168;
						break;
				}
			
			// latch value from RAM on rising edge of phi1
			if (phi1_rising)
			{
				m_latch_168 = m_ram[a] >> 4;
				m_latch_170 = m_ram[a] & 0xf;
			}

			//==============================================
			//
			// Noise generator circuit (patent figure 8)
			//
			//==============================================
			
			// nose is clocked by the NOR of /FA and P1
			UINT8 old_noise_clock = m_noise_clock;
			m_noise_clock = !((m_fa == 0) | m_p1);
			UINT8 noise_clock_rising = (old_noise_clock ^ m_noise_clock) & m_noise_clock;
			UINT8 noise_clock_falling = (old_noise_clock ^ m_noise_clock) & old_noise_clock;
			
			// falling edge clocks the shift register
			if (noise_clock_falling)
			{
				// shift register 252 is actually 4 shift registers (2 4-bit, 2 5-bit)
				// d1 and d3 are the 4-bit registers, d2 and d4 are the 5-bit registers
				// XOR'ed input goes into d4, which shifts in to d2, then d3, then d1
				// thus the full 18-bit value is effectively
				//
				//	d4 = (m_shift_252 >> 0) & 0x1f;
				//	d2 = (m_shift_252 >> 5) & 0x1f;
				//	d3 = (m_shift_252 >> 10) & 0xf;
				//	d1 = (m_shift_252 >> 14) & 0xf;
				//
				// input at the low end is ((d1+4 ^ d2+5) ^ (d4+4 ^ d4+5)) ^ !(counter2 | counter3)
				// output is tapped at d3+4

				UINT32 old_shift = m_shift_252;
				m_shift_252 <<= 1;
				m_shift_252 |= ((BIT(old_shift, 17) ^ BIT(old_shift, 9)) ^ (BIT(old_shift, 3) ^ BIT(old_shift, 4))) ^ 
								 ((m_counter_250 & 0xc) == 0);
			}
			
			// rising edge clocks the counter
			if (noise_clock_rising)
			{
				// counter is reset to 1 if terminal, otherwise it increments
				if (m_counter_250 == 0xf)
					m_counter_250 = 0x1;
				else
					m_counter_250 = (m_counter_250 + 1) & 0xf;
			}
			
			// compute final noise out signal
			UINT8 noise_out_digital = !(BIT(m_shift_252, 13) & (m_fgate | (m_va == 0)));
			noise_out = noise_out_digital ? (double(m_fa) / 15.0f) : 0;
		}
			
		// amplify the glottal pulse by the VA and cheesily mix in the noise just to
		// help see what's going on
		double current = 0.5 * glottal_out * (double(m_va) / 15.0f) + 0.5 * noise_out;
		
		// TODO: apply high pass noise shaping to noise_out (see figure 8)
		
		// TODO: apply resonsant filters based on m_f1, m_f2, m_f2q and
		// inject noise based on m_fc (see figure 9)
		
		// TODO: apply final filter f3 and inject inverse of noise based on ~m_fc (see figure 5)
		
		// TODO: apply closure circuit (undocumented)
		
		// output the current result
		*dest++ = INT16(current * 32767.0);
	}
}



//**************************************************************************
//  DEVICE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  rom_region - return a pointer to the device's
//  internal ROM region
//-------------------------------------------------

const rom_entry *votrax_sc01_device::device_rom_region() const
{
	return ROM_NAME( votrax_sc01 );
}


//-------------------------------------------------
//  device_start - handle device startup
//-------------------------------------------------

void votrax_sc01_device::device_start()
{
	// initialize internal state
	m_master_clock_freq = clock();
	m_stream = stream_alloc(0, 1, m_master_clock_freq / 16);
	m_phoneme_timer = timer_alloc();
	m_rom = subregion("phoneme")->base();
	
	// reset inputs
	m_inflection = 0;
	m_phoneme = 0x3f;
	
	// reset outputs
	m_request_func.resolve(m_request_cb, *this);
	m_request_state = ASSERT_LINE;
	m_internal_request = ASSERT_LINE;
	
	// save inputs
	save_item(NAME(m_inflection));
	save_item(NAME(m_phoneme));
	
	// save outputs
	save_item(NAME(m_request_state));
	save_item(NAME(m_internal_request));
	
	// save timing circuit
	save_item(NAME(m_master_clock_freq));
	save_item(NAME(m_master_clock));
	save_item(NAME(m_counter_34));
	save_item(NAME(m_latch_70));
	save_item(NAME(m_latch_72));
	save_item(NAME(m_beta1));
	save_item(NAME(m_p2));
	save_item(NAME(m_p1));
	save_item(NAME(m_phi2));
	save_item(NAME(m_phi1));
	save_item(NAME(m_subphoneme_period));
	save_item(NAME(m_subphoneme_count));
	save_item(NAME(m_clock_88));
	save_item(NAME(m_latch_42));
	save_item(NAME(m_counter_84));
	save_item(NAME(m_latch_92));
	
	// save low parameter clocking
	save_item(NAME(m_srff_132));
	save_item(NAME(m_srff_114));
	save_item(NAME(m_srff_112));
	save_item(NAME(m_srff_142));
	save_item(NAME(m_latch_80));
	
	// save glottal circuit
	save_item(NAME(m_counter_220));
	save_item(NAME(m_counter_222));
	save_item(NAME(m_counter_224));
	save_item(NAME(m_counter_234));
	save_item(NAME(m_counter_236));
	save_item(NAME(m_fgate));
	save_item(NAME(m_glottal_sync));

	// save transition circuit
	save_item(NAME(m_0625_clock));
	save_item(NAME(m_counter_46));
	save_item(NAME(m_latch_46));
	save_item(NAME(m_ram));
	save_item(NAME(m_latch_168));
	save_item(NAME(m_latch_170));
	save_item(NAME(m_f1));
	save_item(NAME(m_f2));
	save_item(NAME(m_fc));
	save_item(NAME(m_f3));
	save_item(NAME(m_f2q));
	save_item(NAME(m_va));
	save_item(NAME(m_fa));
	
	// save noise generator circuit
	save_item(NAME(m_noise_clock));
	save_item(NAME(m_shift_252));
	save_item(NAME(m_counter_250));
}


//-------------------------------------------------
//  device_reset - handle device reset
//-------------------------------------------------

void votrax_sc01_device::device_reset()
{
	// set the initial state
	m_stream->update();
	
	// reset inputs
	m_phoneme = 0x3f;
	m_request_func(m_internal_request = m_request_state = ASSERT_LINE);
	
	// reset timing circuit
	m_master_clock = 0;
	m_counter_34 = 0;
	m_latch_70 = 0;
	m_latch_72 = 0;
	m_beta1 = 0;
	m_p2 = 0;
	m_p1 = 0;
	m_phi2 = 0;
	m_phi1 = 0;
	m_subphoneme_period = 1000;
	m_subphoneme_count = 0;
	m_clock_88 = 0;
	m_latch_42 = 0;
	m_counter_84 = 0;
	m_latch_92 = 0;
	
	// reset low parameter clocking
	m_srff_132 = 0;
	m_srff_114 = 0;
	m_srff_112 = 0;
	m_srff_142 = 0;
	m_latch_80 = 50;
	update_subphoneme_clock_period();
	
	// reset glottal circuit
	m_counter_220 = 0;
	m_counter_222 = 0;
	m_counter_224 = 0;
	m_counter_234 = 0;
	m_counter_236 = 0;
	m_fgate = 0;
	m_glottal_sync = 0;
	
	// reset transition circuit
	m_0625_clock = 0;
	m_counter_46 = 0;
	m_latch_46 = 0;
	memset(m_ram, 0, sizeof(m_ram));
	m_latch_168 = 0;
	m_latch_170 = 0;
	m_f1 = 0;
	m_f2 = 0;
	m_fc = 0;
	m_f3 = 0;
	m_f2q = 0;
	m_va = 0;
	m_fa = 0;
	
	// reset noise circuit
	m_noise_clock = 0;
	m_shift_252 = 0;
	m_counter_250 = 0;
}


//-------------------------------------------------
//  device_clock_changed - handle dynamic clock
//	changes by altering our output frequency
//-------------------------------------------------

void votrax_sc01_device::device_clock_changed()
{
	// compute new frequency of the master clock, and update if changed
	UINT32 newfreq = clock();
	if (newfreq != m_master_clock_freq)
	{
		// if we have a stream
		if (m_stream != NULL)
		{
			m_stream->update();
			m_stream->set_sample_rate(newfreq / 16);
		}
		
		// determine how many clock ticks remained on the phoneme timer
		UINT64 remaining = m_phoneme_timer->remaining().as_ticks(m_master_clock_freq);
		
		// recompute the master clock
		m_master_clock_freq = newfreq;

		// adjust the phoneme timer to the same number of ticks based on the new frequency
		if (remaining > 0)
			m_phoneme_timer->adjust(attotime::from_ticks(remaining, newfreq));
	}
}


//-------------------------------------------------
//  device_timer - handle device timer
//-------------------------------------------------

void votrax_sc01_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	// force a stream update
	m_stream->update();
	
	// if we're requesting more data, no need for timing
	if (m_request_state == ASSERT_LINE)
		return;
	
	// if we're supposed to have fired, do it now
	if (m_internal_request == ASSERT_LINE)
	{
mame_printf_debug("%s: REQUEST\n", timer.machine().time().as_string(3));
		m_request_func(m_request_state = ASSERT_LINE);
		return;
	}

	// account for the rest of this subphoneme clock
	UINT32 clocks_until_request = 0;
	if (m_counter_84 != 0)
	{
		if (m_subphoneme_count < m_subphoneme_period)
			clocks_until_request += m_subphoneme_period - m_subphoneme_count;
		clocks_until_request += m_subphoneme_period * (m_counter_84 - 1);
	}
			
	// plus 1/2
	clocks_until_request = MAX(clocks_until_request, (1 << P_CLOCK_BIT) / 2);
	timer.adjust(attotime::from_ticks(clocks_until_request, m_master_clock_freq));
}
