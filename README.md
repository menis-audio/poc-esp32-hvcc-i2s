# ime-embarcados-lib

## Instalação e configuração inicial

### Ubuntu

#### 1. Instalando pré-requisitos

Execute o comando abaixo para instalar as dependências necessárias

``` bash
sudo apt-get install \
  git \
  wget \
  flex \
  bison \
  gperf \
  python3 \
  python3-pip \
  python3-venv \
  cmake \
  ninja-build \
  ccache \
  libffi-dev \
  libssl-dev \
  dfu-util \
  libusb-1.0-0
```

#### 2. Clonando ESP-IDF

Para desenvolver aplicativos para ESP32, é necessário instalar a sua framework de desenvolvimento oferecida pela **Espressif** no [repositório ESP-IDF](https://github.com/espressif/esp-idf).

Abra o terminal e execute

``` bash
mkdir -p ~/esp
cd ~/esp
git clone -b v5.2.1 --recursive https://github.com/espressif/esp-idf.git
```

para criar o diretório de instalação e clonar o repositório.

#### 3. Configurando tools

Além do ESP-IDF, é necessário instalar as tools utilizadas pelo ESP-IDF para projetos que suportam ESP32 (como compilador, debugger, pacotes Python, etc.)

Para isso, execute o comando

``` bash
cd ~/esp/esp-idf
./install.sh esp32
```

#### 4. Configurando variáveis ambientais

As tools instaladas ainda não foram adicionadas à variável de ambiente PATH. Para tornar as ferramentas utilizáveis na linha de comando, algumas variáveis de ambiente devem ser definidas. ESP-IDF fornece um script automatiza esse processo.

<!-- No terminal, onde vai usar o ESP-IDF, execute: -->
Execute, dentro de um diretório em que irá usar o ESP-IDF, o comando

``` bash
. $HOME/esp/esp-idf/export.sh
```

para configurar as variáveis de ambiente.

Para facilitar o uso frequente do ESP-IDF, recomenda-se a criação de um `alias`, que cria um atalho para o comando acima. Copie e cole o seguinte comando no perfil do seu shell *(.profile, .bashrc, .zprofile, etc.)*

``` bash
alias get_idf='. $HOME/esp/esp-idf/export.sh'
```

e reinicie o terminal. Agora você pode executar o comando `get_idf` para configurar as variáveis de ambiente automaticamente.

### Windows

#### 1. Instalando Git Bash

O **Git Bash** é um terminal que permite a execução de comandos do Git e de outros comandos do Unix no Windows. Baixe o Git Bash [neste link](https://gitforwindows.org/). Instale e configure em sua máquina de acordo com suas preferências.

#### 1.5 Instalando Windows Terminal (opcional)

O **Windows Terminal** é um emulador de terminal moderno e elegante. Baixe o Windows Terminal [neste link](https://apps.microsoft.com/detail/9n0dx20hk701?hl=pt-BR&gl=BR). Instale e configure  em sua máquina de acordo com suas preferências.

No Windows Terminal, é possível escolher o shell que você deseja utilizar. Dentre eles: Powershell, Prompt de Comando, Azure Cloud Shell, Git Bash e inclusive o próprio shell do ESP-IDF.

![Seleção de perfis com o Windows Terminal](windows-terminal-profiles.png)

#### 2. Instalando ESP-IDF

O **ESP-IDF Tools Installer** é um instalador que automaticamente configura as tools e variáveis de ambiente necessárias para o desenvolvimento de aplicativos para ESP32. Leia as instruções e baixe o instalador [neste link](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/windows-setup.html#).

As principais variáveis de ambiente configuradas pelo instalador são:

- `$IDF_PATH`: caminho para o diretório do ESP-IDF
- `$Env:Path`: PATH do sistema, que agora inclui os diretórios do ESP-IDF

#### 3. Acessando ambiente ESP-IDF

Após instalar o ESP-IDF Tools, é possível acessar um shell do ESP-IDF pelo atalho criado no menu Iniciar. Execute o atalho (*ESP-IDF X.X PowerShell* ou *ESP-IDF X.X CMD*). Se você tem o Windows Terminal, selecione o perfil ESP-IDF X.X. Aguarde a configuração do ambiente. Agora você pode executar comandos do ESP-IDF.

## Configuração do ime-embarcados-lib

Vamos clonar o repositório `ime-embarcados-lib` e configurar corretamente o ambiente para o desenvolvimento de aplicativos para ESP32.

### Ubuntu

Crie uma pasta que usará para seus projetos, navegue até ela e clone nosso repositório `ime-embarcados-lib` com o comando

``` bash
git clone --recursive git@github.com:viniciusfersil123/ime-embarcados-lib.git
```

e execute

``` bash
. $HOME/esp/esp-idf/export.sh
```

**OU** o alias correspondente

``` bash
get_idf
```

para configurar as variáveis de ambiente.

Ligue o ESP32 via USB em seu computador e execute

``` bash
ls /dev/tty*
```

para verificar a porta USB em que o ESP32 está conectado. Anote o nome da porta USB.

Execute

``` bash
idf.py set-target esp32
idf.py build
idf.py -p <PORT> flash
```

substituindo `<PORT>` pelo nome da porta USB em que o ESP32 está conectado (por exemplo `dev/ttyUSB0`). Se `<PORT>` não estiver disponível, o ESP-IDF tentará fazer o flash nas portas USB disponíveis.

*Obs.: Caso seu usuário não esteja autorizado a ler e escrever na porta USB referida, recomenda-se executar*

``` bash
sudo adduser <username> dialout
sudo chmod a+rw /dev/ttyUSB0
```

*substituindo `<username>` pelo seu nome de usuário.*

### Ferramentas Python (hvcc)

Em sistemas Debian/Ubuntu recentes, o `pip3` do sistema é "gerenciado externamente" (PEP 668) e não deve ser usado para instalar pacotes globalmente. Para usar o compilador `hvcc` dentro do repositório de forma reproduzível, há duas opções seguras:

- Opção A — pipx (para instalar a ferramenta como aplicativo):

```bash
sudo apt-get install pipx
pipx install hvcc
# usar
hvcc --help
```

- Opção B — ambiente virtual por repositório (recomendado para colaboração):

```bash
# prepara o venv e instala dependências (hvcc)
bash scripts/setup-python.sh

# usar via wrapper do repositório
bash scripts/hvcc.sh --help

# ou ativar manualmente
source .venv/bin/activate
hvcc --help
```

Observação: Não use `pip3 install hvcc` globalmente nem `--break-system-packages`. Use `pipx` ou o venv do repositório.

## Prova de Conceito: rodar um patch Pure Data com hvcc no ESP32

Esta PoC compila um patch simples do Pure Data com o `hvcc` e toca o áudio via I2S no ESP32.

### Visão geral
- Patch PD de teste em [main/pd/_main.pd](main/pd/_main.pd)
- Nome do patch Heavy: `mySynth` (definido em [main/hv_config.h](main/hv_config.h))
- Código gerado pelo `hvcc` em [main/hvcc/c](main/hvcc/c)
- Saída de áudio I2S em 48 kHz: BCLK=GPIO27, WS=GPIO26, DOUT=GPIO25 (ver [main/config.h](main/config.h))

### Passo a passo (Linux)
1) Preparar ambiente ESP-IDF (se ainda não está no PATH):
```bash
. $HOME/esp/esp-idf/export.sh
```

2) Preparar Python e instalar `hvcc` no venv do repositório:
```bash
bash scripts/setup-python.sh
bash scripts/hvcc.sh -V
```

3) (Opcional) Editar o patch PD de teste:
- Abra e ajuste [main/pd/_main.pd](main/pd/_main.pd). O patch atual é `osc~ 220` → `*~ 0.1` → `dac~`.

4) Gerar o código C/C++ do Heavy com `hvcc`:
```bash
# gera para a pasta main/hvcc/c usando o nome do patch mySynth
bash scripts/hvcc.sh -n mySynth -g c -o main/hvcc/c main/pd/_main.pd
```

