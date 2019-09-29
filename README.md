# pdf
pdf parser


### freetype
1. wget http://download.savannah.gnu.org/releases/freetype/freetype-2.4.10.tar.gz
2. ./configure
3. make
4. sudo make install

### zlib
1. sudo apt-get install zlib1g-dev


### gflags
1. git clone https://github.com/gflags/gflags.git
2. cmake .
3. make -j 24
4. sudo make install


### jpeglib
1. http://www.ijg.org/files/jpegsrc.v9c.tar.gz
2. tar xzvf jpegsrc.v9c.tar.gz
3. ./configure --enable-shared --enable-static
4. make && sudo make install


### glog
1. git clone https://github.com/google/glog
2. sudo apt-get install autoconf automake libtool
3. cd glog
4. mkdir build && cd build
5. cmake ..
6. sudo make install 
