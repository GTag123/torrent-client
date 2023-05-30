#pragma once

#include "torrent_file.h"
#include "piece.h"
#include <queue>
#include <string>
#include <unordered_set>
#include <mutex>
#include <fstream>
#include <filesystem>
#include <set>
/*
 * Хранилище информации о частях скачиваемого файла.
 * В этом классе отслеживается информация о том, какие части файла осталось скачать
 */
class PieceStorage {
public:
    PieceStorage(const TorrentFile& tf, const std::filesystem::path& outputDirectory, size_t count);

    /*
     * Отдает указатель на следующую часть файла, которую надо скачать
     */
    PiecePtr GetNextPieceToDownload();
    void AddPieceToQueue(const PiecePtr& piece);
    /*
     * Эта функция вызывается из PeerConnect, когда скачивание одной части файла завершено.
     * В рамках данного задания требуется очистить очередь частей для скачивания как только хотя бы одна часть будет успешно скачана.
     */
    void PieceProcessed(const PiecePtr& piece);

    /*
     * Остались ли нескачанные части файла?
     */
    bool QueueIsEmpty() const;

    /*
     * Сколько частей файла было сохранено на диск
     */
    size_t PiecesSavedToDiscCount() const;

    /*
     * Сколько частей файла всего
     */
    size_t TotalPiecesCount() const;

    /*
     * Закрыть поток вывода в файл
     */
    void CloseOutputFile();

    /*
     * Отдает список номеров частей файла, которые были сохранены на диск
     */
    const std::vector<size_t>& GetPiecesSavedToDiscIndices() const;

    /*
     * Сколько частей файла в данный момент скачивается
     */
    size_t PiecesInProgressCount() const;
    size_t GetQueueSize() const;
private:
    std::queue<PiecePtr> remainPieces_;
    const TorrentFile& tf_;
    std::filesystem::path outputDirectory_;
    std::ofstream out_;
    bool isOutputFileOpen_;
    size_t countOfPieces_;
//    std::set<size_t> piecesSavedToDiscIndicesSet_;
    std::vector<size_t> piecesSavedToDiscIndicesVector_;
    mutable std::mutex mutex_;
    /*
     * Сохраняет данную скачанную часть файла на диск.
     * Сохранение всех частей происходит в один выходной файл. Позиция записываемых данных зависит от индекса части
     * и размера частей. Данные, содержащиеся в части файла, должны быть записаны сразу в правильную позицию.
     */
    void SavePieceToDisk(const PiecePtr& piece);
};
