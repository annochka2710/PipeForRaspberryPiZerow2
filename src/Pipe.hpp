#pragma once

#include <cstdint>
#include <fstream>
#include <mutex>
#include <thread>
#include "BitArray.hpp"
#include <stdio.h>
#include <concepts>

namespace Pipe {
    /// Интерфейс передачи данных
    class IPipe {
    protected:
        BitArray buffer_;
        std::vector<uint8_t> bufferBytes_; //храним байты
        size_t bufferSize_;
        bool readyRead = false;
        bool readyWrite = true;
        std::mutex buffMutex;
        std::thread thread;
        bool stop = false;

    public:
        enum Direction {
            IN, /// PIPE Принимает данные от пользователя (write)
            OUT /// PIPE Выдает данные пользователю (read)
        };

        IPipe(Direction dir, size_t bufferSize = 2048);

        ~IPipe();

        bool isReadyRead() const;
        bool isReadyWrite() const;

        bool writeBuff2048(const bool* bit);
        bool readBuff2048(bool* bit);

        bool writeBlock(const std::vector<bool>& block);
        bool readBlock(std::vector<bool>& block, int size);

        bool writeBitArrayBlock(const BitArray& block);    
        bool readBitArrayBlock(BitArray& block, int expectedSize);

        static void run(IPipe* pipe);

        void setBufferSize(size_t newSize);
        size_t getBufferSize() const;

    protected:
        virtual void process() = 0;
        Direction dir_;

    private:
        bool writeBuff2048(const bool* bit, size_t sizeBuff);
        bool readBuff2048(bool* bit, size_t size);
    };

    /// Тоннель передачи данных
    template <std::derived_from<IPipe> InPipe, std::derived_from<IPipe> OutPipe>
    class Tonnel {
        OutPipe in;
        InPipe out;

    public:
        Tonnel(size_t bufferSize = 2048) : in(IPipe::OUT, bufferSize), out(IPipe::IN, bufferSize) {}
        void setBufferSize(int newSize) {
            in.setBufferSize(newSize);
            out.setBufferSize(newSize);
        }
    };

    /// Простая реализация интерфейса передачи данных
    class EasyPipe : public IPipe {
        std::fstream file_;
        std::string fileName_;

        std::streamsize getFileSize(const std::string& fileName_);
    public:
        EasyPipe(Direction dir, const std::string& filename = "pipe.bin", int bufferSize = 2048);
        ~EasyPipe();
        void process() override;

    };

    /// Интерфейс передачи данных через лазерный модуль
    class LaserPipe : public IPipe {
    public:
        LaserPipe(Direction dir) : IPipe(dir) {}
        void process() override {}
    };

    /// Интерфейс передачи данных через инфракрасный модуль
    class InfraRedPipe : public IPipe {
    public:
        InfraRedPipe(Direction dir) : IPipe(dir) {}
        void process() override {}
    };
} // namespace Pipe