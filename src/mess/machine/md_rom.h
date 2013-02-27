#ifndef __MD_ROM_H
#define __MD_ROM_H

#include "machine/md_slot.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> md_std_rom_device

class md_std_rom_device : public device_t,
						public device_md_cart_interface
{
public:
	// construction/destruction
	md_std_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);
	md_std_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() {};
	virtual void device_config_complete() { m_shortname = "md_std_rom"; }

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read) { if (offset < 0x400000/2) return m_rom[MD_ADDR(offset)]; else return 0xffff; };
	virtual DECLARE_WRITE16_MEMBER(write) { };
};

// ======================> md_rom_sram_device

class md_rom_sram_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_sram_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_config_complete() { m_shortname = "md_rom_sram"; }

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read);
	virtual DECLARE_WRITE16_MEMBER(write);
	virtual DECLARE_WRITE16_MEMBER(write_a13);
};

// ======================> md_rom_fram_device

class md_rom_fram_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_fram_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_config_complete() { m_shortname = "md_rom_fram"; }

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read);
	virtual DECLARE_WRITE16_MEMBER(write);
	virtual DECLARE_READ16_MEMBER(read_a13);
	virtual DECLARE_WRITE16_MEMBER(write_a13);
};

// ======================> md_rom_ssf2_device

class md_rom_ssf2_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_ssf2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual void device_config_complete() { m_shortname = "md_rom_ssf2"; }

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read);
	virtual DECLARE_WRITE16_MEMBER(write_a13);

private:
	UINT8 m_bank[16];
	int m_lastoff, m_lastdata;
};

// ======================> md_rom_mcpirate_device

class md_rom_mcpirate_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_mcpirate_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual void device_config_complete() { m_shortname = "md_rom_mcpirate"; }

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read);
	virtual DECLARE_WRITE16_MEMBER(write_a13);

private:
	UINT8 m_bank;
};


// ======================> md_rom_bugslife_device

class md_rom_bugslife_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_bugslife_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_config_complete() { m_shortname = "md_rom_bugslife"; }

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read_a13);
};

// ======================> md_rom_chinf3_device

class md_rom_chinf3_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_chinf3_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual void device_config_complete() { m_shortname = "md_rom_chinf3"; }

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read);
	virtual DECLARE_WRITE16_MEMBER(write);

private:
	int m_bank;
};

// ======================> md_rom_elfwor_device

class md_rom_elfwor_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_elfwor_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_config_complete() { m_shortname = "md_rom_elfwor"; }

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read);
};

// ======================> md_rom_yasech_device

class md_rom_yasech_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_yasech_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_config_complete() { m_shortname = "md_rom_yasech"; }

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read);
};

// ======================> md_rom_kof98_device

class md_rom_kof98_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_kof98_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_config_complete() { m_shortname = "md_rom_kof98"; }

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read);
};

// ======================> md_rom_kof99_device

class md_rom_kof99_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_kof99_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_config_complete() { m_shortname = "md_rom_kof99"; }

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read_a13);
};

// ======================> md_rom_lion2_device

class md_rom_lion2_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_lion2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual void device_config_complete() { m_shortname = "md_rom_lion2"; }

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read);
	virtual DECLARE_WRITE16_MEMBER(write);

private:
	UINT16 m_prot1_data, m_prot2_data;
};

// ======================> md_rom_lion3_device

class md_rom_lion3_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_lion3_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual void device_config_complete() { m_shortname = "md_rom_lion3"; }

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read);
	virtual DECLARE_WRITE16_MEMBER(write);

private:
	UINT8 m_reg[3];
	UINT16 m_bank;
};

// ======================> md_rom_mjlov_device

class md_rom_mjlov_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_mjlov_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_config_complete() { m_shortname = "md_rom_mjlov"; }

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read);
};

// ======================> md_rom_pokea_device

class md_rom_pokea_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_pokea_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	
	// device-level overrides
	virtual void device_config_complete() { m_shortname = "md_rom_pokea"; }
	
	// reading and writing
	virtual DECLARE_READ16_MEMBER(read_a13);
};

// ======================> md_rom_pokestad_device

class md_rom_pokestad_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_pokestad_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual void device_config_complete() { m_shortname = "md_rom_pokestad"; }

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read);
	virtual DECLARE_WRITE16_MEMBER(write);

private:
	UINT8 m_bank;
};

// ======================> md_rom_realtec_device

class md_rom_realtec_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_realtec_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual void device_config_complete() { m_shortname = "md_rom_realtec"; }

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read);
	virtual DECLARE_WRITE16_MEMBER(write);

private:
	UINT16 m_bank_addr, m_bank_size, m_old_bank_addr;
};

// ======================> md_rom_redcl_device

class md_rom_redcl_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_redcl_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_config_complete() { m_shortname = "md_rom_redcl"; }

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read);
};

