// license:BSD-3-Clause
// copyright-holders:Michael Zapf
/*
    HDC9234 Hard and Floppy Disk Controller
    For details see hdc9234.c
*/
#ifndef __HDC9234_H__
#define __HDC9234_H__

#include "emu.h"
#include "imagedev/floppy.h"

extern const device_type HDC9234;

/*
    Enumeration of the latches outside of the controller
*/
enum
{
	HDC_INPUT_STATUS    = 0x00,
	HDC_OUTPUT_DMA_ADDR = 0x01,
	HDC_OUTPUT_1        = 0x02,
	HDC_OUTPUT_2        = 0x03
};


/*
    Definition of bits in the Disk-Status register
*/
enum
{
	HDC_DS_ECCERR  = 0x80,        // ECC error
	HDC_DS_INDEX   = 0x40,        // index hole
	HDC_DS_SKCOM   = 0x20,        // seek complete
	HDC_DS_TRK00   = 0x10,        // track 0
	HDC_DS_UDEF    = 0x08,        // user-defined
	HDC_DS_WRPROT  = 0x04,        // write-protected
	HDC_DS_READY   = 0x02,        // drive ready bit
	HDC_DS_WRFAULT = 0x01         // write fault
};

//===================================================================

/* Interrupt line. To be connected with the controller PCB. */
#define MCFG_HDC9234_INTRQ_CALLBACK(_write) \
	devcb = &hdc9234_device::set_intrq_wr_callback(*device, DEVCB_##_write);

/* DMA in progress line. To be connected with the controller PCB. */
#define MCFG_HDC9234_DIP_CALLBACK(_write) \
	devcb = &hdc9234_device::set_dip_wr_callback(*device, DEVCB_##_write);

/* Auxiliary Bus. These 8 lines need to be connected to external latches
   and to a counter circuitry which works together with the external RAM.
   We use the S0/S1 lines as address lines. */
#define MCFG_HDC9234_AUXBUS_OUT_CALLBACK(_write) \
	devcb = &hdc9234_device::set_auxbus_wr_callback(*device, DEVCB_##_write);

/* Auxiliary Bus. This is only used for S0=S1=0. */
#define MCFG_HDC9234_AUXBUS_IN_CALLBACK(_read) \
	devcb = &hdc9234_device::set_auxbus_rd_callback(*device, DEVCB_##_read);

/* Callback to read the contents of the external RAM via the data bus.
   Note that the address must be set and automatically increased
   by external circuitry. */
#define MCFG_HDC9234_DMA_IN_CALLBACK(_read) \
	devcb = &hdc9234_device::set_dma_rd_callback(*device, DEVCB_##_read);

/* Callback to write the contents of the external RAM via the data bus.
   Note that the address must be set and automatically increased
   by external circuitry. */
#define MCFG_HDC9234_DMA_OUT_CALLBACK(_write) \
	devcb = &hdc9234_device::set_dma_wr_callback(*device, DEVCB_##_write);

//===================================================================

class hdc9234_device : public device_t
{
public:
	hdc9234_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// Accesors from the CPU side
	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_WRITE_LINE_MEMBER( reset );

	// Callbacks
	template<class _Object> static devcb_base &set_intrq_wr_callback(device_t &device, _Object object) { return downcast<hdc9234_device &>(device).m_out_intrq.set_callback(object); }
	template<class _Object> static devcb_base &set_dip_wr_callback(device_t &device, _Object object) { return downcast<hdc9234_device &>(device).m_out_dip.set_callback(object); }
	template<class _Object> static devcb_base &set_auxbus_wr_callback(device_t &device, _Object object) { return downcast<hdc9234_device &>(device).m_out_auxbus.set_callback(object); }
	template<class _Object> static devcb_base &set_auxbus_rd_callback(device_t &device, _Object object) { return downcast<hdc9234_device &>(device).m_in_auxbus.set_callback(object); }
	template<class _Object> static devcb_base &set_dma_rd_callback(device_t &device, _Object object) { return downcast<hdc9234_device &>(device).m_in_dma.set_callback(object); }
	template<class _Object> static devcb_base &set_dma_wr_callback(device_t &device, _Object object) { return downcast<hdc9234_device &>(device).m_out_dma.set_callback(object); }

	// Used to reconfigure the drive connections. Floppy drive selection is done
	// using the user-programmable outputs. Hence, the connection
	// is changed outside of the controller, and by this way we let it know.
	void connect_floppy_drive(floppy_image_device *floppy);

protected:
	void device_start();
	void device_reset();

private:
	devcb_write_line   m_out_intrq;    // INT line
	devcb_write_line   m_out_dip;      // DMA in progress line
	devcb_write8       m_out_auxbus;   // AB0-7 lines (using S0,S1 as address)
	devcb_read8        m_in_auxbus;    // AB0-7 lines (S0=S1=0)
	devcb_read8        m_in_dma;       // DMA read access to the cache buffer
	devcb_write8       m_out_dma;      // DMA write access to the cache buffer

	// Internal register pointer used for sequential register loading
	int m_register_pointer;

	// Read and write registers
	UINT8 m_register_r[12];
	UINT8 m_register_w[12];

	// Command processing
	void  process_command(UINT8 opcode);

	// Recent command.
	UINT8 m_command;

	// Interrupt management (outgoing INT pin)
	void set_interrupt(line_state intr);

	// Currently connected floppy
	floppy_image_device* m_floppy;

	// internal register OUTPUT1
	UINT8 m_output1;

	// internal register OUTPUT2
	UINT8 m_output2;

	// Sample the values of the incoming status bits
	void sync_status_in();

	// Write the output registers to the latches
	void sync_latches_out();

	// Utility routine to set or reset bits
	void set_bits(UINT8& byte, int mask, bool set);

	// Drive type that has been selected in drive_select
	int m_selected_drive_type;

	// Enable head load delays
	bool m_head_load_delay_enable;

	// ===================================================
	//   Commands
	// ===================================================
	void drive_select(int driveparm);
};

#endif
