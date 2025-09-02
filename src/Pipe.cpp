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

    // private function
    bool IPipe::writeBuff2048(const bool* bits, size_t sizeBuff) {
        std::lock_guard<std::mutex> lock(buffMutex);
        if (!readyWrite || sizeBuff != bufferSize_)
            return false;

        // записываем биты в BitArray
        for (int i = 0; i < sizeBuff; i++)
            buffer_.setBit(i, bits[i]); // set an index and a value from bits

        // сохраняем в bufferBytes_ для чтения
        auto bytes = buffer_.toBytes();
        bufferBytes_ = bytes;

        readyWrite = false;
        readyRead = true;
        return true;
    }

    // public function
    bool IPipe::writeBuff2048(const bool* bits) {
        return writeBuff2048(bits, 2048);
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

    bool IPipe::readBuff2048(bool* bits, size_t size) {
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

    bool IPipe::readBuff2048(bool* bits) { return readBuff2048(bits, 2048); }

    void IPipe::run(IPipe* pipe) {
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

    EasyPipe::EasyPipe(Direction dir, const std::string& fileName, int bufferSize)
        : IPipe(dir), fileName_(fileName) {
        fs::path dataPath = fs::current_path();
        dataPath = dataPath.parent_path() / "data" / fileName_;

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
            << (dir_ == OUT ? "writing" : "reading") << " a file " << fileName_ << std::endl;
    }

    EasyPipe::~EasyPipe() {
        if (file_.is_open())
            file_.close();
    }

    std::streamsize EasyPipe::getFileSize(const std::string& filename) {
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
                    file_.write((char*)bytes.data(), bytes.size());
                    if (!file_)
                        throw std::runtime_error("[Process] Failed to open for writing");
                    file_.flush();
                    // сохранение копии для чтения
                    bufferBytes_ = bytes;
                }
                file_.close();

                //   // проверяем размер записанного файла
                //   std::streamsize fileSize = getFileSize(filename);
                //   if (fileSize <= 0) {
                //     std::cerr << "Cannot get file size or file is empty " << filename
                //               << std::endl;
                //     return;
                //   } else
                //     std::cout << "Successfully wrote " << fileSize << " bytes to "
                //               << filename;

                readyRead = false;
                readyWrite = true;
            }
            // чтение из файла
            else {
                file_.close();

                // 🔄 Ожидаем пока файл появится и будет не пустым
                int wait_attempts = 0;
                const int max_wait_attempts = 100;
                bool file_ready = false;

                while (wait_attempts < max_wait_attempts && !file_ready) {
                    std::ifstream test_file(fileName_, std::ios::binary | std::ios::ate);
                    if (test_file.is_open()) {
                        std::streamsize test_size = test_file.tellg();
                        test_file.close();

                        if (test_size > 0) {
                            file_ready = true;
                            break;
                        }
                    }
                    wait_attempts++;
                    std::this_thread::sleep_for(std::chrono::milliseconds(5));
                }

                if (!file_ready) {
                    std::cout << "File not ready for reading after " << max_wait_attempts
                        << " attempts" << std::endl;
                    return;
                }
                file_.open(fileName_, std::ios::binary | std::ios::in);

                if (!file_.is_open()) {
                    std::cout << "File cannot be opened for reading" << std::endl;
                    return;
                }

                // Проверяем размер файла
                file_.seekg(0, std::ios::end);
                std::streamsize fileSize = file_.tellg();
                file_.seekg(0, std::ios::beg);

                auto bytes = buffer_.toBytes();
                if (fileSize >= static_cast<std::streamsize>(bytes.size())) {
                    // Читаем данные в bufferBytes_
                    bufferBytes_.resize(bytes.size());
                    file_.read(reinterpret_cast<char*>(bufferBytes_.data()), bytes.size());

                    // Обновляем BitArray из bufferBytes_
                    buffer_.setFromBytes(bufferBytes_);

                    file_.close();

                    // Очищаем файл
                    std::ofstream clear_file(fileName_, std::ios::trunc);
                    clear_file.close();

                    readyRead = true;
                    readyWrite = false;
                }
                else {
                    file_.close();
                }
            }
        }
        catch (const std::exception& e) {
            std::cerr << "[Easy Pipe] error in process() " << e.what() << std::endl;
            if (file_.is_open()) {
                file_.close();
            }
        }
    }

    // Для записи: Закрыть → Открыть с trunc → Записать → Проверить → Закрыть
    // Для чтения: Закрыть → Открыть → Проверить размер → Прочитать → Очистить
    // файл → Закрыть

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

    IPipe::~IPipe() {
        stop = true;
        if (thread.joinable())
            thread.join();
    }
} // namespace Pipe