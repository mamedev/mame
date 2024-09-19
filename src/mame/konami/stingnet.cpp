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

    TODO on Tropical Chance:
    - Finish inputs/DIPs
    - Find out why on a clean first boot the ROM doesn't try to flash the sound ROM
      (bpset fffa9f6c, r3=ff, g or F12 to force flashing and continue)
    - Hook up iButton and find out what data it wants.  You can prevent the game from freezing with the
      PASSWORD ERROR by setting a breakpoing on 0x100010 before the CD finishes loading and changing the
      byte at 0x108C0B to 0x00.  (Changes li r11, 0xff to li r11, 0x00).
*/

#include "emu.h"
#include "k057714.h"

#include "bus/ata/ataintf.h"
#include "bus/ata/atapicdr.h"
#include "cpu/powerpc/ppc.h"
#include "machine/ins8250.h"
#include "machine/intelfsh.h"
#include "machine/msm6242.h"
#include "machine/nvram.h"
#include "sound/ymz280b.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

// This must be outside of the namespace
DECLARE_DEVICE_TYPE(STINGNET_ATAPI_CDROM, stingnet_cdr)

class stingnet_cdr : public atapi_fixed_cdrom_device
{
public:
	stingnet_cdr(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: atapi_fixed_cdrom_device(mconfig, STINGNET_ATAPI_CDROM, tag, owner, clock)
	{
	}

	virtual void device_start() override
	{
		m_sector_timer = timer_alloc(FUNC(stingnet_cdr::sector_tick), this);
		m_sector_timer->adjust(attotime::never);

		atapi_fixed_cdrom_device::device_start();
	}

	// atapicdr has zero delay between the end of a sector and the completion of the next.
	// The boot PROM reads each sector in the ATA IRQ handler, unconditionally
	// clears the IRQ, and exits to wait for the IRQ for the next sector, if any.
	// Without this, the next sector's IRQ fires before the code clears the first IRQ,
	// resulting in the IRQ being lost and the code twiddling its thumbs waiting for
	// an IRQ that never comes.
	virtual void fill_buffer() override
	{
		// tropchnc issues a SET CD SPEED command to 2822KiB/second, which is roughly 18X.
		// 1X is a sector every 75th of a second, so 18X is 75 * 18 = 1350 Hz
		m_sector_timer->adjust(attotime::from_hz(1350));
	}

	TIMER_CALLBACK_MEMBER(sector_tick)
	{
		m_sector_timer->adjust(attotime::never);
		atapi_fixed_cdrom_device::fill_buffer();
	}

private:
	emu_timer *m_sector_timer;
};

DEFINE_DEVICE_TYPE(STINGNET_ATAPI_CDROM, stingnet_cdr, "stingnet_cdr", "Stingnet ATAPI CD-ROM")

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
		m_work_ram(*this, "work_ram"),
		m_nvram(*this, "nvram"),
		m_rtc(*this, "rtc"),
		m_ymz(*this, "ymz"),
		m_sndflash(*this, "sndflash"),
		m_control(0),
		m_ata_irq_pending(false)
	{ }

