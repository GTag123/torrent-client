### Прототип торрент клиента
#### Запуск
1. `$ sudo apt-get install cmake libcurl4-openssl-dev libssl-dev`
2. `cd torrent-client-prototype`
3. `$ make final`
4. `$ cd result`
5. `$ ./torrent-client-prototype -d <путь к директории для сохранения скачанного файла> -p <сколько процентов от файла надо скачать> <путь к torrent-файлу>`
- Важно обновлять tag cpr-а torrent-client-prototype/CMakeLists.txt `FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
  GIT_TAG <commit hash>)`
- В основной директории предоставлен тестовый скрипт для клиента
#### TODO
- Сделать постоянное переподключение к пирам (иначе имеем низкую скорость скачивания)
- Сделать tread-safe singleton логгер вместо cout
