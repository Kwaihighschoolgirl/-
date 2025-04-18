#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <iomanip> 

struct Vector3D {
    double x = 0.0, y = 0.0, z = 0.0;

    Vector3D() = default;
    Vector3D(double x, double y, double z) : x(x), y(y), z(z) {}

    Vector3D operator+(const Vector3D& other) const {
        return Vector3D(x + other.x, y + other.y, z + other.z);
    }

    Vector3D operator-(const Vector3D& other) const {
        return Vector3D(x - other.x, y - other.y, z - other.z);
    }

    Vector3D operator*(double scalar) const {
        return Vector3D(x * scalar, y * scalar, z * scalar);
    }

    Vector3D operator/(double scalar) const {
        if (scalar == 0) {
             // 0으로 나누기 오류 처리 (예: 예외 발생 또는 기본값 반환)
             std::cerr << "Error: Division by zero!" << std::endl;
             return Vector3D(); // 혹은 다른 적절한 처리
        }
        return Vector3D(x / scalar, y / scalar, z / scalar);
    }

    double magnitude() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    Vector3D normalized() const {
        double mag = magnitude();
        if (mag == 0) return Vector3D(); // 0 벡터 처리
        return *this / mag;
    }
};

// --- 물리 상수 ---
const double G = 6.67430e-11; // 중력 상수 (N m^2 / kg^2)

// --- 행성 클래스 ---
class CelestialBody {
public:
    std::string name;
    double mass;       // kg
    double radius;     // m
    Vector3D position; // m (기준 좌표계 기준)
    Vector3D velocity; // m/s (기준 좌표계 기준)

    CelestialBody(std::string n, double m, double r, Vector3D pos = {}, Vector3D vel = {})
        : name(n), mass(m), radius(r), position(pos), velocity(vel) {}

    // 다른 물체에 작용하는 중력 계산
    Vector3D calculateGravitationalForce(const Vector3D& targetPosition, double targetMass) const {
        Vector3D directionVector = position - targetPosition;
        double distance = directionVector.magnitude();

        if (distance == 0) {
            return Vector3D(); // 같은 위치에 있을 경우 중력 없음 (또는 오류 처리)
        }

        double forceMagnitude = (G * mass * targetMass) / (distance * distance);
        return directionVector.normalized() * forceMagnitude;
    }

    // 시간 경과에 따른 위치 업데이트 (간단한 등속 운동 예시)
    void updatePosition(double dt) {
        position = position + velocity * dt;
    }
};

// --- 우주선 클래스 ---
class Spacecraft {
public:
    std::string name = "DefaultCraft";
    Vector3D position;        // m
    Vector3D velocity;        // m/s
    double dryMass;           // kg (연료 제외 질량)
    double fuelMass;          // kg
    double engineThrust;      // N (최대 추력)
    double specificImpulse;   // s (비추력, 엔진 효율 지표)
    bool engineOn = false;
    Vector3D thrustDirection = {1.0, 0.0, 0.0}; // 엔진 추력 방향 (정규화된 벡터)

    Spacecraft(double dryM, double fuelM, double thrust, double isp)
        : dryMass(dryM), fuelMass(fuelM), engineThrust(thrust), specificImpulse(isp) {}

    double getTotalMass() const {
        return dryMass + fuelMass;
    }

    // 엔진 점화/소화
    void setEngineState(bool on) {
        engineOn = on;
    }

    // 엔진 추력 설정 (필요 시)
    void setThrustDirection(const Vector3D& direction) {
        thrustDirection = direction.normalized();
    }

