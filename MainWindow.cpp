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
    scroll_area = new QScrollArea(this);
    image_label = new QLabel(this);
    image_label->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    scroll_area->setWidget(image_label);
    scroll_area->setWidgetResizable(false);

    // Layout principal com a imagem e controles
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->addWidget(scroll_area, 1);
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
    label_window_level = new QLabel("WL: 0", this);
    label_window_level->setMinimumWidth(80);
    label_window_level->setAlignment(Qt::AlignRight);
    slider_window_level = new QSlider(Qt::Horizontal, this);
    slider_window_level->setRange(-2000, 4000);
    slider_window_level->setValue(0);
    slider_window_level->setMaximumWidth(300);
    levelLayout->addWidget(new QLabel("Window Level:", this));
    levelLayout->addWidget(slider_window_level);
    levelLayout->addWidget(label_window_level);
    levelLayout->addStretch();

    // ==================== Segunda linha: Window Width ====================
    QHBoxLayout *widthLayout = new QHBoxLayout();
    label_window_width = new QLabel("WW: 400", this);
    label_window_width->setMinimumWidth(80);
    label_window_width->setAlignment(Qt::AlignRight);
    slider_window_width = new QSlider(Qt::Horizontal, this);
    slider_window_width->setRange(1, 4000);
    slider_window_width->setValue(400);
    slider_window_width->setMaximumWidth(300);
    widthLayout->addWidget(new QLabel("Window Width:", this));
    widthLayout->addWidget(slider_window_width);
    widthLayout->addWidget(label_window_width);
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
    
    label_zoom = new QLabel("Zoom: 100%", this);
    label_zoom->setMinimumWidth(100);
    label_zoom->setAlignment(Qt::AlignCenter);
    
    QPushButton *resetZoomButton = new QPushButton("Reset Zoom", this);
    resetZoomButton->setMaximumWidth(100);
    
    buttonsLayout->addWidget(new QLabel("Zoom:", this));
    buttonsLayout->addWidget(zoomInButton);
    buttonsLayout->addWidget(zoomOutButton);
    buttonsLayout->addWidget(label_zoom);
    buttonsLayout->addWidget(resetZoomButton);
    buttonsLayout->addStretch();

    mainLayout->addLayout(levelLayout);
    mainLayout->addLayout(widthLayout);
    mainLayout->addLayout(buttonsLayout);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Conecta aos slots Window Level/Width
    connect(slider_window_level, &QSlider::valueChanged, this, &MainWindow::onWindowLevelChanged);
    connect(slider_window_width, &QSlider::valueChanged, this, &MainWindow::onWindowWidthChanged);
    connect(resetWLWWButton, &QPushButton::clicked, this, [this]() {
        slider_window_level->blockSignals(true);
        slider_window_width->blockSignals(true);
        
        slider_window_level->setValue(default_window_level);
        slider_window_width->setValue(default_window_width);
        
        slider_window_level->blockSignals(false);
        slider_window_width->blockSignals(false);
        
        window_level = default_window_level;
        window_width = default_window_width;
        label_window_level->setText(QString("WL: %1").arg(window_level));
        label_window_width->setText(QString("WW: %1").arg(window_width));
        updateDisplay();
    });

    // Conecta aos slots de Zoom
    connect(zoomInButton, &QPushButton::clicked, this, &MainWindow::onZoomIn);
    connect(zoomOutButton, &QPushButton::clicked, this, &MainWindow::onZoomOut);
    connect(resetZoomButton, &QPushButton::clicked, this, &MainWindow::onResetZoom);
}

MainWindow::~MainWindow()
{
    if (current_dicom_image) {
        delete current_dicom_image;
    }
    delete ui;
}

