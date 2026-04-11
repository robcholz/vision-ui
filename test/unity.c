#include "unity.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

static const char* g_suite_name = NULL;
static int g_tests_run = 0;
static int g_tests_failed = 0;
static int g_current_test_failed = 0;

int UnityBegin(const char* suite_name) {
    g_suite_name = suite_name;
    g_tests_run = 0;
    g_tests_failed = 0;
    g_current_test_failed = 0;
    printf("test suite: %s\n", suite_name);
    return 0;
}

int UnityEnd(void) {
    printf("tests run: %d\n", g_tests_run);
    printf("tests failed: %d\n", g_tests_failed);
    if (g_tests_failed == 0) {
        printf("result: PASS\n");
    } else {
        printf("result: FAIL\n");
    }
    return g_tests_failed;
}

void UnityRunTest(unity_test_fn_t test_fn, const char* test_name, const int line_number) {
    (void) line_number;
    g_current_test_failed = 0;
    g_tests_run++;
    printf("[ RUN      ] %s\n", test_name);
    test_fn();
    if (g_current_test_failed) {
        g_tests_failed++;
        printf("[  FAILED  ] %s\n", test_name);
        return;
    }
    printf("[       OK ] %s\n", test_name);
}

void UnityFail(const char* message, const char* file, const int line_number) {
    g_current_test_failed = 1;
    printf("%s:%d: %s\n", file, line_number, message);
}

void UnityAssertTrue(const int condition, const char* expression, const char* file, const int line_number) {
    if (!condition) {
        char message[256];
        snprintf(message, sizeof(message), "assertion failed: %s", expression);
        UnityFail(message, file, line_number);
    }
}

void UnityAssertEqualLong(
        const long expected,
        const long actual,
        const char* expected_expr,
        const char* actual_expr,
        const char* file,
        const int line_number
) {
    if (expected != actual) {
        char message[256];
        snprintf(
                message,
                sizeof(message),
                "expected %s=%ld but %s=%ld",
                expected_expr,
                expected,
                actual_expr,
                actual
        );
        UnityFail(message, file, line_number);
    }
}

void UnityAssertEqualUnsignedLong(
        const unsigned long expected,
        const unsigned long actual,
        const char* expected_expr,
        const char* actual_expr,
        const char* file,
        const int line_number
) {
    if (expected != actual) {
        char message[256];
        snprintf(
                message,
                sizeof(message),
                "expected %s=%lu but %s=%lu",
                expected_expr,
                expected,
                actual_expr,
                actual
        );
        UnityFail(message, file, line_number);
    }
}

void UnityAssertEqualPtr(
        const void* expected,
        const void* actual,
        const char* expected_expr,
        const char* actual_expr,
        const char* file,
        const int line_number
) {
    if (expected != actual) {
        char message[256];
        snprintf(
                message,
                sizeof(message),
                "expected %s=%p but %s=%p",
                expected_expr,
                expected,
                actual_expr,
                actual
        );
        UnityFail(message, file, line_number);
    }
}

void UnityAssertEqualFloat(
        const float expected,
        const float actual,
        const float tolerance,
        const char* expected_expr,
        const char* actual_expr,
        const char* file,
        const int line_number
) {
    if (fabsf(expected - actual) > tolerance) {
        char message[256];
        snprintf(
                message,
                sizeof(message),
                "expected %s=%f but %s=%f within %f",
                expected_expr,
                expected,
                actual_expr,
                actual,
                tolerance
        );
        UnityFail(message, file, line_number);
    }
}

void UnityAssertString(const char* expected, const char* actual, const char* file, const int line_number) {
    const char* expected_safe = expected != NULL ? expected : "(null)";
    const char* actual_safe = actual != NULL ? actual : "(null)";
    if ((expected == NULL && actual != NULL) || (expected != NULL && actual == NULL) ||
        (expected != NULL && actual != NULL && strcmp(expected, actual) != 0)) {
        char message[256];
        snprintf(message, sizeof(message), "expected \"%s\" but was \"%s\"", expected_safe, actual_safe);
        UnityFail(message, file, line_number);
    }
}

int UnityCurrentTestFailed(void) {
    return g_current_test_failed;
}
