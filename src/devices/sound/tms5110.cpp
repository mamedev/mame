// license:???
// copyright-holders:Frank Palazzolo, Jarek Burczynski, Aaron Giles, Jonathan Gevaryahu, Couriersud
/**********************************************************************************************

     TMS5110 simulator (modified from TMS5220 by Jarek Burczynski)

     Written for MAME by Frank Palazzolo
     With help from Neill Corlett
     Additional tweaking by Aaron Giles
     Various fixes by Lord Nightmare
     Additional enhancements by Couriersud
     Sub-interpolation-cycle parameter updating added by Lord Nightmare
     Read-bit and Output fixes by Lord Nightmare

     Todo:
        - implement CS
        - TMS5110_CMD_TEST_TALK is only partially implemented

     TMS5100:

                 +-----------------+
        TST      |  1           28 |  CS
        PDC      |  2           27 |  CTL8
        ROM CK   |  3           26 |  ADD8
        CPU CK   |  4           25 |  CTL1
        VDD      |  5           24 |  ADD1
        CR OSC   |  6           23 |  CTL2
        RC OSC   |  7           22 |  ADD2
        T11      |  8           21 |  ADD4
        NC       |  9           20 |  CTL4
        I/O      | 10           19 |  M1
        SPK1     | 11           18 |  NC
        SPK2     | 12           17 |  NC
        PROM OUT | 13           16 |  NC
        VSS      | 14           15 |  M0
                 +-----------------+

        T11: Sync for serial data out


    M58817

    The following connections could be derived from radar scope schematics.
    The M58817 is not 100% pin compatible to the 5100, but really close.

                 +-----------------+
        (NC)     |  1           28 |  CS
        PDC      |  2           27 |  CTL8
        ROM CK   |  3           26 |  ADD8 (to 58819)
        (NC)     |  4           25 |  CTL1
        (VDD,-5) |  5           24 |  ADD1 (to 58819)
        (GND)    |  6           23 |  CTL2
        Xin      |  7           22 |  ADD2 (to 58819)
        Xout     |  8           21 |  ADD4 (to 58819)
        (NC)     |  9           20 |  CTL4
        (VDD,-5) | 10           19 |  Status back to CPU
        (NC)     | 11           18 |  C1 (to 58819)
        SPKR     | 12           17 |  (NC)
        SPKR     | 13           16 |  C0 (to 58819)
        (NC)     | 14           15 |  (5V)
                 +-----------------+

***********************************************************************************************/

#include "emu.h"
#include "tms5110.h"

static INT16 clip_analog(INT16 cliptemp);

/* *****optional defines***** */

/* Hacky improvements which don't match patent: */
/* Interpolation shift logic:
 * One of the following two lines should be used, and the other commented
 * The second line is more accurate mathematically but not accurate to the patent
 */
#define INTERP_SHIFT >> m_coeff->interp_coeff[m_IP]
//define INTERP_SHIFT / (1<<m_coeff->interp_coeff[m_IP])

/* Other hacks */
/* HACK: if defined, outputs the low 4 bits of the lattice filter to the i/o
 * or clip logic, even though the real hardware doesn't do this, partially verified by decap */
#undef ALLOW_4_LSB

/* forces m_TALK active instantly whenever m_SPEN would be activated, causing speech delay to be reduced by up to one frame time */
/* for some reason, this hack makes snmath behave marginally more accurate to hardware, though it does not match the patent */
#define FAST_START_HACK 1


/* *****configuration of chip connection stuff***** */
/* must be defined; if 0, output the waveform as if it was tapped on the speaker pin as usual, if 1, output the waveform as if it was tapped on the i/o pin (volume is much lower in the latter case) */
#define FORCE_DIGITAL 0


/* *****debugging defines***** */
#undef VERBOSE
// above is general, somewhat obsolete, catch all for debugs which don't fit elsewhere
#undef DEBUG_PARSE_FRAME_DUMP
// above dumps each frame to stderr: be sure to select one of the options below if you define it!
#undef DEBUG_PARSE_FRAME_DUMP_BIN
// dumps each speech frame as binary
#undef DEBUG_PARSE_FRAME_DUMP_HEX
// dumps each speech frame as hex
#undef DEBUG_FRAME_ERRORS
// above dumps info if a frame ran out of data
#undef DEBUG_COMMAND_DUMP
// above dumps all command writes and PDC-related state machine changes, plus command writes to VSMs
#undef DEBUG_GENERATION
// above dumps debug information related to the sample generation loop, i.e. whether interpolation is inhibited or not, and what the current and target values for each frame are.
#undef DEBUG_GENERATION_VERBOSE
// above dumps MUCH MORE debug information related to the sample generation loop, namely the excitation, energy, pitch, k*, and output values for EVERY SINGLE SAMPLE during a frame.
#undef DEBUG_LATTICE
// above dumps the lattice filter state data each sample.
#undef DEBUG_CLIP
// above dumps info to stderr whenever the analog clip hardware is (or would be) clipping the signal.


#define MAX_SAMPLE_CHUNK        512

/* 6 Variants, from tms5110r.inc */

#define TMS5110_IS_TMC0281  (1)
#define TMS5110_IS_TMC0281D (2)
#define TMS5110_IS_CD2801   (3)
#define TMS5110_IS_CD2802   (4)
#define TMS5110_IS_TMS5110A (5)
#define TMS5110_IS_M58817   (6)


/* States for CTL */

// ctl bus is input to tms51xx
#define CTL_STATE_INPUT               (0)
// ctl bus is outputting a test talk command on CTL1(bit 0)
#define CTL_STATE_TTALK_OUTPUT        (1)
// ctl bus is switching direction, next will be above
#define CTL_STATE_NEXT_TTALK_OUTPUT   (2)
// ctl bus is outputting a read nybble 'output' command on CTL1,2,4,8 (bits 0-3)
#define CTL_STATE_OUTPUT              (3)
// ctl bus is switching direction, next will be above
#define CTL_STATE_NEXT_OUTPUT         (4)



/* Pull in the ROM tables */
#include "tms5110r.inc"

#define DEBUG_5110  0

void tms5110_device::set_variant(int variant)
{
	switch (variant)
	{
		case TMS5110_IS_TMC0281:
			m_coeff = &T0280B_0281A_coeff;
			break;
		case TMS5110_IS_TMC0281D:
			m_coeff = &T0280D_0281D_coeff;
			break;
		case TMS5110_IS_CD2801:
			m_coeff = &T0280F_2801A_coeff;
			break;
		case TMS5110_IS_M58817:
			m_coeff = &M58817_coeff;
			break;
		case TMS5110_IS_CD2802:
			m_coeff = &T0280F_2802_coeff;
			break;
		case TMS5110_IS_TMS5110A:
			m_coeff = &tms5110a_coeff;
			break;
		default:
			fatalerror("Unknown variant in tms5110_create\n");
	}

	m_variant = variant;
}

void tms5110_device::new_int_write(UINT8 rc, UINT8 m0, UINT8 m1, UINT8 addr)
{
	if (!m_m0_cb.isnull())
		m_m0_cb(m0);
	if (!m_m1_cb.isnull())
		m_m1_cb(m1);
	if (!m_addr_cb.isnull())
		m_addr_cb((offs_t)0, addr);
	if (!m_romclk_cb.isnull())
	{
		//printf("rc %d\n", rc);
		m_romclk_cb(rc);
	}
}

void tms5110_device::new_int_write_addr(UINT8 addr)
{
	new_int_write(1, 0, 1, addr); // romclk 1, m0 0, m1 1, addr bus nybble = xxxx
	new_int_write(0, 0, 1, addr); // romclk 0, m0 0, m1 1, addr bus nybble = xxxx
	new_int_write(1, 0, 0, addr); // romclk 1, m0 0, m1 0, addr bus nybble = xxxx
	new_int_write(0, 0, 0, addr); // romclk 0, m0 0, m1 0, addr bus nybble = xxxx
}

UINT8 tms5110_device::new_int_read()
{
	new_int_write(1, 1, 0, 0); // romclk 1, m0 1, m1 0, addr bus nybble = 0/open bus
	new_int_write(0, 1, 0, 0); // romclk 0, m0 1, m1 0, addr bus nybble = 0/open bus
	new_int_write(1, 0, 0, 0); // romclk 1, m0 0, m1 0, addr bus nybble = 0/open bus
	new_int_write(0, 0, 0, 0); // romclk 0, m0 0, m1 0, addr bus nybble = 0/open bus
	if (!m_data_cb.isnull())
		return m_data_cb();
	if (DEBUG_5110) logerror("WARNING: CALLBACK MISSING, RETURNING 0!\n");
	return 0;
}

