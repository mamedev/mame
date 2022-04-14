// license:BSD-3-Clause
// copyright-holders:smf, windyfairy
/*
 * Konami 573 Memory Card Reader

  Main PCB Layout
  ---------------
  GE885-PWB(A)A
  (C)1999 KONAMI CO. LTD.
  |---------------------------------------|
  |   PQ30RV11                            |
  |         |----------|  |-------|  CN61 |
  | DIP8    |          |  | XCS05 |       |
  |         |   TMPR   |  | /10   |       |
  |         |  3904AF  |  |-------|  CN62 |
  |         |          |                  |
  | CN65    |----------|                  |
  |        8.25 MHz                  CN67 |
  |                       EP4M16          |
  |             DRAM4M                    |
  |                                       |
  |                               --------|
  | USB-A                    CN64 |
  |                               |
  |          ADM485JR             |
  | USB-B                    CN63 |
  |-------------------------------|

Notes:
    DIP8       - 8-position DIP switch
    CN61       - BS8PSHF1AA 8 pin connector, connects to memory card harness
    CN62       - BS8PSHF1AA 8 pin connector
    CN63       - 6P-SHVQ labeled "0", GE885-JB security dongle is connected here
    CN64       - 6P-SHVQ labeled "1"
    CN65       - B4PS-VH, 4 pin power connector
    CN67       - BS15PSHF1AA, 15-pin connector, unpopulated
    USB-A      - USB-A connector
    USB-B      - USB-B connector, connects to USB on System 573 motherboard
    ADM485JR   - Analog Devices ADM485 low power EIA RS-485 transceiver
    TMPR3904AF - Toshiba TMPR3904AF RISC Microprocessor
    XCS05/10   - XILINX XCS10XL VQ100AKP9909 A2026631A
    DRAM4M     - Silicon Magic 66 MHz C9742 SM81C256K16CJ-35, 256K x 16 EDO DRAM
    EP4M16     - ROM labeled "855-A01"
*/

#include "emu.h"
#include "k573mcr.h"

k573mcr_device::k573mcr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	jvs_device(mconfig, KONAMI_573_MEMORY_CARD_READER, tag, owner, clock),
	m_controllers(*this, "controllers"),
	m_meta(*this, "META")
{
	m_ram = std::make_unique<uint8_t[]>(RAM_SIZE);
}

void k573mcr_device::device_start()
{
	jvs_device::device_start();

	save_pointer(NAME(m_ram), RAM_SIZE);
	save_item(NAME(m_is_memcard_initialized));
	save_item(NAME(m_status));
	save_item(NAME(m_psx_clock));
	save_item(NAME(m_psx_rx_data));
	save_item(NAME(m_psx_rx_bit));
}

void k573mcr_device::device_reset()
{
	jvs_device::device_reset();

	std::fill_n(m_ram.get(), RAM_SIZE, 0);
	m_is_memcard_initialized = false;
	m_status = 0;
	m_psx_clock = false;
	m_psx_rx_data = 0;
	m_psx_rx_bit = 0;
}

void k573mcr_device::device_add_mconfig(machine_config &config)
{
	// The actual controllers are only used in Guitar Freaks and it was
	// meant to be used with the home version PS1 guitar controller.
	// The PS1 guitar controller isn't emulated in MAME and it doesn't
	// make much sense to have it enabled by default. You can still select
	// a controller through the slots menu.
	// The memory card ports are still usable even without a controller
	// enabled which is the main reason for using the PSX controller ports.
	PSXCONTROLLERPORTS(config, m_controllers, 0);
	m_controllers->rxd().set(FUNC(k573mcr_device::write_rxd));
	PSX_CONTROLLER_PORT(config, "port1", psx_controllers, nullptr);
	PSX_CONTROLLER_PORT(config, "port2", psx_controllers, nullptr);
}

const char *k573mcr_device::device_id()
{
	return "KONAMI CO.,LTD.;White I/O;Ver1.0;White I/O PCB";
}

uint8_t k573mcr_device::command_format_version()
{
	return 0x11;
}

uint8_t k573mcr_device::jvs_standard_version()
{
	return 0x20;
}

uint8_t k573mcr_device::comm_method_version()
{
	return 0x10;
}

WRITE_LINE_MEMBER(k573mcr_device::write_rxd)
{
	if (m_psx_clock) {
		if (m_psx_rx_bit == 0) {
			m_psx_rx_data = 0;
		}

		m_psx_rx_data |= state << m_psx_rx_bit;
		m_psx_rx_bit = (m_psx_rx_bit + 1) % 8;
	}
}

void k573mcr_device::controller_set_port(uint32_t port_no)
{
	m_controllers->write_dtr(!!port_no);
	m_controllers->write_dtr(!port_no);
}

