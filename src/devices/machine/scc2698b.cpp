// license:GPL-2.0+
// copyright-holders:Brandon Munger, Stephen Stair
/*********************************************************************

    scc2698b.cpp

    Enhanced Octal Universal Asynchronous Receiver/Transmitter

Notes:
    This device is similar to four 2681 DUART chips tied together
    in a single package, with some shared resources.
    The 2681 DUART is implemented in scn2681_device - but this
    chip is being independently emulated separately for mainly
    educational purposes. When functionality for this device is
    completed we will consider merging the devices if it's
    practical.

Quirks:
    * Reading the RX Holding register will advance the HW FIFO even if
      there is no data to be read. This is not currently emulated but
      might be interesting to characterize in HW and emulate properly.

*********************************************************************/


#include "emu.h"
#include "scc2698b.h"

#define LOG_GENERAL (1U << 0)
#define LOG_CONFIG_CHANGE (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_CONFIG_CHANGE)
#include "logmacro.h"

#define TRACE_REGISTER_WRITE(ofs, data, reg_name)   LOG("[0x%02X] << 0x%02X (%s)\n", (ofs), (data), (reg_name));
#define TRACE_REGISTER_READ(ofs, data, reg_name)    LOG("[0x%02X] >> 0x%02X (%s)\n", (ofs), (data), (reg_name));
#define TRACE_CONFIG(...)                           LOGMASKED(LOG_CONFIG_CHANGE, __VA_ARGS__)


DEFINE_DEVICE_TYPE(SCC2698B, scc2698b_device, "scc2698b", "SCC2698B Octal UART")
DEFINE_DEVICE_TYPE(SCC2698B_CHANNEL, scc2698b_channel, "scc2698b_channel", "UART channel")


// Divider values for baud rate generation
// Expecting a crystal of 3.6864MHz, baud rate is crystal frequency / (divider value * 8)
static constexpr int BAUD_DIVIDER_ACR7_0[16] =
{
	9216,4189,3426,2304,1536,768,384,439,192,96,64,48,12,0,0,0
};
static constexpr int BAUD_DIVIDER_ACR7_1[16] =
{
	6144,4189,12,3072,1536,768,384,230,192,96,256,48,24,0,0,0
};

void scc2698b_device::map(address_map &map)
{
	map(0x0, 0x3F).rw(FUNC(scc2698b_device::read), FUNC(scc2698b_device::write));
}

#define CHANA_TAG   "cha"
#define CHANB_TAG   "chb"
#define CHANC_TAG   "chc"
#define CHAND_TAG   "chd"
#define CHANE_TAG   "che"
#define CHANF_TAG   "chf"
#define CHANG_TAG   "chg"
#define CHANH_TAG   "chh"


scc2698b_channel::scc2698b_channel(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SCC2698B_CHANNEL, tag, owner, clock),
	device_serial_interface(mconfig, *this)
{

}

void scc2698b_channel::device_start()
{

}

void scc2698b_channel::device_reset()
{
	reset_all();
	recompute_pin_output(true);
}

void scc2698b_channel::rcv_complete()
{
	// Completed Byte Receive
	receive_register_extract();

	if (!rx_enable)
	{
		// Skip receive
		return;
	}

	int byte = get_received_char();
	if (rx_bytecount >= SCC2698B_RX_FIFO_SIZE)
	{
		logerror("Warning: Received byte lost, RX FIFO Full.\n");
	}
	else
	{
		rx_fifo[rx_bytecount++] = byte;
	}
	recompute_pin_output();
}
void scc2698b_channel::tra_complete()
{
	// Completed Byte Transmit
	if (tx_bytecount > 0)
	{
		transmit_register_setup(tx_fifo);
		tx_bytecount = 0;
	}
	else
	{
		tx_transmitting = 0;
	}
	recompute_pin_output();
}
void scc2698b_channel::tra_callback()
{
	// Started bit transmit - Update output line
	int bit = transmit_register_get_data_bit();
	parent->write_line_tx(channel_port, bit);
}

void scc2698b_channel::write_TXH(int txh)
{
	if (!tx_enable)
	{
		logerror("Warning: TX Holding byte ignored because transmitter is disabled.\n");
		return;
	}
	if (tx_transmitting)
	{
		if (tx_bytecount == 0)
		{
			tx_fifo = txh;
			tx_bytecount = 1;
		}
		else
		{
			// Lost byte
			logerror("Warning: TX Holding byte written to full FIFO - Data lost.\n");
		}
	}
	else
	{
		if (tx_bytecount != 0)
		{
			logerror("Unexpected: TX FIFO should not contain bytes when UART is not transmitting\n");
		}
		transmit_register_setup(txh);
		tx_transmitting = 1;
	}
	recompute_pin_output();
}

