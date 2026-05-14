#include "../include/scheduler.h"

Scheduler::Scheduler() {}

void Scheduler::add_process(Process* p) {
    all_processes.push_back(p);
    p->state = ProcessState::READY;
    ready_queue.push(p);
    std::cout << "[Scheduler] Eklendi: " << p->name
              << " (PID: " << p->process_id << ")" << std::endl;
}

void Scheduler::run_fifo() {
    std::cout << "\n[Scheduler] FIFO basliyor..." << std::endl;
    while (!ready_queue.empty()) {
        Process* cur = ready_queue.front(); ready_queue.pop();
        cur->state = ProcessState::RUNNING;
        std::cout << "[CPU] Calistiriliyor: " << cur->name
                  << " | burst: " << cur->cpu_burst_time << std::endl;
        cur->cpu_burst_time = 0;
        cur->state = ProcessState::TERMINATED;
        std::cout << "[Scheduler] Bitti: " << cur->name << "\n---" << std::endl;
    }
}

// ============================================================
// I/O-Aware FIFO Scheduler
// ============================================================
// Akis:
//  A) READY process'i CPU'ya ver.
//     - I/O isteği varsa FileSystem'e gönder -> BLOCKED olur.
//       * io_wait_ms > 0  : aktif I/O yapıyor  -> blocked_files'a ekle
//       * io_wait_ms == 0 : dosya mesgul, kuyrukta bekliyor -> izleme yok,
//                           FileSystem::release_file() onu uyandıracak
//     - I/O isteği yoksa CPU burst bitir -> TERMINATED.
//  B) Her tick'te BLOCKED process'lerin io_wait_ms azalt.
//     I/O biten (READY olan) process:
//       - release_file() ile dosya serbest bırakılır.
//       - release_file bir kuyruk process'i döndürdüyse onu da blocked_files'a al.
//       - pending_io'dan silinir -> sıradaki CPU turunda direkt TERMINATED.
//       - ready_queue'ya eklenir.
// ============================================================
void Scheduler::run_with_io(FileSystem& fs,
                             const std::map<int, std::string>& io_requests,
                             int tick_ms,
                             MemoryManager* mm,
                             const std::map<int, int>& page_requests) {

    std::cout << "\n[Scheduler] I/O-Aware FIFO Scheduler basliyor (tick=" << tick_ms << "ms)..." << std::endl;
    if (mm) std::cout << "[Scheduler] Page Fault simulasyonu AKTIF." << std::endl;
    std::cout << "=========================================\n" << std::endl;

    std::map<int, std::string> pending_io    = io_requests;
    std::map<int, int>         pending_pages = page_requests; // pid -> sayfa_id
    std::map<int, std::string> blocked_files;

    auto all_done = [&]() {
        for (auto* p : all_processes)
            if (p->state != ProcessState::TERMINATED) return false;
        return true;
    };

    while (!all_done()) {

        // --------------------------------------------------
        // A) CPU: Kuyruktaki READY process'i çalıştır
        // --------------------------------------------------
        if (!ready_queue.empty()) {
            Process* cur = ready_queue.front(); ready_queue.pop();
            cur->state = ProcessState::RUNNING;

            std::cout << "[CPU] >>> " << cur->name << " (PID: " << cur->process_id
                      << ") | burst: " << cur->cpu_burst_time << " birim" << std::endl;

            bool blocked_this_turn = false;

            // ------ 1. PAGE FAULT KONTROLU ------
            auto pit = pending_pages.find(cur->process_id);
            if (!blocked_this_turn && pit != pending_pages.end() && mm != nullptr) {
                int page_id = pit->second;
                std::cout << "[CPU] " << cur->name << " sayfa erisimi: Sayfa-" << page_id << std::endl;
                bool hit = mm->access_page(page_id, cur);
                pending_pages.erase(pit); // Bir kez tetiklenir
                if (!hit && cur->state == ProcessState::BLOCKED) {
                    // Page Fault: io_wait_ms set edildi, tick uyandıracak
                    std::cout << "[CPU] Context Switch: " << cur->name
                              << " PAGE FAULT -> BLOCKED (" << PAGE_FAULT_IO_MS
                              << " ms). Siradaki process aliniyor.\n" << std::endl;
                    blocked_this_turn = true;
                }
                // Page Hit ise devam et (I/O veya burst kontrolune geç)
            }

            // ------ 2. I/O KONTROLU ------
            if (!blocked_this_turn) {
                auto it = pending_io.find(cur->process_id);
                if (it != pending_io.end()) {
                    const std::string& fname = it->second;
                    std::cout << "[CPU] " << cur->name << " -> I/O isteği: '"
                              << fname << "'" << std::endl;

                    fs.read_file(cur, fname);

                    if (cur->state == ProcessState::BLOCKED) {
                        blocked_this_turn = true;
                        if (cur->io_wait_ms > 0) {
                            blocked_files[cur->process_id] = fname;
                            std::cout << "[CPU] Context Switch: " << cur->name
                                      << " BLOCKED (I/O). Siradaki process aliniyor.\n" << std::endl;
                        } else {
                            std::cout << "[CPU] Context Switch: " << cur->name
                                      << " BLOCKED (dosya kuyruğu). Siradaki process aliniyor.\n" << std::endl;
                        }
                    }
                }
            }

            // ------ 3. NE PAGE FAULT NE I/O: CPU burst bitir ------
            if (!blocked_this_turn) {
                cur->cpu_burst_time = 0;
                cur->state = ProcessState::TERMINATED;
                std::cout << "[Scheduler] Tamamlandi: " << cur->name << std::endl;
                std::cout << "-----------------------------------" << std::endl;
            }
        }


        // --------------------------------------------------
        // B) TICK: BLOCKED process'lerin sayaçlarını azalt
        // --------------------------------------------------
        std::this_thread::sleep_for(std::chrono::milliseconds(tick_ms));

        for (auto* p : all_processes) {
            if (p->state == ProcessState::BLOCKED && p->io_wait_ms > 0) {
                p->tick_io(tick_ms);

                if (p->state == ProcessState::READY) {
                    auto fit = blocked_files.find(p->process_id);
                    if (fit != blocked_files.end()) {
                        // ---- DOSYA I/O bitti ----
                        // Dosyayi serbest birak; kuyrukta bekleyen varsa otomatik I/O'ya al
                        Process* next_waiter = fs.release_file(fit->second);
                        if (next_waiter) {
                            blocked_files[next_waiter->process_id] = fit->second;
                            std::cout << "[Scheduler] " << next_waiter->name
                                      << " dosya kuyruğundan I/O'ya alindi." << std::endl;
                        }
                        blocked_files.erase(fit);
                        // Sadece dosya I/O bitince pending_io'dan sil.
                        // Page fault'tan uyananlar pending_io'da KALMALI
                        // (bir sonraki CPU turunda dosyaya erisecekler).
                        pending_io.erase(p->process_id);
                        std::cout << "[Scheduler] " << p->name << " (PID: " << p->process_id
                                  << ") Dosya I/O bitti -> READY kuyruguna eklendi." << std::endl;
                    } else {
                        // ---- PAGE FAULT bitti ----
                        // pending_io SILINMEZ: process bir sonraki CPU turunda
                        // dosya I/O'sunu yapacak (veya dosya mesgulsa bekleme kuyruğuna girecek).
                        std::cout << "[Scheduler] " << p->name << " (PID: " << p->process_id
                                  << ") Page Fault cozuldu -> READY kuyruguna eklendi."
                                  << " (pending I/O korundu)" << std::endl;
                    }
                    ready_queue.push(p);
                }
            }
        }


        // --------------------------------------------------
        // C) Kuyruk boş ama process'ler hâlâ BLOCKED ise bildir
        // --------------------------------------------------
        if (ready_queue.empty() && !all_done()) {
            bool any_active = false;
            for (auto* p : all_processes)
                if (p->state == ProcessState::BLOCKED) { any_active = true; break; }
            if (any_active)
                std::cout << "[CPU] Bos: process'ler BLOCKED bekliyor..." << std::endl;
        }
    }

    std::cout << "\n[Scheduler] Tum process'ler tamamlandi." << std::endl;
}

