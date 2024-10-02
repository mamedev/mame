// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Midway MCR systems

    driver by Christopher Kirmse, Aaron Giles

    Games supported:
        * Solar Fox
        * Kick
        * Draw Poker
        * Satan's Hollow
        * Tron
        * Kozmik Krooz'r
        * Domino Man
        * Wacko
        * Two Tigers
        * Journey
        * Tapper
        * Timber
        * Discs of Tron (Squawk n' Talk)
        * NFL Football (Squawk n' Talk + laserdisk)
        * Demolition Derby (Turbo Cheap Squeak)

****************************************************************************

    Early MCR systems have three PCBs, which can be intermixed to a certain
    degree:

        CPU board contains:
            * the main Z80 and program ROMs
            * the video sync chain
            * the background generation and background graphics ROMs
            * the mixing logic and color generation

        Video generator board contains:
            * the foreground generation and foreground graphics ROMs

        Sound I/O board contains:
            * the Z80 that drivers the sound
            * two AY-8910s for sound generation
            * all input/output ports accessed by the main CPU

    The layout of the PCB is identified by a five-digit number silkscreened
    on the PCB. This number also matches the number in the bottom-right
    corner of the schematics. Note, however, that the wiring diagrams in
    the schematics are generally a horrible cut and paste job and often
    have incorrect board numbers!

    Below is the definitive list of MCR games and which boards comprise
    their stack. Anything with identical boards should be interchangeable
    without problems. If you want to hook up some of the odd boards, you
    should look at the board descriptions below to understand the
    differences between each type of board:


                          CPU   Video  Sound  Others
                         -----  -----  -----  ------------
        Kick             90009  91399  90908
        Solar Fox        90009  91399  90908
        Draw Poker       90009  91399  90908  ?

        Satan's Hollow   90010  91399  90913
        Tron             90010  91399  90913  91418
        Kozmik Krooz'r   90010  91399  91483  91458  91482
        Domino Man       90010  91399  90913
        Wacko            90010  91399  90913
        Two Tigers
        Journey          91475  91464  90913

        Tapper           91490  91464  90913
        Timber
        Discs of Tron    91490  91464  91657  91660
        NFL Football     91490  91464  91657  91660  91695
        Demolition Derby 91490  91464  90913

        Spy Hunter       91442  91433  91657/90913  91671
        Crater Raider    91721  91464  91657
        Turbo Tag        91442  91433  91657

****************************************************************************

    Detailed CPU board descriptions:
        90009 (Kick, Solar Fox, Draw Poker)
            * 2.5MHz Z80
            * up to 7x2k program EPROMs
            * 2x4k background EPROMs
            * 32 entry palette RAM
            * background colors 0-15
            * sprite colors 16-31
            * sprite pen 8 shows foreground through

        90010 (Satan's Hollow, Tron, Kozmik Krooz'r, Domino Man, Wacko)
            * 2.5MHz Z80
            * up to 6x8k program EPROMs
            * 2x8k background EPROMs
            * 64 entry palette RAM
            * background colors 0-63, upper two bits come from tile RAM
            * sprite colors 0-63, upper two bits come from tile RAM
            * sprite pen 8 shows foreground through

        91442 (Spy Hunter, Turbo Tag)
            * 5MHz Z80
            * up to 6x8k program EPROMs
            * 4x8k background EPROMs (odd format)
            * 1x4k alpha EPROM
            * 64 entry palette RAM
            * background colors 0-15 on top half of screen, 32-47 on bottom
            * sprite colors 16-31 on top half of screen, 48-63 on bottom
            * alpha colors hard coded to green, blue, or white

        91475 (Journey)
            * identical to 90010 except:
            * sprite colors 0-63, upper two bits come from video gen
            * sprite colors 48-63 have an extra bit for higher grayscale resolution

        91490 (Tapper, Discs of Tron, Demolition Derby)
            * 5MHz Z80
            * up to 3x16k + 1x8k program EPROMs
            * 2x16k background EPROMs
            * 64 entry palette RAM
            * background colors 0-63, upper two bits come from tile RAM
            * sprite colors 0-63, upper two bits come from either tile RAM or video gen (select via jumpers)

        91721 (Crater Raider)
            * unknown, but expected to be identical to 91442 except:
            * background colors 0-15 on all scanlines
            * sprite colors 0-63, upper two bits come from video gen

****************************************************************************

    Detailed Video board descriptions:
        91399 (Kick, Solar Fox, Draw Poker, Satan's Hollow, Tron, Kozmik Krooz'r, Domino Man, Wacko)
            * 4x8k sprite EPROMs
            * data is ORed into linebuffers
            * support for hflip, vflip (code bits 6,7)
            * output is 4 bits: VID0-3

        91442 (Spy Hunter, Turbo Tag)
            * 8x32k sprite EPROMs
            * data is written to linebuffers if not all F's
            * support for hflip, vflip (flags bits 4,5)
            * output is 4 bits: VID0-3

        91464 (Journey, Tapper, Discs of Tron, Demolition Derby)
            * 8x32k sprite EPROMs
            * data is written to linebuffers if not all F's
            * support for hflip, vflip (flags bits 4,5)
            * support for 4 bits of extra data per pixel (flags bits 0-3)
            * output is 8 bits: VID0-3, COL0-3

****************************************************************************

    Detailed Sound Board descriptions:
        90908 (Kick, Solar Fox, Draw Poker)
            * 2MHz Z80
            * 2x2MHz AY8910
            * each AY8910 channel has a duty cycle controlled @ 50kHz by a down counter
            * each AY8910 has a 3-to-8 demux that controls the left/right panning
                * anecdotal evidence suggests that this was not stuffed
            * input port 0 (J4 1-8)
            * input port 1 (J4 10-13,14-18)
            * input port 2 (J5 1-8)
            * DIP switch (10 position, port 3)
            * DIP switch (8 position, port 4)
            * output port (J5 10-17)

        90913 (Satan's Hollow, Tron, Domino Man, Wacko, Journey, Tapper, Demolition Derby)
            * 2MHz Z80
            * 2x2MHz AY8910
            * each AY8910 channel has a duty cycle controlled @ 50kHz by a down counter
            * removed the panning controls
            * input port 0 (J4 1-8)
            * input port 1 (J4 10-13,14-18)
            * input port 2 (J5 1-8)
            * DIP switch (10 position, port 3), top bit goes to J6 10
            * input port 4 (J6 1-7,9)
            * output port (J5 10-17)

        91483 (Kozmik Krooz'r)
            * basically identical to 90913
            * input port 4 has a set of jumpers between it and the J6 outputs

        91657 (NFL Football, Discs of Tron, Spy Hunter)
            * appears to be identical in every way to 90908
            * the panning circuitry is stuffed on these boards

****************************************************************************

    90009 = CPU Board (Kick, SolarFox, DPoker)
    90010 = Super CPU (Kroozr, Tron, SHollow)
    90908 = Sound I/O (Kick, SolarFox, DPoker)
    90913 = Super Sound I/O (SHollow, Tron)
    91399 = Video Gen (Kick, SolarFox, DPoker, SHollow, Tron, Kroozr)
    91418 = Optical Encoder (Tron)
    91433 = Video Gen III (SpyHunt)
    91434 = Optical Sensor PC (Kroozr)
    91442 = CPU Board MCR III (SpyHunt)
    91458 = Analog Joystick PC (Kroozr)
    91464 = Super Video Gen MCR III (Tapper)
    91482 = Optical Sensor PC (Kroozr)
    91483 = Super Sound I/O (Kroozr)
    91490 = 5MHz CPU (Tapper)
    91649 = Absolute Position PC (SpyHunt)
    91657 = Super Sound I/O with Panning (DOTron)
    91658 = Lamp Sequencer (DOTron)
    91659 = Flashing Fluorescent Assembly (DOTron)
    91660 = Squawk & Talk (DOTron, NFLFoot)
    91671 = Cheap Squeak Deluxe (SpyHunt)
    91673 = Lamp Driver (SpyHunt)
    91695 = IPU laserdisk controller (NFLFoot)
    91794 = Optical Encoder Deluxe (DemoDerb)
    91799 = Turbo Cheap Squeak (DemoDerb)

****************************************************************************

    SSIO outputs:
        Output port 4 = J5=10-17
        Output port

****************************************************************************

    Memory map

****************************************************************************

    ========================================================================
    CPU #1
    ========================================================================
    0000-6FFF   R     xxxxxxxx    Program ROM
    7000-77FF   R/W   xxxxxxxx    NVRAM
    F000-F1FF   R/W   xxxxxxxx    Sprite RAM
    F400-F41F     W   xxxxxxxx    Palette RAM blue/green
    F800-F81F     W   xxxxxxxx    Palette RAM red
    FC00-FFFF   R/W   xxxxxxxx    Background video RAM
    ========================================================================
    0000        R     x-xxxxxx    Input ports
                R     x-------    Service switch (active low)
                R     --x-----    Tilt
                R     ---xxx--    External inputs
                R     ------x-    Right coin
                R     -------x    Left coin
    0000        W     xxxxxxxx    Data latch OP0 (coin meters, 2 led's and cocktail 'flip')
    0001        R     xxxxxxxx    External inputs
    0002        R     xxxxxxxx    External inputs
    0003        R     xxxxxxxx    DIP switches
    0004        R     xxxxxxxx    External inputs
    0004        W     xxxxxxxx    Data latch OP4 (comm. with external hardware)
    0007        R     xxxxxxxx    Audio status
    001C-001F   W     xxxxxxxx    Audio latches 1-4
    00E0        W     --------    Watchdog reset
    00E8        W     xxxxxxxx    Unknown (written at initialization time)
    00F0-00F3   W     xxxxxxxx    CTC communications
    ========================================================================
    Interrupts:
        NMI ???
        INT generated by CTC
    ========================================================================


    ========================================================================
    CPU #2 (Super Sound I/O)
    ========================================================================
    0000-3FFF   R     xxxxxxxx    Program ROM
    8000-83FF   R/W   xxxxxxxx    Program RAM
    9000-9003   R     xxxxxxxx    Audio latches 1-4
    A000          W   xxxxxxxx    AY-8910 #1 control
    A001        R     xxxxxxxx    AY-8910 #1 status
    A002          W   xxxxxxxx    AY-8910 #1 data
    B000          W   xxxxxxxx    AY-8910 #2 control
    B001        R     xxxxxxxx    AY-8910 #2 status
    B002          W   xxxxxxxx    AY-8910 #2 data
    C000          W   xxxxxxxx    Audio status
    E000          W   xxxxxxxx    Unknown
    F000        R     xxxxxxxx    Audio board switches
    ========================================================================
    Interrupts:
        NMI ???
        INT generated by external circuitry 780 times/second
    ========================================================================

***************************************************************************/


#include "emu.h"
#include "mcr.h"

#include "machine/nvram.h"
#include "screen.h"
#include "speaker.h"

#include "dpoker.lh"


void mcr_state::mcr_control_port_w(uint8_t data)
{
	/*
	    Bit layout is as follows:
	        D7 = n/c
	        D6 = cocktail flip
	        D5 = red LED
	        D4 = green LED
	        D3 = n/c
	        D2 = coin meter 3
	        D1 = coin meter 2
	        D0 = coin meter 1
	*/

	machine().bookkeeping().coin_counter_w(0, (data >> 0) & 1);
	machine().bookkeeping().coin_counter_w(1, (data >> 1) & 1);
	machine().bookkeeping().coin_counter_w(2, (data >> 2) & 1);
	m_mcr_cocktail_flip = (data >> 6) & 1;
}


/*************************************
 *
 *  Solar Fox I/O ports
 *
 *************************************/

uint8_t mcr_state::solarfox_ip0_r()
{
	/* This is a kludge; according to the wiring diagram, the player 2 */
	/* controls are hooked up as documented below. If you go into test */
	/* mode, they will respond. However, if you try it in a 2-player   */
	/* game in cocktail mode, they don't work at all. So we fake-mux   */
	/* the controls through player 1's ports */
	if (m_mcr_cocktail_flip)
		return ioport("ssio:IP0")->read() | 0x08;
	else
		return ((ioport("ssio:IP0")->read() & ~0x14) | 0x08) | ((ioport("ssio:IP0")->read() & 0x08) >> 1) | ((ioport("ssio:IP2")->read() & 0x01) << 4);
}


uint8_t mcr_state::solarfox_ip1_r()
{
	/*  same deal as above */
	if (m_mcr_cocktail_flip)
		return ioport("ssio:IP1")->read() | 0xf0;
	else
		return (ioport("ssio:IP1")->read() >> 4) | 0xf0;
}



/*************************************
 *
 *  Kick I/O ports
 *
 *************************************/

uint8_t mcr_state::kick_ip1_r()
{
	return (ioport("DIAL2")->read() << 4) & 0xf0;
}



/*************************************
 *
 *  Draw Poker I/O ports
 *
 *************************************/

TIMER_DEVICE_CALLBACK_MEMBER(mcr_dpoker_state::hopper_callback)
{
	if (m_output & 0x40)
	{
		// hopper timing is a guesstimate
		m_coin_status ^= 8;
		m_hopper_timer->adjust(attotime::from_msec((m_coin_status & 8) ? 100 : 250));
	}
	else
	{
		m_coin_status &= ~8;
	}

	machine().bookkeeping().coin_counter_w(3, m_coin_status & 8);
}

TIMER_DEVICE_CALLBACK_MEMBER(mcr_dpoker_state::coin_in_callback)
{
	m_coin_status &= ~2;
}

INPUT_CHANGED_MEMBER(mcr_dpoker_state::coin_in_hit)
{
	if (newval)
	{
		// The game waits for coin release before it accepts another.
		// It probably does this to prevent tampering, good old coin-on-a-string won't work here.
		m_coin_status |= 2;
		m_coin_in_timer->adjust(attotime::from_msec(100));
	}
}

uint8_t mcr_dpoker_state::ip0_r()
{
	// d0: Coin-in Hit
	// d1: Coin-in Release
	// d2: Coin-out Up
	// d3: Coin-out Down
	// d6: Coin-drop Hit
	// d7: Coin-drop Release
	uint8_t p0 = ioport("ssio:IP0")->read();
	p0 |= (m_coin_status >> 1 & 1);
	p0 ^= (p0 << 1 & 0x80) | m_coin_status;
	return p0;
}


void mcr_dpoker_state::lamps1_w(uint8_t data)
{
	// cpanel button lamps (white)
	m_lamps[0] = BIT(data, 0); // hold 1
	m_lamps[1] = BIT(data, 4); // hold 2
	m_lamps[2] = BIT(data, 5); // hold 3
	m_lamps[3] = BIT(data, 6); // hold 4
	m_lamps[4] = BIT(data, 7); // hold 5
	m_lamps[5] = BIT(data, 1); // deal
	m_lamps[6] = BIT(data, 2); // cancel
	m_lamps[7] = BIT(data, 3); // stand
}

void mcr_dpoker_state::lamps2_w(uint8_t data)
{
	// d5: button lamp: service or change
	m_lamps[8] = BIT(data, 5);

	// d0-d4: marquee lamps: coin 1 to 5 --> output lamps 9 to 13
	for (int i = 0; i < 5; i++)
		m_lamps[9 + i] = BIT(data, i);

	// d6, d7: unused?
}

void mcr_dpoker_state::output_w(uint8_t data)
{
	// d0: ? coin return
	// d1: ? divertor (active low)
	// d3: coin counter?

	// d6: assume hopper coin flow
	// d7: assume hopper motor
	if (data & 0x40 & ~m_output)
		m_hopper_timer->adjust(attotime::from_msec(500));

	// other bits: unused?

	m_output = data;
}

void mcr_dpoker_state::meters_w(uint8_t data)
{
	// meters?
}



/*************************************
 *
 *  Wacko I/O ports
 *
 *************************************/

void mcr_state::wacko_op4_w(uint8_t data)
{
	m_input_mux = data & 1;
}


uint8_t mcr_state::wacko_ip1_r()
{
	if (!m_input_mux)
		return ioport("ssio:IP1")->read();
	else
		return ioport("ssio:IP1.ALT")->read();
}


uint8_t mcr_state::wacko_ip2_r()
{
	if (!m_input_mux)
		return ioport("ssio:IP2")->read();
	else
		return ioport("ssio:IP2.ALT")->read();
}



/*************************************
 *
 *  Kozmik Krooz'r I/O ports
 *
 *************************************/

uint8_t mcr_state::kroozr_ip1_r()
{
	int dial = ioport("DIAL")->read();
	return ((dial & 0x80) >> 1) | ((dial & 0x70) >> 4);
}


void mcr_state::kroozr_op4_w(uint8_t data)
{
	/*
	    bit 2 = ship control
	    bit 4 = cargo light cntl 1
	    bit 5 = cargo light cntl 2
	*/
}



/*************************************
 *
 *  Journey I/O ports
 *
 *************************************/

void mcr_state::journey_op4_w(uint8_t data)
{
	/* if we're not playing the sample yet, start it */
	if (!m_samples->playing(0))
		m_samples->start(0, 0, true);

	/* bit 0 turns cassette on/off */
	m_samples->pause(0, ~data & 1);
}



/*************************************
 *
 *  Two Tigers I/O ports
 *
 *************************************/

void mcr_state::twotiger_op4_w(uint8_t data)
{
	for (int i = 0; i < 2; i++)
	{
		/* play tape, and loop it */
		if (!m_samples->playing(i))
			m_samples->start(i, i, true);

		/* bit 1 turns cassette on/off */
		m_samples->pause(i, ~data & 2);
	}

	// bit 2: lamp control?
	// if (data & 0xfc) printf("%x ",data);
}



/*************************************
 *
 *  Discs of Tron I/O ports
 *
 *************************************/

void mcr_state::dotron_op4_w(uint8_t data)
{
	/*
	    Flasher Control:
	        A 555 timer is set up in astable mode with R1=R2=56k and C=1uF giving
	        a frequency of 8.5714 Hz. The timer is enabled if J1-3 is high (1).
	        The output of the timer is connected to the input of a D-type flip
	        flop at 1A, which is clocked by the AC sync (since this is a
	        fluorescent light fixture).

	        The J1-4 input is also connected the input of another D-type flip flop
	        on the same chip at 1A. The output of this directly controls the light
	        fixture.

	        Thus:
	            J1-3 enables a strobe effect at 8.5714 Hz (77.616ms high, 38.808ms low)
	            J1-4 directly enables/disables the lamp.
	            The two outputs are wire-ored together.
	*/
	/* bit 7 = FL1 (J1-3) on flasher control board */
	/* bit 6 = FL0 (J1-4) on flasher control board */
	m_backlight = BIT(data, 6);

	/*
	    Lamp Sequencer:
	        A 556 timer is set up in astable mode with two different frequencies,
	        one using R1=R2=10k and C=10uF giving a frequency of 4.8 Hz, and the
	        second using R1=R2=5.1k and C=10uF giving a frequency of 9.4118 Hz.

	        The outputs of these clocks go into a mux at U4, whose input is
	        selected by the input bit latched from J1-6.

	        The output of the mux clocks a 16-bit binary counter at U3. The
	        output of the binary counter becomes the low 4 address bits of the
	        82S123 PROM at U2. The upper address bit comes from the input bit
	        latched from J1-5.

	        Each of the 5 output bits from the 82S123 is inverted and connected
	        to one of the lamps. The /CE pin on the 82S123 is connected to the
	        input bit latched from J1-4.

	        Thus:
	            J1-4 enables (0) or disables (1) the lamp sequencing.
	            J1-5 selects one of two 16-entry sequences stored in the 82S123.
	            J1-6 selects one of two speeds (0=4.8 Hz, 1=9.4118 Hz)

	*/
	/* bit 5 = SEL1 (J1-1) on the Lamp Sequencer board */
	if (((m_last_op4 ^ data) & 0x20) && (data & 0x20))
	{
		/* bit 2 -> J1-4 = enable */
		/* bit 1 -> J1-5 = sequence select */
		/* bit 0 -> J1-6 = speed (0=slow, 1=fast) */
		logerror("Lamp: en=%d seq=%d speed=%d\n", (data >> 2) & 1, (data >> 1) & 1, data & 1);
	}
	m_last_op4 = data;

	/* bit 4 = SEL0 (J1-8) on squawk n talk board */
	/* bits 3-0 = MD3-0 connected to squawk n talk (J1-4,3,2,1) */
	m_squawk_n_talk->sound_select(data & 0x0f);
	m_squawk_n_talk->sound_int(BIT(data, 4));
}



/*************************************
 *
 *  NFL Football I/O ports
 *
 *************************************/

uint8_t mcr_nflfoot_state::ip2_r()
{
	/* bit 7 = J3-2 on IPU board = TXDA on SIO */
	uint8_t val = m_ipu_sio_txda << 7;
	return val;
}


void mcr_nflfoot_state::op4_w(uint8_t data)
{
	/* bit 7 = J3-7 on IPU board = /RXDA on SIO */
	m_ipu_sio->rxa_w(!((data >> 7) & 1));

	/* bit 6 = J3-3 on IPU board = CTSA on SIO */
	m_ipu_sio->ctsa_w((data >> 6) & 1);

	/* bit 4 = SEL0 (J1-8) on squawk n talk board */
	/* bits 3-0 = MD3-0 connected to squawk n talk (J1-4,3,2,1) */
	m_squawk_n_talk->sound_select(data & 0x0f);
	m_squawk_n_talk->sound_int(BIT(data, 4));
}



/*************************************
 *
 *  Demolition Derby I/O ports
 *
 *************************************/

uint8_t mcr_state::demoderb_ip1_r()
{
	return ioport("ssio:IP1")->read() |
		(ioport(m_input_mux ? "ssio:IP1.ALT2" : "ssio:IP1.ALT1")->read() << 2);
}


uint8_t mcr_state::demoderb_ip2_r()
{
	return ioport("ssio:IP2")->read() |
		(ioport(m_input_mux ? "ssio:IP2.ALT2" : "ssio:IP2.ALT1")->read() << 2);
}


void mcr_state::demoderb_op4_w(uint8_t data)
{
	if (data & 0x40) m_input_mux = 1;
	if (data & 0x80) m_input_mux = 0;
	m_turbo_cheap_squeak->write(data);
}



/*************************************
 *
 *  CPU board 90009 memory handlers
 *
 *************************************/

/* address map verified from schematics */
void mcr_state::cpu_90009_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x6fff).rom();
	map(0x7000, 0x77ff).mirror(0x0800).ram().share("nvram");
	map(0xf000, 0xf1ff).mirror(0x0200).ram().share("spriteram");
	map(0xf400, 0xf41f).mirror(0x03e0).w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0xf800, 0xf81f).mirror(0x03e0).w(m_palette, FUNC(palette_device::write8_ext)).share("palette_ext");
	map(0xfc00, 0xffff).ram().w(FUNC(mcr_state::mcr_90009_videoram_w)).share("videoram");
}

/* upper I/O map determined by PAL; only SSIO ports are verified from schematics */
void mcr_state::cpu_90009_portmap(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	midway_ssio_device::ssio_input_ports(map, "ssio");
	map(0xe0, 0xe0).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0xe8, 0xe8).nopw();
	map(0xf0, 0xf3).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
}

void mcr_state::cpu_90009_dp_map(address_map &map)
{
	cpu_90009_map(map);
	map(0x8000, 0x81ff).ram();  // meter ram, is it battery backed?
}

void mcr_state::cpu_90009_dp_portmap(address_map &map)
{
	cpu_90009_portmap(map);
	map(0x24, 0x24).portr("P24");
	map(0x28, 0x28).portr("P28");
	map(0x2c, 0x2c).portr("P2C");

	map(0x2c, 0x2c).w(FUNC(mcr_dpoker_state::lamps1_w));
	map(0x30, 0x30).w(FUNC(mcr_dpoker_state::lamps2_w));
	map(0x34, 0x34).w(FUNC(mcr_dpoker_state::output_w));
	map(0x3f, 0x3f).w(FUNC(mcr_dpoker_state::meters_w));
}


/*************************************
 *
 *  CPU board 90010 memory handlers
 *
 *************************************/

/* address map verified from schematics */
void mcr_state::cpu_90010_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xc7ff).mirror(0x1800).ram().share("nvram");
	map(0xe000, 0xe1ff).mirror(0x1600).ram().share("spriteram");
	map(0xe800, 0xefff).mirror(0x1000).ram().w(FUNC(mcr_state::mcr_90010_videoram_w)).share("videoram");
}

/* upper I/O map determined by PAL; only SSIO ports are verified from schematics */
void mcr_state::cpu_90010_portmap(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	midway_ssio_device::ssio_input_ports(map, "ssio");
	map(0xe0, 0xe0).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0xe8, 0xe8).nopw();
	map(0xf0, 0xf3).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
}



/*************************************
 *
 *  CPU board 91490 memory handlers
 *
 *************************************/

/* address map verified from schematics */
void mcr_state::cpu_91490_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xdfff).rom();
	map(0xe000, 0xe7ff).ram().share("nvram");
	map(0xe800, 0xe9ff).mirror(0x0200).ram().share("spriteram");
	map(0xf000, 0xf7ff).ram().w(FUNC(mcr_state::mcr_91490_videoram_w)).share("videoram");
	map(0xf800, 0xf87f).mirror(0x0780).w(FUNC(mcr_state::mcr_paletteram9_w)).share("paletteram");
}

/* upper I/O map determined by PAL; only SSIO ports are verified from schematics */
void mcr_state::cpu_91490_portmap(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	midway_ssio_device::ssio_input_ports(map, "ssio");
	map(0xe0, 0xe0).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0xe8, 0xe8).nopw();
	map(0xf0, 0xf3).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
}



