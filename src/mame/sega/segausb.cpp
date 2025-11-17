// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    segausb.cpp

    Sega Universal Sound Board.

***************************************************************************/

#include "emu.h"
#include "segausb.h"
#include "nl_segausb.h"

#include <cmath>


#define VERBOSE 0
#include "logmacro.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define USB_MASTER_CLOCK    6000000
#define USB_2MHZ_CLOCK      (USB_MASTER_CLOCK/3)
#define USB_PCS_CLOCK       (USB_2MHZ_CLOCK/2)
#define USB_GOS_CLOCK       (USB_2MHZ_CLOCK/16/4)
#define MM5837_CLOCK        100000


/***************************************************************************
    UNIVERSAL SOUND BOARD
***************************************************************************/

DEFINE_DEVICE_TYPE(SEGAUSB, usb_sound_device, "segausb", "Sega Universal Sound Board")

usb_sound_device::usb_sound_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock),
		device_mixer_interface(mconfig, *this),
		m_in_latch(0),
		m_out_latch(0),
		m_last_p2_value(0),
		m_program_ram(*this, "pgmram"),
		m_work_ram(*this, "workram", 4*256, ENDIANNESS_LITTLE),
		m_work_ram_bank(0),
		m_t1_clock(0),
		m_t1_clock_mask(0),
		m_ourcpu(*this, "ourcpu"),
		m_maincpu(*this, finder_base::DUMMY_TAG),
#if (ENABLE_SEGAUSB_NETLIST)
		m_pit(*this, "pit_%u", 0),
		m_nl_dac0(*this, "sound_nl:dac0_%u", 0),
		m_nl_sel0(*this, "sound_nl:sel0"),
		m_nl_pit0_out(*this, "sound_nl:pit0_out%u", 0),
		m_nl_dac1(*this, "sound_nl:dac1_%u", 0),
		m_nl_sel1(*this, "sound_nl:sel1"),
		m_nl_pit1_out(*this, "sound_nl:pit1_out%u", 0),
		m_nl_dac2(*this, "sound_nl:dac2_%u", 0),
		m_nl_sel2(*this, "sound_nl:sel2"),
		m_nl_pit2_out(*this, "sound_nl:pit2_out%u", 0),
		m_gos_clock(0)
#else
		m_stream(nullptr),
		m_noise_shift(0),
		m_noise_state(0),
		m_noise_subcount(0)
#endif
{
}

