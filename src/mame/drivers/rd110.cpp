// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, Jonathan Gevaryahu
/*************************************************************************************************

    Roland D-110 driver

    Driver by Olivier Galibert and Jonathan Gevaryahu

    The Roland D-110 is an expander (synthesizer without the keyboard)
    from 1988.  Internally it's very similar to a mt32, with a better
    LCD screen (16x2) and more control buttons.  More importantly, it
    has more sound rom, a battery-backed ram and a port for memory
    cards allowing to load and save new sounds.

    After the first boot, the ram needs to be reinitialized to factory
    default values.  Press Write/Copy (I) while resetting then
    validate with Enter (K).
*/

#include "emu.h"
#include "machine/ram.h"
#include "machine/nvram.h"
#include "video/msm6222b.h"
#include "cpu/mcs96/i8x9x.h"

static INPUT_PORTS_START( d110 )
	PORT_START("SC0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Write/Copy") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Number +") PORT_CODE(KEYCODE_U)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Bank +") PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Group +") PORT_CODE(KEYCODE_T)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Part +") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Timbre") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Patch") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Exit") PORT_CODE(KEYCODE_Q)

	PORT_START("SC1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Enter") PORT_CODE(KEYCODE_K)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Number -") PORT_CODE(KEYCODE_J)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Bank -") PORT_CODE(KEYCODE_H)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Group -") PORT_CODE(KEYCODE_G)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Part -") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("System") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Part") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Edit") PORT_CODE(KEYCODE_A)
INPUT_PORTS_END

class d110_state : public driver_device
{
public:
	required_device<i8x9x_device> cpu;
	required_device<ram_device> ram;
	required_device<nvram_device> rams;
	required_device<ram_device> memc;
	required_device<nvram_device> memcs;
	required_device<msm6222b_device> lcd;
	required_device<timer_device> midi_timer;

	d110_state(const machine_config &mconfig, device_type type, std::string tag);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	DECLARE_PALETTE_INIT(d110);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE8_MEMBER(bank_w);
	DECLARE_WRITE8_MEMBER(so_w);
	DECLARE_WRITE16_MEMBER(midi_w);

	DECLARE_READ8_MEMBER(lcd_ctrl_r);
	DECLARE_WRITE8_MEMBER(lcd_ctrl_w);
	DECLARE_WRITE8_MEMBER(lcd_data_w);
	DECLARE_READ16_MEMBER(port0_r);

	TIMER_DEVICE_CALLBACK_MEMBER(midi_timer_cb);
	TIMER_DEVICE_CALLBACK_MEMBER(samples_timer_cb);

private:
	UINT8 lcd_data_buffer[256];
	int lcd_data_buffer_pos;
	UINT8 midi;
	int midi_pos;
	UINT8 port0;
	required_device<cpu_device> m_maincpu;
};

d110_state::d110_state(const machine_config &mconfig, device_type type, std::string tag) :
	driver_device(mconfig, type, tag),
	cpu(*this, "maincpu"),
	ram(*this, "ram"),
	rams(*this, "rams"),
	memc(*this, "memc"),
	memcs(*this, "memcs"),
	lcd(*this, "lcd"),
	midi_timer(*this, "midi_timer")
,
		m_maincpu(*this, "maincpu") {
}


UINT32 d110_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0);
	const UINT8 *data = lcd->render();
	for(int l=0; l<2; l++)
		for(int c=0; c<20; c++)
			for(int y=0; y<8; y++) {
				UINT8 v = data[c*16 + l*40*16 + y];
				for(int x=0; x<5; x++)
					bitmap.pix16(y+9*l, c*6+x) = v & (0x10 >> x) ? 1 : 0;
			}
	return 0;
}

