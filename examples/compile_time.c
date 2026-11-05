/* compile_time.c — proof that "crustc" really computes at PREPROCESS time.
 *
 * Every add(...) below is fully evaluated by cpp into a plain decimal token
 * BEFORE gcc parses anything. If the preprocessor didn't compute the right
 * number, these static_asserts would fail the build.
 *
 *   make pp   <- watch cpp itself do the math
 */
#include "crustc.h"
#include <assert.h>

static_assert(add(2, 3) == 5,   "cpp computed 2 + 3");
static_assert(add(40, 2) == 42, "cpp computed the answer");
static_assert(add(20, 22) == 42, "cpp is very sure about 42");

/* results are real compile-time constants: enum values, array sizes */
enum { THE_ANSWER = add(40, 2) };
static int array_sized_by_the_preprocessor[add(6, 7)]; /* 13 elements */

int main(void) {
    assert(THE_ANSWER == 42);
    assert(sizeof array_sized_by_the_preprocessor / sizeof(int) == 13);
    return 0;
}
