// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_INTERGRAPH_INTERPRO_SGA_H
#define MAME_INTERGRAPH_INTERPRO_SGA_H

#pragma once

class interpro_sga_device : public device_t
{
public:
	interpro_sga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto berr_callback() { return m_berr_func.bind(); }

	virtual void map(address_map &map) ATTR_COLD;

	u32 gcsr_r() { return m_gcsr; }
	void gcsr_w(u32 data) { m_gcsr = data; }
	enum ipoll_mask
	{
		IPOLL_ATTN       = 0x000000ff,
		IPOLL_DMAEND     = 0x00000700,
		IPOLL_NOGRANT    = 0x00001000,
		IPOLL_SRMMBE     = 0x00002000,
		IPOLL_SRBERR     = 0x00004000,
		IPOLL_RETRYABORT = 0x00008000,
		IPOLL_INTBERR    = 0x00010000,
		IPOLL_INTMMBE    = 0x00020000
	};
	u32 ipoll_r() { return m_ipoll; }
	void ipoll_w(u32 data) { m_ipoll = data; }

	enum imask_mask
	{
		IMASK_DMAENDCH1  = 0x00000200,
		IMASK_NOGRANT    = 0x00001000,
		IMASK_SRMMBE     = 0x00002000,
		IMASK_SRBERR     = 0x00004000,
		IMASK_RETRYABORT = 0x00008000,
		IMASK_INTBERR    = 0x00010000,
		IMASK_INTMMBE    = 0x00020000
	};
	u32 imask_r() { return m_imask; }
	void imask_w(u32 data) { m_imask = data; }
	u32 range_base_r() { return m_range_base; }
	void range_base_w(u32 data) { m_range_base = data; }
	u32 range_end_r() { return m_range_end; }
	void range_end_w(u32 data) { m_range_end = data; }

	enum cttag_mask
	{
		CTTAG_TAG      = 0x00000007,
		CTTAG_CYCLE    = 0x000001f8,
		CTTAG_MAXBCLK  = 0x0003fe00,
		CTTAG_MAXRETRY = 0x3ffc0000
	};
	u32 cttag_r() { return m_cttag; }
	void cttag_w(u32 data) { m_cttag = data; }
	u32 address_r() { return m_address; }
	void address_w(u32 data) { m_address = data; }

	enum dmacsr_mask
	{
		DMACSR_CH1ENABLE = 0x00000080
	};
	u32 dmacsr_r() { return m_dmacsr; }
	void dmacsr_w(u32 data) { m_dmacsr = data; }

	enum edmacsr_mask
	{
		EDMACSR_CH1RDONLY = 0x00000010
	};
	u32 edmacsr_r() { return m_edmacsr; }
	void edmacsr_w(u32 data) { m_edmacsr = data; }
	u32 reg6_range_r() { return m_reg6_range; }
	void reg6_range_w(u32 data) { m_reg6_range = data; }

	u32 ddpta0_r() { return m_ddpta0; }
	void ddpta0_w(u32 data) { m_ddpta0 = data; }
	u32 ddpad0_r() { return m_ddpad0; }
	void ddpad0_w(u32 data) { m_ddpad0 = data; }
	u32 ddoff0_r() { return m_ddoff0; }
	void ddoff0_w(u32 data) { m_ddoff0 = data; }
	u32 ddtc0_r() { return m_ddtc0; }
	void ddtc0_w(u32 data) { m_ddtc0 = data; }

	u32 dspta0_r() { return m_dspta0; }
	void dspta0_w(u32 data) { m_dspta0 = data; }
	u32 dspad0_r() { return m_dspad0; }
	void dspad0_w(u32 data) { m_dspad0 = data; }
	u32 dsoff0_r() { return m_dsoff0; }
	void dsoff0_w(u32 data) { m_dsoff0 = data; }
	u32 dstc0_r() { return m_dstc0; }
	void dstc0_w(u32 data) { m_dstc0 = data; }

	u32 dspad1_r() { return m_dspad1; }
	void dspad1_w(u32 data) { m_dspad1 = data; }
	u32 dsoff1_r() { return m_dsoff1; }
	void dsoff1_w(u32 data) { m_dsoff1 = data; }
	u32 dstc1_r() { return m_dstc1; }
	void dstc1_w(u32 data) { m_dstc1 = data; }

