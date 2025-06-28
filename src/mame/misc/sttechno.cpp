// license:BSD-3-Clause
// copyright-holders:windyfairy

/*
Only known game: Shamisen Brothers Vol 1 (2003)
The game was also ported for Namco System 10

Kato's STV01 MainPCB Rev. C
ST-Techno PCB
------------------------------------------------------------------------------------------
|                                                                                        |
|                                                          -------------                 |
|    U18        U47   U45   U40   U39   U37   U31         |             |                |
|                                                         |     U6      |       U30      |
|               U48   U44   U41   U38   U33   U32         |   STT-SA1   |                |
|    U19                                                  |             |                |
|                  U4                                      -------------                 |
|    U27                       U3                            ---------           U35     |
|                U5                        U14     X2       |   U36   |                  |
|                                          U13              |         |          U42     |
|    VR1              U16          ------------              ---------                   |
|    ---------                    |            |          -----------------------        |
|   | U29     |       U17         |     U15    |         |  U46 PRG ROM          |       |
|   |         |                   |   STT-GA1  |          -----------------------        |
|   |         |                   |            |                                       C |
|   |         |                    ------------                   --------             N |
|   |         |                 U11               X1             |  U34   |            1 |
|   |         |                            U2                    |  GLU   |              |
|   |         |                                                  |        |              |
|    ---------                                                    --------               |
|                                                                                        |
|                          U61                         U1           U60   U43   U49      |
|                                                                          RESET         |
|     |-----| (EDGE) |--|                EDGE                |------|    CN3       CN2   |
------|     |--------|  |------------------------------------|      |---------------------

X1: 33.3333 MHz
X2: 42.95454 MHz

U1 (TD62083F): Toshiba TD62083AF, 8ch high current darlington sink driver
U2, U3, U4 (AM29F160): Fujitsu MBM29F160TE-70, 16 Megabit (2 M x 8-Bit/1 M x 16-Bit)
U5 (BU9480): 9480F, 16-bit stereo D/A converter
U6: STT-SA1, markings scratched off but patterns are similar to an Actel FPGA and is also 208 pins. Sound chip
U11 (CXD1178Q): Sony CXD1178Q, 8-bit RGB 3-channel D/A converter
U13, U14: Taiwan Memory Technology T14M256A, 32k x 8 high speed CMOS SRAM
U15: STT-GA1 (custom chip? has heatsink), likely an FPGA, 240 pin. Graphics chip
U16, U17 (T224160B): Taiwan Memory Technology T224160B, 256k x 16 DRAM
U18, U19, U27 (NJM2904): 2904 2148B JRC, single-supply dual operational amplifier
U29 (TA8210): Under large heatsink, sound amplifier?
U30 (TA48M025F): 48M025F, three-terminal low dropout voltage regulator
U31, U32, U33, U37, U38, U39, U40, U41, U44, U45, U47, U48 (HY628400): Toshiba TC554001, 512k x 8 SRAM
U34 (GLU): Actel A54SX08A, 208 pin
U35, U42 (HY628100): Hyundai HY628100B 0045A LT1-70, 128k x 8 CMOS SRAM
U36 (68HC000): Toshiba TMP68HC000F-16
U43 (PST594C): Mitsumi PST594C, system reset monolithic IC
U45 (PRG ROM): MX A9630 27C4100DC-12 MR38674-1, labeled G112 V1.01
U49: 74HC14A
U60 (ST232ABD): MAX232A CSE 0107
U61: Toshiba TD62064AF, 4ch high current darlington sink driver

CN1: 40 pin IDE connector for CD-ROM
CN2: 3 pin header, unused
CN3: USB connector (JVS?)

VR1: Sound pot

CD-ROM: Mitsumi FX5400W ATAPI CD-ROM drive

EDGE: EDGE28P x2side (56P) P=3.96mm connector. Not JAMMA.
    Solder Side | Parts Side
       GND - A | | 1  - GND
       GND - B | | 2  - GND
       +5V - C | | 3  - +5V
       +5V - D | | 4  - +5V
      +12V - E | | 5  - +12V
       KEY - F | | 6  - KEY
    METER2 - H | | 7  - METER1
  LOCKOUT2 - J | | 8  - LOCKOUT1
  COIN SW2 - K | | 9  - COIN SW1
  1P BG(-) - L | | 10 - 1P BG(+)
  2P BG(-) - M | | 11 - 2P BG(+)
   VIDEO G - N | | 12 - VIDEO R
VIDEO SYNC - P | | 13 - VIDEO B
SERVICE SW - R | | 14 - VIDEO GND
   EXT. IN - S | | 15 - TEST SW
  2P START - T | | 16 - 1P START
     2P UP - U | | 17 - 1P UP
    2P MID - V | | 18 - 1P MID
    2P LOW - W | | 19 - 1P LOW
   2P SHOT - X | | 20 - 1P SHOT
 2P DETECT - Y | | 21 - 1P DETECT
   UP LAMP - Z | | 22 - SEL UP
 DOWN LAMP - a | | 23 - SEL DOWN
DECI. LAMP - b | | 24 - SEL DECI.
 EXT. OUT0 - c | | 25 - COIN LAMP1
 EXT. OUT1 - d | | 26 - COIN LAMP2
       GND - e | | 27 - GND
       GND - f | | 28 - GND


Notes:
- V1.00K is on the CD (pgx.bin) so it's known to exist
- Unknown exactly where RS232 is connected. Likely candidates are JVS, EXT IN/OUT (unused according to manual), or CN2
- You must send the command "4g63" before the terminal debugger will respond to any commands. Send "h" to see list of most commands.
*/

