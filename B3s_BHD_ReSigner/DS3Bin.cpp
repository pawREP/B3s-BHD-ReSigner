#include "stdafx.h"
#include "DS3Bin.h"
#include <string>
#include <iostream>
#include <filesystem>
#include <exception>
#include <unordered_map>
#include "Crypto.h"

DS3Bin * DS3Bin::s_ins = nullptr;

//Maps BHD names encrypted RSA key location file offset
const std::unordered_map<std::string, int> encrypted_RSA_offsets{
	{ "Data1", 0x51A69C9 },
	{ "Data2", 0x51A6B79 },
	{ "Data3", 0x51A6D29 },
	{ "Data4", 0x51A6ED9 },
	{ "Data5", 0x51A7089 },
	{ "DLC1",  0x54F9F79 },
	{ "DLC2",  0x5D3FDC0 }
};


DS3Bin& DS3Bin::getIns() {
	if (!s_ins)
		s_ins = new DS3Bin;
	return *s_ins;
}

//Get game version based on timestamp in PE header
std::string DS3Bin::getGameVersion(std::fstream& ifs) {
	int timestamp;
	ifs.seekg(60);
	int PE_header_offset = 0;
	ifs.read(reinterpret_cast<char*>(&PE_header_offset), sizeof(int));
	ifs.seekg(PE_header_offset + 8);
	ifs.read(reinterpret_cast<char*>(&timestamp), sizeof(int));
	
	switch (timestamp) {
	case 0x598A43EF:
		game_version = "v1.15";
		break;
	default:
		throw "Unknown game vesion";
	}
	return game_version;
}

DS3Bin::DS3Bin() : EXE_NAME("DarkSoulsIII.exe")
{

	if (!std::experimental::filesystem::exists(EXE_NAME))
		throw "DarkSoulsIII.exe not found";

	io_stream = std::fstream(EXE_NAME, std::ios::in | std::ios::out | std::ios::binary);
	if (!io_stream.is_open())
		throw "Failed to open DarkSoulsIII.exe. File might be in use.";

	if (getGameVersion(io_stream) == "unknown")
		throw "Current game version is not supported";
}

//Loads pem from exe, decrypts it and converts it into ::RSA format.
void DS3Bin::getPublicKey(const std::string& name, ::RSA* rsa){
	char* pem_buffer = new char[crypto::PEM_SIZE];

	io_stream.seekg(encrypted_RSA_offsets.at(name));
	io_stream.read(pem_buffer, crypto::PEM_SIZE);

	const unsigned int* key = crypto::TEA::getKeyByName(name);
	crypto::TEA::xCrypt(pem_buffer, key);

	crypto::RSA::publicKeyPemToRSA(rsa, pem_buffer);

	/*std::cout << name <<":\n\n";
	for(int i = 0; i< crypto::PEM_SIZE-2; i++)
		std::cout << pem_buffer[i];
	std::cout << "\n\n";*/

	delete[] pem_buffer;
}

void DS3Bin::setPublicKey(const std::string& name, ::RSA* rsa) {
	char* pem_buffer = new char[crypto::PEM_SIZE];
	crypto::RSA::RSAToPublicKeyPem(pem_buffer, rsa);

	/*std::cout << name <<":\n\n";
	for(int i = 0; i< crypto::PEM_SIZE-2; i++)
	std::cout << pem_buffer[i];
	std::cout << "\n\n";*/

	const unsigned int* key = crypto::TEA::getKeyByName(name);
	crypto::TEA::xCrypt(pem_buffer, key);
	io_stream.seekp(encrypted_RSA_offsets.at(name));
	io_stream.write(pem_buffer, crypto::PEM_SIZE);
}

void DS3Bin::finalize() {
	io_stream.close();
}