/*************************************
 *
 *  IPU board 91695 memory handlers
 *
 *************************************/

/* address map verified from schematics */
void mcr_nflfoot_state::ipu_91695_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x3fff).rom();
	map(0xe000, 0xffff).ram();
}

/* I/O verified from schematics */
void mcr_nflfoot_state::ipu_91695_portmap(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x03).mirror(0xe0).rw(m_ipu_pio0, FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x04, 0x07).mirror(0xe0).rw(m_ipu_sio, FUNC(z80sio_device::cd_ba_r), FUNC(z80sio_device::cd_ba_w));
	map(0x08, 0x0b).mirror(0xe0).rw(m_ipu_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x0c, 0x0f).mirror(0xe0).rw(m_ipu_pio1, FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x10, 0x13).mirror(0xe0).w(FUNC(mcr_nflfoot_state::ipu_laserdisk_w));
	map(0x1c, 0x1f).mirror(0xe0).rw(FUNC(mcr_nflfoot_state::ipu_watchdog_r), FUNC(mcr_nflfoot_state::ipu_watchdog_w));
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

/* verified from wiring diagram, plus DIP switches from manual */
static INPUT_PORTS_START( solarfox )
	PORT_START("ssio:IP0")  /* J4 1-8 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("ssio:IP1")  /* J4 10-13,15-18 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL

	PORT_START("ssio:IP2")  /* J5 1-8 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ssio:IP3")  /* DIPSW @ B3 */
	PORT_DIPNAME( 0x03, 0x03, "Bonus" )
	PORT_DIPSETTING(    0x02, DEF_STR( None ) )
	PORT_DIPSETTING(    0x03, "After 10 racks" )
	PORT_DIPSETTING(    0x01, "After 20 racks" )
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ))
	PORT_DIPSETTING(    0x10, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x40, 0x40, "Ignore Hardware Failure" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ))
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ))
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ))

	PORT_START("ssio:IP4")  /* J6 1-8 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ssio:DIP")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/* verified from wiring diagram, plus DIP switches from manual */
static INPUT_PORTS_START( kick )
	PORT_START("ssio:IP0")  /* J4 1-8 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("ssio:IP1")  /* J4 10-13,15-18 */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(3) PORT_KEYDELTA(50) PORT_REVERSE

	PORT_START("ssio:IP2")  /* J5 1-8 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ssio:IP3")  /* DIPSW @ B3 */
	PORT_DIPNAME( 0x01, 0x00, "Music" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("ssio:IP4")  /* J6 1-8 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ssio:DIP")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DIAL2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


/* verified from wiring diagram, plus DIP switches from manual */
static INPUT_PORTS_START( kickc )
	PORT_START("ssio:IP0")  /* J4 1-8 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("ssio:IP1")  /* J4 10-13,15-18 */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(3) PORT_KEYDELTA(50) PORT_REVERSE

	PORT_START("ssio:IP2")  /* J5 1-8 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ssio:IP3")  /* DIPSW @ B3 */
	PORT_DIPNAME( 0x01, 0x00, "Music" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_BIT( 0x3e, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ))
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ))
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("ssio:IP4")  /* J6 1-8 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("ssio:DIP")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DIAL2")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(3) PORT_KEYDELTA(50) PORT_REVERSE PORT_COCKTAIL
INPUT_PORTS_END


static INPUT_PORTS_START( dpoker )
	PORT_START("ssio:IP0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, mcr_dpoker_state, coin_in_hit, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_CUSTOM ) // see ip0_r
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_CUSTOM ) // "
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_CUSTOM ) // "
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("Coin-drop")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) // "

	PORT_START("ssio:IP1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_CANCEL )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_STAND )

	PORT_START("ssio:IP2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN ) // only in test mode input test

	// 10-position DIP switch on the sound pcb
	// settings and defaults are verified from a sticker inside the cabinet, I don't know where 9 or 10 are connected
	PORT_START("ssio:IP3")
	PORT_DIPNAME( 0x01, 0x01, "Hopper" )            PORT_DIPLOCATION("B3:1")
	PORT_DIPSETTING(    0x01, "Relay Pulse" )
	PORT_DIPSETTING(    0x00, "Miser On" ) // what is this? - the game locks up if it's enabled
	PORT_DIPNAME( 0x02, 0x02, "Music" )             PORT_DIPLOCATION("B3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x04, 0x04, "Novelty" )           PORT_DIPLOCATION("B3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) )   PORT_DIPLOCATION("B3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unused ) )   PORT_DIPLOCATION("B3:5")
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x20, 0x20, "Cards After 5th Coin" )  PORT_DIPLOCATION("B3:6")
	PORT_DIPSETTING(    0x20, "Face Up" )
	PORT_DIPSETTING(    0x00, "Logo Up" )
	PORT_DIPNAME( 0x40, 0x40, "Currency" )          PORT_DIPLOCATION("B3:7")
	PORT_DIPSETTING(    0x40, "Ike Dollars" )
	PORT_DIPSETTING(    0x00, "Other Coins" )
	PORT_DIPNAME( 0x80, 0x00, "Background Color" )  PORT_DIPLOCATION("B3:8")
	PORT_DIPSETTING(    0x80, "Green" )
	PORT_DIPSETTING(    0x00, "Blue" )
//  PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unused ) )   PORT_DIPLOCATION("B3:9")
//  PORT_DIPSETTING(    0x01, DEF_STR( On ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
//  PORT_DIPNAME( 0x01, 0x00, "Freeze" )            PORT_DIPLOCATION("B3:10")
//  PORT_DIPSETTING(    0x01, DEF_STR( On ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( Off ) )

	PORT_START("ssio:IP4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ssio:DIP")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN ) // only in test mode input test

	PORT_START("P24")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_CUSTOM ) // Hopper Full (not implemented)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) // Coin Return
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Door 1 Open") PORT_CODE(KEYCODE_A) // CAM OPEN
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Door 1 Lock") PORT_CODE(KEYCODE_S) // CAM LOCK
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Door 2 Open") PORT_CODE(KEYCODE_D) // HNG OPEN
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Door 2 Lock") PORT_CODE(KEYCODE_F) // HNG LOCK

	PORT_START("P28")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2C")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) // ? ARM HIT
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW ) // will only work after opening a door
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) // ? XTMSG (ON/OFF)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) // ? EXTLKT (ON/OFF)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


/* verified from wiring diagram, plus DIP switches from manual */
static INPUT_PORTS_START( shollow )
	PORT_START("ssio:IP0")  /* J4 1-8 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("ssio:IP1")  /* J4 10-13,15-18 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_START("ssio:IP2")  /* J5 1-8 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ssio:IP3")  /* DIPSW @ B3 */
	PORT_DIPNAME( 0x01, 0x01, "Coin Meters" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Cocktail ) )
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("ssio:IP4")  /* J6 1-8 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ssio:DIP")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/* verified from wiring diagram, plus DIP switches from manual */
static INPUT_PORTS_START( tron )
	PORT_START("ssio:IP0")  /* J4 1-8 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("ssio:IP1")  /* J4 10-13,15-18 */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_CODE_DEC(KEYCODE_Z) PORT_CODE_INC(KEYCODE_X) PORT_REVERSE

	PORT_START("ssio:IP2")  /* J5 1-8 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL

	PORT_START("ssio:IP3")  /* DIPSW @ B3 */
	PORT_DIPNAME( 0x01, 0x00, "Coin Meters" )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Cabinet ) )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Allow_Continue ) )  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x00, "SW1:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x00, "SW1:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x00, "SW1:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x00, "SW1:7" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	// According to the manual, SW1 is a bank of *10* switches (9 is unused and 10 is freeze)
	// Where are the values for the other two bits read?

	PORT_START("ssio:IP4")  /* J6 1-8 */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_REVERSE PORT_COCKTAIL

	PORT_START("ssio:DIP")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

INPUT_PORTS_END

static INPUT_PORTS_START( tron3 )
	PORT_START("ssio:IP0")  /* J4 1-8 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("ssio:IP1")  /* J4 10-13,15-18 */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_CODE_DEC(KEYCODE_Z) PORT_CODE_INC(KEYCODE_X) PORT_REVERSE

	PORT_START("ssio:IP2")  /* J5 1-8 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
//  PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
//  PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
//  PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
//  PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL

	PORT_START("ssio:IP3")  /* DIPSW @ B3 */
	PORT_DIPNAME( 0x01, 0x00, "Coin Meters" )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Cabinet ) )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Allow_Continue ) )  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x00, "SW1:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x00, "SW1:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x00, "SW1:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x00, "SW1:7" )
//  PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	// According to the manual, SW1 is a bank of *10* switches (9 is unused and 10 is freeze)
	// Where are the values for the other two bits read?

	PORT_START("ssio:IP4")  /* J6 1-8 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
//  PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_REVERSE PORT_COCKTAIL

	PORT_START("ssio:DIP")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

INPUT_PORTS_END



/* verified from wiring diagram, plus DIP switches from manual */
static INPUT_PORTS_START( kroozr )
	PORT_START("ssio:IP0")  /* J4 1-8 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("ssio:IP1")  /* J4 10-13,15-18 */
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_CUSTOM )   /* low 3 bits of spinner */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM )   /* sensor J1-10 */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM )   /* sensor J1-9 */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM )   /* sensor J1-8 */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM )   /* upper 1 bit of spinner */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 )

	PORT_START("ssio:IP2")  /* J5 1-8 */
	PORT_BIT( 0xff, 0x64, IPT_AD_STICK_X ) PORT_MINMAX(48,152) PORT_SENSITIVITY(100) PORT_KEYDELTA(52)

	PORT_START("ssio:IP3")  /* DIPSW @ B3 */
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("ssio:IP4")  /* J6 1-8 */
	PORT_BIT( 0xff, 0x64, IPT_AD_STICK_Y ) PORT_MINMAX(48,152) PORT_SENSITIVITY(100) PORT_KEYDELTA(52)

	PORT_START("ssio:DIP")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DIAL")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(40) PORT_KEYDELTA(10) PORT_CODE_DEC(KEYCODE_Z) PORT_CODE_INC(KEYCODE_X) PORT_REVERSE
INPUT_PORTS_END


/* verified from wiring diagram, plus DIP switches from manual */
static INPUT_PORTS_START( domino )
	PORT_START("ssio:IP0")  /* J4 1-8 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("ssio:IP1")  /* J4 10-13,15-18 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ssio:IP2")  /* J5 1-8 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ssio:IP3")  /* DIPSW @ B3 */
	PORT_DIPNAME( 0x01, 0x00, "Music" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x02, 0x02, "Skin Color" )
	PORT_DIPSETTING(    0x02, "Light" )
	PORT_DIPSETTING(    0x00, "Dark" )
	PORT_BIT( 0x3c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x00, "Coin Meters" )
	PORT_DIPSETTING(    0x80, "1" )
	PORT_DIPSETTING(    0x00, "2" )

	PORT_START("ssio:IP4")  /* J6 1-8 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("ssio:DIP")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/* verified from wiring diagram, plus DIP switches from manual */
static INPUT_PORTS_START( journey )
	PORT_START("ssio:IP0")  /* J4 1-8 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("ssio:IP1")  /* J4 10-13,15-18 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ssio:IP2")  /* J5 1-8 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ssio:IP3")  /* DIPSW @ B3 */
	PORT_DIPNAME( 0x01, 0x01, "Coin Meters" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Cocktail ) )
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ssio:IP4")  /* J6 1-8 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("ssio:DIP")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/* verified from wiring diagram, plus DIP switches from manual */
static INPUT_PORTS_START( wacko )
	PORT_START("ssio:IP0")  /* J4 1-8 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("ssio:IP1")  /* J4 10-13,15-18 */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10)

	PORT_START("ssio:IP2")  /* J5 1-8 */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_REVERSE

	PORT_START("ssio:IP3")  /* DIPSW @ B3 */
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x00, "Coin Meters" )
	PORT_DIPSETTING(    0x80, "1" )
	PORT_DIPSETTING(    0x00, "2" )

	PORT_START("ssio:IP4")  /* J6 1-8 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_4WAY PORT_COCKTAIL

	PORT_START("ssio:DIP")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("ssio:IP1.ALT")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_COCKTAIL

	PORT_START("ssio:IP2.ALT")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_REVERSE PORT_COCKTAIL
INPUT_PORTS_END


/* not verified, no manual found */
static INPUT_PORTS_START( twotiger )
	PORT_START("ssio:IP0")  /* J4 1-8 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START3 ) PORT_NAME("Dogfight Start")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("ssio:IP1")  /* J4 10-13,15-18 */
	PORT_BIT( 0xff, 0x67, IPT_AD_STICK_X ) PORT_MINMAX(0, 206) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("ssio:IP2")  /* J5 1-8 */
	PORT_BIT( 0xff, 0x67, IPT_AD_STICK_X ) PORT_MINMAX(0, 206) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("ssio:IP3")  /* DIPSW @ B3 */
	PORT_DIPNAME( 0x01, 0x00, "Shot Speed" )
	PORT_DIPSETTING(    0x01, "Fast" )
	PORT_DIPSETTING(    0x00, "Slow" )
	PORT_DIPNAME( 0x02, 0x00, "Dogfight" )
	PORT_DIPSETTING(    0x00, "1 Credit" )
	PORT_DIPSETTING(    0x02, "2 Credits" )
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ssio:IP4")  /* J6 1-8 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ssio:DIP")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/* not verified, no manual found */
static INPUT_PORTS_START( twotigrc )
	PORT_START("ssio:IP0")  /* J4 1-8 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START3 ) PORT_NAME("Dogfight Start")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("ssio:IP1")  /* J4 10-13,15-18 */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(10) PORT_KEYDELTA(10) PORT_REVERSE

	PORT_START("ssio:IP2")  /* J5 1-8 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ssio:IP3")  /* DIPSW @ B3 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ssio:IP4")  /* J6 1-8 */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(10) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(2)

	PORT_START("ssio:DIP")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/* verified from wiring diagram, plus DIP switches from manual */
static INPUT_PORTS_START( tapper )
	PORT_START("ssio:IP0")  /* J4 1-8 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("ssio:IP1")  /* J4 10-13,15-18 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ssio:IP2")  /* J5 1-8 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ssio:IP3")  /* DIPSW @ B3 */
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x38, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, "Coin Meters" )
	PORT_DIPSETTING(    0x80, "1" )
	PORT_DIPSETTING(    0x00, "2" )

	PORT_START("ssio:IP4")  /* J6 1-8 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ssio:DIP")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/* not verified, no manual found */
static INPUT_PORTS_START( timber )
	PORT_START("ssio:IP0")  /* J4 1-8 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("ssio:IP1")  /* J4 10-13,15-18 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ssio:IP2")  /* J5 1-8 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ssio:IP3")  /* DIPSW @ B3 */
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x38, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, "Coin Meters" )
	PORT_DIPSETTING(    0x80, "1" )
	PORT_DIPSETTING(    0x00, "2" )

	PORT_START("ssio:IP4")  /* J6 1-8 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ssio:DIP")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/* verified from wiring diagram, plus DIP switches from manual */
static INPUT_PORTS_START( dotron )
	PORT_START("ssio:IP0")  /* J4 1-8 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("ssio:IP1")  /* J4 10-13,15-18 */
	PORT_BIT( 0x7f, 0x00, IPT_DIAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_REVERSE
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ssio:IP2")  /* J5 1-8 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Aim Down")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Aim Up")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, "Environmental" )
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )

	PORT_START("ssio:IP3")  /* DIPSW @ B3 */
	PORT_DIPNAME( 0x01, 0x01, "Coin Meters" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ssio:IP4")  /* J6 1-8 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ssio:DIP")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("FAKE")  /* fake port to make aiming up & down easier */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)
INPUT_PORTS_END

static INPUT_PORTS_START( dotrone )
	PORT_INCLUDE(dotron)

	PORT_MODIFY("ssio:IP2")
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, "Environmental" )
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
INPUT_PORTS_END


/* verified from wiring diagram, plus DIP switches from manual */
static INPUT_PORTS_START( nflfoot )
	PORT_START("ssio:IP0")  /* J4 1-8 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BILL1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )     /* continue game */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )   /* service */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )     /* new game */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("ssio:IP1")  /* J4 10-13,15-18 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)     /* left engage */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)     /* left select #1 play */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)     /* left select #2 play */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)     /* left select #3 play */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)     /* left select #4 play */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)     /* left select #5 play */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(1)     /* select one player */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("ssio:IP2")  /* J5 1-8 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)     /* right engage */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)     /* right select #1 play */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)     /* right select #2 play */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)     /* right select #3 play */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)     /* right select #4 play */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)     /* right select #5 play */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(2)     /* select two player */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM )                    /* connects to IPU board */

	PORT_START("ssio:IP3")  /* DIPSW @ B3 */
	PORT_DIPNAME( 0x01, 0x01, "Coin Meters" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ssio:IP4")  /* J6 1-8 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ssio:DIP")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/* "wiring diagram was not available at time of publication" according to the manual */
