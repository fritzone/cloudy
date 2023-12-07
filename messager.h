#ifndef MESSAGER_H
#define MESSAGER_H

#define MSG_IP_ENTERED              0x01
#define MSG_CONNECTED               0x02
#define MSG_CONNECTION_FAILED       0x03

int messager(int, void *data);

#endif // MESSAGER_H
