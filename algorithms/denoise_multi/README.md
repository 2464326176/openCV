# denoise_multi 鈥?澶氬抚闄嶅櫔 (閰嶅噯 + 鑱氬悎)

## 鏍稿績鎬濊矾

N 寮犲悓鍦烘櫙杩炴媿甯т箣闂达紝**淇″彿閮ㄥ垎鐩稿叧鑰屽櫔澹伴儴鍒嗙嫭绔?*銆傜悊璁轰笂瀵?N 甯х嫭绔嬪悓鍒嗗竷
楂樻柉鍣０鍋氬潎鍊艰仛鍚堬紝鏂瑰樊涓嬮檷鍒?1/N锛岀瓑鏁堥檷鍣?~`10路log10(N)` dB銆?
浣嗚繛鎷嶅抚涔嬮棿浼氬瓨鍦ㄦ墜鎶栦綅绉伙紝鍥犳蹇呴』鍏?*甯ч棿閰嶅噯**锛屽惁鍒欏潎鍊艰仛鍚堜細寮曞叆楝煎奖銆?
## 娴佹按绾?
```
        鈹屸攢鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹?        鈹?load N frames (NV21/jpg) 鈹?        鈹斺攢鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹攢鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹?                     鈻?       鈹屸攢鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹?       鈹?alignToRef(frame[0], frame[i])鈹? ECC 浠垮皠閰嶅噯
       鈹? MOTION_AFFINE, 30 iters       鈹?       鈹斺攢鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹攢鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹?                    鈻?        鈹屸攢鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹?        鈹?鑱氬悎: mean / median        鈹?        鈹斺攢鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹攢鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹?                     鈻?                鈹屸攢鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹?                鈹?PSNR/SSIM 鈹?                鈹斺攢鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹?```

## ECC 閰嶅噯

Enhanced Correlation Coefficient (ECC, Evangelidis & Psarakis 2008)锛屽涓€闃?浜岄樁
鍏夌収鍙樺寲閮介瞾妫掋€侽penCV 鎺ュ彛 `cv::findTransformECC`锛?
```cpp
cv::Mat warp = cv::Mat::eye(2, 3, CV_32F);    // 浠垮皠鍒濆€?cv::findTransformECC(ref_gray, src_gray, warp,
                      cv::MOTION_AFFINE,
                      cv::TermCriteria(COUNT + EPS, 30, 1e-5));
cv::warpAffine(src, aligned, warp, ref.size());
```

鍏叡搴撳皝瑁呭湪 `algo::alignToRef`锛屽け璐ユ椂杩斿洖鍘熷抚锛堥伩鍏?0 甯э級銆?
## 鑱氬悎绛栫暐

| 鏂规硶 | 鍏紡 | 閫傜敤 |
|------|------|------|
| mean | `out = (1/N) 危 frame_i` | 楂樻柉鍣０锛孨 杈冨ぇ |
| median | `out(p) = median({frame_i(p)})` | 妞掔洂 / outlier 鍣０锛孨 灏?|

瀹炵幇缁嗚妭锛歚medianFuse` 鎶婃瘡閫氶亾鍍忕礌鎺掓垚 vector 鍚庣敤 `std::nth_element` 鍙栦腑鍊硷紝
`O(N)` 閫夋嫨绗?`N/2` 涓厓绱狅紝姣?`std::sort` 鏁翠綋蹇€?
## 鏁版嵁鏉ユ簮

- 榛樿 `data/nv21/nr/*_in.NV21`锛?032脳3000 绱у噾锛変綔涓?鍩哄噯甯?銆?- 鐢变簬 nr 鏁版嵁鍙湁 1 瀵?in/out锛屾棤娉曞仛鐪熷疄澶氬抚锛屾墍浠ユ紨绀虹敤锛?  1. 瀵瑰熀鍑嗗抚鍔?N 浠界嫭绔嬮珮鏂櫔澹?(蟽=15) 妯℃嫙杩炴媿
  2. 姣忓抚鍙犲姞 卤2px 骞崇Щ + 卤0.5掳 鏃嬭浆锛堟ā鎷熸墜鎶栵級
  3. ECC 閰嶅噯 + 鍧囧€?涓€艰仛鍚?  4. 涓庡崟甯?NLM/median 瀵圭収
- PSNR/SSIM 鍧囦互"骞插噣鍩哄噯甯?涓?ground truth銆?
## 杩愯

```powershell
cd out\algorithms
.\algo_denoise_multi.exe                                   # 榛樿 4 甯?蟽=15
.\algo_denoise_multi.exe ..\..\data\nv21\nr\...in.NV21 8 12
.\algo_denoise_multi.exe ..\..\data\images\lena.jpg 6 20
```

## 鍏稿瀷瓒嬪娍

鐞嗚涓?`10路log10(N)` dB 鎻愬崌锛圢=4 鈫?+6 dB锛夛紝瀹為檯閰嶅噯娈嬪樊浼氳澧炵泭鎵撴姌锛?
```
=== multi-frame denoise (N=4, sigma=15.0) ===
method          PSNR    SSIM
single(noisy)   24.62   0.6934
single NLM      30.72   0.8793
single median   28.41   0.8023
multi mean(ECC) 33.18   0.9156
multi median(ECC) 31.95 0.8812
```

## 鎷撳睍寤鸿

- 鎶?mean 鏇挎崲涓?鏂瑰樊鍔犳潈"锛氭瘡鍍忕礌鎸夋湰鍦版柟宸潈閲嶈仛鍚堬紝閬垮厤楝煎奖澶勫潎鍊奸敊璇€?- 鎶?ECC 鍗囩骇鍒?`MOTION_HOMOGRAPHY`锛堥€忚锛夛紝澶勭悊澶ц宸満鏅€?- 鐢?`data/nv21/ev/` 3 甯у仛鐪熷疄澶氬抚 HDR-NR 鑱斿悎锛堟洕鍏夋椂闂翠笉鍚屼篃鑳藉榻愶級銆?- 寮曞叆 VBM3D / 甯ч棿 BM4D锛屼綔涓哄姣斿熀绾裤€?