#include "emu.h"

#include "bus/ata/ataintf.h"
#include "bus/ata/atapicdr.h"
#include "bus/rs232/null_modem.h"
#include "bus/rs232/pty.h"
#include "bus/rs232/rs232.h"
#include "bus/rs232/terminal.h"
#include "cpu/m68000/m68000.h"
#include "machine/i8251.h"
#include "machine/intelfsh.h"
#include "machine/timer.h"
#include "sound/stt_sa1.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#define LOG_VIDEO_DISPLAY  (1U << 1)
#define LOG_UART           (1U << 2)

// #define VERBOSE (LOG_GENERAL | LOG_UART)
// #define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

namespace {

class sttechno_state : public driver_device, public device_serial_interface
{
public:
	sttechno_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, device_serial_interface(mconfig, *this)
		, m_maincpu(*this, "maincpu")
		, m_ata(*this, "ata")
		, m_flash(*this, "flash%u", 1)
		, m_video_flash(*this, "video_flash")
		, m_sound(*this, "pcm_sound")
		, m_rs232(*this, "rs232")
		, m_sttga1_ram_obj(*this, "sttga1_ram_obj")
		, m_sttga1_ram_pal(*this, "sttga1_ram_pal")
		, m_sound_ram(*this, "sound_ram")
	{
	}

	void shambros(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual void tra_callback() override;
	virtual void rcv_complete() override;

private:
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void cpu_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;

	void bank_w(uint16_t data);
	void bank_write_enable_w(uint16_t data);

	void data_w(offs_t offset, uint16_t data);
	uint16_t data_r(offs_t offset);

	void uart_data_w(uint16_t data);
	uint16_t uart_data_r();

	uint16_t uart_txrdy_r();

	void uart_config_w(uint16_t data);

	void ata_cs0_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t ata_cs0_r(offs_t offset, uint16_t mem_mask = ~0);

	void ata_control_w(uint16_t data);

	uint16_t video_unk_r();
	void sttga1_video_flash_w(offs_t offset, uint16_t data);
	uint16_t sttga1_video_flash_r(offs_t offset);
	void sttga1_video_flash_write_enable_w(offs_t offset, uint16_t data);
	void sttga1_enabled_w(offs_t offset, uint16_t data);

	void sound_device_enable_w(offs_t offset, uint16_t data);

	TIMER_DEVICE_CALLBACK_MEMBER(irq6_timer);

	required_device<cpu_device> m_maincpu;
	required_device<ata_interface_device> m_ata;
	required_device_array<intelfsh16_device, 2> m_flash;
	required_device<intelfsh16_device> m_video_flash;
	required_device<stt_sa1_device> m_sound;
	required_device<rs232_port_device> m_rs232;

	required_shared_ptr<uint16_t> m_sttga1_ram_obj;
	required_shared_ptr<uint16_t> m_sttga1_ram_pal;
	required_shared_ptr<uint16_t> m_sound_ram;

	uint16_t m_bank;
	uint16_t m_flash_write_enabled;

	bool m_sttga1_video_flash_write_enable;
	bool m_sttga1_enabled;

	uint8_t m_uart_rx;
	bool m_uart_rx_busy;

	bool m_ata_enabled;
};

void sttechno_state::machine_start()
{
	save_item(NAME(m_bank));
	save_item(NAME(m_flash_write_enabled));
	save_item(NAME(m_sttga1_video_flash_write_enable));
	save_item(NAME(m_sttga1_enabled));
	save_item(NAME(m_uart_rx));
	save_item(NAME(m_uart_rx_busy));
	save_item(NAME(m_ata_enabled));

	// TODO: Exact parameters for the UART are unknown
	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);
	set_rate(115200);
}

void sttechno_state::machine_reset()
{
	receive_register_reset();
	transmit_register_reset();

	m_bank = 0;
	m_flash_write_enabled = 0;

	m_sttga1_video_flash_write_enable = false;
	m_sttga1_enabled = false;

	m_uart_rx = 0;
	m_uart_rx_busy = false;

	m_ata_enabled = false;
}

uint32_t sttechno_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	if (!m_sttga1_enabled)
		return 0;

