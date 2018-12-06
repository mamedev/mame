// license:BSD-3-Clause
// copyright-holders:Carl
/****************************************************************************

    Olivetti M24 emulation

****************************************************************************/

#include "emu.h"

#include "cpu/i86/i86.h"
#include "cpu/tms7000/tms7000.h"
#include "imagedev/floppy.h"
#include "machine/m24_kbd.h"
#include "machine/m24_z8000.h"
#include "machine/mm58274c.h"
#include "machine/genpc.h"

#include "formats/pc_dsk.h"
#include "formats/naslite_dsk.h"
#include "formats/m20_dsk.h"

#include "softlist.h"

class m24_state : public driver_device
{
public:
	m24_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mb(*this, "mb"),
		m_kbc(*this, "kbc"),
		m_keyboard(*this, "keyboard"),
		m_z8000_apb(*this, "z8000_apb")
	{ }

	void olivetti(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<pc_noppi_mb_device> m_mb;
	required_device<tms7000_device> m_kbc;
	required_device<m24_keyboard_device> m_keyboard;
	optional_device<m24_z8000_device> m_z8000_apb;

	DECLARE_READ8_MEMBER(keyboard_r);
	DECLARE_WRITE8_MEMBER(keyboard_w);
	DECLARE_READ8_MEMBER(pa_r);
	DECLARE_WRITE8_MEMBER(pb_w);
	DECLARE_READ8_MEMBER(kbcdata_r);
	DECLARE_WRITE8_MEMBER(kbcdata_w);
	DECLARE_WRITE_LINE_MEMBER(kbcin_w);
	DECLARE_WRITE_LINE_MEMBER(dma_hrq_w);
	DECLARE_WRITE_LINE_MEMBER(int_w);
	DECLARE_WRITE_LINE_MEMBER(halt_i86_w);
	DECLARE_FLOPPY_FORMATS( floppy_formats );

	void machine_reset() override;

	uint8_t m_sysctl, m_pa, m_kbcin, m_kbcout;
	bool m_kbcibf, m_kbdata, m_i86_halt, m_i86_halt_perm;
	static void cfg_m20_format(device_t *device);
	void kbc_map(address_map &map);
	void m24_io(address_map &map);
	void m24_map(address_map &map);
};

void m24_state::machine_reset()
{
	m_sysctl = 0;
	m_pa = 0x40;
	m_kbcibf = false;
	m_kbdata = true;
	m_i86_halt = false;
	m_i86_halt_perm = false;
	if(m_z8000_apb)
		m_z8000_apb->halt_w(ASSERT_LINE);
}

READ8_MEMBER(m24_state::keyboard_r)
{
	switch(offset)
	{
		case 0:
			m_pa |= 0x40;
			m_mb->m_pic8259->ir1_w(0);
			return m_kbcout;
		case 1:
			return m_sysctl;
		case 2:
			return 0;
		case 4:
			return (m_kbcibf ? 2 : 0) | ((m_pa & 0x40) ? 0 : 1);
	}
	return 0xff;
}

WRITE8_MEMBER(m24_state::keyboard_w)
{
	switch(offset)
	{
		case 0:
			m_kbc->set_input_line(TMS7000_INT1_LINE, ASSERT_LINE);
			m_kbcibf = true;
			m_kbcin = data;
			break;
		case 1:
			m_sysctl = data;
			m_mb->m_pit8253->write_gate2(BIT(data, 0));
			m_mb->pc_speaker_set_spkrdata(BIT(data, 1));
			if(BIT(data, 6))
				m_pa |= 4;
			else
				m_pa &= ~4;
			break;
		case 5:
			m_maincpu->set_input_line(INPUT_LINE_HALT, (data & 0x40) ? ASSERT_LINE : CLEAR_LINE);
			m_i86_halt = true;
			m_i86_halt_perm = true;
	}
}

READ8_MEMBER(m24_state::pa_r)
{
	return m_pa & (m_kbdata ? 0xff : 0xfd);
}

WRITE8_MEMBER(m24_state::pb_w)
{
	m_keyboard->clock_w(!BIT(data, 0));
	m_keyboard->data_w(!BIT(data, 1));
	m_pa = (m_pa & ~3) | (~data & 3);
}

READ8_MEMBER(m24_state::kbcdata_r)
{
	m_kbc->set_input_line(TMS7000_INT1_LINE, CLEAR_LINE);
	m_kbcibf = false;
	return m_kbcin;
}

WRITE8_MEMBER(m24_state::kbcdata_w)
{
	m_pa &= ~0x40;
	m_mb->m_pic8259->ir1_w(1);
	m_kbcout = data;
}

WRITE_LINE_MEMBER(m24_state::kbcin_w)
{
	m_kbdata = state;
}

WRITE_LINE_MEMBER(m24_state::dma_hrq_w)
{
	if(!m_i86_halt)
		m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);
	if(m_z8000_apb && !m_z8000_apb->halted())
		m_z8000_apb->halt_w(state ? ASSERT_LINE : CLEAR_LINE);

