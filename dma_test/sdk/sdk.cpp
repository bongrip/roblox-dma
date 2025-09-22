#include "SDK.h"
#include "../ext/memory/memory.h"

std::unique_ptr<RobloxSDK> SDK = std::make_unique<RobloxSDK>();

std::string RobloxSDK::readstring(uint64_t address, size_t maxLen) {
    std::string result;
    result.reserve(maxLen);
    for (size_t i = 0; i < maxLen; ++i) {
        char c = TargetProcess.Read<char>(address + i);
        if (c == '\0') break;
        result.push_back(c);
    }
    return result;
}

std::string RobloxSDK::GetName(uint64_t address) {
    auto ptr = TargetProcess.Read<uint64_t>(address + 0x88);
    return ptr ? readstring(ptr) : "";
}

std::vector<uint64_t> RobloxSDK::GetChildren(uint64_t address) {
    std::vector<uint64_t> children;
    auto start = TargetProcess.Read<uint64_t>(address + 0x68);
    auto end = TargetProcess.Read<uint64_t>(start + 0x8);

    for (auto ptr = TargetProcess.Read<uint64_t>(start); ptr != end; ptr += 0x10) {
        children.push_back(TargetProcess.Read<uint64_t>(ptr));
    }
    return children;
}

uint64_t RobloxSDK::FindFirstChild(uint64_t address, const std::string& name) {
    for (const auto& child : GetChildren(address)) {
        if (GetName(child) == name) return child;
    }
    return 0;
}

RobloxSDK::RobloxSDK() : LocalPlayerInstance(this), WorkspaceInstance(this) {
    base = 0;
    datamodel = 0;
    workspace = 0;
    players_service = 0;
    local_player = 0;
}

bool RobloxSDK::Init() {
    if (!TargetProcess.Init("RobloxPlayerBeta.exe")) {
        return false;
    }

    base = TargetProcess.GetBaseAddress("RobloxPlayerBeta.exe");
    uint64_t fake_datamodel = TargetProcess.Read<uint64_t>(base + 0x70A86B8);
    datamodel = TargetProcess.Read<uint64_t>(fake_datamodel + 0x1C0);

    if (!datamodel) {
        return false;
    }

    workspace = FindFirstChild(datamodel, "Workspace");
    players_service = FindFirstChild(datamodel, "Players");

    if (!workspace || !players_service) {
        return false;
    }

    local_player = TargetProcess.Read<uint64_t>(players_service + 0x128);

    return local_player != 0;
}

uint64_t RobloxSDK::GetDataModel() { return datamodel; }
uint64_t RobloxSDK::GetWorkspace() { return workspace; }
uint64_t RobloxSDK::GetPlayersService() { return players_service; }
uint64_t RobloxSDK::GetLocalPlayer() { return local_player; }

std::string RobloxSDK::GetInstanceName(uint64_t address) {
    return GetName(address);
}

uint64_t RobloxSDK::FindChild(uint64_t parent, const std::string& name) {
    return FindFirstChild(parent, name);
}

std::vector<uint64_t> RobloxSDK::GetInstanceChildren(uint64_t address) {
    return GetChildren(address);
}

void RobloxSDK::PrintDebugInfo() {
    printf("[SDK/Debug] Base: 0x%llx\n", base);
    printf("[SDK/Debug] DataModel: 0x%llx\n", datamodel);
    printf("[SDK/Debug] Workspace: 0x%llx\n", workspace);
    printf("[SDK/Debug] Players Service: 0x%llx\n", players_service);
    printf("[SDK/Debug] Local Player: 0x%llx\n", local_player);
    if (LocalPlayerInstance.IsValid()) {
        printf("[SDK/Debug] Player Name: %s\n", LocalPlayerInstance.GetName().c_str());
        printf("[SDK/Debug] Display Name: %s\n", LocalPlayerInstance.GetDisplayName().c_str());
    }
}

RobloxSDK::LocalPlayer::LocalPlayer(RobloxSDK* parent) : sdk(parent) {}

std::string RobloxSDK::LocalPlayer::GetName() {
    return sdk->GetName(sdk->local_player);
}

std::string RobloxSDK::LocalPlayer::GetDisplayName() {
    uint64_t display_name_ptr = TargetProcess.Read<uint64_t>(sdk->local_player + 0x118);
    return display_name_ptr ? sdk->readstring(display_name_ptr) : "";
}

uint64_t RobloxSDK::LocalPlayer::GetCharacter() {
    return sdk->FindFirstChild(sdk->workspace, GetName());
}

uint64_t RobloxSDK::LocalPlayer::GetHumanoid() {
    uint64_t character = GetCharacter();
    return character ? sdk->FindFirstChild(character, "Humanoid") : 0;
}

