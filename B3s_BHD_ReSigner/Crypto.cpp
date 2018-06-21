#include "stdafx.h"
#include "Crypto.h"
#include <string>
#include <openssl\bio.h>
#include <openssl\pem.h>
#include <openssl\rsa.h>
#include <iostream>
#include <iomanip>
#include <experimental\filesystem>

namespace crypto{

	const int RSA_BLOCK_LENGTH = 256;

	namespace RSA {

		void RSA::keyGen(::RSA* rsa) {
			std::cout << "\tkey gen...\t";
			BIGNUM* bne = NULL;

			int bits = 2048;
			unsigned int e = 0x1f00992f;//FROM made that exponent up, not me.

			bne = BN_new();
			if (!BN_set_word(bne, e))
				throw "Failed to set BN during key generation";

			if (!RSA_generate_key_ex(rsa, bits, bne, NULL))
				throw "Failed to generate key";
		
			BN_free(bne);
			std::cout << "OK\n";
		}

		void RSA::publicKeyPemToRSA(::RSA* rsa, char* pem) {
			BIO* bo = BIO_new(BIO_s_mem());
			BIO_write(bo, pem, crypto::PEM_SIZE);
			if (PEM_read_bio_RSAPublicKey(bo, &rsa, 0, 0) == NULL)
				throw "Failed to import public key pem";
			BIO_free(bo);
			int ret = RSA_check_key_ex(rsa, NULL);
		}

		void RSA::RSAToPublicKeyPem(char* pem, ::RSA* rsa) {
			memset(pem, 0, crypto::PEM_SIZE);

			BIO* bo = BIO_new(BIO_s_mem());

			if (PEM_write_bio_RSAPublicKey(bo, rsa) == NULL)
				throw "Failed to write pem to bio in RSA::RSAToPublicKeyPem";
			BIO_read(bo, pem, crypto::PEM_SIZE);

			BIO_free(bo);
		}

		bool RSA::verifyEncryption(char* data, ::RSA* rsa) {
			std::cout << "\tverifying...\t";
			const unsigned int RSA_BLOCK_LENGTH = RSA_size(rsa);
			const int BND5_MAGIC(0x44484200);

			char* tmp_buf = new char[RSA_BLOCK_LENGTH];

			RSA_public_decrypt(RSA_BLOCK_LENGTH, (const unsigned char*)data, (unsigned char*)tmp_buf, rsa, RSA_NO_PADDING);
			
			if (*reinterpret_cast<int*>(tmp_buf) != BND5_MAGIC)
				throw "Failed encrytion verification step";

			delete[] tmp_buf;
			std::cout << "OK\n";
			return 0;		
		}

		void RSA::saveKeyPairAsPemFiles(std::string path, ::RSA* rsa){

			if (!std::experimental::filesystem::exists("BHD_keys")) {
				if (!std::experimental::filesystem::create_directory("BHD_keys"))
					throw "Failed to create key folder";
			}

			BIO* bp_public = NULL;
			BIO* bp_private = NULL;

			bp_public = BIO_new_file(("BHD_keys\\" + path + "_pub.pem").c_str(), "w+b");//TODO:fix name
			if (!PEM_write_bio_RSAPublicKey(bp_public, rsa))
				throw "Failed to save public key";
			bp_private = BIO_new_file(("BHD_keys\\" + path + "_priv.pem").c_str(), "w+b");
			if (!PEM_write_bio_RSAPrivateKey(bp_private, rsa, NULL, NULL, 0, NULL, NULL))
				throw "Failed to save private key";

			BIO_free_all(bp_public);
			BIO_free_all(bp_private);
		}

		void RSA::publicDecrypt(char* data, const unsigned int& len, ::RSA* rsa) {
			const unsigned int RSA_BLOCK_LENGTH = RSA_size(rsa);
			char* tmp_buf = new char[RSA_BLOCK_LENGTH];

			for (int i = 0; i < len / RSA_BLOCK_LENGTH; i++) {
				RSA_public_decrypt(RSA_BLOCK_LENGTH, (const unsigned char*)data + i * RSA_BLOCK_LENGTH, (unsigned char*)tmp_buf, rsa, RSA_NO_PADDING);
				memcpy_s(data + i * RSA_BLOCK_LENGTH, RSA_BLOCK_LENGTH, tmp_buf, RSA_BLOCK_LENGTH);
			}

			delete[] tmp_buf;
		}

