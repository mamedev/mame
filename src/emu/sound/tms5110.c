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
        - implement missing commands
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

#define MAX_SAMPLE_CHUNK        512

/* Variants */

#define TMS5110_IS_5110A    (1)
#define TMS5110_IS_5100     (2)
#define TMS5110_IS_5110     (3)

#define TMS5110_IS_CD2801   TMS5110_IS_5100
#define TMS5110_IS_TMC0281  TMS5110_IS_5100

#define TMS5110_IS_CD2802   TMS5110_IS_5110
#define TMS5110_IS_M58817   TMS5110_IS_5110

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
		case TMS5110_IS_5110A:
			m_coeff = &tms5110a_coeff;
			break;
		case TMS5110_IS_5100:
			m_coeff = &pat4209836_coeff;
			break;
		case TMS5110_IS_5110:
			m_coeff = &pat4403965_coeff;
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
	save_item(NAME(m_fifo));
	save_item(NAME(m_fifo_head));
	save_item(NAME(m_fifo_tail));
	save_item(NAME(m_fifo_count));

	save_item(NAME(m_PDC));
	save_item(NAME(m_CTL_pins));
	save_item(NAME(m_speaking_now));
	save_item(NAME(m_talk_status));
	save_item(NAME(m_state));

	save_item(NAME(m_old_energy));
	save_item(NAME(m_old_pitch));
	save_item(NAME(m_old_k));

	save_item(NAME(m_new_energy));
	save_item(NAME(m_new_pitch));
	save_item(NAME(m_new_k));

	save_item(NAME(m_current_energy));
	save_item(NAME(m_current_pitch));
	save_item(NAME(m_current_k));

	save_item(NAME(m_target_energy));
	save_item(NAME(m_target_pitch));
	save_item(NAME(m_target_k));

	save_item(NAME(m_interp_count));
	save_item(NAME(m_sample_count));
	save_item(NAME(m_pitch_count));

	save_item(NAME(m_next_is_address));
	save_item(NAME(m_address));
	save_item(NAME(m_schedule_dummy_read));
	save_item(NAME(m_addr_bit));
	save_item(NAME(m_CTL_buffer));

	save_item(NAME(m_x));

	save_item(NAME(m_RNG));
}




/******************************************************************************************

     FIFO_data_write -- handle bit data write to the TMS5110 (as a result of toggling M0 pin)

******************************************************************************************/
void tms5110_device::FIFO_data_write(int data)
{
	/* add this bit to the FIFO */
	if (m_fifo_count < FIFO_SIZE)
	{
		m_fifo[m_fifo_tail] = (data&1); /* set bit to 1 or 0 */

		m_fifo_tail = (m_fifo_tail + 1) % FIFO_SIZE;
		m_fifo_count++;

		if (DEBUG_5110) logerror("Added bit to FIFO (size=%2d)\n", m_fifo_count);
	}
	else
	{
		if (DEBUG_5110) logerror("Ran out of room in the FIFO!\n");
	}
}

/******************************************************************************************

     extract_bits -- extract a specific number of bits from the FIFO

******************************************************************************************/

int tms5110_device::extract_bits(int count)
{
	int val = 0;
	if (DEBUG_5110) logerror("requesting %d bits from fifo: ", count);
	while (count--)
	{
		val = (val << 1) | (m_fifo[m_fifo_head] & 1);
		m_fifo_count--;
		m_fifo_head = (m_fifo_head + 1) % FIFO_SIZE;
	}
	if (DEBUG_5110) logerror("returning: %02x\n", val);
	return val;
}

