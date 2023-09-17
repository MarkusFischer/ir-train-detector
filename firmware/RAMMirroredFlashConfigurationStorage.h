#ifndef IR_TRAIN_DETECTOR_RAMMIRROREDFLASHCONFIGURATIONSTORAGE_H
#define IR_TRAIN_DETECTOR_RAMMIRROREDFLASHCONFIGURATIONSTORAGE_H

#include <cstdint>
#include <msp430hal/cpu/flash_controller.h>

template<std::size_t size, std::uint16_t segment_start_address=0x1000>
class RAMMirroredFlashConfigurationStorage
{
private:
    //TODO: make use of any type
    typedef std::uint8_t T;
    //+1 Byte in front of data block for validity checks (reads 0 if data is invalid)
    static constexpr std::size_t data_size = sizeof(T) * size;
    static constexpr std::size_t info_segment_count = (data_size + 1) / 64;
    static constexpr std::size_t redundancy_factor = 64 / (data_size + 1);

    std::uint8_t m_write_protection_flags[(size / 8) + 1];
    std::uint8_t m_data_ram[size];

    bool m_synced;
    std::size_t inter_segment_offset = 0;
public:

    bool reloadFromFlash()
    {
        std::uint16_t flash_data_begin = segment_start_address;
        while (*reinterpret_cast<std::uint8_t*>(flash_data_begin) == 0)
        {
            inter_segment_offset++;
            flash_data_begin += data_size + 1;
            //TODO make sure to not read over segment boundaries
        }
        //Valid data found
        //Next byte is start of actual data
        flash_data_begin++;

        //Copy values to RAM
        auto flash_pointer = reinterpret_cast<std::uint8_t*>(flash_data_begin);
        for (std::size_t i = 0; i < size; ++i)
        {
            m_data_ram[i] = *(flash_pointer + i);
        }
        m_synced = true;
        return true;
    }

    bool writeToFlash()
    {
        //(1) mark old data as invalid
        msp430hal::cpu::writeByte(segment_start_address + inter_segment_offset * (data_size + 1), 0);
        //(2) Erase segment if necessary
        if (inter_segment_offset == (redundancy_factor - 1))
        {
            msp430hal::cpu::eraseSegment(segment_start_address);
            inter_segment_offset = 0;
        }
        else
        {
            inter_segment_offset++;
        }

        //(3) Copy data to flash
        if (size % 2)
            msp430hal::cpu::writeByte(segment_start_address + inter_segment_offset * (data_size + 1) + 1, m_data_ram[0]);
        for (std::size_t i = size % 2; i < size; i = i + 2)
        {
            msp430hal::cpu::writeWord(segment_start_address + inter_segment_offset * (data_size + 1) + i + 1, ((m_data_ram[i + 1] << 8) | m_data_ram[i]));
        }
        m_synced = true;
        return true;
    }

    bool synchronized() const
    {
        return m_synced;
    }

    bool valueWriteProtected(std::size_t index) const
    {
        return (m_write_protection_flags[index / 8] & (1 << index % 8)) != 0;
    }

    void clearWriteProtection(std::size_t index)
    {
        m_write_protection_flags[index / 8] &= ~ (1 << index % 8);
    }

    void setWriteProtection(std::size_t index)
    {
        m_write_protection_flags[index / 8] |= (1 << index % 8);
    }

    void set(std::size_t index, std::uint8_t value)
    {
        if (valueWriteProtected(index))
            return;

        m_data_ram[index] = value;
        m_synced = false;
    }

    std::uint8_t get(std::size_t index)
    {
        return m_data_ram[index];
    }
};

#endif //IR_TRAIN_DETECTOR_RAMMIRROREDFLASHCONFIGURATIONSTORAGE_H
