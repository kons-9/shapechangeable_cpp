#pragma once
#include "esp32c3/fs.hpp"
#include "esp32c3/serial.hpp"
#include <estimation.hpp>
extern auto init_coordinate(TaskArgs<serial::Link, fs::SpiFFS> &args) -> estimation::coordinate_t;