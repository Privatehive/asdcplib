include(GNUInstallDirs)
option(USE_ASDCP_JXS "Build with JPEG XS support in AS-DCP?" OFF)
option(WITHOUT_SSL "Build without encryption support?" OFF)
option(WITHOUT_XML "Build without XML support?" OFF)
option(USE_RANDOM_UUID "Enable mixed case UUID generation when environment variable KM_USE_RANDOM_UUID is defined" OFF)
option(BUILD_SHARED_LIBS "Build as shared libs" OFF)

if (NOT WITHOUT_SSL)
	include(FindOpenSSL)
	message(STATUS "Building with encryption support")
	if (NOT OPENSSL_FOUND)
		message(FATAL_ERROR "OpenSSL not found")
	endif()
	set (HAVE_OPENSSL 1)
else()
	message(STATUS "Building without encryption support")
endif ()

if (NOT WITHOUT_XML)
	include(FindXercesC)
	message(STATUS "Building with XML support")
	if (NOT XercesC_FOUND)
		message(FATAL_ERROR "XercesC not found")
	endif()
	set (HAVE_XERCES_C 1)
else()
	message(STATUS "Building without XML support")
endif ()

# This lib. doesn't export from dll with __declspec(dllexport). So this lib. must be built statically on Windows.
if(WIN32)
	if (BUILD_SHARED_LIBS)
		message(FATAL_ERROR "Building shared libs is not supported on WIN32")
	endif()
endif ()

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

