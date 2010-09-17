/*
    Intel Flash ROM emulation
*/

#ifndef _INTELFLASH_H_
#define _INTELFLASH_H_


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MDRV_INTEL_28F016S5_ADD(_tag) \
	MDRV_DEVICE_ADD(_tag, INTEL_28F016S5, 0)

#define MDRV_SHARP_LH28F016S_ADD(_tag) \
	MDRV_DEVICE_ADD(_tag, SHARP_LH28F016S, 0)

#define MDRV_FUJITSU_29F016A_ADD(_tag) \
	MDRV_DEVICE_ADD(_tag, FUJITSU_29F016A, 0)

#define MDRV_INTEL_E28F400_ADD(_tag) \
	MDRV_DEVICE_ADD(_tag, INTEL_E28F400, 0)

#define MDRV_MACRONIX_29L001MC_ADD(_tag) \
	MDRV_DEVICE_ADD(_tag, MACRONIX_29L001MC, 0)

#define MDRV_PANASONIC_MN63F805MNP_ADD(_tag) \
	MDRV_DEVICE_ADD(_tag, PANASONIC_MN63F805MNP, 0)

#define MDRV_SANYO_LE26FV10N1TS_ADD(_tag) \
	MDRV_DEVICE_ADD(_tag, SANYO_LE26FV10N1TS, 0)

#define MDRV_SHARP_LH28F400_ADD(_tag) \
	MDRV_DEVICE_ADD(_tag, SHARP_LH28F400, 0)

#define MDRV_INTEL_E28F008SA_ADD(_tag) \
	MDRV_DEVICE_ADD(_tag, INTEL_E28F008SA, 0)

#define MDRV_INTEL_TE28F160_ADD(_tag) \
	MDRV_DEVICE_ADD(_tag, INTEL_TE28F160, 0)

#define MDRV_SHARP_UNK128MBIT_ADD(_tag) \
	MDRV_DEVICE_ADD(_tag, SHARP_UNK128MBIT, 0)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class intelfsh_device;


// ======================> intelfsh_device_config

class intelfsh_device_config :	public device_config,
								public device_config_memory_interface,
								public device_config_nvram_interface
{
	friend class intelfsh_device;

protected:
	// constants
	enum
	{
		// 8-bit variants
		FLASH_INTEL_28F016S5 = 0x0800,
		FLASH_FUJITSU_29F016A,
		FLASH_SHARP_LH28F016S,
		FLASH_INTEL_E28F400,
		FLASH_MACRONIX_29L001MC,
		FLASH_PANASONIC_MN63F805MNP,
		FLASH_SANYO_LE26FV10N1TS,

		// 16-bit variants
		FLASH_SHARP_LH28F400 = 0x1000,
		FLASH_INTEL_E28F008SA,
		FLASH_INTEL_TE28F160,
		FLASH_SHARP_UNK128MBIT
	};

	// construction/destruction
	intelfsh_device_config(const machine_config &mconfig, device_type type, const char *name, const char *tag, const device_config *owner, UINT32 clock, UINT32 variant);

	// device_config_memory_interface overrides
	virtual const address_space_config *memory_space_config(int spacenum = 0) const;

	// internal state
	address_space_config	m_space_config;
	UINT32		 			m_type;
	INT32 					m_size;
	UINT8 					m_bits;
	UINT8 					m_device_id;
	UINT8 					m_maker_id;
	bool 					m_sector_is_4k;
};


// ======================> intelfsh_device

class intelfsh_device :	public device_t,
						public device_memory_interface,
						public device_nvram_interface
{
	friend class intelfsh_device_config;

protected:
	// construction/destruction
	intelfsh_device(running_machine &_machine, const intelfsh_device_config &config);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// device_config_nvram_interface overrides
	virtual void nvram_default();
	virtual void nvram_read(mame_file &file);
	virtual void nvram_write(mame_file &file);
	
	// derived helpers
	UINT32 read_full(UINT32 offset);
	void write_full(UINT32 offset, UINT32 data);

	// internal state
	const intelfsh_device_config &	m_config;

	UINT8 						m_status;
	INT32 						m_erase_sector;
	INT32 						m_flash_mode;
	bool 						m_flash_master_lock;
	emu_timer *					m_timer;
};


// ======================> intelfsh8_device_config

class intelfsh8_device_config : public intelfsh_device_config
{
	friend class intelfsh8_device;

protected:
	// construction/destruction
	intelfsh8_device_config(const machine_config &mconfig, device_type type, const char *name, const char *tag, const device_config *owner, UINT32 clock, UINT32 variant);
};


// ======================> intelfsh8_device

