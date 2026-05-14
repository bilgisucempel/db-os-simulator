#include "../include/memory.h" // Bellek yöneticisi (Paging ve Page Fault) sınıfımızı dahil ediyoruz.
#include <iostream> // Konsola çıktı vermek ve sistemin durumunu loglamak (Observability) için standart kütüphane.


int main() {
  // 1. SİSTEM BAŞLATMA
  std::cout << "=========================================" << std::endl;
  std::cout << "[SYSTEM] Veritabani OS Simulatoru Baslatiliyor..." << std::endl;
  std::cout
      << "[SYSTEM] Bellek Tukenmesi (Memory Exhaustion) Senaryosu Yukleniyor."
      << std::endl;
  std::cout << "=========================================\n" << std::endl;

  // 2. BELLEK YÖNETİCİSİNİ BAŞLATMA
  // İşletim sistemine (simülasyona) sadece 2 sayfalık (Page) çok kısıtlı bir
  // RAM veriyoruz. Neden 2? Hocanın istediği 'Memory Exhaustion' (Bellek
  // Tükenmesi) durumunu ve Page Replacement (Sayfa Değiştirme) algoritmasının
  // çalışmasını hızlıca simüle edip gözlemleyebilmek için.
  MemoryManager os_memory(2);

  std::cout << "\n--- ASAMA 1: RAM'in Doldurulmasi ---" << std::endl;

  // Veritabanı Süreci (Process) Sayfa-10'u (Örn: Kullanıcılar Tablosunun ilk
  // parçası) okumak istiyor. İlk başta RAM boş olduğu için 'Page Fault' (Sayfa
  // Hatası) oluşacak ve diskten getirilecek.
  os_memory.access_page(10);

  // Veritabanı Süreci Sayfa-20'yi (Örn: Siparişler Tablosu) okumak istiyor.
  // Yine RAM'de olmadığı için 'Page Fault' oluşacak. Şu an RAM kapasitesi (2/2)
  // tamamen doldu!
  os_memory.access_page(20);

  std::cout << "\n--- ASAMA 2: Hizli Erisim (Page Hit) ---" << std::endl;

  // Süreç, az önce diskten getirdiği Sayfa-10'a tekrar erişmek istiyor.
  // Bu kez veri RAM'de hazır olduğu için hiçbir bekleme yaşanmadan anında
  // okunacak (Page Hit). Trade-off (Ödünleşim): RAM kullanmak diske göre çok
  // pahalıdır ama bu sayede verilere inanılmaz hızlı erişiriz.
  os_memory.access_page(10);

  std::cout
      << "\n--- ASAMA 3: BELLEK TUKENMESI (Memory Exhaustion) VE EVICTION ---"
      << std::endl;

  // Ağır bir veritabanı sorgusu geldi ve Sayfa-30'u talep ediyor!
  // Ancak RAM (2 kapasiteli) şu an Sayfa-10 ve Sayfa-20 ile tamamen dolu.
  // İşletim sistemi çökmek yerine (Failure Handling), RAM'deki en eski sayfayı
  // (Sayfa-10) silecek (Eviction) ve Sayfa-30'a yer açacak.
  os_memory.access_page(30);

  // Yeni bir sayfa (Sayfa-40) daha isteniyor.
  // RAM yine dolu olduğu için bu kez sıradaki en eski sayfa olan Sayfa-20
  // silinecek ve 40 yüklenecek.
  os_memory.access_page(40);

  std::cout << "\n=========================================" << std::endl;
  std::cout << "[SYSTEM] Simulasyon basariyla tamamlandi. Kapatiliyor..."
            << std::endl;
  return 0; // İşletim sisteminin hatasız sonlandığını bildiriyoruz.
}