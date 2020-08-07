// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    segausb.cpp

    Sega Universal Sound Board.

***************************************************************************/

#include "emu.h"
#include "segausb.h"
#include "nl_segausb.h"

#include "includes/segag80r.h"
#include "includes/segag80v.h"

#include <cmath>


#define VERBOSE 0
#include "logmacro.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define USB_MASTER_CLOCK    XTAL(6'000'000)
#define USB_2MHZ_CLOCK      (USB_MASTER_CLOCK/3)
#define USB_PCS_CLOCK       (USB_2MHZ_CLOCK/2)
#define USB_GOS_CLOCK       (USB_2MHZ_CLOCK/16/4)



/***************************************************************************
    UNIVERSAL SOUND BOARD
***************************************************************************/

DEFINE_DEVICE_TYPE(SEGAUSB, usb_sound_device, "segausb", "Sega Universal Sound Board")

usb_sound_device::usb_sound_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock),
		device_mixer_interface(mconfig, *this),
		m_ourcpu(*this, "ourcpu"),
		m_maincpu(*this, finder_base::DUMMY_TAG),
#if (!SEGAUSB_FAKE_8253)
		m_pit(*this, "pit_%u", 0),
#endif
		m_nl_dac0(*this, "sound_nl:dac0_%u", 0),
		m_nl_sel0(*this, "sound_nl:sel0"),
#if (SEGAUSB_FAKE_8253)
		m_nl_pit0_hp0(*this, "sound_nl:pit0_hp0"),
		m_nl_pit0_hp1(*this, "sound_nl:pit0_hp1"),
		m_nl_pit0_hp2_on(*this, "sound_nl:pit0_hp2_on"),
		m_nl_pit0_hp2_off(*this, "sound_nl:pit0_hp2_off"),
#else
		m_nl_pit0_out(*this, "sound_nl:pit0_out%u", 0),
#endif
		m_nl_dac1(*this, "sound_nl:dac1_%u", 0),
		m_nl_sel1(*this, "sound_nl:sel1"),
#if (SEGAUSB_FAKE_8253)
		m_nl_pit1_hp0(*this, "sound_nl:pit1_hp0"),
		m_nl_pit1_hp1(*this, "sound_nl:pit1_hp1"),
		m_nl_pit1_hp2_on(*this, "sound_nl:pit1_hp2_on"),
		m_nl_pit1_hp2_off(*this, "sound_nl:pit1_hp2_off"),
#else
		m_nl_pit1_out(*this, "sound_nl:pit1_out%u", 0),
#endif
		m_nl_dac2(*this, "sound_nl:dac2_%u", 0),
		m_nl_sel2(*this, "sound_nl:sel2"),
#if (SEGAUSB_FAKE_8253)
		m_nl_pit2_hp0(*this, "sound_nl:pit2_hp0"),
		m_nl_pit2_hp1(*this, "sound_nl:pit2_hp1"),
		m_nl_pit2_hp2_on(*this, "sound_nl:pit2_hp2_on"),
		m_nl_pit2_hp2_off(*this, "sound_nl:pit2_hp2_off"),
#else
		m_nl_pit2_out(*this, "sound_nl:pit2_out%u", 0),
#endif
		m_in_latch(0),
		m_out_latch(0),
		m_last_p2_value(0),
		m_program_ram(*this, "pgmram"),
		m_work_ram(*this, "workram"),
		m_work_ram_bank(0),
		m_t1_clock(0),
		m_t1_clock_mask(0),
		m_gos_clock(0)
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
#if (!SEGAUSB_FAKE_8253)
	for (int index = 0; index < 3; index++)
	{
		m_pit[index]->write_gate0(1);
		m_pit[index]->write_gate1(1);
		m_pit[index]->write_gate2(1);
	}
#endif

	// register for save states
	save_item(NAME(m_in_latch));
	save_item(NAME(m_out_latch));
	save_item(NAME(m_last_p2_value));
	save_item(NAME(m_work_ram_bank));
	save_item(NAME(m_t1_clock));
	save_item(NAME(m_gos_clock));
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
	machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(250));
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


