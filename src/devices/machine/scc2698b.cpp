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


DEVICE_ADDRESS_MAP_START(map, 8, scc2698b_device)
AM_RANGE(0x0, 0x3F) AM_READWRITE(read, write)
ADDRESS_MAP_END

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
	uint8_t data = 0;


	TRACE_REGISTER_READ(offset, data, "");
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

}
void scc2698b_device::reset_port_rx(int port)
{

}


void scc2698b_device::write_MR(int port, int value)
{
	scc2698b_channel* channel = get_channel(port);
	if (channel->moderegister_ptr == 0)
	{
		// Write MR1
	}
	else
	{
		// Write MR2
	}

	channel->moderegister_ptr = !channel->moderegister_ptr;
}
void scc2698b_device::write_CSR(int port, int value)
{

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

	default:
		logerror("Unimplemented Command Register write");

	}
}
void scc2698b_device::write_THR(int port, int value)
{

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
