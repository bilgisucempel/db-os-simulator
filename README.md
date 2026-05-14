# Database Server OS Simulator

C++ ile geliştirilmiş, **veritabanı sunucusu** mimarisini temel alan bir işletim sistemi simülatörü. Süreç zamanlama, bellek yönetimi, I/O bloklama ve dosya çakışma senaryolarını uçtan uca simüle eder.

---

## Özellikler

| Modül | Detay |
|---|---|
| **Süreç Yönetimi** | 5-State Model (NEW → READY → RUNNING → BLOCKED → TERMINATED) |
| **FIFO Scheduler** | I/O-Aware, dosya çakışması ve Page Fault desteği |
| **Round Robin Scheduler** | Quantum tabanlı preemptive, I/O-Aware |
| **Bellek Yönetimi** | LRU Page Replacement, Page Fault → Process BLOCKED |
| **Dosya Sistemi** | Bekleme kuyruğu (wait queue), dosya çakışması, otomatik uyanma |
| **Concurrency** | Deadlock tespiti ve çözümü |

---

## Proje Yapısı

```
db-os-simulator/
├── include/
│   ├── process.h        # Process struct, 5-State Model, tick_io()
│   ├── scheduler.h      # Scheduler sınıfı (FIFO + Round Robin)
│   ├── filesystem.h     # FileSystem + IOWaitEntry + wait_queue
│   ├── memory.h         # MemoryManager, LRU, PAGE_FAULT_IO_MS
│   └── concurrency.h    # Deadlock Detection
├── src/
│   ├── scheduler.cpp    # run_with_io() + run_round_robin()
│   ├── filesystem.cpp   # read_file / write_file / release_file
│   ├── memory.cpp       # access_page() — Page Fault bloklama
│   ├── concurrency.cpp  # Deadlock algoritması
│   ├── scenario_fifo.cpp  ← Senaryo 1: FIFO çalıştırılabilir
│   └── scenario_rr.cpp    ← Senaryo 2: Round Robin çalıştırılabilir
└── CMakeLists.txt
```

---

## Derleme ve Çalıştırma

```bash
# Proje dizininde:
cmake -B build -S .
cmake --build build

# Senaryo 1: FIFO + Page Fault + Dosya Çakışması
.\build\Debug\scenario_fifo.exe

# Senaryo 2: Round Robin + I/O Blocking
.\build\Debug\scenario_rr.exe
```

---

## Sistem Mimarisi

```mermaid
graph TD
    subgraph Kernel["Database OS Simulator — Kernel"]
        BOOT[Main / Boot] --> SCH[Scheduler]
        BOOT --> MM[Memory Manager]
        BOOT --> FS[File System]
        BOOT --> CM[Concurrency Manager]

        SCH -->|"add_process()"| RQ([Ready Queue])
        RQ -->|"FIFO / Round Robin"| CPU[CPU]

        CPU -->|"Page erişimi"| MM
        CPU -->|"read_file / write_file"| FS

        MM -->|"Page Hit → devam"| CPU
        MM -->|"Page Fault → BLOCKED"| BQ([Blocked Queue])

        FS -->|"Dosya boş → I/O başlat"| BQ
        FS -->|"Dosya meşgul → wait_queue"| WQ([File Wait Queue])
        WQ -->|"release_file() → otomatik uyanma"| BQ

        BQ -->|"tick_io() → io_wait_ms = 0"| RQ
    end

    subgraph HW["Simulated Hardware"]
        MM -.->|"Disk I/O"| DISK[(Database Disk)]
        CPU -.->|"Execution"| PROC[Process]
    end

    classDef core fill:#6c63ff,color:#fff,stroke:#333,stroke-width:2px;
    classDef queue fill:#f0a500,color:#000,stroke:#333;
    classDef hw fill:#2ec4b6,color:#fff,stroke:#333;
    class SCH,MM,FS,CM core;
    class RQ,BQ,WQ queue;
    class DISK,PROC hw;
```

---

## Process Durum Makinesi (5-State Model)

```mermaid
stateDiagram-v2
    [*] --> NEW
    NEW --> READY : add_process()

    READY --> RUNNING : Scheduler seçti (CPU verdi)

    RUNNING --> BLOCKED : I/O isteği\n(dosya / page fault)
    RUNNING --> READY   : Quantum doldu (Round Robin)
    RUNNING --> TERMINATED : CPU burst bitti

    BLOCKED --> READY : tick_io() → io_wait_ms = 0\n(I/O / Page Fault çözüldü)

    note right of BLOCKED
        io_wait_ms > 0  → Aktif I/O (tick sayar)
        io_wait_ms = 0  → Dosya kuyruğunda bekliyor
                          (release_file() uyandırır)
    end note

    TERMINATED --> [*]
```

---