void tms5110_device::request_bits(int no)
{
	for (int i = 0; i < no; i++)
	{
		UINT8 data = new_int_read();
		if (DEBUG_5110) logerror("bit added to fifo: %d\n", data);
		FIFO_data_write(data);
	}
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
	int i, interp_period, bitout;
	INT16 Y11, cliptemp;

	/* if we're not speaking, fill with nothingness */
	if (!m_speaking_now)
		goto empty;

	/* if we're to speak, but haven't started */
	if (!m_talk_status)
	{
	/* a "dummy read" is mentioned in the tms5200 datasheet */
	/* The Bagman speech roms data are organized in such a way that
	** the bit at address 0 is NOT a speech data. The bit at address 1
	** is the speech data. It seems that the tms5110 performs a dummy read
	** just before it executes a SPEAK command.
	** This has been moved to command logic ...
	**  perform_dummy_read();
	*/

		/* clear out the new frame parameters (it will become old frame just before the first call to parse_frame() ) */
		m_new_energy = 0;
		m_new_pitch = 0;
		for (i = 0; i < m_coeff->num_k; i++)
			m_new_k[i] = 0;

		m_talk_status = 1;
	}


	/* loop until the buffer is full or we've stopped speaking */
	while ((size > 0) && m_speaking_now)
	{
		int current_val;

		/* if we're ready for a new frame */
		if ((m_interp_count == 0) && (m_sample_count == 0))
		{
			/* remember previous frame */
			m_old_energy = m_new_energy;
			m_old_pitch = m_new_pitch;
			for (i = 0; i < m_coeff->num_k; i++)
				m_old_k[i] = m_new_k[i];


			/* if the old frame was a stop frame, exit and do not process any more frames */
			if (m_old_energy == COEFF_ENERGY_SENTINEL)
			{
				if (DEBUG_5110) logerror("processing frame: stop frame\n");
				m_target_energy = m_current_energy = 0;
				m_speaking_now = m_talk_status = 0;
				m_interp_count = m_sample_count = m_pitch_count = 0;
				goto empty;
			}


			/* Parse a new frame into the new_energy, new_pitch and new_k[] */
			parse_frame();


			/* Set old target as new start of frame */
			m_current_energy = m_old_energy;
			m_current_pitch = m_old_pitch;

			for (i = 0; i < m_coeff->num_k; i++)
				m_current_k[i] = m_old_k[i];


			/* is this the stop (ramp down) frame? */
			if (m_new_energy == COEFF_ENERGY_SENTINEL)
			{
				/*logerror("processing frame: ramp down\n");*/
				m_target_energy = 0;
				m_target_pitch = m_old_pitch;
				for (i = 0; i < m_coeff->num_k; i++)
					m_target_k[i] = m_old_k[i];
			}
			else if ((m_old_energy == 0) && (m_new_energy != 0)) /* was the old frame a zero-energy frame? */
			{
				/* if so, and if the new frame is non-zero energy frame then the new parameters
				   should become our current and target parameters immediately,
				   i.e. we should NOT interpolate them slowly in.
				*/

				/*logerror("processing non-zero energy frame after zero-energy frame\n");*/
				m_target_energy = m_new_energy;
				m_target_pitch = m_current_pitch = m_new_pitch;
				for (i = 0; i < m_coeff->num_k; i++)
					m_target_k[i] = m_current_k[i] = m_new_k[i];
			}
			else if ((m_old_pitch == 0) && (m_new_pitch != 0))    /* is this a change from unvoiced to voiced frame ? */
			{
				/* if so, then the new parameters should become our current and target parameters immediately,
				   i.e. we should NOT interpolate them slowly in.
				*/
				/*if (DEBUG_5110) logerror("processing frame: UNVOICED->VOICED frame change\n");*/
				m_target_energy = m_new_energy;
				m_target_pitch = m_current_pitch = m_new_pitch;
				for (i = 0; i < m_coeff->num_k; i++)
					m_target_k[i] = m_current_k[i] = m_new_k[i];
			}
			else if ((m_old_pitch != 0) && (m_new_pitch == 0))    /* is this a change from voiced to unvoiced frame ? */
			{
				/* if so, then the new parameters should become our current and target parameters immediately,
				   i.e. we should NOT interpolate them slowly in.
				*/
				/*if (DEBUG_5110) logerror("processing frame: VOICED->UNVOICED frame change\n");*/
				m_target_energy = m_new_energy;
				m_target_pitch = m_current_pitch = m_new_pitch;
				for (i = 0; i < m_coeff->num_k; i++)
					m_target_k[i] = m_current_k[i] = m_new_k[i];
			}
			else
			{
				/*logerror("processing frame: Normal\n");*/
				/*logerror("*** Energy = %d\n",current_energy);*/
				/*logerror("proc: %d %d\n",last_fbuf_head,fbuf_head);*/

				m_target_energy = m_new_energy;
				m_target_pitch = m_new_pitch;
				for (i = 0; i < m_coeff->num_k; i++)
					m_target_k[i] = m_new_k[i];
			}
		}
		else
		{
			interp_period = m_sample_count / 25;
			switch(m_interp_count)
			{
				/*         PC=X  X cycle, rendering change (change for next cycle which chip is actually doing) */
				case 0: /* PC=0, A cycle, nothing happens (calc energy) */
				break;
				case 1: /* PC=0, B cycle, nothing happens (update energy) */
				break;
				case 2: /* PC=1, A cycle, update energy (calc pitch) */
				m_current_energy += ((m_target_energy - m_current_energy) >> m_coeff->interp_coeff[interp_period]);
				break;
				case 3: /* PC=1, B cycle, nothing happens (update pitch) */
				break;
				case 4: /* PC=2, A cycle, update pitch (calc K1) */
				m_current_pitch += ((m_target_pitch - m_current_pitch) >> m_coeff->interp_coeff[interp_period]);
				break;
				case 5: /* PC=2, B cycle, nothing happens (update K1) */
				break;
				case 6: /* PC=3, A cycle, update K1 (calc K2) */
				m_current_k[0] += ((m_target_k[0] - m_current_k[0]) >> m_coeff->interp_coeff[interp_period]);
				break;
				case 7: /* PC=3, B cycle, nothing happens (update K2) */
				break;
				case 8: /* PC=4, A cycle, update K2 (calc K3) */
				m_current_k[1] += ((m_target_k[1] - m_current_k[1]) >> m_coeff->interp_coeff[interp_period]);
				break;
				case 9: /* PC=4, B cycle, nothing happens (update K3) */
				break;
				case 10: /* PC=5, A cycle, update K3 (calc K4) */
				m_current_k[2] += ((m_target_k[2] - m_current_k[2]) >> m_coeff->interp_coeff[interp_period]);
				break;
				case 11: /* PC=5, B cycle, nothing happens (update K4) */
				break;
				case 12: /* PC=6, A cycle, update K4 (calc K5) */
				m_current_k[3] += ((m_target_k[3] - m_current_k[3]) >> m_coeff->interp_coeff[interp_period]);
				break;
				case 13: /* PC=6, B cycle, nothing happens (update K5) */
				break;
				case 14: /* PC=7, A cycle, update K5 (calc K6) */
				m_current_k[4] += ((m_target_k[4] - m_current_k[4]) >> m_coeff->interp_coeff[interp_period]);
				break;
				case 15: /* PC=7, B cycle, nothing happens (update K6) */
				break;
				case 16: /* PC=8, A cycle, update K6 (calc K7) */
				m_current_k[5] += ((m_target_k[5] - m_current_k[5]) >> m_coeff->interp_coeff[interp_period]);
				break;
				case 17: /* PC=8, B cycle, nothing happens (update K7) */
				break;
				case 18: /* PC=9, A cycle, update K7 (calc K8) */
				m_current_k[6] += ((m_target_k[6] - m_current_k[6]) >> m_coeff->interp_coeff[interp_period]);
				break;
				case 19: /* PC=9, B cycle, nothing happens (update K8) */
				break;
				case 20: /* PC=10, A cycle, update K8 (calc K9) */
				m_current_k[7] += ((m_target_k[7] - m_current_k[7]) >> m_coeff->interp_coeff[interp_period]);
				break;
				case 21: /* PC=10, B cycle, nothing happens (update K9) */
				break;
				case 22: /* PC=11, A cycle, update K9 (calc K10) */
				m_current_k[8] += ((m_target_k[8] - m_current_k[8]) >> m_coeff->interp_coeff[interp_period]);
				break;
				case 23: /* PC=11, B cycle, nothing happens (update K10) */
				break;
				case 24: /* PC=12, A cycle, update K10 (do nothing) */
				m_current_k[9] += ((m_target_k[9] - m_current_k[9]) >> m_coeff->interp_coeff[interp_period]);
				break;
			}
		}


		/* calculate the output */

		if (m_current_energy == 0)
		{
			/* generate silent samples here */
			current_val = 0x00;
		}
		else if (m_old_pitch == 0)
		{
			/* generate unvoiced samples here */
			if (m_RNG&1)
				current_val = -64; /* according to the patent it is (either + or -) half of the maximum value in the chirp table */
			else
				current_val = 64;

		}
		else
		{
						/* generate voiced samples here */
			/* US patent 4331836 Figure 14B shows, and logic would hold, that a pitch based chirp
			 * function has a chirp/peak and then a long chain of zeroes.
			 * The last entry of the chirp rom is at address 0b110011 (50d), the 51st sample,
			 * and if the address reaches that point the ADDRESS incrementer is
			 * disabled, forcing all samples beyond 50d to be == 50d
			 * (address 50d holds zeroes)
			 */

		/*if (m_coeff->subtype & (SUBTYPE_TMS5100 | SUBTYPE_M58817))*/

		if (m_pitch_count > 50)
			current_val = m_coeff->chirptable[50];
		else
			current_val = m_coeff->chirptable[m_pitch_count];
		}

		/* Update LFSR *20* times every sample, like patent shows */
		for (i=0; i<20; i++)
		{
			bitout = ((m_RNG>>12)&1) ^
					((m_RNG>>10)&1) ^
					((m_RNG>> 9)&1) ^
					((m_RNG>> 0)&1);
			m_RNG >>= 1;
			m_RNG |= (bitout<<12);
		}

		/* Lattice filter here */

		Y11 = (current_val * 64 * m_current_energy) / 512;

		for (i = m_coeff->num_k - 1; i >= 0; i--)
		{
			Y11 = Y11 - ((m_current_k[i] * m_x[i]) / 512);
			m_x[i+1] = m_x[i] + ((m_current_k[i] * Y11) / 512);
		}

		m_x[0] = Y11;


		/* clipping & wrapping, just like the patent */

		/* YL10 - YL4 ==> DA6 - DA0 */
		cliptemp = Y11 / 16;

		/* M58817 seems to be different */
		if (m_coeff->subtype & (SUBTYPE_M58817))
			cliptemp = cliptemp / 2;

		if (cliptemp > 511) cliptemp = -512 + (cliptemp-511);
		else if (cliptemp < -512) cliptemp = 511 - (cliptemp+512);

		if (cliptemp > 127)
			buffer[buf_count] = 127*256;
		else if (cliptemp < -128)
			buffer[buf_count] = -128*256;
		else
			buffer[buf_count] = cliptemp *256;

		/* Update all counts */

		m_sample_count = (m_sample_count + 1) % 200;

		if (m_current_pitch != 0)
		{
			m_pitch_count++;
			if (m_pitch_count >= m_current_pitch)
				m_pitch_count = 0;
		}
		else
			m_pitch_count = 0;

		m_interp_count = (m_interp_count + 1) % 25;

		buf_count++;
		size--;
	}

empty:

	while (size > 0)
	{
		m_sample_count = (m_sample_count + 1) % 200;
		m_interp_count = (m_interp_count + 1) % 25;

		buffer[buf_count] = 0x00;
		buf_count++;
		size--;
	}
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
			if (DEBUG_5110) logerror("PDC falling edge: ");
			/* first pdc toggles output, next toggles input */
			switch (m_state)
			{
			case CTL_STATE_INPUT:
				/* continue */
				break;
			case CTL_STATE_NEXT_TTALK_OUTPUT:
				if (DEBUG_5110) logerror("Switching CTL bus direction to output for Test Talk\n");
				m_state = CTL_STATE_TTALK_OUTPUT;
				return;
			case CTL_STATE_TTALK_OUTPUT:
				if (DEBUG_5110) logerror("Switching CTL bus direction back to input from Test Talk\n");
				m_state = CTL_STATE_INPUT;
				return;
			case CTL_STATE_NEXT_OUTPUT:
				if (DEBUG_5110) logerror("Switching CTL bus direction for Read Bit Buffer Output\n");
				m_state = CTL_STATE_OUTPUT;
				return;
			case CTL_STATE_OUTPUT:
				if (DEBUG_5110) logerror("Switching CTL bus direction back to input from Read Bit Buffer Output\n");
				m_state = CTL_STATE_INPUT;
				return;
			}
			/* the only real commands we handle now are SPEAK and RESET */
			if (m_next_is_address)
			{
				if (DEBUG_5110) logerror("Loading address nybble %02x to VSMs\n", m_CTL_pins);
				m_next_is_address = FALSE;
				m_address = m_address | ((m_CTL_pins & 0x0F)<<m_addr_bit);
				m_addr_bit = (m_addr_bit + 4) % 12;
				m_schedule_dummy_read = TRUE;
				new_int_write_addr(m_CTL_pins & 0x0F);
			}
			else
			{
				if (DEBUG_5110) logerror("Got command nybble %02x: ", m_CTL_pins);
				switch (m_CTL_pins & 0xe) /*CTL1 - don't care*/
				{
				case TMS5110_CMD_RESET:
					if (DEBUG_5110) logerror("RESET\n");
					perform_dummy_read();
					reset();
					break;

				case TMS5110_CMD_LOAD_ADDRESS:
					if (DEBUG_5110) logerror("LOAD ADDRESS\n");
					m_next_is_address = TRUE;
					break;

				case TMS5110_CMD_OUTPUT:
					if (DEBUG_5110) logerror("OUTPUT (from read-bit buffer)\n");
					m_state = CTL_STATE_NEXT_OUTPUT;
					break;

				case TMS5110_CMD_SPKSLOW:
					if (DEBUG_5110) logerror("SPKSLOW (todo: this isn't implemented right yet)\n");
					perform_dummy_read();
					m_speaking_now = 1;
					//should FIFO be cleared now ????? there is no fifo! the fifo is a lie!
					break;

				case TMS5110_CMD_READ_BIT:
					if (DEBUG_5110) logerror("READ BIT\n");
					if (m_schedule_dummy_read)
						perform_dummy_read();
					else
					{
						if (DEBUG_5110) logerror("actually reading a bit now\n");
						request_bits(1);
						m_CTL_buffer >>= 1;
						m_CTL_buffer |= (extract_bits(1)<<3);
						m_CTL_buffer &= 0xF;
					}
					break;

				case TMS5110_CMD_SPEAK:
					if (DEBUG_5110) logerror("SPEAK\n");
					perform_dummy_read();
					m_speaking_now = 1;
					//should FIFO be cleared now ????? there is no fifo! the fifo is a lie!
					break;

				case TMS5110_CMD_READ_BRANCH:
					if (DEBUG_5110) logerror("READ AND BRANCH\n");
					new_int_write(0,1,1,0);
					new_int_write(1,1,1,0);
					new_int_write(0,1,1,0);
					new_int_write(0,0,0,0);
					new_int_write(1,0,0,0);
					new_int_write(0,0,0,0);
					m_schedule_dummy_read = FALSE;
					break;

				case TMS5110_CMD_TEST_TALK:
					if (DEBUG_5110) logerror("TEST TALK\n");
					m_state = CTL_STATE_NEXT_TTALK_OUTPUT;
					break;

				default:
					logerror("tms5110.c: unknown command: 0x%02x\n", m_CTL_pins);
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
	int bits, indx, i, rep_flag;
#if (DEBUG_5110)
	int ene;
#endif

	/* count the total number of bits available */
	bits = m_fifo_count;


	/* attempt to extract the energy index */
	bits -= m_coeff->energy_bits;
	if (bits < 0)
	{
		request_bits( -bits ); /* toggle M0 to receive needed bits */
		bits = 0;
	}
	indx = extract_bits(m_coeff->energy_bits);
	m_new_energy = m_coeff->energytable[indx];
#if (DEBUG_5110)
	ene = indx;
#endif

	/* if the energy index is 0 or 15, we're done */

	if ((indx == 0) || (indx == 15))
	{
		if (DEBUG_5110) logerror("  (4-bit energy=%d frame)\n",m_new_energy);

	/* clear the k's */
		if (indx == 0)
		{
			for (i = 0; i < m_coeff->num_k; i++)
				m_new_k[i] = 0;
		}

		/* clear fifo if stop frame encountered */
		if (indx == 15)
		{
			if (DEBUG_5110) logerror("  (4-bit energy=%d STOP frame)\n",m_new_energy);
			m_fifo_head = m_fifo_tail = m_fifo_count = 0;
		}
		return;
	}


	/* attempt to extract the repeat flag */
	bits -= 1;
	if (bits < 0)
	{
		request_bits( -bits ); /* toggle M0 to receive needed bits */
		bits = 0;
	}
	rep_flag = extract_bits(1);

	/* attempt to extract the pitch */
	bits -= m_coeff->pitch_bits;
	if (bits < 0)
	{
		request_bits( -bits ); /* toggle M0 to receive needed bits */
		bits = 0;
	}
	indx = extract_bits(m_coeff->pitch_bits);
	m_new_pitch = m_coeff->pitchtable[indx];

	/* if this is a repeat frame, just copy the k's */
	if (rep_flag)
	{
	//actually, we do nothing because the k's were already loaded (on parsing the previous frame)

		if (DEBUG_5110) logerror("  (10-bit energy=%d pitch=%d rep=%d frame)\n", m_new_energy, m_new_pitch, rep_flag);
		return;
	}


	/* if the pitch index was zero, we need 4 k's */
	if (indx == 0)
	{
		/* attempt to extract 4 K's */
		bits -= 18;
		if (bits < 0)
		{
		request_bits( -bits ); /* toggle M0 to receive needed bits */
		bits = 0;
		}
		for (i = 0; i < 4; i++)
			m_new_k[i] = m_coeff->ktable[i][extract_bits(m_coeff->kbits[i])];

	/* and clear the rest of the new_k[] */
		for (i = 4; i < m_coeff->num_k; i++)
			m_new_k[i] = 0;

		if (DEBUG_5110) logerror("  (28-bit energy=%d pitch=%d rep=%d 4K frame)\n", m_new_energy, m_new_pitch, rep_flag);
		return;
	}

	/* else we need 10 K's */
	bits -= 39;
	if (bits < 0)
	{
			request_bits( -bits ); /* toggle M0 to receive needed bits */
		bits = 0;
	}
#if (DEBUG_5110)
	printf("FrameDump %02d ", ene);
	for (i = 0; i < m_coeff->num_k; i++)
	{
		int x;
		x = extract_bits( m_coeff->kbits[i]);
		m_new_k[i] = m_coeff->ktable[i][x];
		printf("%02d ", x);
	}
	printf("\n");
#else
	for (i = 0; i < m_coeff->num_k; i++)
	{
		int x;
		x = extract_bits( m_coeff->kbits[i]);
		m_new_k[i] = m_coeff->ktable[i][x];
	}
#endif
	if (DEBUG_5110) logerror("  (49-bit energy=%d pitch=%d rep=%d 10K frame)\n", m_new_energy, m_new_pitch, rep_flag);

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

	set_variant(TMS5110_IS_5110A);

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
	set_variant(TMS5110_IS_5100);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tms5110a_device::device_start()
{
	tms5110_device::device_start();
	set_variant(TMS5110_IS_5110A);
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

void tmc0281_device::device_start()
{
	tms5110_device::device_start();
	set_variant(TMS5110_IS_TMC0281);
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
	/* initialize the FIFO */
	memset(m_fifo, 0, sizeof(m_fifo));
	m_fifo_head = m_fifo_tail = m_fifo_count = 0;

	/* initialize the chip state */
	m_speaking_now = m_talk_status = 0;
	m_CTL_pins = 0;
	m_RNG = 0x1fff;
	m_CTL_buffer = 0;
	m_PDC = 0;

	/* initialize the energy/pitch/k states */
	m_old_energy = m_new_energy = m_current_energy = m_target_energy = 0;
	m_old_pitch = m_new_pitch = m_current_pitch = m_target_pitch = 0;
	memset(m_old_k, 0, sizeof(m_old_k));
	memset(m_new_k, 0, sizeof(m_new_k));
	memset(m_current_k, 0, sizeof(m_current_k));
	memset(m_target_k, 0, sizeof(m_target_k));

	/* initialize the sample generators */
	m_interp_count = m_sample_count = m_pitch_count = 0;
	memset(m_x, 0, sizeof(m_x));
	m_next_is_address = FALSE;
	m_address = 0;
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
		if (DEBUG_5110) logerror("Status read while outputting Test Talk (status=%2d)\n", m_talk_status);
		return (m_talk_status << 0); /*CTL1 = still talking ? */
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
	return (m_talk_status << 0); /*CTL1 = still talking ? */
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

     tms5110_ready_r -- return the not ready status from the sound chip

******************************************************************************/

int tms5110_device::ready_r()
{
	/* bring up to date first */
	m_stream->update();
	return (m_fifo_count < FIFO_SIZE-1);
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


const device_type TMS5110A = &device_creator<tms5110a_device>;

tms5110a_device::tms5110a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms5110_device(mconfig, TMS5110A, "TMS5110A", tag, owner, clock, "tms5110a", __FILE__)
{
}


const device_type CD2801 = &device_creator<cd2801_device>;

cd2801_device::cd2801_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms5110_device(mconfig, CD2801, "CD2801", tag, owner, clock, "cd2801", __FILE__)
{
}


const device_type TMC0281 = &device_creator<tmc0281_device>;

tmc0281_device::tmc0281_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms5110_device(mconfig, TMC0281, "TMC0281", tag, owner, clock, "tmc0281", __FILE__)
{
}


const device_type CD2802 = &device_creator<cd2802_device>;

cd2802_device::cd2802_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms5110_device(mconfig, CD2802, "CD2802", tag, owner, clock, "cd2802", __FILE__)
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
