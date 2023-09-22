#ifndef IR_TRAIN_DETECTOR_STATUSMANAGER_H
#define IR_TRAIN_DETECTOR_STATUSMANAGER_H

#include <cstdint>

#include <msp430hal/gpio/pin.h>
#include "RAMMirroredFlashConfigurationStorage.h"

class StatusManager
{
public:
    static const std::size_t size = 6;

private:
    struct LEDBind
    {
        std::size_t m_bit;
        volatile std::uint8_t* m_out_register;
    };

    LEDBind m_led_binds[size];
    std::uint8_t m_status;
    bool m_updated;

    RAMMirroredFlashConfigurationStorage<15>* m_configuration_registers;

public:

    StatusManager(RAMMirroredFlashConfigurationStorage<15>* configuration_registers);

    std::uint8_t& getStatusByte();
    const std::uint8_t& getStatusByte() const;
    bool getBit(std::size_t i) const;
    void setBit(std::size_t i, bool bit);

    void setBits(std::uint8_t bits);
    void clearBits(std::uint8_t bits);

    template<msp430hal::gpio::Port port>
    void bindLED(std::size_t i, msp430hal::gpio::Pin pin);
    void bindLED(std::size_t i, std::size_t pin, volatile std::uint8_t* out_register);

    void updateLEDs();
};

#endif //IR_TRAIN_DETECTOR_STATUSMANAGER_H
