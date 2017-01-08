#ifndef TARGETFINDER_H
#define TARGETFINDER_H

#include <QDialog>
#include <QtNetwork>
#include <QtWidgets>

namespace Ui {
class targetFinder;
}

class targetFinder : public QDialog
{
    Q_OBJECT

signals:
    void ipIsSelected(QString ipAdress);

public:
    explicit targetFinder(QWidget *parent = 0);
    ~targetFinder();

private slots:
    void on_button_find_clicked();
    void readUdpData(void);
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

    void on_listWidget_itemDoubleClicked(QListWidgetItem *item);

    void on_btn_reset_clicked();

    void on_listWidget_itemClicked(QListWidgetItem *item);

private:
    Ui::targetFinder *ui;

    QUdpSocket * m_udpSocketSend;
    QUdpSocket * m_udpSocketGet;

    QString m_selectedIp;


};

#endif // TARGETFINDER_H
