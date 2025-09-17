
#ifndef MICROVSA_CONFIG_H_
#define MICROVSA_CONFIG_H_

#ifndef MICROVSA_IMPL_WORDSIZE
#define MICROVSA_IMPL_WORDSIZE 8
#endif

#ifndef MICROVSA_IMPL_MODE
// 0 = Binary LDC reference (Alg.2)
// 1 = MCU-optimized (Alg.3)
// 2 = Streaming-optimized (framework)
#define MICROVSA_IMPL_MODE 1
#endif

#ifndef MICROVSA_IMPL_FIX_SIZE
#define MICROVSA_IMPL_FIX_SIZE 1
#endif

#endif // MICROVSA_CONFIG_H_
