#include "crust.h"

enum Color { Red, Green, Blue };

fn name_of(enum Color c) {
    match (c) {
        case Red:   return 1;
        case Green: return 2;
        default:    return 3;
    }
}

unsafe fn danger() {
    println("doing unsafe things");
    return 0;
}

async fn fetch() {
    println("async! (not really)");
    return 0;
}

fn main() {
    let c = Green;
    println("green = %d", name_of(c));
    danger();
    fetch();
    return 0;
}