void d110_state::machine_start()
{
	rams->set_base(ram->pointer(), 32768);
	memcs->set_base(memc->pointer(), 32768);

	membank("bank")->configure_entries(0x00, 4, memregion("maincpu")->base(), 0x4000);
	membank("bank")->configure_entries(0x10, 2, ram->pointer(), 0x4000);
	membank("bank")->configure_entries(0x20, 8, memregion("presets")->base(), 0x4000);
	membank("bank")->configure_entries(0x30, 2, memc->pointer(), 0x4000);
	membank("fixed")->set_base(ram->pointer());

	lcd_data_buffer_pos = 0;
}

void d110_state::machine_reset()
{
	//  midi_timer->adjust(attotime::from_hz(1));
	midi_pos = 0;
	port0 = 0x80; // battery ok
}

WRITE8_MEMBER(d110_state::lcd_ctrl_w)
{
	lcd->control_w(data);
	for(int i=0; i != lcd_data_buffer_pos; i++)
		lcd->data_w(lcd_data_buffer[i]);
	lcd_data_buffer_pos = 0;
}

READ8_MEMBER(d110_state::lcd_ctrl_r)
{
	// Busy flag in the msm622b is bit 7, while the software expects it in bit 0...
	return lcd->control_r() >> 7;
}

WRITE8_MEMBER(d110_state::lcd_data_w)
{
	if(lcd_data_buffer_pos == sizeof(lcd_data_buffer)) {
		logerror("Warning: lcd data buffer overflow (%04x)\n", cpu->pc());
		return;
	}
	lcd_data_buffer[lcd_data_buffer_pos++] = data;
}

WRITE8_MEMBER(d110_state::bank_w)
{
	membank("bank")->set_entry(data);
}

WRITE16_MEMBER(d110_state::midi_w)
{
	logerror("midi_out %02x\n", data);
	midi = data;
}

TIMER_DEVICE_CALLBACK_MEMBER(d110_state::midi_timer_cb)
{
	const static UINT8 midi_data[3] = { 0x91, 0x40, 0x7f };
	midi = midi_data[midi_pos++];
	logerror("midi_in %02x\n", midi);
	cpu->serial_w(midi);
	if(midi_pos < sizeof(midi_data))
		midi_timer->adjust(attotime::from_hz(1250));
}

READ16_MEMBER(d110_state::port0_r)
{
	return port0;
}

TIMER_DEVICE_CALLBACK_MEMBER(d110_state::samples_timer_cb)
{
	port0 ^= 0x10;
}

WRITE8_MEMBER(d110_state::so_w)
{
	// bit 0   = led
	// bit 1-2 = reverb program a13/a14
	// bit 3   = R. SW. to ananlog board
	// bit 5   = boss 8Mhz clock, handled internally
	//  logerror("so: rw=%d bank=%d led=%d\n", (data >> 3) & 1, (data >> 1) & 3, data & 1);
}

PALETTE_INIT_MEMBER(d110_state, d110)
{
	palette.set_pen_color(0, rgb_t(0, 255, 0));
	palette.set_pen_color(1, rgb_t(0, 0, 0));
}

static ADDRESS_MAP_START( d110_map, AS_PROGRAM, 8, d110_state )
	AM_RANGE(0x0100, 0x0100) AM_WRITE(bank_w)
	AM_RANGE(0x0200, 0x0200) AM_WRITE(so_w)
	AM_RANGE(0x021a, 0x021a) AM_READ_PORT("SC0") AM_WRITENOP
	AM_RANGE(0x021c, 0x021c) AM_READ_PORT("SC1")
	AM_RANGE(0x0300, 0x0300) AM_WRITE(lcd_data_w)
	AM_RANGE(0x0380, 0x0380) AM_READWRITE(lcd_ctrl_r, lcd_ctrl_w)
	AM_RANGE(0x1000, 0x7fff) AM_ROM AM_REGION("maincpu", 0x1000)
	AM_RANGE(0x8000, 0xbfff) AM_RAMBANK("bank")
	AM_RANGE(0xc000, 0xffff) AM_RAMBANK("fixed")
