cmake_minimum_required(VERSION 3.6)
# Project name
project(Project C ASM)
# Processor defenition **PROCESSOR / BOARD DEPENDANT**
add_definitions(-DSTM32F746xx)
add_definitions(-DARM_MATH_CM7)
add_definitions(-DLWIP_TIMEVAL_PRIVATE=0)

file(GLOB_RECURSE USER_SOURCES "src/*.c")
file(GLOB_RECURSE CUBEMX_SOURCES "cubemx/*.c")
file(GLOB_RECURSE BSP_SOURCES "bsp/*.c")

# Compiler options **PROCESSOR / BOARD DEPENDANT**
SET(COMMON_FLAGS "-mcpu=cortex-m7 -mthumb -mthumb-interwork -mfloat-abi=hard -mfpu=fpv5-sp-d16 -ffunction-sections -fdata-sections -g -fno-common -fmessage-length=0 -Wall")

SET(CMAKE_CXX_FLAGS "${COMMON_FLAGS} -std=c++11")
SET(CMAKE_C_FLAGS "${COMMON_FLAGS} -std=gnu11")

SET(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/cubemx/STM32F746NGHx_FLASH.ld")
SET(CMAKE_EXE_LINKER_FLAGS "-Wl,-gc-sections -T ${LINKER_SCRIPT}")

add_library(LIB
        cubemx/Drivers/CMSIS/Device/ST/STM32F7xx/Source/Templates/system_stm32f7xx.c
        cubemx/Drivers/CMSIS/Device/ST/STM32F7xx/Source/Templates/gcc/startup_stm32f746xx.s)

add_library(BSP ${BSP_SOURCES})

include_directories(src)
include_directories(cubemx/Inc)
include_directories(cubemx/Drivers/CMSIS/Device/ST/STM32F7xx/Include)
include_directories(cubemx/Drivers/CMSIS/Include)
include_directories(cubemx/Drivers/STM32F7xx_HAL_Driver/Inc)
include_directories(cubemx/Middlewares/ST/STM32_USB_Host_Library/Class/CDC/Inc)
include_directories(cubemx/Middlewares/ST/STM32_USB_Host_Library/Core/Inc)
include_directories(cubemx/Middlewares/Third_Party/FatFs/src)
include_directories(cubemx/Middlewares/Third_Party/FatFs/src/drivers)
include_directories(cubemx/Middlewares/Third_Party/LwIP/src/include)
include_directories(cubemx/Middlewares/Third_Party/LwIP/system)
include_directories(cubemx/Middlewares/Third_Party/FreeRTOS/Source/include)
include_directories(cubemx/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS)
include_directories(cubemx/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1)
include_directories(bsp)

add_executable(${PROJECT_NAME}.elf ${USER_SOURCES} ${CUBEMX_SOURCES} ${LINKER_SCRIPT})

target_link_libraries(${PROJECT_NAME}.elf LIB BSP)

# Make map, hex and bin
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Map=${PROJECT_SOURCE_DIR}/build/${PROJECT_NAME}.map")
set(HEX_FILE ${PROJECT_SOURCE_DIR}/build/${PROJECT_NAME}.hex)
set(BIN_FILE ${PROJECT_SOURCE_DIR}/build/${PROJECT_NAME}.bin)
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${HEX_FILE}
        COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${PROJECT_NAME}.elf> ${BIN_FILE}
        COMMENT "Building ${HEX_FILE} \nBuilding ${BIN_FILE}")
