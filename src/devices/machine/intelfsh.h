// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*
    Intel Flash ROM emulation
*/

#ifndef _INTELFLASH_H_
#define _INTELFLASH_H_


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_INTEL_28F016S5_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, INTEL_28F016S5, 0)

#define MCFG_SHARP_LH28F016S_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, SHARP_LH28F016S, 0)

#define MCFG_ATMEL_29C010_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, ATMEL_29C010, 0)

#define MCFG_AMD_29F010_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, AMD_29F010, 0)

#define MCFG_AMD_29F040_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, AMD_29F040, 0)

#define MCFG_AMD_29F080_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, AMD_29F080, 0)

#define MCFG_AMD_29F400T_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, AMD_29F400T, 0)

#define MCFG_AMD_29F800T_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, AMD_29F800T, 0)

#define MCFG_AMD_29LV200T_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, AMD_29LV200T, 0)

#define MCFG_FUJITSU_29F160T_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, FUJITSU_29F160T, 0)

#define MCFG_FUJITSU_29F016A_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, FUJITSU_29F016A, 0)

#define MCFG_FUJITSU_29DL16X_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, FUJITSU_29DL16X, 0)

#define MCFG_INTEL_E28F400B_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, INTEL_E28F400B, 0)

#define MCFG_MACRONIX_29L001MC_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, MACRONIX_29L001MC, 0)

#define MCFG_MACRONIX_29LV160TMC_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, MACRONIX_29LV160TMC, 0)

#define MCFG_PANASONIC_MN63F805MNP_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, PANASONIC_MN63F805MNP, 0)

#define MCFG_SANYO_LE26FV10N1TS_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, SANYO_LE26FV10N1TS, 0)

#define MCFG_SST_28SF040_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, SST_28SF040, 0)

#define MCFG_SST_39VF020_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, SST_39VF020, 0)

#define MCFG_SHARP_LH28F400_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, SHARP_LH28F400, 0)

#define MCFG_INTEL_E28F008SA_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, INTEL_E28F008SA, 0)

#define MCFG_INTEL_TE28F160_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, INTEL_TE28F160, 0)

#define MCFG_INTEL_TE28F320_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, INTEL_TE28F320, 0)

#define MCFG_SHARP_UNK128MBIT_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, SHARP_UNK128MBIT, 0)

#define MCFG_INTEL_28F320J3D_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, INTEL_28F320J3D, 0)

#define MCFG_INTEL_28F320J5_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, INTEL_28F320J5, 0)

#define MCFG_SST_39VF400A_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, SST_39VF400A, 0)

#define MCFG_TMS_29F040_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, TMS_29F040, 0)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class intelfsh_device;


// ======================> intelfsh_device

class intelfsh_device : public device_t,
						public device_memory_interface,
						public device_nvram_interface
{
public:
	enum
	{
		// 8-bit variants
		FLASH_INTEL_28F016S5 = 0x0800,
		FLASH_FUJITSU_29F160T,
		FLASH_FUJITSU_29F016A,
		FLASH_FUJITSU_29DL16X,
		FLASH_ATMEL_29C010,
		FLASH_AMD_29F010,
		FLASH_AMD_29F040,
		FLASH_AMD_29F080,
		FLASH_AMD_29F400T,
		FLASH_AMD_29F800T,
		FLASH_AMD_29LV200T,
		FLASH_SHARP_LH28F016S,
		FLASH_INTEL_E28F008SA,
		FLASH_MACRONIX_29L001MC,
		FLASH_MACRONIX_29LV160TMC,
		FLASH_PANASONIC_MN63F805MNP,
		FLASH_SANYO_LE26FV10N1TS,
		FLASH_SST_28SF040,
		FLASH_SST_39VF020,
		FLASH_TMS_29F040,

		// 16-bit variants
		FLASH_SHARP_LH28F400 = 0x1000,
		FLASH_INTEL_E28F400B,
		FLASH_INTEL_TE28F160,
		FLASH_INTEL_TE28F320,
		FLASH_SHARP_UNK128MBIT,
		FLASH_INTEL_28F320J3D,
		FLASH_INTEL_28F320J5,
		FLASH_SST_39VF400A
	};

protected:
	// construction/destruction
	intelfsh_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT32 variant, const char *shortname, const char *source);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual void nvram_read(emu_file &file) override;
	virtual void nvram_write(emu_file &file) override;

	// derived helpers
	UINT32 read_full(UINT32 offset);
	void write_full(UINT32 offset, UINT32 data);

	// configuration state
	address_space_config    m_space_config;
	UINT32                  m_type;
	INT32                   m_size;
	UINT8                   m_bits;
	UINT32                  m_addrmask;
	UINT16                  m_device_id;
	UINT8                   m_maker_id;
	bool                    m_sector_is_4k;
	bool                    m_sector_is_16k;
	bool                    m_top_boot_sector;
	UINT8                   m_page_size;

	// internal state
	UINT8                   m_status;
	INT32                   m_erase_sector;
	INT32                   m_flash_mode;
	bool                    m_flash_master_lock;
	emu_timer *             m_timer;
	INT32                   m_bank;
	UINT8                   m_byte_count;
};


// ======================> intelfsh8_device

class intelfsh8_device : public intelfsh_device
{
protected:
	// construction/destruction
	intelfsh8_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT32 variant, const char *shortname, const char *source);

