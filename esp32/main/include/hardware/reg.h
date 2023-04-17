#ifndef BITBANG_REG_H
#define BITBANG_REG_H

#define REG_PTR(...) ((uint32_t volatile* const restrict)(__VA_ARGS__))
#define REG(...) (*(REG_PTR(__VA_ARGS__)))

#endif // BITBANG_REG_H