5) Compilar o projeto:
```bash
idf.py build
```
- Dica: quando a pasta `main/hvcc/c` existe, o build inclui automaticamente os fontes gerados e define `HV_ENABLED=1`.

6) Conectar o ESP32 e fazer flash:
```bash
# deixe o ESP32 conectado; se necessário, informe a porta manualmente
idf.py flash              # auto-detecção
# ou
idf.py -p /dev/ttyUSB0 flash
```

7) Abrir o monitor serial para ver os logs de boot:
```bash
idf.py monitor -p /dev/ttyUSB0
```
- Você deverá ouvir um tom de 220 Hz em nível baixo (ganho 0.1). Se o volume estiver muito baixo, veja a seção "Ajustar nível" abaixo.

### Fiação I2S (DAC)
- BCLK: GPIO27
- WS/LRCLK: GPIO26
- DOUT: GPIO25
- 3V3 → 3V0, GND → GND
- Ver [main/config.h](main/config.h) para confirmar/mudar pinos e taxa de amostragem.

### Ajustar nível
- O envio para I2S aplica uma redução extra de 0.5 para evitar clip.
- Para aumentar o volume, edite `to_audio_write()` em [main/config.h](main/config.h) e altere `* 0.5f` para `* 1.0f`.
- Alternativamente, aumente o ganho no patch PD (por exemplo, `*~ 0.2`).

