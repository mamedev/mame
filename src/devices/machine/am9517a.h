// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    AMD AM9517A/8237A Multimode DMA Controller emulation

****************************************************************************
                            _____   _____
                  _IOR   1 |*    \_/     | 40  A7
                  _IOW   2 |             | 39  A6
                 _MEMR   3 |             | 38  A5
                 _MEMW   4 |             | 37  A4
                     *   5 |             | 36  _EOP
                 READY   6 |             | 35  A3
                  HACK   7 |             | 34  A2
                 ADSTB   8 |             | 33  A1
                   AEN   9 |             | 32  A0
                  HREQ  10 |   AM9517A   | 31  Vcc
                   _CS  11 |    8237A    | 30  DB0
                   CLK  12 |             | 29  DB1
                 RESET  13 |             | 28  DB2
                 DACK2  14 |             | 27  DB3
                 DACK3  15 |             | 26  DB4
                 DREQ3  16 |             | 25  DACK0
                 DREQ2  17 |             | 24  DACK1
                 DREQ1  18 |             | 23  DB5
                 DREQ0  19 |             | 22  DB6
                   Vss  20 |_____________| 21  DB7

***************************************************************************/

#ifndef MAME_MACHINE_AM9517_H
#define MAME_MACHINE_AM9517_H

#pragma once



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> am9517a_device

class am9517a_device :  public device_t,
						public device_execute_interface
{
	friend class pcxport_dmac_device;

public:
	// construction/destruction
	am9517a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto out_hreq_callback() { return m_out_hreq_cb.bind(); }
	auto out_eop_callback() { return m_out_eop_cb.bind(); }

	auto in_memr_callback() { return m_in_memr_cb.bind(); }
	auto out_memw_callback() { return m_out_memw_cb.bind(); }

	template <unsigned C> auto in_ior_callback() { return m_in_ior_cb[C].bind(); }
	template <unsigned C> auto out_iow_callback() { return m_out_iow_cb[C].bind(); }
	template <unsigned C> auto out_dack_callback() { return m_out_dack_cb[C].bind(); }

	// define initial (inactive) state of DREQ inputs
	void dreq_active_low() { assert(!configured()); m_status = 0xf0; }
	void dreq_active_high() { assert(!configured()); m_status = 0; }

	virtual uint8_t read(offs_t offset);
	virtual void write(offs_t offset, uint8_t data);

	void hack_w(int state);
	void ready_w(int state);
	void eop_w(int state);

	template <unsigned C> void dreq_w(int state) { dma_request(C, state); }
	void dreq0_w(int state);
	void dreq1_w(int state);
	void dreq2_w(int state);
	void dreq3_w(int state);

protected:
	am9517a_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void execute_run() override;

	virtual void end_of_process();

	virtual void dma_read();
	virtual void dma_write();

	virtual int transfer_size(int const channel) const { return 1; }

	int m_icount;
	uint32_t m_address_mask;

	struct
	{
		uint32_t m_address;
		uint32_t m_count;
		uint32_t m_base_address;
		uint32_t m_base_count;
		uint8_t m_mode;
	} m_channel[4];

	int m_msb;
	int m_hreq;
	int m_hack;
	int m_ready;
	int m_eop;
	int m_state;
	int m_current_channel;
	int m_last_channel;
	uint8_t m_command;
	uint8_t m_mask;
	uint8_t m_status;
	uint16_t m_temp;
	uint8_t m_request;

private:
	void dma_request(int channel, bool state);
	void mask_channel(int channel, bool state);
	void set_mask_register(uint8_t mask);
	inline bool is_request_active(int channel);
	inline bool is_software_request_active(int channel);
	inline void set_hreq(int state);
	inline void set_dack();
	inline void set_eop(int state);
	inline int get_state1(bool msb_changed);
	inline void dma_advance();

	devcb_write_line   m_out_hreq_cb;
	devcb_write_line   m_out_eop_cb;

	devcb_read8        m_in_memr_cb;
	devcb_write8       m_out_memw_cb;

	devcb_read8::array<4> m_in_ior_cb;
	devcb_write8::array<4> m_out_iow_cb;
	devcb_write_line::array<4> m_out_dack_cb;
};