usb_sound_device::usb_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: usb_sound_device(mconfig, SEGAUSB, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void usb_sound_device::device_start()
{
	// register for save states
	save_item(NAME(m_in_latch));
	save_item(NAME(m_out_latch));
	save_item(NAME(m_last_p2_value));
	save_item(NAME(m_work_ram_bank));
	save_item(NAME(m_t1_clock));

#if (ENABLE_SEGAUSB_NETLIST)

	for (int index = 0; index < 3; index++)
	{
		m_pit[index]->write_gate0(1);
		m_pit[index]->write_gate1(1);
		m_pit[index]->write_gate2(1);
	}
	save_item(NAME(m_gos_clock));

#else

	m_stream = stream_alloc(0, 1, USB_2MHZ_CLOCK);

	m_noise_shift = 0x15555;

	for (timer8253 &group : m_timer_group)
	{
		group.chan_filter[0].configure(10e3, 1e-6);
		group.chan_filter[1].configure(10e3, 1e-6);
		group.gate1.configure(100e3, 0.01e-6);
		group.gate2.configure(2 * 100e3, 0.01e-6);
	}

	g80_filter_state temp;
	temp.configure(100e3, 0.01e-6);
	m_gate_rc1_exp[0] = temp.exponent;
	temp.configure(1e3, 0.01e-6);
	m_gate_rc1_exp[1] = temp.exponent;
	temp.configure(2 * 100e3, 0.01e-6);
	m_gate_rc2_exp[0] = temp.exponent;
	temp.configure(2 * 1e3, 0.01e-6);
	m_gate_rc2_exp[1] = temp.exponent;

	m_noise_filters[0].configure(2.7e3 + 2.7e3, 1.0e-6);
	m_noise_filters[1].configure(2.7e3 + 1e3, 0.30e-6);
	m_noise_filters[2].configure(2.7e3 + 270, 0.15e-6);
	m_noise_filters[3].configure(2.7e3 + 0, 0.082e-6);
	m_noise_filters[4].configure(33e3, 0.1e-6);

	m_final_filter.configure(100e3, 4.7e-6);

	for (int tgroup = 0; tgroup < 3; tgroup++)
	{
		timer8253 &group = m_timer_group[tgroup];
		save_item(STRUCT_MEMBER(group.chan, holding), tgroup);
		save_item(STRUCT_MEMBER(group.chan, latchmode), tgroup);
		save_item(STRUCT_MEMBER(group.chan, latchtoggle), tgroup);
		save_item(STRUCT_MEMBER(group.chan, clockmode), tgroup);
		save_item(STRUCT_MEMBER(group.chan, bcdmode), tgroup);
		save_item(STRUCT_MEMBER(group.chan, output), tgroup);
		save_item(STRUCT_MEMBER(group.chan, lastgate), tgroup);
		save_item(STRUCT_MEMBER(group.chan, gate), tgroup);
		save_item(STRUCT_MEMBER(group.chan, subcount), tgroup);
		save_item(STRUCT_MEMBER(group.chan, count), tgroup);
		save_item(STRUCT_MEMBER(group.chan, remain), tgroup);
		save_item(NAME(group.env), tgroup);
		save_item(STRUCT_MEMBER(group.chan_filter, capval), tgroup);
		save_item(NAME(group.gate1.capval), tgroup);
		save_item(NAME(group.gate2.capval), tgroup);
		save_item(NAME(group.config), tgroup);
	}

	save_item(NAME(m_timer_mode));
	save_item(NAME(m_noise_shift));
	save_item(NAME(m_noise_state));
	save_item(NAME(m_noise_subcount));
	save_item(NAME(m_final_filter.capval));
	save_item(NAME(m_noise_filters[0].capval));
	save_item(NAME(m_noise_filters[1].capval));
	save_item(NAME(m_noise_filters[2].capval));
	save_item(NAME(m_noise_filters[3].capval));
	save_item(NAME(m_noise_filters[4].capval));

#endif
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void usb_sound_device::device_reset()
{
	// halt the USB CPU at reset time
	m_ourcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	// start the clock timer
	m_t1_clock_mask = 0x10;
}


/*************************************
 *
 *  Initialization/reset
 *
 *************************************/

TIMER_DEVICE_CALLBACK_MEMBER( usb_sound_device::increment_t1_clock_timer_cb )
{
	// only increment if it is not being forced clear
	if (!(m_last_p2_value & 0x80))
		m_t1_clock++;
}


/*************************************
 *
 *  External access
 *
 *************************************/

u8 usb_sound_device::status_r()
{
	LOG("%s:usb_data_r = %02X\n", machine().describe_context(), (m_out_latch & 0x81) | (m_in_latch & 0x7e));

	if (!machine().side_effects_disabled())
		m_maincpu->adjust_icount(-200);

	// only bits 0 and 7 are controlled by the I8035; the remaining
	// bits 1-6 reflect the current input latch values
	return (m_out_latch & 0x81) | (m_in_latch & 0x7e);
}


TIMER_CALLBACK_MEMBER( usb_sound_device::delayed_usb_data_w )
{
	// look for rising/falling edges of bit 7 to control the RESET line
	int data = param;
	m_ourcpu->set_input_line(INPUT_LINE_RESET, (data & 0x80) ? ASSERT_LINE : CLEAR_LINE);

	// if the CLEAR line is set, the low 7 bits of the input are ignored
	if ((m_last_p2_value & 0x40) == 0)
		data &= ~0x7f;

	// update the effective input latch
	m_in_latch = data;
}


void usb_sound_device::data_w(u8 data)
{
	LOG("%s:usb_data_w = %02X\n", machine().describe_context(), data);
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(usb_sound_device::delayed_usb_data_w), this), data);

	// boost the interleave so that sequences can be sent
	machine().scheduler().perfect_quantum(attotime::from_usec(250));
}


u8 usb_sound_device::ram_r(offs_t offset)
{
	return m_program_ram[offset];
}


