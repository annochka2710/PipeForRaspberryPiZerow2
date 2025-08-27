#include "Pipe.hpp"
#include <iostream>

int main()
{
    std::array<bool, 2048> test; //Исходные данные для записи (2048 бит)
    std::array<bool, 2048> testOut; //поток на чтение
    
    //заполнение случайными битами (0/1)
    for (auto &i : test)
        i = ((double)rand() / RAND_MAX) > 0.5;

    Pipe::EasyPipe in(Pipe::IPipe::IN); //канал для записи
    Pipe::EasyPipe out(Pipe::IPipe::OUT); //канал для чтения
    
    in.writeBuff2048(test.data()); //записываем 2048 бит в канал

    while (true)
        if (out.readBuff2048(testOut.data()) == true) break; //выход при успешном чтении

    for (int i = 0; i < test.size(); i++){
        std::cout << "Original\n";
        std::cout << (test[i] ? "1" : "0");
    }
    std::cout << std::endl;

    for (int i = 0; i < testOut.size(); i++){
        std::cout << "Received\n";
        std::cout << (testOut[i] ? "1" : "0");
    }
    std::cout << std::endl;

    return 0;
}