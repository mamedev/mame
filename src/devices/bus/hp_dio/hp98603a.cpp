// license:BSD-3-Clause
// copyright-holders:Sven Schnelle
/***************************************************************************

  HP98603A BASIC ROM card

***************************************************************************/

#include "emu.h"
#include "hp98603a.h"


#define HP98603A_ROM_REGION    "98603a_rom"

namespace {

class dio16_98603a_device :
		public device_t,
		public bus::hp_dio::device_dio16_card_interface
{
public:
	// construction/destruction
	dio16_98603a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	uint16_t rom_r(offs_t offset);
	void rom_w(offs_t offset, uint16_t data);

protected:
	dio16_98603a_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	uint8_t *m_rom;
};

ROM_START(hp98603a)
	ROM_REGION(0x80000, HP98603A_ROM_REGION, 0)
	ROM_LOAD16_BYTE("98603_80001.bin", 0x00001, 32768, CRC(5d8c9657) SHA1(f5d89e7f8a61072f362532d1b5e05f5b5e3f42b3))
	ROM_LOAD16_BYTE("98603_80002.bin", 0x10001, 32768, CRC(7e4e0ca8) SHA1(3230c7825f5058a1e1a06776da6064b049417b50))
	ROM_LOAD16_BYTE("98603_80003.bin", 0x20001, 32768, CRC(b5da5f81) SHA1(69578ee197531e9474a0edb4c2f068b20118d25f))
	ROM_LOAD16_BYTE("98603_80004.bin", 0x30001, 32768, CRC(ea050835) SHA1(3be7c1bd394b11fdfee38ebf40beb9352f38722d))

	ROM_LOAD16_BYTE("98603_80005.bin", 0x00000, 32768, CRC(1380d489) SHA1(e346857d9df9a06269f9fa57684fbe64edd0134b))
	ROM_LOAD16_BYTE("98603_80006.bin", 0x10000, 32768, CRC(19752031) SHA1(5e28ee9d527b297a8898dde50ab6e41bbc0c2572))
	ROM_LOAD16_BYTE("98603_80007.bin", 0x20000, 32768, CRC(72aa2225) SHA1(3c92cdd40adc7438d30b95eae14e4497b7e14f38))
	ROM_LOAD16_BYTE("98603_80008.bin", 0x30000, 32768, CRC(f617ae2e) SHA1(f59b63058e7e87eabc5dc5233895bb57579eb3f7))

	ROM_LOAD16_BYTE("98603_80009.bin", 0x40001, 32768, CRC(9c128571) SHA1(7827cfbf710d8bba3ce9b42ec3368b17a5b5ff78))
	ROM_LOAD16_BYTE("98603_80010.bin", 0x50001, 32768, CRC(12528317) SHA1(e11d6c40334c41be68e2717d816e0895f8f7afa8))
	ROM_LOAD16_BYTE("98603_80011.bin", 0x60001, 32768, CRC(1d279451) SHA1(2ce966b8499d2d810056d147f71ba2b6cc909bc6))
	ROM_LOAD16_BYTE("98603_80012.bin", 0x70001, 32768, CRC(a1d6374e) SHA1(ca860b762503a05a4bfbd17a77bef28d01d7d279))

	ROM_LOAD16_BYTE("98603_80013.bin", 0x40000, 32768, CRC(e554d97e) SHA1(c792c4f0097bed032ca31c88fde660d4db25b3dc))
	ROM_LOAD16_BYTE("98603_80014.bin", 0x50000, 32768, CRC(c0db7e02) SHA1(add3e7312ac07e5ed4281aae2d46e1a4389d6ea2))
	ROM_LOAD16_BYTE("98603_80015.bin", 0x60000, 32768, CRC(f85bb699) SHA1(bd7f690e3b4fb8952517b0acd6ac878cd50f5736))
	ROM_LOAD16_BYTE("98603_80016.bin", 0x70000, 32768, CRC(d887acab) SHA1(a9cbbaa5f053f374d6cbda614b727df35a61ace1))
ROM_END

void dio16_98603a_device::device_add_mconfig(machine_config &config)
{
}

const tiny_rom_entry *dio16_98603a_device::device_rom_region() const
{
	return ROM_NAME(hp98603a);
}

dio16_98603a_device::dio16_98603a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	dio16_98603a_device(mconfig, HPDIO_98603A, tag, owner, clock)
{
}

dio16_98603a_device::dio16_98603a_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_dio16_card_interface(mconfig, *this)
{
}

void dio16_98603a_device::device_start()
{
}

void dio16_98603a_device::device_reset()
{
	m_rom = device().machine().root_device().memregion(this->subtag(HP98603A_ROM_REGION).c_str())->base();
	dio().install_memory(0x80000, 0xfffff,
			read16sm_delegate(*this, FUNC(dio16_98603a_device::rom_r)),
			write16sm_delegate(*this, FUNC(dio16_98603a_device::rom_w)));
}

uint16_t dio16_98603a_device::rom_r(offs_t offset)
{
	return m_rom[offset*2] | (m_rom[offset*2+1] << 8);
}

void dio16_98603a_device::rom_w(offs_t offset, uint16_t data)
{
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(HPDIO_98603A, bus::hp_dio::device_dio16_card_interface, dio16_98603a_device, "dio98603a", "HP98603A BASIC 4.0 ROM card")
