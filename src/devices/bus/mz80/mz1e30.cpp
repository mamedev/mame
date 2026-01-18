// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

Sharp MZ-1E30 SASI I/F "IPLPRO"

http://retropc.net/ohishi/museum/mz1e30.htm
https://github.com/SuperTurboZ/Enhanced-SASI-driver-for-MZ-2500

TODO:
- just enough to make it load the ROM;

**************************************************************************************************/


#include "emu.h"
#include "mz1e30.h"

#include "sound/ymopl.h"
#include "speaker.h"

namespace {

class mz1e30_device : public mz80_exp_device
{
public:
	mz1e30_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::DISK; }

	virtual void io_map(address_map &map) override ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	u8 *m_iplpro_rom = nullptr;

	uint32_t m_rom_index = 0;
	u8 m_hrom_index = 0;
	u8 m_lrom_index = 0;

	u8 rom_r(offs_t offset);
	void rom_w(offs_t offset, u8 data);
};

mz1e30_device::mz1e30_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mz80_exp_device(mconfig, MZ1E30, tag, owner, clock)
{
}

ROM_START( mz1e30 )
	ROM_REGION( 0x10000, "iplpro", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "hd_v10a", "Sharp HD System V1.0A")
	ROMX_LOAD( "sasi.rom", 0x0000, 0x8000, CRC(a7bf39ce) SHA1(3f4a237fc4f34bac6fe2bbda4ce4d16d42400081), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "eh2022", "SuperTurboZ Enhanced SASI Ver.20220524")
	ROMX_LOAD( "sasirom.bin", 0x0000, 0x8000, CRC(f486b66b) SHA1(8b4f722e9a61c3e6aacebcda03c67253b8d9f468), ROM_BIOS(1) )
ROM_END

const tiny_rom_entry *mz1e30_device::device_rom_region() const
{
	return ROM_NAME( mz1e30 );
}


void mz1e30_device::device_start()
{
	m_iplpro_rom = memregion("iplpro")->base();
}

void mz1e30_device::io_map(address_map &map)
{
//  map(0xa4, 0xa5).mirror(0xff00).rw(FUNC(mz1e30_device::sasi_r), FUNC(mz1e30_device::sasi_w));
	map(0xa8, 0xa8).select(0xff00).w(FUNC(mz1e30_device::rom_w));
	map(0xa9, 0xa9).select(0xff00).r(FUNC(mz1e30_device::rom_r));
}

void mz1e30_device::device_add_mconfig(machine_config &config)
{
	// TODO: SASI I/F
}

u8 mz1e30_device::rom_r(offs_t offset)
{
	m_lrom_index = (offset >> 8) & 0xff;

	m_rom_index = (m_rom_index & 0xffff00) | (m_lrom_index & 0xff);

	return m_iplpro_rom[m_rom_index];
}

void mz1e30_device::rom_w(offs_t offset, u8 data)
{
	m_hrom_index = (offset >> 8) & 0xff;

	m_rom_index = (data << 8) | (m_rom_index & 0x0000ff) | ((m_hrom_index & 0xff)<<16);
	//logerror("%02x\n",data);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(MZ1E30, device_mz80_exp_interface, mz1e30_device, "mz1e30", "Sharp MZ-1E30 SASI I/F")
