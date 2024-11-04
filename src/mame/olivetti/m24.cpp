// license:BSD-3-Clause
// copyright-holders:Carl
/****************************************************************************

    Olivetti M24 emulation

    http://olivettim24.hadesnet.org/index.html
    https://sites.google.com/site/att6300shrine/Home
    http://www.ti99.com/exelvision/website/index.php?page=logabax-persona-1600

    The AT&T PC6300, the Xerox 6060 and the Logabax Persona 1600 were
    badge-engineered Olivetti M24s.

    The Olivetti M21 was a portable version of the M24 that sported a 9"
    monochrome monitor.

    http://www.computinghistory.org.uk/det/43175/Olivetti-M21/
    https://www.nightfallcrew.com/23/02/2014/repairing-a-defective-olivetti-m21/

****************************************************************************/

#include "emu.h"

#include "bus/isa/isa.h"
#include "bus/isa/isa_cards.h"
#include "cpu/i86/i86.h"
#include "cpu/tms7000/tms7000.h"
#include "imagedev/floppy.h"
#include "machine/am9517a.h"
#include "machine/i8087.h"
#include "m24_kbd.h"
#include "m24_z8000.h"
#include "machine/mm58174.h"
#include "machine/pit8253.h"
#include "machine/pic8259.h"
#include "machine/ram.h"
#include "sound/spkrdev.h"
#include "speaker.h"

#include "formats/naslite_dsk.h"
#include "formats/m20_dsk.h"

#include "softlist_dev.h"


namespace {

class m24_state : public driver_device
{
public:
	m24_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ram(*this, RAM_TAG),
		m_isabus(*this, "isabus"),
		m_dmac(*this, "dmac"),
		m_pic(*this, "pic"),
		m_pit(*this, "pit"),
		m_speaker(*this, "speaker"),
		m_kbc(*this, "kbc"),
		m_keyboard(*this, "keyboard"),
		m_z8000_apb(*this, "z8000_apb"),
		m_dsw0(*this, "DSW0"),
		m_nmi_enable(false)
	{ }

