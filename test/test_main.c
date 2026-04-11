#include "unity.h"

extern void run_all_tests(void);

int main(void) {
    UNITY_BEGIN();
    run_all_tests();
    return UNITY_END();
}
