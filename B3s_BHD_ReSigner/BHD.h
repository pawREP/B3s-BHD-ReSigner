#pragma once
#include <string>
#include "Crypto.h"

class BHD
{
public:
	const std::string name;
	unsigned int size;
	char* data;
	::RSA* rsa;

	std::string getFilePath();

public:
	BHD(std::string name);
	~BHD();
	void reSign();
	void save();
	void save(std::string);
};

