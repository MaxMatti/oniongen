#include <cryptopp/rsa.h>
#include <cryptopp/osrng.h>
#include <cryptopp/base64.h>
#include <cryptopp/files.h>

#include "cryptopp.h"
#include "defines.h"

using namespace CryptoPP;
using namespace std;
namespace crypto {

    /**
     * @brief Generate new Key
     * @param rng Random pool
     * @return Key object
     */
    PrivateKey generateKey(Rng &rng) {
        RSA::PrivateKey params;
        params.GenerateRandomWithKeySize(rng, RSA_KEYS_BITLEN);
        return params;
    }

    /**
     * @brief Get DER encoded private key's string
     * @param params Key
     * @return DER encoded string
     */
    std::string getPrivateKey(RSA::PrivateKey &params) {
        string rsaPriv;
        StringSink stringSink(rsaPriv);
        params.DEREncode(stringSink);

        return rsaPriv;
    }

    /**
     * @brief Get DER encoded public key's string
     * @param params Key
     * @return DER encoded string
     */
    std::string getPublicKey(RSA::PrivateKey &params) {
        RSA::PublicKey rsaPublic(params);

        string rsaPub;
        StringSink stringSink(rsaPub);
        rsaPublic.DEREncode(stringSink);

        return rsaPub;
    }
}
/*
int main() {

    crypto::Rng rng;
    crypto::PrivateKey rsa = crypto::generateKey(rng);
    cout << crypto::getPrivateKey(rsa) << endl;
    cout << crypto::getPublicKey(rsa) << endl;
    cout << crypto::getPrivateKey(rsa).length() <<endl;

    return 0;
}*/