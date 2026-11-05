// A second fixture: compact input, nested calls, C11 types and format escaping.
struct Pair { a: i32, b: i32, }
impl Pair {
    fn new(a: i32, b: i32) -> Pair { Pair { a: a, b: b } }
    fn sum(&self) -> i32 { self.a + self.b }
}
fn plus(a: i32, b: i32) -> i32 { a + b }
fn main() {
    let p = Pair::new(1, plus(2, 3));
    let q = Pair { a: 5, b: 6 };
    println!("literal={} field={}", q.sum(), q.a);
    print!("pair={{}} sum={} ", p.sum());
    let total: i64 = p.sum() as i64;
    println!("total={}", total);
    let mut count = 0;
    for i in 1..=3 { count += i; }
    println!("count={}", count);
    let s = "hello";
    let f = 2.5;
    println!("{} {}", s, f);
    if count == 6 { println!("ok"); }
}