    // 연료 소모 및 추력 계산 (dt 시간 동안 엔진 작동 시)
    // 반환값: 실제 적용된 추력 벡터
    Vector3D consumeFuelAndApplyThrust(double dt) {
        if (!engineOn || fuelMass <= 0) {
            return Vector3D(); // 엔진 꺼짐 또는 연료 없음
        }

        // 로켓 방정식 관련: 질량 유량 (mass flow rate) = Thrust / (Isp * g0)
        const double g0 = 9.80665; // 표준 중력 가속도
        double massFlowRate = engineThrust / (specificImpulse * g0);
        double fuelConsumed = massFlowRate * dt;

        // 연료가 부족하면 가능한 만큼만 소모하고 추력 감소
        if (fuelConsumed > fuelMass) {
            fuelConsumed = fuelMass;
            double actualThrustMagnitude = massFlowRate * specificImpulse * g0 * (fuelConsumed / (massFlowRate * dt)); // 비례적으로 추력 계산
            fuelMass = 0;
            engineOn = false; // 연료 소진 시 엔진 자동 정지
             std::cout << "WARNING: Fuel depleted! Engine shut down." << std::endl;
            return thrustDirection * actualThrustMagnitude;
        } else {
            fuelMass -= fuelConsumed;
            return thrustDirection * engineThrust;
        }
    }

    // 외부 힘(중력 등)과 엔진 추력을 받아 상태 업데이트 (dt 시간 후)
    void updateState(const Vector3D& netExternalForce, double dt) {
        Vector3D thrustForce = consumeFuelAndApplyThrust(dt);
        Vector3D totalForce = netExternalForce + thrustForce;
        double currentMass = getTotalMass();

        if (currentMass <= 0) {
             std::cerr << "Error: Spacecraft mass is zero or negative!" << std::endl;
             return; // 질량이 0 이하면 계산 불가
        }

        Vector3D acceleration = totalForce / currentMass;

        // 간단한 오일러 적분 (더 정확한 방법으로 개선 필요: RK4 등)
        velocity = velocity + acceleration * dt;
        position = position + velocity * dt;
    }
};


// --- 물리 엔진 클래스 ---
class PhysicsEngine {
public:
    std::vector<CelestialBody*> celestialBodies;
    std::vector<Spacecraft*> spacecrafts;

    void addBody(CelestialBody* body) {
        celestialBodies.push_back(body);
    }

    void addSpacecraft(Spacecraft* craft) {
        spacecrafts.push_back(craft);
    }

    // 전체 시스템 상태 업데이트
    void update(double dt) {
        // 1. 행성 위치 업데이트 (필요 시, 여기서는 정적이라고 가정)
        // for (CelestialBody* body : celestialBodies) {
        //     body->updatePosition(dt);
        // }

        // 2. 각 우주선에 작용하는 힘 계산 및 상태 업데이트
        for (Spacecraft* craft : spacecrafts) {
            Vector3D netGravitationalForce;
            // 모든 행성으로부터 받는 중력 합산
            for (const CelestialBody* body : celestialBodies) {
                netGravitationalForce = netGravitationalForce + body->calculateGravitationalForce(craft->position, craft->getTotalMass());
            }

            // (추가) 다른 우주선 간의 중력 등 다른 힘들도 고려 가능

            // 우주선 상태 업데이트 (중력 + 엔진 추력)
            craft->updateState(netGravitationalForce, dt);
        }
    }
};

// --- 텍스트 기반 UI 클래스 ---
class TextUI {
public:
    // 사용자로부터 발사 파라미터 입력 받기 (예시)
    void getLaunchParameters(double& angle, double& initialSpeed, double& thrust, double& fuel) {
        std::cout << "--- Launch Parameters ---" << std::endl;
        std::cout << "Enter Launch Angle (degrees from horizon): ";
        std::cin >> angle;
        std::cout << "Enter Initial Speed (m/s relative to surface - simplified): ";
        std::cin >> initialSpeed;
        std::cout << "Enter Engine Max Thrust (N): ";
        std::cin >> thrust;
        std::cout << "Enter Initial Fuel Mass (kg): ";
        std::cin >> fuel;
        std::cout << "-------------------------" << std::endl;
    }

     // 사용자로부터 시뮬레이션 시간 및 스텝 입력 받기 (예시)
    void getSimulationSettings(double& totalTime, double& dt) {
        std::cout << "--- Simulation Settings ---" << std::endl;
        std::cout << "Enter total simulation time (seconds): ";
        std::cin >> totalTime;
        std::cout << "Enter time step dt (seconds): ";
        std::cin >> dt;
        std::cout << "---------------------------" << std::endl;
    }