int scc2698b_channel::read_RXH()
{
	if (rx_bytecount == 0)
	{
		logerror("Warning: RX Holding read with empty RX FIFO");
		return 0;
	}
	else
	{
		int byte = rx_fifo[0];
		for (int i = 1; i < rx_bytecount; i++)
		{
			rx_fifo[i - 1] = rx_fifo[i];
		}
		rx_bytecount--;
		recompute_pin_output();
		return byte;
	}
}


void scc2698b_channel::reset_all()
{
	mpp1_value = -1; // Force pin update
	mpp2_value = -1;
	reset_tx();
	reset_rx();
}
void scc2698b_channel::reset_tx()
{
	tx_transmitting = 0;
	tx_fifo = 0;
	tx_bytecount = 0;
	transmit_register_reset();
	set_tx_enable(false);
}
void scc2698b_channel::reset_rx()
{
	rx_bytecount = 0;
	receive_register_reset();
	set_rx_enable(false);
}

void scc2698b_channel::set_tx_enable(bool enable)
{
	tx_enable = enable;
	recompute_pin_output();
}
void scc2698b_channel::set_rx_enable(bool enable)
{
	if (!rx_enable && enable)
	{
		receive_register_reset();
	}
	rx_enable = enable;
	recompute_pin_output();
}


void scc2698b_channel::update_serial_configuration()
{
	// void set_data_frame(int start_bit_count, int data_bit_count, parity_t parity, stop_bits_t stop_bits);

	// Note: unimplemented RTS/CTS control, Error mode bit.

	int start_bit_count = 1;

	int data_bit_count = (MR1 & 3) + 5;

	parity_t parity_mode = PARITY_NONE;
	switch ((MR1 >> 3) & 3)
	{
	case 0: // Parity enabled
		parity_mode = (MR1 & 4) ? PARITY_ODD : PARITY_EVEN;
		break;
	case 1: // Force parity (force parity bit to a constant)
		parity_mode = (MR1 & 4) ? PARITY_MARK : PARITY_SPACE;
		break;
	case 2: // No parity
		parity_mode = PARITY_NONE;
		break;
	default:
		logerror("Warning: Unsupported special parity mode selected.\n");
		break;
	}

	stop_bits_t stop_bits;
	int stopbitlength = MR2 & 0x0F;
	// Round up to the next highest even bit length.
	// In reality there are a variety of options.
	if (stopbitlength < 8)
	{
		stop_bits = STOP_BITS_1;
	}
	else
	{
		stop_bits = STOP_BITS_2;
	}

	// Check the channel mode
	switch ((MR2 >> 6) & 3)
	{
	case 0: // Normal
		break;
	case 1: // Auto-Echo (receive, and repeat RX line to TX)
	case 2: // Local loopback (TX=1, RX ignored, receive bytes written to TX through internal loopback)
	case 3: // Remote loop (No receiving, but repeat RX line to TX)
		logerror("Warning: Unsupported channel mode selected.\n");
	}

	static char const *const parity_strings[] = { "None", "Odd", "Even", "Mark", "Space" };
	static char const *const stop_bit_strings[] = { "0","1","1.5","2" };
	TRACE_CONFIG("Reconfigured channel to %d data bits, %s Parity, %s stop bits\n", data_bit_count, parity_strings[parity_mode], stop_bit_strings[stop_bits]);

	set_data_frame(start_bit_count, data_bit_count, parity_mode, stop_bits);
}



void scc2698b_channel::set_tx_bittime(const attotime &bittime)
{
	set_tra_rate(bittime);
}
void scc2698b_channel::set_rx_bittime(const attotime &bittime)
{
	set_rcv_rate(bittime);
}


int scc2698b_channel::read_SR()
{
	int data = SR;

	// Compute dynamic bits of SR
	data &= 0xF0; // Clear dynamic bits

	// SR(3) TxEMT Transmitter empty
	if (tx_enable && tx_transmitting == 0) data |= 0x08;
	// SR(2) TxRDY Transmitter Ready
	if (tx_enable && tx_bytecount == 0) data |= 0x04;
	// SR(1) FFULL RX Fifo Full
	if (rx_bytecount == SCC2698B_RX_FIFO_SIZE) data |= 0x02;
	// SR(0) RxRDY Receiver Ready (Data has been received)
	if (rx_bytecount > 0) data |= 0x01;

	return data;
}

