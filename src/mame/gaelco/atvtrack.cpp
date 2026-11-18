// license:BSD-3-Clause
// copyright-holders:Samuele Zannoli, Peter Wilhelmsen
/*
  ATV Track
  (c)2002 Gaelco

ATV Track
Gaelco 2002

PCB Layout

GAELCO
REF. 020419
 |--------------------------------------------------------------|
 |                                                              |
 |   SW3                                               EPC1PC8  |
 |                                         K4S643232            |
 |    LC245A                                         7LB176    |-|
 |                        FLASH.IC14  FLASH.IC19     7LB176    | |
 |       |-----|                                     7LB176    | |DB9
 |       | SH4 |                                     7LB176    |-|
 |       |     |          FLASH.IC15  FLASH.IC20                |
 |       |-----|                                                |
 |                                                |----------|  |
|-|                            K4S643232          |ALTERA    |  |
| |  L4955                                        |FLEX      |  |
| |      |-----|               K4S643232          |EPF10K50  |  |
| |CN1   | SH4 |                                  |EQC240-3  |  |
| |      |     |                                  |----------|  |
| |      |-----|                                                |
|-|                                                             |
 |                                                              |
 |      33MHz                       K4S643232     |----------|  |
 |                                  K4S643232     | GFX      |  |
 |             LED                                |          | |-|
 |             LED                                |          | | |
 |                                  K4S643232     |          | | |DB9
 |                                  K4S643232     |----------| |-|
 | TL074C   TL074C                                     385-1    |
 |     TDA1387   TDA1387                          14.31818MHz   |
 |                                                              |
 |--------------------------------------------------------------|
Notes:
      SH4       - Hitachi HD6417750S SH4 CPU (BGA)
      K4S643232 - Samsung K4S643232E-TC70 64M x 32-bit SDRAM (TSSOP86)
      GFX       - NEC PowerVR Neon 250
      FLASH.IC* - Samsung K9F2808U0B 128MBit (16M + 512k Spare x 8-bit) FlashROM (TSOP48)
      EPF10K50  - Altera Flex EPF10K50EQC240-3 FPGA (QFP240)
      EPC1PC8   - Altera EPC1PC8 FPGA Configuration Device (DIP8)
      TL074C    - Texas Instruments TL074C Low Noise Quad JFet Operational Amplifier (SOIC14)
      TDA1387   - Philips TDA1387 Stereo Continuous Calibration DAC (SOIC8)
      L4955     - ST Microelectronics L4955 low-dropout voltage regulator
      7LB176    - Texas Instruments 7LB176 Differential Bus Tranceiver (SOIC8)
      385-1     - National LM385 Adjustable Micropower Voltage Reference Diode (SOIC8)
      CN1       - Multi-pin connector for filter board (input, video, power & controls connectors etc)
      DB9       - Probably used for cabinet linking
      SW3       - Push button switch


Gaelco Football
(c) 2002 Gaelco

PCB:
GAELCO
REF. 020201
Same PCB as above ATV Track, except for HD6417750 SH4 CPUs was used instead of HD6417750S.

*/

/*

notes from DEMUL team

Smashing Drive needs a working SH4 MMU emulation, ATV Track does not.

Audio - is a simple buffered DAC.
frequency is 32kHz
data written by CPU to buffer have such meaning:
offs 0 - s16 bass channel 0
offs 2 - s16 bass channel 1
offs 4 - s16 left channel
offs 6 - s16 right channel
and so on

buffer is 2x32bytes
then it becomes (I suppose half) empty - SH4 IRL5 IRQ generated


"control registers" (Smashing Drive)
0 - read - various statuses, returning -1 is OK
write - enable slave CPU, gpu, etc most of bits is unclear
4 - w - RS422/485 communication port (for cabinet linking)

SH4 XTAL is 33MHz, SH4 MD0-2 pins is 001 or 011 (CPU core clk = XTAL*6, peripheral clk = XTAL, bus clk is XTAL or XTAL*2)

The volume of Smashing Drive is one of the settings that it stores in the program flash, and the
dumps have it at zero.  Nothing is heard before Volume Up is pressed, and the volume is lost
again unless the settings are then saved, which the last screen of the test mode does.

The DAC that feeds the motor boards is not identified.  The programs drive it as a TLC5620, and
the filter boards have the four analog outputs that such a part would give.

TODO:
    cabinet link: a word port in the FPGA served by DMA with external requests, channel 0 to
    receive and 1 to send.  Smashing Drive arms channel 0 at start; nothing else happens while
    the games are set as not linked, which is how they are dumped and the only way they are
    meant to be run here for now.
    Gaelco Football's ball sensor boards are simulated, their PIC is not dumped
    the platform of ATV Track stays where it is: its position comes back on two potentiometers,
    which read as centred all the time, and the motion test goes on waiting for it to move.
    What the programs put out for the motors, on the DAC and on the three lines next to it:
        channels 0, 1  the motor of the handlebar (ATV Track) or of the wheel (Smashing Drive),
                       a channel for each direction
        channels 2, 3  ATV Track: the speeds of the two platform motors
        channel 2      Smashing Drive: the seat motor
        line 0         ATV Track: high for as long as the program drives the platform motors
        lines 1, 2     ATV Track: low for one direction and high for the other
                       (Smashing Drive keeps all three lines low)
    Smashing Drive's third ROM is read as plain bytes, while the first two are halves of 32-bit words: how is it wired?

*/

#include "emu.h"

#include "cpu/sh/sh4.h"
#include "machine/adc083x.h"
#include "machine/intelfsh.h"
#include "machine/nandflash.h"
#include "machine/tlc5620.h"
#include "sound/dac.h"
#include "video/powervr250.h"

#include "screen.h"
#include "speaker.h"

#include "multibyte.h"

#include <algorithm>
#include <cmath>
#include <iterator>

#define LOG_PORT    (1U << 1)
#define LOG_LINK    (1U << 2)
#define LOG_AUDIO   (1U << 3)

#define VERBOSE (0)
#include "logmacro.h"