## Senaryo 1: FIFO + Page Fault + Dosya Çakışması

**Gösterilen kavramlar:**
- Page Fault → Process BLOCKED (CPU boş kalmaz, sıradaki çalışır)
- Page Fault çözüldüğünde → Process READY (pending I/O korunur)
- Aynı dosyaya eş zamanlı erişim → dosya kuyruğu (wait_queue)
- `release_file()` → kuyruktaki process otomatik I/O'ya başlar

```mermaid
sequenceDiagram
    participant SCH as Scheduler
    participant p1 as DB_Query_Read (PID 1)
    participant p2 as DB_Query_Write (PID 2)
    participant p3 as DB_Backup (PID 3)
    participant p4 as DB_Audit (PID 4)
    participant MM as Memory Manager
    participant FS as File System

    SCH->>p1: CPU ver (RUNNING)
    p1->>MM: access_page(10)
    MM-->>p1: PAGE FAULT → BLOCKED (3000ms)
    SCH->>p4: Context Switch → CPU ver (RUNNING)
    p4->>FS: read_file("orders.db")
    FS-->>p4: ATANDI → BLOCKED (I/O 4000ms)

    SCH->>p2: Context Switch → CPU ver (RUNNING)
    p2->>FS: read_file("users.db")
    FS-->>p2: ATANDI → BLOCKED (I/O 4000ms)

    SCH->>p3: Context Switch → CPU ver (RUNNING)
    p3->>MM: access_page(20)
    MM-->>p3: PAGE FAULT → BLOCKED (3000ms)

    Note over SCH: tick_io() çalışıyor...
    p1-->>SCH: Page Fault çözüldü → READY (pending I/O korundu)
    SCH->>p1: CPU ver → read_file("users.db")
    FS-->>p1: MEŞGUL (p2 kullanıyor) → BLOCKED (wait_queue)

    p4-->>FS: orders.db I/O bitti
    FS-->>SCH: release_file("orders.db")
    p4-->>SCH: READY

    p2-->>FS: users.db I/O bitti
    FS-->>p1: KUYRUK: p1 sıradaydı → users.db atandı (I/O 4000ms)
    p2-->>SCH: READY
    p3-->>SCH: Page Fault çözüldü → READY

    p1-->>SCH: users.db I/O bitti → READY
    SCH->>p1: CPU ver → TERMINATED
```

---

## Senaryo 2: Round Robin + I/O Blocking

