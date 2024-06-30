// license:BSD-3-Clause
// copyright-holders:Sven Schnelle
/***************************************************************************

  HP98603B BASIC ROM card

***************************************************************************/

#include "emu.h"
#include "hp98603b.h"


#define HP98603B_ROM_REGION    "98603b_rom"

namespace {

class dio16_98603b_device :
		public device_t,
		public bus::hp_dio::device_dio16_card_interface
{
public:
	// construction/destruction
	dio16_98603b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	uint16_t rom_r(offs_t offset);
	void rom_w(offs_t offset, uint16_t data);

protected:
	dio16_98603b_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	uint8_t *m_rom;
};

ROM_START(hp98603b)
	ROM_REGION(0x100000, HP98603B_ROM_REGION, 0)

	ROM_LOAD16_BYTE("u1.bin", 0x00000, 65536, CRC(ba50d9e0) SHA1(064e3cbbc7ee01f279a289684616ae3b8744a7c9))
	ROM_LOAD16_BYTE("u2.bin", 0x20000, 65536, CRC(65f58d3e) SHA1(8367cecb46fa6f9abe9e78b42c25ce8699d5d3f3))
	ROM_LOAD16_BYTE("u3.bin", 0x40000, 65536, CRC(9bc70573) SHA1(b7e375e11f6758d59d32236fc1c42eafc892d247))
	ROM_LOAD16_BYTE("u4.bin", 0x60000, 65536, CRC(52debeba) SHA1(996e83e604501979fd80ae47ff1cda9890613982))
	ROM_LOAD16_BYTE("u5.bin", 0x80000, 65536, CRC(93b4bce8) SHA1(77efec9a95b9e543e1cdb00196d1794ca0c8b4ba))
	ROM_LOAD16_BYTE("u6.bin", 0xa0000, 65536, CRC(bda3d054) SHA1(c83d6f571ce5aa63ad3954ac02ff4069c01a1464))

	ROM_LOAD16_BYTE("u9.bin", 0x00001, 65536, CRC(009e9fcb) SHA1(520f583a826516f7d676391daf31ed035162c263))
	ROM_LOAD16_BYTE("u10.bin", 0x20001, 65536, CRC(84f90a1d) SHA1(9358aa7fa83ed9617d318936cbab79002853b0ce))
	ROM_LOAD16_BYTE("u11.bin", 0x40001, 65536, CRC(e486e0f3) SHA1(66b5dd6c2c277156ad6c3ee5b99c34e243d897be))
	ROM_LOAD16_BYTE("u12.bin", 0x60001, 65536, CRC(d5a08c7b) SHA1(be997cf9bbdb8fdd6c7e95754747016293c51c7f))
	ROM_LOAD16_BYTE("u13.bin", 0x80001, 65536, CRC(9811c34c) SHA1(1655cac651889950b881e6be72f035c7de0a1aed))
	ROM_LOAD16_BYTE("u14.bin", 0xa0001, 65536, CRC(96527d4e) SHA1(6706ab97eab4465ea4fa2d6b07e8107468e83818))
ROM_END

void dio16_98603b_device::device_add_mconfig(machine_config &config)
{
}

const tiny_rom_entry *dio16_98603b_device::device_rom_region() const
{
	return ROM_NAME(hp98603b);
}

dio16_98603b_device::dio16_98603b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	dio16_98603b_device(mconfig, HPDIO_98603B, tag, owner, clock)
{
}

dio16_98603b_device::dio16_98603b_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_dio16_card_interface(mconfig, *this)
{
}

void dio16_98603b_device::device_start()
{
}

void dio16_98603b_device::device_reset()
{
		m_rom = device().machine().root_device().memregion(this->subtag(HP98603B_ROM_REGION).c_str())->base();
		dio().install_memory(0x100000, 0x1fffff,
				read16sm_delegate(*this, FUNC(dio16_98603b_device::rom_r)),
				write16sm_delegate(*this, FUNC(dio16_98603b_device::rom_w)));
}

uint16_t dio16_98603b_device::rom_r(offs_t offset)
{
	return m_rom[offset*2] | (m_rom[offset*2+1] << 8);
}

void dio16_98603b_device::rom_w(offs_t offset, uint16_t data)
{
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(HPDIO_98603B, bus::hp_dio::device_dio16_card_interface, dio16_98603b_device, "dio98603b", "HP98603 BASIC ROM card")
