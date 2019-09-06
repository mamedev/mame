// license:BSD-3-Clause
// copyright-holders:smf
#include "emu.h"
#include "k573dio.h"

/*
  Digital I/O PCB
  ---------------

  GX894-PWB(B)A (C)1999 KONAMI CO. LTD.

             |-------------|
             |        CN12 |
             |             |
             | PC847 PC847 |
             |             |
             |        CN11 |
             |             |
             | PC847 PC847 |
             |             |
             | DS2401 CN10 |
             |             |
             | PC847 PC847 |
             |             |
             |  CN14  CN13 |
  |----------|             |----------|
  |                  PC847            |
  | ADM232 CN17              XC9536   |
  |                                   |
  |                    19.6608MHz     |-----------|
  | ADM232 CN15  CY7C109                          |
  |                       HY51V65164A HY51V65164A |
  |                            HY51V65164A        |
  |      CN16    XCS40XL                          |
  |                                               |
  | AK4309B   CN18         29.450MHz  MAS3507D    |
  |                                               |
  |                           CN3                 |
  | HYC24855  RCA-L/R                             |
  |-----------------------------------------------|

  Notes:

  PC847       - High Density Mounting Type Photocoupler
  CN12        - 13 pin connector with 8 wires to external connectors
  CN11        - 12 pin connector with 8 wires to external connectors
  DS2401      - DS2401 911C2  Silicon serial number
  CN10        - 10 pin connector with 8 wires to external connectors
  CN14        - 7 pin connector
  CN13        - 5 pin connector with 2 wires to external connectors
  ADM232      - ADM232AARN 9933 H48475  High Speed, 5 V, 0.1 uF CMOS RS-232 Drivers/Receivers
  CN17        - 3 pin connector
  XC9536      - XILINX XC9536 PC44AEM9933 F1096429A 15C
  CN15        - 8 pin connector
  CY7C109     - CY7C109-25VC 931 H 04 404825  128k x 8 Static RAM
  HY51V65164A - 64M bit dynamic EDO RAM
  CN16        - 4 pin connector joining this PCB to the CD-DA IN on the MAIN PCB.
  XCS40XL     - XILINX XCS40XL PQ208AKP9929 A2033251A 4C
  AK4309B     - AKM AK4309B 3N932N  16bit SCF DAC
  CN18        - 6 pin connector
  MAS3507D    - IM MAS3507D D8 9173 51 HM U 072953.000 ES  MPEG 1/2 Layer 2/3 Audio Decoder
  CN3         - Connector joining this PCB to the MAIN PCB
  HYC24855    - ?
  RCA-L/R     - RCA connectors for left/right audio output

*/

DEFINE_DEVICE_TYPE(KONAMI_573_DIGITAL_IO_BOARD, k573dio_device, "k573_dio", "Konami 573 digital I/O board")

