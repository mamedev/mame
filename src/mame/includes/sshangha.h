/*----------- defined in video/sshangha.c -----------*/

extern UINT16 *sshangha_pf1_data;
extern UINT16 *sshangha_pf2_data;
extern UINT16 *sshangha_pf1_rowscroll, *sshangha_pf2_rowscroll;

VIDEO_START( sshangha );
VIDEO_UPDATE( sshangha );

WRITE16_HANDLER( sshangha_pf2_data_w );
WRITE16_HANDLER( sshangha_pf1_data_w );
WRITE16_HANDLER( sshangha_control_0_w );
WRITE16_HANDLER( sshangha_video_w );