	// Use second half of OBJ RAM as the list to be rendered
	for (int offset = 0x2000; offset < 0x4000; offset += 4) {
		const uint16_t x = m_sttga1_ram_obj[offset] & 0x1ff;
		const uint16_t y = m_sttga1_ram_obj[offset+1] & 0x1ff;
		const uint16_t transparency = (BIT(m_sttga1_ram_obj[offset+1], 10, 5) / 31.0) * 255;

		const uint8_t palidx = BIT(m_sttga1_ram_obj[offset+2], 0, 6);
		const uint8_t unk = BIT(m_sttga1_ram_obj[offset+2], 6, 2);
		const uint8_t tiles_w = BIT(m_sttga1_ram_obj[offset+2], 8, 3) + 1;
		const uint8_t xflip = BIT(m_sttga1_ram_obj[offset+2], 11); // these aren't confirmed and don't seem to actually be used, but I think they are flipped bit flags based on the code
		const uint8_t tiles_h = BIT(m_sttga1_ram_obj[offset+2], 12, 3) + 1;
		const uint8_t yflip = BIT(m_sttga1_ram_obj[offset+2], 15);

		const uint32_t char_offset_base = BIT(m_sttga1_ram_obj[offset+3], 0, 14) * 0x100;
		const bool is_transparent = BIT(m_sttga1_ram_obj[offset+3], 14) && transparency != 255;
		const bool is_last = BIT(m_sttga1_ram_obj[offset+3], 15);

		LOGMASKED(LOG_VIDEO_DISPLAY, "%04x: x[%04x] y[%04x] unk[%x] pal[%02x] w[%02x] h[%02x] xflip[%d] yflip[%d] char_offset_base[%08x] trans[%d %04x] is_last[%d] | obj[%04x]\n", offset, x, y, unk, palidx, tiles_w, tiles_h, xflip, yflip, char_offset_base, is_transparent, transparency, is_last, m_sttga1_ram_obj[offset+2]);

		if (is_last) {
			LOGMASKED(LOG_VIDEO_DISPLAY, "end of list\n");
			break;
		}

		for (int tile_y = 0; tile_y < tiles_h; tile_y++) {
			for (int tile_x = 0; tile_x < tiles_w; tile_x++) {
				for (int pix_y = 0; pix_y < 16; pix_y++) {
					for (int pix_x = 0; pix_x < 16; pix_x++) {
						const int tx = xflip
								? ((x + ((tiles_w - tile_x - 1) * 16) + (15 - pix_x)) % 512) // 512x512 framebuffer size, must be wrapped
								: ((x + (tile_x * 16) + pix_x) % 512);
						const int ty = yflip
								? ((y + (tiles_h - tile_y - 1) * 16 + (15 - pix_y)) % 512)
								: ((y + tile_y * 16 + pix_y) % 512);

						if (!cliprect.contains(tx, ty))
							continue;

						uint16_t *const pix = &bitmap.pix(ty, tx);
						const uint32_t char_offset = char_offset_base + (tile_y * (0x100 * tiles_w)) + (tile_x * 0x100) + (pix_y * 16) + pix_x;
						const uint16_t char_data = m_video_flash->read_raw(char_offset / 2);
						const int colidx = BIT(char_data, 8 * (1 - (char_offset & 1)), 8);
						uint16_t color = m_sttga1_ram_pal[palidx * 0x100 + colidx];

						if (colidx == 0)
							continue;

						pix[0] = is_transparent ? alpha_blend_r16(pix[0], color, transparency) : color;
					}
				}
			}
		}
	}