void k573dio_device::amap(address_map &map)
{
	map(0x00, 0x01).r(FUNC(k573dio_device::a00_r));
	map(0x02, 0x03).r(FUNC(k573dio_device::a02_r));
	map(0x04, 0x05).r(FUNC(k573dio_device::a04_r));
	map(0x06, 0x07).r(FUNC(k573dio_device::a06_r));
	map(0x0a, 0x0b).r(FUNC(k573dio_device::a0a_r));
	map(0x10, 0x11).w(FUNC(k573dio_device::a10_w));
	map(0x80, 0x81).r(FUNC(k573dio_device::a80_r));
	map(0xc4, 0xc5).r(FUNC(k573dio_device::ac4_r));
	map(0xa0, 0xa1).w(FUNC(k573dio_device::mpeg_start_adr_high_w));
	map(0xa2, 0xa3).w(FUNC(k573dio_device::mpeg_start_adr_low_w));
	map(0xa4, 0xa5).w(FUNC(k573dio_device::mpeg_end_adr_high_w));
	map(0xa6, 0xa7).w(FUNC(k573dio_device::mpeg_end_adr_low_w));
	map(0xa8, 0xa9).rw(FUNC(k573dio_device::mpeg_key_1_r), FUNC(k573dio_device::mpeg_key_1_w));
	map(0xac, 0xad).rw(FUNC(k573dio_device::mas_i2c_r), FUNC(k573dio_device::mas_i2c_w));
	map(0xae, 0xaf).rw(FUNC(k573dio_device::mpeg_ctrl_r), FUNC(k573dio_device::mpeg_ctrl_w));
	map(0xb0, 0xb1).w(FUNC(k573dio_device::ram_write_adr_high_w));
	map(0xb2, 0xb3).w(FUNC(k573dio_device::ram_write_adr_low_w));
	map(0xb4, 0xb5).rw(FUNC(k573dio_device::ram_r), FUNC(k573dio_device::ram_w));
	map(0xb6, 0xb7).w(FUNC(k573dio_device::ram_read_adr_high_w));
	map(0xb8, 0xb9).w(FUNC(k573dio_device::ram_read_adr_low_w));
	map(0xca, 0xcb).r(FUNC(k573dio_device::mp3_frame_count_high_r));
	map(0xcc, 0xcd).r(FUNC(k573dio_device::mp3_frame_count_low_r));
	map(0xce, 0xcf).r(FUNC(k573dio_device::mp3_unk_r));
	map(0xe0, 0xe1).w(FUNC(k573dio_device::output_1_w));
	map(0xe2, 0xe3).w(FUNC(k573dio_device::output_0_w));
	map(0xe4, 0xe5).w(FUNC(k573dio_device::output_3_w));
	map(0xe6, 0xe7).w(FUNC(k573dio_device::output_7_w));
	map(0xea, 0xeb).w(FUNC(k573dio_device::mpeg_key_2_w));
	map(0xec, 0xed).w(FUNC(k573dio_device::mpeg_key_3_w));
	map(0xee, 0xef).rw(FUNC(k573dio_device::digital_id_r), FUNC(k573dio_device::digital_id_w));
	map(0xf6, 0xf7).r(FUNC(k573dio_device::fpga_status_r));
	map(0xf8, 0xf9).w(FUNC(k573dio_device::fpga_firmware_w));
	map(0xfa, 0xfb).w(FUNC(k573dio_device::output_4_w));
	map(0xfc, 0xfd).w(FUNC(k573dio_device::output_5_w));
	map(0xfe, 0xff).w(FUNC(k573dio_device::output_2_w));
}

k573dio_device::k573dio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, KONAMI_573_DIGITAL_IO_BOARD, tag, owner, clock),
	k573fpga(*this, "k573fpga"),
	digital_id(*this, "digital_id"),
	mas3507d(*this, "mpeg"),
	output_cb(*this),
	is_ddrsbm_fpga(false)
{
}

void k573dio_device::device_start()
{
	output_cb.resolve_safe();

	ram = std::make_unique<uint16_t[]>(0x2000000/2);
	save_pointer(NAME(ram), 0x2000000/2 );

	k573fpga->set_ram(ram.get());
	k573fpga->set_ddrsbm_fpga(is_ddrsbm_fpga);
}

void k573dio_device::device_reset()
{
	ram_adr = 0;
	ram_read_adr = 0;

	memset(output_data, 0, sizeof(output_data));
}

ROM_START( k573dio )
	ROM_REGION( 0x000008, "digital_id", 0 )
	ROM_LOAD( "digital-id.bin",   0x000000, 0x000008, CRC(2b977f4d) SHA1(2b108a56653f91cb3351718c45dfcf979bc35ef1) )
ROM_END

const tiny_rom_entry *k573dio_device::device_rom_region() const
{
	return ROM_NAME(k573dio);
}

void k573dio_device::device_add_mconfig(machine_config &config)
{
	KONAMI_573_DIGITAL_FPGA(config, k573fpga);
	DS2401(config, digital_id);
	MAS3507D(config, mas3507d);
	mas3507d->sample_cb().set(k573fpga, FUNC(k573fpga_device::get_decrypted));
	mas3507d->add_route(0, ":lspeaker", 1.0);
	mas3507d->add_route(1, ":rspeaker", 1.0);
}

