// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_INCLUDES_MEGADRIV_RAD_H
#define MAME_INCLUDES_MEGADRIV_RAD_H

class megadriv_radica_state : public md_base_state
{
public:
	megadriv_radica_state(const machine_config &mconfig, device_type type, const char *tag)
		: md_base_state(mconfig, type, tag),
		m_bank(0),
		m_romsize(0x400000),
		m_rom(*this, "maincpu")
	{}


	uint16_t read(offs_t offset);
	uint16_t read_a13(offs_t offset);

	void megadriv_radica_map(address_map &map);

protected:
	void megadriv_base_map(address_map &map);
	int m_bank;
	int m_romsize;

private:
	required_region_ptr<uint16_t> m_rom;
};

class megadriv_radica_3button_state : public megadriv_radica_state
{
public:
	megadriv_radica_3button_state(const machine_config& mconfig, device_type type, const char* tag)
		: megadriv_radica_state(mconfig, type, tag)
	{}
public:
	void megadriv_radica_3button_ntsc(machine_config &config);
	void megadriv_radica_3button_pal(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
};

class megadriv_radica_6button_state : public megadriv_radica_3button_state
{
public:
	void megadriv_radica_6button_ntsc(machine_config &config);
	void megadriv_radica_6button_pal(machine_config &config);

	void init_megadriv_radica_6button_pal();
	void init_megadriv_radica_6button_ntsc();

public:
	megadriv_radica_6button_state(const machine_config& mconfig, device_type type, const char* tag)
		: megadriv_radica_3button_state(mconfig, type, tag)
	{}
	virtual void machine_start() override;
};

class megadriv_dgunl_state : public megadriv_radica_3button_state
{
public:
	megadriv_dgunl_state(const machine_config& mconfig, device_type type, const char* tag)
		: megadriv_radica_3button_state(mconfig, type, tag)
	{}
public:
	void megadriv_dgunl_ntsc(machine_config &config);

	void init_dgunl3227();

protected:
	virtual void machine_start() override;
	uint16_t m_a1630a = 0;

private:
	uint16_t read_a16300(offs_t offset, uint16_t mem_mask);
	uint16_t read_a16302(offs_t offset, uint16_t mem_mask);
	virtual void write_a1630a(offs_t offset, uint16_t data, uint16_t mem_mask);

	void megadriv_dgunl_map(address_map &map);
};

class megadriv_ra145_state : public megadriv_dgunl_state
{
public:
	megadriv_ra145_state(const machine_config& mconfig, device_type type, const char* tag)
		: megadriv_dgunl_state(mconfig, type, tag)
	{}

	void init_ra145();

public:

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	virtual void write_a1630a(offs_t offset, uint16_t data, uint16_t mem_mask) override;
};



#endif // MAME_INCLUDES_MEGADRIV_RAD_H