void scc2698b_channel::set_mpp_output(bool output)
{
	mpp_is_output = output;
	recompute_pin_output(true);
}

void scc2698b_channel::recompute_pin_output(bool force)
{
	int new_mpp1 = 0, new_mpp2 = 0;
	int SR = read_SR();
	if (mpp_is_output)
	{
		// TxRDY
		new_mpp1 = (SR & 4) ? 1 : 0;
		// RxRDY
		new_mpp2 = (SR & 1) ? 1 : 0;

		if (new_mpp1 != mpp1_value || force)
		{
			LOG("Channel %d MPP1 => %d\n", channel_port, new_mpp1);
			parent->write_line_mpp1(channel_port, new_mpp1);
			mpp1_value = new_mpp1;
		}
		if (new_mpp2 != mpp2_value || force)
		{
			LOG("Channel %d MPP2 => %d\n", channel_port, new_mpp2);
			parent->write_line_mpp2(channel_port, new_mpp2);
			mpp2_value = new_mpp2;
		}
	}
}



void scc2698b_channel::mpi0_w(int state)
{

}
void scc2698b_channel::mpi1_w(int state)
{

}


scc2698b_device::scc2698b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SCC2698B, tag, owner, clock),
	m_channel(*this, "channel_%u",1),
	write_intr_A(*this),
	write_intr_B(*this),
	write_intr_C(*this),
	write_intr_D(*this),
	write_tx(*this),
	write_mpp1(*this),
	write_mpp2(*this),
	write_mpo(*this)
{

}



void scc2698b_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{

}


void scc2698b_device::device_start()
{
	write_intr_A.resolve_safe();
	write_intr_B.resolve_safe();
	write_intr_C.resolve_safe();
	write_intr_D.resolve_safe();

	write_tx.resolve_all_safe();
	write_mpp1.resolve_all_safe();
	write_mpp2.resolve_all_safe();
	write_mpo.resolve_all_safe();

	for (int i = 0; i < 8; i++)
	{
		m_channel[i]->channel_port = i;
		m_channel[i]->parent = this;
	}

}


void scc2698b_device::device_reset()
{

}


void scc2698b_device::write_line_tx(int port, int value)
{
	if ((0 <= port) && (std::size(write_tx) > port))
		write_tx[port](value);
	else
		logerror("Unsupported port %d in write_line_tx\n", port);
}

void scc2698b_device::write_line_mpp1(int port, int value)
{
	if ((0 <= port) && (std::size(write_mpp1) > port))
		write_mpp1[port](value);
	else
		logerror("Unsupported port %d in write_line_mpp1\n", port);
}

void scc2698b_device::write_line_mpp2(int port, int value)
{
	if ((0 <= port) && (std::size(write_mpp2) > port))
		write_mpp2[port](value);
	else
		logerror("Unsupported port %d in write_line_mpp2\n", port);
}

void scc2698b_device::write_line_mpo(int port, int value)
{
	if ((0 <= port) && (std::size(write_mpo) > port))
		write_mpo[port](value);
	else
		logerror("Unsupported port %d in write_line_mpo\n", port);
}

uint8_t scc2698b_device::read(offs_t offset)
{
	int device = (offset >> 4) & 3;
	int reg = (offset & 15);
	// Port index for port register accesses
	int port = device * 2 + (reg >> 3);
	int data = 0;

	switch (reg)
	{
	case 0: // MR1a, MR2a (Mode Register)
		data = read_MR(port);
		TRACE_REGISTER_READ(offset, data, "MR1a/MR2a");
		break;
	case 1: // SRa (Status Register)
		data = read_SR(port);
		TRACE_REGISTER_READ(offset, data, "SRa");
		break;
	case 2: // BRG Test
		data = 0;
		TRACE_REGISTER_READ(offset, data, "BRG Test");
		break;
	case 3: // RHRa (Receive Holding Register)
		data = read_RHR(port);
		TRACE_REGISTER_READ(offset, data, "RHRa");
		break;

	case 8: // MR1b, MR2b (Mode Register)
		data = read_MR(port);
		TRACE_REGISTER_READ(offset, data, "MR1b/MR2b");
		break;
	case 9: // SRb (Status Register)
		data = read_SR(port);
		TRACE_REGISTER_READ(offset, data, "SRb");
		break;
	case 10: // 1X/16X Test
		data = 0;
		TRACE_REGISTER_READ(offset, data, "1X/16X Test");
		break;
	case 11: // RHRb (Receive Holding Register)
		data = read_RHR(port);
		TRACE_REGISTER_READ(offset, data, "RHRb");
		break;

	default:
		TRACE_REGISTER_READ(offset, data, "");
	}
	return data;
}

