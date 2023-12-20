#ifndef MESSAGER_H
#define MESSAGER_H

// Emitted when the Enter key was pressed on the IP screen. Network will be advanced to TryConnect
#define MSG_IP_ENTERED                  0x01

// Emitted by TryConnect when the network layer managed to connect, but no message exchange took part yet. Network advances, Gui stays
#define MSG_CONNECTED                   0x02

// Emitted by TryConnect when the network connection failed. Network goes back
#define MSG_CONNECTION_FAILED           0x03

// Emitted when the network sent the ConnectionRequest message and received a ConnectionRequestReply message. Gui goes forward
// one step either to enter password screen or the folder browser screen
#define MSG_CONNECTION_ACKNOWLEDGED     0x04

int messager(int, void *data);

#endif // MESSAGER_H
