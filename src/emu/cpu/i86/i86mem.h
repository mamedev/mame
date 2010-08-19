typedef struct _memory_interface memory_interface;
struct _memory_interface
{
	offs_t	fetch_xor;

	UINT8	(*rbyte)(address_space *, offs_t);
	UINT16	(*rword)(address_space *, offs_t);
	void	(*wbyte)(address_space *, offs_t, UINT8);
	void	(*wword)(address_space *, offs_t, UINT16);
};
