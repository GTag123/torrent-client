#include "byte_tools.h"
#include "piece.h"
#include <iostream>
#include <algorithm>
#include <utility>

namespace {
    constexpr size_t BLOCK_SIZE = 1 << 14;
}

Piece::Piece(size_t index, size_t length, std::string hash)
        : index_(index)
        , length_(length)
        , hash_(std::move(hash))
        , blocks_(length / BLOCK_SIZE + (length % BLOCK_SIZE == 0 ? 0 : 1)) {
    for (size_t i = 0; i < blocks_.size(); i++) {
        blocks_[i].piece = index_;
        blocks_[i].offset = i * BLOCK_SIZE;
        blocks_[i].length = std::min(BLOCK_SIZE, length_ - i * BLOCK_SIZE);
        blocks_[i].status = Block::Status::Missing;
    }
}
void Piece::Reset() {
    for (auto& block : blocks_) {
        block.status = Block::Status::Missing;
        block.data.clear();
    }
}

const std::string &Piece::GetHash() const {
    return hash_;
}

size_t Piece::GetIndex() const {
    return index_;
}

std::string Piece::GetData() const {
    std::string data;
    for (const auto& block : blocks_) {
        data += block.data;
    }
    return data;
}

std::string Piece::GetDataHash() const {
    return CalculateSHA1(GetData());
}

bool Piece::HashMatches() const {
    return GetDataHash() == hash_;
}

void Piece::SaveBlock(size_t blockOffset, std::string data) {
    if (blocks_[blockOffset / BLOCK_SIZE].status != Block::Status::Pending) {
        std::cerr << "------Block is not pending-------" << std::endl;
        throw std::runtime_error("Block is not pending");
    }
    blocks_[blockOffset / BLOCK_SIZE].data = std::move(data);
    blocks_[blockOffset / BLOCK_SIZE].status = Block::Status::Retrieved;
}

Block *Piece::FirstMissingBlock() {
    auto it = std::find_if(blocks_.begin(), blocks_.end(), [](const Block& block) {
        return block.status == Block::Status::Missing;
    });
    if (it == blocks_.end()) {
        return nullptr;
    }
    return &(*it);
}

bool Piece::AllBlocksRetrieved() const {
    return std::all_of(blocks_.begin(), blocks_.end(), [](const Block& block) {
        return block.status == Block::Status::Retrieved;
    });
}

