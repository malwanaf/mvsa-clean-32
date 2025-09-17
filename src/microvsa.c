#include "microvsa.h"
#include <string.h>

// Utility: hamming weight untuk 32-bit word
static inline int popcount32(uint32_t x) {
    return __builtin_popcount(x);
}

// Utility: hamming weight untuk 16-bit word
static inline int popcount16(uint16_t x) {
    return __builtin_popcount(x);
}

// Utility: hamming weight untuk 8-bit word
static inline int popcount8(uint8_t x) {
    return __builtin_popcount(x);
}

uint8_t microvsa_run_single_inference(
      const uint8_t* __restrict__ sample,
      uint16_t sample_length,
      const MICROVSA_MODEL_DTYPE* __restrict__ f,
      const MICROVSA_MODEL_DTYPE* __restrict__ v,
      const MICROVSA_MODEL_DTYPE* __restrict__ c,
      uint16_t num_class,
      uint16_t num_feature,
      uint16_t fhv_dim_word,
      uint16_t fhv_dim_bit)
{
    // buffer akumulator hypervector hasil bundling
    MICROVSA_ACC_DTYPE acc[fhv_dim_word];
    memset(acc, 0, sizeof(acc));

    // Proses semua fitur
    for (uint16_t feat = 0; feat < sample_length; feat++) {
        uint8_t q = sample[feat];

        // Ambil V untuk (feature, q)
        const MICROVSA_MODEL_DTYPE* v_ptr = &v[(feat * MICROVSA_MODEL_NUM_QUANT + q) * fhv_dim_word];
        const MICROVSA_MODEL_DTYPE* f_ptr = &f[feat * fhv_dim_word];

        // Bind: XOR F dan V → lalu akumulasi
        for (uint16_t w = 0; w < fhv_dim_word; w++) {
            MICROVSA_MODEL_DTYPE hv = f_ptr[w] ^ v_ptr[w];

            // Akumulasi dengan +1 / -1 berdasarkan bit
            // (majority bundling)
#if MICROVSA_IMPL_WORDSIZE == 32
            acc[w] += popcount32(hv) - (32 - popcount32(hv));
#elif MICROVSA_IMPL_WORDSIZE == 16
            acc[w] += popcount16(hv) - (16 - popcount16(hv));
#elif MICROVSA_IMPL_WORDSIZE == 8
            acc[w] += popcount8(hv) - (8 - popcount8(hv));
#endif
        }
    }

    // Cari kelas terdekat (cosine similarity = dot product)
    int best_class = -1;
    int best_score = -32768;

    for (uint16_t cls = 0; cls < num_class; cls++) {
        const MICROVSA_MODEL_DTYPE* c_ptr = &c[cls * fhv_dim_word];
        int score = 0;

        for (uint16_t w = 0; w < fhv_dim_word; w++) {
#if MICROVSA_IMPL_WORDSIZE == 32
            score += popcount32(~(acc[w] ^ c_ptr[w])); 
#elif MICROVSA_IMPL_WORDSIZE == 16
            score += popcount16(~(acc[w] ^ c_ptr[w]));
#elif MICROVSA_IMPL_WORDSIZE == 8
            score += popcount8(~(acc[w] ^ c_ptr[w]));
#endif
        }

        if (score > best_score) {
            best_score = score;
            best_class = cls;
        }
    }

    return (uint8_t)best_class;
}
