/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ScriptMgr.h"
#include "ObjectMgr.h"
#include "Chat.h"
#include "AnticheatMgr.h"

class anticheat_commandscript : public CommandScript
{
public:
    anticheat_commandscript() : CommandScript("anticheat_commandscript") { }

    ChatCommand* GetCommands() const
    {
        static ChatCommand anticheatCommandTable[] =
        {
            { "global",         SEC_GAMEMASTER,     true,  &HandleAntiCheatGlobalCommand,         "", NULL },
            { "player",         SEC_GAMEMASTER,     true,  &HandleAntiCheatPlayerCommand,         "", NULL },
            { "delete",         SEC_ADMINISTRATOR,  true,  &HandleAntiCheatDeleteCommand,         "", NULL },
            { "handle",         SEC_ADMINISTRATOR,  true,  &HandleAntiCheatHandleCommand,         "", NULL },
            { "jail",           SEC_GAMEMASTER,     true,  &HandleAnticheatJailCommand,         "", NULL },
            { "warn",           SEC_GAMEMASTER,     true,  &HandleAnticheatWarnCommand,         "", NULL },
            { NULL,             0,                     false, NULL,                                           "", NULL }
        };

        static ChatCommand commandTable[] =
        {
            { "anticheat",      SEC_GAMEMASTER,     true, NULL,                     "",  anticheatCommandTable},
            { NULL,             0,                  false, NULL,                               "", NULL }
        };

        return commandTable;
    }

    static bool HandleAnticheatWarnCommand(ChatHandler* handler, const char* args)
    {
        if (!sWorld->getBoolConfig(CONFIG_ANTICHEAT_ENABLE))
            return false;

        Player* pTarget = NULL;
        
        std::string strCommand;

        char* command = strtok((char*)args, " ");

        if (command)
        {
            strCommand = command;
            normalizePlayerName(strCommand);

            pTarget = sObjectMgr->GetPlayer(strCommand.c_str()); //get player by name
        }else 
            pTarget = handler->getSelectedPlayer();

        if (!pTarget)
            return false;
        
        WorldPacket data;

        // need copy to prevent corruption by strtok call in LineFromMessage original string
        char* buf = strdup("OWNED! Le systeme anti-triche montre que vous tricher...A Dieu");
        char* pos = buf;

        while (char* line = handler->LineFromMessage(pos))
        {
            handler->FillSystemMessageData(&data, line);
            pTarget->GetSession()->SendPacket(&data);
        }

        free(buf);
        return true;
    }

