// license:BSD-3-Clause
// copyright-holders:Ville Linde, Angelo Salese
/*  P5TX-LA / Taito Wolf System

Original legacy PCI driver by Ville Linde,
rewritten by Angelo Salese to use the new PCI model

Three board system consisting of a P5TX-LA PC motherboard, a Taito main board and a rom board.

TODO:
- The Retro Web MB pic lists a Winbond w83877tf Super I/O but neither BIOSes properly init that,
  eventually failing with p5txla later on with PnP sequence. Missing power on default?
- p5txla: Rage VGA chip sets up screen with 8x1, making MAME unresponsive.
          Needs x86 VGA legacy map bridge to fix.
- pf2012: verify ISA irq 7 source (particularly ACK, PORT_IMPULSE(1) won't work),
          pinpoint coin counters output and verify tc0510nio write 4 EEPROM style write
          on coin insertion;
- pf2012: Should show bootscreen when Voodoo fbiInit0 bit 0 is off (vga_pass), cfr. GH #11343;
- pf2012: boots in service mode the first time around, needs default EEPROM set;
- pf2012: service mode RTC item always initializes to 0 for hour count;
- pf2012: PC portion returns an EMM386 "WARNING: Unable to set page frame base address"
          during boot up, safe to ignore?

Notes:
- pf2012: -isa1 svga_et4k if you need PC side logs;
- pf2012: game responds to CTRL+ALT+DEL, which will reset the machine.
- pf2012: according to manual hold A+B+service button for 10 seconds for entering
          test mode during initial bootup sequence.
          Board and input test menu doesn't seem to have a dedicated test mode switch
          Update: it just goes "SERVICE SW ERROR"?

===================================================================================================

Hardware configuration:

P5TX-LA Motherboard:
-CPU: Intel SL27J Pentium MMX @ 200 MHz
-Onboard sound: Crystal CS4237B ISA Audio
-Onboard VGA: ATI Rage II 3D Graph (removed from motherboard)

Chipsets (430TX PCIset):
-82439TX Northbridge
-82371AB PIIX4 PCI-ISA Southbridge

Taito W Main Board:
-AMD M4-128N/64 CPLD stamped 'E58-01'
-AMD MACH231 CPLD stamped 'E58-02'
-AMD MACH211 CPLD stamped 'E58-03'
-Panasonic MN1020019 (MN10200 based) Sound CPU
-Zoom ZFX-2 DSP (TMS57002 DSP)
-Zoom ZSG-2 Sound PCM chip
-Taito TC0510NIO I/O chip
-1x RAM NEC 42S4260
-1x RAM GM71C4400
-12x RAM Alliance AS4C256K16E0-35 (256k x 16)
-Mitsubishi M66220 256 x 8-bit CMOS memory
-Fujitsu MB87078 6-bit, 4-channel Electronic Volume Controller
-Atmel 93C66 EEPROM (4kb probably for high scores, test mode settings etc)
-ICS5342-3 GENDAC 16-Bit Integrated Clock-LUT-DAC
-3DFX 500-0003-03 F805281.1 FBI
-3DFX 500-0004-02 F804701.1 TMU
-Rom: E58-04 (bootscreen)
-XTALs 50MHz (near 3DFX) and 14.31818MHz (near RAMDAC)

Taito W Rom Board:
-AMD M4-128N/64 CPLD stamped 'E58-05'
-Program, Sound roms

*/

#include "emu.h"

#include "bus/isa/isa_cards.h"
#include "bus/rs232/hlemouse.h"
#include "bus/rs232/null_modem.h"
#include "bus/rs232/rs232.h"
#include "bus/rs232/sun_kbd.h"
#include "bus/rs232/terminal.h"
#include "cpu/i386/i386.h"
//#include "machine/w83977tf.h"
#include "machine/8042kbdc.h"
#include "machine/ds128x.h"
#include "machine/i82371sb.h"
#include "machine/i82439tx.h"
#include "machine/pci-ide.h"
#include "machine/pci.h"
#include "video/atirage.h"

