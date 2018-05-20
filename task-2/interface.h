#include <cinttypes>

struct bytevector;

class SymmetricalCoder {
  public:
    bytevector encode(bytevector const &M, bytevector const &K,
                      bytevector const &IV) = 0;
    bytevector decode(bytevector const &M, bytevector const &K,
                      bytevector const &IV) = 0;
    uint32_t getKeySize() const = 0;
    uint32_t getIVSize() const = 0;
};

class AsymmetricalCoder {
  public:
    struct KeyPair
    {
        bytevector priv, pub;
    };
    KeyPair createPair(int bits) = 0;
    bytevector encode(bytevector const &M, bytevector const &K) = 0;
    bytevector decode(bytevector const &M, bytevector const &K) = 0;
};

class HashCoder {
  public:
    void init() = 0;
    void update(void *addr, size_t size) = 0;
    bytevector final() = 0;
};

class CryptoRandomGenerator {
    bytevector getRand(size_t bytes) = 0;
};

class Serializer {
    void add(any_type) = 0;
    void add(bytevector) = 0;
    bytevector getBody() = 0;
};

class Deserializer {
    type1 getType1() = 0;
    bytevector getBytevector() = 0;
};

bytevector ChunkAddUser(bytevector const &oldChunk,
                        bytevector const &pubKey);

bytevector ChunkCreate(bytevector const &chunkBody);

bytevector ChunkChange(bytevector const &oldChunk,
                       bytevector const &newBody);
