cmake_minimum_required(VERSION 3.14)
project(OCTCommu)
# 打开允许创建文件夹的开关
set_property(GLOBAL PROPERTY USE_FOLDER ON)

# 加载配置
# 配置文件不存在，则提示
set(OUTPUT_CMAKE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/output.cmake)
if (NOT EXISTS ${OUTPUT_CMAKE})
    message(FATAL_ERROR "文件： ${OUTPUT_CMAKE} 不存在")
endif()
include(${OUTPUT_CMAKE})

# 设置输出路径
set(PROJECT_OUTPUT_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/output/publish)
# 设置项目的输出路径
SET_PROJECT_OUTPUT(${PROJECT_OUTPUT_ROOT})

if(NOT WIN32)
    # message("\ndddddd\n")
    include(CheckStructHasMember)
    include(CheckSymbolExists)

    list(APPEND CMAKE_REQUIRED_DEFINITIONS -D_GNU_SOURCE)
    check_struct_has_member("struct mmsghdr" msg_hdr sys/socket.h HAVE_MMSG_HDR)
    check_symbol_exists(sendmmsg sys/socket.h HAVE_SENDMMSG_API)
    check_symbol_exists(recvmmsg sys/socket.h HAVE_RECVMMSG_API)
endif()

if(HAVE_MMSG_HDR)
    add_definitions(-DHAVE_MMSG_HDR)
endif()
if(HAVE_SENDMMSG_API)
    add_definitions(-DHAVE_SENDMMSG_API)
endif()
if(HAVE_RECVMMSG_API)
    add_definitions(-DHAVE_RECVMMSG_API)
endif()


#加载自定义模块
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
# #设置库文件路径
# set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
# #设置可执行程序路径
# set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
#设置子目录
set(SUB_DIR_LIST "Network" "Poller" "Thread" "Util" "SerialPort")

if(WIN32)
    list(APPEND SUB_DIR_LIST "win32")
    #防止Windows.h包含Winsock.h
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

#安装目录
# if(WIN32)
#     set(INSTALL_PATH_LIB $ENV{HOME}/${PROJECT_NAME}/lib)
#     set(INSTALL_PATH_INCLUDE $ENV{HOME}/${PROJECT_NAME}/include)
# else()
#     set(INSTALL_PATH_LIB lib)
#     set(INSTALL_PATH_INCLUDE include)
# endif()

# foreach(SUB_DIR ${SUB_DIR_LIST})
#     #遍历源文件
#     aux_source_directory(src/${SUB_DIR} SRC_LIST)
#     #安装头文件至系统目录
#     # install(DIRECTORY src/${SUB_DIR} DESTINATION ${INSTALL_PATH_INCLUDE} FILES_MATCHING PATTERN "*.h")
# endforeach()

#非苹果平台移除.mm类型的文件
if(NOT APPLE)
    list(REMOVE_ITEM SRC_LIST "src/Network/Socket_ios.mm")
endif()

if(WIN32)
    set(LINK_LIB_LIST WS2_32 Iphlpapi shlwapi)
else()
    set(LINK_LIB_LIST)
endif()


# set(ENABLE_OPENSSL Off CACHE BOOL "enable openssl")
# set(ENABLE_MYSQL Off CACHE BOOL "enable mysql")


# #查找openssl是否安装
# find_package(OpenSSL QUIET)
# if(OPENSSL_FOUND AND ENABLE_OPENSSL)
#     message(STATUS "找到openssl库:\"${OPENSSL_INCLUDE_DIR}\",ENABLE_OPENSSL宏已打开")
#     include_directories(${OPENSSL_INCLUDE_DIR})
#     add_definitions(-DENABLE_OPENSSL)
#     list(APPEND  LINK_LIB_LIST ${OPENSSL_LIBRARIES})
# endif()

# #查找mysql是否安装
# find_package(MYSQL QUIET)
# if(MYSQL_FOUND AND ENABLE_MYSQL)
#     message(STATUS "找到mysqlclient库:\"${MYSQL_INCLUDE_DIR}\",ENABLE_MYSQL宏已打开")
#     include_directories(${MYSQL_INCLUDE_DIR})
#     include_directories(${MYSQL_INCLUDE_DIR}/mysql)
#     add_definitions(-DENABLE_MYSQL)
#     list(APPEND  LINK_LIB_LIST ${MYSQL_LIBRARIES})
# endif()

#打印库文件
# message(STATUS "将链接依赖库:${LINK_LIB_LIST}")


#使能c++11
set(CMAKE_CXX_STANDARD 14)

