// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA Cosmac VIP Expansion Interface emulation

**********************************************************************

                  CLOCK       1      A       _MWR
                   _EF4       2      B       TPA
                   _EF3       3      C       MA0
                   XTAL       4      D       MA1
                   _EF1       5      E       MA2
                     N0       6      F       MA3
                     N1       7      H       MA4
                     N2       8      J       MA5
                   SPOT       9      K       MA6
                  _SYNC      10      L       MA7
                    TPB      11      M       BUS 0
                    SC0      12      N       BUS 1
             _INTERRUPT      13      P       BUS 2
                    SC1      14      R       BUS 3
               _DMA-OUT      15      S       BUS 4
                      Q      16      T       BUS 5
                _DMA-IN      17      U       BUS 6
                    RUN      18      V       BUS 7
                   MINH      19      W       _MRD
                  _CDEF      20      X       CS
                   +5 V      21      Y       +5 V
                    GND      22      Z       GND

**********************************************************************/

#ifndef MAME_BUS_VIP_EXP_H
#define MAME_BUS_VIP_EXP_H

#pragma once




//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vip_expansion_slot_device

class device_vip_expansion_card_interface;

class vip_expansion_slot_device : public device_t, public device_single_card_slot_interface<device_vip_expansion_card_interface>
{
public:
	// construction/destruction
	template <typename T>
	vip_expansion_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock, T &&opts, char const *dflt)
		: vip_expansion_slot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	vip_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto int_wr_callback() { return m_write_int.bind(); }
	auto dma_out_wr_callback() { return m_write_dma_out.bind(); }
	auto dma_in_wr_callback() { return m_write_dma_in.bind(); }

	// computer interface
	uint8_t program_r(offs_t offset, int cs, int cdef, int *minh);
	void program_w(offs_t offset, uint8_t data, int cdef, int *minh);
	uint8_t io_r(offs_t offset);
	void io_w(offs_t offset, uint8_t data);
	uint8_t dma_r(offs_t offset);
	void dma_w(offs_t offset, uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	int ef1_r();
	int ef3_r();
	int ef4_r();
	void sc_w(offs_t offset, uint8_t data);
	void q_w(int state);
	void tpb_w(int state);
	void run_w(int state);

	// cartridge interface
	void interrupt_w(int state) { m_write_int(state); }
	void dma_out_w(int state) { m_write_dma_out(state); }
	void dma_in_w(int state) { m_write_dma_in(state); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	devcb_write_line m_write_int;
	devcb_write_line m_write_dma_out;
	devcb_write_line m_write_dma_in;

	device_vip_expansion_card_interface *m_card;
};


// ======================> device_vip_expansion_card_interface

class device_vip_expansion_card_interface : public device_interface
{
	friend class vip_expansion_slot_device;

protected:
	// construction/destruction
	device_vip_expansion_card_interface(const machine_config &mconfig, device_t &device);

	// runtime
	virtual uint8_t vip_program_r(offs_t offset, int cs, int cdef, int *minh) { return 0xff; }
	virtual void vip_program_w(offs_t offset, uint8_t data, int cdef, int *minh) { }

	virtual uint8_t vip_io_r(offs_t offset) { return 0xff; }
	virtual void vip_io_w(offs_t offset, uint8_t data) { }

	virtual uint8_t vip_dma_r(offs_t offset) { return 0xff; }
	virtual void vip_dma_w(offs_t offset, uint8_t data) { }

	virtual uint32_t vip_screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) { return 0; }

	virtual int vip_ef1_r() { return CLEAR_LINE; }
	virtual int vip_ef3_r() { return CLEAR_LINE; }
	virtual int vip_ef4_r() { return CLEAR_LINE; }

	virtual void vip_sc_w(int n, int sc) { }

	virtual void vip_q_w(int state) { }

	virtual void vip_tpb_w(int state) { }

	virtual void vip_run_w(int state) { }

	vip_expansion_slot_device *m_slot;
};


// device type definition
DECLARE_DEVICE_TYPE(VIP_EXPANSION_SLOT, vip_expansion_slot_device)


void vip_expansion_cards(device_slot_interface &device);

#endif // MAME_BUS_VIP_EXP_H
