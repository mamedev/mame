## Overview

This GitHub repository contains a standard implementation of the Advanced Encryption Standard (AES) with a focus on AES-128 ECB (Electronic Codebook) encryption. AES-128 is a widely adopted encryption standard known for its security and efficiency.

## Features

- **AES-128 ECB:** This implementation adheres to the AES-128 ECB encryption mode, which encrypts individual 16-byte blocks of data independently.

- **Fixed Block Size:** The algorithm enforces a fixed block size of 16 bytes, and it does not support padding. Input data must be exactly 16 bytes in length for each block.

- **Compatibility:** Data encrypted using this standard AES-128 ECB algorithm can be decrypted using any compliant AES-128 ECB decryption implementation.

## Usage

To utilize this standard AES-128 ECB implementation, follow the instructions provided below:

### Including "AES_128_ECB.h" Header

Before utilizing the AES encryption and decryption functions, ensure that you've included the "AES_128_ECB.h" header in your code. This header provides access to the AES functionality.

```c
#include "AES_128_ECB.h"
```

### AES Context Initialization

To perform AES encryption and decryption, you must initialize an AES context. This context serves as a container for maintaining the encryption and decryption state.

```c
AES_CTX ctx;
```

### Initialize Encryption

To perform encryption, initialize the AES context and provide the encryption key.

```c
AES_EncryptInit(&ctx, key);
```

### Encryption Function

The AES encryption function allows you to encrypt a blocks of data, and this implementation does not support padding. Ensure that you provide data blocks of precisely 16 bytes.

```c
AES_Encrypt(&ctx, plaintext, plaintext_length, ciphertext);
```

### Initialize Decryption

To perform decryption, initialize the AES context with the same key used for encryption.

```c
AES_DecryptInit(&ctx, key);
```

### Decryption Function

The AES decryption function allows you to decrypt a blocks of data, and this implementation does not support padding. Ensure that you provide ciphertext blocks of precisely 16 bytes.

```c
AES_Decrypt(&ctx, ciphertext, ciphertext_length, plaintext);
```

### Encryption and Decryption Function Completion

After you have finished using the AES encryption and decryption functions and no longer require the AES context, it's essential to clear sensitive information from memory. Use the following function to achieve this:

```c
AES_CTX_Free(&ctx);
```

### Example Code (main.c)

Here's an example code snippet in C for using this standard AES-128 ECB implementation:

```c
#include <stdio.h>
#include "AES_128_ECB.h"

// Function to print the title and hexadecimal representation of data
void output(const char *title, const unsigned char *data, unsigned int size) {
    printf("%s", title);
    for (unsigned int index = 0; index < size; index++) {
        printf("%02X", data[index]);
    }
    printf("\n");
}

int main(int argc, const char *argv[]) {
    // AES-128 key (16 bytes)
    unsigned char key[AES_KEY_SIZE] = {
        0x49, 0x2F, 0xA8, 0x1E, 0xD7, 0x82, 0x4C, 0x93,
        0x36, 0x7B, 0xC1, 0xF8, 0xA0, 0xE5, 0x1A, 0x5D
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

    // Initialize encryption with the provided key
    AES_EncryptInit(&ctx, key);

    // Perform encryption
    AES_Encrypt(&ctx, data, 16, data);

    // Print encrypted data
    output("enc: 0x", data, 16);

    // Initialize decryption with the same key
    AES_DecryptInit(&ctx, key);

    // Perform decryption
    AES_Decrypt(&ctx, data, 16, data);

    // Print decrypted data
    output("dec: 0x", data, 16);

    // Clean up: zero out the round key array for security purposes
    AES_CTX_Free(&ctx);

    return 0;
}
```

## Contributions

Contributions and feedback are welcome! If you find issues or have ideas for improvements, please open an issue or submit a pull request.

## License

This standard AES-128 ECB implementation is provided under the [MIT License](./LICENSE).