	/* Assert HLDA */
	m_mb->m_dma8237->hack_w(state);
}

WRITE_LINE_MEMBER(m24_state::int_w)
{
	if(!m_i86_halt)
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, state ? ASSERT_LINE : CLEAR_LINE);
	if(m_z8000_apb && !m_z8000_apb->halted())
		m_z8000_apb->int_w(state ? ASSERT_LINE : CLEAR_LINE);
}

WRITE_LINE_MEMBER(m24_state::halt_i86_w)
{
	if(m_i86_halt_perm)
		return;
	m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);
	m_i86_halt = state ? true : false;
}

void m24_state::m24_map(address_map &map)
{
	map.unmap_value_high();
	map(0xf8000, 0xfffff).rom().region("bios", 0);
}

void m24_state::m24_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x00ff).m(m_mb, FUNC(pc_noppi_mb_device::map));
	map(0x0060, 0x0065).rw(FUNC(m24_state::keyboard_r), FUNC(m24_state::keyboard_w));
	map(0x0066, 0x0067).portr("DSW0");
	map(0x0070, 0x007f).rw("mm58174an", FUNC(mm58274c_device::read), FUNC(mm58274c_device::write));
	map(0x80c1, 0x80c1).rw(m_z8000_apb, FUNC(m24_z8000_device::handshake_r), FUNC(m24_z8000_device::handshake_w));
}

void m24_state::kbc_map(address_map &map)
{
	map(0x8000, 0x8fff).r(FUNC(m24_state::kbcdata_r));
	map(0xa000, 0xafff).w(FUNC(m24_state::kbcdata_w));
	map(0xf800, 0xffff).rom().region("kbc", 0);
}

static INPUT_PORTS_START( m24 )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x8f, 0x89, "RAM banks")
	PORT_DIPSETTING(    0x01, "128K" )
	PORT_DIPSETTING(    0x82, "256K" )
	PORT_DIPSETTING(    0x84, "512K - 256/256" )
	PORT_DIPSETTING(    0x08, "512K - 512/0" )
	PORT_DIPSETTING(    0x85, "640K - 256/384" )
	PORT_DIPSETTING(    0x8d, "640K - 128/512" )
	PORT_DIPSETTING(    0x89, "640K - 512/128" )
	PORT_DIPNAME( 0x10, 0x00, "8087 installed")
	PORT_DIPSETTING(    0x00, DEF_STR(No) )
	PORT_DIPSETTING(    0x10, DEF_STR(Yes) )
	PORT_DIPNAME( 0x20, 0x00, "Serial Port")
	PORT_DIPSETTING(    0x20, "Z8530 SCC")
	PORT_DIPSETTING(    0x00, "INS8250" )

	//PORT_START("DSW1")
	PORT_DIPNAME( 0x0100, 0x0000, "FDD Type")
	PORT_DIPSETTING(    0x0000, "360K" )
	PORT_DIPSETTING(    0x0100, "720K" )
	PORT_DIPNAME( 0x0200, 0x0200, "FDD spinup")
	PORT_DIPSETTING(    0x0000, "Slow" )
	PORT_DIPSETTING(    0x0200, "Fast" )
	PORT_DIPNAME( 0x0400, 0x0400, "HDD ROM")
	PORT_DIPSETTING(    0x0000, "Internal" )
	PORT_DIPSETTING(    0x0400, "External" )
	PORT_DIPNAME( 0x0800, 0x0000, "Scroll rate")
	PORT_DIPSETTING(    0x0800, "Slow" )
	PORT_DIPSETTING(    0x0000, "Fast")
	PORT_DIPNAME( 0x3000, 0x2000, "Graphics adapter")
	PORT_DIPSETTING(    0x0000, "EGA/VGA" )
	PORT_DIPSETTING(    0x1000, "Color 40x25" )
	PORT_DIPSETTING(    0x2000, "Color 80x25" )
	PORT_DIPSETTING(    0x3000, "Monochrome" )
	PORT_DIPNAME( 0xc000, 0x4000, "Number of floppy drives")
	PORT_DIPSETTING(    0x0000, "1" )
	PORT_DIPSETTING(    0x4000, "2" )
	PORT_DIPSETTING(    0x8000, "3" )
	PORT_DIPSETTING(    0xc000, "4" )
INPUT_PORTS_END

FLOPPY_FORMATS_MEMBER( m24_state::floppy_formats )
	FLOPPY_PC_FORMAT,
	FLOPPY_NASLITE_FORMAT,
	FLOPPY_M20_FORMAT
FLOPPY_FORMATS_END

void m24_state::cfg_m20_format(device_t *device)
{
	device->subdevice<floppy_connector>("fdc:0")->set_formats(m24_state::floppy_formats);
	device->subdevice<floppy_connector>("fdc:1")->set_formats(m24_state::floppy_formats);
}

