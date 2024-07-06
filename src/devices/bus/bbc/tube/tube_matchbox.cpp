// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Matchbox Co-Processor

    Original firmware for the Matchbox co-processor can be found at:
    https://github.com/hoglet67/CoPro6502/tree/master

    Later developments of the client firmwares can be found at:
    https://mdfs.net/Software/Tube/Matchbox/

    TODO:
   - fix PDP11 interrupt issues.

**********************************************************************/

#include "emu.h"
#include "tube_matchbox.h"

#include "cpu/arm/arm.h"
#include "cpu/i86/i286.h"
#include "cpu/m6502/m65c02.h"
#include "cpu/m6809/m6809.h"
#include "cpu/m68000/m68000.h"
#include "cpu/ns32000/ns32000.h"
#include "cpu/t11/t11.h"
#include "cpu/z80/z80.h"
#include "machine/ram.h"
#include "machine/tube.h"

#include "softlist_dev.h"


namespace {

class bbc_tube_matchbox_device : public device_t, public device_bbc_tube_interface
{
public:
	bbc_tube_matchbox_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, BBC_TUBE_MATCHBOX, tag, owner, clock)
		, device_bbc_tube_interface(mconfig, *this)
		, m_ula(*this, "ula")
		, m_ram(*this, "ram")
		, m_soft_dip(0)
		, m_prst(0)
		, m_m65c102(*this, "m65c102")
		, m_m65c102_rom(*this, "m65c102_rom")
		, m_m65c102_view(*this, "m65c102_view")
		, m_m65c102_bank(*this, "m65c102_bank%u", 0)
		, m_z80(*this, "z80")
		, m_z80_rom(*this, "z80_rom")
		, m_z80_view(*this, "z80_view")
		, m_i80286(*this, "i80286")
		, m_i80286_rom(*this, "i80286_rom")
		, m_irq_latch(0)
		, m_m6809(*this, "m6809")
		, m_m6809_rom(*this, "m6809_rom")
		, m_m68000(*this, "m68000")
		, m_m68000_rom(*this, "m68000_rom")
		, m_m68000_view(*this, "m68000_view")
		, m_pdp11(*this, "pdp11")
		, m_pdp11_rom(*this, "pdp11_rom")
		, m_irq_vector(0)
		, m_arm2(*this, "arm2")
		, m_arm2_rom(*this, "arm2_rom")
		, m_ns32016(*this, "ns32016")
		, m_ns32016_rom(*this, "ns32016_rom")
		, m_ns32016_view(*this, "ns32016_view")
	{
	}

	DECLARE_INPUT_CHANGED_MEMBER(dip_changed);

protected:
	// device_t overrides
	virtual void device_start() override;
	virtual void device_reset_after_children() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;

	virtual uint8_t host_r(offs_t offset) override;
	virtual void host_w(offs_t offset, uint8_t data) override;

private:
	required_device<tube_device> m_ula;
	required_device<ram_device> m_ram;

	void pnmi_w(int state);
	void pirq_w(int state);
	void prst_w(int state);

	uint8_t m_copro;
	uint8_t m_soft_dip;
	int m_prst;

	// 65C102
	required_device<m6502_device> m_m65c102;
	required_region_ptr<uint8_t> m_m65c102_rom;
	memory_passthrough_handler m_m65c102_rom_shadow_tap;
	memory_view m_m65c102_view;
	required_memory_bank_array<8> m_m65c102_bank;

	void m65c102_mem(address_map &map);
	void m65c102_reset(uint8_t dip);
	void m65c102_bank_w(offs_t offset, uint8_t data);

	// Z80
	required_device<z80_device> m_z80;
	required_region_ptr<uint8_t> m_z80_rom;
	memory_view m_z80_view;

	void z80_opcodes(address_map &map);
	void z80_io(address_map &map);
	void z80_mem(address_map &map);
	void z80_reset(uint8_t dip);
	uint8_t z80_opcode_r(offs_t offset);

	// 80286
	required_device<i80286_cpu_device> m_i80286;
	required_region_ptr<uint16_t> m_i80286_rom;

	void i80286_io(address_map &map);
	void i80286_mem(address_map &map);

	uint8_t m_irq_latch;

	// 6809
	required_device<cpu_device> m_m6809;
	required_region_ptr<uint8_t> m_m6809_rom;
	memory_passthrough_handler m_m6809_rom_shadow_tap;

	void m6809_mem(address_map &map);
	void m6809_reset();
	uint8_t m6809_mem_r(offs_t offset);
	bool m_m6809_rom_enabled;
	int m_m6809_bs_hack_cnt;

	// 68000
	required_device<m68000_base_device> m_m68000;
	required_region_ptr<uint16_t> m_m68000_rom;
	memory_passthrough_handler m_m68000_rom_shadow_tap;
	memory_view m_m68000_view;

