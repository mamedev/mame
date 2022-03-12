// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    BBN BitGraph -- monochrome, raster graphics (768x1024), serial terminal.

    Apparently had at least four hardware revisions, A-D, but which ROM
    revisions support which hardware is unclear.  A Versabus slot, and
    various hardware and software options are mentioned in the docs.  Best
    guesses follow.

    Onboard hardware (common to all revisions) is
    - 32K ROM
    - 128K RAM (includes frame buffer)
    - 3 serial ports, each driven by 6850 ACIA
    - unknown baud rate generator (possibly COM8016)
    - sync serial port, driven by 6854 but apparently never supported by ROM
    - 682x PIA
    - AY-3-891x PSG
    - ER2055 EAROM
    - DEC VT100 keyboard interface

    Rev A has additional 4th serial port for mouse (not supported by ROM 1.25).
    Rev A has 40 hz realtime clock, the rest use 1040 hz.
    Rev A-C use AY-3-8912 (with one external PIO port, to connect the EAROM).
    Rev D uses AY-3-8913 (no external ports; EAROM is wired to TBD).
    Rev B-D have onboard 8035 to talk to parallel printer and mouse.
    Rev B-D have more memory (at least up to 512K).

    ROM 1.25 doesn't support mouse, setup mode, pixel data upload and autowrap.

    Missing/incorrect emulation:
        Bidirectional keyboard interface (to drive LEDs and speaker).
        8035.
        EAROM.
        1.25 only -- clksync() is dummied out -- causes watchdog resets.
        Selectable memory size.
        Video enable/reverse video switch.

****************************************************************************/

#include "emu.h"

#include "bus/centronics/ctronics.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/6821pia.h"
#include "machine/6850acia.h"
#include "machine/clock.h"
#include "machine/com8116.h"
#include "machine/er2055.h"
#include "machine/i8243.h"
#include "machine/mc6854.h"
#include "machine/ram.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


#define M68K_TAG "maincpu"
#define PPU_TAG "ppu"

#define ACIA0_TAG "acia0"
#define ACIA1_TAG "acia1"
#define ACIA2_TAG "acia2"
#define ACIA3_TAG "acia3"
#define RS232_H_TAG "rs232host"
#define RS232_K_TAG "rs232kbd"
#define RS232_D_TAG "rs232debug"
#define RS232_M_TAG "rs232mouse"
#define ADLC_TAG "adlc"
#define PIA_TAG "pia"
#define PSG_TAG "psg"
#define EAROM_TAG "earom"


//#define LOG_GENERAL (1U <<  0) //defined in logmacro.h already
#define LOG_PIA       (1U <<  1)
#define LOG_DEBUG     (1U <<  2)

//#define VERBOSE (LOG_DEBUG)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGPIA(...) LOGMASKED(LOG_PIA, __VA_ARGS__)
#define LOGDBG(...) LOGMASKED(LOG_DEBUG, __VA_ARGS__)


namespace {

class bitgraph_state : public driver_device
{
public:
	bitgraph_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, M68K_TAG)
		, m_ram(*this, RAM_TAG)
		, m_acia0(*this, ACIA0_TAG)
		, m_acia1(*this, ACIA1_TAG)
		, m_acia2(*this, ACIA2_TAG)
		, m_acia3(*this, ACIA3_TAG)
		, m_adlc(*this, ADLC_TAG)
		, m_dbrga(*this, "com8116_a")
		, m_dbrgb(*this, "com8116_b")
		, m_pia(*this, PIA_TAG)
		, m_psg(*this, PSG_TAG)
		, m_earom(*this, EAROM_TAG)
		, m_centronics(*this, "centronics")
		, m_screen(*this, "screen")
	{ }

	static constexpr feature_type imperfect_features() { return feature::KEYBOARD; }

