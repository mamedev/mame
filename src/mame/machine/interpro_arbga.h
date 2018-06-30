// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_MACHINE_INTERPRO_ARBGA_H
#define MAME_MACHINE_INTERPRO_ARBGA_H

#pragma once

class interpro_arbga_device : public device_t
{
public:
	interpro_arbga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map);

	DECLARE_READ32_MEMBER(sdepid_r) { return m_sdepid; }
	DECLARE_WRITE32_MEMBER(sdepid_w) { m_sdepid = data; }

	enum arbsnap_mask
	{
		ARBSNAP_GRANT   = 0x0000000f,
		ARBSNAP_HOGLOCK = 0x000000f0
	};
	DECLARE_READ32_MEMBER(arbsnap_r) { return m_arbsnap; }
	DECLARE_WRITE32_MEMBER(arbsnap_w) { m_arbsnap = data; }
	DECLARE_READ32_MEMBER(fixprils_r) { return m_fixprils; }
	DECLARE_WRITE32_MEMBER(fixprils_w) { m_fixprils = data; }
	DECLARE_READ32_MEMBER(fixprims_r) { return m_fixprims; }
	DECLARE_WRITE32_MEMBER(fixprims_w) { m_fixprims = data; }

	DECLARE_READ32_MEMBER(sysdomls_r) { return m_sysdomls; }
	DECLARE_WRITE32_MEMBER(sysdomls_w) { m_sysdomls = data; }
	DECLARE_READ32_MEMBER(sysdomms_r) { return m_sysdomms; }
	DECLARE_WRITE32_MEMBER(sysdomms_w) { m_sysdomms = data; }

	enum tctrl_mask
	{
		TCTRL_UNUSED = 0x00000007,
		TCTRL_ENNEM  = 0x00000008,
		TCTRL_ENHOG  = 0x00000010
	};
	DECLARE_READ32_MEMBER(tctrl_r) { return m_tctrl; }
	DECLARE_WRITE32_MEMBER(tctrl_w) { m_tctrl = data; }

	DECLARE_READ8_MEMBER(inem_r) { return m_inem; }
	DECLARE_WRITE8_MEMBER(inem_w) { m_inem = data; }
	DECLARE_READ8_MEMBER(enem_r) { return m_enem; }
	DECLARE_WRITE8_MEMBER(enem_w) { m_enem = data; }

	DECLARE_READ32_MEMBER(hog_r) { return m_hog; }
	DECLARE_WRITE32_MEMBER(hog_w) { m_hog = data; }
	DECLARE_READ32_MEMBER(lock_r) { return m_lock; }
	DECLARE_WRITE32_MEMBER(lock_w) { m_lock = data; }
	DECLARE_READ32_MEMBER(lockprs_r) { return m_lockprs; }
	DECLARE_WRITE32_MEMBER(lockprs_w) { m_lockprs = data; }

	DECLARE_READ32_MEMBER(hiblockls_r) { return m_hiblockls; }
	DECLARE_WRITE32_MEMBER(hiblockls_w) { m_hiblockls = data; }
	DECLARE_READ32_MEMBER(hiblockms_r) { return m_hiblockms; }
	DECLARE_WRITE32_MEMBER(hiblockms_w) { m_hiblockms = data; }

	DECLARE_READ32_MEMBER(arbrev_r) { return m_arbrev; }
	DECLARE_WRITE32_MEMBER(arbrev_w) { m_arbrev = data; }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	u32 m_sdepid;
	u32 m_arbsnap;
	u32 m_fixprils;
	u32 m_fixprims;
	u32 m_sysdomls;
	u32 m_sysdomms;
	u32 m_tctrl;
	u8 m_inem;
	u8 m_enem;
	u32 m_hog;
	u32 m_lock;
	u32 m_lockprs;
	u32 m_hiblockls;
	u32 m_hiblockms;
	u32 m_arbrev;
};

// device type definition
DECLARE_DEVICE_TYPE(INTERPRO_ARBGA, interpro_arbga_device)

#endif // MAME_MACHINE_INTERPRO_ARBGA_H
