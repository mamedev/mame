// license:BSD-3-Clause
// copyright-holders:Valley Bell
/*************************************************************************************************

    Roland CM-32P driver

    Driver by Valley Bell

    Thanks to KitsuWhooa for the PCB layout diagram and part list.


Notes:
- When booting, it does a basic check by writing values 0..0xFF to 0x108A.
  It then expects (value & 0x03) to be read back from 0x1080 and (value) to be read back from 0x1081/0x1082.
  The routine for doing the check begins at 0x4686.
  Succeeding means it ends up at 0x4801.
- When in test mode, the firmware gets stuck in the loop at 0xBB06, waiting for Interrupt #5 (calls 0x4014) to fire.
  It gets there by calling function 0xBBBB, which writes text (address w@0x40, num characters b@0x43) to the LCD screen.
  This can be worked around by setting b@BB2D = 03.
- The firmware gets also stuck in the loop at 0x7D70, waiting for Interrupt #8 (calls 0x4020).
  This might be related to finding the best free voice?
  You can exit the loop by setting b@7D80 = 00.
- Test Mode shows the results of 4 checks:
    1. SRAM check
    2. MIDI IN/OUT check
    3. PCM ROM and card check
    4. RCC-CPU check
  Errors in check 2 and 3 are "not abnormal".
  In order to make check 2 pass successfully, you need to connect MIDI Out with MIDI In, creating a loopback.
  Check 3 requires the SN-U110-04 PCM card ("Electric Grand & Clavi") to be inserted in order to succeed.
- In order to access the built-in PCM ROM (IC18), the CPU asks the sound chip to read offsets 0x000000 .. 0x07FFFF.
  The PCM card is accessed via offsets 0x080000 .. 0x0FFFFF.
  The additional PCM ROMs are mapped to 0x100000 .. 0x17FFFF (IC19) and 0x200000 .. 0x27FFFF according to the sample table in IC18.
- The sound chip has 32 voices. Voice 0 is reserved by the firmware for reading data from the PCM ROM.
  The firmware allocates voices from back to front, i.e. voice 31 first.
- The "PCM sound chip" itself doesn't seem to support panning.
  Maybe it just has 6 outputs (one for each part, like the U-110) and an additional DSP does the rest.


TODO:
- figure out how "freeing a voice" works - right now the firmware gets stuck when playing the 32th note.


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
| CN4                          | unpopulated                            |
| CN5                          | PCM Card Slot                          |
| X1                           | Crystal 12MHz                          |
| X2                           | Crystal 32.768MHz                      |
|-----------------------------------------------------------------------|


PCM ROM address/data line scrambling:

     address              data
external internal   external internal
   A0 - - -  A0        D0 - - - D2
   A1 - - -  A5        D1 - - - D7
   A2 - - -  A4        D2 - - - D6
   A3 - - -  A6        D3 - - - D4
   A4 - - -  A1        D4 - - - D1
   A5 - - -  A2        D5 - - - D3
   A6 - - -  A3        D6 - - - D0
   A7 - - -  A8        D7 - - - D5
   A8 - - - A10
   A9 - - - A13
  A10 - - -  A9
  A11 - - -  A7
  A12 - - - A11
  A13 - - - A12
  A14 - - - A16
  A15 - - - A14
  A16 - - - A15
  A17 - - - A17
  A18 - - - A18

Line scramling of A0..A6 and D0..D7 matches the SN-U110 cards.
For A7..A16 they changed the scrambling.


PCM ROM Tables
--------------
Sample Table (address: 0x00100)
------------
Pos Len Description
00  02  start offset, bits 0-15 (Little Endian)
02  01  start offset, bits 16-18 (bits 0-2)
        PCM card select (bit 3): set for sounds on PCM cards
        ROM bank (bits 4-5):
            0 = IC18
            1 = IC19
            2 = IC20
        loop mode (bits 6-7):
            0 = normal loop
            1 = no loop
            2 = ping-pong (forwards, backwards, forwards, backwards, ...)
03  02  last sample (sample length = last sample + 1)
05  02  loop length (loop start = sample length - loop length)
07  01  ??
08  01  reference note (when played back at 32000 Hz)
09  01  ??
-> 0Ah bytes per entry

Tone List (address: 0x01000)
---------
Pos Len Description
00  0A  sample name
0A  01  tone type
            00 - single
            01 - dual
            02 - detune
            03 - velocity mix
            04 - velocity switch
            05..07 - invalid
            08..0F/10..17/.../78..7F - same as 00..07
            80..FF - rhythm?
0B  01  ??
0C  02  ??
0E  01  ??
0F  01  ??
10  0B  some note IDs (padded with FF)
1B  0C  sample IDs (always one more than number of note IDs, padded with FF)
27  09  ??
30  0B  some note IDs (padded with FF)
3B  0C  sample IDs (always one more than number of note IDs, padded with FF)
47  09  ??
-> 50h bytes per entry

Note: Section 30..4F is only used with tone types 01, 03, 04


CM-32P Firmware Work RAM Layout
-------------------------------
2100..22FF - MIDI data receive buffer

2344..23C1 - Part 1..6 "Patch temporary area" (see manual page 21, 0x15 bytes per partial)
23C4..23D4 - "System area" settings (see manual page 22, master volume, reverb setting, channel assignments)
  23CE-23D3 - Part 1..6 MIDI channel
23D6..25B5 - Part 1..6 instrument data (0x50 bytes per partial, from PCM ROM at 0x1000/0x1050/0x10A0/...)
25B8..2B57 - Part 1..6 sample table data (0xF0 bytes per partial, from PCM ROM at 0x0100/0x010A/0x0114/...)

34DC..34E1 - Part 1..6 Modulation (CC #1, initialized with 0)
34E2..34E7 - Part 1..6 Pitch Bend LSB (initialized with 0)
34E8..34ED - Part 1..6 Pitch Bend MSB (initialized with 64)
34EE..34F3 - Part 1..6 Expression setting (CC #11, initialized with 100)
34F4..34F9 - Part 1..6 Sustain setting (CC #64, initialized with 0)
34FA..34FF - Part 1..6 unused (initialized with 0)
3500..3505 - Part 1..6 RPN LSB (CC #98, initialized with 0xFF)
3506..350B - Part 1..6 RPN MSB (CC #99, initialized with 0xFF)
350C..3511 - Part 1..6 NRPN received (initialized with 0xFF, set to 0 when RPN LSB/MSB is received, set to 0xFF when NRPN is received)
3512..3517 - Part 1..6 ?? (initialized with 0)
3518..351D - Part 1..6 Instrument setting
351E - Reverb Mode
351F - Reverb Time
3521 - ?? (initialized with 1)
352A..3889 - voice memory (32 bytes per block)
 35AA..35C9 - some jump table index for interrupt #8
38DE..394D - more voice memory (32 bytes per block)

397E..3??? - state of playing notes
  3986 - current panning volume, left speaker (00..1F)
  39A6 - current panning volume, right speaker (00..1F)
  39C6 - target panning volume, left speaker (00..1F)
  39E6 - target panning volume, right speaker (00..1F)

3D7C..3E84 - SysEx receive data buffer

Some routine locations
----------------------
0x4014  LCD related interrupt handler
0x401C  serial input (MIDI data) interrupt handler
0x4020  some interrupt handler required while playing notes
0x45CB  Initialization (memory clear + checks), external memory is checked from 0x4679 on
0x5024  decide whether or not Test Mode is entered (normal boot == jump to 0x502A)
0x50F5  MIDI handling code
0x65E8  PCM ROM instrument check
0x6650  PCM card instrument check (Note: assumes that the SN-U110-04 PCM card is inserted)
0x6EA4  play a note (parameters: 0040 - part, 0041 = note pitch, 0042 - velocity)
0xB027  load instrument data from PCM ROM (writes to 23D6 + 50*i)
0xB12B  load instrument sample data from PCM ROM (reads sample IDs from 23D6+1B + 50*i, writes to 25B8 + F0*i)
0xB1A0  load secondary instrument sample data from PCM ROM (reads sample IDs from 23D6+3B + 50h*i, writes to 25B8+50 + F0*i)
0xB1E8  load 1 sample table entry from PCM ROM
0xB316  PCM ROM signature check
0xBBBB  write text to LCD

*/

