# DICOM Viewer

Um visualizador de imagens DICOM interativo desenvolvido em C++ com Qt6, oferecendo controle de Window Level/Width e zoom.

## Descrição da Solução

O DICOM Viewer é uma aplicação desktop que permite visualizar e manipular imagens médicas no formato DICOM. A solução oferece:

- **Carregamento de arquivos DICOM** via interface gráfica
- **Ajuste de Window Level (WL) e Window Width (WW)** através de sliders intuitivos
- **Controles de zoom** (ampliar, reduzir, resetar)
- **Exibição de metadados** da imagem (dimensões, modalidade, fotometria, bits armazenados)
- **Overlay de informações** sobre a imagem
- **Interface responsiva** com scroll para navegação em imagens grandes
- **Suporte a múltiplos codecs DICOM** (JPEG, RLE)

## Bibliotecas Utilizadas

- **Qt6 Widgets**: Framework para desenvolvimento da interface gráfica
- **DCMTK (DICOM ToolKit)**: Biblioteca para leitura e processamento de arquivos DICOM
- **CMake**: Sistema de build multiplataforma

## Instalação de Dependências

### 1. Instalar Qt6

```bash
sudo apt update
sudo apt install -y qt6-base-dev qt6-tools-dev
```

### 2. Instalar DCMTK

```bash
sudo apt install -y dcmtk libdcmtk-dev
```

### 3. Instalar CMake (se não estiver instalado)

```bash
sudo apt install -y cmake build-essential
```

## Passos para Compilação e Execução

### 1. Clonar o repositório

```bash
git clone <git@github.com:eniomoreira/dicom_viewer.git>
cd dicom_viewer
```

### 2. Criar diretório de build

```bash
mkdir build
cd build
```

### 3. Configurar o projeto com CMake

```bash
cmake ..
```

### 4. Compilar

```bash
cmake --build .
```

Ou, alternativamente:

```bash
make
```

### 5. Executar a aplicação

```bash
./dicom_viewer
```

## Uso da Aplicação

### Abrir um arquivo DICOM

1. Clique em **File → Open** (ou pressione **Ctrl+O**)
2. Selecione um arquivo `.dcm`
3. A imagem será carregada e exibida com seus metadados

### Ajustar Window Level e Width

- Use os **sliders** para ajustar Window Level (WL) e Window Width (WW)
- Os valores atuais são exibidos em tempo real
- Clique em **Reset WL/WW** para retornar aos valores padrão da imagem

### Controlar o Zoom

- **Zoom +**: Ampliar a imagem em 10% (até 500%)
- **Zoom -**: Reduzir a imagem em 10% (até 10%)
- **Reset Zoom**: Voltar para 100% (tamanho original)
- O zoom atual é exibido em porcentagem

### Navegação

- Use as **barras de scroll** para navegar em imagens que excedem o tamanho da janela
- Redimensione a janela livremente

## Estrutura do Projeto

```
dicom_viewer/
├── CMakeLists.txt          # Configuração do build
├── MainWindow.h            # Header da janela principal
├── MainWindow.cpp          # Implementação da janela principal
├── MainWindow.ui           # Interface gráfica (Qt Designer)
├── main.cpp                # Ponto de entrada da aplicação
├── README.md               # Este arquivo
└── LICENSE                 # Licença MIT
```

## Requisitos do Sistema

- Linux com suporte a X11 ou Wayland
- Qt 6.x ou superior
- DCMTK 3.6.x ou superior
- CMake 3.16 ou superior
- Compilador C++ com suporte a C++17

## Licença

MIT License - Veja o arquivo LICENSE para detalhes.

## Autor

Enio Moreira - 2026