#include "interface.h"

class Message {
    bytearray encrypted_data;
    bytearray pub_encrypted_sym_key;
}

class Chunk {

    using UserPublicKey = bytevector;

    Message(bytevector data, UserPublicKey user_pk) {
        auto symmetric_key = gen_.getRand(32);
        encrypted_data_ = sym_coder_.encode(data, symmetric_key);

        hasher = HashCoder();
        hasher_.update(&data, data.size());
        encrypted_data_hash_ = sym_coder_.encode(hasher_.final(), symmetric_key);

        encrypted_keys_.insert({user_pk,
                                AsymmetricalCoder.encode(symmetric_key)});
    }

    bool AddUser(UserPublicKey owner_pk, UserPublicKey new_pk) {
        if (!encrypted_keys_.count(owner_pk)) {
            return false;
        }
        auto symmetric_key = EndToEndGet(owner_pk,
                                         encrypted_keys_[owner_pk]);
        encrypted_keys_.insert({new_pk,
                                AsymmetricalCoder.encode(symmetric_key)});
    }

    void Change(bytevector new_data, UserPublicKey user_pk) {
        auto symmetric_key = EndToEndGet(owner_pk,
                                         encrypted_keys_[owner_pk]);
        auto data = sym_coder_.decode(encrypted_data_);
        auto hash = sym_coder_.decode(encrypted_data_hash_);
        if (HashCoder(data) != hash) {
            return;
        }

        encrypted_data_ = sym_coder_.encode(new_data, symmetric_key);

        hasher = HashCoder();
        hasher_.update(&new_data, new_data.size());
        encrypted_data_hash_ = sym_coder_.encode(hasher_.final(), symmetric_key);
    }

    void Get(UserPublicKey user_pk) {
        return Message(encrypted_data_, encrypted_keys_[user_pk]);
    }

private:
    bytearray encrypted_data_;
    size_t encrypted_data_hash_;
    std::unordered_map<UserPublicKey, bytevector> encrypted_keys_;
    SymmetricalCoder sym_coder_{};
    AsymmetricalCoder asym_coder_{};
    CryptoRandomGenerator gen_{};
    HashCoder hasher_;
};