	return 0;
}

TIMER_DEVICE_CALLBACK_MEMBER(sttechno_state::irq6_timer)
{
	// IRQ 6 is responsible for incrementing the in-game song sync timer
	// If the IRQ is too fast or too slow then it affects a lot of gameplay-related things: the measure markers, the note placements, the timing judgements
	// The animation during the gameplay where the character on the right side jumps up and down is also based on this IRQ
	m_maincpu->set_input_line(6, HOLD_LINE);
}

void sttechno_state::bank_w(uint16_t data)
{
	m_bank = data;
}

void sttechno_state::bank_write_enable_w(uint16_t data)
{
	// Only appears to be used for banks 3 and 4
	// For bank 3 it'll write 1, for bank 4 it'll write 2
	// The game typically enables write, writes data, then immediately clears the write flag
	m_flash_write_enabled = data;
}

void sttechno_state::data_w(offs_t offset, uint16_t data)
{
	if (m_bank <= 2) {
		const offs_t offs = offset + (0x10'0000 * m_bank);
		if (offs < 0x100 / 2)
			m_sound->write(offs, data);
		else
			m_sound_ram[offs] = swapendian_int16(data);
	} else if (m_bank == 3) {
		if (BIT(m_flash_write_enabled, 0))
			m_flash[0]->write(offset, data);
	} else if (m_bank == 4) {
		if (BIT(m_flash_write_enabled, 1))
			m_flash[1]->write(offset, data);
	}
}

uint16_t sttechno_state::data_r( offs_t offset)
{
	if (m_bank <= 2) {
		const offs_t offs = offset + (0x10'0000 * m_bank);
		if (offs < 0x100 / 2)
			return m_sound->read(offs);
		else
			return swapendian_int16(m_sound_ram[offs]);
	} else if (m_bank == 3) {
		return m_flash[0]->read(offset);
	} else if (m_bank == 4) {
		return m_flash[1]->read(offset);
	}

	return 0;
}

void sttechno_state::uart_data_w(uint16_t data)
{
	LOGMASKED(LOG_UART, "uart_data_w %04x\n", data);
	transmit_register_setup(data & 0xff);
}

uint16_t sttechno_state::uart_data_r()
{
	uint16_t r = m_uart_rx;

	if (!machine().side_effects_disabled()) {
		m_uart_rx = 0;
		m_maincpu->set_input_line(4, CLEAR_LINE);
		m_uart_rx_busy = false;
	}

	return r;
}

void sttechno_state::tra_callback()
{
	m_rs232->write_txd(transmit_register_get_data_bit());
}

void sttechno_state::rcv_complete()
{
	receive_register_extract();
	m_uart_rx = get_received_char();
	m_uart_rx_busy = true;

	m_maincpu->set_input_line(4, ASSERT_LINE);
}

uint16_t sttechno_state::uart_txrdy_r()
{
	// Polled before writing to uart_data_w
	return !m_uart_rx_busy && is_transmit_register_empty() && !is_receive_register_full();
}

void sttechno_state::uart_config_w(uint16_t data)
{
	// 0x49 is written here
	LOGMASKED(LOG_UART, "uart_config_w %04x\n", data);
}

void sttechno_state::ata_cs0_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (!m_ata_enabled)
		return;

	m_ata->cs0_w(offset, data, mem_mask);
}

uint16_t sttechno_state::ata_cs0_r(offs_t offset, uint16_t mem_mask)
{
	if (!m_ata_enabled)
		return 0;

	return m_ata->cs0_r(offset, mem_mask);
}

void sttechno_state::ata_control_w(uint16_t data)
{
	if (data & 0x8000) {
		// TODO: CD-ROM hardware reset
		return;
	}

	m_ata_enabled = (data & 3) != 0;
}

uint16_t sttechno_state::video_unk_r()
{
	// This gets called at two points:
	// - Every time before the OBJ RAM gets updated
	// - Every time before setting FM_BYTEPROGRAM for the flash

	if (m_sttga1_video_flash_write_enable)
		return 0;

	// HACK: Copy the OBJ data into the unused second half of the OBJ RAM for rendering
	// This helps fix some small graphical glitches (sound test menu's text flicker, flickering during initial installation when programming the video flash)
	if (!machine().side_effects_disabled())
		std::copy_n(std::begin(m_sttga1_ram_obj), 0x2000, std::begin(m_sttga1_ram_obj) + 0x2000);

	return 0;
}

void sttechno_state::sttga1_video_flash_w(offs_t offset, uint16_t data)
{
	if (!m_sttga1_video_flash_write_enable)
		return;

	m_video_flash->write(offset, data);
}

uint16_t sttechno_state::sttga1_video_flash_r(offs_t offset)
{
	return m_video_flash->read(offset);
}

void sttechno_state::sttga1_video_flash_write_enable_w(offs_t offset, uint16_t data)
{
	// If it's set to 1 then 0xa0'0000 addresses flash, and if it's 0 then it addresses RAM?
	m_sttga1_video_flash_write_enable = data != 0;
}

void sttechno_state::sttga1_enabled_w(offs_t offset, uint16_t data)
{
	// Set to either 0 or 3 based on enabled status
	m_sttga1_enabled = data != 0;
}

void sttechno_state::cpu_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom().nopw(); // unchecked null pointer makes the game spam a ton of writes to 0x00000c-0x00000e sometimes
	map(0x200000, 0x3fffff).ram();

	map(0x400000, 0x400001).r(FUNC(sttechno_state::video_unk_r));

	map(0x500000, 0x500001).portr("IN1");
	map(0x500002, 0x500003).portr("IN2");
	map(0x500004, 0x500005).portw("OUT1");
	map(0x500006, 0x500007).portw("OUT2");
	map(0x500020, 0x500021).rw(m_sound, FUNC(stt_sa1_device::key_r), FUNC(stt_sa1_device::key_w));
	map(0x500022, 0x500023).w(m_sound, FUNC(stt_sa1_device::enable_w));
	map(0x500024, 0x500025).w(FUNC(sttechno_state::bank_w));
	map(0x500026, 0x500027).w(FUNC(sttechno_state::bank_write_enable_w));

	map(0x600000, 0x600001).rw(FUNC(sttechno_state::uart_data_r), FUNC(sttechno_state::uart_data_w));
	map(0x600002, 0x600003).r(FUNC(sttechno_state::uart_txrdy_r));
	map(0x600004, 0x600005).w(FUNC(sttechno_state::uart_config_w));

	map(0x70000c, 0x70000d).nopw(); // 2 is written here before CD-ROM commands are sent?
	map(0x700010, 0x70001f).rw(FUNC(sttechno_state::ata_cs0_r), FUNC(sttechno_state::ata_cs0_w));
	map(0x700020, 0x700021).w(FUNC(sttechno_state::ata_control_w));

	// Additionally, 0x100000 bytes for other RAM (framebuffer etc?)
	map(0x800000, 0x807fff).ram().share(m_sttga1_ram_obj); // OBJ RAM
	map(0x808000, 0x80ffff).ram().share(m_sttga1_ram_pal); // Palette RAM, 0x200 per palette
	// 810000-81001f region contains some other registers which look like video display configuration registers but I'm not sure how to read it
	map(0x810016, 0x810017).w(FUNC(sttechno_state::sttga1_enabled_w));
	map(0x810018, 0x810019).w(FUNC(sttechno_state::sttga1_video_flash_write_enable_w));

	map(0xa00000, 0xbfffff).rw(FUNC(sttechno_state::sttga1_video_flash_r), FUNC(sttechno_state::sttga1_video_flash_w));

	map(0xc00000, 0xdfffff).rw(FUNC(sttechno_state::data_r), FUNC(sttechno_state::data_w));
}