	void olivetti(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void dma_segment_w(offs_t offset, u8 data);
	void dma_hrq_w(int state);
	u8 dma_memory_read(offs_t offset);
	void dma_memory_write(offs_t offset, u8 data);
	template <int Channel> u8 dma_io_read(offs_t offset);
	template <int Channel> void dma_io_write(offs_t offset, u8 data);
	template <int Channel> void dma_dack_w(int state);
	void dma_tc_w(int state);
	void dreq0_ck_w(int state);
	void speaker_ck_w(int state);
	void update_speaker();

	u8 keyboard_data_r();
	u8 keyboard_status_r();
	void keyboard_data_w(u8 data);

	void ctrlport_a_w(u8 data);
	u8 ctrlport_a_r();
	u8 ctrlport_b_r();

	void alt_w(u8 data);
	void chck_w(int state);
	void int87_w(int state);
	void nmi_enable_w(u8 data);
	void update_nmi();

	required_device<i8086_cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<isa8_device> m_isabus;
	required_device<am9517a_device> m_dmac;
	required_device<pic8259_device> m_pic;
	required_device<pit8253_device> m_pit;
	required_device<speaker_sound_device> m_speaker;
	required_device<tms7000_device> m_kbc;
	required_device<m24_keyboard_device> m_keyboard;
	optional_device<m24_z8000_device> m_z8000_apb;
	required_ioport m_dsw0;

	u8 m_dma_segment[4];
	u8 m_dma_active;
	bool m_tc;
	bool m_dreq0_ck;

	u8 m_ctrlport_a;
	u8 m_ctrlport_b;

	bool m_87int;
	bool m_chck_active;
	bool m_nmi_enable;

	u8 m_pa, m_kbcin, m_kbcout;
	bool m_kbcibf, m_kbdata, m_i86_halt, m_i86_halt_perm;

	u8 pa_r();
	void pb_w(u8 data);
	u8 kbcdata_r();
	void kbcdata_w(u8 data);
	void kbcin_w(int state);
	void int_w(int state);
	void halt_i86_w(int state);
	static void floppy_formats(format_registration &fr);

	static void cfg_m20_format(device_t *device);
	void kbc_map(address_map &map) ATTR_COLD;
	void m24_io(address_map &map) ATTR_COLD;
	void m24_map(address_map &map) ATTR_COLD;
};

void m24_state::machine_start()
{
	m_maincpu->space(AS_PROGRAM).install_ram(0, m_ram->size() - 1, m_ram->pointer());

	std::fill_n(&m_dma_segment[0], 4, 0);
	m_dma_active = 0;
	m_tc = false;
	m_dreq0_ck = true;

	m_ctrlport_a = 0;
	m_ctrlport_b = 0;

	m_87int = false;
	m_chck_active = false;
	m_nmi_enable = false;

	save_item(NAME(m_dma_segment));
	save_item(NAME(m_dma_active));
	save_item(NAME(m_tc));
	save_item(NAME(m_dreq0_ck));
	save_item(NAME(m_ctrlport_a));
	save_item(NAME(m_ctrlport_b));
	save_item(NAME(m_87int));
	save_item(NAME(m_chck_active));
	save_item(NAME(m_nmi_enable));
	save_item(NAME(m_pa));
	save_item(NAME(m_kbcin));
	save_item(NAME(m_kbcout));
	save_item(NAME(m_kbcibf));
	save_item(NAME(m_kbdata));
	save_item(NAME(m_i86_halt));
	save_item(NAME(m_i86_halt_perm));
}

void m24_state::machine_reset()
{
	ctrlport_a_w(0);
	nmi_enable_w(0);
	m_pa = 0x40;
	m_kbcibf = false;
	m_kbdata = true;
	m_i86_halt = false;
	m_i86_halt_perm = false;
	if(m_z8000_apb)
		m_z8000_apb->halt_w(ASSERT_LINE);
}

void m24_state::dma_segment_w(offs_t offset, u8 data)
{
	m_dma_segment[offset] = data & 0x0f;
}

void m24_state::dma_hrq_w(int state)
{
	if(!m_i86_halt)
		m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);
	if(m_z8000_apb && !m_z8000_apb->halted())
		m_z8000_apb->halt_w(state ? ASSERT_LINE : CLEAR_LINE);

