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
    - some kind of baud rate generator, possibly COM8016
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

#include "bitgrpha.lh"
#include "bitgrphb.lh"

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
#define COM8116_A_TAG "com8116_a"
#define COM8116_B_TAG "com8116_b"
#define ADLC_TAG "adlc"
#define PIA_TAG "pia"
#define PSG_TAG "psg"
#define EAROM_TAG "earom"

#define VERBOSE_DBG 1       /* general debug messages */

#define DBG_LOG(N,M,A) \
	do { \
		if(VERBOSE_DBG>=N) \
		{ \
			if( M ) \
				logerror("%11.6f at %s: %-24s",machine().time().as_double(),machine().describe_context(),(char*)M ); \
			logerror A; \
		} \
	} while (0)

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
		, m_dbrga(*this, COM8116_A_TAG)
		, m_dbrgb(*this, COM8116_B_TAG)
		, m_pia(*this, PIA_TAG)
		, m_psg(*this, PSG_TAG)
		, m_earom(*this, EAROM_TAG)
		, m_centronics(*this, "centronics")
		, m_screen(*this, "screen")
	{ }

	DECLARE_READ8_MEMBER( pia_r );
	DECLARE_WRITE8_MEMBER( pia_w );
	DECLARE_READ8_MEMBER( pia_pa_r );
	DECLARE_READ8_MEMBER( pia_pb_r );
	DECLARE_WRITE8_MEMBER( pia_pa_w );
	DECLARE_WRITE8_MEMBER( pia_pb_w );
	DECLARE_READ_LINE_MEMBER( pia_ca1_r );
	DECLARE_READ_LINE_MEMBER( pia_cb1_r );
	DECLARE_WRITE_LINE_MEMBER( pia_ca2_w );
	DECLARE_WRITE_LINE_MEMBER( pia_cb2_w );

	DECLARE_WRITE16_MEMBER( baud_write );
	DECLARE_WRITE_LINE_MEMBER( com8116_a_fr_w );
	DECLARE_WRITE_LINE_MEMBER( com8116_a_ft_w );
	DECLARE_WRITE_LINE_MEMBER( com8116_b_fr_w );
	DECLARE_WRITE_LINE_MEMBER( com8116_b_ft_w );

	DECLARE_READ8_MEMBER( adlc_r );
	DECLARE_WRITE8_MEMBER( adlc_w );

	DECLARE_WRITE8_MEMBER( earom_write );
	DECLARE_WRITE8_MEMBER( misccr_write );
	DECLARE_WRITE_LINE_MEMBER( system_clock_write );

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER( ppu_read );
	DECLARE_WRITE8_MEMBER( ppu_write );
	DECLARE_WRITE8_MEMBER( ppu_i8243_w );

private:
	virtual void machine_start();
	virtual void machine_reset();
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

	UINT8 *m_videoram;
	UINT8 m_misccr;
	UINT8 m_pia_a;
	UINT8 m_pia_b;
	UINT8 m_ppu[4];
};

