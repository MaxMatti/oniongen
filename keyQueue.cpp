#include <cstdint>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>


#include "cryptopp.h
#include "defines.h"

void bla()
{
    while(true)
    {
        /** @todo set 200 to max number of elements created per cyclus */
        unsigned int maxElements = 200;

        /** @todo set 160 to constant for pubkey size */
        /* 4 Byte + 160+72Byte - 2 bits */
        size_t pubKeySize = sizeof(std::uint32_t) + (((160+72)* sizeof(char))& 0xFFFFFFC0);

        /** @todo 4+20 */
        size_t hashSize = sizeof(uint32_t) + (SHA1_LENGTH * sizeof(char));

        char *pubKeys = (char *) malloc(maxElements*pubKeySize);
        /** @todo */
        char *privKeys = (char *) malloc(maxElements*(4*sizeof(char) + 160* sizeof(char)));
        char **keyPointer = (char **) malloc(maxElements * sizeof(char *));

        /** @todo set 100 dynamicaly */
        char *matches = (char *) malloc(100 * hashSize);

        
        /** @todo cuda malloc */
        
        /* generate keys */
        crypto::Rng rng;
        crypto::PrivateKey rsa;
        std::string privKey;
        std::string pubKey;
        size_t privPos = 0;
        for (unsigned int i = 0; i < maxElements; i++) {
            rsa = crypto::generateKey(rng);
            privKey = crypto::getPrivateKey(rsa);
            pubKey = crypto::getPublicKey(rsa);

            pubKeys[i*(pubKeySize)] = (std::uint32_t) i;
            memcpy(&(pubKeys[i*(pubKeySize) + sizeof(std::uint32_t)]), pubKey.c_str(), pubKey.length());

            keyPointer[i] = &(privKeys[privPos]);
            memcpy(&(privKeys[privPos]), privKey.c_str(), privKey.length());
            privPos += privKey.length()*sizeof(char);
        }

        /** @todo copy to cuda */

        /** @todo copy matches from cuda */

        /* output */
        uint32_t privateIndex;
        for (unsigned i = 0; i < 100; i++)
        {
            /** @todo break if no new entries */
            std::cout << "HASH " << matches[i*(hashSize+sizeof(uint32_t))] << std::endl;
            privateIndex = matches[i*hashSize];
            std::cout << "Private Key" << *keyPointer[privateIndex] << std::endl; /** todo grenze? */
            std::cout << std::endl;
        }
    }
}

int main()
{
    bla();
    return 0;
}