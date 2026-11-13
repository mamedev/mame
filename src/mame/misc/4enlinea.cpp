// license:BSD-3-Clause
// copyright-holders:David Haywood, Roberto Fresca, Tomás García-Merás Capote

/*************************************************************************

  Cuatro en Linea.
  System I.
  1991, Compumatic

**************************************************************************

  PCB V487-02 (marked on the solder side).

  1x Z84C00AB6 (Z80A) CPU @ 8 MHz for video (see notes).
  1x Z84C00HB6 (Z80H) CPU @ 4 MHz for master/sound (IC20).
  1x AY-3-8910A (Microchip AY38910A/P).
  1x UMC UM487F (HCGA Controller)

  2x NEC D41464C (64K x 4-bit Dynamic NMOS RAM) for VRAM.
  1x UMC UM6264A (8K x 8-bit CMOS SRAM).

  2x 27C256 EPROMs (IC6 video, IC19 master).
  1x X24C16P Serial EEPROM (IC17).

  1x GAL16V8AS
  1x 74HC245, 2x 74HC244 (IC9, IC10), 1x 74LS32, 1x 74LS04.

  1x ES2 CM3080 (unknown DIP-18 IC)
  1x ES2 9046 (unknown PLCC-84 IC, IC21)
  1x 8952 CM 32 (unknown DIP-40 IC, IC26)

  1x 16.0000 MHz crystal. ; Divided by 2 (through CM3080) for the video CPU.
  1x 8.000 MHz crystal.   ; (XT3) Divided by 2 for the master CPU.
  1x 14.31818 MHz crystal ; For HCGA controller.

  CN1: 1 x 8 connector.
  CN2: 1 x 8 connector.
  CN3: 2 x 5 connector.
  CN4: 2 x 5 connector.
  CN5: 2 x 5 connector.
  CN6: 2 x 28 Jamma connector.
  CN7: 1 x 20 connector.
  CN8: 1 x 4 connector.
  CN9: 1 x 4 connector.
  CN10: DB9 video out connector.
  CN11 1 x 2 bridge connector.

**************************************************************************

  UM487F HCGA Controller notes...

  The 14.31818 MHz crystal is tied to pin 65 (OSC), while pin 64 (MOSC, the
  16.257 MHz MGA clock) is tied to GND, so the chip can only work in CGA mode.

  All the games set the CGA 320x200 graphics mode with the standard BIOS
  mode 4 CRTC parameters (38 28 2D 0A 7F 06 64 70 02 01 06 07 10 00 00 00),
  and toggle the video enable bit while redrawing the screen.

**************************************************************************

  Custom IC's...

  8952 CM 32 pinouts and peripheral circuitry:

                               8952 CM 32
                            .------v------.
    74HC244 (IC9), PIN 08 --|01         40|-- 74HC244 (IC9), PIN 17
    74HC244 (IC9), PIN 06 --|02         39|-- 74HC244 (IC9), PIN 15
    74HC244 (IC9), PIN 04 --|03         38|-- 74HC244 (IC9), PIN 13
    74HC244 (IC9), PIN 02 --|04         37|-- 74HC244 (IC9), PIN 11
   74HC244 (IC10), PIN 08 --|05         36|-- 74HC244 (IC10), PIN 17
   74HC244 (IC10), PIN 06 --|06         35|-- 74HC244 (IC10), PIN 15
   74HC244 (IC10), PIN 04 --|07    8    34|-- 74HC244 (IC10), PIN 13
   74HC244 (IC10), PIN 02 --|08    9    33|-- 74HC244 (IC10), PIN 11
                            |      5      |
                         /--|09    2    32|--\
  To CN1 (through IC15) | --|10         31|-- | To CN1 (through IC15)
                        | --|11         30|-- |
                         \--|12    C    29|--/
                            |      M     |
                         /--|13         28|--\
  To CN2 (through IC14) | --|14    3    27|-- | To CN2 (through IC14)
                        | --|15    2    26|-- |
                         \--|16         25|--/
                            |             |
      To CN3 and ES2 9046 --|17         24|--\
      To CN3 and ES2 9046 --|18         23|-- > bridge to GND
                   To CN3 --|19         22|--/
                            |             |
                      GND --|20         21|-- VCC
                            '-------------'

                 74HC244 (IC9)                                 74HC244 (IC10)
                  .---v---.                                     .---v---.
   GAL (PIN 17) --|01   20|-- VCC                GAL (PIN 16) --|01   20|-- VCC
  8952 (PIN 04) --|02   19|-- GAL (PIN 17)      8952 (PIN 08) --|02   19|-- GAL (PIN 16)
  MAIN Z80 (D7) --|03   18|-- MAIN Z80 (D0)     MAIN Z80 (D7) --|03   18|-- MAIN Z80 (D0)
  8952 (PIN 03) --|04   17|-- 8952 (PIN 40)     8952 (PIN 07) --|04   17|-- 8952 (PIN 36)
  MAIN Z80 (D6) --|05   16|-- MAIN Z80 (D1)     MAIN Z80 (D6) --|05   16|-- MAIN Z80 (D1)
  8952 (PIN 02) --|06   15|-- 8952 (PIN 39)     8952 (PIN 06) --|06   15|-- 8952 (PIN 35)
  MAIN Z80 (D5) --|07   14|-- MAIN Z80 (D2)     MAIN Z80 (D5) --|07   14|-- MAIN Z80 (D2)
  8952 (PIN 01) --|08   13|-- 8952 (PIN 38)     8952 (PIN 05) --|08   13|-- 8952 (PIN 34)
  MAIN Z80 (D4) --|09   12|-- MAIN Z80 (D3)     MAIN Z80 (D4) --|09   12|-- MAIN Z80 (D3)
            GND --|10   11|-- 8952 (PIN 37)               GND --|10   11|-- 8952 (PIN 33)
                  '-------'                                     '-------'


  ES2 CM3080 pinouts and peripheral circuitry:

                    CM3080
                   .---v---.
             VCC --|01   18|-- VCC          .--------.
             N/C --|02   17|----------------+ 16 MHz |
             N/C --|03   16|----------------+  Xtal  |
             N/C --|04   15|-- CLK OUT --.  '--------'
             GND --|05   14|-- N/C       |
             GND --|06   13|-- N/C       '--+-- (8MHz) UM487F (PIN 01, CLK)
  MAIN Z80 (/M1) --|07   12|-- GND          +-- (8MHz) MAIN Z80 (PIN 06, CLK)
    GAL (PIN 11) --|08   11|-- VCC
             GND --|09   10|-- MAIN Z80 (/INT)
                   '-------'


  Notes:

  - Looks like the GAL is switching the different '8952 CM 32' outputs
     through the 74HC244 drivers to the Z80 data bus.
  - CN1, CN2 & CN3 are blind connectors.
  - 8952 pinouts to CN1 & CN2, are also passing through locations
     IC14 & IC15 (both are unpopulated from factory).
  - GAL is GAL16V8 at location IC4.
  - CM3080 pins 16 & 17 have a 1 Megohm resistor in parallel before connect the
     16 MHz. crystal.
  - The CPU roles come from the PCB layout: the Z84C00AB6 sits along with the
     video parts (IC6 EPROM, 6264, CM3080, GAL, the 74HC245 and the IC9/IC10
     link buffers), and the Z84C00HB6 along with the master ones (IC19 EPROM,
     9046, AY-3-8910 and the 8 MHz crystal). So the 4 MHz rated Z80A would be
     the one running at 8 MHz. TODO: confirm by tracing the CM3080 clock
     output.
  - The 8952 CM 32 looks like a 32-bit serial input latched driver: it gets
     the master to video CPU link words from the 9046 (pins 17 & 18), and
     drives 16 bits to the video CPU data bus through IC9/IC10 and the other
     16 bits to CN1/CN2 through IC14/IC15 (DIP-18 footprints, probably for
     ULN2803 style drivers). Dardos has an unused routine that builds 9 lamp
     bits for that half.
  - BT1 (battery) is unpopulated, the settings are kept in the EEPROM.
  - The photographed PCB has some rework: wires around the 74LS04/74LS32 and
     the COMP pads next to CN10, a capacitor and a resistor on the solder
     side, and a wire from the 9046 to a hand-added transistor next to the
     AY-3-8910.

**************************************************************************

  Known games on this or similar hardware:

  - [DUMPED]  4 en Línea (Compumatic).
  - [DUMPED]  Dardos (Oper Coin).
  - [DUMPED]  Olympic Darts (K7 Kursaal).
              There is another (undumped) Z180-based board with a 27C4001 EPROM, a DAC
              for sound and a monochrome monitor output.
  - [DUMPED]  Sport Darts TV (Compumatic / Desarrollos y Recambios S.L.).
              The dumped version never shows the "Desarrollos y Recambios S.L." screen,
              but it's on the ROM, and there's no DIP switch to activate it.
              So, there's another (undumped) version that does show it (see pics at
              https://www.recreativas.org/sport-dart-tv-4371-compumatic).
  - [MISSING] Dart Queen (Compumatic / Daryde).

**************************************************************************

  Compumatic dual CPU boards (Cuatro en Linea, Dardos):

  The video CPU only draws the screens as commanded by the master CPU, which
  runs the game logic and the sound, and handles the inputs and the EEPROM
  through the ES2 9046 I/O ports.

**************************************************************************

  K7 / Sport Darts single CPU boards:

  Z84C00BB6 @ 7.159 MHz, UM487F, WF19054 (AY-3-8910), X24C16P, 6264 + battery
  and a 64 KB EPROM.

  The darts games (Dardos too) register missed darts through an impact
  detector microphone (MICINT, ignored while a sound is playing) and, at the
  end of a turn, wait for the player to cross an ultrasonic detector (ULTRAS,
  40 kHz emitter and receiver on their own board) before continuing on their
  own.

  The darts games take 500 and 100 Pts coins (Sport Dart T.V. manual, K7
  edge connector "COIN 500" and "COIN 100", and their "ENTRADAS 500 PTS" and
  "ENTRADAS 100 PTS" accounting).

  The EEPROM holds the settings, high scores and accounting. The darts games
  initialize it when it's blank.

  Port 1 bit 5 is the coin inhibit for Olympic Darts v3.00 and Sport Darts,
  which accept coins below a credit limit, while Olympic Darts v3.11 blinks
  it at the end of each turn, until the next player starts.

  Sport Darts T.V. schematics (Compumatic "YDESUS" CPU board, 1992):

  - The CM3080 gets the UM487F clock on XTAL1, and outputs the CPU clock, an
     H/2 clock (divided by a 4020 for the AY), the Z80 /INT (acknowledged
     with /M1 and /IORQ), and the /NMI and /RESET power supervisor signals.
     It has no horizontal sync input (pins 3 and 5 go to an RC network), so
     its IRQ rate comes from an internal divider. The later K7 "YDESUS/PLUS"
     board replaces it with a 74HC74 dividing the clock and setting /INT on
     every UM487 horizontal retrace (HRET), and 4011 gates for /NMI and
     /RESET.
  - Port 0 and port 1 outputs are 74LS273 latches cleared on reset, port 1
    inputs a 74LS541. The dart board columns and the buttons common are
    driven through inverting MOSFETs (a set bit selects the line, as the K7
    programs do), the coin counter and inhibit lines through BDX33
    transistors and the lamps through ULN2003 drivers. Port 1 bit 7 drives
    the UM487F RESET through an inverter, and port 1 input bit 5 is the
    UM487F red output (ROUT).
  - TODO: the dumped Sport Darts program selects the dart board lines with
    a clear bit, so its board differs from these schematics.
  - A GAL decodes the I/O ports from A0, A1, A8 and A9 only. Another one
    decodes the memory and the ROM banks, and drives the Z80 /WAIT from the
    UM487F IORDY on video memory accesses.
  - The AY outputs are mixed through resistors to a TDA2003 amplifier with a
    volume trimmer.

**************************************************************************

  TODO:

  - IRQ sources (see the machine configurations).
  - Master to video CPU link timing.
  - Master CPU wait states (see the machine configuration).
  - Unknown inputs of the Compumatic boards (9046 port A and some port C/D lines).
  - Outputs of the Compumatic boards (9046 port A, CN1/CN2) and Olympic Darts v3.00 lamps.
  - UM487F IORDY wait states.
  - UM487F RESET and ROUT lines of the K7 / Sport Darts boards.

*************************************************************************/

