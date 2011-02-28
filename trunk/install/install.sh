cd ../swarmtv
cmake .
make realclean
# do we realy need to real clean ?
make clean
make 
sudo make install
cd ../qtswarmtv
qmake
make clean
make
sudo make install
