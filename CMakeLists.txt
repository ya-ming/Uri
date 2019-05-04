# CMakeLists.txt for Uri
#
# © 2019 by YaMing Wu

cmake_minimum_required(VERSION 3.8)
set(This Uri)

set(Headers
    include/Uri/Uri.hpp
    src/CharacterSet.hpp
    src/PercentEncodedCharacterDecoder.hpp
    src/NormalizeCaseInsensitiveString.hpp
)

set(Sources
    src/Uri.cpp
    src/CharacterSet.cpp
    src/PercentEncodedCharacterDecoder.cpp
    src/NormalizeCaseInsensitiveString.cpp
)

add_library(${This} STATIC ${Sources} ${Headers})
set_target_properties(${This} PROPERTIES
    FOLDER Libraries
)

target_include_directories(${This} PUBLIC include)

add_subdirectory(test)
