# CMake generated Testfile for 
# Source directory: /home/alex/programming/c++/coding4-jenny/hw1/deps/canvas
# Build directory: /home/alex/programming/c++/coding4-jenny/hw1/deps/canvas/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[canvas_window_test]=] "/home/alex/programming/c++/coding4-jenny/hw1/deps/canvas/build/window_test")
set_tests_properties([=[canvas_window_test]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/alex/programming/c++/coding4-jenny/hw1/deps/canvas/CMakeLists.txt;20;add_test;/home/alex/programming/c++/coding4-jenny/hw1/deps/canvas/CMakeLists.txt;0;")
add_test([=[canvas_bmp_test]=] "/home/alex/programming/c++/coding4-jenny/hw1/deps/canvas/build/bmp_test")
set_tests_properties([=[canvas_bmp_test]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/alex/programming/c++/coding4-jenny/hw1/deps/canvas/CMakeLists.txt;26;add_test;/home/alex/programming/c++/coding4-jenny/hw1/deps/canvas/CMakeLists.txt;0;")
subdirs("deps/external/glfw")
subdirs("deps/external/gl3w")
