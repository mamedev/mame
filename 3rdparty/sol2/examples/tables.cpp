#include <sol.hpp>
#include <string>
#include <iostream>

// this example shows how to read data in from a lua table

int main() {
	std::cout << "=== tables example ===" << std::endl;

    sol::state lua;
    // table used as an array
    lua.script("table1 = {\"hello\", \"table\"}");
    // table with a nested table and the key value syntax
    lua.script("table2 = {"
                  "[\"nestedTable\"] = {"
                       "[\"key1\"] = \"value1\","
                       "[\"key2\"]= \"value2\""
                   "},"
                  "[\"name\"]= \"table2\""
                "}");


    /* Shorter Syntax: */
    // using the values stored in table1
    /*std::cout << (std::string)lua["table1"][1] << " "
        << (std::string)lua["table1"][2] << '\n';
	   */
    // some retrieval of values from the nested table
    // the cleaner way of doing things
    // chain off the the get<>() / [] results
    auto t2 = lua.get<sol::table>("table2");
    auto nestedTable = t2.get<sol::table>("nestedTable");
    // Alternatively:
    //sol::table t2 = lua["table2"];
    //sol::table nestedTable = t2["nestedTable"];
    
    std::string x = lua["table2"]["nestedTable"]["key2"];
    std::cout << "nested table: key1 : " << nestedTable.get<std::string>("key1") << ", key2: "
        << x
        << '\n';
    std::cout << "name of t2: " << t2.get<std::string>("name") << '\n';
    std::string t2name = t2["name"];
    std::cout << "name of t2: " << t2name << '\n';

    /* Longer Syntax: */
    // using the values stored in table1
    std::cout << lua.get<sol::table>("table1").get<std::string>(1) << " "
              << lua.get<sol::table>("table1").get<std::string>(2) << '\n';

    // some retrieval of values from the nested table
    // the cleaner way of doing things
    std::cout << "nested table: key1 : " << nestedTable.get<std::string>("key1") << ", key2: "
    // yes you can chain the get<>() results
              << lua.get<sol::table>("table2").get<sol::table>("nestedTable").get<std::string>("key2")
              << '\n';
    std::cout << "name of t2: " << t2.get<std::string>("name") << '\n';

	std::cout << std::endl;
}
