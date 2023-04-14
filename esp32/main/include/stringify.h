#ifndef STRINGIFY_H
#define STRINGIFY_H

// The classic

#define STRINGIFY(...) STRINGIFY_LITERAL(__VA_ARGS__)
#define STRINGIFY_LITERAL(...) #__VA_ARGS__

#endif // STRINGIFY_H
