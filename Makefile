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

# New compiler: implemented exclusively in ISO C11, no awk/sed in this path.
crustcc: tools/crustcc.c
	$(CC) -std=c11 -Wall -Wextra -pedantic -Werror $< -o $@

examples/full_rust.gen.c: examples/full_rust.rs crustcc
	./crustcc $< > $@

examples/full_rust: examples/full_rust.gen.c
	$(CC) -std=c11 -Wall -Wextra -pedantic -Werror $< -o $@

fullrust: examples/full_rust
	./examples/full_rust

examples/edge_rust.gen.c: examples/edge_rust.rs crustcc
	./crustcc $< > $@

examples/edge_rust: examples/edge_rust.gen.c
	$(CC) -std=c11 -Wall -Wextra -pedantic -Werror $< -o $@

check: examples/full_rust examples/edge_rust
	./examples/full_rust > examples/full_rust.actual.txt
	diff -u tests/full_rust.expected.txt examples/full_rust.actual.txt
	./examples/edge_rust > examples/edge_rust.actual.txt
	diff -u tests/edge_rust.expected.txt examples/edge_rust.actual.txt
	@if ./crustcc tests/unsupported.rs > examples/unsupported.gen.c 2> examples/unsupported.err.txt; then echo 'expected compiler error' >&2; exit 1; fi
	grep -q 'unsupported top-level construct' examples/unsupported.err.txt
	@echo 'crustcc tests: ok'

clean:
	rm -f demo rusty compile_time idioms crustpp_bin crustcc examples/hello examples/hello.gen.c examples/real_rust examples/real_rust.gen.c examples/full_rust examples/full_rust.gen.c examples/edge_rust examples/edge_rust.gen.c examples/*.actual.txt examples/unsupported.gen.c examples/unsupported.err.txt

.DELETE_ON_ERROR:
.PHONY: run clean fullrust check rust cplusplus crustc pp

# full Rust-ish syntax (x: i32, -> ret, println!) via awk transpiler, not cpp:
rust: examples/real_rust.rs tools/crustpp.awk crustpp.sh
	./crustpp.sh examples/real_rust.rs
	./examples/real_rust

# the same idea via C++ trailing return types (auto f() -> i32):
cplusplus: examples/crustpp.cpp
	g++ -std=c++11 -Wall examples/crustpp.cpp -o crustpp_bin
	./crustpp_bin
