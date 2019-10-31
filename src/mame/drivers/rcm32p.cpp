// license:BSD-3-Clause
// copyright-holders:Valley Bell
/*************************************************************************************************

    Roland CM-32P driver

    Driver by Valley Bell

    Thanks to KitsuWhooa for the PCB layout diagram and part list.


Notes:
- When booting, it does a basic check by writing values 0..0xFF to 0x108A.
  It then expects (value & 0x03) to be read back from 0x1080 and (value) to be read back from 0x1081/0x1082.
  If there is any unexpected results, it goes into Test Mode. (jump to 0x46D4)
  Booting normally means it finally ends up at 0x4801.
  The routine for doing the check begins at 0x4686.
- When in test mode, the firmware gets stuck in the loop at 0xBB06, waiting for Interrupt #5 (calls 0x4014/0x4030) to fire.
  It gets there by calling function 0xBBBB, which writes text (address w@0x40, num characters b@0x43) to the LCD screen.
  This can be worked around by setting b@BB2D = 03.

  It then gets stuck at 0x6718, trying to read a "mode" value (0 to 7) from 0x1300.
  The byte at 0x1300 contains the bit mask of the pressed test switches. (see service notes, page 10/11)
  Modes are selected by checking the first unset bit in order 0..7.


PCB Layout
----------

PCM BOARD CM-32P ASSY 79554310 00

|   OUTPUT  | PHONES | INPUT (MT-32) | MIDI IN | MIDI THROUGH |  DC 9V |
|----------------------------------------------------------------------|
|  JK2   JK4   JK3   JKLI    JKRI CN4 JK1I         JK1T          JK5   |
|                                                                      |
|                                                                  SW1 |
|                   ||                  IC2    IC1                     |
|    IC35 IC33 IC34 ||  IC31 IC30                              CN0     |
| CN3               ||<--IC32                                          |
|                                       |----|  |----|    |----|       |
|                                       |    |  |    |    |    |       |
|    IC22  ||  IC24                     |IC10|  |IC9 |    |    |       |
|          ||       IC25      IC4       |    |  |    |    |    |       |
|          ||<--IC23                    |SRAM|  |ROM |    |    |       |
|                                       |----|  |----|    |IC3 |   X1  |
| |----|                                                  |    |       |
| | IC |         |------|                                 |    |       |
| | 17 |         | IC16 |                     IC12        |    |       |
| |----|         |------|                 X2              |----|       |
|                                |------|     IC13                     |
|    IC6  |----||----||----|     | IC15 |                              |
| |-||-|  |    ||    ||    |     |------|     IC14      |------|       |
| | || |  |IC20||IC19||IC18|                            | IC8  |       |
| |-||-|  |    ||    ||    |                            |------|       |
| IC7     |SMPL||SMPL||SMPL|   |------------------|                    |
|         |ROM ||ROM ||ROM |   |        CN5       |                    |
|         |----||----||----|   |                  |       CN1          |
|------------------------------|------------------|--------------------|
                               |                  |

Parts:

(All parts THT unless otherwise noted.)
|-----------------------------------------------------------------------|
| JK1I, JK1T                   | DIN 5                                  |
| JK2, JK4, JKRI, JKLI         | 1/4 Mono Jack                          |
| JK3                          | 1/4 Stereo Jack                        |
| JK5                          | DC Barrel Jack (Centre Negative)       |
| SW1                          | SW PCB DPDT                            |
| IC1                          | 74HC04AP                               |
| IC2                          | TLP552                                 |
| IC3                          | P8098 (CPU)                            |
| IC4                          | M51953AL                               |
| IC6, IC7                     | M5M4464AP-12                           |
| IC8                          | Roland R15239111 / M60012-0141FP (QFP) |
| IC9                          | AM27C512-150DC                         |
| IC10                         | TC5564APL-15                           |
| IC12                         | 74HC00AP                               |
| IC13                         | 74HC02AP                               |
| IC14                         | 74HC32AP                               |
| IC15                         | Roland R15229894 / MB87419 (QFP)       |
| IC16                         | Roland R15229895 / MB87420 (QFP)       |
| IC17                         | Roland P15239126 / 23140-007 (QFP)     |
| IC18                         | Roland R15179970 / MB834000A-20 3B1    |
| IC19                         | Roland R15179971 / MB834000A-20 3B2    |
| IC20                         | Roland R15179972 / HN62304BPE98        |
| IC22                         | PCM56P                                 |
| IC23                         | M5238L                                 |
| IC24                         | HD14052BP                              |
| IC25                         | NJM2082D                               |
| IC30, IC31, IC33, IC34, IC35 | NJM4565DD                              |
| IC32                         | M5207L01                               |
| CN0                          | 7805A                                  |
| CN1                          | LED PCB                                |
| CN3                          | Volume Control PCB                     |
| CN4                          | Unpopulated                            |
| CN5                          | PCM Card Slot                          |
| X1                           | Crystal 12MHz                          |
| X2                           | Crystal 32.768KHz                      |
|-----------------------------------------------------------------------|

*/

