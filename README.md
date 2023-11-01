# Alexandreias

## 1. Overview
テストコードやサンプルコード専用のプロジェクトを管理するリポジトリである．
マイコンやセンサーの動作確認や，ライブラリの動作確認などを行う．
マイコンの開発環境には PlatformIO 環境を用いているため，事前に環境構築を行う必要がある．

このリポジトリの存在意義は，プロジェクトにおけるソフトウェア開発を円滑に進めるためである．
例えば，特定のデバイスの動作確認を行う際に従来であればウェブからサンプルコードをその都度探す必要があり，手間がかかる．
しかし，それらのサンプルコードを蓄積しておけば，その都度サンプルコードを検索する手間は省ける．
また，動作確認のコードを「システムの部品」として扱いそれらを蓄積することで，システム作りの際に「部品を組み合わせる」だけで済み，開発の効率が上がることを期待している．

このリポジトリには，動作するサンプルプログラムのみ push することを許可する．

## 2. Directory 

ディレクトリ構造を以下に示す．
```
.
├── .git
├── .gitignore
├── README.md
└── samples/
```

## 3. Branch
ブランチ管理は，サンプルプロジェクト単位でブランチを作成し，ブランチ名には必ず Prefix を付ける．

**Example**

```shell
$ git branch
    master
    feature/NEO-6M_GPS
  * feature/MS4525DO
    feature/MCP2515_CAN
```

## 4. Setup Project

```samples/``` ディレクトリ直下にプロジェクトを作成するためのシェルスクリプトを配置した．

以下の例のようにコマンドを実行することで新規プロジェクトを作成できる．
第1引数にはディレクトリ名を，第2引数にはプロジェクト名を，第3引数にはボード名を入力する．
　

```shell
$ ls
  init_fm_arduino.sh
$ source init_fm_arduino.sh MCP2515_CAN RX esp32dev
  The following files/directories have been created in /Users/wasou/Alexandreias/samples/MCP2515_CAN/RX
  include - Put project header files here
  lib - Put project specific (private) libraries here
  src - Put project source files here
  platformio.ini - Project Configuration File
  Resolving esp32dev dependencies...
  Project has been successfully initialized!
```