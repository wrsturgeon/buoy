#ifndef BITBANG_REG_H
#define BITBANG_REG_H

#define REG(...) (*(volatile uint32_t* const restrict)(__VA_ARGS__))

#endif // BITBANG_REG_H
