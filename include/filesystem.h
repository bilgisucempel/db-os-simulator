#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <string>
#include <map>
#include <queue>
#include "process.h"

// Bekleme kuyruğu girişi: hangi process, hangi I/O süresiyle bekliyor.
struct IOWaitEntry {
    Process* proc;
    int      io_ms; // Bu process dosyaya kavuşunca yapacağı I/O süresi (ms)
};

// Dosya Sistemi Yöneticisi (FileSystem)
// Process tabanlı I/O simülasyonu:
//   - std::thread / std::mutex kullanılmaz.
//   - I/O süresi process'in io_wait_ms alanına yazılır.
//   - Dış döngü (Scheduler) tick_io() ile süreci yönetir.
//   - Dosya meşgulken process BLOCKED + bekleme kuyruğuna alınır;
//     dosya boşalınca sıradaki process otomatik I/O'ya başlar.
class FileSystem {
private:
    // filename -> sahibi PID (-1: boşta)
    std::map<std::string, int> file_in_use;

    // filename -> dosyayı bekleyen process kuyruğu
    std::map<std::string, std::queue<IOWaitEntry>> wait_queues;

    // Dosyanın başka bir süreç tarafından kullanılıp kullanılmadığını kontrol eder.
    bool is_file_busy(const std::string& filename, int requesting_pid) const;

    // Dosyayı process'e ata ve I/O'ya başlat (içinden çağrılır).
    void assign_file(Process* p, const std::string& filename, int io_ms);

public:
    FileSystem();

    // Okuma I/O isteği (2000 ms).
    // Dosya meşgulse process BLOCKED + bekleme kuyruğuna alınır.
    void read_file(Process* p, const std::string& filename);

    // Yazma I/O isteği (3000 ms).
    // Dosya meşgulse process BLOCKED + bekleme kuyruğuna alınır.
    void write_file(Process* p, const std::string& filename);

    // I/O tamamlandığında çağrılır. Dosyayı serbest bırakır.
    // Bekleme kuyruğunda sıradaki process varsa ona dosyayı atar ve
    // o process'in io_wait_ms'ini set eder (BLOCKED olmaya devam eder,
    // ama artık tick_io() tarafından işlenir).
    // Döndürür: dosyaya kavuşan yeni process (yoksa nullptr).
    Process* release_file(const std::string& filename);
};

#endif // FILESYSTEM_H
