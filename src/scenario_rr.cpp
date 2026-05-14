// ============================================================
// SENARYO 2: Round Robin Scheduler + I/O Blocking
// Calistirma: .\build\Debug\scenario_rr.exe
// ============================================================
//
// Gosterilen kavramlar:
//  - Round Robin: her process esit quantum alir
//  - Quantum dolunca process kuyrugun sonuna gider (preemptive)
//  - I/O'ya giren process BLOCKED: CPU bos kalmaz, siradaki calisir
//  - I/O biten process kuyrugun SONUNA eklenir (FIFO'dan farki)
//  - Adil CPU paylasimi: hicbir process aclikmaz (starvation yok)
// ============================================================

#include "../include/scheduler.h"
#include <iostream>

int main() {
  std::cout << "=========================================" << std::endl;
  std::cout << " SENARYO 2: Round Robin Scheduler" << std::endl;
  std::cout << " Preemptive + I/O Blocking" << std::endl;
  std::cout << "=========================================\n" << std::endl;

  FileSystem fs;
  Scheduler scheduler;

  // --------------------------------------------------------
  // SUREC TANIMI
  // Farkli burst sureleri: RR adil paylasim saglar.
  // p1 cok uzun bir CPU islemi yapar -> FIFO'da diger processler bekler
  // ama RR'de her process esit quantum aliyor.
  // --------------------------------------------------------
  Process p1(1, "DB_HeavyQuery", 1, 6000); // Cok buyuk CPU islemi
  Process p2(2, "DB_Report", 2, 3000);     // Orta CPU islemi + I/O
  Process p3(3, "DB_Ping", 3, 1500);       // Kucuk islem, hizli bitmeli
  Process p4(4, "DB_Backup", 2, 4500);     // Buyuk islem + I/O

  scheduler.add_process(&p1);
  scheduler.add_process(&p2);
  scheduler.add_process(&p3);
  scheduler.add_process(&p4);

  // --------------------------------------------------------
  // ASAMA 1: Round Robin - Saf CPU Senaryosu (I/O yok)
  // --------------------------------------------------------
  std::cout << "--- [ASAMA 1] ROUND ROBIN - SAF CPU (I/O YOK) ---" << std::endl;
  std::cout << ">>> Quantum = 2000ms. Her process sirasyla 2000ms CPU alir."
            << std::endl;
  std::cout << ">>> Burst bitmeyene kadar surecler donumlere bolunur.\n"
            << std::endl;

  // I/O isteği yok: sadece quantum mekanizmasini goster
  std::map<int, std::string> no_io = {};
  scheduler.run_round_robin(fs, no_io, /*quantum_ms=*/2000, /*tick_ms=*/1500);

  // --------------------------------------------------------
  // ASAMA 2: Round Robin - I/O Ile Birlikte
  // --------------------------------------------------------
  std::cout << "\n\n--- [ASAMA 2] ROUND ROBIN + I/O BLOCKING ---" << std::endl;
  std::cout << ">>> Quantum = 2000ms. I/O'ya giren process BLOCKED,"
            << std::endl;
  std::cout << ">>> CPU hemen siradaki process'e geciyor.\n" << std::endl;

  // Process'leri sifirla ve yeniden kuyruga ekle
  Scheduler scheduler2;
  FileSystem fs2;

  Process q1(1, "DB_HeavyQuery", 1, 6000);
  Process q2(2, "DB_Report", 2, 3000);
  Process q3(3, "DB_Ping", 3, 1500);
  Process q4(4, "DB_Backup", 2, 4500);

  // q2 ilk CPU'ya geldiginde I/O yapacak
  // q4 de I/O yapacak
  // q1 ve q3 saf CPU
  scheduler2.add_process(&q1);
  scheduler2.add_process(&q2);
  scheduler2.add_process(&q3);
  scheduler2.add_process(&q4);

  std::map<int, std::string> io_requests = {
      {2, "report.db"}, // q2: okuma I/O
      {4, "backup.db"}  // q4: yazma I/O
  };

  scheduler2.run_round_robin(fs2, io_requests, /*quantum_ms=*/2000,
                             /*tick_ms=*/1500);

  // --------------------------------------------------------
  // SONUC
  // --------------------------------------------------------
  std::cout << "\n=========================================" << std::endl;
  std::cout << "[SISTEM] Tum senaryolar tamamlandi." << std::endl;

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

  std::cout << "  [ASAMA 2 - RR + I/O] Son durumlar:" << std::endl;
  for (auto *p : {&q1, &q2, &q3, &q4})
    std::cout << "  PID " << p->process_id << " [" << p->name << "] -> "
              << state_str(p->state) << std::endl;

  std::cout << "=========================================" << std::endl;
  return 0;
}
