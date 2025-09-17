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
