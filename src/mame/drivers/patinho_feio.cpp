// license:GPL-2.0+
// copyright-holders:Felipe Sanches
/*
    Patinho Feio
*/

#include "emu.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "softlist.h"
#include "cpu/patinhofeio/patinhofeio_cpu.h"
#include "includes/patinhofeio.h"
#include "patinho.lh"

/*
    driver init function
*/
DRIVER_INIT_MEMBER(patinho_feio_state, patinho_feio)
{
	m_out = &output();
	m_prev_ACC = 0;
	m_prev_opcode = 0;
	m_prev_mem_data = 0;
	m_prev_mem_addr = 0;
	m_prev_PC = 0;
	m_prev_FLAGS = 0;
	m_prev_RC = 0;
}

READ16_MEMBER(patinho_feio_state::rc_r)
{
	return ioport("RC_HIGH")->read() << 8 | ioport("RC_LOW")->read();
}

READ8_MEMBER(patinho_feio_state::buttons_r)
{
	return ioport("BUTTONS")->read();
}

void patinho_feio_state::update_panel(UINT8 ACC, UINT8 opcode, UINT8 mem_data, UINT16 mem_addr, UINT16 PC, UINT8 FLAGS, UINT16 RC){
	if ((m_prev_ACC ^ ACC) & (1 << 0)) m_out->set_value("acc0", (ACC >> 0) & 1);
	if ((m_prev_ACC ^ ACC) & (1 << 1)) m_out->set_value("acc1", (ACC >> 1) & 1);
	if ((m_prev_ACC ^ ACC) & (1 << 2)) m_out->set_value("acc2", (ACC >> 2) & 1);
	if ((m_prev_ACC ^ ACC) & (1 << 3)) m_out->set_value("acc3", (ACC >> 3) & 1);
	if ((m_prev_ACC ^ ACC) & (1 << 4)) m_out->set_value("acc4", (ACC >> 4) & 1);
	if ((m_prev_ACC ^ ACC) & (1 << 5)) m_out->set_value("acc5", (ACC >> 5) & 1);
	if ((m_prev_ACC ^ ACC) & (1 << 6)) m_out->set_value("acc6", (ACC >> 6) & 1);
	if ((m_prev_ACC ^ ACC) & (1 << 7)) m_out->set_value("acc7", (ACC >> 7) & 1);
        m_prev_ACC = ACC;

	if ((m_prev_opcode ^ opcode) & (1 << 0)) m_out->set_value("opcode0", (opcode >> 0) & 1);
	if ((m_prev_opcode ^ opcode) & (1 << 1)) m_out->set_value("opcode1", (opcode >> 1) & 1);
	if ((m_prev_opcode ^ opcode) & (1 << 2)) m_out->set_value("opcode2", (opcode >> 2) & 1);
	if ((m_prev_opcode ^ opcode) & (1 << 3)) m_out->set_value("opcode3", (opcode >> 3) & 1);
	if ((m_prev_opcode ^ opcode) & (1 << 4)) m_out->set_value("opcode4", (opcode >> 4) & 1);
	if ((m_prev_opcode ^ opcode) & (1 << 5)) m_out->set_value("opcode5", (opcode >> 5) & 1);
	if ((m_prev_opcode ^ opcode) & (1 << 6)) m_out->set_value("opcode6", (opcode >> 6) & 1);
	if ((m_prev_opcode ^ opcode) & (1 << 7)) m_out->set_value("opcode7", (opcode >> 7) & 1);
	m_prev_opcode = opcode;

	if ((m_prev_mem_data ^ mem_data) & (1 << 0)) m_out->set_value("mem_data0", (mem_data >> 0) & 1);
	if ((m_prev_mem_data ^ mem_data) & (1 << 1)) m_out->set_value("mem_data1", (mem_data >> 1) & 1);
	if ((m_prev_mem_data ^ mem_data) & (1 << 2)) m_out->set_value("mem_data2", (mem_data >> 2) & 1);
	if ((m_prev_mem_data ^ mem_data) & (1 << 3)) m_out->set_value("mem_data3", (mem_data >> 3) & 1);
	if ((m_prev_mem_data ^ mem_data) & (1 << 4)) m_out->set_value("mem_data4", (mem_data >> 4) & 1);
	if ((m_prev_mem_data ^ mem_data) & (1 << 5)) m_out->set_value("mem_data5", (mem_data >> 5) & 1);
	if ((m_prev_mem_data ^ mem_data) & (1 << 6)) m_out->set_value("mem_data6", (mem_data >> 6) & 1);
	if ((m_prev_mem_data ^ mem_data) & (1 << 7)) m_out->set_value("mem_data7", (mem_data >> 7) & 1);
	m_prev_mem_data = mem_data;

	if ((m_prev_mem_addr ^ mem_addr) & (1 << 0)) m_out->set_value("mem_addr0", (mem_addr >> 0) & 1);
	if ((m_prev_mem_addr ^ mem_addr) & (1 << 1)) m_out->set_value("mem_addr1", (mem_addr >> 1) & 1);
	if ((m_prev_mem_addr ^ mem_addr) & (1 << 2)) m_out->set_value("mem_addr2", (mem_addr >> 2) & 1);
	if ((m_prev_mem_addr ^ mem_addr) & (1 << 3)) m_out->set_value("mem_addr3", (mem_addr >> 3) & 1);
	if ((m_prev_mem_addr ^ mem_addr) & (1 << 4)) m_out->set_value("mem_addr4", (mem_addr >> 4) & 1);
	if ((m_prev_mem_addr ^ mem_addr) & (1 << 5)) m_out->set_value("mem_addr5", (mem_addr >> 5) & 1);
	if ((m_prev_mem_addr ^ mem_addr) & (1 << 6)) m_out->set_value("mem_addr6", (mem_addr >> 6) & 1);
	if ((m_prev_mem_addr ^ mem_addr) & (1 << 7)) m_out->set_value("mem_addr7", (mem_addr >> 7) & 1);
	if ((m_prev_mem_addr ^ mem_addr) & (1 << 8)) m_out->set_value("mem_addr8", (mem_addr >> 8) & 1);
	if ((m_prev_mem_addr ^ mem_addr) & (1 << 9)) m_out->set_value("mem_addr9", (mem_addr >> 9) & 1);
	if ((m_prev_mem_addr ^ mem_addr) & (1 << 10)) m_out->set_value("mem_addr10", (mem_addr >> 10) & 1);
	if ((m_prev_mem_addr ^ mem_addr) & (1 << 11)) m_out->set_value("mem_addr11", (mem_addr >> 11) & 1);
	m_prev_mem_addr = mem_addr;

	if ((m_prev_PC ^ PC) & (1 << 0)) m_out->set_value("pc0", (PC >> 0) & 1);
	if ((m_prev_PC ^ PC) & (1 << 1)) m_out->set_value("pc1", (PC >> 1) & 1);
	if ((m_prev_PC ^ PC) & (1 << 2)) m_out->set_value("pc2", (PC >> 2) & 1);
	if ((m_prev_PC ^ PC) & (1 << 3)) m_out->set_value("pc3", (PC >> 3) & 1);
	if ((m_prev_PC ^ PC) & (1 << 4)) m_out->set_value("pc4", (PC >> 4) & 1);
	if ((m_prev_PC ^ PC) & (1 << 5)) m_out->set_value("pc5", (PC >> 5) & 1);
	if ((m_prev_PC ^ PC) & (1 << 6)) m_out->set_value("pc6", (PC >> 6) & 1);
	if ((m_prev_PC ^ PC) & (1 << 7)) m_out->set_value("pc7", (PC >> 7) & 1);
	if ((m_prev_PC ^ PC) & (1 << 8)) m_out->set_value("pc8", (PC >> 8) & 1);
	if ((m_prev_PC ^ PC) & (1 << 9)) m_out->set_value("pc9", (PC >> 9) & 1);
	if ((m_prev_PC ^ PC) & (1 << 10)) m_out->set_value("pc10", (PC >> 10) & 1);
	if ((m_prev_PC ^ PC) & (1 << 11)) m_out->set_value("pc11", (PC >> 11) & 1);
	m_prev_PC = PC;

	if ((m_prev_FLAGS ^ FLAGS) & (1 << 0)) m_out->set_value("flags0", (FLAGS >> 0) & 1);
	if ((m_prev_FLAGS ^ FLAGS) & (1 << 1)) m_out->set_value("flags1", (FLAGS >> 1) & 1);
	m_prev_FLAGS = FLAGS;

	if ((m_prev_RC ^ RC) & (1 << 0)) m_out->set_value("rc0", (RC >> 0) & 1);
	if ((m_prev_RC ^ RC) & (1 << 1)) m_out->set_value("rc1", (RC >> 1) & 1);
	if ((m_prev_RC ^ RC) & (1 << 2)) m_out->set_value("rc2", (RC >> 2) & 1);
	if ((m_prev_RC ^ RC) & (1 << 3)) m_out->set_value("rc3", (RC >> 3) & 1);
	if ((m_prev_RC ^ RC) & (1 << 4)) m_out->set_value("rc4", (RC >> 4) & 1);
	if ((m_prev_RC ^ RC) & (1 << 5)) m_out->set_value("rc5", (RC >> 5) & 1);
	if ((m_prev_RC ^ RC) & (1 << 6)) m_out->set_value("rc6", (RC >> 6) & 1);
	if ((m_prev_RC ^ RC) & (1 << 7)) m_out->set_value("rc7", (RC >> 7) & 1);
	if ((m_prev_RC ^ RC) & (1 << 8)) m_out->set_value("rc8", (RC >> 8) & 1);
	if ((m_prev_RC ^ RC) & (1 << 9)) m_out->set_value("rc9", (RC >> 9) & 1);
	if ((m_prev_RC ^ RC) & (1 << 10)) m_out->set_value("rc10", (RC >> 10) & 1);
	if ((m_prev_RC ^ RC) & (1 << 11)) m_out->set_value("rc11", (RC >> 11) & 1);
	m_prev_RC = RC;
}

