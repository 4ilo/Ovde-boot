#include "uploadfirmware.h"
#include "ui_uploadfirmware.h"

uploadFirmware::uploadFirmware(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::uploadFirmware)
{
    ui->setupUi(this);

    m_about = new About(this);
    m_about->setModal(true);
}

uploadFirmware::~uploadFirmware()
{
    delete ui;
}


//
//  De user wilt connectie maken met het target
//
void uploadFirmware::on_buttonConnect_clicked()
{
    m_tcpSocket = new QTcpSocket(this);
    connect(m_tcpSocket, SIGNAL(connected()),this,SLOT(targetConnected()));
    connect(m_tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(tcpError(QAbstractSocket::SocketError)));

    m_tcpSocket->connectToHost(QHostAddress(ui->input_ipAdress->text()), ui->input_poort->text().toInt());

    if(!m_tcpSocket->waitForConnected(3000))
    {
        this->tcpError(QAbstractSocket::ConnectionRefusedError);
    }

}

//
//  De applicatie heeft succesvol connectie gemaakt met het target
//
void uploadFirmware::targetConnected(void)
{
    ui->label_melding->setText("Connectie met target gelukt!");
    ui->label_melding->setStyleSheet("QLabel { color : green; }");

    ui->group_fileinput->setEnabled(true);

    connect(m_tcpSocket, SIGNAL(readyRead()),this,SLOT(tcpDataReady()));
    connect(m_tcpSocket, SIGNAL(disconnected()),this,SLOT(targetDisconnected()));

    // We vragen de bootloader versie
    m_dataState = BOOTLOADER_VERSION;
    m_tcpSocket->write("b",1);
}

//
//  Hier komen we als het target disconnect
//
void uploadFirmware::targetDisconnected(void)
{
    ui->label_melding->setText("Connectie met target is afgesloten!");
    ui->label_melding->setStyleSheet("QLabel { color : orange; }");

    // We resetten de ui
    ui->group_target->setEnabled(true);
    ui->group_fileinput->setEnabled(false);
    ui->group_upload->setEnabled(false);
}


//
//  Er is een error opgetreden bij de tcp connectie
//
void uploadFirmware::tcpError(QAbstractSocket::SocketError socketError)
{
    ui->label_melding->setText("Connectie met target mislukt.");
    ui->label_melding->setStyleSheet("QLabel { color : red; }");
}

//
//  Er is tcp data klaar om te lezen
//
void uploadFirmware::tcpDataReady(void)
{

    QByteArray data,fileData;
    QString stringData;

    switch(m_dataState)
    {
        case BOOTLOADER_VERSION:
            stringData = m_tcpSocket->readAll();
            ui->label_bootlVers->setText("Bootloader: " + stringData);
            this->askFirmWareVersion();

            break;

        case FIRMWARE_VERSION:
            stringData = m_tcpSocket->readAll();
            ui->label_firmVers->setText("Firmware: " + stringData);
            break;

        case UPLOAD_SIZE:
            if((stringData = m_tcpSocket->readAll()) == "ok")
            {
                // We sturen het aantal bytes dat we gaan versturen
                m_dataState = UPLOAD_FILE;
                m_tcpSocket->write((QString::number(m_inputFile->size())).toLocal8Bit());
            }
            break;


        case UPLOAD_FILE:
            stringData = m_tcpSocket->readAll();
            if(stringData == "ok2")
            {
                // We maken vooruitgang op de progressBar
                ui->progressBar_programming->setValue(ui->progressBar_programming->value() + 512);
                // We sturen nu elke keer 512Bytes van de file
                m_tcpSocket->write(m_inputFile->read(512));
            }
            if(stringData == "okEnd")
            {
                // Alle data is verstuurd en succesvol ontvangen op het target
                // We maken vooruitgang op de progressBar
                ui->progressBar_programming->setValue(m_inputFile->size());
                this->sendFileName();
            }
            break;

        case VERIFY_FILE:
            data = m_tcpSocket->readAll();
            stringData = data;
            // We kijken of de verificatie eerder niet mislukt is
            if(m_verificationFailed == true)
                return;

            if(stringData == "end")
            {
                // Het is gedaan
                ui->progressBar_verifying->setValue(m_inputFile->size());
                this->verificationSuccess();
                return;
            }

            // Er is data
            fileData = m_inputFile->read(512);
            if(fileData != data)
            {
                qDebug() << "recData:" << data << "\n" << "fileData:" << fileData << "\n";
                m_verificationFailed = true;
                this->verificationFailed();
                return;
            }

            m_tcpSocket->write("v",1);
            ui->progressBar_verifying->setValue(ui->progressBar_verifying->value() + 512);


            break;

        case RESET:
            // We slutien de tcp connectie af
            if(m_tcpSocket->isOpen())
                m_tcpSocket->close();
            break;

        case UPLOAD_NAME:
            // We moeten de filenaam sturen
            stringData = m_tcpSocket->readAll();

            if(stringData == "ok")
            {
                m_tcpSocket->write(m_fileinfo->fileName().toLocal8Bit());
            }
            else if(stringData == "okEnd")
            {
                this->verifyUploadedFile();
            }


            break;


        default:
            break;
    }
}