#include "emu.h"
#include "cpu/mcs96/i8x9x.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "video/msm6222b.h"
#include "emupal.h"
#include "screen.h"


// unscramble address: ROM dump offset -> proper (descrambled) offset
#define UNSCRAMBLE_ADDRESS(_offset) \
	bitswap<19>(_offset,18,17,15,14,16,12,11, 7, 9,13,10, 8, 3, 2, 1, 6, 4, 5, 0)
// scramble address: proper offset -> ROM dump offset
#define SCRAMBLE_ADDRESS(_offset) \
	bitswap<19>(_offset,18,17,14,16,15, 9,13,12, 8,10, 7,11, 3, 1, 2, 6, 5, 4, 0)

#define UNSCRAMBLE_DATA(_data) \
	bitswap<8>(_data,1,2,7,3,5,0,4,6)


static INPUT_PORTS_START( cm32p )
	PORT_START("A7")
	PORT_BIT(0x03ff, 0x0000, IPT_DIAL) PORT_NAME("Knob") PORT_SENSITIVITY(50) PORT_KEYDELTA(8) PORT_CODE_DEC(KEYCODE_DOWN) PORT_CODE_INC(KEYCODE_UP)

	PORT_START("SW")	// test switches
	//PORT_BIT(0x00, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Check/Tune") PORT_CODE(KEYCODE_0)	// default after booting test mode
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("MSB Adj.") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("THD Check") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("PCM Out: String 1") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("PCM Out: Sax 1") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("PCM + Long Reverb") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("PCM + Short Reverb") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("VCA Down Check") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("VCA Up Check") PORT_CODE(KEYCODE_F)
INPUT_PORTS_END

class cm32p_state : public driver_device
{
public:
	cm32p_state(const machine_config &mconfig, device_type type, const char *tag);

	void cm32p(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<i8x9x_device> cpu;
	required_device<msm6222b_device> lcd;
	required_device<timer_device> midi_timer;
	required_ioport test_sw;

	void mt32_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE16_MEMBER(midi_w);

	DECLARE_READ8_MEMBER(lcd_ctrl_r);
	DECLARE_WRITE8_MEMBER(lcd_ctrl_w);
	DECLARE_WRITE8_MEMBER(lcd_data_w);
	DECLARE_READ16_MEMBER(port0_r);
	DECLARE_READ8_MEMBER(pcmrom_r);
	DECLARE_READ8_MEMBER(snd_io_r);
	DECLARE_WRITE8_MEMBER(snd_io_w);
	DECLARE_READ8_MEMBER(test_sw_r);
	
	TIMER_DEVICE_CALLBACK_MEMBER(midi_timer_cb);
	TIMER_DEVICE_CALLBACK_MEMBER(samples_timer_cb);

	void cm32p_map(address_map &map);

	//uint8_t lcd_data_buffer[256];
	//int lcd_data_buffer_pos;
	uint8_t midi;
	int midi_pos;
	uint8_t port0;
	uint8_t sound_io_buffer[256];
};

cm32p_state::cm32p_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag)
	, cpu(*this, "maincpu")
	, lcd(*this, "lcd")
	, midi_timer(*this, "midi_timer")
	, test_sw(*this, "SW")
{
}


