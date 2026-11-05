CC      ?= gcc
CFLAGS  ?= -std=gnu11 -Wall -Wextra -I.

demo: examples/demo.c crust.h
	$(CC) $(CFLAGS) examples/demo.c -o demo

rusty: examples/rusty.c crust.h
	$(CC) $(CFLAGS) examples/rusty.c -o rusty

idioms: examples/idioms.c crust.h
	$(CC) $(CFLAGS) examples/idioms.c -o idioms

run: demo rusty compile_time idioms
	./demo
	./rusty
	./compile_time
	./idioms

# watch the preprocessor itself do the math (crustc):
pp: examples/compile_time.c crustc.h
	$(CC) -std=gnu11 -I. -E -P examples/compile_time.c | grep -E 'static_assert|THE_ANSWER|int array'

compile_time: examples/compile_time.c crustc.h
	$(CC) $(CFLAGS) examples/compile_time.c -o compile_time

# the "rust compiler": .crust -> (preprocessor) -> .gen.c -> (gcc) -> binary
crustc: examples/hello.crust crust.h crustc.sh
	./crustc.sh examples/hello.crust
	./examples/hello

clean:
	rm -f demo rusty compile_time idioms crustpp_bin examples/hello examples/hello.gen.c examples/real_rust examples/real_rust.gen.c

.PHONY: run clean

# full Rust-ish syntax (x: i32, -> ret, println!) via awk transpiler, not cpp:
rust: examples/real_rust.rs tools/crustpp.awk crustpp.sh
	./crustpp.sh examples/real_rust.rs
	./examples/real_rust

# the same idea via C++ trailing return types (auto f() -> i32):
cplusplus: examples/crustpp.cpp
	g++ -std=c++11 -Wall examples/crustpp.cpp -o crustpp_bin
	./crustpp_bin
