// license:BSD-3-Clause
// copyright-holders: Aaron Giles

/*************************************************************************

    Midway X-unit system

    driver by Aaron Giles
    based on older drivers by Ernesto Corvi, Alex Pasadyn, Zsolt Vasvari

    Games supported:
        * Revolution X

    TODO:
        * hook up PIC dump
***************************************************************************

Revolution X
Midway, 1994

This is the only known game on Midway X-Unit hardware.

PCB Layout
----------

5770-13534-04
WILLIAMS ELECTRONIC GAMES INC.
|----------------------------------------------------------------------------------------------------------------|
|   P8       PIC16C57  BR2325                LED1  LED2                                                          |
|3.6864MHz   MAX691  RESET_SW   |---------|                      42S4260                                         |
|SCC2691   LH5268               |LATTICE  |                                                                      |
|ADC0848                        |PLSI 1032|     MACH110          42S4260             |--------|       40MHz      |
|P4                             |(A-17719)|    (A-17720)                             |TMS34020|                  |
|                               |---------|                      42S4260             |        |                  |
|               8MHz                                                                 |        |                  |
|P12                                           GAL16V8           42S4260             |--------|                  |
|   P7                                        (A-17721)                                                          |
|                        HM538123                                                                                |
|                                                                                                                |
|--| FUSE                HM538123                                                                                |
   | ULN2064                                                                                                     |
|--|                     HM538123                    25MHz       U51           U52           U53         U54     |
|                                                                                                                |
|                        HM538123                                U63           U64           U65         U66     |
|J                                             |-----------|                                                     |
|A                       HM538123              |5410-12862 |     U71           U72           U73         U74     |
|M                                             |-00        |                                                     |
|M             MB84256   HM538123              |WILLIAMS   |     U81           U82           U83         U84     |
|A             MB84256                         |ELECTRONICS|                                                     |
|                        HM538123              |-----------|     U91           U92           U93         U94     |
|                                                                                                                |
|                        HM538123                                U101          U102          U103        U104    |
|--|     DIPSW(U105)           DIPSW(U108)        GAL16V8                                                        |
   |                                             (A-17722)       U110          U111          U112        U113    |
|--|                                                                                                             |
|          P2       P6      P10     P3      P5                   U120          U121          U122        U123    |
|----------------------------------------------------------------------------------------------------------------|
Notes:                                                         |                                               |
      TMS34020 - clock 40MHz                                   |--------------- All ROMs 27C4001 --------------|
      PIC16C57 - clock 0.625MHz [25/40]
      VSync    - 54.7074Hz
      HSync    - 15.8104kHz
      HM538123 - Hitachi HM538123 128k x8 Multiport CMOS Video DRAM with 256 x8 Serial Access Memory
      42S4260  - NEC 42S4260 256k x16 DRAM
      MB84256  - 32k x8 SRAM
      LH5268   - 8k x8 SRAM for CMOS storage / settings
      SCC2691  - Philips SCC2691 Universal asynchronous receiver/transmitter (UART)
      MAX691   - Maxim MAX691 Microprocessor Supervisory Circuit (for battery backup power switching)
      ADC0848  - Analog Devices ADC0848 8-Bit Microprocessor Compatible A/D Converter with Multiplexer Option
      P2       - Player 3 controls connector
      P3       - Player 4 controls connector
      P4       - Gun connector
      P5       - 15 pin connector
      P6       - 7 pin connector
      P7       - Power connector for sound board
      P8       - Sound board data cable connector
      P10      - 10 pin connector
      P12      - 4 pin connector

There's a separate sound board also.

SOUND BOARD - 5766-13959-01 - Williams Electronic Games Inc.
________________________________________________________________
|                   __________   __________  ________________   |
|                   PC74HCT541P  |74HC541N_| |U9 M27C4001    |  |
|                    __________  __________  |_______________|  |
|                  CY7C128A-35PC |74HC541N_|   ________________ |
|                                             |U8 M27C4001     ||
|                    __________  __________   |________________||
|                  CY7C128A-35PC SN74HC245N    ________________ |
|                    __________  __________   |U7 M27C4001     ||
|                  CY7C128A-35PC |74LS174N_|  |________________||
|                                              ________________ |
|                                __________   |U6 M27C4001     ||
|                                |74HC174N_|  |________________||
|                   __________                 ________________ |
|                   |ADSP-2105|  __________   |U5 M27C4001     ||
|             ____  |         |  M74HC138B1   |________________||
|             XTAL  |         |  __________    ________________ |
|        10.0000MHz |_________|  |_74F00PC_|  |U4 M27C4001     ||
|                   __________   __________   |________________||
|   __________ U17->|GAL16V8A_|  |_M7406N__|   ________________ |
|SCC2691AC1N24 _________                      |U3 27C040-10    ||
|    ________  |AD1851N_|                     |________________||
|    M74HC14B1                                 ________________ |
|    ________  _________                      |U2 M27C4001     ||
|   AM26LS31CN |TL084CN_|                     |________________||
|    ________  AUX-IN AUX-OUT                          FUSE     |
|   AM26LS31CN                    _________                     |
| :::::::::: <-SERIAL  PWR/PPKR-> ···· ····              ·· ··  |
|_______________________________________________________________|

**************************************************************************/

#include "emu.h"

#include "dcs.h"
#include "midtunit_v.h"

#include "cpu/pic16c5x/pic16c5x.h"
#include "cpu/tms34010/tms34010.h"
#include "machine/adc0844.h"
#include "machine/nvram.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


#define LOG_IO      (1U << 1)
#define LOG_UART    (1U << 2)
#define LOG_UNKNOWN (1U << 3)
#define LOG_SOUND   (1U << 4)

#define VERBOSE     (0)
#include "logmacro.h"


namespace {

class midxunit_state : public driver_device
{
public:
	midxunit_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_video(*this, "video")
		, m_dcs(*this, "dcs")
		, m_palette(*this, "palette")
		, m_nvram(*this, "nvram")
		, m_pic(*this, "pic")
		, m_nvram_data(*this, "nvram_data", 0x2000, ENDIANNESS_LITTLE)
		, m_gun_recoil(*this, "Player%u_Gun_Recoil", 1U)
		, m_gun_led(*this, "Player%u_Gun_LED", 1U)
	{ }

