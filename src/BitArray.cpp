#include "BitArray.hpp"
#include <cstdio>

BitArray::BitArray(int sizeInBits) : sizeBits_(sizeInBits)
{
    if(sizeBits_ > 0)
        begin(sizeBits_);
}

void BitArray::begin(const int sizeBits){
    sizeBits_ = sizeBits;
    int bytes = (sizeBits + 7) / 8; //округление
    array_.resize(bytes, 0); //инициализация нулями
}

void BitArray::setBit(int index, bool value){
    if(index >= 0 && index < sizeBits_){
        int byteIndex = index / 8;
        int bitOffset = index % 8;
    if (value) 
        array_[byteIndex] |= (1 << bitOffset);    // установить бит
    else
        array_[byteIndex] &= ~(1 << bitOffset);   // сбросить бит
    }
}

bool BitArray::getBit(int index){ 
    if(index >= 0 && index < sizeBits_){
        int byteIndex = index / 8;
        int bitOffset = index % 8;
        return (array_[byteIndex] & (1 << bitOffset)) != 0; //returns a get bit index
    }
}

int BitArray::sizeInBits() const { return sizeBits_; }

int BitArray::sizeInBytes() const { return sizeBits_ / 8; }

bool BitArray::isEmpty() const { return array_.empty(); } 

std::vector<uint8_t> BitArray::toBytes() const {
    return array_;
}

void BitArray::setFromBytes(const std::vector<uint8_t>& bytes) {
    sizeBits_ = bytes.size() * 8;
    array_ = bytes;
}

bool BitArray::loadFromFile(const std::string& filename) {

}
    
bool BitArray::saveToFile(const std::string& filename) const {
    
}