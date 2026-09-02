# denoise_single 鈥?鍗曞抚闄嶅櫔绠楁硶瀵规瘮

## 绠楁硶

鏈?demo 鍦ㄥ悓涓€寮犲惈鍣浘涓婂悓鏃惰窇 5 绉嶅吀鍨嬪崟甯ч檷鍣畻娉曪紝骞剁浉瀵瑰共鍑€鍘熷浘缁欏嚭
PSNR / SSIM锛?
| 鏂规硶 | 鏍稿績鎬濇兂 | 杈圭紭淇濇寔 | 澶嶆潅搴?| 鍏稿瀷鐢ㄩ€?|
|------|---------|---------|--------|----------|
| GaussianBlur (5脳5) | 楂樻柉鍔犳潈鍧囧€?| 鍚?| O(N路k虏) | 鍩虹嚎锛岄澶勭悊 |
| medianBlur (5脳5) | 涓€?| 涓?| O(N路k虏) | 妞掔洂鍣０ |
| bilateralFilter (d=9, 蟽_c=75, 蟽_s=75) | 鍩?绌哄弻鏍搁珮鏂姞鏉?| 楂?| O(N路d虏) | 缇庨鍩虹嚎 |
| fastNlMeansDenoisingColored (h=10) | 鍧楀尮閰嶉潪灞€閮ㄥ潎鍊?| 楂?| O(N路t虏路s虏) | 鏆楀厜闄嶅櫔 |
| GuidedFilter (r=8, 蔚=0.01) | 寮曞婊ゆ尝 (He 2010) | 楂?| O(N) | 淇濊竟骞虫粦 |

## 鏁板鍘熺悊

### 鍙岃竟婊ゆ尝

杈撳嚭鍍忕礌锛?```
J(p) = 1/W_p 路 危_{q鈭埼p} G_蟽_s(鈥杙鈭抭鈥? 路 G_蟽_r(鈥朓(p)鈭扞(q)鈥? 路 I(q)
W_p = 危_q G_蟽_s(鈥杙鈭抭鈥? 路 G_蟽_r(鈥朓(p)鈭扞(q)鈥?
```
绌哄煙鏍?`G_蟽_s` 涓庡€煎煙鏍?`G_蟽_r` 閮芥槸楂樻柉锛涘€煎煙鏍歌澶ф搴﹁竟缂樻潈閲嶈“鍑忥紝浠庤€屼繚杈广€?
### 闈炲眬閮ㄥ潎鍊?(NLM)

```
J(p) = 危_q w(p,q) I(q)
w(p,q) = exp(鈭掆€杤(N_p) 鈭?v(N_q)鈥柭?/ h虏)
```
`v(N_p)` 鏄?p 鍛ㄥ洿妯℃澘閭诲煙鐨勫悜閲忥紱鍧楄秺鐩镐技鏉冮噸瓒婂ぇ锛沗h` 鎺у埗琛板噺寮哄害銆侽penCV 鐨?`fastNlMeansDenoisingColored` 鎶?BGR 杞?YUV锛屽彧瀵?Y 鍋?NLM锛岄伩鍏嶈壊搴︽墿鏁ｃ€?
### 寮曞婊ゆ尝 (Guided Filter)

璁惧紩瀵煎浘 I銆佽緭鍏?p銆佽緭鍑?q锛?```
q_i = a_k 路 I_i + b_k   (鍦ㄧ獥鍙?蠅_k 鍐?
a_k = (危 I路p 鈭?|蠅|鈦宦?危I 危p) / (危I虏 鈭?|蠅|鈦宦?(危I)虏 + 蔚)
b_k = p虅_k 鈭?a_k 路 莫_k
```
鏈€缁?a銆乥 缁?boxFilter 骞虫粦鍚庝笌 I 绾挎€х粍鍚堛€偽?瓒婂皬杈圭紭淇濇寔瓒婂己銆?
## 杩愯

```powershell
cd out\algorithms
.\algo_denoise_single.exe                                   # 榛樿 ../../data/images/lena.jpg, 蟽=15
.\algo_denoise_single.exe ..\..\data\images\VCG5.jpg 20     # 鑷畾涔夊浘涓庡櫔澹板己搴?```

杈撳嚭 `out/algorithms/denoise_single.png`锛? 寮犲浘妯悜鎷兼帴锛? stdout 鎸囨爣琛ㄣ€?
## 鍏稿瀷缁撴灉锛坙ena.jpg, 蟽=15, 浣滀负鍙傝€冿級

```
=== single frame denoise comparison (sigma=15.0) ===
method         PSNR    SSIM
noisy(baseline) 24.62 0.6934
gaussian        27.85 0.7812
median          28.41 0.8023
bilateral       29.98 0.8561
nlm             30.72 0.8793
guided          28.90 0.8155
```
鍏蜂綋鏁板€间細闅?OpenCV 鐗堟湰 / 缂栬瘧閫夐」 / RNG 绉嶅瓙鐣ユ湁涓嶅悓锛涜秼鍔挎槸 NLM > bilateral >
median 鈮?guided > gaussian > noisy銆?
## 鎷撳睍寤鸿

- 鐢?`cv::BM3D` (opencv_contrib) 鏇夸唬 NLM锛孭SNR 浼氬啀鎻愬崌 1鈥? dB锛?- 鎶?NLM 鐨?`h` 鍙傛暟闅?蟽 鑷€傚簲锛堢粡楠屽紡 `h 鈮?0.55路蟽`锛夛紝鍙壒閲忚瘎浼帮紱
- 鍔犲垎缁勶細楂樻柉/涓€奸€傚悎"蹇€熼澶勭悊"锛孨LM/guided/bilateral 閫傚悎"鏈€缁堣緭鍑?銆?