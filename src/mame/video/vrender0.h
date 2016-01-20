// license:BSD-3-Clause
// copyright-holders:ElSemi
#ifndef __VR0VIDEO_H__
#define __VR0VIDEO_H__


/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/

struct RenderStateInfo
{
	UINT32 Tx;
	UINT32 Ty;
	UINT32 Txdx;
	UINT32 Tydx;
	UINT32 Txdy;
	UINT32 Tydy;
	UINT32 SrcAlphaColor;
	UINT32 SrcBlend;
	UINT32 DstAlphaColor;
	UINT32 DstBlend;
	UINT32 ShadeColor;
	UINT32 TransColor;
	UINT32 TileOffset;
	UINT32 FontOffset;
	UINT32 PalOffset;
	UINT32 PaletteBank;
	UINT32 TextureMode;
	UINT32 PixelFormat;
	UINT32 Width;
	UINT32 Height;
};

class vr0video_device : public device_t
{
public:
	vr0video_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	~vr0video_device() {}

	int vrender0_ProcessPacket(UINT32 PacketPtr, UINT16 *Dest, UINT8 *TEXTURE);

	static void set_cpu_tag(device_t &device, std::string tag) { downcast<vr0video_device &>(device).m_cpu.set_tag(tag); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state
	required_device<cpu_device> m_cpu;

	UINT16 m_InternalPalette[256];
	UINT32 m_LastPalUpdate;

	RenderStateInfo m_RenderState;
};

extern const device_type VIDEO_VRENDER0;


#define MCFG_VIDEO_VRENDER0_CPU(_tag) \
	vr0video_device::set_cpu_tag(*device, "^" _tag);

#endif /* __VR0VIDEO_H__ */
