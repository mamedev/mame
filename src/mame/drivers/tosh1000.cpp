// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    Toshiba T1000 portable

    80C88 CPU @ 5 MHz [OKI MSM80C88A-10GS-K (56 pin PQFP)]
    512KB RAM + 16KB video RAM
    32KB BIOS ROM [Toshiba TC54256AD]
    256KB MS-DOS 2.11 ROM [Toshiba TC534000]
    SuperIO chip (Toshiba T7885, T7885A or T7885B) = 82C84 + 82C88 + 82C59 + upd765 + 82C53 + 82C37 + 82C55
    Real Time Clock chip: TC8521P
    Keyboard controller: 80C50
    RS232C controller: TC8570F (8250 compatible)

    Other chips seen on board photo:

    DC2131P137A
    DC2130P174A
    TC5565AFL-15 x2
    TC53257F    32KB Mask ROM (chargen?)
    DC2___P13_A

    To do:
    - floppy
    - backup ram (stores config.sys)
    - HardRAM (static RAM board)
    - native keyboard (MCU dump missing)
    - font selector (system register 0x5A; DIP switches PJ20, PJ21)
    - display contrast and type (CRTC register 0x12)

    Useful links:
    - board photo: http://s8.hostingkartinok.com/uploads/images/2016/05/579e9d152bc772d9c16bc8ac611eb97f.jpg
    - manuals: http://www.minuszerodegrees.net/manuals/Toshiba/Toshiba.htm
    - http://www.seasip.info/VintagePC/t1000.html

***************************************************************************/


#include "emu.h"
#include "machine/genpc.h"
#include "machine/tosh1000_bram.h"

#include "bus/isa/isa_cards.h"
#include "bus/pc_kbd/keyboards.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "cpu/i86/i86.h"
#include "machine/bankdev.h"
#include "machine/ram.h"
#include "machine/rp5c01.h"

#include "softlist.h"


//#define LOG_GENERAL (1U <<  0) //defined in logmacro.h already
#define LOG_KEYBOARD  (1U <<  1)
#define LOG_DEBUG     (1U <<  2)

//#define VERBOSE (LOG_DEBUG)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


namespace {

#define LOGKBD(...) LOGMASKED(LOG_KEYBOARD, __VA_ARGS__)
#define LOGDBG(...) LOGMASKED(LOG_DEBUG, __VA_ARGS__)


class tosh1000_state : public driver_device
{
public:
	tosh1000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_bankdev(*this, "bankdev")
		, m_bram(*this, "bram")
		{ }

	void tosh1000(machine_config &config);

	void init_tosh1000();

protected:
	virtual void machine_reset() override;

private:
	void romdos_bank_w(uint8_t data);

	void bram_w(offs_t offset, uint8_t data);
	uint8_t bram_r(offs_t offset);