class intelfsh8_device : public intelfsh_device
{
	friend class intelfsh8_device_config;
	friend class intel_28f016s5_device_config;
	friend class fujitsu_29f016a_device_config;
	friend class sharp_lh28f016s_device_config;
	friend class intel_e28f008sa_device_config;
	friend class macronix_29l001mc_device_config;
	friend class panasonic_mn63f805mnp_device_config;
	friend class sanyo_le26fv10n1ts_device_config;

protected:
	// construction/destruction
	intelfsh8_device(running_machine &_machine, const intelfsh_device_config &config);

public:
	// public interface
	UINT8 read(offs_t offset) { return read_full(offset); }
	void write(offs_t offset, UINT8 data) { write_full(offset, data); }
	
	UINT8 read_raw(offs_t offset) { return m_addrspace[0]->read_byte(offset); }
	void write_raw(offs_t offset, UINT8 data) { m_addrspace[0]->write_byte(offset, data); }
};


// ======================> intelfsh16_device_config

class intelfsh16_device_config : public intelfsh_device_config
{
	friend class intelfsh16_device;

protected:
	// construction/destruction
	intelfsh16_device_config(const machine_config &mconfig, device_type type, const char *name, const char *tag, const device_config *owner, UINT32 clock, UINT32 variant);
};


// ======================> intelfsh16_device

class intelfsh16_device : public intelfsh_device
{
	friend class intelfsh16_device_config;
	friend class sharp_lh28f400_device_config;
	friend class intel_te28f160_device_config;
	friend class intel_e28f400_device_config;
	friend class sharp_unk128mbit_device_config;

protected:
	// construction/destruction
	intelfsh16_device(running_machine &_machine, const intelfsh_device_config &config);

public:
	// public interface
	UINT16 read(offs_t offset) { return read_full(offset); }
	void write(offs_t offset, UINT16 data) { write_full(offset, data); }
	
	UINT16 read_raw(offs_t offset) { return m_addrspace[0]->read_word(offset * 2); }
	void write_raw(offs_t offset, UINT16 data) { m_addrspace[0]->write_word(offset * 2, data); }
};


// ======================> trivial variants

// 8-bit variants
DECLARE_TRIVIAL_DERIVED_DEVICE(intel_28f016s5_device_config, intelfsh8_device_config, intel_28f016s5_device, intelfsh8_device)
DECLARE_TRIVIAL_DERIVED_DEVICE(fujitsu_29f016a_device_config, intelfsh8_device_config, fujitsu_29f016a_device, intelfsh8_device)
DECLARE_TRIVIAL_DERIVED_DEVICE(sharp_lh28f016s_device_config, intelfsh8_device_config, sharp_lh28f016s_device, intelfsh8_device)
DECLARE_TRIVIAL_DERIVED_DEVICE(intel_e28f008sa_device_config, intelfsh8_device_config, intel_e28f008sa_device, intelfsh8_device)
DECLARE_TRIVIAL_DERIVED_DEVICE(macronix_29l001mc_device_config, intelfsh8_device_config, macronix_29l001mc_device, intelfsh8_device)
DECLARE_TRIVIAL_DERIVED_DEVICE(panasonic_mn63f805mnp_device_config, intelfsh8_device_config, panasonic_mn63f805mnp_device, intelfsh8_device)
DECLARE_TRIVIAL_DERIVED_DEVICE(sanyo_le26fv10n1ts_device_config, intelfsh8_device_config, sanyo_le26fv10n1ts_device, intelfsh8_device)


// 16-bit variants
DECLARE_TRIVIAL_DERIVED_DEVICE(sharp_lh28f400_device_config, intelfsh16_device_config, sharp_lh28f400_device, intelfsh16_device)
DECLARE_TRIVIAL_DERIVED_DEVICE(intel_te28f160_device_config, intelfsh16_device_config, intel_te28f160_device, intelfsh16_device)
DECLARE_TRIVIAL_DERIVED_DEVICE(intel_e28f400_device_config, intelfsh16_device_config, intel_e28f400_device, intelfsh16_device)
DECLARE_TRIVIAL_DERIVED_DEVICE(sharp_unk128mbit_device_config, intelfsh16_device_config, sharp_unk128mbit_device, intelfsh16_device)
		


// device type definition
extern const device_type INTEL_28F016S5;
extern const device_type SHARP_LH28F016S;
extern const device_type FUJITSU_29F016A;
extern const device_type INTEL_E28F400;
extern const device_type MACRONIX_29L001MC;
extern const device_type PANASONIC_MN63F805MNP;
extern const device_type SANYO_LE26FV10N1TS;

extern const device_type SHARP_LH28F400;
extern const device_type INTEL_E28F008SA;
extern const device_type INTEL_TE28F160;
extern const device_type SHARP_UNK128MBIT;


#endif
