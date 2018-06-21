// B3sBHDReSigner.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <string>
#include <iomanip>
#include "DS3Bin.h"
#include "Crypto.h"
#include "BHD.h"
#include <experimental\filesystem>
#include "Backup.h"
#include <vector>
#include <openssl\err.h>
#include <openssl\pem.h>
#include <openssl\conf.h>

const std::string BHD_LIST[] = {
	"Data1",
	"Data2",
	"Data3",
	"Data4",
	"Data5",
	//"DLC1",//uncomment those entries if you want to resign the DLC BHDs. Be aware though, your game will crash with those files unless you bypass the anti-cheat.
	//"DLC2",
};

int main(int argc, char* argv[])
{
	std::cout << "---------------------" << std::endl;
	std::cout << "B3s BHD ReSigner v1.0" << std::endl;
	std::cout << "---------------------\n" << std::endl;
	ERR_load_crypto_strings();
	OpenSSL_add_all_algorithms();
	OPENSSL_config(NULL);

	//load and re-sign BHDs
	std::vector<BHD*> bhds;
	try {
		for (auto& bhd_name : BHD_LIST) {
			bhds.push_back(new BHD(bhd_name));
			bhds.back()->reSign();
		}
	}
	catch (char* msg) {
		std::cout << msg << std::endl;
		std::cin.ignore();
		exit(1);
	}
	std::cout << "\nRe-signing done, saving results...\t";

	//Create backup in case things break while overwriting files.
	backup::createBackup();

	//save BHDs and inject new public keys into exe
	try {
		for (auto& bhd : bhds)
			bhd->save();
	}
	catch (char* msg) {
		std::cout << msg << std::endl;
		std::cout << "Please restore your files from backup." << std::endl;
		exit(1);
	}

	DS3Bin::getIns().finalize();

	std::cout << "Success!\n\nPress Enter to exit...";
	std::cin.ignore();

    return 0;
}

