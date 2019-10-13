# serv

Serv is a a multi-threaded HTTP 1.1 server written in C. 
It is simple, fast and written only using UNIX system libraries, thus requiring no extgernal dependencies.

## Building and spinning the server

To clone and build serv execute:
```sh
git clone https://github.com/marvrez/serv.git
cd serv/
make -j
```

If no errors showed up, you can finally spin up the server by executing:
```sh
./serv <port>
```
