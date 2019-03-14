FROM alpine:3.8 AS build

RUN apk --no-cache add \
        git \
        pkgconfig \
        gcc autoconf \
        automake \
        make \
        linux-headers \
        coreutils \
        musl-dev \
        popt-dev

RUN git clone git://git.sv.gnu.org/gnulib.git

COPY . /rether

RUN cd rether && \
        /gnulib/gnulib-tool --update && \
        autoreconf -ivf && \
        ./configure --disable-dependency-tracking --prefix=/usr CFLAGS="-O2" && \
        make && \
        make install

FROM alpine:3.8

RUN apk --no-cache add \
        curl \
        tcpdump \
        netcat-openbsd \
        openssh-client \
        net-tools \
        bind-tools \
        wpa_supplicant \
        busybox-extras \
        iproute2 \
        popt

COPY --from=build /usr/bin/rether /usr/bin/

VOLUME ["/root"]
