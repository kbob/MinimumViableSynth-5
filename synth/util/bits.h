#ifndef BITS_included
#define BITS_included

template <class T>
unsigned count_bits(const T&);

template <>
unsigned count_bits(const std::uint8_t& word)
{
    unsigned A2 = word - (word >> 1 & 0x55);
    unsigned A5 = (A2 & 0x33) + (A2 >> 2 & 0x33);
    unsigned D4 = A5 + (A5 >> 4) & 0x0F;
    // unsigned X1 = D4 + (D4 >> 8) & 0x00FF;
    return D4;
}

template <>
unsigned count_bits(const std::uint16_t& word)
{
    unsigned A2 = word - (word >> 1 & 0x5555);
    unsigned A5 = (A2 & 0x3333) + (A2 >> 2 & 0x3333);
    unsigned D4 = A5 + (A5 >> 4) & 0x0F0F;
    unsigned X1 = D4 + (D4 >> 8) & 0x00FF;
    return X1;
}

template <>
unsigned count_bits(const std::uint32_t& word)
{
    unsigned A2 = word - (word >> 1 & 0x55555555);
    unsigned A5 = (A2 & 0x33333333) + (A2 >> 2 & 0x33333333);
    unsigned D4 = A5 + (A5 >> 4) & 0x0F0F0F0F;
    std::uint32_t A8 = (0x01010101 * D4) >> 24 & 0x000000FF;
    return A8;
}

template <>
unsigned count_bits(const std::uint64_t& word)
{
    std::uint64_t A2 = word - (word >> 1 & 0x5555555555555555);
    std::uint64_t A5 = (A2 & 0x3333333333333333) + (A2 >> 2 & 0x3333333333333333);
    std::uint64_t D4 = A5 + (A5 >> 4) & 0x0F0F0F0F0F0F0F0F;
    std::uint64_t A8 = (0x0101010101010101 * D4) >> 56;
    return A8;
}

#endif /* !BITS_included */