	/* Assert HLDA */
	m_dmac->hack_w(state);
}

u8 m24_state::dma_memory_read(offs_t offset)
{
	const int seg = (BIT(m_dma_active, 2) ? 0 : 2) | (BIT(m_dma_active, 3) ? 0 : 1);
	return m_maincpu->space(AS_PROGRAM).read_byte(offset | u32(m_dma_segment[seg]) << 16);
}

void m24_state::dma_memory_write(offs_t offset, u8 data)
{
	const int seg = (BIT(m_dma_active, 2) ? 0 : 2) | (BIT(m_dma_active, 3) ? 0 : 1);
	m_maincpu->space(AS_PROGRAM).write_byte(offset | u32(m_dma_segment[seg]) << 16, data);
}

template <int Channel>
u8 m24_state::dma_io_read(offs_t offset)
{
	return m_isabus->dack_r(Channel);
}

template <int Channel>
void m24_state::dma_io_write(offs_t offset, u8 data)
{
	m_isabus->dack_w(Channel, data);
}

template <int Channel>
void m24_state::dma_dack_w(int state)
{
	m_isabus->dack_line_w(Channel, state);

	if (!state)
	{
		m_dma_active |= 1 << Channel;
		if (Channel == 0)
			m_dmac->dreq0_w(0);
		if (m_tc)
			m_isabus->eop_w(Channel, ASSERT_LINE);
	}
	else
	{
		m_dma_active &= ~(1 << Channel);
		if (m_tc)
			m_isabus->eop_w(Channel, CLEAR_LINE);
	}
}

void m24_state::dma_tc_w(int state)
{
	m_tc = (state == ASSERT_LINE);
	for (int channel = 0; channel < 4; channel++)
		if (BIT(m_dma_active, channel))
			m_isabus->eop_w(channel, state);
}

void m24_state::dreq0_ck_w(int state)
{
	if (state && !m_dreq0_ck && !BIT(m_dma_active, 0))
		m_dmac->dreq0_w(1);

	m_dreq0_ck = state;
}

void m24_state::speaker_ck_w(int state)
{
	if (state)
		m_ctrlport_b |= 0x20;
	else
		m_ctrlport_b &= 0xdf;

	update_speaker();
}

void m24_state::update_speaker()
{
	if (BIT(m_ctrlport_a, 1) && BIT(m_ctrlport_b, 5))
	{
		m_speaker->level_w(1);
		m_ctrlport_b &= 0xef;
	}
	else
	{
		m_speaker->level_w(0);
		m_ctrlport_b |= 0x10;
	}
}

u8 m24_state::keyboard_data_r()
{
	if (!machine().side_effects_disabled())
	{
		m_pa |= 0x40;
		m_pic->ir1_w(0);
	}
	return m_kbcout;
}

u8 m24_state::keyboard_status_r()
{
	return (m_kbcibf ? 2 : 0) | ((m_pa & 0x40) ? 0 : 1);
}

void m24_state::keyboard_data_w(u8 data)
{
	m_kbc->set_input_line(TMS7000_INT1_LINE, ASSERT_LINE);
	m_kbcibf = true;
	m_kbcin = data;
}

void m24_state::ctrlport_a_w(u8 data)
{
	const bool spkrdata_en_dis = BIT(data ^ m_ctrlport_a, 1);
	const bool iochk_en_dis = BIT(data ^ m_ctrlport_a, 4);

	m_pit->write_gate2(BIT(data, 0));

	if (BIT(m_ctrlport_a, 4) && !m_chck_active)
		m_ctrlport_b &= 0xbf;

	if (BIT(data, 6))
		m_pa |= 4;
	else
		m_pa &= ~4;

	m_ctrlport_a = data;

	if (spkrdata_en_dis)
		update_speaker();
	if (iochk_en_dis)
		update_nmi();
}

u8 m24_state::ctrlport_a_r()
{
	return m_ctrlport_a;
}

u8 m24_state::ctrlport_b_r()
{
	// Bit 0 = NC
	// Bit 1 = SW4 (8087 present)
	// Bit 2 = ~RI1
	// Bit 3 = ~DSR1
	// Bit 4 = SPKR
	// Bit 5 = OUT2 (8253)
	// Bit 6 = IOCHK
	// Bit 7 = MBMERR (MRD parity check)

	if (BIT(m_dsw0->read(), 4))
		m_ctrlport_b |= 0x02;
	else
		m_ctrlport_b &= 0xfd;

	return m_ctrlport_b;
}

void m24_state::alt_w(u8 data)
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, (data & 0x40) ? ASSERT_LINE : CLEAR_LINE);
	m_i86_halt = true;
	m_i86_halt_perm = true;
}

void m24_state::chck_w(int state)
{
	m_chck_active = (state == 0);
	if (m_chck_active)
	{
		if (!BIT(m_ctrlport_b, 6))
		{
			m_ctrlport_b |= 0x40;
			update_nmi();
		}
	}
	else if (BIT(m_ctrlport_a, 4))
		m_ctrlport_b &= 0xbf;
}

void m24_state::int87_w(int state)
{
	m_87int = state;
	update_nmi();
}

void m24_state::nmi_enable_w(u8 data)
{
	m_nmi_enable = BIT(data, 7);
	update_nmi();
}

void m24_state::update_nmi()
{
	if (m_nmi_enable && ((m_87int && BIT(m_dsw0->read(), 4)) || (BIT(m_ctrlport_b, 6) && !BIT(m_ctrlport_a, 4))))
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	else
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

u8 m24_state::pa_r()
{
	return m_pa & (m_kbdata ? 0xff : 0xfd);
}

void m24_state::pb_w(u8 data)
{
	m_keyboard->clock_w(!BIT(data, 0));
	m_keyboard->data_w(!BIT(data, 1));
	m_pa = (m_pa & ~3) | (~data & 3);
}

u8 m24_state::kbcdata_r()
{
	m_kbc->set_input_line(TMS7000_INT1_LINE, CLEAR_LINE);
	m_kbcibf = false;
	return m_kbcin;
}

void m24_state::kbcdata_w(u8 data)
{
	m_pa &= ~0x40;
	m_pic->ir1_w(1);
	m_kbcout = data;
}

void m24_state::kbcin_w(int state)
{
	m_kbdata = state;
}

void m24_state::int_w(int state)
{
	if(!m_i86_halt)
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, state ? ASSERT_LINE : CLEAR_LINE);
	if(m_z8000_apb && !m_z8000_apb->halted())
		m_z8000_apb->int_w(state ? ASSERT_LINE : CLEAR_LINE);
}

