# watermark 鈥?鍙 + 涓嶅彲瑙佺洸姘村嵃

## 鎬昏

| 绫诲埆 | 绠楁硶 | 椴佹鎬?| 鍙 | 鐢ㄩ€?|
|------|------|--------|------|------|
| 鍙鏂囧瓧 | `cv::putText` + alpha blend | 楂?| 鏄?| 鐗堟潈鏍囪瘑銆乁I 姘村嵃 |
| 鍙 logo | 澶氭骞抽摵 + alpha blend (RGBA 鍚?alpha 閫氶亾鍋?mask) | 楂?| 鏄?| 鍝佺墝骞抽摵 |
| 涓嶅彲瑙?DFT 鍩熺洸姘村嵃 (Cox 1997 绫? | 淇敼 DFT 骞呭害璋变腑棰戯紝鍔犲叡杞绉?| 涓紙JPEG 鎶楋紝閲嶉噰鏍峰急锛?| 鍚?| 鐗堟潈杩借釜 |
| 涓嶅彲瑙?DCT 鍩熸按鍗?(鏈?demo 鏈疄鐜? | 宓屽叆涓 DCT 鍧?| 涓?| 鍚?| DCT 鍩熸按鍗?|

## 鍙姘村嵃

### 鏂囧瓧姘村嵃
```
overlay = src.clone()
putText(overlay, "TRAE 2026", bottom-right, font, scale, white, thick)
putText(overlay, "...", +1/+1, black, ...)                  # 鎻忚竟
out = (1鈭捨?路src + 伪路overlay    # 伪=0.35
```

### Logo 姘村嵃 (RGBA)
- 4 閫氶亾 logo 鐢ㄧ 4 閫氶亾 (alpha) 鍋?mask锛歚mask = alpha > 0`
- 鍙湪 mask 鍖哄煙鍋氬姞鏉冭瀺鍚堬紝閬垮厤 logo 閫忔槑鐭╁舰瑕嗙洊鑳屾櫙
- 涔熷彲浠ョ敤 `BORDER_REPLICATE` + `cv::seamlessClone` 鍋氭棤缂濆悎鎴愶紙鏇撮珮绾э級

## 涓嶅彲瑙?DFT 鍩熺洸姘村嵃

### 宓屽叆娴佺▼

```
1. 鐏板害鍥?鈫?鏈€浼?DFT 灏哄 (getOptimalDFTSize) padding
2. dft(padded) 鈫?澶嶆暟 planes (re, im)
3. mag = 鈭?re虏 + im虏) ; phase = atan2(im, re)
4. logMag = log(mag + 1)
5. 鎶?32脳32 浜屽€兼按鍗颁綅宓屽叆鍒?(cy卤r, cx卤r) 鍏辫江浣嶇疆:
       logMag[cy+dy, cx+dx] += 卤伪    (姘村嵃浣?1鍒?伪, 鍚﹀垯-伪)
       logMag[cy-dy, cx-dx] += 卤伪    (鍏辫江, 淇濇寔 Hermitian 瀵圭О, 瀹?IDFT)
6. mag' = exp(logMag)
7. (re', im') = polarToCart(mag', phase)              鈫?鐢?OpenCV 鍐呯疆
8. idft(re', im') 鈫?杈撳嚭 8U
```

鍏抽敭绾︽潫锛欴FT 鐨?Hermitian 瀵圭О鎬?(澶嶅叡杞绉? 蹇呴』淇濇寔锛屽惁鍒?IDFT 杈撳嚭鍚?铏氶儴锛岀伆搴﹀浘浼氬け鐪熴€傛墍浠?*蹇呴』**鍚屾椂淇敼 `(cy+dy, cx+dx)` 涓?`(cy-dy, cx-dx)`
涓や釜鍏辫江浣嶇疆銆?
### 鎻愬彇娴佺▼

闇€瑕?鍘熷鏈祵鍏ュ浘"浣滀负鍙傝€冿紙闈炵洸鎻愬彇锛涚洸鎻愬彇闇€瑕佹洿澶嶆潅鐨勬按鍗扮粺璁￠噺
浼肩劧姣旀娴嬶紝鏈?demo 绠€鍖栦负鍙傝€冪増锛夛細

```
1. 瀵?marked 涓?ref 閮藉仛 DFT, 鍙?log magnitude
2. 鍦ㄥ祵鍏ヤ綅缃彇 (mag_marked 鈭?mag_ref) 鐨勫潎鍊?3. 闃堝€? >0 鈫?255 (姘村嵃浣?1), 鍚﹀垯 鈫?0 (姘村嵃浣?0)
```

## 杩愯

```powershell
cd out\algorithms
.\watermark.exe                                      # lena
.\watermark.exe ..\..\data\images\VCG2.jpg
```

杈撳嚭 4 寮犲浘鍒?`out/algorithms/`锛?- `watermark_visible_text.png` (鍙鏂囧瓧)
- `watermark_visible_logo.png` (鍙 logo, 鑻?`data/images/opencv-logo.png` 瀛樺湪)
- `watermark_invisible_marked.png` (鍚按鍗扮殑鍥? 瑙嗚涓婁笌鍘熷浘鍑犱箮鏃犲樊)
- `watermark_invisible_extracted.png` (鎻愬彇鍑虹殑 32脳32 姘村嵃浣嶅浘)
- `watermark_compare.png` (鍏紶骞舵帓)

骞舵墦鍗?PSNR/SSIM(src, marked) 涓?mark 鎭㈠鐜?(姝ｇ‘浣嶆暟 / 1024)銆?
## 鍏稿瀷缁撴灉

```
=== invisible watermark (DFT) ===
PSNR(src, marked) = 15.93 dB
SSIM(src, marked) = 0.9028
mark recovery: 962/1024 (93.9%)
```

娉細伪=8 鏃?log 骞呭害鍙樺寲 卤8 = 棰戣氨骞呭害鏀惧ぇ e^8鈮?981 鍊嶏紝绌洪棿鍩熷け鐪熸瀬澶?锛圥SNR鈮?dB锛夛紱伪=2.0 瀹炴祴鍦?lena 涓?PSNR鈮?6dB銆佹仮澶嶇巼鈮?4%銆偽?杩涗竴姝ヨ皟灏?浼氬洜 8U 閲忓寲璇樊娣规病淇″彿鑰岄檷浣庢仮澶嶇巼銆傝嫢瑕佸悓鏃惰揪鍒?瑙嗚鏃犲樊鍒?+ 楂樻仮澶嶇巼"锛?搴旀敼鐢?DCT 鍒嗗潡涓宓屽叆 + 閲忓寲姝ラ暱鎺у埗锛堝吀鍨嬪 JPEG 鍏煎姘村嵃锛夈€?
## 鎷撳睍寤鸿

- DCT 鍩熸按鍗帮細鍒嗗潡 8脳8 DCT锛屾妸姘村嵃宓屽叆涓 DCT 绯绘暟 (`zigzag` 绗?  `[20..40]` 涓郴鏁?锛屽 JPEG 鎶楁€ф洿寮猴紱
- Spread Spectrum锛氭妸姘村嵃涔樹互涓€娈典吉闅忔満 卤1 搴忓垪鍚庡啀宓屽叆锛屾彁楂樺畨鍏ㄦ€?  鍜岀洸鎻愬彇鍙潬鎬э紱
- 鍑犱綍鏀诲嚮椴佹鎬э細鍦ㄥ祵鍏ュ墠瀵瑰浘鍍忓仛 log-polar 鍙樻崲锛屼娇鏃嬭浆/缂╂斁
  鍙樻垚骞崇Щ锛屽啀鍋?DFT 鍩熷祵鍏ワ紱
- 鍔?`--attack` 妯″紡锛氳嚜鍔ㄥ marked 鍥惧仛 JPEG 70/80/90銆乺esize 0.5脳/2脳銆?  楂樻柉鍣０ 蟽=5/10/20銆?脳5 楂樻柉妯＄硦锛岃緭鍑烘瘡绉嶇殑鎭㈠鐜囷紝閲忓寲椴佹鎬с€?