    static bool HandleAnticheatJailCommand(ChatHandler* handler, const char* args)
    {
        if (!sWorld->getBoolConfig(CONFIG_ANTICHEAT_ENABLE))
            return false;

        Player* pTarget = NULL;
        
        std::string strCommand;

        char* command = strtok((char*)args, " ");

        if (command)
        {
            strCommand = command;
            normalizePlayerName(strCommand);

            pTarget = sObjectMgr->GetPlayer(strCommand.c_str()); //get player by name
        }else 
            pTarget = handler->getSelectedPlayer();

        if (!pTarget)
        {
            handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (pTarget == handler->GetSession()->GetPlayer())
            return false;
    
        // teleport both to jail. et gel
        pTarget->TeleportTo(1,16226.5f,16403.6f,-64.5f,3.2f);
		//me->DoCast(pPlayer, 9454);
		handler->PSendSysMessage("OWNED Ptit moche !");
        handler->GetSession()->GetPlayer()->TeleportTo(1,16226.5f,16403.6f,-64.5f,3.2f);

        WorldLocation loc;

        // the player should be already there, but no :(
        // pTarget->GetPosition(&loc);

        loc.m_mapId = 1;
        loc.m_positionX = 16226.5f;
        loc.m_positionY = 16403.6f;
        loc.m_positionZ = -64.5f;
        loc.m_orientation = 3.2f;

        pTarget->SetHomebind(loc,876);
        return true;
    }

    static bool HandleAntiCheatDeleteCommand(ChatHandler* handler, const char* args)
    {
        if (!sWorld->getBoolConfig(CONFIG_ANTICHEAT_ENABLE))
            return false;

        std::string strCommand;

        char* command = strtok((char*)args, " "); //get entered name

        if (!command)
            return true;
        
        strCommand = command;
        
        if (strCommand.compare("deleteall") == 0)
            sAnticheatMgr->AnticheatDeleteCommand(0);
        else
        {
            normalizePlayerName(strCommand);
            Player* player = sObjectMgr->GetPlayer(strCommand.c_str()); //get player by name
            if (!player)
                handler->PSendSysMessage("Le Joueur n'existe pas");
            else
                sAnticheatMgr->AnticheatDeleteCommand(player->GetGUIDLow());
        }

        return true;
    }

    static bool HandleAntiCheatPlayerCommand(ChatHandler* handler, const char* args)
    {
        if (!sWorld->getBoolConfig(CONFIG_ANTICHEAT_ENABLE))
            return false;

        std::string strCommand;

        char* command = strtok((char*)args, " ");

        uint32 guid = 0;
        Player* player = NULL;
        
        if (command)
        {
            strCommand = command;

            normalizePlayerName(strCommand);
            player = sObjectMgr->GetPlayer(strCommand.c_str()); //get player by name

            if (player)
                guid = player->GetGUIDLow();
        }else 
        {
            player = handler->getSelectedPlayer();
            if (player)
                guid = player->GetGUIDLow();  
        }

        if (!guid)
        {
            handler->PSendSysMessage("Il n'y a pas de joueur");
            return true;
        }

        float average = sAnticheatMgr->GetAverage(guid);
        uint32 total_reports = sAnticheatMgr->GetTotalReports(guid);
        uint32 speed_reports = sAnticheatMgr->GetTypeReports(guid,0);
        uint32 fly_reports = sAnticheatMgr->GetTypeReports(guid,1);
        uint32 jump_reports = sAnticheatMgr->GetTypeReports(guid,3);
        uint32 waterwalk_reports = sAnticheatMgr->GetTypeReports(guid,2);
        uint32 teleportplane_reports = sAnticheatMgr->GetTypeReports(guid,4);
        uint32 climb_reports = sAnticheatMgr->GetTypeReports(guid,5);

        handler->PSendSysMessage("Information sur le joueur %s",player->GetName());
        handler->PSendSysMessage("Moyenne: %f || Rapports Totals: %u ",average,total_reports);
        handler->PSendSysMessage("Rapport Speed: %u || Rapport Fly: %u || Rapport Jump: %u ",speed_reports,fly_reports,jump_reports);
        handler->PSendSysMessage("Rapport Walk On Water: %u  || Rapport Teleport To Plane: %u",waterwalk_reports,teleportplane_reports);
        handler->PSendSysMessage("Rapport Climb: %u",climb_reports);

        return true;
    }

    static bool HandleAntiCheatHandleCommand(ChatHandler* handler, const char* args)
    {
        std::string strCommand;

        char* command = strtok((char*)args, " ");
        
        if (!command)
            return true;

        if (!handler->GetSession()->GetPlayer())
            return true;

        strCommand = command;

        if (strCommand.compare("on") == 0)
        {
            sWorld->setBoolConfig(CONFIG_ANTICHEAT_ENABLE,true);
            handler->SendSysMessage("Le Systeme anticheat est maintenant: OK!");
        }
        else if (strCommand.compare("off") == 0)
        {
            sWorld->setBoolConfig(CONFIG_ANTICHEAT_ENABLE,false);
            handler->SendSysMessage("Le Systeme anticheat est maintenant: OFF!");
        }
        
        return true;
    }

    static bool HandleAntiCheatGlobalCommand(ChatHandler* handler, const char* args)
    {
        if (!sWorld->getBoolConfig(CONFIG_ANTICHEAT_ENABLE))
        {
            handler->PSendSysMessage("Le Systeme anticheat est OFF.");
            return true;
        }

        sAnticheatMgr->AnticheatGlobalCommand(handler);

        return true;
    }
};

void AddSC_anticheat_commandscript()
{
    new anticheat_commandscript();
}