// license:BSD-3-Clause
// copyright-holders:Carl, Al Kossow
/***************************************************************************

    Compugraphic MCS Powerview 10

***************************************************************************/

#include "emu.h"
#include "cpu/i86/i186.h"
#include "imagedev/floppy.h"
#include "machine/upd765.h"
#include "machine/i8251.h"
#include "machine/z80dart.h"
#include "machine/pit8253.h"
#include "machine/bankdev.h"
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
		m_ram(*this, "ram"),
		m_biosbank(*this, "bios_bank"),
		m_vram(64*1024)
	{ }

	void pwrview(machine_config &config);

private:
	DECLARE_READ16_MEMBER(bank0_r);
	DECLARE_WRITE16_MEMBER(bank0_w);
	DECLARE_READ8_MEMBER(unk1_r);
	DECLARE_WRITE8_MEMBER(unk1_w);
	DECLARE_READ8_MEMBER(unk2_r);
	DECLARE_WRITE8_MEMBER(unk2_w);
	DECLARE_READ8_MEMBER(unk3_r);
	DECLARE_WRITE8_MEMBER(unk3_w);
	DECLARE_READ8_MEMBER(unk4_r);
	DECLARE_WRITE8_MEMBER(unk4_w);
	DECLARE_READ8_MEMBER(led_r);
	DECLARE_WRITE8_MEMBER(led_w);
	DECLARE_READ8_MEMBER(pitclock_r);
	DECLARE_READ16_MEMBER(nmiio_r);
	DECLARE_WRITE16_MEMBER(nmiio_w);
	DECLARE_WRITE16_MEMBER(nmimem_w);
	DECLARE_READ16_MEMBER(vram1_r);
	DECLARE_WRITE16_MEMBER(vram1_w);
	DECLARE_READ16_MEMBER(vram2_r);
	DECLARE_WRITE16_MEMBER(vram2_w);
	DECLARE_READ16_MEMBER(fbios_r);
	DECLARE_READ8_MEMBER(rotary_r);
	DECLARE_READ8_MEMBER(err_r);
	MC6845_UPDATE_ROW(update_row);

	void bios_bank(address_map &map);
	void pwrview_fetch_map(address_map &map);
	void pwrview_io(address_map &map);
	void pwrview_map(address_map &map);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	required_device<i80186_cpu_device> m_maincpu;
	required_device<pit8253_device> m_pit;
	required_memory_region m_bios;
	required_shared_ptr<u16> m_ram;
	required_device<address_map_bank_device> m_biosbank;
	std::vector<u16> m_vram;
	u8 m_leds[2];
	u8 m_switch, m_c001, m_c009, m_c280, m_c080, m_errcode, m_vramwin[2];
	emu_timer *m_dmahack;
	emu_timer *m_tmr0ext;
	enum {
		DMA_TIMER,
		TMR0_TIMER
	};
};

void pwrview_state::device_start()
{
	save_item(NAME(m_vram));
	m_dmahack = timer_alloc(DMA_TIMER);
	m_tmr0ext = timer_alloc(TMR0_TIMER);
	membank("vram1")->configure_entries(0, 0x400, &m_vram[0], 0x80);
	membank("vram2")->configure_entries(0, 0x400, &m_vram[0], 0x80);
}

void pwrview_state::device_reset()
{
	m_leds[0] = m_leds[1] = 0;
	m_switch = 0xe0;
	m_c001 = m_c009 = m_c080 = 0;
	m_errcode = 0x31;
	membank("vram1")->set_entry(0);
	membank("vram2")->set_entry(0);
	m_vramwin[0] = m_vramwin[1] = 0;
	m_biosbank->set_bank(0);
}

void pwrview_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
		case DMA_TIMER:
			m_maincpu->drq0_w(1);
			m_maincpu->drq1_w(1); // TODO: this is unfortunate
			break;
		case TMR0_TIMER:
			m_maincpu->tmrin0_w(ASSERT_LINE);
			m_maincpu->tmrin0_w(CLEAR_LINE);
			break;
	}
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
	logerror("%s: mem nmi at %05x\n",machine().describe_context(), ((offset & 0x7fff) * 2) + 0xf8000);
	m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	switch(((offset & 0x7fff) * 2) + 0x8000) // TODO: some connection with faulting address?
	{
		case 0x82e4:
			m_errcode = 0xae;
			break;
		case 0xbe80:
			m_errcode = 0xa6;
			break;
		case 0xbefc:
			m_errcode = 0xb6;
			break;
	}
}

