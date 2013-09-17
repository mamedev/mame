#ifndef __VR0VIDEO_H__
#define __VR0VIDEO_H__


/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/

struct vr0video_interface
{
	const char *m_cpu_tag;
};

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


class vr0video_device : public device_t,
										vr0video_interface
{
public:
	vr0video_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~vr0video_device() {}

	int vrender0_ProcessPacket(UINT32 PacketPtr, UINT16 *Dest, UINT8 *TEXTURE);

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

private:
	// internal state
	device_t *m_cpu;

	UINT16 m_InternalPalette[256];
	UINT32 m_LastPalUpdate;

	RenderStateInfo m_RenderState;

	UINT16 Alpha(UINT16 Src, UINT16 Dst);
};

extern const device_type VIDEO_VRENDER0;


#define MCFG_VIDEO_VRENDER0_ADD(_tag, _interface) \
MCFG_DEVICE_ADD(_tag, VIDEO_VRENDER0, 0) \
MCFG_DEVICE_CONFIG(_interface)

#endif /* __VR0VIDEO_H__ */
