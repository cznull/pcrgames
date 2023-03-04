project(kaori's_karate_field LANGUAGES CXX)
#这个项目只能用msvc编译，因为使用到了resource.rc 文件，需要资源编译器，而其他编译器是不具有这种能力的

find_package(OpenGL)
add_executable(kaori game2.cpp resource/Resource.rc)
target_include_directories(kaori PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/dependency/include)
target_compile_definitions(kaori PUBLIC GLEW_STATIC) #静态使用glew

file(GLOB lib "${CMAKE_CURRENT_SOURCE_DIR}/dependency/lib/glew/glew32s.lib")
target_link_libraries(kaori PUBLIC  ${OPENGL_LIBRARIES} ${lib})
