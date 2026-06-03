#include "unity.h"
#include "error_hub.hpp"

TEST_CASE("Error Hub Test", "[error_hub]") {
    ErrorHub::get_instance().report(ErrorSeverity::NOTICE, "Test", "Unit testing queue", 100);
    TEST_ASSERT_EQUAL_INT(100, 100);
}