void MainWindow::loadDicom(const QString &filename)
{
    current_file = filename;

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
    ds->findAndGetUint16(DCM_BitsStored, tmp16); bits_stored = tmp16;

    double ww = 0.0, wl = 0.0;
    ds->findAndGetFloat64(DCM_WindowWidth, ww);
    ds->findAndGetFloat64(DCM_WindowCenter, wl);
    window_width = static_cast<int>(ww);
    window_level = static_cast<int>(wl);

    OFString str;
    ds->findAndGetOFString(DCM_PhotometricInterpretation, str);
    photometric = str.c_str();

    ds->findAndGetOFString(DCM_Modality, str);
    modality = str.c_str();

    // Liberar imagem anterior se existir
    if (current_dicom_image) {
        delete current_dicom_image;
    }

    // --- Imagem ---
    current_dicom_image = new DicomImage(filename.toStdString().c_str());

    if (current_dicom_image->getStatus() != EIS_Normal) {
        QMessageBox::critical(this, "Erro", "Erro ao decodificar imagem DICOM.");
        delete current_dicom_image;
        current_dicom_image = nullptr;
        return;
    }

    // Definir valores padrão ANTES de atualizar os sliders
    default_window_level = window_level;
    default_window_width = window_width > 0 ? window_width : 400;
    zoom_factor = 1.0;
    default_zoom_factor = 1.0;

    // Atualizar sliders com valores padrão
    slider_window_level->blockSignals(true);
    slider_window_width->blockSignals(true);
    
    slider_window_level->setValue(default_window_level);
    slider_window_width->setValue(default_window_width);
    
    slider_window_level->blockSignals(false);
    slider_window_width->blockSignals(false);

    updateZoomLabel();
    updateDisplay();
}

void MainWindow::onWindowLevelChanged(int value)
{
    window_level = value;
    label_window_level->setText(QString("WL: %1").arg(window_level));
    updateDisplay();
}

void MainWindow::onWindowWidthChanged(int value)
{
    window_width = value;
    label_window_width->setText(QString("WW: %1").arg(window_width));
    updateDisplay();
}

void MainWindow::onZoomIn()
{
    if (zoom_factor < max_zoom) {
        zoom_factor += zoom_step;
        if (zoom_factor > max_zoom) {
            zoom_factor = max_zoom;
        }
        updateZoomLabel();
        updateDisplay();
    }
}

void MainWindow::onZoomOut()
{
    if (zoom_factor > min_zoom) {
        zoom_factor -= zoom_step;
        if (zoom_factor < min_zoom) {
            zoom_factor = min_zoom;
        }
        updateZoomLabel();
        updateDisplay();
    }
}

void MainWindow::onResetZoom()
{
    zoom_factor = default_zoom_factor;
    updateZoomLabel();
    updateDisplay();
}

void MainWindow::updateZoomLabel()
{
    int percentage = static_cast<int>(zoom_factor * 100);
    label_zoom->setText(QString("Zoom: %1%").arg(percentage));
}

void MainWindow::updateDisplay()
{
    if (!current_dicom_image || current_dicom_image->getStatus() != EIS_Normal) {
        return;
    }

    // Aplicar Window Level e Width
    if (window_width > 0) {
        current_dicom_image->setWindow(window_level, window_width);
    } else {
        current_dicom_image->setMinMaxWindow();
    }

    const Uint8 *pixelData = (const Uint8 *)current_dicom_image->getOutputData(8);

    if (!pixelData) {
        QMessageBox::critical(this, "Erro", "Falha ao obter pixel data.");
        return;
    }

    int width = current_dicom_image->getWidth();
    int height = current_dicom_image->getHeight();

    QImage img(pixelData, width, height, QImage::Format_Grayscale8);
    dicom_image = img.copy();

    // Desenhar overlay
    QImage overlay = dicom_image.copy();
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
            .arg(window_width)
            .arg(window_level));

    painter.drawText(10, y + 20,
        QString("Bits: %1  Mod: %2")
            .arg(bits_stored)
            .arg(modality));

    painter.end();

    // Aplicar zoom
    int scaledWidth = static_cast<int>(overlay.width() * zoom_factor);
    int scaledHeight = static_cast<int>(overlay.height() * zoom_factor);
    QPixmap pixmap = QPixmap::fromImage(overlay);
    pixmap = pixmap.scaledToWidth(scaledWidth, Qt::SmoothTransformation);

    // Exibe
    image_label->setPixmap(pixmap);
    image_label->adjustSize();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
}