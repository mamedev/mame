#include "../src/pugixml.hpp"
#include "fuzzer/FuzzedDataProvider.h"

#include <stdint.h>
#include <string.h>
#include <string>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* Data, size_t Size)
{
	FuzzedDataProvider fdp(Data, Size);
	std::string text = fdp.ConsumeRandomLengthString(1024);

#ifndef PUGIXML_NO_EXCEPTIONS
	try
#endif
	{
		pugi::xpath_variable_set vars;
		size_t var_count = fdp.ConsumeIntegralInRange<size_t>(0, 50);
		std::vector<std::string> var_name_storage;
		for (size_t i = 0; i < var_count; ++i)
		{
			var_name_storage.push_back(fdp.ConsumeRandomLengthString(128));

			const int xpath_value_type_count = pugi::xpath_type_boolean + 1;
			pugi::xpath_value_type value_type = static_cast<pugi::xpath_value_type>(fdp.ConsumeIntegralInRange(0, xpath_value_type_count));
			vars.add(var_name_storage.back().c_str(), value_type);
		}
		pugi::xpath_query q(text.c_str(), &vars);

		std::vector<uint8_t> xml_buffer = fdp.ConsumeBytes<uint8_t>(fdp.ConsumeIntegralInRange(0, 1024));
		pugi::xml_document doc;
		doc.load_buffer(xml_buffer.data(), xml_buffer.size(), pugi::parse_full);

		q.evaluate_boolean(doc);
		q.evaluate_number(doc);
		q.evaluate_string(doc);
		q.evaluate_node(doc);
		q.evaluate_node_set(doc);
	}
#ifndef PUGIXML_NO_EXCEPTIONS
	catch (pugi::xpath_exception&)
	{
	}
#endif
	return 0;
}
