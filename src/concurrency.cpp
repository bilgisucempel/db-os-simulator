#include "../include/concurrency.h" // Kendi tanımladığımız eşzamanlılık başlık dosyasını çekiyoruz.

ConcurrencyManager::ConcurrencyManager() {
} // Kurucu fonksiyon, başlangıç için özel bir atama gerektirmiyor.

// Bir sürecin bir kaynağı (veritabanı tablosunu) kilitleme talebi.
bool ConcurrencyManager::acquire_lock(Process *p, int resource_id) {
  // 1. Adım: Kaynak (Tablo) boşta mı kontrol et.
  // resource_locks.find() fonksiyonu haritada bu kaynağı arar. Eğer '.end()'
  // dönerse kaynak kilitli değildir.
  if (resource_locks.find(resource_id) == resource_locks.end()) {

    // Kaynak boş. Hemen kilidi bu sürece veriyoruz.
    resource_locks[resource_id] = p;
    p->held_resources.push_back(
        resource_id); // Sürecin "elimdeki kaynaklar" listesine ekliyoruz.
    p->waiting_for_resource =
        -1; // Süreç hiçbir şeyi beklemiyor, çünkü kaynağı aldı.

    // Sistemin davranışı loglanıyor (Observability).
    std::cout << "[Concurrency] " << p->name << " sureci Kaynak-" << resource_id
              << " icin kilit (lock) aldi." << std::endl;
    return true; // Kilit başarıyla alındı.

  } else {
    // 2. Adım: Kaynak başkasının elinde (Kilitli).
    Process *owner =
        resource_locks[resource_id]; // Kaynağın şu anki sahibini buluyoruz.

    // Sürecin durumunu 'Çalışıyor'dan 'Engellendi' (BLOCKED) durumuna çekiyoruz
    // (Context Switch için hazırlık).
    p->state = ProcessState::BLOCKED;
    p->waiting_for_resource =
        resource_id; // Sürecin bu kaynağı beklediğini sisteme kaydediyoruz
                     // (Deadlock tespiti için hayati önem taşır).

    std::cout << "[Concurrency] UYARI: " << p->name << " sureci Kaynak-"
              << resource_id << " icin kilit ISTEDI ama kaynak " << owner->name
              << " tarafindan kullaniliyor. Surec BLOCKED durumuna gecti!"
              << std::endl;
    return false; // Kilit alınamadı, süreç uyumalı.
  }
}

// Sürecin kullandığı kaynağı geri bırakması.
void ConcurrencyManager::release_lock(Process *p, int resource_id) {
  resource_locks.erase(
      resource_id); // Kaynağı kilit haritasından siliyoruz (Kilidi açıyoruz).

  // Sürecin kendi içindeki "elindeki kaynaklar" (held_resources) listesinden bu
  // kaynağı siliyoruz.
  for (auto it = p->held_resources.begin(); it != p->held_resources.end();
       ++it) {
    if (*it == resource_id) {
      p->held_resources.erase(it);
      break;
    }
  }
  std::cout << "[Concurrency] " << p->name << " sureci Kaynak-" << resource_id
            << " kilidini serbest birakti." << std::endl;
}

// Projemizin "Engineering Challenge" çözümü: Deadlock Tespiti (Kördüğüm
// Dedektörü) Mantık: Süreç A, Süreç B'nin elindeki kaynağı bekliyorsa ve Süreç
// B de Süreç A'nın elindeki kaynağı bekliyorsa Deadlock vardır.
bool ConcurrencyManager::detect_deadlock(
    const std::vector<Process *> &all_processes) {
  // Sistemdeki tüm süreçleri tek tek tarıyoruz.
  for (Process *p : all_processes) {
    // Eğer bu süreç BLOCKED (Engellenmiş) durumdaysa ve bir kaynağı bekliyorsa
    // incelemeye al.
    if (p->state == ProcessState::BLOCKED && p->waiting_for_resource != -1) {

      // Sürecin beklediği kaynağın şu anki sahibini buluyoruz.
      Process *owner_of_resource = resource_locks[p->waiting_for_resource];

      // Eğer kaynağın sahibi de BLOCKED durumdaysa ve O DA bir kaynak
      // bekliyorsa...
      if (owner_of_resource != nullptr &&
          owner_of_resource->state == ProcessState::BLOCKED) {

        // ...ve o beklediği kaynak ilk sürecin (p) elindeyse, DÖNGÜ (Cyclic
        // Wait) vardır!
        for (int held_res : p->held_resources) {
          if (owner_of_resource->waiting_for_resource == held_res) {

            // Hocanın istediği 'Failure Scenario' ve 'Engineering Challenge'
            // tespiti anı:
            std::cout << "\n[DEADLOCK DEDEKTORU] KRITIK HATA: Kordugum "
                         "(Deadlock) tespit edildi!"
                      << std::endl;
            std::cout << "-> " << p->name << " ile " << owner_of_resource->name
                      << " birbirlerini bekliyor." << std::endl;
            return true; // Deadlock var!
          }
        }
      }
    }
  }
  return false; // Döngüsel bekleme yok, sistem güvenli.
}