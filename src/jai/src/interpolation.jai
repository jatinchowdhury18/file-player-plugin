interpolate_linear :: (data: *float32, read_ptr: float64) -> float32 {
    ptr0 := cast(s64) read_ptr;
    ptr1 := ptr0 + 1;
    alpha := cast(float32) (read_ptr - cast(float64) ptr0);
    return (1.0 - alpha) * data[ptr0] + alpha * data[ptr1];
}

interpolate_cubic :: (data: *float32, read_ptr: float64) -> float32 {
    ptr0 := cast(s64) read_ptr;
    ptr1 := ptr0 + 1;
    ptr2 := ptr1 + 1;
    ptr3 := ptr2 + 1;

    value0 := data[ptr0];
    value1 := data[ptr1];
    value2 := data[ptr2];
    value3 := data[ptr3];

    alpha := cast(float32) (read_ptr - cast(float64) ptr0);
    d1 := alpha - 1.0;
    d2 := alpha - 2.0;
    d3 := alpha - 3.0;

    one_sixth : float32 : 1.0 / 6.0;
    c0 := -d1 * d2 * d3 * one_sixth;
    c1 := d2 * d3 * 0.5;
    c2 := -d1 * d3 * 0.5;
    c3 := d1 * d2 * one_sixth;

    return value0 * c0 + alpha * (value1 * c1 + value2 * c2 + value3 * c3);
}

interpolate_quintic :: (data: *float32, read_ptr: float64) -> float32 {
    ptr0 := cast(s64) read_ptr;
    ptr1 := ptr0 + 1;
    ptr2 := ptr1 + 1;
    ptr3 := ptr2 + 1;
    ptr4 := ptr3 + 1;
    ptr5 := ptr4 + 1;

    value0 := data[ptr0];
    value1 := data[ptr1];
    value2 := data[ptr2];
    value3 := data[ptr3];
    value4 := data[ptr4];
    value5 := data[ptr5];

    alpha := cast(float32) (read_ptr - cast(float64) ptr0);
    d1 := alpha - 1.0;
    d2 := alpha - 2.0;
    d3 := alpha - 3.0;
    d4 := alpha - 4.0;
    d5 := alpha - 5.0;

    one_120th : float32 : 1.0 / 120.0;
    one_24th : float32 : 1.0 / 24.0;
    one_12th : float32 : 1.0 / 12.0;

    c0 := -d1 * d2 * d3 * d4 * d5 * one_120th;
    c1 := d2 * d3 * d4 * d5 * one_24th;
    c2 := -d1 * d3 * d4 * d5 * one_12th;
    c3 := d1 * d2 * d4 * d5 * one_12th;
    c4 := -d1 * d2 * d3 * d5 * one_24th;
    c5 := d1 * d2 * d3 * d4 * one_120th;

    return value0 * c0 + alpha * (value1 * c1 + value2 * c2 + value3 * c3 + value4 * c4 + value5 * c5);
}
