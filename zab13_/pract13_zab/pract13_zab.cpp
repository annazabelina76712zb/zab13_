#include <iostream>
#include <windows.h>
#include <mutex>
using namespace std;
struct Bayum
{
    long health = 9000000000;
    int resist = 44;
    int damage = 73843;
    int specialDamage = 150000;
    int attackCooldawn = 5;
    int specialCooldawn = 10;

};
struct Player
{
    long health = 500000;
    int resist = 12000;
    int damage = 30000;
    int specialDamage = 2;
    int attackCooldawn = 5;
    int specialCooldawn = 20;
    int defense = 20;
    int dobgeChance = 15;
    char name[64];
    bool isAlive = true;
    int damageTop;
};
HANDLE hboss;
HANDLE hPlayer;
HANDLE hBossSpecEv;

Bayum boss;
bool bossAlive = true;
Player players[10];
int playersCount = 0;
int alivePlCount = 0;
long long DamageBoss(int playerDamage)
{
    return playerDamage * (100 - boss.resist) / 100;
}
long long DamagePlayer(int bossDamage, int playerDef)
{
    return bossDamage * (100 - playerDef) / 100;
}
void BossThread()
{
    while (bossAlive)
    {
        Sleep(1000);
        WaitForSingleObject(hboss, INFINITE);

        if (!bossAlive) break;
        {
            int aliveInd[10];
            int aliveCount = 0;
            for (int i=0; i<playersCount;i++)
            {
                if (players[i].isAlive && players[i].health > 0)
                {
                    aliveInd[aliveCount] = i;
                    aliveCount++;
                }
            }

            if (aliveCount > 0)
            {
                int targetIndex = aliveInd[rand() % alivePlCount];
                if (rand()%100<players[targetIndex].dobgeChance)
                {
                    cout << "Атака Босса " << players[targetIndex].name << "Игрок уклонился!" << endl;
                }
                else
                {
                    long long damage = DamagePlayer(boss.damage, players[targetIndex].defense);
                    cout << "Атака босса " << players[targetIndex].name << "Урон по игроку: " << damage << "Здоровье игрока: " << players[targetIndex].health << endl;
                }
                if (players[targetIndex].health <= 0)
                {
                    players[targetIndex].isAlive = false;
                    alivePlCount--;
                    cout << players[targetIndex].name << " Погиб!" << endl;
                }
                Sleep(1000);
               
            }
        }

    }
    SetEvent(hPlayer);
    this_thread::sleep_for(chrono::seconds(boss.attackCooldawn));
}
void BossSpecialThread()
{
    while (bossAlive)
    {
         Sleep(1000);
        if (!bossAlive) break;
        SetEvent(hBossSpecEv);
        {
            int aliveCount = 0;
            for (int i = 0; i < playersCount; i++)
            {
                if (players[i].isAlive) aliveCount++;
            }
            if (aliveCount > 0)
            {
                double specialAttack = 1.0;
              if (aliveCount > 0)
                {
                    specialAttack = 1.0 - 0.05 * (aliveCount - 1);
                }
                int specialDamage = boss.specialDamage * specialAttack;
                cout << "СПЕЦАТАКА БОССА" << endl;
            }
        }
    }
}
void PlayerThread(int playerId)
{
    while (bossAlive && players[playerId].isAlive)
    {
        Sleep(100);
        WaitForSingleObject(hPlayer, INFINITE);
        if (bossAlive && players[playerId].isAlive) break;
        {
            long damage = DamageBoss(players[playerId].damage);
            boss.health -= damage;
            players[playerId].attackCooldawn += damage;
            cout << players[playerId].name << " атакует босса и наносит урона: " << damage << " Здоровье босса: " << boss.health << endl;
            if (boss.health <= 0)
            {
                bossAlive = false;
                cout << " Босс побеждён!" << endl;
                break;
                Sleep(100);
            } 

        }
    }
    
    SetEvent(hboss);
    this_thread::sleep_for(chrono::seconds(players[playerId].attackCooldawn));
}
int main()
{
    setlocale(LC_ALL, "RU");
    hboss = CreateEvent(NULL, FALSE, TRUE, L"BossEvent");
    hPlayer = CreateEvent(NULL, FALSE, FALSE, L"PlayersEvent");
    hBossSpecEv = CreateEvent(NULL, FALSE, FALSE, L"BossSpecialEvent");
    cout << "Введите количество играков (1-10)" << endl;
    cin >> playersCount;
    Sleep(10000);
    if (playersCount > 10 || playersCount < 1)
    {
        cout << "Неверное количество играков!" << endl;
        return 1;
        
        for (int i = 0; i < playersCount; i++)
        {
            cout << "ВВедите имя игрока " << i+1 << endl;
            cin >> players[i].name;
            players[i].isAlive = true;
            players[i].health = 500000;
            players[i].damageTop = 0;
        }

        Sleep(10000);
        alivePlCount = playersCount;
        cout << "НАЧАЛО БИТВЫ" << endl;

        thread bossThread(BossThread);
        thread bossSpecialThread(BossSpecialThread);
        thread playersThread[10];
        for (int i = 0; i < playersCount; i++)
        {
            
            playersThread[i] = thread(PlayerThread, i);

        }
        while (bossAlive && alivePlCount > 0)
        {
            this_thread::sleep_for(chrono::milliseconds(100));
        }
        bossAlive = false;
        SetEvent(hboss);
        SetEvent(hPlayer);
        SetEvent(hBossSpecEv);
        Sleep(1000);
        for (int i = 0; i = playersCount; i++)
        {
            playersThread[i].join();
        }
        cout << "Результат битвы" << endl;
        if (boss.health <= 0)
        {
            cout << "Победила команда игроков!" << endl;
        }
        else
        {
            cout << "Победил босс!" << endl;
        }
        cout << " \nТоп три игрока по нанесённому уроу" << endl;
        int damageCount[10];
        for (int i = 0; i < playersCount; i++)
        {
            damageCount[i] = i;
        }
      
        for (int i = 0; i < playersCount-1; i++)
        {
            for (int j = 0; j < playersCount - i - 1; j++)
            {
                if (players[damageCount[j]].damageTop < players[damageCount[j + 1]].damageTop)
                {
                    int temp = damageCount[j];
                    damageCount[j] = damageCount[j + 1];
                    damageCount[j] = temp;
                }
            }
        }

        for (int i = 0; i < min(3, playersCount); i++)
        {
            cout << i + 1 << "" << players[damageCount[i]].name << "" << players[damageCount[i]].damageTop << ""<<endl;
        }
        CloseHandle(hboss);
        CloseHandle(hPlayer);
        CloseHandle(hBossSpecEv);
        return 0;
    }

}