	void bitgrpha(machine_config &config);
	void bitgrphb(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	uint8_t pia_r(offs_t offset);
	void pia_w(offs_t offset, uint8_t data);
	uint8_t pia_pa_r();
	uint8_t pia_pb_r();
	void pia_pa_w(uint8_t data);
	void pia_pb_w(uint8_t data);
	DECLARE_READ_LINE_MEMBER(pia_ca1_r);
	DECLARE_WRITE_LINE_MEMBER(pia_cb2_w);

	void baud_write(uint16_t data);
	DECLARE_WRITE_LINE_MEMBER(com8116_a_fr_w);
	DECLARE_WRITE_LINE_MEMBER(com8116_a_ft_w);
	DECLARE_WRITE_LINE_MEMBER(com8116_b_fr_w);
	DECLARE_WRITE_LINE_MEMBER(com8116_b_ft_w);

	uint8_t adlc_r(offs_t offset);
	void adlc_w(offs_t offset, uint8_t data);

	void earom_write(uint8_t data);
	void misccr_write(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(system_clock_write);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	[[maybe_unused]] uint8_t ppu_read(offs_t offset);
	void ppu_write(offs_t offset, uint8_t data);
	template <unsigned Offset> void ppu_i8243_w(uint8_t data);

	void bg_motherboard(machine_config &config);
	[[maybe_unused]] void bg_ppu(machine_config &config);

	void bitgrapha_mem(address_map &map);
	void bitgraphb_mem(address_map &map);
	void ppu_io(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<acia6850_device> m_acia0;
	required_device<acia6850_device> m_acia1;
	required_device<acia6850_device> m_acia2;
	optional_device<acia6850_device> m_acia3;
	optional_device<mc6854_device> m_adlc;
	required_device<com8116_device> m_dbrga;
	required_device<com8116_device> m_dbrgb;
	required_device<pia6821_device> m_pia;
	required_device<ay8912_device> m_psg;
	required_device<er2055_device> m_earom;
	optional_device<centronics_device> m_centronics;
	required_device<screen_device> m_screen;

	uint8_t *m_videoram;
	uint8_t m_misccr;
	uint8_t m_pia_a;
	uint8_t m_pia_b;
	uint8_t m_ppu[4];
};

void bitgraph_state::bitgrapha_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x007fff).rom();
	map(0x010000, 0x010000).rw(m_acia0, FUNC(acia6850_device::data_r), FUNC(acia6850_device::data_w));   // HOST
	map(0x010002, 0x010002).rw(m_acia0, FUNC(acia6850_device::status_r), FUNC(acia6850_device::control_w));
	map(0x010009, 0x010009).rw(m_acia1, FUNC(acia6850_device::data_r), FUNC(acia6850_device::data_w));   // KEYBOARD
	map(0x01000b, 0x01000b).rw(m_acia1, FUNC(acia6850_device::status_r), FUNC(acia6850_device::control_w));
	map(0x010011, 0x010011).rw(m_acia2, FUNC(acia6850_device::data_r), FUNC(acia6850_device::data_w));   // DEBUGGER
	map(0x010013, 0x010013).rw(m_acia2, FUNC(acia6850_device::status_r), FUNC(acia6850_device::control_w));
	map(0x010019, 0x010019).rw(m_acia3, FUNC(acia6850_device::data_r), FUNC(acia6850_device::data_w));   // POINTER
	map(0x01001b, 0x01001b).rw(m_acia3, FUNC(acia6850_device::status_r), FUNC(acia6850_device::control_w));
	map(0x010020, 0x010027).rw(FUNC(bitgraph_state::adlc_r), FUNC(bitgraph_state::adlc_w)).umask16(0xff00);
	map(0x010028, 0x01002f).rw(FUNC(bitgraph_state::pia_r), FUNC(bitgraph_state::pia_w)).umask16(0xff00);    // EAROM, PSG
	map(0x010030, 0x010031).w(FUNC(bitgraph_state::baud_write));
	map(0x3e0000, 0x3fffff).ram();
}

void bitgraph_state::bitgraphb_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x007fff).rom();
	map(0x010000, 0x010000).rw(m_acia0, FUNC(acia6850_device::data_r), FUNC(acia6850_device::data_w));   // HOST
	map(0x010002, 0x010002).rw(m_acia0, FUNC(acia6850_device::status_r), FUNC(acia6850_device::control_w));
	map(0x010009, 0x010009).rw(m_acia1, FUNC(acia6850_device::data_r), FUNC(acia6850_device::data_w));   // KEYBOARD
	map(0x01000b, 0x01000b).rw(m_acia1, FUNC(acia6850_device::status_r), FUNC(acia6850_device::control_w));
	map(0x010011, 0x010011).rw(m_acia2, FUNC(acia6850_device::data_r), FUNC(acia6850_device::data_w));   // DEBUGGER
	map(0x010013, 0x010013).rw(m_acia2, FUNC(acia6850_device::status_r), FUNC(acia6850_device::control_w));
	map(0x01001b, 0x01001b).w(FUNC(bitgraph_state::misccr_write));
	map(0x010020, 0x010027).rw(FUNC(bitgraph_state::adlc_r), FUNC(bitgraph_state::adlc_w)).umask16(0xff00);
	map(0x010028, 0x01002f).rw(FUNC(bitgraph_state::pia_r), FUNC(bitgraph_state::pia_w)).umask16(0xff00);    // EAROM, PSG
	map(0x010030, 0x010031).w(FUNC(bitgraph_state::baud_write));