MACHINE_CONFIG_START(m24_state::olivetti)
	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", I8086, XTAL(8'000'000))
	MCFG_DEVICE_PROGRAM_MAP(m24_map)
	MCFG_DEVICE_IO_MAP(m24_io)
	MCFG_DEVICE_IRQ_ACKNOWLEDGE_DEVICE("mb:pic8259", pic8259_device, inta_cb)

	MCFG_PCNOPPI_MOTHERBOARD_ADD("mb", "maincpu")

	// FIXME: determine ISA bus clock
	MCFG_DEVICE_ADD("mb1", ISA8_SLOT, 0, "mb:isa", pc_isa8_cards, "cga_m24", true)
	MCFG_DEVICE_ADD("mb2", ISA8_SLOT, 0, "mb:isa", pc_isa8_cards, "fdc_xt", true)
	MCFG_SLOT_OPTION_MACHINE_CONFIG("fdc_xt", cfg_m20_format)
	MCFG_DEVICE_ADD("mb3", ISA8_SLOT, 0, "mb:isa", pc_isa8_cards, "lpt", true)
	MCFG_DEVICE_ADD("mb4", ISA8_SLOT, 0, "mb:isa", pc_isa8_cards, "com", true)

	MCFG_DEVICE_ADD("isa1", ISA8_SLOT, 0, "mb:isa", pc_isa8_cards, nullptr, false)
	MCFG_DEVICE_ADD("isa2", ISA8_SLOT, 0, "mb:isa", pc_isa8_cards, nullptr, false)
	MCFG_DEVICE_ADD("isa3", ISA8_SLOT, 0, "mb:isa", pc_isa8_cards, nullptr, false)

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("640K").set_extra_options("64K, 128K, 256K, 512K");

	TMS7000(config, m_kbc, XTAL(4'000'000));
	m_kbc->set_addrmap(AS_PROGRAM, &m24_state::kbc_map);
	m_kbc->in_porta().set(FUNC(m24_state::pa_r));
	m_kbc->out_portb().set(FUNC(m24_state::pb_w));

	M24_KEYBOARD(config, m_keyboard, 0);
	m_keyboard->out_data_handler().set(FUNC(m24_state::kbcin_w));

	MCFG_DEVICE_ADD("mm58174an", MM58274C, 0)
	MCFG_MM58274C_MODE24(1) // ?
	MCFG_MM58274C_DAY1(1)   // ?

	M24_Z8000(config, m_z8000_apb, 0);
	m_z8000_apb->halt_callback().set(FUNC(m24_state::halt_i86_w));

	subdevice<am9517a_device>("mb:dma8237")->out_hreq_callback().set(FUNC(m24_state::dma_hrq_w));
	subdevice<pic8259_device>("mb:pic8259")->out_int_callback().set(FUNC(m24_state::int_w));

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("disk_list","ibm5150")
MACHINE_CONFIG_END

ROM_START( m24 )
	ROM_REGION16_LE(0x8000,"bios", 0)
	ROMX_LOAD("olivetti_m24_version_1.43_high.bin",0x4001, 0x2000, CRC(04e697ba) SHA1(1066dcc849e6289b5ac6372c84a590e456d497a6), ROM_SKIP(1))
	ROMX_LOAD("olivetti_m24_version_1.43_low.bin", 0x4000, 0x2000, CRC(ff7e0f10) SHA1(13423011a9bae3f3193e8c199f98a496cab48c0f), ROM_SKIP(1))

	ROM_REGION(0x800, "kbc", 0)
	ROM_LOAD("pdbd.tms2516.kbdmcu_replacement_board.10u", 0x000, 0x800, CRC(b8c4c18a) SHA1(25b4c24e19ff91924c53557c66513ab242d926c6))
ROM_END

ROM_START( m240 )
	ROM_REGION16_LE(0x8000,"bios", 0)
	ROMX_LOAD("olivetti_m240_pch5_2.04_high.bin", 0x0001, 0x4000, CRC(ceb97b59) SHA1(84fabbeab355e0a4c9445910f2b7d1ec98886642), ROM_SKIP(1))
	ROMX_LOAD("olivetti_m240_pch6_2.04_low.bin",  0x0000, 0x4000, CRC(c463aa94) SHA1(a30c763c1ace9f3ff79e7136b252d624108a50ae), ROM_SKIP(1))

	// is this one the same?
	ROM_REGION(0x800, "kbc", 0)
	ROM_LOAD("pdbd.tms2516.kbdmcu_replacement_board.10u", 0x000, 0x800, BAD_DUMP CRC(b8c4c18a) SHA1(25b4c24e19ff91924c53557c66513ab242d926c6))
ROM_END

COMP( 1983, m24,  ibm5150, 0, olivetti, m24, m24_state, empty_init, "Olivetti", "M24",  MACHINE_NOT_WORKING )
COMP( 1987, m240, ibm5150, 0, olivetti, m24, m24_state, empty_init, "Olivetti", "M240", MACHINE_NOT_WORKING )