static ADDRESS_MAP_START(bitgrapha_mem, AS_PROGRAM, 16, bitgraph_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x007fff) AM_ROM
	AM_RANGE(0x010000, 0x010001) AM_DEVREADWRITE8(ACIA0_TAG, acia6850_device, data_r, data_w, 0xff00)   // HOST
	AM_RANGE(0x010002, 0x010003) AM_DEVREADWRITE8(ACIA0_TAG, acia6850_device, status_r, control_w, 0xff00)
	AM_RANGE(0x010008, 0x010009) AM_DEVREADWRITE8(ACIA1_TAG, acia6850_device, data_r, data_w, 0x00ff)   // KEYBOARD
	AM_RANGE(0x01000a, 0x01000b) AM_DEVREADWRITE8(ACIA1_TAG, acia6850_device, status_r, control_w, 0x00ff)
	AM_RANGE(0x010010, 0x010011) AM_DEVREADWRITE8(ACIA2_TAG, acia6850_device, data_r, data_w, 0x00ff)   // DEBUGGER
	AM_RANGE(0x010012, 0x010013) AM_DEVREADWRITE8(ACIA2_TAG, acia6850_device, status_r, control_w, 0x00ff)
	AM_RANGE(0x010018, 0x010019) AM_DEVREADWRITE8(ACIA3_TAG, acia6850_device, data_r, data_w, 0x00ff)   // POINTER
	AM_RANGE(0x01001a, 0x01001b) AM_DEVREADWRITE8(ACIA3_TAG, acia6850_device, status_r, control_w, 0x00ff)
	AM_RANGE(0x010020, 0x010027) AM_READWRITE8(adlc_r, adlc_w, 0xff00)
	AM_RANGE(0x010028, 0x01002f) AM_READWRITE8(pia_r, pia_w, 0xff00)    // EAROM, PSG
	AM_RANGE(0x010030, 0x010031) AM_WRITE(baud_write)
	AM_RANGE(0x3e0000, 0x3fffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(bitgraphb_mem, AS_PROGRAM, 16, bitgraph_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x007fff) AM_ROM
	AM_RANGE(0x010000, 0x010001) AM_DEVREADWRITE8(ACIA0_TAG, acia6850_device, data_r, data_w, 0xff00)   // HOST
	AM_RANGE(0x010002, 0x010003) AM_DEVREADWRITE8(ACIA0_TAG, acia6850_device, status_r, control_w, 0xff00)
	AM_RANGE(0x010008, 0x010009) AM_DEVREADWRITE8(ACIA1_TAG, acia6850_device, data_r, data_w, 0x00ff)   // KEYBOARD
	AM_RANGE(0x01000a, 0x01000b) AM_DEVREADWRITE8(ACIA1_TAG, acia6850_device, status_r, control_w, 0x00ff)
	AM_RANGE(0x010010, 0x010011) AM_DEVREADWRITE8(ACIA2_TAG, acia6850_device, data_r, data_w, 0x00ff)   // DEBUGGER
	AM_RANGE(0x010012, 0x010013) AM_DEVREADWRITE8(ACIA2_TAG, acia6850_device, status_r, control_w, 0x00ff)
	AM_RANGE(0x01001a, 0x01001b) AM_WRITE8(misccr_write, 0x00ff)
	AM_RANGE(0x010020, 0x010027) AM_READWRITE8(adlc_r, adlc_w, 0xff00)
	AM_RANGE(0x010028, 0x01002f) AM_READWRITE8(pia_r, pia_w, 0xff00)    // EAROM, PSG
	AM_RANGE(0x010030, 0x010031) AM_WRITE(baud_write)
//  AM_RANGE(0x010030, 0x010037) AM_READ8(ppu_read, 0x00ff)
//  AM_RANGE(0x010038, 0x01003f) AM_WRITE8(ppu_write, 0x00ff)
	AM_RANGE(0x380000, 0x3fffff) AM_RAM
ADDRESS_MAP_END

static INPUT_PORTS_START(bitgraph)
INPUT_PORTS_END

READ8_MEMBER(bitgraph_state::pia_r)
{
	DBG_LOG(3,"PIA", ("R %d\n", offset));
	return m_pia->read(space, 3-offset);
}

WRITE8_MEMBER(bitgraph_state::pia_w)
{
	DBG_LOG(3,"PIA", ("W %d < %02X\n", offset, data));
	return m_pia->write(space, 3-offset, data);
}

READ_LINE_MEMBER(bitgraph_state::pia_ca1_r)
{
	return m_screen->frame_number() & 1;
}

WRITE_LINE_MEMBER(bitgraph_state::pia_cb2_w)
{
	// XXX shut up verbose log
}

READ8_MEMBER(bitgraph_state::pia_pa_r)
{
	UINT8 data = BIT(m_pia_b, 3) ? m_earom->data() : m_pia_a;
	DBG_LOG(2,"PIA", ("A == %02X (%s)\n", data, BIT(m_pia_b, 3) ? "earom" : "pia"));
	return data;
}

WRITE8_MEMBER(bitgraph_state::pia_pa_w)
{
	DBG_LOG(2,"PIA", ("A <- %02X\n", data));
	m_pia_a = data;
}

/*
        B0          O: BC1  to noisemaker.
        B1          O: BDIR to noisemaker.
        B2          O: Clock for EAROM.
        B3          O: CS1   for EAROM.
        B4          O: Enable HDLC Xmt interrupt.
        B5          O: Enable HDLC Rcv interrupt.
        B6          O: Clear Clock interrupt.  Must write a 0 [clear interrupt], then a 1.
        B7          I: EVEN field ??
*/
READ8_MEMBER(bitgraph_state::pia_pb_r)
{
	DBG_LOG(2,"PIA", ("B == %02X\n", m_pia_b));
	return m_pia_b;
}

WRITE8_MEMBER(bitgraph_state::pia_pb_w)
{
	DBG_LOG(2,"PIA", ("B <- %02X\n", data));
	m_pia_b = data;

	switch (m_pia_b & 0x03) {
		case 2: m_psg->data_w(space, 0, m_pia_a); break;
		case 3: m_psg->address_w(space, 0, m_pia_a); break;
	}

	if (BIT(m_pia_b, 3)) {
		DBG_LOG(2,"EAROM", ("data <- %02X\n", m_pia_a));
		m_earom->set_data(m_pia_a);
	}
	// CS1, ~CS2, C1, C2, CK
	m_earom->set_control(BIT(m_pia_b, 3), BIT(m_pia_b, 3), BIT(m_pia_a, 6), BIT(m_pia_a, 7), BIT(m_pia_b, 2));

	if (!BIT(m_pia_b, 6)) {
		m_maincpu->set_input_line(M68K_IRQ_6, CLEAR_LINE);
	}
}

WRITE8_MEMBER(bitgraph_state::earom_write)
{
	DBG_LOG(2,"EAROM", ("addr <- %02X (%02X)\n", data & 0x3f, data));
	m_earom->set_address(data & 0x3f);
}

// written once and never changed
WRITE8_MEMBER(bitgraph_state::misccr_write)
{
	DBG_LOG(1,"MISCCR", ("<- %02X (DTR %d MAP %d)\n", data, BIT(data, 3), (data & 3)));
	m_misccr = data;
}

WRITE_LINE_MEMBER(bitgraph_state::system_clock_write)
{
	if (!BIT(m_pia_b, 6)) {
		m_maincpu->set_input_line(M68K_IRQ_6, CLEAR_LINE);
		return;
	}
	if (state) {
		m_maincpu->set_input_line_and_vector(M68K_IRQ_6, ASSERT_LINE, M68K_INT_ACK_AUTOVECTOR);
	} else {
		m_maincpu->set_input_line(M68K_IRQ_6, CLEAR_LINE);
	}
}

WRITE16_MEMBER(bitgraph_state::baud_write)
{
	DBG_LOG(1,"Baud", ("%04X\n", data));
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
	if (m_acia3) {
		m_acia3->write_txc(state);
		m_acia3->write_rxc(state);
	}
}

READ8_MEMBER(bitgraph_state::adlc_r)
{
	DBG_LOG(1,"ADLC", ("R %d\n", offset));
	return m_adlc ? m_adlc->read(space, 3-offset) : 0xff;
}

WRITE8_MEMBER(bitgraph_state::adlc_w)
{
	DBG_LOG(1,"ADLC", ("W %d < %02X\n", offset, data));
	if (m_adlc) return m_adlc->write(space, 3-offset, data);
}

UINT32 bitgraph_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 gfx=0;
	int x,y;

	for (y = 0; y < 768; y++)
	{
		UINT16 *p = &bitmap.pix16(y);

		for (x = 0; x < 1024/8; x+=2)
		{
			gfx = m_videoram[ (x+1) | (y<<7)];

			*p++ = BIT(gfx, 7);
			*p++ = BIT(gfx, 6);
			*p++ = BIT(gfx, 5);
			*p++ = BIT(gfx, 4);
			*p++ = BIT(gfx, 3);
			*p++ = BIT(gfx, 2);
			*p++ = BIT(gfx, 1);
			*p++ = BIT(gfx, 0);

			gfx = m_videoram[ x | (y<<7)];

			*p++ = BIT(gfx, 7);
			*p++ = BIT(gfx, 6);
			*p++ = BIT(gfx, 5);
			*p++ = BIT(gfx, 4);
			*p++ = BIT(gfx, 3);
			*p++ = BIT(gfx, 2);
			*p++ = BIT(gfx, 1);
			*p++ = BIT(gfx, 0);
		}
	}
	return 0;
}

