cd ../swarmtv
cmake .
make clean
make 
make install
cd ../qtswarmtv
qmake
make clean
make
make install
