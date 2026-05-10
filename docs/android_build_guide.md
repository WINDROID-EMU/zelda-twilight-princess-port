# Guia de Compilação para Android (ARM64)

Este guia explica como compilar o Dusk para Android e gerar um APK.

## 📋 Pré-requisitos

### 1. Android SDK e NDK

Instale o Android Studio ou apenas o SDK/NDK:

```bash
# Opção 1: Via Android Studio
# Baixe em: https://developer.android.com/studio

# Opção 2: Via command line tools (sem Android Studio)
wget https://dl.google.com/android/repository/commandlinetools-linux-11076708_latest.zip
unzip commandlinetools-linux-11076708_latest.zip
mkdir -p ~/Android/Sdk/cmdline-tools
mv cmdline-tools ~/Android/Sdk/cmdline-tools/latest

# Adicione ao PATH
export PATH="$HOME/Android/Sdk/cmdline-tools/latest/bin:$PATH"

# Instale SDK e NDK
sdkmanager "platform-tools" "platforms;android-34" "ndk;29.0.14206865"
```

### 2. JDK 17+

```bash
# Ubuntu/Debian
sudo apt install openjdk-17-jdk

# Arch Linux
sudo pacman -S jdk17-openjdk

# macOS
brew install openjdk@17
```

### 3. Variáveis de Ambiente

Adicione ao seu `~/.bashrc` ou `~/.zshrc`:

```bash
export ANDROID_HOME="$HOME/Android/Sdk"
export ANDROID_NDK_VERSION="29.0.14206865"
export JAVA_HOME="/usr/lib/jvm/java-17-openjdk"  # Ajuste conforme seu sistema
export PATH="$ANDROID_HOME/tools:$ANDROID_HOME/platform-tools:$PATH"
```

Verifique se estão corretas:

```bash
source ~/.bashrc
echo $ANDROID_HOME
echo $ANDROID_NDK_VERSION
echo $JAVA_HOME
```

### 4. CMake e Ninja

```bash
# Ubuntu/Debian
sudo apt install cmake ninja-build

# Arch Linux
sudo pacman -S cmake ninja

# macOS
brew install cmake ninja
```

### 5. Python 3

```bash
# Ubuntu/Debian
sudo apt install python3 python-is-python3

# Arch Linux
sudo pacman -S python python-markupsafe
```

## 🔄 Clone o Repositório

```bash
git clone --recursive https://github.com/TwilitRealm/dusk.git
cd dusk

# Atualize os submódulos (importante!)
git submodule update --init --recursive
```

## 🏗️ Passo 1: Compilar Bibliotecas Nativas

### Compilar para ARM64 (dispositivos físicos)

```bash
cmake --preset android-arm64
cmake --build --preset android-arm64
```

### Compilar para x86_64 (emuladores)

```bash
cmake --preset android-x86_64
cmake --build --preset android-x86_64
```

> **Nota**: A compilação pode levar 10-30 minutos dependendo da máquina.

### Saída da Compilação

Após a compilação, os arquivos serão gerados em:

```
build/android-arm64/Binaries/libmain.so
build/android-x86_64/Binaries/libmain.so
```

## 📦 Passo 2: Preparar Bibliotecas para o APK

Execute o script de staging para copiar as bibliotecas para o projeto Android:

```bash
./platforms/android/scripts/stage-jni-libs.sh
```

Este script copia:
- `libmain.so` (ARM64) → `platforms/android/app/src/main/jniLibs/arm64-v8a/`
- `libmain.so` (x86_64) → `platforms/android/app/src/main/jniLibs/x86_64/`

## 🔄 Passo 3: Sincronizar SDL Java (Opcional)

Se você atualizar o SDL e quiser atualizar os arquivos Java:

```bash
./platforms/android/scripts/sync-sdl-java.sh
```

> **Nota**: Normalmente não é necessário executar este passo.

## 📱 Passo 4: Compilar o APK

```bash
cd platforms/android
./gradlew :app:assembleDebug
```

### Saída do APK

O APK será gerado em:

```
platforms/android/app/build/outputs/apk/debug/app-debug.apk
```

### Compilar APK de Release (Opcional)

```bash
./gradlew :app:assembleRelease
```

Saída:
```
platforms/android/app/build/outputs/apk/release/app-release.apk
```

## 🚀 Passo 5: Instalar e Executar

### Instalar no dispositivo

```bash
# Conecte seu dispositivo Android via USB (habilite depuração USB)
adb devices  # Verifique se o dispositivo aparece

# Instale o APK
adb install -r platforms/android/app/build/outputs/apk/debug/app-debug.apk
```