void tms5110_device::register_for_save_states()
{
	save_item(NAME(m_variant));

	save_item(NAME(m_PDC));
	save_item(NAME(m_CTL_pins));
	save_item(NAME(m_SPEN));
	save_item(NAME(m_TALK));
	save_item(NAME(m_TALKD));
	save_item(NAME(m_state));

	save_item(NAME(m_address));
	save_item(NAME(m_next_is_address));
	save_item(NAME(m_schedule_dummy_read));
	save_item(NAME(m_addr_bit));
	save_item(NAME(m_CTL_buffer));

	save_item(NAME(m_OLDE));
	save_item(NAME(m_OLDP));

	save_item(NAME(m_new_frame_energy_idx));
	save_item(NAME(m_new_frame_pitch_idx));
	save_item(NAME(m_new_frame_k_idx));
#ifdef PERFECT_INTERPOLATION_HACK
	save_item(NAME(m_old_frame_energy_idx));
	save_item(NAME(m_old_frame_pitch_idx));
	save_item(NAME(m_old_frame_k_idx));
	save_item(NAME(m_old_zpar));
	save_item(NAME(m_old_uv_zpar));
#endif
	save_item(NAME(m_current_energy));
	save_item(NAME(m_current_pitch));
	save_item(NAME(m_current_k));

	save_item(NAME(m_previous_energy));

	save_item(NAME(m_subcycle));
	save_item(NAME(m_subc_reload));
	save_item(NAME(m_PC));
	save_item(NAME(m_IP));
	save_item(NAME(m_inhibit));
	save_item(NAME(m_uv_zpar));
	save_item(NAME(m_zpar));
	save_item(NAME(m_pitch_zero));
	save_item(NAME(m_pitch_count));

	save_item(NAME(m_u));
	save_item(NAME(m_x));

	save_item(NAME(m_RNG));
	save_item(NAME(m_excitation_data));

	save_item(NAME(m_digital_select));

	save_item(NAME(m_speech_rom_bitnum));

	save_item(NAME(m_romclk_hack_timer_started));
	save_item(NAME(m_romclk_hack_state));
}

/**********************************************************************************************

      printbits helper function: takes a long int input and prints the resulting bits to stderr

***********************************************************************************************/

#ifdef DEBUG_PARSE_FRAME_DUMP_BIN
static void printbits(long data, int num)
{
	int i;
	for (i=(num-1); i>=0; i--)
		fprintf(stderr,"%0ld", (data>>i)&1);
}
#endif
#ifdef DEBUG_PARSE_FRAME_DUMP_HEX
static void printbits(long data, int num)
{
	switch((num-1)&0xFC)
	{
		case 0:
			fprintf(stderr,"%0lx", data);
			break;
		case 4:
			fprintf(stderr,"%02lx", data);
			break;
		case 8:
			fprintf(stderr,"%03lx", data);
			break;
		case 12:
			fprintf(stderr,"%04lx", data);
			break;
		default:
			fprintf(stderr,"%04lx", data);
			break;
	}
}
#endif

/******************************************************************************************

     extract_bits -- extract a specific number of bits from the VSM

******************************************************************************************/

int tms5110_device::extract_bits(int count)
{
	int val = 0;
	if (DEBUG_5110) logerror("requesting %d bits", count);
	for (int i = 0; i < count; i++)
	{
		val = (val<<1) | new_int_read();
		if (DEBUG_5110) logerror("bit read: %d\n", val&1);
	}
	if (DEBUG_5110) logerror("returning: %02x\n", val);
	return val;
}


void tms5110_device::perform_dummy_read()
{
	if (m_schedule_dummy_read)
	{
		int data = new_int_read();
		if (DEBUG_5110) logerror("TMS5110 performing dummy read; value read = %1i\n", data & 1);
		m_schedule_dummy_read = FALSE;
	}
}




/**********************************************************************************************

     tms5110_process -- fill the buffer with a specific number of samples

***********************************************************************************************/

