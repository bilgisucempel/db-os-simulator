#ifndef SCHEDULER_H // Eğer bu başlık dosyası projede birden fazla kez
                    // çağrılırsa, kodların çift tanımlanmasını engeller.
                    // (Include Guard)
#define SCHEDULER_H // Derleyiciye bu dosyanın okunduğunu bildirir.

#include "process.h" // Süreç yapısını (PCB) kullanmak için kendi dosyamızı dahil ediyoruz.
#include <iostream> // Ekrana log (çıktı) yazdırmak için gerekli kütüphane.
#include <queue> // Hazır (READY) süreçleri bir kuyrukta tutmak için standart kütüphaneyi ekliyoruz.
#include <vector> // Sistemdeki tüm süreçlerin genel bir listesini tutmak için kullanacağız.


// Scheduler (Zamanlayıcı) sınıfı: İşletim sisteminin kalbidir. Hangi sürecin ne
// zaman CPU'da çalışacağına karar verir.
class Scheduler {
private: // Dışarıdan doğrudan müdahaleyi engellemek için private (Kapsülleme -
         // Encapsulation) kullanıyoruz.
  std::queue<Process *>
      ready_queue; // Sadece READY durumundaki süreçlerin işaretçilerini
                   // (pointer) tutan kuyruk. Bellek tasarrufu için pointer
                   // kullanıyoruz.
  std::vector<Process *>
      all_processes; // Sistemdeki tüm süreçlerin kaydını tuttuğumuz ana liste
                     // (İleride Deadlock dedektörü bu listeyi tarayacak).

public: // Sınıf dışından (örneğin main.cpp'den) erişilebilecek fonksiyonlar.
  Scheduler(); // Kurucu fonksiyon (Constructor). Sınıf çağrıldığında ilk
               // çalışan fonksiyondur.

  void add_process(Process *p); // Sisteme yeni bir süreç (Örn: Veritabanı
                                // sorgusu) ekleyen fonksiyon.
  void run_fifo(); // Baseline (Temel) algoritmamız: First-In-First-Out (İlk
                   // gelen ilk çalışır) mantığını yürüten fonksiyon.
}; // C++ kuralları gereği sınıf tanımlaması noktalı virgül ile biter.

#endif // SCHEDULER_H tanımının sonu.