if(NOT WIN32)
    add_compile_options(-Wno-deprecated-declarations)
    add_compile_options(-Wno-predefined-identifier-outside-function)
endif()

#编译动态库
# if(NOT IOS AND NOT ANDROID AND NOT WIN32)
    # add_library(${PROJECT_NAME}_shared SHARED ${SRC_LIST})
    # target_include_directories(${PROJECT_NAME}_shared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
    # target_link_libraries(${PROJECT_NAME}_shared ${LINK_LIB_LIST})
    # set_target_properties(${PROJECT_NAME}_shared PROPERTIES OUTPUT_NAME "${PROJECT_NAME}")
    # install(TARGETS ${PROJECT_NAME}_shared  ARCHIVE DESTINATION ${INSTALL_PATH_LIB} LIBRARY DESTINATION ${INSTALL_PATH_LIB})
# else()
    # #编译静态库
    # ---------------------------------------------------------------------------------------------------
    # 2. to get all source files
    # set header files
    # -------------------------------------------------------------------------------------
    set (src_root
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
    file(GLOB_RECURSE lib_src_files
     ${src_root}/Communcation/*.h
     ${src_root}/Communcation/*.cpp

     ${src_root}/Network/*.h
     ${src_root}/Network/*.cpp
     ${src_root}/Network/*.c
     
     ${src_root}/Poller/*.h
     ${src_root}/Poller/*.cpp
     ${src_root}/Poller/*.c

     ${src_root}/Thread/*.h
     ${src_root}/Thread/*.cpp

     ${src_root}/Util/*.h
     ${src_root}/Util/*.cpp

     ${src_root}/win32/*.h
     ${src_root}/win32/*.c

     ${src_root}/SerialPort/*.h
     ${src_root}/SerialPort/*.cpp
     ${src_root}/SerialPort/*.hpp
     )
	set(CMAKE_DEBUG_POSTFIX D) 
    # 64位
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(PROJECT_NAME ${PROJECT_NAME}64)
    endif()

    # 设置CMake生成动态库时导出所有符号
    # set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

    add_library(${PROJECT_NAME} SHARED ${lib_inc_files} ${lib_src_files} )

    # if(MSVC)
    #     target_compile_options( ${PROJECT_NAME} 
    #     PRIVATE /GS
    #     PRIVATE /source-charset:utf-8 /execution-charset:gb2312 
    #     )
    # endif()

    target_include_directories(${PROJECT_NAME}
        PRIVATE ${src_root} 
        PRIVATE ${src_root}/Communcation 
        PRIVATE ${src_root}/Network
        PRIVATE ${src_root}/Poller
        PRIVATE ${src_root}/Util
        PRIVATE ${src_root}/win32
        PRIVATE ${src_root}/Thread
        PRIVATE ${src_root}/SerialPort
        )

     foreach(SUB_DIR ${SUB_DIR_LIST})
        target_include_directories(${PROJECT_NAME}
        PRIVATE  ${src_root}/${SUB_DIR} 
        )
    endforeach()
# endif()


# 串口
if (WIN32)
	target_link_libraries( ${PROJECT_NAME} setupapi)
elseif(APPLE)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(FOUNDATION_LIBRARY Foundation)
    target_link_libraries( ${PROJECT_NAME} ${FOUNDATION_LIBRARY} ${IOKIT_LIBRARY})
elseif (UNIX)
	target_link_libraries( ${PROJECT_NAME} pthread)
endif()


# 避免生成.exp文件


# 引用头文件路径
# target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${PROJECT_NAME}")
#安装静态库至系统目录
# install(TARGETS ${PROJECT_NAME}_static ARCHIVE DESTINATION ${INSTALL_PATH_LIB})


#测试程序
if(NOT IOS)
    # add_subdirectory(tests)
endif()



# 吧工程加到文件夹中
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "CMakeTargets")
# CMAKE 自动创建的工程重新分类
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMakeTargets" )

set(example_is_on OFF)

if (example_is_on)
    set(exam_root 
    ${CMAKE_CURRENT_SOURCE_DIR}/example
    )

    set (exam_src_recv_files
    ${exam_root}/recv.cpp
    )

    add_executable( exam_recv ${exam_src_recv_files} )
    target_include_directories( exam_recv
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/Communcation
    ) 

    target_link_libraries(exam_recv
    PRIVATE ${PROJECT_NAME}
    )

endif(example_is_on)


# 拷贝头文件到输出目录
CP_FILES( TARGET_ROOT ${PIR} SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/Communcation/OCTCommu.h)
# 拷贝项目的cmake文件到输出目录
CP_FILES( TARGET_ROOT ${PC} SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/cmake/OCTCommuConfig.cmake)