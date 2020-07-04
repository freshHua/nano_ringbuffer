cmake_minimum_required(VERSION 3.5)

project(nano_ringbuffer VERSION 1.0)

# Add an executable
add_executable(ringbuffer nano_ringbuffer.c ringbuffer_test.c)
