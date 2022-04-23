add_executable(addconst
    dot.cpp
    main.cpp)

target_link_libraries(addconst
    LLVMPasses
    LLVMCore
    LLVMIRReader
    LLVMSupport)

# target_include_directories(addconst PRIVATE "../include")
