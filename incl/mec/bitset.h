#ifndef MEC_BITSET_H
#define MEC_BITSET_H

#define $bitset(N, B) [((N) + (B) - 1) / (B)]
#define $bit_base(set) (sizeof(*(set)) * 8)
#define $bitset_has(set, index) (((set)[(index) / $bit_base(set)] & (1ULL << ((index) % $bit_base(set)))) != 0)
#define $bitset_set(set, index) ((set)[(index) / $bit_base(set)] |=  (1ULL << ((index) % $bit_base(set))))
#define $bitset_clear(set, index) ((set)[(index) / $bit_base(set)] &= ~(1ULL << ((index) % $bit_base(set))))

#endif // MEC_BITSET_H
