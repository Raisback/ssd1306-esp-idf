# components/ssd1306_i2c/CMakeLists.txt
# This line registers the component with the build system
# The component name must match the directory name
idf_component_register(
    SRCS "ssd1306_i2c.c"
    INCLUDE_DIRS "."
    # This component requires the standard ESP-IDF driver libraries
    REQUIRES "esp_common" "driver" 
)
