#pragma once

#include "tcp_connect.h"
#include "peer.h"
#include "torrent_file.h"
#include "piece_storage.h"
#include <atomic>
/*
 * Структура, хранящая информацию о доступности частей скачиваемого файла у данного пира
 */
class PeerPiecesAvailability {
public:
    PeerPiecesAvailability();

    /*
     * bitfield -- массив байтов, в котором i-й бит означает наличие или отсутствие i-й части файла у пира
     * https://wiki.theory.org/BitTorrentSpecification#bitfield:_.3Clen.3D0001.2BX.3E.3Cid.3D5.3E.3Cbitfield.3E
     */
    explicit PeerPiecesAvailability(std::string bitfield);

    /*
     * Если ли часть под номером `pieceIndex` у пира?
     */
    bool IsPieceAvailable(size_t pieceIndex) const;

    /*
     * Пометить часть под номером `pieceIndex` как доступную
     */
    void SetPieceAvailability(size_t pieceIndex);

    /*
     * Сколько бит хранится в bitfield'е
     */
    size_t Size() const;
private:
    std::string bitfield_;
};

/*
 * Класс, представляющий соединение с одним пиром.
 * С помощью него можно подключиться к пиру и обмениваться с ним сообщениями
 */
class PeerConnect {
public:
    PeerConnect(std::atomic<int>& peerscounter, const Peer& peer, const TorrentFile& tf, std::string selfPeerId, PieceStorage& pieceStorage);

    /*
     * Основная функция, в которой будет происходить цикл общения с пиром.
     * https://wiki.theory.org/BitTorrentSpecification#Messages
     */
    void Run();

    void Terminate();

    /*
     * Соединение не удалось установить или оно было разорвано в результате ошибки.
     */
    bool Failed() const;
    const Peer& peerinfo;
private:
    std::atomic<int>& peerscounter_;
    const TorrentFile& tf_;
    TcpConnect socket_;  // tcp-соединение с пиром
    const std::string selfPeerId_;  // наш id, которым представляется наш клиент
    std::string peerId_;  // id пира, с которым мы общаемся в текущем соединении
    PeerPiecesAvailability piecesAvailability_;
    bool terminated_;  // флаг, необходимый для завершения цикла общения с пиром
    bool choked_;  // https://wiki.theory.org/BitTorrentSpecification#Overview
    PiecePtr pieceInProgress_;
    PieceStorage& pieceStorage_;
    bool pendingBlock_;  // уже послали запрос на скачивание части файла и ждем ответ
    bool failed_;  // соединение не удалось установить или оно было разорвано в результате ошибки

    /*
     * Функция производит handshake.
     * - Подключиться к пиру по протоколу TCP
     * - Отправить пиру сообщение handshake
     * - Проверить правильность ответа пира
     * https://wiki.theory.org/BitTorrentSpecification#Handshake
     */
    void PerformHandshake();

    /*
     * - Провести handshake
     * - Получить bitfield с информацией о наличии у пира различных частей файла
     * - Сообщить пиру, что мы готовы получать от него данные (отправить interested)
     * Возвращает true, если все три этапа прошли без ошибок
     */
    bool EstablishConnection();

    /*
     * Функция читает из сокета bitfield с информацией о наличии у пира различных частей файла.
     * Полученную информацию надо сохранить в поле `piecesAvailability_`.
     * Также надо учесть, что сообщение тип Bitfield является опциональным, то есть пиры необязательно будут слать его.
     * Вместо этого они могут сразу прислать сообщение Unchoke, поэтому надо быть готовым обработать его в этой функции.
     * Обработка сообщения Unchoke заключается в выставлении флага `choked_` в значение `false`
     */
    void ReceiveBitfield();

    /*
     * Функция посылает пиру сообщение типа interested
     */
    void SendInterested();

    /*
     * Функция отправляет пиру сообщение типа request. Это сообщение обозначает запрос части файла у пира.
     * За одно сообщение запрашивается не часть целиком, а блок данных размером 2^14 байт или меньше.
     * Если в данный момент мы не знаем, какую часть файла надо запросить у пира, то надо получить эту информацию у
     * PieceStorage
     */
    void RequestPiece();

    /*
     * Основной цикл общения с пиром. Здесь мы ждем следующее сообщение от пира и обрабатываем его.
     * Также, если мы не ждем в данный момент от пира содержимого части файла, то надо отправить соответствующий запрос
     */
    void MainLoop();
};
