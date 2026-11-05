/* demo.c — a program written in (almost) pure Rust syntax, compiled as C.
 * Build:  make run        (or: gcc -std=gnu11 -I. examples/demo.c -o demo)
 */
#include "crust.h"

fn fib(i32 n) {
    if (n <= 1) {
        return n;
    }
    return fib(n - 1) + fib(n - 2);
}

struct Point {
    i32 x;
    i32 y;
};
typedef struct Point Point;

/* methods are just functions taking a pointer; `self` becomes `this` */
fn dist2(Point* self) {
    return self->x * self->x + self->y * self->y; /* crust: -> instead of . */
}

fn main() {
    let mut i = 0;
    loop {
        if (i >= 10) { break; }
        println("fib(%d) = %d", i, fib(i));
        i += 1;
    }

    let p = (Point){ 3, 4 };
    println("dist2 = %d", dist2(&p));

    let name = "world";
    println("hello, %s!", name);

    let v = vec(10, 20, 30);
    println("v[1] = %d", v[1]);

    let ok = Ok(42);
    println("ok = %d", ok);

    match (fib(7)) {
        case 13:  println("fib(7) is 13, as expected"); break;
        default:  println("math broke");                break;
    }

    assert_eq(fib(10), 55);
    dbg(fib(10));

    if (!(fib(6) == 8)) {
        panic("fib(6) should be 8");
    }

    return 0;
}
