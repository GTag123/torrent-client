#pragma once

#include <vector>
#include <memory>
#include "peer.h"
#include "map"

struct BencodeCommon{
    virtual ~BencodeCommon() = default;
    [[nodiscard]] virtual std::string encode() const = 0;
};

struct BencodeInteger : public BencodeCommon{
    long long int_val;
    explicit BencodeInteger(long long int_val) : int_val(int_val) {}
    [[nodiscard]] std::string encode() const override {
        return "i" + std::to_string(int_val) + "e";
    }
    [[nodiscard]] long long get_int() const {
        return int_val;
    }
};

struct BencodeString : public BencodeCommon{
    std::string str_val;
    explicit BencodeString(std::string str_val) : str_val(std::move(str_val)) {}
    [[nodiscard]] std::string encode() const override {
        return std::to_string(str_val.length()) + ":" + str_val;
    }
    [[nodiscard]] std::string get_str() const {
        return str_val;
    }
};

struct BencodeList : public BencodeCommon{
    std::vector<std::shared_ptr<BencodeCommon>> list_val; // TODO: make iterator
    [[nodiscard]] std::string encode() const override {
        std::string result = "l";
        for (auto &item : list_val) {
            result += item->encode();
        }
        result += "e";
        return result;
    }
    [[nodiscard]] size_t get_size() const{
        return list_val.size();
    }
    [[nodiscard]] std::shared_ptr<BencodeCommon> get(size_t index) const {
        if (index >= list_val.size()) {
            throw std::runtime_error("Invalid bencode: index out of range");
        }
        return list_val[index];
    }
};

struct BencodeDictionary : public BencodeCommon{
    std::map<std::string, std::shared_ptr<BencodeCommon>> dict_val;
    [[nodiscard]] std::string encode() const override {
        std::string result = "d";
        for (auto &item : dict_val) {
            result += BencodeString(item.first).encode() + item.second->encode();
        }
        result += "e";
        return result;
    }
    [[nodiscard]] std::shared_ptr<BencodeCommon> get(const std::string &key) const {
        auto it = dict_val.find(key);
        if (it == dict_val.end()) {
            throw std::runtime_error("Invalid bencode: no key " + key);
        }
        return it->second;
    }
};


namespace Bencode {
    std::shared_ptr<BencodeCommon> parse_bencode(const char *buffer, size_t length, size_t &pos);
    std::string hashing(const BencodeDictionary& dict);
    std::vector<Peer> parsePeers(const std::string& peersHash);
}
