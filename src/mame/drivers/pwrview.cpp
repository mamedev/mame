// license:BSD-3-Clause
// copyright-holders:Carl, Al Kossow
/***************************************************************************

    Compugraphic MCS Powerview 10

***************************************************************************/

#include "emu.h"
#include "cpu/i86/i186.h"
#include "machine/upd765.h"
#include "machine/i8251.h"
#include "machine/z80dart.h"
#include "machine/pit8253.h"
#include "screen.h"
#include "video/mc6845.h"
#include "bus/rs232/rs232.h"

class pwrview_state : public driver_device
{
public:
	pwrview_state(const machine_config &mconfig, device_type type, const char *tag) :
	driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_pit(*this, "pit"),
	m_bios(*this, "bios"),
	m_ram(*this, "ram")
	{ }

	DECLARE_READ16_MEMBER(bank0_r);
	DECLARE_WRITE16_MEMBER(bank0_w);
	DECLARE_READ8_MEMBER(unk1_r);
	DECLARE_WRITE8_MEMBER(unk1_w);
	DECLARE_READ8_MEMBER(unk2_r);
	DECLARE_WRITE8_MEMBER(unk2_w);
	DECLARE_READ8_MEMBER(unk3_r);
	DECLARE_WRITE8_MEMBER(unk3_w);
	DECLARE_READ8_MEMBER(led_r);
	DECLARE_WRITE8_MEMBER(led_w);
	DECLARE_READ16_MEMBER(nmiio_r);
	DECLARE_WRITE16_MEMBER(nmiio_w);
	DECLARE_WRITE16_MEMBER(nmimem_w);
	DECLARE_READ8_MEMBER(rotary_r);
	DECLARE_READ8_MEMBER(err_r);
	MC6845_UPDATE_ROW(update_row);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
private:
	required_device<i80186_cpu_device> m_maincpu;
	required_device<pit8253_device> m_pit;
	required_memory_region m_bios;
	required_shared_ptr<u16> m_ram;
	u8 m_leds[2];
	u8 m_switch, m_c001, m_c009, m_c280, m_errcode;
	emu_timer *m_dmahack;
};

void pwrview_state::device_start()
{
	m_dmahack = timer_alloc();
}

void pwrview_state::device_reset()
{
	m_leds[0] = m_leds[1] = 0;
	m_switch = 0xe0;
	m_c001 = m_c009 = 0;
	m_errcode = 0x31;
}

void pwrview_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	m_maincpu->drq0_w(1);
	m_maincpu->drq1_w(1); // TODO: this is unfortunate
}

MC6845_UPDATE_ROW(pwrview_state::update_row)
{

}

READ8_MEMBER(pwrview_state::rotary_r)
{
	return ~m_switch;
}

READ8_MEMBER(pwrview_state::err_r)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	return m_errcode;
}

READ16_MEMBER(pwrview_state::bank0_r)
{
	if(m_c001 & 2)
		return m_ram[offset];
	else
		return m_bios->as_u16(offset);
}

WRITE16_MEMBER(pwrview_state::bank0_w)
{
	if(m_c001 & 2)
		COMBINE_DATA(&m_ram[offset]);
}

READ16_MEMBER(pwrview_state::nmiio_r)
{
	logerror("%s: io nmi at %04x\n",machine().describe_context(), offset*2);
	m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	return 0xff;
}

WRITE16_MEMBER(pwrview_state::nmiio_w)
{
	logerror("%s: io nmi at %04x\n",machine().describe_context(), offset*2);
	m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	switch(offset) // TODO: some connection with faulting address?
	{
		case 0:
			m_errcode = 0xaa;
			break;
		case 0xc00b / 2:
			m_errcode = 0xb2;
			break;
	}
}

WRITE16_MEMBER(pwrview_state::nmimem_w)
{
	logerror("%s: mem nmi at %05x\n",machine().describe_context(), offset*2);
	m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	m_errcode = 0xae; // TODO: ?
}

READ8_MEMBER(pwrview_state::unk1_r)
{
	return m_c001;
}

WRITE8_MEMBER(pwrview_state::unk1_w)
{
	m_c001 = data;
}

READ8_MEMBER(pwrview_state::unk2_r)
{
	return m_c009;
}

WRITE8_MEMBER(pwrview_state::unk2_w)
{
	if(data & 0x40)
		m_dmahack->adjust(attotime::zero, 0, attotime::from_nsec(50));
	else
		m_dmahack->adjust(attotime::never);
	m_c009 = data;
}

READ8_MEMBER(pwrview_state::unk3_r)
{
	switch(offset)
	{
		case 0:
			return m_c280;
	}
	return 0;
}

WRITE8_MEMBER(pwrview_state::unk3_w)
{
	switch(offset)
	{
		case 0:
			m_c280 = data;
			m_pit->set_clockin(0, data & 0x20 ? 1000000 : 0);
			m_pit->set_clockin(1, data & 0x40 ? 1000000 : 0);
			m_pit->set_clockin(2, data & 0x80 ? 1000000 : 0);
			break;
	}
}

READ8_MEMBER(pwrview_state::led_r)
{
	return m_leds[offset];
}

