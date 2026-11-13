# AES-128-CBC Encryption and Decryption

This repository provides a simple implementation of AES-128-CBC encryption and decryption in C using the `AES_128_CBC.h` header file. AES-128-CBC is a widely used symmetric encryption algorithm that operates on fixed-size blocks of data.

## Table of Contents

- [Introduction](#introduction)
- [Features](#features)
- [Usage](#usage)
  - [Single Block Example](#single-block-example)
  - [Multiple Blocks Example](#multiple-blocks-example)
- [Building and Integration](#building-and-integration)
- [Known Limitations](#known-limitations)
- [Contributing](#contributing)
- [License](#license)

## Introduction

AES-128-CBC operates on 128-bit blocks and supports key sizes of 128 bits. Cipher Block Chaining (CBC) mode introduces an Initialization Vector (IV) to each block, making each encryption operation dependent on the previous one. This implementation includes functions for encryption and decryption using the CBC mode.

## Features

- Simple AES-128-CBC encryption and decryption.
- Header-only implementation for easy integration.

## Usage

### Single Block Example

```c
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "AES_128_CBC.h"

void output(const char* title, uint8_t *data, uint32_t size) {
    printf("%s", title);
    for (uint8_t index = 0; index < size; index++) {
        printf("%02x", data[index]);
    }
    printf("\n");
}

int main() {
    // Example Key (128 bits / 16 bytes)
    uint8_t key[16] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x97, 0x99, 0x89, 0xcf, 0xab, 0x12};

    // Example IV (128 bits / 16 bytes)
    uint8_t iv[16] = {0x0f, 0x47, 0x0e, 0x7f, 0x75, 0x9c, 0x47, 0x0f, 0x42, 0xc6, 0xd3, 0x9c, 0xbc, 0x8e, 0x23, 0x25};

    // Example Data (16 bytes)
    uint8_t data[16] = {0x54, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x61, 0x20, 0x73, 0x69, 0x6d, 0x70, 0x6c, 0x65};

    output("original: 0x", data, 16);

    AES_CTX ctx;
    AES_EncryptInit(&ctx, key, iv);
    AES_Encrypt(&ctx, data, data);

    output("\nencrypted: 0x", data, 16);

    // Decryption Example
    AES_DecryptInit(&ctx, key, iv);
    AES_Decrypt(&ctx, data, data);

    output("\ndecrypted: 0x", data, 16);

    return 0;
}
```

### Multiple Blocks Example

```c
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "AES_128_CBC.h"

void output(const char* title, uint8_t *data, uint32_t size) {
    printf("%s", title);
    for (uint8_t index = 0; index < size; index++) {
        printf("%02x", data[index]);
    }
    printf("\n");
}

int main() {
    // Example Key (128 bits / 16 bytes)
    uint8_t key[16] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x97, 0x99, 0x89, 0xcf, 0xab, 0x12};

    // Example IV (128 bits / 16 bytes)
    uint8_t iv[16] = {0x0f, 0x47, 0x0e, 0x7f, 0x75, 0x9c, 0x47, 0x0f, 0x42, 0xc6, 0xd3, 0x9c, 0xbc, 0x8e, 0x23, 0x25};

    // Example Data (32 bytes)
    uint8_t data[32] = {0x54, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x61, 0x20, 0x73, 0x69, 0x6d, 0x70, 0x6c, 0x65,
                        0x20, 0x41, 0x45, 0x53, 0x2d, 0x31, 0x32, 0x38, 0x2d, 0x43, 0x42, 0x43, 0x20, 0x65, 0x78, 0x61};

    output("original block 1: 0x", data, 16);
    output("original block 2: 0x", data + 16, 16);

    AES_CTX ctx;
    AES_EncryptInit(&ctx, key, iv);

    for (unsigned int offset = 0; offset < 32; offset += 16) {
        AES_Encrypt(&ctx, data + offset, data + offset);
    }



 output("\nencrypted block 1: 0x", data, 16);
    output("encrypted block 2: 0x", data + 16, 16);

    // Decryption Example
    AES_DecryptInit(&ctx, key, iv);

    for (unsigned int offset = 0; offset < 32; offset += 16) {
        AES_Decrypt(&ctx, data + offset, data + offset);
    }

    output("\ndecrypted block 1: 0x", data, 16);
    output("decrypted block 2: 0x", data + 16, 16);

    return 0;
}
```

## Building and Integration

This implementation is header-only, making it easy to integrate into your projects. Simply include the `AES_128_CBC.h` header file in your source code and link against the necessary libraries.

## Known Limitations

- **No Padding Support**: Ensure that your data length is a multiple of 16 bytes, as the algorithm processes data in 16-byte blocks. Padding is not supported.

## Contributing

Feel free to contribute to the project by opening issues or pull requests.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
