
#include "microvsa.h"
#include "microvsa_config.h"
#include <stdlib.h>
#include <string.h>

#if MICROVSA_IMPL_WORDSIZE == 8
typedef uint8_t mv_word_t;
#elif MICROVSA_IMPL_WORDSIZE == 16
typedef uint16_t mv_word_t;
#elif MICROVSA_IMPL_WORDSIZE == 32
typedef uint32_t mv_word_t;
#else
#error unsupported MICROVSA_IMPL_WORDSIZE
#endif

#ifdef MICROVSA_IMPL_FIX_SIZE
#include "model_ldc.h"  // default model to use (you can change compile-time include)
#endif

static inline int popcount_word(mv_word_t x) {
    #ifdef __GNUC__
    return __builtin_popcount((unsigned)x);
    #else
    int c=0; while(x){ c+=x&1; x>>=1;} return c;
    #endif
}

uint8_t microvsa_run_single_inference(
    const uint8_t *processed_sample, int sample_length
#ifndef MICROVSA_IMPL_FIX_SIZE
    , const void *MODEL_F, const void *MODEL_V, const void *MODEL_C,
      int num_class, int num_feature, int fv_dim_word, int fv_dim_bit
#endif
) {
#ifdef MICROVSA_IMPL_FIX_SIZE
    const mv_word_t *C = (const mv_word_t*) MICROVSA_MODEL_C;
    const mv_word_t *F = (const mv_word_t*) MICROVSA_MODEL_F;
    const mv_word_t *V = (const mv_word_t*) MICROVSA_MODEL_V;
    int n_class = MICROVSA_MODEL_NUM_CLASS;
    int n_feature = MICROVSA_MODEL_NUM_FEATURE;
    int dim_word = MICROVSA_MODEL_FHV_DIMENSION_WORD;
#else
    const mv_word_t *C = (const mv_word_t*) MODEL_C;
    const mv_word_t *F = (const mv_word_t*) MODEL_F;
    const mv_word_t *V = (const mv_word_t*) MODEL_V;
    int n_class = num_class;
    int n_feature = num_feature;
    int dim_word = fv_dim_word;
#endif

    int *P = (int*)malloc(sizeof(int)*n_class);
    if(!P) return 0xFF;
    memset(P, 0, sizeof(int)*n_class);

#if MICROVSA_IMPL_MODE == 0
    // Binary LDC (Alg.2) reference
    for (int j=0;j<dim_word;++j) {
        // per-bit accumulator
        int threshold = n_feature / 2;
        int bit_counts[32] = {0}; // safe up to 32bit words (WORDSIZE <=32)
        for (int i=0;i<n_feature;++i) {
            int val = processed_sample[i];
            mv_word_t vf = V[val * dim_word + j] ^ F[i * dim_word + j];
            for (int b=0;b<8*(int)sizeof(mv_word_t);++b) {
                if ((vf >> b) & 1u) bit_counts[b]++;
            }
        }
        // build sign word
        mv_word_t Sword = 0;
        for (int b=0;b<8*(int)sizeof(mv_word_t);++b) {
            if (bit_counts[b] > threshold) Sword |= ((mv_word_t)1u << b);
        }
        // update hamming
        for (int k=0;k<n_class;++k) {
            mv_word_t cw = C[k * dim_word + j];
            int h = popcount_word(Sword ^ cw);
            P[k] += h;
        }
    }
#elif MICROVSA_IMPL_MODE == 1
    // MCU-optimized (Alg.3): XOR per feature-word then use popcount to update P
    for (int j=0;j<dim_word;++j) {
        for (int i=0;i<n_feature;++i) {
            int val = processed_sample[i];
            mv_word_t vf = V[val * dim_word + j] ^ F[i * dim_word + j];
            for (int k=0;k<n_class;++k) {
                mv_word_t x = vf ^ C[k * dim_word + j];
                P[k] += popcount_word(x);
            }
        }
    }
#elif MICROVSA_IMPL_MODE == 2
    // Streaming-ready (same loop; streaming would persist P across windows in caller)
    for (int j=0;j<dim_word;++j) {
        for (int i=0;i<n_feature;++i) {
            int val = processed_sample[i];
            mv_word_t vf = V[val * dim_word + j] ^ F[i * dim_word + j];
            for (int k=0;k<n_class;++k) {
                mv_word_t x = vf ^ C[k * dim_word + j];
                P[k] += popcount_word(x);
            }
        }
    }
#else
#error "Unsupported MICROVSA_IMPL_MODE"
#endif

    // choose minimal Hamming (best match)
    int best = 0; int bestv = P[0];
    for (int k=1;k<n_class;++k) if (P[k] < bestv) { best=k; bestv=P[k]; }

    free(P);
    return (uint8_t)best;
}
