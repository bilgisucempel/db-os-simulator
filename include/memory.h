#ifndef MEMORY_H
#define MEMORY_H

#include <algorithm>
#include <iostream>
#include <vector>
#include "process.h"

// Page Fault I/O suresi (ms): Diskten RAM'e sayfa yuklemesi icin bekleme suresi.
// Gercek sistemlerde bu sure I/O'dan cok daha uzun olabilir.
static const int PAGE_FAULT_IO_MS = 3000;

// Bellek Yoneticisi Sinifi (Memory Manager) - LRU (Least Recently Used)
// Process tabanli Page Fault simulasyonu:
//   - Page Hit  : process devam eder.
//   - Page Fault: process BLOCKED + io_wait_ms = PAGE_FAULT_IO_MS atanir.
//     Scheduler tick_io() ile süreci takip eder; sure dolunca READY olur.
class MemoryManager {
private:
    int              ram_capacity;  // Maks. sayfa sayisi
    std::vector<int> loaded_pages;  // Sol=en eski, sag=en yeni (LRU)

public:
    MemoryManager(int capacity);

    // Sayfaya erisim:
    //  - Page Hit  : true  doner, process etkilenmez.
    //  - Page Fault: false doner, process BLOCKED yapilir (io_wait_ms set).
    //    p == nullptr ise eski davranis: sadece log, bloklama yok.
    bool access_page(int page_id, Process* p = nullptr);

    // RAM dolunca LRU sayfayi sil.
    void evict_page();
};

#endif // MEMORY_H