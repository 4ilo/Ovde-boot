#include "targetfinder.h"
#include "ui_targetfinder.h"

targetFinder::targetFinder(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::targetFinder)
{
    ui->setupUi(this);

    m_udpSocketSend = new QUdpSocket(this);
    m_udpSocketGet = new QUdpSocket(this);
}

targetFinder::~targetFinder()
{
    delete ui;
    delete m_udpSocketGet;
    delete m_udpSocketSend;
}

//
//  De user wilt zoeken naar zijn target in het netwerk
//
void targetFinder::on_button_find_clicked()
{
    // We maken een nieuwe udp_connectie en sturen een broadcast message over het netwerk

    QHostAddress bcast(ui->input_broadcast->text());
    m_udpSocketSend->connectToHost(bcast, 65001);

    // Een udp_connectie om data te ontvangen van de target(s)

    m_udpSocketGet->bind(m_udpSocketSend->localAddress(),65000);
    connect(m_udpSocketGet,SIGNAL(readyRead()),this,SLOT(readUdpData()));


    // De effectieve data voor de broadcast
    m_udpSocketSend->write("discoverOvde-boot");

    m_udpSocketSend->disconnectFromHost();

}

//
//  Er heeft een target geantwoord
//
void targetFinder::readUdpData(void)
{
    char data[100];
    QHostAddress sender;
    m_udpSocketGet->readDatagram(data,100,&sender);

    QStringList splitData = QString::fromUtf8(data).split("-");

    if(splitData[0] == "targetFound")
    {
        ui->listWidget->addItem(sender.toString() + " ( " + splitData[1] + " ) ");
    }
}

//
//  User heeft op ok geklikt
//
void targetFinder::on_buttonBox_accepted()
{
    if(ui->listWidget->selectedItems().count() > 0)
    {
        m_selectedIp = ui->listWidget->selectedItems().at(0)->text().split(" ").at(0);
        m_udpSocketGet->close();
        m_udpSocketSend->close();
        emit ipIsSelected(m_selectedIp);
    }
    else
    {
        m_udpSocketGet->close();
        m_udpSocketSend->close();
        this->hide();
    }
}

//
//  User heeft op cancel geklikt
//
void targetFinder::on_buttonBox_rejected()
{
    m_udpSocketGet->close();
    m_udpSocketSend->close();
    this->hide();
}

//
// De user heeft een item geselecteerd
//
void targetFinder::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
    m_selectedIp = item->text().split(" ").at(0);

    m_udpSocketGet->close();
    m_udpSocketSend->close();

    emit ipIsSelected(m_selectedIp);
}

//
//  We vragen aan het target om zichzelf te resetten in bootloader mode
//
void targetFinder::on_btn_reset_clicked()
{
    // We maken een nieuwe udp_connectie en sturen een broadcast message over het netwerk
    QHostAddress target(m_selectedIp);
    m_udpSocketSend->connectToHost(target, 65001);

    // We sturen het reset commando
    m_udpSocketSend->write("restartOvde-boot");

    m_udpSocketSend->disconnectFromHost();
}

//
//  De user heeft een item in de lijst geselecteerd, we maken de reset knop actief
//
void targetFinder::on_listWidget_itemClicked(QListWidgetItem *item)
{
    ui->btn_reset->setEnabled(true);
    m_selectedIp = item->text().split(" ").at(0);
}
