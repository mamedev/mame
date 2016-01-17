// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Tecmo Mixer */



class tecmo_mix_device : public device_t,
						public device_video_interface
{
public:
	tecmo_mix_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	void mix_bitmaps(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, palette_device* palette, bitmap_ind16* bitmap_bg, bitmap_ind16* bitmap_fg, bitmap_ind16* bitmap_tx, bitmap_ind16* bitmap_sp);
	static void set_mixer_shifts(device_t &device, int sprpri_shift, int sprbln_shift, int sprcol_shift);
	static void set_blendcols(device_t &device, int bgblend_comp, int fgblend_comp, int txblend_comp, int spblend_comp);
	static void set_regularcols(device_t &device, int bgregular_comp, int fgregular_comp, int txregular_comp, int spregular_comp);
	static void set_blendsource(device_t &device, int spblend_source, int fgblend_source);
	static void set_revspritetile(device_t &device);
	static void set_bgpen(device_t &device, int bgpen);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	// mixer shifts
	int m_sprpri_shift;
	int m_sprbln_shift;
	int m_sprcol_shift;

	// when the blend bit is specified in the attribute the source blend palette for that sprite / fg pixel comes from these offsets instead
	int m_spblend_source;
	int m_fgblend_source;
	// the second blend component depends on the pixel we are blending with, the following palettes get used instead of the regular ones
	int m_bgblend_comp;
	int m_fgblend_comp;
	int m_txblend_comp;
	int m_spblend_comp;

	// otherwise the regular palettes are
	int m_bgregular_comp;
	int m_fgregular_comp;
	int m_txregular_comp;
	int m_spregular_comp;

	int m_revspritetile;
	int m_bgpen;

private:



};

extern const device_type TECMO_MIXER;



#define MCFG_TECMO_MIXER_SHIFTS(_sprpri_shift, _sprbln_shift, _sprcol_shift) \
	tecmo_mix_device::set_mixer_shifts(*device, _sprpri_shift, _sprbln_shift, _sprcol_shift);

#define MCFG_TECMO_MIXER_BLENDCOLS(_bgblend_comp, _fgblend_comp, _txblend_comp, _spblend_comp) \
	tecmo_mix_device::set_blendcols(*device, _bgblend_comp, _fgblend_comp, _txblend_comp, _spblend_comp);

#define MCFG_TECMO_MIXER_REGULARCOLS(_bgregular_comp, _fgregular_comp, _txregular_comp, _spregular_comp) \
	tecmo_mix_device::set_regularcols(*device, _bgregular_comp, _fgregular_comp, _txregular_comp, _spregular_comp);

#define MCFG_TECMO_MIXER_BLENDSOUCE(_spblend_source, _fgblend_source) \
	tecmo_mix_device::set_blendsource(*device, _spblend_source, _fgblend_source);

#define MCFG_TECMO_MIXER_REVSPRITETILE \
	tecmo_mix_device::set_revspritetile(*device);

#define MCFG_TECMO_MIXER_BGPEN(_bgpen) \
	tecmo_mix_device::set_bgpen(*device, _bgpen);
