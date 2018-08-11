//
// Created by master on 24/01/18.
//
#include <ctime>
#include <QtCore/QThread>
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //showMaximized();
    /*QPixmap bkgnd("./3132.jpg");  //Load pic
    bkgnd = bkgnd.scaled(this->size(), Qt::IgnoreAspectRatio); //set scale of pic to match the window
    QPalette palette;
    palette.setBrush(QPalette::Background, bkgnd);//set the pic to the background
    this->setPalette(palette); //show the background pic*/

    connect(this, SIGNAL(LogData(const char*)), this, SLOT(appendLog(const char*)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(askUserForMembership(const char *)), this, SLOT(requestUserMembership(const char *)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(askUserForPolicy(const std::string &, char *, uint64_t , std::map<int, std::string> &)),
            this, SLOT(requestPolicy(const std::string &, char *, uint64_t , std::map<int, std::string> &)),
            Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(updateBlockCount(int)), this, SLOT(updateBlockLcd(int)));
    connect(this, SIGNAL(updateProgress(int)), this, SLOT(updateProgressBar(int)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::acceptNewMember(char *ip) {
    bool res = emit askUserForMembership(ip);
    return res;
}

bool MainWindow::requestUserMembership(const char * ip){
    Q_ASSERT(QThread::currentThread() == this->thread());
    bool res = true;
    /*char buffer[BUFFER_SIZE];
    sprintf(buffer, "Device at IP %s requested to join the network.\nDo you approve?", ip);
    int reply;
    reply = QMessageBox::question(nullptr, "Membership Request",
                                  buffer,
                                  QMessageBox::Yes | QMessageBox::No);
    res = reply == QMessageBox::Yes;
    if(res) */
    {
        char b[BUFFER_SIZE];
        sprintf(b, "Device at IP %s join the network.", ip);
        this->ui->textEdit->append(b);
        this->ui->lcdNumber->display(ui->lcdNumber->value() + 1);
    }
    return res;
}


void MainWindow::appendLog(const char* text) {
    Q_ASSERT(QThread::currentThread() == this->thread());
    time_t lTime = time(nullptr);
    char buffer[1024];
    char *timeStr = asctime(localtime(&lTime));
    timeStr[strlen(timeStr) - 1] = 0;
    sprintf(buffer, "[%s] %s", timeStr, text);
    this->ui->textEdit_2->append(buffer);
}


void MainWindow::LogInfo(const char *fmt, ...){
    va_list ap;
    va_start(ap, fmt);
    char text[1024];
    vsnprintf(text, sizeof(text), fmt, ap);
    emit LogData(text);
}

int MainWindow::selectActionPolicy(const std::string &identifier, char *ip, uint64_t score, std::map<int, std::string> &policies) {
   return emit askUserForPolicy(identifier, ip, score, policies);
}


int MainWindow::requestPolicy(const std::string &identifier,
                              char *ip,
                              uint64_t score,
                              std::map<int, std::string> &policies)
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("Alert");
    char buffer[BUFFER_SIZE];
    sprintf(buffer, "Action is needed\nDevice: %s\nIP    : %s\nScore : %lu\nPlease select action:",
            identifier.c_str(), ip, score);

    msgBox.setText(tr(buffer));
    std::map<int, QPushButton *> map;
    for (const auto &it : policies) {
        map.insert(std::make_pair(it.first, msgBox.addButton(tr(it.second.c_str()), QMessageBox::ActionRole)));
    }

    sprintf(buffer, "------------------\nAction is needed\nDevice: %s\nIP    : %s\nScore : %lu",
            identifier.c_str(), ip, score);
    this->ui->textEdit->append(buffer);


    msgBox.exec();
    for (const auto &it : map) {
        if (msgBox.clickedButton() == it.second) {
            sprintf(buffer, "Selection: %d", it.first);
            this->ui->textEdit->append(buffer);
            return it.first;
        }
    }
    return -1;
}

void MainWindow::resizeEvent(QResizeEvent *evt)
{
    /*QPixmap bkgnd("./3132.jpg");//Load pic
    bkgnd = bkgnd.scaled(size(), Qt::IgnoreAspectRatio);//set scale of pic to match the window
    QPalette p = palette(); //copy current, not create new
    p.setBrush(QPalette::Background, bkgnd);//set the pic to the background
    setPalette(p);//show the background pic*/
    QMainWindow::resizeEvent(evt); //call base implementation
}

void MainWindow::on_exitBtn_clicked()
{
    // Attack
    this->close();
}


void MainWindow::updateBlockLcd(int num){
    this->ui->lcdNumber_2->display(num + 1);
    this->ui->progressBar->setValue(0);
}

void MainWindow::updateProgressBar(int num){
    this->ui->progressBar->setValue(num);
}

void MainWindow::onBlockAcceptance(const char *block, size_t ) {
    LogInfo("Accepted new block");
    emit updateBlockCount(static_cast<int>((BLOCK_ID(block))));
}

void MainWindow::onPartialBlockAcceptance(const char *block, size_t ) {
    LogInfo("Accepted new partial block with %d records", BLOCK_NUM_OF_RECORDS(block, _blockchainHashOutputSize));
    emit updateProgress(
            static_cast<int>(BLOCK_NUM_OF_RECORDS(block, _blockchainHashOutputSize) * 100 / _completeBlockSize));
}

void MainWindow::onPartialBlockAcceptance(const char *block, size_t len, bool ) {
    onPartialBlockAcceptance(block, len);
}

void MainWindow::setHashSize(size_t hashSize) {
    MainWindow::_blockchainHashOutputSize = hashSize;
}

void MainWindow::setCompleteBlockSize(size_t completeBlockSize) {
    MainWindow::_completeBlockSize = completeBlockSize;
}






