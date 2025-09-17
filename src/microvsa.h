#ifndef MICROVSA_H_
#define MICROVSA_H_

#include <stdint.h>
#include "model.h"          // ambil definisi MICROVSA_MODEL_DTYPE, hv_dim, dll
#include "microvsa_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Jalankan satu kali inferensi MicroVSA
 * 
 * @param sample            Array quantized input (panjang = num_features)
 * @param sample_length     Jumlah fitur (harus = MICROVSA_MODEL_NUM_FEATURE)
 * @param f                 Matrix F (feature HVs)
 * @param v                 Matrix V (value HVs)
 * @param c                 Matrix C (class HVs)
 * @param num_class         Jumlah kelas
 * @param num_feature       Jumlah fitur
 * @param fhv_dim_word      Dimensi hypervector dalam word
 * @param fhv_dim_bit       Dimensi hypervector dalam bit
 * @return                  Index kelas hasil prediksi
 */
uint8_t microvsa_run_single_inference(
      const uint8_t* __restrict__ sample,
      uint16_t sample_length,
      const MICROVSA_MODEL_DTYPE* __restrict__ f,
      const MICROVSA_MODEL_DTYPE* __restrict__ v,
      const MICROVSA_MODEL_DTYPE* __restrict__ c,
      uint16_t num_class,
      uint16_t num_feature,
      uint16_t fhv_dim_word,
      uint16_t fhv_dim_bit);

#ifdef __cplusplus
}
#endif

#endif // MICROVSA_H_