ADDRESS_MAP_END

static ADDRESS_MAP_START( d110_io, AS_IO, 16, d110_state )
	AM_RANGE(i8x9x_device::SERIAL, i8x9x_device::SERIAL) AM_WRITE(midi_w)
	AM_RANGE(i8x9x_device::P0,     i8x9x_device::P0)     AM_READ(port0_r)
ADDRESS_MAP_END

static MACHINE_CONFIG_START( d110, d110_state )
	MCFG_CPU_ADD( "maincpu", P8098, XTAL_12MHz )
	MCFG_CPU_PROGRAM_MAP( d110_map )
	MCFG_CPU_IO_MAP( d110_io )

// Battery-backed main ram
	MCFG_RAM_ADD( "ram" )
	MCFG_RAM_DEFAULT_SIZE( "32K" )
	MCFG_NVRAM_ADD_0FILL( "rams" )


// Shall become a proper memcard device someday
	MCFG_RAM_ADD( "memc" )
	MCFG_RAM_DEFAULT_SIZE( "32K" )
	MCFG_NVRAM_ADD_0FILL( "memcs" )

	MCFG_SCREEN_ADD( "screen", LCD )
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_UPDATE_DRIVER(d110_state, screen_update)
//  MCFG_SCREEN_SIZE(20*6-1, 2*9-1)
	MCFG_SCREEN_SIZE(16*6-1, (16*6-1)*3/4)
	MCFG_SCREEN_VISIBLE_AREA(0, 16*6-2, 0, (16*6-1)*3/4-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 2)
	MCFG_PALETTE_INIT_OWNER(d110_state, d110)

	MCFG_MSM6222B_01_ADD( "lcd" )

	MCFG_TIMER_DRIVER_ADD( "midi_timer", d110_state, midi_timer_cb )

	MCFG_TIMER_DRIVER_ADD_PERIODIC( "samples_timer", d110_state, samples_timer_cb, attotime::from_hz(32000*2) )
MACHINE_CONFIG_END

ROM_START( d110 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_DEFAULT_BIOS( "110" )

	ROM_SYSTEM_BIOS( 0, "106", "Firmware 1.06" )
	ROMX_LOAD( "d-110.v1.06.ic19.bin",         0,   0x8000, CRC(3dd5b6e9) SHA1(73b155fb0a8adc2362e73cb0803dafba9ccfb508), ROM_BIOS(1) )

	ROM_SYSTEM_BIOS( 1, "110", "Firmware 1.10" )
	ROMX_LOAD( "d-110.v1.10.ic19.bin",         0,   0x8000, CRC(3ae68187) SHA1(28635510f30d6c1fb88e00da03e5b4e045c380cb), ROM_BIOS(2) )

	ROM_REGION( 0x20000, "presets", 0 )
	ROM_LOAD(  "r15179873-lh5310-97.ic12.bin", 0,  0x20000, CRC(580a8f9e) SHA1(05587a0542b01625dcde37de5bb339880e47eb93) )

	ROM_REGION( 0x100000, "la32", 0 )
	ROM_LOAD(  "r15179878.ic7.bin",            0,  0x80000, CRC(e117e6ab) SHA1(6760d14900161b8715c2bfd4ebe997877087c90c) )
	ROM_LOAD(  "r15179880.ic8.bin",      0x80000,  0x80000, CRC(b329f945) SHA1(9c59f50518a070461b2ec6cb4e43ee7cc1e905b6) )

	ROM_REGION( 0x8000, "boss", 0 )
	ROM_LOAD(  "r15179879.ic6.bin",            0,   0x8000, CRC(5d34174e) SHA1(17bd2887711c5c5458aba6d3be5972b2096eb450) )
ROM_END

CONS( 1988, d110,  0, 0, d110, d110, driver_device, 0, "Roland", "D110",  MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