void tms5110_device::process(INT16 *buffer, unsigned int size)
{
	int buf_count=0;
	int i, bitout;
	INT32 this_sample;

	/* loop until the buffer is full or we've stopped speaking */
	while (size > 0)
	{
		if(m_TALKD) // speaking
		{
			/* if we're ready for a new frame to be applied, i.e. when IP=0, PC=12, Sub=1
			 * (In reality, the frame was really loaded incrementally during the entire IP=0
			 * PC=x time period, but it doesn't affect anything until IP=0 PC=12 happens)
			 */
			if ((m_IP == 0) && (m_PC == 12) && (m_subcycle == 1))
			{
				// HACK for regression testing, be sure to comment out before release!
				//m_RNG = 0x1234;
				// end HACK

#ifdef PERFECT_INTERPOLATION_HACK
				/* remember previous frame energy, pitch, and coefficients */
				m_old_frame_energy_idx = m_new_frame_energy_idx;
				m_old_frame_pitch_idx = m_new_frame_pitch_idx;
				for (i = 0; i < m_coeff->num_k; i++)
					m_old_frame_k_idx[i] = m_new_frame_k_idx[i];
#endif

				/* Parse a new frame into the new_target_energy, new_target_pitch and new_target_k[] */
				parse_frame();

				/* if the new frame is a stop frame, unset both TALK and SPEN (via TCON). TALKD remains active while the energy is ramping to 0. */
				if (NEW_FRAME_STOP_FLAG == 1)
				{
					m_TALK = m_SPEN = 0;
				}

				/* in all cases where interpolation would be inhibited, set the inhibit flag; otherwise clear it.
				 * Interpolation inhibit cases:
				 * Old frame was voiced, new is unvoiced
				 * Old frame was silence/zero energy, new has non-zero energy
				 * Old frame was unvoiced, new is voiced
				 * Old frame was unvoiced, new frame is silence/zero energy (non-existent on tms51xx rev D and F (present and working on tms52xx, present but buggy on tms51xx rev A and B))
				 */
				if ( ((OLD_FRAME_UNVOICED_FLAG == 0) && NEW_FRAME_UNVOICED_FLAG)
					|| ((OLD_FRAME_UNVOICED_FLAG == 1) && !NEW_FRAME_UNVOICED_FLAG)
					|| ((OLD_FRAME_SILENCE_FLAG == 1) && !NEW_FRAME_SILENCE_FLAG) )
					//|| ((m_inhibit == 1) && (OLD_FRAME_UNVOICED_FLAG == 1) && (NEW_FRAME_SILENCE_FLAG == 1)) ) //TMS51xx INTERP BUG1
					//|| ((OLD_FRAME_UNVOICED_FLAG == 1) && (NEW_FRAME_SILENCE_FLAG == 1)) )
					m_inhibit = 1;
				else // normal frame, normal interpolation
					m_inhibit = 0;

#ifdef DEBUG_GENERATION
				/* Debug info for current parsed frame */
				fprintf(stderr, "OLDE: %d; NEWE: %d; OLDP: %d; NEWP: %d ", OLD_FRAME_SILENCE_FLAG, NEW_FRAME_SILENCE_FLAG, OLD_FRAME_UNVOICED_FLAG, NEW_FRAME_UNVOICED_FLAG);
				fprintf(stderr,"Processing new frame: ");
				if (m_inhibit == 0)
					fprintf(stderr, "Normal Frame\n");
				else
					fprintf(stderr,"Interpolation Inhibited\n");
				fprintf(stderr,"*** current Energy, Pitch and Ks =      %04d,   %04d, %04d, %04d, %04d, %04d, %04d, %04d, %04d, %04d, %04d, %04d\n",m_current_energy, m_current_pitch, m_current_k[0], m_current_k[1], m_current_k[2], m_current_k[3], m_current_k[4], m_current_k[5], m_current_k[6], m_current_k[7], m_current_k[8], m_current_k[9]);
				fprintf(stderr,"*** target Energy(idx), Pitch, and Ks = %04d(%x),%04d, %04d, %04d, %04d, %04d, %04d, %04d, %04d, %04d, %04d, %04d\n",
					(m_coeff->energytable[m_new_frame_energy_idx] * (1-m_zpar)),
					m_new_frame_energy_idx,
					(m_coeff->pitchtable[m_new_frame_pitch_idx] * (1-m_zpar)),
					(m_coeff->ktable[0][m_new_frame_k_idx[0]] * (1-m_zpar)),
					(m_coeff->ktable[1][m_new_frame_k_idx[1]] * (1-m_zpar)),
					(m_coeff->ktable[2][m_new_frame_k_idx[2]] * (1-m_zpar)),
					(m_coeff->ktable[3][m_new_frame_k_idx[3]] * (1-m_zpar)),
					(m_coeff->ktable[4][m_new_frame_k_idx[4]] * (1-m_uv_zpar)),
					(m_coeff->ktable[5][m_new_frame_k_idx[5]] * (1-m_uv_zpar)),
					(m_coeff->ktable[6][m_new_frame_k_idx[6]] * (1-m_uv_zpar)),
					(m_coeff->ktable[7][m_new_frame_k_idx[7]] * (1-m_uv_zpar)),
					(m_coeff->ktable[8][m_new_frame_k_idx[8]] * (1-m_uv_zpar)),
					(m_coeff->ktable[9][m_new_frame_k_idx[9]] * (1-m_uv_zpar)) );
#endif

			}
			else // Not a new frame, just interpolate the existing frame.
			{
				int inhibit_state = ((m_inhibit==1)&&(m_IP != 0)); // disable inhibit when reaching the last interp period, but don't overwrite the m_inhibit value
#ifdef PERFECT_INTERPOLATION_HACK
				int samples_per_frame = m_subc_reload?175:266; // either (13 A cycles + 12 B cycles) * 7 interps for normal SPEAK/SPKEXT, or (13*2 A cycles + 12 B cycles) * 7 interps for SPKSLOW
				//int samples_per_frame = m_subc_reload?200:304; // either (13 A cycles + 12 B cycles) * 8 interps for normal SPEAK/SPKEXT, or (13*2 A cycles + 12 B cycles) * 8 interps for SPKSLOW
				int current_sample = (m_subcycle - m_subc_reload)+(m_PC*(3-m_subc_reload))+((m_subc_reload?25:38)*((m_IP-1)&7));
				//fprintf(stderr, "CS: %03d", current_sample);
				// reset the current energy, pitch, etc to what it was at frame start
				m_current_energy = (m_coeff->energytable[m_old_frame_energy_idx] * (1-m_old_zpar));
				m_current_pitch = (m_coeff->pitchtable[m_old_frame_pitch_idx] * (1-m_old_zpar));
				for (i = 0; i < m_coeff->num_k; i++)
					m_current_k[i] = (m_coeff->ktable[i][m_old_frame_k_idx[i]] * (1-((i<4)?m_old_zpar:m_old_uv_zpar)));
				// now adjust each value to be exactly correct for each of the samples per frame
				if (m_IP != 0) // if we're still interpolating...
				{
					m_current_energy = (m_current_energy + (((m_coeff->energytable[m_new_frame_energy_idx] - m_current_energy)*(1-inhibit_state))*current_sample)/samples_per_frame)*(1-m_zpar);
					m_current_pitch = (m_current_pitch + (((m_coeff->pitchtable[m_new_frame_pitch_idx] - m_current_pitch)*(1-inhibit_state))*current_sample)/samples_per_frame)*(1-m_zpar);
					for (i = 0; i < m_coeff->num_k; i++)
						m_current_k[i] = (m_current_k[i] + (((m_coeff->ktable[i][m_new_frame_k_idx[i]] - m_current_k[i])*(1-inhibit_state))*current_sample)/samples_per_frame)*(1-((i<4)?m_zpar:m_uv_zpar));
				}
				else // we're done, play this frame for 1/8 frame.
				{
					if (m_subcycle == 2) m_pitch_zero = 0; // this reset happens around the second subcycle during IP=0
					m_current_energy = (m_coeff->energytable[m_new_frame_energy_idx] * (1-m_zpar));
					m_current_pitch = (m_coeff->pitchtable[m_new_frame_pitch_idx] * (1-m_zpar));
					for (i = 0; i < m_coeff->num_k; i++)
						m_current_k[i] = (m_coeff->ktable[i][m_new_frame_k_idx[i]] * (1-((i<4)?m_zpar:m_uv_zpar)));
				}
#else
				//Updates to parameters only happen on subcycle '2' (B cycle) of PCs.
				if (m_subcycle == 2)
				{
					switch(m_PC)
					{
						case 0: /* PC = 0, B cycle, write updated energy */
						if (m_IP==0) m_pitch_zero = 0; // this reset happens around the second subcycle during IP=0
						m_current_energy = (m_current_energy + (((m_coeff->energytable[m_new_frame_energy_idx] - m_current_energy)*(1-inhibit_state)) INTERP_SHIFT))*(1-m_zpar);
						break;
						case 1: /* PC = 1, B cycle, write updated pitch */
						m_current_pitch = (m_current_pitch + (((m_coeff->pitchtable[m_new_frame_pitch_idx] - m_current_pitch)*(1-inhibit_state)) INTERP_SHIFT))*(1-m_zpar);
						break;
						case 2: case 3: case 4: case 5: case 6: case 7: case 8: case 9: case 10: case 11:
						/* PC = 2 through 11, B cycle, write updated K1 through K10 */
						m_current_k[m_PC-2] = (m_current_k[m_PC-2] + (((m_coeff->ktable[m_PC-2][m_new_frame_k_idx[m_PC-2]] - m_current_k[m_PC-2])*(1-inhibit_state)) INTERP_SHIFT))*(1-(((m_PC-2)<4)?m_zpar:m_uv_zpar));
						break;
						case 12: /* PC = 12 */
						/* we should NEVER reach this point, PC=12 doesn't have a subcycle 2 */
						break;
					}
				}
#endif
			}

			// calculate the output
			if (OLD_FRAME_UNVOICED_FLAG == 1)
			{
				// generate unvoiced samples here
				if (m_RNG & 1)
					m_excitation_data = ~0x3F; /* according to the patent it is (either + or -) half of the maximum value in the chirp table, so either 01000000(0x40) or 11000000(0xC0)*/
				else
					m_excitation_data = 0x40;
			}
			else /* (OLD_FRAME_UNVOICED_FLAG == 0) */
			{
				// generate voiced samples here
				/* US patent 4331836 Figure 14B shows, and logic would hold, that a pitch based chirp
				 * function has a chirp/peak and then a long chain of zeroes.
				 * The last entry of the chirp rom is at address 0b110011 (51d), the 52nd sample,
				 * and if the address reaches that point the ADDRESS incrementer is
				 * disabled, forcing all samples beyond 51d to be == 51d
				 */
				if (m_pitch_count >= 51)
					m_excitation_data = (INT8)m_coeff->chirptable[51];
				else /*m_pitch_count < 51*/
					m_excitation_data = (INT8)m_coeff->chirptable[m_pitch_count];
			}

			// Update LFSR *20* times every sample (once per T cycle), like patent shows
			for (i=0; i<20; i++)
			{
				bitout = ((m_RNG >> 12) & 1) ^
						((m_RNG >>  3) & 1) ^
						((m_RNG >>  2) & 1) ^
						((m_RNG >>  0) & 1);
				m_RNG <<= 1;
				m_RNG |= bitout;
			}
			this_sample = lattice_filter(); /* execute lattice filter */
#ifdef DEBUG_GENERATION_VERBOSE
			//fprintf(stderr,"C:%01d; ",m_subcycle);
			fprintf(stderr,"IP:%01d PC:%02d X:%04d E:%03d P:%03d Pc:%03d ",m_IP, m_PC, m_excitation_data, m_current_energy, m_current_pitch, m_pitch_count);
			//fprintf(stderr,"X:%04d E:%03d P:%03d Pc:%03d ", m_excitation_data, m_current_energy, m_current_pitch, m_pitch_count);
			for (i=0; i<10; i++)
				fprintf(stderr,"K%d:%04d ", i+1, m_current_k[i]);
			fprintf(stderr,"Out:%06d ", this_sample);
//#ifdef PERFECT_INTERPOLATION_HACK
//          fprintf(stderr,"%d%d%d%d",m_old_zpar,m_zpar,m_old_uv_zpar,m_uv_zpar);
//#else
//          fprintf(stderr,"x%dx%d",m_zpar,m_uv_zpar);
//#endif
			fprintf(stderr,"\n");
#endif
			/* next, force result to 14 bits (since its possible that the addition at the final (k1) stage of the lattice overflowed) */
			while (this_sample > 16383) this_sample -= 32768;
			while (this_sample < -16384) this_sample += 32768;
			if (m_digital_select == 0) // analog SPK pin output is only 8 bits, with clipping
				buffer[buf_count] = clip_analog(this_sample);
			else // digital I/O pin output is 12 bits
			{
#ifdef ALLOW_4_LSB
				// input:  ssss ssss ssss ssss ssnn nnnn nnnn nnnn
				// N taps:                       ^                 = 0x2000;
				// output: ssss ssss ssss ssss snnn nnnn nnnn nnnN
				buffer[buf_count] = (this_sample<<1)|((this_sample&0x2000)>>13);
#else
				this_sample &= ~0xF;
				// input:  ssss ssss ssss ssss ssnn nnnn nnnn 0000
				// N taps:                       ^^ ^^^            = 0x3E00;
				// output: ssss ssss ssss ssss snnn nnnn nnnN NNNN
				buffer[buf_count] = (this_sample<<1)|((this_sample&0x3E00)>>9);
#endif
			}
			// Update all counts

			m_subcycle++;
			if ((m_subcycle == 2) && (m_PC == 12)) // RESETF3
			{
				/* Circuit 412 in the patent acts a reset, resetting the pitch counter to 0
				 * if INHIBIT was true during the most recent frame transition.
				 * The exact time this occurs is betwen IP=7, PC=12 sub=0, T=t12
				 * and m_IP = 0, PC=0 sub=0, T=t12, a period of exactly 20 cycles,
				 * which overlaps the time OLDE and OLDP are updated at IP=7 PC=12 T17
				 * (and hence INHIBIT itself 2 t-cycles later).
				 * According to testing the pitch zeroing lasts approximately 2 samples.
				 * We set the zeroing latch here, and unset it on PC=1 in the generator.
				 */
				if ((m_IP == 7)&&(m_inhibit==1)) m_pitch_zero = 1;
				if (m_IP == 7) // RESETL4
				{
					// Latch OLDE and OLDP
					//if (OLD_FRAME_SILENCE_FLAG) m_uv_zpar = 0; // TMS51xx INTERP BUG2
					OLD_FRAME_SILENCE_FLAG = NEW_FRAME_SILENCE_FLAG; // m_OLDE
					OLD_FRAME_UNVOICED_FLAG = NEW_FRAME_UNVOICED_FLAG; // m_OLDP
					/* if TALK was clear last frame, halt speech now, since TALKD (latched from TALK on new frame) just went inactive. */
#ifdef DEBUG_GENERATION
					if ((!m_TALK) && (!m_SPEN))
						fprintf(stderr,"tms5110_process: processing frame: TALKD = 0 caused by stop frame, halting speech.\n");
#endif
					m_TALKD = m_TALK; // TALKD is latched from TALK
					if ((!m_TALK) && m_SPEN) m_TALK = 1; // TALK is only activated if it wasn't already active, if m_SPEN is active, and if we're in RESETL4 (which we are).
				}
				m_subcycle = m_subc_reload;
				m_PC = 0;
				m_IP++;
				m_IP&=0x7;
			}
			else if (m_subcycle == 3)
			{
				m_subcycle = m_subc_reload;
				m_PC++;
			}
			m_pitch_count++;
			if ((m_pitch_count >= m_current_pitch)||(m_pitch_zero == 1)) m_pitch_count = 0;
			m_pitch_count &= 0x1FF;
		}
		else // m_TALKD == 0
		{
			m_subcycle++;
			if ((m_subcycle == 2) && (m_PC == 12)) // RESETF3
			{
				if (m_IP == 7) // RESETL4
				{
					m_TALKD = m_TALK; // TALKD is latched from TALK
					if ((!m_TALK) && m_SPEN) m_TALK = 1; // TALK is only activated if it wasn't already active, if m_SPEN is active, and if we're in RESETL4 (which we are).
				}
				m_subcycle = m_subc_reload;
				m_PC = 0;
				m_IP++;
				m_IP&=0x7;
			}
			else if (m_subcycle == 3)
			{
				m_subcycle = m_subc_reload;
				m_PC++;
			}
			buffer[buf_count] = -1; /* should be just -1; actual chip outputs -1 every idle sample; (cf note in data sheet, p 10, table 4) */
		}
	buf_count++;
	size--;
	}
}