void usb_sound_device::ram_w(offs_t offset, u8 data)
{
	if (m_in_latch & 0x80)
		m_program_ram[offset] = data;
	else
		LOG("%s:sega_usb_ram_w(%03X) = %02X while /LOAD disabled\n", machine().describe_context(), offset, data);
}



/*************************************
 *
 *  I8035 port accesses
 *
 *************************************/

u8 usb_sound_device::p1_r()
{
	// bits 0-6 are inputs and map to bits 0-6 of the input latch
	if ((m_in_latch & 0x7f) != 0)
		LOG("%s: P1 read = %02X\n", machine().describe_context(), m_in_latch & 0x7f);
	return m_in_latch & 0x7f;
}


void usb_sound_device::p1_w(u8 data)
{
	// bit 7 maps to bit 0 on the output latch
	m_out_latch = (m_out_latch & 0xfe) | (data >> 7);
	LOG("%s: P1 write = %02X\n", machine().describe_context(), data);
}


void usb_sound_device::p2_w(u8 data)
{
	u8 old = m_last_p2_value;
	m_last_p2_value = data;

	// low 2 bits control the bank of work RAM we are addressing
	m_work_ram_bank = data & 3;

	// bit 6 controls the "ready" bit output to the host
	// it also clears the input latch from the host (active low)
	m_out_latch = ((data & 0x40) << 1) | (m_out_latch & 0x7f);
	if ((data & 0x40) == 0)
		m_in_latch = 0;

	// bit 7 controls the reset on the upper counter at U33
	if ((old & 0x80) && !(data & 0x80))
		m_t1_clock = 0;

	LOG("%s: P2 write -> bank=%d ready=%d clock=%d\n", machine().describe_context(), data & 3, (data >> 6) & 1, (data >> 7) & 1);
}


int usb_sound_device::t1_r()
{
	// T1 returns 1 based on the value of the T1 clock; the exact
	// pattern is determined by one or more jumpers on the board.
	return (m_t1_clock & m_t1_clock_mask) != 0;
}



/*************************************
 *
 *  Sound generation
 *
 *************************************/

#if (!ENABLE_SEGAUSB_NETLIST)

inline void usb_sound_device::g80_filter_state::configure(double r, double c)
{
	capval = 0.0;
	exponent = 1.0 - std::exp(-1.0 / (r * c * USB_2MHZ_CLOCK));
}

inline void usb_sound_device::timer8253::channel::clock()
{
	u8 const old_lastgate = lastgate;

	// update the gate
	lastgate = gate;

	// if we're holding, skip
	if (holding)
		return;

	// switch off the clock mode
	switch (clockmode)
	{
		// oneshot; waits for trigger to restart
		case 1:
			if (!old_lastgate && gate)
			{
				output = 0;
				remain = count;
			}
			else
			{
				if (--remain == 0)
					output = 1;
			}
			break;

		// square wave: counts down by 2 and toggles output
		case 3:
			remain = (remain - 1) & ~1;
			if (remain == 0)
			{
				output ^= 1;
				remain = count;
			}
			break;
	}
}


/*************************************
 *
 *  USB timer and envelope controls
 *
 *************************************/

void usb_sound_device::timer_w(int which, u8 offset, u8 data)
{
	timer8253 &group = m_timer_group[which];

	m_stream->update();

	// switch off the offset
	switch (offset)
	{
		case 0:
		case 1:
		case 2:
		{
			timer8253::channel &ch = group.chan[offset];
			bool was_holding = ch.holding;

			// based on the latching mode
			switch (ch.latchmode)
			{
				case 1: // low word only
					ch.count = data;
					ch.holding = false;
					break;

				case 2: // high word only
					ch.count = data << 8;
					ch.holding = false;
					break;

				case 3: // low word followed by high word
					if (ch.latchtoggle == 0)
					{
						ch.count = (ch.count & 0xff00) | (data & 0x00ff);
						ch.latchtoggle = 1;
					}
					else
					{
						ch.count = (ch.count & 0x00ff) | (data << 8);
						ch.holding = false;
						ch.latchtoggle = 0;
					}
					break;
			}

			// if we're not holding, load the initial count for some modes
			if (was_holding && !ch.holding)
				ch.remain = 1;
			break;
		}

		case 3:
			// break out the components
			if (((data & 0xc0) >> 6) < 3)
			{
				timer8253::channel &ch = group.chan[(data & 0xc0) >> 6];

				// extract the bits
				ch.holding = true;
				ch.latchmode = (data >> 4) & 3;
				ch.clockmode = (data >> 1) & 7;
				ch.bcdmode = (data >> 0) & 1;
				ch.latchtoggle = 0;
				ch.output = (ch.clockmode == 1);
			}
			break;
	}
}


