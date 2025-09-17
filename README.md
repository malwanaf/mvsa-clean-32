# MicroVSA: Alur Kerja End-to-End untuk Model VSA/HDC pada Mikrokontroler

Proyek ini mendemonstrasikan alur kerja lengkap untuk melatih model *Vector Symbolic Architecture* (VSA), yang juga dikenal sebagai *Hyperdimensional Computing* (HDC), dan mengimplementasikannya pada mikrokontroler (seperti ESP32) untuk inferensi yang sangat efisien.

Alur kerja ini menggunakan **Python** dengan **PyTorch** untuk melatih model dan secara otomatis menghasilkan **kode C** yang siap pakai untuk di-deploy.

---

## 🏛️ Struktur Proyek

```
.
├── notebooks/
│   └── NEWCGPT_WORK_CLEAN.ipynb      # Notebook utama untuk training & ekspor
├──  src/                          
│    ├── microvsa.c
│    ├── microvsa.h
│    ├── user_main.c
│    └── ...
└── README.md                       
```

---

## 🚀 Alur Kerja Saat Ini

Alur kerja saat ini berpusat pada satu notebook Jupyter yang menangani semua langkah, mulai dari pemrosesan data hingga pembuatan kode.

### **Langkah 1: Persiapan & Kebutuhan**

1.  **Instal Dependensi Python: Menggunakan UV**
    ```bash
    mvsa-clean-32 $ uv venv (membuat virtual env)
    mvsa-clean-32 $ .venv/Scripts/activate (windows)
    mvsa-clean-32 $ uv pip install platformio (atau) uv add platformio
    ```

### **Langkah 2: Latih Model & Hasilkan Kode C**

Semua proses ini dilakukan di dalam `notebooks/NEWCGPT_WORK_CLEAN.ipynb`.

1.  **Buka dan Jalankan Notebook:** Buka notebook menggunakan Jupyter Lab atau Jupyter Notebook.
2.  **(SEL 0) Unduh Dataset:** Jalankan sel ini untuk mengunduh dataset UCI HAR. Ini hanya perlu dilakukan sekali.
3.  **(SEL 1) Latih Model:**
    * Sel ini berisi logika pelatihan. Anda dapat mengubah **hyperparameter** utama di sini:
        * `NUM_QUANT`: Jumlah level kuantisasi fitur (misalnya 64, 128, 256). Mempengaruhi ukuran matriks `V`.
        * `HV_DIM`: Dimensi hypervector (misalnya 128, 256, 512). Mempengaruhi ukuran semua matriks (F, V, C) dan beban komputasi.
        * `WORD`: Ukuran word target (misalnya 8, 16, 32). Sesuaikan dengan arsitektur target (misalnya 32 untuk ESP32).
    * Setelah pelatihan selesai, sel ini akan menyimpan bobot model yang telah dibinarisasi ke file `ldc_trained_model_bits.npz`.
4.  **(SEL 2) Ekspor ke Kode C:**
    * Jalankan sel ini untuk memuat bobot dari file `.npz` dan menghasilkan file-file C berikut di direktori yang sama dengan notebook:
        * `model.c` & `model.h`: Berisi hypervector `F`, `V`, dan `C` sebagai array C.
        * `model_params_scaled.h`: Berisi parameter normalisasi dan kuantisasi.
        * `microvsa_config.h`: Berisi konfigurasi makro berdasarkan hyperparameter.
        * `test_data.c` & `test_data.h`: Satu sampel data uji untuk validasi.
5.  **(SEL 3) Validasi Cepat (Opsional):** Sel ini melakukan simulasi inferensi di Python untuk memastikan logika model sudah benar sebelum beralih ke C.

### **Langkah 3: Kompilasi dan Jalankan pada Target**

1.  **Salin File yang Dihasilkan:** Pindahkan semua file `.c` dan `.h` yang baru dibuat dari direktori notebook ke dalam direktori `src/` proyek PlatformIO Anda, timpa file yang ada jika perlu.
2.  **Kompilasi dan Unggah:** Dari dalam direktori proyek PlatformIO Anda, jalankan perintah:
    ```bash
    # Ganti 'featheresp32' dengan lingkungan target Anda
    microvsa-clean-32 $ pio run -e featheresp32 --target upload --monitor
    microvsa-clean-32 $ pio run -e featheresp32 -t clean (clean run)
    microvsa-clean-32 $ pio run -e featheresp32 (upload firmware manual)
    ```