#define LOGPORT(...)    LOGMASKED(LOG_PORT, __VA_ARGS__)
#define LOGLINK(...)    LOGMASKED(LOG_LINK, __VA_ARGS__)
#define LOGAUDIO(...)   LOGMASKED(LOG_AUDIO, __VA_ARGS__)


namespace {

class atvtrack_base_state : public driver_device
{
protected:
	atvtrack_base_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_subcpu(*this, "subcpu")
		, m_gpu(*this, "gpu")
		, m_adc(*this, "adc")
		, m_motor_dac(*this, "motordac")
		, m_dac(*this, "dac%u", 0U)
		, m_sharedmem(*this, "sharedmem")
		, m_mainram(*this, "mainram")
		, m_subram(*this, "subram")
		, m_in(*this, "IN%u", 0U)
		, m_analog(*this, "AN%u", 0U)
		, m_lamps(*this, "lamp%u", 0U)
		, m_motor(*this, "motor%u", 0U)
		, m_motor_line(*this, "motor_line%u", 0U)
	{ }

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void atvtrack_common(machine_config &config) ATTR_COLD;

	void main_port(address_map &map) ATTR_COLD;
	void sub_map(address_map &map) ATTR_COLD;

	u32 control_r();
	void control_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void audio_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	virtual void system_w(u8 data);
	double adc_input_r(u8 input);
	void motor_w(offs_t offset, u16 data);

	required_device<sh7750s_device> m_maincpu;
	required_device<sh7750s_device> m_subcpu;
	required_device<powervr250_device> m_gpu;
	optional_device<adc0834_device> m_adc;
	optional_device<tlc5620_device> m_motor_dac;
	required_device_array<dac_word_interface, 4> m_dac;
	required_shared_ptr<u64> m_sharedmem;
	required_shared_ptr<u64> m_mainram;
	required_shared_ptr<u64> m_subram;

private:
	u32 porta_r();
	void porta_w(u32 data);
	void portb_w(u32 data);
	void dma_request_w(u32 data);

	void update_subcpu_reset();
	void update_audio_irq();
	TIMER_CALLBACK_MEMBER(audio_tick);

	required_ioport_array<4> m_in;
	required_ioport_array<4> m_analog;
	output_finder<2> m_lamps;
	output_finder<4> m_motor;
	output_finder<3> m_motor_line;

	emu_timer *m_audio_timer = nullptr;

	u32 m_control = 0;
	u16 m_porta = 0;
	bool m_subcpu_enable = false;

	u16 m_audio_buf[32]{};
	u8 m_audio_pos = 0;
	bool m_audio_irq = false;
};


// REF. 020419 (ATV Track) and REF. 020201 (Gaelco Football): program in NAND flash
class atvtrack_state : public atvtrack_base_state
{
public:
	atvtrack_state(const machine_config &mconfig, device_type type, const char *tag)
		: atvtrack_base_state(mconfig, type, tag)
		, m_nand(*this, "nand%u", 0U)
		, m_fpga_config(*this, "fpga")
	{ }

	void atvtrack(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void nand_board(machine_config &config) ATTR_COLD;

private:
	void main_map(address_map &map) ATTR_COLD;

	required_device_array<samsung_k9f2808u0b_device, 4> m_nand;
	required_region_ptr<u8> m_fpga_config;

	u32 m_boot[256]{};
};


// REF. 020201 (Gaelco Football): the same board, with a filter board for two sets of controls and two ball sensor boards
class gfootbal_state : public atvtrack_state
{
public:
	gfootbal_state(const machine_config &mconfig, device_type type, const char *tag)
		: atvtrack_state(mconfig, type, tag)
		, m_ball_x(*this, "BALL%u_X", 1U)
		, m_ball_y(*this, "BALL%u_Y", 1U)
		, m_ball_z(*this, "BALL%u_Z", 1U)
	{ }

	void gfootbal(machine_config &config) ATTR_COLD;

	template <u32 Which> int ball_data_r() { return m_ball_data[Which] ? 1 : 0; }

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual void system_w(u8 data) override;

private:
	bool ball_kicked(u32 which);

	required_ioport_array<2> m_ball_x;
	required_ioport_array<2> m_ball_y;
	required_ioport_array<2> m_ball_z;

	u32 m_ball_frame[2]{};
	bool m_ball_data[2]{};
	bool m_ball_clock = false;
	u8 m_ball_edges = 0;
	attotime m_ball_edge_time;
	u32 m_ball_swing[2]{};
	u32 m_ball_kick[2]{};
	bool m_ball_away[2]{};
};


// REF. 010131 (Smashing Drive): program in ROM and flash
class smashdrv_state : public atvtrack_base_state
{
public:
	smashdrv_state(const machine_config &mconfig, device_type type, const char *tag)
		: atvtrack_base_state(mconfig, type, tag)
		, m_flash(*this, "prg")
	{ }

	void smashdrv(machine_config &config) ATTR_COLD;

private:
	void main_map(address_map &map) ATTR_COLD;
	void sub_map(address_map &map) ATTR_COLD;

