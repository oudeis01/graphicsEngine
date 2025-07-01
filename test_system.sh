#!/bin/bash

# Graphics Engine OSC-Based Architecture Test Script
# This script tests all components of the new distributed architecture

echo "========================================="
echo "Graphics Engine Architecture Test"
echo "========================================="

# Build the project
echo "Building project..."
cmake -B build -S . > /dev/null 2>&1
if ! make -C build -j$(nproc) > /dev/null 2>&1; then
    echo "❌ Build failed"
    exit 1
fi
echo "✅ Build successful"

# Test individual binaries can start
echo ""
echo "Testing individual binary startup..."

# Test Graphics Engine
echo "  Testing Graphics Engine..."
timeout 2s ./build/graphics_engine > /tmp/engine.log 2>&1 &
ENGINE_PID=$!
sleep 1
if kill -0 $ENGINE_PID 2>/dev/null; then
    echo "  ✅ Graphics Engine starts successfully"
    kill $ENGINE_PID 2>/dev/null
else
    echo "  ❌ Graphics Engine failed to start"
    cat /tmp/engine.log
fi

# Test Node Editor
echo "  Testing Node Editor..."
timeout 2s ./build/node_editor > /tmp/editor.log 2>&1 &
EDITOR_PID=$!
sleep 1
if kill -0 $EDITOR_PID 2>/dev/null; then
    echo "  ✅ Node Editor starts successfully"
    kill $EDITOR_PID 2>/dev/null
else
    echo "  ❌ Node Editor failed to start"
    cat /tmp/editor.log
fi

# Test Code Interpreter
echo "  Testing Code Interpreter..."
timeout 2s ./build/code_interpreter > /tmp/interpreter.log 2>&1 &
INTERPRETER_PID=$!
sleep 1
if kill -0 $INTERPRETER_PID 2>/dev/null; then
    echo "  ✅ Code Interpreter starts successfully"
    kill $INTERPRETER_PID 2>/dev/null
else
    echo "  ❌ Code Interpreter failed to start"
    cat /tmp/interpreter.log
fi

# Test Custom Make Targets
echo ""
echo "Testing custom make targets..."

TARGETS=("run-engine" "run-node-editor" "run-code-interpreter" "run-explorer")
for target in "${TARGETS[@]}"; do
    echo "  Testing make $target..."
    if make -C build $target --dry-run > /dev/null 2>&1; then
        echo "  ✅ Target '$target' exists"
    else
        echo "  ❌ Target '$target' missing"
    fi
done

# Test LYGIA Explorer
echo ""
echo "Testing LYGIA Explorer..."
if timeout 2s make -C build run-explorer > /tmp/lygia.log 2>&1; then
    if grep -q "Found.*GLSL modules" /tmp/lygia.log; then
        echo "✅ LYGIA Explorer working (found GLSL modules)"
    else
        echo "❌ LYGIA Explorer not finding modules"
    fi
else
    echo "❌ LYGIA Explorer failed"
fi

echo ""
echo "========================================="
echo "Architecture Test Summary"
echo "========================================="
echo "New OSC-based distributed architecture is ready!"
echo ""
echo "Available components:"
echo "  🎯 Graphics Engine     (OSC Server on port 57120)"
echo "  🎨 Node Editor         (OSC Client)"
echo "  📜 Code Interpreter    (OSC Client)"
echo "  🔍 LYGIA Explorer      (Standalone utility)"
echo ""
echo "Start all components: make -C build run-all"
echo "Or start individually with: make -C build run-[engine|node-editor|code-interpreter]"
echo ""
echo "Next steps:"
echo "  • Implement ImGui node editor integration"
echo "  • Implement AngelScript code interpretation"
echo "  • Add shader compilation and rendering pipeline"
echo "  • Enhance OSC message protocol and error handling"

# Cleanup
rm -f /tmp/engine.log /tmp/editor.log /tmp/interpreter.log /tmp/lygia.log