//  map(0x010030, 0x010037).r(FUNC(bitgraph_state::ppu_read)).umask16(0x00ff);
	map(0x010038, 0x01003f).w(FUNC(bitgraph_state::ppu_write)).umask16(0x00ff);
	map(0x380000, 0x3fffff).ram();
}

static INPUT_PORTS_START(bitgraph)
INPUT_PORTS_END

static DEVICE_INPUT_DEFAULTS_START( kbd_rs232_defaults )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_300 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_300 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
	DEVICE_INPUT_DEFAULTS( "FLOW_CONTROL", 0x01, 0x01 )
DEVICE_INPUT_DEFAULTS_END


uint8_t bitgraph_state::pia_r(offs_t offset)
{
	LOGPIA("PIA R %d\n", offset);
	return m_pia->read(3 - offset);
}

void bitgraph_state::pia_w(offs_t offset, uint8_t data)
{
	LOGPIA("PIA W %d < %02X\n", offset, data);
	return m_pia->write(3 - offset, data);
}

READ_LINE_MEMBER(bitgraph_state::pia_ca1_r)
{
	return m_screen->frame_number() & 1;
}

WRITE_LINE_MEMBER(bitgraph_state::pia_cb2_w)
{
	// no-op to shut up verbose log
}

uint8_t bitgraph_state::pia_pa_r()
{
	uint8_t data = BIT(m_pia_b, 3) ? m_earom->data() : m_pia_a;
	LOGDBG("PIA A == %02X (%s)\n", data, BIT(m_pia_b, 3) ? "earom" : "pia");
	return data;
}

void bitgraph_state::pia_pa_w(uint8_t data)
{
	LOGDBG("PIA A <- %02X\n", data);
	m_pia_a = data;
}

/*
    B0  O: BC1  to noisemaker.
    B1  O: BDIR to noisemaker.
    B2  O: Clock for EAROM.
    B3  O: CS1   for EAROM.
    B4  O: Enable HDLC Xmt interrupt.
    B5  O: Enable HDLC Rcv interrupt.
    B6  O: Clear Clock interrupt.  Must write a 0 [clear interrupt], then a 1.
    B7  I: EVEN field ??
*/
uint8_t bitgraph_state::pia_pb_r()
{
	LOGDBG("PIA B == %02X\n", m_pia_b);
	return m_pia_b;
}

void bitgraph_state::pia_pb_w(uint8_t data)
{
	LOGDBG("PIA B <- %02X\n", data);
	m_pia_b = data;

	switch (m_pia_b & 0x03)
	{
	case 2:
		m_psg->data_w(m_pia_a);
		break;
	case 3:
		m_psg->address_w(m_pia_a);
		break;
	}

	if (BIT(m_pia_b, 3))
	{
		LOGDBG("EAROM data <- %02X\n", m_pia_a);
		m_earom->set_data(m_pia_a);
	}
	// CS1, ~CS2, C1, C2
	m_earom->set_control(BIT(m_pia_b, 3), BIT(m_pia_b, 3), BIT(m_pia_a, 6), BIT(m_pia_a, 7));
	m_earom->set_clk(BIT(m_pia_b, 2));

	if (!BIT(m_pia_b, 6))
	{
		m_maincpu->set_input_line(M68K_IRQ_6, CLEAR_LINE);
	}
}

