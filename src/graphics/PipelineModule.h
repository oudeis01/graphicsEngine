#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <set>
#include <functional>

/**
 * @brief 그래픽 파이프라인의 기본 모듈 타입
 */
enum class ModuleType {
    GENERATOR,    ///< 데이터 생성 모듈 (noise, gradient, etc.)
    OPERATOR,     ///< 데이터 변환/조합 모듈 (blend, transform, etc.)
    OUTPUT        ///< 최종 출력 모듈
};

/**
 * @brief 모듈 간 연결을 위한 데이터 타입
 */
enum class DataType {
    FLOAT,        ///< 단일 실수값
    VEC2,         ///< 2D 벡터
    VEC3,         ///< 3D 벡터 (RGB 색상)
    VEC4,         ///< 4D 벡터 (RGBA 색상)
    TEXTURE       ///< 텍스처 데이터
};

/**
 * @brief 모듈의 입출력 포트 정의
 */
struct ModulePort {
    std::string name;           ///< 포트 이름
    DataType type;              ///< 데이터 타입
    bool required = true;       ///< 필수 연결 여부
    std::string defaultValue;   ///< 기본값 (연결되지 않은 경우)
};

/**
 * @brief 파이프라인 모듈의 기본 인터페이스
 * 
 * 모든 Generator와 Operator는 이 인터페이스를 구현합니다.
 * 각 모듈은 LYGIA 함수들을 조합하여 GLSL 코드를 생성합니다.
 */
class PipelineModule {
public:
    PipelineModule(const std::string& name, ModuleType type);
    virtual ~PipelineModule() = default;

    /**
     * @brief 모듈의 GLSL 함수 코드 생성
     * @param inputs 입력 포트들의 변수명 매핑
     * @param outputs 출력 포트들의 변수명 매핑
     * @return 생성된 GLSL 함수 코드
     */
    virtual std::string generateGLSL(
        const std::unordered_map<std::string, std::string>& inputs,
        const std::unordered_map<std::string, std::string>& outputs
    ) const = 0;

    /**
     * @brief 모듈이 필요로 하는 LYGIA includes 반환
     * @return LYGIA 모듈 경로들의 벡터
     */
    virtual std::vector<std::string> getRequiredIncludes() const = 0;

    /**
     * @brief 모듈이 필요로 하는 LYGIA 모듈들 반환 (ShaderManager 통합용)
     * @return LYGIA 모듈 경로들의 집합
     */
    virtual std::set<std::string> getRequiredLygiaModules() const {
        auto includes = getRequiredIncludes();
        return std::set<std::string>(includes.begin(), includes.end());
    }

    /**
     * @brief 모듈의 파라미터들 반환
     * @return 파라미터 이름과 기본값의 매핑
     */
    virtual std::unordered_map<std::string, std::string> getParameters() const = 0;

    // Getters
    const std::string& getName() const { return name_; }
    ModuleType getType() const { return type_; }
    const std::vector<ModulePort>& getInputPorts() const { return inputPorts_; }
    const std::vector<ModulePort>& getOutputPorts() const { return outputPorts_; }

protected:
    std::string name_;                      ///< 모듈 이름
    ModuleType type_;                       ///< 모듈 타입
    std::vector<ModulePort> inputPorts_;    ///< 입력 포트들
    std::vector<ModulePort> outputPorts_;   ///< 출력 포트들
};

/**
 * @brief 모듈 팩토리 클래스
 * 
 * 등록된 모듈들을 관리하고 인스턴스를 생성합니다.
 */
class ModuleFactory {
public:
    /**
     * @brief 모듈 타입 등록
     * @param name 모듈 이름
     * @param creator 모듈 생성 함수
     */
    static void registerModule(const std::string& name, 
                              std::function<std::unique_ptr<PipelineModule>()> creator);

    /**
     * @brief 모듈 인스턴스 생성
     * @param name 모듈 이름
     * @return 생성된 모듈 인스턴스 또는 nullptr
     */
    static std::unique_ptr<PipelineModule> createModule(const std::string& name);

    /**
     * @brief 등록된 모든 모듈 이름 반환
     * @return 모듈 이름들의 벡터
     */
    static std::vector<std::string> getAvailableModules();

private:
    static std::unordered_map<std::string, std::function<std::unique_ptr<PipelineModule>()>> creators_;
};

/**
 * @brief 모듈 등록을 위한 매크로
 */
#define REGISTER_MODULE(ModuleClass, name) \
    namespace { \
        static bool registered_##ModuleClass = []() { \
            ModuleFactory::registerModule(name, []() { \
                return std::make_unique<ModuleClass>(); \
            }); \
            return true; \
        }(); \
    }
