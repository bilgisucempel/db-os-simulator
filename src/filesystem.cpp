#include "../include/filesystem.h"
#include <iostream>

FileSystem::FileSystem() {}

bool FileSystem::is_file_busy(const std::string& filename, int requesting_pid) const {
    auto it = file_in_use.find(filename);
    if (it == file_in_use.end() || it->second == -1) return false;
    return it->second != requesting_pid;
}

void FileSystem::assign_file(Process* p, const std::string& filename, int io_ms) {
    file_in_use[filename] = p->process_id;
    p->io_wait_ms = io_ms;
    p->state      = ProcessState::BLOCKED;
    std::cout << "[FileSystem] ATANDI: '" << filename << "' -> Surec " << p->name
              << " (PID: " << p->process_id << ")  |  io_wait_ms = " << io_ms << " ms" << std::endl;
}

void FileSystem::read_file(Process* p, const std::string& filename) {
    std::cout << "[FileSystem] " << p->name << " (PID: " << p->process_id
              << ") '" << filename << "' dosyasini OKUMAK istiyor." << std::endl;

    if (is_file_busy(filename, p->process_id)) {
        // Dosya mesgul: process'i bekleme kuyruğuna al, BLOCKED yap (io_wait_ms=0).
        // Dosya boşalınca release_file() bu process'i otomatik I/O'ya alır.
        std::cout << "[FileSystem] BEKLEMEYE ALINDI: '" << filename << "' mesgul (PID "
                  << file_in_use.at(filename) << " kullaniyor). "
                  << p->name << " BLOCKED (dosya kuyruğu)." << std::endl;
        wait_queues[filename].push({p, 4000});
        p->io_wait_ms = 0;          // 0: tick_io calismaz; release_file uyandıracak
        p->state      = ProcessState::BLOCKED;
        return;
    }

    // Dosya bosta: hemen I/O baslat
    assign_file(p, filename, 4000);
    std::cout << "[FileSystem] I/O BASLADI (okuma): " << p->name
              << " BLOCKED durumuna alindi." << std::endl;
    std::cout << "-----------------------------------" << std::endl;
}

void FileSystem::write_file(Process* p, const std::string& filename) {
    std::cout << "[FileSystem] " << p->name << " (PID: " << p->process_id
              << ") '" << filename << "' dosyasina YAZMAK istiyor." << std::endl;

    if (is_file_busy(filename, p->process_id)) {
        std::cout << "[FileSystem] BEKLEMEYE ALINDI: '" << filename << "' mesgul (PID "
                  << file_in_use.at(filename) << " kullaniyor). "
                  << p->name << " BLOCKED (dosya kuyruğu)." << std::endl;
        wait_queues[filename].push({p, 6000});
        p->io_wait_ms = 0;
        p->state      = ProcessState::BLOCKED;
        return;
    }

    assign_file(p, filename, 6000);
    std::cout << "[FileSystem] I/O BASLADI (yazma): " << p->name
              << " BLOCKED durumuna alindi." << std::endl;
    std::cout << "-----------------------------------" << std::endl;
}

Process* FileSystem::release_file(const std::string& filename) {
    auto it = file_in_use.find(filename);
    if (it == file_in_use.end() || it->second == -1) return nullptr;

    std::cout << "[FileSystem] SERBEST: '" << filename << "' (PID "
              << it->second << " birakti)." << std::endl;
    it->second = -1;

    // Bekleme kuyruğunda bekleyen var mi?
    auto& wq = wait_queues[filename];
    if (!wq.empty()) {
        IOWaitEntry next = wq.front();
        wq.pop();
        // Siradaki process'e dosyayı ata ve I/O'yu baslat
        std::cout << "[FileSystem] KUYRUK: " << next.proc->name
                  << " (PID: " << next.proc->process_id
                  << ") sirada bekliyordu. Dosya ataniyor, I/O basliyor ("
                  << next.io_ms << " ms)." << std::endl;
        assign_file(next.proc, filename, next.io_ms);
        std::cout << "-----------------------------------" << std::endl;
        return next.proc;   // Scheduler bu process'i takip listesine ekleyecek
    }

    std::cout << "[FileSystem] '" << filename << "' artik tamamen bosta." << std::endl;
    std::cout << "-----------------------------------" << std::endl;
    return nullptr;
}
