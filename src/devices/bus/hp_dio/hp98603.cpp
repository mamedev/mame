// license:BSD-3-Clause
// copyright-holders:Sven Schnelle
/***************************************************************************

  HP98603 BASIC ROM card

***************************************************************************/

#include "emu.h"
#include "hp98603.h"

DEFINE_DEVICE_TYPE(HPDIO_98603, dio16_98603_device, "dio98603", "HP98603 BASIC ROM card")

#define HP98603_ROM_REGION    "98603_rom"

ROM_START(hp98603)
	ROM_REGION(0x100000, HP98603_ROM_REGION, 0)

	ROM_SYSTEM_BIOS(0, "basic4", "BASIC 4.0")
	ROMX_LOAD("98603_80001.bin", 0x00001, 32768, CRC(5d8c9657) SHA1(f5d89e7f8a61072f362532d1b5e05f5b5e3f42b3), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("98603_80002.bin", 0x10001, 32768, CRC(7e4e0ca8) SHA1(3230c7825f5058a1e1a06776da6064b049417b50), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("98603_80003.bin", 0x20001, 32768, CRC(b5da5f81) SHA1(69578ee197531e9474a0edb4c2f068b20118d25f), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("98603_80004.bin", 0x30001, 32768, CRC(ea050835) SHA1(3be7c1bd394b11fdfee38ebf40beb9352f38722d), ROM_SKIP(1) | ROM_BIOS(1))

	ROMX_LOAD("98603_80005.bin", 0x00000, 32768, CRC(1380d489) SHA1(e346857d9df9a06269f9fa57684fbe64edd0134b), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("98603_80006.bin", 0x10000, 32768, CRC(19752031) SHA1(5e28ee9d527b297a8898dde50ab6e41bbc0c2572), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("98603_80007.bin", 0x20000, 32768, CRC(72aa2225) SHA1(3c92cdd40adc7438d30b95eae14e4497b7e14f38), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("98603_80008.bin", 0x30000, 32768, CRC(f617ae2e) SHA1(f59b63058e7e87eabc5dc5233895bb57579eb3f7), ROM_SKIP(1) | ROM_BIOS(1))

	ROMX_LOAD("98603_80009.bin", 0x40001, 32768, CRC(9c128571) SHA1(7827cfbf710d8bba3ce9b42ec3368b17a5b5ff78), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("98603_80010.bin", 0x50001, 32768, CRC(12528317) SHA1(e11d6c40334c41be68e2717d816e0895f8f7afa8), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("98603_80011.bin", 0x60001, 32768, CRC(1d279451) SHA1(2ce966b8499d2d810056d147f71ba2b6cc909bc6), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("98603_80012.bin", 0x70001, 32768, CRC(a1d6374e) SHA1(ca860b762503a05a4bfbd17a77bef28d01d7d279), ROM_SKIP(1) | ROM_BIOS(1))

	ROMX_LOAD("98603_80013.bin", 0x40000, 32768, CRC(e554d97e) SHA1(c792c4f0097bed032ca31c88fde660d4db25b3dc), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("98603_80014.bin", 0x50000, 32768, CRC(c0db7e02) SHA1(add3e7312ac07e5ed4281aae2d46e1a4389d6ea2), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("98603_80015.bin", 0x60000, 32768, CRC(f85bb699) SHA1(bd7f690e3b4fb8952517b0acd6ac878cd50f5736), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("98603_80016.bin", 0x70000, 32768, CRC(d887acab) SHA1(a9cbbaa5f053f374d6cbda614b727df35a61ace1), ROM_SKIP(1) | ROM_BIOS(1))

	ROM_SYSTEM_BIOS(1, "basic51", "BASIC 5.1")
	ROMX_LOAD("u1.bin", 0x00000, 65536, CRC(ba50d9e0) SHA1(064e3cbbc7ee01f279a289684616ae3b8744a7c9), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("u2.bin", 0x20000, 65536, CRC(65f58d3e) SHA1(8367cecb46fa6f9abe9e78b42c25ce8699d5d3f3), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("u3.bin", 0x40000, 65536, CRC(9bc70573) SHA1(b7e375e11f6758d59d32236fc1c42eafc892d247), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("u4.bin", 0x60000, 65536, CRC(52debeba) SHA1(996e83e604501979fd80ae47ff1cda9890613982), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("u5.bin", 0x80000, 65536, CRC(93b4bce8) SHA1(77efec9a95b9e543e1cdb00196d1794ca0c8b4ba), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("u6.bin", 0xa0000, 65536, CRC(bda3d054) SHA1(c83d6f571ce5aa63ad3954ac02ff4069c01a1464), ROM_SKIP(1) | ROM_BIOS(2))

	ROMX_LOAD("u9.bin", 0x00001, 65536, CRC(009e9fcb) SHA1(520f583a826516f7d676391daf31ed035162c263), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("u10.bin", 0x20001, 65536, CRC(84f90a1d) SHA1(9358aa7fa83ed9617d318936cbab79002853b0ce), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("u11.bin", 0x40001, 65536, CRC(e486e0f3) SHA1(66b5dd6c2c277156ad6c3ee5b99c34e243d897be), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("u12.bin", 0x60001, 65536, CRC(d5a08c7b) SHA1(be997cf9bbdb8fdd6c7e95754747016293c51c7f), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("u13.bin", 0x80001, 65536, CRC(9811c34c) SHA1(1655cac651889950b881e6be72f035c7de0a1aed), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("u14.bin", 0xa0001, 65536, CRC(96527d4e) SHA1(6706ab97eab4465ea4fa2d6b07e8107468e83818), ROM_SKIP(1) | ROM_BIOS(2))
ROM_END

MACHINE_CONFIG_MEMBER( dio16_98603_device::device_add_mconfig )
MACHINE_CONFIG_END

const tiny_rom_entry *dio16_98603_device::device_rom_region() const
{
	return ROM_NAME(hp98603);
}

dio16_98603_device::dio16_98603_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	dio16_98603_device(mconfig, HPDIO_98603, tag, owner, clock)
{
}

dio16_98603_device::dio16_98603_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_dio16_card_interface(mconfig, *this)
{
}

void dio16_98603_device::device_start()
{
	set_dio_device();
}

void dio16_98603_device::device_reset()
{
		m_rom = device().machine().root_device().memregion(this->subtag(HP98603_ROM_REGION).c_str())->base();
		m_dio->install_memory(0x100000, 0x1fffff, read16_delegate(FUNC(dio16_98603_device::rom_r), this),
				      write16_delegate(FUNC(dio16_98603_device::rom_w), this));
}

READ16_MEMBER(dio16_98603_device::rom_r)
{
	return m_rom[offset*2] | (m_rom[offset*2+1] << 8);
}

WRITE16_MEMBER(dio16_98603_device::rom_w)
{
}