void bitgraph_state::earom_write(uint8_t data)
{
	LOGDBG("EAROM addr <- %02X (%02X)\n", data & 0x3f, data);
	m_earom->set_address(data & 0x3f);
}

// written once and never changed
void bitgraph_state::misccr_write(uint8_t data)
{
	LOG("MISCCR <- %02X (DTR %d MAP %d)\n", data, BIT(data, 3), (data & 3));
	m_misccr = data;
}

WRITE_LINE_MEMBER(bitgraph_state::system_clock_write)
{
	if (!BIT(m_pia_b, 6))
	{
		m_maincpu->set_input_line(M68K_IRQ_6, CLEAR_LINE);
		return;
	}
	if (state)
	{
		m_maincpu->set_input_line(M68K_IRQ_6, ASSERT_LINE);
	}
	else
	{
		m_maincpu->set_input_line(M68K_IRQ_6, CLEAR_LINE);
	}
}

// rev A writes EA5E -- 9600 HOST, 2400 PNT, 300 KBD, 9600 DBG
// rev B writes EE5E -- 9600 HOST, 9600 PNT, 300 KBD, 9600 DBG
void bitgraph_state::baud_write(uint16_t data)
{
	LOG("Baud %04X\n", data);
	m_dbrgb->str_w(data & 15);      // 2 DBG
	m_dbrga->stt_w((data >> 4) & 15);   // 1 KBD
	m_dbrgb->stt_w((data >> 8) & 15);   // 3 PNT
	m_dbrga->str_w((data >> 12) & 15);  // 0 HOST
}

WRITE_LINE_MEMBER(bitgraph_state::com8116_a_fr_w)
{
	m_acia0->write_txc(state);
	m_acia0->write_rxc(state);
}

WRITE_LINE_MEMBER(bitgraph_state::com8116_a_ft_w)
{
	m_acia1->write_txc(state);
	m_acia1->write_rxc(state);
}

WRITE_LINE_MEMBER(bitgraph_state::com8116_b_fr_w)
{
	m_acia2->write_txc(state);
	m_acia2->write_rxc(state);
}

WRITE_LINE_MEMBER(bitgraph_state::com8116_b_ft_w)
{
	if (m_acia3.found())
	{
		m_acia3->write_txc(state);
		m_acia3->write_rxc(state);
	}
}

uint8_t bitgraph_state::adlc_r(offs_t offset)
{
	LOG("ADLC R %d\n", offset);
	return m_adlc.found() ? m_adlc->read(3 - offset) : 0xff;
}

void bitgraph_state::adlc_w(offs_t offset, uint8_t data)
{
	LOG("ADLC W %d < %02X\n", offset, data);
	if (m_adlc.found()) return m_adlc->write(3 - offset, data);
}

uint32_t bitgraph_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int y = 0; y < 768; y++)
	{
		uint16_t *p = &bitmap.pix(y);

		for (int x = 0; x < 1024 / 8; x += 2)
		{
			uint8_t gfx = m_videoram[(x + 1) | (y << 7)];
			for (int i = 7; i >= 0; i--)
			{
				*p++ = BIT(gfx, i);
			}

			gfx = m_videoram[x | (y << 7)];
			for (int i = 7; i >= 0; i--)
			{
				*p++ = BIT(gfx, i);
			}
		}
	}
	return 0;
}

uint8_t bitgraph_state::ppu_read(offs_t offset)
{
	uint8_t data = m_ppu[offset];
	LOGDBG("PPU %d == %02X\n", offset, data);
	return data;
}

void bitgraph_state::ppu_write(offs_t offset, uint8_t data)
{
	LOGDBG("PPU %d <- %02X\n", offset, data);
	m_ppu[offset] = data;
}

void bitgraph_state::ppu_io(address_map &map)
{
//  map(0x00, 0x00).r(FUNC(bitgraph_state::ppu_irq));
}

