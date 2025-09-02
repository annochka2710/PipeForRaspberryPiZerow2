#include "BitArray.hpp"

BitArray::BitArray(int sizeInBits) : sizeBits_(sizeInBits) {
    if (sizeBits_ > 0)
        begin(sizeBits_);
}

void BitArray::begin(const int sizeBits) {
    sizeBits_ = sizeBits;
    int bytes = 0;
    if (sizeBits % 8 == 0)
        bytes = sizeBits / 8;
    else
        bytes = (sizeBits + 7) / 8; // округление
    array_.resize(bytes, 0);      // массив размером bytes с нулями
}

std::vector<uint8_t> BitArray::toBoolVector() {
    if (sizeBits_) {
        std::vector<uint8_t> result(sizeBits_);
        for (int i = 0; i < sizeBits_; i++) {
            result[i] = getBit(i) ? 1 : 0;
        }
        return result;
    }
    else
        return {}; // возвращаем пустой вектор
}

void BitArray::setBit(int index, bool value) {
    if (index >= 0 && index < sizeBits_) {
        int byteIndex = index / 8;   // ищем байт с нашим битом
        int bitPosition = index % 8; // позиция бита в байте
        //(бит под номером bitposition устаналиваем в 1, остальные без изменения с
        // помощью |=) 1 << bitPosition - создание маски с 1 установленным битом
        if (value)
            array_[byteIndex] |=
            (1
                << bitPosition); // установить бит (сдвигаем 1 на bitPosition влево)
        else
            array_[byteIndex] &= ~(1 << bitPosition); // сбросить бит
        // установка бита в 0 в else
        //~ инвертирование всех битов
        //& сбрасывание нужного бита в 0, не трогая остальные
    }
}

bool BitArray::getBit(int index) {
    if (index >= 0 && index < sizeBits_) {
        int byteIndex = index / 8;
        int bitPosition = index % 8;
        return (array_[byteIndex] & (1 << bitPosition)) !=
            0; // returns a get bit index
        // с помощью & получаем нужный бит из байта
    }
    return false;
}

int BitArray::sizeInBits() const { return sizeBits_; }

int BitArray::sizeInBytes() const { return sizeBits_ / 8; }

bool BitArray::isEmpty() const { return array_.empty(); }

std::vector<uint8_t> BitArray::toBytes() const { return array_; }

void BitArray::setFromBytes(const std::vector<uint8_t>& bytes) {
    sizeBits_ = bytes.size() * 8;
    array_ = bytes;
}

// bool BitArray::loadFromFile(const std::string &filename) {
//   std::ofstream fout; //потом для записи
//   fout.open(filename);
//   bool success = false;
//   if (fout.is_open()) {
//     // process data
//     success = true;
//     fout.close();
//   } else{
//     std::cout << "[BitArray] Cannot open a file in BitArray for reading\n";
//     success = true; 
//   }

//   return success;
// }

//bool BitArray::saveToFile(const std::string &filename) const { return true;}