#include "emu.h"
#include "cpu/mcs96/i8x9x.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "sound/roland_lp.h"
#include "video/msm6222b.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

// unscramble address: ROM dump offset -> proper (descrambled) offset
#define UNSCRAMBLE_ADDR_INT(_offset) \
	bitswap<19>(_offset,18,17,15,14,16,12,11, 7, 9,13,10, 8, 3, 2, 1, 6, 4, 5, 0)
// scramble address: proper offset -> ROM dump offset
#define SCRAMBLE_ADDR_INT(_offset) \
	bitswap<19>(_offset,18,17,14,16,15, 9,13,12, 8,10, 7,11, 3, 1, 2, 6, 5, 4, 0)

// PCM cards use a different address line scrambling
#define UNSCRAMBLE_ADDR_EXT(_offset) \
	bitswap<19>(_offset,18,17, 8, 9,16,11,12, 7,14,10,13,15, 3, 2, 1, 6, 4, 5, 0)

#define UNSCRAMBLE_DATA(_data) \
	bitswap<8>(_data,1,2,7,3,5,0,4,6)


static INPUT_PORTS_START( cm32p )
	PORT_START("A7")
	PORT_BIT(0x03ff, 0x0000, IPT_DIAL) PORT_NAME("Knob") PORT_SENSITIVITY(50) PORT_KEYDELTA(8) PORT_CODE_DEC(KEYCODE_DOWN) PORT_CODE_INC(KEYCODE_UP)

	PORT_START("SERVICE")   // connected to Port 0 of the P8098 CPU.
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Test Switch") PORT_TOGGLE PORT_CODE(KEYCODE_F2) // SW A (checked during boot)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Test: Check/Tune") PORT_CODE(KEYCODE_B) // SW B
	//PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("PCM card inserted") PORT_TOGGLE PORT_CODE(KEYCODE_C)

	PORT_START("SW")    // test switches, accessed by reading from address 0x1300
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Test: MSB Adj.") PORT_CODE(KEYCODE_1)   // SW 1
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Test: THD Check") PORT_CODE(KEYCODE_2)  // SW 2
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Test: PCM Out: String 1") PORT_CODE(KEYCODE_3)  // SW 3
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Test: PCM Out: Sax 1") PORT_CODE(KEYCODE_4) // SW 4
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Test: PCM + Long Reverb") PORT_CODE(KEYCODE_5)  // SW 5
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Test: PCM + Short Reverb") PORT_CODE(KEYCODE_6) // SW 6
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Test: VCA Down Check") PORT_CODE(KEYCODE_7) // SW 7
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Test: VCA Up Check") PORT_CODE(KEYCODE_8)   // SW 8
INPUT_PORTS_END

