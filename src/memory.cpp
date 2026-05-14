#include "../include/memory.h" // Akıllı (LRU destekli) bellek başlık dosyamızı dahil ediyoruz.

// Kurucu fonksiyon: İşletim sistemi başlatılırken RAM kapasitesi atanır.
MemoryManager::MemoryManager(int capacity) {
  ram_capacity = capacity;
  std::cout << "[Memory] Bellek Yoneticisi baslatildi. Algoritma: LRU "
               "(Enhanced). Kapasite: "
            << ram_capacity << " sayfa." << std::endl;
}

// Bir sürecin veritabanındaki bir sayfaya (veri bloğuna) erişim isteği.
bool MemoryManager::access_page(int page_id) {
  auto it = std::find(loaded_pages.begin(), loaded_pages.end(), page_id);

  // 1. ADIM: PAGE HIT (Sayfa RAM'de var)
  if (it != loaded_pages.end()) {
    std::cout << "[Memory] PAGE HIT: Sayfa-" << page_id << " RAM'de mevcut."
              << std::endl;

    // --- GELİŞMİŞ TASARIM (ENHANCED DESIGN): LRU GÜNCELLEMESİ ---
    // Madem bu sayfa şu an kullanıldı, artık 'En Eski' değil, 'En Yeni' sayfa
    // oldu! Bu yüzden onu eski konumundan siliyor ve vektörün EN SONUNA
    // (güvenli bölgeye) taşıyoruz.
    loaded_pages.erase(it);
    loaded_pages.push_back(page_id);

    std::cout
        << "         -> [LRU] Sayfa-" << page_id
        << " 'En Yeni Kullanilan' olarak isaretlendi (Silinmekten kurtuldu)."
        << std::endl;
    return true;
  }
  // 2. ADIM: PAGE FAULT (Sayfa RAM'de yok, diskten gelecek)
  else {
    std::cout << "[Memory] PAGE FAULT: Sayfa-" << page_id
              << " RAM'de bulunamadi! Diskten getiriliyor..." << std::endl;

    // RAM tamamen doluysa, en eskiyi kovmak için fonksiyonu çağır.
    if (loaded_pages.size() >= ram_capacity) {
      std::cout << "[Memory] UYARI: RAM tamamen dolu! Yeni sayfa icin yer "
                   "acilmasi gerekiyor."
                << std::endl;
      evict_page();
    }

    // Yeni sayfayı RAM'in en sonuna (yani 'En Yeni' konumuna) ekliyoruz.
    loaded_pages.push_back(page_id);
    std::cout << "[Memory] Sayfa-" << page_id
              << " diskten RAM'e yuklendi ve 'En Yeni' olarak isaretlendi."
              << std::endl;
    return false;
  }
}

// RAM dolduğunda "En Az Yakın Zamanda Kullanılan" (Least Recently Used) sayfayı
// silen fonksiyon.
void MemoryManager::evict_page() {
  if (!loaded_pages.empty()) {
    // LRU algoritmamız sayesinde vektörün en başındaki eleman her zaman 'En Az
    // Bakılan' sayfadır.
    int evicted = loaded_pages.front();

    // O zavallı, unutulmuş sayfayı RAM'den siliyoruz.
    loaded_pages.erase(loaded_pages.begin());

    std::cout << "[Memory] EVICTION (LRU): En uzun suredir kullanilmayan Sayfa-"
              << evicted << " RAM'den silindi." << std::endl;
  }
}