#include "emu.h"

#include "bus/isa/isa.h"
#include "cpu/z80/z80.h"
#include "machine/i2cmem.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "video/um487f.h"

#include "screen.h"
#include "speaker.h"

#include <algorithm>
#include <iterator>

#include "dartboard.lh"


namespace {

// The UM487F is soldered to the board, which has no ISA bus
class isa8_sysi_video_device : public device_t, public device_isa8_card_interface
{
public:
	isa8_sysi_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto hsync_callback() { return m_video.lookup()->hsync_callback(); }

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	// A15 = 1 selects the CGA video memory window of the UM487F
	uint8_t vram_r(offs_t offset) { return m_video->mem_r(0x8000 + offset); }
	void vram_w(offs_t offset, uint8_t data) { m_video->mem_w(0x8000 + offset, data); }
	rgb_t rgbi_to_rgb(uint8_t rgbi);

	required_device<um487f_device> m_video;
};

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(ISA8_SYSI_VIDEO, isa8_sysi_video_device, isa8_sysi_video_device, "isa_sysi_video", "Compumatic System I UM487F video")


namespace {

class sysi_state : public driver_device
{
protected:
	sysi_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ay(*this, "aysnd")
		, m_eeprom(*this, "eeprom")
	{ }

	void hcga_config(machine_config &config) ATTR_COLD;