WRITE8_MEMBER(pwrview_state::led_w)
{
	std::function<char (u8)> xlate = [](u8 val) -> char {
		const u8 segxlat[] = { 0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xf8, 0x80, 0x98, 0x88, 0x83, 0xc6, 0xa1, 0x86, 0x8e };
		const char hex[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
		val |= 0x80;
		if(val == 0xff)
			return ' ';
		for(int i = 0; i < 16; i++)
		{
			if(val == segxlat[i])
				return hex[i];
		}
		return '?';
	};
	m_leds[offset] = data;
	if(offset == 1)
	{
		logerror("%c%c%c%c\n", m_leds[1] & 0x80 ? ' ' : '.', xlate(m_leds[1]), m_leds[0] & 0x80 ? ' ' : '.', xlate(m_leds[0]));
		m_c009 &= ~2;
		m_c009 |= (data & 0x80) ? 0 : 2; // TODO: what this means
	}
}

static ADDRESS_MAP_START(pwrview_map, AS_PROGRAM, 16, pwrview_state)
	AM_RANGE(0x00000, 0x003ff) AM_READWRITE(bank0_r, bank0_w)
	AM_RANGE(0x00000, 0xf7fff) AM_RAM AM_SHARE("ram")
	AM_RANGE(0xf8000, 0xfffff) AM_ROM AM_REGION("bios", 0)
	AM_RANGE(0xf8000, 0xfffff) AM_WRITE(nmimem_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(pwrview_io, AS_IO, 16, pwrview_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0xc000, 0xc001) AM_READWRITE8(unk1_r, unk1_w, 0xff00)
	AM_RANGE(0xc002, 0xc005) AM_READWRITE8(led_r, led_w, 0xff00)
	AM_RANGE(0xc006, 0xc007) AM_READ8(rotary_r, 0xff00)
	AM_RANGE(0xc008, 0xc009) AM_READWRITE8(unk2_r, unk2_w, 0xff00)
	AM_RANGE(0xc00a, 0xc00b) AM_READ8(err_r, 0xff00)
	AM_RANGE(0xc00c, 0xc00d) AM_RAM
	AM_RANGE(0xc080, 0xc081) AM_UNMAP
	AM_RANGE(0xc088, 0xc089) AM_DEVWRITE8("crtc", hd6845_device, address_w, 0x00ff)
	AM_RANGE(0xc08a, 0xc08b) AM_DEVREADWRITE8("crtc", hd6845_device, register_r, register_w, 0x00ff)
	AM_RANGE(0xc280, 0xc287) AM_READWRITE8(unk3_r, unk3_w, 0x00ff)
	AM_RANGE(0xc288, 0xc28f) AM_DEVREADWRITE8("pit", pit8253_device, read, write, 0x00ff)
	AM_RANGE(0xc2a0, 0xc2a7) AM_DEVREADWRITE8("sio", z80sio2_device, cd_ba_r, cd_ba_w, 0x00ff)
	AM_RANGE(0xc2c0, 0xc2c1) AM_DEVREADWRITE8("uart", i8251_device, data_r, data_w, 0x00ff)
	AM_RANGE(0xc2c2, 0xc2c3) AM_DEVREADWRITE8("uart", i8251_device, status_r, control_w, 0x00ff)
	AM_RANGE(0xc2e0, 0xc2e3) AM_DEVICE8("fdc", upd765a_device, map, 0x00ff)
	AM_RANGE(0xc2e4, 0xc2e7) AM_RAM
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(nmiio_r, nmiio_w)
ADDRESS_MAP_END

static SLOT_INTERFACE_START(pwrview_floppies)
	SLOT_INTERFACE("525dd", FLOPPY_525_DD)
SLOT_INTERFACE_END

static MACHINE_CONFIG_START( pwrview, pwrview_state )
	MCFG_CPU_ADD("maincpu", I80186, XTAL_16MHz)
	MCFG_CPU_PROGRAM_MAP(pwrview_map)
	MCFG_CPU_IO_MAP(pwrview_io)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_64MHz/8, 480, 0, 384, 1040, 0, 960)  // clock unknown
	MCFG_SCREEN_UPDATE_DEVICE("crtc", hd6845_device, screen_update)

	MCFG_DEVICE_ADD("pit", PIT8253, 0)
	MCFG_PIT8253_CLK0(XTAL_16MHz/16) // clocks unknown, fix above when found
	MCFG_PIT8253_CLK1(XTAL_16MHz/16)
	MCFG_PIT8253_CLK2(XTAL_16MHz/16)

	// floppy disk controller
	MCFG_UPD765A_ADD("fdc", true, true) // Rockwell R7675P
	//MCFG_UPD765_INTRQ_CALLBACK(DEVWRITELINE("pic1", pic8259_device, ir6_w))
	//MCFG_UPD765_DRQ_CALLBACK(DEVWRITELINE("maincpu", i80186_cpu_device, drq1_w))
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", pwrview_floppies, "525dd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", pwrview_floppies, "525dd", floppy_image_device::default_floppy_formats)

	MCFG_DEVICE_ADD("uart", I8251, 0)
	MCFG_Z80SIO2_ADD("sio", 4000000, 0, 0, 0, 0)
	MCFG_DEVICE_ADD("crtc", HD6845, XTAL_64MHz/64) // clock unknown
	MCFG_MC6845_CHAR_WIDTH(32) // ??
	MCFG_MC6845_UPDATE_ROW_CB(pwrview_state, update_row)
MACHINE_CONFIG_END

ROM_START(pwrview)
	ROM_REGION(0x8000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "bios", "bios")
	ROMX_LOAD("215856-003.bin", 0x0000, 0x4000, CRC(1fa2cd11) SHA1(b4755c7d5200a423a750ecf71c0aed33e364138b), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("215856-004.bin", 0x0001, 0x4000, CRC(4fd01e0a) SHA1(c4d1d40d4e8e529c03857f4a3c8428ccf6b8ff99), ROM_SKIP(1) | ROM_BIOS(1))
ROM_END

COMP(1984, pwrview, 0, 0, pwrview, 0, driver_device, 0, "Compugraphic", "MCS PowerView 10", MACHINE_NOT_WORKING)