// screen update function from Roland D-110
uint32_t cm32p_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t y,ra,gfx;
	uint16_t sy=0,x;
	const uint8_t *data = lcd->render();
	bitmap.fill(0);

	for (y = 0; y < 2; y++)
	{
		for (ra = 0; ra < 9; ra++)
		{
			uint16_t *p = &bitmap.pix16(sy++);

			for (x = 0; x < 16; x++)
			{
				gfx = 0;
				if (ra < 8)
					gfx = data[x*16 + y*640 + ra];

				*p++ = BIT(gfx, 4);
				*p++ = BIT(gfx, 3);
				*p++ = BIT(gfx, 2);
				*p++ = BIT(gfx, 1);
				*p++ = BIT(gfx, 0);
				*p++ = 0;
			}
		}
	}
	return 0;
}

void cm32p_state::machine_start()
{
	//lcd_data_buffer_pos = 0;
	uint8_t *rom = memregion("maincpu")->base();
	rom[0xBB2D] = 0x03;	// hack to make test mode work (TODO: remove)
}

void cm32p_state::machine_reset()
{
	midi_timer->adjust(attotime::from_hz(1));
	midi_pos = 0;
	port0 = 0;
}

WRITE8_MEMBER(cm32p_state::lcd_ctrl_w)
{
	lcd->control_w(data);
	//for(int i=0; i != lcd_data_buffer_pos; i++)
	//	lcd->data_w(lcd_data_buffer[i]);
	//lcd_data_buffer_pos = 0;
}

READ8_MEMBER(cm32p_state::lcd_ctrl_r)
{
	// The CM-64 service manual lists "D-110 LCD UNIT" for using PCM test mode, so I assume it works like that.
	// However, the CM-32P firmware doesn't seem to ever read the status.
	return lcd->control_r() >> 7;
}

WRITE8_MEMBER(cm32p_state::lcd_data_w)
{
	//lcd_data_buffer[lcd_data_buffer_pos++] = data;
	lcd->data_w(data);
}

WRITE16_MEMBER(cm32p_state::midi_w)
{
	logerror("midi_out %02x\n", data);
	midi = data;
}

TIMER_DEVICE_CALLBACK_MEMBER(cm32p_state::midi_timer_cb)
{
	const static uint8_t midi_data[3] = { 0x9a, 0x40, 0x7f };
	midi = midi_data[midi_pos++];
	logerror("midi_in %02x\n", midi);
	cpu->serial_w(midi);
	if(midi_pos < sizeof(midi_data))
		midi_timer->adjust(attotime::from_hz(1250));
}

READ16_MEMBER(cm32p_state::port0_r)
{
	return port0;
}

READ8_MEMBER(cm32p_state::pcmrom_r)
{
	offs_t romOfs = SCRAMBLE_ADDRESS(offset);
	const uint8_t* pcm_rom = memregion("pcm32")->base();
	return UNSCRAMBLE_DATA(pcm_rom[romOfs]);
}

READ8_MEMBER(cm32p_state::snd_io_r)
{
	if (offset == 0x01)
	{
		// code for reading from the PCM sample table is at 0xB027
		// The code at 0xB0AC writes to 1411/1F (??), then 1403/02 (bank), then 1409/08/0B/140A (address).
		// It waits a few cycles and at 0xB0F7 it reads the resulting data from 1401.
		offs_t bank = sound_io_buffer[0x03];
		offs_t addr = (sound_io_buffer[0x09] << 0) | (sound_io_buffer[0x0A] << 8) | (sound_io_buffer[0x0B] << 16);
		addr = (addr >> 6) + 2;
		addr |= (bank << 16);
		// write actual ROM address to 1440..1443 for debugging
		sound_io_buffer[0x43] = (addr >>  0) & 0xFF;
		sound_io_buffer[0x42] = (addr >>  8) & 0xFF;
		sound_io_buffer[0x41] = (addr >> 16) & 0xFF;
		sound_io_buffer[0x40] = (addr >> 24) & 0xFF;
		return pcmrom_r(space, addr, 0xFF);
	}
	return sound_io_buffer[offset];
}

