#include "torrent_file.h"
#include "bencode.h"
#include <vector>
#include <openssl/sha.h>
#include <fstream>
#include <variant>
#include <sstream>
#include <memory>

TorrentFile LoadTorrentFile(const std::string& filename){
    std::ifstream file(filename, std::ios::binary);
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string contents = buffer.str();
    file.close();

    TorrentFile torrent;
    size_t pivot = 0;
    std::shared_ptr<BencodeDictionary> dict = std::dynamic_pointer_cast<BencodeDictionary>(Bencode::parse_bencode(contents.c_str(), contents.size(), pivot));
    torrent.announce = std::dynamic_pointer_cast<BencodeString>(dict->get("announce"))->get_str();
    torrent.comment = std::dynamic_pointer_cast<BencodeString>(dict->get("comment"))->get_str();
    torrent.pieceLength = std::dynamic_pointer_cast<BencodeInteger>(std::dynamic_pointer_cast<BencodeDictionary>(dict->get("info"))->get("piece length"))->get_int();
    torrent.length = std::dynamic_pointer_cast<BencodeInteger>(std::dynamic_pointer_cast<BencodeDictionary>(dict->get("info"))->get("length"))->get_int();
    std::string piecesHash = std::dynamic_pointer_cast<BencodeString>(std::dynamic_pointer_cast<BencodeDictionary>(dict->get("info"))->get("pieces"))->get_str();
    for (int i = 0; i < piecesHash.length(); i += 20) {
        torrent.pieceHashes.push_back(piecesHash.substr(i, 20));
    }
    torrent.infoHash = Bencode::hashing(*dict);
    torrent.name = std::dynamic_pointer_cast<BencodeString>(std::dynamic_pointer_cast<BencodeDictionary>(dict->get("info"))->get("name"))->get_str();
    return torrent;
}