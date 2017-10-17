Examples
====

# 準備

1. bitcoindをインストールしておく。  
        regtestで動かすところからスクリプトにしているので、起動しておく必要はない。


# チャネル開始から送金まで

下のようにチャネルを開いた後、node_4444 --> node_3333 --> node_5555 という送金を行う。  
チャネルを2つ開くとき、それぞれ 10mBTC使う例になっているため、bitcoindに20mBTC以上入っていること。  
(行っている内容についてはスクリプトのコメントを参照)

        +-----------+         +-----------+         +-----------+
        | node_4444 +---------+ node_3333 +---------+ node_5555 |
        |           |         |           |         |           |
        +-----------+         +-----------+         +-----------+

1. ビルド直後の状態とする。  
        bitcoindは `example_st1.sh` で起動するため、立ち上げは不要。  
        前回exampleを動かしたのであれば、 `clean.sh` を実行してファイルを削除しておくこと。

2. ディレクトリ移動

        $ cd install

3. ノードを立てるための設定ファイルを作成する  
        ここで `bitcoind` の起動を行っている。

        $ ./example_st1.sh

4. ノードを起動する

        $ ./example_st2.sh

5. チャネルを開く  
        チャネルが開かれるまでスクリプトはポーリングでチェックしている。

        $ ./example_st3.sh

6. 送金を行う  
        送金前と送金後に、拡張子が.cnlのファイル(チャネル情報)を作るので、額はそれを比較するとよい。

        $ ./example_st4.sh

7. チャネルを閉じる  
        すぐにブロックチェーンに公開するが、内部情報はブロックに取り込まれるまで保持している。  
        その前に ucoind を停止させると使えないチャネル情報が残ってしまう。  
        `example_st1.sh` で起動した `bitcoind` を停止する処理も行っている。

        $ ./example_st5.sh

8. 不要ファイル削除  
        いくつか処理で使用したファイルが残っているので、気になるのであれば `clean.sh` を実行して削除する。