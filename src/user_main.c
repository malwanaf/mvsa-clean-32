#include "user_main.h"
#include "benchmark_util.h"
#include "microvsa.h"
#include "model.h"
#include "test_data.h"
#include "model_params_scaled.h"

#include <stdio.h>
#include <stdint.h>

// Preprocessing + inference
int user_main()
{
    uint8_t result = 0;
    TIMER_COUNTER_DTYPE runtime = 0;

    uint8_t processed_sample[TEST_DATA_SAMPLE_LENGTH];

    // Normalisasi + quantisasi
    for (int i = 0; i < TEST_DATA_SAMPLE_LENGTH; i++) {
        int32_t raw_scaled = (int32_t)(test_data_sample[i] * FIXED_POINT_SCALE_FACTOR);
        int32_t temp = raw_scaled - min_val_scaled[i];
        int64_t norm_64 = (int64_t)temp * inv_range_val_scaled[i];
        int32_t norm_scaled = (int32_t)(norm_64 >> 12);

        int32_t quantized_value = (norm_scaled * (MICROVSA_MODEL_NUM_QUANT - 1) + (FIXED_POINT_SCALE_FACTOR / 2));
        quantized_value = quantized_value >> 12;

        if (quantized_value < 0) {
            processed_sample[i] = 0;
        } else if (quantized_value >= MICROVSA_MODEL_NUM_QUANT) {
            processed_sample[i] = MICROVSA_MODEL_NUM_QUANT - 1;
        } else {
            processed_sample[i] = (uint8_t)quantized_value;
        }
    }

    // Inference
    timer_initialize();
    printf("Menjalankan satu kali inferensi dengan fixed-point...\n");
    timer_reset();

    result = microvsa_run_single_inference(
        processed_sample,
        TEST_DATA_SAMPLE_LENGTH,
        MICROVSA_MODEL_F,
        MICROVSA_MODEL_V,
        MICROVSA_MODEL_C,
        MICROVSA_MODEL_NUM_CLASS,
        MICROVSA_MODEL_NUM_FEATURE,
        MICROVSA_MODEL_FHV_DIMENSION_WORD,
        MICROVSA_MODEL_FHV_DIMENSION_BIT
    );

    runtime = timer_elapsed_cycle();

    // Print hasil
    printf("----------------------------------------\n");
    printf("Hasil Prediksi  : %d\n", result);
    printf("Label Sebenarnya: %d\n", test_data_actual_label);
    printf("Status          : %s\n", (result == test_data_actual_label) ? "BENAR" : "SALAH");
    printf("Runtime         : %llu siklus\n", (unsigned long long)runtime);
    printf("----------------------------------------\n");

    return 0;
}