uint8_t k573mcr_device::controller_port_send_byte(uint8_t data)
{
	for (int i = 0; i < 8; i++) {
		m_controllers->write_sck(m_psx_clock);
		m_psx_clock = !m_psx_clock;
		m_controllers->write_txd(BIT(data, i));
		m_controllers->write_sck(m_psx_clock);
		m_psx_clock = !m_psx_clock;
	}

	return m_psx_rx_data;
}

bool k573mcr_device::pad_read(uint32_t port_no, uint8_t *output)
{
	controller_set_port(port_no);
	controller_port_send_byte(0x01);
	uint8_t a = controller_port_send_byte('B');
	uint8_t b = controller_port_send_byte(0);
	auto connected = a == 0x41 && b == 0x5a;
	if (connected) {
		*output++ = controller_port_send_byte(0);
		*output++ = controller_port_send_byte(0);
	}
	else {
		*output++ = 0;
		*output++ = 0;
	}

	return connected;
}

bool k573mcr_device::memcard_read(uint32_t port_no, uint16_t block_addr, uint8_t *output)
{
	controller_set_port(port_no);
	controller_port_send_byte(0x81);

	if (controller_port_send_byte('R') == 0xff) { // state_command, Request read
		return false;
	}

	controller_port_send_byte(0); // state_command -> state_cmdack
	controller_port_send_byte(0); // state_cmdack -> state_wait
	controller_port_send_byte(block_addr >> 8); // state_wait -> state_addr_hi
	controller_port_send_byte(block_addr & 0xff); // state_addr_hi -> state_addr_lo

	if (controller_port_send_byte(0) != 0x5c) {  // state_addr_lo -> state_read
		// If the command wasn't correct then it transitions to state_illegal at this point
		return false;
	}

	controller_port_send_byte(0); // Skip 0x5d
	controller_port_send_byte(0); // Skip addr hi
	controller_port_send_byte(0); // Skip addr lo

	for (int i = 0; i < MEMCARD_BLOCK_SIZE; i++) {
		auto c = controller_port_send_byte(0);
		if (output != nullptr) {
			*output++ = c;
		}
	}

	controller_port_send_byte(0);

	return controller_port_send_byte(0) == 'G';
}

bool k573mcr_device::memcard_write(uint32_t port_no, uint16_t block_addr, uint8_t *input)
{
	controller_set_port(port_no);
	controller_port_send_byte(0x81);

	if (controller_port_send_byte('W') == 0xff) { // state_command, Request write
		return false;
	}

	controller_port_send_byte(0); // state_command -> state_cmdack
	controller_port_send_byte(0); // state_cmdack -> state_wait
	controller_port_send_byte(block_addr >> 8); // state_wait -> state_addr_hi
	controller_port_send_byte(block_addr & 0xff); // state_addr_hi -> state_addr_lo

	uint8_t checksum = (block_addr >> 8) ^ (block_addr & 0xff);
	for (int i = 0; i < MEMCARD_BLOCK_SIZE; i++) {
		controller_port_send_byte(input[i]); // state_read
		checksum ^= input[i];
	}

	controller_port_send_byte(checksum);
	controller_port_send_byte(0);
	controller_port_send_byte(0);

	return controller_port_send_byte(0) == 'G';
}

