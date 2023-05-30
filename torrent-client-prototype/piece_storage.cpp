#include "piece_storage.h"
#include <iostream>

PieceStorage::PieceStorage(const TorrentFile& tf, const std::filesystem::path& outputDirectory, size_t count) :
    tf_(tf),
    outputDirectory_(outputDirectory),
    out_(outputDirectory_ / tf_.name, std::ios::binary | std::ios::out),
    isOutputFileOpen_(true),
    countOfPieces_(count) {
    out_.seekp(tf.length - 1);
    out_.write("\0", 1);
    std::cout << "OPENED STREAM: " << out_.is_open() << std::endl;
    std::cout << "NAME IS " << tf_.name << std::endl;
    for (size_t i = 0; i < countOfPieces_; ++i) {
        size_t length = (i == tf.length / tf.pieceLength - 1) ? tf.length % tf.pieceLength : tf.pieceLength;
        remainPieces_.push(std::make_shared<Piece>(i, length, tf.pieceHashes[i]));
    }
    std::cout << "QUEUE SIZE: " << remainPieces_.size()  << " " << count << std::endl;
}


PiecePtr PieceStorage::GetNextPieceToDownload() {
    std::lock_guard lock(mutex_);
    if (remainPieces_.empty()) {
        return nullptr;
    }
    auto piece = remainPieces_.front();
    remainPieces_.pop();
    return piece;
}
void PieceStorage::AddPieceToQueue(const PiecePtr &piece) {
    std::lock_guard lock(mutex_);
    piece->Reset();
    remainPieces_.push(piece);
}
void PieceStorage::PieceProcessed(const PiecePtr& piece) {
    // хз, что будет если пир постоянно будет отправлять бракованный кусок,peer_connect будет же крутиться в бесконечном цикле
    std::lock_guard lock(mutex_);
    if (!piece->HashMatches()) {
        piece->Reset();
        std::cerr << "Piece " << piece->GetIndex() << " hash doesn't match" << std::endl;
        remainPieces_.push(piece);
        return;
    }
    SavePieceToDisk(piece);
}

bool PieceStorage::QueueIsEmpty() const {
    std::lock_guard lock(mutex_);
    return remainPieces_.empty();
}

size_t PieceStorage::TotalPiecesCount() const {
    return tf_.length / tf_.pieceLength;
}

size_t PieceStorage::PiecesInProgressCount() const {
    std::lock_guard lock(mutex_);
    return countOfPieces_ - remainPieces_.size() - piecesSavedToDiscIndicesVector_.size();
}

void PieceStorage::CloseOutputFile() {
    std::lock_guard lock(mutex_);
    if (!isOutputFileOpen_) {
        std::cerr << "Output file is already closed" << std::endl;
        return;
    }
    out_.close();
    isOutputFileOpen_ = false;
}

const std::vector<size_t>& PieceStorage::GetPiecesSavedToDiscIndices() const { // проблемный метод
    std::lock_guard lock(mutex_);
    return piecesSavedToDiscIndicesVector_;
}

size_t PieceStorage::PiecesSavedToDiscCount() const {
    std::lock_guard lock(mutex_);
    return piecesSavedToDiscIndicesVector_.size();
}
size_t PieceStorage::GetQueueSize() const {
    std::lock_guard lock(mutex_);
    return remainPieces_.size();
}

void PieceStorage::SavePieceToDisk(const PiecePtr &piece) {
    if (!isOutputFileOpen_) {
        std::cerr << "Output file is already closed" << std::endl;
        return;
    }
    out_.seekp(piece->GetIndex() * tf_.pieceLength);
    out_.write(piece->GetData().data(), piece->GetData().size());
    piecesSavedToDiscIndicesVector_.push_back(piece->GetIndex());
    std::cout << "Saved to Disk piece " << piece->GetIndex() << std::endl;
//    std::cout << "Piece data length: " << piece->GetData().size() << std::endl;
    std::cout << "PieceQueue size: " << remainPieces_.size() << std::endl;
}