	void stingnet(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<ppc_device> m_maincpu;
	required_device<k057714_device> m_gcu;
	required_device<ata_interface_device> m_ata;
	required_device<pc16552_device> m_duart;
	required_shared_ptr<uint32_t> m_work_ram;
	required_device<nvram_device> m_nvram;
	required_device<rtc62423_device> m_rtc;
	required_device<ymz280b_device> m_ymz;
	required_device<fujitsu_29f016a_device> m_sndflash;

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
	void ymz280b_map(address_map &map) ATTR_COLD;

	void gcu_interrupt(int state);
	void ata_interrupt(int state);

	u8 status_r();
	u8 control_r();
	void control_w(u8 data);

	void tropchnc_nvram_init(nvram_device &nvram, void *base, size_t size);

	u8 m_control;
	bool m_ata_irq_pending;
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
	if (state)
	{
		if (BIT(m_control, 2))
		{
			m_control &= ~4;
			m_maincpu->set_input_line(INPUT_LINE_IRQ3, ASSERT_LINE);
		}
		else
		{
			m_ata_irq_pending = true;
		}
	}
	else
	{
		m_maincpu->set_input_line(INPUT_LINE_IRQ3, CLEAR_LINE);
	}
}

u8 stingnet_state::status_r()
{
	// bits 4-5 = battery state (0 = no voltage, 1 & 2 = low, 3 = OK)
	// bit 6 = ready/*busy line from the sound flash
	return 0x03 | (m_sndflash->is_ready() ? 0x04 : 0x00);
}

u8 stingnet_state::control_r()
{
	return m_control;
}

void stingnet_state::control_w(u8 data)
{
	m_control = data;

	if ((BIT(m_control, 2)) && (m_ata_irq_pending))
	{
		m_maincpu->set_input_line(INPUT_LINE_IRQ3, ASSERT_LINE);
		m_ata_irq_pending = false;
	}
}

void stingnet_state::main_map(address_map &map)
{
	map(0x00000000, 0x01ffffff).ram().share("work_ram");
	map(0x70000000, 0x7000ffff).ram().share("nvram");   // security NVRAM
	map(0x70100000, 0x7010000f).rw(m_rtc, FUNC(rtc62423_device::read), FUNC(rtc62423_device::write));
	map(0x70400000, 0x70400001).rw(m_ymz, FUNC(ymz280b_device::read), FUNC(ymz280b_device::write));
	map(0x70600000, 0x707fffff).rw(m_sndflash, FUNC(fujitsu_29f016a_device::read), FUNC(fujitsu_29f016a_device::write));
	map(0x71000000, 0x71000003).portr("IN1").nopw();
	map(0x71000004, 0x71000007).portr("IN2").nopw();
	map(0x71000006, 0x71000006).r(FUNC(stingnet_state::status_r));
	map(0x71000011, 0x71000011).rw(FUNC(stingnet_state::control_r), FUNC(stingnet_state::control_w));
	map(0x72000000, 0x7200000f).rw(m_ata, FUNC(ata_interface_device::cs0_r), FUNC(ata_interface_device::cs0_w));
	map(0x72000010, 0x7200001f).rw(m_ata, FUNC(ata_interface_device::cs1_r), FUNC(ata_interface_device::cs1_w));
	map(0x73000000, 0x730000ff).rw(m_gcu, FUNC(k057714_device::read), FUNC(k057714_device::write));
	map(0x7ff00000, 0x7ff7ffff).mirror(0x00080000).rom().region("program", 0);
}

void stingnet_state::ymz280b_map(address_map &map)
{
	map.global_mask(0x1fffff);
	map(0x000000, 0x1fffff).r(m_sndflash, FUNC(fujitsu_29f016a_device::read));
}

static INPUT_PORTS_START( stingnet )
	PORT_START("IN1")
	PORT_DIPUNKNOWN_DIPLOC( 0x0001, IP_ACTIVE_LOW, "DIP SW3:8" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0002, IP_ACTIVE_HIGH, "DIP SW3:7" ) // Hopper sense, active high
	PORT_DIPUNKNOWN_DIPLOC( 0x0004, IP_ACTIVE_LOW, "DIP SW3:6" ) // Select arm
	PORT_DIPUNKNOWN_DIPLOC( 0x0008, IP_ACTIVE_LOW, "DIP SW3:5" ) // Select sen
	PORT_DIPUNKNOWN_DIPLOC( 0x0010, IP_ACTIVE_LOW, "DIP SW3:4" )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_MEMORY_RESET ) // Reset key
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_TILT ) // Meter key

	PORT_DIPUNKNOWN_DIPLOC( 0x0100, IP_ACTIVE_LOW, "ARCNET:8" ) // All of these are for the arcnet ID
	PORT_DIPUNKNOWN_DIPLOC( 0x0200, IP_ACTIVE_LOW, "ARCNET:7" ) // Combined they make an ID up to 255
	PORT_DIPUNKNOWN_DIPLOC( 0x0400, IP_ACTIVE_LOW, "ARCNET:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0800, IP_ACTIVE_LOW, "ARCNET:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x1000, IP_ACTIVE_LOW, "ARCNET:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x2000, IP_ACTIVE_LOW, "ARCNET:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x4000, IP_ACTIVE_LOW, "ARCNET:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x8000, IP_ACTIVE_LOW, "ARCNET:1" )

	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_GAMBLE_HALF )
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Help")

	PORT_START("IN2")
	PORT_DIPUNKNOWN_DIPLOC( 0x00010000, IP_ACTIVE_LOW, "DIP SW2:8" )
	PORT_DIPUNKNOWN_DIPLOC( 0x00020000, IP_ACTIVE_LOW, "DIP SW2:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x00040000, IP_ACTIVE_LOW, "DIP SW2:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x00080000, IP_ACTIVE_LOW, "DIP SW2:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x00100000, IP_ACTIVE_LOW, "DIP SW2:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x00200000, IP_ACTIVE_LOW, "DIP SW2:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x00400000, IP_ACTIVE_LOW, "DIP SW2:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x00800000, IP_ACTIVE_LOW, "DIP SW2:1" )

	PORT_DIPUNKNOWN_DIPLOC( 0x01000000, IP_ACTIVE_LOW, "DIP SW1:8" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02000000, IP_ACTIVE_LOW, "DIP SW1:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04000000, IP_ACTIVE_LOW, "DIP SW1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08000000, IP_ACTIVE_LOW, "DIP SW1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10000000, IP_ACTIVE_LOW, "DIP SW1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20000000, IP_ACTIVE_LOW, "DIP SW1:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40000000, IP_ACTIVE_LOW, "DIP SW1:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80000000, IP_ACTIVE_LOW, "DIP SW1:1" )