class cm32p_state : public driver_device
{
public:
	cm32p_state(const machine_config &mconfig, device_type type, const char *tag);

	void cm32p(machine_config &config);

	void init_cm32p();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<i8x9x_device> cpu;
	required_device<mb87419_mb87420_device> pcm;
	required_device<generic_slot_device> pcmcard;
	required_device<msm6222b_device> lcd;
	required_device<timer_device> midi_timer;
	required_device<ram_device> some_ram;
	required_ioport test_sw;
	required_ioport service_port;

	void mt32_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void midi_w(u16 data);

	u8 lcd_ctrl_r();
	void lcd_ctrl_w(u8 data);
	void lcd_data_w(u8 data);
	u16 port0_r();
	u8 pcmrom_r(offs_t offset);
	u8 dsp_io_r(offs_t offset);
	void dsp_io_w(offs_t offset, u8 data);
	u8 snd_io_r(offs_t offset);
	void snd_io_w(offs_t offset, u8 data);
	u8 test_sw_r();

	TIMER_DEVICE_CALLBACK_MEMBER(midi_timer_cb);
	TIMER_DEVICE_CALLBACK_MEMBER(samples_timer_cb);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(card_load);
	DECLARE_DEVICE_IMAGE_UNLOAD_MEMBER(card_unload);