void m24_state::halt_i86_w(int state)
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
	map(0x0000, 0x000f).rw(m_dmac, FUNC(am9517a_device::read), FUNC(am9517a_device::write));
	map(0x0020, 0x0021).mirror(0xe).rw(m_pic, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x0040, 0x0043).mirror(0xc).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x0060, 0x0060).rw(FUNC(m24_state::keyboard_data_r), FUNC(m24_state::keyboard_data_w));
	map(0x0061, 0x0061).rw(FUNC(m24_state::ctrlport_a_r), FUNC(m24_state::ctrlport_a_w));
	map(0x0062, 0x0062).r(FUNC(m24_state::ctrlport_b_r));
	map(0x0064, 0x0064).r(FUNC(m24_state::keyboard_status_r));
	map(0x0065, 0x0065).w(FUNC(m24_state::alt_w));
	map(0x0066, 0x0067).portr("DSW0");
	map(0x0070, 0x007f).rw("mm58174an", FUNC(mm58174_device::read), FUNC(mm58174_device::write));
	map(0x0080, 0x0083).mirror(0xc).w(FUNC(m24_state::dma_segment_w));
	map(0x00a0, 0x00a1).mirror(0xe).w(FUNC(m24_state::nmi_enable_w));
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

void m24_state::floppy_formats(format_registration &fr)
{
	fr.add_pc_formats();
	fr.add(FLOPPY_NASLITE_FORMAT);
	fr.add(FLOPPY_M20_FORMAT);
}

void m24_state::cfg_m20_format(device_t *device)
{
	device->subdevice<floppy_connector>("fdc:0")->set_formats(m24_state::floppy_formats);
	device->subdevice<floppy_connector>("fdc:1")->set_formats(m24_state::floppy_formats);
}

