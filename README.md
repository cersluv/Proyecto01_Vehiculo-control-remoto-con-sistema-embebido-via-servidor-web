# Sistema Embebido para Control Remoto de Vehículo mediante Servidor Web

Sistema embebido a la medida para control remoto de vehículo autónomo mediante servidor web, implementado sobre Raspberry Pi 4 con imagen Linux personalizada usando Yocto Project.

---

## Tabla de Contenidos

- [Descripción General](#descripción-general)
- [Características Principales](#características-principales)
- [Arquitectura del Sistema](#arquitectura-del-sistema)
- [Requisitos del Sistema](#requisitos-del-sistema)
- [Instalación y Configuración](#instalación-y-configuración)
- [Compilación del Sistema](#compilación-del-sistema)
- [Documentación de la API](#documentación-de-la-api)
- [Uso del Sistema](#uso-del-sistema)
- [Estructura del Proyecto](#estructura-del-proyecto)
- [Resultados y Pruebas](#resultados-y-pruebas)


---

## Descripción General

Este proyecto implementa un sistema embebido optimizado para el control remoto de un vehículo autónomo mediante WiFi. El sistema utiliza una Raspberry Pi 4 con una imagen Linux mínima construida con Yocto Project, garantizando el uso eficiente de recursos.

### Objetivos Principales

- Desarrollar sistema embebido con recursos optimizados
- Control remoto mediante interfaz web/móvil vía WiFi
- Implementar desarrollo cruzado (cross-compilation)
- Transmisión de video en tiempo real
- Control de motores, luces y sensores mediante GPIO/PWM

---

##  Características Principales

### Funcionalidades Implementadas

#### Requerimientos Obligatorios

- **Control de Movimiento**: 8 direcciones (adelante, atrás, izquierda, derecha, adelante-izquierda, adelante-derecha, atrás-izquierda, atrás-derecha) con velocidad variable mediante PWM
- **Luces Indicadoras**: 4 LEDs (delanteras, traseras, direccionales) con control automático y manual
- **Captura de Imágenes**: Soporte para 2 cámaras (frontal y jardín) con codificación Base64
- **Biblioteca GPIO Personalizada**: libgpio propia para control de GPIO y PWM
- **Desarrollo Cruzado**: Compilación toolchain x86_64 para ARM
- **Sistema Operativo Mínimo**: Imagen Linux Yocto (poky-scarthgap-5.0.10)
- **Autenticación**: HTTP Basic con HMAC-SHA256
- **Servidor Web**: HTTP multihilo en C (puerto 5000)

#### Características Adicionales

- Control de velocidad variable (0-100%)
- Direccionales con efecto de parpadeo (blink)
- Soporte para múltiples cámaras USB
- Sistema de logs detallado
- Gestión de estado del vehículo en tiempo real

---

## Arquitectura del Sistema

### Diagrama de Arquitectura General

                    ┌─────────────────────────┐
                    │   Cliente Web/Móvil     │
                    │   (Frontend React)      │
                    └───────────┬─────────────┘
                                │ HTTP/WiFi
                                │ Puerto 5000
                    ┌───────────▼─────────────┐
                    │   Servidor Web C        │
                    │   (servidor_final.c)    │
                    │   - Autenticación       │
                    │   - API REST            │
                    │   - Multithreading      │
                    └───────────┬─────────────┘
                                │
                ┌───────────────┼───────────────┐
                │               │               │
        ┌───────▼──────┐ ┌─────▼─────┐ ┌──────▼──────┐
        │   libgpio    │ │  fswebcam │ │   OpenSSL   │
        │   - GPIO     │ │  - Cámara │ │   - Auth    │
        │   - PWM      │ │  - Captura│ │   - HMAC    │
        └───────┬──────┘ └───────────┘ └─────────────┘
                │
        ┌───────▼──────────────────────┐
        │   Hardware Raspberry Pi 4    │
        │                              │
        │   GPIOs:                     │
        │   - Motores: 515,516,517,518│
        │   - LEDs: 520,521,522,523   │
        │   - Cámaras: /dev/video0,1  │
        └──────────────────────────────┘


### Arquitectura de Software


**Backend (Sistema Embebido)**
- Lenguaje: C (ANSI C99)
- Servidor HTTP: Implementación custom multihilo
- Autenticación: OpenSSL (HMAC-SHA256)
- GPIO/PWM: Biblioteca libgpio personalizada
- Captura de imágenes: fswebcam
- Sistema Operativo: Linux customizado (Yocto)

**Frontend**
- Framework: React + Vite
- Comunicación: Fetch API con autenticación Basic
- Interfaz: Responsive (web y móvil)

**Build System**
- Yocto Project (Scarthgap 5.0.10)
- Meta-layers: meta-raspberrypi, meta-customlayer, meta-webcam
- Build system: Autotools + CMake
- Cross-compilation: SDK generado por Yocto

### Flujo de Control

1. **Inicialización**: Sistema bootea y ejecuta servidor automáticamente
2. **Autenticación**: Cliente se autentica con usuario/contraseña
3. **Control**: Cliente envía comandos HTTP POST/GET
4. **Procesamiento**: Servidor procesa y ejecuta en hardware
5. **Respuesta**: Estado actualizado retorna al cliente

---

## Requisitos del Sistema

### Hardware Requerido

- **Raspberry Pi 4**
- **Tarjeta microSD** 
- **Kit de vehículo RC**:
  - Chasis
  - 2x Motores DC con encoders
  - Puente H (controlador de motores)
  - 4x LEDs
  - Resistencias apropiadas
  - Fuente de alimentación (batería/powerbank)
- **Cámaras USB** (Compatibles con Video4Linux)
- **Conexión WiFi**

### Software Requerido (Host de Desarrollo)

- **Sistema Operativo**: Linux (Ubuntu 20.04+ recomendado)
- **Espacio en disco**: Mínimo 50GB libres
- **RAM**: 8GB mínimo (16GB recomendado)
- **Dependencias Yocto**:

    sudo apt-get install gawk wget git-core diffstat unzip texinfo gcc-multilib \
    build-essential chrpath socat cpio python3 python3-pip python3-pexpect \
    xz-utils debianutils iputils-ping python3-git python3-jinja2 libegl1-mesa \
    libsdl1.2-dev pylint3 xterm

---

## Instalación y Configuración

### 1. Preparación del Entorno Yocto

#### Clonar Poky

    cd ~
    git clone -b scarthgap git://git.yoctoproject.org/poky.git poky-scarthgap-5.0.10
    cd poky-scarthgap-5.0.10

#### Clonar Meta-Raspberrypi

    git clone -b scarthgap https://github.com/agherzan/meta-raspberrypi.git

#### Crear Meta-Layers Personalizados

    mkdir meta-customlayer
    mkdir meta-webcam

#### Copiar Configuraciones

Copiar los archivos del repositorio a las ubicaciones correspondientes:

    # Copiar configuraciones Yocto
    cp -r Yocto/conf/* rpi4/conf/
    cp -r Yocto/meta-customlayer/* meta-customlayer/
    cp -r Yocto/meta-webcam/* meta-webcam/

### 2. Configuración de Capas

Editar **conf/bblayers.conf**:

    POKY_BBLAYERS_CONF_VERSION = """2"""
    BBPATH = """${TOPDIR}"""
    BBFILES ?= """"""
    BBLAYERS ?= """ \
      /home/cers/Desktop/poky-scarthgap-5.0.10/meta \
      /home/cers/Desktop/poky-scarthgap-5.0.10/meta-poky \
      /home/cers/Desktop/poky-scarthgap-5.0.10/meta-yocto-bsp \
      /home/cers/Desktop/poky-scarthgap-5.0.10/meta-raspberrypi \
      /home/cers/Desktop/poky-scarthgap-5.0.10/meta-webcam \
      /home/cers/Desktop/poky-scarthgap-5.0.10/meta-customlayer \
    """

**Nota**: Ajustar las rutas según tu directorio de trabajo.

### 3. Configuración de la Máquina

Las configuraciones principales en **conf/local.conf** incluyen:

    MACHINE ?= """raspberrypi4"""
    INHERIT += """rm_work"""
    
    # Optimización de recursos
    BB_NUMBER_THREADS ?= """4"""
    PARALLEL_MAKE = """-j 4"""
    
    # Características adicionales
    EXTRA_IMAGE_FEATURES += """ssh-server-openssh"""
    IMAGE_INSTALL:append = """ wpa-supplicant openssh openssl openssh-sftp-server dhcpcd fswebcam libgpio"""
    
    # PWM configuration
    RPI_EXTRA_CONFIG:append = """\ndtoverlay=pwm-2chan,pin=18,func=2,pin2=19,func2=2"""

### 4. Configuración WiFi

Crear **wpa_supplicant.conf** en la partición boot de la SD:

    ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev
    update_config=1
    country=CR
    
    network={
        ssid="""TU_SSID"""
        psk="""TU_PASSWORD"""
        key_mgmt=WPA-PSK
    }

---

## Compilación del Sistema

### Compilar Imagen Base

#### Inicializar Entorno Build

    cd ~/poky-scarthgap-5.0.10
    source oe-init-build-env rpi4

#### Construir Imagen

    bitbake rpi-test-image

**Tiempo estimado**: 2-6 horas (primera compilación)

#### Generar SDK (Opcional pero Recomendado)

    bitbake rpi-test-image -c populate_sdk

El SDK se generará en: **tmp/deploy/sdk/**

### Instalación del SDK

    cd tmp/deploy/sdk/
    ./poky-glibc-x86_64-rpi-test-image-cortexa72-raspberrypi4-toolchain-5.0.10.sh

Instalar en ruta por defecto: **/opt/poky/5.0.10**

### Compilar Biblioteca libgpio

#### Configurar Entorno Cross-Compilation

    source /opt/poky/5.0.10/environment-setup-cortexa72-poky-linux

#### Compilar libgpio

    cd libgpio-1.0
    ./configure --host=aarch64-poky-linux --prefix=/usr
    make
    make install DESTDIR=$(pwd)/install

#### Crear Tarball para Yocto

    tar -czf libgpio-1.0.tar.gz libgpio-1.0/

Copiar a la ubicación esperada por la receta Yocto.

### Compilar Servidor

#### Preparar Código Fuente

    cd servidor-1.0
    autoreconf -i
    ./configure --host=aarch64-poky-linux
    make

#### Crear Tarball

    tar -czf servidor-1.0.tar.gz servidor-1.0/

### Reconstruir Imagen con Aplicaciones

    bitbake rpi-test-image

### Flashear Imagen a SD Card

#### Ubicar Imagen

    cd tmp/deploy/images/raspberrypi4/

#### Flashear con dd

    sudo dd if=rpi-test-image-raspberrypi4.wic of=/dev/sdX bs=4M status=progress
    sync

**Advertencia**: Reemplazar /dev/sdX con el dispositivo correcto.

#### Alternativa: Usar bmaptool

    sudo bmaptool copy rpi-test-image-raspberrypi4.wic /dev/sdX

---

## Documentación de la API

El servidor expone una API REST en el puerto **5000**. Todas las peticiones requieren autenticación HTTP Basic.

### Autenticación

**Método**: HTTP Basic Authentication

**Credenciales por defecto**:
- Usuario: **admin**
- Contraseña: **admin**

**Ejemplo de header**:

    Authorization: Basic YWRtaW46cHJveWVjdG8x

**Seguridad**: La contraseña se valida usando HMAC-SHA256 con salt ***diositoayuda***.

### Endpoints

#### 1. Control de Movimiento

**POST** `/mover`

Controla el movimiento del vehículo con velocidad variable.

**Request Body**:

    {
      """direccion""": """adelante""",
      """velocidad""": 60
    }

**Parámetros**:
- `direccion` (string): Dirección del movimiento
  - Valores: `"""adelante"""`, `"""atras"""`, `"""adelante_izquierda"""`, `"""adelante_derecha"""`, `"""atras_izquierda"""`, `"""atras_derecha"""`, `"""detener"""`
- `velocidad` (float): Velocidad en porcentaje (0-100)

**Response**:

    {
      """mensaje""": """Movimiento ejecutado""",
      """direccion""": """adelante""",
      """velocidad""": 60
    }

**Ejemplo cURL**:

    curl -X POST http://192.168.1.100:5000/mover \
      -u admin:proyecto1 \
      -H """Content-Type: application/json""" \
      -d """{\"direccion\":\"adelante\",\"velocidad\":75}"""

#### 2. Control de Luces

**POST** `/luz`

Controla las luces del vehículo.

**Request Body**:

    {
      """tipo""": """delantera"""
    }

**Parámetros**:
- `tipo` (string): Tipo de luz
  - Valores: `"""delantera"""`, `"""traseras"""`, `"""izquierda"""`, `"""derecha"""`

**Response**:

    {
      """mensaje""": """Luz controlada""",
      """luces""": """Delantera ON"""
    }

**Comportamiento**:
- Las luces delanteras/traseras se togglean (ON/OFF)
- Las direccionales activan efecto de parpadeo (500ms ON/OFF)

**Ejemplo cURL**:

    curl -X POST http://192.168.1.100:5000/luz \
      -u admin:proyecto1 \
      -H """Content-Type: application/json""" \
      -d """{\"tipo\":\"izquierda\"}"""

#### 3. Captura de Fotografía

**GET** `/tomar_foto?camara=frontal`

Captura una fotografía desde la cámara especificada.

**Query Parameters**:
- `camara` (string, opcional): Cámara a usar
  - Valores: `"""frontal"""` (/dev/video0)
  

**Response**:
Imagen codificada en Base64 (texto plano)

    /9j/4AAQSkZJRgABAQEAYABgAAD/2wBDAAgGBgcGBQgHBwcJCQgKDBQNDAsLDBkSEw8UHRof...

**Resolución**: 640x480

**Ejemplo cURL**:

    curl -X GET """http://192.168.1.100:5000/tomar_foto?camara=frontal""" \
      -u admin:proyecto1 \
      -o foto.b64

**Decodificar imagen**:

    base64 -d foto.b64 > foto.jpg

#### 4. Detener Vehículo

**POST** `/detener`

Detiene completamente el vehículo (motores y luces).

**Response**:

    {
      """mensaje""": """Carro detenido""",
      """direccion""": """Detenido""",
      """velocidad""": 0
    }

**Ejemplo cURL**:

    curl -X POST http://192.168.1.100:5000/detener \
      -u admin:proyecto1

### Códigos de Respuesta HTTP

- **200 OK**: Operación exitosa
- **400 Bad Request**: Solicitud malformada
- **401 Unauthorized**: Autenticación fallida
- **404 Not Found**: Endpoint no existe
- **500 Internal Server Error**: Error del servidor

### CORS

El servidor incluye headers CORS para permitir peticiones desde cualquier origen:

    Access-Control-Allow-Origin: *
    Access-Control-Allow-Methods: GET, POST, OPTIONS
    Access-Control-Allow-Headers: Content-Type, Authorization

---

## Uso del Sistema

### Primera Configuración

#### 1. Conectar Hardware

1. Conectar Raspberry Pi 4 a la circuitería del vehículo
2. Verificar conexiones GPIO según configuración:
   - **Motores**: Pines 515, 516, 517, 518
   - **LEDs**: Pines 520, 521, 522, 523
3. Conectar cámaras USB
4. Conectar fuente de alimentación

#### 2. Bootear Sistema

1. Insertar SD card con imagen flasheada
2. Encender Raspberry Pi
3. Esperar secuencia de boot (30-60 segundos)
4. El servidor se iniciará automáticamente

#### 3. Conectar a la Red

La Raspberry Pi se conectará automáticamente al WiFi configurado en **wpa_supplicant.conf**.

**Encontrar IP de la Raspberry**:

    # Desde el router o mediante nmap
    nmap -sn 192.168.1.0/24

### Usando el Frontend Web

#### 1. Configurar Frontend

    cd Frontend
    npm install

#### 2. Configurar IP del Servidor

Editar **src/App.jsx** y actualizar la IP:

    const API_BASE_URL = """http://192.168.1.100:5000""";

#### 3. Ejecutar Frontend

    npm run dev

#### 4. Acceder a la Interfaz

Abrir navegador en: **http://localhost:5173**

**Credenciales**:
- Usuario: **admin**
- Contraseña: **admin**

### Control Manual via API

#### Mover el vehículo adelante

    curl -X POST http://192.168.1.100:5000/mover \
      -u admin:proyecto1 \
      -H """Content-Type: application/json""" \
      -d """{\"direccion\":\"adelante\",\"velocidad\":80}"""

#### Activar luces delanteras

    curl -X POST http://192.168.1.100:5000/luz \
      -u admin:proyecto1 \
      -H """Content-Type: application/json""" \
      -d """{\"tipo\":\"delantera\"}"""

#### Capturar foto

    curl -X GET """http://192.168.1.100:5000/tomar_foto?camara=frontal""" \
      -u admin:proyecto1 > foto.b64
    base64 -d foto.b64 > foto.jpg

#### Detener vehículo

    curl -X POST http://192.168.1.100:5000/detener \
      -u admin:proyecto1

---

## Estructura del Proyecto

    Proyecto01_Vehiculo-control-remoto/
    │
    ├── auth/                          # Autenticación
    │   ├── authentication.c           # Implementación autenticación
    │   └── calcular_hash_admin.c      # Generador de hash
    │
    ├── Frontend/                      # Interfaz web React
    │   ├── public/
    │   ├── src/
    │   │   ├── App.jsx                # Componente principal
    │   │   ├── VideoStream.jsx        # Componente streaming
    │   │   └── App.css                # Estilos
    │   ├── package.json
    │   └── vite.config.js
    │
    ├── libgpio-1.0/                   # Biblioteca GPIO personalizada
    │   ├── include/
    │   │   ├── gpio.h                 # Header GPIO
    │   │   └── pwm.h                  # Header PWM
    │   ├── lib/
    │   │   ├── gpio.c                 # Implementación GPIO
    │   │   └── pwm.c                  # Implementación PWM
    │   ├── src/
    │   │   └── main.c                 # Programa de prueba
    │   ├── configure.ac               # Autotools config
    │   └── Makefile.am
    │
    ├── server/                        # Servidor web embebido
    │   └── server_final.c             # Implementación servidor
    │
    ├── tests/                         # Programas de prueba
    │   ├── gpio_test.c
    │   ├── pwm_test.c
    │   └── test_car.c
    │
    ├── Yocto/                         # Configuraciones Yocto
    │   ├── conf/
    │   │   ├── bblayers.conf          # Capas del proyecto
    │   │   └── local.conf             # Configuración local
    │   ├── meta-customlayer/          # Capa personalizada
    │   │   ├── recipes-gpio/
    │   │   │   └── libgpio/
    │   │   │       └── libgpio_1.0.bb
    │   │   └── recipes-server/
    │   │       └── servidor/
    │   │           ├── servidor_1.0.bb
    │   │           └── files/
    │   └── meta-webcam/               # Capa cámara
    │       └── recipes-multimedia/
    │           └── fswebcam/
    │               └── fswebcam_git.bb
    │
    ├── Raspberry/                     # Configs Raspberry
    │   ├── wifi                       # Script WiFi
    │   └── wpa_supplicant.conf        # Config WiFi
    │
    ├── libgpio-1.0.tar.gz             # Tarball libgpio
    └── README.md                      # Este archivo

---

## Resultados y Pruebas

### Métricas de Desempeño

#### Tamaño de la Imagen

- **Imagen base rpi-test-image**: ~100 MB
- **Imagen con aplicaciones**: ~120 MB

#### Recursos del Sistema

- **Uso de RAM en idle**: ~80 MB
- **Uso de RAM con servidor activo**: ~120 MB
- **Uso de CPU en idle**: <5%
- **Uso de CPU streaming foto**: ~25%

#### Tiempo de Boot

- **Booteo completo**: ~25 segundos
- **Servidor listo**: ~30 segundos

### Pruebas Funcionales

#### ✅ Control de Movimiento

| Prueba | Resultado | Observaciones |
|--------|-----------|---------------|
| Adelante | ✅ Exitoso | PWM funcional, velocidad variable |
| Atrás | ✅ Exitoso | Inversión correcta |
| Izquierda | ✅ Exitoso | Giro en sitio |
| Derecha | ✅ Exitoso | Giro en sitio |
| Adelante-Izquierda | ✅ Exitoso | Curva suave |
| Adelante-Derecha | ✅ Exitoso | Curva suave |
| Atrás-Izquierda | ✅ Exitoso | Reversión con giro |
| Atrás-Derecha | ✅ Exitoso | Reversión con giro |
| Control de velocidad | ✅ Exitoso | Rango 0-100% |

#### ✅ Sistema de Luces

| Prueba | Resultado | Observaciones |
|--------|-----------|---------------|
| Luces delanteras | ✅ Exitoso | Toggle ON/OFF |
| Luces traseras | ✅ Exitoso | Toggle ON/OFF |
| Direccional izquierda | ✅ Exitoso | Blink 500ms |
| Direccional derecha | ✅ Exitoso | Blink 500ms |
| Luces automáticas | ✅ Exitoso | Se activan según dirección |

#### ✅ Captura de Imágenes

| Prueba | Resultado | Observaciones |
|--------|-----------|---------------|
| Cámara frontal | ✅ Exitoso | Captura 640x480 |
| Cámara jardín | ✅ Exitoso | Captura 640x480 |
| Codificación Base64 | ✅ Exitoso | Transmisión correcta |
| Tiempo de captura | ✅ Exitoso | ~1-2 segundos |

#### ✅ Autenticación y Seguridad

| Prueba | Resultado | Observaciones |
|--------|-----------|---------------|
| Login válido | ✅ Exitoso | Acceso concedido |
| Login inválido | ✅ Exitoso | 401 Unauthorized |
| HMAC-SHA256 | ✅ Exitoso | Hash verificado |
| Sin autenticación | ✅ Exitoso | Acceso denegado |


#### Interfaz Web
![image_alt](https://github.com/cersluv/Proyecto01_Vehiculo-control-remoto-con-sistema-embebido-via-servidor-web/blob/43c4650b78ce6f5c9d63127c228cbd4d5ee53213/front_end.jfif)

#### Logs del Servidor

    [INIT] Inicializando sistema...
    [INIT] Sistema inicializado correctamente
    [SERVER] Escuchando en puerto 5000...
    
    [REQUEST] POST /mover?
    [MOVER] Dirección: adelante, Velocidad: 75%
    [MOTOR] Moviendo: adelante (velocidad: 75%)
    
    [REQUEST] POST /luz?
    [LUZ] Tipo: izquierda
    [LUCES] Controlando: izquierda
    
    [REQUEST] GET /tomar_foto?camara=frontal
    [FOTO] Solicitada cámara: frontal
    [FOTO] Capturando desde cámara: frontal (dispositivo: /dev/video1)
    [FOTO] Captura exitosa
    [FOTO] Enviando imagen (12845 bytes en base64)

---



## Contribuidores

Este proyecto fue desarrollado por:

- **Ricardo Borbón Mena** - Conexión WIFI, GPIO/PWM, autoboot.
- **Carlos Rodríguez Segura** - Integración Yocto, cross-compilation, desarrollo servidor embebido
- **Luis Felipe Jimenez** - Frontend React, interfaz de usuario
- **Jorginho, el detonador, Guillen** - Hardware, circuitería, integración física

### Institución

**Instituto Tecnológico de Costa Rica**  
Escuela de Ingeniería en Computadores  
Curso: CE-1113 Sistemas Empotrados  
Profesor: Dr.-Ing. Jeferson González Gómez  
Fecha: Octubre 2025

---

## Referencias

### Documentación Técnica

- [Yocto Project Documentation](https://docs.yoctoproject.org/)
- [Raspberry Pi Documentation](https://www.raspberrypi.org/documentation/)
- [Linux GPIO Documentation](https://www.kernel.org/doc/Documentation/gpio/)
- [OpenSSL Documentation](https://www.openssl.org/docs/)

### Meta-Layers Utilizados

- [meta-raspberrypi](https://github.com/agherzan/meta-raspberrypi)
- [meta-openembedded](https://github.com/openembedded/meta-openembedded)

### Herramientas

- [fswebcam](https://github.com/fsphil/fswebcam) - Captura de imágenes
- [React](https://react.dev/) - Framework frontend
- [Vite](https://vitejs.dev/) - Build tool

---

## Licencia

Este proyecto fue desarrollado con fines académicos para el curso CE-1113 Sistemas Empotrados del Instituto Tecnológico de Costa Rica. Código abierto.

---


##  Historial de Versiones

### v1.0.0 (Octubre 2025)
- ✅ Implementación inicial del sistema completo
- ✅ Control de movimiento con 8 direcciones
- ✅ Sistema de luces con parpadeo
- ✅ Captura de imágenes desde 2 cámaras
- ✅ Autenticación HMAC-SHA256
- ✅ Servidor web multihilo
- ✅ Frontend React responsive
- ✅ Imagen Yocto optimizada
- ✅ Biblioteca libgpio personalizada

---


##  Reconocimientos

Agradecimientos especiales a:

- **Dr.-Ing. Jeferson González Gómez** por la guía y enseñanza durante el curso
- **Comunidad Yocto Project** por la documentación y soporte
- **Comunidad Raspberry Pi** por los recursos y ejemplos
- **Instituto Tecnológico de Costa Rica** por los recursos proporcionados

---

## Apéndices

### Apéndice A: Configuración de Pines GPIO

| Función | Pin GPIO | Pin Físico | Dirección |
|---------|----------|------------|-----------|
| Motor IN1 | GPIO 515 | Pin gpio 3 | Output |
| Motor IN2 | GPIO 516 | Pin gpio 4 | Output |
| Motor IN3 | GPIO 517 | Pin gpio 5 | Output |
| Motor IN4 | GPIO 518 | Pin gpio 6 | Output |
| LED Adelante | GPIO 520 | Pin gpio 8 | Output |
| LED Atrás | GPIO 521 | Pin gpio 9 | Output |
| LED Derecha | GPIO 522 | Pin gpio 10 | Output |
| LED Izquierda | GPIO 523 | Pin gpio 11 | Output |
| PWM 0 | GPIO 18 | Pin 12 | Output |
| PWM 1 | GPIO 19 | Pin 35 | Output |

### Apéndice B: Esquema de Conexiones

#### Puente H (L298N)

    Raspberry Pi          L298N          Motores
    ───────────────────────────────────────────
    GPIO 515 (IN1) ───→ IN1
    GPIO 516 (IN2) ───→ IN2
    GPIO 517 (IN3) ───→ IN3          Motor Izq
    GPIO 518 (IN4) ───→ IN4          Motor Der
    GPIO 18 (PWM)  ───→ ENA
    GPIO 19 (PWM)  ───→ ENB
    
    GND ──────────────→ GND
    
    Batería (+) ──────→ +12V
    Batería (-) ──────→ GND

#### LEDs

    Raspberry Pi       Resistor      LED
    ───────────────────────────────────────
    GPIO 520 ───→ 220Ω ───→ LED+ ───→ GND
    GPIO 521 ───→ 220Ω ───→ LED+ ───→ GND
    GPIO 522 ───→ 220Ω ───→ LED+ ───→ GND
    GPIO 523 ───→ 220Ω ───→ LED+ ───→ GND


### Apéndice C: Comandos Útiles

#### Yocto/Bitbake

    # Limpiar receta específica
    bitbake -c cleansstate <receta>
    
    # Rebuild completo
    bitbake -c cleanall <receta>
    bitbake <receta>
    
    # Ver dependencias
    bitbake -g <receta>
    
    # Lista de tareas disponibles
    bitbake -c listtasks <receta>
    
    # Ver configuración
    bitbake -e <receta> | grep ^VARIABLE=

#### Raspberry Pi

    # Ver temperatura
    vcgencmd measure_temp
    
    # Ver voltaje
    vcgencmd get_config int | grep voltage
    
    # Ver memoria
    vcgencmd get_mem arm && vcgencmd get_mem gpu
    
    # Estado WiFi
    iwconfig wlan0
    
    # Logs del sistema
    journalctl -xe

#### GPIO

    # Exportar GPIO
    echo 520 > /sys/class/gpio/export
    
    # Configurar como output
    echo out > /sys/class/gpio/gpio520/direction
    
    # Encender LED
    echo 1 > /sys/class/gpio/gpio520/value
    
    # Apagar LED
    echo 0 > /sys/class/gpio/gpio520/value


### Apéndice D: Script de Instalación Rápida

Crear archivo **setup.sh**:

    #!/bin/bash
    
    # Script de instalación rápida
    # Uso: ./setup.sh
    
    set -e
    
    echo """=== Instalación Sistema Yocto ==="""
    
    # Colores
    RED="""\033[0;31m"""
    GREEN="""\033[0;32m"""
    NC="""\033[0m"""
    
    # Verificar sistema Linux
    if [[ """$OSTYPE""" != """linux-gnu"""* ]]; then
        echo """${RED}Error: Este script solo funciona en Linux${NC}"""
        exit 1
    fi
    
    # Instalar dependencias Yocto
    echo """${GREEN}Instalando dependencias...${NC}"""
    sudo apt-get update
    sudo apt-get install -y gawk wget git-core diffstat unzip texinfo \
        gcc-multilib build-essential chrpath socat cpio python3 python3-pip \
        python3-pexpect xz-utils debianutils iputils-ping python3-git \
        python3-jinja2 libegl1-mesa libsdl1.2-dev pylint3 xterm
    
    # Clonar Poky
    echo """${GREEN}Clonando Poky...${NC}"""
    if [ ! -d """poky-scarthgap-5.0.10""" ]; then
        git clone -b scarthgap git://git.yoctoproject.org/poky.git poky-scarthgap-5.0.10
    fi
    
    cd poky-scarthgap-5.0.10
    
    # Clonar meta-raspberrypi
    echo """${GREEN}Clonando meta-raspberrypi...${NC}"""
    if [ ! -d """meta-raspberrypi""" ]; then
        git clone -b scarthgap https://github.com/agherzan/meta-raspberrypi.git
    fi
    
    # Crear directorios
    mkdir -p meta-customlayer meta-webcam
    
    echo """${GREEN}Configuración completada!${NC}"""
    echo """Siguiente paso: Copiar configuraciones y ejecutar:"""
    echo """  source oe-init-build-env rpi4"""
    echo """  bitbake rpi-test-image"""

Ejecutar:

    chmod +x setup.sh
    ./setup.sh

---


### Recomendaciones

1. Leer completamente este README antes de iniciar

2. Realizar pruebas individuales de cada componente

3. Mantener backup de la imagen funcional

---

*Última actualización: Octubre 2025*  
*Versión del documento: 1.0.0*