	void midxunit(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<tms340x0_device> m_maincpu;
	required_device<midtunit_video_device> m_video;
	required_device<dcs_audio_device> m_dcs;
	required_device<palette_device> m_palette;
	required_device<nvram_device> m_nvram;
	required_device<pic16c57_device> m_pic;
	memory_share_creator<uint8_t> m_nvram_data;
	output_finder<3> m_gun_recoil;
	output_finder<3> m_gun_led;

	uint16_t m_iodata[8] = {};
	uint8_t m_uart[8] = {};
	bool m_adc_int = false;

	uint8_t m_pic_command = 0;
	uint8_t m_pic_data = 0;
	uint8_t m_pic_clk = 0;
	uint8_t m_pic_status = 0;

	uint8_t cmos_r(offs_t offset);
	void cmos_w(offs_t offset, uint8_t data);
	void io_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void unknown_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void adc_int_w(int state);
	uint32_t status_r();
	uint8_t uart_r(offs_t offset);
	void uart_w(offs_t offset, uint8_t data);
	uint32_t security_r();
	void security_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void security_clock_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void dcs_output_full(int state);
	uint32_t dma_r(offs_t offset, uint32_t mem_mask = ~0);
	void dma_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	void main_map(address_map &map) ATTR_COLD;
};


/*************************************
 *
 *  CMOS reads/writes
 *
 *************************************/

uint8_t midxunit_state::cmos_r(offs_t offset)
{
	return m_nvram_data[offset];
}

void midxunit_state::cmos_w(offs_t offset, uint8_t data)
{
	m_nvram_data[offset] = data;
}


/*************************************
 *
 *  General I/O writes
 *
 *************************************/

void midxunit_state::io_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	offset = (offset / 2) % 8;
	uint16_t const oldword = m_iodata[offset];
	uint16_t newword = oldword;
	COMBINE_DATA(&newword);

	switch (offset)
	{
		case 2:
			// watchdog reset
//          watchdog_reset_w(0,0);
			break;

		default:
			// Gun Outputs for RevX
			// Note: The Gun for the Coin slot you use is supposed to rumble when you insert coins, and it doesn't for P3.
			// Perhaps an Input is hooked up wrong.
			m_gun_recoil[0] = BIT(data, 0);
			m_gun_recoil[1] = BIT(data, 1);
			m_gun_recoil[2] = BIT(data, 2);
			m_gun_led[0] = BIT(~data, 4);
			m_gun_led[1] = BIT(~data, 5);
			m_gun_led[2] = BIT(~data, 6);

			LOGMASKED(LOG_IO, "%s: I/O write to %d = %04X\n", machine().describe_context(), offset, data);
			break;
	}
	m_iodata[offset] = newword;
}


void midxunit_state::unknown_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	int const offs = offset / 0x40000;

	if (offs == 1 && ACCESSING_BITS_0_7)
		m_dcs->reset_w(~data & 2);

	if (ACCESSING_BITS_0_7 && offset % 0x40000 == 0)
		LOGMASKED(LOG_UNKNOWN, "%s: unknown_w @ %d = %02X\n", machine().describe_context(), offs, data & 0xff);
}


void midxunit_state::adc_int_w(int state)
{
	m_adc_int = (state != CLEAR_LINE);
}



/*************************************
 *
 *  General I/O reads
 *
 *************************************/

uint32_t midxunit_state::status_r()
{
	// low bit indicates whether the ADC is done reading the current input
	return (m_pic_status << 1) | (m_adc_int ? 1 : 0);
}



/*************************************
 *
 *  Revolution X UART
 *
 *************************************/

void midxunit_state::dcs_output_full(int state)
{
	// only signal if not in loopback state
	if (m_uart[1] != 0x66)
		m_maincpu->set_input_line(1, state ? ASSERT_LINE : CLEAR_LINE);
}


uint8_t midxunit_state::uart_r(offs_t offset)
{
	uint8_t result = 0;

	// switch off the offset
	switch (offset)
	{
		case 0: // register 0 must return 0x13 in order to pass the self test
			result = 0x13;
			break;

		case 1: // register 1 contains the status

			// loopback case: data always ready, and always ok to send
			if (m_uart[1] == 0x66)
				result |= 5;

			// non-loopback case: bit 0 means data ready, bit 2 means ok to send
			else
			{
				int const temp = m_dcs->control_r();
				result |= (temp & 0x800) >> 9;
				result |= (~temp & 0x400) >> 10;
				machine().scheduler().synchronize();
			}
			break;

		case 3: // register 3 contains the data read

			// loopback case: feed back last data wrtten
			if (m_uart[1] == 0x66)
				result = m_uart[3];

			// non-loopback case: read from the DCS system
			else
			{
				if (!machine().side_effects_disabled())
					LOGMASKED(LOG_SOUND, "%08X:Sound read\n", m_maincpu->pc());

				result = m_dcs->data_r();
			}
			break;

		case 5: // register 5 seems to be like 3, but with in/out swapped

			// loopback case: data always ready, and always ok to send
			if (m_uart[1] == 0x66)
				result |= 5;

			// non-loopback case: bit 0 means data ready, bit 2 means ok to send
			else
			{
				int const temp = m_dcs->control_r();
				result |= (temp & 0x800) >> 11;
				result |= (~temp & 0x400) >> 8;
				machine().scheduler().synchronize();
			}
			break;

		default: // everyone else reads themselves
			result = m_uart[offset];
			break;
	}

	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_UART, "%s: UART R @ %X = %02X\n", machine().describe_context(), offset, result);
	return result;
}


void midxunit_state::uart_w(offs_t offset, uint8_t data)
{
	// switch off the offset
	switch (offset)
	{
		case 3: // register 3 contains the data to be sent

			// loopback case: don't feed through
			if (m_uart[1] == 0x66)
				m_uart[3] = data;

			// non-loopback case: send to the DCS system
			else
				m_dcs->data_w(data);
			break;

		case 5: // register 5 write seems to reset things
			m_dcs->data_r();
			break;

		default: // everyone else just stores themselves
			m_uart[offset] = data;
			break;
	}

	LOGMASKED(LOG_UART, "%s: UART W @ %X = %02X\n", machine().describe_context(), offset, data);
}



/*************************************
 *
 *  X-unit init (DCS)
 *
 *  music: ADSP2101
 *
 *************************************/

/********************** Revolution X **********************/

/*************************************
 *
 *  Machine init
 *
 *************************************/