	void m68000_mem(address_map &map);
	void m68000_reset();

	// PDP-11
	required_device<t11_device> m_pdp11;
	required_region_ptr<uint16_t> m_pdp11_rom;

	void pdp11_mem(address_map &map);
	void pdp11_reset();
	//uint8_t irq_callback(offs_t offset);
	void pdp11_irq_encoder(int irq, int state);
	uint16_t m_irqs;
	uint16_t m_irq_vector;

	// ARM2
	required_device<arm_cpu_device> m_arm2;
	required_region_ptr<uint32_t> m_arm2_rom;
	memory_passthrough_handler m_arm2_rom_shadow_tap;

	void arm2_mem(address_map &map);
	void arm2_reset();

	// 32016
	required_device<ns32016_device> m_ns32016;
	required_region_ptr<uint16_t> m_ns32016_rom;
	memory_passthrough_handler m_ns32016_rom_shadow_tap;
	memory_view m_ns32016_view;

	void ns32016_mem(address_map &map);
	void ns32016_reset();
};


//-------------------------------------------------
//  ADDRESS_MAP( m65c102_mem )
//-------------------------------------------------

void bbc_tube_matchbox_device::m65c102_mem(address_map &map)
{
	map(0x0000, 0xffff).view(m_m65c102_view);
	m_m65c102_view[0](0x0000, 0xffff).rw(m_ram, FUNC(ram_device::read), FUNC(ram_device::write));
	m_m65c102_view[0](0xf800, 0xffff).rom().region("m65c102_rom", 0);
	m_m65c102_view[1](0x0000, 0x1fff).bankrw("m65c102_bank0");
	m_m65c102_view[1](0x2000, 0x3fff).bankrw("m65c102_bank1");
	m_m65c102_view[1](0x4000, 0x5fff).bankrw("m65c102_bank2");
	m_m65c102_view[1](0x6000, 0x7fff).bankrw("m65c102_bank3");
	m_m65c102_view[1](0x8000, 0x9fff).bankrw("m65c102_bank4");
	m_m65c102_view[1](0xa000, 0xbfff).bankrw("m65c102_bank5");
	m_m65c102_view[1](0xc000, 0xdfff).bankrw("m65c102_bank6");
	m_m65c102_view[1](0xe000, 0xffff).bankrw("m65c102_bank7");
	m_m65c102_view[1](0xfee0, 0xfee7).w(FUNC(bbc_tube_matchbox_device::m65c102_bank_w));
	map(0xfef8, 0xfeff).rw("ula", FUNC(tube_device::parasite_r), FUNC(tube_device::parasite_w));
}


//-------------------------------------------------
//  ADDRESS_MAP( z80_mem )
//-------------------------------------------------

void bbc_tube_matchbox_device::z80_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(m_ram, FUNC(ram_device::read), FUNC(ram_device::write));
	map(0x0000, 0x0fff).view(m_z80_view);
	m_z80_view[0](0x0000, 0x0fff).rom().region("z80_rom", 0);
}

void bbc_tube_matchbox_device::z80_opcodes(address_map &map)
{
	map(0x0000, 0xffff).r(FUNC(bbc_tube_matchbox_device::z80_opcode_r));
}

void bbc_tube_matchbox_device::z80_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x07).rw("ula", FUNC(tube_device::parasite_r), FUNC(tube_device::parasite_w));
}


//-------------------------------------------------
//  ADDRESS_MAP( i286_mem )
//-------------------------------------------------

void bbc_tube_matchbox_device::i80286_mem(address_map &map)
{
	map.global_mask(0xfffff);
	map(0x00000, 0xfffff).rw(m_ram, FUNC(ram_device::read), FUNC(ram_device::write));
	map(0xf0000, 0xf3fff).rom().region("i80286_rom", 0).mirror(0xc000);
}

void bbc_tube_matchbox_device::i80286_io(address_map &map)
{
	map.unmap_value_high();
	map(0x60, 0x60).lw8(NAME([this](uint8_t data) { m_irq_latch = data; }));
	map(0x80, 0x8f).rw("ula", FUNC(tube_device::parasite_r), FUNC(tube_device::parasite_w)).umask16(0x00ff);
}


//-------------------------------------------------
//  ADDRESS_MAP( m6809_mem )
//-------------------------------------------------

void bbc_tube_matchbox_device::m6809_mem(address_map &map)
{
	map(0x0000, 0xffff).r(FUNC(bbc_tube_matchbox_device::m6809_mem_r)).w(m_ram, FUNC(ram_device::write));
	map(0xfee0, 0xfeef).rw("ula", FUNC(tube_device::parasite_r), FUNC(tube_device::parasite_w));
}


//-------------------------------------------------
//  ADDRESS_MAP( m68000_mem )
//-------------------------------------------------

