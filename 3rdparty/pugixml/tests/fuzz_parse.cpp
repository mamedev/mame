#include "../src/pugixml.hpp"

#include <stdint.h>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size)
{
    pugi::xml_document doc;

	doc.load_buffer(Data, Size);
	doc.load_buffer(Data, Size, pugi::parse_minimal);
	doc.load_buffer(Data, Size, pugi::parse_full);

	return 0;
}
