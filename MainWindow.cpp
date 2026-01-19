#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QFileDialog>
#include <QPainter>
#include <QResizeEvent>
#include <QMessageBox>
#include <QScrollArea>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QWidget>
#include <QPushButton>

// DCMTK
#include <dcmtk/dcmdata/dctk.h>
#include <dcmtk/dcmimgle/dcmimage.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    createControlPanel();

    // Criar um QScrollArea para navegação
    scrollArea = new QScrollArea(this);
    imageLabel = new QLabel(this);
    imageLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    scrollArea->setWidget(imageLabel);
    scrollArea->setWidgetResizable(false);

    // Layout principal com a imagem e controles
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->addWidget(scrollArea, 1);
    mainLayout->addWidget(ui->controlPanel, 0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    
    setCentralWidget(centralWidget);

    // Conecta o menu "Open"
    connect(ui->actionOpen, &QAction::triggered, this, [this]() {
        QString filename = QFileDialog::getOpenFileName(this,
            "Abrir DICOM", "", "DICOM Files (*.dcm);;All Files (*)");
        if (!filename.isEmpty()) {
            loadDicom(filename);
        }
    });

    connect(ui->actionExit, &QAction::triggered, this, &QMainWindow::close);
}

void MainWindow::createControlPanel()
{
    // Widget para os controles
    QWidget *controlPanel = new QWidget(this);
    ui->controlPanel = controlPanel;
    QVBoxLayout *mainLayout = new QVBoxLayout(controlPanel);

    // ==================== Primeira linha: Window Level ====================
    QHBoxLayout *levelLayout = new QHBoxLayout();
    labelWindowLevel = new QLabel("WL: 0", this);
    labelWindowLevel->setMinimumWidth(80);
    labelWindowLevel->setAlignment(Qt::AlignRight);
    sliderWindowLevel = new QSlider(Qt::Horizontal, this);
    sliderWindowLevel->setRange(-2000, 4000);
    sliderWindowLevel->setValue(0);
    sliderWindowLevel->setMaximumWidth(300);
    levelLayout->addWidget(new QLabel("Window Level:", this));
    levelLayout->addWidget(sliderWindowLevel);
    levelLayout->addWidget(labelWindowLevel);
    levelLayout->addStretch();

    // ==================== Segunda linha: Window Width ====================
    QHBoxLayout *widthLayout = new QHBoxLayout();
    labelWindowWidth = new QLabel("WW: 400", this);
    labelWindowWidth->setMinimumWidth(80);
    labelWindowWidth->setAlignment(Qt::AlignRight);
    sliderWindowWidth = new QSlider(Qt::Horizontal, this);
    sliderWindowWidth->setRange(1, 4000);
    sliderWindowWidth->setValue(400);
    sliderWindowWidth->setMaximumWidth(300);
    widthLayout->addWidget(new QLabel("Window Width:", this));
    widthLayout->addWidget(sliderWindowWidth);
    widthLayout->addWidget(labelWindowWidth);
    widthLayout->addStretch();

    // ==================== Terceira linha: Reset WL/WW e Zoom ====================
    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    
    // Reset WL/WW button
    resetWLWWButton = new QPushButton("Reset WL/WW", this);
    resetWLWWButton->setMaximumWidth(120);
    
    buttonsLayout->addWidget(resetWLWWButton);
    buttonsLayout->addSpacing(30);
    
    // Zoom controls
    QPushButton *zoomInButton = new QPushButton("Zoom +", this);
    zoomInButton->setMaximumWidth(80);
    
    QPushButton *zoomOutButton = new QPushButton("Zoom -", this);
    zoomOutButton->setMaximumWidth(80);
    
    labelZoom = new QLabel("Zoom: 100%", this);
    labelZoom->setMinimumWidth(100);
    labelZoom->setAlignment(Qt::AlignCenter);
    
    QPushButton *resetZoomButton = new QPushButton("Reset Zoom", this);
    resetZoomButton->setMaximumWidth(100);
    
    buttonsLayout->addWidget(new QLabel("Zoom:", this));
    buttonsLayout->addWidget(zoomInButton);
    buttonsLayout->addWidget(zoomOutButton);
    buttonsLayout->addWidget(labelZoom);
    buttonsLayout->addWidget(resetZoomButton);
    buttonsLayout->addStretch();

    mainLayout->addLayout(levelLayout);
    mainLayout->addLayout(widthLayout);
    mainLayout->addLayout(buttonsLayout);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Conecta aos slots Window Level/Width
    connect(sliderWindowLevel, &QSlider::valueChanged, this, &MainWindow::onWindowLevelChanged);
    connect(sliderWindowWidth, &QSlider::valueChanged, this, &MainWindow::onWindowWidthChanged);
    connect(resetWLWWButton, &QPushButton::clicked, this, [this]() {
        sliderWindowLevel->blockSignals(true);
        sliderWindowWidth->blockSignals(true);
        
        sliderWindowLevel->setValue(defaultWindowLevel);
        sliderWindowWidth->setValue(defaultWindowWidth);
        
        sliderWindowLevel->blockSignals(false);
        sliderWindowWidth->blockSignals(false);
        
        windowLevel = defaultWindowLevel;
        windowWidth = defaultWindowWidth;
        labelWindowLevel->setText(QString("WL: %1").arg(windowLevel));
        labelWindowWidth->setText(QString("WW: %1").arg(windowWidth));
        updateDisplay();
    });

    // Conecta aos slots de Zoom
    connect(zoomInButton, &QPushButton::clicked, this, &MainWindow::onZoomIn);
    connect(zoomOutButton, &QPushButton::clicked, this, &MainWindow::onZoomOut);
    connect(resetZoomButton, &QPushButton::clicked, this, &MainWindow::onResetZoom);
}

