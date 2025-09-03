#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING

#include "BitArray.hpp"
#include "Pipe.hpp"
#include <iostream>
#include <filesystem>

const int oneBlockSize = 2048;

int main() {
  const std::string filename = "resources/story.txt";
  if (!std::filesystem::exists(filename)) {
    std::cout << "[MAIN] No file " << filename << " in dir "
              << std::filesystem::current_path() << std::endl;
    return 1;
  }

  std::filesystem::create_directory("output");
  const std::string outfilename = "output/story_processed.txt";

  // загружаем данные из файла
  BitArray buf;
  if (!buf.loadFromFile(filename)) {
    std::cout << "[MAIN] Failed to load a file\n";
    return 1;
  }
  std::cout << "[MAIN] Load success. File size " << buf.sizeInBytes() << "\n";
  
  Pipe::EasyPipe out(Pipe::IPipe::OUT); // канал для чтения (создаем файл здесь)
  Pipe::EasyPipe in(Pipe::IPipe::IN);   // канал для записи
  std::this_thread::sleep_for(std::chrono::milliseconds(5000));

  int totalBits = buf.sizeInBits();
  BitArray output(totalBits);
  int processedBits = 0;

  while (totalBits > processedBits) {
    //определение минимального размера блока
    int blockSize = std::min(oneBlockSize, totalBits - processedBits);

    //создание BitArray для блока
    BitArray blockBits(blockSize);
    for (int i = 0; i < blockSize; i++) 
      blockBits.setBit(i, buf.getBit(processedBits + i));

    //записываем в пайп по блокам
    if (!in.writeBitArrayBlock(blockBits)) {
      std::cout << "[MAIN] Failed to write block to pipe\n";
      return 1;
    }

    //читаем из пайпа блоками
    BitArray processedBlock;
    if (!out.readBitArrayBlock(processedBlock, blockSize)) {
      std::cout << "[MAIN] Failed to read block from pipe\n";
      return 1;
    }

    //копирование результата
    for (int i = 0; i < blockSize; i++)
      output.setBit(processedBits + i, processedBlock.getBit(i));

    processedBits += blockSize;
    std::cout << "Processed " << processedBits << " of " << totalBits << " totalBits\n";
  }

  // сохранение данных в файл
  if (output.saveToFile(outfilename)) {
    std::cout << "Input file " << filename << " sizeBytes " << buf.sizeInBits()
              << "\n";
    std::cout << "Output file " << outfilename << " sizeBytes "
              << output.sizeInBits() << "\n";
    bool equal = BitArray::checkEqualFiles(buf, output);
    std::cout << "Data is " << (equal ? "EQUAL\n" : "DIFFERENT\n");
  }

  std::cout << "First 64 bits\n";
  std::cout << "Original\n";
  for (int i = 0; i < 64; i++) {
    std::cout << (buf.getBit(i) ? "1" : "0");
    if ((i + 1) % 8 == 0)
      std::cout << " ";
  }
  std::cout << std::endl;

  std::cout << "Final\n";
  for (int i = 0; i < 64; i++) {
    std::cout << (output.getBit(i) ? "1" : "0");
    if ((i + 1) % 8 == 0)
      std::cout << " ";
  }
  std::cout << std::endl;
  return 0;
}