public:
	// public interface
	UINT8 read(offs_t offset) { return read_full(offset); }
	void write(offs_t offset, UINT8 data) { write_full(offset, data); }
	DECLARE_READ8_MEMBER(read) { return read_full(offset); }
	DECLARE_WRITE8_MEMBER(write) { write_full(offset, data); }

	UINT8 read_raw(offs_t offset) { return m_addrspace[0]->read_byte(offset); }
	void write_raw(offs_t offset, UINT8 data) { m_addrspace[0]->write_byte(offset, data); }
};


// ======================> intelfsh16_device

class intelfsh16_device : public intelfsh_device
{
protected:
	// construction/destruction
	intelfsh16_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT32 variant, const char *shortname, const char *source);

public:
	// public interface
	UINT16 read(offs_t offset) { return read_full(offset); }
	void write(offs_t offset, UINT16 data) { write_full(offset, data); }
	DECLARE_READ16_MEMBER(read) { return read_full(offset); }
	DECLARE_WRITE16_MEMBER(write) { write_full(offset, data); }

	UINT16 read_raw(offs_t offset) { return m_addrspace[0]->read_word(offset * 2); }
	void write_raw(offs_t offset, UINT16 data) { m_addrspace[0]->write_word(offset * 2, data); }
};


// ======================> trivial variants

// 8-bit variants
class intel_28f016s5_device : public intelfsh8_device
{
public:
	intel_28f016s5_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class fujitsu_29f160t_device : public intelfsh8_device
{
public:
	fujitsu_29f160t_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class fujitsu_29f016a_device : public intelfsh8_device
{
public:
	fujitsu_29f016a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class fujitsu_29dl16x_device : public intelfsh8_device
{
public:
	fujitsu_29dl16x_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class atmel_29c010_device : public intelfsh8_device
{
public:
	atmel_29c010_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class amd_29f010_device : public intelfsh8_device
{
public:
	amd_29f010_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class amd_29f040_device : public intelfsh8_device
{
public:
	amd_29f040_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class amd_29f080_device : public intelfsh8_device
{
public:
	amd_29f080_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class amd_29f400t_device : public intelfsh8_device
{
public:
	amd_29f400t_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class amd_29f800t_device : public intelfsh8_device
{
public:
	amd_29f800t_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class amd_29lv200t_device : public intelfsh8_device
{
public:
	amd_29lv200t_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class sharp_lh28f016s_device : public intelfsh8_device
{
public:
	sharp_lh28f016s_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class intel_e28f008sa_device : public intelfsh8_device
{
public:
	intel_e28f008sa_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class macronix_29l001mc_device : public intelfsh8_device
{
public:
	macronix_29l001mc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class macronix_29lv160tmc_device : public intelfsh8_device
{
public:
	macronix_29lv160tmc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class panasonic_mn63f805mnp_device : public intelfsh8_device
{
public:
	panasonic_mn63f805mnp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class sanyo_le26fv10n1ts_device : public intelfsh8_device
{
public:
	sanyo_le26fv10n1ts_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class sst_28sf040_device : public intelfsh8_device
{
public:
	sst_28sf040_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class sst_39vf020_device : public intelfsh8_device
{
public:
	sst_39vf020_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class tms_29f040_device : public intelfsh8_device
{
public:
	tms_29f040_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

// 16-bit variants
class sharp_lh28f400_device : public intelfsh16_device
{
public:
	sharp_lh28f400_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class intel_te28f160_device : public intelfsh16_device
{
public:
	intel_te28f160_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class intel_te28f320_device : public intelfsh16_device
{
public:
	intel_te28f320_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class intel_e28f400b_device : public intelfsh16_device
{
public:
	intel_e28f400b_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class sharp_unk128mbit_device : public intelfsh16_device
{
public:
	sharp_unk128mbit_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class intel_28f320j3d_device : public intelfsh16_device
{
public:
	intel_28f320j3d_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class intel_28f320j5_device : public intelfsh16_device
{
public:
	intel_28f320j5_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class sst_39vf400a_device : public intelfsh16_device
{
public:
	sst_39vf400a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


// device type definition
extern const device_type INTEL_28F016S5;
extern const device_type SHARP_LH28F016S;
extern const device_type ATMEL_29C010;
extern const device_type AMD_29F010;
extern const device_type AMD_29F040;
extern const device_type AMD_29F080;
extern const device_type AMD_29F400T;
extern const device_type AMD_29F800T;
extern const device_type AMD_29LV200T;
extern const device_type FUJITSU_29F160T;
extern const device_type FUJITSU_29F016A;
extern const device_type FUJITSU_29DL16X;
extern const device_type INTEL_E28F400B;
extern const device_type MACRONIX_29L001MC;
extern const device_type MACRONIX_29LV160TMC;
extern const device_type TMS_29F040;

extern const device_type PANASONIC_MN63F805MNP;
extern const device_type SANYO_LE26FV10N1TS;
extern const device_type SST_28SF040;
extern const device_type SST_39VF020;

extern const device_type SHARP_LH28F400;
extern const device_type INTEL_E28F008SA;
extern const device_type INTEL_TE28F160;
extern const device_type INTEL_TE28F320;
extern const device_type SHARP_UNK128MBIT;
extern const device_type INTEL_28F320J3D;
extern const device_type INTEL_28F320J5;
extern const device_type SST_39VF400A;

#endif