/*
    p4  O: Centronics data 3..0
    p5  O: Centronics data 7..4
    p6  O: Centronics control
    p7  I: Centronics status
*/
template <unsigned Offset> void bitgraph_state::ppu_i8243_w(uint8_t data)
{
	LOG("PPU 8243 %d <- %02X\n", Offset, data);
	switch (Offset)
	{
	case 4:
		m_centronics->write_data0(BIT(data, 0));
		m_centronics->write_data1(BIT(data, 1));
		m_centronics->write_data2(BIT(data, 2));
		m_centronics->write_data3(BIT(data, 3));
		break;
	case 5:
		m_centronics->write_data4(BIT(data, 0));
		m_centronics->write_data5(BIT(data, 1));
		m_centronics->write_data6(BIT(data, 2));
		m_centronics->write_data7(BIT(data, 3));
		break;
	case 6:
		m_centronics->write_strobe(BIT(data, 0));
		// 1: Paper instruction
		m_centronics->write_init(BIT(data, 2));
		break;
	case 7:
		m_centronics->write_ack(BIT(data, 0));
		m_centronics->write_busy(BIT(data, 1));
		m_centronics->write_perror(BIT(data, 2));
		m_centronics->write_select(BIT(data, 3));
		break;
	}
}


void bitgraph_state::machine_start()
{
	m_videoram = (uint8_t *)m_maincpu->space(AS_PROGRAM).get_write_ptr(0x3e0000);
}

void bitgraph_state::machine_reset()
{
	m_maincpu->reset();
	m_misccr = 0;
	m_pia_a = 0;
	m_pia_b = 0;
	memset(m_ppu, 0, sizeof(m_ppu));
}


void bitgraph_state::bg_motherboard(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(40);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	m_screen->set_size(1024, 768);
	m_screen->set_visarea_full();
	m_screen->set_screen_update(FUNC(bitgraph_state::screen_update));
	m_screen->set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME);

	ACIA6850(config, m_acia0, 0);
	m_acia0->txd_handler().set(RS232_H_TAG, FUNC(rs232_port_device::write_txd));
	m_acia0->rts_handler().set(RS232_H_TAG, FUNC(rs232_port_device::write_rts));
	m_acia0->irq_handler().set_inputline(m_maincpu, M68K_IRQ_1);

	rs232_port_device &rs232h(RS232_PORT(config, RS232_H_TAG, default_rs232_devices, "null_modem"));
	rs232h.rxd_handler().set(m_acia0, FUNC(acia6850_device::write_rxd));
	rs232h.dcd_handler().set(m_acia0, FUNC(acia6850_device::write_dcd));
	rs232h.cts_handler().set(m_acia0, FUNC(acia6850_device::write_cts));

	ACIA6850(config, m_acia1, 0);
	m_acia1->txd_handler().set(RS232_K_TAG, FUNC(rs232_port_device::write_txd));
	m_acia1->rts_handler().set(RS232_K_TAG, FUNC(rs232_port_device::write_rts));
	m_acia1->irq_handler().set_inputline(m_maincpu, M68K_IRQ_1);

	rs232_port_device &rs232k(RS232_PORT(config, RS232_K_TAG, default_rs232_devices, "keyboard"));
	rs232k.rxd_handler().set(m_acia1, FUNC(acia6850_device::write_rxd));
	rs232k.dcd_handler().set(m_acia1, FUNC(acia6850_device::write_dcd));
	rs232k.cts_handler().set(m_acia1, FUNC(acia6850_device::write_cts));
	rs232k.set_option_device_input_defaults("keyboard", DEVICE_INPUT_DEFAULTS_NAME(kbd_rs232_defaults));

	ACIA6850(config, m_acia2, 0);
	m_acia2->txd_handler().set(RS232_D_TAG, FUNC(rs232_port_device::write_txd));
	m_acia2->rts_handler().set(RS232_D_TAG, FUNC(rs232_port_device::write_rts));
	m_acia2->irq_handler().set_inputline(m_maincpu, M68K_IRQ_1);

	rs232_port_device &rs232d(RS232_PORT(config, RS232_D_TAG, default_rs232_devices, nullptr));
	rs232d.rxd_handler().set(m_acia2, FUNC(acia6850_device::write_rxd));
	rs232d.dcd_handler().set(m_acia2, FUNC(acia6850_device::write_dcd));
	rs232d.cts_handler().set(m_acia2, FUNC(acia6850_device::write_cts));

	COM8116(config, m_dbrga, 5.0688_MHz_XTAL);
	m_dbrga->fr_handler().set(FUNC(bitgraph_state::com8116_a_fr_w));
	m_dbrga->ft_handler().set(FUNC(bitgraph_state::com8116_a_ft_w));

	COM8116(config, m_dbrgb, 5.0688_MHz_XTAL);
	m_dbrgb->fr_handler().set(FUNC(bitgraph_state::com8116_b_fr_w));
	m_dbrgb->ft_handler().set(FUNC(bitgraph_state::com8116_b_ft_w));

	PIA6821(config, m_pia, 0);
	m_pia->readca1_handler().set(FUNC(bitgraph_state::pia_ca1_r));
	m_pia->cb2_handler().set(FUNC(bitgraph_state::pia_cb2_w));
	m_pia->readpa_handler().set(FUNC(bitgraph_state::pia_pa_r));
	m_pia->writepa_handler().set(FUNC(bitgraph_state::pia_pa_w));
	m_pia->readpb_handler().set(FUNC(bitgraph_state::pia_pb_r));
	m_pia->writepb_handler().set(FUNC(bitgraph_state::pia_pb_w));

	ER2055(config, m_earom, 0);

	SPEAKER(config, "mono").front_center();
	AY8912(config, m_psg, XTAL(1'294'400));
	m_psg->port_a_write_callback().set(FUNC(bitgraph_state::earom_write));
	m_psg->add_route(ALL_OUTPUTS, "mono", 1.00);
}