add_compile_definitions(PACKAGE_VERSION=\"${VERSION_STRING}\")
add_compile_definitions(ASDCP_PLATFORM=\"${CMAKE_SYSTEM_NAME}\")
if(WIN32)
	add_compile_definitions(KM_WIN32)
	if (MSVC)
		add_compile_options(/D_USE_MATH_DEFINES /D_CONSOLE /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_WARNINGS)
		add_link_options(/SAFESEH:NO)
	endif ()
endif()


# ----------libkumu----------

add_library(libkumu
		KM_fileio.h KM_fileio.cpp
		KM_log.h KM_log.cpp
		KM_util.h KM_util.cpp
		KM_tai.h KM_tai.cpp
		KM_prng.h KM_prng.cpp
		KM_aes.h KM_aes.cpp
		KM_xml.h KM_xml.cpp
		KM_sha1.h KM_sha1.cpp
		KM_error.h
		KM_memio.h
		KM_mutex.h
		KM_platform.h
)

set(libkumu_public_header
		KM_error.h
		KM_fileio.h
		KM_util.h
		KM_memio.h
		KM_tai.h
		KM_platform.h
		KM_log.h
		KM_mutex.h
		KM_prng.h
		KM_sha1.h
		KM_aes.h
		KM_xml.h
)

if (APPLE)
	find_library(CORE_FOUNDATION CoreFoundation)
	find_library(CORE_SERVICES CoreServices)
	target_link_libraries(libkumu PRIVATE ${CORE_FOUNDATION} PRIVATE ${CORE_SERVICES})
endif()

if (WIN32)
	set(libkumu_public_header "dirent_win.h;${libkumu_public_header}")
endif ()
set_target_properties(libkumu PROPERTIES PUBLIC_HEADER "${libkumu_public_header}")
set_target_properties(libkumu PROPERTIES PREFIX "" VERSION ${VERSION_STRING} SOVERSION ${VERSION_MAJOR})
if(WIN32)
	target_compile_definitions(libkumu PUBLIC KM_WIN32)
endif ()

if (HAVE_OPENSSL)
	target_link_libraries(libkumu PRIVATE OpenSSL::Crypto)
	target_compile_definitions(libkumu PRIVATE HAVE_OPENSSL)
	if (WIN32)
		target_link_libraries(libkumu PRIVATE -lws2_32 PRIVATE -lcrypt32)
	endif()
endif()

if (HAVE_XERCES_C)
	target_link_libraries(libkumu PRIVATE XercesC::XercesC)
	target_compile_definitions(libkumu PRIVATE HAVE_XERCES_C)
endif()

if(USE_RANDOM_UUID)
	target_compile_definitions(libkumu PRIVATE CONFIG_RANDOM_UUID)
endif()

target_include_directories(libkumu PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>" "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

# ----------libasdcp----------

add_library(libasdcp
		JP2K.h JP2K.cpp
		MPEG.h MPEG.cpp
		Wav.h Wav.cpp
		KLV.h KLV.cpp
		MXFTypes.h MXFTypes.cpp
		Metadata.h Metadata.cpp
		PCMParserList.h PCMParserList.cpp
		AS_DCP.h AS_DCP.cpp
		AtmosSyncChannel_Generator.h AtmosSyncChannel_Generator.cpp
		AtmosSyncChannel_Mixer.h AtmosSyncChannel_Mixer.cpp
		PCMDataProviders.h PCMDataProviders.cpp
		SyncEncoder.h SyncEncoder.cpp
		CRC16.h CRC16.cpp
		UUIDInformation.h UUIDInformation.cpp
		ST2095_PinkNoise.h ST2095_PinkNoise.cpp
		MDD.h MDD.cpp
		MXF.h MXF.cpp
		MPEG2_Parser.cpp
		JP2K_Codestream_Parser.cpp
		JP2K_Sequence_Parser.cpp
		TimedText_Parser.cpp
		PCM_Parser.cpp
		Dict.cpp
		Index.cpp
		DCData_ByteStream_Parser.cpp
		DCData_Sequence_Parser.cpp
		h__Reader.cpp
		h__Writer.cpp
		AS_DCP_MXF.cpp
		AS_DCP_MPEG2.cpp
		AS_DCP_JP2K.cpp
		AS_DCP_PCM.cpp
		AS_DCP_TimedText.cpp
		AS_DCP_ATMOS.cpp
		AS_DCP_DCData.cpp
		AS_DCP_JXS.h
		AS_DCP_internal.h
		WavFileWriter.h
		JXS.h
		S12MTimecode.h
		SyncCommon.h
)

set(libasdcp_public_header
		AS_DCP.h
		AS_DCP_JXS.h
		MXF.h
		MXFTypes.h
		KLV.h
		MDD.h
		Metadata.h
		PCMParserList.h
		AS_DCP_internal.h
)

set_target_properties(libasdcp PROPERTIES PUBLIC_HEADER "${libasdcp_public_header}")
set_target_properties(libasdcp PROPERTIES PREFIX "" VERSION ${VERSION_STRING} SOVERSION ${VERSION_MAJOR})
target_link_libraries(libasdcp PUBLIC libkumu)

if (HAVE_OPENSSL)
	target_sources(libasdcp PRIVATE AS_DCP_AES.cpp)
	target_link_libraries(libasdcp PRIVATE OpenSSL::Crypto)
	target_compile_definitions(libasdcp PRIVATE HAVE_OPENSSL)
	if (WIN32)
		target_link_libraries(libasdcp PRIVATE -lws2_32 PRIVATE -lcrypt32)
	endif()
endif()

if (USE_ASDCP_JXS)
	target_sources(libasdcp PRIVATE AS_DCP_JXS.cpp JXS_Codestream_Parser.cpp JXS_Sequence_Parser.cpp JXS.cpp)
endif ()

target_include_directories(libasdcp PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>" "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

# ----------libas02----------

add_library(libas02
		AS_02_PHDR.h AS_02_PHDR.cpp
		AS_02_JXS.h AS_02_JXS.cpp
		AS_02_ACES.h AS_02_ACES.cpp
		AS_02_IAB.h AS_02_IAB.cpp
		AS_02_MGASADM.h AS_02_MGASADM.cpp
		ACES.h ACES.cpp
		h__02_Reader.cpp
		h__02_Writer.cpp
		AS_02_ISXD.cpp
		AS_02_JP2K.cpp
		AS_02_PCM.cpp
		ST2052_TextParser.cpp
		AS_02_TimedText.cpp
		ACES_Codestream_Parser.cpp
		ACES_Sequence_Parser.cpp
		AS_02_ProRes.cpp
		AS_02.h
		AS_02_internal.h
)

set(libas02_public_header
		AS_02.h
		AS_02_PHDR.h
		AS_02_IAB.h
		AS_02_JXS.h
		AS_02_ACES.h
		ACES.h
		AS_02_internal.h
		AS_02_MGASADM.h
)

set_target_properties(libas02 PROPERTIES PUBLIC_HEADER "${libas02_public_header}")
set_target_properties(libas02 PROPERTIES PREFIX "" VERSION ${VERSION_STRING} SOVERSION ${VERSION_MAJOR})
target_link_libraries(libas02 PUBLIC libasdcp)
target_include_directories(libas02 PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>" "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

# -------executables--------

add_executable(blackwave "blackwave.cpp")
target_link_libraries(blackwave PRIVATE libasdcp)

add_executable(wavesplit "wavesplit.cpp")
target_link_libraries(wavesplit PRIVATE libasdcp)

add_executable(kmuuidgen "kmuuidgen.cpp")
target_link_libraries(kmuuidgen PRIVATE libkumu)

add_executable(kmrandgen "kmrandgen.cpp")
target_link_libraries(kmrandgen PRIVATE libkumu)

add_executable(kmfilegen "kmfilegen.cpp")
target_link_libraries(kmfilegen PRIVATE libkumu)

add_executable(klvwalk "klvwalk.cpp")
target_link_libraries(klvwalk PRIVATE libasdcp)

add_executable(asdcp-test "asdcp-test.cpp")
target_link_libraries(asdcp-test PRIVATE libasdcp)

add_executable(asdcp-wrap "asdcp-wrap.cpp")
target_link_libraries(asdcp-wrap PRIVATE libasdcp)

add_executable(asdcp-unwrap "asdcp-unwrap.cpp")
target_link_libraries(asdcp-unwrap PRIVATE libasdcp)

add_executable(asdcp-info "asdcp-info.cpp")
target_link_libraries(asdcp-info PRIVATE libasdcp)

add_executable(asdcp-util "asdcp-util.cpp")
target_link_libraries(asdcp-util PRIVATE libasdcp)

add_executable(j2c-test "j2c-test.cpp")
target_link_libraries(j2c-test PRIVATE libasdcp)

add_executable(as-02-wrap "as-02-wrap.cpp")
target_link_libraries(as-02-wrap PRIVATE libas02)

add_executable(as-02-wrap-iab "as-02-wrap-iab.cpp")
target_link_libraries(as-02-wrap-iab PRIVATE libas02)

add_executable(as-02-unwrap "as-02-unwrap.cpp")
target_link_libraries(as-02-unwrap PRIVATE libas02)

add_executable(as-02-info "as-02-info.cpp")
target_link_libraries(as-02-info PRIVATE libas02)

add_executable(pinkwave "pinkwave.cpp")
target_link_libraries(pinkwave PRIVATE libasdcp)

add_executable(klvsplit "klvsplit.cpp")
target_link_libraries(klvsplit PRIVATE libasdcp)

set(install_executables
		blackwave
		wavesplit
		pinkwave
		klvwalk
		asdcp-test
		asdcp-wrap
		asdcp-unwrap
		asdcp-info
		asdcp-util
		j2c-test
		as-02-wrap
		as-02-wrap-iab
		as-02-unwrap
		as-02-info
		kmfilegen
		kmuuidgen
		kmrandgen
)

if (USE_ASDCP_JXS)
	add_executable(as-02-wrap-jxs "as-02-wrap-jxs.cpp")
	target_link_libraries(as-02-wrap-jxs PRIVATE libas02)
	list(APPEND install_executables as-02-wrap-jxs)
endif ()

install(TARGETS libkumu libasdcp libas02 ${install_executables}
		EXPORT asdcplib-Targets
 		RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
 		LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
 		ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
 		PRIVATE_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
 		PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
 )
#install(FILES ${as02_deploy_header} ${asdcp_deploy_header} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(EXPORT asdcplib-Targets DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/asdcplib")

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/asdcplibConfig.cmake.in" "@PACKAGE_INIT@")
file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/asdcplibConfig.cmake.in" "\ninclude(\"\${CMAKE_CURRENT_LIST_DIR}/asdcplib-Targets.cmake\")")
file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/asdcplibConfig.cmake.in" "\nset(asdcplib_INCLUDE_DIRS \"\${CMAKE_CURRENT_LIST_DIR}/../../../${CMAKE_INSTALL_INCLUDEDIR}\")")
file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/asdcplibConfig.cmake.in" "\ncheck_required_components(asdcplib)")
file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/asdcplibConfig.cmake.in" "\ninclude(CMakeFindDependencyMacro)")
if (HAVE_OPENSSL)
	file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/asdcplibConfig.cmake.in" "\ninclude(FindOpenSSL)")
endif()
if (HAVE_XERCES_C)
	file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/asdcplibConfig.cmake.in" "\ninclude(FindXercesC)")
endif()

include(CMakePackageConfigHelpers)
configure_package_config_file("${CMAKE_CURRENT_BINARY_DIR}/asdcplibConfig.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/asdcplibConfig.cmake" INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/asdcplib")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/asdcplibConfig.cmake" DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/asdcplib")