3.  **Lihat Hasilnya:** Buka monitor serial untuk melihat output inferensi, termasuk prediksi, label sebenarnya, dan runtime dalam siklus CPU.

    ```
    Menjalankan satu kali inferensi dengan fixed-point...
    ----------------------------------------
    Hasil Prediksi  : 4
    Label Sebenarnya: 4
    Status          : BENAR
    Runtime         : 28230 siklus
    ----------------------------------------
    ```

---

## 💡 Pengembangan Alur Kerja (Lebih Modular & Skalabel)

Meskipun alur kerja saat ini berfungsi dengan baik, ia dapat ditingkatkan agar lebih mudah dikelola, dikustomisasi, dan diotomatisasi. Berikut adalah usulan struktur dan alur kerja yang lebih baik.

### **Struktur Proyek yang Diusulkan:**

```
.
├── config/
│   └── uci_har_config.yaml     # File konfigurasi untuk eksperimen
├── data/                         # Direktori untuk dataset
├── src_c/                        # Kode sumber C untuk mikrokontroler
│   ├── microvsa.c
│   └── ...
├── src_py/                       # Skrip-skrip Python yang modular
│   ├── dataloader.py
│   ├── train.py
│   ├── export.py
│   └── model.py
└── requirements.txt              # Dependensi Python
```

### **Alur Kerja Baru yang Diusulkan:**

Alih-alih satu notebook besar, kita memecah fungsionalitas menjadi skrip-skrip terpisah yang dikendalikan oleh file konfigurasi.

**Langkah 1: Konfigurasi Eksperimen**

Buat file `config/uci_har_config.yaml` untuk mendefinisikan semua parameter. Ini menjadi satu-satunya tempat yang perlu Anda ubah saat bereksperimen.

```yaml
# config/uci_har_config.yaml
dataset:
  name: "UCI_HAR"
  path: "./data/UCI_HAR_Dataset"

model:
  hv_dim: 256
  num_quant_levels: 256
  word_size: 32

training:
  epochs: 5
  batch_size: 64
  learning_rate: 0.001

export:
  output_dir: "./src_c/generated" # Direktori output untuk file C
```

**Langkah 2: Latih Model (via Skrip)**

Jalankan skrip `train.py` dari terminal. Skrip ini akan membaca konfigurasi, melatih model, dan menyimpan artefak (bobot model dan parameter kuantisasi) di direktori `models/`.

```bash
python src_py/train.py --config config/uci_har_config.yaml
```

**Langkah 3: Ekspor ke Kode C (via Skrip)**

Jalankan skrip `export.py`. Skrip ini akan memuat artefak model yang sudah dilatih dan menghasilkan file C langsung ke direktori yang ditentukan dalam file konfigurasi.

```bash
python src_py/export.py --config config/uci_har_config.yaml
```

**Langkah 4: Kompilasi**

Karena file-file C sudah berada di lokasi yang benar (`src_c/generated` yang dapat di-link oleh PlatformIO), Anda bisa langsung mengompilasi proyek.

```bash
cd pio_project/
pio run -e featheresp32
```

### **Keuntungan Alur Kerja Baru:**

1.  **Modularitas:** Setiap skrip memiliki satu tanggung jawab (memuat data, melatih, mengekspor). Ini membuat kode lebih mudah dibaca dan dipelihara.
2.  **Kustomisasi Mudah:** Untuk melatih dataset baru atau mengubah arsitektur, Anda hanya perlu memodifikasi file konfigurasi atau skrip `dataloader.py`, tanpa menyentuh logika utama.
3.  **Otomatisasi:** Alur kerja berbasis skrip ini dapat dengan mudah diotomatisasi dalam pipeline CI/CD atau skrip *batch* untuk menjalankan banyak eksperimen secara otomatis.
4.  **Reproducibility:** Menggunakan file konfigurasi memastikan bahwa setiap eksperimen dapat direproduksi dengan tepat hanya dengan menggunakan file `config` yang sama.

Dengan mengadopsi struktur ini, proyek Anda akan jauh lebih siap untuk dikembangkan lebih lanjut, baik untuk mencoba dataset baru, arsitektur model yang berbeda, maupun target perangkat keras yang beragam.


# Alur Kerja Aplikasi Inferensi Real-Time untuk MicroVSA

Dokumen ini menjelaskan langkah-langkah dan konsep untuk mengubah bukti-konsep (proof-of-concept) inferensi tunggal menjadi aplikasi *Human Activity Recognition* (HAR) yang berjalan secara real-time, terus-menerus memproses data dari sensor pada mikrokontroler.

---

### Konsep Dasar: Dari Inferensi Tunggal ke Loop Real-Time