	void cm32p_map(address_map &map);

	void descramble_rom_internal(u8* dst, const u8* src);
	void descramble_rom_external(u8* dst, const u8* src);

	bool pcmard_loaded;
	u8 midi;
	int midi_pos;
	u8 sound_io_buffer[0x100];
	u8 dsp_io_buffer[0x80];
};

cm32p_state::cm32p_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag)
	, cpu(*this, "maincpu")
	, pcm(*this, "pcm")
	, pcmcard(*this, "cardslot")
	, lcd(*this, "lcd")
	, midi_timer(*this, "midi_timer")
	, some_ram(*this, "some_ram")
	, test_sw(*this, "SW")
	, service_port(*this, "SERVICE")
{
}


// screen update function from Roland D-110
uint32_t cm32p_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u16 sy=0;
	u8 const *const data = lcd->render();
	bitmap.fill(0);

	for (u8 y = 0; y < 2; y++)
	{
		for (u8 ra = 0; ra < 9; ra++)
		{
			u16 *p = &bitmap.pix(sy++);

			for (u16 x = 0; x < 16; x++)
			{
				u8 gfx = 0;
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
	u8 *rom = memregion("maincpu")->base();

	// TODO: The IC8 gate array has an "LCD INT" line that needs to be emulated. Then, the hack can be removed.
	// Note: The hack is not necessary when *not* using test mode.
	rom[0xBB2D] = 0x03; // hack to make test mode not freeze when displaying the LCD text

	// TODO: remove this hack
	rom[0x7D80] = 0x00; // hack to exit some loop waiting for interrupt #8
}

void cm32p_state::machine_reset()
{
	midi_timer->adjust(attotime::from_hz(1));
	midi_pos = 0;
}

DEVICE_IMAGE_LOAD_MEMBER(cm32p_state::card_load)
{
	uint32_t const size = pcmcard->common_get_size("rom");
	if (size > 0x080000)
		return std::make_pair(image_error::INVALIDLENGTH, "Invalid size (maximum supported is 512K)");

	pcmcard->rom_alloc(0x080000, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);    // cards are up to 512K
	pcmcard->common_load_rom(pcmcard->get_rom_base(), size, "rom");
	u8 *base = pcmcard->get_rom_base();
	if (size < 0x080000)
	{
		uint32_t mirror = (1 << (31 - count_leading_zeros_32(size)));
		if (mirror < 0x020000)  // due to how address descrambling works, we can currently only do mirroring for 128K pages
			mirror = 0x020000;
		for (uint32_t ofs = mirror; ofs < 0x080000; ofs += mirror)
			memcpy(base + ofs, base, mirror);
	}

	u8 *src = reinterpret_cast<u8 *>(memregion("pcmorg")->base());
	u8 *dst = reinterpret_cast<u8 *>(memregion("pcm")->base());
	memcpy(&src[0x080000], base, 0x080000);
	// descramble PCM card ROM
	descramble_rom_external(&dst[0x080000], &src[0x080000]);
	pcmard_loaded = true;

	return std::make_pair(std::error_condition(), std::string());
}

DEVICE_IMAGE_UNLOAD_MEMBER(cm32p_state::card_unload)
{
	u8 *src = reinterpret_cast<u8*>(memregion("pcmorg")->base());
	u8 *dst = reinterpret_cast<u8*>(memregion("pcm")->base());
	memset(&src[0x080000], 0xff, 0x080000);
	memset(&dst[0x080000], 0xff, 0x080000);
	pcmard_loaded = false;
}

void cm32p_state::lcd_ctrl_w(u8 data)
{
	lcd->control_w(data);
}

u8 cm32p_state::lcd_ctrl_r()
{
	// The CM-64 service manual lists "D-110 LCD UNIT" for using PCM test mode, so I assume it works like that.
	// However, the CM-32P firmware doesn't seem to ever read the status.
	return lcd->control_r() >> 7;
}

void cm32p_state::lcd_data_w(u8 data)
{
	lcd->data_w(data);
}

void cm32p_state::midi_w(u16 data)
{
	logerror("midi_out %02x\n", data);
	midi = data;
}

TIMER_DEVICE_CALLBACK_MEMBER(cm32p_state::midi_timer_cb)
{
	const static u8 midi_data[3] = { 0x9a, 0x40, 0x7f };
	midi = midi_data[midi_pos++];
	logerror("midi_in %02x\n", midi);
	cpu->serial_w(midi);
	if(midi_pos < sizeof(midi_data))
		midi_timer->adjust(attotime::from_hz(1250));
}

u16 cm32p_state::port0_r()
{
	return service_port->read() | (pcmard_loaded ? 0x10 : 0x00);
}

u8 cm32p_state::pcmrom_r(offs_t offset)
{
	const u8* pcm_rom = memregion("pcm")->base();
	return pcm_rom[offset];
}

u8 cm32p_state::dsp_io_r(offs_t offset)
{
	return dsp_io_buffer[offset];
}

void cm32p_state::dsp_io_w(offs_t offset, u8 data)
{
	dsp_io_buffer[offset] = data;
	// do read/write to some external memory, makes the RCC-CPU check pass. (routine at 0x4679)
	switch(offset)
	{
	case 0x04:
		// write to partials?? (written in loop at 0x4375)
		break;
	case 0x06:
		{
			u8* ram = some_ram->pointer();
			offs_t ofs = data;
			ram[0x000 | ofs] = dsp_io_buffer[0x00] & 0x03;
			ram[0x100 | ofs] = dsp_io_buffer[0x01];
			ram[0x200 | ofs] = dsp_io_buffer[0x02];
		}
		break;
	case 0x0A:
		{
			const u8* ram = some_ram->pointer();
			offs_t ofs = data;
			dsp_io_buffer[0x00] = ram[0x000 | ofs];
			dsp_io_buffer[0x01] = ram[0x100 | ofs];
			dsp_io_buffer[0x02] = ram[0x200 | ofs];
		}
		break;
	}
}

u8 cm32p_state::snd_io_r(offs_t offset)
{
	// lots of offset modification magic to achieve the following:
	//  - offsets 00..1F are "sound chip read"
	//  - offsets 20..3F are a readback of what was written to registers 00..1F
	//  - This behaviour is reversed for offset 01/21, which is used for reading the PCM sample tables.
	// All this is just for making debugging easier, as it allows one to check the
	// register state using the Memory Viewer.
	if (offset == 0x01 || offset == 0x21)
		offset ^= 0x20; // remove when PCM data readback via sound chip is confirmed to work
	if (offset < 0x20)
		return pcm->read(offset);
	if (offset < 0x40)
		offset -= 0x20;

	if (offset == 0x01)
	{
		// code for reading from the PCM sample table is at 0xB027
		// The code at 0xB0AC writes to 1411/1F (??), then 1403/02 (bank), then 1409/08/0B/0A (address).
		// It waits a few cycles and at 0xB0F7 it reads the resulting data from 1401.
		offs_t bank = sound_io_buffer[0x03];
		offs_t addr = (sound_io_buffer[0x09] << 0) | (sound_io_buffer[0x0A] << 8) | (sound_io_buffer[0x0B] << 16);
		addr = ((addr >> 6) + 2) & 0x3FFFF;
		addr |= (bank << 16);
		// write actual ROM address to 1440..1443 for debugging
		sound_io_buffer[0x43] = (addr >>  0) & 0xFF;
		sound_io_buffer[0x42] = (addr >>  8) & 0xFF;
		sound_io_buffer[0x41] = (addr >> 16) & 0xFF;
		sound_io_buffer[0x40] = (addr >> 24) & 0xFF;
		return pcmrom_r(addr);
	}
	return sound_io_buffer[offset];
}

void cm32p_state::snd_io_w(offs_t offset, u8 data)
{
	// register map
	// ------------
	// Note: 16-bit words are Little Endian, the firmware writes the odd byte is first
	//  00/01 - ??
	//  02/03 - ROM bank (only bits 11-13 are used, bit 11 = PCM card, bits 12-13 select between IC18/19/20)
	//  04/05 - frequency (2.14 fixed point, 0x4000 = 32000 Hz)
	//  06/07 - volume
	//  08/09 - sample start address, fraction (2.14 fixed point, i.e. 1 byte = 0x4000)
	//  0A/0B - sample start address (high word, i.e. address bits 2..17)
	//  0C/0D - sample end address (high word)
	//  0E/0F - sample loop address (high word)
	//  11/13/15/17 - voice enable mask (11 = least significant 8 bits, 17 = most significant 8 bits)
	//  1A - ??
	//  1F - voice select
	if (offset < 0x20)
		pcm->write(offset, data);
	sound_io_buffer[offset] = data;
}

u8 cm32p_state::test_sw_r()
{
	return test_sw->read();
}

TIMER_DEVICE_CALLBACK_MEMBER(cm32p_state::samples_timer_cb)
{
	// TODO: does this trigger something?
}

void cm32p_state::mt32_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(0, 0, 0));
	palette.set_pen_color(1, rgb_t(0, 255, 0));
}

