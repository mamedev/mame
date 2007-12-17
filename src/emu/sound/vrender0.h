struct VR0Interface
{
	UINT32 RegBase;
};

void VR0_Snd_Set_Areas(UINT32 *Texture,UINT32 *Frame);

READ32_HANDLER(VR0_Snd_Read);
WRITE32_HANDLER(VR0_Snd_Write);
