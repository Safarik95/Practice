#pragma once

#include "program.h"

#include <QMainWindow>

class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTableWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    void buildUi();
    void refresh();
    void updateSelection();
    void showMessage(const Result &result, const QString &successText);
    bool readNumber(QLineEdit *edit, const QString &name, int &value);
    void addTrack(bool toFront);
    void removeTrack(bool first);
    void moveTracks();
    void changeDuration();
    void saveFile();
    void openFile();
    static QString formatDuration(qint64 seconds);

    Program program;
    QTableWidget *table = nullptr;
    QLineEdit *codeEdit = nullptr;
    QLineEdit *titleEdit = nullptr;
    QLineEdit *artistEdit = nullptr;
    QLineEdit *durationEdit = nullptr;
    QLineEdit *searchEdit = nullptr;
    QLineEdit *kEdit = nullptr;
    QComboBox *orderBox = nullptr;
    QPushButton *removeFirstButton = nullptr;
    QPushButton *removeLastButton = nullptr;
    QLabel *countLabel = nullptr;
    QLabel *totalLabel = nullptr;
    QLabel *selectedLabel = nullptr;
    QLabel *messageLabel = nullptr;
};