void midxunit_state::machine_start()
{
	m_gun_recoil.resolve();
	m_gun_led.resolve();

	m_nvram->set_base(m_nvram_data.target(), 0x2000);

	save_item(NAME(m_iodata));
	save_item(NAME(m_uart));
	save_item(NAME(m_adc_int));

	save_item(NAME(m_pic_command));
	save_item(NAME(m_pic_data));
	save_item(NAME(m_pic_clk));
	save_item(NAME(m_pic_status));
}

void midxunit_state::machine_reset()
{
	// reset sound
	m_dcs->reset_w(0);
	m_dcs->reset_w(1);

	m_pic_command = 0;
	m_pic_data = 0;
	m_pic_clk = 0;
	m_pic_status = 0;

	m_dcs->set_io_callbacks(write_line_delegate(*this, FUNC(midxunit_state::dcs_output_full)), write_line_delegate(*this));
}



/*************************************
 *
 *  Security chip I/O
 *
 *************************************/

uint32_t midxunit_state::security_r()
{
	return m_pic_data;
}

void midxunit_state::security_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_pic_command = data & 0x0f;
}


void midxunit_state::security_clock_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_pic_clk = BIT(data, 1);
}



/*************************************
 *
 *  DMA registers
 *
 *************************************/

uint32_t midxunit_state::dma_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t result = 0;

	result |= m_video->dma_r(offset * 2);
	if (ACCESSING_BITS_0_15)
		result |= uint32_t(m_video->dma_r(offset * 2 + 1)) << 16;

	return result;
}


void midxunit_state::dma_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	m_video->dma_w(offset * 2, data & 0xffff);
	if (ACCESSING_BITS_0_15)
		m_video->dma_w(offset * 2 + 1, data >> 16);
}


/*************************************
 *
 *  Memory maps
 *
 *************************************/

void midxunit_state::main_map(address_map &map)
{
	map(0x00000000, 0x003fffff).rw(m_video, FUNC(midxunit_video_device::midtunit_vram_data_r), FUNC(midxunit_video_device::midtunit_vram_data_w));
	map(0x00800000, 0x00bfffff).rw(m_video, FUNC(midxunit_video_device::midtunit_vram_color_r), FUNC(midxunit_video_device::midtunit_vram_color_w));
	map(0x20000000, 0x20ffffff).ram();
	map(0x40800000, 0x4fffffff).w(FUNC(midxunit_state::unknown_w));
	map(0x60400000, 0x6040001f).rw(FUNC(midxunit_state::status_r), FUNC(midxunit_state::security_clock_w));
	map(0x60c00000, 0x60c0001f).portr("IN0");
	map(0x60c00020, 0x60c0003f).portr("IN1");
	map(0x60c00040, 0x60c0005f).portr("IN2");
	map(0x60c00060, 0x60c0007f).portr("DSW");
	map(0x60c00080, 0x60c000df).w(FUNC(midxunit_state::io_w));
	map(0x60c000e0, 0x60c000ff).rw(FUNC(midxunit_state::security_r), FUNC(midxunit_state::security_w));
	map(0x80800000, 0x8080001f).rw("adc", FUNC(adc0848_device::read), FUNC(adc0848_device::write)).umask32(0x000000ff);
	map(0x80c00000, 0x80c000ff).rw(FUNC(midxunit_state::uart_r), FUNC(midxunit_state::uart_w)).umask32(0x000000ff);
	map(0xa0440000, 0xa047ffff).rw(FUNC(midxunit_state::cmos_r), FUNC(midxunit_state::cmos_w)).umask32(0x000000ff);
	map(0xa0800000, 0xa08fffff).rw(m_video, FUNC(midxunit_video_device::midxunit_paletteram_r), FUNC(midxunit_video_device::midxunit_paletteram_w)).share("palette");
	map(0xc0800000, 0xc08000ff).mirror(0x00400000).rw(FUNC(midxunit_state::dma_r), FUNC(midxunit_state::dma_w));
	map(0xf8000000, 0xfeffffff).r(m_video, FUNC(midxunit_video_device::midwunit_gfxrom_r));
	map(0xff000000, 0xffffffff).rom().region("maincpu", 0);
}



/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( revx )
	PORT_START("IN0")
	PORT_BIT( 0x0000000f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x000000c0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000f00, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0xffffc000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x0000000f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0xffffffc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_TILT ) // Slam Switch
	PORT_SERVICE_NO_TOGGLE(0x00000010, IP_ACTIVE_LOW)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_DOOR ) // coin door
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_BILL1 ) // bill validator
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Flip_Screen ))
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0001, DEF_STR( On ))
	PORT_DIPNAME( 0x0002, 0x0000, "Dipswitch Coinage" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0002, DEF_STR( On ))
	PORT_DIPNAME( 0x001c, 0x001c, DEF_STR( Coinage ))
	PORT_DIPSETTING(      0x001c, "1" )
	PORT_DIPSETTING(      0x0018, "2" )
	PORT_DIPSETTING(      0x0014, "3" )
	PORT_DIPSETTING(      0x000c, "ECA" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ))
	PORT_DIPNAME( 0x00e0, 0x0060, "Credits" )
	PORT_DIPSETTING(      0x0020, "3 Start/1 Continue" )
	PORT_DIPSETTING(      0x00e0, "2 Start/2 Continue" )
	PORT_DIPSETTING(      0x00a0, "2 Start/1 Continue" )
	PORT_DIPSETTING(      0x0000, "1 Start/4 Continue" )
	PORT_DIPSETTING(      0x0040, "1 Start/3 Continue" )
	PORT_DIPSETTING(      0x0060, "1 Start/1 Continue" )
	PORT_DIPNAME( 0x0300, 0x0300, "Country" )
	PORT_DIPSETTING(      0x0300, DEF_STR( USA ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( French ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( German ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Unused ))
	PORT_DIPNAME( 0x0400, 0x0400, "Bill Validator" )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0800, 0x0000, "Two Counters" )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Players ) )
	PORT_DIPSETTING(      0x1000, "3 Players" )
	PORT_DIPSETTING(      0x0000, "2 Players" )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Cabinet ))
	PORT_DIPSETTING(      0x2000, "Rev X" )
	PORT_DIPSETTING(      0x0000, "Terminator 2" )
	PORT_DIPNAME( 0x4000, 0x4000, "Video Freeze" )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x8000, 0x8000, "Test Switch" )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("AN0")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_CROSSHAIR(X, -1.0, 0.0, 0) PORT_SENSITIVITY(20) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("AN1")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(20) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("AN2")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_CROSSHAIR(X, -1.0, 0.0, 0) PORT_SENSITIVITY(20) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(2)

	PORT_START("AN3")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(20) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("AN4")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_CROSSHAIR(X, -1.0, 0.0, 0) PORT_SENSITIVITY(20) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(3)

	PORT_START("AN5")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(20) PORT_KEYDELTA(10) PORT_PLAYER(3)
