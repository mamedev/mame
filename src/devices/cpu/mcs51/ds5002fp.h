// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff, Manuel Abadia, Couriersud

// Dallas DS5002FP

#ifndef MAME_CPU_MCS51_DS5002FP_H
#define MAME_CPU_MCS51_DS5002FP_H

#include "i8051.h"

DECLARE_DEVICE_TYPE(DS5002FP, ds5002fp_device)

/*
 * The DS5002FP has 2 16 bits data address buses (the byte-wide bus and the expanded bus). The exact memory position accessed depends on the
 * partition mode, the memory range and the expanded bus select. The partition mode and the expanded bus select can be changed at any time.
 *
 * In order to simplify memory mapping to the data address bus, the following address map is assumed for partitioned mode:

 * 0x00000-0x0ffff -> data memory on the expanded bus
 * 0x10000-0x1ffff -> data memory on the byte-wide bus

 * For non-partitioned mode the following memory map is assumed:

 * 0x0000-0xffff -> data memory (the bus used to access it does not matter)
 *
 * Internal ram 128k and security features
 */

// these allow the default state of RAM to be set from a region
#define DS5002FP_SET_MON( _mcon) \
	ROM_FILL( 0xc6, 1, _mcon)

#define DS5002FP_SET_RPCTL( _rpctl) \
	ROM_FILL( 0xd8, 1, _rpctl)

#define DS5002FP_SET_CRCR( _crcr) \
	ROM_FILL( 0xc1, 1, _crcr)


class ds5002fp_device : public mcs51_cpu_device, public device_nvram_interface
{
public:
	// construction/destruction
	ds5002fp_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read( util::read_stream &file ) override;
	virtual bool nvram_write( util::write_stream &file ) override;

protected:
	enum {
		MCON_PA  = 4,
		MCON_RG1 = 3,
		MCON_PES = 2,
		MCON_PM  = 1,
		MCON_SL  = 0,
	};

	enum {
		RPCTL_RNR   = 7,  // Bit 6 ??
		RPCTL_EXBS  = 5,
		RPCTL_AE    = 4,
		RPCTL_IBI   = 3,
		RPCTL_DMA   = 2,
		RPCTL_RPCON = 1,
		RPCTL_RG0   = 0,
	};

	u8 m_previous_ta;   // Previous Timed Access value
	u8 m_ta_window;     // Limed Access window
	u8 m_range;         // Memory Range

	s32 m_rnr_delay;    // delay before new random number available

	u16 m_crc;
	u8 m_crcr, m_mcon, m_ta, m_rnr, m_rpctl, m_rps;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual offs_t external_ram_iaddr(offs_t offset, offs_t mem_mask) override;
	virtual void irqs_complete_and_mask(u8 &ints, u8 int_mask) override;
	virtual void handle_ta_window() override;
	virtual bool manage_idle_on_interrupt(u8 ints) override;
	virtual void handle_irq(int irqline, int state, u32 new_state, u32 tr_state) override;
	virtual void sfr_map(address_map &map) override ATTR_COLD;

	u8 pcon_ds_r();
	void pcon_ds_w(u8 data);
	void ip_ds_w(u8 data);
	u8 crcr_r();
	void crcr_w(u8 data);
	u8 crc_r(offs_t offset);
	void crc_w(offs_t offset, u8 data);
	u8 mcon_r();
	void mcon_w(u8 data);
	u8 ta_r();
	void ta_w(u8 data);
	u8 rnr_r();
	void rnr_w(u8 data);
	u8 rpctl_r();
	void rpctl_w(u8 data);
	u8 rps_r();
	void rps_w(u8 data);

	u8 handle_rnr();
	void ds_protected(u8 &val, u8 data, u8 ta_mask, u8 mask);

private:
	optional_memory_region m_region;
};


#endif
