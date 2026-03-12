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
               
            }
        }

    }
    SetEvent(hPlayer);
    this_thread::sleep_for(chrono::seconds(boss.attackCooldawn));
}


int main()
{
    setlocale(LC_ALL, "RU");
    hboss = CreateEvent(NULL, FALSE, TRUE, L"BossEvent");
    cout << "Введите количество играков (1-10)" << endl;
    cin >> playersCount;
    if (playersCount > 10 || playersCount < 1)
    {
        cout << "Неверное количество играков!" << endl;
        for (int i =0;i<playersCount; i++)
        {
            cin >> players[i].name;
            players[i].isAlive = true;
            
        }
    }

}