int k573mcr_device::handle_message(const uint8_t *send_buffer, uint32_t send_size, uint8_t *&recv_buffer)
{
	// Notes:
	// 80678::E0:01:06:70:02:01:C0:00:3A: <- Command from game (E0:01:...)
	// 80681::E0:00:03:01:01:05: <- Response from memory card reader device (E0:00:...)
	//
	// The returned value of this function should be 0 (invalid parameters), -1 (unknown command), or the number of bytes in the message.
	// The number of bytes to return is covered by the xx section:
	// 80678::E0:01:yy:xx:xx:xx:xx:xx:3A:
	// This should correspond to yy - 1, but you don't actually get access to yy in the message handler so you must calculate it yourself.
	//
	// recv_buffer will correspond to this part when returning data:
	// 80681::E0:00:03:01:rr:05:
	// In some special cases there is an empty respond but that is not supported in MAME currently so a single byte is returned instead.

	switch(send_buffer[0]) {
		case 0xf0:
			// The bootloader for System 573 games checks for the master calendar which initializes the JVS device.
			// After the bootloader, the actual game's code tries to initialize the JVS device again and (seemingly)
			// expects it to be in a fresh state. Since it was already initialized in the bootloader, it will throw
			// the error message "JVS SUBS RESET ERROR".
			// There might be something else that happens on real hardware between when it loads the bootloader
			// and when it starts the actual game's code that resets the JVS device, but I do not have hands on
			// access to test such a thing.
			// Reset immediately to hack around that error.
			device_reset();
			break;

		case 0x14:
			// Function list returns nothing on
			// 75502::E0:01:02:14:17:
			// 75503::E0:00:04:01:01:00:06:
			*recv_buffer++ = 0x01;
			*recv_buffer++ = 0x00;
			return 1;

		case 0x70:
		{
			int ram_addr = (send_buffer[2] << 16) | (send_buffer[3] << 8) | send_buffer[4];
			int target_len = send_buffer[5];

			if (send_buffer[1] == 0) {
				// Buffer read
				// 39595::E0:01:07:70:00:02:00:00:80:FA:
				// 39596::E0:00:83:01:01:4D:43:00:00:00:00:00:00:00:00:00
				//       :00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00
				//       :00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00
				//       :00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00
				//       :00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00
				//       :00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00
				//       :00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00
				//       :00:00:00:00:00:00:
				*recv_buffer++ = 0x01;

				if (target_len > 0 && ram_addr + target_len < RAM_SIZE) {
					memcpy(recv_buffer, &m_ram[ram_addr], target_len);
					recv_buffer += target_len;
				}

				return 6;
			} else if (send_buffer[1] == 1) {
				// Buffer write
				// 77565::E0:01:87:70:01:02:04:00:80:24:A5:04:70:30:30:7A
				//       :24:87:C6:85:10:00:64:05:58:02:44:45:CA:8C:FD:A2
				//       :00:04:A3:00:04:8C:A6:00:FF:08:8C:00:82:00:20:00
				//       :E8:00:F4:00:DD:F8:C0:24:EF:85:FC:21:20:20:04:5B
				//       :14:40:51:73:77:
				// 77583::E0:00:01:
				if (target_len > 0) {
					memcpy(&m_ram[ram_addr], send_buffer + 6, target_len);
				}

				*recv_buffer++ = 0x01;

				return 6 + target_len;
			} else if (send_buffer[1] == 2) {
				// Set execution address
				// 80678::E0:01:06:70:02:01:C0:00:3A:
				// 80681::E0:00:03:01:01:05:
				*recv_buffer++ = 0x01;

				return 5;
			}

			return -1;
		}

		case 0x71:
		{
			// Status request
			// 81681::E0:01:02:71:74:
			// 81682::E0:00:05:01:01:00:00:07:
			*recv_buffer++ = 0x01;
			*recv_buffer++ = m_status >> 8;
			*recv_buffer++ = m_status & 0xff;

			return 1;
		}

		case 0x72:
		{
			// Security plate
			uint8_t cmd = send_buffer[1] & ~1;
			int sec_slot = send_buffer[1] & 1;

			if (cmd == 0x00) {
				// Packet: e0 01 03 72 00 76 slot 0 (CN63)
				// Packet: e0 01 03 72 01 77 slot 1 (CN64)
				*recv_buffer++ = 0x01;

				return 2;
			} else if (cmd == 0x10) {
				// Set password (presumably to unlock/read security dongle)
				// Packet: e0 01 0b 72 10 a4 60 f0 5d ea c4 5d ec d6
				*recv_buffer++ = 0x01;

				return 10;
			} else if (cmd == 0x20) {
				// Get dongle data? (slot 0: GE885-JB, slot 1: ?)
				// Packet: e0 01 0a 72 20 00 00 02 00 00 00 08 a7
				// It seems that as long as the checksum matches, the game will accept it as valid
				int ram_addr = (send_buffer[4] << 16) | (send_buffer[5] << 8) | send_buffer[6];

				m_ram[ram_addr] = 'J';
				m_ram[ram_addr + 1] = sec_slot ? 'A' : 'B'; // GF uses GE929-JA, but the games will accept anything
				m_ram[ram_addr + 2] = 0x00;
				m_ram[ram_addr + 3] = 0x00;
				m_ram[ram_addr + 4] = ~(m_ram[ram_addr] + m_ram[ram_addr + 1]); // Checksum byte

				*recv_buffer++ = 0x01;

				return 9;
			} else if (cmd == 0x40) {
				// Get some kind of registration info from dongle?
				// Game code calls it "config register"
				// Packet: e0 01 06 72 40 02 00 00 bb
				int ram_addr = (send_buffer[2] << 16) | (send_buffer[3] << 8) | send_buffer[4];

				m_ram[ram_addr] = 0xFF;
				m_ram[ram_addr + 1] = 0xFF;
				m_ram[ram_addr + 2] = 0xAC;
				m_ram[ram_addr + 3] = 0x09;
				m_ram[ram_addr + 4] = 0x00;

				*recv_buffer++ = 0x01;

				return 5;
			}

			return -1;
		}

		case 0x73:
		{
			// Execute previously set address (0x70 0x02)
			// 81674::E0:01:02:73:76:
			// 81675::E0:00:03:01:01:05:
			*recv_buffer++ = 0x01;

			return 1;
		}

		case 0x76:
		{
			// Memory card
			if (send_buffer[1] == 0x74) {
				// Read from card
				// Packet (port 2): e0 01 0a 76 74 80 00 02 00 00 00 01 78
				int memcard_port = send_buffer[2] >> 7;
				int memcard_addr = ((send_buffer[2] << 8) | send_buffer[3]) & 0x7fff;
				int ram_addr = (send_buffer[4] << 16) | (send_buffer[5] << 8) | send_buffer[6];
				int block_count = (send_buffer[7] << 8) | send_buffer[8];
				bool is_ejected = BIT(m_meta->read(), memcard_port); // Forcefully ejected using hotkey

				if (!is_ejected && memcard_read(memcard_port, 0, nullptr)) {
					// Check if card is inserted
					m_status = MEMCARD_AVAILABLE;
				} else {
					m_status = MEMCARD_UNAVAILABLE;
				}

				if (m_status == MEMCARD_AVAILABLE) {
					for (int i = 0; i < block_count; i++) {
						m_status |= MEMCARD_READING;

						if (memcard_read(memcard_port, memcard_addr + i, &m_ram[ram_addr + (i * MEMCARD_BLOCK_SIZE)])) {
							m_status = MEMCARD_AVAILABLE;
						} else {
							m_status = MEMCARD_ERROR;
							break;
						}
					}
				}

				*recv_buffer++ = 0x01;
				*recv_buffer++ = 0x01;

				return 9;
			} else if (send_buffer[1] == 0x75) {
				// Write to card
				// Packet: e0 01 0a 76 75 02 00 00 00 3f 00 01 38
				int ram_addr = (send_buffer[2] << 16) | (send_buffer[3] << 8) | send_buffer[4];
				int memcard_port = send_buffer[5] >> 7;
				int memcard_addr = ((send_buffer[5] << 8) | send_buffer[6]) & 0x7fff;
				int block_count = (send_buffer[7] << 8) | send_buffer[8];
				bool is_ejected = BIT(m_meta->read(), memcard_port); // Forcefully ejected using hotkey

				if (!is_ejected && memcard_read(memcard_port, 0, nullptr)) {
					// Check if card is inserted
					m_status = MEMCARD_AVAILABLE;
				} else {
					m_status = MEMCARD_UNAVAILABLE;
				}

				if (m_status == MEMCARD_AVAILABLE && m_is_memcard_initialized) {
					for (int i = 0; i < block_count; i++) {
						m_status |= MEMCARD_WRITING;

						if (memcard_write(memcard_port, memcard_addr + i, &m_ram[ram_addr + (i * MEMCARD_BLOCK_SIZE)])) {
							m_status = MEMCARD_AVAILABLE;
						} else {
							m_status = MEMCARD_ERROR;
							break;
						}
					}
				}

				*recv_buffer++ = 0x01;
				*recv_buffer++ = 0x01;

				// The game will write a block of 0xffs to block 3f immediately after inserting a memory card for the first time.
				// I believe it's some kind of initialization, and based on the game's code, the 0xff buffer is prepared when the firmware is being sent.
				// Based on someone else's research with real hardware, the 0xff block doesn't actually get written to the memory card.
				m_is_memcard_initialized = true;

				return 9;
			}

			return -1;
		}

		case 0x77:
		{
			// Controller ports
			// Packet: e0 01 02 77 7a
			//
			// Only used by Guitar Freaks.
			// This was introduced in Guitar Freaks 2nd Mix Link Kit 2, which allowed players to bring their own PS1 guitar controllers to the arcade
			// and plug it into the machine to use as a controller instead of the normal machine's guitar controllers.

			*recv_buffer++ = 0x01;
			pad_read(0, recv_buffer);
			pad_read(1, recv_buffer + 2);

			recv_buffer += 4;

			return 1;
		}
	}

	// Command not recognized, pass it off to the base message handler
	return jvs_device::handle_message(send_buffer, send_size, recv_buffer);
}

ROM_START( k573mcr )
	ROM_REGION( 0x080000, "tmpr3904", 0 )
	ROM_LOAD( "885a01.bin",   0x000000, 0x080000, CRC(e22d093f) SHA1(927f62f63b5caa7899392decacd12fea0e6fdbea) )
ROM_END

const tiny_rom_entry *k573mcr_device::device_rom_region() const
{
	return ROM_NAME( k573mcr );
}


INPUT_PORTS_START( k573mcr_meta_controls )
	PORT_START("META")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1) PORT_TOGGLE PORT_NAME("Insert/Eject Memory Card 1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE2) PORT_TOGGLE PORT_NAME("Insert/Eject Memory Card 2")
INPUT_PORTS_END

ioport_constructor k573mcr_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(k573mcr_meta_controls);
}

DEFINE_DEVICE_TYPE(KONAMI_573_MEMORY_CARD_READER, k573mcr_device, "k573mcr", "Konami 573 Memory Card Reader")