void bitgraph_state::bg_ppu(machine_config &config)
{
	i8035_device &ppu(I8035(config, PPU_TAG, XTAL(6'900'000)));
	ppu.set_addrmap(AS_IO, &bitgraph_state::ppu_io);
//  ppu.t0_in_cb().set(FUNC(bitgraph_state::ppu_t0_r));
	ppu.prog_out_cb().set("i8243", FUNC(i8243_device::prog_w));

	i8243_device &i8243(I8243(config, "i8243"));
	i8243.p4_in_cb().set_constant(0);
	i8243.p5_in_cb().set_constant(0);
	i8243.p6_in_cb().set_constant(0);
	i8243.p7_in_cb().set_constant(0);
	i8243.p4_out_cb().set(FUNC(bitgraph_state::ppu_i8243_w<4>));
	i8243.p5_out_cb().set(FUNC(bitgraph_state::ppu_i8243_w<5>));
	i8243.p6_out_cb().set(FUNC(bitgraph_state::ppu_i8243_w<6>));
	i8243.p7_out_cb().set(FUNC(bitgraph_state::ppu_i8243_w<7>));

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->ack_handler().set("cent_status_in", FUNC(input_buffer_device::write_bit6));
	m_centronics->busy_handler().set("cent_status_in", FUNC(input_buffer_device::write_bit7));
	m_centronics->fault_handler().set("cent_status_in", FUNC(input_buffer_device::write_bit4));
	m_centronics->perror_handler().set("cent_status_in", FUNC(input_buffer_device::write_bit5));

	INPUT_BUFFER(config, "cent_status_in");

	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(cent_data_out);
}