READ_LINE_MEMBER( usb_sound_device::t1_r )
{
	// T1 returns 1 based on the value of the T1 clock; the exact
	// pattern is determined by one or more jumpers on the board.
	return (m_t1_clock & m_t1_clock_mask) != 0;
}



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
	switch (offset)
	{
		case 0x00:  // 8253 U41
		case 0x01:  // 8253 U41
		case 0x02:  // 8253 U41
		case 0x03:  // 8253 U41
#if (SEGAUSB_FAKE_8253)
			if ((offset & 3) == 2) printf("%s: 0.2 count=%d\n", machine().scheduler().time().as_string(), data);
			m_pit_state[0].write(offset & 3, data, *m_nl_pit0_hp0, *m_nl_pit0_hp1, *m_nl_pit0_hp2_on, *m_nl_pit0_hp2_off);
#else
			m_pit[0]->write(offset & 3, data);
#endif
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
#if (SEGAUSB_FAKE_8253)
			if ((offset & 3) == 2) printf("%s: 1.2 count=%d\n", machine().scheduler().time().as_string(), data);
			m_pit_state[1].write(offset & 3, data, *m_nl_pit1_hp0, *m_nl_pit1_hp1, *m_nl_pit1_hp2_on, *m_nl_pit1_hp2_off);
#else
			m_pit[1]->write(offset & 3, data);
#endif
			break;

		case 0x0c:  // ENV1 U12
		case 0x0d:  // ENV1 U13
		case 0x0e:  // ENV1 U14
			m_nl_dac1[offset & 3]->write(double(data) / 255.0);
			break;

		case 0x0f:  // ENV0 U2B
			m_nl_sel1->write(data & 1);
			break;

		case 0x10:  // 8253 U43
		case 0x11:  // 8253 U43
		case 0x12:  // 8253 U43
		case 0x13:  // 8253 U43
#if (SEGAUSB_FAKE_8253)
			if ((offset & 3) == 2) printf("%s: 2.2 count=%d\n", machine().scheduler().time().as_string(), data);
			m_pit_state[2].write(offset & 3, data, *m_nl_pit2_hp0, *m_nl_pit2_hp1, *m_nl_pit2_hp2_on, *m_nl_pit2_hp2_off);
#else
			if ((offset & 3) == 2)
				printf("%s: 2.2 count=%d\n", machine().scheduler().time().as_string(), data);
			m_pit[2]->write(offset & 3, data);
#endif
			break;

		case 0x14:  // ENV2 U27
		case 0x15:  // ENV2 U28
		case 0x16:  // ENV2 U29
			m_nl_dac2[offset & 3]->write(double(data) / 255.0);
			break;

		case 0x17:  // ENV0 U38B
			m_nl_sel2->write(data & 1);
			break;
	}
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
	map(0x00, 0xff).rw(FUNC(usb_sound_device::workram_r), FUNC(usb_sound_device::workram_w)).share("workram");
}


//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------

#if (SEGAUSB_FAKE_8253)

void usb_sound_device::pit_state::write(u8 offset, u8 data, netlist_mame_analog_input_device &hp0, netlist_mame_analog_input_device &hp1, netlist_mame_analog_input_device &hp2on, netlist_mame_analog_input_device &hp2off)
{
	u8 which;
	if (offset == 3)
	{
		which = data >> 6;
		if (which < 3)
		{
			mode[which] = data;
			latch[which] = 0;
		}
	}
	else
	{
		which = offset;
		u8 ctrmode = (mode[which] >> 4) & 3;
		if (ctrmode == 1)
			cvalue[which] = data;
		else if (ctrmode == 2)
			cvalue[which] = data << 8;
		else if (ctrmode == 3)
		{
			if (latch[which] == 0)
			{
				cvalue[which] = data;
				latch[which] = 1;
				return;
			}
			else
			{
				cvalue[which] |= data << 8;
				latch[which] = 0;
			}
		}
	}

	u8 thismode = (mode[which] >> 1) & 7;
	u32 cval = cvalue[which];
	if (cval == 0)
		cval = 65536;
	if (thismode == 3 && which < 2)
	{
		double hp = double(cval) / (USB_PCS_CLOCK.dvalue() * 2);
		if (hp == 0) hp = 0.1;
		if (which == 0)
			hp0.write(hp);
		else
			hp1.write(hp);
//		printf("HP[%d] = %f\n", which, hp);
	}
	else if (thismode == 1 && which == 2)
	{
		double period = 1.0 / USB_GOS_CLOCK.dvalue();
		double hpon = double(cval) / (USB_2MHZ_CLOCK.dvalue() * 2);
		double hpoff = period - hpon;
		if (hpoff <= 0)
			hpon = 0.1, hpoff = 0.000001;
		hp2on.write(hpon);
		hp2off.write(hpoff);
		printf("HP[2] = %f/%f\n", hpon, hpoff);
	}
	else
	{
		printf("WARNING: which=%d mode=%d\n", which, thismode);
	}
}

#else