	u32 ddpad1_r() { return m_ddpad1; }
	void ddpad1_w(u32 data) { m_ddpad1 = data; }
	u32 ddoff1_r() { return m_ddoff1; }
	void ddoff1_w(u32 data) { m_ddoff1 = data; }
	u32 ddtc1_r() { return m_ddtc1; }
	void ddtc1_w(u32 data);

	u32 ddpta2_r() { return m_ddpta2; }
	void ddpta2_w(u32 data) { m_ddpta2 = data; }
	u32 ddpad2_r() { return m_ddpad2; }
	void ddpad2_w(u32 data) { m_ddpad2 = data; }
	u32 ddoff2_r() { return m_ddoff2; }
	void ddoff2_w(u32 data) { m_ddoff2 = data; }
	u32 ddtc2_r() { return m_ddtc2; }
	void ddtc2_w(u32 data) { m_ddtc2 = data; }

	u32 dspta2_r() { return m_dspta2; }
	void dspta2_w(u32 data) { m_dspta2 = data; }
	u32 dspad2_r() { return m_dspad2; }
	void dspad2_w(u32 data) { m_dspad2 = data; }
	u32 dsoff2_r() { return m_dsoff2; }
	void dsoff2_w(u32 data) { m_dsoff2 = data; }
	u32 dstc2_r() { return m_dstc2; }
	void dstc2_w(u32 data) { m_dstc2 = data; }

	u32 ddrd2_r() { return m_ddrd2; }
	void ddrd2_w(u32 data) { m_ddrd2 = data; }
	u32 dsrd2_r() { return m_dsrd2; }
	void dsrd2_w(u32 data) { m_dsrd2 = data; }
	u32 dcksum0_r() { return m_dcksum0; }
	void dcksum0_w(u32 data) { m_dcksum0 = data; }
	u32 dcksum1_r() { return m_dcksum1; }
	void dcksum1_w(u32 data) { m_dcksum1 = data; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	u32 m_gcsr;        // general control/status
	u32 m_ipoll;       // interrupt poll
	u32 m_imask;       // interrupt mask
	u32 m_range_base;
	u32 m_range_end;
	u32 m_cttag;       // error cycletype/tag, aka diag1
	u32 m_address;     // aka diag0
	u32 m_dmacsr;      // dma control/status

	u32 m_edmacsr;     // extended dma control/status
	u32 m_reg6_range;  // region 6 range

	u32 m_ddpta0; // dma 0 device page table address
	u32 m_ddpad0; // dma 0 device page address
	u32 m_ddoff0; // dma 0 device page offset
	u32 m_ddtc0;  // dma 0 device transfer context
	u32 m_dspta0; // dma 0 SRX page table address
	u32 m_dspad0; // dma 0 SRX page address
	u32 m_dsoff0; // dma 0 SRX page offset
	u32 m_dstc0;  // dma 0 SRX transfer context

	u32 m_dspad1; // dma 1 source page address
	u32 m_dsoff1; // dma 1 source page offset
	u32 m_dstc1;  // dma 1 source transfer count
	u32 m_ddpad1; // dma 1 destination page address
	u32 m_ddoff1; // dma 1 destination page offset
	u32 m_ddtc1;  // dma 1 destination transfer count

	u32 m_ddpta2; // dma 2 device page table address
	u32 m_ddpad2; // dma 2 device page address
	u32 m_ddoff2; // dma 2 device page offset
	u32 m_ddtc2;  // dma 2 device transfer context
	u32 m_dspta2; // dma 2 SRX page table address
	u32 m_dspad2; // dma 2 SRX page address
	u32 m_dsoff2; // dma 2 SRX page offset
	u32 m_dstc2;  // dma 2 SRX transfer context

	u32 m_ddrd2;   // dma 2 device record descriptor
	u32 m_dsrd2;   // dma 2 SRX record descriptor
	u32 m_dcksum0; // dma 1 device checksum register 0
	u32 m_dcksum1; // dma 1 device checksum register 1

	devcb_write32 m_berr_func;
};

// device type definition
DECLARE_DEVICE_TYPE(INTERPRO_SGA, interpro_sga_device)

#endif // MAME_INTERGRAPH_INTERPRO_SGA_H
