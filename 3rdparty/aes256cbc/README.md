## Overview
This GitHub repository contains a standard implementation of the Advanced Encryption Standard (AES) with a focus on AES-256 CBC (Cipher Block Chaining) encryption. AES-256 is a highly secure encryption standard due to its 256-bit key size, providing robust protection for sensitive data. CBC mode ensures increased security by chaining each plaintext block to the previous ciphertext block before encryption, making it resistant to certain cryptographic attacks.

## Features
- **Header-only:** No need for external dependencies, simplifying integration into existing projects.
- **AES-256-CBC support:** Enables robust encryption and decryption using the Advanced Encryption Standard with a 256-bit key size and Cipher Block Chaining mode.
- **Seamless integration:** Provides straightforward usage examples for effortless incorporation into applications.
- **Compatibility:** Data encrypted using this standard AES-256 CBC algorithm can be decrypted using any compliant AES-256 CBC decryption implementation.

## Usage

To utilize this standard AES-256 CBC implementation, follow the instructions provided below:

### Including "AES_256_CBC.h" Header

Before utilizing the AES encryption and decryption functions, ensure that you've included the "AES_256_CBC.h" header in your code. This header provides access to the AES functionality.

```c
#include "AES_256_CBC.h"
```


### AES Context Initialization

To perform AES encryption and decryption, you must initialize an AES context. This context serves as a container for maintaining the encryption and decryption state.

```c
AES_CTX ctx;
```

### Initialize Encryption

To perform encryption, initialize the AES context and provide the encryption key and iv.

```c
AES_EncryptInit(&ctx, key, iv);
```



### Encryption Function

The AES encryption function allows you to encrypt a single blocks of data, and this implementation does not support padding. Ensure that you provide data blocks of precisely 16 bytes.

```c
AES_Encrypt(&ctx, plaintext, ciphertext);
```


### Initialize Decryption

To perform decryption, initialize the AES context with the same key used for encryption.

```c
AES_DecryptInit(&ctx, key, iv);
```

### Decryption Function

The AES decryption function allows you to decrypt a single blocks of data, and this implementation does not support padding. Ensure that you provide ciphertext blocks of precisely 16 bytes.

```c
AES_Decrypt(&ctx, ciphertext, plaintext);
```

### Encryption and Decryption Function Completion

After you have finished using the AES encryption and decryption functions and no longer require the AES context, it's essential to clear sensitive information from memory. Use the following function to achieve this:

```c
AES_CTX_Free(&ctx);
```

### Example Code (main.c)

Here's an example code snippet in C for using this standard AES-256 CBC implementation:

```c
#include <stdio.h>
#include "AES_256_CBC.h"

// Function to print the title and hexadecimal representation of data
void output(const char *title, const unsigned char *data, unsigned int size) {
    printf("%s", title);
    for (unsigned int index = 0; index < size; index++) {
        printf("%02X", data[index]);
    }
    printf("\n");
}

int main(int argc, const char *argv[]) {
    // AES-256 key (32 bytes)
    unsigned char key[AES_KEY_SIZE] = {
        0x49, 0x2F, 0xA8, 0x1E, 0xD7, 0x82, 0x4C, 0x93,
        0x36, 0x7B, 0xC1, 0xF8, 0xA0, 0xE5, 0x1A, 0x5D,
        0x98, 0xA1, 0x01, 0x11, 0x32, 0xE3, 0xC6, 0xC1,
        0xF3, 0x5C, 0x3A, 0xD6, 0x1E, 0x64, 0x12, 0xD6
    };
    
    // AES-256 iv (16 bytes)
    unsigned char iv[AES_BLOCK_SIZE] = {
        0x3A, 0x04, 0x2F, 0x3D, 0x37, 0x61, 0x34, 0x3D,
        0x49, 0x60, 0x33, 0x63, 0x4A, 0x3D, 0x36, 0x63
    };


    // Data block to be encrypted (16 bytes)
    unsigned char data[AES_BLOCK_SIZE] = {
        0x54, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20,
        0x61, 0x6E, 0x20, 0x64, 0x61, 0x74, 0x61, 0x21
    };

    // Print original data
    output("ori: 0x", data, 16);

    // Initialize AES context
    AES_CTX ctx;

    // Initialize encryption with the provided key and iv
    AES_EncryptInit(&ctx, key, iv);

    // Perform encryption
    AES_Encrypt(&ctx, data, data);

    // Print encrypted data
    output("enc: 0x", data, 16);

    // Initialize decryption with the same key and iv
    AES_DecryptInit(&ctx, key, iv);

    // Perform decryption
    AES_Decrypt(&ctx, data, data);

    // Print decrypted data
    output("dec: 0x", data, 16);

    // Clean up: zero out the round key array for security purposes
    AES_CTX_Free(&ctx);

    return 0;
}
```

### Example Code (main.c)

Here's an example code snippet in C demonstrating AES-256 CBC encryption and decryption with PKCS#7 padding support:

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include "AES_256_CBC.h"

// Encryption data with padding PKCS7
uint32_t EncryptData(uint8_t *data, uint32_t size, const uint8_t *key, const uint8_t *iv) {
	uint8_t padding_size = AES_BLOCK_SIZE - (size % AES_BLOCK_SIZE);
	uint32_t data_size = size + padding_size;

        // adding padding value
	for (uint8_t index = 0; index < padding_size; index++) {
		data[size + index] = padding_size;
	}
	
	AES_CTX ctx;
	AES_EncryptInit(&ctx, key, iv);
	
	for (uint32_t offset = 0; offset < data_size; offset += AES_BLOCK_SIZE) {
		AES_Encrypt(&ctx, data + offset, data + offset);
	}
	
	AES_CTX_Free(&ctx);
	
	return data_size;
}

// Decryption data with padding PKCS7
uint32_t DecryptData(uint8_t *data, uint32_t size, const uint8_t *key, const uint8_t *iv) {
	AES_CTX ctx;
	AES_DecryptInit(&ctx, key, iv);
	
	for (uint32_t offset = 0; offset < size; offset += AES_BLOCK_SIZE) {
		AES_Decrypt(&ctx, data + offset, data + offset);
	}
	
	AES_CTX_Free(&ctx);
	
	return size - data[size - 1];
}


void output(const char *title, const uint8_t *data, unsigned int size) {
    printf("%s", title);
    for (unsigned int index = 0; index < size; index++) {
        printf("%02X", data[index]);
    }
    printf("\n");
}

int main(int argc, const char *argv[]) {
	uint8_t data[32];
	uint8_t key[AES_KEY_SIZE];
	uint8_t iv[AES_BLOCK_SIZE];
	int data_len = 0;
	
	memcpy(data, "halloweeks", 10);
	memset(key, 0x69, 32);
	memset(iv, 0x78, 16);
	data_len = 10; // original data length
	
	output("ori: 0x", data, data_len);
	
	data_len = EncryptData(data, data_len, key, iv);
	
	output("enc: 0x", data, data_len);
	
	data_len = DecryptData(data, data_len, key, iv);
	
	output("dec: 0x", data, data_len);
	return 0;
}
```

## Contributions

Contributions and feedback are welcome! If you find issues or have ideas for improvements, please open an issue or submit a pull request.

## License

This standard AES-256 CBC implementation is provided under the [MIT License](./LICENSE).