// ======================> md_rom_rx3_device

class md_rom_rx3_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_rx3_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_config_complete() { m_shortname = "md_rom_rx3"; }

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read_a13);
};

// ======================> md_rom_sbubl_device

class md_rom_sbubl_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_sbubl_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_config_complete() { m_shortname = "md_rom_sbubl"; }

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read);
};

// ======================> md_rom_smb_device

class md_rom_smb_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_smb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_config_complete() { m_shortname = "md_rom_smb"; }

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read_a13);
};

// ======================> md_rom_smb2_device

class md_rom_smb2_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_smb2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_config_complete() { m_shortname = "md_rom_smb2"; }

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read_a13);
};

// ======================> md_rom_smw64_device

class md_rom_smw64_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_smw64_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	
	// device-level overrides
	virtual void device_start();
	virtual void device_config_complete() { m_shortname = "md_rom_smw64"; }
	
	// reading and writing
	virtual DECLARE_READ16_MEMBER(read);
	virtual DECLARE_WRITE16_MEMBER(write);
	
private:
	UINT32 m_latch0, m_latch1;
	UINT16 m_reg[6];
	UINT16 m_ctrl[3];
};

// ======================> md_rom_smouse_device

class md_rom_smouse_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_smouse_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_config_complete() { m_shortname = "md_rom_smouse"; }

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read);
};


// ======================> md_rom_soulb_device

class md_rom_soulb_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_soulb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_config_complete() { m_shortname = "md_rom_soulb"; }

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read);
};

// ======================> md_rom_squir_device

class md_rom_squir_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_squir_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual void device_config_complete() { m_shortname = "md_rom_squir"; }

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read);
	virtual DECLARE_WRITE16_MEMBER(write);

private:
	UINT16 m_latch;
};

// ======================> md_rom_topf_device

class md_rom_topf_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_topf_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual void device_config_complete() { m_shortname = "md_rom_topf"; }

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read);
	virtual DECLARE_WRITE16_MEMBER(write);

private:
	UINT16 m_latch;
	UINT8 m_bank[3];
};

// ======================> md_rom_radica_device

class md_rom_radica_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_radica_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual void device_config_complete() { m_shortname = "md_rom_radica"; }

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read);
	virtual DECLARE_READ16_MEMBER(read_a13);

private:
	UINT8 m_bank;
};

// ======================> md_rom_beggarp_device

class md_rom_beggarp_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_beggarp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	
	// device-level overrides
	virtual void device_start();
	virtual void device_config_complete() { m_shortname = "md_rom_beggarp"; }
	
	// reading and writing
	virtual DECLARE_READ16_MEMBER(read);
	virtual DECLARE_WRITE16_MEMBER(write);
	virtual DECLARE_WRITE16_MEMBER(write_a13);

	UINT8 m_mode, m_lock;
};

// ======================> md_rom_wukong_device

class md_rom_wukong_device : public md_std_rom_device
{
public:
	// construction/destruction
	md_rom_wukong_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	
	// device-level overrides
	virtual void device_start();
	virtual void device_config_complete() { m_shortname = "md_rom_wukong"; }
	
	// reading and writing
	virtual DECLARE_READ16_MEMBER(read);
	virtual DECLARE_WRITE16_MEMBER(write);
	virtual DECLARE_WRITE16_MEMBER(write_a13);
	
	UINT8 m_mode;
};



// device type definition
extern const device_type MD_STD_ROM;
extern const device_type MD_ROM_SRAM;
extern const device_type MD_ROM_FRAM;
extern const device_type MD_ROM_BUGSLIFE;
extern const device_type MD_ROM_CHINF3;
extern const device_type MD_ROM_ELFWOR;
extern const device_type MD_ROM_YASECH;
extern const device_type MD_ROM_KOF98;
extern const device_type MD_ROM_KOF99;
extern const device_type MD_ROM_LION2;
extern const device_type MD_ROM_LION3;
extern const device_type MD_ROM_MCPIR;
extern const device_type MD_ROM_MJLOV;
extern const device_type MD_ROM_POKEA;
extern const device_type MD_ROM_POKESTAD;
extern const device_type MD_ROM_REALTEC;
extern const device_type MD_ROM_REDCL;
extern const device_type MD_ROM_RX3;
extern const device_type MD_ROM_SBUBL;
extern const device_type MD_ROM_SMB;
extern const device_type MD_ROM_SMB2;
extern const device_type MD_ROM_SMW64;
extern const device_type MD_ROM_SMOUSE;
extern const device_type MD_ROM_SOULB;
extern const device_type MD_ROM_SSF2;
extern const device_type MD_ROM_SQUIR;
extern const device_type MD_ROM_TOPF;
extern const device_type MD_ROM_RADICA;
extern const device_type MD_ROM_BEGGARP;
extern const device_type MD_ROM_WUKONG;

#endif
