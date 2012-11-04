tinytwit
========

= whats this?
自作ゲームにツイート機能を付けたいと思い。
シンプルなツイートライブラリを作りました。

↓下記のソースコードを元に作っています
http://www.soramimi.jp/twicpp/


= how to use
使用するにはTwitterでいろいろ手続きをする必要があります。
https://dev.twitter.com/

- デベロッパー登録
- アプリケーション登録
- Consumer KeyとConsumer Secretを取得


libauthの派生物のらしいので、下記ライセンス文を含める必要があるようです。

 * Copyright 2007-2010 Robin Gareus <robin@gareus.org>
 * 
 * The base64 functions are by Jan-Henrik Haukeland, <hauk@tildeslash.com>
 * and un/escape_url() was inspired by libcurl's curl_escape under ISC-license
 * many thanks to Daniel Stenberg <daniel@haxx.se>.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.