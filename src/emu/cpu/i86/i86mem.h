typedef struct _memory_interface memory_interface;
struct _memory_interface
{
	offs_t	fetch_xor;

	UINT8	(*rbyte)(offs_t);
	UINT16	(*rword)(offs_t);
	void	(*wbyte)(offs_t, UINT8);
	void	(*wword)(offs_t, UINT16);

	UINT8	(*rbyte_port)(offs_t);
	UINT16	(*rword_port)(offs_t);
	void	(*wbyte_port)(offs_t, UINT8);
	void	(*wword_port)(offs_t, UINT16);
};
