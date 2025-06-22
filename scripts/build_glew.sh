#!/bin/bash

# GLEW를 공식 Makefile로 빌드하고
# 로컬 디렉터리에 설치해 CMake에서 사용할 수 있게 합니다.

set -e

# 현재 경로 기준 GLEW 소스 및 설치 경로 설정
GLEW_SOURCE_DIR="$(pwd)/external/glew"
GLEW_INSTALL_DIR="$(pwd)/build/glew-install"

rm -rf "$GLEW_INSTALL_DIR"  "$GLEW_SOURCE_DIR"

# 클론이 안 되어 있으면 클론 (depth=1으로 최신만 받음)
if [ ! -d "$GLEW_SOURCE_DIR" ]; then
  git clone --depth=1 https://github.com/nigels-com/glew.git "$GLEW_SOURCE_DIR"
fi

# 설치 폴더 생성
mkdir -p "$GLEW_INSTALL_DIR"
mkdir -p "$GLEW_INSTALL_DIR/include/GL"
mkdir -p "$GLEW_INSTALL_DIR/lib"

# GLEW 소스 디렉터리로 이동
cd "$GLEW_SOURCE_DIR"

# auto 디렉터리에서 필요한 소스 생성 (한 번만 실행해도 됨)
if [ -d auto ]; then
  echo "Running make in auto/ to generate GLEW sources..."
  make -j32 -C auto || echo "Auto generation had some warnings, continuing..."
fi

# 정적 라이브러리 빌드 및 로컬 설치
echo "Building GLEW static library..."
make SYSTEM=linux-egl GLEW_DEST="$GLEW_INSTALL_DIR" glew.lib.static || make SYSTEM=linux GLEW_DEST="$GLEW_INSTALL_DIR" glew.lib.static
echo "Installing GLEW..."
make SYSTEM=linux-egl GLEW_DEST="$GLEW_INSTALL_DIR" install || make SYSTEM=linux GLEW_DEST="$GLEW_INSTALL_DIR" install

# 클린업
make clean

echo "GLEW build and install completed successfully."