WRITE8_MEMBER(patinho_feio_state::decwriter_data_w)
{
	m_decwriter->write(space, 0, data);

	m_maincpu->set_iodev_status(0xA, IODEV_BUSY);

	if (data == 0x0D){
		m_decwriter_timer->adjust(attotime::from_hz(1/0.700)); //carriage return takes 700 msecs
	} else {
		m_decwriter_timer->adjust(attotime::from_hz(10)); //10 characters per second
	}
	m_decwriter_timer->enable(1); //start the timer
}

/*
    timer callback to generate decwriter char print completion signal
*/
TIMER_CALLBACK_MEMBER(patinho_feio_state::decwriter_callback)
{
	m_maincpu->set_iodev_status(0xA, IODEV_READY);
	m_decwriter_timer->enable(0); //stop the timer
}

WRITE8_MEMBER(patinho_feio_state::decwriter_kbd_input)
{
	m_maincpu->transfer_byte_from_external_device(0xA, ~data);
}

WRITE8_MEMBER(patinho_feio_state::teletype_data_w)
{
        m_tty->write(space, 0, data);

	m_maincpu->set_iodev_status(0xB, IODEV_READY);
	m_teletype_timer->adjust(attotime::from_hz(10)); //10 characters per second
	m_teletype_timer->enable(1); //start the timer
}