	required_device<z80_device> m_maincpu;
	required_device<ay8910_device> m_ay;
	required_device<i2cmem_device> m_eeprom;
};


class _4enlinea_state : public sysi_state
{
public:
	_4enlinea_state(const machine_config &mconfig, device_type type, const char *tag)
		: sysi_state(mconfig, type, tag)
		, m_mastercpu(*this, "mastercpu")
		, m_matrix(*this, "MATRIX%u", 0U)
		, m_buttons(*this, "BUTTONS%u", 0U)
		, m_in_pc(*this, "IN_PC")
		, m_in_pd(*this, "IN_PD")
	{ }

	void _4enlinea(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// master to video CPU link
	uint8_t link_r(offs_t offset);
	uint8_t handshake_status_r();
	void handshake_w(offs_t offset, uint8_t data);
	void handshake_status_w(uint8_t data);
	uint8_t link_status_r();
	void link_control_w(uint8_t data);
	void link_data_w(offs_t offset, uint8_t data);
	void send_to_video(uint8_t data0, uint8_t data1);
	TIMER_CALLBACK_MEMBER(link_word_w);
	TIMER_CALLBACK_MEMBER(link_read_w);
	TIMER_CALLBACK_MEMBER(link_ready);

	// ES2 9046 I/O ports
	uint8_t port_r(offs_t offset);
	void port_w(offs_t offset, uint8_t data);
	uint8_t port_out(int port) const { return m_port_latch[port] | m_port_dir[port]; } // pulled up when set as inputs
	void update_outputs();
	uint8_t ay_porta_r();
	uint8_t ay_portb_r();

	void master_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void main_portmap(address_map &map) ATTR_COLD;

	required_device<z80_device> m_mastercpu;
	required_ioport_array<4> m_matrix;
	required_ioport_array<2> m_buttons;
	required_ioport m_in_pc;
	required_ioport m_in_pd;

	emu_timer *m_link_timer = nullptr;

	uint8_t m_handshake_status = 0;
	uint8_t m_link_latch[2]{};
	uint8_t m_link_data[4]{};
	bool m_link_ready = true;
	uint8_t m_port_latch[4]{};
	uint8_t m_port_dir[4]{};
};


class k7_state : public sysi_state
{
public:
	k7_state(const machine_config &mconfig, device_type type, const char *tag)
		: sysi_state(mconfig, type, tag)
		, m_rombank(*this, "rombank")
		, m_matrix(*this, "MATRIX%u", 0U)
		, m_buttons(*this, "BUTTONS%u", 0U)
		, m_lamp(*this, "lamp0")
	{ }

	void k7_olym(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void io_map(address_map &map) ATTR_COLD;

	void out0_w(uint8_t data);

	uint8_t m_out0 = 0;
	uint8_t m_selected_lines = 0;

private:
	void mem_map(address_map &map) ATTR_COLD;

	void out1_w(uint8_t data);
	uint8_t ay_porta_r();
	uint8_t ay_portb_r();
	void hsync_w(int state);

	required_memory_bank m_rombank;
	required_ioport_array<4> m_matrix;
	required_ioport_array<2> m_buttons;
	output_finder<> m_lamp;

	uint8_t m_hsync_count = 0;
};


class k7olym30_state : public k7_state
{
public:
	k7olym30_state(const machine_config &mconfig, device_type type, const char *tag)
		: k7_state(mconfig, type, tag)
		, m_in1(*this, "IN1")
		, m_in1_col(*this, "IN1_COL")
		, m_in1_alt(*this, "IN1_ALT")
	{ }

	void k7_olym30(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_reset() override ATTR_COLD;

private:
	void k7olym30_io_map(address_map &map) ATTR_COLD;

	uint8_t k7olym30_in1_r();
	void k7olym30_out1_w(uint8_t data);

	required_ioport m_in1;
	required_ioport m_in1_col;
	required_ioport m_in1_alt;
};


class sprtdart_state : public k7_state
{
public:
	sprtdart_state(const machine_config &mconfig, device_type type, const char *tag)
		: k7_state(mconfig, type, tag)
		, m_lamps(*this, "lamp%u", 1U)
	{ }

