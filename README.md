# msgclient
# msgclient
(1)server make(ubuntu x64):

sudo apt-get install gawk

sudo apt-get install g++

sudo apt-get install make

sudo apt-get install libssl-dev

sudo apt-get install uuid-dev

git clone https://github.com/zenzhang/msgclient.git

cd msgclient/build

make linux-x64 server info

make linux-x64 server -j4

(2) client make(win32 vs2013):

vs2013 open  msgclient/build/msvc-13/base.sln




