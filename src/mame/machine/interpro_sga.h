// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_MACHINE_INTERPRO_SGA_H
#define MAME_MACHINE_INTERPRO_SGA_H

#pragma once

class interpro_sga_device : public device_t
{
public:
	interpro_sga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto berr_callback() { return m_berr_func.bind(); }

	virtual void map(address_map &map);

	DECLARE_READ32_MEMBER(gcsr_r) { return m_gcsr; }
	DECLARE_WRITE32_MEMBER(gcsr_w) { m_gcsr = data; }
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
	DECLARE_READ32_MEMBER(ipoll_r) { return m_ipoll; }
	DECLARE_WRITE32_MEMBER(ipoll_w) { m_ipoll = data; }

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
	DECLARE_READ32_MEMBER(imask_r) { return m_imask; }
	DECLARE_WRITE32_MEMBER(imask_w) { m_imask = data; }
	DECLARE_READ32_MEMBER(range_base_r) { return m_range_base; }
	DECLARE_WRITE32_MEMBER(range_base_w) { m_range_base = data; }
	DECLARE_READ32_MEMBER(range_end_r) { return m_range_end; }
	DECLARE_WRITE32_MEMBER(range_end_w) { m_range_end = data; }

	enum cttag_mask
	{
		CTTAG_TAG      = 0x00000007,
		CTTAG_CYCLE    = 0x000001f8,
		CTTAG_MAXBCLK  = 0x0003fe00,
		CTTAG_MAXRETRY = 0x3ffc0000
	};
	DECLARE_READ32_MEMBER(cttag_r) { return m_cttag; }
	DECLARE_WRITE32_MEMBER(cttag_w) { m_cttag = data; }
	DECLARE_READ32_MEMBER(address_r) { return m_address; }
	DECLARE_WRITE32_MEMBER(address_w) { m_address = data; }

	enum dmacsr_mask
	{
		DMACSR_CH1ENABLE = 0x00000080
	};
	DECLARE_READ32_MEMBER(dmacsr_r) { return m_dmacsr; }
	DECLARE_WRITE32_MEMBER(dmacsr_w) { m_dmacsr = data; }

	enum edmacsr_mask
	{
		EDMACSR_CH1RDONLY = 0x00000010
	};
	DECLARE_READ32_MEMBER(edmacsr_r) { return m_edmacsr; }
	DECLARE_WRITE32_MEMBER(edmacsr_w) { m_edmacsr = data; }
	DECLARE_READ32_MEMBER(reg6_range_r) { return m_reg6_range; }
	DECLARE_WRITE32_MEMBER(reg6_range_w) { m_reg6_range = data; }

	DECLARE_READ32_MEMBER(ddpta0_r) { return m_ddpta0; }
	DECLARE_WRITE32_MEMBER(ddpta0_w) { m_ddpta0 = data; }
	DECLARE_READ32_MEMBER(ddpad0_r) { return m_ddpad0; }
	DECLARE_WRITE32_MEMBER(ddpad0_w) { m_ddpad0 = data; }
	DECLARE_READ32_MEMBER(ddoff0_r) { return m_ddoff0; }
	DECLARE_WRITE32_MEMBER(ddoff0_w) { m_ddoff0 = data; }
	DECLARE_READ32_MEMBER(ddtc0_r) { return m_ddtc0; }
	DECLARE_WRITE32_MEMBER(ddtc0_w) { m_ddtc0 = data; }

	DECLARE_READ32_MEMBER(dspta0_r) { return m_dspta0; }
	DECLARE_WRITE32_MEMBER(dspta0_w) { m_dspta0 = data; }
	DECLARE_READ32_MEMBER(dspad0_r) { return m_dspad0; }
	DECLARE_WRITE32_MEMBER(dspad0_w) { m_dspad0 = data; }
	DECLARE_READ32_MEMBER(dsoff0_r) { return m_dsoff0; }
	DECLARE_WRITE32_MEMBER(dsoff0_w) { m_dsoff0 = data; }
	DECLARE_READ32_MEMBER(dstc0_r) { return m_dstc0; }
	DECLARE_WRITE32_MEMBER(dstc0_w) { m_dstc0 = data; }

	DECLARE_READ32_MEMBER(dspad1_r) { return m_dspad1; }
	DECLARE_WRITE32_MEMBER(dspad1_w) { m_dspad1 = data; }
	DECLARE_READ32_MEMBER(dsoff1_r) { return m_dsoff1; }
	DECLARE_WRITE32_MEMBER(dsoff1_w) { m_dsoff1 = data; }
	DECLARE_READ32_MEMBER(dstc1_r) { return m_dstc1; }
	DECLARE_WRITE32_MEMBER(dstc1_w) { m_dstc1 = data; }

	DECLARE_READ32_MEMBER(ddpad1_r) { return m_ddpad1; }
	DECLARE_WRITE32_MEMBER(ddpad1_w) { m_ddpad1 = data; }
	DECLARE_READ32_MEMBER(ddoff1_r) { return m_ddoff1; }
	DECLARE_WRITE32_MEMBER(ddoff1_w) { m_ddoff1 = data; }
	DECLARE_READ32_MEMBER(ddtc1_r) { return m_ddtc1; }
	DECLARE_WRITE32_MEMBER(ddtc1_w);

	DECLARE_READ32_MEMBER(ddpta2_r) { return m_ddpta2; }
	DECLARE_WRITE32_MEMBER(ddpta2_w) { m_ddpta2 = data; }
	DECLARE_READ32_MEMBER(ddpad2_r) { return m_ddpad2; }
	DECLARE_WRITE32_MEMBER(ddpad2_w) { m_ddpad2 = data; }
	DECLARE_READ32_MEMBER(ddoff2_r) { return m_ddoff2; }
	DECLARE_WRITE32_MEMBER(ddoff2_w) { m_ddoff2 = data; }
	DECLARE_READ32_MEMBER(ddtc2_r) { return m_ddtc2; }
	DECLARE_WRITE32_MEMBER(ddtc2_w) { m_ddtc2 = data; }

	DECLARE_READ32_MEMBER(dspta2_r) { return m_dspta2; }
	DECLARE_WRITE32_MEMBER(dspta2_w) { m_dspta2 = data; }
	DECLARE_READ32_MEMBER(dspad2_r) { return m_dspad2; }
	DECLARE_WRITE32_MEMBER(dspad2_w) { m_dspad2 = data; }
	DECLARE_READ32_MEMBER(dsoff2_r) { return m_dsoff2; }
	DECLARE_WRITE32_MEMBER(dsoff2_w) { m_dsoff2 = data; }
	DECLARE_READ32_MEMBER(dstc2_r) { return m_dstc2; }
	DECLARE_WRITE32_MEMBER(dstc2_w) { m_dstc2 = data; }

	DECLARE_READ32_MEMBER(ddrd2_r) { return m_ddrd2; }
	DECLARE_WRITE32_MEMBER(ddrd2_w) { m_ddrd2 = data; }
	DECLARE_READ32_MEMBER(dsrd2_r) { return m_dsrd2; }
	DECLARE_WRITE32_MEMBER(dsrd2_w) { m_dsrd2 = data; }
	DECLARE_READ32_MEMBER(dcksum0_r) { return m_dcksum0; }
	DECLARE_WRITE32_MEMBER(dcksum0_w) { m_dcksum0 = data; }
	DECLARE_READ32_MEMBER(dcksum1_r) { return m_dcksum1; }
	DECLARE_WRITE32_MEMBER(dcksum1_w) { m_dcksum1 = data; }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

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

#endif // MAME_MACHINE_INTERPRO_SGA_H