	void sprtdart(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_reset() override ATTR_COLD;

private:
	void sprtdart_io_map(address_map &map) ATTR_COLD;

	void sprtdart_out0_w(uint8_t data);
	void sprtdart_out1_w(uint8_t data);

	output_finder<4> m_lamps;
};


/***********************************
*      Memory Map Information      *
***********************************/

/*
  Master to video CPU link (ES2 9046 and 8952 CM 32?)

  The master writes each word twice to 0xfc29-0xfc2c (CN1/CN2 half, always 0,
  then command and parameter), strobes 0xfc28 and waits for 0xfc28 bit 3
  before sending the next one. The video CPU gets a NMI for each word.

  0xfc28 bit 3 can't depend on the video CPU reading the word: Dardos keeps
  sending null words while idle, and its video CPU stops reading them while
  its command queue is full (during the boot delay).

  TODO: guessed from the code of both CPUs, transfer time unknown.
*/
void _4enlinea_state::send_to_video(uint8_t data0, uint8_t data1)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(_4enlinea_state::link_word_w), this), (data1 << 8) | data0);
}

TIMER_CALLBACK_MEMBER(_4enlinea_state::link_word_w)
{
	m_link_latch[0] = uint8_t(param);
	m_link_latch[1] = uint8_t(param >> 8);
	m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

TIMER_CALLBACK_MEMBER(_4enlinea_state::link_read_w)
{
	m_handshake_status |= 0x20;
}

TIMER_CALLBACK_MEMBER(_4enlinea_state::link_ready)
{
	m_link_ready = true;
}

uint8_t _4enlinea_state::link_r(offs_t offset)
{
	if (offset == 0 && !machine().side_effects_disabled())
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(_4enlinea_state::link_read_w), this));

	return m_link_latch[offset];
}

uint8_t _4enlinea_state::handshake_status_r()
{
	return m_handshake_status;
}

void _4enlinea_state::handshake_status_w(uint8_t data)
{
	m_handshake_status = data; // probably just clears
}

void _4enlinea_state::handshake_w(offs_t offset, uint8_t data)
{
	if (offset == 0)
		send_to_video(data, 0);
}

uint8_t _4enlinea_state::link_status_r()
{
	return m_link_ready ? 0x08 : 0x00;
}

void _4enlinea_state::link_control_w(uint8_t data)
{
	// the master writes 0 and then 7 after loading a word
	if (!BIT(data, 0))
	{
		m_link_ready = false;
		m_link_timer->adjust(attotime::from_usec(100)); // guess
		send_to_video(m_link_data[3], m_link_data[2]);
	}
}

void _4enlinea_state::link_data_w(offs_t offset, uint8_t data)
{
	m_link_data[offset] = data;
}


// ES2 9046 I/O ports A-D, port A bits 1 and 3 are unknown inputs and port B is unused
uint8_t _4enlinea_state::port_r(offs_t offset)
{
	int const port = offset >> 1;

	if (BIT(offset, 0))
		return m_port_dir[port];

	uint8_t in = 0xff;
	switch (port)
	{
	case 2:
		in = m_in_pc->read();
		break;
	case 3:
		in = m_in_pd->read();
		break;
	}

	return (in & m_port_dir[port]) | (m_port_latch[port] & ~m_port_dir[port]);
}

void _4enlinea_state::port_w(offs_t offset, uint8_t data)
{
	int const port = offset >> 1;

	if (BIT(offset, 0))
		m_port_dir[port] = data;
	else
		m_port_latch[port] = data;

	update_outputs();
}

void _4enlinea_state::update_outputs()
{
	m_eeprom->write_scl(BIT(port_out(3), 6));
	m_eeprom->write_sda(BIT(port_out(2), 0));
}

uint8_t _4enlinea_state::ay_porta_r()
{
	uint8_t const columns = (BIT(port_out(2), 7) << 0) | ((port_out(3) & 0x07) << 1);
	uint8_t data = 0xff;

	for (int i = 0; i < 4; i++)
	{
		if (BIT(columns, i))
			data &= m_matrix[i]->read();
	}

	if (BIT(port_out(3), 7))
		data &= m_buttons[0]->read();

	return data;
}

uint8_t _4enlinea_state::ay_portb_r()
{
	uint8_t const columns = (BIT(port_out(2), 7) << 0) | ((port_out(3) & 0x07) << 1);
	uint8_t data = 0xff;

	for (int i = 0; i < 4; i++)
	{
		if (BIT(columns, i))
			data &= m_matrix[i]->read() >> 8;
	}

	if (BIT(port_out(3), 7))
		data &= m_buttons[1]->read();

	return data;
}

void _4enlinea_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xc000, 0xdfff).ram();

	map(0xe000, 0xe001).r(FUNC(_4enlinea_state::link_r));
}

void _4enlinea_state::main_portmap(address_map &map)
{
	map.global_mask(0x3ff);
}

void _4enlinea_state::master_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xf800, 0xfbff).ram();
	map(0xfc20, 0xfc27).rw(FUNC(_4enlinea_state::port_r), FUNC(_4enlinea_state::port_w));
	map(0xfc28, 0xfc28).rw(FUNC(_4enlinea_state::link_status_r), FUNC(_4enlinea_state::link_control_w));
	map(0xfc29, 0xfc2c).w(FUNC(_4enlinea_state::link_data_w));
	map(0xfc30, 0xfc31).w(FUNC(_4enlinea_state::handshake_w));
	map(0xfc32, 0xfc32).rw(FUNC(_4enlinea_state::handshake_status_r), FUNC(_4enlinea_state::handshake_status_w));
	map(0xfc48, 0xfc48).w(m_ay, FUNC(ay8910_device::address_w));
	map(0xfc49, 0xfc49).r(m_ay, FUNC(ay8910_device::data_r));
	map(0xfc4a, 0xfc4a).w(m_ay, FUNC(ay8910_device::data_w));
}


void k7_state::out0_w(uint8_t data)
{
	m_out0 = data;
	m_rombank->set_entry(data & 0x03);
	m_eeprom->write_scl(BIT(data, 2));
	m_eeprom->write_sda(!BIT(data, 3)); // through an open collector transistor
}

void k7_state::out1_w(uint8_t data)
{
	m_selected_lines = data;
	m_lamp = BIT(data, 5); // guess: NP button lamp
	machine().bookkeeping().coin_counter_w(0, BIT(data, 6));
}