    // 시뮬레이션 상태 출력
    void displayState(const Spacecraft& craft, double time) {
        std::cout << std::fixed << std::setprecision(2); // 소수점 2자리까지 고정
        std::cout << "Time: " << time << " s | ";
        std::cout << "Pos: (" << craft.position.x / 1000.0 << ", " << craft.position.y / 1000.0 << ", " << craft.position.z / 1000.0 << ") km | ";
        std::cout << "Vel: (" << craft.velocity.x << ", " << craft.velocity.y << ", " << craft.velocity.z << ") m/s | ";
        std::cout << "Speed: " << craft.velocity.magnitude() << " m/s | ";
        std::cout << "Fuel: " << craft.fuelMass << " kg | ";
        std::cout << "Mass: " << craft.getTotalMass() << " kg | ";
        std::cout << "Engine: " << (craft.engineOn ? "ON" : "OFF") << std::endl;
    }

    // 간단한 메뉴 또는 명령 입력 처리 (예시)
    void showMenu() {
        std::cout << "\n--- Commands ---" << std::endl;
        std::cout << "  'e' : Toggle Engine ON/OFF" << std::endl;
        std::cout << "  'd' : Set Thrust Direction (Not implemented in this simple example)" << std::endl;
        std::cout << "  'q' : Quit Simulation" << std::endl;
        std::cout << "  (Press Enter to step forward)" << std::endl;
        std::cout << "Enter command: ";
    }

    char getCommand() {
        std::string line;
        std::getline(std::cin >> std::ws, line); // 버퍼 비우고 한 줄 읽기
        if (line.length() == 1) {
            return line[0];
        } else if (line.empty()) {
            return '\n'; // Enter 키는 스텝 진행으로 처리
        }
        return 0; // 유효하지 않은 입력
    }

};

// --- 시뮬레이션 주 클래스 ---
class Simulation {
private:
    PhysicsEngine physicsEngine;
    TextUI ui;
    Spacecraft* mainCraft = nullptr; // 시뮬레이션 대상 주 우주선
    double currentTime = 0.0;
    double dt = 1.0; // 기본 시간 간격 (초)
    double totalSimulationTime = 600.0; // 기본 총 시뮬레이션 시간

    // 기본 천체 설정 (예: 지구)
    void setupEnvironment() {
        // 지구 데이터 (근사치)
        double earthMass = 5.972e24; // kg
        double earthRadius = 6371e3; // m
        CelestialBody* earth = new CelestialBody("Earth", earthMass, earthRadius);
        physicsEngine.addBody(earth); // 지구를 물리 엔진에 추가

        // 달, 태양 등 다른 천체도 추가 가능 귀찮으니 안함
        // double sunMass = 1.989e30;
        // Vector3D sunPosition = { -1.496e11, 0, 0 }; // 지구 기준 태양 위치 (간략화)
        // CelestialBody* sun = new CelestialBody("Sun", sunMass, 695700e3, sunPosition);
        // physicsEngine.addBody(sun);
    }

