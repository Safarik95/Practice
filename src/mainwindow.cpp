#include "mainwindow.h"

#include "storage.h"

#include <QComboBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    buildUi();
    refresh();
}

void MainWindow::buildUi()
{
    setWindowTitle("Музыкальная программа");
    resize(1100, 640);

    auto *central = new QWidget(this);
    auto *mainLayout = new QHBoxLayout(central);

    auto *leftLayout = new QVBoxLayout;
    auto *viewRow = new QHBoxLayout;
    searchEdit = new QLineEdit;
    searchEdit->setPlaceholderText("Поиск по фрагменту названия");
    searchEdit->setClearButtonEnabled(true);
    orderBox = new QComboBox;
    orderBox->addItems({"Обычный порядок", "Обратный порядок"});
    viewRow->addWidget(searchEdit, 1);
    viewRow->addWidget(orderBox);
    leftLayout->addLayout(viewRow);

    table = new QTableWidget(0, 4);
    table->setHorizontalHeaderLabels({"Код", "Название", "Исполнитель", "Длительность, с"});
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->verticalHeader()->setVisible(false);
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    leftLayout->addWidget(table, 1);

    countLabel = new QLabel;
    totalLabel = new QLabel;
    selectedLabel = new QLabel;
    auto *infoRow = new QHBoxLayout;
    infoRow->setSpacing(24);
    infoRow->addWidget(countLabel);
    infoRow->addWidget(totalLabel);
    infoRow->addWidget(selectedLabel);
    infoRow->addStretch();
    leftLayout->addLayout(infoRow);

    messageLabel = new QLabel;
    messageLabel->setWordWrap(true);
    leftLayout->addWidget(messageLabel);

    auto *rightLayout = new QVBoxLayout;

    auto *trackGroup = new QGroupBox("Номер");
    auto *form = new QFormLayout(trackGroup);
    codeEdit = new QLineEdit;
    codeEdit->setPlaceholderText("от 1 до 1 000 000");
    titleEdit = new QLineEdit;
    titleEdit->setPlaceholderText("от 1 до 100 символов");
    artistEdit = new QLineEdit;
    artistEdit->setPlaceholderText("от 1 до 100 символов");
    durationEdit = new QLineEdit;
    durationEdit->setPlaceholderText("секунды, от 1 до 86 400");
    form->addRow("Код:", codeEdit);
    form->addRow("Название:", titleEdit);
    form->addRow("Исполнитель:", artistEdit);
    form->addRow("Длительность:", durationEdit);
    auto *addFrontButton = new QPushButton("Добавить в начало");
    auto *addBackButton = new QPushButton("Добавить в конец");
    auto *changeButton = new QPushButton("Изменить длительность по коду");
    form->addRow(addFrontButton);
    form->addRow(addBackButton);
    form->addRow(changeButton);
    rightLayout->addWidget(trackGroup);

    auto *editGroup = new QGroupBox("Программа");
    auto *editLayout = new QVBoxLayout(editGroup);
    removeFirstButton = new QPushButton("Отменить первый номер");
    removeLastButton = new QPushButton("Отменить последний номер");
    auto *kRow = new QHBoxLayout;
    kEdit = new QLineEdit;
    kEdit->setPlaceholderText("k");
    kEdit->setMaximumWidth(70);
    auto *moveButton = new QPushButton("Перенести первые k в конец");
    kRow->addWidget(kEdit);
    kRow->addWidget(moveButton, 1);
    editLayout->addWidget(removeFirstButton);
    editLayout->addWidget(removeLastButton);
    editLayout->addLayout(kRow);
    rightLayout->addWidget(editGroup);

    auto *fileGroup = new QGroupBox("Файл");
    auto *fileLayout = new QHBoxLayout(fileGroup);
    auto *openButton = new QPushButton("Открыть...");
    auto *saveButton = new QPushButton("Сохранить...");
    fileLayout->addWidget(openButton);
    fileLayout->addWidget(saveButton);
    rightLayout->addWidget(fileGroup);
    rightLayout->addStretch();

    mainLayout->addLayout(leftLayout, 3);
    mainLayout->addLayout(rightLayout, 1);
    setCentralWidget(central);

    connect(addFrontButton, &QPushButton::clicked, this, [this] { addTrack(true); });
    connect(addBackButton, &QPushButton::clicked, this, [this] { addTrack(false); });
    connect(changeButton, &QPushButton::clicked, this, &MainWindow::changeDuration);
    connect(removeFirstButton, &QPushButton::clicked, this, [this] { removeTrack(true); });
    connect(removeLastButton, &QPushButton::clicked, this, [this] { removeTrack(false); });
    connect(moveButton, &QPushButton::clicked, this, &MainWindow::moveTracks);
    connect(openButton, &QPushButton::clicked, this, &MainWindow::openFile);
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::saveFile);
    connect(searchEdit, &QLineEdit::textChanged, this, &MainWindow::refresh);
    connect(orderBox, &QComboBox::currentIndexChanged, this, &MainWindow::refresh);
    connect(table, &QTableWidget::itemSelectionChanged, this, &MainWindow::updateSelection);
}

