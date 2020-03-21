if (WIN32)
	add_executable(RoCorApp WIN32 RoCorComponent.cpp RoCorApplication.cpp)
else()
    add_executable(RoCorApp RoCorComponent.cpp RoCorApplication.cpp)
endif()

target_link_libraries(RoCorApp rocorlib ${ROCOR_LIBS})