// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        VAX-11

        VAX-11/785
        -------
        M7459 TRS, TERMINATOR & SILO
        M7460 KA785, SBL,CUP SBI LOW BITS
        M7461 KA785, SBH,SBI
        M7462 KA785, CAM, CACHE ADDRESS MATRIX
        M7463 KA785, CDM, CACHE DATA MATRIX
        M7464 KA785, TBM, CPU TRANSLATION BUFFER
        M7465 KA785, IDP, CPU INSTRUCTION DATA
        M7466 KA785, IRC, CPU INSTRUCTION DECODE
        M7467 KA785, DPB, CPU DATA PATH B
        M7468 KA785, DEP, CPU DATA PATH E
        M7469 KA785, DDP, CPU DATA PATH D
        M7470 KA785, DCP, CPU DATA PATH C
        M7471 KA785, DAP, CPU DATA PATH A
        M7472 KA785, CEW (CONDITION CODES, EXCEP
        M7473 KA785, ICL (INTERRUPT CONTROL, LOW
        M7474 KA785, CLK (CPU CLOCK)
        M7475 KA785, JCS, JOINT CONTROL STORE
        M7476 KA785, USC, MICRO SEQUENCER CONTROL
        M7477 KA785, CIB, CPU CONSOLE INTERFACE

                                                                                                                                                 +----------FP785--------+
        1     2     3     4     5     6     7     8     9     10    11    12    13    14    15    16    17   18    19    20    21   22     23    24    25    26    27    28    29
        M7459 M7460 M7461 M7462 M7463 M7464 M7465 M7466 M7467 M7468 M7469 M7470 M7471 M7472 M7473 M7474      M7475       M7475      UNUSED M7476 M7540 M7541 M7542 M7543 M7544 M7477
        TRS   SBL   SBH   CAM   CDM   TBM   IDP   IRC   DBP   DEP   DDP   DCP   DAP   CEH   ICL   CLK        JCS         JCS               USC   FNM   FMH   FML   FAD   FCT   CIB

        VAX-11/780
        -------
        M8237 TERMINATOR SILO
        M8218 SBL LOW BIT INTERFACE
        M8219 SBH HIGH BITS INTERFACE
        M8220 CAM CPU CACHE ADAPTER MATRIX
        M8221 CDM CPU CACHE DATA MATRIX
        M8222 TBM CPU TRANSLATION BUFFER MATRIX
        M8223 IRC CPU INSTRUCT DECODE&CLOCKS
        M8224 IDP CPU INSTRUCTION DATA PATH
        M8225 DBP DATA PATH B
        M8226 DEP DATA PATH E
        M8227 DDP DATA PATH D
        M8228 DCP DATA PATH C
        M8229 DAP DATA PATH A
        M8230 CEH CPU CONDITION CODE
        M8231 ICL CPU TRAPS AND INTERRUPT CONTROL
        M8232 CLK CPU CLOCK
        M8238 WCS KU780-A 2K WCS
        M8234 PCS KA780-A PCS, PROM CONTROL STORE
        M8235 USC MICRO SEQUENCE CONTROL
        M8236 CIB CONSOLE INTERFACE

                                                                                                                                                 +----------FP780--------+
        1     2     3     4     5     6     7     8     9     10    11    12    13    14    15    16    17   18    19    20    21   22     23    24    25    26    27    28    29
        M8237 M8218 M8219 M8220 M8221 M8222 M8223 M8224 M8225 M8226 M8227 M8228 M8229 M8230 M8231 M8232      M8233       M8233      M8234  M8235 M8285 M8286 M8287 M8288 M8289 M8236
        TRS   SBL   SBH   CAM   CDM   TBM   IDP   IRC   DBP   DEP   DDP   DCP   DAP   CEH   ICL   CLK        or          or         PCS    USC   FNM   FMH   FML   FAD   FCT   CIB
                                                                                                             M8238       M8238


        02/08/2012 Skeleton driver.

****************************************************************************/

#include "emu.h"
#include "cpu/t11/t11.h"
#include "machine/terminal.h"
#include "rx01.h"


namespace {

class vax11_state : public driver_device
{
public:
	vax11_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_terminal(*this, "terminal")
	{ }

	void vax11(machine_config &config);

private:
	required_device<t11_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
	uint16_t term_r();
	uint16_t term_tx_status_r();
	uint16_t term_rx_status_r();
	void term_w(uint16_t data);
	void kbd_put(u8 data);
	uint8_t m_term_data = 0;
	uint16_t m_term_status = 0;
	void vax11_mem(address_map &map) ATTR_COLD;
};

void vax11_state::term_w(uint16_t data)
{
	m_terminal->write(data);
}

uint16_t vax11_state::term_r()
{
	m_term_status = 0x0000;
	return m_term_data;
}

uint16_t vax11_state::term_tx_status_r()
{   // always ready
	return 0xffff;
}

uint16_t vax11_state::term_rx_status_r()
{
	return m_term_status;
}

void vax11_state::vax11_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xbfff).ram();  // RAM
	map(0xc000, 0xd7ff).rom();

	map(0xfe78, 0xfe7b).rw("rx01", FUNC(rx01_device::read), FUNC(rx01_device::write));

	map(0xff70, 0xff71).r(FUNC(vax11_state::term_rx_status_r));
	map(0xff72, 0xff73).r(FUNC(vax11_state::term_r));
	map(0xff74, 0xff75).r(FUNC(vax11_state::term_tx_status_r));
	map(0xff76, 0xff77).w(FUNC(vax11_state::term_w));
}

/* Input ports */
static INPUT_PORTS_START( vax11 )
INPUT_PORTS_END

void vax11_state::kbd_put(u8 data)
{
	m_term_data = data;
	m_term_status = 0xffff;
}

void vax11_state::vax11(machine_config &config)
{
	/* basic machine hardware */
	T11(config, m_maincpu, XTAL(4'000'000)); // Need proper CPU here
	m_maincpu->set_initial_mode(0 << 13);
	m_maincpu->set_addrmap(AS_PROGRAM, &vax11_state::vax11_mem);

	/* video hardware */
	GENERIC_TERMINAL(config, m_terminal, 0);
	m_terminal->set_keyboard_callback(FUNC(vax11_state::kbd_put));

	RX01(config, "rx01", 0);
}

ROM_START( vax785 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	// M7477
	ROMX_LOAD( "23-144f1-00.e56", 0xc000, 0x0400, CRC(99c1f117) SHA1(f05b6e97bf258392656058864abc1177379194da), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(1))
	ROMX_LOAD( "23-145f1-00.e68", 0xc000, 0x0400, CRC(098b63d2) SHA1(c2742aaccdac2921e1704c835ee5cef242cd7308), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(1))
	ROMX_LOAD( "23-146f1-00.e84", 0xc001, 0x0400, CRC(0f5f5d7b) SHA1(2fe325d2a78a8ce5146317cc39c084c4967c323c), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(1))
	ROMX_LOAD( "23-147f1-00.e72", 0xc001, 0x0400, CRC(bde386f2) SHA1(fcb5a1fa505912c5f44781619c9508cd142721e3), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(1))

	ROMX_LOAD( "23-148f1-00.e58", 0xc800, 0x0400, CRC(fe4c61e3) SHA1(4641a236761692a8f45b14ed6a73f535d57c2daa), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(1))
	ROMX_LOAD( "23-149f1-00.e70", 0xc800, 0x0400, CRC(a13f5f8a) SHA1(6a9d3b5a71a3249f3b9491d541c9854e071a320c), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(1))
	ROMX_LOAD( "23-150f1-00.e87", 0xc801, 0x0400, CRC(ca8d6419) SHA1(6d9c3e1e2f5a35f92c82240fcede14645aa83340), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(1))
	ROMX_LOAD( "23-151f1-00.e74", 0xc801, 0x0400, CRC(58ce48d3) SHA1(230dbcab1470752befb6733a89e3612ad7fba10d), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(1))

	ROMX_LOAD( "23-236f1-00.e57", 0xd000, 0x0400, CRC(6f23470a) SHA1(d90a0bc56f04c2830f8cfb6b870db207b96e75b1), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(1))
	ROMX_LOAD( "23-237f1-00.e69", 0xd000, 0x0400, CRC(2bf8cf0b) SHA1(6db79c5392b265e38b5b8b386528d7c138d995e9), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(1))
	ROMX_LOAD( "23-238f1-00.e85", 0xd001, 0x0400, CRC(ff569f71) SHA1(05985396047fb4639959000a1abe50d2f184deaa), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(1))
	ROMX_LOAD( "23-239f1-00.e73", 0xd001, 0x0400, CRC(cec7abe3) SHA1(8b8b52bd46340c58efa5adef3f306e0cdcb77520), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(1))
ROM_END

} // anonymous namespace


/*    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  STATE        INIT        COMPANY                          FULLNAME      FLAGS */
COMP( 1984, vax785, 0,      0,      vax11,   vax11, vax11_state, empty_init, "Digital Equipment Corporation", "VAX-11/785", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
