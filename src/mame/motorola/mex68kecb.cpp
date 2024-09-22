// license:BSD-3-Clause
// copyright-holders:Chris Hanson
/*
 * mex68kecb.cpp - Motorola MEX68KECB
 *
 * Documentation:
 *   http://www.bitsavers.org/components/motorola/68000/MEX68KECB/MEX68KECB_D2_EduCompBd_Jul82.pdf
 *
 * The Motorola MC68000 Educational Computer Board is a single-board computer with
 * a 4MHz 68000 CPU, 32KB RAM, 16KB ROM, host and terminal serial ports, a
 * parallel interface/timer, a cassette interface, and a prototyping area with
 * full access to the 68000 bus. The ROM contains TUTOR, a debug and bootstrap
 * system that was the predecessor of MACSBUG.
 *
 * Specifications:
 * - 4MHz MC68000L4 CPU
 * - MC6850 ACIA x 2
 * - MC68230 PIT
 *
 * TODO:
 * - Cassette I/O
 *
 */

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "machine/68230pit.h"
#include "machine/6850acia.h"
#include "machine/mc14411.h"


namespace {

class mex68kecb_state : public driver_device
{
public:
	static constexpr feature_type unemulated_features() { return feature::TAPE; }

	mex68kecb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_bootvect(*this, "bootvect")
		, m_sysram(*this, "ram")
		, m_maincpu(*this, "maincpu")
		, m_pit(*this, "pit")
		, m_brg(*this, "brg")
		, m_acia1(*this, "acia1")
		, m_acia2(*this, "acia2")
		, m_acia1_baud(*this, "ACIA1_BAUD")
		, m_acia2_baud(*this, "ACIA2_BAUD")
		, m_terminal(*this, "terminal")
		, m_host(*this, "host")
	{ }

	void mex68kecb(machine_config &config) ATTR_COLD;

	DECLARE_INPUT_CHANGED_MEMBER(abort_button);

protected:
	virtual void machine_reset() override ATTR_COLD;

private:
	void mem_map(address_map &map) ATTR_COLD;

	// Clocks from Baud Rate Generator
	template <uint8_t Bit> void write_acia_clock(int state);

	void bootvect_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	memory_view m_bootvect;
	required_shared_ptr<uint16_t> m_sysram; // Pointer to System RAM needed by bootvect_w and masking RAM buffer for post reset accesses

	required_device<cpu_device> m_maincpu;
	required_device<pit68230_device> m_pit;
	required_device<mc14411_device> m_brg;
	required_device<acia6850_device> m_acia1;
	required_device<acia6850_device> m_acia2;
	required_ioport m_acia1_baud;
	required_ioport m_acia2_baud;

	required_device<rs232_port_device> m_terminal;
	required_device<rs232_port_device> m_host;
};


/* Input ports */
static INPUT_PORTS_START( mex68kecb )
	PORT_START("ACIA1_BAUD")
	PORT_DIPNAME(0xff, 0x80, "Terminal Baud Rate") PORT_DIPLOCATION("J10:8,7,6,5,4,3,2,1")
	PORT_DIPSETTING(0x80, "9600")
	PORT_DIPSETTING(0x40, "4800")
	PORT_DIPSETTING(0x20, "2400")
	PORT_DIPSETTING(0x10, "1200")
	PORT_DIPSETTING(0x08,  "600")
	PORT_DIPSETTING(0x04,  "300")
	PORT_DIPSETTING(0x02,  "150")
	PORT_DIPSETTING(0x01,  "110")

	PORT_START("ACIA2_BAUD")
	PORT_DIPNAME(0xff, 0x80, "Host Baud Rate") PORT_DIPLOCATION("J9:8,7,6,5,4,3,2,1")
	PORT_DIPSETTING(0x80, "9600")
	PORT_DIPSETTING(0x40, "4800")
	PORT_DIPSETTING(0x20, "2400")
	PORT_DIPSETTING(0x10, "1200")
	PORT_DIPSETTING(0x08,  "600")
	PORT_DIPSETTING(0x04,  "300")
	PORT_DIPSETTING(0x02,  "150")
	PORT_DIPSETTING(0x01,  "110")

