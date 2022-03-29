// license: BSD-3-Clause
// copyright-holders: Dirk Best
/****************************************************************************

    Digilog 400

    Protocol analyzer

    Skeleton driver

****************************************************************************/

#include "emu.h"
#include "cpu/i86/i186.h"
#include "machine/digilog_kbd.h"
#include "emupal.h"


namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class digilog400_state : public driver_device
{
public:
	digilog400_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "subcpu")
	{ }

	void digilog400(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<i80186_cpu_device> m_maincpu;
	required_device<i80186_cpu_device> m_subcpu;

	void main_mem_map(address_map &map);
	void main_io_map(address_map &map);
	void sub_mem_map(address_map &map);
	void sub_io_map(address_map &map);
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void digilog400_state::main_mem_map(address_map &map)
{
	map(0xd0000, 0xfffff).rom().region("maincpu", 0);
}

void digilog400_state::main_io_map(address_map &map)
{
}

void digilog400_state::sub_mem_map(address_map &map)
{
	map(0xe0000, 0xfffff).rom().region("subcpu", 0);
}

void digilog400_state::sub_io_map(address_map &map)
{
}


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

static const gfx_layout char_layout =
{
	8, 12,
	RGN_FRAC(1, 1),
	1,
	{ 0 },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8 },
	8 * 16
};

static GFXDECODE_START(chars)
	GFXDECODE_ENTRY("chargen", 0, char_layout, 0, 1)
GFXDECODE_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void digilog400_state::machine_start()
{
}

void digilog400_state::machine_reset()
{
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void digilog400_state::digilog400(machine_config &config)
{
	I80186(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &digilog400_state::main_mem_map);
	m_maincpu->set_addrmap(AS_IO, &digilog400_state::main_io_map);

	I80186(config, m_subcpu, 16_MHz_XTAL);
	m_subcpu->set_addrmap(AS_PROGRAM, &digilog400_state::sub_mem_map);
	m_subcpu->set_addrmap(AS_IO, &digilog400_state::sub_io_map);

	PALETTE(config, "palette", palette_device::MONOCHROME_HIGHLIGHT);

	GFXDECODE(config, "gfxdecode", "palette", chars);

	DIGILOG_KBD(config, "kbd");
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( digilog400 )
	ROM_REGION(0x30000, "maincpu", 0)
	ROM_LOAD16_BYTE("24-1155-02_j.bin", 0x00000, 0x08000, CRC(2c59f3f3) SHA1(7f2b611645ffd1d560e2fbdf76f8c5e3d07ea381))
	ROM_LOAD16_BYTE("24-1155-01_j.bin", 0x00001, 0x08000, CRC(f4e92608) SHA1(c10b72b287a9f25c3075a9b17648565a1ed3ffb1))
	ROM_LOAD16_BYTE("24-1155-04_j.bin", 0x10000, 0x08000, CRC(b5fb0ad6) SHA1(fa11efc45d7bdf8364ae7457519785b6638da222))
	ROM_LOAD16_BYTE("24-1155-03_j.bin", 0x10001, 0x08000, CRC(bef7d565) SHA1(a34e4798a7111ffea5aa1c608d192884cae6a80b))
	ROM_LOAD16_BYTE("24-1155-06_j.bin", 0x20000, 0x08000, CRC(a38d1a9b) SHA1(b5bf733757fe54d4dd5e876f46fdd674e57f9525))
	ROM_LOAD16_BYTE("24-1155-05_j.bin", 0x20001, 0x08000, CRC(e7238778) SHA1(a56613589886f713904e4e610292971f55afa67a))

	ROM_REGION(0x20000, "subcpu", 0)
	ROM_LOAD16_BYTE("24-1154-01_j.bin", 0x00000, 0x08000, CRC(a2424fb1) SHA1(1b7c76402f8644cd051899e49a96755429692626))
	ROM_LOAD16_BYTE("24-1154-03_j.bin", 0x00001, 0x08000, CRC(71821d4a) SHA1(d137df960f7ffd9c3e14258fc59e15f0997d56bc))
	ROM_LOAD16_BYTE("24-1154-02_j.bin", 0x10000, 0x08000, CRC(0d027d41) SHA1(0a4ffc0d68f632d70def8ff009dedad971b38d31))
	ROM_LOAD16_BYTE("24-1154-04_j.bin", 0x10001, 0x08000, CRC(e5e6fe2e) SHA1(0f130e808ed75efd7525ced5ae30193e60f33644))

	ROM_REGION(0x4000, "chargen", 0)
	ROM_LOAD("24-1140-00_a.bin", 0x0000, 0x4000, CRC(7a4d0b82) SHA1(15952655cef77918a76c0c268b749be34b28634b))
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME        PARENT      COMPAT  MACHINE     INPUT  CLASS             INIT        COMPANY    FULLNAME  FLAGS
COMP( 1987, digilog400, 0,          0,      digilog400, 0,     digilog400_state, empty_init, "Digilog", "400",    MACHINE_IS_SKELETON )