void cm32p_state::cm32p_map(address_map &map)
{
	map(0x1080, 0x10ff).rw(FUNC(cm32p_state::dsp_io_r), FUNC(cm32p_state::dsp_io_w));   // DSP area (writes to 1080..82/86/8C/8D)
	map(0x1100, 0x1100).rw(FUNC(cm32p_state::lcd_ctrl_r), FUNC(cm32p_state::lcd_ctrl_w));
	map(0x1102, 0x1102).w(FUNC(cm32p_state::lcd_data_w));
	map(0x1300, 0x1300).r(FUNC(cm32p_state::test_sw_r));    // test switch state
	map(0x1400, 0x14ff).rw(FUNC(cm32p_state::snd_io_r), FUNC(cm32p_state::snd_io_w));   // sound chip area
	map(0x2000, 0x20ff).rom().region("maincpu", 0x2000);    // init vector @ 2080
	map(0x2100, 0x3fff).ram();  // main RAM
	map(0x4000, 0xffff).rom().region("maincpu", 0x4000);
}

void cm32p_state::cm32p(machine_config &config)
{
	i8x9x_device &maincpu(P8098(config, cpu, 12_MHz_XTAL));
	maincpu.set_addrmap(AS_PROGRAM, &cm32p_state::cm32p_map);
	maincpu.ach7_cb().set_ioport("A7");
	maincpu.serial_tx_cb().set(FUNC(cm32p_state::midi_w));
	maincpu.in_p0_cb().set(FUNC(cm32p_state::port0_r));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	MB87419_MB87420(config, pcm, 32.768_MHz_XTAL);
	pcm->int_callback().set_inputline(cpu, i8x9x_device::EXTINT_LINE);
	pcm->add_route(0, "lspeaker", 1.0);
	pcm->add_route(1, "rspeaker", 1.0);

	RAM(config, some_ram).set_default_size("8K");

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_screen_update(FUNC(cm32p_state::screen_update));
	screen.set_size(16*6-1, (16*6-1)*3/4);
	screen.set_visarea(0, 16*6-2, 0, (16*6-1)*3/4-1);
	screen.set_palette("palette");
	PALETTE(config, "palette", FUNC(cm32p_state::mt32_palette), 2);
	MSM6222B_01(config, lcd, 0);

	generic_cartslot_device &cardslot(GENERIC_CARTSLOT(config, "cardslot", generic_romram_plain_slot, "u110_card", "bin"));
	cardslot.set_device_load(FUNC(cm32p_state::card_load));
	cardslot.set_device_unload(FUNC(cm32p_state::card_unload));
	SOFTWARE_LIST(config, "card_list").set_original("u110_card");

	TIMER(config, midi_timer).configure_generic(FUNC(cm32p_state::midi_timer_cb));

	TIMER(config, "samples_timer").configure_periodic(FUNC(cm32p_state::samples_timer_cb), attotime::from_hz(32000*2));
}

