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
- LC7537N


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

#include "machine/i8255.h"
#include "machine/pit8253.h"
#include "sound/ymopm.h"

#include "speaker.h"

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"


namespace {

class segaai_soundbox_device : public device_t,
								public segaai_exp_interface
{
public:
	segaai_soundbox_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, SEGAAI_SOUNDBOX, tag, owner, clock)
		, segaai_exp_interface(mconfig, *this)
		, m_tmp8253(*this, "tmp8253")
		, m_tmp8255(*this, "tmp8255")
		, m_ym2151(*this, "ym2151")
		, m_rom(*this, "soundbox")
		, m_rows(*this, "ROW%u", 0U)
		, m_row(0)
	{ }

	static constexpr feature_type unemulated_features() { return feature::KEYBOARD; }

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	static constexpr u32 EXPANSION_RAM_SIZE = 0x20000;

	u8 tmp8255_porta_r();
	void tmp8255_portb_w(u8 data);
	void tmp8255_portc_w(u8 data);
	void ym2151_irq_w(int state);
	void tmp8253_out0_w(int state);
	void tmp8253_out1_w(int state);

	required_device<pit8253_device> m_tmp8253;
	required_device<i8255_device> m_tmp8255;
	required_device<ym2151_device> m_ym2151;
	required_region_ptr<u8> m_rom;
	required_ioport_array<8> m_rows;
	std::unique_ptr<u8[]> m_ram;
	u8 m_row;
};

void segaai_soundbox_device::device_add_mconfig(machine_config &config)
{
	PIT8253(config, m_tmp8253);
	m_tmp8253->set_clk<0>(clock());    // ~3.58 MHz, seems to be tied to pin 24 in ym2151
	m_tmp8253->out_handler<0>().set(FUNC(segaai_soundbox_device::tmp8253_out0_w));
	// gate0 not connected
	m_tmp8253->set_clk<1>(clock());    // 3.58 MHz?
	m_tmp8253->out_handler<1>().set(FUNC(segaai_soundbox_device::tmp8253_out1_w));
	// timer 2 is not connected, also not set up by the code

	I8255(config, m_tmp8255);
	m_tmp8255->in_pa_callback().set(FUNC(segaai_soundbox_device::tmp8255_porta_r));
	m_tmp8255->in_pb_callback().set_constant(0xff);
	m_tmp8255->out_pb_callback().set(FUNC(segaai_soundbox_device::tmp8255_portb_w));
	m_tmp8255->out_pc_callback().set(FUNC(segaai_soundbox_device::tmp8255_portc_w));

	SPEAKER(config, "speaker", 2).front();
	YM2151(config, m_ym2151, DERIVED_CLOCK(1,1));   // ~3.58MHz
	m_ym2151->irq_handler().set(FUNC(segaai_soundbox_device::ym2151_irq_w));
	m_ym2151->add_route(0, "speaker", 1.00, 0);
	m_ym2151->add_route(1, "speaker", 1.00, 1);
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
	m_ram = std::make_unique<u8[]>(EXPANSION_RAM_SIZE);

	save_pointer(NAME(m_ram), EXPANSION_RAM_SIZE);
	save_item(NAME(m_row));

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
	for (int i = 0; i < 8; i++)
		if (BIT(m_row, i)) result &= m_rows[i]->read();
	return result;
}

/*
 8255 port B

 76543210
 +-------- 8253 GATE1
  +-------
   +------
    +-----
     +----
      +--- LC7537N pin22 DI
       +-- LC7537N pin21 CLK
        +- LC7537N pin20 DI
*/
void segaai_soundbox_device::tmp8255_portb_w(u8 data)
{
	LOG("soundbox 8255 port B write $%02X\n", data);
	m_tmp8253->write_gate1(BIT(data, 7));
}

void segaai_soundbox_device::tmp8255_portc_w(u8 data)
{
	// Selects music keyboard row to scan (see routine @0x82399)
	LOG("soundbox m_row = $%02X\n", data);
	m_row = data;
}

void segaai_soundbox_device::ym2151_irq_w(int state)
{
	LOG("Soundbox: IRQ from ym2151 is '%s'\n", state ? "ASSERT" : "CLEAR");
}

void segaai_soundbox_device::tmp8253_out0_w(int state)
{
//  LOG("Soundbox: OUT0 from tmp8253 is '%s'\n", state ? "ASSERT" : "CLEAR");
}

void segaai_soundbox_device::tmp8253_out1_w(int state)
{
//  LOG("Soundbox: OUT1 from tmp8253 is '%s'\n", state ? "ASSERT" : "CLEAR");
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(SEGAAI_SOUNDBOX, segaai_exp_interface, segaai_soundbox_device, "segaai_soundbox", "Sega AI Expansion - Soundbox")