void sttechno_state::sound_map(address_map &map)
{
	map(0x000000, 0x5fffff).ram().share(m_sound_ram);
	map(0x600000, 0x7fffff).r(m_flash[0], FUNC(intelfsh16_device::read_raw));
	map(0x800000, 0x9fffff).r(m_flash[1], FUNC(intelfsh16_device::read_raw));
}


static INPUT_PORTS_START( shambros )
	PORT_START("IN1")
	PORT_BIT( 0x3f00, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("Neck Upper") // 1P UP
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("Neck Center") // 1P MID
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME("Neck Lower") // 1P LOW
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME("Bachi") // 1P SHOT
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(2) PORT_NAME("Neck Upper") // 2P UP
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_PLAYER(2) PORT_NAME("Neck Center") // 2P MID
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_PLAYER(2) PORT_NAME("Neck Lower") // 2P LOW
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_PLAYER(2) PORT_NAME("Bachi") // 2P SHOT
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_CUSTOM ) // 1P DETECT
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_CUSTOM ) // 2P DETECT

	PORT_START("IN2")
	PORT_BIT( 0xfe43, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) // Select up
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) // Select down
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START ) // Enter
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_COIN1 ) // Coin
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_SERVICE1 ) // Service button
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_SERVICE ) // Test menu button

	PORT_START( "OUT1" )
	PORT_BIT( 0xfffa, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_OUTPUT ) // Coin meter
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_OUTPUT ) // Coin lockout

	PORT_START( "OUT2" )
	PORT_BIT( 0xfff8, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_OUTPUT ) // SEL UP
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_OUTPUT ) // SEL DOWN
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_OUTPUT ) // SEL DECI.
INPUT_PORTS_END

