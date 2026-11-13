# AES-192 CBC Encryption Implementation in C

This repository showcases a concise implementation of the AES-192 CBC (Cipher Block Chaining) encryption algorithm in C, renowned for its heightened security.

## Features

- **AES-192 CBC:** Implements secure AES-192 CBC encryption mode.
- **Fixed Block Size:** Enforces a 16-byte fixed block size without padding.
- **Compatibility:** Encrypted data is interoperable with any AES-192 CBC decryption.

## Usage

Include "AES_192_CBC.h" before using AES-192 CBC functions.

```c
#include "AES_192_CBC.h"
```

### AES Context Initialization

```c
AES_CTX ctx;
```

### Function: `AES_EncryptInit`

Initialize the AES context for encryption.

```c
void AES_EncryptInit(AES_CTX *ctx, const uint8_t *key, const uint8_t *iv);
```

- `ctx`: AES context structure.
- `key`: Encryption key (24 bytes for AES-192).
- `iv`: Initialization vector (16 bytes).

### Function: `AES_Encrypt`

Encrypt a single 16-byte block of data.

```c
void AES_Encrypt(AES_CTX *ctx, const uint8_t *plaintext, uint8_t *ciphertext);
```

- `ctx`: AES context structure.
- `plaintext`: Input plaintext block (16 bytes).
- `ciphertext`: Output encrypted block (16 bytes).

For multiple blocks, use a loop:

```c
for (unsigned int offset = 0; offset < multiple_block_size; offset += AES_BLOCK_SIZE) {
    AES_Encrypt(&ctx, plaintext + offset, ciphertext + offset);
}
```

### Function: `AES_DecryptInit`

Initialize the AES context for decryption.

```c
void AES_DecryptInit(AES_CTX *ctx, const uint8_t *key, const uint8_t *iv);
```

- `ctx`: AES context structure.
- `key`: Decryption key (24 bytes for AES-192).
- `iv`: Initialization vector (16 bytes).

### Function: `AES_Decrypt`

Decrypt a single 16-byte block of data.

```c
void AES_Decrypt(AES_CTX *ctx, const uint8_t *ciphertext, uint8_t *plaintext);
```

- `ctx`: AES context structure.
- `ciphertext`: Input encrypted block (16 bytes).
- `plaintext`: Output decrypted block (16 bytes).

For multiple blocks, use a loop:

```c
for (unsigned int offset = 0; offset < multiple_block_size; offset += AES_BLOCK_SIZE) {
    AES_Decrypt(&ctx, ciphertext + offset, plaintext + offset);
}
```

### Function: `AES_CTX_Free`

Free resources associated with the AES context.

```c
void AES_CTX_Free(AES_CTX *ctx);
```

- `ctx`: AES context structure.

### Example Code (main.c)

```c
#include <stdio.h>
#include "AES_192_CBC.h"

int main() {
    AES_CTX ctx;
    uint8_t key[AES_KEY_SIZE];
    uint8_t iv[AES_BLOCK_SIZE];
    uint8_t data[AES_BLOCK_SIZE * 3]; // Example data with three blocks

    // Initialization and usage example
    memcpy(key, "This is aes key!", AES_KEY_SIZE);
    memcpy(iv, "This is an IV123", AES_BLOCK_SIZE);
    memcpy(data, "This is test msg123456", AES_BLOCK_SIZE * 3);

    printf("Original:  ");
    for (unsigned int index = 0; index < AES_BLOCK_SIZE * 3; index++) {
        printf("%02X", data[index]);
    }
    printf("\n");

    AES_EncryptInit(&ctx, key, iv);

    for (unsigned int offset = 0; offset < AES_BLOCK_SIZE * 3; offset += AES_BLOCK_SIZE) {
        AES_Encrypt(&ctx, data + offset, data + offset);
    }

    printf("Encrypted: ");
    for (unsigned int index = 0; index < AES_BLOCK_SIZE * 3; index++) {
        printf("%02X", data[index]);
    }
    printf("\n");

    AES_DecryptInit(&ctx, key, iv);

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

This AES-192 CBC implementation is provided under the [MIT License](./LICENSE).
