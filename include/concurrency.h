#ifndef CONCURRENCY_H // Başlık dosyasının birden fazla kez derlenip hata
                      // vermesini önleyen güvenlik kalkanı (Include Guard).
#define CONCURRENCY_H // Derleyiciye bu dosyanın işlendiğini bildirir.

#include "process.h" // Süreçlerin (Process) durumlarını (BLOCKED) değiştirebilmek için dahil ediyoruz.
#include <iostream> // Gözlemlenebilirlik (Observability) gereksinimi için ekrana log basmamızı sağlar.
#include <map> // Kaynak (Resource) ve Süreç eşleşmelerini anahtar-değer (key-value) şeklinde tutmak için kullanıyoruz.
#include <vector> // Deadlock tespiti sırasında sistemdeki süreçleri taramak için kullanacağız.


// Eşzamanlılık Yöneticisi Sınıfı (Concurrency Manager)
// Veritabanı tablolarına konulan kilitleri (Lock/Mutex) yönetir ve Kördüğüm
// (Deadlock) tespiti yapar.
class ConcurrencyManager {
private: // Dışarıdan izinsiz kilit kırılmasını önlemek için verileri private
         // yapıyoruz (Kapsülleme).
  // Hangi kaynağın (Örn: 1 nolu Tablo) hangi süreç (Process pointer) tarafından
  // kilitlendiğini tutan tablo. std::map kullanıyoruz çünkü Arama/Ekleme
  // işlemleri O(log n) süresinde çalışarak performansı artırır.
  std::map<int, Process *> resource_locks;

public: // Scheduler ve Main dosyasından erişilebilecek fonksiyonlar.
  ConcurrencyManager(); // Kurucu fonksiyon.

  // Bir sürecin veritabanı tablosuna kilit (Lock) koymaya çalışmasını simüle
  // eden fonksiyon. Eğer kilit başarılıysa true, kaynak başkasındaysa (süreç
  // bekleyecekse) false döner.
  bool acquire_lock(Process *p, int resource_id);

  // Süreç işini bitirdiğinde elindeki kaynağı (tabloyu) serbest bırakan
  // fonksiyon.
  void release_lock(Process *p, int resource_id);

  // Projenin "Engineering Challenge" (Mühendislik Zorluğu) şartını sağlayan ana
  // fonksiyon: Deadlock Dedektörü! Bekleme Grafı (Wait-For Graph) mantığıyla
  // süreçler arasında döngüsel bir bekleme (Cyclic Wait) olup olmadığını
  // kontrol eder.
  bool detect_deadlock(const std::vector<Process *> &all_processes);
}; // Sınıf tanımlaması noktalı virgül ile sonlandırılır.

#endif // CONCURRENCY_H sonu.