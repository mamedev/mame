## Overview

This repository houses a standard AES-192 ECB (Electronic Codebook) encryption algorithm implemented in C. AES-192 is known for its enhanced security.

## Features

- **AES-192 ECB:** Implements AES-192 ECB encryption mode for secure data processing.

- **Fixed Block Size:** Enforces a fixed 16-byte block size without padding.

- **Compatibility:** Encrypted data is interoperable with any AES-192 ECB decryption.

## Usage

Include "AES_192_ECB.h" before using the AES-192 ECB functions.

```c
#include "AES_192_ECB.h"
```

### AES Context Initialization

```c
AES_CTX ctx;
```

### Initialize Encryption

```c
AES_EncryptInit(&ctx, key);
```

### Encryption Function

```c
AES_Encrypt(&ctx, plaintext, ciphertext);
```

The `AES_Encrypt` function is designed to encrypt a single 16-byte block of data.

### Encryption Function for Multiple Blocks

If your data is in multiple blocks, it is recommended to use a loop to encrypt each block individually:

```c
for (unsigned int offset = 0; offset < multiple_block_size; offset += AES_BLOCK_SIZE) {
    AES_Encrypt(&ctx, plaintext + offset, ciphertext + offset);
}
```

This loop allows you to encrypt each 16-byte block of data individually when dealing with data spanning multiple blocks. Remember that each block must be exactly 16 bytes in size for proper encryption.

### Initialize Decryption

```c
AES_DecryptInit(&ctx, key);
```

### Decryption Function

```c
AES_Decrypt(&ctx, ciphertext, plaintext);
```

The `AES_Decrypt` function is designed to decrypt a single 16-byte block of data.

### Completion

```c
AES_CTX_Free(&ctx);
```

### Example Code (main.c)

```c
#include <stdio.h>
#include "AES_192_ECB.h"

int main() {
    AES_CTX ctx;
    uint8_t key[AES_KEY_SIZE];
    uint8_t data[AES_BLOCK_SIZE * 3]; // Example data with three blocks

    // Initialization and usage example
    memcpy(key, "This is aes key!", AES_KEY_SIZE);
    memcpy(data, "This is test msg123456", AES_BLOCK_SIZE * 3);

    printf("Original:  ");
    for (unsigned int index = 0; index < AES_BLOCK_SIZE * 3; index++) {
        printf("%02X", data[index]);
    }
    printf("\n");

    AES_EncryptInit(&ctx, key);

    for (unsigned int offset = 0; offset < AES_BLOCK_SIZE * 3; offset += AES_BLOCK_SIZE) {
        AES_Encrypt(&ctx, data + offset, data + offset);
    }

    printf("Encrypted: ");
    for (unsigned int index = 0; index < AES_BLOCK_SIZE * 3; index++) {
        printf("%02X", data[index]);
    }
    printf("\n");

    AES_DecryptInit(&ctx, key);

    for (unsigned int offset = 0; offset < AES_BLOCK_SIZE * 3; offset += AES_BLOCK_SIZE) {
        AES_Decrypt(&ctx, data + offset, data + offset);
    }

    printf("Decrypted: ");
    for (unsigned int index = 0; index < AES_BLOCK_SIZE * 3; index++) {
        printf("%02X", data[index]);
    }
    printf("\n");

    AES_CTX_Free(&ctx);
    return 0;
}
```

## Contributions

Contributions and feedback are welcome! Open issues or submit pull requests.

## License

This AES-192 ECB implementation is provided under the [MIT License](./LICENSE).