void m24_state::olivetti(machine_config &config)
{
	/* basic machine hardware */
	I8086(config, m_maincpu, 24_MHz_XTAL / 3);
	m_maincpu->set_addrmap(AS_PROGRAM, &m24_state::m24_map);
	m_maincpu->set_addrmap(AS_IO, &m24_state::m24_io);
	m_maincpu->set_irq_acknowledge_callback("pic", FUNC(pic8259_device::inta_cb));
	m_maincpu->esc_opcode_handler().set("ndp", FUNC(i8087_device::insn_w));
	m_maincpu->esc_data_handler().set("ndp", FUNC(i8087_device::addr_w));

	i8087_device &i8087(I8087(config, "ndp", 24_MHz_XTAL / 3));
	i8087.set_space_86(m_maincpu, AS_PROGRAM);
	i8087.irq().set(FUNC(m24_state::int87_w));
	i8087.busy().set_inputline(m_maincpu, INPUT_LINE_TEST);

	AM9517A(config, m_dmac, 24_MHz_XTAL / 6); // 8237A-4
	m_dmac->out_hreq_callback().set(FUNC(m24_state::dma_hrq_w));
	m_dmac->in_memr_callback().set(FUNC(m24_state::dma_memory_read));
	m_dmac->out_memw_callback().set(FUNC(m24_state::dma_memory_write));
	m_dmac->in_ior_callback<1>().set(FUNC(m24_state::dma_io_read<1>));
	m_dmac->in_ior_callback<2>().set(FUNC(m24_state::dma_io_read<2>));
	m_dmac->in_ior_callback<3>().set(FUNC(m24_state::dma_io_read<3>));
	m_dmac->out_iow_callback<1>().set(FUNC(m24_state::dma_io_write<1>));
	m_dmac->out_iow_callback<2>().set(FUNC(m24_state::dma_io_write<2>));
	m_dmac->out_iow_callback<3>().set(FUNC(m24_state::dma_io_write<3>));
	m_dmac->out_dack_callback<0>().set(FUNC(m24_state::dma_dack_w<0>));
	m_dmac->out_dack_callback<1>().set(FUNC(m24_state::dma_dack_w<1>));
	m_dmac->out_dack_callback<2>().set(FUNC(m24_state::dma_dack_w<2>));
	m_dmac->out_dack_callback<3>().set(FUNC(m24_state::dma_dack_w<3>));
	m_dmac->out_eop_callback().set(FUNC(m24_state::dma_tc_w));

	PIC8259(config, m_pic);
	m_pic->in_sp_callback().set_constant(1);
	m_pic->out_int_callback().set(FUNC(m24_state::int_w));

	PIT8253(config, m_pit); // 8253-5
	m_pit->set_clk<0>(3.6864_MHz_XTAL / 3); // divided by LS175 at 8T
	m_pit->set_clk<1>(3.6864_MHz_XTAL / 3);
	m_pit->set_clk<2>(3.6864_MHz_XTAL / 3);
	m_pit->out_handler<0>().set(m_pic, FUNC(pic8259_device::ir0_w));
	m_pit->out_handler<1>().set(FUNC(m24_state::dreq0_ck_w));
	m_pit->out_handler<2>().set(FUNC(m24_state::speaker_ck_w));

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 1.00);

	ISA8(config, m_isabus, 24_MHz_XTAL / 6);
	m_isabus->set_memspace(m_maincpu, AS_PROGRAM);
	m_isabus->set_iospace(m_maincpu, AS_IO);
	m_isabus->irq2_callback().set(m_pic, FUNC(pic8259_device::ir2_w));
	m_isabus->irq3_callback().set(m_pic, FUNC(pic8259_device::ir3_w));
	m_isabus->irq4_callback().set(m_pic, FUNC(pic8259_device::ir4_w));
	m_isabus->irq5_callback().set(m_pic, FUNC(pic8259_device::ir5_w));
	m_isabus->irq6_callback().set(m_pic, FUNC(pic8259_device::ir6_w));
	m_isabus->irq7_callback().set(m_pic, FUNC(pic8259_device::ir7_w));
	m_isabus->drq1_callback().set(m_dmac, FUNC(am9517a_device::dreq1_w));
	m_isabus->drq2_callback().set(m_dmac, FUNC(am9517a_device::dreq2_w));
	m_isabus->drq3_callback().set(m_dmac, FUNC(am9517a_device::dreq3_w));
	m_isabus->iochck_callback().set(FUNC(m24_state::chck_w));

	ISA8_SLOT(config, "mb1", 0, m_isabus, pc_isa8_cards, "cga_m24", true);
	ISA8_SLOT(config, "mb2", 0, m_isabus, pc_isa8_cards, "fdc_xt", true).set_option_machine_config("fdc_xt", cfg_m20_format);
	ISA8_SLOT(config, "mb3", 0, m_isabus, pc_isa8_cards, "lpt", true);
	ISA8_SLOT(config, "mb4", 0, m_isabus, pc_isa8_cards, "com", true);

	ISA8_SLOT(config, "isa1", 0, m_isabus, pc_isa8_cards, nullptr, false);
	ISA8_SLOT(config, "isa2", 0, m_isabus, pc_isa8_cards, nullptr, false);
	ISA8_SLOT(config, "isa3", 0, m_isabus, pc_isa8_cards, nullptr, false);

	// 2 banks of 16 64Kx1 or 256Kx1 DRAMs on motherboard
	RAM(config, m_ram).set_default_size("640K").set_extra_options("128K, 256K, 512K");

	TMS7000(config, m_kbc, 24_MHz_XTAL / 6);
	m_kbc->set_addrmap(AS_PROGRAM, &m24_state::kbc_map);
	m_kbc->in_porta().set(FUNC(m24_state::pa_r));
	m_kbc->out_portb().set(FUNC(m24_state::pb_w));

	M24_KEYBOARD(config, m_keyboard, 0);
	m_keyboard->out_data_handler().set(FUNC(m24_state::kbcin_w));

	MM58174(config, "mm58174an", 32.768_kHz_XTAL);

	M24_Z8000(config, m_z8000_apb, 0); // TODO: make this a slot device (uses custom bus connector)
	m_z8000_apb->halt_callback().set(FUNC(m24_state::halt_i86_w));

	/* software lists */
	SOFTWARE_LIST(config, "disk_list").set_original("ibm5150");
	SOFTWARE_LIST(config, "m24_disk_list").set_original("m24");
}

