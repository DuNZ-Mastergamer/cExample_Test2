# cExample_Test2
This repository describes the possible tasks of the second programming test. (fork, pipe, dup2, exec.., socket)
!!!!!Bitte beachtet, dass dies nur ein grober struktureller Entwurf für den zweiten Test ist. Das bedeutet, 
    dass nicht alle Code-Teile 100% sauber gemacht sind bzw. nicht alles auf . und , genau erstellt wird.
    Es soll nur zeigen, was zu lernen ist und wie man es ungefähr hinbekommt.

    Weiters wichtig:
    Das Programm reverse.c kann als gegeben betrachtet werden. Für den Test ist nur wichtig, dass Server
    und Client so halbwegs im Kopf sind.
    Auch ist zu beachten, dass entweder ein Server ODER ein Client kommt. Niemals beide, wäre mir zumindest
    nicht bekannt. 
                                                                                                          !!!!!
                                                                                                          



#Der Ablauf:
-Client
Generell soll ein Client erstellt werden, welcher eine Zeile über
seine Options mitbekommt.
Aufruf --> ./client -h localhost -p XXXX -m "Your_Message"
Dabei gibt -h den host, -p den Port und -m eine Nachricht an.
Der Client soll diese korrekt verarbeiten und über einen Socket
eine Verbindung zu einem Server aufbauen. Der Client sendet
die Nachricht dann an den Server. Er wird schließlich eine
Antwort erhalten, welche die umgekehrte Nachricht ist.

BSP: Hello --> olleH

-Server
Der Server soll nur einen Client akzeptieren (ist der Masse an Code geschuldet).
In erster Linie soll er ein Setup des Sockets erstellen und damit erlauben, dass
ein Client angenomme werden kann. 
Aufruf --> ./server -p 8080
Weiters soll er Pipes und eine Kindprozess zur verfügung stellen. Der Kindprozess
soll über die Pipes die Nachricht des Clients weitergeleitet bekommen. Er soll
die erhaltene Nachricht rückwärts an den Server zurückgeben und dann terminieren.
Nachdem der Server die Umgekehrte Nachricht erhalten hat sendet er diese an 
den Client und terminiert.
