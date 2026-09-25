# AES-256 ECB Encryption Implementation in C

This repository showcases a concise implementation of the AES-256 ECB (Electronic Codebook) encryption algorithm in C, renowned for its heightened security.

## Features

- **AES-256 ECB:** Implements secure AES-256 ECB encryption mode.
- **Fixed Block Size:** Enforces a 16-byte fixed block size without padding.
- **Compatibility:** Encrypted data is interoperable with any AES-256 ECB decryption.

## Usage

Include "AES_256_ECB.h" before using AES-256 ECB functions.

```c
#include "AES_256_ECB.h"
```

### AES Context Initialization

```c
AES_CTX ctx;
```

### Function: `AES_EncryptInit`

Initialize the AES context for encryption.

```c
void AES_EncryptInit(AES_CTX *ctx, const unsigned char *key);
```

- `ctx`: AES context structure.
- `key`: Encryption key (32 bytes for AES-256).

### Function: `AES_Encrypt`

Encrypt a single 16-byte block of data.

```c
void AES_Encrypt(AES_CTX *ctx, const unsigned char *plaintext, unsigned char *ciphertext);
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
void AES_DecryptInit(AES_CTX *ctx, const unsigned char *key);
```

- `ctx`: AES context structure.
- `key`: Decryption key (32 bytes for AES-256).

### Function: `AES_Decrypt`

Decrypt a single 16-byte block of data.

```c
void AES_Decrypt(AES_CTX *ctx, const unsigned char *ciphertext, unsigned char *plaintext);
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
#include "AES_256_ECB.h"

void output(const char *title, const unsigned char *data, unsigned int size) {
    printf("%s", title);
    for (unsigned int index = 0; index < size; index++) {
        printf("%02X", data[index]);
    }
    printf("\n");
}

int main() {
    AES_CTX ctx;
    unsigned char key[AES_KEY_SIZE];
    unsigned char data[AES_BLOCK_SIZE];

    // Initialization and usage example
    memset(key, 0x79, AES_KEY_SIZE);
    memset(data, 0x79, AES_BLOCK_SIZE);

    output("Original: 0x", data, 16);

    AES_EncryptInit(&ctx, key);
    AES_Encrypt(&ctx, data, data);

    output("\nEncrypted: 0x", data, 16);

    AES_DecryptInit(&ctx, key);
    AES_Decrypt(&ctx, data, data);

    output("\nDecrypted: 0x", data, 16);

    AES_CTX_Free(&ctx);
    return 0;
}
```

## Contributions

Contributions and feedback are welcome! Open issues or submit pull requests.

## License

This AES-256 ECB implementation is provided under the [MIT License](./LICENSE).

## Other AES Implementations

- [AES-256 CBC by halloweeks](https://github.com/halloweeks/AES-256-CBC)
- [AES-128 ECB by halloweeks](https://github.com/halloweeks/AES-128-ECB)
