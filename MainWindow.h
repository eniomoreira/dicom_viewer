#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include <QScrollArea>
#include <QLabel>
#include <QSlider>
#include <dcmtk/dcmimgle/dcmimage.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void loadDicom(const QString &filename);

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onWindowLevelChanged(int value);
    void onWindowWidthChanged(int value);

private:
    void updateDisplay();
    void createControlPanel();

    Ui::MainWindow *ui;

    QScrollArea *scrollArea;
    QLabel *imageLabel;

    QImage dicomImage;
    QString currentFile;
    
    // Controles
    QSlider *sliderWindowLevel;
    QSlider *sliderWindowWidth;
    QLabel *labelWindowLevel;
    QLabel *labelWindowWidth;

    // Metadados
    int rows = 0;
    int cols = 0;
    int bitsStored = 0;
    int windowWidth = 0;
    int windowLevel = 0;
    int defaultWindowWidth = 400;
    int defaultWindowLevel = 0;
    QString photometric;
    QString modality;
    
    // Imagem original
    DicomImage *currentDicomImage = nullptr;
};

#endif // MAINWINDOW_H