// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari System 2 hardware

    driver by Aaron Giles

    Games supported:
        * Paperboy (1984) [3 sets]
        * 720 Degrees (1986) [6 sets]
        * Super Sprint (1986) [7 sets]
        * Championship Sprint (1986) [8 sets]
        * APB - All Points Bulletin (1987) [9 sets]

    Known bugs:
        * none at this time

****************************************************************************

    Memory map

****************************************************************************

    ========================================================================
    MAIN CPU
    ========================================================================
    0000-0FFF   R/W   xxxxxxxx xxxxxxxx   Program RAM
    1000-107F   R/W   xxxxxxxx xxxxxxxx   Motion object palette RAM (64 entries)
                R/W   xxxx---- --------      (Intensity)
                R/W   ----xxxx --------      (Red)
                R/W   -------- xxxx----      (Green)
                R/W   -------- ----xxxx      (Blue)
    1080-10BF   R/W   xxxxxxxx xxxxxxxx   Alphanumerics palette RAM (32 entries)
    1100-11FF   R/W   xxxxxxxx xxxxxxxx   Playfield palette RAM (128 entries)
    1400        R     -------- xxxxxxxx   ADC data read
    1400          W   xxxxxx-- --------   Bank 1 ROM select
    1402          W   xxxxxx-- --------   Bank 2 ROM select
    1480-148F     W   -------- --------   ADC strobe/select
    1580          W   -------- --------   Sound command read IRQ reset
    15A0          W   -------- --------   Sound CPU reset
    15C0          W   -------- --------   32V IRQ reset
    15E0          W   -------- --------   VBLANK IRQ reset
    1600          W   -------- ----xxxx   IRQ enable
                  W   -------- ----x---      (VBLANK IRQ enable)
                  W   -------- -----x--      (32V IRQ enable)
                  W   -------- ------x-      (Sound response IRQ enable)
                  W   -------- -------x      (Sound command read IRQ enable)
    1680          W   -------- xxxxxxxx   Sound command
    1700          W   xxxxxxxx xx--xxxx   Playfield X scroll/bank 1 select
                  W   xxxxxxxx xx------      (Playfield X scroll)
                  W   -------- ----xxxx      (Playfield bank 1 select)
    1780          W   -xxxxxxx xx--xxxx   Playfield Y scroll/bank 2 select
                  W   -xxxxxxx xx------      (Playfield Y scroll)
                  W   -------- ----xxxx      (Playfield bank 2 select)
    1800        R     x------- xxxxxxxx   Switch inputs
                R     x------- --------      (Test switch)
                R     -------- xx--xxxx      (Game-specific switches)
                R     -------- --x-----      (Sound command buffer full)
                R     -------- ---x----      (Sound response buffer full)
    1800          W   -------- --------   Watchdog reset
    1C00        R     -------- xxxxxxxx   Sound response read
    2000-37FF   R/W   xxx---xx xxxxxxxx   Alphanumerics RAM (bank 0, 64x32 tiles)
                R/W   xxx----- --------      (Palette select)
                R/W   ------xx xxxxxxxx      (Tile index)
    3800-3FFF   R/W   xxxxxxxx xxxxxxxx   Motion object RAM (bank 0, 256 entries x 4 words)
                R/W   xxxxxxxx xx------      (0: Y position)
                R/W   -------- -----xxx      (0: Tile index, 3 MSB)
                R/W   x------- --------      (1: Hold position from last object)
                R/W   -x------ --------      (1: Horizontal flip)
                R/W   --xxx--- --------      (1: Number of Y tiles - 1)
                R/W   -----xxx xxxxxxxx      (1: Tile index, 11 LSB)
                R/W   xxxxxxxx xx------      (2: X position)
                R/W   xx------ --------      (3: Priority)
                R/W   -xxx---- --------      (3: Palette select)
                R/W   -------- xxxxxxxx      (3: Link to the next object)
    2000-3FFF   R/W   --xxxxxx xxxxxxxx   Playfield RAM (banks 2 & 3, 128x64 tiles)
                R/W   --xxx--- --------      (Palette select)
                R/W   -----x-- --------      (Tile bank select)
                R/W   ------xx xxxxxxxx      (Tile index, 10 LSB)
    4000-5FFF   R     xxxxxxxx xxxxxxxx   Bank 1 ROM
    6000-7FFF   R     xxxxxxxx xxxxxxxx   Bank 2 ROM
    8000-FFFF   R     xxxxxxxx xxxxxxxx   Program ROM (slapstic mapped here as well)
    ========================================================================
    Interrupts:
        IRQ0 = sound command read
        IRQ1 = sound command write
        IRQ2 = 32V
        IRQ3 = VBLANK
    ========================================================================


    ========================================================================
    SOUND CPU
    ========================================================================
    0000-0FFF   R/W   xxxxxxxx   Program RAM
    1000-11FF   R/W   xxxxxxxx   EEPROM
    1800-180F   R/W   xxxxxxxx   POKEY 1 (left) communications
    1810-1813   R     xxxxxxxx   LETA analog inputs
    1830-183F   R/W   xxxxxxxx   POKEY 2 (right) communications
    1850-1851   R/W   xxxxxxxx   YM2151 communications
    1860        R     xxxxxxxx   Sound command read
    1870          W   xxxxxxxx   TMS5220 data latch
    1872          W   --------   TMS5220 data strobe low
    1873          W   --------   TMS5220 data strobe high
    1874          W   xxxxxxxx   Sound response write
    1876          W   ------xx   Coin counters
    1878          W   --------   Interrupt acknowledge
    187A          W   xxxxxxxx   Mixer control
                  W   xxx-----      (TMS5220 volume)
                  W   ---xx---      (POKEY volume)
                  W   -----xxx      (YM2151 volume)
    187C          W   --xxxx--   Misc. control bits
                  W   --x-----      (TMS5220 frequency control)
                  W   ---x----      (LETA resolution control)
                  W   ----xx--      (LEDs)
    187E          W   -------x   Sound enable
    4000-FFFF   R     xxxxxxxx   Program ROM
    ========================================================================
    Interrupts:
        IRQ = YM2151 interrupt
        NMI = latch on sound command
    ========================================================================

****************************************************************************/


#include "emu.h"
#include "atarisy2.h"

#include "machine/adc0808.h"
#include "machine/eeprompar.h"
#include "speaker.h"


static constexpr XTAL MASTER_CLOCK = XTAL(20'000'000);



/*************************************
 *
 *  Interrupt updating
 *
 *************************************/

void atarisy2_state::update_interrupts()
{
	if (m_video_int_state)
		m_maincpu->set_input_line(t11_device::CP3_LINE, ASSERT_LINE);
	else
		m_maincpu->set_input_line(t11_device::CP3_LINE, CLEAR_LINE);

	if (m_scanline_int_state)
		m_maincpu->set_input_line(t11_device::CP2_LINE, ASSERT_LINE);
	else
		m_maincpu->set_input_line(t11_device::CP2_LINE, CLEAR_LINE);

	if (m_p2portwr_state)
		m_maincpu->set_input_line(t11_device::CP1_LINE, ASSERT_LINE);
	else
		m_maincpu->set_input_line(t11_device::CP1_LINE, CLEAR_LINE);

	if (m_p2portrd_state)
		m_maincpu->set_input_line(t11_device::CP0_LINE, ASSERT_LINE);
	else
		m_maincpu->set_input_line(t11_device::CP0_LINE, CLEAR_LINE);
}


void atarisy2_state::scanline_int_ack_w(uint8_t data)
{
	m_scanline_int_state = false;
	update_interrupts();
}


void atarisy2_state::video_int_ack_w(uint8_t data)
{
	m_video_int_state = false;
	update_interrupts();
}



/*************************************
 *
 *  Every 8-scanline update
 *
 *************************************/

TIMER_DEVICE_CALLBACK_MEMBER(atarisy2_state::scanline_update)
{
	int const scanline = param;
	if (scanline <= m_screen->height())
	{
		// generate the 32V interrupt (IRQ 2)
		if ((scanline % 64) == 0)
		{
			// clock the state through
			m_scanline_int_state = BIT(m_interrupt_enable, 2);
			update_interrupts();
		}
	}
}



/*************************************
 *
 *  Initialization
 *
 *************************************/

void atarisy2_state::machine_start()
{
	m_leds.resolve();

	m_scanline_int_state = false;
	m_video_int_state = false;
	m_p2portwr_state = false;
	m_p2portrd_state = false;

	save_item(NAME(m_interrupt_enable));
	save_item(NAME(m_scanline_int_state));
	save_item(NAME(m_video_int_state));
	save_item(NAME(m_p2portwr_state));
	save_item(NAME(m_p2portrd_state));
	save_item(NAME(m_sound_reset_state));

	for (int bank = 0; bank < 2; bank++)
		m_rombank[bank]->configure_entries(0, 64, memregion("maincpu")->base() + 0x10000, 0x2000);
}


void atarisy2_state::machine_reset()
{
	m_interrupt_enable = 0;

	sound_reset_w(1);
}



/*************************************
 *
 *  Interrupt handlers
 *
 *************************************/

void atarisy2_state::vblank_int(int state)
{
	if (state)
	{
		// clock the VBLANK through
		m_video_int_state = BIT(m_interrupt_enable, 3);
		update_interrupts();
	}
}


void atarisy2_state::int0_ack_w(uint8_t data)
{
	// reset sound IRQ
	m_p2portrd_state = false;
	update_interrupts();
}


void atarisy2_state::sound_reset_w(uint8_t data)
{
	// reset sound CPU
	m_audiocpu->set_input_line(INPUT_LINE_RESET, BIT(data, 0) ? ASSERT_LINE : CLEAR_LINE);

	sndrst_6502_w(0);
	coincount_w(0);
	switch_6502_w(0);
}


TIMER_CALLBACK_MEMBER(atarisy2_state::delayed_int_enable_w)
{
	m_interrupt_enable = param;
}


void atarisy2_state::int_enable_w(uint8_t data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(atarisy2_state::delayed_int_enable_w),this), data);
}


INTERRUPT_GEN_MEMBER(atarisy2_state::sound_irq_gen)
{
	m_audiocpu->set_input_line(m6502_device::IRQ_LINE, ASSERT_LINE);
}


void atarisy2_state::sound_irq_ack_w(uint8_t data)
{
	m_audiocpu->set_input_line(m6502_device::IRQ_LINE, CLEAR_LINE);
}



/*************************************
 *
 *  Bank selection.
 *
 *************************************/

void atarisy2_state::bankselect_w(offs_t offset, uint16_t data)
{
	/*static const int bankoffset[64] =
	{
	    12, 8, 4, 0,
	    13, 9, 5, 1,
	    14, 10, 6, 2,
	    15, 11, 7, 3,
	    28, 24, 20, 16,
	    29, 25, 21, 17,
	    30, 26, 22, 18,
	    31, 27, 23, 19,
	    44, 40, 36, 32,
	    45, 41, 37, 33,
	    46, 42, 38, 34,
	    47, 43, 39, 35,
	    60, 56, 52, 48,
	    61, 57, 53, 49,
	    62, 58, 54, 50,
	    63, 59, 55, 51
	};*/

	uint8_t banknumber = ((data >> 10) & 077) ^ 3;
	banknumber = bitswap<6>(banknumber, 5, 4, 1, 0, 3, 2);

	m_rombank[offset]->set_entry(banknumber);
}



/*************************************
 *
 *  Controls read
 *
 *************************************/

uint8_t atarisy2_state::leta_r(offs_t offset)
{
	if (offset <= 1 && m_pedal_count == -1) // 720
	{
		switch (m_io_select->read())
		{
			case 0: // Real
				break;

			case 1: // Fake Joystick
			/* special thanks to MAME Analog+ for the mapping code */
			{
				int const analogx = m_io_fake_joy_x->read() - 128;
				int const analogy = m_io_fake_joy_y->read() - 128;

				/* if the joystick is centered, leave the rest of this alone */
				double angle = m_joy_last_angle;
				if (analogx < -32 || analogx > 32 || analogy < -32 || analogy > 32)
					angle = atan2((double)analogx, (double)analogy) * 360 / (2 * M_PI);

				/* detect when we pass the 0 point in either direction */
				if (m_joy_last_angle < -90 && angle > 90)
					m_joy_rotations--;
				else if (m_joy_last_angle > 90 && angle < -90)
					m_joy_rotations++;
				m_joy_last_angle = angle;

				/* make offset 0 return 0xff when the controller blocks one of two gaps */
				/* this is not accurate, as a counter should count up/down 2 counts as it passes through each gap */
				/* this is close enough to pass the service mode controller test the first couple of tries. */
				if (offset == 0)
				{
					/* original controller had two gaps 10 degrees apart, each 2.5 degrees wide */
					/* we fake it a little to make it possible to hit the zeroed state with a digital controller */
					return (angle >= -5.0 && angle <= 5.0) ? 0xff : 0x00;
					/* proper angles */
					// return ((angle >= -12.5 && angle <= -7.5) || (angle >= 7.5 && angle <= 12.5)) ? 0xff : 0x00;
				}

				/* offset 1 returns dial value; 144 units = 1 full rotation */
				else
				{
					/* take the rotations * 144 plus the current angle */
					return (m_joy_rotations * 144 + (int)(angle * 144.0 / 360.0)) & 0xff;
				}
			}

			case 2: // Fake Spinner
			{
				int32_t  diff;
				uint32_t temp;
				uint32_t const rotate_count = m_io_fake_spinner->read() & 0xffff;
				/* rotate_count behaves the same as the real LEAT1 Rotate encoder
				 * we use it to generate the LETA0 Center encoder count
				 */

				if (rotate_count != m_spin_last_rotate_count)
				{
					/* see if count rolled between 0xffff and 0x0000 */
					if ((m_spin_last_rotate_count > 0xc000) && (rotate_count < 0x03ff))
					{
						temp = 0xffff - m_spin_last_rotate_count;
						diff = rotate_count + temp + 1;
					}
					else if ((rotate_count > 0xc000) && (m_spin_last_rotate_count < 0x03ff))
					{
						temp = 0xffff - rotate_count;
						diff = m_spin_last_rotate_count - temp - 1;
					}
					else
					{
						temp = rotate_count - m_spin_last_rotate_count;
						diff = temp;
					}

					m_spin_last_rotate_count = rotate_count;

					/* you may not like this, but it is the easiest way to accurately fake the center count */
					/* diff is never a big number anyways */
					if (diff < 0)
					{
						for (int i = 0; i > diff; i--)
						{
							m_spin_pos--;
							if (m_spin_pos < 0)
								m_spin_pos = 143;
							else
								switch (m_spin_pos)
								{
									case 2:
									case 3:
									case 141:
									case 142:
										m_spin_center_count--;
								}
						}
					}
					else
					{
						for (int i = 0; i < diff; i++)
						{
							m_spin_pos++;
							if (m_spin_pos > 143)
								m_spin_pos = 0;
							else
								switch (m_spin_pos)
								{
									case 2:
									case 3:
									case 141:
									case 142:
										m_spin_center_count++;
								}
						}
					}
				}

				if (offset == 0)
					return m_spin_center_count & 0xff;
				else
					// offset == 1
					return rotate_count & 0xff;
			}

			default:
				logerror("Unknown controller passed to leta_r");
				return 0xff;
		}
	}
	return m_io_leta[offset]->read();
}



/*************************************
 *
 *  Global sound control
 *
 *************************************/

/*
    Information from Derrick R on the mixing for this board:

    Lets start at the YM3012. The fist Op-Amp is a
    non-inverting gain stage.  The small cap can be
    ignored, it will filter out of the audio range.
     Gain = R106/R107 + 1 = 100/18 + 1 = 6.556
    Also the CH1 output is referenced to 2.5V.  Anything
    above is positive, below is negative.  Then amplified
    by 6.556 and clipped to -15V and (15V-1.5V=13.5V).
    This is useful to work out the relative signal levels
    of each effect.

    This then goes to the stage you were asking about.  We
    use milman to work out the voltage at the center of
    R74/R124 and the switched in resistors.

    -------------- Vout
    |    |   |
    R74  RP  R124
    |    |   |
    Vin  0   0

    Vin is the output of the first gain stage
    RP = is the switched resistors in parallel

    Vout = RT * IT

    IT is the total current
    IT = Vin/R74 + 0/RP + 0/R124 = Vin/R74

    RT = all resistors in parallel
    RT = 1/ (1/R74 + 1/R124 + 1/RP)
    Note if no RP resistors are switched in then
     RT= 1/ (1/R74 + 1/R124)

    So If we assume Vin=1V, we have this:
    0  .5
    1  .333
    2  .242
    3  .195
    4  .153
    5  .133
    6  .115
    7  .103

    Or half of Frank's values and in the proper order.
    This is important because later everything together
    will work out the relative volumes for the effects.

    This then goes to the final inverting op-amp which has
    a gain of -R123/R124.  Or in this case -1.  Again the
    cap can be ignored.  This gives us the voltage at YAM1
    (Vyam1)

    Everything is mixed at the bottom left of page 16.
    Using the summing formula:
    Vout = -R148 * (Vpaud1/R142 + Vtiaud/R143 +
    Vyam1/R144)
    Or shortened for just YAM1:
    Vout = -R148/R142 * Vyam1 = -2.128 * Vyam1

    OK so from the top, the YM3012 has a 0-5V out.  The
    first op-amp converts this to +/-2.5V.  Adds a gain of
    6.556. This gives a maximum of +/-16.389V which is
    clipped to +13.5V/-15V.
    Then the gain table is applied.  Then a gain of -1,
    which will not cause the amp stage to clip.  (I
    mention it beause the speech amp has a gain of -4.7
    and might clip).

    Then a final gain of -R148/R144 = -100/47 = -2.128.
    Which clips to +/-13.5V.  But...

    It actually would be better to use the summing formula
    mentioned earlier to mix the 3 effects together.
    Because the final clipping is dependant on the sum of
    all 3 effects.

    I went into complete detail, because the other effects
    use similar stages, but with different values.

    Hope it helps.  My personally opinion is clipping is
    important.  So it would be nice to add it in properly.
*/

void atarisy2_state::mixer_w(uint8_t data)
{
	// these gains are cheesed up, but give an approximate effect

	/*
	 * Before the volume adjustment, all channels pass through
	 * a high-pass filter which removes DC components. The
	 * filter frequency does also depend on the settings on
	 * the resistors.
	 *
	 * The op-amp after the pokey feeds mixes the op-amp output voltage
	 * with a low impedance back to the input. The internal resistance of the
	 * pokey now is the ground pole of a three pole resistor mixer: ground,
	 * 15V and op-amp output voltage.
	 *
	 * ==> DISCRETE candidate
	 *
	 */

	// bits 0-2 control the volume of the YM2151, using 22k, 47k, and 100k resistors
	double rtop = 1.0/(1.0/100 + 1.0/100);
	double rbott = 0;
	if (BIT(~data, 0)) rbott += 1.0/100;
	if (BIT(~data, 1)) rbott += 1.0/47;
	if (BIT(~data, 2)) rbott += 1.0/22;
	double gain = (rbott == 0) ? 1.0 : ((1.0/rbott) / (rtop + (1.0/rbott)));
	m_ym2151->set_output_gain(ALL_OUTPUTS, gain);

	// bits 3-4 control the volume of the POKEYs, using 47k and 100k resistors
	rtop = 1.0/(1.0/100 + 1.0/100);
	rbott = 0;
	if (BIT(~data, 3)) rbott += 1.0/47;
	if (BIT(~data, 4)) rbott += 1.0/22;
	gain = (rbott == 0) ? 1.0 : ((1.0/rbott) / (rtop + (1.0/rbott)));
	m_pokey[0]->set_output_gain(ALL_OUTPUTS, gain);
	m_pokey[1]->set_output_gain(ALL_OUTPUTS, gain);

	// bits 5-7 control the volume of the TMS5220, using 22k, 47k, and 100k resistors
	if (m_tms5220.found())
	{
		rtop = 1.0/(1.0/100 + 1.0/100);
		rbott = 0;
		if (BIT(~data, 5)) rbott += 1.0/100;
		if (BIT(~data, 6)) rbott += 1.0/47;
		if (BIT(~data, 7)) rbott += 1.0/22;
		gain = (rbott == 0) ? 1.0 : ((1.0/rbott) / (rtop + (1.0/rbott)));
		m_tms5220->set_output_gain(ALL_OUTPUTS, gain);
	}
}


void atarisy2_state::sndrst_6502_w(uint8_t data)
{
	// if no change, do nothing
	if ((BIT(data, 0)) == m_sound_reset_state)
		return;
	m_sound_reset_state = BIT(data, 0);
	m_ym2151->reset_w(m_sound_reset_state);

	// only track the 0 -> 1 transition
	if (m_sound_reset_state == 0)
		return;

	if (m_tms5220.found())
	{
		// Technically what happens is the tms5220 gets a long stream of 0xFF written to it
		// when sound_reset_state is 0, which halts the chip after a few frames, but this works
		// just as well, even if it isn't exactly true to hardware...
		// The hardware may not have worked either, the resistors to pull input to 0xFF are
		// fighting against the ls263 gate holding the latched value to be sent to the chip.
		m_tms5220->reset();
	}
	mixer_w(0);
}


uint16_t atarisy2_state::sound_r()
{
	if (!machine().side_effects_disabled())
	{
		// clear the p2portwr state on a p1portrd
		m_p2portwr_state = false;
		update_interrupts();
	}

	// handle it normally otherwise
	return m_mainlatch->read() | 0xff00;
}


void atarisy2_state::sound_6502_w(uint8_t data)
{
	// clock the state through
	m_p2portwr_state = BIT(m_interrupt_enable, 1);
	update_interrupts();

	// handle it normally otherwise
	m_mainlatch->write(data);
}


uint8_t atarisy2_state::sound_6502_r()
{
	if (!machine().side_effects_disabled())
	{
		// clock the state through
		m_p2portrd_state = BIT(m_interrupt_enable, 0);
		update_interrupts();
	}

	// handle it normally otherwise
	return m_soundlatch->read();
}



/*************************************
 *
 *  Speech chip
 *
 *************************************/

void atarisy2_state::tms5220_w(uint8_t data)
{
	if (m_tms5220.found())
	{
		m_tms5220->data_w(data);
	}
}

void atarisy2_state::tms5220_strobe_w(offs_t offset, uint8_t data)
{
	if (m_tms5220.found())
	{
		m_tms5220->wsq_w(1-(offset & 1));
	}
}


/*************************************
 *
 *  Misc. sound
 *
 *************************************/

void atarisy2_state::coincount_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 1));
}

void atarisy2_state::switch_6502_w(uint8_t data)
{
	m_leds[0] = BIT(data, 2);
	m_leds[1] = BIT(data, 3);
	if (m_tms5220.found())
	{
		int const divider = 16 - (12 | BIT(data, 5));
		m_tms5220->set_unscaled_clock(MASTER_CLOCK/4 / divider / 2);
	}
}


/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

// full memory map derived from schematics
void atarisy2_state::main_map(address_map &map)
{
	map.unmap_value_high();
	map(0000000, 0007777).ram();
	map(0010000, 0010777).mirror(01000).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0012000, 0012000).mirror(00176).r("adc", FUNC(adc0808_device::data_r));
	map(0012000, 0012003).mirror(00174).w(FUNC(atarisy2_state::bankselect_w));
	map(0012200, 0012217).mirror(00160).w("adc", FUNC(adc0808_device::address_offset_start_w)).umask16(0x00ff);
	map(0012600, 0012600).mirror(00036).w(FUNC(atarisy2_state::int0_ack_w));
	map(0012640, 0012640).mirror(00036).w(FUNC(atarisy2_state::sound_reset_w));
	map(0012700, 0012700).mirror(00036).w(FUNC(atarisy2_state::scanline_int_ack_w));
	map(0012740, 0012740).mirror(00036).w(FUNC(atarisy2_state::video_int_ack_w));
	map(0013000, 0013000).mirror(00176).w(FUNC(atarisy2_state::int_enable_w));
	map(0013200, 0013200).mirror(00176).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0013400, 0013401).mirror(00176).w(FUNC(atarisy2_state::xscroll_w)).share(m_xscroll);
	map(0013600, 0013601).mirror(00176).w(FUNC(atarisy2_state::yscroll_w)).share(m_yscroll);
	map(0014000, 0014001).mirror(01776).portr("IN0");
	map(0014000, 0014000).mirror(01776).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0016000, 0016001).mirror(01776).r(FUNC(atarisy2_state::sound_r));
	map(0020000, 0037777).view(m_vmmu);
	m_vmmu[0](020000, 033777).ram().w(m_alpha_tilemap, FUNC(tilemap_device::write16)).share("alpha");
	m_vmmu[0](034000, 037777).ram().w(FUNC(atarisy2_state::spriteram_w)).share("mob");
	m_vmmu[2](020000, 037777).ram().w(FUNC(atarisy2_state::playfieldt_w)).share(m_playfieldt);
	m_vmmu[3](020000, 037777).ram().w(FUNC(atarisy2_state::playfieldb_w)).share(m_playfieldb);
	map(0040000, 0057777).bankr(m_rombank[0]);
	map(0060000, 0077777).bankr(m_rombank[1]);
	map(0100000, 0177777).rom();
}


/*************************************
 *
 *  Sound CPU memory handlers
 *
 *************************************/

// full memory map derived from schematics
void atarisy2_state::sound_map(address_map &map)
{
	map(0x0000, 0x0fff).mirror(0x2000).ram();
	map(0x1000, 0x11ff).mirror(0x2600).rw("eeprom", FUNC(eeprom_parallel_28xx_device::read), FUNC(eeprom_parallel_28xx_device::write));
	map(0x1800, 0x180f).mirror(0x2780).rw(m_pokey[0], FUNC(pokey_device::read), FUNC(pokey_device::write));
	map(0x1810, 0x1813).mirror(0x278c).r(FUNC(atarisy2_state::leta_r));
	map(0x1830, 0x183f).mirror(0x2780).rw(m_pokey[1], FUNC(pokey_device::read), FUNC(pokey_device::write));
	map(0x1840, 0x1840).mirror(0x278f).portr("IN1");
	map(0x1850, 0x1851).mirror(0x278e).rw(m_ym2151, FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x1860, 0x1860).mirror(0x278f).r(FUNC(atarisy2_state::sound_6502_r));
	map(0x1870, 0x1870).mirror(0x2781).w(FUNC(atarisy2_state::tms5220_w));
	map(0x1872, 0x1873).mirror(0x2780).w(FUNC(atarisy2_state::tms5220_strobe_w));
	map(0x1874, 0x1874).mirror(0x2781).w(FUNC(atarisy2_state::sound_6502_w));
	map(0x1876, 0x1876).mirror(0x2781).w(FUNC(atarisy2_state::coincount_w));
	map(0x1878, 0x1878).mirror(0x2781).w(FUNC(atarisy2_state::sound_irq_ack_w));
	map(0x187a, 0x187a).mirror(0x2781).w(FUNC(atarisy2_state::mixer_w));
	map(0x187c, 0x187c).mirror(0x2781).w(FUNC(atarisy2_state::switch_6502_w));
	map(0x187e, 0x187e).mirror(0x2781).w(FUNC(atarisy2_state::sndrst_6502_w));
	map(0x4000, 0xffff).rom();
}


/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( paperboy )
	PORT_START("IN0") // T11
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("mainlatch", FUNC(generic_latch_8_device::pending_r)) // P2TALK
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("soundlatch", FUNC(generic_latch_8_device::pending_r)) // P1TALK
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x7f00, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE( 0x8000, IP_ACTIVE_LOW )

	PORT_START("IN1") // 6502
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("soundlatch", FUNC(generic_latch_8_device::pending_r)) // P1TALK
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("mainlatch", FUNC(generic_latch_8_device::pending_r)) // P2TALK
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("tms", FUNC(tms5220_device::readyq_r))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_CONDITION("IN0", 0x8000, EQUALS, 0x8000) // self-test
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("ADC0")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("ADC1")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("ADC2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ADC3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("LETA0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("LETA1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("LETA2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("LETA3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("6/7A:!8,!7")
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x0c, 0x00, "Right Coin" )            PORT_DIPLOCATION("6/7A:!6,!5")
	PORT_DIPSETTING(    0x00, "*1" )
	PORT_DIPSETTING(    0x04, "*4" )
	PORT_DIPSETTING(    0x08, "*5" )
	PORT_DIPSETTING(    0x0c, "*6" )
	PORT_DIPNAME( 0x10, 0x00, "Left Coin" )             PORT_DIPLOCATION("6/7A:!4")
	PORT_DIPSETTING(    0x00, "*1" )
	PORT_DIPSETTING(    0x10, "*2" )
	PORT_DIPNAME( 0xe0, 0x00, "Bonus Coins" )           PORT_DIPLOCATION("6/7A:!3,!2,!1")
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPSETTING(    0x80, "1 Each 5" )
	PORT_DIPSETTING(    0x40, "1 Each 4" )
	PORT_DIPSETTING(    0xa0, "1 Each 3" )
	PORT_DIPSETTING(    0x60, "2 Each 4" )
	PORT_DIPSETTING(    0x20, "1 Each 2" )
	PORT_DIPSETTING(    0xc0, "1 Each ?" )              // Not Documented
	PORT_DIPSETTING(    0xe0, DEF_STR( Free_Play ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("5/6A:!8,!7")
	PORT_DIPSETTING(    0x01, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Medium_Hard ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("5/6A:!6,!5")
	PORT_DIPSETTING(    0x08, "10000" )
	PORT_DIPSETTING(    0x00, "15000" )
	PORT_DIPSETTING(    0x0c, "20000" )
	PORT_DIPSETTING(    0x04, DEF_STR( None ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("5/6A:!4,!3")
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPSETTING(    0x30, "5" )
	PORT_DIPSETTING(    0x10, "Infinite (Cheat)")
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "5/6A:!2" )      // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "5/6A:!1" )      // Listed as "Unused"
INPUT_PORTS_END


static INPUT_PORTS_START( 720 )
	PORT_INCLUDE( paperboy )

	PORT_MODIFY("ADC0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("ADC1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	/* 720 uses a special controller to control the player rotation.
	 * It uses 1 disc with 72 teeth for the rotation and another disc
	 * with 2 teeth for the alignment of the joystick to the top position.
	 * The following graph shows how the Center and Rotate disc align.
	 * The numbers show how the optical count varies from center.
	 *
	 *   _____2  1________1  2_____
	 *        |__|        |__|          Center disc - 2 teeth.  Shown lined up with Rotate disc
	 *      __    __    __    __
	 *   __|  |__|  |__|  |__|  |__     Rotate disc - 72 teeth (144 positions)
	 *     4  3  2  1  1  2  3  4
	 */

	/* Center disc */
	/* X1, X2 LETA inputs */
	PORT_MODIFY("LETA0")    // not direct mapped
	PORT_BIT( 0xff, 0x00, IPT_DIAL_V ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_NAME("Center") PORT_CONDITION("SELECT",0x03,EQUALS,0x00)

	/* Rotate disc */
	/* Y1, Y2 LETA inputs */
	/* The disc has 72 teeth which are read by the hardware at 2x */
	/* Computer hardware reads at 4x, so we set the sensitivity to 50% */
	PORT_MODIFY("LETA1")    // not direct mapped
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_FULL_TURN_COUNT(144) PORT_NAME("Rotate") PORT_CONDITION("SELECT",0x03,EQUALS,0x00)

	PORT_START("FAKE_JOY_X")    // not direct mapped
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_CONDITION("SELECT",0x03,EQUALS,0x01)

	PORT_START("FAKE_JOY_Y")    // not direct mapped
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_REVERSE PORT_CONDITION("SELECT",0x03,EQUALS,0x01)

	/* Let's assume we are using a 1200 count spinner.  We scale to get a 144 count.
	 * 144/1200 = 0.12 = 12% */
	PORT_START("FAKE_SPINNER")  // not direct mapped
	PORT_BIT( 0xffff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(12) PORT_KEYDELTA(10) PORT_CONDITION("SELECT",0x03,EQUALS,0x02)

	PORT_START("SELECT")
	PORT_CONFNAME( 0x03, 0x02, "Controller Type" )
	PORT_CONFSETTING(    0x00, "Real" )
	PORT_CONFSETTING(    0x01, "Joystick" )
	PORT_CONFSETTING(    0x02, "Spinner" )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("5/6A:!8,!7")
	PORT_DIPSETTING(    0x01, "3000" )
	PORT_DIPSETTING(    0x00, "5000" )
	PORT_DIPSETTING(    0x02, "8000" )
	PORT_DIPSETTING(    0x03, "12000" )
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("5/6A:!6,!5")
	PORT_DIPSETTING(    0x04, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x30, 0x10, "Maximum Add. A. Coins" )     PORT_DIPLOCATION("5/6A:!4,!3")
	PORT_DIPSETTING(    0x10, "0" )
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPNAME( 0xc0, 0x40, "Coins Required" )            PORT_DIPLOCATION("5/6A:!2,!1")
	PORT_DIPSETTING(    0x80, "3 To Start, 2 To Continue" )
	PORT_DIPSETTING(    0xc0, "3 To Start, 1 To Continue" )
	PORT_DIPSETTING(    0x00, "2 To Start, 1 To Continue" )
	PORT_DIPSETTING(    0x40, "1 To Start, 1 To Continue" )
INPUT_PORTS_END


static INPUT_PORTS_START( ssprint )
	PORT_INCLUDE( paperboy )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3 )

	PORT_MODIFY("ADC0")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00,0x3f) PORT_SENSITIVITY(100) PORT_KEYDELTA(4) PORT_INVERT PORT_PLAYER(1)

	PORT_MODIFY("ADC1")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00,0x3f) PORT_SENSITIVITY(100) PORT_KEYDELTA(4) PORT_INVERT PORT_PLAYER(2)

	PORT_MODIFY("ADC2")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00,0x3f) PORT_SENSITIVITY(100) PORT_KEYDELTA(4) PORT_INVERT PORT_PLAYER(3)

	PORT_MODIFY("LETA0")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_MODIFY("LETA1")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_MODIFY("LETA2")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_PLAYER(3)

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x1c, 0x00, "Coin Multiplier" )       PORT_DIPLOCATION("6/7A:!6,!5,!4")
	PORT_DIPSETTING(    0x00, "*1" )
	PORT_DIPSETTING(    0x04, "*2" )
	PORT_DIPSETTING(    0x08, "*3" )
	PORT_DIPSETTING(    0x0c, "*4" )
	PORT_DIPSETTING(    0x10, "*5" )
	PORT_DIPSETTING(    0x14, "*6" )
	PORT_DIPSETTING(    0x18, "*7" )
	PORT_DIPSETTING(    0x1c, "*8" )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("5/6A:!8,!7")
	PORT_DIPSETTING(    0x01, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Medium_Hard ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x0c, 0x00, "Obstacles" )             PORT_DIPLOCATION("5/6A:!6,!5")
	PORT_DIPSETTING(    0x04, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Medium_Hard ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x30, 0x00, "Wrenches" )              PORT_DIPLOCATION("5/6A:!4,!3")
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x30, "5" )
INPUT_PORTS_END


static INPUT_PORTS_START( csprint )
	PORT_INCLUDE( ssprint )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_MODIFY("ADC2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("LETA2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x80, 0x00, "Auto High Score Reset" )     PORT_DIPLOCATION("5/6A:!1") // "After 2000 Plays."
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( apb )
	PORT_INCLUDE( paperboy )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_SERVICE1 )

	PORT_MODIFY("ADC0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("ADC1")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00,0x3f) PORT_SENSITIVITY(100) PORT_KEYDELTA(4) PORT_INVERT PORT_PLAYER(1)

	PORT_MODIFY("LETA0")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0xe0, 0x00, "Bonus Coins" )           PORT_DIPLOCATION("6/7A:!3,!2,!1")
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPSETTING(    0xc0, "1 Each 6" )              // Not documented
	PORT_DIPSETTING(    0xa0, "1 Each 5" )
	PORT_DIPSETTING(    0x80, "1 Each 4" )
	PORT_DIPSETTING(    0x60, "1 Each 3" )
	PORT_DIPSETTING(    0x40, "1 Each 2" )
	PORT_DIPSETTING(    0x20, "1 Each 1" )
	PORT_DIPSETTING(    0xe0, DEF_STR( Free_Play ) )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "Attract Lights" )        PORT_DIPLOCATION("5/6A:!8") // Listed As Unused.
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x00, "Max Continues" )         PORT_DIPLOCATION("5/6A:!7,!6")
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x04, "10" )
	PORT_DIPSETTING(    0x00, "25" )
	PORT_DIPSETTING(    0x06, "199" )
	PORT_DIPNAME( 0x38, 0x00, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("5/6A:!5,!4,!3")   /* No. Of Demerits Allowed  Bonus Inc. Every x Pts  Perfect Day Bonus   */
	PORT_DIPSETTING(    0x38, DEF_STR( Easiest ) )                                          /* 11                       5000                    Yes                 */
	PORT_DIPSETTING(    0x30, DEF_STR( Very_Easy ) )                                        /* 10                       6000                    Yes                 */
	PORT_DIPSETTING(    0x28, DEF_STR( Easy ) )                                             /* 9                        8000                    Yes                 */
	PORT_DIPSETTING(    0x00, DEF_STR( Medium_Easy ) )                                              /* 8                        10000                   Yes                 */
	PORT_DIPSETTING(    0x20, DEF_STR( Medium_Hard ) )                                              /* 7                        11000                   Yes                 */
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )                                             /* 6                        13000                   Yes                 */
	PORT_DIPSETTING(    0x08, DEF_STR( Very_Hard ) )                                        /* 5                        15000                   No                  */
	PORT_DIPSETTING(    0x18, DEF_STR( Hardest ) )                                          /* 4                        18000                   No                  */
	PORT_DIPNAME( 0xc0, 0x00, "Coins Required" )        PORT_DIPLOCATION("5/6A:!2,!1")
	PORT_DIPSETTING(    0x80, "3 To Start, 2 To Continue" )
	PORT_DIPSETTING(    0xc0, "3 To Start, 1 To Continue" )
	PORT_DIPSETTING(    0x00, "2 To Start, 1 To Continue" )
	PORT_DIPSETTING(    0x40, "1 To Start, 1 To Continue" )
