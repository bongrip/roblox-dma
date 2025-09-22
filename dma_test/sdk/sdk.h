#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <memory>

class RobloxSDK {
private:
    uint64_t base;
    uint64_t datamodel;
    uint64_t workspace;
    uint64_t players_service;
    uint64_t local_player;

    std::string readstring(uint64_t address, size_t maxLen = 200);
    std::string GetName(uint64_t address);
    std::vector<uint64_t> GetChildren(uint64_t address);
    uint64_t FindFirstChild(uint64_t address, const std::string& name);

public:
    class LocalPlayer {
    private:
        RobloxSDK* sdk;

    public:
        LocalPlayer(RobloxSDK* parent);

        std::string GetName();
        std::string GetDisplayName();
        uint64_t GetCharacter();
        uint64_t GetHumanoid();
        uint64_t GetRootPart();

        void SetWalkSpeed(float speed);
        void SetJumpPower(float power);
        void SetHealth(float health);
        void SetMaxHealth(float maxHealth);

        float GetWalkSpeed();
        float GetJumpPower();
        float GetHealth();
        float GetMaxHealth();

        float GetPositionX();
        float GetPositionY();
        float GetPositionZ();
        void PrintPosition();

        bool IsValid();
    };

    class Workspace {
    private:
        RobloxSDK* sdk;

    public:
        Workspace(RobloxSDK* parent);

        void SetGravity(float gravity);
        float GetGravity();
        uint64_t FindFirstChild(const std::string& name);
        std::vector<uint64_t> GetChildren();
    };

    LocalPlayer LocalPlayerInstance;
    Workspace WorkspaceInstance;

    RobloxSDK();

    bool Init();

    uint64_t GetDataModel();
    uint64_t GetWorkspace();
    uint64_t GetPlayersService();
    uint64_t GetLocalPlayer();

    std::string GetInstanceName(uint64_t address);
    uint64_t FindChild(uint64_t parent, const std::string& name);
    std::vector<uint64_t> GetInstanceChildren(uint64_t address);

    void PrintDebugInfo();
};

extern std::unique_ptr<RobloxSDK> SDK;