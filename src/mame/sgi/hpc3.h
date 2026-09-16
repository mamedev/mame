// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

    SGI HPC3 "High-performance Peripheral Controller" emulation

**********************************************************************/

#ifndef MAME_SGI_HPC3_H
#define MAME_SGI_HPC3_H

#pragma once

#include "hal2.h"
#include "machine/edlc.h"

class hpc3_device : public device_t, public device_memory_interface
{
public:
	enum pbus_space
	{
		AS_PIO0 = 0,
		AS_PIO1,
		AS_PIO2,
		AS_PIO3,
		AS_PIO4,
		AS_PIO5,
		AS_PIO6,
		AS_PIO7,
		AS_PIO8,
		AS_PIO9
	};

	hpc3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T>
	hpc3_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&hal2_tag)
		: hpc3_device(mconfig, tag, owner, (uint32_t)0)
	{
		set_hal2_tag(std::forward<T>(hal2_tag));
	}

	template <typename T> void set_gio64_space(T &&tag, int spacenum) { m_gio64_space.set_tag(std::forward<T>(tag), spacenum); }
	template <typename T> void set_hal2_tag(T &&tag) { m_hal2.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_enet(T &&tag) { m_enet.set_tag(std::forward<T>(tag)); }

	auto enet_intr_out_cb() { return m_enet_intr_out_cb.bind(); }
	template <int N> auto hd_rd_cb() { return m_hd_rd_cb[N].bind(); }
	template <int N> auto hd_wr_cb() { return m_hd_wr_cb[N].bind(); }
	template <int N> auto hd_dma_rd_cb() { return m_hd_dma_rd_cb[N].bind(); }
	template <int N> auto hd_dma_wr_cb() { return m_hd_dma_wr_cb[N].bind(); }
	template <int N> auto hd_reset_cb() { return m_hd_reset_cb[N].bind(); }
	auto bbram_rd_cb() { return m_bbram_rd_cb.bind(); }
	auto bbram_wr_cb() { return m_bbram_wr_cb.bind(); }
	auto eeprom_dati_cb() { return m_eeprom_dati_cb.bind(); }
	auto eeprom_dato_cb() { return m_eeprom_dato_cb.bind(); }
	auto eeprom_clk_cb() { return m_eeprom_clk_cb.bind(); }
	auto eeprom_cs_cb() { return m_eeprom_cs_cb.bind(); }
	auto eeprom_pre_cb() { return m_eeprom_pre_cb.bind(); }
	auto dma_complete_int_cb() { return m_dma_complete_int_cb.bind(); }

	void map(address_map &map) ATTR_COLD;

	void enet_rxrdy_w(int state);
	void enet_intr_in_w(int state);

	void scsi0_drq(int state);
	void scsi1_drq(int state);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual space_config_vector memory_space_config() const override;

	enum fifo_type_t : uint32_t
	{
		FIFO_PBUS,
		FIFO_SCSI0,
		FIFO_SCSI1,
		FIFO_ENET_RECV,
		FIFO_ENET_XMIT
	};

	uint32_t enet_r(offs_t offset, uint32_t mem_mask = ~0);
	void enet_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t hd_enet_r(offs_t offset, uint32_t mem_mask = ~0);
	void hd_enet_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	template <uint32_t index> uint32_t hd_r(offs_t offset, uint32_t mem_mask = ~0);
	template <uint32_t index> void hd_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	template <fifo_type_t Type> uint32_t fifo_r(offs_t offset);
	template <fifo_type_t Type> void fifo_w(offs_t offset, uint32_t data);
	uint32_t intstat_r();
	uint32_t misc_r();
	void misc_w(uint32_t data);
	uint32_t eeprom_r();
	void eeprom_w(uint32_t data);
	uint32_t pio_data_r(offs_t offset);
	void pio_data_w(offs_t offset, uint32_t data);
	uint32_t pbusdma_r(offs_t offset, uint32_t mem_mask = ~0);
	void pbusdma_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint32_t dma_config_r(offs_t offset, uint32_t mem_mask = ~0);
	void dma_config_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t pio_config_r(offs_t offset, uint32_t mem_mask = ~0);
	void pio_config_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t bbram_r(offs_t offset);
	void bbram_w(offs_t offset, uint32_t data);

	TIMER_CALLBACK_MEMBER(do_pbus_dma);
	void do_scsi_dma(int channel);

	void dump_chain(uint32_t base);
	void fetch_chain(int channel);
	void decrement_chain(int channel);
	void scsi_fifo_flush(int channel);
	void scsi_drq(bool state, int channel);
	//void scsi_dma(int channel);

	void enet_transmit(int32_t param = 0);
	void enet_misc_w(u32 data);
	bool enet_rx_bc_dec(unsigned const count = 1);

	struct pbus_dma_t
	{
		bool m_active = false;
		uint32_t m_cur_ptr = 0;
		uint32_t m_desc_ptr = 0;
		uint32_t m_desc_flags = 0;
		uint32_t m_next_ptr = 0;
		uint32_t m_bytes_left = 0;
		uint32_t m_config = 0;
		uint32_t m_control = 0;
		emu_timer *m_timer = nullptr;
	};

	enum
	{
		PBUS_CTRL_ENDIAN    = 0x00000002,
		PBUS_CTRL_RECV      = 0x00000004,
		PBUS_CTRL_FLUSH     = 0x00000008,
		PBUS_CTRL_DMASTART  = 0x00000010,
		PBUS_CTRL_LOAD_EN   = 0x00000020,
		PBUS_CTRL_REALTIME  = 0x00000040,
		PBUS_CTRL_HIGHWATER = 0x0000ff00,
		PBUS_CTRL_FIFO_BEG  = 0x003f0000,
		PBUS_CTRL_FIFO_END  = 0x3f000000,
	};

	enum
	{
		PBUS_DMADESC_EOX  = 0x80000000,
		PBUS_DMADESC_EOXP = 0x40000000,
		PBUS_DMADESC_XIE  = 0x20000000,
		PBUS_DMADESC_IPG  = 0x00ff0000,
		PBUS_DMADESC_TXD  = 0x00008000,
		PBUS_DMADESC_BC   = 0x00003fff,
	};

	enum
	{
		HPC3_DMACTRL_IRQ    = 0x01,
		HPC3_DMACTRL_ENDIAN = 0x02,
		HPC3_DMACTRL_DIR    = 0x04,
		HPC3_DMACTRL_FLUSH  = 0x08,
		HPC3_DMACTRL_ENABLE = 0x10,
		HPC3_DMACTRL_WRMASK = 0x20,
	};

	enum
	{
		ENET_RECV = 0,
		ENET_XMIT = 1
	};

	const address_space_config m_pio_space_config[10]{};

	required_address_space m_gio64_space;
	address_space *m_pio_space[10]{};
	required_device<hal2_device> m_hal2;
	required_device<seeq80c03_device> m_enet;

	devcb_write_line m_enet_intr_out_cb;
	devcb_read8::array<2> m_hd_rd_cb;
	devcb_write8::array<2> m_hd_wr_cb;
	devcb_read8::array<2> m_hd_dma_rd_cb;
	devcb_write8::array<2> m_hd_dma_wr_cb;
	devcb_write_line::array<2> m_hd_reset_cb;
	devcb_read8 m_bbram_rd_cb;
	devcb_write8 m_bbram_wr_cb;
	devcb_read_line m_eeprom_dati_cb;
	devcb_write_line m_eeprom_dato_cb;
	devcb_write_line m_eeprom_clk_cb;
	devcb_write_line m_eeprom_cs_cb;
	devcb_write_line m_eeprom_pre_cb;
	devcb_write_line m_dma_complete_int_cb;

	uint32_t m_intstat = 0;
	uint32_t m_misc = 0;
	uint32_t m_cpu_aux_ctrl = 0;

	struct scsi_dma_t
	{
		uint32_t m_cbp = 0;
		uint32_t m_nbdp = 0;
		uint8_t m_ctrl = 0;
		uint32_t m_bc = 0;
		uint16_t m_count = 0;
		uint32_t m_dmacfg = 0;
		uint32_t m_piocfg = 0;
		bool m_drq = false;
		bool m_big_endian = false;
		bool m_to_device = false;
		bool m_active = false;
	};

	scsi_dma_t m_scsi_dma[2];
	pbus_dma_t m_pbus_dma[8];
	uint32_t m_pio_config[10];

	std::unique_ptr<uint32_t[]> m_pbus_fifo;
	std::unique_ptr<uint32_t[]> m_scsi_fifo[2];
	std::unique_ptr<uint32_t[]> m_enet_fifo[2];

private:
	emu_timer *m_enet_tx_timer = nullptr;

	enum enet_dma_rx_ctrl : u32
	{
		RXC_ST   = 0x0000'00bf, // edlc receive status
		RXC_LC   = 0x0000'0040, // late receive collision
		RXC_LE   = 0x0000'0100, // receive little endian
		RXC_CA   = 0x0000'0200, // receive channel active
		RXC_CAM  = 0x0000'0400, // receive channel active mask
		RXC_RBO  = 0x0000'0800, // receive buffer overflow

		// individual receive status bits
		RXC_ST_V = 0x0000'0001, // received frame with overflow error
		RXC_ST_C = 0x0000'0002, // received frame with crc error
		RXC_ST_D = 0x0000'0004, // received frame with dribble error
		RXC_ST_S = 0x0000'0008, // received short frame
		RXC_ST_E = 0x0000'0010, // received end of frame
		RXC_ST_G = 0x0000'0020, // received good frame
		RXC_ST_O = 0x0000'0080, // old/new status
	};
	enum enet_dma_tx_ctrl : u32
	{
		TXC_ST   = 0x0000'00ef, // edlc transmit status
		TXC_LC   = 0x0000'0010, // late transmit collision
		TXC_LE   = 0x0000'0100, // transmit little endian
		TXC_CA   = 0x0000'0200, // transmit channel active
		TXC_CAM  = 0x0000'0400, // transmit channel active mask

		// individual transmit status bits
		TXC_ST_U = 0x0000'0001, // transmit underflow
		TXC_ST_C = 0x0000'0002, // transmit collision
		TXC_ST_R = 0x0000'0004, // 16 transmission attempts
		TXC_ST_S = 0x0000'0008, // transmission successful
		TXC_ST_O = 0x0000'0080, // old/new status
	};
	enum enet_dma_bc : u32
	{
		BC_BC   = 0x0000'3fff, // byte count
		BC_TXD  = 0x0000'8000, // transmit done
		BC_IPG  = 0x00ff'0000, // inter-packet gap
		BC_XIE  = 0x2000'0000, // transmit interrupt enable
		BC_EOXP = 0x4000'0000, // end of packet
		BC_EOX  = 0x8000'0000, // end of descriptor chain
	};
	enum enet_enet_misc : u32
	{
		MISC_RESET    = 0x0000'0001, // channel reset
		MISC_INT      = 0x0000'0002, // interrupt
		MISC_LOOPBACK = 0x0000'0004, // loopback
	};

	// ethernet registers
	u32 m_enet_rx_cbp = 0;    // current receive buffer pointer
	u32 m_enet_rx_nbdp = 0;   // next receive buffer descriptor pointer
	u32 m_enet_rx_bc = 0;     // receive byte count
	u32 m_enet_rx_ctrl = 0;   // receive status
	u32 m_enet_rx_gio = 0;    // receive gio fifo pointer
	u32 m_enet_rx_dev = 0;    // receive device fifo pointer
	u32 m_enet_misc = 0;      // reset, clear interrupt, loopback
	u32 m_enet_piocfg = 0;    // pio configuration
	u32 m_enet_dmacfg = 0;    // dma configuration
	u32 m_enet_tx_cbp = 0;    // current transmit buffer pointer
	u32 m_enet_tx_nbdp = 0;   // next transmit buffer descriptor pointer
	u32 m_enet_tx_bc = 0;     // transmit byte count
	u32 m_enet_tx_ctrl = 0;   // transmit status
	u32 m_enet_tx_gio = 0;    // transmit gio fifo pointer
	u32 m_enet_tx_dev = 0;    // transmit device fifo pointer
	u32 m_enet_rx_cbdp = 0;   // current receive buffer descriptor pointer
	u32 m_enet_tx_cpfbdp = 0; // current/previous first transmit buffer descriptor pointer
	u32 m_enet_tx_ppfbdp = 0; // previous/previous? first transmit buffer descriptor pointer
};

DECLARE_DEVICE_TYPE(SGI_HPC3, hpc3_device)

#endif // MAME_SGI_HPC3_H
