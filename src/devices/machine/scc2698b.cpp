// license:GPL-2.0+
// copyright-holders:Brandon Munger, Stephen Stair
/*********************************************************************

	scc2698b.cpp

	Enhanced Octal Universal Asynchronous Receiver/Transmitter

Notes:
	This device is similiar to four 2681 DUART chips tied together
	in a single package, with some shared resources.
	The 2681 DUART is implemented in scn2681_device - but this
	chip is being independently emulated seperately for mainly
	educational purposes. When functionality for this device is
	completed we will consider merging the devices if it's
	practical.

*********************************************************************/


#include "emu.h"
#include "scc2698b.h"

DEFINE_DEVICE_TYPE(SCC2698B, scc2698b_device, "scc2698b", "SCC2698B Octal UART")
DEFINE_DEVICE_TYPE(SCC2698B_CHANNEL, scc2698b_channel, "scc2698b_channel", "UART channel")

#define TRACE_ENABLE 1


#define TRACE_REGISTER_WRITE(ofs, data, reg_name)	if(TRACE_ENABLE) { log_register_access((ofs), (data), "<<", (reg_name)); }
#define TRACE_REGISTER_READ(ofs, data, reg_name)	if(TRACE_ENABLE) { log_register_access((ofs), (data), ">>", (reg_name)); }


// Divider values for baud rate generation
// Expecting a crystal of 3.6864MHz, baud rate is crystal frequency / (divider value * 8)
static const int BAUD_DIVIDER_ACR7_0[16] =
{
	9216,4189,3426,2304,1536,768,384,439,192,96,64,48,12,0,0,0
};
static const int BAUD_DIVIDER_ACR7_1[16] =
{
	6144,4189,12,3072,1536,768,384,230,192,96,256,48,24,0,0,0
};

void scc2698b_device::map(address_map &map)
{
	map(0x0, 0x3F).rw(this, FUNC(scc2698b_device::read), FUNC(scc2698b_device::write));
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
	device_serial_interface(mconfig, *this),
	write_tx(*this),
	write_mpp1(*this),
	write_mpp2(*this),
	write_mpo(*this)
{

}

void scc2698b_channel::device_start()
{
	write_tx.resolve_safe();
	write_mpp1.resolve_safe();
	write_mpp2.resolve_safe();
	write_mpo.resolve_safe();

}

void scc2698b_channel::device_reset()
{
	reset_all();
}

void scc2698b_channel::rcv_complete()
{
	// Completed Byte Receive
	receive_register_extract();
	int byte = get_received_char();
	if (rx_bytecount >= SCC2698B_RX_FIFO_SIZE)
	{
		logerror("Warning: Received byte lost, RX FIFO Full.\n");
	}
	else
	{
		rx_fifo[rx_bytecount++] = byte;
	}
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
}
void scc2698b_channel::tra_callback()
{
	// Started bit transmit - Update output line
	int bit = transmit_register_get_data_bit();
	write_tx(bit);
}

void scc2698b_channel::write_TXH(int txh)
{
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
		return byte;
	}
}


void scc2698b_channel::reset_all()
{
	reset_tx();
	reset_rx();
}
void scc2698b_channel::reset_tx()
{
	tx_transmitting = 0;
	tx_fifo = 0;
	tx_bytecount = 0;
	transmit_register_reset();
}
void scc2698b_channel::reset_rx()
{
	rx_bytecount = 0;
	receive_register_reset();
}


void scc2698b_channel::update_serial_configuration()
{

}



void scc2698b_channel::set_tx_bittime(const attotime &bittime)
{
	set_tra_rate(bittime);
}
void scc2698b_channel::set_rx_bittime(const attotime &bittime)
{
	set_rcv_rate(bittime);
}



WRITE_LINE_MEMBER( scc2698b_channel::mpi0_w )
{

}
WRITE_LINE_MEMBER( scc2698b_channel::mpi1_w )
{

}