void bbc_tube_matchbox_device::m68000_mem(address_map &map)
{
	map.global_mask(0x3fffff);
	map(0x000000, 0x3fffff).view(m_m68000_view);
	m_m68000_view[0](0x000000, 0x007fff).mirror(0x3f8000).rom().region("m68000_rom", 0);
	m_m68000_view[1](0x000000, 0x0fffff).mirror(0x300000).rw(m_ram, FUNC(ram_device::read), FUNC(ram_device::write));
	map(0x3e0000, 0x3e000f).mirror(0x00fff0).rw("ula", FUNC(tube_device::parasite_r), FUNC(tube_device::parasite_w));
	map(0x3f0000, 0x3f7fff).mirror(0x008000).rom().region("m68000_rom", 0);
}


//-------------------------------------------------
//  ADDRESS_MAP( pdp11_mem )
//-------------------------------------------------

void bbc_tube_matchbox_device::pdp11_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(m_ram, FUNC(ram_device::read), FUNC(ram_device::write));
	map(0xf800, 0xffff).rom().region("pdp11_rom", 0);
	map(0xfff0, 0xffff).rw("ula", FUNC(tube_device::parasite_r), FUNC(tube_device::parasite_w)).umask16(0x00ff);
}


//-------------------------------------------------
//  ADDRESS_MAP( arm2_mem )
//-------------------------------------------------

void bbc_tube_matchbox_device::arm2_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000000, 0x0ffffff).noprw();
	map(0x1000000, 0x100001f).rw(m_ula, FUNC(tube_device::parasite_r), FUNC(tube_device::parasite_w)).umask32(0x000000ff);
	map(0x3000000, 0x3003fff).rom().region("arm2_rom", 0).mirror(0xc000);
}


//-------------------------------------------------
//  ADDRESS_MAP( ns32016_mem )
//-------------------------------------------------

void bbc_tube_matchbox_device::ns32016_mem(address_map &map)
{
	map(0x000000, 0xffffff).view(m_ns32016_view);
	m_ns32016_view[0](0x000000, 0x007fff).mirror(0xff8000).rom().region("ns32016_rom", 0);
	m_ns32016_view[1](0x000000, 0x1fffff).mirror(0x600000).rw(m_ram, FUNC(ram_device::read), FUNC(ram_device::write));
	m_ns32016_view[1](0x800000, 0xffffff).noprw();
	map(0xf00000, 0xf07fff).mirror(0x038000).rom().region("ns32016_rom", 0);
	map(0xfffff0, 0xffffff).rw("ula", FUNC(tube_device::parasite_r), FUNC(tube_device::parasite_w)).umask16(0x00ff);
}


//-------------------------------------------------
//  INPUT_PORTS( matchbox )
//-------------------------------------------------

static INPUT_PORTS_START(matchbox)
	PORT_START("DIPSW")
	PORT_DIPNAME(0x0f, 0x00, "Co-Processor") PORT_CHANGED_MEMBER(DEVICE_SELF, bbc_tube_matchbox_device, dip_changed, 0)
	PORT_DIPSETTING(0x00, "65C102 3 MHz")
	PORT_DIPSETTING(0x01, "65C102 4 MHz")
	PORT_DIPSETTING(0x02, "65C102 16 MHz")
	PORT_DIPSETTING(0x03, "65C102 64 MHz")
	PORT_DIPSETTING(0x04, "Z80 8 MHz")
	PORT_DIPSETTING(0x05, "Z80 32 MHz")
	PORT_DIPSETTING(0x06, "Z80 56 MHz")
	PORT_DIPSETTING(0x07, "Z80 112 MHz")
	PORT_DIPSETTING(0x08, "80286 12 MHz")
	PORT_DIPSETTING(0x09, "6809 4 MHz")
	PORT_DIPSETTING(0x0a, "68000 16 MHz")
	PORT_DIPSETTING(0x0b, "PDP-11 32 MHz")
	PORT_DIPSETTING(0x0c, "ARM2 32 MHz")
	PORT_DIPSETTING(0x0d, "32016 32 MHz")
	PORT_DIPSETTING(0x0e, "Null / SPI")
	PORT_DIPSETTING(0x0f, "BIST")
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(bbc_tube_matchbox_device::dip_changed)
{
	m_soft_dip = newval;
}

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor bbc_tube_matchbox_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( matchbox );
}


//-------------------------------------------------
//  ROM( matchbox )
//-------------------------------------------------

