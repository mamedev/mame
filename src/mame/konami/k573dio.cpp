// license:BSD-3-Clause
// copyright-holders:smf
#include "emu.h"
#include "k573dio.h"

#define LOG_FPGA       (1U << 1)
#define LOG_MP3        (1U << 2)
#define LOG_UNKNOWNREG (1U << 3)
// #define VERBOSE        (LOG_GENERAL | LOG_FPGA | LOG_MP3 | LOG_UNKNOWNREG)
// #define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

#define LOGFPGA(...)       LOGMASKED(LOG_FPGA, __VA_ARGS__)
#define LOGMP3(...)        LOGMASKED(LOG_MP3, __VA_ARGS__)
#define LOGUNKNOWNREG(...) LOGMASKED(LOG_UNKNOWNREG, __VA_ARGS__)

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
  | HYC2485S  RCA-1/2                             |
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
  HYC2485S    - RS485 transceiver
  RCA-1/2     - RCA connectors for network communication

*/

DEFINE_DEVICE_TYPE(KONAMI_573_DIGITAL_IO_BOARD, k573dio_device, "k573_dio", "Konami 573 digital I/O board")

void k573dio_device::amap(address_map &map)
{
	// TODO: Split address maps between DDR Solo Bass Mix's FPGA code and the normal FPGA code.
	// For example, DDR Solo Bass Mix's FPGA code returns 0x7654 for unused registers like mp3_counter_high_r.

	map(0x00, 0x01).r(FUNC(k573dio_device::a00_r));
	map(0x02, 0x03).r(FUNC(k573dio_device::a02_r));
	map(0x04, 0x05).r(FUNC(k573dio_device::a04_r));
	map(0x06, 0x07).r(FUNC(k573dio_device::a06_r));
	map(0x0a, 0x0b).r(FUNC(k573dio_device::a0a_r));
	map(0x10, 0x11).w(FUNC(k573dio_device::a10_w));
	map(0x80, 0x81).r(FUNC(k573dio_device::a80_r));
	map(0x90, 0x91).w(FUNC(k573dio_device::network_id_w));
	//map(0x92, 0x93).w(FUNC(k573dio_device::network_unk_w));
	map(0xa0, 0xa1).rw(FUNC(k573dio_device::mpeg_start_adr_high_r), FUNC(k573dio_device::mpeg_start_adr_high_w));
	map(0xa2, 0xa3).rw(FUNC(k573dio_device::mpeg_start_adr_low_r), FUNC(k573dio_device::mpeg_start_adr_low_w));
	map(0xa4, 0xa5).rw(FUNC(k573dio_device::mpeg_end_adr_high_r), FUNC(k573dio_device::mpeg_end_adr_high_w));
	map(0xa6, 0xa7).rw(FUNC(k573dio_device::mpeg_end_adr_low_r), FUNC(k573dio_device::mpeg_end_adr_low_w));
	map(0xa8, 0xa9).rw(FUNC(k573dio_device::mpeg_frame_counter_r), FUNC(k573dio_device::mpeg_key_1_w));
	map(0xaa, 0xab).r(FUNC(k573dio_device::mpeg_ctrl_r));
	map(0xac, 0xad).rw(FUNC(k573dio_device::mas_i2c_r), FUNC(k573dio_device::mas_i2c_w));
	map(0xae, 0xaf).rw(FUNC(k573dio_device::fpga_ctrl_r), FUNC(k573dio_device::fpga_ctrl_w));
	map(0xb0, 0xb1).w(FUNC(k573dio_device::ram_write_adr_high_w));
	map(0xb2, 0xb3).w(FUNC(k573dio_device::ram_write_adr_low_w));
	map(0xb4, 0xb5).rw(FUNC(k573dio_device::ram_r), FUNC(k573dio_device::ram_w));
	map(0xb6, 0xb7).w(FUNC(k573dio_device::ram_read_adr_high_w));
	map(0xb8, 0xb9).w(FUNC(k573dio_device::ram_read_adr_low_w));
	map(0xc0, 0xc1).rw(FUNC(k573dio_device::network_r), FUNC(k573dio_device::network_w));
	map(0xc2, 0xc3).r(FUNC(k573dio_device::network_output_buf_size_r));
	map(0xc4, 0xc5).r(FUNC(k573dio_device::network_input_buf_size_r));
	//map(0xc8, 0xc9).w(FUNC(k573dio_device::network_unk2_w));
	map(0xca, 0xcb).r(FUNC(k573dio_device::mp3_counter_high_r));
	map(0xcc, 0xcd).rw(FUNC(k573dio_device::mp3_counter_low_r), FUNC(k573dio_device::mp3_counter_low_w));
	map(0xce, 0xcf).r(FUNC(k573dio_device::mp3_counter_diff_r));
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
	ram(*this, "ram", 0x2000000, ENDIANNESS_LITTLE),
	k573fpga(*this, "k573fpga"),
	digital_id(*this, "digital_id"),
	output_cb(*this),
	is_ddrsbm_fpga(false)
{
}

void k573dio_device::device_start()
{
	save_item(NAME(ram_adr));
	save_item(NAME(ram_read_adr));
	save_item(NAME(output_data));
	save_item(NAME(is_ddrsbm_fpga));
	save_item(NAME(crypto_key1));
	save_item(NAME(fpga_counter));
	save_item(NAME(network_id));

	k573fpga->set_ddrsbm_fpga(is_ddrsbm_fpga);
}

void k573dio_device::device_reset()
{
	ram_adr = 0;
	ram_read_adr = 0;
	crypto_key1 = 0;
	fpga_counter = 0;

	network_id = 0;

	std::fill(std::begin(output_data), std::end(output_data), 0);
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
	k573fpga->set_ram(ram);
	k573fpga->add_route(0, ":speaker", 1.0, 0);
	k573fpga->add_route(1, ":speaker", 1.0, 1);

	DS2401(config, digital_id);
}

uint16_t k573dio_device::a00_r()
{
	LOGUNKNOWNREG("%s: a00_r (%s)\n", tag(), machine().describe_context());
	return 0x0000;
}

uint16_t k573dio_device::a02_r()
{
	LOGUNKNOWNREG("%s: a02_r (%s)\n", tag(), machine().describe_context());
	return 0x0001;
}

uint16_t k573dio_device::a04_r()
{
	LOGUNKNOWNREG("%s: a04_r (%s)\n", tag(), machine().describe_context());
	return 0x0000;
}

uint16_t k573dio_device::a06_r()
{
	LOGUNKNOWNREG("%s: a06_r (%s)\n", tag(), machine().describe_context());
	return 0x0000;
}

uint16_t k573dio_device::a0a_r()
{
	LOGUNKNOWNREG("%s: a0a_r (%s)\n", tag(), machine().describe_context());
	return 0x0000;
}

void k573dio_device::a10_w(uint16_t data)
{
	LOGUNKNOWNREG("%s: a10_w: %04x (%s)\n", tag(), data, machine().describe_context());
}

uint16_t k573dio_device::a80_r()
{
	LOGUNKNOWNREG("%s: a80_r (%s)\n", tag(), machine().describe_context());
	return 0x1234;
}

uint16_t k573dio_device::mpeg_start_adr_high_r()
{
	return k573fpga->get_mp3_start_addr() >> 16;
}

void k573dio_device::mpeg_start_adr_high_w(uint16_t data)
{
	LOGMP3("FPGA MPEG start address high %04x\n", data);
	k573fpga->set_mp3_start_addr((k573fpga->get_mp3_start_addr() & 0x0000ffff) | (data << 16)); // high
}

uint16_t k573dio_device::mpeg_start_adr_low_r()
{
	return k573fpga->get_mp3_start_addr() & 0xffff;
}

void k573dio_device::mpeg_start_adr_low_w(uint16_t data)
{
	LOGMP3("FPGA MPEG start address low %04x\n", data);
	k573fpga->set_mp3_start_addr((k573fpga->get_mp3_start_addr() & 0xffff0000) | data); // low
}

uint16_t k573dio_device::mpeg_end_adr_high_r()
{
	return k573fpga->get_mp3_end_addr() >> 16;
}

void k573dio_device::mpeg_end_adr_high_w(uint16_t data)
{
	LOGMP3("FPGA MPEG end address high %04x\n", data);
	k573fpga->set_mp3_end_addr((k573fpga->get_mp3_end_addr() & 0x0000ffff) | (data << 16)); // high
}

uint16_t k573dio_device::mpeg_end_adr_low_r()
{
	return k573fpga->get_mp3_end_addr() & 0xffff;
}

void k573dio_device::mpeg_end_adr_low_w(uint16_t data)
{
	LOGMP3("FPGA MPEG end address low %04x\n", data);
	k573fpga->set_mp3_end_addr((k573fpga->get_mp3_end_addr() & 0xffff0000) | data); // low
}

uint16_t k573dio_device::mpeg_frame_counter_r()
{
	return k573fpga->get_mp3_frame_count();
}

void k573dio_device::mpeg_key_1_w(uint16_t data)
{
	LOGMP3("FPGA MPEG key 1/3 %04x\n", data);
	crypto_key1 = data;
	k573fpga->set_crypto_key1(data);
}

uint16_t k573dio_device::mas_i2c_r()
{
	return k573fpga->mas_i2c_r();
}

void k573dio_device::mas_i2c_w(uint16_t data)
{
	k573fpga->mas_i2c_w(data);
}

uint16_t k573dio_device::mpeg_ctrl_r()
{
	return k573fpga->get_mpeg_ctrl();
}

uint16_t k573dio_device::fpga_ctrl_r()
{
	return k573fpga->get_fpga_ctrl();
}

void k573dio_device::fpga_ctrl_w(uint16_t data)
{
	k573fpga->set_fpga_ctrl(data);
}

void k573dio_device::ram_write_adr_high_w(uint16_t data)
{
	// read and write address are shared
	ram_adr = ((ram_adr & 0x0000ffff) | (data << 16)) & 0x1ffffff;
}

void k573dio_device::ram_write_adr_low_w(uint16_t data)
{
	// read and write address are shared
	ram_adr = ((ram_adr & 0xffff0000) | data) & 0x1ffffff;
}

uint16_t k573dio_device::ram_r()
{
	uint16_t res = ram[(ram_read_adr & 0x1ffffff) >> 1];
	ram_read_adr += 2;
	return res;
}

void k573dio_device::ram_w(uint16_t data)
{
	ram[(ram_adr & 0x1ffffff) >> 1] = data;
	ram_adr += 2;
}

void k573dio_device::ram_read_adr_high_w(uint16_t data)
{
	// read and write address are shared
	ram_read_adr = ((ram_read_adr & 0x0000ffff) | (data << 16)) & 0x1ffffff;
}

void k573dio_device::ram_read_adr_low_w(uint16_t data)
{
	// read and write address are shared
	ram_read_adr = ((ram_read_adr & 0xffff0000) | data) & 0x1ffffff;
}

uint16_t k573dio_device::mp3_counter_high_r()
{
	return (fpga_counter & 0xffff0000) >> 16;
}

uint16_t k573dio_device::mp3_counter_low_r()
{
	fpga_counter = k573fpga->get_counter();
	return fpga_counter & 0x0000ffff;
}

void k573dio_device::mp3_counter_low_w(uint16_t data)
{
	LOGMP3("mp3_counter_low_w %04x\n", data);
	k573fpga->reset_counter();
}

uint16_t k573dio_device::mp3_counter_diff_r()
{
	return k573fpga->get_counter_diff() & 0x0000ffff;
}

void k573dio_device::output_1_w(uint16_t data)
{
	output(1, data);
}

void k573dio_device::output_0_w(uint16_t data)
{
	output(0, data);
}

void k573dio_device::output_3_w(uint16_t data)
{
	output(3, data);
}

void k573dio_device::output_7_w(uint16_t data)
{
	output(7, data);
}

void k573dio_device::mpeg_key_2_w(uint16_t data)
{
	LOGMP3("FPGA MPEG key 2/3 %04x\n", data);
	k573fpga->set_crypto_key2(data);
}

void k573dio_device::mpeg_key_3_w(uint16_t data)
{
	LOGMP3("FPGA MPEG key 3/3 %04x\n", data);
	k573fpga->set_crypto_key3(data);
}

uint16_t k573dio_device::digital_id_r()
{
	return digital_id->read() << 12;
}

void k573dio_device::digital_id_w(uint16_t data)
{
	digital_id->write( !( ( data >> 12 ) & 1 ) );
}

uint16_t k573dio_device::fpga_status_r()
{
	LOGFPGA("%s: fpga_status_r (%s)\n", tag(), machine().describe_context());

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

void k573dio_device::fpga_firmware_w(uint16_t data)
{
	// Firmware bits in bit 15, always the same firmware
}

void k573dio_device::output_4_w(uint16_t data)
{
	output(4, data);
}

void k573dio_device::output_5_w(uint16_t data)
{
	output(5, data);
}

void k573dio_device::output_2_w(uint16_t data)
{
	output(2, data);
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

uint16_t k573dio_device::network_r()
{
	// Return a byte from the input buffer
	return 0;
}

void k573dio_device::network_w(uint16_t data)
{
	// Write a byte to the output buffer
}

uint16_t k573dio_device::network_output_buf_size_r()
{
	// Number of bytes in the output buffer waiting to be sent
	return 0;
}

uint16_t k573dio_device::network_input_buf_size_r()
{
	// Number of bytes in the input buffer waiting to be read
	return 0;
}

void k573dio_device::network_id_w(uint16_t data)
{
	// The network ID configured in the operator menu
	network_id = data;
}
