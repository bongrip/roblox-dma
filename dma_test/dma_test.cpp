#include "sdk/SDK.h"
#include <thread>
#include <chrono>

int main() {
    if (!SDK->Init()) {
        printf("[-] Failed to initialize SDK\n");
        return -1;
    }

    printf("[+] SDK initialized! Player: %s\n", SDK->LocalPlayerInstance.GetName().c_str());
	printf("[+] Walk Speed: %.2f\n", SDK->LocalPlayerInstance.GetWalkSpeed());
    return 0;
}