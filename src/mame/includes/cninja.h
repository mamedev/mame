/*----------- defined in video/cninja.c -----------*/

extern UINT16 *cninja_pf1_rowscroll,*cninja_pf2_rowscroll;
extern UINT16 *cninja_pf3_rowscroll,*cninja_pf4_rowscroll;

VIDEO_START( stoneage );

VIDEO_UPDATE( cninja );
VIDEO_UPDATE( cninjabl );
VIDEO_UPDATE( edrandy );
VIDEO_UPDATE( robocop2 );
VIDEO_UPDATE( mutantf );

VIDEO_EOF( cninja );
