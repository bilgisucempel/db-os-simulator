#include "../include/memory.h"
#include "../include/scheduler.h" // scheduler.h -> filesystem.h + process.h zaten içeriyor
#include <iostream>

int main() {
  std::cout << "=========================================" << std::endl;
  std::cout << "[SYSTEM] Veritabani OS Simulatoru Baslatiliyor..." << std::endl;
  std::cout << "=========================================\n" << std::endl;

  // --------------------------------------------------------
  // ALT SISTEMLER
  // --------------------------------------------------------
  MemoryManager os_memory(2);
  FileSystem fs;
  Scheduler scheduler;

  // --------------------------------------------------------
  // SUREC TANIMI
  //   p1 ve p4 AYNI dosyayi ("users.db") okumak istiyor!
  //   p1 once CPU'ya girecek, users.db'yi alacak.
  //   p4 CPU'ya geldiginde users.db mesgul -> BLOCKED (kuyruk).
  //   p1 I/O bitince release_file() -> p4 otomatik I/O'ya giriyor.
  // --------------------------------------------------------
  Process p1(1, "DB_Query_Read", 2, 500);  // users.db okuyacak
  Process p2(2, "DB_Query_Write", 1, 800); // orders.db yazacak
  Process p3(3, "DB_Backup", 3, 300);      // I/O yok, saf CPU
  Process p4(4, "DB_Audit", 2, 400);       // users.db okuyacak (CAKISMA!)

  // Kuyruk sirasi ONEMLI:
  //   p1 -> p4 -> p2 -> p3
  //   p4, p1'in hemen arkasinda CPU'ya gelecek.
  //   O sirada p1'in I/O'su devam ediyor, users.db mesgul
  //   -> p4 BLOCKED (dosya kuyruğu) gozlemlenecek.
  scheduler.add_process(&p1);
  scheduler.add_process(&p4); // <-- p1'den hemen sonra: cakisma garantili
  scheduler.add_process(&p2);
  scheduler.add_process(&p3);

  // --------------------------------------------------------
  // BELLEK SENARYOSU (LRU Paging)
  // --------------------------------------------------------
  std::cout << "\n--- [ASAMA 1] LRU BELLEK YONETIMI ---" << std::endl;
  os_memory.access_page(10); // Page Fault
  os_memory.access_page(20); // Page Fault, RAM dolu
  os_memory.access_page(10); // Page Hit
  os_memory.access_page(30); // Page Fault -> Sayfa-20 tahliye
  os_memory.access_page(40); // Page Fault -> Sayfa-10 tahliye

  // --------------------------------------------------------
  // I/O-AWARE SCHEDULER SENARYOSU
  //
  // Beklenen akis:
  //  [CPU] p1 alinir  -> users.db I/O (2000ms) -> p1 BLOCKED
  //  [CPU] p2 alinir  -> orders.db I/O (3000ms) -> p2 BLOCKED
  //  [CPU] p3 alinir  -> I/O yok -> TERMINATED
  //  [CPU] p4 alinir  -> users.db MESGUL (p1 kullaniyor)
  //                   -> p4 BLOCKED (dosya kuyruğu, io_wait_ms=0)
  //  [TICK ~2000ms] p1 I/O bitti -> release("users.db")
  //                   -> p4 kuyruktan cikar, I/O baslar (2000ms)
  //                   -> p1 READY kuyruğuna eklenir
  //  [CPU] p1 alinir  -> pending_io'da yok -> TERMINATED
  //  [TICK ~3000ms] p2 I/O bitti -> release("orders.db") -> p2 READY
  //  [CPU] p2 alinir  -> TERMINATED
  //  [TICK ~4000ms] p4 I/O bitti -> release("users.db") -> p4 READY
  //  [CPU] p4 alinir  -> TERMINATED
  // --------------------------------------------------------
  std::cout << "\n--- [ASAMA 2] I/O-AWARE CPU ZAMANLAYICI ---" << std::endl;
  std::cout << ">>> SENARYO: p1 ve p4 ayni dosyayi okumak istiyor!"
            << std::endl;
  std::cout << ">>> p4, p1 bitmeden 'users.db'ye erisemez -> BLOCKED (kuyruk)\n"
            << std::endl;

  // { process_id -> dosya adi }
  // Her iki process de "users.db" okuyacak
  std::map<int, std::string> io_requests = {
      {1, "users.db"},  // p1: okuma
      {4, "users.db"},  // p4: okuma — CAKISMA!
      {2, "orders.db"}, // p2: yazma
  };

  scheduler.run_with_io(fs, io_requests, /*tick_ms=*/1500);

  // --------------------------------------------------------
  // FINAL DURUM RAPORU
  // --------------------------------------------------------
  std::cout << "\n=========================================" << std::endl;
  std::cout << "[SYSTEM] Simulasyon tamamlandi." << std::endl;
  std::cout << "[SYSTEM] Son process durumlari:" << std::endl;

  auto state_str = [](ProcessState s) -> const char * {
    switch (s) {
    case ProcessState::NEW:
      return "NEW";
    case ProcessState::READY:
      return "READY";
    case ProcessState::RUNNING:
      return "RUNNING";
    case ProcessState::BLOCKED:
      return "BLOCKED";
    case ProcessState::TERMINATED:
      return "TERMINATED";
    }
    return "?";
  };

  for (auto *p : {&p1, &p2, &p3, &p4}) {
    std::cout << "  PID " << p->process_id << " [" << p->name << "] -> "
              << state_str(p->state) << std::endl;
  }

  std::cout << "=========================================" << std::endl;
  return 0;
}