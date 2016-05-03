// license:GPL-2.0+
// copyright-holders:Felipe Sanches
/*
    Patinho Feio
*/

#include "emu.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "softlist.h"
#include "cpu/patinhofeio/patinho_feio.h"
#include "machine/terminal.h"

class patinho_feio_state : public driver_device
{
public:
	patinho_feio_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_terminal(*this, "decwriter_paper")
	{ }

	DECLARE_DRIVER_INIT(patinho_feio);
	DECLARE_READ16_MEMBER(rc_r);
	DECLARE_READ8_MEMBER(decwriter_status_r);
//	DECLARE_READ8_MEMBER(decwriter_data_r);
	DECLARE_WRITE8_MEMBER(decwriter_data_w);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( patinho_tape );
	void load_tape(const char* name);
	void load_raw_data(const char* name, unsigned int start_address, unsigned int data_length);
	virtual void machine_start() override;

	required_device<generic_terminal_device> m_terminal;
private:
        UINT8* paper_tape_data;
        UINT32 paper_tape_length;
        UINT32 paper_tape_address;
};

/*
    driver init function
*/
DRIVER_INIT_MEMBER(patinho_feio_state, patinho_feio)
{
}

READ16_MEMBER(patinho_feio_state::rc_r)
{
	return ioport("RC_HIGH")->read() << 8 | ioport("RC_LOW")->read();
}

WRITE8_MEMBER(patinho_feio_state::decwriter_data_w)
{
	m_terminal->write(space, 0, data);
}

READ8_MEMBER(patinho_feio_state::decwriter_status_r)
{
	//This should only return true after a certain delay
	// We should verify in the DECWRITER specs what is its speed
	// (in characters per second) in order to implement
	// the high-level emulation of its behaviour here.
        return true;
}

/* The hardware does not perform this checking.
   This is implemented here only for debugging purposes.

   Also, proper punched paper tape emulation does
   not use this function at all.
*/
void patinho_feio_state::load_tape(const char* name){
	UINT8 *RAM = (UINT8 *) memshare("maincpu:internalram")->ptr();
	UINT8 *data = memregion(name)->base();
	unsigned int data_length = data[0];
	unsigned int start_address = data[1]*256 + data[2];
	INT8 expected_checksum = data[data_length + 3];
	INT8 checksum = 0;

	for (int i = 0; i < data_length + 3; i++){
		checksum -= (INT8) data[i];
	}

	if (checksum != expected_checksum){
		printf("[WARNING] Tape \"%s\": checksum = 0x%02X (expected 0x%02X)\n",
				name, (unsigned char) checksum, (unsigned char) expected_checksum);
	}

	memcpy(&RAM[start_address], &data[3], data_length);
}

void patinho_feio_state::load_raw_data(const char* name, unsigned int start_address, unsigned int data_length){
	UINT8 *RAM = (UINT8 *) memshare("maincpu:internalram")->ptr();
	UINT8 *data = memregion(name)->base();

	memcpy(&RAM[start_address], data, data_length);
}

DEVICE_IMAGE_LOAD_MEMBER( patinho_feio_state, patinho_tape )
{
    if (image.software_entry() != nullptr)
    {
        paper_tape_length = image.get_software_region_length("rom");
        paper_tape_data = image.get_software_region("rom");
        paper_tape_address = 0;
    }

    return IMAGE_INIT_PASS;
}

void patinho_feio_state::machine_start(){
	// Copy some programs directly into RAM.
	// This is a hack for setting up the computer
	// while we don't support loading programs
	// from punched tape rolls...

	//"absolute program example" from page 16.7
	//    Prints "PATINHO FEIO" on the DECWRITER:
	load_tape("exemplo_16.7");

	//"absolute program example" from appendix G:
	//    Allows users to load programs from the
	//    console into the computer memory.
	load_raw_data("hexam", 0xE00, 0x0D5);

	load_raw_data("loader", 0xF80, 0x080);
}

static INPUT_PORTS_START( patinho_feio )
		PORT_START("RC_LOW")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RC bit 0") PORT_CODE(KEYCODE_EQUALS) PORT_TOGGLE
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RC bit 1") PORT_CODE(KEYCODE_MINUS) PORT_TOGGLE
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RC bit 2") PORT_CODE(KEYCODE_0) PORT_TOGGLE
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RC bit 3") PORT_CODE(KEYCODE_9) PORT_TOGGLE
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RC bit 4") PORT_CODE(KEYCODE_8) PORT_TOGGLE
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RC bit 5") PORT_CODE(KEYCODE_7) PORT_TOGGLE
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RC bit 6") PORT_CODE(KEYCODE_6) PORT_TOGGLE
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RC bit 7") PORT_CODE(KEYCODE_5) PORT_TOGGLE

		PORT_START("RC_HIGH")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RC bit 8") PORT_CODE(KEYCODE_4) PORT_TOGGLE
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RC bit 7") PORT_CODE(KEYCODE_3) PORT_TOGGLE
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RC bit 10") PORT_CODE(KEYCODE_2) PORT_TOGGLE
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RC bit 11") PORT_CODE(KEYCODE_1) PORT_TOGGLE
INPUT_PORTS_END

