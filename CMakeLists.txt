# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)
#set(EXTRA_COMPONENT_DIRS lib/)
set(COMPONENT_REQUIRES log nvs_flash mdns wpa_supplicant lwip esp_http_server)
set(COMPONENT_EMBED_FILES style.css code.js index.html)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(esp-ecial)
