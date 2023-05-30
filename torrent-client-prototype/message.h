#pragma once

#include <cstdint>
#include <string>
#include "byte_tools.h"

enum class MessageId : uint8_t {
    Choke = 0,
    Unchoke,
    Interested,
    NotInterested,
    Have,
    BitField,
    Request,
    Piece,
    Cancel,
    Port,
    KeepAlive,
};

struct Message {
    MessageId id;
    size_t messageLength;
    std::string payload;

    /*
     * Выделяем тип сообщения и длину и создаем объект типа Message.
     * Подразумевается, что здесь в качестве `messageString` будет приниматься строка, прочитанная из TCP-сокета
     */
    static Message Parse(const std::string& messageString);

    /*
     * Создаем сообщение с заданным типом и содержимым. Длина вычисляется автоматически
     */
    static Message Init(MessageId id, const std::string& payload);

    /*
     * Формируем строку с сообщением, которую можно будет послать пиру в соответствии с протоколом.
     * Получается строка вида "<1 + payload length><message id><payload>"
     * Секция с длиной сообщения занимает 4 байта и представляет собой целое число в формате big-endian
     * id сообщения занимает 1 байт и может принимать значения от 0 до 9 включительно
     */
    std::string ToString() const;
};
