#pragma once

#include "PipelineModule.h"
#include "PipelineGraph.h"
#include <functional>

/**
 * @brief 두 입력을 블렌딩하는 오퍼레이터
 * 
 * LYGIA의 blend 함수들을 사용하여 다양한 블렌딩 모드를 지원합니다.
 */
class BlendOperator : public PipelineModule {
public:
    BlendOperator();

    std::string generateGLSL(
        const std::unordered_map<std::string, std::string>& inputs,
        const std::unordered_map<std::string, std::string>& outputs
    ) const override;

    std::vector<std::string> getRequiredIncludes() const override;
    std::unordered_map<std::string, std::string> getParameters() const override;
};

/**
 * @brief 입력을 변환하는 오퍼레이터
 * 
 * 회전, 스케일, 이동 등의 변환을 적용합니다.
 */
class TransformOperator : public PipelineModule {
public:
    TransformOperator();

    std::string generateGLSL(
        const std::unordered_map<std::string, std::string>& inputs,
        const std::unordered_map<std::string, std::string>& outputs
    ) const override;

    std::vector<std::string> getRequiredIncludes() const override;
    std::unordered_map<std::string, std::string> getParameters() const override;
};

/**
 * @brief 수학적 연산 오퍼레이터
 * 
 * 덧셈, 곱셈, 거듭제곱 등의 수학 연산을 수행합니다.
 */
class MathOperator : public PipelineModule {
public:
    MathOperator();

    std::string generateGLSL(
        const std::unordered_map<std::string, std::string>& inputs,
        const std::unordered_map<std::string, std::string>& outputs
    ) const override;

    std::vector<std::string> getRequiredIncludes() const override;
    std::unordered_map<std::string, std::string> getParameters() const override;
};

/**
 * @brief 색상 변환 오퍼레이터
 * 
 * 단일 값을 RGB 색상으로 변환하거나 색상을 조정합니다.
 */
class ColorOperator : public PipelineModule {
public:
    ColorOperator();

    std::string generateGLSL(
        const std::unordered_map<std::string, std::string>& inputs,
        const std::unordered_map<std::string, std::string>& outputs
    ) const override;

    std::vector<std::string> getRequiredIncludes() const override;
    std::unordered_map<std::string, std::string> getParameters() const override;
};

/**
 * @brief 필터 오퍼레이터
 * 
 * 블러, 엣지 검출 등의 필터링 효과를 적용합니다.
 */
class FilterOperator : public PipelineModule {
public:
    FilterOperator();

    std::string generateGLSL(
        const std::unordered_map<std::string, std::string>& inputs,
        const std::unordered_map<std::string, std::string>& outputs
    ) const override;

    std::vector<std::string> getRequiredIncludes() const override;
    std::unordered_map<std::string, std::string> getParameters() const override;
};

/**
 * @brief Operator 모듈들의 팩토리 및 관리 클래스
 * 
 * ShaderManager와의 통합을 위한 인터페이스를 제공합니다.
 */
class OperatorModules {
public:
    OperatorModules();
    ~OperatorModules() = default;

    /**
     * @brief 특정 타입의 Operator가 존재하는지 확인
     * @param type Operator 타입 이름
     * @return 존재 여부
     */
    bool hasOperator(const std::string& type) const;

    /**
     * @brief Operator 노드를 위한 GLSL 함수 생성
     * @param node 노드 정보
     * @return 생성된 GLSL 함수 코드
     */
    std::string generateFunction(const PipelineGraph::Node& node) const;

    /**
     * @brief 사용 가능한 모든 Operator 타입 반환
     * @return Operator 타입 이름들
     */
    std::vector<std::string> getAvailableTypes() const;

private:
    std::unordered_map<std::string, std::function<std::unique_ptr<PipelineModule>()>> operators_;
};
