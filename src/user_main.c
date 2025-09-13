#include "user_main.h"
#include "benchmark_util.h"
#include "microvsa.h"
#include "model.h"
#include <stdio.h>
#include <stdint.h>

// 1. Sertakan header data uji untuk mengakses 'test_data_sample' dan 'TEST_DATA_SAMPLE_LENGTH'.
#include "test_data.h"

int user_main()
{
    uint8_t result = 0;
    TIMER_COUNTER_DTYPE runtime = 0;

    // Inisialisasi yang diperlukan (diasumsikan ada)
    // uart_initialize(); // Jika Anda ingin output melalui UART
    timer_initialize();

    // 2. Hapus loop `while(1)` agar program hanya berjalan sekali.
    
    printf("Menjalankan satu kali inferensi...\n");

    timer_reset();

    // 3. Panggil fungsi inferensi secara langsung dengan data dari 'test_data_har_64.c'.
    //    - 'test_data_sample' adalah array global dari file data uji. 
    //      Secara default, ini akan menggunakan sampel pertama (561 byte pertama).
    //    - 'TEST_DATA_SAMPLE_LENGTH' adalah panjang sampel yang didefinisikan di header.
    result = microvsa_run_single_inference(
        test_data_sample, 
        TEST_DATA_SAMPLE_LENGTH
#ifndef MICROVSA_IMPL_FIX_SIZE
        , MICROVSA_MODEL_F, MICROVSA_MODEL_V, MICROVSA_MODEL_C, 
        MICROVSA_MODEL_NUM_CLASS, MICROVSA_MODEL_NUM_FEATURE, 
        MICROVSA_MODEL_FHV_DIMENSION_WORD, MICROVSA_MODEL_FHV_DIMENSION_BIT
#endif
    );
            
    runtime = timer_elapsed_cycle();

    // 4. Cetak hasil prediksi dan waktu eksekusi ke konsol standar.
    //    Fungsi 'uart_send_result' diganti dengan 'printf' agar lebih umum.
    printf("----------------------------------------\n");
    printf("Hasil Prediksi: %d\n", result);
    printf("Runtime: %llu siklus\n", (unsigned long long)runtime);
    printf("----------------------------------------\n");

    // Bandingkan dengan hasil sebenarnya (opsional)
    // Nilai 'test_data_actual[0]' dari test_data_har_64.c adalah '3'
    printf("Hasil Sebenarnya (untuk sampel pertama): %d\n", test_data_actual[0]);
    if (result == test_data_actual[0]) {
        printf("Prediksi BENAR.\n");
    } else {
        printf("Prediksi SALAH.\n");
    }

    // 5. Program selesai setelah satu kali eksekusi.
    return 0;
}