ROM_START( m21 )
	ROM_REGION16_LE(0x8000,"bios", 0)
	ROMX_LOAD( "bios_m24_144_even.bin", 0x4000, 0x2000, CRC(5f3d7084) SHA1(d55c0d8472b45e4c4ca9cb0066cd5c122056ba8e), ROM_SKIP(1))
	ROMX_LOAD( "bios_m24_144_odd.bin", 0x4001, 0x2000, CRC(18fd8db8) SHA1(f2c9d189f7ded88946a99432abd7106d509a7411), ROM_SKIP(1))

	ROM_REGION(0x800, "kbc", 0)
	ROM_LOAD("pdbd.tms2516.kbdmcu_replacement_board.10u", 0x000, 0x800, CRC(b8c4c18a) SHA1(25b4c24e19ff91924c53557c66513ab242d926c6))
ROM_END

ROM_START( m24 )
	ROM_REGION16_LE(0x8000,"bios", 0)
	ROM_SYSTEM_BIOS(0,"v1.1","v1.1")
	ROMX_LOAD("m24_bios11h.rom",0x4001, 0x2000, CRC(f08e859a) SHA1(c2ede7ce4472c77462d1d841e2b47e8b306c563d), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("m24_bios11l.rom", 0x4000, 0x2000, CRC(ec494e66) SHA1(51259cf9fd9f6a6855d52730206ff66ad3367ea4), ROM_SKIP(1) | ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1,"v1.21","v1.21")
	ROMX_LOAD("m24_bios121h.rom",0x4001, 0x2000, CRC(93292715) SHA1(863eccfb3beca6e64c5b0cc070c64394bad7da82), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("m24_bios121l.rom", 0x4000, 0x2000, CRC(1acbc9d7) SHA1(d3696e38853cea31e70ffa4e13e127ec7551bf57), ROM_SKIP(1) | ROM_BIOS(1))

	ROM_SYSTEM_BIOS(2,"v1.36","v1.36")
	ROMX_LOAD("m24_bios136h.rom",0x4001, 0x2000, CRC(25cbf8ba) SHA1(1ab90985852544d2c12b47bb7f20f9faccabdf88), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("m24_bios136l.rom", 0x4000, 0x2000, CRC(e2f738c0) SHA1(da9771325a5021cf9908997e0e0d14e47258125f), ROM_SKIP(1) | ROM_BIOS(2))

	ROM_SYSTEM_BIOS(3,"v1.43","v1.43")
	ROMX_LOAD("olivetti_m24_version_1.43_high.bin",0x4001, 0x2000, CRC(04e697ba) SHA1(1066dcc849e6289b5ac6372c84a590e456d497a6), ROM_SKIP(1) | ROM_BIOS(3))
	ROMX_LOAD("olivetti_m24_version_1.43_low.bin", 0x4000, 0x2000, CRC(ff7e0f10) SHA1(13423011a9bae3f3193e8c199f98a496cab48c0f), ROM_SKIP(1) | ROM_BIOS(3))

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

} // Anonymous namespace


COMP( 1984, m21,  ibm5150, 0, olivetti, m24, m24_state, empty_init, "Olivetti", "M21",  MACHINE_NOT_WORKING )
COMP( 1983, m24,  ibm5150, 0, olivetti, m24, m24_state, empty_init, "Olivetti", "M24",  MACHINE_NOT_WORKING )
COMP( 1987, m240, ibm5150, 0, olivetti, m24, m24_state, empty_init, "Olivetti", "M240", MACHINE_NOT_WORKING )
