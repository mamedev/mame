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

	DECLARE_READ32_MEMBER(tmctrl_r) { return tmctrl; }
	DECLARE_WRITE32_MEMBER(tmctrl_w) { tmctrl = data; }
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
};

// device type definition
extern const device_type INTERPRO_SRARB;

#endif // MAME_MACHINE_INTERPRO_SRARB_H
