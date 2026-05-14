#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "process.h"
#include "filesystem.h"
#include "memory.h"
#include <iostream>
#include <queue>
#include <vector>
#include <map>
#include <chrono>
#include <thread>

// Scheduler (Zamanlayici) sinifi: Isletim sisteminin kalbidir.
class Scheduler {
private:
    std::queue<Process*>  ready_queue;
    std::vector<Process*> all_processes;

public:
    Scheduler();

    void add_process(Process* p);
    void run_fifo();

    // I/O-Aware + Page Fault-Aware FIFO Scheduler.
    // 'fs'           : FileSystem (dosya kilitleri icin)
    // 'io_requests'  : { pid -> dosya adi } (I/O yapacak processler)
    // 'mm'           : MemoryManager (page fault simulasyonu icin, nullptr = devre disi)
    // 'page_requests': { pid -> sayfa_id } (page fault tetiklenecek processler)
    // 'tick_ms'      : simulasyon tick suresi (ms)
    void run_with_io(FileSystem& fs,
                     const std::map<int, std::string>& io_requests,
                     int tick_ms = 500,
                     MemoryManager* mm = nullptr,
                     const std::map<int, int>& page_requests = {});

    // Round Robin Scheduler (I/O + Page Fault aware).
    void run_round_robin(FileSystem& fs,
                         const std::map<int, std::string>& io_requests,
                         int quantum_ms = 2000,
                         int tick_ms    = 500,
                         MemoryManager* mm = nullptr,
                         const std::map<int, int>& page_requests = {});
};

#endif // SCHEDULER_H