INPUT_PORTS_END


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void midxunit_state::midxunit(machine_config &config)
{
	constexpr XTAL PIXEL_CLOCK = 8_MHz_XTAL;

	MIDXUNIT_VIDEO(config, m_video, m_palette);
	m_video->dma_irq_cb().set_inputline(m_maincpu, 0);

	// basic machine hardware
	TMS34020(config, m_maincpu, 40_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &midxunit_state::main_map);
	m_maincpu->set_halt_on_reset(false);        // halt on reset
	m_maincpu->set_pixel_clock(PIXEL_CLOCK);    // pixel clock
	m_maincpu->set_pixels_per_clock(1);         // pixels per clock
	m_maincpu->set_scanline_ind16_callback(m_video, FUNC(midxunit_video_device::scanline_update));  // scanline updater (indexed16)
	m_maincpu->set_shiftreg_in_callback(m_video, FUNC(midxunit_video_device::to_shiftreg));         // write to shiftreg function
	m_maincpu->set_shiftreg_out_callback(m_video, FUNC(midtunit_video_device::from_shiftreg));      // read from shiftreg function
	m_maincpu->set_screen("screen");

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// video hardware
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 32768);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(PIXEL_CLOCK, 506, 101, 501, 289, 20, 274);
	screen.set_screen_update(m_maincpu, FUNC(tms34010_device::tms340x0_ind16));
	screen.set_palette(m_palette);

	PIC16C57(config, m_pic, 625000); // need to be verified
	m_pic->read_a().set([this]() { return m_pic_command; });
	m_pic->write_b().set([this](u8 data) { m_pic_data = data; });
	m_pic->read_c().set([this]() { return m_pic_clk ^ 1; });
	m_pic->write_c().set([this](u8 data) { m_pic_status = BIT(data, 1); });
	// there also should be PIC16 reset line control, unknown at the moment

	adc0848_device &adc(ADC0848(config, "adc"));
	adc.intr_callback().set(FUNC(midxunit_state::adc_int_w)); // ADC INT passed through PLSI1032
	adc.ch1_callback().set_ioport("AN0");
	adc.ch2_callback().set_ioport("AN1");
	adc.ch3_callback().set_ioport("AN2");
	adc.ch4_callback().set_ioport("AN3");
	adc.ch5_callback().set_ioport("AN4");
	adc.ch6_callback().set_ioport("AN5");

	// sound hardware
	SPEAKER(config, "mono").front_center();

	DCS_AUDIO_2K_UART(config, m_dcs, 0);
	m_dcs->set_maincpu_tag(m_maincpu);
	m_dcs->add_route(0, "mono", 1.0);
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( revx )
	ROM_REGION32_LE( 0x200000, "maincpu", 0 ) // 34020 code
	ROM_LOAD32_BYTE( "revx_l2.0_u51.u51", 0x00000, 0x80000, CRC(2c996003) SHA1(54819bdf1f9a77f43281effdbcd2dd69a3787e88) )
	ROM_LOAD32_BYTE( "revx_l2.0_u52.u52", 0x00001, 0x80000, CRC(f1d4d6fb) SHA1(5927126eb0e54e99a2cfcccbe81739e3b165ec69) )
	ROM_LOAD32_BYTE( "revx_l2.0_u53.u53", 0x00002, 0x80000, CRC(59ca7084) SHA1(802d7cb8e15476133394c871416e0a5ffd9c00ee) )
	ROM_LOAD32_BYTE( "revx_l2.0_u54.u54", 0x00003, 0x80000, CRC(4f577dd9) SHA1(0a2fa80a1473f16b5a2227e596f8dc1d45ef7d01) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  // sound data
	//ROM_LOAD16_BYTE( "p5_revolution_x_sound_rom_u2.u2", 0x000000, 0x80000, CRC(4ed9e803) SHA1(ba50f1beb9f2a2cf5110897209b5e9a2951ff165) ) // the prototype "Release 1" ROM was on the board but the game rejects it with a startup error (not upgraded properly?)
	ROM_LOAD16_BYTE( "l1_revolution_x_sound_rom_u2.u2", 0x000000, 0x80000, CRC(d2ed9f5e) SHA1(415ce5e41a560d135ea41c7924219fdeda504237) ) // shows as "Sound Software Version - Release 2"
	ROM_LOAD16_BYTE( "l1_revolution_x_sound_rom_u3.u3", 0x200000, 0x80000, CRC(af8f253b) SHA1(25a0000cab177378070f7a6e3c7378fe87fad63e) )
	ROM_LOAD16_BYTE( "l1_revolution_x_sound_rom_u4.u4", 0x400000, 0x80000, CRC(3ccce59c) SHA1(e81a31d64c64e7b1d25f178c53da3d68453c203c) )
	ROM_LOAD16_BYTE( "l1_revolution_x_sound_rom_u5.u5", 0x600000, 0x80000, CRC(a0438006) SHA1(560d216d21cb8073dbee0fd20ebe589932a9144e) )
	ROM_LOAD16_BYTE( "l1_revolution_x_sound_rom_u6.u6", 0x800000, 0x80000, CRC(b7b34f60) SHA1(3b9682c6a00fa3bdb47e69d8e8ceccc244ee55b5) )
	ROM_LOAD16_BYTE( "l1_revolution_x_sound_rom_u7.u7", 0xa00000, 0x80000, CRC(6795fd88) SHA1(7c3790730a8b99b63112c851318b1c7e4989e5e0) )
	ROM_LOAD16_BYTE( "l1_revolution_x_sound_rom_u8.u8", 0xc00000, 0x80000, CRC(793a7eb5) SHA1(4b1f81b68f95cedf1b356ef362d1eb37acc74b16) )
	ROM_LOAD16_BYTE( "l1_revolution_x_sound_rom_u9.u9", 0xe00000, 0x80000, CRC(14ddbea1) SHA1(8dba9dc5529ea77c4312ea61f825bf9062ffc6c3) )

	ROM_REGION( 0x2000, "pic", 0 )
	ROM_LOAD( "419_revolution-x_u444.u444", 0x0000, 0x2000, CRC(7df57330) SHA1(fa6733972f45d90563c184b6735da7a40cee1bf2) )

	ROM_REGION( 0x1000000, "video", 0 )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u120.u120", 0x0000000, 0x80000, CRC(523af1f0) SHA1(a67c0fd757e860fc1c1236945952a295b4d5df5a) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u121.u121", 0x0000001, 0x80000, CRC(78201d93) SHA1(fb0b8f887eec433f7624f387d7fb6f633ea30d7c) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u122.u122", 0x0000002, 0x80000, CRC(2cf36144) SHA1(22ed0eefa2c7c836811fac5f717c3f38254eabc2) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u123.u123", 0x0000003, 0x80000, CRC(6912e1fb) SHA1(416f0de711d80e9182ede524c568c5095b1bec61) )

	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u110.u110", 0x0200000, 0x80000, CRC(e3f7f0af) SHA1(5877d9f488b0f4362a9482007c3ff7f4589a036f) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u111.u111", 0x0200001, 0x80000, CRC(49fe1a69) SHA1(9ae54b461f0524c034fbcb6fcd3fd5ccb5d7265a) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u112.u112", 0x0200002, 0x80000, CRC(7e3ba175) SHA1(dd2fe90988b544f67dbe6151282fd80d49631388) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u113.u113", 0x0200003, 0x80000, CRC(c0817583) SHA1(2f866e5888e212b245984344950d0e1fb8957a73) )

	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u101.u101", 0x0400000, 0x80000, CRC(5a08272a) SHA1(17da3c9d71114f5fdbf50281a942be3da3b6f564) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u102.u102", 0x0400001, 0x80000, CRC(11d567d2) SHA1(7ebe6fd39a0335e1fdda150d2dc86c3eaab17b2e) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u103.u103", 0x0400002, 0x80000, CRC(d338e63b) SHA1(0a038217542667b3a01ecbcad824ee18c084f293) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u104.u104", 0x0400003, 0x80000, CRC(f7b701ee) SHA1(0fc5886e5857326bee7272d5d482a878cbcea83c) )

	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u91.u91",   0x0600000, 0x80000, CRC(52a63713) SHA1(dcc0ff3596bd5d273a8d4fd33b0b9b9d588d8354) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u92.u92",   0x0600001, 0x80000, CRC(fae3621b) SHA1(715d41ea789c0c724baa5bd90f6f0f06b9cb1c64) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u93.u93",   0x0600002, 0x80000, CRC(7065cf95) SHA1(6c5888da099e51c4b1c592721c5027c899cf52e3) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u94.u94",   0x0600003, 0x80000, CRC(600d5b98) SHA1(6aef98c91f87390c0759fe71a272a3ccadd71066) )

	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u81.u81",   0x0800000, 0x80000, CRC(729eacb1) SHA1(d130162ae22b99c84abfbe014c4e23e20afb757f) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u82.u82",   0x0800001, 0x80000, CRC(19acb904) SHA1(516059b516bc5b1669c9eb085e0cdcdee520dff0) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u83.u83",   0x0800002, 0x80000, CRC(0e223456) SHA1(1eedbd667f4a214533d1c22ca5312ecf2d4a3ab4) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u84.u84",   0x0800003, 0x80000, CRC(d3de0192) SHA1(2d22c5bac07a7411f326691167c7c70eba4b371f) )

	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u71.u71",   0x0a00000, 0x80000, CRC(2b29fddb) SHA1(57b71e5c18b56bf58216e690fdefa6d30d88d34a) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u72.u72",   0x0a00001, 0x80000, CRC(2680281b) SHA1(d1ae0701d20166a00d8733d9d12246c140a5fb96) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u73.u73",   0x0a00002, 0x80000, CRC(420bde4d) SHA1(0f010cdeddb59631a5420dddfc142c50c2a1e65a) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u74.u74",   0x0a00003, 0x80000, CRC(26627410) SHA1(a612121554549afff5c8e8c54774ca7b0220eda8) )

	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u63.u63",   0x0c00000, 0x80000, CRC(3066e3f3) SHA1(25548923db111bd6c6cff44bfb63cb9eb2ef0b53) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u64.u64",   0x0c00001, 0x80000, CRC(c33f5309) SHA1(6bb333f563ea66c4c862ffd5fb91fb5e1b919fe8) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u65.u65",   0x0c00002, 0x80000, CRC(6eee3e71) SHA1(0ef22732e0e2bb5207559decd43f90d1e338ad7b) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u66.u66",   0x0c00003, 0x80000, CRC(b43d6fff) SHA1(87584e7aeea9d52a43023d40c359591ff6342e84) )

	//TODO: Fix the need of reloading program ROMs here, there are no duplicate ROMs on the real PCB
	ROM_LOAD32_BYTE( "revx_l2.0_u51.u51",                  0x0e00000, 0x80000, CRC(2c996003) SHA1(54819bdf1f9a77f43281effdbcd2dd69a3787e88) )
	ROM_LOAD32_BYTE( "revx_l2.0_u52.u52",                  0x0e00001, 0x80000, CRC(f1d4d6fb) SHA1(5927126eb0e54e99a2cfcccbe81739e3b165ec69) )
	ROM_LOAD32_BYTE( "revx_l2.0_u53.u53",                  0x0e00002, 0x80000, CRC(59ca7084) SHA1(802d7cb8e15476133394c871416e0a5ffd9c00ee) )
	ROM_LOAD32_BYTE( "revx_l2.0_u54.u54",                  0x0e00003, 0x80000, CRC(4f577dd9) SHA1(0a2fa80a1473f16b5a2227e596f8dc1d45ef7d01) )

	ROM_REGION( 0x600, "plds", 0 )
	ROM_LOAD( "a-17722.u1",   0x000, 0x117, CRC(054de7a3) SHA1(bb7abaec50ed704c03b44d5d54296898f7c80d38) )
	ROM_LOAD( "a-17721.u955", 0x200, 0x117, CRC(033fe902) SHA1(6efb4e519ed3c9d49fff046a679762b506b3a75b) )
	ROM_LOAD( "snd-gal16v8a.u17", 0x400, 0x117, NO_DUMP ) // Protected