// Specific to taitowlf
#include "machine/bankdev.h"
#include "machine/eepromser.h"
#include "video/voodoo_pci.h"
#include "taitoio.h"


/***********************
 *
 * Taito extra ISA board
 *
 **********************/

class isa16_taito_rom_disk : public device_t, public device_isa16_card_interface
{
public:
	// construction/destruction
	isa16_taito_rom_disk(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_program_rom_tag(T &&tag) { m_program_rom.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_data_rom_tag(T &&tag) { m_data_rom.set_tag(std::forward<T>(tag)); }

	// TODO: confirm routing
	DECLARE_INPUT_CHANGED_MEMBER(coin_irq) { m_isa->irq7_w(ASSERT_LINE); };

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_memory_region m_program_rom;
	required_memory_region m_data_rom;
	required_device<address_map_bank_device> m_bankdev;
	required_device<tc0510nio_device> m_tc0510nio;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_ioport m_eepromin;
	required_ioport m_eepromout;

	void io_map(address_map &map) ATTR_COLD;

	u8 m_program_bank = 0;
	u8 m_program_select = 0;

	u8 read_bank(offs_t offset);
	void write_bank(offs_t offset, u8 data);

	void remap(int space_id, offs_t start, offs_t end) override;

	void bankdev_map(address_map &map) ATTR_COLD;

	void nio3_w(u8 data);
	void nio4_w(u8 data);
};

DEFINE_DEVICE_TYPE(ISA16_TAITO_ROM_DISK, isa16_taito_rom_disk, "isa16_taito_rom_disk", "ISA16 Taito Wolf System ROM DISK")

isa16_taito_rom_disk::isa16_taito_rom_disk(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ISA16_TAITO_ROM_DISK, tag, owner, clock)
	, device_isa16_card_interface(mconfig, *this)
	, m_program_rom(*this, finder_base::DUMMY_TAG)
	, m_data_rom(*this, finder_base::DUMMY_TAG)
	, m_bankdev(*this, "bankdev")
	, m_tc0510nio(*this, "tc0510nio")
	, m_eeprom(*this, "eeprom")
	, m_eepromin(*this, "EEPROMIN")
	, m_eepromout(*this, "EEPROMOUT")
{
}

void isa16_taito_rom_disk::device_add_mconfig(machine_config &config)
{
	ADDRESS_MAP_BANK(config, m_bankdev).set_map(&isa16_taito_rom_disk::bankdev_map).set_options(ENDIANNESS_LITTLE, 8, 16 + 14, 0x4000);

	TC0510NIO(config, m_tc0510nio, 0);
	m_tc0510nio->read_0_callback().set_ioport("IN0");
	m_tc0510nio->read_1_callback().set_ioport("IN1");
	m_tc0510nio->read_2_callback().set_ioport("IN2");
	m_tc0510nio->read_3_callback().set_ioport("IN3");
	m_tc0510nio->write_3_callback().set(FUNC(isa16_taito_rom_disk::nio3_w));
	m_tc0510nio->write_4_callback().set(FUNC(isa16_taito_rom_disk::nio4_w));
	m_tc0510nio->read_7_callback().set_ioport("IN7");

	// TODO: verify erase/write times
	EEPROM_93C66_16BIT(config, m_eeprom)
		.erase_time(attotime::from_usec(250))
		.write_time(attotime::from_usec(250));


	// TODO: sound CPU et al.
}

void isa16_taito_rom_disk::io_map(address_map &map)
{
	map.unmap_value_high();

	map(0x080, 0x081).rw(FUNC(isa16_taito_rom_disk::read_bank), FUNC(isa16_taito_rom_disk::write_bank));
	// writes watchdog to port 1, in 8-bit fashion
	map(0x200, 0x207).lrw8(
		NAME([this] (offs_t offset) { m_isa->irq7_w(CLEAR_LINE); return m_tc0510nio->read(offset ^ 1); }),
		NAME([this] (offs_t offset, u8 data) { m_tc0510nio->write(offset ^ 1, data); })
	);

	//map(0x400, 0x4ff) sound CPU shared RAM?

	map(0x600, 0x600).lw8(
		NAME([this] (offs_t offset, u8 data) {
			// 0x30 before showing 3dfx logo
			// 0x3d in-game
			// 0xf3 in service mode, 0xff when accessing sound volume/panning settings
			logerror("Write %02x to $600\n", data);
		})
	);
	map(0x601, 0x601).lrw8(
		NAME([this] (offs_t offset) { return m_eepromin->read(); }),
		NAME([this] (offs_t offset, u8 data) { m_eepromout->write(data, 0xff); })
	);

	//map(0x602, 0x603) to/from sound CPU
}

void isa16_taito_rom_disk::bankdev_map(address_map &map)
{
	map.unmap_value_high();
	// TODO: EMM386 and flush interactions only for writes?
	map(0x0000'0000, 0x003f'ffff).lr8(
		NAME([this] (offs_t offset) { return m_program_rom->base()[offset]; })
	);
	// TODO: unconfirmed upper bounds
	map(0x0080'0000, 0x047f'ffff).lr8(
		NAME([this] (offs_t offset) { return m_data_rom->base()[offset]; })
	);
}

void isa16_taito_rom_disk::device_start()
{
	set_isa_device();
}

void isa16_taito_rom_disk::device_reset()
{
}

void isa16_taito_rom_disk::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_PROGRAM)
	{
		// emm386 /X=CB00-D400
		m_isa->install_memory(0xcb000, 0xcbfff, *this, &isa16_taito_rom_disk::io_map);
		//m_isa->install_memory(0xcb080, 0xcb081, read8sm_delegate(*this, FUNC(isa16_taito_rom_disk::read_bank)), write8sm_delegate(*this, FUNC(isa16_taito_rom_disk::write_bank)));

		m_isa->install_memory(0xd0000, 0xd3fff, *m_bankdev, &address_map_bank_device::amap8);
	}
}

