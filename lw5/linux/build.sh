unlink ./$1.exe
sudo g++ -std=c++11 ./$1.cpp -lpthread -o $1.exe