ROM_END

ROM_START( revx1 )
	ROM_REGION32_LE( 0x200000, "maincpu", 0 )   // 34020 code
	ROM_LOAD32_BYTE( "l1_revolution_x_game_rom_u51.u51",  0x00000, 0x80000, CRC(9960ac7c) SHA1(441322f061d627ca7573f612f370a85794681d0f) ) // labels needs to be verified, so far was observed only reprogrammed P5 ROMs
	ROM_LOAD32_BYTE( "l1_revolution_x_game_rom_u52.u52",  0x00001, 0x80000, CRC(fbf55510) SHA1(8a5b0004ed09391fe37f0f501b979903d6ae4868) )
	ROM_LOAD32_BYTE( "l1_revolution_x_game_rom_u53.u53",  0x00002, 0x80000, CRC(a045b265) SHA1(b294d3a56e41f5ec4ab9bbcc0088833b1cab1879) )
	ROM_LOAD32_BYTE( "l1_revolution_x_game_rom_u54.u54",  0x00003, 0x80000, CRC(24471269) SHA1(262345bd147402100785459af422dafd1c562787) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  // sound data
	ROM_LOAD16_BYTE( "l1_revolution_x_sound_rom_u2.u2", 0x000000, 0x80000, CRC(d2ed9f5e) SHA1(415ce5e41a560d135ea41c7924219fdeda504237) ) // shows as "Sound Software Version - Release 2"
	ROM_LOAD16_BYTE( "l1_revolution_x_sound_rom_u3.u3", 0x200000, 0x80000, CRC(af8f253b) SHA1(25a0000cab177378070f7a6e3c7378fe87fad63e) )
	ROM_LOAD16_BYTE( "l1_revolution_x_sound_rom_u4.u4", 0x400000, 0x80000, CRC(3ccce59c) SHA1(e81a31d64c64e7b1d25f178c53da3d68453c203c) )
	ROM_LOAD16_BYTE( "l1_revolution_x_sound_rom_u5.u5", 0x600000, 0x80000, CRC(a0438006) SHA1(560d216d21cb8073dbee0fd20ebe589932a9144e) )
	ROM_LOAD16_BYTE( "l1_revolution_x_sound_rom_u6.u6", 0x800000, 0x80000, CRC(b7b34f60) SHA1(3b9682c6a00fa3bdb47e69d8e8ceccc244ee55b5) )
	ROM_LOAD16_BYTE( "l1_revolution_x_sound_rom_u7.u7", 0xa00000, 0x80000, CRC(6795fd88) SHA1(7c3790730a8b99b63112c851318b1c7e4989e5e0) )
	ROM_LOAD16_BYTE( "l1_revolution_x_sound_rom_u8.u8", 0xc00000, 0x80000, CRC(793a7eb5) SHA1(4b1f81b68f95cedf1b356ef362d1eb37acc74b16) )
	ROM_LOAD16_BYTE( "l1_revolution_x_sound_rom_u9.u9", 0xe00000, 0x80000, CRC(14ddbea1) SHA1(8dba9dc5529ea77c4312ea61f825bf9062ffc6c3) )

	ROM_REGION( 0x2000, "pic", 0 )
	ROM_LOAD( "419_revolution-x_u444.u444", 0x0000, 0x2000, CRC(7df57330) SHA1(fa6733972f45d90563c184b6735da7a40cee1bf2) )

	ROM_REGION( 0x1000000, "video", 0 )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u120.u120", 0x0000000, 0x80000, CRC(523af1f0) SHA1(a67c0fd757e860fc1c1236945952a295b4d5df5a) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u121.u121", 0x0000001, 0x80000, CRC(78201d93) SHA1(fb0b8f887eec433f7624f387d7fb6f633ea30d7c) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u122.u122", 0x0000002, 0x80000, CRC(2cf36144) SHA1(22ed0eefa2c7c836811fac5f717c3f38254eabc2) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u123.u123", 0x0000003, 0x80000, CRC(6912e1fb) SHA1(416f0de711d80e9182ede524c568c5095b1bec61) )

	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u110.u110", 0x0200000, 0x80000, CRC(e3f7f0af) SHA1(5877d9f488b0f4362a9482007c3ff7f4589a036f) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u111.u111", 0x0200001, 0x80000, CRC(49fe1a69) SHA1(9ae54b461f0524c034fbcb6fcd3fd5ccb5d7265a) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u112.u112", 0x0200002, 0x80000, CRC(7e3ba175) SHA1(dd2fe90988b544f67dbe6151282fd80d49631388) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u113.u113", 0x0200003, 0x80000, CRC(c0817583) SHA1(2f866e5888e212b245984344950d0e1fb8957a73) )

	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u101.u101", 0x0400000, 0x80000, CRC(5a08272a) SHA1(17da3c9d71114f5fdbf50281a942be3da3b6f564) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u102.u102", 0x0400001, 0x80000, CRC(11d567d2) SHA1(7ebe6fd39a0335e1fdda150d2dc86c3eaab17b2e) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u103.u103", 0x0400002, 0x80000, CRC(d338e63b) SHA1(0a038217542667b3a01ecbcad824ee18c084f293) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u104.u104", 0x0400003, 0x80000, CRC(f7b701ee) SHA1(0fc5886e5857326bee7272d5d482a878cbcea83c) )

	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u91.u91",  0x0600000, 0x80000, CRC(52a63713) SHA1(dcc0ff3596bd5d273a8d4fd33b0b9b9d588d8354) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u92.u92",  0x0600001, 0x80000, CRC(fae3621b) SHA1(715d41ea789c0c724baa5bd90f6f0f06b9cb1c64) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u93.u93",  0x0600002, 0x80000, CRC(7065cf95) SHA1(6c5888da099e51c4b1c592721c5027c899cf52e3) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u94.u94",  0x0600003, 0x80000, CRC(600d5b98) SHA1(6aef98c91f87390c0759fe71a272a3ccadd71066) )

	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u81.u81",  0x0800000, 0x80000, CRC(729eacb1) SHA1(d130162ae22b99c84abfbe014c4e23e20afb757f) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u82.u82",  0x0800001, 0x80000, CRC(19acb904) SHA1(516059b516bc5b1669c9eb085e0cdcdee520dff0) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u83.u83",  0x0800002, 0x80000, CRC(0e223456) SHA1(1eedbd667f4a214533d1c22ca5312ecf2d4a3ab4) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u84.u84",  0x0800003, 0x80000, CRC(d3de0192) SHA1(2d22c5bac07a7411f326691167c7c70eba4b371f) )

	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u71.u71",  0x0a00000, 0x80000, CRC(2b29fddb) SHA1(57b71e5c18b56bf58216e690fdefa6d30d88d34a) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u72.u72",  0x0a00001, 0x80000, CRC(2680281b) SHA1(d1ae0701d20166a00d8733d9d12246c140a5fb96) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u73.u73",  0x0a00002, 0x80000, CRC(420bde4d) SHA1(0f010cdeddb59631a5420dddfc142c50c2a1e65a) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u74.u74",  0x0a00003, 0x80000, CRC(26627410) SHA1(a612121554549afff5c8e8c54774ca7b0220eda8) )

	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u63.u63",  0x0c00000, 0x80000, CRC(3066e3f3) SHA1(25548923db111bd6c6cff44bfb63cb9eb2ef0b53) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u64.u64",  0x0c00001, 0x80000, CRC(c33f5309) SHA1(6bb333f563ea66c4c862ffd5fb91fb5e1b919fe8) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u65.u65",  0x0c00002, 0x80000, CRC(6eee3e71) SHA1(0ef22732e0e2bb5207559decd43f90d1e338ad7b) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u66.u66",  0x0c00003, 0x80000, CRC(b43d6fff) SHA1(87584e7aeea9d52a43023d40c359591ff6342e84) )

	//TODO: Fix the need of reloading program ROMs here, there are no duplicate ROMs on the real PCB
	ROM_LOAD32_BYTE( "l1_revolution_x_game_rom_u51.u51",  0x0e00000, 0x80000, CRC(9960ac7c) SHA1(441322f061d627ca7573f612f370a85794681d0f) ) // labels needs to be verified, so far was observed only reprogrammed P5 ROMs
	ROM_LOAD32_BYTE( "l1_revolution_x_game_rom_u52.u52",  0x0e00001, 0x80000, CRC(fbf55510) SHA1(8a5b0004ed09391fe37f0f501b979903d6ae4868) )
	ROM_LOAD32_BYTE( "l1_revolution_x_game_rom_u53.u53",  0x0e00002, 0x80000, CRC(a045b265) SHA1(b294d3a56e41f5ec4ab9bbcc0088833b1cab1879) )
	ROM_LOAD32_BYTE( "l1_revolution_x_game_rom_u54.u54",  0x0e00003, 0x80000, CRC(24471269) SHA1(262345bd147402100785459af422dafd1c562787) )

	ROM_REGION( 0x600, "plds", 0 )
	ROM_LOAD( "a-17722.u1",   0x000, 0x117, CRC(054de7a3) SHA1(bb7abaec50ed704c03b44d5d54296898f7c80d38) )
	ROM_LOAD( "a-17721.u955", 0x200, 0x117, CRC(033fe902) SHA1(6efb4e519ed3c9d49fff046a679762b506b3a75b) )
	ROM_LOAD( "snd-gal16v8a.u17", 0x400, 0x117, NO_DUMP ) // Protected
