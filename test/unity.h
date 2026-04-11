#ifndef TEST_UNITY_H
#define TEST_UNITY_H

#include <stddef.h>
#include <stdint.h>

typedef void (*unity_test_fn_t)(void);

extern int UnityBegin(const char* suite_name);
extern int UnityEnd(void);
extern void UnityRunTest(unity_test_fn_t test_fn, const char* test_name, int line_number);
extern void UnityFail(const char* message, const char* file, int line_number);
extern void UnityAssertTrue(int condition, const char* expression, const char* file, int line_number);
extern void UnityAssertEqualLong(
        long expected,
        long actual,
        const char* expected_expr,
        const char* actual_expr,
        const char* file,
        int line_number
);
extern void UnityAssertEqualUnsignedLong(
        unsigned long expected,
        unsigned long actual,
        const char* expected_expr,
        const char* actual_expr,
        const char* file,
        int line_number
);
extern void UnityAssertEqualPtr(
        const void* expected,
        const void* actual,
        const char* expected_expr,
        const char* actual_expr,
        const char* file,
        int line_number
);
extern void UnityAssertEqualFloat(
        float expected,
        float actual,
        float tolerance,
        const char* expected_expr,
        const char* actual_expr,
        const char* file,
        int line_number
);
extern void UnityAssertString(const char* expected, const char* actual, const char* file, int line_number);
extern int UnityCurrentTestFailed(void);

#define UNITY_BEGIN() UnityBegin(__FILE__)
#define UNITY_END() UnityEnd()
#define RUN_TEST(test_fn) UnityRunTest((test_fn), #test_fn, __LINE__)

#define TEST_ASSERT_TRUE(condition) UnityAssertTrue((condition) != 0, #condition, __FILE__, __LINE__)
#define TEST_ASSERT_FALSE(condition) UnityAssertTrue((condition) == 0, #condition, __FILE__, __LINE__)
#define TEST_ASSERT_NULL(value) UnityAssertEqualPtr(NULL, (value), "NULL", #value, __FILE__, __LINE__)
#define TEST_ASSERT_NOT_NULL(value) UnityAssertTrue((value) != NULL, #value " != NULL", __FILE__, __LINE__)
#define TEST_ASSERT_EQUAL_INT(expected, actual)                                                                        \
    UnityAssertEqualLong((long) (expected), (long) (actual), #expected, #actual, __FILE__, __LINE__)
#define TEST_ASSERT_EQUAL_UINT(expected, actual)                                                                       \
    UnityAssertEqualUnsignedLong(                                                                                      \
            (unsigned long) (expected), (unsigned long) (actual), #expected, #actual, __FILE__, __LINE__               \
    )
#define TEST_ASSERT_EQUAL_UINT8(expected, actual) TEST_ASSERT_EQUAL_UINT((expected), (actual))
#define TEST_ASSERT_EQUAL_UINT16(expected, actual) TEST_ASSERT_EQUAL_UINT((expected), (actual))
#define TEST_ASSERT_EQUAL_UINT32(expected, actual) TEST_ASSERT_EQUAL_UINT((expected), (actual))
#define TEST_ASSERT_EQUAL_SIZE_T(expected, actual) TEST_ASSERT_EQUAL_UINT((expected), (actual))
#define TEST_ASSERT_EQUAL_PTR(expected, actual)                                                                        \
    UnityAssertEqualPtr((expected), (actual), #expected, #actual, __FILE__, __LINE__)
#define TEST_ASSERT_EQUAL_FLOAT(expected, actual)                                                                      \
    UnityAssertEqualFloat((expected), (actual), 0.0001f, #expected, #actual, __FILE__, __LINE__)
#define TEST_ASSERT_FLOAT_WITHIN(delta, expected, actual)                                                              \
    UnityAssertEqualFloat((expected), (actual), (delta), #expected, #actual, __FILE__, __LINE__)
#define TEST_ASSERT_EQUAL_STRING(expected, actual) UnityAssertString((expected), (actual), __FILE__, __LINE__)
#define TEST_FAIL_MESSAGE(message) UnityFail((message), __FILE__, __LINE__)

#endif
