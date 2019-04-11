# Battleship_Server
server managing battleship game

server is using winsock lib for socket communication

server takes care of managing game logic, authenticated players can create new game room with spots for opponent and observers, where those can chat and participate in game session

commands are based on simple text protocol, updates of local client game state are done with only hit-miss server confirmations

![alt text](https://github.com/andrzejkowalczyk227/Battleship_Server/blob/master/sample_with_3_clients.png)
