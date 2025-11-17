# CMakeLists.txt for Test Component

idf_component_register(
    SRCS "test_mocks.h"
    INCLUDE_DIRS "."
    REQUIRES unity
)