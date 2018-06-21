#include "stdafx.h"
#include "BHD.h"
#include <fstream>
#include "DS3Bin.h"
#include "Crypto.h"
#include <iostream>
#include <iomanip>

#define DEBUG

const int BND5_MAGIC(0x44484200);

std::string BHD::getFilePath() {
	return name + std::string(".bhd");
}

BHD::BHD(std::string name_) : name(name_)
{
	std::cout << name << ":\n";
	std::cout << "\treading...\t";
	//Load data from file
	std::ifstream ifs(getFilePath(), std::ios::in | std::ios::binary | std::ios::ate);
	if (!ifs.is_open())
		throw "Failed to open BHD";
	size = ifs.tellg();
	ifs.seekg(0, std::ios::beg);

	data = new char[size];
	ifs.read(data, size);
	if (ifs.fail())
		throw "Error while reading BND file";
	ifs.close();
	std::cout << "OK" << "\n";

	//Get key and decrypt data
	std::cout << "\tdecrypting...\t";
	rsa = RSA_new();
	DS3Bin::getIns().getPublicKey(name, rsa);
	crypto::RSA::publicDecrypt(data, size, rsa);

	if (*reinterpret_cast<int*>(data) != BND5_MAGIC) {
		throw "Oh no, looks like your BHD and Exe files are out of sync.\nRestore your files from backup and try again.";
	}
	std::cout << "OK" << "\n";
}


BHD::~BHD()
{
	RSA_free(rsa);
	delete[] data;
}

void BHD::reSign() {
	RSA_free(rsa);
	rsa = RSA_new();
	crypto::RSA::keyGen(rsa);
	crypto::RSA::saveKeyPairAsPemFiles(name, rsa);
	crypto::RSA::privateEncrypt(data, size, rsa);
	crypto::RSA::verifyEncryption(data, rsa);
}

void BHD::save() {
	DS3Bin::getIns().setPublicKey(name, rsa);
	save(name);
}

void BHD::save(std::string name_) {
	std::ofstream ofs(name_ + ".bhd", std::ios::binary | std::ios::out);
	if (!ofs.is_open())
		throw "could not write bhd out";
	ofs.write(data, size);
	ofs.close();
}
