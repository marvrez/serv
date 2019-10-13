# serv

Serv is a a multi-threaded HTTP 1.1 server written in C. 
It is simple, fast and written only using UNIX system libraries, thus requiring no extgernal dependencies.

## Building and running scyte

To clone and build scyte execute:
```sh
git clone https://github.com/marvrez/serv.git
cd serv/
make -j
```

If no errors popped up, you can run it by executing:
```sh
./serv <port>
```
