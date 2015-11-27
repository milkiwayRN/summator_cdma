#ifndef MYSERVER_H
#define MYSERVER_H
#include <QtWidgets/QWidget>
#include <QString>
class QTcpServer;
class QTextEdit;
class QTcpSocket;
//======================================================================
/*
  в данном классе сохраняется указатель на сокет,через который мы общаемся с конкретным пользователем,
 сообщение,которое он нам выслал и код, спомощью которого он его кодировал
 */
class Sender_socket
{
public:

QTcpSocket* my_client = NULL;

int* message;

int* code;

};

// ======================================================================
class MyServer : public QWidget
{
  Q_OBJECT

private:

    QTcpServer* m_ptcpServer;

    QTextEdit*  m_ptxt;

    quint16     m_nNextBlockSize;

    Sender_socket clients[4];

    int i;//индикатор кол-ва подключенных клиентов. Макс. - 4!

private:

    void sendToClient_code(Sender_socket* pSocket);

public:
//nPort - номер порта, который сумматор будет слушать
    MyServer(int nPort, QWidget* pwgt = 0);

public slots:

     void slotNewConnection();

     void slotReadClient();

     void CloseConnection(QTcpSocket *pClientSocket);
};
#endif // MYSERVER_H

