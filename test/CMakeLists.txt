# Test component CMakeLists.txt

idf_component_register(
    SRCS "test_radio_comm.cpp"
         "test_display_driver.cpp"
         "test_integration.cpp"
    INCLUDE_DIRS "../include"
    REQUIRES driver esp_common unity
)