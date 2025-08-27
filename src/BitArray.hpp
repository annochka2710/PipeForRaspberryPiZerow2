#pragma once

#include <vector>
#include <string>

class BitArray{
    public:
        BitArray(int sizeInBits_);
        ~BitArray() = default;
        
        void begin(const int sizeBits); //инициализация массива размером sizeBits

        void setBit(int index, bool value); //установка значений из другого массива
        bool getBit(int index); //копирование значений в другой массив

        int sizeInBits() const;
        int sizeInBytes() const;

        bool isEmpty() const;

        std::vector<uint8_t> toBytes() const;

        void setFromBytes(const std::vector<uint8_t>& bytes);

        bool loadFromFile(const std::string& filename); //загрузка из файла
        bool saveToFile(const std::string& filename) const; //сохранение в файл

    bool operator[](int index) { return getBit(index); } //оператор доступа
    
    
    private:
        int sizeBits_; //размер массива в битах 
        std::vector<uint8_t> array_; //массив байтов
};