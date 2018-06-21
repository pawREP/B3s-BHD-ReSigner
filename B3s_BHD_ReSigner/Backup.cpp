#include "stdafx.h"
#include "Backup.h"
#include <string>
#include <experimental\filesystem>
#include <iostream>

const std::string BACKUP_DIR("BHD_ReSign_Backup");
const std::string FILE_LIST[] = {
	"Data1.bhd",
	"Data2.bhd",
	"Data3.bhd",
	"Data4.bhd",
	"Data5.bhd",
	"DLC1.bhd",
	"DLC2.bhd",
	"DarkSoulsIII.exe"
};

void backup::createBackup() {
	if (!std::experimental::filesystem::exists(BACKUP_DIR)) {
		if (!std::experimental::filesystem::create_directory(BACKUP_DIR))
			throw "Failed to create backup directory";

		for (auto& f : FILE_LIST) {
			if (std::experimental::filesystem::exists(f)) {
				if (!std::experimental::filesystem::copy_file(f, BACKUP_DIR + "\\" + f)) {
					std::experimental::filesystem::remove_all(BACKUP_DIR);
					throw "Failed to create backup";
				}
			}
		}
	}
}