/* DIPs verified from the manual */
static INPUT_PORTS_START( demoderb )
	PORT_START("ssio:IP0")  /* J4 1-8 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE( 0x20, IP_ACTIVE_LOW )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ssio:IP1")  /* J4 10-13,15-18 */    /* The high 6 bits contain the steering wheel value */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)

	PORT_START("ssio:IP1.ALT1") /* J4 10-13,15-18 */    /* The high 6 bits contain the steering wheel value */
	PORT_BIT( 0x3f, 0x00, IPT_DIAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_INVERT PORT_PLAYER(1)

	PORT_START("ssio:IP1.ALT2") /* IN1 (muxed) -- the high 6 bits contain the steering wheel value */
	PORT_BIT( 0x3f, 0x00, IPT_DIAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_INVERT PORT_PLAYER(3)

	PORT_START("ssio:IP2")  /* J5 1-8 */    /* The high 6 bits contain the steering wheel value */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)

	PORT_START("ssio:IP2.ALT1") /* J5 1-8 */    /* The high 6 bits contain the steering wheel value */
	PORT_BIT( 0x3f, 0x00, IPT_DIAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_INVERT PORT_PLAYER(2)

	PORT_START("ssio:IP2.ALT2") /* IN2 (muxed) -- the high 6 bits contain the steering wheel value */
	PORT_BIT( 0x3f, 0x00, IPT_DIAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_INVERT PORT_PLAYER(4)

	PORT_START("ssio:IP3")  /* DIPSW @ B3 */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x01, "2P Upright" )
	PORT_DIPSETTING(    0x00, "4P Cocktail" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Harder ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Reward Screen" )
	PORT_DIPSETTING(    0x08, "Expanded" )
	PORT_DIPSETTING(    0x00, "Limited" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("ssio:IP4")  /* J6 1-8 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)

	PORT_START("ssio:DIP")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

INPUT_PORTS_END

static INPUT_PORTS_START( demoderbc )
	PORT_INCLUDE(demoderb)

	PORT_MODIFY("ssio:IP3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static GFXDECODE_START( gfx_mcr )
	GFXDECODE_SCALE( "gfx1", 0, mcr_bg_layout,     0, 4, 2, 2 ) /* colors 0-15, 2x2 */
	GFXDECODE_ENTRY( "gfx2", 0, mcr_sprite_layout, 0, 4 )       /* colors 16-31 */
GFXDECODE_END



/*************************************
 *
 *  Sound definitions
 *
 *************************************/

static const char *const journey_sample_names[] =
{
	"*journey",
	"sepways",
	nullptr
};

static const char *const twotiger_sample_names[] =
{
	"*twotiger",
	"left",
	"right",
	nullptr
};

/*************************************
 *
 *  Machine driver
 *
 *************************************/

/* 90009 CPU board plus 90908/90913/91483 sound board */
void mcr_state::mcr_90009(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, MAIN_OSC_MCR_I/8);
	m_maincpu->set_daisy_config(mcr_daisy_chain);
	m_maincpu->set_addrmap(AS_PROGRAM, &mcr_state::cpu_90009_map);
	m_maincpu->set_addrmap(AS_IO, &mcr_state::cpu_90009_portmap);

	TIMER(config, "scantimer").configure_scanline(FUNC(mcr_state::mcr_interrupt), "screen", 0, 1);

	Z80CTC(config, m_ctc, MAIN_OSC_MCR_I/8 /* same as "maincpu" */);
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ctc->zc_callback<0>().set(m_ctc, FUNC(z80ctc_device::trg1));

	WATCHDOG_TIMER(config, "watchdog").set_vblank_count("screen", 16);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	screen.set_refresh_hz(30);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(32*16, 30*16);
	screen.set_visarea(0*16, 32*16-1, 0*16, 30*16-1);
	screen.set_screen_update(FUNC(mcr_state::screen_update_mcr));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_mcr);
	PALETTE(config, m_palette).set_format(palette_device::xRBG_444, 32);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	MIDWAY_SSIO(config, m_ssio);
	m_ssio->add_route(0, "lspeaker", 1.0);
	m_ssio->add_route(1, "rspeaker", 1.0);
}

/* as above, but in a casino cabinet */
void mcr_dpoker_state::mcr_90009_dp(machine_config &config)
{
	mcr_90009(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &mcr_state::cpu_90009_dp_map);
	m_maincpu->set_addrmap(AS_IO, &mcr_state::cpu_90009_dp_portmap);

	/* basic machine hardware */
	TIMER(config, "coinin").configure_generic(FUNC(mcr_dpoker_state::coin_in_callback));
	TIMER(config, "hopper").configure_generic(FUNC(mcr_dpoker_state::hopper_callback));
}

/* 90010 CPU board plus 90908/90913/91483 sound board */
void mcr_state::mcr_90010(machine_config &config)
{
	mcr_90009(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &mcr_state::cpu_90010_map);
	m_maincpu->set_addrmap(AS_IO, &mcr_state::cpu_90010_portmap);

	/* video hardware */
	m_palette->set_format(palette_device::xRBG_444, 64);
}

/* as above, plus 8-track tape */
void mcr_state::mcr_90010_tt(machine_config &config)
{
	mcr_90010(config);

	/* sound hardware */
	SAMPLES(config, m_samples);
	m_samples->set_channels(2);
	m_samples->set_samples_names(twotiger_sample_names);
	m_samples->add_route(0, "lspeaker", 0.25);
	m_samples->add_route(1, "rspeaker", 0.25);
}

/* 91475 CPU board plus 90908/90913/91483 sound board plus cassette interface */
void mcr_state::mcr_91475(machine_config &config)
{
	mcr_90010(config);

	/* video hardware */
	m_palette->set_format(palette_device::xRBG_444, 128);

	/* sound hardware */
	SAMPLES(config, m_samples);
	m_samples->set_channels(1);
	m_samples->set_samples_names(journey_sample_names);
	m_samples->add_route(ALL_OUTPUTS, "lspeaker", 0.25);
	m_samples->add_route(ALL_OUTPUTS, "rspeaker", 0.25);
}


/* 91490 CPU board plus 90908/90913/91483 sound board */
void mcr_state::mcr_91490(machine_config & config)
{
	mcr_90010(config);

	/* basic machine hardware */
	m_maincpu->set_clock(5000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &mcr_state::cpu_91490_map);
	m_maincpu->set_addrmap(AS_IO, &mcr_state::cpu_91490_portmap);

	m_ctc->set_clock(5000000 /* same as "maincpu" */);
}


/* 91490 CPU board plus 90908/90913/91483 sound board plus Squawk n' Talk sound board */
void mcr_state::mcr_91490_snt(machine_config &config)
{
	mcr_91490(config);

	/* basic machine hardware */
	BALLY_SQUAWK_N_TALK(config, m_squawk_n_talk);
	m_squawk_n_talk->add_route(ALL_OUTPUTS, "lspeaker", 1.0);
	m_squawk_n_talk->add_route(ALL_OUTPUTS, "rspeaker", 1.0);
}


/* 91490 CPU board plus 90908/90913/91483 sound board plus Squawk n' Talk sound board plus IPU */
void mcr_nflfoot_state::mcr_91490_ipu(machine_config &config)
{
	mcr_91490_snt(config);

	/* basic machine hardware */
	Z80(config, m_ipu, 7372800/2);
	m_ipu->set_daisy_config(mcr_ipu_daisy_chain);
	m_ipu->set_addrmap(AS_PROGRAM, &mcr_nflfoot_state::ipu_91695_map);
	m_ipu->set_addrmap(AS_IO, &mcr_nflfoot_state::ipu_91695_portmap);

	subdevice<timer_device>("scantimer")->set_callback(FUNC(mcr_nflfoot_state::ipu_interrupt));

	Z80CTC(config, m_ipu_ctc, 7372800/2 /* same as "ipu" */);
	m_ipu_ctc->intr_callback().set_inputline(m_ipu, INPUT_LINE_IRQ0);

	Z80PIO(config, m_ipu_pio0, 7372800/2);
	m_ipu_pio0->out_int_callback().set_inputline(m_ipu, INPUT_LINE_IRQ0);

	Z80PIO(config, m_ipu_pio1, 7372800/2);
	m_ipu_pio1->out_int_callback().set_inputline(m_ipu, INPUT_LINE_IRQ0);

	Z80SIO(config, m_ipu_sio, 7372800/2); // Z8440BB1
	m_ipu_sio->out_int_callback().set_inputline(m_ipu, INPUT_LINE_IRQ0);
	m_ipu_sio->out_txda_callback().set(FUNC(mcr_nflfoot_state::sio_txda_w));
	m_ipu_sio->out_txdb_callback().set(FUNC(mcr_nflfoot_state::sio_txdb_w));
}


/* 91490 CPU board plus 90908/90913/91483 sound board plus Turbo Cheap Squeak sound board */
void mcr_state::mcr_91490_tcs(machine_config &config)
{
	mcr_91490(config);

	/* basic machine hardware */
	MIDWAY_TURBO_CHEAP_SQUEAK(config, m_turbo_cheap_squeak);
	m_turbo_cheap_squeak->add_route(ALL_OUTPUTS, "lspeaker", 1.0);
	m_turbo_cheap_squeak->add_route(ALL_OUTPUTS, "rspeaker", 1.0);
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( solarfox )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sfcpu.3b",     0x0000, 0x1000, CRC(8c40f6eb) SHA1(a323897cfa8771edd28d58d806913e62110a2689) )
	ROM_LOAD( "sfcpu.4b",     0x1000, 0x1000, CRC(4d47bd7e) SHA1(0cfa09f2c1fe6d662c3a96abc43edf431ccf6d02) )
	ROM_LOAD( "sfcpu.5b",     0x2000, 0x1000, CRC(b52c3bd5) SHA1(bc289641830a3c6640303b1a799c378bf456bed1) )
	ROM_LOAD( "sfcpu.4d",     0x3000, 0x1000, CRC(bd5d25ba) SHA1(b7be1250dfb6af9cc0f9c6b446fb183528eab7de) )
	ROM_LOAD( "sfcpu.5d",     0x4000, 0x1000, CRC(dd57d817) SHA1(059a020313cf929130d1ae9a80f3b91c54fe7699) )
	ROM_LOAD( "sfcpu.6d",     0x5000, 0x1000, CRC(bd993cd9) SHA1(c074a6a40d0b9c0f4bf3fc5982263c89549fb338) )
	ROM_LOAD( "sfcpu.7d",     0x6000, 0x1000, CRC(8ad8731d) SHA1(ffd19c3fbad3c5a240ab27963812cc300f3d7b89) )

	ROM_REGION( 0x10000, "ssio:cpu", 0 )
	ROM_LOAD( "sfsnd.7a",     0x0000, 0x1000, CRC(cdecf83a) SHA1(5acd2709e214408d756b39916bb98cd4ecda7988) )
	ROM_LOAD( "sfsnd.8a",     0x1000, 0x1000, CRC(cb7788cb) SHA1(9e86f9131a6f0fc96dd436e21baf45e215ee65f4) )
	ROM_LOAD( "sfsnd.9a",     0x2000, 0x1000, CRC(304896ce) SHA1(00ff640eab50022da980cdc5ce8cedebaaebc9cf) )

	ROM_REGION( 0x02000, "gfx1", 0 )
	ROM_LOAD( "sfcpu.4g",     0x0000, 0x1000, CRC(ba019a60) SHA1(81923f8c51eedfef6f5e9d6ace1d785d7afafc14) )
	ROM_LOAD( "sfcpu.5g",     0x1000, 0x1000, CRC(7ff0364e) SHA1(b04034fc076008302931a485e00762dc34660498) )

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "sfvid.1a",     0x0000, 0x2000, CRC(9d9b5d7e) SHA1(4896c532a3d5763284a4403e8558f634f7b968d8) )
	ROM_LOAD( "sfvid.1b",     0x2000, 0x2000, CRC(78801e83) SHA1(23b5811a03fe4ad576c5313d2205203577300159) )
	ROM_LOAD( "sfvid.1d",     0x4000, 0x2000, CRC(4d8445cf) SHA1(fbe427da0e758b79eb2230713f2cd12e6f8bdeb7) )
	ROM_LOAD( "sfvid.1e",     0x6000, 0x2000, CRC(3da25495) SHA1(e7b703bc8caca7497af92efc869c6f1b7dbc8bf1) )
ROM_END


ROM_START( kick )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1200a-v2.b3",  0x0000, 0x1000, CRC(65924917) SHA1(4fbc7161f4b03bc395c775fa6239a23bf7357e89) )
	ROM_LOAD( "1300b-v2.b4",  0x1000, 0x1000, CRC(27929f52) SHA1(e03a550792df68eeb2a1f5177309fe01b5fcaa3d) )
	ROM_LOAD( "1400c-v2.b5",  0x2000, 0x1000, CRC(69107ce6) SHA1(4aedbb9f1072e315f7e5e3c8559f2995146f4b9d) )
	ROM_LOAD( "1500d-v2.d4",  0x3000, 0x1000, CRC(04a23aa1) SHA1(9a70ca3dc6db984dbb490076cf0ec11cc213f199) )
	ROM_LOAD( "1600e-v2.d5",  0x4000, 0x1000, CRC(1d2834c0) SHA1(176fad90ab14c922a575c3d12a2c8a339d1518d4) )
	ROM_LOAD( "1700f-v2.d6",  0x5000, 0x1000, CRC(ddf84ce1) SHA1(6f80b9a5cbd75b6e4af569ca4bcfcde7daaad64f) )

	ROM_REGION( 0x10000, "ssio:cpu", 0 )
	ROM_LOAD( "4200-a.a7",    0x0000, 0x1000, CRC(9e35c02e) SHA1(92afd0126dcfb2d4401927b2cf261090e186b6fa) )
	ROM_LOAD( "4300-b.a8",    0x1000, 0x1000, CRC(ca2b7c28) SHA1(fdcca3b755822c045c3c321cccc3f58112e2ad11) )
	ROM_LOAD( "4400-c.a9",    0x2000, 0x1000, CRC(d1901551) SHA1(fd7d6059f8ac59f95ae6f8ef12fbfce7ed16ec12) )
	ROM_LOAD( "4500-d.a10",   0x3000, 0x1000, CRC(d36ddcdc) SHA1(2d3ec83b9fa5a9d309c393a0c3ee45f0ba8192c9) )

	ROM_REGION( 0x02000, "gfx1", 0 )
	ROM_LOAD( "1800g-v2.g4",  0x0000, 0x1000, CRC(b4d120f3) SHA1(e61ae236f14eb1f519e196046fe8240c10932c2e) )
	ROM_LOAD( "1900h-v2.g5",  0x1000, 0x1000, CRC(c3ba4893) SHA1(76998db55519d7f1cf0f6b51d4f8ad7161b3a92e) )

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "2600a-v2.1e",  0x0000, 0x2000, CRC(2c5d6b55) SHA1(c326b8c7bcb903ea3a1a721443a37e1e8eabe975) )
	ROM_LOAD( "2700b-v2.1d",  0x2000, 0x2000, CRC(565ea97d) SHA1(4a30a371ad407bf774cf08bf528f824675383698) )
	ROM_LOAD( "2800c-v2.1b",  0x4000, 0x2000, CRC(f3be56a1) SHA1(eb3eb0379a918a2959565572d88f9b0f021d1c2a) )
	ROM_LOAD( "2900d-v2.1a",  0x6000, 0x2000, CRC(77da795e) SHA1(ebebc8551e7a7534d89bb8458eb2576713cfbeaf) )
ROM_END

ROM_START( kickman )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1200-a-ur.b3",  0x0000, 0x1000, CRC(d8cd9f0f) SHA1(a55f4423d57510256fb9b20b1bded06636c7bd05) )
	ROM_LOAD( "1300-b-ur.b4",  0x1000, 0x1000, CRC(4dee27bb) SHA1(b411d0c05339ae92732c89a4d0b7038d6b360475) )
	ROM_LOAD( "1400-c-ur.b5",  0x2000, 0x1000, CRC(06f070c9) SHA1(02eb19296e6c32544041ab5bf3dbb5a4f20c8ace) )
	ROM_LOAD( "1500-d-ur.d4",  0x3000, 0x1000, CRC(8d95b740) SHA1(93287324b599ec50dd84cc2dc70e82ccd8314a8a) )
	ROM_LOAD( "1600-e-ur.d5",  0x4000, 0x1000, CRC(f24bc0d7) SHA1(31dc996898c01f3427403e396a47444732904674) )
	ROM_LOAD( "1700-f-ur.d6",  0x5000, 0x1000, CRC(672361fc) SHA1(010029460c25935f2156eb64c9109c26ce40b752) )

	ROM_REGION( 0x10000, "ssio:cpu", 0 )
	ROM_LOAD( "4200-a.a7",    0x0000, 0x1000, CRC(9e35c02e) SHA1(92afd0126dcfb2d4401927b2cf261090e186b6fa) )
	ROM_LOAD( "4300-b.a8",    0x1000, 0x1000, CRC(ca2b7c28) SHA1(fdcca3b755822c045c3c321cccc3f58112e2ad11) )
	ROM_LOAD( "4400-c.a9",    0x2000, 0x1000, CRC(d1901551) SHA1(fd7d6059f8ac59f95ae6f8ef12fbfce7ed16ec12) )
	ROM_LOAD( "4500-d.a10",   0x3000, 0x1000, CRC(d36ddcdc) SHA1(2d3ec83b9fa5a9d309c393a0c3ee45f0ba8192c9) )

	ROM_REGION( 0x02000, "gfx1", 0 )
	ROM_LOAD( "1800g-v2.g4",  0x0000, 0x1000, CRC(b4d120f3) SHA1(e61ae236f14eb1f519e196046fe8240c10932c2e) )
	ROM_LOAD( "1900h-v2.g5",  0x1000, 0x1000, CRC(c3ba4893) SHA1(76998db55519d7f1cf0f6b51d4f8ad7161b3a92e) )

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "2600a-v2.1e",  0x0000, 0x2000, CRC(2c5d6b55) SHA1(c326b8c7bcb903ea3a1a721443a37e1e8eabe975) )
	ROM_LOAD( "2700b-v2.1d",  0x2000, 0x2000, CRC(565ea97d) SHA1(4a30a371ad407bf774cf08bf528f824675383698) )
	ROM_LOAD( "2800c-v2.1b",  0x4000, 0x2000, CRC(f3be56a1) SHA1(eb3eb0379a918a2959565572d88f9b0f021d1c2a) )
	ROM_LOAD( "2900d-v2.1a",  0x6000, 0x2000, CRC(77da795e) SHA1(ebebc8551e7a7534d89bb8458eb2576713cfbeaf) )
ROM_END

ROM_START( kickc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1200-a.b3",    0x0000, 0x1000, CRC(22fa42ed) SHA1(3922ce1f13e21cae9ee8d1af58f2bbe83d5eb979) )
	ROM_LOAD( "1300-b.b4",    0x1000, 0x1000, CRC(afaca819) SHA1(383f40d49e4c256e9eb83e778c140b0b97860f69) )
	ROM_LOAD( "1400-c.b5",    0x2000, 0x1000, CRC(6054ee56) SHA1(77da5a8d6209c746b233808a452dbf1917da7f3e) )
	ROM_LOAD( "1500-d.d4",    0x3000, 0x1000, CRC(263af0f3) SHA1(f8c511852f48b6fb5617d5b3666df53b08584db5) )
	ROM_LOAD( "1600-e.d5",    0x4000, 0x1000, CRC(eaaa78a7) SHA1(3c057d486f3938561fb9947e0463b1255ae04ef9) )
	ROM_LOAD( "1700-f.d6",    0x5000, 0x1000, CRC(c06c880f) SHA1(d5ac5682de316b9cb09d433e2c02746efadd2a81) )

	ROM_REGION( 0x10000, "ssio:cpu", 0 )
	ROM_LOAD( "4200-a.a7",    0x0000, 0x1000, CRC(9e35c02e) SHA1(92afd0126dcfb2d4401927b2cf261090e186b6fa) )
	ROM_LOAD( "4300-b.a8",    0x1000, 0x1000, CRC(ca2b7c28) SHA1(fdcca3b755822c045c3c321cccc3f58112e2ad11) )
	ROM_LOAD( "4400-c.a9",    0x2000, 0x1000, CRC(d1901551) SHA1(fd7d6059f8ac59f95ae6f8ef12fbfce7ed16ec12) )
	ROM_LOAD( "4500-d.a10",   0x3000, 0x1000, CRC(d36ddcdc) SHA1(2d3ec83b9fa5a9d309c393a0c3ee45f0ba8192c9) )

	ROM_REGION( 0x02000, "gfx1", 0 )
	ROM_LOAD( "1000-g.g4",    0x0000, 0x1000, CRC(acdae4f6) SHA1(8af065a7fe07f5b444314adc0ac69073e7bd5354) )
	ROM_LOAD( "1100-h.g5",    0x1000, 0x1000, CRC(dbb18c96) SHA1(a437d5d83b0638ca498e066bab1222bb5f39d5fb) )

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "2600-a.1e",    0x0000, 0x2000, CRC(74b409d7) SHA1(13769060c02ab514d70e29a2452512f342189fa9) )
	ROM_LOAD( "2700-b.1d",    0x2000, 0x2000, CRC(78eda36c) SHA1(5cf9da6f364f586f324e7ac529db0dc273498320) )
	ROM_LOAD( "2800-c.1b",    0x4000, 0x2000, CRC(c93e0170) SHA1(a7efdb6fd13dccd8d8d10de61b87585828bde6ac) )
	ROM_LOAD( "2900-d.1a",    0x6000, 0x2000, CRC(91e59383) SHA1(bf87642cc747f1abbd80c6f529adfa60a1d9bc9e) )
ROM_END


ROM_START( dpoker )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "vppp.b3",      0x0000, 0x1000, CRC(2a76ded2) SHA1(3bb5467f0cbca07e72f6d61deb2687b7c1c839c8) )
	ROM_LOAD( "vppp.b4",      0x1000, 0x1000, CRC(d6948faa) SHA1(4b9c3df45b1333c4e5595b790b439c104ab89eda) )
	ROM_LOAD( "vppp.b5",      0x2000, 0x1000, CRC(a49916e5) SHA1(b21268c87bc5c3ea6e072846c5b97a7ae1e3995e) )
	ROM_LOAD( "vppp.d4",      0x3000, 0x1000, CRC(c496934f) SHA1(cf27b6fb764fbf7ed4c5e1030c43498f0ac60c1c) )
	ROM_LOAD( "vppp.d5",      0x4000, 0x1000, CRC(84f4bd38) SHA1(d1c53d8c6ba10d3bc343fe120eecbca70d48b759) )
	ROM_LOAD( "vppp.d6",      0x5000, 0x1000, CRC(b0023bf1) SHA1(77a57a42dd403ef56f334ca295b5b43e94b99598) )
	ROM_LOAD( "vppp.d7",      0x6000, 0x1000, CRC(a4012f5a) SHA1(011e77a6634fbb02a6ae99fe6685c92f2fad3fee) )

	ROM_REGION( 0x10000, "ssio:cpu", ROMREGION_ERASEFF ) // a9 and a10 are unpopulated
	ROM_LOAD( "vssp.a7",      0x0000, 0x1000, CRC(f78b2283) SHA1(b69f987112f82c92f5cf38ca2577195a4972cc47) )
	ROM_LOAD( "vssp.a8",      0x1000, 0x1000, CRC(3f531bd0) SHA1(0e860bb5af83985b7143abdab5715f46c6a1e804) )

	ROM_REGION( 0x02000, "gfx1", 0 )
	ROM_LOAD( "vpbg.g4",      0x0000, 0x1000, CRC(9fe9aad8) SHA1(f9174bcce3886548b8c18c5a06995d5c69ce5486) )
	ROM_LOAD( "vpbg.g5",      0x1000, 0x1000, CRC(d43aeaae) SHA1(7bbabf9641c73154a769aa9bfc56ab0bc050e964) )

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "vpfg.a1",      0x6000, 0x2000, CRC(d76ec7dd) SHA1(a7be6f05a988c59c7f83c640dd0ef824ea4ce839) )
	ROM_LOAD( "vpfg.b1",      0x4000, 0x2000, CRC(cdba9a7d) SHA1(9076b52363cd84ae6e01ac46b369d7719536fac0) )
	ROM_LOAD( "vpfg.d1",      0x2000, 0x2000, CRC(c661cace) SHA1(d5755f0c32a7d9ba283822daaf37ccbd2f6667de) )
	ROM_LOAD( "vpfg.e1",      0x0000, 0x2000, CRC(acb3b469) SHA1(9769d6cfd49cba48264034fb5aed1d1b84ebba4c) )
ROM_END


ROM_START( shollow )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sh-pro.00",    0x0000, 0x2000, CRC(95e2b800) SHA1(8781295e21e5202288293fb9eb558cb1835b11ea) )
	ROM_LOAD( "sh-pro.01",    0x2000, 0x2000, CRC(b99f6ff8) SHA1(575cdba5bf9965c7765ce913a3376c7ba36b8291) )
	ROM_LOAD( "sh-pro.02",    0x4000, 0x2000, CRC(1202c7b2) SHA1(ebae4711f124a34f19305da16657ece805e1da26) )
	ROM_LOAD( "sh-pro.03",    0x6000, 0x2000, CRC(0a64afb9) SHA1(3ac6660d2b0166c58ec4fecfe37ce36e545064df) )
	ROM_LOAD( "sh-pro.04",    0x8000, 0x2000, CRC(22fa9175) SHA1(fd8ea76b3a7ffaf48fc11dd3b7c58e548e3e57c5) )
	ROM_LOAD( "sh-pro.05",    0xa000, 0x2000, CRC(1716e2bb) SHA1(771e4c20d63e4e1d99723e6355db67064a278ae5) )

	ROM_REGION( 0x10000, "ssio:cpu", 0 )
	ROM_LOAD( "sh-snd.01",    0x0000, 0x1000, CRC(55a297cc) SHA1(b34f37fca61cdba26b5671feee991d133b8697a4) )
	ROM_LOAD( "sh-snd.02",    0x1000, 0x1000, CRC(46fc31f6) SHA1(9b1a56962b2d210b1013bc35de780c6d5b3eb4bc) )
	ROM_LOAD( "sh-snd.03",    0x2000, 0x1000, CRC(b1f4a6a8) SHA1(ba724f9cc0cc35dd31d4ad8b36a51da9d6cbfbcf) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "sh-bg.00",     0x0000, 0x2000, CRC(3e2b333c) SHA1(b4347d0b3d6149e94da4a38684c0fab931e76cc5) )
	ROM_LOAD( "sh-bg.01",     0x2000, 0x2000, CRC(d1d70cc4) SHA1(550e2e8c0bcbf7913b218fe48cd3324622c8d0f4) )

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "sh-fg.00",     0x0000, 0x2000, CRC(33f4554e) SHA1(88dd803158d3c65429b79d2aaf8334130315aa93) )
	ROM_LOAD( "sh-fg.01",     0x2000, 0x2000, CRC(ba1a38b4) SHA1(cee5f0a66e40c779f1bfbb4bfafe5301385e7dae) )
	ROM_LOAD( "sh-fg.02",     0x4000, 0x2000, CRC(6b57f6da) SHA1(71165df59dd5ca037116dc0f46fd4f6aac6dbfd1) )
	ROM_LOAD( "sh-fg.03",     0x6000, 0x2000, CRC(37ea9d07) SHA1(82e006b01ee12390308be530d4b68fc2404b6b91) )
ROM_END

ROM_START( shollow2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sh-pro.00",    0x0000, 0x2000, CRC(95e2b800) SHA1(8781295e21e5202288293fb9eb558cb1835b11ea) )
	ROM_LOAD( "sh-pro.01",    0x2000, 0x2000, CRC(b99f6ff8) SHA1(575cdba5bf9965c7765ce913a3376c7ba36b8291) )
	ROM_LOAD( "sh-pro.02",    0x4000, 0x2000, CRC(1202c7b2) SHA1(ebae4711f124a34f19305da16657ece805e1da26) )
	ROM_LOAD( "sh-pro.03",    0x6000, 0x2000, CRC(0a64afb9) SHA1(3ac6660d2b0166c58ec4fecfe37ce36e545064df) )
	ROM_LOAD( "sh-pro.04",    0x8000, 0x2000, CRC(22fa9175) SHA1(fd8ea76b3a7ffaf48fc11dd3b7c58e548e3e57c5) )
	ROM_LOAD( "sh-pro.05",    0xa000, 0x2000, CRC(1716e2bb) SHA1(771e4c20d63e4e1d99723e6355db67064a278ae5) )

	ROM_REGION( 0x10000, "ssio:cpu", 0 )
	ROM_LOAD( "snd-0.a7",     0x0000, 0x1000, CRC(9d815bb3) SHA1(51af797e08dbe3921e11ce70c3d0da50979336a4) )
	ROM_LOAD( "snd-1.a8",     0x1000, 0x1000, CRC(9f253412) SHA1(a526e864073a2f9e67e2cbe53ab17fe726336241) )
	ROM_LOAD( "snd-2.a9",     0x2000, 0x1000, CRC(7783d6c6) SHA1(1fb2117532e7da28afdb9837bcb6848165cf8173) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "sh-bg.00",     0x0000, 0x2000, CRC(3e2b333c) SHA1(b4347d0b3d6149e94da4a38684c0fab931e76cc5) )
	ROM_LOAD( "sh-bg.01",     0x2000, 0x2000, CRC(d1d70cc4) SHA1(550e2e8c0bcbf7913b218fe48cd3324622c8d0f4) )

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "sh-fg.00",     0x0000, 0x2000, CRC(33f4554e) SHA1(88dd803158d3c65429b79d2aaf8334130315aa93) )
	ROM_LOAD( "sh-fg.01",     0x2000, 0x2000, CRC(ba1a38b4) SHA1(cee5f0a66e40c779f1bfbb4bfafe5301385e7dae) )
	ROM_LOAD( "sh-fg.02",     0x4000, 0x2000, CRC(6b57f6da) SHA1(71165df59dd5ca037116dc0f46fd4f6aac6dbfd1) )
	ROM_LOAD( "sh-fg.03",     0x6000, 0x2000, CRC(37ea9d07) SHA1(82e006b01ee12390308be530d4b68fc2404b6b91) )
ROM_END

/*
  TRON (Set 1 - 8/9), program ROMs stamped as "AUG 9"
*/

ROM_START( tron )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* ROM's located on the Super CPU Board (90010) */
	ROM_LOAD( "scpu-pga_lctn-c2_tron_aug_9.c2", 0x0000, 0x2000, CRC(0de0471a) SHA1(378847604a6a9c079d887348010ab9539d5f6195) ) // labeled: SCPU-PGA   LCTN-C2   TRON   (c)1982 BALLY   MIDWAY
	ROM_LOAD( "scpu-pgb_lctn-c3_tron_aug_9.c3", 0x2000, 0x2000, CRC(8ddf8717) SHA1(e0c294afa8ba0b0ba89e3e0fb3ff6d8fc4398e32) ) // labeled: SCPU-PGB   LCTN-C3   TRON   (c)1982 BALLY   MIDWAY
	ROM_LOAD( "scpu-pgc_lctn-c4_tron_aug_9.c4", 0x4000, 0x2000, CRC(4241e3a0) SHA1(24c1bd2f31e194542571c391c5dccf21354115c2) ) // labeled: SCPU-PGC   LCTN-C4   TRON   (c)1982 BALLY   MIDWAY
	ROM_LOAD( "scpu-pgd_lctn-c5_tron_aug_9.c5", 0x6000, 0x2000, CRC(035d2fe7) SHA1(1b827ca30a439d2f4cc94fcc0e90ee0cf87e018c) ) // labeled: SCPU-PGD   LCTN-C5   TRON   (c)1982 BALLY   MIDWAY
	ROM_LOAD( "scpu-pge_lctn-c6_tron_aug_9.c6", 0x8000, 0x2000, CRC(24c185d8) SHA1(45ac7c53f6f4eba5c7bf3fc6559cddd3821eddad) ) // labeled: SCPU-PGE   LCTN-C6   TRON   (c)1982 BALLY   MIDWAY
	ROM_LOAD( "scpu-pgf_lctn-c7_tron_aug_9.c7", 0xa000, 0x2000, CRC(38c4bbaf) SHA1(a7cd496ce75199b8279ea963520cf70d5f562bb2) ) // labeled: SCPU-PGF   LCTN-C7   TRON   (c)1982 BALLY   MIDWAY

	ROM_REGION( 0x10000, "ssio:cpu", 0 ) /* ROM's located on the Super Sound I/O Board (90913) */
	ROM_LOAD( "ssi-0a_lctn-a7_tron.a7",   0x0000, 0x1000, CRC(765e6eba) SHA1(42efeefc8571dfc237c0be3368248f1e56add92e) ) // labeled: SSI/0A   LCTN-A7   TRON   (c)1982 BALLY   MIDWAY
	ROM_LOAD( "ssi-0b_lctn-a8_tron.a8",   0x1000, 0x1000, CRC(1b90ccdd) SHA1(0876e5eeaa63bb8cc97f3634a6ddd8a29a9b012f) ) // labeled: SSI/0B   LCTN-A8   TRON   (c)1982 BALLY   MIDWAY
	ROM_LOAD( "ssi-0c_lctn-a9_tron.a9",   0x2000, 0x1000, CRC(3a4bc629) SHA1(ce8452a99a313ae7429de471bbea39de08c9fd4b) ) // labeled: SSI/0C   LCTN-A9   TRON   (c)1982 BALLY   MIDWAY

	ROM_REGION( 0x04000, "gfx1", 0 ) /* ROM's located on the Super CPU Board (90010) */
	ROM_LOAD( "scpu-bgg_lctn-g3_tron.g3", 0x0000, 0x2000, CRC(1a9ed2f5) SHA1(b0d85b47873ac8ad475da18b9540d37232cb2b7c) ) // labeled: SCPU-BGG   LCTN-G3   TRON   (c)1982 BALLY   MIDWAY
	ROM_LOAD( "scpu-bgh_lctn-g4_tron.g4", 0x2000, 0x2000, CRC(3220f974) SHA1(a38ea5f1db27f05d9689db838ce7a8de98f34837) ) // labeled: SCPU-BGH   LCTN-G4   TRON   (c)1982 BALLY   MIDWAY

	ROM_REGION( 0x08000, "gfx2", 0 ) /* ROM's located on the MCR/II Video Gen Board (91399) */
	ROM_LOAD( "vga_lctn-e1_tron.e1",      0x0000, 0x2000, CRC(bc036d1d) SHA1(c5d54d0b80ac768ccf6fdd32cad1ef6359fa324c) ) // labeled: VGA   LCTN-E1   TRON   (c)1982 BALLY   MIDWAY
	ROM_LOAD( "vgb_lctn-dc1_tron.dc1",    0x2000, 0x2000, CRC(58ee14d3) SHA1(5fb4268c9c73bdfc3b1e866618979aea3f219bbc) ) // labeled: VGB   LCTN-DC1   TRON   (c)1982 BALLY   MIDWAY
	ROM_LOAD( "vgc_lctn-cb1_tron.cb1",    0x4000, 0x2000, CRC(3329f9d4) SHA1(11f4d744374e475d2c5b195a9f70888414529dd3) ) // labeled: VGC   LCTN-CB1   TRON   (c)1982 BALLY   MIDWAY
	ROM_LOAD( "vgd_lctn-a1_tron.a1",      0x6000, 0x2000, CRC(9743f873) SHA1(71ed80ecd8caaf9fce1d7010f95c4678c9bd7102) ) // labeled: VGD   LCTN-A1   TRON   (c)1982 BALLY   MIDWAY

	ROM_REGION( 0x0005, "scpu_pals", 0) /* PAL's located on the Super CPU Board (90010) */
	ROM_LOAD( "0066-313bx-xxqx.a12.bin", 0x0000, 0x0001, NO_DUMP)
	ROM_LOAD( "0066-315bx-xxqx.b12.bin", 0x0000, 0x0001, NO_DUMP)
	ROM_LOAD( "0066-322bx-xx0x.e3.bin",  0x0000, 0x0001, NO_DUMP)
	ROM_LOAD( "0066-316bx-xxqx.g11.bin", 0x0000, 0x0001, NO_DUMP)
	ROM_LOAD( "0066-314bx-xxqx.g12.bin", 0x0000, 0x0001, NO_DUMP)
