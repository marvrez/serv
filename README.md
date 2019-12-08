# serv

serv is a a multi-threaded HTTP 1.1 server written in C. 
It is simple, fast and written only using UNIX system libraries, thus requiring no external dependencies.

## Building and spinning up the server

To install serv, execute:
```sh
git clone https://github.com/marvrez/serv.git
cd serv/
make -j
```

If no errors showed up, you can finally spin up the server by executing:
```sh
./serv <port>
```
Where `<port>` is an optional argument, set to the value `4950` by default, specifying the port the server will listen to.
