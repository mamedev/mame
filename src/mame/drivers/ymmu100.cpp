// license:BSD-3-Clause
// copyright-holders:R. Belmont, Olivier Galibert
/*************************************************************************************

    Yamaha MU-100 : 32-voice polyphonic/multitimbral General MIDI/GS/XG tone module
    Preliminary driver by R. Belmont and O. Galibert

    CPU: Hitachi H8S/2655 (HD6432655F), strapped for mode 4 (24-bit address, 16-bit data, no internal ROM)
    Sound ASIC: Yamaha XS725A0
    RAM: 1 MSM51008 (1 meg * 1 bit = 128KBytes)

    I/O ports from service manual:

    Port 1:
        0 - LCD data, SW data, LED 1
        1 - LCD data, SW data, LED 2
        2 - LCD data, SW data, LED 3
        3 - LCD data, SW data, LED 4
        4 - LCD data, SW data, LED 5
        5 - LCD data, SW strobe data
        6 - LCD data, SW strobe data
        7 - LCD data, SW data, LED 6

    Port 2:
        0 - (out) LCD control RS
        1 - (out) LCD control R/W
        2 - (out) LCD control E
        3 - (out) LCD contrast A
        4 - (out) LCD contrast B
        5 - (out) LCD contrast C
        6 - (out) 1 MHz clock for serial
        7 - NC

    Port 3:
        4 - (out) A/D gain control 1
        5 - (out) A/D gain control 2

    Port 5:
        3 - (out) Reset signal for rotary encoder

    Port 6:
        1 - NC
        2 - (out) PB select (SW1)
        3 - (out) PB select (SW2)
        4 - (out) reset PB
        5 - (out) reset SWP30 (sound chip)
        6 - NC
        7 - (in) Plug detection for A/D input

    Port A:
        5 - (in) Off Line Detection
        6 - (out) Signal for rotary encoder (REB)
        7 - (out) Signal for rotary encoder (REA)

    Port F:
        0 - (out) (sws) LED,SW Strobe data latch
        1 - (out) (swd) SW data read control
        2 - (out) PB select (SW4)

    Port G:
        0 - (out) PB select (SW3)

    Analog input channels:
        0 - level input R
        2 - level output L
        4 - host SW type switch position
        6 - battery voltage
        7 - model check (0 for MU100, 0.5 for OEM, 1 for MU100R)

    Switch map at the connector (17=ground)
        09 8 play
        10 8 edit
        11 8 mute/solo
        12 8 part -
        13 8 part +
        14 8 util
        15 8 effect
        16 8 enter
        12 7 select <
        13 7 select >
        16 7 mode
        15 7 eq
        14 7 exit
        10 7 value -
        11 7 value +
           2 led play
           3 led edit
           4 led util
           5 led effect
           6 led mode
           1 led eq

     IC32:
        1 p10 c.2
        2 p11 c.3
        3 p12 c.4
        4 p13 c.5
        5 p14 c.6
        6 p15 c.7
        7 p16 c.8
        8 p17 c.1
        g sws

     IC33
        1 p17 c.09
        2 p16 c.10
        3 p15 c.11
        4 p14 c.12
        5 p13 c.13
        6 p12 c.14
        7 p11 c.15
        8 p10 c.16
        g swd

**************************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "cpu/h8/h8s2655.h"
#include "video/hd44780.h"
#include "rendlay.h"
#include "bus/midi/midiinport.h"
#include "bus/midi/midioutport.h"

static INPUT_PORTS_START( mu100 )
	PORT_START("P7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Enter")     PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Effect")    PORT_CODE(KEYCODE_F)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Util")      PORT_CODE(KEYCODE_U)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Part +")    PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Part -")    PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Mute/Solo") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Edit")      PORT_CODE(KEYCODE_E)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Play")      PORT_CODE(KEYCODE_A)

	PORT_START("P8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Mode")      PORT_CODE(KEYCODE_M)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Eq")        PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Exit")      PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Select >")  PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Select <")  PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Value +")   PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Value -")   PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

class mu100_state : public driver_device
{
public:
	enum {
		P2_LCD_RS     = 0x01,
		P2_LCD_RW     = 0x02,
		P2_LCD_ENABLE = 0x04
	};

	mu100_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_lcd(*this, "lcd"),
			m_ioport_p7(*this, "P7"),
			m_ioport_p8(*this, "P8")
	{ }

	required_device<h8s2655_device> m_maincpu;
	required_device<hd44780_device> m_lcd;
	required_ioport m_ioport_p7;
	required_ioport m_ioport_p8;

	UINT8 cur_p1, cur_p2, cur_p3, cur_p5, cur_p6, cur_pa, cur_pf, cur_pg;
	UINT8 cur_ic32;
	float contrast;

	DECLARE_READ16_MEMBER(adc0_r);
	DECLARE_READ16_MEMBER(adc2_r);
	DECLARE_READ16_MEMBER(adc4_r);
	DECLARE_READ16_MEMBER(adc6_r);
	virtual DECLARE_READ16_MEMBER(adc7_r);

	DECLARE_WRITE16_MEMBER(p1_w);
	DECLARE_READ16_MEMBER(p1_r);
	DECLARE_WRITE16_MEMBER(p2_w);
	DECLARE_WRITE16_MEMBER(p3_w);
	DECLARE_WRITE16_MEMBER(p5_w);
	DECLARE_WRITE16_MEMBER(p6_w);
	DECLARE_READ16_MEMBER(p6_r);
	DECLARE_WRITE16_MEMBER(pa_w);
	DECLARE_READ16_MEMBER(pa_r);
	DECLARE_WRITE16_MEMBER(pf_w);
	DECLARE_WRITE16_MEMBER(pg_w);

	DECLARE_READ16_MEMBER(snd_r);
	DECLARE_WRITE16_MEMBER(snd_w);

	float lightlevel(const UINT8 *src, const UINT8 *render);
	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	virtual void machine_start() override;
};

class mu100r_state : public mu100_state {
public:
	mu100r_state(const machine_config &mconfig, device_type type, std::string tag)
		: mu100_state(mconfig, type, tag)
	{ }

	virtual DECLARE_READ16_MEMBER(adc7_r) override;
};

#include "../drivers/ymmu100.inc"

void mu100_state::machine_start()
{
	cur_p1 = cur_p2 = cur_p3 = cur_p5 = cur_p6 = cur_pa = cur_pf = cur_pg = cur_ic32 = 0xff;
	contrast = 1.0;
}

float mu100_state::lightlevel(const UINT8 *src, const UINT8 *render)
{
	UINT8 l = *src;
	if(l == 0)
		return 1.0;
	int slot = (src[1] << 8) | src[2];
	if(slot >= 0xff00)
		return (255-l)/255.0;

	int bit = slot & 7;
	int adr = (slot >> 3);
	if(render[adr] & (1 << bit))
		return 1-(1-(255-l)/255.0f)*contrast;
	return 0.95f;
}

UINT32 mu100_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const UINT8 *render = m_lcd->render();
	const UINT8 *src = ymmu100_bkg + 15;

	for(int y=0; y<241; y++) {
		UINT32 *pix = reinterpret_cast<UINT32 *>(bitmap.raw_pixptr(y));
		for(int x=0; x<800; x++) {
			float light = lightlevel(src, render);
			UINT32 col = (int(0xef*light) << 16) | (int(0xf5*light) << 8);
			*pix++ = col;
			src += 3;
		}
		for(int x=800; x<900; x++)
			*pix++ = 0;
	}

	for(int i=0; i<6; i++)
		if(cur_ic32 & (1 << (i == 5 ? 7 : i))) {
			int x = 830 + 40*(i & 1);
			int y = 55 + 65*(i >> 1);
			for(int yy=-9; yy <= 9; yy++) {
				int dx = int(sqrt((float)(99-yy*yy)));
				UINT32 *pix = reinterpret_cast<UINT32 *>(bitmap.raw_pixptr(y+yy)) + (x-dx);
				for(int xx=0; xx<2*dx+1; xx++)
					*pix++ = 0x00ff00;
			}
		}
	return 0;
}

static ADDRESS_MAP_START( mu100_map, AS_PROGRAM, 16, mu100_state )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE(0x200000, 0x21ffff) AM_RAM // 128K work RAM
	AM_RANGE(0x400000, 0x401fff) AM_READWRITE(snd_r, snd_w)
ADDRESS_MAP_END

READ16_MEMBER(mu100_state::snd_r)
{
	int chan = (offset >> 6) & 0x3f;
	int slot = offset & 0x3f;
	logerror("snd_r %02x.%02x (%06x)\n", chan, slot, m_maincpu->pc());
	return 0x0000;
}

WRITE16_MEMBER(mu100_state::snd_w)
{
	int chan = (offset >> 6) & 0x3f;
	int slot = offset & 0x3f;
	logerror("snd_w %02x.%02x, %04x (%06x)\n", chan, slot, data, m_maincpu->pc());
}

READ16_MEMBER(mu100_state::adc0_r)
{
	logerror("adc0_r\n");
	return 0;
}

READ16_MEMBER(mu100_state::adc2_r)
{
	logerror("adc2_r\n");
	return 0;
}

// Put the host switch to pure midi
READ16_MEMBER(mu100_state::adc4_r)
{
	return 0;
}

// Battery level
READ16_MEMBER(mu100_state::adc6_r)
{
	logerror("adc6_r\n");
	return 0x3ff;
}

// model detect.  pulled to GND (0) on MU100, to 0.5Vcc on the card version, to Vcc on MU100R
READ16_MEMBER(mu100_state::adc7_r)
{
	return 0;
}

READ16_MEMBER(mu100r_state::adc7_r)
{
	return 0x3ff;
}

WRITE16_MEMBER(mu100_state::p1_w)
{
	cur_p1 = data;
}

READ16_MEMBER(mu100_state::p1_r)
{
	if((cur_p2 & P2_LCD_ENABLE)) {
		if(cur_p2 & P2_LCD_RW) {
			if(cur_p2 & P2_LCD_RS)
				return m_lcd->data_read(space, offset);
			else
				return m_lcd->control_read(space, offset);
		} else
			return 0x00;
	}

	if(!(cur_pf & 0x02)) {
		UINT8 val = 0xff;
		if(!(cur_ic32 & 0x20))
			val &= m_ioport_p7->read();
		if(!(cur_ic32 & 0x40))
			val &= m_ioport_p8->read();
		return val;
	}

	return 0xff;
}

WRITE16_MEMBER(mu100_state::p2_w)
{
	// LCB enable edge
	if(!(cur_p2 & P2_LCD_ENABLE) && (data & P2_LCD_ENABLE)) {
		if(!(cur_p2 & P2_LCD_RW)) {
			if(cur_p2 & P2_LCD_RS)
				m_lcd->data_write(space, offset, cur_p1);
			else
				m_lcd->control_write(space, offset, cur_p1);
		}
	}
	contrast = (8 - ((cur_p2 >> 3) & 7))/8.0;
	cur_p2 = data;
}

WRITE16_MEMBER(mu100_state::p3_w)
{
	cur_p3 = data;
	logerror("A/D gain control %d\n", (data >> 4) & 3);
}

WRITE16_MEMBER(mu100_state::p5_w)
{
	cur_p5 = data;
	logerror("Rotary reset %d\n", (data >> 3) & 1);
}

WRITE16_MEMBER(mu100_state::p6_w)
{
	cur_p6 = data;
	logerror("pbsel %d pbreset %d soundreset %d\n", (data >> 2) & 3, (data >> 4) & 1, (data >> 5) & 1);
}

READ16_MEMBER(mu100_state::p6_r)
{
	logerror("plug in detect read\n");
	return 0x00;
}

WRITE16_MEMBER(mu100_state::pa_w)
{
	cur_pa = data;
	logerror("rotary encoder %d\n", (data >> 6) & 3);
}

READ16_MEMBER(mu100_state::pa_r)
{
	logerror("offline detect read\n");
	return 0x00;
}

WRITE16_MEMBER(mu100_state::pf_w)
{
	if(!(cur_pf & 0x01) && (data & 0x01))
		cur_ic32 = cur_p1;
	cur_pf = data;
}

WRITE16_MEMBER(mu100_state::pg_w)
{
	cur_pg = data;
	logerror("pbsel3 %d\n", data & 1);
}

static ADDRESS_MAP_START( mu100_iomap, AS_IO, 16, mu100_state )
	AM_RANGE(h8_device::PORT_1,  h8_device::PORT_1)  AM_READWRITE(p1_r, p1_w)
	AM_RANGE(h8_device::PORT_2,  h8_device::PORT_2)  AM_WRITE(p2_w)
	AM_RANGE(h8_device::PORT_3,  h8_device::PORT_3)  AM_WRITE(p3_w)
	AM_RANGE(h8_device::PORT_5,  h8_device::PORT_5)  AM_WRITE(p5_w)
	AM_RANGE(h8_device::PORT_6,  h8_device::PORT_6)  AM_READWRITE(p6_r, p6_w)
	AM_RANGE(h8_device::PORT_A,  h8_device::PORT_A)  AM_READWRITE(pa_r, pa_w)
	AM_RANGE(h8_device::PORT_F,  h8_device::PORT_F)  AM_WRITE(pf_w)
	AM_RANGE(h8_device::PORT_G,  h8_device::PORT_G)  AM_WRITE(pg_w)
	AM_RANGE(h8_device::ADC_0,   h8_device::ADC_0)   AM_READ(adc0_r)
	AM_RANGE(h8_device::ADC_2,   h8_device::ADC_2)   AM_READ(adc2_r)
	AM_RANGE(h8_device::ADC_4,   h8_device::ADC_4)   AM_READ(adc4_r)
	AM_RANGE(h8_device::ADC_6,   h8_device::ADC_6)   AM_READ(adc6_r)
	AM_RANGE(h8_device::ADC_7,   h8_device::ADC_7)   AM_READ(adc7_r)
ADDRESS_MAP_END

static MACHINE_CONFIG_START( mu100, mu100_state )
	MCFG_CPU_ADD( "maincpu", H8S2655, XTAL_16MHz )
	MCFG_CPU_PROGRAM_MAP( mu100_map )
	MCFG_CPU_IO_MAP( mu100_iomap )

	MCFG_HD44780_ADD("lcd")
	MCFG_HD44780_LCD_SIZE(4, 20)

	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate, asynchronous updating anyway */
	MCFG_SCREEN_UPDATE_DRIVER(mu100_state, screen_update)
	MCFG_SCREEN_SIZE(900, 241)
	MCFG_SCREEN_VISIBLE_AREA(0, 899, 0, 240)
	MCFG_DEFAULT_LAYOUT(layout_lcd)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_MIDI_PORT_ADD("mdin", midiin_slot, "midiin")
	MCFG_MIDI_RX_HANDLER(DEVWRITELINE("maincpu:sci0", h8_sci_device, rx_w))

	MCFG_MIDI_PORT_ADD("mdout", midiout_slot, "midiout")
	MCFG_DEVICE_MODIFY("maincpu:sci0")
	MCFG_H8_SCI_TX_CALLBACK(DEVWRITELINE(":mdout", midi_port_device, write_txd))
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED_CLASS( mu100r, mu100, mu100r_state )
MACHINE_CONFIG_END