	required_device<intelfsh8_device> m_flash;
};


/*
    FPGA: control registers

    0 read  - status, unknown
      write - bit 2: release the second CPU from reset (it also needs the system port bit)
              bit 3: ATV Track and Gaelco Football change it with every audio interrupt, use unknown
              bits 15-14: link port mode
    4 write - word to the cabinet link
*/

u32 atvtrack_base_state::control_r()
{
	// FIXME: the status bits are not understood, the programs are happy with all of them set
	return 0xffff'ffff;
}

void atvtrack_base_state::control_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (offset == 0)
	{
		COMBINE_DATA(&m_control);
		update_subcpu_reset();
	}
	else
	{
		LOGLINK("%s: link word %08x & %08x\n", machine().describe_context(), data, mem_mask);
	}
}

void atvtrack_base_state::update_subcpu_reset()
{
	m_subcpu->set_input_line(INPUT_LINE_RESET, (BIT(m_control, 2) && m_subcpu_enable) ? CLEAR_LINE : ASSERT_LINE);
}


/*
    FPGA: the cabinet's inputs and outputs, on port A of the main SH-4

    The CPU puts a function on bits 15-11, which are outputs, and passes eight bits of data
    either way on bits 10-3.  Each of bits 14-11 that is 0 connects a set of switches to the
    data bits, where a closed switch reads as 0.

    15-11  hex   function                  data bits
    11110  f0xx  operator panel switches   3 volume up, 4 service, 5 volume down, 6 test,
                                           8 coin, 9 data from the ADC
    11101  e8xx  player 1 game switches
    11011  d8xx  player 2 game switches (Gaelco Football)
    11111  f8xx  serial lines, outputs     3 data to the ADC, 4 data to the DAC, 5 clock of
                                           the two, 6 ADC chip select, 7 DAC load, 10-8 to
                                           the motor drivers
    01111  78xx  system outputs            3 coin counter, 4 first lamp, 5 second lamp,
                                           8 allows the second CPU to run

    The other bits:
    2  input, the FPGA has been configured
    1  output, 1 for as long as the CPU reads the data bits
    0  output, on Smashing Drive the clock that the FPGA is configured with, and bit 1 has
       the data for it
*/

u32 atvtrack_base_state::porta_r()
{
	// the pins read high; a switch connector whose select line is low pulls the data pins of its closed switches low
	u16 pins = 0xffff;
	for (u32 group = 0; group < 4; group++)
	{
		if (!BIT(m_porta, 11 + group))
			pins &= (m_in[group]->read() << 3) | 0xf807;
	}
	return pins;
}

void atvtrack_base_state::porta_w(u32 data)
{
	const u16 pins = data & 0xffff;
	LOGPORT("%s: port A %04x\n", machine().describe_context(), pins);

	if ((pins & 0xf000) == 0x7000 && BIT(pins, 8))
	{
		m_subcpu_enable = true;
		update_subcpu_reset();
	}

	// the outputs behind the data bits change only when the CPU drives those bits, and stay as they are when it reads them.
	// The upper half of what is written here is the port's control register for bits 7-0, which has the direction of data
	// bits 7-3 and not of 10-8: that of bit 3 is looked at, and the others are taken to go the same way
	if (!BIT(pins, 1) && BIT(data, 22))
	{
		switch (BIT(pins, 11, 5))
		{
		case 0x1f:
			if (m_adc)
			{
				m_adc->cs_write(BIT(pins, 6));
				m_adc->clk_write(BIT(pins, 5));
				m_adc->di_write(BIT(pins, 3));
			}
			if (m_motor_dac)
			{
				m_motor_dac->data_w(BIT(pins, 4));
				m_motor_dac->clk_w(BIT(pins, 5));
				m_motor_dac->load_w(BIT(pins, 7));
				for (u32 i = 0; i < 3; i++)
					m_motor_line[i] = BIT(pins, 8 + i);
			}
			break;
		case 0x0f:
			system_w(BIT(pins, 3, 8));
			break;
		}
	}

	m_porta = pins;
}

void atvtrack_base_state::portb_w(u32 data)
{
	// TODO: the main CPU toggles bit 0 of the 4-bit port, its use is not known
	LOGPORT("%s: port B %x\n", machine().describe_context(), data & 0x0f);
}

void atvtrack_base_state::dma_request_w(u32 data)
{
	// TODO: the cabinet link is served by DMA channels 0 and 1
	LOGLINK("%s: DMA channel %u armed, request source %u\n", machine().describe_context(), data & 0xffff, data >> 16);
}

void atvtrack_base_state::system_w(u8 data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));
	m_lamps[0] = BIT(data, 1);
	m_lamps[1] = BIT(data, 2);
}

void atvtrack_base_state::motor_w(offs_t offset, u16 data)
{
	m_motor[offset] = data;
}

double atvtrack_base_state::adc_input_r(u8 input)
{
	switch (input)
	{
	case ADC083X_CH0:
	case ADC083X_CH1:
	case ADC083X_CH2:
	case ADC083X_CH3:
		return double(m_analog[input - ADC083X_CH0]->read()) * 5.0 / 255.0;
	case ADC083X_VREF:
		return 5.0;
	default:
		return 0.0;
	}
}


/*
    FPGA: audio

    The CPU fills a buffer of 64 bytes, which the FPGA plays round and round: eight frames,
    each of them four signed 16-bit samples that go out together.  Every time four frames, one
    half of the buffer, have been played the FPGA requests more on the IRL1 pin, and it keeps
    the request up until the buffer is written to.

    The four samples of a frame, in their order:
    0, 1  subwoofer.  ATV Track and Gaelco Football keep sample 0 silent and play their
          subwoofer test on sample 1, Smashing Drive writes one value to the two
    2     right speaker
    3     left speaker
*/

void atvtrack_base_state::audio_w(offs_t offset, u16 data, u16 mem_mask)
{
	LOGAUDIO("%s: audio buffer %02x = %04x & %04x\n", machine().describe_context(), offset * 2, data, mem_mask);
	COMBINE_DATA(&m_audio_buf[offset]);
	if (m_audio_irq)
	{
		m_audio_irq = false;
		update_audio_irq();
	}
}

TIMER_CALLBACK_MEMBER(atvtrack_base_state::audio_tick)
{
	for (u32 ch = 0; ch < 4; ch++)
		m_dac[ch]->write(m_audio_buf[m_audio_pos * 4 + ch]);

	m_audio_pos = (m_audio_pos + 1) & 7;
	if (!(m_audio_pos & 3))
	{
		m_audio_irq = true;
		update_audio_irq();
	}
}

void atvtrack_base_state::update_audio_irq()
{
	// the program takes the four IRL pins as separate interrupts
	m_maincpu->set_input_line(SH4_IRL1, m_audio_irq ? ASSERT_LINE : CLEAR_LINE);
}


void atvtrack_base_state::machine_start()
{
	m_audio_timer = timer_alloc(FUNC(atvtrack_base_state::audio_tick), this);

	// let the recompiler get at the RAM without going through the memory system
	m_maincpu->sh2drc_add_fastram(0x0c00'0000, 0x0c7f'ffff, false, m_mainram);
	m_subcpu->sh2drc_add_fastram(0x0c00'0000, 0x0cff'ffff, false, m_subram);

	save_item(NAME(m_control));
	save_item(NAME(m_porta));
	save_item(NAME(m_subcpu_enable));
	save_item(NAME(m_audio_buf));
	save_item(NAME(m_audio_pos));
	save_item(NAME(m_audio_irq));
}

