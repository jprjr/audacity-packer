FROM ubuntu:18.04 as windows-builder

RUN apt-get update && apt-get install -y gcc-multilib upx \
    mingw-w64 curl make patch build-essential git cmake nsis

RUN mkdir -p /src && \
    cd /src && \
    git clone https://github.com/jprjr/iup-build.git iup
WORKDIR /src/iup

RUN \
    make TARGET=i686-w64-mingw32 clean && \
    make TARGET=x86_64-w64-mingw32 clean && \
    make TARGET=i686-w64-mingw32 && \
    make TARGET=i686-w64-mingw32 dist && \
    make TARGET=x86_64-w64-mingw32 && \
    make TARGET=x86_64-w64-mingw32 dist && \
    tar xzf output/lua-i686-w64-mingw32.tar.gz -C /usr/i686-w64-mingw32 && \
    tar xzf output/ftgl-i686-w64-mingw32.tar.gz -C /usr/i686-w64-mingw32 && \
    tar xzf output/freetype-i686-w64-mingw32.tar.gz -C /usr/i686-w64-mingw32 && \
    tar xzf output/zlib-i686-w64-mingw32.tar.gz -C /usr/i686-w64-mingw32 && \
    tar xzf output/im-i686-w64-mingw32.tar.gz -C /usr/i686-w64-mingw32 && \
    tar xzf output/cd-i686-w64-mingw32.tar.gz -C /usr/i686-w64-mingw32 && \
    tar xzf output/iup-i686-w64-mingw32.tar.gz -C /usr/i686-w64-mingw32 && \
    tar xzf output/lua-x86_64-w64-mingw32.tar.gz -C /usr/x86_64-w64-mingw32 && \
    tar xzf output/ftgl-x86_64-w64-mingw32.tar.gz -C /usr/x86_64-w64-mingw32 && \
    tar xzf output/freetype-x86_64-w64-mingw32.tar.gz -C /usr/x86_64-w64-mingw32 && \
    tar xzf output/zlib-x86_64-w64-mingw32.tar.gz -C /usr/x86_64-w64-mingw32 && \
    tar xzf output/im-x86_64-w64-mingw32.tar.gz -C /usr/x86_64-w64-mingw32 && \
    tar xzf output/cd-x86_64-w64-mingw32.tar.gz -C /usr/x86_64-w64-mingw32 && \
    tar xzf output/iup-x86_64-w64-mingw32.tar.gz -C /usr/x86_64-w64-mingw32 && \
    make TARGET=i686-w64-mingw32 clean && \
    make TARGET=x86_64-w64-mingw32 clean && \
    cd / && \
    rm -rf /src/iup

RUN mkdir -p /src && \
    cd /src && \
    curl -R -L -o wavpack-5.1.0.tar.bz2 "http://www.wavpack.com/wavpack-5.1.0.tar.bz2" && \
    tar xf wavpack-5.1.0.tar.bz2 && \
    mv wavpack-5.1.0 wavpack

RUN cd /src/wavpack && \
    ./configure \
      --prefix=/usr/i686-w64-mingw32 \
      --host=i686-w64-mingw32 \
      --disable-shared \
      --enable-static && \
    make && \
    make install && \
    make clean && \
    ./configure \
      --prefix=/usr/x86_64-w64-mingw32 \
      --host=x86_64-w64-mingw32 \
      --disable-shared \
      --enable-static && \
    make && \
    make install

COPY . /src/aupack

WORKDIR /src/aupack

RUN mkdir -p /dist

RUN make clean && \
    make -f Makefile.win32 CC=i686-w64-mingw32-gcc RC=i686-w64-mingw32-windres audacity-packer.exe && \
    upx audacity-packer.exe && \
    mv audacity-packer.exe audacity-packer-32.exe && \
    cp audacity-packer-32.exe /dist/ && \
    make -f Makefile.win32 clean && \
    make -f Makefile.win32 CC=x86_64-w64-mingw32-gcc RC=x86_64-w64-mingw32-windres audacity-packer.exe && \
    upx audacity-packer.exe && \
    mv audacity-packer.exe audacity-packer-64.exe && \
    cp audacity-packer-64.exe /dist/ && \
    make -f Makefile.win32  audacity-packer-installer.exe && \
    cp audacity-packer-installer.exe /dist/

FROM alpine:3.10
RUN apk add rsync tar zip gzip && mkdir -p /dist
COPY --from=windows-builder /dist/* /dist/

COPY entrypoint.sh /
ENTRYPOINT ["/entrypoint.sh"]
