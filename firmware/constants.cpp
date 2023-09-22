#include <cstdint>

const std::uint16_t version __attribute__((location(0x1002), retain)) = 0x2021;
const std::uint16_t default_config_ch0 __attribute__((location(0x1004), retain)) = 0x00ff;
const std::uint16_t default_config_ch1_ch2 __attribute__((location(0x1006), retain)) = 0x0000;
const std::uint16_t default_config_ch3_ch4 __attribute__((location(0x1008), retain)) = 0x0000;
const std::uint16_t default_config_ch5_invert __attribute__((location(0x100a), retain)) = 0x0000;
const std::uint16_t default_config_disable_reserved __attribute__((location(0x100c), retain)) = 0x0000;


extern const std::uint16_t comparator_count_value __attribute__((location(0x1042), retain)) = 100;
