#include "torrent_tracker.h"
#include "bencode.h"
#include "byte_tools.h"
#include <cpr/cpr.h>
#include "iostream"

TorrentTracker::TorrentTracker(const std::string& url): url_(url) {}

void TorrentTracker::UpdatePeers(const TorrentFile &tf, std::string peerId, int port) {
    cpr::Response res = cpr::Get(
            cpr::Url{url_},
            cpr::Parameters{
                    {"info_hash",  tf.infoHash},
                    {"peer_id",    peerId},
                    {"port",       std::to_string(port)},
                    {"uploaded",   std::to_string(0)},
                    {"downloaded", std::to_string(0)},
                    {"left",       std::to_string(tf.length)},
                    {"compact",    std::to_string(1)}
            },
            cpr::Timeout{20000}
    );
    if (res.status_code != 200) {
        std::cerr << "Error: failed to connect to the tracker (status code " << res.status_code << ")"
                  << std::endl;
        return;
    }
    if (res.text.find("failure reason") != std::string::npos) {
        std::cerr << "Error. Server responded '" << res.text << "'" << std::endl;
        return;
    }


    size_t pivot = 0;
    std::shared_ptr<BencodeDictionary> dict = std::dynamic_pointer_cast<BencodeDictionary>(Bencode::parse_bencode(res.text.c_str(), res.text.size(), pivot));
    std::string peers = std::dynamic_pointer_cast<BencodeString>(dict->get("peers"))->get_str();
    std::vector<Peer> parsedPeers = Bencode::parsePeers(peers);
    peers_ = std::move(parsedPeers);
}


const std::vector<Peer> &TorrentTracker::GetPeers() const {
    return peers_;
}