void k573dio_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
}

READ16_MEMBER(k573dio_device::a00_r)
{
	logerror("%s: a00_r (%s)\n", tag(), machine().describe_context());
	return 0x0000;
}

READ16_MEMBER(k573dio_device::a02_r)
{
	logerror("%s: a02_r (%s)\n", tag(), machine().describe_context());
	return 0x0001;
}

READ16_MEMBER(k573dio_device::a04_r)
{
	logerror("%s: a04_r (%s)\n", tag(), machine().describe_context());
	return 0x0000;
}

READ16_MEMBER(k573dio_device::a06_r)
{
	logerror("%s: a06_r (%s)\n", tag(), machine().describe_context());
	return 0x0000;
}

READ16_MEMBER(k573dio_device::a0a_r)
{
	logerror("%s: a0a_r (%s)\n", tag(), machine().describe_context());
	return 0x0000;
}

WRITE16_MEMBER(k573dio_device::a10_w)
{
	logerror("%s: a10_w (%s)\n", tag(), machine().describe_context());
}

READ16_MEMBER(k573dio_device::ac4_r)
{
	// What is this?
	return 0;
}

READ16_MEMBER(k573dio_device::a80_r)
{
	logerror("%s: a80_r (%s)\n", tag(), machine().describe_context());
	return 0x1234;
}

WRITE16_MEMBER(k573dio_device::mpeg_start_adr_high_w)
{
	logerror("FPGA MPEG start address high %04x\n", data);
	k573fpga->set_mp3_cur_adr((k573fpga->get_mp3_cur_adr() & 0x0000ffff) | (data << 16)); // high
}

WRITE16_MEMBER(k573dio_device::mpeg_start_adr_low_w)
{
	logerror("FPGA MPEG start address low %04x\n", data);
	k573fpga->set_mp3_cur_adr((k573fpga->get_mp3_cur_adr() & 0xffff0000) | data); // low

	if(is_ddrsbm_fpga)
		k573fpga->set_crypto_key3(0);
}

WRITE16_MEMBER(k573dio_device::mpeg_end_adr_high_w)
{
	logerror("FPGA MPEG end address high %04x\n", data);
	k573fpga->set_mp3_end_adr((k573fpga->get_mp3_end_adr() & 0x0000ffff) | (data << 16)); // high
}

WRITE16_MEMBER(k573dio_device::mpeg_end_adr_low_w)
{
	logerror("FPGA MPEG end address low %04x\n", data);
	k573fpga->set_mp3_end_adr((k573fpga->get_mp3_end_adr() & 0xffff0000) | data); // low
}

READ16_MEMBER(k573dio_device::mpeg_key_1_r)
{
	// Dance Dance Revolution Solo Bass Mix reads this key before starting songs
	return crypto_key1;
}

WRITE16_MEMBER(k573dio_device::mpeg_key_1_w)
{
	logerror("FPGA MPEG key 1/3 %04x\n", data);
	crypto_key1 = data;
	k573fpga->set_crypto_key1(data);
}

READ16_MEMBER(k573dio_device::mas_i2c_r)
{
	int scl = mas3507d->i2c_scl_r() << 13;
	int sda = mas3507d->i2c_sda_r() << 12;

	return scl | sda;
}

WRITE16_MEMBER(k573dio_device::mas_i2c_w)
{
	mas3507d->i2c_scl_w(data & 0x2000);
	mas3507d->i2c_sda_w(data & 0x1000);
}

READ16_MEMBER(k573dio_device::mpeg_ctrl_r)
{
	if (k573fpga->get_mpeg_ctrl() == 0x1000 && !k573fpga->is_playing()) {
		// Set the FPGA to stop mode so that data won't be sent anymore
		k573fpga->set_mpeg_ctrl(0xa000);
	}

	return k573fpga->get_mpeg_ctrl();
}

WRITE16_MEMBER(k573dio_device::mpeg_ctrl_w)
{
	k573fpga->set_mpeg_ctrl(data);
}

