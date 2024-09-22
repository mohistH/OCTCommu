# 检查文件是否存在
function( CHECK_FILE_EXISTS WHICH_FILE)
    if (NOT EXISTS ${WHICH_FILE})
        message(FATAL_ERROR "文件:${CMAKE_CURRENT_LIST_FILE}, 函数：${CMAKE_CURRENT_FUNCTION}, 行：${CMAKE_CURRENT_LIST_LINE}， WHICH_FILE(${WHICH_FILE} 不存在)")
    endif()

endfunction()

#
# 设置库的 lib
#
function(SET_TARGET_LIB)
    set(options)
    # TARGET_NAME-项目名称
    # CONFIG_NAME - 配置管理器名字， 比如: DEBUG RELESE
    # LIB_FILE - LIB文件
    set(oneValueArgs TARGET_NAME CONFIG_NAME LIB_FILE)
    set(multiValueArgs)

    # 解析参数列表
    cmake_parse_arguments(STL "${options}" "${oneValueArgs}"	"${multiValueArgs}" ${ARGN} )

    CHECK_FILE_EXISTS(${STL_LIB_FILE})

	# 2. 绑定库文件
    set_target_properties(${STL_TARGET_NAME} PROPERTIES	"IMPORTED_IMPLIB_${STL_CONFIG_NAME}" ${STL_LIB_FILE})
endfunction()

#
# 设置库的 bin文件
#
function(SET_TARGET_BIN)
    set(options)
    # TARGET_NAME-项目名称
    # CONFIG_NAME - 配置管理器名字， 比如: DEBUG RELESE
    # BIN_FILE - 可执行程序
    set(oneValueArgs TARGET_NAME CONFIG_NAME BIN_FILE)
    set(multiValueArgs)

    # 解析参数列表
    cmake_parse_arguments(STB "${options}" "${oneValueArgs}"	"${multiValueArgs}" ${ARGN} )

    CHECK_FILE_EXISTS(${STB_BIN_FILE})

    # 绑定动态库
	set_target_properties(${STB_TARGET_NAME}  PROPERTIES	"IMPORTED_LOCATION_${STB_CONFIG_NAME}" ${STB_BIN_FILE})
endfunction()




# lib文件后缀
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # message(STATUS "编译器是 MSVC")
    set(LIB_SUFFIX .lib)
    set(LIB_SUFFIX .lib)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(LIB_SUFFIX .so)
    set(LIB_SUFFIX .so)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(LIB_SUFFIX .so)
    set(LIB_SUFFIX .so)
else()
    message(FATAL_ERROR "未知编译器")
endif()


# 判断系统是否为64位
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    # message(STATUS "系统是64位")
    set(PLAT_FROM 64)
else()
    message(STATUS "系统不是64位")
    set(PLAT_FROM )
endif()

# 库文件有这些
set(LIB_ROOT            ${CMAKE_CURRENT_LIST_DIR}/../lib)
set(LIB_DEBUG_FILE      ${LIB_ROOT}/Debug/OCTCommu${PLAT_FROM}D${LIB_SUFFIX})
set(LIB_RELEASE_FILE    ${LIB_ROOT}/Release/OCTCommu${PLAT_FROM}${LIB_SUFFIX})

# 可执行有这些
set(BIN_ROOT            ${CMAKE_CURRENT_LIST_DIR}/../bin)
set(BIN_DEBUG_FILE      ${BIN_ROOT}/Debug/OCTCommu${PLAT_FROM}D.dll)
set(BIN_RELEASE_FILE    ${BIN_ROOT}/Release/OCTCommu${PLAT_FROM}.dll)



if (NOT TARGET OCT::COMMU)
	# 建库
	add_library(OCT::COMMU SHARED IMPORTED)
endif()

# 配置管理
set(CONFIG_NAME_LIST
DEBUG
RELEASE
)

if (TARGET OCT::COMMU)
	# 指定头文件去哪儿找
	set_property(TARGET OCT::COMMU PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_LIST_DIR}/../include)

	# 如果是windows，设置管理员权限。注意使用if需要配合endif使用
	if (MSVC)
        foreach(EACH_CONFIG ${CONFIG_NAME_LIST})
            # 设置库文件
            SET_TARGET_LIB(TARGET_NAME OCT::COMMU LIB_FILE ${LIB_${EACH_CONFIG}_FILE} CONFIG_NAME ${EACH_CONFIG})
            # 设置动态库文件
            SET_TARGET_BIN(TARGET_NAME OCT::COMMU BIN_FILE ${BIN_${EACH_CONFIG}_FILE} CONFIG_NAME ${EACH_CONFIG})
        endforeach()
    else() 
        message(FATAL_ERROR "暂未适配动态库")
    endif()

    set_target_properties(OCT::COMMU PROPERTIES
    MAP_IMPORTED_CONFIG_MINSIZEREL Release
    MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
    )
endif()