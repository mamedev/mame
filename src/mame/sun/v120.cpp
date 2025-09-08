#include "emu.h"

#include "cpu/h8/h83437.h"
#include "bus/rs232/rs232.h"
#include "machine/input_merger.h"
#include "machine/i2cmem.h"

#include "speaker.h"

namespace {

class v120_state : public driver_device
{
public:
	v120_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_flashv(0x11),
		m_lom(*this, "lom"),
		m_ttya(*this, "ttya"),
		m_lom_eeprom(*this, "lom_eeprom")
	{ }

	void v120(machine_config &config);

private:
	u8 m_flashv;

	required_device<h83437_device> m_lom;
	required_device<rs232_port_device> m_ttya;
	required_device<i2cmem_device> m_lom_eeprom;

	void mcu_map(address_map &map);
	auto sda_read();

	void lom_set_i2c(int state);
};

void v120_state::mcu_map(address_map &map)
{
	map(0x0000, 0xf77f).rom().region("lom", 0);

	// Kludge! Needed sp that the newly booted firmware doesn't think it just
	// upgraded. Probably needs to be removed altogether with a real ROM dump
	// as opposed to one from firmware upgrade package.
	map(0xf77e, 0xf77e).lrw8(NAME([this]() { return m_flashv; }), NAME([this](u8 data) { m_flashv = data; }));
}

static DEVICE_INPUT_DEFAULTS_START(terminal)
	DEVICE_INPUT_DEFAULTS("RS232_RXBAUD", 0xff, RS232_BAUD_9600)
	DEVICE_INPUT_DEFAULTS("RS232_TXBAUD", 0xff, RS232_BAUD_9600)
	DEVICE_INPUT_DEFAULTS("RS232_DATABITS", 0xff, RS232_DATABITS_8)
	DEVICE_INPUT_DEFAULTS("RS232_PARITY", 0xff, RS232_PARITY_NONE)
	DEVICE_INPUT_DEFAULTS("RS232_STOPBITS", 0xff, RS232_STOPBITS_1)
DEVICE_INPUT_DEFAULTS_END

auto v120_state::sda_read()
{
	u8 lom_eeprom_sda = m_lom_eeprom->read_sda();
	logerror ("sda_read: eeprom=%d\n", lom_eeprom_sda);

	// SDA lines from other devices will be mixed in here
	return lom_eeprom_sda;
}

void v120_state::v120(machine_config &config)
{
	// Lights-Out Management controller.
	H83437(config, m_lom, XTAL(14'318'181));
	m_lom->set_addrmap(AS_PROGRAM, &v120_state::mcu_map);
	m_lom->write_sci_tx<1>().set(m_ttya, FUNC(rs232_port_device::write_txd));

	RS232_PORT(config, m_ttya, default_rs232_devices, "terminal");
	m_ttya->rxd_handler().set(m_lom, FUNC(h83437_device::sci_rx_w<1>));
	m_ttya->set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));

	// Debug chatter
	m_lom->scl_cb().set([this](int state) { logerror("scl_write %d\n", state); });
	m_lom->sda_cb().set([this](int state) { logerror("sda_write %d\n", state); });

	// Shared open collector data line
	m_lom->sda_out_cb().set(FUNC(v120_state::sda_read));

	// Lights-Out Management controller configuration rom.
	I2C_24C64(config, m_lom_eeprom);
	m_lom->scl_cb().append(m_lom_eeprom, FUNC(i2cmem_device::write_scl));
	m_lom->sda_cb().append(m_lom_eeprom, FUNC(i2cmem_device::write_sda));

	// TODO:
	// - the actual computer:
	// - main processor & its peripherals
}

/*
 * The ROMs can be dumped from the running system via the LOM shell.
 * Some undocumented commands have been identified that can help
 * with that. These are available:
 *
 * General/debugging:
 *
 * set extra-cmds <val> - Enables all commands documented below.
 *            <val> is "on" or "off". "God mode".
 * set gdw <data>   - Sets debugging bitmask. Enables some
 *            extra debug chatter, apparently dealing
 *            with the terminal input buffer.
 *
 * H8 memory address space access:
 *
 * rb <addr>        - Read (8-bit) byte from memory
 * rw <addr>        - Read (16-bit) word from memory
 * rl <addr>        - Read (32-bit) long from memory
 * wb <addr> <data> - Write (8-bit) byte to memory
 * ww <addr> <data> - Write (16-bit) word to memory
 * wl <addr> <data> - Write (32-bit) long to memory
 *
 * Configuration EEPROM access:
 *
 * reb <addr>       - Read (8-bit) byte from eeprom
 * rew <addr>       - Read (16-bit) word from eeprom
 * rel <addr>       - Read (32-bit) long from eeprom
 * web <addr> <data>    - Write (8-bit) byte to eeprom
 * wew <addr> <data>    - Write (16-bit) word to eeprom
 * wel <addr> <data>    - Write (32-bit) long to eeprom
 * eepromreset      - No idea, doesn't seem to do anything
 *
 * <addr> and <data> plain hex numbers (no 0x).
 *
 * First the memory commands need to be enabled with:
 *
 * lom> set extra-cmds on
 * (Congratulations, you just voided your machine's warranty)
 *
 * The "lomlite2-v3.14.bin" can be obtained from a firmware update
 * package or dumped from main LOM memory addresses 0000 to f780
 *
 * lom> rw 0000
 * lom> rw 0002
 * ...
 * lom> rw f77e
 *
 * The configuation EEPROM describes which fans, supplies, etc. are
 * available and stores the console buffer. It's dumped analogously:
 *
 * lom> rew 0000
 * ...
 * lom> rew 1ffe
 */

ROM_START(v120)
	ROM_REGION16_BE(0x0f780, "lom", 0)
	ROM_LOAD( "lomlite2-v3.14.bin",  0x00000, 0x0f780, CRC(0c6e6eba) SHA1(a3cd00d875cbcdb58060415b0a3ef56df5078631) )

	ROM_REGION( 0x2000, "lom_eeprom", 0 )
	ROM_LOAD( "lomlite2-eeprom.bin", 0x0000, 0x2000, NO_DUMP)
ROM_END

} // anonymous namespace

//    YEAR, NAME, PARENT, COMPAT, MACHINE, INPUT, CLASS,      INIT,       COMPANY,            FULLNAME,   FLAGS
COMP( 1996, v120, 0,      0,      v120,    0,     v120_state, empty_init, "Sun Microsystems", "v120",     MACHINE_IS_SKELETON )
