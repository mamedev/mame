# CMake Module to locate the PugiXML library
#
# Use via find_package( PugiXML ) in CMakeLists.txt
#
# Sets the following variables:
#
# PugiXML_INCLUDE_DIR
# PugiXML_LIBRARY_DIR
# PugiXML_LIBRARIES
# PugiXML_FOUND
#

find_path (PugiXML_INCLUDE_DIR NAMES pugixml.hpp)
find_library (PugiXML_LIBRARIES NAMES pugixml)
get_filename_component (PugiXML_LIBRARY_PATH ${PugiXML_LIBRARIES} DIRECTORY)

set (PugiXML_INCLUDE_DIR ${PugiXML_INCLUDE_DIR})
set (PugiXML_LIBRARIES ${PugiXML_LIBRARIES})

include (FindPackageHandleStandardArgs)

find_package_handle_standard_args (PugiXML DEFAULT_MSG PugiXML_LIBRARIES
													   PugiXML_INCLUDE_DIR)

mark_as_advanced (PugiXML_LIBRARIES PugiXML_INCLUDE_DIR)
