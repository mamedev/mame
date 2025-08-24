// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    Konami PowerPC medal hardware

    GI Turf Wild 3 (c) 2005  GSF51
*/

#include "emu.h"
#include "k057714.h"

#include "bus/ata/ataintf.h"
#include "bus/ata/hdd.h"
#include "cpu/powerpc/ppc.h"
#include "sound/ymz280b.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

namespace {

class konmedalppc_state : public driver_device
{
public:
	konmedalppc_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gcu(*this, "gcu"),
		m_ata(*this, "ata"),
		m_work_ram(*this, "work_ram")
	{ }

	void konmedalppc(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<ppc_device> m_maincpu;
	required_device<k057714_device> m_gcu;
	required_device<ata_interface_device> m_ata;
	required_shared_ptr<uint32_t> m_work_ram;

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
	void ymz280b_map(address_map &map) ATTR_COLD;

	u16 ata_r(offs_t offset, u16 mem_mask);
	void ata_w(offs_t offset, u16 data, u16 mem_mask);
};

uint32_t konmedalppc_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return m_gcu->draw(screen, bitmap, cliprect);
}

/*
   Offset 0 is the full 16 bits of ATA register 0 (1F0h on PC)
   Offset 1 is ATA registers 2 & 3 (3322)
   Offset 2 is ATA registers 4 & 5 (5544)
   Offset 3 is ATA registers 6 & 7 (7766)
*/
u16 konmedalppc_state::ata_r(offs_t offset, u16 mem_mask)
{
	if (offset == 0)
	{
		return m_ata->cs0_swap_r(offset, mem_mask);
	}

	u16 result = m_ata->cs0_r((offset<<1) + 1, 0x00ff)<<8;
	result |= m_ata->cs0_r(offset<<1, 0xff);
	return result;
}

void konmedalppc_state::ata_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (offset == 0)
	{
		m_ata->cs0_swap_w(offset, data, mem_mask);
		return;
	}

	if (ACCESSING_BITS_0_7)
	{
		m_ata->cs0_w(offset << 1, data & 0xff, 0xff);
	}

	if (ACCESSING_BITS_8_15)
	{
		m_ata->cs0_w((offset<<1) + 1, data>>8, 0xff);
	}
}

void konmedalppc_state::main_map(address_map &map)
{
	map(0x00000000, 0x01ffffff).ram().share("work_ram");
	map(0x70000000, 0x7000ffff).ram();
	map(0x71100000, 0x7110001f).ram();
	map(0x721001f0, 0x721001ff).rw(FUNC(konmedalppc_state::ata_r), FUNC(konmedalppc_state::ata_w));
	map(0x73000000, 0x730000ff).rw(m_gcu, FUNC(k057714_device::read), FUNC(k057714_device::write));
	map(0x7ff00000, 0x7ff7ffff).mirror(0x00080000).rom().region("program", 0);
}

void konmedalppc_state::ymz280b_map(address_map &map)
{
	map.global_mask(0x1fffff);
	map(0x000000, 0x1fffff).rom().region("ymz280b", 0);
}

static INPUT_PORTS_START( konmedalppc )
INPUT_PORTS_END

void konmedalppc_state::machine_start()
{
	m_maincpu->ppcdrc_set_options(PPCDRC_COMPATIBLE_OPTIONS);
	m_maincpu->ppcdrc_add_fastram(0x00000000, 0x01ffffff, false, m_work_ram);
}

void konmedalppc_state::machine_reset()
{
	m_gcu->set_pixclock(66_MHz_XTAL);   // VESA 1024x768 @ 60 Hz has a pixel clock in this range
}

void konmedalppc_devices(device_slot_interface &device)
{
	device.option_add("cfcard", ATA_CF);
}

void konmedalppc_state::konmedalppc(machine_config &config)
{
	// basic machine hardware
	PPC403GA(config, m_maincpu, 66000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &konmedalppc_state::main_map);

	ATA_INTERFACE(config, m_ata).options(konmedalppc_devices, "cfcard", nullptr, true);

	// video hardware
	PALETTE(config, "palette", palette_device::RGB_555);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_size(1280, 800);
	screen.set_visarea(0, 1024-1, 0, 768-1);
	screen.set_screen_update(FUNC(konmedalppc_state::screen_update));
	screen.set_palette("palette");

	K057714(config, m_gcu, 0).set_screen("screen");

	SPEAKER(config, "speaker", 2).front();

	ymz280b_device &ymz(YMZ280B(config, "ymz", 16934400));
	ymz.set_addrmap(0, &konmedalppc_state::ymz280b_map);
	ymz.add_route(1, "speaker", 1.0, 0);
	ymz.add_route(0, "speaker", 1.0, 1);
}

ROM_START( turfwld3 )
	ROM_REGION32_BE(0x80000, "program", 0) // PowerPC program ROMs
	ROM_LOAD16_WORD_SWAP("a89-pce-b01.u2", 0x000000, 0x080000, CRC(3c5d44d4) SHA1(5ba46e3405b3c2ed5b4880abbbe557064b3492e9))

	ROM_REGION(0x200000, "ymz280b", 0)  // YMZ280B samples
	ROM_LOAD("29f016.u130", 0x000000, 0x200000, CRC(4aed68b5) SHA1(40e019d389a252f6f210b2a6ebadd2a92d5b692a))

	DISK_REGION("ata:0:cfcard")
	// Sector 0 appears to have been overwritten by Windows, the information needed to chain the boot is now "Replace the disk and press any key"
	DISK_IMAGE("f51-pce-a02", 0, BAD_DUMP SHA1(a2971e1c10071c6ae8d9e74c632d0d0b175f69f4))
ROM_END

} // Anonymous namespace

GAME(2005, turfwld3, 0, konmedalppc, konmedalppc, konmedalppc_state, empty_init, ROT0, "Konami", "GI Turf Wild 3", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
