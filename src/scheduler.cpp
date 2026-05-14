#include "../include/scheduler.h" // Sınıf tanımlamalarımızın bulunduğu başlık dosyasını dahil ediyoruz. ('../' komutu bir üst klasöre çıkıp include klasörüne girmesini sağlar)

// Kurucu fonksiyon: Nesne oluşturulduğunda yapılacak ilk işler. Şu an için
// ekstra bir ayara gerek yok.
Scheduler::Scheduler() {}

// Sisteme yeni bir süreç (Process) ekler. İşletim sistemi yeni bir veritabanı
// sorgusu aldığında bu çalışır.
void Scheduler::add_process(Process *p) {
  all_processes.push_back(p); // Süreci tüm süreçler listesine kaydediyoruz.
  p->state =
      ProcessState::READY; // Sürecin durumunu 'Yeni' (NEW) durumundan 'Hazır'
                           // (READY) durumuna geçiriyoruz. CPU'yu bekliyor.
  ready_queue.push(p);     // Süreci hazır kuyruğunun (FIFO mantığı gereği en
                           // arkasına) ekliyoruz.

  // Hocanın 'Observability & Logging' zorunluluğu için sisteme log basıyoruz.
  std::cout << "[Scheduler] Yeni surec eklendi: " << p->name
            << " (PID: " << p->process_id << ")" << std::endl;
}

// Baseline (Temel) zamanlayıcı algoritmamız: First-In-First-Out (FIFO)
// Çalışma mantığı: Kuyruktaki ilk süreci alır, işi bitene kadar
// (Non-preemptive) CPU'da çalıştırır.
void Scheduler::run_fifo() {
  std::cout << "\n[Scheduler] FIFO algoritmasi ile calistirma basliyor..."
            << std::endl; // İşlemin başladığını logluyoruz.

  while (!ready_queue.empty()) { // Hazır kuyruğunda süreç olduğu sürece CPU
                                 // çalışmaya devam eder (Döngü).
    Process *current =
        ready_queue.front(); // Kuyruğun en başındaki (en uzun süredir bekleyen)
                             // süreci seçiyoruz.
    ready_queue.pop(); // Seçtiğimiz süreci kuyruktan çıkarıyoruz ki sonsuz
                       // döngüye girmeyelim.

    current->state =
        ProcessState::RUNNING; // Sürecin durumunu 'Çalışıyor' yapıyoruz. Bağlam
                               // geçişi (Context Switch) başlangıcı.

    // Sürecin CPU'ya alındığını ve iş yükünü logluyoruz. Veritabanı temamıza
    // uygun bir log.
    std::cout << "[CPU] Calistiriliyor: " << current->name
              << " | Islem yuku: " << current->cpu_burst_time << " birim."
              << std::endl;

    // Sürecin çalışmasını simüle ediyoruz. FIFO kesintisiz olduğu için süreç
    // anında işini bitirir.
    current->cpu_burst_time = 0; // İşlem yükü sıfırlandı, yani süreç işini
                                 // (veritabanı sorgusunu) tamamladı.

    current->state =
        ProcessState::TERMINATED; // İşi biten sürecin durumunu 'Sonlandı'
                                  // (Terminated) olarak güncelliyoruz. İşletim
                                  // sistemi kaynakları geri alır.

    // Sürecin başarıyla bittiğini logluyoruz.
    std::cout << "[Scheduler] Surec sonlandi: " << current->name << std::endl;
    std::cout << "-----------------------------------"
              << std::endl; // Okunabilirliği artırmak için loglara ayırıcı
                            // çizgi koyuyoruz.
  }
}