### Troubleshooting
- Porta serial ocupada/inexistente:
  - Feche `idf.py monitor` antes de `idf.py flash`.
  - Liste portas disponíveis:
    ```bash
    ls -l /dev/ttyUSB* /dev/ttyACM*
    ```
  - Informe explicitamente: `idf.py -p /dev/ttyUSB0 flash`.
  - Permissões: adicione seu usuário ao grupo `dialout` e replugue o dispositivo:
    ```bash
    sudo usermod -aG dialout "$USER"
    # depois, desconecte e reconecte o ESP32
    ```
- Sem som:
  - Verifique fiação dos pinos (BCLK/WS/DOUT) e taxa de amostragem.
  - Confirme que `main/hvcc/c` contém `Heavy_mySynth.*` e que o build definiu `HV_ENABLED`.
  - Aumente o ganho conforme seção acima.
- `hvcc` não encontrado:
  - Rode `bash scripts/setup-python.sh` e use o wrapper `bash scripts/hvcc.sh`.

### Comandos rápidos
```bash
# preparar ambiente
. $HOME/esp/esp-idf/export.sh && bash scripts/setup-python.sh
# gerar heavy
bash scripts/hvcc.sh -n mySynth -g c -o main/hvcc/c main/pd/_main.pd
# build + flash + monitor
idf.py build && idf.py -p /dev/ttyUSB0 flash && idf.py -p /dev/ttyUSB0 monitor
```

### Windows

Crie uma pasta que usará para seus projetos, navegue até ela e clone nosso repositório `ime-embarcados-lib` com o comando

``` bash
git clone --recursive git@github.com:viniciusfersil123/ime-embarcados-lib.git
```

e execute os comandos abaixo

``` bash
idf.py set-target esp32
idf.py build
idf.py -p COM<PORT> flash
```

para configurar e fazer o flash no ESP32, substituindo `<PORT>` pela porta COM em que o ESP32 está conectado (por exemplo `COM3`).

## Ligação do ESP32 com DAC

Para conectar o ESP32 com o DAC, ligue os pinos seguindo a tabela abaixo:

| ESP32  |DAC |
| ------------- | ------------- |
| 3V3  | 3V0  |
| GND | GND  |
| GPIO26 | WSEL  |
| GPIO25 | DIN|
| GPIO27 |BCLK  |
