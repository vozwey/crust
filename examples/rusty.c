/* rusty.c — crust v2: fake generics, and/or/not, format!, eprintln, vec! */
#include "crust.h"

fn sum(Vec(i32) xs, usize n) {
    let mut total = 0;
    let mut i = (usize)0;
    while (i < n) {
        total += xs[i];
        i += 1;
    }
    return total;
}

fn in_range(i32 x) {
    /* Rust devs love `and`/`or`/`not` — crust has them too */
    if (x > 0 and x < 100) {
        println("%d is a nice number", x);
    } else {
        eprintln("%d is not nice", x);
    }
    return 0;
}

fn main() {
    let v = vec(1, 2, 3, 4, 5);
    println("sum = %d", sum(v, 5));

    /* format! without the bang — GNU statement expression under the hood */
    let msg = format("the sum of five numbers is %d", sum(v, 5));
    println("%s", msg);

    let opt = Some(7);          /* Option<i32> would need <> — impossible */
    let res = Ok(42);           /* same for Result */
    println("opt = %d, res = %d", opt, res);

    let b = not (opt == 0);     /* not x -> !x (in expression position) */
    println("opt is some: %d", b);

    in_range(50);
    in_range(500);

    let boxed = (Box(i32)) malloc(sizeof(i32));  /* Box(i32) -> int32_t* */
    *boxed = 1337;
    println("boxed = %d", *boxed);
    free(boxed);

    return 0;
}