ROM_END

ROM_START( revxp5 )
	ROM_REGION32_LE( 0x200000, "maincpu", 0 )   // 34020 code
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u51.u51",  0x00000, 0x80000, CRC(f3877eee) SHA1(7a4fdce36edddd35308c107c992ce626a2c9eb8c) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u52.u52",  0x00001, 0x80000, CRC(199a54d8) SHA1(45319437e11176d4926c00c95c372098203a32a3) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u53.u53",  0x00002, 0x80000, CRC(fcfcf72a) SHA1(b471afb416e3d348b046b0b40f497d27b0afa470) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u54.u54",  0x00003, 0x80000, CRC(fd684c31) SHA1(db3453792e4d9fc375297d030f0b3f9cc3cad925) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  // sound data
	ROM_LOAD16_BYTE( "p5_revolution_x_sound_rom_u2.u2", 0x000000, 0x80000, CRC(4ed9e803) SHA1(ba50f1beb9f2a2cf5110897209b5e9a2951ff165) ) // shows as "Sound Software Version - Release 1"
	ROM_LOAD16_BYTE( "l1_revolution_x_sound_rom_u3.u3", 0x200000, 0x80000, CRC(af8f253b) SHA1(25a0000cab177378070f7a6e3c7378fe87fad63e) )
	ROM_LOAD16_BYTE( "l1_revolution_x_sound_rom_u4.u4", 0x400000, 0x80000, CRC(3ccce59c) SHA1(e81a31d64c64e7b1d25f178c53da3d68453c203c) )
	ROM_LOAD16_BYTE( "l1_revolution_x_sound_rom_u5.u5", 0x600000, 0x80000, CRC(a0438006) SHA1(560d216d21cb8073dbee0fd20ebe589932a9144e) )
	ROM_LOAD16_BYTE( "l1_revolution_x_sound_rom_u6.u6", 0x800000, 0x80000, CRC(b7b34f60) SHA1(3b9682c6a00fa3bdb47e69d8e8ceccc244ee55b5) )
	ROM_LOAD16_BYTE( "l1_revolution_x_sound_rom_u7.u7", 0xa00000, 0x80000, CRC(6795fd88) SHA1(7c3790730a8b99b63112c851318b1c7e4989e5e0) )
	ROM_LOAD16_BYTE( "l1_revolution_x_sound_rom_u8.u8", 0xc00000, 0x80000, CRC(793a7eb5) SHA1(4b1f81b68f95cedf1b356ef362d1eb37acc74b16) )
	ROM_LOAD16_BYTE( "l1_revolution_x_sound_rom_u9.u9", 0xe00000, 0x80000, CRC(14ddbea1) SHA1(8dba9dc5529ea77c4312ea61f825bf9062ffc6c3) )

	ROM_REGION( 0x2000, "pic", 0 )
	ROM_LOAD( "419_revolution-x_u444.u444", 0x0000, 0x2000, CRC(7df57330) SHA1(fa6733972f45d90563c184b6735da7a40cee1bf2) )

	ROM_REGION( 0x1000000, "video", 0 )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u120.u120", 0x0000000, 0x80000, CRC(523af1f0) SHA1(a67c0fd757e860fc1c1236945952a295b4d5df5a) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u121.u121", 0x0000001, 0x80000, CRC(78201d93) SHA1(fb0b8f887eec433f7624f387d7fb6f633ea30d7c) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u122.u122", 0x0000002, 0x80000, CRC(2cf36144) SHA1(22ed0eefa2c7c836811fac5f717c3f38254eabc2) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u123.u123", 0x0000003, 0x80000, CRC(6912e1fb) SHA1(416f0de711d80e9182ede524c568c5095b1bec61) )

	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u110.u110", 0x0200000, 0x80000, CRC(e3f7f0af) SHA1(5877d9f488b0f4362a9482007c3ff7f4589a036f) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u111.u111", 0x0200001, 0x80000, CRC(49fe1a69) SHA1(9ae54b461f0524c034fbcb6fcd3fd5ccb5d7265a) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u112.u112", 0x0200002, 0x80000, CRC(7e3ba175) SHA1(dd2fe90988b544f67dbe6151282fd80d49631388) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u113.u113", 0x0200003, 0x80000, CRC(c0817583) SHA1(2f866e5888e212b245984344950d0e1fb8957a73) )

	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u101.u101", 0x0400000, 0x80000, CRC(5a08272a) SHA1(17da3c9d71114f5fdbf50281a942be3da3b6f564) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u102.u102", 0x0400001, 0x80000, CRC(11d567d2) SHA1(7ebe6fd39a0335e1fdda150d2dc86c3eaab17b2e) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u103.u103", 0x0400002, 0x80000, CRC(d338e63b) SHA1(0a038217542667b3a01ecbcad824ee18c084f293) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u104.u104", 0x0400003, 0x80000, CRC(f7b701ee) SHA1(0fc5886e5857326bee7272d5d482a878cbcea83c) )

	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u91.u91",  0x0600000, 0x80000, CRC(52a63713) SHA1(dcc0ff3596bd5d273a8d4fd33b0b9b9d588d8354) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u92.u92",  0x0600001, 0x80000, CRC(fae3621b) SHA1(715d41ea789c0c724baa5bd90f6f0f06b9cb1c64) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u93.u93",  0x0600002, 0x80000, CRC(7065cf95) SHA1(6c5888da099e51c4b1c592721c5027c899cf52e3) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u94.u94",  0x0600003, 0x80000, CRC(600d5b98) SHA1(6aef98c91f87390c0759fe71a272a3ccadd71066) )

	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u81.u81",  0x0800000, 0x80000, CRC(729eacb1) SHA1(d130162ae22b99c84abfbe014c4e23e20afb757f) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u82.u82",  0x0800001, 0x80000, CRC(19acb904) SHA1(516059b516bc5b1669c9eb085e0cdcdee520dff0) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u83.u83",  0x0800002, 0x80000, CRC(0e223456) SHA1(1eedbd667f4a214533d1c22ca5312ecf2d4a3ab4) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u84.u84",  0x0800003, 0x80000, CRC(d3de0192) SHA1(2d22c5bac07a7411f326691167c7c70eba4b371f) )

	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u71.u71",  0x0a00000, 0x80000, CRC(2b29fddb) SHA1(57b71e5c18b56bf58216e690fdefa6d30d88d34a) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u72.u72",  0x0a00001, 0x80000, CRC(2680281b) SHA1(d1ae0701d20166a00d8733d9d12246c140a5fb96) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u73.u73",  0x0a00002, 0x80000, CRC(420bde4d) SHA1(0f010cdeddb59631a5420dddfc142c50c2a1e65a) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u74.u74",  0x0a00003, 0x80000, CRC(26627410) SHA1(a612121554549afff5c8e8c54774ca7b0220eda8) )

	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u63.u63",  0x0c00000, 0x80000, CRC(3066e3f3) SHA1(25548923db111bd6c6cff44bfb63cb9eb2ef0b53) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u64.u64",  0x0c00001, 0x80000, CRC(c33f5309) SHA1(6bb333f563ea66c4c862ffd5fb91fb5e1b919fe8) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u65.u65",  0x0c00002, 0x80000, CRC(6eee3e71) SHA1(0ef22732e0e2bb5207559decd43f90d1e338ad7b) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u66.u66",  0x0c00003, 0x80000, CRC(b43d6fff) SHA1(87584e7aeea9d52a43023d40c359591ff6342e84) )

	//TODO: Fix the need of reloading program ROMs here, there are no duplicate ROMs on the real PCB
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u51.u51",  0xe00000, 0x80000, CRC(f3877eee) SHA1(7a4fdce36edddd35308c107c992ce626a2c9eb8c) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u52.u52",  0xe00001, 0x80000, CRC(199a54d8) SHA1(45319437e11176d4926c00c95c372098203a32a3) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u53.u53",  0xe00002, 0x80000, CRC(fcfcf72a) SHA1(b471afb416e3d348b046b0b40f497d27b0afa470) )
	ROM_LOAD32_BYTE( "p5_revolution_x_game_rom_u54.u54",  0xe00003, 0x80000, CRC(fd684c31) SHA1(db3453792e4d9fc375297d030f0b3f9cc3cad925) )

	ROM_REGION( 0x600, "plds", 0 )
	ROM_LOAD( "a-17722.u1",       0x000, 0x117, CRC(054de7a3) SHA1(bb7abaec50ed704c03b44d5d54296898f7c80d38) )
	ROM_LOAD( "a-17721.u955",     0x200, 0x117, CRC(033fe902) SHA1(6efb4e519ed3c9d49fff046a679762b506b3a75b) )
	ROM_LOAD( "snd-gal16v8a.u17", 0x400, 0x117, NO_DUMP ) // Protected
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1994, revx,   0,    midxunit, revx, midxunit_state, empty_init, ROT0, "Midway", "Revolution X (revision 2.0 9/8/94)",             MACHINE_SUPPORTS_SAVE )
GAME( 1994, revx1,  revx, midxunit, revx, midxunit_state, empty_init, ROT0, "Midway", "Revolution X (revision 1.0 6/16/94)",            MACHINE_SUPPORTS_SAVE )
GAME( 1994, revxp5, revx, midxunit, revx, midxunit_state, empty_init, ROT0, "Midway", "Revolution X (prototype, revision 5.0 5/23/94)", MACHINE_SUPPORTS_SAVE )