		void RSA::privateEncrypt(char* data, const unsigned int& len, ::RSA* rsa) {
			int ret = RSA_check_key_ex(rsa, NULL);
			switch (ret) {
			case -1:
				throw "failure during key check 1";
				break;
			case 0:
				std::cout << "invalid key in privateEncrypt";
				throw "";
				break;
			}

			std::cout << "\tencrypting...\t";
			const unsigned int RSA_BLOCK_LENGTH = RSA_size(rsa);
			char* tmp_buf = new char[RSA_BLOCK_LENGTH];

			for (int i = 0; i < len / RSA_BLOCK_LENGTH; i++) {
				if(RSA_private_encrypt( RSA_BLOCK_LENGTH, (const unsigned char*)data + i * RSA_BLOCK_LENGTH, (unsigned char*)tmp_buf, rsa, RSA_NO_PADDING)==-1)
					std::cout << "RSA_private_encrypt failed"<< std::endl;
				memcpy_s(data + i * RSA_BLOCK_LENGTH, RSA_BLOCK_LENGTH, tmp_buf, RSA_BLOCK_LENGTH);
			}

			delete[] tmp_buf;
			std::cout << "OK\n";
		}
	}

	namespace TEA {

		const char* RSA_PUBLIC_KEY_HEADER = "-----BEGIN RSA PUBLIC KEY-----";
		const unsigned int N_ROUNDS = 0x20;
		const unsigned int SEED_INCR = 0xDF6A15C3 ^ 0x415D6C7A;
#pragma warning(suppress: 4307)
		const unsigned int DEOB_SEED = SEED_INCR * N_ROUNDS;

		void TEA_encrypt(char* PEM, const unsigned int* KEK) {
			int j;
			unsigned int vd;
			unsigned int* key = reinterpret_cast<unsigned int*>(PEM);

			for (int i = 0; i < PEM_SIZE / 4; i = i + 2) {
				j = 0;
				vd = 0;
				while (j != N_ROUNDS) {
					j++;
					vd += SEED_INCR;
					key[i] += (vd + key[i + 1]) ^ ((key[i + 1] << 4) + KEK[0]) ^ ((key[i + 1] >> 5) + KEK[1]);
					key[i + 1] += (vd + key[i]) ^ ((key[i] << 4) + KEK[2]) ^ ((key[i] >> 5) + KEK[3]);
				}
			};
		}

		void TEA_decrypt(char* PEM, const unsigned int* KEK) {
			int j;
			unsigned int vd;
			unsigned int* key = reinterpret_cast<unsigned int*>(PEM);

			for (int i = 0; i < PEM_SIZE / 4; i = i + 2) {
				j = N_ROUNDS;
				vd = DEOB_SEED;
				while (j != 0) {
					key[i + 1] -= (vd + key[i]) ^ ((key[i] << 4) + KEK[2]) ^ ((key[i] >> 5) + KEK[3]);
					key[i] -= (vd + key[i + 1]) ^ ((key[i + 1] << 4) + KEK[0]) ^ ((key[i + 1] >> 5) + KEK[1]);
					vd -= SEED_INCR;
					j--;
				}
			};
		}

		void xCrypt(char* PEM, const unsigned int* KEK) {
			if (memcmp(PEM, RSA_PUBLIC_KEY_HEADER, strlen(RSA_PUBLIC_KEY_HEADER)) == 0) {
				TEA_encrypt(PEM, KEK);
			}
			else {
				TEA_decrypt(PEM, KEK);
			}
		}

		const unsigned int* TEA::getKeyByName(std::string name) {
			const unsigned int* key;
			if (name == "DLC1") {
				key = crypto::TEA::DLC1_KEK;
			}
			else if (name == "DLC2") {
				key = crypto::TEA::DLC2_KEK;
			}
			else {
				key = crypto::TEA::DATA1_5_KEK;
			}
			return key;
		}
	}
}

