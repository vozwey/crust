/* crustpp.cpp — crust compiled as C++: unlocks the REAL `->` return type.
 * In C++11, `auto f() -> int` is a trailing return type, so `fn` -> `auto`
 * and Rust's `fn add(a, b) -> i32` becomes valid C++.  (Params stay C-order:
 * even C++ has no `a: i32`.)  Build: make cplusplus  */
#include <cstdio>
#include <cstdint>
#define fn auto
#define i32 int32_t
#define u64 uint64_t

fn add(i32 a, i32 b) -> i32 {
    return a + b;
}

fn main() -> i32 {
    std::printf("add(40, 2) = %d\n", add(40, 2));
    return 0;
}
