idf_component_register(
    SRCS "main.cpp"
         "../test_main.cpp"
         "../test/component/test_mocks.h"
    INCLUDE_DIRS "../include" "../test/component"
    REQUIRES driver esp_common unity
)