void k7_state::hsync_w(int state)
{
	if (state && !(++m_hsync_count & 0x07))
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
}

uint8_t k7_state::ay_porta_r()
{
	uint8_t data = 0xff;

	for (int i = 0; i < 4; i++)
	{
		if (BIT(m_selected_lines, i))
			data &= m_matrix[i]->read();
	}

	if (BIT(m_selected_lines, 4))
		data &= m_buttons[0]->read();

	return data;
}

uint8_t k7_state::ay_portb_r()
{
	uint8_t data = 0xff;

	for (int i = 0; i < 4; i++)
	{
		if (BIT(m_selected_lines, i))
			data &= m_matrix[i]->read() >> 8;
	}

	if (BIT(m_selected_lines, 4))
		data &= m_buttons[1]->read();

	return data;
}

void k7_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xc000, 0xdfff).bankr(m_rombank);
	map(0xe000, 0xffff).ram().share("nvram");
}

void k7_state::io_map(address_map &map)
{
	map.global_mask(0x3ff);

	map(0x0000, 0x0000).w(FUNC(k7_state::out0_w));
	map(0x0001, 0x0001).portr("IN1").w(FUNC(k7_state::out1_w));
	map(0x0100, 0x0100).w(m_ay, FUNC(ay8910_device::address_w));
	map(0x0101, 0x0101).r(m_ay, FUNC(ay8910_device::data_r));
	map(0x0102, 0x0102).w(m_ay, FUNC(ay8910_device::data_w));
}


uint8_t k7olym30_state::k7olym30_in1_r()
{
	if (BIT(m_out0, 7))
		return m_in1_alt->read();

	uint8_t data = m_in1->read();
	if (BIT(m_selected_lines, 4))
		data = (data & ~0x0c) | (m_in1_col->read() & 0x0c);

	return data;
}

void k7olym30_state::k7olym30_out1_w(uint8_t data)
{
	m_selected_lines = data;
	machine().bookkeeping().coin_lockout_global_w(!BIT(data, 5));
	machine().bookkeeping().coin_counter_w(0, BIT(data, 6));
}

void k7olym30_state::k7olym30_io_map(address_map &map)
{
	io_map(map);
	map(0x0001, 0x0001).rw(FUNC(k7olym30_state::k7olym30_in1_r), FUNC(k7olym30_state::k7olym30_out1_w));
}


void sprtdart_state::sprtdart_out0_w(uint8_t data)
{
	out0_w(data);

	for (int i = 0; i < 4; i++)
		m_lamps[i] = BIT(data, 4 + i);
}

void sprtdart_state::sprtdart_out1_w(uint8_t data)
{
	m_selected_lines = ~data;
	machine().bookkeeping().coin_lockout_global_w(!BIT(data, 5));
	machine().bookkeeping().coin_counter_w(0, BIT(data, 6));
}

void sprtdart_state::sprtdart_io_map(address_map &map)
{
	io_map(map);
	map(0x0000, 0x0000).w(FUNC(sprtdart_state::sprtdart_out0_w));
	map(0x0001, 0x0001).w(FUNC(sprtdart_state::sprtdart_out1_w));
}


/***********************************
*           Input Ports            *
***********************************/

static INPUT_PORTS_START( 4enlinea )
	PORT_START("BUTTONS0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY  PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY  PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY  PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY  PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )                   PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )                   PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Service Credit")

	PORT_START("BUTTONS1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY  PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY  PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY  PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY  PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )                   PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )                   PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_TOGGLE

	// no dart board
	PORT_START("MATRIX0")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("MATRIX1")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("MATRIX2")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("MATRIX3")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	// each coin line adds a credit (with its own counter)
	PORT_START("IN_PC")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(i2cmem_device::read_sda))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN5 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN_PD")
	PORT_BIT( 0x18, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xe7, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


// the dartboard layout finds the targets by these names
static INPUT_PORTS_START( k7_matrix )
	PORT_START("MATRIX0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Single 3")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Double 3")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Single 19")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Double 19")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Single 7")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Double 7")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Single 16")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Double 16")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Double 17")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Single 17")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Triple 16")
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Triple 7")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Triple 19")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Triple 3")
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Triple 17")

	PORT_START("MATRIX1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Single 6")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Double 6")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Single 10")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Double 10")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Single 15")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Double 15")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Single 2")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Double 2")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Double 13")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Single 13")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Double Bull")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Triple 2")
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Triple 15")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Triple 10")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Triple 6")
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Triple 13")

	PORT_START("MATRIX2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Single 20")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Double 20")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Single 1")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Double 1")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Single 18")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Double 18")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Single 4")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Double 4")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Double 5")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Single 5")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Single Bull")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Triple 4")
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Triple 18")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Triple 1")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Triple 20")
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Triple 5")

	PORT_START("MATRIX3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Single 11")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Double 11")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Single 14")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Double 14")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Single 9")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Double 9")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Single 12")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Double 12")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Double 8")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Single 8")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Triple 12")
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Triple 9")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Triple 14")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Triple 11")
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Triple 8")
INPUT_PORTS_END

static INPUT_PORTS_START( k7_olym )
	PORT_INCLUDE( k7_matrix )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_NAME("Coin 1 (500 Pts)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("Coin 2 (100 Pts)")
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNUSED ) // coin selector lines 2-3, ignored by the games
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(i2cmem_device::read_sda))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED ) // UM487F ROUT (not emulated), the boot code reinitializes the UM487F until it reads low
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Missed Dart Sensor") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Player Sensor") PORT_CODE(KEYCODE_S)

	PORT_START("BUTTONS0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Up")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Down")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("NP (Start / Next Player)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Player")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Reset (Clear Credits)")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("BUTTONS1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( dardos )
	PORT_INCLUDE( k7_matrix )

	// the game calls the buttons FLECHA ARRIBA, FLECHA ABAJO, OK and PLAY
	PORT_START("BUTTONS0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Up")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Down")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("OK")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Play")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_TOGGLE // only read at power on
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Reset (Clear Credits)")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("BUTTONS1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN_PC")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(i2cmem_device::read_sda))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_NAME("Coin 1 (500 Pts)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("Coin 2 (100 Pts)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_NAME("Coin 3 (50 Pts)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_NAME("Coin 4 (200 Pts)")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN5 ) // credited as 800 Pts, but no such coin
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Player Sensor") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN_PD")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Missed Dart Sensor") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0xe7, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

