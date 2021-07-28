# Copyright 2010-2019 Branimir Karadzic. All rights reserved.
# License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause

set(GLSL_OPTIMIZER ${MAME_DIR}/3rdparty/bgfx/3rdparty/glsl-optimizer)
set(FCPP_DIR     ${MAME_DIR}/3rdparty/bgfx/3rdparty/fcpp)
set(GLSLANG      ${MAME_DIR}/3rdparty/bgfx/3rdparty/glslang)
set(SPIRV_CROSS  ${MAME_DIR}/3rdparty/bgfx/3rdparty/spirv-cross)
set(SPIRV_HEADERS ${MAME_DIR}/3rdparty/bgfx/3rdparty/spirv-headers)
set(SPIRV_TOOLS   ${MAME_DIR}/3rdparty/bgfx/3rdparty/spirv-tools)

add_library(spirv-opt STATIC EXCLUDE_FROM_ALL)

target_include_directories(spirv-opt PRIVATE
	${SPIRV_TOOLS}
	${SPIRV_TOOLS}/include
	${SPIRV_TOOLS}/include/generated
	${SPIRV_TOOLS}/source
	${SPIRV_HEADERS}/include
)

FILE(GLOB spirv-opt-opt-cpp ${SPIRV_TOOLS}/source/opt/*.cpp)
FILE(GLOB spirv-opt-reduce-cpp ${SPIRV_TOOLS}/source/reduce/*.cpp)

target_sources(spirv-opt PRIVATE
	${spirv-opt-opt-cpp}
	${spirv-opt-reduce-cpp}

	# libspirv
	${SPIRV_TOOLS}/source/assembly_grammar.cpp
	${SPIRV_TOOLS}/source/assembly_grammar.h
	${SPIRV_TOOLS}/source/binary.cpp
	${SPIRV_TOOLS}/source/binary.h
	${SPIRV_TOOLS}/source/cfa.h
	${SPIRV_TOOLS}/source/diagnostic.cpp
	${SPIRV_TOOLS}/source/diagnostic.h
	${SPIRV_TOOLS}/source/disassemble.cpp
	${SPIRV_TOOLS}/source/disassemble.h
	${SPIRV_TOOLS}/source/enum_set.h
	${SPIRV_TOOLS}/source/enum_string_mapping.cpp
	${SPIRV_TOOLS}/source/enum_string_mapping.h
	${SPIRV_TOOLS}/source/ext_inst.cpp
	${SPIRV_TOOLS}/source/ext_inst.h
	${SPIRV_TOOLS}/source/extensions.cpp
	${SPIRV_TOOLS}/source/extensions.h
	${SPIRV_TOOLS}/source/instruction.h
	${SPIRV_TOOLS}/source/latest_version_glsl_std_450_header.h
	${SPIRV_TOOLS}/source/latest_version_opencl_std_header.h
	${SPIRV_TOOLS}/source/latest_version_spirv_header.h
	${SPIRV_TOOLS}/source/libspirv.cpp
	${SPIRV_TOOLS}/source/macro.h
	${SPIRV_TOOLS}/source/name_mapper.cpp
	${SPIRV_TOOLS}/source/name_mapper.h
	${SPIRV_TOOLS}/source/opcode.cpp
	${SPIRV_TOOLS}/source/opcode.h
	${SPIRV_TOOLS}/source/operand.cpp
	${SPIRV_TOOLS}/source/operand.h
	${SPIRV_TOOLS}/source/parsed_operand.cpp
	${SPIRV_TOOLS}/source/parsed_operand.h
	${SPIRV_TOOLS}/source/print.cpp
	${SPIRV_TOOLS}/source/print.h
	${SPIRV_TOOLS}/source/software_version.cpp
	${SPIRV_TOOLS}/source/spirv_constant.h
	${SPIRV_TOOLS}/source/spirv_definition.h
	${SPIRV_TOOLS}/source/spirv_endian.cpp
	${SPIRV_TOOLS}/source/spirv_endian.h
	${SPIRV_TOOLS}/source/spirv_optimizer_options.cpp
	${SPIRV_TOOLS}/source/spirv_reducer_options.cpp
	${SPIRV_TOOLS}/source/spirv_target_env.cpp
	${SPIRV_TOOLS}/source/spirv_target_env.h
	${SPIRV_TOOLS}/source/spirv_validator_options.cpp
	${SPIRV_TOOLS}/source/spirv_validator_options.h
	${SPIRV_TOOLS}/source/table.cpp
	${SPIRV_TOOLS}/source/table.h
	${SPIRV_TOOLS}/source/text.cpp
	${SPIRV_TOOLS}/source/text.h
	${SPIRV_TOOLS}/source/text_handler.cpp
	${SPIRV_TOOLS}/source/text_handler.h
	${SPIRV_TOOLS}/source/util/bit_vector.cpp
	${SPIRV_TOOLS}/source/util/bit_vector.h
	${SPIRV_TOOLS}/source/util/bitutils.h
	${SPIRV_TOOLS}/source/util/hex_float.h
	${SPIRV_TOOLS}/source/util/parse_number.cpp
	${SPIRV_TOOLS}/source/util/parse_number.h
	${SPIRV_TOOLS}/source/util/string_utils.cpp
	${SPIRV_TOOLS}/source/util/string_utils.h
	${SPIRV_TOOLS}/source/util/timer.h
	${SPIRV_TOOLS}/source/val/basic_block.cpp
	${SPIRV_TOOLS}/source/val/construct.cpp
	${SPIRV_TOOLS}/source/val/decoration.h
	${SPIRV_TOOLS}/source/val/function.cpp
	${SPIRV_TOOLS}/source/val/instruction.cpp
	${SPIRV_TOOLS}/source/val/validate.cpp
	${SPIRV_TOOLS}/source/val/validate.h
	${SPIRV_TOOLS}/source/val/validate_adjacency.cpp
	${SPIRV_TOOLS}/source/val/validate_annotation.cpp
	${SPIRV_TOOLS}/source/val/validate_arithmetics.cpp
	${SPIRV_TOOLS}/source/val/validate_atomics.cpp
	${SPIRV_TOOLS}/source/val/validate_barriers.cpp
	${SPIRV_TOOLS}/source/val/validate_bitwise.cpp
	${SPIRV_TOOLS}/source/val/validate_builtins.cpp
	${SPIRV_TOOLS}/source/val/validate_capability.cpp
	${SPIRV_TOOLS}/source/val/validate_cfg.cpp
	${SPIRV_TOOLS}/source/val/validate_composites.cpp
	${SPIRV_TOOLS}/source/val/validate_constants.cpp
	${SPIRV_TOOLS}/source/val/validate_conversion.cpp
	${SPIRV_TOOLS}/source/val/validate_debug.cpp
	${SPIRV_TOOLS}/source/val/validate_decorations.cpp
	${SPIRV_TOOLS}/source/val/validate_derivatives.cpp
	${SPIRV_TOOLS}/source/val/validate_execution_limitations.cpp
	${SPIRV_TOOLS}/source/val/validate_extensions.cpp
	${SPIRV_TOOLS}/source/val/validate_function.cpp
	${SPIRV_TOOLS}/source/val/validate_id.cpp
	${SPIRV_TOOLS}/source/val/validate_image.cpp
	${SPIRV_TOOLS}/source/val/validate_instruction.cpp
	${SPIRV_TOOLS}/source/val/validate_interfaces.cpp
	${SPIRV_TOOLS}/source/val/validate_layout.cpp
	${SPIRV_TOOLS}/source/val/validate_literals.cpp
	${SPIRV_TOOLS}/source/val/validate_logicals.cpp
	${SPIRV_TOOLS}/source/val/validate_memory.cpp
	${SPIRV_TOOLS}/source/val/validate_memory_semantics.cpp
	${SPIRV_TOOLS}/source/val/validate_misc.cpp
	${SPIRV_TOOLS}/source/val/validate_mode_setting.cpp
	${SPIRV_TOOLS}/source/val/validate_non_uniform.cpp
	${SPIRV_TOOLS}/source/val/validate_primitives.cpp
	${SPIRV_TOOLS}/source/val/validate_scopes.cpp
	${SPIRV_TOOLS}/source/val/validate_small_type_uses.cpp
	${SPIRV_TOOLS}/source/val/validate_type.cpp
	${SPIRV_TOOLS}/source/val/validation_state.cpp
)

if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
	target_compile_options(spirv-opt PRIVATE
		-Wno-switch
		-Wno-range-loop-construct
	)
endif()
add_project_to_group(shaderc spirv-opt)

add_library(spirv-cross STATIC EXCLUDE_FROM_ALL)
target_compile_definitions(spirv-cross PRIVATE SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS)
target_include_directories(spirv-cross PRIVATE ${SPIRV_CROSS}/include)

target_sources(spirv-cross PRIVATE
	${SPIRV_CROSS}/spirv.hpp
	${SPIRV_CROSS}/spirv_cfg.cpp
	${SPIRV_CROSS}/spirv_cfg.hpp
	${SPIRV_CROSS}/spirv_common.hpp
	${SPIRV_CROSS}/spirv_cpp.cpp
	${SPIRV_CROSS}/spirv_cpp.hpp
	${SPIRV_CROSS}/spirv_cross.cpp
	${SPIRV_CROSS}/spirv_cross.hpp
	${SPIRV_CROSS}/spirv_cross_parsed_ir.cpp
	${SPIRV_CROSS}/spirv_cross_parsed_ir.hpp
	${SPIRV_CROSS}/spirv_cross_util.cpp
	${SPIRV_CROSS}/spirv_cross_util.hpp
	${SPIRV_CROSS}/spirv_glsl.cpp
	${SPIRV_CROSS}/spirv_glsl.hpp
	${SPIRV_CROSS}/spirv_hlsl.cpp
	${SPIRV_CROSS}/spirv_hlsl.hpp
	${SPIRV_CROSS}/spirv_msl.cpp
	${SPIRV_CROSS}/spirv_msl.hpp
	${SPIRV_CROSS}/spirv_parser.cpp
	${SPIRV_CROSS}/spirv_parser.hpp
	${SPIRV_CROSS}/spirv_reflect.cpp
	${SPIRV_CROSS}/spirv_reflect.hpp
)

if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU"))
	target_compile_options(spirv-cross PRIVATE
		-Wno-overloaded-virtual
	)
endif()

add_project_to_group(shaderc spirv-cross)

add_library(glslang STATIC EXCLUDE_FROM_ALL)

target_compile_definitions(glslang PRIVATE
	ENABLE_OPT=1 # spirv-tools
	ENABLE_HLSL=1
)

target_include_directories(glslang PRIVATE
	${GLSLANG}
	${SPIRV_TOOLS}/include
	${SPIRV_TOOLS}/source
)

FILE(GLOB_RECURSE gslang-glslang-cpp ${GLSLANG}/glslang/*.cpp)
FILE(GLOB gslang-hlsl-cpp ${GLSLANG}/hlsl/*.cpp)
FILE(GLOB gslang-SPIRV-cpp ${GLSLANG}/SPIRV/*.cpp)
FILE(GLOB gslang-OGLCompilersDLL-cpp ${GLSLANG}/OGLCompilersDLL/*.cpp)

list(FILTER gslang-glslang-cpp EXCLUDE REGEX "${GLSLANG}/glslang/OSDependent/.*$")

target_sources(glslang PRIVATE
	${gslang-glslang-cpp}
	${gslang-hlsl-cpp}
	${gslang-SPIRV-cpp}
	${gslang-OGLCompilersDLL-cpp}
)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	target_sources(glslang PRIVATE ${GLSLANG}/glslang/OSDependent/Windows/ossource.cpp)
else()
	target_sources(glslang PRIVATE ${GLSLANG}/glslang/OSDependent/Unix/ossource.cpp)
endif()

if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
	target_compile_options(glslang PRIVATE
		-Wno-ignored-qualifiers
		-Wno-implicit-fallthrough
		-Wno-missing-field-initializers
		-Wno-reorder
		-Wno-return-type
		-Wno-shadow
		-Wno-sign-compare
		-Wno-switch
		-Wno-undef
		-Wno-unknown-pragmas
		-Wno-unused-function
		-Wno-unused-parameter
		-Wno-unused-variable
		-fno-strict-aliasing # glslang has bugs if strict aliasing is used.
		-Wno-overloaded-virtual

		-Wno-misleading-indentation
		-Wno-pessimizing-move
		-Wno-maybe-uninitialized
	)
endif()

add_project_to_group(shaderc glslang)

add_library(glsl-optimizer STATIC EXCLUDE_FROM_ALL)

target_include_directories(glsl-optimizer PRIVATE
	${GLSL_OPTIMIZER}/src
	${GLSL_OPTIMIZER}/include
	${GLSL_OPTIMIZER}/src/mesa
	${GLSL_OPTIMIZER}/src/mapi
	${GLSL_OPTIMIZER}/src/glsl
)

target_sources(glsl-optimizer PRIVATE
	${GLSL_OPTIMIZER}/src/glsl/glcpp/glcpp.h
	${GLSL_OPTIMIZER}/src/glsl/glcpp/glcpp-lex.c
	${GLSL_OPTIMIZER}/src/glsl/glcpp/glcpp-parse.c
	${GLSL_OPTIMIZER}/src/glsl/glcpp/glcpp-parse.h
	${GLSL_OPTIMIZER}/src/glsl/glcpp/pp.c

	${GLSL_OPTIMIZER}/src/glsl/ast.h
	${GLSL_OPTIMIZER}/src/glsl/ast_array_index.cpp
	${GLSL_OPTIMIZER}/src/glsl/ast_expr.cpp
	${GLSL_OPTIMIZER}/src/glsl/ast_function.cpp
	${GLSL_OPTIMIZER}/src/glsl/ast_to_hir.cpp
	${GLSL_OPTIMIZER}/src/glsl/ast_type.cpp
	${GLSL_OPTIMIZER}/src/glsl/builtin_functions.cpp
	${GLSL_OPTIMIZER}/src/glsl/builtin_type_macros.h
	${GLSL_OPTIMIZER}/src/glsl/builtin_types.cpp
	${GLSL_OPTIMIZER}/src/glsl/builtin_variables.cpp
	${GLSL_OPTIMIZER}/src/glsl/glsl_lexer.cpp
	${GLSL_OPTIMIZER}/src/glsl/glsl_optimizer.cpp
	${GLSL_OPTIMIZER}/src/glsl/glsl_optimizer.h
	${GLSL_OPTIMIZER}/src/glsl/glsl_parser.cpp
	${GLSL_OPTIMIZER}/src/glsl/glsl_parser.h
	${GLSL_OPTIMIZER}/src/glsl/glsl_parser_extras.cpp
	${GLSL_OPTIMIZER}/src/glsl/glsl_parser_extras.h
	${GLSL_OPTIMIZER}/src/glsl/glsl_symbol_table.cpp
	${GLSL_OPTIMIZER}/src/glsl/glsl_symbol_table.h
	${GLSL_OPTIMIZER}/src/glsl/glsl_types.cpp
	${GLSL_OPTIMIZER}/src/glsl/glsl_types.h
	${GLSL_OPTIMIZER}/src/glsl/hir_field_selection.cpp
	${GLSL_OPTIMIZER}/src/glsl/ir.cpp
	${GLSL_OPTIMIZER}/src/glsl/ir.h
	${GLSL_OPTIMIZER}/src/glsl/ir_basic_block.cpp
	${GLSL_OPTIMIZER}/src/glsl/ir_basic_block.h
	${GLSL_OPTIMIZER}/src/glsl/ir_builder.cpp
	${GLSL_OPTIMIZER}/src/glsl/ir_builder.h
	${GLSL_OPTIMIZER}/src/glsl/ir_clone.cpp
	${GLSL_OPTIMIZER}/src/glsl/ir_constant_expression.cpp
	${GLSL_OPTIMIZER}/src/glsl/ir_equals.cpp
	${GLSL_OPTIMIZER}/src/glsl/ir_expression_flattening.cpp
	${GLSL_OPTIMIZER}/src/glsl/ir_expression_flattening.h
	${GLSL_OPTIMIZER}/src/glsl/ir_function.cpp
	${GLSL_OPTIMIZER}/src/glsl/ir_function_can_inline.cpp
	${GLSL_OPTIMIZER}/src/glsl/ir_function_detect_recursion.cpp
	${GLSL_OPTIMIZER}/src/glsl/ir_function_inlining.h
	${GLSL_OPTIMIZER}/src/glsl/ir_hierarchical_visitor.cpp
	${GLSL_OPTIMIZER}/src/glsl/ir_hierarchical_visitor.h
	${GLSL_OPTIMIZER}/src/glsl/ir_hv_accept.cpp
	${GLSL_OPTIMIZER}/src/glsl/ir_import_prototypes.cpp
	${GLSL_OPTIMIZER}/src/glsl/ir_optimization.h
	${GLSL_OPTIMIZER}/src/glsl/ir_print_glsl_visitor.cpp
	${GLSL_OPTIMIZER}/src/glsl/ir_print_glsl_visitor.h
	${GLSL_OPTIMIZER}/src/glsl/ir_print_metal_visitor.cpp
	${GLSL_OPTIMIZER}/src/glsl/ir_print_metal_visitor.h
	${GLSL_OPTIMIZER}/src/glsl/ir_print_visitor.cpp
	${GLSL_OPTIMIZER}/src/glsl/ir_print_visitor.h
	${GLSL_OPTIMIZER}/src/glsl/ir_rvalue_visitor.cpp
	${GLSL_OPTIMIZER}/src/glsl/ir_rvalue_visitor.h
	${GLSL_OPTIMIZER}/src/glsl/ir_stats.cpp
	${GLSL_OPTIMIZER}/src/glsl/ir_stats.h
	${GLSL_OPTIMIZER}/src/glsl/ir_uniform.h
	${GLSL_OPTIMIZER}/src/glsl/ir_unused_structs.cpp
	${GLSL_OPTIMIZER}/src/glsl/ir_unused_structs.h
	${GLSL_OPTIMIZER}/src/glsl/ir_validate.cpp
	${GLSL_OPTIMIZER}/src/glsl/ir_variable_refcount.cpp
	${GLSL_OPTIMIZER}/src/glsl/ir_variable_refcount.h
	${GLSL_OPTIMIZER}/src/glsl/ir_visitor.h
	${GLSL_OPTIMIZER}/src/glsl/link_atomics.cpp
	${GLSL_OPTIMIZER}/src/glsl/link_functions.cpp
	${GLSL_OPTIMIZER}/src/glsl/link_interface_blocks.cpp
	${GLSL_OPTIMIZER}/src/glsl/link_uniform_block_active_visitor.cpp
	${GLSL_OPTIMIZER}/src/glsl/link_uniform_block_active_visitor.h
	${GLSL_OPTIMIZER}/src/glsl/link_uniform_blocks.cpp
	${GLSL_OPTIMIZER}/src/glsl/link_uniform_initializers.cpp
	${GLSL_OPTIMIZER}/src/glsl/link_uniforms.cpp
	${GLSL_OPTIMIZER}/src/glsl/link_varyings.cpp
	${GLSL_OPTIMIZER}/src/glsl/link_varyings.h
	${GLSL_OPTIMIZER}/src/glsl/linker.cpp
	${GLSL_OPTIMIZER}/src/glsl/linker.h
	${GLSL_OPTIMIZER}/src/glsl/list.h
	${GLSL_OPTIMIZER}/src/glsl/loop_analysis.cpp
	${GLSL_OPTIMIZER}/src/glsl/loop_analysis.h
	${GLSL_OPTIMIZER}/src/glsl/loop_controls.cpp
	${GLSL_OPTIMIZER}/src/glsl/loop_unroll.cpp
	${GLSL_OPTIMIZER}/src/glsl/lower_clip_distance.cpp
	${GLSL_OPTIMIZER}/src/glsl/lower_discard.cpp
	${GLSL_OPTIMIZER}/src/glsl/lower_discard_flow.cpp
	${GLSL_OPTIMIZER}/src/glsl/lower_if_to_cond_assign.cpp
	${GLSL_OPTIMIZER}/src/glsl/lower_instructions.cpp
	${GLSL_OPTIMIZER}/src/glsl/lower_jumps.cpp
	${GLSL_OPTIMIZER}/src/glsl/lower_mat_op_to_vec.cpp
	${GLSL_OPTIMIZER}/src/glsl/lower_named_interface_blocks.cpp
	${GLSL_OPTIMIZER}/src/glsl/lower_noise.cpp
	${GLSL_OPTIMIZER}/src/glsl/lower_offset_array.cpp
	${GLSL_OPTIMIZER}/src/glsl/lower_output_reads.cpp
	${GLSL_OPTIMIZER}/src/glsl/lower_packed_varyings.cpp
	${GLSL_OPTIMIZER}/src/glsl/lower_packing_builtins.cpp
	${GLSL_OPTIMIZER}/src/glsl/lower_ubo_reference.cpp
	${GLSL_OPTIMIZER}/src/glsl/lower_variable_index_to_cond_assign.cpp
	${GLSL_OPTIMIZER}/src/glsl/lower_vec_index_to_cond_assign.cpp
	${GLSL_OPTIMIZER}/src/glsl/lower_vec_index_to_swizzle.cpp
	${GLSL_OPTIMIZER}/src/glsl/lower_vector.cpp
	${GLSL_OPTIMIZER}/src/glsl/lower_vector_insert.cpp
	${GLSL_OPTIMIZER}/src/glsl/lower_vertex_id.cpp
	${GLSL_OPTIMIZER}/src/glsl/opt_algebraic.cpp
	${GLSL_OPTIMIZER}/src/glsl/opt_array_splitting.cpp
	${GLSL_OPTIMIZER}/src/glsl/opt_constant_folding.cpp
	${GLSL_OPTIMIZER}/src/glsl/opt_constant_propagation.cpp
	${GLSL_OPTIMIZER}/src/glsl/opt_constant_variable.cpp
	${GLSL_OPTIMIZER}/src/glsl/opt_copy_propagation.cpp
	${GLSL_OPTIMIZER}/src/glsl/opt_copy_propagation_elements.cpp
	${GLSL_OPTIMIZER}/src/glsl/opt_cse.cpp
	${GLSL_OPTIMIZER}/src/glsl/opt_dead_builtin_variables.cpp
	${GLSL_OPTIMIZER}/src/glsl/opt_dead_builtin_varyings.cpp
	${GLSL_OPTIMIZER}/src/glsl/opt_dead_code.cpp
	${GLSL_OPTIMIZER}/src/glsl/opt_dead_code_local.cpp
	${GLSL_OPTIMIZER}/src/glsl/opt_dead_functions.cpp
	${GLSL_OPTIMIZER}/src/glsl/opt_flatten_nested_if_blocks.cpp
	${GLSL_OPTIMIZER}/src/glsl/opt_flip_matrices.cpp
	${GLSL_OPTIMIZER}/src/glsl/opt_function_inlining.cpp
	${GLSL_OPTIMIZER}/src/glsl/opt_if_simplification.cpp
	${GLSL_OPTIMIZER}/src/glsl/opt_minmax.cpp
	${GLSL_OPTIMIZER}/src/glsl/opt_noop_swizzle.cpp
	${GLSL_OPTIMIZER}/src/glsl/opt_rebalance_tree.cpp
	${GLSL_OPTIMIZER}/src/glsl/opt_redundant_jumps.cpp
	${GLSL_OPTIMIZER}/src/glsl/opt_structure_splitting.cpp
	${GLSL_OPTIMIZER}/src/glsl/opt_swizzle_swizzle.cpp
	${GLSL_OPTIMIZER}/src/glsl/opt_tree_grafting.cpp
	${GLSL_OPTIMIZER}/src/glsl/opt_vectorize.cpp
	${GLSL_OPTIMIZER}/src/glsl/program.h
	${GLSL_OPTIMIZER}/src/glsl/s_expression.cpp
	${GLSL_OPTIMIZER}/src/glsl/s_expression.h
	${GLSL_OPTIMIZER}/src/glsl/standalone_scaffolding.cpp
	${GLSL_OPTIMIZER}/src/glsl/standalone_scaffolding.h
	${GLSL_OPTIMIZER}/src/glsl/strtod.c
	${GLSL_OPTIMIZER}/src/glsl/strtod.h

	${GLSL_OPTIMIZER}/src/mesa/main/compiler.h
	${GLSL_OPTIMIZER}/src/mesa/main/config.h
	${GLSL_OPTIMIZER}/src/mesa/main/context.h
	${GLSL_OPTIMIZER}/src/mesa/main/core.h
	${GLSL_OPTIMIZER}/src/mesa/main/dd.h
	${GLSL_OPTIMIZER}/src/mesa/main/errors.h
	${GLSL_OPTIMIZER}/src/mesa/main/glheader.h
	${GLSL_OPTIMIZER}/src/mesa/main/glminimal.h
	${GLSL_OPTIMIZER}/src/mesa/main/imports.c
	${GLSL_OPTIMIZER}/src/mesa/main/imports.h
	${GLSL_OPTIMIZER}/src/mesa/main/macros.h
	${GLSL_OPTIMIZER}/src/mesa/main/mtypes.h
	${GLSL_OPTIMIZER}/src/mesa/main/simple_list.h

	${GLSL_OPTIMIZER}/src/mesa/program/hash_table.h
	${GLSL_OPTIMIZER}/src/mesa/program/prog_hash_table.c
	${GLSL_OPTIMIZER}/src/mesa/program/prog_instruction.h
	${GLSL_OPTIMIZER}/src/mesa/program/prog_parameter.h
	${GLSL_OPTIMIZER}/src/mesa/program/prog_statevars.h
	${GLSL_OPTIMIZER}/src/mesa/program/symbol_table.c
	${GLSL_OPTIMIZER}/src/mesa/program/symbol_table.h

	${GLSL_OPTIMIZER}/src/util/hash_table.c
	${GLSL_OPTIMIZER}/src/util/hash_table.h
	${GLSL_OPTIMIZER}/src/util/macros.h
	${GLSL_OPTIMIZER}/src/util/ralloc.c
	${GLSL_OPTIMIZER}/src/util/ralloc.h
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	target_compile_options(glsl-optimizer PRIVATE /wd4090) # warning C4090: 'function': different 'const' qualifiers
	target_compile_options(glsl-optimizer PRIVATE /wd4132) # warning C4132: 'deleted_key_value': const object should be initialized
	target_compile_options(glsl-optimizer PRIVATE /wd4189) # warning C4189: 'interface_type': local variable is initialized but not referenced
	target_compile_options(glsl-optimizer PRIVATE /wd4291) # warning C4291: 'no matching operator delete found; memory will not be freed if initialization throws an exception
	target_compile_options(glsl-optimizer PRIVATE /wd4701) # warning C4701: potentially uninitialized local variable 'lower' used
	target_compile_options(glsl-optimizer PRIVATE /wd5033) # warning C5033: 'register' is no longer a supported storage class
endif()

if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
	target_compile_options(glsl-optimizer PRIVATE
		-fno-strict-aliasing # glsl-optimizer has bugs if strict aliasing is used.
		-Wno-implicit-fallthrough
		-Wno-parentheses
		-Wno-sign-compare
		-Wno-unused-function
		-Wno-unused-parameter
		-Wno-misleading-indentation
		$<$<COMPILE_LANGUAGE:C>:-Wno-strict-prototypes>
		$<$<COMPILE_LANGUAGE:CXX>:-Wno-register>
	)
endif()
add_project_to_group(shaderc glsl-optimizer)

add_library(fcpp STATIC EXCLUDE_FROM_ALL)

target_compile_definitions(fcpp PRIVATE
		NINCLUDE=64
		NWORK=65536
		NBUFF=65536
		OLD_PREPROCESSOR=0
)

target_sources(fcpp PRIVATE
	${FCPP_DIR}/cpp1.c
	${FCPP_DIR}/cpp2.c
	${FCPP_DIR}/cpp3.c
	${FCPP_DIR}/cpp4.c
	${FCPP_DIR}/cpp5.c
	${FCPP_DIR}/cpp6.c
	${FCPP_DIR}/cpp6.c
)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")

	target_compile_options(fcpp PRIVATE /wd4055) # warning C4055: 'type cast': from data pointer 'void *' to function pointer 'void (__cdecl *)(char *,void *)'
	target_compile_options(fcpp PRIVATE /wd4244) # warning C4244: '=': conversion from 'const flex_int32_t' to 'YY_CHAR', possible loss of data
	target_compile_options(fcpp PRIVATE /wd4701) # warning C4701: potentially uninitialized local variable 'lower' used
	target_compile_options(fcpp PRIVATE /wd4706) # warning C4706: assignment within conditional expression
endif()

if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
	target_compile_options(fcpp PRIVATE
			-Wno-implicit-fallthrough
			-Wno-incompatible-pointer-types
			-Wno-parentheses-equality
			-Wno-strict-prototypes
	)
	if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		target_compile_options(fcpp PRIVATE
			-Wno-discarded-qualifiers
		)
	endif()
endif()

add_project_to_group(shaderc fcpp)

add_executable(shaderc EXCLUDE_FROM_ALL)

target_include_directories(shaderc PRIVATE
	${MAME_DIR}/3rdparty/bx/include
	${MAME_DIR}/3rdparty/bimg/include
	${MAME_DIR}/3rdparty/bgfx/include

	${MAME_DIR}/3rdparty/bgfx/3rdparty/dxsdk/include

	${FCPP_DIR}

	${GLSLANG}/glslang/Public
	${GLSLANG}/glslang/Include
	${GLSLANG}

	${GLSL_OPTIMIZER}/include
	${GLSL_OPTIMIZER}/src/glsl

	${SPIRV_CROSS}

	${SPIRV_TOOLS}/include
)

target_link_libraries(shaderc PRIVATE
	bx
	fcpp
	glslang
	glsl-optimizer
	spirv-opt
	spirv-cross
)

target_sources(shaderc PRIVATE
		${MAME_DIR}/3rdparty/bgfx/tools/shaderc/shaderc.cpp
		${MAME_DIR}/3rdparty/bgfx/tools/shaderc/shaderc_spirv.cpp
		${MAME_DIR}/3rdparty/bgfx/tools/shaderc/shaderc_metal.cpp
		${MAME_DIR}/3rdparty/bgfx/tools/shaderc/shaderc_hlsl.cpp
		${MAME_DIR}/3rdparty/bgfx/tools/shaderc/shaderc_glsl.cpp
		${MAME_DIR}/3rdparty/bgfx/tools/shaderc/shaderc_pssl.cpp
		${MAME_DIR}/3rdparty/bgfx/src/vertexdecl.cpp
		${MAME_DIR}/3rdparty/bgfx/src/shader_spirv.cpp
)

if (NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	target_link_libraries(shaderc PRIVATE pthread dl)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	target_compile_options(shaderc PRIVATE -Wno-pessimizing-move)
endif()

strip_executable(shaderc)
minimal_symbols(shaderc)
