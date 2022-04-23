all: env
	cd build && cmake --build . && cd ..

env:
	if [ -d "./build" ]; then rm -rf ./build; fi
	mkdir build
	cd build && cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DLLVM_INSTALL_DIR=${LLVM_INSTALL_DIR} -G "Ninja"
	if [ -f "./compile_commands.json" ]; then rm -rf ./compile_commands.json; fi
	ln ./build/compile_commands.json .

