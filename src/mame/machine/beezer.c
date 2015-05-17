// license:BSD-3-Clause
// copyright-holders:Mathis Rosenhauer
#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "machine/6522via.h"
#include "includes/beezer.h"



/* VIA 0 (aka "PPCNP74", U6 @1C on schematics)
    enabled at CE00-CFFF of main m6809 cpu when bankswitch is set to 0
    port A:
        bit 7: input, X from banking latch (d3 of banking register)
        bit 6: input, Y from banking latch (d4 of banking register)
        bit 5: input, Z from banking latch (d5 of banking register)
        bit 4: N/C
        bit 3: output, /RESET for audio subcpu
        bit 2: output, /ENABLE for LS139@2H for reading control and dipswitch inputs to pbus
        bit 1: output, MSb of selector for inputs to pbus
        bit 0: output, LSb of "
    port B:
        bits 7-0: input/output: pbus
    port C:
        CA1: N/C
        CA2: input: "TDISP" (one of the higher bits in the video line counter, a mirror of the D5 bit from beezer_line_r), done in /video/beezer.c
        CB1: ASH1 to via 1
        CB2: ASH2 to via 1
    /IRQ: to main m6809 cpu
    /RES: from main reset generator/watchdog/button

    TODO: find a better way to attach ca2 read to beezer_line_r
    */

/* VIA 1 (U18 @3C on schematics)
    port A:
        bits 7-0: input/output: pbus
    port B:
        bit 7: output: TIMER1 OUT (used to gate NOISE (see below) to clock channel 1 of 6840, plus acts as channel 0 by itself)
        bit 6: input: NOISE (from mm5837 14-bit LFSR, which also connects to clock above)
        bit 5: output?: N/C
        bit 4: output?: FMSEL1 (does not appear elsewhere on schematics! what does this do? needs tracing) - always 0?
        bit 3: output?: FMSEL0 (does not appear elsewhere on schematics! what does this do? needs tracing) - always 0?
        bit 2: output?: AM (does not appear elsewhere on schematics! what does this do? needs tracing) - always 0?
        bit 1: output: FM or AM (appears to control some sort of suppression or filtering change of the post-DAC amplifier when enabled, only during the TIMER1 OUT time-slot of the multiplexer, see page 1B 3-3 of schematics) - always 0? why is there a special circuit for it?
        bit 0: output?: DMOD DISABLE (does not appear elsewhere on schematics! what does this do? needs tracing) - on startup is 0, turns to 1 and stays that way?
    port C:
        CA1: AHS2 from via 0 (are these two switched?)
        CA2: AHS1 from via 0 "
        CB1: ??put: DMOD CLR (does not appear elsewhere on schematics! what does this do? needs tracing)
        CB2: ??put: DMOD DATA (does not appear elsewhere on schematics! what does this do? needs tracing)
    /IRQ: to audio/sub m6809 cpu
    /RES: from audio reset bit of via 0

    TODO: the entirety of port B, much needs tracing
    TODO: ports CB1 and CB2, need tracing; ports CA1 and CA2 could use verify as well
    */

READ8_MEMBER(beezer_state::b_via_0_pa_r)
{
	return (m_banklatch&0x38)<<2; // return X,Y,Z bits TODO: the Z bit connects somewhere else... where?
}

READ8_MEMBER(beezer_state::b_via_0_pb_r)
{
	return m_pbus;
}

WRITE8_MEMBER(beezer_state::b_via_0_pa_w)
{
	if ((data & 0x08) == 0)
		m_audiocpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	else
		m_audiocpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);

	if ((data & 0x04) == 0)
	{
		switch (data & 0x03)
		{
		case 0:
			m_pbus = ioport("IN0")->read();
			break;
		case 1:
			m_pbus = ioport("IN1")->read() | (ioport("IN2")->read() << 4);
			break;
		case 2:
			m_pbus = ioport("DSWB")->read();
			break;
		case 3:
			m_pbus = ioport("DSWA")->read(); // Technically DSWA isn't populated on the board and is pulled to 0xFF with resistor pack, but there IS a DSWA port in the driver so we may as well use it.
			break;
		}
	}
}

WRITE8_MEMBER(beezer_state::b_via_0_pb_w)
{
	m_pbus = data;
}

READ8_MEMBER(beezer_state::b_via_1_pa_r)
{
	return m_pbus;
}

READ8_MEMBER(beezer_state::b_via_1_pb_r)
{
	return 0x1F | (m_custom->noise_r(space, 0)?0x40:0);
}

WRITE8_MEMBER(beezer_state::b_via_1_pa_w)
{
	m_pbus = data;
}

WRITE8_MEMBER(beezer_state::b_via_1_pb_w)
{
	m_custom->timer1_w(space, 0, data&0x80);
	//if ((data&0x1f) != 0x01)
	//  popmessage("via1 pb low write of 0x%02x is not supported! contact mamedev!", data&0x1f);
}

DRIVER_INIT_MEMBER(beezer_state,beezer)
{
	m_pbus = 0;
	m_banklatch = 0;
}

WRITE8_MEMBER(beezer_state::beezer_bankswitch_w)
{
	m_banklatch = data&0x3f; // latched 'x,y,z' plus bank bits in ls174 @ 4H
	if ((data & 0x07) == 0)
	{
		via6522_device *via_0 = machine().device<via6522_device>("via6522_0");
		space.install_write_handler(0xc600, 0xc7ff, write8_delegate(FUNC(beezer_state::watchdog_reset_w),this));
		space.install_write_handler(0xc800, 0xc9ff, write8_delegate(FUNC(beezer_state::beezer_map_w),this));
		space.install_read_handler(0xca00, 0xcbff, read8_delegate(FUNC(beezer_state::beezer_line_r),this));
		space.install_readwrite_handler(0xce00, 0xcfff, read8_delegate(FUNC(via6522_device::read), via_0), write8_delegate(FUNC(via6522_device::write), via_0));
	}
	else
	{
		UINT8 *rom = memregion("maincpu")->base() + 0x10000;
		space.install_ram(0xc000, 0xcfff, rom + (data & 0x07) * 0x2000 + ((data & 0x08) ? 0x1000: 0));
	}
}