void cm32p_state::init_cm32p()
{
	// Roland did a fair amount of scrambling on the address and data lines.
	// Only the first 0x80 bytes of the ROMs are readable text in a raw dump.
	// The CM-32P actually checks some of these header bytes, but it uses post-scrambling variants of offsets/values.
	u8* src = static_cast<u8*>(memregion("pcmorg")->base());
	u8* dst = static_cast<u8*>(memregion("pcm")->base());
	// descramble internal ROMs
	descramble_rom_internal(&dst[0x000000], &src[0x000000]);
	descramble_rom_internal(&dst[0x100000], &src[0x100000]);
	descramble_rom_internal(&dst[0x200000], &src[0x200000]);
	// descramble PCM card ROM
	descramble_rom_external(&dst[0x080000], &src[0x080000]);
}

void cm32p_state::descramble_rom_internal(u8* dst, const u8* src)
{
	for (offs_t srcpos = 0x00; srcpos < 0x80000; srcpos ++)
	{
		offs_t dstpos = UNSCRAMBLE_ADDR_INT(srcpos);
		dst[dstpos] = UNSCRAMBLE_DATA(src[srcpos]);
	}
}

void cm32p_state::descramble_rom_external(u8* dst, const u8* src)
{
	for (offs_t srcpos = 0x00; srcpos < 0x80000; srcpos ++)
	{
		offs_t dstpos = UNSCRAMBLE_ADDR_EXT(srcpos);
		dst[dstpos] = UNSCRAMBLE_DATA(src[srcpos]);
	}
}


