#pragma once

#include <string>
#include <vector>

class BitArray {
public:
	BitArray(int sizeInBits_);
	BitArray();
	~BitArray() = default;

	void begin(const int sizeBits); // инициализация массива размером sizeBits

	void setBit(int index, bool value); // установка значения бита (в 1 или 0)
	bool getBit(int index) const; // получение значения бита

	void setBits(int index, const std::vector<bool>& bits);

	int sizeInBits() const;
	int sizeInBytes() const;

	bool isEmpty() const;

	std::vector<uint8_t> toBytes() const;

	void setFromBytes(const std::vector<uint8_t>& bytes);

	bool loadFromFile(const std::string& filename); //чтение из файла
	bool saveToFile(const std::string& filename);   //запись в файл
	static bool checkEqualFiles(BitArray file1, BitArray file2);

	std::vector<uint8_t> toBoolVector();

	bool operator[](int index) { return getBit(index); } // оператор доступа
private:
	std::streamsize findFileSize(std::ifstream &file); //узнать размер файла

	int sizeBits_;                // размер массива в битах
	std::vector<uint8_t> array_;  // массив байтов
};