// license:BSD-3-Clause
// copyright-holders:David Haywood
/* */
#ifndef MAME_KONAMI_K055555_H
#define MAME_KONAMI_K055555_H

#pragma once


/* K055555 registers */
/* priority inputs */
#define K55_PALBASE_BG      0   // background palette
#define K55_CONTROL         1   // control register
#define K55_COLSEL_0        2   // layer A, B color depth
#define K55_COLSEL_1        3   // layer C, D color depth
#define K55_COLSEL_2        4   // object, S1 color depth
#define K55_COLSEL_3        5   // S2, S3 color depth

#define K55_PRIINP_0        7   // layer A pri 0
#define K55_PRIINP_1        8   // layer A pri 1
#define K55_PRIINP_2        9   // layer A "COLPRI"
#define K55_PRIINP_3        10  // layer B pri 0
#define K55_PRIINP_4        11  // layer B pri 1
#define K55_PRIINP_5        12  // layer B "COLPRI"
#define K55_PRIINP_6        13  // layer C pri
#define K55_PRIINP_7        14  // layer D pri
#define K55_PRIINP_8        15  // OBJ pri
#define K55_PRIINP_9        16  // sub 1 (GP:PSAC) pri
#define K55_PRIINP_10       17  // sub 2 (GX:PSAC) pri
#define K55_PRIINP_11       18  // sub 3 pri

#define K55_OINPRI_ON       19  // object priority bits selector

#define K55_PALBASE_A       23  // layer A palette
#define K55_PALBASE_B       24  // layer B palette
#define K55_PALBASE_C       25  // layer C palette
#define K55_PALBASE_D       26  // layer D palette
#define K55_PALBASE_OBJ     27  // OBJ palette
#define K55_PALBASE_SUB1    28  // SUB1 palette
#define K55_PALBASE_SUB2    29  // SUB2 palette
#define K55_PALBASE_SUB3    30  // SUB3 palette

#define K55_BLEND_ENABLES   33  // blend enables for tilemaps
#define K55_VINMIX_ON       34  // additional blend enables for tilemaps
#define K55_OSBLEND_ENABLES 35  // obj/sub blend enables
#define K55_OSBLEND_ON      36  // not sure, related to obj/sub blend

#define K55_SHAD1_PRI       37  // shadow/highlight 1 priority
#define K55_SHAD2_PRI       38  // shadow/highlight 2 priority
#define K55_SHAD3_PRI       39  // shadow/highlight 3 priority
#define K55_SHD_ON          40  // shadow/highlight
#define K55_SHD_PRI_SEL     41  // shadow/highlight

#define K55_VBRI            42  // VRAM layer brightness enable
#define K55_OSBRI           43  // obj/sub brightness enable, part 1
#define K55_OSBRI_ON        44  // obj/sub brightness enable, part 2
#define K55_INPUT_ENABLES   45  // input enables

/* bit masks for the control register */
#define K55_CTL_GRADDIR     0x01    // 0=vertical, 1=horizontal
#define K55_CTL_GRADENABLE  0x02    // 0=BG is base color only, 1=gradient
#define K55_CTL_FLIPPRI     0x04    // 0=standard Konami priority, 1=reverse
#define K55_CTL_SDSEL       0x08    // 0=normal shadow timing, 1=(not used by GX)

/* bit masks for the input enables */
#define K55_INP_VRAM_A      0x01
#define K55_INP_VRAM_B      0x02
#define K55_INP_VRAM_C      0x04
#define K55_INP_VRAM_D      0x08
#define K55_INP_OBJ         0x10
#define K55_INP_SUB1        0x20
#define K55_INP_SUB2        0x40
#define K55_INP_SUB3        0x80


class k055555_device : public device_t
{
public:
	k055555_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~k055555_device() { }

	void K055555_write_reg(uint8_t regnum, uint8_t regdat);

	void K055555_word_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void K055555_long_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	int K055555_read_register(int regnum);
	int K055555_get_palette_index(int idx);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint8_t   m_regs[128];
};

DECLARE_DEVICE_TYPE(K055555, k055555_device)


#endif // MAME_KONAMI_K055555_H
