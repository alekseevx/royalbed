project(vru-srv)

cmrc_add_resource_library(vru-srv-swagger
    NAMESPACE vru_srv
    api/example.yml
)
target_compile_features(vru-srv-swagger PRIVATE cxx_std_17)

file(GLOB_RECURSE SRC_FILES *.cpp)
add_executable(${PROJECT_NAME} ${SRC_FILES})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_link_libraries(${PROJECT_NAME} vru-srv-swagger royalbed nhope)