/*
    timer callback to generate teletype char print completion signal
*/
TIMER_CALLBACK_MEMBER(patinho_feio_state::teletype_callback)
{
	m_maincpu->set_iodev_status(0xB, IODEV_READY);
	m_teletype_timer->enable(0); //stop the timer
}

WRITE8_MEMBER(patinho_feio_state::teletype_kbd_input)
{
	//I figured out that the data is provided inverted (2's complement)
	//based on a comment in the source code listing of the HEXAM program.
	//It is not clear though, if all I/O devices complement the data when
	//communicating with the computer, or if this behavious is a particular
	//caracteristics of the teletype.

	m_maincpu->transfer_byte_from_external_device(0xB, ~data);
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
	m_teletype_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(patinho_feio_state::teletype_callback),this));
	m_decwriter_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(patinho_feio_state::decwriter_callback),this));

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
	//load_raw_data("micro_pre_loader", 0x000, 0x02A); //this is still experimental
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

		PORT_START("BUTTONS")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("PARTIDA") PORT_CODE(KEYCODE_Z)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("INTERRUPÇÃO") PORT_CODE(KEYCODE_X)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ESPERA") PORT_CODE(KEYCODE_C)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("PREPARAÇÃO") PORT_CODE(KEYCODE_V)
INPUT_PORTS_END