ROM_END

/*
  TRON (Set 2 - 6/25), program ROMs stamped as "JUN 25" - also known to be stamped 6 25 or handwritten 6-25
*/

ROM_START( tron2 )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* ROM's located on the Super CPU Board (90010) */
	ROM_LOAD( "scpu-pga_lctn-c2_tron_jun_25.c2", 0x0000, 0x2000, CRC(5151770b) SHA1(26f4d830de7be228528e462dd628f439629a2641) ) // labeled: SCPU-PGA   LCTN-C2   TRON   (c)1982 BALLY   MIDWAY
	ROM_LOAD( "scpu-pgb_lctn-c3_tron_jun_25.c3", 0x2000, 0x2000, CRC(8ddf8717) SHA1(e0c294afa8ba0b0ba89e3e0fb3ff6d8fc4398e32) ) // labeled: SCPU-PGB   LCTN-C3   TRON   (c)1982 BALLY   MIDWAY
	ROM_LOAD( "scpu-pgc_lctn-c4_tron_jun_25.c4", 0x4000, 0x2000, CRC(4241e3a0) SHA1(24c1bd2f31e194542571c391c5dccf21354115c2) ) // labeled: SCPU-PGC   LCTN-C4   TRON   (c)1982 BALLY   MIDWAY
	ROM_LOAD( "scpu-pgd_lctn-c5_tron_jun_25.c5", 0x6000, 0x2000, CRC(035d2fe7) SHA1(1b827ca30a439d2f4cc94fcc0e90ee0cf87e018c) ) // labeled: SCPU-PGD   LCTN-C5   TRON   (c)1982 BALLY   MIDWAY
	ROM_LOAD( "scpu-pge_lctn-c6_tron_jun_25.c6", 0x8000, 0x2000, CRC(24c185d8) SHA1(45ac7c53f6f4eba5c7bf3fc6559cddd3821eddad) ) // labeled: SCPU-PGE   LCTN-C6   TRON   (c)1982 BALLY   MIDWAY
	ROM_LOAD( "scpu-pgf_lctn-c7_tron_jun_25.c7", 0xa000, 0x2000, CRC(38c4bbaf) SHA1(a7cd496ce75199b8279ea963520cf70d5f562bb2) ) // labeled: SCPU-PGF   LCTN-C7   TRON   (c)1982 BALLY   MIDWAY

	ROM_REGION( 0x10000, "ssio:cpu", 0 ) /* ROM's located on the Super Sound I/O Board (90913) */
	ROM_LOAD( "ssi-0a_lctn-a7_tron.a7",   0x0000, 0x1000, CRC(765e6eba) SHA1(42efeefc8571dfc237c0be3368248f1e56add92e) ) // labeled: SSI/0A   LCTN-A7   TRON   (c)1982 BALLY   MIDWAY - also stamped JUN 25
	ROM_LOAD( "ssi-0b_lctn-a8_tron.a8",   0x1000, 0x1000, CRC(1b90ccdd) SHA1(0876e5eeaa63bb8cc97f3634a6ddd8a29a9b012f) ) // labeled: SSI/0B   LCTN-A8   TRON   (c)1982 BALLY   MIDWAY - also stamped JUN 25
	ROM_LOAD( "ssi-0c_lctn-a9_tron.a9",   0x2000, 0x1000, CRC(3a4bc629) SHA1(ce8452a99a313ae7429de471bbea39de08c9fd4b) ) // labeled: SSI/0C   LCTN-A9   TRON   (c)1982 BALLY   MIDWAY - also stamped JUN 25

	ROM_REGION( 0x04000, "gfx1", 0 ) /* ROM's located on the Super CPU Board (90010) */
	ROM_LOAD( "scpu-bgg_lctn-g3_tron.g3", 0x0000, 0x2000, CRC(1a9ed2f5) SHA1(b0d85b47873ac8ad475da18b9540d37232cb2b7c) ) // labeled: SCPU-BGG   LCTN-G3   TRON   (c)1982 BALLY   MIDWAY
	ROM_LOAD( "scpu-bgh_lctn-g4_tron.g4", 0x2000, 0x2000, CRC(3220f974) SHA1(a38ea5f1db27f05d9689db838ce7a8de98f34837) ) // labeled: SCPU-BGH   LCTN-G4   TRON   (c)1982 BALLY   MIDWAY

	ROM_REGION( 0x08000, "gfx2", 0 ) /* ROM's located on the MCR/II Video Gen Board (91399) */
	ROM_LOAD( "vga_lctn-e1_tron.e1",      0x0000, 0x2000, CRC(bc036d1d) SHA1(c5d54d0b80ac768ccf6fdd32cad1ef6359fa324c) ) // labeled: VGA   LCTN-E1   TRON   (c)1982 BALLY   MIDWAY
	ROM_LOAD( "vgb_lctn-dc1_tron.dc1",    0x2000, 0x2000, CRC(58ee14d3) SHA1(5fb4268c9c73bdfc3b1e866618979aea3f219bbc) ) // labeled: VGB   LCTN-DC1   TRON   (c)1982 BALLY   MIDWAY
	ROM_LOAD( "vgc_lctn-cb1_tron.cb1",    0x4000, 0x2000, CRC(3329f9d4) SHA1(11f4d744374e475d2c5b195a9f70888414529dd3) ) // labeled: VGC   LCTN-CB1   TRON   (c)1982 BALLY   MIDWAY
	ROM_LOAD( "vgd_lctn-a1_tron.a1",      0x6000, 0x2000, CRC(9743f873) SHA1(71ed80ecd8caaf9fce1d7010f95c4678c9bd7102) ) // labeled: VGD   LCTN-A1   TRON   (c)1982 BALLY   MIDWAY

	ROM_REGION( 0x0005, "scpu_pals", 0) /* PAL's located on the Super CPU Board (90010) */
	ROM_LOAD( "0066-313bx-xxqx.a12.bin", 0x0000, 0x0001, NO_DUMP)
	ROM_LOAD( "0066-315bx-xxqx.b12.bin", 0x0000, 0x0001, NO_DUMP)
	ROM_LOAD( "0066-322bx-xx0x.e3.bin",  0x0000, 0x0001, NO_DUMP)
	ROM_LOAD( "0066-316bx-xxqx.g11.bin", 0x0000, 0x0001, NO_DUMP)
	ROM_LOAD( "0066-314bx-xxqx.g12.bin", 0x0000, 0x0001, NO_DUMP)
ROM_END

/*
  TRON (Set 3 - 6/17), program ROMs stamped as "JUN 17" - also known to be stamped 6 17 or handwritten 6-17
*/

ROM_START( tron3 )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* ROM's located on the Super CPU Board (90010) */
	ROM_LOAD( "scpu-pga_lctn-c2_tron_jun_17.c2", 0x0000, 0x2000, CRC(fc33afd7) SHA1(99a2ed972c3db477f35a7162079563367864f207) ) // labeled: SCPU-PGA   LCTN-C2   TRON   (c)1982 BALLY   MIDWAY
	ROM_LOAD( "scpu-pgb_lctn-c3_tron_jun_17.c3", 0x2000, 0x2000, CRC(7d9e22ac) SHA1(16a6e9651d5f764e8762fd8d6e53d13fda7473de) ) // labeled: SCPU-PGB   LCTN-C3   TRON   (c)1982 BALLY   MIDWAY
	ROM_LOAD( "scpu-pgc_lctn-c4_tron_jun_17.c4", 0x4000, 0x2000, CRC(902011c6) SHA1(17ac768a0fd1278ae83414f0d67d6ac8337f4773) ) // labeled: SCPU-PGC   LCTN-C4   TRON   (c)1982 BALLY   MIDWAY
	ROM_LOAD( "scpu-pgd_lctn-c5_tron_jun_17.c5", 0x6000, 0x2000, CRC(86477e89) SHA1(196f0d3930d10bfe4ddee82ce8b28bb99324069e) ) // labeled: SCPU-PGD   LCTN-C5   TRON   (c)1982 BALLY   MIDWAY
	ROM_LOAD( "scpu-pge_lctn-c6_tron_jun_17.c6", 0x8000, 0x2000, CRC(ea198fa8) SHA1(d8c97ea87d504e77edc38c87c2953c8c4f1a405b) ) // labeled: SCPU-PGE   LCTN-C6   TRON   (c)1982 BALLY   MIDWAY
	ROM_LOAD( "scpu-pgf_lctn-c7_tron_jun_17.c7", 0xa000, 0x2000, CRC(4325fb08) SHA1(70727aa37354425315d8a8b3ca07bbe91f7e8f08) ) // labeled: SCPU-PGF   LCTN-C7   TRON   (c)1982 BALLY   MIDWAY

	ROM_REGION( 0x10000, "ssio:cpu", 0 ) /* ROM's located on the Super Sound I/O Board (90913) */
	ROM_LOAD( "ssi-0a_lctn-a7_tron.a7",   0x0000, 0x1000, CRC(765e6eba) SHA1(42efeefc8571dfc237c0be3368248f1e56add92e) ) // labeled: SSI/0A   LCTN-A7   TRON   (c)1982 BALLY   MIDWAY
	ROM_LOAD( "ssi-0b_lctn-a8_tron.a8",   0x1000, 0x1000, CRC(1b90ccdd) SHA1(0876e5eeaa63bb8cc97f3634a6ddd8a29a9b012f) ) // labeled: SSI/0B   LCTN-A8   TRON   (c)1982 BALLY   MIDWAY
	ROM_LOAD( "ssi-0c_lctn-a9_tron.a9",   0x2000, 0x1000, CRC(3a4bc629) SHA1(ce8452a99a313ae7429de471bbea39de08c9fd4b) ) // labeled: SSI/0C   LCTN-A9   TRON   (c)1982 BALLY   MIDWAY

	ROM_REGION( 0x04000, "gfx1", 0 ) /* ROM's located on the Super CPU Board (90010) */
	ROM_LOAD( "scpu-bgg_lctn-g3_tron.g3", 0x0000, 0x2000, CRC(1a9ed2f5) SHA1(b0d85b47873ac8ad475da18b9540d37232cb2b7c) ) // labeled: SCPU-BGG   LCTN-G3   TRON   (c)1982 BALLY   MIDWAY
	ROM_LOAD( "scpu-bgh_lctn-g4_tron.g4", 0x2000, 0x2000, CRC(3220f974) SHA1(a38ea5f1db27f05d9689db838ce7a8de98f34837) ) // labeled: SCPU-BGH   LCTN-G4   TRON   (c)1982 BALLY   MIDWAY

	ROM_REGION( 0x08000, "gfx2", 0 ) /* ROM's located on the MCR/II Video Gen Board (91399) */
	ROM_LOAD( "vga_lctn-e1_tron.e1",      0x0000, 0x2000, CRC(bc036d1d) SHA1(c5d54d0b80ac768ccf6fdd32cad1ef6359fa324c) ) // labeled: VGA   LCTN-E1   TRON   (c)1982 BALLY   MIDWAY
	ROM_LOAD( "vgb_lctn-dc1_tron.dc1",    0x2000, 0x2000, CRC(58ee14d3) SHA1(5fb4268c9c73bdfc3b1e866618979aea3f219bbc) ) // labeled: VGB   LCTN-DC1   TRON   (c)1982 BALLY   MIDWAY
	ROM_LOAD( "vgc_lctn-cb1_tron.cb1",    0x4000, 0x2000, CRC(3329f9d4) SHA1(11f4d744374e475d2c5b195a9f70888414529dd3) ) // labeled: VGC   LCTN-CB1   TRON   (c)1982 BALLY   MIDWAY
	ROM_LOAD( "vgd_lctn-a1_tron.a1",      0x6000, 0x2000, CRC(9743f873) SHA1(71ed80ecd8caaf9fce1d7010f95c4678c9bd7102) ) // labeled: VGD   LCTN-A1   TRON   (c)1982 BALLY   MIDWAY

	ROM_REGION( 0x0005, "scpu_pals", 0) /* PAL's located on the Super CPU Board (90010) */
	ROM_LOAD( "0066-313bx-xxqx.a12.bin", 0x0000, 0x0001, NO_DUMP)
	ROM_LOAD( "0066-315bx-xxqx.b12.bin", 0x0000, 0x0001, NO_DUMP)
	ROM_LOAD( "0066-322bx-xx0x.e3.bin",  0x0000, 0x0001, NO_DUMP)
	ROM_LOAD( "0066-316bx-xxqx.g11.bin", 0x0000, 0x0001, NO_DUMP)
	ROM_LOAD( "0066-314bx-xxqx.g12.bin", 0x0000, 0x0001, NO_DUMP)
ROM_END

/*
  TRON (Set 4 - 6/15), program ROMs stamped as "JUN 15" - also known to be stamped 6 15 or handwritten 6-15
*/

ROM_START( tron4 )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* ROM's located on the Super CPU Board (90010) */
	ROM_LOAD( "scpu-pga_lctn-c2_tron_jun_15.c2", 0x0000, 0x2000, CRC(09d7a95a) SHA1(630f4c43204df0510926c5fc07e0c1b3783cf5e4) ) // labeled: SCPU-PGA   LCTN-C2   TRON   (c)1982 BALLY   MIDWAY
	ROM_LOAD( "scpu-pgb_lctn-c3_tron_jun_15.c3", 0x2000, 0x2000, CRC(b454337d) SHA1(c2306ad5401a3f60e231be59198610a36b527f1f) ) // labeled: SCPU-PGB   LCTN-C3   TRON   (c)1982 BALLY   MIDWAY
	ROM_LOAD( "scpu-pgc_lctn-c4_tron_jun_15.c4", 0x4000, 0x2000, CRC(ac1836ff) SHA1(a8d20facf91b4c93082f8e71a663e5da8b036ecb) ) // labeled: SCPU-PGC   LCTN-C4   TRON   (c)1982 BALLY   MIDWAY
	ROM_LOAD( "scpu-pgd_lctn-c5_tron_jun_15.c5", 0x6000, 0x2000, CRC(1a7bec6d) SHA1(4d47020494e0963db22473a459e97c1c06c4b5ee) ) // labeled: SCPU-PGD   LCTN-C5   TRON   (c)1982 BALLY   MIDWAY
	ROM_LOAD( "scpu-pge_lctn-c6_tron_jun_15.c6", 0x8000, 0x2000, CRC(ea198fa8) SHA1(d8c97ea87d504e77edc38c87c2953c8c4f1a405b) ) // labeled: SCPU-PGE   LCTN-C6   TRON   (c)1982 BALLY   MIDWAY
	ROM_LOAD( "scpu-pgf_lctn-c7_tron_jun_15.c7", 0xa000, 0x2000, CRC(790ee743) SHA1(14dc84b2bbaab22772e0579f11fe0bf136a0ddab) ) // labeled: SCPU-PGF   LCTN-C7   TRON   (c)1982 BALLY   MIDWAY

	ROM_REGION( 0x10000, "ssio:cpu", 0 ) /* ROM's located on the Super Sound I/O Board (90913) */
	ROM_LOAD( "tron_snd-0_may_10_82.a7",  0x0000, 0x1000, CRC(2cbb332b) SHA1(48d1cbb336733588af728a3d0e02c8613d2b5fb2) ) // hand written TRON SND-0, stamped MAY 10 '82
	ROM_LOAD( "tron_snd-1_may_10_82.a8",  0x1000, 0x1000, CRC(1355b7e6) SHA1(61ed045212da67cd449910ae601058cf209b37e5) ) // hand written TRON SND-1, stamped MAY 10 '82
	ROM_LOAD( "tron_snd-2_may_10_82.a9",  0x2000, 0x1000, CRC(6dd4b7c9) SHA1(1ce78c242d1a7d9a4524a663a42fc8bc2870053a) ) // hand written TRON SND-2, stamped MAY 10 '82

	ROM_REGION( 0x04000, "gfx1", 0 ) /* ROM's located on the Super CPU Board (90010) */
	ROM_LOAD( "scpu-bgg_lctn-g3_tron.g3", 0x0000, 0x2000, CRC(1a9ed2f5) SHA1(b0d85b47873ac8ad475da18b9540d37232cb2b7c) ) // labeled: SCPU-BGG   LCTN-G3   TRON   (c)1982 BALLY   MIDWAY
	ROM_LOAD( "scpu-bgh_lctn-g4_tron.g4", 0x2000, 0x2000, CRC(3220f974) SHA1(a38ea5f1db27f05d9689db838ce7a8de98f34837) ) // labeled: SCPU-BGH   LCTN-G4   TRON   (c)1982 BALLY   MIDWAY

	ROM_REGION( 0x08000, "gfx2", 0 ) /* ROM's located on the MCR/II Video Gen Board (91399) */
	ROM_LOAD( "vga_lctn-e1_tron.e1",      0x0000, 0x2000, CRC(bc036d1d) SHA1(c5d54d0b80ac768ccf6fdd32cad1ef6359fa324c) ) // labeled: VGA   LCTN-E1   TRON   (c)1982 BALLY   MIDWAY
	ROM_LOAD( "vgb_lctn-dc1_tron.dc1",    0x2000, 0x2000, CRC(58ee14d3) SHA1(5fb4268c9c73bdfc3b1e866618979aea3f219bbc) ) // labeled: VGB   LCTN-DC1   TRON   (c)1982 BALLY   MIDWAY
	ROM_LOAD( "vgc_lctn-cb1_tron.cb1",    0x4000, 0x2000, CRC(3329f9d4) SHA1(11f4d744374e475d2c5b195a9f70888414529dd3) ) // labeled: VGC   LCTN-CB1   TRON   (c)1982 BALLY   MIDWAY
	ROM_LOAD( "vgd_lctn-a1_tron.a1",      0x6000, 0x2000, CRC(9743f873) SHA1(71ed80ecd8caaf9fce1d7010f95c4678c9bd7102) ) // labeled: VGD   LCTN-A1   TRON   (c)1982 BALLY   MIDWAY

	ROM_REGION( 0x0005, "scpu_pals", 0) /* PAL's located on the Super CPU Board (90010) */
	ROM_LOAD( "0066-313bx-xxqx.a12.bin", 0x0000, 0x0001, NO_DUMP)
	ROM_LOAD( "0066-315bx-xxqx.b12.bin", 0x0000, 0x0001, NO_DUMP)
	ROM_LOAD( "0066-322bx-xx0x.e3.bin",  0x0000, 0x0001, NO_DUMP)
	ROM_LOAD( "0066-316bx-xxqx.g11.bin", 0x0000, 0x0001, NO_DUMP)
	ROM_LOAD( "0066-314bx-xxqx.g12.bin", 0x0000, 0x0001, NO_DUMP)
ROM_END

/*
  TRON (Set 5 - 5/12), handwritten labels for program & sound ROMs (though the sound ROMs were stamped MAY 10 '82)
*/

ROM_START( tron5 )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* ROM's located on the Super CPU Board (90010) */
	ROM_LOAD( "tron_pro-0_5-12.c2", 0x0000, 0x2000, CRC(ccc4119f) SHA1(a07677ed4dae4062849867d91be96ddf342826a3) ) // all labels were hand written
	ROM_LOAD( "tron_pro-1_5-12.c3", 0x2000, 0x2000, CRC(153f148c) SHA1(3d0065a9a92d4a1ad85cc907da18c03c94328e5d) )
	ROM_LOAD( "tron_pro-2_5-12.c4", 0x4000, 0x2000, CRC(e62bb8a1) SHA1(817987251e015c3290507ebc88ee7acc07c5accd) )
	ROM_LOAD( "tron_pro-3_5-12.c5", 0x6000, 0x2000, CRC(dbc06c91) SHA1(4d75c84844ade35c577e91787639dfdc48f9cb28) )
	ROM_LOAD( "tron_pro-4_5-12.c6", 0x8000, 0x2000, CRC(30adb624) SHA1(5baf0294dfb01be5bdf34cfc63d764d7f34657e1) )
	ROM_LOAD( "tron_pro-5_5-12.c7", 0xa000, 0x2000, CRC(191c72bb) SHA1(bbf406ebfb80e46f1f20bedd0d20d0154f009317) )

	ROM_REGION( 0x10000, "ssio:cpu", 0 ) /* ROM's located on the Super Sound I/O Board (90913) */
	ROM_LOAD( "tron_snd-0_may_10_82.a7",  0x0000, 0x1000, CRC(2cbb332b) SHA1(48d1cbb336733588af728a3d0e02c8613d2b5fb2) ) // hand written TRON SND-0, stamped MAY 10 '82
	ROM_LOAD( "tron_snd-1_may_10_82.a8",  0x1000, 0x1000, CRC(1355b7e6) SHA1(61ed045212da67cd449910ae601058cf209b37e5) ) // hand written TRON SND-1, stamped MAY 10 '82
	ROM_LOAD( "tron_snd-2_may_10_82.a9",  0x2000, 0x1000, CRC(6dd4b7c9) SHA1(1ce78c242d1a7d9a4524a663a42fc8bc2870053a) ) // hand written TRON SND-2, stamped MAY 10 '82

	ROM_REGION( 0x04000, "gfx1", 0 ) /* ROM's located on the Super CPU Board (90010) */
	ROM_LOAD( "scpu-bgg_lctn-g3_tron.g3", 0x0000, 0x2000, CRC(1a9ed2f5) SHA1(b0d85b47873ac8ad475da18b9540d37232cb2b7c) ) // labeled: SCPU-BGG   LCTN-G3   TRON   (c)1982 BALLY   MIDWAY
	ROM_LOAD( "scpu-bgh_lctn-g4_tron.g4", 0x2000, 0x2000, CRC(3220f974) SHA1(a38ea5f1db27f05d9689db838ce7a8de98f34837) ) // labeled: SCPU-BGH   LCTN-G4   TRON   (c)1982 BALLY   MIDWAY

	ROM_REGION( 0x08000, "gfx2", 0 ) /* ROM's located on the MCR/II Video Gen Board (91399) */
	ROM_LOAD( "vga_lctn-e1_tron.e1",      0x0000, 0x2000, CRC(bc036d1d) SHA1(c5d54d0b80ac768ccf6fdd32cad1ef6359fa324c) ) // labeled: VGA   LCTN-E1   TRON   (c)1982 BALLY   MIDWAY
	ROM_LOAD( "vgb_lctn-dc1_tron.dc1",    0x2000, 0x2000, CRC(58ee14d3) SHA1(5fb4268c9c73bdfc3b1e866618979aea3f219bbc) ) // labeled: VGB   LCTN-DC1   TRON   (c)1982 BALLY   MIDWAY
	ROM_LOAD( "vgc_lctn-cb1_tron.cb1",    0x4000, 0x2000, CRC(3329f9d4) SHA1(11f4d744374e475d2c5b195a9f70888414529dd3) ) // labeled: VGC   LCTN-CB1   TRON   (c)1982 BALLY   MIDWAY
	ROM_LOAD( "vgd_lctn-a1_tron.a1",      0x6000, 0x2000, CRC(9743f873) SHA1(71ed80ecd8caaf9fce1d7010f95c4678c9bd7102) ) // labeled: VGD   LCTN-A1   TRON   (c)1982 BALLY   MIDWAY

	ROM_REGION( 0x0005, "scpu_pals", 0) /* PAL's located on the Super CPU Board (90010) */
	ROM_LOAD( "0066-313bx-xxqx.a12.bin", 0x0000, 0x0001, NO_DUMP)
	ROM_LOAD( "0066-315bx-xxqx.b12.bin", 0x0000, 0x0001, NO_DUMP)
	ROM_LOAD( "0066-322bx-xx0x.e3.bin",  0x0000, 0x0001, NO_DUMP)
	ROM_LOAD( "0066-316bx-xxqx.g11.bin", 0x0000, 0x0001, NO_DUMP)
	ROM_LOAD( "0066-314bx-xxqx.g12.bin", 0x0000, 0x0001, NO_DUMP)