u8 isa16_taito_rom_disk::read_bank(offs_t offset)
{
	logerror("isa16_taito_rom_disk: unconfirmed read bank\n");
	if (!offset)
		return m_program_bank;
	else
		return m_program_select;
}

void isa16_taito_rom_disk::write_bank(offs_t offset, u8 data)
{
	if (!offset)
		m_program_bank = data;
	else
		m_program_select = data;

	m_bankdev->set_bank(m_program_select << 8 | m_program_bank);
}

// may be unconnected
void isa16_taito_rom_disk::nio3_w(u8 data)
{
	logerror("NIO3 %02x state\n", data);
}

// TODO: reacts to each coin trigger
// Game has individual coin counters, this rather looks an i2c/EEPROM serial protocol at bits 3-0?
void isa16_taito_rom_disk::nio4_w(u8 data)
{
	logerror("NIO4 %02x state\n", data);
}

static INPUT_PORTS_START(pf2012)
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 ) // marked as セレクト (Select) in test mode
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, isa16_taito_rom_disk, coin_irq, 0)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 ) // as above
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, isa16_taito_rom_disk, coin_irq, 0)

	PORT_START("IN2")
	PORT_DIPNAME( 0x0001, 0x0001, "IN2" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	// service mode doesn't explicitly tell, but goes service sw error if left on during boot
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, isa16_taito_rom_disk, coin_irq, 0)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_TILT )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("IN3")
	PORT_DIPNAME( 0x0001, 0x0001, "IN3" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)

	PORT_START("EEPROMIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, di_write)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, clk_write)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, cs_write)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

ioport_constructor isa16_taito_rom_disk::device_input_ports() const
{
	return INPUT_PORTS_NAME(pf2012);
}


/***********************
 *
 * Motherboard resources
 *
 **********************/

class isa16_p5txla_mb : public device_t, public device_isa16_card_interface
{
public:
	// construction/destruction
	isa16_p5txla_mb(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	required_device<ds12885_device> m_rtc;
	required_device<kbdc8042_device> m_kbdc;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void remap(int space_id, offs_t start, offs_t end) override;

	void device_map(address_map &map) ATTR_COLD;
};

DEFINE_DEVICE_TYPE(ISA16_P5TXLA_MB, isa16_p5txla_mb, "isa16_p5txla_mb", "ISA16 P5TX-LA Virtual MB resources")

isa16_p5txla_mb::isa16_p5txla_mb(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ISA16_P5TXLA_MB, tag, owner, clock)
	, device_isa16_card_interface(mconfig, *this)
	, m_rtc(*this, "rtc")
	, m_kbdc(*this, "kbdc")
{
}

void isa16_p5txla_mb::device_add_mconfig(machine_config &config)
{
	// TODO: verify keyboard / RTC types, latter lies inside PIIX4?
	// need at least a DS12885 otherwise EMM386 will complain to not have enough memory
	DS12885(config, m_rtc, 32.768_kHz_XTAL);
	//m_rtc->irq().set(m_pic8259_2, FUNC(pic8259_device::ir0_w));
	m_rtc->set_century_index(0x32);

	KBDC8042(config, m_kbdc, 0);
	m_kbdc->set_keyboard_type(kbdc8042_device::KBDC8042_STANDARD);
	m_kbdc->system_reset_callback().set_inputline(":maincpu", INPUT_LINE_RESET);
	m_kbdc->gate_a20_callback().set_inputline(":maincpu", INPUT_LINE_A20);
	m_kbdc->input_buffer_full_callback().set(":pci:07.0", FUNC(i82371sb_isa_device::pc_irq1_w));
}


void isa16_p5txla_mb::device_start()
{
	set_isa_device();
}

void isa16_p5txla_mb::device_reset()
{

}

void isa16_p5txla_mb::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_IO)
		m_isa->install_device(0x60, 0x7f, *this, &isa16_p5txla_mb::device_map);
}