WRITE16_MEMBER(k573dio_device::ram_write_adr_high_w)
{
	// read and write address are shared
	ram_adr = ((ram_adr & 0x0000ffff) | (data << 16)) & 0x1ffffff;
}

WRITE16_MEMBER(k573dio_device::ram_write_adr_low_w)
{
	// read and write address are shared
	ram_adr = ((ram_adr & 0xffff0000) | data) & 0x1ffffff;
}

READ16_MEMBER(k573dio_device::ram_r)
{
	uint16_t res = ram[(ram_read_adr & 0x1ffffff) >> 1];
	ram_read_adr += 2;
	return res;
}

WRITE16_MEMBER(k573dio_device::ram_w)
{
	ram[(ram_adr & 0x1ffffff) >> 1] = data;
	ram_adr += 2;
}

WRITE16_MEMBER(k573dio_device::ram_read_adr_high_w)
{
	// read and write address are shared
	ram_read_adr = ((ram_read_adr & 0x0000ffff) | (data << 16)) & 0x1ffffff;
}

WRITE16_MEMBER(k573dio_device::ram_read_adr_low_w)
{
	// read and write address are shared
	ram_read_adr = ((ram_read_adr & 0xffff0000) | data) & 0x1ffffff;
}

READ16_MEMBER(k573dio_device::mp3_frame_count_high_r)
{
	return (mas3507d->get_frame_count() & 0xffff0000) >> 16;
}

READ16_MEMBER(k573dio_device::mp3_frame_count_low_r)
{
	return mas3507d->get_frame_count() & 0x0000ffff;
}

WRITE16_MEMBER(k573dio_device::output_1_w)
{
	output(1, data);
}

WRITE16_MEMBER(k573dio_device::output_0_w)
{
	output(0, data);
}

WRITE16_MEMBER(k573dio_device::output_3_w)
{
	output(3, data);
}

WRITE16_MEMBER(k573dio_device::output_7_w)
{
	output(7, data);
}

WRITE16_MEMBER(k573dio_device::mpeg_key_2_w)
{
	logerror("FPGA MPEG key 2/3 %04x\n", data);
	k573fpga->set_crypto_key2(data);
}

WRITE16_MEMBER(k573dio_device::mpeg_key_3_w)
{
	logerror("FPGA MPEG key 3/3 %04x\n", data);
	k573fpga->set_crypto_key3(data);
}

READ16_MEMBER(k573dio_device::digital_id_r)
{
	return digital_id->read() << 12;
}

WRITE16_MEMBER(k573dio_device::digital_id_w)
{
	digital_id->write( !( ( data >> 12 ) & 1 ) );
}

READ16_MEMBER(k573dio_device::fpga_status_r)
{
	//logerror("%s: fpga_status_r (%s)\n", tag(), machine().describe_context());

	// fpga/digital board status checks
	// wants & c000 = 8000 (just after program upload?)
	// write 0000 to +f4.w
	// write 8000 to +f6.w

	/* fails if !8000 */
	/* fails if  4000 */
	/* fails if !2000 */
	/* fails if !1000 */
	return 0x8000 | 0x2000 | 0x1000;
}

WRITE16_MEMBER(k573dio_device::fpga_firmware_w)
{
	// Firmware bits in bit 15, always the same firmware
}

WRITE16_MEMBER(k573dio_device::output_4_w)
{
	output(4, data);
}

WRITE16_MEMBER(k573dio_device::output_5_w)
{
	output(5, data);
}

WRITE16_MEMBER(k573dio_device::output_2_w)
{
	output(2, data);
}

READ16_MEMBER(k573dio_device::mp3_unk_r)
{
	return 0;
}

void k573dio_device::output(int offset, uint16_t data)
{
	data = (data >> 12) & 0x0f;
	static const int shift[] = { 0, 2, 3, 1 };
	for(int i = 0; i < 4; i++) {
		int oldbit = (output_data[offset] >> shift[i]) & 1;
		int newbit = (data                >> shift[i]) & 1;
		if(oldbit != newbit)
			output_cb(4*offset + i, newbit, 0xff);
	}
	output_data[offset] = data;
}