// named as on the edge connector
static INPUT_PORTS_START( sprtdart )
	PORT_INCLUDE( k7_olym )

	PORT_MODIFY("BUTTONS0")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("OK")
INPUT_PORTS_END

// Olympic Darts v3.00 PCB, with a button (and lamp) for each game and number of players
static INPUT_PORTS_START( k7_olym30 )
	PORT_INCLUDE( k7_olym )

	PORT_MODIFY("BUTTONS0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("High Score / Down")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Shanghai / Up")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Scram / Player")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Roulette")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("NP (Start / Next Player)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Tres en Raya")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Cricket")

	PORT_MODIFY("BUTTONS1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("301")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("501")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("Double In / Cut Throat")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("Double Out / Team")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1 Player") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2 Players") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3 Players") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("4 Players") PORT_CODE(KEYCODE_4_PAD)

	PORT_START("IN1_COL")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("5 Players") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("6 Players") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0xf3, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1_ALT")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_NAME("Ahorcado")
	PORT_BIT( 0xfb, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


/****************************************
*          Machine Start/Reset          *
****************************************/

void _4enlinea_state::machine_start()
{
	m_link_timer = timer_alloc(FUNC(_4enlinea_state::link_ready), this);

	save_item(NAME(m_handshake_status));
	save_item(NAME(m_link_latch));
	save_item(NAME(m_link_data));
	save_item(NAME(m_link_ready));
	save_item(NAME(m_port_latch));
	save_item(NAME(m_port_dir));
}

void _4enlinea_state::machine_reset()
{
	std::fill(std::begin(m_port_dir), std::end(m_port_dir), 0xff);
	update_outputs();

	m_link_ready = true;
	m_link_timer->adjust(attotime::never);
}

void k7_state::machine_start()
{
	m_rombank->configure_entries(0, 4, memregion("maincpu")->base() + 0x8000, 0x2000);
	m_rombank->set_entry(0);

	save_item(NAME(m_out0));
	save_item(NAME(m_selected_lines));
	save_item(NAME(m_hsync_count));
}

void k7_state::machine_reset()
{
	out0_w(0);
	out1_w(0);
}

void k7olym30_state::machine_reset()
{
	k7_state::machine_reset();
	k7olym30_out1_w(0);
}

void sprtdart_state::machine_reset()
{
	k7_state::machine_reset();
	sprtdart_out0_w(0);
	sprtdart_out1_w(0);
}


/***********************************
*          Video Hardware          *
***********************************/

isa8_sysi_video_device::isa8_sysi_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ISA8_SYSI_VIDEO, tag, owner, clock)
	, device_isa8_card_interface(mconfig, *this)
	, m_video(*this, "um487f")
{
}

void isa8_sysi_video_device::device_add_mconfig(machine_config &config)
{
	// the UM487F reconfigures it from its CRTC registers
	screen_device &screen(SCREEN(config, "screen"));
	screen.set_raw(14.318181_MHz_XTAL / 2, 456, 0, 320, 262, 0, 200);
	screen.set_screen_update(m_video, FUNC(um487f_device::screen_update));

	UM487F(config, m_video, 14.318181_MHz_XTAL); // MOSC (MGA clock) tied to GND
	m_video->set_screen("screen");
	m_video->set_rgbi_callback(FUNC(isa8_sysi_video_device::rgbi_to_rgb));
}

void isa8_sysi_video_device::device_start()
{
	set_isa_device();

	m_isa->install_memory(0x8000, 0xbfff, read8sm_delegate(*this, FUNC(isa8_sysi_video_device::vram_r)), write8sm_delegate(*this, FUNC(isa8_sysi_video_device::vram_w)));
	m_isa->install_device(0x03b0, 0x03df, read8sm_delegate(*m_video, FUNC(um487f_device::io_r)), write8sm_delegate(*m_video, FUNC(um487f_device::io_w)));
}

/*
  RGB output stage (Sport Darts and K7 schematics): each UM487F color output
  drives the monitor through a 100 ohm resistor, loaded with another 100 ohm
  resistor by a 2N2369 switched by IOUT. None of the games use intensified
  colors, and real Sport Darts screens show yellow instead of the CGA brown.
  TODO: the dual CPU boards have the transistor footprints bypassed by
  jumpers, assume the same levels.
*/
rgb_t isa8_sysi_video_device::rgbi_to_rgb(uint8_t rgbi)
{
	uint8_t const level = BIT(rgbi, 3) ? 0x80 : 0xff; // guess: half level when loaded
	return rgb_t(BIT(rgbi, 2) ? level : 0, BIT(rgbi, 1) ? level : 0, BIT(rgbi, 0) ? level : 0);
}


/***********************************
*         Machine Drivers          *
***********************************/

void sysi_isa8_cards(device_slot_interface &device)
{
	device.option_add_internal("sysi_video", ISA8_SYSI_VIDEO);
}

void sysi_state::hcga_config(machine_config &config)
{
	isa8_device &isa(ISA8(config, "isa"));
	isa.set_memspace(m_maincpu, AS_PROGRAM);
	isa.set_iospace(m_maincpu, AS_IO);

	ISA8_SLOT(config, "board1", 0, "isa", sysi_isa8_cards, "sysi_video", true);
}