READ8_MEMBER(bitgraph_state::ppu_read)
{
	UINT8 data = m_ppu[offset];
	DBG_LOG(1,"PPU", ("%d == %02X\n", offset, data));
	return data;
}

WRITE8_MEMBER(bitgraph_state::ppu_write)
{
	DBG_LOG(1,"PPU", ("%d <- %02X\n", offset, data));
	m_ppu[offset] = data;
}

#ifdef UNUSED_FUNCTION
static ADDRESS_MAP_START(ppu_io, AS_IO, 8, bitgraph_state)
//  AM_RANGE(0x00, 0x00) AM_READ(ppu_irq)
//  AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1)
//  AM_RANGE(MCS48_PORT_T0, MCS48_PORT_T0) AM_READ(ppu_t0_r)
	AM_RANGE(MCS48_PORT_PROG, MCS48_PORT_PROG) AM_DEVWRITE("i8243", i8243_device, i8243_prog_w)
ADDRESS_MAP_END
#endif

/*
    p4  O: Centronics data 3..0
    p5  O: Centronics data 7..4
    p6  O: Centronics control
    p7  I: Centronics status
*/
WRITE8_MEMBER(bitgraph_state::ppu_i8243_w)
{
	DBG_LOG(1,"PPU", ("8243 %d <- %02X\n", offset + 4, data));
	switch (offset) {
		case 0:
			m_centronics->write_data0(BIT(data, 0));
			m_centronics->write_data1(BIT(data, 1));
			m_centronics->write_data2(BIT(data, 2));
			m_centronics->write_data3(BIT(data, 3));
			break;
		case 1:
			m_centronics->write_data4(BIT(data, 0));
			m_centronics->write_data5(BIT(data, 1));
			m_centronics->write_data6(BIT(data, 2));
			m_centronics->write_data7(BIT(data, 3));
			break;
		case 2:
			m_centronics->write_strobe(BIT(data, 0));
			// 1: Paper instruction
			m_centronics->write_init(BIT(data, 2));
			break;
		case 3:
			m_centronics->write_ack(BIT(data, 0));
			m_centronics->write_busy(BIT(data, 1));
			m_centronics->write_perror(BIT(data, 2));
			m_centronics->write_select(BIT(data, 3));
			break;
	}
}


