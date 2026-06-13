## build steps

conan install . --output-folder=build --build=missing

cd build

cmake ..

make
