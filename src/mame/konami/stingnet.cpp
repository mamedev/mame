// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    Konami "Stingnet" medal hardware

    PowerPC 403GA
    Yamaha YMZ280B
    ATAPI CD-ROM
    Dallas DS1991 iButton security

    Dumped games:
    GC968 Tropical Chance

    Other known games on this hardware:
    GC965 Magic Timer Poker
    GC966 Joker Time
    GC967 Egyptian Slot

    TODO: With VBL IRQs working, the deferred function to load the first stage boot from the CD-ROM is jumped to
    at 0xFFFA9578 but it immediately hits an alignment fault trying to store a 32-bit word to a 16-bit aligned
    address.  (Does the 403 maybe not have that exception?  The PPC architecture docs indicate it's not a requirement
    for all implementations.  Or is this an early security failure?).
*/

#include "emu.h"
#include "k057714.h"

#include "bus/ata/ataintf.h"
#include "bus/ata/atapicdr.h"
#include "cpu/powerpc/ppc.h"
#include "machine/ins8250.h"
#include "machine/intelfsh.h"
#include "sound/ymz280b.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

namespace {

class stingnet_state : public driver_device
{
public:
	stingnet_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gcu(*this, "gcu"),
		m_ata(*this, "ata"),
		m_duart(*this, "duart"),
		m_work_ram(*this, "work_ram")
	{ }

	void stingnet(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<ppc_device> m_maincpu;
	required_device<k057714_device> m_gcu;
	required_device<ata_interface_device> m_ata;
	required_device<pc16552_device> m_duart;
	required_shared_ptr<uint32_t> m_work_ram;

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map);
	void ymz280b_map(address_map &map);

	void gcu_interrupt(int state);
	void ata_interrupt(int state);

	u32 unk_r(offs_t offset, u32 mem_mask);
};

uint32_t stingnet_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return m_gcu->draw(screen, bitmap, cliprect);
}

void stingnet_state::gcu_interrupt(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ1, state);
}

void stingnet_state::ata_interrupt(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ2, state);
}

void stingnet_state::main_map(address_map &map)
{
	map(0x00000000, 0x01ffffff).ram().share("work_ram");
	map(0x71000000, 0x71000007).noprw();    // TODO: what is going on here?
	map(0x71000010, 0x71000013).ram();
	map(0x72000000, 0x7200000f).rw(m_ata, FUNC(ata_interface_device::cs0_r), FUNC(ata_interface_device::cs0_w));
	map(0x73000000, 0x730000ff).rw(m_gcu, FUNC(k057714_device::read), FUNC(k057714_device::write));
	map(0x7ff00000, 0x7ff7ffff).mirror(0x00080000).rom().region("program", 0);
}

void stingnet_state::ymz280b_map(address_map &map)
{
	map.global_mask(0x1fffff);
	map(0x000000, 0x1fffff).rom().region("ymz280b", 0);
}

static INPUT_PORTS_START( stingnet )
INPUT_PORTS_END

void stingnet_state::machine_start()
{
	m_maincpu->ppcdrc_set_options(PPCDRC_COMPATIBLE_OPTIONS);
	m_maincpu->ppcdrc_add_fastram(0x00000000, 0x01ffffff, false, m_work_ram);
}

void stingnet_state::machine_reset()
{
	// assume VGA 640x480
	m_gcu->set_pixclock(25.175_MHz_XTAL);
}

void stingnet_devices(device_slot_interface &device)
{
	device.option_add("cdrom", ATAPI_FIXED_CDROM);
}

void stingnet_state::stingnet(machine_config &config)
{
	// basic machine hardware
	PPC403GA(config, m_maincpu, 66000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &stingnet_state::main_map);

	ATA_INTERFACE(config, m_ata).options(stingnet_devices, "cdrom", nullptr, true);
	m_ata->irq_handler().set(FUNC(stingnet_state::ata_interrupt));

	// video hardware
	PALETTE(config, "palette", palette_device::RGB_555);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_size(800, 600);
	screen.set_visarea(0, 640-1, 0, 480-1);
	screen.set_screen_update(FUNC(stingnet_state::screen_update));
	screen.set_palette("palette");
	screen.screen_vblank().set(m_gcu, FUNC(k057714_device::vblank_w));

	K057714(config, m_gcu, 0).set_screen("screen");
	m_gcu->irq_callback().set(FUNC(stingnet_state::gcu_interrupt));

	PC16552D(config, m_duart, 0);
	NS16550(config, "duart:chan0", XTAL(19'660'800));
	NS16550(config, "duart:chan1", XTAL(19'660'800));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ymz280b_device &ymz(YMZ280B(config, "ymz", 16934400));
	ymz.set_addrmap(0, &stingnet_state::ymz280b_map);
	ymz.add_route(1, "lspeaker", 1.0);
	ymz.add_route(0, "rspeaker", 1.0);
}

ROM_START( tropchnc )
	ROM_REGION32_BE(0x80000, "program", 0) // PowerPC program ROMs
	ROM_LOAD("839ssua01.bin", 0x000000, 0x080000, CRC(007ae177) SHA1(7ded12ee3c07ce1d607d1c9fba2c0e3a69dfb294))

	ROM_REGION(0x200000, "ymz280b", 0)  // YMZ280B samples

	DISK_REGION("ata:0:cdrom")
	DISK_IMAGE("gc968_ver_01", 0, SHA1(e96731a68e306876b9665cb9c1d69b9aa38acc3b))
ROM_END

} // Anonymous namespace

GAME(1999, tropchnc, 0, stingnet, stingnet, stingnet_state, empty_init, ROT90, "Konami", "Tropical Chance", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