#define ROM_LOAD16_WORD_SWAP_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(bios+1)) /* Note '+1' */

ROM_START( mu100 )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "bios0", "xu50720 (v1.11, Aug. 3, 1999)" )
	ROM_LOAD16_WORD_SWAP_BIOS( 0, "xu50720.ic11", 0x000000, 0x200000, CRC(1126a8a4) SHA1(e90b8bd9d14297da26ba12f4d9a4f2d22cd7d34a) )
	ROM_SYSTEM_BIOS( 1, "bios1", "xt71420 (v1.05, Sep. 19, 1997)" )
	ROM_LOAD16_WORD_SWAP_BIOS( 1, "xt71420.ic11", 0x000000, 0x200000, CRC(0e5b3bae) SHA1(3148c5bd59a3d00809d3ab1921216215fe2582c5) )

	ROM_REGION( 0x2800000, "waverom", 0 )
	ROM_LOAD32_WORD( "sx518b0.ic34", 0x000000, 0x400000, CRC(2550d44f) SHA1(fd3cce228c7d389a2fde25c808a5b26080588cba) )
	ROM_LOAD32_WORD( "sx743b0.ic35", 0x000002, 0x400000, CRC(a9109a6c) SHA1(a67bb49378a38a2d809bd717d286e18bc6496db0) )
	ROM_LOAD32_WORD( "xt445a0-828.ic36", 0x800000, 0x1000000, CRC(d4483a43) SHA1(5bfd0762dea8598eda19db20251dac20e31fa02c) )
	ROM_LOAD32_WORD( "xt461a0-829.ic37", 0x800002, 0x1000000, CRC(c5af4501) SHA1(1c88de197c36382311053add8b19a5740802cb78) )

	ROM_REGION( 0x1000, "lcd", 0)
	// Hand made, 3 characters unused
	ROM_LOAD( "mu100-font.bin", 0x0000, 0x1000, BAD_DUMP CRC(a7d6c1d6) SHA1(9f0398d678bdf607cb34d83ee535f3b7fcc97c41) )
