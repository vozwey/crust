// full_rust.rs — Rust-like dialect translated to ISO C11 by tools/crustcc.c
// Build & run: make fullrust (compiler is written entirely in C)

struct Point {
    x: i32,
    y: i32,
}

impl Point {
    fn new(x: i32, y: i32) -> Point {
        Point { x: x, y: y }
    }

    fn dist2(&self) -> i32 {
        self.x * self.x + self.y * self.y
    }
}

enum Color {
    Red,
    Green,
    Blue,
}

fn describe(c: Color) -> i32 {
    match c {
        Color::Red => 1,
        Color::Green => 2,
        _ => 3,
    }
}

fn main() {
    let p = Point::new(3, 4);
    println!("dist2 = {}", p.dist2());

    let mut i = 0;
    loop {
        if i >= 3 { break; }
        println!("i = {}", i);
        i += 1;
    }

    for n in 0..3 {
        println!("n = {}", n);
    }

    let c = Color::Green;
    println!("color = {}", describe(c));

    let answer: i32 = 40 + 2;
    println!("answer = {}", answer);
}