### Executar com argumentos

```bash
# Exemplo: Executar com backend Vulkan
adb shell am start -n dev.twilitrealm.dusk/.DuskActivity \
  --es dusk_args "--backend vulkan"

# Exemplo: Executar com um jogo específico
adb shell am start -n dev.twilitrealm.dusk/.DuskActivity \
  --es dusk_args "--dvd /sdcard/Download/game.iso"
```

### Argumentos suportados:

- `dusk_args`: String única com argumentos (estilo shell)
- `dusk_argv`: Array de strings argv

## 🐛 Depuração

### Logs do Android

```bash
# Ver logs em tempo real
adb logcat -s "DuskActivity:I" "SDL:I" "dusk:*" "*:E"

# Filtrar apenas erros
tadb logcat *:E
```

### Debug via Android Studio

1. Abra o projeto em `platforms/android` no Android Studio
2. Conecte seu dispositivo
3. Clique em "Run" (Shift+F10)

## 🔧 Solução de Problemas

### Erro: "ANDROID_HOME not set"

```bash
export ANDROID_HOME="$HOME/Android/Sdk"
```

### Erro: "NDK version not found"

Verifique se o NDK está instalado:

```bash
ls $ANDROID_HOME/ndk/
# Deve mostrar: 29.0.14206865 (ou sua versão)
```

Se não estiver, instale:

```bash
sdkmanager "ndk;29.0.14206865"
```

### Erro: "CMake toolchain file not found"

Verifique se `ANDROID_NDK_VERSION` está correto:

```bash
echo $ANDROID_NDK_VERSION
ls $ANDROID_HOME/ndk/$ANDROID_NDK_VERSION/build/cmake/android.toolchain.cmake
```

### Erro de compilação: "aurora" não encontrado

O submódulo `aurora` é necessário. Verifique:

```bash
git submodule status
```

Se estiver vazio, inicialize:

```bash
git submodule update --init --recursive
```

### APK não instala

```bash
# Desinstale a versão anterior
adb uninstall dev.twilitrealm.dusk

# Instale novamente
adb install -r platforms/android/app/build/outputs/apk/debug/app-debug.apk
```

## 📁 Estrutura do Projeto Android

```
platforms/android/
├── app/
│   ├── build.gradle           # Configuração do app
│   └── src/
│       └── main/
│           ├── AndroidManifest.xml
│           ├── java/          # Código Java/SDL shim
│           ├── jniLibs/       # Bibliotecas nativas (.so)
│           │   ├── arm64-v8a/
│           │   │   └── libmain.so
│           │   └── x86_64/
│           │       └── libmain.so
│           └── res/           # Recursos Android
├── build.gradle               # Configuração do projeto
├── gradle/
├── gradlew                    # Wrapper Gradle
└── settings.gradle
```

## 📝 Resumo dos Comandos

```bash
# 1. Configurar ambiente
export ANDROID_HOME="$HOME/Android/Sdk"
export ANDROID_NDK_VERSION="29.0.14206865"
export JAVA_HOME="/usr/lib/jvm/java-17-openjdk"

# 2. Compilar nativo (ARM64)
cmake --preset android-arm64
cmake --build --preset android-arm64

# 3. Copiar bibliotecas
./platforms/android/scripts/stage-jni-libs.sh

# 4. Compilar APK
cd platforms/android
./gradlew :app:assembleDebug

# 5. Instalar
adb install -r app/build/outputs/apk/debug/app-debug.apk
```

## 📚 Recursos Adicionais

- [Documentação do CMake para Android](https://developer.android.com/ndk/guides/cmake)
- [Gradle Plugin for Android](https://developer.android.com/studio/releases/gradle-plugin)
- [SDL Android README](https://github.com/libsdl-org/SDL/blob/main/docs/README-android.md)

## ✅ Checklist Pré-compilação

- [ ] Android SDK instalado (`$ANDROID_HOME` configurado)
- [ ] Android NDK instalado (`$ANDROID_NDK_VERSION` configurado)
- [ ] JDK 17+ instalado (`$JAVA_HOME` configurado)
- [ ] CMake 3.25+ instalado
- [ ] Ninja instalado
- [ ] Python 3 instalado
- [ ] Repositório clonado com `--recursive`
- [ ] Submódulos inicializados (`git submodule update --init --recursive`)
- [ ] Dispositivo Android com modo desenvolvedor ativado (para testar)