MainWindow::~MainWindow()
{
    if (currentDicomImage) {
        delete currentDicomImage;
    }
    delete ui;
}

void MainWindow::loadDicom(const QString &filename)
{
    currentFile = filename;

    DcmFileFormat file;
    OFCondition status = file.loadFile(filename.toStdString().c_str());

    if (!status.good()) {
        QMessageBox::critical(this, "Error", "The selected file is not a valid DICOM file");
        return;
    }

    DcmDataset *ds = file.getDataset();

    // --- Metadados ---
    Uint16 tmp16;
    ds->findAndGetUint16(DCM_Rows, tmp16);      rows = tmp16;
    ds->findAndGetUint16(DCM_Columns, tmp16);   cols = tmp16;
    ds->findAndGetUint16(DCM_BitsStored, tmp16); bitsStored = tmp16;

    double ww = 0.0, wl = 0.0;
    ds->findAndGetFloat64(DCM_WindowWidth, ww);
    ds->findAndGetFloat64(DCM_WindowCenter, wl);
    windowWidth = static_cast<int>(ww);
    windowLevel = static_cast<int>(wl);

    OFString str;
    ds->findAndGetOFString(DCM_PhotometricInterpretation, str);
    photometric = str.c_str();

    ds->findAndGetOFString(DCM_Modality, str);
    modality = str.c_str();

    // Liberar imagem anterior se existir
    if (currentDicomImage) {
        delete currentDicomImage;
    }

    // --- Imagem ---
    currentDicomImage = new DicomImage(filename.toStdString().c_str());

    if (currentDicomImage->getStatus() != EIS_Normal) {
        QMessageBox::critical(this, "Erro", "Erro ao decodificar imagem DICOM.");
        delete currentDicomImage;
        currentDicomImage = nullptr;
        return;
    }

    // Definir valores padrão ANTES de atualizar os sliders
    defaultWindowLevel = windowLevel;
    defaultWindowWidth = windowWidth > 0 ? windowWidth : 400;
    zoomFactor = 1.0;
    defaultZoomFactor = 1.0;

    // Atualizar sliders com valores padrão
    sliderWindowLevel->blockSignals(true);
    sliderWindowWidth->blockSignals(true);
    
    sliderWindowLevel->setValue(defaultWindowLevel);
    sliderWindowWidth->setValue(defaultWindowWidth);
    
    sliderWindowLevel->blockSignals(false);
    sliderWindowWidth->blockSignals(false);

    updateZoomLabel();
    updateDisplay();
}

void MainWindow::onWindowLevelChanged(int value)
{
    windowLevel = value;
    labelWindowLevel->setText(QString("WL: %1").arg(windowLevel));
    updateDisplay();
}

void MainWindow::onWindowWidthChanged(int value)
{
    windowWidth = value;
    labelWindowWidth->setText(QString("WW: %1").arg(windowWidth));
    updateDisplay();
}

void MainWindow::onZoomIn()
{
    if (zoomFactor < maxZoom) {
        zoomFactor += zoomStep;
        if (zoomFactor > maxZoom) {
            zoomFactor = maxZoom;
        }
        updateZoomLabel();
        updateDisplay();
    }
}

void MainWindow::onZoomOut()
{
    if (zoomFactor > minZoom) {
        zoomFactor -= zoomStep;
        if (zoomFactor < minZoom) {
            zoomFactor = minZoom;
        }
        updateZoomLabel();
        updateDisplay();
    }
}

void MainWindow::onResetZoom()
{
    zoomFactor = defaultZoomFactor;
    updateZoomLabel();
    updateDisplay();
}

void MainWindow::updateZoomLabel()
{
    int percentage = static_cast<int>(zoomFactor * 100);
    labelZoom->setText(QString("Zoom: %1%").arg(percentage));
}

void MainWindow::updateDisplay()
{
    if (!currentDicomImage || currentDicomImage->getStatus() != EIS_Normal) {
        return;
    }

    // Aplicar Window Level e Width
    if (windowWidth > 0) {
        currentDicomImage->setWindow(windowLevel, windowWidth);
    } else {
        currentDicomImage->setMinMaxWindow();
    }

    const Uint8 *pixelData = (const Uint8 *)currentDicomImage->getOutputData(8);

    if (!pixelData) {
        QMessageBox::critical(this, "Erro", "Falha ao obter pixel data.");
        return;
    }

    int width = currentDicomImage->getWidth();
    int height = currentDicomImage->getHeight();

    QImage img(pixelData, width, height, QImage::Format_Grayscale8);
    dicomImage = img.copy();

    // Desenhar overlay
    QImage overlay = dicomImage.copy();
    QPainter painter(&overlay);

    painter.setPen(Qt::green);
    painter.setFont(QFont("Monospace", 12));

    // Superior esquerdo
    painter.drawText(10, 20,
        QString("%1 x %2").arg(cols).arg(rows));

    painter.drawText(10, 40,
        QString("Photo: %1").arg(photometric));

    // Inferior esquerdo
    int y = overlay.height() - 60;

    painter.drawText(10, y,
        QString("WW: %1  WL: %2")
            .arg(windowWidth)
            .arg(windowLevel));

    painter.drawText(10, y + 20,
        QString("Bits: %1  Mod: %2")
            .arg(bitsStored)
            .arg(modality));

    painter.end();

    // Aplicar zoom
    int scaledWidth = static_cast<int>(overlay.width() * zoomFactor);
    int scaledHeight = static_cast<int>(overlay.height() * zoomFactor);
    QPixmap pixmap = QPixmap::fromImage(overlay);
    pixmap = pixmap.scaledToWidth(scaledWidth, Qt::SmoothTransformation);

    // Exibe
    imageLabel->setPixmap(pixmap);
    imageLabel->adjustSize();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
}