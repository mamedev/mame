// license:LGPL-2.1+
// copyright-holders:Angelo Salese
class nb1414m4_device : public device_t,
									public device_video_interface
{
public:
	nb1414m4_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~nb1414m4_device() {}

	void exec(UINT16 mcu_cmd, UINT8 *vram, UINT16 &scrollx, UINT16 &scrolly, tilemap_t *tilemap);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	void dma(UINT16 src, UINT16 dst, UINT16 size, UINT8 condition, UINT8 *vram);
	void fill(UINT16 dst, UINT8 tile, UINT8 pal, UINT8 *vram);
	void insert_coin_msg(UINT8 *vram);
	void credit_msg(UINT8 *vram);
	void kozure_score_msg(UINT16 dst, UINT8 src_base, UINT8 *vram);
	void _0200(UINT16 mcu_cmd, UINT8 *vram);
	void _0600(UINT8 is2p, UINT8 *vram);
	void _0e00(UINT16 mcu_cmd, UINT8 *vram);

	UINT8 *m_data;

};

extern const device_type NB1414M4;
