# monitoring-work
Two programs: a client and a server. Server - simple application to show current work activity of all employers in organization. Client - runs in the background and connects to the server.
The 1069 port is used

### Server cmdComs
- `list` - get the list of active clients
- `full list` - get the list of all clients and the date of last activity
- `screen` - get screenshot with further possibility to save it.

### Client argv
At startup you should specify the ip address `xxx.xxx.xxx.xxx.xxx` as a parameter, the previous ip will be used at subsequent startups.
