# B3s BHD Re-signer
This tool allows re-signing of Dark Souls 3's archive header files (BHD) with a new RSA key pairs.

### How it works
Dark Souls 3's archive header files are encrypted with RSA private keys only available to FromSoftware. The corresponding public keys are TEA encrypted with one of three keks and embedded in the games executable. The lack of publicly available private keys makes modding BHD files impossible or at least difficult and limited.
To get around this issue, the BHD re-signer extracts and decrypts the RSA public keys, decrypts all BHD files, generates new key pairs, encrypts BHD files with the new private keys and finally encrypts and injects the new public keys into the game's binary.

### How to use it
Close the game, put the re-signer exe into your GAME directory, execute it.

### Notes about DLC1-2.bhd & tamper protection
The DLC BHD files or more precisely the encrypted public keys are subject to additional run-time protections. Re-signed DLC BHDs can be detected by DS3's anti-tamper system and detections will result in a game crash. Re-signing DLC files is therefor disabled by default. Data1-5.bhd modifications might be detected as well but don't result in a game crash. The tool is intended for offline use only.

#### "Why did you make this you could just ..."
I'm well aware that the game can be modified to load resources from outside of the archives, that's not the point. My main goal with this works was to improve my reverse engineering skills and understanding of cryptography. The re-sign tool is just a means of sharing the information. 