ROM_START( matchbox )
	ROM_REGION(0x0800, "m65c102_rom", 0)
	ROM_LOAD("client65v2.bin", 0x0000, 0x0800, CRC(866a5b7b) SHA1(40e2de0443e3447483fe6ee43fe66bac87fed1c4)) // latest from https://mdfs.net/Software/Tube/Matchbox/

	ROM_REGION(0x1000, "z80_rom", 0)
	ROM_LOAD("tuberom_z80.rom", 0x0000, 0x1000, CRC(229a764c) SHA1(9702f9a821e5beddd04f7f71c4a07ab660fcc962)) // from LX9CoProCombined_20171007_0719_dmb firmware

	ROM_REGION16_LE(0x4000, "i80286_rom", 0)
	ROM_LOAD("client86_101.bin", 0x0000, 0x4000, CRC(6da326ce) SHA1(8acaf42a4a6b5f62f6be5e62df5fcffbf48890dd)) // latest from https://mdfs.net/Software/Tube/Matchbox/

	ROM_REGION(0x0800, "m6809_rom", 0)
	ROM_LOAD("client09_106.bin", 0x0000, 0x0800, CRC(5f499c5b) SHA1(5c615d16f579b4b6fb42f8ff1188b1f3404425fa)) // latest from https://mdfs.net/Software/Tube/Matchbox/

	ROM_REGION16_BE(0x8000, "m68000_rom", 0)
	ROM_LOAD("tuberom_68000.bin", 0x0000, 0x8000, CRC(906d6733) SHA1(c376a6741e40d5b37a2e1fa7c2d766b9a575c57c)) // from LX9CoProCombined_20171007_0719_dmb firmware

	ROM_REGION16_LE(0x0800, "pdp11_rom", 0)
	ROM_LOAD("client11_040.bin",  0x0000, 0x0800, CRC(ab409e78) SHA1(3756c0b78f94c6a79d077c25c5c74c55cfacf65c)) // latest from https://mdfs.net/Software/Tube/Matchbox/

	ROM_REGION32_LE(0x4000, "arm2_rom", 0)
	ROM_LOAD("armeval_101.rom", 0x0000, 0x4000, CRC(cabd6a1b) SHA1(9f451f53a5cb6f649fa8a2946e22f1ee49199a93))

	ROM_REGION16_LE(0x8000, "ns32016_rom", 0)
	ROM_LOAD("tuberom_32016.bin", 0x0000, 0x8000, CRC(11dfca39) SHA1(80343cb198b03ea91c8790d765f7d3869ba9bb32)) // from LX9CoProCombined_20171007_0719_dmb firmware
