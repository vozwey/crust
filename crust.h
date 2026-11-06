/* ============================================================================
 * crust.h — Rust syntax in pure C, using ONLY the C preprocessor.
 * No compiler patches, no external tools. Just macros. Beautiful, dirty macros.
 *
 * Usage:
 *     #include "crust.h"
 *     gcc -std=gnu11 -I. demo.c -o demo && ./demo
 *
 * THE TWO TOKENS THAT ARE PHYSICALLY IMPOSSIBLE (they are not identifiers,
 * so the preprocessor cannot make them macro names):
 *
 *   ->   Rust's return-type arrow  `fn f() -> i32`
 *        In C, `->` is pointer member access and is only valid inside
 *        expressions, never in a file-scope declarator. So in crust every
 *        `fn` returns int (classic C style) — write `fn f() { return ...; }`
 *
 *   :    Rust's type annotation    `let x: i32`, `fn f(x: i32)`, `x: i32` fields
 *        A colon is legal in a parameter list / initializer for exactly zero
 *        C constructs. So in crust the type goes FIRST (C order) but keeps
 *        the Rust type name: `fn f(i32 x)`, `struct P { i32 x; }`
 *
 * Also impossible: the `!` in macro names (`println!`), `..` ranges,
 * `<>` generics, `::` paths, `=>` match arms. Crust uses `println(...)`,
 * `while`, plain types, `case`, etc. instead.
 * ==========================================================================*/
#ifndef CRUST_H
#define CRUST_H

/* System headers FIRST — the macros below are landmines and must not
 * leak into libc's own identifiers. */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>

/* ----------------------------- keywords ---------------------------------- */
#define fn      int          /* fn foo() { }        ->  int foo() { }          */
#define pub                      /* pub fn foo()        ->  fn foo()             */
#define mut                      /* let mut x = 1;      ->  let x = 1;           */
#define let     __auto_type      /* let x = 1;          ->  __auto_type x = 1;    */
#define loop    while (1)        /* loop { }            ->  while (1) { }         */
#define match   switch           /* match x { }         ->  switch (x) { }        */
#define self    this             /* fn f(T* self)       ->  fn f(T* this)         */
#define not     !                /* not x               ->  !x                    */
#define String  char*            /* String s = "hi";    ->  char* s = "hi";       */
#define None    0                /* Option-ish sugar                              */
#define Some(x) (x)
#define Ok(x)   (x)
#define Err(x)  (x)

/* --------------------- primitive types: Rust names ----------------------- */
#define i8    int8_t
#define i16   int16_t
#define i32   int32_t
#define i64   int64_t
#define isize ptrdiff_t
#define u8    uint8_t
#define u16   uint16_t
#define u32   uint32_t
#define u64   uint64_t
#define usize size_t
#define f32   float
#define f64   double

/* ------------- "macros" — same as Rust but without the bang (!) ----------
 * `println!` cannot be a macro (`!` is not an identifier), so crust drops
 * the bang: println("...", args). Format specifiers stay C-style (%d etc.),
 * because the preprocessor cannot rewrite the inside of string literals.   */
#define println(...)  (printf(__VA_ARGS__), putchar('\n'))
#define print(...)    printf(__VA_ARGS__)
#define panic(...)    (fprintf(stderr, __VA_ARGS__), fputc('\n', stderr), abort())
#define assert_eq(a, b) assert((a) == (b))
#define assert_ne(a, b) assert((a) != (b))
#define dbg(x)        (printf("%s = %ld\n", #x, (long)(x)), (x))
#define todo()        (fprintf(stderr, "crust: not yet implemented\n"), abort())
#define vec(...)      ((int[]){ __VA_ARGS__ })   /* int-only vec! for the lolz */
#define eprintln(...) (fprintf(stderr, __VA_ARGS__), fputc('\n', stderr))

/* GNU statement-expression: Rust's format! returns a String,
 * crust's format() returns a char* buffer. Dirty, but works. */
#define format(...) ({ static char _crust_fmt[1024]; \
                       snprintf(_crust_fmt, sizeof _crust_fmt, __VA_ARGS__); \
                       _crust_fmt; })

/* ------------------------- v2: even more Rust-isms ------------------------- */
#define and   &&
#define or    ||
#define move                  /* `move` closures don't exist in C, so neither does move */
#define unsafe                /* unsafe { }  ->  plain block { }                        */
#define async                 /* async fn f() -> fn f()  (no runtime, just vibes)       */
#define dyn                   /* dyn Trait   ->  Trait                                  */
#define ref                   /* ref x       ->  x                                      */

/* "Generics" without <>: Vec(i32) -> int32_t* etc. Not real generics,
 * just function-like macros eating the type. */
#define Vec(T)     T*
#define Box(T)     T*
#define Rc(T)      T*
#define Arc(T)     T*
#define Option(T)  T
#define Result(T,E) T

#endif /* CRUST_H */