void bitgraph_state::machine_start()
{
	m_videoram = (UINT8 *)m_maincpu->space(AS_PROGRAM).get_write_ptr(0x3e0000);
}

void bitgraph_state::machine_reset()
{
	m_maincpu->reset();
	m_misccr = 0;
	m_pia_a = 0;
	m_pia_b = 0;
	memset(m_ppu, 0, sizeof(m_ppu));
}


static MACHINE_CONFIG_FRAGMENT( bg_motherboard )
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(40)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(1024, 768)
	MCFG_SCREEN_VISIBLE_AREA(0, 1024-1, 0, 768-1)
	MCFG_SCREEN_UPDATE_DRIVER(bitgraph_state, screen_update)

	MCFG_SCREEN_PALETTE("palette")
	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")

	MCFG_DEVICE_ADD(ACIA0_TAG, ACIA6850, 0)
	MCFG_ACIA6850_TXD_HANDLER(DEVWRITELINE(RS232_H_TAG, rs232_port_device, write_txd))
	MCFG_ACIA6850_RTS_HANDLER(DEVWRITELINE(RS232_H_TAG, rs232_port_device, write_rts))
	MCFG_ACIA6850_IRQ_HANDLER(DEVWRITELINE(M68K_TAG, m68000_device, write_irq1))

	MCFG_RS232_PORT_ADD(RS232_H_TAG, default_rs232_devices, "null_modem")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(ACIA0_TAG, acia6850_device, write_rxd))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(ACIA0_TAG, acia6850_device, write_dcd))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(ACIA0_TAG, acia6850_device, write_cts))

	MCFG_DEVICE_ADD(ACIA1_TAG, ACIA6850, 0)
	MCFG_ACIA6850_TXD_HANDLER(DEVWRITELINE(RS232_K_TAG, rs232_port_device, write_txd))
	MCFG_ACIA6850_RTS_HANDLER(DEVWRITELINE(RS232_K_TAG, rs232_port_device, write_rts))
	MCFG_ACIA6850_IRQ_HANDLER(DEVWRITELINE(M68K_TAG, m68000_device, write_irq1))

	MCFG_RS232_PORT_ADD(RS232_K_TAG, default_rs232_devices, "keyboard")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(ACIA1_TAG, acia6850_device, write_rxd))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(ACIA1_TAG, acia6850_device, write_dcd))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(ACIA1_TAG, acia6850_device, write_cts))

	MCFG_DEVICE_ADD(ACIA2_TAG, ACIA6850, 0)
	MCFG_ACIA6850_TXD_HANDLER(DEVWRITELINE(RS232_D_TAG, rs232_port_device, write_txd))
	MCFG_ACIA6850_RTS_HANDLER(DEVWRITELINE(RS232_D_TAG, rs232_port_device, write_rts))
	MCFG_ACIA6850_IRQ_HANDLER(DEVWRITELINE(M68K_TAG, m68000_device, write_irq1))

	MCFG_RS232_PORT_ADD(RS232_D_TAG, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(ACIA2_TAG, acia6850_device, write_rxd))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(ACIA2_TAG, acia6850_device, write_dcd))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(ACIA2_TAG, acia6850_device, write_cts))

	// XXX actual part may be something else
	MCFG_DEVICE_ADD(COM8116_A_TAG, COM8116, XTAL_5_0688MHz)
	MCFG_COM8116_FR_HANDLER(WRITELINE(bitgraph_state, com8116_a_fr_w))
	MCFG_COM8116_FT_HANDLER(WRITELINE(bitgraph_state, com8116_a_ft_w))

	MCFG_DEVICE_ADD(COM8116_B_TAG, COM8116, XTAL_5_0688MHz)
	MCFG_COM8116_FR_HANDLER(WRITELINE(bitgraph_state, com8116_b_fr_w))
	MCFG_COM8116_FT_HANDLER(WRITELINE(bitgraph_state, com8116_b_ft_w))

	MCFG_DEVICE_ADD(PIA_TAG, PIA6821, 0)
	MCFG_PIA_READCA1_HANDLER(READLINE(bitgraph_state, pia_ca1_r))
	MCFG_PIA_CB2_HANDLER(WRITELINE(bitgraph_state, pia_cb2_w))
	MCFG_PIA_READPA_HANDLER(READ8(bitgraph_state, pia_pa_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(bitgraph_state, pia_pa_w))
	MCFG_PIA_READPB_HANDLER(READ8(bitgraph_state, pia_pb_r))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(bitgraph_state, pia_pb_w))

	MCFG_ER2055_ADD(EAROM_TAG)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(PSG_TAG, AY8912, XTAL_1_2944MHz)
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(bitgraph_state, earom_write))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_CONFIG_END