uint64_t RobloxSDK::LocalPlayer::GetRootPart() {
    uint64_t character = GetCharacter();
    if (!character) return 0;

    uint64_t rootPart = sdk->FindFirstChild(character, "HumanoidRootPart");
    if (!rootPart) {
        rootPart = sdk->FindFirstChild(character, "Torso");
    }
    return rootPart;
}

void RobloxSDK::LocalPlayer::SetWalkSpeed(float speed) {
    uint64_t humanoid = GetHumanoid();
    if (humanoid) {
        TargetProcess.Write<float>(humanoid + 0x1DC, speed);
    }
}

void RobloxSDK::LocalPlayer::SetJumpPower(float power) {
    uint64_t humanoid = GetHumanoid();
    if (humanoid) {
        TargetProcess.Write<float>(humanoid + 0x1B8, power);
    }
}

void RobloxSDK::LocalPlayer::SetHealth(float health) {
    uint64_t humanoid = GetHumanoid();
    if (humanoid) {
        TargetProcess.Write<float>(humanoid + 0x19C, health);
    }
}

void RobloxSDK::LocalPlayer::SetMaxHealth(float maxHealth) {
    uint64_t humanoid = GetHumanoid();
    if (humanoid) {
        TargetProcess.Write<float>(humanoid + 0x1BC, maxHealth);
    }
}

float RobloxSDK::LocalPlayer::GetWalkSpeed() {
    uint64_t humanoid = GetHumanoid();
    return humanoid ? TargetProcess.Read<float>(humanoid + 0x1DC) : 0.0f;
}

float RobloxSDK::LocalPlayer::GetJumpPower() {
    uint64_t humanoid = GetHumanoid();
    return humanoid ? TargetProcess.Read<float>(humanoid + 0x1B8) : 0.0f;
}

float RobloxSDK::LocalPlayer::GetHealth() {
    uint64_t humanoid = GetHumanoid();
    return humanoid ? TargetProcess.Read<float>(humanoid + 0x19C) : 0.0f;
}

float RobloxSDK::LocalPlayer::GetMaxHealth() {
    uint64_t humanoid = GetHumanoid();
    return humanoid ? TargetProcess.Read<float>(humanoid + 0x1BC) : 0.0f;
}

float RobloxSDK::LocalPlayer::GetPositionX() {
    uint64_t character = GetCharacter();
    if (!character) return 0.0f;
    uint64_t lowerTorso = sdk->FindChild(character, "LowerTorso");
    if (!lowerTorso) return 0.0f;
    uint64_t primitive = TargetProcess.Read<uint64_t>(lowerTorso + 0x178);
    if (!primitive) return 0.0f;
    return TargetProcess.Read<float>(primitive + 0x150);
}

float RobloxSDK::LocalPlayer::GetPositionY() {
    uint64_t character = GetCharacter();
    if (!character) return 0.0f;
    uint64_t lowerTorso = sdk->FindChild(character, "LowerTorso");
    if (!lowerTorso) return 0.0f;
    uint64_t primitive = TargetProcess.Read<uint64_t>(lowerTorso + 0x178);
    if (!primitive) return 0.0f;
    return TargetProcess.Read<float>(primitive + 0x150 + 0x4);
}

float RobloxSDK::LocalPlayer::GetPositionZ() {
    uint64_t character = GetCharacter();
    if (!character) return 0.0f;
    uint64_t lowerTorso = sdk->FindChild(character, "LowerTorso");
    if (!lowerTorso) return 0.0f;
    uint64_t primitive = TargetProcess.Read<uint64_t>(lowerTorso + 0x178);
    if (!primitive) return 0.0f;
    return TargetProcess.Read<float>(primitive + 0x150 + 0x8);
}

void RobloxSDK::LocalPlayer::PrintPosition() {
    printf("[Debug] X: %.2f, Y: %.2f, Z: %.2f\n", GetPositionX(), GetPositionY(), GetPositionZ());
}

bool RobloxSDK::LocalPlayer::IsValid() {
    return sdk->local_player != 0;
}

RobloxSDK::Workspace::Workspace(RobloxSDK* parent) : sdk(parent) {}

void RobloxSDK::Workspace::SetGravity(float gravity) {
    if (sdk->workspace) {
        TargetProcess.Write<float>(sdk->workspace + 0x998, gravity);
    }
}

float RobloxSDK::Workspace::GetGravity() {
    return sdk->workspace ? TargetProcess.Read<float>(sdk->workspace + 0x998) : 0.0f;
}

uint64_t RobloxSDK::Workspace::FindFirstChild(const std::string& name) {
    return sdk->FindFirstChild(sdk->workspace, name);
}

std::vector<uint64_t> RobloxSDK::Workspace::GetChildren() {
    return sdk->GetChildren(sdk->workspace);
}