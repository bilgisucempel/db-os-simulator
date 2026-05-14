#ifndef MEMORY_H // Başlık dosyasının çift derlenmesini önleyen güvenlik kalkanı
                 // (Include Guard).
#define MEMORY_H // Derleyiciye dosyanın işlendiğini bildirir.

#include <algorithm> // Vektörler içinde arama ve silme yapmak için standart algoritma kütüphanesi.
#include <iostream> // Gözlemlenebilirlik (Observability) için loglama yapmamızı sağlar.
#include <vector> // RAM'deki sayfaların listesini tutmak için dinamik dizi kullanıyoruz.


// Bellek Yöneticisi Sınıfı (Memory Manager) - ENHANCED (Geliştirilmiş) Sürüm
// Baseline (Temel - FIFO) modelin yerine, zeki bir LRU (Least Recently Used)
// algoritması kullanır.
class MemoryManager {
private: // Bellek bloklarına izinsiz müdahaleyi engelliyoruz (Encapsulation).
  int ram_capacity; // İşletim sistemimizin RAM kapasitesi (Maksimum sayfa
                    // sayısı).

  // LRU Mantığı: Bu vektörün EN BAŞI (index 0) 'En Eski / En Az Kullanılan',
  // EN SONU ise 'En Yeni / En Son Kullanılan' sayfayı temsil edecek.
  std::vector<int> loaded_pages;

public:
  MemoryManager(int capacity); // Kurucu fonksiyon.

  // Süreçlerin sayfaya erişimini sağlar ve LRU algoritmasını beyninde
  // çalıştırır.
  bool access_page(int page_id);

  // RAM dolduğunda 'En Az Yakın Zamanda Kullanılan' sayfayı tespit edip siler.
  void evict_page();
};

#endif // MEMORY_H sonu.