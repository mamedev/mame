// license:BSD-3-Clause
// copyright-holders:David Haywood

#pragma once

#ifndef __SEGACRPT_Z80__
#define __SEGACRPT_Z80__


#define MCFG_SEGACRPT_SET_DECRYPTED_TAG(_tag) \
	segacrpt_z80_device::set_decrypted_tag(*device, _tag);

#define MCFG_SEGACRPT_SET_DECRYPTED_PTR(_tag) \
	segacrpt_z80_device::set_decrypted_ptr(*device, _ptr);

#define MCFG_SEGACRPT_SET_SIZE(_size) \
	segacrpt_z80_device::set_size(*device, _size);

#define MCFG_SEGACRPT_SET_NUMBANKS(_numbanks) \
	segacrpt_z80_device::set_numbanks(*device, _numbanks);

#define MCFG_SEGACRPT_SET_BANKSIZE(_banksize) \
	segacrpt_z80_device::set_banksize(*device, _banksize);


#include "emu.h"
#include "cpu/z80/z80.h"

// base class
class segacrpt_z80_device : public z80_device
{
public:
	segacrpt_z80_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32);

	static void set_decrypted_tag(device_t &device, const char* decrypted_tag);
	static void set_decrypted_ptr(device_t &device, UINT8* ptr); // toprollr
	static void set_size(device_t &device, int size);
	static void set_numbanks(device_t &device, int _numbanks);
	static void set_banksize(device_t &device, int _banksize);

	const char*         m_decrypted_tag;
	UINT8* m_decrypted_ptr;
	UINT8* m_region_ptr;
	int m_decode_size;
	int m_numbanks;
	int m_banksize;
	bool m_decryption_done;

	void set_decrypted_p(UINT8* ptr);
	void set_region_p(UINT8* ptr);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void decrypt();
};


// actual encrypted CPUs
class sega_315_5132_device : public segacrpt_z80_device
{
public:
	sega_315_5132_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32);
protected:
	virtual void decrypt() override;
};

class sega_315_5155_device : public segacrpt_z80_device
{
public:
	sega_315_5155_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32);
protected:
	virtual void decrypt() override;
};

class sega_315_5110_device : public segacrpt_z80_device
{
public:
	sega_315_5110_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32);
protected:
	virtual void decrypt() override;
};

class sega_315_5135_device : public segacrpt_z80_device
{
public:
	sega_315_5135_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32);
protected:
	virtual void decrypt() override;
};

class sega_315_5051_device : public segacrpt_z80_device
{
public:
	sega_315_5051_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32);
protected:
	virtual void decrypt() override;
};

class sega_315_5098_device : public segacrpt_z80_device
{
public:
	sega_315_5098_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32);
protected:
	virtual void decrypt() override;
};

class sega_315_5102_device : public segacrpt_z80_device
{
public:
	sega_315_5102_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32);
protected:
	virtual void decrypt() override;
};

class sega_315_5065_device : public segacrpt_z80_device
{
public:
	sega_315_5065_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32);
protected:
	virtual void decrypt() override;
};


class sega_315_5064_device : public segacrpt_z80_device
{
public:
	sega_315_5064_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32);
protected:
	virtual void decrypt() override;
};


class sega_315_5033_device : public segacrpt_z80_device
{
public:
	sega_315_5033_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32);
protected:
	virtual void decrypt() override;
};

class sega_315_5041_device : public segacrpt_z80_device
{
public:
	sega_315_5041_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32);
protected:
	virtual void decrypt() override;
};

class sega_315_5048_device : public segacrpt_z80_device
{
public:
	sega_315_5048_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32);
protected:
	virtual void decrypt() override;
};

class sega_315_5093_device : public segacrpt_z80_device
{
public:
	sega_315_5093_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32);
protected:
	virtual void decrypt() override;
};

class sega_315_5099_device : public segacrpt_z80_device
{
public:
	sega_315_5099_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32);
protected:
	virtual void decrypt() override;
};

class sega_315_spat_device : public segacrpt_z80_device
{
public:
	sega_315_spat_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32);
protected:
	virtual void decrypt() override;
};

class sega_315_5015_device : public segacrpt_z80_device
{
public:
	sega_315_5015_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32);
protected:
	virtual void decrypt() override;
};


class sega_315_5133_device : public segacrpt_z80_device
{
public:
	sega_315_5133_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32);
protected:
	virtual void decrypt() override;
};

class sega_315_5014_device : public segacrpt_z80_device
{
public:
	sega_315_5014_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32);
protected:
	virtual void decrypt() override;
};

class sega_315_5013_device : public segacrpt_z80_device
{
public:
	sega_315_5013_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32);
protected:
	virtual void decrypt() override;
};

class sega_315_5061_device : public segacrpt_z80_device
{
public:
	sega_315_5061_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32);
protected:
	virtual void decrypt() override;
};



class sega_315_5018_device : public segacrpt_z80_device
{
public:
	sega_315_5018_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32);
protected:
	virtual void decrypt() override;
};

class sega_315_5010_device : public segacrpt_z80_device
{
public:
	sega_315_5010_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32);
protected:
	virtual void decrypt() override;
};



class sega_cpu_pbactio4_device : public segacrpt_z80_device
{
public:
	sega_cpu_pbactio4_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32);
protected:
	virtual void decrypt() override;
};

class sega_315_5028_device : public segacrpt_z80_device
{
public:
	sega_315_5028_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32);
protected:
	virtual void decrypt() override;
};

class sega_315_5084_device : public segacrpt_z80_device
{
public:
	sega_315_5084_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32);
protected:
	virtual void decrypt() override;
};


extern const device_type SEGA_315_5132;
extern const device_type SEGA_315_5155;
extern const device_type SEGA_315_5110;
extern const device_type SEGA_315_5135;
extern const device_type SEGA_315_5051;
extern const device_type SEGA_315_5098;
extern const device_type SEGA_315_5102;
extern const device_type SEGA_315_5065;
extern const device_type SEGA_315_5064;
extern const device_type SEGA_315_5033;
extern const device_type SEGA_315_5041;
extern const device_type SEGA_315_5048;
extern const device_type SEGA_315_5093;
extern const device_type SEGA_315_5099;
extern const device_type SEGA_315_SPAT;
extern const device_type SEGA_315_5015;
extern const device_type SEGA_315_5133;
extern const device_type SEGA_315_5014;
extern const device_type SEGA_315_5013;
extern const device_type SEGA_315_5061;
extern const device_type SEGA_315_5018;
extern const device_type SEGA_315_5010;
extern const device_type SEGA_CPU_PBACTIO4;
extern const device_type SEGA_315_5028;
extern const device_type SEGA_315_5084;


#endif /// __SEGACRPT_Z80__
