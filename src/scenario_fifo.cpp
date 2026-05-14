// ============================================================
// SENARYO 1: FIFO Scheduler + Page Fault Blocking + I/O Blocking + Dosya
// Cakisma Calistirma: .\build\Debug\scenario_fifo.exe
// ============================================================
//
// Gosterilen kavramlar:
//  1. LRU Page Replacement (Bellek Yonetimi)
//  2. Page Fault -> Process BLOCKED (disk I/O bekler, CPU bos kalmaz)
//  3. I/O-Aware FIFO Scheduler (dosya I/O bloklama)
//  4. Dosya Cakismasi: p1 ve p4 ayni dosyayi istiyor
//     p4, p1 bitene kadar BLOCKED (dosya kuyruğu) bekler
// ============================================================

#include "../include/memory.h"
#include "../include/scheduler.h"
#include <iostream>

int main() {
  std::cout << "=========================================" << std::endl;
  std::cout << " SENARYO 1: FIFO + Page Fault + I/O Blocking" << std::endl;
  std::cout << " Dosya Cakisma Senaryosu" << std::endl;
  std::cout << "=========================================\n" << std::endl;

  MemoryManager os_memory(2); // Sadece 2 sayfa kapasiteli kisitli RAM
  FileSystem fs;
  Scheduler scheduler;

  // --------------------------------------------------------
  // SUREC TANIMI
  // p1 ve p4 ayni dosyayi ("users.db") istiyor.
  // p1 ayrica sayfa erisimi yapacak -> Page Fault senaryosu.
  // p4, p1'in hemen arkasinda CPU'ya gelecek -> dosya cakismasi.
  // --------------------------------------------------------
  Process p1(1, "DB_Query_Read", 2, 4000);  // users.db okuyacak + sayfa erisimi
  Process p2(2, "DB_Query_Write", 1, 5000); // orders.db yazacak
  Process p3(3, "DB_Backup", 3, 2000);      // I/O yok, saf CPU islemi
  Process p4(4, "DB_Audit", 2, 3000);       // users.db okuyacak (CAKISMA!)

  // Kuyruk: p1 -> p4 -> p2 -> p3  (p4 hemen p1'in arkasinda -> cakisma
  // garantili)
  scheduler.add_process(&p1);
  scheduler.add_process(&p4);
  scheduler.add_process(&p2);
  scheduler.add_process(&p3);

  // --------------------------------------------------------
  // ASAMA 1: Page Fault + I/O Blocking Birlikte
  //
  // Beklenen akis:
  //  [CPU] p1 alinir -> Sayfa-10 erisimi -> PAGE FAULT -> p1 BLOCKED (3000ms)
  //  [CPU] CPU bos kalmaz! p4 alinir -> users.db MESGUL degil (p1 dosyaya
  //  gitmedi)
  //        -> p4 users.db I/O'ya baslayabilir
  //  ... (p1 page fault biter, READY olur, tekrar CPU alir)
  //  [CPU] p1 -> users.db I/O -> BLOCKED
  //  [CPU] p4 -> users.db MESGUL -> BLOCKED (dosya kuyruğu)
  //  [TICK] p1 I/O bitti -> release -> p4 otomatik I/O'ya alindi
  // --------------------------------------------------------
  std::cout << "--- [ASAMA 1] PAGE FAULT + I/O BLOCKING ---" << std::endl;
  std::cout << ">>> p1 once Page Fault ile BLOCKED olacak (CPU bos kalmaz)."
            << std::endl;
  std::cout << ">>> p1 uyandiktan sonra users.db I/O yapacak." << std::endl;
  std::cout << ">>> p4 users.db icin BLOCKED (dosya kuyruğu) kalacak.\n"
            << std::endl;

  // { pid -> dosya adi }
  std::map<int, std::string> io_requests = {
      {1, "users.db"}, // p1: okuma I/O
      {2, "users.db"}, // p2: yazma I/O — CAKISMA!
      {4, "orders.db"} // p4: okuma I/O
  };

  // { pid -> sayfa_id }: p1 CPU'ya gelince Sayfa-10'a erisecek -> PAGE FAULT
  std::map<int, int> page_requests = {
      {1, 10}, // p1: Sayfa-10 -> RAM bos, PAGE FAULT tetiklenir
      {3, 20}  // p3: Sayfa-20 -> RAM'de Sayfa-10 var, LRU silinir
  };

  scheduler.run_with_io(fs, io_requests, /*tick_ms=*/1500, &os_memory,
                        page_requests);

  // --------------------------------------------------------
  // SONUC
  // --------------------------------------------------------
  std::cout << "\n=========================================" << std::endl;
  std::cout << "[SISTEM] Son process durumlari:" << std::endl;

  auto state_str = [](ProcessState s) -> const char * {
    switch (s) {
    case ProcessState::TERMINATED:
      return "TERMINATED";
    case ProcessState::READY:
      return "READY";
    case ProcessState::BLOCKED:
      return "BLOCKED";
    case ProcessState::RUNNING:
      return "RUNNING";
    default:
      return "NEW";
    }
  };

  for (auto *p : {&p1, &p2, &p3, &p4})
    std::cout << "  PID " << p->process_id << " [" << p->name << "] -> "
              << state_str(p->state) << std::endl;

  std::cout << "=========================================" << std::endl;
  return 0;
}
