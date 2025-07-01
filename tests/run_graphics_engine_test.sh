#!/bin/bash
set -e

# 프로젝트 루트 디렉토리로 이동
cd $(dirname $0)/..

# 빌드
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# 테스트 실행
./build/GraphicsEngineTest 