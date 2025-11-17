idf_component_register(
    SRCS "main.cpp"
         "display_driver.cpp"
         "radio_comm.cpp"
    INCLUDE_DIRS "../include"
    REQUIRES driver esp_common
)