void scc2698b_device::write(offs_t offset, u8 data)
{
	int device = (offset >> 4) & 3;
	int reg = (offset & 15);
	// Port index for port register accesses
	int port = device * 2 + (reg >> 3);

	switch (reg)
	{
	case 0: // MR1a, MR2a (Mode Register)
		TRACE_REGISTER_WRITE(offset, data, "MR1a/MR2a");
		write_MR(port, data);
		break;
	case 1: // CSRa (Clock Select Register)
		TRACE_REGISTER_WRITE(offset, data, "CSRa");
		write_CSR(port, data);
		break;
	case 2: // CRa (Command Register)
		TRACE_REGISTER_WRITE(offset, data, "CRa");
		write_CR(port, data);
		break;
	case 3: // THRa (Transmit Holding Register)
		TRACE_REGISTER_WRITE(offset, data, "THRa");
		write_THR(port, data);
		break;
	case 4: // ACRA
		TRACE_REGISTER_WRITE(offset, data, "ACRA");
		m_blocks[device].ACR = data;
		update_block_baudrate(device);
		break;
	case 5: // IMRA
		TRACE_REGISTER_WRITE(offset, data, "IMRA");

		break;
	case 6: // CTPUA
		TRACE_REGISTER_WRITE(offset, data, "CTPUA");

		break;
	case 7: // CTPLA
		TRACE_REGISTER_WRITE(offset, data, "CTPLA");

		break;
	case 8: // MR1b, MR2b (Mode Register)
		TRACE_REGISTER_WRITE(offset, data, "MR1b/MR2b");
		write_MR(port, data);
		break;
	case 9: // CSRb (Clock Select Register)
		TRACE_REGISTER_WRITE(offset, data, "CSRb");
		write_CSR(port, data);
		break;
	case 10: // CRb (Command Register)
		TRACE_REGISTER_WRITE(offset, data, "CRb");
		write_CR(port, data);
		break;
	case 11: // THRb (Transmit Holding Register)
		TRACE_REGISTER_WRITE(offset, data, "THRb");
		write_THR(port, data);
		break;
	case 12: // Reserved
		TRACE_REGISTER_WRITE(offset, data, "Reserved");
		break;
	case 13: // OPCRA
		TRACE_REGISTER_WRITE(offset, data, "OPCRA");
		// Set the MPP pin input/ouput based on OPCR bit 7
		{
			scc2698b_channel* channel = get_channel(device * 2);
			channel->set_mpp_output(!!(data & 0x80));
			channel = get_channel(device * 2 + 1);
			channel->set_mpp_output(!!(data & 0x80));
		}
		break;
	case 14: // Reserved
		TRACE_REGISTER_WRITE(offset, data, "Reserved");
		break;
	case 15: // Reserved
		TRACE_REGISTER_WRITE(offset, data, "Reserved");
		break;
	}
}

void scc2698b_device::port_a_rx_w(int state) { m_channel[0]->rx_w(state); }
void scc2698b_device::port_b_rx_w(int state) { m_channel[1]->rx_w(state); }
void scc2698b_device::port_c_rx_w(int state) { m_channel[2]->rx_w(state); }
void scc2698b_device::port_d_rx_w(int state) { m_channel[3]->rx_w(state); }
void scc2698b_device::port_e_rx_w(int state) { m_channel[4]->rx_w(state); }
void scc2698b_device::port_f_rx_w(int state) { m_channel[5]->rx_w(state); }
void scc2698b_device::port_g_rx_w(int state) { m_channel[6]->rx_w(state); }
void scc2698b_device::port_h_rx_w(int state) { m_channel[7]->rx_w(state); }


scc2698b_channel* scc2698b_device::get_channel(int port)
{
	return &*m_channel[port];
}

