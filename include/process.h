#ifndef PROCESS_H // Eğer bu başlık dosyası projede birden fazla kez çağrılırsa,
                  // kodların mükerrer (çift) tanımlanmasını engeller. (Include
                  // Guard)
#define PROCESS_H // Derleyiciye bu dosyanın işlendiğini bildirir. C++
                  // projelerindeki standart bir güvenlik önlemidir.

#include <string> // Süreç isimlerini (örn: "DB_Query_1") metin (string) olarak tutabilmek için standart string kütüphanesini ekliyoruz.
#include <vector> // Bir sürecin elinde tuttuğu birden fazla kilit (lock) olabileceği için, bunları dinamik bir dizide (vector) tutmak adına ekliyoruz.

// İşletim sistemindeki bir sürecin (Process) geçebileceği yaşam döngüsü
// durumlarını (States) tanımlıyoruz. İşletim sistemleri dersindeki 5 durumlu
// (5-State) süreç modelinin tam simülasyonudur.
enum class ProcessState {
  NEW, // Süreç yeni oluşturuldu, ancak henüz RAM'e veya Scheduler (Zamanlayıcı)
       // kuyruğuna tam olarak alınmadı.
  READY,   // Süreç CPU'da çalışmaya hazır, Scheduler'ın (Zamanlayıcı) onu seçip
           // CPU'ya atamasını bekliyor.
  RUNNING, // Süreç şu an CPU üzerinde aktif olarak işlemlerini (örneğin
           // veritabanı sorgusunu) gerçekleştiriyor.
  BLOCKED, // Süreç bir I/O işlemi (diske yazma) veya bir veritabanı tablosu
           // kilidi (lock) beklediği için uykuya alındı.
  TERMINATED // Süreç işini bitirdi veya Deadlock (Kördüğüm) dedektörü
             // tarafından zorla öldürüldü. OS, bellekleri bu aşamada geri alır.
};

// Süreç Kontrol Bloğu (Process Control Block - PCB)
// OS'in bir süreci yönetmek için kullandığı "Kimlik Kartı". Bağlam geçişlerinde
// (Context Switch) bu bilgiler kaydedilip geri yüklenir.
class Process {
public: // Scheduler ve Memory Manager gibi diğer alt sistemler bu verilere
        // hızlıca erişebilsin diye erişimi public (açık) bırakıyoruz.
  // 1. TEMEL KİMLİK BİLGİLERİ (Identification)
  int process_id; // OS'in süreci tanıdığı benzersiz işlem numarası (PID -
                  // Process ID). Arama işlemlerinde anahtar rol oynar.
  std::string
      name; // Sürecin okunabilir ismi (Örn: "Transaction_A"). Projedeki
            // "Observability & Logging" zorunluluğu için loglarda kullanılacak.

  // 2. ÇİZELGELEME (Scheduling) BİLGİLERİ
  ProcessState state; // Sürecin mevcut durumu (READY, BLOCKED vb.). Scheduler
                      // sadece READY durumundakileri CPU'ya alabilir.
  int priority; // Sürecin öncelik değeri. Deadlock çözümünde "hangi süreci
                // öldürelim?" kararını verirken düşük öncelikliği kurban etmek
                // için kullanacağız.
  int cpu_burst_time; // Sürecin CPU'da toplam ne kadar işlem yapması gerektiği
                      // (Simüle edilmiş çalışma süresi).

  // 3. EŞZAMANLILIK VE KÖRDÜĞÜM (Concurrency & Deadlock) BİLGİLERİ (Projemizin
  // asıl mühendislik zorluğu için)
  int waiting_for_resource; // Sürecin çalışmaya devam etmek için beklediği
                            // kaynak ID'si. Eğer -1 ise hiçbir şey beklemiyor
                            // (özgür) demektir.
  std::vector<int>
      held_resources; // Sürecin şu an elinde tuttuğu (başkasının kullanmasını
                      // engellediği) kaynakların (Örn: tabloların) listesi.

  // Kurucu fonksiyon (Constructor): İşletim sistemine yeni bir süreç
  // eklendiğinde ilk varsayılan değerlerini atamak için çalışır.
  Process(int id, std::string n, int prio, int burst) {
    process_id = id;        // Dışarıdan gelen ID numarası atanıyor.
    name = n;               // Dışarıdan gelen isim atanıyor.
    priority = prio;        // Öncelik değeri belirleniyor.
    cpu_burst_time = burst; // Sürecin simüle edilecek işlem yükü belirleniyor.
    state =
        ProcessState::NEW; // Her süreç işletim sistemi teorisi gereği
                           // doğduğunda ilk olarak NEW (Yeni) durumunda başlar.
    waiting_for_resource = -1; // -1 atayarak başlangıçta hiçbir veritabanı
                               // kaynağını beklemediğini ifade ediyoruz.
  }
}; // Sınıf (class) tanımının sonu. C++ kuralları gereği noktalı virgül ile
   // bitmek zorundadır.

#endif // PROCESS_H tanımının sonlandırılması.