	PORT_START("ABORT")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Abort button") PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, mex68kecb_state, abort_button, 0)
INPUT_PORTS_END


void mex68kecb_state::mex68kecb(machine_config &config)
{
	M68000(config, m_maincpu, 8_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &mex68kecb_state::mem_map);

	// Set up BRG.

	MC14411(config, m_brg, 1.8432_MHz_XTAL);
	m_brg->out_f<1>().set(FUNC(mex68kecb_state::write_acia_clock<7>));  // 9600bps
	m_brg->out_f<3>().set(FUNC(mex68kecb_state::write_acia_clock<6>));  // 4800bps
	m_brg->out_f<5>().set(FUNC(mex68kecb_state::write_acia_clock<5>));  // 2400bps
	m_brg->out_f<7>().set(FUNC(mex68kecb_state::write_acia_clock<4>));  // 1200bps
	m_brg->out_f<8>().set(FUNC(mex68kecb_state::write_acia_clock<3>));  //  600bps
	m_brg->out_f<9>().set(FUNC(mex68kecb_state::write_acia_clock<2>));  //  300bps
	m_brg->out_f<11>().set(FUNC(mex68kecb_state::write_acia_clock<1>)); //  150bps
	m_brg->out_f<13>().set(FUNC(mex68kecb_state::write_acia_clock<0>)); //  110bps

	// Set up PIT and ACIAs.

	PIT68230(config, m_pit, 8_MHz_XTAL / 2);
	ACIA6850(config, m_acia1);
	ACIA6850(config, m_acia2);

	// Set up interrupts.

	// Nothing at IRQ1
	m_pit->timer_irq_callback().set_inputline(m_maincpu, M68K_IRQ_2);
	m_pit->port_irq_callback().set_inputline(m_maincpu, M68K_IRQ_3);
	// Optional 6800 peripherals at IRQ4
	m_acia1->irq_handler().set_inputline(m_maincpu, M68K_IRQ_5);
	m_acia2->irq_handler().set_inputline(m_maincpu, M68K_IRQ_6);
	// Abort button at IRQ7, see abort_button()

	// Set up terminal RS-232.

	RS232_PORT(config, m_terminal, default_rs232_devices, "terminal");
	m_terminal->rxd_handler().set(m_acia1, FUNC(acia6850_device::write_rxd));
	m_terminal->cts_handler().set(m_acia1, FUNC(acia6850_device::write_cts));
	m_terminal->dcd_handler().set(m_acia1, FUNC(acia6850_device::write_dcd));
	m_acia1->txd_handler().set(m_terminal, FUNC(rs232_port_device::write_txd));
	m_acia1->rts_handler().set(m_terminal, FUNC(rs232_port_device::write_rts));

	// Set up host RS-232.

	RS232_PORT(config, m_host, default_rs232_devices, nullptr);
	m_host->rxd_handler().set(m_acia2, FUNC(acia6850_device::write_rxd));
	m_host->cts_handler().set(m_acia2, FUNC(acia6850_device::write_cts));
	m_host->dcd_handler().set(m_acia2, FUNC(acia6850_device::write_dcd));
	m_acia2->txd_handler().set(m_host, FUNC(rs232_port_device::write_txd));
	m_acia2->rts_handler().set(m_host, FUNC(rs232_port_device::write_rts));
}

void mex68kecb_state::mem_map(address_map &map)
{
	map.unmap_value_high();

	map(0x000000, 0x007fff).ram().share("ram"); // 32KB RAM
	map(0x008000, 0x00bfff).rom().region("roms", 0); // 16KB ROM
	map(0x010000, 0x01003f).rw(m_pit, FUNC(pit68230_device::read), FUNC(pit68230_device::write)).umask16(0x00ff);
	map(0x010040, 0x010043).rw(m_acia1, FUNC(acia6850_device::read), FUNC(acia6850_device::write)).umask16(0xff00);
	map(0x010040, 0x010043).rw(m_acia2, FUNC(acia6850_device::read), FUNC(acia6850_device::write)).umask16(0x00ff);

	map(0x000000, 0x000007).view(m_bootvect);
	m_bootvect[0](0x000000, 0x000007).rom().region("roms", 0);              // After first write we act as RAM
	m_bootvect[0](0x000000, 0x000007).w(FUNC(mex68kecb_state::bootvect_w)); // ROM mirror just during reset
}

void mex68kecb_state::machine_reset()
{
	// Reset BRG.
	m_brg->rsa_w(CLEAR_LINE);
	m_brg->rsb_w(ASSERT_LINE);

	// Reset pointer to bootvector in ROM for bootvector view
	m_bootvect.select(0);
}

template <uint8_t Bit>
void mex68kecb_state::write_acia_clock(int state)
{
	if (BIT(m_acia1_baud->read(), Bit)) {
		m_acia1->write_txc(state);
		m_acia1->write_rxc(state);
	}

	if (BIT(m_acia2_baud->read(), Bit)) {
		m_acia2->write_txc(state);
		m_acia2->write_rxc(state);
	}
}

// Abort button handler
INPUT_CHANGED_MEMBER(mex68kecb_state::abort_button)
{
	m_maincpu->set_input_line(M68K_IRQ_7, newval ? CLEAR_LINE : ASSERT_LINE); // active low
}

// Boot vector handler, the PCB hardwires the first 8 bytes from 0x008000 to 0x0 at reset.
void mex68kecb_state::bootvect_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_sysram[offset]);
	m_bootvect.disable(); // redirect all upcoming accesses to masking RAM until reset.
}


/* ROM definition */
ROM_START( mex68kecb )
	ROM_REGION16_BE(0x4000, "roms", ROMREGION_ERASE00)
	ROM_DEFAULT_BIOS("tutor13")

	ROM_SYSTEM_BIOS(0, "tutor13", "Motorola TUTOR 1.3")
	ROMX_LOAD("tutor13u.bin", 0x000000, 0x002000, CRC(7d11a0e9) SHA1(18ec8899651e78301b406f4fe6d4141c853e9e30), ROM_SKIP(1) | ROM_BIOS(0) )
	ROMX_LOAD("tutor13l.bin", 0x000001, 0x002000, CRC(2bb3a4e2) SHA1(3dac64ec5af4f46a367959ec80677103e3822f20), ROM_SKIP(1) | ROM_BIOS(0) )
ROM_END

} // anonymous namespace


// Driver
//    YEAR  NAME       PARENT  COMPAT  MACHINE    INPUT      CLASS            INIT        COMPANY     FULLNAME                            FLAGS
COMP( 1981, mex68kecb, 0,      0,      mex68kecb, mex68kecb, mex68kecb_state, empty_init, "Motorola", "68000 Educational Computer Board", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
