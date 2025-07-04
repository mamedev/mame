// license:BSD-3-Clause
// copyright-holders:R. Belmont
/************************************************************************************************************

    Sega electromechanical fruit machines (834-6912 PCB)

    Z80 x2
    RF5C68 - sound
    RTC62421A
    16 MHz XTAL
    8-dip bank

    TODO:
    decryption needs to be verified (may be good), then everything else

************************************************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/msm6242.h"
#include "machine/nvram.h"
#include "machine/timer.h"
#include "sound/rf5c68.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class segafruit_state : public driver_device
{
public:
	segafruit_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_soundcpu(*this, "soundcpu")
		, m_soundlatch(*this, "soundlatch")
		, m_rf5c68(*this, "rf5c68")
		, m_decrypted_opcodes(*this, "decrypted_opcodes")
		//, m_soundbank(*this, "soundbank")
	{
	}

	void segafruit(machine_config &config) ATTR_COLD;

	void decrypt_m3001() ATTR_COLD;
	void decrypt_m4001() ATTR_COLD;
	void decrypt_m5001() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void opcodes_map(address_map &map) ATTR_COLD;
	void pcm_map(address_map &map) ATTR_COLD;
	void sound_mem_map(address_map &map) ATTR_COLD;
	void sound_io_map(address_map &map) ATTR_COLD;

	required_device<z80_device> m_maincpu;
	required_device<z80_device> m_soundcpu;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<rf5c68_device> m_rf5c68;

	required_shared_ptr<uint8_t> m_decrypted_opcodes;
	// required_memory_bank m_soundbank; // disabled until mapping is done
};

void segafruit_state::machine_start()
{
	// m_soundbank->configure_entries(0, 0x10, memregion("rf5c68")->base(), 0x2000);
};

void segafruit_state::mem_map(address_map &map) // TODO: check everything
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
	map(0x8000, 0x8fff).ram();
	// map(0x9000, 0x9000).rw();
	// map(0x9001, 0x9001).rw();
	// map(0x9001, 0x9001).w("soundlatch", FUNC(generic_latch_8_device::write));// main / sound latch?
	map(0x9800, 0x9fff).ram();
	//map(0xa000, 0xa000).r(); // inputs?
	//map(0xa001, 0xa001).r(); // "
	//map(0xa002, 0xa002).r(); // "
	//map(0xa003, 0xa003).r(); // "
	//map(0xa004, 0xa004).r(); // "
	// map(0xc000, 0xc04f).nopw(); // lamps / LEDs / reels?
	map(0xe000, 0xe00f).rw("rtc", FUNC(rtc62421_device::read), FUNC(rtc62421_device::write));
}

void segafruit_state::opcodes_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().share(m_decrypted_opcodes);
}

void segafruit_state::io_map(address_map &map)
{
}

void segafruit_state::sound_mem_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("soundcpu", 0);
	map(0x8000, 0x83ff).ram();
	//map(0xa000, 0xa000).r("soundlatch", FUNC(generic_latch_8_device::read));// main / sound latch?
	//map(0xc000, 0xc000).lw8(NAME([this] (uint8_t data) { m_soundbank->set_entry(data & 0x0f); })); // sound bank? where is the banked ROM mapped though?
	map(0xe000, 0xffff).m("rf5c68", FUNC(rf5c68_device::map));
}

void segafruit_state::sound_io_map(address_map &map)
{
}

void segafruit_state::pcm_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).ram();
}

static INPUT_PORTS_START(segafruit)
	PORT_START("SW2") // marked SW2, SW1 is a momentary pushbutton (reset?)
	PORT_DIPUNUSED_DIPLOC(0x01, 0x01, "SW2:1")
	PORT_DIPUNUSED_DIPLOC(0x02, 0x02, "SW2:2")
	PORT_DIPUNUSED_DIPLOC(0x04, 0x04, "SW2:3")
	PORT_DIPUNUSED_DIPLOC(0x08, 0x08, "SW2:4")
	PORT_DIPUNUSED_DIPLOC(0x10, 0x10, "SW2:5")
	PORT_DIPUNUSED_DIPLOC(0x20, 0x20, "SW2:6")
	PORT_DIPUNUSED_DIPLOC(0x40, 0x40, "SW2:7")
	PORT_DIPUNUSED_DIPLOC(0x80, 0x80, "SW2:8")
INPUT_PORTS_END

void segafruit_state::decrypt_m3001()
{
	uint8_t *rom = memregion("maincpu")->base();

	for (int i = 0; i < 0x8000; i++)
	{
		switch (i & 0x1111)
		{
			case 0x0000: m_decrypted_opcodes[i] = bitswap<8>(rom[i] ^ 0xa8, 7, 6, 3, 4, 5, 2, 1, 0); break;
			case 0x0001: m_decrypted_opcodes[i] = bitswap<8>(rom[i] ^ 0xa8, 3, 6, 5, 4, 7, 2, 1, 0); break;
			case 0x0010: m_decrypted_opcodes[i] = bitswap<8>(rom[i] ^ 0x28, 5, 6, 3, 4, 7, 2, 1, 0); break;
			case 0x0011: m_decrypted_opcodes[i] = bitswap<8>(rom[i] ^ 0xa8, 7, 6, 5, 4, 3, 2, 1, 0); break;
			case 0x0100: m_decrypted_opcodes[i] = bitswap<8>(rom[i] ^ 0xa8, 3, 6, 5, 4, 7, 2, 1, 0); break;
			case 0x0101: m_decrypted_opcodes[i] = bitswap<8>(rom[i] ^ 0xa8, 3, 6, 5, 4, 7, 2, 1, 0); break;
			case 0x0110: m_decrypted_opcodes[i] = bitswap<8>(rom[i] ^ 0x28, 5, 6, 3, 4, 7, 2, 1, 0); break;
			case 0x0111: m_decrypted_opcodes[i] = bitswap<8>(rom[i] ^ 0xa8, 7, 6, 3, 4, 5, 2, 1, 0); break;
			case 0x1000: m_decrypted_opcodes[i] = bitswap<8>(rom[i] ^ 0xa8, 7, 6, 3, 4, 5, 2, 1, 0); break;
			case 0x1001: m_decrypted_opcodes[i] = bitswap<8>(rom[i] ^ 0x28, 5, 6, 7, 4, 3, 2, 1, 0); break;
			case 0x1010: m_decrypted_opcodes[i] = bitswap<8>(rom[i] ^ 0xa8, 3, 6, 5, 4, 7, 2, 1, 0); break;
			case 0x1011: m_decrypted_opcodes[i] = bitswap<8>(rom[i] ^ 0xa8, 3, 6, 5, 4, 7, 2, 1, 0); break;
			case 0x1100: m_decrypted_opcodes[i] = bitswap<8>(rom[i] ^ 0xa8, 3, 6, 5, 4, 7, 2, 1, 0); break;
			case 0x1101: m_decrypted_opcodes[i] = bitswap<8>(rom[i] ^ 0xa8, 7, 6, 3, 4, 5, 2, 1, 0); break;
			case 0x1110: m_decrypted_opcodes[i] = bitswap<8>(rom[i] ^ 0x28, 5, 6, 7, 4, 3, 2, 1, 0); break;
			case 0x1111: m_decrypted_opcodes[i] = bitswap<8>(rom[i] ^ 0xa8, 3, 6, 5, 4, 7, 2, 1, 0); break;
		}
	}

	for (int i = 0; i < 0x8000; i++)
	{
		switch (i & 0x1111)
		{
			case 0x0000: rom[i] = bitswap<8>(rom[i] ^ 0xa8, 3, 6, 5, 4, 7, 2, 1, 0); break;
			case 0x0001: rom[i] = bitswap<8>(rom[i] ^ 0xa8, 3, 6, 5, 4, 7, 2, 1, 0); break;
			case 0x0010: rom[i] = bitswap<8>(rom[i] ^ 0x28, 5, 6, 3, 4, 7, 2, 1, 0); break;
			case 0x0011: rom[i] = bitswap<8>(rom[i] ^ 0xa8, 7, 6, 5, 4, 3, 2, 1, 0); break;
			case 0x0100: rom[i] = bitswap<8>(rom[i] ^ 0xa8, 3, 6, 7, 4, 5, 2, 1, 0); break;
			case 0x0101: rom[i] = bitswap<8>(rom[i] ^ 0xa8, 3, 6, 5, 4, 7, 2, 1, 0); break;
			case 0x0110: rom[i] = bitswap<8>(rom[i] ^ 0x28, 5, 6, 3, 4, 7, 2, 1, 0); break;
			case 0x0111: rom[i] = bitswap<8>(rom[i] ^ 0xa8, 7, 6, 3, 4, 5, 2, 1, 0); break;
			case 0x1000: rom[i] = bitswap<8>(rom[i] ^ 0xa8, 3, 6, 5, 4, 7, 2, 1, 0); break;
			case 0x1001: rom[i] = bitswap<8>(rom[i] ^ 0x28, 5, 6, 7, 4, 3, 2, 1, 0); break;
			case 0x1010: rom[i] = bitswap<8>(rom[i] ^ 0x28, 5, 6, 7, 4, 3, 2, 1, 0); break;
			case 0x1011: rom[i] = bitswap<8>(rom[i] ^ 0xa8, 3, 6, 5, 4, 7, 2, 1, 0); break;
			case 0x1100: rom[i] = bitswap<8>(rom[i] ^ 0xa8, 7, 6, 3, 4, 5, 2, 1, 0); break;
			case 0x1101: rom[i] = bitswap<8>(rom[i] ^ 0x28, 5, 6, 7, 4, 3, 2, 1, 0); break;
			case 0x1110: rom[i] = bitswap<8>(rom[i] ^ 0xa8, 3, 6, 5, 4, 7, 2, 1, 0); break;
			case 0x1111: rom[i] = bitswap<8>(rom[i] ^ 0xa8, 7, 6, 3, 4, 5, 2, 1, 0); break;
		}
	}
}

void segafruit_state::decrypt_m4001()
{
	uint8_t *rom = memregion("maincpu")->base();

	for (int i = 0; i < 0x8000; i++)
	{
		switch (i & 0x1111)
		{
			case 0x0000: m_decrypted_opcodes[i] = bitswap<8>(rom[i] ^ 0x28, 3, 6, 7, 4, 5, 2, 1, 0); break;
			case 0x0001: m_decrypted_opcodes[i] = bitswap<8>(rom[i] ^ 0x00, 5, 6, 3, 4, 7, 2, 1, 0); break;
			case 0x0010: m_decrypted_opcodes[i] = bitswap<8>(rom[i] ^ 0x88, 3, 6, 5, 4, 7, 2, 1, 0); break;
			case 0x0011: m_decrypted_opcodes[i] = bitswap<8>(rom[i] ^ 0x00, 5, 6, 3, 4, 7, 2, 1, 0); break;
			case 0x0100: m_decrypted_opcodes[i] = bitswap<8>(rom[i] ^ 0x00, 5, 6, 3, 4, 7, 2, 1, 0); break;
			case 0x0101: m_decrypted_opcodes[i] = bitswap<8>(rom[i] ^ 0x00, 5, 6, 3, 4, 7, 2, 1, 0); break;
			case 0x0110: m_decrypted_opcodes[i] = bitswap<8>(rom[i] ^ 0x80, 5, 6, 7, 4, 3, 2, 1, 0); break;
			case 0x0111: m_decrypted_opcodes[i] = bitswap<8>(rom[i] ^ 0x80, 5, 6, 7, 4, 3, 2, 1, 0); break;
			case 0x1000: m_decrypted_opcodes[i] = bitswap<8>(rom[i] ^ 0x28, 3, 6, 7, 4, 5, 2, 1, 0); break;
			case 0x1001: m_decrypted_opcodes[i] = bitswap<8>(rom[i] ^ 0x08, 5, 6, 7, 4, 3, 2, 1, 0); break;
			case 0x1010: m_decrypted_opcodes[i] = bitswap<8>(rom[i] ^ 0x20, 7, 6, 3, 4, 5, 2, 1, 0); break;
			case 0x1011: m_decrypted_opcodes[i] = bitswap<8>(rom[i] ^ 0x28, 3, 6, 7, 4, 5, 2, 1, 0); break;
			case 0x1100: m_decrypted_opcodes[i] = bitswap<8>(rom[i] ^ 0x20, 7, 6, 3, 4, 5, 2, 1, 0); break;
			case 0x1101: m_decrypted_opcodes[i] = bitswap<8>(rom[i] ^ 0x20, 7, 6, 3, 4, 5, 2, 1, 0); break;
			case 0x1110: m_decrypted_opcodes[i] = bitswap<8>(rom[i] ^ 0x80, 5, 6, 7, 4, 3, 2, 1, 0); break;
			case 0x1111: m_decrypted_opcodes[i] = bitswap<8>(rom[i] ^ 0x80, 5, 6, 7, 4, 3, 2, 1, 0); break;
		}
	}

	for (int i = 0; i < 0x8000; i++)
	{
		switch (i & 0x1111)
		{
			case 0x0000: rom[i] = bitswap<8>(rom[i] ^ 0x00, 5, 6, 3, 4, 7, 2, 1, 0); break;
			case 0x0001: rom[i] = bitswap<8>(rom[i] ^ 0x28, 3, 6, 7, 4, 5, 2, 1, 0); break;
			case 0x0010: rom[i] = bitswap<8>(rom[i] ^ 0x00, 5, 6, 3, 4, 7, 2, 1, 0); break;
			case 0x0011: rom[i] = bitswap<8>(rom[i] ^ 0x88, 3, 6, 5, 4, 7, 2, 1, 0); break;
			case 0x0100: rom[i] = bitswap<8>(rom[i] ^ 0x08, 5, 6, 7, 4, 3, 2, 1, 0); break;
			case 0x0101: rom[i] = bitswap<8>(rom[i] ^ 0x80, 5, 6, 7, 4, 3, 2, 1, 0); break;
			case 0x0110: rom[i] = bitswap<8>(rom[i] ^ 0x00, 5, 6, 3, 4, 7, 2, 1, 0); break;
			case 0x0111: rom[i] = bitswap<8>(rom[i] ^ 0x88, 3, 6, 5, 4, 7, 2, 1, 0); break;
			case 0x1000: rom[i] = bitswap<8>(rom[i] ^ 0x08, 5, 6, 7, 4, 3, 2, 1, 0); break;
			case 0x1001: rom[i] = bitswap<8>(rom[i] ^ 0x28, 3, 6, 7, 4, 5, 2, 1, 0); break;
			case 0x1010: rom[i] = bitswap<8>(rom[i] ^ 0x20, 7, 6, 3, 4, 5, 2, 1, 0); break;
			case 0x1011: rom[i] = bitswap<8>(rom[i] ^ 0x20, 7, 6, 3, 4, 5, 2, 1, 0); break;
			case 0x1100: rom[i] = bitswap<8>(rom[i] ^ 0x08, 5, 6, 7, 4, 3, 2, 1, 0); break;
			case 0x1101: rom[i] = bitswap<8>(rom[i] ^ 0x80, 5, 6, 7, 4, 3, 2, 1, 0); break;
			case 0x1110: rom[i] = bitswap<8>(rom[i] ^ 0x20, 7, 6, 3, 4, 5, 2, 1, 0); break;
			case 0x1111: rom[i] = bitswap<8>(rom[i] ^ 0x20, 7, 6, 3, 4, 5, 2, 1, 0); break;
		}
	}
}

void segafruit_state::decrypt_m5001()
{
	uint8_t *rom = memregion("maincpu")->base();

	for (int i = 0; i < 0x8000; i++)
	{
		switch (i & 0x1111)
		{
			case 0x0000: m_decrypted_opcodes[i] = bitswap<8>(rom[i] ^ 0x20, 7, 6, 5, 4, 3, 2, 1, 0); break;
			case 0x0001: m_decrypted_opcodes[i] = bitswap<8>(rom[i] ^ 0x20, 7, 6, 5, 4, 3, 2, 1, 0); break;
			case 0x0010: m_decrypted_opcodes[i] = bitswap<8>(rom[i] ^ 0x88, 5, 6, 3, 4, 7, 2, 1, 0); break;
			case 0x0011: m_decrypted_opcodes[i] = bitswap<8>(rom[i] ^ 0xa0, 5, 6, 7, 4, 3, 2, 1, 0); break;
			case 0x0100: m_decrypted_opcodes[i] = bitswap<8>(rom[i] ^ 0x20, 7, 6, 5, 4, 3, 2, 1, 0); break;
			case 0x0101: m_decrypted_opcodes[i] = bitswap<8>(rom[i] ^ 0xa0, 5, 6, 7, 4, 3, 2, 1, 0); break;
			case 0x0110: m_decrypted_opcodes[i] = bitswap<8>(rom[i] ^ 0x88, 7, 6, 3, 4, 5, 2, 1, 0); break;
			case 0x0111: m_decrypted_opcodes[i] = bitswap<8>(rom[i] ^ 0xa0, 5, 6, 7, 4, 3, 2, 1, 0); break;
			case 0x1000: m_decrypted_opcodes[i] = bitswap<8>(rom[i] ^ 0x08, 3, 6, 5, 4, 7, 2, 1, 0); break;
			case 0x1001: m_decrypted_opcodes[i] = bitswap<8>(rom[i] ^ 0x88, 3, 6, 7, 4, 5, 2, 1, 0); break;
			case 0x1010: m_decrypted_opcodes[i] = bitswap<8>(rom[i] ^ 0x88, 3, 6, 7, 4, 5, 2, 1, 0); break;
			case 0x1011: m_decrypted_opcodes[i] = bitswap<8>(rom[i] ^ 0x88, 7, 6, 3, 4, 5, 2, 1, 0); break;
			case 0x1100: m_decrypted_opcodes[i] = bitswap<8>(rom[i] ^ 0x08, 3, 6, 5, 4, 7, 2, 1, 0); break;
			case 0x1101: m_decrypted_opcodes[i] = bitswap<8>(rom[i] ^ 0xa0, 5, 6, 7, 4, 3, 2, 1, 0); break;
			case 0x1110: m_decrypted_opcodes[i] = bitswap<8>(rom[i] ^ 0x08, 3, 6, 5, 4, 7, 2, 1, 0); break;
			case 0x1111: m_decrypted_opcodes[i] = bitswap<8>(rom[i] ^ 0xa0, 5, 6, 7, 4, 3, 2, 1, 0); break;
		}
	}

	for (int i = 0; i < 0x8000; i++)
	{
		switch (i & 0x1111)
		{
			case 0x0000: rom[i] = bitswap<8>(rom[i] ^ 0x08, 3, 6, 5, 4, 7, 2, 1, 0); break;
			case 0x0001: rom[i] = bitswap<8>(rom[i] ^ 0xa0, 5, 6, 7, 4, 3, 2, 1, 0); break;
			case 0x0010: rom[i] = bitswap<8>(rom[i] ^ 0x88, 7, 6, 3, 4, 5, 2, 1, 0); break;
			case 0x0011: rom[i] = bitswap<8>(rom[i] ^ 0x88, 7, 6, 3, 4, 5, 2, 1, 0); break;
			case 0x0100: rom[i] = bitswap<8>(rom[i] ^ 0x88, 5, 6, 7, 4, 3, 2, 1, 0); break;
			case 0x0101: rom[i] = bitswap<8>(rom[i] ^ 0xa0, 5, 6, 7, 4, 3, 2, 1, 0); break;
			case 0x0110: rom[i] = bitswap<8>(rom[i] ^ 0x88, 5, 6, 3, 4, 7, 2, 1, 0); break;
			case 0x0111: rom[i] = bitswap<8>(rom[i] ^ 0x88, 5, 6, 3, 4, 7, 2, 1, 0); break;
			case 0x1000: rom[i] = bitswap<8>(rom[i] ^ 0x08, 3, 6, 5, 4, 7, 2, 1, 0); break;
			case 0x1001: rom[i] = bitswap<8>(rom[i] ^ 0xa0, 5, 6, 7, 4, 3, 2, 1, 0); break;
			case 0x1010: rom[i] = bitswap<8>(rom[i] ^ 0x88, 7, 6, 3, 4, 5, 2, 1, 0); break;
			case 0x1011: rom[i] = bitswap<8>(rom[i] ^ 0x88, 7, 6, 3, 4, 5, 2, 1, 0); break;
			case 0x1100: rom[i] = bitswap<8>(rom[i] ^ 0x20, 7, 6, 3, 4, 5, 2, 1, 0); break;
			case 0x1101: rom[i] = bitswap<8>(rom[i] ^ 0x88, 3, 6, 7, 4, 5, 2, 1, 0); break;
			case 0x1110: rom[i] = bitswap<8>(rom[i] ^ 0x88, 7, 6, 3, 4, 5, 2, 1, 0); break;
			case 0x1111: rom[i] = bitswap<8>(rom[i] ^ 0x88, 3, 6, 7, 4, 5, 2, 1, 0); break;
		}
	}
}


void segafruit_state::segafruit(machine_config & config)
{
	Z80(config, m_maincpu, 16_MHz_XTAL / 4); // divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &segafruit_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &segafruit_state::io_map);
	m_maincpu->set_addrmap(AS_OPCODES, &segafruit_state::opcodes_map);
	m_maincpu->set_periodic_int(FUNC(segafruit_state::irq0_line_hold), attotime::from_hz(4 * 60)); // TODO: wrong

	Z80(config, m_soundcpu, 16_MHz_XTAL / 4); // divider not verified
	m_soundcpu->set_addrmap(AS_PROGRAM, &segafruit_state::sound_mem_map);
	m_soundcpu->set_addrmap(AS_IO, &segafruit_state::sound_io_map);

	RTC62421(config, "rtc", 32.768_kHz_XTAL);//.out_int_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	GENERIC_LATCH_8(config, m_soundlatch);//.data_pending_callback().set_inputline(m_soundcpu, INPUT_LINE_IRQ0);

	SPEAKER(config, "speaker", 2).front();

	RF5C68(config, m_rf5c68, 16_MHz_XTAL / 2); // divider not verified;
	m_rf5c68->set_addrmap(0, &segafruit_state::pcm_map);
	m_rf5c68->add_route(0, "speaker", 0.40, 0);
	m_rf5c68->add_route(1, "speaker", 0.40, 1);
}

ROM_START(m4001)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("epr-a12334.ic46", 0x000000, 0x008000, CRC(d56dbf57) SHA1(94f0ca78c14852a0fb12ad9ac9ce35fe0c19f113))

	ROM_REGION(0x4000, "soundcpu", 0)
	ROM_LOAD("epr-12335.ic51", 0x000000, 0x004000, CRC(d5ee7239) SHA1(9dad92623c8d732d26fffab8e50cd74616e2ac24))

	ROM_REGION(0x20000, "rf5c68", 0)
	ROM_LOAD("epr-12332.ic38", 0x000000, 0x010000, CRC(26b8a8ff) SHA1(2f0aa625d4162d63265618691701dc40593bdbdf))
	ROM_LOAD("epr-12333.ic24", 0x010000, 0x010000, CRC(b9db6c6e) SHA1(fc4610abd593e7f54422f4dcd2ff0170d6221d65))

	ROM_REGION(0x104, "plds", 0) // probably for decryption purpose
	ROM_LOAD("315-5352.ic72", 0x000, 0x104, NO_DUMP) // PAL16L8, protected
ROM_END

ROM_START(m3001)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("epr-12602.ic46", 0x000000, 0x008000, CRC(00445ca5) SHA1(8aaa2117b5a3e36fcb5ef9b24258390ab4585163))

	ROM_REGION(0x4000, "soundcpu", 0)
	ROM_LOAD("epr-12603.ic51", 0x000000, 0x004000, CRC(90663368) SHA1(f5134a0146fabb09c165055b8da542cbb83d717a))

	ROM_REGION(0x20000, "rf5c68", 0)
	ROM_LOAD("epr-12600.ic38", 0x000000, 0x010000, CRC(99179df6) SHA1(2c26bea6724ded2bef90738fbb4b38d21a1f0906))
	ROM_LOAD("epr-12601.ic24", 0x010000, 0x010000, CRC(b1cb4265) SHA1(ce55bd47b0db8ad99233d98d4afc686ab2c66d63))

	// PAL16L8
	ROM_REGION(0x104, "plds", 0) // probably for decryption purpose
	ROM_LOAD("315-5371.ic72", 0x000, 0x104, NO_DUMP) // PAL16L8, protected
ROM_END

ROM_START(m3004)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("epr-14116.ic46", 0x000000, 0x008000, CRC(2b02d391) SHA1(d4f700f10ff03a090c90223fab967b74f42826e7))

	ROM_REGION(0x4000, "soundcpu", 0)
	ROM_LOAD("epr-14115.ic51", 0x000000, 0x004000, CRC(b199edb3) SHA1(17d6c99eb926a49d573672acbbc597ad6ea9955c))

	ROM_REGION(0x20000, "rf5c68", 0)
	ROM_LOAD("epr-14113.ic38", 0x000000, 0x010000, CRC(5d51900b) SHA1(bf28d709e19b80e405f8401f8e6123bee56a8dd3))
	ROM_LOAD("epr-14114.ic24", 0x010000, 0x010000, CRC(6caf7c9e) SHA1(12a44fd6c74ed28b751ec35d2f59f406d5bad1df))

	// PAL16L8
	ROM_REGION(0x104, "plds", 0) // probably for decryption purpose
	ROM_LOAD("315-6510.ic72", 0x000, 0x104, NO_DUMP) // PAL16L8, protected. 6510 may be wrong, difficult to read.
ROM_END

ROM_START(m4002)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("epr-13248.ic46", 0x000000, 0x008000, CRC(ddcad7a6) SHA1(1ab2508a220476e9eb783ac0f4129734225c258a))

	ROM_REGION(0x4000, "soundcpu", 0) // very few bytes different to m4001. Both need to be checked.
	ROM_LOAD("epr-13247.ic51", 0x000000, 0x004000, CRC(39d6f8ef) SHA1(ebc5eb2e68202058919e960415123a34d87a1f9b)) // 1xxxxxxxxxxxxx = 0xFF

	ROM_REGION(0x20000, "rf5c68", 0) // same as m4001
	ROM_LOAD("epr-13245.ic38", 0x000000, 0x010000, CRC(26b8a8ff) SHA1(2f0aa625d4162d63265618691701dc40593bdbdf))
	ROM_LOAD("epr-13246.ic24", 0x010000, 0x010000, CRC(b9db6c6e) SHA1(fc4610abd593e7f54422f4dcd2ff0170d6221d65))

	ROM_REGION(0x104, "plds", 0) // probably for decryption purpose
	ROM_LOAD("315-5438.ic72", 0x000, 0x104, NO_DUMP) // PAL16L8, protected. 5438 may be wrong, difficult to read.
ROM_END

ROM_START(m5001)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("epr-a13163.ic46", 0x000000, 0x008000, CRC(d220c761) SHA1(5b69f80285e8aa5457244f62c5096a083ef9eaed))

	ROM_REGION(0x4000, "soundcpu", 0)
	ROM_LOAD("epr-13162.ic51", 0x000000, 0x004000, CRC(efb13f18) SHA1(bb04ed352dc0ccdefe33ad12a7932dc646edfc96)) // 1xxxxxxxxxxxxx = 0xFF

	ROM_REGION(0x20000, "rf5c68", 0)
	ROM_LOAD("epr-13160.ic38", 0x000000, 0x010000, CRC(3bc70ce4) SHA1(bfb4652002b3e9a48fd5645035306b35d4b4a699))
	ROM_LOAD("epr-13161.ic24", 0x010000, 0x010000, CRC(e8e3ad8e) SHA1(bfd466d150bba4b80981cd68323eac69935d3a25))

	ROM_REGION(0x104, "plds", 0) // probably for decryption purpose
	ROM_LOAD("315-5432.ic72", 0x000, 0x104, NO_DUMP) // PAL16L8, protected. 5438 may be wrong, difficult to read.
ROM_END

} // anonymous namespace


GAME(1990, m3001, 0, segafruit, segafruit, segafruit_state, decrypt_m3001, ROT0, "Sega", "M3001",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
GAME(199?, m3004, 0, segafruit, segafruit, segafruit_state, decrypt_m3001, ROT0, "Sega", "M3004",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND) // apparently same XORs / bitswaps as m3001
GAME(1990, m4001, 0, segafruit, segafruit, segafruit_state, decrypt_m4001, ROT0, "Sega", "M4001 (rev. A)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
GAME(199?, m4002, 0, segafruit, segafruit, segafruit_state, decrypt_m4001, ROT0, "Sega", "M4002",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND) // apparently same XORs / bitswaps as m4001
GAME(199?, m5001, 0, segafruit, segafruit, segafruit_state, decrypt_m5001, ROT0, "Sega", "M5001 (rev. A)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
