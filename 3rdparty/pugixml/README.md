pugixml [![Actions Status](https://github.com/zeux/pugixml/workflows/build/badge.svg)](https://github.com/zeux/pugixml/actions) [![Build status](https://ci.appveyor.com/api/projects/status/9hdks1doqvq8pwe7/branch/master?svg=true)](https://ci.appveyor.com/project/zeux/pugixml) [![codecov.io](https://codecov.io/github/zeux/pugixml/coverage.svg?branch=master)](https://codecov.io/github/zeux/pugixml?branch=master) ![MIT](https://img.shields.io/badge/license-MIT-blue.svg)
=======

pugixml is a C++ XML processing library, which consists of a DOM-like interface with rich traversal/modification
capabilities, an extremely fast XML parser which constructs the DOM tree from an XML file/buffer, and an XPath 1.0
implementation for complex data-driven tree queries. Full Unicode support is also available, with Unicode interface
variants and conversions between different Unicode encodings (which happen automatically during parsing/saving).

pugixml is used by a lot of projects, both open-source and proprietary, for performance and easy-to-use interface.

## Documentation

Documentation for the current release of pugixml is available on-line as two separate documents:

* [Quick-start guide](https://pugixml.org/docs/quickstart.html), that aims to provide enough information to start using the library;
* [Complete reference manual](https://pugixml.org/docs/manual.html), that describes all features of the library in detail.

You’re advised to start with the quick-start guide; however, many important library features are either not described in it at all or only mentioned briefly; if you require more information you should read the complete manual.

## Example

Here's an example of how code using pugixml looks; it opens an XML file, goes over all Tool nodes and prints tools that have a Timeout attribute greater than 0:

```c++
#include "pugixml.hpp"
#include <iostream>

int main()
{
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file("xgconsole.xml");
    if (!result)
        return -1;
        
    for (pugi::xml_node tool: doc.child("Profile").child("Tools").children("Tool"))
    {
        int timeout = tool.attribute("Timeout").as_int();
        
        if (timeout > 0)
            std::cout << "Tool " << tool.attribute("Filename").value() << " has timeout " << timeout << "\n";
    }
}
```

And the same example using XPath:

```c++
#include "pugixml.hpp"
#include <iostream>

int main()
{
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file("xgconsole.xml");
    if (!result)
        return -1;
        
    pugi::xpath_node_set tools_with_timeout = doc.select_nodes("/Profile/Tools/Tool[@Timeout > 0]");
    
    for (pugi::xpath_node node: tools_with_timeout)
    {
        pugi::xml_node tool = node.node();
        std::cout << "Tool " << tool.attribute("Filename").value() <<
            " has timeout " << tool.attribute("Timeout").as_int() << "\n";
    }
}
```


## License

This library is available to anybody free of charge, under the terms of MIT License (see LICENSE.md).

This work is based on the pugxml parser, which carried the following Public Domain dedication:

    // Pug XML Parser - Version 1.0002
    // --------------------------------------------------------
    // Copyright (C) 2003, by Kristen Wegner (kristen@tima.net)
    // Released into the Public Domain. Use at your own risk.
    // See pugxml.xml for further information, history, etc.
    // Contributions by Neville Franks (readonly@getsoft.com).
