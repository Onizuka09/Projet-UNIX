## Projet UNIX 
- UDP MONO Client/Server 
- TCP MONO Client/Server
- TCP Multi Client/Mono Server  
#### compiling the UDP program 
- server 
```bash 
gcc udp_serv.c logger.c -o serv
```
- client 
```bash 
gcc udp_clnt.c logger.c -o clnt
```

#### Compiling the TCP prgrams  
- server 
```bash 
gcc tcp_ser.c network.c services.c -g -o ser
```
- client 
```bash 
gcc tcp_clnt.c services.c network.c -g  -o clnt 

```