#ifdef UNUSED_FUNCTION
static MACHINE_CONFIG_FRAGMENT( bg_ppu )
	MCFG_CPU_ADD(PPU_TAG, I8035, XTAL_6_9MHz)
	MCFG_CPU_IO_MAP(ppu_io)

	MCFG_I8243_ADD("i8243", NOOP, WRITE8(bitgraph_state, ppu_i8243_w))

	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_ACK_HANDLER(DEVWRITELINE("cent_status_in", input_buffer_device, write_bit6))
	MCFG_CENTRONICS_BUSY_HANDLER(DEVWRITELINE("cent_status_in", input_buffer_device, write_bit7))
	MCFG_CENTRONICS_FAULT_HANDLER(DEVWRITELINE("cent_status_in", input_buffer_device, write_bit4))
	MCFG_CENTRONICS_PERROR_HANDLER(DEVWRITELINE("cent_status_in", input_buffer_device, write_bit5))

	MCFG_DEVICE_ADD("cent_status_in", INPUT_BUFFER, 0)

	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", "centronics")
MACHINE_CONFIG_END
#endif

static MACHINE_CONFIG_START( bitgrpha, bitgraph_state )
	MCFG_CPU_ADD(M68K_TAG, M68000, XTAL_6_9MHz)
	MCFG_CPU_PROGRAM_MAP(bitgrapha_mem)

	MCFG_FRAGMENT_ADD(bg_motherboard)

	MCFG_DEVICE_ADD("system_clock", CLOCK, 40)
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(bitgraph_state, system_clock_write))

	MCFG_DEVICE_ADD(ACIA3_TAG, ACIA6850, 0)
	MCFG_ACIA6850_TXD_HANDLER(DEVWRITELINE(RS232_M_TAG, rs232_port_device, write_txd))
	MCFG_ACIA6850_RTS_HANDLER(DEVWRITELINE(RS232_M_TAG, rs232_port_device, write_rts))
	MCFG_ACIA6850_IRQ_HANDLER(DEVWRITELINE(M68K_TAG, m68000_device, write_irq1))

	MCFG_RS232_PORT_ADD(RS232_M_TAG, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(ACIA3_TAG, acia6850_device, write_rxd))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(ACIA3_TAG, acia6850_device, write_dcd))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(ACIA3_TAG, acia6850_device, write_cts))

	MCFG_DEFAULT_LAYOUT(layout_bitgrpha)

	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("128K")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( bitgrphb, bitgraph_state )
	MCFG_CPU_ADD(M68K_TAG, M68000, XTAL_6_9MHz)
	MCFG_CPU_PROGRAM_MAP(bitgraphb_mem)

	MCFG_FRAGMENT_ADD(bg_motherboard)
