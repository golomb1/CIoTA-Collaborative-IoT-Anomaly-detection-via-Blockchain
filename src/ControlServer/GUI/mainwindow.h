//
// Created by master on 24/01/18.
//

#ifndef CIOTA_COMPLETE_PROJECT_MAIN_WINDOW_H
#define CIOTA_COMPLETE_PROJECT_MAIN_WINDOW_H


#include <QMainWindow>
#include <QMessageBox>
#include <headers/CIoTA/PrinterObserverListener.h>
#include <headers/AnomalyDetection/ExtendedAnomalyControlServer.h>
#include <headers/CppUtils/ThreadBarrier.h>

#define BUFFER_SIZE 1024


namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow, public Printer, public ExtendedAnomalyControlServerListener, public BlockchainListener
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void LogInfo(const char *fmt, ...) override;

    int
    selectActionPolicy(const std::string & identifier, char *ip, uint64_t score, std::map<int, std::string> &policies) override;

    bool acceptNewMember(char *ip) override;

    void onBlockAcceptance(const char *block, size_t len) override;

    void onPartialBlockAcceptance(const char *block, size_t len) override;

    void onPartialBlockAcceptance(const char *block, size_t len, bool selfUpdate) override;

    void setHashSize(size_t hashSize);

    void setCompleteBlockSize(size_t completeBlockSize) override;

signals:
    void LogData(const char*);
    bool askUserForMembership(const char*);
    int  askUserForPolicy(const std::string &, char *, uint64_t , std::map<int, std::string> &);
    void updateBlockCount(int);
    void updateProgress(int);

private slots:
    void on_exitBtn_clicked();
    void appendLog(const char*);
    bool requestUserMembership(const char *);
    int  requestPolicy(const std::string &, char *, uint64_t , std::map<int, std::string> &);
    void updateBlockLcd(int num);
    void updateProgressBar(int num);

private:
    Ui::MainWindow *ui;
    void resizeEvent(QResizeEvent* evt) override; // declare The members

    size_t _blockchainHashOutputSize;
    size_t _completeBlockSize;
};


#endif //CIOTA_COMPLETE_PROJECT_MAIN_WINDOW_H