void atvtrack_base_state::machine_reset()
{
	m_control = 0;
	m_porta = 0;
	m_subcpu_enable = false;
	update_subcpu_reset();
	system_w(0);

	std::fill(std::begin(m_audio_buf), std::end(m_audio_buf), 0);
	m_audio_pos = 0;
	m_audio_irq = false;
	update_audio_irq();
	for (auto &dac : m_dac)
		dac->write(0);

	// the divider is not confirmed on a board, but with it the tone of ATV Track's speaker tests comes out at 125Hz
	const attotime period = attotime::from_hz(33_MHz_XTAL / 1024);
	m_audio_timer->adjust(period, 0, period);
}


// extract a 4kbit embedded array block from the configuration stream of the Altera FLEX 10KE
void get_altera10ke_eab(u8 *dst, u8 const *pof, int eab)
{
	// the base offset depends on the size of the POF header, which this dump of the EPC1PC8 lacks
	const u32 startbit = 0x45b * 8 + 1 + (0x25e6 * 8) * eab;

	for (u32 bit = 0; bit < 4096; bit++)
	{
		u32 tbit = bitswap<16>(bit, 15, 14, 13, 12,
			9, 8, 7, 6, 5, 4, 3,
			11, 10,
			2, 1, 0);
		tbit ^= 0x0fc0;
		const u32 pofbit = startbit + (tbit & 0x1f) + (BIT(tbit, 5) * 63) + (BIT(tbit, 6, 6) * 0x4d * 8);
		dst[bit / 8] &= ~(1 << (bit & 7));
		dst[bit / 8] |= BIT(pof[pofbit / 8], pofbit & 7) << (bit & 7);
	}
}

void atvtrack_state::machine_start()
{
	atvtrack_base_state::machine_start();

	// the first two EABs are the RAM both CPUs share, configured with the boot loader
	u8 eab[2][512] = { };
	get_altera10ke_eab(eab[0], m_fpga_config, 0);
	get_altera10ke_eab(eab[1], m_fpga_config, 1);

	for (u32 i = 0; i < 256; i++)
	{
		const u16 lword = bitswap<16>(get_u16le(&eab[1][i * 2]), 7, 9, 0, 10, 3, 11, 4, 12, 2, 15, 1, 13, 6, 8, 5, 14);
		const u16 hword = bitswap<16>(get_u16le(&eab[0][i * 2]), 5, 10, 7, 9, 6, 13, 3, 15, 2, 11, 1, 8, 0, 12, 4, 14);
		m_boot[i] = (u32(hword) << 16) | lword;
	}
}

/*
    Gaelco Football: ball sensor boards

    Each ball has a board with accelerometers and a PIC16C710 that is not dumped.  Gaelco's
    schematic of an earlier board of the kind (ACEL_V2, January 2000) has an ADXL150 on the
    PIC's AN0 and the two axes of an ADXL250 on AN1 and AN2, a line from the main board on RB4,
    where either edge interrupts the PIC, and a line to it from RB1.

    One clock line, bit 7 of the system outputs, goes to the two boards, and each board answers
    on a data line of its own; the program reads these on the pins that have the ADC in the
    other games.  The program goes through the steps below once a frame, and from what it does
    with the data lines a board has to answer like this:

    clock                      data line
    rises                      goes low, the program does not go on before that
    falls                      goes high again when there is a kick to report
    rises and falls 12 times   a bit after each of the 24 edges, which the program collects:
                               three bytes, each from bit 0 up
    rises and falls once more

    Of the three bytes the program makes a kick.  The height is half of what the first byte is
    above 0x30.  The second and third bytes count from 0x80, and as nothing when they are below
    that: each is the power on one side, and how far the two are apart is the direction.  This
    agrees with the bytes being the three accelerometers in the order of the PIC's inputs, if
    the two axes of the ADXL250 point 45 degrees to the left and to the right of straight ahead.

    Not known is how a board gets from what it measures to the three bytes, and what is on its
    data line while it has nothing to tell.  The balls are analog sticks here, as those of
    Football Power are in gaelco3d.cpp, with an axis for each accelerometer: a ball is kicked
    as far as its stick went forwards and sideways, once the stick does not go any further, and
    then has to come back.  The third axis, what the ADXL150 has got, is the first byte as it is.
*/

bool gfootbal_state::ball_kicked(u32 which)
{
	// forwards and to either side, there is no kicking a ball backwards
	const int forward = std::max(int(m_ball_y[which]->read()) - 0x80, 0);
	const int side = int(m_ball_x[which]->read()) - 0x80;
	const u32 swing = (forward * forward) + (side * side);

	// a ball that was kicked has to come back before it is kicked again
	if (m_ball_away[which])
	{
		m_ball_away[which] = swing != 0;
		return false;
	}

	if (swing > m_ball_swing[which])
	{
		// on its way out, what the accelerometers have got is the most so far
		const double scale = ((swing > (127 * 127)) ? (127.0 / std::sqrt(double(swing))) : 1.0) / std::sqrt(2.0);
		const u8 left = 0x80 + std::max(int(std::lround((forward - side) * scale)), 0);
		const u8 right = 0x80 + std::max(int(std::lround((forward + side) * scale)), 0);
		m_ball_kick[which] = (u32(right) << 16) | (u32(left) << 8) | m_ball_z[which]->read();
		m_ball_swing[which] = swing;
		return false;
	}

	if (!m_ball_swing[which])
		return false;
	m_ball_swing[which] = 0;
	m_ball_away[which] = swing != 0;
	return true;
}

