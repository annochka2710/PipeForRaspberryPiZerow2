#include <iostream>
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include "Pipe.hpp"
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

namespace Pipe {
IPipe::IPipe(Direction dir) : dir_(dir) {
  thread = std::thread(IPipe::run, this);
}

bool IPipe::isReadyRead() const { return readyRead; }

bool IPipe::isReadyWrite() const { return readyWrite; }

bool IPipe::writeBuff2048(const bool *buff) {
  if (!readyWrite)
    return false;
  if (!buffMutex.try_lock())
    return false;
  std::swap(readyWrite, readyRead);
  memcpy(buf.data(), buff, buf.size());
  buffMutex.unlock();
  return true;
}

bool IPipe::readBuff2048(bool *buff) {
  if (!readyRead)
    return false;
  if (!buffMutex.try_lock())
    return false;
  std::swap(readyWrite, readyRead);
  memcpy(buff, buf.data(), buf.size());
  buffMutex.unlock();
  return true;
}

void IPipe::run(IPipe *pipe) {
  while (!pipe->stop) {
    if (pipe->dir_ == IN && !pipe->isReadyRead())
      continue;
    if (pipe->dir_ == OUT && !pipe->isReadyWrite())
      continue;
    pipe->process();
  }
}

EasyPipe::EasyPipe(Direction dir, const std::string &fname)
    : IPipe(dir), filename(fname) {
  if (dir_ == IN)
    file.open(filename, std::ios::out);
  if (dir_ == OUT)
    file.open(filename, std::ios::in);

  if (!file.is_open())
    throw std::runtime_error("Failed to open file");
}

inline void EasyPipe::process() {
  buffMutex.lock();
        while (!stop) {
            if (dir_ == IN)
            {/// запись в файл
                file.seekg(0, std::ios::end);
                if (file.tellg() != 0) continue;
                file.write((char*)buf.data(), buf.size());
                file.flush();
                readyRead = false;
                readyWrite = true;
            }
            else
            {/// чтение из файла
                file.seekg(0, std::ios::end);
                if (file.tellg() < buf.size() - 1) continue;
                file.seekg(0, std::ios::beg);
                file.read((char*)buf.data(), buf.size());
                fs::resize_file("pipe.bin", 0);
                file.seekg(0, std::ios::beg);
                readyRead = true;
                readyWrite = false;
            }
            break;
        }
        buffMutex.unlock();
}

IPipe::~IPipe() {
  stop = true;
  if (thread.joinable())
    thread.join();
}
} // namespace Pipe