	static void cfg_fdc_35(device_t *device);
	void tosh1000_io(address_map &map);
	void tosh1000_map(address_map &map);
	void tosh1000_romdos(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<address_map_bank_device> m_bankdev;
	required_device<tosh1000_bram_device> m_bram;

	enum {
		IDLE, READ_DATA, WRITE_DATA
	};

	bool m_bram_latch = false;
	int m_bram_offset = 0;
	int m_bram_state = IDLE;
};


void tosh1000_state::init_tosh1000()
{
}

void tosh1000_state::machine_reset()
{
	m_bram_latch = false;
	m_bram_offset = 0;
	m_bram_state = IDLE;
	m_bankdev->set_bank(8);
}


void tosh1000_state::romdos_bank_w(uint8_t data)
{
	LOGDBG("ROM-DOS <- %02x (%s, accessing bank %d)\n", data, BIT(data, 7)?"enable":"disable", data&7);

	if (BIT(data, 7))
	{
		m_bankdev->set_bank(data & 7);
	}
	else
	{
		m_bankdev->set_bank(8);
	}
}

void tosh1000_state::bram_w(offs_t offset, uint8_t data)
{
	LOGDBG("BRAM %02x <- %02x\n", 0xc0 + offset, data);

	switch (offset)
	{
	case 1:
		if (m_bram_latch)
		{
			LOG("Backup RAM %02x <- %02x\n", m_bram_offset % 160, data);
			m_bram->write(m_bram_offset % 160, data);
			m_bram_offset++;
		}
		else
		{
			if (m_bram_state == WRITE_DATA)
			{
				m_bram_latch = true;
				m_bram_offset = 0;
			}
		}
		break;

	case 3:
		switch (data & 0xc0)
		{
		case 0:
			m_bram_state = IDLE;
			break;

		case 0x40:
			m_bram_state = READ_DATA;
			break;

		case 0x80:
			m_bram_state = WRITE_DATA;
			m_bram_latch = false;
			break;

		default:
			m_bram_state = IDLE;
			break;
		}
		break;
	}
}

uint8_t tosh1000_state::bram_r(offs_t offset)
{
	uint8_t data = 0;

	switch (offset)
	{
	case 2:
		if (m_bram_state == READ_DATA)
		{
			data = m_bram->read(m_bram_offset % 160);
			LOG("BRAM @ %02x == %02x\n", m_bram_offset % 160, data);
			m_bram_offset++;
		}
		break;

	case 3:
		data = 0x2c;
		switch (m_bram_state)
		{
		case IDLE:
			// bit 4 -- floppy drive disk change signal
			break;

		case READ_DATA:
			data |= 0x43; // always ready to read (and write ??)
			break;

		case WRITE_DATA:
			data |= 0x82; // always ready to write
			break;
		}
		break;
	}

	LOGDBG("BRAM %02x == %02x\n", 0xc0 + offset, data);

	return data;
}


void tosh1000_state::tosh1000_romdos(address_map &map)
{
	map(0x00000, 0x0ffff).rom().region("romdos", 0);
	map(0x10000, 0x1ffff).rom().region("romdos", 0x10000);
	map(0x20000, 0x2ffff).rom().region("romdos", 0x20000);
	map(0x30000, 0x3ffff).rom().region("romdos", 0x30000);
	map(0x40000, 0x4ffff).rom().region("romdos", 0x40000);
	map(0x50000, 0x5ffff).rom().region("romdos", 0x50000);
	map(0x60000, 0x6ffff).rom().region("romdos", 0x60000);
	map(0x70000, 0x7ffff).rom().region("romdos", 0x70000);
}

void tosh1000_state::tosh1000_map(address_map &map)
{
	map.unmap_value_high();
	map(0xa0000, 0xaffff).rw(m_bankdev, FUNC(address_map_bank_device::read8), FUNC(address_map_bank_device::write8));
	map(0xf8000, 0xfffff).rom().region("bios", 0);
}

void tosh1000_state::tosh1000_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x00ff).m("mb", FUNC(ibm5160_mb_device::map));
	map(0x00c0, 0x00c3).rw(FUNC(tosh1000_state::bram_r), FUNC(tosh1000_state::bram_w));
	map(0x00c8, 0x00c8).w(FUNC(tosh1000_state::romdos_bank_w));    // ROM-DOS page select [p. B-15]
	map(0x02c0, 0x02df).rw("rtc", FUNC(tc8521_device::read), FUNC(tc8521_device::write));
}


void tosh1000_state::cfg_fdc_35(device_t *device)
{
	dynamic_cast<device_slot_interface &>(*device->subdevice("fdc:0")).set_default_option("35dd");
	dynamic_cast<device_slot_interface &>(*device->subdevice("fdc:0")).set_fixed(true);
	dynamic_cast<device_slot_interface &>(*device->subdevice("fdc:1")).set_default_option(nullptr);
}

