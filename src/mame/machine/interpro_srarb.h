// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_MACHINE_INTERPRO_SRARB_H
#define MAME_MACHINE_INTERPRO_SRARB_H

#pragma once

class interpro_srarb_device : public device_t
{
public:
	interpro_srarb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_ADDRESS_MAP(map, 32);

	DECLARE_READ32_MEMBER(sdepid_r) { return sdepid; }
	DECLARE_WRITE32_MEMBER(sdepid_w) { sdepid = data; }

	enum snapid_mask
	{
		SNAPID_GRANT   = 0x0000000f,
		SNAPID_HOGLOCK = 0x000000f0
	};
	DECLARE_READ32_MEMBER(snapid_r) { return snapid; }
	DECLARE_WRITE32_MEMBER(snapid_w) { snapid = data; }
	DECLARE_READ32_MEMBER(prilo_r) { return prilo; }
	DECLARE_WRITE32_MEMBER(prilo_w) { prilo = data; }
	DECLARE_READ32_MEMBER(prihi_r) { return prihi; }
	DECLARE_WRITE32_MEMBER(prihi_w) { prihi = data; }

	DECLARE_READ32_MEMBER(errdomlo_r) { return errdomlo; }
	DECLARE_WRITE32_MEMBER(errdomlo_w) { errdomlo = data; }
	DECLARE_READ32_MEMBER(errdomhi_r) { return errdomhi; }
	DECLARE_WRITE32_MEMBER(errdomhi_w) { errdomhi = data; }

	enum tmctrl_mask
	{
		TMCTRL_UNUSED = 0x00000007,
		TMCTRL_ENNEM  = 0x00000008,
		TMCTRL_ENHOG  = 0x00000010
	};
	DECLARE_READ32_MEMBER(tmctrl_r) { return tmctrl; }
	DECLARE_WRITE32_MEMBER(tmctrl_w) { tmctrl = data; }

	DECLARE_READ8_MEMBER(unknown0_r) { return unknown0; }
	DECLARE_WRITE8_MEMBER(unknown0_w) { unknown0 = data; }
	DECLARE_READ8_MEMBER(unknown1_r) { return unknown1; }
	DECLARE_WRITE8_MEMBER(unknown1_w) { unknown1 = data; }

	DECLARE_READ32_MEMBER(tmsrnem_r) { return tmsrnem; }
	DECLARE_WRITE32_MEMBER(tmsrnem_w) { tmsrnem = data; }
	DECLARE_READ32_MEMBER(tmsrhog_r) { return tmsrhog; }
	DECLARE_WRITE32_MEMBER(tmsrhog_w) { tmsrhog = data; }
	DECLARE_READ32_MEMBER(tmscale_r) { return tmscale; }
	DECLARE_WRITE32_MEMBER(tmscale_w) { tmscale = data; }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	u32 sdepid, snapid, prilo, prihi;
	u32 errdomlo, errdomhi;
	u32 tmctrl, tmsrnem, tmsrhog, tmscale;

	u8 unknown0, unknown1;
};

// device type definition
extern const device_type INTERPRO_SRARB;

#endif // MAME_MACHINE_INTERPRO_SRARB_H
