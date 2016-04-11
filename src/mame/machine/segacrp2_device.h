// license:BSD-3-Clause
// copyright-holders:David Haywood

#pragma once

#ifndef __SEGACRP2_Z80__
#define __SEGACRP2_Z80__


#define MCFG_SEGAZ80_SET_DECRYPTED_TAG(_tag) \
	segacrp2_z80_device::set_decrypted_tag(*device, _tag);

#include "emu.h"
#include "cpu/z80/z80.h"

// base class
class segacrp2_z80_device : public z80_device
{
public:
	segacrp2_z80_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32);

	static void set_decrypted_tag(device_t &device, const char* decrypted_tag);
	const char*         m_decrypted_tag;
protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void decrypt();
};



// actual encrypted CPUs
class sega_315_5179_device : public segacrp2_z80_device
{
public:
	sega_315_5179_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32);
protected:
	virtual void decrypt() override;
};


class sega_315_5178_device : public segacrp2_z80_device
{
public:
	sega_315_5178_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32);
protected:
	virtual void decrypt() override;
};

class sega_315_5177_device : public segacrp2_z80_device
{
public:
	sega_315_5177_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32);
protected:
	virtual void decrypt() override;
};

class sega_315_5176_device : public segacrp2_z80_device
{
public:
	sega_315_5176_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32);
protected:
	virtual void decrypt() override;
};

class sega_315_5162_device : public segacrp2_z80_device
{
public:
	sega_315_5162_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32);
protected:
	virtual void decrypt() override;
};

class sega_317_0004_device : public segacrp2_z80_device
{
public:
	sega_317_0004_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32);
protected:
	virtual void decrypt() override;
};


class sega_317_0005_device : public segacrp2_z80_device
{
public:
	sega_317_0005_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32);
protected:
	virtual void decrypt() override;
};


class sega_317_0006_device : public segacrp2_z80_device
{
public:
	sega_317_0006_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32);
protected:
	virtual void decrypt() override;
};

class sega_317_0007_device : public segacrp2_z80_device
{
public:
	sega_317_0007_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32);
protected:
	virtual void decrypt() override;
};


extern const device_type SEGACRP2_Z80;
extern const device_type SEGA_315_5179;
extern const device_type SEGA_315_5178;
extern const device_type SEGA_315_5177;
extern const device_type SEGA_315_5176;
extern const device_type SEGA_315_5162;

extern const device_type SEGA_317_0004;
extern const device_type SEGA_317_0005;
extern const device_type SEGA_317_0006;
extern const device_type SEGA_317_0007;


#endif /// __SEGACRP2_Z80__