/**********************************************************************************************

     clip_analog -- clips the 14 bit return value from the lattice filter to its final 10 bit value (-512 to 511), and upshifts/range extends this to 16 bits

***********************************************************************************************/

static INT16 clip_analog(INT16 cliptemp)
{
	/* clipping, just like the patent shows:
	 * the top 10 bits of this result are visible on the digital output IO pin.
	 * next, if the top 3 bits of the 14 bit result are all the same, the lowest of those 3 bits plus the next 7 bits are the signed analog output, otherwise the low bits are all forced to match the inverse of the topmost bit, i.e.:
	 * 1x xxxx xxxx xxxx -> 0b10000000
	 * 11 1bcd efgh xxxx -> 0b1bcdefgh
	 * 00 0bcd efgh xxxx -> 0b0bcdefgh
	 * 0x xxxx xxxx xxxx -> 0b01111111
	 */
#ifdef DEBUG_CLIP
	if ((cliptemp > 2047) || (cliptemp < -2048)) fprintf(stderr,"clipping cliptemp to range; was %d\n", cliptemp);
#endif
	if (cliptemp > 2047) cliptemp = 2047;
	else if (cliptemp < -2048) cliptemp = -2048;
	/* at this point the analog output is tapped */
#ifdef ALLOW_4_LSB
	// input:  ssss snnn nnnn nnnn
	// N taps:       ^^^ ^         = 0x0780
	// output: snnn nnnn nnnn NNNN
	return (cliptemp << 4)|((cliptemp&0x780)>>7); // upshift and range adjust
#else
	cliptemp &= ~0xF;
	// input:  ssss snnn nnnn 0000
	// N taps:       ^^^ ^^^^      = 0x07F0
	// P taps:       ^             = 0x0400
	// output: snnn nnnn NNNN NNNP
	return (cliptemp << 4)|((cliptemp&0x7F0)>>3)|((cliptemp&0x400)>>10); // upshift and range adjust
#endif
}


/**********************************************************************************************

     matrix_multiply -- does the proper multiply and shift
     a is the k coefficient and is clamped to 10 bits (9 bits plus a sign)
     b is the running result and is clamped to 14 bits.
     output is 14 bits, but note the result LSB bit is always 1.
     Because the low 4 bits of the result are trimmed off before
     output, this makes almost no difference in the computation.

**********************************************************************************************/
static INT32 matrix_multiply(INT32 a, INT32 b)
{
	INT32 result;
	while (a>511) { a-=1024; }
	while (a<-512) { a+=1024; }
	while (b>16383) { b-=32768; }
	while (b<-16384) { b+=32768; }
	result = ((a*b)>>9); /** TODO: this isn't technically right to the chip, which truncates the lowest result bit, but it causes glitches otherwise. **/
#ifdef VERBOSE
	if (result>16383) fprintf(stderr,"matrix multiplier overflowed! a: %x, b: %x, result: %x", a, b, result);
	if (result<-16384) fprintf(stderr,"matrix multiplier underflowed! a: %x, b: %x, result: %x", a, b, result);
#endif
	return result;
}

/**********************************************************************************************

     lattice_filter -- executes one 'full run' of the lattice filter on a specific byte of
     excitation data, and specific values of all the current k constants,  and returns the
     resulting sample.

***********************************************************************************************/

