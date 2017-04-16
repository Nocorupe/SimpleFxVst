# SimpleFxVst
---------------------------------
シンプルなVST2.xオーディオエフェクトプラグイン

## 概要
* ゲームやメディアアートでの使用を想定したプラグイン群
* 少ないパラメータ + resume()/suspend()をサポート
* DLLは再頒布自由


## Effects
* Flanger
* Transe Gate
* Bit Crusher
* Delay
* Retrigger
* Peaking Equalizer
* Band Pass Filter
* Low Pass Filter
* High Pass Filter

## Build
* "VST3 SDK"を [Steinberg Website](http://www.steinberg.net/en/company/developer.html) からダウンロード、適宜展開。
* Windowsの環境変数に以下の値を追加
```
name : VST3SDK_DIR
value : "VST3 SDK"を展開したディレクトリパス
```
* VisualStudioを起動しソリューションをビルド

## License
[MIT](https://github.com/Nocorupe/SimpleFxVst/blob/master/LICENSE)

