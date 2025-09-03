#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING

#include "Pipe.hpp"
#include <ios>
#include <iostream>
#include <mutex>

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#include <stdio.h>

namespace Pipe {
IPipe::IPipe(Direction dir, size_t bufferSize)
    : buffer_(bufferSize), bufferBytes_((bufferSize + 7) / 8, 0),
      bufferSize_(bufferSize), dir_(dir) {
  thread = std::thread(IPipe::run, this);
}

bool IPipe::isReadyRead() const { return readyRead; }

bool IPipe::isReadyWrite() const { return readyWrite; }

// bool IPipe::writeBuff2048(const bool *buff) {
//   if (!readyWrite)
//     return false;
//   if (!buffMutex.try_lock())
//     return false;
//   std::swap(readyWrite, readyRead);
//   memcpy(buf.data(), buff, buf.size());
//   buffMutex.unlock();
//   return true;
// }

// bool IPipe::readBuff2048(bool *buff) {
//   if (!readyRead)
//     return false;
//   if (!buffMutex.try_lock())
//     return false;
//   std::swap(readyWrite, readyRead);
//   memcpy(buff, buf.data(), buf.size());
//   buffMutex.unlock();
//   return true;
// }

// private function
bool IPipe::writeBuff2048(const bool *bits, size_t sizeBuff) {
  std::lock_guard<std::mutex> lock(buffMutex);
  if (!readyWrite || sizeBuff != bufferSize_)
    return false;

  // записываем биты в BitArray
  for (int i = 0; i < sizeBuff; i++)
    buffer_.setBit(i, bits[i]); // set an index and a value from bits

  bufferBytes_ = buffer_.toBytes();
  readyWrite = false;
  readyRead = true;
  return true;
}

// public function
bool IPipe::writeBuff2048(const bool *bits) {
  return writeBuff2048(bits, 2048);
}

bool IPipe::writeBlock(const std::vector<bool> &block) {
  std::lock_guard<std::mutex> lock(buffMutex);
  if (!readyWrite || block.size() > bufferSize_)
    return false;

  // запись блока в buffer_
  for (size_t i = 0; i < block.size(); i++)
    buffer_.setBit(i, block[i]);

  // очистка остатка буфера, если блок меньше
  for (size_t i = block.size(); i < bufferSize_; i++)
    buffer_.setBit(i, false);

  bufferBytes_ = buffer_.toBytes();
  readyWrite = false;
  readyRead = true;
  return true;
}

bool IPipe::readBlock(std::vector<bool> &block, int size) {
  std::lock_guard<std::mutex> lock(buffMutex);
  if (!readyRead || size > bufferSize_)
    return false;
  block.clear();                  // очищение блока
  block.resize(size);    // резерв памяти

  // чтение битов из BitArray
  for (int i = 0; i < size; i++)
    block.push_back(buffer_.getBit(i));

  // сброс флагов
  readyRead = false;
  readyWrite = true;
  return true;
}

void IPipe::setBufferSize(size_t newSize) {
  std::lock_guard<std::mutex> lock(buffMutex); // вместо lock / unlock
  if (newSize)
    bufferSize_ = newSize;
  buffer_.begin(newSize); // инициализация новым размером
  readyRead = false;
  readyRead = true;
}

size_t IPipe::getBufferSize() const { return bufferSize_; }

bool IPipe::readBuff2048(bool *bits, size_t size) {
  std::lock_guard<std::mutex> lock(buffMutex);
  if (!readyRead || size != bufferSize_)
    return false;

  // читаем биты из BitArray
  for (int i = 0; i < size; i++) {
    int byteIndex = i / 8;
    int bitPosition = i % 8;
    bits[i] = (bufferBytes_[byteIndex] & (1 << bitPosition)) != 0;
  }

  readyRead = false;
  readyWrite = true;
  return true;
}

bool IPipe::readBuff2048(bool *bits) { return readBuff2048(bits, 2048); }

void IPipe::run(IPipe *pipe) {
  while (!pipe->stop) {
    if (pipe->dir_ == IN && !pipe->isReadyRead()) // прием данных от
      // пользователя
      continue;
    if (pipe->dir_ == OUT &&
        !pipe->isReadyWrite()) ////отправка данных пользователю
      continue;
    pipe->process();
  }
}

EasyPipe::EasyPipe(Direction dir, const std::string &fileName, int bufferSize)
    : IPipe(dir), fileName_(fileName) {
  fs::path dataPath = fs::current_path();
  dataPath = dataPath.parent_path() / "resources" / fileName_;
        
  std::ios_base::openmode mode = std::ios::binary;

  if (dir_ == OUT)
    mode |= std::ios::out | std::ios::trunc;
  else if (dir_ == IN)
    mode |= std::ios::in;

  file_.open(fileName, mode);

  if (!file_.is_open()) {
    throw std::runtime_error("Failed to open file" + fileName_);
  }

  std::cout << "[EasyPipe] File opened successfully for "
            << (dir_ == OUT ? "writing" : "reading") << " a file " << fileName_
            << std::endl;
}

EasyPipe::~EasyPipe() {
  if (file_.is_open())
    file_.close();
}

std::streamsize EasyPipe::getFileSize(const std::string &filename) {
  std::ifstream file(filename, std::ios::binary | std::ios::ate);
  if (!file.is_open())
    return -1;
  std::streampos pos = file.tellg();
  file.close();
  return (pos == -1) ? -1 : static_cast<std::streamsize>(pos);
}

inline void EasyPipe::process() {
  std::lock_guard<std::mutex> lock(buffMutex);
  if (stop)
    return;

  try {
    if (dir_ == IN) { // запись в файл
      file_.close();
      file_.open(fileName_, std::ios::binary | std::ios::out | std::ios::trunc);

      if (!file_.is_open())
        throw std::runtime_error("[Process] Not open for writing");

      // получаем массив байтов
      auto bytes = buffer_.toBytes();

      if (!bytes.empty()) {
        file_.write((char *)bytes.data(), bytes.size());
        if (!file_)
          throw std::runtime_error("[Process] Failed to open for writing");
        file_.flush();
        // сохранение копии для чтения
        bufferBytes_ = bytes;
      }
      file_.close();

      readyRead = false;
      readyWrite = true;
    }
    // чтение из файла
    else {
      file_.close();
      file_.open(fileName_, std::ios::binary | std::ios::in);

      if (!file_.is_open()) {
        std::cout << "File cannot be opened for reading" << std::endl;
        return;
      }

      // Получаем размер файла
      file_.seekg(0, std::ios::end);
      std::streamsize fileSize = file_.tellg();
      file_.seekg(0, std::ios::beg);

      if (fileSize <= 0) {
        file_.close();
        return;
      }
      // Читаем данные напрямую в bufferBytes_
      bufferBytes_.resize((bufferSize_ + 7) / 8);
      file_.read(reinterpret_cast<char *>(bufferBytes_.data()),
                 std::min(static_cast<std::streamsize>(bufferBytes_.size()),
                          fileSize));

      file_.close();

      // Обновляем BitArray
      buffer_.setFromBytes(bufferBytes_);

      readyRead = true;
      readyWrite = false;
    }
  } catch (const std::exception &e) {
    std::cerr << "[Easy Pipe] error in process() " << e.what() << std::endl;
    if (file_.is_open()) {
      file_.close();
    }
  }
}

// inline void EasyPipe::process() {
//   std::lock_guard<std::mutex> lock(buffMutex);

//   // Получаем данные в виде байтов и записываем в файл
//   auto bytes = buffer_.toBytes();
//   if (file.is_open() && !bytes.empty()) {
//     file.write(reinterpret_cast<const char *>(bytes.data()),
//     bytes.size()); file.flush();
//   }
//   // Очищаем буфер для следующей записи
//   for (size_t i = 0; i < bufferSize_; i++) {
//     buffer_.setBit(i, false);
//   }

//   readyRead = false;
//   readyWrite = true;
// }

bool IPipe::writeBitArrayBlock(const BitArray &block) {
  std::lock_guard<std::mutex> lock(buffMutex);
  if (!readyWrite || block.sizeInBits() > bufferSize_)
    return false;

  for (int i = 0; i < block.sizeInBits(); i++)
    buffer_.setBit(i, block.getBit(i));

  for (int i = block.sizeInBits(); i < bufferSize_; i++)
    buffer_.setBit(i, false);

  bufferBytes_ = buffer_.toBytes();
  readyWrite = false;
  readyRead = true;
  return true;
}

bool IPipe::readBitArrayBlock(BitArray &block, int expectedSize) {
  std::lock_guard<std::mutex> lock(buffMutex);
  if (!readyRead || expectedSize > bufferSize_)
    return false;

  block.begin(expectedSize);
  for (int i = 0; i < expectedSize; i++)
    block.setBit(i, buffer_.getBit(i));

  readyRead = false;
  readyWrite = true;
  return true;
}

IPipe::~IPipe() {
  stop = true;
  if (thread.joinable())
    thread.join();
}
} // namespace Pipe