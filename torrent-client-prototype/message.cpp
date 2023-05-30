#include "message.h"

Message Message::Parse(const std::string &messageString) {
    MessageId id = static_cast<MessageId>(messageString[0]);
    size_t messageLength = messageString.size();
    std::string payload = messageString.substr(1, messageLength - 1);
    return {id, messageLength, payload};
}

Message Message::Init(MessageId id, const std::string &payload) {
    return {id, payload.size() + 1, payload};
}

std::string Message::ToString() const {
    return IntToBytes(messageLength) + static_cast<char>(id) + payload;
}

