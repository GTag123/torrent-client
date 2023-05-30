#include "bencode.h"
#include <string>
#include <unordered_map>
#include <utility>
#include "byte_tools.h"

namespace Bencode {
    std::shared_ptr<BencodeCommon> parse_bencode(const char *buffer, size_t length, size_t &pos) {
        std::shared_ptr<BencodeCommon> result;
        char c = buffer[pos++];
        switch (c) {
            case 'i': {
                // Integer
                size_t end_pos = pos;
                while (end_pos < length && buffer[end_pos] != 'e') {
                    end_pos++;
                }
                if (end_pos >= length) {
                    throw std::runtime_error("Invalid bencode: unterminated integer");
                }
                std::string int_str(buffer + pos, end_pos - pos);
                result = std::make_shared<BencodeInteger>(stoll(int_str));
                pos = end_pos + 1;
                break;
            }
            case 'l': {
                // List
                result = std::make_shared<BencodeList>();
                while (buffer[pos] != 'e') {
                    std::dynamic_pointer_cast<BencodeList>(result)->list_val.push_back(parse_bencode(buffer, length, pos));
                }
                pos++;
                break;
            }
            case 'd': {
                // Dictionary
                result = std::make_shared<BencodeDictionary>();
                while (buffer[pos] != 'e') {
                    std::string key = std::dynamic_pointer_cast<BencodeString>(parse_bencode(buffer, length, pos))->str_val;
                    std::dynamic_pointer_cast<BencodeDictionary>(result)->dict_val[key] = parse_bencode(buffer, length, pos);
                }
                pos++;
                break;
            }
            default: {
                // String
                size_t end_pos = pos - 1;
                while (end_pos < length && buffer[end_pos] != ':') {
                    if (buffer[end_pos] == 'e') {
                        throw std::runtime_error("Invalid bencode: unexpected end of list or dictionary");
                    }
                    end_pos++;
                }
                if (end_pos >= length) {
                    throw std::runtime_error("Invalid bencode: unterminated string length");
                }
                std::string length_str(buffer + pos - 1, end_pos - pos + 1);
                size_t str_length = stoll(length_str);
                pos = end_pos + 1;
                result = std::make_shared<BencodeString>(std::string(buffer + pos, str_length));
                pos += str_length;
                break;
            }
        }
        return result;
    }

    std::string hashing(const BencodeDictionary& dict){
        if (dict.dict_val.find("info") == dict.dict_val.end()) {
            throw std::runtime_error("Invalid bencode: no info dictionary");
        }
        return CalculateSHA1(dict.get("info")->encode());
    }

    std::vector<Peer> parsePeers(const std::string& peersHash){
        std::vector<Peer> parsedPeers;
        for (int i = 0; i < peersHash.size(); i += 6) {
            std::string ip_string;
            for (int j = 0; j < 4; j++){
                ip_string += std::to_string(uint8_t(static_cast<unsigned char>(peersHash[i+j])));
                if (j < 3) {
                    ip_string += ".";
                }
            }
            auto port = (uint16_t(static_cast<unsigned char>(peersHash[i + 4])) << 8)
                        + uint16_t(static_cast<unsigned char>(peersHash[i + 5]));
            std::unordered_map<std::string, std::string> peerMap;
            parsedPeers.push_back({ip_string, port});
        }
        return parsedPeers;
    }
}
