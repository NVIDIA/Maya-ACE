add_subdirectory(ace_grpc_cpp)
add_subdirectory(aceclient)
add_subdirectory(maya_aceclient)

add_custom_target(mace)
add_dependencies(mace
    maya_aceclient_2024
)