INPUT_PORTS_END

void stingnet_state::machine_start()
{
	m_maincpu->ppcdrc_set_options(PPCDRC_COMPATIBLE_OPTIONS);
	m_maincpu->ppcdrc_add_fastram(0x00000000, 0x01ffffff, false, m_work_ram);
}

void stingnet_state::tropchnc_nvram_init(nvram_device &nvram, void *base, size_t size)
{
	// is NVRAM valid?
	static char magic[11] = "TRAVEL   A";
	u8 *nv = (u8 *)base;
	bool nvValid = true;

	for (int i = 0; i < 10; i++)
	{
		if (nv[BYTE4_XOR_BE(0x1104 + i)] != magic[i])
		{
			nvValid = false;
			break;
		}
	}

	// Create a valid NVRAM.  This is similar to what the code at 0x107B44 does, but you can't get to that code
	// on first boot without a minimally valid NVRAM.
	if (!nvValid)
	{
		std::fill_n(nv, size, 0);

		for (int i = 0; i < 10; i++)
		{
			nv[BYTE4_XOR_BE(0x1104 + i)] = magic[i];
		}
	}
}

void stingnet_state::machine_reset()
{
	// assume VGA 640x480
	m_gcu->set_pixclock(25.175_MHz_XTAL);
}

void stingnet_devices(device_slot_interface &device)
{
	device.option_add("cdrom", STINGNET_ATAPI_CDROM);
}

void stingnet_state::stingnet(machine_config &config)
{
	// basic machine hardware
	PPC403GA(config, m_maincpu, 33'000'000);        // TODO: unknown clocks, but 403GA is rated for 25 or 33 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &stingnet_state::main_map);

	ATA_INTERFACE(config, m_ata).options(stingnet_devices, "cdrom", nullptr, true);
	m_ata->irq_handler().set(FUNC(stingnet_state::ata_interrupt));

	NVRAM(config, m_nvram, nvram_device::DEFAULT_ALL_0);
	m_nvram->set_custom_handler(FUNC(stingnet_state::tropchnc_nvram_init));

	FUJITSU_29F016A(config, m_sndflash);

	RTC62423(config, m_rtc, 0);

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

	ymz280b_device &ymz(YMZ280B(config, m_ymz, 16934400));
	ymz.set_addrmap(0, &stingnet_state::ymz280b_map);
	ymz.add_route(1, "lspeaker", 1.0);
	ymz.add_route(0, "rspeaker", 1.0);
}

ROM_START( tropchnc )
	ROM_REGION32_BE(0x80000, "program", 0) // PowerPC program ROMs
	ROM_LOAD("839ssua01.bin", 0x000000, 0x080000, CRC(007ae177) SHA1(7ded12ee3c07ce1d607d1c9fba2c0e3a69dfb294))

	ROM_REGION(0x200000, "ymz280b", ROMREGION_ERASE00)  // YMZ280B samples

	DISK_REGION("ata:0:cdrom")
	DISK_IMAGE("gc968_ver_01", 0, SHA1(e96731a68e306876b9665cb9c1d69b9aa38acc3b))
ROM_END

} // Anonymous namespace

GAME(1999, tropchnc, 0, stingnet, stingnet, stingnet_state, empty_init, ROT90, "Konami", "Tropical Chance", MACHINE_NOT_WORKING)