static MACHINE_CONFIG_START( patinho_feio, patinho_feio_state )
	/* basic machine hardware */
	/* CPU @ approx. 500 kHz (memory cycle time is 2usec) */
	MCFG_CPU_ADD("maincpu", PATINHO_FEIO, 500000)
	MCFG_PATINHO_RC_READ_CB(READ16(patinho_feio_state, rc_r))
	MCFG_PATINHO_BUTTONS_READ_CB(READ8(patinho_feio_state, buttons_r))

	/* Printer */
//	MCFG_PATINHO_IODEV_WRITE_CB(0x5, WRITE8(patinho_feio_state, printer_data_w))

	/* Papertape Puncher */
//	MCFG_PATINHO_IODEV_WRITE_CB(0x8, WRITE8(patinho_feio_state, papertape_punch_data_w))

	/* Card Reader */
//	MCFG_PATINHO_IODEV_READ_CB(0x9, READ8(patinho_feio_state, cardreader_data_r))

	/* DECWRITER
           (max. speed: ?) */
	MCFG_PATINHO_IODEV_WRITE_CB(0xA, WRITE8(patinho_feio_state, decwriter_data_w))

	/* Teleprinter
           TeleType ASR33
           (max. speed: 10 characteres per second)
           with paper tape reading (and optionally punching) capabilities */
	MCFG_PATINHO_IODEV_WRITE_CB(0xB, WRITE8(patinho_feio_state, teletype_data_w))

	/* Papertape Reader
           Hewlett-Packard HP-2737-A
           Optical Papertape Reader (max. speed: 300 characteres per second) */
//	MCFG_PATINHO_IODEV_READ_CB(0xE, READ8(patinho_feio_state, papertapereader_data_r))


        /* DECWRITER */
	MCFG_DEVICE_ADD("decwriter", TELEPRINTER, 0)
	MCFG_GENERIC_TELEPRINTER_KEYBOARD_CB(WRITE8(patinho_feio_state, decwriter_kbd_input))

        /* Teletype */
	MCFG_DEVICE_ADD("teletype", TELEPRINTER, 1)
	MCFG_GENERIC_TELEPRINTER_KEYBOARD_CB(WRITE8(patinho_feio_state, teletype_kbd_input))

	/* punched tape */
	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "patinho_tape")
	MCFG_GENERIC_EXTENSIONS("bin")
	MCFG_GENERIC_LOAD(patinho_feio_state, patinho_tape)


	MCFG_DEFAULT_LAYOUT(layout_patinho)

	// software lists
//	MCFG_SOFTWARE_LIST_ADD("tape_list", "patinho")
MACHINE_CONFIG_END

ROM_START( patinho )
	ROM_REGION( 0x0d5, "hexam", 0 )
	ROM_LOAD( "apendice_g__hexam.bin", 0x000, 0x0d5, CRC(e608f6d3) SHA1(3f76b5f91d9b2573e70919539d47752e7623e40a) )

	ROM_REGION( 0x0d5, "exemplo_16.7", 0 )
	ROM_LOAD( "exemplo_16.7.bin", 0x000, 0x028, CRC(0a87ac8d) SHA1(7c35ac3eed9ed239f2ef56c26e6f0c59f635e1ac) )

	ROM_REGION( 0x080, "loader", 0 )
	ROM_LOAD( "loader.bin", 0x000, 0x080, BAD_DUMP CRC(c2a8fa9d) SHA1(0ae4f711ef5d6e9d26c611fd2c8c8ac45ecbf9e7) )

	/* Micro pre-loader:
	   This was re-created by professor Joao Jose Neto based on his vague
           recollection of sequences of opcode values from almost 40 years ago :-) */
	ROM_REGION( 0x02a, "micro_pre_loader", 0 ) 
	ROM_LOAD( "micro-pre-loader.bin", 0x000, 0x02a, CRC(1921feab) SHA1(bb063102e44e9ab963f95b45710141dc2c5046b0) )
ROM_END

/*    YEAR  NAME      PARENT    COMPAT  MACHINE        INPUT         INIT                              COMPANY                                           FULLNAME */
COMP( 1972, patinho,  0,        0,      patinho_feio,  patinho_feio, patinho_feio_state, patinho_feio, "Escola Politecnica - Universidade de Sao Paulo", "Patinho Feio" , MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING)
