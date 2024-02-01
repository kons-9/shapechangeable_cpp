# shapechangeable_cpp
形状自在計算機の形状自在ディスプレイデモ用のプログラム

platformioでの実行を想定しています。マイコンはesp32c3を用いています。

data以下のものを書き込んでから使用しますが、macaddressをmacaddress_generater.pyで毎回更新してください。新しいmacaddressが振られるはずです。
hist以下に履歴を残していますが、一からマイコンにmacaddressを書き込む場合は削除してから行ってください

## directory
lib 以下の説明です。
.
├── README 
├── application TODO:アプリ作る
├── arch　ハードウェア依存のものです
├── common archで使用するクラスの抽象化です
├── display ディスプレイです。TODO:これもハードウェア依存
├── estimation 位置推定用です。TODO:システムアプリケーションの階層を作る
└── network　ネットワーク用です