#include <QtNetwork>
#include <QtGui>
#include "myserver.h"
#include <QMessageBox>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QTime>
#include <QString>
#include <QLabel>
#include <ctime>
//-----------------------------------------------------------------------

//функция генерирует рандомную последовательность 32 чисел из 1 и -1
int* get_cdma_code()
{
 srand(time(0));

 int* a = new int[32];

 for (int j = 0; j < 32;j++)
 {
     a[j] = rand()%2 - 1;

     if ( a[j] == 0 )
         a[j] = 1;

 }
return a;

}




// ----------------------------------------------------------------------
MyServer::MyServer(int nPort, QWidget* pwgt ) : QWidget(pwgt)
                                                    , m_nNextBlockSize(0)
{  //создается tcp сервер и ставится на прослушку заданного порта
    m_ptcpServer = new QTcpServer(this);

    if (!m_ptcpServer->listen(QHostAddress::Any, nPort)) {
        QMessageBox::critical(0,
                              "Server Error",
                              "Unable to start the server:"
                              + m_ptcpServer->errorString()
                             );
        m_ptcpServer->close();
        return;
    }

    connect(m_ptcpServer , SIGNAL(newConnection()) , this , SLOT(slotNewConnection()));
// создается бокс, в который будут выводиться все серверные сообщения
    m_ptxt = new QTextEdit;

    m_ptxt->setReadOnly(true);

    QVBoxLayout* pvbxLayout = new QVBoxLayout;

    pvbxLayout->addWidget(new QLabel("<H1>Сумматор</H1>"));

    pvbxLayout->addWidget(m_ptxt);

    setLayout(pvbxLayout);

    i = 0;//индикатор кол-ва подключенных клиентов. Макс. - 4!
}

// ----------------------------------------------------------------------
void MyServer::slotNewConnection()
{
// если уже подключено максималное количество пользователей - отсоединяем всех новых
    if (i>=4)
    {
        QTcpSocket* pClientSocket_err = m_ptcpServer->nextPendingConnection();

        pClientSocket_err->close();

        return;
    }
// если пришел, запрос на подключение и у нас еще есть места для пользователей - создаем сокет для общения с ним
// и отправляем ему код для кодирования сообщения

    QTcpSocket* pClientSocket = m_ptcpServer->nextPendingConnection();

    connect(pClientSocket, SIGNAL(disconnected()) , this , SLOT(CloseConnection( pClientSocket)));

    connect(pClientSocket, SIGNAL(readyRead()), this , SLOT(slotReadClient()));

    for ( int j = 0; j < 4 ; j++ )
      if(clients[i].my_client == NULL)
        {
          clients[i].my_client = pClientSocket;

          clients[i].code = get_cdma_code();

          sendToClient_code(&clients[i] );

          i++;
          break;
        }
}

// ----------------------------------------------------------------------
void MyServer::slotReadClient()
{
    QTcpSocket* pClientSocket = (QTcpSocket*)sender();

    QDataStream in(pClientSocket);

    in.setVersion(QDataStream::Qt_4_7);

// в цикле проверяем - а все ли данные нам уже пришли? Как только удостоверяемся,что все - продолжаем
    for (;;) {
        if (!m_nNextBlockSize)
          {
            if (pClientSocket->bytesAvailable() < (int)sizeof(quint16))
              {
                break;
              }
            in >> m_nNextBlockSize;
          }

        if (pClientSocket->bytesAvailable() < m_nNextBlockSize)
          {
            break;
          }

        QTime time = QTime::currentTime();

        Sender_socket* pcSocket;

//смотрим, а какой из клиентов отправил нам данные
        for (int j = 0; j < 4;j++)
            if (clients[j].my_client == pClientSocket)
            {
                pcSocket = &clients[j];
                break;
            }

         pcSocket->message = new int[32];

         qDebug() <<"Message: ";

//записываем полученные данные
         for(int j = 0 ; j < 32;j++)
         {
             in >>pcSocket->message[j];
             qDebug()<< pcSocket->message[j]<< " ";

         }


        QString strMessage =
            time.toString() + " " + "Client has sent message.";

        m_ptxt->append(strMessage);


        m_nNextBlockSize = 0;


    }
}

// ----------------------------------------------------------------------
void MyServer::sendToClient_code(Sender_socket* pSocket)
{
  //отправляем код новому пользователю
    QByteArray  arrBlock;

    QDataStream out(&arrBlock, QIODevice::WriteOnly);

    out.setVersion(QDataStream::Qt_4_7);

    out << quint16(0);

    qDebug()<<"Code: ";

    for (int j = 0; j < 32;j++)
    {
        out << pSocket->code[j];

        qDebug()<< pSocket->code[j]<<" ";

    }

qDebug()<<endl;

    out.device()->seek(0);

    out << quint16(arrBlock.size() - sizeof(quint16));

   (pSocket->my_client)->write(arrBlock);
}
void MyServer::CloseConnection(QTcpSocket* pClientSocket)
{

  Sender_socket* pcSocket;

//смотрим, а какой из клиентов отправил нам данные
  for (int j = 0; j < 4;j++)
      if (clients[j].my_client == pClientSocket)
      {
          pcSocket = &clients[j];
          break;
      }
  pcSocket->my_client = NULL;

  delete pcSocket->code;

  delete pcSocket->message;

  delete pClientSocket;

  m_ptxt->append("Connection was closed");

  i--;

}

