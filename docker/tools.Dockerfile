# OpenPak nn-nimbus build image.
#
# The upstream devkitARM image ships the compiler, libctru, citro2d, tex3ds and
# picasso, but not every host tool the Nimbus build calls: makerom and bannertool
# (the CIA), armips and flips (the sysmodule IPS patches), 3gxtool (the plugin)
# and CTRPluginFramework (the library the plugin links). This image adds them,
# each from its upstream source, so a build needs nothing installed by hand.

FROM devkitpro/devkitarm:latest

RUN apt-get update \
 && apt-get install -y --no-install-recommends ca-certificates xz-utils zip \
 && rm -rf /var/lib/apt/lists/*

# makerom — packs the CIA (Project_CTR)
RUN git clone --depth=1 https://github.com/3DSGuy/Project_CTR /tmp/Project_CTR \
 && make -C /tmp/Project_CTR/makerom deps \
 && make -C /tmp/Project_CTR/makerom -j"$(nproc)" \
 && cp /tmp/Project_CTR/makerom/bin/makerom /opt/devkitpro/tools/bin/ \
 && rm -rf /tmp/Project_CTR

# bannertool — builds the CIA banner (maintained fork of Steveice10's)
RUN git clone --depth=1 https://github.com/carstene1ns/3ds-bannertool /tmp/bannertool \
 && cmake -S /tmp/bannertool -B /tmp/bannertool/build -DCMAKE_BUILD_TYPE=Release \
 && cmake --build /tmp/bannertool/build -j"$(nproc)" \
 && cp /tmp/bannertool/build/bannertool /opt/devkitpro/tools/bin/ \
 && rm -rf /tmp/bannertool

# armips — assembles the sysmodule patches
RUN git clone --depth=1 --recursive https://github.com/Kingcom/armips /tmp/armips \
 && cmake -S /tmp/armips -B /tmp/armips/build -DCMAKE_BUILD_TYPE=Release \
 && cmake --build /tmp/armips/build -j"$(nproc)" \
 && cp /tmp/armips/build/armips /opt/devkitpro/tools/bin/ \
 && rm -rf /tmp/armips

# flips — makes the IPS patches from original and patched code
RUN git clone --depth=1 https://github.com/Alcaro/Flips /tmp/Flips \
 && TARGET=cli make -C /tmp/Flips -j"$(nproc)" \
 && cp /tmp/Flips/flips /opt/devkitpro/tools/bin/ \
 && rm -rf /tmp/Flips

# 3gxtool — turns the plugin ELF into a .3gx
RUN git clone --depth=1 --recursive https://gitlab.com/thepixellizeross/3gxtool /tmp/3gxtool \
 && cmake -S /tmp/3gxtool -B /tmp/3gxtool/build -DCMAKE_BUILD_TYPE=Release \
 && cmake --build /tmp/3gxtool/build -j"$(nproc)" \
 && cp /tmp/3gxtool/build/3gxtool /opt/devkitpro/tools/bin/ \
 && rm -rf /tmp/3gxtool

# CTRPluginFramework — the library the Nimbus plugin links against.
# Full clone (not shallow): the build stamps itself with `git describe`.
# Serial on purpose: its Makefile races the debug and release libs on
# libcwav.a under -j.
RUN git clone --recursive https://gitlab.com/thepixellizeross/ctrpluginframework /tmp/ctrpf \
 && sed -i "s/-Wall -Werror/-Wall/" /tmp/ctrpf/Library/Makefile \
 && make -C /tmp/ctrpf/Library install \
 && rm -rf /tmp/ctrpf