void _4enlinea_state::_4enlinea(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 16_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &_4enlinea_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &_4enlinea_state::main_portmap);
	/* TODO: IRQ sources are unknown.
	   All these boards run their main IRQ routine around 1 kHz:
	   - The video CPU runs its software tick every 20 IRQs, and expects it
	     around 50 Hz: it shows the boot logo for 300 ticks, reprograms the
	     UM487F every 50 ticks...
	   - The master CPU runs its tick every 10 IRQs, and counts the game time
	     seconds every 100 ticks.
	   - Sport Darts runs its IRQ routine every 16 horizontal syncs.
	   The CM3080 gets the 16 MHz crystal and drives the video CPU /INT, the
	   master one probably comes from the 9046. */
	m_maincpu->set_periodic_int(FUNC(_4enlinea_state::irq0_line_assert), attotime::from_hz(16_MHz_XTAL / 16384));
	m_maincpu->irqack_cb().set_inputline(m_maincpu, INPUT_LINE_IRQ0, CLEAR_LINE); // the CM3080 gets /M1

	Z80(config, m_mastercpu, 8_MHz_XTAL / 2); // measured
	m_mastercpu->set_addrmap(AS_PROGRAM, &_4enlinea_state::master_map);
	m_mastercpu->set_periodic_int(FUNC(_4enlinea_state::irq0_line_hold), attotime::from_hz(8_MHz_XTAL / 8192));
	/* guess: one wait state for every memory access.
	   Without it, Dardos registers a dart every few milliseconds while a
	   dart board sector is held down, instead of showing "SECTOR PISADO"
	   (stuck sector) after half a second as the real board does: the main
	   loop discards the sector when it doesn't get a new matrix scan from
	   the IRQ handler since the previous pass, so it must be slower than the
	   IRQ period. */
	m_mastercpu->z80_set_m1_cycles(4 + 1);
	m_mastercpu->z80_set_mreq_cycles(3 + 1);

	I2C_24C16(config, m_eeprom); // X24C16P

	// video hardware
	hcga_config(config);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	AY8910(config, m_ay, 8_MHz_XTAL / 4); // measured
	m_ay->port_a_read_callback().set(FUNC(_4enlinea_state::ay_porta_r));
	m_ay->port_b_read_callback().set(FUNC(_4enlinea_state::ay_portb_r));
	m_ay->add_route(ALL_OUTPUTS, "mono", 0.50);
}


void k7_state::k7_olym(machine_config &config)
{
	Z80(config, m_maincpu, 14.318181_MHz_XTAL / 2); // Z84C00BB6
	m_maincpu->set_addrmap(AS_PROGRAM, &k7_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &k7_state::io_map);
	m_maincpu->irqack_cb().set_inputline(m_maincpu, INPUT_LINE_IRQ0, CLEAR_LINE);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // D4464C-15L (6264) + battery

	I2C_24C16(config, m_eeprom); // X24C16P

	hcga_config(config);

	/* TODO: IRQ source is unknown.
	   The game runs its IRQ routine every 2 IRQs and its tick every 10 of
	   them, with the same timings as Sport Darts (IRQ routine every 16
	   horizontal syncs) and the Compumatic master CPU (100 ticks per second).
	   The CM3080 subboard has two HEF4020 ripple counters. */
	subdevice<isa8_slot_device>("board1")->set_option_machine_config("sysi_video", [this] (device_t *device)
	{
		downcast<isa8_sysi_video_device &>(*device).hsync_callback().set(*this, FUNC(k7_state::hsync_w));
	});

	SPEAKER(config, "mono").front_center();
	AY8910(config, m_ay, 14.318181_MHz_XTAL / 8); // Winbond WF19054
	m_ay->port_a_read_callback().set(FUNC(k7_state::ay_porta_r));
	m_ay->port_b_read_callback().set(FUNC(k7_state::ay_portb_r));
	m_ay->add_route(ALL_OUTPUTS, "mono", 0.50);
}

void k7olym30_state::k7_olym30(machine_config &config)
{
	k7_olym(config);

	m_maincpu->set_addrmap(AS_IO, &k7olym30_state::k7olym30_io_map);
}

void sprtdart_state::sprtdart(machine_config &config)
{
	k7_olym(config);

	m_maincpu->set_addrmap(AS_IO, &sprtdart_state::sprtdart_io_map);

	/* guess: an IRQ every horizontal sync.
	   The CM3080 derives /INT from the UM487F clock, it has no horizontal sync
	   input, but the IRQ handler counts the IRQs from the vertical retrace to
	   change the background color at a given raster line. */
	subdevice<isa8_slot_device>("board1")->set_option_machine_config("sysi_video", [this] (device_t *device)
	{
		downcast<isa8_sysi_video_device &>(*device).hsync_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0, ASSERT_LINE); // TODO: polarity
	});
}


/***********************************
*             Rom Load             *
***********************************/

ROM_START( 4enlinea )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cuatro_en_linea_27c256__cicplay-2.ic6",  0x0000, 0x8000, CRC(f8f14bf8) SHA1(e48fbedbd1b9be6fb56a0f65db80eddbedb487c7) )

	ROM_REGION( 0x10000, "mastercpu", 0 )
	ROM_LOAD( "cuatro_en_linea_27c256__cicplay-1.ic19", 0x0000, 0x8000, CRC(307a57a3) SHA1(241329d919ec43d0eeb1dad0a4db6cf6de06e7e1) )

	// the game only reads the settings (game time and accounting) and doesn't initialize them
	ROM_REGION( 0x0800, "eeprom", 0 )
	ROM_LOAD( "cuatro_en_linea_x24c16p_handcrafted.ic17", 0x0000, 0x0800, BAD_DUMP CRC(f07b9347) SHA1(4653793216fdb10d5cc8657e7f1b1479949c58e5) ) // handcrafted: 2:00 game time, counters cleared

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "cuatro_en_linea_gal16v8as__nosticker.ic04", 0x0000, 0x0117, NO_DUMP )
ROM_END

ROM_START( 4enlineb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cuatro_en_linea_2_a06.ic6",  0x0000, 0x8000, CRC(f8f14bf8) SHA1(e48fbedbd1b9be6fb56a0f65db80eddbedb487c7) )

	ROM_REGION( 0x10000, "mastercpu", 0 )
	ROM_LOAD( "cuatro_en_linea_1_a06.ic19", 0x0000, 0x8000, CRC(993d0581) SHA1(d6e366dd827543508037d2071c4b6e638c2cf87b) )

	// the game only reads the settings (game time and accounting) and doesn't initialize them
	ROM_REGION( 0x0800, "eeprom", 0 )
	ROM_LOAD( "cuatro_en_linea_x24c16p_handcrafted.ic17", 0x0000, 0x0800, BAD_DUMP CRC(f07b9347) SHA1(4653793216fdb10d5cc8657e7f1b1479949c58e5) ) // handcrafted: 2:00 game time, counters cleared

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "cuatro_en_linea_gal16v8a.ic04", 0x0000, 0x0117, NO_DUMP )
ROM_END

