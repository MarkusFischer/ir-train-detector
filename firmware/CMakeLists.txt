cmake_minimum_required(VERSION 3.19)
project(ir-train-detector CXX ASM)

set(CMAKE_CXX_STANDARD 14)

add_subdirectory(msp430hal/)

set(ELF_FILE ${CMAKE_PROJECT_NAME}.elf)
set(LST_FILE ${CMAKE_PROJECT_NAME}.lst)
set(MAP_FILE ${CMAKE_PROJECT_NAME}.map)
set(SYM_FILE ${CMAKE_PROJECT_NAME}.sym)
set(HEX_FILE ${CMAKE_PROJECT_NAME}.hex)

set(DEVICE MSP430G2553)

IF(${CMAKE_CXX_COMPILER_ID} STREQUAL "TI")
    link_directories(/usr/ti/ti-cgt-msp430-21.6.1.lts/lib/)
ENDIF()

    add_executable(${ELF_FILE} main.cpp interrupts.h interrupts.cpp flags.h StatusManager.h UartHandler.h UartHandler.cpp StatusManager.cpp)
    #add_executable(${ELF_FILE} main.cpp)
IF(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    target_include_directories(${ELF_FILE} PRIVATE include msp430hal/include)
    target_compile_options(${ELF_FILE} PRIVATE "-mmcu=${DEVICE}")
    target_link_options(${ELF_FILE} PRIVATE -mmcu=${DEVICE} -Wl,-Map,${MAP_FILE})

    add_custom_command(TARGET ${ELF_FILE} POST_BUILD COMMAND ${CMAKE_OBJDUMP} -h -S ${ELF_FILE} > ${LST_FILE} BYPRODUCTS ${LST_FILE})
    add_custom_command(TARGET ${ELF_FILE} POST_BUILD COMMAND ${CMAKE_NM} -l -a -S -s --size-sort ${ELF_FILE} > ${SYM_FILE} BYPRODUCTS ${SYM_FILE})
    add_custom_command(TARGET ${ELF_FILE} POST_BUILD COMMAND ${CMAKE_OBJCOPY} -Oihex ${ELF_FILE} ${HEX_FILE} BYPRODUCTS ${HEX_FILE})
    add_custom_command(TARGET ${ELF_FILE} POST_BUILD COMMAND ${CMAKE_SIZE} ${ELF_FILE})
ELSEIF(${CMAKE_CXX_COMPILER_ID} STREQUAL "TI")

    target_include_directories(${ELF_FILE} PRIVATE include msp430hal/include)
    target_compile_options(${ELF_FILE} PUBLIC "--define=__${DEVICE}__")
    target_link_options(${ELF_FILE} PRIVATE --rom_model /usr/ti/ti-cgt-msp430-21.6.1.lts/supportfiles_include/lnk_msp430g2553.cmd)

    target_link_libraries(${ELF_FILE} PRIVATE /usr/ti/ti-cgt-msp430-21.6.1.lts/lib/rts430_eabi.lib)

    add_custom_command(TARGET ${ELF_FILE} POST_BUILD COMMAND ${CMAKE_OBJCOPY} -i --romwidth=16 ${ELF_FILE} --order=MS -o ${HEX_FILE} BYPRODUCTS ${HEX_FILE})
    #/usr/ti/ti-cgt-msp430-21.6.1.lts/supportfiles_include/lnk_msp430g2553.cmd
ENDIF()
