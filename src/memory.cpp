#include "../include/memory.h"

MemoryManager::MemoryManager(int capacity) : ram_capacity(capacity) {
    std::cout << "[Memory] Bellek Yoneticisi baslatildi. Algoritma: LRU. Kapasite: "
              << ram_capacity << " sayfa." << std::endl;
}

bool MemoryManager::access_page(int page_id, Process* p) {
    auto it = std::find(loaded_pages.begin(), loaded_pages.end(), page_id);

    // -------------------------------------------------------
    // PAGE HIT: Sayfa RAM'de mevcut
    // -------------------------------------------------------
    if (it != loaded_pages.end()) {
        std::cout << "[Memory] PAGE HIT: Sayfa-" << page_id << " RAM'de mevcut." << std::endl;

        // LRU: kullanilanı en sona tas (en yeni)
        loaded_pages.erase(it);
        loaded_pages.push_back(page_id);

        std::cout << "         -> [LRU] Sayfa-" << page_id
                  << " 'En Yeni Kullanilan' olarak isaretlendi (Silinmekten kurtuldu)."
                  << std::endl;
        return true; // Process etkilenmez, devam eder
    }

    // -------------------------------------------------------
    // PAGE FAULT: Sayfa RAM'de yok, diskten getirilmeli
    // -------------------------------------------------------
    std::cout << "[Memory] PAGE FAULT: Sayfa-" << page_id
              << " RAM'de bulunamadi! Diskten getiriliyor..." << std::endl;

    // RAM doluysa LRU sayfayi tahliye et
    if ((int)loaded_pages.size() >= ram_capacity) {
        std::cout << "[Memory] UYARI: RAM tamamen dolu! Yer aciliyor (LRU Eviction)." << std::endl;
        evict_page();
    }

    // Sayfayi RAM'e yukle
    loaded_pages.push_back(page_id);
    std::cout << "[Memory] Sayfa-" << page_id
              << " diskten RAM'e yuklendi." << std::endl;

    // Process verilmisse: page fault suresi kadar BLOCKED yap
    if (p != nullptr) {
        p->io_wait_ms = PAGE_FAULT_IO_MS;
        p->state      = ProcessState::BLOCKED;
        std::cout << "[Memory] PAGE FAULT BLOKLAMA: " << p->name
                  << " (PID: " << p->process_id
                  << ") BLOCKED durumuna alindi (io_wait_ms = "
                  << PAGE_FAULT_IO_MS << " ms)." << std::endl;
        std::cout << "[Memory] Sayfa yukleme tamamlandiginda Scheduler süreci uyandıracak."
                  << std::endl;
    }

    std::cout << "-----------------------------------" << std::endl;
    return false; // Page fault: process bloklandi
}

void MemoryManager::evict_page() {
    if (!loaded_pages.empty()) {
        int evicted = loaded_pages.front();
        loaded_pages.erase(loaded_pages.begin());
        std::cout << "[Memory] EVICTION (LRU): En uzun suredir kullanilmayan Sayfa-"
                  << evicted << " RAM'den silindi." << std::endl;
    }
}