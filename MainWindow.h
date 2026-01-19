#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include <QScrollArea>
#include <QLabel>
#include <QSlider>
#include <QPushButton>
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
    void onZoomIn();
    void onZoomOut();
    void onResetZoom();

private:
    void updateDisplay();
    void createControlPanel();
    void updateZoomLabel();

    Ui::MainWindow *ui;

    QScrollArea *scrollArea;
    QLabel *imageLabel;

    QImage dicomImage;
    QString currentFile;
    
    // Controles Window Level/Width
    QSlider *sliderWindowLevel;
    QSlider *sliderWindowWidth;
    QLabel *labelWindowLevel;
    QLabel *labelWindowWidth;
    QPushButton *resetWLWWButton;

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
    
    // Zoom
    double zoomFactor = 1.0;
    double defaultZoomFactor = 1.0;
    const double zoomStep = 0.1;
    const double minZoom = 0.1;
    const double maxZoom = 5.0;
    QLabel *labelZoom;
    
    // Imagem original
    DicomImage *currentDicomImage = nullptr;
};

#endif // MAINWINDOW_H