void tosh1000_state::tosh1000(machine_config &config)
{
	I8088(config, m_maincpu, XTAL(5'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &tosh1000_state::tosh1000_map);
	m_maincpu->set_addrmap(AS_IO, &tosh1000_state::tosh1000_io);
	m_maincpu->set_irq_acknowledge_callback("mb:pic8259", FUNC(pic8259_device::inta_cb));

	ADDRESS_MAP_BANK(config, "bankdev").set_map(&tosh1000_state::tosh1000_romdos).set_options(ENDIANNESS_LITTLE, 8, 20, 0x10000);

	ibm5160_mb_device &mb(IBM5160_MOTHERBOARD(config, "mb"));
	mb.set_cputag(m_maincpu);
	mb.int_callback().set_inputline(m_maincpu, 0);
	mb.nmi_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	mb.kbdclk_callback().set("kbd", FUNC(pc_kbdc_device::clock_write_from_mb));
	mb.kbddata_callback().set("kbd", FUNC(pc_kbdc_device::data_write_from_mb));

	TC8521(config, "rtc", XTAL(32'768));

	// FIXME: determine ISA bus clock
	ISA8_SLOT(config, "isa1", 0, "mb:isa", pc_isa8_cards, "cga", false);
	ISA8_SLOT(config, "isa2", 0, "mb:isa", pc_isa8_cards, "fdc_xt", false).set_option_machine_config("fdc_xt", cfg_fdc_35);
	ISA8_SLOT(config, "isa3", 0, "mb:isa", pc_isa8_cards, "lpt", false);
	ISA8_SLOT(config, "isa4", 0, "mb:isa", pc_isa8_cards, "com", false);
	ISA8_SLOT(config, "isa5", 0, "mb:isa", pc_isa8_cards, nullptr, false);
	ISA8_SLOT(config, "isa6", 0, "mb:isa", pc_isa8_cards, nullptr, false);

//  SOFTWARE_LIST(config, "flop_list").set_original("tosh1000");

	// TODO: uses a 80C50 instead of 8042 for KBDC
	pc_kbdc_device &kbd(PC_KBDC(config, "kbd", pc_xt_keyboards, STR_KBD_KEYTRONIC_PC3270));
	kbd.out_clock_cb().set("mb", FUNC(ibm5160_mb_device::keyboard_clock_w));
	kbd.out_data_cb().set("mb", FUNC(ibm5160_mb_device::keyboard_data_w));

	RAM(config, RAM_TAG).set_default_size("512K");

	TOSH1000_BRAM(config, m_bram, 0);
}


ROM_START( tosh1000 )
	ROM_REGION(0x8000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "v410", "V4.10")
	ROMX_LOAD( "026f.27c256.ic25", 0x0000, 0x8000, CRC(a854939f) SHA1(0ff532f295a40716f53949a2fd64d02bf76d575a), ROM_BIOS(0))

	ROM_REGION(0x80000, "romdos", 0)
	ROM_LOAD("tc534000p__b004.dos.ic26", 0x0000, 0x80000, CRC(716027f6) SHA1(563e3a7e1961d4cda216169bd1ecc66925a101aa))

	/* XXX IBM 1501981(CGA) and 1501985(MDA) Character rom */
	ROM_REGION(0x2000, "gfx1", 0)
	ROM_LOAD("5788005.u33", 0x00000, 0x2000, CRC(0bf56d70) SHA1(c2a8b10808bf51a3c123ba3eb1e9dd608231916f)) /* "AMI 8412PI // 5788005 // (C) IBM CORP. 1981 // KOREA" */
ROM_END

} // Anonymous namespace


//    YEAR  NAME      PARENT   COMPAT  MACHINE   INPUT  CLASS           INIT           COMPANY    FULLNAME         FLAGS
COMP( 1987, tosh1000, ibm5150, 0,      tosh1000, 0,     tosh1000_state, init_tosh1000, "Toshiba", "Toshiba T1000", MACHINE_IS_SKELETON )
