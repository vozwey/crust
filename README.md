# crust
C with Rust syntax vibecoded from scratch

Пишешь на Rust-подобном диалекте, компилируешь как C. Есть два пути: исходный режим макросов препроцессора и отдельный транслятор [`crustcc`](#crustcc-отдельный-компилятор-целиком-написанный-на-c), написанный на C.

## Быстрый старт

```sh
make run
```

или вручную:

```sh
gcc -std=gnu11 -I. examples/demo.c -o demo && ./demo
```

## Как это работает

Один заголовок — [`crust.h`](crust.h) — и вся магия макросов:

```c
#include "crust.h"

fn fib(i32 n) {
    if (n <= 1) { return n; }
    return fib(n - 1) + fib(n - 2);
}

fn main() {
    let mut i = 0;
    loop {
        if (i >= 10) { break; }
        println("fib(%d) = %d", i, fib(i));
        i += 1;
    }
    return 0;
}
```

## Таблица соответствия

| Rust | crust (C) | макрос |
|---|---|---|
| `fn foo() { }` | `fn foo() { }` → `int foo() { }` | `#define fn int` |
| `pub fn` | `pub fn` | `#define pub` |
| `let x = 5;` | `let x = 5;` | `#define let __auto_type` |
| `let mut x = 5;` | `let mut x = 5;` | `#define mut` |
| `loop { }` | `loop { }` | `#define loop while (1)` |
| `match (x) { }` | `match (x) { }` | `#define match switch` |
| `println!("..", x)` | `println("..", x)` | `#define println(...)` |
| `panic!("..")` | `panic("..")` | `#define panic(...)` |
| `assert_eq!(a, b)` | `assert_eq(a, b)` | `#define assert_eq(a,b)` |
| `dbg!(x)` | `dbg(x)` | `#define dbg(x)` |
| `i32`, `u64`, `usize`, `f64`, ... | `i32`, `u64`, `usize`, `f64`, ... | `#define i32 int32_t` и т.д. |
| `String` | `String` | `#define String char*` |
| `Some(x)` / `Ok(x)` / `Err(x)` / `None` | то же самое | макросы-обёртки |
| `vec![1,2,3]` | `vec(1, 2, 3)` | compound literal |
| `self` | `self` → `this` | `#define self this` |
| `enum Color { Red, Green }` | то же самое (C enum) | встроено в C |
| `unsafe { }` / `unsafe fn` | `unsafe { }` | `#define unsafe` |
| `async fn` | `async fn` | `#define async` |
| `dyn Trait`, `move`, `ref` | то же | пустые макросы |
| `and` / `or` / `not` | `and` / `or` / `not` | `&&` / `||` / `!` |

## Что физически невозможно (и почему)

Препроцессор умеет заменять только **идентификаторы**. Эти токены — не идентификаторы, их трогать нельзя:

- **`->`** (стрелка возврата `fn f() -> i32`) — в C это доступ к члену через указатель, допустим только внутри выражений, но не в объявлении функции. Поэтому в crust все `fn` возвращают `int`.
- **`:`** (аннотации типов `x: i32`) — в списке параметров C двоеточие легально ровно ни в одном месте. Поэтому тип идёт первым: `fn f(i32 x)`, `struct P { i32 x; }`.
- **`!`** в именах макросов (`println!`) — поэтому в crust макросы пишутся без восклицательного знака: `println(...)`.
- **`..`** (диапазоны), **`<>`** (дженерики), **`::`** (пути), **`=>`** (ветки match) — та же причина.

## Демо

Смотри [`examples/demo.c`](examples/demo.c) — там fib, `loop`, `match`, `let mut`, структуры, `vec`, `Ok`, `dbg!`-аналог и `panic`.

И [`examples/rusty.c`](examples/rusty.c) — «crust v2»: псевдо-дженерики `Vec(i32)` / `Box(i32)` / `Option(i32)`, операторы `and` / `or` / `not`, `format(...)` на GNU statement-expression, `eprintln(...)`.

## Почему на препроцессоре нельзя написать НАСТОЯЩИЙ компилятор Rust

Хорошая попытка 😄 Чистый cpp:

- заменяет только **идентификаторы**, а грамматика Rust полна токенов `->`, `::`, `!`, `'a`, `?`, `..`, `<>` — их макросом не назовёшь;
- **не умеет читать файлы и вообще не имеет I/O** — входной `.rs` ему не скормить;
- формально Turing-complete (да, на рекурсивных макросах теоретически можно), но парсить чужой синтаксис он всё равно не сможет.

Максимум возможного — транслятор Rust-подобного синтаксиса в C, чем crust, собственно, и является.

## crustc: «компилятор», который живёт внутри препроцессора

[`crustc.h`](crustc.h) — настоящие вычисления на этапе препроцессинга по технике deferred evaluation от Paul Fultz II ([pfultz2.com/blog/2012/05/10/turing](https://pfultz2.com/blog/2012/05/10/turing/), [обсуждение Turing-completeness cpp](https://stackoverflow.com/questions/3136686/is-the-c99-preprocessor-turing-complete), λ-исчисление на макросах — [Metalang99](https://news.ycombinator.com/item?id=28895622)):

- `EVAL` гоняет тысячи пересканирований макроэкспандера;
- `DEFER`/`OBSTRUCT` откладывают рекурсивные вызовы мимо «синей краски» (blue paint), поэтому рекурсия реально идёт;
- `INC`/`DEC` — арифметика через таблицы токен-пастинга, `CHECK`/`NOT`/`IIF` — логика.

В результате `add(40, 2)` раскрывается в токен `42` **до того, как gcc вообще начал парсить файл** — см. `make pp`:

```
enum { THE_ANSWER = 42 };                            <- посчитал cpp
static int array_sized_by_the_preprocessor[13];      <- и это тоже
```

`CRUST_REPEAT(8, M, ~)` вообще стримит `0 1 2 3 4 5 6 7` прямо в токенный поток.

## crustc.sh: настоящий source-to-source «компилятор» на препроцессоре

Да, `#include` — это чтение файлов, а `gcc -E` — запись результата ([gcc -E как генератор кода](https://stackoverflow.com/questions/4900870/can-gcc-output-c-code-after-preprocessing)), так что пайплайн `.crust → cpp → .c → бинарник` вполне реален:

```sh
make crustc        # или: ./crustc.sh examples/hello.crust
```

`examples/hello.crust` прогоняется через `gcc -E -P -x c` — препроцессор читает файл, инлайнит `crust.h` через `#include` и переписывает все Rust-образные идентификаторы — и на выходе получается валидный C (`int greet(char* name) { ... while (1) { ... } }`), который компилируется обычным gcc.

**Предел всё тот же**: переписывать можно только идентификаторы. Токены `->`, `::`, `!`, `..`, `<>` препроцессор не трогает, поэтому настоящий Rust (с его `fn f() -> i32`, `x: i32`, `println!`) на чистом cpp не напишешь — а вот «Rust-подобный диалект → C» и «вычисления на этапе препроцессинга» — запросто.

## Лазейки в «других компиляторах»

- **Чистый C любой версии (C89…C23) и любой компилятор (gcc/clang/tcc): нет.** `->` в деклараторе и `:` в параметрах — не фича, а запрет грамматики C, его не обходит ни один диалект.
- **C++ (g++) — да для `->`!** В C++11 `auto f() -> i32` — легальный trailing return type ([cppreference](https://en.cppreference.com/cpp/language/auto)). Поэтому `examples/crustpp.cpp` с `#define fn auto` компилируется через `g++`, и там **настоящий** `fn add(i32 a, i32 b) -> i32` — см. `make cplusplus`.
- **awk/sed/m4 вместо cpp — да для всего остального.** Эти макропроцессоры работают с произвольными токенами, а не только с идентификаторами (у m4 вообще regex через `patsubst`). Поэтому `tools/crustpp.awk` + `crustpp.sh` честно перегоняют настоящий Rust-диалект в C: `make rust` берёт `examples/real_rust.rs` с `fn add(a: i32, b: i32) -> i32 { a + b }`, `let x: i32 = ...` и `println!("{}", x)` и собирает бинарник, печатающий `42`. Это и есть путь «любым способом» — просто он уже не чистый препроцессор C.

## crustcc: отдельный компилятор, целиком написанный на C

Препроцессорный режим выше остаётся для шутки, а здесь — **другой путь**:
[`tools/crustcc.c`](tools/crustcc.c) написан на ISO C11 и использует только
стандартную библиотеку C. Он сам читает `.rs`, разбивает исходник на токены,
разбирает объявления и блоки, ведёт таблицу типов и переменных и пишет
**обычный C11** в stdout. В трансляции нет awk, sed, Python или макросов cpp;
`gcc`/`cc` нужен только как отдельный бэкенд для полученного C.

```sh
make fullrust   # crustcc.c -> crustcc -> full_rust.gen.c -> C-бэкенд -> запуск
make check      # ещё один пример, сравнение результатов и проверка ошибки
```

Вручную (можно заменить `cc` на `gcc` или другой C11-компилятор):

```sh
cc -std=c11 -Wall -Wextra tools/crustcc.c -o crustcc
./crustcc examples/full_rust.rs > examples/full_rust.gen.c
cc -std=c11 examples/full_rust.gen.c -o examples/full_rust
./examples/full_rust
```

Подмножество диалекта: `fn` с `a: i32` и `-> T`, возвращаемое выражение
без `;`, `let`/`let mut` с выведенным или явным типом, `struct` с полями,
`impl` и методы с `&self`, конструктор `Type::new`, литерал структуры,
поля `self.x`, простой `enum` и возвращающий значение `match` (`=>` с `_`),
`loop`, `if`, `for i in a..b` / `a..=b`, `as`, `println!`/`print!`/`eprintln!`
с `{}`. Примеры: [`examples/full_rust.rs`](examples/full_rust.rs),
[`examples/edge_rust.rs`](examples/edge_rust.rs).

Это **не rustc**: нет владения/заимствований, lifetimes, трейтов, обобщений,
замыканий, pattern matching произвольной формы, async-runtime и
проверки безопасности памяти. `pub`/`unsafe`/`async` сейчас только
синтаксические модификаторы без семантики; определения нужно размещать
до их использования. Вывод типа ограничен простыми выражениями — для
сложных добавляйте `: Type`. Неподдержанный синтаксис даёт ошибку
`файл:строка`, а не обещание, что любой Rust скомпилируется.
