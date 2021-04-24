# openCV mingw64 cmake
计算机视觉

在ubuntu上学习一段时间感觉挺不方便的，还是windows比较方便操作点，整理了下windows的步骤，给大家分享下
1.mingw64 下载
地址：https://sourceforge.net/projects/mingw-w64/files/
MinGW-W64 GCC-8.1.0
    x86_64-posix-sjlj
    x86_64-posix-seh
    x86_64-win32-sjlj
    x86_64-win32-seh
    i686-posix-sjlj
    i686-posix-dwarf
    i686-win32-sjlj
    i686-win32-dwarf
可以看到有很多版本，这里要选择 x86_64-posix-sjlj
POSIX表示可移植操作系统接口(Portable Operating System Interface of UNIX，缩写为 POSIX )
seh 是新发明的，而 sjlj 则是古老的。seh 性能比较好，但不支持32位。sjlj 稳定性好，支持32位，兼容前面的平台。
2.camake 自己安装下

3.opencv源码：https://www.bzblog.online/wordpress/index.php/2020/03/09/opencvdownload/
我选择是4.5.0的，详细的安装步骤：https://blog.huihut.com/2018/07/31/CompiledOpenCVWithMinGW64/