// ============================================================
// Round Robin Scheduler (I/O-Aware)
// ============================================================
// Her process 'quantum_ms' kadar CPU alir.
// Bitmezse cpu_burst_time azaltilip kuyrugun SONUNA geri eklenir.
// I/O isteği varsa: process BLOCKED, CPU hemen digerine gecer.
// I/O biten process kuyrugun sonuna eklenir (kalan burst ile).
// ============================================================
void Scheduler::run_round_robin(FileSystem& fs,
                                const std::map<int, std::string>& io_requests,
                                int quantum_ms,
                                int tick_ms,
                                MemoryManager* mm,
                                const std::map<int, int>& page_requests) {

    std::cout << "\n[Scheduler] Round Robin Scheduler basliyor..." << std::endl;
    std::cout << "[Scheduler] Quantum: " << quantum_ms << " ms | Tick: " << tick_ms << " ms" << std::endl;
    if (mm) std::cout << "[Scheduler] Page Fault simulasyonu AKTIF." << std::endl;
    std::cout << "=========================================\n" << std::endl;

    std::map<int, std::string> pending_io    = io_requests;
    std::map<int, int>         pending_pages = page_requests;
    std::map<int, std::string> blocked_files;


    auto all_done = [&]() {
        for (auto* p : all_processes)
            if (p->state != ProcessState::TERMINATED) return false;
        return true;
    };

    while (!all_done()) {

        // --------------------------------------------------
        // A) CPU: Quantum kadar calistir
        // --------------------------------------------------
        if (!ready_queue.empty()) {
            Process* cur = ready_queue.front(); ready_queue.pop();
            cur->state = ProcessState::RUNNING;

            int used = std::min(quantum_ms, cur->cpu_burst_time);
            std::cout << "[CPU][RR] >>> " << cur->name
                      << " (PID: " << cur->process_id
                      << ") | Kalan burst: " << cur->cpu_burst_time
                      << " | Quantum: " << used << " ms" << std::endl;

            // I/O isteği var mi?
            auto it = pending_io.find(cur->process_id);
            if (it != pending_io.end()) {
                const std::string& fname = it->second;
                std::cout << "[CPU][RR] " << cur->name
                          << " I/O istegiyle karsilasti ('" << fname
                          << "'). CPU birakiliyor..." << std::endl;

                fs.read_file(cur, fname);

                if (cur->state == ProcessState::BLOCKED) {
                    if (cur->io_wait_ms > 0) {
                        blocked_files[cur->process_id] = fname;
                        std::cout << "[CPU][RR] Context Switch: " << cur->name
                                  << " BLOCKED (I/O). Siradaki process aliniyor.\n" << std::endl;
                    } else {
                        // Dosya mesgul: kuyrukta bekliyor
                        std::cout << "[CPU][RR] Context Switch: " << cur->name
                                  << " BLOCKED (dosya kuyruğu). Siradaki process aliniyor.\n" << std::endl;
                    }
                }
            } else {
                // I/O yok: quantum kadar is yap
                cur->cpu_burst_time -= used;

                if (cur->cpu_burst_time <= 0) {
                    cur->cpu_burst_time = 0;
                    cur->state = ProcessState::TERMINATED;
                    std::cout << "[Scheduler][RR] Tamamlandi: " << cur->name << std::endl;
                    std::cout << "-----------------------------------" << std::endl;
                } else {
                    // Quantum doldu ama is bitmedi: kuyrugun sonuna al
                    cur->state = ProcessState::READY;
                    ready_queue.push(cur);
                    std::cout << "[CPU][RR] Quantum doldu! " << cur->name
                              << " -> Kalan: " << cur->cpu_burst_time
                              << " ms. Kuyrugun sonuna alindi." << std::endl;
                    std::cout << "-----------------------------------" << std::endl;
                }
            }
        }

        // --------------------------------------------------
        // B) TICK: BLOCKED process'lerin I/O sayacini azalt
        // --------------------------------------------------
        std::this_thread::sleep_for(std::chrono::milliseconds(tick_ms));

        for (auto* p : all_processes) {
            if (p->state == ProcessState::BLOCKED && p->io_wait_ms > 0) {
                p->tick_io(tick_ms);

                if (p->state == ProcessState::READY) {
                    auto fit = blocked_files.find(p->process_id);
                    if (fit != blocked_files.end()) {
                        Process* next_waiter = fs.release_file(fit->second);
                        if (next_waiter) {
                            blocked_files[next_waiter->process_id] = fit->second;
                            std::cout << "[Scheduler][RR] " << next_waiter->name
                                      << " dosya kuyrugundan I/O'ya alindi." << std::endl;
                        }
                        blocked_files.erase(fit);
                        // Sadece dosya I/O bitince pending_io sil
                        pending_io.erase(p->process_id);
                        std::cout << "[Scheduler][RR] " << p->name
                                  << " (PID: " << p->process_id
                                  << ") Dosya I/O bitti -> READY kuyruğunun sonuna eklendi." << std::endl;
                    } else {
                        // Page Fault bitti: pending_io SILINMEZ
                        std::cout << "[Scheduler][RR] " << p->name
                                  << " (PID: " << p->process_id
                                  << ") Page Fault cozuldu -> READY kuyruğunun sonuna eklendi."
                                  << " (pending I/O korundu)" << std::endl;
                    }
                    ready_queue.push(p);
                }
            }
        }


        // --------------------------------------------------
        // C) Idle: tum process'ler BLOCKED
        // --------------------------------------------------
        if (ready_queue.empty() && !all_done()) {
            bool any_blocked = false;
            for (auto* p : all_processes)
                if (p->state == ProcessState::BLOCKED) { any_blocked = true; break; }
            if (any_blocked)
                std::cout << "[CPU][RR] Bos: tum process'ler BLOCKED. I/O bekleniyor..." << std::endl;
        }
    }

    std::cout << "\n[Scheduler][RR] Tum process'ler tamamlandi." << std::endl;
}