scc2698b_device::scc2698b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SCC2698B, tag, owner, clock),
	m_channel_a(*this, CHANA_TAG),
	m_channel_b(*this, CHANB_TAG),
	m_channel_c(*this, CHANC_TAG),
	m_channel_d(*this, CHAND_TAG),
	m_channel_e(*this, CHANE_TAG),
	m_channel_f(*this, CHANF_TAG),
	m_channel_g(*this, CHANG_TAG),
	m_channel_h(*this, CHANH_TAG),
	write_intr_A(*this),
	write_intr_B(*this),
	write_intr_C(*this),
	write_intr_D(*this)
{
}



void scc2698b_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{

}


void scc2698b_device::device_start()
{
	write_intr_A.resolve_safe();
	write_intr_B.resolve_safe();
	write_intr_C.resolve_safe();
	write_intr_D.resolve_safe();
}


void scc2698b_device::device_reset()
{

}

READ8_MEMBER(scc2698b_device::read)
{
	return read_reg(offset);
}

WRITE8_MEMBER(scc2698b_device::write)
{
	write_reg(offset, data);
}


void scc2698b_device::log_register_access(int offset, int value, const char* direction, const char* reg_name)
{
	logerror("[0x%02X] %s 0x%02X (%s)\n", offset, direction, value, reg_name);
}



uint8_t scc2698b_device::read_reg(int offset)
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

void scc2698b_device::write_reg(int offset, uint8_t data)
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

		break;
	case 14: // Reserved
		TRACE_REGISTER_WRITE(offset, data, "Reserved");
		break;
	case 15: // Reserved
		TRACE_REGISTER_WRITE(offset, data, "Reserved");
		break;
	}
}

scc2698b_channel* scc2698b_device::get_channel(int port)
{
	switch (port)
	{
	case 0: return &*m_channel_a;
	case 1: return &*m_channel_b;
	case 2: return &*m_channel_c;
	case 3: return &*m_channel_d;
	case 4: return &*m_channel_e;
	case 5: return &*m_channel_f;
	case 6: return &*m_channel_g;
	case 7: return &*m_channel_h;
	}
	return NULL;
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
	channel->moderegister_ptr = !channel->moderegister_ptr;

	// todo: Change channel serial configuration
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
		logerror("Unimplemented Command Register write");

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
		// Write MR1
		data = channel->MR1;
	}
	else
	{
		// Write MR2
		data = channel->MR2;
	}

	channel->moderegister_ptr = !channel->moderegister_ptr;
	return data;
}
int scc2698b_device::read_SR(int port)
{
	scc2698b_channel* channel = get_channel(port);

	int data = channel->SR;

	// Compute dynamic bits of SR
	data &= 0xF0; // Clear dynamic bits

	// SR(3) TxEMT Transmitter empty
	if (channel->tx_transmitting == 0) data |= 0x08;
	// SR(2) TxRDY Transmitter Ready
	if (channel->tx_bytecount == 0) data |= 0x04;
	// SR(1) FFULL RX Fifo Full
	if (channel->rx_bytecount == SCC2698B_RX_FIFO_SIZE) data |= 0x02;
	// SR(0) RxRDY Receiver Ready (Data has been received)
	if (channel->rx_bytecount > 0) data |= 0x01;

	return data;
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
			return attotime::never;
		}

		return attotime::from_hz(configured_clock()) / (divider * 8);
	}
	else
	{
		// todo: Actually do timer based and more advanced baud rate generation.
		logerror("Warning: Unimplemented baud rate mode");
		return attotime::never;
	}
}




MACHINE_CONFIG_START(scc2698b_device::device_add_mconfig)
	MCFG_DEVICE_ADD(CHANA_TAG, SCC2698B_CHANNEL, 0)
	MCFG_DEVICE_ADD(CHANB_TAG, SCC2698B_CHANNEL, 0)
	MCFG_DEVICE_ADD(CHANC_TAG, SCC2698B_CHANNEL, 0)
	MCFG_DEVICE_ADD(CHAND_TAG, SCC2698B_CHANNEL, 0)
	MCFG_DEVICE_ADD(CHANE_TAG, SCC2698B_CHANNEL, 0)
	MCFG_DEVICE_ADD(CHANF_TAG, SCC2698B_CHANNEL, 0)
	MCFG_DEVICE_ADD(CHANG_TAG, SCC2698B_CHANNEL, 0)
	MCFG_DEVICE_ADD(CHANH_TAG, SCC2698B_CHANNEL, 0)
MACHINE_CONFIG_END
