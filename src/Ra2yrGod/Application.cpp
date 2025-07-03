#include "Application.h"

#include "Ra2yrGodImpl.h"

#include "imgui.h"

namespace Ra2yrGod
{
    static Ra2yrGodImpl::Ra2yrGod god;

    static bool windowsShow = true;

    static int playerMoney = -1;

    void renderUI(HWND hwnd)
    {
        if (!windowsShow)
        {
            exit(0);
        }
        bool processConnected = god.processConnected();

        ImGui::Begin("Ra2yr God", &windowsShow);

        std::string mainCommandLabel;
        if (processConnected)
        {
            mainCommandLabel = "Повторный поиск игры";
        }
        else
        {
            mainCommandLabel = "Найти игру";
        }

        if (ImGui::Button(mainCommandLabel.c_str()))
        {
            god.initProcess();
            processConnected = god.processConnected(true);

            playerMoney = god.getPlayerMoney();

            ImGui::OpenPopup("Поиск игры");
        }
        if (ImGui::BeginPopupModal("Поиск игры", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            int processId = god.getProcessId();
            std::string processName = god.getProcessName();
            if (processId > 0)
            {
                ImGui::Text("Процесс игры успешно найден!\n\n%s (%d)", processName.c_str(), processId);
            }
            else
            {
                ImGui::Text("Не удалось найти процесс игры!");
            }

            ImGui::Separator();

            if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();

            ImGui::EndPopup();
        }

        if (processConnected)
        {
            ImGui::Text("Процесс: %s (%d)", god.getProcessName().c_str(), god.getProcessId());

            if (ImGui::Button("Получить баланс"))
            {
                playerMoney = god.getPlayerMoney();

                ImGui::OpenPopup("Получить баланс");
            }

            if (ImGui::InputInt("Баланс", &playerMoney, 1, 1000))
            {
                god.setPlayerMoney(playerMoney);
            };

            if (ImGui::BeginPopupModal("Получить баланс", NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {
                if (playerMoney >= 0)
                {
                    ImGui::Text("Текущий баланс игрока: %d", playerMoney);
                }
                else
                {
                    ImGui::Text("Не удалось определить баланс игрока!");
                }

                ImGui::Separator();

                if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
                ImGui::SetItemDefaultFocus();
                ImGui::SameLine();

                ImGui::EndPopup();
            }

            std::string frozenBalanceLabel;
            if (god.playerMoneyFrozenState())
            {
                frozenBalanceLabel = "Разморозить баланс";
            }
            else
            {
                frozenBalanceLabel = "Заморозить баланс";
            }
            if (ImGui::Button(frozenBalanceLabel.c_str()))
            {
                bool playerMoneyFrozen = !god.playerMoneyFrozenState();
                god.freezePlayerMoney(playerMoneyFrozen, playerMoney);
            }
        }

        ImGui::End();
    }    
}