void isa16_p5txla_mb::device_map(address_map &map)
{
	map(0x00, 0x0f).rw(m_kbdc, FUNC(kbdc8042_device::data_r), FUNC(kbdc8042_device::data_w));
	map(0x10, 0x1f).w(m_rtc, FUNC(mc146818_device::address_w)).umask32(0x00ff00ff);
	map(0x10, 0x1f).rw(m_rtc, FUNC(mc146818_device::data_r), FUNC(mc146818_device::data_w)).umask32(0xff00ff00);
}

namespace {

#define PCI_VIDEO_ID "pci:12.0"

class p5txla_state : public driver_device
{
public:
	p5txla_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void p5txla(machine_config &config);

protected:
	required_device<pentium_mmx_device> m_maincpu;

private:
	void p5txla_io(address_map &map) ATTR_COLD;
	void p5txla_map(address_map &map) ATTR_COLD;

//  static void winbond_superio_config(device_t *device);
};

class taitowlf_state : public p5txla_state
{
public:
	taitowlf_state(const machine_config &mconfig, device_type type, const char *tag)
		: p5txla_state(mconfig, type, tag)
		, m_voodoo(*this, PCI_VIDEO_ID)
		, m_screen(*this, "screen")
	{ }

	void taitowlf(machine_config &config);

protected:
	required_device<voodoo_1_pci_device> m_voodoo;
	required_device<screen_device> m_screen;

private:
	static void romdisk_config(device_t *device);

