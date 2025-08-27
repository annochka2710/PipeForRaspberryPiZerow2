#pragma once

#include <array>
#include <fstream>
#include <mutex>
#include <thread>

namespace Pipe {
    /// Интерфейс передачи данных
    class IPipe {
    protected:
        std::array<bool, 2048> buf;
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

        IPipe(Direction dir);

        ~IPipe();

        bool isReadyRead() const;
        bool isReadyWrite() const;

        bool writeBuff2048(const bool *bits);

        bool readBuff2048(bool *bits);

        static void run(IPipe *pipe);

    protected:
        virtual void process() = 0;
        Direction dir_;
    };

    /// Тоннель передачи данных
    template <std::derived_from<IPipe> InPipe, std::derived_from<IPipe> OutPipe>
    class Tonnel {
        OutPipe in;
        InPipe out;

    public:
    };

    /// Простая реализация интерфейса передачи данных
    class EasyPipe : public IPipe {
        std::fstream file;
        std::string filename;

    public:
        EasyPipe(Direction dir, const std::string& filename = "story.txt");
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