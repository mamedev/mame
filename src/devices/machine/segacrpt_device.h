// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_MACHINE_SEGACRYPT_DEVICE_H
#define MAME_MACHINE_SEGACRYPT_DEVICE_H

#pragma once

#include "cpu/z80/z80.h"


// base class
class segacrpt_z80_device : public z80_device
{
public:
	void set_decrypted_tag(const char* decrypted_tag) { m_decrypted_tag = decrypted_tag; }
	void set_size(int size) { m_decode_size = size; }
	void set_numbanks(int numbanks) { m_numbanks = numbanks; }
	void set_banksize(int banksize) { m_banksize = banksize; }

	void set_decrypted_p(uint8_t* ptr);
	void set_region_p(uint8_t* ptr);

protected:
	segacrpt_z80_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void decrypt() = 0;

	const char* m_decrypted_tag = nullptr;
	uint8_t* m_decrypted_ptr;
	uint8_t* m_region_ptr;
	int m_decode_size;
	int m_numbanks;
	int m_banksize;

private:
	bool m_decryption_done;
};


// actual encrypted CPUs
class sega_315_5132_device : public segacrpt_z80_device
{
public:
	sega_315_5132_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t);
protected:
	virtual void decrypt() override;
};

class sega_315_5155_device : public segacrpt_z80_device
{
public:
	sega_315_5155_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t);
protected:
	virtual void decrypt() override;
};

class sega_315_5110_device : public segacrpt_z80_device
{
public:
	sega_315_5110_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t);
protected:
	virtual void decrypt() override;
};

class sega_315_5135_device : public segacrpt_z80_device
{
public:
	sega_315_5135_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t);
protected:
	virtual void decrypt() override;
};

class sega_315_5051_device : public segacrpt_z80_device
{
public:
	sega_315_5051_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t);
protected:
	virtual void decrypt() override;
};

class sega_315_5098_device : public segacrpt_z80_device
{
public:
	sega_315_5098_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t);
protected:
	virtual void decrypt() override;
};

class sega_315_5102_device : public segacrpt_z80_device
{
public:
	sega_315_5102_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t);
protected:
	virtual void decrypt() override;
};

class sega_315_5065_device : public segacrpt_z80_device
{
public:
	sega_315_5065_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t);
protected:
	virtual void decrypt() override;
};


class sega_315_5064_device : public segacrpt_z80_device
{
public:
	sega_315_5064_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t);
protected:
	virtual void decrypt() override;
};


class sega_315_5033_device : public segacrpt_z80_device
{
public:
	sega_315_5033_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t);
protected:
	virtual void decrypt() override;
};

class sega_315_5041_device : public segacrpt_z80_device
{
public:
	sega_315_5041_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t);
protected:
	virtual void decrypt() override;
};

class sega_315_5048_device : public segacrpt_z80_device
{
public:
	sega_315_5048_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t);
	sega_315_5048_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void decrypt() override;
};

class sega_315_5093_device : public segacrpt_z80_device
{
public:
	sega_315_5093_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t);
protected:
	virtual void decrypt() override;
};

class sega_315_5099_device : public segacrpt_z80_device
{
public:
	sega_315_5099_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t);
protected:
	virtual void decrypt() override;
};

class sega_315_5006_device : public segacrpt_z80_device
{
public:
	sega_315_5006_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t);
protected:
	sega_315_5006_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt() override;
};

class sega_315_5096_device : public sega_315_5006_device
{
public:
	sega_315_5096_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t);
};

class sega_315_5111_device : public sega_315_5006_device
{
public:
	sega_315_5111_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t);
};

class sega_315_5015_device : public segacrpt_z80_device
{
public:
	sega_315_5015_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t);
protected:
	virtual void decrypt() override;
};


class sega_315_5133_device : public sega_315_5048_device
{
public:
	sega_315_5133_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t);
};

class sega_315_5014_device : public segacrpt_z80_device
{
public:
	sega_315_5014_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t);
protected:
	virtual void decrypt() override;
};

class sega_315_5013_device : public segacrpt_z80_device
{
public:
	sega_315_5013_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t);
protected:
	virtual void decrypt() override;
};

class sega_315_5061_device : public segacrpt_z80_device
{
public:
	sega_315_5061_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t);
protected:
	virtual void decrypt() override;
};



class sega_315_5018_device : public segacrpt_z80_device
{
public:
	sega_315_5018_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t);
protected:
	virtual void decrypt() override;
};

class sega_315_5010_device : public segacrpt_z80_device
{
public:
	sega_315_5010_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t);
protected:
	virtual void decrypt() override;
};



class sega_315_5128_device : public segacrpt_z80_device
{
public:
	sega_315_5128_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t);
protected:
	virtual void decrypt() override;
};

class sega_315_5028_device : public segacrpt_z80_device
{
public:
	sega_315_5028_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t);
protected:
	virtual void decrypt() override;
};

class sega_315_5084_device : public segacrpt_z80_device
{
public:
	sega_315_5084_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t);
protected:
	virtual void decrypt() override;
};


DECLARE_DEVICE_TYPE(SEGA_315_5132, sega_315_5132_device)
DECLARE_DEVICE_TYPE(SEGA_315_5155, sega_315_5155_device)
DECLARE_DEVICE_TYPE(SEGA_315_5110, sega_315_5110_device)
DECLARE_DEVICE_TYPE(SEGA_315_5135, sega_315_5135_device)
DECLARE_DEVICE_TYPE(SEGA_315_5051, sega_315_5051_device)
DECLARE_DEVICE_TYPE(SEGA_315_5098, sega_315_5098_device)
DECLARE_DEVICE_TYPE(SEGA_315_5102, sega_315_5102_device)
DECLARE_DEVICE_TYPE(SEGA_315_5065, sega_315_5065_device)
DECLARE_DEVICE_TYPE(SEGA_315_5064, sega_315_5064_device)
DECLARE_DEVICE_TYPE(SEGA_315_5033, sega_315_5033_device)
DECLARE_DEVICE_TYPE(SEGA_315_5041, sega_315_5041_device)
DECLARE_DEVICE_TYPE(SEGA_315_5048, sega_315_5048_device)
DECLARE_DEVICE_TYPE(SEGA_315_5093, sega_315_5093_device)
DECLARE_DEVICE_TYPE(SEGA_315_5099, sega_315_5099_device)
DECLARE_DEVICE_TYPE(SEGA_315_5006, sega_315_5006_device)
DECLARE_DEVICE_TYPE(SEGA_315_5096, sega_315_5096_device)
DECLARE_DEVICE_TYPE(SEGA_315_5111, sega_315_5111_device)
DECLARE_DEVICE_TYPE(SEGA_315_5015, sega_315_5015_device)
DECLARE_DEVICE_TYPE(SEGA_315_5133, sega_315_5133_device)
DECLARE_DEVICE_TYPE(SEGA_315_5014, sega_315_5014_device)
DECLARE_DEVICE_TYPE(SEGA_315_5013, sega_315_5013_device)
DECLARE_DEVICE_TYPE(SEGA_315_5061, sega_315_5061_device)
DECLARE_DEVICE_TYPE(SEGA_315_5018, sega_315_5018_device)
DECLARE_DEVICE_TYPE(SEGA_315_5010, sega_315_5010_device)
DECLARE_DEVICE_TYPE(SEGA_315_5128, sega_315_5128_device)
DECLARE_DEVICE_TYPE(SEGA_315_5028, sega_315_5028_device)
DECLARE_DEVICE_TYPE(SEGA_315_5084, sega_315_5084_device)


#endif // MAME_MACHINE_SEGACRYPT_DEVICE_H
