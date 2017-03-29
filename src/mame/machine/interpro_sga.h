// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#pragma once

#ifndef INTERPRO_SGA_H_
#define INTERPRO_SGA_H_


class interpro_sga_device : public device_t
{
public:
	interpro_sga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_ADDRESS_MAP(map, 32);

	DECLARE_READ32_MEMBER(gcs_r) { return m_gcs; }
	DECLARE_WRITE32_MEMBER(gcs_w) { m_gcs = data; }
	DECLARE_READ32_MEMBER(ipoll_r) { return m_ipoll; }
	DECLARE_WRITE32_MEMBER(ipoll_w) { m_ipoll = data; }
	DECLARE_READ32_MEMBER(imask_r) { return m_imask; }
	DECLARE_WRITE32_MEMBER(imask_w) { m_imask = data; }
	DECLARE_READ32_MEMBER(range_base_r) { return m_range_base; }
	DECLARE_WRITE32_MEMBER(range_base_w) { m_range_base = data; }
	DECLARE_READ32_MEMBER(range_end_r) { return m_range_end; }
	DECLARE_WRITE32_MEMBER(range_end_w) { m_range_end = data; }
	DECLARE_READ32_MEMBER(cttag_r) { return m_cttag; }
	DECLARE_WRITE32_MEMBER(cttag_w) { m_cttag = data; }
	DECLARE_READ32_MEMBER(address_r) { return m_address; }
	DECLARE_WRITE32_MEMBER(address_w) { m_address = data; }
	DECLARE_READ32_MEMBER(dmacs_r) { return m_dmacs; }
	DECLARE_WRITE32_MEMBER(dmacs_w) { m_dmacs = data; }
	DECLARE_READ32_MEMBER(edmacs_r) { return m_edmacs; }
	DECLARE_WRITE32_MEMBER(edmacs_w) { m_edmacs = data; }
	DECLARE_READ32_MEMBER(dspad1_r) { return m_dspad1; }
	DECLARE_WRITE32_MEMBER(dspad1_w) { m_dspad1 = data; }
	DECLARE_READ32_MEMBER(dsoff1_r) { return m_dsoff1; }
	DECLARE_WRITE32_MEMBER(dsoff1_w) { m_dsoff1 = data; }
	DECLARE_READ32_MEMBER(unknown1_r) { return m_unknown1; }
	DECLARE_WRITE32_MEMBER(unknown1_w) { m_unknown1 = data; }
	DECLARE_READ32_MEMBER(unknown2_r) { return m_unknown2; }
	DECLARE_WRITE32_MEMBER(unknown2_w) { m_unknown2 = data; }
	DECLARE_READ32_MEMBER(ddtc1_r) { return m_ddtc1; }
	DECLARE_WRITE32_MEMBER(ddtc1_w);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	uint32_t m_gcs;         // general control/status
	uint32_t m_ipoll;       // interrupt poll
	uint32_t m_imask;       // interrupt mask
	uint32_t m_range_base;
	uint32_t m_range_end;
	uint32_t m_cttag;       // error cycletype/tag
	uint32_t m_address;
	uint32_t m_dmacs;       // dma control/status
	uint32_t m_edmacs;      // extended dma control/status
	uint32_t m_dspad1;
	uint32_t m_dsoff1;
	uint32_t m_unknown1;
	uint32_t m_unknown2;
	uint32_t m_ddtc1;
};

// device type definition
extern const device_type INTERPRO_SGA;

#endif