READ16_MEMBER(pwrview_state::fbios_r)
{
	switch(m_c009 & 0xc)
	{
		case 0x0:
		case 0x4:
			return m_bios->as_u16(offset);
		case 0x8:
			return m_ram[offset + 0xf8000/2];
		case 0xc:
			return 0;
	}
	return 0;
}

READ16_MEMBER(pwrview_state::vram1_r)
{
	return m_vramwin[0];
}

WRITE16_MEMBER(pwrview_state::vram1_w)
{
	data &= 0x3ff;
	membank("vram1")->set_entry(data);
	m_vramwin[0] = data;
}

READ16_MEMBER(pwrview_state::vram2_r)
{
	return m_vramwin[1];
}

WRITE16_MEMBER(pwrview_state::vram2_w)
{
	data &= 0x3ff;
	membank("vram2")->set_entry(data);
	m_vramwin[1] = data;
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
	m_biosbank->set_bank((data >> 2) & 3);
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

READ8_MEMBER(pwrview_state::unk4_r)
{
	return m_c080;
}

WRITE8_MEMBER(pwrview_state::unk4_w)
{
	m_c080 = data;
	if(!BIT(data, 7))
	{
		m_tmr0ext->adjust(attotime::never);
		return;
	}
	switch(data & 7) // this is all hand tuned to match the expected ratio with the pit clock
	{
		case 2:
			m_tmr0ext->adjust(attotime::from_hz(31500), 0, attotime::from_hz(31500));
			break;
		case 3:
			m_tmr0ext->adjust(attotime::from_hz(90), 0, attotime::from_hz(90));
			break;
		case 4:
			m_tmr0ext->adjust(attotime::from_hz(500000), 0, attotime::from_hz(500000));
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

READ8_MEMBER(pwrview_state::pitclock_r)
{
	m_pit->write_clk0(ASSERT_LINE);
	m_pit->write_clk0(CLEAR_LINE);
	return 0;
}

void pwrview_state::bios_bank(address_map &map)
{
	map(0x00000, 0x07fff).rom().region("bios", 0);
	map(0x00000, 0x07fff).w(FUNC(pwrview_state::nmimem_w));

	map(0x08000, 0x0ffff).w(FUNC(pwrview_state::nmimem_w));
	map(0x0be00, 0x0be7f).bankrw("vram1");
	map(0x0befe, 0x0beff).rw(FUNC(pwrview_state::vram1_r), FUNC(pwrview_state::vram1_w));
	map(0x0bf00, 0x0bf7f).bankrw("vram2");
	map(0x0bffe, 0x0bfff).rw(FUNC(pwrview_state::vram2_r), FUNC(pwrview_state::vram2_w));
	map(0x0c000, 0x0ffff).rom().region("bios", 0x4000);

	map(0x10000, 0x17fff).ram();

	map(0x18000, 0x1ffff).w(FUNC(pwrview_state::nmimem_w));

	map(0x1be00, 0x1be7f).bankrw("vram1");
	map(0x1befe, 0x1beff).rw(FUNC(pwrview_state::vram1_r), FUNC(pwrview_state::vram1_w));
	map(0x1bf00, 0x1bf7f).bankrw("vram2");
	map(0x1bffe, 0x1bfff).rw(FUNC(pwrview_state::vram2_r), FUNC(pwrview_state::vram2_w));
	map(0x1c000, 0x1ffff).rom().region("bios", 0x4000);
}

void pwrview_state::pwrview_map(address_map &map)
{
	map(0x00000, 0xf7fff).ram().share("ram");
	map(0x00000, 0x003ff).rw(FUNC(pwrview_state::bank0_r), FUNC(pwrview_state::bank0_w));
	map(0xf8000, 0xfffff).m(m_biosbank, FUNC(address_map_bank_device::amap16));
}

void pwrview_state::pwrview_fetch_map(address_map &map)
{
	map(0x00000, 0xf7fff).ram().share("ram");
	map(0x00000, 0x003ff).r(FUNC(pwrview_state::bank0_r));
	map(0xf8000, 0xfffff).r(FUNC(pwrview_state::fbios_r));
}

void pwrview_state::pwrview_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).rw(FUNC(pwrview_state::nmiio_r), FUNC(pwrview_state::nmiio_w));
	map(0xc001, 0xc001).rw(FUNC(pwrview_state::unk1_r), FUNC(pwrview_state::unk1_w));
	map(0xc002, 0xc005).rw(FUNC(pwrview_state::led_r), FUNC(pwrview_state::led_w)).umask16(0xff00);
	map(0xc007, 0xc007).r(FUNC(pwrview_state::rotary_r));
	map(0xc009, 0xc009).rw(FUNC(pwrview_state::unk2_r), FUNC(pwrview_state::unk2_w));
	map(0xc00b, 0xc00b).r(FUNC(pwrview_state::err_r));
	map(0xc00c, 0xc00d).ram();
	map(0xc080, 0xc080).rw(FUNC(pwrview_state::unk4_r), FUNC(pwrview_state::unk4_w));
	map(0xc088, 0xc088).w("crtc", FUNC(hd6845s_device::address_w));
	map(0xc08a, 0xc08a).rw("crtc", FUNC(hd6845s_device::register_r), FUNC(hd6845s_device::register_w));
	map(0xc280, 0xc287).rw(FUNC(pwrview_state::unk3_r), FUNC(pwrview_state::unk3_w)).umask16(0x00ff);
	map(0xc288, 0xc28f).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write)).umask16(0x00ff);
	map(0xc2a0, 0xc2a7).rw("sio", FUNC(z80sio2_device::cd_ba_r), FUNC(z80sio2_device::cd_ba_w)).umask16(0x00ff);
	map(0xc2c0, 0xc2c3).rw("uart", FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0x00ff);
	map(0xc2e0, 0xc2e3).m("fdc", FUNC(upd765a_device::map)).umask16(0x00ff);
	map(0xc2e4, 0xc2e5).ram();
	map(0xc2e6, 0xc2e6).r(FUNC(pwrview_state::pitclock_r));
}