void MainWindow::refresh()
{
    const std::vector<Track> found = program.search(searchEdit->text());
    const bool reverse = orderBox->currentIndex() == 1;
    const std::vector<Track> shown = reverse ? std::vector<Track>(found.rbegin(), found.rend())
                                             : found;

    table->clearContents();
    table->setRowCount(static_cast<int>(shown.size()));
    for (int row = 0; row < table->rowCount(); ++row) {
        const Track &track = shown[static_cast<std::size_t>(row)];
        table->setItem(row, 0, new QTableWidgetItem(QString::number(track.code)));
        table->setItem(row, 1, new QTableWidgetItem(track.title));
        table->setItem(row, 2, new QTableWidgetItem(track.artist));
        table->setItem(row, 3, new QTableWidgetItem(QString::number(track.duration)));
    }

    QString count = QString("Номеров: %1 из %2").arg(program.size()).arg(Program::maxTracks);
    if (!searchEdit->text().trimmed().isEmpty())
        count += QString(", найдено: %1").arg(shown.size());
    countLabel->setText(count);
    totalLabel->setText(QString("Общая длительность: %1 с (%2)")
                            .arg(program.totalDuration())
                            .arg(formatDuration(program.totalDuration())));
    removeFirstButton->setEnabled(!program.isEmpty());
    removeLastButton->setEnabled(!program.isEmpty());
    updateSelection();
}

void MainWindow::updateSelection()
{
    const QList<QTableWidgetItem *> items = table->selectedItems();
    const Track *track = nullptr;
    if (!items.isEmpty()) {
        const QTableWidgetItem *codeItem = table->item(items.first()->row(), 0);
        if (codeItem != nullptr)
            track = program.findByCode(codeItem->text().toInt());
    }
    if (track == nullptr) {
        selectedLabel->setText("Выбран: нет");
        return;
    }
    selectedLabel->setText(QString("Выбран: %1 «%2»").arg(track->code).arg(track->title));
    codeEdit->setText(QString::number(track->code));
    durationEdit->setText(QString::number(track->duration));
}

void MainWindow::showMessage(const Result &result, const QString &successText)
{
    messageLabel->setStyleSheet(result.ok ? "color: #1b7f3b;" : "color: #c0392b;");
    messageLabel->setText(result.ok ? successText : "Ошибка: " + result.message);
}

bool MainWindow::readNumber(QLineEdit *edit, const QString &name, int &value)
{
    bool ok = false;
    value = edit->text().trimmed().toInt(&ok);
    if (!ok)
        showMessage(Result::failure(QString("поле «%1» должно содержать целое число.").arg(name)),
                    {});
    return ok;
}

void MainWindow::addTrack(bool toFront)
{
    Track track;
    if (!readNumber(codeEdit, "Код", track.code)
        || !readNumber(durationEdit, "Длительность", track.duration))
        return;
    track.title = titleEdit->text();
    track.artist = artistEdit->text();

    const Result result = toFront ? program.addFront(track) : program.addBack(track);
    showMessage(result, QString("Номер %1 добавлен в %2 программы.")
                            .arg(track.code)
                            .arg(toFront ? QString("начало") : QString("конец")));
    refresh();
    if (result.ok) {
        codeEdit->clear();
        titleEdit->clear();
        artistEdit->clear();
        durationEdit->clear();
    }
}

void MainWindow::removeTrack(bool first)
{
    QString text;
    if (!program.isEmpty()) {
        const Track &track = first ? program.tracks().front() : program.tracks().back();
        text = QString("Номер %1 «%2» отменён.").arg(track.code).arg(track.title);
    }
    const Result result = first ? program.removeFirst() : program.removeLast();
    showMessage(result, text);
    refresh();
}

void MainWindow::moveTracks()
{
    int k = 0;
    if (!readNumber(kEdit, "k", k))
        return;
    const Result result = program.moveFirstToEnd(k);
    showMessage(result, QString("Перенесено номеров в конец: %1.").arg(k));
    refresh();
}

void MainWindow::changeDuration()
{
    int code = 0;
    int duration = 0;
    if (!readNumber(codeEdit, "Код", code) || !readNumber(durationEdit, "Длительность", duration))
        return;
    const Result result = program.changeDuration(code, duration);
    showMessage(result, QString("Длительность номера %1 изменена на %2 с.").arg(code).arg(duration));
    refresh();
}

void MainWindow::saveFile()
{
    const QString path = QFileDialog::getSaveFileName(this, "Сохранить программу", QString(),
                                                      "JSON (*.json)");
    if (path.isEmpty())
        return;
    showMessage(Storage::save(program, path), "Программа сохранена в файл " + path);
}

void MainWindow::openFile()
{
    const QString path = QFileDialog::getOpenFileName(this, "Открыть программу", QString(),
                                                      "JSON (*.json)");
    if (path.isEmpty())
        return;
    const Result result = Storage::load(path, program);
    if (result.ok) {
        searchEdit->clear();
        orderBox->setCurrentIndex(0);
    }
    showMessage(result, QString("Загружено номеров: %1.").arg(program.size()));
    refresh();
}

QString MainWindow::formatDuration(qint64 seconds)
{
    const qint64 hours = seconds / 3600;
    const qint64 minutes = seconds % 3600 / 60;
    const qint64 rest = seconds % 60;
    return QString("%1:%2:%3")
        .arg(hours)
        .arg(minutes, 2, 10, QChar('0'))
        .arg(rest, 2, 10, QChar('0'));
}