ROM_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *bbc_tube_matchbox_device::device_rom_region() const
{
	return ROM_NAME( matchbox );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_tube_matchbox_device::device_add_mconfig(machine_config &config)
{
	TUBE(config, m_ula);
	m_ula->pnmi_handler().set(FUNC(bbc_tube_matchbox_device::pnmi_w));
	m_ula->pirq_handler().set(FUNC(bbc_tube_matchbox_device::pirq_w));
	m_ula->prst_handler().set(FUNC(bbc_tube_matchbox_device::prst_w));

	RAM(config, m_ram).set_default_size("2MB");

	// 65C102
	M65C02(config, m_m65c102, 32_MHz_XTAL);
	m_m65c102->set_addrmap(AS_PROGRAM, &bbc_tube_matchbox_device::m65c102_mem);

	SOFTWARE_LIST(config, "flop_ls_6502").set_original("bbc_flop_6502");

	// Z80
	Z80(config, m_z80, 32_MHz_XTAL);
	m_z80->set_addrmap(AS_PROGRAM, &bbc_tube_matchbox_device::z80_mem);
	m_z80->set_addrmap(AS_OPCODES, &bbc_tube_matchbox_device::z80_opcodes);
	m_z80->set_addrmap(AS_IO, &bbc_tube_matchbox_device::z80_io);
	m_z80->set_irq_acknowledge_callback(NAME([](device_t &, int) -> int { return 0xfe; }));

	SOFTWARE_LIST(config, "flop_ls_z80").set_original("bbc_flop_z80");

	// 80286
	I80286(config, m_i80286, 32_MHz_XTAL * 6 / 16);
	m_i80286->set_addrmap(AS_PROGRAM, &bbc_tube_matchbox_device::i80286_mem);
	m_i80286->set_addrmap(AS_IO, &bbc_tube_matchbox_device::i80286_io);
	m_i80286->set_irq_acknowledge_callback(NAME([this](device_t &, int) -> int { return m_irq_latch; }));

	SOFTWARE_LIST(config, "flop_ls_80186").set_original("bbc_flop_80186");
	SOFTWARE_LIST(config, "pc_disk_list").set_compatible("ibm5150");

	// 6809
	MC6809(config, m_m6809, 32_MHz_XTAL / 2);
	m_m6809->set_addrmap(AS_PROGRAM, &bbc_tube_matchbox_device::m6809_mem);
	m_m6809->set_irq_acknowledge_callback(NAME([this](device_t&, int) -> int { m_m6809_bs_hack_cnt = 3; return 0; }));

	// 68000
	M68000(config, m_m68000, 32_MHz_XTAL / 4);
	m_m68000->set_addrmap(AS_PROGRAM, &bbc_tube_matchbox_device::m68000_mem);

	SOFTWARE_LIST(config, "flop_ls_68000").set_original("bbc_flop_68000");

	// PDP-11
	T11(config, m_pdp11, 32_MHz_XTAL);
	m_pdp11->set_initial_mode(6 << 13);
	m_pdp11->set_addrmap(AS_PROGRAM, &bbc_tube_matchbox_device::pdp11_mem);
	m_pdp11->in_iack().set([this](uint8_t addr) { return m_irq_vector; });

	// ARM2
	ARM(config, m_arm2, 32_MHz_XTAL);
	m_arm2->set_addrmap(AS_PROGRAM, &bbc_tube_matchbox_device::arm2_mem);

	SOFTWARE_LIST(config, "flop_ls_arm").set_original("bbc_flop_arm");

	// 32016
	NS32016(config, m_ns32016, 32_MHz_XTAL);
	m_ns32016->set_addrmap(AS_PROGRAM, &bbc_tube_matchbox_device::ns32016_mem);

	SOFTWARE_LIST(config, "flop_ls_32016").set_original("bbc_flop_32016");
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_tube_matchbox_device::device_start()
{
	m_prst = 0;
}


//-------------------------------------------------
//  device_reset_after_children - reset after child devices
//-------------------------------------------------

void bbc_tube_matchbox_device::device_reset_after_children()
{
	m_m65c102->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_z80->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_i80286->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_m6809->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_m68000->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_pdp11->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_arm2->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_ns32016->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	m_copro = m_soft_dip;

	switch (m_copro)
	{
	case 0x00: case 0x01: case 0x02: case 0x03:
		m65c102_reset(m_copro);
		m_m65c102->set_input_line(INPUT_LINE_RESET, m_prst);
		break;

	case 0x04: case 0x05: case 0x06: case 0x07:
		z80_reset(m_copro);
		m_z80->set_input_line(INPUT_LINE_RESET, m_prst);
		break;

	case 0x08:
		m_i80286->set_input_line(INPUT_LINE_RESET, m_prst);
		break;

	case 0x09:
		m6809_reset();
		m_m6809->set_input_line(INPUT_LINE_RESET, m_prst);
		break;

	case 0x0a:
		m68000_reset();
		m_m68000->set_input_line(INPUT_LINE_RESET, m_prst);
		break;

	case 0x0b:
		pdp11_reset();
		m_pdp11->set_input_line(INPUT_LINE_RESET, m_prst);
		break;

	case 0x0c:
		arm2_reset();
		m_arm2->set_input_line(INPUT_LINE_RESET, m_prst);
		break;

	case 0x0d:
		ns32016_reset();
		m_ns32016->set_input_line(INPUT_LINE_RESET, m_prst);
		break;

	default:
		break;
	}
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bbc_tube_matchbox_device::m65c102_reset(uint8_t copro)
{
	address_space &program = m_m65c102->space(AS_PROGRAM);

	// address map during booting
	m_m65c102_view.select(0);

	m_m65c102_rom_shadow_tap.remove();
	m_m65c102_rom_shadow_tap = program.install_read_tap(
			0x0fef8, 0xfeff,
			"rom_shadow_r",
			[this](offs_t offset, u8 &data, u8 mem_mask)
			{
				if (!machine().side_effects_disabled())
				{
					// delete this tap
					m_m65c102_rom_shadow_tap.remove();

					// address map after booting
					m_m65c102_view.select(1);
				}

				// return the original data
				return data;
			},
			&m_m65c102_rom_shadow_tap);

	for (int i = 0; i < 8; i++)
	{
		m_m65c102_bank[i]->set_base(m_ram->pointer() + (i * 0x2000));
	}

	switch (copro & 3)
	{
	case 0: // 3MHz
		m_m65c102->set_clock_scale(1.0 / 11);

		// original startup banner
		m_m65c102_rom[0x06f] = '6';
		m_m65c102_rom[0x070] = '5';
		m_m65c102_rom[0x071] = 'C';
		m_m65c102_rom[0x072] = '1';
		m_m65c102_rom[0x073] = '0';
		m_m65c102_rom[0x074] = '2';
		m_m65c102_rom[0x075] = ' ';
		m_m65c102_rom[0x076] = 'C';
		m_m65c102_rom[0x077] = 'o';
		m_m65c102_rom[0x078] = '-';
		m_m65c102_rom[0x079] = 'P';
		m_m65c102_rom[0x07a] = 'r';
		m_m65c102_rom[0x07b] = 'o';
		m_m65c102_rom[0x07c] = 'c';
		m_m65c102_rom[0x07d] = 'e';
		m_m65c102_rom[0x07e] = 's';
		m_m65c102_rom[0x07f] = 's';
		m_m65c102_rom[0x080] = 'o';
		m_m65c102_rom[0x081] = 'r';
		break;
	case 1: // 4MHz
		m_m65c102->set_clock_scale(1.0 / 8);

		// patch startup banner
		m_m65c102_rom[0x06f] = '0';
		m_m65c102_rom[0x070] = '4';
		m_m65c102_rom[0x071] = 'M';
		m_m65c102_rom[0x072] = 'H';
		m_m65c102_rom[0x073] = 'z';
		m_m65c102_rom[0x074] = ' ';
		m_m65c102_rom[0x075] = '6';
		m_m65c102_rom[0x076] = '5';
		m_m65c102_rom[0x077] = 'C';
		m_m65c102_rom[0x078] = '1';
		m_m65c102_rom[0x079] = '0';
		m_m65c102_rom[0x07a] = '2';
		m_m65c102_rom[0x07b] = ' ';
		m_m65c102_rom[0x07c] = 'C';
		m_m65c102_rom[0x07d] = 'o';
		m_m65c102_rom[0x07e] = '-';
		m_m65c102_rom[0x07f] = 'P';
		m_m65c102_rom[0x080] = 'r';
		m_m65c102_rom[0x081] = 'o';
		break;
	case 2: // 16MHz
		m_m65c102->set_clock_scale(1.0 / 2);

		// patch startup banner
		m_m65c102_rom[0x06f] = '1';
		m_m65c102_rom[0x070] = '6';
		m_m65c102_rom[0x071] = 'M';
		m_m65c102_rom[0x072] = 'H';
		m_m65c102_rom[0x073] = 'z';
		m_m65c102_rom[0x074] = ' ';
		m_m65c102_rom[0x075] = '6';
		m_m65c102_rom[0x076] = '5';
		m_m65c102_rom[0x077] = 'C';
		m_m65c102_rom[0x078] = '1';
		m_m65c102_rom[0x079] = '0';
		m_m65c102_rom[0x07a] = '2';
		m_m65c102_rom[0x07b] = ' ';
		m_m65c102_rom[0x07c] = 'C';
		m_m65c102_rom[0x07d] = 'o';
		m_m65c102_rom[0x07e] = '-';
		m_m65c102_rom[0x07f] = 'P';
		m_m65c102_rom[0x080] = 'r';
		m_m65c102_rom[0x081] = 'o';
		break;
	case 3: // 64MHz
		m_m65c102->set_clock_scale(1.0 * 2);

		// patch startup banner
		m_m65c102_rom[0x06f] = '6';
		m_m65c102_rom[0x070] = '4';
		m_m65c102_rom[0x071] = 'M';
		m_m65c102_rom[0x072] = 'H';
		m_m65c102_rom[0x073] = 'z';
		m_m65c102_rom[0x074] = ' ';
		m_m65c102_rom[0x075] = '6';
		m_m65c102_rom[0x076] = '5';
		m_m65c102_rom[0x077] = 'C';
		m_m65c102_rom[0x078] = '1';
		m_m65c102_rom[0x079] = '0';
		m_m65c102_rom[0x07a] = '2';
		m_m65c102_rom[0x07b] = ' ';
		m_m65c102_rom[0x07c] = 'C';
		m_m65c102_rom[0x07d] = 'o';
		m_m65c102_rom[0x07e] = '-';
		m_m65c102_rom[0x07f] = 'P';
		m_m65c102_rom[0x080] = 'r';
		m_m65c102_rom[0x081] = 'o';
		break;
	}
}


void bbc_tube_matchbox_device::z80_reset(uint8_t copro)
{
	// address map during booting
	m_z80_view.select(0);

	switch (copro & 3)
	{
	case 0: // 8MHz
		m_z80->set_clock_scale(1.0 / 4);

		// original startup banner
		m_z80_rom[0x2a2] = '6';
		m_z80_rom[0x2a3] = '4';
		m_z80_rom[0x2a4] = 'K';
		m_z80_rom[0x2a5] = ' ';
		m_z80_rom[0x2a6] = '1';
		m_z80_rom[0x2a7] = '.';
		m_z80_rom[0x2a8] = '2';
		m_z80_rom[0x2a9] = '1';
		break;
	case 1: // 32MHz
		m_z80->set_clock_scale(1.0);

		// patch startup banner
		m_z80_rom[0x2a2] = 0x06;
		m_z80_rom[0x2a3] = '3';
		m_z80_rom[0x2a4] = '2';
		m_z80_rom[0x2a5] = ' ';
		m_z80_rom[0x2a6] = 'M';
		m_z80_rom[0x2a7] = 'H';
		m_z80_rom[0x2a8] = 'z';
		m_z80_rom[0x2a9] = ' ';
		break;
	case 2: // 56MHz
		m_z80->set_clock_scale(1.0 * (7.0 / 4));

		// patch startup banner
		m_z80_rom[0x2a2] = 0x06;
		m_z80_rom[0x2a3] = '5';
		m_z80_rom[0x2a4] = '6';
		m_z80_rom[0x2a5] = ' ';
		m_z80_rom[0x2a6] = 'M';
		m_z80_rom[0x2a7] = 'H';
		m_z80_rom[0x2a8] = 'z';
		m_z80_rom[0x2a9] = ' ';
		break;
	case 3: // 112MHz
		m_z80->set_clock_scale(1.0 * (7.0 / 2));

		// patch startup banner
		m_z80_rom[0x2a2] = '1';
		m_z80_rom[0x2a3] = '1';
		m_z80_rom[0x2a4] = '2';
		m_z80_rom[0x2a5] = ' ';
		m_z80_rom[0x2a6] = 'M';
		m_z80_rom[0x2a7] = 'H';
		m_z80_rom[0x2a8] = 'z';
		m_z80_rom[0x2a9] = ' ';
		break;
	}
}


void bbc_tube_matchbox_device::m6809_reset()
{
	m_m6809_bs_hack_cnt = 0;

	address_space &program = m_m6809->space(AS_PROGRAM);

	// enable rom
	m_m6809_rom_enabled = true;

	m_m6809_rom_shadow_tap.remove();
	m_m6809_rom_shadow_tap = program.install_read_tap(
			0x0fee0, 0xfeef,
			"rom_shadow_r",
			[this](offs_t offset, u8 &data, u8 mem_mask)
			{
				if (!machine().side_effects_disabled())
				{
					// delete this tap
					m_m6809_rom_shadow_tap.remove();

					// disable rom
					m_m6809_rom_enabled = false;
				}

				// return the original data
				return data;
			},
			&m_m6809_rom_shadow_tap);
}


void bbc_tube_matchbox_device::m68000_reset()
{
	address_space &program = m_m68000->space(AS_PROGRAM);

	// address map during booting
	m_m68000_view.select(0);

	m_m68000_rom_shadow_tap.remove();
	m_m68000_rom_shadow_tap = program.install_read_tap(
			0x3f0000, 0x3fffff,
			"rom_shadow_r",
			[this](offs_t offset, u16 &data, u16 mem_mask)
			{
				if (!machine().side_effects_disabled())
				{
					// delete this tap
					m_m68000_rom_shadow_tap.remove();

					// address map after booting
					m_m68000_view.select(1);
				}

				// return the original data
				return data;
			},
			&m_m68000_rom_shadow_tap);
}


void bbc_tube_matchbox_device::pdp11_reset()
{
	m_irqs = 0;

	m_pdp11->set_input_line(t11_device::VEC_LINE, ASSERT_LINE);
	m_pdp11->set_state_int(T11_PC, 0xf800);
}


void bbc_tube_matchbox_device::arm2_reset()
{
	address_space &program = m_arm2->space(AS_PROGRAM);

	// enable the reset vector to be fetched from ROM
	m_arm2->space(AS_PROGRAM).install_rom(0x000000, 0x003fff, 0x3fc000, m_arm2_rom);

	m_arm2_rom_shadow_tap.remove();
	m_arm2_rom_shadow_tap = program.install_write_tap(
			0x0000000, 0x01fffff,
			"rom_shadow_w",
			[this] (offs_t offset, u32 &data, u32 mem_mask)
			{
				// delete this tap
				m_arm2_rom_shadow_tap.remove();

				// install ram
				m_arm2->space(AS_PROGRAM).install_ram(0x0000000, 0x01fffff, m_ram->pointer());
			},
			&m_arm2_rom_shadow_tap);
}


void bbc_tube_matchbox_device::ns32016_reset()
{
	address_space &program = m_ns32016->space(AS_PROGRAM);

	// address map during booting
	m_ns32016_view.select(0);

	m_ns32016_rom_shadow_tap.remove();
	m_ns32016_rom_shadow_tap = program.install_write_tap(
			0x000000, 0xffffff,
			"rom_shadow_w",
			[this] (offs_t offset, u16 &data, u16 mem_mask)
			{
				// delete this tap
				m_ns32016_rom_shadow_tap.remove();

				// address map after booting
				m_ns32016_view.select(1);
			},
			&m_ns32016_rom_shadow_tap);
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t bbc_tube_matchbox_device::host_r(offs_t offset)
{
	if (m_copro < 14)
		return m_ula->host_r(offset);
	else
		return 0xfe;
}

void bbc_tube_matchbox_device::host_w(offs_t offset, uint8_t data)
{
	if (m_copro < 14)
		m_ula->host_w(offset, data);

	if (offset == 0x06)
		m_soft_dip = data;
}

void bbc_tube_matchbox_device::pnmi_w(int state)
{
	switch (m_copro)
	{
	case 0x00: case 0x01: case 0x02: case 0x03:
		m_m65c102->set_input_line(M65C02_NMI_LINE, state);
		break;
	case 0x04: case 0x05: case 0x06: case 0x07:
		m_z80->set_input_line(INPUT_LINE_NMI, state);
		break;
	case 0x08:
		m_i80286->set_input_line(INPUT_LINE_NMI, state);
		break;
	case 0x09:
		m_m6809->set_input_line(M6809_IRQ_LINE, state);
		break;
	case 0x0a:
		m_m68000->set_input_line(M68K_IRQ_5, state);
		break;
	case 0x0b:
		m_irq_vector = 0x80;
		pdp11_irq_encoder(7, state);
		break;
	case 0x0c:
		m_arm2->set_input_line(ARM_FIRQ_LINE, state);
		break;
	case 0x0d:
		m_ns32016->set_input_line(INPUT_LINE_NMI, state);
		break;
	}
}

void bbc_tube_matchbox_device::pirq_w(int state)
{
	switch (m_copro)
	{
	case 0x00: case 0x01: case 0x02: case 0x03:
		m_m65c102->set_input_line(M65C02_IRQ_LINE, state);
		break;
	case 0x04: case 0x05: case 0x06: case 0x07:
		m_z80->set_input_line(INPUT_LINE_IRQ0, state);
		break;
	case 0x08:
		m_i80286->set_input_line(INPUT_LINE_INT0, state);
		break;
	case 0x09:
		m_m6809->set_input_line(M6809_FIRQ_LINE, state);
		break;
	case 0x0a:
		m_m68000->set_input_line(M68K_IRQ_2, state);
		break;
	case 0x0b:
		m_irq_vector = 0x84;
		pdp11_irq_encoder(6, state);
		break;
	case 0x0c:
		m_arm2->set_input_line(ARM_IRQ_LINE, state);
		break;
	case 0x0d:
		m_ns32016->set_input_line(INPUT_LINE_IRQ0, state);
		break;
	}
}

void bbc_tube_matchbox_device::prst_w(int state)
{
	m_prst = state;

	device_reset_after_children();
}


//-------------------------------------------------
//  65C102
//-------------------------------------------------

void bbc_tube_matchbox_device::m65c102_bank_w(offs_t offset, uint8_t data)
{
	if (BIT(data, 7))
		m_m65c102_bank[offset & 7]->set_base(m_ram->pointer() + (data & 0x7f) * 0x2000 + 0x100000); // external RAM
	else
		m_m65c102_bank[offset & 7]->set_base(m_ram->pointer() + (data & 0x07) * 0x2000); // internal RAM
}


//-------------------------------------------------
//  Z80
//-------------------------------------------------

uint8_t bbc_tube_matchbox_device::z80_opcode_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		if (offset == 0x0066 && m_z80->input_state(INPUT_LINE_NMI))
			m_z80_view.select(0);
		else if (offset & 0x8000)
			m_z80_view.disable();
	}
	return m_z80->space(AS_PROGRAM).read_byte(offset);
}


//-------------------------------------------------
//  6809
//-------------------------------------------------

uint8_t bbc_tube_matchbox_device::m6809_mem_r(offs_t offset)
{
	if (m_m6809_bs_hack_cnt > 0)
	{
		// BA/BS is decoded so that when vectors are fetched A8 is toggled so
		// vectors are fetched from &FEFx instead of &FFFx
		offset ^= 0x100;
		m_m6809_bs_hack_cnt--;
	}

	if (m_m6809_rom_enabled && (offset >= 0xf800))
		return m_m6809_rom[offset & 0x7ff];
	else
		return m_ram->pointer()[offset];
}


//-------------------------------------------------
//  PDP-11
//-------------------------------------------------

void bbc_tube_matchbox_device::pdp11_irq_encoder(int irq, int state)
{
	if (state == ASSERT_LINE)
		m_irqs |= (1 << irq);
	else
		m_irqs &= ~(1 << irq);

	int i;
	for (i = 15; i > 0; i--)
	{
		if (m_irqs & (1 << i))
			break;
	}
	m_pdp11->set_input_line(t11_device::CP3_LINE, (i & 8) ? ASSERT_LINE : CLEAR_LINE);
	m_pdp11->set_input_line(t11_device::CP2_LINE, (i & 4) ? ASSERT_LINE : CLEAR_LINE);
	m_pdp11->set_input_line(t11_device::CP1_LINE, (i & 2) ? ASSERT_LINE : CLEAR_LINE);
	m_pdp11->set_input_line(t11_device::CP0_LINE, (i & 1) ? ASSERT_LINE : CLEAR_LINE);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(BBC_TUBE_MATCHBOX, device_bbc_tube_interface, bbc_tube_matchbox_device, "bbc_tube_matchbox", "Matchbox Co-Processor")