INT32 tms5110_device::lattice_filter()
{
	// Lattice filter here
	// Aug/05/07: redone as unrolled loop, for clarity - LN
	/* Originally Copied verbatim from table I in US patent 4,209,804, now updated to be in same order as the actual chip does it, not that it matters.
	  notation equivalencies from table:
	  Yn(i) == m_u[n-1]
	  Kn = m_current_k[n-1]
	  bn = m_x[n-1]
	 */
	/*
	    int ep = matrix_multiply(m_previous_energy, (m_excitation_data<<6));  //Y(11)
	     m_u[10] = ep;
	    for (int i = 0; i < 10; i++)
	    {
	        int ii = 10-i; // for m = 10, this would be 11 - i, and since i is from 1 to 10, then ii ranges from 10 to 1
	        //int jj = ii+1; // this variable, even on the fortran version, is never used. it probably was intended to be used on the two lines below the next one to save some redundant additions on each.
	        ep = ep - (((m_current_k[ii-1] * m_x[ii-1])>>9)|1); // subtract reflection from lower stage 'top of lattice'
	         m_u[ii-1] = ep;
	        m_x[ii] = m_x[ii-1] + (((m_current_k[ii-1] * ep)>>9)|1); // add reflection from upper stage 'bottom of lattice'
	    }
	m_x[0] = ep; // feed the last section of the top of the lattice directly to the bottom of the lattice
	*/
		m_u[10] = matrix_multiply(m_previous_energy, (m_excitation_data<<6));  //Y(11)
		m_u[9] = m_u[10] - matrix_multiply(m_current_k[9], m_x[9]);
		m_u[8] = m_u[9] - matrix_multiply(m_current_k[8], m_x[8]);
		m_u[7] = m_u[8] - matrix_multiply(m_current_k[7], m_x[7]);
		m_u[6] = m_u[7] - matrix_multiply(m_current_k[6], m_x[6]);
		m_u[5] = m_u[6] - matrix_multiply(m_current_k[5], m_x[5]);
		m_u[4] = m_u[5] - matrix_multiply(m_current_k[4], m_x[4]);
		m_u[3] = m_u[4] - matrix_multiply(m_current_k[3], m_x[3]);
		m_u[2] = m_u[3] - matrix_multiply(m_current_k[2], m_x[2]);
		m_u[1] = m_u[2] - matrix_multiply(m_current_k[1], m_x[1]);
		m_u[0] = m_u[1] - matrix_multiply(m_current_k[0], m_x[0]);
#ifdef DEBUG_LATTICE
		INT32 err = m_x[9] + matrix_multiply(m_current_k[9], m_u[9]); //x_10, real chip doesn't use or calculate this
#endif
		m_x[9] = m_x[8] + matrix_multiply(m_current_k[8], m_u[8]);
		m_x[8] = m_x[7] + matrix_multiply(m_current_k[7], m_u[7]);
		m_x[7] = m_x[6] + matrix_multiply(m_current_k[6], m_u[6]);
		m_x[6] = m_x[5] + matrix_multiply(m_current_k[5], m_u[5]);
		m_x[5] = m_x[4] + matrix_multiply(m_current_k[4], m_u[4]);
		m_x[4] = m_x[3] + matrix_multiply(m_current_k[3], m_u[3]);
		m_x[3] = m_x[2] + matrix_multiply(m_current_k[2], m_u[2]);
		m_x[2] = m_x[1] + matrix_multiply(m_current_k[1], m_u[1]);
		m_x[1] = m_x[0] + matrix_multiply(m_current_k[0], m_u[0]);
		m_x[0] = m_u[0];
		m_previous_energy = m_current_energy;
#ifdef DEBUG_LATTICE
		int i;
		fprintf(stderr,"V:%04d ", m_u[10]);
		for (i = 9; i >= 0; i--)
		{
			fprintf(stderr,"Y%d:%04d ", i+1, m_u[i]);
		}
		fprintf(stderr,"\n");
		fprintf(stderr,"E:%04d ", err);
		for (i = 9; i >= 0; i--)
		{
			fprintf(stderr,"b%d:%04d ", i+1, m_x[i]);
		}
		fprintf(stderr,"\n");
#endif
		return m_u[0];
}




/******************************************************************************************

     PDC_set -- set Processor Data Clock. Execute CTL_pins command on hi-lo transition.

******************************************************************************************/

void tms5110_device::PDC_set(int data)
{
	if (m_PDC != (data & 0x1) )
	{
		m_PDC = data & 0x1;
		if (m_PDC == 0) /* toggling 1->0 processes command on CTL_pins */
		{
#ifdef DEBUG_COMMAND_DUMP
			fprintf(stderr,"PDC falling edge(%02X): ",m_state);
#endif
			/* first pdc toggles output, next toggles input */
			switch (m_state)
			{
			case CTL_STATE_INPUT:
				/* continue */
				break;
			case CTL_STATE_NEXT_TTALK_OUTPUT:
#ifdef DEBUG_COMMAND_DUMP
				fprintf(stderr,"Switching CTL bus direction to output for Test Talk\n");
#endif
				m_state = CTL_STATE_TTALK_OUTPUT;
				return;
			case CTL_STATE_TTALK_OUTPUT:
#ifdef DEBUG_COMMAND_DUMP
				fprintf(stderr,"Switching CTL bus direction back to input from Test Talk\n");
#endif
				m_state = CTL_STATE_INPUT;
				return;
			case CTL_STATE_NEXT_OUTPUT:
#ifdef DEBUG_COMMAND_DUMP
				fprintf(stderr,"Switching CTL bus direction for Read Bit Buffer Output\n");
#endif
				m_state = CTL_STATE_OUTPUT;
				return;
			case CTL_STATE_OUTPUT:
#ifdef DEBUG_COMMAND_DUMP
				fprintf(stderr,"Switching CTL bus direction back to input from Read Bit Buffer Output\n");
#endif
				m_state = CTL_STATE_INPUT;
				return;
			}
			/* the only real commands we handle now are SPEAK and RESET */
			if (m_next_is_address)
			{
#ifdef DEBUG_COMMAND_DUMP
				fprintf(stderr,"Loading address nybble %02x to VSMs\n", m_CTL_pins);
#endif
				m_next_is_address = FALSE;
				m_address = m_address | ((m_CTL_pins & 0x0F)<<m_addr_bit);
				m_addr_bit = (m_addr_bit + 4) % 12;
				m_schedule_dummy_read = TRUE;
				new_int_write_addr(m_CTL_pins & 0x0F);
			}
			else
			{
#ifdef DEBUG_COMMAND_DUMP
				fprintf(stderr,"Got command nybble %02x: ", m_CTL_pins);
#endif
				switch (m_CTL_pins & 0xe) /*CTL1 - don't care*/
				{
				case TMS5110_CMD_RESET:
#ifdef DEBUG_COMMAND_DUMP
					fprintf(stderr,"RESET\n");
#endif
					perform_dummy_read();
					reset();
					break;

				case TMS5110_CMD_LOAD_ADDRESS:
#ifdef DEBUG_COMMAND_DUMP
					fprintf(stderr,"LOAD ADDRESS\n");
#endif
					m_next_is_address = TRUE;
					break;

				case TMS5110_CMD_OUTPUT:
#ifdef DEBUG_COMMAND_DUMP
					fprintf(stderr,"OUTPUT (from read-bit buffer)\n");
#endif
					m_state = CTL_STATE_NEXT_OUTPUT;
					break;

				case TMS5110_CMD_SPKSLOW:
#ifdef DEBUG_COMMAND_DUMP
					fprintf(stderr,"SPKSLOW\n");
#endif
					perform_dummy_read();
					m_SPEN = 1; /* start immediately */
#ifdef FAST_START_HACK
					m_TALK = 1;
#endif
					/* clear out variables before speaking */
					m_zpar = 1; // zero all the parameters
					m_uv_zpar = 1; // zero k4-k10 as well
					m_OLDE = 1; // 'silence/zpar' frames are zero energy
					m_OLDP = 1; // 'silence/zpar' frames are zero pitch
#ifdef PERFECT_INTERPOLATION_HACK
					m_old_zpar = 1; // zero all the old parameters
					m_old_uv_zpar = 1; // zero old k4-k10 as well
#endif
					m_subc_reload = 0; // SPKSLOW means this is 0
					break;

				case TMS5110_CMD_READ_BIT:
#ifdef DEBUG_COMMAND_DUMP
					fprintf(stderr,"READ BIT\n");
#endif
					if (m_schedule_dummy_read)
						perform_dummy_read();
					else
					{
#ifdef DEBUG_COMMAND_DUMP
						fprintf(stderr,"actually reading a bit now\n");
#endif
						m_CTL_buffer >>= 1;
						m_CTL_buffer |= (extract_bits(1)<<3);
						m_CTL_buffer &= 0xF;
					}
					break;

				case TMS5110_CMD_SPEAK:
#ifdef DEBUG_COMMAND_DUMP
					fprintf(stderr,"SPEAK\n");
#endif
					perform_dummy_read();
					m_SPEN = 1; /* start immediately */
#ifdef FAST_START_HACK
					m_TALK = 1;
#endif
					/* clear out variables before speaking */
					m_zpar = 1; // zero all the parameters
					m_uv_zpar = 1; // zero k4-k10 as well
					m_OLDE = 1; // 'silence/zpar' frames are zero energy
					m_OLDP = 1; // 'silence/zpar' frames are zero pitch
#ifdef PERFECT_INTERPOLATION_HACK
					m_old_zpar = 1; // zero all the old parameters
					m_old_uv_zpar = 1; // zero old k4-k10 as well
#endif
					m_subc_reload = 1; // SPEAK means this is 1
					break;

				case TMS5110_CMD_READ_BRANCH:
#ifdef DEBUG_COMMAND_DUMP
					fprintf(stderr,"READ AND BRANCH\n");
#endif
					new_int_write(0,1,1,0);
					new_int_write(1,1,1,0);
					new_int_write(0,1,1,0);
					new_int_write(0,0,0,0);
					new_int_write(1,0,0,0);
					new_int_write(0,0,0,0);
					m_schedule_dummy_read = FALSE;
					break;

				case TMS5110_CMD_TEST_TALK:
#ifdef DEBUG_COMMAND_DUMP
					fprintf(stderr,"TEST TALK\n");
#endif
					m_state = CTL_STATE_NEXT_TTALK_OUTPUT;
					break;

				default:
#ifdef DEBUG_COMMAND_DUMP
					fprintf(stderr,"tms5110.c: unknown command: 0x%02x\n", m_CTL_pins);
#endif
					break;
				}

			}
		}
	}
}



