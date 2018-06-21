#pragma once
#include <Windows.h>
#include <wincrypt.h>
#include <openssl\rsa.h>
#include <string>

namespace crypto{

	const unsigned int PEM_SIZE = 0x1B0;

	namespace RSA
	{
		void keyGen(::RSA*);
		void publicDecrypt(char* data, const unsigned int& len, ::RSA* rsa);
		void privateEncrypt(char* data, const unsigned int& len, ::RSA* rsa);

		bool verifyEncryption(char* data, ::RSA* rsa);
		void publicKeyPemToRSA(::RSA* rsa, char* pem);
		void RSAToPublicKeyPem(char* pem, ::RSA* rsa);

		void saveKeyPairAsPemFiles(std::string path, ::RSA* rsa);
	}
	namespace TEA{

		const unsigned int DATA1_5_KEK[4] = { 0x72AC72C4, 0x0ABE541E, 0x6CA329E6 ,0xD342C3BE };
		const unsigned int DLC1_KEK[4] = { 0x87FA0F11, 0x910EB47A, 0xC927705B, 0xC18BBD90 };
		const unsigned int DLC2_KEK[4] = { 0x29A007A8, 0x13C9A857, 0x7E685464, 0x0B5B0562 };

		const unsigned int* getKeyByName(std::string);
		void xCrypt(char* PEM, const unsigned int* KEK);
	};
};