static DEVICE_INPUT_DEFAULTS_START(debug_terminal)
	DEVICE_INPUT_DEFAULTS("RS232_TXBAUD", 0xff, RS232_BAUD_115200)
	DEVICE_INPUT_DEFAULTS("RS232_RXBAUD", 0xff, RS232_BAUD_115200)
	DEVICE_INPUT_DEFAULTS("RS232_DATABITS", 0xff, RS232_DATABITS_8)
	DEVICE_INPUT_DEFAULTS("RS232_PARITY", 0xff, RS232_PARITY_NONE)
	DEVICE_INPUT_DEFAULTS("RS232_STOPBITS", 0xff, RS232_STOPBITS_1)
DEVICE_INPUT_DEFAULTS_END


static void sttechno_debug_serial_devices(device_slot_interface &device)
{
	device.option_add("null_modem", NULL_MODEM);
	device.option_add("pty", PSEUDO_TERMINAL);
	device.option_add("terminal", SERIAL_TERMINAL);
}

void sttechno_state::shambros(machine_config &config)
{
	M68000(config, m_maincpu, XTAL(42'954'545) / 2); // divisor guessed, anything slower and the game stops functioning (timer flickers, CD-ROM reads are too slow)
	m_maincpu->set_addrmap(AS_PROGRAM, &sttechno_state::cpu_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(42'954'545) / 6, 456, 0, 336, 262, 0, 240); // guessed
	screen.set_screen_update(FUNC(sttechno_state::screen_update));
	screen.screen_vblank().set_inputline(m_maincpu, M68K_IRQ_2);
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::BGR_555);

	ATA_INTERFACE(config, m_ata).options(ata_devices, "cdrom", nullptr, true);
	m_ata->slot(0).set_option_machine_config("cdrom", [] (device_t *device) { downcast<atapi_cdrom_device &>(*device).set_is_ready(true); });

	FUJITSU_29F160TE_16BIT(config, m_flash[0]);
	FUJITSU_29F160TE_16BIT(config, m_flash[1]);
	FUJITSU_29F160TE_16BIT(config, m_video_flash);

	SPEAKER(config, "speaker", 2).front();

	STT_SA1(config, m_sound, XTAL(42'954'545) / 3);
	m_sound->set_addrmap(0, &sttechno_state::sound_map);
	m_sound->add_route(0, "speaker", 1.0, 0);
	m_sound->add_route(1, "speaker", 1.0, 1);
	TIMER(config, "irq6_timer").configure_periodic(FUNC(sttechno_state::irq6_timer), attotime::from_hz(XTAL(42'954'545) / 3 / 448 / 128)); // probably some interrupt?

	RS232_PORT(config, m_rs232, sttechno_debug_serial_devices, nullptr);
	m_rs232->set_option_device_input_defaults("null_modem", DEVICE_INPUT_DEFAULTS_NAME(debug_terminal));
	m_rs232->set_option_device_input_defaults("pty", DEVICE_INPUT_DEFAULTS_NAME(debug_terminal));
	m_rs232->set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(debug_terminal));
	m_rs232->rxd_handler().set(FUNC(sttechno_state::rx_w));
}


ROM_START( shambros )
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD16_WORD_SWAP( "g112 v1.01.u46", 0x00000, 0x80000, CRC(6e13bcde) SHA1(189b0052083717030ca868b95b9d43e1faaf9695) )

	DISK_REGION( "ata:0:cdrom" )
	DISK_IMAGE( "sb01-100", 0, SHA1(abd1d61871bcb4635acc691e35ec386823763ba2) )
ROM_END

} // anonymous namespace


GAME( 2003, shambros, 0, shambros, shambros, sttechno_state, empty_init, ROT0, "Kato's", "Shamisen Brothers Vol 1 (V1.01K)", MACHINE_IMPERFECT_TIMING )
