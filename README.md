# パズルゲーム - クロスプラットフォーム対応

このプロジェクトは Windows、Linux、macOS で動作するパズルゲームです。

## 必要な依存関係

### Windows
- CMake 3.10+
- MinGW-w64 or Visual Studio
- GLFW3 (既に含まれている)

### Linux (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install cmake build-essential libglfw3-dev libgl1-mesa-dev
```

### macOS
```bash
# Homebrewを使用
brew install cmake glfw
```

または

```bash
# MacPortsを使用
sudo port install cmake glfw
```

## ビルド手順

### Windows
```powershell
mkdir build
cd build
cmake ..
cmake --build .
```

### Linux
```bash
mkdir build
cd build
cmake ..
make
```

### macOS
```bash
mkdir build
cd build
cmake ..
make
```

## 実行

ビルド後、実行可能ファイルは `build/bin/` ディレクトリに生成されます：

### Windows
```powershell
cd build/bin
./game.exe
```

### Linux/macOS
```bash
cd build/bin
./game
```

## ゲームの遊び方

1. 数字キー（1-5）で列を選択
2. 残り3列になると+1が+2に変化
3. 青のAIは終盤戦略を持っています
4. Rキーでゲームリスタート

## 技術仕様

- OpenGL 3.3 Core Profile
- C++17
- GLFW for window management
- Custom bitmap font rendering
- PNG texture loading with stb_image
- Cross-platform CMake configuration

## トラブルシューティング

### macOS特有の問題

1. **OpenGL deprecation warning**: macOS 10.14以降でOpenGLが非推奨になりましたが、まだ使用可能です。
2. **Apple Silicon**: Universal Binaryとしてビルドされ、Intel/Apple Silicon両方で動作します。
3. **Gatekeeper**: 初回実行時に「開発元が未確認」の警告が出る場合があります。

### 依存関係の問題

各プラットフォームで必要なライブラリが見つからない場合：

#### Windows
- GLFWが見つからない場合、vcpkgを使用してインストール
- OpenGL32.libは通常Windowsに含まれています

#### Linux
- 追加のOpenGL関連パッケージが必要な場合があります
- `sudo apt install libglu1-mesa-dev freeglut3-dev mesa-common-dev`

#### macOS
- Xcodeコマンドラインツールが必要: `xcode-select --install`
- 古いmacOSバージョンでは OpenGL support が制限される場合があります