static MACHINE_CONFIG_START( patinho_feio, patinho_feio_state )
	/* basic machine hardware */
	/* CPU @ approx. 500 kHz (memory cycle time is 2usec) */
	MCFG_CPU_ADD("maincpu", PATINHO_FEIO, 500000)
	MCFG_PATINHO_RC_READ_CB(READ16(patinho_feio_state, rc_r))

	/* Printer */
//	MCFG_PATINHO_IODEV_WRITE_CB(0x5, WRITE8(patinho_feio_state, printer_data_w))
//	MCFG_PATINHO_IODEV_STATUS_CB(0x5, READ8(patinho_feio_state, printer_status_r))

	/* Papertape Puncher */
//	MCFG_PATINHO_IODEV_WRITE_CB(0x8, WRITE8(patinho_feio_state, papertape_punch_data_w))
//	MCFG_PATINHO_IODEV_STATUS_CB(0x8, READ8(patinho_feio_state, papertape_punch_status_r))

	/* Card Reader */
//	MCFG_PATINHO_IODEV_READ_CB(0x9, READ8(patinho_feio_state, cardreader_data_r))
//	MCFG_PATINHO_IODEV_STATUS_CB(0x9, READ8(patinho_feio_state, cardreader_status_r))

	/* DECWRITER */
//	MCFG_PATINHO_IODEV_READ_CB(0xA, READ8(patinho_feio_state, decwriter_data_r))
	MCFG_PATINHO_IODEV_WRITE_CB(0xA, WRITE8(patinho_feio_state, decwriter_data_w))
	MCFG_PATINHO_IODEV_STATUS_CB(0xA, READ8(patinho_feio_state, decwriter_status_r))

	/* Teletype */
//	MCFG_PATINHO_IODEV_READ_CB(0xB, READ8(patinho_feio_state, teletype_data_r))
//	MCFG_PATINHO_IODEV_STATUS_CB(0xB, READ8(patinho_feio_state, teletype_status_r))

	/* Papertape Reader */
//	MCFG_PATINHO_IODEV_READ_CB(0xE, READ8(patinho_feio_state, papertapereader_data_r))
//	MCFG_PATINHO_IODEV_STATUS_CB(0xE, READ8(patinho_feio_state, papertapereader_status_r))


        /* video hardware to represent what you'd see
           printed on paper on the DECWRITER */
        MCFG_DEVICE_ADD("decwriter_paper", GENERIC_TERMINAL, 0)

	/* punched tape */
	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "patinho_tape")
	MCFG_GENERIC_EXTENSIONS("bin")
	MCFG_GENERIC_LOAD(patinho_feio_state, patinho_tape)

	// software lists
//	MCFG_SOFTWARE_LIST_ADD("tape_list", "patinho")
MACHINE_CONFIG_END

ROM_START( patinho )
	ROM_REGION( 0x0d5, "hexam", 0 )
	ROM_LOAD( "apendice_g__hexam.bin", 0x000, 0x0d5, CRC(c6addc59) SHA1(126bc97247eac45c58708eaac216c2438e9e4af9) )

	ROM_REGION( 0x0d5, "exemplo_16.7", 0 )
	ROM_LOAD( "exemplo_16.7.bin", 0x000, 0x028, CRC(0a87ac8d) SHA1(7c35ac3eed9ed239f2ef56c26e6f0c59f635e1ac) )

	ROM_REGION( 0x080, "loader", 0 )
	ROM_LOAD( "loader.bin", 0x000, 0x080, BAD_DUMP CRC(c2a8fa9d) SHA1(0ae4f711ef5d6e9d26c611fd2c8c8ac45ecbf9e7) )
ROM_END

/*    YEAR  NAME      PARENT    COMPAT  MACHINE        INPUT         INIT                              COMPANY                                           FULLNAME */
COMP( 1972, patinho,  0,        0,      patinho_feio,  patinho_feio, patinho_feio_state, patinho_feio, "Escola Politecnica - Universidade de Sao Paulo", "Patinho Feio" , MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING)
