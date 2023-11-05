// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Fabio Priuli
// thanks-to:Chris Covell
/***********************************************************************************************************

 Sega AI Soundbox expansion emulation


 Sega AI Computer Sound Box, Model "AI-2002"  quick PCB overview by Chris Covell

ICs on board:

IC 2       TMP82C53F-2    (91/09)  Toshiba (Peripheral Timer)
IC 3       HN27512G-25    (87/12)  Hitachi 64K EPROM
IC 6       YM2151         (91/10)  Yamaha FM chip
IC 7       TMP82C55AF-10  (88/15)  Toshiba (Peripheral Interface)
IC 8       YM3012         (91/10)  Yamaha Stereo DAC
IC 9       HA17358                 Hitachi Dual Op-Amp
IC 10      LC7537N                 Sanyo (Volume Control IC)
IC 11      C324C          (90/42)  NEC Quad Op-Amp
IC 12      LA4520                  (Sanyo Power Audio Amp?)
IC 16-19   MB81464-12     (91/12)  Fujitsu 32K DRAMs


Misc Flat DIPs

IC ??      LS125A        Hitachi (near C41)
IC ??      74HC04        TI      (near C38)
IC ??      74HC157A x2   Toshiba (near C37)
IC ??      74HC138       TI      (near C44, furthest)
IC ??      74HC139       TI      (near C44, closest)

TODO:
- Connections of the 8253
- Keyboard matrix is scanned on a timer irq (#FC) from 8253??


HC04
pin 1 A1   - PB7
pin 2 Y1   -> HC04 pin 3 A2
pin 3 A2   <- HC04 pin 2 Y1
pin 4 Y2   -> 4th point, 1st row below HC04?
pin 5 A3   - 
pin 6 Y3   - 
pin 7 GND  -
pin 8 Y4   -  1st point, 2nd row below HC04 
pin 9 A4   -  1st point, 1st row below HC04
pin 10 Y5  - 
pin 11 A5  <- HC04 pin 12 Y6
pin 12 Y6  -> HC04 pin 11 A5
pin 13 A6  -  point just below C38 then continues to DRAMs
pin 14 VCC - 

8255 PB7 - connected to HC04 pin 1?, pulled low

TMP8253
pin 9 CLK0 - seems to be tied to pin 24 in ym2151
pin 14 OUT0 - --> 2nd point, 2nd row below HC04
pin 15 GATE0 - NC
pin 18 OUT1  - 7th point, 1st row below HC04 -> 8th point, 1st row below HC04 -> LS125 pin 2?
pin 19 GATE1 - 6th point, 1st row below HC04 -> 4th point, 1st row below HC04
pin 20 CLK1  - 5th point, 1st row below HC04 -> 2nd point, 1st row below HC04 -> left point above C37 -> pin 1 2 lc157s to the right of IC16 (can't be right)

timer 0 - mode 3 - square wave (000A), gate not involved
timer 1 - mode 2 - rate generator (0E90), gate involved
0e90 = 3818

 ***********************************************************************************************************/


#include "emu.h"
#include "soundbox.h"
#include "speaker.h"


DEFINE_DEVICE_TYPE(SEGAAI_SOUNDBOX, segaai_soundbox_device, "segaai_soundbox", "Sega AI Expansion - Soundbox")


segaai_soundbox_device::segaai_soundbox_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SEGAAI_SOUNDBOX, tag, owner, clock)
	, segaai_exp_interface( mconfig, *this )
	, m_tmp8253(*this, "tmp8253")
	, m_tmp8255(*this, "tmp8255")
	, m_ym2151(*this, "ym2151")
	, m_rom(*this, "soundbox")
	, m_rows(*this, "ROW%u", 0U)
	, m_row(0)
	, m_8255_portb(0)
{
}

void segaai_soundbox_device::device_add_mconfig(machine_config &config)
{
	PIT8253(config, m_tmp8253);
	m_tmp8253->set_clk<0>(21.477272_MHz_XTAL/6);    // ~3.58 MHz, seems to be tied to pin 24 in ym2151
	m_tmp8253->out_handler<0>().set(FUNC(segaai_soundbox_device::tmp8253_out0_w));
	// gate0 not connected
	m_tmp8253->set_clk<1>(21.477272_MHz_XTAL/6);    // 5MHz or 3.58 MHz?
	m_tmp8253->out_handler<1>().set(FUNC(segaai_soundbox_device::tmp8253_out1_w));
    // timer 2 is not connected, also not set up by the code

	I8255(config, m_tmp8255);
	m_tmp8255->in_pa_callback().set(FUNC(segaai_soundbox_device::tmp8255_porta_r));
	// Port B is connected to LC7537N?
	// b0 - pin20 DI
	// b1 - pin21 CLK
	// b2 - pin22 CE
	// b7 - 8253 GATE1
	m_tmp8255->in_pb_callback().set(FUNC(segaai_soundbox_device::tmp8255_portb_r));
	m_tmp8255->out_pb_callback().set(FUNC(segaai_soundbox_device::tmp8255_portb_w));
	m_tmp8255->out_pc_callback().set(FUNC(segaai_soundbox_device::tmp8255_portc_w));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	YM2151(config, m_ym2151, 21.477272_MHz_XTAL/6);   // ~3.58MHz
	m_ym2151->irq_handler().set(FUNC(segaai_soundbox_device::ym2151_irq_w));
	m_ym2151->add_route(0, "lspeaker", 1.00);
	m_ym2151->add_route(1, "rspeaker", 1.00);
}

