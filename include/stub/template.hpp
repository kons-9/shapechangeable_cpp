#pragma once
#include "stub/fs.hpp"
#include "stub/serial.hpp"
#include <estimation.hpp>
extern auto init_coordinate(TaskArgs<test::SerialMock, test::FsMock> &args) -> estimation::coordinate_t;