void gfootbal_state::system_w(u8 data)
{
	atvtrack_state::system_w(data);

	const bool clock = BIT(data, 7);
	if (clock == m_ball_clock)
		return;
	m_ball_clock = clock;

	// an exchange that was cut short is forgotten
	const attotime now = machine().time();
	if ((now - m_ball_edge_time) > attotime::from_msec(10))
		m_ball_edges = 0;
	m_ball_edge_time = now;
	if (!m_ball_edges && !clock)
		return;

	for (u32 i = 0; i < 2; i++)
	{
		// low at first, then whether there is a kick, and the three bytes of it
		if (!m_ball_edges)
			m_ball_frame[i] = ball_kicked(i) ? ((m_ball_kick[i] << 2) | 2) : 0;
		m_ball_data[i] = (m_ball_edges < 26) ? BIT(m_ball_frame[i], m_ball_edges) : true;
	}
	if (++m_ball_edges == 28)
		m_ball_edges = 0;
}

void gfootbal_state::machine_start()
{
	atvtrack_state::machine_start();

	save_item(NAME(m_ball_frame));
	save_item(NAME(m_ball_data));
	save_item(NAME(m_ball_clock));
	save_item(NAME(m_ball_edges));
	save_item(NAME(m_ball_edge_time));
	save_item(NAME(m_ball_swing));
	save_item(NAME(m_ball_kick));
	save_item(NAME(m_ball_away));
}

void gfootbal_state::machine_reset()
{
	atvtrack_state::machine_reset();

	m_ball_frame[0] = m_ball_frame[1] = 0;
	m_ball_data[0] = m_ball_data[1] = true;
	m_ball_clock = false;
	m_ball_edges = 0;
	m_ball_edge_time = attotime::zero;
	m_ball_swing[0] = m_ball_swing[1] = 0;
	m_ball_kick[0] = m_ball_kick[1] = 0;
	m_ball_away[0] = m_ball_away[1] = false;
}

void atvtrack_state::machine_reset()
{
	atvtrack_base_state::machine_reset();

	// the FPGA is taken to be configured again by a reset
	for (u32 i = 0; i < 128; i++)
		m_sharedmem[i] = (u64(m_boot[i * 2 + 1]) << 32) | m_boot[i * 2];
}


void atvtrack_state::main_map(address_map &map)
{
	map(0x0000'0000, 0x0000'03ff).ram().share(m_sharedmem);
	map(0x0002'0000, 0x0002'0007).rw(FUNC(atvtrack_state::control_r), FUNC(atvtrack_state::control_w));
	map(0x0002'0040, 0x0002'007f).w(FUNC(atvtrack_state::audio_w));
	map(0x0c00'0000, 0x0c7f'ffff).mirror(0x0380'0000).ram().share(m_mainram);

	// the 32 data lines are shared out among the four NAND flash chips, eight to each
	for (u32 lane = 0; lane < 4; lane++)
	{
		const u64 mask = u64(0x0000'00ff'0000'00ff) << (8 * lane);
		map(0x1400'0000, 0x1400'0007).umask64(mask).rw(m_nand[lane], FUNC(nand_device::data_r), FUNC(nand_device::data_w));
		map(0x1410'0000, 0x1410'0007).umask64(mask).w(m_nand[lane], FUNC(nand_device::command_w));
		map(0x1420'0000, 0x1420'0007).umask64(mask).w(m_nand[lane], FUNC(nand_device::address_w));
	}
}

void smashdrv_state::main_map(address_map &map)
{
	map(0x0000'0000, 0x03ff'ffff).rom();
	map(0x0c00'0000, 0x0c7f'ffff).mirror(0x0380'0000).ram().share(m_mainram);
	map(0x1000'0000, 0x1000'03ff).ram().share(m_sharedmem);
	map(0x1000'0400, 0x1000'0407).rw(FUNC(smashdrv_state::control_r), FUNC(smashdrv_state::control_w));
	map(0x1000'0440, 0x1000'047f).w(FUNC(smashdrv_state::audio_w));
	map(0x1400'0000, 0x143f'ffff).rw(m_flash, FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write));
}

void atvtrack_base_state::main_port(address_map &map)
{
	map(0x00, 0x07).umask64(0x0000'0000'ffff'ffff).rw(FUNC(atvtrack_base_state::porta_r), FUNC(atvtrack_base_state::porta_w));
	map(0x08, 0x0f).umask64(0x0000'0000'ffff'ffff).w(FUNC(atvtrack_base_state::portb_w));
	map(0x10, 0x17).umask64(0x0000'0000'ffff'ffff).w(FUNC(atvtrack_base_state::dma_request_w));
}

void smashdrv_state::sub_map(address_map &map)
{
	// the ROMs and the program flash are on the second CPU's bus as well, but for the parts that lie under the shared RAM and the PowerVR 250
	atvtrack_base_state::sub_map(map);
	map(0x0000'0400, 0x03ff'ffff).rom().region("maincpu", 0x400);
	map(0x1400'8000, 0x143f'ffff).lr8(NAME([this] (offs_t offset) -> u8 { return m_flash->read(offset + 0x8000); }));
}

void atvtrack_base_state::sub_map(address_map &map)
{
	map(0x0000'0000, 0x0000'03ff).ram().share(m_sharedmem);
	map(0x0c00'0000, 0x0cff'ffff).mirror(0x0300'0000).ram().share(m_subram);
	map(0x1400'0000, 0x1400'3fff).rw(m_gpu, FUNC(powervr250_device::register_r), FUNC(powervr250_device::register_w));
	map(0x1400'4000, 0x1400'40ff).rw(m_gpu, FUNC(powervr250_device::config_r), FUNC(powervr250_device::config_w));
	map(0x1800'0000, 0x1bff'ffff).rw(m_gpu, FUNC(powervr250_device::fb_r), FUNC(powervr250_device::fb_w));
}


static INPUT_PORTS_START( gaelco_sh4 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("adc", FUNC(adc0834_device::do_read))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("AN0")
	PORT_BIT( 0xff, 0x80, IPT_UNUSED )

	PORT_START("AN1")
	PORT_BIT( 0xff, 0x80, IPT_UNUSED )

	PORT_START("AN2")
	PORT_BIT( 0xff, 0x80, IPT_UNUSED )

	PORT_START("AN3")
	PORT_BIT( 0xff, 0x80, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( atvtrack )
	PORT_INCLUDE( gaelco_sh4 )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Left Brake") // rear
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Right Brake") // front
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_TOGGLE PORT_NAME("Emergency Stop") // a break contact
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start / View")

	PORT_MODIFY("AN0")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x10, 0xf0) PORT_SENSITIVITY(25) PORT_KEYDELTA(25) PORT_REVERSE PORT_NAME("Handlebar")

	PORT_MODIFY("AN1")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(20) PORT_NAME("Throttle")
INPUT_PORTS_END

static INPUT_PORTS_START( gfootbal )
	PORT_INCLUDE( gaelco_sh4 )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(gfootbal_state::ball_data_r<0>))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(gfootbal_state::ball_data_r<1>))

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Change Player / Long Feint") // A
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Clear / Short Feint") // B
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 Run") // C
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_MODIFY("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Change Player / Long Feint") // A
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Clear / Short Feint") // B
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 Run") // C
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	// the balls: see the notes at the ball sensor boards
	PORT_START("BALL1_X")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(50) PORT_NAME("P1 Ball Sideways")

	PORT_START("BALL1_Y")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x00, 0xff) PORT_REVERSE PORT_SENSITIVITY(25) PORT_KEYDELTA(50) PORT_NAME("P1 Ball Forwards")

	PORT_START("BALL1_Z")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Z ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(50) PORT_NAME("P1 Ball Upwards")

	PORT_START("BALL2_X")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x00, 0xff) PORT_PLAYER(2) PORT_SENSITIVITY(25) PORT_KEYDELTA(50) PORT_NAME("P2 Ball Sideways")

	PORT_START("BALL2_Y")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x00, 0xff) PORT_PLAYER(2) PORT_REVERSE PORT_SENSITIVITY(25) PORT_KEYDELTA(50) PORT_NAME("P2 Ball Forwards")

	PORT_START("BALL2_Z")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Z ) PORT_MINMAX(0x00, 0xff) PORT_PLAYER(2) PORT_SENSITIVITY(25) PORT_KEYDELTA(50) PORT_NAME("P2 Ball Upwards")
INPUT_PORTS_END

static INPUT_PORTS_START( smashdrv )
	PORT_INCLUDE( gaelco_sh4 )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Accelerator")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Brake")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start / Horn")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("AN0")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x10, 0xf0) PORT_SENSITIVITY(25) PORT_KEYDELTA(25) PORT_NAME("Steering Wheel")
