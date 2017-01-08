#ifndef UPLOADFIRMWARE_H
#define UPLOADFIRMWARE_H

#include <QMainWindow>
#include <QtWidgets>
#include <QtNetwork>
#include "about.h"
#include "targetfinder.h"

namespace Ui {
    class uploadFirmware;
}

typedef enum { BOOTLOADER_VERSION, FIRMWARE_VERSION, UPLOAD_SIZE, UPLOAD_FILE, UPLOAD_NAME, VERIFY_FILE, RESET } ReceiveDataState;

class uploadFirmware : public QMainWindow
{
    Q_OBJECT

public:
    explicit uploadFirmware(QWidget *parent = 0);
    ~uploadFirmware();

private slots:
    void on_buttonConnect_clicked();
    void targetConnected(void);
    void tcpError(QAbstractSocket::SocketError socketError);
    void tcpDataReady(void);
    void targetDisconnected(void);
    void getTargetIp(QString);


    void on_button_openFile_clicked();
    void on_button_upload_clicked();
    void on_actionAbout_triggered();
    void on_button_reset_clicked();
    void on_button_findTarget_clicked();

private:
    Ui::uploadFirmware *ui;
    QTcpSocket * m_tcpSocket;
    ReceiveDataState m_dataState = BOOTLOADER_VERSION;
    QFile * m_inputFile;
    QString m_path = QDir::home().absolutePath();
    bool m_verificationFailed = false;
    About * m_about;
    targetFinder * m_targetFinder = nullptr;
    QFileInfo * m_fileinfo;



    void askFirmWareVersion(void);
    void verifyUploadedFile(void);
    void verificationFailed(void);
    void verificationSuccess(void);
    void sendFileName(void);

};

#endif // UPLOADFIRMWARE_H
