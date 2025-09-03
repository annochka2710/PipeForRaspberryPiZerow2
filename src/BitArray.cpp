#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING

#include "BitArray.hpp"
#include <experimental/filesystem>
#include <fstream>
#include <ios>
#include <iostream>

namespace fs = std::experimental::filesystem;

BitArray::BitArray(int sizeInBits) : sizeBits_(sizeInBits) {
  if (sizeBits_ > 0)
    begin(sizeBits_);
}

BitArray::BitArray() : sizeBits_(0) {}

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
  } else
    return {};
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

void BitArray::setBits(int index, const std::vector<bool> &bits) {
  for (int i = 0; i < bits.size(); i++)
    setBit(index + i, bits[i]);
}

bool BitArray::getBit(int index) const {
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

void BitArray::setFromBytes(const std::vector<uint8_t> &bytes) {
  sizeBits_ = bytes.size() * 8;
  array_ = bytes;
}

std::streamsize BitArray::findFileSize(std::ifstream &file) {
  if (!file.is_open()) {
    std::cout << "[BitArray findFileSize()] Cannot open file for reading\n";
    return 0;
  }
  std::streamsize position = 0;
  if (file) {
    file.seekg(0, std::ios_base::end);
    position = file.tellg();
    file.seekg(0, std::ios_base::beg);
    if (position == -1 || !file.good()) {
      file.clear();
      return 0;
    }
    return position;
  } else {
    std::cout << "No file\n";
    return 0;
  }
}

bool BitArray::loadFromFile(const std::string &filename) {
  std::ifstream in;
  in.open(filename, std::ios_base::in | std::ios_base::binary);

  if (!in.is_open()) {
    std::cout << "[BitArray loadFromFile()] Cannot open file for reading\n";
    std::cout << "[BitArray] Current path: " << fs::current_path() << "\n";
    return false;
  }

  auto fileSize = findFileSize(in);
  std::cout << "[BitArray] File size: " << fileSize << " bytes\n";
  if (fileSize <= 0) {
    std::cout << "[BitArray loadFromFile()] Empty file\n";
    return false;
  }

  // создаем массив для буфера
  std::vector<uint8_t> buffer(fileSize);
  std::cout << "filesize " << fileSize;

  if (in.read(reinterpret_cast<char *>(buffer.data()), fileSize)) {
    array_ = buffer;
    sizeBits_ = buffer.size() * 8;
    std::cout << "[BitArray] Successful load/reading\n";
    return true;
  }
  std::cout << "[BitArray] Error of reading file into buffer\n";
  return false;
}

bool BitArray::saveToFile(const std::string &filename) {
  std::ofstream out;
  out.open(filename, std::ios_base::out | std::ios_base::trunc);

  if (!out.is_open()) {
    std::cout << "[BitArray] Cannot open a file for saving\n\n";
    return false;
  }
  // записываем данные в файл
  if (out.write(reinterpret_cast<char *>(array_.data()), array_.size())) {
    return true;
  }
  std::cout << "[BitArray] Error in saving to file\n";
  return false;
}

bool BitArray::checkEqualFiles(BitArray file1, BitArray file2) {
  if (file1.sizeInBits() != file2.sizeInBits())
    return false;
  for (int i = 0; i < file1.sizeBits_; i++) {
    if (file1.getBit(i) != file2.getBit(i)) {
      return false;
    }
  }
  return true;
}