ROM_END

ROM_START( tronger )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* ROM's located on the Super CPU Board (90010) */
	ROM_LOAD( "scpu_pga.c2", 0x0000, 0x2000, CRC(ba14603d) SHA1(1cc30c4ea659926314343f00ccbcfe9021f4de26) )
	ROM_LOAD( "scpu_pgb.c3", 0x2000, 0x2000, CRC(063a748f) SHA1(aefe647e9b562d6a9da1ec32a9d403fce7e62012) )
	ROM_LOAD( "scpu_pgc.c4", 0x4000, 0x2000, CRC(6ca50365) SHA1(76e17284da7c3ddf752d67b5e80d3c145f64068e) )
	ROM_LOAD( "scpu_pgd.c5", 0x6000, 0x2000, CRC(b5b241c9) SHA1(4a9bde02387365912b3e9878428c8aa1f87a365a) )
	ROM_LOAD( "scpu_pge.c6", 0x8000, 0x2000, CRC(04597abe) SHA1(7a896b9415a2479da8519329568e5fb8a429d03e) )
	ROM_LOAD( "scpu_pgf.c7", 0xa000, 0x2000, CRC(3908e404) SHA1(d61b73c87ba4b0ab8751d9c653b663b1342d5d73) )

	ROM_REGION( 0x10000, "ssio:cpu", 0 ) /* ROM's located on the Super Sound I/O Board (90913) */
	ROM_LOAD( "ssi-0a_lctn-a7_tron.a7",   0x0000, 0x1000, CRC(765e6eba) SHA1(42efeefc8571dfc237c0be3368248f1e56add92e) ) // labeled: SSI/0A   LCTN-A7   TRON   (c)1982 BALLY   MIDWAY
	ROM_LOAD( "ssi-0b_lctn-a8_tron.a8",   0x1000, 0x1000, CRC(1b90ccdd) SHA1(0876e5eeaa63bb8cc97f3634a6ddd8a29a9b012f) ) // labeled: SSI/0B   LCTN-A8   TRON   (c)1982 BALLY   MIDWAY
	ROM_LOAD( "ssi-0c_lctn-a9_tron.a9",   0x2000, 0x1000, CRC(3a4bc629) SHA1(ce8452a99a313ae7429de471bbea39de08c9fd4b) ) // labeled: SSI/0C   LCTN-A9   TRON   (c)1982 BALLY   MIDWAY

	ROM_REGION( 0x04000, "gfx1", 0 ) /* ROM's located on the Super CPU Board (90010) */
	ROM_LOAD( "scpu-bgg_lctn-g3_tron.g3", 0x0000, 0x2000, CRC(1a9ed2f5) SHA1(b0d85b47873ac8ad475da18b9540d37232cb2b7c) ) // labeled: SCPU-BGG   LCTN-G3   TRON   (c)1982 BALLY   MIDWAY
	ROM_LOAD( "scpu-bgh_lctn-g4_tron.g4", 0x2000, 0x2000, CRC(3220f974) SHA1(a38ea5f1db27f05d9689db838ce7a8de98f34837) ) // labeled: SCPU-BGH   LCTN-G4   TRON   (c)1982 BALLY   MIDWAY

	ROM_REGION( 0x08000, "gfx2", 0 ) /* ROM's located on the MCR/II Video Gen Board (91399) */
	ROM_LOAD( "vga_lctn-e1_tron.e1",      0x0000, 0x2000, CRC(bc036d1d) SHA1(c5d54d0b80ac768ccf6fdd32cad1ef6359fa324c) ) // labeled: VGA   LCTN-E1   TRON   (c)1982 BALLY   MIDWAY
	ROM_LOAD( "vgb_lctn-dc1_tron.dc1",    0x2000, 0x2000, CRC(58ee14d3) SHA1(5fb4268c9c73bdfc3b1e866618979aea3f219bbc) ) // labeled: VGB   LCTN-DC1   TRON   (c)1982 BALLY   MIDWAY
	ROM_LOAD( "vgc_lctn-cb1_tron.cb1",    0x4000, 0x2000, CRC(3329f9d4) SHA1(11f4d744374e475d2c5b195a9f70888414529dd3) ) // labeled: VGC   LCTN-CB1   TRON   (c)1982 BALLY   MIDWAY
	ROM_LOAD( "vgd_lctn-a1_tron.a1",      0x6000, 0x2000, CRC(9743f873) SHA1(71ed80ecd8caaf9fce1d7010f95c4678c9bd7102) ) // labeled: VGD   LCTN-A1   TRON   (c)1982 BALLY   MIDWAY

	ROM_REGION( 0x0005, "scpu_pals", 0) /* PAL's located on the Super CPU Board (90010) */
	ROM_LOAD( "0066-313bx-xxqx.a12.bin", 0x0000, 0x0001, NO_DUMP)
	ROM_LOAD( "0066-315bx-xxqx.b12.bin", 0x0000, 0x0001, NO_DUMP)
	ROM_LOAD( "0066-322bx-xx0x.e3.bin",  0x0000, 0x0001, NO_DUMP)
	ROM_LOAD( "0066-316bx-xxqx.g11.bin", 0x0000, 0x0001, NO_DUMP)
	ROM_LOAD( "0066-314bx-xxqx.g12.bin", 0x0000, 0x0001, NO_DUMP)
ROM_END

ROM_START( kroozr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "kozmkcpu.2d",  0x0000, 0x2000, CRC(61e02045) SHA1(42ba00f28466870407be96e864fadba5d6908648) )
	ROM_LOAD( "kozmkcpu.3d",  0x2000, 0x2000, CRC(caabed57) SHA1(1effe2285939ad55b5ff254c7e5ea3512e843f84) )
	ROM_LOAD( "kozmkcpu.4d",  0x4000, 0x2000, CRC(2bc83fc7) SHA1(71a3b32309e368f4afb650444d6427ac4e05b5fa) )
	ROM_LOAD( "kozmkcpu.5d",  0x6000, 0x2000, CRC(a0ec38c1) SHA1(adf3ef36355d255e4ebc0d4dc86b9d7910e26b03) )
	ROM_LOAD( "kozmkcpu.6d",  0x8000, 0x2000, CRC(7044f2b6) SHA1(55b64c9233fe0c8b351688fab29aad049d49faf2) )

	ROM_REGION( 0x10000, "ssio:cpu", 0 )
	ROM_LOAD( "kozmksnd.7a",  0x0000, 0x1000, CRC(6736e433) SHA1(d43216ef34a67f047b7c35001767d838386add7d) )
	ROM_LOAD( "kozmksnd.8a",  0x1000, 0x1000, CRC(ea9cd919) SHA1(a1533b2857c881c83adce2c7bbfaa4a3148ead8e) )
	ROM_LOAD( "kozmksnd.9a",  0x2000, 0x1000, CRC(9dfa7994) SHA1(0a2d824e9fe1d48c43027f5f10f4c43476f08e07) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "kozmkcpu.3g",  0x0000, 0x2000, CRC(eda6ed2d) SHA1(c999b2aa31a83800ff205cf665f344d67eb149a0) )
	ROM_LOAD( "kozmkcpu.4g",  0x2000, 0x2000, CRC(ddde894b) SHA1(b2fe32196e947e992a959af806d6684227aaf3a2) )

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "kozmkvid.1e",  0x0000, 0x2000, CRC(ca60e2cc) SHA1(e26bf1284a0e74933ac193178ac8b834e5cf0146) )
	ROM_LOAD( "kozmkvid.1d",  0x2000, 0x2000, CRC(4e23b35b) SHA1(322221ea207ad0eeb0e711f3af473f71b70f7128) )
	ROM_LOAD( "kozmkvid.1b",  0x4000, 0x2000, CRC(c6041ba7) SHA1(9bd55f06d360657af5c56062053221dd24027978) )
	ROM_LOAD( "kozmkvid.1a",  0x6000, 0x2000, CRC(b57fb0ff) SHA1(e4dde9fb09a0e5051352ba05d776ebbe0cd81504) )
ROM_END


ROM_START( domino )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dmanpg0.bin",  0x0000, 0x2000, CRC(3bf3bb1c) SHA1(0dc38b47fc1f6828cd1dc2e14f43448d878ccd8a) )
	ROM_LOAD( "dmanpg1.bin",  0x2000, 0x2000, CRC(85cf1d69) SHA1(32198a2b8fbfb5b9593f083034ef4762b7ab1a27) )
	ROM_LOAD( "dmanpg2.bin",  0x4000, 0x2000, CRC(7dd2177a) SHA1(b4b17e2580679fbe340d8b8d8cb7171c49ae0a21) )
	ROM_LOAD( "dmanpg3.bin",  0x6000, 0x2000, CRC(f2e0aa44) SHA1(2f04dc74c69dfe3847d5e4330e560b0a9f18c33a) )

	ROM_REGION( 0x10000, "ssio:cpu", 0 )
	ROM_LOAD( "dm-a7.snd",    0x0000, 0x1000, CRC(fa982dcc) SHA1(970340bfa0ac13ad8c2bf5adc21d7ca7aa9e525a) )
	ROM_LOAD( "dm-a8.snd",    0x1000, 0x1000, CRC(72839019) SHA1(4aa278cfb00fac76cba88600bb300ee88ec3f7ee) )
	ROM_LOAD( "dm-a9.snd",    0x2000, 0x1000, CRC(ad760da7) SHA1(024fce0f5d46e82b66c4283925556130735b863e) )
	ROM_LOAD( "dm-a10.snd",   0x3000, 0x1000, CRC(958c7287) SHA1(0dd1ae1b6073f19925d0ec1ba1090d736e0a7cf6) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "dmanbg0.bin",  0x0000, 0x2000, CRC(9163007f) SHA1(a7b2e3ed68105ff99841776e8114279f557c90ed) )
	ROM_LOAD( "dmanbg1.bin",  0x2000, 0x2000, CRC(28615c56) SHA1(d4b91c092f594b1216d9a135345ba8955e5e5ec5) )

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "dmanfg0.bin",  0x0000, 0x2000, CRC(0b1f9f9e) SHA1(051d0b126d060300283a5572e1179c7624cedbbf) )
	ROM_LOAD( "dmanfg1.bin",  0x2000, 0x2000, CRC(16aa4b9b) SHA1(700f7dc3ce20dacce38dc40783b28fc9b4c1443a) )
	ROM_LOAD( "dmanfg2.bin",  0x4000, 0x2000, CRC(4a8e76b8) SHA1(563b9db39940060265b771f3b0c4a2055963cf2b) )
	ROM_LOAD( "dmanfg3.bin",  0x6000, 0x2000, CRC(1f39257e) SHA1(645f9b7e8bd2254167d15567c3bd577d3a574f7d) )
ROM_END


ROM_START( dominoa ) // PCB had Satan's Hollow stickers with 'DM' written over them. Also there was a date of 'Nov 10' stamped on the ROM stickers on the CPU PCB.
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "d2",  0x0000, 0x2000, CRC(c7b6331e) SHA1(9288d1d20818ff0083e0c82f11e02e1abeed587f) )
	ROM_LOAD( "d3",  0x2000, 0x2000, CRC(81115c86) SHA1(bf6a6233dfb5cfbb7d31cc4fc61d9a9e9c8ed3e3) )
	ROM_LOAD( "d4",  0x4000, 0x2000, CRC(5cf32c55) SHA1(71dfb1f05f1ab760f2a8130ced925be62d1fb1d8) )
	ROM_LOAD( "d5",  0x6000, 0x2000, CRC(4f89b68c) SHA1(5546c65315b568cfba7fed387afb0e041d8742a1) )

	ROM_REGION( 0x10000, "ssio:cpu", 0 )
	ROM_LOAD( "a7",    0x0000, 0x1000, CRC(fa982dcc) SHA1(970340bfa0ac13ad8c2bf5adc21d7ca7aa9e525a) )
	ROM_LOAD( "a8",    0x1000, 0x1000, CRC(72839019) SHA1(4aa278cfb00fac76cba88600bb300ee88ec3f7ee) )
	ROM_LOAD( "a9",    0x2000, 0x1000, CRC(ad760da7) SHA1(024fce0f5d46e82b66c4283925556130735b863e) )
	ROM_LOAD( "a10",   0x3000, 0x1000, CRC(958c7287) SHA1(0dd1ae1b6073f19925d0ec1ba1090d736e0a7cf6) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "g3",  0x0000, 0x2000, CRC(9163007f) SHA1(a7b2e3ed68105ff99841776e8114279f557c90ed) )
	ROM_LOAD( "g4",  0x2000, 0x2000, CRC(28615c56) SHA1(d4b91c092f594b1216d9a135345ba8955e5e5ec5) )

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "e1",  0x0000, 0x2000, CRC(0b1f9f9e) SHA1(051d0b126d060300283a5572e1179c7624cedbbf) )
	ROM_LOAD( "d1",  0x2000, 0x2000, CRC(16aa4b9b) SHA1(700f7dc3ce20dacce38dc40783b28fc9b4c1443a) )
	ROM_LOAD( "b1",  0x4000, 0x2000, CRC(4a8e76b8) SHA1(563b9db39940060265b771f3b0c4a2055963cf2b) )
	ROM_LOAD( "a1",  0x6000, 0x2000, CRC(1f39257e) SHA1(645f9b7e8bd2254167d15567c3bd577d3a574f7d) )
ROM_END


ROM_START( wacko )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wackocpu.2d",  0x0000, 0x2000, CRC(c98e29b6) SHA1(38fbb663c238b354925b34b67de246d8ba3356e1) )
	ROM_LOAD( "wackocpu.3d",  0x2000, 0x2000, CRC(90b89774) SHA1(1f9cbe8134e7d7f797d88c0cb4be3d8c39ca03a3) )
	ROM_LOAD( "wackocpu.4d",  0x4000, 0x2000, CRC(515edff7) SHA1(9288cb5efb51086ef8610eecf8e3feae1da9fc2a) )
	ROM_LOAD( "wackocpu.5d",  0x6000, 0x2000, CRC(9b01bf32) SHA1(d209ba2503d7b54786f74107bb399313a08a09ba) )

	ROM_REGION( 0x10000, "ssio:cpu", 0 )
	ROM_LOAD( "wackosnd.7a",  0x0000, 0x1000, CRC(1a58763f) SHA1(37f0870d67d52c86ae2d188e9beaa56a3a8fa130) )
	ROM_LOAD( "wackosnd.8a",  0x1000, 0x1000, CRC(a4e3c771) SHA1(fe677090423e1d80cde07d2e74be8380d8c55e95) )
	ROM_LOAD( "wackosnd.9a",  0x2000, 0x1000, CRC(155ba3dd) SHA1(51aaeeb68b2b7eb8238c7c3b06e84dcf44683ee9) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "wackocpu.3g",  0x0000, 0x2000, CRC(33160eb1) SHA1(7d66198cb84294cc3689d7f51365566f252d6252) )
	ROM_LOAD( "wackocpu.4g",  0x2000, 0x2000, CRC(daf37d7c) SHA1(37fe68b0a5984828ec03886860f84a20461713fc) )

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "wackovid.1e",  0x0000, 0x2000, CRC(dca59be7) SHA1(c78af8e0c99a6f58a35f8aa73bcee15ee8fda372) )
	ROM_LOAD( "wackovid.1d",  0x2000, 0x2000, CRC(a02f1672) SHA1(1d1b7eada4dae5e31dc1474b13cefd71926cbfc7) )
	ROM_LOAD( "wackovid.1b",  0x4000, 0x2000, CRC(7d899790) SHA1(aa320f4ac41d2a1acec4880a8d95dfe6406e210e) )
	ROM_LOAD( "wackovid.1a",  0x6000, 0x2000, CRC(080be3ad) SHA1(ead2be17d749a15841123e42d434aab573870fba) )
ROM_END


ROM_START( twotiger )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cpu_d2",  0x0000, 0x2000, CRC(a682ed24) SHA1(e4418143b02739e417c44e6b4089354778e8d77f) )
	ROM_LOAD( "cpu_d3",  0x2000, 0x2000, CRC(5b48fde9) SHA1(52e07ffdd360631ea322935af5fb560afe3006ea) )
	ROM_LOAD( "cpu_d4",  0x4000, 0x2000, CRC(f1ab8c4d) SHA1(0c410ddd2e1cd8a19c73bc0c7aca70d8c4308eeb) )
	ROM_LOAD( "cpu_d5",  0x6000, 0x2000, CRC(d7129900) SHA1(af5093082cfbc9fa4b42cfc74e62adbf9b6c63db) )

	ROM_REGION( 0x10000, "ssio:cpu", 0 )
	ROM_LOAD( "ssio_a7",   0x0000, 0x1000, CRC(64ddc16c) SHA1(e119e1702ea00ffb86d413ed8e68b4e9dfefa79e) )
	ROM_LOAD( "ssio_a8",   0x1000, 0x1000, CRC(c3467612) SHA1(c968776d9561a7ac67e95a987b6d826ec2dc748e) )
	ROM_LOAD( "ssio_a9",   0x2000, 0x1000, CRC(c50f7b2d) SHA1(0f4779d4955d500c50b544d945fa78a5428b86ce) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "2tgrbg0.bin",  0x0000, 0x2000, CRC(52f69068) SHA1(30422e66ae1a6fe8c10494037758758dcd1211d1) )
	ROM_LOAD( "2tgrbg1.bin",  0x2000, 0x2000, CRC(758d4f7d) SHA1(272127f802bccf47958b5dcc949b7468b1ba4512) )

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "vid_d1",  0x0000, 0x2000, CRC(da5f49da) SHA1(9396d708d5771ec19f7abdadd8c8f874af68ab10) )
	ROM_LOAD( "vid_c1",  0x2000, 0x2000, CRC(62ed737b) SHA1(954c1f17da2ceb77e710faf0822d29381b873a07) )
	ROM_LOAD( "vid_b1",  0x4000, 0x2000, CRC(0939921e) SHA1(f52d3475232557959e501f70765a4ad472300e84) )
	ROM_LOAD( "vid_a1",  0x6000, 0x2000, CRC(ef515824) SHA1(983af762733405b96351ef4910f4f4be40c4880e) )
ROM_END

ROM_START( twotigerc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2tgrpg0.bin",  0x0000, 0x2000, CRC(e77a924b) SHA1(c1720a8a6ca8e6551ef38d565d5f18db7fbd5d2e) )
	ROM_LOAD( "2tgrpg1.bin",  0x2000, 0x2000, CRC(2699ebdc) SHA1(bed7c5fe8783dab79e7563f6b2348b9a980bdcc7) )
	ROM_LOAD( "2tgrpg2.bin",  0x4000, 0x2000, CRC(b5ca3f17) SHA1(ac51eefe9ff49bc358daf58525e529070684ed1b) )
	ROM_LOAD( "2tgrpg3.bin",  0x6000, 0x2000, CRC(8aa82049) SHA1(6e42d082d29986f5c0698ae39750fb8f9eb1e6cd) )

	ROM_REGION( 0x10000, "ssio:cpu", 0 )
	ROM_LOAD( "2tgra7.bin",   0x0000, 0x1000, CRC(4620d970) SHA1(2c2c1da84199b846575a6291dc235f30539959fa) )
	ROM_LOAD( "2tgra8.bin",   0x1000, 0x1000, CRC(e95d8cfe) SHA1(846d5543596bb86cf08f998056c1fc695cb4f62c) )
	ROM_LOAD( "2tgra9.bin",   0x2000, 0x1000, CRC(81e6ce0e) SHA1(77e145e150763bfe5760ac3e4f68218a65b9bfe0) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "2tgrbg0.bin",  0x0000, 0x2000, CRC(52f69068) SHA1(30422e66ae1a6fe8c10494037758758dcd1211d1) )
	ROM_LOAD( "2tgrbg1.bin",  0x2000, 0x2000, CRC(758d4f7d) SHA1(272127f802bccf47958b5dcc949b7468b1ba4512) )

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "2tgrfg0.bin",  0x0000, 0x2000, CRC(4abf3ca0) SHA1(8cef27a1c91d32473a95e9da76db061466977559) )
	ROM_LOAD( "2tgrfg1.bin",  0x2000, 0x2000, CRC(fbcaffa5) SHA1(5145cd7cc08552388b8662b5d159b3af6344c51a) )
	ROM_LOAD( "2tgrfg2.bin",  0x4000, 0x2000, CRC(08e3e1a6) SHA1(c9c5ce072b235a299a3151f984d73bab97289e19) )
	ROM_LOAD( "2tgrfg3.bin",  0x6000, 0x2000, CRC(9b22697b) SHA1(4c3fa30ce1b645ab18c0e53ce0f3754133a267ad) )
ROM_END


ROM_START( journey )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "d2",           0x0000, 0x2000, CRC(f2618913) SHA1(eee323ac502cedefef0675a90c3c9c17f9610fc9) )
	ROM_LOAD( "d3",           0x2000, 0x2000, CRC(2f290d2e) SHA1(b672064969326e594b6a4225e73bf51d25f33871) )
	ROM_LOAD( "d4",           0x4000, 0x2000, CRC(cc6c0150) SHA1(83b00b137e0c343db9b61c90469d5e2392444fc3) )
	ROM_LOAD( "d5",           0x6000, 0x2000, CRC(c3023931) SHA1(e591a18c5fc8befcd9f2b93d9131374c572cdbcd) )
	ROM_LOAD( "d6",           0x8000, 0x2000, CRC(5d445c99) SHA1(df2bce203f510b4bda42bb7114b79eb0b2b4e2e0) )

	ROM_REGION( 0x10000, "ssio:cpu", 0 )
	ROM_LOAD( "a",            0x0000, 0x1000, CRC(2524a2aa) SHA1(4bd78b4fb42c2506fa6734419b42cbbe4c240e94) )
	ROM_LOAD( "b",            0x1000, 0x1000, CRC(b8e35814) SHA1(379308431d1204d6cb5ae8a13e378ec7b3fab0a9) )
	ROM_LOAD( "c",            0x2000, 0x1000, CRC(09c488cf) SHA1(7aa3321db748f2612693f8348e590369e8d48140) )
	ROM_LOAD( "d",            0x3000, 0x1000, CRC(3d627bee) SHA1(42239ee73ba88206d28fd9cff9787b11c40bb2f1) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "g3",           0x0000, 0x2000, CRC(c14558de) SHA1(f47624ec235f782559eff076758ff28366dbf21d) )
	ROM_LOAD( "g4",           0x2000, 0x2000, CRC(9104c1d0) SHA1(9ae732d6f1edb8656c54ac9b8fa6b35b342adc4b) )

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "a7",           0x0000, 0x2000, CRC(4ca2bb2d) SHA1(62ae3b30a2c52649d4c8f1264b9f8726c3ac81ce) )
	ROM_LOAD( "a8",           0x2000, 0x2000, CRC(4fb7925d) SHA1(a1f4a2bdd6e80a3a9e5f1e698c014b6f91dfb000) )
	ROM_LOAD( "a5",           0x4000, 0x2000, CRC(560c474f) SHA1(bb44fb5f1bd14b92bff110e74d8c9f22df07b47f) )
	ROM_LOAD( "a6",           0x6000, 0x2000, CRC(b1f31583) SHA1(3ebedacc75d031525d166b3265c136f0f7407d0f) )
	ROM_LOAD( "a3",           0x8000, 0x2000, CRC(f295afda) SHA1(de8086ca5750736eba939f80c089ba96b7e53300) )
	ROM_LOAD( "a4",           0xa000, 0x2000, CRC(765876a7) SHA1(9c477f5fae068f7e424939652e959711b3ad3a80) )
	ROM_LOAD( "a1",           0xc000, 0x2000, CRC(4af986f8) SHA1(56af9525a404bcf6d76b19318efe541189844210) )
	ROM_LOAD( "a2",           0xe000, 0x2000, CRC(b30cd2a7) SHA1(df1b36a3481fdec49f73d504f23951070c121291) )
ROM_END


