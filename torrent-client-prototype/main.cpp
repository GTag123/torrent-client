#include "torrent_tracker.h"
#include "piece_storage.h"
#include "peer_connect.h"
#include "byte_tools.h"
#include <iostream>
#include <filesystem>
#include <random>
#include <thread>
#include <algorithm>
#include "mutex"
#include "atomic"
namespace fs = std::filesystem;
std::atomic<int> cntStart = 0;
std::atomic<int> cntEnd = 0;
std::atomic<int> peerConnectsRemain = 0;
std::string RandomString(size_t length) {
    std::random_device random;
    std::string result;
    result.reserve(length);
    for (size_t i = 0; i < length; ++i) {
        result.push_back(random() % ('Z' - 'A') + 'A');
    }
    return result;
}

const std::string PeerId = "TESTAPPDONTWORRY" + RandomString(4);

size_t PiecesToDownload = 20;
size_t percents = 0;
std::filesystem::path outputDirectory;

bool RunDownloadMultithread(PieceStorage& pieces, const TorrentFile& torrentFile, const std::string& ourId, const TorrentTracker& tracker) {
    using namespace std::chrono_literals;

    std::vector<std::thread> peerThreads;
    std::vector<PeerConnect> peerConnections;

    for (const Peer& peer : tracker.GetPeers()) {
        peerConnections.emplace_back(peerConnectsRemain, peer, torrentFile, ourId, pieces);
    }
    std::cout << "Created " << peerConnections.size() << " peer connections" << std::endl;
    for (PeerConnect& peerConnect : peerConnections) {
        peerThreads.emplace_back(
                [&peerConnect] () {
                    bool tryAgain = true;
                    int attempts = 0;
                    do {
                        try {
                            ++attempts;
                            std::cout << std::endl << "Ip: " << peerConnect.peerinfo.ip << " Port: " << peerConnect.peerinfo.port << std::endl;
                            cntStart++;
                            std::cout << std::endl << "Run stated: " << cntStart << std::endl;
                            peerConnect.Run();
                        } catch (const std::runtime_error& e) {
                            std::cerr << "Runtime error: " << e.what() << std::endl;
                        } catch (const std::exception& e) {
                            std::cerr << "Exception: " << e.what() << std::endl;
                        } catch (...) {
                            std::cerr << "Unknown error" << std::endl;
                        }
                        cntEnd++;
                        std::cout << "Run ended: " << cntEnd << std::endl << std::endl;
                        tryAgain = peerConnect.Failed() && attempts < 3;
                    } while (tryAgain);
                }
        );
    }

    std::this_thread::sleep_for(10s);
    while (pieces.PiecesSavedToDiscCount() < PiecesToDownload) {
        // TODO: постоянный реконнект к пирам для максимальной скорости загрузки
        if (pieces.PiecesInProgressCount() == 0) {
            std::cerr << "Want to download more pieces but all peer connections are not working. Let's request new peers" << std::endl;
            int termcount = 0;
            for (PeerConnect& peerConnect : peerConnections) {
                termcount++;
                std::cout << "Terminate count: " << termcount << std::endl;
                peerConnect.Terminate();
            }
            for (std::thread& thread : peerThreads) {
                thread.join();
            }
            return true;
        }
        std::cout << "\n-----------------------\n";
        if ((pieces.PiecesInProgressCount() <= 2 && pieces.GetQueueSize() > 10)
            || (pieces.PiecesInProgressCount() <= 3 && pieces.GetQueueSize() > 30)) {
            std::cout << "Rerun started" << std::endl;
            for (auto& peer: peerConnections){
                peer.ReRun();
            }
        }
        std::cout << "InProgress: " << pieces.PiecesInProgressCount() << "\n";
        std::cout << "ToDownloadTotal: " << PiecesToDownload << "\n";
        std::cout << "SavedToDisk: " << pieces.PiecesSavedToDiscCount() << "\n";
        std::cout << "QueueSize: " << pieces.GetQueueSize() << "\n";
        std::cout << "-----------------------\n" << std::endl;
        std::this_thread::sleep_for(2s);
    }
    std::cout << "Pieces downloaded finished" << std::endl << std::endl;
    for (PeerConnect& peerConnect : peerConnections) {
        peerConnect.Terminate();
    }

    for (std::thread& thread : peerThreads) {
        thread.join();
    }

    return false;
}

void DownloadTorrentFile(const TorrentFile& torrentFile, PieceStorage& pieces, const std::string& ourId) {
    std::cout << "Connecting to tracker " << torrentFile.announce << std::endl;
    TorrentTracker tracker(torrentFile.announce);
    bool requestMorePeers = false;
    do {
        tracker.UpdatePeers(torrentFile, ourId, 12345);

        if (tracker.GetPeers().empty()) {
            std::cerr << "No peers found. Cannot download a file" << std::endl;
        }

        std::cout << "Found " << tracker.GetPeers().size() << " peers" << std::endl;
        for (const Peer& peer : tracker.GetPeers()) {
            std::cout << "Found peer " << peer.ip << ":" << peer.port << std::endl;
        }

        requestMorePeers = RunDownloadMultithread(pieces, torrentFile, ourId, tracker);
    } while (requestMorePeers);
}

void PrepareTorrentFile(const fs::path& file) {
    TorrentFile torrentFile;
    try {
        torrentFile = LoadTorrentFile(file);
        PiecesToDownload = (((torrentFile.length / torrentFile.pieceLength) * percents) / 100);
        std::cout << "Loaded torrent file " << file << ". " << torrentFile.comment << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cerr << e.what() << std::endl;
        return;
    }
    PieceStorage pieces(torrentFile, outputDirectory, PiecesToDownload);

    DownloadTorrentFile(torrentFile, pieces, PeerId);
    std::cout << "Downloaded" << std::endl;
}
int main(int argc, char* argv[]) {
//    std::string path1;
//    int number;
//    std::string path2;
//
//    if (argc < 6) {
//        std::cout << "Not enough arguments" << std::endl;
//        return 1;
//    }
//
//    for (int i = 1; i < argc; ++i) {
//        std::string arg = argv[i];
//        if (arg == "-d") {
//            if (i + 1 < argc) {
//                path1 = argv[i + 1];
//                i++;
//            } else {
//                std::cout << "Incorrect arguments" << std::endl;
//                return 1;
//            }
//        } else if (arg == "-p") {
//            if (i + 1 < argc) {
//                try {
//                    number = std::stoi(argv[i + 1]);
//                } catch (const std::exception& e) {
//                    std::cout << "Incorrect arguments" << std::endl;
//                    return 1;
//                }
//                i++;
//            } else {
//                std::cout << "Incorrect arguments!" << std::endl;
//                return 1;
//            }
//        } else {
//            path2 = arg;
//        }
//    }
//
//    std::cout << "Path1: " << path1 << std::endl;
//    std::cout << "Percents: " << number << std::endl;
//    std::cout << "Path2: " << path2 << std::endl;
    percents = 100;
    outputDirectory = "../result";
    PrepareTorrentFile("../../test-resources/debian-9.3.0-ppc64el-netinst.torrent");
//    percents = number;
//    outputDirectory = path1;
//    PrepareTorrentFile(path2);
    return 0;
}
