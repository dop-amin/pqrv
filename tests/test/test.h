#ifndef TEST_H
#define TEST_H
#include <stdint.h>
#include <stdio.h>
extern void test_opt_c908();
__attribute__((optimize("O0"))) void empty_dummy() {}
#endif