ROM_START( cm32p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cm-32_p__1.0.0.am27c512.7d.ic9",  0x000000, 0x10000, CRC(6f2f6dfd) SHA1(689f77c1d56f923ef1dab7d993a124c47736bc56) ) // "CM-32 P // 1 0 0 <filled bubbles in red marker>" sticker on an AM27C512-150DC eprom @ IC9

	ROM_REGION( 0x400000, "pcmorg", 0 ) // ROMs before descrambling
	ROM_LOAD( "roland__r15179970__mb834000a-20__3b1_aa__8917_r00.45f.ic18", 0x000000, 0x80000, CRC(8e53b2a3) SHA1(4872530870d5079776e80e477febe425dc0ec1df) ) // markings under chip footprint are "MB834000A-20P-G-3B1"
	// 0x080000 .. 0x0FFFFF is reserved for the PCM card
	ROM_LOAD( "roland__r15179971__mb834000a-20__3b2_aa__8919_r02.34f.ic19", 0x100000, 0x80000, CRC(c8220761) SHA1(49e55fa672020f95fd9c858ceaae94d6db93df7d) ) // markings under chip footprint are "MB834000A20P-G-3B2" (including the missing dash, which is a typo on the board silkscreen)
	ROM_LOAD( "roland__r15179972__hn62304bpe98__9d1_japan.3f.ic20", 0x200000, 0x80000, CRC(733c4054) SHA1(9b6b59ab74e5bf838702abb087c408aaa85b7b1f) ) // markings under chip footprint are "HN62304BPE98"
	ROM_REGION( 0x400000, "pcm", ROMREGION_ERASEFF )    // ROMs after descrambling
ROM_END

} // anonymous namespace


SYST( 1989, cm32p, 0, 0, cm32p, cm32p, cm32p_state, init_cm32p, "Roland", "CM-32P", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