ROM_START( tapper )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tapper_c.p.u._pg_0_1c_1-27-84.1c",   0x00000, 0x4000, CRC(bb060bb0) SHA1(ff5a729e36faea3758c8c7b345a42dd8bb465f44) ) /* labeled TAPPER C.P.U. PG 0 1C 1/27/84 */
	ROM_LOAD( "tapper_c.p.u._pg_1_2c_1-27-84.2c",   0x04000, 0x4000, CRC(fd9acc22) SHA1(b9f0396e2eba5772deec4725c41fa9de49658e72) ) /* labeled TAPPER C.P.U. PG 1 2C 1/27/84 */
	ROM_LOAD( "tapper_c.p.u._pg_2_3c_1-27-84.3c",   0x08000, 0x4000, CRC(b3755d41) SHA1(434d3c27b9f1e43def081d79b9f56dbce93a9207) ) /* labeled TAPPER C.P.U. PG 2 3C 1/27/84 */
	ROM_LOAD( "tapper_c.p.u._pg_3_4c_1-27-84.4c",   0x0c000, 0x2000, CRC(77273096) SHA1(5e4e2dc1703b39f588ba374f6a610f273d710532) ) /* labeled TAPPER C.P.U. PG 3 4C 1/27/84 */

	ROM_REGION( 0x10000, "ssio:cpu", 0 )
	ROM_LOAD( "tapper_sound_snd_0_a7_12-7-83.a7",   0x0000, 0x1000, CRC(0e8bb9d5) SHA1(9e281c340b7702523c86d56317efad9e3688e585) ) /* labeled TAPPER SOUND SND 0 A7 12/7/83 */
	ROM_LOAD( "tapper_sound_snd_1_a8_12-7-83.a8",   0x1000, 0x1000, CRC(0cf0e29b) SHA1(14334b9d2bfece3fe5bda0cbd53158ead8d27e53) ) /* labeled TAPPER SOUND SND 1 A8 12/7/83 */
	ROM_LOAD( "tapper_sound_snd_2_a9_12-7-83.a9",   0x2000, 0x1000, CRC(31eb6dc6) SHA1(b38bba5f12516d899e023f99147868e3402fbd7b) ) /* labeled TAPPER SOUND SND 2 A9 12/7/83 */
	ROM_LOAD( "tapper_sound_snd_3_a10_12-7-83.a10", 0x3000, 0x1000, CRC(01a9be6a) SHA1(0011407c1e886071282808c0a561789b1245a789) ) /* labeled TAPPER SOUND SND 3 A10 12/7/83 */

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "tapper_c.p.u._bg_1_6f_12-7-83.6f",   0x00000, 0x4000, CRC(2a30238c) SHA1(eb30b9bb654324340f0fc5b44776ac2440c1e869) ) /* labeled TAPPER C.P.U. BG 1 6F 12/7/83 */
	ROM_LOAD( "tapper_c.p.u._bg_0_5f_12-7-83.5f",   0x04000, 0x4000, CRC(394ab576) SHA1(23e29ec942e1e7516ae8068837af2d1c79592378) ) /* labeled TAPPER C.P.U. BG 0 5F 12/7/83 */

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "tapper_video_fg_1_a7_12-7-83.a7",   0x00000, 0x4000, CRC(32509011) SHA1(a38667573d235efe2dc515e52af05825fe4e0f30) ) /* labeled TAPPER VIDEO FG1 A7 12/7/83 */
	ROM_LOAD( "tapper_video_fg_0_a8_12-7-83.a8",   0x04000, 0x4000, CRC(8412c808) SHA1(2077f79177fda26f9c674b2ab525ec3833802059) ) /* labeled TAPPER VIDEO FG0 A8 12/7/83 */
	ROM_LOAD( "tapper_video_fg_3_a5_12-7-83.a5",   0x08000, 0x4000, CRC(818fffd4) SHA1(930142dd73fb30c4d3ec09a1e37517c6c6774024) ) /* labeled TAPPER VIDEO FG3 A5 12/7/83 */
	ROM_LOAD( "tapper_video_fg_2_a6_12-7-83.a6",   0x0c000, 0x4000, CRC(67e37690) SHA1(d553b8517c1d03a2be0b065f4da2fa99d9e6fb30) ) /* labeled TAPPER VIDEO FG2 A6 12/7/83 */
	ROM_LOAD( "tapper_video_fg_5_a3_12-7-83.a3",   0x10000, 0x4000, CRC(800f7c8a) SHA1(8aead89e1adaee5f0b679661c4bfba36e0d639e8) ) /* labeled TAPPER VIDEO FG5 A3 12/7/83 */
	ROM_LOAD( "tapper_video_fg_4_a4_12-7-83.a4",   0x14000, 0x4000, CRC(32674ee6) SHA1(402c166d50b4a693959b3f0706a7931a5daef6ce) ) /* labeled TAPPER VIDEO FG4 A4 12/7/83 */
	ROM_LOAD( "tapper_video_fg_7_a1_12-7-83.a1",   0x18000, 0x4000, CRC(070b4c81) SHA1(95879a455ecfe2e3de7fe2a75078f9e6934960f5) ) /* labeled TAPPER VIDEO FG7 A1 12/7/83 */
	ROM_LOAD( "tapper_video_fg_6_a2_12-7-83.a2",   0x1c000, 0x4000, CRC(a37aef36) SHA1(a24696f16d467d9eea4f25aef5f4c5ff55bf51ff) ) /* labeled TAPPER VIDEO FG6 A2 12/7/83 */
ROM_END

ROM_START( tapperg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tapper_c.p.u._pg_0_1c_1-27-84.1c",   0x00000, 0x4000, CRC(bb060bb0) SHA1(ff5a729e36faea3758c8c7b345a42dd8bb465f44) ) /* labeled TAPPER C.P.U. PG 0 1C 1/27/84 */
	ROM_LOAD( "tapper_c.p.u._pg_1_2c_1-27-84.2c",   0x04000, 0x4000, CRC(fd9acc22) SHA1(b9f0396e2eba5772deec4725c41fa9de49658e72) ) /* labeled TAPPER C.P.U. PG 1 2C 1/27/84 */
	ROM_LOAD( "tapper_c.p.u._pg_2_3c_1-27-84.3c",   0x08000, 0x4000, CRC(b3755d41) SHA1(434d3c27b9f1e43def081d79b9f56dbce93a9207) ) /* labeled TAPPER C.P.U. PG 2 3C 1/27/84 */
	ROM_LOAD( "tapper_c.p.u._pg_3_4c_1-27-84.4c",   0x0c000, 0x2000, CRC(77273096) SHA1(5e4e2dc1703b39f588ba374f6a610f273d710532) ) /* labeled TAPPER C.P.U. PG 3 4C 1/27/84 */

	ROM_REGION( 0x10000, "ssio:cpu", 0 )
	ROM_LOAD( "tapper_sound_snd_0_a7_12-7-83.a7",   0x0000, 0x1000, CRC(0e8bb9d5) SHA1(9e281c340b7702523c86d56317efad9e3688e585) ) /* labeled TAPPER SOUND SND 0 A7 12/7/83 */
	ROM_LOAD( "tapper_sound_snd_1_a8_12-7-83.a8",   0x1000, 0x1000, CRC(0cf0e29b) SHA1(14334b9d2bfece3fe5bda0cbd53158ead8d27e53) ) /* labeled TAPPER SOUND SND 1 A8 12/7/83 */
	ROM_LOAD( "tapper_sound_snd_2_a9_12-7-83.a9",   0x2000, 0x1000, CRC(31eb6dc6) SHA1(b38bba5f12516d899e023f99147868e3402fbd7b) ) /* labeled TAPPER SOUND SND 2 A9 12/7/83 */
	ROM_LOAD( "tapper_sound_snd_3_a10_12-7-83.a10", 0x3000, 0x1000, CRC(01a9be6a) SHA1(0011407c1e886071282808c0a561789b1245a789) ) /* labeled TAPPER SOUND SND 3 A10 12/7/83 */

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "tapper_c.p.u._bg_1_6f_12-7-83.6f",   0x00000, 0x4000, CRC(2a30238c) SHA1(eb30b9bb654324340f0fc5b44776ac2440c1e869) ) /* labeled TAPPER C.P.U. BG 1 6F 12/7/83 */
	ROM_LOAD( "tapper_c.p.u._bg_0_5f_12-7-83.5f",   0x04000, 0x4000, CRC(394ab576) SHA1(23e29ec942e1e7516ae8068837af2d1c79592378) ) /* labeled TAPPER C.P.U. BG 0 5F 12/7/83 */

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "fg1_a7.128",   0x00000, 0x4000, CRC(bac70b69) SHA1(7fd26cc8ff2faab86d04fcee2b5ec49ecf6b8143) ) /* Are these correct or hacked? Customers from ALL other sets match the 12/7/83 */
	ROM_LOAD( "fg0_a8.128",   0x04000, 0x4000, CRC(c300925d) SHA1(45df1ac033512be942460d678a7c1ba9dcef1937) ) /* graphics set including the Root Beer and Suntory sets. */
	ROM_LOAD( "fg3_a5.128",   0x08000, 0x4000, CRC(ecff6c23) SHA1(0b28e7e59eba983bc1929758f8dcaf315b7134a1) ) /* The 1/27/84 program set has been verified to come with 12/7/83 graphics in at least one machine */
	ROM_LOAD( "fg2_a6.128",   0x0c000, 0x4000, CRC(a4f2d1be) SHA1(faf631d4ee96edf6b2c4349780e2d89eaedf70ab) )
	ROM_LOAD( "fg5_a3.128",   0x10000, 0x4000, CRC(16ce38cb) SHA1(9829c9574fff0803973246f9d22311ca76be4e58) ) /* Noticed differences in customers: */
	ROM_LOAD( "fg4_a4.128",   0x14000, 0x4000, CRC(082a4059) SHA1(52672b853d67432fd80e4612fa54208c225d2444) ) /* Black cowboy: has yellow hat & green hair/mustache vs green hat and black hair/mustache   */
	ROM_LOAD( "fg7_a1.128",   0x18000, 0x4000, CRC(3b476abe) SHA1(6fe170695386fc77310384a5590a7cc3671ae853) ) /* Cowgirl: Has blue shirt, added color dither spots in hair & blue stripe on her hat vs red */
	ROM_LOAD( "fg6_a2.128",   0x1c000, 0x4000, CRC(6717264c) SHA1(5a6d30974e73f952694b8c09fb3a5393a76db4f2) ) /* shirt and blonde hair. On the points screen she has dithered purple hair vs brown hair    */
ROM_END

ROM_START( tappera )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tapper_c.p.u._pg_0_1c_1-12-84.1c",   0x00000, 0x4000, CRC(127171d1) SHA1(373e9a9d73b71e100c02862662d025f5ead2f94d) ) /* labeled TAPPER C.P.U. PRO 0 1C 1/12/84 */
	ROM_LOAD( "tapper_c.p.u._pg_1_2c_1-12-84.1c",   0x04000, 0x4000, CRC(9d6a47f7) SHA1(e493e46fc70a765e54bfdd7ba7ca570e6a5c79d6) ) /* labeled TAPPER C.P.U. PRO 1 2C 1/12/84 */
	ROM_LOAD( "tapper_c.p.u._pg_2_3c_1-12-84.3c",   0x08000, 0x4000, CRC(3a1f8778) SHA1(cb46a2248289ced7282b1463f433dcb970c42c1a) ) /* labeled TAPPER C.P.U. PRO 2 3C 1/12/84 */
	ROM_LOAD( "tapper_c.p.u._pg_3_4c_1-12-84.4c",   0x0c000, 0x2000, CRC(e8dcdaa4) SHA1(45bf1571a2418c7dc00ccc7061a3e04e65cb6bff) ) /* labeled TAPPER C.P.U. PRO 3 4C 1/12/84 */

	ROM_REGION( 0x10000, "ssio:cpu", 0 )
	ROM_LOAD( "tapper_sound_snd_0_a7_12-7-83.a7",   0x0000, 0x1000, CRC(0e8bb9d5) SHA1(9e281c340b7702523c86d56317efad9e3688e585) ) /* labeled TAPPER SOUND SND 0 A7 12/7/83 */
	ROM_LOAD( "tapper_sound_snd_1_a8_12-7-83.a8",   0x1000, 0x1000, CRC(0cf0e29b) SHA1(14334b9d2bfece3fe5bda0cbd53158ead8d27e53) ) /* labeled TAPPER SOUND SND 1 A8 12/7/83 */
	ROM_LOAD( "tapper_sound_snd_2_a9_12-7-83.a9",   0x2000, 0x1000, CRC(31eb6dc6) SHA1(b38bba5f12516d899e023f99147868e3402fbd7b) ) /* labeled TAPPER SOUND SND 2 A9 12/7/83 */
	ROM_LOAD( "tapper_sound_snd_3_a10_12-7-83.a10", 0x3000, 0x1000, CRC(01a9be6a) SHA1(0011407c1e886071282808c0a561789b1245a789) ) /* labeled TAPPER SOUND SND 3 A10 12/7/83 */

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "tapper_c.p.u._bg_1_6f_12-7-83.6f",   0x00000, 0x4000, CRC(2a30238c) SHA1(eb30b9bb654324340f0fc5b44776ac2440c1e869) ) /* labeled TAPPER C.P.U. BG 1 6F 12/7/83 */
	ROM_LOAD( "tapper_c.p.u._bg_0_5f_12-7-83.5f",   0x04000, 0x4000, CRC(394ab576) SHA1(23e29ec942e1e7516ae8068837af2d1c79592378) ) /* labeled TAPPER C.P.U. BG 0 5F 12/7/83 */

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "tapper_video_fg_1_a7_12-7-83.a7",   0x00000, 0x4000, CRC(32509011) SHA1(a38667573d235efe2dc515e52af05825fe4e0f30) ) /* labeled TAPPER VIDEO FG1 A7 12/7/83 */
	ROM_LOAD( "tapper_video_fg_0_a8_12-7-83.a8",   0x04000, 0x4000, CRC(8412c808) SHA1(2077f79177fda26f9c674b2ab525ec3833802059) ) /* labeled TAPPER VIDEO FG0 A8 12/7/83 */
	ROM_LOAD( "tapper_video_fg_3_a5_12-7-83.a5",   0x08000, 0x4000, CRC(818fffd4) SHA1(930142dd73fb30c4d3ec09a1e37517c6c6774024) ) /* labeled TAPPER VIDEO FG3 A5 12/7/83 */
	ROM_LOAD( "tapper_video_fg_2_a6_12-7-83.a6",   0x0c000, 0x4000, CRC(67e37690) SHA1(d553b8517c1d03a2be0b065f4da2fa99d9e6fb30) ) /* labeled TAPPER VIDEO FG2 A6 12/7/83 */
	ROM_LOAD( "tapper_video_fg_5_a3_12-7-83.a3",   0x10000, 0x4000, CRC(800f7c8a) SHA1(8aead89e1adaee5f0b679661c4bfba36e0d639e8) ) /* labeled TAPPER VIDEO FG5 A3 12/7/83 */
	ROM_LOAD( "tapper_video_fg_4_a4_12-7-83.a4",   0x14000, 0x4000, CRC(32674ee6) SHA1(402c166d50b4a693959b3f0706a7931a5daef6ce) ) /* labeled TAPPER VIDEO FG4 A4 12/7/83 */
	ROM_LOAD( "tapper_video_fg_7_a1_12-7-83.a1",   0x18000, 0x4000, CRC(070b4c81) SHA1(95879a455ecfe2e3de7fe2a75078f9e6934960f5) ) /* labeled TAPPER VIDEO FG7 A1 12/7/83 */
	ROM_LOAD( "tapper_video_fg_6_a2_12-7-83.a2",   0x1c000, 0x4000, CRC(a37aef36) SHA1(a24696f16d467d9eea4f25aef5f4c5ff55bf51ff) ) /* labeled TAPPER VIDEO FG6 A2 12/7/83 */
ROM_END

ROM_START( tapperb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tapper_c.p.u._pg_0_1c_12-9-83.1c",   0x00000, 0x4000, CRC(496a8e04) SHA1(a1e85d20aafb362d73fdfb7761d14c83e4abed19) ) /* labeled TAPPER C.P.U. PG 0 1C 12/9/83 */
	ROM_LOAD( "tapper_c.p.u._pg_1_2c_12-9-83.2c",   0x04000, 0x4000, CRC(e79c4b0c) SHA1(6d2e7fad732efbe5063e311e92337d9d24375f7f) ) /* labeled TAPPER C.P.U. PG 1 2C 12/9/83 */
	ROM_LOAD( "tapper_c.p.u._pg_2_3c_12-9-83.3c",   0x08000, 0x4000, CRC(3034ccf0) SHA1(523cfc8f5a52f41ef8de34ed602a5c684d0ff0a4) ) /* labeled TAPPER C.P.U. PG 2 3C 12/9/83 */
	ROM_LOAD( "tapper_c.p.u._pg_3_4c_12-9-83.4c",   0x0c000, 0x2000, CRC(2dc99e05) SHA1(e9bc159b55939b923e92976ad90058fe3ed712d4) ) /* labeled TAPPER C.P.U. PG 3 4C 12/9/83 */

	ROM_REGION( 0x10000, "ssio:cpu", 0 )
	ROM_LOAD( "tapper_sound_snd_0_a7_12-7-83.a7",   0x0000, 0x1000, CRC(0e8bb9d5) SHA1(9e281c340b7702523c86d56317efad9e3688e585) ) /* labeled TAPPER SOUND SND 0 A7 12/7/83 */
	ROM_LOAD( "tapper_sound_snd_1_a8_12-7-83.a8",   0x1000, 0x1000, CRC(0cf0e29b) SHA1(14334b9d2bfece3fe5bda0cbd53158ead8d27e53) ) /* labeled TAPPER SOUND SND 1 A8 12/7/83 */
	ROM_LOAD( "tapper_sound_snd_2_a9_12-7-83.a9",   0x2000, 0x1000, CRC(31eb6dc6) SHA1(b38bba5f12516d899e023f99147868e3402fbd7b) ) /* labeled TAPPER SOUND SND 2 A9 12/7/83 */
	ROM_LOAD( "tapper_sound_snd_3_a10_12-7-83.a10", 0x3000, 0x1000, CRC(01a9be6a) SHA1(0011407c1e886071282808c0a561789b1245a789) ) /* labeled TAPPER SOUND SND 3 A10 12/7/83 */

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "tapper_c.p.u._bg_1_6f_12-7-83.6f",   0x00000, 0x4000, CRC(2a30238c) SHA1(eb30b9bb654324340f0fc5b44776ac2440c1e869) ) /* labeled TAPPER C.P.U. BG 1 6F 12/7/83 */
	ROM_LOAD( "tapper_c.p.u._bg_0_5f_12-7-83.5f",   0x04000, 0x4000, CRC(394ab576) SHA1(23e29ec942e1e7516ae8068837af2d1c79592378) ) /* labeled TAPPER C.P.U. BG 0 5F 12/7/83 */

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "tapper_video_fg_1_a7_12-7-83.a7",   0x00000, 0x4000, CRC(32509011) SHA1(a38667573d235efe2dc515e52af05825fe4e0f30) ) /* labeled TAPPER VIDEO FG1 A7 12/7/83 */
	ROM_LOAD( "tapper_video_fg_0_a8_12-7-83.a8",   0x04000, 0x4000, CRC(8412c808) SHA1(2077f79177fda26f9c674b2ab525ec3833802059) ) /* labeled TAPPER VIDEO FG0 A8 12/7/83 */
	ROM_LOAD( "tapper_video_fg_3_a5_12-7-83.a5",   0x08000, 0x4000, CRC(818fffd4) SHA1(930142dd73fb30c4d3ec09a1e37517c6c6774024) ) /* labeled TAPPER VIDEO FG3 A5 12/7/83 */
	ROM_LOAD( "tapper_video_fg_2_a6_12-7-83.a6",   0x0c000, 0x4000, CRC(67e37690) SHA1(d553b8517c1d03a2be0b065f4da2fa99d9e6fb30) ) /* labeled TAPPER VIDEO FG2 A6 12/7/83 */
	ROM_LOAD( "tapper_video_fg_5_a3_12-7-83.a3",   0x10000, 0x4000, CRC(800f7c8a) SHA1(8aead89e1adaee5f0b679661c4bfba36e0d639e8) ) /* labeled TAPPER VIDEO FG5 A3 12/7/83 */
	ROM_LOAD( "tapper_video_fg_4_a4_12-7-83.a4",   0x14000, 0x4000, CRC(32674ee6) SHA1(402c166d50b4a693959b3f0706a7931a5daef6ce) ) /* labeled TAPPER VIDEO FG4 A4 12/7/83 */
	ROM_LOAD( "tapper_video_fg_7_a1_12-7-83.a1",   0x18000, 0x4000, CRC(070b4c81) SHA1(95879a455ecfe2e3de7fe2a75078f9e6934960f5) ) /* labeled TAPPER VIDEO FG7 A1 12/7/83 */
	ROM_LOAD( "tapper_video_fg_6_a2_12-7-83.a2",   0x1c000, 0x4000, CRC(a37aef36) SHA1(a24696f16d467d9eea4f25aef5f4c5ff55bf51ff) ) /* labeled TAPPER VIDEO FG6 A2 12/7/83 */
ROM_END

ROM_START( sutapper ) /* Distributed by Sega, Sega game ID# 834-5384 TAPPER */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-5791", 0x0000, 0x4000, CRC(87119cc4) SHA1(155dc1df977a474f3f1bd238d851c2ff8fe1faba) )
	ROM_LOAD( "epr-5792", 0x4000, 0x4000, CRC(4c23ad89) SHA1(0eebe98be6c21d701c7b7fc6547b5c94f934f5ab) )
	ROM_LOAD( "epr-5793", 0x8000, 0x4000, CRC(fecbf683) SHA1(de365f4e567d93a9ed9672fabbc739a3a0d47d59) )
	ROM_LOAD( "epr-5794", 0xc000, 0x2000, CRC(5bdc1916) SHA1(ee038443ae55598568bd1a53c0a671a2828d3949) )

	ROM_REGION( 0x10000, "ssio:cpu", 0 )
	ROM_LOAD( "epr-5788.h11.ic8", 0x00000, 0x1000, CRC(5c1d0982) SHA1(c2c94ab26ebce30ce4efc239e555c6368794d265) )
	ROM_LOAD( "epr-5787.h10.ic6", 0x01000, 0x1000, CRC(09e74ed8) SHA1(f5c8585d443bca67d4065314a06431d1f104c553) )
	ROM_LOAD( "epr-5786.h9.ic5",  0x02000, 0x1000, CRC(c3e98284) SHA1(2a4dc0deca48f4d2ac9fe673ecb9548415c996a9) )
	ROM_LOAD( "epr-5785.h7.ic4",  0x03000, 0x1000, CRC(ced2fd47) SHA1(a41323149c50adcae7675efcef69fd7d8365e527) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "epr-5790", 0x00000, 0x4000, CRC(ac1558c1) SHA1(f976889b529937948043460679f84b962b4c12bc) )
	ROM_LOAD( "epr-5789", 0x04000, 0x4000, CRC(fa66cab5) SHA1(96b89dc08f2feeb9950fbbba43d0ee57a9e31804) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "epr-5795", 0x00000, 0x4000, CRC(5d987c92) SHA1(3c26b0b0d903fb6782c6a1d72e32cd8c57ecad1f) )
	ROM_LOAD( "epr-5796", 0x04000, 0x4000, CRC(de5700b4) SHA1(c613c2225eeff5cc65dc6ec301e616e54755b1c2) )
	ROM_LOAD( "epr-5797", 0x08000, 0x4000, CRC(f10a1d05) SHA1(ca54d1fa6704d38e65a4d2a94449ed8dd56cc94b) )
	ROM_LOAD( "epr-5798", 0x0c000, 0x4000, CRC(614990cd) SHA1(1a6eac2a8fa99d86889d5042c6b64f828b3c5d65) )
	ROM_LOAD( "epr-5799", 0x10000, 0x4000, CRC(02c69432) SHA1(7f4260f4a4e8b33842355e9d8e859ffb9278c3c2) )
	ROM_LOAD( "epr-5800", 0x14000, 0x4000, CRC(ebf1f948) SHA1(251cf018da8db11c3844123255082146b22507e5) )
	ROM_LOAD( "epr-5801", 0x18000, 0x4000, CRC(d70defa7) SHA1(e8ceabe94080eb28aa393b97ec54729cf8aba001) )
	ROM_LOAD( "epr-5802", 0x1c000, 0x4000, CRC(d4f114b9) SHA1(58ae647b4fd0f48af4158b85e29c813605d930d3) )
ROM_END