class v5x_dmau_device : public am9517a_device
{
public:
	// construction/destruction
	v5x_dmau_device(const machine_config &mconfig,  const char *tag, device_t *owner, uint32_t clock);

	auto in_mem16r_callback() { return m_in_mem16r_cb.bind(); }
	auto out_mem16w_callback() { return m_out_mem16w_cb.bind(); }

	template <unsigned C> auto in_io16r_callback() { return m_in_io16r_cb[C].bind(); }
	template <unsigned C> auto out_io16w_callback() { return m_out_io16w_cb[C].bind(); }

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void dma_read() override;
	virtual void dma_write() override;

	virtual int transfer_size(int const channel) const override { return (m_channel[channel].m_mode & 0x1) ? 2 : 1; }

	// 16 bit transfer callbacks
	devcb_read16 m_in_mem16r_cb;
	devcb_write16 m_out_mem16w_cb;
	devcb_read16::array<4> m_in_io16r_cb;
	devcb_write16::array<4> m_out_io16w_cb;

	int m_selected_channel;
	int m_base;
	uint8_t m_command_high;
};


class pcxport_dmac_device : public am9517a_device
{
public:
	// construction/destruction
	pcxport_dmac_device(const machine_config &mconfig,  const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_reset() override ATTR_COLD;

	virtual void end_of_process() override;
};

class eisa_dma_device : public am9517a_device
{
public:
	eisa_dma_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <unsigned Channel> u8 get_address_page() { return m_channel[Channel].m_address >> 16; }
	template <unsigned Channel> void set_address_page(u8 data)
	{
		m_channel[Channel].m_base_address = (m_channel[Channel].m_base_address & 0x0000ffffU) | (u32(data) << 16);
		m_channel[Channel].m_address = (m_channel[Channel].m_address & 0x0000ffffU) | (u32(data) << 16);
	}

	template <unsigned Channel> u8 get_address_page_high() { return m_channel[Channel].m_address >> 24; }
	template <unsigned Channel> void set_address_page_high(u8 data)
	{
		m_channel[Channel].m_base_address = (m_channel[Channel].m_base_address & 0x00ffffffU) | (u32(data) << 24);
		m_channel[Channel].m_address = (m_channel[Channel].m_address & 0x00ffffffU) | (u32(data) << 24);
	}

	template <unsigned Channel> u8 get_count_high() { return m_channel[Channel].m_count >> 16; }
	template <unsigned Channel> void set_count_high(u8 data)
	{
		m_channel[Channel].m_base_count = (m_channel[Channel].m_base_count & 0x0000ffffU) | (u32(data) << 16);
		m_channel[Channel].m_count = (m_channel[Channel].m_count & 0x0000ffffU) | (u32(data) << 16);
	}

	void set_ext_mode(u8 data)
	{
		m_ext_mode[data & 3] = data;
	}

	template <unsigned Channel> u32 get_stop() { return m_stop[Channel]; }
	template <unsigned Channel> void set_stop(offs_t offset, u32 data, u32 mem_mask)
	{
		mem_mask &= 0x00fffffcU;
		COMBINE_DATA(&m_stop[Channel]);
	}

protected:
	virtual void device_start() override ATTR_COLD;

private:
	u32 m_stop[4];
	u8 m_ext_mode[4];
};

// device type definition
DECLARE_DEVICE_TYPE(AM9517A,      am9517a_device)
DECLARE_DEVICE_TYPE(V5X_DMAU,     v5x_dmau_device)
DECLARE_DEVICE_TYPE(PCXPORT_DMAC, pcxport_dmac_device)
DECLARE_DEVICE_TYPE(EISA_DMA,     eisa_dma_device)

#endif // MAME_MACHINE_AM9517_H