**Gösterilen kavramlar:**
- Quantum tabanlı CPU paylaşımı (preemptive)
- Quantum dolunca process kuyruğun sonuna gider
- I/O isteği → BLOCKED, CPU hemen sıradakine geçer
- I/O biten process kuyruğun **sonuna** eklenir (FIFO'dan farkı)

```mermaid
sequenceDiagram
    participant SCH as Scheduler (RR)
    participant p1 as DB_HeavyQuery (6000ms)
    participant p2 as DB_Report (3000ms)
    participant p3 as DB_Ping (1500ms)
    participant p4 as DB_Backup (4500ms)
    participant FS as File System

    Note over SCH: Quantum = 2000ms

    SCH->>p1: CPU (2000ms)
    p1-->>SCH: Quantum doldu → Kalan: 4000ms → Kuyruğun sonu

    SCH->>p2: CPU → I/O isteği "report.db"
    FS-->>p2: BLOCKED (I/O 4000ms)
    SCH->>p3: Context Switch → CPU (1500ms)
    p3-->>SCH: TERMINATED (burst bitti)

    SCH->>p4: CPU → I/O isteği "backup.db"
    FS-->>p4: BLOCKED (I/O 4000ms)

    Note over SCH: tick_io() → p2 I/O bitti
    p2-->>SCH: READY → Kuyruğun SONUNA (RR semantiği)

    SCH->>p1: CPU (2000ms) → Kalan: 2000ms → Kuyruğun sonu
    SCH->>p2: CPU (2000ms) → Kalan: 1000ms → Kuyruğun sonu

    Note over SCH: tick_io() → p4 I/O bitti
    p4-->>SCH: READY → Kuyruğun SONUNA

    SCH->>p1: CPU (2000ms) → TERMINATED
    SCH->>p2: CPU (1000ms) → TERMINATED
    SCH->>p4: CPU (2000ms) → Kalan: 2500ms → ...
    SCH->>p4: TERMINATED
```

---

## I/O Bloklama Mantığı

### Dosya Meşgul → Bekleme Kuyruğu

```mermaid
flowchart LR
    A[Process CPU'ya alındı] --> B{pending_io var?}
    B -- Evet --> C[fs.read_file / write_file]
    C --> D{Dosya boş mu?}
    D -- Evet --> E[file_in_use = PID\nio_wait_ms = 4000\nstate = BLOCKED]
    D -- Hayır --> F[wait_queue'ya ekle\nio_wait_ms = 0\nstate = BLOCKED]
    E --> G[blocked_files'a ekle]
    G --> H[tick_io azaltır\nio_wait_ms = 0 → READY]
    H --> I[release_file çağrılır]
    I --> J{wait_queue'da bekleyen?}
    J -- Evet --> K[Sıradakine dosya ata\nio_wait_ms set et\ntick_io takip eder]
    J -- Hayır --> L[Dosya tamamen boş]
    F --> M[Bekle\ntick_io çalışmaz]
    M --> K
    B -- Hayır --> N[cpu_burst = 0\nTERMINATED]
```

### Page Fault → Process Bloklama

```mermaid
flowchart LR
    A[Process CPU'ya alındı] --> B{pending_pages var?}
    B -- Evet --> C[mm.access_page]
    C --> D{RAM'de var mı?}
    D -- Evet / HIT --> E[Devam et\nI/O kontrolüne geç]
    D -- Hayır / FAULT --> F[RAM'e yükle\nio_wait_ms = 3000\nstate = BLOCKED]
    F --> G[Context Switch\nSıradaki process CPU'ya]
    G --> H[tick_io → io_wait_ms = 0]
    H --> I[READY kuyruğuna ekle\npending_io KORUNUR]
    I --> J[Bir sonraki CPU turunda\ndosya I/O yapacak]
    B -- Hayır --> K[I/O kontrol bloğuna geç]
```

---

## Temel Bileşenler

### `Process` (process.h)

```cpp
struct Process {
    int          process_id;
    std::string  name;
    int          priority;
    int          cpu_burst_time;
    int          io_wait_ms;     // > 0: aktif I/O  | = 0: dosya kuyruğunda
    ProcessState state;          // NEW/READY/RUNNING/BLOCKED/TERMINATED

    void tick_io(int elapsed_ms);  // io_wait_ms azalt; 0'a düşünce READY
};
```

### `FileSystem` (filesystem.h)

```cpp
class FileSystem {
    map<string, int>              file_in_use;   // filename → sahip PID
    map<string, queue<IOWaitEntry>> wait_queues; // filename → bekleyen processler

    void    read_file(Process* p, const string& filename);
    void    write_file(Process* p, const string& filename);
    Process* release_file(const string& filename); // → kuyruktaki process döner
};
```

### `MemoryManager` (memory.h)

```cpp
class MemoryManager {
    // Page Hit  → true  (process etkilenmez)
    // Page Fault→ false (process BLOCKED, io_wait_ms = PAGE_FAULT_IO_MS)
    bool access_page(int page_id, Process* p = nullptr);
    void evict_page(); // LRU: en eski sayfayı RAM'den çıkar
};
```

### `Scheduler` (scheduler.h)

```cpp
class Scheduler {
    void run_with_io(FileSystem& fs,
                     const map<int,string>& io_requests,
                     int tick_ms = 500,
                     MemoryManager* mm = nullptr,
                     const map<int,int>& page_requests = {});

    void run_round_robin(FileSystem& fs,
                         const map<int,string>& io_requests,
                         int quantum_ms = 2000,
                         int tick_ms = 500,
                         MemoryManager* mm = nullptr,
                         const map<int,int>& page_requests = {});
};
```

---

## FIFO vs Round Robin Karşılaştırması

| Özellik | FIFO (I/O-Aware) | Round Robin |
|---|---|---|
| CPU verme şekli | Burst tamamen biter | Quantum kadar verilir |
| Preemption | Sadece I/O'da | Quantum + I/O'da |
| I/O sonrası pozisyon | Kuyruğun önü | Kuyruğun **sonu** |
| Starvation riski | Uzun process öncelik alabilir | Tüm processler eşit pay alır |
| Page Fault | ✅ BLOCKED → READY | ✅ BLOCKED → READY (kuyruk sonu) |
| Dosya çakışması | ✅ wait_queue | ✅ wait_queue |

---

## Kullanılan OS Kavramları

- **5-State Process Model** — NEW, READY, RUNNING, BLOCKED, TERMINATED
- **Context Switch** — I/O / Page Fault / Quantum dolduğunda CPU'nun el değiştirmesi
- **I/O-Aware Scheduling** — CPU, I/O bekleyen süreçlerde boş kalmaz
- **Page Fault Handling** — Disk I/O simülasyonu, process bloklama ve uyanma
- **File Locking & Wait Queue** — Dosya bazlı kilitleme, FIFO bekleme kuyruğu
- **LRU Page Replacement** — En az kullanılan sayfayı RAM'den çıkar
- **Deadlock Detection** — Döngüsel bekleme tespiti ve victim seçimi
- **Tick-Based Simulation** — `tick_io()` ile gerçek thread uyutmadan süre simülasyonu