ROM_START( rbtapper )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rbtpg0.bin",   0x00000, 0x4000, CRC(20b9adf4) SHA1(31b583c98e7c2cecab42223df8b5400004c9f3d3) )
	ROM_LOAD( "rbtpg1.bin",   0x04000, 0x4000, CRC(87e616c2) SHA1(7de10632b2538524fee418175e2cc41be9c8c0e8) )
	ROM_LOAD( "rbtpg2.bin",   0x08000, 0x4000, CRC(0b332c97) SHA1(b9878c8a61a98e787e547bb6ab81c809875891f3) )
	ROM_LOAD( "rbtpg3.bin",   0x0c000, 0x2000, CRC(698c06f2) SHA1(ddb21e39ede2222cb2286ec9dba06341fe1c9db7) )

	ROM_REGION( 0x10000, "ssio:cpu", 0 )
	ROM_LOAD( "5788",         0x00000, 0x1000, CRC(5c1d0982) SHA1(c2c94ab26ebce30ce4efc239e555c6368794d265) )
	ROM_LOAD( "5787",         0x01000, 0x1000, CRC(09e74ed8) SHA1(f5c8585d443bca67d4065314a06431d1f104c553) )
	ROM_LOAD( "5786",         0x02000, 0x1000, CRC(c3e98284) SHA1(2a4dc0deca48f4d2ac9fe673ecb9548415c996a9) )
	ROM_LOAD( "5785",         0x03000, 0x1000, CRC(ced2fd47) SHA1(a41323149c50adcae7675efcef69fd7d8365e527) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "rbtbg1.bin",   0x00000, 0x4000, CRC(44dfa483) SHA1(9e96a3ff0b66a4b1d07fe86059f7dfab0d53231c) )
	ROM_LOAD( "rbtbg0.bin",   0x04000, 0x4000, CRC(510b13de) SHA1(a4b24fffbbe299e0c2058abe372f00954e5edf98) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "rbtfg1.bin",   0x00000, 0x4000, CRC(1c0b8791) SHA1(532239605b74b137cd0f6035a6bc7ac43f559d82) )
	ROM_LOAD( "rbtfg0.bin",   0x04000, 0x4000, CRC(e99f6018) SHA1(3a8e3e927b0fd2d49222e417e4c1f28b5a45c6ca) )
	ROM_LOAD( "rbtfg3.bin",   0x08000, 0x4000, CRC(3e725e77) SHA1(3811f499aa076fb81af42f7cb522879b0af50e37) )
	ROM_LOAD( "rbtfg2.bin",   0x0c000, 0x4000, CRC(4ee8b624) SHA1(d02a306f8b32b3a097fc51e75e106a40fe942d35) )
	ROM_LOAD( "rbtfg5.bin",   0x10000, 0x4000, CRC(9eeca46e) SHA1(523efaba9a3cbe871868ee09caea062382f31fad) )
	ROM_LOAD( "rbtfg4.bin",   0x14000, 0x4000, CRC(8c79e7d7) SHA1(e2a427324fc1d4422e50a1ab79822aff0290e109) )
	ROM_LOAD( "rbtfg7.bin",   0x18000, 0x4000, CRC(8dbf0c36) SHA1(dab52db0bca13b9f3470060bc3babe4a6c2cdcc8) )
	ROM_LOAD( "rbtfg6.bin",   0x1c000, 0x4000, CRC(441201a0) SHA1(400d390729601610b152e21da55399399830221f) )
ROM_END


ROM_START( timber )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "timpg0.bin",   0x00000, 0x4000, CRC(377032ab) SHA1(7b6c6e9c5088651720990c49296ddccb5416a729) )
	ROM_LOAD( "timpg1.bin",   0x04000, 0x4000, CRC(fd772836) SHA1(ce5266f7b1ab7dc8c1af864bc8afe33d724649fc) )
	ROM_LOAD( "timpg2.bin",   0x08000, 0x4000, CRC(632989f9) SHA1(9e9dc343746299bb0dc7ada206211366c5a05075) )
	ROM_LOAD( "timpg3.bin",   0x0c000, 0x2000, CRC(dae8a0dc) SHA1(f065fa3184efa6524d4f950616f3fbae4ea17513) )

	ROM_REGION( 0x10000, "ssio:cpu", 0 )
	ROM_LOAD( "tima7.bin",    0x00000, 0x1000, CRC(c615dc3e) SHA1(664d5e3ac3936fd04a855ee0c88f1c1b4d1dea5b) )
	ROM_LOAD( "tima8.bin",    0x01000, 0x1000, CRC(83841c87) SHA1(bd5a2e567e015e10e45651e15b42ffb3b69d2305) )
	ROM_LOAD( "tima9.bin",    0x02000, 0x1000, CRC(22bcdcd3) SHA1(69cedc8cec52ca310f828dfe73d7de04729b06d3) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "timbg1.bin",   0x00000, 0x4000, CRC(b1cb2651) SHA1(799efcc35b08a3432ee2c13de3f6c65477a6de07) )
	ROM_LOAD( "timbg0.bin",   0x04000, 0x4000, CRC(2ae352c4) SHA1(8039f09cdba6ce43005f98dcff91958ba16363bb) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "timfg1.bin",   0x00000, 0x4000, CRC(81de4a73) SHA1(38e06b67004aa44dc58d205793ae83d3707472a6) )
	ROM_LOAD( "timfg0.bin",   0x04000, 0x4000, CRC(7f3a4f59) SHA1(2723cb93c1b7b229b370d069651cf83ecb1cff3a) )
	ROM_LOAD( "timfg3.bin",   0x08000, 0x4000, CRC(37c03272) SHA1(2f5793b6a148af43c5b6efe718714942ce7d271b) )
	ROM_LOAD( "timfg2.bin",   0x0c000, 0x4000, CRC(e2c2885c) SHA1(618dab8cf2ee189343210d0e20dd246447c4c542) )
	ROM_LOAD( "timfg5.bin",   0x10000, 0x4000, CRC(eb636216) SHA1(c347a64933f547cf1925e5fa187deda6e0bf713c) )
	ROM_LOAD( "timfg4.bin",   0x14000, 0x4000, CRC(b7105eb7) SHA1(913687f0e5dd105a3dfdc0afd1b5610b27124451) )
	ROM_LOAD( "timfg7.bin",   0x18000, 0x4000, CRC(d9c27475) SHA1(24a7e6200e4f3514d4d3f25b2ffe3338c0c2a495) )
	ROM_LOAD( "timfg6.bin",   0x1c000, 0x4000, CRC(244778e8) SHA1(494bc1e627997cd4f1d5267c5fafd0779ccf9255) )
ROM_END

/*
Disc of TRON 10/4/83

referred to as version 2, known differences:
  shows "DEREZZ A RING    2000" in score table
  Sound Test menu shows sounds 1 through 16 (though the other sounds are still in the game)
  Player Input screen shows "UPRIGHT   ACTIVATE ALL DEVICES   TILT TO EXIT"
*/
ROM_START( dotron )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "disc_tron_uprt_pg0_10-4-83.1c",  0x00000, 0x4000, CRC(40d00195) SHA1(e06a8097f02b9f445df0dd5c0ec13f9a0a1dcd8a) ) // labeled: DISC/TRON   UPRT PG0   10/4/83   BALLY/MIDWY
	ROM_LOAD( "disc_tron_uprt_pg1_10-4-83.2c",  0x04000, 0x4000, CRC(5a7d1300) SHA1(8a1f088de9289cd902e72b55d3e72c3f07246778) ) // labeled: DISC/TRON   UPRT PG1   10/4/83   BALLY/MIDWY
	ROM_LOAD( "disc_tron_uprt_pg2_10-4-83.3c",  0x08000, 0x4000, CRC(cb89c9be) SHA1(c773a68891fbf94808a2ee0036928c0c48d6673d) ) // labeled: DISC/TRON   UPRT PG2   10/4/83   BALLY/MIDWY
	ROM_LOAD( "disc_tron_uprt_pg3_10-4-83.4c",  0x0c000, 0x2000, CRC(5098faf4) SHA1(9f861f99cb170513b68aee48bbfd60ee439d7fa9) ) // labeled: DISC/TRON   UPRT PG3   10/4/83   BALLY/MIDWY

	ROM_REGION( 0x10000, "ssio:cpu", 0 )
	ROM_LOAD( "disc_tron_uprt_snd0_10-4-83.a7",  0x00000, 0x1000, CRC(7fb54293) SHA1(6d538a3e48f98e269623850f1f6774848a89fd59) ) // labeled: DISC/TRON   UPRT SND0   10/4/83   BALLY/MIDWY
	ROM_LOAD( "disc_tron_uprt_snd1_10-4-83.a8",  0x01000, 0x1000, CRC(edef7326) SHA1(5c9a64604252eea0628bf9d6221e8add82f66abe) ) // labeled: DISC/TRON   UPRT SND1   10/4/83   BALLY/MIDWY
	ROM_LOAD( "disc_tron_uprt_snd2_9-22-83.a9",  0x02000, 0x1000, CRC(e8ef6519) SHA1(261b0463a73b403bc46df3e04f3d12173787d6e7) ) // labeled: DISC/TRON   UPRT SND2   9/22/83   BALLY/MIDWY
	ROM_LOAD( "disc_tron_uprt_snd3_9-22-83.a10", 0x03000, 0x1000, CRC(6b5aeb02) SHA1(039d8d664f067bc0d085ad7730ef63dbd6dc387e) ) // labeled: DISC/TRON   UPRT SND3   9/22/83   BALLY/MIDWY

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "loc-bg2.6f",   0x00000, 0x2000, CRC(40167124) SHA1(782c8192dd58a3f23ff2338452dd03206d79030a) ) // need to verify actual labels
	ROM_LOAD( "loc-bg1.5f",   0x02000, 0x2000, CRC(bb2d7a5d) SHA1(8044be9ffca9520fd77e0da492147e553f9f7da3) )

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "loc-g.cp4",    0x00000, 0x2000, CRC(57a2b1ff) SHA1(b97539ffd2f5fc8b86fc2f8f233cc26ba16f82ee) ) // these ROMs dated 8/15/83 - need to verify actual labels
	ROM_LOAD( "loc-h.cp3",    0x02000, 0x2000, CRC(3bb4d475) SHA1(3795ba1640790041da51ebeac8517cc7d32e243e) )
	ROM_LOAD( "loc-e.cp6",    0x04000, 0x2000, CRC(ce957f1a) SHA1(24177a8dd6dcb377cf8aee7c7b47b26f29e77e20) )
	ROM_LOAD( "loc-f.cp5",    0x06000, 0x2000, CRC(d26053ce) SHA1(b7fb3d1df9b80c056cf131574565addb529645e1) )
	ROM_LOAD( "loc-c.cp8",    0x08000, 0x2000, CRC(ef45d146) SHA1(6cd83909b4376abce287e435a10e5bc25e18b265) )
	ROM_LOAD( "loc-d.cp7",    0x0a000, 0x2000, CRC(5e8a3ef3) SHA1(74983c922eae1326ecd0ff14000851e0b424cc61) )
	ROM_LOAD( "loc-a.cp0",    0x0c000, 0x2000, CRC(b35f5374) SHA1(3f330ffde52ac57c02dfdf8e105aefcc10f87a0b) )
	ROM_LOAD( "loc-b.cp9",    0x0e000, 0x2000, CRC(565a5c48) SHA1(9dfafd58bd552bfda4e1799a175735ecc1369ba3) )
ROM_END

/*
Disc of TRON 9/22/83

referred to as version 1, known differences:
  Missing "DEREZZ A RING    2000" in score table
  Sound Test menu shows sounds 1 through 19
  Player Input screen shows "UPRIGHT   ACTIVATE ALL PLAYER INPUT   SWITCHES AND DEVICES   HIT TILT TO EXIT"
*/
ROM_START( dotrona )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "disc_tron_uprt_pg0_9-22-83.1c",   0x00000, 0x4000, CRC(ba0da15f) SHA1(c0dfac2e5d6549620525b9e3d64b7c5494164dbd) ) // labeled: DISC/TRON   UPRT PG0   9/22/83   BALLY/MIDWY
	ROM_LOAD( "disc_tron_uprt_pg1_9-22-83.2c",   0x04000, 0x4000, CRC(dc300191) SHA1(417e964f38bfbdd84cae79939c23a7de41cd7bae) ) // labeled: DISC/TRON   UPRT PG1   9/22/83   BALLY/MIDWY
	ROM_LOAD( "disc_tron_uprt_pg2_9-22-83.3c",   0x08000, 0x4000, CRC(ab0b3800) SHA1(457a18bd98a3c4a9f893a3704dbc7d0fde4ef8ba) ) // labeled: DISC/TRON   UPRT PG2   9/22/83   BALLY/MIDWY
	ROM_LOAD( "disc_tron_uprt_pg3_9-22-83.4c",   0x0c000, 0x2000, CRC(f98c9f8e) SHA1(a215f0fd6cd9e8cacbe06cb7bfe4e2cced150c86) ) // labeled: DISC/TRON   UPRT PG3   9/22/83   BALLY/MIDWY

	ROM_REGION( 0x10000, "ssio:cpu", 0 )
	ROM_LOAD( "disc_tron_uprt_snd0_9-22-83.a7",  0x00000, 0x1000, CRC(6d39bf19) SHA1(3d27466fcb6d41133f16119cddb815833c8b4eda) ) // labeled: DISC/TRON   UPRT SND0   9/22/83   BALLY/MIDWY
	ROM_LOAD( "disc_tron_uprt_snd1_9-22-83.a8",  0x01000, 0x1000, CRC(ac872e1d) SHA1(c2833b20e124c505be3d5be2c885b9cf9927ca4c) ) // labeled: DISC/TRON   UPRT SND1   9/22/83   BALLY/MIDWY
	ROM_LOAD( "disc_tron_uprt_snd2_9-22-83.a9",  0x02000, 0x1000, CRC(e8ef6519) SHA1(261b0463a73b403bc46df3e04f3d12173787d6e7) ) // labeled: DISC/TRON   UPRT SND2   9/22/83   BALLY/MIDWY
	ROM_LOAD( "disc_tron_uprt_snd3_9-22-83.a10", 0x03000, 0x1000, CRC(6b5aeb02) SHA1(039d8d664f067bc0d085ad7730ef63dbd6dc387e) ) // labeled: DISC/TRON   UPRT SND3   9/22/83   BALLY/MIDWY

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "loc-bg2.6f",   0x00000, 0x2000, CRC(40167124) SHA1(782c8192dd58a3f23ff2338452dd03206d79030a) ) // need to verify actual labels
	ROM_LOAD( "loc-bg1.5f",   0x02000, 0x2000, CRC(bb2d7a5d) SHA1(8044be9ffca9520fd77e0da492147e553f9f7da3) )

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "loc-g.cp4",    0x00000, 0x2000, CRC(57a2b1ff) SHA1(b97539ffd2f5fc8b86fc2f8f233cc26ba16f82ee) ) // these ROMs dated 8/15/83 - need to verify actual labels
	ROM_LOAD( "loc-h.cp3",    0x02000, 0x2000, CRC(3bb4d475) SHA1(3795ba1640790041da51ebeac8517cc7d32e243e) )
	ROM_LOAD( "loc-e.cp6",    0x04000, 0x2000, CRC(ce957f1a) SHA1(24177a8dd6dcb377cf8aee7c7b47b26f29e77e20) )
	ROM_LOAD( "loc-f.cp5",    0x06000, 0x2000, CRC(d26053ce) SHA1(b7fb3d1df9b80c056cf131574565addb529645e1) )
	ROM_LOAD( "loc-c.cp8",    0x08000, 0x2000, CRC(ef45d146) SHA1(6cd83909b4376abce287e435a10e5bc25e18b265) )
	ROM_LOAD( "loc-d.cp7",    0x0a000, 0x2000, CRC(5e8a3ef3) SHA1(74983c922eae1326ecd0ff14000851e0b424cc61) )
	ROM_LOAD( "loc-a.cp0",    0x0c000, 0x2000, CRC(b35f5374) SHA1(3f330ffde52ac57c02dfdf8e105aefcc10f87a0b) )
	ROM_LOAD( "loc-b.cp9",    0x0e000, 0x2000, CRC(565a5c48) SHA1(9dfafd58bd552bfda4e1799a175735ecc1369ba3) )
ROM_END

ROM_START( dotrone )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "loc-cpu1",     0x00000, 0x4000, CRC(eee31b8c) SHA1(c05ad1d10588a6c1050c608f1a473685ebe4daad) ) // need to verify actual labels
	ROM_LOAD( "loc-cpu2",     0x04000, 0x4000, CRC(75ba6ad3) SHA1(d02c3d731073fb6083bd8f771f76338939384a07) )
	ROM_LOAD( "loc-cpu3",     0x08000, 0x4000, CRC(94bb1a0e) SHA1(af4769fac39e67eff840675bf93cc4304f2875fd) )
	ROM_LOAD( "loc-cpu4",     0x0c000, 0x2000, CRC(c137383c) SHA1(ccf7cf9c7c0528aa819cfca34c1c0e89ab2d586a) )

	ROM_REGION( 0x10000, "ssio:cpu", 0 ) // These 4 ROMs stamped "AUG 19"
	ROM_LOAD( "ssi_o_loc-a_disc_of_tron_aug_19", 0x00000, 0x1000, CRC(2de6a8a8) SHA1(6bba00daed8836297f3189db4e4fe8e158adc465) ) // labeled: SSI/O   LOC-A   DISC OF TRON   (c)1983 BALLY/MIDAY
	ROM_LOAD( "ssi_o_loc-b_disc_of_tron_aug_19", 0x01000, 0x1000, CRC(4097663e) SHA1(afb5224529550cec378415a5cd81b47f6c6c101b) ) // labeled: SSI/O   LOC-B   DISC OF TRON   (c)1983 BALLY/MIDAY
	ROM_LOAD( "ssi_o_loc-c_disc_of_tron_aug_19", 0x02000, 0x1000, CRC(f576b9e7) SHA1(4ff39c46c390aa93d900f5f7a0b35fa71f066863) ) // labeled: SSI/O   LOC-C   DISC OF TRON   (c)1983 BALLY/MIDAY
	ROM_LOAD( "ssi_o_loc-d_disc_of_tron_aug_19", 0x03000, 0x1000, CRC(74b0059e) SHA1(1fe393721446538036fb6110fdc3920959ebd596) ) // labeled: SSI/O   LOC-D   DISC OF TRON   (c)1983 BALLY/MIDAY

	ROM_REGION( 0x10000, "snt:cpu", 0 )
	ROM_LOAD( "pre.u3",       0x09000, 0x1000, CRC(c3d0f762) SHA1(a1857641c35b5bcb33f29fe79a1a581c4cbf129b) ) // need to verify actual labels
	ROM_LOAD( "pre.u4",       0x0a000, 0x1000, CRC(7ca79b43) SHA1(c995e1e67d70706a090eb777e9fec0f1ba03f82d) )
	ROM_LOAD( "pre.u5",       0x0b000, 0x1000, CRC(24e9618e) SHA1(eb245ff381a76b314a0ed3519e140444afae341c) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "loc-bg2.6f",   0x00000, 0x2000, CRC(40167124) SHA1(782c8192dd58a3f23ff2338452dd03206d79030a) ) // need to verify actual labels
	ROM_LOAD( "loc-bg1.5f",   0x02000, 0x2000, CRC(bb2d7a5d) SHA1(8044be9ffca9520fd77e0da492147e553f9f7da3) )

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "loc-g.cp4",    0x00000, 0x2000, CRC(57a2b1ff) SHA1(b97539ffd2f5fc8b86fc2f8f233cc26ba16f82ee) ) // need to verify actual labels
	ROM_LOAD( "loc-h.cp3",    0x02000, 0x2000, CRC(3bb4d475) SHA1(3795ba1640790041da51ebeac8517cc7d32e243e) )
	ROM_LOAD( "loc-e.cp6",    0x04000, 0x2000, CRC(ce957f1a) SHA1(24177a8dd6dcb377cf8aee7c7b47b26f29e77e20) )
	ROM_LOAD( "loc-f.cp5",    0x06000, 0x2000, CRC(d26053ce) SHA1(b7fb3d1df9b80c056cf131574565addb529645e1) )
	ROM_LOAD( "loc-c.cp8",    0x08000, 0x2000, CRC(ef45d146) SHA1(6cd83909b4376abce287e435a10e5bc25e18b265) )
	ROM_LOAD( "loc-d.cp7",    0x0a000, 0x2000, CRC(5e8a3ef3) SHA1(74983c922eae1326ecd0ff14000851e0b424cc61) )
	ROM_LOAD( "loc-a.cp0",    0x0c000, 0x2000, CRC(b35f5374) SHA1(3f330ffde52ac57c02dfdf8e105aefcc10f87a0b) )
	ROM_LOAD( "loc-b.cp9",    0x0e000, 0x2000, CRC(565a5c48) SHA1(9dfafd58bd552bfda4e1799a175735ecc1369ba3) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "edotlamp.u2",  0x0020, 0x0020, CRC(fb58b867) SHA1(45beb55f2c2e9197f091fc06e9a2f595e57e5c93) )    // lamp sequencer PROM
ROM_END

ROM_START( dotronep ) // BTANB: upon boot up, you get a "SOUND BOARD INTERFACE ERROR" message
	ROM_REGION( 0x10000, "maincpu", 0 ) // all 27128s
	ROM_LOAD( "pgm0_8-9.1c", 0x00000, 0x4000, CRC(a997e93d) SHA1(bf32c8b7e0ab2acabf177d686dbb3696d259e6af) )
	ROM_LOAD( "pgm1_8-9.2c", 0x04000, 0x4000, CRC(31e5c11e) SHA1(9443c22ed436dcafbea86fdb9ecbaa61659960d9) )
	ROM_LOAD( "pgm2_8-9.3c", 0x08000, 0x4000, CRC(5162fe8d) SHA1(82cf3c6af339bf81e22294ff91e99ca9e08b86f2) )
	ROM_LOAD( "pgm3_8-9.4c", 0x0c000, 0x4000, CRC(4e7085e1) SHA1(c26363ae9c2dc422f79200d4d7829c8ca3b2fa18) )

	ROM_REGION( 0x10000, "ssio:cpu", 0 ) // all 2532s
	ROM_LOAD( "0_8-9.a7",  0x00000, 0x1000, CRC(872160b3) SHA1(824fd62fc086399db5a52f0d14b4b0022c8a67a2) )
	ROM_LOAD( "1_8-9.a8",  0x01000, 0x1000, CRC(d9c3d4ca) SHA1(02128979a1445a6e757c98aee5d486ef4c6add5a) )
	ROM_LOAD( "2_8-9.a9",  0x02000, 0x1000, CRC(78cc8bad) SHA1(eef57ded6392d9d68756c2e0686456584144494a) )
	ROM_LOAD( "3_8-9.a10", 0x03000, 0x1000, CRC(b369ec5a) SHA1(5e9d410f060bfa417f3a399b3704e2251fb601e5) )

	ROM_REGION( 0x10000, "snt:cpu", 0 ) // all 2532s
	ROM_LOAD( "pre_aug_19.u3", 0x09000, 0x1000, CRC(c3d0f762) SHA1(a1857641c35b5bcb33f29fe79a1a581c4cbf129b) )
	ROM_LOAD( "pre.u4",        0x0a000, 0x1000, CRC(7ca79b43) SHA1(c995e1e67d70706a090eb777e9fec0f1ba03f82d) )
	ROM_LOAD( "pre.u5",        0x0b000, 0x1000, CRC(24e9618e) SHA1(eb245ff381a76b314a0ed3519e140444afae341c) )

	ROM_REGION( 0x04000, "gfx1", 0 ) // all 2764s
	ROM_LOAD( "bg0_8-9.6f", 0x00000, 0x2000, CRC(40167124) SHA1(782c8192dd58a3f23ff2338452dd03206d79030a) )
	ROM_LOAD( "bg1_8-9.5f", 0x02000, 0x2000, CRC(bb2d7a5d) SHA1(8044be9ffca9520fd77e0da492147e553f9f7da3) )

	ROM_REGION( 0x10000, "gfx2", 0 ) // all 2764s
	ROM_LOAD( "cp4_fg7-8-9-83.a7", 0x00000, 0x2000, CRC(1e689f9c) SHA1(1ea79ba810eedb03581aa84dd27c52a53f542636) )
	ROM_LOAD( "cp3_fg8-8-9-83.a8", 0x02000, 0x2000, CRC(172685ee) SHA1(52499af5540b401d3deed4667bb082c16d47e088) )
	ROM_LOAD( "cp6_fg5-8-9-83.a5", 0x04000, 0x2000, CRC(4326041b) SHA1(77f365686edae474927b2eced0398389ea0fd169) )
	ROM_LOAD( "cp5_fg6-8-9-83.a6", 0x06000, 0x2000, CRC(1df69f11) SHA1(2ca2d97b619751206b2411663159fc36577440e1) )
	ROM_LOAD( "cp8_fg3-8-9-83.a3", 0x08000, 0x2000, CRC(7587a8fd) SHA1(95da15f103596e7a4d9c3602539e5222846dccf6) )
	ROM_LOAD( "cp7_fg4-8-9-83.a4", 0x0a000, 0x2000, CRC(6e1af64f) SHA1(09c3c13baed7a18d02a254741461bc1b9d46002f) )
	ROM_LOAD( "cp10_fg1.a1",       0x0c000, 0x2000, CRC(759f36a7) SHA1(e295e009b7f00f5a7a8c664a4117f17020ceddf6) )
	ROM_LOAD( "cp9_fg2-8-9-83.a2", 0x0e000, 0x2000, CRC(80ba9553) SHA1(72cdf373280ba62d01af7269121f6270c4a303d1) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "1f26.u2", 0x0020, 0x0020, CRC(fb58b867) SHA1(45beb55f2c2e9197f091fc06e9a2f595e57e5c93) )    // lamp sequencer PROM (N82S123)
ROM_END