INPUT_PORTS_END


/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout anlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ STEP4(0,1), STEP4(8,1) },
	{ STEP8(0,8*2) },
	8*8*2
};


static const gfx_layout pflayout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ 0, 4, RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4 },
	{ STEP4(0,1), STEP4(8,1) },
	{ STEP8(0,8*2) },
	8*8*2
};


static const gfx_layout molayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ 0, 4, RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4 },
	{ STEP4(8*0,1), STEP4(8*1,1), STEP4(8*2,1), STEP4(8*3,1) },
	{ STEP16(0,8*4) },
	16*16*2
};


static GFXDECODE_START( gfx_atarisy2 )
	GFXDECODE_ENTRY( "tiles",   0, pflayout, 128, 8 )
	GFXDECODE_ENTRY( "sprites", 0, molayout,   0, 4 )
	GFXDECODE_ENTRY( "chars",   0, anlayout,  64, 8 )
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void atarisy2_state::atarisy2(machine_config &config)
{
	constexpr XTAL SOUND_CLOCK  = XTAL(14'318'181);
	constexpr XTAL VIDEO_CLOCK  = XTAL(32'000'000);

	// basic machine hardware
	T11(config, m_maincpu, MASTER_CLOCK/2);
	m_maincpu->set_initial_mode(0x36ff); // initial mode word has DAL15,14,11,8 pulled low
	m_maincpu->set_addrmap(AS_PROGRAM, &atarisy2_state::main_map);

	M6502(config, m_audiocpu, SOUND_CLOCK/8);
	m_audiocpu->set_addrmap(AS_PROGRAM, &atarisy2_state::sound_map);
	m_audiocpu->set_periodic_int(FUNC(atarisy2_state::sound_irq_gen), attotime::from_hz(MASTER_CLOCK/2/16/16/16/10));

	adc0809_device &adc(ADC0809(config, "adc", MASTER_CLOCK/32)); // 625 kHz
	adc.in_callback<0>().set_ioport("ADC0"); // J102 pin 5 (POT1)
	adc.in_callback<1>().set_ioport("ADC1"); // J102 pin 7 (POT2)
	adc.in_callback<2>().set_ioport("ADC2"); // J102 pin 9 (POT3)
	adc.in_callback<3>().set_ioport("ADC3"); // J102 pin 8 (POT4)
	// IN4 = J102 pin 6 (unused)
	// IN5 = J102 pin 4 (unused)
	// IN6 = J102 pin 2 (unused)
	// IN7 = J102 pin 3 (unused)

	EEPROM_2804(config, "eeprom");

	TIMER(config, "scantimer").configure_scanline(FUNC(atarisy2_state::scanline_update), m_screen, 0, 64);

	WATCHDOG_TIMER(config, "watchdog").set_time(attotime::from_hz(MASTER_CLOCK/2/16/16/16/256));

	// video hardware
	GFXDECODE(config, "gfxdecode", "palette", gfx_atarisy2);
	PALETTE(config, "palette").set_format(2, &atarisy2_state::RRRRGGGGBBBBIIII, 256);

	TILEMAP(config, m_playfield_tilemap, "gfxdecode", 2, 8,8, TILEMAP_SCAN_ROWS, 128,64).set_info_callback(FUNC(atarisy2_state::get_playfield_tile_info));
	TILEMAP(config, m_alpha_tilemap, "gfxdecode", 2, 8,8, TILEMAP_SCAN_ROWS, 64,48, 0).set_info_callback(FUNC(atarisy2_state::get_alpha_tile_info));

	ATARI_MOTION_OBJECTS(config, m_mob, 0, m_screen, atarisy2_state::s_mob_config);
	m_mob->set_gfxdecode(m_gfxdecode);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	screen.set_raw(VIDEO_CLOCK/2, 640, 0, 512, 416, 0, 384);
	screen.set_screen_update(FUNC(atarisy2_state::screen_update));
	screen.set_palette("palette");
	screen.screen_vblank().set(FUNC(atarisy2_state::vblank_int));

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, m6502_device::NMI_LINE);
	// apb3 fails the self-test with a 100 µs delay or less
	m_soundlatch->data_pending_callback().append([this](int state) { if (state) machine().scheduler().perfect_quantum(attotime::from_usec(200)); });

	GENERIC_LATCH_8(config, m_mainlatch);

	YM2151(config, m_ym2151, SOUND_CLOCK/4);
	m_ym2151->add_route(0, "speaker", 0.60, 0);
	m_ym2151->add_route(1, "speaker", 0.60, 1);

	POKEY(config, m_pokey[0], SOUND_CLOCK/8);
	m_pokey[0]->allpot_r().set_ioport("DSW0");
	m_pokey[0]->add_route(ALL_OUTPUTS, "speaker", 1.35, 0);

	POKEY(config, m_pokey[1], SOUND_CLOCK/8);
	m_pokey[1]->allpot_r().set_ioport("DSW1");
	m_pokey[1]->add_route(ALL_OUTPUTS, "speaker", 1.35, 1);

	TMS5220C(config, m_tms5220, MASTER_CLOCK/4/4/2);
	m_tms5220->add_route(ALL_OUTPUTS, "speaker", 0.75, 0);
	m_tms5220->add_route(ALL_OUTPUTS, "speaker", 0.75, 1);
}


void atarisy2_state::paperboy(machine_config &config)
{
	atarisy2(config);
	SLAPSTIC(config, m_slapstic, 105);
	m_slapstic->set_range(m_maincpu, AS_PROGRAM, 0100000, 0100777, 0);
	m_slapstic->set_view(m_vmmu);
}


void atarisy2_state::_720(machine_config &config)
{
	atarisy2(config);
	/* without the default EEPROM, 720 hangs at startup due to communication
	   issues with the sound CPU; temporarily increasing the sound CPU frequency
	   to ~2.2MHz "fixes" the problem */

	SLAPSTIC(config, m_slapstic, 107);
	m_slapstic->set_range(m_maincpu, AS_PROGRAM, 0100000, 0100777, 0);
	m_slapstic->set_view(m_vmmu);
}


void atarisy2_state::ssprint(machine_config &config)
{
	atarisy2(config);
	SLAPSTIC(config, m_slapstic, 108);
	m_slapstic->set_range(m_maincpu, AS_PROGRAM, 0100000, 0100777, 0);
	m_slapstic->set_view(m_vmmu);

	// sound hardware
	config.device_remove("tms");
}


void atarisy2_state::csprint(machine_config &config)
{
	atarisy2(config);
	SLAPSTIC(config, m_slapstic, 109);
	m_slapstic->set_range(m_maincpu, AS_PROGRAM, 0100000, 0100777, 0);
	m_slapstic->set_view(m_vmmu);

	// sound hardware
	config.device_remove("tms");
}