INPUT_PORTS_END


void atvtrack_base_state::atvtrack_common(machine_config &config)
{
	// MD2-0 put both CPUs in clock mode 3, in which they run at six times the frequency of the 33MHz crystal
	auto const cpu_config =
			[] (sh7750s_device &cpu)
			{
				cpu.set_md(0, 1);
				cpu.set_md(1, 1);
				cpu.set_md(2, 0);
				cpu.set_md(3, 0);
				cpu.set_md(4, 0);
				cpu.set_md(5, 1);
				cpu.set_md(6, 0);
				cpu.set_md(7, 1);
				cpu.set_md(8, 0);
				cpu.set_sh4_clock(33_MHz_XTAL * 6);
			};

	SH7750S(config, m_maincpu, 33_MHz_XTAL * 6);
	cpu_config(*m_maincpu);
	m_maincpu->set_addrmap(AS_IO, &atvtrack_base_state::main_port);

	SH7750S(config, m_subcpu, 33_MHz_XTAL * 6);
	cpu_config(*m_subcpu);
	m_subcpu->set_addrmap(AS_PROGRAM, &atvtrack_base_state::sub_map);

	// the CPUs talk through the RAM they share
	config.set_maximum_quantum(attotime::from_hz(6000));

	// the PowerVR 250 sets the screen up from the mode the program asks for
	screen_device &screen(SCREEN(config, "screen"));
	screen.set_raw(25'210'000, 804, 0, 640, 528, 0, 480);
	screen.set_screen_update(m_gpu, FUNC(powervr250_device::screen_update));

	POWERVR250(config, m_gpu, 14.318181_MHz_XTAL);
	m_gpu->set_screen("screen");
	m_gpu->irq_callback().set([this] (int state) { m_subcpu->sh4_set_irln_input(state ? 14 : 15); });

	// two stereo DACs; that both channels of the first go to the subwoofer is an assumption
	SPEAKER(config, "speaker", 2).front();
	SPEAKER(config, "woofer").lfe();
	DAC_16BIT_R2R_TWOS_COMPLEMENT(config, m_dac[0]).add_route(ALL_OUTPUTS, "woofer", 0.5);
	DAC_16BIT_R2R_TWOS_COMPLEMENT(config, m_dac[1]).add_route(ALL_OUTPUTS, "woofer", 0.5);
	DAC_16BIT_R2R_TWOS_COMPLEMENT(config, m_dac[2]).add_route(ALL_OUTPUTS, "speaker", 1.0, 1);
	DAC_16BIT_R2R_TWOS_COMPLEMENT(config, m_dac[3]).add_route(ALL_OUTPUTS, "speaker", 1.0, 0);
}

void atvtrack_state::nand_board(machine_config &config)
{
	atvtrack_common(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &atvtrack_state::main_map);

	for (auto &nand : m_nand)
		SAMSUNG_K9F2808U0B(config, nand);
}

void atvtrack_state::atvtrack(machine_config &config)
{
	nand_board(config);

	ADC0834(config, m_adc);
	m_adc->set_input_callback(FUNC(atvtrack_state::adc_input_r));

	// channels 0 and 1 turn the motor of the handlebar, each in one direction; 2 is the left motor of the platform, 3 the right
	TLC5620(config, m_motor_dac);
	m_motor_dac->out_callback().set(FUNC(atvtrack_state::motor_w));
}

void gfootbal_state::gfootbal(machine_config &config)
{
	nand_board(config);
}

void smashdrv_state::smashdrv(machine_config &config)
{
	atvtrack_common(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &smashdrv_state::main_map);
	m_subcpu->set_addrmap(AS_PROGRAM, &smashdrv_state::sub_map);

	ADC0834(config, m_adc);
	m_adc->set_input_callback(FUNC(smashdrv_state::adc_input_r));

	// channels 0 and 1 turn the motor of the wheel, each in one direction; 2 is the motor of the seat
	TLC5620(config, m_motor_dac);
	m_motor_dac->out_callback().set(FUNC(smashdrv_state::motor_w));

	// the program appends its settings and counters to the last two sectors with AMD's byte-wide commands; the type of the chip is not known
	AMD_29F032(config, m_flash);
}


