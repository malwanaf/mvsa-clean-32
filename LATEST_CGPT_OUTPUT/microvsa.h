
#ifndef MICROVSA_H_
#define MICROVSA_H_

#include <stdint.h>
#include "microvsa_config.h"

uint8_t microvsa_run_single_inference(
    const uint8_t *processed_sample, int sample_length
#ifndef MICROVSA_IMPL_FIX_SIZE
    , const void *MODEL_F, const void *MODEL_V, const void *MODEL_C,
      int num_class, int num_feature, int fv_dim_word, int fv_dim_bit
#endif
);

#endif // MICROVSA_H_