void bitgraph_state::bitgrpha(machine_config &config)
{
	M68000(config, m_maincpu, XTAL(6'900'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &bitgraph_state::bitgrapha_mem);

	bg_motherboard(config);

	CLOCK(config, "system_clock", 40).signal_handler().set(FUNC(bitgraph_state::system_clock_write));

	ACIA6850(config, m_acia3, 0);
	m_acia3->txd_handler().set(RS232_M_TAG, FUNC(rs232_port_device::write_txd));
	m_acia3->rts_handler().set(RS232_M_TAG, FUNC(rs232_port_device::write_rts));
	m_acia3->irq_handler().set_inputline(M68K_TAG, M68K_IRQ_1);

	rs232_port_device &rs232m(RS232_PORT(config, RS232_M_TAG, default_rs232_devices, nullptr));
	rs232m.rxd_handler().set(m_acia3, FUNC(acia6850_device::write_rxd));
	rs232m.dcd_handler().set(m_acia3, FUNC(acia6850_device::write_dcd));
	rs232m.cts_handler().set(m_acia3, FUNC(acia6850_device::write_cts));

	RAM(config, RAM_TAG).set_default_size("128K");
}

void bitgraph_state::bitgrphb(machine_config &config)
{
	M68000(config, m_maincpu, XTAL(6'900'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &bitgraph_state::bitgraphb_mem);

	bg_motherboard(config);
//  bg_ppu(config);

	CLOCK(config, "system_clock", 1040).signal_handler().set(FUNC(bitgraph_state::system_clock_write));

	RAM(config, RAM_TAG).set_default_size("512K");
}

/* ROM definition */
ROM_START( bitgrpha )
	ROM_REGION16_BE( 0x8000, M68K_TAG, 0 )
	ROM_LOAD( "bg125.rom", 0x000000, 0x008000, CRC(b86c974e) SHA1(5367db80a856444c2a55de22b69a13f97a62f602))
	ROM_FILL( 0x38e4, 1, 0x4e ) // disable clksync()
	ROM_FILL( 0x38e5, 1, 0x75 )
ROM_END

ROM_START( bitgrphb )
	ROM_REGION16_BE( 0x8000, M68K_TAG, 0 )
	ROM_DEFAULT_BIOS("2.33a")

	ROM_SYSTEM_BIOS(0, "2.33a", "rev 2.33 Alpha' ROM")
	ROMX_LOAD( "bg2.32lo_u10.bin", 0x004001, 0x002000, CRC(6a702a96) SHA1(acdf1ba34038b4ccafb5b8069e70ae57a3b8a7e0), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD( "bg2.32hi_u12.bin", 0x004000, 0x002000, CRC(a282a2c8) SHA1(ea7e4d4e197201c8944acef54479d5c2b26d409f), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD( "bg2.32lo_u11.bin", 0x000001, 0x002000, CRC(46912afd) SHA1(c1f771adc1ef62b1fb1b904ed1d2a61009e24f55), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD( "bg2.32hi_u13.bin", 0x000000, 0x002000, CRC(731df44f) SHA1(8c238b5943b8864e539f92891a0ffa6ddd4fc779), ROM_BIOS(0) | ROM_SKIP(1))

	ROM_SYSTEM_BIOS(1, "3.0p", "rev 3.0P ROM")
	ROMX_LOAD( "bg5173_u10.bin", 0x004001, 0x002000, CRC(40014850) SHA1(ef0b7da58a5183391a3a03947882197f25694518), ROM_BIOS(1) | ROM_SKIP(1))
	ROMX_LOAD( "bg5175_u12.bin", 0x004000, 0x002000, CRC(c2c4cc6c) SHA1(dbbce7cb58b4cef1557a834cbb07b3ace298cb8b), ROM_BIOS(1) | ROM_SKIP(1))
	ROMX_LOAD( "bg5174_u11.bin", 0x000001, 0x002000, CRC(639768b9) SHA1(68f623bcf3bb75390ba2b17efc067cf25f915ec0), ROM_BIOS(1) | ROM_SKIP(1))
	ROMX_LOAD( "bg5176_u13.bin", 0x000000, 0x002000, CRC(984e7e8c) SHA1(dd13cbaff96a8b9936ae8cb07205c6abe8b27b6e), ROM_BIOS(1) | ROM_SKIP(1))

	ROM_SYSTEM_BIOS(2, "ramtest", "RAM test")
	ROMX_LOAD( "ramtest.rom", 0x000000, 0x004000, CRC(fabe3b34) SHA1(4d892a2ed2b7ea12d83843609981be9069611d43), ROM_BIOS(2))

	ROM_REGION( 0x800, PPU_TAG, 0 )
	ROM_LOAD( "bg_mouse_u9.bin", 0x0000, 0x0800, CRC(fd827ff5) SHA1(6d4a8e9b18c7610c5cfde40464826d144d387601))
ROM_END

} // anonymous namespace

/* Driver */
//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY  FULLNAME          FLAGS
COMP( 1981, bitgrpha, 0,      0,      bitgrpha, bitgraph, bitgraph_state, empty_init, "BBN",   "BitGraph rev A", ROT90 )
COMP( 1982, bitgrphb, 0,      0,      bitgrphb, bitgraph, bitgraph_state, empty_init, "BBN",   "BitGraph rev B", ROT270 | MACHINE_NOT_WORKING )
