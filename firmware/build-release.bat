mkdir out

make clean
make TARGET=HIDman_ZX_OSC_EXTERNAL_DEBUG OSC_TYPE=OSC_EXTERNAL BUILD_OPTIONS=DEBUG
cp ./build/HIDman_ZX_OSC_EXTERNAL_DEBUG.bin ./out

make clean
make TARGET=HIDman_ZX_OSC_EXTERNAL OSC_TYPE=OSC_EXTERNAL
cp ./build/HIDman_ZX_OSC_EXTERNAL.bin ./out
