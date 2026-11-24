#include <QApplication>
#include <QMainWindow>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QMainWindow window;
    window.setWindowTitle("Музыкальная программа");
    window.resize(1000, 640);
    window.show();
    return app.exec();
}
