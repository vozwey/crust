// real_rust.rs — as close to real Rust as it gets without rustc.
// Transpiled to C by tools/crustpp.awk (sed/awk, token-agnostic — no identifier limits!)
fn add(a: i32, b: i32) -> i32 {
    a + b
}

fn main() {
    let x: i32 = add(40, 2);
    println!("{}", x);
}
