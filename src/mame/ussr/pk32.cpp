// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/**********************************************************************
    Elektronika PK-32
**********************************************************************/

#include "emu.h"

#include "cpu/mpk1839/kl1839vm1.h"
#include "machine/ram.h"
#include "machine/terminal.h"


#define LOG_IO    (1U << 1)
#define LOG_MEM   (1U << 2)
#define LOG_TERM  (1U << 3)

#define VERBOSE ( LOG_GENERAL /*| LOG_IO | LOG_MEM | LOG_TERM*/ )
#include "logmacro.h"

#define LOGIO(...)   LOGMASKED(LOG_IO,   __VA_ARGS__)
#define LOGMEM(...)  LOGMASKED(LOG_MEM,  __VA_ARGS__)
#define LOGTERM(...) LOGMASKED(LOG_TERM, __VA_ARGS__)


namespace {

class pk32_state : public driver_device
{
public:
	pk32_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, RAM_TAG)
		, m_terminal(*this, "terminal")
		, m_io_rnp(*this, "RNP")
	{}

	void pk32(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void pk32_map_microcode(address_map &map) ATTR_COLD;
	void pk32_map_sysram(address_map &map) ATTR_COLD;
	void pk32_map_ram(address_map &map) ATTR_COLD;
	void pk32_map_io(address_map &map) ATTR_COLD;

	required_device<kl1839vm1_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<generic_terminal_device> m_terminal;

	required_ioport m_io_rnp; // Initial Start Register

	u32 term_rx_db_r();
	void term_tx_db_w(u32 data);
	u32 term_rx_cs_r();
	void term_rx_cs_w(u32 data);
	u32 term_tx_cs_r();
	void term_tx_cs_w(u32 data);
	void kbd_put(u8 data);

	u8 m_term_data = 0;
	u16 m_term_status = 0;
};


u32 pk32_state::term_rx_db_r()
{
	LOGTERM("RX DB -> %X\n", m_term_data);
	m_term_status = 0x00;
	return m_term_data;
}

void pk32_state::term_tx_db_w(u32 data)
{
	LOGTERM("RX DB <- %X\n", data);
	m_terminal->write(data);
}

u32 pk32_state::term_rx_cs_r()
{
	//LOGTERM("RX CS -> %X\n", m_term_status);
	return m_term_status;
}

void pk32_state::term_rx_cs_w(u32 data)
{
	LOGTERM("RX CS <- %X\n", data);
	m_term_status = data;
}

u32 pk32_state::term_tx_cs_r()
{
	LOGTERM("TX CS -> %X\n", 0xff);
	// always ready
	return 0xff;
}

void pk32_state::term_tx_cs_w(u32 data)
{
	LOGTERM("TX CS <- %X\n", data);
}

void pk32_state::pk32_map_microcode(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("microcode", 0);
}

void pk32_state::pk32_map_sysram(address_map &map)
{
	map(0x000000, 0x001fff).ram();
	map(0x800000, 0x800000).lr8(NAME([this](offs_t offset) { return m_io_rnp->read() | ((m_ram->size() >> 20) - 1); }));
}

void pk32_state::pk32_map_ram(address_map &map)
{
	map(0x000000, 0xffffff).ram();
}

void pk32_state::pk32_map_io(address_map &map)
{
	map.unmap_value_high();
	map(0x00, 0x3f).unmaprw();

	map(0x20, 0x20).r(FUNC(pk32_state::term_rx_cs_r)).w(FUNC(pk32_state::term_rx_cs_w));
	map(0x21, 0x21).r(FUNC(pk32_state::term_rx_db_r));
	map(0x22, 0x22).r(FUNC(pk32_state::term_tx_cs_r)).w(FUNC(pk32_state::term_tx_cs_w));
	map(0x23, 0x23).w(FUNC(pk32_state::term_tx_db_w));
}

/* Input ports */
static INPUT_PORTS_START( pk32 )
	PORT_START("RNP")
	PORT_CONFNAME( 0xf0, 0x10, "Boot" )
	PORT_CONFSETTING(0x00, "Terminal w/ test")
	PORT_CONFSETTING(0x10, "Terminal w/o test")
	PORT_CONFSETTING(0x20, "OS from MSCP w/ test")
	PORT_CONFSETTING(0x30, "OS from MSCP w/o test")
	PORT_CONFSETTING(0x40, "OS from user's device w/ test")
	PORT_CONFSETTING(0x50, "User's program from 0")
	PORT_CONFSETTING(0x60, "Resident devices")
	PORT_CONFSETTING(0x80, "Accelerator ON")
INPUT_PORTS_END

void pk32_state::kbd_put(u8 data)
{
	m_term_data = data;
	m_term_status = 0xff;
}


void pk32_state::machine_start()
{
	save_item(NAME(m_term_data));
	save_item(NAME(m_term_status));
}

void pk32_state::machine_reset()
{
}

void pk32_state::pk32(machine_config &config)
{
	/* basic machine hardware */
	KL1839VM1(config, m_maincpu, XTAL(10'000'000));
	m_maincpu->set_addrmap(AS_OPCODES, &pk32_state::pk32_map_microcode);
	m_maincpu->set_addrmap(AS_DATA, &pk32_state::pk32_map_sysram);
	m_maincpu->set_addrmap(AS_PROGRAM, &pk32_state::pk32_map_ram);
	m_maincpu->set_addrmap(AS_IO, &pk32_state::pk32_map_io);

	RAM(config, m_ram).set_default_size("16M").set_extra_options("8M,4M");

	/* video hardware */
	GENERIC_TERMINAL(config, m_terminal, 0);
	m_terminal->set_keyboard_callback(FUNC(pk32_state::kbd_put));
}

ROM_START(pk32)
	ROM_REGION32_BE(0x10000, "microcode", ROMREGION_ERASE00)
	ROM_DEFAULT_BIOS("v1")

	ROM_SYSTEM_BIOS(0, "v1", "1839src)")
	ROMX_LOAD("fw-1839-src.bin", 0x0000, 0x10000, CRC(78c4d298) SHA1(eb3828718991968b4121e6819ae4c6859a8a3a5a), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "v2", "1839isa")
	ROMX_LOAD("fw-1839-isa.bin", 0x0000, 0x10000, CRC(180930a7) SHA1(dbcc7d665d28011b9c2beba3cc0e4073f34d7fc6), ROM_BIOS(1))

	ROM_SYSTEM_BIOS(2, "v3", "1839pe1")
	ROMX_LOAD("fw-1839pe1-006.bin", 0x0000, 0x10000, CRC(e6bb7639) SHA1(759993afc9b61d9cf83acf3e361dcec571beccf1), ROM_BIOS(2))
ROM_END


} // Anonymous namespace

/*    YEAR  NAME   PARENT COMPAT MACHINE INPUT CLASS       INIT        COMPANY        FULLNAME FLAGS */
COMP( 1991, pk32,  0,     0,     pk32,   pk32, pk32_state, empty_init, "Elektronika", "PK-32", MACHINE_NOT_WORKING)
