#pragma once
#include <fstream>
#include <string>
#include <unordered_map>
#include "Crypto.h"

class DS3Bin
{
private:
	static DS3Bin* s_ins;

	const std::string EXE_NAME;
	std::string game_version;
	std::fstream io_stream;

	DS3Bin();
	DS3Bin(DS3Bin const&);
	void operator=(DS3Bin const &);

	std::string getGameVersion(std::fstream& ifs);

public:
	static DS3Bin& getIns();
	
	void getPublicKey(const std::string& name, ::RSA* rsa);
	void setPublicKey(const std::string& name, ::RSA* src);

	void finalize();
};