    // 우주선 초기 설정
    void setupSpacecraft() {
        double launchAngle, initialSpeed, thrust, fuel;
        ui.getLaunchParameters(launchAngle, initialSpeed, thrust, fuel);

        // 발사 각도와 초기 속도를 벡터로 변환 (지구 표면 기준, 매우 간략화된 모델)
        // TODO: 실제 발사 시뮬레이션은 지구 자전 속도, 발사 위치 위도 등을 고려해야 함
        double angleRad = launchAngle * M_PI / 180.0;
        Vector3D initialVelocityRelSurface = {initialSpeed * cos(angleRad), initialSpeed * sin(angleRad), 0};

        // 발사 위치 (예: 지구 적도 표면)
        CelestialBody* earth = physicsEngine.celestialBodies[0]; // 첫 번째 천체를 지구로 가정
        Vector3D launchPosition = {earth->radius, 0, 0}; // 지구 중심 기준 X축 방향 표면

        // TODO: 지구 자전 속도 고려
        // Vector3D earthRotationVelocity = {0, 0, earth->radius * 2 * M_PI / (24 * 3600)};
        // Vector3D absoluteInitialVelocity = initialVelocityRelSurface + earthRotationVelocity;
        Vector3D absoluteInitialVelocity = initialVelocityRelSurface; // 간략화


        double dryMass = 5000; // 예시 건조 질량 (kg)
        double isp = 300;      // 예시 비추력 (s)

        mainCraft = new Spacecraft(dryMass, fuel, thrust, isp);
        mainCraft->position = launchPosition + earth->position; // 지구 위치 고려
        mainCraft->velocity = absoluteInitialVelocity + earth->velocity; // 지구 공전 속도 고려 (여기서는 0으로 가정)
        mainCraft->setThrustDirection({cos(angleRad), sin(angleRad), 0}); // 초기 추력 방향 설정

        physicsEngine.addSpacecraft(mainCraft);
    }

public:
    Simulation() {
        // 소멸자에서 메모리 해제 필요
        // std::unique_ptr 또는 std::shared_ptr 사용 권장
    }

    ~Simulation() {
         // 동적으로 할당된 메모리 해제
        for (CelestialBody* body : physicsEngine.celestialBodies) {
            delete body;
        }
        for (Spacecraft* craft : physicsEngine.spacecrafts) {
            delete craft; // mainCraft와 중복 해제될 수 있으니 주의 필요
        }
        // 포인터 벡터를 비워 dangling pointer 방지
        physicsEngine.celestialBodies.clear();
        physicsEngine.spacecrafts.clear();
        mainCraft = nullptr; // 이미 해제되었음을 명시
    }


    void run() {
        setupEnvironment();
        setupSpacecraft();
        ui.getSimulationSettings(totalSimulationTime, dt);


        if (!mainCraft) {
            std::cerr << "Error: Spacecraft not initialized!" << std::endl;
            return;
        }

        std::cout << "\n--- Simulation Start ---" << std::endl;
        ui.displayState(*mainCraft, currentTime);

        while (currentTime < totalSimulationTime) {
            ui.showMenu();
            char command = ui.getCommand();

             if (command == 'q') {
                 std::cout << "Quitting simulation." << std::endl;
                 break;
             } else if (command == 'e') {
                 mainCraft->setEngineState(!mainCraft->engineOn);
                 std::cout << "Engine toggled " << (mainCraft->engineOn ? "ON" : "OFF") << std::endl;
             } else if (command == 'd') {
                 // TODO: 추력 방향 변경 로직 구현
                 std::cout << "Set Thrust Direction (Not implemented yet)." << std::endl;
             } else if (command == '\n') { // Enter 키: 시간 진행
                 // 시뮬레이션 스텝 진행
                 physicsEngine.update(dt);
                 currentTime += dt;
                 ui.displayState(*mainCraft, currentTime);

                 // TODO: 궤도 진입 성공 여부 판정 로직
                 // 예: 고도, 속도 등을 기준으로 지구 궤도 진입 조건 확인
             } else if (command != 0) { // 유효하지 않은 문자 입력 시
                 std::cout << "Invalid command." << std::endl;
             }

             // 간단한 충돌 감지 (행성 표면에 닿았는지)
             for (const CelestialBody* body : physicsEngine.celestialBodies) {
                 double distanceToCenter = (mainCraft->position - body->position).magnitude();
                 if (distanceToCenter <= body->radius) {
                     std::cout << "\nCRASH! Impacted " << body->name << " at time " << currentTime << " s." << std::endl;
                     return; // 시뮬레이션 종료
                 }
             }
        }

        std::cout << "--- Simulation End ---" << std::endl;
    }
};


int main() {
    Simulation sim;
    sim.run();

    return 0;
}