	u32 screen_update( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect );
};

u32 taitowlf_state::screen_update( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect )
{
// TODO: debug code for bootscreen, ISA card sub-device
// Also fixed area, not worth decoding more than once (bitmap_device?)
#if 0
	static bool enable_switch = 1;
	// bits 0-2 TAITOWOLF splash screen, bits 4-6 cross hatch
	static u8 base_pen = 0;
	const u8 *bootscreen_rom = memregion("bootscreen")->base();

	if (machine().input().code_pressed_once(KEYCODE_Z))
		enable_switch ^= 1;

	if (machine().input().code_pressed_once(KEYCODE_X))
		base_pen ^= 4;

	if (enable_switch)
	{
		for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
		{
			for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
			{
				const u8 pen = bootscreen_rom[y * 512 + x];
				// TODO: palette color routing (game barely uses red and white in both screens)
				const u32 to_rgb = rgb_t(
					pal1bit(BIT(pen, 0 + base_pen)),
					pal1bit(BIT(pen, 1 + base_pen)),
					pal1bit(BIT(pen, 2 + base_pen))
				);
				bitmap.pix(y, x) = to_rgb;
			}
		}

		return 0;
	}
#endif

	return m_voodoo->screen_update(screen, bitmap, cliprect);
}

void p5txla_state::p5txla_map(address_map &map)
{
	map.unmap_value_high();
}

void p5txla_state::p5txla_io(address_map &map)
{
	map.unmap_value_high();
}

/*****************************************************************************/

static void isa_internal_devices(device_slot_interface &device)
{
	// TODO: w83877tf
	// It actually don't seem to access any kind of Super I/O, wtf
//  device.option_add("w83977tf", W83977TF);
	device.option_add_internal("taito_romdisk", ISA16_TAITO_ROM_DISK);
	device.option_add_internal("p5txla_mb", ISA16_P5TXLA_MB);
}

#if 0
void p5txla_state::winbond_superio_config(device_t *device)
{
	w83977tf_device &fdc = *downcast<w83977tf_device *>(device);
//  fdc.set_sysopt_pin(1);
	fdc.gp20_reset().set_inputline(":maincpu", INPUT_LINE_RESET);
	fdc.gp25_gatea20().set_inputline(":maincpu", INPUT_LINE_A20);
	fdc.irq1().set(":pci:07.0", FUNC(i82371sb_isa_device::pc_irq1_w));
	fdc.irq8().set(":pci:07.0", FUNC(i82371sb_isa_device::pc_irq8n_w));
//  fdc.txd1().set(":serport0", FUNC(rs232_port_device::write_txd));
//  fdc.ndtr1().set(":serport0", FUNC(rs232_port_device::write_dtr));
//  fdc.nrts1().set(":serport0", FUNC(rs232_port_device::write_rts));
//  fdc.txd2().set(":serport1", FUNC(rs232_port_device::write_txd));
//  fdc.ndtr2().set(":serport1", FUNC(rs232_port_device::write_dtr));
//  fdc.nrts2().set(":serport1", FUNC(rs232_port_device::write_rts));
}
#endif

// TODO: PCI address mapping is unconfirmed
void p5txla_state::p5txla(machine_config &config)
{
	// P55C 133, 150, 166, 200, 233 MHz desktop options
	// 266, 300 MHz should be mobile only
	PENTIUM_MMX(config, m_maincpu, 133'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &p5txla_state::p5txla_map);
	m_maincpu->set_addrmap(AS_IO, &p5txla_state::p5txla_io);
	m_maincpu->set_irq_acknowledge_callback("pci:07.0:pic8259_master", FUNC(pic8259_device::inta_cb));
//  m_maincpu->smiact().set("pci:00.0", FUNC(i82439tx_host_device::smi_act_w));

	// FSB 66 MHz
	PCI_ROOT(config, "pci", 0);
	// 64MB for Taito Wolf HW, to be checked for base p5txla
	I82439TX(config, "pci:00.0", 0, m_maincpu, 64*1024*1024);

	// TODO: 82371AB
	i82371sb_isa_device &isa(I82371SB_ISA(config, "pci:07.0", 0, m_maincpu));
	isa.boot_state_hook().set([](u8 data) { /* printf("%02x\n", data); */ });
	isa.smi().set_inputline(m_maincpu, INPUT_LINE_SMI);

	i82371sb_ide_device &ide(I82371SB_IDE(config, "pci:07.1", 0, m_maincpu));
	ide.irq_pri().set("pci:07.0", FUNC(i82371sb_isa_device::pc_irq14_w));
	ide.irq_sec().set("pci:07.0", FUNC(i82371sb_isa_device::pc_mirq0_w));

//  ISA16_SLOT(config, "board4", 0, "pci:07.0:isabus", isa_internal_devices, "w83977tf", true).set_option_machine_config("w83977tf", winbond_superio_config);
	ISA16_SLOT(config, "board2", 0, "pci:07.0:isabus", isa_internal_devices, "p5txla_mb", true);
	ISA16_SLOT(config, "isa1", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa2", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa3", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa4", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa5", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);

#if 0
	rs232_port_device& serport0(RS232_PORT(config, "serport0", isa_com, nullptr)); // "microsoft_mouse"));
	serport0.rxd_handler().set("board4:w83977tf", FUNC(fdc37c93x_device::rxd1_w));
	serport0.dcd_handler().set("board4:w83977tf", FUNC(fdc37c93x_device::ndcd1_w));
	serport0.dsr_handler().set("board4:w83977tf", FUNC(fdc37c93x_device::ndsr1_w));
	serport0.ri_handler().set("board4:w83977tf", FUNC(fdc37c93x_device::nri1_w));
	serport0.cts_handler().set("board4:w83977tf", FUNC(fdc37c93x_device::ncts1_w));

	rs232_port_device &serport1(RS232_PORT(config, "serport1", isa_com, nullptr));
	serport1.rxd_handler().set("board4:w83977tf", FUNC(fdc37c93x_device::rxd2_w));
	serport1.dcd_handler().set("board4:w83977tf", FUNC(fdc37c93x_device::ndcd2_w));
	serport1.dsr_handler().set("board4:w83977tf", FUNC(fdc37c93x_device::ndsr2_w));
	serport1.ri_handler().set("board4:w83977tf", FUNC(fdc37c93x_device::nri2_w));
	serport1.cts_handler().set("board4:w83977tf", FUNC(fdc37c93x_device::ncts2_w));
#endif

	// on-board
	ATI_RAGEIIDVD(config, PCI_VIDEO_ID, 0);
}

void taitowlf_state::romdisk_config(device_t *device)
{
	isa16_taito_rom_disk &romdisk = *downcast<isa16_taito_rom_disk *>(device);
	romdisk.set_program_rom_tag("program_rom");
	romdisk.set_data_rom_tag("data_rom");
}

void taitowlf_state::taitowlf(machine_config &config)
{
	p5txla_state::p5txla(config);

	m_maincpu->set_clock(200'000'000);

	ISA16_SLOT(config, "board1", 0, "pci:07.0:isabus", isa_internal_devices, "taito_romdisk", true).set_option_machine_config("taito_romdisk", romdisk_config);
	// TODO: remove keyboard slot option

	VOODOO_1_PCI(config.replace(), m_voodoo, 0, m_maincpu, m_screen);
	// TODO: unverified parameters
	m_voodoo->set_fbmem(2);
	m_voodoo->set_tmumem(4, 0);
	m_voodoo->set_status_cycles(1000);

	// TODO: displays bootscreen ROM contents (512x240 8bpp) while the board is in vga_pass off state
	// This is provided by one of the CPLDs that is on the Taito PCB stack, CRTC values needs to be verified in this state
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(57);
	m_screen->set_size(800, 262);
	m_screen->set_visarea(0, 512 - 1, 0, 240 - 1);
	m_screen->set_screen_update(FUNC(taitowlf_state::screen_update));
}

/*****************************************************************************/

ROM_START(p5txla)
	ROM_REGION32_LE(0x40000, "pci:07.0", 0)
	ROM_LOAD("p5tx-la.bin", 0x00000, 0x40000, CRC(072e6d51) SHA1(70414349b37e478fc28ecbaba47ad1033ae583b7))
ROM_END

ROM_START(pf2012)
	ROM_REGION32_LE(0x40000, "pci:07.0", ROMREGION_ERASEFF)
	// TAITO Ver1.0 1998/5/7
	ROM_LOAD("p5tx-la_861.u16", 0x20000, 0x20000, CRC(a4d4a0fc) SHA1(af3a49a1bee416b58a61af28473f3dac0a4160c8))

	ROM_REGION16_LE(0x400000, "board1:program_rom", 0) // Program ROM (FAT12)
	ROM_LOAD("u1.bin", 0x000000, 0x200000, CRC(8f4c09cb) SHA1(0969a92fec819868881683c580f9e01cbedf4ad2))
	ROM_LOAD("u2.bin", 0x200000, 0x200000, CRC(59881781) SHA1(85ff074ab2a922eac37cf96f0bf153a2dac55aa4))

	ROM_REGION16_LE(0x4000000, "board1:data_rom", 0) // Data ROM (FAT12)
	ROM_LOAD("e59-01.u20", 0x0000000, 0x800000, CRC(60f2ce4a) SHA1(322dd62022527997ecc655347fdf75a092aefa8a) )
	ROM_LOAD("e59-02.u23", 0x0800000, 0x800000, CRC(626df682) SHA1(35bb4f91201734ce7ccdc640a75030aaca3d1151) )
	ROM_LOAD("e59-03.u26", 0x1000000, 0x800000, CRC(74e4efde) SHA1(630235c2e4a11f615b5f3b8c93e1e645da09eefe) )
	ROM_LOAD("e59-04.u21", 0x1800000, 0x800000, CRC(c900e8df) SHA1(93c06b8f5082e33f0dcc41f1be6a79283de16c40) )
	ROM_LOAD("e59-05.u24", 0x2000000, 0x800000, CRC(85b0954c) SHA1(1b533d5888d56d1510c79f790e4fa708f77e836f) )
	ROM_LOAD("e59-06.u27", 0x2800000, 0x800000, CRC(0573a113) SHA1(ee76a71dfd31289a9a5428653a36d01d914fc5d9) )
	ROM_LOAD("e59-07.u22", 0x3000000, 0x800000, CRC(1f0ddcdc) SHA1(72ffe08f5effab093bdfe9863f8a11f80e914272) )
	ROM_LOAD("e59-08.u25", 0x3800000, 0x800000, CRC(8db38ffd) SHA1(4b71ea86fb774ba6a8ac45abf4191af64af007e7) )

	ROM_REGION(0x180000, "board1:taito_zoom:mn10200", 0) // MN10200 program
	ROM_LOAD("e59-12.u13", 0x000000, 0x80000, CRC(9a473a7e) SHA1(b0ec7b0ae2b33a32da98899aa79d44e8e318ceb7) )
	ROM_LOAD("e59-13.u15", 0x080000, 0x80000, CRC(77719880) SHA1(8382dd2dfb0dae60a3831ed6d3ff08539e2d94eb) )
	ROM_LOAD("e59-14.u14", 0x100000, 0x40000, CRC(d440887c) SHA1(d965871860d757bc9111e9adb2303a633c662d6b) )
	ROM_LOAD("e59-15.u16", 0x140000, 0x40000, CRC(eae8e523) SHA1(8a054d3ded7248a7906c4f0bec755ddce53e2023) )

	ROM_REGION(0x1400000, "board1:taito_zoom:zsg2", 0) // ZOOM sample data
	ROM_LOAD("e59-09.u29", 0x0000000, 0x800000, CRC(d0da5c50) SHA1(56fb3c38f35244720d32a44fed28e6b58c7851f7) )
	ROM_LOAD("e59-10.u32", 0x0800000, 0x800000, CRC(4c0e0a5c) SHA1(6454befa3a1dd532eb2a760129dcd7e611508730) )
	ROM_LOAD("e59-11.u33", 0x1000000, 0x400000, CRC(c90a896d) SHA1(2b62992f20e4ca9634e7953fe2c553906de44f04) )

	ROM_REGION(0x20000, "bootscreen", 0) // bootscreen
	ROM_LOAD("e58-04.u71", 0x000000, 0x20000, CRC(500e6113) SHA1(93226706517c02e336f96bdf9443785158e7becf) )
ROM_END

} // Anonymous namespace


/*****************************************************************************/

COMP(1997, p5txla, 0,   0, p5txla, 0, p5txla_state,   empty_init, "ECS",    "P5TX-LA (i430TX)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)

GAME(1998, pf2012, 0,   taitowlf, 0,  taitowlf_state, empty_init, ROT0, "Taito",  "Psychic Force 2012 (Ver 2.04J)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND) // 1998/05/07 18:30:00