WRITE8_MEMBER(cm32p_state::snd_io_w)
{
	sound_io_buffer[offset] = data;
}

READ8_MEMBER(cm32p_state::test_sw_r)
{
	return test_sw->read();
}

TIMER_DEVICE_CALLBACK_MEMBER(cm32p_state::samples_timer_cb)
{
	port0 ^= 0x10;
}

void cm32p_state::mt32_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(0, 0, 0));
	palette.set_pen_color(1, rgb_t(0, 255, 0));
}

void cm32p_state::cm32p_map(address_map &map)
{
	map(0x1000, 0x10ff).ram();	// I/O ?? (writes to 1080..82/86/8C/8D)
	map(0x1100, 0x1100).rw(FUNC(cm32p_state::lcd_ctrl_r), FUNC(cm32p_state::lcd_ctrl_w));
	map(0x1102, 0x1102).w(FUNC(cm32p_state::lcd_data_w));
	//map(0x1200, 0x13ff).ram();	// I/O ?? (reads 1300)
	map(0x1300, 0x1300).r(FUNC(cm32p_state::test_sw_r));
	map(0x1400, 0x14ff).rw(FUNC(cm32p_state::snd_io_r), FUNC(cm32p_state::snd_io_w));	// sound chip area? (writes to 1400..1F, reads 1401)
	map(0x2000, 0x20ff).rom().region("maincpu", 0x2000);	// init vector @ 2080
	map(0x2100, 0x3fff).ram();	// the program clears region 2100..3EFF at init and sets SP=3FFE
	map(0x4000, 0xbfff).rom().region("maincpu", 0x4000);
	map(0xc000, 0xffff).r(FUNC(cm32p_state::pcmrom_r));	// show descrambled PCM ROM (for debugging)
}

void cm32p_state::cm32p(machine_config &config)
{
	i8x9x_device &maincpu(P8098(config, cpu, 12_MHz_XTAL));
	maincpu.set_addrmap(AS_PROGRAM, &cm32p_state::cm32p_map);
	maincpu.ach7_cb().set_ioport("A7");
	maincpu.serial_tx_cb().set(FUNC(cm32p_state::midi_w));
	maincpu.in_p0_cb().set(FUNC(cm32p_state::port0_r));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_screen_update(FUNC(cm32p_state::screen_update));
	screen.set_size(16*6-1, (16*6-1)*3/4);
	screen.set_visarea(0, 16*6-2, 0, (16*6-1)*3/4-1);
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(cm32p_state::mt32_palette), 2);

	MSM6222B_01(config, lcd, 0);

	TIMER(config, midi_timer).configure_generic(FUNC(cm32p_state::midi_timer_cb));

	TIMER(config, "samples_timer").configure_periodic(FUNC(cm32p_state::samples_timer_cb), attotime::from_hz(32000*2));
}

ROM_START( cm32p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cm-32p-ic9.bin",  0x000000, 0x10000, CRC(6f2f6dfd) SHA1(689f77c1d56f923ef1dab7d993a124c47736bc56) )

	ROM_REGION( 0x200000, "pcm32", 0 )
	ROM_LOAD( "cm-32p-ic18.bin", 0x000000, 0x80000, CRC(8e53b2a3) SHA1(4872530870d5079776e80e477febe425dc0ec1df) )
	ROM_LOAD( "cm-32p-ic19.bin", 0x080000, 0x80000, CRC(c8220761) SHA1(49e55fa672020f95fd9c858ceaae94d6db93df7d) )
	ROM_LOAD( "cm-32p-ic20.bin", 0x100000, 0x80000, CRC(733c4054) SHA1(9b6b59ab74e5bf838702abb087c408aaa85b7b1f) )
ROM_END

CONS( 1989, cm32p, 0, 0, cm32p, cm32p, cm32p_state, empty_init, "Roland", "CM-32P", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
