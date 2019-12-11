# pdfium_with_cmake   
convert pdf  to image    

compile pdfium with cmake.    
using agg, without V8/xfa     
using system libjpeg, zlib   



## download pdfium source code follow [PDFium](https://pdfium.googlesource.com/pdfium/)
```shell
mkdir repo
cd repo
gclient config --unmanaged https://pdfium.googlesource.com/pdfium.git
gclient sync
cd pdfium
```

## build
```
mkdir out
cd out
cmake ..
make -j4
```
