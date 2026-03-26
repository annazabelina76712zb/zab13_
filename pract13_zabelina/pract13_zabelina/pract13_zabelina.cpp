#include <iostream>
#include <windows.h>
#include <thread>
using namespace std;

struct Bayum {
    long long health = 9000000000;
    int resist = 44;
    int damage = 73843;
    int specialDamage = 150000;
    int attackCooldown = 5;
    int specialCooldown = 10;
};

struct Player {
    long health = 500000;
    int damage = 12000;
    int specialDamage = 30000;
    int attackCooldown = 2;
    int defense = 20;
    int dodgeChance = 15;
    char name[64];
    bool isAlive = true;
    long long totalDamage = 0;
};

Bayum boss;
bool bossAlive = true;
Player players[10];
int playersCount = 0;
int alivePlCount = 0;
HANDLE hPlayerAttack;
CRITICAL_SECTION cs;

long long DamageToBoss(int dmg)
{
    return (long long)dmg * (100 - boss.resist) / 100;
}

long long DamageToPlayer(int dmg, int def)
{
    return (long long)dmg * (100 - def) / 100;
}

void BossThread()
{
    while (bossAlive)
    {
        Sleep(boss.attackCooldown * 1000);

        if (!bossAlive) break;

        EnterCriticalSection(&cs);

        int alive[10];
        int cnt = 0;

        for (int i = 0; i < playersCount; i++)
        {
            if (players[i].isAlive && players[i].health > 0)
            {
                alive[cnt] = i;
                cnt++;
            }
        }

        if (cnt > 0)
        {
            int target = alive[rand() % cnt];

            if (rand() % 100 < players[target].dodgeChance)
            {
                cout << "Игрок: " << players[target].name << " уклонился!" << endl;
            }
            else
            {
                long long dmg = DamageToPlayer(boss.damage, players[target].defense);
                players[target].health -= dmg;
                cout << "Атака Босса: " << players[target].name << " получил " << dmg << " урона" << endl;
                cout << "Здоровье игрока: " << players[target].health << endl;

                if (players[target].health <= 0)
                {
                    players[target].isAlive = false;
                    alivePlCount--;
                    cout << "Смерть: " << players[target].name << " погиб!" << endl;
                }
            }
            cout << endl;
        }

        LeaveCriticalSection(&cs);
        SetEvent(hPlayerAttack);
    }
}
void BossSpecialThread()
{



    while (bossAlive)
    {
        Sleep(boss.specialCooldown * 1000);

        if (!bossAlive) break;

        EnterCriticalSection(&cs);

        int aliveCount = 0;
        for (int i = 0; i < playersCount; i++)
        {
            if (players[i].isAlive) aliveCount++;
        }

        if (aliveCount > 0)
        {
            double mult = 1.0;
            if (aliveCount > 1)
            {
                mult = 1.0 - 0.05 * (aliveCount - 1);
            }

            int specDmg = (int)(boss.specialDamage * mult);

            cout << "\nСПЕЦИАЛЬНАЯ АТАКА БОССА" << endl;
            cout << "Урон по всем: " << specDmg << endl;

            for (int i = 0; i < playersCount; i++)
            {
                if (players[i].isAlive)
                {
                    long long dmg = DamageToPlayer(specDmg, players[i].defense);
                    players[i].health -= dmg;
                    cout << players[i].name << " получил " << dmg << " урона" << endl;
                    cout << "Здоровье: " << players[i].health << endl;

                    if (players[i].health <= 0)
                    {
                        players[i].isAlive = false;
                        alivePlCount--;
                        cout << "Смерть: " << players[i].name << " погиб!" << endl;
                    }
                }
            }
            cout << endl;
        }

        LeaveCriticalSection(&cs);
        SetEvent(hPlayerAttack);
    }
}

void PlayerThread(int id)
{
    while (bossAlive && players[id].isAlive)
    {
        Sleep(500);
        WaitForSingleObject(hPlayerAttack, INFINITE);

        if (!bossAlive || !players[id].isAlive) break;

        EnterCriticalSection(&cs);

        long long dmg = DamageToBoss(players[id].damage);
        boss.health -= dmg;
        players[id].totalDamage += dmg;

        cout << "Игрок " << players[id].name << " атакует босса" << endl;
        cout << "Нанесено урона: " << dmg << endl;
        cout << "Здоровье босса: " << boss.health << endl;

        if (boss.health <= 0)
        {
            bossAlive = false;
            cout << "\nБОСС ПОВЕРЖЕН" << endl;
            LeaveCriticalSection(&cs);
            break;
        }

        LeaveCriticalSection(&cs);
        Sleep(players[id].attackCooldown * 1000);
    }
}

int main()
{
    setlocale(LC_ALL, "ru");


    InitializeCriticalSection(&cs);
    hPlayerAttack = CreateEvent(NULL, FALSE, FALSE, NULL);

    cout << "СРАЖЕНИЕ С БОССОМ" << endl;
    cout << "Введите количество игроков (1-10): ";
    cin >> playersCount;

    if (playersCount < 1 || playersCount > 10)
    {
        cout << "Неверное количество игроков!" << endl;
        DeleteCriticalSection(&cs);
        CloseHandle(hPlayerAttack);
        return 1;
    }

    for (int i = 0; i < playersCount; i++)
    {
        string name =players[i].name;
        cout << "Введите имя игрока " << i + 1 << ": ";
        cin >> players[i].name;
        players[i].health = 500000;
        players[i].isAlive = true;
        players[i].totalDamage = 0;
    }

    alivePlCount = playersCount;

    cout << "\nБИТВА НАЧИНАЕТСЯ!" << endl;
    cout << "Босс: Здоровье " << boss.health << endl;
    cout << "Сопротивление: " << boss.resist << "%" << endl;
    cout << endl;

    thread bossThread(BossThread);
    thread bossSpecThread(BossSpecialThread);
    thread playersThread[10];

    for (int i = 0; i < playersCount; i++)
    {
        playersThread[i] = thread(PlayerThread, i);
    }

    while (bossAlive && alivePlCount > 0)
    {
        Sleep(100);
    }

    bossAlive = false;
    SetEvent(hPlayerAttack);

    bossThread.join();
    bossSpecThread.join();

    for (int i = 0; i < playersCount; i++)
    {
        if (playersThread[i].joinable())
        {
            playersThread[i].join();
        }
    }

    cout << "\nРЕЗУЛЬТАТ БИТВЫ" << endl;

    if (boss.health <= 0)
    {
        cout << "ПОБЕДИТЕЛЬ: Команда игроков!" << endl;
       
    }
    else
    {
        cout << "ПОБЕДИТЕЛЬ: Босс!" << endl;
     
    }

    cout << "\nТОП-3 ИГРОКА ПО УРОНУ" << endl;

    int idx[10];
    for (int i = 0; i < playersCount; i++)
    {
        idx[i] = i;
    }

    for (int i = 0; i < playersCount - 1; i++)
    {
        for (int j = 0; j < playersCount - i - 1; j++)
        {
            if (players[idx[j]].totalDamage < players[idx[j + 1]].totalDamage)
            {
                int temp = idx[j];
                idx[j] = idx[j + 1];
                idx[j + 1] = temp;
            }
        }
    }

    for (int i = 0; i < min(3, playersCount); i++)
    {
        cout << i + 1 << ". " << players[idx[i]].name;
        cout << " - " << players[idx[i]].totalDamage << " урона" << endl;
    }

    DeleteCriticalSection(&cs);
    CloseHandle(hPlayerAttack);

    system("pause");
    return 0;
}