TIMER_DEVICE_CALLBACK_MEMBER( usb_sound_device::gos_timer )
{
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

#if (!SEGAUSB_FAKE_8253)
	TIMER(config, "gos_timer", 0).configure_periodic(
			FUNC(usb_sound_device::gos_timer), attotime::from_hz(USB_GOS_CLOCK));
#endif

	NETLIST_SOUND(config, "sound_nl", 48000)
		.set_source(NETLIST_NAME(segausb))
		.add_route(ALL_OUTPUTS, *this, 1.0);

	// channel 0 inputs
	NETLIST_ANALOG_INPUT(config, m_nl_dac0[0], "I_U26_DAC.IN");
	NETLIST_ANALOG_INPUT(config, m_nl_dac0[1], "I_U25_DAC.IN");
	NETLIST_ANALOG_INPUT(config, m_nl_dac0[2], "I_U24_DAC.IN");
	NETLIST_LOGIC_INPUT(config, m_nl_sel0, "I_U38B_SEL.IN", 0);
#if (SEGAUSB_FAKE_8253)
	NETLIST_ANALOG_INPUT(config, m_nl_pit0_hp0, "I_U41_HP0.IN");
	NETLIST_ANALOG_INPUT(config, m_nl_pit0_hp1, "I_U41_HP1.IN");
	NETLIST_ANALOG_INPUT(config, m_nl_pit0_hp2_on, "I_U41_HP2_ON.IN");
	NETLIST_ANALOG_INPUT(config, m_nl_pit0_hp2_off, "I_U41_HP2_OFF.IN");
#else
	NETLIST_LOGIC_INPUT(config, m_nl_pit0_out[0], "I_U41_OUT0.IN", 0);
	NETLIST_LOGIC_INPUT(config, m_nl_pit0_out[1], "I_U41_OUT1.IN", 0);
	NETLIST_LOGIC_INPUT(config, m_nl_pit0_out[2], "I_U41_OUT2.IN", 0);
#endif

	// channel 1 inputs
	NETLIST_ANALOG_INPUT(config, m_nl_dac1[0], "I_U12_DAC.IN");
	NETLIST_ANALOG_INPUT(config, m_nl_dac1[1], "I_U13_DAC.IN");
	NETLIST_ANALOG_INPUT(config, m_nl_dac1[2], "I_U14_DAC.IN");
	NETLIST_LOGIC_INPUT(config, m_nl_sel1, "I_U2B_SEL.IN", 0);
#if (SEGAUSB_FAKE_8253)
	NETLIST_ANALOG_INPUT(config, m_nl_pit1_hp0, "I_U42_HP0.IN");
	NETLIST_ANALOG_INPUT(config, m_nl_pit1_hp1, "I_U42_HP1.IN");
	NETLIST_ANALOG_INPUT(config, m_nl_pit1_hp2_on, "I_U42_HP2_ON.IN");
	NETLIST_ANALOG_INPUT(config, m_nl_pit1_hp2_off, "I_U42_HP2_OFF.IN");
#else
	NETLIST_LOGIC_INPUT(config, m_nl_pit1_out[0], "I_U42_OUT0.IN", 0);
	NETLIST_LOGIC_INPUT(config, m_nl_pit1_out[1], "I_U42_OUT1.IN", 0);
	NETLIST_LOGIC_INPUT(config, m_nl_pit1_out[2], "I_U42_OUT2.IN", 0);
#endif

	// channel 2 inputs
	NETLIST_ANALOG_INPUT(config, m_nl_dac2[0], "I_U27_DAC.IN");
	NETLIST_ANALOG_INPUT(config, m_nl_dac2[1], "I_U28_DAC.IN");
	NETLIST_ANALOG_INPUT(config, m_nl_dac2[2], "I_U29_DAC.IN");
	NETLIST_LOGIC_INPUT(config, m_nl_sel2, "I_U2A_SEL.IN", 0);
#if (SEGAUSB_FAKE_8253)
	NETLIST_ANALOG_INPUT(config, m_nl_pit2_hp0, "I_U43_HP0.IN");
	NETLIST_ANALOG_INPUT(config, m_nl_pit2_hp1, "I_U43_HP1.IN");
	NETLIST_ANALOG_INPUT(config, m_nl_pit2_hp2_on, "I_U43_HP2_ON.IN");
	NETLIST_ANALOG_INPUT(config, m_nl_pit2_hp2_off, "I_U43_HP2_OFF.IN");
#else
	NETLIST_LOGIC_INPUT(config, m_nl_pit2_out[0], "I_U43_OUT0.IN", 0);
	NETLIST_LOGIC_INPUT(config, m_nl_pit2_out[1], "I_U43_OUT1.IN", 0);
	NETLIST_LOGIC_INPUT(config, m_nl_pit2_out[2], "I_U43_OUT2.IN", 0);
#endif

	// final output
	NETLIST_STREAM_OUTPUT(config, "sound_nl:cout0", 0, "OUTPUT").set_mult_offset(5000.0, 0.0);

#if (!SEGAUSB_FAKE_8253)
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

	/* CPU for the usb board */
	m_ourcpu->set_addrmap(AS_PROGRAM, &usb_rom_sound_device::usb_map_rom);
}