void atarisy2_state::apb(machine_config &config)
{
	atarisy2(config);
	SLAPSTIC(config, m_slapstic, 110);
	m_slapstic->set_range(m_maincpu, AS_PROGRAM, 0100000, 0100777, 0);
	m_slapstic->set_view(m_vmmu);
}


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( paperboy ) // ALL of these roms should be 136034-xxx but the correct labels aren't known per game rev!
	ROM_REGION( 0x90000, "maincpu", 0 ) // 9*64k for T11 code
	ROM_LOAD16_BYTE( "cpu_l07.rv3", 0x008000, 0x004000, CRC(4024bb9b) SHA1(9030ce5a6a1a3d769c699a92b32a55013f9766aa) )
	ROM_LOAD16_BYTE( "cpu_n07.rv3", 0x008001, 0x004000, CRC(0260901a) SHA1(39d786f5c440ca1fd529ee73e2a4d2406cd1db8f) )
	ROM_LOAD16_BYTE( "cpu_f06.rv2", 0x010000, 0x004000, CRC(3fea86ac) SHA1(90722bfd0426efbfb69714151f8644b56075b4c1) )
	ROM_LOAD16_BYTE( "cpu_n06.rv2", 0x010001, 0x004000, CRC(711b17ba) SHA1(7c9b19f754f1e3ba4d081edce2a39e81ce87f6bb) )
	ROM_LOAD16_BYTE( "cpu_j06.rv1", 0x030000, 0x004000, CRC(a754b12d) SHA1(7b07efe70f9696041355b72f5cded7fcbd8be460) )
	ROM_LOAD16_BYTE( "cpu_p06.rv1", 0x030001, 0x004000, CRC(89a1ff9c) SHA1(aa947e0726bb68164b9556d57daf6547b4580ed0) )
	ROM_LOAD16_BYTE( "cpu_k06.rv1", 0x050000, 0x004000, CRC(290bb034) SHA1(71dfceb6a8b3b0e3be2cc907c3d4b91fe6973fec) )
	ROM_LOAD16_BYTE( "cpu_r06.rv1", 0x050001, 0x004000, CRC(826993de) SHA1(59c6b87bcbca80b0a6192d7bb534a0747f32b907) )
	ROM_LOAD16_BYTE( "cpu_l06.rv2", 0x070000, 0x004000, CRC(8a754466) SHA1(2c4c6ca797c7f4349c2893d8c0ba7e2658fdca99) )
	ROM_LOAD16_BYTE( "cpu_s06.rv2", 0x070001, 0x004000, CRC(224209f9) SHA1(c41269bfadb8fff1c8ff0f6ea0b8e8b34feb49d6) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    // 64k for 6502 code
	ROM_LOAD( "cpu_a02.rv3", 0x004000, 0x004000, CRC(ba251bc4) SHA1(768e42608263205e412e651082ffa2a083b04644) )
	ROM_LOAD( "cpu_b02.rv2", 0x008000, 0x004000, CRC(e4e7a8b9) SHA1(f11a0cf40d5c51ff180f0fa1cf676f95090a1010) )
	ROM_LOAD( "cpu_c02.rv2", 0x00c000, 0x004000, CRC(d44c2aa2) SHA1(f1b00e36d87f6d77746cf003198c7f19aa2f4fab) )

	ROM_REGION( 0x20000, "tiles", 0 )
	ROM_LOAD( "vid_a06.rv1", 0x000000, 0x008000, CRC(b32ffddf) SHA1(5b7619008e34ed7f5eb5e85e5f45c375e078086a) )
	ROM_LOAD( "vid_b06.rv1", 0x00c000, 0x004000, CRC(301b849d) SHA1(d608a854027da5eb88c071df1d01f31124db89a8) )
	ROM_LOAD( "vid_c06.rv1", 0x010000, 0x008000, CRC(7bb59d68) SHA1(fcaa8bd32448d8f951ae446eb425b608f2cecbef) )
	ROM_LOAD( "vid_d06.rv1", 0x01c000, 0x004000, CRC(1a1d4ba8) SHA1(603d61fd17e312d0784d883a50ce6b03aba27d10) )

	ROM_REGION( 0x40000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "vid_l06.rv1", 0x000000, 0x008000, CRC(067ef202) SHA1(519f32995a32ed96086f4ed3d49530b6917ad7d3) )
	ROM_LOAD( "vid_k06.rv1", 0x008000, 0x008000, CRC(76b977c4) SHA1(09988aceaf398279556980e3a21c0dc1b619fb72) )
	ROM_LOAD( "vid_j06.rv1", 0x010000, 0x008000, CRC(2a3cc8d0) SHA1(c0165286486a0844baf99c782d2fffdd6ad003b6) )
	ROM_LOAD( "vid_h06.rv1", 0x018000, 0x008000, CRC(6763a321) SHA1(15ed912f0346f6b5c3ad23ff22e7493d31ad18a7) )
	ROM_LOAD( "vid_s06.rv1", 0x020000, 0x008000, CRC(0a321b7b) SHA1(681317494a0bd50569bb822783336e68551cfd5e) )
	ROM_LOAD( "vid_p06.rv1", 0x028000, 0x008000, CRC(5bd089ee) SHA1(9ac98391a6c70d3cfbe609342294668530d690b4) )
	ROM_LOAD( "vid_n06.rv1", 0x030000, 0x008000, CRC(c34a517d) SHA1(f0af3db87f73c1fad00a270269ba380898ef5a4b) )
	ROM_LOAD( "vid_m06.rv1", 0x038000, 0x008000, CRC(df723956) SHA1(613d398f30463086c0cc720a760bda652e0f3832) )

	ROM_REGION( 0x2000, "chars", 0 )
	ROM_LOAD( "vid_t06.rv1", 0x000000, 0x002000, CRC(60d7aebb) SHA1(ad74221c4270496ebcfedd46ea16dca2cda1b4be) )

	ROM_REGION( 0x200, "eeprom", 0 )
	ROM_LOAD( "paperboy-eeprom.bin", 0x0000, 0x0200, CRC(756b90cc) SHA1(b78762e354f1316087f9de4005734c343356c8ef) )
ROM_END


ROM_START( paperboyr2 )
	ROM_REGION( 0x90000, "maincpu", 0 ) // 9*64k for T11 code
	ROM_LOAD16_BYTE( "cpu_l07.rv2", 0x008000, 0x004000, CRC(39d0a625) SHA1(c4f62cecbc8a122f58f98312517feccf9429f28b) )
	ROM_LOAD16_BYTE( "cpu_n07.rv2", 0x008001, 0x004000, CRC(3c5de588) SHA1(faad02fee1528cd52af1fac315096a46a9eb9a85) )
	ROM_LOAD16_BYTE( "cpu_f06.rv2", 0x010000, 0x004000, CRC(3fea86ac) SHA1(90722bfd0426efbfb69714151f8644b56075b4c1) )
	ROM_LOAD16_BYTE( "cpu_n06.rv2", 0x010001, 0x004000, CRC(711b17ba) SHA1(7c9b19f754f1e3ba4d081edce2a39e81ce87f6bb) )
	ROM_LOAD16_BYTE( "cpu_j06.rv1", 0x030000, 0x004000, CRC(a754b12d) SHA1(7b07efe70f9696041355b72f5cded7fcbd8be460) )
	ROM_LOAD16_BYTE( "cpu_p06.rv1", 0x030001, 0x004000, CRC(89a1ff9c) SHA1(aa947e0726bb68164b9556d57daf6547b4580ed0) )
	ROM_LOAD16_BYTE( "cpu_k06.rv1", 0x050000, 0x004000, CRC(290bb034) SHA1(71dfceb6a8b3b0e3be2cc907c3d4b91fe6973fec) )
	ROM_LOAD16_BYTE( "cpu_r06.rv1", 0x050001, 0x004000, CRC(826993de) SHA1(59c6b87bcbca80b0a6192d7bb534a0747f32b907) )
	ROM_LOAD16_BYTE( "cpu_l06.rv2", 0x070000, 0x004000, CRC(8a754466) SHA1(2c4c6ca797c7f4349c2893d8c0ba7e2658fdca99) )
	ROM_LOAD16_BYTE( "cpu_s06.rv2", 0x070001, 0x004000, CRC(224209f9) SHA1(c41269bfadb8fff1c8ff0f6ea0b8e8b34feb49d6) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    // 64k for 6502 code
	ROM_LOAD( "cpu_a02.rv2", 0x004000, 0x004000, CRC(4a759092) SHA1(26909df77f53ac19e205411b90558488badc82bd) )
	ROM_LOAD( "cpu_b02.rv2", 0x008000, 0x004000, CRC(e4e7a8b9) SHA1(f11a0cf40d5c51ff180f0fa1cf676f95090a1010) )
	ROM_LOAD( "cpu_c02.rv2", 0x00c000, 0x004000, CRC(d44c2aa2) SHA1(f1b00e36d87f6d77746cf003198c7f19aa2f4fab) )

	ROM_REGION( 0x20000, "tiles", 0 )
	ROM_LOAD( "vid_a06.rv1", 0x000000, 0x008000, CRC(b32ffddf) SHA1(5b7619008e34ed7f5eb5e85e5f45c375e078086a) )
	ROM_LOAD( "vid_b06.rv1", 0x00c000, 0x004000, CRC(301b849d) SHA1(d608a854027da5eb88c071df1d01f31124db89a8) )
	ROM_LOAD( "vid_c06.rv1", 0x010000, 0x008000, CRC(7bb59d68) SHA1(fcaa8bd32448d8f951ae446eb425b608f2cecbef) )
	ROM_LOAD( "vid_d06.rv1", 0x01c000, 0x004000, CRC(1a1d4ba8) SHA1(603d61fd17e312d0784d883a50ce6b03aba27d10) )

	ROM_REGION( 0x40000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "vid_l06.rv1", 0x000000, 0x008000, CRC(067ef202) SHA1(519f32995a32ed96086f4ed3d49530b6917ad7d3) )
	ROM_LOAD( "vid_k06.rv1", 0x008000, 0x008000, CRC(76b977c4) SHA1(09988aceaf398279556980e3a21c0dc1b619fb72) )
	ROM_LOAD( "vid_j06.rv1", 0x010000, 0x008000, CRC(2a3cc8d0) SHA1(c0165286486a0844baf99c782d2fffdd6ad003b6) )
	ROM_LOAD( "vid_h06.rv1", 0x018000, 0x008000, CRC(6763a321) SHA1(15ed912f0346f6b5c3ad23ff22e7493d31ad18a7) )
	ROM_LOAD( "vid_s06.rv1", 0x020000, 0x008000, CRC(0a321b7b) SHA1(681317494a0bd50569bb822783336e68551cfd5e) )
	ROM_LOAD( "vid_p06.rv1", 0x028000, 0x008000, CRC(5bd089ee) SHA1(9ac98391a6c70d3cfbe609342294668530d690b4) )
	ROM_LOAD( "vid_n06.rv1", 0x030000, 0x008000, CRC(c34a517d) SHA1(f0af3db87f73c1fad00a270269ba380898ef5a4b) )
	ROM_LOAD( "vid_m06.rv1", 0x038000, 0x008000, CRC(df723956) SHA1(613d398f30463086c0cc720a760bda652e0f3832) )

	ROM_REGION( 0x2000, "chars", 0 )
	ROM_LOAD( "vid_t06.rv1", 0x000000, 0x002000, CRC(60d7aebb) SHA1(ad74221c4270496ebcfedd46ea16dca2cda1b4be) )

	ROM_REGION( 0x200, "eeprom", 0 )
	ROM_LOAD( "paperboy-eeprom.bin", 0x0000, 0x0200, CRC(756b90cc) SHA1(b78762e354f1316087f9de4005734c343356c8ef) )
ROM_END


ROM_START( paperboyr1 )
	ROM_REGION( 0x90000, "maincpu", 0 ) // 9*64k for T11 code
	ROM_LOAD16_BYTE( "cpu_l07.rv1", 0x008000, 0x004000, CRC(fd87a8ee) SHA1(f42fe59f62928bb36c00b6814e1af173d713fb2e) )
	ROM_LOAD16_BYTE( "cpu_n07.rv1", 0x008001, 0x004000, CRC(a997e217) SHA1(85d97e62bb225f6302cdad18bf1299d364614ce4) )
	ROM_LOAD16_BYTE( "cpu_f06.rv1", 0x010000, 0x004000, CRC(e871248d) SHA1(c660e21e47a958ee72857ca41e6a299ce4328076) )
	ROM_LOAD16_BYTE( "cpu_n06.rv1", 0x010001, 0x004000, CRC(4d110e5f) SHA1(fc6dfbce48b297f9645c74f66e9b01c3373c6b59) )
	ROM_LOAD16_BYTE( "cpu_j06.rv1", 0x030000, 0x004000, CRC(a754b12d) SHA1(7b07efe70f9696041355b72f5cded7fcbd8be460) )
	ROM_LOAD16_BYTE( "cpu_p06.rv1", 0x030001, 0x004000, CRC(89a1ff9c) SHA1(aa947e0726bb68164b9556d57daf6547b4580ed0) )
	ROM_LOAD16_BYTE( "cpu_k06.rv1", 0x050000, 0x004000, CRC(290bb034) SHA1(71dfceb6a8b3b0e3be2cc907c3d4b91fe6973fec) )
	ROM_LOAD16_BYTE( "cpu_r06.rv1", 0x050001, 0x004000, CRC(826993de) SHA1(59c6b87bcbca80b0a6192d7bb534a0747f32b907) )
	ROM_LOAD16_BYTE( "cpu_l06.rv1", 0x070000, 0x004000, CRC(ccbc58a6) SHA1(dd66317146c295524f83b8d40c20164e873752b5) )
	ROM_LOAD16_BYTE( "cpu_s06.rv1", 0x070001, 0x004000, CRC(a7f14643) SHA1(d73c8ec2493617fce2e6822e8a6cde16a2de5965) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    // 64k for 6502 code
	ROM_LOAD( "cpu_a02.rv1", 0x004000, 0x004000, CRC(5479a788) SHA1(4cc5145e75ac6370f54eea33531f1f96160ee82b) )
	ROM_LOAD( "cpu_b02.rv1", 0x008000, 0x004000, CRC(de4147c6) SHA1(c997510b2018291924abddfe604a8f738fd8035c) )
	ROM_LOAD( "cpu_c02.rv1", 0x00c000, 0x004000, CRC(b71505fc) SHA1(15fd156038861cb715fce10f1c56f3ded851be39) )

	ROM_REGION( 0x20000, "tiles", 0 )
	ROM_LOAD( "vid_a06.rv1", 0x000000, 0x008000, CRC(b32ffddf) SHA1(5b7619008e34ed7f5eb5e85e5f45c375e078086a) )
	ROM_LOAD( "vid_b06.rv1", 0x00c000, 0x004000, CRC(301b849d) SHA1(d608a854027da5eb88c071df1d01f31124db89a8) )
	ROM_LOAD( "vid_c06.rv1", 0x010000, 0x008000, CRC(7bb59d68) SHA1(fcaa8bd32448d8f951ae446eb425b608f2cecbef) )
	ROM_LOAD( "vid_d06.rv1", 0x01c000, 0x004000, CRC(1a1d4ba8) SHA1(603d61fd17e312d0784d883a50ce6b03aba27d10) )

	ROM_REGION( 0x40000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "vid_l06.rv1", 0x000000, 0x008000, CRC(067ef202) SHA1(519f32995a32ed96086f4ed3d49530b6917ad7d3) )
	ROM_LOAD( "vid_k06.rv1", 0x008000, 0x008000, CRC(76b977c4) SHA1(09988aceaf398279556980e3a21c0dc1b619fb72) )
	ROM_LOAD( "vid_j06.rv1", 0x010000, 0x008000, CRC(2a3cc8d0) SHA1(c0165286486a0844baf99c782d2fffdd6ad003b6) )
	ROM_LOAD( "vid_h06.rv1", 0x018000, 0x008000, CRC(6763a321) SHA1(15ed912f0346f6b5c3ad23ff22e7493d31ad18a7) )
	ROM_LOAD( "vid_s06.rv1", 0x020000, 0x008000, CRC(0a321b7b) SHA1(681317494a0bd50569bb822783336e68551cfd5e) )
	ROM_LOAD( "vid_p06.rv1", 0x028000, 0x008000, CRC(5bd089ee) SHA1(9ac98391a6c70d3cfbe609342294668530d690b4) )
	ROM_LOAD( "vid_n06.rv1", 0x030000, 0x008000, CRC(c34a517d) SHA1(f0af3db87f73c1fad00a270269ba380898ef5a4b) )
	ROM_LOAD( "vid_m06.rv1", 0x038000, 0x008000, CRC(df723956) SHA1(613d398f30463086c0cc720a760bda652e0f3832) )

	ROM_REGION( 0x2000, "chars", 0 )
	ROM_LOAD( "vid_t06.rv1", 0x000000, 0x002000, CRC(60d7aebb) SHA1(ad74221c4270496ebcfedd46ea16dca2cda1b4be) )

	ROM_REGION( 0x200, "eeprom", 0 )
	ROM_LOAD( "paperboy-eeprom.bin", 0x0000, 0x0200, CRC(756b90cc) SHA1(b78762e354f1316087f9de4005734c343356c8ef) )
ROM_END


/*

Paperboy prototype pcb

brown in colour, A042571-01 rev 2
handwritten in red '#10'
sticky label with '10' on it.
Had a tag attatched saying 'suspect reset problem'

SOUND section:
There is a quad pokey onboard, with a TMS5220 and a YM2151 and YM3012.
There is a 6502 next to the quad pokey. 3 roms are nearby, with the naming convention paptst.

CPU section:
one empty 40pin socket, looks like the main cpu.

There are no designations for IC location, only a
handwritten column of letters to denote the left/right.
I am guessing at the up/down by making up my own numbers,
as atari used to start the numbers from furthest away from the edge connector first.

All IC's and components on this pcb are hand soldered-in
*********************************************************************************************
NOT ALL ROMS READ CONSISTENTLY IN MY READER, AFTER READING 2-3 TIMES I GOT A STABLE CHECKSUM.
So bear this in mind, there may be a few bytes out here and there.
*********************************************************************************************


Label           Loc.    Type    File
FIX-LOW CA50    5P      27128   fix-low.bin
FIX-HI 0AB3     5M      27128   fix-hi.bin
L-0 208E        7T      27128   l-0.bin
H-0 9C75        6T      27128   h-0.bin
PAGROM L-1      6R/S    27128   pagroml-1.bin
H-1             5T      27128   h-1.bin
PAGROM L-2      7R/S    27128   pagroml-2.bin
PAGROM H-2      4T      27128   pagromh-2.bin
LOW-3           5R/S    27128   low-3.bin
HI-3            4R/S    27128   hi-3.bin

PAPTST 4000     2A      27128   paptst4000.bin
PAPTST 8000     2C      27128   paptst8000.bin
PAPTST c000     2G      27128   paptstc000.bin

*/

ROM_START( paperboyp )
	// program roms are all marked as BAD_DUMP due to (non-specific) comments about some roms not reading consistently (the first 4 had checksums printed on so could be verified)
	ROM_REGION( 0x90000, "maincpu", 0 ) // 9*64k for T11 code
	ROM_LOAD16_BYTE( "fix-low.5p",   0x008000, 0x004000, BAD_DUMP CRC(55a7137b) SHA1(19bc2b7a64ab277b0e8e3d131d427db96da8e91e) )
	ROM_LOAD16_BYTE( "fix-hi.5m",    0x008001, 0x004000, BAD_DUMP CRC(e386b4f9) SHA1(0c156c4fca38d6aaae1e5b4c52c6bfd0231f9954) )
	ROM_LOAD16_BYTE( "l-0.7t",       0x010000, 0x004000, BAD_DUMP CRC(fbf26418) SHA1(f34e7ac253723f175dedf8126a3ee25e37ebdc5b) )
	ROM_LOAD16_BYTE( "h-0.6t",       0x010001, 0x004000, BAD_DUMP CRC(ee4334ea) SHA1(0ada76c4251b75fb8bb35fafbfd15490f2c80f2c) )
	ROM_LOAD16_BYTE( "pagroml-1.6rs",0x030000, 0x004000, BAD_DUMP CRC(1414b432) SHA1(1d5f990499925a5474a11730989f86c4807fd4a7) )
	ROM_LOAD16_BYTE( "h-1.5t",       0x030001, 0x004000, BAD_DUMP CRC(ee902968) SHA1(e292da19cc56fdb18709dc6040b80a6860f0fb45) )
	ROM_LOAD16_BYTE( "pagroml-2.7rs",0x050000, 0x004000, BAD_DUMP CRC(be537e48) SHA1(906eeaffeb4a97fa3afe864dbbae7a544c1ddb7c) )
	ROM_LOAD16_BYTE( "pagromh-2.4t", 0x050001, 0x004000, BAD_DUMP CRC(949defeb) SHA1(07305311e8446636171f29d3b20142c03f982cb9) )
	ROM_LOAD16_BYTE( "low-3.5rs",    0x070000, 0x004000, BAD_DUMP CRC(a0afde83) SHA1(f4188af58c08b180fe2799a42f3cd92d425370df) )
	ROM_LOAD16_BYTE( "hi-3.4rs",     0x070001, 0x004000, BAD_DUMP CRC(7a1a4d69) SHA1(492e95d4d418d91c0693c04664d3ee9c220e21f5) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    // 64k for 6502 code
	ROM_LOAD( "paptst4000.2a", 0x004000, 0x004000, BAD_DUMP CRC(e5ee1bca) SHA1(f467aae75ea3fe7178f64fe0cbcc20503ac0fa21) )
	ROM_LOAD( "paptst8000.2c", 0x008000, 0x004000, BAD_DUMP CRC(c51ebdb0) SHA1(abba5c8a408c709ad447af02304bc04f887cf914) )
	ROM_LOAD( "paptstc000.2g", 0x00c000, 0x004000, BAD_DUMP CRC(e663d9c2) SHA1(4d0c0e3b6710d2db03b35d0eded413d1cb43c9a7) )

	/* there was no video board, so all GFX roms are marked as BAD_DUMP as they might not be correct for this revision
	   it is also possible that the video hardware is closer to system1 rather than system2 */
	ROM_REGION( 0x20000, "tiles", 0 )
	ROM_LOAD( "vid_a06.rv1", 0x000000, 0x008000, BAD_DUMP CRC(b32ffddf) SHA1(5b7619008e34ed7f5eb5e85e5f45c375e078086a) )
	ROM_LOAD( "vid_b06.rv1", 0x00c000, 0x004000, BAD_DUMP CRC(301b849d) SHA1(d608a854027da5eb88c071df1d01f31124db89a8) )
	ROM_LOAD( "vid_c06.rv1", 0x010000, 0x008000, BAD_DUMP CRC(7bb59d68) SHA1(fcaa8bd32448d8f951ae446eb425b608f2cecbef) )
	ROM_LOAD( "vid_d06.rv1", 0x01c000, 0x004000, BAD_DUMP CRC(1a1d4ba8) SHA1(603d61fd17e312d0784d883a50ce6b03aba27d10) )

	ROM_REGION( 0x40000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "vid_l06.rv1", 0x000000, 0x008000, BAD_DUMP CRC(067ef202) SHA1(519f32995a32ed96086f4ed3d49530b6917ad7d3) )
	ROM_LOAD( "vid_k06.rv1", 0x008000, 0x008000, BAD_DUMP CRC(76b977c4) SHA1(09988aceaf398279556980e3a21c0dc1b619fb72) )
	ROM_LOAD( "vid_j06.rv1", 0x010000, 0x008000, BAD_DUMP CRC(2a3cc8d0) SHA1(c0165286486a0844baf99c782d2fffdd6ad003b6) )
	ROM_LOAD( "vid_h06.rv1", 0x018000, 0x008000, BAD_DUMP CRC(6763a321) SHA1(15ed912f0346f6b5c3ad23ff22e7493d31ad18a7) )
	ROM_LOAD( "vid_s06.rv1", 0x020000, 0x008000, BAD_DUMP CRC(0a321b7b) SHA1(681317494a0bd50569bb822783336e68551cfd5e) )
	ROM_LOAD( "vid_p06.rv1", 0x028000, 0x008000, BAD_DUMP CRC(5bd089ee) SHA1(9ac98391a6c70d3cfbe609342294668530d690b4) )
	ROM_LOAD( "vid_n06.rv1", 0x030000, 0x008000, BAD_DUMP CRC(c34a517d) SHA1(f0af3db87f73c1fad00a270269ba380898ef5a4b) )
	ROM_LOAD( "vid_m06.rv1", 0x038000, 0x008000, BAD_DUMP CRC(df723956) SHA1(613d398f30463086c0cc720a760bda652e0f3832) )

	ROM_REGION( 0x2000, "chars", 0 )
	ROM_LOAD( "vid_t06.rv1", 0x000000, 0x002000, BAD_DUMP CRC(60d7aebb) SHA1(ad74221c4270496ebcfedd46ea16dca2cda1b4be) )
ROM_END


ROM_START( 720 )
	ROM_REGION( 0x90000, "maincpu", 0 )     // 9 * 64k T11 code
	ROM_LOAD16_BYTE( "136047-3126.7lm", 0x008000, 0x004000, CRC(43abd367) SHA1(bb58c42f25ef0ee5357782652e9e2b28df0ba82e) )
	ROM_LOAD16_BYTE( "136047-3127.7mn", 0x008001, 0x004000, CRC(772e1e5b) SHA1(1ee9b6bd7b2a5e4866b7157db95ee38b53f5c4ce) )
	ROM_LOAD16_BYTE( "136047-3128.6fh", 0x010000, 0x010000, CRC(bf6f425b) SHA1(22732465959c2d30383523e0354b8d3759963765) )
	ROM_LOAD16_BYTE( "136047-4131.6mn", 0x010001, 0x010000, CRC(2ea8a20f) SHA1(927f464e7da540221e341524581cb7bc65e1a31e) )
	ROM_LOAD16_BYTE( "136047-1129.6hj", 0x030000, 0x010000, CRC(eabf0b01) SHA1(aaf5ab31b63c6ba414f0d4c95bbbebcceedd1ae4) )
	ROM_LOAD16_BYTE( "136047-1132.6p",  0x030001, 0x010000, CRC(a24f333e) SHA1(e4bfa4c670bfb375118d5774f1dbe848e39e6460) )
	ROM_LOAD16_BYTE( "136047-1130.6k",  0x050000, 0x010000, CRC(93fba845) SHA1(4de5867272af63be696855f2a4dff99476b213ad) )
	ROM_LOAD16_BYTE( "136047-1133.6r",  0x050001, 0x010000, CRC(53c177be) SHA1(a60c81899944e0dda9886e6697edc4d9309ca8f4) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     // 64k for 6502 code
	ROM_LOAD( "136047-2134.2a",  0x004000, 0x004000, CRC(0db4ca28) SHA1(71c2e0eee0eee418bdd2f806bd6ce5ae1c72bf69) )
	ROM_LOAD( "136047-1135.2b",  0x008000, 0x004000, CRC(b1f157d0) SHA1(26355324d49baa02acb777940d7f49d074a75fe5) )
	ROM_LOAD( "136047-2136.2cd", 0x00c000, 0x004000, CRC(00b06bec) SHA1(cd771eea329e0f6ab5bff1035f931800cc5da545) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "136047-1121.6a",  0x000000, 0x008000, CRC(7adb5f9a) SHA1(8b4dba6c7ecd9d1c03c5d87326b5971ad1cb8863) )
	ROM_LOAD( "136047-1122.6b",  0x008000, 0x008000, CRC(41b60141) SHA1(a426a0a5f6d4b500571731b3ce5ce8acb5e1db92) )
	ROM_LOAD( "136047-1123.7a",  0x010000, 0x008000, CRC(501881d5) SHA1(f38b13774c45eb5b48c87c4410afe4bd311cf3c7) )
	ROM_LOAD( "136047-1124.7b",  0x018000, 0x008000, CRC(096f2574) SHA1(6b59ff9a89a93c39c18011a0ac7043457617f336) )
	ROM_LOAD( "136047-1117.6c",  0x020000, 0x008000, CRC(5a55f149) SHA1(9dbee28a0bc8ec0d3936d61b7359cb63f4860fff) )
	ROM_LOAD( "136047-1118.6d",  0x028000, 0x008000, CRC(9bb2429e) SHA1(80655839e5f53aea19115d83bf395b4f70997edc) )
	ROM_LOAD( "136047-1119.7d",  0x030000, 0x008000, CRC(8f7b20e5) SHA1(9f0928a442f63c66350e66b35b1503fe4f9d8e33) )
	ROM_LOAD( "136047-1120.7c",  0x038000, 0x008000, CRC(46af6d35) SHA1(c3c2b131245f1231839b3649c117bf5bbace0641) )

	ROM_REGION( 0x100000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "136047-1109.6t",  0x020000, 0x008000, CRC(0a46b693) SHA1(77a743816663a8b8fe6bd9aa2dd0a4e570071068) )
	ROM_CONTINUE(                0x000000, 0x008000 )
	ROM_LOAD( "136047-1110.6sr", 0x028000, 0x008000, CRC(457d7e38) SHA1(9ac8e5b49e8f61cb8ce4d739462d17049c966a5d) )
	ROM_CONTINUE(                0x008000, 0x008000 )
	ROM_LOAD( "136047-1111.6p",  0x030000, 0x008000, CRC(ffad0a5b) SHA1(127502a256e31c3fca92323544129ec8fcabacb8) )
	ROM_CONTINUE(                0x010000, 0x008000 )
	ROM_LOAD( "136047-1112.6n",  0x038000, 0x008000, CRC(06664580) SHA1(2173536af27d9af5b506997a5bbcfd5a40e2023a) )
	ROM_CONTINUE(                0x018000, 0x008000 )
	ROM_LOAD( "136047-1113.6m",  0x060000, 0x008000, CRC(7445dc0f) SHA1(cfaa535a4a81a00d0cf47ca3e89625e12abde0f5) )
	ROM_CONTINUE(                0x040000, 0x008000 )
	ROM_LOAD( "136047-1114.6l",  0x068000, 0x008000, CRC(23eaceb0) SHA1(8206da45d09b03c51d5c41fdbe964fec0e399837) )
	ROM_CONTINUE(                0x048000, 0x008000 )
	ROM_LOAD( "136047-1115.6kj", 0x070000, 0x008000, CRC(0cc8de53) SHA1(656fc4011e6ea362f706048a36e99ff31ecbf7cc) )
	ROM_CONTINUE(                0x050000, 0x008000 )
	ROM_LOAD( "136047-1116.6jh", 0x078000, 0x008000, CRC(2d8f1369) SHA1(d35fc5f6733c83d59b0029eb6ee3945e22f0d13b) )
	ROM_CONTINUE(                0x058000, 0x008000 )
	ROM_LOAD( "136047-1101.5t",  0x0a0000, 0x008000, CRC(2ac77b80) SHA1(cae6de4ef8a3cf5fb370c0178f734332369e17da) )
	ROM_CONTINUE(                0x080000, 0x008000 )
	ROM_LOAD( "136047-1102.5sr", 0x0a8000, 0x008000, CRC(f19c3b06) SHA1(12e2194e5cc9604f02bad03dd6f62bba7f459e73) )
	ROM_CONTINUE(                0x088000, 0x008000 )
	ROM_LOAD( "136047-1103.5p",  0x0b0000, 0x008000, CRC(78f9ab90) SHA1(c531e264edaacf61abfbdc8f15b1b47e85a4cdf0) )
	ROM_CONTINUE(                0x090000, 0x008000 )
	ROM_LOAD( "136047-1104.5n",  0x0b8000, 0x008000, CRC(77ce4a7f) SHA1(5c4a6fb01bd744f17cbacc3087c4bdb5e3bfe475) )
	ROM_CONTINUE(                0x098000, 0x008000 )
	ROM_LOAD( "136047-1105.5m",  0x0e0000, 0x008000, CRC(bef5a025) SHA1(5cfe82f1ef2dd95cc5fa317bd59f69c4cd69fdd2) )
	ROM_CONTINUE(                0x0c0000, 0x008000 )
	ROM_LOAD( "136047-1106.5l",  0x0e8000, 0x008000, CRC(92a159c8) SHA1(bc4f06eb666967ac726b7f85719d2fcd74e3b573) )
	ROM_CONTINUE(                0x0c8000, 0x008000 )
	ROM_LOAD( "136047-1107.5kj", 0x0f0000, 0x008000, CRC(0a94a3ef) SHA1(7dec8c768d0673ab3c8211f19b17674531dda308) )
	ROM_CONTINUE(                0x0d0000, 0x008000 )
	ROM_LOAD( "136047-1108.5jh", 0x0f8000, 0x008000, CRC(9815eda6) SHA1(89a80c67f4b3426e7516cd1179d5712779ef5db7) )
	ROM_CONTINUE(                0x0d8000, 0x008000 )

	ROM_REGION( 0x4000, "chars", 0 )
	ROM_LOAD( "136047-1125.4t",  0x000000, 0x004000, CRC(6b7e2328) SHA1(cc9a315ccafe7228951b7c32cf3b31caa89ae7d3) )

	ROM_REGION( 0x200, "eeprom", 0 )
	ROM_LOAD( "720-eeprom.bin", 0x0000, 0x0200, CRC(cfe1c24e) SHA1(5f7623b0a2ff0d99ffa8e6420a5bc03e0c55250d) )
ROM_END


ROM_START( 720r3 )
	ROM_REGION( 0x90000, "maincpu", 0 )     // 9 * 64k T11 code
	ROM_LOAD16_BYTE( "136047-2126.7lm", 0x008000, 0x004000, CRC(d07e731c) SHA1(0ea742c14702f386fdbaf9a51b3a3439c2bc2f6b) )
	ROM_LOAD16_BYTE( "136047-2127.7mn", 0x008001, 0x004000, CRC(2d19116c) SHA1(6f86b735b2c685f51d43c7b290c3fd245daa032c) )
	ROM_LOAD16_BYTE( "136047-2128.6fh", 0x010000, 0x010000, CRC(edad0bc0) SHA1(9e50776221e25573ef6e072e604a52323bc0cef4) )
	ROM_LOAD16_BYTE( "136047-3131.6mn", 0x010001, 0x010000, CRC(704dc925) SHA1(ca0e0eb6826ca30e1ba2e67132ab7496b61abc37) )
	ROM_LOAD16_BYTE( "136047-1129.6hj", 0x030000, 0x010000, CRC(eabf0b01) SHA1(aaf5ab31b63c6ba414f0d4c95bbbebcceedd1ae4) )
	ROM_LOAD16_BYTE( "136047-1132.6p",  0x030001, 0x010000, CRC(a24f333e) SHA1(e4bfa4c670bfb375118d5774f1dbe848e39e6460) )
	ROM_LOAD16_BYTE( "136047-1130.6k",  0x050000, 0x010000, CRC(93fba845) SHA1(4de5867272af63be696855f2a4dff99476b213ad) )
	ROM_LOAD16_BYTE( "136047-1133.6r",  0x050001, 0x010000, CRC(53c177be) SHA1(a60c81899944e0dda9886e6697edc4d9309ca8f4) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     // 64k for 6502 code
	ROM_LOAD( "136047-1134.2a",  0x004000, 0x004000, CRC(09a418c2) SHA1(017491bbcd0def695a23ab17b1e4fbd1fdf4d5d1) )
	ROM_LOAD( "136047-1135.2b",  0x008000, 0x004000, CRC(b1f157d0) SHA1(26355324d49baa02acb777940d7f49d074a75fe5) )
	ROM_LOAD( "136047-1136.2cd", 0x00c000, 0x004000, CRC(dad40e6d) SHA1(a94bc1b5f0a5218e9e44cd32f2ca6268b48072c2) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "136047-1121.6a",  0x000000, 0x008000, CRC(7adb5f9a) SHA1(8b4dba6c7ecd9d1c03c5d87326b5971ad1cb8863) )
	ROM_LOAD( "136047-1122.6b",  0x008000, 0x008000, CRC(41b60141) SHA1(a426a0a5f6d4b500571731b3ce5ce8acb5e1db92) )
	ROM_LOAD( "136047-1123.7a",  0x010000, 0x008000, CRC(501881d5) SHA1(f38b13774c45eb5b48c87c4410afe4bd311cf3c7) )
	ROM_LOAD( "136047-1124.7b",  0x018000, 0x008000, CRC(096f2574) SHA1(6b59ff9a89a93c39c18011a0ac7043457617f336) )
	ROM_LOAD( "136047-1117.6c",  0x020000, 0x008000, CRC(5a55f149) SHA1(9dbee28a0bc8ec0d3936d61b7359cb63f4860fff) )
	ROM_LOAD( "136047-1118.6d",  0x028000, 0x008000, CRC(9bb2429e) SHA1(80655839e5f53aea19115d83bf395b4f70997edc) )
	ROM_LOAD( "136047-1119.7d",  0x030000, 0x008000, CRC(8f7b20e5) SHA1(9f0928a442f63c66350e66b35b1503fe4f9d8e33) )
	ROM_LOAD( "136047-1120.7c",  0x038000, 0x008000, CRC(46af6d35) SHA1(c3c2b131245f1231839b3649c117bf5bbace0641) )

	ROM_REGION( 0x100000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "136047-1109.6t",  0x020000, 0x008000, CRC(0a46b693) SHA1(77a743816663a8b8fe6bd9aa2dd0a4e570071068) )
	ROM_CONTINUE(                0x000000, 0x008000 )
	ROM_LOAD( "136047-1110.6sr", 0x028000, 0x008000, CRC(457d7e38) SHA1(9ac8e5b49e8f61cb8ce4d739462d17049c966a5d) )
	ROM_CONTINUE(                0x008000, 0x008000 )
	ROM_LOAD( "136047-1111.6p",  0x030000, 0x008000, CRC(ffad0a5b) SHA1(127502a256e31c3fca92323544129ec8fcabacb8) )
	ROM_CONTINUE(                0x010000, 0x008000 )
	ROM_LOAD( "136047-1112.6n",  0x038000, 0x008000, CRC(06664580) SHA1(2173536af27d9af5b506997a5bbcfd5a40e2023a) )
	ROM_CONTINUE(                0x018000, 0x008000 )
	ROM_LOAD( "136047-1113.6m",  0x060000, 0x008000, CRC(7445dc0f) SHA1(cfaa535a4a81a00d0cf47ca3e89625e12abde0f5) )
	ROM_CONTINUE(                0x040000, 0x008000 )
	ROM_LOAD( "136047-1114.6l",  0x068000, 0x008000, CRC(23eaceb0) SHA1(8206da45d09b03c51d5c41fdbe964fec0e399837) )
	ROM_CONTINUE(                0x048000, 0x008000 )
	ROM_LOAD( "136047-1115.6kj", 0x070000, 0x008000, CRC(0cc8de53) SHA1(656fc4011e6ea362f706048a36e99ff31ecbf7cc) )
	ROM_CONTINUE(                0x050000, 0x008000 )
	ROM_LOAD( "136047-1116.6jh", 0x078000, 0x008000, CRC(2d8f1369) SHA1(d35fc5f6733c83d59b0029eb6ee3945e22f0d13b) )
	ROM_CONTINUE(                0x058000, 0x008000 )
	ROM_LOAD( "136047-1101.5t",  0x0a0000, 0x008000, CRC(2ac77b80) SHA1(cae6de4ef8a3cf5fb370c0178f734332369e17da) )
	ROM_CONTINUE(                0x080000, 0x008000 )
	ROM_LOAD( "136047-1102.5sr", 0x0a8000, 0x008000, CRC(f19c3b06) SHA1(12e2194e5cc9604f02bad03dd6f62bba7f459e73) )
	ROM_CONTINUE(                0x088000, 0x008000 )
	ROM_LOAD( "136047-1103.5p",  0x0b0000, 0x008000, CRC(78f9ab90) SHA1(c531e264edaacf61abfbdc8f15b1b47e85a4cdf0) )
	ROM_CONTINUE(                0x090000, 0x008000 )
	ROM_LOAD( "136047-1104.5n",  0x0b8000, 0x008000, CRC(77ce4a7f) SHA1(5c4a6fb01bd744f17cbacc3087c4bdb5e3bfe475) )
	ROM_CONTINUE(                0x098000, 0x008000 )
	ROM_LOAD( "136047-1105.5m",  0x0e0000, 0x008000, CRC(bef5a025) SHA1(5cfe82f1ef2dd95cc5fa317bd59f69c4cd69fdd2) )
	ROM_CONTINUE(                0x0c0000, 0x008000 )
	ROM_LOAD( "136047-1106.5l",  0x0e8000, 0x008000, CRC(92a159c8) SHA1(bc4f06eb666967ac726b7f85719d2fcd74e3b573) )
	ROM_CONTINUE(                0x0c8000, 0x008000 )
	ROM_LOAD( "136047-1107.5kj", 0x0f0000, 0x008000, CRC(0a94a3ef) SHA1(7dec8c768d0673ab3c8211f19b17674531dda308) )
	ROM_CONTINUE(                0x0d0000, 0x008000 )
	ROM_LOAD( "136047-1108.5jh", 0x0f8000, 0x008000, CRC(9815eda6) SHA1(89a80c67f4b3426e7516cd1179d5712779ef5db7) )
	ROM_CONTINUE(                0x0d8000, 0x008000 )

	ROM_REGION( 0x4000, "chars", 0 )
	ROM_LOAD( "136047-1125.4t",  0x000000, 0x004000, CRC(6b7e2328) SHA1(cc9a315ccafe7228951b7c32cf3b31caa89ae7d3) )

	ROM_REGION( 0x200, "eeprom", 0 )
	ROM_LOAD( "720-eeprom.bin", 0x0000, 0x0200, CRC(cfe1c24e) SHA1(5f7623b0a2ff0d99ffa8e6420a5bc03e0c55250d) )
ROM_END


ROM_START( 720r2 )
	ROM_REGION( 0x90000, "maincpu", 0 )     // 9 * 64k T11 code
	ROM_LOAD16_BYTE( "136047-2126.7lm", 0x008000, 0x004000, CRC(d07e731c) SHA1(0ea742c14702f386fdbaf9a51b3a3439c2bc2f6b) )
	ROM_LOAD16_BYTE( "136047-2127.7mn", 0x008001, 0x004000, CRC(2d19116c) SHA1(6f86b735b2c685f51d43c7b290c3fd245daa032c) )
	ROM_LOAD16_BYTE( "136047-2128.6fh", 0x010000, 0x010000, CRC(edad0bc0) SHA1(9e50776221e25573ef6e072e604a52323bc0cef4) )
	ROM_LOAD16_BYTE( "136047-2131.6mn", 0x010001, 0x010000, CRC(bfdd95a4) SHA1(0d07a5401706b0df01f8797962c61866043db88c) )
	ROM_LOAD16_BYTE( "136047-1129.6hj", 0x030000, 0x010000, CRC(eabf0b01) SHA1(aaf5ab31b63c6ba414f0d4c95bbbebcceedd1ae4) )
	ROM_LOAD16_BYTE( "136047-1132.6p",  0x030001, 0x010000, CRC(a24f333e) SHA1(e4bfa4c670bfb375118d5774f1dbe848e39e6460) )
	ROM_LOAD16_BYTE( "136047-1130.6k",  0x050000, 0x010000, CRC(93fba845) SHA1(4de5867272af63be696855f2a4dff99476b213ad) )
	ROM_LOAD16_BYTE( "136047-1133.6r",  0x050001, 0x010000, CRC(53c177be) SHA1(a60c81899944e0dda9886e6697edc4d9309ca8f4) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     // 64k for 6502 code
	ROM_LOAD( "136047-1134.2a",  0x004000, 0x004000, CRC(09a418c2) SHA1(017491bbcd0def695a23ab17b1e4fbd1fdf4d5d1) )
	ROM_LOAD( "136047-1135.2b",  0x008000, 0x004000, CRC(b1f157d0) SHA1(26355324d49baa02acb777940d7f49d074a75fe5) )
	ROM_LOAD( "136047-1136.2cd", 0x00c000, 0x004000, CRC(dad40e6d) SHA1(a94bc1b5f0a5218e9e44cd32f2ca6268b48072c2) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "136047-1121.6a",  0x000000, 0x008000, CRC(7adb5f9a) SHA1(8b4dba6c7ecd9d1c03c5d87326b5971ad1cb8863) )
	ROM_LOAD( "136047-1122.6b",  0x008000, 0x008000, CRC(41b60141) SHA1(a426a0a5f6d4b500571731b3ce5ce8acb5e1db92) )
	ROM_LOAD( "136047-1123.7a",  0x010000, 0x008000, CRC(501881d5) SHA1(f38b13774c45eb5b48c87c4410afe4bd311cf3c7) )
	ROM_LOAD( "136047-1124.7b",  0x018000, 0x008000, CRC(096f2574) SHA1(6b59ff9a89a93c39c18011a0ac7043457617f336) )
	ROM_LOAD( "136047-1117.6c",  0x020000, 0x008000, CRC(5a55f149) SHA1(9dbee28a0bc8ec0d3936d61b7359cb63f4860fff) )
	ROM_LOAD( "136047-1118.6d",  0x028000, 0x008000, CRC(9bb2429e) SHA1(80655839e5f53aea19115d83bf395b4f70997edc) )
	ROM_LOAD( "136047-1119.7d",  0x030000, 0x008000, CRC(8f7b20e5) SHA1(9f0928a442f63c66350e66b35b1503fe4f9d8e33) )
	ROM_LOAD( "136047-1120.7c",  0x038000, 0x008000, CRC(46af6d35) SHA1(c3c2b131245f1231839b3649c117bf5bbace0641) )

	ROM_REGION( 0x100000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "136047-1109.6t",  0x020000, 0x008000, CRC(0a46b693) SHA1(77a743816663a8b8fe6bd9aa2dd0a4e570071068) )
	ROM_CONTINUE(                0x000000, 0x008000 )
	ROM_LOAD( "136047-1110.6sr", 0x028000, 0x008000, CRC(457d7e38) SHA1(9ac8e5b49e8f61cb8ce4d739462d17049c966a5d) )
	ROM_CONTINUE(                0x008000, 0x008000 )
	ROM_LOAD( "136047-1111.6p",  0x030000, 0x008000, CRC(ffad0a5b) SHA1(127502a256e31c3fca92323544129ec8fcabacb8) )
	ROM_CONTINUE(                0x010000, 0x008000 )
	ROM_LOAD( "136047-1112.6n",  0x038000, 0x008000, CRC(06664580) SHA1(2173536af27d9af5b506997a5bbcfd5a40e2023a) )
	ROM_CONTINUE(                0x018000, 0x008000 )
	ROM_LOAD( "136047-1113.6m",  0x060000, 0x008000, CRC(7445dc0f) SHA1(cfaa535a4a81a00d0cf47ca3e89625e12abde0f5) )
	ROM_CONTINUE(                0x040000, 0x008000 )
	ROM_LOAD( "136047-1114.6l",  0x068000, 0x008000, CRC(23eaceb0) SHA1(8206da45d09b03c51d5c41fdbe964fec0e399837) )
	ROM_CONTINUE(                0x048000, 0x008000 )
	ROM_LOAD( "136047-1115.6kj", 0x070000, 0x008000, CRC(0cc8de53) SHA1(656fc4011e6ea362f706048a36e99ff31ecbf7cc) )
	ROM_CONTINUE(                0x050000, 0x008000 )
	ROM_LOAD( "136047-1116.6jh", 0x078000, 0x008000, CRC(2d8f1369) SHA1(d35fc5f6733c83d59b0029eb6ee3945e22f0d13b) )
	ROM_CONTINUE(                0x058000, 0x008000 )
	ROM_LOAD( "136047-1101.5t",  0x0a0000, 0x008000, CRC(2ac77b80) SHA1(cae6de4ef8a3cf5fb370c0178f734332369e17da) )
	ROM_CONTINUE(                0x080000, 0x008000 )
	ROM_LOAD( "136047-1102.5sr", 0x0a8000, 0x008000, CRC(f19c3b06) SHA1(12e2194e5cc9604f02bad03dd6f62bba7f459e73) )
	ROM_CONTINUE(                0x088000, 0x008000 )
	ROM_LOAD( "136047-1103.5p",  0x0b0000, 0x008000, CRC(78f9ab90) SHA1(c531e264edaacf61abfbdc8f15b1b47e85a4cdf0) )
	ROM_CONTINUE(                0x090000, 0x008000 )
	ROM_LOAD( "136047-1104.5n",  0x0b8000, 0x008000, CRC(77ce4a7f) SHA1(5c4a6fb01bd744f17cbacc3087c4bdb5e3bfe475) )
	ROM_CONTINUE(                0x098000, 0x008000 )
	ROM_LOAD( "136047-1105.5m",  0x0e0000, 0x008000, CRC(bef5a025) SHA1(5cfe82f1ef2dd95cc5fa317bd59f69c4cd69fdd2) )
	ROM_CONTINUE(                0x0c0000, 0x008000 )
	ROM_LOAD( "136047-1106.5l",  0x0e8000, 0x008000, CRC(92a159c8) SHA1(bc4f06eb666967ac726b7f85719d2fcd74e3b573) )
	ROM_CONTINUE(                0x0c8000, 0x008000 )
	ROM_LOAD( "136047-1107.5kj", 0x0f0000, 0x008000, CRC(0a94a3ef) SHA1(7dec8c768d0673ab3c8211f19b17674531dda308) )
	ROM_CONTINUE(                0x0d0000, 0x008000 )
	ROM_LOAD( "136047-1108.5jh", 0x0f8000, 0x008000, CRC(9815eda6) SHA1(89a80c67f4b3426e7516cd1179d5712779ef5db7) )
	ROM_CONTINUE(                0x0d8000, 0x008000 )

	ROM_REGION( 0x4000, "chars", 0 )
	ROM_LOAD( "136047-1125.4t",  0x000000, 0x004000, CRC(6b7e2328) SHA1(cc9a315ccafe7228951b7c32cf3b31caa89ae7d3) )

	ROM_REGION( 0x200, "eeprom", 0 )
	ROM_LOAD( "720-eeprom.bin", 0x0000, 0x0200, CRC(cfe1c24e) SHA1(5f7623b0a2ff0d99ffa8e6420a5bc03e0c55250d) )
ROM_END


ROM_START( 720r1 )
	ROM_REGION( 0x90000, "maincpu", 0 )     // 9 * 64k T11 code
	ROM_LOAD16_BYTE( "136047-1126.7lm", 0x008000, 0x004000, CRC(f0ef298a) SHA1(5bbb84666333cd81ef3c5ffe7ad75a768c2af9aa) )
	ROM_LOAD16_BYTE( "136047-1127.7mn", 0x008001, 0x004000, CRC(57e49398) SHA1(511310ff83067f62532894617c6a7de29807f265) )
	ROM_LOAD16_BYTE( "136047-1128.6fh", 0x010000, 0x010000, CRC(2884dcff) SHA1(4a41a5064137673353ac6000d1435742f7b394c3) )
	ROM_LOAD16_BYTE( "136047-1131.6mn", 0x010001, 0x010000, CRC(94c8195e) SHA1(93840665e7ed5ceeb4d58ae11713263791f96b6f) )
	ROM_LOAD16_BYTE( "136047-1129.6hj", 0x030000, 0x010000, CRC(eabf0b01) SHA1(aaf5ab31b63c6ba414f0d4c95bbbebcceedd1ae4) )
	ROM_LOAD16_BYTE( "136047-1132.6p",  0x030001, 0x010000, CRC(a24f333e) SHA1(e4bfa4c670bfb375118d5774f1dbe848e39e6460) )
	ROM_LOAD16_BYTE( "136047-1130.6k",  0x050000, 0x010000, CRC(93fba845) SHA1(4de5867272af63be696855f2a4dff99476b213ad) )
	ROM_LOAD16_BYTE( "136047-1133.6r",  0x050001, 0x010000, CRC(53c177be) SHA1(a60c81899944e0dda9886e6697edc4d9309ca8f4) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     // 64k for 6502 code
	ROM_LOAD( "136047-1134.2a",  0x004000, 0x004000, CRC(09a418c2) SHA1(017491bbcd0def695a23ab17b1e4fbd1fdf4d5d1) )
	ROM_LOAD( "136047-1135.2b",  0x008000, 0x004000, CRC(b1f157d0) SHA1(26355324d49baa02acb777940d7f49d074a75fe5) )
	ROM_LOAD( "136047-1136.2cd", 0x00c000, 0x004000, CRC(dad40e6d) SHA1(a94bc1b5f0a5218e9e44cd32f2ca6268b48072c2) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "136047-1121.6a",  0x000000, 0x008000, CRC(7adb5f9a) SHA1(8b4dba6c7ecd9d1c03c5d87326b5971ad1cb8863) )
	ROM_LOAD( "136047-1122.6b",  0x008000, 0x008000, CRC(41b60141) SHA1(a426a0a5f6d4b500571731b3ce5ce8acb5e1db92) )
	ROM_LOAD( "136047-1123.7a",  0x010000, 0x008000, CRC(501881d5) SHA1(f38b13774c45eb5b48c87c4410afe4bd311cf3c7) )
	ROM_LOAD( "136047-1124.7b",  0x018000, 0x008000, CRC(096f2574) SHA1(6b59ff9a89a93c39c18011a0ac7043457617f336) )
	ROM_LOAD( "136047-1117.6c",  0x020000, 0x008000, CRC(5a55f149) SHA1(9dbee28a0bc8ec0d3936d61b7359cb63f4860fff) )
	ROM_LOAD( "136047-1118.6d",  0x028000, 0x008000, CRC(9bb2429e) SHA1(80655839e5f53aea19115d83bf395b4f70997edc) )
	ROM_LOAD( "136047-1119.7d",  0x030000, 0x008000, CRC(8f7b20e5) SHA1(9f0928a442f63c66350e66b35b1503fe4f9d8e33) )
	ROM_LOAD( "136047-1120.7c",  0x038000, 0x008000, CRC(46af6d35) SHA1(c3c2b131245f1231839b3649c117bf5bbace0641) )

	ROM_REGION( 0x100000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "136047-1109.6t",  0x020000, 0x008000, CRC(0a46b693) SHA1(77a743816663a8b8fe6bd9aa2dd0a4e570071068) )
	ROM_CONTINUE(                0x000000, 0x008000 )
	ROM_LOAD( "136047-1110.6sr", 0x028000, 0x008000, CRC(457d7e38) SHA1(9ac8e5b49e8f61cb8ce4d739462d17049c966a5d) )
	ROM_CONTINUE(                0x008000, 0x008000 )
	ROM_LOAD( "136047-1111.6p",  0x030000, 0x008000, CRC(ffad0a5b) SHA1(127502a256e31c3fca92323544129ec8fcabacb8) )
	ROM_CONTINUE(                0x010000, 0x008000 )
	ROM_LOAD( "136047-1112.6n",  0x038000, 0x008000, CRC(06664580) SHA1(2173536af27d9af5b506997a5bbcfd5a40e2023a) )
	ROM_CONTINUE(                0x018000, 0x008000 )
	ROM_LOAD( "136047-1113.6m",  0x060000, 0x008000, CRC(7445dc0f) SHA1(cfaa535a4a81a00d0cf47ca3e89625e12abde0f5) )
	ROM_CONTINUE(                0x040000, 0x008000 )
	ROM_LOAD( "136047-1114.6l",  0x068000, 0x008000, CRC(23eaceb0) SHA1(8206da45d09b03c51d5c41fdbe964fec0e399837) )
	ROM_CONTINUE(                0x048000, 0x008000 )
	ROM_LOAD( "136047-1115.6kj", 0x070000, 0x008000, CRC(0cc8de53) SHA1(656fc4011e6ea362f706048a36e99ff31ecbf7cc) )
	ROM_CONTINUE(                0x050000, 0x008000 )
	ROM_LOAD( "136047-1116.6jh", 0x078000, 0x008000, CRC(2d8f1369) SHA1(d35fc5f6733c83d59b0029eb6ee3945e22f0d13b) )
	ROM_CONTINUE(                0x058000, 0x008000 )
	ROM_LOAD( "136047-1101.5t",  0x0a0000, 0x008000, CRC(2ac77b80) SHA1(cae6de4ef8a3cf5fb370c0178f734332369e17da) )
	ROM_CONTINUE(                0x080000, 0x008000 )
	ROM_LOAD( "136047-1102.5sr", 0x0a8000, 0x008000, CRC(f19c3b06) SHA1(12e2194e5cc9604f02bad03dd6f62bba7f459e73) )
	ROM_CONTINUE(                0x088000, 0x008000 )
	ROM_LOAD( "136047-1103.5p",  0x0b0000, 0x008000, CRC(78f9ab90) SHA1(c531e264edaacf61abfbdc8f15b1b47e85a4cdf0) )
	ROM_CONTINUE(                0x090000, 0x008000 )
	ROM_LOAD( "136047-1104.5n",  0x0b8000, 0x008000, CRC(77ce4a7f) SHA1(5c4a6fb01bd744f17cbacc3087c4bdb5e3bfe475) )
	ROM_CONTINUE(                0x098000, 0x008000 )
	ROM_LOAD( "136047-1105.5m",  0x0e0000, 0x008000, CRC(bef5a025) SHA1(5cfe82f1ef2dd95cc5fa317bd59f69c4cd69fdd2) )
	ROM_CONTINUE(                0x0c0000, 0x008000 )
	ROM_LOAD( "136047-1106.5l",  0x0e8000, 0x008000, CRC(92a159c8) SHA1(bc4f06eb666967ac726b7f85719d2fcd74e3b573) )
	ROM_CONTINUE(                0x0c8000, 0x008000 )
	ROM_LOAD( "136047-1107.5kj", 0x0f0000, 0x008000, CRC(0a94a3ef) SHA1(7dec8c768d0673ab3c8211f19b17674531dda308) )
	ROM_CONTINUE(                0x0d0000, 0x008000 )
	ROM_LOAD( "136047-1108.5jh", 0x0f8000, 0x008000, CRC(9815eda6) SHA1(89a80c67f4b3426e7516cd1179d5712779ef5db7) )
	ROM_CONTINUE(                0x0d8000, 0x008000 )

	ROM_REGION( 0x4000, "chars", 0 )
	ROM_LOAD( "136047-1125.4t",  0x000000, 0x004000, CRC(6b7e2328) SHA1(cc9a315ccafe7228951b7c32cf3b31caa89ae7d3) )

	ROM_REGION( 0x200, "eeprom", 0 )
	ROM_LOAD( "720-eeprom.bin", 0x0000, 0x0200, CRC(cfe1c24e) SHA1(5f7623b0a2ff0d99ffa8e6420a5bc03e0c55250d) )
ROM_END


ROM_START( 720g )
	ROM_REGION( 0x90000, "maincpu", 0 )     // 9 * 64k T11 code
	ROM_LOAD16_BYTE( "136047-3226.7lm", 0x008000, 0x004000, CRC(472be9aa) SHA1(4635df889d4e5b798074950ebb433c3f101a414d) )
	ROM_LOAD16_BYTE( "136047-2227.7mn", 0x008001, 0x004000, CRC(c628fcc9) SHA1(ed8194e10a6d66216b4977be77a197f7f9918899) )
	ROM_LOAD16_BYTE( "136047-3228.6fh", 0x010000, 0x010000, CRC(10bbbce7) SHA1(cc23c606151ae7a49eef051c8e0649230902e705) )
	ROM_LOAD16_BYTE( "136047-4231.6mn", 0x010001, 0x010000, CRC(c29188b0) SHA1(8f9098719fb007298571ff6430bda4b757368f1c) )
	ROM_LOAD16_BYTE( "136047-1129.6hj", 0x030000, 0x010000, CRC(eabf0b01) SHA1(aaf5ab31b63c6ba414f0d4c95bbbebcceedd1ae4) )
	ROM_LOAD16_BYTE( "136047-1132.6p",  0x030001, 0x010000, CRC(a24f333e) SHA1(e4bfa4c670bfb375118d5774f1dbe848e39e6460) )
	ROM_LOAD16_BYTE( "136047-1130.6k",  0x050000, 0x010000, CRC(93fba845) SHA1(4de5867272af63be696855f2a4dff99476b213ad) )
	ROM_LOAD16_BYTE( "136047-1133.6r",  0x050001, 0x010000, CRC(53c177be) SHA1(a60c81899944e0dda9886e6697edc4d9309ca8f4) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     // 64k for 6502 code
	ROM_LOAD( "136047-2134.2a",  0x004000, 0x004000, CRC(0db4ca28) SHA1(71c2e0eee0eee418bdd2f806bd6ce5ae1c72bf69) )
	ROM_LOAD( "136047-1135.2b",  0x008000, 0x004000, CRC(b1f157d0) SHA1(26355324d49baa02acb777940d7f49d074a75fe5) )
	ROM_LOAD( "136047-2136.2cd", 0x00c000, 0x004000, CRC(00b06bec) SHA1(cd771eea329e0f6ab5bff1035f931800cc5da545) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "136047-1121.6a",  0x000000, 0x008000, CRC(7adb5f9a) SHA1(8b4dba6c7ecd9d1c03c5d87326b5971ad1cb8863) )
	ROM_LOAD( "136047-1122.6b",  0x008000, 0x008000, CRC(41b60141) SHA1(a426a0a5f6d4b500571731b3ce5ce8acb5e1db92) )
	ROM_LOAD( "136047-1123.7a",  0x010000, 0x008000, CRC(501881d5) SHA1(f38b13774c45eb5b48c87c4410afe4bd311cf3c7) )
	ROM_LOAD( "136047-1124.7b",  0x018000, 0x008000, CRC(096f2574) SHA1(6b59ff9a89a93c39c18011a0ac7043457617f336) )
	ROM_LOAD( "136047-1117.6c",  0x020000, 0x008000, CRC(5a55f149) SHA1(9dbee28a0bc8ec0d3936d61b7359cb63f4860fff) )
	ROM_LOAD( "136047-1118.6d",  0x028000, 0x008000, CRC(9bb2429e) SHA1(80655839e5f53aea19115d83bf395b4f70997edc) )
	ROM_LOAD( "136047-1119.7d",  0x030000, 0x008000, CRC(8f7b20e5) SHA1(9f0928a442f63c66350e66b35b1503fe4f9d8e33) )
	ROM_LOAD( "136047-1120.7c",  0x038000, 0x008000, CRC(46af6d35) SHA1(c3c2b131245f1231839b3649c117bf5bbace0641) )

	ROM_REGION( 0x100000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "136047-1109.6t",  0x020000, 0x008000, CRC(0a46b693) SHA1(77a743816663a8b8fe6bd9aa2dd0a4e570071068) )
	ROM_CONTINUE(                0x000000, 0x008000 )
	ROM_LOAD( "136047-1110.6sr", 0x028000, 0x008000, CRC(457d7e38) SHA1(9ac8e5b49e8f61cb8ce4d739462d17049c966a5d) )
	ROM_CONTINUE(                0x008000, 0x008000 )
	ROM_LOAD( "136047-1111.6p",  0x030000, 0x008000, CRC(ffad0a5b) SHA1(127502a256e31c3fca92323544129ec8fcabacb8) )
	ROM_CONTINUE(                0x010000, 0x008000 )
	ROM_LOAD( "136047-1112.6n",  0x038000, 0x008000, CRC(06664580) SHA1(2173536af27d9af5b506997a5bbcfd5a40e2023a) )
	ROM_CONTINUE(                0x018000, 0x008000 )
	ROM_LOAD( "136047-1113.6m",  0x060000, 0x008000, CRC(7445dc0f) SHA1(cfaa535a4a81a00d0cf47ca3e89625e12abde0f5) )
	ROM_CONTINUE(                0x040000, 0x008000 )
	ROM_LOAD( "136047-1114.6l",  0x068000, 0x008000, CRC(23eaceb0) SHA1(8206da45d09b03c51d5c41fdbe964fec0e399837) )
	ROM_CONTINUE(                0x048000, 0x008000 )
	ROM_LOAD( "136047-1115.6kj", 0x070000, 0x008000, CRC(0cc8de53) SHA1(656fc4011e6ea362f706048a36e99ff31ecbf7cc) )
	ROM_CONTINUE(                0x050000, 0x008000 )
	ROM_LOAD( "136047-1116.6jh", 0x078000, 0x008000, CRC(2d8f1369) SHA1(d35fc5f6733c83d59b0029eb6ee3945e22f0d13b) )
	ROM_CONTINUE(                0x058000, 0x008000 )
	ROM_LOAD( "136047-1101.5t",  0x0a0000, 0x008000, CRC(2ac77b80) SHA1(cae6de4ef8a3cf5fb370c0178f734332369e17da) )
	ROM_CONTINUE(                0x080000, 0x008000 )
	ROM_LOAD( "136047-1102.5sr", 0x0a8000, 0x008000, CRC(f19c3b06) SHA1(12e2194e5cc9604f02bad03dd6f62bba7f459e73) )
	ROM_CONTINUE(                0x088000, 0x008000 )
	ROM_LOAD( "136047-1103.5p",  0x0b0000, 0x008000, CRC(78f9ab90) SHA1(c531e264edaacf61abfbdc8f15b1b47e85a4cdf0) )
	ROM_CONTINUE(                0x090000, 0x008000 )
	ROM_LOAD( "136047-1104.5n",  0x0b8000, 0x008000, CRC(77ce4a7f) SHA1(5c4a6fb01bd744f17cbacc3087c4bdb5e3bfe475) )
	ROM_CONTINUE(                0x098000, 0x008000 )
	ROM_LOAD( "136047-1105.5m",  0x0e0000, 0x008000, CRC(bef5a025) SHA1(5cfe82f1ef2dd95cc5fa317bd59f69c4cd69fdd2) )
	ROM_CONTINUE(                0x0c0000, 0x008000 )
	ROM_LOAD( "136047-1106.5l",  0x0e8000, 0x008000, CRC(92a159c8) SHA1(bc4f06eb666967ac726b7f85719d2fcd74e3b573) )
	ROM_CONTINUE(                0x0c8000, 0x008000 )
	ROM_LOAD( "136047-1107.5kj", 0x0f0000, 0x008000, CRC(0a94a3ef) SHA1(7dec8c768d0673ab3c8211f19b17674531dda308) )
	ROM_CONTINUE(                0x0d0000, 0x008000 )
	ROM_LOAD( "136047-1108.5jh", 0x0f8000, 0x008000, CRC(9815eda6) SHA1(89a80c67f4b3426e7516cd1179d5712779ef5db7) )
	ROM_CONTINUE(                0x0d8000, 0x008000 )

	ROM_REGION( 0x4000, "chars", 0 )
	ROM_LOAD( "136047-1225.4t",  0x000000, 0x004000, CRC(264eda88) SHA1(f0f5fe87741e0e17117085cf45f700090a02cb94) )

	ROM_REGION( 0x200, "eeprom", 0 )
	ROM_LOAD( "720-eeprom.bin", 0x0000, 0x0200, CRC(cfe1c24e) SHA1(5f7623b0a2ff0d99ffa8e6420a5bc03e0c55250d) )
ROM_END


ROM_START( 720gr1 )
	ROM_REGION( 0x90000, "maincpu", 0 )     // 9 * 64k T11 code
	ROM_LOAD16_BYTE( "136047-2226.7lm", 0x008000, 0x004000, CRC(bbe90b2a) SHA1(a7310b0d71db11f1fac0724b1f9aef425f50b691) )
	ROM_LOAD16_BYTE( "136047-2227.7mn", 0x008001, 0x004000, CRC(c628fcc9) SHA1(ed8194e10a6d66216b4977be77a197f7f9918899) )
	ROM_LOAD16_BYTE( "136047-2228.6fh", 0x010000, 0x010000, CRC(a115aa94) SHA1(226b62d3a5caa6ada774ae79f4a92e09bacee89b) )
	ROM_LOAD16_BYTE( "136047-3231.6mn", 0x010001, 0x010000, CRC(b704e865) SHA1(6e1ae13056d182c535feac625fb102601b87adf5) )
	ROM_LOAD16_BYTE( "136047-1129.6hj", 0x030000, 0x010000, CRC(eabf0b01) SHA1(aaf5ab31b63c6ba414f0d4c95bbbebcceedd1ae4) )
	ROM_LOAD16_BYTE( "136047-1132.6p",  0x030001, 0x010000, CRC(a24f333e) SHA1(e4bfa4c670bfb375118d5774f1dbe848e39e6460) )
	ROM_LOAD16_BYTE( "136047-1130.6k",  0x050000, 0x010000, CRC(93fba845) SHA1(4de5867272af63be696855f2a4dff99476b213ad) )
	ROM_LOAD16_BYTE( "136047-1133.6r",  0x050001, 0x010000, CRC(53c177be) SHA1(a60c81899944e0dda9886e6697edc4d9309ca8f4) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     // 64k for 6502 code
	ROM_LOAD( "136047-1134.2a",  0x004000, 0x004000, CRC(09a418c2) SHA1(017491bbcd0def695a23ab17b1e4fbd1fdf4d5d1) )
	ROM_LOAD( "136047-1135.2b",  0x008000, 0x004000, CRC(b1f157d0) SHA1(26355324d49baa02acb777940d7f49d074a75fe5) )
	ROM_LOAD( "136047-1136.2cd", 0x00c000, 0x004000, CRC(dad40e6d) SHA1(a94bc1b5f0a5218e9e44cd32f2ca6268b48072c2) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "136047-1121.6a",  0x000000, 0x008000, CRC(7adb5f9a) SHA1(8b4dba6c7ecd9d1c03c5d87326b5971ad1cb8863) )
	ROM_LOAD( "136047-1122.6b",  0x008000, 0x008000, CRC(41b60141) SHA1(a426a0a5f6d4b500571731b3ce5ce8acb5e1db92) )
	ROM_LOAD( "136047-1123.7a",  0x010000, 0x008000, CRC(501881d5) SHA1(f38b13774c45eb5b48c87c4410afe4bd311cf3c7) )
	ROM_LOAD( "136047-1124.7b",  0x018000, 0x008000, CRC(096f2574) SHA1(6b59ff9a89a93c39c18011a0ac7043457617f336) )
	ROM_LOAD( "136047-1117.6c",  0x020000, 0x008000, CRC(5a55f149) SHA1(9dbee28a0bc8ec0d3936d61b7359cb63f4860fff) )
	ROM_LOAD( "136047-1118.6d",  0x028000, 0x008000, CRC(9bb2429e) SHA1(80655839e5f53aea19115d83bf395b4f70997edc) )
	ROM_LOAD( "136047-1119.7d",  0x030000, 0x008000, CRC(8f7b20e5) SHA1(9f0928a442f63c66350e66b35b1503fe4f9d8e33) )
	ROM_LOAD( "136047-1120.7c",  0x038000, 0x008000, CRC(46af6d35) SHA1(c3c2b131245f1231839b3649c117bf5bbace0641) )

	ROM_REGION( 0x100000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "136047-1109.6t",  0x020000, 0x008000, CRC(0a46b693) SHA1(77a743816663a8b8fe6bd9aa2dd0a4e570071068) )
	ROM_CONTINUE(                0x000000, 0x008000 )
	ROM_LOAD( "136047-1110.6sr", 0x028000, 0x008000, CRC(457d7e38) SHA1(9ac8e5b49e8f61cb8ce4d739462d17049c966a5d) )
	ROM_CONTINUE(                0x008000, 0x008000 )
	ROM_LOAD( "136047-1111.6p",  0x030000, 0x008000, CRC(ffad0a5b) SHA1(127502a256e31c3fca92323544129ec8fcabacb8) )
	ROM_CONTINUE(                0x010000, 0x008000 )
	ROM_LOAD( "136047-1112.6n",  0x038000, 0x008000, CRC(06664580) SHA1(2173536af27d9af5b506997a5bbcfd5a40e2023a) )
	ROM_CONTINUE(                0x018000, 0x008000 )
	ROM_LOAD( "136047-1113.6m",  0x060000, 0x008000, CRC(7445dc0f) SHA1(cfaa535a4a81a00d0cf47ca3e89625e12abde0f5) )
	ROM_CONTINUE(                0x040000, 0x008000 )
	ROM_LOAD( "136047-1114.6l",  0x068000, 0x008000, CRC(23eaceb0) SHA1(8206da45d09b03c51d5c41fdbe964fec0e399837) )
	ROM_CONTINUE(                0x048000, 0x008000 )
	ROM_LOAD( "136047-1115.6kj", 0x070000, 0x008000, CRC(0cc8de53) SHA1(656fc4011e6ea362f706048a36e99ff31ecbf7cc) )
	ROM_CONTINUE(                0x050000, 0x008000 )
	ROM_LOAD( "136047-1116.6jh", 0x078000, 0x008000, CRC(2d8f1369) SHA1(d35fc5f6733c83d59b0029eb6ee3945e22f0d13b) )
	ROM_CONTINUE(                0x058000, 0x008000 )
	ROM_LOAD( "136047-1101.5t",  0x0a0000, 0x008000, CRC(2ac77b80) SHA1(cae6de4ef8a3cf5fb370c0178f734332369e17da) )
	ROM_CONTINUE(                0x080000, 0x008000 )
	ROM_LOAD( "136047-1102.5sr", 0x0a8000, 0x008000, CRC(f19c3b06) SHA1(12e2194e5cc9604f02bad03dd6f62bba7f459e73) )
	ROM_CONTINUE(                0x088000, 0x008000 )
	ROM_LOAD( "136047-1103.5p",  0x0b0000, 0x008000, CRC(78f9ab90) SHA1(c531e264edaacf61abfbdc8f15b1b47e85a4cdf0) )
	ROM_CONTINUE(                0x090000, 0x008000 )
	ROM_LOAD( "136047-1104.5n",  0x0b8000, 0x008000, CRC(77ce4a7f) SHA1(5c4a6fb01bd744f17cbacc3087c4bdb5e3bfe475) )
	ROM_CONTINUE(                0x098000, 0x008000 )
	ROM_LOAD( "136047-1105.5m",  0x0e0000, 0x008000, CRC(bef5a025) SHA1(5cfe82f1ef2dd95cc5fa317bd59f69c4cd69fdd2) )
	ROM_CONTINUE(                0x0c0000, 0x008000 )
	ROM_LOAD( "136047-1106.5l",  0x0e8000, 0x008000, CRC(92a159c8) SHA1(bc4f06eb666967ac726b7f85719d2fcd74e3b573) )
	ROM_CONTINUE(                0x0c8000, 0x008000 )
	ROM_LOAD( "136047-1107.5kj", 0x0f0000, 0x008000, CRC(0a94a3ef) SHA1(7dec8c768d0673ab3c8211f19b17674531dda308) )
	ROM_CONTINUE(                0x0d0000, 0x008000 )
	ROM_LOAD( "136047-1108.5jh", 0x0f8000, 0x008000, CRC(9815eda6) SHA1(89a80c67f4b3426e7516cd1179d5712779ef5db7) )
	ROM_CONTINUE(                0x0d8000, 0x008000 )

	ROM_REGION( 0x4000, "chars", 0 )
	ROM_LOAD( "136047-1225.4t",  0x000000, 0x004000, CRC(264eda88) SHA1(f0f5fe87741e0e17117085cf45f700090a02cb94) )

	ROM_REGION( 0x200, "eeprom", 0 )
	ROM_LOAD( "720-eeprom.bin", 0x0000, 0x0200, CRC(cfe1c24e) SHA1(5f7623b0a2ff0d99ffa8e6420a5bc03e0c55250d) )
ROM_END


ROM_START( ssprint )
	ROM_REGION( 0x90000, "maincpu", 0 ) // 9*64k for T11 code
	ROM_LOAD16_BYTE( "136042-330.7l",   0x008000, 0x004000, CRC(ee312027) SHA1(7caeaf6220022ffffc7d1feefec24163bed70275) )
	ROM_LOAD16_BYTE( "136042-331.7n",   0x008001, 0x004000, CRC(2ef15354) SHA1(c8044bd2e435bdd423877b78f375f13588d1dfd1) )
	ROM_LOAD16_BYTE( "136042-329.6f",   0x010000, 0x008000, CRC(ed1d6205) SHA1(7b2b2fd5eb12b1b6266d2becb96c8cf23cdaed26) )
	ROM_LOAD16_BYTE( "136042-325.6n",   0x010001, 0x008000, CRC(aecaa2bf) SHA1(919469912bb9e764f60ebf1e231f951a41b370ff) )
	ROM_LOAD16_BYTE( "136042-127.6k",   0x050000, 0x008000, CRC(de6c4db9) SHA1(bda7a9628e1ae4bf3fdd67abb8aaa43e9bd8b72b) )
	ROM_LOAD16_BYTE( "136042-123.6r",   0x050001, 0x008000, CRC(aff23b5a) SHA1(20bedf8efc927ddd4d513e570bbbec7c8e849a29) )
	ROM_LOAD16_BYTE( "136042-126.6l",   0x070000, 0x008000, CRC(92f5392c) SHA1(064ccf24a68440caa565c0467ba4bf4246133698) )
	ROM_LOAD16_BYTE( "136042-122.6s",   0x070001, 0x008000, CRC(0381f362) SHA1(e33b6d4949cdee33f27cedf00ef20f1ce5011e24) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    // 64k for 6502 code
	ROM_LOAD( "136042-419.2bc",  0x008000, 0x004000, CRC(b277915a) SHA1(e0e8cd713950f45352b7c1de986b5b0b5c1703b3) )
	ROM_LOAD( "136042-420.2d",   0x00c000, 0x004000, CRC(170b2c53) SHA1(c6d5657da29cf637cea940406fcff9a7328964f8) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "136042-105.6a",   0x020000, 0x008000, CRC(911499fe) SHA1(d53139b3a94c4e3d2c0be9cd4be97256de6b2386) )
	ROM_CONTINUE(                0x000000, 0x008000 )
	ROM_LOAD( "136042-106.6b",   0x008000, 0x008000, CRC(a39b25ed) SHA1(d91b3e5a9d1b0ff56cb8e012c349b7c3d8a9b91d) )
	ROM_LOAD( "136042-101.7a",   0x030000, 0x008000, CRC(6d015c72) SHA1(0f8ada9cb65f13c88efffc5174d14a1babff699b) )
	ROM_CONTINUE(                0x010000, 0x008000 )
	ROM_LOAD( "136042-102.7b",   0x018000, 0x008000, CRC(54e21f0a) SHA1(d7e288b1d25e77afc435690f6c7362b8e256f72e) )
	ROM_LOAD( "136042-107.6c",   0x060000, 0x008000, CRC(b7ded658) SHA1(8827b18c577ba256822a567a034e03fc4426f34a) )
	ROM_CONTINUE(                0x040000, 0x008000 )
	ROM_LOAD( "136042-108.6de",  0x048000, 0x008000, CRC(4a804a4c) SHA1(e90354c0b6d9d7c23d86b590a30bdc8e8fcf144a) )
	ROM_LOAD( "136042-104.7de",  0x070000, 0x008000, CRC(339644ed) SHA1(3fbd7bed4838f01f9d3588f02719b6fe13ac5435) )
	ROM_CONTINUE(                0x050000, 0x008000 )
	ROM_LOAD( "136042-103.7c",   0x058000, 0x008000, CRC(64d473a8) SHA1(12f4aa981d1ab2f9b5bffeda2d02fcf6b497f1e9) )

	ROM_REGION( 0x40000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "136042-113.6l",   0x000000, 0x008000, CRC(f869b0fc) SHA1(d8bf0df492bbe5d228748be0976d8f68254ffb3f) )
	ROM_LOAD( "136042-112.6k",   0x008000, 0x008000, CRC(abcbc114) SHA1(5d11fdfd7ed968949d884f1245571aed052abe46) )
	ROM_LOAD( "136042-110.6jh",  0x010000, 0x008000, CRC(9e91e734) SHA1(d9424a46dec2885fdc1ebe973f7deebfcb6ff19e) )
	ROM_LOAD( "136042-109.6fh",  0x018000, 0x008000, CRC(3a051f36) SHA1(1f74d53e1950447fd79a552d934038c4661124cf) )
	ROM_LOAD( "136042-117.6rs",  0x020000, 0x008000, CRC(b15c1b90) SHA1(a3d517919fe267f30383cff4142dd0d6e1102492) )
	ROM_LOAD( "136042-116.6pr",  0x028000, 0x008000, CRC(1dcdd5aa) SHA1(99eb230597132f04aa7d4d66907a14596f780365) )
	ROM_LOAD( "136042-115.6n",   0x030000, 0x008000, CRC(fb5677d9) SHA1(fa3190af7eaf0ef738fd86a8280c1c0b0556415f) )
	ROM_LOAD( "136042-114.6m",   0x038000, 0x008000, CRC(35e70a8d) SHA1(953bb609143f2660e5e5ca980691b962c1e7a81e) )

	ROM_REGION( 0x4000, "chars", 0 )
	ROM_LOAD( "136042-118.6t",   0x000000, 0x004000, CRC(8489d113) SHA1(f8ead7954d9be95792fd7e9d2487957d1e194641) )

	ROM_REGION( 0x200, "eeprom", 0 )
	ROM_LOAD( "ssprint-eeprom.bin", 0x0000, 0x0200, CRC(9301ed27) SHA1(5edd9688ce36520ab79e1388d489b72525a686ff) )
ROM_END


ROM_START( ssprints )
	ROM_REGION( 0x90000, "maincpu", 0 ) // 9*64k for T11 code
	ROM_LOAD16_BYTE( "136042-138.7l",   0x008000, 0x004000, CRC(234a7c65) SHA1(2686cb83f339e20b7168ebf22f97d11511815859) )
	ROM_LOAD16_BYTE( "136042-139.7n",   0x008001, 0x004000, CRC(7652a461) SHA1(9afe5b1d8ad16906b9927e8ca7e1ce81f86352d2) )
	ROM_LOAD16_BYTE( "136042-137.6f",   0x010000, 0x008000, CRC(fa4c7e9d) SHA1(88eedd7c24da591f75525d0229ff91fac8c2d4ad) )
	ROM_LOAD16_BYTE( "136042-136.6n",   0x010001, 0x008000, CRC(7c20a249) SHA1(fda8011096ae0e6b525637fabb4f3616ec2145e7) )
	ROM_LOAD16_BYTE( "136042-127.6k",   0x050000, 0x008000, CRC(de6c4db9) SHA1(bda7a9628e1ae4bf3fdd67abb8aaa43e9bd8b72b) )
	ROM_LOAD16_BYTE( "136042-123.6r",   0x050001, 0x008000, CRC(aff23b5a) SHA1(20bedf8efc927ddd4d513e570bbbec7c8e849a29) )
	ROM_LOAD16_BYTE( "136042-126.6l",   0x070000, 0x008000, CRC(92f5392c) SHA1(064ccf24a68440caa565c0467ba4bf4246133698) )
	ROM_LOAD16_BYTE( "136042-122.6s",   0x070001, 0x008000, CRC(0381f362) SHA1(e33b6d4949cdee33f27cedf00ef20f1ce5011e24) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    // 64k for 6502 code
	ROM_LOAD( "136042-119.2bc",  0x008000, 0x004000, CRC(0c810231) SHA1(a5a637e12df7eae234fdc2d3957d122c196c65cd) )
	ROM_LOAD( "136042-120.2d",   0x00c000, 0x004000, CRC(647b7481) SHA1(51b1b09919eee3d98e65d48e3a2af8321ccf8a02) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "136042-105.6a",   0x020000, 0x008000, CRC(911499fe) SHA1(d53139b3a94c4e3d2c0be9cd4be97256de6b2386) )
	ROM_CONTINUE(                0x000000, 0x008000 )
	ROM_LOAD( "136042-106.6b",   0x008000, 0x008000, CRC(a39b25ed) SHA1(d91b3e5a9d1b0ff56cb8e012c349b7c3d8a9b91d) )
	ROM_LOAD( "136042-101.7a",   0x030000, 0x008000, CRC(6d015c72) SHA1(0f8ada9cb65f13c88efffc5174d14a1babff699b) )
	ROM_CONTINUE(                0x010000, 0x008000 )
	ROM_LOAD( "136042-102.7b",   0x018000, 0x008000, CRC(54e21f0a) SHA1(d7e288b1d25e77afc435690f6c7362b8e256f72e) )
	ROM_LOAD( "136042-107.6c",   0x060000, 0x008000, CRC(b7ded658) SHA1(8827b18c577ba256822a567a034e03fc4426f34a) )
	ROM_CONTINUE(                0x040000, 0x008000 )
	ROM_LOAD( "136042-108.6de",  0x048000, 0x008000, CRC(4a804a4c) SHA1(e90354c0b6d9d7c23d86b590a30bdc8e8fcf144a) )
	ROM_LOAD( "136042-104.7de",  0x070000, 0x008000, CRC(339644ed) SHA1(3fbd7bed4838f01f9d3588f02719b6fe13ac5435) )
	ROM_CONTINUE(                0x050000, 0x008000 )
	ROM_LOAD( "136042-103.7c",   0x058000, 0x008000, CRC(64d473a8) SHA1(12f4aa981d1ab2f9b5bffeda2d02fcf6b497f1e9) )

	ROM_REGION( 0x40000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "136042-113.6l",   0x000000, 0x008000, CRC(f869b0fc) SHA1(d8bf0df492bbe5d228748be0976d8f68254ffb3f) )
	ROM_LOAD( "136042-112.6k",   0x008000, 0x008000, CRC(abcbc114) SHA1(5d11fdfd7ed968949d884f1245571aed052abe46) )
	ROM_LOAD( "136042-110.6jh",  0x010000, 0x008000, CRC(9e91e734) SHA1(d9424a46dec2885fdc1ebe973f7deebfcb6ff19e) )
	ROM_LOAD( "136042-109.6fh",  0x018000, 0x008000, CRC(3a051f36) SHA1(1f74d53e1950447fd79a552d934038c4661124cf) )
	ROM_LOAD( "136042-117.6rs",  0x020000, 0x008000, CRC(b15c1b90) SHA1(a3d517919fe267f30383cff4142dd0d6e1102492) )
	ROM_LOAD( "136042-116.6pr",  0x028000, 0x008000, CRC(1dcdd5aa) SHA1(99eb230597132f04aa7d4d66907a14596f780365) )
	ROM_LOAD( "136042-115.6n",   0x030000, 0x008000, CRC(fb5677d9) SHA1(fa3190af7eaf0ef738fd86a8280c1c0b0556415f) )
	ROM_LOAD( "136042-114.6m",   0x038000, 0x008000, CRC(35e70a8d) SHA1(953bb609143f2660e5e5ca980691b962c1e7a81e) )

	ROM_REGION( 0x4000, "chars", 0 )
	ROM_LOAD( "136042-218.6t",   0x000000, 0x004000, CRC(8e500be1) SHA1(f21799bf97c8bf82328999cb912ad5f293035d55) )

	ROM_REGION( 0x200, "eeprom", 0 )
	ROM_LOAD( "ssprint1-eeprom.bin", 0x0000, 0x0200, CRC(ed263888) SHA1(8e0545853823b2c0a820361a14acd9e3cb407173) )
ROM_END


ROM_START( ssprintf )
	ROM_REGION( 0x90000, "maincpu", 0 ) // 9*64k for T11 code
	ROM_LOAD16_BYTE( "136042-134.7l",   0x008000, 0x004000, CRC(b7757b44) SHA1(4d38addb68cb272e5cb9cfbfeb7c3a5aeb21ad26) )
	ROM_LOAD16_BYTE( "136042-135.7n",   0x008001, 0x004000, CRC(4fc132ba) SHA1(6724c59d4942bb9196918f1f83bac7bb07099076) )
	ROM_LOAD16_BYTE( "136042-133.6f",   0x010000, 0x008000, CRC(0b9f89da) SHA1(025650687af247f4bb7d070d69073cf7afbf9a27) )
	ROM_LOAD16_BYTE( "136042-132.6n",   0x010001, 0x008000, CRC(fe02509d) SHA1(6e18876cb6685dad431e05d5e240222bf78b0f0e) )
	ROM_LOAD16_BYTE( "136042-127.6k",   0x050000, 0x008000, CRC(de6c4db9) SHA1(bda7a9628e1ae4bf3fdd67abb8aaa43e9bd8b72b) )
	ROM_LOAD16_BYTE( "136042-123.6r",   0x050001, 0x008000, CRC(aff23b5a) SHA1(20bedf8efc927ddd4d513e570bbbec7c8e849a29) )
	ROM_LOAD16_BYTE( "136042-126.6l",   0x070000, 0x008000, CRC(92f5392c) SHA1(064ccf24a68440caa565c0467ba4bf4246133698) )
	ROM_LOAD16_BYTE( "136042-122.6s",   0x070001, 0x008000, CRC(0381f362) SHA1(e33b6d4949cdee33f27cedf00ef20f1ce5011e24) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    // 64k for 6502 code
	ROM_LOAD( "136042-119.2bc",  0x008000, 0x004000, CRC(0c810231) SHA1(a5a637e12df7eae234fdc2d3957d122c196c65cd) )
	ROM_LOAD( "136042-120.2d",   0x00c000, 0x004000, CRC(647b7481) SHA1(51b1b09919eee3d98e65d48e3a2af8321ccf8a02) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "136042-105.6a",   0x020000, 0x008000, CRC(911499fe) SHA1(d53139b3a94c4e3d2c0be9cd4be97256de6b2386) )
	ROM_CONTINUE(                0x000000, 0x008000 )
	ROM_LOAD( "136042-106.6b",   0x008000, 0x008000, CRC(a39b25ed) SHA1(d91b3e5a9d1b0ff56cb8e012c349b7c3d8a9b91d) )
	ROM_LOAD( "136042-101.7a",   0x030000, 0x008000, CRC(6d015c72) SHA1(0f8ada9cb65f13c88efffc5174d14a1babff699b) )
	ROM_CONTINUE(                0x010000, 0x008000 )
	ROM_LOAD( "136042-102.7b",   0x018000, 0x008000, CRC(54e21f0a) SHA1(d7e288b1d25e77afc435690f6c7362b8e256f72e) )
	ROM_LOAD( "136042-107.6c",   0x060000, 0x008000, CRC(b7ded658) SHA1(8827b18c577ba256822a567a034e03fc4426f34a) )
	ROM_CONTINUE(                0x040000, 0x008000 )
	ROM_LOAD( "136042-108.6de",  0x048000, 0x008000, CRC(4a804a4c) SHA1(e90354c0b6d9d7c23d86b590a30bdc8e8fcf144a) )
	ROM_LOAD( "136042-104.7de",  0x070000, 0x008000, CRC(339644ed) SHA1(3fbd7bed4838f01f9d3588f02719b6fe13ac5435) )
	ROM_CONTINUE(                0x050000, 0x008000 )
	ROM_LOAD( "136042-103.7c",   0x058000, 0x008000, CRC(64d473a8) SHA1(12f4aa981d1ab2f9b5bffeda2d02fcf6b497f1e9) )

	ROM_REGION( 0x40000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "136042-113.6l",   0x000000, 0x008000, CRC(f869b0fc) SHA1(d8bf0df492bbe5d228748be0976d8f68254ffb3f) )
	ROM_LOAD( "136042-112.6k",   0x008000, 0x008000, CRC(abcbc114) SHA1(5d11fdfd7ed968949d884f1245571aed052abe46) )
	ROM_LOAD( "136042-110.6jh",  0x010000, 0x008000, CRC(9e91e734) SHA1(d9424a46dec2885fdc1ebe973f7deebfcb6ff19e) )
	ROM_LOAD( "136042-109.6fh",  0x018000, 0x008000, CRC(3a051f36) SHA1(1f74d53e1950447fd79a552d934038c4661124cf) )
	ROM_LOAD( "136042-117.6rs",  0x020000, 0x008000, CRC(b15c1b90) SHA1(a3d517919fe267f30383cff4142dd0d6e1102492) )
	ROM_LOAD( "136042-116.6pr",  0x028000, 0x008000, CRC(1dcdd5aa) SHA1(99eb230597132f04aa7d4d66907a14596f780365) )
	ROM_LOAD( "136042-115.6n",   0x030000, 0x008000, CRC(fb5677d9) SHA1(fa3190af7eaf0ef738fd86a8280c1c0b0556415f) )
	ROM_LOAD( "136042-114.6m",   0x038000, 0x008000, CRC(35e70a8d) SHA1(953bb609143f2660e5e5ca980691b962c1e7a81e) )

	ROM_REGION( 0x4000, "chars", 0 )
	ROM_LOAD( "136042-218.6t",   0x000000, 0x004000, CRC(8e500be1) SHA1(f21799bf97c8bf82328999cb912ad5f293035d55) )

	ROM_REGION( 0x200, "eeprom", 0 )
	ROM_LOAD( "ssprint1-eeprom.bin", 0x0000, 0x0200, CRC(ed263888) SHA1(8e0545853823b2c0a820361a14acd9e3cb407173) )
ROM_END


ROM_START( ssprintg )
	ROM_REGION( 0x90000, "maincpu", 0 ) // 9*64k for T11 code
	ROM_LOAD16_BYTE( "136042-430.7l",   0x008000, 0x004000, CRC(c21df5f5) SHA1(0b79cfa0a6f3c1c59d09ff6a741abc71dc3da240) )
	ROM_LOAD16_BYTE( "136042-431.7n",   0x008001, 0x004000, CRC(5880fc58) SHA1(c4e6c48d99b903f80408f0ee81672ff259f131ae) )
	ROM_LOAD16_BYTE( "136042-429.6f",   0x010000, 0x008000, CRC(2060f68a) SHA1(b435a6de3e5ea5c1b5ba14f755660f747d972c38) )
	ROM_LOAD16_BYTE( "136042-425.6n",   0x010001, 0x008000, CRC(b7274985) SHA1(03fde30d153906fb1f9d33bf8dd5c472052e62ae) )
	ROM_LOAD16_BYTE( "136042-127.6k",   0x050000, 0x008000, CRC(de6c4db9) SHA1(bda7a9628e1ae4bf3fdd67abb8aaa43e9bd8b72b) )
	ROM_LOAD16_BYTE( "136042-123.6r",   0x050001, 0x008000, CRC(aff23b5a) SHA1(20bedf8efc927ddd4d513e570bbbec7c8e849a29) )
	ROM_LOAD16_BYTE( "136042-126.6l",   0x070000, 0x008000, CRC(92f5392c) SHA1(064ccf24a68440caa565c0467ba4bf4246133698) )
	ROM_LOAD16_BYTE( "136042-122.6s",   0x070001, 0x008000, CRC(0381f362) SHA1(e33b6d4949cdee33f27cedf00ef20f1ce5011e24) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    // 64k for 6502 code
	ROM_LOAD( "136042-119.2bc",  0x008000, 0x004000, CRC(0c810231) SHA1(a5a637e12df7eae234fdc2d3957d122c196c65cd) )
	ROM_LOAD( "136042-120.2d",   0x00c000, 0x004000, CRC(647b7481) SHA1(51b1b09919eee3d98e65d48e3a2af8321ccf8a02) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "136042-105.6a",   0x020000, 0x008000, CRC(911499fe) SHA1(d53139b3a94c4e3d2c0be9cd4be97256de6b2386) )
	ROM_CONTINUE(                0x000000, 0x008000 )
	ROM_LOAD( "136042-106.6b",   0x008000, 0x008000, CRC(a39b25ed) SHA1(d91b3e5a9d1b0ff56cb8e012c349b7c3d8a9b91d) )
	ROM_LOAD( "136042-101.7a",   0x030000, 0x008000, CRC(6d015c72) SHA1(0f8ada9cb65f13c88efffc5174d14a1babff699b) )
	ROM_CONTINUE(                0x010000, 0x008000 )
	ROM_LOAD( "136042-102.7b",   0x018000, 0x008000, CRC(54e21f0a) SHA1(d7e288b1d25e77afc435690f6c7362b8e256f72e) )
	ROM_LOAD( "136042-107.6c",   0x060000, 0x008000, CRC(b7ded658) SHA1(8827b18c577ba256822a567a034e03fc4426f34a) )
	ROM_CONTINUE(                0x040000, 0x008000 )
	ROM_LOAD( "136042-108.6de",  0x048000, 0x008000, CRC(4a804a4c) SHA1(e90354c0b6d9d7c23d86b590a30bdc8e8fcf144a) )
	ROM_LOAD( "136042-104.7de",  0x070000, 0x008000, CRC(339644ed) SHA1(3fbd7bed4838f01f9d3588f02719b6fe13ac5435) )
	ROM_CONTINUE(                0x050000, 0x008000 )
	ROM_LOAD( "136042-103.7c",   0x058000, 0x008000, CRC(64d473a8) SHA1(12f4aa981d1ab2f9b5bffeda2d02fcf6b497f1e9) )

	ROM_REGION( 0x40000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "136042-113.6l",   0x000000, 0x008000, CRC(f869b0fc) SHA1(d8bf0df492bbe5d228748be0976d8f68254ffb3f) )
	ROM_LOAD( "136042-112.6k",   0x008000, 0x008000, CRC(abcbc114) SHA1(5d11fdfd7ed968949d884f1245571aed052abe46) )
	ROM_LOAD( "136042-110.6jh",  0x010000, 0x008000, CRC(9e91e734) SHA1(d9424a46dec2885fdc1ebe973f7deebfcb6ff19e) )
	ROM_LOAD( "136042-109.6fh",  0x018000, 0x008000, CRC(3a051f36) SHA1(1f74d53e1950447fd79a552d934038c4661124cf) )
	ROM_LOAD( "136042-117.6rs",  0x020000, 0x008000, CRC(b15c1b90) SHA1(a3d517919fe267f30383cff4142dd0d6e1102492) )
	ROM_LOAD( "136042-116.6pr",  0x028000, 0x008000, CRC(1dcdd5aa) SHA1(99eb230597132f04aa7d4d66907a14596f780365) )
	ROM_LOAD( "136042-115.6n",   0x030000, 0x008000, CRC(fb5677d9) SHA1(fa3190af7eaf0ef738fd86a8280c1c0b0556415f) )
	ROM_LOAD( "136042-114.6m",   0x038000, 0x008000, CRC(35e70a8d) SHA1(953bb609143f2660e5e5ca980691b962c1e7a81e) )

	ROM_REGION( 0x4000, "chars", 0 )
	ROM_LOAD( "136042-118.6t",   0x000000, 0x004000, CRC(8489d113) SHA1(f8ead7954d9be95792fd7e9d2487957d1e194641) )

	ROM_REGION( 0x200, "eeprom", 0 )
	ROM_LOAD( "ssprint1-eeprom.bin", 0x0000, 0x0200, CRC(ed263888) SHA1(8e0545853823b2c0a820361a14acd9e3cb407173) )
ROM_END


ROM_START( ssprint3 )
	ROM_REGION( 0x90000, "maincpu", 0 ) // 9*64k for T11 code
	ROM_LOAD16_BYTE( "136042-330.7l",   0x008000, 0x004000, CRC(ee312027) SHA1(7caeaf6220022ffffc7d1feefec24163bed70275) )
	ROM_LOAD16_BYTE( "136042-331.7n",   0x008001, 0x004000, CRC(2ef15354) SHA1(c8044bd2e435bdd423877b78f375f13588d1dfd1) )
	ROM_LOAD16_BYTE( "136042-329.6f",   0x010000, 0x008000, CRC(ed1d6205) SHA1(7b2b2fd5eb12b1b6266d2becb96c8cf23cdaed26) )
	ROM_LOAD16_BYTE( "136042-325.6n",   0x010001, 0x008000, CRC(aecaa2bf) SHA1(919469912bb9e764f60ebf1e231f951a41b370ff) )
	ROM_LOAD16_BYTE( "136042-127.6k",   0x050000, 0x008000, CRC(de6c4db9) SHA1(bda7a9628e1ae4bf3fdd67abb8aaa43e9bd8b72b) )
	ROM_LOAD16_BYTE( "136042-123.6r",   0x050001, 0x008000, CRC(aff23b5a) SHA1(20bedf8efc927ddd4d513e570bbbec7c8e849a29) )
	ROM_LOAD16_BYTE( "136042-126.6l",   0x070000, 0x008000, CRC(92f5392c) SHA1(064ccf24a68440caa565c0467ba4bf4246133698) )
	ROM_LOAD16_BYTE( "136042-122.6s",   0x070001, 0x008000, CRC(0381f362) SHA1(e33b6d4949cdee33f27cedf00ef20f1ce5011e24) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    // 64k for 6502 code
	ROM_LOAD( "136042-319.2bc",  0x008000, 0x004000, CRC(c7f31c16) SHA1(cfacf22405da5e3cf95059ea6b9677a5a8471496) )
	ROM_LOAD( "136042-320.2d",   0x00c000, 0x004000, CRC(9815ece9) SHA1(95239e15fe3e3f9a66e0f4dae365f763656cb70b) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "136042-105.6a",   0x020000, 0x008000, CRC(911499fe) SHA1(d53139b3a94c4e3d2c0be9cd4be97256de6b2386) )
	ROM_CONTINUE(                0x000000, 0x008000 )
	ROM_LOAD( "136042-106.6b",   0x008000, 0x008000, CRC(a39b25ed) SHA1(d91b3e5a9d1b0ff56cb8e012c349b7c3d8a9b91d) )
	ROM_LOAD( "136042-101.7a",   0x030000, 0x008000, CRC(6d015c72) SHA1(0f8ada9cb65f13c88efffc5174d14a1babff699b) )
	ROM_CONTINUE(                0x010000, 0x008000 )
	ROM_LOAD( "136042-102.7b",   0x018000, 0x008000, CRC(54e21f0a) SHA1(d7e288b1d25e77afc435690f6c7362b8e256f72e) )
	ROM_LOAD( "136042-107.6c",   0x060000, 0x008000, CRC(b7ded658) SHA1(8827b18c577ba256822a567a034e03fc4426f34a) )
	ROM_CONTINUE(                0x040000, 0x008000 )
	ROM_LOAD( "136042-108.6de",  0x048000, 0x008000, CRC(4a804a4c) SHA1(e90354c0b6d9d7c23d86b590a30bdc8e8fcf144a) )
	ROM_LOAD( "136042-104.7de",  0x070000, 0x008000, CRC(339644ed) SHA1(3fbd7bed4838f01f9d3588f02719b6fe13ac5435) )
	ROM_CONTINUE(                0x050000, 0x008000 )
	ROM_LOAD( "136042-103.7c",   0x058000, 0x008000, CRC(64d473a8) SHA1(12f4aa981d1ab2f9b5bffeda2d02fcf6b497f1e9) )

	ROM_REGION( 0x40000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "136042-113.6l",   0x000000, 0x008000, CRC(f869b0fc) SHA1(d8bf0df492bbe5d228748be0976d8f68254ffb3f) )
	ROM_LOAD( "136042-112.6k",   0x008000, 0x008000, CRC(abcbc114) SHA1(5d11fdfd7ed968949d884f1245571aed052abe46) )
	ROM_LOAD( "136042-110.6jh",  0x010000, 0x008000, CRC(9e91e734) SHA1(d9424a46dec2885fdc1ebe973f7deebfcb6ff19e) )
	ROM_LOAD( "136042-109.6fh",  0x018000, 0x008000, CRC(3a051f36) SHA1(1f74d53e1950447fd79a552d934038c4661124cf) )
	ROM_LOAD( "136042-117.6rs",  0x020000, 0x008000, CRC(b15c1b90) SHA1(a3d517919fe267f30383cff4142dd0d6e1102492) )
	ROM_LOAD( "136042-116.6pr",  0x028000, 0x008000, CRC(1dcdd5aa) SHA1(99eb230597132f04aa7d4d66907a14596f780365) )
	ROM_LOAD( "136042-115.6n",   0x030000, 0x008000, CRC(fb5677d9) SHA1(fa3190af7eaf0ef738fd86a8280c1c0b0556415f) )
	ROM_LOAD( "136042-114.6m",   0x038000, 0x008000, CRC(35e70a8d) SHA1(953bb609143f2660e5e5ca980691b962c1e7a81e) )

	ROM_REGION( 0x4000, "chars", 0 )
	ROM_LOAD( "136042-118.6t",   0x000000, 0x004000, CRC(8489d113) SHA1(f8ead7954d9be95792fd7e9d2487957d1e194641) )

	ROM_REGION( 0x200, "eeprom", 0 )
	ROM_LOAD( "ssprint1-eeprom.bin", 0x0000, 0x0200, CRC(ed263888) SHA1(8e0545853823b2c0a820361a14acd9e3cb407173) )
ROM_END


ROM_START( ssprintg1 )
	ROM_REGION( 0x90000, "maincpu", 0 ) // 9*64k for T11 code
	ROM_LOAD16_BYTE( "136042-230.7l",   0x008000, 0x004000, CRC(e5b2da29) SHA1(99150184a3f065e934ed6f60731fe534a75ba991) )
	ROM_LOAD16_BYTE( "136042-231.7n",   0x008001, 0x004000, CRC(fac14b00) SHA1(6e5bf1e80f3d04f670b8290195609c0ac0cacea2) )
	ROM_LOAD16_BYTE( "136042-229.6f",   0x010000, 0x008000, CRC(78b01070) SHA1(ccfa6bd1068e7bd3524a7da93a901633256f0524) )
	ROM_LOAD16_BYTE( "136042-225.6n",   0x010001, 0x008000, CRC(03688b4c) SHA1(d76de1cc2d827bdc57956a2d7a9cd03e1906483f) )
	ROM_LOAD16_BYTE( "136042-127.6k",   0x050000, 0x008000, CRC(de6c4db9) SHA1(bda7a9628e1ae4bf3fdd67abb8aaa43e9bd8b72b) )
	ROM_LOAD16_BYTE( "136042-123.6r",   0x050001, 0x008000, CRC(aff23b5a) SHA1(20bedf8efc927ddd4d513e570bbbec7c8e849a29) )
	ROM_LOAD16_BYTE( "136042-126.6l",   0x070000, 0x008000, CRC(92f5392c) SHA1(064ccf24a68440caa565c0467ba4bf4246133698) )
	ROM_LOAD16_BYTE( "136042-122.6s",   0x070001, 0x008000, CRC(0381f362) SHA1(e33b6d4949cdee33f27cedf00ef20f1ce5011e24) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    // 64k for 6502 code
	ROM_LOAD( "136042-119.2bc",  0x008000, 0x004000, CRC(0c810231) SHA1(a5a637e12df7eae234fdc2d3957d122c196c65cd) )
	ROM_LOAD( "136042-120.2d",   0x00c000, 0x004000, CRC(647b7481) SHA1(51b1b09919eee3d98e65d48e3a2af8321ccf8a02) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "136042-105.6a",   0x020000, 0x008000, CRC(911499fe) SHA1(d53139b3a94c4e3d2c0be9cd4be97256de6b2386) )
	ROM_CONTINUE(                0x000000, 0x008000 )
	ROM_LOAD( "136042-106.6b",   0x008000, 0x008000, CRC(a39b25ed) SHA1(d91b3e5a9d1b0ff56cb8e012c349b7c3d8a9b91d) )
	ROM_LOAD( "136042-101.7a",   0x030000, 0x008000, CRC(6d015c72) SHA1(0f8ada9cb65f13c88efffc5174d14a1babff699b) )
	ROM_CONTINUE(                0x010000, 0x008000 )
	ROM_LOAD( "136042-102.7b",   0x018000, 0x008000, CRC(54e21f0a) SHA1(d7e288b1d25e77afc435690f6c7362b8e256f72e) )
	ROM_LOAD( "136042-107.6c",   0x060000, 0x008000, CRC(b7ded658) SHA1(8827b18c577ba256822a567a034e03fc4426f34a) )
	ROM_CONTINUE(                0x040000, 0x008000 )
	ROM_LOAD( "136042-108.6de",  0x048000, 0x008000, CRC(4a804a4c) SHA1(e90354c0b6d9d7c23d86b590a30bdc8e8fcf144a) )
	ROM_LOAD( "136042-104.7de",  0x070000, 0x008000, CRC(339644ed) SHA1(3fbd7bed4838f01f9d3588f02719b6fe13ac5435) )
	ROM_CONTINUE(                0x050000, 0x008000 )
	ROM_LOAD( "136042-103.7c",   0x058000, 0x008000, CRC(64d473a8) SHA1(12f4aa981d1ab2f9b5bffeda2d02fcf6b497f1e9) )

	ROM_REGION( 0x40000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "136042-113.6l",   0x000000, 0x008000, CRC(f869b0fc) SHA1(d8bf0df492bbe5d228748be0976d8f68254ffb3f) )
	ROM_LOAD( "136042-112.6k",   0x008000, 0x008000, CRC(abcbc114) SHA1(5d11fdfd7ed968949d884f1245571aed052abe46) )
	ROM_LOAD( "136042-110.6jh",  0x010000, 0x008000, CRC(9e91e734) SHA1(d9424a46dec2885fdc1ebe973f7deebfcb6ff19e) )
	ROM_LOAD( "136042-109.6fh",  0x018000, 0x008000, CRC(3a051f36) SHA1(1f74d53e1950447fd79a552d934038c4661124cf) )
	ROM_LOAD( "136042-117.6rs",  0x020000, 0x008000, CRC(b15c1b90) SHA1(a3d517919fe267f30383cff4142dd0d6e1102492) )
	ROM_LOAD( "136042-116.6pr",  0x028000, 0x008000, CRC(1dcdd5aa) SHA1(99eb230597132f04aa7d4d66907a14596f780365) )
	ROM_LOAD( "136042-115.6n",   0x030000, 0x008000, CRC(fb5677d9) SHA1(fa3190af7eaf0ef738fd86a8280c1c0b0556415f) )
	ROM_LOAD( "136042-114.6m",   0x038000, 0x008000, CRC(35e70a8d) SHA1(953bb609143f2660e5e5ca980691b962c1e7a81e) )

	ROM_REGION( 0x4000, "chars", 0 )
	ROM_LOAD( "136042-118.6t",   0x000000, 0x004000, CRC(8489d113) SHA1(f8ead7954d9be95792fd7e9d2487957d1e194641) )

	ROM_REGION( 0x200, "eeprom", 0 )
	ROM_LOAD( "ssprint1-eeprom.bin", 0x0000, 0x0200, CRC(ed263888) SHA1(8e0545853823b2c0a820361a14acd9e3cb407173) )
ROM_END


ROM_START( ssprint1 )
	ROM_REGION( 0x90000, "maincpu", 0 ) // 9*64k for T11 code
	ROM_LOAD16_BYTE( "136042-130.7l",   0x008000, 0x004000, CRC(b1edc688) SHA1(2b5c4a20e54fda43b49e5f811ed144675f8e019b) )
	ROM_LOAD16_BYTE( "136042-131.7n",   0x008001, 0x004000, CRC(df49dc5a) SHA1(7cdd54cbfd0dc0428394047a057892e7f7d17b50) )
	ROM_LOAD16_BYTE( "136042-129.6f",   0x010000, 0x008000, CRC(8be22fca) SHA1(d663ef2e71bafbda5351d73e0b9a86bbfa66e225) )
	ROM_LOAD16_BYTE( "136042-125.6n",   0x010001, 0x008000, CRC(30b9e101) SHA1(c0fb167ab4e889963f7538c3fbada005b0aa80a4) )
	ROM_LOAD16_BYTE( "136042-127.6k",   0x050000, 0x008000, CRC(de6c4db9) SHA1(bda7a9628e1ae4bf3fdd67abb8aaa43e9bd8b72b) )
	ROM_LOAD16_BYTE( "136042-123.6r",   0x050001, 0x008000, CRC(aff23b5a) SHA1(20bedf8efc927ddd4d513e570bbbec7c8e849a29) )
	ROM_LOAD16_BYTE( "136042-126.6l",   0x070000, 0x008000, CRC(92f5392c) SHA1(064ccf24a68440caa565c0467ba4bf4246133698) )
	ROM_LOAD16_BYTE( "136042-122.6s",   0x070001, 0x008000, CRC(0381f362) SHA1(e33b6d4949cdee33f27cedf00ef20f1ce5011e24) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    // 64k for 6502 code
	ROM_LOAD( "136042-119.2bc",  0x008000, 0x004000, CRC(0c810231) SHA1(a5a637e12df7eae234fdc2d3957d122c196c65cd) )
	ROM_LOAD( "136042-120.2d",   0x00c000, 0x004000, CRC(647b7481) SHA1(51b1b09919eee3d98e65d48e3a2af8321ccf8a02) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "136042-105.6a",   0x020000, 0x008000, CRC(911499fe) SHA1(d53139b3a94c4e3d2c0be9cd4be97256de6b2386) )
	ROM_CONTINUE(                0x000000, 0x008000 )
	ROM_LOAD( "136042-106.6b",   0x008000, 0x008000, CRC(a39b25ed) SHA1(d91b3e5a9d1b0ff56cb8e012c349b7c3d8a9b91d) )
	ROM_LOAD( "136042-101.7a",   0x030000, 0x008000, CRC(6d015c72) SHA1(0f8ada9cb65f13c88efffc5174d14a1babff699b) )
	ROM_CONTINUE(                0x010000, 0x008000 )
	ROM_LOAD( "136042-102.7b",   0x018000, 0x008000, CRC(54e21f0a) SHA1(d7e288b1d25e77afc435690f6c7362b8e256f72e) )
	ROM_LOAD( "136042-107.6c",   0x060000, 0x008000, CRC(b7ded658) SHA1(8827b18c577ba256822a567a034e03fc4426f34a) )
	ROM_CONTINUE(                0x040000, 0x008000 )
	ROM_LOAD( "136042-108.6de",  0x048000, 0x008000, CRC(4a804a4c) SHA1(e90354c0b6d9d7c23d86b590a30bdc8e8fcf144a) )
	ROM_LOAD( "136042-104.7de",  0x070000, 0x008000, CRC(339644ed) SHA1(3fbd7bed4838f01f9d3588f02719b6fe13ac5435) )
	ROM_CONTINUE(                0x050000, 0x008000 )
	ROM_LOAD( "136042-103.7c",   0x058000, 0x008000, CRC(64d473a8) SHA1(12f4aa981d1ab2f9b5bffeda2d02fcf6b497f1e9) )

	ROM_REGION( 0x40000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "136042-113.6l",   0x000000, 0x008000, CRC(f869b0fc) SHA1(d8bf0df492bbe5d228748be0976d8f68254ffb3f) )
	ROM_LOAD( "136042-112.6k",   0x008000, 0x008000, CRC(abcbc114) SHA1(5d11fdfd7ed968949d884f1245571aed052abe46) )
	ROM_LOAD( "136042-110.6jh",  0x010000, 0x008000, CRC(9e91e734) SHA1(d9424a46dec2885fdc1ebe973f7deebfcb6ff19e) )
	ROM_LOAD( "136042-109.6fh",  0x018000, 0x008000, CRC(3a051f36) SHA1(1f74d53e1950447fd79a552d934038c4661124cf) )
	ROM_LOAD( "136042-117.6rs",  0x020000, 0x008000, CRC(b15c1b90) SHA1(a3d517919fe267f30383cff4142dd0d6e1102492) )
	ROM_LOAD( "136042-116.6pr",  0x028000, 0x008000, CRC(1dcdd5aa) SHA1(99eb230597132f04aa7d4d66907a14596f780365) )
	ROM_LOAD( "136042-115.6n",   0x030000, 0x008000, CRC(fb5677d9) SHA1(fa3190af7eaf0ef738fd86a8280c1c0b0556415f) )
	ROM_LOAD( "136042-114.6m",   0x038000, 0x008000, CRC(35e70a8d) SHA1(953bb609143f2660e5e5ca980691b962c1e7a81e) )

	ROM_REGION( 0x4000, "chars", 0 )
	ROM_LOAD( "136042-118.6t",   0x000000, 0x004000, CRC(8489d113) SHA1(f8ead7954d9be95792fd7e9d2487957d1e194641) )

	ROM_REGION( 0x200, "eeprom", 0 )
	ROM_LOAD( "ssprint1-eeprom.bin", 0x0000, 0x0200, CRC(ed263888) SHA1(8e0545853823b2c0a820361a14acd9e3cb407173) )
ROM_END


ROM_START( csprints )
	ROM_REGION( 0x90000, "maincpu", 0 ) // 9*64k for T11 code
	ROM_LOAD16_BYTE( "136045-2326.7l",  0x008000, 0x004000, CRC(fd4ed0d3) SHA1(22d7a290c56975b8af82054b5fd8c9298f17f99a) )
	ROM_LOAD16_BYTE( "136045-2327.7n",  0x008001, 0x004000, CRC(5ef2a65a) SHA1(3ead2b91abca5ff95bffcd1fd40d3ff635d7801f) )
	ROM_LOAD16_BYTE( "136045-2325.6f",  0x010000, 0x008000, CRC(57253376) SHA1(100901de38f8561fc29d5b135b76b24755a4b1b2) )
	ROM_LOAD16_BYTE( "136045-2322.6n",  0x010001, 0x008000, CRC(b4265cae) SHA1(94d5c07b47442b513679bef7ade86b11c2c3ea8c) )
	ROM_LOAD16_BYTE( "136045-1124.6k",  0x050000, 0x008000, CRC(47efca1f) SHA1(ab2fb8479c8606d8f180657734d8d320815c5408) )
	ROM_LOAD16_BYTE( "136045-1121.6r",  0x050001, 0x008000, CRC(6ca404bb) SHA1(27ceda243410edcff0dc0aa08fb6466d0c6c80c7) )
	ROM_LOAD16_BYTE( "136045-1123.6l",  0x070000, 0x008000, CRC(0a4d216a) SHA1(53a4af7673c9dae1f6f2f13dce3c38a31ee12ee2) )
	ROM_LOAD16_BYTE( "136045-1120.6s",  0x070001, 0x008000, CRC(103f3fde) SHA1(9a0e82c3294369858b7a6c978143d8145a8df5a2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    // 64k for 6502 code
	ROM_LOAD( "136045-1118.2bc", 0x008000, 0x004000, CRC(eba41b2f) SHA1(a0d6e9f4609f2587b0fad6845e75653c10bf4249) )
	ROM_LOAD( "136045-1119.2d",  0x00c000, 0x004000, CRC(9e49043a) SHA1(ec467fe1cd59c51e43c3acd83d300f5b3309a47a) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "136045-1105.6a",  0x000000, 0x008000, CRC(3773bfbb) SHA1(beca29d8b9296d723304aab391dd9589a830a150) )
	ROM_LOAD( "136045-1106.6b",  0x008000, 0x008000, CRC(13a24886) SHA1(a1ac9ca04a60fdb38c6d81e451e5638bdb537c8f) )
	ROM_LOAD( "136045-1101.7a",  0x030000, 0x008000, CRC(5a55f931) SHA1(b2f8e8f3488a9d3840ca9a15fa53896ae4013e0a) )
	ROM_CONTINUE(                0x010000, 0x008000 )
	ROM_LOAD( "136045-1102.7b",  0x018000, 0x008000, CRC(37548a60) SHA1(f3395df534cdc75d4e1b2b415ee2cd6683e49204) )
	ROM_LOAD( "136045-1107.6c",  0x040000, 0x008000, CRC(e35e354e) SHA1(fa07737eb2ca19bf96ce15d8bbae7513c7dddd3c) )
	ROM_LOAD( "136045-1108.6de", 0x048000, 0x008000, CRC(361db8b7) SHA1(fb85e63c6e9122ab3d62eb7d0f5f715d8936910b) )
	ROM_LOAD( "136045-1104.7de", 0x070000, 0x008000, CRC(d1f8fe7b) SHA1(2612a397fdebe6062f66d26060c36af4a9ca28dc) )
	ROM_CONTINUE(                0x050000, 0x008000 )
	ROM_LOAD( "136045-1103.7c",  0x058000, 0x008000, CRC(8f8c9692) SHA1(57418c5dd3554055e5bce8c2a15f6f6fc3201d99) )

	ROM_REGION( 0x40000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "136045-1112.6l",  0x000000, 0x008000, CRC(f869b0fc) SHA1(d8bf0df492bbe5d228748be0976d8f68254ffb3f) )
	ROM_LOAD( "136045-1111.6k",  0x008000, 0x008000, CRC(abcbc114) SHA1(5d11fdfd7ed968949d884f1245571aed052abe46) )
	ROM_LOAD( "136045-1110.6hj", 0x010000, 0x008000, CRC(9e91e734) SHA1(d9424a46dec2885fdc1ebe973f7deebfcb6ff19e) )
	ROM_LOAD( "136045-1109.6fh", 0x018000, 0x008000, CRC(3a051f36) SHA1(1f74d53e1950447fd79a552d934038c4661124cf) )
	ROM_LOAD( "136045-1116.6rs", 0x020000, 0x008000, CRC(b15c1b90) SHA1(a3d517919fe267f30383cff4142dd0d6e1102492) )
	ROM_LOAD( "136045-1115.6pr", 0x028000, 0x008000, CRC(1dcdd5aa) SHA1(99eb230597132f04aa7d4d66907a14596f780365) )
	ROM_LOAD( "136045-1114.6n",  0x030000, 0x008000, CRC(fb5677d9) SHA1(fa3190af7eaf0ef738fd86a8280c1c0b0556415f) )
	ROM_LOAD( "136045-1113.6m",  0x038000, 0x008000, CRC(35e70a8d) SHA1(953bb609143f2660e5e5ca980691b962c1e7a81e) )

	ROM_REGION( 0x4000, "chars", 0 )
	ROM_LOAD( "136045-1117.6t",  0x000000, 0x004000, CRC(82da786d) SHA1(929cc4ebac3d4404e1a8b22b80aae975e0c9da85) )

	ROM_REGION( 0x200, "eeprom", 0 )
	ROM_LOAD( "csprint-eeprom.bin", 0x0000, 0x0200, CRC(ce1c7319) SHA1(a12926efd898cda2a05cf23fd9cd6674b9ffc702) )
ROM_END


ROM_START( csprint )
	ROM_REGION( 0x90000, "maincpu", 0 ) // 9*64k for T11 code
	ROM_LOAD16_BYTE( "136045-3126.7l",  0x008000, 0x004000, CRC(1dcf8b98) SHA1(6d83ea725a8448cd1fc31cdce2e24662db6b9bcf) )
	ROM_LOAD16_BYTE( "136045-2127.7n",  0x008001, 0x004000, CRC(bdcbe42c) SHA1(6dce564ce53f7171f8c713185cbf8b99a421ca41) )
	ROM_LOAD16_BYTE( "136045-2125.6f",  0x010000, 0x008000, CRC(76cc68b9) SHA1(651dbe8862afe2b7985a0a1cd1dabdbb1accc163) )
	ROM_LOAD16_BYTE( "136045-2122.6n",  0x010001, 0x008000, CRC(87dda6e5) SHA1(76a92d0ce9ce481661aa9235e4ce0d809b4e479d) )
	ROM_LOAD16_BYTE( "136045-1124.6k",  0x050000, 0x008000, CRC(47efca1f) SHA1(ab2fb8479c8606d8f180657734d8d320815c5408) )
	ROM_LOAD16_BYTE( "136045-1121.6r",  0x050001, 0x008000, CRC(6ca404bb) SHA1(27ceda243410edcff0dc0aa08fb6466d0c6c80c7) )
	ROM_LOAD16_BYTE( "136045-1123.6l",  0x070000, 0x008000, CRC(0a4d216a) SHA1(53a4af7673c9dae1f6f2f13dce3c38a31ee12ee2) )
	ROM_LOAD16_BYTE( "136045-1120.6s",  0x070001, 0x008000, CRC(103f3fde) SHA1(9a0e82c3294369858b7a6c978143d8145a8df5a2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    // 64k for 6502 code
	ROM_LOAD( "136045-1118.2bc", 0x008000, 0x004000, CRC(eba41b2f) SHA1(a0d6e9f4609f2587b0fad6845e75653c10bf4249) )
	ROM_LOAD( "136045-1119.2d",  0x00c000, 0x004000, CRC(9e49043a) SHA1(ec467fe1cd59c51e43c3acd83d300f5b3309a47a) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "136045-1105.6a",  0x000000, 0x008000, CRC(3773bfbb) SHA1(beca29d8b9296d723304aab391dd9589a830a150) )
	ROM_LOAD( "136045-1106.6b",  0x008000, 0x008000, CRC(13a24886) SHA1(a1ac9ca04a60fdb38c6d81e451e5638bdb537c8f) )
	ROM_LOAD( "136045-1101.7a",  0x030000, 0x008000, CRC(5a55f931) SHA1(b2f8e8f3488a9d3840ca9a15fa53896ae4013e0a) )
	ROM_CONTINUE(                0x010000, 0x008000 )
	ROM_LOAD( "136045-1102.7b",  0x018000, 0x008000, CRC(37548a60) SHA1(f3395df534cdc75d4e1b2b415ee2cd6683e49204) )
	ROM_LOAD( "136045-1107.6c",  0x040000, 0x008000, CRC(e35e354e) SHA1(fa07737eb2ca19bf96ce15d8bbae7513c7dddd3c) )
	ROM_LOAD( "136045-1108.6de", 0x048000, 0x008000, CRC(361db8b7) SHA1(fb85e63c6e9122ab3d62eb7d0f5f715d8936910b) )
	ROM_LOAD( "136045-1104.7de", 0x070000, 0x008000, CRC(d1f8fe7b) SHA1(2612a397fdebe6062f66d26060c36af4a9ca28dc) )
	ROM_CONTINUE(                0x050000, 0x008000 )
	ROM_LOAD( "136045-1103.7c",  0x058000, 0x008000, CRC(8f8c9692) SHA1(57418c5dd3554055e5bce8c2a15f6f6fc3201d99) )

	ROM_REGION( 0x40000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "136045-1112.6l",  0x000000, 0x008000, CRC(f869b0fc) SHA1(d8bf0df492bbe5d228748be0976d8f68254ffb3f) )
	ROM_LOAD( "136045-1111.6k",  0x008000, 0x008000, CRC(abcbc114) SHA1(5d11fdfd7ed968949d884f1245571aed052abe46) )
	ROM_LOAD( "136045-1110.6hj", 0x010000, 0x008000, CRC(9e91e734) SHA1(d9424a46dec2885fdc1ebe973f7deebfcb6ff19e) )
	ROM_LOAD( "136045-1109.6fh", 0x018000, 0x008000, CRC(3a051f36) SHA1(1f74d53e1950447fd79a552d934038c4661124cf) )
	ROM_LOAD( "136045-1116.6rs", 0x020000, 0x008000, CRC(b15c1b90) SHA1(a3d517919fe267f30383cff4142dd0d6e1102492) )
	ROM_LOAD( "136045-1115.6pr", 0x028000, 0x008000, CRC(1dcdd5aa) SHA1(99eb230597132f04aa7d4d66907a14596f780365) )
	ROM_LOAD( "136045-1114.6n",  0x030000, 0x008000, CRC(fb5677d9) SHA1(fa3190af7eaf0ef738fd86a8280c1c0b0556415f) )
	ROM_LOAD( "136045-1113.6m",  0x038000, 0x008000, CRC(35e70a8d) SHA1(953bb609143f2660e5e5ca980691b962c1e7a81e) )

	ROM_REGION( 0x4000, "chars", 0 )
	ROM_LOAD( "136045-1117.6t",  0x000000, 0x004000, CRC(82da786d) SHA1(929cc4ebac3d4404e1a8b22b80aae975e0c9da85) )

	ROM_REGION( 0x200, "eeprom", 0 )
	ROM_LOAD( "csprint-eeprom.bin", 0x0000, 0x0200, CRC(ce1c7319) SHA1(a12926efd898cda2a05cf23fd9cd6674b9ffc702) )
ROM_END


ROM_START( csprints1 )
	ROM_REGION( 0x90000, "maincpu", 0 ) // 9*64k for T11 code
	ROM_LOAD16_BYTE( "136045-1326.7l",  0x008000, 0x004000, CRC(cfa673a6) SHA1(db630ce26b6b2cf9215a7810ab6b93a5485bd5ae) )
	ROM_LOAD16_BYTE( "136045-1327.7n",  0x008001, 0x004000, CRC(16c1dcab) SHA1(deb3eaff35e7b3810133c7ce74a528d3a58babb2) )
	ROM_LOAD16_BYTE( "136045-1325.6f",  0x010000, 0x008000, CRC(8661f17b) SHA1(a9271fca78eba39484b1f806f675a69e33007633) )
	ROM_LOAD16_BYTE( "136045-1322.6n",  0x010001, 0x008000, CRC(7f440847) SHA1(cbc21a2ed3dd27bb66fb08fcc68f2d91de314ae1) )
	ROM_LOAD16_BYTE( "136045-1124.6k",  0x050000, 0x008000, CRC(47efca1f) SHA1(ab2fb8479c8606d8f180657734d8d320815c5408) )
	ROM_LOAD16_BYTE( "136045-1121.6r",  0x050001, 0x008000, CRC(6ca404bb) SHA1(27ceda243410edcff0dc0aa08fb6466d0c6c80c7) )
	ROM_LOAD16_BYTE( "136045-1123.6l",  0x070000, 0x008000, CRC(0a4d216a) SHA1(53a4af7673c9dae1f6f2f13dce3c38a31ee12ee2) )
	ROM_LOAD16_BYTE( "136045-1120.6s",  0x070001, 0x008000, CRC(103f3fde) SHA1(9a0e82c3294369858b7a6c978143d8145a8df5a2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    // 64k for 6502 code
	ROM_LOAD( "136045-1118.2bc", 0x008000, 0x004000, CRC(eba41b2f) SHA1(a0d6e9f4609f2587b0fad6845e75653c10bf4249) )
	ROM_LOAD( "136045-1119.2d",  0x00c000, 0x004000, CRC(9e49043a) SHA1(ec467fe1cd59c51e43c3acd83d300f5b3309a47a) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "136045-1105.6a",  0x000000, 0x008000, CRC(3773bfbb) SHA1(beca29d8b9296d723304aab391dd9589a830a150) )
	ROM_LOAD( "136045-1106.6b",  0x008000, 0x008000, CRC(13a24886) SHA1(a1ac9ca04a60fdb38c6d81e451e5638bdb537c8f) )
	ROM_LOAD( "136045-1101.7a",  0x030000, 0x008000, CRC(5a55f931) SHA1(b2f8e8f3488a9d3840ca9a15fa53896ae4013e0a) )
	ROM_CONTINUE(                0x010000, 0x008000 )
	ROM_LOAD( "136045-1102.7b",  0x018000, 0x008000, CRC(37548a60) SHA1(f3395df534cdc75d4e1b2b415ee2cd6683e49204) )
	ROM_LOAD( "136045-1107.6c",  0x040000, 0x008000, CRC(e35e354e) SHA1(fa07737eb2ca19bf96ce15d8bbae7513c7dddd3c) )
	ROM_LOAD( "136045-1108.6de", 0x048000, 0x008000, CRC(361db8b7) SHA1(fb85e63c6e9122ab3d62eb7d0f5f715d8936910b) )
	ROM_LOAD( "136045-1104.7de", 0x070000, 0x008000, CRC(d1f8fe7b) SHA1(2612a397fdebe6062f66d26060c36af4a9ca28dc) )
	ROM_CONTINUE(                0x050000, 0x008000 )
	ROM_LOAD( "136045-1103.7c",  0x058000, 0x008000, CRC(8f8c9692) SHA1(57418c5dd3554055e5bce8c2a15f6f6fc3201d99) )

	ROM_REGION( 0x40000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "136045-1112.6l",  0x000000, 0x008000, CRC(f869b0fc) SHA1(d8bf0df492bbe5d228748be0976d8f68254ffb3f) )
	ROM_LOAD( "136045-1111.6k",  0x008000, 0x008000, CRC(abcbc114) SHA1(5d11fdfd7ed968949d884f1245571aed052abe46) )
	ROM_LOAD( "136045-1110.6hj", 0x010000, 0x008000, CRC(9e91e734) SHA1(d9424a46dec2885fdc1ebe973f7deebfcb6ff19e) )
	ROM_LOAD( "136045-1109.6fh", 0x018000, 0x008000, CRC(3a051f36) SHA1(1f74d53e1950447fd79a552d934038c4661124cf) )
	ROM_LOAD( "136045-1116.6rs", 0x020000, 0x008000, CRC(b15c1b90) SHA1(a3d517919fe267f30383cff4142dd0d6e1102492) )
	ROM_LOAD( "136045-1115.6pr", 0x028000, 0x008000, CRC(1dcdd5aa) SHA1(99eb230597132f04aa7d4d66907a14596f780365) )
	ROM_LOAD( "136045-1114.6n",  0x030000, 0x008000, CRC(fb5677d9) SHA1(fa3190af7eaf0ef738fd86a8280c1c0b0556415f) )
	ROM_LOAD( "136045-1113.6m",  0x038000, 0x008000, CRC(35e70a8d) SHA1(953bb609143f2660e5e5ca980691b962c1e7a81e) )

	ROM_REGION( 0x4000, "chars", 0 )
	ROM_LOAD( "136045-1117.6t",  0x000000, 0x004000, CRC(82da786d) SHA1(929cc4ebac3d4404e1a8b22b80aae975e0c9da85) )

	ROM_REGION( 0x200, "eeprom", 0 )
	ROM_LOAD( "csprint-eeprom.bin", 0x0000, 0x0200, CRC(ce1c7319) SHA1(a12926efd898cda2a05cf23fd9cd6674b9ffc702) )
ROM_END


ROM_START( csprintf )
	ROM_REGION( 0x90000, "maincpu", 0 ) // 9*64k for T11 code
	ROM_LOAD16_BYTE( "136045-1626.7l",  0x008000, 0x004000, CRC(f9d4fbd3) SHA1(df8bea5190203be6157b0825af107c31404bbdf8) )
	ROM_LOAD16_BYTE( "136045-1627.7n",  0x008001, 0x004000, CRC(637f0afa) SHA1(ef89300b3d8fd8c2ddba76c0cdd2589f5ae16c81) )
	ROM_LOAD16_BYTE( "136045-1625.6f",  0x010000, 0x008000, CRC(1edc6462) SHA1(6e1653b71240fdc865dca7ea7916e8468245ea2f) )
	ROM_LOAD16_BYTE( "136045-1622.6n",  0x010001, 0x008000, CRC(a1c78189) SHA1(7eb1d2637167d3b9707d16580a2058afec0eca2d) )
	ROM_LOAD16_BYTE( "136045-1124.6k",  0x050000, 0x008000, CRC(47efca1f) SHA1(ab2fb8479c8606d8f180657734d8d320815c5408) )
	ROM_LOAD16_BYTE( "136045-1121.6r",  0x050001, 0x008000, CRC(6ca404bb) SHA1(27ceda243410edcff0dc0aa08fb6466d0c6c80c7) )
	ROM_LOAD16_BYTE( "136045-1123.6l",  0x070000, 0x008000, CRC(0a4d216a) SHA1(53a4af7673c9dae1f6f2f13dce3c38a31ee12ee2) )
	ROM_LOAD16_BYTE( "136045-1120.6s",  0x070001, 0x008000, CRC(103f3fde) SHA1(9a0e82c3294369858b7a6c978143d8145a8df5a2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    // 64k for 6502 code
	ROM_LOAD( "136045-1118.2bc", 0x008000, 0x004000, CRC(eba41b2f) SHA1(a0d6e9f4609f2587b0fad6845e75653c10bf4249) )
	ROM_LOAD( "136045-1119.2d",  0x00c000, 0x004000, CRC(9e49043a) SHA1(ec467fe1cd59c51e43c3acd83d300f5b3309a47a) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "136045-1105.6a",  0x000000, 0x008000, CRC(3773bfbb) SHA1(beca29d8b9296d723304aab391dd9589a830a150) )
	ROM_LOAD( "136045-1106.6b",  0x008000, 0x008000, CRC(13a24886) SHA1(a1ac9ca04a60fdb38c6d81e451e5638bdb537c8f) )
	ROM_LOAD( "136045-1101.7a",  0x030000, 0x008000, CRC(5a55f931) SHA1(b2f8e8f3488a9d3840ca9a15fa53896ae4013e0a) )
	ROM_CONTINUE(                0x010000, 0x008000 )
	ROM_LOAD( "136045-1102.7b",  0x018000, 0x008000, CRC(37548a60) SHA1(f3395df534cdc75d4e1b2b415ee2cd6683e49204) )
	ROM_LOAD( "136045-1107.6c",  0x040000, 0x008000, CRC(e35e354e) SHA1(fa07737eb2ca19bf96ce15d8bbae7513c7dddd3c) )
	ROM_LOAD( "136045-1108.6de", 0x048000, 0x008000, CRC(361db8b7) SHA1(fb85e63c6e9122ab3d62eb7d0f5f715d8936910b) )
	ROM_LOAD( "136045-1104.7de", 0x070000, 0x008000, CRC(d1f8fe7b) SHA1(2612a397fdebe6062f66d26060c36af4a9ca28dc) )
	ROM_CONTINUE(                0x050000, 0x008000 )
	ROM_LOAD( "136045-1103.7c",  0x058000, 0x008000, CRC(8f8c9692) SHA1(57418c5dd3554055e5bce8c2a15f6f6fc3201d99) )

	ROM_REGION( 0x40000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "136045-1112.6l",  0x000000, 0x008000, CRC(f869b0fc) SHA1(d8bf0df492bbe5d228748be0976d8f68254ffb3f) )
	ROM_LOAD( "136045-1111.6k",  0x008000, 0x008000, CRC(abcbc114) SHA1(5d11fdfd7ed968949d884f1245571aed052abe46) )
	ROM_LOAD( "136045-1110.6hj", 0x010000, 0x008000, CRC(9e91e734) SHA1(d9424a46dec2885fdc1ebe973f7deebfcb6ff19e) )
	ROM_LOAD( "136045-1109.6fh", 0x018000, 0x008000, CRC(3a051f36) SHA1(1f74d53e1950447fd79a552d934038c4661124cf) )
	ROM_LOAD( "136045-1116.6rs", 0x020000, 0x008000, CRC(b15c1b90) SHA1(a3d517919fe267f30383cff4142dd0d6e1102492) )
	ROM_LOAD( "136045-1115.6pr", 0x028000, 0x008000, CRC(1dcdd5aa) SHA1(99eb230597132f04aa7d4d66907a14596f780365) )
	ROM_LOAD( "136045-1114.6n",  0x030000, 0x008000, CRC(fb5677d9) SHA1(fa3190af7eaf0ef738fd86a8280c1c0b0556415f) )
	ROM_LOAD( "136045-1113.6m",  0x038000, 0x008000, CRC(35e70a8d) SHA1(953bb609143f2660e5e5ca980691b962c1e7a81e) )

	ROM_REGION( 0x4000, "chars", 0 )
	ROM_LOAD( "136045-1117.6t",  0x000000, 0x004000, CRC(82da786d) SHA1(929cc4ebac3d4404e1a8b22b80aae975e0c9da85) )

	ROM_REGION( 0x200, "eeprom", 0 )
	ROM_LOAD( "csprint-eeprom.bin", 0x0000, 0x0200, CRC(ce1c7319) SHA1(a12926efd898cda2a05cf23fd9cd6674b9ffc702) )
ROM_END


ROM_START( csprintg )
	ROM_REGION( 0x90000, "maincpu", 0 ) // 9*64k for T11 code
	ROM_LOAD16_BYTE( "136045-2226.7l",  0x008000, 0x004000, CRC(1f437a3f) SHA1(f976a023124d002e922669585eb22334720c15e5) )
	ROM_LOAD16_BYTE( "136045-1227.7n",  0x008001, 0x004000, CRC(d1dce1cc) SHA1(2de07c4730e1b5e4b11466220bb350f3263d43e7) )
	ROM_LOAD16_BYTE( "136045-1225.6f",  0x010000, 0x008000, CRC(e787da64) SHA1(8a5a9731b39808525a69522006801322d27d1e6b) )
	ROM_LOAD16_BYTE( "136045-1222.6n",  0x010001, 0x008000, CRC(5656cc40) SHA1(7431ac6ed941b5a3157c63f6d4e65cf302f2d482) )
	ROM_LOAD16_BYTE( "136045-1124.6k",  0x050000, 0x008000, CRC(47efca1f) SHA1(ab2fb8479c8606d8f180657734d8d320815c5408) )
	ROM_LOAD16_BYTE( "136045-1121.6r",  0x050001, 0x008000, CRC(6ca404bb) SHA1(27ceda243410edcff0dc0aa08fb6466d0c6c80c7) )
	ROM_LOAD16_BYTE( "136045-1123.6l",  0x070000, 0x008000, CRC(0a4d216a) SHA1(53a4af7673c9dae1f6f2f13dce3c38a31ee12ee2) )
	ROM_LOAD16_BYTE( "136045-1120.6s",  0x070001, 0x008000, CRC(103f3fde) SHA1(9a0e82c3294369858b7a6c978143d8145a8df5a2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    // 64k for 6502 code
	ROM_LOAD( "136045-1118.2bc", 0x008000, 0x004000, CRC(eba41b2f) SHA1(a0d6e9f4609f2587b0fad6845e75653c10bf4249) )
	ROM_LOAD( "136045-1119.2d",  0x00c000, 0x004000, CRC(9e49043a) SHA1(ec467fe1cd59c51e43c3acd83d300f5b3309a47a) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "136045-1105.6a",  0x000000, 0x008000, CRC(3773bfbb) SHA1(beca29d8b9296d723304aab391dd9589a830a150) )
	ROM_LOAD( "136045-1106.6b",  0x008000, 0x008000, CRC(13a24886) SHA1(a1ac9ca04a60fdb38c6d81e451e5638bdb537c8f) )
	ROM_LOAD( "136045-1101.7a",  0x030000, 0x008000, CRC(5a55f931) SHA1(b2f8e8f3488a9d3840ca9a15fa53896ae4013e0a) )
	ROM_CONTINUE(                0x010000, 0x008000 )
	ROM_LOAD( "136045-1102.7b",  0x018000, 0x008000, CRC(37548a60) SHA1(f3395df534cdc75d4e1b2b415ee2cd6683e49204) )
	ROM_LOAD( "136045-1107.6c",  0x040000, 0x008000, CRC(e35e354e) SHA1(fa07737eb2ca19bf96ce15d8bbae7513c7dddd3c) )
	ROM_LOAD( "136045-1108.6de", 0x048000, 0x008000, CRC(361db8b7) SHA1(fb85e63c6e9122ab3d62eb7d0f5f715d8936910b) )
	ROM_LOAD( "136045-1104.7de", 0x070000, 0x008000, CRC(d1f8fe7b) SHA1(2612a397fdebe6062f66d26060c36af4a9ca28dc) )
	ROM_CONTINUE(                0x050000, 0x008000 )
	ROM_LOAD( "136045-1103.7c",  0x058000, 0x008000, CRC(8f8c9692) SHA1(57418c5dd3554055e5bce8c2a15f6f6fc3201d99) )

	ROM_REGION( 0x40000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "136045-1112.6l",  0x000000, 0x008000, CRC(f869b0fc) SHA1(d8bf0df492bbe5d228748be0976d8f68254ffb3f) )
	ROM_LOAD( "136045-1111.6k",  0x008000, 0x008000, CRC(abcbc114) SHA1(5d11fdfd7ed968949d884f1245571aed052abe46) )
	ROM_LOAD( "136045-1110.6hj", 0x010000, 0x008000, CRC(9e91e734) SHA1(d9424a46dec2885fdc1ebe973f7deebfcb6ff19e) )
	ROM_LOAD( "136045-1109.6fh", 0x018000, 0x008000, CRC(3a051f36) SHA1(1f74d53e1950447fd79a552d934038c4661124cf) )
	ROM_LOAD( "136045-1116.6rs", 0x020000, 0x008000, CRC(b15c1b90) SHA1(a3d517919fe267f30383cff4142dd0d6e1102492) )
	ROM_LOAD( "136045-1115.6pr", 0x028000, 0x008000, CRC(1dcdd5aa) SHA1(99eb230597132f04aa7d4d66907a14596f780365) )
	ROM_LOAD( "136045-1114.6n",  0x030000, 0x008000, CRC(fb5677d9) SHA1(fa3190af7eaf0ef738fd86a8280c1c0b0556415f) )
	ROM_LOAD( "136045-1113.6m",  0x038000, 0x008000, CRC(35e70a8d) SHA1(953bb609143f2660e5e5ca980691b962c1e7a81e) )

	ROM_REGION( 0x4000, "chars", 0 )
	ROM_LOAD( "136045-1117.6t",  0x000000, 0x004000, CRC(82da786d) SHA1(929cc4ebac3d4404e1a8b22b80aae975e0c9da85) )

	ROM_REGION( 0x200, "eeprom", 0 )
	ROM_LOAD( "csprint-eeprom.bin", 0x0000, 0x0200, CRC(ce1c7319) SHA1(a12926efd898cda2a05cf23fd9cd6674b9ffc702) )
ROM_END


ROM_START( csprint2 )
	ROM_REGION( 0x90000, "maincpu", 0 ) // 9*64k for T11 code
	ROM_LOAD16_BYTE( "136045-2126.7l",  0x008000, 0x004000, CRC(0ff83de8) SHA1(23f90b8f3ebd3d3bbd7a68aaecae5f45f1b477c0) )
	ROM_LOAD16_BYTE( "136045-1127.7n",  0x008001, 0x004000, CRC(e3e37258) SHA1(64f48c1acbb02cc8f6b76232d142382007485fb2) )
	ROM_LOAD16_BYTE( "136045-1125.6f",  0x010000, 0x008000, CRC(650623d2) SHA1(036cb441aba64d08f3b50f72cb22fed3b4766341) )
	ROM_LOAD16_BYTE( "136045-1122.6n",  0x010001, 0x008000, CRC(ca1b1cbf) SHA1(98674b75a1d38db32ceb24d57f6dba836bdc8566) )
	ROM_LOAD16_BYTE( "136045-1124.6k",  0x050000, 0x008000, CRC(47efca1f) SHA1(ab2fb8479c8606d8f180657734d8d320815c5408) )
	ROM_LOAD16_BYTE( "136045-1121.6r",  0x050001, 0x008000, CRC(6ca404bb) SHA1(27ceda243410edcff0dc0aa08fb6466d0c6c80c7) )
	ROM_LOAD16_BYTE( "136045-1123.6l",  0x070000, 0x008000, CRC(0a4d216a) SHA1(53a4af7673c9dae1f6f2f13dce3c38a31ee12ee2) )
	ROM_LOAD16_BYTE( "136045-1120.6s",  0x070001, 0x008000, CRC(103f3fde) SHA1(9a0e82c3294369858b7a6c978143d8145a8df5a2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    // 64k for 6502 code
	ROM_LOAD( "136045-1118.2bc", 0x008000, 0x004000, CRC(eba41b2f) SHA1(a0d6e9f4609f2587b0fad6845e75653c10bf4249) )
	ROM_LOAD( "136045-1119.2d",  0x00c000, 0x004000, CRC(9e49043a) SHA1(ec467fe1cd59c51e43c3acd83d300f5b3309a47a) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "136045-1105.6a",  0x000000, 0x008000, CRC(3773bfbb) SHA1(beca29d8b9296d723304aab391dd9589a830a150) )
	ROM_LOAD( "136045-1106.6b",  0x008000, 0x008000, CRC(13a24886) SHA1(a1ac9ca04a60fdb38c6d81e451e5638bdb537c8f) )
	ROM_LOAD( "136045-1101.7a",  0x030000, 0x008000, CRC(5a55f931) SHA1(b2f8e8f3488a9d3840ca9a15fa53896ae4013e0a) )
	ROM_CONTINUE(                0x010000, 0x008000 )
	ROM_LOAD( "136045-1102.7b",  0x018000, 0x008000, CRC(37548a60) SHA1(f3395df534cdc75d4e1b2b415ee2cd6683e49204) )
	ROM_LOAD( "136045-1107.6c",  0x040000, 0x008000, CRC(e35e354e) SHA1(fa07737eb2ca19bf96ce15d8bbae7513c7dddd3c) )
	ROM_LOAD( "136045-1108.6de", 0x048000, 0x008000, CRC(361db8b7) SHA1(fb85e63c6e9122ab3d62eb7d0f5f715d8936910b) )
	ROM_LOAD( "136045-1104.7de", 0x070000, 0x008000, CRC(d1f8fe7b) SHA1(2612a397fdebe6062f66d26060c36af4a9ca28dc) )
	ROM_CONTINUE(                0x050000, 0x008000 )
	ROM_LOAD( "136045-1103.7c",  0x058000, 0x008000, CRC(8f8c9692) SHA1(57418c5dd3554055e5bce8c2a15f6f6fc3201d99) )

	ROM_REGION( 0x40000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "136045-1112.6l",  0x000000, 0x008000, CRC(f869b0fc) SHA1(d8bf0df492bbe5d228748be0976d8f68254ffb3f) )
	ROM_LOAD( "136045-1111.6k",  0x008000, 0x008000, CRC(abcbc114) SHA1(5d11fdfd7ed968949d884f1245571aed052abe46) )
	ROM_LOAD( "136045-1110.6hj", 0x010000, 0x008000, CRC(9e91e734) SHA1(d9424a46dec2885fdc1ebe973f7deebfcb6ff19e) )
	ROM_LOAD( "136045-1109.6fh", 0x018000, 0x008000, CRC(3a051f36) SHA1(1f74d53e1950447fd79a552d934038c4661124cf) )
	ROM_LOAD( "136045-1116.6rs", 0x020000, 0x008000, CRC(b15c1b90) SHA1(a3d517919fe267f30383cff4142dd0d6e1102492) )
	ROM_LOAD( "136045-1115.6pr", 0x028000, 0x008000, CRC(1dcdd5aa) SHA1(99eb230597132f04aa7d4d66907a14596f780365) )
	ROM_LOAD( "136045-1114.6n",  0x030000, 0x008000, CRC(fb5677d9) SHA1(fa3190af7eaf0ef738fd86a8280c1c0b0556415f) )
	ROM_LOAD( "136045-1113.6m",  0x038000, 0x008000, CRC(35e70a8d) SHA1(953bb609143f2660e5e5ca980691b962c1e7a81e) )

	ROM_REGION( 0x4000, "chars", 0 )
	ROM_LOAD( "136045-1117.6t",  0x000000, 0x004000, CRC(82da786d) SHA1(929cc4ebac3d4404e1a8b22b80aae975e0c9da85) )

	ROM_REGION( 0x200, "eeprom", 0 )
	ROM_LOAD( "csprint-eeprom.bin", 0x0000, 0x0200, CRC(ce1c7319) SHA1(a12926efd898cda2a05cf23fd9cd6674b9ffc702) )
ROM_END


ROM_START( csprintg1 )
	ROM_REGION( 0x90000, "maincpu", 0 ) // 9*64k for T11 code
	ROM_LOAD16_BYTE( "136045-1226.7l",  0x008000, 0x004000, CRC(becfc276) SHA1(cc3a6ef91fb3a47426a347ba4f6df41582e6deba) )
	ROM_LOAD16_BYTE( "136045-1227.7n",  0x008001, 0x004000, CRC(d1dce1cc) SHA1(2de07c4730e1b5e4b11466220bb350f3263d43e7) )
	ROM_LOAD16_BYTE( "136045-1225.6f",  0x010000, 0x008000, CRC(e787da64) SHA1(8a5a9731b39808525a69522006801322d27d1e6b) )
	ROM_LOAD16_BYTE( "136045-1222.6n",  0x010001, 0x008000, CRC(5656cc40) SHA1(7431ac6ed941b5a3157c63f6d4e65cf302f2d482) )
	ROM_LOAD16_BYTE( "136045-1124.6k",  0x050000, 0x008000, CRC(47efca1f) SHA1(ab2fb8479c8606d8f180657734d8d320815c5408) )
	ROM_LOAD16_BYTE( "136045-1121.6r",  0x050001, 0x008000, CRC(6ca404bb) SHA1(27ceda243410edcff0dc0aa08fb6466d0c6c80c7) )
	ROM_LOAD16_BYTE( "136045-1123.6l",  0x070000, 0x008000, CRC(0a4d216a) SHA1(53a4af7673c9dae1f6f2f13dce3c38a31ee12ee2) )
	ROM_LOAD16_BYTE( "136045-1120.6s",  0x070001, 0x008000, CRC(103f3fde) SHA1(9a0e82c3294369858b7a6c978143d8145a8df5a2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    // 64k for 6502 code
	ROM_LOAD( "136045-1118.2bc", 0x008000, 0x004000, CRC(eba41b2f) SHA1(a0d6e9f4609f2587b0fad6845e75653c10bf4249) )
	ROM_LOAD( "136045-1119.2d",  0x00c000, 0x004000, CRC(9e49043a) SHA1(ec467fe1cd59c51e43c3acd83d300f5b3309a47a) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "136045-1105.6a",  0x000000, 0x008000, CRC(3773bfbb) SHA1(beca29d8b9296d723304aab391dd9589a830a150) )
	ROM_LOAD( "136045-1106.6b",  0x008000, 0x008000, CRC(13a24886) SHA1(a1ac9ca04a60fdb38c6d81e451e5638bdb537c8f) )
	ROM_LOAD( "136045-1101.7a",  0x030000, 0x008000, CRC(5a55f931) SHA1(b2f8e8f3488a9d3840ca9a15fa53896ae4013e0a) )
	ROM_CONTINUE(                0x010000, 0x008000 )
	ROM_LOAD( "136045-1102.7b",  0x018000, 0x008000, CRC(37548a60) SHA1(f3395df534cdc75d4e1b2b415ee2cd6683e49204) )
	ROM_LOAD( "136045-1107.6c",  0x040000, 0x008000, CRC(e35e354e) SHA1(fa07737eb2ca19bf96ce15d8bbae7513c7dddd3c) )
	ROM_LOAD( "136045-1108.6de", 0x048000, 0x008000, CRC(361db8b7) SHA1(fb85e63c6e9122ab3d62eb7d0f5f715d8936910b) )
	ROM_LOAD( "136045-1104.7de", 0x070000, 0x008000, CRC(d1f8fe7b) SHA1(2612a397fdebe6062f66d26060c36af4a9ca28dc) )
	ROM_CONTINUE(                0x050000, 0x008000 )
	ROM_LOAD( "136045-1103.7c",  0x058000, 0x008000, CRC(8f8c9692) SHA1(57418c5dd3554055e5bce8c2a15f6f6fc3201d99) )

	ROM_REGION( 0x40000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "136045-1112.6l",  0x000000, 0x008000, CRC(f869b0fc) SHA1(d8bf0df492bbe5d228748be0976d8f68254ffb3f) )
	ROM_LOAD( "136045-1111.6k",  0x008000, 0x008000, CRC(abcbc114) SHA1(5d11fdfd7ed968949d884f1245571aed052abe46) )
	ROM_LOAD( "136045-1110.6hj", 0x010000, 0x008000, CRC(9e91e734) SHA1(d9424a46dec2885fdc1ebe973f7deebfcb6ff19e) )
	ROM_LOAD( "136045-1109.6fh", 0x018000, 0x008000, CRC(3a051f36) SHA1(1f74d53e1950447fd79a552d934038c4661124cf) )
	ROM_LOAD( "136045-1116.6rs", 0x020000, 0x008000, CRC(b15c1b90) SHA1(a3d517919fe267f30383cff4142dd0d6e1102492) )
	ROM_LOAD( "136045-1115.6pr", 0x028000, 0x008000, CRC(1dcdd5aa) SHA1(99eb230597132f04aa7d4d66907a14596f780365) )
	ROM_LOAD( "136045-1114.6n",  0x030000, 0x008000, CRC(fb5677d9) SHA1(fa3190af7eaf0ef738fd86a8280c1c0b0556415f) )
	ROM_LOAD( "136045-1113.6m",  0x038000, 0x008000, CRC(35e70a8d) SHA1(953bb609143f2660e5e5ca980691b962c1e7a81e) )

	ROM_REGION( 0x4000, "chars", 0 )
	ROM_LOAD( "136045-1117.6t",  0x000000, 0x004000, CRC(82da786d) SHA1(929cc4ebac3d4404e1a8b22b80aae975e0c9da85) )

	ROM_REGION( 0x200, "eeprom", 0 )
	ROM_LOAD( "csprint-eeprom.bin", 0x0000, 0x0200, CRC(ce1c7319) SHA1(a12926efd898cda2a05cf23fd9cd6674b9ffc702) )
ROM_END


ROM_START( csprint1 )
	ROM_REGION( 0x90000, "maincpu", 0 ) // 9*64k for T11 code
	ROM_LOAD16_BYTE( "136045-1126.7l",  0x008000, 0x004000, CRC(a04ecbac) SHA1(56a77c9fd8cdd963616cf25838ade2a87a87947b) )
	ROM_LOAD16_BYTE( "136045-1127.7n",  0x008001, 0x004000, CRC(e3e37258) SHA1(64f48c1acbb02cc8f6b76232d142382007485fb2) )
	ROM_LOAD16_BYTE( "136045-1125.6f",  0x010000, 0x008000, CRC(650623d2) SHA1(036cb441aba64d08f3b50f72cb22fed3b4766341) )
	ROM_LOAD16_BYTE( "136045-1122.6n",  0x010001, 0x008000, CRC(ca1b1cbf) SHA1(98674b75a1d38db32ceb24d57f6dba836bdc8566) )
	ROM_LOAD16_BYTE( "136045-1124.6k",  0x050000, 0x008000, CRC(47efca1f) SHA1(ab2fb8479c8606d8f180657734d8d320815c5408) )
	ROM_LOAD16_BYTE( "136045-1121.6r",  0x050001, 0x008000, CRC(6ca404bb) SHA1(27ceda243410edcff0dc0aa08fb6466d0c6c80c7) )
	ROM_LOAD16_BYTE( "136045-1123.6l",  0x070000, 0x008000, CRC(0a4d216a) SHA1(53a4af7673c9dae1f6f2f13dce3c38a31ee12ee2) )
	ROM_LOAD16_BYTE( "136045-1120.6s",  0x070001, 0x008000, CRC(103f3fde) SHA1(9a0e82c3294369858b7a6c978143d8145a8df5a2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    // 64k for 6502 code
	ROM_LOAD( "136045-1118.2bc", 0x008000, 0x004000, CRC(eba41b2f) SHA1(a0d6e9f4609f2587b0fad6845e75653c10bf4249) )
	ROM_LOAD( "136045-1119.2d",  0x00c000, 0x004000, CRC(9e49043a) SHA1(ec467fe1cd59c51e43c3acd83d300f5b3309a47a) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "136045-1105.6a",  0x000000, 0x008000, CRC(3773bfbb) SHA1(beca29d8b9296d723304aab391dd9589a830a150) )
	ROM_LOAD( "136045-1106.6b",  0x008000, 0x008000, CRC(13a24886) SHA1(a1ac9ca04a60fdb38c6d81e451e5638bdb537c8f) )
	ROM_LOAD( "136045-1101.7a",  0x030000, 0x008000, CRC(5a55f931) SHA1(b2f8e8f3488a9d3840ca9a15fa53896ae4013e0a) )
	ROM_CONTINUE(                0x010000, 0x008000 )
	ROM_LOAD( "136045-1102.7b",  0x018000, 0x008000, CRC(37548a60) SHA1(f3395df534cdc75d4e1b2b415ee2cd6683e49204) )
	ROM_LOAD( "136045-1107.6c",  0x040000, 0x008000, CRC(e35e354e) SHA1(fa07737eb2ca19bf96ce15d8bbae7513c7dddd3c) )
	ROM_LOAD( "136045-1108.6de", 0x048000, 0x008000, CRC(361db8b7) SHA1(fb85e63c6e9122ab3d62eb7d0f5f715d8936910b) )
	ROM_LOAD( "136045-1104.7de", 0x070000, 0x008000, CRC(d1f8fe7b) SHA1(2612a397fdebe6062f66d26060c36af4a9ca28dc) )
	ROM_CONTINUE(                0x050000, 0x008000 )
	ROM_LOAD( "136045-1103.7c",  0x058000, 0x008000, CRC(8f8c9692) SHA1(57418c5dd3554055e5bce8c2a15f6f6fc3201d99) )

	ROM_REGION( 0x40000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "136045-1112.6l",  0x000000, 0x008000, CRC(f869b0fc) SHA1(d8bf0df492bbe5d228748be0976d8f68254ffb3f) )
	ROM_LOAD( "136045-1111.6k",  0x008000, 0x008000, CRC(abcbc114) SHA1(5d11fdfd7ed968949d884f1245571aed052abe46) )
	ROM_LOAD( "136045-1110.6hj", 0x010000, 0x008000, CRC(9e91e734) SHA1(d9424a46dec2885fdc1ebe973f7deebfcb6ff19e) )
	ROM_LOAD( "136045-1109.6fh", 0x018000, 0x008000, CRC(3a051f36) SHA1(1f74d53e1950447fd79a552d934038c4661124cf) )
	ROM_LOAD( "136045-1116.6rs", 0x020000, 0x008000, CRC(b15c1b90) SHA1(a3d517919fe267f30383cff4142dd0d6e1102492) )
	ROM_LOAD( "136045-1115.6pr", 0x028000, 0x008000, CRC(1dcdd5aa) SHA1(99eb230597132f04aa7d4d66907a14596f780365) )
	ROM_LOAD( "136045-1114.6n",  0x030000, 0x008000, CRC(fb5677d9) SHA1(fa3190af7eaf0ef738fd86a8280c1c0b0556415f) )
	ROM_LOAD( "136045-1113.6m",  0x038000, 0x008000, CRC(35e70a8d) SHA1(953bb609143f2660e5e5ca980691b962c1e7a81e) )

	ROM_REGION( 0x4000, "chars", 0 )
	ROM_LOAD( "136045-1117.6t",  0x000000, 0x004000, CRC(82da786d) SHA1(929cc4ebac3d4404e1a8b22b80aae975e0c9da85) )

	ROM_REGION( 0x200, "eeprom", 0 )
	ROM_LOAD( "csprint-eeprom.bin", 0x0000, 0x0200, CRC(ce1c7319) SHA1(a12926efd898cda2a05cf23fd9cd6674b9ffc702) )
ROM_END


ROM_START( apb )
	ROM_REGION( 0x90000, "maincpu", 0 )     // 9 * 64k T11 code
	ROM_LOAD16_BYTE( "136051-2126.7l",  0x008000, 0x004000, CRC(8edf4726) SHA1(48ee833c84808abe0eb8b7932abf082af09066da) )
	ROM_LOAD16_BYTE( "136051-2127.7n",  0x008001, 0x004000, CRC(e2b2aff2) SHA1(bded098d56d68a95333eb68bc0c9629a2257aa16) )
	ROM_LOAD16_BYTE( "136051-7128.6f",  0x010000, 0x010000, CRC(c08504d2) SHA1(91762807270b6294f676efb40a2a7deb3732f358) )
	ROM_LOAD16_BYTE( "136051-7129.6n",  0x010001, 0x010000, CRC(79adb57f) SHA1(c5a3bf4e3da221ebf05b7175f975d9564f0e8c32) )
	ROM_LOAD16_BYTE( "136051-1130.6j",  0x030000, 0x010000, CRC(f64c752e) SHA1(a0a7a0629b228ffd2a0c92725305b8d094484ddb) )
	ROM_LOAD16_BYTE( "136051-1131.6p",  0x030001, 0x010000, CRC(0a506e04) SHA1(98ff0de15632397e7371d8473b0174251b1ef9a1) )
	ROM_LOAD16_BYTE( "136051-1132.6l",  0x070000, 0x010000, CRC(6d0e7a4e) SHA1(75aae74571c50d36639d0ae69b0614e5aedeb6e3) )
	ROM_LOAD16_BYTE( "136051-1133.6s",  0x070001, 0x010000, CRC(af88d429) SHA1(432720afd4179d3df871226e0eb576d2ffde44c1) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     // 64k for 6502 code
	ROM_LOAD( "136051-5134.2a",  0x004000, 0x004000, CRC(1c8bdeed) SHA1(bbdbbb9a4903f98842d9a697410a2e3a2069284c) )
	ROM_LOAD( "136051-5135.2bc", 0x008000, 0x004000, CRC(ed6adb91) SHA1(b1f1f0d1bda445a53de798fb6847c605afe53e3c) )
	ROM_LOAD( "136051-5136.2d",  0x00c000, 0x004000, CRC(341f8486) SHA1(4cea39c0d8551ce7193e51de341f7297a94b8d9b) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "136051-1118.6a",  0x000000, 0x008000, CRC(93752c49) SHA1(6b4ed2defab85ab4d4943bfdf43c04dd42cc2ec5) )
	ROM_LOAD( "136051-1120.6bc", 0x028000, 0x008000, CRC(043086f8) SHA1(8920c8aab37d7b0ecfa17381c65cc00b9b99d4da) )
	ROM_CONTINUE(                0x008000, 0x008000 )
	ROM_LOAD( "136051-1122.7a",  0x030000, 0x008000, CRC(5ee79481) SHA1(82a0eee49cdfe08aeb8619bc7b83c915a8de95c7) )
	ROM_CONTINUE(                0x010000, 0x008000 )
	ROM_LOAD( "136051-1124.7bc", 0x038000, 0x008000, CRC(27760395) SHA1(5c6201743d973389f7ed13ec4253c2034ab23c62) )
	ROM_CONTINUE(                0x018000, 0x008000 )
	ROM_LOAD( "136051-1117.6cd", 0x040000, 0x008000, CRC(cfc3f8a3) SHA1(7fed2a40c2ba28444c5f5470d0b7c86198469651) )
	ROM_LOAD( "136051-1119.6de", 0x068000, 0x008000, CRC(68850612) SHA1(de899a0eb0b9fdbc96ccd3987d1dc942b54f2cc6) )
	ROM_CONTINUE(                0x048000, 0x008000 )
	ROM_LOAD( "136051-1121.7de", 0x070000, 0x008000, CRC(c7977062) SHA1(3a2f8da5c4cd7693575ad13246c6dbb6e2f02131) )
	ROM_CONTINUE(                0x050000, 0x008000 )
	ROM_LOAD( "136051-1123.7cd", 0x078000, 0x008000, CRC(3c96c848) SHA1(a90a0c14ee5d5cdf60a1d3ecd9984b74c31b9f36) )
	ROM_CONTINUE(                0x058000, 0x008000 )

	ROM_REGION( 0x100000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "136051-1105.6t",  0x020000, 0x008000, CRC(9b78a88e) SHA1(d7dff7a72041ebb7ff4f56da74cc7eb4b71bc5cc) )
	ROM_CONTINUE(                0x000000, 0x008000 )
	ROM_LOAD( "136051-1106.6rs", 0x028000, 0x008000, CRC(4787ff58) SHA1(35b276eb1ad1ce8b143485502430335c08aa9474) )
	ROM_CONTINUE(                0x008000, 0x008000 )
	ROM_LOAD( "136051-1107.6pr", 0x030000, 0x008000, CRC(0e85f2ac) SHA1(f78769962b4d6f6b4eaac5b6ae8e08ff90ad55ac) )
	ROM_CONTINUE(                0x010000, 0x008000 )
	ROM_LOAD( "136051-1108.6n",  0x038000, 0x008000, CRC(70ff9308) SHA1(c1cdc428058d5a6a9706431d4aa3d2ae81815d9c) )
	ROM_CONTINUE(                0x018000, 0x008000 )
	ROM_LOAD( "136051-1113.6m",  0x060000, 0x008000, CRC(4a445356) SHA1(0aa995bc1234c4b84b72c2e779968b1052cfa6e5) )
	ROM_CONTINUE(                0x040000, 0x008000 )
	ROM_LOAD( "136051-1114.6kl", 0x068000, 0x008000, CRC(b9b27f3c) SHA1(ef3378a2569ccf34360edf6ea1630bed2b83c438) )
	ROM_CONTINUE(                0x048000, 0x008000 )
	ROM_LOAD( "136051-1115.6jk", 0x070000, 0x008000, CRC(a7671dd8) SHA1(faaa088b43f354ad5b5bcf96ed9eab54a2ad162e) )
	ROM_CONTINUE(                0x050000, 0x008000 )
	ROM_LOAD( "136051-1116.6h",  0x078000, 0x008000, CRC(879fc7de) SHA1(4a5994898d546fa914d5eb6e5a1a9c4c2febea5a) )
	ROM_CONTINUE(                0x058000, 0x008000 )
	ROM_LOAD( "136051-1101.5t",  0x0a0000, 0x008000, CRC(0ef13513) SHA1(ee9c1088f203c607b10c9e7927c8c4a29d727726) )
	ROM_CONTINUE(                0x080000, 0x008000 )
	ROM_LOAD( "136051-1102.5rs", 0x0a8000, 0x008000, CRC(401e06fd) SHA1(a0c3a90fd400d146b7b9cc12a8dfaa5db0e4426a) )
	ROM_CONTINUE(                0x088000, 0x008000 )
	ROM_LOAD( "136051-1103.5pr", 0x0b0000, 0x008000, CRC(50d820e8) SHA1(28473949570d402754737c6d38de2a096f804676) )
	ROM_CONTINUE(                0x090000, 0x008000 )
	ROM_LOAD( "136051-1104.5n",  0x0b8000, 0x008000, CRC(912d878f) SHA1(87e0eb5910ec7042628378bcfd58d9b7beca690d) )
	ROM_CONTINUE(                0x098000, 0x008000 )
	ROM_LOAD( "136051-1109.5m",  0x0e0000, 0x008000, CRC(6716a408) SHA1(cb05fa401456177170c32958dbcfec95667944cd) )
	ROM_CONTINUE(                0x0c0000, 0x008000 )
	ROM_LOAD( "136051-1110.5kl", 0x0e8000, 0x008000, CRC(7e184981) SHA1(475b5c3224ee2e5b5330cc2b4ad018b6bcf0abd5) )
	ROM_CONTINUE(                0x0c8000, 0x008000 )
	ROM_LOAD( "136051-1111.5jk", 0x0f0000, 0x008000, CRC(353a14fd) SHA1(6464b089395ce52f26a604357a4783fe5c673362) )
	ROM_CONTINUE(                0x0d0000, 0x008000 )
	ROM_LOAD( "136051-1112.5h",  0x0f8000, 0x008000, CRC(3af7c50f) SHA1(e100ec8a8aee643d3175a8f54cc7e9266cf50e42) )
	ROM_CONTINUE(                0x0d8000, 0x008000 )

	ROM_REGION( 0x4000, "chars", 0 )
	ROM_LOAD( "136051-1125.4t",  0x000000, 0x004000, CRC(05a0341c) SHA1(90b96e0645a01939c681a7a5fe5d236f3dfc71b7) )
ROM_END


ROM_START( apb6 )
	ROM_REGION( 0x90000, "maincpu", 0 )     // 9 * 64k T11 code
	ROM_LOAD16_BYTE( "136051-2126.7l",  0x008000, 0x004000, CRC(8edf4726) SHA1(48ee833c84808abe0eb8b7932abf082af09066da) )
	ROM_LOAD16_BYTE( "136051-2127.7n",  0x008001, 0x004000, CRC(e2b2aff2) SHA1(bded098d56d68a95333eb68bc0c9629a2257aa16) )
	ROM_LOAD16_BYTE( "136051-6128.6f",  0x010000, 0x010000, CRC(c852959d) SHA1(1bc5c3130ad5c9eae40646db25a038f93a802822) )
	ROM_LOAD16_BYTE( "136051-6129.6n",  0x010001, 0x010000, CRC(b5d1d8eb) SHA1(eb53d7272dd5f18f8fa3e484f957b68901009973) )
	ROM_LOAD16_BYTE( "136051-1130.6j",  0x030000, 0x010000, CRC(f64c752e) SHA1(a0a7a0629b228ffd2a0c92725305b8d094484ddb) )
	ROM_LOAD16_BYTE( "136051-1131.6p",  0x030001, 0x010000, CRC(0a506e04) SHA1(98ff0de15632397e7371d8473b0174251b1ef9a1) )
	ROM_LOAD16_BYTE( "136051-1132.6l",  0x070000, 0x010000, CRC(6d0e7a4e) SHA1(75aae74571c50d36639d0ae69b0614e5aedeb6e3) )
	ROM_LOAD16_BYTE( "136051-1133.6s",  0x070001, 0x010000, CRC(af88d429) SHA1(432720afd4179d3df871226e0eb576d2ffde44c1) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     // 64k for 6502 code
	ROM_LOAD( "136051-5134.2a",  0x004000, 0x004000, CRC(1c8bdeed) SHA1(bbdbbb9a4903f98842d9a697410a2e3a2069284c) )
	ROM_LOAD( "136051-5135.2bc", 0x008000, 0x004000, CRC(ed6adb91) SHA1(b1f1f0d1bda445a53de798fb6847c605afe53e3c) )
	ROM_LOAD( "136051-5136.2d",  0x00c000, 0x004000, CRC(341f8486) SHA1(4cea39c0d8551ce7193e51de341f7297a94b8d9b) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "136051-1118.6a",  0x000000, 0x008000, CRC(93752c49) SHA1(6b4ed2defab85ab4d4943bfdf43c04dd42cc2ec5) )
	ROM_LOAD( "136051-1120.6bc", 0x028000, 0x008000, CRC(043086f8) SHA1(8920c8aab37d7b0ecfa17381c65cc00b9b99d4da) )
	ROM_CONTINUE(                0x008000, 0x008000 )
	ROM_LOAD( "136051-1122.7a",  0x030000, 0x008000, CRC(5ee79481) SHA1(82a0eee49cdfe08aeb8619bc7b83c915a8de95c7) )
	ROM_CONTINUE(                0x010000, 0x008000 )
	ROM_LOAD( "136051-1124.7bc", 0x038000, 0x008000, CRC(27760395) SHA1(5c6201743d973389f7ed13ec4253c2034ab23c62) )
	ROM_CONTINUE(                0x018000, 0x008000 )
	ROM_LOAD( "136051-1117.6cd", 0x040000, 0x008000, CRC(cfc3f8a3) SHA1(7fed2a40c2ba28444c5f5470d0b7c86198469651) )
	ROM_LOAD( "136051-1119.6de", 0x068000, 0x008000, CRC(68850612) SHA1(de899a0eb0b9fdbc96ccd3987d1dc942b54f2cc6) )
	ROM_CONTINUE(                0x048000, 0x008000 )
	ROM_LOAD( "136051-1121.7de", 0x070000, 0x008000, CRC(c7977062) SHA1(3a2f8da5c4cd7693575ad13246c6dbb6e2f02131) )
	ROM_CONTINUE(                0x050000, 0x008000 )
	ROM_LOAD( "136051-1123.7cd", 0x078000, 0x008000, CRC(3c96c848) SHA1(a90a0c14ee5d5cdf60a1d3ecd9984b74c31b9f36) )
	ROM_CONTINUE(                0x058000, 0x008000 )

	ROM_REGION( 0x100000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "136051-1105.6t",  0x020000, 0x008000, CRC(9b78a88e) SHA1(d7dff7a72041ebb7ff4f56da74cc7eb4b71bc5cc) )
	ROM_CONTINUE(                0x000000, 0x008000 )
	ROM_LOAD( "136051-1106.6rs", 0x028000, 0x008000, CRC(4787ff58) SHA1(35b276eb1ad1ce8b143485502430335c08aa9474) )
	ROM_CONTINUE(                0x008000, 0x008000 )
	ROM_LOAD( "136051-1107.6pr", 0x030000, 0x008000, CRC(0e85f2ac) SHA1(f78769962b4d6f6b4eaac5b6ae8e08ff90ad55ac) )
	ROM_CONTINUE(                0x010000, 0x008000 )
	ROM_LOAD( "136051-1108.6n",  0x038000, 0x008000, CRC(70ff9308) SHA1(c1cdc428058d5a6a9706431d4aa3d2ae81815d9c) )
	ROM_CONTINUE(                0x018000, 0x008000 )
	ROM_LOAD( "136051-1113.6m",  0x060000, 0x008000, CRC(4a445356) SHA1(0aa995bc1234c4b84b72c2e779968b1052cfa6e5) )
	ROM_CONTINUE(                0x040000, 0x008000 )
	ROM_LOAD( "136051-1114.6kl", 0x068000, 0x008000, CRC(b9b27f3c) SHA1(ef3378a2569ccf34360edf6ea1630bed2b83c438) )
	ROM_CONTINUE(                0x048000, 0x008000 )
	ROM_LOAD( "136051-1115.6jk", 0x070000, 0x008000, CRC(a7671dd8) SHA1(faaa088b43f354ad5b5bcf96ed9eab54a2ad162e) )
	ROM_CONTINUE(                0x050000, 0x008000 )
	ROM_LOAD( "136051-1116.6h",  0x078000, 0x008000, CRC(879fc7de) SHA1(4a5994898d546fa914d5eb6e5a1a9c4c2febea5a) )
	ROM_CONTINUE(                0x058000, 0x008000 )
	ROM_LOAD( "136051-1101.5t",  0x0a0000, 0x008000, CRC(0ef13513) SHA1(ee9c1088f203c607b10c9e7927c8c4a29d727726) )
	ROM_CONTINUE(                0x080000, 0x008000 )
	ROM_LOAD( "136051-1102.5rs", 0x0a8000, 0x008000, CRC(401e06fd) SHA1(a0c3a90fd400d146b7b9cc12a8dfaa5db0e4426a) )
	ROM_CONTINUE(                0x088000, 0x008000 )
	ROM_LOAD( "136051-1103.5pr", 0x0b0000, 0x008000, CRC(50d820e8) SHA1(28473949570d402754737c6d38de2a096f804676) )
	ROM_CONTINUE(                0x090000, 0x008000 )
	ROM_LOAD( "136051-1104.5n",  0x0b8000, 0x008000, CRC(912d878f) SHA1(87e0eb5910ec7042628378bcfd58d9b7beca690d) )
	ROM_CONTINUE(                0x098000, 0x008000 )
	ROM_LOAD( "136051-1109.5m",  0x0e0000, 0x008000, CRC(6716a408) SHA1(cb05fa401456177170c32958dbcfec95667944cd) )
	ROM_CONTINUE(                0x0c0000, 0x008000 )
	ROM_LOAD( "136051-1110.5kl", 0x0e8000, 0x008000, CRC(7e184981) SHA1(475b5c3224ee2e5b5330cc2b4ad018b6bcf0abd5) )
	ROM_CONTINUE(                0x0c8000, 0x008000 )
	ROM_LOAD( "136051-1111.5jk", 0x0f0000, 0x008000, CRC(353a14fd) SHA1(6464b089395ce52f26a604357a4783fe5c673362) )
	ROM_CONTINUE(                0x0d0000, 0x008000 )
	ROM_LOAD( "136051-1112.5h",  0x0f8000, 0x008000, CRC(3af7c50f) SHA1(e100ec8a8aee643d3175a8f54cc7e9266cf50e42) )
	ROM_CONTINUE(                0x0d8000, 0x008000 )

	ROM_REGION( 0x4000, "chars", 0 )
	ROM_LOAD( "136051-1125.4t",  0x000000, 0x004000, CRC(05a0341c) SHA1(90b96e0645a01939c681a7a5fe5d236f3dfc71b7) )
ROM_END


ROM_START( apb5 )
	ROM_REGION( 0x90000, "maincpu", 0 )     // 9 * 64k T11 code
	ROM_LOAD16_BYTE( "136051-2126.7l",  0x008000, 0x004000, CRC(8edf4726) SHA1(48ee833c84808abe0eb8b7932abf082af09066da) )
	ROM_LOAD16_BYTE( "136051-2127.7n",  0x008001, 0x004000, CRC(e2b2aff2) SHA1(bded098d56d68a95333eb68bc0c9629a2257aa16) )
	ROM_LOAD16_BYTE( "136051-5128.6f",  0x010000, 0x010000, CRC(4b4ff365) SHA1(89b203c03501a8762b24baa7dc911eaa76e259b3) )
	ROM_LOAD16_BYTE( "136051-5129.6n",  0x010001, 0x010000, CRC(059ab792) SHA1(9712efc5a19b8bef6fc5f8befce284e63537a3ce) )
	ROM_LOAD16_BYTE( "136051-1130.6j",  0x030000, 0x010000, CRC(f64c752e) SHA1(a0a7a0629b228ffd2a0c92725305b8d094484ddb) )
	ROM_LOAD16_BYTE( "136051-1131.6p",  0x030001, 0x010000, CRC(0a506e04) SHA1(98ff0de15632397e7371d8473b0174251b1ef9a1) )
	ROM_LOAD16_BYTE( "136051-1132.6l",  0x070000, 0x010000, CRC(6d0e7a4e) SHA1(75aae74571c50d36639d0ae69b0614e5aedeb6e3) )
	ROM_LOAD16_BYTE( "136051-1133.6s",  0x070001, 0x010000, CRC(af88d429) SHA1(432720afd4179d3df871226e0eb576d2ffde44c1) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     // 64k for 6502 code
	ROM_LOAD( "136051-5134.2a",  0x004000, 0x004000, CRC(1c8bdeed) SHA1(bbdbbb9a4903f98842d9a697410a2e3a2069284c) )
	ROM_LOAD( "136051-5135.2bc", 0x008000, 0x004000, CRC(ed6adb91) SHA1(b1f1f0d1bda445a53de798fb6847c605afe53e3c) )
	ROM_LOAD( "136051-5136.2d",  0x00c000, 0x004000, CRC(341f8486) SHA1(4cea39c0d8551ce7193e51de341f7297a94b8d9b) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "136051-1118.6a",  0x000000, 0x008000, CRC(93752c49) SHA1(6b4ed2defab85ab4d4943bfdf43c04dd42cc2ec5) )
	ROM_LOAD( "136051-1120.6bc", 0x028000, 0x008000, CRC(043086f8) SHA1(8920c8aab37d7b0ecfa17381c65cc00b9b99d4da) )
	ROM_CONTINUE(                0x008000, 0x008000 )
	ROM_LOAD( "136051-1122.7a",  0x030000, 0x008000, CRC(5ee79481) SHA1(82a0eee49cdfe08aeb8619bc7b83c915a8de95c7) )
	ROM_CONTINUE(                0x010000, 0x008000 )
	ROM_LOAD( "136051-1124.7bc", 0x038000, 0x008000, CRC(27760395) SHA1(5c6201743d973389f7ed13ec4253c2034ab23c62) )
	ROM_CONTINUE(                0x018000, 0x008000 )
	ROM_LOAD( "136051-1117.6cd", 0x040000, 0x008000, CRC(cfc3f8a3) SHA1(7fed2a40c2ba28444c5f5470d0b7c86198469651) )
	ROM_LOAD( "136051-1119.6de", 0x068000, 0x008000, CRC(68850612) SHA1(de899a0eb0b9fdbc96ccd3987d1dc942b54f2cc6) )
	ROM_CONTINUE(                0x048000, 0x008000 )
	ROM_LOAD( "136051-1121.7de", 0x070000, 0x008000, CRC(c7977062) SHA1(3a2f8da5c4cd7693575ad13246c6dbb6e2f02131) )
	ROM_CONTINUE(                0x050000, 0x008000 )
	ROM_LOAD( "136051-1123.7cd", 0x078000, 0x008000, CRC(3c96c848) SHA1(a90a0c14ee5d5cdf60a1d3ecd9984b74c31b9f36) )
	ROM_CONTINUE(                0x058000, 0x008000 )

	ROM_REGION( 0x100000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "136051-1105.6t",  0x020000, 0x008000, CRC(9b78a88e) SHA1(d7dff7a72041ebb7ff4f56da74cc7eb4b71bc5cc) )
	ROM_CONTINUE(                0x000000, 0x008000 )
	ROM_LOAD( "136051-1106.6rs", 0x028000, 0x008000, CRC(4787ff58) SHA1(35b276eb1ad1ce8b143485502430335c08aa9474) )
	ROM_CONTINUE(                0x008000, 0x008000 )
	ROM_LOAD( "136051-1107.6pr", 0x030000, 0x008000, CRC(0e85f2ac) SHA1(f78769962b4d6f6b4eaac5b6ae8e08ff90ad55ac) )
	ROM_CONTINUE(                0x010000, 0x008000 )
	ROM_LOAD( "136051-1108.6n",  0x038000, 0x008000, CRC(70ff9308) SHA1(c1cdc428058d5a6a9706431d4aa3d2ae81815d9c) )
	ROM_CONTINUE(                0x018000, 0x008000 )
	ROM_LOAD( "136051-1113.6m",  0x060000, 0x008000, CRC(4a445356) SHA1(0aa995bc1234c4b84b72c2e779968b1052cfa6e5) )
	ROM_CONTINUE(                0x040000, 0x008000 )
	ROM_LOAD( "136051-1114.6kl", 0x068000, 0x008000, CRC(b9b27f3c) SHA1(ef3378a2569ccf34360edf6ea1630bed2b83c438) )
	ROM_CONTINUE(                0x048000, 0x008000 )
	ROM_LOAD( "136051-1115.6jk", 0x070000, 0x008000, CRC(a7671dd8) SHA1(faaa088b43f354ad5b5bcf96ed9eab54a2ad162e) )
	ROM_CONTINUE(                0x050000, 0x008000 )
	ROM_LOAD( "136051-1116.6h",  0x078000, 0x008000, CRC(879fc7de) SHA1(4a5994898d546fa914d5eb6e5a1a9c4c2febea5a) )
	ROM_CONTINUE(                0x058000, 0x008000 )
	ROM_LOAD( "136051-1101.5t",  0x0a0000, 0x008000, CRC(0ef13513) SHA1(ee9c1088f203c607b10c9e7927c8c4a29d727726) )
	ROM_CONTINUE(                0x080000, 0x008000 )
	ROM_LOAD( "136051-1102.5rs", 0x0a8000, 0x008000, CRC(401e06fd) SHA1(a0c3a90fd400d146b7b9cc12a8dfaa5db0e4426a) )
	ROM_CONTINUE(                0x088000, 0x008000 )
	ROM_LOAD( "136051-1103.5pr", 0x0b0000, 0x008000, CRC(50d820e8) SHA1(28473949570d402754737c6d38de2a096f804676) )
	ROM_CONTINUE(                0x090000, 0x008000 )
	ROM_LOAD( "136051-1104.5n",  0x0b8000, 0x008000, CRC(912d878f) SHA1(87e0eb5910ec7042628378bcfd58d9b7beca690d) )
	ROM_CONTINUE(                0x098000, 0x008000 )
	ROM_LOAD( "136051-1109.5m",  0x0e0000, 0x008000, CRC(6716a408) SHA1(cb05fa401456177170c32958dbcfec95667944cd) )
	ROM_CONTINUE(                0x0c0000, 0x008000 )
	ROM_LOAD( "136051-1110.5kl", 0x0e8000, 0x008000, CRC(7e184981) SHA1(475b5c3224ee2e5b5330cc2b4ad018b6bcf0abd5) )
	ROM_CONTINUE(                0x0c8000, 0x008000 )
	ROM_LOAD( "136051-1111.5jk", 0x0f0000, 0x008000, CRC(353a14fd) SHA1(6464b089395ce52f26a604357a4783fe5c673362) )
	ROM_CONTINUE(                0x0d0000, 0x008000 )
	ROM_LOAD( "136051-1112.5h",  0x0f8000, 0x008000, CRC(3af7c50f) SHA1(e100ec8a8aee643d3175a8f54cc7e9266cf50e42) )
	ROM_CONTINUE(                0x0d8000, 0x008000 )

	ROM_REGION( 0x4000, "chars", 0 )
	ROM_LOAD( "136051-1125.4t",  0x000000, 0x004000, CRC(05a0341c) SHA1(90b96e0645a01939c681a7a5fe5d236f3dfc71b7) )
ROM_END


ROM_START( apb4 )
	ROM_REGION( 0x90000, "maincpu", 0 )     // 9 * 64k T11 code
	ROM_LOAD16_BYTE( "136051-2126.7l",  0x008000, 0x004000, CRC(8edf4726) SHA1(48ee833c84808abe0eb8b7932abf082af09066da) )
	ROM_LOAD16_BYTE( "136051-2127.7n",  0x008001, 0x004000, CRC(e2b2aff2) SHA1(bded098d56d68a95333eb68bc0c9629a2257aa16) )
	ROM_LOAD16_BYTE( "136051-4128.6f",  0x010000, 0x010000, CRC(46009f6b) SHA1(344cbb6cc5c1ad5c1aec26ca12cafaf73305d801) )
	ROM_LOAD16_BYTE( "136051-4129.6n",  0x010001, 0x010000, CRC(e8ca47e2) SHA1(16d705e55a373d6e88792881ffa01e084faf58a9) )
	ROM_LOAD16_BYTE( "136051-1130.6j",  0x030000, 0x010000, CRC(f64c752e) SHA1(a0a7a0629b228ffd2a0c92725305b8d094484ddb) )
	ROM_LOAD16_BYTE( "136051-1131.6p",  0x030001, 0x010000, CRC(0a506e04) SHA1(98ff0de15632397e7371d8473b0174251b1ef9a1) )
	ROM_LOAD16_BYTE( "136051-1132.6l",  0x070000, 0x010000, CRC(6d0e7a4e) SHA1(75aae74571c50d36639d0ae69b0614e5aedeb6e3) )
	ROM_LOAD16_BYTE( "136051-1133.6s",  0x070001, 0x010000, CRC(af88d429) SHA1(432720afd4179d3df871226e0eb576d2ffde44c1) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     // 64k for 6502 code
	ROM_LOAD( "136051-5134.2a",  0x004000, 0x004000, CRC(1c8bdeed) SHA1(bbdbbb9a4903f98842d9a697410a2e3a2069284c) )
	ROM_LOAD( "136051-5135.2bc", 0x008000, 0x004000, CRC(ed6adb91) SHA1(b1f1f0d1bda445a53de798fb6847c605afe53e3c) )
	ROM_LOAD( "136051-5136.2d",  0x00c000, 0x004000, CRC(341f8486) SHA1(4cea39c0d8551ce7193e51de341f7297a94b8d9b) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "136051-1118.6a",  0x000000, 0x008000, CRC(93752c49) SHA1(6b4ed2defab85ab4d4943bfdf43c04dd42cc2ec5) )
	ROM_LOAD( "136051-1120.6bc", 0x028000, 0x008000, CRC(043086f8) SHA1(8920c8aab37d7b0ecfa17381c65cc00b9b99d4da) )
	ROM_CONTINUE(                0x008000, 0x008000 )
	ROM_LOAD( "136051-1122.7a",  0x030000, 0x008000, CRC(5ee79481) SHA1(82a0eee49cdfe08aeb8619bc7b83c915a8de95c7) )
	ROM_CONTINUE(                0x010000, 0x008000 )
	ROM_LOAD( "136051-1124.7bc", 0x038000, 0x008000, CRC(27760395) SHA1(5c6201743d973389f7ed13ec4253c2034ab23c62) )
	ROM_CONTINUE(                0x018000, 0x008000 )
	ROM_LOAD( "136051-1117.6cd", 0x040000, 0x008000, CRC(cfc3f8a3) SHA1(7fed2a40c2ba28444c5f5470d0b7c86198469651) )
	ROM_LOAD( "136051-1119.6de", 0x068000, 0x008000, CRC(68850612) SHA1(de899a0eb0b9fdbc96ccd3987d1dc942b54f2cc6) )
	ROM_CONTINUE(                0x048000, 0x008000 )
	ROM_LOAD( "136051-1121.7de", 0x070000, 0x008000, CRC(c7977062) SHA1(3a2f8da5c4cd7693575ad13246c6dbb6e2f02131) )
	ROM_CONTINUE(                0x050000, 0x008000 )
	ROM_LOAD( "136051-1123.7cd", 0x078000, 0x008000, CRC(3c96c848) SHA1(a90a0c14ee5d5cdf60a1d3ecd9984b74c31b9f36) )
	ROM_CONTINUE(                0x058000, 0x008000 )

	ROM_REGION( 0x100000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "136051-1105.6t",  0x020000, 0x008000, CRC(9b78a88e) SHA1(d7dff7a72041ebb7ff4f56da74cc7eb4b71bc5cc) )
	ROM_CONTINUE(                0x000000, 0x008000 )
	ROM_LOAD( "136051-1106.6rs", 0x028000, 0x008000, CRC(4787ff58) SHA1(35b276eb1ad1ce8b143485502430335c08aa9474) )
	ROM_CONTINUE(                0x008000, 0x008000 )
	ROM_LOAD( "136051-1107.6pr", 0x030000, 0x008000, CRC(0e85f2ac) SHA1(f78769962b4d6f6b4eaac5b6ae8e08ff90ad55ac) )
	ROM_CONTINUE(                0x010000, 0x008000 )
	ROM_LOAD( "136051-1108.6n",  0x038000, 0x008000, CRC(70ff9308) SHA1(c1cdc428058d5a6a9706431d4aa3d2ae81815d9c) )
	ROM_CONTINUE(                0x018000, 0x008000 )
	ROM_LOAD( "136051-1113.6m",  0x060000, 0x008000, CRC(4a445356) SHA1(0aa995bc1234c4b84b72c2e779968b1052cfa6e5) )
	ROM_CONTINUE(                0x040000, 0x008000 )
	ROM_LOAD( "136051-1114.6kl", 0x068000, 0x008000, CRC(b9b27f3c) SHA1(ef3378a2569ccf34360edf6ea1630bed2b83c438) )
	ROM_CONTINUE(                0x048000, 0x008000 )
	ROM_LOAD( "136051-1115.6jk", 0x070000, 0x008000, CRC(a7671dd8) SHA1(faaa088b43f354ad5b5bcf96ed9eab54a2ad162e) )
	ROM_CONTINUE(                0x050000, 0x008000 )
	ROM_LOAD( "136051-1116.6h",  0x078000, 0x008000, CRC(879fc7de) SHA1(4a5994898d546fa914d5eb6e5a1a9c4c2febea5a) )
	ROM_CONTINUE(                0x058000, 0x008000 )
	ROM_LOAD( "136051-1101.5t",  0x0a0000, 0x008000, CRC(0ef13513) SHA1(ee9c1088f203c607b10c9e7927c8c4a29d727726) )
	ROM_CONTINUE(                0x080000, 0x008000 )
	ROM_LOAD( "136051-1102.5rs", 0x0a8000, 0x008000, CRC(401e06fd) SHA1(a0c3a90fd400d146b7b9cc12a8dfaa5db0e4426a) )
	ROM_CONTINUE(                0x088000, 0x008000 )
	ROM_LOAD( "136051-1103.5pr", 0x0b0000, 0x008000, CRC(50d820e8) SHA1(28473949570d402754737c6d38de2a096f804676) )
	ROM_CONTINUE(                0x090000, 0x008000 )
	ROM_LOAD( "136051-1104.5n",  0x0b8000, 0x008000, CRC(912d878f) SHA1(87e0eb5910ec7042628378bcfd58d9b7beca690d) )
	ROM_CONTINUE(                0x098000, 0x008000 )
	ROM_LOAD( "136051-1109.5m",  0x0e0000, 0x008000, CRC(6716a408) SHA1(cb05fa401456177170c32958dbcfec95667944cd) )
	ROM_CONTINUE(                0x0c0000, 0x008000 )
	ROM_LOAD( "136051-1110.5kl", 0x0e8000, 0x008000, CRC(7e184981) SHA1(475b5c3224ee2e5b5330cc2b4ad018b6bcf0abd5) )
	ROM_CONTINUE(                0x0c8000, 0x008000 )
	ROM_LOAD( "136051-1111.5jk", 0x0f0000, 0x008000, CRC(353a14fd) SHA1(6464b089395ce52f26a604357a4783fe5c673362) )
	ROM_CONTINUE(                0x0d0000, 0x008000 )
	ROM_LOAD( "136051-1112.5h",  0x0f8000, 0x008000, CRC(3af7c50f) SHA1(e100ec8a8aee643d3175a8f54cc7e9266cf50e42) )
	ROM_CONTINUE(                0x0d8000, 0x008000 )

	ROM_REGION( 0x4000, "chars", 0 )
	ROM_LOAD( "136051-1125.4t",  0x000000, 0x004000, CRC(05a0341c) SHA1(90b96e0645a01939c681a7a5fe5d236f3dfc71b7) )
ROM_END


ROM_START( apb3 )
	ROM_REGION( 0x90000, "maincpu", 0 )     // 9 * 64k T11 code
	ROM_LOAD16_BYTE( "136051-2126.7l",  0x008000, 0x004000, CRC(8edf4726) SHA1(48ee833c84808abe0eb8b7932abf082af09066da) )
	ROM_LOAD16_BYTE( "136051-2127.7n",  0x008001, 0x004000, CRC(e2b2aff2) SHA1(bded098d56d68a95333eb68bc0c9629a2257aa16) )
	ROM_LOAD16_BYTE( "136051-3128.6f",  0x010000, 0x010000, CRC(cbdbfb42) SHA1(2ae94264122d20903e760225468929396a7c855c) )
	ROM_LOAD16_BYTE( "136051-3129.6n",  0x010001, 0x010000, CRC(14d1cc8d) SHA1(1df1d6d3af9a8ef46ea22a0a22748ebb248361f0) )
	ROM_LOAD16_BYTE( "136051-1130.6j",  0x030000, 0x010000, CRC(f64c752e) SHA1(a0a7a0629b228ffd2a0c92725305b8d094484ddb) )
	ROM_LOAD16_BYTE( "136051-1131.6p",  0x030001, 0x010000, CRC(0a506e04) SHA1(98ff0de15632397e7371d8473b0174251b1ef9a1) )
	ROM_LOAD16_BYTE( "136051-1132.6l",  0x070000, 0x010000, CRC(6d0e7a4e) SHA1(75aae74571c50d36639d0ae69b0614e5aedeb6e3) )
	ROM_LOAD16_BYTE( "136051-1133.6s",  0x070001, 0x010000, CRC(af88d429) SHA1(432720afd4179d3df871226e0eb576d2ffde44c1) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     // 64k for 6502 code
	ROM_LOAD( "136051-1134.2a",  0x004000, 0x004000, CRC(a65748b9) SHA1(20d51300382543f09e47bee7273b9074e5a4618a) )
	ROM_LOAD( "136051-1135.2bc", 0x008000, 0x004000, CRC(e9692cea) SHA1(2b2d9638e012d326777e2e730e28cbacea6d9a72) )
	ROM_LOAD( "136051-1136.2d",  0x00c000, 0x004000, CRC(92fc7657) SHA1(cfda3a191a5f7ee4157f9d226bcf3dd601cabee1) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "136051-1118.6a",  0x000000, 0x008000, CRC(93752c49) SHA1(6b4ed2defab85ab4d4943bfdf43c04dd42cc2ec5) )
	ROM_LOAD( "136051-1120.6bc", 0x028000, 0x008000, CRC(043086f8) SHA1(8920c8aab37d7b0ecfa17381c65cc00b9b99d4da) )
	ROM_CONTINUE(                0x008000, 0x008000 )
	ROM_LOAD( "136051-1122.7a",  0x030000, 0x008000, CRC(5ee79481) SHA1(82a0eee49cdfe08aeb8619bc7b83c915a8de95c7) )
	ROM_CONTINUE(                0x010000, 0x008000 )
	ROM_LOAD( "136051-1124.7bc", 0x038000, 0x008000, CRC(27760395) SHA1(5c6201743d973389f7ed13ec4253c2034ab23c62) )
	ROM_CONTINUE(                0x018000, 0x008000 )
	ROM_LOAD( "136051-1117.6cd", 0x040000, 0x008000, CRC(cfc3f8a3) SHA1(7fed2a40c2ba28444c5f5470d0b7c86198469651) )
	ROM_LOAD( "136051-1119.6de", 0x068000, 0x008000, CRC(68850612) SHA1(de899a0eb0b9fdbc96ccd3987d1dc942b54f2cc6) )
	ROM_CONTINUE(                0x048000, 0x008000 )
	ROM_LOAD( "136051-1121.7de", 0x070000, 0x008000, CRC(c7977062) SHA1(3a2f8da5c4cd7693575ad13246c6dbb6e2f02131) )
	ROM_CONTINUE(                0x050000, 0x008000 )
	ROM_LOAD( "136051-1123.7cd", 0x078000, 0x008000, CRC(3c96c848) SHA1(a90a0c14ee5d5cdf60a1d3ecd9984b74c31b9f36) )
	ROM_CONTINUE(                0x058000, 0x008000 )

	ROM_REGION( 0x100000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "136051-1105.6t",  0x020000, 0x008000, CRC(9b78a88e) SHA1(d7dff7a72041ebb7ff4f56da74cc7eb4b71bc5cc) )
	ROM_CONTINUE(                0x000000, 0x008000 )
	ROM_LOAD( "136051-1106.6rs", 0x028000, 0x008000, CRC(4787ff58) SHA1(35b276eb1ad1ce8b143485502430335c08aa9474) )
	ROM_CONTINUE(                0x008000, 0x008000 )
	ROM_LOAD( "136051-1107.6pr", 0x030000, 0x008000, CRC(0e85f2ac) SHA1(f78769962b4d6f6b4eaac5b6ae8e08ff90ad55ac) )
	ROM_CONTINUE(                0x010000, 0x008000 )
	ROM_LOAD( "136051-1108.6n",  0x038000, 0x008000, CRC(70ff9308) SHA1(c1cdc428058d5a6a9706431d4aa3d2ae81815d9c) )
	ROM_CONTINUE(                0x018000, 0x008000 )
	ROM_LOAD( "136051-1113.6m",  0x060000, 0x008000, CRC(4a445356) SHA1(0aa995bc1234c4b84b72c2e779968b1052cfa6e5) )
	ROM_CONTINUE(                0x040000, 0x008000 )
	ROM_LOAD( "136051-1114.6kl", 0x068000, 0x008000, CRC(b9b27f3c) SHA1(ef3378a2569ccf34360edf6ea1630bed2b83c438) )
	ROM_CONTINUE(                0x048000, 0x008000 )
	ROM_LOAD( "136051-1115.6jk", 0x070000, 0x008000, CRC(a7671dd8) SHA1(faaa088b43f354ad5b5bcf96ed9eab54a2ad162e) )
	ROM_CONTINUE(                0x050000, 0x008000 )
	ROM_LOAD( "136051-1116.6h",  0x078000, 0x008000, CRC(879fc7de) SHA1(4a5994898d546fa914d5eb6e5a1a9c4c2febea5a) )
	ROM_CONTINUE(                0x058000, 0x008000 )
	ROM_LOAD( "136051-1101.5t",  0x0a0000, 0x008000, CRC(0ef13513) SHA1(ee9c1088f203c607b10c9e7927c8c4a29d727726) )
	ROM_CONTINUE(                0x080000, 0x008000 )
	ROM_LOAD( "136051-1102.5rs", 0x0a8000, 0x008000, CRC(401e06fd) SHA1(a0c3a90fd400d146b7b9cc12a8dfaa5db0e4426a) )
	ROM_CONTINUE(                0x088000, 0x008000 )
	ROM_LOAD( "136051-1103.5pr", 0x0b0000, 0x008000, CRC(50d820e8) SHA1(28473949570d402754737c6d38de2a096f804676) )
	ROM_CONTINUE(                0x090000, 0x008000 )
	ROM_LOAD( "136051-1104.5n",  0x0b8000, 0x008000, CRC(912d878f) SHA1(87e0eb5910ec7042628378bcfd58d9b7beca690d) )
	ROM_CONTINUE(                0x098000, 0x008000 )
	ROM_LOAD( "136051-1109.5m",  0x0e0000, 0x008000, CRC(6716a408) SHA1(cb05fa401456177170c32958dbcfec95667944cd) )
	ROM_CONTINUE(                0x0c0000, 0x008000 )
	ROM_LOAD( "136051-1110.5kl", 0x0e8000, 0x008000, CRC(7e184981) SHA1(475b5c3224ee2e5b5330cc2b4ad018b6bcf0abd5) )
	ROM_CONTINUE(                0x0c8000, 0x008000 )
	ROM_LOAD( "136051-1111.5jk", 0x0f0000, 0x008000, CRC(353a14fd) SHA1(6464b089395ce52f26a604357a4783fe5c673362) )
	ROM_CONTINUE(                0x0d0000, 0x008000 )
	ROM_LOAD( "136051-1112.5h",  0x0f8000, 0x008000, CRC(3af7c50f) SHA1(e100ec8a8aee643d3175a8f54cc7e9266cf50e42) )
	ROM_CONTINUE(                0x0d8000, 0x008000 )

	ROM_REGION( 0x4000, "chars", 0 )
	ROM_LOAD( "136051-1125.4t",  0x000000, 0x004000, CRC(05a0341c) SHA1(90b96e0645a01939c681a7a5fe5d236f3dfc71b7) )
ROM_END


ROM_START( apb2 )
	ROM_REGION( 0x90000, "maincpu", 0 )     // 9 * 64k T11 code
	ROM_LOAD16_BYTE( "136051-2126.7l",  0x008000, 0x004000, CRC(8edf4726) SHA1(48ee833c84808abe0eb8b7932abf082af09066da) )
	ROM_LOAD16_BYTE( "136051-2127.7n",  0x008001, 0x004000, CRC(e2b2aff2) SHA1(bded098d56d68a95333eb68bc0c9629a2257aa16) )
	ROM_LOAD16_BYTE( "136051-2128.6f",  0x010000, 0x010000, CRC(61a81436) SHA1(d4b964e4b1a801c9b4ed984d8b20a988d3a1f485) )
	ROM_LOAD16_BYTE( "136051-2129.6n",  0x010001, 0x010000, CRC(24500ed6) SHA1(3b73f4567d5b8430766df01f7ac65f1f1fb5dc1a) )
	ROM_LOAD16_BYTE( "136051-1130.6j",  0x030000, 0x010000, CRC(f64c752e) SHA1(a0a7a0629b228ffd2a0c92725305b8d094484ddb) )
	ROM_LOAD16_BYTE( "136051-1131.6p",  0x030001, 0x010000, CRC(0a506e04) SHA1(98ff0de15632397e7371d8473b0174251b1ef9a1) )
	ROM_LOAD16_BYTE( "136051-1132.6l",  0x070000, 0x010000, CRC(6d0e7a4e) SHA1(75aae74571c50d36639d0ae69b0614e5aedeb6e3) )
	ROM_LOAD16_BYTE( "136051-1133.6s",  0x070001, 0x010000, CRC(af88d429) SHA1(432720afd4179d3df871226e0eb576d2ffde44c1) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     // 64k for 6502 code
	ROM_LOAD( "136051-1134.2a",  0x004000, 0x004000, CRC(a65748b9) SHA1(20d51300382543f09e47bee7273b9074e5a4618a) )
	ROM_LOAD( "136051-1135.2bc", 0x008000, 0x004000, CRC(e9692cea) SHA1(2b2d9638e012d326777e2e730e28cbacea6d9a72) )
	ROM_LOAD( "136051-1136.2d",  0x00c000, 0x004000, CRC(92fc7657) SHA1(cfda3a191a5f7ee4157f9d226bcf3dd601cabee1) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "136051-1118.6a",  0x000000, 0x008000, CRC(93752c49) SHA1(6b4ed2defab85ab4d4943bfdf43c04dd42cc2ec5) )
	ROM_LOAD( "136051-1120.6bc", 0x028000, 0x008000, CRC(043086f8) SHA1(8920c8aab37d7b0ecfa17381c65cc00b9b99d4da) )
	ROM_CONTINUE(                0x008000, 0x008000 )
	ROM_LOAD( "136051-1122.7a",  0x030000, 0x008000, CRC(5ee79481) SHA1(82a0eee49cdfe08aeb8619bc7b83c915a8de95c7) )
	ROM_CONTINUE(                0x010000, 0x008000 )
	ROM_LOAD( "136051-1124.7bc", 0x038000, 0x008000, CRC(27760395) SHA1(5c6201743d973389f7ed13ec4253c2034ab23c62) )
	ROM_CONTINUE(                0x018000, 0x008000 )
	ROM_LOAD( "136051-1117.6cd", 0x040000, 0x008000, CRC(cfc3f8a3) SHA1(7fed2a40c2ba28444c5f5470d0b7c86198469651) )
	ROM_LOAD( "136051-1119.6de", 0x068000, 0x008000, CRC(68850612) SHA1(de899a0eb0b9fdbc96ccd3987d1dc942b54f2cc6) )
	ROM_CONTINUE(                0x048000, 0x008000 )
	ROM_LOAD( "136051-1121.7de", 0x070000, 0x008000, CRC(c7977062) SHA1(3a2f8da5c4cd7693575ad13246c6dbb6e2f02131) )
	ROM_CONTINUE(                0x050000, 0x008000 )
	ROM_LOAD( "136051-1123.7cd", 0x078000, 0x008000, CRC(3c96c848) SHA1(a90a0c14ee5d5cdf60a1d3ecd9984b74c31b9f36) )
	ROM_CONTINUE(                0x058000, 0x008000 )

	ROM_REGION( 0x100000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "136051-1105.6t",  0x020000, 0x008000, CRC(9b78a88e) SHA1(d7dff7a72041ebb7ff4f56da74cc7eb4b71bc5cc) )
	ROM_CONTINUE(                0x000000, 0x008000 )
	ROM_LOAD( "136051-1106.6rs", 0x028000, 0x008000, CRC(4787ff58) SHA1(35b276eb1ad1ce8b143485502430335c08aa9474) )
	ROM_CONTINUE(                0x008000, 0x008000 )
	ROM_LOAD( "136051-1107.6pr", 0x030000, 0x008000, CRC(0e85f2ac) SHA1(f78769962b4d6f6b4eaac5b6ae8e08ff90ad55ac) )
	ROM_CONTINUE(                0x010000, 0x008000 )
	ROM_LOAD( "136051-1108.6n",  0x038000, 0x008000, CRC(70ff9308) SHA1(c1cdc428058d5a6a9706431d4aa3d2ae81815d9c) )
	ROM_CONTINUE(                0x018000, 0x008000 )
	ROM_LOAD( "136051-1113.6m",  0x060000, 0x008000, CRC(4a445356) SHA1(0aa995bc1234c4b84b72c2e779968b1052cfa6e5) )
	ROM_CONTINUE(                0x040000, 0x008000 )
	ROM_LOAD( "136051-1114.6kl", 0x068000, 0x008000, CRC(b9b27f3c) SHA1(ef3378a2569ccf34360edf6ea1630bed2b83c438) )
	ROM_CONTINUE(                0x048000, 0x008000 )
	ROM_LOAD( "136051-1115.6jk", 0x070000, 0x008000, CRC(a7671dd8) SHA1(faaa088b43f354ad5b5bcf96ed9eab54a2ad162e) )
	ROM_CONTINUE(                0x050000, 0x008000 )
	ROM_LOAD( "136051-1116.6h",  0x078000, 0x008000, CRC(879fc7de) SHA1(4a5994898d546fa914d5eb6e5a1a9c4c2febea5a) )
	ROM_CONTINUE(                0x058000, 0x008000 )
	ROM_LOAD( "136051-1101.5t",  0x0a0000, 0x008000, CRC(0ef13513) SHA1(ee9c1088f203c607b10c9e7927c8c4a29d727726) )
	ROM_CONTINUE(                0x080000, 0x008000 )
	ROM_LOAD( "136051-1102.5rs", 0x0a8000, 0x008000, CRC(401e06fd) SHA1(a0c3a90fd400d146b7b9cc12a8dfaa5db0e4426a) )
	ROM_CONTINUE(                0x088000, 0x008000 )
	ROM_LOAD( "136051-1103.5pr", 0x0b0000, 0x008000, CRC(50d820e8) SHA1(28473949570d402754737c6d38de2a096f804676) )
	ROM_CONTINUE(                0x090000, 0x008000 )
	ROM_LOAD( "136051-1104.5n",  0x0b8000, 0x008000, CRC(912d878f) SHA1(87e0eb5910ec7042628378bcfd58d9b7beca690d) )
	ROM_CONTINUE(                0x098000, 0x008000 )
	ROM_LOAD( "136051-1109.5m",  0x0e0000, 0x008000, CRC(6716a408) SHA1(cb05fa401456177170c32958dbcfec95667944cd) )
	ROM_CONTINUE(                0x0c0000, 0x008000 )
	ROM_LOAD( "136051-1110.5kl", 0x0e8000, 0x008000, CRC(7e184981) SHA1(475b5c3224ee2e5b5330cc2b4ad018b6bcf0abd5) )
	ROM_CONTINUE(                0x0c8000, 0x008000 )
	ROM_LOAD( "136051-1111.5jk", 0x0f0000, 0x008000, CRC(353a14fd) SHA1(6464b089395ce52f26a604357a4783fe5c673362) )
	ROM_CONTINUE(                0x0d0000, 0x008000 )
	ROM_LOAD( "136051-1112.5h",  0x0f8000, 0x008000, CRC(3af7c50f) SHA1(e100ec8a8aee643d3175a8f54cc7e9266cf50e42) )
	ROM_CONTINUE(                0x0d8000, 0x008000 )

	ROM_REGION( 0x4000, "chars", 0 )
	ROM_LOAD( "136051-1125.4t",  0x000000, 0x004000, CRC(05a0341c) SHA1(90b96e0645a01939c681a7a5fe5d236f3dfc71b7) )
ROM_END


ROM_START( apb1 )
	ROM_REGION( 0x90000, "maincpu", 0 )     // 9 * 64k T11 code
	ROM_LOAD16_BYTE( "136051-1126.7l",  0x008000, 0x004000, CRC(d385994c) SHA1(647ceccc4b434fca5a58e64193a369defc3d9c49) )
	ROM_LOAD16_BYTE( "136051-1127.7n",  0x008001, 0x004000, CRC(9b40b0b4) SHA1(e23885e66ce2ad437db6e02313d2db11f3498bba) )
	ROM_LOAD16_BYTE( "136051-1128.6f",  0x010000, 0x010000, CRC(8d5d9f4a) SHA1(96dcbd034f431dfad424331e0854a0c418988055) )
	ROM_LOAD16_BYTE( "136051-1129.6n",  0x010001, 0x010000, CRC(2948cef0) SHA1(74f06b548bdf11ce24b105e95f1ad63ece61e17f) )
	ROM_LOAD16_BYTE( "136051-1130.6j",  0x030000, 0x010000, CRC(f64c752e) SHA1(a0a7a0629b228ffd2a0c92725305b8d094484ddb) )
	ROM_LOAD16_BYTE( "136051-1131.6p",  0x030001, 0x010000, CRC(0a506e04) SHA1(98ff0de15632397e7371d8473b0174251b1ef9a1) )
	ROM_LOAD16_BYTE( "136051-1132.6l",  0x070000, 0x010000, CRC(6d0e7a4e) SHA1(75aae74571c50d36639d0ae69b0614e5aedeb6e3) )
	ROM_LOAD16_BYTE( "136051-1133.6s",  0x070001, 0x010000, CRC(af88d429) SHA1(432720afd4179d3df871226e0eb576d2ffde44c1) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     // 64k for 6502 code
	ROM_LOAD( "136051-1134.2a",  0x004000, 0x004000, CRC(a65748b9) SHA1(20d51300382543f09e47bee7273b9074e5a4618a) )
	ROM_LOAD( "136051-1135.2bc", 0x008000, 0x004000, CRC(e9692cea) SHA1(2b2d9638e012d326777e2e730e28cbacea6d9a72) )
	ROM_LOAD( "136051-1136.2d",  0x00c000, 0x004000, CRC(92fc7657) SHA1(cfda3a191a5f7ee4157f9d226bcf3dd601cabee1) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "136051-1118.6a",  0x000000, 0x008000, CRC(93752c49) SHA1(6b4ed2defab85ab4d4943bfdf43c04dd42cc2ec5) )
	ROM_LOAD( "136051-1120.6bc", 0x028000, 0x008000, CRC(043086f8) SHA1(8920c8aab37d7b0ecfa17381c65cc00b9b99d4da) )
	ROM_CONTINUE(                0x008000, 0x008000 )
	ROM_LOAD( "136051-1122.7a",  0x030000, 0x008000, CRC(5ee79481) SHA1(82a0eee49cdfe08aeb8619bc7b83c915a8de95c7) )
	ROM_CONTINUE(                0x010000, 0x008000 )
	ROM_LOAD( "136051-1124.7bc", 0x038000, 0x008000, CRC(27760395) SHA1(5c6201743d973389f7ed13ec4253c2034ab23c62) )
	ROM_CONTINUE(                0x018000, 0x008000 )
	ROM_LOAD( "136051-1117.6cd", 0x040000, 0x008000, CRC(cfc3f8a3) SHA1(7fed2a40c2ba28444c5f5470d0b7c86198469651) )
	ROM_LOAD( "136051-1119.6de", 0x068000, 0x008000, CRC(68850612) SHA1(de899a0eb0b9fdbc96ccd3987d1dc942b54f2cc6) )
	ROM_CONTINUE(                0x048000, 0x008000 )
	ROM_LOAD( "136051-1121.7de", 0x070000, 0x008000, CRC(c7977062) SHA1(3a2f8da5c4cd7693575ad13246c6dbb6e2f02131) )
	ROM_CONTINUE(                0x050000, 0x008000 )
	ROM_LOAD( "136051-1123.7cd", 0x078000, 0x008000, CRC(3c96c848) SHA1(a90a0c14ee5d5cdf60a1d3ecd9984b74c31b9f36) )
	ROM_CONTINUE(                0x058000, 0x008000 )

	ROM_REGION( 0x100000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "136051-1105.6t",  0x020000, 0x008000, CRC(9b78a88e) SHA1(d7dff7a72041ebb7ff4f56da74cc7eb4b71bc5cc) )
	ROM_CONTINUE(                0x000000, 0x008000 )
	ROM_LOAD( "136051-1106.6rs", 0x028000, 0x008000, CRC(4787ff58) SHA1(35b276eb1ad1ce8b143485502430335c08aa9474) )
	ROM_CONTINUE(                0x008000, 0x008000 )
	ROM_LOAD( "136051-1107.6pr", 0x030000, 0x008000, CRC(0e85f2ac) SHA1(f78769962b4d6f6b4eaac5b6ae8e08ff90ad55ac) )
	ROM_CONTINUE(                0x010000, 0x008000 )
	ROM_LOAD( "136051-1108.6n",  0x038000, 0x008000, CRC(70ff9308) SHA1(c1cdc428058d5a6a9706431d4aa3d2ae81815d9c) )
	ROM_CONTINUE(                0x018000, 0x008000 )
	ROM_LOAD( "136051-1113.6m",  0x060000, 0x008000, CRC(4a445356) SHA1(0aa995bc1234c4b84b72c2e779968b1052cfa6e5) )
	ROM_CONTINUE(                0x040000, 0x008000 )
	ROM_LOAD( "136051-1114.6kl", 0x068000, 0x008000, CRC(b9b27f3c) SHA1(ef3378a2569ccf34360edf6ea1630bed2b83c438) )
	ROM_CONTINUE(                0x048000, 0x008000 )
	ROM_LOAD( "136051-1115.6jk", 0x070000, 0x008000, CRC(a7671dd8) SHA1(faaa088b43f354ad5b5bcf96ed9eab54a2ad162e) )
	ROM_CONTINUE(                0x050000, 0x008000 )
	ROM_LOAD( "136051-1116.6h",  0x078000, 0x008000, CRC(879fc7de) SHA1(4a5994898d546fa914d5eb6e5a1a9c4c2febea5a) )
	ROM_CONTINUE(                0x058000, 0x008000 )
	ROM_LOAD( "136051-1101.5t",  0x0a0000, 0x008000, CRC(0ef13513) SHA1(ee9c1088f203c607b10c9e7927c8c4a29d727726) )
	ROM_CONTINUE(                0x080000, 0x008000 )
	ROM_LOAD( "136051-1102.5rs", 0x0a8000, 0x008000, CRC(401e06fd) SHA1(a0c3a90fd400d146b7b9cc12a8dfaa5db0e4426a) )
	ROM_CONTINUE(                0x088000, 0x008000 )
	ROM_LOAD( "136051-1103.5pr", 0x0b0000, 0x008000, CRC(50d820e8) SHA1(28473949570d402754737c6d38de2a096f804676) )
	ROM_CONTINUE(                0x090000, 0x008000 )
	ROM_LOAD( "136051-1104.5n",  0x0b8000, 0x008000, CRC(912d878f) SHA1(87e0eb5910ec7042628378bcfd58d9b7beca690d) )
	ROM_CONTINUE(                0x098000, 0x008000 )
	ROM_LOAD( "136051-1109.5m",  0x0e0000, 0x008000, CRC(6716a408) SHA1(cb05fa401456177170c32958dbcfec95667944cd) )
	ROM_CONTINUE(                0x0c0000, 0x008000 )
	ROM_LOAD( "136051-1110.5kl", 0x0e8000, 0x008000, CRC(7e184981) SHA1(475b5c3224ee2e5b5330cc2b4ad018b6bcf0abd5) )
	ROM_CONTINUE(                0x0c8000, 0x008000 )
	ROM_LOAD( "136051-1111.5jk", 0x0f0000, 0x008000, CRC(353a14fd) SHA1(6464b089395ce52f26a604357a4783fe5c673362) )
	ROM_CONTINUE(                0x0d0000, 0x008000 )
	ROM_LOAD( "136051-1112.5h",  0x0f8000, 0x008000, CRC(3af7c50f) SHA1(e100ec8a8aee643d3175a8f54cc7e9266cf50e42) )
	ROM_CONTINUE(                0x0d8000, 0x008000 )

	ROM_REGION( 0x4000, "chars", 0 )
	ROM_LOAD( "136051-1125.4t",  0x000000, 0x004000, CRC(05a0341c) SHA1(90b96e0645a01939c681a7a5fe5d236f3dfc71b7) )
ROM_END


ROM_START( apbg )
	ROM_REGION( 0x90000, "maincpu", 0 )     // 9 * 64k T11 code
	ROM_LOAD16_BYTE( "136051-2126.7l",  0x008000, 0x004000, CRC(8edf4726) SHA1(48ee833c84808abe0eb8b7932abf082af09066da) )
	ROM_LOAD16_BYTE( "136051-2127.7n",  0x008001, 0x004000, CRC(e2b2aff2) SHA1(bded098d56d68a95333eb68bc0c9629a2257aa16) )
	ROM_LOAD16_BYTE( "136051-1228.6f",  0x010000, 0x010000, CRC(44781913) SHA1(46659edbbbe664a22d3dbef63a62757d87403365) )
	ROM_LOAD16_BYTE( "136051-1229.6n",  0x010001, 0x010000, CRC(f18afffd) SHA1(3f34b2a1fa738fcbc45ac7873f98c8a0f34832d3) )
	ROM_LOAD16_BYTE( "136051-1130.6j",  0x030000, 0x010000, CRC(f64c752e) SHA1(a0a7a0629b228ffd2a0c92725305b8d094484ddb) )
	ROM_LOAD16_BYTE( "136051-1131.6p",  0x030001, 0x010000, CRC(0a506e04) SHA1(98ff0de15632397e7371d8473b0174251b1ef9a1) )
	ROM_LOAD16_BYTE( "136051-1132.6l",  0x070000, 0x010000, CRC(6d0e7a4e) SHA1(75aae74571c50d36639d0ae69b0614e5aedeb6e3) )
	ROM_LOAD16_BYTE( "136051-1133.6s",  0x070001, 0x010000, CRC(af88d429) SHA1(432720afd4179d3df871226e0eb576d2ffde44c1) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     // 64k for 6502 code
	ROM_LOAD( "136051-4134.2a",  0x004000, 0x004000, CRC(45e03b0e) SHA1(d58f5e1095fd6a7c0253fcc9f0e55812c1677387) )
	ROM_LOAD( "136051-4135.2bc", 0x008000, 0x004000, CRC(b4ca24b2) SHA1(86461da310b334f6b32c0e079de6852792284cc6) )
	ROM_LOAD( "136051-4136.2d",  0x00c000, 0x004000, CRC(11efaabf) SHA1(76446b09bf7cacd713ab88d58793460c9d1a8b9b) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "136051-1118.6a",  0x000000, 0x008000, CRC(93752c49) SHA1(6b4ed2defab85ab4d4943bfdf43c04dd42cc2ec5) )
	ROM_LOAD( "136051-1120.6bc", 0x028000, 0x008000, CRC(043086f8) SHA1(8920c8aab37d7b0ecfa17381c65cc00b9b99d4da) )
	ROM_CONTINUE(                0x008000, 0x008000 )
	ROM_LOAD( "136051-1122.7a",  0x030000, 0x008000, CRC(5ee79481) SHA1(82a0eee49cdfe08aeb8619bc7b83c915a8de95c7) )
	ROM_CONTINUE(                0x010000, 0x008000 )
	ROM_LOAD( "136051-1124.7bc", 0x038000, 0x008000, CRC(27760395) SHA1(5c6201743d973389f7ed13ec4253c2034ab23c62) )
	ROM_CONTINUE(                0x018000, 0x008000 )
	ROM_LOAD( "136051-1117.6cd", 0x040000, 0x008000, CRC(cfc3f8a3) SHA1(7fed2a40c2ba28444c5f5470d0b7c86198469651) )
	ROM_LOAD( "136051-1119.6de", 0x068000, 0x008000, CRC(68850612) SHA1(de899a0eb0b9fdbc96ccd3987d1dc942b54f2cc6) )
	ROM_CONTINUE(                0x048000, 0x008000 )
	ROM_LOAD( "136051-1121.7de", 0x070000, 0x008000, CRC(c7977062) SHA1(3a2f8da5c4cd7693575ad13246c6dbb6e2f02131) )
	ROM_CONTINUE(                0x050000, 0x008000 )
	ROM_LOAD( "136051-1123.7cd", 0x078000, 0x008000, CRC(3c96c848) SHA1(a90a0c14ee5d5cdf60a1d3ecd9984b74c31b9f36) )
	ROM_CONTINUE(                0x058000, 0x008000 )

	ROM_REGION( 0x100000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "136051-1105.6t",  0x020000, 0x008000, CRC(9b78a88e) SHA1(d7dff7a72041ebb7ff4f56da74cc7eb4b71bc5cc) )
	ROM_CONTINUE(                0x000000, 0x008000 )
	ROM_LOAD( "136051-1106.6rs", 0x028000, 0x008000, CRC(4787ff58) SHA1(35b276eb1ad1ce8b143485502430335c08aa9474) )
	ROM_CONTINUE(                0x008000, 0x008000 )
	ROM_LOAD( "136051-1107.6pr", 0x030000, 0x008000, CRC(0e85f2ac) SHA1(f78769962b4d6f6b4eaac5b6ae8e08ff90ad55ac) )
	ROM_CONTINUE(                0x010000, 0x008000 )
	ROM_LOAD( "136051-1108.6n",  0x038000, 0x008000, CRC(70ff9308) SHA1(c1cdc428058d5a6a9706431d4aa3d2ae81815d9c) )
	ROM_CONTINUE(                0x018000, 0x008000 )
	ROM_LOAD( "136051-1113.6m",  0x060000, 0x008000, CRC(4a445356) SHA1(0aa995bc1234c4b84b72c2e779968b1052cfa6e5) )
	ROM_CONTINUE(                0x040000, 0x008000 )
	ROM_LOAD( "136051-1114.6kl", 0x068000, 0x008000, CRC(b9b27f3c) SHA1(ef3378a2569ccf34360edf6ea1630bed2b83c438) )
	ROM_CONTINUE(                0x048000, 0x008000 )
	ROM_LOAD( "136051-1115.6jk", 0x070000, 0x008000, CRC(a7671dd8) SHA1(faaa088b43f354ad5b5bcf96ed9eab54a2ad162e) )
	ROM_CONTINUE(                0x050000, 0x008000 )
	ROM_LOAD( "136051-1116.6h",  0x078000, 0x008000, CRC(879fc7de) SHA1(4a5994898d546fa914d5eb6e5a1a9c4c2febea5a) )
	ROM_CONTINUE(                0x058000, 0x008000 )
	ROM_LOAD( "136051-1101.5t",  0x0a0000, 0x008000, CRC(0ef13513) SHA1(ee9c1088f203c607b10c9e7927c8c4a29d727726) )
	ROM_CONTINUE(                0x080000, 0x008000 )
	ROM_LOAD( "136051-1102.5rs", 0x0a8000, 0x008000, CRC(401e06fd) SHA1(a0c3a90fd400d146b7b9cc12a8dfaa5db0e4426a) )
	ROM_CONTINUE(                0x088000, 0x008000 )
	ROM_LOAD( "136051-1103.5pr", 0x0b0000, 0x008000, CRC(50d820e8) SHA1(28473949570d402754737c6d38de2a096f804676) )
	ROM_CONTINUE(                0x090000, 0x008000 )
	ROM_LOAD( "136051-1104.5n",  0x0b8000, 0x008000, CRC(912d878f) SHA1(87e0eb5910ec7042628378bcfd58d9b7beca690d) )
	ROM_CONTINUE(                0x098000, 0x008000 )
	ROM_LOAD( "136051-1109.5m",  0x0e0000, 0x008000, CRC(6716a408) SHA1(cb05fa401456177170c32958dbcfec95667944cd) )
	ROM_CONTINUE(                0x0c0000, 0x008000 )
	ROM_LOAD( "136051-1110.5kl", 0x0e8000, 0x008000, CRC(7e184981) SHA1(475b5c3224ee2e5b5330cc2b4ad018b6bcf0abd5) )
	ROM_CONTINUE(                0x0c8000, 0x008000 )
	ROM_LOAD( "136051-1111.5jk", 0x0f0000, 0x008000, CRC(353a14fd) SHA1(6464b089395ce52f26a604357a4783fe5c673362) )
	ROM_CONTINUE(                0x0d0000, 0x008000 )
	ROM_LOAD( "136051-1112.5h",  0x0f8000, 0x008000, CRC(3af7c50f) SHA1(e100ec8a8aee643d3175a8f54cc7e9266cf50e42) )
	ROM_CONTINUE(                0x0d8000, 0x008000 )

	ROM_REGION( 0x4000, "chars", 0 )
	ROM_LOAD( "136051-1125.4t",  0x000000, 0x004000, CRC(05a0341c) SHA1(90b96e0645a01939c681a7a5fe5d236f3dfc71b7) )
ROM_END


ROM_START( apbf )
	ROM_REGION( 0x90000, "maincpu", 0 )     // 9 * 64k T11 code
	ROM_LOAD16_BYTE( "136051-2126.7l",  0x008000, 0x004000, CRC(8edf4726) SHA1(48ee833c84808abe0eb8b7932abf082af09066da) )
	ROM_LOAD16_BYTE( "136051-2127.7n",  0x008001, 0x004000, CRC(e2b2aff2) SHA1(bded098d56d68a95333eb68bc0c9629a2257aa16) )
	ROM_LOAD16_BYTE( "136051-1628.6f",  0x010000, 0x010000, CRC(075e9a18) SHA1(48376a51210f5967b355ea2ed83e967ac28de455) )
	ROM_LOAD16_BYTE( "136051-1629.6n",  0x010001, 0x010000, CRC(8951514a) SHA1(d5aaf7543adba2298f1faf25e57e144829e93b5d) )
	ROM_LOAD16_BYTE( "136051-1130.6j",  0x030000, 0x010000, CRC(f64c752e) SHA1(a0a7a0629b228ffd2a0c92725305b8d094484ddb) )
	ROM_LOAD16_BYTE( "136051-1131.6p",  0x030001, 0x010000, CRC(0a506e04) SHA1(98ff0de15632397e7371d8473b0174251b1ef9a1) )
	ROM_LOAD16_BYTE( "136051-1132.6l",  0x070000, 0x010000, CRC(6d0e7a4e) SHA1(75aae74571c50d36639d0ae69b0614e5aedeb6e3) )
	ROM_LOAD16_BYTE( "136051-1133.6s",  0x070001, 0x010000, CRC(af88d429) SHA1(432720afd4179d3df871226e0eb576d2ffde44c1) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     // 64k for 6502 code
	ROM_LOAD( "136051-5134.2a",  0x004000, 0x004000, CRC(1c8bdeed) SHA1(bbdbbb9a4903f98842d9a697410a2e3a2069284c) )
	ROM_LOAD( "136051-5135.2bc", 0x008000, 0x004000, CRC(ed6adb91) SHA1(b1f1f0d1bda445a53de798fb6847c605afe53e3c) )
	ROM_LOAD( "136051-5136.2d",  0x00c000, 0x004000, CRC(341f8486) SHA1(4cea39c0d8551ce7193e51de341f7297a94b8d9b) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "136051-1118.6a",  0x000000, 0x008000, CRC(93752c49) SHA1(6b4ed2defab85ab4d4943bfdf43c04dd42cc2ec5) )
	ROM_LOAD( "136051-1120.6bc", 0x028000, 0x008000, CRC(043086f8) SHA1(8920c8aab37d7b0ecfa17381c65cc00b9b99d4da) )
	ROM_CONTINUE(                0x008000, 0x008000 )
	ROM_LOAD( "136051-1122.7a",  0x030000, 0x008000, CRC(5ee79481) SHA1(82a0eee49cdfe08aeb8619bc7b83c915a8de95c7) )
	ROM_CONTINUE(                0x010000, 0x008000 )
	ROM_LOAD( "136051-1124.7bc", 0x038000, 0x008000, CRC(27760395) SHA1(5c6201743d973389f7ed13ec4253c2034ab23c62) )
	ROM_CONTINUE(                0x018000, 0x008000 )
	ROM_LOAD( "136051-1117.6cd", 0x040000, 0x008000, CRC(cfc3f8a3) SHA1(7fed2a40c2ba28444c5f5470d0b7c86198469651) )
	ROM_LOAD( "136051-1119.6de", 0x068000, 0x008000, CRC(68850612) SHA1(de899a0eb0b9fdbc96ccd3987d1dc942b54f2cc6) )
	ROM_CONTINUE(                0x048000, 0x008000 )
	ROM_LOAD( "136051-1121.7de", 0x070000, 0x008000, CRC(c7977062) SHA1(3a2f8da5c4cd7693575ad13246c6dbb6e2f02131) )
	ROM_CONTINUE(                0x050000, 0x008000 )
	ROM_LOAD( "136051-1123.7cd", 0x078000, 0x008000, CRC(3c96c848) SHA1(a90a0c14ee5d5cdf60a1d3ecd9984b74c31b9f36) )
	ROM_CONTINUE(                0x058000, 0x008000 )

	ROM_REGION( 0x100000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "136051-1105.6t",  0x020000, 0x008000, CRC(9b78a88e) SHA1(d7dff7a72041ebb7ff4f56da74cc7eb4b71bc5cc) )
	ROM_CONTINUE(                0x000000, 0x008000 )
	ROM_LOAD( "136051-1106.6rs", 0x028000, 0x008000, CRC(4787ff58) SHA1(35b276eb1ad1ce8b143485502430335c08aa9474) )
	ROM_CONTINUE(                0x008000, 0x008000 )
	ROM_LOAD( "136051-1107.6pr", 0x030000, 0x008000, CRC(0e85f2ac) SHA1(f78769962b4d6f6b4eaac5b6ae8e08ff90ad55ac) )
	ROM_CONTINUE(                0x010000, 0x008000 )
	ROM_LOAD( "136051-1108.6n",  0x038000, 0x008000, CRC(70ff9308) SHA1(c1cdc428058d5a6a9706431d4aa3d2ae81815d9c) )
	ROM_CONTINUE(                0x018000, 0x008000 )
	ROM_LOAD( "136051-1113.6m",  0x060000, 0x008000, CRC(4a445356) SHA1(0aa995bc1234c4b84b72c2e779968b1052cfa6e5) )
	ROM_CONTINUE(                0x040000, 0x008000 )
	ROM_LOAD( "136051-1114.6kl", 0x068000, 0x008000, CRC(b9b27f3c) SHA1(ef3378a2569ccf34360edf6ea1630bed2b83c438) )
	ROM_CONTINUE(                0x048000, 0x008000 )
	ROM_LOAD( "136051-1115.6jk", 0x070000, 0x008000, CRC(a7671dd8) SHA1(faaa088b43f354ad5b5bcf96ed9eab54a2ad162e) )
	ROM_CONTINUE(                0x050000, 0x008000 )
	ROM_LOAD( "136051-1116.6h",  0x078000, 0x008000, CRC(879fc7de) SHA1(4a5994898d546fa914d5eb6e5a1a9c4c2febea5a) )
	ROM_CONTINUE(                0x058000, 0x008000 )
	ROM_LOAD( "136051-1101.5t",  0x0a0000, 0x008000, CRC(0ef13513) SHA1(ee9c1088f203c607b10c9e7927c8c4a29d727726) )
	ROM_CONTINUE(                0x080000, 0x008000 )
	ROM_LOAD( "136051-1102.5rs", 0x0a8000, 0x008000, CRC(401e06fd) SHA1(a0c3a90fd400d146b7b9cc12a8dfaa5db0e4426a) )
	ROM_CONTINUE(                0x088000, 0x008000 )
	ROM_LOAD( "136051-1103.5pr", 0x0b0000, 0x008000, CRC(50d820e8) SHA1(28473949570d402754737c6d38de2a096f804676) )
	ROM_CONTINUE(                0x090000, 0x008000 )
	ROM_LOAD( "136051-1104.5n",  0x0b8000, 0x008000, CRC(912d878f) SHA1(87e0eb5910ec7042628378bcfd58d9b7beca690d) )
	ROM_CONTINUE(                0x098000, 0x008000 )
	ROM_LOAD( "136051-1109.5m",  0x0e0000, 0x008000, CRC(6716a408) SHA1(cb05fa401456177170c32958dbcfec95667944cd) )
	ROM_CONTINUE(                0x0c0000, 0x008000 )
	ROM_LOAD( "136051-1110.5kl", 0x0e8000, 0x008000, CRC(7e184981) SHA1(475b5c3224ee2e5b5330cc2b4ad018b6bcf0abd5) )
	ROM_CONTINUE(                0x0c8000, 0x008000 )
	ROM_LOAD( "136051-1111.5jk", 0x0f0000, 0x008000, CRC(353a14fd) SHA1(6464b089395ce52f26a604357a4783fe5c673362) )
	ROM_CONTINUE(                0x0d0000, 0x008000 )
	ROM_LOAD( "136051-1112.5h",  0x0f8000, 0x008000, CRC(3af7c50f) SHA1(e100ec8a8aee643d3175a8f54cc7e9266cf50e42) )
	ROM_CONTINUE(                0x0d8000, 0x008000 )

	ROM_REGION( 0x4000, "chars", 0 )
	ROM_LOAD( "136051-1125.4t",  0x000000, 0x004000, CRC(05a0341c) SHA1(90b96e0645a01939c681a7a5fe5d236f3dfc71b7) )
ROM_END


/*************************************
 *
 *  Driver initialization
 *
 *************************************/

void atarisy2_state::init_paperboy()
{
	uint8_t *cpu1 = memregion("maincpu")->base();

	// expand the 16k program ROMs into full 64k chunks
	for (int i = 0x10000; i < 0x90000; i += 0x20000)
	{
		memcpy(&cpu1[i + 0x08000], &cpu1[i], 0x8000);
		memcpy(&cpu1[i + 0x10000], &cpu1[i], 0x8000);
		memcpy(&cpu1[i + 0x18000], &cpu1[i], 0x8000);
	}

	m_pedal_count = 0;
	m_tms5220->rsq_w(1); // /RS is tied high on sys2 hw
}


void atarisy2_state::init_720()
{
	m_pedal_count = -1;
	m_tms5220->rsq_w(1); // /RS is tied high on sys2 hw
}


void atarisy2_state::init_ssprint()
{
	uint8_t *cpu1 = memregion("maincpu")->base();

	// expand the 32k program ROMs into full 64k chunks
	for (int i = 0x10000; i < 0x90000; i += 0x20000)
		memcpy(&cpu1[i + 0x10000], &cpu1[i], 0x10000);

	m_pedal_count = 3;
}


void atarisy2_state::init_csprint()
{
	uint8_t *cpu1 = memregion("maincpu")->base();

	// expand the 32k program ROMs into full 64k chunks
	for (int i = 0x10000; i < 0x90000; i += 0x20000)
		memcpy(&cpu1[i + 0x10000], &cpu1[i], 0x10000);

	m_pedal_count = 2;
}


void atarisy2_state::init_apb()
{
	m_pedal_count = 2;
	m_tms5220->rsq_w(1); // /RS is tied high on sys2 hw
}


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1984, paperboy,   0,        paperboy, paperboy, atarisy2_state, init_paperboy, ROT0,   "Atari Games", "Paperboy (rev 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, paperboyr2, paperboy, paperboy, paperboy, atarisy2_state, init_paperboy, ROT0,   "Atari Games", "Paperboy (rev 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, paperboyr1, paperboy, paperboy, paperboy, atarisy2_state, init_paperboy, ROT0,   "Atari Games", "Paperboy (rev 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, paperboyp,  paperboy, paperboy, paperboy, atarisy2_state, init_paperboy, ROT0,   "Atari Games", "Paperboy (prototype)", MACHINE_NOT_WORKING )

GAME( 1986, 720,        0,        _720,     720,      atarisy2_state, init_720,      ROT0,   "Atari Games", "720 Degrees (rev 4)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, 720r3,      720,      _720,     720,      atarisy2_state, init_720,      ROT0,   "Atari Games", "720 Degrees (rev 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, 720r2,      720,      _720,     720,      atarisy2_state, init_720,      ROT0,   "Atari Games", "720 Degrees (rev 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, 720r1,      720,      _720,     720,      atarisy2_state, init_720,      ROT0,   "Atari Games", "720 Degrees (rev 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, 720g,       720,      _720,     720,      atarisy2_state, init_720,      ROT0,   "Atari Games", "720 Degrees (German, rev 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, 720gr1,     720,      _720,     720,      atarisy2_state, init_720,      ROT0,   "Atari Games", "720 Degrees (German, rev 1)", MACHINE_SUPPORTS_SAVE )

GAME( 1986, ssprint,    0,        ssprint,  ssprint,  atarisy2_state, init_ssprint,  ROT0,   "Atari Games", "Super Sprint (rev 4)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, ssprint3,   ssprint,  ssprint,  ssprint,  atarisy2_state, init_ssprint,  ROT0,   "Atari Games", "Super Sprint (rev 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, ssprint1,   ssprint,  ssprint,  ssprint,  atarisy2_state, init_ssprint,  ROT0,   "Atari Games", "Super Sprint (rev 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, ssprintg,   ssprint,  ssprint,  ssprint,  atarisy2_state, init_ssprint,  ROT0,   "Atari Games", "Super Sprint (German, rev 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, ssprintg1,  ssprint,  ssprint,  ssprint,  atarisy2_state, init_ssprint,  ROT0,   "Atari Games", "Super Sprint (German, rev 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, ssprintf,   ssprint,  ssprint,  ssprint,  atarisy2_state, init_ssprint,  ROT0,   "Atari Games", "Super Sprint (French)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, ssprints,   ssprint,  ssprint,  ssprint,  atarisy2_state, init_ssprint,  ROT0,   "Atari Games", "Super Sprint (Spanish)", MACHINE_SUPPORTS_SAVE )

GAME( 1986, csprint,    0,        csprint,  csprint,  atarisy2_state, init_csprint,  ROT0,   "Atari Games", "Championship Sprint (rev 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, csprint2,   csprint,  csprint,  csprint,  atarisy2_state, init_csprint,  ROT0,   "Atari Games", "Championship Sprint (rev 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, csprint1,   csprint,  csprint,  csprint,  atarisy2_state, init_csprint,  ROT0,   "Atari Games", "Championship Sprint (rev 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, csprintg,   csprint,  csprint,  csprint,  atarisy2_state, init_csprint,  ROT0,   "Atari Games", "Championship Sprint (German, rev 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, csprintg1,  csprint,  csprint,  csprint,  atarisy2_state, init_csprint,  ROT0,   "Atari Games", "Championship Sprint (German, rev 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, csprintf,   csprint,  csprint,  csprint,  atarisy2_state, init_csprint,  ROT0,   "Atari Games", "Championship Sprint (French)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, csprints,   csprint,  csprint,  csprint,  atarisy2_state, init_csprint,  ROT0,   "Atari Games", "Championship Sprint (Spanish, rev 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, csprints1,  csprint,  csprint,  csprint,  atarisy2_state, init_csprint,  ROT0,   "Atari Games", "Championship Sprint (Spanish, rev 1)", MACHINE_SUPPORTS_SAVE )

GAME( 1987, apb,        0,        apb,      apb,      atarisy2_state, init_apb,      ROT270, "Atari Games", "APB - All Points Bulletin (rev 7)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, apb6,       apb,      apb,      apb,      atarisy2_state, init_apb,      ROT270, "Atari Games", "APB - All Points Bulletin (rev 6)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, apb5,       apb,      apb,      apb,      atarisy2_state, init_apb,      ROT270, "Atari Games", "APB - All Points Bulletin (rev 5)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, apb4,       apb,      apb,      apb,      atarisy2_state, init_apb,      ROT270, "Atari Games", "APB - All Points Bulletin (rev 4)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, apb3,       apb,      apb,      apb,      atarisy2_state, init_apb,      ROT270, "Atari Games", "APB - All Points Bulletin (rev 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, apb2,       apb,      apb,      apb,      atarisy2_state, init_apb,      ROT270, "Atari Games", "APB - All Points Bulletin (rev 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, apb1,       apb,      apb,      apb,      atarisy2_state, init_apb,      ROT270, "Atari Games", "APB - All Points Bulletin (rev 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, apbg,       apb,      apb,      apb,      atarisy2_state, init_apb,      ROT270, "Atari Games", "APB - All Points Bulletin (German)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, apbf,       apb,      apb,      apb,      atarisy2_state, init_apb,      ROT270, "Atari Games", "APB - All Points Bulletin (French)", MACHINE_SUPPORTS_SAVE )