/*
  Dardos
  Oper Coin. 1991.
  Running in 487 System I.
*/
ROM_START( dardos )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "diana_iv_video_27-1-92.bin",  0x0000, 0x8000, CRC(f23b5313) SHA1(488cf9bedce7b0c7b474bd93da70181c81fa300b) )

	ROM_REGION( 0x10000, "mastercpu", 0 )
	ROM_LOAD( "diana_iv_master_27-1-92.bin", 0x0000, 0x8000, CRC(4b2c868a) SHA1(91120a32fac9c5a6e7746d2e2587921f7d42eaa3) )
ROM_END


/* Kursaal K7 Olympic Darts PCB
    __________________________________________________      SUBBOARD CM3080
    |           ________  __   ______  ______________ |     ________________
    |  _______  | DB9   | |_| |_CN8__| |____CN7______||     |___ __________ |
    |  |______| |_______| CN9                         |__   ||  ||HEF4020BP||
    | ________                                         __|  ||A | _________ |
    | |D41464C|                                        __|  ||  | |________||
    | ________                           _____         __|  ||__| _________ |
    | |D41464C|                         DA741CN        __|  |     |TC4011BP||
    | ________                         _______    ___  __|  |    __________ |
 IC4->|GAL16V8|  _______              HCF4069UBE  XT5  __|  |    |__EMPTY__||
    | ________   |UMC   |  ________   _______________  __|  |    __________ |
IC11->|GAL16V8|  |UM487F|  74HC273AP  |WF19054       | __|  |    |HEF4020BP||
    |            |______|  ________   |______________| __|  |_______________|
    | ________             74HC273AP  _______________  __|    A=74LS368ANA
    | |74LS04N|           ___________ |Z84C00BB6     | __|
    |  _____              | SUBBOARD ||______________| __|
    |  |XT2_|<-14.31818MHz| CM3080   |____________     __|
    |           ________  |          ||M27C512 ROM|    __|
    |          HCF4069UBE |          ||___________|    __|
    |                     |          |____________     __|
    |                     |          ||D4464C-15L |    __|
    |                     |__________||___________|    __|
    | 7808CT    ________    ________  _____   _____   |
    |           |_______|  74LS541B1 X24C16P  |BATT|  |
    | _____  ___________  _____________       |____|  |
    | |CN1_| |__CN5_____| |__CN4_______|              |
    |_________________________________________________|
*/
ROM_START( k7_olym )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "odk7_v3.11_27c512.ic18", 0x00000, 0x10000, CRC(063d24fe) SHA1(ad4509438d2028ede779f5aa9a918d1020c1db41) )

	ROM_REGION( 0x0300, "plds", 0 )
	ROM_LOAD( "a1_gal16v8a.ic11", 0x0000, 0x0117, NO_DUMP ) // protected
	ROM_LOAD( "b1_gal16v8a.ic4",  0x0117, 0x0117, NO_DUMP ) // protected
ROM_END

ROM_START( k7_olym30 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dardos_k7_3.0_21-11-94_27c512.ic19", 0x00000, 0x10000,  CRC(87af55a6) SHA1(7d12ce7afe8a50ba895f05029c1bd05a3641f7fd) )

	ROM_REGION( 0x0300, "plds", 0 )
	ROM_LOAD( "a1_gal16v8a.ic11", 0x0000, 0x0117, NO_DUMP ) // protected
	ROM_LOAD( "b1_gal16v8a.ic4",  0x0117, 0x0117, NO_DUMP ) // protected
ROM_END


ROM_START( sprtdart )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sport_dart_27c512.ic19", 0x00000, 0x10000, CRC(6c9ae27f) SHA1(92fbdef7747a9096daf4714f45b119ad8f3a1436) )

	ROM_REGION( 0x0300, "plds", 0 )
	ROM_LOAD( "gal16v8a.ic11", 0x0000, 0x0117, NO_DUMP ) // protected
	ROM_LOAD( "gal16v8a.ic4",  0x0117, 0x0117, NO_DUMP ) // protected
ROM_END


} // anonymous namespace


/***********************************
*           Game Drivers           *
***********************************/

//    YEAR  NAME       PARENT    MACHINE    INPUT      CLASS            INIT        ROT   COMPANY                                      FULLNAME                       FLAGS
GAME( 1991, 4enlinea,  0,        _4enlinea, 4enlinea,  _4enlinea_state, empty_init, ROT0, "Compumatic / CIC Play",                     "Cuatro en Linea (rev. A-07)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, 4enlineb,  4enlinea, _4enlinea, 4enlinea,  _4enlinea_state, empty_init, ROT0, "Compumatic / CIC Play",                     "Cuatro en Linea (rev. A-06)", MACHINE_SUPPORTS_SAVE )
GAMEL(1992, dardos,    0,        _4enlinea, dardos,    _4enlinea_state, empty_init, ROT0, "Oper Coin",                                 "Dardos",                      MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE, layout_dartboard )
GAMEL(1994, k7_olym,   0,        k7_olym,   k7_olym,   k7_state,        empty_init, ROT0, "K7 Kursaal / NMI Electronics",              "Olympic Darts K7 (v3.11)",    MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE, layout_dartboard )
GAMEL(1994, k7_olym30, k7_olym,  k7_olym30, k7_olym30, k7olym30_state,  empty_init, ROT0, "K7 Kursaal / NMI Electronics",              "Olympic Darts K7 (v3.00)",    MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE, layout_dartboard )
GAMEL(1993, sprtdart,  0,        sprtdart,  sprtdart,  sprtdart_state,  empty_init, ROT0, "Compumatic / Desarrollos y Recambios S.L.", "Sport Darts T.V.",            MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE, layout_dartboard )
