project(mongoose)

# the final lib
add_library(mongoose STATIC mongoose.c mongoose.h)

# ignore warnings
set_target_properties(mongoose PROPERTIES COMPILE_FLAGS "-w")