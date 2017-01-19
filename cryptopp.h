#include <cryptopp/rsa.h>
#include <cryptopp/osrng.h>
#include <string>

using namespace CryptoPP;
using namespace std;

namespace crypto {

    typedef AutoSeededRandomPool Rng;
    typedef RSA::PrivateKey PrivateKey;

    PrivateKey generateKey(Rng &rng);

    std::string getPrivateKey(RSA::PrivateKey &params);

    std::string getPublicKey(RSA::PrivateKey &params);
}