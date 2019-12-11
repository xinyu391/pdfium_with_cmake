# pdfium_with_cmake   
convert pdf  to image    

compile pdfium with cmake on Ubuntu.   
using agg, without V8/xfa     
using libjpeg-turbo (need to install  nasm)  
using system zlib   



## download pdfium source code follow [PDFium](https://pdfium.googlesource.com/pdfium/)
```shell
mkdir repo
cd repo
gclient config --unmanaged https://pdfium.googlesource.com/pdfium.git
gclient sync
cd pdfium
```

## 修改代码
third_party/freetype/src/src/psnames/psmodule.c
remove first **#include "pstables.h"**
```C
// #include "pstables.h"
#define  DEFINE_PS_TABLES
#define  DEFINE_PS_TABLES_DATA
#include "pstables.h"
```

## build
```
mkdir out
cd out
cmake ..
make -j4
```

## known issue  
> /usr/bin/ld: libpdfium.so: undefined reference to 'icudt64_dat'
need define varient with name **icudt64_dat** in your cpp.
```C++
const char* icudt64_dat =NULL;
```
who can tell me the reason?