//  MCFG_FRAGMENT_ADD(bg_ppu)

	MCFG_DEVICE_ADD("system_clock", CLOCK, 1040)
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(bitgraph_state, system_clock_write))

	MCFG_DEFAULT_LAYOUT(layout_bitgrphb)

	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("512K")
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( bitgrpha )
	ROM_REGION16_BE( 0x8000, M68K_TAG, 0 )
	ROM_LOAD( "bg125.rom", 0x000000, 0x008000, CRC(b86c974e) SHA1(5367db80a856444c2a55de22b69a13f97a62f602))
	ROM_FILL( 0x38e4, 1, 0x4e ) // disable clksync()
	ROM_FILL( 0x38e5, 1, 0x75 )
ROM_END

ROM_START( bitgrphb )
	ROM_REGION16_BE( 0x8000, M68K_TAG, 0 )
	ROM_DEFAULT_BIOS("2.33A")

	ROM_SYSTEM_BIOS(0, "2.33A", "rev 2.33 Alpha' ROM")
	ROMX_LOAD( "bg2.32lo_u10.bin", 0x004001, 0x002000, CRC(6a702a96) SHA1(acdf1ba34038b4ccafb5b8069e70ae57a3b8a7e0), ROM_BIOS(1)|ROM_SKIP(1))
	ROMX_LOAD( "bg2.32hi_u12.bin", 0x004000, 0x002000, CRC(a282a2c8) SHA1(ea7e4d4e197201c8944acef54479d5c2b26d409f), ROM_BIOS(1)|ROM_SKIP(1))
	ROMX_LOAD( "bg2.32lo_u11.bin", 0x000001, 0x002000, CRC(46912afd) SHA1(c1f771adc1ef62b1fb1b904ed1d2a61009e24f55), ROM_BIOS(1)|ROM_SKIP(1))
	ROMX_LOAD( "bg2.32hi_u13.bin", 0x000000, 0x002000, CRC(731df44f) SHA1(8c238b5943b8864e539f92891a0ffa6ddd4fc779), ROM_BIOS(1)|ROM_SKIP(1))

	ROM_SYSTEM_BIOS(1, "3.0P", "rev 3.0P ROM")
	ROMX_LOAD( "bg5173_u10.bin", 0x004001, 0x002000, CRC(40014850) SHA1(ef0b7da58a5183391a3a03947882197f25694518), ROM_BIOS(2)|ROM_SKIP(1))
	ROMX_LOAD( "bg5175_u12.bin", 0x004000, 0x002000, CRC(c2c4cc6c) SHA1(dbbce7cb58b4cef1557a834cbb07b3ace298cb8b), ROM_BIOS(2)|ROM_SKIP(1))
	ROMX_LOAD( "bg5174_u11.bin", 0x000001, 0x002000, CRC(639768b9) SHA1(68f623bcf3bb75390ba2b17efc067cf25f915ec0), ROM_BIOS(2)|ROM_SKIP(1))
	ROMX_LOAD( "bg5176_u13.bin", 0x000000, 0x002000, CRC(984e7e8c) SHA1(dd13cbaff96a8b9936ae8cb07205c6abe8b27b6e), ROM_BIOS(2)|ROM_SKIP(1))

	ROM_SYSTEM_BIOS(2, "ramtest", "RAM test")
	ROMX_LOAD( "ramtest.rom", 0x000000, 0x004000, CRC(fabe3b34) SHA1(4d892a2ed2b7ea12d83843609981be9069611d43), ROM_BIOS(3))

	ROM_REGION( 0x800, PPU_TAG, 0 )
	ROM_LOAD( "bg_mouse_u9.bin", 0x0000, 0x0800, CRC(fd827ff5) SHA1(6d4a8e9b18c7610c5cfde40464826d144d387601))
ROM_END

/* Driver */
/*       YEAR  NAME      PARENT  COMPAT   MACHINE    INPUT    CLASS          INIT     COMPANY          FULLNAME       FLAGS */
COMP( 1981, bitgrpha, 0, 0, bitgrpha, bitgraph, driver_device, 0, "BBN", "BitGraph rev A", MACHINE_IMPERFECT_KEYBOARD)
COMP( 1982, bitgrphb, 0, 0, bitgrphb, bitgraph, driver_device, 0, "BBN", "BitGraph rev B", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_KEYBOARD)