ROM_START( atvtrack )
	ROM_REGION( 0x1080000, "nand0", 0 )
	ROM_LOAD( "15.bin", 0x0000000, 0x1080000, CRC(84eaede7) SHA1(6e6230165c3bb35e49c660dfd0d07c132ed89e6a) )

	ROM_REGION( 0x1080000, "nand1", 0 )
	ROM_LOAD( "20.bin", 0x0000000, 0x1080000, CRC(649dc331) SHA1(0cac2d0c15dd564c7fdebdf4365422958f453d63) )

	ROM_REGION( 0x1080000, "nand2", 0 )
	ROM_LOAD( "14.bin", 0x0000000, 0x1080000, CRC(67983453) SHA1(05389a0ffc1a1bae9bac16a53a97d78b6eccc626) )

	ROM_REGION( 0x1080000, "nand3", 0 )
	ROM_LOAD( "19.bin", 0x0000000, 0x1080000, CRC(9fc5c579) SHA1(8829329ef229564952aea2108ef1750dc226cbac) )

	ROM_REGION( 0x20000, "fpga", ROMREGION_ERASEFF)
	ROM_LOAD("epc1pc8.ic23", 0x0000000, 0x1ff01, CRC(752444c7) SHA1(c77e8fcfcbe15b53eda25553763bdac45f0ef7df) ) // contains configuration data for the fpga
ROM_END

ROM_START( atvtracka )
	ROM_REGION( 0x1080000, "nand0", 0 )
	ROM_LOAD( "k9f2808u0b.ic15", 0x0000000, 0x1080000, CRC(10730001) SHA1(48c685a6ff7135abd074dc7fb7d10834c44da58f) )

	ROM_REGION( 0x1080000, "nand1", 0 )
	ROM_LOAD( "k9f2808u0b.ic20", 0x0000000, 0x1080000, CRC(b0c34433) SHA1(852c79bb3d7082cd2c056140071ae7d71679ec1d) )

	ROM_REGION( 0x1080000, "nand2", 0 )
	ROM_LOAD( "k9f2808u0b.ic14", 0x0000000, 0x1080000, CRC(02a12085) SHA1(acb112c9c7b29d92610465fb92268ce787ca06f4) )

	ROM_REGION( 0x1080000, "nand3", 0 )
	ROM_LOAD( "k9f2808u0b.ic19", 0x0000000, 0x1080000, CRC(856c1e6a) SHA1(a6b2839120d61811c36cc6b4095de9cefceb394b) )

	ROM_REGION( 0x20000, "fpga", ROMREGION_ERASEFF)
	ROM_LOAD("epc1pc8.ic23", 0x0000000, 0x1ff01, CRC(752444c7) SHA1(c77e8fcfcbe15b53eda25553763bdac45f0ef7df) ) // contains configuration data for the fpga
ROM_END

/* Gaelco Football uses a small I/O PCB for connecting the balls (you control the game by kicking a real ball):
    -ADXL250JQC single/dual axis accelerometer.
    -PIC16C710.
    -8L05A linear voltage regulator.
    -UA741C single operational amplifier.
    -16 MHz osc.
*/
ROM_START( gfootbal )
	ROM_REGION( 0x1080000, "nand0", 0 )
	ROM_LOAD( "k9f2808u0b.ic15", 0x0000000, 0x1080000, CRC(876ca493) SHA1(d888be59d924fe23e725c6a8aa9609e9abcab608) )

	ROM_REGION( 0x1080000, "nand1", 0 )
	ROM_LOAD( "k9f2808u0b.ic20", 0x0000000, 0x1080000, CRC(df9cf6e2) SHA1(07be171c3768de8f548bceeaf3da8d3bf7cb85d3) )

	ROM_REGION( 0x1080000, "nand2", 0 )
	ROM_LOAD( "k9f2808u0b.ic14", 0x0000000, 0x1080000, CRC(48d901a3) SHA1(00f151b67e5603354a97708247bed46a2f1bbe1d) )

	ROM_REGION( 0x1080000, "nand3", 0 )
	ROM_LOAD( "k9f2808u0b.ic19", 0x0000000, 0x1080000, CRC(2db6c016) SHA1(490d23e338014b4e5f2f12dd04dfae2a93a15e11) )

	ROM_REGION( 0x20000, "fpga", ROMREGION_ERASEFF)
	ROM_LOAD("epc1pc8.ic23", 0x0000000, 0x1ff01, CRC(752444c7) SHA1(c77e8fcfcbe15b53eda25553763bdac45f0ef7df) ) // contains configuration data for the fpga

	ROM_REGION16_LE( 0x4010, "io", ROMREGION_ERASEFF )
	ROM_LOAD( "4r_pic16c710.u1", 0x0000, 0x4010, NO_DUMP ) // I/O for the ball controller
ROM_END

/*

Smashing Drive
Gaelco 2000

PCB Layout
----------

REF 010131
|----------------------------------------------|
|                                              |
|                              K4S643232C      |
|        |------|SDRB.IC14                     |
|        |SH4   |                              |-|
|        |      |                              | |DB9
|        |      |SDRA.IC15  SDRC.IC20  PRG.IC23|-|
|        |------|             |----------|     |
|                             |ALTERA    |     |
|                             |FLEX0K50  |     |
|        |------|  K4S643232C |EPF10K50  |     |
|        |SH4   |  K4S643232C |EQC240-3  |     |
|        |      |             |          |     |
|        |      |             |          |     |
|        |------|             |----------|     |
|                                              |
|         33MHz                 |--------|     |
|                    K4S643232C |NEC     |     |
|                    K4S643232C |POWERVR |     |
|                               |250     |     |
|                    K4S643232C |        |     |-|
|                    K4S643232C |        |     | |DB9
|                               |--------|     |-|
|                                              |
|  TL074C   TL074C                14.31818MHz  |
|  TDA1543  TDA1543                            |
|----------------------------------------------|

 PRG ROM useful locations:
   0x64C44 Country: 0 - World, 1 - Spain, 2 - UK, 3 - Italy, 4 - USA
   0x727AC 1 - enable Develop/debug option in test mode
*/

