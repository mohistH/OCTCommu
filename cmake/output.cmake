# ------------------------------------------------------------
# 拷贝文件
macro(CP_FILES )	
	set(options)
	set(oneValueArgs )
	# SRC_FILES - 要拷贝哪些文件
	# TARGET_ROOT - 拷贝到哪里
	set(multiValueArgs SRC_FILES TARGET_ROOT )
	# 解析参数
	cmake_parse_arguments(CF "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

	# 有这么多文件需要拷贝
    foreach(WHICH_FILE ${CF_SRC_FILES})
		# 将文件拷贝到这么多的文件夹
		foreach(WHICH_ROOT ${CF_TARGET_ROOT})
			# 将文件WHICH_FILE拷贝到WHICH_ROOT目录
			execute_process( COMMAND ${CMAKE_COMMAND} -E copy ${WHICH_FILE} ${WHICH_ROOT})
		endforeach()
    endforeach()
endmacro()


# ------------------------------------------------------------
# 创建文件夹
macro(MKDIR)
	set(options)
	set(oneValueArgs )
	# TARGET_ROOT - 有哪些路径需要创建
	set(multiValueArgs TARGET_ROOT )
	# 解析参数
	cmake_parse_arguments(MK "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

	# 需要创建这么多的的文件夹
	foreach(WHICH_ROOT ${MK_TARGET_ROOT})
		# 创建文件夹落地
		execute_process( COMMAND ${CMAKE_COMMAND} -E make_directory ${WHICH_ROOT})
	endforeach()
endmacro()


# ------------------------------------------------------------
# 拷贝文件夹
macro(CP_ROOT)
	set(options)
	set(oneValueArgs )
	# SRC_ROOT - 要拷贝哪些文件夹
	# TARGET_ROOT - 拷贝到哪里
	set(multiValueArgs SRC_ROOT TARGET_ROOT )
	# 解析参数
	cmake_parse_arguments(CR "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

	# 有这么多需要拷贝
    foreach(WHICH_SRC_ROOT ${CR_SRC_ROOT})
		# 将文件拷贝到这么多的文件夹
		foreach(WHICH_TARGET_ROOT ${CR_TARGET_ROOT})
			# 将文件 WHICH_SRC_ROOT 拷贝到 WHICH_TARGET_ROOT 目录
			execute_process( COMMAND ${CMAKE_COMMAND} -E copy_directory ${WHICH_SRC_ROOT} ${WHICH_TARGET_ROOT})
		endforeach()
    endforeach()
endmacro()


#
# 设置项目输出
# 
macro(SET_PROJECT_OUTPUT OUTPUT_PREFIX)
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    # using Clang
        set(compiler clang)
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    # using GCC
        set(compiler gcc)
        
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
        set(compiler intel)
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
        if(MSVC_VERSION EQUAL 1400)
            set(compiler vc80)
        elseif(MSVC_VERSION EQUAL 1500)
            set(compiler vc90)
        elseif(MSVC_VERSION EQUAL 1600)
            set(compiler vc100)
        elseif(MSVC_VERSION EQUAL 1700)
            set(compiler vc110)
        elseif(MSVC_VERSION EQUAL 1800)
            set(compiler vc120)
        elseif(MSVC_VERSION EQUAL 1900)
            set(compiler vc140)
        elseif(MSVC_VERSION EQUAL 1910)
            set(compiler vc141)
        elseif(MSVC_VERSION EQUAL 1920)
            set(compiler vc142)
        elseif(MSVC_VERSION EQUAL 1930)
            set(compiler vc143)
        else()
            set(compiler vcXX)
        endif()
        
    endif()

    # 64位
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(platform x64)
    else()
        set(platform x86)
    endif()

    # 输出前缀
    set(PUBLISH_ROOT ${OUTPUT_PREFIX})
    set(PUBLISH_PREFIX ${PUBLISH_ROOT}/${compiler}/${platform})

    # 设置 可执行程序输出目录
    set(PBD 			${PUBLISH_PREFIX}/bin/Debug)
    set(PBR 		    ${PUBLISH_PREFIX}/bin/Release)

    # 设置库文件输出目录
    set(PLD 			${PUBLISH_PREFIX}/lib/Debug)
    set(PLR 	    	${PUBLISH_PREFIX}/lib/Release)
    # 头文件所在路径
    set(PIR             ${PUBLISH_PREFIX}/include)


    # 指定可执行程序输出目录
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG 	${PBD})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE 	${PBR})

    # 指定 库文件输出目录
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG 	${PLD})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RLEASE 	${PLR})

    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG    ${PLD})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE  ${PLR})

    

    # 输出cmake文件夹
    set(PC ${PUBLISH_PREFIX}/cmake)

    # 有这么多文件夹需要创建
    set(TO_MK_ROOT 
    ${PIR}
    ${PLD}
    ${PLR}
    ${PBD}
    ${PBR}
    ${PC}
    )

    MKDIR(TARGET_ROOT ${TO_MK_ROOT})
endmacro()