void usb_sound_device::env_w(int which, u8 offset, u8 data)
{
	timer8253 &group = m_timer_group[which];

	m_stream->update();

	if (offset < 3)
		group.env[offset] = double(data);
	else
		group.config = data & 1;
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void usb_sound_device::sound_stream_update(sound_stream &stream)
{
	// iterate over samples
	for (int sampindex = 0; sampindex < stream.samples(); sampindex++)
	{
		/*----------------
		    Noise Source
		  ----------------

		                 RC
		   MM5837 ---> FILTER ---> CR FILTER ---> 3.2x AMP ---> NOISE
		               LADDER
		*/

		// update the noise source
		if (m_noise_subcount-- == 0)
		{
			m_noise_shift = (m_noise_shift << 1) | (((m_noise_shift >> 13) ^ (m_noise_shift >> 16)) & 1);
			m_noise_state = (m_noise_shift >> 16) & 1;
			m_noise_subcount += USB_2MHZ_CLOCK / MM5837_CLOCK;
		}

		// update the filtered noise value -- this is just an approximation to the pink noise filter
		// being applied on the PCB, but it sounds pretty close
		m_noise_filters[0].capval = 0.99765 * m_noise_filters[0].capval + m_noise_state * 0.0990460;
		m_noise_filters[1].capval = 0.96300 * m_noise_filters[1].capval + m_noise_state * 0.2965164;
		m_noise_filters[2].capval = 0.57000 * m_noise_filters[2].capval + m_noise_state * 1.0526913;
		double noiseval = m_noise_filters[0].capval + m_noise_filters[1].capval + m_noise_filters[2].capval + m_noise_state * 0.1848;

		// final output goes through a CR filter; the scaling factor is arbitrary to get the noise to the
		// correct relative volume
		noiseval = m_noise_filters[4].step_cr(noiseval);
		noiseval *= 0.075;

		// there are 3 identical groups of circuits, each with its own 8253
		double sample = 0;
		for (int groupnum = 0; groupnum < 3; groupnum++)
		{
			timer8253 &group = m_timer_group[groupnum];

			/*-------------
			    Channel 0
			  -------------

			    8253        CR                   AD7524
			    OUT0 ---> FILTER ---> BUFFER--->  VRef  ---> 100k ---> mix
			*/

			// channel 0 clocks with the PCS clock
			if (group.chan[0].subcount-- == 0)
			{
				group.chan[0].subcount += USB_2MHZ_CLOCK / USB_PCS_CLOCK;
				group.chan[0].gate = 1;
				group.chan[0].clock();
			}

			// channel 0 is mixed in with a resistance of 100k
			double chan0 = group.chan_filter[0].step_cr(group.chan[0].output) * group.env[0] * (1.0/100.0);

			/*-------------
			    Channel 1
			  -------------

			    8253        CR                   AD7524
			    OUT1 ---> FILTER ---> BUFFER--->  VRef  ---> 100k ---> mix
			*/

			// channel 1 clocks with the PCS clock
			if (group.chan[1].subcount-- == 0)
			{
				group.chan[1].subcount += USB_2MHZ_CLOCK / USB_PCS_CLOCK;
				group.chan[1].gate = 1;
				group.chan[1].clock();
			}

			// channel 1 is mixed in with a resistance of 100k
			double chan1 = group.chan_filter[1].step_cr(group.chan[1].output) * group.env[1] * (1.0/100.0);

			/*-------------
			    Channel 2
			  -------------

			  If timer_mode == 0:

			               SWITCHED                                  AD7524
			    NOISE --->    RC   ---> 1.56x AMP ---> INVERTER --->  VRef ---> 33k ---> mix
			                FILTERS

			  If timer mode == 1:

			                             AD7524                                    SWITCHED
			    NOISE ---> INVERTER --->  VRef ---> 33k ---> mix ---> INVERTER --->   RC   ---> 1.56x AMP ---> finalmix
			                                                                        FILTERS
			*/

			// channel 2 clocks with the 2MHZ clock and triggers with the GOS clock
			if (group.chan[2].subcount-- == 0)
			{
				group.chan[2].subcount += USB_2MHZ_CLOCK / USB_GOS_CLOCK / 2;
				group.chan[2].gate = !group.chan[2].gate;
			}
			group.chan[2].clock();

			// the exponents for the gate filters are determined by channel 2's output
			group.gate1.exponent = m_gate_rc1_exp[group.chan[2].output];
			group.gate2.exponent = m_gate_rc2_exp[group.chan[2].output];

			// based on the envelope mode, we do one of two things with source 2
			double chan2, mix;
			if (group.config == 0)
			{
				chan2 = group.gate2.step_rc(group.gate1.step_rc(noiseval)) * -1.56 * group.env[2] * (1.0/33.0);
				mix = chan0 + chan1 + chan2;
			}
			else
			{
				chan2 = -noiseval * group.env[2] * (1.0/33.0);
				mix = chan0 + chan1 + chan2;
				mix = group.gate2.step_rc(group.gate1.step_rc(-mix)) * 1.56;
			}

			// accumulate the sample
			sample += mix;
		}

		/*-------------
		    Final mix
		  -------------

		  INPUTS
		  EQUAL ---> 1.2x INVERTER ---> CR FILTER ---> out
		  WEIGHT

		*/
		stream.put(0, sampindex, 0.1 * m_final_filter.step_cr(sample));
	}
}

#endif



/*************************************
 *
 *  USB work RAM access
 *
 *************************************/

u8 usb_sound_device::workram_r(offs_t offset)
{
	offset += 256 * m_work_ram_bank;
	return m_work_ram[offset];
}


void usb_sound_device::workram_w(offs_t offset, u8 data)
{
	offset += 256 * m_work_ram_bank;
	m_work_ram[offset] = data;

	// writes to the low 32 bytes go to various controls
#if (ENABLE_SEGAUSB_NETLIST)

	switch (offset)
	{
		case 0x00:  // 8253 U41
		case 0x01:  // 8253 U41
		case 0x02:  // 8253 U41
		case 0x03:  // 8253 U41
			if ((offset & 3) != 3)
				printf("%s: 2.%d count=%d\n", machine().scheduler().time().as_string(), offset & 3, data);
			m_pit[0]->write(offset & 3, data);
			break;

		case 0x04:  // ENV0 U26
		case 0x05:  // ENV0 U25
		case 0x06:  // ENV0 U24
			m_nl_dac0[offset & 3]->write(double(data) / 255.0);
			break;

		case 0x07:  // ENV0 U38B
			m_nl_sel0->write(data & 1);
			break;

		case 0x08:  // 8253 U42
		case 0x09:  // 8253 U42
		case 0x0a:  // 8253 U42
		case 0x0b:  // 8253 U42
			if ((offset & 3) != 3)
				printf("%s: 2.%d count=%d\n", machine().scheduler().time().as_string(), offset & 3, data);
			m_pit[1]->write(offset & 3, data);
			break;

		case 0x0c:  // ENV1 U12
		case 0x0d:  // ENV1 U13
		case 0x0e:  // ENV1 U14
			m_nl_dac1[offset & 3]->write(double(data) / 255.0);
			break;

		case 0x0f:  // ENV1 U2B
			m_nl_sel1->write(data & 1);
			break;

		case 0x10:  // 8253 U43
		case 0x11:  // 8253 U43
		case 0x12:  // 8253 U43
		case 0x13:  // 8253 U43
			if ((offset & 3) != 3)
				printf("%s: 2.%d count=%d\n", machine().scheduler().time().as_string(), offset & 3, data);
			m_pit[2]->write(offset & 3, data);
			break;

		case 0x14:  // ENV2 U27
		case 0x15:  // ENV2 U28
		case 0x16:  // ENV2 U29
			m_nl_dac2[offset & 3]->write(double(data) / 255.0);
			break;

		case 0x17:  // ENV2 U38B
			m_nl_sel2->write(data & 1);
			break;
	}

#else

	switch (offset & ~3)
	{
		case 0x00:  // CTC0
			timer_w(0, offset & 3, data);
			break;

		case 0x04:  // ENV0
			env_w(0, offset & 3, data);
			break;

		case 0x08:  // CTC1
			timer_w(1, offset & 3, data);
			break;

		case 0x0c:  // ENV1
			env_w(1, offset & 3, data);
			break;

		case 0x10:  // CTC2
			timer_w(2, offset & 3, data);
			break;

		case 0x14:  // ENV2
			env_w(2, offset & 3, data);
			break;
	}

#endif
}



/*************************************
 *
 *  USB address maps
 *
 *************************************/

void usb_sound_device::usb_map(address_map &map)
{
	map(0x0000, 0x0fff).ram().share("pgmram");
}

void usb_sound_device::usb_portmap(address_map &map)
{
	map(0x00, 0xff).rw(FUNC(usb_sound_device::workram_r), FUNC(usb_sound_device::workram_w));
}


//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------

#if (ENABLE_SEGAUSB_NETLIST)

TIMER_DEVICE_CALLBACK_MEMBER( usb_sound_device::gos_timer )
{
	// technically we should clock at 2x and toggle between states
	// however, in practice this is just used to trigger a oneshot
	// so we can halve the high frequency rate and just toggle both
	// ways to initiate the countdown
	m_pit[0]->write_gate2(0);
	m_pit[1]->write_gate2(0);
	m_pit[2]->write_gate2(0);
	m_pit[0]->write_gate2(1);
	m_pit[1]->write_gate2(1);
	m_pit[2]->write_gate2(1);
}

#endif


void usb_sound_device::device_add_mconfig(machine_config &config)
{
	// CPU for the usb board
	I8035(config, m_ourcpu, USB_MASTER_CLOCK);     // divide by 15 in CPU
	m_ourcpu->set_addrmap(AS_PROGRAM, &usb_sound_device::usb_map);
	m_ourcpu->set_addrmap(AS_IO, &usb_sound_device::usb_portmap);
	m_ourcpu->p1_in_cb().set(FUNC(usb_sound_device::p1_r));
	m_ourcpu->p1_out_cb().set(FUNC(usb_sound_device::p1_w));
	m_ourcpu->p2_out_cb().set(FUNC(usb_sound_device::p2_w));
	m_ourcpu->t1_in_cb().set(FUNC(usb_sound_device::t1_r));

	TIMER(config, "usb_timer", 0).configure_periodic(
			FUNC(usb_sound_device::increment_t1_clock_timer_cb),
			attotime::from_hz(USB_2MHZ_CLOCK / 256));

#if (ENABLE_SEGAUSB_NETLIST)

	TIMER(config, "gos_timer", 0).configure_periodic(
			FUNC(usb_sound_device::gos_timer), attotime::from_hz(USB_GOS_CLOCK));

	NETLIST_SOUND(config, "sound_nl", 48000)
		.set_source(NETLIST_NAME(segausb))
		.add_route(ALL_OUTPUTS, *this, 1.0);

	// channel 0 inputs
	NETLIST_ANALOG_INPUT(config, m_nl_dac0[0], "I_U26_DAC.IN");
	NETLIST_ANALOG_INPUT(config, m_nl_dac0[1], "I_U25_DAC.IN");
	NETLIST_ANALOG_INPUT(config, m_nl_dac0[2], "I_U24_DAC.IN");
	NETLIST_LOGIC_INPUT(config, m_nl_sel0, "I_U38B_SEL.IN", 0);
	NETLIST_LOGIC_INPUT(config, m_nl_pit0_out[0], "I_U41_OUT0.IN", 0);
	NETLIST_LOGIC_INPUT(config, m_nl_pit0_out[1], "I_U41_OUT1.IN", 0);
	NETLIST_LOGIC_INPUT(config, m_nl_pit0_out[2], "I_U41_OUT2.IN", 0);

	// channel 1 inputs
	NETLIST_ANALOG_INPUT(config, m_nl_dac1[0], "I_U12_DAC.IN");
	NETLIST_ANALOG_INPUT(config, m_nl_dac1[1], "I_U13_DAC.IN");
	NETLIST_ANALOG_INPUT(config, m_nl_dac1[2], "I_U14_DAC.IN");
	NETLIST_LOGIC_INPUT(config, m_nl_sel1, "I_U2B_SEL.IN", 0);
	NETLIST_LOGIC_INPUT(config, m_nl_pit1_out[0], "I_U42_OUT0.IN", 0);
	NETLIST_LOGIC_INPUT(config, m_nl_pit1_out[1], "I_U42_OUT1.IN", 0);
	NETLIST_LOGIC_INPUT(config, m_nl_pit1_out[2], "I_U42_OUT2.IN", 0);

	// channel 2 inputs
	NETLIST_ANALOG_INPUT(config, m_nl_dac2[0], "I_U27_DAC.IN");
	NETLIST_ANALOG_INPUT(config, m_nl_dac2[1], "I_U28_DAC.IN");
	NETLIST_ANALOG_INPUT(config, m_nl_dac2[2], "I_U29_DAC.IN");
	NETLIST_LOGIC_INPUT(config, m_nl_sel2, "I_U2A_SEL.IN", 0);
	NETLIST_LOGIC_INPUT(config, m_nl_pit2_out[0], "I_U43_OUT0.IN", 0);
	NETLIST_LOGIC_INPUT(config, m_nl_pit2_out[1], "I_U43_OUT1.IN", 0);
	NETLIST_LOGIC_INPUT(config, m_nl_pit2_out[2], "I_U43_OUT2.IN", 0);

	// final output
	NETLIST_STREAM_OUTPUT(config, "sound_nl:cout0", 0, "OUTPUT").set_mult_offset(5000.0, 0.0);

	// configure the PIT clocks and gates
	for (int index = 0; index < 3; index++)
	{
		PIT8253(config, m_pit[index], 0);
		m_pit[index]->set_clk<0>(USB_PCS_CLOCK);
		m_pit[index]->set_clk<1>(USB_PCS_CLOCK);
		m_pit[index]->set_clk<2>(USB_2MHZ_CLOCK);
	}

	// connect the PIT outputs to the netlist
	m_pit[0]->out_handler<0>().set(m_nl_pit0_out[0], FUNC(netlist_mame_logic_input_device::write_line));
	m_pit[0]->out_handler<1>().set(m_nl_pit0_out[1], FUNC(netlist_mame_logic_input_device::write_line));
	m_pit[0]->out_handler<2>().set(m_nl_pit0_out[2], FUNC(netlist_mame_logic_input_device::write_line));
	m_pit[1]->out_handler<0>().set(m_nl_pit1_out[0], FUNC(netlist_mame_logic_input_device::write_line));
	m_pit[1]->out_handler<1>().set(m_nl_pit1_out[1], FUNC(netlist_mame_logic_input_device::write_line));
	m_pit[1]->out_handler<2>().set(m_nl_pit1_out[2], FUNC(netlist_mame_logic_input_device::write_line));
	m_pit[2]->out_handler<0>().set(m_nl_pit2_out[0], FUNC(netlist_mame_logic_input_device::write_line));
	m_pit[2]->out_handler<1>().set(m_nl_pit2_out[1], FUNC(netlist_mame_logic_input_device::write_line));
	m_pit[2]->out_handler<2>().set(m_nl_pit2_out[2], FUNC(netlist_mame_logic_input_device::write_line));

#endif
}


DEFINE_DEVICE_TYPE(SEGAUSBROM, usb_rom_sound_device, "segausbrom", "Sega Universal Sound Board with ROM")

usb_rom_sound_device::usb_rom_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: usb_sound_device(mconfig, SEGAUSBROM, tag, owner, clock)
{
}

void usb_sound_device::usb_map_rom(address_map &map)
{
	map(0x0000, 0x0fff).rom().region(":usbcpu", 0);
}

void usb_rom_sound_device::device_add_mconfig(machine_config &config)
{
	usb_sound_device::device_add_mconfig(config);

	// CPU for the usb board
	m_ourcpu->set_addrmap(AS_PROGRAM, &usb_rom_sound_device::usb_map_rom);
}