/******************************************************************************************

     parse_frame -- parse a new frame's worth of data; returns 0 if not enough bits in buffer

******************************************************************************************/

void tms5110_device::parse_frame()
{
	int i, rep_flag;
#ifdef PERFECT_INTERPOLATION_HACK
	m_old_uv_zpar = m_uv_zpar;
	m_old_zpar = m_zpar;
#endif
	// since we're parsing a frame, we must be talking, so clear zpar here
	// before we start parsing a frame, the P=0 and E=0 latches were both reset by RESETL4, so clear m_uv_zpar here
	m_uv_zpar = m_zpar = 0;

	// attempt to extract the energy index
	m_new_frame_energy_idx = extract_bits(m_coeff->energy_bits);
#ifdef DEBUG_PARSE_FRAME_DUMP
	printbits(m_new_frame_energy_idx,m_coeff->energy_bits);
	fprintf(stderr," ");
#endif

	// if the energy index is 0 or 15, we're done
	if ((m_new_frame_energy_idx == 0) || (m_new_frame_energy_idx == 15))
		return;

	rep_flag = extract_bits(1);
#ifdef DEBUG_PARSE_FRAME_DUMP
	printbits(rep_flag, 1);
	fprintf(stderr," ");
#endif

	m_new_frame_pitch_idx = extract_bits(m_coeff->pitch_bits);
#ifdef DEBUG_PARSE_FRAME_DUMP
	printbits(m_new_frame_pitch_idx,m_coeff->pitch_bits);
	fprintf(stderr," ");
#endif
	// if the new frame is unvoiced, be sure to zero out the k5-k10 parameters
	m_uv_zpar = NEW_FRAME_UNVOICED_FLAG;
	// if this is a repeat frame, just do nothing, it will reuse the old coefficients
	if (rep_flag)
		return;

	// extract first 4 K coefficients
	for (i = 0; i < 4; i++)
	{
		m_new_frame_k_idx[i] = extract_bits(m_coeff->kbits[i]);
#ifdef DEBUG_PARSE_FRAME_DUMP
		printbits(m_new_frame_k_idx[i],m_coeff->kbits[i]);
		fprintf(stderr," ");
#endif
	}

	// if the pitch index was zero, we only need 4 K's...
	if (m_new_frame_pitch_idx == 0)
	{
		/* and the rest of the coefficients are zeroed, but that's done in the generator code */
		return;
	}

	// If we got here, we need the remaining 6 K's
	for (i = 4; i < m_coeff->num_k; i++)
	{
		m_new_frame_k_idx[i] = extract_bits(m_coeff->kbits[i]);
#ifdef DEBUG_PARSE_FRAME_DUMP
		printbits(m_new_frame_k_idx[i],m_coeff->kbits[i]);
		fprintf(stderr," ");
#endif
	}
#ifdef VERBOSE
		logerror("Parsed a frame successfully in ROM\n");
#endif
	return;
}