ROM_START( nflfoot )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nflcpupg.1c",  0x00000, 0x4000, CRC(d76a7a41) SHA1(591c1f530469ac2131f2c0fb4c3350546f44c358) )
	ROM_LOAD( "nflcpupg.2c",  0x04000, 0x4000, CRC(2aa76168) SHA1(608df883f5e960153a963404d5cc4b4ce4ec435d) )
	ROM_LOAD( "nflcpupg.3c",  0x08000, 0x4000, CRC(5ec01e09) SHA1(2bf60ab7d47f53583b677195976d6f6a9e90c55c) )

	ROM_REGION( 0x10000, "ssio:cpu", 0 )
	ROM_LOAD( "nflsnd.a7",    0x0000, 0x1000, CRC(1339be2e) SHA1(5c1743f4d20f94053eb306d3749057608df4a6a2) )
	ROM_LOAD( "nflsnd.a8",    0x1000, 0x1000, CRC(8630b560) SHA1(0c537f48184d3a7a9ee51c30d7c33dc39c46e823) )
	ROM_LOAD( "nflsnd.a9",    0x2000, 0x1000, CRC(1e0fe4c8) SHA1(718dfaced2d8d84dab4c32265bed422e07af0f9e) )

	ROM_REGION( 0x10000, "snt:cpu", 0 )
	ROM_LOAD( "nfl-sqtk-11-15-83.u2", 0x08000, 0x1000, CRC(aeddda31) SHA1(8ebe9d8606c4328b1b3f4633db30d7636acf210b) )
	ROM_LOAD( "nfl-sqtk-11-15-83.u3", 0x09000, 0x1000, CRC(36229d13) SHA1(d174098ce1e4bc89ded15a08db37933ab9532f2b) )
	ROM_LOAD( "nfl-sqtk-11-15-83.u4", 0x0a000, 0x1000, CRC(b202439b) SHA1(b09e94b0b176f80b12fb4cefa6efd5b2cccb6192) )
	ROM_LOAD( "nfl-sqtk-11-15-83.u5", 0x0b000, 0x1000, CRC(bbfe4d39) SHA1(161ed211701e576978d6ef8b9766eb7742a29eb3) )

	ROM_REGION( 0x10000, "ipu", 0 ) /* 64k for the IPU CPU */
	ROM_LOAD( "ipu-7-9.a2", 0x0000, 0x2000, CRC(0e083adb) SHA1(b799568ff851f7320869fb84821a90eb1156556f) )
	ROM_LOAD( "ipu-7-9.a4", 0x2000, 0x2000, CRC(5c9c4764) SHA1(ee9fe1d85dbfb1089bc8ed106a28fe5f3c36fb42) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "nflcpubg.6f",  0x00000, 0x2000, CRC(6d116cd9) SHA1(72acbb593e011b7732915c05fd2376eb5a9c5078) )
	ROM_LOAD( "nflcpubg.5f",  0x02000, 0x2000, CRC(5f1b0b67) SHA1(90b223ce65f814de26507d45a6db257ceaa932b1) )

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "nflvidfg.cp4", 0x00000, 0x2000, CRC(eb6b808d) SHA1(e81580959710ecfd2f56a661c61b681ac22887cf) )
	ROM_LOAD( "nflvidfg.cp3", 0x02000, 0x2000, CRC(be21580a) SHA1(f057635a0a4d7fd99d50e0496fe0af8c6ed27ed9) )
	ROM_LOAD( "nflvidfg.cp6", 0x04000, 0x2000, CRC(54a0bff8) SHA1(1a4de884a1a43b97d402d068d3281696c7b48904) )
	ROM_LOAD( "nflvidfg.cp5", 0x06000, 0x2000, CRC(6aeba0ab) SHA1(2d99a28bc8bd8289ac12824258b16118c808d6c7) )
	ROM_LOAD( "nflvidfg.cp8", 0x08000, 0x2000, CRC(112ee67b) SHA1(fc9ca6cd87f84f7f033729ca0a06978ec0bb7c32) )
	ROM_LOAD( "nflvidfg.cp7", 0x0a000, 0x2000, CRC(73f62392) SHA1(18f28be7264f8edff38f8a6aa067eeb1970f544c) )
	ROM_LOAD( "nflvidfg.c10", 0x0c000, 0x2000, CRC(1766dcc7) SHA1(df499e3c66ae702d2d56e6cd095a754665569fcd) )
	ROM_LOAD( "nflvidfg.cp9", 0x0e000, 0x2000, CRC(46558146) SHA1(4bedfae8cf0fcb9d837706ee13fbe3944ab47216) )

	DISK_REGION( "ced_videodisc" )
	DISK_IMAGE_READONLY( "nflfoot", 0, NO_DUMP )
ROM_END


ROM_START( demoderb ) /* Dipswitch selectable 2 player Upright / 4 player Cocktail */
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Actually used "DRBY" for program roms, all others used full "DERBY" */
	ROM_LOAD( "demo_drby_pro_0", 0x00000, 0x4000, CRC(be7da2f3) SHA1(6c43a7e4334d80829a333841ccd9fdc5824c915d) ) /* Dated 6/12/85 */
	ROM_LOAD( "demo_drby_pro_1", 0x04000, 0x4000, CRC(c6f6604c) SHA1(69ce86e762ccfd9d15accf5ddcb2406eb1b11132) ) /* Dated 6/12/85 */
	ROM_LOAD( "demo_drby_pro_2", 0x08000, 0x4000, CRC(fa93b9d9) SHA1(61891b7850da93c16d11cd6d20a72e1b371f47d3) ) /* Dated 6/12/85 */
	ROM_LOAD( "demo_drby_pro_3", 0x0c000, 0x4000, CRC(4e964883) SHA1(a1cb4e07c7417abc8a08bdae31de2bda8063dedc) ) /* Dated 6/12/85 */

	ROM_REGION( 0x10000, "ssio:cpu", ROMREGION_ERASE00 )    /* 64k for the audio CPU, not populated */

	ROM_REGION( 0x10000, "tcs:cpu", 0 ) /* 64k for the Turbo Cheap Squeak */
	ROM_LOAD( "tcs_u5.bin", 0x0c000, 0x2000, CRC(eca33b2c) SHA1(938b021ea3b0f23aed7a98a930a58af371a02303) )
	ROM_LOAD( "tcs_u4.bin", 0x0e000, 0x2000, CRC(3490289a) SHA1(a9d56ea60bb901267da41ab408f8e1ed3742b0ac) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "demo_derby_bg_06f.6f", 0x00000, 0x2000, CRC(cf80be19) SHA1(a2ab09ee2dc76fab472fec7520ed972ccc10e826) ) /* Dated 2/7/85 */
	ROM_LOAD( "demo_derby_bg_15f.5f", 0x02000, 0x2000, CRC(4e173e52) SHA1(ac5ae8007a63f9c074444783c1058109327dd118) ) /* Dated 2/7/85 */

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "demo_derby_fg0_a4.a4",   0x00000, 0x4000, CRC(e57a4de6) SHA1(d1b2396a85b984e171d751ef8e1cf970ac4ff9fb) ) /* Dated 3/11/85 */
	ROM_LOAD( "demo_derby_fg4_a3.a3",   0x04000, 0x4000, CRC(55aa667f) SHA1(d611dbf9e8ef383d02514b0edb9ea36670193bf0) ) /* Dated 3/11/85 */
	ROM_LOAD( "demo_derby_fg1_a6.a6",   0x08000, 0x4000, CRC(70259651) SHA1(55967aaf2a7617c8f5a199d1e07128d79ce16970) ) /* Dated 3/11/85 */
	ROM_LOAD( "demo_derby_fg5_a5.a5",   0x0c000, 0x4000, CRC(5fe99007) SHA1(9d640b4715333efdc6300dc353991d6934929399) ) /* Dated 3/11/85 */
	ROM_LOAD( "demo_derby_fg2_a8.a8",   0x10000, 0x4000, CRC(6cab7b95) SHA1(8faff7458ab5ff2dd096dd78b1449a4096cc6345) ) /* Dated 3/11/85 */
	ROM_LOAD( "demo_derby_fg6_a7.a7",   0x14000, 0x4000, CRC(abfb9a8b) SHA1(14ab416bc76db25ad97353c9072048c64ec95344) ) /* Dated 3/11/85  - Mislabeled as DEMO DERBY FG1 A7 */
	ROM_LOAD( "demo_derby_fg3_a10.a10", 0x18000, 0x4000, CRC(801d9b86) SHA1(5a8c72d1060eea1a3ad67b98aa6eff13f6837af6) ) /* Dated 3/11/85 */
	ROM_LOAD( "demo_derby_fg7_a9.a9",   0x1c000, 0x4000, CRC(0ec3f60a) SHA1(4176b246b0ea7bce9498c20e12678f16f7173529) ) /* Dated 3/11/85 */
ROM_END


ROM_START( demoderbc ) /* Only supports 4 player cocktail mode! */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dd_pro0", 0x00000, 0x4000, CRC(8781b367) SHA1(52fe4f4e3fa0370284845d88bc7b30a95c962b22) )
	ROM_LOAD( "dd_pro1", 0x04000, 0x4000, CRC(4c713bfe) SHA1(493b6ba01e86e7586ad123c53cf7f0a0c191d670) )
	ROM_LOAD( "dd_pro2", 0x08000, 0x4000, CRC(c2cbd2a4) SHA1(fa642b2f61ff5529ab688a43c1dc14357a4eba6f) )

	ROM_REGION( 0x10000, "ssio:cpu", ROMREGION_ERASE00 )    /* 64k for the audio CPU, not populated */

	ROM_REGION( 0x10000, "tcs:cpu", 0 ) /* 64k for the Turbo Cheap Squeak */
	ROM_LOAD( "tcs_u5.bin", 0x0c000, 0x2000, CRC(eca33b2c) SHA1(938b021ea3b0f23aed7a98a930a58af371a02303) )
	ROM_LOAD( "tcs_u4.bin", 0x0e000, 0x2000, CRC(3490289a) SHA1(a9d56ea60bb901267da41ab408f8e1ed3742b0ac) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "demo_derby_bg_06f.6f", 0x00000, 0x2000, CRC(cf80be19) SHA1(a2ab09ee2dc76fab472fec7520ed972ccc10e826) ) /* Dated 2/7/85 */
	ROM_LOAD( "demo_derby_bg_15f.5f", 0x02000, 0x2000, CRC(4e173e52) SHA1(ac5ae8007a63f9c074444783c1058109327dd118) ) /* Dated 2/7/85 */

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "demo_derby_fg0_a4.a4",   0x00000, 0x4000, CRC(e57a4de6) SHA1(d1b2396a85b984e171d751ef8e1cf970ac4ff9fb) ) /* Dated 3/11/85 */
	ROM_LOAD( "demo_derby_fg4_a3.a3",   0x04000, 0x4000, CRC(55aa667f) SHA1(d611dbf9e8ef383d02514b0edb9ea36670193bf0) ) /* Dated 3/11/85 */
	ROM_LOAD( "demo_derby_fg1_a6.a6",   0x08000, 0x4000, CRC(70259651) SHA1(55967aaf2a7617c8f5a199d1e07128d79ce16970) ) /* Dated 3/11/85 */
	ROM_LOAD( "demo_derby_fg5_a5.a5",   0x0c000, 0x4000, CRC(5fe99007) SHA1(9d640b4715333efdc6300dc353991d6934929399) ) /* Dated 3/11/85 */
	ROM_LOAD( "demo_derby_fg2_a8.a8",   0x10000, 0x4000, CRC(6cab7b95) SHA1(8faff7458ab5ff2dd096dd78b1449a4096cc6345) ) /* Dated 3/11/85 */
	ROM_LOAD( "demo_derby_fg6_a7.a7",   0x14000, 0x4000, CRC(abfb9a8b) SHA1(14ab416bc76db25ad97353c9072048c64ec95344) ) /* Dated 3/11/85 */
	ROM_LOAD( "demo_derby_fg3_a10.a10", 0x18000, 0x4000, CRC(801d9b86) SHA1(5a8c72d1060eea1a3ad67b98aa6eff13f6837af6) ) /* Dated 3/11/85 */
	ROM_LOAD( "demo_derby_fg7_a9.a9",   0x1c000, 0x4000, CRC(0ec3f60a) SHA1(4176b246b0ea7bce9498c20e12678f16f7173529) ) /* Dated 3/11/85 */
ROM_END



/*************************************
 *
 *  Driver initialization
 *
 *************************************/

void mcr_state::mcr_init(int cpuboard, int vidboard, int ssioboard)
{
	m_mcr_cpu_board = cpuboard;
	m_mcr_sprite_board = vidboard;

	m_mcr12_sprite_xoffs = 0;
	m_mcr12_sprite_xoffs_flip = 0;

	save_item(NAME(m_input_mux));
	save_item(NAME(m_last_op4));

	if (m_ssio.found())
	{
		m_ssio->set_custom_output(0, 0xff, *this, FUNC(mcr_state::mcr_control_port_w));
	}
}


void mcr_state::init_solarfox()
{
	mcr_init(90009, 91399, 90908);
	m_mcr12_sprite_xoffs = 16;

	m_ssio->set_custom_input(0, 0x1c, *this, FUNC(mcr_state::solarfox_ip0_r));
	m_ssio->set_custom_input(1, 0xff, *this, FUNC(mcr_state::solarfox_ip1_r));
}


void mcr_state::init_kick()
{
	mcr_init(90009, 91399, 90908);
	m_mcr12_sprite_xoffs_flip = 16;

	m_ssio->set_custom_input(1, 0xf0, *this, FUNC(mcr_state::kick_ip1_r));
}


void mcr_dpoker_state::init_dpoker()
{
	mcr_init(90009, 91399, 90908);
	m_mcr12_sprite_xoffs_flip = 16;

	m_ssio->set_custom_input(0, 0x8e, *this, FUNC(mcr_dpoker_state::ip0_r));

	// meter ram, is it battery backed?
	m_maincpu->space(AS_PROGRAM).install_ram(0x8000, 0x81ff, m_meter_ram);

	// extra I/O
	m_maincpu->space(AS_IO).install_read_port(0x24, 0x24, "P24");
	m_maincpu->space(AS_IO).install_read_port(0x28, 0x28, "P28");
	m_maincpu->space(AS_IO).install_read_port(0x2c, 0x2c, "P2C");

	m_maincpu->space(AS_IO).install_write_handler(0x2c, 0x2c, write8smo_delegate(*this, FUNC(mcr_dpoker_state::lamps1_w)));
	m_maincpu->space(AS_IO).install_write_handler(0x30, 0x30, write8smo_delegate(*this, FUNC(mcr_dpoker_state::lamps2_w)));
	m_maincpu->space(AS_IO).install_write_handler(0x34, 0x34, write8smo_delegate(*this, FUNC(mcr_dpoker_state::output_w)));
	m_maincpu->space(AS_IO).install_write_handler(0x3f, 0x3f, write8smo_delegate(*this, FUNC(mcr_dpoker_state::meters_w)));

	m_coin_status = 0;
	m_output = 0;

	save_item(NAME(m_coin_status));
	save_item(NAME(m_output));
}


void mcr_state::init_mcr_90010()
{
	mcr_init(90010, 91399, 90913);
}


void mcr_state::init_wacko()
{
	mcr_init(90010, 91399, 90913);

	m_ssio->set_custom_input(1, 0xff, *this, FUNC(mcr_state::wacko_ip1_r));
	m_ssio->set_custom_input(2, 0xff, *this, FUNC(mcr_state::wacko_ip2_r));
	m_ssio->set_custom_output(4, 0x01, *this, FUNC(mcr_state::wacko_op4_w));
}


void mcr_state::init_twotiger()
{
	mcr_init(90010, 91399, 90913);

	m_ssio->set_custom_output(4, 0xff, *this, FUNC(mcr_state::twotiger_op4_w));
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0xe800, 0xefff, 0, 0x1000, 0, read8sm_delegate(*this, FUNC(mcr_state::twotiger_videoram_r)), write8sm_delegate(*this, FUNC(mcr_state::twotiger_videoram_w)));
}


void mcr_state::init_kroozr()
{
	mcr_init(90010, 91399, 91483);

	m_ssio->set_custom_input(1, 0x47, *this, FUNC(mcr_state::kroozr_ip1_r));
	m_ssio->set_custom_output(4, 0x34, *this, FUNC(mcr_state::kroozr_op4_w));
}


void mcr_state::init_journey()
{
	mcr_init(91475, 91464, 90913);

	m_ssio->set_custom_output(4, 0x01, *this, FUNC(mcr_state::journey_op4_w));
}


void mcr_state::init_mcr_91490()
{
	mcr_init(91490, 91464, 90913);
}


void mcr_state::init_dotrone()
{
	mcr_init(91490, 91464, 91657);

	m_ssio->set_custom_output(4, 0xff, *this, FUNC(mcr_state::dotron_op4_w));
}


void mcr_nflfoot_state::init_nflfoot()
{
	mcr_init(91490, 91464, 91657);

	m_ssio->set_custom_input(2, 0x80, *this, FUNC(mcr_nflfoot_state::ip2_r));
	m_ssio->set_custom_output(4, 0xff, *this, FUNC(mcr_nflfoot_state::op4_w));

	save_item(NAME(m_ipu_sio_txda));
	save_item(NAME(m_ipu_sio_txdb));
}


void mcr_state::init_demoderb()
{
	mcr_init(91490, 91464, 90913);

	m_ssio->set_custom_input(1, 0xfc, *this, FUNC(mcr_state::demoderb_ip1_r));
	m_ssio->set_custom_input(2, 0xfc, *this, FUNC(mcr_state::demoderb_ip2_r));
	m_ssio->set_custom_output(4, 0xff, *this, FUNC(mcr_state::demoderb_op4_w));

	// the SSIO Z80 doesn't have any program to execute
	m_ssio->suspend_cpu();
}



/*************************************
 *
 *  Game drivers
 *
 *************************************/

/* 90009 CPU board + 91399 video gen + 90908 sound I/O */
GAME(  1981, solarfox,  0,        mcr_90009,     solarfox,  mcr_state,         init_solarfox,  ROT90 ^ ORIENTATION_FLIP_Y, "Bally Midway", "Solar Fox (upright)", MACHINE_SUPPORTS_SAVE )
GAME(  1981, kick,      0,        mcr_90009,     kick,      mcr_state,         init_kick,      ORIENTATION_SWAP_XY, "Midway", "Kick (upright)", MACHINE_SUPPORTS_SAVE )
GAME(  1981, kickman,   kick,     mcr_90009,     kick,      mcr_state,         init_kick,      ORIENTATION_SWAP_XY, "Midway", "Kickman (upright)", MACHINE_SUPPORTS_SAVE )
GAME(  1981, kickc,     kick,     mcr_90009,     kickc,     mcr_state,         init_kick,      ROT90, "Midway", "Kick (cocktail)", MACHINE_SUPPORTS_SAVE )
GAMEL( 1985, dpoker,    0,        mcr_90009_dp,  dpoker,    mcr_dpoker_state,  init_dpoker,    ROT0,  "Bally",  "Draw Poker (Bally, 03-20)", MACHINE_SUPPORTS_SAVE, layout_dpoker )

/* 90010 CPU board + 91399 video gen + 90913 sound I/O */
GAME(  1981, shollow,   0,        mcr_90010,     shollow,   mcr_state,         init_mcr_90010, ROT90, "Bally Midway", "Satan's Hollow (set 1)", MACHINE_SUPPORTS_SAVE )
GAME(  1981, shollow2,  shollow,  mcr_90010,     shollow,   mcr_state,         init_mcr_90010, ROT90, "Bally Midway", "Satan's Hollow (set 2)", MACHINE_SUPPORTS_SAVE )
GAME(  1982, tron,      0,        mcr_90010,     tron,      mcr_state,         init_mcr_90010, ROT90, "Bally Midway", "Tron (8/9)", MACHINE_SUPPORTS_SAVE )
GAME(  1982, tron2,     tron,     mcr_90010,     tron,      mcr_state,         init_mcr_90010, ROT90, "Bally Midway", "Tron (6/25)", MACHINE_SUPPORTS_SAVE )
GAME(  1982, tron3,     tron,     mcr_90010,     tron3,     mcr_state,         init_mcr_90010, ROT90, "Bally Midway", "Tron (6/17)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
GAME(  1982, tron4,     tron,     mcr_90010,     tron3,     mcr_state,         init_mcr_90010, ROT90, "Bally Midway", "Tron (6/15)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
GAME(  1982, tron5,     tron,     mcr_90010,     tron3,     mcr_state,         init_mcr_90010, ROT90, "Bally Midway", "Tron (5/12)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
GAME(  1982, tronger,   tron,     mcr_90010,     tron3,     mcr_state,         init_mcr_90010, ROT90, "Bally Midway", "Tron (Germany)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
GAME(  1982, domino,    0,        mcr_90010,     domino,    mcr_state,         init_mcr_90010, ROT0,  "Bally Midway", "Domino Man (set 1)", MACHINE_SUPPORTS_SAVE )
GAME(  1982, dominoa,   domino,   mcr_90010,     domino,    mcr_state,         init_mcr_90010, ROT0,  "Bally Midway", "Domino Man (set 2)", MACHINE_SUPPORTS_SAVE )
GAME(  1982, wacko,     0,        mcr_90010,     wacko,     mcr_state,         init_wacko,     ROT0,  "Bally Midway", "Wacko", MACHINE_SUPPORTS_SAVE )
GAME(  1984, twotigerc, twotiger, mcr_90010,     twotigrc,  mcr_state,         init_mcr_90010, ROT0,  "Bally Midway", "Two Tigers (Tron conversion)", MACHINE_SUPPORTS_SAVE )

/* hacked 90010 CPU board + 91399 video gen + 90913 sound I/O + 8-track interface */
GAME(  1984, twotiger,  0,        mcr_90010_tt,  twotiger,  mcr_state,         init_twotiger,  ROT0,  "Bally Midway", "Two Tigers (dedicated)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

/* 90010 CPU board + 91399 video gen + 91483 sound I/O */
GAME(  1982, kroozr,    0,        mcr_90010,     kroozr,    mcr_state,         init_kroozr,    ROT0,  "Bally Midway", "Kozmik Kroozr", MACHINE_SUPPORTS_SAVE )

/* 91475 CPU board + 91464 video gen + 90913 sound I/O + cassette interface */
GAME(  1983, journey,   0,        mcr_91475,     journey,   mcr_state,         init_journey,   ROT90, "Bally Midway", "Journey", MACHINE_SUPPORTS_SAVE )

/* 91490 CPU board + 91464 video gen + 90913 sound I/O */
GAME(  1983, tapper,    0,        mcr_91490,     tapper,    mcr_state,         init_mcr_91490, ROT0,  "Bally Midway", "Tapper (Budweiser, 1/27/84)", MACHINE_SUPPORTS_SAVE ) /* Date from program ROM labels - Newest version */
GAME(  1983, tapperg,   tapper,   mcr_91490,     tapper,    mcr_state,         init_mcr_91490, ROT0,  "Bally Midway", "Tapper (Budweiser, 1/27/84 - Alternate graphics)", MACHINE_SUPPORTS_SAVE ) /* Date from program ROM labels - Newest version */
GAME(  1983, tappera,   tapper,   mcr_91490,     tapper,    mcr_state,         init_mcr_91490, ROT0,  "Bally Midway", "Tapper (Budweiser, 1/12/84)", MACHINE_SUPPORTS_SAVE ) /* Date from program ROM labels */
GAME(  1983, tapperb,   tapper,   mcr_91490,     tapper,    mcr_state,         init_mcr_91490, ROT0,  "Bally Midway", "Tapper (Budweiser, 12/9/83)", MACHINE_SUPPORTS_SAVE ) /* Date from program ROM labels - The oldest set? */
GAME(  1983, sutapper,  tapper,   mcr_91490,     tapper,    mcr_state,         init_mcr_91490, ROT0,  "Bally Midway", "Tapper (Suntory)", MACHINE_SUPPORTS_SAVE )
GAME(  1984, rbtapper,  tapper,   mcr_91490,     tapper,    mcr_state,         init_mcr_91490, ROT0,  "Bally Midway", "Tapper (Root Beer)", MACHINE_SUPPORTS_SAVE )
GAME(  1984, timber,    0,        mcr_91490,     timber,    mcr_state,         init_mcr_91490, ROT0,  "Bally Midway", "Timber", MACHINE_SUPPORTS_SAVE )
GAME(  1983, dotron,    0,        mcr_91490,     dotron,    mcr_state,         init_mcr_91490, ORIENTATION_FLIP_X, "Bally Midway", "Discs of Tron (Upright, 10/4/83)", MACHINE_SUPPORTS_SAVE )
GAME(  1983, dotrona,   dotron,   mcr_91490,     dotron,    mcr_state,         init_mcr_91490, ORIENTATION_FLIP_X, "Bally Midway", "Discs of Tron (Upright, 9/22/83)", MACHINE_SUPPORTS_SAVE )

/* 91490 CPU board + 91464 video gen + 91657 sound I/O + Squawk n' Talk */
GAME(  1983, dotrone,   dotron,   mcr_91490_snt, dotrone,   mcr_state,         init_dotrone,   ORIENTATION_FLIP_X, "Bally Midway", "Discs of Tron (Environmental)", MACHINE_SUPPORTS_SAVE )
GAME(  1983, dotronep,  dotron,   mcr_91490_snt, dotrone,   mcr_state,         init_dotrone,   ORIENTATION_FLIP_X, "Bally Midway", "Discs of Tron (Environmental, prototype, 8/9/83)", MACHINE_SUPPORTS_SAVE )

/* 91490 CPU board + 91464 video gen + 91657 sound I/O + Squawk n' Talk + IPU laserdisk interface */
GAME(  1983, nflfoot,   0,        mcr_91490_ipu, nflfoot,   mcr_nflfoot_state, init_nflfoot,   ROT0,  "Bally Midway", "NFL Football", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )

/* 91490 CPU board + 91464 video gen + 90913 sound I/O + Turbo Cheap Squeak */
GAME(  1984, demoderb,  0,        mcr_91490_tcs, demoderb,  mcr_state,         init_demoderb,  ROT0,  "Bally Midway", "Demolition Derby", MACHINE_SUPPORTS_SAVE )
GAME(  1984, demoderbc, demoderb, mcr_91490_tcs, demoderbc, mcr_state,         init_demoderb,  ROT0,  "Bally Midway", "Demolition Derby (cocktail)", MACHINE_SUPPORTS_SAVE )
