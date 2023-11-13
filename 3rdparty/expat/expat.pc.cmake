prefix=$<TARGET_PROPERTY:expat,pkgconfig_prefix>
exec_prefix=$<TARGET_PROPERTY:expat,pkgconfig_exec_prefix>
libdir=$<TARGET_PROPERTY:expat,pkgconfig_libdir>
includedir=$<TARGET_PROPERTY:expat,pkgconfig_includedir>

Name: $<TARGET_PROPERTY:expat,pkgconfig_$<LOWER_CASE:$<CONFIG>>_name>
Version: $<TARGET_PROPERTY:expat,pkgconfig_version>
Description: expat XML parser
URL: https://libexpat.github.io/
Libs: -L${libdir} -l$<TARGET_PROPERTY:expat,pkgconfig_$<LOWER_CASE:$<CONFIG>>_output_name>
Libs.private: $<TARGET_PROPERTY:expat,pkgconfig_libm>
Cflags: -I${includedir}