ROM_START(soundbox)
	ROM_REGION(0x10000, "soundbox", 0)
	ROM_LOAD("ai-snd-2002-cecb.bin", 0x0000, 0x10000, CRC(ef2dabc0) SHA1(b60cd9f6f46b6c77dba8610df6fd83368569e713))
ROM_END

const tiny_rom_entry *segaai_soundbox_device::device_rom_region() const
{
	return ROM_NAME(soundbox);
}

static INPUT_PORTS_START(soundbox)
	PORT_START("ROW0")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW1")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW2")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW3")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW4")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW5")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW6")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW7")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

ioport_constructor segaai_soundbox_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(soundbox);
}

void segaai_soundbox_device::device_start()
{
	m_row = 0;
	m_8255_portb = 0;
	m_ram.resize(0x20000);

	save_item(NAME(m_ram));
	save_item(NAME(m_row));
	save_item(NAME(m_8255_portb));

	mem_space().install_ram(0x20000, 0x3ffff, &m_ram[0]);
	mem_space().install_rom(0x80000, 0x8ffff, m_rom);
	io_space().install_read_handler(0x20, 0x23, emu::rw_delegate(*m_ym2151, FUNC(ym2151_device::read)));
	io_space().install_write_handler(0x20, 0x23, emu::rw_delegate(*m_ym2151, FUNC(ym2151_device::write)));
	io_space().install_read_handler(0x24, 0x27, emu::rw_delegate(*m_tmp8253, FUNC(pit8253_device::read)));
	io_space().install_write_handler(0x24, 0x27, emu::rw_delegate(*m_tmp8253, FUNC(pit8253_device::write)));
	io_space().install_read_handler(0x28, 0x2b, emu::rw_delegate(*m_tmp8255, FUNC(i8255_device::read)));
	io_space().install_write_handler(0x28, 0x2b, emu::rw_delegate(*m_tmp8255, FUNC(i8255_device::write)));
}

u8 segaai_soundbox_device::tmp8255_porta_r()
{
	// Read pressed keys on music keyboard row (see routine @0x82399)
	u8 result = 0xff;
	if (BIT(m_row, 0)) result &= m_rows[0]->read();
	if (BIT(m_row, 1)) result &= m_rows[1]->read();
	if (BIT(m_row, 2)) result &= m_rows[2]->read();
	if (BIT(m_row, 3)) result &= m_rows[3]->read();
	if (BIT(m_row, 4)) result &= m_rows[4]->read();
	if (BIT(m_row, 5)) result &= m_rows[5]->read();
	if (BIT(m_row, 6)) result &= m_rows[6]->read();
	if (BIT(m_row, 7)) result &= m_rows[7]->read();
	return result;
}


u8 segaai_soundbox_device::tmp8255_portb_r()
{
	return 0xff;
}


void segaai_soundbox_device::tmp8255_portb_w(u8 data)
{
	osd_printf_info("soundbox 8255 port B write $%02X\n", data);
	m_tmp8253->write_gate1(BIT(data, 7));
}


void segaai_soundbox_device::tmp8255_portc_w(u8 data)
{
	// Selects music keyboard row to scan (see routine @0x82399)
	osd_printf_info("soundbox m_row = $%02X\n", data);
	m_row = data;
}


void segaai_soundbox_device::ym2151_irq_w(int state)
{
	osd_printf_info("Soundbox: IRQ from ym2151 is '%s'\n", state ? "ASSERT" : "CLEAR");
}


void segaai_soundbox_device::tmp8253_out0_w(int state)
{
//	osd_printf_info("Soundbox: OUT0 from tmp8253 is '%s'\n", state ? "ASSERT" : "CLEAR");
}


void segaai_soundbox_device::tmp8253_out1_w(int state)
{
//	osd_printf_info("Soundbox: OUT1 from tmp8253 is '%s'\n", state ? "ASSERT" : "CLEAR");
}