//
//  Vraag het target zijn firmware versie
//
void uploadFirmware::askFirmWareVersion(void)
{
    // We vragen om de firmware versie
    m_tcpSocket->write("f",1);
    m_dataState = FIRMWARE_VERSION;
}


//
//  De user wilt een file openen
//
void uploadFirmware::on_button_openFile_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,"Open programma",m_path,"Binary Files (*.bin)");

    // We slagen het pad op zodat we volgende keer ineens juist zitten
    m_fileinfo = new QFileInfo(fileName);
    m_path = m_fileinfo->absolutePath();

    m_inputFile = new QFile(fileName);

    if(!m_inputFile->open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this,"Oops","De file is niet te openen",QMessageBox::Ok,QMessageBox::Ok);
        return;
    }

    QDataStream file(m_inputFile);
    file.setByteOrder(QDataStream::LittleEndian);
    quint32 beginStack;
    file >> beginStack;

    if(beginStack != 0x2004E200)
    {
        QMessageBox::warning(this,"File niet geschikt","De file die u probeert de openen is ofwel niet geschikt als binary file, Of u hebt het project niet gecompileerd nadat u het voorbereidend script gerund hebt.",QMessageBox::Ok,QMessageBox::Ok);
        m_inputFile->close();
        return;
    }


    ui->label_inputFile->setText(m_fileinfo->fileName());
    ui->group_upload->setEnabled(true);
}

//
//  De user wilt de file uploaden naar het target
//
void uploadFirmware::on_button_upload_clicked()
{
    if(m_inputFile->isOpen())
        m_inputFile->close();

    if(!m_inputFile->open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this,"Oops","De file is niet te openen",QMessageBox::Ok,QMessageBox::Ok);
        return;
    }

    // We maken de progressbars leeg
    ui->progressBar_programming->setMaximum(m_inputFile->size());
    ui->progressBar_programming->setValue(0);
    ui->progressBar_verifying->setMaximum(m_inputFile->size());
    ui->progressBar_verifying->setValue(0);

    // We disabelen tijdelijk de reset knop!
    ui->button_reset->setEnabled(false);

    // We sturen het upload commando
    m_dataState = UPLOAD_SIZE;
    m_verificationFailed = 0;
    m_tcpSocket->write("u",1);

}

//
//  We gaan nu de file die we gestuurd hebben verifieren
//
void uploadFirmware::verifyUploadedFile(void)
{
    m_inputFile->close();
    if(!m_inputFile->open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this,"Oops","De file kan niet geopend worden voor verificatie",QMessageBox::Ok,QMessageBox::Ok);
        return;
    }

    m_dataState = VERIFY_FILE;
    // We vragen om alle data terug te sturen
    m_tcpSocket->write("v",1);
}

//
//  Er is iets misgegaan bij de verificatie
//
void uploadFirmware::verificationFailed(void)
{

    QMessageBox::warning(this,"Oops","Bij het verifieren van de file zijn fouten ontdekt.",QMessageBox::Ok,QMessageBox::Ok);
}

//
//  Alle data in flash is geverifierd!
//
void uploadFirmware::verificationSuccess(void)
{
    // Een paar meldingen
    QMessageBox::information(this,"Yes!","Uw programma is succesvol geupload!",QMessageBox::Ok,QMessageBox::Ok);
    ui->label_succes->setText("Uw programma is succesvol geupload! Herstart het target om het te laden.");
    ui->label_succes->setStyleSheet("QLabel { color : green; }");

    // Reset knop wordt weer actief
    ui->button_reset->setEnabled(true);
}


//
//  De user wilt het about window zien
//
void uploadFirmware::on_actionAbout_triggered()
{
    m_about->show();
}

//
//  De user wilt het target heropstarten
//
void uploadFirmware::on_button_reset_clicked()
{
    // We sturen het reset commando
    m_tcpSocket->write("r",1);

    m_dataState = RESET;
}

//
//  We moeten straks de filename versturen
//
void uploadFirmware::sendFileName(void)
{
    m_tcpSocket->write("n",1);
    m_dataState = UPLOAD_NAME;
}

//
//  User wilt een target zoeken in het netwerk
//
void uploadFirmware::on_button_findTarget_clicked()
{
    if(m_targetFinder == nullptr)
        delete m_targetFinder;

    m_targetFinder = new targetFinder(this);
    m_targetFinder->show();
    connect(m_targetFinder,SIGNAL(ipIsSelected(QString)),this,SLOT(getTargetIp(QString)));
}

//
//  Het target ip is gevonden
//
void uploadFirmware::getTargetIp(QString ipAdress)
{
    ui->input_ipAdress->setText(ipAdress);
    m_targetFinder->hide();
    //delete m_targetFinder;
}
