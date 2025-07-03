#include "Application.h"
#include "Ra2yrGodImpl.h"

#include <ctime>

#include "imgui.h"

namespace Ra2yrGod
{
    static Ra2yrGodImpl::Ra2yrGod god;
    static bool windowsShow = true;
    static int playerMoney = -1;
    static time_t lastMoneyCheck;
    static float balanceHistory[60] = {};
    static int balanceHistoryItem = 0;

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
            mainCommandLabel = "Переподключиться к процессу игры";
        }
        else
        {
            mainCommandLabel = "Подключиться к процессу игры";
        }

        if (ImGui::Button(mainCommandLabel.c_str()))
        {
            god.initProcess();
            processConnected = god.processConnected(true);

            playerMoney = god.getPlayerMoney();
            time(&lastMoneyCheck);

            ImGui::OpenPopup("Подключение##ProcessConnect");
        }

        // Модальное окно открываем по центру родительского
        ImVec2 parentPos = ImGui::GetWindowPos();
        ImVec2 parentSize = ImGui::GetWindowSize();
        ImVec2 modalSize(300, 150);
        ImVec2 modalPos = ImVec2(
            parentPos.x + (parentSize.x - modalSize.x) * 0.5f,
            parentPos.y + (parentSize.y - modalSize.y) * 0.5f
        );
        ImGui::SetNextWindowPos(modalPos, ImGuiCond_Appearing);
        ImGui::SetNextWindowSize(modalSize, ImGuiCond_Appearing);
        if (ImGui::BeginPopupModal("Подключение##ProcessConnect", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            int processId = god.getProcessId();
            std::string processName = god.getProcessName();
            if (processId > 0)
            {
                ImGui::Text("Процесс игры успешно подключен!\n\n%s (%d)", processName.c_str(), processId);
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
                        
            if (ImGui::Button("Обновить##UpdateBalance"))
            {
                playerMoney = god.getPlayerMoney();
                time(&lastMoneyCheck);
            }
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Обновить данные о текущем балансе игрока.");

            ImGui::SameLine();
            std::string frozenBalanceLabel;
            if (god.playerMoneyFrozenState())
            {
                frozenBalanceLabel = "Разморозить";
            }
            else
            {
                frozenBalanceLabel = "Заморозить";
            }
            frozenBalanceLabel += "##FreezeBalance";
            if (ImGui::Button(frozenBalanceLabel.c_str()))
            {
                bool playerMoneyFrozen = !god.playerMoneyFrozenState();
                god.freezePlayerMoney(playerMoneyFrozen, playerMoney);
            }
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("\"Заморозить\" текущий баланс игрока.");
                        
            if (ImGui::InputInt("##Balance", &playerMoney, 1, 1000))
            {
                god.setPlayerMoney(playerMoney);
            };            
            if (!ImGui::IsItemActivated() && !ImGui::IsItemFocused())
            {
                // Раз в секунду обновляем информацию о текущем балансе,
                // если элемент не активен в данный момент.
                time_t currentDateTime;
                time(&currentDateTime);
                int lastCheckTimeLeftSec = std::difftime(currentDateTime, lastMoneyCheck);
                if (lastCheckTimeLeftSec >= 1)
                {
                    playerMoney = god.getPlayerMoney();
                    time(&lastMoneyCheck);

                    balanceHistoryItem++;
                    if (balanceHistoryItem > 60)
                        balanceHistoryItem = 1;
                    balanceHistory[balanceHistoryItem - 1] = playerMoney;                    
                }
            }
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Денежный баланс игрока.");
            ImGui::PlotLines("##BalanceStatistic", balanceHistory, 60);
        }

        ImGui::End();
    }    
}
