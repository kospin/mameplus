CONFIG += build_static
CONFIG += build_sdl

#INCLUDE
INCLUDEPATH += quazip lzma include include/zlib include/SDL
win32 {
INCLUDEPATH += include/SDL/Win32
}
unix {
INCLUDEPATH += include/SDL/Linux
}

#LIBS
build_static {
  QTPLUGIN += qico qjpeg
  DEFINES += USE_STATIC
}

build_sdl {
  win32 {
  LIBS += -Llib/Win32 -lSDLmain
  }
  unix {
  LIBS += -Llib/Linux
  }
  LIBS += -lSDL
  DEFINES += USE_SDL
}

win32 {
RC_FILE = mamepgui.rc
}

macx {
DEFINES += _LZMA_UINT32_IS_ULONG
}

FORMS += mamepguimain.ui playoptions.ui options.ui csvcfg.ui directories.ui about.ui ips.ui m1.ui
TRANSLATIONS = lang/mamepgui_zh_CN.ts lang/mamepgui_zh_TW.ts lang/mamepgui_ja_JP.ts lang/mamepgui_hu_HU.ts lang/mamepgui_ko_KR.ts lang/mamepgui_pt_BR.ts

HEADERS += mamepguimain.h dialogs.h audit.h gamelist.h mameopt.h utils.h ips.h m1.h quazip/ioapi.h quazip/zip.h quazip/unzip.h quazip/quazip.h quazip/quazipfile.h quazip/quazipfileinfo.h
SOURCES += mamepguimain.cpp dialogs.cpp audit.cpp gamelist.cpp mameopt.cpp utils.cpp ips.cpp m1.cpp quazip/ioapi.c quazip/zip.c quazip/unzip.c quazip/quazip.cpp quazip/quazipfile.cpp

HEADERS += lzma/7zBuf.h lzma/7zCrc.h lzma/7zFile.h lzma/7zVersion.h lzma/Bcj2.h lzma/Bra.h lzma/CpuArch.h lzma/LzmaDec.h lzma/Types.h
HEADERS += lzma/7zAlloc.h lzma/7zDecode.h lzma/7zExtract.h lzma/7zHeader.h lzma/7zIn.h lzma/7zItem.h
SOURCES += lzma/7zBuf.c lzma/7zBuf2.c lzma/7zCrc.c lzma/7zFile.c lzma/7zStream.c lzma/Bcj2.c lzma/Bra.c lzma/Bra86.c lzma/LzmaDec.c
SOURCES += lzma/7zAlloc.c lzma/7zDecode.c lzma/7zExtract.c lzma/7zHeader.c lzma/7zIn.c lzma/7zItem.c

RESOURCES = mamepguimain.qrc

TARGET = mamepgui
QT += xml
