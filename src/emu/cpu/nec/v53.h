/* V53 */

#include "nec.h"
#include "necpriv.h"

class v53_base_device : public nec_common_device
{
public:
	v53_base_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, offs_t fetch_xor, UINT8 prefetch_size, UINT8 prefetch_cycles, UINT32 chip_type);

	DECLARE_WRITE8_MEMBER(BSEL_w);
	DECLARE_WRITE8_MEMBER(BADR_w);
	DECLARE_WRITE8_MEMBER(BRC_w);
	DECLARE_WRITE8_MEMBER(WMB0_w);
	DECLARE_WRITE8_MEMBER(WCY1_w);
	DECLARE_WRITE8_MEMBER(WCY0_w);
	DECLARE_WRITE8_MEMBER(WAC_w);
	DECLARE_WRITE8_MEMBER(TCKS_w);
	DECLARE_WRITE8_MEMBER(SBCR_w);
	DECLARE_WRITE8_MEMBER(REFC_w);
	DECLARE_WRITE8_MEMBER(WMB1_w);
	DECLARE_WRITE8_MEMBER(WCY2_w);
	DECLARE_WRITE8_MEMBER(WCY3_w);
	DECLARE_WRITE8_MEMBER(WCY4_w);
	DECLARE_WRITE8_MEMBER(SULA_w);
	DECLARE_WRITE8_MEMBER(TULA_w);
	DECLARE_WRITE8_MEMBER(IULA_w);
	DECLARE_WRITE8_MEMBER(DULA_w);
	DECLARE_WRITE8_MEMBER(OPHA_w);
	DECLARE_WRITE8_MEMBER(OPSEL_w);
	DECLARE_WRITE8_MEMBER(SCTL_w);

	UINT8 m_SCTL;
	UINT8 m_OPSEL;
	
	UINT8 m_SULA;
	UINT8 m_TULA;
	UINT8 m_IULA;
	UINT8 m_DULA;
	UINT8 m_OPHA;

	// TMU
	DECLARE_READ8_MEMBER(tmu_tst0_r);
	DECLARE_WRITE8_MEMBER(tmu_tct0_w);
	DECLARE_READ8_MEMBER(tmu_tst1_r);
	DECLARE_WRITE8_MEMBER(tmu_tct1_w);
	DECLARE_READ8_MEMBER(tmu_tst2_r);
	DECLARE_WRITE8_MEMBER(tmu_tct2_w);
	DECLARE_WRITE8_MEMBER(tmu_tmd_w);

	void install_peripheral_io();

	const address_space_config m_io_space_config;
	
	const address_space_config *memory_space_config(address_spacenum spacenum) const
	{
		switch (spacenum)
		{
			case AS_IO: return &m_io_space_config;
			default: return nec_common_device::memory_space_config(spacenum);
		}
	}

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
};


class v53_device : public v53_base_device
{
public:
	v53_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class v53a_device : public v53_base_device
{
public:
	v53a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type V53;
extern const device_type V53A;
