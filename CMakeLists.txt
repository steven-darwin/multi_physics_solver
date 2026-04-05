cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# Enable C++ module dependency scanning
set(CMAKE_CXX_SCAN_FOR_MODULES ON)

project ("MultiPhysicsSolver")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# add source to this project's executable.
set(PROJECT_SOURCES
    multi_physics_solver.cpp
)

add_executable (${PROJECT_NAME} ${PROJECT_SOURCES}) 

target_include_directories(${PROJECT_NAME} 
    PRIVATE 
	    include
        ../../include
)

# add shared library
set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};../../shared_lib/vcpkg_installed/x64-windows")
find_package(GTest CONFIG REQUIRED)
find_package(hdf5 CONFIG REQUIRED)
find_package(LibXml2 REQUIRED)
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        GTest::gtest 
        GTest::gtest_main 
        GTest::gmock 
        GTest::gmock_main
        hdf5::hdf5-shared 
        hdf5::hdf5_hl-shared
        LibXml2::LibXml2
)

target_link_directories(${PROJECT_NAME}
    PRIVATE
         ../../shared_lib/xdmf-3.0.0/lib/
)

target_include_directories(${PROJECT_NAME} 
    PRIVATE 
	    ../../shared_lib/xdmf-3.0.0/include/
	    ../../shared_lib/xdmf-3.0.0/include/core/
	    ../../shared_lib/xdmf-3.0.0/include/core/loki/
	    ../../shared_lib/xdmf-3.0.0/include/core/dsm/
	    ../../shared_lib/xdmf-3.0.0/include/utils/
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Xdmf
        XdmfCore
)

# add local vcpkg library
find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        nlohmann_json::nlohmann_json
)