/*

Settings are stored in the main program flash ROM

They're stored at 3e0000 - 3e01ff then at 3f0000 - 3f01ff
             then 3e0200 - 3e03ff then at 3f0200 - 3f03ff
etc.

3eea00 - 3febff and 3fea00 - 3febff are the final ranges
used before it loops back to using 3e0000

The game loading the most recent settings found

There must be a valid settings block at 3e0000, blanking it
to a 0xff fill prevents the game from ever saving settings
and having invalid data there prevents the game from booting

Volume is saved as part of the settings, steering calibration
is not, so you must move the steering wheel fully to the left
and fully to the right on boot several times to correctly
center it.

The program ROMs in the sets below have been reset to a
single defaulted block at 3e0000 - 3e01ff

*/
ROM_START( smashdrv ) // World Version: 3.3, Version 3D: 1.9, Checksum: 707A
	ROM_REGION( 0x0400000, "prg", 0 )
	ROM_LOAD("prg_world.ic23", 0x0000000, 0x0400000, CRC(36d0328f) SHA1(22a8008a339168bc68e5a1cef413b8b465bb80e4) )

	ROM_REGION( 0x4000000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("sdra.ic15",    0x00000000, 0x01000000, CRC(cf702287) SHA1(84cd83c339831deff15fe5fcc353e0b596667500) )
	ROM_LOAD32_WORD("sdrb.ic14",    0x00000002, 0x01000000, CRC(39b76f0e) SHA1(529943b6075925e5f72c6e966796e04b2c33686c) )
	ROM_LOAD(       "sdrc.ic20",    0x02000000, 0x01000000, CRC(c9021dd7) SHA1(1d08aab433614810af858a0fc5d7f03c7b782237) ) // in one piece, where the two above each hold one half of every 32-bit word
	// ic21 unpopulated
ROM_END

ROM_START( smashdrvs ) // Spain/Portugal Version: 3.3, Version 3D: 1.9, Checksum: 707B. There is another known (undumped) Spain/Portugal (Covielsa license) set with Version: 3.3, Version 3D: 1.9, Checksum: EDD9.
	ROM_REGION( 0x0400000, "prg", 0 )
	ROM_LOAD("prg_spain.ic23", 0x0000000, 0x0400000, CRC(962a8055) SHA1(4f64536cb15bbfc9bc1a0d387e148b994a19f750) )

	ROM_REGION( 0x4000000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("sdra.ic15",    0x00000000, 0x01000000, CRC(cf702287) SHA1(84cd83c339831deff15fe5fcc353e0b596667500) )
	ROM_LOAD32_WORD("sdrb.ic14",    0x00000002, 0x01000000, CRC(39b76f0e) SHA1(529943b6075925e5f72c6e966796e04b2c33686c) )
	ROM_LOAD(       "sdrc.ic20",    0x02000000, 0x01000000, CRC(c9021dd7) SHA1(1d08aab433614810af858a0fc5d7f03c7b782237) ) // in one piece, where the two above each hold one half of every 32-bit word
	// ic21 unpopulated
ROM_END

ROM_START( smashdrvb ) // UK Version: 3.3, Version 3D: 1.9, Checksum: 707C
	ROM_REGION( 0x0400000, "prg", 0 )
	ROM_LOAD("prg.ic23", 0x0000000, 0x0400000, CRC(ac54517a) SHA1(fe53aa4022fdd7f8de786c860b8e4e97e2f44378) )

	ROM_REGION( 0x4000000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("sdra.ic15",    0x00000000, 0x01000000, CRC(cf702287) SHA1(84cd83c339831deff15fe5fcc353e0b596667500) )
	ROM_LOAD32_WORD("sdrb.ic14",    0x00000002, 0x01000000, CRC(39b76f0e) SHA1(529943b6075925e5f72c6e966796e04b2c33686c) )
	ROM_LOAD(       "sdrc.ic20",    0x02000000, 0x01000000, CRC(c9021dd7) SHA1(1d08aab433614810af858a0fc5d7f03c7b782237) ) // in one piece, where the two above each hold one half of every 32-bit word
	// ic21 unpopulated
ROM_END

} // anonymous namespace


GAME( 2002, atvtrack,  0,        atvtrack, atvtrack, atvtrack_state, empty_init, ROT0, "Gaelco",           "ATV Track (set 1)", MACHINE_NODEVICE_LAN | MACHINE_SUPPORTS_SAVE )
GAME( 2002, atvtracka, atvtrack, atvtrack, atvtrack, atvtrack_state, empty_init, ROT0, "Gaelco",           "ATV Track (set 2)", MACHINE_NODEVICE_LAN | MACHINE_SUPPORTS_SAVE )
GAME( 2002, gfootbal,  0,        gfootbal, gfootbal, gfootbal_state, empty_init, ROT0, "Gaelco / Zigurat", "Gaelco Football",   MACHINE_IMPERFECT_CONTROLS | MACHINE_SUPPORTS_SAVE ) // the balls

// almost identical PCB, FlashROM mapping and master registers addresses different
// these are all the same code revision, different region byte
GAME( 2000, smashdrv,  0,        smashdrv, smashdrv, smashdrv_state, empty_init, ROT0, "Gaelco",                       "Smashing Drive (World)",           MACHINE_NODEVICE_LAN | MACHINE_SUPPORTS_SAVE )
GAME( 2000, smashdrvb, smashdrv, smashdrv, smashdrv, smashdrv_state, empty_init, ROT0, "Gaelco (Brent Sales license)", "Smashing Drive (UK)",              MACHINE_NODEVICE_LAN | MACHINE_SUPPORTS_SAVE )
GAME( 2000, smashdrvs, smashdrv, smashdrv, smashdrv, smashdrv_state, empty_init, ROT0, "Gaelco (Covielsa license)",    "Smashing Drive (Spain, Portugal)", MACHINE_NODEVICE_LAN | MACHINE_SUPPORTS_SAVE )