void scc2698b_device::reset_port(int port)
{
	reset_port_tx(port);
	reset_port_rx(port);

	scc2698b_channel* channel = get_channel(port);
	channel->moderegister_ptr = 0;
}
void scc2698b_device::reset_port_tx(int port)
{
	scc2698b_channel* channel = get_channel(port);
	channel->reset_tx();
}
void scc2698b_device::reset_port_rx(int port)
{
	scc2698b_channel* channel = get_channel(port);
	channel->reset_rx();
}


void scc2698b_device::write_MR(int port, int value)
{
	scc2698b_channel* channel = get_channel(port);
	if (channel->moderegister_ptr == 0)
	{
		// Write MR1
		channel->MR1 = value;
	}
	else
	{
		// Write MR2
		channel->MR2 = value;
	}
	channel->moderegister_ptr = 1;

	channel->update_serial_configuration();
}
void scc2698b_device::write_CSR(int port, int value)
{
	scc2698b_channel* channel = get_channel(port);

	channel->CSR = value;
	update_port_baudrate(port);
}
void scc2698b_device::write_CR(int port, int value)
{
	scc2698b_channel* channel = get_channel(port);
	// Todo: enable/disable TX/RX
	if (value & 1) // Enable RX
	{
		channel->set_rx_enable(true);
	}
	if (value & 2) // Disable RX
	{
		channel->set_rx_enable(false);
	}
	if (value & 4) // Enable TX
	{
		channel->set_tx_enable(true);
	}
	if (value & 8) // Disable TX
	{
		channel->set_tx_enable(false);
	}


	switch (value >> 4)
	{
	case 0: // NOP
		break;
	case 1: // Reset MR Pointer
		channel->moderegister_ptr = 0;
		break;
	case 2: // Reset Receiver
		reset_port_rx(port);
		break;
	case 3: // Reset Transmitter
		reset_port_tx(port);
		break;
	case 4: // Reset status register error bits
		channel->SR &= 0x0F;
		break;
	default:
		logerror("Unimplemented Command Register write\n");

	}
}
void scc2698b_device::write_THR(int port, int value)
{
	scc2698b_channel* channel = get_channel(port);
	channel->write_TXH(value);
}

int scc2698b_device::read_MR(int port)
{
	scc2698b_channel* channel = get_channel(port);
	int data = 0;
	if (channel->moderegister_ptr == 0)
	{
		// Read MR1
		data = channel->MR1;
	}
	else
	{
		// Read MR2
		data = channel->MR2;
	}

	channel->moderegister_ptr = 1;
	return data;
}
int scc2698b_device::read_SR(int port)
{
	scc2698b_channel* channel = get_channel(port);
	return channel->read_SR();
}
int scc2698b_device::read_RHR(int port)
{
	scc2698b_channel* channel = get_channel(port);
	return channel->read_RXH();
}

void scc2698b_device::update_block_baudrate(int block)
{
	update_port_baudrate(block * 2);
	update_port_baudrate(block * 2 + 1);
}

void scc2698b_device::update_port_baudrate(int port)
{
	scc2698b_channel* channel = get_channel(port);
	channel->set_tx_bittime(generate_baudrate(port / 2, 1, channel->CSR & 15));
	channel->set_rx_bittime(generate_baudrate(port / 2, 0, channel->CSR >> 4));
}

attotime scc2698b_device::generate_baudrate(int block, int tx, int table_index)
{
	if (table_index < 13)
	{
		// Table based bit time calculation
		int divider = 0;
		if (m_blocks[block].ACR & 0x80)
		{
			divider = BAUD_DIVIDER_ACR7_1[table_index];
		}
		else
		{
			divider = BAUD_DIVIDER_ACR7_0[table_index];
		}

		if (divider == 0)
		{
			logerror("Unimplemented baud rate selected. (%d)\n", table_index);
			return attotime::never;
		}

		int frequency = configured_clock() / (divider * 8);
		TRACE_CONFIG("Set %s baud rate to %dHz (clock divider = %d)\n", tx ? "Transmit" : "Receive", frequency, divider * 8);

		return attotime::from_hz(configured_clock()) * (divider * 8);
	}
	else
	{
		// todo: Actually do timer based and more advanced baud rate generation.
		logerror("Warning: Unimplemented baud rate mode");
		return attotime::never;
	}
}




void scc2698b_device::device_add_mconfig(machine_config &config)
{
	for (required_device<scc2698b_channel> &channel : m_channel)
		SCC2698B_CHANNEL(config, channel, 0);
}