Aplikasi real-time tidak lagi menggunakan sampel data statis yang disimpan di `test_data.c`. Sebaliknya, ia beroperasi dalam sebuah **loop tak terbatas** yang secara kontinu menjalankan siklus berikut:

1.  **Akuisisi Data:** Mengumpulkan data mentah dari sensor (misalnya, akselerometer dan giroskop) secara berkelanjutan.
2.  **Windowing/Buffering:** Menyimpan data sensor dalam sebuah *buffer* hingga jumlah data yang cukup terkumpul untuk membentuk satu "jendela" (window) analisis.
3.  **Ekstraksi Fitur:** Mengubah data sensor mentah dari *window* tersebut menjadi vektor fitur numerik yang dapat dipahami oleh model. **Ini adalah langkah paling krusial yang perlu diimplementasikan.**
4.  **Preprocessing & Inferensi:** Melakukan normalisasi dan kuantisasi pada vektor fitur, lalu menjalankannya melalui model MicroVSA untuk mendapatkan prediksi aktivitas.
5.  **Aksi/Output:** Menampilkan hasil prediksi (misalnya, mencetak ke Serial Monitor, menampilkan di LCD) atau memicu aksi lain berdasarkan hasil tersebut.

!(https://i.imgur.com/9yI4eUj.png)

---

### 🧠 Langkah Kritis: Implementasi Ekstraksi Fitur

Sangat penting untuk dipahami bahwa model yang dilatih di notebook **tidak** menggunakan data sensor mentah (seperti nilai akselerasi X, Y, Z) secara langsung. Model ini dilatih menggunakan **561 fitur** yang telah dihitung sebelumnya dari data mentah tersebut.

* **Tugas Utama Anda:** Anda perlu menulis kode dalam C/C++ untuk menghitung ke-561 fitur ini pada mikrokontroler Anda.
* **Sumber Referensi:** Deskripsi lengkap tentang bagaimana setiap fitur dihitung dapat ditemukan dalam file `features_info.txt` yang ada di dalam dataset UCI HAR asli. Fitur-fitur ini mencakup berbagai perhitungan statistik dan sinyal, seperti:
    * Rata-rata (mean), standar deviasi, median.
    * *Fast Fourier Transform* (FFT) untuk analisis domain frekuensi.
    * Dan banyak lainnya.

Saat ini, bagian ekstraksi fitur ini **belum ada** di dalam kode C yang disediakan dan harus dibuat dari awal agar aplikasi real-time dapat berfungsi.

---

### 💻 Struktur Kode yang Diusulkan untuk Aplikasi Real-Time

Berikut adalah contoh kerangka kode (pseudocode) untuk implementasi pada platform yang menggunakan `setup()` dan `loop()` seperti Arduino atau ESP-IDF.

#### **1. Deklarasi Global dan Buffer**

```c
#include "microvsa.h"
#include <Arduino.h> // atau header spesifik platform lainnya
// #include "YourIMUSensorLibrary.h" // Library untuk sensor Anda

// --- Konfigurasi Sensor & Buffering ---
const int WINDOW_SIZE = 128; // Jumlah sampel data mentah per window (sesuaikan dengan dataset UCI HAR)
int sample_count = 0;
// Buffer untuk 6-axis (AccX, AccY, AccZ, GyroX, GyroY, GyroZ)
float sensor_buffer[WINDOW_SIZE][6];

// --- Vektor Fitur & Sampel Terkuantisasi ---
// Vektor untuk menampung hasil ekstraksi fitur (561 fitur)
float feature_vector[MICROVSA_MODEL_NUM_FEATURE];
uint8_t quantized_sample[MICROVSA_MODEL_NUM_FEATURE];

// Label aktivitas untuk mempermudah pembacaan output
const char* activity_labels[MICROVSA_MODEL_NUM_CLASS] = {
    "WALKING", "WALKING_UPSTAIRS", "WALKING_DOWNSTAIRS",
    "SITTING", "STANDING", "LAYING"
};

// Deklarasi fungsi helper yang perlu Anda implementasikan
void extract_features(float buffer[WINDOW_SIZE][6], float out_features[]);
```

#### **2. Fungsi `setup()`**

Fungsi ini berjalan sekali saat perangkat dinyalakan untuk melakukan inisialisasi.

```c
void setup() {
    Serial.begin(115200);
    // Inisialisasi sensor IMU Anda (misalnya MPU-6050)
    // initialize_sensor();
    Serial.println("Sistem HAR Real-time Siap.");
}
```

#### **3. Fungsi `loop()` Utama**

Fungsi ini berjalan terus-menerus setelah `setup()` selesai.

```c
void loop() {
    // 1. Akuisisi Data Sensor
    // Baca data terbaru dari sensor IMU Anda
    float ax, ay, az, gx, gy, gz;
    // read_sensor_data(&ax, &ay, &az, &gx, &gy, &gz); // Ganti dengan fungsi dari library sensor Anda

    // 2. Simpan ke Buffer (Windowing)
    sensor_buffer[sample_count][0] = ax;
    sensor_buffer[sample_count][1] = ay;
    sensor_buffer[sample_count][2] = az;
    sensor_buffer[sample_count][3] = gx;
    sensor_buffer[sample_count][4] = gy;
    sensor_buffer[sample_count][5] = gz;
    sample_count++;

    // 3. Jika Buffer Penuh, Lakukan Inferensi
    if (sample_count >= WINDOW_SIZE) {
        // Langkah 3a: Ekstraksi Fitur (FUNGSI INI HARUS ANDA BUAT)
        // Fungsi ini akan memproses 'sensor_buffer' dan mengisi 'feature_vector'
        extract_features(sensor_buffer, feature_vector);

        // Langkah 3b: Preprocessing & Kuantisasi (logika dari user_main.c)
        for (int i = 0; i < MICROVSA_MODEL_NUM_FEATURE; i++) {
            // Salin dan adaptasi logika normalisasi dan kuantisasi dari user_main.c
            // untuk setiap elemen di 'feature_vector' dan simpan hasilnya di 'quantized_sample'
            int32_t raw_scaled = (int32_t)(feature_vector[i] * FIXED_POINT_SCALE_FACTOR);
            int32_t temp = raw_scaled - min_val_scaled[i];
            int64_t norm_64 = (int64_t)temp * inv_range_val_scaled[i];
            int32_t norm_scaled = (int32_t)(norm_64 >> 12);
            int32_t quantized_value = (norm_scaled * (MICROVSA_MODEL_NUM_QUANT - 1) + (FIXED_POINT_SCALE_FACTOR / 2)) >> 12;

            if (quantized_value < 0) quantized_sample[i] = 0;
            else if (quantized_value >= MICROVSA_MODEL_NUM_QUANT) quantized_sample[i] = MICROVSA_MODEL_NUM_QUANT - 1;
            else quantized_sample[i] = (uint8_t)quantized_value;
        }

        // Langkah 3c: Jalankan Inferensi MicroVSA
        uint8_t result_index = microvsa_run_single_inference(
            quantized_sample, MICROVSA_MODEL_NUM_FEATURE,
            MICROVSA_MODEL_F, MICROVSA_MODEL_V, MICROVSA_MODEL_C,
            MICROVSA_MODEL_NUM_CLASS, MICROVSA_MODEL_NUM_FEATURE,
            MICROVSA_MODEL_FHV_DIMENSION_WORD, MICROVSA_MODEL_FHV_DIMENSION_BIT
        );

        // Langkah 3d: Tampilkan Hasil
        Serial.print("Aktivitas Terdeteksi: ");
        Serial.println(activity_labels[result_index]);

        // Langkah 3e: Reset Buffer untuk memulai window berikutnya
        // (Opsional: Anda bisa menggunakan overlapping window dengan memindahkan data lama)
        sample_count = 0;
    }

    // Beri jeda singkat agar sesuai dengan frekuensi sampling (misal, 50Hz)
    delay(20);
}
```

---

### **🔌 Kebutuhan Perangkat Keras**

Untuk membangun aplikasi ini, Anda akan memerlukan:

* **Mikrokontroler:** **ESP32** adalah pilihan yang sangat baik karena memiliki CPU dual-core yang cepat dan memori yang cukup. Platform lain seperti Arduino Nano 33 BLE Sense (dengan IMU terintegrasi) juga bisa digunakan.
* **Sensor:** Sebuah **Inertial Measurement Unit (IMU)** 6-axis, seperti **MPU-6050** atau **LSM6DS3**, untuk mendapatkan data akselerometer (3-axis) dan giroskop (3-axis).
* **Perangkat Tambahan:** Kabel jumper dan breadboard untuk menghubungkan sensor ke mikrokontroler jika tidak terintegrasi.

Dengan mengikuti panduan ini, Anda dapat mengembangkan proyek Anda dari sebuah validasi model menjadi aplikasi cerdas yang mampu mengenali aktivitas manusia secara terus-menerus di dunia nyata.
