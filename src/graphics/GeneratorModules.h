#pragma once

#include "PipelineModule.h"
#include "PipelineGraph.h"
#include <functional>

/**
 * @brief Simplex Noise 생성 모듈
 * 
 * LYGIA의 snoise 함수를 사용하여 Simplex 노이즈를 생성합니다.
 */
class NoiseGenerator : public PipelineModule {
public:
    NoiseGenerator();

    std::string generateGLSL(
        const std::unordered_map<std::string, std::string>& inputs,
        const std::unordered_map<std::string, std::string>& outputs
    ) const override;

    std::vector<std::string> getRequiredIncludes() const override;
    std::unordered_map<std::string, std::string> getParameters() const override;
};

/**
 * @brief Voronoi 패턴 생성 모듈
 * 
 * LYGIA의 voronoi 함수를 사용하여 Voronoi 다이어그램을 생성합니다.
 */
class VoronoiGenerator : public PipelineModule {
public:
    VoronoiGenerator();

    std::string generateGLSL(
        const std::unordered_map<std::string, std::string>& inputs,
        const std::unordered_map<std::string, std::string>& outputs
    ) const override;

    std::vector<std::string> getRequiredIncludes() const override;
    std::unordered_map<std::string, std::string> getParameters() const override;
};

/**
 * @brief 그라디언트 생성 모듈
 * 
 * 선형/방사형 그라디언트를 생성합니다.
 */
class GradientGenerator : public PipelineModule {
public:
    GradientGenerator();

    std::string generateGLSL(
        const std::unordered_map<std::string, std::string>& inputs,
        const std::unordered_map<std::string, std::string>& outputs
    ) const override;

    std::vector<std::string> getRequiredIncludes() const override;
    std::unordered_map<std::string, std::string> getParameters() const override;
};

/**
 * @brief 랜덤 패턴 생성 모듈
 * 
 * LYGIA의 random 함수를 사용하여 랜덤 패턴을 생성합니다.
 */
class RandomGenerator : public PipelineModule {
public:
    RandomGenerator();

    std::string generateGLSL(
        const std::unordered_map<std::string, std::string>& inputs,
        const std::unordered_map<std::string, std::string>& outputs
    ) const override;

    std::vector<std::string> getRequiredIncludes() const override;
    std::unordered_map<std::string, std::string> getParameters() const override;
};

/**
 * @brief FBM (Fractal Brownian Motion) 생성 모듈
 * 
 * LYGIA의 fbm 함수를 사용하여 프랙탈 패턴을 생성합니다.
 */
class FBMGenerator : public PipelineModule {
public:
    FBMGenerator();

    std::string generateGLSL(
        const std::unordered_map<std::string, std::string>& inputs,
        const std::unordered_map<std::string, std::string>& outputs
    ) const override;

    std::vector<std::string> getRequiredIncludes() const override;
    std::unordered_map<std::string, std::string> getParameters() const override;
};

/**
 * @brief Generator 모듈들의 팩토리 및 관리 클래스
 * 
 * ShaderManager와의 통합을 위한 인터페이스를 제공합니다.
 */
class GeneratorModules {
public:
    GeneratorModules();
    ~GeneratorModules() = default;

    /**
     * @brief 특정 타입의 Generator가 존재하는지 확인
     * @param type Generator 타입 이름
     * @return 존재 여부
     */
    bool hasGenerator(const std::string& type) const;

    /**
     * @brief Generator 노드를 위한 GLSL 함수 생성
     * @param node 노드 정보
     * @return 생성된 GLSL 함수 코드
     */
    std::string generateFunction(const PipelineGraph::Node& node) const;

    /**
     * @brief 사용 가능한 모든 Generator 타입 반환
     * @return Generator 타입 이름들
     */
    std::vector<std::string> getAvailableTypes() const;

private:
    std::unordered_map<std::string, std::function<std::unique_ptr<PipelineModule>()>> generators_;
};