#if 0
/*This is an example word TEN taken from the TMS5110A datasheet*/
static const unsigned int example_word_TEN[619]={
/* 1*/1,0,0,0,  0,  0,0,0,0,0,  1,1,0,0,0,  0,0,0,1,0,  0,1,1,1,    0,1,0,1,
/* 2*/1,0,0,0,  0,  0,0,0,0,0,  1,0,0,1,0,  0,0,1,1,0,  0,0,1,1,    0,1,0,1,
/* 3*/1,1,0,0,  0,  1,0,0,0,0,  1,0,1,0,0,  0,1,0,1,0,  0,1,0,0,    1,0,1,0,    1,0,0,0,    1,0,0,1,    0,1,0,1,    0,0,1,  0,1,0,  0,1,1,
/* 4*/1,1,1,0,  0,  0,1,1,1,1,  1,0,1,0,1,  0,1,1,1,0,  0,1,0,1,    0,1,1,1,    0,1,1,1,    1,0,1,1,    1,0,1,0,    0,1,1,  0,1,0,  0,1,1,
/* 5*/1,1,1,0,  0,  1,0,0,0,0,  1,0,1,0,0,  0,1,1,1,0,  0,1,0,1,    1,0,1,0,    1,0,0,0,    1,1,0,0,    1,0,1,1,    1,0,0,  0,1,0,  0,1,1,
/* 6*/1,1,1,0,  0,  1,0,0,0,1,  1,0,1,0,1,  0,1,1,0,1,  0,1,1,0,    0,1,1,1,    0,1,1,1,    1,0,1,0,    1,0,1,0,    1,1,0,  0,0,1,  1,0,0,
/* 7*/1,1,1,0,  0,  1,0,0,1,0,  1,0,1,1,1,  0,1,1,1,0,  0,1,1,1,    0,1,1,1,    0,1,0,1,    0,1,1,0,    1,0,0,1,    1,1,0,  0,1,0,  0,1,1,
/* 8*/1,1,1,0,  1,  1,0,1,0,1,
/* 9*/1,1,1,0,  0,  1,1,0,0,1,  1,0,1,1,1,  0,1,0,1,1,  1,0,1,1,    0,1,1,1,    0,1,0,0,    1,0,0,0,    1,0,0,0,    1,1,0,  0,1,1,  0,1,1,
/*10*/1,1,0,1,  0,  1,1,0,1,0,  1,0,1,0,1,  0,1,1,0,1,  1,0,1,1,    0,1,0,1,    0,1,0,0,    1,0,0,0,    1,0,1,0,    1,1,0,  0,1,0,  1,0,0,
/*11*/1,0,1,1,  0,  1,1,0,1,1,  1,0,0,1,1,  1,0,0,1,0,  0,1,1,0,    0,0,1,1,    0,1,0,1,    1,0,0,1,    1,0,1,0,    1,0,0,  0,1,1,  0,1,1,
/*12*/1,0,0,0,  0,  1,1,1,0,0,  1,0,0,1,1,  0,0,1,1,0,  0,1,0,0,    0,1,1,0,    1,1,0,0,    0,1,0,1,    1,0,0,0,    1,0,0,  0,1,0,  1,0,1,
/*13*/0,1,1,1,  1,  1,1,1,0,1,
/*14*/0,1,1,1,  0,  1,1,1,1,0,  1,0,0,1,1,  0,0,1,1,1,  0,1,0,1,    0,1,0,1,    1,1,0,0,    0,1,1,1,    1,0,0,0,    1,0,0,  0,1,0,  1,0,1,
/*15*/0,1,1,0,  0,  1,1,1,1,0,  1,0,1,0,1,  0,0,1,1,0,  0,1,0,0,    0,0,1,1,    1,1,0,0,    1,0,0,1,    0,1,1,1,    1,0,1,  0,1,0,  1,0,1,
/*16*/1,1,1,1
};
#endif


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tms5110_device::device_start()
{
	m_table = region()->base();

	set_variant(TMS5110_IS_TMS5110A);

	/* resolve lines */
	m_m0_cb.resolve();
	m_m1_cb.resolve();
	m_romclk_cb.resolve();
	m_addr_cb.resolve();
	m_data_cb.resolve();

	/* initialize a stream */
	m_stream = machine().sound().stream_alloc(*this, 0, 1, clock() / 80);

	m_state = CTL_STATE_INPUT; /* most probably not defined */
	m_romclk_hack_timer = timer_alloc(0);

	register_for_save_states();
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tms5100_device::device_start()
{
	tms5110_device::device_start();
	set_variant(TMS5110_IS_TMC0281);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tmc0281_device::device_start()
{
	tms5110_device::device_start();
	set_variant(TMS5110_IS_TMC0281);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tms5100a_device::device_start()
{
	tms5110_device::device_start();
	set_variant(TMS5110_IS_TMC0281D);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tmc0281d_device::device_start()
{
	tms5110_device::device_start();
	set_variant(TMS5110_IS_TMC0281D);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cd2801_device::device_start()
{
	tms5110_device::device_start();
	set_variant(TMS5110_IS_CD2801);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cd2802_device::device_start()
{
	tms5110_device::device_start();
	set_variant(TMS5110_IS_CD2802);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tms5110a_device::device_start()
{
	tms5110_device::device_start();
	set_variant(TMS5110_IS_TMS5110A);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void m58817_device::device_start()
{
	tms5110_device::device_start();
	set_variant(TMS5110_IS_M58817);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tms5110_device::device_reset()
{
	m_digital_select = FORCE_DIGITAL; // assume analog output

	/* initialize the chip state */
	m_SPEN = m_TALK = m_TALKD = 0;
	m_CTL_pins = 0;
	m_RNG = 0x1fff;
	m_CTL_buffer = 0;
	m_PDC = 0;

	/* initialize the energy/pitch/k states */
#ifdef PERFECT_INTERPOLATION_HACK
	m_old_frame_energy_idx = m_old_frame_pitch_idx = 0;
	memset(m_old_frame_k_idx, 0, sizeof(m_old_frame_k_idx));
	m_old_zpar = m_old_uv_zpar = 0;
#endif
	m_new_frame_energy_idx = m_current_energy = m_previous_energy = 0;
	m_new_frame_pitch_idx = m_current_pitch = 0;
	m_zpar = m_uv_zpar = 0;
	memset(m_new_frame_k_idx, 0, sizeof(m_new_frame_k_idx));
	memset(m_current_k, 0, sizeof(m_current_k));

	/* initialize the sample generators */
	m_inhibit = 1;
	m_subcycle = m_pitch_count = m_pitch_zero = m_PC = m_zpar = 0;
	m_subc_reload = 1;
	m_OLDE = m_OLDP = 1;
	m_IP = 0;
	m_RNG = 0x1FFF;
	memset(m_u, 0, sizeof(m_u));
	memset(m_x, 0, sizeof(m_x));
	if (m_table != NULL)
	{
		/* legacy interface */
		m_schedule_dummy_read = TRUE;
	}
	else
	{
		/* no dummy read! This makes bagman and ad2083 speech fail
		 * with the new cycle and transition exact interfaces
		 */
		m_schedule_dummy_read = FALSE;
	}
	m_next_is_address = FALSE;
	m_address = 0;
	m_addr_bit = 0;
}



/******************************************************************************

     tms5110_ctl_w -- write Control Command to the sound chip
commands like Speech, Reset, etc., are loaded into the chip via the CTL pins

******************************************************************************/

WRITE8_MEMBER( tms5110_device::ctl_w )
{
	/* bring up to date first */
	m_stream->update();
	m_CTL_pins = data & 0xf;
}


/******************************************************************************

     tms5110_pdc_w -- write to PDC pin on the sound chip

******************************************************************************/

WRITE_LINE_MEMBER( tms5110_device::pdc_w )
{
	/* bring up to date first */
	m_stream->update();
	PDC_set(state);
}



/******************************************************************************

     tms5110_ctl_r -- read from the VSP (51xx) control bus
        The CTL bus can be in three states:
        1. Test talk output:
            bit 0 = TS - Talk Status is active (high) when the VSP is processing speech data.
                Talk Status goes active at the initiation of a SPEAK command.
                It goes inactive (low) when the stop code (Energy=1111) is processed, or
                immediately(?????? not TMS5110) by a RESET command.
            other bits may be open bus
        2. 'read bit' buffer contents output:
            bits 0-3 = buffer contents
        3. Input 'open bus' state:
            bits 0-3 = high-z

******************************************************************************/

READ8_MEMBER( tms5110_device::ctl_r )
{
	/* bring up to date first */
	m_stream->update();
	if (m_state == CTL_STATE_TTALK_OUTPUT)
	{
		if (DEBUG_5110) logerror("Status read while outputting Test Talk (status=%2d)\n", TALK_STATUS);
		return (TALK_STATUS << 0); /*CTL1 = still talking ? */
	}
	else if (m_state == CTL_STATE_OUTPUT)
	{
		if (DEBUG_5110) logerror("Status read while outputting buffer (buffer=%2d)\n", m_CTL_buffer);
		return (m_CTL_buffer);
	}
	else // we're reading with the bus in input mode! just return the last thing written to the bus
	{
		if (DEBUG_5110) logerror("Status read (not in output mode), returning %02x\n", m_CTL_pins);
		return (m_CTL_pins);
	}
}

READ8_MEMBER( m58817_device::status_r )
{
	/* bring up to date first */
	m_stream->update();
	return (TALK_STATUS << 0); /*CTL1 = still talking ? */
}

/******************************************************************************

     tms5110_romclk_hack_r -- read status of romclk

******************************************************************************/

void tms5110_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	m_romclk_hack_state = !m_romclk_hack_state;
}

READ8_MEMBER( tms5110_device::romclk_hack_r )
{
	/* bring up to date first */
	m_stream->update();

	/* create and start timer if necessary */
	if (!m_romclk_hack_timer_started)
	{
		m_romclk_hack_timer_started = TRUE;
		m_romclk_hack_timer->adjust(attotime::from_hz(clock() / 40), 0, attotime::from_hz(clock() / 40));
	}
	return m_romclk_hack_state;
}


/******************************************************************************

     tms5110_update -- update the sound chip so that it is in sync with CPU execution

******************************************************************************/

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void tms5110_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	INT16 sample_data[MAX_SAMPLE_CHUNK];
	stream_sample_t *buffer = outputs[0];

	/* loop while we still have samples to generate */
	while (samples)
	{
		int length = (samples > MAX_SAMPLE_CHUNK) ? MAX_SAMPLE_CHUNK : samples;
		int index;

		/* generate the samples and copy to the target buffer */
		process(sample_data, length);
		for (index = 0; index < length; index++)
			*buffer++ = sample_data[index];

		/* account for the samples */
		samples -= length;
	}
}



/******************************************************************************

     tms5110_set_frequency -- adjusts the playback frequency
     TODO: kill this function; we should be adjusting the tms51xx device clock itself,
     not setting it here!

******************************************************************************/

void tms5110_device::set_frequency(int frequency)
{
	m_stream->set_sample_rate(frequency / 80);
}



/* from here on in this file is a VSM 'Emulator' circuit used by bagman and ad2083 */

/*
 *
 * General Interface design (Bagman)
 *
 *                         +------------------------------------------------------------------------+
 *                         |                                                                        |
 *       +-------------+   |           +-------------+       +-------------+       +-------------+  |
 *       | TMS5100     |   |           | Counters    |       | Rom(s)      |       | Decoder     |  |
 *       |        ADD8 |<--+           | LS393s      |       |             |       |             |  |
 *       |             |               |             |       |             |       |        Out  |--+
 *       |          M0 |---+           |     Address |======>| Address     |       | IN1         |
 *       |             |   |           |             |       |       Data  |======>| ...         |
 *   M   |             |   +---------->| Clk         |       |             |       | IN8         |
 *   A-->| CTL1        |               |             |       |             |       |             |
 *   P-->| CTL2        |          +--->| Reset       |       |             |       |             |
 *   P-->| CTL3        |          |    |             |       |             |       |    A  B  C  |
 *   E-->| CTL4        |          |    +-------------+       +-------------+       +-------------+
 *   D-->| PDC         |          |                                                     ^  ^  ^
 *       |             |          +-------------------------------------------------+   |  |  |
 *       |             |                                                            |   Bit Select
 *       |      ROMCLK |---+           +-------------+       +-------------+        |
 *       |             |   |           | Counter     |       | PROM        |        |
 *       +-------------+   |           | LS393       |       |          D1 |   M  --+ Reset Bit
 *                         |           |          Q0 |------>| A0          |   A
 *                         +---------->| Clk      Q1 |------>| A1          |   P ==>  CTL1 ... CTL4
 *                                     |          Q2 |------>| A2          |   P -->  PDC
 *                                     | Reset    Q3 |------>| A3          |   E  --+ Stop Bit
 *                                     |             |   +-->| A4       D8 |   D    |
 *                                     +-------------+   |   +-------------+        |
 *                                                       |                          |
 *                                                       |   +---+                  |
 *                                                       |   |   |<-----------------+
 *                                                       +---| & |
 *                                                           |   |<-------- Enable
 *                                                           +---+
 *
 */

/******************************************************************************

     device_start( tmsprom ) -- allocate buffers initialize

******************************************************************************/

void tmsprom_device::register_for_save_states()
{
	save_item(NAME(m_address));
	save_item(NAME(m_base_address));
	save_item(NAME(m_bit));
	save_item(NAME(m_enable));
	save_item(NAME(m_prom_cnt));
	save_item(NAME(m_m0));
}

void tmsprom_device::update_prom_cnt()
{
	UINT8 prev_val = m_prom[m_prom_cnt] | 0x0200;
	if (m_enable && (prev_val & (1<<m_stop_bit)))
		m_prom_cnt |= 0x10;
	else
		m_prom_cnt &= 0x0f;
}

void tmsprom_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	/* only 16 bytes needed ... The original dump is bad. This
	 * is what is needed to get speech to work. The prom data has
	 * been updated and marked as BAD_DUMP. The information below
	 * is given for reference once another dump should surface.
	 *
	 * static const int prom[16] = {0x00, 0x00, 0x02, 0x00, 0x00, 0x02, 0x00, 0x00,
	 *              0x02, 0x00, 0x40, 0x00, 0x04, 0x06, 0x04, 0x84 };
	 */
	UINT16 ctrl;

	update_prom_cnt();
	ctrl = (m_prom[m_prom_cnt] | 0x200);

	//if (m_enable && m_prom_cnt < 0x10) printf("ctrl %04x, enable %d cnt %d\n", ctrl, m_enable, m_prom_cnt);
	m_prom_cnt = ((m_prom_cnt + 1) & 0x0f) | (m_prom_cnt & 0x10);

	if (ctrl & (1 << m_reset_bit))
		m_address = 0;

	m_ctl_cb((offs_t)0, BITSWAP8(ctrl,0,0,0,0,m_ctl8_bit,
			m_ctl4_bit,m_ctl2_bit,m_ctl1_bit));

	m_pdc_cb((ctrl >> m_pdc_bit) & 0x01);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tmsprom_device::device_start()
{
	/* resolve lines */
	m_pdc_cb.resolve_safe();
	m_ctl_cb.resolve_safe();

	m_rom = region()->base();
	assert_always(m_rom != NULL, "Error creating TMSPROM chip: No rom region found");
	m_prom = owner()->memregion(m_prom_region)->base();
	assert_always(m_prom != NULL, "Error creating TMSPROM chip: No prom region found");

	m_romclk_timer = timer_alloc(0);
	m_romclk_timer->adjust(attotime::zero, 0, attotime::from_hz(clock()));

	m_bit = 0;
	m_base_address = 0;
	m_address = 0;
	m_enable = 0;
	m_m0 = 0;
	m_prom_cnt = 0;

	register_for_save_states();
}

WRITE_LINE_MEMBER( tmsprom_device::m0_w )
{
	/* falling edge counts */
	if (m_m0 && !state)
	{
		m_address += 1;
		m_address &= (m_rom_size-1);
	}
	m_m0 = state;
}

READ_LINE_MEMBER( tmsprom_device::data_r )
{
	return (m_rom[m_base_address + m_address] >> m_bit) & 0x01;
}


WRITE8_MEMBER( tmsprom_device::rom_csq_w )
{
	if (!data)
		m_base_address = offset * m_rom_size;
}

WRITE8_MEMBER( tmsprom_device::bit_w )
{
	m_bit = data;
}

WRITE_LINE_MEMBER( tmsprom_device::enable_w )
{
	if (state != m_enable)
	{
		m_enable = state;
		update_prom_cnt();

		/* the following is needed for ad2084.
		 * It is difficult to derive the actual connections from
		 * pcb pictures but the reset pin of the LS393 driving
		 * the prom address line is connected somewhere.
		 *
		 * This does not affect bagman. It just simulates that a
		 * write to ads3 is always happening when the four lower
		 * counter bits are 0!
		 */
		if (state)
			m_prom_cnt &= 0x10;
	}
}


/*-------------------------------------------------
    TMS 5110 device definition
-------------------------------------------------*/

const device_type TMS5110 = &device_creator<tms5110_device>;

tms5110_device::tms5110_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TMS5110, "TMS5110", tag, owner, clock, "tms5110", __FILE__),
		device_sound_interface(mconfig, *this),
		m_m0_cb(*this),
		m_m1_cb(*this),
		m_addr_cb(*this),
		m_data_cb(*this),
		m_romclk_cb(*this)
{
}

tms5110_device::tms5110_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_sound_interface(mconfig, *this),
		m_m0_cb(*this),
		m_m1_cb(*this),
		m_addr_cb(*this),
		m_data_cb(*this),
		m_romclk_cb(*this)
{
}


const device_type TMS5100 = &device_creator<tms5100_device>;

tms5100_device::tms5100_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms5110_device(mconfig, TMS5100, "TMS5100", tag, owner, clock, "tms5100", __FILE__)
{
}

const device_type TMC0281 = &device_creator<tmc0281_device>;

tmc0281_device::tmc0281_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms5110_device(mconfig, TMC0281, "TMC0281", tag, owner, clock, "tmc0281", __FILE__)
{
}

const device_type TMS5100A = &device_creator<tms5100a_device>;

tms5100a_device::tms5100a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms5110_device(mconfig, TMS5100A, "TMS5100A", tag, owner, clock, "tms5100a", __FILE__)
{
}

const device_type TMC0281D = &device_creator<tmc0281d_device>;

tmc0281d_device::tmc0281d_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms5110_device(mconfig, TMC0281D, "TMC0281D", tag, owner, clock, "tmc0281d", __FILE__)
{
}

const device_type CD2801 = &device_creator<cd2801_device>;

cd2801_device::cd2801_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms5110_device(mconfig, CD2801, "CD2801", tag, owner, clock, "cd2801", __FILE__)
{
}

const device_type CD2802 = &device_creator<cd2802_device>;

cd2802_device::cd2802_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms5110_device(mconfig, CD2802, "CD2802", tag, owner, clock, "cd2802", __FILE__)
{
}

const device_type TMS5110A = &device_creator<tms5110a_device>;

tms5110a_device::tms5110a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms5110_device(mconfig, TMS5110A, "TMS5110A", tag, owner, clock, "tms5110a", __FILE__)
{
}

const device_type M58817 = &device_creator<m58817_device>;

m58817_device::m58817_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms5110_device(mconfig, M58817, "M58817", tag, owner, clock, "m58817", __FILE__)
{
}


const device_type TMSPROM = &device_creator<tmsprom_device>;

tmsprom_device::tmsprom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TMSPROM, "TMSPROM", tag, owner, clock, "tmsprom", __FILE__),
		m_prom_region(""),
		m_rom_size(0),
		m_pdc_bit(0),
		m_ctl1_bit(0),
		m_ctl2_bit(0),
		m_ctl4_bit(0),
		m_ctl8_bit(0),
		m_reset_bit(0),
		m_stop_bit(0),
		m_pdc_cb(*this),
		m_ctl_cb(*this)
{
}
