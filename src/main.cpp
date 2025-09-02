#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING

#include "BitArray.hpp"
#include "Pipe.hpp"
#include <iostream>
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#define RED "\033[31m"

int main()
{
    BitArray testIn(2048); //Исходные данные для записи (2048 бит)
    BitArray testOut(2048); //поток на чтение
    //fs::path dataDir = fs::current_path() / "data";
    //fs::create_directories(dataDir); // создаем папку если не существует

    std::ofstream outfile;
    outfile.open("pipe.bin", std::ios_base::out);
    if (outfile.is_open()) {
        std::cout << "[MAIN] File was created\n";
        outfile.close();
    }
    else {
        std::cerr << "[MAIN] File was NOT created\n";
        return 0;
    }

    //заполнение случайными битами
    for (int i = 0; i < 2048; i++) {
        bool randomBit = ((double)rand() / RAND_MAX) > 0.5;
        testIn.setBit(i, randomBit);
    }

    Pipe::EasyPipe in(Pipe::IPipe::IN); //канал для записи
    Pipe::EasyPipe out(Pipe::IPipe::OUT); //канал для чтения

    //конвертация BitArray в буловый массив для записи
    auto vec = testIn.toBoolVector();

    //in.writeBuff2048(test.data()); //записываем 2048 бит в канал
    in.writeBuff2048(reinterpret_cast<const bool*>(vec.data()));

    while (true) {
        std::vector<uint8_t> temp(2048);
        if (out.readBuff2048(reinterpret_cast<bool*>(temp.data()))) {
            for (int i = 0; i < 2048; i++)
                testOut.setBit(i, temp[i] != 0);
            break;
        }
    }

    std::cout << RED << "Original:" << "\033[0m" << std::endl;
    for (int i = 0; i < 2048; i++) {
        std::cout << (testIn.getBit(i) ? "1" : "0");
        if ((i + 1) % 8 == 0) std::cout << " "; // разделяем байты
        if ((i + 1) % 64 == 0) std::cout << std::endl; // новая строка каждые 8 байт
    }
    std::cout << std::endl << std::endl;

    std::cout << RED << "Received:" << "\033[0m" << std::endl;
    for (int i = 0; i < 2048; i++) {
        std::cout << (testOut.getBit(i) ? "1" : "0");
        if ((i + 1) % 8 == 0) std::cout << " ";
        if ((i + 1) % 64 == 0) std::cout << std::endl;
    }
    std::cout << std::endl;

    // Проверка на идентичность
    bool identical = true;
    for (int i = 0; i < 2048; i++) {
        if (testIn.getBit(i) != testOut.getBit(i)) {
            identical = false;
            break;
        }
    }

    std::cout << "Data is " << (identical ? "IDENTICAL" : "DIFFERENT") << std::endl;

    return 0;
}