static void pwrview_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
}

void pwrview_state::pwrview(machine_config &config)
{
	I80186(config, m_maincpu, XTAL(16'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &pwrview_state::pwrview_map);
	m_maincpu->set_addrmap(AS_OPCODES, &pwrview_state::pwrview_fetch_map);
	m_maincpu->set_addrmap(AS_IO, &pwrview_state::pwrview_io);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(64'000'000)/8, 480, 0, 384, 1040, 0, 960);  // clock unknown
	screen.set_screen_update("crtc", FUNC(hd6845s_device::screen_update));

	PIT8253(config, m_pit, 0);
	m_pit->set_clk<0>(XTAL(16'000'000)/16); // clocks unknown, fix above when found
	m_pit->set_clk<1>(XTAL(16'000'000)/16);
	m_pit->set_clk<2>(XTAL(16'000'000)/16);

	// floppy disk controller
	UPD765A(config, "fdc", 8'000'000, true, true); // Rockwell R7675P
	//fdc.intrq_wr_callback().set("pic1", FUNC(pic8259_device::ir6_w));
	//fdc.drq_wr_callback().set(m_maincpu, FUNC(i80186_cpu_device::drq1_w));
	FLOPPY_CONNECTOR(config, "fdc:0", pwrview_floppies, "525dd", floppy_image_device::default_floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", pwrview_floppies, "525dd", floppy_image_device::default_floppy_formats);

	I8251(config, "uart", 0);

	Z80SIO2(config, "sio", 4000000);

	hd6845s_device &crtc(HD6845S(config, "crtc", XTAL(64'000'000)/64)); // clock unknown
	crtc.set_char_width(32);   /* ? */
	crtc.set_update_row_callback(FUNC(pwrview_state::update_row), this);

	ADDRESS_MAP_BANK(config, "bios_bank").set_map(&pwrview_state::bios_bank).set_options(ENDIANNESS_LITTLE, 16, 17, 0x8000);
}

ROM_START(pwrview)
	ROM_REGION(0x8000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "bios", "bios")
	ROMX_LOAD("215856-003.bin", 0x0000, 0x4000, CRC(1fa2cd11) SHA1(b4755c7d5200a423a750ecf71c0aed33e364138b), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("215856-004.bin", 0x0001, 0x4000, CRC(4fd01e0a) SHA1(c4d1d40d4e8e529c03857f4a3c8428ccf6b8ff99), ROM_SKIP(1) | ROM_BIOS(0))
ROM_END

COMP(1984, pwrview, 0, 0, pwrview, 0, pwrview_state, empty_init, "Compugraphic", "MCS PowerView 10", MACHINE_NOT_WORKING)