ROM_END

// Identical to the mu100
ROM_START( mu100r )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "bios0", "xu50720 (v1.11, Aug. 3, 1999)" )
	ROM_LOAD16_WORD_SWAP_BIOS( 0, "xu50720.ic11", 0x000000, 0x200000, CRC(1126a8a4) SHA1(e90b8bd9d14297da26ba12f4d9a4f2d22cd7d34a) )
	ROM_SYSTEM_BIOS( 1, "bios1", "xt71420 (v1.05, Sep. 19, 1997)" )
	ROM_LOAD16_WORD_SWAP_BIOS( 1, "xt71420.ic11", 0x000000, 0x200000, CRC(0e5b3bae) SHA1(3148c5bd59a3d00809d3ab1921216215fe2582c5) )

	ROM_REGION( 0x2800000, "waverom", 0 )
	ROM_LOAD32_WORD( "sx518b0.ic34", 0x000000, 0x400000, CRC(2550d44f) SHA1(fd3cce228c7d389a2fde25c808a5b26080588cba) )
	ROM_LOAD32_WORD( "sx743b0.ic35", 0x000002, 0x400000, CRC(a9109a6c) SHA1(a67bb49378a38a2d809bd717d286e18bc6496db0) )
	ROM_LOAD32_WORD( "xt445a0-828.ic36", 0x800000, 0x1000000, CRC(d4483a43) SHA1(5bfd0762dea8598eda19db20251dac20e31fa02c) )
	ROM_LOAD32_WORD( "xt461a0-829.ic37", 0x800002, 0x1000000, CRC(c5af4501) SHA1(1c88de197c36382311053add8b19a5740802cb78) )

	ROM_REGION( 0x1000, "lcd", 0)
	// Hand made, 3 characters unused
	ROM_LOAD( "mu100-font.bin", 0x0000, 0x1000, BAD_DUMP CRC(a7d6c1d6) SHA1(9f0398d678bdf607cb34d83ee535f3b7fcc97c41) )
ROM_END

CONS( 1997, mu100,  0,     0, mu100,  mu100, driver_device, 0, "Yamaha", "MU100",                  MACHINE_NOT_WORKING )
CONS( 1997, mu100r, mu100, 0, mu100r, mu100, driver_device, 0, "Yamaha", "MU100 Rackable version", MACHINE_NOT_WORKING )
